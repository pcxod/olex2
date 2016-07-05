
/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
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

#include <stdio.h>
#include "wx/app.h"

using namespace std;

class TProgress: public AActionHandler  {
public:
  TProgress(){}
  bool Exit(const IOlxObject *Sender, const IOlxObject *Data)  {  return true;  }
  bool Enter(const IOlxObject *Sender, const IOlxObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::NewLogEntry(logInfo) << A->GetAction();
    return true;
  }
  bool Execute(const IOlxObject *Sender, const IOlxObject *Data)  {
    return true;
  }
};
class TEProgress: public AActionHandler  {
public:
  TEProgress(){}
  bool Exit(const IOlxObject *Sender, const IOlxObject *Data)  {
    TBasicApp::GetLog() << "Done\n";
    return true;
  }
  bool Enter(const IOlxObject *Sender, const IOlxObject *Data)  {  return true;  }
  bool Execute(const IOlxObject *Sender, const IOlxObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog() << (olxstr("Extracting: ") << A->GetAction() << '\n');
    return true;
  }
};
class TUProgress: public AActionHandler  {
public:
  TUProgress(){}
  bool Exit(const IOlxObject *Sender, const IOlxObject *Data)  {
    TBasicApp::NewLogEntry() << "Done";
    return true;
  }
  bool Enter(const IOlxObject *Sender, const IOlxObject *Data)  {  return true;  }
  bool Execute(const IOlxObject *Sender, const IOlxObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::NewLogEntry() << "Copying: " << A->GetAction();
    return true;
  }
};
class TDProgress: public AActionHandler  {
public:
  TDProgress(){}
  bool Exit(const IOlxObject *Sender, const IOlxObject *Data)  {
    TBasicApp::NewLogEntry() << "\rDone";
    return true;
  }
  bool Enter(const IOlxObject *Sender, const IOlxObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog() <<
      (olxstr("Downloading ") << A->GetAction() << "\n0%");
    return true;
  }
  bool Execute(const IOlxObject *Sender, const IOlxObject *Data)  {
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
void DoLaunch(const TStrList &args);
char **ListToArgs(TStrList &args_list) {
  char **args = new char*[args_list.Count()+1];
  args[args_list.Count()] = NULL;
  for (size_t i=0; i < args_list.Count(); i++) {
    // options are to be quoted by the called
    bool option = args_list[i].StartsFrom('-') ||
      args_list[i].IndexOf('=') != InvalidIndex;
    if (!option && args_list[i].IndexOf(' ') != InvalidIndex)
      args_list[i] = olxstr('"') << args_list[i] << '"';
    olxcstr v = TUtf8::Encode(args_list[i]);
    args[i] = new char [v.Length()+1];
    strcpy(args[i], v.c_str());
  }
  return args;
}

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
  // ensure we end up with the same datadir!
  app.SetAppName(wxT("olex2"));
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
      if (!print_help && TEFile::Exists(arg) && TEFile::IsDir(arg)) {
        base_dir = arg;
        var_name.SetLength(0);
      }
// must be done on mac!
#ifdef __MAC__
      var_name.SetLength(0);
#endif
    }
    TBasicApp bapp(TBasicApp::GuessBaseDir(base_dir, var_name));
    bapp.GetLog().AddStream(&out, false);
    //bapp.GetLog().AddStream(TUtf8File::Create(
    //  patcher::PatchAPI::GetInstanceDir() + "unirun.log", true), false);
    if( print_help ) {
      TLog& log = bapp.GetLog();
      log.NewEntry().nl() << "Unirun, Olex2 update/install program";
      log.NewEntry() << "Compiled on " << __DATE__ << " at " << __TIME__;
      log.NewEntry() << "Usage: unirun [olex2_gui_dir]";
      log.NewEntry() << "If no arguments provided, the system variable "
        "OLEX2_DIR will be checked first, if the variable is not set, "
        "current folder will be updated";
      log.NewEntry() << "(c) OlexSys, Oleg V. Dolomanov 2007-2012" <<
        NewLineSequence();
      return 0;
    }
    // parse out options
    bapp.InitArguments(argc, argv);
    bapp.SetSharedDir(patcher::PatchAPI::GetSharedDir());
    bapp.SetInstanceDir(patcher::PatchAPI::GetInstanceDir());
    DoRun();
#ifdef _DEBUG
    system("PAUSE");
#endif
    if( !bapp.GetOptions().Contains("-run") )
      DoLaunch(bapp.GetArguments().SubListFrom(1));
  }
  catch(const TExceptionBase& exc)  {
    TStrList err;
    out.Write(
      exc.GetException()->GetStackTrace(err).Text(NewLineSequence()));
    res = 1;
  }
  out.Write(olxstr("Finished") << NewLineSequence());
#if defined(__WIN32__ ) && defined(_DEBUG)
  system("PAUSE");
#endif
  return res;
}

void DoRun()  {
  bool temporary_run=false;
  olxstr bd = TBasicApp::GetInstance().GetOptions().FindValue(
    "-basedir", EmptyString());
//  TBasicApp::NewLogEntry() << "L: " << bd;
  if (!bd.IsEmpty()) {
    TBasicApp::SetBaseDir(TEFile::AddPathDelimeterI(bd) << "dymmy.exe");
    temporary_run = true;
    TBasicApp::SetSharedDir(patcher::PatchAPI::GetSharedDir(true));
    TBasicApp::GetInstance().SetInstanceDir(
      patcher::PatchAPI::GetInstanceDir(true));
  }
#ifdef _DEBUG
  TBasicApp::NewLogEntry() << "Base dir: " << TBasicApp::GetBaseDir();
  TBasicApp::NewLogEntry() << "Instance dir: " << TBasicApp::GetInstanceDir();
  TBasicApp::NewLogEntry() << "Shared dir: " << TBasicApp::GetSharedDir();
#endif
  if( updater::UpdateAPI::IsInstallRequired() )  {
    TStrList repos;
    updater::UpdateAPI api;
    olxstr repo;
    TBasicApp::NewLogEntry() << "Installation folder: "
      << TBasicApp::GetBaseDir();
    if( TBasicApp::GetInstance().GetOptions().Contains("-tag") )  {
      olxstr tag = TBasicApp::GetInstance().GetOptions()["-tag"];
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
    // compatibility with older versions!
    olxstr old_lf = TBasicApp::GetBaseDir() + "__location.update";
    if (TEFile::Exists(old_lf)) {
      TEFile::Copy(old_lf, patcher::PatchAPI::GetUpdateLocationFileName());
      TEFile::DelFile(old_lf);
    }
    olxstr patch_dir = TBasicApp::GetInstanceDir() +
      patcher::PatchAPI::GetPatchFolder();
    bool force_update = (TEFile::Exists(patch_dir) &&
      !TEFile::IsEmptyDir(patch_dir) &&
      !TEFile::Exists(TEFile::AddPathDelimeter(patch_dir) + "index.ind"));
    if (force_update && !patcher::PatchAPI::HaveUpdates())
      patcher::PatchAPI::MarkPatchComplete();

    olxstr tmp_exe_name = TBasicApp::GetInstanceDir() +
      TEFile::ExtractFileName(TBasicApp::GetArg(0));
    bool can_copy = true;
    if (TEFile::Exists(tmp_exe_name)) {
      can_copy = TEFile::DelFile(tmp_exe_name);
    }
#ifdef _DEBUG
    TBasicApp::NewLogEntry() << "Have updates: " <<
      patcher::PatchAPI::HaveUpdates();
#endif
    if (patcher::PatchAPI::HaveUpdates()) {
      if (!temporary_run) {
        if (!can_copy) {
          TBasicApp::NewLogEntry(logError) <<
            "Another update is currently running, skiping";
          return;
        }
        if (!TBasicApp::IsBaseDirWriteable()) {
          TBasicApp::NewLogEntry(logError) <<
            "Updates are available, but the process cannot write to the "
            "installation folder. Please run with elevated permissions. Or - ";
          TBasicApp::NewLogEntry() << "Move the content of this folder:";
          TBasicApp::NewLogEntry() << patcher::PatchAPI::GetUpdateLocation();
          TBasicApp::NewLogEntry() << "Into this folder:";
          TBasicApp::NewLogEntry() << TBasicApp::GetBaseDir();
          TBasicApp::NewLogEntry() << "And delete this file:";
          TBasicApp::NewLogEntry() <<
            patcher::PatchAPI::GetUpdateLocationFileName();
          return;
        }
        else if( TBasicApp::GetInstance().IsBaseDirWriteable() )  {
          if (!TEFile::Copy(TBasicApp::GetArg(0), tmp_exe_name)) {
            TBasicApp::NewLogEntry(logError) <<
              "Could not copy itself, aborting update";
            return;
          }
          TStrList args_list;
          args_list << tmp_exe_name;
          args_list << (olxstr("-basedir=").quote('"') <<
            TEFile::TrimPathDelimeter(TBasicApp::GetBaseDir()));
          args_list << TBasicApp::GetInstance().GetArguments().SubListFrom(1);
          for (size_t i=0; i < TBasicApp::GetInstance().GetOptions().Count(); i++) {
            olxstr &l = args_list.Add(TBasicApp::GetInstance().GetOptions().GetName(i));
            if (!TBasicApp::GetInstance().GetOptions().GetValue(i).IsEmpty()) {
              l << (olxstr('=').quote('"') <<
                TBasicApp::GetInstance().GetOptions().GetValue(i));
            }
          }
          char **args = ListToArgs(args_list);
          olxstr c_cmdl = TUtf8::Encode(tmp_exe_name);
          execv(c_cmdl.c_str(), args);
          TBasicApp::NewLogEntry(logError) <<
            "Could re-launch itself";
          return;
        }
      }
      else {
        short res = patcher::PatchAPI::DoPatch(NULL, new TUProgress);
        if( res != patcher::papi_OK )
          TBasicApp::NewLogEntry() << "Update has failed with error code: "
          << res;
      }
    }
  }
}

void DoLaunch(const TStrList &args_)  {
  olxcstr bd = TBasicApp::GetBaseDir();
  olxstr path = olx_getenv("PATH");
  path.Insert(bd.SubStringTo(bd.Length()-1) + ':', 0);
  olx_setenv("PATH", path);
  olx_setenv("OLEX2_CCTBX_DIR", EmptyString());
  olx_setenv("OLEX2_DIR", EmptyString());
#ifdef __WIN32__
  olx_setenv("PYTHONHOME", bd + "Python27");
  const olxstr cmdl = bd + "olex2.dll";
#else
  olx_setenv("BOOST_ADAPTBX_FPE_DEFAULT", "1");
  olx_setenv("BOOST_ADAPTBX_SIGNALS_DEFAULT", "1");
  olxstr gl_stereo = olx_getenv("OLEX2_GL_STEREO");
  if (gl_stereo.IsEmpty())
    olx_setenv("OLEX2_GL_STEREO", "FALSE");
#  ifdef __MAC__
  olx_setenv("OLEX2_DIR", bd);
  olxstr data_dir = olx_getenv("OLEX2_DATADIR");
  // override new ~/Library/Application Support!
  if (data_dir.IsEmpty()) {
    olx_setenv("OLEX2_DATADIR", patcher::PatchAPI::_GetSharedDirRoot());
  }
  static const olxcstr ld_var = "DYLD_LIBRARY_PATH";
#  else
  static const olxcstr ld_var = "LD_LIBRARY_PATH";
#  endif
  olxcstr ld_path;
  ld_path << bd << "lib:" << bd << "cctbx/cctbx_build/lib";
  olx_setenv(ld_var, ld_path);
  olx_setenv("PYTHONHOME", bd);
  const olxstr cmdl = bd + "olex2_exe";
#endif
  TEFile::ChangeDir(bd);
  TEFile::Chmod(cmdl, S_IEXEC|S_IREAD|S_IWRITE);
  TStrList args_list;
  args_list << cmdl;
  args_list << args_;
  char **args = ListToArgs(args_list);
  olxstr c_cmdl = TUtf8::Encode(cmdl);
  execv(c_cmdl.c_str(), args);
  TBasicApp::NewLogEntry(logError) << "Failed to launch '" << cmdl << '\'';
}
