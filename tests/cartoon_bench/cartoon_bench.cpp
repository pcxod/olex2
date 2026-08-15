/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

/* Does a compiled OpenGL display list of a protein-sized triangle count still
rotate?

This is the question the cartoon architecture stands on, and it is a property of
the driver rather than of Olex2: a compatibility-profile driver may compile a
display list into an internal vertex buffer, or may replay it as immediate mode,
and the two differ by orders of magnitude. It needs no model, no lattice and no
Olex2, so it is answered here instead of inside the application.

The geometry is the real one: gxlib/cartoon_geom.cpp is compiled in and the
triangles come from BuildTube, emitted exactly as TXCartoon::EmitList emits
them, one glBegin for the whole chain and one list per chain.

Two things this measures that Olex2's own fps function cannot:
  - VSync is turned off, so the result is a frame time rather than an
    observation censored at the refresh rate.
  - An empty scene is measured in the same run as the control, so the window,
    driver and compositor overhead is subtracted rather than assumed.

  cl /O2 /EHsc /I..\..\sdl /I..\..\xlib /I..\..\gxlib cartoon_bench.cpp
    ..\..\gxlib\cartoon_geom.cpp sdl.lib opengl32.lib gdi32.lib user32.lib
*/

/* cartoon_geom.h comes first on purpose: the sdl string headers below it do
not survive being included after windows.h.
*/
#include "cartoon_geom.h"

#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>

typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXT)(int);

/* Ask a hybrid-graphics laptop for the discrete adapter.

Without these the driver hands out the integrated GPU, and on this machine the
two differ by more than an order of magnitude at protein triangle counts. Both
vendors document an exported symbol as the mechanism; it has to be exported
from the executable itself, not from a library it links.
*/
extern "C" {
  __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

//.............................................................................
/* The same bundle of ideal alpha helices TGXApp::CreateTestCartoon builds: a
grid rather than one long chain, so the object stays compact on screen. A long
thin rod would be mostly clipped by the driver and would flatter the result.
*/
static void BuildBundle(size_t n_residues,
  TTypeList<xlib::protein::ChainSegment> &out)
{
  const size_t per_chain = 100;
  const size_t n_chains = (n_residues + per_chain - 1)/per_chain;
  size_t grid = 1;
  while (grid*grid < n_chains) {
    grid++;
  }
  const double helix_r = 2.3, rise = 1.5, turn = 100*M_PI/180, spacing = 12;
  size_t residues = 0;
  for (size_t ci = 0; ci < n_chains; ci++) {
    xlib::protein::ChainSegment &seg = out.AddNew();
    seg.chain_id = olxch('A' + (ci % 26));
    const double ox = spacing*(ci % grid), oy = spacing*(ci/grid);
    const size_t rc = olx_min(per_chain, n_residues - residues);
    for (size_t i = 0; i < rc; i++) {
      xlib::protein::BackboneResidue r;
      r.number = (int)(i + 1);
      const double a = turn*i;
      r.ca = vec3d(ox + helix_r*cos(a), oy + helix_r*sin(a), rise*i);
      seg.residues.AddCopy(r);
    }
    residues += rc;
  }
}
//.............................................................................
struct Scene {
  TArrayList<GLuint> lists;
  size_t triangles;
  vec3f centre;
  float radius;
  double build_ms;
  Scene() : triangles(0), centre(0, 0, 0), radius(1), build_ms(0) {}
};

static double Now() {
  LARGE_INTEGER f, c;
  QueryPerformanceFrequency(&f);
  QueryPerformanceCounter(&c);
  return 1000.0*c.QuadPart/f.QuadPart;
}
//.............................................................................
static void BuildScene(size_t n_residues, Scene &s) {
  TTypeList<xlib::protein::ChainSegment> segments;
  BuildBundle(n_residues, segments);
  gxlib::cartoon::CartoonParams p;

  const double t0 = Now();
  vec3f mn(1e9f, 1e9f, 1e9f), mx(-1e9f, -1e9f, -1e9f);
  for (size_t i = 0; i < segments.Count(); i++) {
    gxlib::cartoon::Mesh m;
    /* The shipped path, secondary structure included: the bundle is built from
    ideal alpha helices, so this measures ribbons rather than plain tube, which
    is roughly twice the triangles per residue.
    */
    TArrayList<short> ss;
    xlib::protein::AssignSecondaryStructure(segments[i], ss);
    gxlib::cartoon::BuildCartoon(segments[i], ss, p, m);
    if (m.triangles.IsEmpty()) {
      continue;
    }
    const GLuint id = glGenLists(1);
    glNewList(id, GL_COMPILE);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glColor4f(0.4f + 0.02f*(i % 8), 0.6f, 0.8f, 1);
    glBegin(GL_TRIANGLES);
    for (size_t j = 0; j < m.triangles.Count(); j++) {
      const IndexTriangle &tr = m.triangles[j];
      for (size_t k = 0; k < 3; k++) {
        const size_t vi = tr.vertices[k];
        glNormal3fv(m.normals[vi].GetData());
        glVertex3fv(m.vertices[vi].GetData());
      }
    }
    glEnd();
    glEndList();
    s.lists.Add(id);
    s.triangles += m.triangles.Count();
    for (size_t j = 0; j < m.vertices.Count(); j++) {
      vec3f::UpdateMinMax(m.vertices[j], mn, mx);
    }
  }
  glFinish();
  s.build_ms = Now() - t0;
  if (!s.lists.IsEmpty()) {
    s.centre = (mn + mx)/2;
    s.radius = olx_max(1.0f, (mx - mn).Length()/2);
  }
}
//.............................................................................
static double MedianOf(TArrayList<double> &v) {
  for (size_t i = 0; i + 1 < v.Count(); i++) {
    for (size_t j = i + 1; j < v.Count(); j++) {
      if (v[j] < v[i]) {
        const double t = v[i]; v[i] = v[j]; v[j] = t;
      }
    }
  }
  return v.IsEmpty() ? 0 : v[v.Count()/2];
}
//.............................................................................
static void Draw(const Scene &s, float angle, int w, int h) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  const double r = s.radius*1.2, aspect = double(w)/h;
  glOrtho(-r*aspect, r*aspect, -r, r, -r*4, r*4);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glRotatef(angle, 0.3f, 1, 0.2f);
  glTranslatef(-s.centre[0], -s.centre[1], -s.centre[2]);
  for (size_t i = 0; i < s.lists.Count(); i++) {
    glCallList(s.lists[i]);
  }
}
//.............................................................................
static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
  return DefWindowProc(h, m, w, l);
}
//.............................................................................
int main(int argc, char *argv[]) {
  const int width = 1280, height = 800;
  WNDCLASSA wc;
  memset(&wc, 0, sizeof(wc));
  wc.lpfnWndProc = WndProc;
  wc.hInstance = GetModuleHandle(0);
  wc.lpszClassName = "olx_cartoon_bench";
  RegisterClassA(&wc);
  HWND hwnd = CreateWindowA("olx_cartoon_bench", "Olex2 cartoon benchmark",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE, 40, 40, width, height, 0, 0,
    wc.hInstance, 0);
  HDC dc = GetDC(hwnd);

  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;
  SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd);
  HGLRC rc = wglCreateContext(dc);
  wglMakeCurrent(dc, rc);

  printf("GL_RENDERER: %s\n", (const char *)glGetString(GL_RENDERER));
  printf("GL_VERSION : %s\n", (const char *)glGetString(GL_VERSION));

  PFNWGLSWAPINTERVALEXT swap_interval =
    (PFNWGLSWAPINTERVALEXT)wglGetProcAddress("wglSwapIntervalEXT");
  if (swap_interval != 0) {
    swap_interval(0);
    printf("VSync    : off\n");
  }
  else {
    printf("VSync    : COULD NOT BE DISABLED, frame times are censored at the"
      " refresh rate\n");
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_SMOOTH);
  glClearColor(0.1f, 0.1f, 0.12f, 1);

  size_t sizes[] = {0, 1000, 5000, 20000, 50000};
  const size_t n_sizes = sizeof(sizes)/sizeof(sizes[0]);
  if (argc > 1) {
    sizes[0] = 0;
    sizes[1] = (size_t)atoi(argv[1]);
  }
  const size_t warmup = 30, frames = 300;

  printf("\n%10s %8s %11s %9s %9s %9s %8s\n", "residues", "chains",
    "triangles", "build/ms", "frame/ms", "median/ms", "fps");
  for (size_t si = 0; si < n_sizes; si++) {
    Scene s;
    if (sizes[si] != 0) {
      BuildScene(sizes[si], s);
    }
    TArrayList<double> ts;
    float angle = 0;
    for (size_t i = 0; i < warmup + frames; i++) {
      MSG msg;
      while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
      const double t0 = Now();
      Draw(s, angle, width, height);
      glFinish();
      SwapBuffers(dc);
      const double dt = Now() - t0;
      angle += 1;
      if (i >= warmup) {
        ts.Add(dt);
      }
    }
    double sum = 0;
    for (size_t i = 0; i < ts.Count(); i++) {
      sum += ts[i];
    }
    const double mean = sum/ts.Count(), median = MedianOf(ts);
    printf("%10u %8u %11u %9.1f %9.3f %9.3f %8.0f\n",
      (unsigned)sizes[si], (unsigned)s.lists.Count(), (unsigned)s.triangles,
      s.build_ms, mean, median, median > 0 ? 1000/median : 0);
    fflush(stdout);
    for (size_t i = 0; i < s.lists.Count(); i++) {
      glDeleteLists(s.lists[i], 1);
    }
  }
  printf("\nThe first row is the empty-scene control: window, driver and"
    " compositor cost,\nmeasured in the same run rather than assumed.\n");

  wglMakeCurrent(0, 0);
  wglDeleteContext(rc);
  ReleaseDC(hwnd, dc);
  DestroyWindow(hwnd);
  return 0;
}
