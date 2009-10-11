//---------------------------------------------------------------------------

#include <vcl.h>
#include <filectrl.hpp>
#pragma hdrstop

#include "progress.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgProgress *dlgProgress;
//---------------------------------------------------------------------------
__fastcall TdlgProgress::TdlgProgress(TComponent* Owner)
  : TForm(Owner)
{
  OnCancel = NULL;
  Forms = new TList;
}
//---------------------------------------------------------------------------
void __fastcall TdlgProgress::bbCancelClick(TObject *Sender)
{
  if( OnCancel )
    *OnCancel = true;
  for( int i=0; i < Forms->Count; i++ )
  {
    ((TForm*)Forms->Items[i])->Enabled = true;
    ((TForm*)Forms->Items[i])->BringToFront();
  }
  Forms->Clear();
}
//---------------------------------------------------------------------------

__fastcall TdlgProgress::~TdlgProgress()
{
  for( int i=0; i < Forms->Count; i++ )
  {
    ((TForm*)Forms->Items[i])->Enabled = true;
    ((TForm*)Forms->Items[i])->BringToFront();
  }
  delete Forms;
  return;
}
void _fastcall TdlgProgress::SetAction(const olxstr& A)  {
  stAction->Caption = MinimizeName(A.c_str(), Canvas, 390);
  stAction->Repaint();
}

void __fastcall TdlgProgress::FormShow(TObject *Sender)
{
  for( int i=0; i < Forms->Count; i++ )
    ((TForm*)Forms->Items[i])->Enabled = false;
}
//---------------------------------------------------------------------------
void _fastcall TdlgProgress::AddForm(TForm *F)
{
  Forms->Add(F);
}
//---------------------------------------------------------------------------

