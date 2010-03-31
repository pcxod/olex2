//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "splash.h"
#include "bapp.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgSplash *dlgSplash;
//---------------------------------------------------------------------------
__fastcall TdlgSplash::TdlgSplash(TComponent* Owner)
  : TForm(Owner)
{
  AnsiString spf( TBasicApp::GetBaseDir().c_str() );
  spf += "splash.jpg";
  if( FileExists(spf) )
  {
    iImage->Picture->LoadFromFile(spf);
    this->Width = iImage->Picture->Width;
    this->Height = iImage->Picture->Height +
                   pOverall->Height +
                   pFile->Height +
                   stVersion->Height;
  }
}
//---------------------------------------------------------------------------
