import bpy
from mathutils import Vector
s=bpy.context.scene
camd=bpy.data.cameras.new('BallLegCamFinal');cam=bpy.data.objects.new('BallLegCamFinal',camd);s.collection.objects.link(cam);s.camera=cam;camd.type='ORTHO';camd.ortho_scale=8.0;cam.location=(-7,-22,5.5);cam.rotation_euler=(Vector((-1.8,-3.2,3.8))-cam.location).to_track_quat('-Z','Y').to_euler()
for name,loc,power,size in [('FinalLegKey',(-7,-12,10),2200,7),('FinalLegFill',(3,-9,4),1000,6)]:
 d=bpy.data.lights.new(name,'AREA');d.energy=power;d.shape='DISK';d.size=size;o=bpy.data.objects.new(name,d);s.collection.objects.link(o);o.location=loc;o.rotation_euler=(Vector((-1.8,-3.2,3.8))-o.location).to_track_quat('-Z','Y').to_euler()
s.render.resolution_x=700;s.render.resolution_y=900;s.render.resolution_percentage=70;s.render.filepath=bpy.path.abspath('//goyo_leg_final.png');bpy.ops.render.render(write_still=True)
