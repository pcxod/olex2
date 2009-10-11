//---------------------------------------------------------------------------

#ifndef splashH
#define splashH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <jpeg.hpp>
#include <ComCtrls.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TdlgSplash : public TForm
{
__published:	// IDE-managed Components
  TStaticText *stVersion;
  TImage *iImage;
  TPanel *pFile;
  TProgressBar *pbFProgress;
  TStaticText *stFileName;
  TPanel *pOverall;
  TProgressBar *pbOProgress;
  TStaticText *StaticText2;
  TStaticText *StaticText3;
private:	// User declarations
public:		// User declarations
  __fastcall TdlgSplash(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgSplash *dlgSplash;
//---------------------------------------------------------------------------
#endif
