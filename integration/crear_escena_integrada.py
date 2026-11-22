"""Build a unified, keyframed Blender scene from the team's source projects.

Run inside Blender 5.2 from the Python Console:
    exec(compile(open('/absolute/path/integration/crear_escena_integrada.py').read(), 'crear_escena_integrada.py', 'exec'))

All source .blend files remain unchanged. The output is a new project file.
"""
import bpy
import math
import os
from mathutils import Vector, Euler, Quaternion

ROOT = '/Users/ur182/Desktop/proyectoGrafica'
OUT = os.path.join(ROOT, 'integration', 'Escenario_Deportivo_Integrado.blend')
TEMP_CURRENT = '/tmp/proyectoGrafica_sesion_blender.blend'

# Save a temporary copy of the open file first. This also lets the script
# append trofeo.blend when that happens to be the file currently open.
bpy.ops.wm.save_as_mainfile(filepath=TEMP_CURRENT)

# Start with a clean scene while keeping Blender's current UI session alive.
for obj in list(bpy.data.objects):
    bpy.data.objects.remove(obj, do_unlink=True)
for scene in bpy.data.scenes:
    for col in list(scene.collection.children):
        scene.collection.children.unlink(col)
        if col.users == 0:
            bpy.data.collections.remove(col)
for datablocks in (bpy.data.cameras, bpy.data.lights):
    for datablock in list(datablocks):
        if datablock.users == 0:
            datablocks.remove(datablock)

scene = bpy.context.scene
scene.name = 'Partido | gol y señal arbitral'
scene.render.engine = 'CYCLES'
scene.cycles.samples = 24
scene.render.resolution_x = 1920
scene.render.resolution_y = 1080
scene.render.resolution_percentage = 100
scene.render.fps = 24
scene.frame_start = 1
scene.frame_end = 420
scene.render.image_settings.file_format = 'PNG'
scene.render.film_transparent = False
scene.render.filepath = '//renders/partido_'
scene.view_settings.view_transform = 'AgX'

def collection(name):
    col = bpy.data.collections.new(name)
    scene.collection.children.link(col)
    return col

def append_asset(label, relative_path, include=None):
    """Append objects from a source blend and place them in a named collection."""
    path = os.path.join(ROOT, relative_path)
    if not os.path.isfile(path):
        raise FileNotFoundError(path)
    col = collection(label)
    with bpy.data.libraries.load(path, link=False) as (src, dst):
        names = [n for n in src.objects if include is None or include(n)]
        dst.objects = names
    objs = [obj for obj in dst.objects if obj is not None]
    valid = []
    for obj in objs:
        # Remove source cameras/lights so the final project has one intentional
        # camera and its own stadium lighting.
        if obj.type in {'CAMERA', 'LIGHT'}:
            bpy.data.objects.remove(obj, do_unlink=True)
            continue
        col.objects.link(obj)
        valid.append(obj)
    return col, valid

def create_root(name, col, objects, location=(0, 0, 0), rotation=(0, 0, 0), scale=1.0):
    root = bpy.data.objects.new(name, None)
    col.objects.link(root)
    root.empty_display_type = 'PLAIN_AXES'
    root.empty_display_size = 2.0
    members = set(objects)
    top_level = [obj for obj in objects if obj.parent not in members]
    for obj in top_level:
        world = obj.matrix_world.copy()
        obj.parent = root
        obj.matrix_parent_inverse = root.matrix_world.inverted()
        obj.matrix_world = world
    root.location = location
    root.rotation_euler = rotation
    root.scale = (scale, scale, scale)
    return root

def shift_actions(objects, start, name_filter=None):
    shifted = []
    for obj in objects:
        ad = obj.animation_data
        if not ad or not ad.action:
            continue
        action = ad.action
        if name_filter and not name_filter(obj, action):
            continue
        bounds = action.frame_range
        if bounds[1] <= bounds[0]:
            ad.action = None
            continue
        ad.action = None
        track = ad.nla_tracks.new()
        track.name = action.name + ' | secuencia'
        strip = track.strips.new(action.name, start, action)
        strip.action_frame_start = bounds[0]
        strip.action_frame_end = bounds[1]
        strip.blend_type = 'REPLACE'
        strip.extrapolation = 'NOTHING'
        if action.name.startswith('Jugador CHICHARITO Carrera'):
            strip.repeat = 4.25
        shifted.append((obj.name, action.name, int(bounds[0]), int(bounds[1]), int(start)))
    return shifted

def aim_at(obj, point):
    direction = Vector(point) - obj.location
    obj.rotation_euler = direction.to_track_quat('-Z', 'Y').to_euler()

def key_camera(frame, location, target):
    camera.location = location
    aim_at(camera, target)
    camera.keyframe_insert(data_path='location', frame=frame)
    camera.keyframe_insert(data_path='rotation_euler', frame=frame)

def key_linear(obj):
    if not obj.animation_data or not obj.animation_data.action:
        return
    # Blender 5.2 stores keyframes in layered Actions; older builds expose
    # action.fcurves directly. Leave default Bezier interpolation if the
    # legacy collection is unavailable.
    for fc in getattr(obj.animation_data.action, 'fcurves', []):
        for key in fc.keyframe_points:
            key.interpolation = 'BEZIER'

# Stadium and field (stadium source is authored in Z-up world units).
stadium_col, stadium = append_asset('01 | estadio y cancha', 'modelos-jesus/estadio.blend')

# Animated goal and ball. The goal's opening is turned toward the center of the
# field; the source ball action remains on its original object.
goal_col, goal = append_asset('02 | portería y gol', 'modelos-uriel/Animaciones/Porteria_Gol_Animada.blend')
goal_root = create_root('Control | portería gol', goal_col, goal,
                        location=(-77.0, 3.66, 0.0), rotation=(0.0, 0.0, -math.pi / 2))
goal_actions = shift_actions(goal, 126)
# The five source planes are the net panels. Give them a brief, damped push at
# the ball impact so the goal reads clearly even though those source actions
# were not assigned to the planes in their original file.
for obj in goal:
    if obj.name.startswith('Plane'):
        base = obj.location.copy()
        obj.location = base
        obj.keyframe_insert(data_path='location', frame=120)
        obj.location = base + Vector((0.0, 0.24, 0.0))
        obj.keyframe_insert(data_path='location', frame=134)
        obj.location = base + Vector((0.0, 0.10, 0.0))
        obj.keyframe_insert(data_path='location', frame=150)
        obj.location = base
        obj.keyframe_insert(data_path='location', frame=184)

# Character source is Y-up. Rotate its complete hierarchy to the stadium's Z-up
# coordinates; the referee and player keep their own armature/action hierarchy.
actor_filter = lambda n: n.startswith('Jugador CHICHARITO') or n.startswith('Arbitro')
actors_col, actors = append_asset('03 | jugador y árbitro',
                                  'modelos-uriel/Animaciones/Jugador_Arbitro_Animaciones.blend',
                                  actor_filter)
player_objs = [o for o in actors if o.name.startswith('Jugador CHICHARITO')]
ref_objs = [o for o in actors if o.name.startswith('Arbitro')]
character_rotation = (math.pi / 2, 0.0, -math.pi / 2)
player_root = create_root('Control | jugador carrera', actors_col, player_objs,
                          location=(-46.0, -8.0, 0.0), rotation=character_rotation)
ref_root = create_root('Control | árbitro', actors_col, ref_objs,
                       location=(-65.0, 9.0, 0.0), rotation=character_rotation)
# Compose rotations as quaternions: first convert the source Y-up axis to Z-up,
# then yaw toward the goal. Euler XYZ applies in the opposite order here and
# incorrectly turns the characters onto their sides.
character_orientation = (Quaternion((0, 1, 0), math.pi / 2) @
                        Quaternion((0, 0, 1), -math.pi / 2) @
                        Quaternion((1, 0, 0), math.pi / 2))
for root in (player_root, ref_root):
    root.rotation_mode = 'QUATERNION'
    root.rotation_quaternion = character_orientation
actor_actions = []
actor_actions += shift_actions(player_objs, 55)
actor_actions += shift_actions(ref_objs, 178)

# Character run spans the approach to the goal; the armature action supplies
# the leg/arm cycle while this parent motion moves the player along the pitch.
player_root.location = (-46.0, -8.0, 0.0)
player_root.keyframe_insert(data_path='location', frame=1)
player_root.keyframe_insert(data_path='location', frame=55)
player_root.location = (-64.0, -8.0, 0.0)
player_root.keyframe_insert(data_path='location', frame=126)
player_root.keyframe_insert(data_path='location', frame=180)
key_linear(player_root)

# Stadium floodlight model, placed around the outer corners. Animated lighting
# is created with real Blender spot lights and a controlled fade to night.
floodlight_objects = []
for i, pos in enumerate(((-86, -58, 0), (-86, 58, 0), (86, -58, 0), (86, 58, 0)), 1):
    col, objs = append_asset('04.%d | torre de iluminación' % i,
                            'modelos-uriel/Animaciones/Iluminador_Estadio_Animado.blend')
    create_root('Control | torre %d' % i, col, objs, location=pos, scale=3.5)
    floodlight_objects += objs

# Scoreboard, reserve bench, Cruz Azul team bus and mascot.
score_col, scoreboard = append_asset('05 | marcador', 'modelos-erick/scoreboard.blend')
create_root('Control | marcador', score_col, scoreboard, location=(0, 53, 2))

bench_filter = lambda n: n not in {'Piso de estudio', 'SurfPatch'}
for i, pos in enumerate(((-18, 55, 0), (18, 55, 0)), 1):
    col, objs = append_asset('06.%d | banca técnica' % i,
                            'modelos-vico/Banca_Dia_Noche.blend', bench_filter)
    create_root('Control | banca %d' % i, col, objs, location=pos)

bus_col, bus_objs = append_asset('07 | autobús del equipo', 'modelos-erick/autobus.blend')
bus_root = create_root('Control | autobús', bus_col, bus_objs, location=(0, -108, 0))
bus_actions = shift_actions(bus_objs, 255)
bus_root.keyframe_insert(data_path='location', frame=1)
bus_root.keyframe_insert(data_path='location', frame=255)
bus_root.location.x = 28
bus_root.keyframe_insert(data_path='location', frame=345)
key_linear(bus_root)

goyo_col, goyo = append_asset('08 | mascota Goyo', 'modelos-vico/Goyo.blend')
create_root('Control | mascota', goyo_col, goyo, location=(26, -18, 0))

for i, pos in enumerate(((-75, -50, 0), (75, -50, 0), (-75, 50, 0), (75, 50, 0)), 1):
    corner_col, corner = append_asset('09.%d | banderín esquina' % i,
                                      'modelos-vico/tiro de equina.blend')
    create_root('Control | esquina %d' % i, corner_col, corner, location=pos)

# Service and award areas sit outside the playing surface but inside the venue.
cart_col, cart = append_asset('10 | carrito de emergencias', 'modelos-jesus/carrito_emergencias.blend')
create_root('Control | emergencias', cart_col, cart, location=(66, -70, 0))

food_col, food = append_asset('11 | carrito de alimentos', 'modelos-vico/Carrito.blend')
food_root = create_root('Control | carrito de alimentos', food_col, food, location=(-74, -73, 0))
food_actions = shift_actions(food, 245)

podium_col, podium = append_asset('12 | podio de premiación', 'modelos-jesus/podio.blend',
                                  lambda n: n != 'Pedestal de suelo')
create_root('Control | podio', podium_col, podium, location=(90, -80, 0), scale=0.45)

# The open file at script start may itself be trofeo.blend. Its temporary copy
# above frees the original path, so it is safe to append the source now.
trophy_col, trophy = append_asset('13 | trofeo', 'modelos-jesus/trofeo.blend')
create_root('Control | trofeo', trophy_col, trophy, location=(90, -80, 0), scale=0.45)

# An exterior presentation zone with a neutral floor for the podium and service
# vehicles; stadium geometry still defines the complete main venue.
ground_mat = bpy.data.materials.new('Pavimento | grafito')
ground_mat.diffuse_color = (0.035, 0.055, 0.075, 1)
ground_mesh = bpy.data.meshes.new('Pavimento exterior | malla')
half = 170.0
ground_mesh.from_pydata([(-half, -half, 0), (half, -half, 0),
                         (half, half, 0), (-half, half, 0)], [], [(0, 1, 2, 3)])
ground_mesh.update()
ground = bpy.data.objects.new('Pavimento exterior | base de escena', ground_mesh)
scene.collection.objects.link(ground)
ground.location.z = -0.12
ground.name = 'Pavimento exterior | base de escena'
ground.data.materials.append(ground_mat)

# Camera path: stadium overview -> approach -> goal -> referee -> night-wide.
cam_data = bpy.data.cameras.new('Cámara | transmisión')
camera = bpy.data.objects.new('Cámara | recorrido cinematográfico', cam_data)
scene.collection.objects.link(camera)
scene.camera = camera
camera.data.lens = 38
camera.data.clip_end = 600
key_camera(1, (158, -210, 148), (0, 0, 8))
key_camera(65, (10, -50, 20), (-45, -8, 1))
key_camera(140, (-55, -35, 15), (-75, 0, 2))
key_camera(205, (-59, 12, 8), (-65, 9, 2))
key_camera(290, (80, -130, 90), (0, 0, 9))
key_camera(420, (158, -210, 148), (0, 0, 8))
camera.data.lens = 52
key_linear(camera)

# Daylight and stadium spots fade on after the referee's call.
world = bpy.data.worlds.new('Ambiente | noche de partido') if not bpy.data.worlds else bpy.data.worlds[0]
scene.world = world
world.use_nodes = True
bg = world.node_tree.nodes.get('Background')
bg.inputs['Color'].default_value = (0.22, 0.28, 0.37, 1)
bg.inputs['Strength'].default_value = 0.35

sun_data = bpy.data.lights.new('Sol | luz de tarde', 'SUN')
sun = bpy.data.objects.new('Sol | luz de tarde', sun_data)
scene.collection.objects.link(sun)
sun.rotation_euler = (math.radians(25), math.radians(-20), math.radians(-25))
sun_data.energy = 2.0
sun_data.keyframe_insert(data_path='energy', frame=1)
sun_data.energy = 1.25
sun_data.keyframe_insert(data_path='energy', frame=245)
sun_data.energy = 0.0
sun_data.keyframe_insert(data_path='energy', frame=290)
key_linear(sun)

for i, pos in enumerate(((-84, -55, 22), (-84, 55, 22), (84, -55, 22), (84, 55, 22)), 1):
    light_data = bpy.data.lights.new('Reflector %d | encendido gradual' % i, 'SPOT')
    light = bpy.data.objects.new('Reflector %d | encendido gradual' % i, light_data)
    scene.collection.objects.link(light)
    light.location = pos
    aim_at(light, (0, 0, 0))
    light_data.energy = 0
    light_data.spot_size = math.radians(82)
    light_data.spot_blend = 0.55
    light_data.shadow_soft_size = 6
    light_data.keyframe_insert(data_path='energy', frame=1)
    light_data.keyframe_insert(data_path='energy', frame=250)
    light_data.energy = 45000
    light_data.keyframe_insert(data_path='energy', frame=300)
    key_linear(light)

# Organize sequence markers for easy scrubbing in the timeline.
for frame, name in ((1, '01 | Estadio'), (55, '02 | Carrera'), (126, '03 | Gol'),
                    (178, '04 | Silbato y señal'), (250, '05 | Se encienden las luces'),
                    (420, '06 | Cierre')):
    scene.timeline_markers.new(name, frame=frame)

# Make imported images self-contained in the integrated file.
try:
    bpy.ops.file.pack_all()
except Exception:
    pass

scene.frame_set(1)
bpy.context.view_layer.update()
for area in bpy.context.screen.areas if bpy.context.screen else []:
    if area.type == 'VIEW_3D':
        area.spaces.active.region_3d.view_perspective = 'CAMERA'

os.makedirs(os.path.dirname(OUT), exist_ok=True)
bpy.ops.wm.save_as_mainfile(filepath=OUT)

report = os.path.join(ROOT, 'integration', 'RESUMEN_INTEGRACION.txt')
with open(report, 'w', encoding='utf-8') as f:
    f.write('Escena integrada generada: %s\n' % OUT)
    f.write('Rango: 1-420, 24 fps. Marcadores: estadio, carrera, gol, señal, luces, cierre.\n')
    f.write('Acciones desplazadas a NLA:\n')
    for row in goal_actions + actor_actions + bus_actions + food_actions:
        f.write('  %s | %s | origen %s-%s | inicio %s\n' % row)
    f.write('Recursos importados: estadio, portería, jugador/árbitro, reflectores, marcador, bancas, autobús, Goyo, esquina, carritos, podio y trofeo.\n')
print('ESCENA INTEGRADA GUARDADA: ' + OUT)
