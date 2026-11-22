"""Fix the integrated scene framing and place a flag at each field corner."""
import bpy
import os
import math
from mathutils import Vector

ROOT = '/Users/ur182/Desktop/proyectoGrafica'
OUTPUT = os.path.join(ROOT, 'integration', 'Escenario_Deportivo_Integrado.blend')
scene = bpy.context.scene

# Place one corner flag at each field corner (field is 154 x 104 units).
flag_root = bpy.data.objects.get('Control | esquina')
if flag_root:
    flag_root.location = (-75.0, -50.0, 0.0)
    flag_root.rotation_euler.z = 0.0

source_path = os.path.join(ROOT, 'modelos-vico', 'tiro de equina.blend')
for index, pos in enumerate(((75.0, -50.0, 0.0), (-75.0, 50.0, 0.0), (75.0, 50.0, 0.0)), 2):
    if bpy.data.objects.get('Control | esquina %d' % index):
        continue
    col = bpy.data.collections.new('09.%d | banderín esquina' % index)
    scene.collection.children.link(col)
    with bpy.data.libraries.load(source_path, link=False) as (src, dst):
        dst.objects = list(src.objects)
    objects = [obj for obj in dst.objects if obj]
    valid = []
    for obj in objects:
        if obj.type in {'CAMERA', 'LIGHT'}:
            bpy.data.objects.remove(obj, do_unlink=True)
        else:
            col.objects.link(obj)
            valid.append(obj)
    objects = valid
    root = bpy.data.objects.new('Control | esquina %d' % index, None)
    col.objects.link(root)
    root.empty_display_type = 'SPHERE'
    root.empty_display_size = 0.08
    root.hide_select = True
    members = set(objects)
    for obj in objects:
        if obj.parent not in members:
            world = obj.matrix_world.copy()
            obj.parent = root
            obj.matrix_parent_inverse = root.matrix_world.inverted()
            obj.matrix_world = world
    root.location = pos

# Keep the player and referee over the playable grass and at readable scale.
player_root = bpy.data.objects.get('Control | jugador carrera')
ref_root = bpy.data.objects.get('Control | árbitro')
if player_root:
    player_root.scale = (1.0, 1.0, 1.0)
if ref_root:
    ref_root.scale = (1.0, 1.0, 1.0)
    # Bring the official closer to the play so both characters read in the
    # opening shot, while keeping him beside the runner's path.
    ref_root.location = (-58.0, -2.0, 0.0)
for root in (player_root, ref_root):
    if root:
        root.empty_display_type = 'SPHERE'
        root.empty_display_size = 0.08
        root.hide_select = True
for obj in scene.objects:
    if obj.type == 'ARMATURE' and (
        obj.name.startswith('Arbitro') or obj.name.startswith('Jugador')
    ):
        # Keep the rigs active for deformation, but hide their bone sticks in
        # the viewport so they do not look like black poles through the models.
        obj.hide_set(True)

# Hold the referee's signal after its 16-frame source action so it is still
# visible during the close camera shot.
for obj in scene.objects:
    ad = obj.animation_data
    if not ad:
        continue
    for track in ad.nla_tracks:
        for strip in track.strips:
            if strip.action and ('Arbitro' in strip.action.name or 'árbitro' in strip.action.name):
                try:
                    strip.extrapolation = 'HOLD_FORWARD'
                    # The source whistle signal lasts only 16 frames. Stretch
                    # it so the raised arm is readable during the close shot.
                    if abs(strip.frame_start - 178.0) < 0.01:
                        strip.frame_start = 174.0
                        strip.scale = 4.0
                        strip.frame_end = strip.frame_start + (
                            strip.action_frame_end - strip.action_frame_start
                        ) * strip.scale
                except Exception:
                    pass

for marker in scene.timeline_markers:
    if 'silbato' in marker.name.lower() or 'señal' in marker.name.lower():
        marker.frame = 174

camera = scene.camera or bpy.data.objects.get('Cámara | recorrido cinematográfico')
if not camera:
    raise RuntimeError('No se encontró la cámara cinematográfica')
scene.camera = camera
camera.animation_data_clear()
camera.data.lens = 58
camera.data.clip_end = 600

def key_camera(frame, location, target):
    scene.frame_set(frame)
    camera.location = location
    direction = Vector(target) - camera.location
    camera.rotation_euler = direction.to_track_quat('-Z', 'Y').to_euler()
    camera.keyframe_insert(data_path='location', frame=frame)
    camera.keyframe_insert(data_path='rotation_euler', frame=frame)

# Start on the two characters; open out briefly to establish the venue, then
# follow the run, the ball into the goal, and the referee's call.
key_camera(1, (-36, -25, 9), (-52, -5, 1.5))
key_camera(35, (130, -185, 125), (0, 0, 8))
key_camera(55, (-30, -17, 9), (-48, -8, 1.5))
key_camera(126, (-66, -12, 9), (-76, 0, 1.6))
key_camera(155, (-65, -10, 8), (-75, 0, 1.6))
key_camera(178, (-47, -11, 8), (-58, -2, 1.7))
key_camera(210, (-47, -11, 8), (-58, -2, 1.7))
key_camera(250, (82, -132, 94), (0, 0, 8))
key_camera(300, (110, -170, 112), (0, 0, 8))
key_camera(420, (130, -185, 125), (0, 0, 8))

scene.frame_start = 1
scene.frame_end = 420
scene.frame_set(1)
for screen in bpy.data.screens:
    for area in screen.areas:
        for space in area.spaces:
            if space.type == 'VIEW_3D':
                space.region_3d.view_perspective = 'CAMERA'
                space.region_3d.view_camera_zoom = 0
                space.overlay.show_extras = False
                space.overlay.show_relationship_lines = False

# Avoid a .blend1 backup when updating the generated deliverable.
bpy.context.preferences.filepaths.save_version = 0
bpy.ops.wm.save_as_mainfile(filepath=OUTPUT)
with open(os.path.join(ROOT, 'integration', 'RESUMEN_INTEGRACION.txt'), 'a', encoding='utf-8') as f:
    f.write('\nCorrección visual: cámara inicial enfocada a personajes; encuadres de carrera, gol y árbitro; cuatro banderines, uno en cada esquina de cancha.\n')
print('CORRECCION GUARDADA: ' + OUTPUT)
