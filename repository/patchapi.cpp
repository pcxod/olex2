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
#include <sys/stat.h>

using namespace patcher;

TEFile* PatchAPI::lock_file = NULL;

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
      ft.OnFileCopy->Add(OnFileCopy);
    if( OnOverallCopy != NULL )
      ft.OnSynchronise->Add(OnOverallCopy);
    OnFileCopy = NULL;
    OnOverallCopy = NULL;

    try  {  ft.CopyTo(TBasicApp::GetBaseDir(), &AfterFileCopy);  }
    catch(PatchAPI::DeletionExc)  {  res = papi_DeleteError;  }
    catch(const TExceptionBase& exc)  {  
      res = papi_CopyError;  
      TBasicApp::NewLogEntry(logError) << exc.GetException()->GetFullMessage();
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
  TEFile::ListDir(TBasicApp::GetBaseDir(), pid_files, olxstr("*.") <<
    GetOlex2PIDFileExt(), sefAll);
  for( size_t i=0; i < pid_files.Count(); i++ )  {
    if( TEFile::DelFile(TBasicApp::GetBaseDir() + pid_files[i]) )
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
  olxstr tag_fn = (base_dir.IsEmpty() ? TBasicApp::GetBaseDir() : base_dir) +
    GetTagFileName();
  if( !TEFile::Exists(tag_fn) )
    return EmptyString();
  TStrList sl;
  sl.LoadFromFile(tag_fn);
  return sl.Count() == 1 ? sl[0] : EmptyString();
}
//.............................................................................
olxstr PatchAPI::_GetSharedDirRoot()  {
  const olxstr dd_str = olx_getenv("OLEX2_DATADIR");
  olxstr data_dir;
  if( !dd_str.IsEmpty() )  {
    data_dir = dd_str;
    if( !TEFile::IsDir(data_dir) )
      data_dir.SetLength(0);
  }
  if( data_dir.IsEmpty() )
    data_dir = TShellUtil::GetSpecialFolderLocation(fiAppData);
  return TEFile::AddPathDelimeterI(data_dir);
}
//.............................................................................
olxstr PatchAPI::GetCurrentSharedDir(olxstr* DataDir)  {
  olxstr data_dir = _GetSharedDirRoot();
  if( DataDir != NULL )
    *DataDir = data_dir;
  if( olx_getenv("OLEX2_DATADIR_STATIC").Equalsi("TRUE") )
     return data_dir;
  return ComposeNewSharedDir(data_dir);
}
//.............................................................................
