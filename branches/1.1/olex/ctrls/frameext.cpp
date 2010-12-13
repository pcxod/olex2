#include "frameext.h"
#include "estrlist.h"
#include "bapp.h"
#include "log.h"

IMPLEMENT_CLASS(TMainFrame, wxFrame)

TMainFrame* TMainFrame::MainFrameInstance = NULL;

void TMainFrame::RestorePosition(wxWindow *Window)  {  // restores previously saved position
  size_t ind = WindowPos.IndexOf(Window->GetName().c_str());
  if ( ind == InvalidIndex )  return;
  TWindowInfo &wi = WindowPos[Window->GetName().c_str()];
  Window->Move(wi.x, wi.y);
}
//..............................................................................
void TMainFrame::SavePosition(wxWindow *Window)  {  //saves current position of the window on screen
  TWindowInfo &wi = WindowPos.Add(Window->GetName().c_str());
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
      for( size_t k=di+1; k < masks[j].Length(); k++ )
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
//..............................................................................
int TMainFrame::ShowAlert(const olxstr &msg, const olxstr &title, int flags)  {
  return ::wxMessageBox(msg.u_str(), title.u_str(), flags, MainFrameInstance);
}
//..............................................................................
void TMainFrame::ShowAlert(const TExceptionBase &e, const olxstr &msg, bool log)  {
  if( log )  {
    TStrList sl;
    e.GetException()->GetStackTrace(sl);
    TBasicApp::NewLogEntry() << "Exception occured:" << NewLineSequence << msg << NewLineSequence << sl;
  }
  wxMessageBox(e.GetException()->GetError().u_str(),
    (olxstr("Exception: ") << EsdlObjectName(*e.GetException())).u_str(), wxOK|wxICON_ERROR, MainFrameInstance);
}
