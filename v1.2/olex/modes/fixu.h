#ifndef __OLX_FIXU_MODE_H
#define __OLX_FIXU_MODE_H

class TFixUMode : public AModeWithLabels  {
  double Val;
protected:
  class TFixUModeUndo : public TUndoData {
    TXAtom* Atom;
    double Uiso, Vars[6];

  public:
    TFixUModeUndo(TXAtom* XA) :
        TUndoData(new TUndoActionImpl<TFixUModeUndo>(this, &TFixUModeUndo::undo))  {
      Atom = XA;
      Uiso = XA->Atom().CAtom().GetUiso();
      for( int i=0; i < 6; i++ )
        Vars[i] = XA->Atom().CAtom().FixedValues()[TCAtom::UisoFixedValuesOffset + i];
    }
    void undo(TUndoData* data)  {
      TGlXApp::GetGXApp()->MarkLabel(Atom, false);
      Atom->Atom().CAtom().SetUiso( Uiso );
      for( int i=0; i < 6; i++ )
        Atom->Atom().CAtom().FixedValues()[TCAtom::UisoFixedValuesOffset + i] = Vars[i];
    }
  };

public:
  TFixUMode(int id) : AModeWithLabels(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    Val = Cmds.IsEmpty() ? 1 : Cmds[0].ToDouble();
    TGlXApp::GetMainForm()->executeMacro("labels -f");
    if( Val == 0 )
      TGlXApp::GetMainForm()->SetUserCursor( "<U>", "fix" );
    else
      TGlXApp::GetMainForm()->SetUserCursor( Val, "fixU" );
    return true;
  }
  ~TFixUMode() {  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom *XA = &(TXAtom&)obj;

      TGlXApp::GetMainForm()->GetUndoStack()->Push( new TFixUModeUndo(XA) );

      if( Val != 0 )  {
        if( XA->Atom().CAtom().GetEllipsoid() == NULL )  {
          if( Val < 0 )  {  // riding atom
            XA->Atom().CAtom().SetUisoVar( Val );
            XA->Atom().CAtom().SetUiso( caDefIso );
          }
          else  {
            int iv = (int)Val/10;  iv *= 10;
            XA->Atom().CAtom().SetUisoVar( Val );
            XA->Atom().CAtom().SetUiso( Val - iv );
          }
        }
        else
          TBasicApp::GetLog().Error("Do not know how to fix anisotropic atom U");
      }
      else  {
        for( int i=0; i < 6; i++ )
          XA->Atom().CAtom().FixedValues()[TCAtom::UisoFixedValuesOffset + i] = 10;
      }
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
