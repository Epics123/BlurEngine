#pragma once

#include <vulkan/vulkan.h>

#include <functional>
#include <string>

#define ASSERT(expr, message) \
  {                           \
    void(message);            \
    assert(expr);             \
  }

#define MOVABLE_ONLY(CLASS_NAME)                     \
  CLASS_NAME(const CLASS_NAME&) = delete;            \
  CLASS_NAME& operator=(const CLASS_NAME&) = delete; \
  CLASS_NAME(CLASS_NAME&&) noexcept = default;       \
  CLASS_NAME& operator=(CLASS_NAME&&) noexcept = default;

  namespace Util
  {
	  // from: https://stackoverflow.com/a/57595105
	  template <typename T, typename... Rest>
	  void HashCombine(std::size_t& seed, const T& v, const Rest&... rest)
	  {
		  seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		  (HashCombine(seed, rest), ...);
	  };
  }

  namespace VulkanUtils
  {
	  VkImageViewType ImageTypeToImageViewType(VkImageType ImageType, VkImageCreateFlags Flags, bool Multiview);

	  std::string PresentModeToString(const VkPresentModeKHR PresentMode);
  }