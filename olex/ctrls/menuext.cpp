/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "menuext.h"
#include "frameext.h"
#include "olxstate.h"

using namespace ctrl_ext;
IMPLEMENT_CLASS(TMenu, wxMenu)
IMPLEMENT_CLASS(TMenuItem, wxMenuItem)

TMenu *TMenu::CopyMenu(const wxMenu &menu)  {
  TMenu *M = new TMenu;
  for( unsigned int i=0; i < menu.GetMenuItemCount(); i++ )  {
    wxMenuItem *mi = menu.FindItemByPosition(i);
    if( mi->GetSubMenu() != NULL )  {
      TMenu *nm = CopyMenu(*mi->GetSubMenu());
      int miId = mi->GetId();
      wxItemKind miKind = mi->GetKind();
      const wxString &miText = mi->GetText();
      const wxString &miHelpStr = mi->GetHelp();
      M->Append(miId, miText, nm, miHelpStr);
    }
    else  {
      int miId = mi->GetId();
      wxItemKind miKind = mi->GetKind();
      const wxString &miText = mi->GetText();
      const wxString &miHelpStr = mi->GetHelp();
      wxMenuItem *nmi = new wxMenuItem(M, miId, miText, miHelpStr, miKind);
      M->Append(nmi);
    }
  }
  return M;
}
//..............................................................................
void TMenu::Clear()  {
  TPtrList<wxMenuItem> menues;
  const size_t count = GetMenuItemCount();
  menues.SetCapacity(count+1);
  for( size_t i=0; i < count; i++ )
    menues.Add(FindItemByPosition(i));
  for( size_t i=0; i < count; i++ )
    Destroy(menues[i]);
}
//----------------------------------------------------------------------------//
// TMenuItem implementation
//----------------------------------------------------------------------------//
TMenuItem::~TMenuItem()  {}
//..............................................................................
void TMenuItem::SetActionQueue(TActionQueue& q, const olxstr& dependMode,
  short dependentOn)
{
  const size_t cmdind = dependMode.FirstIndexOf(';');
  if( cmdind != InvalidIndex )  {
    DependMode = dependMode.SubStringTo(cmdind);
    OnModeChangeCmd = dependMode.SubStringFrom(cmdind+1);
  }
  else
    DependMode = dependMode;
  DependentOn = dependentOn;
  ActionQueue = &q;
  ActionQueue->Add(this);
}
//..............................................................................
void TMenuItem::ValidateState()  {
  if( DependentOn != 0 && IsCheckable() && !DependMode.IsEmpty() )  {
    if( DependentOn == ModeDependent )
      Check(TModeRegistry::CheckMode(DependMode));
    else if( DependentOn == StateDependent ) {
      Check(TStateRegistry::GetInstance().CheckState(
        DependMode, OnModeChangeCmd));
    }
  }
}
//..............................................................................
bool TMenuItem::Execute(const IOlxObject *Sender, const IOlxObject *Data,
  TActionQueue *)
{
  if( Data && EsdlInstanceOf(*Data, TModeChange) )  {
    const TModeChange* mc = (const TModeChange*)Data;
    if( this->IsCheckable() )
      Check(TModeRegistry::CheckMode(DependMode));
  }
  if( Data && EsdlInstanceOf(*Data, TStateChange) )  {
    TStateChange* sc = (TStateChange*)Data;
    if( this->IsCheckable() ) {
      Check(
        TStateRegistry::GetInstance().CheckState(DependMode, OnModeChangeCmd));
    }
  }
  return true;
}
