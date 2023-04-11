# GAM300

Things to note:

1) To generate solution, click on "GenerateProject.bat".
2) Everytime you edit a premake5.lua file, you need to run GenerateProject.bat to update the visual studio properties.
3) All .cpp source file requires to include "pch.h" at the top
4) This project setup includes pch header for all. If speific folders doesn't want pch, it needs to be specified in premake5.lua file in root folder specifically.
