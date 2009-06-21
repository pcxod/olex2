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
#include "patchapi.h"
#include "shellutil.h"

#include <iostream>
using namespace std;

class TProgress: public AActionHandler  {
public:
  TProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  return true;  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog().Info( A->GetAction() );
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    return true;
  }
};
class TEProgress: public AActionHandler  {
public:
  TEProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  
    TBasicApp::GetLog() << "Done\n";
    return true;  
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {  return true;  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog() << (olxstr("Extracting: ") << A->GetAction() << '\n');
    return true;
  }
};
class TDProgress: public AActionHandler  {
public:
  TDProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  
    TBasicApp::GetLog() << "\rDone\n";
    return true;  
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog() << (olxstr("Downloading ") << A->GetAction() << '\n');
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog() << (olxstr("\r") << (int)(100*A->GetPos()/A->GetMax()) << '%');
    return true;
  }
};

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
        bapp = new TBasicApp( TEFile::AddTrailingBackslash(olex_dir) << TEFile::ExtractFileName(argv[0]) );
      else
        bapp = new TBasicApp(TEFile::AddTrailingBackslashI(TEFile::CurrentDir()) << TEFile::ExtractFileName(argv[0]));
    }
    else  {
      olxstr arg(argv[1]);
#ifdef _WIN32
      if( arg == "-help" || arg == "/help" )  {
#else
      if( arg == "--help" )  {
#endif     
        TBasicApp _bapp(  TEFile::AddTrailingBackslashI(TEFile::CurrentDir()) << TEFile::ExtractFileName(argv[0]) );
        TLog& log = _bapp.GetLog();
        log.AddStream( new TOutStream, true);
        log << "Unirun, Olex2 update/install program\n";
        log << "Compiled on " << __DATE__ << " at " << __TIME__ << '\n';
        log << "Usage: unirun [olex2_gui_dir]\n";
        log << "If no arguments provided, the system variable OLEX2_DIR will be checked first, if the variable is not set,\
               current folder will be updated\n";
        log << "(c) Oleg V. Dolomanov 2007-2009\n";
        return 0;
      }
      if( arg.EndsWith('.') || arg.EndsWith("..") )
        arg = TEFile::AbsolutePathTo(TEFile::CurrentDir(), arg);
      bapp = new TBasicApp(TEFile::AddTrailingBackslash(arg) << TEFile::ExtractFileName(argv[0]));
    }
    bapp->GetLog().AddStream( new TOutStream, true);
    bapp->SetSharedDir( TShellUtil::GetSpecialFolderLocation(fiAppData) << "Olex2u/");
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
#if defined(__WIN32__ ) && defined(_DEBUG)
  system("PAUSE");
#endif
  return res;
}

void DoRun()  {
  if( updater::UpdateAPI::IsInstallRequired() )  {
    TStrList repos;
    updater::UpdateAPI api;
    api.GetAvailableRepositories(repos);
    if( repos.IsEmpty() )
      TBasicApp::GetLog() << "Could not locate any installation repositories, aborting...\n";
    olxstr repo = repos[0];
    if( repos.Count() > 1 )  {
      cout << "Please choose the installation repository:\n";
      for( int i=0; i < repos.Count(); i++ )
        cout << (i+1) << ": " << repos[i].c_str() << '\n';
      cout << (repos.Count()+1) << ": Cancel\n";
      int repo_ind = 0;
      while( true )  {
        cout << "Your choice: ";
        cin >> repo_ind;
        if( cin.fail() )  continue;
        if( repo_ind == repos.Count()+1 )
          return;
        if( repo_ind > 0 && repo_ind <= repos.Count() )
          break;
      }
      repo = repos[repo_ind-1];
    }
    short res = api.DoInstall(new TDProgress, new TEProgress, repo);
    if( res != updater::uapi_OK && res != updater::uapi_UptoDate )  {
      TBasicApp::GetLog() << "Installation has failed...\n";
      TBasicApp::GetLog() << api.GetLog();
    }
  }
  else  {
    short res = patcher::PatchAPI::DoPatch(NULL, new TProgress);
    if( res != patcher::papi_OK )
      TBasicApp::GetLog() << "Update has failed...\n";
  }
}

