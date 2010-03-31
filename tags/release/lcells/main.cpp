//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <math.h>
#include <filectrl.hpp>
#include <dir.h>
#include <registry.hpp>
#pragma hdrstop

#include "msearch.h"
#include "moldraw.h"
#include "preferences.h"
#include "evpoint.h"
//#include "ematrix.h"
#include "iinfo.h"
#include "about.h"
#include "main.h"
#include "conindex.h"

#include "bapp.h"
#include "efile.h"
#include "symmlib.h"
#include "egc.h"
#include "shellutil.h"
#include "typelist.h"
#include "ememstream.h"
#include "estlist.h"
//---------------------------------------------------------------------------
#define  ID_EXCEPTION         1002
#define  ID_ERROR             1003
#pragma package(smart_init)
#pragma resource "*.dfm"

#define qrt(a)  ((a)*(a))

typedef TSStrPObjList<olxstr, TTreeNode*, true > TreeNode;
TreeNode TheTreeRoot;

void AddNode( TTreeNodes* tree, const olxstr& path )  {
  TStrList toks(path, '\\');
  size_t i=0, ind;
  TTreeNode* nd = NULL;
  TreeNode* theNode = &TheTreeRoot;
  while( i < toks.Count() && (ind = theNode->IndexOf(toks[i])) != InvalidIndex )  {
    i++;
    nd = theNode->GetObject(ind);
    theNode = ((TreeNode*)nd->Data);
  }
  for( size_t j=i; j < toks.Count(); j++ )  {
    nd = tree->AddChild(nd, toks[j].c_str() );
    theNode->Add(toks[j],nd);
    if( (j+1) < toks.Count() )  {
      theNode = new TreeNode;
      nd->Data = theNode;
    }
    else  {
      nd->Data = new olxstr( path );
    }
  }
}

bool LogListener::Dispatch( int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data)  {
  if( Data == NULL )  return true;  
  switch( MsgId )  {
    case ID_ERROR:
      form->AddMessage( Data->ToString().u_str() );
      break;
    case ID_EXCEPTION:
      form->AddMessage( Data->ToString().u_str() );
      break;
  }
  return true;
}


olxstr IniHdr = "CifsIni";

short IniVer = 1;
double ProgVer = 1.3;
TdlgMain *dlgMain;
__fastcall TdlgMain::TdlgMain(TComponent* Owner)
  : TForm(Owner)
{

  new TBasicApp( TEFile::ExtractFilePath(ParamStr(0).c_str()));
  TEGC::NewG<TSymmLib>( TBasicApp::GetBaseDir() + "symmlib.xld" );
  LogListener& logListener = TEGC::NewG<LogListener>( this );
  TBasicApp::GetLog().OnException.Add(&logListener, ID_EXCEPTION);
  TBasicApp::GetLog().OnError.Add(&logListener, ID_ERROR);


  DecimalSeparator = '.';
  Bitmap = new Graphics::TBitmap;
  Organiser = new TOrganiser(Bitmap);

  TRegistry *Reg = new TRegistry;
  olxstr Path = TEFile::ExtractFilePath( ParamStr(0).c_str() ),
                Key = "\\Software\\OVD\\";
  Key <<  "LCELLS";
  try  {
  // check if olex2 is installed
    Reg->RootKey = HKEY_CLASSES_ROOT;
    AnsiString cmdKey = "Applications\\olex2.dll\\shell\\open\\command";
    if( Reg->OpenKey(cmdKey, false) )  {
      Olex2Path = Reg->ReadString("").c_str();
      if( Olex2Path.Length() && Olex2Path[0] == '\"' )  {
        int lind = Olex2Path.LastIndexOf('"');
        if( lind > 0 )  {
          Olex2Path = TEFile::ExtractFilePath(Olex2Path.SubString(1, lind-1));
          TEFile::AddPathDelimeterI(Olex2Path);
        }
      }
      sbOlex2->Visible = TEFile::Exists(Olex2Path+"olex2.dll");
      Reg->CloseKey();
    }
    Reg->RootKey = HKEY_CURRENT_USER;
    if (Reg->OpenKey(Key.c_str(), true))  {
      Reg->WriteString("PATH", Path.c_str());
      Reg->WriteFloat("VERSION", ProgVer);
      Reg->CloseKey();
    }
  }
  catch(...)
  {
    ;
  }
exit:
  delete Reg;

  char * sxtl = getenv("SXTL");
  olxstr XPDir;
  if( sxtl != NULL )
    XPDir = sxtl;
  if( XPDir.IsEmpty() )  {
    olxstr xppath = TEFile::Which("xp.exe");
    sbView->Visible = !xppath.IsEmpty();
  }
  else  {
    sbView->Visible = true;
    AddPath( TEFile::ExtractFilePath(XPDir) );
  }
  
  CurrentDir = TEFile::ExtractFilePath(ParamStr(0).c_str() );
  TEFile::AddPathDelimeterI(CurrentDir);

  TmpDir = TShellUtil::GetSpecialFolderLocation(fiAppData);
  TmpDir << "lcells/";
  if( !TEFile::Exists( TmpDir) && !TEFile::MakeDir(TmpDir) )
      Application->MessageBox("Cannot create the tmp folder!", "Error", MB_OK);

  Zip = new TZipShell;
  Zip->TmpPath = TmpDir;
  IndexFile = CurrentDir + "index.dat";
  CIndexFile = CurrentDir + "cindex.dat";
  IniFile = CurrentDir + "cifs.ini";
  Index = new TCifIndex();
  if( TEFile::Exists(IniFile) )  {
    TEFile inf( IniFile, "rb");
    TEMemoryStream S(inf);
    if( S.GetSize() != 0 )  {
      short Ver;
      olxstr Hdr;
      S >> Hdr;
      if( Hdr != IniHdr )
        return;
      S >> Ver;
      S >> IndexFiles;
      S >> UpdatePaths;
    }
  }

  if( IndexFiles.IndexOf(IndexFile) == -1 )
    IndexFiles.Add(IndexFile);

  if( ParamCount() == 1 )  {
    olxstr Tmp = ParamStr(1).LowerCase().c_str();
    bool res;
    if( Tmp == "/update" )  {
      if( TEFile::Exists(IndexFile) )
        Index->LoadFromFile(IndexFile, false);
      for( size_t i=0; i < UpdatePaths.Count(); i++ )  {
        if( TEFile::Exists(UpdatePaths[i]) )
          res = Index->Update(false, UpdatePaths[i], 50*1024*1024, this);
      }
      Index->SaveToFile(dlgMain->IndexFile);
      if( res )
        Application->MessageBox("CMD update is complete and the index is saved.","LCELLS", MB_OK|MB_ICONINFORMATION);
      else
        Application->MessageBox("CMD update is not complete.","LCELLS", MB_OK|MB_ICONINFORMATION);
    }
  }
  if( ParamCount() == 2 ) // searhc a file or a title
  {
    olxstr Tmp = ParamStr(1).LowerCase().c_str();
    olxstr FN = ParamStr(2).c_str();
    if( Tmp == "/searchfile" )  {
      if( !TEFile::Exists(FN) )  {
        FN = CurrentDir + FN;
        if( !TEFile::Exists(FN) )  {
          Application->MessageBox("Search file does not exist...","LCELLS", MB_OK|MB_ICONERROR);
          Application->Terminate();
          return;
        }
      }
      if( dlgSearch == NULL )  dlgSearch = new TdlgSearch(this);
      if( !dlgSearch->LoadMolecule(FN) )  {
        Application->MessageBox("Could not load MDL MOL file...","LCELLS", MB_OK|MB_ICONERROR);
        Application->Terminate();
        return;
      }
      dlgSearch->bbSearchClick(NULL);
      TStrList List;
      TCifFile *C;
      for( int i=0; i < lvList->Items->Count; i++ )  {
        if( lvList->Items->Item[i]->Data )  {
          C = (TCifFile*)lvList->Items->Item[i]->Data;
          if( C->Parent )
            List.Add(C->Parent->GetCif(C->Name));
          else
            List.Add(C->Name);
        }
      }
      List.SaveToFile(CurrentDir + "lcells.out");
      delete dlgSearch;
      dlgSearch = NULL;
      Application->Terminate();
    }
    if( Tmp == "/searchtitle" )  {
      if( !dlgSearch )  dlgSearch = new TdlgSearch(this);
      dlgSearch->eTitle->Text = FN.c_str();
      dlgSearch->bbSearchTitleClick(NULL);
      TStrList List;
      TCifFile *C;
      for( int i=0; i < lvList->Items->Count; i++ )  {
        if( lvList->Items->Item[i]->Data )  {
          C = (TCifFile*)lvList->Items->Item[i]->Data;
          if( C->Parent )
            List.Add(C->Parent->GetCif(C->Name));
          else
            List.Add(C->Name);
        }
      }
      List.SaveToFile(CurrentDir + "lcells.out");
      delete dlgSearch;
      dlgSearch = NULL;
      Application->Terminate();
    }
  }
  if( ParamCount() == 8 )  {  // searhc a cell
    olxstr Tmp = ParamStr(1).LowerCase().c_str();
    eA->Text = ParamStr(2);
    eB->Text = ParamStr(3);
    eC->Text = ParamStr(4);
    eAA->Text = ParamStr(5);
    eAB->Text = ParamStr(6);
    eAC->Text = ParamStr(7);
    eDev->Text = ParamStr(8);
    if( Tmp == "/searchcell" )  {
      bbSearchClick(NULL);
      TStrList List;
      TCifFile *C;
      for( int i=0; i < lvList->Items->Count; i++ )  {
        if( lvList->Items->Item[i]->Data )  {
          C = (TCifFile*)lvList->Items->Item[i]->Data;
          if( C->Parent )
            List.Add(C->Parent->GetCif(C->Name));
          else
            List.Add(C->Name);
        }
      }
      List.SaveToFile(CurrentDir + "lcells.out");
      Application->Terminate();
    }
  }
  InitTree();
}
__fastcall TdlgMain::~TdlgMain()
{
//  Index->SaveToFile(IndexFile); multi index files. so we dont know which is the one
  ClearTree();
  delete Index;
  Index = NULL;
  FreeConsole();
  delete Zip;
  if( dlgSearch )  {
    delete dlgSearch;
    dlgSearch = NULL;
  }
  delete Organiser;
  Organiser = NULL;
  delete Bitmap;
  delete &TBasicApp::GetInstance();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TdlgMain::bbSearchClick(TObject *Sender)  {
  if( !eA->Text.Length() || !eB->Text.Length() || !eC->Text.Length() ||
    !eAA->Text.Length() || !eAB->Text.Length() || !eAC->Text.Length() )  {

    Application->MessageBox("Please, fill all the fields to perform search...",
      "Error", MB_OK);
    return;
  }
  int i = cbLattice->Items->IndexOf(cbLattice->Text);
  if( i < 0 )  return;
  i++;
  TCell C;
  TTypeList<TCifFile*> Cifs;
  double Dev = atof(eDev->Text.c_str());
  TCifFile *info;
  olxstr Tmp;
  C.a = atof(eA->Text.c_str());
  C.b = atof(eB->Text.c_str());
  C.c = atof(eC->Text.c_str());
  C.aa = atof(eAA->Text.c_str());
  C.ab = atof(eAB->Text.c_str());
  C.ac = atof(eAC->Text.c_str());
  C.Lattice = i;
  ReduceCell(C);
  Index->Clear();
  for( size_t i=0; i < IndexFiles.Count(); i++ )  {
    if( TEFile::Exists(IndexFiles[i]) )  {
      Index->LoadFromFile(IndexFiles[i], true);
      Index->Search(C, Dev, Cifs, false);
    }
    else  {
      Tmp = "Cannot find the index file: ";
      Tmp << IndexFiles[i];
      Application->MessageBox(Tmp.c_str(), "Error", MB_OK|MB_ICONERROR);
    }
  }
  lvList->Items->BeginUpdate();
  lvList->Items->Clear();
  for( size_t i=0; i < Cifs.Count(); i++ )
    AddResult(Cifs[i]);

  lvList->Items->EndUpdate();
}
void _fastcall TdlgMain::AddDummy(TConFile *info)
{
  TListItem *I = lvList->Items->Add();
  olxstr Tmp;
  I->Data = NULL;
  I->Caption = lvList->Items->Count;
  if( info->Parent )
  {
    Tmp = info->FileName;
    Tmp << '{' << info->Parent->FileName << '}';
    I->SubItems->Add( Tmp.c_str() );
  }
  else
    I->SubItems->Add( info->FileName.c_str() );
}
void _fastcall TdlgMain::AddResult(TCifFile *info)
{
  TListItem *I = lvList->Items->Add();
  olxstr Tmp;
  I->Data = info;
  I->Caption = lvList->Items->Count;
  if( info->Parent )
  {
    Tmp = info->Name;
    Tmp << '{' << info->Parent->FileName << '}';
    I->SubItems->Add( Tmp.c_str() );
  }
  else
    I->SubItems->Add( info->Name.c_str() );
}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::lvListSelectItem(TObject *Sender,
    TListItem *Item, bool Selected)
{
  olxstr Tmp, Tmp1;
    if( !Item->Data )  return;
  if( Selected )
  {
    TCifFile *C = (TCifFile*)Item->Data;
    Tmp = "Cell: ";
    Tmp << "a=";
    Tmp << olxstr::FormatFloat(4, C->Cell.a);
    Tmp << ", b=";
    Tmp << olxstr::FormatFloat(4, C->Cell.b);
    Tmp << ", c=";
    Tmp << olxstr::FormatFloat(4, C->Cell.c);
    Tmp << "; alpha=";
    Tmp << olxstr::FormatFloat(4, C->Cell.aa);
    Tmp << ", beta=";
    Tmp << olxstr::FormatFloat(4, C->Cell.ab);
    Tmp << ", gamma=";
    Tmp << olxstr::FormatFloat(4, C->Cell.ac);
    Tmp << ';';
    lFound->Caption = Tmp.c_str();

    Tmp = "Niggli Cell: ";
    Tmp << "a=";
    Tmp << olxstr::FormatFloat(4, C->Cell.na);
    Tmp << ", b=";
    Tmp << olxstr::FormatFloat(4, C->Cell.nb);
    Tmp << ", c=";
    Tmp << olxstr::FormatFloat(4, C->Cell.nc);
    Tmp << "; alpha=";
    Tmp << olxstr::FormatFloat(4, C->Cell.naa);
    Tmp << ", beta=";
    Tmp << olxstr::FormatFloat(4, C->Cell.nab);
    Tmp << ", gamma=";
    Tmp << olxstr::FormatFloat(4, C->Cell.nac);
    Tmp << ';';
    lNiggli->Caption = Tmp.c_str();
  }
  else  {
    if( mMemo->Lines->Count ) {
      mMemo->Lines->Clear();
      sbView->Enabled = false;
      sbOlex2->Enabled = false;
    }
  }
}
void _fastcall TdlgMain::ClearMessages()
{
  mMemo->Lines->Clear();
}
void _fastcall TdlgMain::AddMessage(const olxstr& A)  {
  mMemo->Lines->Add(A.c_str());
}
//---------------------------------------------------------------------------
void _fastcall TdlgMain::AddMessage(const TStrList& sl)  {
  for( size_t i=0; i < sl.Count(); i++ )
    mMemo->Lines->Add( sl[i].c_str() );
}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::miAboutClick(TObject *Sender)
{
  dlgAbout = new TdlgAbout(this);
  dlgAbout->ShowModal();
  delete dlgAbout;
}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::miInfoClick(TObject *Sender)
{
  Index->Clear();
  Index->LoadFromFile(IndexFile, false);
  dlgIndexInfo = new TdlgIndexInfo(this);
  dlgIndexInfo->ShowModal();
  delete dlgIndexInfo;
  lvList->Items->BeginUpdate();
  lvList->Items->Clear();
  lvList->Items->EndUpdate();
  sbView->Enabled = false;
  sbOlex2->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::lvListDblClick(TObject *Sender)
{
  sbView->Enabled = false;
  sbOlex2->Enabled = false;
  if( lvList->Selected )  {
    if( lvList->Selected->Data )  {
      TCifFile *C = (TCifFile*)lvList->Selected->Data;
      if( C->Parent )
        CurrentFile = C->Parent->GetCif(C->Name);
      else
        CurrentFile = C->Name;
      LoadCurrentFile();
    }
    else  {
      if( !dlgSearch )  {
        Application->MessageBox("Cannot find the file... Use 'Index Cleanup' to avoid this message.", "Error", MB_OK|MB_ICONERROR);
      }
      else  {
       /* try
        {
          TConFile *CF = dlgSearch->CIndex->GetRecord(lvList->Selected->SubItems->Strings[0]);
          if( !CF )  return;
          sbView->Enabled = true;
          Organiser->Clear();
          TMolecule *M = Organiser->AddMolecule();
          M->LoadNetwork(CF->Net);
          if( !dlgMolDraw->Visible )
            dlgMolDraw->Visible = true;
          Organiser->MoleculeAdded(M); // do zoom and mpln calculations
          Organiser->Draw();
        }
        catch(...)
        {
          Application->MessageBox("Cannot load the file for viewing...", "Error", MB_OK|MB_ICONERROR);
        }*/
      }
    }
  }
}
void _fastcall TdlgMain::DeleteDir(const olxstr& Dir, TStrList *SubDirs)
{

}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::sbSaveAsClick(TObject *Sender)  {
  dlgSave->FileName = TEFile::ExtractFileName(CurrentFile).c_str();
  if( dlgSave->Execute() )
    mMemo->Lines->SaveToFile(dlgSave->FileName);
}
//---------------------------------------------------------------------------

bool _fastcall TdlgMain::ReduceCell(TCell &C)  {
  vec3d X,Y,Z;
  int R[6];
  double Rd[6];
  double val, TX, TY, TZ, AX, AY, AZ, cosa, sina;

  bool Used;
  int Cycles;
  cosa = cos(C.ac/180.0*M_PI);
  sina = sin(C.ac/180.0*M_PI);

  TX = cos(C.ab/180.0*M_PI);
  TY = (cos(C.aa*M_PI/180.0) - cosa*cos(C.ab*M_PI/180.0))/sina;
  TZ = sqrt( 1.0 - TX*TX-TY*TY);

  switch(C.Lattice)
  {
    case 1:  // P
      X[0] = 1;    X[1] = 0;    X[2] = 0;
      Y[0] = 0;    Y[1] = 1;    Y[2] = 0;
      Z[0] = 0;    Z[1] = 0;    Z[2] = 1;
      break;
    case 2:  // I
      X[0] = 1;    X[1] = 0;    X[2] = 0;
      Y[0] = 0;    Y[1] = 1;    Y[2] = 0;
      Z[0] = 1./2;  Z[1] = 1./2;  Z[2] = 1./2;
      break;
    case 3:  // R
      X[0] = 2./3;  X[1] = 1./3;  X[2] = 1./3;
      Y[0] = -1./3;  Y[1] = 1./3;  Y[2] = 1./3;
      Z[0] = -1./3;  Z[1] = -2./3;  Z[2] = 1./3;
      break;
    case 4:  // F
      X[0] = 1./2;  X[1] = 0;    X[2] = 1./2;
      Y[0] = 1./2;  Y[1] = 1./2;  Y[2] = 0;
      Z[0] = 0;    Z[1] = 1./2;  Z[2] = 1./2;
      break;
    case 5:  // A
      X[0] = 1;    X[1] = 0;    X[2] = 0;
      Y[0] = 0;    Y[1] = 1;    Y[2] = 0;
      Z[0] = 0;    Z[1] = 1./2;  Z[2] = 1./2;
      break;
    case 6:  // B
      X[0] = 1./2;  X[1] = 0;    X[2] = 1./2;
      Y[0] = 0;    Y[1] = 1;    Y[2] = 0;
      Z[0] = 0;    Z[1] = 0;    Z[2] = 1;
      break;
    case 7:  // C
      X[0] = 1;    X[1] = 0;    X[2] = 0;
      Y[0] = 1./2;  Y[1] = 1./2;  Y[2] = 0;
      Z[0] = 0;    Z[1] = 0;    Z[2] = 1;
      break;
    default:
      Application->MessageBox("Cannot reduce the cell: unknown centering...", "Error", MB_OK|MB_ICONERROR);
      return false;
  }

  AX = X[0]*C.a;  AY = X[1]*C.b;  AZ = X[2]*C.c;
  X[0] = AX + AY*cosa+ AZ*TX;
  X[1] = AY*sina + AZ*TY;
  X[2] = AZ*TZ;

  AX = Y[0]*C.a;  AY = Y[1]*C.b;  AZ = Y[2]*C.c;
  Y[0] = AX + AY*cosa + AZ*TX;
  Y[1] = AY*sina + AZ*TY;
  Y[2] = AZ*TZ;

  AX = Z[0]*C.a;  AY = Z[1]*C.b;  AZ = Z[2]*C.c;
  Z[0] = AX + AY*cosa+ AZ*TX;
  Z[1] = AY*sina + AZ*TY;
  Z[2] = AZ*TZ;

  C.na = X.Length();
  C.nb = Y.Length();
  C.nc = Z.Length();

  C.naa = acos(Y.CAngle(Z))*180/M_PI;
  C.nab = acos(X.CAngle(Z))*180/M_PI;
  C.nac = acos(Y.CAngle(X))*180/M_PI;

  Rd[0] = qrt(C.na);
  Rd[1] = qrt(C.nb);
  Rd[2] = qrt(C.nc);

  Rd[3] = 2*C.nb*C.nc*cos(C.naa*M_PI/180);
  Rd[4] = 2*C.na*C.nc*cos(C.nab*M_PI/180);
  Rd[5] = 2*C.na*C.nb*cos(C.nac*M_PI/180);

  for( int i=0; i < 6; i++ )
    R[i] = olx_round(Rd[i]);

  Used = true;
  Cycles = 0;
  while ( Used )
  {
    Used = false;
    Cycles ++;
    if( Cycles > 100 )
    {
      Application->MessageBox("Cannot evaluate the niggli cell...", "Error", MB_OK|MB_ICONERROR);
      return false;
    }
    if( (R[0] > R[1]) || ((R[0] == R[1])&&(fabs(R[3])> fabs(R[4]))) ) //1
    {
      val = R[0];      R[0] = R[1];      R[1] = val;
      val = R[3];      R[3] = R[4];      R[4] = val;

      val = Rd[0];    Rd[0] = Rd[1];      Rd[1] = val;
      val = Rd[3];    Rd[3] = Rd[4];      Rd[4] = val;
      Used = true;
    }
    if( (R[1] > R[2]) || ( (R[1] == R[2])&&(fabs(R[4])> fabs(R[5]))) )//2
    {
      val = R[1];      R[1] = R[2];      R[2] = val;
      val = R[4];      R[4] = R[5];      R[5] = val;

      val = Rd[1];    Rd[1] = Rd[2];      Rd[2] = val;
      val = Rd[4];    Rd[4] = Rd[5];      Rd[5] = val;
      Used = true;
      continue;
    }
    if( R[3]*R[4]*R[5] > 0 ) //3
    {
      R[3] = fabs(R[3]);      R[4] = fabs(R[4]);      R[5] = fabs(R[5]);

      Rd[3] = fabs(Rd[3]);    Rd[4] = fabs(Rd[4]);     Rd[5] = fabs(Rd[5]);
    }
    else
    {
      R[3] = -fabs(R[3]);      R[4] = -fabs(R[4]);      R[5] = -fabs(R[5]);

      Rd[3] = -fabs(Rd[3]);    Rd[4] = -fabs(Rd[4]);     Rd[5] = -fabs(Rd[5]);
    }
    if( (fabs(R[3]) > R[1]) || ( (R[3] == R[1])&& (2*R[4]<R[5]) ) || //5
      ( (R[3]==-R[1])&&(R[5]<0)) )
    {
      R[2] = R[1]+R[2] - R[3]*olx_sign(R[3]);
      R[4] = R[4]-R[5]*olx_sign(R[3]);
      R[3] = R[3]-2*R[1]*olx_sign(R[3]);

      Rd[2] = Rd[1]+Rd[2] - Rd[3]*olx_sign(Rd[3]);
      Rd[4] = Rd[4]-Rd[5]*olx_sign(Rd[3]);
      Rd[3] = Rd[3]-2*Rd[1]*olx_sign(Rd[3]);
      Used = true;
      continue;
    }
    if( (fabs(R[4]) > R[0]) || ( (R[4] == R[0])&& (2*R[3]<R[5]) ) ||  //6
      ( (R[4]==-R[0])&&(R[5]<0)) )
    {
      R[2] = R[0]+R[2] - R[4]*olx_sign(R[4]);
      R[3] = R[3]-R[5]*olx_sign(R[4]);
      R[4] = R[4]-2*R[0]*olx_sign(R[4]);

      Rd[2] = Rd[0]+Rd[2] - Rd[4]*olx_sign(Rd[4]);
      Rd[3] = Rd[3]-Rd[5]*olx_sign(Rd[4]);
      Rd[4] = Rd[4]-2*Rd[0]*olx_sign(Rd[4]);
      Used = true;
      continue;
    }
    if( (fabs(R[5]) > R[0]) || ( (R[5] == R[0])&& (2*R[3]<R[4]) ) ||  //7
      ( (R[5]==-R[0])&&(R[4]<0)) )
    {
      R[1] = R[0]+R[1] - R[5]*olx_sign(R[5]);
      R[3] = R[3]-R[4]*olx_sign(R[5]);
      R[5] = R[5]-2*R[0]*olx_sign(R[5]);

      Rd[1] = Rd[0]+Rd[1] - Rd[5]*olx_sign(Rd[5]);
      Rd[3] = Rd[3]-Rd[4]*olx_sign(Rd[5]);
      Rd[5] = Rd[5]-2*Rd[0]*olx_sign(Rd[5]);
      Used = true;
      continue;
    }
    if( ((R[0]+R[1] + R[3]+R[4]+R[5])< 0 ) ||
      ( ((R[0]+R[1] + R[3]+R[4]+R[5])==0) && (2*(R[0]+R[4])+R[5])>0 )) //8
    {
      R[2] = R[0]+R[1]+R[2]+R[3]+R[4]+R[5];
      R[3] = 2*R[1] + R[3]+R[5];
      R[4] = 2*R[0] + R[4]+R[5];

      Rd[2] = Rd[0]+Rd[1]+Rd[2]+Rd[3]+Rd[4]+Rd[5];
      Rd[3] = 2*Rd[1] + Rd[3]+Rd[5];
      Rd[4] = 2*Rd[0] + Rd[4]+Rd[5];
      Used = true;
      continue;
    }
  }
//  S = S * M;
//  A = A * M;

  Rd[0] = sqrt(Rd[0]);
  Rd[1] = sqrt(Rd[1]);
  Rd[2] = sqrt(Rd[2]);
  C.na = Rd[0];
  C.nb = Rd[1];
  C.nc = Rd[2];
  C.naa = acos(Rd[3]/(Rd[1]*Rd[2]*2))*180/M_PI;
  C.nab = acos(Rd[4]/(Rd[0]*Rd[2]*2))*180/M_PI;
  C.nac = acos(Rd[5]/(Rd[0]*Rd[1]*2))*180/M_PI;
  return true;
}

void __fastcall TdlgMain::sbUpdateCellClick(TObject *Sender)  {
  if( !eA->Text.Length() || !eB->Text.Length() || !eC->Text.Length() ||
    !eAA->Text.Length() || !eAB->Text.Length() || !eAC->Text.Length() )
  {
    return;
  }
  int i = cbLattice->Items->IndexOf(cbLattice->Text);
  if( i < 0 )
    return;
  i++;
  TCell C;
  olxstr Tmp, Tmp1;
  int F = 4;
  C.a = atof(eA->Text.c_str());
  C.b = atof(eB->Text.c_str());
  C.c = atof(eC->Text.c_str());
  C.aa = atof(eAA->Text.c_str());
  C.ab = atof(eAB->Text.c_str());
  C.ac = atof(eAC->Text.c_str());
  C.Lattice = i;
  if( ReduceCell(C) )  {
    Tmp = "Cell: ";
    Tmp << "a=";
    Tmp << olxstr::FormatFloat(F, C.a);
    Tmp << ", b=";
    Tmp << olxstr::FormatFloat(F, C.b);
    Tmp << ", c=";
    Tmp << olxstr::FormatFloat(F, C.c);
    Tmp << "; alpha=";
    Tmp << olxstr::FormatFloat(F, C.aa);
    Tmp << ", beta=";
    Tmp << olxstr::FormatFloat(F, C.ab);
    Tmp << ", gamma=";
    Tmp << olxstr::FormatFloat(F, C.ac);
    Tmp << ';';
    lFound->Caption = Tmp.c_str();

    Tmp = "Niggli Cell: ";
    Tmp << "a=";
    Tmp << olxstr::FormatFloat(F, C.na);
    Tmp << ", b=";
    Tmp << olxstr::FormatFloat(F, C.nb);
    Tmp << ", c=";
    Tmp << olxstr::FormatFloat(F, C.nc);
    Tmp << "; alpha=";
    Tmp << olxstr::FormatFloat(F, C.naa);
    Tmp << ", beta=";
    Tmp << olxstr::FormatFloat(F, C.nab);
    Tmp << ", gamma=";
    Tmp << olxstr::FormatFloat(F, C.nac);
    Tmp << ';';
    lNiggli->Caption = Tmp.c_str();
  }
}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::miPreferencesClick(TObject *Sender)
{
  dlgPref = new TdlgPref(this);
  TListItem *I;

  for( size_t i=0; i < UpdatePaths.Count(); i++ )  {
     I = dlgPref->lvPaths->Items->Add();
     I->Caption = UpdatePaths[i].c_str();
  }
  for( size_t i=0; i < IndexFiles.Count(); i++ )  {
     I = dlgPref->lvFiles->Items->Add();
     I->Caption = IndexFiles[i].c_str();
  }
  if( dlgPref->ShowModal() == mrOk )  {
    IndexFiles.Clear();
    UpdatePaths.Clear();
    for(int i=0; i < dlgPref->lvPaths->Items->Count; i++ )  {
       I = dlgPref->lvPaths->Items->Item[i];
       UpdatePaths.Add( I->Caption.c_str() );
    }
    for(int i=0; i < dlgPref->lvFiles->Items->Count; i++ )  {
       I = dlgPref->lvFiles->Items->Item[i];
       IndexFiles.Add( I->Caption.c_str() );
    }
  }
  if( IndexFiles.IndexOf(IndexFile) == -1 )
    IndexFiles.Add(IndexFile);

  TEMemoryStream S;
  S << IniHdr;
  S << IniVer;
  S << IndexFiles;
  S << UpdatePaths;
  S.SaveToFile(IniFile);
  delete dlgPref;
}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::miExitClick(TObject *Sender)
{
  Application->Terminate();
}
//---------------------------------------------------------------------------
void _fastcall TdlgMain::PostMsg(HWND H, const olxstr& Msg)  {
  for( size_t i=0; i < Msg.Length(); i++ )
    PostMessage(H, WM_CHAR, Msg[i], 0);
  }
void __fastcall TdlgMain::sbViewClick(TObject *Sender)  {
  olxstr Tmp;
  Tmp = "xp ";
  Tmp << TEFile::ExtractFileName(CurrentFile);
  olxstr CD = TEFile::ExtractFilePath(CurrentFile);
  Tmp.c_str();  // make sure the raw str is zero ended
  CD.c_str();   //
//  unsigned long Status;
  STARTUPINFO SI;
  SI.cb = sizeof(STARTUPINFO);
  SI.lpDesktop = NULL;
  SI.lpTitle = NULL;
  SI.dwX = SI.dwY = 0;
  SI.dwXSize = 80;
  SI.dwYSize = 25;
  SI.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
  SI.cbReserved2 = 0;
  SI.lpReserved2 = NULL;
//  SI.hStdInput = Console;
  PROCESS_INFORMATION PI;
  if(    !CreateProcess(
      NULL,  // pointer to name of executable module
      Tmp.raw_str(),  // pointer to command line string
      NULL,  // pointer to process security attributes
      NULL,  // pointer to thread security attributes
      TRUE,  // handle inheritance flag
      CREATE_DEFAULT_ERROR_MODE, //DETACHED_PROCESS,
      NULL,  // pointer to new environment block
      CD.raw_str(),
      &SI,
      &PI   )
    )
  {
    Application->MessageBox("Cannot launch XP...", "Error", MB_OK|MB_ICONERROR);
  }
  else  {
    HWND XpWindow;
    XpWindow = FindWindow("XRAYFWClass", NULL);
    int count = 0;
    while( !XpWindow )  {
      Sleep(100);
      Application->ProcessMessages();
      XpWindow = FindWindow("XRAYFWClass", NULL);
      count ++;
      if( count == 20 )  // 20 secs
        break;
    }
    if( XpWindow )  {
      PostMsg(XpWindow, "fmol/n\n");
      PostMsg(XpWindow, "uniq\n");
      PostMsg(XpWindow, "mpln/n\n");
      PostMsg(XpWindow, "proj\n");
      ShowWindow(XpWindow, SW_MAXIMIZE);
    }
/*    GetExitCodeProcess(PI.hProcess, &Status);
    while ( Status == STILL_ACTIVE )
    {
      Application->ProcessMessages();
      GetExitCodeProcess(PI.hProcess, &Status);
    }*/
  }
//  BringWindowToTop(Handle);
}
//---------------------------------------------------------------------------
void _fastcall TdlgMain::AddPath(const olxstr& T)  {
  olxstr Dir, Tmp;
  Dir = TEFile::TrimPathDelimeter(T).UpperCase();

  TStrList toks(getenv("PATH"), ';');
  if( toks.IndexOf(Dir) == -1 )  {
    toks.Add(Dir);
    putenv( (olxstr("PATH=") << toks.Text(';')).c_str() );
  }
}
//---------------------------------------------------------------------------


void __fastcall TdlgMain::miSearchClick(TObject *Sender)
{
  if( !dlgSearch )
  {
    dlgSearch = new TdlgSearch(this);
  }
  dlgSearch->Show();
}
//---------------------------------------------------------------------------
void __fastcall TdlgMain::sbOlex2Click(TObject *Sender)  {
  olxstr tmp(Olex2Path);
  tmp << "olex2.dll";
  //olxstr path;
  if( !CurrentFile.IsEmpty() )  {
    tmp << ' ' << '\'' << CurrentFile << '\'';
    //path = TEFile::ExtractFilePath(CurrentFile);
  }
  tmp.c_str();  //! make sure rawstr is zero ended
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  si.wShowWindow = SW_SHOW;
  si.dwFlags = STARTF_USESHOWWINDOW;
//  SI.hStdInput = Console;
  PROCESS_INFORMATION PI;
  if( !CreateProcess(NULL, tmp.raw_str(), NULL, NULL,   true,
        0, NULL,
        NULL,
        &si, &PI))  {
    Application->MessageBox("Cannot launch Olex2...", "Error", MB_OK|MB_ICONERROR);
  }
}
//---------------------------------------------------------------------------
void TdlgMain::ClearTree()  {
  TheTreeRoot.Clear();
  for( int i=0; i < tvTree->Items->Count; i++ )  {
    if( tvTree->Items->Item[i]->Data != NULL )
      delete (IEObject*)tvTree->Items->Item[i]->Data;
  }
}
//---------------------------------------------------------------------------
void TdlgMain::InitTree()  {
  ClearTree();
  olxstr tmp;
  if( TEFile::Exists(IndexFile) )  {
    Index->LoadFromFile(IndexFile, false);
    for( size_t i=0; i < Index->IFiles.Count(); i++)  {
      //tvTree->Items->Add( NULL, Index->IFiles[i]->Name.c_str());
      AddNode( tvTree->Items, Index->IFiles[i]->Name);
    }
    for( size_t i=0; i < Index->ZFiles.Count(); i++)  {
      for( size_t j=0; j < Index->ZFiles[i]->Index->IFiles.Count(); j++ )  {
        tmp = Index->ZFiles[i]->FileName;
        tmp << '@' << Index->ZFiles[i]->Index->IFiles[j]->Name;
        AddNode( tvTree->Items, TEFile::WinPath(tmp));
      }
    }
  }
}
//---------------------------------------------------------------------------
void TdlgMain::LoadCurrentFile()  {
  if( TEFile::Exists(CurrentFile) )  {
    try   {
      //TSAtomPList toGrow;
      olxstr FExt = TEFile::ExtractFileExt(CurrentFile);
      mMemo->Lines->LoadFromFile( CurrentFile.c_str() );
      sbView->Enabled = true;
      sbOlex2->Enabled = true;
      Organiser->XFile->LoadFromFile(CurrentFile);
      Organiser->XFile->GetAsymmUnit().DetachAtomType(iQPeakZ, true);
      Organiser->XFile->GetLattice().UpdateConnectivity();
      Organiser->XFile->GetLattice().CompaqAll();
      //for( int i=0; i < Organiser->XFile->GetLattice().AtomCount(); i++ )
      //  if( Organiser->XFile->GetLattice().IsExpandable( Organiser->XFile->GetLattice().GetAtom(i)) )
      //    toGrow.AddACopy( &Organiser->XFile->GetLattice().GetAtom(i) );
      //Organiser->XFile->GetLattice().GrowAtoms(toGrow, false, NULL);
      //Organiser->XFile->GetLattice().GenerateWholeContent(NULL);
      FExt = "LCELLS: ";
      FExt << Organiser->XFile->LastLoader()->GetTitle();
      dlgMolDraw->Caption = FExt.c_str();
      Organiser->Update(); // do zoom and mpln calculations
      if( !dlgMolDraw->Visible )
        dlgMolDraw->Visible = true;
      Organiser->Draw();
    }
    catch(...)  {
      Application->MessageBox("Cannot load the file for viewing...", "Error", MB_OK|MB_ICONERROR);
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::tvTreeGetSelectedIndex(TObject *Sender,
      TTreeNode *Node)
{
  if( tvTree->Selected == NULL )  return;
  if( tvTree->Selected->Data == NULL )  return;

  sbView->Enabled = false;
  sbOlex2->Enabled = false;
  if( EsdlInstanceOf(*(IEObject*)tvTree->Selected->Data, olxstr) )  {
    if( CurrentFile == *(olxstr*)tvTree->Selected->Data )
      return;
    CurrentFile = *(olxstr*)tvTree->Selected->Data;
    LoadCurrentFile();
  }
}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::Expandall1Click(TObject *Sender)
{
  tvTree->Items->BeginUpdate();
  for( int i=0; i < tvTree->Items->Count; i++ )
    tvTree->Items->Item[i]->Expand(true);
  tvTree->Items->EndUpdate();
}
//---------------------------------------------------------------------------

void __fastcall TdlgMain::Collapseall1Click(TObject *Sender)
{
  tvTree->Items->BeginUpdate();
  for( int i=0; i < tvTree->Items->Count; i++ )
    tvTree->Items->Item[i]->Collapse(true);
  tvTree->Items->EndUpdate();
}
//---------------------------------------------------------------------------

