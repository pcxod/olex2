//---------------------------------------------------------------------------

#include <vcl.h>
#include <filectrl.hpp>
#pragma hdrstop

#include "main.h"
#include "sync_dlg.h"

#include "filetree.h"
#include "tptrlist.h"
#include "typelist.h"
#include "bapp.h"
#include "log.h"
#include "outstream.h"
#include "actions.h"
#include "emath.h"
#include "etime.h"
#include "shellutil.h"

#pragma package(smart_init)
#pragma resource "*.dfm"
TfMain *fMain;

  TOnProgress OnExpand, OnFileCopy, OnSync, OnCompare;
  TFileTree* FileTree = NULL;
  uint64_t Start = 0;
  
class ProgressListener : public AActionHandler {
public:
  ProgressListener()  {
    AActionHandler::SetToDelete(false);
  }
  virtual bool Execute(const IEObject *Sender, const IEObject *Data) {
    if( Data == &OnExpand )  {
      fMain->sbSyncStatus->Panels->Items[0]->Text = OnExpand.GetAction().u_str();
    }
    else if( Data == &OnCompare )  {
      fMain->reEdit->Lines->Add( OnCompare.GetAction().u_str() );
    }
    else if( Data == &OnFileCopy )  {
      if( OnFileCopy.GetMax() != 0 )  {  // empty file
        double dv = 100.0/OnFileCopy.GetMax();
        fMain->pbFC->Position = (int)(OnFileCopy.GetPos()*dv);
        fMain->stCurrent->Caption = TEFile::ExtractFileName(OnFileCopy.GetAction()).u_str();
        fMain->sbSyncStatus->Panels->Items[0]->Text = TEFile::ExtractFilePath(OnFileCopy.GetAction()).u_str();
      }
    }
    if( Data == &OnSync || Data == &OnFileCopy )  {
      if( OnSync.GetMax() != 0 )  {
        double dv =  1024*1024*100.0/OnSync.GetMax(), pos = (OnSync.GetPos() + OnFileCopy.GetPos())/(1024*1024);
        fMain->pbOverall->Position = Round(pos*dv);
        if( Data == &OnSync )
          fMain->reEdit->Lines->Add(OnFileCopy.GetAction().u_str());
        if( pos < 1024 )
          fMain->stTotal->Caption = AnsiString::FormatFloat("0.00", pos) + " MByte";
        else
          fMain->stTotal->Caption = AnsiString::FormatFloat("0.00", pos/1024) + " GByte";
        uint64_t now = TETime::msNow();
        if( now > Start )  {
          double speed = (pos*1000)/(double)(now - Start);
          fMain->stSpeed->Caption = AnsiString::FormatFloat("0.00", speed) + " Mb/s";
        }
      }
    }
    Application->ProcessMessages();
    return false;
  }
};


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
  : TForm(Owner)
{
  bapp = new TBasicApp( ParamStr(0).c_str() );
  eSrc->Text = TShellUtil::GetSpecialFolderLocation(fiMyDocuments).u_str();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::sbSrcClick(TObject *Sender)
{
  AnsiString dir, caption = (Sender == sbSrc) ? "Choose source dir" : "Choose destination dir";

  if( SelectDirectory(caption, "", dir) )  {
    if( Sender == sbSrc )
      eSrc->Text = dir;
    else
    eDest->Text = dir;
  }
}
//---------------------------------------------------------------------------
void __fastcall TfMain::bbRunClick(TObject *Sender)
{
  if( bbRun->Caption == "Stop!" )  {
    if( FileTree != NULL )  {
      FileTree->Stop = true;
      bbRun->Caption = "Run...";
    }
    return;
  }
  else
    bbRun->Caption = "Stop!";
  reEdit->Lines->Clear();
  TFileTree ft1(eSrc->Text.c_str()),
            ft2( eDest->Text.c_str() );
  ProgressListener pl;
  ft1.OnExpand->Add( &pl );
  ft1.OnSynchronise->Add( &pl );
  ft1.OnFileCopy->Add( &pl );
  ft1.OnFileCompare->Add( &pl );
  ft1.OnCompare->Add( &pl );
  ft2.OnExpand->Add( &pl );
  FileTree = &ft1;
  ft1.Root.Expand(OnExpand);
  ft2.Root.Expand(OnExpand);
  //OnSync.SetMax( ft1.Root.Compare(ft2.Root, OnCompare) );
  Start = TETime::msNow();
  dlgSync->Init(ft1, ft2);
  if( dlgSync->ShowModal() == mrOk )  {
    OnSync.SetPos(0);
    OnSync.SetMax( dlgSync->TotalSize );
    ft1.DoMerge(*dlgSync->Root, OnSync, OnFileCopy);
  }
//  try  {  ft1.Root.Synchronise(ft2.Root, OnSync, OnFileCopy);  }
//  catch( const TExceptionBase& exc )  {
//    TStrList out;
//    exc.GetException()->GetStackTrace(out);
//    for( int i=0; i < out.Count(); i++ )
//      reEdit->Lines->Add( out[i].u_str() );
//  }

  ft1.OnExpand->Remove( &pl );
  ft1.OnSynchronise->Remove( &pl );
  ft1.OnFileCopy->Remove( &pl );
  ft1.OnFileCompare->Remove( &pl );
  ft1.OnCompare->Remove( &pl );
  ft2.OnExpand->Remove( &pl );
  stCurrent->Caption = "Done";
  pbOverall->Position = 0;
  pbFC->Position = 0;
  FileTree = NULL;
  bbRun->Caption = "Run...";
}
//---------------------------------------------------------------------------

