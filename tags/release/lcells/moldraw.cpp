//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "moldraw.h"
#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgMolDraw *dlgMolDraw;
//---------------------------------------------------------------------------
__fastcall TdlgMolDraw::TdlgMolDraw(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TdlgMolDraw::FormPaint(TObject *Sender)
{
  Canvas->Draw(0, 0, dlgMain->Bitmap);
}
//---------------------------------------------------------------------------
void __fastcall TdlgMolDraw::FormResize(TObject *Sender)
{
  dlgMain->Bitmap->Height = ClientHeight;
  dlgMain->Bitmap->Width = ClientWidth;
  dlgMain->Organiser->OnResize(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TdlgMolDraw::FormMouseDown(TObject *Sender,
    TMouseButton B, TShiftState Shift, int X, int Y)
{
  dlgMain->Organiser->OnMouseDown(Sender, B, Shift, X, Y);
}
//---------------------------------------------------------------------------
void __fastcall TdlgMolDraw::FormMouseMove(TObject *Sender,
    TShiftState Shift, int X, int Y)
{
  dlgMain->Organiser->OnMouseMove(Sender, Shift, X, Y);
}
//---------------------------------------------------------------------------
void __fastcall TdlgMolDraw::FormClick(TObject *Sender)
{
  dlgMain->Organiser->OnClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TdlgMolDraw::FormMouseUp(TObject *Sender,
    TMouseButton Button, TShiftState Shift, int X, int Y)
{
  dlgMain->Organiser->OnMouseUp(Sender, Button, Shift, X, Y);
}
//---------------------------------------------------------------------------

void __fastcall TdlgMolDraw::Grow1Click(TObject *Sender)  {
  TSAtomPList toGrow;
  for( int i=0; i < dlgMain->Organiser->XFile->GetLattice().AtomCount(); i++ )
    if( dlgMain->Organiser->XFile->GetLattice().IsExpandable( dlgMain->Organiser->XFile->GetLattice().GetAtom(i)) )
      toGrow.Add( &dlgMain->Organiser->XFile->GetLattice().GetAtom(i) );
  dlgMain->Organiser->XFile->GetLattice().GrowAtoms(toGrow, false, NULL);
  //dlgMain->Organiser->XFile->GetLattice().GenerateWholeContent(NULL);
  dlgMain->Organiser->Update(); // do zoom and mpln calculations
  dlgMain->Organiser->Draw();
}
//---------------------------------------------------------------------------

