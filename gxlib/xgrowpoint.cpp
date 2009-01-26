//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXGrowLine
// (c) Oleg V. Dolomanov, 2006
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xgrowpoint.h"
#include "gpcollection.h"

//----------------------------------------------------------------------------//
// TXGrowLine function bodies
//----------------------------------------------------------------------------//
TXGrowPoint::TXGrowPoint(const olxstr& collectionName, const vec3d& center,
                         const smatd& transform, TGlRender *Render) :
  AGDrawObject(collectionName)  {
  AGDrawObject::Groupable(false);
  FParent = Render;
  Params().Resize(1);
  Params()[0] = 1;
  Transform = transform;
  Center = center;
}
//..............................................................................
void TXGrowPoint::Create(const olxstr& cName, const ACreationParams* cpar) {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  if( GPC == NULL )  {
    GPC = FParent->NewCollection( GetCollectionName() );
    TGraphicsStyle* GS = GPC->Style();
    GPC->AddObject(this);

    TGlPrimitive* GlP = GPC->NewPrimitive( GetCollectionName() );
    TGlMaterial* GlM = const_cast<TGlMaterial*>(GS->Material(GetCollectionName()));

    if( GlM->Mark() )  {
      GlM->SetFlags( sglmAmbientF|sglmAmbientB);
      GlM->AmbientF = 0x000000ff;
      GlM->DiffuseF = 0x000000FF;
      GlM->AmbientB = 0x000000FF;
      GlM->DiffuseB = 0x000000FF;
      GlM->Mark(false);
    }
    GlP->SetProperties(GlM);
    double sz = 1;
    GlP->Type(sgloTriangles);
    GlP->Data().Resize(4, 12);  // four triangles + normals
    const double c_30 = sqrt(3.0)/2, s_30 = 0.5;
    // bottom
    GlP->Data()[0][0] = 0;      GlP->Data()[1][0] = sz;
    GlP->Data()[0][1] = sz*c_30;   GlP->Data()[1][1] = -sz*s_30;
    GlP->Data()[0][2] = -sz*c_30;  GlP->Data()[1][2] = -sz*s_30;
    vec3d n = vec3d(GlP->Data()[0][1], GlP->Data()[1][1], GlP->Data()[2][1]).XProdVec( 
              vec3d(GlP->Data()[0][2], GlP->Data()[1][2], GlP->Data()[2][2]) ).Normalise();
    GlP->Data()[3][0] = n[0];  GlP->Data()[3][1] = n[1];  GlP->Data()[3][2] = n[2];  

    GlP->Data()[0][4] = 0;      GlP->Data()[1][4] = sz;
    GlP->Data()[0][3] = sz*c_30;   GlP->Data()[1][3] = -sz*s_30;
    GlP->Data()[0][5] = 0;      GlP->Data()[1][5] = 0;         GlP->Data()[2][5] = sz;
    n = vec3d(GlP->Data()[0][4], GlP->Data()[1][4], GlP->Data()[2][4]).XProdVec( 
        vec3d(GlP->Data()[0][5], GlP->Data()[1][5], GlP->Data()[2][5]) ).Normalise();
    GlP->Data()[3][3] = n[0];  GlP->Data()[3][4] = n[1];  GlP->Data()[3][5] = n[2];  

    GlP->Data()[0][6] = 0;      GlP->Data()[1][6] = sz;
    GlP->Data()[0][7] = -sz*c_30;  GlP->Data()[1][7] = -sz*s_30;
    GlP->Data()[0][8] = 0;      GlP->Data()[1][8] = 0;         GlP->Data()[2][8] = sz;
    n = vec3d(GlP->Data()[0][7], GlP->Data()[1][7], GlP->Data()[2][7]).XProdVec( 
        vec3d(GlP->Data()[0][8], GlP->Data()[1][8], GlP->Data()[2][8]) ).Normalise();
    GlP->Data()[3][6] = n[0];  GlP->Data()[3][7] = n[1];  GlP->Data()[3][8] = n[2];  

    GlP->Data()[0][10] = sz*c_30;  GlP->Data()[1][10] = -sz*s_30;
    GlP->Data()[0][9] = -sz*c_30;  GlP->Data()[1][9] = -sz*s_30;
    GlP->Data()[0][11] = 0;     GlP->Data()[1][11] = 0;        GlP->Data()[2][11] = sz;
    n = vec3d(GlP->Data()[0][10], GlP->Data()[1][10], GlP->Data()[2][10]).XProdVec( 
        vec3d(GlP->Data()[0][11], GlP->Data()[1][11], GlP->Data()[2][11]) ).Normalise();
    GlP->Data()[3][9] = n[0];  GlP->Data()[3][10] = n[1];  GlP->Data()[3][11] = n[2];  
  }
  else  {
    GPC->AddObject(this);
  }
}
//..............................................................................
TXGrowPoint::~TXGrowPoint()  {  }
//..............................................................................
bool TXGrowPoint::Orient(TGlPrimitive *GlP)  {
  Parent()->GlTranslate(Center);
  Parent()->GlScale((float)Params()[0], (float)Params()[0], (float)Params()[0]);
  return false;
} 
//..............................................................................
void TXGrowPoint::SetRadius(float V)  {
  Params()[0] = V;
}
//..............................................................................
bool TXGrowPoint::GetDimensions(vec3d &Max, vec3d &Min)  {
  if( Center[0] > Max[0] )  Max[0] = Center[0];
  if( Center[1] > Max[1] )  Max[1] = Center[1];
  if( Center[2] > Max[2] )  Max[2] = Center[2];
  if( Center[0] < Min[0] )  Min[0] = Center[0];
  if( Center[1] < Min[1] )  Min[1] = Center[1];
  if( Center[2] < Min[2] )  Min[2] = Center[2];

  return true;
}
//..............................................................................

