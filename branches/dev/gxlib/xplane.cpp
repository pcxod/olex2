//---------------------------------------------------------------------------//
// TXPlane
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "xplane.h"
#include "gpcollection.h"
#include "estlist.h"
#include "planesort.h"
#include "glprimitive.h"
#include "styles.h"

//..............................................................................
TXPlane::TXPlane(TGlRenderer& r, const olxstr& collectionName, TSPlane *Plane) :
  AGDrawObject(r, collectionName)
{
  FPlane = Plane;
}
//..............................................................................
void TXPlane::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  GS.SetPersistent(true);

  TGlMaterial GlM;
  GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmAmbientB|sglmDiffuseB|sglmTransparent);
  GlM.AmbientF = 0x7f00007f;
  GlM.DiffuseF = 0x7f3f3f3f;
  GlM.AmbientB = 0x7f00007f;
  GlM.DiffuseB = 0x7f3f3f3f;
  TGlPrimitive& GlP = GPC.NewPrimitive("Plane", sgloPolygon);
  GlP.SetProperties(GS.GetMaterial("Plane", GlM));
  if( !FPlane->IsRegular() )  
    GlP.Vertices.SetCount(FPlane->CrdCount());
  else                 
    GlP.Vertices.SetCount(5);

  PlaneSort::Sorter sp( *FPlane );
  if( !FPlane->IsRegular() )  {
    for( size_t i=0; i < sp.sortedPlane.Count(); i++ )  {
      const vec3d* crd = sp.sortedPlane.GetObject(i);
      double d = FPlane->DistanceTo(*crd);
      vec3d vec = *crd - FPlane->GetNormal()*d;
      vec -= FPlane->GetCenter();
      GlP.Vertices[i] = vec;
    }
  }
  else  {
    vec3d marv;
    double maxr = 0;
    for( size_t i=0; i < sp.sortedPlane.Count(); i++ )  {
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
      GlP.Vertices[i] = marv;    
      marv *= rm;
    }
  }
  TGlMaterial GlM1;
  GlM1.SetFlags(sglmAmbientF);
  GlM1.AmbientF = 0;
  TGlPrimitive& glpC = GPC.NewPrimitive("Centroid", sgloSphere);
  glpC.SetProperties(GS.GetMaterial("Centroid", GlM1));
  glpC.Params[0] = 0.25;  glpC.Params[1] = 6;  glpC.Params[2] = 6;
  glpC.Compile();
}
//..............................................................................
TXPlane::~TXPlane()
{  ;  }
//..............................................................................
bool TXPlane::Orient(TGlPrimitive& P)  {
  Parent.GlTranslate(FPlane->GetCenter());
  glNormal3d(FPlane->GetNormal()[0], FPlane->GetNormal()[1], FPlane->GetNormal()[2]);
  return false;
}
//..............................................................................

 
 
