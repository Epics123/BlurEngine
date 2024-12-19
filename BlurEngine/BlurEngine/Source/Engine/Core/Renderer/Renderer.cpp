#include "Renderer.h"
#include "../VulkanCore/Swapchain.h"

#include <vulkan/vulkan.h>

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
}
