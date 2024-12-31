#pragma once

#include "VulkanCommon.h"
#include "Utility.h"

namespace VulkanCore
{

class Context;
class Texture;

struct FramebufferCreateInfo
{
	std::vector<std::shared_ptr<Texture>> Attachments;
	std::shared_ptr<Texture> DepthAttachment;
	std::shared_ptr<Texture> StencilAttachment;
	std::string Name = "";
};

class Framebuffer final
{
public:
	MOVABLE_ONLY(Framebuffer);

	explicit Framebuffer(const Context& DeviceContext, VkRenderPass Pass, const FramebufferCreateInfo& FramebufferInfo);

	~Framebuffer();

	VkFramebuffer GetVkFramebuffer() const { return VulkanFramebuffer; }

private:
	VkDevice VulkanDevice;
	VkFramebuffer VulkanFramebuffer;

	std::string DebugName;
};

}