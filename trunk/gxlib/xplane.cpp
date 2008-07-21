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

//..............................................................................
TXPlane::TXPlane(const olxstr& collectionName, TSPlane *Plane, TGlRender *Render) :
  AGDrawObject(collectionName)
{
  FPlane = Plane;
  FParent = Render;
  FRectangular = false;
}
//..............................................................................
void TXPlane::Create(const olxstr& cName)
{
  if( cName.Length() != 0 )  SetCollectionName(cName);
  TGlPrimitive *GlP;
  TGPCollection *GPC;
  TGlMaterial GlM, GlM1;
  TEList PSort;
  TPlaneSort *PS;
  vec3d Center;
  GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);

  GlM.SetFlags( sglmAmbientF|sglmDiffuseF|sglmAmbientB|sglmDiffuseB|sglmTransparent);
  GlM1.SetFlags( sglmAmbientF );
  GlM1.AmbientF = 0;

  GlP = GPC->NewPrimitive("Plane");
  GlM.AmbientF = 0x7f00007f;
  GlM.DiffuseF = 0x7f3f3f3f;
  GlM.AmbientB = 0x7f00007f;
  GlM.DiffuseB = 0x7f3f3f3f;
  GlP->SetProperties(&GlM);
  GlP->Type(sgloPolygon);
  if( !FRectangular )  GlP->Data().Resize(3, FPlane->CrdCount());
  else                 GlP->Data().Resize(3, 4);
  Center = FPlane->Center();
  for( int i=0; i < FPlane->CrdCount(); i++ )  {
    PS = new TPlaneSort;
    PS->V = FPlane->Crd(i);
    PS->V -= Center;
    PS->V.Normalise();
    PS->Crd = &FPlane->Crd(i);
    PSort.Add(PS);
  }

  PSort.Sort(PlaneSort);
  if( !FRectangular )  {
    for( int i=0; i < PSort.Count(); i++ )  {
      PS = (TPlaneSort*)PSort.Item(i);
      GlP->Data()[0][i] = (*PS->Crd)[0]-Center[0];
      GlP->Data()[1][i] = (*PS->Crd)[1]-Center[1];
      GlP->Data()[2][i] = -FPlane->Z((*PS->Crd)[0], (*PS->Crd)[1])-Center[2];
      delete PS;
    }
  }
  else  {
    double maxX=-1000, maxY=-1000, minX=1000, minY=1000;
    double x, y;
    for( int i=0; i < PSort.Count(); i++ )  {
      PS = (TPlaneSort*)PSort.Item(i);
      x = (*PS->Crd)[0]-Center[0];
      y = (*PS->Crd)[1]-Center[1];
      if( x < minX )  minX = x;
      if( x > maxX )  maxX = x;
      if( y < minY )  minY = y;
      if( y > maxY )  maxY = y;
      delete PS;
    }
    GlP->Data()[0][0] = minX;    GlP->Data()[1][0] = minY;
    GlP->Data()[2][0] = -FPlane->Z(minX + Center[0], minY + Center[1])-Center[2];

    GlP->Data()[0][1] = maxX;    GlP->Data()[1][1] = minY;
    GlP->Data()[2][1] = -FPlane->Z(maxX + Center[0], minY + Center[1])-Center[2];

    GlP->Data()[0][2] = maxX;    GlP->Data()[1][2] = maxY;
    GlP->Data()[2][2] = -FPlane->Z(maxX + Center[0], maxY + Center[1])-Center[2];

    GlP->Data()[0][3] = minX;    GlP->Data()[1][3] = maxY;
    GlP->Data()[2][3] = -FPlane->Z(minX + Center[0], maxY + Center[1])-Center[2];
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
int TXPlane::PlaneSort(const void *I, const void *I1)
{
  // function works with normalised values and calculates 2PI angle
  // to make comparison
  double v = acos( ((TPlaneSort*)I)->V[0] );
  if( ((TPlaneSort*)I)->V[1] < 0 )    v = 2*M_PI-v;
  double v1 = acos( ((TPlaneSort*)I1)->V[0] );
  if( ((TPlaneSort*)I1)->V[1] < 0 )    v1 = 2*M_PI-v1;
  v1 -= v;
  if( v1 < 0 )  return -1;
  if( v1 > 0 )  return 1;
  return 0;
}
//..............................................................................

 
 
