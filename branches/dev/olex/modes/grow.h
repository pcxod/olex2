#ifndef __OLX_GROW_MODE_H
#define __OLX_GROW_MODE_H

class TGrowMode : public AMode  {
protected:
  bool GrowShells;
  short mode;
public:
  TGrowMode(size_t id) : AMode(id), GrowShells(false), mode(0)  {}
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    bool SI = Options.Contains('s'),
         Cov = Options.Contains('c'),
         VdW = Options.Contains('v'),
         Rad = Options.Contains('r');
    GrowShells = Options.Contains("shells");
    const olxstr AtomsToGrow = Cmds.Text(' ');
    mode = 0;
    if( SI )   mode = gmSInteractions;
    if( Cov )  mode = gmCovalent;
    else if( VdW )  {
      mode = gmVanDerWaals;
      olxstr vr = Options.FindValue('v');
      TGlXApp::GetGXApp()->SetDeltaV(vr.IsEmpty() ? 2.0 : vr.ToDouble());
    }
    else if( Rad )
      mode = gmSameAtoms;
    if( mode == 0 )  
      mode = gmCovalent;

    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    TGlXApp::GetGXApp()->SetGrowMode(mode, AtomsToGrow);
    TGlXApp::GetGXApp()->SetXGrowLinesVisible(true);
    TGlXApp::GetGXApp()->SetZoomAfterModelBuilt(false);
    return true;
  }
  void Finalise() {
    TGlXApp::GetGXApp()->SetZoomAfterModelBuilt(true);
    TGlXApp::GetGXApp()->SetXGrowLinesVisible(false);
  }
  virtual bool OnObject(AGDrawObject& obj)  {
    TLattice& latt = TXApp::GetInstance().XFile().GetLattice();
    if( EsdlInstanceOf(obj, TXGrowLine) )  {
      TXGrowLine& xl = (TXGrowLine&)obj;
      if( GrowShells && mode == gmCovalent )
        latt.GrowAtom(*xl.CAtom(), xl.GetTransform());
      else
        latt.GrowFragment(xl.CAtom()->GetFragmentId(), xl.GetTransform());
      return true;
    }
    else if( EsdlInstanceOf(obj, TXAtom) )  {
      TXAtom& a = (TXAtom&)obj;
      if( !a.IsGrown() )
        latt.GrowAtom(a, GrowShells, NULL);
      return true;
    }
    else
      return false;
  }
};

#endif
