#include "mainform.h"

#include "updateth.h"
#include "updateapi.h"


UpdateThread::UpdateThread(const olxstr& patch_dir) : time_out(0), PatchDir(patch_dir), 
    srcFS(NULL), destFS(NULL), Index(NULL) 
{
  UpdateSize = 0;
  updater::UpdateAPI uapi;
  Valid = false;
  Update = false;
  srcFS = uapi.GetRepositoryFS();
  if( srcFS == NULL )
    return;
  Index = new TFSIndex(*srcFS);
  destFS = new TOSFileSystem( TBasicApp::GetInstance()->BaseDir() );
  uapi.EvaluateProperties(properties);
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
    if( UpdateSize != 0 )
      Update = false;
    else
      return 0;
    while( !Update )  {
      if( Terminate )
        return 0;
      if( !TBasicApp::HasInstance() )  {  // nobody took care ?
        CleanUp();
        return 0;
      }
      TBasicApp::Sleep(100);
    }
    // download completion file
    olxstr download_vf( TBasicApp::GetInstance()->BaseDir() + "__completed.update");
    if( TEFile::Exists(download_vf) )
      TEFile::DelFile(download_vf);
    Index->Synchronise(*destFS, properties, skip ? NULL : &toSkip, &dfs, &cmds);
    olxstr cmd_fn(TBasicApp::GetInstance()->BaseDir() + "__cmds.update");
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
