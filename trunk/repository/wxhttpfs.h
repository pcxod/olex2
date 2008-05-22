//---------------------------------------------------------------------------
#ifndef wxHttpFSH
#define wxHttpFSH
#include "httpex.h"
#include "filesystem.h"
#include "url.h"
//---------------------------------------------------------------------------
class TwxHttpFileSystem: public AFileSystem, public IEObject  {
  TUrl Url;
  THttp Http;
public:
  TwxHttpFileSystem(const TUrl& url) : Url(url) {
    if( !Http.Connect( (url.HasProxy() ? url.GetProxy().GetHost() : url.GetHost()).u_str(), 
                       url.HasProxy() ? url.GetProxy().GetPort() : url.GetPort() ) )  {
      throw TFunctionFailedException(__OlxSourceInfo, "connection failed");
    }
    SetBase( olxstr('/') << url.GetPath() );
  }
  virtual ~TwxHttpFileSystem()  {  }

  virtual IInputStream* OpenFile(const olxstr& Source);
  virtual bool FileExists(const olxstr& DN)  {  return true;  }

  virtual bool DelFile(const olxstr& FN)     {  throw TNotImplementedException(__OlxSourceInfo);    }
  virtual bool DelDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool AdoptFile(const TFSItem& Source){  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual bool NewDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool ChangeDir(const olxstr& DN)   {  throw TNotImplementedException(__OlxSourceInfo);  }
};

#endif
