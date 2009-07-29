#include "mainform.h"

#include "updateth.h"
#include "updateapi.h"
#include "patchapi.h"
#include "log.h"


UpdateThread::UpdateThread(const olxstr& patch_dir) : time_out(0), PatchDir(patch_dir), 
    srcFS(NULL), destFS(NULL), Index(NULL), _DoUpdate(false), UpdateSize(0)  
{ 
	OnDownload = &Actions.NewQueue("ON_DOWNLOAD");
}
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
    srcFS->OnProgress->Add( new DListenerProxy(*OnDownload) );
  }
  catch(const TExceptionBase& exc)  {
    if( TBasicApp::HasInstance() )
      TBasicApp::GetLog().Info( exc.GetException()->GetFullMessage() );
  }
}
//....................................................................................
int UpdateThread::Run()  {
  // wait for ~3 minutes
  //while( time_out < 50*20*3*60 )  {
  while( time_out < 50*20*1 )  {
    TBasicApp::Sleep(50);
    if( Terminate )  {
      CleanUp();
      return 0;
    }
    time_out += 50;
  }
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
    TBasicApp::Sleep(100);
    if( Terminate || !TBasicApp::HasInstance() )  {
      CleanUp();
      return 0;
    }
  }
  try  {
    TOSFileSystem dfs(PatchDir);
    TStrList cmds;
    bool skip = (extensionsToSkip.IsEmpty() && filesToSkip.IsEmpty());
    UpdateSize = Index->CalcDiffSize(*destFS, properties, skip ? NULL : &toSkip);
    patcher::PatchAPI::UnlockUpdater();
    if( UpdateSize == 0 )
      return 0;
    while( !_DoUpdate )  {
      if( Terminate || !TBasicApp::HasInstance() )  {  // nobody took care ?
        CleanUp();
        patcher::PatchAPI::UnlockUpdater(); // safe to call without app instance
        return 0;
      }
      TBasicApp::Sleep(100);
    }
#ifdef __WIN32__
    olxstr updater_file( dfs.GetBase() + "olex2.exe");
#else
    olxstr updater_file( dfs.GetBase() + "unirun");
#endif
    // download completion file
    olxstr download_vf(patcher::PatchAPI::GetUpdateLocationFileName());
    if( TEFile::Exists(download_vf) ) // do not run subsequent temporary updates
      return 0;
  // try to lock updateAPI for update
    while( !patcher::PatchAPI::LockUpdater() )  {
      TBasicApp::Sleep(100);
      if( Terminate || !TBasicApp::HasInstance() )  {
        CleanUp();
        return 0;
      }
    }
    Index->Synchronise(*destFS, properties, skip ? NULL : &toSkip, &dfs, &cmds);
    // try to update the updater, should check the name of executable though!
    if( dfs.Exists(updater_file) )
      TEFile::Rename(updater_file, TBasicApp::GetBaseDir() + TEFile::ExtractFileName(updater_file) );
    
    olxstr cmd_fn( TEFile::ParentDir(dfs.GetBase()) + patcher::PatchAPI::GetUpdaterCmdFileName());
    if( TEFile::Exists(cmd_fn) )  {
      TStrList pc;
      TUtf8File::ReadLines(cmd_fn, pc);
      cmds.Insert(0, pc);
    }
    TUtf8File::WriteLines(cmd_fn, cmds);
    // mark download as complete
    if( !Index->IsInterrupted() )  {
      //TBasicApp::GetLog().Info("Done update downloading");
      TEFile f(download_vf, "w+b");
      CString location(dfs.GetBase());
      f.Write(location);
    }
    else  {
      //TBasicApp::GetLog().Info("Update downloading interrupted, will continue in the new session");
    }
    patcher::PatchAPI::UnlockUpdater();
  }
  catch( const TExceptionBase& exc)  { // oups...
    CleanUp();
    patcher::PatchAPI::UnlockUpdater();
    //TBasicApp::GetLog().Info("Update failed...");
    //TBasicApp::GetLog().Info(exc.GetException()->GetFullMessage());
    return 0;  
  }  
  return 1;
}
//....................................................................................
void UpdateThread::OnSendTerminate()  {
  if( Index != NULL )
    Index->DoBreak();
}
