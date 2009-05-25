//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include "bapp.h"
//---------------------------------------------------------------------------
class TfMain : public TForm
{
__published:	// IDE-managed Components
  TLabel *Label5;
  TLabel *Label4;
  TLabel *Label3;
  TSpeedButton *sbSrc;
  TSpeedButton *sbDest;
  TLabel *Label2;
  TLabel *Label1;
  TLabel *lLog;
  TLabel *Label6;
  TStatusBar *sbSyncStatus;
  TRichEdit *reEdit;
  TStaticText *stCurrent;
  TProgressBar *pbOverall;
  TProgressBar *pbFC;
  TStaticText *stTotal;
  TStaticText *stSpeed;
  TEdit *eDest;
  TEdit *eSrc;
  TBitBtn *bbRun;
  void __fastcall sbSrcClick(TObject *Sender);
  void __fastcall bbRunClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
  __fastcall TfMain(TComponent* Owner);
  TBasicApp* bapp;
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif

