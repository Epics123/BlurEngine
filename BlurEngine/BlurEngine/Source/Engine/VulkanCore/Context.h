#pragma once

#include "Utility.h"
#include "VulkanCommon.h"
#include "PhysicalDevice.h"
#include "ShaderModule.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "CommandQueueManager.h"
#include "Framebuffer.h"

#include "../Core/Window.h"

#include <vma/vk_mem_alloc.h>

#include <memory>
#include <any>

namespace VulkanCore
{

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

struct QueueFamilyIndices
{
	uint32_t GraphicsFamily;
	uint32_t PresentFamily;
	bool GraphicsFamilyHasValue = false;
	bool PresentFamilyHasValue = false;

	bool IsComplete() { return GraphicsFamilyHasValue && PresentFamilyHasValue; }
};

template <size_t ChainSize = 10>
class VulkanFeatureChain
{
public:
	VulkanFeatureChain() = default;
	MOVABLE_ONLY(VulkanFeatureChain);

	template<typename T>
	auto& PushBack(T NextVulkanChainStruct)
	{
		ASSERT(CurrentIndex < ChainSize, "Vulkan Feature Chain is full!");
		Data[CurrentIndex] = NextVulkanChainStruct;

		// TODO: Probably don't need to be casting here anymore
		auto& Next = std::any_cast<decltype(NextVulkanChainStruct)&>(Data[CurrentIndex]);

		Next.pNext = std::exchange(FirstNext, &Next);
		CurrentIndex++;

		return Next;
	}

	[[nodiscard]] void* FirsNextPtr() const { return FirstNext; }

private:
	std::array<std::any, ChainSize> Data;
	VkBaseInStructure* Root = nullptr;
	int CurrentIndex = 0;
	void* FirstNext = VK_NULL_HANDLE;
};

struct PhysicalDeviceFeatures
{
	PhysicalDeviceFeatures()
	{
		DeviceFeatures = {};
		DeviceFeatures.independentBlend = VK_TRUE;
		DeviceFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;
		DeviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

		Vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;

		Vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

		Vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

		AccelStructFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

		RayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,

		RayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;

		MultiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;

		FragmentDensityMapFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT;
		FragmentDensityMapOffsetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM;
	}

	VkPhysicalDeviceFeatures DeviceFeatures;
	VkPhysicalDeviceVulkan11Features Vulkan11Features{};
	VkPhysicalDeviceVulkan12Features Vulkan12Features{};
	VkPhysicalDeviceVulkan13Features Vulkan13Features{};

	// Ray tracing features
	VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelStructFeatures{};
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingPipelineFeatures{};
	VkPhysicalDeviceRayQueryFeaturesKHR RayQueryFeatures{};
	VkPhysicalDeviceMultiviewFeatures MultiviewFeatures{};
	VkPhysicalDeviceFragmentDensityMapFeaturesEXT FragmentDensityMapFeatures{};
	VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM FragmentDensityMapOffsetFeatures{};
};

class Swapchain;

class Context final
{
public:
	MOVABLE_ONLY(Context);

	Context(std::shared_ptr<Window> ContextWindow, VkQueueFlags RequestedQueueTypes = VK_QUEUE_GRAPHICS_BIT);
	~Context();

	VkDevice GetDevice() const { return Device; }

	inline VmaAllocator GetAllocator() const { return Allocator; }

	SwapChainSupportDetails GetSupportDetails();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats);
	VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes, VkPresentModeKHR DesiredPresentMode);
	VkExtent2D ChooseSwapchainExtents(const VkSurfaceCapabilitiesKHR& Capabilities, VkExtent2D WindowExtent);
	Swapchain* GetSwapchain() const { return SwapChain.get(); }

	void CreateSwapchain(VkFormat Format, VkSurfaceFormatKHR SurfaceFormat, VkPresentModeKHR PresentMode, const VkExtent2D& Extent);

	std::shared_ptr<ShaderModule> CreateShaderModule(const std::string& Filepath, VkShaderStageFlagBits Stages, const std::string& Name = "");
	std::shared_ptr<ShaderModule> CreateShaderModule(const std::string& Filepath, const std::string& EntryPoint, VkShaderStageFlagBits Stages, const std::string& Name = "");
	std::shared_ptr<ShaderModule> CreateShaderModule(const std::vector<char>& ShaderCode, const std::string& EntryPoint, VkShaderStageFlagBits Stages, const std::string& Name = "");

	std::shared_ptr<RenderPass> CreateRenderPass(const std::vector<RenderPassInitInfo>& InitInfos, VkPipelineBindPoint BindPoint, 
												 std::vector<std::shared_ptr<Texture>> ResolveAttachments = {}, const std::string& Name = "");

	std::shared_ptr<Pipeline> CreateGraphicsPipeline(const GraphicsPipelineDescriptor& Desc, VkRenderPass Pass, const std::string& Name = "");
	std::shared_ptr<Pipeline> CreateComputePipeline(const ComputePipelineDescriptor& Desc, const std::string& Name = "");

	std::unique_ptr<CommandQueueManager> CreateGraphicsCommandQueue(uint32_t Count, uint32_t NumConcurrentCommands, int GraphicsQueueIndex = -1, const std::string Name = "");
	CommandQueueManager CreateTransferCommandQueue(uint32_t Count, uint32_t NumConcurrentCommands, int TransferQueueIndex = -1, const std::string Name = "");

	std::unique_ptr<Framebuffer> CreateFramebuffer(VkRenderPass Pass, const FramebufferCreateInfo& CreateInfo);

	void RecreateSwapchain(const VkExtent2D& NewExtent);
	
	static void EndableDefaultFeatures();
	static void EnableIndirectRenderingFeature();
	static void EnableSyncronizationFeature();
	static void EnableBufferDeviceAddressFeature();

private:
	void CreateInstance();
	void CreateSurface();
	void CreateLogicalDevice();

	void ChoosePhysicalDevice();
	bool IsPhysicalDeviceSuitable(VkPhysicalDevice Device);
	bool CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice Device);

	void FindQueueFamilies(VkPhysicalDevice Device, QueueFamilyIndices& OutIndices);
	void QuerySwapChainSupport(VkPhysicalDevice Device, SwapChainSupportDetails& OutDetails);

	void ResizeQueues();

	std::vector<const char*> GetRequiredExtentions();
	void EnumerateGLFWExtensions();

	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& OutInfo);
	bool IsValidationLayersSupported();

	void CreateMemoryAllocatior();

public:
#ifdef _DEBUG
	const bool EnableValidationLayers = true;
#else
	const bool EnableValidationLayers = false;
#endif

private:
	static PhysicalDeviceFeatures sPhysicalDeviceFeatures;
	PhysicalDevice GPUDevice;

	VkInstance Instance;
	VkSurfaceKHR Surface;
	VkDevice Device;

	VkApplicationInfo ApplicationInfo;

	VmaAllocator Allocator;

	VkQueueFlags RequestedQueues;

	std::unique_ptr<Swapchain> SwapChain;
	SwapChainSupportDetails SupportDetails;

	// Main queue
	VkQueue PresentQueue;

	// Optional queues
	std::vector<VkQueue> GraphicsQueues;
	std::vector<VkQueue> ComputeQueues;
	std::vector<VkQueue> TransferQueues;

	std::shared_ptr<class Window> ActiveWindow;

	bool bEnableValidationLayers = false;
	VkDebugUtilsMessengerEXT DebugMessenger;

	bool ShouldSupportRayTracing = false;
	VkPresentModeKHR DefaultPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
														VK_EXT_ATTACHMENT_FEEDBACK_LOOP_LAYOUT_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
														VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
														VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME };
	const std::vector<const char*> RequestedInstanceExtensions = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
};

}