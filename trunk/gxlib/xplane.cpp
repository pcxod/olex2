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
  this->SetGroupable(false);
  Params().Resize(3);
}
//..............................................................................
void TXPlane::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  Params()[0] = acos(FPlane->GetNormal()[2])*180/M_PI;
  if( olx_abs(Params()[0]-180) < 1e-3 )  { // degenerate case with Pi rotation
    Params()[1] = 0;
    Params()[2] = 1;
  }
  else {
    Params()[1] = -FPlane->GetNormal()[1];
    Params()[2] = FPlane->GetNormal()[0];
  }
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
    mat3d norm_matr;
    CreateRotationMatrix(norm_matr,
      vec3d(-FPlane->GetNormal()[1], FPlane->GetNormal()[0], 0.0).Normalise(), FPlane->GetNormal()[2]);  
    if( !FPlane->IsRegular() )  {
      for( size_t i=0; i < sp.sortedPlane.Count(); i++ )  {
        const vec3d* crd = sp.sortedPlane.GetObject(i);
        double d = FPlane->DistanceTo(*crd);
        vec3d vec = *crd - FPlane->GetNormal()*d;
        vec -= FPlane->GetCenter();
        GlP.Vertices[i] = norm_matr*vec;
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
        GlP.Vertices[i] = norm_matr*marv;    
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
  olx_gl::normal(FPlane->GetNormal());
  olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
  return false;
}
//..............................................................................
void TXPlane::ListPrimitives(TStrList &List) const {
  List.Add("Plane");
  List.Add("Centroid");
}
//..............................................................................

 
 
