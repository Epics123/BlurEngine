#include "Renderer.h"
#include "../VulkanCore/Swapchain.h"
#include "../VulkanCore/Texture.h"

#include "../VulkanCore/ShaderModule.h" // Temporary

#include <vulkan/vulkan.h>

std::filesystem::path Renderer::sShaderDirectory;

Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

void Renderer::Init(std::shared_ptr<Window> AppWindow)
{
	VulkanCore::Context::EndableDefaultFeatures();
	VulkanCore::Context::EnableIndirectRenderingFeature();
	VulkanCore::Context::EnableSyncronizationFeature();
	VulkanCore::Context::EnableBufferDeviceAddressFeature();

	ActiveWindow = AppWindow;

	RenderingContext = std::make_unique<VulkanCore::Context>(AppWindow, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

	const VulkanCore::SwapChainSupportDetails SwapChainSupport = RenderingContext->GetSupportDetails();
	const VkFormat SwapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
	const VkSurfaceFormatKHR SwapchainSurfaceFormat = RenderingContext->ChooseSwapSurfaceFormat(SwapChainSupport.Formats);
	
	// See https://developer.nvidia.com/blog/advanced-api-performance-vulkan-clearing-and-presenting/ for NVIDIA recommendations on presenting
	const VkPresentModeKHR SwapchainPresentMode = RenderingContext->ChooseSwapchainPresentMode(SwapChainSupport.PresentModes, VK_PRESENT_MODE_FIFO_KHR);
	const VkExtent2D SwapchainExtent = RenderingContext->ChooseSwapchainExtents(SwapChainSupport.Capabilities, AppWindow->GetExtent());	
	
	RenderingContext->CreateSwapchain(SwapchainFormat, SwapchainSurfaceFormat, SwapchainPresentMode, SwapchainExtent);

	RenderArea.offset = {0, 0};
	RenderArea.extent = SwapchainExtent;
	
	FramesInFlight = RenderingContext->GetSwapchain()->GetImageCount();

	Renderer::sShaderDirectory = std::filesystem::current_path() / "Source/Resources/Shaders";
	Renderer::sShaderDirectory.make_preferred();

	const auto VertexShaderPath = Renderer::sShaderDirectory / "SimpleTriangle.vert";
	const auto FragShaderPath = Renderer::sShaderDirectory / "SimpleTriangle.frag";

	const std::shared_ptr<VulkanCore::ShaderModule> VertexShader = RenderingContext->CreateShaderModule(VertexShaderPath.string(), VK_SHADER_STAGE_VERTEX_BIT);
	const std::shared_ptr<VulkanCore::ShaderModule> FragmentShader = RenderingContext->CreateShaderModule(FragShaderPath.string(), VK_SHADER_STAGE_FRAGMENT_BIT);

	VulkanCore::RenderPassInitInfo PassInitInfo;
	PassInitInfo.AttachmentTexture = RenderingContext->GetSwapchain()->GetTexture(0);
	PassInitInfo.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	PassInitInfo.StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
	PassInitInfo.InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	PassInitInfo.FinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	SimpleTrianglePass = RenderingContext->CreateRenderPass({PassInitInfo}, VK_PIPELINE_BIND_POINT_GRAPHICS, {}, "Simple Triangle");

	VkViewport Viewport{};
	Viewport.x = 0;
	Viewport.y = 0;
	Viewport.width = static_cast<float>(RenderingContext->GetSwapchain()->GetExtent().width);
	Viewport.height = static_cast<float>(RenderingContext->GetSwapchain()->GetExtent().height);
	Viewport.minDepth = 0.0f;
	Viewport.maxDepth = 1.0f;

	VulkanCore::GraphicsPipelineDescriptor GraphicsPipelineDesc{};
	GraphicsPipelineDesc.VertexShader = VertexShader;
	GraphicsPipelineDesc.FragmentShader = FragmentShader;
	GraphicsPipelineDesc.ColorTextureFormats = {SwapchainFormat};
	GraphicsPipelineDesc.FrontFace = VK_FRONT_FACE_CLOCKWISE;
	GraphicsPipelineDesc.Viewport = Viewport;
	GraphicsPipelineDesc.bDepthTestEnable = false;

	GraphicsPipeline = RenderingContext->CreateGraphicsPipeline(GraphicsPipelineDesc, SimpleTrianglePass->GetVkRenderPass(), "Simple Triangle");

	const uint32_t ImageCount = RenderingContext->GetSwapchain()->GetImageCount();
	GraphicsCommandManager = RenderingContext->CreateGraphicsCommandQueue(ImageCount, ImageCount, -1, "Graphics Command Manager");
}

void Renderer::Draw(float DeltaTime)
{
	if (ActiveWindow->WasResized())
	{
		HandleWindowResized();
	}

	const std::shared_ptr<VulkanCore::Texture> SwapchainImage = RenderingContext->GetSwapchain()->AcquireImage();
	const uint32_t SwapchainImageIndex = RenderingContext->GetSwapchain()->CurrentImageIndex();
	
	if(RenderingContext->GetSwapchain()->GetFramebuffers()[SwapchainImageIndex] == nullptr || ActiveWindow->WasResized())
	{
		VulkanCore::FramebufferCreateInfo FramebufferInfo;
		FramebufferInfo.Attachments = {SwapchainImage};
		FramebufferInfo.DepthAttachment = nullptr;
		FramebufferInfo.StencilAttachment = nullptr;
		FramebufferInfo.Name = "Swapchain Framebuffer " + std::to_string(SwapchainImageIndex);

		RenderingContext->GetSwapchain()->SetFramebuffer(RenderingContext->CreateFramebuffer(SimpleTrianglePass->GetVkRenderPass(), FramebufferInfo), SwapchainImageIndex);
	}

	VkCommandBuffer CmdBuffer = GraphicsCommandManager->BeginCmdBuffer();

	constexpr VkClearValue ClearColor{ 0.0f, 0.0f, 0.0f, 0.0f };
	VkRenderPassBeginInfo RenderPassInfo{};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassInfo.renderPass = SimpleTrianglePass->GetVkRenderPass();
	RenderPassInfo.framebuffer = RenderingContext->GetSwapchain()->GetFramebuffer(SwapchainImageIndex)->GetVkFramebuffer();
	RenderPassInfo.renderArea = RenderArea;
	RenderPassInfo.clearValueCount = 1;
	RenderPassInfo.pClearValues = &ClearColor;
	RenderPassInfo.pNext = VK_NULL_HANDLE;

	vkCmdBeginRenderPass(CmdBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	GraphicsPipeline->Bind(CmdBuffer);

	vkCmdDraw(CmdBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(CmdBuffer);

	GraphicsCommandManager->EndCmdBuffer(CmdBuffer);

	constexpr VkPipelineStageFlags Flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	const VkSubmitInfo SubmitInfo = RenderingContext->GetSwapchain()->CreateSubmitInfo(&CmdBuffer, &Flags);
	GraphicsCommandManager->Submit(&SubmitInfo);
	GraphicsCommandManager->ToNextCmdBuffer();

	RenderingContext->GetSwapchain()->Present();

	ActiveWindow->SetFrameBufferResized(false);
}

void Renderer::DeviceWaitIdle()
{
	vkDeviceWaitIdle(RenderingContext->GetDevice());
}

void Renderer::WaitForAllSubmits()
{
	GraphicsCommandManager->WaitForAllSubmissions();
}

void Renderer::HandleWindowResized()
{
	VkExtent2D NewExtent = ActiveWindow->GetExtent();

	// Check if window is minimized 
	while (NewExtent.width == 0 || NewExtent.height == 0)
	{
		NewExtent = ActiveWindow->GetExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(RenderingContext->GetDevice());

	RenderingContext->RecreateSwapchain(NewExtent);

	RenderArea.offset = { 0, 0 };
	RenderArea.extent = RenderingContext->GetSwapchain()->GetExtent();

#if _DEBUG
	BE_INFO("Window resized to: {0} x {1}", NewExtent.width, NewExtent.height);
#endif
}
