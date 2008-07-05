#ifndef __OLX_HFIX_MODE_H
#define __OLX_HFIX_MODE_H

class THfixMode : public AModeWithLabels  {
  int Hfix;
protected:
public:
  THfixMode(int id) : AModeWithLabels(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    if( !TGlXApp::GetGXApp()->CheckFileType<TIns>() )  return false;
    Hfix = Cmds.IsEmpty() ? 0 : Cmds[0].ToInt();
    TGlXApp::GetMainForm()->SetUserCursor( Hfix, "hfix");
    TGlXApp::GetMainForm()->executeMacro("labels -a -h");
    return true;
  }
  ~THfixMode() {  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom *XA = &(TXAtom&)obj;
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      if( TGlXApp::GetMainForm()->executeMacro(
            olxstr("addins HFIX ") << Hfix << ' ' << XA->Atom().GetLabel()) )
        XA->Atom().CAtom().SetHfix(Hfix);
      return true;
    }
    return false;
  }
};

#endif
