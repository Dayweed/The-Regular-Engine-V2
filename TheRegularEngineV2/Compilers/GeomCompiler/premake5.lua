project "GeomCompiler"
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
		"%{IncludeDir.glm}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.MeshOptimizer}",
		"%{IncludeDir.Compiler}",
		"%{IncludeDir.glm}",
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
		"CompilerLib",
		"%{Library.Assimp}",
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"
		postbuildcommands
		{
			'{COPY} "%{Binaries.Assimp}" "../"',
		}

	filter "configurations:Release"
		runtime "Release"
		optimize "On"
		symbols "Off"
		postbuildcommands
		{
			'{COPY} "%{Binaries.Assimp}" "../"',
		}