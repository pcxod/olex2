#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "url.h"

// default constructor, sets Port to 80
TUrl::TUrl() : Proxy(NULL)  {
  Port = 80;
}
//..............................................................................
TUrl::TUrl( const olxstr& url ) : Proxy(NULL)  {
  Port = 80;
  // prtocol index
  int pri = url.IndexOf("://");
  // check if the proxy is used and protocol is defined for the target, not the proxy
  int doti = url.FirstIndexOf('.');
  if( doti != -1 && doti < pri )  pri = -1;
  // port index
  int poi;
  if( pri < 0 )  poi = url.FirstIndexOf(':');
  else           poi = url.FirstIndexOf(':', pri+3);
  //path index
  int pai;
  if( pri < 0 )
    pai = url.FirstIndexOf('/');
  else
    if( poi == -1 )
      pai = url.FirstIndexOf('/', pri+3);
    else
      pai = url.FirstIndexOf('/', poi+1);

  if( pri >= 0 )  {
    SetProtocol( url.SubStringTo(pri) );
    if( poi >= 0 )
      SetHost( url.SubString(pri+3, poi-pri-3) );
    else
      if( pai >=0 )
        SetHost( url.SubString(pri+3, pai-pri-3) );
      else
        SetHost( url.SubStringFrom(pri+3) );
  }
  else  {
    if( poi >= 0 )
      SetHost( url.SubStringTo(poi) );
    else
      if( pai >=0 )
        SetHost( url.SubStringTo(pai) );
      else
        SetHost( url );
  }
  if( poi >= 0 )  {
    if( pai >= 0 )
      SetPort( url.SubString(poi+1, pai-poi-1).ToInt() );
    else
      SetPort( url.SubStringFrom(poi+1).ToInt() );
  }
  if( pai >= 0 )
    SetPath( url.SubStringFrom(pai+1) );
}
//..............................................................................
TUrl::TUrl( const TUrl& url ) : Proxy(NULL)  {
  *this = url;
}
//..............................................................................
const TUrl& TUrl::operator = (const TUrl& url)  {
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
  return url;
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


