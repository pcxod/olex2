
/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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
#include "wxzipfs.h"
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
    TBasicApp::GetLog() <<
      (olxstr("Downloading ") << A->GetAction() << "\n0%");
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog() <<
      (olxstr("\r") << (int)(100*A->GetPos()/A->GetMax()) << '%');
		fflush(NULL);
    return true;
  }
};

//---------------------------------------------------------------------------
void DoRun();
void DoLaunch();

class MyApp: public wxAppConsole { 
  virtual bool OnInit() { 
    return true; 
  } 
  virtual int OnRun() {  return 0;  } 
};
IMPLEMENT_APP_NO_MAIN(MyApp)
int main(int argc, char** argv)  {
  MyApp app;
  int res = 0;
  /* as soon as we create TBasicApp, this instance gets attached to it
  */
  wxAppConsole::SetInstance(&app);
  TEGC::Initialise();
  TOutStream out;
#if defined(__WIN32__) && !defined(__WXWIDGETS__)
  #include "winzipfs.h"
  typedef TWinZipFileSystem ZipFS;
#else
  #include "wxzipfs.h"
  typedef TwxZipFileSystem ZipFS;
#endif
#ifdef __WXWIDGETS__
  #include "wxftpfs.h"
#endif
  ZipFS::RegisterFactory();
  try  {
    bool print_help = false;
    olxstr base_dir = argv[0], var_name = "OLEX2_DIR";
    if( argc > 1 )  {
      olxstr arg(argv[1]);
      print_help = (arg == "-help" || arg == "/help" || arg == "--help");
      if( !print_help && TEFile::Exists(arg) )  {
        base_dir = arg;
      }
      var_name.SetLength(0);
    }
    TBasicApp bapp(TBasicApp::GuessBaseDir(base_dir, var_name));
    bapp.GetLog().AddStream(&f_out, false);
    bapp.GetLog().AddStream(&out, false);
    if( print_help ) {
      TLog& log = bapp.GetLog();
      log.NewEntry().nl() << "Unirun, Olex2 update/install program";
      log.NewEntry() << "Compiled on " << __DATE__ << " at " << __TIME__;
      log.NewEntry() << "Usage: unirun [olex2_gui_dir]";
      log.NewEntry() << "If no arguments provided, the system variable "
        "OLEX2_DIR will be checked first, if the variable is not set, "
        "current folder will be updated";
      log.NewEntry() << "(c) OlexSys, Oleg V. Dolomanov 2007-2011" <<
        NewLineSequence();
      return 0;
    }
		// parse out options
    for( int i=0; i < argc; i++ )
      bapp.Arguments.Add(argv[i]);
    for( size_t i=0; i < bapp.Arguments.Count(); i++ )  {
      if( bapp.Arguments[i].FirstIndexOf('=') != InvalidIndex )  {
        bapp.Options.FromString(bapp.Arguments[i], '=');
        bapp.Arguments.Delete(i--);
      }
    }
    DoRun();
    if( bapp.Arguments.IndexOf("-run") == InvalidIndex )
      DoLaunch();
  }
  catch(const TExceptionBase& exc)  {
    TStrList err;
    exc.GetException()->GetStackTrace(err);
    out.Write(err.Text(NewLineSequence()));
    res = 1;
  }
  out.Write(olxstr("Finished") << NewLineSequence());
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
    TBasicApp::NewLogEntry() << "Installation folder: "
      << TBasicApp::GetBaseDir();
    if( TBasicApp::GetInstance().Options.Contains("-tag") )  {
      olxstr tag = TBasicApp::GetInstance().Options["-tag"];
      if( tag.Equalsi("max") )  {
        TStrList tags;
        api.GetAvailableTags(tags, repo);
        if( tags.IsEmpty() )  {
          TBasicApp::NewLogEntry() << "Could not locate any installation "
            "repositories/tags, aborting...";
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
          TBasicApp::NewLogEntry() << "Could not locate any installation "
            "repositories, aborting...";
          return;
        }
        repo << tag;
      }
    }
    else  {
      api.GetAvailableRepositories(repos);
      if( repos.IsEmpty() )  {
        TBasicApp::NewLogEntry() << "Could not locate any installation "
          "repositories, aborting...";
        return;
      }
      repo = repos[0];
      if( repos.Count() >= 1 )  {
        TBasicApp::NewLogEntry() << "Please choose the installation repository "
          "or Cancel:";
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
      TBasicApp::NewLogEntry() << "Installation has failed with error code: "
        << res;
      TBasicApp::NewLogEntry() << api.GetLog();
    }
    else  {
      updater::UpdateAPI::TagInstallationAsNew();
    }
  }
  else  {
    short res = patcher::PatchAPI::DoPatch(NULL, new TUProgress);
    if( res != patcher::papi_OK )
      TBasicApp::NewLogEntry() << "Update has failed with error code: "
      << res;
  }
}

void DoLaunch()  {
  olxcstr bd = TBasicApp::GetBaseDir();
  olxstr path = olx_getenv("PATH");
  path.Insert(bd.SubStringTo(bd.Length()-1) + ':', 0);
  olx_setenv("PATH", path);
  olx_setenv("OLEX2_CCTBX_DIR", EmptyString());
  olx_setenv("OLEX2_DIR", EmptyString());
#ifdef __WIN32__
  olx_setenv("PYTHONHOME", bd + "Python26");
  const olxcstr cmdl = bd + "olex2.dll";
#else
#  ifdef __MAC__
  olx_setenv("OLEX2_DIR", bd);
  static const olxcstr ld_var = "DYLD_LIBRARY_PATH";
#  else
  static const olxcstr ld_var = "LD_LIBRARY_PATH";
#  endif
  olxcstr ld_path;
  ld_path << bd << "lib:" << bd << "Python26:" << bd << "cctbx/cctbx_build/lib";
  olx_setenv(ld_var, ld_path);
  olx_setenv("PYTHONHOME", bd + "Python26/python2.6");
  const olxcstr cmdl = bd + "olex2_exe";
#endif
  TEFile::ChangeDir(bd);
  execl(cmdl.u_str(), cmdl.u_str(), NULL);
  TBasicApp::NewLogEntry(logError) << "Failed to launch '" << cmdl << '\'';
}

