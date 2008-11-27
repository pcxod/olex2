//---------------------------------------------------------------------------//
// namespace TXClasses: TCAtom - basic crystalographic atom
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "catom.h"
#include "ellipsoid.h"
#include "asymmunit.h"
#include "exception.h"
#include "estrlist.h"
#include "refmodel.h"

const short TCAtom::CrdFixedValuesOffset  = 0;
const short TCAtom::OccpFixedValuesOffset = 3;
const short TCAtom::UisoFixedValuesOffset = 4;
//----------------------------------------------------------------------------//
// TCAtom function bodies
//----------------------------------------------------------------------------//
TCAtom::TCAtom(TAsymmUnit *Parent)  {
  Part   = 0;
  Occp   = 1;
  QPeak  = -1;
  SetId(0);
  SetResiId(-1);  // default residue is unnamed one
  SetSameId(-1);
  FParent = Parent;
  EllpId = -1;
  Uiso = caDefIso;
  UisoEsd = 0;
  LoaderId = -1;
  FragmentId = -1;
  CanBeGrown = Deleted = false;
  FAttachedAtoms = NULL;
  FAttachedAtomsI = NULL;
  Degeneracy = 1;
  FFixedValues.Resize(10);
  HAttached = false;
  Saved = false;
  Tag = -1;
  DependentAfixGroup = ParentAfixGroup = NULL;
  DependentHfixGroups = NULL;
  ExyzGroup = NULL;
}
//..............................................................................
TCAtom::~TCAtom()  {
  if( FAttachedAtoms != NULL )   delete FAttachedAtoms;
  if( FAttachedAtomsI != NULL )  delete FAttachedAtomsI;
  if( DependentHfixGroups != NULL )  delete DependentHfixGroups; 
}
//..............................................................................
bool TCAtom::SetLabel(const olxstr &L)  {
  if( L.IsEmpty() )
    throw TInvalidArgumentException(__OlxSourceInfo, "empty label");

  olxstr Tmp;
  TAtomsInfo *AI = AtomsInfo();
  TBasicAtomInfo *BAI = NULL;
  if( L.Length() >= 2 )  {
    Tmp = L.SubString(0, 2);
    if( AI->IsElement(Tmp) )
      BAI = AI->FindAtomInfoBySymbol( Tmp );
    else  {
      Tmp = L.SubString(0, 1);
      if( AI->IsElement(Tmp) )
        BAI = AI->FindAtomInfoBySymbol( Tmp );
    }
  }
  else  {
    Tmp = L.SubString(0, 1);
    if( AI->IsElement(Tmp) )
      BAI = AI->FindAtomInfoBySymbol( Tmp );
  }
  if( BAI == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Unknown element: '") << L << '\'' );
  else  {
    FAtomInfo = BAI;
    FLabel = L;
    if( *BAI != iQPeakIndex )
      SetQPeak(0);
  }

  if( GetAtomInfo().GetSymbol().Length() == 2 )
      FLabel[1] = FLabel.o_tolower(FLabel[1]);
  return true;
}
//..............................................................................
void TCAtom::AtomInfo(TBasicAtomInfo* A)  {
  FAtomInfo = A;
  return;
  olxstr Tmp(A->GetSymbol());
  if( FLabel.Length() > Tmp.Length() )
    Tmp << FLabel.SubStringFrom(FAtomInfo->GetSymbol().Length());

  FAtomInfo = A;
  FLabel = FParent->CheckLabel(this, Tmp);
//  FLabel = Tmp;
}
//..............................................................................
void TCAtom::Assign(const TCAtom& S)  {
  DependentAfixGroup = ParentAfixGroup = NULL;  // managed by the group
  if( DependentHfixGroups != NULL )  {
    delete DependentHfixGroups;
    DependentHfixGroups = NULL;
  }
  ExyzGroup = NULL;  // also managed by the group
  SetPart( S.GetPart() );
  SetOccp( S.GetOccp() );
  SetOccpVar( S.GetOccpVar() );
  SetQPeak( S.GetQPeak() );
  SetResiId( S.GetResiId() );
  SetSameId( S.GetSameId() );
  EllpId = S.EllpId;
  SetUiso( S.GetUiso() );
  SetUisoVar( S.GetUisoVar() );
  FLabel   = S.FLabel;
  FAtomInfo = &S.GetAtomInfo();
//  Frag    = S.Frag;
  LoaderId = S.GetLoaderId();
  Id = S.GetId();
  FragmentId = S.GetFragmentId();
  Center = S.Center;
  Esd = S.Esd;
  SetDeleted( S.IsDeleted() );
  SetCanBeGrown( S.GetCanBeGrown() );
  Degeneracy = S.GetDegeneracy();
  UisoEsd = S.GetUisoEsd();

  /*
  if( FAttachedAtoms )  FAttachedAtomS.Clear();
  if( S.AttachedAtomCount() )
  {
    for(int i=0; i < S.AttachedAtomCount(); i++ )
    {
      AddAttachedAtom( FParent->Atom(S.AttachedAtom(i)->Label()));
    }
  }
  else
  {
    if( FAttachedAtoms )  {  delete FAttachedAtoms;  FAttachedAtoms = NULL;  }
  }
  */
  FFixedValues = S.GetFixedValues();
}
//..............................................................................
int TCAtom::GetAfix() const {
  if( ParentAfixGroup == NULL )  {
    if( DependentAfixGroup != NULL && (DependentAfixGroup->IsFitted() || DependentAfixGroup->GetM() == 0) )
      return DependentAfixGroup->GetAfix();
    //if( DependentHfixGroup != NULL && !DependentHfixGroup->IsRiding() )
    //  return DependentHfixGroup->GetAfix();
    return 0;
  }
  if( ParentAfixGroup->HasExcplicitPivot() )
    return (ParentAfixGroup->GetAfix()/10)*10 + 5;
  else
    return ParentAfixGroup->GetAfix();
}
//..............................................................................
TAtomsInfo* TCAtom::AtomsInfo() const {  return FParent->GetAtomsInfo(); }
//..............................................................................
TEllipsoid* TCAtom::GetEllipsoid() const {  return EllpId == -1 ? NULL : &FParent->GetEllp(EllpId);  }
//..............................................................................
void TCAtom::AssignEllp(TEllipsoid* NV) {  NV == NULL ? EllpId = -1 : EllpId = NV->GetId();  }
//..............................................................................
void TCAtom::UpdateEllp(const TEllipsoid &NV ) {
  double Q[6], E[6];
  NV.GetQuad(Q, E);
  if( EllpId == -1 )  {
    TEllipsoid& elp = FParent->NewEllp();
    elp.Initialise(Q, E);
    EllpId = elp.GetId();
  }
  else
    FParent->GetEllp(EllpId).Initialise(Q);
}
//..............................................................................
void TCAtom::ToDataItem(TDataItem& item) const  {
  item.AddField("label", FLabel );
  item.AddField("type", FAtomInfo->GetSymbol() );
  if( Part != 0 )
    item.AddField("part", Part);
  if( EllpId == -1 )
    item.AddField("Uiso", (GetUisoVar() != 0) ? GetUisoVar() : Uiso);
  else  {
    double Q[6], E[6];
    GetEllipsoid()->GetQuad(Q, E);
    TDataItem& elp = item.AddItem("adp");
    elp.AddField("xx", TEValue<double>(Q[0]+FFixedValues[UisoFixedValuesOffset+0], E[0]).ToString());
    elp.AddField("yy", TEValue<double>(Q[1]+FFixedValues[UisoFixedValuesOffset+1], E[1]).ToString());
    elp.AddField("zz", TEValue<double>(Q[2]+FFixedValues[UisoFixedValuesOffset+2], E[2]).ToString());
    elp.AddField("yz", TEValue<double>(Q[3]+FFixedValues[UisoFixedValuesOffset+3], E[3]).ToString());
    elp.AddField("xz", TEValue<double>(Q[4]+FFixedValues[UisoFixedValuesOffset+4], E[4]).ToString());
    elp.AddField("xy", TEValue<double>(Q[5]+FFixedValues[UisoFixedValuesOffset+5], E[5]).ToString());
  }
  item.AddField("occu", (GetOccpVar() != 0 && GetOccpVar() != 10) ? 
    GetOccpVar() : GetOccpVar() + GetOccp());
  
  item.AddField("x", TEValue<double>(Center[0]+FFixedValues[TCAtom::CrdFixedValuesOffset+0], Esd[0]).ToString());
  item.AddField("y", TEValue<double>(Center[1]+FFixedValues[TCAtom::CrdFixedValuesOffset+1], Esd[1]).ToString());
  item.AddField("z", TEValue<double>(Center[2]+FFixedValues[TCAtom::CrdFixedValuesOffset+2], Esd[2]).ToString());
  
}
//..............................................................................
void TCAtom::FromDataItem(TDataItem& item)  {
  FAtomInfo = TAtomsInfo::GetInstance().FindAtomInfoBySymbol( item.GetFieldValue("type") );
  if( FAtomInfo == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid atom type");

}
//..............................................................................
void DigitStrtok(const olxstr &str, TStrPObjList<olxstr,bool> &chars)  {
  olxstr Dig, Char;
  for(int i=0; i < str.Length(); i++ )  {
    if( str[i] <= '9' && str[i] >= '0' )  {
      if( !Char.IsEmpty() )      {
        chars.Add(Char, true);
        Char = EmptyString;
      }
      Dig << str[i];
    }
    else  {
      if( !Dig.IsEmpty() )  {
        chars.Add(Dig, false);
        Dig = EmptyString;
      }
      Char << str[i];
    }
  }
  if( !Char.IsEmpty() )  chars.Add(Char, true);
  if( !Dig.IsEmpty() )   chars.Add(Dig, false);
}
//..............................................................................
int TCAtom::CompareAtomLabels(const olxstr& S, const olxstr& S1)  {
  TStrPObjList<olxstr, bool> Chars1, Chars2;

  DigitStrtok(S, Chars1);
  DigitStrtok(S1, Chars2);
  for( int i=0; i < olx_min(Chars1.Count(), Chars2.Count()); i++ )  {
    if( Chars1.Object(i) && Chars2.Object(i) )  {
      int res = Chars1.String(i).Comparei(Chars2.String(i));
      if( res != 0 )  return res;
    }
    if( !Chars1.Object(i) && !Chars2.Object(i) )  {
      int res = Chars1[i].ToInt() - Chars2[i].ToInt();
      //if( !res )  // to tackle 01 < 1
      //{  res = Chars1->String(i).Length() - Chars2->String(i).Length();  }
      //the following commented line allows sorting like C01 C02 and C01A then
      //but though it looks better, it is not correct, so using C01 C01A C02
      //if( res && (Chars1->Count() == Chars2->Count()))  return res;
      if( res != 0 )  return res;
    }

    if( !Chars1.Object(i) && Chars2.Object(i) )  return 1;
    if( Chars1.Object(i) && !Chars2.Object(i) )  return -1;
  }
  return Chars1.Count() - Chars2.Count();
}
//..............................................................................
void TCAtom::AttachAtom(TCAtom *CA)  {
  if( FAttachedAtoms == NULL )  FAttachedAtoms = new TCAtomPList;
  FAttachedAtoms->Add(CA);
}
//..............................................................................
void TCAtom::AttachAtomI(TCAtom *CA)  {
  if( !FAttachedAtomsI )  FAttachedAtomsI = new TCAtomPList;
  FAttachedAtomsI->Add(CA);
}
//..............................................................................
//..............................................................................
//..............................................................................
olxstr TGroupCAtom::GetFullLabel(RefinementModel& rm) const  {
  olxstr name(Atom->GetLabel());
  if( Atom->GetResiId() == -1 )  {
    if( Matrix != 0 )
      name << "_$" << (rm.UsedSymmIndex(*Matrix) + 1);
  }
  else  {
    name << '_' << Atom->GetParent()->GetResidue(Atom->GetResiId()).GetNumber();
    if( Matrix != 0 )
      name << '$' << (rm.UsedSymmIndex(*Matrix) + 1);
  }
  return name;
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAfixGroup::Clear()  {  Parent.Delete(Id);  }
//..............................................................................
void TAfixGroup::Assign(TAsymmUnit& tau, const TAfixGroup& ag)  {
  D = ag.D;
  Sof = ag.Sof;
  U = ag.U;
  Afix = ag.Afix;
  
  Pivot = tau.FindCAtomByLoaderId(ag.Pivot->GetLoaderId());
  if( Pivot == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
  SetPivot( *Pivot );
  for( int i=0; i < ag.Dependent.Count(); i++ )  {
    Dependent.Add( tau.FindCAtomByLoaderId( ag.Dependent[i]->GetLoaderId()) );
    if( Dependent.Last() == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
    Dependent.Last()->SetParentAfixGroup(this);
  }
}
//..............................................................................
void TAfixGroup::ToDataItem(TDataItem& item) const {
  item.AddField("afix", Afix);
  item.AddField("d", D);
  item.AddField("u", U);
  item.AddField("pivot", Pivot->GetTag());
  TDataItem& dep = item.AddItem("dependent");
  int dep_id = 0;
  for( int i=0; i < Dependent.Count(); i++ )  {
    if( Dependent[i]->IsDeleted() )  continue;
    dep.AddField(dep_id++, Dependent[i]->GetTag());
  }
}
//..............................................................................
void TAfixGroup::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAfixGroups::ToDataItem(TDataItem& item) const {
  int group_id = 0;
  for( int i=0; i < Groups.Count(); i++ )  {
    if( Groups[i].IsEmpty() )  continue;
    Groups[i].ToDataItem( item.AddItem(group_id++) );
  }
}
//..............................................................................
void TAfixGroups::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TExyzGroup::Clear()  {  Parent.Delete(Id);  }
//..............................................................................
void TExyzGroup::Assign(TAsymmUnit& tau, const TExyzGroup& ag)  {
  for( int i=0; i < ag.Atoms.Count(); i++ )  {
    Atoms.Add( tau.FindCAtomByLoaderId( ag.Atoms[i]->GetLoaderId()) );
    if( Atoms.Last() == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
    Atoms.Last()->SetExyzGroup(this);
  }
}
//..............................................................................
void TExyzGroup::ToDataItem(TDataItem& item) const {
  int atom_id = 0;
  for( int i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    item.AddField(atom_id++, Atoms[i]->GetTag());
  }
}
//..............................................................................
void TExyzGroup::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TExyzGroups::ToDataItem(TDataItem& item) const {
  int group_id = 0;
  for( int i=0; i < Groups.Count(); i++ )  {
    if( Groups[i].IsEmpty() )  continue;
    Groups[i].ToDataItem( item.AddItem(group_id++) );
  }
}
//..............................................................................
void TExyzGroups::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
