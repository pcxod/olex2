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
bool TGlMouseListener::OnMouseDown(const IEObject *Sender, const TMouseData *Data)  {
//  if( ! (Data->Button & smbLeft) )  return false;
  SX = Data->DownX;
  SY = Data->DownY;
  return true;
}
//..............................................................................
bool TGlMouseListener::OnMouseUp(const IEObject *Sender, const TMouseData *Data)  {
  return false;
}
//..............................................................................
bool TGlMouseListener::OnMouseMove(const IEObject *Sender, const TMouseData *Data)  {
  int dx = Data->X - SX, dy = SY - Data->Y;
  bool res = false;
  if( (Data->Button == smbLeft) && (Data->Shift == sssShift) )  {  // move
    if( !Moveable() )  {  SX = Data->X;  SY = Data->Y;  return res;}
    if( !Move2D() ) {  // move in 3D
      vec3d T;
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
  if( (Data->Button == smbLeft) && (!Data->Shift || (Data->Shift & sssCtrl)))  { // rotate
    if( !Roteable() )  {  
      SX = Data->X;  
      SY = Data->Y;  
      return res;
    }
    /* a not trivial (for some) task, to rotate in current basis as if the ritation
    happens in on screen (identity) basis; For Z the rotation axis is defined by 
    {0,0,1} = ra*Current_Basis and so on, this leasd to three linear equations for 
    three values of the rotation angle...
    */
//    mat3d basis( mat3d::Transpose(FParent->GetBasis().GetMatrix()) );
    mat3d basis( FParent->GetBasis().GetMatrix() );
    if( Data->Shift == sssCtrl )  {
      double RZ = 0;
      if( SX > FParent->GetWidth()/2 ) RZ -= (double)dy/FRotationDiv;
      else                             RZ += (double)dy/FRotationDiv;
      if( SY > FParent->GetHeight()/2 )  RZ -= (double)dx/FRotationDiv;
      else                               RZ += (double)dx/FRotationDiv;
      if( RZ != 0 ) 
        Basis.Rotate(mat3d::CramerSolve(basis, vec3d(0,0,1)), RZ*M_PI/180);
      res = true;
    }
    if( !Data->Shift )  {// rotate XY
      double RX = (double)(dy)/FRotationDiv;
      double RY = (double)(dx)/FRotationDiv;
      if( RX != 0 ) 
        Basis.Rotate(mat3d::CramerSolve(basis, vec3d(1,0,0)), RX*M_PI/180);
      if( RY != 0 )
        Basis.Rotate(mat3d::CramerSolve(basis, vec3d(0,1,0)), RY*M_PI/180);
      res = true;
    }
  }
  if( (Data->Button & smbRight) && (!Data->Shift) )  {  // zoom
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

