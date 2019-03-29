/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_HFIX_MODE_H
#define __OLX_HFIX_MODE_H

#include "xlcongen.h"
#include "unitcell.h"

class THfixMode : public AModeWithLabels {
  int Hfix;
protected:
  TXlConGen* xlConGen;
public:
  THfixMode(size_t id) : AModeWithLabels(id), xlConGen(0)
  {}
  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    if (!gxapp.CheckFileType<TIns>()) {
      return false;
    }
    Hfix = Cmds.IsEmpty() ? 0 : Cmds[0].ToInt();
    xlConGen = new TXlConGen(gxapp.XFile().GetRM());
    SetUserCursor(Hfix, "hfix");
    olex2.processMacro("labels -a -h");
    return true;
  }
  void Finalise_() {
    olx_del_obj(xlConGen);
  }
  virtual bool OnObject_(AGDrawObject &obj) {
    if (obj.Is<TXAtom>()) {
      TXAtom *XA = &(TXAtom&)obj;
      if (TAfixGroup::IsFittedRing(Hfix)) {
        gxapp.AutoAfixRings(Hfix, XA, true);
      }
      else if (Hfix == 0) {  // special case
        TCAtom& ca = XA->CAtom();
        if (ca.GetDependentAfixGroup() != 0) {
          ca.GetDependentAfixGroup()->Clear();
        }
        else if (ca.DependentHfixGroupCount() != 0) {
          for (size_t i = 0; i < ca.DependentHfixGroupCount(); i++) {
            ca.GetDependentHfixGroup(i).Clear();
          }
        }
        else if (ca.GetParentAfixGroup() != 0) {
          ca.GetParentAfixGroup()->Clear();
        }
      }
      else {
        olex2.processMacro(
          olxstr("hadd ") << Hfix << " #c" << XA->CAtom().GetId());
      }
      return true;
    }
    return false;
  }
};

#endif
