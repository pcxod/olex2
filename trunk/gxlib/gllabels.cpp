//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "gllabels.h"
#include "xatom.h"
#include "gxapp.h"
#include "asymmunit.h"
#include "refmodel.h"

//----------------------------------------------------------------------------//
// TXGlLabels function bodies
//----------------------------------------------------------------------------//
TXGlLabels::TXGlLabels(TGlRenderer& Render, const olxstr& collectionName) :
  AGDrawObject(Render, collectionName)
{
  AGDrawObject::SetSelectable(false);
  FMarkMaterial = Render.GetSelection().GetGlM();
  FMarkMaterial.SetFlags(sglmAmbientF|sglmIdentityDraw);
}
//..............................................................................
void TXGlLabels::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  
  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;
  FontIndex = Parent.GetScene().FindFontIndexForType<TXGlLabels>();
  TGlPrimitive& GlP = GPC.NewPrimitive("Text", sgloText);
  GlP.SetProperties(GPC.GetStyle().GetMaterial("Text", GetFont().GetMaterial()));
  GlP.Params[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
void TXGlLabels::Clear()  {  Marks.Clear();  }
//..............................................................................
bool TXGlLabels::Orient(TGlPrimitive& P)  {
  TGlFont &Fnt = GetFont();
  TGXApp& app = TGXApp::GetInstance();
  const size_t ac = app.AtomCount();
  if( ac == 0 || Marks.Count() != ac )
    return true;
  bool currentGlM, matInited = false;
  P.SetFont(&Fnt);
  TGlMaterial& OGlM = P.GetProperties();
  if( Parent.IsATI() )  {
    olx_gl::rasterPos(0, 0, 0);
    olx_gl::callList(Fnt.GetFontBase() + ' ');
  }
  const RefinementModel& rm = app.XFile().GetRM();
  for( size_t i=0; i < ac; i++ )  {
    const TXAtom& XA = app.GetAtom(i);
    if( XA.IsDeleted() || !XA.IsVisible() )  continue;
    if( (Mode & lmHydr) == 0 && (XA.Atom().GetType() == iHydrogenZ) )  
      continue;
    if( (Mode & lmQPeak) == 0 && (XA.Atom().GetType() == iQPeakZ) )  continue;
    if( (Mode & lmIdentity) != 0 && !XA.Atom().GetMatrix(0).IsFirst() )  continue;
    const TCAtom& ca = XA.Atom().CAtom();
    olxstr Tmp(EmptyString, 48);
    if( (Mode & lmLabels) != 0 )  {
      Tmp << XA.Atom().GetLabel();
      if( XA.Atom().CAtom().GetResiId() != 0 )  {
        size_t resi = ca.GetParent()->GetResidue(ca.GetResiId()).GetNumber();
        Tmp << '_' << resi;
      }
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
    if( (Mode & lmQPeakI) != 0 && (XA.Atom().GetType() == iQPeakZ) )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
      Tmp << olxstr::FormatFloat(1, ca.GetQPeak());
    }
    if( (Mode & lmAOcc) != 0 )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
      Tmp << olxstr::FormatFloat(2, rm.Vars.GetParam(ca, catom_var_name_Sof));
    }
    if( (Mode & lmCOccu) != 0 )  {
      const double val = ca.GetOccu()*ca.GetDegeneracy();
      if( olx_abs(val-1) < 1e-5 )  continue;
      if( !Tmp.IsEmpty() )  Tmp << ", ";
      Tmp << olxstr::FormatFloat(3, ca.GetOccu()*ca.GetDegeneracy());
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
        if( ca.GetVarRef(catom_var_name_X+j) != NULL && ca.GetVarRef(catom_var_name_X+j)->relation_type == relation_None )
          fXyz << (olxch)('X'+j);
      }
      if( !fXyz.IsEmpty() )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << fXyz;
      }
      if( ca.GetVarRef(catom_var_name_Sof) != NULL && ca.GetVarRef(catom_var_name_Sof)->relation_type == relation_None )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << "occu";
      }
      if( ca.GetEllipsoid() != NULL )  {
        olxstr eadp((const char*)"Ua:", 40);
        size_t ec=0;
        for( size_t j=0; j < 6; j++ )  {
          if( ca.GetVarRef(catom_var_name_U11+j) != NULL && ca.GetVarRef(catom_var_name_U11+j)->relation_type == relation_None )  {
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
      else if( ca.GetVarRef(catom_var_name_Uiso) != NULL && ca.GetVarRef(catom_var_name_Uiso)->relation_type == relation_None )  {
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
    if( Tmp.IsEmpty() )  continue;
    P.SetString(&Tmp);
    if( !Fnt.IsVectorFont() )  {
      if( !matInited )  {
        if( Marks[i] ) {
          FMarkMaterial.Init(Parent.IsColorStereo());
          currentGlM = false;
          if( Parent.IsATI() )  {
            olx_gl::rasterPos(0, 0, 0);
            olx_gl::callList(Fnt.GetFontBase() + ' ');
          } 
        }
        else  {
          P.GetProperties().Init(Parent.IsColorStereo());
          currentGlM = true;
          if( Parent.IsATI() )  {
            olx_gl::rasterPos(0, 0, 0);
            olx_gl::callList(Fnt.GetFontBase() + ' ');
          } 
        }
        matInited = true;
      }
      else  {
        if( Marks[i] )  {
          if( currentGlM )  {
            FMarkMaterial.Init(Parent.IsColorStereo());
            currentGlM = false;
            if( Parent.IsATI() )  {
              olx_gl::rasterPos(0, 0, 0);
              olx_gl::callList(Fnt.GetFontBase() + ' ');
            } 
          }
        }
        else  {
          if( !currentGlM )  {
            P.GetProperties().Init(Parent.IsColorStereo());
            currentGlM = true;
            if( Parent.IsATI() )  {
              olx_gl::rasterPos(0, 0, 0);
              olx_gl::callList(Fnt.GetFontBase() + ' ');
            } 
          }
        }
      }
      vec3d V = XA.Atom().crd() + Parent.GetBasis().GetCenter();
      V *= Parent.GetBasis().GetMatrix();
      V *= Parent.GetBasis().GetZoom();
      const double MaxZ = (Parent.GetMaxRasterZ()-0.001);
      Parent.DrawText(P, V[0]+0.01, V[1]+0.01, MaxZ);
      olx_gl::rasterPos(V[0]+0.01, V[1]+0.01, MaxZ);
      P.Draw();
    }
    else  {  // vector font?
      vec3d T = Parent.GetBasis().GetCenter() + XA.Atom().crd();
      T *= Parent.GetBasis().GetMatrix();
      T *= Parent.GetBasis().GetZoom();
      T[2] = Parent.GetMaxRasterZ() - 0.001;
      Fnt.DrawGlText(T, Tmp, Parent.GetBasis().GetZoom()*0.75/Parent.CalcZoom(), true);
    }
  }
  OGlM.Init(Parent.IsColorStereo());
  return true;
}
//..............................................................................
void TXGlLabels::Init()  {
  TGXApp& app = TGXApp::GetInstance();
  Marks.SetSize((uint32_t)app.AtomCount());
}
//..............................................................................
void TXGlLabels::Selected(bool On) {  
  AGDrawObject::SetSelected(false);  
}
//..............................................................................
void TXGlLabels::ClearLabelMarks()  {
  Marks.SetAll(false);
}
//..............................................................................
void TXGlLabels::MarkLabel(const TXAtom& A, bool v)  {
  if( A.GetXAppId() < Marks.Count() )
    Marks.Set(A.GetXAppId(), v);
}
//..............................................................................
void TXGlLabels::MarkLabel(size_t i, bool v)  {
  if( i < Marks.Count() )
    Marks.Set(i, v);
}
//..............................................................................
bool TXGlLabels::IsLabelMarked(const TXAtom& atom) const {
  if( atom.GetXAppId() < Marks.Count() )
    return Marks[atom.GetXAppId()];
  return false;  // should not happen...
}
//..............................................................................
bool TXGlLabels::IsLabelMarked(size_t i) const {
  if( i < Marks.Count() )  return Marks[i];
  return false;  // should not happen...
}
//..............................................................................
TGlFont& TXGlLabels::GetFont() const {  return Parent.GetScene().GetFont(FontIndex, true);  }
//..............................................................................

