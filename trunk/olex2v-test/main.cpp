//---------------------------------------------------------------------------

#include <vcl.h>
#include <sys/stat.h>
#pragma hdrstop

#include "main.h"
#include "olex2v.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner)  {
  dc = NULL;
}
//---------------------------------------------------------------------------
__fastcall TForm1::~TForm1() {
  olxv_Finalize();
  if( dc != NULL )
    ReleaseDC(Handle, dc);
//  DragAcceptFiles(Handle, TRUE);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormPaint(TObject *Sender)
{
  olxv_OnPaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormShow(TObject *Sender)  {
  dc = GetDC(Handle);
  olxv_Initialize(dc, Width, Height );
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
void __fastcall TForm1::FormResize(TObject *Sender)  {
  olxv_OnSize(Width, Height);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Listen1Click(TObject *Sender)  {
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
      struct stat st;
      if( stat( FileName.c_str(), &st ) == 0 && (st.st_mode & S_IREAD) != 0 )  {
        olxv_OnFileChanged(FileName.c_str());
        Refresh();
        age = fa;
      }
    }
  }
}
//---------------------------------------------------------------------------
void TForm1::CheckLabels()  {
  short t = miLabels->Checked ? olxv_LabelsAll : 0;
  if( miH->Checked )  t |= olxv_LabelsH;
  else                t &= ~olxv_LabelsH;
  if( miQ->Checked )  t |= olxv_LabelsQ;
  else                t &= ~olxv_LabelsQ;

  olxv_ShowLabels(t);

  olxv_OnPaint();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::miLabelsClick(TObject *Sender)  {
  miLabels->Checked = !miLabels->Checked;
  CheckLabels();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Telp1Click(TObject *Sender)  {
  olxv_DrawStyle( olxv_DrawStyleTelp );
  olxv_OnPaint();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Pers1Click(TObject *Sender)  {
  olxv_DrawStyle( olxv_DrawStylePers );
  olxv_OnPaint();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Sfil1Click(TObject *Sender)  {
  olxv_DrawStyle( olxv_DrawStyleSfil );
  olxv_OnPaint();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::miHClick(TObject *Sender)  {
  miH->Checked = !miH->Checked;
  CheckLabels();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::miQClick(TObject *Sender)  {
  miQ->Checked = !miQ->Checked;
  CheckLabels();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::miCellClick(TObject *Sender)  {
  miCell->Checked = !miCell->Checked;   
  olxv_ShowCell( miCell->Checked );
}
//---------------------------------------------------------------------------

void __fastcall TForm1::miClearClick(TObject *Sender)  {
  olxv_Clear();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::miTestClick(TObject *Sender)  {
  if( FileName.IsEmpty() )  return;
  for( int i=0; i < 1024; i++ )  {
    olxv_OnFileChanged(FileName.c_str());
    Refresh();
  }
}
//---------------------------------------------------------------------------

