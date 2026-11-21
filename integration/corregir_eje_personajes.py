"""Correct character axes in the integrated Blender scene and resave it."""
import bpy
import math
import os
from mathutils import Quaternion

ROOT = '/Users/ur182/Desktop/proyectoGrafica'
OUTPUT = os.path.join(ROOT, 'integration', 'Escenario_Deportivo_Integrado.blend')
orientation = (Quaternion((0, 1, 0), math.pi / 2) @
               Quaternion((0, 0, 1), -math.pi / 2) @
               Quaternion((1, 0, 0), math.pi / 2))
for name in ('Control | jugador carrera', 'Control | árbitro'):
    root = bpy.data.objects.get(name)
    if root:
        root.rotation_mode = 'QUATERNION'
        root.rotation_quaternion = orientation

scene = bpy.context.scene
scene.frame_set(1)
for screen in bpy.data.screens:
    for area in screen.areas:
        for space in area.spaces:
            if space.type == 'VIEW_3D':
                space.region_3d.view_perspective = 'CAMERA'
                space.region_3d.view_camera_zoom = 0
bpy.context.preferences.filepaths.save_version = 0
bpy.ops.wm.save_as_mainfile(filepath=OUTPUT)
print('EJE DE PERSONAJES CORREGIDO: ' + OUTPUT)
