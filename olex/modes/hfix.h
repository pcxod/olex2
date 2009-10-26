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
    xlConGen = new TXlConGen( TGlXApp::GetGXApp()->XFile().GetRM() );
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
      int n = TAfixGroup::GetN(Hfix);
      if( TAfixGroup::IsFitted(Hfix) && (n == 6 || n == 9) )  {
        TGlXApp::GetGXApp()->AutoAfixRings(Hfix, &XA->Atom(), true);
      }
      else if( Hfix == 0 )  {  // special case
        TCAtom& ca = XA->Atom().CAtom();
        if( ca.GetDependentAfixGroup() != NULL )       ca.GetDependentAfixGroup()->Clear();
        else if( ca.DependentHfixGroupCount() != 0 )  {
          for( size_t i=0; i < ca.DependentHfixGroupCount(); i++ )
            ca.GetDependentHfixGroup(i).Clear();
        }
        else if( ca.GetParentAfixGroup() != NULL )     ca.GetParentAfixGroup()->Clear();
      }
      else  {
        TGlXApp::GetMainForm()->executeMacro( olxstr("hadd ") << Hfix << " #c" << XA->Atom().CAtom().GetId() );
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
