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
  TStrList properties, filesToSkip, extensionsToSkip;
  TFSItem::SkipOptions toSkip;
  olxstr PatchDir;
public:
  UpdateThread(const olxstr& patch_dir);

  virtual ~UpdateThread()  {
    if( srcFS != NULL )
      delete srcFS;
    if( destFS != NULL )
      delete destFS;
  }
  void DoUpdate() {  Update = true;  }  
  uint64_t GetUpdateSize() const {  return UpdateSize;  }

  virtual int Run();
};

#endif
