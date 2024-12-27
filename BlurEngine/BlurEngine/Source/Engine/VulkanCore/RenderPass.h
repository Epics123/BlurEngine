#pragma once

#include "VulkanCommon.h"
#include "Utility.h"

namespace VulkanCore
{

class Context;
class Texture;

struct RenderPassInitInfo
{
	std::shared_ptr<Texture> AttachmentTexture;

	uint32_t ResolveAttachmentIndex;
	uint32_t DepthAttachmentIndex;
	uint32_t StencilAttachmentIndex = UINT32_MAX;

	VkAttachmentLoadOp LoadOp;
	VkAttachmentStoreOp StoreOp;

	VkImageLayout InitialLayout;
	VkImageLayout FinalLayout;
	VkFormat Format;
};

class RenderPass final
{
public:
	MOVABLE_ONLY(RenderPass);

	RenderPass(const Context& DeviceContect, const std::vector<RenderPassInitInfo>& InitInfos, 
			   const std::vector<std::shared_ptr<Texture>> ResolveAttachments, VkPipelineBindPoint BindPoint, 
			   const std::string& Name = "");

	~RenderPass();

	VkRenderPass GetVkRenderPass() const { return VulkanRenderPass; }

private:
	VkDevice VulkanDevice = VK_NULL_HANDLE;
	VkRenderPass VulkanRenderPass = VK_NULL_HANDLE;

	std::string DebugName = "";
};
}