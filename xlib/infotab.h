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

#include "refmodel.h"

BeginXlibNamespace()

const short 
  infotab_htab = 1,
  infotab_rtab = 2,
  infotab_mpla = 3;

class InfoTab : public IEObject {  // need to cast to delete
  olxstr ResiName, ParamName;
  TCAtomGroup atoms;
  RefinementModel& RM;
  short Type;
public:
  InfoTab(RefinementModel& rm, short type, const olxstr& paramName=EmptyString(),
    const olxstr& resiName=EmptyString())
    : RM(rm), Type(type), ParamName(paramName), ResiName(resiName) {}
  
  InfoTab(RefinementModel& rm, const TDataItem& di) : RM(rm) {
    FromDataItem(di, rm);
  }

  InfoTab(RefinementModel& rm, const InfoTab& it) : RM(rm)  {  this->operator = (it);  }

  virtual ~InfoTab()  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( atoms[i].GetMatrix() != NULL ) 
        RM.RemUsedSymm(*atoms[i].GetMatrix());
    }
  }
      
  bool operator == (const InfoTab& it) const {
    if( Type != it.Type )  return false;
    // planes...
    if( Type == infotab_mpla )  return false;
    if( ResiName != it.ResiName )  return false;
    if( atoms.Count() != it.atoms.Count() )  return false;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( atoms[i].GetAtom()->GetId() != it.atoms[i].GetAtom()->GetId() )
        return false;
      if( atoms[i].GetMatrix() != NULL && it.atoms[i].GetMatrix() != NULL )  {
        if( !(*atoms[i].GetMatrix() == *it.atoms[i].GetMatrix()) )
          return false;
      }
      else if( atoms[i].GetMatrix() == NULL && it.atoms[i].GetMatrix() == NULL )
        ;
      else
        return false;
    }
    return true;
  }

  InfoTab& operator = (const InfoTab& it)  {
    ResiName = it.ResiName;
    ParamName = it.ParamName;
    Type = it.Type;
    for( size_t i=0; i < it.atoms.Count(); i++ )  {
      if( it.atoms[i].GetMatrix() != NULL )  {
        atoms.Add(new TGroupCAtom(&RM.aunit.GetAtom(it.atoms[i].GetAtom()->GetId()),
          &RM.AddUsedSymm(*it.atoms[i].GetMatrix())));
      }
      else
        atoms.Add(new TGroupCAtom(&RM.aunit.GetAtom(it.atoms[i].GetAtom()->GetId())));
    }
    return *this;
  }

  void AssignAtoms(const TCAtomGroup& ag)  {
    atoms.Clear();
    for( size_t i=0; i < ag.Count(); i++ )
      AddAtom(ag[i].GetAtom(), ag[i].GetMatrix());
  }
  void AddAtom(TCAtom* ca, const smatd* sm)  {
    atoms.Add(new TGroupCAtom(ca, (sm == NULL ? NULL : &RM.AddUsedSymm(*sm))));
  }

  size_t Count() const {  return atoms.Count();  }
  TGroupCAtom& GetAtom(size_t i) {  return atoms[i];  }
  const TGroupCAtom& GetAtom(size_t i) const {  return atoms[i];  }

  short GetType() const {  return Type;  }

  bool HasDeletedAtom() const {
    for( size_t i=0; i < atoms.Count(); i++ )
      if( atoms[i].GetAtom()->IsDeleted() )
        return true;
    return false;
  }
  bool IsValid() const {
    size_t ac = 0;
    for( size_t i=0; i < atoms.Count(); i++ )
      if( !atoms[i].GetAtom()->IsDeleted() )
        ac++;
    if( Type == infotab_htab && ac == 2 )  return true;
    if( Type == infotab_rtab && ac >= 1 && ac <= 4 && !ParamName.IsEmpty() )  return true;
    if( Type == infotab_mpla && ac >= 3 )  return true;
    return false;
  }

  olxstr InsStr() const {
    olxstr rv = (Type == infotab_htab ? "HTAB" : (Type == infotab_rtab ? "RTAB" : "MPLA"));
    if( !ResiName.IsEmpty() )  {
      rv << '_' << ResiName;
      rv << ' ' << ParamName;
      if( ResiName.IsNumber() )  {
        int resi = ResiName.ToInt();
        for( size_t i=0; i < atoms.Count(); i++ )  {
          if( !atoms[i].GetAtom()->IsDeleted() )
            rv << ' ' << atoms[i].GetFullLabel(RM, resi);
        }
      }
      else  {
        for( size_t i=0; i < atoms.Count(); i++ )  {
          if( !atoms[i].GetAtom()->IsDeleted() )
            rv << ' ' << atoms[i].GetFullLabel(RM, ResiName);
        }
      }
    }
    else  {
      rv << ' ' << ParamName;
      for( size_t i=0; i < atoms.Count(); i++ )  {
        if( !atoms[i].GetAtom()->IsDeleted() )
          rv << ' ' << atoms[i].GetFullLabel(RM);
      }
    }
    return rv;
  }
  void ToDataItem(TDataItem& di) const {
    di.SetValue(Type == infotab_htab ? "HTAB" : (Type == infotab_rtab ? "RTAB" : "MPLA"));
    di.AddField("resi", ResiName);
    di.AddField("param", ParamName);
    TDataItem& ais = di.AddItem("atoms");
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( !atoms[i].GetAtom()->IsDeleted() )  {
        TDataItem& ai = ais.AddItem("atom", atoms[i].GetAtom()->GetTag());
        if( atoms[i].GetMatrix() != NULL )
          ai.AddField("matrix", atoms[i].GetMatrix()->GetId());
      }
    }
  }

  void FromDataItem(const TDataItem& di, RefinementModel& rm)  {
    if( di.GetValue() == "HTAB")
      Type = infotab_htab;
    else if( di.GetValue() == "RTAB")
      Type = infotab_rtab;
    else
      Type = infotab_mpla;
    ResiName = di.GetFieldValue("resi");
    ParamName = di.GetFieldValue("param");
    const TDataItem& ais = di.FindRequiredItem("atoms");
    for( size_t i=0; i < ais.ItemCount(); i++ )  {
      const TDataItem& ai = ais.GetItem(i);
      size_t atom_id = ai.GetValue().ToSizeT();
      olxstr matr_id = ai.GetFieldValue("matrix");
      AddAtom(&rm.aunit.GetAtom(atom_id),
        matr_id.IsEmpty() ? NULL : &rm.GetUsedSymm(matr_id.ToSizeT()));
    }
  }
#ifndef _NO_PYTHON
  PyObject* PyExport()  {
    PyObject* main = PyDict_New();
    PythonExt::SetDictItem(main, "type",
      PythonExt::BuildString(Type == infotab_htab ? "HTAB" : (Type == infotab_rtab ? "RTAB" : "MPLA")));
    PythonExt::SetDictItem(main, "param_name", PythonExt::BuildString(ParamName));
    size_t ac_cnt = 0;
    for( size_t i=0; i < atoms.Count(); i++ )
      if( !atoms[i].GetAtom()->IsDeleted() )
        ac_cnt++;
    PyObject* pya = PyTuple_New(ac_cnt);
    ac_cnt = 0;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      PyTuple_SetItem(pya, ac_cnt++,
        Py_BuildValue("(i,i)", atoms[i].GetAtom()->GetTag(),
          atoms[i].GetMatrix() == NULL ? -1 : atoms[i].GetMatrix()->GetId()));
    }
    PythonExt::SetDictItem(main, "atoms", pya);
    return main;
  }
#endif
};
EndXlibNamespace()

#endif
