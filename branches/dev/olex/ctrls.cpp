//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004-2009
//----------------------------------------------------------------------------//
#include "ctrls.h"

IMPLEMENT_CLASS(TDialog, wxDialog)
IMPLEMENT_CLASS(TTimer, wxTimer)

//----------------------------------------------------------------------------//
// TDialog implementation
//----------------------------------------------------------------------------//
TDialog::TDialog(TMainFrame *Parent, const wxString &Title, const wxString &ClassName) :
  wxDialog(Parent, -1,  Title, wxPoint(0, 0), wxSize(425, 274), wxRESIZE_BORDER | wxDEFAULT_DIALOG_STYLE, ClassName),
  AOlxCtrl(this),
  Parent(Parent)
{
  if( Parent != NULL )
    Parent->RestorePosition(this);
}
TDialog::~TDialog()  {
  if( Parent != NULL )
    Parent->SavePosition(this);
}

