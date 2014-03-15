/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "frameext.h"
#include "estrlist.h"
#include "bapp.h"
#include "log.h"

IMPLEMENT_CLASS(TMainFrame, wxFrame)

TMainFrame* TMainFrame::MainFrameInstance = NULL;

// restores previously saved position
void TMainFrame::RestorePosition(wxWindow *Window)  {
  size_t ind = WindowPos.IndexOf(Window->GetName());
  if ( ind == InvalidIndex )  return;
  TWindowInfo &wi = WindowPos.Get(Window->GetName());
  Window->Move(wi.x, wi.y);
}
//..............................................................................
//saves current position of the window on screen
void TMainFrame::SavePosition(wxWindow *Window)  {
  TWindowInfo &wi = WindowPos.Add(Window->GetName());
  Window->GetPosition(&(wi.x), &(wi.y));
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
    for( size_t j=0; j < masks.Count(); j++ )  {
      size_t di = masks[j].LastIndexOf('.');
      if( di == InvalidIndex )  {
        rv << masks[j];
        if( j+1 < masks.Count() )
          rv << ';';
        continue;
      }
      rv << masks[j].SubStringTo(di+1);
      for( size_t k=di+1; k < masks[j].Length(); k++ ) {
        rv << '[' << olxstr::o_tolower(masks[j].CharAt(k)) <<
          olxstr::o_toupper(masks[j].CharAt(k)) << ']';
      }
      if( j+1 < masks.Count() )
        rv << ';';
    }
  }
  return rv;
#endif
}
//..............................................................................
olxstr TMainFrame::PickFile(const olxstr &Caption, const olxstr &Filter,
  const olxstr &DefFolder, const olxstr &DefFile, bool Open)
{
  int Style = Open ? wxFD_OPEN : wxFD_SAVE;
  wxFileDialog dlgFile(this, Caption.u_str(),
    DefFolder.u_str(), DefFile.u_str(), PortableFilter(Filter).u_str(), Style);
  return (dlgFile.ShowModal() == wxID_OK ? olxstr(dlgFile.GetPath())
    : EmptyString());
}
//..............................................................................
int TMainFrame::ShowAlert(const olxstr &msg, const olxstr &title, int flags)  {
  return ::wxMessageBox(msg.u_str(), title.u_str(), flags, MainFrameInstance);
}
//..............................................................................
void TMainFrame::ShowAlert(const TExceptionBase &e, const olxstr &msg, bool log)  {
  TStrList sl;
  e.GetException()->GetStackTrace(sl);
  if( log )  {
    TBasicApp::NewLogEntry() << "Exception occured:" << NewLineSequence() <<
      msg << NewLineSequence() << sl;
  }
  wxMessageBox(sl.Text(NewLineSequence()).u_str(),
    EsdlObjectName(*e.GetException()).u_str(),
    wxOK|wxICON_ERROR, MainFrameInstance);
}
