#pragma once

#include "VulkanCommon.h"

#include <glslang/Public/ShaderLang.h>

namespace VulkanCore
{
	class Context;

	class ShaderModule
	{
	public:
		explicit ShaderModule(const Context& DeviceContext, const std::string& Filepath, const std::string& EntryPoint, VkShaderStageFlagBits Stages, const std::string& Name);
		explicit ShaderModule(const Context& DeviceContext, const std::vector<char>& Data, const std::string& EntryPoint, VkShaderStageFlagBits Stages, const std::string& Name);
		explicit ShaderModule(const Context& DeviceContext, const std::string& Filepath, VkShaderStageFlagBits Stages, const std::string& Name);

		~ShaderModule();

		VkShaderModule GetVkShaderModule() const { return VulkanShaderModule; }
		VkShaderStageFlagBits GetShaderFlags() const { return ShaderStageFlags; }
		const std::string& GetEntryPoint() const { return ShaderEntryPoint; }

	private:
		void CreateShader(const std::string& Filepath, const std::string& EntryPoint, const std::string& Name);
		void CreateShader(const std::vector<char>& SpirV, const std::string& EntryPoint, const std::string& Name);

		void GlslToSpirV(const std::vector<char>& Data, EShLanguage ShaderStage, const std::string& ShaderDir, 
						 const char* EntryPoint, std::vector<char>& OutSpirV);

		std::string RemoveUnecessaryLines(const std::string Str);

		EShLanguage ShaderStageFromFileName(const char* Filename);

	private:
		VkDevice Device;
		VkShaderModule VulkanShaderModule = VK_NULL_HANDLE;
		VkShaderStageFlagBits ShaderStageFlags;

		std::string ShaderEntryPoint = "main";
		std::string DebugName = "";
	};
}

