/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olxconn_info_H
#define __olxconn_info_H

#include "edict.h"
#include "bapp.h"
#include "connext.h"
#include "dataitem.h"

BeginXlibNamespace()

class RefinementModel;

class ConnInfo  {
public:
protected:
  struct AtomConnInfo : public CXConnInfoBase  {
    TCAtom* atom;
    bool temporary;
    BondInfoList BondsToCreate, BondsToRemove;
    AtomConnInfo()
      : atom(NULL), temporary(false)
    {}
    AtomConnInfo(TCAtom& ca)
      : atom(&ca), temporary(false)
    {}
    AtomConnInfo(const AtomConnInfo& ci)
      : CXConnInfoBase(ci), atom(ci.atom), temporary(ci.temporary)
    {}
    AtomConnInfo& operator = (const AtomConnInfo& ci)  {
      CXConnInfoBase::operator = (ci);
      atom = ci.atom;
      BondsToCreate = ci.BondsToCreate;
      BondsToRemove = ci.BondsToRemove;
      temporary = ci.temporary;
      return *this;
    }
    void ToDataItem(TDataItem& item) const;
    void FromDataItem(const TDataItem& item, RefinementModel& rm, TCAtom& atom);
#ifdef _PYTHON
    PyObject* PyExport();
#endif
  };
  struct TypeConnInfo : public CXConnInfoBase  {
    const cm_Element* atomType;
    TypeConnInfo() : atomType(NULL) {}
    TypeConnInfo(const cm_Element& type)
      : atomType(&type)
    {}
    TypeConnInfo(const TypeConnInfo& ci)
      : CXConnInfoBase(ci), atomType(ci.atomType)
    {}
    TypeConnInfo& operator = (const TypeConnInfo& ti)  {
      CXConnInfoBase::operator = (ti);
      atomType = ti.atomType;
      return *this;
    }
    void ToDataItem(TDataItem& item) const;
    void FromDataItem(const TDataItem& item, const cm_Element* elm);
#ifdef _PYTHON
    PyObject* PyExport();
#endif
  };
  olxdict<TCAtom*, AtomConnInfo, TPointerComparator> AtomInfo;
  olxdict<const cm_Element*, TypeConnInfo, TPointerComparator> TypeInfo;
  typedef SortedObjectList<int, TPrimitiveComparator> SortedIntList;
  TTypeList<SortedIntList> PartGroups;
  // optimised for search
  olx_pdict<int, SortedIntList> PartGroups_;
  //
  const smatd* GetCorrectMatrix(const smatd* eqiv1, const smatd* eqiv2,
    bool release) const;
public:
  ConnInfo(RefinementModel& _rm) : rm(_rm) {}

  RefinementModel& rm;

  // prepares a list of extra connectivity info for each atom of the AUnit
  CXConnInfo& GetConnInfo(const TCAtom& ca) const;
  // an object created with new is returned always
  CXConnInfo& GetConnInfo(const cm_Element& elm) const;

  void ProcessConn(TStrList& ins);
  // the atom's connetivity table to have no bonds
  void Disconnect(TCAtom& ca);
  // eqiv corresponds to a2
  static size_t FindBondIndex(const BondInfoList& list, TCAtom* key,
    TCAtom& a1, TCAtom& a2, const smatd* eqiv);
  void AddBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv1,
    const smatd* eqiv2, bool release_eqiv);
  void RemBond(TCAtom& a1, TCAtom& a2, const smatd* eqiv1,
    const smatd* eqiv2, bool release_eqiv);
  void AddBond(TCAtom& a1, TCAtom& a2, const smatd &eqiv1,
    const smatd &eqiv2);
  void RemBond(TCAtom& a1, TCAtom& a2, const smatd &eqiv1,
    const smatd &eqiv2);
  void SetMaxBondsAndR(TCAtom &a, size_t maxBonds, double r = -1);
  /* combines all bonds to create and delete for this atom and imposed by
  others assumes that all atoms already have connectivity information attached.
  The newly created matrices are stored in the ml - note then the list is
  deleted the compiled information might become invalid (as some bonds will
  refer to matrices in the list)
  */
  static void Compile(const TCAtom& a, BondInfoList& toCreate,
    BondInfoList& toDelete, smatd_list& ml);
  //.................................................................
  void ProcessFree(const TStrList& ins);
  void ProcessBind(const TStrList& ins);
  // checks if the two parts are bound by BIND
  bool ArePartsGroupped(int a, int b) {
    size_t idx = PartGroups_.IndexOf(a);
    if (idx != InvalidIndex) {
      return PartGroups_.GetValue(idx).Contains(b);
    }
    return false;
  }
  void Clear()  {
    AtomInfo.Clear();
    TypeInfo.Clear();
    PartGroups.Clear();
    PartGroups_.Clear();
  }

  void ToInsList(TStrList& ins) const;

  void Assign(const ConnInfo& ci);

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item);
#ifdef _PYTHON
  PyObject* PyExport();
#endif
};

struct DistanceGenerator {
  struct idx_pair_t : public olx_pair_t<size_t, size_t> {
    idx_pair_t(size_t a, size_t b)
      : olx_pair_t<size_t, size_t>(a, b)
    {}
    int Compare(const idx_pair_t &p) const {
      int d = olx_cmp(a, p.a);
      if (d == 0) {
        d = olx_cmp(b, p.b);
      }
      return d;
    }
  };
  typedef olxset<idx_pair_t, TComparableComparator> distance_set_t;
  typedef olxset<size_t, TPrimitiveComparator> atom_set_t;
  typedef olxdict<size_t, size_t, TPrimitiveComparator> atom_map_1_t;
  typedef olxdict<size_t, TSizeList, TPrimitiveComparator> atom_map_N_t;
  distance_set_t distances_12, distances_13;
  /* inclusive - if true only atoms in the groups are considered, otherwise -
  the immediate environment is consedered too
  Note that then generating restraints in non-exclusive mode - if the atom of
  a 'dependent' group is not in the atom_map - the atom of the reference, 0
  group will be used!
  */
  void Generate(const TAsymmUnit &au, const atom_set_t &atoms,
    bool generate_13, bool inclusive);
  void Generate(const TCAtomPList atoms, bool generate_13, bool inclusive);
  void GenerateSADI(RefinementModel &rm, const atom_map_1_t &atom_map,
    double esd12, double esd13) const
  {
    GenerateSADI_(distances_12, esd12, rm, atom_map);
    GenerateSADI_(distances_13, esd13, rm, atom_map);
  }
  void GenerateSADI(RefinementModel &rm, const atom_map_N_t &atom_map,
    double esd12, double esd13) const
  {
    GenerateSADI_(distances_12, esd12, rm, atom_map);
    GenerateSADI_(distances_13, esd13, rm, atom_map);
  }
  TStrList::const_list_type GenerateSADIList(const TAsymmUnit& au,
    const atom_map_1_t& atom_map, double esd12 = 0.02, double esd13 = 0.04) const
  {
    TStrList rv = GenerateSADIList_(distances_12, esd12, au, atom_map);
    return rv.AddAll(GenerateSADIList_(distances_13, esd13, au, atom_map));
  }
  TStrList::const_list_type GenerateSADIList(const TAsymmUnit& au,
    const atom_map_N_t& atom_map, double esd12 = 0.02,double esd13 = 0.04) const
  {
    TStrList rv = GenerateSADIList_(distances_12, esd12, au, atom_map);
    return rv.AddAll(GenerateSADIList_(distances_13, esd13, au, atom_map));
  }
  /* Generates a set of 1-2 and 1-3 DFIXes for the given connected set of atoms
  * in the RefinementModel of the AU of first atom in the set.
  * Set esd to a negatove value to suppress the generation of the related DFIXes.
  */
  static void GenerateDFIX(TCAtomPList &atoms, bool explict = true,
    double esd_12=0.02, double esd_13=0.04);
private:
  static void GenerateSADI_(const distance_set_t &d, double esd,
    RefinementModel &rm, const atom_map_1_t &atom_map);
  static void GenerateSADI_(const distance_set_t &d, double esd,
    RefinementModel &rm, const atom_map_N_t &atom_map);
  static TStrList::const_list_type GenerateSADIList_(
    const distance_set_t& d, double esd, const TAsymmUnit& au,
    const atom_map_1_t& atom_map);
  static TStrList::const_list_type GenerateSADIList_(
    const distance_set_t& d, double esd, const TAsymmUnit& au,
    const atom_map_N_t& atom_map);
};

EndXlibNamespace()
#endif
