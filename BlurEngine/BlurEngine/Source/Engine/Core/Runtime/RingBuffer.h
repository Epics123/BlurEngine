#pragma once

#include "../VulkanCore/VulkanCommon.h"
#include "../VulkanCore/Buffer.h"
#include "../VulkanCore/Context.h"

namespace EngineCore
{

class RingBuffer
{
public:
	RingBuffer(const VulkanCore::Context& InContext, uint32_t InRingSize, size_t BufferSize, const std::string& Name = "");

	void ToNextBuffer();

	const VulkanCore::Buffer* GetCurrentBuffer() const
	{
		return Buffers[RingIndex].get();
	}

	const std::shared_ptr<VulkanCore::Buffer> GetBuffer(uint32_t Index) const
	{
		ASSERT(Index < Buffers.size(), "Provided index is outside of ring buffer range!");
		return Buffers[Index];
	}

	uint32_t GetRingSize() const { return RingSize; }

private:
	uint32_t RingSize;
	uint32_t RingIndex;
	size_t BufferSize;

	std::vector<std::shared_ptr<VulkanCore::Buffer>> Buffers;

	std::string DebugName;
};

}