#pragma once


// CRepositoriesDlg dialog

class CRepositoriesDlg : public CDialog
{
	DECLARE_DYNAMIC(CRepositoriesDlg)

public:
	CRepositoriesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRepositoriesDlg();

// Dialog Data
	enum { IDD = IDD_REPOSITORIES };
  void OnClose()  {  DestroyWindow();  }
  BOOL CRepositoriesDlg::OnInitDialog();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
