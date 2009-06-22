#ifdef  __BORLANDC__
  #pragma hdrstop
#endif

#include "filesystem.h"
#include "actions.h"
#include "bapp.h"
#include "log.h"
#include "efile.h"
#include "tptrlist.h"
#include "estack.h"

#ifdef __WXWIDGETS__
  #include "wxzipfs.h"
#elif __WIN32__
  #include "winzipfs.h"
#endif

#undef GetObject

olxstr TOSFileSystem::F__N;

TOSFileSystem::TOSFileSystem(const olxstr& base)  {
  SetBase(base);
  OnRmFile   = &Events.NewQueue("rmf");
  OnRmDir    = &Events.NewQueue("rmd");
  OnMkDir    = &Events.NewQueue("mkd");
  OnChDir    = &Events.NewQueue("chd");
  OnAdoptFile = &Events.NewQueue("af");
  OnOpenFile  = &Events.NewQueue("of");
}
//..............................................................................
bool TOSFileSystem::DelFile(const olxstr& FN)  {
  F__N = FN;
  OnRmFile->Execute( this, &F__N);
  return TEFile::DelFile(FN);
}
//..............................................................................
bool TOSFileSystem::DelDir(const olxstr& DN)  {
  F__N = DN;
  OnRmDir->Execute( this, &F__N);
  return TEFile::DeleteDir(DN);
}
//..............................................................................
bool TOSFileSystem::AdoptStream(IInputStream& f, const olxstr& name)  {
  F__N = name;
  OnAdoptFile->Execute( this, &F__N);

  try {
    olxstr path = TEFile::ExtractFilePath( name );
    if( !TEFile::Exists(path) )
      if( !TEFile::MakeDir(path) )
        if( !TEFile::MakeDirs(path) )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Mkdir \'") << path << '\'');
    TEFile destFile(name, "wb+");
    destFile << f;
  }
  catch( const TExceptionBase& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  return true;
}
//..............................................................................
bool TOSFileSystem::AdoptFile(const TFSItem& Src)  {
  F__N = GetBase() + Src.GetFullName();
  OnAdoptFile->Execute( this, &F__N);

  olxstr DFN = GetBase() + Src.GetFullName();
  // vlidate if already on the disk and with the same size and timestamp
  if( TEFile::Exists(DFN) )  {
    if( TEFile::FileLength(DFN) == Src.GetSize() && TEFile::FileAge(DFN) == Src.GetDateTime() )
      return true;
  }
  IInputStream* is = NULL;
  try  {  is = Src.GetIndexFS().OpenFile(Src.GetIndexFS().GetBase() + Src.GetFullName() );  }
  catch(const TExceptionBase& exc)  {  return false;  }
  if( is == NULL )  return false;

  try {
    olxstr path = TEFile::ExtractFilePath( DFN );
    if( !TEFile::Exists(path) )
      if( !TEFile::MakeDir(path) )
        if( !TEFile::MakeDirs(path) )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Mkdir \'") << path << '\'');
    TEFile destFile(DFN, "wb+");
    destFile << *is;
    delete is;
  }
  catch( const TExceptionBase& exc )  {
    delete is;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  TEFile::SetFileTimes( DFN, Src.GetDateTime(), Src.GetDateTime() );
  return true;
}
//..............................................................................
bool TOSFileSystem::NewDir(const olxstr& DN)  {
  F__N = DN;
  OnMkDir->Execute(this, &F__N);
  return TEFile::MakeDir(DN);
}
//..............................................................................
bool TOSFileSystem::FileExists(const olxstr& FN)  {
  return TEFile::Exists(FN);
}
//..............................................................................
IInputStream* TOSFileSystem::OpenFile(const olxstr& fileName)  {
  F__N = fileName;
  OnOpenFile->Execute(this, &F__N);
  return new TEFile( fileName, "rb");
}
//..............................................................................
bool TOSFileSystem::ChangeDir(const olxstr &DN)  {
  F__N = DN;
  OnChDir->Execute(this, &F__N);
  return TEFile::ChangeDir(DN);
}

//..............................................................................
//..............................................................................
//..............................................................................
//..............................................................................
AFileSystem& TFSItem::GetIndexFS()  const   {  return Index.IndexFS; }
//..............................................................................
AFileSystem& TFSItem::GetDestFS()  const   {  return *Index.DestFS; }
//..............................................................................
void TFSItem::Clear()  {
  for( int i=0; i < Items.Count(); i++ )
    delete Items.GetObject(i);  // destructor calls Clear()
  Items.Clear();
  Name = EmptyString;
  DateTime = 0;
  Size = 0;
}
//..............................................................................
void TFSItem::operator >> (TStrList& S) const  {
  olxstr str = olxstr::CharStr('\t', GetLevel()-1 );
  S.Add( str + GetName() );
  str << DateTime;
  str << ',' << GetSize() << ',' << '{';

  for( int i=0; i < Properties.Count(); i++ )  {
    str << Properties[i];
    if( (i+1) < Properties.Count() )  
      str << ';';
  }
  if( !Properties.IsEmpty() && !Actions.IsEmpty() )
    str << ';';
  for( int i=0; i < Actions.Count(); i++ )  {
    str << "action:" << Actions[i];
    if( (i+1) < Actions.Count() )  
      str << ';';
  }
  str << '}';

  S.Add( str );
  for( int i=0; i < Items.Count(); i++ )
    Item(i) >> S;
}
//..............................................................................
int TFSItem::ReadStrings(int& index, TFSItem* caller, TStrList& strings, const TFSItem::SkipOptions* toSkip)  {
  TStrList toks, propToks;
  while( (index + 2) <= strings.Count() )  {
    int level = strings[index].LeadingCharCount( '\t' ), 
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
          for( int i=0; i < toSkip->filesToSkip->Count(); i++ )  {
            if( (*toSkip->filesToSkip)[i].Equalsi(name) )  {
              skip = true;
              break;
            }
          }
          if( skip )  {
            index+=2;
            for( int i=index; i < strings.Count(); i+=2 )  {
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
          for( int i=0; i < toSkip->extsToSkip->Count(); i++ )  {
            if( (*toSkip->extsToSkip)[i].Equalsi(ext) )  {
              skip = true;
              break;
            }
          }
        }
        if( !skip && toSkip->filesToSkip != NULL )  {
          for( int i=0; i < toSkip->filesToSkip->Count(); i++ )  {
            if( (*toSkip->filesToSkip)[i].Equalsi(name) )  {
              skip = true;
              break;
            }
          }
        }
      }
    }  // end skip business
    if( !skip )  {
      item = &NewItem( name );
      item->SetFolder(folder);
      index++;
      toks.Strtok( strings[index], ',');
      if( toks.Count() < 2 )
        throw TInvalidArgumentException(__OlxSourceInfo, "token number");
      item->SetDateTime( toks[0].Trim('\t').RadInt<long>() );
      item->SetSize( toks[1].RadInt<long>() );
      for( int i=2; i < toks.Count(); i++ )  {
        if( toks[i].StartsFrom('{') && toks[i].EndsWith('}') )  {
          olxstr tmp = toks[i].SubString(1, toks[i].Length()-2);
          propToks.Clear();
          propToks.Strtok(tmp, ';');
          for( int j=0; j < propToks.Count(); j++ )  {
            if( propToks[j].StartsFrom("action:") )
              item->Actions.Add(propToks[j].SubStringFrom(7));
            else
              item->AddProperty( propToks[j] );
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
        int slevel = item->ReadStrings(index, this, strings, toSkip);
        if( slevel != level )
          return slevel;
      }
      if( nextlevel < level )  return  nextlevel;
    }
  }
  return 0;
}
//..............................................................................
TFSItem& TFSItem::NewItem(const olxstr& name)  {
  return *Items.Add(name, new TFSItem(Index, this, name)).Object;
}
//..............................................................................
olxstr TFSItem::GetFullName() const  {
// an alernative implementation can be done with olxstr::Insert ... to be considered
  TFSItem *FI = const_cast<TFSItem*>(this);
  olxstr Tmp;
  TStack<TFSItem*> stack;
  while( FI != NULL )  {
    stack.Push(FI);
    FI = FI->GetParent();
    if( !FI->GetParent() )  break;  //ROOT
  }
  while( !stack.IsEmpty() )  {
    Tmp << stack.Pop()->GetName();
    if( !stack.IsEmpty() )  
      Tmp << '\\';
  }
  return Tmp;
}
//..............................................................................
int TFSItem::GetLevel()  const  {
  int level = 0;
  TFSItem *FI = const_cast<TFSItem*>(this);
  while( FI && FI->GetParent() )  {  
    FI = FI->GetParent();  
    level++;  
  }
  return level;
}
//..............................................................................
void TFSItem::SetProcessed(bool V)  {
  Processed = V;
  for( int i=0; i < Count(); i++ )
    Item(i).SetProcessed(V);
}
//..............................................................................
uint64_t TFSItem::CalcTotalItemsSize(const TStrList& props) const {
  if( !IsFolder() )
    return ValidateProperties(props) ? GetSize() : 0;
  uint64_t sz = 0;
  for( int i=0; i < Count(); i++ )
    sz += Item(i).CalcTotalItemsSize(props);
  return sz;
}
//..............................................................................
double TFSItem::Synchronise(TFSItem& Dest, const TStrList& properties, TStrList* cmds, 
                            TFSItem* Caller)  
{
  if( Caller == NULL )  {
    size_t sz = CalcDiffSize(Dest, properties);
    Index.Progress.SetMax( (double)sz );
    if( sz == 0 )  return 0;  // nothing to do then ...
    Index.Progress.SetPos( 0.0 );
    Index.OnProgress->Enter(this, &Index.Progress);
  }
  /* check the repository files are at the destination - if not delete them
  (now implemented in TOSFileSystem */
  for( int i=0; i < Dest.Count(); i++ )  {
    TFSItem& FI = Dest.Item(i);
    if( Index.Break )  // temination signal
      return Index.Progress.GetPos();
    Index.Progress.SetAction( FI.GetFullName() );
    Index.OnProgress->Execute(this, &Index.Progress);

    TFSItem* Res = FindByName( FI.GetName() );
    if( Res == NULL )  {
      FI.DelFile();
      if( FI.GetParent() != NULL )  {
        if( cmds != NULL )
          cmds->Add("rm \'") << FI.GetIndexFS().GetBase() << FI.GetFullName() << '\'';
        TFSItem::Remove(FI);
      }
    }
    else  {
      Res->SetProcessed(true);
      if( !Res->ValidateProperties(properties) )
        continue;
      if( FI.IsFolder() )
        Res->Synchronise(FI, properties, cmds, this);
      else if( Dest.Index.ShallAdopt(*Res, FI) )  {
        if( FI.UpdateFile(*Res) != NULL )  {
          Index.Progress.IncPos( (double)FI.GetSize() );
          Index.OnProgress->Execute(this, &Index.Progress);
        }
      }
    }
  }
  // add new files
  for( int i=0; i < this->Count(); i++ )  {
    TFSItem& FI = Item(i);
    if( FI.IsProcessed() || !FI.ValidateProperties(properties) ) 
      continue;
    if( Index.Break )  // termination signal    
      return Index.Progress.GetPos();
    Index.Progress.SetAction( FI.GetFullName() );
    Index.OnProgress->Execute(this, &Index.Progress);
    if( FI.IsFolder() )  {
      TFSItem* Res = Dest.UpdateFile(FI);
      if( Res != NULL)
        FI.Synchronise(*Res, properties, cmds, this);
    }
    else  {
      if( Dest.UpdateFile(FI) != NULL )  {
        Index.Progress.IncPos( (double)FI.GetSize() );
        Index.OnProgress->Execute(this, &Index.Progress);
      }
    }
  }
  if( this->GetParent() == NULL )  {
    Index.OnProgress->Exit(NULL, &Index.Progress);
  }
  return Index.Progress.GetMax();
}
//..............................................................................
uint64_t TFSItem::CalcDiffSize(TFSItem& Dest, const TStrList& properties)  {
  uint64_t sz = 0;
  /* check the repository files are at the destination */
  for( int i=0; i < Dest.Count(); i++ )  {
    TFSItem& FI = Dest.Item(i);
    TFSItem* Res = FindByName( FI.GetName() );
    if( Res != NULL )  {
      Res->SetProcessed(true);
      if( !Res->ValidateProperties(properties) )
        continue;
      if( FI.IsFolder() ) 
        sz += Res->CalcDiffSize(FI, properties);
      else if( Dest.Index.ShallAdopt(*Res, FI) )
        sz += FI.GetSize();
    }
  }
  for( int i=0; i < this->Count(); i++ )  {
    TFSItem& res = Item(i);
    if( res.IsProcessed() || !res.ValidateProperties(properties) ) 
      continue;
    sz += res.CalcTotalItemsSize(properties);
  }
  return sz;
}
//..............................................................................
TFSItem* TFSItem::UpdateFile(TFSItem& item)  {
  if( item.IsFolder() )  {
    olxstr FN = GetIndexFS().GetBase() + item.GetFullName();
    if( TEFile::Exists(FN) || GetDestFS().NewDir(FN) )    {
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
        FI = FindByName( item.GetName() );
        if( FI == NULL )  {
          FI = &NewItem( item.GetName() );
          is_new = true;
        }
      }
      if( !is_new && !Index.ShallAdopt(item, *FI) )  {
        *FI = item;  // just update so it does not trigger update next time
        return FI;
      }
      if( GetDestFS().AdoptFile(item) )  {
        *FI = item;
        Index.ProcessActions(*FI);
        return FI;
      }
      else  {
        Remove(*FI);
        return NULL;
      }
    }
    catch( TExceptionBase& )  {  
      return NULL;
    }
  }
}
//..............................................................................
void TFSItem::DeleteItem(TFSItem* item)  {
  int ind = Items.IndexOfComparable( item->GetName() );
  if( ind != -1 )  {
    Items.Remove( ind );
    item->DelFile();
    delete item;
  }
  else
    throw TInvalidArgumentException(__OlxSourceInfo, "item");
}
//..............................................................................
void TFSItem::Remove(TFSItem& item)  {
  if( item.GetParent() == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot delete ROOT");
  item.GetParent()->DeleteItem( &item );
}
//..............................................................................
void TFSItem::DelFile() {
  if( IsFolder() )  {
    for( int i=0; i < Count(); i++ )
      delete Items.GetObject(i);
    Items.Clear();
    // this will remove ALL files, not only the files in the index
    GetIndexFS().DelDir( GetIndexFS().GetBase() + GetFullName() );
  }
  else
    GetIndexFS().DelFile( GetIndexFS().GetBase() + GetFullName());
}
//..............................................................................
TFSItem& TFSItem::operator = (const TFSItem& FI)  {
  Name = FI.GetName();
  Size = FI.GetSize();
  DateTime = FI.GetDateTime();
  Folder = FI.IsFolder();
  Properties = FI.Properties;
  Actions = FI.Actions;
  return *this;
}
//..............................................................................
void TFSItem::ListUniqueProperties(TStrList& uProps)  {
  for( int i=0; i < Properties.Count(); i++ )
    if( uProps.IndexOf( Properties[i] ) == -1 )
      uProps.Add( Properties[i] );
  for( int i=0; i < Items.Count(); i++ )
    Item(i).ListUniqueProperties( uProps );
}
//..............................................................................
TFSItem* TFSItem::FindByFullName(const olxstr& Name)  const {
  TStrList toks(TEFile::UnixPath(Name), '/');
  if( toks.IsEmpty() )  return NULL;

  TFSItem* root = const_cast<TFSItem*>(this);
  for(int i=0; i < toks.Count(); i++ )  {
    root = root->FindByName( toks[i] );
    if( root == NULL )  return root;
  }
  return root;
}
//..............................................................................
TFSItem& TFSItem::NewItem(TFSItem* item)  {
  TPtrList<TFSItem> items;
  while( item != NULL && item->GetParent() != NULL )  {
    items.Add( item );
    item = item->GetParent();
  }
// not in the items all path to the item
  TFSItem* ti = this;
  for( int i = items.Count()-1; i >= 0;  i-- )  {
    TFSItem* nti = ti->UpdateFile( *items[i] );
    if( nti == NULL )
      throw TFunctionFailedException( __OlxSourceInfo, "failed to update file");
    ti = nti;
  }
  return *ti;
}
//..............................................................................
void TFSItem::ClearNonexisting()  {
  if( !EsdlInstanceOf(GetIndexFS(), TOSFileSystem) ) 
    return;
  for( int i=0; i < Count(); i++ )  {
    if( !Item(i).IsFolder() )  {
      if( !GetIndexFS().FileExists(GetIndexFS().GetBase() + Item(i).GetFullName()) && 
        Index.ShouldExist(Item(i)) )  
      {
        // check if not already downloaded
        if( Index.DestFS != NULL && !Index.DestFS->FileExists(Index.DestFS->GetBase() + Item(i).GetFullName()) )
        {
          delete Items.GetObject(i);
          Items.Remove(i);
          i--;
        }
      }
    }
    else  {
      Item(i).ClearNonexisting();
      if( Item(i).IsEmpty() )  {
        delete Items.GetObject(i);
        Items.Remove(i);
        i--;
      }
    }
  }
}
//..............................................................................
void TFSItem::ClearEmptyFolders()  {
  for( int i=0; i < Count(); i++ )  {
    if( Item(i).IsFolder() )  {
      if( Item(i).Count() > 0 )
        Item(i).ClearEmptyFolders();
      if( Item(i).IsEmpty() )  {
        delete Items.GetObject(i);
        Items.Remove(i);
        i--;
      }
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
TFSIndex::TFSIndex(AFileSystem& fs) : IndexFS(fs)  {
  Root = new TFSItem(*this, NULL, "ROOT");
  OnProgress = &Actions.NewQueue("ON_PROGRESS");
  DestFS = NULL;
  IndexLoaded = false;
  Break = false;
}
//..............................................................................
TFSIndex::~TFSIndex()  {
  delete Root;
}
//..............................................................................
void TFSIndex::LoadIndex(const olxstr& IndexFile, const TFSItem::SkipOptions* toSkip)  {
  if( IndexLoaded )
    return;
  GetRoot().Clear();
  if( !IndexFS.FileExists(IndexFile) )
    throw TFileDoesNotExistException(__OlxSourceInfo, IndexFile);

  IInputStream* is = NULL;
  try {  is = IndexFS.OpenFile(IndexFile);  }
  catch( TExceptionBase &exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc.GetException()->GetFullMessage() );
  }
  if( is == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "could not load index file" );
  TStrList strings;
  strings.LoadFromTextStream( *is );
  delete is;
  int index = 0;
  GetRoot().ReadStrings(index, NULL, strings, toSkip);
  Properties.Clear();
  GetRoot().ListUniqueProperties( Properties );
  GetRoot().ClearNonexisting();
  IndexLoaded = true;
}
//..............................................................................
void TFSIndex::SaveIndex(const olxstr &IndexFile)  {
  TStrList strings;

  GetRoot().ClearEmptyFolders();

  for( int i=0; i < GetRoot().Count(); i++ )
    GetRoot().Item(i) >> strings;
  TEFile* tmp_f = TEFile::TmpFile(EmptyString);
  TCStrList(strings).SaveToTextStream(*tmp_f);
  tmp_f->SetPosition(0);
  try  {  IndexFS.AdoptStream(*tmp_f, IndexFile);  }
  catch(const TExceptionBase& exc)  {
    delete tmp_f;
    throw TFunctionFailedException(__OlxSourceInfo, exc, "could not save index");
  }
  delete tmp_f;
}
//..............................................................................
double TFSIndex::Synchronise(AFileSystem& To, const TStrList& properties, const TFSItem::SkipOptions* toSkip,
                             AFileSystem* dest_fs, TStrList* cmds, const olxstr& indexName)  
{
  TFSIndex DestI(To);
  Break = false;
  DestI.DestFS = (dest_fs == NULL ? &To : dest_fs);
  olxstr SrcInd = IndexFS.GetBase() + indexName;
  olxstr DestInd = To.GetBase() + indexName;
  try  {
    LoadIndex(SrcInd, toSkip);
    if( To.FileExists(DestInd) )
      DestI.LoadIndex(DestInd);
    GetRoot().SetProcessed(false);
    double BytesTransfered = GetRoot().Synchronise(DestI.GetRoot(), properties, cmds);
    if( BytesTransfered != 0 )
      DestI.SaveIndex(DestInd);
    return BytesTransfered;
  }
  catch( const TExceptionBase& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
}
//..............................................................................
uint64_t TFSIndex::CalcDiffSize(AFileSystem& To, const TStrList& properties, const TFSItem::SkipOptions* toSkip,
                                const olxstr& indexName)  
{
  TFSIndex DestI(To);
  olxstr SrcInd = IndexFS.GetBase() + indexName;
  olxstr DestInd = To.GetBase() + indexName;
  try  {
    LoadIndex(SrcInd, toSkip);
    if( To.FileExists(DestInd) )
      DestI.LoadIndex(DestInd);
    GetRoot().SetProcessed(false);
    return GetRoot().CalcDiffSize(DestI.GetRoot(), properties);
  }
  catch( const TExceptionBase& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
}
//..............................................................................
bool TFSIndex::UpdateFile(AFileSystem& To, const olxstr& fileName, bool Force, const olxstr& indexName)  {
  TFSIndex DestI(To);
  DestI.DestFS = &To;
  olxstr SrcInd = IndexFS.GetBase() + indexName;
  olxstr DestInd = To.GetBase() + indexName;
  bool res = false;
  try  {
    LoadIndex(SrcInd);
    if( To.FileExists(DestInd) )
      DestI.LoadIndex(DestInd);

    TFSItem* src = GetRoot().FindByFullName(fileName);
    if( src == NULL )
      throw TFileDoesNotExistException( __OlxSourceInfo, fileName );
    TFSItem* dest = DestI.GetRoot().FindByFullName(fileName);
    if( dest != NULL )  {
      if( Force || (src->GetDateTime() > dest->GetDateTime()) ||
        !To.FileExists( To.GetBase() + dest->GetFullName()) )  {
          olxstr test = To.GetBase() + dest->GetFullName();
        if( To.AdoptFile( *src ) )  {
          *dest = *src;
          res = true;
        }
      }
    }
    else  {
      DestI.GetRoot().NewItem( src );
      //To.AdoptFile( *src );
      res = true;
    }
    if( res )
      DestI.SaveIndex( DestInd );
  }
  catch( const TExceptionBase& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  return res;
}
//..............................................................................
bool TFSIndex::ShallAdopt(const TFSItem& src, const TFSItem& dest) const  {
  if( src.GetActions().IndexOfi("delete") != -1 )
    return !(dest.GetDateTime() == src.GetDateTime() && dest.GetSize() == src.GetSize());
  if( dest.GetDateTime() != src.GetDateTime() || 
    !dest.GetIndexFS().FileExists(dest.GetIndexFS().GetBase() + dest.GetFullName()) )  
  {
    // validate if not already downlaoded
    if( &dest.GetDestFS() != NULL )
      return !dest.GetDestFS().FileExists(dest.GetDestFS().GetBase() + dest.GetFullName());  
    return true;
  }
  return false;
}
//..............................................................................
void TFSIndex::ProcessActions(TFSItem& item)  {
  const TStrList& actions = item.GetActions();
#ifdef __WXWIDGETS__
  typedef TwxZipFileSystem ZipFS;
#elif __WIN32__
  typedef TWinZipFileSystem ZipFS;
#endif
  if( actions.IndexOfi("extract") != -1 )  {
    ZipFS zp( DestFS->GetBase() + item.GetFullName() );
    zp.ExtractAll( DestFS->GetBase() );
  }
  if( actions.IndexOfi("delete") != -1 )
      item.DelFile();
}
//..............................................................................

