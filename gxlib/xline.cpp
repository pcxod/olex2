//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXLine
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xline.h"
#include "gpcollection.h"
#include "glscene.h"

#include "xatom.h"

//----------------------------------------------------------------------------//
// TXLine function bodies
//----------------------------------------------------------------------------//
TXLine::TXLine(TGlRenderer& r, const olxstr& collectionName, const vec3d& base, const vec3d& edge): 
  TXBond(r, collectionName, *(TSBond*)NULL)
{
  FBase = base;
  FEdge = edge;
  vec3d C( FEdge - FBase);
  if( !C.IsNull() )  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = (float)(acos(C[2])*180/M_PI);
    Params()[1] = -C[1];
    Params()[2] = C[0];
  }
}
//..............................................................................
void TXLine::Create(const olxstr& cName, const ACreationParams* cpar)  {
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
  TXBond::Create(cName, cpar);
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
    glRasterPos3d(V[0]+0.15, V[1]+0.15, V[2]+5);
    GlP.SetString(&Length);
    GlP.Draw();
    GlP.SetString(NULL);
    return true;
  }
  else  {
    Parent.GlTranslate(FBase);
    Parent.GlRotate((float)Params()[0], (float)Params()[1], (float)Params()[2], 0.0);
    Parent.GlScale((float)Params()[4], (float)Params()[4], (float)Params()[3]);
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
