#pragma once

#ifdef _WIN32
#if !defined(VK_USE_PLATFORM_WIN32_KHR)
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#endif

#ifdef _WIN32
#include <vulkan/vk_enum_string_helper.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "../Core/Logger.h"

#define VK_CHECK(func)                                                                 \
  {                                                                                    \
    const VkResult result = func;                                                      \
    if (result != VK_SUCCESS)                                                          \
    {                                                                                  \
      const std::string ErrorMsg = "Error calling function " + #func + " at "          \
      + __FILE__ + ":" + __LINE__ + ". Result is " + string_VkResult(result);          \
      BE_CRITICAL(ErrorMsg);                                                           \
    }                                                                                  \
  }                                                                                    \

namespace VulkanCore
{
	VkImageViewType ImageTypeToImageViewType(VkImageType ImageType, VkImageCreateFlags Flags, bool Multiview);

	uint32_t BytesPerPixel(VkFormat Format);
}