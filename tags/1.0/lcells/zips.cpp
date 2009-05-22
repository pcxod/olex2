  //---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "zips.h"
#include "efile.h"
#include "unzip.h"

//---------------------------------------------------------------------------
TZipShell::TZipShell()  {  }
TZipShell::~TZipShell()  {  }
void _fastcall TZipShell::SetTmpPath(const olxstr& S)  {
  FTmpPath = TEFile::AddTrailingBackslash(S);
}
bool _fastcall TZipShell::Initialize(const olxstr& Arch)  {
  if( !TEFile::FileExists(Arch) )
    return false;
  HZIP hz = OpenZip(Arch.c_str(),0);
  ZIPENTRY ze;
  GetZipItem(hz,-1,&ze);
  FFiles.Clear();
  int numitems = ze.index;
  FFiles.SetCapacity( numitems );
  for (int i=0; i < numitems; i++)  {
    GetZipItem(hz,i,&ze);
    FFiles.Add( ze.name );
  }
  CloseZip(hz);
  FArchive = Arch;
  return true;
}
void _fastcall TZipShell::MaskFiles(const olxstr& Mask, const olxstr& Mask1)  {
  olxstr Tmp;
  for( int i=0; i < FFiles.Count(); i++ )  {
    Tmp = TEFile::ExtractFileExt(FFiles.String(i));
    if( (Tmp != Mask)  && (Tmp != Mask1) )  {
      FFiles.Delete(i);
      i--;
    }
  }
}
void _fastcall TZipShell::MaskFiles(const olxstr& Mask)  {
  olxstr Tmp;
  for( int i=0; i < FFiles.Count(); i++ )  {
    Tmp = TEFile::ExtractFileExt(FFiles.String(i));
    if( Tmp != Mask )  {
      FFiles.Delete(i);
      i--;
    }
  }
}
bool _fastcall TZipShell::Extract()  {
  if( Files.Count() == 0  )
    return false;
  HZIP hz = OpenZip(FArchive.c_str(),0);
  ZIPENTRY ze;
  GetZipItem(hz,-1,&ze);
  int numitems = ze.index;
  for (int i=0; i < numitems; i++)  {
    GetZipItem(hz,i,&ze);
    if( FFiles.IndexOf(ze.name ) >= 0 )
      UnzipItem(hz,i, (FTmpPath+ze.name).c_str() );
  }
  CloseZip(hz);
  return true;
}
bool _fastcall TZipShell::GetFile(const olxstr& Name)  {
  FFiles.Clear();
  FFiles.Add(Name);
  return Extract();
}

#pragma package(smart_init)

