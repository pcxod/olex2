#include "selected.h"
#include "cif.h"
#include "estrbuffer.h"

//.............................................................................
//.............................................................................
//.............................................................................
IAtomMask *IAtomMask::build(const olxstr &mask, const TAsymmUnit &au) {
  if (mask.StartsFrom('$'))
    return new AtomTypeMask(mask.SubStringFrom(1), au);
  TCAtom *a = au.FindCAtom(mask);
  if (a == NULL) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("atom name ").quote() << mask);
  }
  return new AtomNameMask(*a);
}
//.............................................................................
//.............................................................................
//.............................................................................
AtomNameMask::AtomNameMask(const AtomNameMask &m, const TAsymmUnit &au)
: atom(au.FindCAtomById(m.atom->GetId()))
{
  if (atom == 0) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "aysmmetric units missmatch");
  }
}
//.............................................................................
bool AtomNameMask::matches(const TCAtom &a) const {
  return atom->GetLabel() == a.GetLabel();
}
//.............................................................................
bool AtomNameMask::equals(const IAtomMask &m_) const {
  const AtomNameMask *m = dynamic_cast<const AtomNameMask *>(&m_);
  if (m == 0) return false;
  return atom == m->atom;
}
//.............................................................................
//.............................................................................
//.............................................................................
AtomTypeMask::AtomTypeMask(const olxstr &exp, const TAsymmUnit &) {
  TStrList toks(exp, ',');
  if (toks[0] == '*') {
    Not = true;
    for (size_t i = 1; i < toks.Count(); i++) {
      olxstr en = toks[i].StartsFrom('-') ? toks[i].SubStringFrom(1) : toks[i];
      if (en.Equalsi('M')) {
        types.AddUnique(AtomType(-2, true));
      }
      else if (en.Equalsi('X')) {
        types.AddUnique(AtomType(-3, true));
      }
      cm_Element *elm = XElementLib::FindBySymbol(en);
      if (elm == NULL) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("element ").quote() << en);
      }
      types.AddUnique(AtomType(*elm, true));
    }
  }
  else {
    Not = false;
    for (size_t i = 0; i < toks.Count(); i++) {
      bool excl = toks[i].StartsFrom('-');
      olxstr en = excl ? toks[i].SubStringFrom(1) : toks[i];
      if (en.Equalsi('M')) {
        types.AddUnique(AtomType(-2, excl));
      }
      else if (en.Equalsi('X')) {
        types.AddUnique(AtomType(-3, excl));
      }
      else {
        cm_Element *elm = XElementLib::FindBySymbol(en);
        if (elm == NULL) {
          throw TInvalidArgumentException(__OlxSourceInfo,
            olxstr("element ").quote() << en);
        }
        types.AddUnique(AtomType(*elm, excl));
      }
    }
  }
}
//.............................................................................
bool AtomTypeMask::matches(const TCAtom &a) const {
  size_t idx = types.IndexOf(AtomType(a.GetType(), false));
  if (idx != InvalidIndex) {
    return !types[idx].Not;
  }
  return Not;
}
//.............................................................................
bool AtomTypeMask::equals(const IAtomMask &m_) const {
  const AtomTypeMask *m = dynamic_cast<const AtomTypeMask *>(&m_);
  if (m == 0 || types.Count() != m->types.Count())
    return false;
  for (size_t i = 0; i < types.Count(); i++) {
    if (types[i].Compare(m->types[i]) != 0) // pointer comparator!
      return false;
  }
  return true;
}
//.............................................................................
olxstr AtomTypeMask::toString() const {
  if (types.IsEmpty() && Not)
    return "$*";
  olxstr rv = olxstr(',').Join(types,
    FunctionAccessor::MakeConst(&AtomType::toString));
  return Not ? rv.Insert("$*,", 0) : rv.Insert('$', 0);
}
//.............................................................................
//.............................................................................
//.............................................................................
bool SelectedTableRows::AddRow(TTypeList<TTypeList<IAtomMask> > &where,
  const olxstr &row_, const TAsymmUnit &au)
{
  TStrList toks(row_, ' ');
  TTypeList<IAtomMask> &row = where.AddNew();
  for (size_t i = 0; i < toks.Count(); i++) {
    row.Add(IAtomMask::build(toks[i], au));
  }
  for (size_t i = 0; i < where.Count() - 1; i++) {
    if (where[i].Count() != row.Count()) continue;
    bool equals = true;
    for (size_t j = 0; j < row.Count(); j++) {
      if (!row[j].equals(where[i][j])) {
        equals = false;
        break;
      }
    }
    if (equals) {
      where.Delete(where.Count() - 1);
      return false;
    }
  }
  return true;
}
//.............................................................................
void SelectedTableRows::ToDataItem(const TTypeList<SelectedTableRows::row_t> &what,
  TDataItem &di) const
{
  for (size_t i = 0; i < what.Count(); i++) {
    bool valid = true;
    for (size_t j = 0; j < what[i].Count(); j++) {
      if (!what[i][j].isValid()) {
        valid = false;
        break;
      }
    }
    if (!valid) continue;
    di.AddItem("row", olxstr(' ').Join(what[i],
      FunctionAccessor::MakeConst(&IAtomMask::toString)));
  }
}
//.............................................................................
void SelectedTableRows::ToDataItem(TDataItem &di) const {
  TDataItem *i;
  ToDataItem(bonds, *(i = &di.AddItem("bonds")));
  if (i->ItemCount() == 0)
    di.DeleteItem(i);
  ToDataItem(angles, *(i = &di.AddItem("angles")));
  if (i->ItemCount() == 0)
    di.DeleteItem(i);
  ToDataItem(dihedrals, *(i = &di.AddItem("dihedrals")));
  if (i->ItemCount() == 0)
    di.DeleteItem(i);
}
//.............................................................................
void SelectedTableRows::FromDataItem(TTypeList<row_t> &what,
  const TDataItem &di, const TAsymmUnit &au)
{
  what.Clear();
  for (size_t i = 0; i < di.ItemCount(); i++) {
    TDataItem &r = di.GetItemByIndex(i);
    row_t &row = what.AddNew();
    TStrList toks(r.GetValue(), ' ');
    for (size_t j = 0; j < toks.Count(); j++) {
      row.Add(IAtomMask::build(toks[j], au));
    }
  }
}
//.............................................................................
void SelectedTableRows::Clear() {
  bonds.Clear();
  angles.Clear();
  dihedrals.Clear();
}
//.............................................................................
void SelectedTableRows::FromDataItem(const TDataItem &di,
  const TAsymmUnit &au)
{
  TDataItem *i;
  if ((i = di.FindItem("bonds")) !=0 )
    FromDataItem(bonds, *i, au);
  if ((i = di.FindItem("bonds")) !=0 )
  if ((i = di.FindItem("angles")) != 0)
    FromDataItem(angles, *i, au);
  if ((i = di.FindItem("dihedrals")) != 0)
    FromDataItem(dihedrals, *i, au);
}
//.............................................................................
void SelectedTableRows::Assign(TTypeList<SelectedTableRows::row_t> &dest,
  const TTypeList<SelectedTableRows::row_t> &src,
  const TAsymmUnit &au)
{
  dest.Clear();
  for (size_t i = 0; i < src.Count(); i++) {
    row_t &r = dest.AddNew();
    for (size_t j = 0; j < src[i].Count(); j++) {
      r.Add(src[i][j].replicate(au));
    }
  }
}
//.............................................................................
void SelectedTableRows::Assign(const SelectedTableRows &str,
  const TAsymmUnit& au)
{
  Assign(bonds, str.bonds, au);
  Assign(angles, str.angles, au);
  Assign(dihedrals, str.dihedrals, au);
}
//.............................................................................
void SelectedTableRows::ProcessRows(TTypeList<row_t> &dest,
  const TStrList &defs, const TAsymmUnit &au)
{
  for (size_t i = 0; i < defs.Count(); i++)
    AddRow(dest, defs[i], au);
}
//.............................................................................
bool SelectedTableRows::MatchRows(const TCAtomPList &row,
  TTypeList<row_t> &where) const
{
  for (size_t i = 0; i < where.Count(); i++) {
    row_t &mr = where[i];
    if (mr.Count() > row.Count()) continue;
    // match any?
    if (mr.Count() == 1) {
      for (size_t j = 0; j < row.Count(); j++) {
        if (mr[0].matches(*row[j]))
          return true;
      }
      continue;
    }
    size_t mm = olx_min(mr.Count(), row.Count());
    bool match = true;
    // left tor right
    for (size_t j = 0; j < mm; j++) {
      if (!mr[j].matches(*row[j])) {
        match = false;
        break;
      }
    }
    if (match) return true;
    match = true;
    // left tor right
    for (size_t j = 0; j < mm; j++) {
      if (!mr[j].matches(*row[row.Count()-j-1])) {
        match = false;
        break;
      }
    }
    if (match) return true;
  }
  return false;
}
//.............................................................................
void SelectedTableRows::Process(TCif &cif) {
  using namespace cif_dp;
  TAsymmUnit &au = cif.GetAsymmUnit();
  cetTable *tab = cif.FindLoop("_geom_bond");
  if (tab != NULL) {
    const size_t i1 = tab->ColIndex("_geom_bond_atom_site_label_1");
    const size_t i2 = tab->ColIndex("_geom_bond_atom_site_label_2");
    if (i1 != InvalidIndex && i2 != InvalidIndex) {
      size_t pfi = tab->ColIndex("_geom_bond_publ_flag");
      if (pfi == InvalidIndex) {
        tab->AddCol("_geom_bond_publ_flag");
        pfi = tab->ColCount() - 1;
        for (size_t tr = 0; tr < tab->RowCount(); tr++) {
          tab->Set(tr, pfi, new cetString('?'));
        }
      }
      for (size_t tr = 0; tr < tab->RowCount(); tr++) {
        const CifRow& row = (*tab)[tr];
        TCAtom *a1 = au.FindCAtomDirect(row[i1]->GetStringValue());
        TCAtom *a2 = au.FindCAtomDirect(row[i2]->GetStringValue());
        if (a1 == 0 || a2 == 0) continue;
        if (MatchRows(TCAtomPList() << a1 << a2, bonds)) {
          tab->Set(tr, pfi, new cetString('y'));
        }
      }
    }
  }
  tab = cif.FindLoop("_geom_angle");
  if (tab != NULL) {
    const size_t i1 = tab->ColIndex("_geom_angle_atom_site_label_1");
    const size_t i2 = tab->ColIndex("_geom_angle_atom_site_label_2");
    const size_t i3 = tab->ColIndex("_geom_angle_atom_site_label_3");
    if (i1 != InvalidIndex && i2 != InvalidIndex && i3 != InvalidIndex) {
      size_t pfi = tab->ColIndex("_geom_angle_publ_flag");
      if (pfi == InvalidIndex) {
        tab->AddCol("_geom_angle_publ_flag");
        pfi = tab->ColCount() - 1;
        for (size_t tr = 0; tr < tab->RowCount(); tr++) {
          tab->Set(tr, pfi, new cetString('?'));
        }
      }
      for (size_t tr = 0; tr < tab->RowCount(); tr++) {
        const CifRow& row = (*tab)[tr];
        TCAtom *a1 = au.FindCAtomDirect(row[i1]->GetStringValue());
        TCAtom *a2 = au.FindCAtomDirect(row[i2]->GetStringValue());
        TCAtom *a3 = au.FindCAtomDirect(row[i3]->GetStringValue());
        if (a1 == 0 || a2 == 0 || a3 == 0) continue;
        if (MatchRows(TCAtomPList() << a1 << a2 << a3, angles)) {
          tab->Set(tr, pfi, new cetString('y'));
        }
      }
    }
  }
  tab = cif.FindLoop("_geom_torsion");
  if (tab != NULL) {
    const size_t i1 = tab->ColIndex("_geom_torsion_atom_site_label_1");
    const size_t i2 = tab->ColIndex("_geom_torsion_atom_site_label_2");
    const size_t i3 = tab->ColIndex("_geom_torsion_atom_site_label_3");
    const size_t i4 = tab->ColIndex("_geom_torsion_atom_site_label_4");
    if ((i1|i2|i3|i4) != InvalidIndex) {
      size_t pfi = tab->ColIndex("_geom_torsion_publ_flag");
      if (pfi == InvalidIndex) {
        tab->AddCol("_geom_torsion_publ_flag");
        pfi = tab->ColCount() - 1;
        for (size_t tr = 0; tr < tab->RowCount(); tr++) {
          tab->Set(tr, pfi, new cetString('?'));
        }
      }
      for (size_t tr = 0; tr < tab->RowCount(); tr++) {
        const CifRow& row = (*tab)[tr];
        TCAtom *a1 = au.FindCAtomDirect(row[i1]->GetStringValue());
        TCAtom *a2 = au.FindCAtomDirect(row[i2]->GetStringValue());
        TCAtom *a3 = au.FindCAtomDirect(row[i3]->GetStringValue());
        TCAtom *a4 = au.FindCAtomDirect(row[i4]->GetStringValue());
        if (a1 == 0 || a2 == 0 || a3 == 0 || a4 == 0) continue;
        if (MatchRows(TCAtomPList() << a1 << a2 << a3 << a4, dihedrals)) {
          tab->Set(tr, pfi, new cetString('y'));
        }
      }
    }
  }
}
//.............................................................................
