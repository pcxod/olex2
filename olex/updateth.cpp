#include "mainform.h"

#include "updateth.h"
#include "updateapi.h"


UpdateThread::UpdateThread(const olxstr& patch_dir) : time_out(0), PatchDir(patch_dir), 
    srcFS(NULL), destFS(NULL), Index(NULL), _DoUpdate(false) 
{
  UpdateSize = 0;
  updater::UpdateAPI uapi;
  srcFS = uapi.FindActiveUpdateRepositoryFS(NULL);
  if( srcFS == NULL )  return;
  Index = new TFSIndex(*srcFS);
  destFS = new TOSFileSystem( TBasicApp::GetInstance()->BaseDir() );
  uapi.EvaluateProperties(properties);
}
//....................................................................................
int UpdateThread::Run()  {
  // wait for ~3 minutes
  //while( time_out < 50*20*3*60 )  {
  while( time_out < 50*20*10 )  {
    TBasicApp::Sleep(50);
    time_out += 50;
  }
  if( !TBasicApp::HasInstance() || Terminate || 
    srcFS == NULL || destFS == NULL || Index == NULL )  
  {
    CleanUp();
    return 0;
  }

  try  {
    TOSFileSystem dfs(PatchDir);
    TStrList cmds;
    bool skip = (extensionsToSkip.IsEmpty() && filesToSkip.IsEmpty());
    UpdateSize = Index->CalcDiffSize(*destFS, properties, skip ? NULL : &toSkip);
    if( UpdateSize == 0 )
      return 0;
    while( !_DoUpdate )  {
      if( Terminate ) return 0;
      if( !TBasicApp::HasInstance() )  {  // nobody took care ?
        CleanUp();
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
    olxstr download_vf( TBasicApp::GetInstance()->GetConfigDir() + "__completed.update");
    if( TEFile::Exists(download_vf) )
      TEFile::DelFile(download_vf);
    Index->Synchronise(*destFS, properties, skip ? NULL : &toSkip, &dfs, &cmds);
    // try to update the updater
    if( dfs.FileExists(updater_file) )  {
      TEFile::Rename(updater_file, TBasicApp::GetInstance()->BaseDir() + TEFile::ExtractFileName(updater_file) );
    }
    olxstr cmd_fn(TBasicApp::GetInstance()->GetConfigDir() + "__cmds.update");
    if( TEFile::Exists(cmd_fn) )  {
      TStrList pc;
      TUtf8File::ReadLines(cmd_fn, pc);
      cmds.Insert(0, pc);
    }
    TUtf8File::WriteLines(cmd_fn, cmds);
    // mark download as complete
    if( !Index->IsInterrupted() )
      TEFile(download_vf, "w+b");
  }
  catch(...)  { // oups...
    CleanUp();
    return 0;  
  }  
  return 1;
}
//....................................................................................
void UpdateThread::OnSendTerminate()  {
  if( Index != NULL )
    Index->DoBreak();
}
