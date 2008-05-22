#ifndef __OLX_HIMP_MODE_H
#define __OLX_HIMP_MODE_H

class THimpMode : public AMode  {
  double BondLength;
protected:
public:
  THimpMode(int id) : AMode(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    BondLength = Cmds.IsEmpty() ? 0 : Cmds[0].ToDouble();
    if( BondLength <= 0.5 )  {
      TBasicApp::GetLog().Error("suspicious bond length");
      return false;
    }
    TGlXApp::GetMainForm()->SetUserCursor( "<->", olxstr(BondLength) );
    return true;
  }
  ~THimpMode() {
    TGlXApp::GetMainForm()->executeMacro("fuse");
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom *XA = &(TXAtom&)obj;
      if( XA->Atom().GetAtomInfo() == iHydrogenIndex )  {
        TSAtom* aa = NULL;
        for(int i=0; i < XA->Atom().NodeCount(); i++ )  {
          if( XA->Atom().Node(i).GetAtomInfo() != iQPeakIndex )  {
            if( aa == NULL )  aa = &XA->Atom().Node(i);
            else  {  // bad connectivity
              aa = NULL;
              break;
            }
          }
        }
        if( aa != NULL )  {
          TVPointD v(XA->Atom().Center());
          v -= aa->Center();
          v.Normalise();
          v *= BondLength;
          v += aa->Center();
          XA->Atom().Center() = v;
          TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CartesianToCell(v);
          XA->Atom().CCenter() = v;
          XA->Atom().CAtom().CCenter() = v;
          TGlXApp::GetGXApp()->MarkLabel(XA, true);
        }
      }
      return true;
    }
    return false;
  }
};

#endif
