//----------------------------------------------------------------------------//
// namespace TEXLib
// TDUnitCell - a drawing object for unit cell
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "dunitcell.h"

#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "glscene.h"
#include "gpcollection.h"

#include "styles.h"


TDUnitCell::TDUnitCell(TGlRenderer& R, const olxstr& collectionName) : 
AGDrawObject(R, collectionName) 
{
  SetGroupable(false);
  Reciprocal = false;
  FGlP = NULL;
  // FCenter[0] == M_PI - the object is not initialised
  OldCenter[0] = OldCenter[1] = OldCenter[2] = 100;
  CellToCartesian.I();
  HklToCartesian.I();
}
//...........................................................................
void TDUnitCell::ResetCentres()  {
  Center.Null();
  OldCenter[0] = OldCenter[1] = OldCenter[2] = 100;
}
//...........................................................................
void TDUnitCell::Init(const double cell[6])  {
  if( cell[0] == 0 )  return;
  const double cG = cos(cell[5]/180*M_PI),
         cB = cos(cell[4]/180*M_PI),
         cA = cos(cell[3]/180*M_PI),
         sG = sin(cell[5]/180*M_PI),
         sB = sin(cell[4]/180*M_PI),
         sA = sin(cell[3]/180*M_PI);

  const double V = cell[0]*cell[1]*cell[2]*sqrt( (1-cA*cA-cB*cB-cG*cG) + 2*(cA*cB*cG));

  const double cGs = (cA*cB-cG)/(sA*sB),
         cBs = (cA*cG-cB)/(sA*sG),
         cAs = (cB*cG-cA)/(sB*sG),
         as = cell[1]*cell[2]*sA/V,
         bs = cell[0]*cell[2]*sB/V,
         cs = cell[0]*cell[1]*sG/V;
  // cell to cartesian transformation matrix
  CellToCartesian.I();
  CellToCartesian[0][0] = cell[0];
  CellToCartesian[1][0] = cell[1]*cG;
  CellToCartesian[2][0] = cell[2]*cB;

  CellToCartesian[1][1] = cell[1]*sG;
  CellToCartesian[2][1] = -cell[2]*(cB*cG-cA)/sG;

  CellToCartesian[2][2] = 1./cs;

  vec3d v0(CellToCartesian[0]), v1(CellToCartesian[1]), v2(CellToCartesian[2]);

  HklToCartesian[0] = v1.XProdVec(v2);
  HklToCartesian[1] = v2.XProdVec(v0);
  HklToCartesian[2] = v0.XProdVec(v1);
  HklToCartesian[0] /= V;
  HklToCartesian[1] /= V;
  HklToCartesian[2] /= V;
}
//...........................................................................
void TDUnitCell::SetReciprocal(bool v)  {
  if( FGlP == NULL )  return;
  mat3d M;
  if( v )  {
    OldCenter = Center;
    M = HklToCartesian;
    //M.Transpose();
    vec3d scaleV(1, 1, 1);
    scaleV = M*scaleV;
    double scale = olx_max(scaleV[2],  olx_max(scaleV[0], scaleV[1]) );
    // with this scale it will cover 10 reflections
    M *= (10.0/scale);                    // extra scaling;
    Center = (M[0] + M[1] + M[2])*(-0.5);
  }
  else  {
    M = CellToCartesian;
    if( OldCenter.Length() == 1000000 )
      Center = OldCenter;
  }

  FGlP->Vertices[1] = M[0];  //000-A
  FGlP->Vertices[3] = M[1];  //000-B
  FGlP->Vertices[5] = M[2];  //000-C

  FGlP->Vertices[6] = M[0];  //A
  FGlP->Vertices[7] = M[1]+M[0];  //AB

  FGlP->Vertices[8] = M[0];  //A
  FGlP->Vertices[9] = M[2]+M[0];  //AC

  FGlP->Vertices[10] = M[1];  //B
  FGlP->Vertices[11] = M[1]+M[0];  //AB

  FGlP->Vertices[12] = M[1];  //B
  FGlP->Vertices[13] = M[2]+M[1];  //BC

  FGlP->Vertices[14] = M[2];  //C
  FGlP->Vertices[15] = M[2]+M[0];  //AC

  FGlP->Vertices[16] = M[2];  //C
  FGlP->Vertices[17] = M[2]+M[1];  //BC

  FGlP->Vertices[18] = M[1]+M[0];  //AB
  FGlP->Vertices[19] = M[2]+M[0]+M[1] ;  //ABC

  FGlP->Vertices[20] = M[2]+M[1];  //BC
  FGlP->Vertices[21] = M[2]+M[1]+M[0] ;  //ABC

  FGlP->Vertices[22] = M[2]+M[0];  //AC
  FGlP->Vertices[23] = M[2]+M[1]+M[0];  //ABC

  Reciprocal = v;
}

void TDUnitCell::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX( GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = &Parent.NewCollection(NewL);
  if( GPC->PrimitiveCount() == 0 )  {
    TGraphicsStyle& GS = GPC->GetStyle();
    FGlP = &GPC->NewPrimitive("Lines", sgloLines);
    TGlMaterial GlM;
    GlM.SetFlags(sglmAmbientF);
    GlM.AmbientF = 0;

    FGlP->SetProperties( GS.GetMaterial("Lines", GlM));
    FGlP->Vertices.SetCount(24);
    SetReciprocal(Reciprocal);

    TGlPrimitive& glpLabel = GPC->NewPrimitive("Label", sgloText);  // labels

    TGlMaterial lMat;
    lMat.SetFlags(sglmAmbientF);
    lMat.AmbientF = 0xff00ff;
    lMat.SetIdentityDraw(true);
    glpLabel.SetProperties( GS.GetMaterial("Label", lMat) );

    glpLabel.SetFont( Parent.GetScene().DefFont() );
  }
  GPC->AddObject(*this);
}
//..............................................................................
bool TDUnitCell::GetDimensions(vec3d &Max, vec3d &Min)  {
  if( FGlP == NULL )  return false;
  vec3f _min(100,100,100), _max(-100, -100, -100);
  for( int i=0; i < 8; i++ )
    vec3f::UpdateMinMax(GetVertex(i), _min, _max);
  Min = vec3d(_min) + Center;  
  Max = vec3d(_max) + Center;  
  return true;
}
//..............................................................................
bool TDUnitCell::Orient(TGlPrimitive& P)  {
  if( P.GetType() == sgloText )  {
    olxstr Str('O');
    const TGlFont& fnt = *Parent.GetScene().DefFont();
    vec3d T;
    const double tr = 0.3, 
      scale = Parent.GetBasis().GetZoom()/Parent.GetScale(),
      maxZ = Parent.GetMaxRasterZ();
    vec3d cnt(Parent.GetBasis().GetCenter());
    cnt += Center;
    T += tr;
    T += cnt;
    T *= Parent.GetBasis().GetMatrix();
    T *= scale;
    T[2] = maxZ;
    Parent.DrawTextSafe(T, Str, fnt);
    for( int i=0; i < 3; i++ )  {
      const int ind = i*2+1;
      T = FGlP->Vertices[ind];  
      for( int j=0; j < 3; j++ )
        T[j] -= (j==i ? -tr : tr);
      T += cnt;
      T *= Parent.GetBasis().GetMatrix();
      T *= scale;
      T[2] = maxZ;
      Str[0] = (char)('a'+i);
      Parent.DrawTextSafe(T, Str, fnt);
    }
    return true;
  }
  Parent.GlTranslate(Center);
/*
  const TMatrixD& m = FAU->GetCellToCartesian();
  const TMatrixD& n = FAU->GetCartesianToCell();
  TMatrixD rm(3,3), rm1(3,3);
   glDisable(GL_CULL_FACE);
   vec3d v, p;
   p = n[2];
   p.Normalise();
   p *= 0.5;
   // current normal
   v = m[0];   v.Normalise();  
   CreateRotationMatrix(rm, v, 0.5);
   CreateRotationMatrix(rm1, v, cos(18.0/180*M_PI));
   v *= -1;
   glNormal3d(v[0], v[1], v[2]);
   //
   glBegin(GL_POLYGON);
   for( int i=0; i < 6; i++ )  {
     if( i != 0 )
       p *= rm;
     glVertex3d(p[0], p[1], p[2]);

   }
   glEnd();
   //
   glTranslated(-v[0], -v[1], -v[2]);
   glScaled(1, 0.25, 1);
   p = n[2];
   p.Normalise();
   p *= 0.25;
   glBegin(GL_POLYGON);
   for( int i=0; i < 20; i++ )  {
     if( i )  p *= rm1;
     glVertex3d(p[0], p[1], p[2]);

   }
   glEnd();


   glBegin(GL_QUADS);
   v = m[0];   v /= 2;
    glVertex3d(v[0], v[1], v[2]);
   v = m[0];   v /= 2;  v += m[2];
    glVertex3d(v[0], v[1], v[2]);
   v = m[0];   v /= 2;  v += m[2];  v += m[1];
    glVertex3d(v[0], v[1], v[2]);
   v = m[0];   v /= 2;  v += m[1];
    glVertex3d(v[0], v[1], v[2]);
   glEnd();
   glEnable(GL_CULL_FACE);
*/
   return false;
}

