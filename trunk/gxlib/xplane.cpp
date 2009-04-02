//---------------------------------------------------------------------------//
// namespace TEXLib:
// TXPlane
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xplane.h"
#include "gpcollection.h"
#include "estlist.h"
#include "planesort.h"

//..............................................................................
TXPlane::TXPlane(const olxstr& collectionName, TSPlane *Plane, TGlRenderer *Render) :
  AGDrawObject(collectionName)
{
  FPlane = Plane;
  FParent = Render;
}
//..............................................................................
void TXPlane::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  if( GPC == NULL )
    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGlMaterial GlM, GlM1;
  GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmAmbientB|sglmDiffuseB|sglmTransparent);
  GlM1.SetFlags( sglmAmbientF );
  GlM1.AmbientF = 0;

  TGlPrimitive* GlP = GPC->NewPrimitive("Plane", sgloPolygon);
  GlM.AmbientF = 0x7f00007f;
  GlM.DiffuseF = 0x7f3f3f3f;
  GlM.AmbientB = 0x7f00007f;
  GlM.DiffuseB = 0x7f3f3f3f;
  GlP->SetProperties(&GlM);
  if( !FPlane->IsRegular() )  
    GlP->Data.Resize(3, FPlane->CrdCount());
  else                 
    GlP->Data.Resize(3, 5);

  PlaneSort::Sorter sp( *FPlane );
  if( !FPlane->IsRegular() )  {
    for( int i=0; i < sp.sortedPlane.Count(); i++ )  {
      const vec3d* crd = sp.sortedPlane.GetObject(i);
      double d = FPlane->DistanceTo(*crd);
      vec3d vec = *crd - FPlane->GetNormal()*d;
      vec -= FPlane->GetCenter();
      GlP->Data[0][i] = vec[0];
      GlP->Data[1][i] = vec[1];
      GlP->Data[2][i] = vec[2];
    }
  }
  else  {
    vec3d marv;
    double maxr = 0;
    for( int i=0; i < sp.sortedPlane.Count(); i++ )  {
      vec3d vec = *sp.sortedPlane.GetObject(i);
      double d = FPlane->DistanceTo(vec);
      vec -= FPlane->GetNormal()*d;
      vec -= FPlane->GetCenter();
      d = vec.Length();
      if( d > maxr )  {
        maxr = d;
        marv = vec;
      }
    }
    mat3d rm;
    CreateRotationMatrix(rm, FPlane->GetNormal(), cos(M_PI*72.0/180) );
    for( int i=0; i < 5; i++ )  {
      GlP->Data[0][i] = marv[0];    
      GlP->Data[1][i] = marv[1];
      GlP->Data[2][i] = marv[2];
      marv *= rm;
    }
  }
  GlP = GPC->NewPrimitive("Centroid", sgloSphere);
  GlP->SetProperties(&GlM1);
  GlP->Params[0] = 0.25;  GlP->Params[1] = 6;  GlP->Params[2] = 6;
  GlP->Compile();
}
//..............................................................................
TXPlane::~TXPlane()
{  ;  }
//..............................................................................
bool TXPlane::Orient(TGlPrimitive *P)  {
  FParent->GlTranslate(FPlane->GetCenter());
  glNormal3d(FPlane->GetNormal()[0], FPlane->GetNormal()[1], FPlane->GetNormal()[2]);
  return false;
}
//..............................................................................

 
 
