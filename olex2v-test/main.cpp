//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include "olex2v.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner)  {}
//---------------------------------------------------------------------------
__fastcall TForm1::~TForm1() {
  olxv_Finalize();
//  DragAcceptFiles(Handle, TRUE);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormPaint(TObject *Sender)
{
  olxv_OnPaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormShow(TObject *Sender)  {
  HDC hdc = GetDC(Handle);
  olxv_Initialize(hdc, Width, Height );
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormMouseDown(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
  short shift_state=0, button=0;
  if( Shift.Contains(ssAlt) )   shift_state |= olxv_ShiftAlt;
  if( Shift.Contains(ssShift) ) shift_state |= olxv_ShiftShift;
  if( Shift.Contains(ssCtrl) )  shift_state |= olxv_ShiftCtrl;

  if( Button == mbLeft )        button |= olxv_MouseLeft;
  else if( Button == mbRight )  button |= olxv_MouseRight;
  else if( Button == mbMiddle ) button |= olxv_MouseMiddle;

  olxv_OnMouse(X, Y + (Height-ClientHeight), olxv_MouseDown, button, shift_state);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormMouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{
  short shift_state=0;
  if( Shift.Contains(ssAlt) )   shift_state |= olxv_ShiftAlt;
  if( Shift.Contains(ssShift) ) shift_state |= olxv_ShiftShift;
  if( Shift.Contains(ssCtrl) )  shift_state |= olxv_ShiftCtrl;
  if( !olxv_OnMouse(X, Y + (Height-ClientHeight), olxv_MouseMove, 0, shift_state) )  {
//    Hint = olxv_GetObjectLabelAt(X, Y + (Height-ClientHeight));
//    Update();
  }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormMouseUp(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
  short shift_state=0, button=0;
  if( Shift.Contains(ssAlt) )   shift_state |= olxv_ShiftAlt;
  if( Shift.Contains(ssShift) ) shift_state |= olxv_ShiftShift;
  if( Shift.Contains(ssCtrl) )  shift_state |= olxv_ShiftCtrl;

  if( Button == mbLeft )        button |= olxv_MouseLeft;
  else if( Button == mbRight )  button |= olxv_MouseRight;
  else if( Button == mbMiddle ) button |= olxv_MouseMiddle;

  if( olxv_OnMouse(X, Y + (Height-ClientHeight), olxv_MouseUp, button, shift_state) )
    sbStatus->Panels->Items[0]->Text = olxv_GetSelectionInfo();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormResize(TObject *Sender)
{
  olxv_OnSize(Width, Height);  
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Listen1Click(TObject *Sender)
{
   if( dlgOpen->Execute() )  {
     FileName = dlgOpen->FileName;
   }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::tTimerTimer(TObject *Sender)  {
  static long age = 0;
  if( FileName.IsEmpty() )  return;
  if( FileExists( FileName ) )  {
    int fa = FileAge(FileName);
    if( fa != age )  {
      olxv_OnFileChanged(FileName.c_str());
      Refresh();
      age = fa;
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::miLabelsClick(TObject *Sender)  {
  miLabels->Checked = !miLabels->Checked;
  olxv_ShowLabels(miLabels->Checked);
  olxv_OnPaint();
}
//---------------------------------------------------------------------------

