//---------------------------------------------------------------------------
#ifndef draw_molH
#define draw_molH
#include <vcl.h>
#include "evpoint.h"
#include "ebasis.h"

#include "satom.h"
#include "xfiles.h"
//---------------------------------------------------------------------------

struct TDrawSort {
  vec3d P;
  TSAtom *A;
};

class TOrganiser  {
  TXFile* FXFile;
  Graphics::TBitmap *FBitmap;

  bool MouseDown;
  int MouseX, MouseY;
  TMouseButton Button;

public:
  TEBasis Basis;
  __fastcall TOrganiser(Graphics::TBitmap *B);
  __fastcall ~TOrganiser();
  void _fastcall Update();
  void _fastcall CalcZoom();
  __property TXFile* XFile  = {read = FXFile};
  void _fastcall Draw();

  void __fastcall OnResize(TObject *Sender);
  void __fastcall OnClick(TObject *Sender);
  bool __fastcall OnMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
  bool __fastcall OnMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
  bool __fastcall OnMouseUp(TObject *Sender, TMouseButton Button,  TShiftState Shift, int X, int Y);
};
#endif

