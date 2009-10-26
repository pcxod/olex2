#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "patchapi.h"
#include "bapp.h"
#include "efile.h"
#include "log.h"
#include "utf8file.h"
#include "egc.h"
#include "filetree.h"
#include "updateapi.h"

using namespace patcher;

TEFile* PatchAPI::lock_file = NULL;

short PatchAPI::DoPatch(AActionHandler* OnFileCopy, AActionHandler* OnOverallCopy)  {
  if( !TBasicApp::IsBaseDirWriteable() )
    return papi_AccessDenied;
  if( !TEFile::Exists( GetUpdateLocationFileName() ) )  {
    CleanUp(OnFileCopy, OnOverallCopy);
    return papi_OK;
  }
  olxstr patch_dir = GetUpdateLocation();
  if( patch_dir.IsEmpty() )  {
    TEFile::DelFile( GetUpdateLocationFileName() );
    CleanUp(OnFileCopy, OnOverallCopy);
    return papi_InvalidUpdate;
  }
  olxstr cmd_file(TEFile::ParentDir(patch_dir) + GetUpdaterCmdFileName());
  // make sure that only one instance is running
  if( !LockUpdater() || IsOlex2Running() )  {
    CleanUp(OnFileCopy, OnOverallCopy);
    return papi_Busy;
  }
  // clean
  short res = papi_OK;
  if( TEFile::Exists(cmd_file) )  {
    TWStrList _cmds;
    TUtf8File::ReadLines(cmd_file, _cmds);
    TStrList cmds(_cmds);
    for( size_t i=0; i < cmds.Count(); i++ )  {
      if( cmds[i].StartsFrom("rm ") )  {
        olxstr fdn = cmds[i].SubStringFrom(3).Trim('\'');
        if( !TEFile::Exists(fdn) )  {
          cmds[i].SetLength(0);
          continue;
        }
        if( TEFile::IsDir(fdn) )  {
          if( !TEFile::DeleteDir(fdn) )  {
            res = papi_DeleteError;
            break;  // next time then...
          }
        }
        else  {
          if( !TEFile::DelFile(fdn) )  {
            res = papi_DeleteError;
            break; // next time then...
          }
        }
        cmds[i].SetLength(0);
      }
    }
    if( res == papi_OK )
      TEFile::DelFile(cmd_file);
    else  {
      cmds.Pack();
      TUtf8File::WriteLines(cmd_file, cmds, true);
    }
  }
  // copy...
  if( res == papi_OK )  {
    TFileTree ft(patch_dir);
    ft.Expand();
    if( OnFileCopy != NULL )
      ft.OnFileCopy->Add( OnFileCopy );
    if( OnOverallCopy != NULL )
      ft.OnSynchronise->Add( OnOverallCopy );
    OnFileCopy = NULL;
    OnOverallCopy = NULL;

    try  {  ft.CopyTo(TBasicApp::GetBaseDir(), &AfterFileCopy);  }
    catch(PatchAPI::DeletionExc)  {  res = papi_DeleteError;  }
    catch(const TExceptionBase& exc)  {  
      res = papi_CopyError;  
      TBasicApp::GetLog().Error( exc.GetException()->GetFullMessage() );
    }
    
    if( res == papi_OK )  {
      try  {
        TEFile::DelFile(GetUpdateLocationFileName());
        TEFile::DeleteDir(patch_dir);
      }
      catch(...)  {  res = papi_DeleteError;  }
    }
    if( res == papi_OK )  {
      updater::SettingsFile sf( updater::UpdateAPI::GetSettingsFileName() );
      sf.last_updated = TETime::EpochTime();
      sf.Save();
    }
  }
  CleanUp(OnFileCopy, OnOverallCopy);
  UnlockUpdater();
  return res;
}
//.........................................................................
size_t PatchAPI::GetNumberOfOlex2Running()  {
  TStrList pid_files;
  TEFile::ListDir(TBasicApp::GetBaseDir(), pid_files, olxstr("*.") << GetOlex2PIDFileExt(), sefAll);
  for( size_t i=0; i < pid_files.Count(); i++ )  {
    if( TEFile::DelFile( TBasicApp::GetBaseDir() + pid_files[i]) )
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
//.........................................................................
olxstr PatchAPI::ReadRepositoryTag()  {
  olxstr tag_fn = TBasicApp::GetBaseDir() + GetTagFileName();
  if( !TEFile::Exists(tag_fn) )
    return EmptyString;
  TStrList sl;
  sl.LoadFromFile(tag_fn);
  return sl.Count() == 1 ? sl[0] : EmptyString;
}
//.............................................................................
