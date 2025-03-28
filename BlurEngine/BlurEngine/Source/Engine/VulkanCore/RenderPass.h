#pragma once

#include "VulkanCommon.h"
#include "Utility.h"

namespace VulkanCore
{

class Context;
class Texture;

struct RenderPassInitInfo
{
	std::vector<std::shared_ptr<Texture>> AttachmentTextures;

	uint32_t ResolveAttachmentIndex;
	uint32_t DepthAttachmentIndex;
	uint32_t StencilAttachmentIndex = UINT32_MAX;

	std::vector<VkAttachmentLoadOp> LoadOps;
	std::vector<VkAttachmentStoreOp> StoreOps;

	std::vector<VkImageLayout> InitialLayouts;
	std::vector<VkImageLayout> FinalLayouts;
	VkFormat Format;
};

class RenderPass final
{
public:
	MOVABLE_ONLY(RenderPass);

	RenderPass(const Context& DeviceContect, const RenderPassInitInfo& InitInfo, 
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