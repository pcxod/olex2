#include <stdlib.h>
#include "macrolib.h"

bool TEMacroLib::ProcessFunction(olxstr& Cmd, TMacroError& E, bool has_owner)  {
  if( Cmd.IndexOf('(') == -1 )  return true;
  E.GetStack().Push(Cmd);
  int specialFunctionIndex = Cmd.IndexOf('$');
  if( specialFunctionIndex != -1 )  {
    int i=specialFunctionIndex, bc = 0;
    bool funcStarted = false;
    while( i++ < Cmd.Length() )  {
      if( Cmd.CharAt(i) == '(' )  {  bc++;  funcStarted = true;  }
      else if( Cmd.CharAt(i) == ')' )  bc--;
      else if( !funcStarted && !is_allowed_in_name(Cmd.CharAt(i)) )  {
        if( i+1 >= Cmd.Length() )  {
          E.GetStack().Pop();
          return true;
        }
        specialFunctionIndex = Cmd.FirstIndexOf('$', i+1);
        if( specialFunctionIndex == -1 )  {
          E.GetStack().Pop();
          return true;
        }
      }
      if( bc == 0 && funcStarted )  {
        olxstr spFunction = Cmd.SubString(specialFunctionIndex+1, i-specialFunctionIndex);
        if( ProcessFunction(spFunction, E, false) )  {
          Cmd.Delete(specialFunctionIndex, i - specialFunctionIndex + 1);
          Cmd.Insert(spFunction, specialFunctionIndex);
        }
        else  {
          Cmd.Delete(specialFunctionIndex, i - specialFunctionIndex + 1);
        }
        specialFunctionIndex = Cmd.IndexOf('$');
        if( specialFunctionIndex == -1 )  {
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

  int bc=0, fstart=0, astart = 0, aend = 0;
  for( int i=0; i < Cmd.Length(); i++ )  {
    if( Cmd.CharAt(i) == '(' && i != fstart)  {
      if( Cmd.CharAt(i-1) =='.' )  // ffff.(
        continue;
      const olxstr func_name = Cmd.SubString(fstart, i-fstart);
      bc++;
      i++;
      int fend = -1;
      astart = aend = i;
      while( i < Cmd.Length() )  {
        if( Cmd.CharAt(i) == '(' )  bc++;
        else if( Cmd.CharAt(i) == ')' && (--bc == 0))  {
          fend = i+1;  
          break; 
        }
        i++;
        aend++;
      }
      olxstr ArgV = Cmd.SubString(astart, aend-astart);
      TStrObjList Params;
      if( !ArgV.IsEmpty() )  { // arecursive call to all inner functions
        TParamList::StrtokParams(ArgV, ',', Params);
        // evaluation will be called from within the functions
        if( func_name.Comparei("or") != 0 && func_name.Comparei("and") != 0 )  {
          olxstr localArg;
          for( int j=0; j < Params.Count(); j++ )  {
            if( !ProcessFunction(Params[j], E, true) )  {
              if( func_name.Equalsi("eval") ) // put the function back
                Params[j] = ArgV;
              else  return false;
            }
          }
        }
        if( !Params.IsEmpty() )  
          ArgV = Params[0];
      }
      if( fend == -1 )  {
        E.ProcessingError(__OlxSourceInfo, olxstr("Number of brackets does not match: ") << Cmd);
        return false;
      }
      if( func_name.IsEmpty() )  {  // in case arithmetic ()
        Cmd.Delete(fstart+1, fend-fstart-2);  // have to leave ()
        Cmd.Insert(ArgV, fstart+1);
        E.GetStack().Pop();
        return true;
      }
      ABasicFunction *Function = OlexProcessor.GetLibrary().FindFunction(func_name);//, Params.Count() );
      if( Function == NULL )  {
        if( !func_name.IsEmpty() && !has_owner )  {
          E.NonexitingMacroError( func_name );
          return false;
        }
        E.GetStack().Pop();
        return true;
      }
      Cmd.Delete(fstart, fend-fstart);
      //TBasicApp::GetLog().Info( Function->GetRuntimeSignature() );
      Function->Run(Params, E);
      if( !E.IsSuccessful() ) //&& E.DoesFunctionExist() )  
        return false;
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
  TBasicApp::GetLog().Info(Cmd);
  TStrObjList Cmds;
  TStrList MCmds;
  TParamList Options;
  olxstr Command = olxstr(Cmd).TrimWhiteChars();
  if( Command.IsEmpty() )  
    return;

  Error.GetStack().Push(Cmd);
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
    for( int i=0; i < Cmds.Count(); i++ )  {
      if( !ProcessFunction(Cmds[i], Error, true) )
        return;
    }
    MF->Run(Cmds, Options, Error);
    if( Error.IsSuccessful() )  
      Error.GetStack().Pop();
    return;
  }
  //..... macro processing
  TEMacro* macro = FindMacro( Command );
  if( macro == NULL )  {  // macro does not exist
    if( Command.FirstIndexOf('(') != -1 )  {
      if( !ProcessFunction(Command, Error, false) )
        return;
      else
        Error.GetStack().Pop();
    }
    else
      Error.NonexitingMacroError( Command );
    return;
  }
  TStrList onAbort, onTerminate, onListen;
  macro->ListCommands(Cmds, MCmds, onAbort, onListen, onTerminate, OlexProcessor);
  for( int i=0; i < MCmds.Count(); i++ )  {
    if( MCmds[i].IsEmpty() ) continue;
    ProcessMacro(MCmds[i], Error);
    // if something does not exist - do not hide it by the on_abort block
    if( !Error.DoesFunctionExist() )
      return;
    if( !Error.IsSuccessful() && !onAbort.IsEmpty() )  {
      TBasicApp::GetLog().Info( olxstr("OnAbort at ") << Error.GetLocation() << ": " << Error.GetInfo() );
      Error.ClearErrorFlag();
      for( int j=0; j < onAbort.Count(); j++ )  {
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
