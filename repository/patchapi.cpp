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

TEFile* PatchAPI::lock_file = NULL;
olxstr
  PatchAPI::repository_tag, PatchAPI::repository_base_dir,
  PatchAPI::shared_dir,
  PatchAPI::instance_dir;

short PatchAPI::DoPatch(AActionHandler* OnFileCopy,
  AActionHandler* OnOverallCopy)
{
  if( !TBasicApp::IsBaseDirWriteable() )
    return papi_AccessDenied;
  if( !TEFile::Exists(GetUpdateLocationFileName()) )  {
    CleanUp(OnFileCopy, OnOverallCopy);
    return papi_OK;
  }
  olxstr patch_dir = GetUpdateLocation();
  if( patch_dir.IsEmpty() )  {
    TEFile::DelFile(GetUpdateLocationFileName());
    CleanUp(OnFileCopy, OnOverallCopy);
    return papi_InvalidUpdate;
  }
  olxstr cmd_file(TEFile::ParentDir(patch_dir) + GetUpdaterCmdFileName());
  // make sure that only one instance is running
  if( !LockUpdater() || IsOlex2Running() )  {
    CleanUp(OnFileCopy, OnOverallCopy);
    return papi_Busy;
  }
  // clean...
  short res = papi_OK;
  if( TEFile::Exists(cmd_file) )  {
    TEFile::DelFile(cmd_file);
  }
  // copy...
  if( res == papi_OK )  {
    TFileTree ft(patch_dir);
    ft.Expand();
    if( OnFileCopy != NULL )
      ft.OnFileCopy.Add(OnFileCopy);
    if( OnOverallCopy != NULL )
      ft.OnSynchronise.Add(OnOverallCopy);
    OnFileCopy = NULL;
    OnOverallCopy = NULL;

    try  {  ft.CopyTo(TBasicApp::GetBaseDir(), &AfterFileCopy);  }
    catch(PatchAPI::DeletionExc)  {  res = papi_DeleteError;  }
    catch(const TExceptionBase& exc)  {  
      res = papi_CopyError;  
      TBasicApp::NewLogEntry(logException) << exc;
    }

    if( res == papi_OK )  {
      try  {
        TEFile::DelFile(GetUpdateLocationFileName());
        TEFile::DeleteDir(patch_dir);
      }
      catch(...)  {  res = papi_DeleteError;  }
    }
    if( res == papi_OK )  {
      updater::SettingsFile sf(updater::UpdateAPI::GetSettingsFileName());
      sf.last_updated = TETime::EpochTime();
      sf.Save();
    }
  }
  CleanUp(OnFileCopy, OnOverallCopy);
  _RestoreExecuableFlags();
  UnlockUpdater();
  return res;
}
//.........................................................................
olxstr PatchAPI::GetUpdateLocation()  {
  olxstr update_location = GetUpdateLocationFileName();
  if( TEFile::Exists(update_location) )  {
    TCStrList fc;
    fc.LoadFromFile(update_location);
    if( fc.Count() == 1 ) {
      olxstr path = TUtf8::Decode(fc[0]);
      if (TEFile::IsAbsolutePath(path))
        return fc[0];
      return TEFile::ExpandRelativePath(path, TBasicApp::GetBaseDir());
    }
    else if (fc.IsEmpty())
      return TBasicApp::GetInstanceDir() + GetPatchFolder();
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
    if (TEFile::Exists(fn))
      TEFile::Chmod(fn, S_IEXEC|S_IREAD|S_IWRITE);
  }
#endif
}
//.........................................................................
size_t PatchAPI::GetNumberOfOlex2Running()  {
  TStrList pid_files;
  TEFile::ListDir(TBasicApp::GetInstanceDir(), pid_files, olxstr("*.") <<
    GetOlex2PIDFileExt(), sefAll);
  for( size_t i=0; i < pid_files.Count(); i++ )  {
    if( TEFile::DelFile(TBasicApp::GetInstanceDir() + pid_files[i]) )
      pid_files[i].SetLength(0);
  }
  pid_files.Pack();
  return pid_files.Count();
}
//.........................................................................
bool PatchAPI::LockUpdater() {
  if( lock_file != NULL )
    return false;
  try  {  
    if( TEFile::Exists(GetUpdaterPIDFileName()) )
      if( !TEFile::DelFile(GetUpdaterPIDFileName()) )
        return false;
    lock_file = new TEFile(GetUpdaterPIDFileName(), "w+");
  }
  catch(...)  {  return false;  }
  return true;
}
//.............................................................................
bool PatchAPI::UnlockUpdater() {
  if( lock_file == NULL )  return true;
  if( !TBasicApp::HasInstance() ) // have to make sure the TEGC exists...
    TEGC::Initialise();
  lock_file->Delete();
  delete lock_file;
  lock_file = NULL;
  TEGC::Finalise();
  return true;
}
//.............................................................................
olxstr PatchAPI::ReadRepositoryTag(const olxstr& base_dir)  {
  if (repository_base_dir == base_dir)
    return repository_tag;
  repository_base_dir = base_dir.IsEmpty() ? TBasicApp::GetBaseDir()
    : base_dir;
  olxstr tag_fn = repository_base_dir + GetTagFileName();
  if( !TEFile::Exists(tag_fn) )
    return (repository_tag=EmptyString());
  TStrList sl;
  sl.LoadFromFile(tag_fn);
  return sl.Count() == 1 ? (repository_tag=sl[0]) : EmptyString();
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
  if (is_manually_set) return false;
  is_static = olx_getenv("OLEX2_DATADIR_STATIC").Equalsi("TRUE");
  data_dir = olx_getenv("OLEX2_DATADIR");
  if (!data_dir.IsEmpty())
    TEFile::AddPathDelimeterI(data_dir);
  return true;
}
//.............................................................................
//.............................................................................
//.............................................................................
olxstr PatchAPI::GetSharedDir(bool refresh) {
  olxstr rv = TEFile::AddPathDelimeter(_GetSharedDirRoot(refresh));
  if (GetDDSetting().is_static)
    return rv;
#ifdef __WIN32__
  return rv << "Olex2Data/";
#else
  return rv << "data/";
#endif
}
//.............................................................................
olxstr PatchAPI::_GetSharedDirRoot(bool refresh)  {
  if (!refresh && !shared_dir.IsEmpty()) return shared_dir;
  if (GetDDSetting().is_manually_set)
    return GetDDSetting().data_dir;
  if (refresh)
    GetDDSetting().Refresh();
  const olxstr dd_str = GetDDSetting().data_dir;
  olxstr data_dir;
  if( !dd_str.IsEmpty() )  {
    data_dir = dd_str;
    if( !TEFile::IsDir(data_dir) )
      data_dir.SetLength(0);
  }
  if( data_dir.IsEmpty() )
    data_dir = TShellUtil::GetSpecialFolderLocation(fiAppData);
  return (shared_dir=TEFile::AddPathDelimeterI(data_dir));
}
//.............................................................................
olxstr PatchAPI::GetInstanceDir(bool refresh)  {
  if (!refresh && !instance_dir.IsEmpty()) return instance_dir;
  olxstr data_dir = _GetSharedDirRoot(refresh);
  if (GetDDSetting().is_static)
     return (instance_dir=data_dir);
  return (instance_dir=ComposeInstanceDir(data_dir));
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
