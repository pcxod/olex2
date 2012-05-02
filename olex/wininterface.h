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

  inline int GetWidth()   const { return Window->GetRect().width; };
  inline void SetWidth(int w)   { Window->SetSize(-1, -1, w, -1, wxSIZE_USE_EXISTING); };
  inline int GetHeight()  const { return Window->GetRect().height; };
  inline void SetHeight(int h)  { Window->SetSize(-1, -1, -1, h, wxSIZE_USE_EXISTING); };

  inline int GetLeft()    const { return Window->GetRect().x; };
  inline void SetLeft(int l)    { Window->SetSize(l, -1, -1, -1, wxSIZE_USE_EXISTING); };
  inline int GetTop()     const { return Window->GetRect().y; };
  inline void SetTop(int r)     { Window->SetSize(-1, r, -1, -1, wxSIZE_USE_EXISTING); };
  uint32_t GetColor() const {
    wxColour c = Window->GetBackgroundColour();
    return RGB(c.Red(), c.Green(), c.Blue());
  }
  inline void SetColor(uint32_t c)  {
    Window->SetBackgroundColour(wxColour(GetRValue(c), GetGValue(c), GetBValue(c)));
    Window->Refresh();
  }
  inline void SetColor(wxColor c)  {  Window->SetBackgroundColour(c);  Window->Refresh();  }
  inline bool IsVisible()    const {  return Window->IsShown();  }
  inline void SetVisible(bool v)   {  
    if( v )  
      Window->Show();  
    else  
      Window->Hide();  
  }

  inline long GetWindowStyle()  const {   return Window->GetWindowStyleFlag();  }
  inline void SetWindowStyle(long v)  { Window->SetWindowStyleFlag(v);  }
  inline void AddWindowStyle(long v)  { Window->SetWindowStyleFlag(GetWindowStyle() | v);  }
  void DelWindowStyle(long v)  {
    Window->SetWindowStyleFlag(GetWindowStyle() & ~v);
  }
  inline bool HasWindowStyle(long v) const {  return (GetWindowStyle() & v) != 0;  }

  inline long GetExtraStyle()   const {   return Window->GetExtraStyle();  }
  inline void SetExtraStyle(long v)   { Window->SetExtraStyle(v);  }
  inline void AddExtraStyle(long v)   { Window->SetExtraStyle(GetWindowStyle() | v);  }
  void DelExtraStyle(long v)   {
    SetExtraStyle(GetExtraStyle() & ~v);
  }
  inline bool HasExtraStyle(long v) const {  return (GetExtraStyle() & v) != 0;  }
};

#endif
