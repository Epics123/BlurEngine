#pragma once

#include <functional>

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
	  VkImageViewType VulkanUtils::ImageTypeToImageViewType(VkImageType ImageType, VkImageCreateFlags Flags, bool Multiview)
	  {
		  switch (ImageType)
		  {
		  case VK_IMAGE_TYPE_1D:
			  return Multiview ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
		  case VK_IMAGE_TYPE_2D:
		  {
			  if (Flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
			  {
				  return VK_IMAGE_VIEW_TYPE_CUBE;
			  }
			  return Multiview ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		  }
		  case VK_IMAGE_TYPE_3D:
			  return VK_IMAGE_VIEW_TYPE_3D;
		  case VK_IMAGE_TYPE_MAX_ENUM:
			  return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		  default:
			  break;
		  }

		  return VK_IMAGE_VIEW_TYPE_2D;
	  }
  }