include "Dependencies.lua"

project "TextureCompiler"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings ("Extra") -- enables Warning Level 4(/W4)
	dependson "{Library.Compiler}"

	targetdir ("../")
	-- ! makes .obj files appear in the same folder
	-- regardless of build configuration
	objdir ("!bin/obj")

	includedirs
    {
        "include",
		"%{IncludeDir.Compiler}",
    }

	files
	{
		"include/**.h", 
		"src/**.c", 
		"include/**.hpp", 
		"src/**.cpp",
	}

	links
	{
		"%{Library.Compiler}",
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On"
		symbols "Off"