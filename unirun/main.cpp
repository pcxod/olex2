#include <stdio.h>

#include "wx/app.h"

#include "efile.h"
#include "bapp.h"
#include "log.h"
#include "outstream.h"
#include "efile.h"
#include "wxzipfs.h"
#include "etime.h"
#include "settingsfile.h"
#include "datafile.h"
#include "dataitem.h"
#include "wxhttpfs.h"
#include "wxftpfs.h"
#include "updateapi.h"

#include <iostream>
using namespace std;

class TProgress: public AActionHandler  {
public:
  TProgress(){  return; }
  ~TProgress(){  return; }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    //TBasicApp::GetLog() << '\n';
    return true;
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {
      return false;
    }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog().Info( A->GetAction() );
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    return true;
  }
};

bool UpdateMirror( AFileSystem& src, TwxFtpFileSystem& dest )  {
  try  {
    TFSIndex FI( src );
    dest.OnProgress->Add( new TProgress );
    TStrList empty;
    return FI.Synchronise(dest, empty, NULL);
  }
  catch( TExceptionBase& exc )  {
    TStrList out;
    exc.GetException()->GetStackTrace(out);
    TBasicApp::GetLog() << "Update failed due to :\n" << out.Text('\n');
    return false;
  }
}
//---------------------------------------------------------------------------

void DoRun();

class MyApp: public wxAppConsole { 
  virtual bool OnInit() { 
//    wxSocketBase::Initialize();
    return true; 
  } 
  virtual int OnRun() {  return 0;  } 
};
IMPLEMENT_APP_NO_MAIN(MyApp)
int main(int argc, char** argv)  {
  MyApp app;
  TBasicApp* bapp = NULL;
  int res = 0;
  wxAppConsole::SetInstance(&app);
  try  {
    if( argc == 1 )  { // no folder to update provided
      char* olex_dir = getenv("OLEX2_DIR");
      if( olex_dir != NULL )
        bapp = new TBasicApp( olxstr(olex_dir) << "/dummy.txt" );
      else
        bapp = new TBasicApp(TEFile::CurrentDir() << "/dummy.txt");
    }
    else  {
      olxstr arg(argv[1]);
#ifdef _WIN32
      if( arg == "-help" || arg == "/help" )  {
#else
      if( arg == "--help" )  {
#endif     
        TBasicApp _bapp(  TEFile::CurrentDir() << "/dummy.txt" );
        TLog& log = _bapp.GetLog();
        log.AddStream( new TOutStream, true);
        log << "Unirun, Olex2 update program\n";
        log << "Compiled on " << __DATE__ << " at " << __TIME__ << '\n';
        log << "Usage: unirun [olex2_gui_dir]\n";
        log << "If no arguments provided, the system variable OLEX2_DIR will be checked first, if the variable is not set,\
               current folder will be updated\n";
        log << "(c) Oleg V. Dolomanov 2007-2009\n";
        return 0;
      }
      if( arg.EndsWith('.') || arg.EndsWith("..") )
        arg = TEFile::AbsolutePathTo(TEFile::CurrentDir(), arg);
      bapp = new TBasicApp(arg << "/dummy.txt");
    }
    bapp->GetLog().AddStream( new TOutStream, true);
    DoRun();
  }
  catch(const TExceptionBase& exc)  {
    if( bapp != NULL )  {
      TStrList out;
      exc.GetException()->GetStackTrace(out);
      bapp->GetLog() << out;
    }
    res = 1;
  }
  if( bapp != NULL )  {
    bapp->GetLog() << "\nFinished\n";
    delete bapp;
  }
  else  {
    cout << "\nFinished\n";
  }
  return res;
}

void DoRun()  {
  updater::UpdateAPI api;
  api.DoUpdate(new TProgress, NULL);
}

