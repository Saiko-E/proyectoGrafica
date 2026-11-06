#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct V3 { float x=0, y=0, z=0; };
static V3 operator+(V3 a,V3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
static V3 operator-(V3 a,V3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
static V3 operator*(V3 a,float s){return {a.x*s,a.y*s,a.z*s};}
static V3 cross(V3 a,V3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
static V3 normalized(V3 a){float n=std::sqrt(a.x*a.x+a.y*a.y+a.z*a.z);return n>1e-8f?a*(1.0f/n):V3{0,1,0};}
struct Corner { V3 p,n; bool hasNormal=false; };
struct Triangle { Corner c[3]; };
struct Batch { std::string material; std::vector<Triangle> triangles; };
struct Group { std::string name; std::vector<Batch> batches; V3 lo{1e30f,1e30f,1e30f},hi{-1e30f,-1e30f,-1e30f}; };
struct Material { float r=.72f,g=.72f,b=.72f; };
static std::vector<Group> g_groups;
static std::unordered_map<std::string,Material> g_materials;
static HGLRC g_glrc=nullptr; static HDC g_hdc=nullptr; static HWND g_hwnd=nullptr;
static bool g_paused=false; static double g_time=0, g_last=0; static LARGE_INTEGER g_freq{},g_start{};
static V3 g_min{1e30f,1e30f,1e30f},g_max{-1e30f,-1e30f,-1e30f};
static float g_centerX=0,g_centerZ=0,g_floor=0,g_height=5,g_scale=1;

static std::string trim(const std::string& s){auto a=s.find_first_not_of(" \t\r\n"); if(a==std::string::npos)return {}; auto b=s.find_last_not_of(" \t\r\n"); return s.substr(a,b-a+1);}
static int objIndex(const std::string& token,int count){try{int i=std::stoi(token); return i>0?i-1:count+i;}catch(...){return -1;}}
static bool parseFaceCorner(const std::string& token,int nv,int nn,Corner& out){
    std::stringstream ss(token); std::string a,b,c; std::getline(ss,a,'/'); std::getline(ss,b,'/'); std::getline(ss,c,'/');
    int vi=objIndex(a,nv); if(vi<0||vi>=nv)return false;
    // OBJ vertex positions are copied separately by the caller through a temporary position table.
    if(!c.empty()){int ni=objIndex(c,nn); if(ni>=0&&ni<nn){out.hasNormal=true; out.n={}; out.n.x=float(ni+1);}}
    return true;
}

static bool loadMtl(const std::filesystem::path& path){
    std::ifstream in(path); if(!in)return false; std::string line,current;
    while(std::getline(in,line)){std::istringstream ss(line);std::string key;ss>>key;
        if(key=="newmtl"){ss>>current;g_materials[current]=Material{};}
        else if(key=="Kd"&&!current.empty()){ss>>g_materials[current].r>>g_materials[current].g>>g_materials[current].b;}
    } return true;
}

static bool loadObj(const std::filesystem::path& path){
    std::ifstream in(path); if(!in)return false;
    std::vector<V3> positions,normals; std::string line,mtllib,currentMaterial;
    Group* group=nullptr; Batch* batch=nullptr;
    auto newGroup=[&](const std::string& name){g_groups.push_back(Group{});group=&g_groups.back();group->name=name;batch=nullptr;};
    auto ensureBatch=[&](){if(!group)newGroup("modelo"); if(!batch||batch->material!=currentMaterial){group->batches.push_back(Batch{});batch=&group->batches.back();batch->material=currentMaterial;} return batch;};
    while(std::getline(in,line)){
        std::istringstream ss(line);std::string key;ss>>key;if(key.empty()||key[0]=='#')continue;
        if(key=="v"){V3 v;ss>>v.x>>v.y>>v.z;positions.push_back(v);}
        else if(key=="vn"){V3 n;ss>>n.x>>n.y>>n.z;normals.push_back(n);}
        else if(key=="mtllib"){std::string f;std::getline(ss,f);mtllib=trim(f);}
        else if(key=="o"||key=="g"){std::string name;std::getline(ss,name);newGroup(trim(name));}
        else if(key=="usemtl"){ss>>currentMaterial;batch=nullptr;}
        else if(key=="f"){
            std::vector<Corner> face; std::string t;
            while(ss>>t){Corner c;std::stringstream cs(t);std::string vi,vt,vn;std::getline(cs,vi,'/');std::getline(cs,vt,'/');std::getline(cs,vn,'/');int pi=objIndex(vi,(int)positions.size());if(pi<0||pi>=(int)positions.size())continue;c.p=positions[pi];if(!vn.empty()){int ni=objIndex(vn,(int)normals.size());if(ni>=0&&ni<(int)normals.size()){c.n=normals[ni];c.hasNormal=true;}}face.push_back(c);}
            if(face.size()<3)continue; Batch* b=ensureBatch();
            for(size_t i=1;i+1<face.size();++i){Triangle tri{{face[0],face[i],face[i+1]}};V3 fn=normalized(cross(tri.c[1].p-tri.c[0].p,tri.c[2].p-tri.c[0].p));for(auto& c:tri.c){if(!c.hasNormal)c.n=fn;group->lo.x=std::min(group->lo.x,c.p.x);group->lo.y=std::min(group->lo.y,c.p.y);group->lo.z=std::min(group->lo.z,c.p.z);group->hi.x=std::max(group->hi.x,c.p.x);group->hi.y=std::max(group->hi.y,c.p.y);group->hi.z=std::max(group->hi.z,c.p.z);}b->triangles.push_back(tri);}
        }
    }
    if(!mtllib.empty())loadMtl(path.parent_path()/std::filesystem::path(mtllib));
    size_t total=0;for(auto& g:g_groups){for(auto& b:g.batches)total+=b.triangles.size();if(g.lo.x<1e20f){g_min.x=std::min(g_min.x,g.lo.x);g_min.y=std::min(g_min.y,g.lo.y);g_min.z=std::min(g_min.z,g.lo.z);g_max.x=std::max(g_max.x,g.hi.x);g_max.y=std::max(g_max.y,g.hi.y);g_max.z=std::max(g_max.z,g.hi.z);}}
    if(!total)return false;g_floor=g_min.y;g_centerX=(g_min.x+g_max.x)*.5f;g_centerZ=(g_min.z+g_max.z)*.5f;g_height=std::max(.1f,g_max.y-g_min.y);g_scale=4.4f/g_height;return true;
}

static float wave(double t,double phase=0){return float(std::sin(t+phase));}
static bool has(const std::string& s,const char* token){return s.find(token)!=std::string::npos;}
static void drawGroup(const Group& g,double t){
    float w=float(2*M_PI/2.8), beat=std::sin(float(t)*w), side=(g.hi.x+g.lo.x)*.5f;
    V3 pivot{(g.lo.x+g.hi.x)*.5f,(g.lo.y+g.hi.y)*.5f,(g.lo.z+g.hi.z)*.5f};
    float angle=0; int axis=2; // Y-up OBJ: Z swings limbs sideways, X swings legs forward/back.
    if(has(g.name,"Brazo_")||has(g.name,"Manga_")||has(g.name,"Mano_")||has(g.name,"Puno_")||has(g.name,"Puño_")||has(g.name,"Borde_dorado_manga")){
        // Share one shoulder pivot across the arm, sleeve, cuff and hand groups.
        // The OBJ is flattened, so using each piece's own center makes the hand drift away.
        pivot.x=side>0?0.62f:-0.62f; pivot.y=4.43f; pivot.z=0.0f; axis=2;
        float outward=side>0?1.f:-1.f;
        angle=outward*(7.f+13.f*wave(t*w,(side>0?0.f:float(M_PI))));
    } else if(has(g.name,"Pierna_izquierda")||has(g.name,"Pierna_derecha")){
        pivot.y=g.hi.y; axis=0; angle=18.f*wave(t*w,(has(g.name,"izquierda")?0.f:float(M_PI)));
    } else if(has(g.name,"Zapato_")){
        pivot.y=g.hi.y; axis=0; angle=12.f*wave(t*w,(side>0?0.f:float(M_PI)));
    } else if(has(g.name,"Cabeza")||has(g.name,"Oreja_")){
        axis=2; angle=5.f*wave(t*w*.5f);
    } else if(has(g.name,"Torso")||has(g.name,"Short_blanco")){
        axis=2; angle=4.f*beat;
    }
    glPushMatrix();
    glTranslatef(pivot.x,pivot.y,pivot.z);
    if(angle!=0){if(axis==0)glRotatef(angle,1,0,0);else glRotatef(angle,0,0,1);}
    glTranslatef(-pivot.x,-pivot.y,-pivot.z);
    for(const auto& b:g.batches){Material m;auto it=g_materials.find(b.material);if(it!=g_materials.end())m=it->second;glColor3f(m.r,m.g,m.b);glBegin(GL_TRIANGLES);for(const auto& tri:b.triangles)for(const auto& c:tri.c){glNormal3f(c.n.x,c.n.y,c.n.z);glVertex3f(c.p.x,c.p.y,c.p.z);}glEnd();}
    glPopMatrix();
}
static void drawFloor(){
    glDisable(GL_LIGHTING);glColor3f(.10f,.16f,.23f);glBegin(GL_QUADS);glVertex3f(-20,-.03f,-20);glVertex3f(20,-.03f,-20);glVertex3f(20,-.03f,20);glVertex3f(-20,-.03f,20);glEnd();
    glColor3f(.20f,.28f,.36f);glBegin(GL_LINES);for(int i=-10;i<=10;++i){glVertex3f((float)i,-.015f,-10);glVertex3f((float)i,-.015f,10);glVertex3f(-10,-.015f,(float)i);glVertex3f(10,-.015f,(float)i);}glEnd();glEnable(GL_LIGHTING);
}
static void render(int width,int height){
    if(height<1)height=1;glViewport(0,0,width,height);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();float aspect=float(width)/float(height),nearP=.1f,top=nearP*std::tan(20.f*float(M_PI)/180.f);glFrustum(-top*aspect,top*aspect,-top,top,nearP,100.f);
    glMatrixMode(GL_MODELVIEW);glLoadIdentity();float dist=std::max(6.f,g_height*g_scale*2.15f);glTranslatef(0,-g_height*g_scale*0.50f,-dist);glRotatef(12,1,0,0);
    glEnable(GL_LIGHTING);glEnable(GL_LIGHT0);GLfloat lightPos[]={-3.f,8.f,7.f,0.f};GLfloat ambient[]={.48f,.48f,.52f,1.f};GLfloat diffuse[]={.95f,.92f,.84f,1.f};glLightfv(GL_LIGHT0,GL_POSITION,lightPos);glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
    glPushMatrix();glScalef(g_scale,g_scale,g_scale);glTranslatef(-g_centerX,-g_floor,-g_centerZ);
    drawFloor();
    LARGE_INTEGER q;QueryPerformanceCounter(&q);double now=double(q.QuadPart-g_start.QuadPart)/double(g_freq.QuadPart);if(!g_paused)g_time=now;double t=g_paused?g_time:now;
    float w=float(2*M_PI/2.8), bob=.10f+.16f*std::fabs(std::sin(float(t)*w));
    glPushMatrix();glTranslatef(0,bob,0);glTranslatef(0,g_height*.5f,0);glRotatef(5.f*wave(t*w),0,1,0);glRotatef(3.f*wave(t*w*.5f),0,0,1);glTranslatef(0,-g_height*.5f,0);
    for(const auto& group:g_groups)drawGroup(group,t);
    glPopMatrix();glPopMatrix();SwapBuffers(g_hdc);
}
static void setupGL(){
    PIXELFORMATDESCRIPTOR pfd{};pfd.nSize=sizeof(pfd);pfd.nVersion=1;pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;pfd.iPixelType=PFD_TYPE_RGBA;pfd.cColorBits=24;pfd.cDepthBits=24;pfd.iLayerType=PFD_MAIN_PLANE;
    int pf=ChoosePixelFormat(g_hdc,&pfd);SetPixelFormat(g_hdc,pf,&pfd);g_glrc=wglCreateContext(g_hdc);wglMakeCurrent(g_hdc,g_glrc);
    glEnable(GL_DEPTH_TEST);glEnable(GL_NORMALIZE);glEnable(GL_COLOR_MATERIAL);glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);glShadeModel(GL_SMOOTH);glClearColor(.025f,.045f,.085f,1.f);QueryPerformanceFrequency(&g_freq);QueryPerformanceCounter(&g_start);
}
static LRESULT CALLBACK wndProc(HWND h,UINT m,WPARAM w,LPARAM l){
    switch(m){case WM_KEYDOWN:if(w==VK_ESCAPE){DestroyWindow(h);return 0;}if(w==VK_SPACE){g_paused=!g_paused;return 0;}if(w=='R'){g_time=0;QueryPerformanceCounter(&g_start);return 0;}break;
    case WM_SIZE:InvalidateRect(h,nullptr,FALSE);return 0;
    case WM_CLOSE:DestroyWindow(h);return 0;
    case WM_DESTROY:PostQuitMessage(0);return 0;}return DefWindowProc(h,m,w,l);
}
int WINAPI WinMain(HINSTANCE inst,HINSTANCE,LPSTR,int){
    char exe[MAX_PATH]{};GetModuleFileNameA(nullptr,exe,MAX_PATH);std::filesystem::path dir=std::filesystem::path(exe).parent_path();SetCurrentDirectoryA(dir.string().c_str());
    if(!loadObj(dir/"Goyo.obj")){MessageBoxA(nullptr,"No pude cargar Goyo.obj. Coloca Goyo.obj y Goyo.mtl junto al programa.","Goyo Dance",MB_ICONERROR);return 1;}
    const char* cls="GoyoDanceWindow";WNDCLASSA wc{};wc.style=CS_OWNDC;wc.lpfnWndProc=wndProc;wc.hInstance=inst;wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.lpszClassName=cls;RegisterClassA(&wc);
    RECT r{0,0,1100,800};AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,FALSE);g_hwnd=CreateWindowA(cls,"Goyo | Baile Pumas — Espacio: pausa  |  R: reiniciar  |  Esc: salir",WS_OVERLAPPEDWINDOW|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,nullptr,nullptr,inst,nullptr);
    if(!g_hwnd)return 2;g_hdc=GetDC(g_hwnd);setupGL();ShowWindow(g_hwnd,SW_SHOW);
    MSG msg{};bool running=true;while(running){while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE)){if(msg.message==WM_QUIT){running=false;break;}TranslateMessage(&msg);DispatchMessage(&msg);}if(!running)break;RECT cr;GetClientRect(g_hwnd,&cr);render(cr.right-cr.left,cr.bottom-cr.top);Sleep(1);}
    if(g_glrc){wglMakeCurrent(nullptr,nullptr);wglDeleteContext(g_glrc);}if(g_hdc&&g_hwnd)ReleaseDC(g_hwnd,g_hdc);return 0;
}
