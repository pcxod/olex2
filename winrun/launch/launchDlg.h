#pragma once

class MainDlg : public CDialog  {
public:
	MainDlg(CWnd* pParent = NULL);	// standard constructor

	enum { IDD = IDD_LAUNCH_DIALOG };

  void SetVersion(const olxstr &version);
  void SetFileName(const olxstr &fn);
  void SetFileProgressMax(double v);
  void SetFileProgress(double v);
  void SetOverallProgressMax(double v);
  void SetOverallProgress(double v);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
  bool mouse_down;
  CPoint mouse_pos;
  double max_file, max_overall;
  HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
  void OnCancel()  {  DestroyWindow();  }
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
