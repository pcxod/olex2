
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

#include "glrender.h"
#include "glscene.h"

//----------------------------------------------------------------------------//
// TXGlLabels function bodies
//----------------------------------------------------------------------------//
TXGlLabels::TXGlLabels(const olxstr& collectionName, TGlRender *Render) :
  AGDrawObject(collectionName)
{
  AGDrawObject::Parent(Render);
  FAtoms = new TEList;
  FFontIndex = -1;
  AGDrawObject::Groupable(false);

  // this can be changed to the bitarray object for compactness
  FMarks = new TTypeList<bool>();

  FMarkMaterial = *Render->Selection()->GlM();
  FMarkMaterial.SetFlags(sglmAmbientF|sglmIdentityDraw);
}
//..............................................................................
void TXGlLabels::Create(const olxstr& cName)  {
  if( cName.Length() != 0 )  SetCollectionName(cName);
  TGlPrimitive *GlP;
  TGPCollection *GPC;

  GPC = FParent->FindCollection( GetCollectionName() );
  if( !GPC )    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);

  TGlMaterial* GlM = const_cast<TGlMaterial*>(GPC->Style()->Material("Text"));
  if( GlM->Mark() )
    *GlM = Font()->GetMaterial();
  GlP = GPC->NewPrimitive("Text");
  GlP->SetProperties(GlM);
  GlP->Type(sgloText);
  GlP->Params()[0] = -1;  //bitmap; TTF by default
}
//..............................................................................
TXGlLabels::~TXGlLabels()  {
  Clear();
  delete FAtoms;
  delete FMarks;
}
//..............................................................................
void TXGlLabels::Clear()  {
  FAtoms->Clear();
  FMarks->Clear();
}
//..............................................................................
bool TXGlLabels::Orient(TGlPrimitive *P)  {
  TGlFont *Fnt = Font();
  if( !Fnt )  return true;

  TXAtom *XA;
  TVPointD V;
  bool currentGlM, matInited = false;
  olxstr Tmp;
  P->Font(Fnt);
  TGlMaterial *OGlM = (TGlMaterial*)P->GetProperties();
//  FMaterial.Init();
  for( int i=0; i < AtomCount(); i++ )  {
    XA = Atom(i);
    if( XA->Deleted() || (!XA->Visible()))  continue;
    if( !(FMode & lmHydr) && (XA->Atom().GetAtomInfo() == iHydrogenIndex ) )  continue;
    Tmp  = EmptyString;
    if( FMode & lmLabels )  {
      Tmp << XA->Atom().GetLabel();
      if( XA->Atom().CAtom().GetResiId() != -1 )  {
        int resi = XA->Atom().CAtom().GetParent()->GetResidue(XA->Atom().CAtom().GetResiId()).GetNumber();
        Tmp << '_' << resi;
      }
    }
    if( FMode & lmPart )  {
      if( XA->Atom().CAtom().GetPart() != 0)  {
        if( Tmp.Length() )  Tmp << ", ";
        Tmp << XA->Atom().CAtom().GetPart();
      }
    }
    if( FMode & lmAfix )  {
      if( XA->Atom().CAtom().GetAfix() != 0 ) {
        if( Tmp.Length() )  Tmp << ", ";
        Tmp << XA->Atom().CAtom().GetAfix();
      }
      if( XA->Atom().CAtom().GetHfix() != 0 )  {
        if( Tmp.Length() )  Tmp << ", ";
        Tmp << XA->Atom().CAtom().GetHfix();
      }
    }
    if( FMode & lmOVar )  {
      if( XA->Atom().CAtom().GetOccpVar() != 0 && XA->Atom().CAtom().GetOccpVar() != 10 )  {
        if( Tmp.Length() )  Tmp << ", ";
        Tmp << (int)XA->Atom().CAtom().GetOccpVar();
      }
    }
    if( FMode & lmAOcc )  {
      if( Tmp.Length() )  Tmp << ", ";
      double v;
      if( XA->Atom().CAtom().GetOccpVar() != 0 && XA->Atom().CAtom().GetOccpVar() != 10 )
        v = XA->Atom().CAtom().GetOccpVar();
      else
        v = XA->Atom().CAtom().GetOccpVar() + XA->Atom().CAtom().GetOccp();
      Tmp << olxstr::FormatFloat(2, v );
    }
    if( FMode & lmUiso )  {
      if( XA->Atom().CAtom().GetUisoVar() != 0 )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
          Tmp << olxstr::FormatFloat(2, XA->Atom().CAtom().GetUisoVar());
      }
      else  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << olxstr::FormatFloat(2, XA->Atom().CAtom().GetUiso());
      }
    }
    if( FMode & lmUisR )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
      if( XA->Atom().CAtom().GetUisoVar() < 0)
        Tmp << olxstr::FormatFloat(2, XA->Atom().CAtom().GetUisoVar());
    }
    if( FMode & lmFixed )  {
      olxstr fXyz;
      for( int j=0; j < 3; j++ )  {
        if( XA->Atom().CAtom().FixedValues()[TCAtom::CrdFixedValuesOffset + j] != 0 )
          fXyz << (char)('X'+j);
      }
      if( fXyz.Length() != 0 )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << fXyz;
      }
      if( XA->Atom().CAtom().GetOccpVar() == 10 )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << "occu";
      }
      if( XA->Atom().CAtom().GetUisoVar() != 0 )  {
        if( !Tmp.IsEmpty() )  Tmp << ", ";
        Tmp << "uiso";
      }
    }
    if( FMode & lmOccp )  {
      if( !Tmp.IsEmpty() )  Tmp << ", ";
      if( XA->Atom().CAtom().GetOccp() != 1 )
        Tmp << olxstr::FormatFloat(3, XA->Atom().CAtom().GetOccp() );
    }
    if( Tmp.IsEmpty() )  continue;
    P->String(&Tmp);
    if( !matInited )  {
      if( FMarks->Item(i) == true ) {
        FMarkMaterial.Init();
        currentGlM = false;
      }
      else  {
      ((TGlMaterial*)P->GetProperties())->Init();
        currentGlM = true;
      }
      matInited = true;
    }
    else  {
      if( FMarks->Item(i) == true )  {
        if( currentGlM )  {
          FMarkMaterial.Init();
          currentGlM = false;
        }
      }
      else  {
        if( !currentGlM )  {
          ((TGlMaterial*)P->GetProperties())->Init();
          currentGlM = true;
        }
      }
    }
    V = XA->Atom().Center();
    V += FParent->GetBasis().GetCenter();
    V *= FParent->GetBasis().GetMatrix();
    glRasterPos3d(V[0]+0.15, V[1]+0.15, V[2]+5);
//    V[0] += 0.45;  V[1] += 0.45;  V[2] += 5;
    P->Draw();
//    Fnt->DrawGlText(V, Tmp, false);
  }
  OGlM->Init();
  return true;
}
//..............................................................................
void TXGlLabels::AddAtom(TXAtom *A)
{  FAtoms->Add(A); FMarks->AddACopy(false);  }
//..............................................................................
void TXGlLabels::Selected(bool On){  AGDrawObject::Selected(false);  return;  };
//..............................................................................
void TXGlLabels::Mode(short lmode){  FMode = lmode;  }
//..............................................................................
void TXGlLabels::ClearLabelMarks()
{
  for( int i=0; i < FMarks->Count(); i++ )
    FMarks->Item(i) = false;;
}
//..............................................................................
void TXGlLabels::MarkLabel(TXAtom *A, bool v)  {
  int i = FAtoms->IndexOf(A);
  if( i != -1 && 1 < FMarks->Count() )
    FMarks->Item(i) = v;
}
//..............................................................................
TGlFont* TXGlLabels::Font() const {  return FParent->Scene()->Font(FFontIndex); }
//..............................................................................

