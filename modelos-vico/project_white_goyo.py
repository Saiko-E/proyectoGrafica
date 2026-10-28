import bpy, numpy as np
from mathutils import Vector
from bpy_extras.object_utils import world_to_camera_view
s=bpy.context.scene;cd=bpy.data.cameras.new('ProjectCam');cam=bpy.data.objects.new('ProjectCam',cd);s.collection.objects.link(cam);cd.type='ORTHO';cd.ortho_scale=8;cam.location=(-7,-22,5.5);cam.rotation_euler=(Vector((-1.8,-3.2,3.8))-cam.location).to_track_quat('-Z','Y').to_euler();s.camera=cam;s.render.resolution_x=700;s.render.resolution_y=900;s.render.resolution_percentage=70;bpy.context.view_layer.update()
print('target projection',world_to_camera_view(s,cam,Vector((-1.8,-3.2,3.8))))
print('frame',[(v.x,v.y,v.z) for v in cd.view_frame(scene=s)])
m=bpy.data.objects['MascotaPumasGoyo'].data;n=len(m.polygons);c=np.empty((n,3),np.float32);no=np.empty_like(c);m.polygons.foreach_get('center',c.ravel());m.polygons.foreach_get('normal',no.ravel());tot=np.empty(n,np.int32);m.polygons.foreach_get('loop_total',tot);lp=np.repeat(np.arange(n,dtype=np.int32),tot);a=m.color_attributes['GoyoColor'];r=np.empty(len(a.data)*4,np.float32);a.data.foreach_get('color',r);r=r.reshape(-1,4);fc=np.zeros((n,4),np.float32)
for j in range(4):fc[:,j]=np.bincount(lp,weights=r[:,j],minlength=n)/np.maximum(tot,1)
q=(fc[:,:3].min(1)>.8)&(c[:,0]>-3.4)&(c[:,0]<-.4)&(c[:,1]<-.5)&(c[:,1]>-5.8)&(c[:,2]>2.5)&(c[:,2]<5.8)
arr=[]
for i in np.where(q)[0]:
 v=world_to_camera_view(s,cam,Vector(c[i]));arr.append((v.x,v.y,c[i,0],c[i,1],c[i,2],abs(no[i,2])))
a=np.array(arr);print('n',len(a),'range',a[:,:2].min(0),a[:,:2].max(0))
for lo,hi in [(0,.4),(.4,.5),(.5,.55),(.55,.6),(.6,.65),(.65,.7),(.7,.8),(.8,1)]:
 sel=(a[:,1]>=lo)&(a[:,1]<hi)
 if sel.any():print(lo,hi,int(sel.sum()),'x y z quant',np.quantile(a[sel,2:5],[0,.25,.5,.75,1],axis=0).round(2).tolist(),'screenx',np.quantile(a[sel,0],[0,.5,1]).round(2).tolist(),'nz',np.quantile(a[sel,5],[0,.5,1]).round(2).tolist())
