#ifndef _T_HTTP_H
#define _T_HTTP_H

#include "estlist.h"
#include "wx/defs.h"

#include "wx/protocol/protocol.h"

class THttp : public wxProtocol  {
public:
  THttp();
  virtual ~THttp();

  virtual bool Connect(const wxString& host, unsigned short port);
  virtual bool Connect(const wxString& host) { return Connect(host, 0); }
  virtual bool Connect(wxSockAddress& addr, bool wait);
  bool Abort();
  wxInputStream *GetInputStream(const wxString& path);
  inline wxProtocolError GetError() { return m_perr; }
  wxString GetContentType();

  void SetHeader(const olxstr& header, const olxstr& h_data);
  const olxstr& GetHeader(const olxstr& header) const;
  void SetPostBuffer(const olxstr& post_buf);

  void SetProxyMode(bool on);

  int GetResponse() { return m_http_response; }

  virtual void SetUser(const wxString& user)        { m_username = user; }
  virtual void SetPassword(const wxString& passwd ) { m_password = passwd; }

protected:
  enum wxHTTP_Req
  {
    wxHTTP_GET,
    wxHTTP_POST,
    wxHTTP_HEAD
  };

  bool BuildRequest(const wxString& path, wxHTTP_Req req);
  void SendHeaders();
  bool ParseHeaders();

  // deletes the header value strings
  void ClearHeaders();
  TCSTypeList<olxstr, olxstr> Headers;
  wxProtocolError m_perr;
  bool m_read,
       m_proxy_mode;
  wxSockAddress *m_addr;
  olxstr m_post_buf;
  int m_http_response;
  wxString m_username;
  wxString m_password;
};

#endif // _WX_HTTP_H

