#pragma once

#include "Utility.h"
#include "VulkanCommon.h"

#include "../Core/Window.h"

#include <vma/vk_mem_alloc.h>

namespace VulkanCore
{

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

	std::vector<const char*> GetRequiredExtentions();
	void EnumerateGLFWExtensions();

	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& OutInfo);
	bool IsValidationLayersSupported();

private:
	static PhysicalDeviceFeatures sPhysicalDeviceFeatures;

	VkInstance Instance;
	VkSurfaceKHR Surface;

	VmaAllocator Allocator;

	std::shared_ptr<class Window> ActiveWindow;

	bool bEnableValidationLayers = false;
	VkDebugUtilsMessengerEXT DebugMessenger;

	const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> RequestedInstanceExtensions = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
};

}