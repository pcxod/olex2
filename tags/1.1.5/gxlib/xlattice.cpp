/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xlattice.h"
#include "glrender.h"
#include "styles.h"
#include "gpcollection.h"
#include "glprimitive.h"

TXLattice::TXLattice(TGlRenderer& Render, const olxstr& collectionName) :
  AGlMouseHandlerImp(Render, collectionName)
{
  Fixed = false;
  Size = 4;
  SetMove2D(false);
  SetMoveable(true);
  SetZoomable(false);
}
//..............................................................................
void TXLattice::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  Lines = &GPC.NewPrimitive("Lines", sgloLines);
  TGlMaterial GlM;
  GlM.SetFlags(sglmAmbientF);
  GlM.AmbientF = 0;
  GlM.SetIdentityDraw(false);
  GlM.SetTransparent(false);

  Lines->SetProperties(GS.GetMaterial("Lines", GlM));

  // initialise data
  SetSize( GetSize() );

  TGlPrimitive& glpLabel = GPC.NewPrimitive("Label", sgloText);  // labels
  TGlMaterial GlM1;
  GlM1.SetIdentityDraw(true);
  GlM1.SetTransparent(false);
  glpLabel.SetProperties(GS.GetMaterial("Label", GlM1));
  glpLabel.SetFont(
    &Parent.GetScene().GetFont(Parent.GetScene().FindFontIndexForType<TXLattice>(), true));
}
//..............................................................................
bool TXLattice::Orient(TGlPrimitive& P)  {
  if( Fixed )  {
    vec3d c = Parent.GetBasis().GetCenter();
    olx_gl::translate(c*=-1);
    mat3d m = Parent.GetBasis().GetMatrix();
    olx_gl::orient(Basis.GetMatrix() * m.Transpose());
    olx_gl::translate(Basis.GetCenter());
  }
  else
    olx_gl::translate(Basis.GetCenter());

  if( P.GetType() == sgloLines )  {
    olx_gl::pointSize(5);
    olx_gl::begin(GL_POINTS);
    for( int i=-Size; i  < Size; i++ )  {
      for( int j=-Size; j  < Size; j++ )  {
        for( int k=-Size; k  < Size; k++ )
          olx_gl::vertex(vec3d(i,j,k)*LatticeBasis);
      }
    }
    olx_gl::end();
  }
  return true;
}
//..............................................................................
bool TXLattice::GetDimensions(vec3d &Max, vec3d &Min)  {
  return false;
}
//..............................................................................
bool TXLattice::OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {
  if( !IsMoveable() )  return true;
  return AGlMouseHandlerImp::OnMouseDown(Sender, Data);
}
//..............................................................................
bool TXLattice::OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {
  if( !IsMoveable() )  return true;
  return AGlMouseHandlerImp::OnMouseUp(Sender, Data);
}
//..............................................................................
bool TXLattice::OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {
  AGlMouseHandlerImp::OnMouseMove(Sender, Data);
  return true;
}
//..............................................................................
void TXLattice::SetSize(short v)  {
  if( v < 1 || v > 10 )  v = 2;
  Size = v;
//  Lines->Data().Resize(3, 24);

}
//..............................................................................
void TXLattice::SetFixed(bool v )  {
  if( v )  {
    vec3d c = Basis.GetCenter();
    Basis.Orient( Parent.GetBasis().GetMatrix() );
    Basis.SetCenter(c);
  }
  else  {
    vec3d c = Basis.GetCenter();
    Basis.Reset();
    Basis.SetCenter(c);
  }
  SetMove2D(v);
  Fixed = v;
}
