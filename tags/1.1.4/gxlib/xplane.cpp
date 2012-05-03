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
  const mat3d& m = FPlane->GetBasis();
  Params().Resize(16);
  FParams[0] = m[0][0];  FParams[1] = m[0][1];  FParams[2] = m[0][2];  FParams[3] = 0;
  FParams[4] = m[1][0];  FParams[5] = m[1][1];  FParams[6] = m[1][2];  FParams[7] = 0;
  FParams[8] = m[2][0];  FParams[9] = m[2][1];  FParams[10]= m[2][2];  FParams[11]= 0;
  FParams[12] = 0;       FParams[13] = 0;       FParams[14]= 0;        FParams[15]= 1;

  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  GS.SetPersistent(true);
  const int PMask = GS.GetParam(GetPrimitiveMaskName(), "3", true).ToInt();
  if( PMask == 0 )  return;
  if( (PMask & 1) != 0 )  {
    TGlMaterial GlM;
    GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmAmbientB|sglmDiffuseB|sglmTransparent);
    GlM.AmbientF = 0x7f00007f;
    GlM.DiffuseF = 0x7f3f3f3f;
    GlM.AmbientB = 0x7f00007f;
    GlM.DiffuseB = 0x7f3f3f3f;
    TGlPrimitive& GlP = GPC.NewPrimitive("Plane", sgloPolygon);
    GlP.SetProperties(GS.GetMaterial(GlP.GetName(), GlM));
    if( !FPlane->IsRegular() )  
      GlP.Vertices.SetCount(FPlane->CrdCount());
    else                 
      GlP.Vertices.SetCount(5);

    PlaneSort::Sorter sp(*FPlane);
    const mat3d transform = FPlane->GetBasis();
    if( !FPlane->IsRegular() )  {
      for( size_t i=0; i < sp.sortedPlane.Count(); i++ )  {
        const vec3d* crd = sp.sortedPlane.GetObject(i);
        GlP.Vertices[i] = transform*(*crd-FPlane->GetCenter());
      }
    }
    else  {
      double maxrs = 0;
      for( size_t i=0; i < sp.sortedPlane.Count(); i++ )  {
        const vec3d* crd = sp.sortedPlane.GetObject(i);
        const double qd = (*crd-FPlane->GetCenter()).QLength();
        if( qd > maxrs )
          maxrs = qd;
      }
      vec3d marv(0, sqrt(maxrs), 0);
      mat3d rm;
      olx_create_rotation_matrix(rm, vec3d(1,0,0), cos(M_PI*72.0/180));
      for( int i=0; i < 5; i++ )  {
        GlP.Vertices[i] = marv;    
        marv *= rm;
      }
    }

  }
  if( (PMask & 2) != 0 )  {
    TGlMaterial GlM1;
    GlM1.SetFlags(sglmAmbientF);
    GlM1.AmbientF = 0;
    TGlPrimitive& glpC = GPC.NewPrimitive("Centroid", sgloSphere);
    glpC.SetProperties(GS.GetMaterial(glpC.GetName(), GlM1));
    glpC.Params[0] = 0.25;  glpC.Params[1] = 10;  glpC.Params[2] = 10;
  }
  Compile();
}
//..............................................................................
bool TXPlane::Orient(TGlPrimitive& P)  {
  olx_gl::translate(FPlane->GetCenter());
  olx_gl::orient(Params().GetRawData());
  if( P.GetType() != sgloSphere )
    olx_gl::normal(FPlane->GetNormal());
  return false;
}
//..............................................................................
void TXPlane::ListPrimitives(TStrList &List) const {
  List.Add("Plane");
  List.Add("Centroid");
}
//..............................................................................