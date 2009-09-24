
//---------------------------------------------------------------------------//
// namespace TEObjects
// TEList - list of void* pointers
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gllabels.h"
#include "glgroup.h"
#include "gpcollection.h"

#include "xatom.h"
#include "gxapp.h"
#include "asymmunit.h"
#include "refmodel.h"

#include "glrender.h"
#include "glscene.h"

//----------------------------------------------------------------------------//
// TXGlLabels function bodies
//----------------------------------------------------------------------------//
TXGlLabels::TXGlLabels(TGlRenderer& Render, const olxstr& collectionName) :
  AGDrawObject(Render, collectionName)
{
  FontIndex = -1;
  AGDrawObject::SetGroupable(false);

  FMarkMaterial = *Render.GetSelection().GlM();
  FMarkMaterial.SetFlags(sglmAmbientF|sglmIdentityDraw);
}
//..............................................................................
TXGlLabels::~TXGlLabels() {}
//..............................................................................
void TXGlLabels::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  
  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGlPrimitive& GlP = GPC.NewPrimitive("Text", sgloText);
  GlP.SetProperties( GPC.GetStyle().GetMaterial("Text", Font()->GetMaterial()) );
  GlP.Params[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
void TXGlLabels::Clear()  {  Marks.Clear();  }
//..............................................................................
bool TXGlLabels::Orient(TGlPrimitive& P)  {
  TGlFont *Fnt = Font();
  TGXApp& app = TGXApp::GetInstance();
  const int ac = app.AtomCount();
  if( Fnt == NULL || ac == 0 )  return true;

  vec3d V;
  bool currentGlM, matInited = false;
  P.SetFont(Fnt);
  TGlMaterial& OGlM = P.GetProperties();
  if( Parent.IsATI() )  {
    glRasterPos3d(0, 0, 0);
    glCallList(Fnt->FontBase() + ' ');
  }
  const RefinementModel& rm = app.XFile().GetRM();
  for( int i=0; i < ac; i++ )  {
    const TXAtom& XA = app.GetAtom(i);
    if( XA.IsDeleted() || (!XA.IsVisible()))  continue;
    if( (Mode & lmHydr) == 0 && (XA.Atom().GetAtomInfo() == iHydrogenIndex || XA.Atom().GetAtomInfo() == iDeuteriumIndex) )  
      continue;
    if( !(Mode & lmQPeak) && (XA.Atom().GetAtomInfo() == iQPeakIndex ) )  continue;
    const TCAtom& ca = XA.Atom().CAtom();
    olxstr Tmp(EmptyString, 48);
    if( Mode & lmLabels )  {
      Tmp << XA.Atom().GetLabel();
      if( XA.Atom().CAtom().GetResiId() != -1 )  {
        int resi = ca.GetParent()->GetResidue(ca.GetResiId()).GetNumber();
        Tmp << '_' << resi;
      }
    }
    if( Mode & lmPart )  {
      if( ca.GetPart() != 0)  {
        if( Tmp.Length() )  Tmp << ", ";
        Tmp << ca.GetPart();
      }
    }
    if( Mode & lmAfix )  {
      if( ca.GetAfix() != 0 ) {
        if( Tmp.Length() )  Tmp << ", ";
        Tmp << ca.GetAfix();
      }
    }
    if( Mode & lmOVar )  {
      const XVarReference* vr = ca.GetVarRef(catom_var_name_Sof);
      if( vr != NULL )  {
        if( vr->relation_type != relation_None )  {
          if( Tmp.Length() )  Tmp << ", ";
          if( vr->relation_type == relation_AsVar )
            Tmp << vr->Parent.GetId()+1;
          else
            Tmp << -(vr->Parent.GetId()+1);
        }
      }
    }
    if( (Mode & lmQPeakI) && (XA.Atom().GetAtomInfo() == iQPeakIndex ) )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
      Tmp << olxstr::FormatFloat(1, ca.GetQPeak());
    }
    if( Mode & lmAOcc )  {
      if( Tmp.Length() )  Tmp << ", ";
      Tmp << olxstr::FormatFloat(2, rm.Vars.GetParam(ca, catom_var_name_Sof) );
    }
    if( Mode & lmUiso && ca.GetUisoOwner() == NULL )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << olxstr::FormatFloat(2, rm.Vars.GetParam(ca, catom_var_name_Uiso));
    }
    if( Mode & lmUisR )  {
      if( ca.GetUisoOwner() != NULL )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
          Tmp << olxstr::FormatFloat(2, ca.GetUisoScale());
      }
    }
    if( Mode & lmFixed )  {
      olxstr fXyz;
      for( int j=0; j < 3; j++ )  {
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
        int ec=0;
        for( int j=0; j < 6; j++ )  {
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
    if( Mode & lmOccp )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
      if( ca.GetOccu() != 1.0 )
        Tmp << olxstr::FormatFloat(3, ca.GetOccu() );
    }
    if( Mode & lmConRes )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
      if( ca.GetOccu() != 1.0 )
        Tmp << olxstr::FormatFloat(3, ca.GetOccu() );
    }
#ifdef _DEBUG
    if( ca.GetSameId() != -1 )
      Tmp << ':' << ca.GetSameId();
#endif
    if( Tmp.IsEmpty() )  continue;
    P.SetString(&Tmp);
    if( !matInited )  {
      if( Marks[i] ) {
        FMarkMaterial.Init();
        currentGlM = false;
        if( Parent.IsATI() )  {
          glRasterPos3d(0, 0, 0);
          glCallList(Fnt->FontBase() + ' ');
        } 
      }
      else  {
      P.GetProperties().Init();
        currentGlM = true;
        if( Parent.IsATI() )  {
          glRasterPos3d(0, 0, 0);
          glCallList(Fnt->FontBase() + ' ');
        } 
      }
      matInited = true;
    }
    else  {
      if( Marks[i] )  {
        if( currentGlM )  {
          FMarkMaterial.Init();
          currentGlM = false;
          if( Parent.IsATI() )  {
            glRasterPos3d(0, 0, 0);
            glCallList(Fnt->FontBase() + ' ');
          } 
        }
      }
      else  {
        if( !currentGlM )  {
          P.GetProperties().Init();
          currentGlM = true;
          if( Parent.IsATI() )  {
            glRasterPos3d(0, 0, 0);
            glCallList(Fnt->FontBase() + ' ');
          } 
        }
      }
    }
    V = XA.Atom().crd();
    V += Parent.GetBasis().GetCenter();
    V *= Parent.GetBasis().GetMatrix();
    glRasterPos3d(V[0]+0.15, V[1]+0.15, Parent.GetMaxRasterZ());
    P.Draw();
  }
  OGlM.Init();
  return true;
}
//..............................................................................
void TXGlLabels::Init()  {
  TGXApp& app = TGXApp::GetInstance();
  Marks.SetSize( app.AtomCount() );
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
void TXGlLabels::MarkLabel(int i, bool v)  {
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
bool TXGlLabels::IsLabelMarked(int i) const {
  if( i < Marks.Count() )
    return Marks[i];
  return false;  // should not happen...
}
//..............................................................................
TGlFont* TXGlLabels::Font() const {  
  return Parent.GetScene().GetFont(FontIndex); 
}
//..............................................................................

