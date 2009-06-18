//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <jpeg.hpp>

#include "actions.h"
#include "bapp.h"
#include "filesystem.h"
#include "estrlist.h"
//---------------------------------------------------------------------------
class TdlgMain : public TForm
{
__published:  // IDE-managed Components
  TTimer *tTimer;
  void __fastcall tTimerTimer(TObject *Sender);
private: // User declarations
  TBasicApp *FBApp;
  void Launch();
  bool UpdateInstallation();
public: // User declarations
  __fastcall TdlgMain(TComponent* Owner);
  virtual __fastcall ~TdlgMain();
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgMain *dlgMain;
//---------------------------------------------------------------------------
#endif
