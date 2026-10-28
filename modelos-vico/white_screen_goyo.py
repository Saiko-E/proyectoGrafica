import bpy,numpy as np
from mathutils import Vector
from bpy_extras.object_utils import world_to_camera_view
s=bpy.context.scene;cd=bpy.data.cameras.new('ProjectionCam');cam=bpy.data.objects.new('ProjectionCam',cd);s.collection.objects.link(cam);cd.type='ORTHO';cd.ortho_scale=8;cam.location=(-7,-22,5.5);cam.rotation_euler=(Vector((-1.8,-3.2,3.8))-cam.location).to_track_quat('-Z','Y').to_euler();s.camera=cam
m=bpy.data.objects['MascotaPumasGoyo'].data;n=len(m.polygons);cent=np.empty((n,3),np.float32);m.polygons.foreach_get('center',cent.ravel());tot=np.empty(n,np.int32);m.polygons.foreach_get('loop_total',tot);lp=np.repeat(np.arange(n,dtype=np.int32),tot);a=m.color_attributes['GoyoColor'];r=np.empty(len(a.data)*4,np.float32);a.data.foreach_get('color',r);r=r.reshape(-1,4);fc=np.zeros((n,4),np.float32)
for j in range(4):fc[:,j]=np.bincount(lp,weights=r[:,j],minlength=n)/np.maximum(tot,1)
q=(fc[:,:3].min(1)>.8)&(cent[:,0]>-3.4)&(cent[:,0]<-.4)&(cent[:,1]<-.5)&(cent[:,1]>-5.8)&(cent[:,2]>2.5)&(cent[:,2]<5.8)
idx=np.where(q)[0]; coords=[]
for i in idx:
 v=world_to_camera_view(s,cam,Vector(cent[i]));coords.append((v.x,1-v.y,cent[i,0],cent[i,1],cent[i,2]))
a=np.array(coords)
print('n',len(a),'pixel-like range',a[:,:2].min(0),a[:,:2].max(0))
for lo,hi in [(0,.32),(.32,.4),(.4,.46),(.46,.52),(.52,.58),(.58,.65),(.65,1)]:
 zq=(a[:,1]>=lo)&(a[:,1]<hi)
 if zq.any(): print(lo,hi,'n',zq.sum(),'x/y/z quant',np.quantile(a[zq,2:],[0,.25,.5,.75,1],axis=0).round(3).tolist(),'screenx',np.quantile(a[zq,0],[0,.5,1]).round(3).tolist())
