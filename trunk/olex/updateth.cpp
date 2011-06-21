#include "mainform.h"
#include "updateth.h"
#include "updateapi.h"
#include "patchapi.h"
#include "log.h"

UpdateThread::UpdateThread(const olxstr& patch_dir) : time_out(0), PatchDir(patch_dir), 
  srcFS(NULL), destFS(NULL), Index(NULL), _DoUpdate(false), UpdateSize(0),
  OnDownload(Actions.New("ON_DOWNLOAD")),
  OnAction(Actions.New("ON_ACTION"))  {}
//....................................................................................
void UpdateThread::DoInit()  {
  if( !TBasicApp::HasInstance() || Terminate ) 
    return;
  try  {
    updater::UpdateAPI uapi;
    srcFS = uapi.FindActiveUpdateRepositoryFS(NULL);
    if( srcFS == NULL )  return;
    Index = new TFSIndex(*srcFS);
    destFS = new TOSFileSystem( TBasicApp::GetBaseDir() );
    uapi.EvaluateProperties(properties);
    srcFS->OnProgress.Add( new TActionProxy(OnDownload) );
    Index->OnAction.Add( new TActionProxy(OnAction) );
  }
  catch(const TExceptionBase& exc)  {
    if( TBasicApp::HasInstance() )
      TBasicApp::NewLogEntry(logInfo) << exc.GetException()->GetFullMessage();
  }
}
//....................................................................................
int UpdateThread::Run()  {
  DoInit();
  if( !TBasicApp::HasInstance() || Terminate || 
    srcFS == NULL || destFS == NULL || Index == NULL )  
  {
    CleanUp();
    return 0;
  }
  if( TEFile::Exists(patcher::PatchAPI::GetUpdateLocationFileName()) )
    return 0;
  // try to lock updateAPI
  while( !patcher::PatchAPI::LockUpdater() )  {
    olx_sleep(100);
    if( Terminate || !TBasicApp::HasInstance() )  {
      CleanUp();
      return 0;
    }
  }
  try  {
    TOSFileSystem dfs(PatchDir);
    TStrList cmds;
    bool skip = (extensionsToSkip.IsEmpty() && filesToSkip.IsEmpty());
    // need to keep to check if sync was completed
    const uint64_t update_size = Index->CalcDiffSize(*destFS, properties, skip ? NULL : &toSkip);
    UpdateSize = update_size;
    patcher::PatchAPI::UnlockUpdater();
    if( UpdateSize == 0 )  return 0;
    while( !_DoUpdate )  {
      if( Terminate || !TBasicApp::HasInstance() )  {  // nobody took care ?
        CleanUp();
        patcher::PatchAPI::UnlockUpdater(); // safe to call without app instance
        return 0;
      }
      olx_sleep(100);
    }
#ifdef __WIN32__
    olxstr updater_file = dfs.GetBase() + "olex2.exe";
#else
    olxstr updater_file = dfs.GetBase() + "unirun";
#endif
    // download completion file
    olxstr download_vf(patcher::PatchAPI::GetUpdateLocationFileName());
    if( TEFile::Exists(download_vf) ) // do not run subsequent temporary updates
      return 0;
  // try to lock updateAPI for update
    while( !patcher::PatchAPI::LockUpdater() )  {
      olx_sleep(100);
      if( Terminate || !TBasicApp::HasInstance() )  {
        CleanUp();
        return 0;
      }
    }
    bool completed = false;
    try {  
      if( Index->Synchronise(*destFS, properties, skip ? NULL : &toSkip, &dfs, &cmds) == update_size )
        completed = true;
    }
    catch(const TExceptionBase&)  {}
    // try to update the updater, should check the name of executable though!
    if( dfs.Exists(updater_file) )  {
      try  {
        olxstr dest = TBasicApp::GetBaseDir() + TEFile::ExtractFileName(updater_file);
        if( !TEFile::Rename(updater_file, dest) )  {  // are on different disks?
          if( TEFile::Copy(updater_file, dest) )
            TEFile::DelFile(updater_file);
        }
        TEFile::Chmod(dest, S_IEXEC|S_IWRITE|S_IREAD);
      }
      catch(...) {}
    }
    if( completed )  {
      olxstr cmd_fn(TEFile::ParentDir(dfs.GetBase()) + patcher::PatchAPI::GetUpdaterCmdFileName());
      if( TEFile::Exists(cmd_fn) )  {
        TStrList pc;
#ifdef _UNICODE
        TUtf8File::ReadLines(cmd_fn, pc);
#else
        pc.LoadFromFile(cmd_fn);
#endif
        cmds.Insert(0, pc);
      }
#ifdef _UNICODE
      TUtf8File::WriteLines(cmd_fn, cmds);
#else
      cmds.SaveToFile(cmd_fn);
#endif
      // mark download as complete
      TEFile f(download_vf, "w+b");
      olxcstr location(TEFile::CreateRelativePath(dfs.GetBase(), TEFile::ExtractFilePath(download_vf)));
      f.Write(location);
    }
    patcher::PatchAPI::UnlockUpdater();
  }
  catch(const TExceptionBase&)  { // oups...
    CleanUp();
    patcher::PatchAPI::UnlockUpdater();
    //TBasicApp::NewLogEntry(logInfo)("Update failed...");
    //TBasicApp::NewLogEntry(logInfo)(exc.GetException()->GetFullMessage());
    return 0;  
  }  
  return 1;
}
//....................................................................................
void UpdateThread::OnSendTerminate()  {
  if( Index != NULL )
    Index->DoBreak();
}
