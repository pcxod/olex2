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
      else if( Hfix == 0 )  {  // special case
        TCAtom& ca = XA->Atom().CAtom();
        if( ca.GetDependentAfixGroup() != NULL )       ca.GetDependentAfixGroup()->Clear();
        else if( ca.DependentHfixGroupCount() != 0 )  {
          for( int i=0; i < ca.DependentHfixGroupCount(); i++ )
            ca.GetDependentHfixGroup(i).Clear();
        }
        else if( ca.GetParentAfixGroup() != NULL )     ca.GetParentAfixGroup()->Clear();
      }
      else  {
        TIntList parts;
        for( int i=0; i < AE.Count(); i++ )  {
          if( AE.GetCAtom(i).GetPart() != 0 && AE.GetCAtom(i).GetPart() != AE.GetBase().CAtom().GetPart() ) 
            if( parts.IndexOf(AE.GetCAtom(i).GetPart()) == -1 )
              parts.Add( AE.GetCAtom(i).GetPart() );
        }
        if( parts.IsEmpty() )  {
          int afix = TXlConGen::ShelxToOlex(Hfix, AE);
          if( afix != -1 )  {
            xlConGen->FixAtom(AE, afix, TAtomsInfo::GetInstance()->GetAtomInfo(iHydrogenIndex));
            TGlXApp::GetMainForm()->executeMacro("fuse");
          }
        }
        else  {
          bool processed = false;
          TCAtomPList generated;
          for( int i=0; i < parts.Count(); i++ )  {
            AE.Clear();
            TGlXApp::GetGXApp()->XFile().GetUnitCell().GetAtomEnviList(XA->Atom(), AE, false, parts[i]);
            int afix = TXlConGen::ShelxToOlex(Hfix, AE);
            if( afix != -1 )  {
              xlConGen->FixAtom(AE, afix, TAtomsInfo::GetInstance()->GetAtomInfo(iHydrogenIndex), false, &generated);
              if( !generated.IsEmpty() )  {
                for( int j=0; j < generated.Count(); j++ )
                  generated[j]->SetPart( parts[i] );
                generated.Clear();
              }
              processed = true;
            }
          }
          if( processed )
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
