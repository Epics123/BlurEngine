#include "Sampler.h"
#include "Context.h"

namespace VulkanCore
{

	Sampler::Sampler(const Context& DeviceContext, const SamplerCreateInfo& SamplerInfo)
		: VulkanDevice{DeviceContext.GetDevice()}
	{
		VkSamplerCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		CreateInfo.minFilter = SamplerInfo.MinFilter;
		CreateInfo.magFilter = SamplerInfo.MagFilter;
		CreateInfo.mipmapMode = SamplerInfo.MaxLod > 0 ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
		CreateInfo.addressModeU = SamplerInfo.AddressModeU;
		CreateInfo.addressModeV = SamplerInfo.AddressModeV;
		CreateInfo.addressModeW = SamplerInfo.AddressModeW;
		CreateInfo.mipLodBias = 0;
		CreateInfo.anisotropyEnable = VK_FALSE;
		CreateInfo.compareEnable = SamplerInfo.bEnableCompare;
		CreateInfo.compareOp = SamplerInfo.bEnableCompare ? SamplerInfo.CompareOp : VK_COMPARE_OP_NEVER;
		CreateInfo.minLod = 0;
		CreateInfo.maxLod = SamplerInfo.MaxLod;

		VK_CHECK(vkCreateSampler(VulkanDevice, &CreateInfo, nullptr, &VulkanSampler));
	}

}