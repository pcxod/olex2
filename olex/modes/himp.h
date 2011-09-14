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

class THimpMode : public AMode  {
  double BondLength;
protected:
public:
  THimpMode(size_t id) : AMode(id)  {}
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    BondLength = Cmds.IsEmpty() ? 0 : Cmds[0].ToDouble();
    if( BondLength <= 0.5 )  {
      TBasicApp::NewLogEntry(logError) << "suspicious bond length";
      return false;
    }
    TGlXApp::GetMainForm()->SetUserCursor("<->", olxstr(BondLength));
    return true;
  }
  void Finalise()  {
    TXApp::GetInstance().XFile().GetLattice().UpdateConnectivity();
  }
  virtual bool OnObject(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      TXAtom& XA = (TXAtom&)obj;
      if( XA.GetType() == iHydrogenZ )  {
        TSAtom* aa = NULL;
        for( size_t i=0; i < XA.NodeCount(); i++ )  {
          if( XA.Node(i).GetType() != iQPeakZ )  {
            if( aa == NULL )  aa = &XA.Node(i);
            else  {  // bad connectivity
              aa = NULL;
              break;
            }
          }
        }
        if( aa != NULL )  {
          vec3d v(XA.crd());
          v -= aa->crd();
          v.NormaliseTo(BondLength);
          v += aa->crd();
          XA.crd() = v;
          TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CartesianToCell(v);
          XA.ccrd() = v;
          XA.CAtom().ccrd() = v;
          TGlXApp::GetGXApp()->MarkLabel(XA, true);
        }
      }
      return true;
    }
    return false;
  }
};

#endif
