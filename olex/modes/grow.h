#ifndef __OLX_GROW_MODE_H
#define __OLX_GROW_MODE_H

class TGrowMode : public AMode  {
protected:
public:
  TGrowMode(int id) : AMode(id)  {  }
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    bool SI = Options.Contains('s'),
         Cov = Options.Contains('c');

    olxstr AtomsToGrow( Cmds.Text(' ') );

    short mode = 0;
    if( SI )   mode |= gmSInteractions;
    if( Cov )  mode |= gmCovalent;
    if( !mode )  mode = gmCovalent;

    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    if( !AtomsToGrow.IsEmpty() )
      TGlXApp::GetGXApp()->SetGrowMode( gmSameAtoms, AtomsToGrow );
    else
      TGlXApp::GetGXApp()->SetGrowMode( mode, EmptyString );
    TGlXApp::GetGXApp()->SetXGrowLinesVisible(true);
    return true;
  }
  ~TGrowMode() {
    TGlXApp::GetGXApp()->SetXGrowLinesVisible(false);
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXGrowLine) )  {
      TGlXApp::GetGXApp()->Grow( (TXGrowLine&)obj );
      return true;
    }
    return false;
  }
};

#endif
