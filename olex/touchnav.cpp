/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "touchnav.h"
#include "glrender.h"
#include "wx/window.h"  // wxTOUCH_* masks
#include <cmath>

int TTouchNav::EventMask() {
  return wxTOUCH_ZOOM_GESTURE | wxTOUCH_PAN_GESTURES |
    wxTOUCH_ROTATE_GESTURE | wxTOUCH_PRESS_GESTURES;
}
//..............................................................................
void TTouchNav::Begin(const wxGestureEvent &e, int g) {
  if (ActiveMask == 0 && Mouse != 0) {
    // drop the one-finger drag the first finger's synthesised mouse down began
    Mouse->ResetMouseState(e.GetPosition().x, e.GetPosition().y);
  }
  ActiveMask |= g;
  PendingUp = true;
#ifdef __ANDROID__
  if (Mouse != 0) {
    Mouse->BakeView(true);
  }
#endif
}
//..............................................................................
void TTouchNav::End(const wxGestureEvent &e, int g) {
  ActiveMask &= ~g;
#ifdef __ANDROID__
  if (ActiveMask == 0 && Mouse != 0) {
    Mouse->BakeView(false);
  }
#endif
}
//..............................................................................
bool TTouchNav::OnZoom(const wxZoomGestureEvent &e) {
  if (Mouse == 0) {
    return false;
  }
  if (e.IsGestureStart()) {
    Begin(e, gZoom);
    // the factor is cumulative from the start of the gesture
    Zoom0 = Mouse->Parent()->GetZoom();
  }
  bool res = Mouse->ZoomView(Zoom0*e.GetZoomFactor());
  if (e.IsGestureEnd()) {
    End(e, gZoom);
  }
  return res;
}
//..............................................................................
bool TTouchNav::OnPan(const wxPanGestureEvent &e) {
  if (Mouse == 0) {
    return false;
  }
  if (e.IsGestureStart()) {
    Begin(e, gPan);
  }
  // delta is since the previous event; screen y grows down, view y up
  const wxPoint d = e.GetDelta();
  bool res = (d.x != 0 || d.y != 0) ? Mouse->MoveView(d.x, -d.y) : false;
  if (e.IsGestureEnd()) {
    End(e, gPan);
  }
  return res;
}
//..............................................................................
bool TTouchNav::OnRotate(const wxRotateGestureEvent &e) {
  if (Mouse == 0) {
    return false;
  }
  // cumulative clockwise radians; MSW reports it in [0, 2pi)
  const double a = e.GetRotationAngle();
  if (e.IsGestureStart()) {
    Begin(e, gRotate);
    LastAngle = 0;
  }
  double d = a - LastAngle;
  LastAngle = a;
  const double pi = 3.14159265358979323846;
  while (d > pi) {
    d -= 2 * pi;
  }
  while (d <= -pi) {
    d += 2 * pi;
  }
  /* a clockwise drag around the centre with the mouse (meRotateZ) increases
  RZ, so a clockwise twist does too
  */
  bool res = (d != 0) ? Mouse->RotateViewZ(d * 180 / pi) : false;
  if (e.IsGestureEnd()) {
    End(e, gRotate);
  }
  return res;
}
//..............................................................................
void TTouchNav::OnLongPress(const wxLongPressEvent &e) {
  if (Mouse != 0) {
    Mouse->ResetMouseState(e.GetPosition().x, e.GetPosition().y);
  }
  // the finger lifting after the menu must not select
  PendingUp = true;
  if (OnContextMenu) {
    OnContextMenu(e.GetPosition().x, e.GetPosition().y);
  }
}
//..............................................................................
bool TTouchNav::SwallowMouse(wxEventType t) {
  if (ActiveMask != 0) {
    return true;
  }
  if (!PendingUp) {
    return false;
  }
  if (t == wxEVT_LEFT_DOWN || t == wxEVT_RIGHT_DOWN) {
    // no echo arrived - a new touch or click starts clean
    PendingUp = false;
    return false;
  }
  if (t == wxEVT_LEFT_UP || t == wxEVT_RIGHT_UP) {
    PendingUp = false;
  }
  return true;
}
