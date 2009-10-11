//$$---- Active Form HDR ---- (stActiveFormHdr)
//---------------------------------------------------------------------------


#ifndef Olex2xImplH
#define Olex2xImplH
//---------------------------------------------------------------------------
#include <classes.hpp>
#include <controls.hpp>
#include <stdCtrls.hpp>
#include <forms.hpp>
#include "Olex2xProj_TLB.h"
#include <AxCtrls.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ImgList.hpp>
#include <ToolWin.hpp>
//---------------------------------------------------------------------------
class TOlex2x : public TActiveForm
{
__published:	// IDE-managed Components
  TOpenDialog *dlgOpen1;
  TTimer *tTimer;
  TOpenDialog *dlgOpen;
  TToolBar *ToolBar1;
  TToolButton *tbOpen;
  TImageList *ilImages;
  TToolButton *ToolButton1;
  TSpeedButton *sbViewA;
  TSpeedButton *sbViewB;
  TSpeedButton *sbViewC;
        void __fastcall ActiveFormMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall ActiveFormMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall ActiveFormMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
        void __fastcall Button1Click(TObject *Sender);
  void __fastcall tTimerTimer(TObject *Sender);
  void __fastcall ActiveFormDestroy(TObject *Sender);
  void __fastcall ActiveFormCreate(TObject *Sender);
  void __fastcall ActiveFormPaint(TObject *Sender);
private:	// User declarations
  AnsiString FileName;
  HDC hdc;
  int width, height;
public:		// User declarations
    __fastcall TOlex2x(HWND ParentWindow);
    __fastcall TOlex2x(TComponent* AOwner): TActiveForm(AOwner) {};
    __fastcall ~TOlex2x();
protected:
	void _fastcall GetClipboardFiles(TMessage &Msg);
//BEGIN_MESSAGE_MAP
//	VCL_MESSAGE_HANDLER(WM_DROPFILES, TMessage, GetClipboardFiles);
//END_MESSAGE_MAP(TActiveForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TOlex2x *Olex2x;
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
class ATL_NO_VTABLE TOlex2xImpl:
  VCLCONTROL_IMPL(TOlex2xImpl, Olex2x, TOlex2x, IOlex2x, DIID_IOlex2xEvents)
{
  void __fastcall ActivateEvent(TObject *Sender);
  void __fastcall ClickEvent(TObject *Sender);
  void __fastcall CreateEvent(TObject *Sender);
  void __fastcall DblClickEvent(TObject *Sender);
  void __fastcall DeactivateEvent(TObject *Sender);
  void __fastcall DestroyEvent(TObject *Sender);
  void __fastcall KeyPressEvent(TObject *Sender, char &Key);
  void __fastcall PaintEvent(TObject *Sender);
public:

  void InitializeControl()
  {
    m_VclCtl->OnActivate = ActivateEvent;
    m_VclCtl->OnClick = ClickEvent;
    m_VclCtl->OnCreate = CreateEvent;
    m_VclCtl->OnDblClick = DblClickEvent;
    m_VclCtl->OnDeactivate = DeactivateEvent;
    m_VclCtl->OnDestroy = DestroyEvent;
    m_VclCtl->OnKeyPress = KeyPressEvent;
    m_VclCtl->OnPaint = PaintEvent;
  }

// The COM MAP entries declares the interfaces your object exposes (through
// QueryInterface). CComRootObjectEx::InternalQueryInterface only returns
// pointers for interfaces in the COM map. VCL controls exposed as OCXes
// have a minimum set of interfaces defined by the
// VCL_CONTROL_COM_INTERFACE_ENTRIES macro. Add other interfaces supported
// by your object with additional COM_INTERFACE_ENTRY[_xxx] macros.
//
BEGIN_COM_MAP(TOlex2xImpl)
  VCL_CONTROL_COM_INTERFACE_ENTRIES(IOlex2x)
END_COM_MAP()



// The PROPERTY map stores property descriptions, property DISPIDs,
// property page CLSIDs and IDispatch IIDs. You may use use
// IPerPropertyBrowsingImpl, IPersistPropertyBagImpl, IPersistStreamInitImpl,
// and ISpecifyPropertyPageImpl to utilize the information in you property
// map.
//
// NOTE: The BCB Wizard does *NOT* maintain your PROPERTY_MAP table. You must
//       add or remove entries manually.
//
BEGIN_PROPERTY_MAP(TOlex2xImpl)
  // PROP_PAGE(CLSID_Olex2xPage)
END_PROPERTY_MAP()

/* DECLARE_VCL_CONTROL_PERSISTENCE(CppClass, VclClass) is needed for VCL
 * controls to persist via the VCL streaming mechanism and not the ATL mechanism.
 * The macro adds static IPersistStreamInit_Load and IPersistStreamInit_Save
 * methods to your implementation class, overriding the methods in IPersistStreamImpl. 
 * This macro must be manually undefined or removed if you port to C++Builder 4.0. */

DECLARE_VCL_CONTROL_PERSISTENCE(TOlex2xImpl, TOlex2x);

// The DECLARE_ACTIVEXCONTROL_REGISTRY macro declares a static 'UpdateRegistry' 
// routine which registers the basic information about your control. The
// parameters expected by the macro are the ProgId & the ToolboxBitmap ID of
// your control.
//
DECLARE_ACTIVEXCONTROL_REGISTRY("Olex2xProj.Olex2x", 1);

protected: 
  STDMETHOD(_set_Font(IFontDisp** Value));
  STDMETHOD(get_Active(VARIANT_BOOL* Value));
  STDMETHOD(get_AlignDisabled(VARIANT_BOOL* Value));
  STDMETHOD(get_AutoScroll(VARIANT_BOOL* Value));
  STDMETHOD(get_AutoSize(VARIANT_BOOL* Value));
  STDMETHOD(get_AxBorderStyle(TxActiveFormBorderStyle* Value));
  STDMETHOD(get_BorderWidth(long* Value));
  STDMETHOD(get_Caption(BSTR* Value));
  STDMETHOD(get_Color(::OLE_COLOR* Value));
  STDMETHOD(get_DoubleBuffered(VARIANT_BOOL* Value));
  STDMETHOD(get_DropTarget(VARIANT_BOOL* Value));
  STDMETHOD(get_Enabled(VARIANT_BOOL* Value));
  STDMETHOD(get_Font(IFontDisp** Value));
  STDMETHOD(get_HelpFile(BSTR* Value));
  STDMETHOD(get_KeyPreview(VARIANT_BOOL* Value));
  STDMETHOD(get_PixelsPerInch(long* Value));
  STDMETHOD(get_PrintScale(TxPrintScale* Value));
  STDMETHOD(get_Scaled(VARIANT_BOOL* Value));
  STDMETHOD(get_Visible(VARIANT_BOOL* Value));
  STDMETHOD(get_VisibleDockClientCount(long* Value));
  STDMETHOD(set_AutoScroll(VARIANT_BOOL Value));
  STDMETHOD(set_AutoSize(VARIANT_BOOL Value));
  STDMETHOD(set_AxBorderStyle(TxActiveFormBorderStyle Value));
  STDMETHOD(set_BorderWidth(long Value));
  STDMETHOD(set_Caption(BSTR Value));
  STDMETHOD(set_Color(::OLE_COLOR Value));
  STDMETHOD(set_DoubleBuffered(VARIANT_BOOL Value));
  STDMETHOD(set_DropTarget(VARIANT_BOOL Value));
  STDMETHOD(set_Enabled(VARIANT_BOOL Value));
  STDMETHOD(set_Font(IFontDisp* Value));
  STDMETHOD(set_HelpFile(BSTR Value));
  STDMETHOD(set_KeyPreview(VARIANT_BOOL Value));
  STDMETHOD(set_PixelsPerInch(long Value));
  STDMETHOD(set_PrintScale(TxPrintScale Value));
  STDMETHOD(set_Scaled(VARIANT_BOOL Value));
  STDMETHOD(set_Visible(VARIANT_BOOL Value));
};
//---------------------------------------------------------------------------
#endif
