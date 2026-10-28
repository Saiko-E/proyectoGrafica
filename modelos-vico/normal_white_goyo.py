import bpy,numpy as np
m=bpy.data.objects['MascotaPumasGoyo'].data;n=len(m.polygons);c=np.empty((n,3),np.float32);no=np.empty_like(c);m.polygons.foreach_get('center',c.ravel());m.polygons.foreach_get('normal',no.ravel());tot=np.empty(n,np.int32);m.polygons.foreach_get('loop_total',tot);lp=np.repeat(np.arange(n,dtype=np.int32),tot);a=m.color_attributes['GoyoColor'];r=np.empty(len(a.data)*4,np.float32);a.data.foreach_get('color',r);r=r.reshape(-1,4);fc=np.zeros((n,4),np.float32)
for j in range(4):fc[:,j]=np.bincount(lp,weights=r[:,j],minlength=n)/np.maximum(tot,1)
q=(fc[:,:3].min(1)>.8)&(c[:,0]>-3.4)&(c[:,0]<-.4)&(c[:,1]<-.5)&(c[:,1]>-5.8)&(c[:,2]>4.4)&(c[:,2]<5.8)
a=np.abs(no[:,2]);
print('white faces',q.sum(),'normal z',np.quantile(a[q],[0,.1,.25,.5,.75,.9,1]).round(2))
for lo,hi in [(0,.2),(.2,.4),(.4,.6),(.6,.8),(.8,1.01)]:
 s=q&(a>=lo)&(a<hi)
 if s.any():print(lo,hi,int(s.sum()),'bounds',c[s].min(0).round(2),c[s].max(0).round(2),'mean',c[s].mean(0).round(2))
