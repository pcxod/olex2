#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "url.h"

// default constructor, sets Port to 80
TUrl::TUrl() : Proxy(NULL)  {
  Port = 80;
}
//..............................................................................
TUrl::TUrl( const olxstr& _url ) : Proxy(NULL)  {
  Port = 80;
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
  // check if the proxy is used and protocol is defined for the target, not the proxy
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
    SetProtocol( url.SubStringTo(pri) );
    if( poi != InvalidIndex )
      SetHost( url.SubString(pri+3, poi-pri-3) );
    else
      if( pai != InvalidIndex )
        SetHost( url.SubString(pri+3, pai-pri-3) );
      else
        SetHost( url.SubStringFrom(pri+3) );
  }
  else  {
    if( poi != InvalidIndex )
      SetHost( url.SubStringTo(poi) );
    else
      if( pai != InvalidIndex )
        SetHost( url.SubStringTo(pai) );
      else
        SetHost( url );
  }
  if( poi != InvalidIndex )  {
    if( pai != InvalidIndex )
      SetPort( url.SubString(poi+1, pai-poi-1).ToInt() );
    else
      SetPort( url.SubStringFrom(poi+1).ToInt() );
  }
  if( pai != InvalidIndex )
    SetPath( url.SubStringFrom(pai+1) );
}
//..............................................................................
TUrl::TUrl( const TUrl& url ) : Proxy(NULL)  {
  *this = url;
}
//..............................................................................
TUrl& TUrl::operator = (const TUrl& url) {
  this->Port = url.GetPort();
  this->Host = url.GetHost();
  this->Protocol = url.GetProtocol();
  this->Path = url.GetPath();
  if( url.HasProxy() )
    this->Proxy = new TUrl(*url.Proxy);
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
olxstr TUrl::GetFullHost()  const  {
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
olxstr TUrl::GenerateHTTPAuthString(const olxstr& user, const olxstr& pass) {
  static const char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  olxstr buf("Basic ");
  olxstr toencode(user);
  toencode << ':' << pass;

  size_t len = toencode.Length();
  size_t from = 0;
  while (len >= 3) { // encode full blocks first
    buf << (char)base64[(toencode[from] >> 2) & 0x3f] << 
      (char)base64[((toencode[from] << 4) & 0x30) | ((toencode[from+1] >> 4) & 0xf)];
    buf << (char)base64[((toencode[from+1] << 2) & 0x3c) | ((toencode[from+2] >> 6) & 0x3)] <<
      (char) base64[toencode[from+2] & 0x3f];
    from += 3;
    len -= 3;
  }
  if (len > 0) { // pad the remaining characters
    buf << (char)base64[(toencode[from] >> 2) & 0x3f];
    if (len == 1) {
      buf << (char)base64[(toencode[from] << 4) & 0x30] << '=';
    } 
    else {
      buf << (char)base64[(toencode[from] << 4) & 0x30] + ((toencode[from+1] >> 4) & 0xf) <<
        (char)base64[(toencode[from+1] << 2) & 0x3c];
    }
    buf << '=';
  }
  return buf;
}
//............................................................................//


