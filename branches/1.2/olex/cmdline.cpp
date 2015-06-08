/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "cmdline.h"
#include "glmouse.h"
#include "bapp.h"

TCmdLine::TCmdLine(wxWindow* parent, int flags) :
  TTextEdit(parent, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, flags),
  OnCommand(Actions.New("ONCOMMAND"))
{
  PromptStr = ">>";
  SetText(PromptStr);
  SetInsertionPointEnd();
  CmdIndex = 0;
}
//..............................................................................
TCmdLine::~TCmdLine()  {}
//..............................................................................
bool TCmdLine::ProcessKey(wxKeyEvent& evt)  {
  if( (evt.GetKeyCode() == WXK_LEFT) )  {
    if (evt.GetModifiers() == 0) {
      if( GetInsertionPoint() <= (long)PromptStr.Length() )
        return true;
    }
    else if (evt.ControlDown())
      return !GetText().SubStringTo(GetInsertionPoint())
        .TrimWhiteChars(true).Contains(' ');
  }
  else if( (evt.GetKeyCode() == WXK_BACK) )  {
    if( GetInsertionPoint() <= (long)PromptStr.Length() )
      return true;
  }
  else if( (evt.GetKeyCode() == WXK_UP) )  {
    if( --CmdIndex < 0 )
      CmdIndex = 0;
    if( CmdIndex < (int)Commands.Count() )
      SetCommand(Commands[CmdIndex]);
    return true;
  }
  else if( (evt.GetKeyCode() == WXK_DOWN) )  {
    if( ++CmdIndex >= (int)Commands.Count() )  {
      CmdIndex = Commands.Count()-1;
    }
    if( CmdIndex < (int)Commands.Count() && Commands.Count() != 0 )
      SetCommand(Commands[CmdIndex]);
    return true;
  }
  else if( (evt.GetKeyCode() == WXK_RETURN) && evt.GetModifiers() == 0 )  {
    olxstr cmd = GetCommand();
    if( !Commands.IsEmpty() && (Commands.GetLastString() == cmd ) )
      ;
    else
      Commands.Add(GetCommand());
    CmdIndex = Commands.Count();
    OnCommand.Execute(dynamic_cast<IOlxObject*>((AActionHandler*)this) );
    return true;
  }
  else if( (evt.GetKeyCode() == WXK_ESCAPE) && evt.GetModifiers() == 0 )  {
    SetCommand(EmptyString());
    return true;
  }
  else if( (evt.GetKeyCode() == WXK_HOME) && evt.GetModifiers() == 0 )  {
    SetInsertionPoint(PromptStr.Length());
    return true;
  }
  else if( (evt.GetKeyCode() == WXK_UP) && evt.ControlDown() )  {
    SetInsertionPoint(PromptStr.Length());
    return true;
  }
  return false;
}
//..............................................................................
void TCmdLine::SetCommand(const olxstr& cmd)  {
  SetText(PromptStr + cmd);
  SetInsertionPointEnd();
}
//..............................................................................
const olxstr& TCmdLine::GetLastCommand(const olxstr &name) const {
  for (size_t i=Commands.Count()-1; i != InvalidIndex; i--) {
    if (Commands[i].StartsFromi(name) &&
        Commands[i].Length() > name.Length() &&
        Commands[i][name.Length()] == ' ')
    {
      return Commands[i];
    }
  }
  return EmptyString();
}
//..............................................................................
bool TCmdLine::WillProcessKey(wxKeyEvent& evt) {
  if (evt.HasModifiers()) return false;
  olxstr tx = GetText();
  if (evt.GetKeyCode() == WXK_DELETE)
    return GetInsertionPoint() < (long)tx.Length();
  else if (evt.GetKeyCode() == WXK_BACK)
    return GetInsertionPoint() >= (long)PromptStr.Length();
  return false;
}
//..............................................................................
