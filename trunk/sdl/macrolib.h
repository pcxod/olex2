#ifndef _olx_macrolib
#define _olx_macrolib
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
  TEMacro(const olxstr& name, const olxstr& desc) : Name(name), Description(desc) { }
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
      if( !Args.Object(i).IsEmpty() && Args.Object(i).IndexOf('(') != -1 )  {
        olex_processor.executeFunction(Args.Object(i), argV[i]);
      }
      else // set the defaults!!
        argV[i] = Args.Object(i);
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
  TSStrPObjList<olxstr, TEMacro*, false> Macros;
  olex::IOlexProcessor& OlexProcessor;
protected:
/////////////////////////////////////////////////////////////////////////////////////////
  bool ExtractItemVal(const TDataItem& tdi, olxstr& val)  {  // helper function
    val = tdi.GetValue();
    if( val.IsEmpty() )
      val = tdi.GetFieldValue("cmd", EmptyString);
    return !val.IsEmpty();
  }
  void ParseMacro(const TDataItem& macro_def, TEMacro& macro)  {
    olxstr Tmp;
    TDataItem* di = macro_def.FindItem("cmd");
    if( di != NULL )  {
      for( int i=0; i < di->ItemCount(); i++ )
        if( ExtractItemVal(di->GetItem(i), Tmp) )
          macro.AddCmd( Tmp );
    }
    di = macro_def.FindItem("args");
    if( di != NULL )  {
      for( int i=0; i < di->ItemCount(); i++ )  {
        const TDataItem& tdi = di->GetItem(i);
        macro.AddArg(tdi.GetFieldValue("name", EmptyString), tdi.GetFieldValue("def"));
      }
    }
    di = macro_def.FindItem("onterminate");
    if( di != NULL )  {
      for( int i=0; i < di->ItemCount(); i++ )
        if( ExtractItemVal(di->GetItem(i), Tmp) )
          macro.AddOnTerminateCmd( Tmp );
    }
    di = macro_def.FindItem("onlisten");
    if( di != NULL )  {
      for( int i=0; i < di->ItemCount(); i++ )
        if( ExtractItemVal(di->GetItem(i), Tmp) )
          macro.AddOnListenCmd( Tmp );
    }
    di = macro_def.FindItem("onabort");
    if( di != NULL )  {
      for( int i=0; i < di->ItemCount(); i++ ) 
        if( ExtractItemVal(di->GetItem(i), Tmp) )
        macro.AddOnAbortCmd( Tmp );
    }
  }
/////////////////////////////////////////////////////////////////////////////////////////
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
      delete Macros.Object(i);
    Macros.Clear();
  }
  
  TEMacro* FindMacro(const olxstr& name )  {  return Macros[name];  }

  bool ProcessMacroFunc(olxstr& Cmd, TMacroError& E)  {
    if( Cmd.IndexOf('(') == -1 )  return true;
    if( E.GetStack() != NULL )
      E.GetStack()->Push(Cmd);
    int specialFunctionIndex = Cmd.IndexOf('$');
    if( specialFunctionIndex != -1 )  {
      olxstr spFunction;
      int i=specialFunctionIndex, bc = 0;
      bool funcStarted = false;
      while( i++ < Cmd.Length() )  {
        if( Cmd.CharAt(i) == '(' )  {  bc++;  funcStarted = true;  }
        if( Cmd.CharAt(i) == ')' )  bc--;
        if( bc == 0 && funcStarted )  {
          spFunction << ')';
          if( ProcessMacroFunc( spFunction, E ) )  {
            Cmd.Delete( specialFunctionIndex, i - specialFunctionIndex + 1 );
            Cmd.Insert( spFunction, specialFunctionIndex );
          }
          else  {
            Cmd.Delete( specialFunctionIndex, i - specialFunctionIndex + 1 );
          }
          specialFunctionIndex = Cmd.IndexOf('$');
          if( specialFunctionIndex == -1 )  {
            if( E.GetStack() != NULL )  E.GetStack()->Pop();
            return true;
          }
          i = specialFunctionIndex;
          spFunction = EmptyString;
          funcStarted = false;
          continue;
        }
        spFunction << Cmd[i];
      }
      return false;
    }

    int bc=0, fstart=0, fend, astart = 0, aend = 0;
    olxstr Func, ArgV;
    TStrObjList Params;
    ABasicFunction *Function;
    for( int i=0; i < Cmd.Length(); i++ )  {
      if( Cmd.CharAt(i) == '(' && !Func.IsEmpty())  {
        if( Func.EndsWith('.') )  {  Func = EmptyString;  continue;  }
        bc++;
        i++;
        fend = -1;
        astart = aend = i;
        while( i < Cmd.Length() )  {
          if( Cmd.CharAt(i) == '(' )  bc++;
          if( Cmd.CharAt(i) == ')' )  {
            bc--;
            if( bc == 0 )  {  fend = i+1;  break; }
          }
          ArgV << Cmd.CharAt(i);
          i++;
          aend++;
        }
        if( !ArgV.IsEmpty() )  { // arecursive call to all inner functions
          Params.Clear();
          TParamList::StrtokParams(ArgV, ',', Params);
          // evaluation will be called from within the functions
          if( Func.Comparei("or") != 0 && Func.Comparei("and") != 0 )  {
            olxstr localArg;
            for( int j=0; j < Params.Count(); j++ )  {
              if( !ProcessMacroFunc(Params[j], E) )  {
                if( !Func.Comparei("eval") ) // put the function back
                  Params[j] = ArgV;
                else  return false;
              }
            }
          }
          if( !Params.IsEmpty() )  ArgV = Params[0];
        }
        if( fend == -1 )  {
          E.ProcessingError(__OlxSourceInfo, olxstr("Number of brackets does not match: ") << Cmd);
          return false;
        }
        if( Func.IsEmpty() )  {  // in case arithmetic ()
          Cmd.Delete(fstart+1, fend-fstart-2);  // have to leave ()
          Cmd.Insert(ArgV, fstart+1);
          if( E.GetStack() != NULL )  E.GetStack()->Pop();
          return true;
        }
        E.Reset();
        Function = OlexProcessor.GetLibrary().FindFunction(Func);//, Params.Count() );
        //TODO: proper function management is needed ...
        if( Function == NULL )  {
          if( !Func.IsEmpty() )  {
            E.NonexitingMacroError( Func );
            return false;
          }
          if( E.GetStack() != NULL )  E.GetStack()->Pop();
          return true;
        }
        Cmd.Delete(fstart, fend-fstart);
        Function->Run(Params, E);
        if( !E.IsSuccessful() )  return false;
        Cmd.Insert(E.GetRetVal(), fstart);
        i = fstart + E.GetRetVal().Length();
      }
      if( i >= Cmd.Length() )  {
        if( E.GetStack() != NULL )  E.GetStack()->Pop();
        return true;
      }
      olxch ch = Cmd.CharAt(i);
      if( ch == ' ' || ch == '+' || ch == '-' || ch == '/' ||
        ch == '*' || ch == '~' || ch == '&' || ch == '\'' ||
        ch == '\\' || ch == '|' || ch == '!' || ch == '"' ||
        ch == '$' || ch == '%' || ch == '^' || ch == '#'  ||
        ch == '=' || ch == '[' || ch == ']' || ch == '{' ||
        ch == '}' || ch == ':' || ch == ';' || ch == '?' ||
        ch == '(' || ch == ')')
      {
        Func = EmptyString;
        fstart = i+1;
        Params.Clear();
        ArgV = EmptyString;
        continue;
      }
      Func << ch;
    }
    if( E.GetStack() != NULL )  E.GetStack()->Pop();
    return true;
  }
  //..............................................................................
  void ProcessMacro(const olxstr& Cmd, TMacroError& Error, bool ClearMacroError=true)  {
    if( ClearMacroError )  Error.Reset();
    if( Cmd.IsEmpty() )  return;
    
    if( Error.GetStack() != NULL )  Error.GetStack()->Push(Cmd);

    TStrObjList Cmds;
    TStrList MCmds;
    TParamList Options;
    olxstr Command(Cmd);
    // processing environment variables
    int ind = Command.FirstIndexOf('|'), ind1;
    while( ind >= 0 )  {
      if( ind+1 >= Command.Length() )  break;
      ind1 = Command.FirstIndexOf('|', ind+1);
      if( ind1 == -1 )  break;
      if( ind1 == ind+1 )  { // %%
        Command.Delete(ind1, 1);
        ind = Command.FirstIndexOf('|', ind1);
        continue;
      }
      CString envn( Command.SubString(ind+1, ind1-ind-1) );
      char* eval = getenv( envn.c_str() );
      if( eval != NULL )  {
        Command.Delete( ind, ind1-ind+1);
        Command.Insert(eval, ind );
        ind1 = ind + olxstr::o_strlen(eval);
      }
      else  // variable is not found - leave as it is
        ind1 = ind + envn.Length();

      if( ind1+1 >= Command.Length() )  break;
      ind = Command.FirstIndexOf('|', ind1+1);
    }
    // end processing environment variables
    // special treatment of pyhton commands
    TParamList::StrtokParams(Command, ' ', Cmds);
    //  CommandCS = Cmds[0];
    //  Command = CommandCS.LowerCase();
    Command = Cmds[0];
    Cmds.Delete(0);
    for( int i = 0; i < Cmds.Count(); i++ )  {
      if( Cmds[i].IsEmpty() )  continue;
      if( Cmds[i].CharAt(0) == '-' && !Cmds[i].IsNumber() )  {  // an option
        if( Cmds[i].Length() > 1 &&
          ((Cmds[i].CharAt(1) >= '0' && Cmds[i].CharAt(1) <= '9') || Cmds[i].CharAt(1) == '-') )  // cannot start from number
          continue;
        if( Cmds[i].Length() > 1 )  {
          Options.FromString(Cmds[i].SubStringFrom(1), '=');
          // 18.04.07 added - !!!
          if( !ProcessMacroFunc( Options.Value(Options.Count()-1), Error ) )
            return;
        }
        Cmds.Delete(i);  
        i--;
        continue;
      }
      else if( Cmds[i].Length() > 1 && Cmds[i].CharAt(0) == '\\' && Cmds[i].CharAt(1) == '-' )
        Cmds[i] = Cmds[i].SubStringFrom(1);
    }
    ABasicFunction *MF = OlexProcessor.GetLibrary().FindMacro(Command);//, Cmds.Count());
    if( MF != NULL )  {
      if( Command.Comparei("if") == 0 )  {
        MF->Run(Cmds, Options, Error);
        if( Error.IsSuccessful() && Error.GetStack() != NULL )  
          Error.GetStack()->Pop();
        return;
      }
      for( int i=0; i < Cmds.Count(); i++ )  {
        if( !ProcessMacroFunc(Cmds[i], Error) )  {
          if( !Error.DoesFunctionExist() )  {
            Error.Reset();
            continue;
          }
          else
            return;
        }
      }
      MF->Run(Cmds, Options, Error);
      if( Error.IsSuccessful() && Error.GetStack() != NULL )  Error.GetStack()->Pop();
      return;
    }
    //..............................................................................
    // macro processing
    TEMacro* macro = FindMacro( Command );
    if( macro == NULL )  {  // macro does not exist
      if( Command.FirstIndexOf('(') != -1 )  {
        if( !ProcessMacroFunc(Command, Error) )
          return;
      }
      else  {
        // 2009.02.03 commented - cutom macros might not exist, so just keep quiet
        //Error.NonexitingMacroError( Command );
        TBasicApp::GetLog().Warning( olxstr("Non-existing macro: ") << Command );
        return;
      }
      return;
    }
    TStrList onAbort, onTerminate, onListen;
    macro->ListCommands(Cmds, MCmds, onAbort, onListen, onTerminate, OlexProcessor);
    for( int i=0; i < MCmds.Count(); i++ )  {
      if( MCmds[i].IsEmpty() ) continue;
      ProcessMacro(MCmds[i], Error);
      if( !Error.IsSuccessful() )  {
        TMacroError E;
        if( Error.GetStack() != NULL && !onAbort.IsEmpty() )  {
          Error.GetStack()->Push("Aborting last call...");
          E.SetStack( *Error.GetStack() );
        }
        for( int j=0; j < onAbort.Count(); j++ )  {
          ProcessMacro(onAbort[j], E);
          if( !E.IsSuccessful() )
            break;
        }
        if( Error.GetStack() != NULL && !onAbort.IsEmpty() )
          Error.GetStack()->Push("Aborting done...");
        break;
      }
    }
    if( Error.IsSuccessful() && Error.GetStack() != NULL )  Error.GetStack()->Pop();
  }
};
EndEsdlNamespace()
#endif