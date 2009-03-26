#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "library.h"
//---------------------------------------------------------------------------
UseEsdlNamespace()

TLibrary::TLibrary(const olxstr& libraryName, ALibraryContainer* owner)  {
  LibraryName  = libraryName;
  LibraryOwner = owner;
  ParentLibrary = NULL;
}
//..............................................................................
TLibrary::~TLibrary()  {
  for( int i=0; i < Functions.Count(); i++ )
    delete Functions.GetObject(i);
  for( int i=0; i < Macros.Count(); i++ )
    delete Macros.GetObject(i);
  for( int i=0; i < Libraries.Count(); i++ )
    delete Libraries.GetObject(i);
}
//..............................................................................
TLibrary* TLibrary::AddLibrary(const olxstr& name, ALibraryContainer* owner )  {
  if( Libraries.IndexOfComparable( name ) != -1 )
    throw TDuplicateEntry(__OlxSourceInfo, name, "library" );
  TLibrary* lib = new TLibrary(name, owner);
  Libraries.Add(name, lib);
  lib->SetParentLibrary( this );
  return lib;
}
//..............................................................................
void TLibrary::AttachLibrary( TLibrary* lib )  {
  if( Libraries.IndexOfComparable( lib->GetName() ) != -1 )
    throw TDuplicateEntry(__OlxSourceInfo, lib->GetName(), "library" );
  Libraries.Add(lib->GetName(), lib);
  lib->SetParentLibrary( this );
}
//..............................................................................
void TLibrary::RegisterStaticMacro( TStaticMacro* func, bool replace )  {
  TIntList list;
  Macros.GetIndexes( func->GetName(), list );
  for(int i=0; i < list.Count(); i++ )  {
    unsigned int argc = Macros.GetObject(list[i])->GetArgStateMask();
    if( func->GetArgStateMask() & argc )  {
      if( replace )  {
        Macros.Delete(list[i]);
        break;
      }
      throw TDuplicateEntry(__OlxSourceInfo, olxstr("macro (same number of args)") << func->GetName(), "static macro");
    }
  }
  func->SetParentLibrary( *this );
  Macros.Add(func->GetName(), func);
}
//..............................................................................
void TLibrary::RegisterStaticFunction( TStaticFunction* func, bool replace )  {
  TIntList list;
  Functions.GetIndexes( func->GetName(), list );
  for(int i=0; i < list.Count(); i++ )  {
    unsigned int argc = Functions.GetObject(list[i])->GetArgStateMask();
    if( func->GetArgStateMask() & argc )  {
      if( replace )  {
        Functions.Delete(list[i]);
        break;
      }
      throw TDuplicateEntry(__OlxSourceInfo, olxstr("function (same number of args)") << func->GetName(), "static function");
    }
  }
  func->SetParentLibrary( *this );
  Functions.Add(func->GetName(), func);
}
//..............................................................................
size_t TLibrary::LocateLocalFunctions( const olxstr& name, TBasicFunctionPList& store)  {
  TIntList list;
  Functions.GetIndexes(name, list);
  for(int i=0; i < list.Count(); i++ )
    store.Add(  Functions.GetObject(list[i]) );
  return list.Count();
}
//..............................................................................
size_t TLibrary::LocateLocalMacros( const olxstr& name, TBasicFunctionPList& store)  {
  TIntList list;
  Functions.GetIndexes(name, list);
  for(int i=0; i < list.Count(); i++ )
    store.Add(  Macros.GetObject(list[i]) );
  return list.Count();
}
//..............................................................................
size_t TLibrary::LocateFunctions(const olxstr& name, TBasicFunctionPList& store)  {
  size_t retVal = LocateLocalFunctions(name, store);
  for( int i=0; i < Libraries.Count(); i++ )
    retVal += GetLibraryByIndex(i)->LocateFunctions(name, store);
  return retVal;
}
//..............................................................................
size_t TLibrary::LocateMacros(const olxstr& name, TBasicFunctionPList& store)  {
  size_t retVal = LocateLocalMacros(name, store);

  for( int i=0; i < Libraries.Count(); i++ )
    retVal += GetLibraryByIndex(i)->LocateMacros(name, store);
  return retVal;
}
//..............................................................................
ABasicFunction* TLibrary::LocateFunction(const olxstr& name, unsigned int argc)  {
  if( argc == 0 )  {
    int index = Functions.IndexOf(name);
    return (index != -1) ? Functions.GetObject( index ) : NULL;
  }
  else  {
    TIntList list;
    Functions.GetIndexes( name, list );
    for(int i=0; i < list.Count(); i++ )  {
      if( Functions.GetObject( list[i] )->GetArgStateMask() & (1 << argc) )
        return Functions.GetObject( list[i] );
    }
    return NULL;
  }
}
//..............................................................................
ABasicFunction* TLibrary::LocateMacro(const olxstr& name, unsigned int argc)  {
  if( argc == 0 )  {
    int index = Macros.IndexOfComparable(name);
    return (index != -1) ? Macros.GetObject( index ) : NULL;
  }
  else  {
    TIntList list;
    Macros.GetIndexes( name, list );
    for(int i=0; i < list.Count(); i++ )  {
      if( Macros.GetObject( list[i] )->GetArgStateMask() & (1 << argc) )
        return Macros.GetObject( list[i] );
    }
    return NULL;
  }
}
//..............................................................................
ABasicFunction* TLibrary::FindFunction(const olxstr& name, unsigned int argc)  {
  if( name.IndexOf('.') != -1 )  {
    TLibrary* searchLib = this;
    TStrList libPath( name, '.') ;
    for( int i=0; i < libPath.Count()-1; i++ )  {
      searchLib = searchLib->GetLibraryByName( libPath[i] );
      if( !searchLib )  break;
    }
    return (searchLib) ? searchLib->LocateFunction( libPath[libPath.Count()-1], argc ) 
      : NULL;
  }
  else
    return this->LocateFunction( name, argc );
}
//..............................................................................
ABasicFunction* TLibrary::FindMacro(const olxstr& name, unsigned int argc)  {
  if( name.IndexOf('.') != -1 && !name.EndsWith('.') )  {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    for( int i=0; i < libPath.Count()-1; i++ )  {
      searchLib = searchLib->GetLibraryByName( libPath[i] );
      if( !searchLib )  break;
    }
    return (searchLib) ? searchLib->LocateMacro( libPath[libPath.Count()-1], argc ) : NULL;
  }
  else
    return this->LocateMacro( name, argc );
}
//..............................................................................
size_t TLibrary::LocateSimilarFunctions(const olxstr& name, TBasicFunctionPList& store)  {
  size_t cnt = store.Count();
  for( int i=0; i < Functions.Count(); i++ )
    if( Functions.GetString(i).StartFromi(name) )
      store.Add( Functions.GetObject(i) );
  return Functions.Count() - cnt;
}
//..............................................................................
size_t TLibrary::LocateSimilarMacros(const olxstr& name, TBasicFunctionPList& store)  {
  size_t cnt = store.Count();
  for( int i=0; i < Macros.Count(); i++ )
    if( Macros.GetString(i).StartFromi(name) )
      store.Add( Macros.GetObject(i) );
  return Macros.Count() - cnt;
}
//..............................................................................
size_t TLibrary::FindSimilarFunctions(const olxstr& name, TBasicFunctionPList& store)  {
  if( name.IndexOf('.') != -1 )  {
    TLibrary* searchLib = this;
    TStrList libPath( name, '.') ;
    for( int i=0; i < libPath.Count()-1; i++ )  {
      searchLib = searchLib->GetLibraryByName( libPath[i] );
      if( !searchLib )  break;
    }
    return searchLib ? 
      searchLib->LocateSimilarFunctions( libPath[libPath.Count()-1], store ) : 0;
  }
  else
    return this->LocateSimilarFunctions( name, store );
}
//..............................................................................
size_t TLibrary::FindSimilarMacros(const olxstr& name, TBasicFunctionPList& store)  {
  if( name.IndexOf('.') != -1 && !name.EndsWith('.') )  {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    for( int i=0; i < libPath.Count()-1; i++ )  {
      searchLib = searchLib->GetLibraryByName( libPath[i] );
      if( !searchLib )  break;
    }
    return searchLib ? 
      searchLib->LocateSimilarMacros( libPath[libPath.Count()-1], store ) : 0;
  }
  else
    return this->LocateSimilarMacros( name, store );
}
//..............................................................................
bool TLibrary::CheckProgramState(unsigned int state)  {
  return (LibraryOwner != NULL) ? LibraryOwner->CheckProgramState( state ) : true;
}
//..............................................................................
void TLibrary::ListAllFunctions(TBasicFunctionPList& store)  {
  for( int i=0; i < Functions.Count(); i++ )
    store.Add( Functions.GetObject(i) );
  for( int i=0; i < Libraries.Count(); i++ )
    GetLibraryByIndex(i)->ListAllFunctions(store);
}
//..............................................................................
void TLibrary::ListAllMacros(TBasicFunctionPList& store)  {
  for( int i=0; i < Macros.Count(); i++ )
    store.Add( Macros.GetObject(i) );
  for( int i=0; i < Libraries.Count(); i++ )
    GetLibraryByIndex(i)->ListAllMacros(store);
}
//..............................................................................


