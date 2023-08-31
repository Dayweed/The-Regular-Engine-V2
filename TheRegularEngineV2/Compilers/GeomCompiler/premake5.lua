project "GeomCompiler"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	warnings ("Extra") -- enables Warning Level 4(/W4)

	targetdir ("bin/%{prj.name}")
	-- ! makes .obj files appear in the same folder
	-- regardless of build configuration
	objdir ("bin/%{prj.name}")

	includedirs
    {
        "include",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.MeshOptimizer}"
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
		"MeshOptimizer",
		"%{Library.Assimp}",
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"