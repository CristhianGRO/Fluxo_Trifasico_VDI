@echo off
setlocal

cd "%~dp0"
del ".\build\FP_VDI_TRI.exe" 2>nul
meson setup .\build --reconfigure
ninja -C .\build
.\build\FP_VDI_TRI.exe %*

endlocal