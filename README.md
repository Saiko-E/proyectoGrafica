# proyectoGrafica
Proyecto de Computación Gráfica.

## Escena deportiva integrada

La escena Blender que reúne los modelos del equipo está en [`integration/Escenario_Deportivo_Integrado.blend`](integration/Escenario_Deportivo_Integrado.blend). Las instrucciones para abrirla, reproducir la secuencia y regenerarla están en [`integration/README.md`](integration/README.md).

## Estadio interactivo en Windows

Instala Visual Studio 2022 con **Desarrollo para el escritorio con C++**, el
conjunto de herramientas MSVC v143 y el SDK de Windows. Instala
[vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-msbuild)
y ejecuta `vcpkg integrate install` una vez desde PowerShell.

Abre [La Pepo cumbia.sln](LabGrafica/La%20Pepo%20cumbia.sln) en Visual Studio,
selecciona **Release | x64** y presiona **F5**. En la primera compilación,
vcpkg instala GLEW, GLFW y GLM a partir de `LabGrafica/vcpkg.json`.
La solución ejecuta el programa desde la carpeta del proyecto para que
encuentre los modelos, las texturas y los shaders incluidos en el clon.
El programa requiere un controlador gráfico compatible con OpenGL 3.3.
