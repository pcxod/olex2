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

//..............................................................................
TXPlane::TXPlane(const olxstr& collectionName, TSPlane *Plane, TGlRender *Render) :
  AGDrawObject(collectionName)
{
  FPlane = Plane;
  FParent = Render;
  FRectangular = false;
}
//..............................................................................
void TXPlane::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGlMaterial GlM, GlM1;
  TGPCollection* GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);

  GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmAmbientB|sglmDiffuseB|sglmTransparent);
  GlM1.SetFlags( sglmAmbientF );
  GlM1.AmbientF = 0;

  TGlPrimitive* GlP = GPC->NewPrimitive("Plane");
  GlM.AmbientF = 0x7f00007f;
  GlM.DiffuseF = 0x7f3f3f3f;
  GlM.AmbientB = 0x7f00007f;
  GlM.DiffuseB = 0x7f3f3f3f;
  GlP->SetProperties(&GlM);
  GlP->Type(sgloPolygon);
  if( !FRectangular )  GlP->Data().Resize(3, FPlane->CrdCount());
  else                 GlP->Data().Resize(3, 5);
  vec3d Center( FPlane->Center() ), 
        org(FPlane->GetAtom(0).crd()-FPlane->Center()), vec;
  TPSTypeList<double, vec3d const*> sortedPlane;
  sortedPlane.Add( 0, &FPlane->GetAtom(0).crd() );

  for( int i=1; i < FPlane->CrdCount(); i++ )  {
    vec = FPlane->GetAtom(i).crd() - Center;
    double ca = org.CAngle(vec);
    vec = org.XProdVec(vec);
    // negative - vec is on the right, positive - on the left
    double vo = vec.CAngle(FPlane->Normal());
    if( ca >= 0 )  { // -90 to 90
       if( vo < 0 )  // -90 to 0 3->4
         sortedPlane.Add( 3.0 + ca, &FPlane->GetAtom(i).crd() );
       else  // 0 to 90 0->1
         sortedPlane.Add( 1.0 - ca, &FPlane->GetAtom(i).crd() );
    }
    else if( ca > -1 ) {  // 90-270
       if( vo < 0 )  // 180 to 270 2->3
         sortedPlane.Add( 3.0 + ca, &FPlane->GetAtom(i).crd() );
       else  // 90 to 180 1->2
         sortedPlane.Add( 1.0 - ca, &FPlane->GetAtom(i).crd() );
    }
    else  {  //-1, special case
      sortedPlane.Add( 2, &FPlane->GetAtom(i).crd() );
    }
  }

  if( !FRectangular )  {
    for( int i=0; i < sortedPlane.Count(); i++ )  {
      vec3d const* crd = sortedPlane.Object(i);
      double d = FPlane->DistanceTo(*crd);
      vec = *crd - FPlane->Normal()*d;
      vec -= Center;
      GlP->Data()[0][i] = vec[0];
      GlP->Data()[1][i] = vec[1];
      GlP->Data()[2][i] = vec[2];
    }
  }
  else  {
    vec3d marv;
    double maxr = 0;
    for( int i=0; i < sortedPlane.Count(); i++ )  {
      vec = *sortedPlane.Object(i);
      double d = FPlane->DistanceTo(vec);
      vec -= FPlane->Normal()*d;
      vec -= Center;
      d = vec.Length();
      if( d > maxr )  {
        maxr = d;
        marv = vec;
      }
    }
    mat3d rm;
    CreateRotationMatrix(rm, FPlane->Normal(), cos(M_PI*72.0/180) );
    for( int i=0; i < 5; i++ )  {
      GlP->Data()[0][i] = marv[0];    
      GlP->Data()[1][i] = marv[1];
      GlP->Data()[2][i] = marv[2];
      marv *= rm;
    }
  }
  GlP = GPC->NewPrimitive("Centroid");
  GlP->SetProperties(&GlM1);
  GlP->Type(sgloSphere);
  GlP->Params()[0] = 1./FParent->GetZoom();  GlP->Params()[1] = 6;  GlP->Params()[2] = 6;
  GlP->Compile();
}
//..............................................................................
TXPlane::~TXPlane()
{  ;  }
//..............................................................................
bool TXPlane::Orient(TGlPrimitive *P)  {
  FParent->GlTranslate(FPlane->Center());
  return false;
}
//..............................................................................

 
 
