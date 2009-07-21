#ifndef __olx_update_thread_H
#define __olx_update_thread_H
#include "olxth.h"
#include "settingsfile.h"
#include "wxhttpfs.h"
#include "wxzipfs.h"
#include "utf8file.h"

class UpdateThread : public AOlxThread  {
  size_t time_out;
  uint64_t UpdateSize;
  AFileSystem* srcFS, *destFS;
  TFSIndex* Index;
  TStrList properties, filesToSkip, extensionsToSkip;
  TFSItem::SkipOptions toSkip;
  olxstr PatchDir;
  volatile bool _DoUpdate;
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
  void DoInit();
  virtual void OnSendTerminate();
  class DListener : public AActionHandler  {
  public:
    virtual bool Enter(const IEObject* Sender, const IEObject* data)  {
      return true;
    }
    virtual bool Execute(const IEObject* Sender, const IEObject* data)  {
      if( data == NULL || !EsdlInstanceOf(*data, TOnProgress) )  return false;
      TBasicApp::GetLog().Info( olxstr("Downloading: ") << ((TOnProgress*)data)->GetAction() );
      return true;
    }
    virtual bool Exit(const IEObject* Sender, const IEObject* data)  {
      return true;
    }
  };
public:
  UpdateThread(const olxstr& patch_dir);

  virtual ~UpdateThread()  {  CleanUp();  }
  void DoUpdate() {  _DoUpdate = true;  }  
  uint64_t GetUpdateSize() const {  return UpdateSize;  }
  void ResetUpdateSize() {  UpdateSize = 0;  }
  virtual int Run();
};

#endif
