/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef _WinInterfaceH
#define _WinInterfaceH
#include <wx/wx.h>
#include "ebase.h"
#include "gldefs.h"

class TWindowInterface  {
  wxWindow *Window;
public:
  TWindowInterface(wxWindow *window) : Window(window) {}

  int GetWidth()   const { return Window->GetRect().width; };
  void SetWidth(int w)   { Window->SetSize(-1, -1, w, -1, wxSIZE_USE_EXISTING); };
  int GetHeight()  const { return Window->GetRect().height; };
  void SetHeight(int h)  { Window->SetSize(-1, -1, -1, h, wxSIZE_USE_EXISTING); };

  int GetLeft()    const { return Window->GetRect().x; };
  void SetLeft(int l)    { Window->SetSize(l, -1, -1, -1, wxSIZE_USE_EXISTING); };
  int GetTop()     const { return Window->GetRect().y; };
  void SetTop(int r)     { Window->SetSize(-1, r, -1, -1, wxSIZE_USE_EXISTING); };
  uint32_t GetColor() const {
    wxColour c = Window->GetBackgroundColour();
    return OLX_RGB(c.Red(), c.Green(), c.Blue());
  }
  void SetColor(uint32_t c)  {
    Window->SetBackgroundColour(wxColour(OLX_GetRValue(c), OLX_GetGValue(c),
      OLX_GetBValue(c)));
    Window->Refresh();
  }
  void SetColor(wxColor c)  {  Window->SetBackgroundColour(c);  Window->Refresh();  }
  bool IsVisible()    const {  return Window->IsShown();  }
  void SetVisible(bool v)   {
    if( v )
      Window->Show();
    else
      Window->Hide();
  }

  bool IsEnabled() const { return Window->IsEnabled(); }
  void SetEnabled(bool v) { Window->Enable(v); }


  long GetWindowStyle()  const {   return Window->GetWindowStyleFlag();  }
  void SetWindowStyle(long v)  { Window->SetWindowStyleFlag(v);  }
  void AddWindowStyle(long v)  { Window->SetWindowStyleFlag(GetWindowStyle() | v);  }
  void DelWindowStyle(long v)  {
    Window->SetWindowStyleFlag(GetWindowStyle() & ~v);
  }
  bool HasWindowStyle(long v) const {  return (GetWindowStyle() & v) != 0;  }

  long GetExtraStyle()   const {   return Window->GetExtraStyle();  }
  void SetExtraStyle(long v)   { Window->SetExtraStyle(v);  }
  void AddExtraStyle(long v)   { Window->SetExtraStyle(GetWindowStyle() | v);  }
  void DelExtraStyle(long v)   {
    SetExtraStyle(GetExtraStyle() & ~v);
  }
  bool HasExtraStyle(long v) const {  return (GetExtraStyle() & v) != 0;  }
};

#endif
