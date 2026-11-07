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

/* maps wx touch gestures onto the TGlMouse view paths: pinch zooms, two-finger
pan moves, twist rotates about z, long press opens the context menu. One-finger
drag, tap and double tap come as synthesised mouse events and take the mouse
path. The On* handlers return true when the view changed
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
