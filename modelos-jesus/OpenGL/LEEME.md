# modelos-jesus: versión realista lista para OpenGL

Pipeline del proyecto: **Assimp 4.1 + SOIL2 + Model.h**. Todo se validó cargándolo con Assimp, con las mismas garantías que las otras carpetas: triángulos, normales y UV en todas las mallas, texturas JPG/PNG en potencias de 2, rutas `textures/...` sin espacios, sin opciones raras en el MTL y sin líneas ni puntos sueltos.

## Estadio/ — `estadio.obj`
Por su tamaño (240 × 190 m) usa **texturas repetibles en escala real** en lugar de un atlas horneado, así se ve nítido aunque la cámara esté en la cancha:
- **Pasto:** franjas de corte y desgaste en las áreas chicas y el círculo central (una textura de 4096 × 2048 para todo el terreno).
- **Gradas:** butacas azules en las huellas y respaldos en los peraltes que miran a la cancha, alineados con cada fila.
- **Superficies:** concreto con juntas de cimbra y orificios de moños, adoquín de 60 cm afuera, membrana del techo con costuras, acero galvanizado y franja roja pintada.
- **Parpadeo corregido:** las líneas de cal estaban 5 mm sobre el pasto y el piso exterior 2 cm debajo; ahora hay 3 cm y 8 cm de separación.
- **⚠ Requisito:** las texturas se repiten, así que `Model.h` debe usar `GL_REPEAT` (es lo normal en la versión de LearnOpenGL: `glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S/T, GL_REPEAT)`). Si su versión usa `GL_CLAMP_TO_EDGE`, cámbienlo a `GL_REPEAT`.

## Trofeo/ — `trofeo.obj`
Copa torneada con nudo en el tallo y asas curvas, oro pulido con huellas, base de mármol negro con vetas en dos niveles y banda dorada, y placa de latón grabada ("CAMPEON", "TEMPORADA 2026").

## Podio/ — `podio.obj`
Misma forma y textos. Superficies superiores de alfombra con fibra y pisadas, costados de laca azul con capa transparente, faldón de tela con pliegues, molduras de oro cepillado y letras de vinil. El piso de estudio de 400 × 400 m no se exporta.

## CarritoEmergencias/
- `carrito_emergencias_completo.obj`: el carrito entero, listo para colocar.
- `carrito_emergencias_cuerpo.obj` + 4 `rueda_*.obj`: para animarlo. Las posiciones de las ruedas están en `pivotes_ruedas.txt` (radio 0.22).
- **Modelo:** carrocería de fibra de vidrio blanca con franja y cruces rojas, techo, postes de aluminio, parabrisas de vidrio (material con `d 0.35`), asiento de vinil, volante, torreta roja/azul, faros cromados, calaveras y camilla rígida naranja con cinturones.
- **Detalle técnico:** cada cruz es una sola pieza, porque dos barras encimadas en el mismo plano darían *z-fighting* en OpenGL.

## Archivos `.blend`
Junto a los originales, con sufijo `_realista`. Los originales no se modificaron.
