#pragma once
#include <afx.h>
#include "resource.h"
#include "ebase.h"

class CLicenceDlg : public CDialog  {
public:
	CLicenceDlg(CWnd* pParent, const olxstr& file_name);	// standard constructor
	enum { IDD = IDD_LICENCE };
protected:
  olxstr file_name;
protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
};
