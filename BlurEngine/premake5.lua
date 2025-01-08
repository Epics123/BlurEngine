include "Dependencies.lua"

workspace "BlurEngine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}={cfg.architecture}"

project "BlurEngine"
	location "BlurEngine"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/Source/Engine/**.h",
		"%{prj.name}/Source/Engine/**.cpp",
		"%{prj.name}/Source/Engine/Core/**.cpp",
		"%{prj.name}/Source/Engine/VulkanCore/**.cpp",
		--"%{prj.name}/Libraries/ImGui/**.h",
		--"%{prj.name}/Libraries/ImGui/**.cpp",
		--"%{prj.name}/Libraries/ImGui/ImGuizmo/**.h",
		--"%{prj.name}/Libraries/ImGui/ImGuizmo/**.cpp",
		--"%{prj.name}/Libraries/yaml/src/**.h",
		--"%{prj.name}/Libraries/yaml/src/**.cpp",
		"%{prj.name}/Source/Resources/Shaders/**.vert",
		"%{prj.name}/Source/Resources/Shaders/**.frag"
	}

	includedirs
	{
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.Core}",
		"%{IncludeDir.VulkanCore}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		--"%{IncludeDir.ImGui}",
		"%{IncludeDir.ObjLoader}",
		--"%{IncludeDir.ImgLoader}",
		"%{IncludeDir.spdlog}",
		--"%{IncludeDir.yaml}"
	}

	libdirs
	{
		"%{LibraryDir.VulkanSDK}",
		"%{LibraryDir.GLFW}",
		"%{Library.VulkanUtils}"
	}

	links
	{
		"glfw3_mt.lib",
		"vulkan-1.lib",
		"kernel32.lib",
		"user32.lib",
		"gdi32.lib",
		"winspool.lib",
		"comdlg32.lib",
		"advapi32.lib",
		"shell32.lib",
		"ole32.lib",
		"oleaut32.lib",
		"uuid.lib",
		"odbc32.lib",
		"odbccp32.lib"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "On"
		staticruntime "Off"
		buildoptions {"/wd4996", "/wd4099"}
		links
		{
			"glslangd.lib",
			"glslang-default-resource-limitsd.lib",
			"GenericCodeGend.lib",
			"SPIRVd.lib",
			"SPIRV-Toolsd.lib",
			"SPIRV-Tools-optd.lib",
			"OSDependentd.lib",
			"MachineIndependentd.lib"
		}

	filter "configurations:Release"
		optimize "On"
		staticruntime "Off"
		runtime "Release"
		links
		{
			"glslang.lib",
			"glslang-default-resource-limits.lib",
			"GenericCodeGen.lib",
			"SPIRV.lib",
			"SPIRV-Tools.lib",
			"SPIRV-Tools-opt.lib",
			"OSDependent.lib",
			"MachineIndependent.lib"
		}