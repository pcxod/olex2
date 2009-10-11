//---------------------------------------------------------------------------

#ifndef iinfoH
#define iinfoH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TdlgIndexInfo : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	TLabel *Label2;
	TEdit *eCount;
	TEdit *eDate;
	TBitBtn *bbClose;
	TGroupBox *GroupBox1;
	TBitBtn *bbClean;
	TGroupBox *GroupBox2;
	TRadioButton *rbAllDrives;
	TRadioButton *rbFromFolder;
	TEdit *eDir;
	TSpeedButton *sbBrowse;
	TBitBtn *bbUpdate;
	TLabel *Label3;
	TLabel *Label4;
	TBitBtn *bbClear;
	TCheckBox *cbDead;
	TCheckBox *cbCell;
	TEdit *eLimit;
	TRadioButton *rbList;
	void __fastcall sbBrowseClick(TObject *Sender);
	void __fastcall bbCleanClick(TObject *Sender);
	void __fastcall bbUpdateClick(TObject *Sender);
	void __fastcall bbClearClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TdlgIndexInfo(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgIndexInfo *dlgIndexInfo;
//---------------------------------------------------------------------------
#endif
