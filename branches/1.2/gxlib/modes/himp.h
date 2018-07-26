/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_HIMP_MODE_H
#define __OLX_HIMP_MODE_H

class THimpMode : public AMode {
  double BondLength;
protected:
public:
  THimpMode(size_t id) : AMode(id) {}
  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    BondLength = Cmds.IsEmpty() ? 0 : Cmds[0].ToDouble();
    if (BondLength <= 0.5) {
      TBasicApp::NewLogEntry(logError) << "suspicious bond length";
      return false;
    }
    SetUserCursor("<->", olxstr(BondLength));
    return true;
  }
  void Finalise_() {
    gxapp.XFile().GetLattice().UpdateConnectivity();
  }
  virtual bool OnObject_(AGDrawObject& obj) {
    if (obj.Is<TXAtom>()) {
      TXAtom& XA = (TXAtom&)obj;
      if (XA.GetType() == iHydrogenZ) {
        TSAtom* aa = 0;
        for (size_t i = 0; i < XA.NodeCount(); i++) {
          if (XA.Node(i).GetType().z > 2) {
            if (aa == 0) {
              aa = &XA.Node(i);
            }
            else {  // bad connectivity
              aa = 0;
              break;
            }
          }
        }
        if (aa != 0) {
          XA.crd() = aa->crd() + (XA.crd() - aa->crd()).NormaliseTo(BondLength);
          XA.ccrd() = gxapp.XFile().GetAsymmUnit().Fractionalise(XA.crd());
          XA.CAtom().ccrd() = XA.ccrd();
          gxapp.MarkLabel(XA, true);
        }
      }
      else {
        for (size_t i = 0; i < XA.NodeCount(); i++) {
          if (XA.Node(i).GetType().z != iHydrogenZ) continue;
          XA.Node(i).crd() = XA.crd() + (XA.Node(i).crd() - XA.crd())
            .NormaliseTo(BondLength);
          XA.Node(i).ccrd() = gxapp.XFile().GetAsymmUnit().Fractionalise(
            XA.Node(i).crd());
          XA.Node(i).CAtom().ccrd() = XA.Node(i).ccrd();
          gxapp.MarkLabel(XA.Node(i), true);
          if (XA.Node(i).CAtom().GetParentAfixGroup() != 0) {
            XA.Node(i).CAtom().GetParentAfixGroup()->SetD(BondLength);
          }
        }
      }
      return true;
    }
    return false;
  }
};

#endif
