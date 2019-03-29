/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "filesystem.h"
#include "actions.h"
#include "bapp.h"
#include "log.h"
#include "efile.h"
#include "tptrlist.h"
#include "estack.h"
#include "md5.h"
#include "zipfs.h"
#include <sys/stat.h>

#undef GetObject

TOSFileSystem::TOSFileSystem(const olxstr& base) :
  OnRmFile(Events.New("rmf")),
  OnRmDir(Events.New("rmd")),
  OnMkDir(Events.New("mkd")),
  OnAdoptFile(Events.New("af")),
  OnOpenFile(Events.New("of"))
{
  SetBase(base);
}
//.............................................................................
bool TOSFileSystem::_DoDelFile(const olxstr& FN)  {
  OnRmFile.Execute(this, &FN);
  return TEFile::DelFile(FN);
}
//.............................................................................
bool TOSFileSystem::_DoDelDir(const olxstr& DN)  {
  OnRmDir.Execute(this, &DN);
  return TEFile::DeleteDir(DN);
}
//.............................................................................
bool TOSFileSystem::_DoAdoptStream(IInputStream& f, const olxstr& name)  {
  OnAdoptFile.Execute(this, &name);
  try {
    olxstr path = TEFile::ExtractFilePath( name );
    if( !TEFile::Exists(path) )
      if( !TEFile::MakeDir(path) )
        if( !TEFile::MakeDirs(path) )  {
          TBasicApp::NewLogEntry(logError) << "MKdir failed on \'" << path
            << "\'";
          return false;
        }
    {  // make sure file gets closed etc
      TEFile destFile(name+".tmp", "wb+");
      destFile << f;
    }
    TEFile::Rename(name+".tmp", name);
    return true;
  }
  catch( const TExceptionBase& )  {  return false;  }
}
//.............................................................................
bool TOSFileSystem::_DoAdoptFile(const TFSItem& Src) {
  const olxstr _fn = GetBase() + Src.GetFullName();
  OnAdoptFile.Execute(this, &_fn);

  olxstr DFN = GetBase() + Src.GetFullName();
  // vlidate if already on the disk and with the same size and timestamp
  if (TEFile::Exists(DFN)) {
    if (TEFile::FileLength(DFN) == Src.GetSize()) {
      TEFile f = TEFile(DFN, "rb");
      if (MD5::Digest(f) == Src.GetDigest())
        return true;
    }
  }
  olx_object_ptr<IInputStream> is;
  try {
    is = Src.GetIndexFS().OpenFile(
      Src.GetIndexFS().GetBase() + Src.GetFullName());
  }
  catch(const TExceptionBase &) {
    return false;
  }
  if (!is.is_valid())  return false;

  try {
    olxstr path = TEFile::ExtractFilePath(DFN);
    if (!TEFile::Exists(path)) {
      if (!TEFile::MakeDir(path)) {
        if (!TEFile::MakeDirs(path)) {
          TBasicApp::NewLogEntry(logError) << "MKdir failed on \'" << path
            << "\'";
          return false;
        }
      }
    }
    TEFile destFile(DFN, "wb+");
    destFile << is();
    destFile.SetPosition(0);
    if (MD5::Digest(destFile) != Src.GetDigest()) {
      TBasicApp::NewLogEntry(logError) << "Digest mismatch for " << DFN <<
        ", skipping";
      destFile.Delete();
      return false;
    }
  }
  catch (const TExceptionBase& exc) {
    TBasicApp::NewLogEntry(logException) <<
      exc.GetException()->GetFullMessage();
    return false;
  }
  TEFile::SetFileTimes(DFN, Src.GetDateTime(), Src.GetDateTime());
  return true;
}
//.............................................................................
bool TOSFileSystem::_DoNewDir(const olxstr& DN)  {
  OnMkDir.Execute(this, &DN);
  return TEFile::MakeDirs(DN);
}
//.............................................................................
bool TOSFileSystem::_DoesExist(const olxstr& FN, bool)  {
  return TEFile::Exists(FN);
}
IInputStream* TOSFileSystem::_DoOpenFile(const olxstr& fileName)  {
  OnOpenFile.Execute(this, &fileName);
  return new TEFile(fileName, "rb");
}
//.............................................................................
//.............................................................................
//.............................................................................
bool TUpdateFS::_DoesExist(const olxstr& df, bool) {
  if (TOSFileSystem::_DoesExist(df, true)) return true;
  return Proxied.Exists(
    Proxied.GetBase() + df.SubStringFrom(GetBase().Length()));
}
//.............................................................................
IInputStream* TUpdateFS::_DoOpenFile(const olxstr& df) {
  if (TOSFileSystem::_DoesExist(df, true))
    return TOSFileSystem::_DoOpenFile(df);
  return Proxied.OpenFile(
    Proxied.GetBase() + df.SubStringFrom(GetBase().Length()));
}
//.............................................................................
//.............................................................................
//.............................................................................
AFileSystem& TFSItem::GetIndexFS() const  {  return Index.IndexFS; }
//.............................................................................
void TFSItem::Clear()  {
  for( size_t i=0; i < Items.Count(); i++ )
    delete Items.GetValue(i);  // destructor calls Clear()
  Items.Clear();
  Name.SetLength(0);
  DateTime = 0;
  Size = 0;
}
//.............................................................................
size_t TFSItem::UpdateDigest()  {
  if( !Digest.IsEmpty() )  return 0;
  if( IsFolder() )  {
    size_t rv = 0;
    for( size_t i=0; i < Items.Count(); i++ )
      rv += Items.GetValue(i)->UpdateDigest();
    return rv;
  }
  try  {
    olxstr fn = Index.IndexFS.GetBase() + GetFullName();
    if( !Index.IndexFS.Exists(fn) )  return 0;
    IInputStream* is = Index.IndexFS.OpenFile(fn);
    if( is == NULL )  return 0;
    Digest = MD5::Digest(*is);
    delete is;
    return 1;
  }
  catch(...)  {  return 0;  }
}
//.............................................................................
void TFSItem::operator >> (TStrList& S) const  {
  olxstr str = olxstr::CharStr('\t', GetLevel()-1);
  S.Add( str + GetName() );
  str << DateTime;
  str << ',' << GetSize() << ',' << GetDigest() << ",{";
  for( size_t i=0; i < Properties.Count(); i++ )  {
    str << Properties[i];
    if( (i+1) < Properties.Count() )
      str << ';';
  }
  if( !Properties.IsEmpty() && !Actions.IsEmpty() )
    str << ';';
  for( size_t i=0; i < Actions.Count(); i++ )  {
    str << "action:" << Actions[i];
    if( (i+1) < Actions.Count() )
      str << ';';
  }
  str << '}';

  S.Add( str );
  for( size_t i=0; i < Items.Count(); i++ )
    Item(i) >> S;
}
//.............................................................................
size_t TFSItem::ReadStrings(size_t& index, TFSItem* caller, TStrList& strings,
  const TFSItem::SkipOptions* toSkip)
  {
  TStrList toks, propToks;
  while( (index + 2) <= strings.Count() )  {
    size_t level = strings[index].LeadingCharCount('\t'),
        nextlevel = 0;
    olxstr name( strings[index].Trim('\t') ),
           ext( TEFile::ExtractFileExt(name) );
    bool skip = false, folder = false;
    TFSItem* item = NULL;
    if( (index+2) < strings.Count() )  {
      nextlevel = strings[index+2].LeadingCharCount('\t');
      if( nextlevel > level )
        folder = true;
    }
    if( toSkip != NULL )  {  // skip business
      if( folder )  {
        if( toSkip->filesToSkip != NULL )  {
          for( size_t i=0; i < toSkip->filesToSkip->Count(); i++ )  {
            if( (*toSkip->filesToSkip)[i].Equalsi(name) )  {
              skip = true;
              break;
            }
          }
          if( skip )  {
            index+=2;
            for( size_t i=index; i < strings.Count(); i+=2 )  {
              nextlevel = strings[i].LeadingCharCount('\t');
              if( nextlevel <= level )  {
                index = i;
                break;
              }
            }
            if( nextlevel > level )  //reached the end
              return 0;
            if( nextlevel < level )
              return  nextlevel;
            continue;
          }
        }
      }
      else  {
        if( toSkip->extsToSkip != NULL && !ext.IsEmpty() )  {
          for( size_t i=0; i < toSkip->extsToSkip->Count(); i++ )  {
            if( (*toSkip->extsToSkip)[i].Equalsi(ext) )  {
              skip = true;
              break;
            }
          }
        }
        if( !skip && toSkip->filesToSkip != NULL )  {
          for( size_t i=0; i < toSkip->filesToSkip->Count(); i++ )  {
            if( (*toSkip->filesToSkip)[i].Equalsi(name) )  {
              skip = true;
              break;
            }
          }
        }
      }
    }  // end skip business
    if( !skip )  {
      item = &NewItem(name);
      item->SetFolder(folder);
      index++;
      toks.Strtok(strings[index], ',', false);
      if( toks.Count() < 2 )
        throw TInvalidArgumentException(__OlxSourceInfo, "token number");
      item->SetDateTime( toks[0].Trim('\t').RadInt<int64_t>() );
      item->SetSize(toks[1].RadInt<int64_t>());
      int start = 2;
      if( toks.Count() > 3 )  {
        item->SetDigest(toks[2]);
        start++;
      }
      for( size_t i=start; i < toks.Count(); i++ )  {
        if( toks[i].StartsFrom('{') && toks[i].EndsWith('}') )  {
          olxstr tmp = toks[i].SubString(1, toks[i].Length()-2);
          propToks.Clear();
          propToks.Strtok(tmp, ';');
          for( size_t j=0; j < propToks.Count(); j++ )  {
            if( propToks[j].StartsFrom("action:") )
              item->Actions.Add(propToks[j].SubStringFrom(7));
            else
              item->AddProperty(propToks[j]);
          }
        }
      }
      toks.Clear();
    }
    else
      index++;
    index++;
    if( index < strings.Count() )  {
      if( folder )  {
        size_t slevel = item->ReadStrings(index, this, strings, toSkip);
        if( slevel != level )
          return slevel;
      }
      if( nextlevel < level )  return  nextlevel;
    }
  }
  return 0;
}
//.............................................................................
TFSItem& TFSItem::NewItem(const olxstr& name)  {
  return *Items.Add(name, new TFSItem(Index, this, name));
}
//.............................................................................
olxstr TFSItem::GetFullName() const {
  if (this->GetParent() == NULL)
    return EmptyString();
  olxstr_buf res = GetName();
  olxstr ds = '\\';
  const TFSItem *p = GetParent();
  while( p->GetParent() != NULL )  {
    res << ds << p->GetName();
    p = p->GetParent();
  }
  return olxstr::FromExternal(
          res.ReverseRead(olx_malloc<olxch>(res.Length()+1)), res.Length());
}
//.............................................................................
int TFSItem::GetLevel() const {
  int level = 0;
  TFSItem *FI = const_cast<TFSItem*>(this);
  while( FI && FI->GetParent() )  {
    FI = FI->GetParent();
    level++;
  }
  return level;
}
//.............................................................................
void TFSItem::SetProcessed(bool V) const {
  Processed = V;
  for( size_t i=0; i < Count(); i++ )
    Item(i).SetProcessed(V);
}
//.............................................................................
uint64_t TFSItem::CalcTotalItemsSize(const TStrList& props) const {
  // some folders like for plugins might have properties....
  if( !ValidateProperties(props) )  return 0;
  if( !IsFolder() )  return GetSize();
  uint64_t sz = 0;
  for( size_t i=0; i < Count(); i++ )
    sz += Item(i).CalcTotalItemsSize(props);
  return sz;
}
//.............................................................................
uint64_t TFSItem::Synchronise(TFSItem& Dest, const TStrList& properties,
  TStrList* cmds)
{
  if( Parent == NULL )  {
    uint64_t sz = CalcDiffSize(Dest, properties);
    Index.Progress.SetMax(sz);
    if( sz == 0 )  return 0;  // nothing to do then ...
    Index.Progress.SetPos(0);
    Index.OnProgress.Enter(this, &Index.Progress);
    SetProcessed(false);
  }
  /* check the repository files are at the destination - if not delete them
  (now implemented in TOSFileSystem */
  for( size_t i=0; i < Dest.Count(); i++ )  {
    TFSItem& FI = Dest.Item(i);
    if( Index.Break )  // temination signal
      return Index.Progress.GetPos();
    Index.Progress.SetAction(FI.GetFullName());
    Index.OnProgress.Execute(this, &Index.Progress);

    TFSItem* Res = FindByName(FI.GetName());
    if( Res == NULL )  {
      if( FI.GetParent() != NULL )  {
        if( cmds != NULL ) {
          cmds->Add("rm \'") << FI.GetIndexFS().GetBase() <<
            FI.GetFullName() << '\'';
        }
        TFSItem::Remove(FI);
      }
    }
    else  {
      Res->Processed = true;
      if( !Res->ValidateProperties(properties) )
        continue;
      if( FI.IsFolder() )
        Res->Synchronise(FI, properties, cmds);
      else if( Dest.Index.ShallAdopt(*Res, FI) )  {
        if( FI.UpdateFile(*Res) != NULL )  {
          Index.Progress.IncPos(FI.GetSize());
          Index.OnProgress.Execute(this, &Index.Progress);
        }
      }
    }
  }
  // add new files
  for( size_t i=0; i < this->Count(); i++ )  {
    TFSItem& FI = Item(i);
    if( FI.IsProcessed() || !FI.ValidateProperties(properties) )
      continue;
    if( Index.Break )  // termination signal
      return Index.Progress.GetPos();
    Index.Progress.SetAction(FI.GetFullName());
    Index.OnProgress.Execute(this, &Index.Progress);
    if( FI.IsFolder() )  {
      TFSItem* Res = Dest.UpdateFile(FI);
      if( Res != NULL)
        FI.Synchronise(*Res, properties, cmds);
    }
    else  {
      if( Dest.UpdateFile(FI) != NULL )  {
        Index.Progress.IncPos(FI.GetSize());
        Index.OnProgress.Execute(this, &Index.Progress);
      }
    }
  }
  if( this->GetParent() == NULL )  {
    Index.OnProgress.Exit(NULL, &Index.Progress);
  }
  return Index.Progress.GetMax();
}
//.............................................................................
uint64_t TFSItem::CalcDiffSize(const TFSItem& Dest,
  const TStrList& properties) const
{
  uint64_t sz = 0;
  if( Parent == NULL )
    SetProcessed(false);
  /* check the repository files are at the destination */
  for( size_t i=0; i < Dest.Count(); i++ )  {
    TFSItem& FI = Dest.Item(i);
    TFSItem* Res = FindByName(FI.GetName());
    if( Res != NULL )  {
      Res->Processed = true;
      if( !Res->ValidateProperties(properties) )
        continue;
      if( FI.IsFolder() )
        sz += Res->CalcDiffSize(FI, properties);
      else if( Dest.Index.ShallAdopt(*Res, FI) )  {
        sz += Res->GetSize();
      }
    }
  }
  for( size_t i=0; i < this->Count(); i++ )  {
    TFSItem& res = Item(i);
    if( res.IsProcessed() || !res.ValidateProperties(properties) )
      continue;
    sz += res.CalcTotalItemsSize(properties);
  }
  return sz;
}
//.............................................................................
TFSItem* TFSItem::UpdateFile(TFSItem& item)  {
  if( item.IsFolder() )  {
    olxstr FN = GetIndexFS().GetBase() + item.GetFullName();
    if( TEFile::Exists(FN) || GetIndexFS().NewDir(FN) )    {
      TFSItem* FI = FindByName(item.GetName());
      if( FI == NULL )
        FI = &NewItem(item.GetName());
      *FI = item;
      return FI;
    }
    return NULL;
  }
  else  {
    // an error can be only caused by impossibility to open the file
    try  {
      TFSItem* FI = const_cast<TFSItem*>(this);
      // if names are different - we are looking at the content of this
      bool is_new = false;
      if( this->GetName() != item.GetName() )  {
        FI = FindByName(item.GetName());
        if( FI == NULL )  {
          FI = &NewItem(item.GetName());
          is_new = true;
        }
      }
      if( !is_new && !Index.ShallAdopt(item, *FI) )  {
        *FI = item;  // just update so it does not trigger update next time
        return FI;
      }
      if( GetIndexFS().AdoptFile(item) )  {
        TFSItem tmp(Index, this, EmptyString());
        tmp = *FI;
        *FI = item;
        bool res = Index.ProcessActions(*FI);
        if( !res )
          *FI = tmp;
        return res ? FI : NULL;
      }
      else  {
        Remove(*FI);
        return NULL;
      }
    }
    catch( const TExceptionBase& )  {
      return NULL;
    }
  }
}
//.............................................................................
void TFSItem::DeleteItem(TFSItem* item) {
  if (Items.Remove(item->GetName())) {
    item->DelFile();
    delete item;
  }
  else
    throw TInvalidArgumentException(__OlxSourceInfo, "item");
}
//.............................................................................
void TFSItem::Remove(TFSItem& item)  {
  if( !item.GetIndexFS().HasAccess(afs_DeleteAccess) )
    return;
  if( item.GetParent() == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot delete ROOT");
  item.GetParent()->DeleteItem(&item);
}
//.............................................................................
void TFSItem::DelFile() {
  if (!GetIndexFS().HasAccess(afs_DeleteAccess)) return;
  if (IsFolder()) {
    for (size_t i=0; i < Count(); i++)
      delete Items.GetValue(i);
    Items.Clear();
    // this will remove ALL files, not only the files in the index
    GetIndexFS().DelDir(GetIndexFS().GetBase() + GetFullName());
  }
  else {
    GetIndexFS().DelFile(GetIndexFS().GetBase()+ GetFullName());
  }
}
//.............................................................................
void TFSItem::_DelFile() {
  if (!olx_type<TUpdateFS>::check(GetIndexFS()) || IsFolder()) {
    return;
  }
  olxstr fn = GetIndexFS().GetBase()+ GetFullName();
  if (TEFile::Exists(fn)) {
    TEFile::Chmod(fn, S_IWRITE);
    TEFile::DelFile(fn);
  }
}
//.............................................................................
TFSItem& TFSItem::operator = (const TFSItem& FI)  {
  Name = FI.GetName();
  Size = FI.GetSize();
  DateTime = FI.GetDateTime();
  Digest = FI.Digest;
  Folder = FI.IsFolder();
  Properties = FI.Properties;
  Actions = FI.Actions;
  return *this;
}
//.............................................................................
void TFSItem::ListUniqueProperties(TStrList& uProps)  {
  for( size_t i=0; i < Properties.Count(); i++ )
    if( uProps.IndexOf( Properties[i] ) == InvalidIndex )
      uProps.Add( Properties[i] );
  for( size_t i=0; i < Items.Count(); i++ )
    Item(i).ListUniqueProperties( uProps );
}
//.............................................................................
TFSItem* TFSItem::FindByFullName(const olxstr& Name) const {
  TStrList toks(TEFile::UnixPath(Name), '/');
  if( toks.IsEmpty() )  return NULL;

  const TFSItem* root = this;
  for( size_t i=0; i < toks.Count(); i++ )  {
    root = root->FindByName( toks[i] );
    if( root == NULL )  return const_cast<TFSItem*>(root);
  }
  return const_cast<TFSItem*>(root);
}
//.............................................................................
TFSItem& TFSItem::NewItem(TFSItem* item)  {
  TPtrList<TFSItem> items;
  while( item != NULL && item->GetParent() != NULL )  {
    items.Add(item);
    item = item->GetParent();
  }
// not in the items all path to the item
  TFSItem* ti = this;
  for( size_t i = items.Count(); i > 0;  i-- )  {
    TFSItem* nti = ti->UpdateFile(*items[i-1]);
    if( nti == NULL )
      throw TFunctionFailedException( __OlxSourceInfo, "failed to update file");
    ti = nti;
  }
  return *ti;
}
//.............................................................................
void TFSItem::ClearNonexisting()  {
  if (!olx_type<TOSFileSystem>::check(GetIndexFS())) {
    return;
  }
  for( size_t i=0; i < Count(); i++ )  {
    if( !Item(i).IsFolder() )  {
      if( Index.ShouldExist(Item(i)) &&
        !GetIndexFS().Exists(GetIndexFS().GetBase() + Item(i).GetFullName()) )
      {
        delete Items.GetValue(i);
        Items.Delete(i--);
      }
    }
    else  {
      Item(i).ClearNonexisting();
      if( Item(i).IsEmpty() )  {
        delete Items.GetValue(i);
        Items.Delete(i--);
      }
    }
  }
}
//.............................................................................
void TFSItem::ClearEmptyFolders()  {
  for( size_t i=0; i < Count(); i++ )  {
    if( Item(i).IsFolder() )  {
      if( Item(i).Count() > 0 )
        Item(i).ClearEmptyFolders();
      if( Item(i).IsEmpty() )  {
        delete Items.GetValue(i);
        Items.Delete(i);
        i--;
      }
    }
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
TFSIndex::TFSIndex(AFileSystem& fs) :
  IndexFS(fs),
  OnBreak(Actions.New("ON_BREAK")),
  OnProgress(Actions.New("ON_PROGRESS")),
  OnAction(Actions.New("ON_ACTION"))
{
  Root = new TFSItem(*this, NULL, "ROOT");
  IndexLoaded = false;
  Break = false;
  OnBreak.Add(&IndexFS);
}
//.............................................................................
TFSIndex::~TFSIndex()  {  delete Root;  }
//.............................................................................
void TFSIndex::LoadIndex(const olxstr& IndexFile,
  const TFSItem::SkipOptions* toSkip)
{
  if( IndexLoaded )  return;
  GetRoot().Clear();
  // just try instead...
  //if( !IndexFS.Exists(IndexFile) )
  //  throw TFileDoesNotExistException(__OlxSourceInfo, IndexFile);

  olx_object_ptr<IInputStream> is;
  try  { is = IndexFS.OpenFile(IndexFile); }
  catch (TExceptionBase &exc) {
    throw TFunctionFailedException(__OlxSourceInfo,
      exc.GetException()->GetFullMessage());
  }
  if (!is.is_valid()) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "could not load index file");
  }
  TStrList strings;
  strings.LoadFromTextStream(is());
  size_t index = 0;
  GetRoot().ReadStrings(index, NULL, strings, toSkip);
  Properties.Clear();
  GetRoot().ListUniqueProperties(Properties);
  GetRoot().ClearNonexisting();
  IndexFS.SetIndex(this);
  IndexLoaded = true;
}
//.............................................................................
void TFSIndex::SaveIndex(const olxstr &IndexFile) {
  TStrList strings;
  GetRoot().ClearEmptyFolders();
  for( size_t i=0; i < GetRoot().Count(); i++ )
    GetRoot().Item(i) >> strings;
  olx_object_ptr<TEFile> tmp_f(TEFile::TmpFile(EmptyString()));
  TCStrList(strings).SaveToTextStream(tmp_f());
  tmp_f().SetPosition(0);
  try { IndexFS.AdoptStream(tmp_f(), IndexFile); }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc,
      "could not save index");
  }
}
//.............................................................................
uint64_t TFSIndex::Synchronise(AFileSystem& To, const TStrList& properties,
  const TFSItem::SkipOptions* toSkip,TStrList* cmds,
  const olxstr& indexName)
{
  TFSIndex DestI(To);
  OnBreak.Add(&DestI.OnBreak);
  Break = false;
  olxstr SrcInd = IndexFS.GetBase() + indexName;
  olxstr DestInd = To.GetBase() + indexName;
  try  {
    LoadIndex(SrcInd, toSkip);
    if( To.Exists(DestInd) )
      DestI.LoadIndex(DestInd);
    // proxy events ...
    DestI.OnAction.Add( new TActionProxy(OnAction) );
    uint64_t BytesTransfered = GetRoot().Synchronise(
      DestI.GetRoot(), properties, cmds);
    if( BytesTransfered != 0 )
      DestI.SaveIndex(DestInd);
    OnBreak.Remove(&DestI.OnBreak);
    return BytesTransfered;
  }
  catch( const TExceptionBase& exc )  {
    OnBreak.Remove(&DestI.OnBreak);
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
}
//.............................................................................
uint64_t TFSIndex::CalcDiffSize(AFileSystem& To, const TStrList& properties,
  const TFSItem::SkipOptions* toSkip, const olxstr& indexName)
{
  TFSIndex DestI(To);
  olxstr SrcInd = IndexFS.GetBase() + indexName;
  olxstr DestInd = To.GetBase() + indexName;
  try  {
    LoadIndex(SrcInd, toSkip);
    if( To.Exists(DestInd) )
      DestI.LoadIndex(DestInd);
    return GetRoot().CalcDiffSize(DestI.GetRoot(), properties);
  }
  catch( const TExceptionBase& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
}
//.............................................................................
bool TFSIndex::UpdateFile(AFileSystem& To, const olxstr& fileName, bool Force,
  const olxstr& indexName)
{
  TFSIndex DestI(To);
  OnBreak.Add(&DestI.OnBreak);
  olxstr SrcInd = IndexFS.GetBase() + indexName;
  olxstr DestInd = To.GetBase() + indexName;
  bool res = false;
  try  {
    LoadIndex(SrcInd);
    if( To.Exists(DestInd) )
      DestI.LoadIndex(DestInd);

    TFSItem* src = GetRoot().FindByFullName(fileName);
    if( src == NULL )
      throw TFileDoesNotExistException(__OlxSourceInfo, fileName);
    TFSItem* dest = DestI.GetRoot().FindByFullName(fileName);
    if( dest != NULL )  {
      bool update = Force;
      if( !update  )  {
        if( !src->GetDigest().IsEmpty() && !dest->GetDigest().IsEmpty() )
          update = src->GetDigest() != dest->GetDigest();
        else {
          update = (src->GetDateTime() > dest->GetDateTime()) ||
            (src->GetSize() != dest->GetSize());
        }
      }
      if( update )  {
        olxstr test = To.GetBase() + dest->GetFullName();
        if( To.AdoptFile(*src) )  {
          *dest = *src;
          res = true;
        }
      }
    }
    else  {
      DestI.GetRoot().NewItem(src);
      res = true;
    }
    if( res )
      DestI.SaveIndex(DestInd);
    OnBreak.Remove(&DestI.OnBreak);
  }
  catch( const TExceptionBase& exc )  {
    OnBreak.Remove(&DestI.OnBreak);
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  return res;
}
//.............................................................................
bool TFSIndex::ShallAdopt(const TFSItem& src, TFSItem& dest) const  {
  if( !dest.GetDigest().IsEmpty() && !src.GetDigest().IsEmpty() )  {
    if( src.GetActions().IndexOfi("delete") != InvalidIndex )
      return !(dest.GetDigest() == src.GetDigest() &&
        dest.GetSize() == src.GetSize());
    if( dest.GetDigest() != src.GetDigest() ||
      !dest.GetIndexFS().Exists(
        dest.GetIndexFS().GetBase() + dest.GetFullName()) )
    {
      return true;
    }
  }
  else  {  // do it the old way, based on timestamp then, but update the digest
    dest.SetDigest(src.GetDigest());
    if( src.GetActions().IndexOfi("delete") != InvalidIndex )
      return !(dest.GetDateTime() == src.GetDateTime() &&
        dest.GetSize() == src.GetSize());
    if( dest.GetDateTime() != src.GetDateTime() ||
      !dest.GetIndexFS().Exists(
        dest.GetIndexFS().GetBase() + dest.GetFullName()) )
    {
      return true;
    }
  }
  return false;
}
//.............................................................................
bool TFSIndex::ProcessActions(TFSItem& item)  {
  const TStrList& actions = item.GetActions();
  bool res= true;
  if( actions.IndexOfi("extract") != InvalidIndex )  {
    olx_object_ptr<AZipFS> fs(ZipFSFactory::GetInstance(
      IndexFS.GetBase() + item.GetFullName(), false));
    fs().OnProgress.Add(new TActionProxy(OnAction));
    OnBreak.Add(&fs());
    try  {  res = fs().ExtractAll(IndexFS.GetBase());  }
    catch(...)  {  res = false;  }
    OnBreak.Remove(&fs());
  }
  if( res && actions.IndexOfi("delete") != InvalidIndex ) {
    item._DelFile();
  }
  return res;
}
//.............................................................................
