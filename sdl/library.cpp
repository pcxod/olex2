/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "library.h"
UseEsdlNamespace()

TLibrary::TLibrary(const olxstr& libraryName, ALibraryContainer* owner)  {
  LibraryName  = libraryName;
  LibraryOwner = owner;
  ParentLibrary = NULL;
}
//..............................................................................
TLibrary::~TLibrary()  {
  for( size_t i=0; i < Functions.Count(); i++ )
    delete Functions.GetObject(i);
  for( size_t i=0; i < Macros.Count(); i++ )
    delete Macros.GetObject(i);
  for( size_t i=0; i < Libraries.Count(); i++ )
    delete Libraries.GetObject(i);
}
//..............................................................................
TLibrary* TLibrary::AddLibrary(const olxstr& name, ALibraryContainer* owner )  {
  if( Libraries.IndexOf(name) != InvalidIndex )
    throw TDuplicateEntry(__OlxSourceInfo, name, "library" );
  TLibrary* lib = new TLibrary(name, owner);
  Libraries.Add(name, lib);
  lib->SetParentLibrary( this );
  if( owner == NULL )
    lib->LibraryOwner = this->LibraryOwner;
  return lib;
}
//..............................................................................
void TLibrary::AttachLibrary( TLibrary* lib )  {
  if( Libraries.IndexOf(lib->GetName()) != InvalidIndex )
    throw TDuplicateEntry(__OlxSourceInfo, lib->GetName(), "library" );
  Libraries.Add(lib->GetName(), lib);
  lib->SetParentLibrary( this );
  lib->LibraryOwner = this->LibraryOwner;
}
//..............................................................................
void TLibrary::RegisterStaticMacro( TStaticMacro* func, bool replace )  {
  TSizeList list;
  Macros.GetIndexes(func->GetName(), list);
  for( size_t i=0; i < list.Count(); i++ )  {
    unsigned int argc = Macros.GetObject(list[i])->GetArgStateMask();
    if( (func->GetArgStateMask() & argc) != 0 )  {
      if( replace )  {
        Macros.Delete(list[i]);
        break;
      }
      throw TDuplicateEntry(__OlxSourceInfo, olxstr("macro (same number of args)") << func->GetName(), "static macro");
    }
  }
  func->SetParentLibrary(*this);
  Macros.Add(func->GetName(), func);
}
//..............................................................................
void TLibrary::RegisterStaticFunction( TStaticFunction* func, bool replace )  {
  TSizeList list;
  Functions.GetIndexes(func->GetName(), list);
  for( size_t i=0; i < list.Count(); i++ )  {
    unsigned int argc = Functions.GetObject(list[i])->GetArgStateMask();
    if( (func->GetArgStateMask() & argc) != 0 )  {
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
  TSizeList list;
  Functions.GetIndexes(name, list);
  for( size_t i=0; i < list.Count(); i++ )
    store.Add(Functions.GetObject(list[i]));
  return list.Count();
}
//..............................................................................
size_t TLibrary::LocateLocalMacros( const olxstr& name, TBasicFunctionPList& store)  {
  TSizeList list;
  Functions.GetIndexes(name, list);
  for( size_t i=0; i < list.Count(); i++ )
    store.Add(Macros.GetObject(list[i]));
  return list.Count();
}
//..............................................................................
size_t TLibrary::LocateFunctions(const olxstr& name, TBasicFunctionPList& store)  {
  size_t retVal = LocateLocalFunctions(name, store);
  for( size_t i=0; i < Libraries.Count(); i++ )
    retVal += GetLibraryByIndex(i)->LocateFunctions(name, store);
  return retVal;
}
//..............................................................................
size_t TLibrary::LocateMacros(const olxstr& name, TBasicFunctionPList& store)  {
  size_t retVal = LocateLocalMacros(name, store);
  for( size_t i=0; i < Libraries.Count(); i++ )
    retVal += GetLibraryByIndex(i)->LocateMacros(name, store);
  return retVal;
}
//..............................................................................
ABasicFunction* TLibrary::LocateFunction(const olxstr& name, unsigned int argc)  {
  if( argc == 0 )  {
    const size_t index = Functions.IndexOf(name);
    return (index != InvalidIndex) ? Functions.GetObject(index) : NULL;
  }
  else  {
    TSizeList list;
    Functions.GetIndexes(name, list);
    for( size_t i=0; i < list.Count(); i++ )  {
      if( Functions.GetObject( list[i] )->GetArgStateMask() & (1 << argc) )
        return Functions.GetObject(list[i]);
    }
    return NULL;
  }
}
//..............................................................................
ABasicFunction* TLibrary::LocateMacro(const olxstr& name, unsigned int argc)  {
  if( argc == 0 )  {
    const size_t index = Macros.IndexOf(name);
    return (index != InvalidIndex) ? Macros.GetObject( index ) : NULL;
  }
  else  {
    TSizeList list;
    Macros.GetIndexes(name, list);
    for( size_t i=0; i < list.Count(); i++ )  {
      if( Macros.GetObject(list[i])->GetArgStateMask() & (1 << argc) )
        return Macros.GetObject(list[i]);
    }
    return NULL;
  }
}
//..............................................................................
ABasicFunction* TLibrary::FindFunction(const olxstr& name, unsigned int argc)  {
  if( name.IndexOf('.') != InvalidIndex )  {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.') ;
    for( size_t i=0; i < libPath.Count()-1; i++ )  {
      searchLib = searchLib->GetLibraryByName(libPath[i]);
      if( searchLib == NULL )  break;
    }
    return (searchLib) ? searchLib->LocateFunction(libPath.GetLastString(), argc) : NULL;
  }
  else
    return this->LocateFunction(name, argc);
}
//..............................................................................
ABasicFunction* TLibrary::FindMacro(const olxstr& name, unsigned int argc)  {
  if( name.IndexOf('.') != InvalidIndex && !name.EndsWith('.') )  {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    for( size_t i=0; i < libPath.Count()-1; i++ )  {
      searchLib = searchLib->GetLibraryByName(libPath[i]);
      if( !searchLib )  break;
    }
    return (searchLib) ? searchLib->LocateMacro(libPath.GetLastString(), argc) : NULL;
  }
  else
    return this->LocateMacro(name, argc);
}
//..............................................................................
size_t TLibrary::LocateSimilarFunctions(const olxstr& name, TBasicFunctionPList& store)  {
  const size_t cnt = store.Count();
  for( size_t i=0; i < Functions.Count(); i++ )  {
    if( Functions.GetString(i).StartsFromi(name) )
      store.Add(Functions.GetObject(i));
  }
  return store.Count() - cnt;
}
//..............................................................................
size_t TLibrary::LocateSimilarMacros(const olxstr& name, TBasicFunctionPList& store)  {
  const size_t cnt = store.Count();
  for( size_t i=0; i < Macros.Count(); i++ )  {
    if( Macros.GetString(i).StartsFromi(name) )
      store.Add(Macros.GetObject(i));
  }
  return store.Count() - cnt;
}
//..............................................................................
size_t TLibrary::LocateSimilarLibraries(const olxstr& name, TBasicLibraryPList& store)  {
  const size_t cnt = store.Count();
  for( size_t i=0; i < Libraries.Count(); i++ )  {
    if( Libraries.GetString(i).StartsFromi(name) )
      store.Add(Libraries.GetObject(i));
  }
  return store.Count() - cnt;
}
//..............................................................................
size_t TLibrary::FindSimilarFunctions(const olxstr& name, TBasicFunctionPList& store)  {
  if( name.IndexOf('.') != InvalidIndex )  {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.') ;
    if( libPath.IsEmpty() )  return 0;
    const size_t cnt = libPath.Count() - (name.EndsWith('.') ? 0 : 1);
    for( size_t i=0; i < cnt; i++ )  {
      searchLib = searchLib->GetLibraryByName(libPath[i]);
      if( searchLib == NULL )  break;
    }
    if( searchLib != NULL )  {
      if( name.EndsWith('.') )  {
        for( size_t i=0; i < searchLib->FunctionCount(); i++ )
          store.Add(searchLib->GetFunctionByIndex(i));
        return searchLib->LibraryCount();
      }
      return searchLib->LocateSimilarFunctions(libPath.GetLastString(), store);
    }
    return 0;
  }
  else
    return this->LocateSimilarFunctions(name, store);
}
//..............................................................................
size_t TLibrary::FindSimilarMacros(const olxstr& name, TBasicFunctionPList& store)  {
  if( name.IndexOf('.') != InvalidIndex )  {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    if( libPath.IsEmpty() )  return 0;
    const size_t cnt = libPath.Count() - (name.EndsWith('.') ? 0 : 1);
    for( size_t i=0; i < cnt; i++ )  {
      searchLib = searchLib->GetLibraryByName(libPath[i]);
      if( searchLib == NULL )  break;
    }
    if( searchLib != NULL )  {
      if( name.EndsWith('.') )  {
        for( size_t i=0; i < searchLib->MacroCount(); i++ )
          store.Add(searchLib->GetMacroByIndex(i));
        return searchLib->MacroCount();
      }
      return searchLib->LocateSimilarMacros(libPath.GetLastString(), store );
    }
    return 0;
  }
  else
    return this->LocateSimilarMacros(name, store);
}
//..............................................................................
size_t TLibrary::FindSimilarLibraries(const olxstr& name, TBasicLibraryPList& store)  {
  if( name.IndexOf('.') != InvalidIndex )  {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    for( size_t i=0; i < libPath.Count(); i++ )  {
      TLibrary* lib = searchLib->GetLibraryByName(libPath[i]);
      if( lib == NULL )
        return searchLib->LocateSimilarLibraries(libPath[i], store);
      searchLib = lib;
    }
    // must be '.' ending string - get all libs
    for( size_t i=0; i < searchLib->LibraryCount(); i++ )
      store.Add( searchLib->GetLibraryByIndex(i) );
    return searchLib->LibraryCount();
  }
  else
    return this->LocateSimilarLibraries(name, store);
}
//..............................................................................
bool TLibrary::CheckProgramState(unsigned int state)  {
  return (LibraryOwner != NULL) ? LibraryOwner->CheckProgramState( state ) : true;
}
//..............................................................................
void TLibrary::ListAllFunctions(TBasicFunctionPList& store)  {
  for( size_t i=0; i < Functions.Count(); i++ )
    store.Add( Functions.GetObject(i) );
  for( size_t i=0; i < Libraries.Count(); i++ )
    GetLibraryByIndex(i)->ListAllFunctions(store);
}
//..............................................................................
void TLibrary::ListAllMacros(TBasicFunctionPList& store)  {
  for( size_t i=0; i < Macros.Count(); i++ )
    store.Add( Macros.GetObject(i) );
  for( size_t i=0; i < Libraries.Count(); i++ )
    GetLibraryByIndex(i)->ListAllMacros(store);
}
//..............................................................................
