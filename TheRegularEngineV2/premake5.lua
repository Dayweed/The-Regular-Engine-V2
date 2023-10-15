include "Dependencies.lua"

workspace "TheRegularEngineV2"
	configurations { "Debug", "Release" }
	targetdir "build"
	startproject "TheRegularEditor"

	flags
	{
		"MultiProcessorCompile"
	}

	filter "language:C++ or language:C"
		architecture "x86_64"
	filter ""

outputdir = "%{cfg.buildcfg}"

group "Dependencies"
include "Dependencies/ImGui"
include "Dependencies/Math"
-- include "Dependencies/MeshOptimizer"
-- include "Dependencies/CompilerLib"
-- include "Compilers/TextureCompiler"
-- include "Compilers/GeomCompiler"
-- include "Compilers/ShaderCompiler"
group ""

project "TheRegularEngine"
	location "TheRegularEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings "Extra"

	targetdir ("Executable/" .. outputdir .. "/%{prj.name}/")
	objdir ("Executable/" .. outputdir .. "/%{prj.name}/")

	pchheader "pch.h"
	pchsource "TheRegularEngine/pch.cpp"

	files
	{ 
		"%{prj.name}/**.h", 
		"%{prj.name}/**.c", 
		"%{prj.name}/**.hpp", 
		"%{prj.name}/**.cpp",
	}

	includedirs
	{
		"%{prj.name}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.FMOD}",
		"%{IncludeDir.Freetype}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuiDirect}",
		"%{IncludeDir.ImGuiBackEnd}",
		"%{IncludeDir.ImGuiMisc}",
		"%{IncludeDir.Math}",
		"%{IncludeDir.Mono}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.VULKANSDK}",
		"%{IncludeDir.Rapidjson}",
		"%{IncludeDir.Nlohmannjson}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stbi}",
		"%{IncludeDir.Compiler}",
		"%{IncludeDir.Properties}",
		"%{IncludeDir.SPIRVREFLECT}",
	}

	defines
	{
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_RADIANS",
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_CXX20_CISO646_REMOVED_WARNING", -- to remove C4996 warning about some STL header being deprecated
	}

	links
	{ 
		"%{Library.Assimp}",
		"%{Library.Freetype}",
		"%{Library.GLFW}",
		"%{Library.Mono}",
		"%{Library.PhysX_64}",
		"%{Library.PhysX_Character}",
		"%{Library.PhysX_Common}",
		--"%{Library.PhysX_Cooking}",
		"%{Library.PhysX_Extension}",
		"%{Library.PhysX_Foundation}",
		"%{Library.PhysX_Pvd}",
		"%{Library.PhysX_TaskStatic}",
		"%{Library.PhysX_VehicleStatic}",
		"%{Library.PhysX_Vehicle2}",
		"%{Library.Vulkan}",
		"%{Library.Math}",
		"%{Library.Compiler}",
	}

	filter "configurations:Debug"
		symbols "On"

		defines
		{
			"DEBUG",
			"_DEBUG",
		}

		links
		{
			"%{Library.FMOD_Debug}",
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}",
		}
		
	filter "configurations:Release"
		optimize "On"

		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.FMOD_Release}",
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
		}

project "TheRegularEditor"
	location "TheRegularEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings "Extra"
	
	targetdir ("Executable/" .. outputdir .. "/%{prj.name}/")
	objdir ("Executable/" .. outputdir .. "/%{prj.name}/")

	links 
	{ 
		"TheRegularEngine",
		"ImGui",
	}

	defines 
	{
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_RADIANS",
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_CXX20_CISO646_REMOVED_WARNING", -- to remove C4996 warning about some STL header being deprecated
	}

	files 
	{ 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp", 
	}

	includedirs 
	{
		"%{prj.name}/src",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.FMOD}",
		"%{IncludeDir.Freetype}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuiDirect}",
		"%{IncludeDir.ImGuiBackEnd}",
		"%{IncludeDir.ImGuiMisc}",
		"%{IncludeDir.Mono}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.VULKANSDK}",
		"%{IncludeDir.Rapidjson}",
		"%{IncludeDir.Nlohmannjson}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.Math}",
		"%{IncludeDir.Compiler}",
		"%{IncludeDir.Properties}",
		"%{IncludeDir.SPIRVREFLECT}",
		"TheRegularEngine"
	}

	postbuildcommands
	{
		'{COPY} "%{Binaries.PhysX_64}" "%{cfg.targetdir}"',
		'{COPY} "%{Binaries.PhysX_Common}" "%{cfg.targetdir}"',
		'{COPY} "%{Binaries.PhysX_Cooking}" "%{cfg.targetdir}"',
		-- '{COPY} "%{Binaries.PhysX_Device}" "%{cfg.targetdir}"',
		'{COPY} "%{Binaries.PhysX_Foundation}" "%{cfg.targetdir}"',
		'{COPY} "imgui.ini" "%{cfg.targetdir}"',
	}

	filter "configurations:Debug"
		symbols "on"

		defines
		{
			"DEBUG",
			"_DEBUG",
		}

		links
		{
			"%{Library.FMOD_Debug}"
		}

		postbuildcommands
		{
			'{COPY} "%{Binaries.Assimp}" "%{cfg.targetdir}"',
			'{COPY} "%{Binaries.FMOD_Debug}" "%{cfg.targetdir}"',
			'{COPY} "%{Binaries.Mono}/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
		}

	filter "configurations:Release"
		optimize "on"

		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.FMOD_Release}"
		}

		postbuildcommands
		{
			'{COPY} "%{Binaries.Assimp}" "%{cfg.targetdir}"',
			'{COPY} "%{Binaries.FMOD_Release}" "%{cfg.targetdir}"',
			'{COPY} "%{Binaries.Mono}/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
		}


project "TRE-Runtime"
	location "TRE-Runtime"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings "Extra"
	
	targetdir ("Executable/" .. outputdir .. "/%{prj.name}/")
	objdir ("Executable/" .. outputdir .. "/%{prj.name}/")

	links 
	{ 
		"TheRegularEngine",
		"ImGui"
	}

	defines 
	{
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_RADIANS",
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_CXX20_CISO646_REMOVED_WARNING", -- to remove C4996 warning about some STL header being deprecated
	}

	files 
	{ 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp", 
	}

	includedirs 
	{
		"TheRegularEngine",
		"Dependencies/Math/include",
		-- "%{IncludeDir.Assimp}",
		"%{IncludeDir.FMOD}",
		-- "%{IncludeDir.Freetype}",
		-- "%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Mono}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.VULKANSDK}",
		-- "%{IncludeDir.Rapidjson}",
		"%{IncludeDir.Nlohmannjson}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.Compiler}",
		"%{IncludeDir.Properties}",
	}

	postbuildcommands
	{
		'{COPY} "%{Binaries.PhysX_64}" "%{cfg.targetdir}"',
		'{COPY} "%{Binaries.PhysX_Common}" "%{cfg.targetdir}"',
		'{COPY} "%{Binaries.PhysX_Cooking}" "%{cfg.targetdir}"',
		'{COPY} "%{Binaries.PhysX_Foundation}" "%{cfg.targetdir}"',
	}

	filter "configurations:Debug"
		symbols "on"

		defines
		{
			"DEBUG",
			"_DEBUG",
		}

		links
		{
			"%{Library.FMOD_Debug}"
		}

		postbuildcommands
		{
			'{COPY} "%{Binaries.Assimp}" "%{cfg.targetdir}"',
			'{COPY} "%{Binaries.FMOD_Debug}" "%{cfg.targetdir}"',
			'{COPY} "%{Binaries.Mono}/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
		}

	filter "configurations:Release"
		optimize "on"

		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.FMOD_Release}"
		}

		postbuildcommands
		{
			'{COPY} "%{Binaries.Assimp}" "%{cfg.targetdir}"',
			'{COPY} "%{Binaries.FMOD_Release}" "%{cfg.targetdir}"',
			'{COPY} "%{Binaries.Mono}/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
		}

project "TRE-ScriptCore"
	location "TRE-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("Resources/Scripts")
	objdir ("Resources/Scripts")

	files 
	{
		"%{prj.name}/src/**.cs",
	}

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

