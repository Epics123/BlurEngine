#pragma once

#include "VulkanCommon.h"
#include "Utility.h"

namespace VulkanCore
{

class Context;

struct SamplerCreateInfo
{
	VkFilter MinFilter;
	VkFilter MagFilter;

	VkSamplerAddressMode AddressModeU;
	VkSamplerAddressMode AddressModeV;
	VkSamplerAddressMode AddressModeW;

	float MaxLod;
	bool bEnableCompare = false;
	VkCompareOp CompareOp;

	std::string Name;
};

class Sampler final
{
public:
	MOVABLE_ONLY(Sampler);

	explicit Sampler(const Context& DeviceContext, const SamplerCreateInfo& SamplerInfo);

	~Sampler()
	{
		vkDestroySampler(VulkanDevice, VulkanSampler, nullptr);
	}

	VkSampler GetVkSampler() const { return VulkanSampler; }

private:
	VkDevice VulkanDevice = VK_NULL_HANDLE;
	VkSampler VulkanSampler = VK_NULL_HANDLE;
};

}