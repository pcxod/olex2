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
#include "E:\SVN\srcf-svn\trunk\sdl\bapp.h"
//---------------------------------------------------------------------------
class TfMain : public TForm
{
__published:	// IDE-managed Components
  TLabel *Label1;
  TLabel *Label2;
  TEdit *eDest;
  TEdit *eSrc;
  TSpeedButton *sbDest;
  TSpeedButton *sbSrc;
  TBitBtn *bbRun;
  TRichEdit *reEdit;
  TProgressBar *pbOverall;
  TProgressBar *pbFC;
  TLabel *Label3;
  TLabel *Label4;
  TStaticText *stCurrent;
  TLabel *Label5;
  TStaticText *stTotal;
  TStaticText *stSpeed;
  TStatusBar *sbStatus;
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

