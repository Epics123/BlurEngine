#include "Context.h"
#include "../Core/Window.h"

namespace VulkanCore
{

#pragma region DebugMessenger
	// local callback functions
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		BE_ERROR("Validaiton Layer: {0}", pCallbackData->pMessage)

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
#pragma endregion

PhysicalDeviceFeatures Context::sPhysicalDeviceFeatures = PhysicalDeviceFeatures();

Context::Context(class Window& window, VkQueueFlags RequestedQueueTypes)
{
#ifdef _DEBUG
	bEnableValidationLayers = true;
#else
	bEnableValidationLayers = false;
#endif
}

Context::~Context()
{

}

void Context::EndableDefaultFeatures()
{

}

void Context::EnableIndirectRenderingFeature()
{

}

void Context::EnableSyncronizationFeature()
{

}

void Context::EnableBufferDeviceAddressFeature()
{

}

void Context::CreateInstance()
{
	if(!bEnableValidationLayers && IsValidationLayersSupported())
	{
		BE_CRITICAL("Validation layers requested, but not available!");
	}

	VkApplicationInfo AppInfo = {};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pApplicationName = "VulkanRenderer";
	AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	AppInfo.pEngineName = "No Engine";
	AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	AppInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	CreateInfo.pApplicationInfo = &AppInfo;
}

void Context::SetupDebugMessenger()
{
	if(bEnableValidationLayers && !IsValidationLayersSupported())
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT CreateInfo;
	PopulateDebugMessengerCreateInfo(CreateInfo);

	VK_CHECK(CreateDebugUtilsMessengerEXT(Instance, &CreateInfo, nullptr, &DebugMessenger));
}

void Context::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& OutInfo)
{
	OutInfo = {};
	OutInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	OutInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	OutInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	OutInfo.pfnUserCallback = DebugCallback;
	OutInfo.pUserData = nullptr;  // Optional
}

bool Context::IsValidationLayersSupported()
{
	uint32_t LayerCount;
	vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

	std::vector<VkLayerProperties> AvailableLayers(LayerCount);
	vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

	for (const std::string LayerName : ValidationLayers)
	{
		bool bLayerFound = false;

		for (const auto& LayerProperties : AvailableLayers)
		{
			if (strcmp(LayerName.c_str(), LayerProperties.layerName) == 0)
			{
				bLayerFound = true;
				break;
			}
		}

		if (!bLayerFound)
		{
			return false;
		}
	}

	return true;
}

}