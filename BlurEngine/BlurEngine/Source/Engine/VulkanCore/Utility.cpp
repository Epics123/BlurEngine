#include "Utility.h"

namespace VulkanUtils
{
	
	VkImageViewType ImageTypeToImageViewType(VkImageType ImageType, VkImageCreateFlags Flags, bool Multiview)
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

	std::string PresentModeToString(const VkPresentModeKHR PresentMode)
	{
		std::string Result = "";
		switch (PresentMode)
		{
		case VK_PRESENT_MODE_FIFO_KHR:
			Result = "V-Sync";
			break;
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			Result = "V-Sync Catch Up";
			break;
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			Result = "Immediate";
			break;
		case VK_PRESENT_MODE_MAILBOX_KHR:
			Result = "V-Sync Triple Buffering";
			break;
		default:
			Result = "Unsupported Present Mode";
			break;
		}

		return Result;
	}
}