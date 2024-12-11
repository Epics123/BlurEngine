#include "Texture.h"
#include "Context.h"

namespace VulkanCore
{
	
	Texture::Texture(const Context& InContext, const TextureCreateInfo& CreateInfo)
		: DeviceContext{InContext}, Allocator{InContext.GetAllocator()}, UsageFlags{CreateInfo.UsageFlags}, Flags{CreateInfo.Flags},
		  ImageType{CreateInfo.Type}, TextureFormat{CreateInfo.Format}, TextureExtents{CreateInfo.Extents}, bOwnsVkImage{true},
		  MipLevels{CreateInfo.NumMipLevels}, LayerCount{CreateInfo.LayerCount}, bMultiview{CreateInfo.bMultiview}, bGenerateMips{CreateInfo.bGenerateMips},
		  MsaaSamples{CreateInfo.MsaaSamples}, ImageTiling{CreateInfo.Tiling}, DebugName{CreateInfo.Name}
	{
		DebugName = "Texture: " + CreateInfo.Name;

		ASSERT(TextureExtents.width > 0 && TextureExtents.height > 0, "Texture cannot have dimensions equal to 0");
		ASSERT(MipLevels > 0, "Texture must have at least one mip level");

		if(bGenerateMips)
		{
			MipLevels = GetMipLevelCount(TextureExtents.width, TextureExtents.height);
		}

		ASSERT(!(MipLevels > 1 && MsaaSamples != VK_SAMPLE_COUNT_1_BIT), "Multisampled images cannot have more than 1 mip level");

		VkImageCreateInfo ImageInfo{};
		ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageInfo.flags = Flags;
		ImageInfo.imageType = ImageType;
		ImageInfo.format = TextureFormat;
		ImageInfo.extent = TextureExtents;
		ImageInfo.mipLevels = MipLevels;
		ImageInfo.arrayLayers = LayerCount;
		ImageInfo.samples = MsaaSamples;
		ImageInfo.tiling = ImageTiling;
		ImageInfo.usage = UsageFlags;
		ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ImageInfo.pNext = VK_NULL_HANDLE;
		ImageInfo.pQueueFamilyIndices = VK_NULL_HANDLE;

		VmaAllocationCreateInfo AllocInfo{};
		AllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		AllocInfo.usage = CreateInfo.MemoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ? VMA_MEMORY_USAGE_AUTO_PREFER_HOST : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		AllocInfo.priority = 1.0f;

		VK_CHECK(vmaCreateImage(Allocator, &ImageInfo, &AllocInfo, &TextureImage, &Allocation, nullptr));

		if(Allocation != nullptr)
		{
			VmaAllocationInfo AllocationInfo;
			vmaGetAllocationInfo(Allocator, Allocation, &AllocationInfo);
			DeviceSize = AllocationInfo.size;
		}

		ViewType = VulkanUtils::ImageTypeToImageViewType(ImageType, Flags, bMultiview);

		ImageView = CreateImageView(ViewType, TextureFormat, MipLevels, LayerCount, CreateInfo.Name);
	}

	Texture::Texture(const Context& InContext, VkDevice Device, VkImage Image, VkFormat Format, 
					 VkExtent3D Extents, uint32_t NumLayers, bool IsMultiview, const std::string& Name)
		: DeviceContext{InContext}, TextureImage{Image}, TextureFormat{Format}, TextureExtents{Extents}, LayerCount{NumLayers},
		  bMultiview{IsMultiview}, DebugName{Name}
	{
		ImageView = CreateImageView(bMultiview ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, TextureFormat, 1, LayerCount, DebugName);
	}

	Texture::~Texture()
	{
		for(const auto View : ImageViewFramebuffers)
		{
			vkDestroyImageView(DeviceContext.GetDevice(), View.second, nullptr);
		}

		vkDestroyImageView(DeviceContext.GetDevice(), ImageView, nullptr);

		if(bOwnsVkImage)
		{
			vmaDestroyImage(Allocator, TextureImage, Allocation);
		}
	}

	bool Texture::IsDepth() const
	{
		return (TextureFormat == VK_FORMAT_D16_UNORM || TextureFormat == VK_FORMAT_D16_UNORM_S8_UINT || TextureFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
				TextureFormat == VK_FORMAT_D32_SFLOAT || TextureFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || TextureFormat == VK_FORMAT_X8_D24_UNORM_PACK32);
	}

	bool Texture::IsStencil() const
	{
		return (TextureFormat == VK_FORMAT_S8_UINT || TextureFormat == VK_FORMAT_D16_UNORM_S8_UINT || 
				TextureFormat == VK_FORMAT_D24_UNORM_S8_UINT || TextureFormat == VK_FORMAT_D32_SFLOAT_S8_UINT);
	}

	VkImageView Texture::CreateImageView(VkImageViewType ImageViewType, VkFormat ImageFormat, uint32_t NumMips, uint32_t Layers, const std::string& Name)
	{
		const VkImageAspectFlags AspectMask = IsDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : (IsStencil() ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT);

		VkImageViewCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		CreateInfo.flags = VkImageViewCreateFlags(0);
		CreateInfo.image = TextureImage;
		CreateInfo.viewType = ViewType;
		CreateInfo.format = TextureFormat;

		VkComponentMapping Components;
		Components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		Components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		Components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		Components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components = Components;

		VkImageSubresourceRange SubresourceRange;
		SubresourceRange.aspectMask = AspectMask;
		SubresourceRange.baseMipLevel = 0;
		SubresourceRange.levelCount = NumMips;
		SubresourceRange.baseArrayLayer = 0;
		SubresourceRange.layerCount = bMultiview ? VK_REMAINING_ARRAY_LAYERS : Layers;
		CreateInfo.subresourceRange = SubresourceRange;

		VkImageView Result{VK_NULL_HANDLE};
		VK_CHECK(vkCreateImageView(DeviceContext.GetDevice(), &CreateInfo, nullptr, &Result));

		return Result;
	}

	uint32_t Texture::GetMipLevelCount(uint32_t TextureWidth, uint32_t TextureHeight) const
	{
		return static_cast<uint32_t>(std::floor(std::log2(std::max(TextureWidth, TextureHeight))));
	}

}
