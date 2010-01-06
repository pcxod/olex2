//---------------------------------------------------------------------------

#ifndef progressH
#define progressH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include "ebase.h"
//---------------------------------------------------------------------------
class TdlgProgress : public TForm
{
__published:	// IDE-managed Components
	TProgressBar *pbBar;
	TStaticText *stAction;
	TBitBtn *bbCancel;
	void __fastcall bbCancelClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:	// User declarations
	TList *Forms;
public:		// User declarations
	__fastcall TdlgProgress(TComponent* Owner);
	bool	*OnCancel;
	__fastcall ~TdlgProgress();
	void _fastcall SetAction(const olxstr& A);
	void _fastcall AddForm(TForm *F);
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgProgress *dlgProgress;
//---------------------------------------------------------------------------
#endif
