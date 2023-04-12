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
group ""

project "TheRegularEngine"
	location "TheRegularEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("Executable/" .. outputdir .. "/%{prj.name}")
	objdir ("Executable/" .. outputdir .. "/%{prj.name}")

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
		"%{IncludeDir.ImGuiBackEnd}",
		"%{IncludeDir.Math}",
		"%{IncludeDir.Mono}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.VULKANSDK}",
		"%{IncludeDir.Rapidjson}",
		"%{IncludeDir.spdlog}",
	}

	defines
	{
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
	}

	links
	{ 
		"%{Library.Assimp}",
		"%{Library.Freetype}",
		"%{Library.GLFW}",
		"%{Library.Mono}",
		"%{Library.Vulkan}",
		"ImGui"
	}

	filter "configurations:Debug"
		symbols "On"

		links
		{
			"%{Library.FMOD_Debug}"
		}

	filter "configurations:Release"
		optimize "On"

		defines
		{
			"NDEBUG" -- PhysX Requires This
		}

		links
		{
			"%{Library.FMOD_Release}"
		}

project "TheRegularEditor"
	location "TheRegularEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	
	targetdir ("Executable/" .. outputdir .. "/%{prj.name}")
	objdir ("Executable/" .. outputdir .. "/%{prj.name}")

	links 
	{ 
		"TheRegularEngine"
	}

	defines 
	{
		"GLM_FORCE_DEPTH_ZERO_TO_ONE"
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
		"TheRegularEngine",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.FMOD}",
		"%{IncludeDir.Freetype}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuiBackEnd}",
		"%{IncludeDir.Math}",
		"%{IncludeDir.Mono}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.VULKANSDK}",
		"%{IncludeDir.Rapidjson}",
		"%{IncludeDir.spdlog}",
	}

	filter "configurations:Debug"
		symbols "on"

		links
		{
			"%{Library.FMOD_Debug}"
		}

		postbuildcommands 
		{
			'{COPY} "%{Binaries.Assimp}" "%{cfg.targetdir}"',
			'{COPY} "%{Binaries.FMOD_Debug}" "%{cfg.targetdir}"',
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
		}