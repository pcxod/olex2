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

TLibrary::TLibrary(const olxstr& libraryName, ALibraryContainer* owner) {
  LibraryName = libraryName;
  LibraryOwner = owner;
  ParentLibrary = 0;
}
//..............................................................................
TLibrary::~TLibrary()  {
  Functions.ForEach(olx_obj_deleter());
  Macros.ForEach(olx_obj_deleter());
  for (size_t i = 0; i < Libraries.Count(); i++) {
    delete Libraries.GetValue(i);
  }
}
//..............................................................................
TLibrary* TLibrary::AddLibrary(const olxstr& name, ALibraryContainer* owner) {
  if (Libraries.IndexOf(name) != InvalidIndex) {
    throw TDuplicateEntry(__OlxSourceInfo, name, "library");
  }
  TLibrary* lib = new TLibrary(name, owner);
  Libraries.Add(name, lib);
  lib->SetParentLibrary(this);
  if (owner == 0) {
    lib->LibraryOwner = this->LibraryOwner;
  }
  return lib;
}
//..............................................................................
void TLibrary::AttachLibrary(TLibrary* lib) {
  if (Libraries.IndexOf(lib->GetName()) != InvalidIndex) {
    throw TDuplicateEntry(__OlxSourceInfo, lib->GetName(), "library");
  }
  Libraries.Add(lib->GetName(), lib);
  lib->SetParentLibrary(this);
  lib->LibraryOwner = this->LibraryOwner;
}
//..............................................................................
ABasicFunction* TLibrary::Register(
  container_t& container,
  ABasicFunction* fm,
  uint16_t flags)
{
  container_t::entry_t* fs = container.Find(fm->GetName());
  ABasicFunction* rv = 0;
  if (fm->GetArgStateMask() == uint32_t(~0)) {
    if (fs != 0 && fs->Count() > 1)
      throw TInvalidArgumentException(__OlxSourceInfo, "ambiguous replacement");
  }
  fm->SetParentLibrary(*this);
  if (fs != 0) {
    ABasicFunction* src = fs->get_value();
    fm->SetArgStateMask(fm->GetArgStateMask() | src->GetArgStateMask());
    if (src->HasOptions()) {
      olxstr_dict<olxstr> options = src->GetOptions();
      options.Merge(fm->GetOptions());
      fm->SetOptions(options);
    }
    IteratorJ<ABasicFunction*> itr(&fs->get_value(), fs->Iterate());
    while (itr.HasNext()) {
      ABasicFunction* f = itr.Next();
      if ((fm->GetArgStateMask() & f->GetArgStateMask()) != 0) {
        if ((flags & libReplace) != 0) {
          if (fm->GetDescription().IsEmpty()) {
            fm->SetDescription(f->GetDescription());
          }
          fs->replace(f, fm);
          if ((flags & libReturn) != 0) {
            rv = f;
          }
          else {
            delete f;
          }
          return rv;
        }
        else if ((flags & libChain) != 0) {
          if (dynamic_cast<AMacro*>(f) != 0) {
            TMacro<FunctionChainer>* fcm =
              dynamic_cast<TMacro<FunctionChainer>*>(f);
            FunctionChainer* fc = 0;
            if (fcm == 0) {
              fc = &Chains.AddNew();
              fc->Add(f);
              fcm = new TMacro<FunctionChainer>(fc, &FunctionChainer::RunMacro,
                f->GetName(), EmptyString(), 0);
              fcm->SetParentLibrary(*this);
              fs->replace(f, fcm);
            }
            else {
              fc = &fcm->GetBaseInstance();
            }
            fc->Add(fm).Update(fcm);
          }
          else if (dynamic_cast<AFunction*>(f) != 0) {
            TFunction<FunctionChainer>* fcm =
              dynamic_cast<TFunction<FunctionChainer>*>(f);
            FunctionChainer* fc = 0;
            if (fcm == 0) {
              fc = &Chains.AddNew();
              fc->Add(f);
              fcm = new TFunction<FunctionChainer>(fc, &FunctionChainer::RunFunction,
                f->GetName(), 0);
              fcm->SetParentLibrary(*this);
              fs->replace(f, fcm);
            }
            else {
              fc = &fcm->GetBaseInstance();
            }
            fc->Add(fm).Update(fcm);
          }
          return 0;
        }
        throw TDuplicateEntry(__OlxSourceInfo,
          olxstr("macro/function (same number of args) ").quote() <<
          fm->GetName(),
          "static macro");
      }
    }
    fs->AddSame(fm);
  }
  else {
    container.Add(fm->GetName(), fm);
  }
  return rv;
}
//..............................................................................
size_t TLibrary::LocateLocalFunctions(const olxstr& name, TBasicFunctionPList& store) {
  container_t::entry_t *e = Functions.Find(name);
  if (e == 0) {
    return 0;
  }
  size_t cnt = store.Count();
  container_t::entry_t::iterator_t i = e->iterate();
  while (i->HasNext()) {
    store.Add(i->Next());
  }
  return store.Count() - cnt;
}
//..............................................................................
size_t TLibrary::LocateLocalMacros(const olxstr& name, TBasicFunctionPList& store) {
  container_t::entry_t* e = Macros.Find(name);
  if (e == 0) {
    return 0;
  }
  size_t cnt = store.Count();
  container_t::entry_t::iterator_t i = e->iterate();
  while (i->HasNext()) {
    store.Add(i->Next());
  }
  return store.Count() - cnt;
}
//..............................................................................
size_t TLibrary::LocateFunctions(const olxstr& name, TBasicFunctionPList& store) {
  size_t retVal = LocateLocalFunctions(name, store);
  
  for (size_t i = 0; i < Libraries.Count(); i++) {
    retVal += GetLibraryByIndex(i)->LocateFunctions(name, store);
  }
  return retVal;
}
//..............................................................................
size_t TLibrary::LocateMacros(const olxstr& name, TBasicFunctionPList& store) {
  size_t retVal = LocateLocalMacros(name, store);
  for (size_t i = 0; i < Libraries.Count(); i++) {
    retVal += GetLibraryByIndex(i)->LocateMacros(name, store);
  }
  return retVal;
}
//..............................................................................
ABasicFunction* TLibrary::LocateFunction(const olxstr& name, uint32_t argc) {
  container_t::entry_t* m = Functions.Find(name);
  if (m == 0) {
    return 0;
  }
  if (argc == 0) {
    return m->get_value();
  }
  else {
    container_t::entry_t::iterator_t i = m->iterate();
    while (i->HasNext()) {
      ABasicFunction* f = i->Next();
      if (argc >= 10) {
        uint32_t mm = f->GetArgStateMask();
        uint32_t im = (fpAny ^ (1 << 10));
        // any or any but not fewer than
        if (mm == fpAny || mm > im) {
          return f;
        }
        continue;
      }
      if (f->GetArgStateMask() & (1 << argc)) {
        return f;
      }
    }
    return 0;
  }
}
//..............................................................................
ABasicFunction* TLibrary::LocateMacro(const olxstr& name, uint32_t argc) {
  container_t::entry_t* m = Macros.Find(name);
  if (m == 0) {
    return 0;
  }
  if (argc == 0) {
    return m->get_value();
  }
  else {
    IteratorJ<ABasicFunction*> i(&m->get_value(), m->Iterate());
    while (i.HasNext()) {
      ABasicFunction* f = i.Next();
      if (argc >= 10) {
        uint32_t mm = f->GetArgStateMask();
        uint32_t im = (fpAny ^ (1 << 10));
        // any or any but not fewer than
        if (mm == fpAny || mm > im) {
          return f;
        }
        continue;
      }
      if (f->GetArgStateMask() & (1 << argc)) {
        return f;
      }
    }
    return 0;
  }
}
//..............................................................................
ABasicFunction* TLibrary::FindFunction(const olxstr& name, uint32_t argc) {
  if (name.IndexOf('.') != InvalidIndex) {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    if (libPath.IsEmpty()) {
      return 0;
    }
    for (size_t i = 0; i < libPath.Count() - 1; i++) {
      searchLib = searchLib->GetLibraryByName(libPath[i]);
      if (searchLib == 0) {
        break;
      }
    }
    return (searchLib) ? searchLib->LocateFunction(libPath.GetLastString(), argc) : 0;
  }
  else {
    return this->LocateFunction(name, argc);
  }
}
//..............................................................................
ABasicFunction* TLibrary::FindMacro(const olxstr& name, uint32_t argc) {
  if (name.IndexOf('.') != InvalidIndex && !name.EndsWith('.')) {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    if (libPath.IsEmpty()) {
      return 0;
    }
    for (size_t i = 0; i < libPath.Count() - 1; i++) {
      searchLib = searchLib->GetLibraryByName(libPath[i]);
      if (searchLib == 0) {
        break;
      }
    }
    return (searchLib != 0)
      ? searchLib->LocateMacro(libPath.GetLastString(), argc) : 0;
  }
  else {
    return this->LocateMacro(name, argc);
  }
}
//..............................................................................
size_t TLibrary::LocateSimilarFunctions(const olxstr& name, TBasicFunctionPList& store) {
  const size_t cnt = store.Count();
  container_t::iterator_t fi = Functions.iterate();
  while (fi->HasNext()) {
    ABasicFunction* f = fi->Next();
    if (f->GetName().StartsFromi(name)) {
      store.Add(f);
    }
  }
  return store.Count() - cnt;
}
//..............................................................................
size_t TLibrary::LocateSimilarMacros(const olxstr& name, TBasicFunctionPList& store) {
  const size_t cnt = store.Count();
  container_t::iterator_t fi = Macros.iterate();
  while (fi->HasNext()) {
    ABasicFunction* f = fi->Next();
    if (f->GetName().StartsFromi(name)) {
      store.Add(f);
    }
  }
  return store.Count() - cnt;
}
//..............................................................................
size_t TLibrary::LocateSimilarLibraries(const olxstr& name, TBasicLibraryPList& store) {
  const size_t cnt = store.Count();
  for (size_t i = 0; i < Libraries.Count(); i++) {
    if (Libraries.GetKey(i).StartsFromi(name)) {
      store.Add(Libraries.GetValue(i));
    }
  }
  return store.Count() - cnt;
}
//..............................................................................
size_t TLibrary::FindSimilarFunctions(const olxstr& name, TBasicFunctionPList& store) {
  if (name.IndexOf('.') != InvalidIndex) {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    if (libPath.IsEmpty()) {
      return 0;
    }
    const size_t cnt = libPath.Count() - (name.EndsWith('.') ? 0 : 1);
    for (size_t i = 0; i < cnt; i++) {
      searchLib = searchLib->GetLibraryByName(libPath[i]);
      if (searchLib == 0) {
        break;
      }
    }
    if (searchLib != 0) {
      if (name.EndsWith('.')) {
        TLibrary::container_t::iterator_t itr = searchLib->IterateFunctions();
        while (itr->HasNext()) {
          store.Add(itr->Next());
        }
        return searchLib->FunctionCount();
      }
      return searchLib->LocateSimilarFunctions(libPath.GetLastString(), store);
    }
    return 0;
  }
  else {
    return this->LocateSimilarFunctions(name, store);
  }
}
//..............................................................................
size_t TLibrary::FindSimilarMacros(const olxstr& name, TBasicFunctionPList& store) {
  if (name.IndexOf('.') != InvalidIndex) {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    if (libPath.IsEmpty()) {
      return 0;
    }
    const size_t cnt = libPath.Count() - (name.EndsWith('.') ? 0 : 1);
    for (size_t i = 0; i < cnt; i++) {
      searchLib = searchLib->GetLibraryByName(libPath[i]);
      if (searchLib == 0) {
        break;
      }
    }
    if (searchLib != 0) {
      if (name.EndsWith('.')) {
        TLibrary::container_t::iterator_t itr = searchLib->IterateMacros();
        while (itr->HasNext()) {
          store.Add(itr->Next());
        }
        return searchLib->MacroCount();
      }
      return searchLib->LocateSimilarMacros(libPath.GetLastString(), store);
    }
    return 0;
  }
  else {
    return this->LocateSimilarMacros(name, store);
  }
}
//..............................................................................
size_t TLibrary::FindSimilarLibraries(const olxstr& name, TBasicLibraryPList& store) {
  if (name.IndexOf('.') != InvalidIndex) {
    TLibrary* searchLib = this;
    TStrList libPath(name, '.');
    for (size_t i = 0; i < libPath.Count(); i++) {
      TLibrary* lib = searchLib->GetLibraryByName(libPath[i]);
      if (lib == 0) {
        return searchLib->LocateSimilarLibraries(libPath[i], store);
      }
      searchLib = lib;
    }
    // must be '.' ending string - get all libs
    for (size_t i = 0; i < searchLib->LibraryCount(); i++) {
      store.Add(searchLib->GetLibraryByIndex(i));
    }
    return searchLib->LibraryCount();
  }
  else {
    return this->LocateSimilarLibraries(name, store);
  }
}
//..............................................................................
bool TLibrary::CheckProgramState(uint32_t state)  {
  return (LibraryOwner != 0) ? LibraryOwner->CheckProgramState( state ) : true;
}
//..............................................................................
void TLibrary::ListAllFunctions(TBasicFunctionPList& store) {
  container_t::iterator_t fi = Functions.iterate();
  while (fi->HasNext()) {
    store.Add(fi->Next());
  }
  for (size_t i = 0; i < Libraries.Count(); i++) {
    GetLibraryByIndex(i)->ListAllFunctions(store);
  }
}
//..............................................................................
void TLibrary::ListAllMacros(TBasicFunctionPList& store) {
  container_t::iterator_t fi = Macros.iterate();
  while (fi->HasNext()) {
    store.Add(fi->Next());
  }
  for (size_t i = 0; i < Libraries.Count(); i++) {
    GetLibraryByIndex(i)->ListAllMacros(store);
  }
}
//..............................................................................
