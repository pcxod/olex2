//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "frame1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrMain *frMain;
//---------------------------------------------------------------------------
__fastcall TfrMain::TfrMain(TComponent* Owner)
  : TFrame(Owner)
{
}
//---------------------------------------------------------------------------