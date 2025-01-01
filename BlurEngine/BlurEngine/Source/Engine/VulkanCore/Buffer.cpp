#include "Buffer.h"
#include "Context.h"

namespace VulkanCore
{
	
	Buffer::Buffer(const Context& DeviceContext, VmaAllocator InAllocator, VkDeviceSize Size, VkBufferUsageFlags Usage, 
				   std::shared_ptr<Buffer> ActualBuffer, const std::string& Name)
		: VulkanDevice{DeviceContext.GetDevice()}, Allocator{InAllocator}, DeviceSize{Size}, UsageFlags{Usage}, StagingBuffer{ActualBuffer}, DebugName{Name}
	{
		ASSERT(StagingBuffer, "Actual buffer must not be null when creating a staging buffer!");
		ASSERT(StagingBuffer->UsageFlags & VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Actual buffer must be dst buffer when creating a staging buffer!");
		ASSERT(StagingBuffer->AllocCreateInfo.usage == VMA_MEMORY_USAGE_GPU_ONLY,
			   "Actual buffer must be GPU only when creating a staging buffer. Buffer will be uploaded from CPU to this GPU buffer");

		VkBufferCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		CreateInfo.flags = {};
		CreateInfo.size = DeviceSize;
		CreateInfo.usage = Usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		CreateInfo.queueFamilyIndexCount = {};
		CreateInfo.pQueueFamilyIndices = {};
		CreateInfo.pNext = VK_NULL_HANDLE;

		AllocCreateInfo = { VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_CPU_ONLY };

		VK_CHECK(vmaCreateBuffer(Allocator, &CreateInfo, &AllocCreateInfo, &VulkanBuffer, &Allocation, nullptr));
		vmaGetAllocationInfo(Allocator, Allocation, &AllocationInfo);

		DebugName = "Staging Buffer: " + DebugName;
	}

	Buffer::Buffer(const Context& DeviceContext, VmaAllocator InAllocator, const VkBufferCreateInfo& CreateInfo, 
				   const VmaAllocationCreateInfo& AllocInfo, const std::string& Name)
		: VulkanDevice{DeviceContext.GetDevice()}, Allocator{InAllocator}, DeviceSize{CreateInfo.size}, UsageFlags{CreateInfo.usage}, 
		  AllocCreateInfo{AllocInfo}, DebugName{Name} 
	{
		VK_CHECK(vmaCreateBuffer(Allocator, &CreateInfo, &AllocCreateInfo, &VulkanBuffer, &Allocation, nullptr));
		vmaGetAllocationInfo(Allocator, Allocation, &AllocationInfo);

		DebugName = "Buffer: " + DebugName;
	}

	Buffer::~Buffer()
	{
		if(MappedMemory)
		{
			vmaUnmapMemory(Allocator, Allocation);
		}

		for(auto& BufferView : BufferViews)
		{
			vkDestroyBufferView(VulkanDevice, BufferView.second, nullptr);
		}

		vmaDestroyBuffer(Allocator, VulkanBuffer, Allocation);
	}

	void Buffer::Upload(VkDeviceSize Offset) const
	{
		Upload(Offset, DeviceSize);
	}

	void Buffer::Upload(VkDeviceSize Offset, VkDeviceSize Size) const
	{
		VK_CHECK(vmaFlushAllocation(Allocator, Allocation, Offset, Size));
	}

	void Buffer::UploadStagingBuffer(const VkCommandBuffer CmdBuffer, uint64_t SrcOffset, uint64_t DstOffset)
	{
		VkBufferCopy CopyRegion{};
		CopyRegion.srcOffset = SrcOffset;
		CopyRegion.dstOffset = DstOffset;
		CopyRegion.size = DeviceSize;

		ASSERT(StagingBuffer != nullptr, "Staging buffer cannot be null when uploading to GPU!");
		vkCmdCopyBuffer(CmdBuffer, VulkanBuffer, StagingBuffer->GetVkBuffer(), 1, &CopyRegion);
	}

	void Buffer::CopyToBuffer(const void* Data, size_t Size)
	{
		if(!MappedMemory)
		{
			VK_CHECK(vmaMapMemory(Allocator, Allocation, &MappedMemory));
		}
		memcpy(MappedMemory, Data, Size);
	}

	VkDeviceAddress Buffer::GetDeviceAddress()
	{
		if (StagingBuffer)
		{
			return StagingBuffer->GetDeviceAddress();
		}

#if defined(VK_KHR_buffer_device_address)
		if(!BufferDeviceAddress)
		{
			VkBufferDeviceAddressInfo AddressInfo{};
			AddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			AddressInfo.buffer = VulkanBuffer;
			AddressInfo.pNext = VK_NULL_HANDLE;

			BufferDeviceAddress = vkGetBufferDeviceAddress(VulkanDevice, &AddressInfo);
		}

		return BufferDeviceAddress;
#else
		return 0;
#endif
	}

	VkBufferView Buffer::RequestBufferView(VkFormat ViewFormat)
	{
		auto Itr = BufferViews.find(ViewFormat);
		if(Itr != BufferViews.end())
		{
			return Itr->second;
		}

		VkBufferViewCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		CreateInfo.flags = 0;
		CreateInfo.buffer = VulkanBuffer;
		CreateInfo.format = ViewFormat;
		CreateInfo.offset = 0;
		CreateInfo.range = DeviceSize;
		CreateInfo.pNext = VK_NULL_HANDLE;

		VkBufferView BufferView;
		VK_CHECK(vkCreateBufferView(VulkanDevice, &CreateInfo, nullptr, &BufferView));
		BufferViews[ViewFormat] = BufferView;

		return BufferView;
	}

}