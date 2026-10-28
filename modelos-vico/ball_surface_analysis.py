import bpy,numpy as np
m=bpy.data.objects['MascotaPumasGoyo'].data;n=len(m.polygons);c=np.empty((n,3),np.float32);no=np.empty_like(c);m.polygons.foreach_get('center',c.ravel());m.polygons.foreach_get('normal',no.ravel());tot=np.empty(n,np.int32);m.polygons.foreach_get('loop_total',tot);lp=np.repeat(np.arange(n,dtype=np.int32),tot);a=m.color_attributes['GoyoColor'];r=np.empty(len(a.data)*4,np.float32);a.data.foreach_get('color',r);r=r.reshape(-1,4);fc=np.zeros((n,4),np.float32)
for k in range(4):fc[:,k]=np.bincount(lp,weights=r[:,k],minlength=n)/np.maximum(tot,1)
red=(fc[:,0]>fc[:,1]*4)&(fc[:,0]>fc[:,2]*3)&(c[:,0]<.2)&(c[:,1]<-1.7)&(c[:,2]<4.5)
black=(fc[:,:3].max(1)<.08)&(c[:,0]<.2)&(c[:,1]<-1.7)&(c[:,2]<4.5)
for center in [(-1.7,-3.93,2.4),(-1.7,-3.93,1.76),(-1.7,-3.6,2.4),(-1.7,-4.3,2.4)]:
 v=c-np.array(center,np.float32);rads=np.linalg.norm(v,axis=1);dot=np.einsum('ij,ij->i',no,v/np.maximum(rads[:,None],1e-6))
 print('center',center)
 for name,q in [('red',red),('black',black)]:
  print(name,'n',q.sum(),'radius',np.quantile(rads[q],[0,.1,.5,.9,1]).round(2),'dot',np.quantile(dot[q],[0,.1,.5,.9,1]).round(2),'mean',dot[q].mean().round(2))
