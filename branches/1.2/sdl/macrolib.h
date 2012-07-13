/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_macrolib
#define __olx_sdl_macrolib
#include "bapp.h"
#include "estrlist.h"
#include "datafile.h"
#include "dataitem.h"
#include "integration.h"
#include "library.h"
#include "estack.h"
BeginEsdlNamespace()

const uint8_t
  macro_log_macro    = 0x01,  //default log level
  macro_log_function = 0x02;

class TEMacro  {
  olxstr Name, Description;
  TStrList OnListen, OnTerminate, OnAbort, Commands;
  TStrStrList Args;
protected:
  void SubstituteArguments(olxstr &Cmd, TStrList& argv)  {
    olxstr args;
    size_t index = Cmd.FirstIndexOf('%');  // argument by index
    while( index != InvalidIndex && index < (Cmd.Length()-1) )  {
      args.SetLength(0);
      size_t iindex = index;
      while( olxstr::o_isdigit(Cmd.CharAt(iindex+1)) )  {  // extract argument number
        args << Cmd.CharAt(iindex+1);
        iindex++;
        if( iindex >= (Cmd.Length()-1) )  break;
      }
      if( !args.IsEmpty() )  {
        size_t pindex = args.ToSizeT()-1;  // index of the parameter
        if( pindex < argv.Count() )  {  // check if valid argument index
          Cmd.Delete(index, args.Length()+1); // delete %xx value
          Cmd.Insert(argv[pindex], index);  // insert value parameter
        }
        else  {
          TBasicApp::NewLogEntry(logError) << Name << ": wrong argument index: " << (pindex+1)
            << NewLineSequence();
        }
      }
      if( index++ < Cmd.Length() )
        index = Cmd.FirstIndexOf('%', index);  // next argument by index
      else
        index = InvalidIndex;
    }
  }
public:
  TEMacro(const olxstr& name, const olxstr& desc) : 
      Name(name), Description(desc)  { }
  
  inline const olxstr& GetName() const {  return Name;  }
  inline const olxstr& GetDescription() const {  return Description;  }
  template <class ArgsClass, class CmdClass> // must be one of the string lists
  bool ListCommands(const ArgsClass& args, CmdClass& cmds, TStrList& onAbort,
    TStrList& onListen,
    TStrList& onTerminate, olex::IOlexProcessor& olex_processor )
  {
    
    if( Args.Count() < args.Count() )  {
      TBasicApp::NewLogEntry(logError) << Name << ": too many arguments" <<
        NewLineSequence();
      return false;
    }
    TStrList argV( Args.Count() );
    for( size_t i=0; i < argV.Count(); i++ )  {
      if( !Args.GetObject(i).IsEmpty() && Args.GetObject(i).IndexOf('(') !=
        InvalidIndex )
      {
        argV[i] = Args.GetObject(i);
        olex_processor.processFunction(argV[i]);
      }
      else // set the defaults!!
        argV[i] = Args.GetObject(i);
    }
    for( size_t i=0; i < args.Count(); i++ )  {
      size_t eqsi = args[i].FirstIndexOf('=');
      if( eqsi != InvalidIndex )  {  // argument with name and value
        size_t argi = Args.IndexOf( args[i].SubStringTo(eqsi) );
        if( argi == InvalidIndex )  {
          TBasicApp::NewLogEntry(logError) << Name << ": wrong argument name: "
            << args[i].SubStringTo(eqsi);
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
    for( size_t i=0; i < cmds.Count(); i++ )
      SubstituteArguments(cmds[i], argV);
    for( size_t i=0; i < onAbort.Count(); i++ )
      SubstituteArguments(onAbort[i], argV);
    for( size_t i=0; i < onListen.Count(); i++ )
      SubstituteArguments(onListen[i], argV);
    for( size_t i=0; i < onTerminate.Count(); i++ )
      SubstituteArguments(onTerminate[i], argV);
    return true;
  }
  void AddCmd(const olxstr& cmd)  {  Commands.Add(cmd);  }
  void AddOnListenCmd(const olxstr& cmd)  {  OnListen.Add(cmd);  }
  void AddOnAbortCmd(const olxstr& cmd)  {  OnAbort.Add(cmd);  }
  void AddOnTerminateCmd(const olxstr& cmd)  {  OnTerminate.Add(cmd);  }
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
  uint8_t LogLevel;
protected:
/////////////////////////////////////////////////////////////////////////////////////////
  bool ExtractItemVal(const TDataItem& tdi, olxstr& val)  {  // helper function
    val = tdi.GetValue();
    if( val.IsEmpty() )
      val = tdi.GetFieldValue("cmd", EmptyString());
    return !val.IsEmpty();
  }
  void ParseMacro(const TDataItem& macro_def, TEMacro& macro);
  DefMacro(IF)
  DefMacro(Abort)
  DefFunc(LastError)
  DefFunc(Or)
  DefFunc(And)
  DefFunc(Not)
  DefFunc(LogLevel)
public:
  TEMacroLib(olex::IOlexProcessor& olexProcessor)
    : OlexProcessor(olexProcessor), LogLevel(macro_log_macro)  {}
  ~TEMacroLib() {  Clear();  }
  void Init();  // extends the Library with functionality
  void Load(const TDataItem& m_root)  {
    Clear();
    for( size_t i=0; i < m_root.ItemCount(); i++ )  {
      const TDataItem& m_def = m_root.GetItem(i);
      const TDataItem* di = m_def.FindItem("body");
      if( di == NULL )  {
        TBasicApp::NewLogEntry(logError) << "Macro body is not defined: " <<
          m_def.GetName();
        continue;
      }
      TEMacro* m = new TEMacro( m_def.GetName(), m_def.GetFieldValue("help"));
      Macros.Add( m_def.GetName(), m);
      ParseMacro(*di, *m);
    }
  }
  void Clear()  {
    for( size_t i=0; i < Macros.Count(); i++ )
      delete Macros.GetObject(i);
    Macros.Clear();
  }
  
  TEMacro* FindMacro(const olxstr& name )  {  return Macros[name];  }
  
  size_t FindSimilar(const olxstr& name, TPtrList<TEMacro>& out) const {
    size_t cnt = out.Count();
    for( size_t i=0; i < Macros.Count(); i++ )  {
      if( Macros.GetKey(i).StartsFromi(name) )
        out.Add(Macros.GetObject(i));
    }
    return out.Count() - cnt;
  }
  template <class StrLst>
  size_t FindSimilarNames(const olxstr& name, StrLst& out) const {
    size_t cnt = out.Count();
    for( size_t i=0; i < Macros.Count(); i++ )  {
      if( Macros.GetKey(i).StartsFromi(name) )
        out.Add(Macros.GetKey(i));
    }
    return out.Count() - cnt;
  }
  /* if has_owner is true, then in th case the function does not exist no flags
  are set in E
  */
  bool ProcessFunction(olxstr& Cmd, TMacroError& E, bool has_owner);
  //..............................................................................
  template <class Base> void ProcessTopMacro(const olxstr& Cmd, TMacroError& Error, 
    Base& base_instance, void (Base::*ErrorAnalyser)(TMacroError& error))  
  {
    ProcessMacro(Cmd, Error);
    if (ErrorAnalyser != NULL)
      (base_instance.*ErrorAnalyser)(Error);
  }
  //..............................................................................
  void ProcessMacro(const olxstr& Cmd, TMacroError& Error);
  DefPropP(uint8_t, LogLevel)
};

EndEsdlNamespace()
#endif
