/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#ifndef _olx_xlib_selected_H
#define _olx_xlib_selected_H
#include "asymmunit.h"
#undef AddAtom

BeginXlibNamespace()

class IAtomMask {
public:
  virtual ~IAtomMask() {}
  virtual bool matches(const TCAtom &a) const = 0;
  virtual bool equals(const IAtomMask &m) const = 0;
  virtual bool isValid() const = 0;
  virtual IAtomMask *replicate(const TAsymmUnit &) const = 0;
  virtual olxstr toString() const = 0;
  static IAtomMask *build(const olxstr &mask, const TAsymmUnit &au);
};
//.............................................................................
//.............................................................................
class AtomNameMask : public IAtomMask {
  const TCAtom &atom;
public:
  AtomNameMask(const AtomNameMask &m, const TAsymmUnit &au);
  AtomNameMask(const TCAtom &atom)
    : atom(atom)
  {}
  virtual bool matches(const TCAtom &a) const;
  virtual bool isValid() const { return !atom.IsDeleted(); }
  virtual bool equals(const IAtomMask &m_) const;
  virtual olxstr toString() const { return atom.GetLabel(); }
  virtual IAtomMask *replicate(const TAsymmUnit &au) const {
    return new AtomNameMask(*this, au);
  }
};
//.............................................................................
//.............................................................................
class AtomTypeMask : public IAtomMask {
  struct AtomType {
    short z;
    const cm_Element *element;
    bool not;
    AtomType(short z, bool not) : z(z), element(0), not(not)
    {}
    AtomType(const cm_Element &elm, bool not) : z(0), element(&elm), not(not)
    {}
    int Compare(const AtomType &a) const {
      if (element == 0) {
        if (a.element == 0)
          return olx_cmp(z, a.z);
        if (z == -2 && XElementLib::IsMetal(*a.element))
          return 0;
        if (z == -3 && XElementLib::IsHalogen(*a.element))
          return 0;
        return -1;
      }
      else {
        if (a.element == 0)
          return a.Compare(*this);
        return olx_cmp(element->z, a.element->z);
      }
    }
    olxstr str() const {
      if (element != 0) return element->symbol;
      if (z == -2) return 'M';
      return 'X';
    }
    olxstr toString() const {
      return not ? str().Insert('-', 0) : str();
    }
  };
  SortedObjectList<AtomType, TComparableComparator> types;
  bool not;
public:
  AtomTypeMask(const AtomTypeMask &m)
    : types(m.types), not(m.not)
  {}
  AtomTypeMask(const olxstr &exp, const TAsymmUnit &);
  virtual bool matches(const TCAtom &a) const;
  virtual bool equals(const IAtomMask &m_) const;
  virtual bool isValid() const { return true; }
  virtual olxstr toString() const;
  virtual IAtomMask *replicate(const TAsymmUnit &) const {
    return new AtomTypeMask(*this);
  }
};
//.............................................................................
//.............................................................................
//.............................................................................
class SelectedTableRows {
  typedef TTypeList<IAtomMask> row_t;
  TTypeList<row_t> bonds,
    angles,
    dihedrals;
  bool MatchRows(const TCAtomPList &row, TTypeList<row_t> &where) const;
  bool AddRow(TTypeList<row_t> &where, const olxstr &row_,
    const TAsymmUnit &au);
  void ProcessRows(TTypeList<row_t> &dest,
    const TStrList &rows, const TAsymmUnit &au);
  void ToDataItem(const TTypeList<row_t> &what, TDataItem &di) const;
  void FromDataItem(TTypeList<row_t> &what, const TDataItem &di,
    const TAsymmUnit &au);
  void Assign(TTypeList<row_t> &dest, const TTypeList<row_t> &src,
    const TAsymmUnit &au);
public:
  SelectedTableRows()
  {}
  void Assign(const SelectedTableRows &str, const TAsymmUnit& au);
  void Clear();
  void AddSelectedBonds(const TStrList &defs, const TAsymmUnit& au) {
    ProcessRows(bonds, defs, au);
  }
  void AddSelectedAngles(const TStrList &defs, const TAsymmUnit& au) {
    ProcessRows(angles, defs, au);
  }
  void AddSelectedDihedrals(const TStrList &defs, const TAsymmUnit& au) {
    ProcessRows(dihedrals, defs, au);
  }
  void Process(class TCif &cif);
  void ToDataItem(TDataItem &di) const;
  void FromDataItem(const TDataItem &di, const TAsymmUnit &au);
};
EndXlibNamespace()

#endif
