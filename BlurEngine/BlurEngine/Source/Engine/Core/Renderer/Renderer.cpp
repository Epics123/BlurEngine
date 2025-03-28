#include "Renderer.h"
#include "../VulkanCore/Swapchain.h"
#include "../VulkanCore/Texture.h"

#include "../VulkanCore/ShaderModule.h" // Temporary

#include "../AssetManagement/ObjLoader.h"

#include <vulkan/vulkan.h>

std::filesystem::path Renderer::sShaderDirectory;
std::filesystem::path Renderer::sModelDirectory;

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
	VulkanCore::Context::EnableDynamicRenderingFeature();
	VulkanCore::Context::EnableDynamicStateFeature();

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

	EngineCore::RingBuffer CameraBuffer(*RenderingContext.get(), RenderingContext->GetSwapchain()->GetImageCount(), sizeof(CameraUniforms));
	MainCamera.Init(CameraBuffer);

	Renderer::sModelDirectory = std::filesystem::current_path() / "Source/Resources/Models";
	Renderer::sModelDirectory.make_preferred();

	EngineCore::OBJLoader ObjLoader;
	auto Teapot = ObjLoader.Load(Renderer::sModelDirectory.string() + "/teapot.obj");
	SceneMeshes.push_back(Teapot);

	Renderer::sShaderDirectory = std::filesystem::current_path() / "Source/Resources/Shaders";
	Renderer::sShaderDirectory.make_preferred();

	const auto VertexShaderPath = Renderer::sShaderDirectory / "IndirectDraw.vert";
	const auto FragShaderPath = Renderer::sShaderDirectory / "IndirectDraw.frag";

	const std::shared_ptr<VulkanCore::ShaderModule> VertexShader = RenderingContext->CreateShaderModule(VertexShaderPath.string(), VK_SHADER_STAGE_VERTEX_BIT);
	const std::shared_ptr<VulkanCore::ShaderModule> FragmentShader = RenderingContext->CreateShaderModule(FragShaderPath.string(), VK_SHADER_STAGE_FRAGMENT_BIT);

	std::vector<VulkanCore::SetDescriptor> Sets;
	VulkanCore::SetDescriptor Desc;

	Desc.SetIndex = CAMERA_SET;
	Desc.Bindings = {VkDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)};
	Sets.push_back(Desc);

	Desc.SetIndex = TEXTURES_SET;
	Desc.Bindings = { VkDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) };
	Sets.push_back(Desc);

	Desc.SetIndex = SAMPLER_SET;
	Desc.Bindings = { VkDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_SAMPLER, 1000, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) };
	Sets.push_back(Desc);

	Desc.SetIndex = STORAGE_BUFFER_SET;
	Desc.Bindings = { VkDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) };
	Sets.push_back(Desc);

	VulkanCore::TextureCreateInfo DepthTextureInfo;
	DepthTextureInfo.Type = VK_IMAGE_TYPE_2D;
	DepthTextureInfo.Format = VK_FORMAT_D24_UNORM_S8_UINT;
	DepthTextureInfo.Flags = 0;
	DepthTextureInfo.UsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	DepthTextureInfo.Extents = VkExtent3D{ RenderingContext->GetSwapchain()->GetExtent().width, RenderingContext->GetSwapchain()->GetExtent().height, 1 };
	DepthTextureInfo.NumMipLevels = 1;
	DepthTextureInfo.LayerCount = 1;
	DepthTextureInfo.MemoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	DepthTextureInfo.bGenerateMips = false;
	DepthTextureInfo.MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	DepthTextureInfo.Name = "Depth Buffer";

	std::shared_ptr<VulkanCore::Texture> DepthTexture = RenderingContext->CreateTexture(DepthTextureInfo);

	VulkanCore::RenderPassInitInfo PassInitInfo;
	PassInitInfo.AttachmentTextures = { RenderingContext->GetSwapchain()->GetTexture(0), DepthTexture };
	PassInitInfo.LoadOps = { VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_LOAD_OP_CLEAR };
	PassInitInfo.StoreOps = { VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_STORE_OP_DONT_CARE };
	PassInitInfo.InitialLayouts = { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED };
	PassInitInfo.FinalLayouts = { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	IndirectDrawPass = RenderingContext->CreateRenderPass({PassInitInfo}, VK_PIPELINE_BIND_POINT_GRAPHICS, {}, "Indirect Draw");

	for(uint32_t i = 0; i < RenderingContext->GetSwapchain()->GetImageCount(); i++)
	{
		VulkanCore::FramebufferCreateInfo FramebufferInfo;
		FramebufferInfo.Attachments = { RenderingContext->GetSwapchain()->GetTexture(i), DepthTexture };
		FramebufferInfo.DepthAttachment = nullptr;
		FramebufferInfo.StencilAttachment = nullptr;
		FramebufferInfo.Name = "Swapchain Framebuffer " + std::to_string(i);

		RenderingContext->GetSwapchain()->SetFramebuffer(RenderingContext->CreateFramebuffer(IndirectDrawPass->GetVkRenderPass(), FramebufferInfo), i);
	}

	VulkanCore::GraphicsPipelineDescriptor GraphicsPipelineDesc{};
	GraphicsPipelineDesc.SetDescriptors = Sets;
	GraphicsPipelineDesc.VertexShader = VertexShader;
	GraphicsPipelineDesc.FragmentShader = FragmentShader;
	GraphicsPipelineDesc.DynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE };
	GraphicsPipelineDesc.ColorTextureFormats = {SwapchainFormat};
	GraphicsPipelineDesc.DepthTextureFormat = DepthTexture->GetFormat();
	GraphicsPipelineDesc.Viewport = RenderingContext->GetSwapchain()->GetExtent();
	GraphicsPipelineDesc.bDepthTestEnable = true;
	GraphicsPipelineDesc.bDepthWriteEnable = true;
	GraphicsPipelineDesc.DepthCompareOperation = VK_COMPARE_OP_LESS;

	GraphicsPipeline = RenderingContext->CreateGraphicsPipeline(GraphicsPipelineDesc, IndirectDrawPass->GetVkRenderPass(), "Indirect Draw");
	GraphicsPipeline->AllocateDescriptors({ {CAMERA_SET, 3}, {TEXTURES_SET, 1}, {SAMPLER_SET, 1}, {STORAGE_BUFFER_SET, 1} });
	for(uint32_t i = 0; i < MainCamera.GetCameraBuffer()->GetRingSize(); i++)
	{
		GraphicsPipeline->BindResource(CAMERA_SET, BINDING_0, i, MainCamera.GetCameraBuffer()->GetBuffer(i), 0, sizeof(CameraUniforms), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	}

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
	
	/*if(RenderingContext->GetSwapchain()->GetFramebuffers()[SwapchainImageIndex] == nullptr || ActiveWindow->WasResized())
	{
		VulkanCore::FramebufferCreateInfo FramebufferInfo;
		FramebufferInfo.Attachments = {SwapchainImage};
		FramebufferInfo.DepthAttachment = nullptr;
		FramebufferInfo.StencilAttachment = nullptr;
		FramebufferInfo.Name = "Swapchain Framebuffer " + std::to_string(SwapchainImageIndex);

		RenderingContext->GetSwapchain()->SetFramebuffer(RenderingContext->CreateFramebuffer(IndirectDrawPass->GetVkRenderPass(), FramebufferInfo), SwapchainImageIndex);
	}*/

	VkCommandBuffer CmdBuffer = GraphicsCommandManager->BeginCmdBuffer();

	constexpr VkClearValue ClearColor{ 0.0f, 0.0f, 0.0f, 0.0f };
	VkRenderPassBeginInfo RenderPassInfo{};
	RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RenderPassInfo.renderPass = IndirectDrawPass->GetVkRenderPass();
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
