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

  //TOnProgress OnExpand, OnFileCopy, OnSync, OnCompare;
  TFileTree* FileTree = NULL;
  uint64_t Start = 0;
const int
  actionExpand = 0,
  actionFileCopy = 1,
  actionFileCmp = 2,
  actionSync = 3,
  actionCompare = 4;

class ProgressListener : public AActionHandler {
  int action;
public:
  ProgressListener(int _action) : action(_action) {  }
  virtual bool Execute(const IEObject *Sender, const IEObject *Data) {
    if( Data == NULL || !EsdlInstanceOf(*Data, TOnProgress) )
      return false;
    TOnProgress& pg = *(TOnProgress*)Data;
    if( action == actionExpand )  {
      fMain->sbSyncStatus->Panels->Items[0]->Text = pg.GetAction().u_str();
    }
    else if( action == actionCompare )  {
      fMain->reEdit->Lines->Add( pg.GetAction().u_str() );
    }
    else if( action == actionFileCopy )  {
      if( pg.GetMax() != 0 )  {  // empty file
        double dv = 100.0/pg.GetMax();
        fMain->pbFC->Position = (int)(pg.GetPos()*dv);
        fMain->stCurrent->Caption = TEFile::ExtractFileName(pg.GetAction()).u_str();
        fMain->sbSyncStatus->Panels->Items[0]->Text = TEFile::ExtractFilePath(pg.GetAction()).u_str();
      }
    }
    if( action == actionSync )  {
      if( pg.GetMax() != 0 )  {
        double dv =  1024*1024*100.0/pg.GetMax(), pos = pg.GetPos()/(1024*1024);
        fMain->pbOverall->Position = Round(pos*dv);
        fMain->reEdit->Lines->Add(pg.GetAction().u_str());
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
      FileTree->DoBreak();
      bbRun->Caption = "Run...";
    }
    return;
  }
  else
    bbRun->Caption = "Stop!";
  reEdit->Lines->Clear();
  TFileTree ft1(eSrc->Text.c_str()),
            ft2( eDest->Text.c_str() );
  ft1.OnExpand->Add( new ProgressListener(actionExpand) );
  ft1.OnSynchronise->Add( new ProgressListener(actionSync) );
  ft1.OnFileCopy->Add( new ProgressListener(actionFileCopy) );
  ft1.OnFileCompare->Add( new ProgressListener(actionFileCmp) );
  ft1.OnCompare->Add( new ProgressListener(actionCompare) );
  ft2.OnExpand->Add( new ProgressListener(actionExpand) );
  FileTree = &ft1;
  ft1.Expand();
  ft2.Expand();
  //OnSync.SetMax( ft1.Root.Compare(ft2.Root, OnCompare) );
  Start = TETime::msNow();
  dlgSync->Init(ft1, ft2);
  if( dlgSync->ShowModal() == mrOk )  {
    ft1.Merge(*dlgSync->Root);
  }
//  try  {  ft1.Root.Synchronise(ft2.Root, OnSync, OnFileCopy);  }
//  catch( const TExceptionBase& exc )  {
//    TStrList out;
//    exc.GetException()->GetStackTrace(out);
//    for( int i=0; i < out.Count(); i++ )
//      reEdit->Lines->Add( out[i].u_str() );
//  }

  stCurrent->Caption = "Done";
  pbOverall->Position = 0;
  pbFC->Position = 0;
  FileTree = NULL;
  bbRun->Caption = "Run...";
}
//---------------------------------------------------------------------------

