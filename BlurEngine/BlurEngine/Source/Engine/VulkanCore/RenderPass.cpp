#include "RenderPass.h"
#include "Context.h"
#include "Texture.h"

#include <optional>

namespace VulkanCore
{

	RenderPass::RenderPass(const Context& DeviceContect, const std::vector<RenderPassInitInfo>& InitInfos, 
						   const std::vector<std::shared_ptr<Texture>> ResolveAttachments, 
						   VkPipelineBindPoint BindPoint, const std::string& Name /*= ""*/)
		: VulkanDevice{DeviceContect.GetDevice()}
	{
		std::vector<VkAttachmentDescription> AttachmentDescriptors;
		std::vector<VkAttachmentReference> ColorAttachmentReferences;
		std::vector<VkAttachmentReference> ResolveAttachmentReferences;
		std::optional<VkAttachmentReference> DepthStencilAttachmentReference;

		for(uint32_t Index = 0; Index < (uint32_t) InitInfos.size(); Index++)
		{
			std::shared_ptr<Texture> AttachmentTexture = InitInfos[Index].AttachmentTexture;

			VkAttachmentDescription AttachmentDesc{};
			AttachmentDesc.format = AttachmentTexture->GetFormat();
			AttachmentDesc.samples = AttachmentTexture->GetSampleCount();
			AttachmentDesc.loadOp = InitInfos[Index].LoadOp;
			AttachmentDesc.storeOp = InitInfos[Index].StoreOp;
			AttachmentDesc.stencilLoadOp = AttachmentTexture->IsStencil() ? InitInfos[Index].LoadOp : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			AttachmentDesc.stencilStoreOp = AttachmentTexture->IsStencil() ? InitInfos[Index].StoreOp : VK_ATTACHMENT_STORE_OP_DONT_CARE;
			AttachmentDesc.initialLayout = AttachmentTexture->GetLayout();
			AttachmentDesc.finalLayout = InitInfos[Index].FinalLayout;

			AttachmentDescriptors.emplace_back(AttachmentDesc);

			if(AttachmentTexture->IsDepth() || AttachmentTexture->IsStencil())
			{
				DepthStencilAttachmentReference = VkAttachmentReference{};
				DepthStencilAttachmentReference->attachment = Index;
				DepthStencilAttachmentReference->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			else
			{
				VkAttachmentReference ColorAttachmentRef{};
				ColorAttachmentRef.attachment = Index;
				ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				ColorAttachmentReferences.emplace_back(ColorAttachmentRef);
			}
		}

		const uint32_t NumAttachments = (uint32_t)AttachmentDescriptors.size();
		for(uint32_t Index = 0; Index < (uint32_t)ResolveAttachments.size(); Index++)
		{
			VkAttachmentDescription AttachmentDesc{};
			AttachmentDesc.format = ResolveAttachments[Index]->GetFormat();
			AttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			AttachmentDesc.loadOp = InitInfos[Index + NumAttachments].LoadOp;
			AttachmentDesc.storeOp = InitInfos[Index + NumAttachments].StoreOp;
			AttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			AttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			AttachmentDesc.initialLayout = ResolveAttachments[Index]->GetLayout();
			AttachmentDesc.finalLayout = InitInfos[Index + NumAttachments].FinalLayout;

			VkAttachmentReference AttachmentRef{};
			AttachmentRef.attachment = static_cast<uint32_t>(AttachmentDescriptors.size() - 1);
			AttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			ResolveAttachmentReferences.emplace_back(AttachmentRef);
		}

		VkSubpassDescription SubpassDesc{};
		SubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		SubpassDesc.colorAttachmentCount = static_cast<uint32_t>(ColorAttachmentReferences.size());
		SubpassDesc.pColorAttachments = ColorAttachmentReferences.data();
		SubpassDesc.pResolveAttachments = ResolveAttachmentReferences.data();
		SubpassDesc.pDepthStencilAttachment = DepthStencilAttachmentReference.has_value() ? &DepthStencilAttachmentReference.value() : nullptr;

		std::array<VkSubpassDependency, 2> Dependencies;
		Dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		Dependencies[0].dstSubpass = 0;

		Dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		Dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
									   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		Dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		Dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
										VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
										VK_ACCESS_SHADER_READ_BIT;

		Dependencies[1].srcSubpass = 0;
		Dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;

		Dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
									   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		Dependencies[1].dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		Dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
										VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		Dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
										VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
										VK_ACCESS_SHADER_READ_BIT;

		Dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT; // Not sure if these are needed
		Dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		CreateInfo.attachmentCount = static_cast<uint32_t>(AttachmentDescriptors.size());
		CreateInfo.pAttachments = AttachmentDescriptors.data();
		CreateInfo.subpassCount = 1;
		CreateInfo.pSubpasses = &SubpassDesc;
		CreateInfo.dependencyCount = 2;
		CreateInfo.pDependencies = Dependencies.data(); //TODO: provide dependencies based on each pass

		VK_CHECK(vkCreateRenderPass(VulkanDevice, &CreateInfo, nullptr, &VulkanRenderPass));

		DebugName = "Render Pass: " + Name;
	}

	RenderPass::~RenderPass()
	{
		vkDestroyRenderPass(VulkanDevice, VulkanRenderPass, nullptr);
	}

}