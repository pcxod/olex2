//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "mf.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfMain *fMain;
//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfMain::Test1Click(TObject *Sender)
{
  Application->MessageBox("Hello", "SS", 1);  
}
//---------------------------------------------------------------------------
void __fastcall TfMain::CPP1Click(TObject *Sender)  {
  if( dlgOpen->Execute() )  {

  }
}
//---------------------------------------------------------------------------

