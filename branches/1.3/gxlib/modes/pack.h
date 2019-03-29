/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_PACK_MODE_H
#define __OLX_PACK_MODE_H

class TPackMode : public AMode {
public:
  TPackMode(size_t id) : AMode(id)
  {}
  bool Initialise_(TStrObjList &Cmds, const TParamList &Options) {
    olxstr AtomsToGrow(Cmds.Text(' '));
    olex2.processMacro("cursor(hand)");
    gxapp.SetPackMode(0, AtomsToGrow);
    gxapp.SetXGrowPointsVisible(true);
    gxapp.SetZoomAfterModelBuilt(false);
    return true;
  }
  void Finalise_() {
    gxapp.SetZoomAfterModelBuilt(true);
    gxapp.SetXGrowPointsVisible(false);
  }
  virtual bool OnObject_(AGDrawObject& obj) {
    if (obj.Is<TXGrowPoint>()) {
      gxapp.Grow((TXGrowPoint&)obj);
      return true;
    }
    return false;
  }
};

#endif
