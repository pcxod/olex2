#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "winzipfs.h"
#include "efile.h"
#include "bapp.h"

#include "io.h"
#include "sys/stat.h"

TWinZipFileSystem::TWinZipFileSystem(const olxstr& zip_name)  {
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

  char  TmpFN[512];
  GetTempPath(512, TmpFN);
  olxstr Tmp(TmpFN);
  GetTempFileName(Tmp.u_str(), "zip", 0, TmpFN);
  int zindex = -1;
  ZIPENTRY* ze = NULL;
  FindZipItem(zip, fn.u_str(), true, &zindex, ze);
  TmpFiles.Add( TmpFN );
  if( zindex == -1 )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate zip entry: ") << Source);
  Progress.SetMax(TEFile::FileLength(TmpFN));
  TBasicApp::GetInstance()->OnProgress->Enter(this, &Progress);
  Progress.SetAction( olxstr("Extracting ") << Source );
  Progress.SetPos(0);
  Progress.SetMax(1);

  UnzipItem(zip, zindex, TmpFN );
  chmod( TmpFN, S_IREAD|S_IWRITE);

  Progress.SetMax(TEFile::FileLength(TmpFN));
  TBasicApp::GetInstance()->OnProgress->Enter(this, &Progress);
  Progress.SetAction("Done");
  Progress.SetPos( TEFile::FileLength(TmpFN) );
  TBasicApp::GetInstance()->OnProgress->Exit(this, &Progress);
  return new TEFile( TmpFN, "rb" );
}
//..............................................................................

