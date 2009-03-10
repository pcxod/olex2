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
#include "pers_util.h"

olxstr TCAtom::VarNames[] = {"Scale", "X", "Y", "Z", "Sof", "Uiso", "U11", "U22", "U33", "U23", "U13", "U12"};
//----------------------------------------------------------------------------//
// TCAtom function bodies
//----------------------------------------------------------------------------//
TCAtom::TCAtom(TAsymmUnit *Parent)  {
  Part   = 0;
  Occu   = 1;
  QPeak  = 0;
  SetId(0);
  SetResiId(-1);  // default residue is unnamed one
  SetSameId(-1);
  FParent = Parent;
  EllpId = -1;
  Uiso = caDefIso;
  UisoEsd = 0;
  UisoScale = 0;
  UisoOwner = NULL;
  FragmentId = -1;
  CanBeGrown = Deleted = false;
  FAttachedAtoms = NULL;
  FAttachedAtomsI = NULL;
  Degeneracy = 1;
  HAttached = false;
  Saved = false;
  Tag = -1;
  DependentAfixGroup = ParentAfixGroup = NULL;
  DependentHfixGroups = NULL;
  ExyzGroup = NULL;
  memset(Vars, 0, sizeof(Vars));
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
  TAtomsInfo& AI = TAtomsInfo::GetInstance();
  TBasicAtomInfo *BAI = NULL;
  if( L.Length() >= 2 )  {
    Tmp = L.SubString(0, 2);
    if( AI.IsElement(Tmp) )
      BAI = AI.FindAtomInfoBySymbol( Tmp );
    else  {
      Tmp = L.SubString(0, 1);
      if( AI.IsElement(Tmp) )
        BAI = AI.FindAtomInfoBySymbol( Tmp );
    }
  }
  else  {
    Tmp = L.SubString(0, 1);
    if( AI.IsElement(Tmp) )
      BAI = AI.FindAtomInfoBySymbol( Tmp );
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
void TCAtom::SetAtomInfo(TBasicAtomInfo* A)  {
  FAtomInfo = A;
  return;
  olxstr Tmp(A->GetSymbol());
  if( FLabel.Length() > Tmp.Length() )
    Tmp << FLabel.SubStringFrom(FAtomInfo->GetSymbol().Length());

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
  SetOccu( S.GetOccu() );
  SetQPeak( S.GetQPeak() );
  SetResiId( S.GetResiId() );
  SetSameId( S.GetSameId() );
  EllpId = S.EllpId;
  SetUiso( S.GetUiso() );
  SetUisoScale( S.GetUisoScale() );
  if( S.UisoOwner != NULL )  {
    UisoOwner = FParent->FindCAtomById(S.UisoOwner->GetId());
    if( UisoOwner == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
  }
  else
    UisoOwner = NULL;
  FLabel   = S.FLabel;
  FAtomInfo = &S.GetAtomInfo();
//  Frag    = S.Frag;
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
  item.AddField("part", Part);
  item.AddField("sof", Occu);
  item.AddField("x", TEValue<double>(Center[0], Esd[0]).ToString());
  item.AddField("y", TEValue<double>(Center[1], Esd[1]).ToString());
  item.AddField("z", TEValue<double>(Center[2], Esd[2]).ToString());
  if( EllpId == -1 )
    item.AddField("Uiso", Uiso);
  else {
    double Q[6], E[6];
    GetEllipsoid()->GetQuad(Q, E);
    TDataItem& elp = item.AddItem("adp");
    elp.AddField("xx", TEValue<double>(Q[0], E[0]).ToString());
    elp.AddField("yy", TEValue<double>(Q[1], E[1]).ToString());
    elp.AddField("zz", TEValue<double>(Q[2], E[2]).ToString());
    elp.AddField("yz", TEValue<double>(Q[3], E[3]).ToString());
    elp.AddField("xz", TEValue<double>(Q[4], E[4]).ToString());
    elp.AddField("xy", TEValue<double>(Q[5], E[5]).ToString());
  }
  if( *FAtomInfo == iQPeakIndex )
    item.AddField("peak", QPeak);

}
//..............................................................................
#ifndef _NO_PYTHON
PyObject* TCAtom::PyExport()  {
  PyObject* main = PyDict_New();
  PyDict_SetItemString(main, "label", PythonExt::BuildString(FLabel) );
  PyDict_SetItemString(main, "type", PythonExt::BuildString(FAtomInfo->GetSymbol()) );
  PyDict_SetItemString(main, "part", Py_BuildValue("i", Part) );
  PyDict_SetItemString(main, "occu", Py_BuildValue("d", Occu) );
  PyDict_SetItemString(main, "tag", Py_BuildValue("i", GetTag()) );
  PyDict_SetItemString(main, "crd", 
    Py_BuildValue("(ddd)(ddd)", Center[0], Center[1], Center[2], Esd[0], Esd[1], Esd[2]) );
  if( EllpId == -1 )
    PyDict_SetItemString(main, "uiso", Py_BuildValue("(dd)", Uiso, UisoEsd) );
  else  {
    double Q[6], E[6];
    GetEllipsoid()->GetQuad(Q, E);
    PyDict_SetItemString(main, "adp", 
      Py_BuildValue("(dddddd)(dddddd)", Q[0], Q[1], Q[2], Q[3], Q[4], Q[5], 
       E[0], E[1], E[2], E[3], E[4], E[5]
      ) );
  }
  if( *FAtomInfo == iQPeakIndex )
    PyDict_SetItemString(main, "peak", Py_BuildValue("d", QPeak) );
  return main;
}
#endif
//..............................................................................
void TCAtom::FromDataItem(TDataItem& item)  {
  FAtomInfo = TAtomsInfo::GetInstance().FindAtomInfoBySymbol( item.GetRequiredField("type") );
  if( FAtomInfo == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid atom type");
  FLabel = item.GetRequiredField("label");
  Part = item.GetRequiredField("part").ToInt();
  Occu = item.GetRequiredField("sof").ToDouble();
  TEValue<double> ev;
  ev = item.GetRequiredField("x");
  Center[0] = ev.GetV();  Esd[0] = ev.GetE();
  ev = item.GetRequiredField("y");
  Center[1] = ev.GetV();  Esd[1] = ev.GetE();
  ev = item.GetRequiredField("z");
  Center[2] = ev.GetV();  Esd[2] = ev.GetE();

  TDataItem* adp = item.FindItem("adp");
  if( adp != NULL )  {
    double Q[6], E[6];
    if( adp->FieldCount() != 6 )
      throw TInvalidArgumentException(__OlxSourceInfo, "6 parameters expecetd for the ADP");
    for( int i=0; i < 6; i++ )  {
      ev = adp->GetField(i);
      E[i] = ev.GetE();
      Q[i] = ev.GetV();
    }
    EllpId = FParent->NewEllp().Initialise(Q,E).GetId();
    Uiso = GetEllipsoid()->GetUiso();
  }
  else  {
    EllpId = -1;
    Uiso = item.GetRequiredField("Uiso").ToDouble();
  }
  if( *FAtomInfo == iQPeakIndex )
    QPeak = item.GetRequiredField("peak").ToDouble();
}
//..............................................................................
void DigitStrtok(const olxstr &str, TStrPObjList<olxstr,bool>& chars)  {
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
IXVarReferencerContainer& TCAtom::GetParentContainer()  {  return *FParent;  }
//..............................................................................
double TCAtom::GetValue(short var_index) const {
  switch( var_index)  {
    case catom_var_name_X:     return Center[0];
    case catom_var_name_Y:     return Center[1];
    case catom_var_name_Z:     return Center[2];
    case catom_var_name_Sof:   return Occu;
    case catom_var_name_Uiso:  return Uiso;  
    case catom_var_name_U11:
    case catom_var_name_U22:
    case catom_var_name_U33:
    case catom_var_name_U23:
    case catom_var_name_U13:
    case catom_var_name_U12:
      if( EllpId == -1 )
        throw TInvalidArgumentException(__OlxSourceInfo, "Uanis is not defined");
      return FParent->GetEllp(EllpId).GetQuadVal(var_index-catom_var_name_U11);
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "parameter name");
  }
}
//..............................................................................
void TCAtom::SetValue(short var_index, const double& val) {
  static double q[6];
  switch( var_index)  {
    case catom_var_name_X:     Center[0] = val;  break;
    case catom_var_name_Y:     Center[1] = val;  break;
    case catom_var_name_Z:     Center[2] = val;  break;
    case catom_var_name_Sof:   Occu = val;  break;
    case catom_var_name_Uiso:  Uiso = val;  break;  
    case catom_var_name_U11:
    case catom_var_name_U22:
    case catom_var_name_U33:
    case catom_var_name_U23:
    case catom_var_name_U13:
    case catom_var_name_U12:
      break;
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "parameter name");
  }
}
