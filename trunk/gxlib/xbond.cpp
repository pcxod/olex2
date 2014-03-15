/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xbond.h"
#include "gpcollection.h"
#include "xatom.h"
#include "lattice.h"
#include "symmparser.h"
#include "unitcell.h"
#include "povdraw.h"
#include "wrldraw.h"

bool TXBond::TStylesClear::Enter(const IEObject *Sender, const IEObject *Data,
  TActionQueue *)
{
  TXBond::FBondParams = NULL;
  TXBond::ClearStaticObjects();
  return true;
}
//..............................................................................
bool TXBond::TStylesClear::Exit(const IEObject *Sender, const IEObject *Data,
  TActionQueue *)
{
  TXBond::ValidateBondParams();
  TXBond::ClearStaticObjects();
  return true;
}
//..............................................................................
//..............................................................................
TXBond::TContextClear::TContextClear(TGlRenderer& Render)  {
  Render.OnClear.Add(this);
}
//..............................................................................
bool TXBond::TContextClear::Enter(const IEObject *Sender, const IEObject *Data,
  TActionQueue *)
{
  TXBond::ClearStaticObjects();
  return true;
}
//..............................................................................
bool TXBond::TContextClear::Exit(const IEObject *Sender, const IEObject *Data,
  TActionQueue *)
{
  return true;
}
//..............................................................................
//----------------------------------------------------------------------------//
// TXBond function bodies
//----------------------------------------------------------------------------//
TGraphicsStyle* TXBond::FBondParams=NULL;
TXBond::TStylesClear *TXBond::OnStylesClear=NULL;
double TXBond::FDefR = -1;
int TXBond::FDefM = -1;
bool TXBond::DefSelectable = true;
//..............................................................................
TXBond::TXBond(TNetwork* net, TGlRenderer& R, const olxstr& collectionName) :
  TSBond(net),
  AGDrawObject(R, collectionName),
  FDrawStyle(0x0001)
{
  SetGroupable(true);
  SetSelectable(DefSelectable);
  Params().Resize(5);
  Params()[4] = 0.8;
  Label = new TXGlLabel(GetParent(), PLabelsCollectionName);
  Label->SetVisible(false);
  label_forced = false;
}
//..............................................................................
TXBond::~TXBond()  {
  if (GetParentGroup() != NULL) {
    GetParentGroup()->Remove(*this);
  }
  delete Label;
}
//..............................................................................
void TXBond::Update()  {
  if( !IsValid() )  return;
  vec3d C(B().crd() - A().crd());
  if( C.IsNull() )
    Params().Null();
  else  {
    Params()[3] = C.Length();
    C /= Params()[3];
    Params()[0] = acos(C[2])*180/M_PI;
    if( olx_abs(Params()[0]-180) < 1e-3 )  { // degenerate case with Pi rotation
      Params()[1] = 0;
      Params()[2] = 1;
    }
    else {
      Params()[1] = -C[1];
      Params()[2] = C[0];
    }
  }
}
//..............................................................................
void TXBond::Create(const olxstr& cName)  {
  SetCreated(true);
  if (!cName.IsEmpty())
    SetCollectionName(cName);
  if (GetStaticPrimitives().IsEmpty())
    CreateStaticObjects(Parent);
  Label->SetFontIndex(Parent.GetScene().FindFontIndexForType<TXBond>());
  Label->Create();
  // find collection
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = &Parent.NewCollection(NewL);
  else if( GPC->PrimitiveCount() != 0 )  {
    GPC->AddObject(*this);
    Params()[4] = GPC->GetStyle().GetNumParam('R', DefR());
    return;
  }
  TGraphicsStyle& GS = GPC->GetStyle();
  GS.SetSaveable(IsStyleSaveable());

  const int PrimitiveMask = GS.GetNumParam(GetPrimitiveMaskName(),
    (GetType() == sotHBond) ? 2048 : DefMask(), IsMaskSaveable());

  GPC->AddObject(*this);
  if( PrimitiveMask == 0 )
    return;  // nothing to create then...

  Params()[4]= GS.GetNumParam('R', DefR());
  const uint16_t legend_level = TXAtom::LegendLevel(GetPrimitives().GetName());
  const TStringToList<olxstr, TGlPrimitive*> &primitives = GetStaticPrimitives();
  for (size_t i=0; i < primitives.Count(); i++) {
    if( (PrimitiveMask & (1<<i)) != 0 ) {
      TGlPrimitive* SGlP = primitives.GetObject(i);
      TGlPrimitive& GlP = GPC->NewPrimitive(primitives[i], sgloCommandList);
      if (i >= 14) {
        GlP.SetOwnerId(i);
      }
      /* copy the default drawing style tag source */
      GlP.Params.Resize(GlP.Params.Count()+1);
      GlP.Params.GetLast() = SGlP->Params.GetLast();

      GlP.StartList();
      GlP.CallList(SGlP);
      GlP.EndList();
      TGlMaterial* style_mat =
        legend_level == 3 ? GS.FindMaterial(primitives[i]) : NULL;
      if( IsValid() )  {
        if( style_mat != NULL )
          GlP.SetProperties(*style_mat);
        else  {
          TGlMaterial RGlM;
          if( SGlP->Params.GetLast() == ddsDefAtomA || SGlP->Params.GetLast() == ddsDef )  {
            if( !A().IsCreated() )
              A().Create();
            const size_t mi = A().Style().IndexOfMaterial("Sphere");
            if( mi != InvalidIndex )
              RGlM = A().Style().GetPrimitiveStyle(mi).GetProperties();
            else
              TXAtom::GetDefSphereMaterial(A(), RGlM);
          }
          else if( SGlP->Params.GetLast() == ddsDefAtomB )  {
            if( !B().IsCreated() )
              B().Create();
            const size_t mi = B().Style().IndexOfMaterial("Sphere");
            if( mi != InvalidIndex )
              RGlM = B().Style().GetPrimitiveStyle(mi).GetProperties();
            else
              TXAtom::GetDefSphereMaterial(B(), RGlM);
          }
          if( legend_level == 4 )
            GlP.SetProperties(GS.GetMaterial(primitives[i], RGlM));
          else // must be updated from atoms always
            GlP.SetProperties(RGlM);
        }
      }
      else  {  // no atoms
        GlP.SetProperties(GS.GetMaterial(primitives[i],
          TGlMaterial("85;2155839359;2155313015;1.000,1.000,1.000,0.502;36")));
      }
    }
  }
}
//..............................................................................
void TXBond::UpdateStyle()  {
  TGPCollection &gpc = GetPrimitives();
  const uint16_t legend_level = TXAtom::LegendLevel(gpc.GetName());
  if( legend_level == 3 )  // is user managed?
    return;
  TGraphicsStyle& GS = gpc.GetStyle();
  const int PrimitiveMask = GS.GetNumParam(GetPrimitiveMaskName(),
    (GetType() == sotHBond) ? 2048 : DefMask(), IsMaskSaveable());
  const TStringToList<olxstr, TGlPrimitive*> &primitives = GetStaticPrimitives();
  for (size_t i = 0; i < primitives.Count(); i++) {
    if( (PrimitiveMask & (1<<i)) != 0 )  {
      TGlPrimitive *SGlP = primitives.GetObject(i);
      TGlPrimitive *GlP = gpc.FindPrimitiveByName(primitives[i]);
      if (GlP == NULL)  // must not ever happen...
        continue;
      if( IsValid() )  {
        TGlMaterial RGlM;
        if( SGlP->Params.GetLast() == ddsDefAtomA || SGlP->Params.GetLast() == ddsDef )  {
          if( !A().IsCreated() )
            A().Create();
          const size_t mi = A().Style().IndexOfMaterial("Sphere");
          if( mi != InvalidIndex )
            RGlM = A().Style().GetPrimitiveStyle(mi).GetProperties();
          else
            TXAtom::GetDefSphereMaterial(A(), RGlM);
        }
        else if( SGlP->Params.GetLast() == ddsDefAtomB )  {
          if( !B().IsCreated() )
            B().Create();
          const size_t mi = B().Style().IndexOfMaterial("Sphere");
          if( mi != InvalidIndex )
            RGlM = B().Style().GetPrimitiveStyle(mi).GetProperties();
          else
            TXAtom::GetDefSphereMaterial(B(), RGlM);
        }
        GlP->SetProperties(RGlM);
      }
      else  {  // no atoms
        GlP->SetProperties(GS.GetMaterial(primitives[i],
          TGlMaterial("85;2155839359;2155313015;1.000,1.000,1.000,0.502;36")));
      }
    }
  }
}
//..............................................................................
bool TXBond::Orient(TGlPrimitive& GlP)  {
  if (GlP.GetOwnerId() >= 14 && GlP.GetOwnerId() <= 18) {
    vec3d v = (B().crd() - A().crd()).Normalise();
    const mat3d& m = Parent.GetBasis().GetMatrix();
    v = m*v*m;
    v = vec3d(m[0][2], m[1][2], m[2][2]).XProdVec(v);
    double vl = v.Length();
    if (vl > 0) {
      v *= (0.1*Params()[4] / vl);
      if (GlP.GetOwnerId() > 15) {
        olx_gl::pushMatrix();
        olx_gl::translate(A().crd());
        olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
        olx_gl::scale(Params()[4], Params()[4], Params()[3]);
        GlP.Draw();
        olx_gl::popMatrix();
      }
      olx_gl::pushMatrix();
      olx_gl::translate(A().crd() + v);
      olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
      olx_gl::scale(Params()[4], Params()[4], Params()[3]);
      GlP.Draw();
      olx_gl::popMatrix();
      olx_gl::translate(A().crd() - v);
      olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
      olx_gl::scale(Params()[4], Params()[4], Params()[3]);
      GlP.Draw();
    }
    return true;
  }
  else {
    olx_gl::translate(A().crd());
    olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
    olx_gl::scale(Params()[4], Params()[4], Params()[3]);
  }
  return false;
}
//..............................................................................
void TXBond::ListParams(TStrList &List, TGlPrimitive *Primitive)  {
}
//..............................................................................
void TXBond::ListParams(TStrList &List)  {
}
//..............................................................................
void TXBond::UpdatePrimitiveParams(TGlPrimitive *Primitive)  {
}
//..............................................................................
void TXBond::ListPrimitives(TStrList &List) const {
  List.Assign(GetStaticPrimitives());
}
//..............................................................................
int16_t TXBond::Quality(int16_t Val)  {
  static int16_t previous_quality = -1;
  if (Val == -1) Val = qaMedium;
  ValidateBondParams();
  olxstr& ConeQ = FBondParams->GetParam("ConeQ", "15", true);
  switch( Val )  {
    case qaPict:
    case qaHigh:   ConeQ = 30;  break;
    case qaMedium: ConeQ = 15;  break;
    case qaLow:    ConeQ = 5;  break;
  }
  int16_t pq = previous_quality;
  previous_quality = Val;
  return pq;
}
//..............................................................................
void TXBond::ListDrawingStyles(TStrList &L){  return; }
//..............................................................................
const vec3d &TXBond::GetBaseCrd() const {
  if( !IsValid() )
    throw TFunctionFailedException(__OlxSourceInfo, "atoms are not defined");
  return A().crd();
}
//..............................................................................
const_strlist TXBond::ToPov(olxdict<TGlMaterial, olxstr,
  TComparableComparator> &materials) const
{
  TStrList out;
  if (olx_abs(Params()[1]) + olx_abs(Params()[2]) < 1e-3)
    return out;
  out.Add(" object { union {");
  const TGPCollection &gpc = GetPrimitives();
  for (size_t i=0; i < gpc.PrimitiveCount(); i++) {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    olxstr p_mat = pov::get_mat_name(glp.GetProperties(), materials, this);
    out.Add("  object {") << "bond_" << glp.GetName().ToLowerCase().Replace(' ', '_')
      << " texture {" << p_mat << "}}";
  }
  out.Add("  }");
  mat3d m;
  olx_create_rotation_matrix(m, vec3d(Params()[1], Params()[2], 0).Normalise(),
    cos(Params()[0]*M_PI/180));
  const mat3d &b = Parent.GetBasis().GetMatrix();
  m *= b;

  vec3d x(1, 0, 0), v = x*m;
  if (!v.IsParallel(x)) {
    mat3d cm;
    vec3d d = vec3d(B().crd() - A().crd()).Normalise()*b;
    olx_create_rotation_matrix(cm, -d, v.CAngle(d.Normal(x)));
    m *= cm;
  }
  m[0] *= Params()[4];
  m[1] *= Params()[4];
  m[2] *= Params()[3];
  vec3d t = pov::CrdTransformer(Parent.GetBasis()).crd(GetBaseCrd());
  out.Add("  transform {");
  out.Add("   matrix") << pov::to_str(m, t);
  out.Add("   }");
  out.Add(" }");
  return out;
}
//..............................................................................
const_strlist TXBond::PovDeclare()  {
  TStrList out;
  out.Add("#declare bond_single_cone=object{ cylinder {<0,0,0>, <0,0,1>, 0.1} }");
  out.Add("#declare bond_top_disk=object{ disc {<0,0,1><0,0,1>, 0.1} }");
  out.Add("#declare bond_bottom_disk=object{ disc {<0,0,0><0,0,-1>, 0.1} }");
  out.Add("#declare bond_middle_disk=object{ disc {<0,0,0.5><0,0,1>, 0.1} }");
  out.Add("#declare bond_bottom_cone=object{ cylinder {<0,0,0>, <0,0,0.5>, 0.1} }");
  out.Add("#declare bond_top_cone=object{ cylinder {<0,0,0.5>, <0,0,1>, 0.1} }");
  out.Add("#declare bond_bottom_line=object{ cylinder {<0,0,0>, <0,0,0.5>, 0.01} }");
  out.Add("#declare bond_top_line=object{ cylinder {<0,0,0.5>, <0,0,1>, 0.01} }");

  double ConeStipples = FBondParams->GetNumParam("ConeStipples", 6.0, true);
  out.Add("#declare bond_stipple_cone=object { union {");
  for( double i=0; i < ConeStipples; i++ )  {
    out.Add(" disc {<0,0,") << i/ConeStipples << "><0,0,-1>, 0.1}";
    out.Add(" cylinder {<0,0,") << i/ConeStipples << ">, <0,0,"
      << (i+0.5)/ConeStipples << ">, 0.1}";
    out.Add(" disc {<0,0,") << (double)(i+0.5)/ConeStipples << "><0,0,1>, 0.1}";
  }
  out.Add("}}");
  
  out.Add("#declare bond_bottom_stipple_cone=object { union {");
  for( double i=0; i < ConeStipples/2; i++ )  {
    out.Add(" disc {<0,0,") << (i+0.5)/ConeStipples << "><0,0,-1>, 0.1}";
    out.Add(" cylinder {<0,0,") << (i+0.5)/ConeStipples << ">, <0,0,"
      << (i+1)/ConeStipples << ">, 0.1}";
    out.Add(" disc {<0,0,") << (i+1)/ConeStipples << "><0,0,1>, 0.1}";
  }
  out.Add("}}");

  out.Add("#declare bond_top_stipple_cone=object { union {");
  for( int i=0; i < ConeStipples/2; i++ )  {
    out.Add(" disc {<0,0,") << ((i+0.5)/ConeStipples+0.5) << "><0,0,-1>, 0.1}";
    out.Add(" cylinder {<0,0,") << ((i+0.5)/ConeStipples+0.5) << ">, <0,0,"
      << ((i+1)/ConeStipples+0.5) << ">, 0.1}";
    out.Add(" disc {<0,0,") << ((i+1)/ConeStipples+0.5) << "><0,0,1>, 0.1}";
  }
  out.Add("}}");

  out.Add("#declare bond_balls_bond=object { union {");
  for( int i=0; i < 12; i++ )
    out.Add(" sphere {<0,0,") << (double)i/12 << ">, 0.02}";
  out.Add("}}");

  out.Add("#declare bond_line=object{ cylinder {<0,0,0>, <0,0,1>, 0.01} }");
  out.Add("#declare bond_stippled_line=object{ cylinder {<0,0,0>, <0,0,1>, 0.01} }");

  out.Add("#declare bond_bottom_double_cone=object { union {");
  out.Add(" cylinder {<0.1,0,0>, <0.1,0,0.5>, 0.05}");
  out.Add(" cylinder {<-0.1,0,0>, <-0.1,0,0.5>, 0.05}");
  out.Add("}}");

  out.Add("#declare bond_top_double_cone=object { union {");
  out.Add(" cylinder {<0.1,0,0.5>, <0.1,0,1>, 0.05}");
  out.Add(" cylinder {<-0.1,0,0.5>, <-0.1,0,1>, 0.05}");
  out.Add("}}");

  out.Add("#declare bond_bottom_triple_cone=object { union {");
  out.Add(" cylinder {<0.1,0,0>, <0.1,0,0.5>, 0.03}");
  out.Add(" cylinder {<0,0,0>, <0,0,0.5>, 0.03}");
  out.Add(" cylinder {<-0.1,0,0>, <-0.1,0,0.5>, 0.03}");
  out.Add("}}");

  out.Add("#declare bond_top_triple_cone=object { union {");
  out.Add(" cylinder {<0.1,0,0.5>, <0.1,0,1>, 0.03}");
  out.Add(" cylinder {<0,0,0.5>, <0,0,1>, 0.03}");
  out.Add(" cylinder {<-0.1,0,0.5>, <-0.1,0,1>, 0.03}");
  out.Add("}}");

  return out;
}
//..............................................................................
const_strlist TXBond::ToWrl(olxdict<TGlMaterial, olxstr,
    TComparableComparator> &materials) const
{
  TStrList out;
  if (olx_abs(Params()[1]) + olx_abs(Params()[2]) < 1e-3 || Params()[3] < 1e-3)
    return out;
  out.Add(" Group{ children[ Transform{");
  wrl::CrdTransformer crdt(Parent.GetBasis());
  vec3d t = crdt.crd(GetBaseCrd());
  mat3d m;
  olx_create_rotation_matrix(m, vec3d(Params()[1], Params()[2], 0).Normalise(),
    cos(Params()[0]*M_PI/180));
  m = crdt.matr(m);
  vec3d r;
  double ang = wrl::decompose(m, r);
  out.Add("  translation ") << wrl::to_str(t);
  out.Add("  rotation ").stream(' ') << r[0] << r[1] << r[2] << ang;
  out.Add("  scale ").stream(' ') << Params()[4] << Params()[4] << Params()[3];
  out.Add("  children[");
  const TGPCollection &gpc = GetPrimitives();
  for (size_t i=0; i < gpc.PrimitiveCount(); i++) {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    olxstr p_mat = wrl::get_mat_str(glp.GetProperties(), materials, this);
    out.Add("   DEF b ") << "bond_" <<
      glp.GetName().ToLowerCase().Replace(' ', '_') << "{appr " << p_mat << '}';
  }
  out.Add(" ]}]}");
  return out;
}
//..............................................................................
const_strlist TXBond::WrlDeclare() {
  TStrList out;
  out.Add("PROTO bond_single_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.5 "
    "children Shape{ appearance IS appr "
    "geometry Cylinder{ height 1 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_top_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.75 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_bottom_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.25 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_top_line[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.75 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.01 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_bottom_line[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.25 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.01 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_top_disk[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 1 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0 radius 0.1 side FALSE}}}}";
  out.Add("PROTO bond_bottom_disk[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0 radius 0.1 side FALSE}}}}";
  out.Add("PROTO bond_middle_disk[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.5 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0 radius 0.1 side FALSE}}}}";

  double ConeStipples = FBondParams->GetNumParam("ConeStipples", 6.0, true);
  double step = 0.5/ConeStipples;
  out.Add("PROTO bond_stipple_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 children[";
  for (double i=0; i < ConeStipples; i++) {
    out.Add(" Transform{ translation 0 ") << 0.05+i/ConeStipples <<
      " 0 children Shape{ appearance IS appr geometry Cylinder{ height " <<
      step << " radius 0.1}}}";
  }
  out.Add("]}}");
  out.Add("PROTO bond_bottom_stipple_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 children[";
  for( double i=0; i < ConeStipples/2; i++ )  {
    out.Add(" Transform{ translation 0 ") << 0.05+i/ConeStipples <<
      " 0 children Shape{ appearance IS appr geometry Cylinder{ height " <<
      step << " radius 0.1}}}";
  }
  out.Add("]}}");
  out.Add("PROTO bond_top_stipple_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 children[";
  for( double i=0; i < ConeStipples/2; i++ )  {
    out.Add(" Transform{ translation 0 ") << 0.55+i/ConeStipples <<
      " 0 children Shape{ appearance IS appr geometry Cylinder{ height " <<
      step << " radius 0.1}}}";
  }
  out.Add("]}}");
  out.Add("PROTO bond_balls_bond[exposedField SFNode appr NULL]{") <<
    " Transform{ children[";
  for( double i=0; i < 12; i++ )  {
    out.Add(" Transform{ translation 0 0 ") << i/12 <<
      " children Shape{ appearance IS appr geometry Sphere{ radius 0.02}}}";
  }
  out.Add("]}}");

  out.Add("PROTO bond_bottom_double_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.25 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_top_double_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.75 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_bottom_triple_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.25 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";
  out.Add("PROTO bond_top_triple_cone[exposedField SFNode appr NULL]{") <<
    " Transform{ rotation 1 0 0 1.5708 translation 0 0 0.75 "
    "children Shape{ appearance IS appr geometry "
    "Cylinder{ height 0.5 radius 0.1 top FALSE bottom FALSE}}}}";

  //out.Add("#declare bond_line=object{ cylinder {<0,0,0>, <0,0,1>, 0.01} }");
  //out.Add("#declare bond_stippled_line=object{ cylinder {<0,0,0>, <0,0,1>, 0.01} }");
  return out;
}
//..............................................................................
void TXBond::CreateStaticObjects(TGlRenderer& Parent)  {
  if( OnStylesClear == NULL )  {
    OnStylesClear = new TStylesClear(Parent);
    new TContextClear(Parent);
  }
  ClearStaticObjects();
  TStringToList<olxstr, TGlPrimitive*> &primitives = GetStaticPrimitives();
  TGlMaterial GlM;
  TGlPrimitive *GlP, *GlPRC1, *GlPRD1, *GlPRD2;
  ValidateBondParams();
  double ConeQ = FBondParams->GetNumParam("ConeQ", 15.0, true);
  double ConeStipples = FBondParams->GetNumParam("ConeStipples", 6.0, true);
//..............................
  // create single color cylinder
  GlP = &Parent.NewPrimitive(sgloCylinder);
  primitives.Add("Single cone", GlP);

  GlP->Params[0] = 0.1;  GlP->Params[1] = 0.1;  GlP->Params[2] = 1;
  GlP->Params[3] = ConeQ;   GlP->Params[4] = 1;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create top disk
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top disk", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloDisk); 
  GlPRC1->Params[0] = 0;  GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = ConeQ;
  GlPRC1->Params[3] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0, 0, 1);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
//..............................
  // create bottom disk
  GlP = &Parent.NewPrimitive(sgloDisk);
  primitives.Add("Bottom disk", GlP);

  GlP->SetQuadricOrientation(GLU_INSIDE);
  GlP->Params[0] = 0;  GlP->Params[1] = 0.1;  GlP->Params[2] = ConeQ;
  GlP->Params[3] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create middle disk
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Middle disk", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloDisk);
  GlPRC1->Params[0] = 0;  GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = ConeQ;
  GlPRC1->Params[3] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create bottom cylinder
  GlP = &Parent.NewPrimitive(sgloCylinder);
  primitives.Add("Bottom cone", GlP);

  GlP->Params[0] = 0.1;  GlP->Params[1] = 0.1;  GlP->Params[2] = 0.5;
  GlP->Params[3] = ConeQ;   GlP->Params[4] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create top cylinder
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top cone", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = 0.5;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
//..............................
  // create bottom line
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Bottom line", GlP);

  GlP->StartList();
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0, 0, 0);
      olx_gl::vertex(0.0f, 0.0f, 0.5f);
    olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create top line
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top line", GlP);

  GlP->StartList();
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0.0f, 0.0f, 0.5f);
      olx_gl::vertex(0, 0, 1);
    olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
//..............................
  // create stipple cone
  float CL = (float)(1.0/(2*ConeStipples));
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Stipple cone", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = CL;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlPRD1 = &Parent.NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = 0;  GlPRD1->Params[1] = 0.1;  GlPRD1->Params[2] = ConeQ;
  GlPRD1->Params[3] = 1;
  GlPRD1->Compile();

  GlPRD2 = &Parent.NewPrimitive(sgloDisk);
  GlPRD2->SetQuadricOrientation(GLU_INSIDE);
  GlPRD2->Params[0] = 0;  GlPRD2->Params[1] = 0.1;  GlPRD2->Params[2] = ConeQ;
  GlPRD2->Params[3] = 1;
  GlPRD2->Compile();

  GlP->StartList();
  for( int i=0; i < ConeStipples; i++ )  {
    if( i != 0 )
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0f, 0.0f, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDef;
  //..............................
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Bottom stipple cone", GlP);

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, CL/2);
  for( int i=0; i < ConeStipples/2; i++ )  {
    if( i != 0 )
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0f, 0.0f, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top stipple cone", GlP);

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, (float)(0.5 + CL/2));
  for( int i=0; i < ConeStipples/2; i++ )  {
    GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0f, 0.0f, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;

//..............................
  // create stipped ball bond
  CL = (float)(1.0/(12.0));
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Balls bond", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloSphere);
  GlPRC1->Params[0] = 0.02;    GlPRC1->Params[1] = 5;  GlPRC1->Params[2] = 5;
  GlPRC1->Compile();

  GlP->StartList();
  for( int i=0; i < 12; i++ )  {
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRC1);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDef;
//..............................
  // create line
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Line", GlP);

  GlP->StartList();
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0, 0, 0);
      olx_gl::vertex(0, 0, 1);
    olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create stippled line
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Stippled line", GlP);

  GlP->StartList();
    olx_gl::enable(GL_LINE_STIPPLE);
    olx_gl::lineStipple(1, 0xf0f0);
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0, 0, 0);
      olx_gl::vertex(0, 0, 1);
    olx_gl::end();
  olx_gl::disable(GL_LINE_STIPPLE);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  // create bottom double cylinder
  GlP = &Parent.NewPrimitive(sgloCylinder);
  primitives.Add("Bottom double cone", GlP);
  GlP->Params[0] = 0.1 / 2;    GlP->Params[1] = 0.1 / 2;  GlP->Params[2] = 0.5;
  GlP->Params[3] = ConeQ;  GlP->Params[4] = 1;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  // create top double cylinder
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top double cone", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1 / 2;    GlPRC1->Params[1] = 0.1 / 2;  GlPRC1->Params[2] = 0.5;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
  //..............................
  // create bottom tripple cylinder
  GlP = &Parent.NewPrimitive(sgloCylinder);
  primitives.Add("Bottom triple cone", GlP);
  GlP->Params[0] = 0.1 / 3;    GlP->Params[1] = 0.1 / 3;  GlP->Params[2] = 0.5;
  GlP->Params[3] = ConeQ;  GlP->Params[4] = 1;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  // create top double cylinder
  GlP = &Parent.NewPrimitive(sgloCommandList);
  primitives.Add("Top triple cone", GlP);

  GlPRC1 = &Parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1 / 3;    GlPRC1->Params[1] = 0.1 / 3;  GlPRC1->Params[2] = 0.5;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count() + 1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
}
//..............................................................................
olxstr TXBond::GetLegend(const TSBond& Bnd, const short level)  {
  olxstr L(EmptyString(), 32);
  const TSAtom *A = &Bnd.A(),
               *B = &Bnd.B();
  if (A->GetType() != B->GetType()) {
    if (A->GetType() < B->GetType())
      olx_swap(A, B);
  }
  else {
    if (A->GetLabel().Compare(B->GetLabel()) < 0)
      olx_swap(A, B);
  }
  L << A->GetType().symbol << '-' << B->GetType().symbol;
  if( Bnd.GetType() == sotHBond )
    L << "@H";
  if (Bnd.GetOrder() > sboSingle) {
    L << "*" << Bnd.GetOrder();
  }
  if( level == 0 )  return L;
  L << '.' << A->GetLabel() << '-' << B->GetLabel();
  if( level == 1 )  return L;
  TUnitCell::SymmSpace sp =
    A->GetNetwork().GetLattice().GetUnitCell().GetSymmSpace();
  L << '.' << TSymmParser::MatrixToSymmCode(sp, A->GetMatrix()) <<
    '-' <<
    TSymmParser::MatrixToSymmCode(sp, B->GetMatrix());
  if( level == 2 )  return L;
  return L << ".u";
}
//..............................................................................
void TXBond::SetRadius(double V)  {
  Params()[4] = V;
  if( this->Primitives != NULL )  {
    GetPrimitives().GetStyle().SetParam("R", V, IsRadiusSaveable());
    // update radius for all members of the collection
    for( size_t i=0; i < GetPrimitives().ObjectCount(); i++ )
      GetPrimitives().GetObject(i).Params()[4] = V;
  }
}
//..............................................................................
uint32_t TXBond::GetPrimitiveMask() const {
  return GetPrimitives().GetStyle().GetNumParam(GetPrimitiveMaskName(),
    (GetType() == sotHBond) ? 2048 : DefMask(), IsMaskSaveable());
}
//..............................................................................
void TXBond::OnPrimitivesCleared() {
  if (!GetStaticPrimitives().IsEmpty())
    ClearStaticObjects();
}
//..............................................................................
void TXBond::ValidateBondParams() {
  if (FBondParams == NULL) {
    FBondParams = &TGlRenderer::_GetStyles().NewStyle("BondParams", true);
    FBondParams->SetPersistent(true);
  }
}
//..............................................................................
void TXBond::DefMask(int V) {
  ValidateBondParams();
  FBondParams->SetParam("DefM", (FDefM=V), true);
}
//..............................................................................
int TXBond::DefMask()  {
  if (FDefM != -1) return FDefM;
  ValidateBondParams();
  return (FDefM = FBondParams->GetNumParam("DefM", 7, true));
}
//..............................................................................
void TXBond::DefR(double V)  {
  ValidateBondParams();
  FBondParams->SetParam("DefR", (FDefR=V), true);
}
//..............................................................................
double TXBond::DefR()  {
  if (FDefR > 0) return FDefR;
  ValidateBondParams();
  return (FDefR = FBondParams->GetNumParam("DefR", 1.0, true));
}
//..............................................................................
bool TXBond::OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseDown(Sender, Data) : false;
}
//..............................................................................
bool TXBond::OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseMove(Sender, Data) : false;
}
//..............................................................................
bool TXBond::OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseMove(Sender, Data) : false;
}
//..............................................................................
