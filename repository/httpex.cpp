// Hashmap caused too many problems to me - adopted by OVD for X-net


#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdio.h>
#include <stdlib.h>

#include "httpex.h"

#include "wx/string.h"
#include "wx/app.h"

#include "wx/tokenzr.h"
#include "wx/socket.h"
#include "wx/protocol/protocol.h"
#include "wx/url.h"
#include "wx/sckstrm.h"

THttp::THttp() : wxProtocol()  {
  m_addr = NULL;
  m_read = false;
  m_proxy_mode = false;
  m_post_buf = wxEmptyString;
  m_http_response = 0;

  SetNotify(wxSOCKET_LOST_FLAG);
}
//............................................................................//
THttp::~THttp()  {
  ClearHeaders();
  if( m_addr != NULL )  delete m_addr;
}
//............................................................................//
void THttp::ClearHeaders()       {  Headers.Clear();  }
//............................................................................//
wxString THttp::GetContentType() {  return uiStr(GetHeader("Content-Type"));  }
//............................................................................//
void THttp::SetProxyMode(bool on){  m_proxy_mode = on;  }
//............................................................................//
void THttp::SetHeader(const olxstr& header, const olxstr& h_data)  {
  if( m_read ) {
    ClearHeaders();
    m_read = false;
  }
  int ind = Headers.IndexOfComparable( header );
  if( ind == -1 )
    Headers.Add(header, h_data);
  else
    Headers.Object(ind) = h_data;
}
//............................................................................//
const olxstr& THttp::GetHeader(const olxstr& header) const  {
  int ind = Headers.IndexOfComparable( header );
  return ind == -1 ? EmptyString : Headers.GetObject(ind);
}
//............................................................................//
wxString THttp::GenerateAuthString(const wxString& user, const wxString& pass) const  {
  static const char *base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  wxString buf;
  wxString toencode;

  buf.Printf(wxT("Basic "));

  toencode.Printf(wxT("%s:%s"),user.c_str(),pass.c_str());

  size_t len = toencode.length();
  const wxChar *from = toencode.c_str();
  while (len >= 3) { // encode full blocks first
    buf << wxString::Format(wxT("%c%c"), base64[(from[0] >> 2) & 0x3f], base64[((from[0] << 4) & 0x30) | ((from[1] >> 4) & 0xf)]);
    buf << wxString::Format(wxT("%c%c"), base64[((from[1] << 2) & 0x3c) | ((from[2] >> 6) & 0x3)], base64[from[2] & 0x3f]);
    from += 3;
    len -= 3;
  }
  if (len > 0) { // pad the remaining characters
    buf << wxString::Format(wxT("%c"), base64[(from[0] >> 2) & 0x3f]);
    if (len == 1) {
      buf << wxString::Format(wxT("%c="), base64[(from[0] << 4) & 0x30]);
    } else {
      buf << wxString::Format(wxT("%c%c"), base64[(from[0] << 4) & 0x30] + ((from[1] >> 4) & 0xf), base64[(from[1] << 2) & 0x3c]);
    }
    buf << wxString::Format(wxT("="));
  }

  return buf;
}
//............................................................................//
void THttp::SetPostBuffer(const olxstr& post_buf)  {  m_post_buf = post_buf;  }
//............................................................................//
void THttp::SendHeaders()  {
  CString buf;
  for( int i=0; i < Headers.Count(); i++ )  {
    buf = Headers.GetComparable(i);  buf  << ": "  << Headers.GetObject(i) << "\r\n";
//    Write( buf.raw_str(), buf.RawLen() );
  }
}
//............................................................................//
bool THttp::ParseHeaders()  {
  wxString line;

  ClearHeaders();
  m_read = true;

  while( true )  {
    m_perr = ReadLine(this, line);
    if( m_perr != wxPROTO_NOERR )  return false;
    if( line.length() == 0 )       break;

    SetHeader( line.BeforeFirst(wxT(':')).c_str(), line.AfterFirst(wxT(':')).Strip(wxString::both).c_str() );
  }
  return true;
}
//............................................................................//
bool THttp::Connect(const wxString& host, unsigned short port) {
  wxIPV4address *addr;

  if( m_addr ) {
    delete m_addr;
    m_addr = NULL;
    Close();
  }

  m_addr = addr = new wxIPV4address();

  if( !addr->Hostname(host) ) {
    delete m_addr;
    m_addr = NULL;
    m_perr = wxPROTO_NETERR;
    return false;
  }

  if ( port != 0 )
    addr->Service(port);
  else if( !addr->Service( wxT("http") ) )
    addr->Service(80);

  SetHeader(wxT("Host"), host.c_str() );
  return true;
}
//............................................................................//
bool THttp::Connect(wxSockAddress& addr, bool WXUNUSED(wait))  {
  if( m_addr ) {
    delete m_addr;
    Close();
  }

  m_addr = addr.Clone();

  wxIPV4address *ipv4addr = wxDynamicCast(&addr, wxIPV4address);
  if( ipv4addr )
    SetHeader(wxT("Host"), ipv4addr->OrigHostname().c_str() );

  return true;
}
//............................................................................//
bool THttp::BuildRequest(const wxString& path, wxHTTP_Req req)  {
  const wxChar *request;

  switch( req )  {
      case wxHTTP_GET:
        request = wxT("GET");
        break;
      case wxHTTP_POST:
        request = wxT("POST");
        if ( GetHeader( wxT("Content-Length") ).Length() == 0 )
          SetHeader( wxT("Content-Length"), m_post_buf.Length() );
        break;
      default:
        return false;
  }

  m_http_response = 0;

  // If there is no User-Agent defined, define it.
  if (GetHeader(wxT("User-Agent")).Length() == 0 )
    SetHeader(wxT("User-Agent"), olxstr("wxWidgets ") << wxMAJOR_VERSION << '.' << wxMINOR_VERSION);

  // Send authentication information
  if (!m_username.empty() || !m_password.empty()) {
    SetHeader("Authorization", GenerateAuthString(m_username, m_password).c_str() );
  }

  SaveState();

  // we may use non blocking sockets only if we can dispatch events from them
  SetFlags( wxIsMainThread() && wxApp::IsMainLoopRunning() ? wxSOCKET_NONE
    : wxSOCKET_BLOCK );
  Notify(false);

  wxString buf;
  buf.Printf(wxT("%s %s HTTP/1.0\r\n"), request, path.c_str());
  const wxWX2MBbuf pathbuf = wxConvLocal.cWX2MB(buf);
  Write(pathbuf, strlen(wxMBSTRINGCAST pathbuf));
  SendHeaders();
  Write("\r\n", 2);

  if ( req == wxHTTP_POST ) {
    Write(m_post_buf.c_str(), m_post_buf.Length());
    m_post_buf = wxEmptyString;
  }

  wxString tmp_str;
  m_perr = ReadLine(this, tmp_str);
  if (m_perr != wxPROTO_NOERR) {
    RestoreState();
    return false;
  }

  if (!tmp_str.Contains(wxT("HTTP/"))) {
    // TODO: support HTTP v0.9 which can have no header.
    // FIXME: tmp_str is not put back in the in-queue of the socket.
    SetHeader(wxT("Content-Length"), wxT("-1"));
    SetHeader(wxT("Content-Type"), wxT("none/none"));
    RestoreState();
    return true;
  }

  wxStringTokenizer token(tmp_str,wxT(' '));
  wxString tmp_str2;
  bool ret_value;

  token.NextToken();
  tmp_str2 = token.NextToken();

  m_http_response = wxAtoi(tmp_str2);

  switch( tmp_str2[0] )  {
  case wxT('1'):
    /* INFORMATION / SUCCESS */
    break;
  case wxT('2'):
    /* SUCCESS */
    break;
  case wxT('3'):
    /* REDIRECTION */
    break;
  default:
    m_perr = wxPROTO_NOFILE;
    RestoreState();
    return false;
  }

  ret_value = ParseHeaders();
  RestoreState();
  return ret_value;
}

class THTTPStream : public wxSocketInputStream  {
public:
  THttp *m_http;
  size_t m_httpsize;
  unsigned long m_read_bytes;

  THTTPStream(THttp *http) : wxSocketInputStream(*http), m_http(http) {}
  size_t GetSize() const { return m_httpsize; }
  virtual ~THTTPStream(void) { m_http->Abort(); }

protected:
  size_t OnSysRead(void *buffer, size_t bufsize);

  DECLARE_NO_COPY_CLASS(THTTPStream)
};

size_t THTTPStream::OnSysRead(void *buffer, size_t bufsize)  {
  if (m_httpsize > 0 && m_read_bytes >= m_httpsize)  {
    m_lasterror = wxSTREAM_EOF;
    return 0;
  }

  size_t ret = wxSocketInputStream::OnSysRead(buffer, bufsize);
  m_read_bytes += ret;

  if (m_httpsize==(size_t)-1 && m_lasterror == wxSTREAM_READ_ERROR )  {
    // if m_httpsize is (size_t) -1 this means read until connection closed
    // which is equivalent to getting a READ_ERROR, for clients however this
    // must be translated into EOF, as it is the expected way of signalling
    // end end of the content
    m_lasterror = wxSTREAM_EOF ;
  }

  return ret;
}

bool THttp::Abort(void)  {
  return wxSocketClient::Close();
}

wxInputStream *THttp::GetInputStream(const wxString& path)  {
  THTTPStream *inp_stream;

  wxString new_path;

  m_perr = wxPROTO_CONNERR;
  if (!m_addr)
    return NULL;

  // We set m_connected back to false so wxSocketBase will know what to do.
#ifdef __WXMAC__
  wxSocketClient::Connect(*m_addr , false );
  wxSocketClient::WaitOnConnect(10);

  if (!wxSocketClient::IsConnected())
    return NULL;
#else
  if (!wxProtocol::Connect(*m_addr))
    return NULL;
#endif

  if (!BuildRequest(path, m_post_buf.Length() == 0 ? wxHTTP_GET : wxHTTP_POST))
    return NULL;

  inp_stream = new THTTPStream(this);

  if( !GetHeader(wxT("Content-Length")).Length() == 0 )
    inp_stream->m_httpsize = GetHeader(wxT("Content-Length")).ToInt();
  else
    inp_stream->m_httpsize = (size_t)-1;

  inp_stream->m_read_bytes = 0;

  Notify(false);
  SetFlags(wxSOCKET_BLOCK | wxSOCKET_WAITALL);

  return inp_stream;
}

