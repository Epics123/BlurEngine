#include "Context.h"

#include <unordered_set>

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

Context::Context(std::shared_ptr<Window> ContextWindow, VkQueueFlags RequestedQueueTypes)
	:ActiveWindow{ContextWindow}
{
#ifdef _DEBUG
	bEnableValidationLayers = true;
#else
	bEnableValidationLayers = false;
#endif

	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
}

Context::~Context()
{
	if(Surface != VK_NULL_HANDLE)
	{
		vkDestroySurfaceKHR(Instance, Surface, nullptr);
	}

	if (bEnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
	}

	vkDestroyInstance(Instance, nullptr);
}

void Context::EndableDefaultFeatures()
{
	sPhysicalDeviceFeatures.Vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	sPhysicalDeviceFeatures.Vulkan12Features.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
	sPhysicalDeviceFeatures.Vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	sPhysicalDeviceFeatures.Vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
	sPhysicalDeviceFeatures.Vulkan12Features.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
	sPhysicalDeviceFeatures.Vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
	sPhysicalDeviceFeatures.Vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
	sPhysicalDeviceFeatures.Vulkan12Features.descriptorIndexing = VK_TRUE;
	sPhysicalDeviceFeatures.Vulkan12Features.runtimeDescriptorArray = VK_TRUE;
}

void Context::EnableIndirectRenderingFeature()
{
	sPhysicalDeviceFeatures.Vulkan11Features.shaderDrawParameters = VK_TRUE;
	sPhysicalDeviceFeatures.Vulkan12Features.drawIndirectCount = VK_TRUE;
	sPhysicalDeviceFeatures.DeviceFeatures.multiDrawIndirect = VK_TRUE;
	sPhysicalDeviceFeatures.DeviceFeatures.drawIndirectFirstInstance = VK_TRUE;
}

void Context::EnableSyncronizationFeature()
{
	sPhysicalDeviceFeatures.Vulkan13Features.synchronization2 = VK_TRUE;
}

void Context::EnableBufferDeviceAddressFeature()
{
	sPhysicalDeviceFeatures.Vulkan12Features.bufferDeviceAddress = VK_TRUE;
	sPhysicalDeviceFeatures.Vulkan12Features.bufferDeviceAddressCaptureReplay = VK_TRUE;
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

	EnumerateGLFWExtensions();
	std::vector<const char*> Extensions = GetRequiredExtentions();
	for(const char* Extension : RequestedInstanceExtensions)
	{
		Extensions.push_back(Extension);
	}

	CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
	CreateInfo.ppEnabledExtensionNames = Extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo;
	if (bEnableValidationLayers)
	{
		CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		CreateInfo.ppEnabledLayerNames = ValidationLayers.data();

		PopulateDebugMessengerCreateInfo(DebugCreateInfo);
		CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;
	}
	else
	{
		CreateInfo.enabledLayerCount = 0;
		CreateInfo.pNext = nullptr;
	}

	VK_CHECK(vkCreateInstance(&CreateInfo, nullptr, &Instance));
}

void Context::CreateSurface()
{
	if(ActiveWindow)
	{
		ActiveWindow->CreateWindowSurface(Instance, &Surface);
	}
	else
	{
		BE_CRITICAL("Trying to create a surface without a valid window!");
	}
}

std::vector<const char*> Context::GetRequiredExtentions()
{
	uint32_t GlfwExtensionCount = 0;
	const char** GlfwExtensions;
	GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwExtensionCount);

	std::vector<const char*> Extensions(GlfwExtensions, GlfwExtensions + GlfwExtensionCount);

	if (bEnableValidationLayers)
	{
		Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return Extensions;
}

void Context::EnumerateGLFWExtensions()
{
	uint32_t ExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);
	std::vector<VkExtensionProperties> Extensions(ExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, Extensions.data());

	std::unordered_set<std::string> AvailableExtensions;
	BE_INFO("Available Instance Extensions:")
	for (const auto& Extension : Extensions)
	{
		BE_INFO("\t{0}", Extension.extensionName)
		AvailableExtensions.insert(Extension.extensionName);
	}

	BE_WARN("Required Instance Extensions:")
	auto RequiredExtensions = GetRequiredExtentions();
	for (const auto& RequiredExtension : RequiredExtensions)
	{
		BE_WARN("\t{0}", RequiredExtension)
		if (AvailableExtensions.find(RequiredExtension) == AvailableExtensions.end())
		{
			const std::string ErrorMsg = "Missing required glfw extension: " + std::string(RequiredExtension);
			BE_CRITICAL(ErrorMsg);
		}
	}
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