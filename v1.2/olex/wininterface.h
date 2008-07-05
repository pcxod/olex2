#ifndef _WinInterfaceH
#define _WinInterfaceH

#include <wx/wx.h>
#include "gldefs.h"


class TWindowInterface
{
  wxWindow *FWindow;
public:
  TWindowInterface(wxWindow *W){  FWindow = W; }

  inline int GetWidth()   const { return FWindow->GetRect().width; };
  inline void SetWidth(int w)   { FWindow->SetSize(-1, -1, w, -1, wxSIZE_USE_EXISTING); };
  inline int GetHeight()  const { return FWindow->GetRect().height; };
  inline void SetHeight(int h)  { FWindow->SetSize(-1, -1, -1, h, wxSIZE_USE_EXISTING); };

  inline int GetLeft()    const { return FWindow->GetRect().x; };
  inline void SetLeft(int l)    { FWindow->SetSize(l, -1, -1, -1, wxSIZE_USE_EXISTING); };
  inline int GetTop()     const { return FWindow->GetRect().y; };
  inline void SetTop(int r)     { FWindow->SetSize(-1, r, -1, -1, wxSIZE_USE_EXISTING); };
  int GetColor()  const  {
    wxColour c = FWindow->GetBackgroundColour();
    return RGB(c.Red(), c.Green(), c.Blue());
  }
  inline void SetColor(int c)  {
    FWindow->SetBackgroundColour(wxColour(GetRValue(c), GetGValue(c), GetBValue(c)));
    FWindow->Refresh();
  }
  inline void SetColor(wxColor c)  {  FWindow->SetBackgroundColour(c);  FWindow->Refresh();  }
  inline bool IsVisible()    const {  return FWindow->IsShown();  }
  inline void SetVisible(bool v)   {  if( v )  FWindow->Show();  else  FWindow->Hide();  }

  inline long GetWindowStyle()  const {   return FWindow->GetWindowStyleFlag();  }
  inline void SetWindowStyle(long v)  { FWindow->SetWindowStyleFlag(v);  }
  inline void AddWindowStyle(long v)  { FWindow->SetWindowStyleFlag(GetWindowStyle() | v);  }
  void DelWindowStyle(long v)  {
    if( (GetWindowStyle() & v) != 0 )
      FWindow->SetWindowStyleFlag(GetWindowStyle() ^ v);
  }
  inline bool HasWindowStyle(long v) const {  return (GetWindowStyle() & v) != 0;  }

  inline long GetExtraStyle()   const {   return FWindow->GetExtraStyle();  }
  inline void SetExtraStyle(long v)   { FWindow->SetExtraStyle(v);  }
  inline void AddExtraStyle(long v)   { FWindow->SetExtraStyle(GetWindowStyle() | v);  }
  void DelExtraStyle(long v)   {
    if( (GetExtraStyle() & v) != 0 )
      SetExtraStyle(GetExtraStyle() ^ v);
  }
  inline bool HasExtraStyle(long v) const {  return (GetExtraStyle() & v) != 0;  }
};

#endif

