#include "stdafx.h"
#include "reinstallDlg.h"
#include "estrlist.h"
#include "mfc_ext.h"
using namespace mfc_ext;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CReinstallDlg, CDialog)
  ON_BN_CLICKED(IDC_RG_RENAME, &CReinstallDlg::OnBnClickedRgRename)
  ON_BN_CLICKED(IDC_RG_UNINSTALL, &CReinstallDlg::OnBnClickedRgUninstall)
  ON_BN_CLICKED(IDOK, &CReinstallDlg::OnBnClickedOk)
END_MESSAGE_MAP()

CReinstallDlg::CReinstallDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReinstallDlg::IDD, pParent)
{
//  SetIcon
//  m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON);
}

BOOL CReinstallDlg::OnInitDialog()  {
	CDialog::OnInitDialog();
  check_box::set_checked(this, IDC_CB_REINSTALL, true);
  check_box::set_checked(this, IDC_RG_UNINSTALL, true);
  wnd::set_enabled(this, IDC_TE_RENAME, false);
  wnd::set_text(this, IDC_TE_RENAME, rename_to);
  return TRUE;
}


void CReinstallDlg::OnBnClickedRgRename()  {
  bool checked = check_box::is_checked(this, IDC_RG_RENAME);
  wnd::set_enabled(this, IDC_TE_RENAME, checked);
  wnd::set_enabled(this, IDC_CB_REMOVE_USER_DATA, !checked);
}

void CReinstallDlg::OnBnClickedRgUninstall()  {
  CReinstallDlg::OnBnClickedRgRename();
}

void CReinstallDlg::OnBnClickedOk()  {
  is_install = check_box::is_checked(this, IDC_CB_REINSTALL);
  is_remove = check_box::is_checked(this, IDC_RG_UNINSTALL);
  is_remove_data = check_box::is_checked(this, IDC_CB_REMOVE_USER_DATA);
  rename_to = wnd::get_text(this, IDC_TE_RENAME);
  OnOK();
}
