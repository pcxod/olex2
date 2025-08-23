/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_library_H
#define __olx_sdl_library_H
#include "function.h"
//#include "ehashed.h"
#undef GetObject

BeginEsdlNamespace()

const uint16_t
  libReplace = 0x0001,
  libReturn  = 0x0002,
  libChain   = 0x0004;

class TLibrary: public IOlxObject, public ABasicLibrary  {
public:
  //typedef TEHashTreeMap<olxstr, ABasicFunction* , olxstrComparator<true> >
  typedef sorted::StringAssociation<ABasicFunction*, true>
    container_t;
private:
  container_t Functions, Macros;
  TTypeList<FunctionChainer> Chains;
  olxstr LibraryName;
  olxstr_dict<TLibrary*, true> Libraries;
  ALibraryContainer* LibraryOwner;
  TLibrary* ParentLibrary;
protected:
  /* these are the helper functions to find func/macro in this library only
  using unqualified names
  */
  ABasicFunction* LocateFunction(const olxstr& fuctionName, uint32_t argc = 0);
  ABasicFunction* LocateMacro(const olxstr& macroName, uint32_t argc = 0);

  size_t LocateLocalFunctions(const olxstr& name, TBasicFunctionPList& store);
  size_t LocateLocalMacros(const olxstr& name, TBasicFunctionPList& store);
  void SetParentLibrary(TLibrary* lib)  {  ParentLibrary = lib;  }
  /* Adds a macro or a function to the given container. If an object with the
  same name and number of arguments already exists and replace is set to false
  the TDuplicateEntry exception will be thrown. Otherwise, if do_return is
  set to true, the duplicate object will be returned or deleted if it is set to
  false. Note that if the description of the fm is empty - the description of
  the previous function will be copied to
  */
  ABasicFunction *Register(
    container_t &container,
    ABasicFunction* fm,
    uint16_t flags);
public:
  /* it important to pass the owner for the special program state checks to be
  performed
  */
  TLibrary(const olxstr& libraryName, ALibraryContainer* owner = NULL);
  virtual ~TLibrary();

  virtual const olxstr& GetName() const {  return LibraryName;  }
  virtual ABasicLibrary* GetParentLibrary() const {  return ParentLibrary;  }
  // implementation of the state checker
  virtual bool CheckProgramState(uint32_t state);
  virtual ALibraryContainer* GetOwner() const {  return LibraryOwner;  }

  size_t FunctionCount() const {  return Functions.Count(); }
  ABasicFunction* GetFunctionByIndex(size_t i)  const {
    return Functions.GetValue(i);
  }

  size_t MacroCount() const {  return Macros.Count(); }
  ABasicFunction* GetMacroByIndex(size_t i) const {
    return Macros.GetValue(i);
  }

  TLibrary* AddLibrary(const olxstr& name, ALibraryContainer* owner = NULL);
  // note that the library will be deleted upon destruction
  void AttachLibrary(TLibrary* lib);

  size_t LibraryCount() const {  return Libraries.Count();  }
  TLibrary* GetLibraryByName(const olxstr& name) const {
    return Libraries.Find(name, NULL);
  }
  TLibrary* GetLibraryByIndex(size_t index) const {
    return Libraries.GetValue(index);
  }

  ABasicFunction *Register(
    AFunction* func, uint16_t flags=0)
  {
    return Register(Functions, func, flags);
  }
  ABasicFunction *Register(
    AMacro* func, uint16_t flags=0)
  {
    return Register(Macros, func, flags);
  }

  /* if function name is no qualified, current lib is searched only, for
  quailified function names like, html.home, the library will be located and
  searched
  */
  ABasicFunction* FindFunction(const olxstr& name, uint32_t argc = 0);
  ABasicFunction* FindMacro(const olxstr& name, uint32_t argc = 0);

  /* finds similar macros and puts them to the list, returns the number of
  added entries
  */
  size_t FindSimilarMacros(const olxstr& name, TBasicFunctionPList& store);
  /* finds similar functions and puts them to the list, returns the number of
  added entries
  */
  size_t FindSimilarFunctions(const olxstr& name, TBasicFunctionPList& store);
  /* finds similar library names and puts them to the list, returns the number
  of added entries
  */
  size_t FindSimilarLibraries(const olxstr& name, TBasicLibraryPList& store);

  /* the functions do search sublibrraies too and return the list of available
  functions the return value is the number of found functions
  */
  size_t LocateFunctions(const olxstr& name, TBasicFunctionPList& store);
  size_t LocateMacros(const olxstr& name, TBasicFunctionPList& store);
  size_t LocateSimilarFunctions(const olxstr& name, TBasicFunctionPList& store);
  size_t LocateSimilarMacros(const olxstr& name, TBasicFunctionPList& store);
  size_t LocateSimilarLibraries(const olxstr& name, TBasicLibraryPList& store);

  void ListAllFunctions(TBasicFunctionPList& store);
  void ListAllMacros(TBasicFunctionPList& store);

  bool IsEmpty() const {
    if( MacroCount() == 0 && FunctionCount() == 0 )  {
      for( size_t i=0; i < LibraryCount(); i++ )
        if( !GetLibraryByIndex(i)->IsEmpty() )
          return false;
      return true;
    }
    return false;
  }

  class TDuplicateEntry : public TBasicException  {
  public:
    TDuplicateEntry(const olxstr& location, const olxstr& entry,
      const olxstr& entryType) :
      TBasicException(location, olxstr("Duplicate ") << entryType << '-' << entry)
      {}
    virtual IOlxObject* Replicate() const {  return new TDuplicateEntry(*this);  }
  };

  class TLibraryNotFound : public TBasicException  {
  public:
    TLibraryNotFound(const olxstr& location, const olxstr& libName) :
      TBasicException(location, olxstr("Library ") << libName << " not found")
      {}
    virtual IOlxObject* Replicate() const {  return new TLibraryNotFound(*this);  }
  };
};

EndEsdlNamespace()
#endif
