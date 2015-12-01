/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xatom.h"
#include "glprimitive.h"
#include "gpcollection.h"
#include "styles.h"
#include "ellipsoid.h"
#include "symmparser.h"
#include "network.h"
#include "asymmunit.h"
#include "unitcell.h"
#include "lattice.h"
#include "planesort.h"
#include "glgroup.h"
#include "exyzgroup.h"
#include "glutil.h"
#include "povdraw.h"
#include "wrldraw.h"
#include "gltexture.h"
#include "glbackground.h"

//----------------------------------------------------------------------------//
// TSAtom function bodies
//----------------------------------------------------------------------------//
TXAtom::TXAtom(TNetwork* net, TGlRenderer& Render, const olxstr& collectionName) :
  TSAtom(net),
  AGlMouseHandlerImp(Render, collectionName),
  Polyhedron(0), settings(0)
{
  SetGroupable(true);
  FDrawStyle = (GetEllipsoid() == 0 ? adsSphere : adsEllipsoid);
  FRadius = darIsot;
  ActualSphere = ~0;
  Params().Resize(2);
  Params()[0] = 1; // radius
  Params()[1] = 1; // zoom
  // the objects will be automatically deleted by the corresponding action collections
  Label = new TXGlLabel(Render, PLabelsCollectionName());
  Label->SetOffset(crd());
  Label->SetVisible(false);
  label_forced = false;
}
//..............................................................................
TXAtom::~TXAtom() {
  if (GetParentGroup() != NULL) {
    GetParentGroup()->Remove(*this);
#ifdef _DEBUG
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
#endif
  }
  if (Polyhedron != NULL) {
    delete Polyhedron;
    Polyhedron = NULL;
  }
  delete Label;
}
//..............................................................................
void TXAtom::Update()  {
  InitActualSphere();
}
//..............................................................................
void TXAtom::InitActualSphere() {
  Settings &defs = GetSettings();
  ActualSphere = ~0;
  if (defs.GetQuality() == qaPict)
    return;
  if (CAtom().GetExyzGroup() != NULL && defs.ConstrainedAtomSphere != ~0)
    ActualSphere = defs.ConstrainedAtomSphere;
  else if (CAtom().IsFixedType() && defs.LockedAtomSphere != ~0)
    ActualSphere = defs.LockedAtomSphere;
}
//..............................................................................
int TXAtom::Quality(TGlRenderer &r, int V) {
  Settings &defs = GetSettings(r);
  defs.SetRimR(1.02);
  defs.SetDiskS(defs.GetRimW());
  switch (V) {
    case qaHigh:
      defs.SetSphereQ(30);
      defs.SetRimQ(30);
      defs.SetDiskQ(30);
      break;
    case qaMedium:
    default:
      defs.SetSphereQ(15);
      defs.SetRimQ(15);
      defs.SetDiskQ(15);
      break;
    case qaLow:
      defs.SetSphereQ(5);
      defs.SetRimQ(5);
      defs.SetDiskQ(5);
      break;
    case qaPict:
      defs.SetSphereQ(120);
      defs.SetRimQ(120);
      defs.SetDiskQ(120);
      defs.SetRimR(1.0005);
      break;
  }
  defs.SetDiskOR(defs.GetRimR());
  int rv = defs.GetQuality();
  defs.SetQuality(V);
  return rv;
}
//..............................................................................
void TXAtom::ListPrimitives(TStrList &List) const {
  List.Assign(GetSettings().GetPrimitives());
}
//..............................................................................
TStrList* TXAtom::FindPrimitiveParams(TGlPrimitive *P) const {
  const Settings &defs = GetSettings();
  for (size_t i = 0; i < defs.PrimitiveParams.Count(); i++) {
    if (defs.PrimitiveParams[i].GlP == P)
      return &(defs.PrimitiveParams[i].Params);
  }
  return NULL;
}
//..............................................................................
void TXAtom::ListParams(TStrList &l, TGlPrimitive *P) {
  TStrList *L = FindPrimitiveParams(P);
  if (L != NULL)
    l = *L;
}
//..............................................................................
void TXAtom::ListParams(TStrList &l) {
  l <<"Radius" << "Color";
}
//..............................................................................
void TXAtom::CalcRad(short DefRadius) {
  FRadius = DefRadius;
  GetPrimitives().GetStyle().SetParam("DR", DefRadius);

  if (DefRadius == darPers)
    SetR(GetType().r_pers);
  else if (DefRadius == darPack)
    SetR(GetType().r_sfil);
  else if (DefRadius == darVdW)
    SetR(GetType().r_vdw);
  else if (DefRadius == darBond)
    SetR(sqrt(caDefIso) / 2);
  else if (DefRadius == darIsot) {
    if (GetType() == iHydrogenZ)
      SetR(3 * caDefIso);
    else {
      if (GetType() == iQPeakZ)
        SetR(sqrt(CAtom().GetUiso())*GetSettings().GetQPeakSizeScale());
      else if (CAtom().GetUiso() > 0)
        SetR(sqrt(CAtom().GetUiso()));
      else
        SetR(3 * caDefIso); //sqrt(caDefIso);
    }
  }
  else if (DefRadius == darIsotH) {
    if (CAtom().GetUiso() > 0)
      SetR(sqrt(CAtom().GetUiso()));
    else
      SetR(3 * caDefIso); //sqrt(caDefIso);
  }
}
//..............................................................................
void TXAtom::ValidateRadius(TGraphicsStyle& GS) {
  const Settings &defs = GetSettings();
  Params()[1] = GS.GetNumParam("Z", defs.GetZoom());
  short dr = GS.GetNumParam("DR", defs.GetR());
  if (dr == darCustom) {
    double r;
    if ((r=GS.GetNumParam("R", -1.0, LegendLevel(GetCollectionName()) == 0)) == -1)
      CalcRad(darIsot);
    else
      Params()[0] = r;
  }
  else
    CalcRad(dr);

}
void TXAtom::ValidateDS(TGraphicsStyle& GS)  {
  DrawStyle(GS.GetNumParam("DS", GetSettings().GetDS()));
}
//..............................................................................
void TXAtom::Create(const olxstr& cName) {
  SetCreated(true);
  olxstr Legend, strRef = GetRef().ToString();
  if (!cName.IsEmpty()) {
    SetCollectionName(cName);
    NamesRegistry().Add(strRef, Legend = cName, true);
  }
  else {
    Legend = NamesRegistry().Find(strRef, EmptyString());
    if (Legend.IsEmpty()) {
      Legend = GetLegend(*this);
    }
  }

  TGPCollection *GPC = NULL;
  Settings &defs = GetSettings();
  const TStringToList<olxstr, TGlPrimitive*> &primitives =
    defs.GetPrimitives(true);
  InitActualSphere();
  Label->SetFontIndex(Parent.GetScene().FindFontIndexForType<TXAtom>());
  //Label->SetLabel(Atom().GetLabel());
  Label->Create();
  // find collection
  if (GetType() == iQPeakZ) {
    Legend = GetLabelLegend(*this);
    GPC = Parent.FindCollection(Legend);
    // if the collection is empty, need to fill it...
    if (GPC == NULL || GPC->PrimitiveCount() == 0) {
      if (GPC == NULL)
        GPC = &Parent.NewCollection(Legend);
      GPC->GetStyle().SetSaveable(false);
    }
    else {
      GPC->AddObject(*this);
      ValidateRadius(GPC->GetStyle());
      return;
    }
  }
  else {
    olxstr NewL;
    GPC = Parent.FindCollectionX(Legend, NewL);
    if (GPC == NULL)
      GPC = &Parent.NewCollection(NewL);
    else {
      if (GPC->PrimitiveCount() != 0) {
        GPC->AddObject(*this);
        if ((GPC->GetStyle().GetNumParam(GetPrimitiveMaskName(), 0) &
          (1 << defs.PolyhedronIndex)) != 0)
        {
          CreatePolyhedron(true);
        }
        ValidateRadius(GPC->GetStyle());
        ValidateDS(GPC->GetStyle());
        return;
      }
    }
  }

  TGraphicsStyle& GS = GPC->GetStyle();
  GS.SetSaveable(GPC->GetName().CharCount('.') == 0);

  olxstr& SMask = GS.GetParam(GetPrimitiveMaskName(), EmptyString());
  if (SMask.IsEmpty())
    SMask = defs.GetMask();
  int PMask = SMask.ToInt();

  GPC->AddObject(*this);
  if( PMask == 0 )
    return; // nothing to create then...
  // update primitives list
  ValidateDS(GS);
  ValidateRadius(GS);

  TGlMaterial RGlM;
  for (size_t i=0; i < primitives.Count(); i++) {
    const int off = 1 << i;
    if (PMask & off) {
      TGlPrimitive* SGlP = primitives.GetObject(i);
      TGlPrimitive& GlP = GPC->NewPrimitive(primitives.GetString(i),
        sgloCommandList);
      if (i == defs.SphereIndex)
        GlP.SetOwnerId(xatom_SphereId);
      else if (i == defs.SmallSphereIndex)
        GlP.SetOwnerId(xatom_SmallSphereId);
      else if (i == defs.RimsIndex)
        GlP.SetOwnerId(xatom_RimsId);
      else if (i == defs.DisksIndex)
        GlP.SetOwnerId(xatom_DisksId);
      else if (i == defs.CrossIndex)
        GlP.SetOwnerId(xatom_CrossId);
      else if (i == defs.TetrahedronIndex)
        GlP.SetOwnerId(xatom_TetrahedronId);
      else if (i == defs.PolyhedronIndex) {
        GlP.SetOwnerId(xatom_PolyId);
        CreatePolyhedron(true);
      }
      else {
        GlP.SetOwnerId(0);
      }
      /* copy the default drawing style tag*/
      GlP.Params.Resize(GlP.Params.Count()+1);
      GlP.Params.GetLast() = SGlP->Params.GetLast();

      GlP.StartList();
      GlP.CallList(SGlP);
      GlP.EndList();

      if (GetType() == iQPeakZ) {
        GetDefSphereMaterial(*this, RGlM, defs);
        GlP.SetProperties(RGlM);
      }
      else {
        const size_t mi = GS.IndexOfMaterial(primitives.GetString(i));
        if (mi != InvalidIndex)
          GlP.SetProperties(GS.GetPrimitiveStyle(mi).GetProperties());
        else if (SGlP->Params.GetLast() == ddsDefSphere) {
          const size_t lmi = GS.IndexOfMaterial("Sphere");
          if (lmi != InvalidIndex)
            RGlM = GS.GetPrimitiveStyle(lmi).GetProperties();
          else
            GetDefSphereMaterial(*this, RGlM, defs);
        }
        else if (SGlP->Params.GetLast() == ddsDefRim) {
          const size_t lmi = GS.IndexOfMaterial("Rims");
          if (lmi != InvalidIndex)
            RGlM = GS.GetPrimitiveStyle(lmi).GetProperties();
          else
            GetDefRimMaterial(*this, RGlM);
        }
        GlP.SetProperties(GS.GetMaterial(primitives[i], RGlM));
      }
    }
  }
}
//..............................................................................
olxstr TXAtom::GetLabelLegend(const TSAtom& A) {
  olxstr L(A.GetType().symbol, A.CAtom().GetLabel().Length()+4);
  return (L << '.' << A.CAtom().GetLabel());
}
//..............................................................................
olxstr TXAtom::GetLegend(const TSAtom& A, const short Level) {
  if (A.GetType() == iQPeakZ)
    return GetLabelLegend(A);
  olxstr L(A.GetType().symbol, 16);
  if (Level == 0 ) return L;
  L << '.' << A.CAtom().GetLabel();
  if (Level == 1)  return L;
  L << '.';
  L << TSymmParser::MatrixToSymmCode(
    A.GetNetwork().GetLattice().GetUnitCell().GetSymmSpace(), A.GetMatrix());
  return L;
}
//..............................................................................
uint16_t TXAtom::LegendLevel(const olxstr& legend) {
  return (uint16_t)legend.CharCount('.');
}
//..............................................................................
bool TXAtom::Orient(TGlPrimitive& GlP) {
  // override for standalone atoms
  if (FDrawStyle == adsStandalone && !IsStandalone())
    return true;

  if (GlP.GetOwnerId() == xatom_PolyId) {
    if (Polyhedron == NULL) return true;
    olx_gl::begin(GL_TRIANGLES);
    const TXAtom::Poly& pl = *Polyhedron;
    for (size_t j=0; j < pl.faces.Count(); j++) {
      olx_gl::normal(pl.norms[j]);
      olx_gl::vertex(pl.vecs[pl.faces[j][0]]);
      olx_gl::vertex(pl.vecs[pl.faces[j][1]]);
      olx_gl::vertex(pl.vecs[pl.faces[j][2]]);
    }
    olx_gl::end();
    return true;
  }
  // override for iso atoms
  if (GetEllipsoid() == NULL &&
     (GlP.GetOwnerId() == xatom_DisksId || GlP.GetOwnerId() == xatom_RimsId))
  {
    return true;
  }

  vec3d c = GetCenter();
  c += crd();

  const TExyzGroup* exyz = CAtom().GetExyzGroup();
  if (exyz != NULL) {
    if (IsSpecialDrawing()) { // draw separate atoms
      const mat3d& m = Parent.GetBasis().GetMatrix();
      vec3d v(m[0][0], m[1][0], m[2][0]);
      v *= 0.75;
      if (exyz->Count() == 2) {
        if (&(*exyz)[1] == &CAtom())
          c += v;
      }
      if (exyz->Count() == 3) {
        if (&(*exyz)[1] == &CAtom())
          c += v;
        else if (&(*exyz)[2] == &CAtom())
          c -= v;
      }
      else if (exyz->Count() > 3) {
        size_t id=0;
        for (size_t i=0; i < exyz->Count(); i++) {
          if (&(*exyz)[i] == &CAtom()) {
            id = i;
            break;
          }
        }
        if (id > 0) {
          vec3d rv(m[0][2], m[1][2], m[2][2]);
          mat3d rm;
          double a = (id-1)*2*M_PI/(exyz->Count()-1);
          olx_create_rotation_matrix(rm, rv, cos(a), sin(a));
          c += v*rm;
        }
      }
    }
    // for the case when parts are shown - we need to render all atoms
    //else if (&(*exyz)[0] != &CAtom())
    //  return true;
  }
  const Settings &defs = GetSettings();
  olx_gl::translate(c);

  double scale = GetZoom();
  if ((FRadius & darIsot) != 0) {
    if (GetType().z != 1)
      scale *= defs.GetTelpProb();
  }
  else if ((FRadius & darIsotH) != 0)
    scale *= defs.GetTelpProb();

  if (FDrawStyle == adsEllipsoid || FDrawStyle == adsOrtep) {
    if (GetEllipsoid() != NULL) {
      // override for NPD atoms
      if (GetEllipsoid()->IsNPD() ||
        (CAtom().GetUiso() < 1e-2 && IsSpecialDrawing()))
      {
        olx_gl::scale(caDefIso*2*scale);
        if (GlP.GetOwnerId() == xatom_SphereId) {
          defs.GetPrimitives().GetObject(defs.TetrahedronIndex)->Draw();
          return true;
        }
        if (GlP.GetOwnerId() == xatom_SmallSphereId)
          return true;
      }
      else {
        olx_gl::orient(GetEllipsoid()->GetMatrix());
        olx_gl::scale(
          GetEllipsoid()->GetSX()*scale,
          GetEllipsoid()->GetSY()*scale,
          GetEllipsoid()->GetSZ()*scale
          );
        if (GlP.GetOwnerId() == xatom_SphereId && !Parent.IsSelecting()) {
          if (ActualSphere != ~0) {
            olx_gl::callList(ActualSphere);
            return true;
          }
          if (FDrawStyle == adsOrtep) {
            short mask = 0;
            const mat3d mat = GetEllipsoid()->GetMatrix()*
              Parent.GetBasis().GetMatrix();
            for (int i=0; i < 3; i++) {
              if (mat[i][2] < 0)
                mask |= (1<<i);
            }
            olx_gl::callList(defs.OrtepSpheres+mask);
            return true;
          }
        }
      }
    }
    else {
      if (CAtom().GetUiso() < 1e-2 && IsSpecialDrawing()) {
        if (GlP.GetOwnerId() == xatom_SphereId) {
          olx_gl::scale(caDefIso * 2 * scale);
          defs.GetPrimitives().GetObject(defs.TetrahedronIndex)->Draw();
        }
        return true;
      }
      olx_gl::scale(GetR()*scale);
    }
  }
  else if (FDrawStyle == adsSphere)
    olx_gl::scale(GetR()*scale);
  else if (FDrawStyle == adsStandalone) {
    olx_gl::scale(GetR()*scale);
    return false;
  }

  if (GlP.GetOwnerId() == xatom_SphereId && !Parent.IsSelecting()) {
    if (ActualSphere != ~0) {
      olx_gl::callList(ActualSphere);
      return true;
    }
  }
  return false;
}
//..............................................................................
bool TXAtom::GetDimensions(vec3d& Max, vec3d& Min)  {
  const double dZ = GetType().r_sfil;
  Max = crd();
  Min = crd();
  Max += dZ;
  Min -= dZ;
  return true;
}
//..............................................................................
void TXAtom::GetDefSphereMaterial(const TSAtom& Atom, TGlMaterial& M,
  const Settings &defs)
{
  uint32_t Mask = OLX_RGBA(0x5f, 0x5f, 0x5f, 0x00);
  uint32_t Cl = Atom.GetType().def_color;
///////////
  if (Atom.GetType() == iQPeakZ) {
    const double peak = Atom.CAtom().GetQPeak();
    const TAsymmUnit &au = *Atom.CAtom().GetParent();
    M.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|
      sglmTransparent);
    M.DiffuseF = 0x00007f;
    M.SpecularF = 0xffffff;
    M.ShininessF = 36;
    // this is to tackle the shelxs86 output...
    if (olx_abs(au.GetMaxQPeak() - au.GetMinQPeak()) < 0.001) {
      M.AmbientF = 0x007f7f;
      M.DiffuseF[3] = 0.5;
    }
    else {
      if (peak > 0)  {
        M.AmbientF = 0x007f7f;
        M.DiffuseF[3] = (float)(atan(
          defs.GetQPeakScale()*peak/au.GetMaxQPeak())*2/M_PI);
      }
      else {
        M.AmbientF = 0x7f007f;
        if (Atom.CAtom().GetParent()->GetMaxQPeak() < 0) {
          M.DiffuseF[3] = (float)(atan(
            defs.GetQPeakScale()*peak / au.GetMinQPeak()) * 2 / M_PI);
        }
        else {
          M.DiffuseF[3] = (float)(atan(
            -defs.GetQPeakScale()*peak / au.GetMaxQPeak()) * 2 / M_PI);
        }
      }
    }
    M.DiffuseF[3] = olx_max(defs.GetQPeakMinAlpha(), M.DiffuseF[3]);
    return;
  }
//////////
  M.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF);
  M.AmbientF = Cl;
  M.DiffuseF = Mask ^ Cl;
  M.SpecularF = 0xffffffff;
  M.ShininessF = 12;
  M.ShininessB = M.ShininessF;
  M.AmbientB = M.AmbientF;
  M.DiffuseB = M.DiffuseF;
  M.SpecularB = M.SpecularF;
}
//..............................................................................
void TXAtom::GetDefRimMaterial(const TSAtom& Atom, TGlMaterial &M) {
  uint32_t Mask = OLX_RGBA(0x5f, 0x5f, 0x5f, 0x00);
  M.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|sglmEmissionF);
//  |  sglmAmbientB|sglmDiffuseB|sglmSpecularB|sglmShininessB|sglmEmissionB);
  uint32_t Cl = Atom.GetType().def_color;
  M.AmbientF = Cl;
  M.DiffuseF = Mask ^ Cl;
  M.SpecularF = 0xffffffff;
  M.EmissionF =  0x14141414;
  M.ShininessF = 12;
}
//..............................................................................
TGraphicsStyle& TXAtom::Style()  {
  return GetPrimitives().GetStyle();
}
//..............................................................................
void TXAtom::ApplyStyle(TGraphicsStyle& Style)  {
  for( size_t i=0; i < Style.PrimitiveStyleCount(); i++ )  {
    TGlPrimitive* GP = GetPrimitives().FindPrimitiveByName(
      Style.GetPrimitiveStyle(i).GetName() );
    if( GP != NULL )
      GP->SetProperties(Style.GetPrimitiveStyle(i).GetProperties());
  }
}
//..............................................................................
void TXAtom::DrawStyle(short V)  {
  olxstr &DS = GetPrimitives().GetStyle().GetParam("DS", EmptyString());
  DS = V;
  FDrawStyle = V;
}
//..............................................................................
void TXAtom::UpdateStyle(TGraphicsStyle& Style) {
/*  TGraphicsStyle *GS=NULL;
  TGPCollection *GPC=NULL, *OGPC;
  TGDrawObject *GDO;
  olxstr Tmp;
  int i;
  GS = Render()->Styles()->FindStyle(Style);
  if( !GS )  return;  // uniq then
  GPC = Render()->Collection(GS.Label());

  OGPC = Atom->Primitives();
  for( i=0; i < GPC->ObjectCount(); i++ ) // copy the group
  { OGPC->Add(GPC->Object(i));  }
  for( i=0; i < GPC->ObjectCount(); i++ )
  { GPC->DeleteObject(i);  }  // delete the old group
  Render()->RemoveCollection(GPC);*/
}
//..............................................................................
void TXAtom::ListDrawingStyles(TStrList &List)  {
  List.Add("Sphere");
  List.Add("Ellipsoid");
  List.Add("NPD Ellipsoid");
}
void UpdateDrawingStyle(int Index) {
  return;
};
//..............................................................................
void TXAtom::UpdatePrimitiveParams(TGlPrimitive* GlP)  {
  Settings &defs = GetSettings();
  const TStringToList<olxstr, TGlPrimitive*> &primitives =
    defs.GetPrimitives();
  size_t ind = primitives.IndexOfObject(GlP);
  if (ind == InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo, "undefined primitive");
  }
  olxstr Legend("Atoms");
  TGraphicsStyle& GS= Parent.GetStyles().NewStyle(Legend, true);
  if (primitives[ind] == "Sphere")
    GS.SetParam("SphereQ", GlP->Params[1], true);
  else if (primitives[ind] == "Small sphere")
    GS.SetParam("SphereQ", GlP->Params[1], true);
  else if (primitives.GetString(ind) == "Rims") {
    GS.SetParam("RimR", GlP->Params[0], true);
    GS.SetParam("RimW", GlP->Params[1], true);
    GS.SetParam("RimQ", GlP->Params[2], true);
  }
  else if (primitives[ind] == "Disks") {
    GS.SetParam("DiskIR", GlP->Params[0], true);
    GS.SetParam("DiskOR", GlP->Params[1], true);
    GS.SetParam("DiskQ", GlP->Params[2], true);
    GS.SetParam("DiskS", GlP->Params[3], true);
  }
}
//..............................................................................
const_strlist TXAtom::ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const
{
  TStrList out;
  if( DrawStyle() == adsStandalone && !IsStandalone() )
    return out;
  out.Add(" object { union {");
  const TGPCollection &gpc = GetPrimitives();
  for( size_t i=0; i < gpc.PrimitiveCount(); i++ )  {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    if( glp.GetOwnerId() == xatom_PolyId ) continue;
    if( GetEllipsoid() == NULL &&
      (glp.GetOwnerId() == xatom_DisksId || glp.GetOwnerId() == xatom_RimsId ) )
      continue;
    olxstr glp_name = glp.GetName().ToLowerCase().Replace(' ', '_'),
      name_extra;
    if (GetEllipsoid() != NULL) {
      if (GetEllipsoid()->IsNPD() && DrawStyle() != adsSphere) {
        glp_name = "tetrahedron";
      }
      else if (FDrawStyle == adsOrtep &&
          glp.GetOwnerId() == xatom_SphereId)
      {
        short mask = 0;
        const mat3d mat = GetEllipsoid()->GetMatrix()*Parent.GetBasis().GetMatrix();
        for( int i=0; i < 3; i++ )  {
          if( mat[i][2] < 0 )
            mask |= (1<<i);
        }
        name_extra = mask+1;
      }
    }

    olxstr p_mat = pov::get_mat_name(glp.GetProperties(), materials, this);
    out.Add("  object {") << "atom_" << glp_name << name_extra <<
      " texture {" << p_mat << "}}";
  }
  pov::CrdTransformer crdc(Parent.GetBasis());
  out.Add("  }");
  if( (DrawStyle() == adsEllipsoid || DrawStyle() == adsOrtep) &&
    GetEllipsoid() != NULL && !GetEllipsoid()->IsNPD())
  {
    mat3d m = GetEllipsoid()->GetMatrix()*GetDrawScale();
    m[0] *= GetEllipsoid()->GetSX();
    m[1] *= GetEllipsoid()->GetSY();
    m[2] *= GetEllipsoid()->GetSZ();
    out.Add("  transform {");
    out.Add("   matrix") << pov::to_str(crdc.matr(m), crdc.crd(crd()));
    out.Add("   }");
  }
  else {
    out.Add("  scale ") << GetDrawScale();
    out.Add("  translate") << pov::to_str(crdc.crd(crd()));
  }
  out.Add(" }");
  if( GetPolyhedronType() != polyNone && GetPolyhedron() != NULL )  {
    olxstr poly_mat_name = pov::get_mat_name("Polyhedron",
      GetPrimitives().GetStyle(), materials, this);
    TXAtom::Poly &p = *GetPolyhedron();
    out.Add(" union { //") << GetLabel();
    for( size_t i=0; i < p.faces.Count(); i++ )  {
      out.Add("  smooth_triangle {");
      for( int j=0; j < 3; j++ )  {
        out.Add("   ") << pov::to_str(crdc.crd(p.vecs[p.faces[i][j]]))
          << pov::to_str(crdc.normal(p.norms[i]));
      }
      out.Add("   texture {") << poly_mat_name << '}';
      out.Add("  }");
    }
    out.Add(" }");
  }
  return out;
}
//..............................................................................
const_strlist TXAtom::PovDeclare(TGlRenderer &r) {
  TStrList out;
  out.Add("#declare atom_sphere=object{ sphere {<0,0,0>, 1} }");
  out.Add("#declare atom_sphere1=object{ difference {"
    "sphere {<0,0,0>, 1} box {<0,0,0>, <1,1,1>} } }");
  out.Add("#declare atom_sphere2=object{ difference {"
    " sphere {<0,0,0>, 1} box {<-1,0,0>, <0,1,1>} } }");
  out.Add("#declare atom_sphere3=object{ difference {"
    " sphere {<0,0,0>, 1} box {<0,-1,0>, <1,0,1>} } }");
  out.Add("#declare atom_sphere4=object{ difference {"
    " sphere {<0,0,0>, 1} box {<-1,-1,0>, <0,0,1>} } }");
  out.Add("#declare atom_sphere5=object{ difference {"
    " sphere {<0,0,0>, 1} box {<0,0,-1>, <1,1,0>} } }");
  out.Add("#declare atom_sphere6=object{ difference {"
    " sphere {<0,0,0>, 1} box {<-1,0,-1>, <0,1,0>} } }");
  out.Add("#declare atom_sphere7=object{ difference {"
    " sphere {<0,0,0>, 1} box {<0,-1,-1>, <1,0,0>} } }");
  out.Add("#declare atom_sphere8=object{ difference {"
    " sphere {<0,0,0>, 1} box {<-1,-1,-1>, <0,0,0>} } }");
  out.Add("#declare atom_small_sphere=object{ sphere {<0,0,0>, 0.5} }");
  Settings &defs = GetSettings(r);
  const double RimR = defs.GetRimR();
  const double RimW = defs.GetRimW();
  out.Add("#declare atom_rims=object{ union {");
  out.Add(" cylinder {<") << RimW << ",0,0>, <-" << RimW << ",0,0>, " << RimR << " open}";
  out.Add(" cylinder {<0,") << RimW << ",0>, <0,-" << RimW << ",0>, " << RimR << " open}";
  out.Add(" cylinder {<0,0,") << RimW << ">, <0,0,-" << RimW << ">, " << RimR << " open}";
  out.Add("}}");

  double DiskIR = defs.GetDiskIR();
  double DiskOR = defs.GetDiskOR();
  double DiskS = defs.GetDiskS();
  out.Add("#declare atom_disks=object{ union {");
  out.Add(" disc {<") << DiskS << ",0,0>, <-1,0,0>, " << DiskOR << ',' << DiskIR << '}';
  out.Add(" disc {<-") << DiskS << ",0,0>, <1,0,0>, " << DiskOR << ',' << DiskIR << '}';
  out.Add(" disc {<0,") << DiskS << ",0>, <0,-1,0>, " << DiskOR << ',' << DiskIR << '}';
  out.Add(" disc {<0,-") << DiskS << ",0>, <0,1,0>, " << DiskOR << ',' << DiskIR << '}';
  out.Add(" disc {<0,0,") << DiskS << ">, <0,0,-1>, " << DiskOR << ',' << DiskIR << '}';
  out.Add(" disc {<0,0,-") << DiskS << ">, <0,0,1>, " << DiskOR << ',' << DiskIR << '}';
  out.Add("}}");
  out.Add("#declare atom_cross=object{ union {");
  out.Add(" cylinder {<-1,0,0>, <1,0,0>, 0.05}");
  out.Add(" cylinder {<0,-1,0>, <0,1,0>, 0.05}");
  out.Add(" cylinder {<0,0,-1>, <0,0,1>, 0.05}");
  out.Add("}}");
  if (defs.TetrahedronIndex == InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo, "uninitialised object");
  }
  TGlPrimitive *glp = defs.GetPrimitives().GetObject(defs.TetrahedronIndex);
  out.Add("#declare atom_tetrahedron=object{ union {");
  for (size_t i=0; i < glp->Vertices.Count(); i+=3) {
    out.Add("  smooth_triangle {");
    for( int j=0; j < 3; j++ )  {
      out.Add("   ") << pov::to_str(glp->Vertices[i+j])
        << pov::to_str(glp->Normals[i/3]);
    }
    out.Add("  }");
  }
  out.Add("}}");
  return out;
}
//..............................................................................
const_strlist TXAtom::ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const
{
  TStrList out;
  if (DrawStyle() == adsStandalone && !IsStandalone())
    return out;
  out.Add(" Group { children [");
  pov::CrdTransformer crdc(Parent.GetBasis());
  if( (DrawStyle() == adsEllipsoid || DrawStyle() == adsOrtep) &&
    GetEllipsoid() != NULL && !GetEllipsoid()->IsNPD())
  {
    mat3d m = GetEllipsoid()->GetMatrix()*GetDrawScale(),
      q;
    m[0] *= GetEllipsoid()->GetSX();
    m[1] *= GetEllipsoid()->GetSY();
    m[2] *= GetEllipsoid()->GetSZ();
    m = crdc.matr(m);
    vec3d scale(m[0].Length(), m[1].Length(), m[2].Length());
    m[0] /= scale[0];
    m[1] /= scale[1];
    m[2] /= scale[2];
    vec3d rv;
    double ang = wrl::decompose(m, rv);
    out.Add("  Transform {");
    double ds = GetDrawScale();
    out.Add("   scale").stream(' ') << scale[0] << scale[1] << scale[2];
    out.Add("   rotation").stream(' ') << rv[0] << rv[1] << rv[2] << ang;
  }
  else {
    out.Add("  Transform {");
    double ds = GetDrawScale();
    out.Add("   scale").stream(' ') << ds << ds << ds;
  }
  out.Add("   translation ") << wrl::to_str(crdc.crd(crd()));
  out.Add("   children [");
  const TGPCollection &gpc = GetPrimitives();
  bool th_drawn = false;
  for (size_t i=0; i < gpc.PrimitiveCount(); i++) {
    TGlPrimitive &glp = gpc.GetPrimitive(i);
    if (glp.GetOwnerId() == xatom_PolyId || th_drawn) continue;
    if (GetEllipsoid() == NULL &&
        (glp.GetOwnerId() == xatom_DisksId || glp.GetOwnerId() == xatom_RimsId ))
      continue;
    olxstr glp_name = glp.GetName().ToLowerCase().Replace(' ', '_'),
      name_extra;
    if (GetEllipsoid() != NULL) {
      if (GetEllipsoid()->IsNPD() && DrawStyle() != adsSphere) {
        glp_name = "tetrahedron";
        th_drawn = true;
      }
    }
    olxstr p_mat = wrl::get_mat_str(glp.GetProperties(), materials, this);
    out.Add("   DEF a ") << "atom_" << glp_name << name_extra << "{appr " <<
      p_mat << '}';
  }
  out.Add("  ]}");
  if (GetPolyhedronType() != polyNone && GetPolyhedron() != NULL) {
    olxstr poly_mat_str = wrl::get_mat_str("Polyhedron",
      GetPrimitives().GetStyle(), materials, this);
    TXAtom::Poly &p = *GetPolyhedron();
    out.Add("  Shape{ appearance ") << poly_mat_str;
    out.Add("   geometry IndexedFaceSet{ coord Coordinate{ point[");
    for (size_t i=0; i < p.vecs.Count(); i++) {
      out.Add("    ") << wrl::to_str(crdc.crd(p.vecs[i]));
      if (i+1 < p.vecs.Count())
        out.GetLastString() << ',';
    }
    out.GetLastString() << "]}";
    olxstr &ci = out.Add("    coordIndex[");
    for (size_t i=0; i < p.faces.Count(); i++) {
      ci.stream(' ') << p.faces[i][0] << p.faces[i][1] << p.faces[i][2] << "-1";
    }
    ci << ']';
    out.Add(" }}");
  }
  out.Add(" ]}");  // group
  return out;
}
//..............................................................................
const_strlist TXAtom::WrlDeclare(TGlRenderer &r) {
  TStrList out;
  out.Add("PROTO atom_sphere[exposedField SFNode appr NULL]{") <<
    " Transform{ children Shape{ appearance IS appr "
    "geometry Sphere{ radius 1}}}}";
  out.Add("PROTO atom_small_sphere[exposedField SFNode appr NULL]{") <<
    " Transform{ children Shape{ appearance IS "
    "appr geometry Sphere{ radius 0.5}}}}";
  Settings &defs = GetSettings(r);
  const double RimR = defs.GetRimR();
  const double RimW = defs.GetRimW();
  out.Add("PROTO atom_rims[exposedField SFNode appr NULL]{ Group{ children[ ");
  out.Add(" DEF rim Shape { appearance IS appr geometry Cylinder{ height ")
    << RimW << " radius " << RimR << " top FALSE bottom FALSE}}";
  out.Add(" Transform{ rotation 0 0 1 1.5708 children USE rim}");
  out.Add(" Transform{ rotation 1 0 0 1.5708 children USE rim}");
  out.Add("]}}");

  double DiskIR = defs.GetDiskIR();
  double DiskOR = defs.GetDiskOR();
  double DiskS = defs.GetDiskS();
  out.Add("PROTO atom_disks[exposedField SFNode appr NULL]{ Group{ children[ ");
  out.Add(" DEF disk Shape { appearance IS appr geometry Cylinder{ height ")
    << DiskS << " radius " << DiskOR << "}}";
  out.Add(" Transform{ rotation 0 0 1 1.5708 children USE disk}");
  out.Add(" Transform{ rotation 1 0 0 1.5708 children USE disk}");
  out.Add("]}}");

  out.Add("PROTO atom_cross[exposedField SFNode appr NULL]{ Group{ children[ ");
  out.Add(" DEF stick Shape { appearance IS appr geometry Cylinder{ "
    "height 1 radius 0.05}}");
  out.Add(" Transform{ rotation 0 0 1 1.5708 children USE stick}");
  out.Add(" Transform{ rotation 1 0 0 1.5708 children USE stick}");
  out.Add("]}}");

  if (defs.TetrahedronIndex == InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo, "uninitialised object");
  }
  TGlPrimitive *glp = defs.GetPrimitives().GetObject(defs.TetrahedronIndex);
  out.Add("PROTO atom_tetrahedron[exposedField SFNode appr NULL]{")
  << " Shape{ appearance IS appr geometry IndexedFaceSet{";
  olxstr &p = out.Add("  coord Coordinate{ point[");
  for (size_t i=0; i < glp->Vertices.Count(); i+=3) {
    p <<  wrl::to_str(glp->Vertices[i]);
    if (i+3 < glp->Vertices.Count())
      p << ", ";
  }
  p << "]}";
  // counterclockwise faces
  out.Add("  coordIndex[0 1 2 -1 0 3 1 -1 0 2 3 -1 1 3 2 -1]");
  out.Add("}}}");
  return out;
}
//..............................................................................
void TXAtom::UpdatePrimitives(int32_t Mask)  {
  Settings &defs = GetSettings();
  AGDrawObject::UpdatePrimitives(Mask);
  bool create_polyhedron = (Mask & (1 << defs.PolyhedronIndex)) != 0;
  for (size_t i=0; i < GetPrimitives().ObjectCount(); i++) {
    if (EsdlInstanceOf(GetPrimitives().GetObject(i), TXAtom)) {
      ((TXAtom&)GetPrimitives().GetObject(i))
        .CreatePolyhedron(create_polyhedron);
    }
  }
}
//..............................................................................
void TXAtom::SetZoom(double V)  {
  GetPrimitives().GetStyle().SetParam("Z",
    V, LegendLevel(GetCollectionName()) == 0);
  Params()[1] = V;
}
//..............................................................................
void TXAtom::SetR(double V)  {
  GetPrimitives().GetStyle().SetParam("R", V,
    LegendLevel(GetCollectionName()) == 0);
  Params()[0] = V;
}
//..............................................................................
uint32_t TXAtom::GetPrimitiveMask() const {
  return GetPrimitives().GetStyle().GetNumParam(GetPrimitiveMaskName(), 0);
}
//..............................................................................
bool TXAtom::OnMouseDown(const IOlxObject *Sender, const TMouseData& Data) {
  if (!IsMoveable()) {
    return Label->IsVisible() ? Label->OnMouseDown(Sender, Data) : false;
  }
  return AGlMouseHandlerImp::OnMouseDown(Sender, Data);
}
//..............................................................................
bool TXAtom::OnMouseUp(const IOlxObject *Sender, const TMouseData& Data)  {
  if (!IsMoveable()) {
    return Label->IsVisible() ? Label->OnMouseUp(Sender, Data) : false;
  }
  return AGlMouseHandlerImp::OnMouseUp(Sender, Data);
}
//..............................................................................
bool TXAtom::OnMouseMove(const IOlxObject *Sender, const TMouseData& Data)  {
  if (!IsMoveable()) {
    return Label->IsVisible() ? Label->OnMouseMove(Sender, Data) : false;
  }
  return AGlMouseHandlerImp::OnMouseMove(Sender, Data);
}
//..............................................................................
void TXAtom::CreateNormals(TXAtom::Poly& pl, const vec3f& cnt) {
  const size_t off = pl.norms.Count();
  pl.norms.SetCapacity(pl.faces.Count());
  for (size_t i=off; i < pl.faces.Count(); i++) {
    const vec3f& v1 = pl.vecs[pl.faces[i][0]];
    const vec3f& v2 = pl.vecs[pl.faces[i][1]];
    const vec3f& v3 = pl.vecs[pl.faces[i][2]];
    vec3f n = (v2-v1).XProdVec(v3-v1);
    if (n.QLength() < 1e-6) {  // oups...
      pl.faces.Clear();
      pl.vecs.Clear();
      pl.norms.Clear();
      return;
    }
    const float d = n.DotProd((v1+v2+v3)/3)/n.Length();
    n.Normalise();
    if ((n.DotProd(cnt) - d) > 0) { // normal looks inside?
      olx_swap(pl.faces[i][0], pl.faces[i][1]);
      n *= -1;
    }
    pl.norms.AddCopy(n);
  }
}
vec3f TXAtom::TriangulateType2(Poly& pl, const TSAtomPList& atoms) {
  TSPlane plane(NULL);
  TTypeList< olx_pair_t<TSAtom*, double> > pa;
  vec3f cnt;
  double wght = 0;
  for (size_t i=0; i < atoms.Count(); i++ ) {
    pa.AddNew( atoms[i], atoms[i]->CAtom().GetOccu());
    cnt += atoms[i]->crd()*atoms[i]->CAtom().GetOccu();
    wght += atoms[i]->CAtom().GetOccu();
  }
  cnt += crd();
  wght += CAtom().GetOccu();
  cnt /= (float)wght;
  plane.Init(pa);
  plane.GetCenter();
  // this might fail if one of the atoms is at the center
  PlaneSort::Sorter sp;
  try  { sp.DoSort(plane); }
  catch (...) {
    return cnt;
  }
  const size_t start = pl.vecs.Count();
  pl.vecs.SetCount(pl.vecs.Count()+atoms.Count()+2);
  pl.vecs[start] = crd();
  pl.vecs[start+1] = plane.GetCenter();
  for (size_t i=0; i < sp.sortedPlane.Count(); i++)
    pl.vecs[start+i+2] = sp.sortedPlane[i];
  // create the base
  for (size_t i=0; i < sp.sortedPlane.Count()-1; i++)
    pl.faces.AddNew(start+1, start+i+2, start+i+3);
  pl.faces.AddNew(start+1, start+2, start+sp.sortedPlane.Count()+1);
  // and the rest to close the polyhedron
  for (size_t i=0; i < sp.sortedPlane.Count()-1; i++)
    pl.faces.AddNew(start, start+i+2, start+i+3);
  pl.faces.AddNew(start, start+2, start+sp.sortedPlane.Count()+1);
  return cnt;
}
//..............................................................................
void TXAtom::CreatePoly(const TSAtomPList& bound, short type,
  const vec3d* _normal, const vec3d* _pc)
{
  try  {
    static const vec3d NullVec;
    TXAtom::Poly& pl = *((Polyhedron == NULL) ?
      (Polyhedron=new TXAtom::Poly) : Polyhedron);
    if( type == polyRegular )  {
      vec3d pc;
      for (size_t ai=0; ai < bound.Count(); ai++)
        pc += bound[ai]->crd();
      pc /= bound.Count();
      bool centered = pc.DistanceTo(crd()) < 0.15;
      pl.vecs.SetCount(bound.Count());
      for( size_t i=0; i < bound.Count(); i++ )  {
        pl.vecs[i] = bound[i]->crd();
        for( size_t j=i+1; j < bound.Count(); j++ )  {
          for( size_t k=j+1; k < bound.Count(); k++ )  {
            // regular octahedron vol would be ~0.47A^3
            double vol = olx_tetrahedron_volume(
              NullVec,
              (bound[i]->crd()-crd()).Normalise(),
              (bound[j]->crd()-crd()).Normalise(),
              (bound[k]->crd()-crd()).Normalise());
            if (vol > 0.03) {
              pl.faces.AddNew(i, j, k);
            }
            // check if there is another atom on the other side
            else if (!centered) {
              vec3d n = (bound[i]->crd()-bound[j]->crd()).XProdVec(
                bound[k]->crd()-bound[j]->crd()).Normalise();
              if ((pc-crd()).DotProd(n) > 0)
                n *= -1;
              bool exists=false;
              for (size_t ai=0; ai < bound.Count(); ai++) {
                double d = (bound[ai]->crd()-crd()).DotProd(n);
                if (d > 0.1) { // at least 0.1 A away from this face
                  exists = true;
                  break;
                }
              }
              if (!exists) {
                pl.faces.AddNew(i, j, k);
              }
            }
          }
        }
      }
      CreateNormals(pl, crd());
    }
    else if( type == polyPyramid )  {
      vec3f cnt = TriangulateType2(pl, bound);
      CreateNormals(pl, cnt);
    }
    else if( type == polyBipyramid )  {
      vec3d normal, pc;
      if( _normal == NULL || _pc == NULL )  {
        vec3d rms;
        mat3d normals;
        TSPlane::CalcPlanes(bound, normals, rms, pc);
        normal = normals[2];
      }
      else  {
        normal = *_normal;
        pc = *_pc;
      }
      TSAtomPList sidea, sideb;
      pl.vecs.Clear();
      for( size_t i=0; i < bound.Count(); i++ )  {
        const double ca = normal.CAngle(bound[i]->crd() - pc);
        if( ca >= 0 )
          sidea.Add(bound[i]);
        else
          sideb.Add(bound[i]);
      }
      if( sidea.Count() > 2 )  {
        vec3f cnt = TriangulateType2(pl, sidea);
        CreateNormals(pl, cnt);
      }
      if( sideb.Count() > 2 )  {
        vec3f cnt = TriangulateType2(pl, sideb);
        CreateNormals(pl, cnt);
      }
    }
  }
  catch(...)  {
    if( Polyhedron != NULL )
      delete Polyhedron;
    Polyhedron = NULL;
  }
}
//..............................................................................
void TXAtom::CreatePolyhedron(bool v)  {
  if( Polyhedron != NULL )  {
    delete Polyhedron;
    Polyhedron = NULL;
  }
  if( !v )  return;
  if( NodeCount() < 4 )  return;
  if( IsDeleted() || GetType() == iQPeakZ )
    return;
  TPtrList<TSAtom> bound;
  for( size_t i=0; i < NodeCount(); i++ )  {
    if( Node(i).IsDeleted() || Node(i).GetType().GetMr() < 3.5  )
      continue;
    //if( FAtom.Node(i).NodeCount() > FAtom.NodeCount() )
    //  continue;
    bound.Add(Node(i));
  }
  if( bound.Count() < 4 )  return;
  int type = GetPrimitives().GetStyle().GetNumParam(PolyTypeName(), 0, true);
  if( type != polyAuto && type != polyNone )  {
    CreatePoly(bound, type);
    return;
  }

  Polyhedron = new TXAtom::Poly;
  vec3f sv;
  double sd = 0;
  for( size_t i=0; i < bound.Count(); i++ )  {
    vec3d nd = bound[i]->crd() - crd();
    sd += nd.Length();
    sv += nd.Normalise();
  }
  sv /= bound.Count();
  sd /= bound.Count();
  double ratio = sv.Length()/sd;
  if( ratio < 0.2 )  {  // atom is inside
    // test for Cp-kind polyhedron, should be drawn as two of the other kind (atom outside)
    vec3d pc, rms;
    mat3d normals;
    TSPlane::CalcPlanes(bound, normals, rms, pc);
    const double pd = normals[0].DotProd(pc)/normals[0].Length();
    const double pd_x = normals[2].DotProd(pc)/normals[2].Length();
    normals.Normalise();
    int deviating = 0, deviating_x = 0;
    for( size_t i=0; i < bound.Count(); i++ )  {
      if( olx_abs(bound[i]->crd().DotProd(normals[0]) - pd) > rms[0] )
        deviating++;
      if( olx_abs(bound[i]->crd().DotProd(normals[2]) - pd_x) > rms[2] )
        deviating_x++;
    }
    if( deviating < 3 || deviating_x < 3 )  {  // a proper polyhedra
      CreatePoly(bound, polyRegular);
    }
    else  // two polyhedra of atom outside..
      CreatePoly(bound, polyBipyramid, &normals[2], &pc);
  }
  else  // atom outside
    CreatePoly(bound, polyPyramid);
  GetPrimitives().GetStyle().SetParam(PolyTypeName(), polyAuto, true);
}
//..............................................................................
void TXAtom::SetPolyhedronType(short type) {
  const Settings &defs = GetSettings();
  olxstr& str_type = GetPrimitives().GetStyle()
    .GetParam(PolyTypeName(), "1", true);
  int int_type = str_type.ToInt();
  int int_mask = GetPrimitives().GetStyle()
    .GetNumParam(GetPrimitiveMaskName(), 0);
  if (type == polyNone) {
    if ((int_mask & (1 << defs.PolyhedronIndex)) != 0)
      UpdatePrimitives(int_mask & ~(1 << defs.PolyhedronIndex));
  }
  else {
    if (int_type != type || (int_mask & (1 << defs.PolyhedronIndex)) == 0) {
      str_type = type;
      if ((int_mask & (1 << defs.PolyhedronIndex)) == 0)
        UpdatePrimitives(int_mask | (1 << defs.PolyhedronIndex));
      else {
        GetPrimitives().ClearPrimitives();
        GetPrimitives().RemoveObject(*this);
        Create();
        for (size_t i=0; i < GetPrimitives().ObjectCount(); i++) {
          if (EsdlInstanceOf(GetPrimitives().GetObject(i), TXAtom))
            ((TXAtom&)GetPrimitives().GetObject(i)).CreatePolyhedron(true);
        }
      }
    }
  }
}
//..............................................................................
int TXAtom::GetPolyhedronType() const {
  const Settings &defs = GetSettings();
  int int_mask = GetPrimitives().GetStyle()
    .GetNumParam(GetPrimitiveMaskName(), 0);
  return (int_mask & (1 << defs.PolyhedronIndex)) == 0 ? polyNone :
    GetPrimitives().GetStyle().GetParam(PolyTypeName(), "0", true).ToInt();
}
//..............................................................................
//..............................................................................
//..............................................................................
void TXAtom::Settings::CreatePrimitives() {
  ClearPrimitives();
  TGlMaterial GlM;
  TGlPrimitiveParams *PParams;
  TGlPrimitive *GlP, *GlPRC1, *GlPRD1, *GlPRD2;
  int SphereQ = GetSphereQ();
  double RimR = GetRimR();
  double RimW = GetRimW();
  int RimQ = GetRimsQ();

  double DiskIR = GetDiskIR();
  double DiskOR = GetDiskOR();
  int DiskQ = GetDiskQ();
  double DiskS = GetDiskS();

  //..............................
  // create sphere
  primitives.Add("Sphere", GlP = &parent.NewPrimitive(sgloSphere));
  GlP->Params[0] = 1;  GlP->Params[1] = SphereQ; GlP->Params[2] = SphereQ;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count() + 1);
  GlP->Params.GetLast() = ddsDefSphere;
  //..............................
  // create a small sphere
  primitives.Add("Small sphere", GlP = &parent.NewPrimitive(sgloSphere));
  GlP->Params[0] = 0.5;  GlP->Params[1] = SphereQ; GlP->Params[2] = SphereQ;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count() + 1);
  GlP->Params.GetLast() = ddsDefSphere;
  //..............................
  // create simple rims
  GlPRC1 = &parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = RimR;  GlPRC1->Params[1] = RimR;  GlPRC1->Params[2] = RimW;
  GlPRC1->Params[3] = RimQ; GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  primitives.Add("Rims", GlP = &parent.NewPrimitive(sgloCommandList));
  GlP->StartList();
  olx_gl::translate(0.0, 0.0, -RimW / 2);
  GlP->CallList(GlPRC1);
  olx_gl::translate(0.0, 0.0, +RimW / 2);
  olx_gl::rotate(90, 1, 0, 0);
  olx_gl::translate(0.0, 0.0, -RimW / 2);
  GlP->CallList(GlPRC1);
  olx_gl::translate(0.0, 0.0, +RimW / 2);
  olx_gl::rotate(90, 0, 1, 0);
  olx_gl::translate(0.0, 0.0, -RimW / 2);
  GlP->CallList(GlPRC1);
  olx_gl::translate(0.0, 0.0, +RimW / 2);
  GlP->EndList();
  GlP->Params.Resize(3 + 1);  // radius, height, quality
  GlP->Params[0] = RimR;
  GlP->Params[1] = RimW;
  GlP->Params[2] = RimQ;
  GlP->Params[3] = ddsDefRim;
  PParams = new TGlPrimitiveParams;
  PParams->Params << "Radius" << "Width" << "Slices";
  PrimitiveParams.Add(PParams);
  //..............................
  // create disks
  GlPRD1 = &parent.NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = DiskIR;  GlPRD1->Params[1] = DiskOR;
  GlPRD1->Params[2] = DiskQ;   GlPRD1->Params[3] = 1;
  GlPRD1->Compile();

  GlPRD2 = &parent.NewPrimitive(sgloDisk);
  GlPRD2->SetQuadricOrientation(GLU_INSIDE);
  GlPRD2->Params[0] = DiskIR;  GlPRD1->Params[1] = DiskOR;
  GlPRD2->Params[2] = DiskQ;   GlPRD1->Params[3] = 1;
  GlPRD2->Compile();

  primitives.Add("Disks", GlP = &parent.NewPrimitive(sgloCommandList));
  GlP->StartList();
  olx_gl::translate(0.0, 0.0, -DiskS / 2);
  GlP->CallList(GlPRD2);
  olx_gl::translate(0.0, 0.0, DiskS);
  GlP->CallList(GlPRD1);
  olx_gl::translate(0.0, 0.0, -DiskS / 2);

  olx_gl::rotate(90, 1, 0, 0);
  olx_gl::translate(0.0, 0.0, -DiskS / 2);
  GlP->CallList(GlPRD2);
  olx_gl::translate(0.0, 0.0, DiskS);
  GlP->CallList(GlPRD1);
  olx_gl::translate(0.0, 0.0, -DiskS / 2);

  olx_gl::rotate(90, 0, 1, 0);
  olx_gl::translate(0.0, 0.0, -DiskS / 2);
  GlP->CallList(GlPRD2);
  olx_gl::translate(0.0, 0.0, DiskS);
  GlP->CallList(GlPRD1);
  GlP->EndList();
  GlP->Params.Resize(4 + 1);  // inner radius, outer radius, Quality, offset
  GlP->Params[0] = DiskIR;
  GlP->Params[1] = DiskOR;
  GlP->Params[2] = DiskQ;
  GlP->Params[3] = DiskS;
  GlP->Params[4] = ddsDefRim;
  PParams = new TGlPrimitiveParams;
  PParams->Params << "Inner radius" << "Outer radius" << "Slices" <<
    "Separation";
  PrimitiveParams.Add(PParams);
  //..............................
  // create cross
  primitives.Add("Cross", GlP = &parent.NewPrimitive(sgloLines));
  GlP->Vertices.SetCount(6);
  GlP->Vertices[0][0] = -1;
  GlP->Vertices[1][0] = 1;
  GlP->Vertices[2][1] = -1;
  GlP->Vertices[3][1] = 1;
  GlP->Vertices[4][2] = -1;
  GlP->Vertices[5][2] = 1;
  GlP->Params[0] = 1.0;
  GlP->Params.Resize(GlP->Params.Count() + 1);
  GlP->Params.GetLast() = ddsDefSphere;
  //..............................
  // polyhedron - dummy
  primitives.Add("Polyhedron", GlP = &parent.NewPrimitive(sgloMacro));
  GlP->Params.Resize(GlP->Params.Count() + 1);
  GlP->Params.GetLast() = ddsDefSphere;
  // create tetrahedron
  primitives.Add("Tetrahedron", GlP = &parent.NewPrimitive(sgloTriangles));
  GlP->MakeTetrahedron(1.5);
  GlP->Params[0] = 1.0;
  GlP->Params.Resize(GlP->Params.Count() + 1);
  GlP->Params.GetLast() = ddsDefSphere;
  //..............................
  // init indexes after all are added
  PolyhedronIndex = primitives.IndexOf("Polyhedron");
  SphereIndex = primitives.IndexOf("Sphere");
  SmallSphereIndex = primitives.IndexOf("Small sphere");
  RimsIndex = primitives.IndexOf("Rims");
  DisksIndex = primitives.IndexOf("Disks");
  CrossIndex = primitives.IndexOf("Cross");
  TetrahedronIndex = primitives.IndexOf("Tetrahedron");
  //..............................
  TTypeList<vec3f> vecs;
  TTypeList<IndexTriangle> triags;
  TArrayList<vec3f> norms;
  typedef GlSphereEx<float, OctahedronFP<vec3f> > gls;
  gls::Generate(1, olx_round(log((float)SphereQ) + 0.5f), vecs, triags, norms);
  OrtepSpheres = olx_gl::genLists(9);

  olx_gl::newList(OrtepSpheres, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(0, 0, 0), vec3f(1, 1, 1));
  olx_gl::endList();
  olx_gl::newList(OrtepSpheres + 1, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(-1, 0, 0), vec3f(0, 1, 1));
  olx_gl::endList();
  olx_gl::newList(OrtepSpheres + 2, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(0, -1, 0), vec3f(1, 0, 1));
  olx_gl::endList();
  olx_gl::newList(OrtepSpheres + 3, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(-1, -1, 0), vec3f(0, 0, 1));
  olx_gl::endList();
  olx_gl::newList(OrtepSpheres + 4, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(0, 0, -1), vec3f(1, 1, 0));
  olx_gl::endList();
  olx_gl::newList(OrtepSpheres + 5, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(-1, 0, -1), vec3f(0, 1, 0));
  olx_gl::endList();
  olx_gl::newList(OrtepSpheres + 6, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(0, -1, -1), vec3f(1, 0, 0));
  olx_gl::endList();
  olx_gl::newList(OrtepSpheres + 7, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(-1, -1, -1), vec3f(0, 0, 0));
  olx_gl::endList();
  olx_gl::newList(OrtepSpheres + 8, GL_COMPILE);
  gls::Render(vecs, triags, norms);
  olx_gl::endList();

  // create textured spheres
  TGlTexture
    *glt_l = parent.GetTextureManager().FindByName("LockedAtoms"),
    *glt_c = parent.GetTextureManager().FindByName("ConstrainedAtoms");
  if (glt_l != NULL && glt_l->IsEnabled()) {
    TGlTexture *current = new TGlTexture;
    glt_l->ReadCurrent(*current);
    GLUquadric *s = gluNewQuadric();
    gluQuadricTexture(s, true);
    gluQuadricNormals(s, GLU_OUTSIDE);
    LockedAtomSphere = olx_gl::genLists(1);
    olx_gl::newList(LockedAtomSphere, GL_COMPILE);
    glt_l->SetCurrent();
    gluSphere(s, 1, SphereQ, SphereQ);
    current->SetCurrent();
    olx_gl::endList();
    gluDeleteQuadric(s);
    delete current;
  }
  else {
    LockedAtomSphere = ~0;
  }
  if (glt_c != NULL && glt_c->IsEnabled()) {
    TGlTexture *current = new TGlTexture;
    glt_c->ReadCurrent(*current);
    GLUquadric *s = gluNewQuadric();
    gluQuadricTexture(s, true);
    gluQuadricNormals(s, GLU_OUTSIDE);
    ConstrainedAtomSphere = olx_gl::genLists(1);
    olx_gl::newList(ConstrainedAtomSphere, GL_COMPILE);
    glt_c->SetCurrent();
    gluSphere(s, 1, SphereQ, SphereQ);
    current->SetCurrent();
    olx_gl::endList();
    gluDeleteQuadric(s);
    delete current;
  }
  else {
    ConstrainedAtomSphere = ~0;
  }
}
//..............................................................................
void TXAtom::Settings::ClearPrimitives() {
  primitives.Clear();
  PrimitiveParams.Clear();
  if (OrtepSpheres != ~0) {
    olx_gl::deleteLists(OrtepSpheres, 9);
    OrtepSpheres = ~0;
  }
  if (ConstrainedAtomSphere != ~0) {
    olx_gl::deleteLists(ConstrainedAtomSphere, 1);
    ConstrainedAtomSphere = ~0;
  }
  if (LockedAtomSphere != ~0) {
    olx_gl::deleteLists(LockedAtomSphere, 1);
    LockedAtomSphere = ~0;
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
TXAtomLabelAligner::TXAtomLabelAligner(const TPtrList<TXAtom> & atoms,
  double offset, size_t positions)
  : Atoms(atoms),
  Offset(offset),
  Positions(positions)
{

}
void TXAtomLabelAligner::Align() {
  if (Atoms.IsEmpty()) return;
  for (size_t i = 0; i < Atoms.Count(); i++) {
    Atoms[i]->GetGlLabel().SetVisible(false);
  }
  TGlRenderer &r = Atoms[0]->GetParent();
  bool bg_vis = r.Background()->IsVisible(),
    fog_e = r.IsFogEnabled();
  TGlOption cc = r.LightModel.GetClearColor();
  r.LightModel.SetClearColor(0);
  r.EnableFog(false);
  r.Background()->SetVisible(false);
  r.InitLights();
  r.DrawSilhouette();
  olx_array_ptr<unsigned char> data = (unsigned char *)r.GetPixels(false, 1);
  r.Background()->SetVisible(bg_vis);
  r.EnableFog(fog_e);
  r.LightModel.SetClearColor(cc);
  r.InitLights();

  int w = r.GetWidth(), h = r.GetHeight(), tw = 3 * w, hh = h / 2;
  vec3d SceneOrigin = r.GetBasis().GetCenter();
  mat3d ProjMatr = r.GetBasis().GetMatrix()*r.GetBasis().GetZoom()
    / r.GetScale();
  vec3d DrawOrigin(w / 2, hh, 0);
  mat3d rm;
  olx_create_rotation_matrix(rm, vec3d(0, 0, 1), cos(M_PI / (180/Positions)));
  bool vector_font = Atoms[0]->GetGlLabel().GetFont().IsVectorFont();
  for (size_t i = 0; i < Atoms.Count(); i++) {
    TXGlLabel &l = Atoms[i]->GetGlLabel();
    TTextRect rc = l.GetRect();
    if (vector_font) {
      double scale = r.GetBasis().GetZoom() / r.CalcZoom() / r.GetScale();
      rc.top *= scale;
      rc.left *= scale;
      rc.width *= scale;
      rc.height *= scale;
    }
    rc.height += 2;
    sorted::PrimitiveAssociation<double, olx_pair_t<vec3d, vec3i> >
      positions;
    vec3d v(1, 0, 0);
    for (int j = 0; j < 360 / Positions; j++) {
      vec3d p = v;
      p = r.GetBasis().GetMatrix()*p;
      if ((Atoms[i]->DrawStyle() == adsEllipsoid ||
        Atoms[i]->DrawStyle() == adsOrtep) &&
        Atoms[i]->GetEllipsoid() != NULL)
      {
        p *= 1. / Atoms[i]->GetEllipsoid()->CalcScale(p);
      }
      p *= Atoms[i]->GetDrawScale();
      vec3d off = (p*Offset)*ProjMatr;
      vec3d vp = ((Atoms[i]->crd() + SceneOrigin)*ProjMatr
        + DrawOrigin + off);
      double ovr_extra = 0;
      if (v[0] < 0 && v[0] < -1e-3) {
        vp[0] -= rc.width;
        off[0] -= rc.width;
      }
      else {
        ovr_extra -= 1e-6;
      }
      if (v[1] < 0 && v[1] < -1e-3) {
        vp[1] -= rc.height;
        off[1] -= rc.height;
      }
      else {
        ovr_extra -= 1e-6;
      }
      if (!vector_font)
        off[1] -= rc.height / 2;
      vec3i c = vp;
      positions.Add(calc_overlap(data(), w, h, c[0], c[1], rc) + ovr_extra,
        olx_pair::make(off, c));
      v = v*rm;
    }
    vec3i lr = positions.GetValue(0).b;
    fill_rect(data(), w, h, lr[0], lr[1], rc);
    l.TranslateBasis(positions.GetValue(0).a / r.GetZoom() - l.GetCenter());
    l.SetVisible(true);
  }
}
//..............................................................................
double TXAtomLabelAligner::calc_overlap(unsigned const char *data, size_t w, size_t h,
  size_t x_, size_t y_, const TTextRect &r_)
{
  size_t bw = static_cast<size_t>(r_.width),
    bh = static_cast<size_t>(r_.height),
    cnt = 0, max_idx = h*w * 3;
  for (size_t i = 0; i < bw; i++) {
    size_t x = x_ + i;
    if (x >= w) break;
    for (size_t j = 0; j < bh; j++) {
      size_t y = y_ + j;
      size_t idx = (y*w + x) * 3;
      if (idx >= max_idx) continue;
      if ((data[idx] | data[idx + 1] | data[idx + 2]) != 0) {
        cnt++;
      }
    }
  }
  return double(cnt) / (bh*bw);
}
//..............................................................................
void TXAtomLabelAligner::fill_rect(unsigned char *data, size_t w, size_t h,
  size_t x_, size_t y_, const TTextRect &r_)
{
  size_t bw = static_cast<size_t>(r_.width),
    bh = static_cast<size_t>(r_.height),
    max_idx = h*w * 3;
  for (size_t i = 0; i < bw; i++) {
    size_t x = x_ + i;
    if (x >= w) break;
    for (size_t j = 0; j < bh; j++) {
      size_t y = y_ + j;
      size_t idx = (y*w + x) * 3;
      if (idx >= max_idx) continue;
      data[idx] = data[idx + 1] = data[idx + 2] = 0xfe;
    }
  }
}

//..............................................................................
