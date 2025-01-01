#pragma once

#include "VulkanCommon.h"
#include "Utility.h"
#include "Buffer.h"

namespace VulkanCore
{
class Context;

class CommandQueueManager final
{
public:
	CommandQueueManager(){}
	explicit CommandQueueManager(const Context& DeviceContext, uint32_t Count, uint32_t NumConcurrentCommands, uint32_t QueueFamilyIdx,
								 VkQueue Queue, VkCommandPoolCreateFlags Flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, const std::string& Name = "");
	~CommandQueueManager();

	void Submit(const VkSubmitInfo* SubmitInfo);

	void ToNextCmdBuffer();

	void WaitForSubmit();
	void WaitForAllSubmissions();

	void DisposeOnSubmitCompletion(std::shared_ptr<Buffer> DisposeBuffer);
	void DisposeOnSubmitCompletion(std::function<void()>&& Deallocator);

	VkCommandBuffer BeginCmdBuffer();
	void EndCmdBuffer(VkCommandBuffer CmdBuffer);

	VkCommandBuffer GetNewCmdBuffer();

private:
	void DeallocateResources();

private:
	uint32_t CommandsInFlight = 2;
	uint32_t QueueFamilyIndex = 0;

	VkQueue VulkanQueue = VK_NULL_HANDLE;
	VkDevice VulkanDevice = VK_NULL_HANDLE;

	VkCommandPool CommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> CommandBuffers;

	std::vector<VkFence> Fences;
	std::vector<bool> IsSubmittedQueue;
	uint32_t CurrentFenceIndex = 0;
	uint32_t CurrentCommandBufferIndex = 0;

	// FenceIndex to list of buffers associated with that fence that need to be released
	std::vector<std::vector<std::shared_ptr<Buffer>>> BuffersToDispose;
	std::vector<std::vector<std::function<void()>>> Deallocators;

	std::string DebugName;
};

}