//$$---- Active Form CPP ---- (stActiveFormSource)
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <atl\atlvcl.h>

#include "Olex2xImpl.h"
#include "interface.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOlex2x *Olex2x;
//---------------------------------------------------------------------------
__fastcall TOlex2x::TOlex2x(HWND ParentWindow)
        : TActiveForm(ParentWindow)
{
  hdc = NULL;
  width = height = -1;
//  DragAcceptFiles(Handle, TRUE);
}
//---------------------------------------------------------------------------
__fastcall TOlex2x::~TOlex2x()  {
}

STDMETHODIMP TOlex2xImpl::_set_Font(IFontDisp** Value)
{
  try
  {
    const DISPID dispid = -512;
    if (FireOnRequestEdit(dispid) == S_FALSE)
      return S_FALSE;
    SetVclCtlProp(m_VclCtl->Font, Value);
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_Active(VARIANT_BOOL* Value)
{
  try
  {
   *Value = m_VclCtl->Active;
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_AlignDisabled(VARIANT_BOOL* Value)
{
  try
  {
   *Value = m_VclCtl->AlignDisabled;
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_AutoScroll(VARIANT_BOOL* Value)
{
  try
  {
   *Value = m_VclCtl->AutoScroll;
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_AutoSize(VARIANT_BOOL* Value)
{
  try
  {
   *Value = m_VclCtl->AutoSize;
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_AxBorderStyle(TxActiveFormBorderStyle* Value)
{
  try
  {
   *Value = (TxActiveFormBorderStyle)(m_VclCtl->AxBorderStyle);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_BorderWidth(long* Value)
{
  try
  {
   *Value = (long)(m_VclCtl->BorderWidth);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_Caption(BSTR* Value)
{
  try
  {
    *Value = WideString(m_VclCtl->Caption).Copy();
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_Color(::OLE_COLOR* Value)
{
  try
  {
   *Value = (::OLE_COLOR)(m_VclCtl->Color);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_DoubleBuffered(VARIANT_BOOL* Value)
{
  try
  {
   *Value = m_VclCtl->DoubleBuffered;
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_DropTarget(VARIANT_BOOL* Value)
{
  try
  {
   *Value = m_VclCtl->DropTarget;
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_Enabled(VARIANT_BOOL* Value)
{
  try
  {
   *Value = m_VclCtl->Enabled;
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_Font(IFontDisp** Value)
{
  try
  {
    GetVclCtlProp(m_VclCtl->Font, Value);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_HelpFile(BSTR* Value)
{
  try
  {
    *Value = WideString(m_VclCtl->HelpFile).Copy();
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_KeyPreview(VARIANT_BOOL* Value)
{
  try
  {
   *Value = m_VclCtl->KeyPreview;
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_PixelsPerInch(long* Value)
{
  try
  {
   *Value = (long)(m_VclCtl->PixelsPerInch);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_PrintScale(TxPrintScale* Value)
{
  try
  {
   *Value = (TxPrintScale)(m_VclCtl->PrintScale);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_Scaled(VARIANT_BOOL* Value)
{
  try
  {
   *Value = m_VclCtl->Scaled;
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_Visible(VARIANT_BOOL* Value)
{
  try
  {
   *Value = m_VclCtl->Visible;
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::get_VisibleDockClientCount(long* Value)
{
  try
  {
   *Value = (long)(m_VclCtl->VisibleDockClientCount);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_AutoScroll(VARIANT_BOOL Value)
{
  try
  {
    const DISPID dispid = 2;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->AutoScroll = Value;
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_AutoSize(VARIANT_BOOL Value)
{
  try
  {
    const DISPID dispid = 3;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->AutoSize = Value;
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_AxBorderStyle(TxActiveFormBorderStyle Value)
{
  try
  {
    const DISPID dispid = 4;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->AxBorderStyle = (TActiveFormBorderStyle)(Value);
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_BorderWidth(long Value)
{
  try
  {
    const DISPID dispid = 5;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->BorderWidth = (int)(Value);
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_Caption(BSTR Value)
{
  try
  {
    const DISPID dispid = -518;
    if (FireOnRequestEdit(dispid) == S_FALSE)
      return S_FALSE;
    m_VclCtl->Caption = AnsiString(Value);
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_Color(::OLE_COLOR Value)
{
  try
  {
    const DISPID dispid = -501;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->Color = (TColor)(Value);
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_DoubleBuffered(VARIANT_BOOL Value)
{
  try
  {
    const DISPID dispid = 13;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->DoubleBuffered = Value;
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_DropTarget(VARIANT_BOOL Value)
{
  try
  {
    const DISPID dispid = 11;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->DropTarget = Value;
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_Enabled(VARIANT_BOOL Value)
{
  try
  {
    const DISPID dispid = -514;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->Enabled = Value;
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_Font(IFontDisp* Value)
{
  try
  {
    const DISPID dispid = -512;
    if (FireOnRequestEdit(dispid) == S_FALSE)
      return S_FALSE;
    SetVclCtlProp(m_VclCtl->Font, Value);
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_HelpFile(BSTR Value)
{
  try
  {
    const DISPID dispid = 12;
    if (FireOnRequestEdit(dispid) == S_FALSE)
      return S_FALSE;
    m_VclCtl->HelpFile = AnsiString(Value);
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_KeyPreview(VARIANT_BOOL Value)
{
  try
  {
    const DISPID dispid = 6;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->KeyPreview = Value;
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_PixelsPerInch(long Value)
{
  try
  {
    const DISPID dispid = 7;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->PixelsPerInch = (int)(Value);
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_PrintScale(TxPrintScale Value)
{
  try
  {
    const DISPID dispid = 8;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->PrintScale = (TPrintScale)(Value);
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_Scaled(VARIANT_BOOL Value)
{
  try
  {
    const DISPID dispid = 9;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->Scaled = Value;
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};


STDMETHODIMP TOlex2xImpl::set_Visible(VARIANT_BOOL Value)
{
  try
  {
    const DISPID dispid = 1;
    if (FireOnRequestEdit(dispid) == S_FALSE)
     return S_FALSE;
    m_VclCtl->Visible = Value;
    FireOnChanged(dispid);
  }
  catch(Exception &e)
  {
    return Error(e.Message.c_str(), IID_IOlex2x);
  }
  return S_OK;
};



void __fastcall TOlex2xImpl::ActivateEvent(TObject *Sender)
{
  Fire_OnActivate();
}


void __fastcall TOlex2xImpl::ClickEvent(TObject *Sender)
{
  Fire_OnClick();
}


void __fastcall TOlex2xImpl::CreateEvent(TObject *Sender)
{
  Fire_OnCreate();
}


void __fastcall TOlex2xImpl::DblClickEvent(TObject *Sender)
{
  Fire_OnDblClick();
}


void __fastcall TOlex2xImpl::DeactivateEvent(TObject *Sender)
{
  Fire_OnDeactivate();
}


void __fastcall TOlex2xImpl::DestroyEvent(TObject *Sender)
{
  Fire_OnDestroy();
}


void __fastcall TOlex2xImpl::KeyPressEvent(TObject *Sender, char &Key)
{
  short TempKey;
  TempKey = (short)Key;
  Fire_OnKeyPress(&TempKey);
  Key = (short)TempKey;
}


void __fastcall TOlex2xImpl::PaintEvent(TObject *Sender)
{
  Fire_OnPaint();
}




void __fastcall TOlex2x::ActiveFormMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  short shift_state=0, button=0;
  if( Shift.Contains(ssAlt) )   shift_state |= olxv_ShiftAlt;
  if( Shift.Contains(ssShift) ) shift_state |= olxv_ShiftShift;
  if( Shift.Contains(ssCtrl) )  shift_state |= olxv_ShiftCtrl;

  if( Button == Controls::mbLeft )        button |= olxv_MouseLeft;
  else if( Button == Controls::mbRight )  button |= olxv_MouseRight;
  else if( Button == Controls::mbMiddle ) button |= olxv_MouseMiddle;

  olxv_OnMouse(X, Y + (Height-ClientHeight), olxv_MouseDown, button, shift_state);
}
//---------------------------------------------------------------------------
void __fastcall TOlex2x::ActiveFormMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  short shift_state=0, button=0;
  if( Shift.Contains(ssAlt) )   shift_state |= olxv_ShiftAlt;
  if( Shift.Contains(ssShift) ) shift_state |= olxv_ShiftShift;
  if( Shift.Contains(ssCtrl) )  shift_state |= olxv_ShiftCtrl;

  if( Button == Controls::mbLeft )        button |= olxv_MouseLeft;
  else if( Button == Controls::mbRight )  button |= olxv_MouseRight;
  else if( Button == Controls::mbMiddle ) button |= olxv_MouseMiddle;

  olxv_OnMouse(X, Y + (Height-ClientHeight), olxv_MouseUp, button, shift_state);
}
//---------------------------------------------------------------------------
void __fastcall TOlex2x::ActiveFormMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
  short shift_state=0;
  if( Shift.Contains(ssAlt) )   shift_state |= olxv_ShiftAlt;
  if( Shift.Contains(ssShift) ) shift_state |= olxv_ShiftShift;
  if( Shift.Contains(ssCtrl) )  shift_state |= olxv_ShiftCtrl;
  olxv_OnMouse(X, Y + (Height-ClientHeight), olxv_MouseMove, 0, shift_state);
}
//---------------------------------------------------------------------------
void __fastcall TOlex2x::Button1Click(TObject *Sender)  {
  olxv_OnSize(Width, Height);
  if( dlgOpen->Execute() )
    FileName = dlgOpen->FileName;
}
//---------------------------------------------------------------------------
void _fastcall TOlex2x::GetClipboardFiles(TMessage &M)  {
	char* bf;
	int len = DragQueryFile((HANDLE)M.WParam, 0, NULL, 200);
	if( len >=0 )
		bf = new char[len+2];
	else
		return;
	DragQueryFile((HANDLE)M.WParam, 0, bf, len+1);
	DragFinish((HANDLE)M.WParam);
  AnsiString ext = ExtractFileExt(bf).UpperCase();
	if( ext != ".INS" && ext != ".RES")	{
    Application->MessageBox("Shelx ins/res files expected", "Error", MB_OK|MB_ICONERROR);
    delete [] bf;
		return;
	}
  FileName = bf;
//  olxv_OnFileChanged(bf);
	delete [] bf;
}
//---------------------------------------------------------------------------
void __fastcall TOlex2x::tTimerTimer(TObject *Sender)  {
  static long age = 0;
  if( FileName.IsEmpty() )  return;
  if( FileExists( FileName ) )  {
    int fa = FileAge(FileName);
    if( fa != age )  {
      olxv_OnFileChanged(FileName.c_str());
      olxv_OnPaint();
      //Refresh();
      age = fa;
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TOlex2x::ActiveFormDestroy(TObject *Sender)  {
  FileName = "";
  if( hdc != NULL )
    ReleaseDC(Handle, hdc);
}
//---------------------------------------------------------------------------

void __fastcall TOlex2x::ActiveFormCreate(TObject *Sender)  {
  hdc = GetDC(Handle);
  olxv_Initialize("", hdc, Width, Height );
}
//---------------------------------------------------------------------------

void __fastcall TOlex2x::ActiveFormPaint(TObject *Sender)  {
  olxv_OnPaint();
}
//---------------------------------------------------------------------------


