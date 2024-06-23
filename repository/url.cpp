/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "url.h"
#include "encodings.h"
#include "eutf8.h"

//..............................................................................
TUrl::TUrl( const olxstr& _url ) : Proxy(NULL)  {
  Port = _url.StartsFromi("https://") ? 443 : 80;
  olxstr url(_url);
  // extract proxy user and password, if any
  size_t useri = url.IndexOf("@");
  if( useri != InvalidIndex )  {
    olxstr up = url.SubStringTo(useri);
    size_t pwdi = up.IndexOf(':');
    if( pwdi != InvalidIndex )  {
      Password = up.SubStringFrom(pwdi+1);
      User = up.SubStringTo(pwdi);
    }
    else
      User = up;
    url = _url.SubStringFrom(useri+1);
  }
  // prtocol index
  size_t pri = url.IndexOf("://");
  /* check if the proxy is used and protocol is defined for the target, not
  the proxy
  */
  size_t doti = url.FirstIndexOf('.');
  if( doti != InvalidIndex && doti < pri )  pri = InvalidIndex;
  // port index
  size_t poi;
  if( pri == InvalidIndex )
    poi = url.FirstIndexOf(':');
  else
    poi = url.FirstIndexOf(':', pri+3);
  //path index
  size_t pai;
  if( pri == InvalidIndex )
    pai = url.FirstIndexOf('/');
  else
    if( poi == InvalidIndex )
      pai = url.FirstIndexOf('/', pri+3);
    else
      pai = url.FirstIndexOf('/', poi+1);

  if( pri != InvalidIndex )  {
    SetProtocol(url.SubStringTo(pri));
    if( poi != InvalidIndex )
      SetHost(url.SubString(pri+3, poi-pri-3));
    else
      if( pai != InvalidIndex )
        SetHost(url.SubString(pri+3, pai-pri-3));
      else
        SetHost(url.SubStringFrom(pri+3));
  }
  else  {
    if( poi != InvalidIndex )
      SetHost(url.SubStringTo(poi));
    else
      if( pai != InvalidIndex )
        SetHost(url.SubStringTo(pai));
      else
        SetHost(url);
  }
  if( poi != InvalidIndex )  {
    if( pai != InvalidIndex )
      SetPort(url.SubString(poi+1, pai-poi-1).ToInt());
    else
      SetPort(url.SubStringFrom(poi+1).ToInt());
  }
  if( pai != InvalidIndex )
    SetPath(url.SubStringFrom(pai+1));
}
//..............................................................................
TUrl& TUrl::operator = (const TUrl& url) {
  this->Port = url.GetPort();
  this->Host = url.GetHost();
  this->Protocol = url.GetProtocol();
  this->Path = url.GetPath();
  if( url.HasProxy() ) {
    if( this->Proxy )
      delete Proxy;
    this->Proxy = new TUrl(*url.Proxy);
  }
  else  {
    if( this->Proxy )  {
      delete this->Proxy;
      this->Proxy = NULL;
    }
  }
  this->Password = url.Password;
  this->User = url.User;
  return *this;
}
//..............................................................................
olxstr TUrl::GetFullHost() const {
  olxstr retVal;

  if( !Protocol.IsEmpty() )
    retVal << Protocol << "://";
  retVal << Host;
  if( Port != 80 )
    retVal << ':' << (int)Port;
  return retVal;
}
//..............................................................................
olxstr TUrl::GetFullAddress() const  {
  olxstr retVal;

  if( Protocol.Length() )
    retVal << Protocol << "://";
  retVal << Host;
  if( Port != 80 )
    retVal << ':' << (int)Port;
  if( Path.Length() )
    retVal << '/' << Path;
  return retVal;
}
//..............................................................................
void TUrl::SetHost( const olxstr& host )  {
  Host = host;
}
//..............................................................................
void TUrl::SetPath( const olxstr& path )  {
  Path = path;
}
//..............................................................................
void TUrl::SetProtocol( const olxstr& protocol )  {
  Protocol = protocol;
}
//..............................................................................
olxcstr TUrl::GenerateHTTPAuthString(const olxstr& user, const olxstr& pass)  {
  olxcstr buf("Basic ");
  olxcstr toencode(user);
  toencode << ':' << pass;
  return (buf << encoding::base64::encode(TUtf8::Encode(toencode)));
}
//............................................................................//
