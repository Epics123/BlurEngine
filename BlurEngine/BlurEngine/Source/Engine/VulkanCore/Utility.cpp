#include "Utility.h"

#include <fstream>
#include <iostream>

namespace Util
{

	std::vector<char> ReadFile(const std::string& Filepath, bool IsBinary)
	{
		std::ios_base::openmode Mode = std::ios::ate;
		if (IsBinary)
		{
			Mode |= std::ios::binary;
		}

		std::ifstream File(Filepath, Mode);

		size_t FileSize = (size_t)File.tellg();
		if (!IsBinary)
		{
			FileSize++; // Add extra for null char at end
		}

		std::vector<char> Buffer(FileSize);
		File.seekg(0);
		File.read(reinterpret_cast<char*>(Buffer.data()), FileSize);
		File.close();

		if (!IsBinary)
		{
			Buffer[Buffer.size() - 1] = '\0';
		}

		return Buffer;
	}

}

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