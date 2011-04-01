//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXLine
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "xline.h"
#include "gpcollection.h"
#include "xatom.h"

//----------------------------------------------------------------------------//
// TXLine function bodies
//----------------------------------------------------------------------------//
TXLine::TXLine(TGlRenderer& r, const olxstr& collectionName, const vec3d& base, const vec3d& edge): 
  TXBond(NULL, r, collectionName),
  FBase(base), FEdge(edge)
{
  vec3d C(edge - base);
  GetGlLabel().SetOffset((base+edge)/2);
  GetGlLabel().SetLabel(olxstr::FormatFloat(3, C.Length()));
  GetGlLabel().SetVisible(true);
  if( !C.IsNull() )  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = (float)(acos(C[2])*180/M_PI);
    Params()[1] = -C[1];
    Params()[2] = C[0];
  }
}
//..............................................................................
void TXLine::Create(const olxstr& cName)  {
  //if( !cName.IsEmpty() )  
  //  SetCollectionName(cName);

  //TGPCollection* GPC = Parent.FindCollection( GetCollectionName() );
  //if( GPC == NULL || GPC->PrimitiveCount() == 0 )  {
  //  TXBond::Create();
  //  TGraphicsStyle& GS = Primitives()->GetStyle();

  //  for( int i=0; i < GetPrimitives().PrimitiveCount(); i++ )  {
  //    TGlPrimitive& GlP = GetPrimitives().GetPrimitive(i);
  //    TGlMaterial* GlM = const_cast<TGlMaterial*>(GS->Material(GlP->GetName()));
  //    GlM->SetIdentityDraw( false );
  //    GlP->SetProperties(GlM);
  //  }
  //}
  //else  
  //  GPC->AddObject(this);
  TXBond::Create(cName);
}
//..............................................................................
TXLine::~TXLine(){}
//..............................................................................
bool TXLine::Orient(TGlPrimitive& GlP)  {
  olxstr Length = olxstr::FormatFloat(3, Params()[3]);
  if( GlP.GetType() == sgloText )  {
    vec3d V;
    V = (FEdge+FBase)/2;
    V += Parent.GetBasis().GetCenter();
    V = Parent.GetBasis().GetMatrix()*V;
    olx_gl::rasterPos(V[0]+0.15, V[1]+0.15, V[2]+5);
    GlP.SetString(&Length);
    GlP.Draw();
    GlP.SetString(NULL);
    return true;
  }
  else  {
    olx_gl::translate(FBase);
    olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
    olx_gl::scale(Params()[4], Params()[4], Params()[3]);
  }
  return false;
} 
//..............................................................................
void TXLine::Radius(float V)  {
  Params()[4] = V;
}
//..............................................................................
void TXLine::Length(float V)  {
  Params()[3] = V;
}
//..............................................................................
