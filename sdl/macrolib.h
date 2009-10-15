#ifndef _olx_macrolib
#define _olx_macrolib
#include "bapp.h"
#include "log.h"
#include "estrlist.h"
#include "datafile.h"
#include "dataitem.h"
#include "integration.h"
#include "library.h"
#include "estack.h"

BeginEsdlNamespace()

class TEMacro  {
  olxstr Name, Description;
  TStrList OnListen, OnTerminate, OnAbort, Commands;
  TStrStrList Args;
protected:
  void SubstituteArguments(olxstr &Cmd, TStrList& argv)  {
    int index;
    olxstr args;
    index = Cmd.FirstIndexOf('%');  // argument by index
    while( index >= 0 && index < (Cmd.Length()-1) )  {
      args = EmptyString;
      int iindex = index;
      while( olxstr::o_isdigit(Cmd.CharAt(iindex+1)) )  {  // extract argument number
        args << Cmd.CharAt(iindex+1);
        iindex ++;
        if( iindex >= (Cmd.Length()-1) )  break;
      }
      if( !args.IsEmpty() )  {
        int pindex = args.ToInt()-1;  // index of the parameter
        if( pindex < argv.Count() && pindex >= 0 )  {  // check if valid argument index
          Cmd.Delete(index, args.Length()+1); // delete %xx value
          Cmd.Insert(argv[pindex], index);  // insert value parameter
        }
        else
          TBasicApp::GetLog().Error(olxstr(Name) << ": wrong argument index: " << (pindex+1) << '\n');
      }
      if( index++ < Cmd.Length() )
        index = Cmd.FirstIndexOf('%', index);  // next argument by index
      else
        index = -1;
    }
  }
public:
  TEMacro(const olxstr& name, const olxstr& desc) : 
      Name(name), Description(desc)  { }
  
  inline const olxstr& GetName() const {  return Name;  }
  inline const olxstr& GetDescription() const {  return Description;  }
  template <class ArgsClass, class CmdClass> // must be one of the string lists
  bool ListCommands(const ArgsClass& args, CmdClass& cmds, TStrList& onAbort, TStrList& onListen, 
    TStrList& onTerminate, olex::IOlexProcessor& olex_processor )  {
    
    if( Args.Count() < args.Count() )  {
      TBasicApp::GetLog().Error(olxstr(Name) << ": too many arguments\n");
      return false;
    }
    TStrList argV( Args.Count() );
    for( int i=0; i < argV.Count(); i++ )  {
      if( !Args.GetObject(i).IsEmpty() && Args.GetObject(i).IndexOf('(') != -1 )  {
        olex_processor.executeFunction(Args.GetObject(i), argV[i]);
      }
      else // set the defaults!!
        argV[i] = Args.GetObject(i);
    }
    for( int i=0; i < args.Count(); i++ )  {
      int eqsi = args[i].FirstIndexOf('=');
      if( eqsi != -1 )  {  // argument with name and value
        int argi = Args.IndexOf( args[i].SubStringTo(eqsi) );
        if( argi == -1 )  {
          TBasicApp::GetLog().Error(olxstr(Name) << ": wrong argument name: " << args[i].SubStringTo(eqsi));
          return false;
        }
        else
          argV[argi] = args[i].SubStringFrom(eqsi+1);
      }
      else  // argument by position
        argV[i] = args[i];
    }
    cmds.Assign( Commands );
    onAbort.Assign( OnAbort );
    onListen.Assign( OnListen );
    onTerminate.Assign( OnTerminate );
    for( int i=0; i < cmds.Count(); i++ )
      SubstituteArguments(cmds[i], argV);
    for( int i=0; i < onAbort.Count(); i++ )
      SubstituteArguments(onAbort[i], argV);
    for( int i=0; i < onListen.Count(); i++ )
      SubstituteArguments(onListen[i], argV);
    for( int i=0; i < onTerminate.Count(); i++ )
      SubstituteArguments(onTerminate[i], argV);
    return true;
  }
  void AddCmd(const olxstr& cmd)           {  Commands.Add(cmd);  }
  void AddOnListenCmd(const olxstr& cmd)    {  OnListen.Add(cmd);  }
  void AddOnAbortCmd(const olxstr& cmd)     {  OnAbort.Add(cmd);  }
  void AddOnTerminateCmd(const olxstr& cmd) {  OnTerminate.Add(cmd);  }
  void AddArg(const olxstr& name, const olxstr& val)  {
    Args.Add(name, val);
  }
};

class TEMacroLib {
  TSStrPObjList<olxstr, TEMacro*, true> Macros;
  olex::IOlexProcessor& OlexProcessor;
  static bool is_allowed_in_name(olxch ch) {
    return (olxstr::o_isalphanumeric(ch) || ch == '_' || ch == '.');
  }
protected:
/////////////////////////////////////////////////////////////////////////////////////////
  bool ExtractItemVal(const TDataItem& tdi, olxstr& val)  {  // helper function
    val = tdi.GetValue();
    if( val.IsEmpty() )
      val = tdi.GetFieldValue("cmd", EmptyString);
    return !val.IsEmpty();
  }
  void ParseMacro(const TDataItem& macro_def, TEMacro& macro);
public:
  TEMacroLib(olex::IOlexProcessor& olexProcessor) : OlexProcessor(olexProcessor) {}
  ~TEMacroLib() {  Clear();  }
  void Load(const TDataItem& m_root)  {
    Clear();
    for( int i=0; i < m_root.ItemCount(); i++ )  {
      const TDataItem& m_def = m_root.GetItem(i);
      const TDataItem* di = m_def.FindItem("body");
      if( di == NULL )  {
        TBasicApp::GetLog().Error(olxstr("Macro body is not defined: ") << m_def.GetName());
        continue;
      }
      TEMacro* m = new TEMacro( m_def.GetName(), m_def.GetFieldValue("help"));
      Macros.Add( m_def.GetName(), m);
      ParseMacro(*di, *m);
    }
  }
  void Clear()  {
    for( int i=0; i < Macros.Count(); i++ )
      delete Macros.GetObject(i);
    Macros.Clear();
  }
  
  TEMacro* FindMacro(const olxstr& name )  {  return Macros[name];  }
  
  int FindSimilar(const olxstr& name, TPtrList<TEMacro>& out) const {
    int cnt = out.Count();
    for( int i=0; i < Macros.Count(); i++ )  {
      if( Macros.GetComparable(i).StartsFromi(name) )
        out.Add(Macros.GetObject(i));
    }
    return out.Count() - cnt;
  }
  template <class StrLst>
  int FindSimilarNames(const olxstr& name, StrLst& out) const {
    int cnt = out.Count();
    for( int i=0; i < Macros.Count(); i++ )  {
      if( Macros.GetComparable(i).StartsFromi(name) )
        out.Add(Macros.GetComparable(i));
    }
    return out.Count() - cnt;
  }
  // if has_owner is true, then in th case the function does not exist no flags are set in E
  bool ProcessFunction(olxstr& Cmd, TMacroError& E, bool has_owner);
  //..............................................................................
  template <class Base> void ProcessTopMacro(const olxstr& Cmd, TMacroError& Error, 
    Base& base_instance, void (Base::*ErrorAnalyser)(TMacroError& error))  
  {
    ProcessMacro(Cmd, Error);
    (base_instance.*ErrorAnalyser)(Error);
  }
  //..............................................................................
  void ProcessMacro(const olxstr& Cmd, TMacroError& Error);
};
EndEsdlNamespace()
#endif
