//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "splash.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgSplash *dlgSplash;
//---------------------------------------------------------------------------
__fastcall TdlgSplash::TdlgSplash(TComponent* Owner)
  : TForm(Owner)
{
  if( FileExists("splash.jpg") )
  {
    iImage->Picture->LoadFromFile("splash.jpg");
    this->Width = iImage->Picture->Width;
    this->Height = iImage->Picture->Height +
                   pOverall->Height +
                   pFile->Height +
                   stVersion->Height;
  }
}
//---------------------------------------------------------------------------
