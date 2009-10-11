#ifndef __OLX_PACK_MODE_H
#define __OLX_PACK_MODE_H

class TPackMode : public AMode  {
protected:
public:
  TPackMode(int id) : AMode(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {

    olxstr AtomsToGrow( Cmds.Text(' ') );

    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    TGlXApp::GetGXApp()->SetPackMode( 0, AtomsToGrow );
    TGlXApp::GetGXApp()->SetXGrowPointsVisible(true);

    return true;
  }
  ~TPackMode() {
    TGlXApp::GetGXApp()->SetXGrowPointsVisible(false);
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXGrowPoint) )  {
      TGlXApp::GetGXApp()->Grow( (TXGrowPoint&)obj );
      return true;
    }
    return false;
  }
};

#endif
