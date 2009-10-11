//---------------------------------------------------------------------------

#include <Filectrl.hpp>
#include <vcl.h>
#pragma hdrstop

#include "preferences.h"
#include "efile.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgPref *dlgPref;
//---------------------------------------------------------------------------
__fastcall TdlgPref::TdlgPref(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TdlgPref::lvPathsSelectItem(TObject *Sender,
      TListItem *Item, bool Selected)
{
  bbRemove->Enabled = Selected;
}
//---------------------------------------------------------------------------
void __fastcall TdlgPref::lvFilesSelectItem(TObject *Sender,
      TListItem *Item, bool Selected)
{
  bbRemoveF->Enabled = Selected;
}
//---------------------------------------------------------------------------
void __fastcall TdlgPref::bbAddFClick(TObject *Sender)
{
  if( dlgOpen->Execute() )
  {
    TListItem *I;
    AnsiString Tmp = TEFile::UNCFileName(dlgOpen->FileName.c_str()).c_str();
    for( int i=0; i < lvFiles->Items->Count; i++ )
    {
      I = lvFiles->Items->Item[i];
      if( I->Caption == Tmp )
      {
        Application->MessageBox("The file is already in the list.","Error", MB_OK|MB_ICONWARNING);
        return;
      }
    }
    I = lvFiles->Items->Add();
    I->Caption = Tmp;
  }
}
//---------------------------------------------------------------------------
void __fastcall TdlgPref::bbAddClick(TObject *Sender)
{
  AnsiString Tmp;
  if( !SelectDirectory(Tmp, TSelectDirOpts(), 0) )
    return;
  TListItem *I;
//  Tmp = TEFile::UNCFileName(Tmp);  does it works with folders?
  for( int i=0; i < lvPaths->Items->Count; i++ )
  {
    I = lvPaths->Items->Item[i];
    if( I->Caption == Tmp )
    {
      Application->MessageBox("The path is already in the list.","Error", MB_OK|MB_ICONWARNING);
      return;
    }
  }
  I = lvPaths->Items->Add();
  I->Caption = Tmp;
}
//---------------------------------------------------------------------------
void __fastcall TdlgPref::bbRemoveClick(TObject *Sender)
{
  if( lvPaths->Selected )
    lvPaths->Selected->Delete();
}
//---------------------------------------------------------------------------
void __fastcall TdlgPref::bbRemoveFClick(TObject *Sender)
{
  if( lvFiles->Selected )
    lvFiles->Selected->Delete();
}
//---------------------------------------------------------------------------

