/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "catomlist.h"
#include "refmodel.h"
#include "satom.h"
#include "atomref.h"

//.............................................................................
IAtomRef &IAtomRef::FromDataItem(const TDataItem &di, RefinementModel& rm) {
  olxstr t = di.GetFieldByName("type");
  if (t == ExplicitCAtomRef::GetTypeId())
    return *(new ExplicitCAtomRef(di, rm));
  else if (t == ImplicitCAtomRef::GetTypeId())
    return *(new ImplicitCAtomRef(di));
  else if (t == ListIAtomRef::GetTypeId())
    return *(new ListIAtomRef(di, rm));
  throw TInvalidArgumentException(__OlxSourceInfo,
    olxstr("reference type: ").quote() << t);
}
//.............................................................................
//.............................................................................
//.............................................................................
ExplicitCAtomRef::ExplicitCAtomRef(const TDataItem & di, RefinementModel& rm) {
  size_t aid = di.GetFieldByName("atom_id").ToSizeT();
  uint32_t eid = di.GetFieldByName("eqiv_id").ToUInt();
  atom = &rm.aunit.GetAtom(aid);
  DealWithSymm(eid == ~0 ? NULL : &rm.GetUsedSymm(eid));
}
//.............................................................................
ExplicitCAtomRef::~ExplicitCAtomRef() {
  if (matrix != NULL)
    atom->GetParent()->GetRefMod()->RemUsedSymm(*matrix);
}
//.............................................................................
ExplicitCAtomRef* ExplicitCAtomRef::NewInstance(const RefinementModel& rm,
  const olxstr& exp, TResidue* resi)
{
  olxstr aname(exp);
  size_t symm_ind = exp.IndexOf('_');
  const smatd* symm = NULL;
  if (symm_ind != InvalidIndex && (symm_ind+1) < exp.Length())  {
    aname = exp.SubStringTo(symm_ind);
    olxstr sm = exp.SubStringFrom(symm_ind+1);
    if (sm.CharAt(0) != '$') {
      size_t di = sm.IndexOf('$');
      if (di != InvalidIndex) {
        resi = rm.aunit.FindResidue(sm.SubStringTo(di).ToInt());
        sm = sm.SubStringFrom(di);
      }
      else {
        resi = rm.aunit.FindResidue(sm.ToInt());
        sm.SetLength(0);
      }
    }
    if (!sm.IsEmpty())
      symm = rm.FindUsedSymm(sm);
  }
  TCAtom* ca = rm.aunit.FindCAtom(aname, resi);
  if( ca == NULL )
    return NULL;
  return new ExplicitCAtomRef(*ca, symm);
}
//.............................................................................
void ExplicitCAtomRef::DealWithSymm(const smatd* m)  {
  matrix = (m != NULL && !m->IsFirst())
    ? &atom->GetParent()->GetRefMod()->AddUsedSymm(*m) : NULL;
}
//.............................................................................
IAtomRef* ExplicitCAtomRef::Clone(RefinementModel& rm) const {
  TCAtom *a = rm.aunit.FindCAtomById(atom->GetId());
  if (a == NULL) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "asymmetric units do not match");
  }
  const smatd *m = NULL;
  if (matrix != NULL)
    m = &rm.AddUsedSymm(*matrix);
  return new ExplicitCAtomRef(*a, m);
}
//.............................................................................
olxstr ExplicitCAtomRef::GetExpression(TResidue *r) const {
  olxstr rv = atom->GetLabel();
  bool valid_s = true;
  bool use_resi = (atom->GetResiId() !=0 &&
    (r == NULL || r->GetId() != atom->GetResiId()));
  if (use_resi) {
    if (matrix != NULL) valid_s = false;
    rv << '_' << atom->GetParent()->GetResidue(atom->GetResiId()).GetNumber();
  }
  if (matrix != NULL) {
    if (use_resi)
      rv << '$';
    else
      rv << "_$";
    rv << (atom->GetParent()->GetRefMod()->UsedSymmIndex(*matrix)+1);
  }
  if (!valid_s) {
    TBasicApp::NewLogEntry(logInfo) << "Expression will not be read correctly"
      " by SHELX: '" << rv << '\'';
  }
  return rv;
}
//.............................................................................
bool ExplicitCAtomRef::IsValid() const {
  return !atom->IsDeleted();
}
//.............................................................................
void ExplicitCAtomRef::ToDataItem(TDataItem &di) const {
  di.AddField("atom_id", atom->GetTag()).
    AddField("type", GetTypeId()).
    AddField("eqiv_id", matrix == NULL ? uint32_t(~0) : matrix->GetId());
}
//.............................................................................
//.............................................................................
//.............................................................................
ImplicitCAtomRef::ImplicitCAtomRef(const TDataItem &di) {
  Name = di.GetValue();
}
//.............................................................................
size_t ImplicitCAtomRef::Expand(const RefinementModel& rm,
  TAtomRefList& res, TResidue& resi) const
{
  if (Name.Equalsi("last")) {
    if (res.IsEmpty()) return 0;
    size_t i = resi.Count()-1;
    while (resi[i].IsDeleted() && --i != InvalidIndex)
      ;
    if (i == InvalidIndex ) return 0;
    res.Add(new ExplicitCAtomRef(resi[i], NULL));
    return 1;
  }
  if (Name.Equalsi("first")) {
    size_t i = 0;
    while (resi[i++].IsDeleted() && i <= resi.Count())
      ;
    if (i == resi.Count()) return 0;
    res.Add(new ExplicitCAtomRef(resi[i], NULL));
    return 1;
  }
  if (Name.Equalsi('*')) {
    size_t ac = res.Count();
    for (size_t i=0; i < resi.Count(); i++) {
      TCAtom& ca = resi[i];
      // skip deleted atoms, q-peaks and H (D)
      if (ca.IsDeleted() || ca.GetType().z < 2) continue;
      res.Add(new ExplicitCAtomRef(resi[i], NULL));
    }
    return res.Count()-ac;
  }
  if (Name.EndsWith("_+")) {
    TResidue* next_resi = resi.Next();
    if (next_resi != NULL) {
      IAtomRef* ar = ImplicitCAtomRef::NewInstance(rm, Name.SubStringFrom(0,2),
        EmptyString(), next_resi);
      if (ar != NULL) {
        size_t ac = ar->Expand(rm, res, *next_resi);
        delete ar;
        return ac;
      }
    }
    return 0;
  }
  if (Name.EndsWith("_-")) {
    TResidue* prev_resi = resi.Prev();
    if (prev_resi != NULL) {
      IAtomRef* ar = ImplicitCAtomRef::NewInstance(rm, Name.SubStringFrom(0,2),
        EmptyString(), prev_resi);
      if (ar != NULL) {
        size_t ac = ar->Expand(rm, res, *prev_resi);
        delete ar;
        return ac;
      }
    }
    return 0;
  }
  ResiPList residues;
  const smatd* symm = NULL;
  olxstr aname = Name;
  size_t us_ind = Name.IndexOf('_');
  if (us_ind == InvalidIndex)  // atom name, type
    residues.Add(resi);
  else {
    if (us_ind+1 == Name.Length())  // invalid residue/symm reference
      return 0;
    olxstr resi_ref = Name.SubStringFrom(us_ind+1);
    // symmetry reference
    size_t symm_ind = resi_ref.IndexOf('$');
    if (symm_ind != InvalidIndex) {
      symm = rm.FindUsedSymm(resi_ref.SubStringFrom(symm_ind));
      if (symm == NULL) return 0;
      resi_ref = resi_ref.SubStringTo(symm_ind);
    }
    if (resi_ref.IsEmpty())
      residues.Add(resi);
    else
      residues.AddList(rm.aunit.FindResidues(resi_ref));
    aname = Name.SubStringTo(us_ind);
  }
  size_t ac = 0;
  if (aname.StartsFrom('$')) {
    SortedElementPList elms =
      TAtomReference::DecodeTypes(aname.SubStringFrom(1), rm.aunit);
    for (size_t ei = 0; ei < elms.Count(); ei++) {
      for (size_t i = 0; i < residues.Count(); i++) {
        for (size_t j = 0; j < residues[i]->Count(); j++) {
          if (residues[i]->GetAtom(j).IsDeleted() ||
            residues[i]->GetAtom(j).GetType() != *elms[ei])
          {
            continue;
          }
          res.Add(new ExplicitCAtomRef(residues[i]->GetAtom(j), symm));
          ac++;
        }
      }
    }
  }
  else {
    for( size_t i=0; i < residues.Count(); i++ )  {
      for( size_t j=0; j < residues[i]->Count(); j++ )  {
        if( !residues[i]->GetAtom(j).IsDeleted() &&
            residues[i]->GetAtom(j).GetLabel().Equalsi(aname) )
        {
          res.Add(new ExplicitCAtomRef(residues[i]->GetAtom(j), symm));
          ac++;
          break;  // must be unique to the RESI
        }
      }
    }
  }
  return ac;
}
//.............................................................................
IAtomRef* ImplicitCAtomRef::NewInstance(const RefinementModel& rm,
  const olxstr& exp, const olxstr& resi, TResidue* _resi)
{
  // a chance to create explicit reference
  if (resi.IsEmpty() || _resi != NULL) {
    if (!exp.Contains('+') && !exp.Contains('-') &&
        !exp.Contains('*') && !exp.StartsFrom('$'))
    {
      size_t us_ind = exp.IndexOf('_');
      if (us_ind != InvalidIndex) {
        if (us_ind+1 <= exp.Length()) {
          if (exp.CharAt(us_ind+1) != '$') { // is symm reference?
            olxstr ri = exp.SubStringFrom(us_ind+1);
            size_t di = ri.FirstIndexOf('$');
             if (di != InvalidIndex)
               ri = ri.SubStringTo(di);
            if (!ri.IsNumber()) //is explicit?
              return new ImplicitCAtomRef(exp);
          }
        }
      }
      return ExplicitCAtomRef::NewInstance(rm, exp, _resi);
    }
  }
  return new ImplicitCAtomRef(exp);
}
//.............................................................................
void ImplicitCAtomRef::ToDataItem(TDataItem &di) const {
  di.AddField("type", GetTypeId());
  di.SetValue(Name);
}
//.............................................................................
//.............................................................................
//.............................................................................
ListIAtomRef::ListIAtomRef(const TDataItem &di, RefinementModel& rm)
  : start(IAtomRef::FromDataItem(di.GetItemByName("start"), rm)),
    end(IAtomRef::FromDataItem(di.GetItemByName("end"), rm)),
    op(di.GetFieldByName("operation"))
{}
//.............................................................................
size_t ListIAtomRef::Expand(const RefinementModel& rm, TAtomRefList& res,
  TResidue& _resi) const
{
  TAtomRefList boundaries;
  if( start.Expand(rm, boundaries, _resi) != 1 )    return 0;
  if( end.Expand(rm, boundaries, _resi) != 1 )    return 0;
  if( boundaries[0].GetMatrix() != boundaries[1].GetMatrix() )
    return 0;
  if( boundaries[0].GetAtom().GetResiId() != boundaries[1].GetAtom().GetResiId() )
    return 0;
  TResidue& resi = (boundaries[0].GetAtom().GetResiId() == _resi.GetId()
    ? _resi : rm.aunit.GetResidue(boundaries[0].GetAtom().GetResiId()) );
  size_t si = resi.IndexOf(boundaries[0].GetAtom());
  size_t ei = resi.IndexOf(boundaries[1].GetAtom());
  // would be odd, since expansion worked...
  if( si == InvalidIndex || ei == InvalidIndex )
    return 0;
  if( op == '>' && si <= ei )  {
    size_t ac = 0;
    for( size_t i=si; i <= ei; i++ )  {
      if( resi[i].IsDeleted() || resi[i].GetType().z < 2 )  continue;
      res.Add(new ExplicitCAtomRef(resi[i], boundaries[0].GetMatrix()));
      ac++;
    }
    return ac;
  }
  if( op == '<' && si >= ei )  {
    size_t ac = 0;
    for( size_t i=si; i >= ei; i-- )  {
      if( resi[i].IsDeleted() || resi[i].GetType().z < 2 )  continue;
      res.Add(new ExplicitCAtomRef(resi[i], boundaries[0].GetMatrix()));
      ac++;
      if( i == 0 )  break;
    }
    return ac;
  }
  return 0;
}
//.............................................................................
void ListIAtomRef::ToDataItem(TDataItem &di) const {
  di.AddField("type", GetTypeId());
  di.AddField("operation", op);
  start.ToDataItem(di.AddItem("start"));
  end.ToDataItem(di.AddItem("end"));
}
//.............................................................................
//.............................................................................
//.............................................................................
AtomRefList::AtomRefList(RefinementModel& _rm, const olxstr& exp,
  const olxstr& resi) : rm(_rm), residue(resi)
{
  Build(exp, resi);
}
//.............................................................................
void AtomRefList::Build(const olxstr& exp, const olxstr& resi_) {
  Clear();
  olxstr r_c;
  TResidue *r_r = NULL;
  if (resi_.IsNumber())
    r_r = rm.aunit.FindResidue(resi_.ToInt());
  else
    r_c = resi_;
  expression = exp;
  residue = resi_;
  Valid = true;
  ContainsImplicitAtoms = false;
  TStrList toks(exp, ' ');
  for( size_t i=0; i < toks.Count(); i++ )  {
    if (!Valid) break;
    if( (i+2) < toks.Count() )  {
      if( toks[i+1] == '>' || toks[i+1] == '<' )  {
        IAtomRef* start = ImplicitCAtomRef::NewInstance(rm, toks[i], r_c, r_r);
        if( start == NULL )  {
          Valid = false;
          break;
        }
        IAtomRef* end = ImplicitCAtomRef::NewInstance(rm, toks[i+2], r_c, r_r);
        if( end == NULL )  {
          delete start;
          Valid = false;
          break;
        }
        refs.Add(new ListIAtomRef(*start, *end, toks[i+1]));
        i += 2;
        continue;
      }
    }
    IAtomRef* ar = ImplicitCAtomRef::NewInstance(rm, toks[i], r_c, r_r);
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
  UpdateResi();
}
//.............................................................................
void AtomRefList::EnsureAtomGroups(const RefinementModel& rm,
  TAtomRefList& al, size_t group_size) const
{
  for (size_t i=0; i < al.Count(); i += group_size)  {
    bool valid = true;
    for (size_t j=i; j < group_size; j++) {
      if (j >= al.Count() || al.IsNull(j) || al[j].GetAtom().IsDeleted()) {
        valid = false;
        break;
      }
    }
    if (!valid)  {
      for( size_t j=i; j < group_size; j++) {
        if( j >= al.Count() )  break;
        if( al.IsNull(j) )  continue;
        if( al[j].GetMatrix() != NULL )
          rm.RemUsedSymm(*al[j].GetMatrix());
        al.NullItem(j);
      }
    }
  }
  al.Pack();
}
//.............................................................................
TTypeList<TAtomRefList> &AtomRefList::Expand(const RefinementModel& rm,
  TTypeList<TAtomRefList>& c_res, size_t group_size) const
{
  if( !Valid )  return c_res;
  TPtrList<TResidue> residues = rm.aunit.FindResidues(residue);
  for( size_t i=0; i < residues.Count(); i++ )  {
    TAtomRefList& res = c_res.AddNew();
    for( size_t j=0; j < refs.Count(); j++ )
      refs[j].Expand(rm, res, *residues[i]);
    if (group_size != InvalidSize)
      EnsureAtomGroups(rm, res, group_size);
    if( res.IsEmpty() )
      c_res.NullItem(i);
  }
  c_res.Pack();
  return c_res;
}
//.............................................................................
ConstTypeList<ExplicitCAtomRef> AtomRefList::ExpandList(
  const RefinementModel& rm, size_t group_size) const
{
  TTypeList<ExplicitCAtomRef> rv;
  TPtrList<TResidue> residues = rm.aunit.FindResidues(residue);
  for( size_t i=0; i < residues.Count(); i++ )  {
    TAtomRefList l;
    for( size_t j=0; j < refs.Count(); j++ )
      refs[j].Expand(rm, l, *residues[i]);
    if (group_size != InvalidSize)
      EnsureAtomGroups(rm, l, group_size);
    rv.SetCapacity(rv.Count()+l.Count());
    for ( size_t j=0; j < l.Count(); j++)
      rv.Add(l[j]);
    l.ReleaseAll();
  }
  return rv;
}
//.............................................................................
bool AtomRefList::IsExpandable(const RefinementModel& rm,
  size_t group_size) const
{
  if( !Valid )  return false;
  TPtrList<TResidue> residues = rm.aunit.FindResidues(residue);
  size_t ac = 0;
  for( size_t i=0; i < residues.Count(); i++ )  {
    TAtomRefList res;
    for( size_t j=0; j < refs.Count(); j++ )
      refs[j].Expand(rm, res, *residues[i]);
    if (group_size != InvalidSize)
      EnsureAtomGroups(rm, res, group_size);
    ac += res.Count();
  }
  return ac != 0;
}
//.............................................................................
void AtomRefList::Assign(const AtomRefList &arl) {
  ContainsImplicitAtoms = arl.ContainsImplicitAtoms;
  expression = arl.expression;
  residue = arl.residue;
  Valid = arl.Valid;
  refs.Clear();
  refs.SetCapacity(arl.refs.Count());
  for (size_t i=0; i < arl.refs.Count(); i++) {
    IAtomRef *ar = arl.refs[i].Clone(rm);
    if (ar != NULL)
      refs.Add(ar);
  }
}
//.............................................................................
void AtomRefList::EnsureAtomGroups(size_t group_size) {
  for (size_t i=0; i < refs.Count(); i += group_size)  {
    bool valid = true;
    for (size_t j=i; j < i+group_size; j++) {
      if (j >= refs.Count() || refs.IsNull(j) || !refs[j].IsValid()) {
        valid = false;
        break;
      }
    }
    if (!valid)  {
      for( size_t j=i; j < i+group_size; j++) {
        if( j >= refs.Count() )  break;
        if( refs.IsNull(j) )  continue;
        refs.NullItem(j);
      }
    }
  }
  refs.Pack();
}
//.............................................................................
AtomRefList &AtomRefList::Validate(size_t group_size) {
  if (group_size != InvalidIndex && IsExplicit()) {
    EnsureAtomGroups(group_size);
  }
  for (size_t i=0; i < refs.Count(); i++) {
    if (!refs[i].IsValid())
      refs.NullItem(i);
  }
  refs.Pack();
  return *this;
}
//.............................................................................
void AtomRefList::ToDataItem(TDataItem &di) const {
  di.AddField("residue", residue).
    AddField("expression", expression).
    AddField("has_implicit", ContainsImplicitAtoms);
  size_t cnt=0;
  for (size_t i=0; i < refs.Count(); i++) {
    if (refs[i].IsValid())
      refs[i].ToDataItem(di.AddItem("item"));
  }

}
//.............................................................................
void AtomRefList::FromDataItem(const TDataItem &di) {
  residue = di.GetFieldByName("residue");
  expression = di.GetFieldByName("expression");
  ContainsImplicitAtoms = di.GetFieldByName("has_implicit").ToBool();
  refs.Clear();
  refs.SetCapacity(di.ItemCount());
  for (size_t i=0; i < di.ItemCount(); i++)
    refs.Add(IAtomRef::FromDataItem(di.GetItemByIndex(i), rm));
}
//.............................................................................
void AtomRefList::UpdateResi() {
  if (!IsExplicit()) return;
  uint32_t r_id = 0;
  for (size_t i=0; i < refs.Count(); i++) {
    ExplicitCAtomRef * r = dynamic_cast<ExplicitCAtomRef *>(&refs[i]);
    if (r == NULL) return;
    uint32_t ri = r->GetAtom().GetResiId();
    if (r_id != 0 && ri != 0 && ri != r_id) {
      if (refs.Count() == 2) {  // special case, 'easy' to handle
        ExplicitCAtomRef * r0 = dynamic_cast<ExplicitCAtomRef *>(&refs[0]);
        if (r->GetMatrix() != NULL && r0->GetMatrix() != NULL)
          return;
        if (r->GetMatrix() == NULL && r0->GetMatrix() == NULL)
          return;
        if (r->GetMatrix() != NULL)
          r_id = r->GetAtom().GetResiId();
        else if (r0->GetMatrix() != NULL)
          r_id = r0->GetAtom().GetResiId();
        break;
      }
      return;
    }
    r_id = ri;
  }
  if (r_id != 0) {
    residue = this->rm.aunit.GetResidue(r_id).GetNumber();
  }
}
//.............................................................................
olxstr AtomRefList::GetExpression() const {
  if (!Valid) return expression;
  if (residue.IsNumber())
    return BuildExpression(rm.aunit.FindResidue(residue.ToInt()));
  return BuildExpression(NULL);
}
//.............................................................................
void AtomRefList::AddExplicit(class TSAtom &a) {
  refs.Add(new ExplicitCAtomRef(a.CAtom(),
    a.GetMatrix().IsFirst() ? NULL : &a.GetMatrix()));
}
//.............................................................................
