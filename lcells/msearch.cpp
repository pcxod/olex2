//---------------------------------------------------------------------------

#include <vcl.h>
#include <Clipbrd.hpp>
#pragma hdrstop

#include "main.h"
#include "draw_mol.h"
#include "msearch.h"
#include "conindex.h"
#include "progress.h"

#include "efile.h"
#include "ememstream.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgSearch *dlgSearch;
//---------------------------------------------------------------------------
__fastcall TdlgSearch::TdlgSearch(TComponent* Owner)
  : TForm(Owner)
{
  Organiser = new TOrganiser(iQuery->Picture->Bitmap);
  CIndex = new TConIndex;
  iQuery->Picture->Bitmap->Height = iQuery->Height;
  iQuery->Picture->Bitmap->Width  = iQuery->Width;
  if( TEFile::FileExists(dlgMain->CIndexFile) )
    CIndex->LoadFromFile(dlgMain->CIndexFile);
  else  {
    CIndex->Update(dlgMain->Indexes, *Organiser->XFile);
    CIndex->SaveToFile(dlgMain->CIndexFile);
  }
  stSize->Caption = CIndex->GetCount();
}
__fastcall TdlgSearch::~TdlgSearch()  {
  delete Organiser;
  delete CIndex;
}
//---------------------------------------------------------------------------
void __fastcall TdlgSearch::sbPasteClick(TObject *Sender)  {

  TStrList cbc;
  TEMemoryStream ms;
  ms.Write( Clipboard()->AsText.c_str(), Clipboard()->AsText.Length());
  ms.SetPosition(0);
  cbc.LoadFromTextStream( ms );
  try  {
    Organiser->XFile->FindFormat("mol")->LoadFromStrings(cbc);
  }
  catch( const TExceptionBase& exc )  {
    Application->MessageBox("Nothing to insert (MDL MOL file is expected)!", "Message", MB_OK|MB_ICONWARNING);
  }
  Organiser->Basis.Reset();
  Organiser->CalcZoom();
  Organiser->Draw();
}
//---------------------------------------------------------------------------
void __fastcall TdlgSearch::bbSearchClick(TObject *Sender)  {
  TNet *N = new TNet;
  TPtrList<TConFile> Res;
  N->Assign( Organiser->XFile->GetLattice() );
  CIndex->Search(NULL, N, Res);
  AddResults(Res);
  delete N;
}
//---------------------------------------------------------------------------

void __fastcall TdlgSearch::aPasteExecute(TObject *Sender)  {
  if( ActiveControl == eTitle )
      eTitle->Text = Clipboard()->AsText;
  else
    sbPasteClick(NULL);
}
//---------------------------------------------------------------------------
bool TdlgSearch::LoadMolecule(const olxstr& FN)  {
  try  {
    Organiser->XFile->LoadFromFile( FN );
  }
  catch( const TExceptionBase& exc )  {
    return false;
  }
  return true;
}
//---------------------------------------------------------------------------
void __fastcall TdlgSearch::bbLoadClick(TObject *Sender)  {
  if( !dlgOpen->Execute() )
    return;

  if( !LoadMolecule( dlgOpen->FileName.c_str() ) )  {
    Application->MessageBox("Failed to read file!", "Message", MB_OK|MB_ICONWARNING);
  }
  Organiser->Update();
  Organiser->Draw();
}
//---------------------------------------------------------------------------

void __fastcall TdlgSearch::bbUpdateClick(TObject *Sender)  {
  CIndex->Clear();
  if( TEFile::FileExists(dlgMain->CIndexFile) )
    CIndex->LoadFromFile(dlgMain->CIndexFile);

  CIndex->Update(dlgMain->Indexes, *Organiser->XFile);
  CIndex->SaveToFile(dlgMain->CIndexFile);
  stSize->Caption = CIndex->GetCount();
}
//---------------------------------------------------------------------------

void __fastcall TdlgSearch::bbCloseClick(TObject *Sender)
{
  Hide();
}
//---------------------------------------------------------------------------

void __fastcall TdlgSearch::iQueryMouseDown(TObject *Sender,
    TMouseButton Button, TShiftState Shift, int X, int Y)
{
  Organiser->OnMouseDown(Sender, Button, Shift, X, Y);
}
//---------------------------------------------------------------------------

void __fastcall TdlgSearch::iQueryMouseMove(TObject *Sender,
    TShiftState Shift, int X, int Y)
{
  Organiser->OnMouseMove(Sender, Shift, X, Y);
}
//---------------------------------------------------------------------------

void __fastcall TdlgSearch::iQueryMouseUp(TObject *Sender,
    TMouseButton Button, TShiftState Shift, int X, int Y)
{
  Organiser->OnMouseUp(Sender, Button, Shift, X, Y);
}
//---------------------------------------------------------------------------

void __fastcall TdlgSearch::iQueryClick(TObject *Sender)
{
  Organiser->OnClick(Sender);
}
//---------------------------------------------------------------------------

void _fastcall TdlgSearch::AddResults(const TPtrList<TConFile>& Res)  {
  dlgMain->lvList->Items->BeginUpdate();
  dlgMain->lvList->Clear();
  dlgMain->lvList->Items->EndUpdate();
  bool found, Cancel = false;
  TCifFile *CifFile;
  TZipFile *ZipFile;
  TdlgProgress *dlgProg = new TdlgProgress(NULL);
  dlgProg->OnCancel = &Cancel;
//  dlgProg->AddForm(dlgSearch);  //the form is not defined
  dlgProg->Show();
  dlgProg->Caption = "Loading Index File...";

  for( int i=0; i < dlgMain->Indexes.Count(); i++ )  {
    if( TEFile::FileExists(dlgMain->Indexes.String(i)) )  {
      dlgProg->SetAction(dlgMain->Indexes.String(i));
      dlgMain->Index->LoadFromFile(dlgMain->Indexes.String(i), true);
    }
  }

  dlgMain->lvList->Items->BeginUpdate();
  dlgProg->Caption = "Creating Results List...";
  dlgProg->pbBar->Max = Res.Count();
  for( int i=0; i < Res.Count(); i++ )  {
    if( !(i%2) )  {
      dlgProg->SetAction(Res[i]->FileName);
      dlgProg->pbBar->Position = i;
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    found = false;
    if( !Res[i]->Parent )  {
      for( int j=0; j < dlgMain->Index->IFiles.Count(); j++ )  {
        CifFile = dlgMain->Index->IFiles[j];
        if( CifFile->Name == Res[i]->FileName )  {
          dlgMain->AddResult(CifFile);
          found = true;
          break;
        }
      }
    }
    else  {
      for( int j=0; j < dlgMain->Index->ZFiles.Count(); j++ )  {
        ZipFile = dlgMain->Index->ZFiles[j];
        if( ZipFile->FileName == Res[i]->Parent->FileName )  {
          for( int k=0; k < ZipFile->Index->IFiles.Count(); k++ )  {
            CifFile = ZipFile->Index->IFiles[k];
            if( CifFile->Name == Res[i]->FileName )  {
              dlgMain->AddResult(CifFile);
              found = true;
              break;
            }
          }
          break;
        }
      }
    }
    if( !found )
      dlgMain->AddDummy(Res[i]);
  }
exit:
  delete dlgProg;
  dlgMain->lvList->Items->EndUpdate();
}
void __fastcall TdlgSearch::bbSearchTitleClick(TObject *Sender)  {
  dlgMain->lvList->Items->BeginUpdate();
  dlgMain->lvList->Clear();
  dlgMain->lvList->Items->EndUpdate();
  TPtrList<TConFile> Res;
  short what = 0;
  AnsiString txt;
  if( Sender == bbSearchTitle )  {
    what = ssTitle;
    txt = eTitle->Text;
  }
  else if( Sender == bbSearchSG )  {
    what = ssSG;
    txt = eSG->Text;
  }
  else if( Sender == bbSearchIns )  {
    what = ssIns;
    txt = eIns->Text;
  }
  if( !txt.Length() )  {
    Application->MessageBox("Please, enter some text to search!", "Warning", MB_OK|MB_ICONWARNING);
    return;
  }
  CIndex->Search( what, txt.c_str(), Res);
  AddResults(Res);
}
//---------------------------------------------------------------------------



