#include "bapp.h"
#include "log.h"
#include "edict.h"
#include "outstream.h"
#include "efile.h"
#include "settingsfile.h"
#include "patchapi.h"

#ifdef __WIN32__
const char exe_name[] = "olex2.dll";
#else
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
      const olxstr prefix = sf.GetParam("prefix_win64");
      env_toks.Strtok(sf.GetParam("env_win64"), ',');
#elif __WIN32__
      const olxstr prefix = sf.GetParam("prefix_win32");
      env_toks.Strtok(sf.GetParam("env_win32"), ',');
#elif __MAC__
      const olxstr prefix = sf.GetParam("prefix_mac");
      env_toks.Strtok(sf.GetParam("env_mac"), ',');
#else
      const olxstr prefix = sf.GetParam("prefix_linux");
      env_toks.Strtok(sf.GetParam("env_linux"), ',');
#endif
      TEFile::AddPathDelimeterI(base_dir) << prefix;
      if( !TEFile::IsAbsolutePath(base_dir) )  {
        if( base_dir.StartsFrom('.') || base_dir.StartsFrom("..") )
          base_dir = TEFile::ExpandRelativePath(base_dir, original_bd);
        else
          base_dir = original_bd + base_dir;
      }
#  ifdef __MAC__
  base_dir = TEFile::AddPathDelimeterI(base_dir) << "olex2.app/Contents/MacOS/";
#endif
      TBasicApp::SetBaseDir(TEFile::AddPathDelimeter(base_dir)+"dummy.exe");
      OlexFN = TBasicApp::GetBaseDir()+ "olex2.dll";
      olxstr data_dir = sf.GetParam("data_dir", TBasicApp::GetBaseDir() + "olex2data");
      if( !TEFile::IsAbsolutePath(data_dir) )  {
        if( data_dir.StartsFrom('.') || data_dir.StartsFrom("..") )
          data_dir = TEFile::ExpandRelativePath(data_dir, original_bd);
        else
          data_dir = original_bd + data_dir;
      }
      TEFile::AddPathDelimeterI(data_dir) << prefix;
      if( !TEFile::Exists(data_dir) )  {
        if( !TEFile::MakeDirs(data_dir) )
          throw TFunctionFailedException(__OlxSourceInfo, "Failed to create DATA_DIR");
      }
      olx_setenv("OLEX2_DATADIR", TEFile::AddPathDelimeterI(data_dir));
      for( size_t i=0; i < env_toks.Count(); i++ )
        olx_setenv(env_toks[i]);
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
        TBasicApp::NewLogEntry(logError) << msg;
      }
      Launch();
    }
    else
      TBasicApp::NewLogEntry(logError) << "Read-only file system...";
  }
  catch(const TExceptionBase& e)  {
    out.Write(e.GetException()->GetFullMessage());
  }
}

void Launch()  {
  const olxstr bd = TBasicApp::GetBaseDir();
  olxstr path = olx_getenv("PATH");
  path.Insert(bd.SubStringTo(bd.Length()-1) + ':', 0);
  olx_setenv("PATH", path);
  olx_setenv("OLEX2_CCTBX_DIR", EmptyString);
  olx_setenv("OLEX2_DIR", EmptyString);
#ifdef __WIN32__
  olx_setenv("PYTHONHOME", bd + "Python26");
  const olxstr cmdl = bd + "olex2.dll";
#else
#  ifdef __MAC__
  olx_setenv("OLEX2_DIR", bd);
  static const olxstr ld_var = "DYLD_LIBRARY_PATH";
#  else
  static const olxstr ld_var = "LD_LIBRARY_PATH";
#  endif
  olxstr ld_path;
  ld_path << bd << "lib:" << bd << "Python26:" << bd << "cctbx/cctbx_build/lib";
  olx_setenv(ld_var, ld_path);
  olx_setenv("PYTHONHOME", bd + "Python26/python2.6");
  const olxstr cmdl = bd + "olex2";
#endif
  TEFile::ChangeDir(bd);
  execl(cmdl.u_str(), cmdl.u_str(), NULL);
  TBasicApp::NewLogEntry(logError) << "Failed to launch Olex2";
}
