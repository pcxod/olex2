//----------------------------------------------------------------------------
#ifndef aboutH
#define aboutH
//----------------------------------------------------------------------------
#include <vcl\System.hpp>
#include <vcl\Windows.hpp>
#include <vcl\SysUtils.hpp>
#include <vcl\Classes.hpp>
#include <vcl\Graphics.hpp>
#include <vcl\Forms.hpp>
#include <vcl\Controls.hpp>
#include <vcl\StdCtrls.hpp>
#include <vcl\Buttons.hpp>
#include <vcl\ExtCtrls.hpp>
//----------------------------------------------------------------------------
class TdlgAbout : public TForm
{
__published:
	TPanel *Panel1;
	TImage *ProgramIcon;
	TLabel *ProductName;
	TLabel *Version;
	TButton *OKButton;
	TMemo *Memo1;
private:
public:
	virtual __fastcall TdlgAbout(TComponent* AOwner);
};
//----------------------------------------------------------------------------
extern PACKAGE TdlgAbout *dlgAbout;
//----------------------------------------------------------------------------
#endif    
