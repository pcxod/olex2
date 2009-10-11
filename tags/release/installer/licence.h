//---------------------------------------------------------------------------

#ifndef licenceH
#define licenceH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TdlgLicence : public TForm
{
__published:	// IDE-managed Components
  TRichEdit *reEdit;
  TBitBtn *bbOk;
  TBitBtn *bbCancel;
private:	// User declarations
public:		// User declarations
  __fastcall TdlgLicence(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgLicence *dlgLicence;
//---------------------------------------------------------------------------
#endif
