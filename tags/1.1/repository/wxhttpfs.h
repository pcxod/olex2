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
protected:
  virtual bool _DoDelFile(const olxstr& f) {  return false;  }
  virtual bool _DoDelDir(const olxstr& f)  {  return false;  }
  virtual bool _DoNewDir(const olxstr& f)  {  return false;  }
  virtual bool _DoAdoptFile(const TFSItem& Source) {  return false;  }
  virtual bool _DoesExist(const olxstr& df);
  virtual IInputStream* _DoOpenFile(const olxstr& src);
  virtual bool _DoAdoptStream(IInputStream& file, const olxstr& name) {  return false;  }
  bool ReadToStream(IOutputStream& s, const olxstr& fn);
public:
  TwxHttpFileSystem(const TUrl& url) : Url(url) {
    if( url.HasProxy() )  {
      Http.SetUser( url.GetProxy().GetUser().u_str() );
      Http.SetPassword( url.GetProxy().GetPassword().u_str() );
    }
    Access = afs_ReadOnlyAccess;
    if( !Http.Connect( (url.HasProxy() ? url.GetProxy().GetHost() : url.GetHost()).u_str(), 
                       url.HasProxy() ? url.GetProxy().GetPort() : url.GetPort() ) )  {
      throw TFunctionFailedException(__OlxSourceInfo, "connection failed");
    }
    SetBase( olxstr('/') << url.GetPath() );
  }
  virtual ~TwxHttpFileSystem(){}

  // saves stream to a temprray file and returs the object which must be deleted manually
  TEFile* SaveFile(const olxstr& fn);
  virtual wxInputStream* wxOpenFile(const olxstr& Source);
};

#endif
