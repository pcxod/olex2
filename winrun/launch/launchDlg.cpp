#include "stdafx.h"
//#include <atlstr.h>
#include <atlimage.h>

#include "launch.h"
#include "launchDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

MainDlg::MainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MainDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON);
  mouse_down = false;
}

void MainDlg::DoDataExchange(CDataExchange* pDX)  {
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(MainDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// MainDlg message handlers

BOOL MainDlg::OnInitDialog()  {
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

  CImage img;
  img.Load( _T("splash.jpg") );
  const int width = img.GetWidth(),
    height = img.GetHeight();
  HBITMAP bmp = img.Detach();
  CWnd* ctrl_img = GetDlgItem(IDC_IMAGE);
  ctrl_img->SendMessage(STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);
  CWnd* ctrl_version = GetDlgItem(IDC_VERSION);
  ctrl_version->SetWindowPos(NULL, 0, height, width, 0, SWP_NOSIZE|SWP_NOZORDER);
  RECT version_size;
  ctrl_version->GetWindowRect(&version_size);
  CWnd* ctrl_file = GetDlgItem(IDC_FILE);
  const int y_inc = version_size.bottom - version_size.top;
  ctrl_file->SetWindowPos(NULL, 0, height + y_inc, width/2, y_inc, SWP_NOZORDER); 
  CWnd* ctrl_file_progress = GetDlgItem(IDC_PB_FILE);
  ctrl_file_progress->SetWindowPos(NULL, width/2, height + y_inc, width/2, y_inc, SWP_NOZORDER); 
  CWnd* ctrl_overall_progress = GetDlgItem(IDC_PB_OVERALL);
  ctrl_overall_progress->SetWindowPos(NULL, 0, height + y_inc*2, width, y_inc, SWP_NOZORDER); 
  SetWindowPos(&wndTopMost, 0, 0, width, height + y_inc*3, SWP_NOREPOSITION); 
  return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void MainDlg::OnPaint()  {
	if (IsIconic()) {
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR MainDlg::OnQueryDragIcon()  {
	return static_cast<HCURSOR>(m_hIcon);
}


void MainDlg::OnLButtonDown(UINT nFlags, CPoint point)  {
  mouse_down = true;
  mouse_pos = point;
  CDialog::OnLButtonDown(nFlags, point);
}

void MainDlg::OnLButtonUp(UINT nFlags, CPoint point) {
  mouse_down = false;
  CDialog::OnLButtonUp(nFlags, point);
}

void MainDlg::OnMouseMove(UINT nFlags, CPoint point)  {
  if( (nFlags&MK_LBUTTON) != 0 && mouse_down )  {
    RECT pos;
    GetWindowRect(&pos);
    int x_inc = point.x - mouse_pos.x;
    int y_inc = point.y - mouse_pos.y;
    SetWindowPos(&wndTopMost, pos.left + x_inc, pos.top + y_inc, 0, 0, SWP_NOSIZE|SWP_NOZORDER); 
  }
  CDialog::OnMouseMove(nFlags, point);
}

void MainDlg::SetFileName(const olxstr &fn) {
  GetDlgItem(IDC_FILE)->SendMessage(WM_SETTEXT, 0, (LPARAM)fn.u_str());
}

void MainDlg::SetVersion(const olxstr &version) {
  GetDlgItem(IDC_VERSION)->SendMessage(WM_SETTEXT, 0, (LPARAM)version.u_str());
}


void MainDlg::SetFileProgressMax(double v) {
  max_file = v;
  GetDlgItem(IDC_PB_FILE)->SendMessage(PBM_SETRANGE, 0, MAKELPARAM(0, (int)v));  
}

void MainDlg::SetFileProgress(double v) {
  GetDlgItem(IDC_PB_FILE)->SendMessage(PBM_SETPOS, (WPARAM)(int)(v < 0 ? max_file : v), 0);  
  UpdateWindow();
}

void MainDlg::SetOverallProgressMax(double v) {
  max_overall = v;
  GetDlgItem(IDC_PB_OVERALL)->SendMessage(PBM_SETRANGE, 0, MAKELPARAM(0, (int)v));  
}

void MainDlg::SetOverallProgress(double v) {
  GetDlgItem(IDC_PB_OVERALL)->SendMessage(PBM_SETPOS, (WPARAM)(int)(v < 0 ? max_overall : v), 0);
}

void MainDlg::OnShowWindow(BOOL bShow, UINT nStatus) {
  CDialog::OnShowWindow(bShow, nStatus);
}
