import bpy,numpy as np
m=bpy.data.objects['MascotaPumasGoyo'].data;a=m.color_attributes['GoyoColor'];vloop=np.empty(len(m.loops),np.int32);m.loops.foreach_get('vertex_index',vloop);co=np.empty(len(m.vertices)*3,np.float32);m.vertices.foreach_get('co',co);p=co.reshape(-1,3)[vloop];x,y,z=p.T;ax=np.abs(x);raw=np.empty(len(a.data)*4,np.float32);a.data.foreach_get('color',raw);colors=raw.reshape(-1,4)
GOLD=(.72,.46,.12,1);BLUE=(.006,.055,.32,1);WHITE=(.99,.99,.99,1);BLACK=(.004,.005,.008,1);RED=(.62,.008,.018,1)
raised=(x>-3.3)&(x<-1.15)&(y<-1.4)&(z<6.25);arms=(ax>2.05)&(ax<5.05)&(z>5.1)&(z<8.68)&(y>-5.55)&(y<4.5)&(~raised);blue=(colors[:,2]>colors[:,0]*1.25)&(colors[:,2]>colors[:,1]*1.35)&(colors[:,2]>.08);colors[arms&blue]=GOLD
short=(x>-2.85)&(x<-.15)&(y>-5.3)&(y<3.4)&(z>=5.38)&(z<6.22);colors[short]=BLUE
# Shoe black below the sock cuff; sock is the narrow white band above it.
sock=(x>-3.08)&(x<-1.34)&(y>-5.7)&(y<-1.55)&(z>=5.22)&(z<5.38);colors[sock]=WHITE
shoe=(x>-3.28)&(x<-.45)&(y>-5.55)&(y<-.65)&(z>2.72)&(z<5.22);colors[shoe]=BLACK
# Recolor all soccer-ball panels from their fitted spherical shell, preserving the red ball.
nf=len(m.polygons);cent=np.empty((nf,3),np.float64);norm=np.empty_like(cent);m.polygons.foreach_get('center',cent.ravel());m.polygons.foreach_get('normal',norm.ravel());tot=np.empty(nf,np.int32);m.polygons.foreach_get('loop_total',tot);lp=np.repeat(np.arange(nf,dtype=np.int32),tot)
fc=np.empty((nf,4),np.float32)
for j in range(4):fc[:,j]=np.bincount(lp,weights=raw.reshape(-1,4)[:,j],minlength=nf)/np.maximum(tot,1)
seed=(fc[:,0]>.35)&(fc[:,0]>fc[:,1]*4)&(fc[:,0]>fc[:,2]*3)&(cent[:,0]<0)&(cent[:,1]<-1.7)&(cent[:,2]<4.5);pts=cent[seed];A=np.column_stack((2*pts,np.ones(len(pts))));rhs=np.einsum('ij,ij->i',pts,pts);sol=np.linalg.lstsq(A,rhs,rcond=None)[0];ctr=sol[:3];rad=float(np.sqrt(sol[3]+ctr@ctr));rr=np.linalg.norm(cent-ctr,axis=1);ball=(np.abs(rr-rad)<.13)&(cent[:,0]>-3.5)&(cent[:,0]<-.05)&(cent[:,1]>-5.6)&(cent[:,1]<-1.7)&(cent[:,2]>.3)&(cent[:,2]<4.4);colors[ball[lp]]=RED
a.data.foreach_set('color',colors.ravel());m.update();bpy.ops.wm.save_as_mainfile(filepath=bpy.path.abspath('//Goyo_texturizado.blend'));print('boundary z=5.22, sock loops',int(sock.sum()),'shoe',int(shoe.sum()),'ball',int(ball.sum()))
