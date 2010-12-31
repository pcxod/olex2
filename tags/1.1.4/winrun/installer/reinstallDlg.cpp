#include "stdafx.h"
#include "reinstallDlg.h"
#include "estrlist.h"
#include "mfc_ext.h"
using namespace mfc_ext;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CReinstallDlg, CDialog)
  ON_BN_CLICKED(IDOK, &CReinstallDlg::OnBnClickedOk)
END_MESSAGE_MAP()

CReinstallDlg::CReinstallDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReinstallDlg::IDD, pParent)
{
  is_install = is_remove_data = false;
}

BOOL CReinstallDlg::OnInitDialog()  {
	CDialog::OnInitDialog();
  return TRUE;
}

void CReinstallDlg::OnBnClickedOk()  {
  is_remove_data = check_box::is_checked(this, IDC_CB_REMOVE_USER_DATA);
  is_install = check_box::is_checked(this, IDC_CB_REINSTALL);
  OnOK();
}
