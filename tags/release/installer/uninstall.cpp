//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "uninstall.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgUninstall *dlgUninstall;
//---------------------------------------------------------------------------
__fastcall TdlgUninstall::TdlgUninstall(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TdlgUninstall::rgRemoveClick(TObject *Sender)
{
  cbRemoveUserSettings->Enabled = rgRemove->Checked;  
}
//---------------------------------------------------------------------------
