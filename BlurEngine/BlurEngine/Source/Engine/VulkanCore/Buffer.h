#pragma once

#include "VulkanCommon.h"
#include "Utility.h"

#include <vma/vk_mem_alloc.h>

#include <unordered_map>

namespace VulkanCore
{

class Context;

class Buffer final
{
public:
	MOVABLE_ONLY(Buffer);

	// For staging buffer
	explicit Buffer(const Context& DeviceContext, VmaAllocator InAllocator, VkDeviceSize Size, VkBufferUsageFlags Usage, 
					std::shared_ptr<Buffer> ActualBuffer, const std::string& Name = "");

	explicit Buffer(const Context& DeviceContext, VmaAllocator InAllocator, const VkBufferCreateInfo& CreateInfo, 
					const VmaAllocationCreateInfo& AllocInfo, const std::string& Name = "");

	~Buffer();

	void Upload(VkDeviceSize Offset = 0) const;
	void Upload(VkDeviceSize Offset, VkDeviceSize Size) const;
	// Uploads staging buffer to the GPU
	void UploadStagingBuffer(const VkCommandBuffer CmdBuffer, uint64_t SrcOffset = 0, uint64_t DstOffset = 0);

	void CopyToBuffer(const void* Data, size_t Size);

	VkDeviceSize GetSize() const { return DeviceSize; }
	VkBuffer GetVkBuffer() const {return VulkanBuffer; }
	VkDeviceAddress GetDeviceAddress();

	VkBufferView RequestBufferView(VkFormat ViewFormat);

private:
	VkDevice VulkanDevice = VK_NULL_HANDLE;

	VmaAllocator Allocator;
	VmaAllocationCreateInfo AllocCreateInfo = {};
	VmaAllocation Allocation;
	VmaAllocationInfo AllocationInfo = {};

	VkDeviceSize DeviceSize;
	VkBufferUsageFlags UsageFlags;

	VkBuffer VulkanBuffer = VK_NULL_HANDLE;
	std::shared_ptr<Buffer> StagingBuffer = nullptr;

	mutable VkDeviceAddress BufferDeviceAddress = 0;
	mutable void* MappedMemory = nullptr;

	std::unordered_map<VkFormat, VkBufferView> BufferViews;

	std::string DebugName;
};

}