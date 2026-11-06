@echo off
setlocal
cd /d "%~dp0"
where g++ >nul 2>nul
if errorlevel 1 (
  echo No se encontro g++. Instala MSYS2 MinGW-w64 y agrega mingw64\bin al PATH.
  pause
  exit /b 1
)
echo Compilando GoyoDance.cpp...
g++ -std=c++17 -O2 -Wall -Wextra -mwindows GoyoDance.cpp -o GoyoDance.exe -lopengl32 -lgdi32 -lwinmm
if errorlevel 1 (
  echo La compilacion fallo.
  pause
  exit /b 1
)
echo Listo. Abriendo el baile de Goyo...
GoyoDance.exe


