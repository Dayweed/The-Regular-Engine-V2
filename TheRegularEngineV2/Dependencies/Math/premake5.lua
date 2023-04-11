project "Math"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("Executable/" .. outputdir .. "/%{prj.name}")
	objdir ("Executable/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/**.h",
		"src/**.cpp",
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"