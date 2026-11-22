# modelos-erick: versión realista lista para OpenGL

Pipeline del proyecto: **Assimp 4.1 + SOIL2 + Model.h**. Todo se validó cargándolo con Assimp, con las mismas garantías que las otras carpetas: triángulos, normales y UV en todas las mallas, texturas JPG/PNG en potencias de 2, rutas `textures/...` sin espacios, sin opciones raras en el MTL y **sin líneas ni puntos sueltos**.

## Autobus/ (compatible con `autobusAnimationMain.cpp`)
Mismos nombres que usa su programa: `carroceria.obj`, `llanta_1.obj` … `llanta_4obj.obj` … `llanta_6.obj`, y `ruedas.txt` (centros y radio iguales al original). Solo copien la carpeta encima de `Models/Autobus/`.
- **Carrocería:** bordes redondeados y pintura con laca. Las ventanas se comportan como vidrio polarizado con reflejo de cielo horneado, así se ven reales aun con el shader Phong. Tiene tierra de camino en la parte baja y espejos laterales tipo "oreja".
- **Llantas:** hule con dibujo y costado, y rin de aluminio con polvo de freno.

## Balon/
`balon.obj`: conserva su diseño con logo y le agrega granulado de PU, laca, raspones, manchas de pasto y costuras de hilo con grosor real.
- Se corrigieron dos fallas del original: la subdivisión estaba en nivel 6 (3.3 millones de triángulos), y las costuras eran aristas sueltas, que Assimp importa como líneas y que desordenan los índices de Model.h.

## Camara/
- `camara_completa.obj`: cámara de cine con su tripié, en reposo (el lente apunta a +X).
- `camara_tripie.obj` + `camara_cabezal.obj`: por separado para animar. El cabezal está centrado en su pivote; `pivote_camara.txt` trae la posición del pivote y la fórmula para que siga al balón (igual que el *Track To* del `.blend`).
- **Materiales:** cuerpo de magnesio mate (antes los metales tenían rugosidad 0, como espejos), patas de fibra de carbono, y el tally y la pantalla con su color de encendido.

## Marcador/
- `marcador.obj`: marcador completo en 0 – 0, reloj 00:00.
- `marcador_sin_numeros.obj` + `digitos/digito_0.obj` … `digito_9.obj`, `digito_dos_puntos.obj`: para mostrar marcador y reloj en tiempo real. `digitos/posiciones.txt` dice dónde y de qué tamaño dibujarlos. Los Geometry Nodes del `.blend` no se exportan a OBJ; esta es la forma de animarlos en OpenGL.
- **Materiales:** pantalla de matriz de LEDs real, marco de acero pintado, panel trasero de aluminio con nervaduras, postes de tubo cuadrado con cartabones sobre dados de concreto.
- Se quitaron los números duplicados que se encimaban (`Score_Local` repetía el reloj sobre el marcador local; `Score_Visita` y `Text.001/.002` eran "0" extra).

## Archivos `.blend`
Junto a los originales, con sufijo `_realista`. La cámara conserva su seguimiento al balón, y el marcador conserva sus Geometry Nodes animados, ahora con material LED.
