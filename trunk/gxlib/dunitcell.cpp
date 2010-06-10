//----------------------------------------------------------------------------//
// TDUnitCell - a drawing object for unit cell
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "dunitcell.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"

TDUnitCell::TDUnitCell(TGlRenderer& R, const olxstr& collectionName) : 
AGDrawObject(R, collectionName) 
{
  SetSelectable(false);
  Reciprocal = false;
  FGlP = NULL;
  CellToCartesian.I();
  HklToCartesian.I();
  olxstr label_cn("duc_label");
  for( int i=0; i < 4; i++ )
    (Labels[i] = new TXGlLabel(Parent, label_cn))->SetVisible(false);
}
//...........................................................................
TDUnitCell::~TDUnitCell()  {
  for( int i=0; i < 4; i++ )
    delete Labels[i];
}
void TDUnitCell::Init(const double cell[6])  {
  if( cell[0] == 0 )  return;
  const double cG = cos(cell[5]/180*M_PI),
         cB = cos(cell[4]/180*M_PI),
         cA = cos(cell[3]/180*M_PI),
         sG = sin(cell[5]/180*M_PI),
         sB = sin(cell[4]/180*M_PI),
         sA = sin(cell[3]/180*M_PI);

  const double V = cell[0]*cell[1]*cell[2]*sqrt((1-cA*cA-cB*cB-cG*cG) + 2*(cA*cB*cG));

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

  mat3d M = v ? HklToCartesian : CellToCartesian;
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
  // reinitialise labels
  for( int i=0; i < 3; i++ )  {  
    Labels[i]->SetCenter(FGlP->Vertices[i*2+1]);
    Labels[i]->SetLabel(olxstr((char)('a'+i)));
  }
  Labels[3]->SetLabel('o');
}
//...........................................................................
void TDUnitCell::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  if( GPC.PrimitiveCount() != 0 )  {  // GetDimensions will be called 
    FGlP = GPC.FindPrimitiveByName("Lines");
    if( FGlP != NULL )  {
      GPC.AddObject(*this);
      return;
    }
    GPC.ClearPrimitives();
  }
  TGraphicsStyle& GS = GPC.GetStyle();
  FGlP = &GPC.NewPrimitive("Lines", sgloLines);
  TGlMaterial GlM;
  GlM.SetFlags(sglmAmbientF);
  GlM.AmbientF = 0;
  FGlP->SetProperties(GS.GetMaterial(FGlP->GetName(), GlM));
  FGlP->Vertices.SetCount(24);
  SetReciprocal(Reciprocal);
  GPC.AddObject(*this);
  for( int i=0; i < 4; i++ )
    Labels[i]->Create();
}
//..............................................................................
bool TDUnitCell::GetDimensions(vec3d &Max, vec3d &Min)  {
  if( FGlP == NULL )  return false;
  vec3d _min(100,100,100), _max(-100, -100, -100);
  for( int i=0; i < 8; i++ )
    vec3d::UpdateMinMax(GetVertex(i), _min, _max);
  Min = _min;  
  Max = _max;  
  return true;
}
//..............................................................................
bool TDUnitCell::Orient(TGlPrimitive& P)  {
  if( P.GetType() == sgloText )  {
    olxstr Str('O');
    const TGlFont& fnt = *Parent.GetScene().DefFont();
    vec3d T;
    const double tr = 0.003, 
      scale = Parent.GetBasis().GetZoom()/Parent.GetScale(),
      maxZ = Parent.GetMaxRasterZ()-0.001;
    if( !fnt.IsVectorFont() )  {
      vec3d cnt(Parent.GetBasis().GetCenter());
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
    }
    else  {
      vec3d cnt(Parent.GetBasis().GetCenter());
      T += tr;
      T += cnt;
      T *= Parent.GetBasis().GetMatrix();
      T *= Parent.GetBasis().GetZoom();
      T[2] = maxZ;
      fnt.DrawGlText(T, Str, Parent.GetBasis().GetZoom()*0.75/Parent.CalcZoom(), true);
      for( int i=0; i < 3; i++ )  {
        const int ind = i*2+1;
        T = FGlP->Vertices[ind];  
        for( int j=0; j < 3; j++ )
          T[j] -= (j==i ? -tr : tr);
        T += cnt;
        T *= Parent.GetBasis().GetMatrix();
        T *= Parent.GetBasis().GetZoom();
        T[2] = maxZ;
        Str[0] = (char)('a'+i);
        fnt.DrawGlText(T, Str, Parent.GetBasis().GetZoom()*0.75/Parent.CalcZoom(), true);
      }
    }
    return true;
  }
  return false;
}
//..............................................................................
void TDUnitCell::ListPrimitives(TStrList &List) const {}
//..............................................................................
void TDUnitCell::UpdatePrimitives(int32_t Mask, const ACreationParams* cpar)  {}
//..............................................................................
void TDUnitCell::SetLabelsFont(uint16_t fnt_index)  {
  for( int i=0; i < 4; i++ )
    Labels[i]->SetFontIndex(fnt_index);
}
//..............................................................................
void TDUnitCell::UpdateLabels()  {
  for( int i=0; i < 4; i++ )
    Labels[i]->SetLabel(Labels[i]->GetLabel());
}
//..............................................................................
void TDUnitCell::SetVisible(bool v)  {
  AGDrawObject::SetVisible(v);
  for( int i=0; i < 4; i++ )
    Labels[i]->SetVisible(v);
}
//..............................................................................
