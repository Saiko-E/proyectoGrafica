# Integración de la escena deportiva

## Archivos

- `Escenario_Deportivo_Integrado.blend`: escena unificada con estadio, cancha, jugador, árbitro, gol, luces, marcador, bancas y modelos de apoyo. Las imágenes vinculadas se empaquetaron en el `.blend`.
- `crear_escena_integrada.py`: script de Blender que arma la escena desde los `.blend` originales. No modifica las fuentes.
- `RESUMEN_INTEGRACION.txt`: acciones encontradas y sus posiciones en la línea de tiempo.

## Abrir y reproducir

1. Abre `Escenario_Deportivo_Integrado.blend` en Blender 5.2.
2. La escena abre en la vista de cámara, con el jugador y el árbitro encuadrados sobre la cancha.
3. Pulsa **Espacio** para reproducir o arrastra el cabezal de la línea de tiempo.
4. Los marcadores sobre la línea de tiempo señalan los momentos principales.

| Fotograma | Momento |
|---:|---|
| 1 | Jugador y árbitro en cancha |
| 55 | Carrera del jugador |
| 126 | Entrada del balón y reacción de la red |
| 174–234 | Silbato y señal del árbitro (acción ralentizada para que se distinga) |
| 250–300 | Cambio de luz de tarde a reflectores |
| 420 | Cierre de la cámara |

## Estructura de la escena

Las colecciones están numeradas por zonas y cada grupo tiene un objeto `Control | ...` para moverlo completo. Los dos personajes conservan sus jerarquías de huesos y quedan erguidos en el eje vertical del estadio. Los armatures siguen deformando los modelos, pero sus varillas están ocultas en la vista para no atravesar visualmente los personajes. El árbitro se acerca a la jugada para aparecer junto al jugador en el primer plano; después, la cámara encuadra su señal. Hay un banderín en cada una de las cuatro esquinas de la cancha. El jugador corre hacia el área; el balón conserva su acción de gol y la red recibe un pequeño impulso con claves adicionales. La cámara recorre la cancha y vuelve al plano general.

El foco nocturno se anima con luces Spot de Blender. Algunas acciones de iluminación y red estaban en los `.blend` de origen como acciones sin asignación a objetos; por eso esos efectos se animan en el archivo integrado con fotogramas propios.

## Regenerar

Abre Blender con el repositorio disponible en la ruta usada en el encabezado de los scripts. En el espacio de trabajo, abre la consola Python con **Shift + F4** y ejecuta primero:

```python
exec(compile(open('/Users/ur182/Desktop/proyectoGrafica/integration/crear_escena_integrada.py').read(), 'crear_escena_integrada.py', 'exec'))
```

Después ejecuta `ajustar_encuadre_y_esquinas.py` y `corregir_eje_personajes.py` del mismo modo para aplicar el encuadre, las cuatro esquinas y la orientación final de los personajes.

El script guarda la nueva escena en esta carpeta de integración. Conserva intactos los archivos fuente del equipo.
