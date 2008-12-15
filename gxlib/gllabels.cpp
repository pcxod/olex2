
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
#include "asymmunit.h"
#include "refmodel.h"

#include "glrender.h"
#include "glscene.h"

//----------------------------------------------------------------------------//
// TXGlLabels function bodies
//----------------------------------------------------------------------------//
TXGlLabels::TXGlLabels(const olxstr& collectionName, TGlRender *Render) :
  AGDrawObject(collectionName)
{
  AGDrawObject::Parent(Render);
  FFontIndex = -1;
  AGDrawObject::Groupable(false);

  FMarkMaterial = *Render->Selection()->GlM();
  FMarkMaterial.SetFlags(sglmAmbientF|sglmIdentityDraw);
}
//..............................................................................
TXGlLabels::~TXGlLabels() {}
//..............................................................................
void TXGlLabels::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  
  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  if( GPC == NULL )    
    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);

  TGlMaterial* GlM = const_cast<TGlMaterial*>(GPC->Style()->Material("Text"));
  if( GlM->Mark() )
    *GlM = Font()->GetMaterial();
  TGlPrimitive* GlP = GPC->NewPrimitive("Text");
  GlP->SetProperties(GlM);
  GlP->Type(sgloText);
  GlP->Params()[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
void TXGlLabels::Clear()  {
  FAtoms.Clear();
  FMarks.Clear();
}
//..............................................................................
bool TXGlLabels::Orient(TGlPrimitive *P)  {
  TGlFont *Fnt = Font();
  const int ac = AtomCount();
  if( Fnt == NULL || ac == 0 )  return true;

  vec3d V;
  bool currentGlM, matInited = false;
  P->Font(Fnt);
  TGlMaterial *OGlM = (TGlMaterial*)P->GetProperties();
  if( FParent->IsATI() )  {
    glRasterPos3d(0, 0, 0);
    glCallList(Fnt->FontBase() + ' ');
  }
  RefinementModel* rm = Atom(0)->Atom().CAtom().GetParent()->GetRefMod();
  for( int i=0; i < ac; i++ )  {
    TXAtom* XA = Atom(i);
    if( XA->Deleted() || (!XA->Visible()))  continue;
    if( !(Mode & lmHydr) && (XA->Atom().GetAtomInfo() == iHydrogenIndex ) )  continue;
    if( !(Mode & lmQPeak) && (XA->Atom().GetAtomInfo() == iQPeakIndex ) )  continue;
    TCAtom& ca = XA->Atom().CAtom();
    olxstr Tmp(EmptyString, 48);
    if( Mode & lmLabels )  {
      Tmp << XA->Atom().GetLabel();
      if( XA->Atom().CAtom().GetResiId() != -1 )  {
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
      const XVarReference* vr = ca.GetVarRef(var_name_Sof);
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
    if( (Mode & lmQPeakI) && (XA->Atom().GetAtomInfo() == iQPeakIndex ) )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
      Tmp << olxstr::FormatFloat(1, ca.GetQPeak());
    }
    if( Mode & lmAOcc )  {
      if( Tmp.Length() )  Tmp << ", ";
      Tmp << olxstr::FormatFloat(2, rm->Vars.GetAtomParam(ca, var_name_Sof) );
    }
    if( Mode & lmUiso && ca.GetUisoOwner() == NULL )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << olxstr::FormatFloat(2, rm->Vars.GetAtomParam(ca, var_name_Uiso));
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
        if( ca.GetVarRef(var_name_X+j) != NULL && ca.GetVarRef(var_name_X+j)->relation_type == relation_None )
          fXyz << (olxch)('X'+j);
      }
      if( !fXyz.IsEmpty() )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << fXyz;
      }
      if( ca.GetVarRef(var_name_Sof) != NULL && ca.GetVarRef(var_name_Sof)->relation_type == relation_None )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << "occu";
      }
      if( ca.GetEllipsoid() != NULL )  {
        olxstr eadp("Ua:", 40);
        int ec=0;
        for( int j=0; j < 6; j++ )  {
          if( ca.GetVarRef(var_name_U11+j) != NULL && ca.GetVarRef(var_name_U11+j)->relation_type == relation_None )  {
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
      else if( ca.GetVarRef(var_name_Uiso) != NULL && ca.GetVarRef(var_name_Uiso)->relation_type == relation_None )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << "Uiso";
      }
    }
    if( Mode & lmOccp )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
      if( ca.GetOccu() != 1.0 )
        Tmp << olxstr::FormatFloat(3, ca.GetOccu() );
    }
#ifdef _DEBUG
    if( ca.GetSameId() != -1 )
      Tmp << ':' << ca.GetSameId();
#endif
    if( Tmp.IsEmpty() )  continue;
    P->String(&Tmp);
    if( !matInited )  {
      if( FMarks[i] == true ) {
        FMarkMaterial.Init();
        currentGlM = false;
        if( FParent->IsATI() )  {
          glRasterPos3d(0, 0, 0);
          glCallList(Fnt->FontBase() + ' ');
        } 
      }
      else  {
      ((TGlMaterial*)P->GetProperties())->Init();
        currentGlM = true;
        if( FParent->IsATI() )  {
          glRasterPos3d(0, 0, 0);
          glCallList(Fnt->FontBase() + ' ');
        } 
      }
      matInited = true;
    }
    else  {
      if( FMarks[i] )  {
        if( currentGlM )  {
          FMarkMaterial.Init();
          currentGlM = false;
          if( FParent->IsATI() )  {
            glRasterPos3d(0, 0, 0);
            glCallList(Fnt->FontBase() + ' ');
          } 
        }
      }
      else  {
        if( !currentGlM )  {
          ((TGlMaterial*)P->GetProperties())->Init();
          currentGlM = true;
          if( FParent->IsATI() )  {
            glRasterPos3d(0, 0, 0);
            glCallList(Fnt->FontBase() + ' ');
          } 
        }
      }
    }
    V = XA->Atom().crd();
    V += FParent->GetBasis().GetCenter();
    V *= FParent->GetBasis().GetMatrix();
    glRasterPos3d(V[0]+0.15, V[1]+0.15, V[2]+5);
    P->Draw();
  }
  OGlM->Init();
  return true;
}
//..............................................................................
void TXGlLabels::AddAtom(TXAtom *A)  {
  FAtoms.Add(A); 
  FMarks.Add(false);  
}
//..............................................................................
void TXGlLabels::Selected(bool On) {  
  AGDrawObject::Selected(false);  
}
//..............................................................................
void TXGlLabels::ClearLabelMarks()  {
  for( int i=0; i < FMarks.Count(); i++ )
    FMarks[i] = false;;
}
//..............................................................................
void TXGlLabels::MarkLabel(TXAtom *A, bool v)  {
  int i = FAtoms.IndexOf(A);
  if( i != -1 && 1 < FMarks.Count() )
    FMarks[i] = v;
}
//..............................................................................
TGlFont* TXGlLabels::Font() const {  
  return FParent->Scene()->Font(FFontIndex); 
}
//..............................................................................

