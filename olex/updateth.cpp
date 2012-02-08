/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "mainform.h"
#include "updateth.h"
#include "updateapi.h"
#include "patchapi.h"
#include "log.h"

UpdateThread::UpdateThread(const olxstr& patch_dir) : time_out(0),
  PatchDir(patch_dir), srcFS(NULL), destFS(NULL), Index(NULL),
  _DoUpdate(false), UpdateSize(0),
  OnDownload(Actions.New("ON_DOWNLOAD")),
  OnAction(Actions.New("ON_ACTION"))
{}
//.............................................................................
void UpdateThread::DoInit()  {
  if( !TBasicApp::HasInstance() || Terminate ) 
    return;
  try  {
    if (TEFile::Exists(patcher::PatchAPI::GetUpdateLocationFileName()))
      return;
    // compatibility with older versions!
    olxstr old_lf = TBasicApp::GetBaseDir() +
      "__location.update";
    if (TEFile::Exists(old_lf)) {
      TEFile::Copy(old_lf, patcher::PatchAPI::GetUpdateLocationFileName());
      TEFile::DelFile(old_lf);
      return;
    }
    // end of the compatibility section
    updater::UpdateAPI uapi;
    srcFS = uapi.FindActiveUpdateRepositoryFS(NULL);
    if( srcFS == NULL )  return;
    Index = new TFSIndex(*srcFS);
    destFS = new TOSFileSystem(TBasicApp::GetBaseDir());
    uapi.EvaluateProperties(properties);
    srcFS->OnProgress.Add(new TActionProxy(OnDownload));
    Index->OnAction.Add(new TActionProxy(OnAction));
  }
  catch(const TExceptionBase& exc)  {
    if( TBasicApp::HasInstance() )
      TBasicApp::NewLogEntry(logExceptionTrace) << exc;
  }
}
//.............................................................................
int UpdateThread::Run()  {
  DoInit();
  if( !TBasicApp::HasInstance() || Terminate || 
    srcFS == NULL || destFS == NULL || Index == NULL )  
  {
    CleanUp();
    return 0;
  }
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
    AFileSystem &to = dfs.Exists(dfs.GetBase() + "index.ind") ? dfs : *destFS;
    const uint64_t update_size =
      Index->CalcDiffSize(to, properties, skip ? NULL : &toSkip);
    UpdateSize = update_size;
    patcher::PatchAPI::UnlockUpdater();
    if( UpdateSize == 0 )  return 0;
    while( !_DoUpdate )  {
      if( Terminate || !TBasicApp::HasInstance() )  {  // nobody took care ?
        CleanUp();
        // safe to call without app instance
        patcher::PatchAPI::UnlockUpdater();
        return 0;
      }
      olx_sleep(100);
    }
    // download completion file
    olxstr download_vf(patcher::PatchAPI::GetUpdateLocationFileName());
    // do not run subsequent temporary updates
    if( TEFile::Exists(download_vf) )
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
      if( Index->Synchronise(to, properties, skip ? NULL
            : &toSkip, &to == destFS ? &dfs : NULL, &cmds) == update_size )
      {
        completed = true;
      }
    }
    catch(const TExceptionBase&)  {}
    if( completed )  {
      olxstr cmd_fn(TEFile::ParentDir(dfs.GetBase()) +
        patcher::PatchAPI::GetUpdaterCmdFileName());
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
      f.Write(TUtf8::Encode(PatchDir));
    }
    patcher::PatchAPI::UnlockUpdater();
  }
  catch(const TExceptionBase&)  { // oups...
    CleanUp();
    patcher::PatchAPI::UnlockUpdater();
    return 0;  
  }  
  return 1;
}
//.............................................................................
void UpdateThread::OnSendTerminate()  {
  if( Index != NULL )
    Index->DoBreak();
}
