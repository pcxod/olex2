/* glprobe: the three wxQt-on-Android risks Olex2 depends on, in one canvas.
 (a) wxGLCanvas::SetCurrent on wxQt / Qt 6 (a QOpenGLWidget FBO)
 (b) gl4es fixed-function drawing (glBegin, display list, gluSphere, glBitmap)
     reaching the screen, from paint and from a timer outside paint
 (c) touch: which mouse, zoom, pan, rotate and long-press events arrive, with
     which positions and pinch semantics
Everything goes to logcat, tag "glprobe". The canvas sits below a label so
screen and client coordinates differ. The canvas uses Olex2's attribute list
and gesture mask (olex/xglcanv.cpp, olex/touchnav.cpp).

GLPROBE_SELF_INIT (stock wx): the app does what android/patches/wxwidgets
otherwise does inside wxGLCanvas, and the "mode" button switches the parts
off one at a time: 0 = all, 1 = no gl4es_setMainFBO, 2 = no gl4es swap. */
#include "wx/wx.h"
#include "wx/glcanvas.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <dlfcn.h>
#include <android/log.h>
#include <cstdarg>
#include <chrono>

extern "C" {
void initialize_gl4es();
void gl4es_setMainFBO(unsigned int fbo);
void gl4es_pre_swap();
void gl4es_post_swap();
void gl4es_resyncState();
}

static void L(const char *fmt, ...);
// logs and clears the GL error, naming the step it came from
static void Chk(const char *from, const char *step) {
  for (GLenum e; (e = glGetError()) != GL_NO_ERROR;)
    L("%s: glGetError=0x%x after %s", from, e, step);
}
static void L(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  __android_log_vprint(ANDROID_LOG_INFO, "glprobe", fmt, ap);
  va_end(ap);
}

// 5x7 digits, top row first, the 5 bits left aligned
static const unsigned char font[10][7] = {
  {0x70,0x88,0x98,0xA8,0xC8,0x88,0x70}, {0x20,0x60,0x20,0x20,0x20,0x20,0x70},
  {0x70,0x88,0x08,0x10,0x20,0x40,0xF8}, {0xF8,0x10,0x20,0x10,0x08,0x88,0x70},
  {0x10,0x30,0x50,0x90,0xF8,0x10,0x10}, {0xF8,0x80,0xF0,0x08,0x08,0x88,0x70},
  {0x30,0x40,0x80,0xF0,0x88,0x88,0x70}, {0xF8,0x08,0x10,0x20,0x40,0x40,0x40},
  {0x70,0x88,0x88,0x70,0x88,0x88,0x70}, {0x70,0x88,0x88,0x78,0x08,0x10,0x60}};

// glBitmap ignores glPixelZoom: blow each digit up 4x into a 24x28 bitmap
static void DrawNumber(float x, float y, int n) {
  char s[16];
  snprintf(s, sizeof(s), "%d", n);
  glRasterPos2f(x, y);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (const char *c = s; *c; c++) {
    unsigned char bm[28][3] = {};
    for (int r = 0; r < 28; r++) {
      const unsigned char row = font[*c - '0'][6 - r / 4];  // bottom row first
      for (int b = 0; b < 20; b++)
        if (row & (0x80 >> (b / 4))) bm[r][b / 8] |= 0x80 >> (b % 8);
    }
    glBitmap(24, 28, 0, 0, 28, 0, &bm[0][0]);
  }
}

class Canvas : public wxGLCanvas {
public:
  Canvas(wxWindow *parent, const int *attrs)
    : wxGLCanvas(parent, wxID_ANY, attrs), ctx(this), timer(this) {
    Bind(wxEVT_PAINT, &Canvas::OnPaint, this);
    Bind(wxEVT_TIMER, &Canvas::OnTimer, this);
    Bind(wxEVT_LEFT_DOWN, &Canvas::OnMouse, this);
    Bind(wxEVT_LEFT_UP, &Canvas::OnMouse, this);
    Bind(wxEVT_LEFT_DCLICK, &Canvas::OnMouse, this);
    Bind(wxEVT_RIGHT_DOWN, &Canvas::OnMouse, this);
    Bind(wxEVT_RIGHT_UP, &Canvas::OnMouse, this);
    Bind(wxEVT_MOTION, &Canvas::OnMouse, this);
    Bind(wxEVT_GESTURE_ZOOM, [this](wxZoomGestureEvent &e) {
      Gesture("zoom", e, wxString::Format("factor=%.4f", e.GetZoomFactor()));
    });
    Bind(wxEVT_GESTURE_PAN, [this](wxPanGestureEvent &e) {
      Gesture("pan", e, wxString::Format("delta=%d,%d", e.GetDelta().x, e.GetDelta().y));
    });
    Bind(wxEVT_GESTURE_ROTATE, [this](wxRotateGestureEvent &e) {
      Gesture("rotate", e, wxString::Format("angle=%.4f rad", e.GetRotationAngle()));
    });
    Bind(wxEVT_LONG_PRESS, [this](wxLongPressEvent &e) {
      Gesture("longpress", e, "");
    });
    timer.Start(500);
  }
  void SetStatus(wxStaticText *s) { status = s; }
  int mode = 0;

private:
  wxGLContext ctx;
  wxTimer timer;
  wxStaticText *status = nullptr;
  int paints = 0, ticks = 0;
  float angle = 0;
  GLuint list = 0;
  GLUquadric *quad = nullptr;

  void Gesture(const char *name, wxGestureEvent &e, const wxString &extra) {
    const wxString s = wxString::Format("%s pos=%d,%d %s%s%s", name,
      e.GetPosition().x, e.GetPosition().y, extra,
      e.IsGestureStart() ? " START" : "", e.IsGestureEnd() ? " END" : "");
    L("%s", (const char *)s.utf8_str());
    if (status) status->SetLabel(s);
  }

  void OnMouse(wxMouseEvent &e) {
    L("mouse %s pos=%d,%d", e.LeftDown() ? "left_down" : e.LeftUp() ? "left_up"
      : e.LeftDClick() ? "left_dclick" : e.RightDown() ? "right_down"
      : e.RightUp() ? "right_up" : e.Dragging() ? "drag" : "move",
      e.GetX(), e.GetY());
    e.Skip();
  }

  // true when the frame should be visible (reports SetCurrent)
  bool Render(const char *from) {
    const bool cur = SetCurrent(ctx);
    QOpenGLWidget *qw = static_cast<QOpenGLWidget *>(GetHandle());
#ifdef GLPROBE_SELF_INIT
    initialize_gl4es();
    gl4es_setMainFBO(mode == 1 ? 0 : qw->defaultFramebufferObject());
    gl4es_resyncState();
#endif
    const wxSize sz = GetClientSize() * GetContentScaleFactor();
    if (paints + ticks < 3 || (paints + ticks) % 50 == 0) {
      const wxPoint o = ClientToScreen(wxPoint(0, 0));
      L("%s: SetCurrent=%d qt_ctx=%p fbo=%u screen_origin=%d,%d wx_client=%dx%d"
        " scale=%.2f qt_px=%dx%d GL_VERSION=%s GL_RENDERER=%s", from, cur,
        (void *)qw->context(), qw->defaultFramebufferObject(), o.x, o.y,
        GetClientSize().x, GetClientSize().y,
        GetContentScaleFactor(), int(qw->width() * qw->devicePixelRatioF()),
        int(qw->height() * qw->devicePixelRatioF()),
        (const char *)glGetString(GL_VERSION), (const char *)glGetString(GL_RENDERER));
    }
    if (paints + ticks < 6) {  // what Qt left bound in the real GLES context
      // straight from the driver: in this library glGetIntegerv is gl4es,
      // and so is any inline Qt wrapper compiled here
      static auto real = (void (*)(GLenum, GLint *))dlsym(
        dlopen("libGLESv2.so", RTLD_NOW), "glGetIntegerv");
      GLint v[6] = {};
      const GLenum n[6] = {0x8CA6, 0x85B5, 0x8894, 0x8895, 0x8B8D, 0x8069};
      for (int i = 0; i < 6; i++) real(n[i], &v[i]);
      L("%s: real GLES fbo=%d vao=%d array_buf=%d element_buf=%d program=%d tex2d=%d",
        from, v[0], v[1], v[2], v[3], v[4], v[5]);
    }
    Chk(from, "start");
    glViewport(0, 0, sz.x, sz.y);
    glClearColor(0.1f, 0.15f, 0.3f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, double(sz.x) / wxMax(sz.y, 1), 1, 20);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1.2f, 0, -6);
    glRotatef(angle, 1, 1, 0);
    if (list == 0) {  // the cube, as a display list
      list = glGenLists(1);
      glNewList(list, GL_COMPILE);
      static const float v[8][3] = {{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
                                    {-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};
      static const int f[6][4] = {{0,1,2,3},{4,5,6,7},{0,1,5,4},{2,3,7,6},
                                  {0,3,7,4},{1,2,6,5}};
      static const float c[6][3] = {{1,0,0},{0,1,0},{0,0,1},{1,1,0},{0,1,1},{1,0,1}};
      glBegin(GL_QUADS);
      for (int i = 0; i < 6; i++) {
        glColor3fv(c[i]);
        for (int j = 0; j < 4; j++) glVertex3fv(v[f[i][j]]);
      }
      glEnd();
      glEndList();
    }
    Chk(from, "list built");
    glCallList(list);
    Chk(from, "glCallList");
    // a lit sphere through GLU
    glLoadIdentity();
    glTranslatef(1.5f, 0, -6);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColor3f(0.9f, 0.6f, 0.2f);
    if (!quad) quad = gluNewQuadric();
    gluSphere(quad, 0.9, 24, 16);
    Chk(from, "gluSphere");
    glDisable(GL_LIGHTING);
    // immediate mode last: the batch gl4es may hold back without a swap
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sz.x, 0, sz.y, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1, 1, 1);
    DrawNumber(20, 20, paints);   // bottom left: paint count
    DrawNumber(20, 70, ticks);    // above it: timer frames (drawn outside paint)
    Chk(from, "glBitmap");
    glBegin(GL_TRIANGLES);        // white triangle, bottom right
    glVertex2f(sz.x - 120, 20);
    glVertex2f(sz.x - 20, 20);
    glVertex2f(sz.x - 70, 110);
    glEnd();
    Chk(from, "triangle");
#ifdef GLPROBE_SELF_INIT
    if (mode != 2) {
      gl4es_pre_swap();
      gl4es_post_swap();
    }
#endif
    unsigned char px[4] = {};
    if (mode != 2)  // glReadPixels flushes gl4es and would hide mode 2
      glReadPixels(sz.x - 70, 50, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
    Chk(from, "glReadPixels");
    if (paints + ticks < 3 || (paints + ticks) % 50 == 0)
      L("%s: mode=%d triangle pixel=%d,%d,%d,%d", from, mode, px[0], px[1], px[2], px[3]);
    SwapBuffers();
    return cur;
  }

  void OnPaint(wxPaintEvent &) {
    wxPaintDC dc(this);
    paints++;
    Render("paint");
  }

  // no Refresh(): shows whether a frame drawn outside paint reaches the screen
  void OnTimer(wxTimerEvent &) {
    ticks++;
    angle += 15;
    Render("timer");
    if (ticks == 6) Bench();
  }

  /* Frame time of a mid-size structure the way Olex2 draws it: 64 lit atom
  spheres and 144 bond cylinders, each a display-listed GLU quadric placed by
  the matrix stack, rotating. glFinish per frame, so the time is CPU (gl4es)
  plus GPU, without vsync. Logged as "bench: ...". */
  void Bench() {
    SetCurrent(ctx);
    const wxSize sz = GetClientSize() * GetContentScaleFactor();
    GLuint sph = glGenLists(2), cyl = sph + 1;
    glNewList(sph, GL_COMPILE);
    gluSphere(quad, 0.25, 16, 12);
    glEndList();
    glNewList(cyl, GL_COMPILE);
    gluCylinder(quad, 0.08, 0.08, 1.0, 12, 1);
    glEndList();
    const int N = 120;
    const auto t0 = std::chrono::steady_clock::now();
    for (int f = 0; f < N; f++) {
      glViewport(0, 0, sz.x, sz.y);
      glClearColor(0.1f, 0.15f, 0.3f, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      glEnable(GL_COLOR_MATERIAL);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(45, double(sz.x) / wxMax(sz.y, 1), 1, 30);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glTranslatef(0, 0, -12);
      glRotatef(f * 3.0f, 0.3f, 1, 0.1f);
      for (int i = 0; i < 64; i++) {  // 4x4x4 grid, 1.4 apart
        const float x = (i % 4 - 1.5f) * 1.4f, y = (i / 4 % 4 - 1.5f) * 1.4f,
          z = (i / 16 - 1.5f) * 1.4f;
        glColor3f(0.3f + 0.2f * (i % 4), 0.5f, 0.9f - 0.2f * (i / 16));
        glPushMatrix();
        glTranslatef(x, y, z);
        glCallList(sph);
        glColor3f(0.7f, 0.7f, 0.7f);
        if (i % 4 != 3) {  // bond to +x
          glPushMatrix(); glRotatef(90, 0, 1, 0); glScalef(1, 1, 1.4f);
          glCallList(cyl); glPopMatrix();
        }
        if (i / 4 % 4 != 3) {  // bond to +y
          glPushMatrix(); glRotatef(-90, 1, 0, 0); glScalef(1, 1, 1.4f);
          glCallList(cyl); glPopMatrix();
        }
        if (i / 16 != 3) {  // bond to +z
          glPushMatrix(); glScalef(1, 1, 1.4f); glCallList(cyl); glPopMatrix();
        }
        glPopMatrix();
      }
      glFinish();
    }
    const double ms = std::chrono::duration<double, std::milli>(
      std::chrono::steady_clock::now() - t0).count();
    Chk("bench", "frames");
    L("bench: %d frames, 64 spheres + 144 cylinders, %dx%d px, %.2f ms/frame (%.1f fps)",
      N, sz.x, sz.y, ms / N, 1000.0 * N / ms);
    glDeleteLists(sph, 2);
    SwapBuffers();
  }
};

class Frame : public wxFrame {
public:
  Frame() : wxFrame(nullptr, wxID_ANY, "glprobe") {
    wxBoxSizer *s = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *top = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *status = new wxStaticText(this, wxID_ANY, "touch the canvas");
    status->SetMinSize(wxSize(-1, 80));
    top->Add(status, 1, wxALL, 8);
    // Olex2's list (xglcanv.cpp), multisampling and stereo off as on Android
    static const int attrs[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER,
      WX_GL_DEPTH_SIZE, 24, WX_GL_STENCIL_SIZE, 8, 0};
    Canvas *c = new Canvas(this, attrs);
#ifdef GLPROBE_SELF_INIT
    wxButton *b = new wxButton(this, wxID_ANY, "mode 0");
    b->Bind(wxEVT_BUTTON, [b, c](wxCommandEvent &) {
      c->mode = (c->mode + 1) % 3;
      b->SetLabel(wxString::Format("mode %d", c->mode));
      L("mode -> %d", c->mode);
      c->Refresh();
    });
    top->Add(b, 0, wxALL, 8);
#endif
    s->Add(top, 0, wxEXPAND);
    s->Add(c, 1, wxEXPAND);
    SetSizer(s);
    c->SetStatus(status);
    const bool touch = c->EnableTouchEvents(wxTOUCH_ZOOM_GESTURE |
      wxTOUCH_PAN_GESTURES | wxTOUCH_ROTATE_GESTURE | wxTOUCH_PRESS_GESTURES);
    L("EnableTouchEvents=%d", touch);
  }
};

class App : public wxApp {
public:
  bool OnInit() override {
    // a wxQt frame on Android keeps wx's default 400x250 size unless maximised
    Frame *f = new Frame();
    f->Maximize();
    f->Show();
    L("started, wx %d.%d.%d", wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER);
    return true;
  }
};
wxIMPLEMENT_APP(App);
