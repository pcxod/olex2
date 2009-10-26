#ifndef __OLX_MOVE_MODE_H
#define __OLX_MOVE_MODE_H

class TMoveMode : public AMode  {
  bool Copy;
  vec3d Center;
protected:
public:
  TMoveMode(int id) : AMode(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    Center.Null();
    Copy = Options.Contains('c');

    TXAtomPList Atoms;
    TGlXApp::GetGXApp()->FindXAtoms(Cmds.Text(' '), Atoms, true);
    for( size_t i=0; i < Atoms.Count(); i++ )
      Center += Atoms[i]->Atom().ccrd();

    if( Atoms.Count() != 0 )  Center /= Atoms.Count();

    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    return true;
  }
  ~TMoveMode() {  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TGlXApp::GetGXApp()->MoveFragment(Center, &(TXAtom&)obj, Copy);
      return true;
    }
    return false;
  }
};

#endif
