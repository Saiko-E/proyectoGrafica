import bpy,numpy as np
m=bpy.data.objects['MascotaPumasGoyo'].data;n=len(m.polygons);c=np.empty((n,3),np.float32);m.polygons.foreach_get('center',c.ravel());tot=np.empty(n,np.int32);m.polygons.foreach_get('loop_total',tot);lp=np.repeat(np.arange(n,dtype=np.int32),tot);a=m.color_attributes['GoyoColor'];r=np.empty(len(a.data)*4,np.float32);a.data.foreach_get('color',r);r=r.reshape(-1,4);fc=np.zeros((n,4),np.float32)
for j in range(4):fc[:,j]=np.bincount(lp,weights=r[:,j],minlength=n)/np.maximum(tot,1)
q=(fc[:,:3].min(1)>.8)&(c[:,0]>-3.4)&(c[:,0]<-.4)&(c[:,1]<-.5)&(c[:,1]>-5.8)&(c[:,2]>2.5)&(c[:,2]<5.8)
print('white n',q.sum())
for lo in np.arange(2.5,5.81,.2):
 s=q&(c[:,2]>=lo)&(c[:,2]<lo+.2)
 if s.sum()>100:print(round(lo,2),int(s.sum()),'mean xyz',c[s].mean(0).round(2),'yquant',np.quantile(c[s,1],[0,.25,.5,.75,1]).round(2),'xquant',np.quantile(c[s,0],[0,.25,.5,.75,1]).round(2))
