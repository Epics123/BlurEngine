#pragma once

#include "VulkanCommon.h"

#include <glslang/Public/ShaderLang.h>

class Context;

namespace VulkanCore
{
	class ShaderModule
	{
	public:
		ShaderModule(const Context& DeviceContext, const std::string& Filepath, const std::string& EntryPoint, VkShaderStageFlagBits Stages, const std::string& Name);
		ShaderModule(const Context&	DeviceContext, const std::vector<char>& Data, const std::string& EntryPoint, VkShaderStageFlagBits Stages, const std::string& Name);
		ShaderModule(const Context& DeviceContext, const std::string& Filepath, VkShaderStageFlagBits Stages, const std::string& Name);

		~ShaderModule();

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

		std::string ShaderEntryPoint;
		std::string DebugName = "";
	};
}
