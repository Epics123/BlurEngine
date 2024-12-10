#pragma once

#include "Utility.h"
#include "VulkanCommon.h"
#include "PhysicalDevice.h"

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

class Context final
{
public:
	MOVABLE_ONLY(Context);

	Context(std::shared_ptr<Window> ContextWindow, VkQueueFlags RequestedQueueTypes = VK_QUEUE_GRAPHICS_BIT);
	~Context();

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