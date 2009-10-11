//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TfMain : public TForm
{
__published:	// IDE-managed Components
  TImage *iSplash;
  TBitBtn *Continue;
  TBitBtn *Exit;
  TRadioButton *rbInstall;
  TRadioButton *rbWebsite;
  TRadioButton *rbRunFromCD;
  TImage *Image1;
  void __fastcall iSplashMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall iSplashMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
  void __fastcall iSplashMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall ExitClick(TObject *Sender);
  void __fastcall ContinueClick(TObject *Sender);
private:	// User declarations
  bool Dragging, MouseDown;
  int MouseDownX, MouseDownY;
  bool LaunchFile( const AnsiString& fileName );
  
public:		// User declarations
  __fastcall TfMain(TComponent* Owner);
  __fastcall ~TfMain();
};
//---------------------------------------------------------------------------
extern PACKAGE TfMain *fMain;
//---------------------------------------------------------------------------
#endif
