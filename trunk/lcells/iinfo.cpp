//---------------------------------------------------------------------------

#include <vcl.h>
#include <Filectrl.hpp>
#pragma hdrstop

#include "iinfo.h"
#include "main.h"
#include "shellutil.h"
#include "etime.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgIndexInfo *dlgIndexInfo;
//---------------------------------------------------------------------------
__fastcall TdlgIndexInfo::TdlgIndexInfo(TComponent* Owner) : TForm(Owner)  {
  eCount->Text = dlgMain->Index->GetCount();
  eDate->Text = TETime::FormatDateTime( dlgMain->Index->Updated ).c_str();
}
//..............................................................................
void __fastcall TdlgIndexInfo::sbBrowseClick(TObject *Sender)  {
  olxstr d = TShellUtil::PickFolder("Select a folder", TEFile::CurrentDir(), EmptyString );
  if( !d.IsEmpty() )  {
    eDir->Text = d.c_str();
    rbFromFolder->Checked = true;
  }
}
//..............................................................................
void __fastcall TdlgIndexInfo::bbCleanClick(TObject *Sender)  {
  int res = Application->MessageBox("Are you sure to clean up the index?", "Question", MB_YESNO|MB_ICONQUESTION);
  if( res != IDYES )
    return;
  if( cbCell->Checked )
    dlgMain->Index->Clean(this);
  if( cbDead->Checked )
    dlgMain->Index->CleanDead(this);
  dlgMain->Index->SaveToFile(dlgMain->IndexFile);
  eCount->Text = dlgMain->Index->GetCount();
}
//..............................................................................
void __fastcall TdlgIndexInfo::bbUpdateClick(TObject *Sender)  {
  if( rbAllDrives->Checked )  {
    dlgMain->Index->Update(true, EmptyString, atoi(eLimit->Text.c_str())*1024*1024, this);
  }
  if( rbFromFolder->Checked )  {
    if( DirectoryExists(eDir->Text) )
      dlgMain->Index->Update(false, eDir->Text.c_str(), atoi(eLimit->Text.c_str())*1024*1024, this);
  }
  if( rbList->Checked )  {
    for( int i=0; i < dlgMain->Paths.Count(); i++ )  {
      if( TEFile::Exists(dlgMain->Paths[i]) )
        dlgMain->Index->Update(false, dlgMain->Paths[i], atoi(eLimit->Text.c_str())*1024*1024, this);
    }
  }
  eCount->Text = dlgMain->Index->GetCount();
  eDate->Text = TETime::FormatDateTime( dlgMain->Index->Updated ).c_str();
  dlgMain->Index->SaveToFile(dlgMain->IndexFile);
}
//..............................................................................
void __fastcall TdlgIndexInfo::bbClearClick(TObject *Sender)  {
  int res = Application->MessageBox("Are you sure to clear the index?", "Question", MB_YESNO|MB_ICONQUESTION);
  if( res == IDYES )  {
    dlgMain->Index->Clear();
    dlgMain->Index->SaveToFile(dlgMain->IndexFile);
  }
  eCount->Text = dlgMain->Index->GetCount();
  eDate->Text = TETime::FormatDateTime( dlgMain->Index->Updated ).c_str();
}
//..............................................................................

