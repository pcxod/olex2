#ifndef __olx_gui_labels_H
#define __olx_gui_labels_H
#include "ctrls.h"

class GuiLabels;

class GuiLabel : public wxStaticText  {
  void OnMouseDblClick(wxMouseEvent& event);
  void OnMouseDown(wxMouseEvent& event);
  void OnMouseUp(wxMouseEvent& event);
  void OnMouseMotion(wxMouseEvent& event);
  GuiLabels& Parent;
public:
  GuiLabel(GuiLabels& Parent, wxWindow* wnd, int x, int y);

  DECLARE_EVENT_TABLE()
};
class GuiLabels  {
  TPtrList<GuiLabel> Labels;
  GuiLabel* active;
  int FMouseX, FMouseY;
public:
  GuiLabels();
  GuiLabel& NewLabel(wxWindow* parent, const olxstr& text, int x, int y);
  inline int Count() const {  return Labels.Count();  }
  GuiLabel& operator [] (int i) const {  return *Labels[i];  }

  bool OnMouseDblClick(wxMouseEvent& event, GuiLabel* sender=NULL);
  bool OnMouseDown(wxMouseEvent& event, GuiLabel* sender=NULL);
  bool OnMouseUp(wxMouseEvent& event, GuiLabel* sender=NULL);
  bool OnMouseMotion(wxMouseEvent& event, GuiLabel* sender=NULL);
};

#endif