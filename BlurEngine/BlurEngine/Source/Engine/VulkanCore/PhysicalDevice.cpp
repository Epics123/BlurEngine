#include "PhysicalDevice.h"
#include "Utility.h"

#include <set>

PhysicalDevice::PhysicalDevice(VkPhysicalDevice Device, VkSurfaceKHR Surface, bool EnableRayTracing)
	:VulkanPhysicalDevice{Device}
{
	InitFeatures();
	InitProperties();

	MemoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
	vkGetPhysicalDeviceMemoryProperties2(VulkanPhysicalDevice, &MemoryProperties);

	{
		uint32_t PropertyCount = 0;
		vkEnumerateDeviceExtensionProperties(VulkanPhysicalDevice, nullptr, &PropertyCount, nullptr);

		std::vector<VkExtensionProperties> Properties(PropertyCount);
		vkEnumerateDeviceExtensionProperties(VulkanPhysicalDevice, nullptr, &PropertyCount, Properties.data());
	}

	{
		uint32_t QueueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(VulkanPhysicalDevice, &QueueFamilyCount, nullptr);
		QueueFamilyProperties.resize(QueueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(VulkanPhysicalDevice, &QueueFamilyCount, QueueFamilyProperties.data());
	}

	if(Surface != VK_NULL_HANDLE)
	{
		EnumerateSurfaceFormats(Surface);
		EnumerateSurfaceCapabilities(Surface);
		EnumeratePresentationModes(Surface);
	}
}

void PhysicalDevice::ReserveQueues(VkQueueFlags RequestedQueueTypes, VkSurfaceKHR Surface)
{
	/*Below is from The Modern Vulkan Cookbook:
	* - Only share queues with presentation, vulkan queues can support multiple
	*	operations (graphics, compute, transfer, etc).
	* - If supporting multiple operations, a queue can only be used on one thread
	* - The below code treats each queue as independent, and it can only be used for
	*	either graphics/compute/transfer or sparse. This should help w/ multithreading
	* - If the device only has one queue for everything, then we may nor be able to
	*	create compute/transfer queues
	*/

	ASSERT(RequestedQueueTypes > 0, "Requested queue types cannot be empty!");

	for (size_t QueueFamilyIndex = 0; QueueFamilyIndex < QueueFamilyProperties.size(); ++QueueFamilyIndex)
	{
		VkQueueFlags QueueFamilyPropertiesFlags = QueueFamilyProperties[QueueFamilyIndex].queueFlags;

		// Present queue
		if (!PresentationFamilyIndex.has_value() && Surface != VK_NULL_HANDLE)
		{
			VkBool32 bSupportsPresetQueue = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(VulkanPhysicalDevice, QueueFamilyIndex, Surface, &bSupportsPresetQueue);
			if (bSupportsPresetQueue == VK_TRUE)
			{
				PresentationFamilyIndex = QueueFamilyIndex;
				PresentationQueueCount = QueueFamilyProperties[QueueFamilyIndex].queueCount;
			}
		}

		// Graphics queue
		if (!GraphicsFamilyIndex.has_value() && (RequestedQueueTypes & QueueFamilyPropertiesFlags) & VK_QUEUE_GRAPHICS_BIT)
		{
			GraphicsFamilyIndex = QueueFamilyIndex;
			GraphicsQueueCount = QueueFamilyProperties[QueueFamilyIndex].queueCount;
			RequestedQueueTypes &= ~VK_QUEUE_GRAPHICS_BIT;
			continue;
		}

		// Compute queue
		if (!ComputeFamilyIndex.has_value() && (RequestedQueueTypes & QueueFamilyPropertiesFlags) & VK_QUEUE_COMPUTE_BIT)
		{
			ComputeFamilyIndex = QueueFamilyIndex;
			ComputeQueueCount = QueueFamilyProperties[QueueFamilyIndex].queueCount;
			RequestedQueueTypes &= ~VK_QUEUE_COMPUTE_BIT;
			continue;
		}

		// Transfer queue
		if (!TransferFamilyIndex.has_value() && (RequestedQueueTypes & QueueFamilyPropertiesFlags) & VK_QUEUE_TRANSFER_BIT)
		{
			TransferFamilyIndex = QueueFamilyIndex;
			TransferQueueCount = QueueFamilyProperties[QueueFamilyIndex].queueCount;
			RequestedQueueTypes &= ~VK_QUEUE_TRANSFER_BIT;
			continue;
		}
	}

	ASSERT(GraphicsFamilyIndex.has_value() || ComputeFamilyIndex.has_value() || TransferFamilyIndex.has_value(), "No suitable queue(s) found!");
	ASSERT(Surface == VK_NULL_HANDLE || PresentationFamilyIndex.has_value(), "No queues with presentation capabilities found!");
}

bool PhysicalDevice::IsRayTracingSupported() const
{
	return (AccelStructFeature.accelerationStructure && RayTracingFeature.rayTracingPipeline && RayQueryFeature.rayQuery);
}

std::vector<QueueFamilyPair> PhysicalDevice::FindQueueFamilies() const
{
	std::set<QueueFamilyPair> FamilyIndices;

	if(GraphicsFamilyIndex.has_value())
	{
		FamilyIndices.insert({ GraphicsFamilyIndex.value(), GraphicsQueueCount });
	}
	if (ComputeFamilyIndex.has_value())
	{
		FamilyIndices.insert({ ComputeFamilyIndex.value(), ComputeQueueCount });
	}
	if (TransferFamilyIndex.has_value())
	{
		FamilyIndices.insert({ TransferFamilyIndex.value(), TransferQueueCount });
	}
	if (PresentationFamilyIndex.has_value())
	{
		FamilyIndices.insert({ PresentationFamilyIndex.value(), PresentationQueueCount });
	}

	std::vector<QueueFamilyPair> QueueFamilies(FamilyIndices.begin(), FamilyIndices.end());
	return QueueFamilies;
}

void PhysicalDevice::InitFeatures()
{
	Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	Features.pNext = &Features12;

	Features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	Features12.pNext = (void*)&BufferDeviceAddressFeatures;

	BufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	BufferDeviceAddressFeatures.pNext = (void*)&DescIndexFeature;

	DescIndexFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
	DescIndexFeature.pNext = (void*)&AccelStructFeature;

	AccelStructFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	AccelStructFeature.pNext = (void*)&RayTracingFeature;

	RayTracingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	RayTracingFeature.pNext = (void*)&RayQueryFeature;

	RayQueryFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
	RayQueryFeature.pNext = (void*)&MeshShaderFeature;

	MeshShaderFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
	MeshShaderFeature.pNext = (void*)&TimelineSemaphoreFeature;

	TimelineSemaphoreFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
	TimelineSemaphoreFeature.pNext = &MultiviewFeature;

	MultiviewFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
	MultiviewFeature.pNext = &FragmentDensityMapFeature;

	FragmentDensityMapFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT;
	FragmentDensityMapFeature.pNext = nullptr;

	vkGetPhysicalDeviceFeatures2(VulkanPhysicalDevice, &Features);
}

void PhysicalDevice::InitProperties()
{
	Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	Properties.pNext = &RayTracingPipelineProperties;

	RayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	RayTracingPipelineProperties.pNext = &FragmentDensityMapProperties;

	FragmentDensityMapProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT;
	FragmentDensityMapProperties.pNext = nullptr;

	vkGetPhysicalDeviceProperties2(VulkanPhysicalDevice, &Properties);
}

void PhysicalDevice::EnumerateSurfaceFormats(VkSurfaceKHR Surface)
{
	uint32_t FormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanPhysicalDevice, Surface, &FormatCount, nullptr);
	SurfaceFormats.resize(FormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanPhysicalDevice, Surface, &FormatCount, SurfaceFormats.data());
}

void PhysicalDevice::EnumerateSurfaceCapabilities(VkSurfaceKHR Surface)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanPhysicalDevice, Surface, &SurfaceCapabilities);
}

void PhysicalDevice::EnumeratePresentationModes(VkSurfaceKHR Surface)
{
	uint32_t PresentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanPhysicalDevice, Surface, &PresentModeCount, nullptr);

	PresentModes.resize(PresentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanPhysicalDevice, Surface, &PresentModeCount, PresentModes.data());
}
