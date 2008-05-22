//----------------------------------------------------------------------------//
// namespace TEXLib
// TGlMouseListner - an implementation of GDrawObject which responde to mouse events
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "glmouselistener.h"
#include "glmouse.h"
#include "glrender.h"

UseGlNamespace();
//..............................................................................
//..............................................................................

TGlMouseListener::TGlMouseListener(const olxstr& collectionName, TGlRender *R) :
  AGDrawObject(collectionName)
{
  FParent = R;
  Flags = 0;
}
//..............................................................................
TGlMouseListener::~TGlMouseListener()  {  return;  }
//..............................................................................
bool TGlMouseListener::OnMouseDown(const IEObject *Sender, const TMouseData *Data)
{
//  if( ! (Data->Button & smbLeft) )  return false;
  SX = Data->DownX;
  SY = Data->DownY;
  return true;
}
//..............................................................................
bool TGlMouseListener::OnMouseUp(const IEObject *Sender, const TMouseData *Data)
{
  return false;
}
//..............................................................................
bool TGlMouseListener::OnMouseMove(const IEObject *Sender, const TMouseData *Data)
{
  int dx = Data->X - SX, dy = SY - Data->Y;
  bool res = false;
  if( (Data->Button == smbLeft) && (Data->Shift == sssShift) )  // move
  {
    if( !Moveable() )  {  SX = Data->X;  SY = Data->Y;  return res;}
    if( !Move2D() ) {  // move in 3D
      TVPointD T;
      double v = FParent->GetScale();
      if( Data->Shift & sssCtrl )
      { T[2] = (float)(dx+dy)*v;  }
      else
      { T[0] = (float)(dx)*v;      T[1] = (float)(dy)*v;    }
      // use V*M not M*V, as the basis is transposed (See TEBasis::Orient for details)
      T = FParent->GetBasis().GetMatrix() * T;
      Basis.Translate(T);
      res = true;
    }
    else  {  // move on screen (2D)
      Basis.TranslateX( dx );
      Basis.TranslateY( dy );
      res = true;
    }
  }
  if( (Data->Button == smbLeft) && (!Data->Shift || (Data->Shift & sssCtrl)))  // rotate
  {
    if( !Roteable() )  {  SX = Data->X;  SY = Data->Y;  return res;}
    if( Data->Shift == sssCtrl )
    {
      double RZ = Basis.GetRZ();
      if( SX > FParent->GetWidth()/2 ) RZ -= (double)dy/FRotationDiv;
      else                          RZ += (double)dy/FRotationDiv;
      if( SY > FParent->GetHeight()/2 )  RZ -= (double)dx/FRotationDiv;
      else                            RZ += (double)dx/FRotationDiv;
      if( RZ > 360 )  RZ = 0;
      if( RZ < 0 )    RZ = 360;
      Basis.RotateZ(RZ);
      res = true;
    }
    if( !Data->Shift )  // rotate XY
    {
      double RX;
      double RY;
      RX = Basis.GetRX() + (double)(dy)/FRotationDiv;
      RY = Basis.GetRY() + (double)(dx)/FRotationDiv;
      if( RX > 360 )  RX = 0;
      if( RX < 0 )    RX = 360;
      if( RY > 360 )  RY = 0;
      if( RY < 0 )    RY = 360;
      Basis.RotateX(RX);
      Basis.RotateY(RY);
      res = true;
    }
  }
  if( (Data->Button & smbRight) && (!Data->Shift) )  // zoom
  {
    if( !Zoomable() )  {
      SX = Data->X;
      SY = Data->Y;
      return res;
    }
    Basis.SetZoom( Basis.GetZoom() + (double)(dx)/FZoomDiv - (double)(dy)/FZoomDiv);
    if( Basis.GetZoom() < 0.01 )  Basis.SetZoom( 0.01 );
      res = true;
  }
  SX = Data->X;
  SY = Data->Y;
  return res;
}
//..............................................................................

