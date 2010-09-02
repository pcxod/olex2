#ifndef __OLX_GROW_MODE_H
#define __OLX_GROW_MODE_H

class TGrowMode : public AMode  {
protected:
public:
  TGrowMode(size_t id) : AMode(id)  {}
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    bool SI = Options.Contains('s'),
         Cov = Options.Contains('c'),
         VdW = Options.Contains('v'),
         Rad = Options.Contains('r');

    olxstr AtomsToGrow( Cmds.Text(' ') );

    short mode = 0;
    if( SI )   mode |= gmSInteractions;
    if( Cov )  mode |= gmCovalent;
    else if( VdW )  {
      mode = gmVanDerWaals;
      olxstr vr = Options.FindValue('v');
      TGlXApp::GetGXApp()->SetDeltaV(vr.IsEmpty() ? 3.0 : vr.ToDouble());
    }
    else if( Rad )
      mode = gmSameAtoms;
    if( mode == 0 )  
      mode = gmCovalent;

    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    TGlXApp::GetGXApp()->SetGrowMode( mode, AtomsToGrow );
    TGlXApp::GetGXApp()->SetXGrowLinesVisible(true);
    return true;
  }
  void Finalise() {
    TGlXApp::GetGXApp()->SetXGrowLinesVisible(false);
  }
  virtual bool OnObject(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXGrowLine) )  {
      TGlXApp::GetGXApp()->Grow((TXGrowLine&)obj);
      return true;
    }
    return false;
  }
};

#endif
