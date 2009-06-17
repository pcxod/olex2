//---------------------------------------------------------------------------
#ifndef wxHttpFSH
#define wxHttpFSH
#include "httpex.h"
#include "filesystem.h"
#include "url.h"
#include "wxzipfs.h"
//---------------------------------------------------------------------------
class TwxHttpFileSystem: public AFileSystem, public IEObject  {
  TUrl Url;
  THttp Http;
  TwxZipFileSystem* ZipFS;
public:
  TwxHttpFileSystem(const TUrl& url, TwxZipFileSystem* zipFS=NULL) : Url(url) {
    ZipFS = zipFS;
    if( url.HasProxy() )  {
      Http.SetUser( url.GetProxy().GetUser().u_str() );
      Http.SetPassword( url.GetProxy().GetPassword().u_str() );
    }
    if( !Http.Connect( (url.HasProxy() ? url.GetProxy().GetHost() : url.GetHost()).u_str(), 
                       url.HasProxy() ? url.GetProxy().GetPort() : url.GetPort() ) )  {
      throw TFunctionFailedException(__OlxSourceInfo, "connection failed");
    }
    SetBase( olxstr('/') << url.GetPath() );
  }
  virtual ~TwxHttpFileSystem();

  // saves stream to a temprray file and returs the object which must be deleted manually
  TEFile* SaveFile(const olxstr& fn);
  // zip is as primary source of the files, if a file is not in the zip - Url is used
  void SetZipFS( TwxZipFileSystem* zipFS )  {
    if( ZipFS != NULL )  delete ZipFS;
      ZipFS = zipFS;
  }
  virtual IInputStream* OpenFile(const olxstr& Source);
  virtual wxInputStream* wxOpenFile(const olxstr& Source);
  virtual bool FileExists(const olxstr& DN)  {  return true;  }

  virtual bool DelFile(const olxstr& FN)     {  throw TNotImplementedException(__OlxSourceInfo);    }
  virtual bool DelDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool AdoptFile(const TFSItem& Source){  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual bool AdoptStream(IInputStream& in, const olxstr& as)  {  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual bool NewDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool ChangeDir(const olxstr& DN)   {  throw TNotImplementedException(__OlxSourceInfo);  }
};

#endif
