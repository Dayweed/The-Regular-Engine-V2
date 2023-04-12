--Dependencies File--

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["Assimp"] = "%{wks.location}/Dependencies/Assimp/include"
IncludeDir["FMOD"] = "%{wks.location}/Dependencies/FMOD/include"
IncludeDir["Freetype"] = "%{wks.location}/Dependencies/Freetype/include"
IncludeDir["GLFW"] = "%{wks.location}/Dependencies/GLFW/include"
IncludeDir["glm"] = "%{wks.location}/Dependencies/GLM/include/"
IncludeDir["ImGui"] = "%{wks.location}/Dependencies/ImGui/include"
IncludeDir["ImGuiBackEnd"] = "%{wks.location}/Dependencies/ImGui/backends"
IncludeDir["Math"] = "%{wks.location}/Dependencies/Math/include"
IncludeDir["Mono"] = "%{wks.location}/Dependencies/Mono/include"
IncludeDir["PhysX"] = "%{wks.location}/Dependencies/PhysX/include"
IncludeDir["VULKANSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["Rapidjson"] = "%{wks.location}/Dependencies/Rapidjson/include"
IncludeDir["spdlog"] = "%{wks.location}/Dependencies/spdlog"

LibraryDir = {}
LibraryDir["Assimp"] = "%{wks.location}/Dependencies/assimp/lib"
LibraryDir["FMOD"] = "%{wks.location}/Dependencies/FMOD/lib"
LibraryDir["Freetype"] = "%{wks.location}/Dependencies/Freetype/lib"
LibraryDir["GLFW"] = "%{wks.location}/Dependencies/GLFW/lib-vc2022"
LibraryDir["Mono"] = "%{wks.location}/Dependencies/Mono/lib/%{cfg.buildcfg}"
LibraryDir["PhysX"] = "%{wks.location}/Dependencies/PhysX/lib/%{cfg.buildcfg}"
LibraryDir["Vulkan"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Assimp"] = "%{LibraryDir.Assimp}/assimp-vc143-mt.lib"
Library["FMOD_Debug"] = "%{LibraryDir.FMOD}/fmodL_vc.lib"
Library["FMOD_Release"] = "%{LibraryDir.FMOD}/fmod_vc.lib"
Library["Freetype"] = "%{LibraryDir.Freetype}/freetype.lib"
Library["GLFW"] = "%{LibraryDir.GLFW}/glfw3.lib"
Library["Mono"] = "%{LibraryDir.Mono}/libmono-static-sgen.lib"
-- Library["PhysX"] = "%{LibraryDir.PhysX}/"
Library["Vulkan"] = "%{LibraryDir.Vulkan}/vulkan-1.lib"

Binaries = {}
Binaries["Assimp"] = "%{wks.location}/Dependencies/Assimp/lib/assimp-vc143-mt.dll"
Binaries["FMOD_Debug"] = "%{wks.location}/Dependencies/FMOD/lib/fmodL.dll"
Binaries["FMOD_Release"] = "%{wks.location}/Dependencies/FMOD/lib/fmod.dll"
