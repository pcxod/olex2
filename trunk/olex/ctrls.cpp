//----------------------------------------------------------------------------//
// controls implementation
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif
#include "ctrls.h"
#include <wx/filedlg.h>
#include <wx/tooltip.h>
//#include <wx/mstream.h>
#include "efile.h"
#include "bapp.h"

#include "mainform.h"

//#include "fsext.h"
#include "html/htmlext.h"

#include "obase.h"

#include "xglapp.h"
#include "etime.h"

IMPLEMENT_CLASS(TMainFrame, wxFrame)
IMPLEMENT_CLASS(TDialog, wxDialog)
IMPLEMENT_CLASS(TTimer, wxTimer)

//----------------------------------------------------------------------------//
// TMainFrame implementation
//----------------------------------------------------------------------------//
TMainFrame::TMainFrame(const wxString& title, const wxPoint& pos, const wxSize& size, const wxString &ClassName)
: wxFrame((wxFrame*)NULL, wxID_ANY, title, pos, size, wxDEFAULT_FRAME_STYLE),
  AOlxCtrl(this)  {}
TMainFrame::~TMainFrame()  {
  for( int i=0; i < WindowPos.Count(); i++ )
    delete WindowPos.GetObject(i);
}
//..............................................................................
void TMainFrame::RestorePosition(wxWindow *Window)  {  // restores previously saved position
  TWindowInfo *wi = WindowPos[Window->GetName().c_str()];
  if( wi != NULL )
    Window->Move(wi->x, wi->y);
}
//..............................................................................
void TMainFrame::SavePosition(wxWindow *Window)  {  //saves current position of the window on screen
  TWindowInfo *wi = WindowPos[Window->GetName().c_str()];
  if( wi == NULL )  {
    wi = new TWindowInfo;
    WindowPos.Add(Window->GetName().c_str(), wi);
  }
  Window->GetPosition(&(wi->x), &(wi->y));
}
//..............................................................................
olxstr TMainFrame::PortableFilter(const olxstr& filter)  {
#if defined(__WIN32__) || defined(__MAC__)
  return filter;
#else
  olxstr rv;
  TStrList fitems(filter, '|');
  for( int i=0; i < fitems.Count(); i+=2 )  {
    if( i+1 >= fitems.Count() )
      break;
    if( i != 0 )
      rv << '|';
    rv << fitems[i] << '|';
    TStrList masks(fitems[i+1], ';');
    for( int j=0; j < masks.Count(); j++ )  {
      int di = masks[j].LastIndexOf('.');
      if( di == -1 )  {
        rv << masks[j];
        if( j+1 < masks.Count() )
          rv << ';';
        continue;
      }
      rv << masks[j].SubStringTo(di+1);
      for( int k=di+1; k < masks[j].Length(); k++ )
        rv << '[' << olxstr::o_tolower(masks[j].CharAt(k)) << olxstr::o_toupper(masks[j].CharAt(k)) << ']';
      if( j+1 < masks.Count() )
        rv << ';';
    }
  }
  return rv;
#endif
}
//..............................................................................
olxstr TMainFrame::PickFile(const olxstr &Caption, const olxstr &Filter,
                              const olxstr &DefFolder, bool Open)  {
  olxstr FN;
  int Style;
  if( Open )  Style = wxFD_OPEN;
  else        Style = wxFD_SAVE;
  wxFileDialog *dlgFile = new wxFileDialog( this, Caption.u_str(), DefFolder.u_str(), wxString(),
    PortableFilter(Filter).u_str(), Style);
  if( dlgFile->ShowModal() ==  wxID_OK )
    FN = dlgFile->GetPath().c_str();
  delete dlgFile;
  return FN;
}
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

