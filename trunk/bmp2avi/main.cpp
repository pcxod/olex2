//---------------------------------------------------------------------------

#include <vcl.h>
#include <vfw.h>
#pragma hdrstop

#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgMain *dlgMain;
//---------------------------------------------------------------------------
__fastcall TdlgMain::TdlgMain(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void TdlgMain::ClearFileList()  {
  lvFiles->Items->BeginUpdate();
  for( int i=0; i < lvFiles->Items->Count; i++ )
    delete (AnsiString*)lvFiles->Items->Item[i]->Data;
  lvFiles->Items->Clear();
  lvFiles->Items->EndUpdate();
}
//---------------------------------------------------------------------------
void __fastcall TdlgMain::bbLoadFileListClick(TObject *Sender)
{
  if( !dlgBmpLoad->Execute() )
    return;
  lvFiles->Items->BeginUpdate();
  for( int i=0; i < dlgBmpLoad->Files->Count; i++ )  {
    TListItem* li = lvFiles->Items->Add();
    li->Checked = true;
    li->Caption = ExtractFileName(dlgBmpLoad->Files->Strings[i]);
    li->Data = new AnsiString(dlgBmpLoad->Files->Strings[i]);
  }
  lvFiles->Items->EndUpdate();
  bbSaveAVI->Enabled = (lvFiles->Items->Count > 3);
}
//---------------------------------------------------------------------------
void __fastcall TdlgMain::FormClose(TObject *Sender, TCloseAction &Action)
{
  ClearFileList();
}
//---------------------------------------------------------------------------
void __fastcall TdlgMain::bbClearFileListClick(TObject *Sender)
{
  ClearFileList();
  bbSaveAVI->Enabled = false;
}
//---------------------------------------------------------------------------
void __fastcall TdlgMain::bbSaveAVIClick(TObject *Sender)
{
  if( !dlgSaveAVI->Execute() )
    return;
////////////////////////////////////////////////////////////////////////////////
  Graphics::TBitmap *bmp = new Graphics::TBitmap;
  bmp->LoadFromFile(*(AnsiString*)lvFiles->Items->Item[0]->Data);
  int BmpWidth = bmp->Width;
  int BmpHeight = bmp->Height;
  const int bits = 16;
  UINT                wLineLen ;
  DWORD               dwSize ;
  DWORD               wColSize ;
  wLineLen = (BmpWidth*bits+31)/32 * 4;
  wColSize = sizeof(RGBQUAD)*((bits <= 8) ? 1<<bits : 0);
  dwSize = sizeof(BITMAPINFOHEADER) + wColSize +
    (DWORD)(UINT)wLineLen*(DWORD)(UINT)BmpHeight;
  //
  // Allocate room for a DIB and set the LPBI fields
  //
  LPBITMAPINFOHEADER BmpStart = (LPBITMAPINFOHEADER)new char[dwSize];

  BmpStart->biSize = sizeof(BITMAPINFOHEADER) ;
  BmpStart->biWidth = BmpWidth;
  BmpStart->biHeight = BmpHeight;
  BmpStart->biPlanes = 1;
  BmpStart->biBitCount = (WORD) bits;
  BmpStart->biCompression = BI_RGB;
  BmpStart->biSizeImage = dwSize - sizeof(BITMAPINFOHEADER) - wColSize;
  BmpStart->biXPelsPerMeter = 0;
  BmpStart->biYPelsPerMeter = 0;
  BmpStart->biClrUsed = (bits <= 8) ? 1<<bits : 0;
  BmpStart->biClrImportant = 0;

  LPBYTE DIBits = (LPBYTE)(BmpStart+1)+wColSize;
  BmpStart->biClrUsed = (bits <= 8) ? 1<<bits : 0;
////////////////////////////////////////////////////////////////////////////////
  AVICOMPRESSOPTIONS AviComp;
  AVICOMPRESSOPTIONS AviComps[1];
  AVISTREAMINFO  AviInfo;
  PAVIFILE AviFile;
  PAVISTREAM AviStream, AviCStream;
  AnsiString Tmp;
  TMemoryStream *bf = new TMemoryStream;
  COMPVARS CV;
  ZeroMemory(&CV, sizeof(CV));
  CV.cbSize = sizeof(CV);
  int fccHandler =   mmioFOURCC('M','S','V','C'), ms, fc=0, res;

  CV.fccHandler = fccHandler;
  CV.lpbiOut  = (LPBITMAPINFO)BmpStart;

  ZeroMemory(&AviComp, sizeof(AviComp));
  AviComp.fccType = streamtypeVIDEO;
  if( ICCompressorChoose(Handle, ICMF_CHOOSE_ALLCOMPRESSORS|ICMF_CHOOSE_PREVIEW,  NULL, NULL,  &CV,
    "Please, Select Compressor...") )
  {
    fccHandler = CV.fccHandler;
    AviComp.fccHandler = fccHandler;
    AviComp.dwQuality     = CV.lQ;  // compquality;        // compress quality 0-10,000
    AviComp.dwBytesPerSecond = CV.lDataRate;    // bytes per second
    AviComp.dwFlags       = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES;    // flags
  }
  else  {
    AviComp.fccHandler = fccHandler;
    AviComp.dwQuality     = 100000;  // compquality;        // compress quality 0-10,000
    AviComp.dwBytesPerSecond = 0;    // bytes per second
    AviComp.dwFlags       = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES;    // flags
  }

  ZeroMemory(&AviInfo, sizeof(AviInfo));
  AviInfo.fccType = streamtypeVIDEO;
  AviInfo.fccHandler             = 0;
  AviInfo.dwScale                = 1;
  AviInfo.dwRate                 = atoi(leFps->Text.c_str());//fps;
  AviInfo.dwSuggestedBufferSize  = BmpStart->biSizeImage;

  HDC hdc = GetDC(Handle);
  AVIFileInit();
  if( FileExists(dlgSaveAVI->FileName) )
    DeleteFile(dlgSaveAVI->FileName);
  if( (res = AVIFileOpen(&AviFile, dlgSaveAVI->FileName.c_str(), OF_CREATE, NULL)) != AVIERR_OK )
    goto exit;
  if( (res = AVIFileCreateStream(AviFile,  &AviStream, &AviInfo)) != AVIERR_OK )
    goto exit1;
  if( (res = AVIMakeCompressedStream(&AviCStream, AviStream, &AviComp, NULL)) != AVIERR_OK )
    goto exit2;
  if( (res = AVIStreamSetFormat(AviCStream, 0,
         BmpStart,      // stream format
         BmpStart->biSize +   // format size
         BmpStart->biClrUsed * sizeof(RGBQUAD))) )
    goto exit3;

  bf->Write("INFO", 4);

  Tmp = "Bmp2Avi, (c) Oleg Dolomanov 2009";
  bf->Write("ICOP", 4);
  ms = Tmp.Length();
  bf->Write(&ms, 4);
  bf->Write(Tmp.c_str(), ms+1);

  Tmp = FormatDateTime("yyyy-mm-dd", Now());
  bf->Write("ICRD", 4);
  ms = Tmp.Length();
  bf->Write(&ms, 4);
  bf->Write(Tmp.c_str(), ms+1);

  if( AVIFileWriteData(AviFile, mmioFOURCC('L','I','S','T'), bf->Memory, bf->Size) != AVIERR_OK )
    goto exit2;
  for( int i=0; i < lvFiles->Items->Count; i++ )  {
    if( i != 0 )  {
      bmp->LoadFromFile(*(AnsiString*)lvFiles->Items->Item[i]->Data);
      if( bmp->Width != BmpWidth || bmp->Height != BmpHeight )  // should really pre-analyse
        continue;
    }
    iImage->Picture->Bitmap = bmp;
    Application->ProcessMessages();
    GetDIBits(hdc, bmp->Handle, 0, BmpHeight, DIBits, (LPBITMAPINFO)BmpStart, DIB_RGB_COLORS);
    if( AVIStreamWrite(
          AviCStream,  // stream pointer
          fc,        // time of this frame in ms
          1,        // number to write
          DIBits,
          BmpStart->biSizeImage,  // size of this frame
          0,    //Dependent n previous frame, not key frame
          NULL,
          NULL) != AVIERR_OK )
      goto exit4;
    fc += 1;  // 1/2 of the second...
  }
exit4:
  if( res != AVIERR_OK )
    Application->MessageBox("Failed write data to stream", "Error", MB_OK|MB_ICONERROR);
exit3:
  AVIStreamClose(AviCStream);
  if( res != AVIERR_OK )
    Application->MessageBox("Failed to inialise stream format", "Error", MB_OK|MB_ICONERROR);
exit2:
  AVIStreamClose(AviStream);
  if( res != AVIERR_OK )
    Application->MessageBox("Failed to inialise compressed stream", "Error", MB_OK|MB_ICONERROR);
exit1:
  AVIFileClose(AviFile);
  if( res != AVIERR_OK )
    Application->MessageBox("Failed to open file", "Error", MB_OK|MB_ICONERROR);
exit:
  delete BmpStart;
  delete bmp;
  delete bf;
  ReleaseDC(Handle, hdc);
  if( res == AVIERR_OK )
    ShellExecute(NULL, "open", dlgSaveAVI->FileName.c_str(), NULL, ExtractFileDir(dlgSaveAVI->FileName).c_str(), SW_SHOWNA);
}
//---------------------------------------------------------------------------
void __fastcall TdlgMain::lvFilesSelectItem(TObject *Sender,
      TListItem *Item, bool Selected)
{
  if( !Selected || Item == NULL )
    return;
  iImage->Picture->LoadFromFile( *(AnsiString*)Item->Data);
}
//---------------------------------------------------------------------------
