#include "Pipeline.h"
#include "Context.h"

namespace VulkanCore
{
	
	Pipeline::Pipeline(const Context& DeviceContext, const GraphicsPipelineDescriptor& Desc, VkRenderPass Pass, const std::string& Name)
		: VulkanDevice {DeviceContext.GetDevice()}, GraphicsPipelineDesc{Desc}, BindPoint{VK_PIPELINE_BIND_POINT_GRAPHICS}, VulkanRenderPass{Pass}, DebugName{Name}
	{
		CreateGraphicsPipeline();
	}

	Pipeline::Pipeline(const Context& DeviceContext, const ComputePipelineDescriptor& Desc, const std::string& Name)
		: VulkanDevice {DeviceContext.GetDevice()}, ComputePipelineDesc{Desc}, BindPoint{VK_PIPELINE_BIND_POINT_COMPUTE}, DebugName{Name}
	{
		CreateComputePipeline();
	}

	Pipeline::~Pipeline()
	{
		vkDestroyPipeline(VulkanDevice, VulkanPipeline, nullptr);
		vkDestroyPipelineLayout(VulkanDevice, VulkanPipelineLayout, nullptr);
		vkDestroyDescriptorPool(VulkanDevice, VulkanDescriptorPool, nullptr);

		for(const std::pair<uint32_t, DescriptorSet> Set : DescriptorSets)
		{
			vkDestroyDescriptorSetLayout(VulkanDevice, Set.second.Layout, nullptr);
		}
	}

	void Pipeline::Bind(VkCommandBuffer CmdBuffer)
	{
		vkCmdBindPipeline(CmdBuffer, BindPoint, VulkanPipeline);
		UpdateDescriptorSets();
	}

	void Pipeline::UpdateDescriptorSets()
	{
		if(!WriteDescSets.empty())
		{
			std::unique_lock<std::mutex> MutexLock(Mutex);
			vkUpdateDescriptorSets(VulkanDevice, WriteDescSets.size(), WriteDescSets.data(), 0, nullptr);

			WriteDescSets.clear();
			BufferInfos.clear();
			BufferViewInfos.clear();
			ImageInfos.clear();
		}
	}

	void Pipeline::CreateGraphicsPipeline()
	{
		const std::vector<VkSpecializationMapEntry>& VertexSpecConstants = GraphicsPipelineDesc.VertexSpecConstants;

		VkSpecializationInfo VertexSpecializationInfo{};
		VertexSpecializationInfo.mapEntryCount = static_cast<uint32_t>(VertexSpecConstants.size());
		VertexSpecializationInfo.pMapEntries = VertexSpecConstants.data();
		VertexSpecializationInfo.dataSize = !VertexSpecConstants.empty() ? VertexSpecConstants.back().offset + VertexSpecConstants.back().size : 0;
		VertexSpecializationInfo.pData = GraphicsPipelineDesc.VertexSpecializationData;

		const std::vector<VkSpecializationMapEntry>& FragmentSpecConstants = GraphicsPipelineDesc.FragmentSpecConstants;

		VkSpecializationInfo FragmentSpecializationInfo{};
		FragmentSpecializationInfo.mapEntryCount = static_cast<uint32_t>(FragmentSpecConstants.size());
		FragmentSpecializationInfo.pMapEntries = FragmentSpecConstants.data();
		FragmentSpecializationInfo.dataSize = !FragmentSpecConstants.empty() ? FragmentSpecConstants.back().offset + FragmentSpecConstants.back().size : 0;
		FragmentSpecializationInfo.pData = GraphicsPipelineDesc.FragmentSpecializationData;

		const std::shared_ptr<ShaderModule> VertexShader = GraphicsPipelineDesc.VertexShader.lock();
		ASSERT(VertexShader, "Vertex shader's ShaderModule had been destroyed before being used to create a pipeline!");

		const std::shared_ptr<ShaderModule> FragmentShader = GraphicsPipelineDesc.FragmentShader.lock();
		ASSERT(FragmentShader, "Fragment shader's ShaderModule had been destroyed before being used to create a pipeline!");

		VkPipelineShaderStageCreateInfo VertexCreateInfo{};
		VertexCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		VertexCreateInfo.stage = VertexShader->GetShaderFlags();
		VertexCreateInfo.module = VertexShader->GetVkShaderModule();
		VertexCreateInfo.pName = VertexShader->GetEntryPoint().c_str();
		VertexCreateInfo.pSpecializationInfo = !VertexSpecConstants.empty() ? &VertexSpecializationInfo : nullptr;
		VertexCreateInfo.pNext = nullptr;

		VkPipelineShaderStageCreateInfo FragCreateInfo{};
		FragCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		FragCreateInfo.stage = FragmentShader->GetShaderFlags();
		FragCreateInfo.module = FragmentShader->GetVkShaderModule();
		FragCreateInfo.pName = FragmentShader->GetEntryPoint().c_str();
		FragCreateInfo.pSpecializationInfo = !FragmentSpecConstants.empty() ? &FragmentSpecializationInfo : nullptr;
		FragCreateInfo.pNext = nullptr;

		std::array<VkPipelineShaderStageCreateInfo, 2> ShaderStages{ VertexCreateInfo, FragCreateInfo };

		VkPipelineInputAssemblyStateCreateInfo InputAssembly{};
		InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		InputAssembly.topology = GraphicsPipelineDesc.PrimitiveTopology;
		InputAssembly.primitiveRestartEnable = VK_FALSE;
		InputAssembly.pNext = VK_NULL_HANDLE;

		const VkViewport Viewport = GraphicsPipelineDesc.Viewport.ToVkViewport();

		VkRect2D Scissor{};
		Scissor.offset = {0, 0};
		Scissor.extent = GraphicsPipelineDesc.Viewport.ToExtent2D();

		VkPipelineViewportStateCreateInfo ViewportState{};
		ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportState.viewportCount = 1;
		ViewportState.pViewports = &Viewport;
		ViewportState.scissorCount = 1;
		ViewportState.pScissors = &Scissor;
		ViewportState.pNext = VK_NULL_HANDLE;

		VkPipelineRasterizationStateCreateInfo RasterizerCreateInfo{};
		RasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		RasterizerCreateInfo.depthClampEnable = VK_FALSE;
		RasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		RasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		RasterizerCreateInfo.cullMode = VkCullModeFlags(GraphicsPipelineDesc.CullMode);
		RasterizerCreateInfo.frontFace = GraphicsPipelineDesc.FrontFace;
		RasterizerCreateInfo.depthBiasEnable = VK_FALSE;
		RasterizerCreateInfo.depthBiasConstantFactor = 0.0f; // Optional
		RasterizerCreateInfo.depthBiasClamp = 0.0f; // Optional
		RasterizerCreateInfo.depthBiasSlopeFactor = 0.0f; // Optional
		RasterizerCreateInfo.lineWidth = 1.0f;
		RasterizerCreateInfo.pNext = VK_NULL_HANDLE;

		VkPipelineMultisampleStateCreateInfo MultisampleCreateInfo{};
		MultisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		MultisampleCreateInfo.rasterizationSamples = GraphicsPipelineDesc.SampleCount;
		MultisampleCreateInfo.sampleShadingEnable = VK_FALSE;
		MultisampleCreateInfo.minSampleShading = 1.0f; // Optional
		MultisampleCreateInfo.pSampleMask = nullptr; // Optional
		MultisampleCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
		MultisampleCreateInfo.alphaToOneEnable = VK_FALSE; // Optional
		MultisampleCreateInfo.pNext = VK_NULL_HANDLE;

		std::vector<VkPipelineColorBlendAttachmentState> ColorBlendAttachments;

		if(GraphicsPipelineDesc.BlendAttachmentStates.size() > 0)
		{
			ASSERT(GraphicsPipelineDesc.BlendAttachmentStates.size() == GraphicsPipelineDesc.ColorTextureFormats.size(), "Blend states need to be provided for all color textures!");
			ColorBlendAttachments = GraphicsPipelineDesc.BlendAttachmentStates;
		}
		else
		{
			VkPipelineColorBlendAttachmentState DefaultAttachmentState{};
			DefaultAttachmentState.blendEnable = GraphicsPipelineDesc.bBlendEnable;
			DefaultAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
			DefaultAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
			DefaultAttachmentState.colorBlendOp = VK_BLEND_OP_ADD; // Optional
			DefaultAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
			DefaultAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA; // Optional
			DefaultAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
			DefaultAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

			ColorBlendAttachments = std::vector<VkPipelineColorBlendAttachmentState>(GraphicsPipelineDesc.ColorTextureFormats.size(), DefaultAttachmentState);
		}

		VkPipelineColorBlendStateCreateInfo ColorBlendCreateInfo{};
		ColorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ColorBlendCreateInfo.logicOpEnable = VK_FALSE;
		ColorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
		ColorBlendCreateInfo.attachmentCount = uint32_t(ColorBlendAttachments.size());
		ColorBlendCreateInfo.pAttachments = ColorBlendAttachments.data();
		ColorBlendCreateInfo.pNext = VK_NULL_HANDLE;

		InitDescriptorLayout();

		std::vector<VkDescriptorSetLayout> DescriptorSetLayouts(DescriptorSets.size());
		for(const std::pair<uint32_t, DescriptorSet>& DescSet : DescriptorSets)
		{
			DescriptorSetLayouts[DescSet.first] = DescSet.second.Layout;
		}

		VulkanPipelineLayout = CreatePipelineLayout(DescriptorSetLayouts, GraphicsPipelineDesc.PushConstants);

		VkPipelineDepthStencilStateCreateInfo DepthStencilInfo{};
		DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		DepthStencilInfo.depthTestEnable = GraphicsPipelineDesc.bDepthTestEnable;
		DepthStencilInfo.depthWriteEnable = GraphicsPipelineDesc.bDepthWriteEnable;
		DepthStencilInfo.depthCompareOp = GraphicsPipelineDesc.DepthCompareOperation;
		DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		DepthStencilInfo.stencilTestEnable = VK_TRUE;
		DepthStencilInfo.front = {};
		DepthStencilInfo.back = {};
		DepthStencilInfo.minDepthBounds = 0.0f;
		DepthStencilInfo.maxDepthBounds = 1.0f;
		DepthStencilInfo.pNext = VK_NULL_HANDLE;

		VkPipelineDynamicStateCreateInfo DynamicStateInfo{};
		DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		DynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(GraphicsPipelineDesc.DynamicStates.size());
		DynamicStateInfo.pDynamicStates = GraphicsPipelineDesc.DynamicStates.data();
		DynamicStateInfo.pNext = VK_NULL_HANDLE;

		VkPipelineRenderingCreateInfo PipelineRenderingCreateInfo{};
		PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		PipelineRenderingCreateInfo.colorAttachmentCount = uint32_t(GraphicsPipelineDesc.ColorTextureFormats.size());
		PipelineRenderingCreateInfo.pColorAttachmentFormats = GraphicsPipelineDesc.ColorTextureFormats.data();
		PipelineRenderingCreateInfo.depthAttachmentFormat = GraphicsPipelineDesc.DepthTextureFormat;
		PipelineRenderingCreateInfo.stencilAttachmentFormat = GraphicsPipelineDesc.StencilTextureFormat;
		PipelineRenderingCreateInfo.pNext = VK_NULL_HANDLE;

		VkGraphicsPipelineCreateInfo PipelineInfo{};
		PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		PipelineInfo.pNext = GraphicsPipelineDesc.bUseDynamicRendering ? &PipelineRenderingCreateInfo : VK_NULL_HANDLE;
		PipelineInfo.stageCount = uint32_t(ShaderStages.size());
		PipelineInfo.pStages = ShaderStages.data();
		PipelineInfo.pVertexInputState = &GraphicsPipelineDesc.VertexInputCreateInfo;
		PipelineInfo.pInputAssemblyState = &InputAssembly;
		PipelineInfo.pViewportState = &ViewportState;
		PipelineInfo.pRasterizationState = &RasterizerCreateInfo;
		PipelineInfo.pMultisampleState = &MultisampleCreateInfo;
		PipelineInfo.pDepthStencilState = &DepthStencilInfo; // Optional
		PipelineInfo.pColorBlendState = &ColorBlendCreateInfo;
		PipelineInfo.pDynamicState = &DynamicStateInfo;
		PipelineInfo.layout = VulkanPipelineLayout;
		PipelineInfo.renderPass = VulkanRenderPass;
		PipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		PipelineInfo.basePipelineIndex = -1; // Optional
		PipelineInfo.pTessellationState = VK_NULL_HANDLE;

		DebugName = "Graphics Pipeline: " + DebugName;
		const std::string ErrorMsg = "Failed to create " + DebugName + "!";
		VK_CHECK(vkCreateGraphicsPipelines(VulkanDevice, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &VulkanPipeline), ErrorMsg.c_str());
	}

	void Pipeline::CreateComputePipeline()
	{
		const std::shared_ptr<ShaderModule> ComputeShader = ComputePipelineDesc.ComputeShader.lock();
		ASSERT(ComputeShader, "Compute Shader's ShaderMopdule has been destroyed before being used to create a pipeline!");

		const std::vector<VkSpecializationMapEntry>& SpecConstants = ComputePipelineDesc.SpecializationConstants;

		VkSpecializationInfo SpecializationInfo{};
		SpecializationInfo.mapEntryCount = static_cast<uint32_t>(SpecConstants.size());
		SpecializationInfo.pMapEntries = SpecConstants.data();
		SpecializationInfo.dataSize = !SpecConstants.empty() ? SpecConstants.back().offset + SpecConstants.back().size : 0;
		SpecializationInfo.pData = ComputePipelineDesc.SpecializationData;

		InitDescriptorLayout();

		std::vector<VkDescriptorSetLayout> DescSetLayouts;
		for (const std::pair<uint32_t, DescriptorSet>& DescSet : DescriptorSets)
		{
			DescSetLayouts.push_back(DescSet.second.Layout);
		}

		VulkanPipelineLayout = CreatePipelineLayout(DescSetLayouts, ComputePipelineDesc.PushConstants);

		VkPipelineShaderStageCreateInfo ShaderStageInfo{};
		ShaderStageInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		ShaderStageInfo.flags = 0;
		ShaderStageInfo.stage = ComputeShader->GetShaderFlags();
		ShaderStageInfo.module = ComputeShader->GetVkShaderModule();
		ShaderStageInfo.pName = ComputeShader->GetEntryPoint().c_str();
		ShaderStageInfo.pSpecializationInfo = !ComputePipelineDesc.SpecializationConstants.empty() ? &SpecializationInfo : nullptr;
		ShaderStageInfo.pNext = nullptr;

		VkComputePipelineCreateInfo ComputePipelineInfo{};
		ComputePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		ComputePipelineInfo.flags = 0;
		ComputePipelineInfo.stage = ShaderStageInfo;
		ComputePipelineInfo.layout = VulkanPipelineLayout;
		ComputePipelineInfo.pNext = VK_NULL_HANDLE;

		DebugName = "Compute Pipeline: " + DebugName;
		const std::string ErrorMsg = "Failed to create " + DebugName + "!";
		VK_CHECK(vkCreateComputePipelines(VulkanDevice, VK_NULL_HANDLE, 1, &ComputePipelineInfo, nullptr, &VulkanPipeline), ErrorMsg);
	}

	void Pipeline::InitDescriptorLayout()
	{
		std::vector<SetDescriptor> Sets;
		GetSetDescriptorsFromBindPoint(Sets);

		constexpr VkDescriptorBindingFlags FlagsToEnable = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

		for(SetDescriptor& Set : Sets)
		{
			std::vector<VkDescriptorBindingFlags> BindFlags(Set.Bindings.size(), FlagsToEnable);

			VkDescriptorSetLayoutBindingFlagsCreateInfo LayoutFlagsInfo{};
			LayoutFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			LayoutFlagsInfo.pNext = nullptr;
			LayoutFlagsInfo.bindingCount = static_cast<uint32_t>(Set.Bindings.size());
			LayoutFlagsInfo.pBindingFlags = BindFlags.data();

			VkDescriptorSetLayoutCreateInfo DescriptorLayoutInfo{};
			DescriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			DescriptorLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
			DescriptorLayoutInfo.bindingCount = static_cast<uint32_t>(Set.Bindings.size());
			DescriptorLayoutInfo.pBindings = !Set.Bindings.empty() ? Set.Bindings.data() : nullptr;
			DescriptorLayoutInfo.pNext = VK_NULL_HANDLE;

			VkDescriptorSetLayout Layout = VK_NULL_HANDLE;

			const std::string ErrorMsg = "Failed to create descriptor set: " + DebugName;
			VK_CHECK(vkCreateDescriptorSetLayout(VulkanDevice, &DescriptorLayoutInfo, nullptr, &Layout), ErrorMsg.c_str());

			DescriptorSets[Set.SetIndex].Layout = Layout;
		}
	}

	void Pipeline::GetSetDescriptorsFromBindPoint(std::vector<SetDescriptor>& InOutSets)
	{
		switch (BindPoint)
		{
		case VK_PIPELINE_BIND_POINT_GRAPHICS:
			InOutSets = GraphicsPipelineDesc.SetDescriptors;
			break;
		case VK_PIPELINE_BIND_POINT_COMPUTE:
			InOutSets = ComputePipelineDesc.SetDescriptors;
			break;
		case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
			break;
		default:
			break;
		}
	}

	VkPipelineLayout Pipeline::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& DescLayouts, const std::vector<VkPushConstantRange>& PushConstants)
	{
		VkPipelineLayoutCreateInfo PipelineLayoutInfo{};
		PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		PipelineLayoutInfo.setLayoutCount = (uint32_t)DescLayouts.size();
		PipelineLayoutInfo.pSetLayouts = DescLayouts.data();
		PipelineLayoutInfo.pushConstantRangeCount = !PushConstants.empty() ? static_cast<uint32_t>(PushConstants.size()) : 0;
		PipelineLayoutInfo.pPushConstantRanges = !PushConstants.empty() ? PushConstants.data() : nullptr;
		PipelineLayoutInfo.pNext = VK_NULL_HANDLE;

		VkPipelineLayout Layout = VK_NULL_HANDLE;
		VK_CHECK(vkCreatePipelineLayout(VulkanDevice, &PipelineLayoutInfo, nullptr, &Layout), "Failed to create pipeline layout");

		return Layout;
	}

}