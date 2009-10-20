#include "stdafx.h"
#include "installer.h"
#include "RepositoriesDlg.h"


IMPLEMENT_DYNAMIC(CRepositoriesDlg, CDialog)

CRepositoriesDlg::CRepositoriesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRepositoriesDlg::IDD, pParent)
{

}

CRepositoriesDlg::~CRepositoriesDlg()
{
}

BOOL CRepositoriesDlg::OnInitDialog()  {
	CDialog::OnInitDialog();
  return TRUE;
}
void CRepositoriesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRepositoriesDlg, CDialog)
END_MESSAGE_MAP()


// CRepositoriesDlg message handlers
