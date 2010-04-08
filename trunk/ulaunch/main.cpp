#include "bapp.h"
#include "log.h"
#include "edict.h"
#include "outstream.h"
#include "efile.h"
#include "settingsfile.h"
#include "patchapi.h"

#ifdef __WIN32__
#  include <process.h>
#  define PutEnv _putenv
#  define GetEnv getenv
void SetEnv(const olxstr& v)  {  putenv(v.u_str());  }
const char exe_name[] = "olex2.dll";
#else
#  include <unistd.h>
#  define PutEnv putenv
#  define GetEnv getenv
void SetEnv(const olxstr& v)  {
  size_t ei = v.IndexOf('=');
  if( ei == InvalidIndex )  return;
    setenv(v.SubStringTo(ei).u_str(), v.SubStringFrom(ei+1).u_str(), 1);
}
#  ifdef __MAC__
const char exe_name[] = "olex2.app";
#  else
const char exe_name[] = "olex2";
#  endif
#endif

void Launch();

class TUProgress: public AActionHandler  {
public:
  TUProgress(){}
  bool Exit(const IEObject *Sender, const IEObject *Data)  {  
    TBasicApp::GetLog() << "Done\n";
    return true;  
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {  return true;  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( Data == NULL )  {  return false;  }
    const TOnProgress *A = dynamic_cast<const TOnProgress*>(Data);
    TBasicApp::GetLog() << (olxstr("Copying: ") << A->GetAction() << '\n');
    return true;
  }
};

int main(int argc, char** argv)  {
  TOutStream out;
  puts(argv[0]);
  try  {
    olxstr argv_0 = argv[0];
    if( argv_0.IndexOf(' ') != InvalidIndex && !argv_0.StartsFrom('"') )
      argv_0 = olxstr('"') << argv_0 << '"';
    TBasicApp app(TBasicApp::GuessBaseDir(argv_0));
    app.GetLog().AddStream(new TOutStream, true);
    olxstr OlexFN(TBasicApp::GetBaseDir()+ exe_name);
    const olxstr set_fn = TBasicApp::GetBaseDir()+ "launch.dat";
    if( TEFile::Exists(set_fn) )  {
      TSettingsFile sf(set_fn);
      const olxstr original_bd = TBasicApp::GetBaseDir();
      olxstr base_dir = sf.GetParam("base_dir");
      TStrList env_toks(sf.GetParam("env"), ',');
#ifdef __WIN64__
      TEFile::AddPathDelimeterI(base_dir) << sf.GetParam("prefix_win64");
      env_toks.Strtok(sf.GetParam("env_win64"), ',');
#elif __WIN32__
      TEFile::AddPathDelimeterI(base_dir) << sf.GetParam("prefix_win32");
      env_toks.Strtok(sf.GetParam("env_win32"), ',');
#elif __MAC__
      TEFile::AddPathDelimeterI(base_dir) << sf.GetParam("prefix_mac");
      env_toks.Strtok(sf.GetParam("env_mac"), ',');
#else
      TEFile::AddPathDelimeterI(base_dir) << sf.GetParam("prefix_linux");
      env_toks.Strtok(sf.GetParam("env_linux"), ',');
#endif
      if( !TEFile::IsAbsolutePath(base_dir) )  {
        if( base_dir.StartsFrom('.') || base_dir.StartsFrom("..") )
          base_dir = TEFile::AbsolutePathTo(base_dir, original_bd);
        else
          base_dir = original_bd + base_dir;
      }
      TBasicApp::SetBaseDir(TEFile::AddPathDelimeter(base_dir)+"dummy.exe");
      OlexFN = TBasicApp::GetBaseDir()+ "olex2.dll";
      olxstr data_dir = sf.GetParam("data_dir", TBasicApp::GetBaseDir() + "olex2data");
      if( !TEFile::IsAbsolutePath(data_dir) )  {
        if( data_dir.StartsFrom('.') || data_dir.StartsFrom("..") )
          data_dir = TEFile::AbsolutePathTo(data_dir, original_bd);
        else
          data_dir = original_bd + data_dir;
      }
#ifndef __WIN32__ // speciat treatment for non-windows data dirs...
      TEFile::AddPathDelimeterI(data_dir) << ".olex2";
#endif
      if( !TEFile::Exists(data_dir) )  {
        if( !TEFile::MakeDirs(data_dir) )
          throw TFunctionFailedException(__OlxSourceInfo, "Failed to create DATA_DIR");
      }
      SetEnv(olxstr("OLEX2_DATADIR=") << data_dir);

      for( size_t i=0; i < env_toks.Count(); i++ )
        SetEnv(env_toks[i]);
    }
    else if( !TEFile::Exists(OlexFN) )
      throw TFunctionFailedException(__OlxSourceInfo, "No settings file is provided or Olex2 executable found");
    if( TBasicApp::GetInstance().IsBaseDirWriteable() )  {
      short res = patcher::PatchAPI::DoPatch(NULL, new TUProgress);
      if( res != patcher::papi_OK )  {
        olxstr msg;
        if( res == patcher::papi_Busy )
          msg = "Another update or Olex2 instance are running at the moment";
        else if( res == patcher::papi_CopyError || res == patcher::papi_DeleteError )  {
          msg = "Please make sure that no Olex2 instances are running,\n\
                you have enough right to modify the installation folder and\n\
                no Olex2 folders are opened in browsers";
        }
        TBasicApp::GetLog() << "Update failed: ";
        TBasicApp::GetLog().Error(msg);
      }
      Launch();
    }
  }
  catch(const TExceptionBase& e)  {
    out.Write(e.GetException()->GetFullMessage());
  }
}

void Launch()  {
  const olxstr bd = TBasicApp::GetBaseDir();
  olxch* _path = GetEnv("PATH");
  olxstr path;
  if( _path != NULL )  path = _path;
  path.Insert(bd.SubStringTo(bd.Length()-1) + ':', 0);
  SetEnv(olxstr("PATH=") << path);
  SetEnv("OLEX2_CCTBX_DIR=");
  SetEnv("OLEX2_DIR=");
#ifdef __WIN32__
  SetEnv(olxstr("PYTHONHOME=") << TBasicApp::GetBaseDir() << "Python26");
  const olxstr cmdl = bf + "olex2.dll";
#else
#  ifdef __MAC__
  const olxstr base_dir = bd + "olex2.app/Contents/MacOS/";
  SetEnv(olxstr("OLEX2_DIR=") << base_dir);
  static const olxch* ld_var = "DYLD_LIBRARY_PATH";
#  else
  const olxstr base_dir = bd;
  static const olxch* ld_var = "LD_LIBRARY_PATH";
#  endif
  olxch* _ld_path = GetEnv(ld_var);
  olxstr ld_path;
  if( _ld_path != NULL )  ld_path = _ld_path;
  ld_path.Insert(base_dir+"cctbx/cctbx_build/lib:", 0);
  ld_path.Insert(base_dir+"Python26:", 0);
  ld_path.Insert(base_dir+"lib:", 0);
  SetEnv(olxstr(ld_var) << '=' << ld_path);
  SetEnv(olxstr("PYTHONHOME=") << base_dir << "Python26/python2.6");
  const olxstr cmdl = base_dir + "olex2";
#endif
  TEFile::ChangeDir(bd);
  execl(cmdl.u_str(), cmdl.u_str(), NULL);
  TBasicApp::GetLog().Error("Failed to launch Olex2");
}
