#include "CommandQueueManager.h"
#include "Context.h"
#include "Logger.h"

namespace VulkanCore
{
	
	CommandQueueManager::CommandQueueManager(const Context& DeviceContext, uint32_t Count, uint32_t NumConcurrentCommands, uint32_t QueueFamilyIdx, VkQueue Queue, 
											 VkCommandPoolCreateFlags Flags, const std::string& Name)
		: VulkanDevice{DeviceContext.GetDevice()}, CommandsInFlight{NumConcurrentCommands}, QueueFamilyIndex{QueueFamilyIdx}, VulkanQueue{Queue}
	{
		Fences.reserve(CommandsInFlight);
		IsSubmittedQueue.reserve(CommandsInFlight);
		BuffersToDispose.resize(CommandsInFlight);
		Deallocators.resize(CommandsInFlight);

		VkCommandPoolCreateInfo CmdPoolInfo{};
		CmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CmdPoolInfo.flags = Flags;
		CmdPoolInfo.queueFamilyIndex = QueueFamilyIndex;
		CmdPoolInfo.pNext = VK_NULL_HANDLE;

		VK_CHECK(vkCreateCommandPool(VulkanDevice, &CmdPoolInfo, nullptr, &CommandPool));

		VkCommandBufferAllocateInfo CmdBufferInfo{};
		CmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CmdBufferInfo.commandPool = CommandPool;
		CmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CmdBufferInfo.commandBufferCount = 1;
		CmdBufferInfo.pNext = VK_NULL_HANDLE;

		for(uint32_t Index = 0; Index < Count; Index++)
		{
			VkCommandBuffer CmdBuffer;
			VK_CHECK(vkAllocateCommandBuffers(VulkanDevice, &CmdBufferInfo, &CmdBuffer));
			CommandBuffers.push_back(CmdBuffer);
		}

		for(uint32_t Index = 0; Index < CommandsInFlight; Index++)
		{
			VkFence Fence;

			VkFenceCreateInfo FenceInfo{};
			FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			FenceInfo.pNext = VK_NULL_HANDLE;

			VK_CHECK(vkCreateFence(VulkanDevice, &FenceInfo, nullptr, &Fence));

			Fences.push_back(std::move(Fence));
			IsSubmittedQueue.push_back(false);
		}

		DebugName = "Command Queue Manager: " + Name;
	}

	CommandQueueManager::~CommandQueueManager()
	{
		for(uint32_t Index = 0; Index < CommandsInFlight; Index++)
		{
			vkDestroyFence(VulkanDevice, Fences[Index], nullptr);
		}

		for(size_t Index = 0; Index < CommandBuffers.size(); Index++)
		{
			vkFreeCommandBuffers(VulkanDevice, CommandPool, 1, &CommandBuffers[Index]);
		}

		vkDestroyCommandPool(VulkanDevice, CommandPool, nullptr);
	}

	void CommandQueueManager::Submit(const VkSubmitInfo* SubmitInfo)
	{
		VK_CHECK(vkResetFences(VulkanDevice, 1, &Fences[CurrentFenceIndex]));
		VK_CHECK(vkQueueSubmit(VulkanQueue, 1, SubmitInfo, Fences[CurrentFenceIndex]));
		IsSubmittedQueue[CurrentFenceIndex] = true;
	}

	void CommandQueueManager::ToNextCmdBuffer()
	{
		CurrentCommandBufferIndex = (CurrentCommandBufferIndex + 1) % static_cast<uint32_t>(CommandBuffers.size());
		CurrentFenceIndex = (CurrentFenceIndex + 1) % CommandsInFlight;
	}

	void CommandQueueManager::WaitForSubmit()
	{
		if(!IsSubmittedQueue[CurrentFenceIndex])
		{
			return;
		}

		const VkResult Result = vkWaitForFences(VulkanDevice, 1, &Fences[CurrentFenceIndex], true, UINT32_MAX);
		if(Result == VK_TIMEOUT)
		{
			BE_ERROR("Wait for fences timeout!");
			vkDeviceWaitIdle(VulkanDevice);
		}

		IsSubmittedQueue[CurrentFenceIndex] = false;
		BuffersToDispose[CurrentFenceIndex].clear();
		
		DeallocateResources();
	}

	void CommandQueueManager::WaitForAllSubmissions()
	{
		uint32_t Index = 0;
		for(VkFence Fence : Fences)
		{
			VK_CHECK(vkWaitForFences(VulkanDevice, 1, &Fence, true, UINT32_MAX));
			VK_CHECK(vkResetFences(VulkanDevice, 1, &Fence));
			IsSubmittedQueue[Index++] = false;
		}

		BuffersToDispose.clear();
		DeallocateResources();
	}

	void CommandQueueManager::DisposeOnSubmitCompletion(std::shared_ptr<Buffer> DisposeBuffer)
	{
		BuffersToDispose[CurrentFenceIndex].push_back(std::move(DisposeBuffer));
	}

	void CommandQueueManager::DisposeOnSubmitCompletion(std::function<void()>&& Deallocator)
	{
		Deallocators[CurrentFenceIndex].push_back(std::move(Deallocator));
	}

	VkCommandBuffer CommandQueueManager::BeginCmdBuffer()
	{
		VkFence* Fence = &Fences[CurrentFenceIndex];

		VK_CHECK(vkWaitForFences(VulkanDevice, 1, Fence, true, UINT32_MAX));
		VK_CHECK(vkResetCommandBuffer(CommandBuffers[CurrentCommandBufferIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

		VkCommandBufferBeginInfo BeginInfo{};
		BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		BeginInfo.pNext = VK_NULL_HANDLE;
		BeginInfo.pInheritanceInfo = VK_NULL_HANDLE;

		VK_CHECK(vkBeginCommandBuffer(CommandBuffers[CurrentCommandBufferIndex], &BeginInfo));

		return CommandBuffers[CurrentCommandBufferIndex];
	}

	void CommandQueueManager::EndCmdBuffer(VkCommandBuffer CmdBuffer)
	{
		VK_CHECK(vkEndCommandBuffer(CmdBuffer));
	}

	VkCommandBuffer CommandQueueManager::GetNewCmdBuffer()
	{
		VkCommandBufferAllocateInfo AllocInfo{};
		AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		AllocInfo.commandPool = CommandPool;
		AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		AllocInfo.commandBufferCount = 1;
		AllocInfo.pNext = VK_NULL_HANDLE;

		VkCommandBuffer CmdBuffer{VK_NULL_HANDLE};
		VK_CHECK(vkAllocateCommandBuffers(VulkanDevice, &AllocInfo, &CmdBuffer));

		return CmdBuffer;
	}

	void CommandQueueManager::DeallocateResources()
	{
		for(auto& DeallocatorList : Deallocators)
		{
			for(auto& Deallocator : DeallocatorList)
			{
				Deallocator();
			}
		}
	}

}