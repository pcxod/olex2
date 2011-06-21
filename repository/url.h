#ifndef __olx_urlH
#define __olx_urlH
#include "exception.h"

class TUrl : public IEObject  {
  unsigned int Port;
  olxstr Protocol, Host, Path, User, Password;
  TUrl* Proxy;
public:
  // default constructor, sets Port to 80
  TUrl();
  // parsers a valid url
  TUrl( const olxstr& url );
  // copy constructor
  TUrl( const TUrl& url );

  virtual ~TUrl()  {
    if( Proxy != NULL )
      delete Proxy;
  }

  TUrl& operator = (const TUrl& url);

  olxstr GetFullAddress() const;

  // for http://www.dur.ac.uk:8080/index.html returns www.dur.ac.uk
  inline const olxstr& GetHost()  const              {  return Host;  }
  void SetHost( const olxstr& host );
  olxstr GetFullHost()   const;
  inline bool HasProxy() const {  return Proxy != NULL;  }
  void SetProxy( const TUrl& proxy_url)  {
    if( Proxy != NULL )  delete Proxy;
    Proxy = new TUrl(proxy_url);
  }
  void SetProxy( const olxstr& proxy_str)  {
    if( Proxy != NULL )  delete Proxy;
    Proxy = new TUrl(proxy_str);
  }
  TUrl& GetProxy() const {  return *Proxy;  }

  // for http://www.dur.ac.uk:8080/index.html returns index.html
  inline const olxstr& GetPath()  const              {  return Path;  }
  void SetPath( const olxstr& path );

  // for http://www.dur.ac.uk:8080/index.html returns http
  inline const olxstr& GetProtocol()  const          {  return Protocol;  }
  void SetProtocol( const olxstr& protocol );

  // for http://www.dur.ac.uk:8080/index.html returns 8080
  inline unsigned int GetPort()  const      {  return Port;  }
  inline void SetPort( unsigned int port )  {  Port = port;  }

  class TInvalidUrlException: public TBasicException  {
  public:
    TInvalidUrlException(const olxstr& location) :
      TBasicException(location, EmptyString())  { }
  };

  DefPropC(olxstr, User)
  DefPropC(olxstr, Password)

  static olxcstr GenerateHTTPAuthString(const olxstr& user, const olxstr& pass);
  olxcstr GenerateHTTPAuthString() const {
    return GenerateHTTPAuthString(User, Password);
  }
};
#endif
