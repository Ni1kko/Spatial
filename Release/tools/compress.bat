@echo off

REM https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/for
FOR /F "delims=" %%i IN ("%~1") DO (
	binarytocompressed.exe -base85 "%~1" %%~ni > %%~ni.cpp 
)