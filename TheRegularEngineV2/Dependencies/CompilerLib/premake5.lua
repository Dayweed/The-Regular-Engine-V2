project "CompilerLib"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings ("Extra") -- enables Warning Level 4(/W4)

	targetdir ("bin")
	-- ! makes .obj files appear in the same folder
	-- regardless of build configuration
	objdir ("!bin/obj")

	includedirs
    {
        "src",
		"%{IncludeDir.glm}"
    }

	files
	{
		"src/**.h", 
		"src/**.c", 
		"src/**.hpp", 
		"src/**.cpp"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"