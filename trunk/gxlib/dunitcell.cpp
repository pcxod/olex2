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


TDUnitCell::TDUnitCell(const olxstr& collectionName, TGlRenderer *Render) : AGDrawObject(collectionName) {
  FParent = Render;
  Groupable(false);
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

  FGlP->Data[0][1] = M[0][0];  //000-A
  FGlP->Data[1][1] = M[0][1];
  FGlP->Data[2][1] = M[0][2];
  FGlP->Data[0][3] = M[1][0];  //000-B
  FGlP->Data[1][3] = M[1][1];
  FGlP->Data[2][3] = M[1][2];
  FGlP->Data[0][5] = M[2][0];  //000-C
  FGlP->Data[1][5] = M[2][1];
  FGlP->Data[2][5] = M[2][2];

  FGlP->Data[0][6] = M[0][0];  //A
  FGlP->Data[1][6] = M[0][1];
  FGlP->Data[2][6] = M[0][2];
  FGlP->Data[0][7] = M[1][0]+M[0][0];  //AB
  FGlP->Data[1][7] = M[1][1]+M[0][1];
  FGlP->Data[2][7] = M[1][2]+M[0][2];

  FGlP->Data[0][8] = M[0][0];  //A
  FGlP->Data[1][8] = M[0][1];
  FGlP->Data[2][8] = M[0][2];
  FGlP->Data[0][9] = M[2][0]+M[0][0];  //AC
  FGlP->Data[1][9] = M[2][1]+M[0][1];
  FGlP->Data[2][9] = M[2][2]+M[0][2];

  FGlP->Data[0][10] = M[1][0];  //B
  FGlP->Data[1][10] = M[1][1];
  FGlP->Data[2][10] = M[1][2];
  FGlP->Data[0][11] = M[1][0]+M[0][0];  //AB
  FGlP->Data[1][11] = M[1][1]+M[0][1];
  FGlP->Data[2][11] = M[1][2]+M[0][2];

  FGlP->Data[0][12] = M[1][0];  //B
  FGlP->Data[1][12] = M[1][1];
  FGlP->Data[2][12] = M[1][2];
  FGlP->Data[0][13] = M[2][0]+M[1][0];  //BC
  FGlP->Data[1][13] = M[2][1]+M[1][1];
  FGlP->Data[2][13] = M[2][2]+M[1][2];

  FGlP->Data[0][14] = M[2][0];  //C
  FGlP->Data[1][14] = M[2][1];
  FGlP->Data[2][14] = M[2][2];
  FGlP->Data[0][15] = M[2][0]+M[0][0];  //AC
  FGlP->Data[1][15] = M[2][1]+M[0][1];
  FGlP->Data[2][15] = M[2][2]+M[0][2];

  FGlP->Data[0][16] = M[2][0];  //C
  FGlP->Data[1][16] = M[2][1];
  FGlP->Data[2][16] = M[2][2];
  FGlP->Data[0][17] = M[2][0]+M[1][0];  //BC
  FGlP->Data[1][17] = M[2][1]+M[1][1];
  FGlP->Data[2][17] = M[2][2]+M[1][2];

  FGlP->Data[0][18] = M[1][0]+M[0][0];  //AB
  FGlP->Data[1][18] = M[1][1]+M[0][1];
  FGlP->Data[2][18] = M[1][2]+M[0][2];
  FGlP->Data[0][19] = M[2][0]+M[0][0]+M[1][0] ;  //ABC
  FGlP->Data[1][19] = M[2][1]+M[1][1]+M[0][1];
  FGlP->Data[2][19] = M[2][2]+M[1][2]+M[0][2];

  FGlP->Data[0][20] = M[2][0]+M[1][0];  //BC
  FGlP->Data[1][20] = M[2][1]+M[1][1];
  FGlP->Data[2][20] = M[2][2]+M[1][2];
  FGlP->Data[0][21] = M[2][0]+M[1][0]+M[0][0] ;  //ABC
  FGlP->Data[1][21] = M[2][1]+M[1][1]+M[0][1];
  FGlP->Data[2][21] = M[2][2]+M[1][2]+M[0][2];

  FGlP->Data[0][22] = M[2][0]+M[0][0];  //AC
  FGlP->Data[1][22] = M[2][1]+M[0][1];
  FGlP->Data[2][22] = M[2][2]+M[0][2];
  FGlP->Data[0][23] = M[2][0]+M[1][0]+M[0][0];  //ABC
  FGlP->Data[1][23] = M[2][1]+M[1][1]+M[0][1];
  FGlP->Data[2][23] = M[2][2]+M[1][2]+M[0][2];

  Reciprocal = v;
}

void TDUnitCell::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = FParent->CollectionX( GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = FParent->NewCollection(NewL);
  GPC->AddObject(this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGraphicsStyle* GS = GPC->Style();
  FGlP = GPC->NewPrimitive("Lines", sgloLines);
  TGlMaterial* SGlM = const_cast<TGlMaterial*>(GS->Material("Lines"));
  if( SGlM->Mark() )  {
    SGlM->SetFlags(sglmAmbientF);
    SGlM->AmbientF = 0;
  }
  FGlP->SetProperties(SGlM);

  FGlP->Data.Resize(3, 24);
  SetReciprocal(Reciprocal);

  TGlPrimitive* GlP = GPC->NewPrimitive("Label", sgloText);  // labels
  SGlM = const_cast<TGlMaterial*>(GS->Material("Label"));
  if( SGlM->Mark() )  {
    SGlM->SetFlags(sglmAmbientF);
    SGlM->AmbientF = 0xff00ff;
    SGlM->SetIdentityDraw(true);
  }
  GlP->SetProperties(SGlM);

  GlP->SetFont( Parent()->Scene()->DefFont() );
}
//..............................................................................
bool TDUnitCell::GetDimensions(vec3d &Max, vec3d &Min)  {
  //Min[0] = FGlP->Data[0][1];  
  //Min[1] = FGlP->Data[1][1];  
  //Min[2] = FGlP->Data[2][1];
  //Max[0] = FGlP->Data[0][23];  
  //Max[1] = FGlP->Data[1][23];  
  //Max[2] = FGlP->Data[2][23];
  return false;
}
//..............................................................................
bool TDUnitCell::Orient(TGlPrimitive *P)  {
  if( P->GetType() == sgloText )  {
    olxstr Str('O');
    const TGlFont& fnt = *Parent()->Scene()->DefFont();
    vec3d T;
    const double tr = 0.3, 
      scale = 1./FParent->GetScale();;

    vec3d cnt( FParent->GetBasis().GetCenter() );
    cnt += Center;
    T += tr;
    T += cnt;
    T *= FParent->GetBasis().GetMatrix();
    T *= scale;
    FParent->DrawTextSafe(T, Str, fnt);
    for( int i=0; i < 3; i++ )  {
      const int ind = i*2+1;
      T[0] = FGlP->Data[0][ind];  
      T[1] = FGlP->Data[1][ind];  
      T[2] = FGlP->Data[2][ind];
      for( int j=0; j < 3; j++ )
        T[j] -= (j==i ? -tr : tr);
      T += cnt;
      T *= FParent->GetBasis().GetMatrix();
      T *= scale;
      Str[0] = (char)('a'+i);
      FParent->DrawTextSafe(T, Str, fnt);
    }
    return true;
  }
  Parent()->GlTranslate(Center);
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

