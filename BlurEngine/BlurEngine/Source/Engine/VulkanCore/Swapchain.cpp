#include "Swapchain.h"
#include "Context.h"
#include "Texture.h"

namespace VulkanCore
{
	Swapchain::Swapchain(const Context& DeviceContext, const PhysicalDevice& GPUDevice, VkSurfaceKHR Surface, VkQueue PresentQueue,
		VkSurfaceFormatKHR SurfaceFormat, VkPresentModeKHR PresentMode, VkExtent2D Extent, const std::string& Name)
		: VulkanDevice{DeviceContext.GetDevice()}, SwapchainPresentQueue{PresentQueue}, SwapchainExtent{Extent}, 
		  SwapchainSurfaceFormat{SurfaceFormat}, CurrentPresentMode{PresentMode}
	{
		CreateSwapchain(DeviceContext, GPUDevice, Surface, SurfaceFormat.format, SurfaceFormat.colorSpace, PresentMode, SwapchainExtent);
	}

	Swapchain::Swapchain(const Context& DeviceContext, const PhysicalDevice& GPUDevice, VkSurfaceKHR Surface, VkQueue PresentQueue,
						 VkExtent2D Extent, std::shared_ptr<Swapchain> OldSwapchain)
		: VulkanDevice{DeviceContext.GetDevice()}, SwapchainPresentQueue{PresentQueue}, SwapchainExtent{Extent}
	{
		SwapchainSurfaceFormat = OldSwapchain->GetSurfaceFormat();
		CurrentPresentMode = OldSwapchain->GetPresentMode();
		CreateSwapchain(DeviceContext, GPUDevice, Surface, SwapchainSurfaceFormat.format, SwapchainSurfaceFormat.colorSpace, 
						CurrentPresentMode, SwapchainExtent, OldSwapchain->GetVulkanSwapchain());
	}

	Swapchain::~Swapchain()
	{
		VK_CHECK(vkWaitForFences(VulkanDevice, 1, &AcquireFence, VK_TRUE, UINT64_MAX));
		vkDestroyFence(VulkanDevice, AcquireFence, nullptr);
		vkDestroySemaphore(VulkanDevice, ImageRendered, nullptr);
		vkDestroySemaphore(VulkanDevice, ImageAvailable, nullptr);
		vkDestroySwapchainKHR(VulkanDevice, VulkanSwapchain, nullptr);
	}

	std::shared_ptr<VulkanCore::Framebuffer> Swapchain::GetFramebuffer(uint32_t Index)
	{
		if(Index < 0 || Index >= (uint32_t)Framebuffers.size())
		{
			return Framebuffers[0];
		}

		return Framebuffers[Index];
	}

	void Swapchain::CreateSwapchain(const Context& DeviceContext, const PhysicalDevice& GPUDevice, VkSurfaceKHR Surface, VkFormat ImageFormat,
									VkColorSpaceKHR ImageColorSpace, VkPresentModeKHR PresentMode, VkExtent2D Extent, VkSwapchainKHR OldSwapchain)
	{
		const uint32_t MinImageCount = GPUDevice.GetSurfaceCapabilities().minImageCount;
		const uint32_t NumImages = std::clamp(MinImageCount + 1, MinImageCount, GPUDevice.GetSurfaceCapabilities().maxImageCount);

		const std::optional<uint32_t> PresentationFamilyIndex = GPUDevice.GetPresentationFamilyIndex();
		ASSERT(PresentationFamilyIndex.has_value(), "There are no presentation queues available for the Swapchain!");

		const bool IsPresentationQueueShared = GPUDevice.GetGraphicsFamilyIndex().value() == PresentationFamilyIndex.value();

		std::array<uint32_t, 2> FamilyIndices{ GPUDevice.GetGraphicsFamilyIndex().value(), PresentationFamilyIndex.value() };

		VkSwapchainCreateInfoKHR CreateInfo;
		CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		CreateInfo.flags = VkSwapchainCreateFlagsKHR();
		CreateInfo.surface = Surface;
		CreateInfo.minImageCount = NumImages;
		CreateInfo.imageFormat = ImageFormat;
		CreateInfo.imageColorSpace = ImageColorSpace;
		CreateInfo.imageExtent = Extent;
		CreateInfo.imageArrayLayers = 1;
		CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		CreateInfo.imageSharingMode = IsPresentationQueueShared ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
		CreateInfo.queueFamilyIndexCount = IsPresentationQueueShared ? 0u : 2u;
		CreateInfo.pQueueFamilyIndices = IsPresentationQueueShared ? nullptr : FamilyIndices.data();
		CreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		CreateInfo.presentMode = PresentMode;
		CreateInfo.clipped = VK_TRUE;
		CreateInfo.oldSwapchain = OldSwapchain;
		CreateInfo.pNext = VK_NULL_HANDLE;

		VK_CHECK(vkCreateSwapchainKHR(VulkanDevice, &CreateInfo, nullptr, &VulkanSwapchain));

		CreateSwapchainImages(DeviceContext, ImageFormat, Extent);
		CreateSemaphores();
		CreateFence();

		Framebuffers.resize(NumImages);
	}

	void Swapchain::CreateSwapchainImages(const Context& DeviceContext, VkFormat Format, const VkExtent2D& Extent)
	{
		/* We only specified a minimum number of images in the swap chain, so the implementation is
		   allowed to create a swap chain with more. That's why we'll first query the final number of
		   images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
		   retrieve the handles.
		*/

		uint32_t ImageCount = 0;
		vkGetSwapchainImagesKHR(DeviceContext.GetDevice(), VulkanSwapchain, &ImageCount, nullptr);
		std::vector<VkImage> SwapchainImages(ImageCount);
		vkGetSwapchainImagesKHR(DeviceContext.GetDevice(), VulkanSwapchain, &ImageCount, SwapchainImages.data());

		Images.reserve(ImageCount);
		for(size_t i = 0; i < ImageCount; i++)
		{
			VkExtent3D Extents;
			Extents.width = Extent.width;
			Extents.height = Extent.height;
			Extents.depth = 1;

			const std::string DebugName = "Swapchain Image " + std::to_string(i);

			std::shared_ptr<Texture> SwapchainImage = std::make_shared<Texture>(DeviceContext, VulkanDevice, SwapchainImages[i], Format, Extents, 1, false, DebugName);
			Images.emplace_back(SwapchainImage);
		}
	}

	void Swapchain::CreateSemaphores()
	{
		VkSemaphoreCreateInfo CreateInfo;
		CreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		CreateInfo.pNext = VK_NULL_HANDLE;
		CreateInfo.flags = 0;

		VK_CHECK(vkCreateSemaphore(VulkanDevice, &CreateInfo, nullptr, &ImageAvailable));
		VK_CHECK(vkCreateSemaphore(VulkanDevice, &CreateInfo, nullptr, &ImageRendered));
	}

	void Swapchain::CreateFence()
	{
		VkFenceCreateInfo CreateInfo;
		CreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		CreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		CreateInfo.pNext = VK_NULL_HANDLE;

		VK_CHECK(vkCreateFence(VulkanDevice, &CreateInfo, nullptr, &AcquireFence));
	}

}

