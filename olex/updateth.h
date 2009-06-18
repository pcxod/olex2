#ifndef __olx_update_thread_H
#define __olx_update_thread_H
#include "olxth.h"
#include "settingsfile.h"
#include "wxhttpfs.h"
#include "wxzipfs.h"
#include "utf8file.h"

class UpdateThread : public AOlxThread  {
  size_t time_out;
  bool Valid, Update;
  uint64_t UpdateSize;
  AFileSystem* srcFS, *destFS;
  TFSIndex* Index;
  TStrList properties, filesToSkip, extensionsToSkip;
  TFSItem::SkipOptions toSkip;
  olxstr PatchDir;
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
  virtual void OnSendTerminate();
public:
  UpdateThread(const olxstr& patch_dir);

  virtual ~UpdateThread()  {  CleanUp();  }
  void DoUpdate() {  Update = true;  }  
  uint64_t GetUpdateSize() const {  return UpdateSize;  }
  void ResetUpdateSize() {  UpdateSize = 0;  }
  virtual int Run();
};

#endif
