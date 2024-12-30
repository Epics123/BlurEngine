#pragma once

#include "VulkanCommon.h"
#include "Utility.h"

#include <vma/vk_mem_alloc.h>

namespace VulkanCore
{

class Context;

struct TextureCreateInfo
{
	VkImageType Type;
	VkFormat Format;
	VkImageCreateFlags Flags = 0;
	VkImageUsageFlags UsageFlags;
	VkExtent3D Extents;
	uint32_t NumMipLevels;
	uint32_t LayerCount;
	VkMemoryPropertyFlags MemoryFlags;
	VkSampleCountFlagBits MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	bool bGenerateMips = false;
	bool bMultiview = false;
	VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL;
	std::string Name = "";
};

class Texture final
{
public:
	MOVABLE_ONLY(Texture);

	explicit Texture(const Context& InContext, const TextureCreateInfo& CreateInfo);

	explicit Texture(const Context& InContext, VkDevice Device, VkImage Image, VkFormat Format, VkExtent3D Extents, 
					 uint32_t NumLayers = 1, bool IsMultiview = false, const std::string& Name = "");

	~Texture();

	bool IsDepth() const;
	bool IsStencil() const;

	VkFormat GetFormat() const { return TextureFormat; }
	VkSampleCountFlagBits GetSampleCount() const { return MsaaSamples; }
	VkImageLayout GetLayout() const { return ImageLayout; }
	VkImageView GetImageView(uint32_t MipLevel);
	VkExtent3D GetExtents() const { return TextureExtents; }

private:
	VkImageView CreateImageView(VkImageViewType ImageViewType, VkFormat ImageFormat, uint32_t NumMips, uint32_t Layers, const std::string& Name);

	uint32_t GetMipLevelCount(uint32_t TextureWidth, uint32_t TextureHeight) const;

private:
	const Context& DeviceContext;

	VmaAllocator Allocator = nullptr;
	VmaAllocation Allocation = nullptr;

	VkDeviceSize DeviceSize = 0;

	VkImageUsageFlags UsageFlags = 0;
	VkImageCreateFlags Flags = 0;

	VkImageType ImageType = VK_IMAGE_TYPE_2D;
	VkImage TextureImage = VK_NULL_HANDLE;
	VkImageView ImageView = VK_NULL_HANDLE;
	std::unordered_map<uint32_t, VkImageView> ImageViewFramebuffers;

	VkFormat TextureFormat = VK_FORMAT_UNDEFINED;
	VkExtent3D TextureExtents;
	VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	bool bOwnsVkImage = false;

	uint32_t MipLevels = 1;
	uint32_t LayerCount = 1;
	bool bMultiview = false;
	bool bGenerateMips = false;

	VkImageViewType ViewType;
	VkSampleCountFlagBits MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling ImageTiling = VK_IMAGE_TILING_OPTIMAL;

	std::string DebugName;
};

}