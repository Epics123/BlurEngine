#pragma once

#include "VulkanCommon.h"

#include <optional>

namespace VulkanCore
{
	typedef std::pair<uint32_t, uint32_t> QueueFamilyPair;

	class PhysicalDevice
	{
	public:
		PhysicalDevice() {};
		PhysicalDevice(VkPhysicalDevice Device, VkSurfaceKHR Surface, bool EnableRayTracing = false);

		// Reserve queues for rendering. Surface may be VK_NULL_HANDLE, as we may be rendering offscreen
		void ReserveQueues(VkQueueFlags RequestedQueueTypes, VkSurfaceKHR Surface);

		VkPhysicalDevice GetVkPhysicalDevice() const { return VulkanPhysicalDevice; }
		const VkSurfaceCapabilitiesKHR GetSurfaceCapabilities() const { return SurfaceCapabilities; }

		bool IsDeviceValid() const { return VulkanPhysicalDevice != VK_NULL_HANDLE; }

		bool IsRayTracingSupported() const;
		bool IsMultiviewSupported() const { return MultiviewFeature.multiview; }
		bool IsFragmentDensityMapSupported() const { return FragmentDensityMapFeature.fragmentDensityMap == VK_TRUE; }

		std::optional<uint32_t> GetComputeFamilyIndex() const { return ComputeFamilyIndex; }
		std::optional<uint32_t> GetGraphicsFamilyIndex() const { return GraphicsFamilyIndex; }
		std::optional<uint32_t> GetTransferFamilyIndex() const { return TransferFamilyIndex; }
		std::optional<uint32_t> GetPresentationFamilyIndex() const { return PresentationFamilyIndex; }

		uint32_t GraphicsFamilyCount() const { return GraphicsQueueCount; }
		uint32_t ComputeFamilyCount() const { return ComputeQueueCount; }
		uint32_t TransferFamilyCount() const { return TransferQueueCount; }
		uint32_t PresentationFamilyCount() const { return PresentationQueueCount; }

		const VkPhysicalDeviceProperties GetDeviceProperties() const { return Properties.properties; }
		const VkPhysicalDeviceProperties2 GetDeviceProperties2() const { return Properties; }

		std::vector<QueueFamilyPair> FindQueueFamilies() const;

	private:
		void InitFeatures();
		void InitProperties();

		void EnumerateSurfaceFormats(VkSurfaceKHR Surface);
		void EnumerateSurfaceCapabilities(VkSurfaceKHR Surface);
		void EnumeratePresentationModes(VkSurfaceKHR Surface);

	private:
		VkPhysicalDevice VulkanPhysicalDevice = VK_NULL_HANDLE;

		// Properties
		VkPhysicalDeviceFragmentDensityMapPropertiesEXT FragmentDensityMapProperties;
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR RayTracingPipelineProperties;
		VkPhysicalDeviceProperties2 Properties;

		// Features
		VkPhysicalDeviceFragmentDensityMapFeaturesEXT FragmentDensityMapFeature;
		VkPhysicalDeviceMultiviewFeatures MultiviewFeature;
		VkPhysicalDeviceTimelineSemaphoreFeatures TimelineSemaphoreFeature;
		VkPhysicalDeviceMeshShaderFeaturesNV MeshShaderFeature;
		VkPhysicalDeviceRayQueryFeaturesKHR RayQueryFeature;
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR RayTracingFeature;
		VkPhysicalDeviceAccelerationStructureFeaturesKHR AccelStructFeature;
		VkPhysicalDeviceDescriptorIndexingFeatures DescIndexFeature;
		VkPhysicalDeviceBufferDeviceAddressFeatures BufferDeviceAddressFeatures;
		VkPhysicalDeviceVulkan12Features Features12;
		VkPhysicalDeviceFeatures2 Features;

		// Memory properties
		VkPhysicalDeviceMemoryProperties2 MemoryProperties;

		std::vector<VkSurfaceFormatKHR> SurfaceFormats;
		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		std::vector<VkPresentModeKHR> PresentModes;

		std::vector<VkQueueFamilyProperties> QueueFamilyProperties;

		std::optional<uint32_t> GraphicsFamilyIndex;
		uint32_t GraphicsQueueCount = 0;
		std::optional<uint32_t> ComputeFamilyIndex;
		uint32_t ComputeQueueCount = 0;
		std::optional<uint32_t> TransferFamilyIndex;
		uint32_t TransferQueueCount = 0;
		std::optional<uint32_t> PresentationFamilyIndex;
		uint32_t PresentationQueueCount = 0;
	};
}