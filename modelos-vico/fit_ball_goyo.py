import bpy,numpy as np
m=bpy.data.objects['MascotaPumasGoyo'].data;n=len(m.polygons);c=np.empty((n,3),np.float64);no=np.empty((n,3),np.float64);m.polygons.foreach_get('center',c.ravel());m.polygons.foreach_get('normal',no.ravel());tot=np.empty(n,np.int32);m.polygons.foreach_get('loop_total',tot);lp=np.repeat(np.arange(n,dtype=np.int32),tot);a=m.color_attributes['GoyoColor'];r=np.empty(len(a.data)*4,np.float32);a.data.foreach_get('color',r);r=r.reshape(-1,4);fc=np.zeros((n,4),np.float32)
for k in range(4):fc[:,k]=np.bincount(lp,weights=r[:,k],minlength=n)/np.maximum(tot,1)
red=(fc[:,0]>.35)&(fc[:,0]>fc[:,1]*4)&(fc[:,0]>fc[:,2]*3)&(c[:,0]<0)&(c[:,1]<-1.5)&(c[:,2]<4.5)
p=c[red];A=np.c_[2*p,np.ones(len(p))];b=(p*p).sum(1);sol=np.linalg.lstsq(A,b,rcond=None)[0];center=sol[:3];radius=np.sqrt(sol[3]+center@center)
print('fit',center,'radius',radius)
rr=np.linalg.norm(c-center,axis=1);res=np.abs(rr-radius);candidate=(res<.09)&(c[:,0]<-.05)&(c[:,0]>-3.5)&(c[:,1]<-1.7)&(c[:,1]>-5.6)&(c[:,2]>.3)&(c[:,2]<4.4)
for name,q in [('red',candidate&red),('black',candidate&(fc[:,:3].max(1)<.08)),('gold',candidate&(fc[:,0]>fc[:,1]*1.25)&(fc[:,1]>fc[:,2]*1.3)),('other',candidate&~red&(fc[:,:3].max(1)>=.08)&~((fc[:,0]>fc[:,1]*1.25)&(fc[:,1]>fc[:,2]*1.3)))]:
 print(name,int(q.sum()),'z',c[q,2].min() if q.any() else None,c[q,2].max() if q.any() else None,'dots',np.quantile(np.einsum('ij,ij->i',no[q],(c[q]-center)/np.maximum(rr[q,None],1e-8)),[.1,.5,.9]).round(2) if q.any() else None)
