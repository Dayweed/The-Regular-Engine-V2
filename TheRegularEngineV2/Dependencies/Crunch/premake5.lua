project "Crunch"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
    staticruntime "off"
	warnings ("Extra") -- enables Warning Level 4(/W4)

	targetdir ("bin")
	-- ! makes .obj files appear in the same folder
	-- regardless of build configuration
	objdir ("!bin/obj")

	defines
	{
		"WIN32=true" -- PhysX Requires This
	}

    includedirs
    {
		"inc"
    }

	files
	{
		"inc/**.h",
		"inc/**.hpp",
		"src/**.cpp",
		"src/**.c",
		"crnlib/**.c",
		"crnlib/**.cpp",
	}

	links
	{
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
