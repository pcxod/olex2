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
      TBasicApp::GetLog().Error("suspicious bond length");
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
      if( XA.Atom().GetType() == iHydrogenZ )  {
        TSAtom* aa = NULL;
        for( size_t i=0; i < XA.Atom().NodeCount(); i++ )  {
          if( XA.Atom().Node(i).GetType() != iQPeakZ )  {
            if( aa == NULL )  aa = &XA.Atom().Node(i);
            else  {  // bad connectivity
              aa = NULL;
              break;
            }
          }
        }
        if( aa != NULL )  {
          vec3d v(XA.Atom().crd());
          v -= aa->crd();
          v.NormaliseTo(BondLength);
          v += aa->crd();
          XA.Atom().crd() = v;
          TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CartesianToCell(v);
          XA.Atom().ccrd() = v;
          XA.Atom().CAtom().ccrd() = v;
          TGlXApp::GetGXApp()->MarkLabel(XA, true);
        }
      }
      return true;
    }
    return false;
  }
};

#endif