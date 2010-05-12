#include "stdafx.h"
#include "licenceDlg.h"
#include "efile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CLicenceDlg, CDialog)
END_MESSAGE_MAP()

CLicenceDlg::CLicenceDlg(CWnd* pParent, const olxstr &_file_name)
	: CDialog(CLicenceDlg::IDD, pParent), file_name(_file_name) { }


DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE lpBuff,
                                  LONG cb, PLONG pcb)
{
  HANDLE hFile = (HANDLE)dwCookie;
  if (ReadFile(hFile, lpBuff, cb, (DWORD *)pcb, NULL)) 
    return 0;
  return -1;
}

BOOL CLicenceDlg::OnInitDialog()  {
	CDialog::OnInitDialog();
  BOOL fSuccess = FALSE;
  HANDLE hFile = CreateFile(file_name.u_str(), GENERIC_READ, 
    FILE_SHARE_READ, 0, OPEN_EXISTING,
    FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if (hFile != INVALID_HANDLE_VALUE) 
  {
    EDITSTREAM es = { 0 };
    es.pfnCallback = EditStreamCallback;
    es.dwCookie = (DWORD_PTR)hFile;
    if (GetDlgItem(IDC_RE_LICENCE)->SendMessage(EM_STREAMIN, SF_RTF, (LPARAM)&es) 
      && es.dwError == 0) 
    {
      fSuccess = TRUE;
    }
    CloseHandle(hFile);
  }
  GetDlgItem(IDC_RE_LICENCE)->SendMessage(EM_SETWORDWRAPMODE, WBF_WORDBREAK);
  return TRUE;
}
