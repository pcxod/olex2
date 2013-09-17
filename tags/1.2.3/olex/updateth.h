/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_update_thread_H
#define __olx_update_thread_H
#include "olxth.h"
#include "settingsfile.h"
#include "filesystem.h"
#include "utf8file.h"

class UpdateThread : public AOlxThread  {
  size_t time_out;
  uint64_t UpdateSize;
  AFileSystem *srcFS, *destFS;
  TFSIndex* Index;
  TStrList properties, filesToSkip, extensionsToSkip;
  TFSItem::SkipOptions toSkip;
  olxstr PatchDir;
  volatile bool _DoUpdate;
  bool ForceUpdate;
  void CleanUp()  {
    if( Index != NULL )  {
      delete Index;
      Index = NULL;
    }
    if( srcFS != NULL )  {
      delete srcFS;
      srcFS = NULL;
    }
    if( destFS != NULL )  {
      delete destFS;
      destFS = NULL;
    }
  }
  void DoInit(bool force);
  virtual void OnSendTerminate();
  TActionQList Actions;
  void MarkCompleted(const TStrList &cmds);
public:
  UpdateThread(const olxstr& patch_dir, bool force_update);

  virtual ~UpdateThread()  {  CleanUp();  }
  void DoUpdate() {  _DoUpdate = true;  }  
  uint64_t GetUpdateSize() const {  return UpdateSize;  }
  void ResetUpdateSize() {  UpdateSize = 0;  }
  virtual int Run();

  TActionQueue &OnDownload, &OnAction;
};

#endif
