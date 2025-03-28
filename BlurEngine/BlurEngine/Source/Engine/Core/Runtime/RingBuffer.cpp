#include "RingBuffer.h"

namespace EngineCore
{

	RingBuffer::RingBuffer(const VulkanCore::Context& InContext, uint32_t InRingSize, size_t BufferSize, const std::string& Name)
		:RingSize(InRingSize), BufferSize(BufferSize), RingIndex(0)
	{
		for(uint32_t i = 0; i < RingSize; i ++)
		{
			VkBufferUsageFlags Flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
#if VK_KHR_buffer_device_address && _WIN32
			Flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
#endif
			std::shared_ptr<VulkanCore::Buffer> NewBuffer = InContext.CreatePersistentBuffer(BufferSize, Flags, Name + " " + std::to_string(i));
			Buffers.emplace_back(NewBuffer);
		}
	}

	void RingBuffer::ToNextBuffer()
	{
		RingIndex++;
		if(RingIndex >= RingSize)
		{
			RingIndex = 0;
		}
	}

}