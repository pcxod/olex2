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
  TPageControl *pgTasks;
  TTabSheet *tbSync;
  TTabSheet *tsDublicates;
  TStatusBar *sbSyncStatus;
  TRichEdit *reEdit;
  TStaticText *stCurrent;
  TLabel *Label5;
  TLabel *Label4;
  TLabel *Label3;
  TProgressBar *pbOverall;
  TProgressBar *pbFC;
  TStaticText *stTotal;
  TStaticText *stSpeed;
  TEdit *eDest;
  TEdit *eSrc;
  TSpeedButton *sbSrc;
  TSpeedButton *sbDest;
  TLabel *Label2;
  TLabel *Label1;
  TBitBtn *bbRun;
  TLabel *lLog;
  TLabel *Label6;
  TLabel *Label7;
  TEdit *eDupSource;
  TSpeedButton *SpeedButton1;
  TBitBtn *BitBtn1;
  TLabel *Label8;
  TProgressBar *pbDupAverall;
  TStaticText *StaticText1;
  TLabel *Label9;
  TProgressBar *pbDupFC;
  TLabel *Label10;
  TStaticText *stDupCurrentFile;
  TLabel *Label11;
  TStaticText *stDupAverageSpeed;
  TLabel *Label12;
  TRichEdit *RichEdit1;
  TListView *lvDublicates;
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

