/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
/* Feeds synthesised wx gesture events through TTouchNav into a real
TGlRenderer/TGlMouse (no window, no GL context) and checks the view. Also
checks the refactored mouse handlers still give the original numbers.
Exit code = number of failed checks.
*/
#include "touchnav.h"
#include "glrender.h"
#include "glscene.h"
#include "glfont.h"
#include "bapp.h"
#include <cmath>
#include <cstdio>

static int failures = 0;
#define CHECK(c) do { if (!(c)) { ++failures; \
  std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)
static bool near_(double a, double b, double eps = 1e-9) {
  return std::fabs(a - b) <= eps;
}

class TStubScene : public AGlScene {
protected:
  virtual TGlFont& DoCreateFont(TGlFont& glf, bool) const { return glf; }
public:
  virtual void ScaleFonts(double) {}
  virtual void RestoreFontScale() {}
  virtual olxstr ShowFontDialog(TGlFont*, const olxstr&) {
    return EmptyString();
  }
  virtual bool MakeCurrent() { return false; }
};

static const double pi = 3.14159265358979323846;

template <class E> E gesture(int x, int y, bool start, bool end) {
  E e;
  e.SetPosition(wxPoint(x, y));
  e.SetGestureStart(start);
  e.SetGestureEnd(end);
  return e;
}
static wxZoomGestureEvent zoom(double f, bool s = false, bool e = false) {
  wxZoomGestureEvent ev = gesture<wxZoomGestureEvent>(200, 150, s, e);
  ev.SetZoomFactor(f);
  return ev;
}
static wxPanGestureEvent pan(int dx, int dy, bool s = false, bool e = false) {
  wxPanGestureEvent ev = gesture<wxPanGestureEvent>(200, 150, s, e);
  ev.SetDelta(wxPoint(dx, dy));
  return ev;
}
static wxRotateGestureEvent rot(double a, bool s = false, bool e = false) {
  wxRotateGestureEvent ev = gesture<wxRotateGestureEvent>(200, 150, s, e);
  ev.SetRotationAngle(a);
  return ev;
}

int main(int argc, char *argv[]) {
  TBasicApp app(TBasicApp::GuessBaseDir(argv[0]));
  // ponytail: renderer is leaked - its dtor frees GL lists, there is no context
  TGlRenderer &R = *new TGlRenderer(new TStubScene(), 400, 300);
  TGlMouse m(&R, 0);
  TTouchNav nav;
  nav.SetMouse(&m);

  // pinch: zoom is z0 * cumulative factor, clamped like the mouse path
  R.SetZoom(1.5);
  CHECK(nav.OnZoom(zoom(1.0, true)));
  CHECK(nav.IsActive());
  nav.OnZoom(zoom(1.3));
  CHECK(near_(R.GetZoom(), 1.5*1.3));
  nav.OnZoom(zoom(2.0, false, true));
  CHECK(near_(R.GetZoom(), 3.0));
  CHECK(!nav.IsActive());
  nav.OnZoom(zoom(1.0, true));
  nav.OnZoom(zoom(1e6, false, true));
  CHECK(near_(R.GetZoom(), 100));

  // two-finger pan == the mouse translate for the same screen motion
  R.SetZoom(2);
  const vec3d c0 = R.GetBasis().GetCenter();
  nav.OnPan(pan(0, 0, true));
  CHECK(nav.OnPan(pan(30, -20)));  // right and up
  nav.OnPan(pan(0, 0, false, true));
  const vec3d dt = R.GetBasis().GetCenter() - c0;
  CHECK(near_(dt[0], 30.0/300/2) && near_(dt[1], 20.0/300/2) &&
    near_(dt[2], 0));
  // shift+ctrl+left drag 100,100 -> 130,80 through the TGlMouse dispatch
  m.ResetMouseState(100, 100, sssShift | sssCtrl, smbLeft);
  CHECK(m.MouseMove(130, 80, sssShift | sssCtrl));
  const vec3d dm = R.GetBasis().GetCenter() - c0 - dt;
  CHECK(near_(dm[0], dt[0]) && near_(dm[1], dt[1]) && near_(dm[2], dt[2]));

  // twist: clockwise radians become degrees added to RZ; MSW wraps to [0,2pi)
  R.RotateZ(90);
  nav.OnRotate(rot(0, true));
  CHECK(nav.OnRotate(rot(0.25)));
  nav.OnRotate(rot(0.5, false, true));
  CHECK(near_(R.GetBasis().GetRZ(), 90 + 0.5*180/pi));
  const double rz1 = R.GetBasis().GetRZ();
  nav.OnRotate(rot(0, true));
  nav.OnRotate(rot(2*pi - 0.1, false, true));  // 0.1 rad anticlockwise
  CHECK(near_(R.GetBasis().GetRZ(), rz1 - 0.1*180/pi));
  // ctrl+left drag on the right half moving down (clockwise) also adds RZ
  const double rz2 = R.GetBasis().GetRZ();
  m.ResetMouseState(300, 150, sssCtrl, smbLeft);
  CHECK(m.MouseMove(300, 160, sssCtrl));
  CHECK(near_(R.GetBasis().GetRZ(), rz2 + 10/FRotationDiv));
  // disabled rotation is honoured by the shared path
  m.SetRotationEnabled(false);
  CHECK(!nav.OnRotate(rot(0.3, true, true)));
  CHECK(near_(R.GetBasis().GetRZ(), rz2 + 10/FRotationDiv));
  m.SetRotationEnabled(true);

  // mouse zoom (right drag) unchanged by the refactor: z + dx/600 - dy/600
  R.SetZoom(1);
  m.ResetMouseState(100, 100, 0, smbRight);
  CHECK(m.MouseMove(160, 100, 0));
  CHECK(near_(R.GetZoom(), 1 + 60.0/600));

  // a gesture drops the drag the first finger started and eats its echo
  m.ResetMouseState(100, 100, 0, smbLeft);
  CHECK(m.GetMouseData().Button == smbLeft);
  nav.OnPan(pan(0, 0, true));
  CHECK(m.GetMouseData().Button == 0);
  CHECK(nav.SwallowMouse(wxEVT_MOTION));
  CHECK(nav.SwallowMouse(wxEVT_LEFT_UP));  // during the gesture
  nav.OnPan(pan(0, 0, false, true));
  CHECK(nav.SwallowMouse(wxEVT_MOTION));
  CHECK(nav.SwallowMouse(wxEVT_LEFT_UP));  // the echo of the lift
  CHECK(!nav.SwallowMouse(wxEVT_MOTION));
  CHECK(!nav.SwallowMouse(wxEVT_LEFT_DOWN));
  // no echo at all: the next press goes through and clears the state
  nav.OnZoom(zoom(1, true, true));
  CHECK(!nav.SwallowMouse(wxEVT_LEFT_DOWN));
  CHECK(!nav.SwallowMouse(wxEVT_LEFT_UP));

  // long press opens the menu at the touch point; the lift does not select
  int mx = -1, my = -1;
  nav.OnContextMenu = [&](int x, int y) { mx = x; my = y; };
  nav.OnLongPress(gesture<wxLongPressEvent>(42, 24, true, true));
  CHECK(mx == 42 && my == 24);
  CHECK(nav.SwallowMouse(wxEVT_LEFT_UP));
  CHECK(!nav.SwallowMouse(wxEVT_LEFT_DOWN));

  // mouse only (touch never fired): nothing is swallowed
  TTouchNav idle;
  CHECK(!idle.SwallowMouse(wxEVT_LEFT_DOWN) &&
    !idle.SwallowMouse(wxEVT_MOTION) && !idle.SwallowMouse(wxEVT_LEFT_UP) &&
    !idle.SwallowMouse(wxEVT_LEFT_DCLICK));

  std::printf("%d failure(s)\n", failures);
  return failures;
}
