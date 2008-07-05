//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include "shellutil.h"
#include "efile.h"
#include "unzip.h"
#include "bapp.h"
#include "io.h"
#include "sys/stat.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfMain *fMain;
//---------------------------------------------------------------------------
__fastcall TfMain::TfMain(TComponent* Owner)
  : TForm(Owner)
{
  new TBasicApp( ParamStr(0).c_str() );
}
//---------------------------------------------------------------------------
__fastcall TfMain::~TfMain()  {
  delete TBasicApp::GetInstance();
}
//---------------------------------------------------------------------------
void __fastcall TfMain::iSplashMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  MouseDownX = X;
  MouseDownY = Y;
  MouseDown = true;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::iSplashMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
  if( !MouseDown )  return;
  if( !Dragging )
  {
    if( abs(X-MouseDownX) >= 3 || abs(Y-MouseDownY) >= 3 )
      Dragging = true;
  }
  if( !Dragging )  return;
  Left = Left + (X-MouseDownX);
  Top = Top + (Y-MouseDownY);
}
//---------------------------------------------------------------------------
void __fastcall TfMain::iSplashMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  MouseDown = Dragging = false;
}
//---------------------------------------------------------------------------
void __fastcall TfMain::ExitClick(TObject *Sender)
{
  Application->Terminate();  
}
//---------------------------------------------------------------------------
bool ExtractZip(const olxstr& zipfn, const olxstr& extractPath)  {
  HZIP hz = OpenZip(zipfn.c_str(), NULL);
  if( hz == NULL )  return false;
  ZIPENTRY ze;
  GetZipItem(hz, -1, &ze);
  int numitems = ze.index;
  olxstr outFN;
  // -1 gives overall information about the zipfile
  for( int zi=0; zi < numitems; zi++ )  {
    ZIPENTRY ze;
    GetZipItem(hz, zi, &ze); // fetch individual details
    outFN = extractPath;
    outFN << ze.name;
    if( !TEFile::FileExists(outFN) )  {
      UnzipItem(hz, zi, outFN.c_str() );         // e.g. the item's name.
      chmod( outFN.c_str(), S_IREAD|S_IWRITE);
    }
  }
  CloseZip(hz);
  return true;
}

void __fastcall TfMain::ContinueClick(TObject *Sender)
{
  if( rbInstall->Checked )  {
    olxstr tmp( TBasicApp::GetInstance()->BaseDir() );
    tmp << "installer.exe";
    LaunchFile( tmp.c_str() );
  }
  else if( rbWebsite->Checked )  {
    LaunchFile("explorer http://www.olex2.org/olex");
  }
  else if( rbRunFromCD->Checked )  {
    olxstr mdf = TShellUtil::GetSpecialFolderLocation(fiAppData);
    if( !TEFile::FileExists(mdf) )  {
      Application->MessageBox("Could not locate the Application data folder...", "Error", MB_OK|MB_ICONERROR);
      return;
    }
    TEFile::AddTrailingBackslashI(mdf);
    mdf << "Olex2u\\";
    if( !TEFile::FileExists( mdf ) )
      TEFile::MakeDir(mdf);
    if( !ExtractZip( TBasicApp::GetInstance()->BaseDir() + "samples.zip", mdf) )  {
      Application->MessageBox("Could not extract sample data...", "Error", MB_OK|MB_ICONERROR);
      return;
    }
    olxstr lastosp( TBasicApp::GetInstance()->BaseDir() + "last.osp" );
    bool provide_arg = true;
    if( TEFile::FileExists(lastosp) )  {
      TEFile::Copy(lastosp, mdf + "last.osp");
      chmod( (olxstr(mdf) << mdf + "last.osp").c_str(), S_IREAD|S_IWRITE);
      provide_arg = false;
    }
    olxstr tmp( TBasicApp::GetInstance()->BaseDir() );
    if( provide_arg )  tmp << "Olex2/olex2.exe '" << mdf << "samples/sucrose/sucrose.ins'";
    else               tmp << "Olex2/olex2.exe";
    LaunchFile( tmp.c_str() );
//    Application->MessageBox( "Please wait for the program to startup from CD.\nIt may take some time...",
//      "Wait",
//      MB_OK|MB_ICONINFORMATION);
  }
  Application->Terminate();
}
//---------------------------------------------------------------------------
bool TfMain::LaunchFile( const AnsiString& fileName )  {
  STARTUPINFO si;
  PROCESS_INFORMATION ProcessInfo;
  AnsiString Tmp;
  Tmp += fileName.c_str();
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  si.wShowWindow = SW_SHOW;
  si.dwFlags = STARTF_USESHOWWINDOW;

  // Launch the child process.
  if( !CreateProcess(
        NULL,
        Tmp.c_str(),
//        NULL,
        NULL, NULL,   true,
        0, NULL,
        NULL,
        &si, &ProcessInfo))
  {
    Application->MessageBox( (AnsiString("Could not start ") += fileName.c_str()).c_str(),
      "Error",
      MB_OK|MB_ICONERROR);
      return false;
  }
  return true;
}
