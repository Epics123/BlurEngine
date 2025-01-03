#include "Context.h"
#include "Swapchain.h"
#include "Utility.h"

#include "../Core/Logger.h"

#include <unordered_set>
#include <set>

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

	ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ApplicationInfo.pApplicationName = ActiveWindow->GetName().c_str();
	ApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	ApplicationInfo.apiVersion = VK_API_VERSION_1_3;


	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	ChoosePhysicalDevice();
	CreateLogicalDevice();

	CreateMemoryAllocatior();
}

Context::~Context()
{
	vmaDestroyAllocator(Allocator);

	SwapChain.reset(); // Make sure swapchain is destroyed before destroying the VkDevice
	
	vkDestroyDevice(Device, nullptr);


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

SwapChainSupportDetails Context::GetSupportDetails()
{
	QuerySwapChainSupport(GPUDevice.GetVkPhysicalDevice(), SupportDetails);
	return SupportDetails;
}

VkSurfaceFormatKHR Context::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats)
{
	for(const VkSurfaceFormatKHR Format : AvailableFormats)
	{
		if(Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return Format;
		}
	}

	return AvailableFormats[0];
}

VkPresentModeKHR Context::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes, VkPresentModeKHR DesiredPresentMode)
{
	for(const VkPresentModeKHR PresentMode : AvailablePresentModes)
	{
		if(PresentMode == DesiredPresentMode)
		{
			BE_INFO("Present Mode: {0}", VulkanUtils::PresentModeToString(PresentMode));
			return PresentMode;
		}
	}

	BE_INFO("Desired present mode not available. Defaulting to: {0}", VulkanUtils::PresentModeToString(DefaultPresentMode));
	return DefaultPresentMode;
}

VkExtent2D Context::ChooseSwapchainExtents(const VkSurfaceCapabilitiesKHR& Capabilities, VkExtent2D WindowExtent)
{
	if(Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return Capabilities.currentExtent;
	}
	else
	{
		VkExtent2D ActualExtent = WindowExtent;
		ActualExtent.width = std::max(Capabilities.minImageExtent.width, std::min(Capabilities.maxImageExtent.width, ActualExtent.width));
		ActualExtent.height = std::max(Capabilities.minImageExtent.height, std::min(Capabilities.maxImageExtent.height, ActualExtent.height));

		return ActualExtent;
	}
}

void Context::CreateSwapchain(VkFormat Format, VkSurfaceFormatKHR SurfaceFormat, VkPresentModeKHR PresentMode, const VkExtent2D& Extent)
{
	ASSERT(Surface != VK_NULL_HANDLE, "Trying to create a swapchain without a surface! The context must be provided a valid surface to create a swapchain.")
	SwapChain = std::make_unique<Swapchain>(*this, GPUDevice, Surface, PresentQueue, SurfaceFormat, PresentMode, Extent);
}

std::shared_ptr<VulkanCore::ShaderModule> Context::CreateShaderModule(const std::string& Filepath, VkShaderStageFlagBits Stages, const std::string& Name /*= ""*/)
{
	return std::make_shared<ShaderModule>(*this, Filepath, Stages, Name);
}

std::shared_ptr<VulkanCore::ShaderModule> Context::CreateShaderModule(const std::string& Filepath, const std::string& EntryPoint, VkShaderStageFlagBits Stages, const std::string& Name /*= ""*/)
{
	return std::make_shared<ShaderModule>(*this, Filepath, EntryPoint, Stages, Name);
}

std::shared_ptr<VulkanCore::ShaderModule> Context::CreateShaderModule(const std::vector<char>& ShaderCode, const std::string& EntryPoint, VkShaderStageFlagBits Stages, const std::string& Name /*= ""*/)
{
	return std::make_shared<ShaderModule>(*this, ShaderCode, EntryPoint, Stages, Name);
}

std::shared_ptr<VulkanCore::RenderPass> Context::CreateRenderPass(const std::vector<RenderPassInitInfo>& InitInfos, VkPipelineBindPoint BindPoint, std::vector<std::shared_ptr<Texture>> ResolveAttachments /*= {}*/, const std::string& Name /*= ""*/)
{
	return std::make_shared<RenderPass>(*this, InitInfos, ResolveAttachments, BindPoint, Name);
}

std::shared_ptr<VulkanCore::Pipeline> Context::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& Desc, VkRenderPass Pass, const std::string& Name)
{
	return std::make_shared<Pipeline>(*this, Desc, Pass, Name);
}

std::shared_ptr<VulkanCore::Pipeline> Context::CreateComputePipeline(const ComputePipelineDescriptor& Desc, const std::string& Name)
{
	return std::make_shared<Pipeline>(*this, Desc, Name);
}

std::unique_ptr<VulkanCore::CommandQueueManager> Context::CreateGraphicsCommandQueue(uint32_t Count, uint32_t NumConcurrentCommands, int GraphicsQueueIndex, const std::string Name)
{
	if(GraphicsQueueIndex != -1)
	{
		ASSERT(GraphicsQueueIndex < GraphicsQueues.size(), "Not enough graphics queues available, specify a smaller queue index!");
	}

	const uint32_t FamilyIndex = GPUDevice.GetGraphicsFamilyIndex().value();
	VkQueue GraphicsQueue = GraphicsQueueIndex != -1 ? GraphicsQueues[GraphicsQueueIndex] : GraphicsQueues[0];

	return std::make_unique<CommandQueueManager>(*this, Count, NumConcurrentCommands, FamilyIndex, GraphicsQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, Name);
}

VulkanCore::CommandQueueManager Context::CreateTransferCommandQueue(uint32_t Count, uint32_t NumConcurrentCommands, int TransferQueueIndex, const std::string Name)
{
	if (TransferQueueIndex != -1)
	{
		ASSERT(TransferQueueIndex < TransferQueues.size(), "Not enough transfer queues available, specify a smaller queue index!");
	}

	const uint32_t FamilyIndex = GPUDevice.GetTransferFamilyIndex().value();
	VkQueue TransferQueue = TransferQueueIndex != -1 ? TransferQueues[TransferQueueIndex] : TransferQueues[0];

	return CommandQueueManager(*this, Count, NumConcurrentCommands, FamilyIndex, TransferQueue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, Name);
}

std::unique_ptr<VulkanCore::Framebuffer> Context::CreateFramebuffer(VkRenderPass Pass, const FramebufferCreateInfo& CreateInfo)
{
	return std::make_unique<Framebuffer>(*this, Pass, CreateInfo);
}

void Context::RecreateSwapchain(const VkExtent2D& NewExtent)
{
	ASSERT(Surface != VK_NULL_HANDLE, "Trying to recreate swapchain without a valid surface!");

	SwapChainSupportDetails SupportDetails = GetSupportDetails();

	VkExtent2D UpdatedExtent = ChooseSwapchainExtents(SupportDetails.Capabilities, NewExtent);

	std::shared_ptr<Swapchain> OldSwapchain = std::move(SwapChain);
	SwapChain = std::make_unique<Swapchain>(*this, GPUDevice, Surface, PresentQueue, UpdatedExtent, OldSwapchain);
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

void Context::CreateLogicalDevice()
{
	const auto FamilyIndices = GPUDevice.FindQueueFamilies();

	std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
	std::vector<std::vector<float>> QueueFamilyPriorities(FamilyIndices.size());

	size_t Index = 0;
	for(QueueFamilyPair QueueFamily : FamilyIndices)
	{
		const uint32_t QueueFamilyIndex = QueueFamily.first;
		const uint32_t QueueCount = QueueFamily.second;

		QueueFamilyPriorities[Index] = std::vector<float>(QueueCount, 1.0f);

		VkDeviceQueueCreateInfo QueueCreateInfo = {};
		QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueCreateInfo.queueFamilyIndex = QueueFamilyIndex;
		QueueCreateInfo.queueCount = QueueCount;
		QueueCreateInfo.pQueuePriorities = QueueFamilyPriorities[Index].data();
		QueueCreateInfos.emplace_back(QueueCreateInfo);

		++Index;
	}

	VkPhysicalDeviceFeatures2 DeviceFeatures{};
	DeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	DeviceFeatures.features = sPhysicalDeviceFeatures.DeviceFeatures;

	VulkanFeatureChain<> FeatureChain;
	FeatureChain.PushBack(DeviceFeatures);
	FeatureChain.PushBack(sPhysicalDeviceFeatures.Vulkan11Features);
	FeatureChain.PushBack(sPhysicalDeviceFeatures.Vulkan12Features);
#if _WIN32
	FeatureChain.PushBack(sPhysicalDeviceFeatures.Vulkan13Features);
#endif

	if(GPUDevice.IsRayTracingSupported() && ShouldSupportRayTracing)
	{
		FeatureChain.PushBack(sPhysicalDeviceFeatures.AccelStructFeatures);
		FeatureChain.PushBack(sPhysicalDeviceFeatures.RayTracingPipelineFeatures);
		FeatureChain.PushBack(sPhysicalDeviceFeatures.RayQueryFeatures);
	}

	if(GPUDevice.IsMultiviewSupported())
	{
		sPhysicalDeviceFeatures.Vulkan11Features.multiview = VK_TRUE;
	}

	if(GPUDevice.IsFragmentDensityMapSupported())
	{
		FeatureChain.PushBack(sPhysicalDeviceFeatures.FragmentDensityMapFeatures);
	}

	VkDeviceCreateInfo DeviceCreateInfo{};
	DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	DeviceCreateInfo.pNext = FeatureChain.FirsNextPtr();
	DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
	DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
	DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
	DeviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

	if(EnableValidationLayers)
	{
		DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		DeviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
	}

	VK_CHECK(vkCreateDevice(GPUDevice.GetVkPhysicalDevice(), &DeviceCreateInfo, nullptr, &Device));

	ResizeQueues();
}

void Context::ChoosePhysicalDevice()
{
	uint32_t DeviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);
	if(DeviceCount == 0)
	{
		BE_CRITICAL("Failed to find GPUs with Vulkan support!");
	}

	BE_INFO("Device Count: {0}", DeviceCount);

	std::vector<VkPhysicalDevice> Devices(DeviceCount);
	vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());

	for(const VkPhysicalDevice& PhysDevice : Devices)
	{
		if(IsPhysicalDeviceSuitable(PhysDevice))
		{
			GPUDevice = PhysicalDevice(PhysDevice, Surface);
			break;
		}
	}

	if(!GPUDevice.IsDeviceValid())
	{
		BE_CRITICAL("Failed to find a suitable GPU!");
	}

	BE_INFO("Selected GPU: {0}", GPUDevice.GetDeviceProperties().deviceName);

	GPUDevice.ReserveQueues(RequestedQueues | VK_QUEUE_GRAPHICS_BIT, Surface);
}

bool Context::IsPhysicalDeviceSuitable(VkPhysicalDevice Device)
{
	QueueFamilyIndices Indices;
	FindQueueFamilies(Device, Indices);

	bool ExtensionsSupported = CheckPhysicalDeviceExtensionSupport(Device);

	bool SwapChainAdequate = false;
	if(ExtensionsSupported)
	{
		SwapChainSupportDetails SwapChainSupport;
		QuerySwapChainSupport(Device, SwapChainSupport);

		SwapChainAdequate = !SwapChainSupport.Formats.empty() && !SwapChainSupport.PresentModes.empty();
	}

	VkPhysicalDeviceFeatures SupportedFeatures;
	vkGetPhysicalDeviceFeatures(Device, &SupportedFeatures);

	return Indices.IsComplete() && ExtensionsSupported && SwapChainAdequate && SupportedFeatures.samplerAnisotropy;
}

bool Context::CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice Device)
{
	uint32_t ExtensionCount;
	vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, nullptr);

	std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
	vkEnumerateDeviceExtensionProperties(Device, nullptr, &ExtensionCount, AvailableExtensions.data());

	std::set<std::string> RequiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	BE_INFO("Available Physical Device Extensions:");
	for(const VkExtensionProperties& Extension : AvailableExtensions)
	{
		BE_INFO("\t{0}", Extension.extensionName);
		RequiredExtensions.erase(Extension.extensionName);
	}

	return RequiredExtensions.empty();
}

void Context::FindQueueFamilies(VkPhysicalDevice Device, QueueFamilyIndices& OutIndices)
{
	uint32_t QueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilies.data());

	uint32_t i = 0;
	for(const auto& QueueFamily : QueueFamilies)
	{
		if(QueueFamily.queueCount > 0 && QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			OutIndices.GraphicsFamily = i;
			OutIndices.GraphicsFamilyHasValue = true;
		}

		VkBool32 PresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, Surface, &PresentSupport);
		if(QueueFamily.queueCount > 0 && PresentSupport)
		{
			OutIndices.PresentFamily = i;
			OutIndices.PresentFamilyHasValue = true;
		}

		if(OutIndices.IsComplete())
		{
			break;
		}

		i++;
	}
}

void Context::QuerySwapChainSupport(VkPhysicalDevice Device, SwapChainSupportDetails& OutDetails)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &OutDetails.Capabilities);

	uint32_t FormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, nullptr);
	if(FormatCount != 0)
	{
		OutDetails.Formats.resize(FormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, OutDetails.Formats.data());
	}

	uint32_t PresentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, nullptr);
	if(PresentModeCount != 0)
	{
		OutDetails.PresentModes.resize(PresentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, OutDetails.PresentModes.data());
	}
}

void Context::ResizeQueues()
{
	if (GPUDevice.GetGraphicsFamilyIndex().has_value() && GPUDevice.GraphicsFamilyCount() > 0)
	{
		GraphicsQueues.resize(GPUDevice.GraphicsFamilyCount(), VK_NULL_HANDLE);
		for (size_t i = 0; i < GraphicsQueues.size(); i++)
		{
			vkGetDeviceQueue(Device, GPUDevice.GetGraphicsFamilyIndex().value(), uint32_t(i), &GraphicsQueues[i]);
		}
	}
	if (GPUDevice.GetComputeFamilyIndex().has_value() && GPUDevice.ComputeFamilyCount() > 0)
	{
		ComputeQueues.resize(GPUDevice.ComputeFamilyCount(), VK_NULL_HANDLE);
		for (size_t i = 0; i < ComputeQueues.size(); i++)
		{
			vkGetDeviceQueue(Device, GPUDevice.GetComputeFamilyIndex().value(), uint32_t(i), &ComputeQueues[i]);
		}
	}
	if (GPUDevice.GetTransferFamilyIndex().has_value() && GPUDevice.TransferFamilyCount() > 0)
	{
		TransferQueues.resize(GPUDevice.TransferFamilyCount(), VK_NULL_HANDLE);
		for (size_t i = 0; i < TransferQueues.size(); i++)
		{
			vkGetDeviceQueue(Device, GPUDevice.GetTransferFamilyIndex().value(), uint32_t(i), &TransferQueues[i]);
		}
	}
	if (GPUDevice.GetPresentationFamilyIndex().has_value())
	{
		vkGetDeviceQueue(Device, GPUDevice.GetPresentationFamilyIndex().value(), 0, &PresentQueue);
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
	if(!bEnableValidationLayers || !IsValidationLayersSupported())
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
#if _DEBUG
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
#endif

	return false;
}

void Context::CreateMemoryAllocatior()
{
	VmaAllocatorCreateInfo AllocInfo{};
#if defined(VK_KHR_buffer_device_address) && defined(_WIN32)
	AllocInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
#endif
	AllocInfo.physicalDevice = GPUDevice.GetVkPhysicalDevice();
	AllocInfo.device = Device;
	AllocInfo.instance = Instance;
	AllocInfo.vulkanApiVersion = ApplicationInfo.apiVersion;

	VK_CHECK(vmaCreateAllocator(&AllocInfo, &Allocator));
}

}