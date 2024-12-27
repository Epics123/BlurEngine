#include "ShaderModule.h"
#include "Context.h"

#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace VulkanCore
{
	static constexpr uint32_t MAX_DESC_BINDLESS = 1000;

	class CustomIncluder final : public glslang::TShader::Includer
	{
	public:
		explicit CustomIncluder(const std::string& ShaderDir) : ShaderDirectory{ShaderDir} {}
		~CustomIncluder() = default;

		IncludeResult* includeSystem(const char* HeaderName, const char* IncluderName, size_t InclusionDepth) override
		{
			// You can implement system include paths here if needed.
			return nullptr;
		}

		IncludeResult* includeLocal(const char* HeaderName, const char* IncluderName, size_t InclusionDepth) override
		{
			std::string FullPath = ShaderDirectory + "/" + HeaderName;
			std::ifstream FileStream(FullPath, std::ios::in);
			if (!FileStream.is_open())
			{
				std::string errMsg = "Failed to open included file: ";
				errMsg.append(HeaderName);
				BE_ERROR(errMsg);
				return nullptr;
			}

			std::stringstream FileContent;
			FileContent << FileStream.rdbuf();
			FileStream.close();

			// The Includer owns the content memory and will delete it when it is no longer needed.
			char* Content = new char[FileContent.str().length() + 1];
			strncpy(Content, FileContent.str().c_str(), FileContent.str().length());
			Content[FileContent.str().length()] = '\0';

			return new IncludeResult(HeaderName, Content, FileContent.str().length(), nullptr);
		}

		void releaseInclude(IncludeResult* Result) override
		{
			if (Result)
			{
				delete Result;
			}
		}

	private:
		std::string ShaderDirectory;
	};

	
	ShaderModule::ShaderModule(const Context& DeviceContext, const std::string& Filepath, const std::string& EntryPoint, VkShaderStageFlagBits Stages, const std::string& Name)
		: Device{DeviceContext.GetDevice()}, ShaderEntryPoint{EntryPoint}, ShaderStageFlags{Stages}
	{
		DebugName = "Shader Module: " + Name;
		CreateShader(Filepath, ShaderEntryPoint, DebugName);
	}

	ShaderModule::ShaderModule(const Context& DeviceContext, const std::vector<char>& Data, const std::string& EntryPoint, VkShaderStageFlagBits Stages, const std::string& Name)
		: Device{ DeviceContext.GetDevice() }, ShaderEntryPoint{ EntryPoint }, ShaderStageFlags{ Stages }
	{
		DebugName = "Shader Module: " + Name;
		CreateShader(Data, ShaderEntryPoint, DebugName);
	}

	ShaderModule::ShaderModule(const Context& DeviceContext, const std::string& Filepath, VkShaderStageFlagBits Stages, const std::string& Name)
		: ShaderModule(DeviceContext, Filepath, "Main", Stages, Name)
	{

	}

	ShaderModule::~ShaderModule()
	{
		vkDestroyShaderModule(Device, VulkanShaderModule, nullptr);
	}

	void ShaderModule::CreateShader(const std::string& Filepath, const std::string& EntryPoint, const std::string& Name)
	{
		std::vector<char> SpirV;
		const bool bIsBinary = Util::FileEndsWith(Filepath.c_str(), ".spv");

		std::vector<char> FileData = Util::ReadFile(Filepath, bIsBinary);

		std::filesystem::path File(Filepath);
		if(bIsBinary)
		{
			SpirV = std::move(FileData);
		}
		else
		{
			GlslToSpirV(FileData, ShaderStageFromFileName(Filepath.c_str()), File.parent_path().string(), EntryPoint.c_str(), SpirV);
		}

		CreateShader(SpirV, EntryPoint, Name);
	}

	void ShaderModule::CreateShader(const std::vector<char>& SpirV, const std::string& EntryPoint, const std::string& Name)
	{
		VkShaderModuleCreateInfo ShaderModuleInfo{};
		ShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ShaderModuleInfo.codeSize = SpirV.size();
		ShaderModuleInfo.pCode = (const uint32_t*)SpirV.data();
		ShaderModuleInfo.pNext = VK_NULL_HANDLE;

		VK_CHECK(vkCreateShaderModule(Device, &ShaderModuleInfo, nullptr, &VulkanShaderModule));
	}

	void ShaderModule::GlslToSpirV(const std::vector<char>& Data, EShLanguage ShaderStage, const std::string& ShaderDir, 
								   const char* EntryPoint, std::vector<char>& OutSpirV)
	{
		static bool sGlslangInitialized = false;
		if(!sGlslangInitialized)
		{
			glslang::InitializeProcess();
			sGlslangInitialized = true;
		}

		glslang::TShader TmpShader(ShaderStage);
		const char* GlslCstr = Data.data();
		TmpShader.setStrings(&GlslCstr, 1);

		glslang::EShTargetClientVersion ClientVersion = glslang::EShTargetVulkan_1_3;
		glslang::EShTargetLanguageVersion LanguageVersion = glslang::EShTargetSpv_1_0;

		if(ShaderStage == EShLangRayGen || ShaderStage == EShLangAnyHit || ShaderStage == EShLangClosestHit || ShaderStage == EShLangMiss)
		{
			LanguageVersion = glslang::EShTargetSpv_1_4;
		}

		TmpShader.setEnvInput(glslang::EShSourceGlsl, ShaderStage, glslang::EShClientVulkan, 460);
		TmpShader.setEnvClient(glslang::EShClientVulkan, ClientVersion);
		TmpShader.setEntryPoint(EntryPoint);
		TmpShader.setSourceEntryPoint(EntryPoint);

		glslang::TShader Shader(ShaderStage);
		Shader.setEnvClient(glslang::EShClientVulkan, ClientVersion);
		Shader.setEnvTarget(glslang::EShTargetSpv, LanguageVersion);
		Shader.setEntryPoint(EntryPoint);
		Shader.setSourceEntryPoint(EntryPoint);

		const TBuiltInResource* Resources = GetDefaultResources();
		const EShMessages Messages = static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo);

		CustomIncluder Includer(ShaderDir);

		std::string PreprocessedGLSL;
		if(!TmpShader.preprocess(Resources, 460, ENoProfile, false, false, Messages, &PreprocessedGLSL, Includer))
		{
			BE_ERROR("Preprocessing failed for shader: {0}", DebugName);
			BE_ERROR("\t{0}", TmpShader.getInfoLog());
			BE_ERROR("\t{0}", TmpShader.getInfoDebugLog());

			return;
		}

		PreprocessedGLSL = RemoveUnecessaryLines(PreprocessedGLSL);

		const char* PreprocessedGLSLCstr = PreprocessedGLSL.c_str();
		Shader.setStrings(&PreprocessedGLSLCstr, 1);

		if(!Shader.parse(Resources, 460, false, Messages))
		{
			BE_ERROR("Parsing failed for shader: {0}", DebugName);
			BE_ERROR("\t{0}", Shader.getInfoLog());
			BE_ERROR("\t{0}", Shader.getInfoDebugLog());

			return;
		}

		glslang::SpvOptions Options;
#if _DEBUG
		Shader.setDebugInfo(true);
		Options.generateDebugInfo = true;
		Options.disableOptimizer = false;
		Options.optimizeSize = false;
		Options.stripDebugInfo = false;
#else
		Options.disableOptimizer = true; // this ensure that variables that aren't used in shaders are not removed, without this flag, SPIRV
										 // generated will be optimized & unused variables will be removed,
										 // this will cause issues in debug vs release if struct on cpu vs gpu are different
		Options.optimizeSize = true;
		Options.stripDebugInfo = true;
#endif

		glslang::TProgram Program;
		Program.addShader(&Shader);
		if(!Program.link(Messages))
		{
			BE_ERROR("Linking failed for shader: {0}", DebugName);
			BE_ERROR("\t{0}", Shader.getInfoLog());
			BE_ERROR("\t{0}", Shader.getInfoDebugLog());

			return;
		}

		std::vector<uint32_t> SpirvData;
		spv::SpvBuildLogger SpvLogger;
		glslang::GlslangToSpv(*Program.getIntermediate(ShaderStage), SpirvData, &SpvLogger, &Options);

		OutSpirV.resize(SpirvData.size() * (sizeof(uint32_t) / sizeof(char)));
		std::memcpy(OutSpirV.data(), SpirvData.data(), OutSpirV.size());
	}

	std::string ShaderModule::RemoveUnecessaryLines(const std::string Str)
	{
		std::istringstream Iss(Str);
		std::ostringstream Oss;
		std::string Line;

		while (std::getline(Iss, Line))
		{
			if (Line != "#extension GL_GOOGLE_include_directive : require" && Line.substr(0, 5) != "#line")
			{
				Oss << Line << '\n';
			}
		}

		return Oss.str();
	}

	EShLanguage ShaderModule::ShaderStageFromFileName(const char* Filename)
	{
		if (Util::FileEndsWith(Filename, ".vert"))
		{
			return EShLangVertex;
		}
		else if (Util::FileEndsWith(Filename, ".frag"))
		{
			return EShLangFragment;
		}
		else if (Util::FileEndsWith(Filename, ".comp"))
		{
			return EShLangCompute;
		}
		else if (Util::FileEndsWith(Filename, ".rgen"))
		{
			return EShLangRayGen;
		}
		else if (Util::FileEndsWith(Filename, ".rmiss"))
		{
			return EShLangMiss;
		}
		else if (Util::FileEndsWith(Filename, ".rchit"))
		{
			return EShLangClosestHit;
		}
		else if (Util::FileEndsWith(Filename, ".rahit"))
		{
			return EShLangAnyHit;
		}

		return EShLangVertex;
	}

}