#include "Framebuffer.h"

#include "Context.h"
#include "Texture.h"

namespace VulkanCore
{
	
	Framebuffer::Framebuffer(const Context& DeviceContext, VkRenderPass Pass, const FramebufferCreateInfo& FramebufferInfo)
		: VulkanDevice {DeviceContext.GetDevice()}
	{
		std::vector<VkImageView> ImageViews;

		for(const std::shared_ptr<Texture> Attachment : FramebufferInfo.Attachments)
		{
			ImageViews.push_back(Attachment->GetImageView(0));
		}

		if(FramebufferInfo.DepthAttachment)
		{
			ImageViews.push_back(FramebufferInfo.DepthAttachment->GetImageView(0));
		}

		if(FramebufferInfo.StencilAttachment)
		{
			ImageViews.push_back(FramebufferInfo.StencilAttachment->GetImageView(0));
		}

		ASSERT(!ImageViews.empty(), "Creating a framebuffer with no attachments is not supported!");

		const uint32_t Width = !FramebufferInfo.Attachments.empty() ? FramebufferInfo.Attachments[0]->GetExtents().width : 
																	  FramebufferInfo.DepthAttachment->GetExtents().width;
		const uint32_t Height = !FramebufferInfo.Attachments.empty() ? FramebufferInfo.Attachments[0]->GetExtents().height : 
																	   FramebufferInfo.DepthAttachment->GetExtents().height;

		VkFramebufferCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		CreateInfo.renderPass = Pass;
		CreateInfo.attachmentCount = static_cast<uint32_t>(ImageViews.size());
		CreateInfo.pAttachments = ImageViews.data();
		CreateInfo.width = Width;
		CreateInfo.height = Height;
		CreateInfo.layers = 1;
		CreateInfo.pNext = VK_NULL_HANDLE;

		VK_CHECK(vkCreateFramebuffer(VulkanDevice, &CreateInfo, nullptr, &VulkanFramebuffer));

		DebugName = "Framebuffer: " + FramebufferInfo.Name;
	}

	Framebuffer::~Framebuffer()
	{
		vkDestroyFramebuffer(VulkanDevice, VulkanFramebuffer, nullptr);
	}

}