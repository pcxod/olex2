/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dunitcell.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"
#include "povdraw.h"
#include "wrldraw.h"

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
//...........................................................................
void TDUnitCell::Init(const double cell[6])  {
  if( cell[0] == 0 )  return;
  const double cG = cos(cell[5]/180*M_PI),
         cB = cos(cell[4]/180*M_PI),
         cA = cos(cell[3]/180*M_PI),
         sG = sin(cell[5]/180*M_PI);

  const double V =
      cell[0]*cell[1]*cell[2]*sqrt((1-cA*cA-cB*cB-cG*cG) + 2*(cA*cB*cG));

  const double cs = cell[0]*cell[1]*sG/V;
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
void TDUnitCell::SetReciprocal(bool v, double scale)  {
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
  for (size_t i=0; i < FGlP->Vertices.Count(); i++)
    FGlP->Vertices[i] *= scale;
  // reinitialise labels
  const size_t FontIndex = Parent.GetScene().FindFontIndexForType<TDUnitCell>();
  for( int i=0; i < 3; i++ )  {  
    Labels[i]->SetFontIndex(FontIndex);
    Labels[i]->SetOffset(FGlP->Vertices[i*2+1]);
    Labels[i]->SetLabel(olxstr((char)('a'+i)));
  }
  Labels[3]->SetFontIndex(FontIndex);
  Labels[3]->SetLabel('o');
}
//...........................................................................
void TDUnitCell::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  if( GPC.PrimitiveCount() != 0 )  {  // GetDimensions will be called
    FGlP = GPC.FindPrimitiveByName("Lines");
    if( FGlP != NULL )  {
      GPC.AddObject(*this);
      for( int i=0; i < 4; i++ )
        Labels[i]->Create();
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
bool TDUnitCell::Orient(TGlPrimitive& P)  {  return false;  }
//..............................................................................
void TDUnitCell::ListPrimitives(TStrList &List) const {}
//..............................................................................
void TDUnitCell::UpdatePrimitives(int32_t Mask)  {}
//..............................................................................
void TDUnitCell::UpdateLabel()  {
  for( int i=0; i < 4; i++ )
    Labels[i]->Update();
}
//..............................................................................
void TDUnitCell::SetVisible(bool v)  {
  AGDrawObject::SetVisible(v);
  for( int i=0; i < 4; i++ )
    Labels[i]->SetVisible(v);
}
//..............................................................................
void TDUnitCell::ToDataItem(TDataItem& di) const {
  di.AddField("reciprocal", IsReciprocal());
  TDataItem& labels = di.AddItem("Labels");
  for( int i=0; i < 4; i++ )
    Labels[i]->ToDataItem(labels.AddItem(olxstr((olxch)('x'+i))));
}
//..............................................................................
void TDUnitCell::FromDataItem(const TDataItem& di)  {
  SetReciprocal(di.FindField("reciprocal").ToBool());
  const TDataItem& labels = di.GetItemByName("Labels");
  for( int i=0; i < 4; i++ )
    Labels[i]->FromDataItem(labels.GetItemByIndex(i));
}
//..............................................................................
const_strlist TDUnitCell::ToPov(olxdict<TGlMaterial, olxstr,
  TComparableComparator> &materials) const
{
  TStrList out;
  if (FGlP == NULL) return out;
  out.Add(" object { union {");
  const TGPCollection &gpc = GetPrimitives();
  pov::CrdTransformer crdc(Parent.GetBasis());
  olxstr p_mat = pov::get_mat_name(FGlP->GetProperties(), materials);
  for (int i=0; i < 24; i+=2) {
    out.Add("  object { cylinder {") << pov::to_str(crdc.crd(GetEdge(i))) <<
      ',' << pov::to_str(crdc.crd(GetEdge(i+1))) << ", 0.01} texture {" <<
      p_mat << "}}";
  }
  for (int i=0; i < 4; i++)
    out << Labels[i]->ToPov(materials);
  out.Add("  }}");
  return out;
}
//..............................................................................
const_strlist TDUnitCell::ToWrl(olxdict<TGlMaterial, olxstr,
  TComparableComparator> &materials) const
{
  TStrList out;
  if (FGlP == NULL) return out;
  olxstr p_mat = wrl::get_mat_str(FGlP->GetProperties(), materials);
  out.Add(" Group{ children[ Shape{ appearance ") << p_mat <<
    " geometry IndexedLineSet{ colorPerVertex FALSE coord Coordinate{ point[";
  const TGPCollection &gpc = GetPrimitives();
  wrl::CrdTransformer crdc(Parent.GetBasis());
  for (size_t i=0; i < VertexCount(); i++) {
    out.Add("  ") << wrl::to_str(crdc.crd(GetVertex(i)));
    if (i+1 < VertexCount())
      out.GetLastString() << ',';
  }
  out.GetLastString() << "]}";
  out.Add("  coordIndex[0 1 5 3 0 2 4 7 6 2 -1 3 6 -1 5 7 -1 1 4 -1]");
  out.Add("  color Color{ color[") <<
    wrl::to_str(FGlP->GetProperties().AmbientF) << "]}";
  out.Add("  colorIndex[0 0 0 0]}}");
  for (int i=0; i < 4; i++)
    out << Labels[i]->ToWrl(materials);
  out.Add(" ]}");
  return out;
}
//..............................................................................
