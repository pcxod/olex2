/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifdef __WXWIDGETS__
#include "wxzipfs.h"
#include "efile.h"
#include "bapp.h"
#include "log.h"
#include "ememstream.h"

//..............................................................................
TMemoryBlock* TZipWrapper::GetMemoryBlock(const olxstr &EM) {
  olxstr entryName = TEFile::UnixPath(EM);
  TMemoryBlock * mb = NULL;
  if (UseCache)
    mb = FMemoryBlocks.Find(entryName, NULL);
  if (mb == NULL) {
    wxZipEntry *entry = FEntries.Find(TEFile::UnixPath(entryName), NULL);
    if (entry ==0) return NULL;
    if (!FInputStream->OpenEntry(*entry))
      return NULL;
    //if( FInputStream->GetLength() <=0 )  {  FInputStream->CloseEntry();  return NULL;  }

    mb = new TMemoryBlock;
    mb->Length = FInputStream->GetLength();
    mb->Buffer = new char [mb->Length + 1];
    mb->DateTime = entry->GetDateTime().GetTicks();
    FInputStream->Read( mb->Buffer, FInputStream->GetLength());
    FInputStream->CloseEntry();
    if (UseCache)
      FMemoryBlocks.Add(entryName, mb);
  }
  return mb;
}
//..............................................................................
TZipWrapper::TZipWrapper(const olxstr &zipName, bool useCache)
: zip_name(zipName), OnProgress(Actions.New("ON_PROGRESS")), Break(false)
{
  FInputStream = NULL;
  wxfile = NULL;
  UseCache = useCache;
  if (!TEFile::Exists(zipName))  return;
  FInputStream = new wxZipInputStream(new wxFileInputStream(zipName.u_str()));
  wxZipEntry *entry;
  while ((entry = FInputStream->GetNextEntry()) != NULL )
    FEntries.Add(TEFile::UnixPath(entry->GetName()), entry);
}
//..............................................................................
TZipWrapper::TZipWrapper(TEFile* file, bool useCache)
: OnProgress(Actions.New("ON_PROGRESS")), Break(false)
{
  FInputStream = NULL;
  wxfile = NULL;
  UseCache = useCache;
  if (file == NULL) return;
  wxfile = new wxFile(file->FileNo());
  FInputStream = new wxZipInputStream(new wxFileInputStream(*wxfile));
  wxZipEntry *entry;
  while ((entry = FInputStream->GetNextEntry()) != NULL)
    FEntries.Add(TEFile::UnixPath(entry->GetName()), entry);
}
//..............................................................................
TZipWrapper::~TZipWrapper() {
  for (size_t i=0; i < FMemoryBlocks.Count(); i++) {
    TMemoryBlock *mb = FMemoryBlocks.GetValue(i);
    delete [] mb->Buffer;
    delete mb;
  }
  for (size_t i=0; i < FEntries.Count(); i++)
    delete FEntries.GetValue(i);
  if (FInputStream != NULL) delete FInputStream;
  if (wxfile != NULL) {
    wxfile->Detach();
    delete wxfile;
  }
}
//..............................................................................
IDataInputStream* TZipWrapper::OpenEntry(const olxstr &EN) {
  TMemoryBlock * mb = GetMemoryBlock(EN);
  if (mb == NULL) return NULL;
  TEMemoryStream *ms = new TEMemoryStream();
  ms->Write(mb->Buffer, mb->Length);
  ms->SetPosition(0);
  if (!UseCache) {

    delete [] mb->Buffer;
    delete mb;
  }
  return ms;
}
//..............................................................................
wxInputStream* TZipWrapper::OpenWxEntry(const olxstr &EN) {
  TMemoryBlock * mb = GetMemoryBlock(EN);
  if (mb != NULL) {
    wxMemoryInputStream* rv = new wxMemoryInputStream(mb->Buffer, mb->Length);
    if (!UseCache) {
      delete mb->Buffer;
      delete mb;
    }
    return rv;
  }
  return NULL;
}
//..............................................................................
bool TZipWrapper::ExtractAll(const olxstr& dest) {
  olxstr extractPath(TEFile::AddPathDelimeter(dest));
  TOnProgress Progress;
  Progress.SetMax(FEntries.Count());
  Progress.SetAction(olxstr("Unpacking ") << zip_name << "...");
  OnProgress.Enter(NULL, &Progress);
  const size_t bf_sz = 1024*64;
  char* bf = new char[bf_sz];
  bool res = true;
  for (size_t i=0; i < FEntries.Count(); i++) {
    if( this->Break )  {
      res = false;
      break;
    }
    if (FEntries.GetKey(i).EndsWith('/')) continue;
    if (!FInputStream->OpenEntry(*FEntries.GetValue(i)))
      continue;
    Progress.SetPos(i+1);
    Progress.SetAction(FEntries.GetKey(i));
    OnProgress.Execute(NULL, &Progress);

    olxstr dest_file = extractPath + FEntries.GetKey(i);
    olxstr dst_dir = TEFile::ExtractFilePath(dest_file);
    if (!TEFile::Exists(dst_dir))
      if (!TEFile::MakeDirs(dst_dir)) {
        TBasicApp::NewLogEntry(logError) << "Failed to create folder: " <<
          dst_dir;
        res = false;
        break;
      }
    if (TEFile::Exists(dest_file)) {
      if (!TEFile::DelFile(dest_file)) {
        TBasicApp::NewLogEntry(logError) << "Failed to remove/update file: " <<
          dest_file;
        res = false;
        break;
      }
    }
    wxFileOutputStream fos(dest_file.u_str());
    while (FInputStream->Read(bf, bf_sz).LastRead() != 0)
      fos.Write(bf, FInputStream->LastRead());
  }
  delete [] bf;
  Progress.SetPos(FEntries.Count());
  Progress.SetAction("Done");
  OnProgress.Exit(NULL, &Progress);
  return res;
}
//..............................................................................
bool TZipWrapper::IsValidFileName(const olxstr &FN) {
  size_t zi = FN.IndexOf(ZipUrlSignature_());
  if (zi != InvalidIndex) {
    if (TEFile::Exists(ExtractZipName(FN)))
      return true;
    return false;
  }
  return TEFile::Exists(FN);
}
//..............................................................................
bool TZipWrapper::IsZipFile(const olxstr &FN) {
  size_t ind = FN.IndexOf(ZipUrlSignature_());
  return ind != InvalidIndex && ind != 0;
}
//..............................................................................
olxstr TZipWrapper::ExtractZipName(const olxstr &FN) {
  size_t zi = FN.IndexOf(ZipUrlSignature_());
  if (zi != InvalidIndex && zi > 0)
    return FN.SubStringTo(zi);
  return EmptyString();
}
//..............................................................................
olxstr TZipWrapper::ExtractZipEntryName(const olxstr &FN) {
  size_t zi = FN.IndexOf(ZipUrlSignature_());
  if (zi != InvalidIndex && zi > 0)
    return FN.SubStringFrom(zi+ZipUrlSignature_().Length());
  return EmptyString();
}
//..............................................................................
bool TZipWrapper::SplitZipUrl(const olxstr &fullName, TZipEntry &ZE) {
  size_t zi = fullName.IndexOf(ZipUrlSignature_());
  if (zi == InvalidIndex)  return false;
  ZE.ZipName = fullName.SubStringTo(zi);
  ZE.ZipName = TEFile::UnixPath( ZE.ZipName );
  ZE.EntryName = fullName.SubStringFrom(zi + ZipUrlSignature_().Length());
  ZE.EntryName = TEFile::UnixPath( ZE.EntryName );
  return true;
}
//..............................................................................
olxstr TZipWrapper::ComposeFileName(const olxstr &ZipFileNameA,
  const olxstr &FNA)
{
  size_t zi = ZipFileNameA.IndexOf(ZipUrlSignature_());
  if (zi == InvalidIndex)  return FNA;
  olxstr ZipFileName = TEFile::WinPath(ZipFileNameA),
    FN = TEFile::WinPath(FNA);
  olxstr zipPart = ZipFileName.SubStringTo(zi+ZipUrlSignature_().Length());
  olxstr zipPath = TEFile::ExtractFilePath(zipPart);
  TEFile::AddPathDelimeterI(zipPath);
  if (FN.StartsFrom(zipPath))
    return zipPart + TEFile::UnixPath(FN.SubStringFrom(zipPath.Length()));
  return FNA;
}
/*____________________________________________________________________________*/


TwxZipFileSystem::TwxZipFileSystem(const olxstr& zip_name, bool UseCache)
: zip(zip_name, UseCache)
{
  Access = afs_ReadOnlyAccess;
  AActionHandler::SetToDelete(false);
  zip.OnProgress.Add(this);
}
//..............................................................................
TwxZipFileSystem::TwxZipFileSystem(TEFile* zip_fh, bool UseCache)
: zip(zip_fh, UseCache)
{
  Access = afs_ReadOnlyAccess;
  AActionHandler::SetToDelete(false);
  zip.OnProgress.Add(this);
}
//..............................................................................
TwxZipFileSystem::~TwxZipFileSystem() {
  zip.OnProgress.Remove(this);
}
//..............................................................................
olx_object_ptr<IInputStream> TwxZipFileSystem::_DoOpenFile(const olxstr& Source) {
  TOnProgress Progress;
  olxstr fn( TEFile::UnixPath(Source) );
  Progress.SetAction( olxstr("Extracting ") << fn );
  Progress.SetPos(0);
  Progress.SetMax(1);
  OnProgress.Enter(this, &Progress);

  IDataInputStream* rv = zip.OpenEntry(Source);
  if (rv == 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("Could not locate zip entry: ") << fn);
  }

  Progress.SetAction("Done");
//  Progress.SetPos( TEFile::FileLength(TmpFN) );
  OnProgress.Exit(this, &Progress);
  return rv;
}
//..............................................................................
bool TwxZipFileSystem::ExtractAll(const olxstr& dest) {
  return zip.ExtractAll(dest);
}
//..............................................................................
#endif // __WXWIDGETS__
