#include <stdio.h>

#include "wx/app.h"

#include "efile.h"
#include "bapp.h"
#include "log.h"
#include "outstream.h"
#include "efile.h"
#include "wxzipfs.h"
#include "httpfs.h"
#include "etime.h"
#include "settingsfile.h"
#include "datafile.h"
#include "dataitem.h"
#include "wxftpfs.h"
#include "updateapi.h"
#include "patchapi.h"
#include "shellutil.h"
#include "egc.h"

#include <iostream>
using namespace std;

class TProgress: public AActionHandler  {
public:
  TProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  return true;  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::NewLogEntry(logInfo) << A->GetAction();
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
class TUProgress: public AActionHandler  {
public:
  TUProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  
    TBasicApp::NewLogEntry() << "Done";
    return true;  
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {  return true;  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::NewLogEntry() << "Copying: " << A->GetAction();
    return true;
  }
};
class TDProgress: public AActionHandler  {
public:
  TDProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  
    TBasicApp::NewLogEntry() << "\rDone";
    return true;  
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog() << (olxstr("Downloading ") << A->GetAction() << "\n0%");
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog() << (olxstr("\r") << (int)(100*A->GetPos()/A->GetMax()) << '%');
		fflush(NULL);
    return true;
  }
};

//---------------------------------------------------------------------------

void DoRun();

class MyApp: public wxAppConsole { 
  virtual bool OnInit() { 
    return true; 
  } 
  virtual int OnRun() {  return 0;  } 
};
IMPLEMENT_APP_NO_MAIN(MyApp)
int main(int argc, char** argv)  {
  MyApp app;
  TBasicApp* bapp = NULL;
  int res = 0;
  wxAppConsole::SetInstance(&app); // as soon as we create TBasicApp, this instance gets attached to it
  TEGC::Initialise();
  try  {
    if( argc == 1 )  // no folder to update provided
      bapp = new TBasicApp(TBasicApp::GuessBaseDir(argv[0], "OLEX2_DIR"));
    else  {
      olxstr arg(argv[1]);
#ifdef _WIN32
      if( arg == "-help" || arg == "/help" )  {
#else
      if( arg == "--help" )  {
#endif     
        TBasicApp _bapp(TBasicApp::GuessBaseDir(argv[0], "") );
        TLog& log = _bapp.GetLog();
        log.AddStream( new TOutStream, true);
        log.NewEntry().nl() << "Unirun, Olex2 update/install program";
        log.NewEntry() << "Compiled on " << __DATE__ << " at " << __TIME__;
        log.NewEntry() << "Usage: unirun [olex2_gui_dir]";
        log.NewEntry() << "If no arguments provided, the system variable OLEX2_DIR will be checked "
          "first, if the variable is not set, current folder will be updated";
        log.NewEntry() << "(c) Oleg V. Dolomanov 2007-2009" << NewLineSequence;
        return 0;
      }
      bapp = new TBasicApp(TBasicApp::GuessBaseDir(argv[1], ""));
    }
		// parse out options
    for( int i=0; i < argc; i++ )
      bapp->Arguments.Add(argv[i]);
    for( size_t i=0; i < bapp->Arguments.Count(); i++ )  {
      if( bapp->Arguments[i].FirstIndexOf('=') != InvalidIndex )  {
        bapp->Options.FromString(bapp->Arguments[i], '=');
        bapp->Arguments.Delete(i--);
      }
    }
    bapp->GetLog().AddStream(new TOutStream, true);
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
    bapp->NewLogEntry().nl() << "Finished";
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
    olxstr repo;
    TBasicApp::NewLogEntry() << "Installation folder: "  << TBasicApp::GetBaseDir();
    if( TBasicApp::GetInstance().Options.Contains("-tag") )  {
      olxstr tag = TBasicApp::GetInstance().Options["-tag"];
      if( tag.Equalsi("max") )  {
        TStrList tags;
        api.GetAvailableTags(tags, repo);
        if( tags.IsEmpty() )  {
          TBasicApp::NewLogEntry() << "Could not locate any installation repositories/tags, aborting...";
          return;
        }  
        double max_tag = 0;
        for( size_t i=0; i < tags.Count(); i++ )  {
          if( tags[i].IsNumber() && tags[i].ToDouble() > max_tag )
            max_tag = tags[i].ToDouble();
        }
        repo << olxstr::FormatFloat(1,max_tag);
      }
      else if( tag.Equalsi("zip") )  {
        olxstr zfn = TBasicApp::GetBaseDir() + api.GetInstallationFileName();
        if( TEFile::Exists(zfn) )
          repo = zfn;
      }
      else  {
        repo = api.FindActiveRepositoryUrl();
        if( repo.IsEmpty() )  {
          TBasicApp::NewLogEntry() << "Could not locate any installation repositories, aborting...";
          return;
        }
        repo << tag;
      }
    }
    else  {
      api.GetAvailableRepositories(repos);
      if( repos.IsEmpty() )  {
        TBasicApp::NewLogEntry() << "Could not locate any installation repositories, aborting...";
        return;
      }
      repo = repos[0];
      if( repos.Count() >= 1 )  {
        TBasicApp::NewLogEntry() << "Please choose the installation repository or Cancel:";
        for( size_t i=0; i < repos.Count(); i++ )
          TBasicApp::GetLog() << (i+1) << ": " << repos[i].c_str() << '\n';
        TBasicApp::NewLogEntry() << (repos.Count()+1) << ": Cancel";
        size_t repo_ind = 0;
        while( true )  {
          TBasicApp::GetLog() << "Your choice: ";
          cin >> repo_ind;
          if( cin.fail() )  continue;
          if( repo_ind == repos.Count()+1 )
            return;
          if( repo_ind <= repos.Count() )
            break;
        }
        repo = repos[repo_ind-1];
      }
    }
    TBasicApp::NewLogEntry() << "Installing using: " << repo;
    short res = api.DoInstall(new TDProgress, new TEProgress, repo);
    if( res != updater::uapi_OK && res != updater::uapi_UptoDate )  {
      TBasicApp::NewLogEntry() << "Installation has failed with error code: " << res;
      TBasicApp::NewLogEntry() << api.GetLog();
    }
    else  {
      updater::UpdateAPI::TagInstallationAsNew();
    }
  }
  else  {
    short res = patcher::PatchAPI::DoPatch(NULL, new TUProgress);
    if( res != patcher::papi_OK )
      TBasicApp::NewLogEntry() << "Update has failed with error code: " << res;
  }
}
