#pragma once
#include "stdafx.h"
#include "resource.h"
#include "ebase.h"

class CReinstallDlg : public CDialog  {
public:
	CReinstallDlg(CWnd* pParent);	// standard constructor
	enum { IDD = IDD_REINSTALL };
  void SetRenameToValue(const olxstr &v)  {  rename_to = v;  }
  bool IsInstall() {  return is_install;  }
  bool IsRemove() {  return is_remove;  }
  bool IsRename() {  return is_rename;  }
  bool IsRemoveUserData()  {  return is_remove_data; }
  void DisableDoNothing()  {  do_nothing_enabled = false;  }
  olxstr GetRenameToText() {  return rename_to;  }
protected:
  olxstr rename_to;
  bool is_install, is_remove, is_remove_data, is_rename;
  bool do_nothing_enabled;
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedRgRename();
  afx_msg void OnBnClickedRgUninstall();
  afx_msg void OnBnClickedOk();
};
