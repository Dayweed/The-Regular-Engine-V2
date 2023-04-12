# GAM300

Required things to have installed:
- Vulkan (https://vulkan.lunarg.com/sdk/home#windows)

Steps to do when pull:
1) Run "GenerateProject.bat".
2) Everytime you edit a premake5.lua file, run GenerateProject.bat to update.
3) All .cpp source file requires to include "pch.h" at the top.
4) Speific .cpp that doesn't want pch need to specify in premake5.lua file in root folder.

How to add libraries:

1) Open Dependencies.lua and there are 3 array-like variables (IncludeDir, LibraryDir, Library)
    - IncludeDir is the path you want to include for that lib
    - LibraryDir is the path where you lib is located
    - Library is the .lib itself
2) After specifiying those 3 variables, open premake5.lua in root folder.
3) Under "includedirs" in the project you want, add the include path following how it's done.
4) Under "links" in the project you want, add the library following how it's done.

