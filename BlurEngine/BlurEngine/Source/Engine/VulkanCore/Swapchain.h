#pragma once

#include "VulkanCommon.h"
#include "Utility.h"

namespace VulkanCore
{
	class Context;
	class PhysicalDevice;
	class Texture;

	class Swapchain final
	{
	public:
		Swapchain() = default;
		Swapchain(const Context& DeviceContext, const PhysicalDevice& GPUDevice, VkSurfaceKHR Surface, VkQueue PresentQueue,
			VkSurfaceFormatKHR SurfaceFormat, VkPresentModeKHR PresentMode, VkExtent2D Extent, const std::string& Name = "");
		Swapchain(const Context& DeviceContext, const PhysicalDevice& GPUDevice, VkSurfaceKHR Surface, VkQueue PresentQueue,
			VkExtent2D Extent, std::shared_ptr<Swapchain> OldSwapchain);

		~Swapchain();

		uint32_t GetImageCount() const
		{
			return static_cast<uint32_t>(Images.size());
		}

		uint32_t CurrentImageIndex() const { return ImageIndex; }

		VkFormat GetImageFormat() const { return SwapchainImageFormat; }

		VkExtent2D GetExtent() const { return SwapchainExtent; }

		VkSurfaceFormatKHR GetSurfaceFormat() const { return SwapchainSurfaceFormat; }

		VkPresentModeKHR GetPresentMode() const { return CurrentPresentMode; }

		VkSwapchainKHR GetVulkanSwapchain() const { return VulkanSwapchain; }

		std::shared_ptr<Texture> GetTexture(uint32_t Index)
		{
			ASSERT(Index < Images.size(), "Index is greater than the number of images in the swapchain");
			return Images[Index];
		}

	private:
		void CreateSwapchain(const Context& DeviceContext, const PhysicalDevice& GPUDevice, VkSurfaceKHR Surface, VkFormat ImageFormat, 
							 VkColorSpaceKHR ImageColorSpace, VkPresentModeKHR PresentMode, VkExtent2D Extent, VkSwapchainKHR OldSwapchain = VK_NULL_HANDLE);
		void CreateSwapchainImages(const Context& DeviceContext, VkFormat Format, const VkExtent2D& Extent);

		void CreateSemaphores();
		void CreateFence();

	private:
		VkDevice VulkanDevice = VK_NULL_HANDLE;
		VkSwapchainKHR VulkanSwapchain = VK_NULL_HANDLE;
		VkQueue SwapchainPresentQueue = VK_NULL_HANDLE;

		std::vector<std::shared_ptr<Texture>> Images;
		VkSemaphore ImageAvailable = VK_NULL_HANDLE;
		VkSemaphore ImageRendered = VK_NULL_HANDLE;
		uint32_t ImageIndex = 0;
		VkFormat SwapchainImageFormat;

		VkExtent2D SwapchainExtent;
		VkFence AcquireFence = VK_NULL_HANDLE;

		VkSurfaceFormatKHR SwapchainSurfaceFormat;
		VkPresentModeKHR CurrentPresentMode;
	};
}