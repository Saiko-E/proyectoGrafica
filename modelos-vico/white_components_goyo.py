import bpy,numpy as np
m=bpy.data.objects['MascotaPumasGoyo'].data;n=len(m.polygons)
cent=np.empty((n,3),np.float32);m.polygons.foreach_get('center',cent.ravel()); tot=np.empty(n,np.int32);m.polygons.foreach_get('loop_total',tot); loop_poly=np.repeat(np.arange(n,dtype=np.int32),tot)
a=m.color_attributes['GoyoColor'];r=np.empty(len(a.data)*4,np.float32);a.data.foreach_get('color',r);r=r.reshape(-1,4);fc=np.zeros((n,4),np.float32)
for j in range(4):fc[:,j]=np.bincount(loop_poly,weights=r[:,j],minlength=n)/np.maximum(tot,1)
# White faces near the leg that steps on the ball.
q=(fc[:,:3].min(1)>.8)&(cent[:,0]>-3.4)&(cent[:,0]<-.4)&(cent[:,1]<-.5)&(cent[:,1]>-5.8)&(cent[:,2]>2.5)&(cent[:,2]<5.8)
ids=np.where(q)[0];print('white candidate faces',len(ids))
parent={int(f):int(f) for f in ids}
def find(a):
 while parent[a]!=a: parent[a]=parent[parent[a]];a=parent[a]
 return a
def union(a,b):
 ra,rb=find(a),find(b)
 if ra!=rb:parent[rb]=ra
edge_owner={}
for fi in ids:
 poly=m.polygons[int(fi)]; vs=list(poly.vertices)
 for j in range(len(vs)):
  a,b=vs[j],vs[(j+1)%len(vs)]; key=(a,b) if a<b else (b,a)
  prev=edge_owner.get(key)
  if prev is None:edge_owner[key]=int(fi)
  else:union(int(fi),prev)
comp={}
for fi in ids:comp.setdefault(find(int(fi)),[]).append(int(fi))
print('components',len(comp))
for k,fs in sorted(comp.items(),key=lambda kv:len(kv[1]),reverse=True)[:20]:
 p=cent[fs];print('n',len(fs),'bounds',p.min(0).round(3).tolist(),p.max(0).round(3).tolist(),'mean',p.mean(0).round(3).tolist())
