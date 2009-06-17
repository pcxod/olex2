#include "mainform.h"

#include "updateth.h"


UpdateThread::UpdateThread(const olxstr& patch_dir) : time_out(0), PatchDir(patch_dir), 
    srcFS(NULL), destFS(NULL) 
{
  UpdateSize = 0;
  olxstr SettingsFile( TBasicApp::GetInstance()->BaseDir() + "usettings.dat" );
  TSettingsFile settings;
  uint64_t LastUpdate = 0;
  Valid = false;
  Update = false;
  if( TEFile::Exists(SettingsFile) )  {
    olxstr Proxy, Repository, UpdateInterval("Always");

    settings.LoadSettings( SettingsFile );
    if( settings.ParamExists("proxy") )        Proxy = settings.ParamValue("proxy");
    if( settings.ParamExists("repository") )   Repository = settings.ParamValue("repository");
    if( settings.ParamExists("lastupdate") )   LastUpdate = settings.ParamValue("lastupdate", '0').RadInt<uint64_t>();
    if( settings.ParamExists("update") )       UpdateInterval = settings.ParamValue("update");

    if( Repository.Length() && !Repository.EndsWith('/') )  Repository << '/';

    if( settings.ParamExists("exceptions") )
      extensionsToSkip.Strtok(settings.ParamValue("exceptions", EmptyString), ';');
    if( settings.ParamExists("skip") )
      filesToSkip.Strtok(settings.ParamValue("skip", EmptyString), ';');

    toSkip.extsToSkip = extensionsToSkip.IsEmpty() ? NULL : &extensionsToSkip;
    toSkip.filesToSkip = filesToSkip.IsEmpty() ? NULL : &filesToSkip;
    if( TEFile::ExtractFileExt(Repository).Equalsi("zip") )  {
      if( !TEFile::IsAbsolutePath(Repository) )
        Repository = TBasicApp::GetInstance()->BaseDir() + Repository;
      if( !TEFile::Exists(Repository) ) 
        return;
      if( TEFile::FileAge(Repository) > LastUpdate )  {
        Update = true;
        srcFS = new TwxZipFileSystem(Repository, false);
      }
    }
    else if( TEFile::Exists(Repository) && TEFile::IsDir(Repository) )  {
      Update = true;
      srcFS = new TOSFileSystem(Repository);
    }
    else  {
      if( UpdateInterval.Equalsi("Always") )  Update = true;
      else if( UpdateInterval.Equalsi("Daily") )
        Update = ((TETime::EpochTime() - LastUpdate ) > SecsADay );
      else if( UpdateInterval.Equalsi("Weekly") )
        Update = ((TETime::EpochTime() - LastUpdate ) > SecsADay*7 );
      else if( UpdateInterval.Equalsi("Monthly") )
        Update = ((TETime::EpochTime() - LastUpdate ) > SecsADay*30 );
      if( Update )  {
        TUrl url(Repository);
        if( !Proxy.IsEmpty() )  
          url.SetProxy( TUrl(Proxy) );
        srcFS = new TwxHttpFileSystem(url);
      }
    }
    destFS = new TOSFileSystem( TBasicApp::GetInstance()->BaseDir() );
    properties.Add("olex-update");
#ifdef __WIN32__
    properties.Add("port-win32");
#else
    olxstr olex_port = settings.ParamValue("olex-port");
    if( !olex_port.IsEmpty() )
      props.Add(olex_port);
#endif
  }
  Valid = true;
}
//....................................................................................
int UpdateThread::Run()  {
  if( !Valid || !Update )
    return 0;
  // wait for ~3 minutes
  //while( time_out < 50*20*3*60 )  {
  while( time_out < 50*2*60 )  {
    TBasicApp::Sleep(50);
    time_out += 50;
  }
  if( !TBasicApp::HasInstance() || Terminate || srcFS == NULL || destFS == NULL )
    return 0;

  TFSIndex fsi( *srcFS );
  try  {
    TOSFileSystem dfs(PatchDir);
    TStrList cmds;
    bool skip = (extensionsToSkip.IsEmpty() && filesToSkip.IsEmpty());
    UpdateSize = fsi.CalcDiffSize(*destFS, properties, skip ? NULL : &toSkip);
    if( UpdateSize != 0 )
      Update = false;
    else
      return 0;
    while( !Update )  {
      if( Terminate )
        return 0;
      if( !TBasicApp::HasInstance() )  {  // nobody took care ?
        delete srcFS;
        srcFS = NULL;
        delete destFS;
        destFS = NULL;
        return 0;
      }
      TBasicApp::Sleep(100);
    }
    fsi.Synchronise(*destFS, properties, skip ? NULL : &toSkip, &dfs, &cmds);
    TUtf8File::WriteLines(dfs.GetBase() + "__cmds__", cmds);
  }
  catch(...)  { return 0;  }  // oups...
  return 1;
}

