#pragma once

#include "VulkanCommon.h"
#include "Utility.h"

#include <unordered_map>
#include <list>

namespace VulkanCore
{

class Context;
class ShaderModule;

#pragma region PipelineDataStructures

struct SetDescriptor
{
	uint32_t SetIndex;
	std::vector<VkDescriptorSetLayoutBinding> Bindings;
};

struct DescriptorSet
{
	std::vector<VkDescriptorSet> Sets;
	VkDescriptorSetLayout Layout = VK_NULL_HANDLE;
};

class PipelineViewport
{
public:
	PipelineViewport() = default;
	PipelineViewport(const PipelineViewport&) = default;
	PipelineViewport& operator=(const PipelineViewport&) = default;

	PipelineViewport(const VkViewport& InViewport) : Viewport(InViewport) {};

	PipelineViewport(const VkExtent2D& Extents)
	{
		Viewport = FromExtents(Extents);
	}

	PipelineViewport& operator=(const VkViewport& InViewport)
	{
		Viewport = InViewport;
		return *this;
	}

	PipelineViewport& operator=(const VkExtent2D& Extents)
	{
		Viewport = FromExtents(Extents);
		return *this;
	}

	VkExtent2D ToExtent2D()
	{
		return VkExtent2D{ static_cast<uint32_t>(std::abs(Viewport.width), static_cast<uint32_t>(std::abs(Viewport.height))) };
	}

	VkViewport ToVkViewport() { return Viewport; }

private:
	
	VkViewport FromExtents(const VkExtent2D& Extents)
	{
		VkViewport NewViewport;
		NewViewport.x = 0;
		NewViewport.y = 0;
		NewViewport.width = (float)Extents.width;
		NewViewport.height = (float)Extents.height;
		NewViewport.minDepth = 0.0f;
		NewViewport.maxDepth = 1.0f;

		return NewViewport;
	}

	VkViewport Viewport = {};
};

#pragma endregion

#pragma region PipelineDescriptors

struct GraphicsPipelineDescriptor
{
	GraphicsPipelineDescriptor()
	{
		VertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		VertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
		VertexInputCreateInfo.pVertexBindingDescriptions = VK_NULL_HANDLE;
		VertexInputCreateInfo.pVertexAttributeDescriptions = VK_NULL_HANDLE;
		VertexInputCreateInfo.flags = 0;
		VertexInputCreateInfo.pNext = VK_NULL_HANDLE;
	}

	std::vector<SetDescriptor> SetDescriptors;

	std::weak_ptr<ShaderModule> VertexShader;
	std::weak_ptr<ShaderModule> FragmentShader;

	std::vector<VkPushConstantRange> PushConstants;
	std::vector<VkDynamicState> DynamicStates;

	std::vector<VkFormat> ColorTextureFormats;
	VkFormat DepthTextureFormat = VK_FORMAT_UNDEFINED;
	VkFormat StencilTextureFormat = VK_FORMAT_UNDEFINED;

	bool bUseDynamicRendering = false;
	bool bDepthTestEnable = true;
	bool bDepthWriteEnable = true;

	VkCompareOp DepthCompareOperation = VK_COMPARE_OP_LESS;

	bool bBlendEnable = false;
	uint32_t NumBlendAttachments = 0;

	VkPrimitiveTopology PrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	VkSampleCountFlagBits SampleCount = VK_SAMPLE_COUNT_1_BIT;
	VkCullModeFlagBits CullMode = VK_CULL_MODE_BACK_BIT;
	VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	PipelineViewport Viewport;

	VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo;

	std::vector<VkSpecializationMapEntry> VertexSpecConstants;
	std::vector<VkSpecializationMapEntry> FragmentSpecConstants;

	void* VertexSpecializationData = nullptr;
	void* FragmentSpecializationData = nullptr;

	std::vector<VkPipelineColorBlendAttachmentState> BlendAttachmentStates;
};

struct ComputePipelineDescriptor
{
	std::vector<SetDescriptor> SetDescriptors;
	std::weak_ptr<ShaderModule> ComputeShader;
	std::vector<VkPushConstantRange> PushConstants;
	std::vector<VkSpecializationMapEntry> SpecializationConstants;
	void* SpecializationData = nullptr;
};

// TODO: Add raytracing pipeline desc

#pragma endregion

class Pipeline final
{
public:
	Pipeline(const Context& DeviceContext, const GraphicsPipelineDescriptor& Desc, VkRenderPass Pass, const std::string& Name = "");
	Pipeline(const Context& DeviceContext, const ComputePipelineDescriptor& Desc, const std::string& Name = "");

	~Pipeline();

	bool IsValid() const { return VulkanPipeline != VK_NULL_HANDLE; }

	VkPipeline GetVkPipeline() const { return VulkanPipeline; }

	void Bind(VkCommandBuffer CmdBuffer);

	void UpdateDescriptorSets();

private:
	void CreateGraphicsPipeline();
	void CreateComputePipeline();

	void InitDescriptorLayout();
	void GetSetDescriptorsFromBindPoint(std::vector<SetDescriptor>& InOutSets);

	VkPipelineLayout CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& DescLayouts, const std::vector<VkPushConstantRange>& PushConstants);

private:
	VkDevice VulkanDevice = VK_NULL_HANDLE;
	VkRenderPass VulkanRenderPass = VK_NULL_HANDLE;
	
	VkPipeline VulkanPipeline = VK_NULL_HANDLE;
	VkPipelineLayout VulkanPipelineLayout = VK_NULL_HANDLE;
	VkPipelineBindPoint BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	GraphicsPipelineDescriptor GraphicsPipelineDesc;
	ComputePipelineDescriptor ComputePipelineDesc;

	std::unordered_map<uint32_t, DescriptorSet> DescriptorSets;
	std::vector<VkWriteDescriptorSet> WriteDescSets;
	VkDescriptorPool VulkanDescriptorPool = VK_NULL_HANDLE;

	std::list<std::vector<VkDescriptorBufferInfo>> BufferInfos;
	std::list<VkBufferView> BufferViewInfos;
	std::list<std::vector<VkDescriptorImageInfo>> ImageInfos;

	std::mutex Mutex;

	std::string DebugName;
};

}