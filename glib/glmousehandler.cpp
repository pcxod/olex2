/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glmousehandler.h"
#include "glmouse.h"
#include "glrender.h"

bool AGlMouseHandler::EventHandler::OnMouseDown(AGlMouseHandler& Sender,
  const TMouseData& Data)
{
  SX = Data.DownX;
  SY = Data.DownY;
  MouseHandled = false;
  if (Data.Button == smbLeft || (Sender.IsZoomable() && Data.Button == smbRight)) {
    MouseDown = true;
    return true;
  }
  return false;
}
//..............................................................................
bool AGlMouseHandler::EventHandler::OnMouseUp(AGlMouseHandler& Sender,
  const TMouseData& Data)
{
  if (!MouseDown || !MouseHandled) {
    return false;
  }
  MouseDown = false;
  return Data.Button == smbLeft;
}
//..............................................................................
bool AGlMouseHandler::EventHandler::OnMouseMove(AGlMouseHandler& Sender,
  const TMouseData& Data)
{
  if (!MouseDown) {
    return false;
  }
  const int dx = Data.X - SX, dy = SY - Data.Y;
  bool res = false;
  if ((Data.Button == smbLeft)) {
    if ((Data.Shift == sssShift)) {  // move
      if (Sender.IsMoveable()) {
        if (Sender.IsMove2D()) {
          res = Sender.DoTranslate(vec3d(dx, dy, 0));
        }
        else if (Sender.IsMove2DZ()) {
          res = Sender.DoTranslate(
            vec3d((double)dx / Sender.DoGetRenderer().GetZoom(),
              (double)dy / Sender.DoGetRenderer().GetZoom(),
              0));
        }
        else {  // move in 3D
          vec3d T;
          const double v = Sender.DoGetRenderer().GetScale();
          if ((Data.Shift & sssCtrl) != 0)
            T[2] = (dx + dy)*v;
          else {
            T[0] = dx*v;
            T[1] = dy*v;
          }
          // use V*M not M*V, as the basis is transposed (See TEBasis::Orient for details)
          T = Sender.DoGetRenderer().GetBasis().GetMatrix() * T;
          T /= Sender.DoGetRenderer().GetBasis().GetZoom();
          res = Sender.DoTranslate(T);
        }
      }
    }
    else if ((Data.Shift == 0 || ((Data.Shift&sssCtrl) != 0))) { // rotate
      if (Sender.IsRoteable()) {
        /* not a trivial (for some) task, to rotate in current basis as if the
        rotation happens in on screen (identity) basis; so we need to find such
        a vector, which becomes {0,0,1} for the Z rotation etc for X and Y
        after multiplied by current basis. for Z axis it is defined by {0,0,1}
        = ra*Current_Basis and so on, this leads to three linear equations for
        three values of the rotation vector...
        */
        mat3d basis(mat3d::Transpose(Sender.DoGetRenderer().GetBasis().GetMatrix()));
        if (Data.Shift == sssCtrl) {
          double RZ = 0;
          if (SX > Sender.DoGetRenderer().GetWidth() / 2)
            RZ -= (double)dy / FRotationDiv;
          else
            RZ += (double)dy / FRotationDiv;
          if (SY > Sender.DoGetRenderer().GetHeight() / 2)
            RZ -= (double)dx / FRotationDiv;
          else
            RZ += (double)dx / FRotationDiv;
          if (RZ != 0) {
            res = Sender.DoRotate(mat3d::CramerSolve(basis,
              vec3d(0, 0, 1)).Normalise(), RZ*M_PI / 180);
          }
        }
        else if (Data.Shift == 0) {// rotate XY
          const double RX = -(double)(dy) / FRotationDiv;
          const double RY = (double)(dx) / FRotationDiv;
          if (RX != 0) {
            res = Sender.DoRotate(mat3d::CramerSolve(basis,
              vec3d(1, 0, 0)).Normalise(), RX*M_PI / 180);
          }
          if (RY != 0) {
            res = Sender.DoRotate(mat3d::CramerSolve(basis,
              vec3d(0, 1, 0)).Normalise(), RY*M_PI / 180);
          }
        }
      }
    }
  }
  else if ((Data.Button&smbRight) != 0 && (Data.Shift == 0)) {  // zoom
    if (Sender.IsZoomable()) {
      res = Sender.DoZoom((double)(dx) / FZoomDiv - (double)(dy) / FZoomDiv, true);
    }
  }
  SX = Data.X;
  SY = Data.Y;
  return (MouseHandled = res);
}
//..............................................................................
