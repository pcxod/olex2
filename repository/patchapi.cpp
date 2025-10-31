/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "patchapi.h"
#include "bapp.h"
#include "efile.h"
#include "log.h"
#include "utf8file.h"
#include "egc.h"
#include "filetree.h"
#include "updateapi.h"
#include "shellutil.h"
#include "eutf8.h"
#include <sys/stat.h>

using namespace patcher;
using namespace updater;

short PatchAPI::DoPatch(olx_object_ptr<AActionHandler> OnFileCopy,
  olx_object_ptr<AActionHandler> OnOverallCopy)
{
  if (!TBasicApp::IsBaseDirWriteable()) {
    return papi_AccessDenied;
  }
  if (!TEFile::Exists(GetUpdateLocationFileName())) {
    return papi_OK;
  }
  olxstr patch_dir = GetUpdateLocation();
  if (patch_dir.IsEmpty()) {
    TEFile::DelFile(GetUpdateLocationFileName());
    return papi_InvalidUpdate;
  }
  olxstr cmd_file(TEFile::ParentDir(patch_dir) + GetUpdaterCmdFileName());
  // make sure that only one instance is running
  if (!LockUpdater() || IsOlex2Running()) {
    return papi_Busy;
  }
  // clean...
  short res = papi_OK;
  if (TEFile::Exists(cmd_file)) {
    TStrList cmdl = TEFile::ReadLines(cmd_file);
    for (size_t i = 0; i < cmdl.Count(); i++) {
      if (cmdl[i].StartsFrom("rmdir")) {
        TBasicApp::NewLogEntry(logInfo) << cmdl[i];
        olxstr dp = TEFile::JoinPath(TStrList() << TBasicApp::GetBaseDir()
          << cmdl[i].SubStringFrom(5).TrimWhiteChars());
        if (TEFile::Exists(dp) && TEFile::IsDir(dp)) {
          TEFile::DeleteDir(dp, false, true);
        }
      }
      if (cmdl[i].StartsFrom("rm")) {
        TBasicApp::NewLogEntry(logInfo) << cmdl[i];
        olxstr dp = TEFile::JoinPath(TStrList() << TBasicApp::GetBaseDir()
          << cmdl[i].SubStringFrom(2).TrimWhiteChars());
        if (TEFile::Exists(dp) && !TEFile::IsDir(dp)) {
          TEFile::DelFile(dp);
        }
      }
    }
    TEFile::DelFile(cmd_file);
  }
  // copy...
  if (res == papi_OK) {
    TFileTree ft(patch_dir);
    ft.Expand();
    if (OnFileCopy.ok()) {
      ft.OnFileCopy.Add(OnFileCopy.release());
    }
    if (OnOverallCopy.ok()) {
      ft.OnSynchronise.Add(OnOverallCopy.release());
    }
    try {
      ft.CopyTo(TBasicApp::GetBaseDir(), &AfterFileCopy);
    }
    catch (PatchAPI::DeletionExc) {
      res = papi_DeleteError;
    }
    catch (const TExceptionBase& exc) {
      res = papi_CopyError;
      TBasicApp::NewLogEntry(logException) << exc;
    }

    if (res == papi_OK) {
      try {
        TEFile::DelFile(GetUpdateLocationFileName());
        TEFile::DeleteDir(patch_dir);
      }
      catch (...) {
        res = papi_DeleteError;
      }
    }
    if (res == papi_OK) {
      updater::SettingsFile sf(updater::UpdateAPI::GetSettingsFileName());
      sf.last_updated = TETime::EpochTime();
      sf.Save();
    }
  }
  _RestoreExecuableFlags();
  UnlockUpdater();
  return res;
}
//.........................................................................
olxstr PatchAPI::GetUpdateLocation() {
  olxstr update_location = GetUpdateLocationFileName();
  if (TEFile::Exists(update_location)) {
    TCStrList fc = TEFile::ReadCLines(update_location);
    if (fc.Count() == 1) {
      olxstr path = TUtf8::Decode(fc[0]);
      if (TEFile::IsAbsolutePath(path)) {
        return fc[0];
      }
      return TEFile::ExpandRelativePath(path, TBasicApp::GetBaseDir());
    }
    else if (fc.IsEmpty()) {
      return TBasicApp::GetInstanceDir() + GetPatchFolder();
    }
  }
  return EmptyString();
}
//.........................................................................
void PatchAPI::_RestoreExecuableFlags() {
#if !defined(__WIN32__)
  TStrList file_list;
  file_list << "olex2_exe" << "olex2";
  for (size_t i=0; i < file_list.Count(); i++) {
    olxstr fn = TBasicApp::GetBaseDir() + file_list[i];
    if (TEFile::Exists(fn)) {
      TEFile::Chmod(fn, S_IEXEC | S_IREAD | S_IWRITE);
    }
  }
#endif
}
//.........................................................................
size_t PatchAPI::GetNumberOfOlex2Running() {
  TStrList pid_files;
  TEFile::ListDir(TBasicApp::GetInstanceDir(), pid_files, olxstr("*.") <<
    GetOlex2PIDFileExt(), sefAll);
  for (size_t i = 0; i < pid_files.Count(); i++) {
    if (TEFile::DelFile(TBasicApp::GetInstanceDir() + pid_files[i])) {
      pid_files[i].SetLength(0);
    }
  }
  pid_files.Pack();
  return pid_files.Count();
}
//.........................................................................
bool PatchAPI::LockUpdater() {
  if (LockFile() != 0) {
    return false;
  }
  try {
    if (TEFile::Exists(GetUpdaterPIDFileName())) {
      if (!TEFile::DelFile(GetUpdaterPIDFileName())) {
        return false;
      }
    }
    LockFile() = new TEFile(GetUpdaterPIDFileName(), "w+");
  }
  catch (...) {
    return false;
  }
  return true;
}
//.............................................................................
bool PatchAPI::UnlockUpdater() {
  if (LockFile() == 0) {
    return true;
  }
  if (!TBasicApp::HasInstance()) { // have to make sure the TEGC exists...
    TEGC::Initialise();
  }
  LockFile()->Delete();
  delete LockFile();
  LockFile() = 0;
  TEGC::Finalise();
  return true;
}
//.............................................................................
olxstr PatchAPI::ReadRepositoryTag(const olxstr& base_dir)  {
  if (RepositoryBase() == base_dir && !base_dir.IsEmpty()) {
    return RepositoryTag();
  }
  RepositoryBase() = base_dir.IsEmpty() ? TBasicApp::GetBaseDir()
    : base_dir;
  olxstr tag_fn = RepositoryBase() + GetTagFileName();
  if (!TEFile::Exists(tag_fn)) {
    return (RepositoryTag() = EmptyString());
  }
  TStrList sl = TEFile::ReadLines(tag_fn);
  return sl.Count() == 1 ? (RepositoryTag () = sl[0]) : EmptyString();
}
//.............................................................................
//.............................................................................
//.............................................................................
PatchAPI::DataDirSettings::DataDirSettings()
  : is_manually_set(false)
{
  Refresh();
}
//.............................................................................
bool PatchAPI::DataDirSettings::Refresh() {
  if (is_manually_set) {
    return false;
  }
  is_static = olx_getenv("OLEX2_DATADIR_STATIC").Equalsi("TRUE");
  data_dir = olx_getenv("OLEX2_DATADIR");
  if (!data_dir.IsEmpty()) {
    TEFile::AddPathDelimeterI(data_dir);
  }
  return true;
}
//.............................................................................
//.............................................................................
//.............................................................................
olxstr PatchAPI::GetSharedDir(bool refresh) {
  olxstr rv = TEFile::AddPathDelimeter(_GetSharedDirRoot(refresh));
  if (GetDDSetting().is_static) {
    return rv;
  }
#ifdef __WIN32__
  return rv << "Olex2Data/";
#else
  return rv << "data/";
#endif
}
//.............................................................................
olxstr PatchAPI::_GetSharedDirRoot(bool refresh) {
  if (!refresh && !SharedDir().IsEmpty()) {
    return SharedDir();
  }
  if (GetDDSetting().is_manually_set) {
    return GetDDSetting().data_dir;
  }
  if (refresh) {
    GetDDSetting().Refresh();
  }
  const olxstr dd_str = GetDDSetting().data_dir;
  olxstr data_dir;
  if (!dd_str.IsEmpty()) {
    data_dir = dd_str;
    if (!TEFile::IsDir(data_dir)) {
      data_dir.SetLength(0);
    }
  }
  if (data_dir.IsEmpty()) {
    data_dir = TShellUtil::GetSpecialFolderLocation(fiAppData);
  }
  return (SharedDir() = TEFile::AddPathDelimeterI(data_dir));
}
//.............................................................................
olxstr PatchAPI::GetInstanceDir(bool refresh) {
  if (!refresh && !InstanceDir().IsEmpty()) {
    return InstanceDir();
  }
  olxstr data_dir = _GetSharedDirRoot(refresh);
  if (GetDDSetting().is_static) {
    return (InstanceDir() = data_dir);
  }
  return (InstanceDir() = ComposeInstanceDir(data_dir));
}
//.............................................................................
void PatchAPI::MarkPatchComplete() {
  TEFile f(GetUpdateLocationFileName(), "wb+");
  f.Write(
    TUtf8::Encode(TBasicApp::GetInstanceDir() +
      patcher::PatchAPI::GetPatchFolder())
  );
}
//.............................................................................
//.............................................................................
//.............................................................................
void SettingsFile::Init(const olxstr& file_name) {
  source_file = file_name;
  if (!TEFile::Exists(file_name)) return;
  const TSettingsFile settings(file_name);
  proxy = settings["proxy"];
  repository = settings["repository"];
  update_interval = settings["update"];
  const olxstr last_update_str = settings.GetParam("lastupdate", "0");
  last_updated = last_update_str.IsEmpty() ? 0 : last_update_str.RadInt<int64_t>();
  extensions_to_skip.Strtok(settings["exceptions"], ';');
  dest_repository = settings["dest_repository"];
  src_for_dest = settings["src_for_dest"];
  files_to_skip.Strtok(settings["skip"], ';');
  olex2_port = settings["olex-port"];
  ask_for_update = settings.GetParam("ask_update", TrueString()).ToBool();
}
//.............................................................................
// save will change repo/update/ to repo...
bool SettingsFile::Save() const {
  TSettingsFile settings;
  settings["proxy"] = proxy;
  settings["repository"] = repository;
  settings["update"] = update_interval;
  settings["lastupdate"] = last_updated;
  settings["exceptions"] = extensions_to_skip.Text(';');
  settings["dest_repository"] = dest_repository;
  settings["src_for_dest"] = src_for_dest;
  settings["skip"] = files_to_skip.Text(';');
  settings["olex-port"] = olex2_port;
  settings["ask_update"] = ask_for_update;
  try { settings.SaveSettings(source_file); }
  catch (...) {
    return false;
  }
  return true;
}
