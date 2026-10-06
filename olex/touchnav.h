/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_touchnav_H
#define __olx_touchnav_H
#include "glmouse.h"
#include "wx/event.h"
#include <functional>

/* Maps wx touch gestures onto the TGlMouse view paths shared with the mouse
handlers. One-finger drag (rotate), tap (select) and double tap arrive as the
mouse events the platform synthesises and take the existing mouse path.
Gestures handled here:
  pinch           -> TGlMouse::ZoomView(zoom at start * factor)
  two-finger pan  -> TGlMouse::MoveView(dx, -dy)
  two-finger twist-> TGlMouse::RotateViewZ(clockwise degrees)
  long press      -> OnContextMenu(x, y)
The On* handlers return true when the view changed (caller redraws).
*/
class TTouchNav {
  TGlMouse *Mouse;
  double Zoom0, LastAngle;
  int ActiveMask;  // gestures between start and end
  /* a gesture began while a finger was down: the platform's synthesised
  mouse events up to and including the next button up belong to it
  */
  bool PendingUp;
  enum { gPan = 1, gZoom = 2, gRotate = 4 };
  void Begin(const wxGestureEvent &e, int g);
  void End(const wxGestureEvent &e, int g);
public:
  TTouchNav()
    : Mouse(0), Zoom0(1), LastAngle(0), ActiveMask(0), PendingUp(false)
  {}
  std::function<void(int x, int y)> OnContextMenu;
  void SetMouse(TGlMouse *m) { Mouse = m; }
  bool OnZoom(const wxZoomGestureEvent &e);
  bool OnPan(const wxPanGestureEvent &e);
  bool OnRotate(const wxRotateGestureEvent &e);
  void OnLongPress(const wxLongPressEvent &e);
  bool IsActive() const { return ActiveMask != 0; }
  /* true if a mouse event of type t is the platform echo of a gesture and
  must not reach the mouse path
  */
  bool SwallowMouse(wxEventType t);
  // the mask to pass to wxWindow::EnableTouchEvents
  static int EventMask();
};
#endif
