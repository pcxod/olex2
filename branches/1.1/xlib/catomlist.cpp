#include "catomlist.h"
#include "refmodel.h"

ExplicitCAtomRef* ExplicitCAtomRef::NewInstance(RefinementModel& rm, const olxstr& exp, TResidue* resi)  {
  olxstr aname(exp);
  size_t symm_ind = exp.IndexOf('_');
  const smatd* symm = NULL;
  if( symm_ind != InvalidIndex )  {
    aname = exp.SubStringTo(symm_ind);
    rm.FindUsedSymm(exp.SubStringFrom(symm_ind));
  }
  TCAtom* ca = rm.aunit.FindCAtom(aname, resi); 
  if( ca == NULL )
    return NULL;
  return new ExplicitCAtomRef(*ca, symm);
}
//...................................................................................................
olxstr ExplicitCAtomRef::GetFullLabel(RefinementModel& rm, const TResidue& ref) const  {
  olxstr rv( atom.GetLabel() );
  if( atom.GetResiId() != 0 && atom.GetResiId() != ref.GetId() )  {
    if( matrix != NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "expression invalidates SHELX syntax");
    rv << '_' << rm.aunit.GetResidue(atom.GetResiId()).GetNumber();
  }
  if( matrix != NULL )
    rv << "_$" << (rm.UsedSymmIndex(*matrix)+1);
  return rv;
}
//...................................................................................................
//...................................................................................................
//...................................................................................................
size_t ImplicitCAtomRef::Expand(RefinementModel& rm, TAtomRefList& res, TResidue& resi) const {
  if( Name.Equalsi("last") )  {
    if( res.IsEmpty() )  return 0;
    size_t i = resi.Count()-1;
    while( resi[i].IsDeleted() && --i != InvalidIndex )
      ;
    if( i == InvalidIndex )  return 0;
    res.Add( new ExplicitCAtomRef(resi[i], NULL) );
    return 1;
  }
  if( Name.Equalsi("first") )  {
    size_t i = 0;
    while( resi[i++].IsDeleted() && i <= resi.Count() )
      ;
    if( i == resi.Count() )   return 0;
    res.Add( new ExplicitCAtomRef(resi[i], NULL) );
    return 1;
  }
  if( Name.Equalsi('*') )  {
    size_t ac = 0;
    for( size_t i=0; i < resi.Count(); i++ )  {
      TCAtom& ca = resi[i];
      // skip deleted atoms, q-peaks and H (D)
      if( ca.IsDeleted() || ca.GetType().GetMr() < 3.5 )  continue;
      res.Add( new ExplicitCAtomRef(resi[i], NULL) );
      ac++;
    }
    return ac;
  }
  if( Name.EndsWith("_+") )  {
    TResidue* next_resi = resi.Next();
    if( next_resi != NULL )  {
      IAtomRef* ar = ImplicitCAtomRef::NewInstance(rm, Name.SubStringFrom(0,2), EmptyString(), next_resi);
      if( ar != NULL )  {
        size_t ac = ar->Expand(rm, res, *next_resi);
        delete ar;
        return ac;
      }
    }
    return 0;
  }
  if( Name.EndsWith("_-") )  {
    TResidue* prev_resi = resi.Prev();
    if( prev_resi != NULL )  {
      IAtomRef* ar = ImplicitCAtomRef::NewInstance(rm, Name.SubStringFrom(0,2), EmptyString(), prev_resi);
      if( ar != NULL )  {
        size_t ac = ar->Expand(rm, res, *prev_resi);
        delete ar;
        return ac;
      }
    }
    return 0;
  }
  ResiPList residues;
  const smatd* symm = NULL;
  olxstr aname(Name);
  size_t us_ind = Name.IndexOf('_');
  if( us_ind == InvalidIndex )  // atom name, type
    residues.Add(resi);
  else  {
    if( us_ind+1 == Name.Length() )  // invalid residue/symm reference
      return 0;
    olxstr resi_ref = Name.SubStringFrom(us_ind+1);
    // symmetry reference
    size_t symm_ind = resi_ref.IndexOf('$');
    if( symm_ind != InvalidIndex )  {  
      symm = rm.FindUsedSymm( Name.SubStringFrom(symm_ind) );
      if( symm == NULL )  return 0;
      resi_ref = resi_ref.SubStringTo(symm_ind);
    }
    rm.aunit.FindResidues(resi_ref, residues);
    aname = Name.SubStringTo(us_ind);
  }
  size_t ac = 0;
  if( aname.StartsFrom('$') )  {
    cm_Element* elm = XElementLib::FindBySymbol(aname.SubStringFrom(1));
    if( elm == NULL )  return 0;
    for( size_t i=0; i < residues.Count(); i++ )  {
      for( size_t j=0; j < residues[i]->Count(); j++ )  {
        if( residues[i]->GetAtom(j).IsDeleted() || 
          residues[i]->GetAtom(j).GetType() != *elm )  
        {
          continue;
        }
        res.Add(new ExplicitCAtomRef(residues[i]->GetAtom(j), symm));
        ac++;
      }
    }
  }
  else  {
    for( size_t i=0; i < residues.Count(); i++ )  {
      for( size_t j=0; j < residues[i]->Count(); j++ )  {
        if( !residues[i]->GetAtom(j).IsDeleted() && residues[i]->GetAtom(j).GetLabel().Equalsi(aname) )  {
          res.Add(new ExplicitCAtomRef(residues[i]->GetAtom(j), symm));
          ac++;
          break;  // must be unique to the RESI
        }
      }
    }
  }
  return ac;
}
//...................................................................................................
//...................................................................................................
//...................................................................................................
size_t ListIAtomRef::Expand(RefinementModel& rm, TAtomRefList& res, TResidue& _resi) const  {
  TAtomRefList boundaries;
  if( start.Expand(rm, boundaries, _resi) != 1 )    return 0;
  if( end.Expand(rm, boundaries, _resi) != 1 )    return 0;
  if( boundaries[0].GetMatrix() != boundaries[1].GetMatrix() )
    return 0;
  if( boundaries[0].GetAtom().GetResiId() != boundaries[1].GetAtom().GetResiId() )
    return 0;
  TResidue& resi = (boundaries[0].GetAtom().GetResiId() == _resi.GetId() ? _resi :
    rm.aunit.GetResidue(boundaries[0].GetAtom().GetResiId()) );
  size_t si = resi.IndexOf(boundaries[0].GetAtom());
  size_t ei = resi.IndexOf(boundaries[1].GetAtom());
  if( si == InvalidIndex || ei == InvalidIndex )  // would be odd, since expansion worked...
    return 0;
  if( op == '>' && si <= ei )  {
    size_t ac = 0;
    for( size_t i=si; i <= ei; i++ )  {
      if( resi[i].IsDeleted() || resi[i].GetType().GetMr() < 3.5 )  continue;
      res.Add(new ExplicitCAtomRef(resi[i], boundaries[0].GetMatrix()));
      ac++;
    }
    return ac;
  }
  if( op == '<' && si >= ei )  {
    size_t ac = 0;
    for( size_t i=si; i >= ei; i-- )  {
      if( resi[i].IsDeleted() || resi[i].GetType().GetMr() < 3.5 )  continue;
      res.Add(new ExplicitCAtomRef(resi[i], boundaries[0].GetMatrix()));
      ac++;
      if( i == 0 )  break;
    }
    return ac;
  }
  return 0;
}
//...................................................................................................
//...................................................................................................
//...................................................................................................
AtomRefList::AtomRefList(RefinementModel& _rm, const olxstr& exp, const olxstr& resi) : rm(_rm), residue(resi)  {
  Valid = true;
  ContainsImplicitAtoms = false;
  TStrList toks(exp, ' ');
  for( size_t i=0; i < toks.Count(); i++ )  {
    if( (i+2) < toks.Count() )  {
      if( toks[i+1] == '>' || toks[i+1] == '<' )  {
        IAtomRef* start = ImplicitCAtomRef::NewInstance(rm, toks[i], resi, NULL);
        if( start == NULL )  {
          Valid = false;
          break;
        }
        IAtomRef* end = ImplicitCAtomRef::NewInstance(rm, toks[i+2], resi, NULL);
        if( end == NULL )  {
          delete start;
          Valid = false;
          break;
        }
        refs.Add( new ListIAtomRef(*start, *end, toks[i+1]) );
        i += 2;
        continue;
      }
    }
    IAtomRef* ar = ImplicitCAtomRef::NewInstance(rm, toks[i], resi, NULL);
    if( ar == NULL )  {
      Valid = false;
      break;
    }
    refs.Add(ar);
  }
  if( !Valid )
    refs.Clear();
  for( size_t i=0; i < refs.Count(); i++ )  {
    if( !refs[i].IsExplicit() )  {
      ContainsImplicitAtoms = true;
      break;
    }
  }
}
//...................................................................................................
void AtomRefList::EnsureAtomPairs(RefinementModel& rm, TAtomRefList& al) const {
  for( size_t i=0; i < al.Count(); i++ )  {
    if( !al.IsNull(i) && al[i].GetAtom().IsDeleted() )  {
      size_t ei = i + ((i%2)==0 ? 1 : -1);
      if( al[ei].GetMatrix() != NULL )
        rm.RemUsedSymm( *al[ei].GetMatrix() );
      al.NullItem(ei);
    }
  }
}
//...................................................................................................
void AtomRefList::EnsureAtomTriplets(RefinementModel& rm, TAtomRefList& al) const {
  for( size_t i=0; i < al.Count(); i++ )  {
    if( !al.IsNull(i) && al[i].GetAtom().IsDeleted() )  {
      if( (i%3) == 0 )  {
        if( al[i+1].GetMatrix() != NULL )
          rm.RemUsedSymm( *al[i+1].GetMatrix() );
        al.NullItem(i+2);
        if( al[i+2].GetMatrix() != NULL )
          rm.RemUsedSymm( *al[i+2].GetMatrix() );
        al.NullItem(i+2);
      }
      else if( (i%3) == 1 )  {
        if( al[i-1].GetMatrix() != NULL )
          rm.RemUsedSymm( *al[i-1].GetMatrix() );
        al.NullItem(i+1);
        if( al[i+1].GetMatrix() != NULL )
          rm.RemUsedSymm( *al[i+1].GetMatrix() );
        al.NullItem(i+1);
      }
      else if( (i%3) == 2 )  {
        if( al[i-1].GetMatrix() != NULL )
          rm.RemUsedSymm( *al[i-1].GetMatrix() );
        al.NullItem(i-2);
        if( al[i-2].GetMatrix() != NULL )
          rm.RemUsedSymm( *al[i-2].GetMatrix() );
        al.NullItem(i-2);
      }
    }
  }
}
//...................................................................................................
void AtomRefList::Expand(RefinementModel& rm, TTypeList<TAtomRefList>& c_res, const short list_type) const  {
  if( !Valid )  
    return;
  TPtrList<TResidue> residues;
  rm.aunit.FindResidues(residue, residues);
  for( size_t i=0; i < residues.Count(); i++ )  {
    TAtomRefList& res = c_res.AddNew();
    for( size_t j=0; j < refs.Count(); j++ )
      refs[j].Expand(rm, res, *residues[i]);
    if( list_type == atom_list_type_pairs )
      EnsureAtomPairs(rm, res);
    else if( list_type == atom_list_type_triplets )
      EnsureAtomTriplets(rm, res);
    if( res.IsEmpty() )
      c_res.NullItem(i);
  }
  c_res.Pack();
}
//...................................................................................................
bool AtomRefList::IsExpandable(RefinementModel& rm, const short list_type) const  {
  if( !Valid )  return false;
  TPtrList<TResidue> residues;
  rm.aunit.FindResidues(residue, residues);
  TAtomRefList res;
  size_t ac = 0;
  for( size_t i=0; i < residues.Count(); i++ )  {
    res.Clear();
    for( size_t j=0; j < refs.Count(); j++ )
      refs[j].Expand(rm, res, *residues[i]);
    if( list_type == atom_list_type_pairs )
      EnsureAtomPairs(rm, res);
    else if( list_type == atom_list_type_triplets )
      EnsureAtomTriplets(rm, res);
    ac += res.Count();
  }
  return ac != 0;
}

