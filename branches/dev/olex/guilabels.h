#ifndef __olx_gui_labels_H
#define __olx_gui_labels_H
#include "ctrls.h"

class GuiLabels;

class GuiLabel : public wxStaticText  {
  void OnMouseDblClick(wxMouseEvent& event);
  void OnMouseDown(wxMouseEvent& event);
  void OnMouseUp(wxMouseEvent& event);
  void OnMouseMotion(wxMouseEvent& event);
  int FMouseX, FMouseY;
  bool FMouseDown;
  GuiLabels& Parent;
public:
  GuiLabel(GuiLabels& Parent, wxWindow* wnd, int x, int y);

  DECLARE_EVENT_TABLE()
};
class GuiLabels  {
  TPtrList<GuiLabel> Labels;
public:
  GuiLabel& NewLabel(wxWindow* parent, const olxstr& text, int x, int y);
  inline int Count() const {  return Labels.Count();  }
  GuiLabel& operator [] (int i) const {  return *Labels[i];  }
};

#endif