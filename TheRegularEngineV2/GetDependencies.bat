@echo OFF
setlocal enabledelayedexpansion
set MAIN_FOLDER="%cd%"

:DOWNLOAD_DEPENDENCIES
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
powershell write-host -fore White DOWNLOADING ASSIMP
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
echo.

rem ------------------------------------------------------------
rem ASSIMP
rem ------------------------------------------------------------
:ASSIMP
if exist "./Dependencies/assimp" (
	powershell write-host -fore White ------------------------------------------------------------------------------------------------------
	powershell write-host -fore White ASSIMP FOLDER EXIST, SKIPPING ASSIMP
	powershell write-host -fore White ------------------------------------------------------------------------------------------------------
echo.
) else (
	rmdir "./Dependencies/assimp" /S /Q
	git clone --branch v5.2.4 https://github.com/assimp/assimp.git "../dependencies/assimp"
	if %ERRORLEVEL% GEQ 1 goto :ERROR

	cd ../dependencies/assimp
	if %ERRORLEVEL% GEQ 1 goto :ERROR

	cmake CMakeLists.txt -S . -B ./BINARIES/Win32
	if %ERRORLEVEL% GEQ 1 goto :ERROR

	cmake --build ./BINARIES/Win32 --config release
	if %ERRORLEVEL% GEQ 1 goto :ERROR

	cd /d %MAIN_FOLDER%
	if %ERRORLEVEL% GEQ 1 goto :ERROR
)

:DONE
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
powershell write-host -fore White DONE!!
powershell write-host -fore White ------------------------------------------------------------------------------------------------------
goto :PAUSE

:ERROR
powershell write-host -fore Red ------------------------------------------------------------------------------------------------------
powershell write-host -fore Red DONE WITH ERRORS!!
powershell write-host -fore Red ------------------------------------------------------------------------------------------------------

:PAUSE
rem if no one give us any parameters then we will pause it at the end, else we are assuming that another batch file called us
if %1.==. pause
