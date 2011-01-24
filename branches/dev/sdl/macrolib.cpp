#include <stdlib.h>
#include "macrolib.h"
#include "exparse/exptree.h"

using namespace exparse::parser_util;

void TEMacroLib::Init()  {
  TLibrary &lib = OlexProcessor.GetLibrary();
  lib.RegisterFunction<TEMacroLib>(new TFunction<TEMacroLib>(this,  &TEMacroLib::funAnd, "And", fpAny^(fpNone|fpOne),
    "Logical 'and' function"));
  lib.RegisterFunction<TEMacroLib>(new TFunction<TEMacroLib>(this,  &TEMacroLib::funOr, "Or", fpAny^(fpNone|fpOne),
    "Logical 'or' function"));
  lib.RegisterFunction<TEMacroLib>(new TFunction<TEMacroLib>(this,  &TEMacroLib::funNot, "Not", fpOne,
    "Logical 'not' function"));
  lib.RegisterFunction<TEMacroLib>(new TFunction<TEMacroLib>(this,  &TEMacroLib::funLastError, "LastError", fpNone,
    "Returns last error"));
  lib.RegisterFunction<TEMacroLib>(new TFunction<TEMacroLib>(this,  &TEMacroLib::funLogLevel, "LogLevel", fpNone|fpOne,
    "Returns/sets log level, default is 'm' - for macro, accepts/returns 'm', 'mf' or 'f'"));
  lib.RegisterMacro<TEMacroLib>(new TMacro<TEMacroLib>(this,  &TEMacroLib::macIF, "IF", EmptyString(), fpAny^fpNone,
    "'if' construct"));
}
//..............................................................................................
bool TEMacroLib::ProcessFunction(olxstr& Cmd, TMacroError& E, bool has_owner)  {
  if( Cmd.IndexOf('(') == InvalidIndex )  return true;
  E.GetStack().Push(Cmd);
  size_t specialFunctionIndex = Cmd.IndexOf('$');
  while( specialFunctionIndex != InvalidIndex && is_escaped(Cmd, specialFunctionIndex) )  {
    Cmd.Delete(specialFunctionIndex-1, 1);
    specialFunctionIndex = Cmd.FirstIndexOf('$', specialFunctionIndex);
  }
  if( specialFunctionIndex != InvalidIndex )  {
    size_t i=specialFunctionIndex;
    int bc = 0;
    bool funcStarted = false;
    while( ++i < Cmd.Length() )  {
      if( Cmd.CharAt(i) == '(' )  {  bc++;  funcStarted = true;  }
      else if( Cmd.CharAt(i) == ')' )  bc--;
      else if( !funcStarted && !is_allowed_in_name(Cmd.CharAt(i)) )  {
        specialFunctionIndex = next_unescaped('$', Cmd, i);
        if( specialFunctionIndex == InvalidIndex )  {
          E.GetStack().Pop();
          return true;
        }
        i = specialFunctionIndex;
        funcStarted = false;
        continue;
      }
      if( bc == 0 && funcStarted )  {
        olxstr spFunction = Cmd.SubString(specialFunctionIndex+1, i-specialFunctionIndex);
        if( ProcessFunction(spFunction, E, false) )  {
          Cmd.Delete(specialFunctionIndex, i - specialFunctionIndex + 1);
          Cmd.Insert(spFunction, specialFunctionIndex);
        }
        else
          Cmd.Delete(specialFunctionIndex, i - specialFunctionIndex + 1);
        specialFunctionIndex = next_unescaped('$', Cmd, 0);
        if( specialFunctionIndex == InvalidIndex )  {
          E.GetStack().Pop();
          return true;
        }
        i = specialFunctionIndex;
        funcStarted = false;
        continue;
      }
    }
    return false;
  }

  int bc=0;
  size_t fstart=0;
  for( size_t i=0; i < Cmd.Length(); i++ )  {
    if( Cmd.CharAt(i) == '(' && i != fstart)  {
      if( Cmd.CharAt(i-1) =='.' )  // ffff.(
        continue;
      const olxstr func_name = Cmd.SubString(fstart, i-fstart);
      bc++;
      size_t aend = i;
      if( !skip_brackets(Cmd, aend) )  {
        E.ProcessingError(__OlxSourceInfo, olxstr("Number of brackets does not match: ") << Cmd);
        return false;
      }
      olxstr ArgV = Cmd.SubString(i+1, aend-i-1);
      TStrObjList Params;
      if( !ArgV.IsEmpty() )  { // arecursive call to all inner functions
        TParamList::StrtokParams(ArgV, ',', Params);
        // evaluation will be called from within the functions
        if( func_name.Comparei("or") != 0 && func_name.Comparei("and") != 0 )  {
          olxstr localArg;
          for( size_t j=0; j < Params.Count(); j++ )  {
            if( !ProcessFunction(Params[j], E, true) )  {
              if( func_name.Equalsi("eval") ) // put the function back
                Params[j] = ArgV;
              else  {
                TBasicApp::NewLogEntry(logInfo) << "Possibly incorrect argument: " << Params[j];
                E.GetStack().Pop();  // clear the error
                E.ClearErrorFlag();
              }
            }
          }
        }
        if( !Params.IsEmpty() )  
          ArgV = Params[0];
      }
      if( func_name.IsEmpty() )  { // in case arithmetic ()
        Cmd.Delete(fstart+1, aend-fstart-2);  // have to leave ()
        Cmd.Insert(ArgV, fstart+1);
        E.GetStack().Pop();
        return true;
      }
      else if( !olxstr::o_isalpha(func_name.CharAt(0)) )  {
        E.GetStack().Pop();
        return true;
      }
      ABasicFunction *Function = OlexProcessor.GetLibrary().FindFunction(func_name);//, Params.Count() );
      if( Function == NULL )  {
        if( !func_name.IsEmpty() && !has_owner )  {
          E.NonexitingMacroError(func_name);
          return false;
        }
        E.GetStack().Pop();
        return true;
      }
      Cmd.Delete(fstart, aend-fstart+1);
      Function->Run(Params, E);
      if( !E.IsSuccessful() )  {  //&& E.DoesFunctionExist() )  
        if( (GetLogLevel()&macro_log_function) != 0 )
          TBasicApp::NewLogEntry(logInfo) << Function->GetRuntimeSignature() << ": failed";
        return false;
      }
      else  {
        if( (GetLogLevel()&macro_log_function) != 0 )
          TBasicApp::NewLogEntry(logInfo) << Function->GetRuntimeSignature() << ": '" << E.GetRetVal() << '\'';
      }
      Cmd.Insert(E.GetRetVal(), fstart);
      i = fstart + E.GetRetVal().Length();
    }
    if( i >= Cmd.Length() )  {
      E.GetStack().Pop();
      return true;
    }
    if( !is_allowed_in_name(Cmd.CharAt(i)) )  {
      fstart = i+1;
      continue;
    }
  }
  E.GetStack().Pop();
  return true;
}
//..............................................................................................
void TEMacroLib::ProcessMacro(const olxstr& Cmd, TMacroError& Error)  {
  if( Cmd.IsEmpty() )  return;
  if( (GetLogLevel()&macro_log_macro) != 0 )
    TBasicApp::NewLogEntry(logInfo) << Cmd;
  TStrObjList Cmds;
  TStrList MCmds;
  TParamList Options;
  olxstr Command = olxstr(Cmd).TrimWhiteChars();
  if( Command.IsEmpty() )  
    return;

  Error.GetStack().Push(Cmd);
  // processing environment variables
  size_t ind = Command.FirstIndexOf('|');
  while( ind != InvalidIndex )  {
    if( ind+1 >= Command.Length() )  break;
    size_t ind1 = Command.FirstIndexOf('|', ind+1);
    if( ind1 == InvalidIndex )  break;
    if( ind1 == ind+1 )  { // %%
      Command.Delete(ind1, 1);
      ind = Command.FirstIndexOf('|', ind1);
      continue;
    }
    const olxstr var_name = Command.SubString(ind+1, ind1-ind-1);
    const olxstr eval = olx_getenv(var_name);
    if( !eval.IsEmpty() )  {
      Command.Delete(ind, ind1-ind+1);
      Command.Insert(eval, ind);
      ind1 = ind + eval.Length();
    }
    else  // variable is not found - leave as it is
      ind1 = ind + var_name.Length();

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
  for( size_t i = 0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsEmpty() )  continue;
    if( Cmds[i].CharAt(0) == '-' && !Cmds[i].IsNumber() )  {  // an option
      if( Cmds[i].Length() > 1 &&
        ((Cmds[i].CharAt(1) >= '0' && Cmds[i].CharAt(1) <= '9') || Cmds[i].CharAt(1) == '-') )  // cannot start from number
        continue;
      if( Cmds[i].Length() > 1 )  {
        Options.FromString(Cmds[i].SubStringFrom(1), '=');
        // 18.04.07 added - !!!
        if( !ProcessFunction(Options.Value(Options.Count()-1), Error, true) )
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
    if( Command.Equalsi("if") )  {
      MF->Run(Cmds, Options, Error);
      if( Error.IsSuccessful() )  
        Error.GetStack().Pop();
      return;
    }
    for( size_t i=0; i < Cmds.Count(); i++ )  {
      if( !ProcessFunction(Cmds[i], Error, true) )
        return;
    }
    MF->Run(Cmds, Options, Error);
    if( Error.IsSuccessful() )  
      Error.GetStack().Pop();
    return;
  }
  //..... macro processing
  TEMacro* macro = FindMacro(Command);
  if( macro == NULL )  {  // macro does not exist
    if( Command.FirstIndexOf('(') != InvalidIndex )  {
      if( !ProcessFunction(Command, Error, false) )
        return;
      else
        Error.GetStack().Pop();
    }
    else
      Error.NonexitingMacroError(Command);
    return;
  }
  TStrList onAbort, onTerminate, onListen;
  macro->ListCommands(Cmds, MCmds, onAbort, onListen, onTerminate, OlexProcessor);
  for( size_t i=0; i < MCmds.Count(); i++ )  {
    if( MCmds[i].IsEmpty() ) continue;
    ProcessMacro(MCmds[i], Error);
    // if something does not exist - do not hide it by the on_abort block
    if( !Error.DoesFunctionExist() )
      return;
    if( !Error.IsSuccessful() && !onAbort.IsEmpty() )  {
      TBasicApp::NewLogEntry(logInfo) << "OnAbort at " << Error.GetLocation() << ": " << Error.GetInfo();
      Error.ClearErrorFlag();
      for( size_t j=0; j < onAbort.Count(); j++ )  {
        ProcessMacro(onAbort[j], Error);
        if( !Error.IsSuccessful() )
          break;
      }
      break;
    }
  }
  if( Error.IsSuccessful() )  
    Error.GetStack().Pop();
}
//...........................................................................................
void TEMacroLib::ParseMacro(const TDataItem& macro_def, TEMacro& macro)  {
  olxstr Tmp;
  TDataItem* di = macro_def.FindItem("cmd");
  if( di != NULL )  {
    for( size_t i=0; i < di->ItemCount(); i++ )
      if( ExtractItemVal(di->GetItem(i), Tmp) )
        macro.AddCmd( Tmp );
  }
  di = macro_def.FindItem("args");
  if( di != NULL )  {
    for( size_t i=0; i < di->ItemCount(); i++ )  {
      const TDataItem& tdi = di->GetItem(i);
      macro.AddArg(tdi.GetFieldValue("name", EmptyString()), tdi.GetFieldValue("def"));
    }
  }
  di = macro_def.FindItem("onterminate");
  if( di != NULL )  {
    for( size_t i=0; i < di->ItemCount(); i++ )
      if( ExtractItemVal(di->GetItem(i), Tmp) )
        macro.AddOnTerminateCmd( Tmp );
  }
  di = macro_def.FindItem("onlisten");
  if( di != NULL )  {
    for( size_t i=0; i < di->ItemCount(); i++ )
      if( ExtractItemVal(di->GetItem(i), Tmp) )
        macro.AddOnListenCmd( Tmp );
  }
  di = macro_def.FindItem("onabort");
  if( di != NULL )  {
    for( size_t i=0; i < di->ItemCount(); i++ ) 
      if( ExtractItemVal(di->GetItem(i), Tmp) )
        macro.AddOnAbortCmd( Tmp );
  }
}
//...........................................................................................
void TEMacroLib::macIF(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() < 2 || !Cmds[1].Equalsi("then"))  {
    E.ProcessingError(__OlxSrcInfo, "incorrect syntax - two commands or a command and \'then\' are expected" );
    return;
  }
  olxstr Condition = Cmds[0];
  if( !ProcessFunction(Condition, E, false) )  {
    E.ProcessingError(__OlxSrcInfo, "error processing condition" );
    return;
  }
  if( Condition.ToBool() )  {
    if( !Cmds[2].Equalsi("none") )  {
      if( Cmds[2].IndexOf(">>") != InvalidIndex )  {
        TStrList toks(Cmds[2], ">>");
        for( size_t i=0; i < toks.Count(); i++ )  {
          ProcessMacro(toks[i], E);
          if( !E.IsSuccessful() )  return;
        }
      }
      else
        ProcessMacro(Cmds[2], E);
    }
  }
  else  {
    if( Cmds.Count() == 5 )  {
      if( Cmds[3].Equalsi("else") )  {
        if( !Cmds[4].Equalsi("none") )  {
          if( Cmds[4].IndexOf(">>") != InvalidIndex )  {
            TStrList toks(Cmds[4], ">>");
            for( size_t i=0; i < toks.Count(); i++ )  {
              ProcessMacro(toks[i], E);
              if( !E.IsSuccessful() )  return;
            }
          }
          else
            ProcessMacro(Cmds[4], E);
        }
      }
      else  {
        E.ProcessingError(__OlxSrcInfo, "no keyword 'else' found" );
        return;
      }
    }
  }
}
//...........................................................................................
void TEMacroLib::funAnd(const TStrObjList& Params, TMacroError &E) {
  for( size_t i=0; i < Params.Count(); i++ )  {
    olxstr tmp = Params[i];
    if( !ProcessFunction(tmp, E, false) )  {
      E.ProcessingError(__OlxSrcInfo, "could not process: ") << tmp;
      return;
    }
    if( !tmp.ToBool() )  {
      E.SetRetVal(false);
      return;
    }
  }
  E.SetRetVal(true);
}
//..............................................................................
void TEMacroLib::funOr(const TStrObjList& Params, TMacroError &E) {
  for( size_t i=0; i < Params.Count(); i++ )  {
    olxstr tmp = Params[i];
    if( !ProcessFunction(tmp, E, false) )  {
      E.ProcessingError(__OlxSrcInfo, "could not process: ") << tmp;
      return;
    }
    if( tmp.ToBool() )  {
      E.SetRetVal(true);
      return;
    }
  }
  E.SetRetVal(false);
}
//..............................................................................
void TEMacroLib::funNot(const TStrObjList& Params, TMacroError &E) {
  olxstr tmp = Params[0];
  if( !ProcessFunction(tmp, E, false) )  {
    E.ProcessingError(__OlxSrcInfo, "could not process: ") << tmp;
    return;
  }
  E.SetRetVal(!tmp.ToBool());
}
//..............................................................................
void TEMacroLib::funLastError(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal(E.GetInfo());
}
//..............................................................................
void TEMacroLib::funLogLevel(const TStrObjList& Params, TMacroError &E) {
  if( Params.IsEmpty() )  {
    olxstr ll;
    if( (GetLogLevel()&macro_log_macro) != 0 )  ll << 'm';
    if( (GetLogLevel()&macro_log_function) != 0 )  ll << 'f';
    E.SetRetVal(ll);
  }
  else  {
    uint8_t ll = 0;
    if( Params[0].IndexOfi('m') != InvalidIndex )  ll |= macro_log_macro;
    if( Params[0].IndexOfi('f') != InvalidIndex )  ll |= macro_log_function;
    SetLogLevel(ll);
  }
}
//..............................................................................
