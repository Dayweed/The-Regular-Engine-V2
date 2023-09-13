project "CompilerLib"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings ("Extra") -- enables Warning Level 4(/W4)

	targetdir ("%{cfg.buildcfg}")
	objdir ("!%{cfg.buildcfg}/obj")

	includedirs
    {
        "include",
		"%{IncludeDir.glm}"
    }

	files
	{
		"include/**.h", 
		"src/**.c", 
		"include/**.hpp", 
		"src/**.cpp"
	}

	links
	{
		"Crunch",
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"