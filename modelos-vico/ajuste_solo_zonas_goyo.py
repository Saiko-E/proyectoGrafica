import bpy, numpy as np
obj=bpy.data.objects['MascotaPumasGoyo'];mesh=obj.data;attr=mesh.color_attributes['GoyoColor']
loop_vid=np.empty(len(mesh.loops),np.int32);mesh.loops.foreach_get('vertex_index',loop_vid)
co=np.empty(len(mesh.vertices)*3,np.float32);mesh.vertices.foreach_get('co',co);p=co.reshape(-1,3)[loop_vid];x,y,z=p.T;ax=np.abs(x)
rgba=np.empty(len(attr.data)*4,np.float32);attr.data.foreach_get('color',rgba);colors=rgba.reshape(-1,4)
GOLD=(.72,.46,.12,1);BLUE=(.006,.055,.32,1);WHITE=(.99,.99,.99,1);BLACK=(.004,.005,.008,1);RED=(.62,.008,.018,1)
# Correct only blue spill on exposed arm skin; preserve blue sleeves and the ball-side leg.
raised_leg=(x>-3.3)&(x<-1.15)&(y<-1.4)&(z<6.25)
arm_zone=(ax>2.05)&(ax<5.05)&(z>5.1)&(z<8.68)&(y>-5.55)&(y<4.5)&(~raised_leg)
blue_spill=(colors[:,2]>colors[:,0]*1.25)&(colors[:,2]>colors[:,1]*1.35)&(colors[:,2]>.08)
colors[arm_zone&blue_spill]=GOLD
# Raised leg's shorts remain blue only above the sock.
short=(x>-2.85)&(x<-.15)&(y>-5.3)&(y<3.4)&(z>=5.38)&(z<6.22)
colors[short]=BLUE
# Set the sock band above the shoe, with a clean shared boundary.
sock=(x>-3.08)&(x<-1.34)&(y>-5.7)&(y<-1.55)&(z>=4.94)&(z<5.38)
colors[sock]=WHITE
shoe=(x>-3.28)&(x<-.45)&(y>-5.55)&(y<-.65)&(z>2.72)&(z<4.94)
colors[shoe]=BLACK

# Fit the sphere from its existing red panels, then recolor the full spherical shell.
nf=len(mesh.polygons);cent=np.empty((nf,3),np.float64);norm=np.empty_like(cent)
mesh.polygons.foreach_get('center',cent.ravel());mesh.polygons.foreach_get('normal',norm.ravel())
tot=np.empty(nf,np.int32);mesh.polygons.foreach_get('loop_total',tot)
loop_poly=np.repeat(np.arange(nf,dtype=np.int32),tot)
facecol=np.empty((nf,4),np.float32)
for ch in range(4): facecol[:,ch]=np.bincount(loop_poly,weights=rgba.reshape(-1,4)[:,ch],minlength=nf)/np.maximum(tot,1)
red_seed=(facecol[:,0]>.35)&(facecol[:,0]>facecol[:,1]*4)&(facecol[:,0]>facecol[:,2]*3)&(cent[:,0]<0)&(cent[:,1]<-1.7)&(cent[:,2]<4.5)
pts=cent[red_seed]
A=np.column_stack((2*pts,np.ones(len(pts))));rhs=np.einsum('ij,ij->i',pts,pts);sol=np.linalg.lstsq(A,rhs,rcond=None)[0];center=sol[:3];radius=float(np.sqrt(sol[3]+center@center))
rad=np.linalg.norm(cent-center,axis=1);err=np.abs(rad-radius)
ball=(err<.13)&(cent[:,0]>-3.5)&(cent[:,0]<-.05)&(cent[:,1]>-5.6)&(cent[:,1]<-1.7)&(cent[:,2]>.3)&(cent[:,2]<4.4)
colors[ball[loop_poly]]=RED
attr.data.foreach_set('color',colors.ravel());mesh.update()
bpy.ops.wm.save_as_mainfile(filepath=bpy.path.abspath('//Goyo_texturizado.blend'))
print('arm corrections',int((arm_zone&blue_spill).sum()),'short',int(short.sum()),'sock',int(sock.sum()),'shoe',int(shoe.sum()),'ball fit',center.round(3),round(radius,3),'ball faces',int(ball.sum()))
