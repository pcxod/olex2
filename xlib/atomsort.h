/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_atom_sort_H
#define __olx_atom_sort_H
#include "lattice.h"
#include "edict.h"

const short
  atom_sort_None      = 0x0000,
  atom_sort_Mw        = 0x0001,
  atom_sort_Label     = 0x0002,  // clever sort C1a1
  atom_sort_Label1    = 0x0004, // string comparison sort
  atom_sort_MoietySize = 0x0008,  // fgrament size
  atom_sort_KeepH     = 0x0010,  // keeps H atoms after the pivoting one
  moiety_sort_Create   = 0x0020,
  moiety_sort_Size     = 0x0040,
  moiety_sort_Weight   = 0x0080,
  moiety_sort_Heaviest = 0x0100;  // sorts moieties by heaviest element of the moiety

class AtomSorter {
public:
  struct Sorter {
    int(*func)(const TCAtom &, const TCAtom &);
    olxdict<const cm_Element *, int, TPointerComparator> type_exc;
    bool reverse;

    Sorter(int(*f)(const TCAtom &, const TCAtom &), bool reverse = false)
      : func(f),
      reverse(reverse)
    {}
    void AddExceptions(const TStrList &e) {
      for (size_t i = 0; i < e.Count(); i++) {
        if (e[i].StartsFrom('$')) {
          cm_Element *elm = XElementLib::FindBySymbol(e[i].SubStringFrom(1));
          if (elm == 0) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              olxstr("element ").quote() << e[i].SubStringFrom(1));
          }
          type_exc.Add(elm, (int)type_exc.Count());
        }
      }
    }
    int sort(const TCAtom &a, const TCAtom &b) const {
      if (type_exc.HasKey(&a.GetType()) && type_exc.HasKey(&b.GetType())) {
        return olx_cmp(type_exc.Get(&a.GetType()), type_exc.Get(&b.GetType()));
      }
      return reverse ? -(*func)(a, b) : (*func)(a, b);
    }
  };

  static int atom_cmp_Part(const TCAtom &a1, const TCAtom &a2) {
    if (a2.GetPart() < 0 && a1.GetPart() >= 0) {
      return -1;
    }
    if (a2.GetPart() >= 0 && a1.GetPart() < 0) {
      return 1;
    }
    return a1.GetPart() - a2.GetPart();  // smallest goes first
  }
  static int atom_cmp_Z(const TCAtom &a1, const TCAtom &a2) {
    return olx_cmp(a2.GetType().z, a1.GetType().z);
  }
  static int atom_cmp_Mw(const TCAtom &a1, const TCAtom &a2) {
    return olx_cmp(a2.GetType().GetMr(), a1.GetType().GetMr());
  }
  static int atom_cmp_Label(const TCAtom &a1, const TCAtom &a2) {
    return TCAtom::CompareAtomLabels(a1.GetLabel(), a2.GetLabel());
  }
  static int atom_cmp_Suffix(const TCAtom &a1, const TCAtom &a2) {
    olxstr sa, sb;
    size_t i = a1.GetType().symbol.Length();
    while (++i < a1.GetLabel().Length() && olxstr::o_isdigit(a1.GetLabel()[i]))
    {
    }
    if (i < a1.GetLabel().Length()) {
      sa = a1.GetLabel().SubStringFrom(i);
    }
    i = a2.GetType().symbol.Length();
    while (++i < a2.GetLabel().Length() && olxstr::o_isdigit(a2.GetLabel()[i]))
    {
    }
    if (i < a2.GetLabel().Length()) {
      sb = a2.GetLabel().SubStringFrom(i);
    }
    return olxstrComparator<false>().Compare(sa, sb);
  }
  static int atom_cmp_Number(const TCAtom &a1, const TCAtom &a2) {
    olxstr sa, sb;
    for (size_t i = a1.GetType().symbol.Length(); i < a1.GetLabel().Length(); i++) {
      if (olxstr::o_isdigit(a1.GetLabel().CharAt(i))) {
        sa << a1.GetLabel().CharAt(i);
      }
      else {
        break;
      }
    }
    for (size_t i = a2.GetType().symbol.Length(); i < a2.GetLabel().Length(); i++) {
      if (olxstr::o_isdigit(a2.GetLabel().CharAt(i))) {
        sb << a2.GetLabel().CharAt(i);
      }
      else {
        break;
      }
    }
    if (sa.IsEmpty()) {
      return (sb.IsEmpty() ? 0 : -1);
    }
    return (sb.IsEmpty() ? 1 : olx_cmp(sa.ToInt(), sb.ToInt()));
  }
  static int atom_cmp_Id(const TCAtom &a1, const TCAtom &a2) {
    return olx_cmp(a1.GetId(), a2.GetId());
  }
  static int atom_cmp_Label1(const TCAtom &a1, const TCAtom &a2) {
    return a1.GetLabel().Comparei(a2.GetLabel());
  }
  static int atom_cmp_MoietySize(const TCAtom &a1, const TCAtom &a2) {
    return a1.GetFragmentId() - a2.GetFragmentId();
  }

  struct CombiSort {
    TTypeList<Sorter> sequence;
    int atom_cmp(const TCAtom &a1, const TCAtom &a2) const {
      for (size_t i = 0; i < sequence.Count(); i++) {
        int res = sequence[i].sort(a1, a2);
        if (res != 0) {
          return res;
        }
      }
      return 0;
    }
  };

  static void KeepH(TCAtomPList& list, const TAsymmUnit& au,
    int(*sort_func)(const TCAtom &, const TCAtom &))
  {
    typedef olx_pair_t<TCAtom*, TCAtomPList> tree_node;
    TTypeList<tree_node> atom_tree;
    au.GetAtoms().ForEach(ACollectionItem::TagSetter(-1));
    list.ForEach(ACollectionItem::TagSetter(0));
    for (size_t i = 0; i < list.Count(); i++) {
      if (list[i]->GetType() == iHydrogenZ || list[i]->IsDeleted()) {
        continue;
      }
      TCAtomPList& ca_list = atom_tree.Add(new tree_node(list[i])).b;
      list[i]->SetTag(1);
      for (size_t j = 0; j < list[i]->AttachedSiteCount(); j++) {
        TCAtom &a = list[i]->GetAttachedAtom(j);
        if (a.GetTag() == 0 && a.GetType() == iHydrogenZ) {
          ca_list.Add(a)->SetTag(1);
        }
      }
      if (ca_list.Count() > 1 && sort_func != 0) {
        QuickSorter::SortSF(ca_list, sort_func);
      }
    }
    TCAtomPList left = list.Filter(ACollectionItem::TagAnalyser(0));
    size_t atom_count = 0;
    for (size_t i = 0; i < atom_tree.Count(); i++) {
      list[atom_count++] = atom_tree[i].a;
      for (size_t j = 0; j < atom_tree[i].GetB().Count(); j++) {
        list[atom_count++] = atom_tree[i].GetB()[j];
      }
    }
    for (size_t i = 0; i < left.Count(); i++) {
      list[atom_count++] = left[i];
    }
  }

  /* Sorts the list according to the atom_names. The atoms named by the
  atom_names will be following each other in the specified order and will be
  postioned at the location of the first found of the atom_names if
  insert_at_first_name is true and at the smallest index in the list
  */
  static void SortByName(TCAtomPList& list, const TStrList& atom_names,
    bool insert_at_first_name)
  {
    if (atom_names.Count() < 2) {
      return;
    }
    TCAtomPList sorted;
    size_t fi = InvalidIndex;
    for (size_t i = 0; i < atom_names.Count(); i++) {
      for (size_t j = 0; j < list.Count(); j++) {
        if (list[j] == 0) {
          continue;
        }
        if (list[j]->GetLabel().Equalsi(atom_names[i])) {
          if (insert_at_first_name) {
            if (fi == InvalidIndex) {
              fi = j;
            }
          }
          else if (j < fi) {
            fi = j;
          }
          sorted.Add(list[j]);
          list[j] = 0;
          break;
        }
      }
    }
    if (fi == InvalidIndex) {
      return;
    }
    size_t ins_pos = 0;
    for (size_t i = 0; i < fi; i++) {
      if (list[i] != 0) {
        ins_pos++;
      }
    }
    list.Pack();
    list.Insert(ins_pos, sorted);
  }

  /* Reorders the atom_names in the given order (the positions of the other
  atoms stay the same.
  */
  static void ReorderByName(TCAtomPList& list, const TStrList& atom_names) {
    if (atom_names.Count() < 2) return;
    TSizeList indices(list.Count(), olx_list_init::index());
    TPtrList<TCAtom> atoms;
    TSizeList found;
    for (size_t i = 0; i < atom_names.Count(); i++) {
      size_t pos = InvalidIndex;
      for (size_t j = 0; j < list.Count(); j++) {
        if (list[j]->GetLabel().Equalsi(atom_names[i])) {
          found.Add(j);
          atoms.Add(list[j]);
          break;
        }
      }
    }
    QuickSorter::Sort(found, TPrimitiveComparator());
    for (size_t i = 0; i < found.Count(); i++) {
      list[found[i]] = atoms[i];
    }
  }

  static void Sort(TCAtomPList& list,
    int(*sort_func)(const TCAtom&, const TCAtom&))
  {
    QuickSorter::SortSF(list, sort_func);
  }
  static void Sort(TCAtomPList& list, AtomSorter::CombiSort& cs) {
    QuickSorter::SortMF(list, cs, &AtomSorter::CombiSort::atom_cmp);
  }
};

class MoietySorter {
public:
  typedef TCAtomPList moiety_t;
  static TTypeList<moiety_t>::const_list_type SplitIntoMoieties(
    TCAtomPList& list)
  {
    TTypeList<moiety_t> moieties;
    olx_pdict<uint32_t, moiety_t *> cache;
    for (size_t i = 0; i < list.Count(); i++) {
      moiety_t* ca_list = cache.Find(list[i]->GetFragmentId(), 0);
      if (ca_list == 0) {
        cache.Add(list[i]->GetFragmentId(),
          ca_list = &moieties.Add(new moiety_t()));
      }
      ca_list->Add(list[i]);
    }
    return moieties;
  }
  static int moiety_cmp_size(const moiety_t &a, const moiety_t &b) {
    return olx_cmp(b.Count(), a.Count());
  }
  static int moiety_cmp_label(const moiety_t &a, const moiety_t &b) {
    return TCAtom::CompareAtomLabels(a[0]->GetLabel(), b[0]->GetLabel());
  }
  static int moiety_cmp_mass(const moiety_t &a, const moiety_t &b) {
    double m1 = 0, m2 = 0;
    for (size_t i = 0; i < a.Count(); i++) {
      m1 += a[i]->GetType().GetMr();
    }
    for (size_t i = 0; i < b.Count(); i++) {
      m2 += b[i]->GetType().GetMr();
    }
    return olx_cmp(m2, m1);
  }
  static int moiety_cmp_heaviest(const moiety_t &a, const moiety_t &b) {
    double m1 = 0, m2 = 0;
    for (size_t i = 0; i < a.Count(); i++) {
      if (a[i]->GetType().GetMr() > m1) {
        m1 = a[i]->GetType().GetMr();
      }
    }
    for (size_t i = 0; i < b.Count(); i++) {
      if (b[i]->GetType().GetMr() > m2) {
        m2 = b[i]->GetType().GetMr();
      }
    }
    return olx_cmp(m2, m1);
  }

  struct CombiSort {
    typedef int(*cmp_f)(const moiety_t &, const moiety_t &);
    TArrayList<cmp_f> sequence;
    int moiety_cmp(const moiety_t &a, const moiety_t &b) const {
      for (size_t i = 0; i < sequence.Count(); i++) {
        int res = (*sequence[i])(a, b);
        if (res != 0) {
          return res;
        }
      }
      return 0;
    }
  };

  static void UpdateList(TCAtomPList& list,
    const TTypeList<moiety_t> &moieties)
  {
    size_t atom_cnt = 0;
    for (size_t i = 0; i < moieties.Count(); i++) {
      for (size_t j = 0; j < moieties[i].Count(); j++) {
        list[atom_cnt++] = moieties[i][j];
      }
    }
  }

  static void ReoderByMoietyAtom(TTypeList<moiety_t> &moieties,
    const TStrList& atom_names)
  {
    if (atom_names.Count() < 2) {
      return;
    }
    MoietyAtomCmp(moieties, atom_names);
  }
protected:
  struct MoietyAtomCmp {
    TStrList labels;
    olxdict<moiety_t*, olxstr, TPointerComparator> moiety_cache;
    MoietyAtomCmp(TTypeList<moiety_t> &moieties, const TStrList &labels)
      : labels(labels)
    {
      for (size_t i = 0; i < moieties.Count(); i++) {
        for (size_t j = 0; j < moieties[i].Count(); j++) {
          for (size_t li = 0; li < labels.Count(); li++) {
            if (moieties[i][j]->GetLabel().Equalsi(labels[li])) {
              moiety_cache.Add(&moieties[i], labels[li]);
              break;
            }
          }
        }
      }
      if (moiety_cache.Count() < 2) {
        return;
      }
      TPtrList<moiety_t> to_sort;
      for (size_t i = 0; i < moiety_cache.Count(); i++) {
        to_sort.Add(moiety_cache.GetKey(i));
        for (size_t j = 0; j < moieties.Count(); j++) {
          if (&moieties[j] == moiety_cache.GetKey(i)) {
            moieties.Release(j);
            break;
          }
        }
      }
      QuickSorter::SortMF(to_sort, *this, &MoietyAtomCmp::compare);
      for (size_t i = 0; i < moieties.Count(); i++) {
        to_sort.Add(moieties[i]);
      }
      moieties.ReleaseAll();
      for (size_t i = 0; i < to_sort.Count(); i++) {
        moieties.Add(to_sort[i]);
      }
    }
  protected:
    int compare(const moiety_t &a, const moiety_t &b) const {
      return olx_cmp(labels.IndexOf(moiety_cache[&a]),
        labels.IndexOf(moiety_cache[&b]));
    }
  };
};



#endif
