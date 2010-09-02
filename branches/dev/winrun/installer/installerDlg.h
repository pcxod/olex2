#pragma once

enum {
  actionInstall,
  actionRun,
  actionReinstall,
  actionExit
};
const short
  rename_status_BaseDir = 0x0001,
  rename_status_DataDir = 0x0002;

const unsigned int bgColor = 0xEFEFEF;

class CInstallerDlg : public CDialog  {
public:
	CInstallerDlg(CWnd* pParent = NULL);	// standard constructor
  ~CInstallerDlg();
	enum { IDD = IDD_INSTALLER_DIALOG };

  void SetAction(const olxch *val)  {
    GetDlgItem(IDC_ST_PROGRESS)->SendMessage(WM_SETTEXT, 0, (LPARAM)val);
  }
  void SetAction(const olxstr &val)  {
    GetDlgItem(IDC_ST_PROGRESS)->SendMessage(WM_SETTEXT, 0, (LPARAM)val.u_str());
  }
  CToolTipCtrl *tooltipCtrl;
  CBrush *ctrlBrush;
  void ProcessMessages();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
  bool mouse_down;
  CPoint mouse_pos;
  bool run_as_admin, olex2_installed;
  olxstr olex2_installed_path, olex2_data_dir, 
    olex2_install_path, olex2_install_tag;
  int action;
  short rename_status;
  // initialises (if olex2 is installed) olex2_install_path and returns it or CmdLine
  olxstr LocateBaseDir();
  olxstr ReadTag(const olxstr& zip_fn) const;
  void InitRepositories();
  void DisableInterface(bool v);
  void SetAction(int a);
  bool _DoInstall(const olxstr &zip_name, const olxstr &location);
  bool DoInstall();
  bool InitRegistry(const olxstr &installPath);
  bool DoUninstall();
  bool CleanRegistry();
  bool CleanRegistryAndShortcuts(bool sc);
  bool DoRun();
  bool LaunchFile(const olxstr &fileName, bool quiet, bool do_exit);
  void SetInstallationPath(const olxstr& path);
  TBasicApp bapp;
  const static olxstr exts[];
  const static size_t exts_sz;
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedBtnChoosePath();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnBnClickedInstall();
  afx_msg void OnBnClickedBtnClose();
  afx_msg void OnBnClickedCbProxy();
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  afx_msg void OnBnClickedBtnProxy();
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  afx_msg void OnBnClickedBtnRepository();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
  afx_msg void OnCbnSelendokCbRepository();
};
