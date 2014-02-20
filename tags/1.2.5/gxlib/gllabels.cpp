/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gllabels.h"
#include "xatom.h"
#include "gxapp.h"
#include "asymmunit.h"
#include "refmodel.h"

TXGlLabels::TXGlLabels(TGlRenderer& Render, const olxstr& collectionName)
  : AGDrawObject(Render, collectionName)
{
  AGDrawObject::SetSelectable(false);
  Colors_ << 0 << 0xff;
  Mode = 0;
}
//..............................................................................
void TXGlLabels::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )
    SetCollectionName(cName);
  
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;
  FontIndex = Parent.GetScene().FindFontIndexForType<TXGlLabels>();
  TGlPrimitive& GlP = GPC.NewPrimitive("Text", sgloText);
  GlP.SetProperties(
    GPC.GetStyle().GetMaterial("Text", GetFont().GetMaterial()));
  GlP.Params[0] = -1;  //bitmap; TTF by default
  TGraphicsStyle &gs = GPC.GetStyle();
  Colors_[0] = gs.GetNumParam("processed_color", Colors_[0], true);
  Colors_[1] = gs.GetNumParam("duplicate_color", Colors_[1], true);
}
//..............................................................................
void TXGlLabels::Clear()  {  Marks.Clear();  }
//..............................................................................
void TXGlLabels::RenderLabel(const vec3d &crd, const olxstr &label,
  size_t index, TXGlLabels::RenderContext &rc) const
{
  if (label.IsEmpty())  return;
  rc.primitive.SetString(&label);
  const double Z = Parent.CalcRasterZ(0.001);
  if (!rc.font.IsVectorFont()) {
    if (!rc.matInitialised) {
      if (Marks[index] < Colors_.Count()) {
        rc.GlM.AmbientF = Colors_[Marks[index]];
        rc.GlM.Init(Parent.ForcePlain());
        rc.currentMaterial = Marks[index];
        rc.font.Reset_ATI(rc.optimizeATI);
      }
      else  {
        rc.primitive.GetProperties().Init(Parent.ForcePlain());
        rc.currentMaterial = -1;
        rc.font.Reset_ATI(rc.optimizeATI);
      }
      rc.matInitialised = true;
    }
    else  {
      if (Marks[index] < Colors_.Count()) {
        if (rc.currentMaterial != Marks[index])  {
          rc.GlM.AmbientF = Colors_[Marks[index]];
          rc.GlM.Init(Parent.ForcePlain());
          rc.currentMaterial = Marks[index];
          rc.font.Reset_ATI(rc.optimizeATI);
        }
      }
      else  {
        if (rc.currentMaterial != -1) {
          rc.primitive.GetProperties().Init(Parent.ForcePlain());
          rc.currentMaterial = -1;
          rc.font.Reset_ATI(rc.optimizeATI);
        }
      }
    }
    vec3d V = crd + Parent.GetBasis().GetCenter();
    V *= Parent.GetBasis().GetMatrix();
    V *= Parent.GetBasis().GetZoom();
    if( Parent.GetExtraZoom() > 1 )  {
      V *= (1./Parent.GetScale());
      Parent.DrawTextSafe(vec3d(V[0]+0.01, V[1]+0.01, Z), label, rc.font);
    }
    else  {
      olx_gl::rasterPos(V[0]+0.01, V[1]+0.01, Z);
      rc.primitive.Draw();
    }
  }
  else  {  // vector font?
    vec3d T = Parent.GetBasis().GetCenter() + crd;
    T *= Parent.GetBasis().GetMatrix();
    T *= Parent.GetBasis().GetZoom();
    T[2] = Z;
    rc.font.DrawVectorText(T, label, rc.vectorZoom);
  }
}
//..............................................................................
bool TXGlLabels::Orient(TGlPrimitive& P)  {
  if (Mode == 0) return true;
  TGlFont &Fnt = GetFont();
  TGXApp& app = TGXApp::GetInstance();
  bool matInited = false;
  const bool zoomed_rendering = Parent.GetExtraZoom() > 1;
  RenderContext rc = {P, Fnt, matInited, P.GetProperties(), -1,
    (Parent.IsATI() && !zoomed_rendering),
    Parent.GetBasis().GetZoom()/Parent.CalcZoom()
  };
  P.SetFont(&Fnt);
  Fnt.Reset_ATI(rc.optimizeATI);
  if ((Mode&lmBonds) == 0) {
    TGXApp::AtomIterator ai = app.GetAtoms();
    if (ai.count == 0 || Marks.Count() < ai.count)
      return true;
    const RefinementModel& rm = app.XFile().GetRM();
    for (size_t i=0; ai.HasNext(); i++) {
      const TXAtom& XA = ai.Next();
      if (Marks[i] == lmiMasked) continue;
      if( XA.IsDeleted() || !XA.IsVisible() )  continue;
      if( (Mode & lmHydr) == 0 && (XA.GetType() == iHydrogenZ) )
        continue;
      if( (Mode & lmQPeak) == 0 && (XA.GetType() == iQPeakZ) )  continue;
      if( (Mode & lmIdentity) != 0 && !XA.IsAUAtom() )  continue;
      const TCAtom& ca = XA.CAtom();
      olxstr Tmp(EmptyString(), 48);
      if( (Mode & lmLabels) != 0 )  {
        Tmp << XA.CAtom().GetResiLabel();
      }
      if( (Mode & lmPart) != 0 && ca.GetPart() != 0 )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << (int)ca.GetPart();
      }
      if( (Mode & lmAfix) != 0 && ca.GetAfix() != 0 ) {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << ca.GetAfix();
      }
      if( (Mode & lmOVar) != 0 )  {
        const XVarReference* vr = ca.GetVarRef(catom_var_name_Sof);
        if( vr != NULL )  {
          if( vr->relation_type != relation_None )  {
            if( !Tmp.IsEmpty() )  Tmp << ", ";
            if( vr->relation_type == relation_AsVar )
              Tmp << vr->Parent.GetId()+1;
            else
              Tmp << -(int)(vr->Parent.GetId()+1);
          }
        }
      }
      if( (Mode & lmQPeakI) != 0 && (XA.GetType() == iQPeakZ) )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << olxstr::FormatFloat(1, ca.GetQPeak());
      }
      if( (Mode & lmAOcc) != 0 )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << olxstr::FormatFloat(2, rm.Vars.GetParam(ca, catom_var_name_Sof));
      }
      if( (Mode & lmCOccu) != 0 )  {
        const double val = ca.GetChemOccu();
        if( olx_abs(val-1) < 1e-5 )  continue;
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << olxstr::FormatFloat(3, val);
      }
      if( (Mode & lmUiso) != 0 && ca.GetUisoOwner() == NULL )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << olxstr::FormatFloat(2, rm.Vars.GetParam(ca, catom_var_name_Uiso));
      }
      if( (Mode & lmUisR) != 0 )  {
        if( ca.GetUisoOwner() != NULL )  {
          if( !Tmp.IsEmpty() )  Tmp << ", ";
          Tmp << olxstr::FormatFloat(2, ca.GetUisoScale());
        }
      }
      if( (Mode & lmFixed) != 0 )  {
        olxstr fXyz;
        for( size_t j=0; j < 3; j++ )  {
          if( ca.GetVarRef(catom_var_name_X+j) != NULL &&
              ca.GetVarRef(catom_var_name_X+j)->relation_type == relation_None )
          {
            fXyz << (olxch)('X'+j);
          }
        }
        if( !fXyz.IsEmpty() )  {
          if( !Tmp.IsEmpty() )  Tmp << ", ";
          Tmp << fXyz;
        }
        if( ca.GetVarRef(catom_var_name_Sof) != NULL &&
            ca.GetVarRef(catom_var_name_Sof)->relation_type == relation_None )
        {
          if( !Tmp.IsEmpty() )  Tmp << ", ";
          Tmp << "occu";
        }
        if( ca.GetEllipsoid() != NULL )  {
          olxstr eadp((const char*)"Ua:", 40);
          size_t ec=0;
          for( size_t j=0; j < 6; j++ )  {
            if( ca.GetVarRef(catom_var_name_U11+j) != NULL &&
                ca.GetVarRef(catom_var_name_U11+j)->relation_type == relation_None )
            {
              ec++;
              eadp << (olxch)('A'+j);
            }
          }
          if( ec > 0 )  {
            if( !Tmp.IsEmpty() )  Tmp << ", ";
            if( ec == 6 )
              Tmp << "Uani";
            else Tmp << eadp;
          }
        }
        else if( ca.GetVarRef(catom_var_name_Uiso) != NULL &&
                 ca.GetVarRef(catom_var_name_Uiso)->relation_type == relation_None )
        {
          if( !Tmp.IsEmpty() )  Tmp << ", ";
          Tmp << "Uiso";
        }
      }
      if( (Mode & lmOccp) != 0 && ca.GetOccu() != 1.0 )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << olxstr::FormatFloat(3, ca.GetOccu());
      }
      if( (Mode & lmConRes) != 0 && ca.GetOccu() != 1.0 )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << olxstr::FormatFloat(3, ca.GetOccu());
      }
  #ifdef _DEBUG
      if( olx_is_valid_index(ca.GetSameId()) )
        Tmp << ':' << ca.GetSameId();
  #endif
      RenderLabel(XA.crd(), Tmp, i, rc);
    }
  }
  else { // bonds
    TGXApp::BondIterator bi = app.GetBonds();
    if (bi.count == 0 || Marks.Count() < bi.count)
      return true;
    for (size_t i=0; bi.HasNext(); i++) {
      TXBond &b = bi.Next();
      if (!b.IsVisible() || Marks[i] == lmiMasked) continue;
      if( (Mode & lmHydr) == 0 &&
        (b.A().GetType() == iHydrogenZ || b.B().GetType() == iHydrogenZ))
      {
        continue;
      }
      RenderLabel((b.A().crd()+b.B().crd())/2,
        olxstr::FormatFloat(3, b.A().crd().DistanceTo(b.B().crd())),
        i, rc);
    }
  }
  P.GetProperties().Init(Parent.ForcePlain());
  return true;
}
//..............................................................................
void TXGlLabels::Init(bool clear, uint8_t value) {
  if (Mode == 0) return;
  bool size_changed = false;
  if ((Mode&lmBonds) != 0) {
    size_t sz = TGXApp::GetInstance().GetBonds().count;
    if (sz > Marks.Count()) {
      Marks.SetCount(sz);
      size_changed = true;
    }
  }
  else {
    size_t sz = TGXApp::GetInstance().GetAtoms().count;
    if (sz > Marks.Count()) {
      Marks.SetCount(sz);
      size_changed = true;
    }
  }
  if (clear || size_changed)
    ClearLabelMarks(value);
}
//..............................................................................
void TXGlLabels::Selected(bool On) {
  AGDrawObject::SetSelected(false);
}
//..............................................................................
void TXGlLabels::ClearLabelMarks(uint8_t value) {
  Marks.ForEach(olx_list_init::value(value));
}
//..............................................................................
void TXGlLabels::MarkLabel(const TXAtom& A, bool v)  {
  if( A.GetOwnerId() < Marks.Count() )
    Marks[A.GetOwnerId()] = v ? lmiMark : ~0;
}
//..............................................................................
void TXGlLabels::MarkLabel(size_t i, bool v)  {
  if (i < Marks.Count())
    Marks[i] = v ? lmiMark : ~0;
}
//..............................................................................
bool TXGlLabels::IsLabelMarked(const TXAtom& atom) const {
  return IsLabelMarked(atom.GetOwnerId());
}
//..............................................................................
bool TXGlLabels::IsLabelMarked(size_t i) const {
  if(i < Marks.Count()) return Marks[i] == lmiMark;
  return false;  // should not happen...
}
//..............................................................................
TGlFont& TXGlLabels::GetFont() const {
  return Parent.GetScene().GetFont(FontIndex, true);
}
//..............................................................................
void TXGlLabels::SetMaterialIndex(size_t idx, LabelMaterialIndex mi) {
  if (idx < Marks.Count())
    Marks[idx] = mi;
}
//..............................................................................
