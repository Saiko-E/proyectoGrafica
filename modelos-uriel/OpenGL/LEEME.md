# modelos-uriel: versión realista lista para OpenGL

Pipeline del proyecto: **Assimp 4.1 + SOIL2 + Model.h**. Mismas garantías que en `modelos-vico`: triángulos, normales y UV en todas las mallas, texturas JPG/PNG en potencias de 2, rutas `textures/...` sin espacios, y MTL sin opciones raras. Todo se validó cargándolo con Assimp.

| Carpeta | Archivo(s) | Qué cambió |
|---|---|---|
| `Porteria/` | `porteria.obj` (completa) · `porteria_marco.obj` + `porteria_red.obj` (por separado, mismas coordenadas) | Postes y larguero redondos de 12 cm (reglamento: 7.32 × 2.44 m) de aluminio blanco con marcas de balón y pasto, marco trasero delgado, anclas al piso y **red de cuerda real** (malla de 12 cm, geometría y no textura) con caída natural |
| `Iluminador/` | `iluminador.obj` | Base de concreto, placa con pernos de anclaje, poste cónico galvanizado con registro de acceso, caja de conexiones y pararrayos, y cabezal de **6 reflectores LED** con disipadores, copas reflectoras y yugos. Se quitaron los dos tubos diagonales, que no existen en un poste real |
| `Jugador/` | `jugador.obj` | Jersey verde de punto con estampado tonal y el 9, short blanco, medias acanaladas, tacos con pasto y piel con poros y variación de tono |
| `Arbitro/` | `arbitro.obj` | Jersey amarillo jaspeado, short y medias negras, silbato metálico, tarjetas y reloj |
| `Balon/` | `balon.obj` | PU granulado con raspones |

**Notas**
- La red de la portería es geometría real, unos 45 mil triángulos. Si en una escena con varias porterías se vuelve pesada, usen solo `porteria_marco.obj` para las lejanas.
- Jugador, árbitro y balón conservan su posición del `.blend` original (están uno al lado del otro). Para colocarlos en la cancha, apliquen su propia matriz `model`.
- Los archivos de `Animaciones/` no se modificaron: son versiones aparte hechas sobre los modelos base.
