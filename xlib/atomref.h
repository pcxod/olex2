#ifndef __OLX__ATOM_GROUP
#define __OLX__ATOM_GROUP
#include "asymmunit.h"

BeginXlibNamespace()
/* Possible Shelx syntax
  C_*  - 'C' atom for all residues
  C_Res - 'C' of particular residue
  $C_Res - ? all carbons of particular residue
  $C_Res$Eqiv - all carbons of Res in Eqiv symmetry
  C_$1
  C_+, C_- - cannot be expanded, just references for previous and next residues
    when something is being calculated
  O21 > O25 -> [021..O25] 
  O25 < O21 -> [025..O21] 
*/
//TODO: add C? C?1 kind of expressions
class ASelectionOwner {
  bool DoClearSelection;
public:
  ASelectionOwner() : DoClearSelection(true) {}
  virtual void ExpandSelection(TCAtomGroup& catoms) = 0;
  DefPropP(bool, DoClearSelection)
};
class TAtomReference : public IEObject  {
  olxstr Expression;
protected:
  inline bool IsValidAtom(TCAtom* ca)  {
    return !(ca->IsDeleted() || ca->GetAtomInfo() == iHydrogenIndex ||
                                ca->GetAtomInfo() == iDeuteriumIndex ||
                                ca->GetAtomInfo() == iQPeakIndex );
  }
  ASelectionOwner* SelectionOwner;
public:
  TAtomReference(const olxstr& expression, ASelectionOwner* selectionOwner = NULL) : 
      Expression(expression), SelectionOwner(selectionOwner)  {  }
  const olxstr& GetExpression() const {  return Expression;  }
  int _Expand(TAsymmUnit& au, TCAtomGroup& atoms, TAsymmUnit::TResidue* CurrResi)  {
    int ac = atoms.Count();
    if( Expression.IsEmpty() )  {  // all atoms of au
      atoms.SetCapacity( atoms.Count() + au.AtomCount() );
      for( int i=0; i < au.AtomCount(); i++ )  {
        TCAtom* ca = &au.GetAtom(i);
        if( IsValidAtom(ca) )
          atoms.AddNew( ca );
      }
      return atoms.Count() - ac;
    }
    else if( Expression.Comparei("sel") == 0 )  { 
      if( SelectionOwner == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, "invalid slection owner");
      SelectionOwner->ExpandSelection(atoms);
    }
    else if( Expression.Comparei("first") == 0 )  { 
      if( au.AtomCount() == 0 )  return 0;
      int i=0;
      TCAtom* ca = &au.GetAtom(i);
      while( (i+1) < au.AtomCount() && !IsValidAtom(ca) )  {
        i++;
        ca = &au.GetAtom(i);
      }
      if( !IsValidAtom(ca) )  return 0;
      atoms.AddNew( ca );
      return 1;
    }
    else if( Expression.StartsFrom("#c") )  { 
      if( au.AtomCount() == 0 )  return 0;
      int i= Expression.SubStringFrom(2).ToInt();
      if( i < 0 || i >= au.AtomCount() )
        throw TInvalidArgumentException(__OlxSourceInfo, "catom id");
      TCAtom* ca = &au.GetAtom(i);
      if( !IsValidAtom(ca) )  return 0;
      atoms.AddNew( ca );
      return 1;
    }
    else if( Expression.Comparei("last") == 0 )  { 
      if( au.AtomCount() == 0 )  return 0;
      int i=au.AtomCount()-1;
      TCAtom* ca = &au.GetAtom(i);
      while( i > 0 && !IsValidAtom(ca) )  {
        i--;
        ca = &au.GetAtom(i);
      }
      if( !IsValidAtom(ca) )  return 0;
      atoms.AddNew( ca );
      return 1;
    }
    // validate complex expressions with >< chars
    int gs_ind = Expression.IndexOf('>'),
        ls_ind = Expression.IndexOf('<');
    if( gs_ind != -1 || ls_ind != -1 )  {
      TCAtomGroup from, to;
      if( gs_ind != -1 )  {  // it is inverted in shelx ...
        TAtomReference(Expression.SubStringTo(gs_ind).Trim(' '))._Expand(au, from, CurrResi);
        TAtomReference(Expression.SubStringFrom(gs_ind+1).Trim(' '))._Expand(au, to, CurrResi);
      }
      else  {
        TAtomReference(Expression.SubStringTo(ls_ind).Trim(' '))._Expand(au, to, CurrResi);
        TAtomReference(Expression.SubStringFrom(ls_ind+1).Trim(' '))._Expand(au, from, CurrResi);
      }
      if( to.Count() != 1 || from.Count() != 1 )
        throw TFunctionFailedException(__OlxSourceInfo, "failed to expand >/< expression");
      if( from[0].GetAtom()->GetId() >= to[0].GetAtom()->GetId() )
        throw TFunctionFailedException(__OlxSourceInfo, "invalid direction");
      if( from[0].GetMatrix() != to[0].GetMatrix() )
        throw TFunctionFailedException(__OlxSourceInfo, "EQIV must be the same in >/< expresion");
      if( gs_ind != -1 )  {
        for( int i=from[0].GetAtom()->GetId(); i <= to[0].GetAtom()->GetId(); i++ )  {
          TCAtom* ca = &au.GetAtom(i);
          if( !IsValidAtom(ca) )  continue;
          atoms.AddNew( ca, from[0].GetMatrix() );
        }
      }
      else  {
        for( int i=to[0].GetAtom()->GetId(); i >= from[0].GetAtom()->GetId(); i-- )  {
          TCAtom* ca = &au.GetAtom(i);
          if( !IsValidAtom(ca) )  continue;
          atoms.AddNew( ca, from[0].GetMatrix() );
        }
      }
      return atoms.Count() - ac;
    }
    //
    int resi_ind = Expression.IndexOf('_');
    olxstr resi_name = (resi_ind == -1 ? EmptyString : Expression.SubStringFrom(resi_ind+1));
    // check if it is just an equivalent position
    const smatd* eqiv = NULL;
    int eqiv_ind = resi_name.IndexOf('$');
    if( eqiv_ind > 0 )  {  // 0 is for SFAC type
      olxstr str_eqiv( resi_name.SubStringFrom(eqiv_ind+1) );
      if( !str_eqiv.IsNumber() )  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("equivalent id: ") << str_eqiv);
      int eqi = str_eqiv.ToInt()-1;
      if( eqi < 0 || eqi >= au.UsedSymmCount() )  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("equivalent index: ") << str_eqiv);
      eqiv = &au.GetUsedSymm(eqi);
      resi_name = resi_name.SubStringTo(eqiv_ind);
    }
    // validate syntax
    TPtrList<TAsymmUnit::TResidue> residues;
    if( !resi_name.IsEmpty() && (resi_name.CharAt(0) == '+' || resi_name.CharAt(0) == '-') )  {
      if( CurrResi == NULL )  throw TInvalidArgumentException(__OlxSourceInfo, "current residue");
      if( resi_name.CharAt(0) == '+' )  residues.Add(au.NextResidue(*CurrResi));
      else                              residues.Add(au.PrevResidue(*CurrResi));
    }
    else  {
      if( CurrResi != NULL )  residues.Add(CurrResi);
      if( !resi_name.IsEmpty() )  // empty resi name refers to all atom outside RESI
        au.FindResidues(resi_name, residues);  
      if( residues.IsEmpty() )  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("invalid residue class/number: ") << resi_name);
    }
    if( Expression.CharAt(0) == '$' )  {  // sfac type
      olxstr sfac = ((resi_ind == -1) ? Expression.SubStringFrom(1) : Expression.SubString(1, resi_ind-1));
      TBasicAtomInfo* bai = au.GetAtomsInfo()->FindAtomInfoBySymbol(sfac);
      if( bai == NULL )  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("sfac=") << sfac);
      for( int i=0; i < residues.Count(); i++ )  {
        for( int j=0; j < residues[i]->Count(); j++ )  {
          TCAtom* ca = &residues[i]->GetAtom(j);
          if( !ca->IsDeleted() && ca->GetAtomInfo() == *bai )  // cannot use IsValid here, $H woill not work
            atoms.AddNew( ca, eqiv );
        }
      }
    }
    else  {  // just an atom
      olxstr aname = ( (resi_ind == -1) ? Expression : Expression.SubStringTo(resi_ind) );
      for( int i=0; i < residues.Count(); i++ )  {
        if( residues[i] == NULL )  continue;
        for( int j=0; j < residues[i]->Count(); j++ )  {
          TCAtom* ca = &residues[i]->GetAtom(j);
          if( !ca->IsDeleted() && ca->GetLabel().Comparei(aname) == 0 )  {  // must be unique!
            atoms.AddNew( ca, eqiv );
            break;
          }
        }
      }
    }
    return atoms.Count() - ac;
  }
  // prcesses shelx expressions and returns unprocessed expressions (like sel)
  olxstr Expand(TAsymmUnit& au, TCAtomGroup& atoms, const olxstr& DefResi, int& atomAGroup)  {
    olxstr nexp, exp( olxstr::DeleteSequencesOf(Expression, ' ').Trim(' ') );
    nexp.SetCapacity( exp.Length() );
    // remove spaces from arounf >< chars for strtok
    for( int i=0; i < exp.Length(); i++ )  {
      if( (i+1) < exp.Length() && exp.CharAt(i) == ' ' && (exp.CharAt(i+1) == '<' || exp.CharAt(i+1) == '>') )
        continue;
      if( (i > 0) && (exp.CharAt(i-1) == '<' || exp.CharAt(i-1) == '>') && exp.CharAt(i) == ' ')
        continue;
      nexp << exp.CharAt(i);
    }
    atomAGroup = 0;
    TCAtomGroup tmp_atoms;
    TPtrList<TAsymmUnit::TResidue> residues;
    au.FindResidues(DefResi, residues);  // empty resi name refers to all atom outside RESI
    TStrList toks(nexp, ' '), unprocessed;
    for( int i=0; i < residues.Count(); i++ )  {
      bool succeded = true;
      for( int j=0; j < toks.Count(); j++ )  {
        if( TAtomReference(toks[j], SelectionOwner)._Expand(au, tmp_atoms, residues[i]) == 0 )  {
          if( i == 0 )  unprocessed.Add( toks[j] );
          succeded = false;
          break;
        }
      }
      if( succeded )  {
        atoms.AddList(tmp_atoms);
        if( atomAGroup == 0 )
          atomAGroup = tmp_atoms.Count();
      }
      tmp_atoms.Clear();
    }
    return unprocessed.IsEmpty() ? EmptyString : unprocessed.Text(' ');
  }
};


EndXlibNamespace()
#endif
