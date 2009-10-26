#ifdef __BORLANDC__
  #pragma hdrstop
  #include <winbase.h>
#endif

#ifdef __WXWIDGETS__

#include "wxzipfs.h"
#include "efile.h"
#include "bapp.h"
#include "log.h"
#include "ememstream.h"

olxstr TZipWrapper::ZipUrlSignature = "@zip:";
//---------------------------------------------------------------------------
//..............................................................................
TMemoryBlock* TZipWrapper::GetMemoryBlock(const olxstr &EM)  {
  olxstr entryName( TEFile::UnixPath(EM) );
  TMemoryBlock * mb = NULL;
  if( UseCache )
    mb = FMemoryBlocks[entryName];

  if( mb == NULL )  {
    wxZipEntry *entry = FEntries[TEFile::UnixPath(entryName)];
    if( !entry )  return NULL;
    if( !FInputStream->OpenEntry(*entry) )  return NULL;
    //if( FInputStream->GetLength() <=0 )  {  FInputStream->CloseEntry();  return NULL;  }

    mb = new TMemoryBlock;
    mb->Length = FInputStream->GetLength();
    mb->Buffer = new char [mb->Length + 1];
    mb->DateTime = entry->GetDateTime().GetTicks();
    FInputStream->Read( mb->Buffer, FInputStream->GetLength());
    FInputStream->CloseEntry();
    if( UseCache )
      FMemoryBlocks.Add( entryName, mb );
  }
  return mb;
}
//..............................................................................
TZipWrapper::TZipWrapper(const olxstr &zipName, bool useCache) : zip_name(zipName)  {
  OnProgress = &Actions.NewQueue("ON_PROGRESS");
  FInputStream = NULL;
  wxfile = NULL;
  UseCache = useCache;
  //FFileInputStream = NULL;
  if( !TEFile::Exists( zipName )  )  return;
  FInputStream = new wxZipInputStream( new wxFileInputStream( zipName.u_str() ) );
  wxZipEntry *entry;
  while( (entry = FInputStream->GetNextEntry() ) != NULL )
    FEntries.Add( TEFile::UnixPath(entry->GetName().c_str()), entry );
}
//..............................................................................
TZipWrapper::TZipWrapper(TEFile* file, bool useCache)  {
  FInputStream = NULL;
  wxfile = NULL;
  UseCache = useCache;
  //FFileInputStream = NULL;
  if( file == NULL )  return;
  wxfile = new wxFile( file->FileNo() );
  FInputStream = new wxZipInputStream( new wxFileInputStream( *wxfile ) );
  wxZipEntry *entry;
  while( (entry = FInputStream->GetNextEntry() ) != NULL )
    FEntries.Add( TEFile::UnixPath(entry->GetName().c_str()), entry );
}
//..............................................................................
TZipWrapper::~TZipWrapper()  {
  TMemoryBlock *mb;
  for( size_t i=0; i < FMemoryBlocks.Count(); i++ )  {
    mb = FMemoryBlocks.GetObject(i);
    delete [] mb->Buffer;
    delete mb;
  }
  for( size_t i=0; i < FEntries.Count(); i++ )
    delete FEntries.GetObject(i);
  if( FInputStream != NULL )  delete FInputStream;
  if( wxfile != NULL )   {
    wxfile->Detach();
    delete wxfile;
  }
//  if( FFileInputStream )  delete FFileInputStream;
}
//..............................................................................
IDataInputStream* TZipWrapper::OpenEntry(const olxstr &EN)  {
  TMemoryBlock * mb = GetMemoryBlock(EN);
  if( mb == NULL )  return NULL;
  TEMemoryStream *ms = new TEMemoryStream();
  ms->Write( mb->Buffer, mb->Length );
  ms->SetPosition(0);
  if( !UseCache )  {
    delete [] mb->Buffer;
    delete mb;
  }
  return ms;
}
//..............................................................................
wxInputStream* TZipWrapper::OpenWxEntry(const olxstr &EN)  {
  TMemoryBlock * mb = GetMemoryBlock(EN);
  if( mb != NULL )  {
    wxMemoryInputStream* rv = new wxMemoryInputStream(mb->Buffer, mb->Length);
    if( !UseCache )  {
      delete mb->Buffer;
      delete mb;
    }
    return rv;
  }
  return NULL;
}
//..............................................................................
void TZipWrapper::ExtractAll(const olxstr& dest)  {
  olxstr extractPath( TEFile::AddTrailingBackslash(dest) );
  TOnProgress Progress;
  Progress.SetMax( FEntries.Count() );
  Progress.SetAction( olxstr("Unpacking ") << zip_name << "...");
  OnProgress->Enter( NULL, &Progress );
  const size_t bf_sz = 1024*64;
  char* bf = new char[bf_sz];
  for( size_t i=0; i < FEntries.Count(); i++ )  {
    if( !FInputStream->OpenEntry( *FEntries.GetObject(i) ) )
      continue;
    if( FEntries.GetString(i).EndsWith('/') )  continue;

    Progress.SetPos( i+1 );
    Progress.SetAction( FEntries.GetString(i) );
    OnProgress->Execute( NULL, &Progress );

    olxstr dest_file = extractPath + FEntries.GetString(i);
    olxstr dst_dir = TEFile::ExtractFilePath(dest_file);
    if( !TEFile::Exists(dst_dir) )
      if( !TEFile::MakeDirs(dst_dir) )
        TBasicApp::GetLog().Error( olxstr("Failed to create folder: ") << dst_dir);
    if( TEFile::Exists(dest_file) )  {
      if( !TEFile::DelFile(dest_file) )
        TBasicApp::GetLog().Error( olxstr("Failed to remove/update file: ") << dest_file);
    }
    wxFileOutputStream fos( dest_file.u_str() );
    while( FInputStream->Read(bf, bf_sz).LastRead() != 0 )
      fos.Write(bf, FInputStream->LastRead());
  }
  delete [] bf;
  Progress.SetPos( FEntries.Count() );
  Progress.SetAction("Done");
  OnProgress->Exit( NULL, &Progress );
}
//..............................................................................
bool TZipWrapper::IsValidFileName(const olxstr &FN)  {
  int zi = FN.IndexOf(ZipUrlSignature);
  if( zi != InvalidIndex && zi > 0 )  {
    if( TEFile::Exists( ExtractZipName(FN) ) )  return true;
    return false;
  }
  return TEFile::Exists(FN);
}
//..............................................................................
bool TZipWrapper::IsZipFile(const olxstr &FN)  {
  size_t ind = FN.IndexOf(ZipUrlSignature);
  return ind != InvalidIndex && ind != 0;
}
//..............................................................................
olxstr TZipWrapper::ExtractZipName(const olxstr &FN)  {
  size_t zi = FN.IndexOf(ZipUrlSignature);
  if( zi != InvalidIndex && zi > 0 )
    return FN.SubStringTo(zi);
  return EmptyString;
}
//..............................................................................
olxstr TZipWrapper::ExtractZipEntryName(const olxstr &FN)  {
  size_t zi = FN.IndexOf(ZipUrlSignature);
  if( zi != InvalidIndex && zi > 0 )
    return FN.SubStringFrom(zi+ZipUrlSignature.Length());
  return EmptyString;
}
//..............................................................................
bool TZipWrapper::SplitZipUrl(const olxstr &fullName, TZipEntry &ZE)  {
  int zi = fullName.IndexOf(ZipUrlSignature);
  if( zi == InvalidIndex )  return false;
  ZE.ZipName = fullName.SubStringTo(zi);
  ZE.ZipName = TEFile::UnixPath( ZE.ZipName );
  ZE.EntryName = fullName.SubStringFrom( zi + ZipUrlSignature.Length() );
  ZE.EntryName = TEFile::UnixPath( ZE.EntryName );
  return true;
}
//..............................................................................
olxstr TZipWrapper::ComposeFileName(const olxstr &ZipFileNameA, const olxstr &FNA)  {
  int zi = ZipFileNameA.IndexOf(ZipUrlSignature);
  if( zi == InvalidIndex )  return FNA;
  olxstr ZipFileName = TEFile::WinPath(ZipFileNameA),
           FN = TEFile::WinPath(FNA);
  olxstr zipPart = ZipFileName.SubStringTo(zi+ZipUrlSignature.Length());
  olxstr zipPath = TEFile::ExtractFilePath(zipPart);
  TEFile::AddTrailingBackslashI(zipPath);
  if( FN.StartsFrom(zipPath) )
    return zipPart + TEFile::UnixPath( FN.SubStringFrom(zipPath.Length()) );
  return FNA;
}
/*____________________________________________________________________________*/


TwxZipFileSystem::TwxZipFileSystem(const olxstr& zip_name, bool UseCache) : 
zip(zip_name, UseCache) 
{ 
  Access = afs_ReadOnlyAccess;
  AActionHandler::SetToDelete(false);
  zip.OnProgress->Add(this);
}
//..............................................................................
TwxZipFileSystem::TwxZipFileSystem(TEFile* zip_fh, bool UseCache) : 
  zip(zip_fh, UseCache) { }
//..............................................................................
IInputStream* TwxZipFileSystem::_DoOpenFile(const olxstr& Source)  {
  TOnProgress Progress;
  olxstr fn( TEFile::UnixPath(Source) );
  Progress.SetAction( olxstr("Extracting ") << fn );
  Progress.SetPos(0);
  Progress.SetMax(1);
  OnProgress.Enter(this, &Progress);
  
  IDataInputStream* rv = zip.OpenEntry(Source);
  if( rv == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate zip entry: ") << fn);
  
  Progress.SetAction("Done");
//  Progress.SetPos( TEFile::FileLength(TmpFN) );
  OnProgress.Exit(this, &Progress);
  return rv;
}
//..............................................................................
void TwxZipFileSystem::ExtractAll(const olxstr& dest)  {
  zip.ExtractAll(dest);
}
//..............................................................................

#endif // __WXWIDGETS__
