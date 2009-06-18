#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "patchapi.h"
#include "bapp.h"
#include "efile.h"
#include "utf8file.h"
#ifdef __WIN32__
  #include <windows.h>
#endif
#include "filetree.h"

using namespace patch;

short PatchAPI::DoPatch(AActionHandler* OnFileCopy, AActionHandler* OnOverallCopy)  {
  TBasicApp& bapp = *TBasicApp::GetInstance();

  olxstr patch_dir(bapp.BaseDir() + "patch/");
  olxstr download_vf(bapp.BaseDir() + "__completed.update");
  olxstr cmd_file(bapp.BaseDir() + "__cmds.update");
  // make sure that only one instance is running
  olxstr pid_file(bapp.BaseDir() + "updater.pid_");
  if( TEFile::Exists(pid_file) )  {
    if( !TEFile::DelFile(pid_file) )  {
      CleanUp(OnFileCopy, OnOverallCopy);
      return papi_Busy;
    }
  }
  if( !TEFile::Exists(patch_dir) || !TEFile::Exists(download_vf) )  {
    CleanUp(OnFileCopy, OnOverallCopy);
    return papi_OK;
  }
  // create lock if possible or exit
  TEFile* pid_file_inst = NULL;
  try  {
    pid_file_inst = new TEFile(pid_file, "w+b");
  }
  catch( TExceptionBase& )  {
    CleanUp(OnFileCopy, OnOverallCopy);
    return papi_Busy;
  }
  // check that there are no olex2 instances...
  TStrList pid_files;
  TEFile::ListDir(bapp.BaseDir(), pid_files, "*.olex2_pid", sefAll);
  for( int i=0; i < pid_files.Count(); i++ )  {
    if( TEFile::DelFile( bapp.BaseDir() + pid_files[i]) )
      pid_files[i].SetLength(0);
  }
  pid_files.Pack();
  if( !pid_files.IsEmpty() )  {
    CleanUp(OnFileCopy, OnOverallCopy);
    return papi_Busy;
  }
  // clean
  short res = papi_OK;
  if( TEFile::Exists(cmd_file) )  {
    TWStrList _cmds;
    TUtf8File::ReadLines(cmd_file, _cmds);
    TStrList cmds(_cmds);
    for( int i=0; i < cmds.Count(); i++ )  {
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
    ft.Root.Expand();
    TOnProgress fc_pg, oa_pg;
    if( OnFileCopy != NULL )
      ft.OnFileCopy->Add( OnFileCopy );
    if( OnOverallCopy != NULL )
      ft.OnSynchronise->Add( OnOverallCopy );
    OnFileCopy = NULL;
    OnOverallCopy = NULL;

    try  {
      ft.CopyTo(bapp.BaseDir(), &AfterFileCopy,
        OnFileCopy == NULL ? NULL : &fc_pg,
        OnOverallCopy == NULL ? NULL : &oa_pg
      );
    }
    catch(PatchAPI::DeletionExc)  {  res = papi_DeleteError;  }
    catch(...)  {  res = papi_CopyError;  }
    
    if( res == papi_OK )  {
      try  {
        TEFile::DeleteDir(patch_dir);
        TEFile::DelFile(download_vf);
      }
      catch(...)  {  res = papi_DeleteError;  }
    }
  }
  pid_file_inst->Delete();
  delete pid_file_inst;
  CleanUp(OnFileCopy, OnOverallCopy);
  return res;
}


