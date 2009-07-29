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
  class DListenerProxy : public AActionHandler  {
    TActionQueue& dest;
  public:
    DListenerProxy(TActionQueue& _dest) : dest(_dest) {}
    virtual bool Enter(const IEObject* Sender, const IEObject* data)  {
      return dest.Enter(Sender, data);
    }
    virtual bool Execute(const IEObject* Sender, const IEObject* data)  {
      return dest.Execute(Sender, data);
      //TBasicApp::GetLog().Info( olxstr("Downloading: ") << ((TOnProgress*)data)->GetAction() );
    }
    virtual bool Exit(const IEObject* Sender, const IEObject* data)  {
      return dest.Exit(Sender, data);
    }
  };
  TActionQList Actions;
public:
  UpdateThread(const olxstr& patch_dir);

  virtual ~UpdateThread()  {  CleanUp();  }
  void DoUpdate() {  _DoUpdate = true;  }  
  uint64_t GetUpdateSize() const {  return UpdateSize;  }
  void ResetUpdateSize() {  UpdateSize = 0;  }
  virtual int Run();

  TActionQueue* OnDownload;
};

#endif
