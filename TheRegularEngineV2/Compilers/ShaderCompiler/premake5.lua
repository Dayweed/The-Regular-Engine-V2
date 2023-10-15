project "ShaderCompiler"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings ("Extra") -- enables Warning Level 4(/W4)

	targetdir ("../")
	-- ! makes .obj files appear in the same folder
	-- regardless of build configuration
	objdir ("!bin/obj")

	includedirs
    {
        "include",
		"%{IncludeDir.Shaderc}",
		"%{IncludeDir.VULKANSDK}"
    }

	files
	{
		"include/**.h", 
		"src/**.c", 
		"include/**.hpp", 
		"src/**.cpp"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.Vulkan}"
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "On"
		symbols "Off"
		
		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.Vulkan}"
		}
