#pragma once

#ifdef _WIN32
#include <vulkan/vk_enum_string_helper.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <source_location>

#include "../Core/Logger.h"

#define VK_CHECK(func)                                                                 \
  {                                                                                    \
    const VkResult result = func;                                                      \
    if (result != VK_SUCCESS)                                                          \
    {                                                                                  \
      const std::source_location location = std::source_location::current();           \
      std::string ErrorMsg = "Error calling function ";                                \
      ErrorMsg.append(#func);                                                          \
      ErrorMsg.append(" at");                                                          \
      ErrorMsg.append(location.file_name());                                           \
      ErrorMsg.append(":");                                                            \
      ErrorMsg.append(std::to_string(location.line()));                                \
      ErrorMsg.append(". Result is ");                                                 \
      ErrorMsg.append(string_VkResult(result));                                        \
      BE_CRITICAL(ErrorMsg);                                                           \
    }                                                                                  \
  }                                                                                    \

namespace VulkanCore
{
	VkImageViewType ImageTypeToImageViewType(VkImageType ImageType, VkImageCreateFlags Flags, bool Multiview);

	uint32_t BytesPerPixel(VkFormat Format);
}