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
    delete Functions.GetValue(i);
  for( size_t i=0; i < Macros.Count(); i++ )
    delete Macros.GetValue(i);
  for( size_t i=0; i < Libraries.Count(); i++ )
    delete Libraries.GetValue(i);
}
//..............................................................................
TLibrary* TLibrary::AddLibrary(const olxstr& name, ALibraryContainer* owner)  {
  if( Libraries.IndexOf(name) != InvalidIndex )
    throw TDuplicateEntry(__OlxSourceInfo, name, "library");
  TLibrary* lib = new TLibrary(name, owner);
  Libraries.Add(name, lib);
  lib->SetParentLibrary(this);
  if( owner == NULL )
    lib->LibraryOwner = this->LibraryOwner;
  return lib;
}
//..............................................................................
void TLibrary::AttachLibrary(TLibrary* lib)  {
  if( Libraries.IndexOf(lib->GetName()) != InvalidIndex )
    throw TDuplicateEntry(__OlxSourceInfo, lib->GetName(), "library");
  Libraries.Add(lib->GetName(), lib);
  lib->SetParentLibrary(this);
  lib->LibraryOwner = this->LibraryOwner;
}
//..............................................................................
ABasicFunction *TLibrary::Register(
    sorted::StringAssociation<ABasicFunction*, true> &container,
    ABasicFunction* fm,
    uint16_t flags)
{
  TSizeList list;
  ABasicFunction *rv = NULL;
  container.GetIndices(fm->GetName(), list);
  if (fm->GetArgStateMask() == uint32_t(~0)) {
    if (list.Count() > 1)
      throw TInvalidArgumentException(__OlxSourceInfo, "ambiguous replacement");
  }
  if (!list.IsEmpty()) {
    ABasicFunction *src = container.GetValue(list[0]);
    fm->SetArgStateMask(fm->GetArgStateMask() | src->GetArgStateMask());
    if (src->HasOptions()) {
      olxstr_dict<olxstr> options = src->GetOptions();
      options.Merge(fm->GetOptions());
      fm->SetOptions(options);
    }
  }
  fm->SetParentLibrary(*this);
  for( size_t i=0; i < list.Count(); i++ )  {
    ABasicFunction *f = container.GetValue(list[i]);
    if( (fm->GetArgStateMask() & f->GetArgStateMask()) != 0 )  {
      if( (flags&libReplace) != 0 )  {
        if( fm->GetDescription().IsEmpty() )  {
          fm->SetDescription(f->GetDescription());
        }
        if ( (flags&libReturn) != 0 )
          rv = f;
        else
          delete f;
        container.Delete(list[i]);
        break;
      }
      else if ((flags&libChain) != 0) {
        if (dynamic_cast<AMacro *>(f) != NULL) {
          TMacro<FunctionChainer> *fcm =
            dynamic_cast<TMacro<FunctionChainer> *>(f);
          FunctionChainer *fc = NULL;
          if (fcm == NULL) {
            fc = &Chains.AddNew();
            fc->Add(f);
            fcm = new TMacro<FunctionChainer>(fc, &FunctionChainer::RunMacro,
              f->GetName(), EmptyString(), 0);
            fcm->SetParentLibrary(*this);
            container.GetValue(list[i]) = fcm;
          }
          else
            fc = &fcm->GetBaseInstance();
          fc->Add(fm).Update(fcm);
        }
        else if (dynamic_cast<AFunction *>(f) != NULL) {
          TFunction<FunctionChainer> *fcm =
            dynamic_cast<TFunction<FunctionChainer> *>(f);
          FunctionChainer *fc = NULL;
          if (fcm == NULL) {
            fc = &Chains.AddNew();
            fc->Add(f);
            fcm = new TFunction<FunctionChainer>(fc, &FunctionChainer::RunFunction,
              f->GetName(), 0);
            fcm->SetParentLibrary(*this);
            container.GetValue(list[i]) = fcm;
          }
          else
            fc = &fcm->GetBaseInstance();
          fc->Add(fm).Update(fcm);
        }
        return NULL;
      }
      throw TDuplicateEntry(__OlxSourceInfo,
        olxstr("macro/function (same number of args) ").quote() <<
          fm->GetName(),
        "static macro");
    }
  }
  container.Add(fm->GetName(), fm);
  return rv;
}
//..............................................................................
size_t TLibrary::LocateLocalFunctions( const olxstr& name, TBasicFunctionPList& store)  {
  TSizeList list;
  Functions.GetIndices(name, list);
  for( size_t i=0; i < list.Count(); i++ )
    store.Add(Functions.GetValue(list[i]));
  return list.Count();
}
//..............................................................................
size_t TLibrary::LocateLocalMacros( const olxstr& name, TBasicFunctionPList& store)  {
  TSizeList list;
  Functions.GetIndices(name, list);
  for( size_t i=0; i < list.Count(); i++ )
    store.Add(Macros.GetValue(list[i]));
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
ABasicFunction* TLibrary::LocateFunction(const olxstr& name, uint32_t argc)  {
  if( argc == 0 )  {
    const size_t index = Functions.IndexOf(name);
    return (index != InvalidIndex) ? Functions.GetValue(index) : NULL;
  }
  else  {
    TSizeList list;
    Functions.GetIndices(name, list);
    for( size_t i=0; i < list.Count(); i++ )  {
      if( Functions.GetValue( list[i] )->GetArgStateMask() & (1 << argc) )
        return Functions.GetValue(list[i]);
    }
    return NULL;
  }
}
//..............................................................................
ABasicFunction* TLibrary::LocateMacro(const olxstr& name, uint32_t argc)  {
  if( argc == 0 )  {
    const size_t index = Macros.IndexOf(name);
    return (index != InvalidIndex) ? Macros.GetValue( index ) : NULL;
  }
  else  {
    TSizeList list;
    Macros.GetIndices(name, list);
    for( size_t i=0; i < list.Count(); i++ )  {
      if( Macros.GetValue(list[i])->GetArgStateMask() & (1 << argc) )
        return Macros.GetValue(list[i]);
    }
    return NULL;
  }
}
//..............................................................................
ABasicFunction* TLibrary::FindFunction(const olxstr& name, uint32_t argc)  {
  if( name.IndexOf('.') != InvalidIndex )  {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.') ;
    if (libPath.IsEmpty()) return NULL;
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
ABasicFunction* TLibrary::FindMacro(const olxstr& name, uint32_t argc)  {
  if( name.IndexOf('.') != InvalidIndex && !name.EndsWith('.') )  {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    if (libPath.IsEmpty()) return NULL;
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
  for (size_t i=0; i < Functions.Count(); i++) {
    if (Functions.GetKey(i).StartsFromi(name))
      store.Add(Functions.GetValue(i));
  }
  return store.Count() - cnt;
}
//..............................................................................
size_t TLibrary::LocateSimilarMacros(const olxstr& name, TBasicFunctionPList& store)  {
  const size_t cnt = store.Count();
  for (size_t i=0; i < Macros.Count(); i++) {
    if (Macros.GetKey(i).StartsFromi(name))
      store.Add(Macros.GetValue(i));
  }
  return store.Count() - cnt;
}
//..............................................................................
size_t TLibrary::LocateSimilarLibraries(const olxstr& name, TBasicLibraryPList& store)  {
  const size_t cnt = store.Count();
  for (size_t i=0; i < Libraries.Count(); i++) {
    if (Libraries.GetKey(i).StartsFromi(name))
      store.Add(Libraries.GetValue(i));
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
      return searchLib->LocateSimilarMacros(libPath.GetLastString(), store);
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
      store.Add(searchLib->GetLibraryByIndex(i));
    return searchLib->LibraryCount();
  }
  else
    return this->LocateSimilarLibraries(name, store);
}
//..............................................................................
bool TLibrary::CheckProgramState(uint32_t state)  {
  return (LibraryOwner != NULL) ? LibraryOwner->CheckProgramState( state ) : true;
}
//..............................................................................
void TLibrary::ListAllFunctions(TBasicFunctionPList& store)  {
  for( size_t i=0; i < Functions.Count(); i++ )
    store.Add(Functions.GetValue(i));
  for( size_t i=0; i < Libraries.Count(); i++ )
    GetLibraryByIndex(i)->ListAllFunctions(store);
}
//..............................................................................
void TLibrary::ListAllMacros(TBasicFunctionPList& store)  {
  for( size_t i=0; i < Macros.Count(); i++ )
    store.Add(Macros.GetValue(i));
  for( size_t i=0; i < Libraries.Count(); i++ )
    GetLibraryByIndex(i)->ListAllMacros(store);
}
//..............................................................................
