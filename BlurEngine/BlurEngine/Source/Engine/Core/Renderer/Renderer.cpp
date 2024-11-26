#include "Renderer.h"

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
}
