#ifndef __OLX_HFIX_MODE_H
#define __OLX_HFIX_MODE_H

#include "xlcongen.h"
#include "unitcell.h"

class THfixMode : public AModeWithLabels  {
  int Hfix;
protected:
  TXlConGen* xlConGen;
public:
  THfixMode(int id) : AModeWithLabels(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    if( !TGlXApp::GetGXApp()->CheckFileType<TIns>() )  return false;
    Hfix = Cmds.IsEmpty() ? 0 : Cmds[0].ToInt();
    xlConGen = new TXlConGen( (TIns*)TGlXApp::GetGXApp()->XFile().GetLastLoader() );
    TGlXApp::GetMainForm()->SetUserCursor( Hfix, "hfix");
    TGlXApp::GetMainForm()->executeMacro("labels -a -h");
    return true;
  }
  ~THfixMode() {  
    if( xlConGen != NULL )
      delete xlConGen;
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom *XA = &(TXAtom&)obj;
      TAtomEnvi AE;
      TGlXApp::GetGXApp()->XFile().GetUnitCell().GetAtomEnviList(XA->Atom(), AE);
      int n = TAfixGroup::GetN(Hfix);
      if( TAfixGroup::IsFitted(Hfix) && (n == 6 || n == 9) )  {
        TGlXApp::GetGXApp()->AutoAfixRings(Hfix, &XA->Atom(), true);
      }
      else  {
        int afix = TXlConGen::ShelxToOlex(Hfix, AE);
        if( afix != -1 )  {
          xlConGen->FixAtom(AE, afix, TAtomsInfo::GetInstance()->GetAtomInfo(iHydrogenIndex));
          TGlXApp::GetMainForm()->executeMacro("fuse");
        }
      }
      //if( TGlXApp::GetMainForm()->executeMacro(
      //      olxstr("addins HFIX ") << Hfix << ' ' << XA->Atom().GetLabel()) )
      //  XA->Atom().CAtom().SetHfix(Hfix);
      
      return true;
    }
    return false;
  }
};

#endif
