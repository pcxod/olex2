/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

/* implementation of the HTAB and RTAB for management of deleted atoms etc */
#ifndef _olx_info_tab_H
#define _olx_info_tab_H
#include "catomlist.h"
#include "refmodel.h"
#undef AddAtom

BeginXlibNamespace()

const short
  infotab_htab = 1,
  infotab_rtab = 2,
  infotab_mpla = 3,
  infotab_bond = 4,
  infotab_conf = 5;

class InfoTab : public IEObject {  // need to cast to delete
  RefinementModel& RM;
  short Type,
    AtomCount; // for MPLA, CONF
  olxstr ParamName;
  AtomRefList atoms;
public:
  InfoTab(RefinementModel& rm, short type,
    const olxstr& paramName=EmptyString(), short atomCount=-1)
    : RM(rm), Type(type), AtomCount(atomCount),
    ParamName(paramName), atoms(rm)
  {}

  InfoTab(RefinementModel& rm, const TDataItem& di)
    : RM(rm), atoms(rm)
  {
    FromDataItem(di, rm);
  }

  InfoTab(RefinementModel& rm, const InfoTab& it)
    : RM(rm), atoms(rm)
  {
    this->operator = (it);
  }

  virtual ~InfoTab() {}
  bool operator == (const InfoTab& it) const;
  InfoTab& operator = (const InfoTab& it);

  void FromExpression(const olxstr &e, const olxstr &resi=EmptyString())  {
    atoms.Build(e, resi);
  }
  void AddAtom(TCAtom& ca, const smatd* sm);
  void UpdateResi() { atoms.UpdateResi(); }
  // this is called internally by the RM
  void OnAUUpdate() { atoms.OnAUUpdate();  }
  void BeginAUSort() { atoms.BeginAUSort(); }
  void EndAUSort() { atoms.EndAUSort(); }

  AtomRefList& GetAtoms() {  return atoms;  }
  const AtomRefList& GetAtoms() const {  return atoms;  }

  short GetType() const {  return Type;  }

  bool IsValid() const;

  olxstr GetName() const {
    return olxstr(Type == infotab_htab ? "HTAB" :
      (Type == infotab_rtab ? "RTAB"
        : (Type == infotab_mpla ? "MPLA"
        : (Type == infotab_bond ? "BOND" : "CONF"))));
  }

  TIString ToString() const;

  olxstr InsStr() const;

  void ToDataItem(TDataItem& di) const;
  void FromDataItem(const TDataItem& di, RefinementModel& rm);
#ifdef _PYTHON
  PyObject* PyExport();
#endif
};

EndXlibNamespace()
#endif
