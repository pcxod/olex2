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
#include "zipfs.h"

UpdateThread::UpdateThread(const olxstr& patch_dir, bool force_update,
  bool reinstall, bool cleanup)
  : time_out(0),
  PatchDir(patch_dir),
  _DoUpdate(false), UpdateSize(0),
  OnDownload(Actions.New("ON_DOWNLOAD")),
  OnAction(Actions.New("ON_ACTION"))
{
  ForceUpdate = force_update;
  this->reinstall = reinstall;
  this->cleanup = cleanup;
}
//.............................................................................
void UpdateThread::DoInit() {
  if (!TBasicApp::HasInstance() || Terminate()) {
    return;
  }
  try {
    if (TEFile::Exists(patcher::PatchAPI::GetUpdateLocationFileName())) {
      return;
    }
    updater::UpdateAPI uapi;
    srcFS = uapi.FindActiveRepositoryFS(0, ForceUpdate, !reinstall);
    if (!srcFS.ok()) {
      return;
    }
    if (reinstall) {
      destFS = new TUpdateFS(PatchDir,
        *(new TOSFileSystem(TBasicApp::GetBaseDir())));
      properties = uapi.EvaluateProperties();
      srcFS->OnProgress.Add(new TActionProxy(OnDownload));

      return;
    }
    Index = new TFSIndex(*srcFS);
    destFS = new TUpdateFS(PatchDir,
      *(new TOSFileSystem(TBasicApp::GetBaseDir())));
    properties = uapi.EvaluateProperties();
    srcFS->OnProgress.Add(new TActionProxy(OnDownload));
    Index->OnAction.Add(new TActionProxy(OnAction));
  }
  catch (const TExceptionBase& exc) {
    if (TBasicApp::HasInstance()) {
      TBasicApp::NewLogEntry(logExceptionTrace) << exc;
    }
  }
}
//.............................................................................
int UpdateThread::Run() {
  DoInit();
  if (!TBasicApp::HasInstance() || Terminate() ||
    !srcFS.ok() || !destFS.ok() ||
    (!reinstall && !Index.ok()))
  {
    CleanUp();
    return 0;
  }
  // try to lock updateAPI
  while (!patcher::PatchAPI::LockUpdater()) {
    olx_sleep(100);
    if (Terminate() || !TBasicApp::HasInstance()) {
      CleanUp();
      return 0;
    }
  }
  if (reinstall) {
    olxstr inst_fn = updater::UpdateAPI::GetInstallationFileName();
    updater::UpdateAPI uapi;
    olx_object_ptr<IInputStream> s = srcFS->OpenFile(srcFS->GetBase() + inst_fn);
    if (!s.ok()) {
      TBasicApp::NewLogEntry(logError) << "Could not locate: " << inst_fn;
      return 0;
    }
    destFS->AdoptStream(*s, destFS->GetBase() + inst_fn);
    olx_object_ptr<AZipFS> zp = ZipFSFactory::GetInstance(destFS->GetBase() + inst_fn, false);
    if (zp->ExtractAll(destFS->GetBase())) {
      TEFile::DelFile(destFS->GetBase() + inst_fn);
      TEFile rfn(uapi.GetReinstallFileName(), "w+b");
      patcher::PatchAPI::MarkPatchComplete();
    }
    return 1;
  }
  try {
    TStrList cmds;
    bool skip = (extensionsToSkip.IsEmpty() && filesToSkip.IsEmpty());
    // need to keep to check if sync was completed
    const uint64_t update_size =
      Index->CalcDiffSize(*destFS, properties, skip ? 0 : &toSkip);
    UpdateSize = update_size;
    patcher::PatchAPI::UnlockUpdater();
    if (UpdateSize == 0) {
      // complete interupted update
      if (TEFile::Exists(destFS->GetBase() + "index.ind")) {
        MarkCompleted(cmds);
      }
      return 0;
    }
    while (!_DoUpdate) {
      if (Terminate() || !TBasicApp::HasInstance()) {  // nobody took care ?
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
    if (TEFile::Exists(download_vf)) {
      return 0;
    }
    // try to lock updateAPI for update
    while (!patcher::PatchAPI::LockUpdater()) {
      olx_sleep(100);
      if (Terminate() || !TBasicApp::HasInstance()) {
        CleanUp();
        return 0;
      }
    }
    bool completed = false;
    try {
      if (Index->Synchronise(*destFS, properties, skip ? 0
        : &toSkip, &cmds) == update_size)
      {
        completed = true;
      }
    }
    catch (const TExceptionBase&) {}
    if (completed) {
      MarkCompleted(cmds);
    }
    patcher::PatchAPI::UnlockUpdater();
  }
  catch (const TExceptionBase&) { // oups...
    CleanUp();
    patcher::PatchAPI::UnlockUpdater();
    return 0;
  }
  return 1;
}
//.............................................................................
void UpdateThread::CleanUp() {
  Index = 0; // depends on srcFS - should go first
  srcFS = 0;
  destFS = 0;
}
//.............................................................................
void UpdateThread::OnSendTerminate()  {
  if (Index.ok()) {
    Index->DoBreak();
  }
}
//.............................................................................
void UpdateThread::MarkCompleted(const TStrList &cmds_) {
  TStrList cmds(cmds_);
  TOSFileSystem dfs(PatchDir);
  olxstr cmd_fn(TEFile::ParentDir(dfs.GetBase()) +
    patcher::PatchAPI::GetUpdaterCmdFileName());
  if( TEFile::Exists(cmd_fn) )  {
    TStrList pc;
#ifdef _UNICODE
    pc = TUtf8File::ReadLines(cmd_fn);
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
  patcher::PatchAPI::MarkPatchComplete();
}
//.............................................................................
