/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_urlH
#define __olx_urlH
#include "exception.h"

class TUrl : public IEObject  {
  unsigned int Port;
  olxstr Protocol, Host, Path, User, Password;
  TUrl* Proxy;
public:
  // default constructor, sets Port to 80
  TUrl() : Port(80), Proxy(NULL) {}
  // parsers a valid url
  TUrl(const olxstr& url);
  // copy constructor
  TUrl(const TUrl& url) : Proxy(NULL)  {  *this = url; }

  virtual ~TUrl()  {
    if( Proxy != NULL )
      delete Proxy;
  }

  TUrl& operator = (const TUrl& url);

  olxstr GetFullAddress() const;

  // for http://www.dur.ac.uk:8080/index.html returns www.dur.ac.uk
  const olxstr& GetHost() const {  return Host;  }
  void SetHost( const olxstr& host );
  olxstr GetFullHost() const;
  bool HasProxy() const {  return Proxy != NULL;  }
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
  const olxstr& GetPath() const {  return Path;  }
  void SetPath( const olxstr& path );

  // for http://www.dur.ac.uk:8080/index.html returns http
  const olxstr& GetProtocol() const {  return Protocol;  }
  void SetProtocol( const olxstr& protocol );

  // for http://www.dur.ac.uk:8080/index.html returns 8080
  unsigned int GetPort() const {  return Port;  }
  void SetPort( unsigned int port )  {  Port = port;  }

  class TInvalidUrlException: public TBasicException  {
  public:
    TInvalidUrlException(const olxstr& location) :
      TBasicException(location, EmptyString())  { }
  };

  DefPropC(olxstr, User)
  DefPropC(olxstr, Password)

  static olxcstr GenerateHTTPAuthString(const olxstr& user,
  const olxstr& pass);
  olxcstr GenerateHTTPAuthString() const {
    return GenerateHTTPAuthString(User, Password);
  }
};
#endif
