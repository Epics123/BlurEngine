#include "Renderer.h"
#include "../VulkanCore/Swapchain.h"

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

	RenderingContext = std::make_unique<VulkanCore::Context>(AppWindow, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

	const VulkanCore::SwapChainSupportDetails SwapChainSupport = RenderingContext->GetSupportDetails();
	const VkFormat SwapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
	const VkSurfaceFormatKHR SwapchainSurfaceFormat = RenderingContext->ChooseSwapSurfaceFormat(SwapChainSupport.Formats);
	
	// See https://developer.nvidia.com/blog/advanced-api-performance-vulkan-clearing-and-presenting/ for NVIDIA recommendations on presenting
	const VkPresentModeKHR SwapchainPresentMode = RenderingContext->ChooseSwapchainPresentMode(SwapChainSupport.PresentModes, VK_PRESENT_MODE_FIFO_KHR);
	const VkExtent2D SwapchainExtent = RenderingContext->ChooseSwapchainExtents(SwapChainSupport.Capabilities, AppWindow->GetExtent());	
	
	RenderingContext->CreateSwapchain(SwapchainFormat, SwapchainSurfaceFormat, SwapchainPresentMode, SwapchainExtent);
	
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
	PassInitInfo.FinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	std::shared_ptr<VulkanCore::RenderPass> RenderPass = RenderingContext->CreateRenderPass({PassInitInfo}, VK_PIPELINE_BIND_POINT_GRAPHICS, {}, "Simple Triangle");

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

	GraphicsPipeline = RenderingContext->CreateGraphicsPipeline(GraphicsPipelineDesc, RenderPass->GetVkRenderPass(), "Simple Triangle");
}
