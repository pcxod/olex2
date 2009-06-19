#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "winzipfs.h"

#ifdef __WIN32__ // non-portable code

#include "efile.h"
#include "bapp.h"

#include "io.h"
#include "sys/stat.h"

TWinZipFileSystem::TWinZipFileSystem(const olxstr& zip_name, bool unused)  {
  TEFile::CheckFileExists(__OlxSourceInfo, zip_name);
  zip = OpenZip(zip_name.u_str(), NULL);
  if( zip == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid zip file");
}
//..............................................................................
TWinZipFileSystem::~TWinZipFileSystem()  {
  if( zip != NULL )
    CloseZip(zip);
  for( int i=0; i < TmpFiles.Count(); i++ )
    TEFile::DelFile( TmpFiles[i] );
}
//..............................................................................
IDataInputStream* TWinZipFileSystem::OpenFile(const olxstr& Source)  {
  TOnProgress Progress;
  olxstr fn( TEFile::UnixPath(Source) );

  int zindex = -1;
  ZIPENTRY* ze = NULL;
  FindZipItem(zip, fn.u_str(), true, &zindex, ze);
  if( zindex == -1 )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate zip entry: ") << Source);
  olxch TmpFN[512];
  GetTempPath(512, TmpFN);
  olxstr Tmp(TmpFN);
  GetTempFileName(Tmp.u_str(), olx_T("zip"), 0, TmpFN);
  olxstr tmp_fn(TmpFN);

  Progress.SetMax( (double)TEFile::FileLength(TmpFN) );
  OnProgress->Enter(this, &Progress);
  Progress.SetAction( olxstr("Extracting ") << Source );
  Progress.SetPos(0);
  Progress.SetMax(1);

  UnzipItem(zip, zindex, tmp_fn.u_str() );
  chmod( tmp_fn.c_str(), S_IREAD|S_IWRITE);

  Progress.SetMax( (double)TEFile::FileLength(tmp_fn) );
  OnProgress->Enter(this, &Progress);
  Progress.SetAction("Done");
  Progress.SetPos( (double)TEFile::FileLength(tmp_fn) );
  OnProgress->Exit(this, &Progress);
  TmpFiles.Add( TmpFN );
  return new TEFile( TmpFN, "rb" );
}
//..............................................................................
void TWinZipFileSystem::ExtractAll(const olxstr& dest)  {
  ZIPENTRY ze;
  olxstr extractPath( TEFile::AddTrailingBackslash(dest) );
  GetZipItem(zip, -1, &ze);
  int numitems = ze.index;
  TOnProgress Progress;
  Progress.SetMax( numitems );
  OnProgress->Enter( NULL, &Progress );
  // -1 gives overall information about the zipfile
  for( int zi=0; zi < numitems; zi++ )  {
    ZIPENTRY ze;
    GetZipItem(zip, zi, &ze); // fetch individual details
    Progress.SetPos( zi );
    Progress.SetAction( ze.name );
    OnProgress->Execute( NULL, &Progress );
    UnzipItem(zip, zi, (extractPath + ze.name).u_str() );         // e.g. the item's name.
  }
  Progress.SetPos( numitems );
  OnProgress->Exit( NULL, &Progress );
}
#endif // __WIN32__
