#pragma once
#include "stdafx.h"
#include "resource.h"
#include "ebase.h"

class CReinstallDlg : public CDialog  {
public:
	CReinstallDlg(CWnd* pParent);	// standard constructor
	enum { IDD = IDD_REINSTALL };
  bool IsInstall() {  return is_install;  }
  bool IsRemoveUserData()  {  return is_remove_data; }
protected:
  bool is_install, is_remove_data;
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedOk();
};
