#ifndef libraryH
#define libraryH
#include "function.h"
#undef GetObject
//---------------------------------------------------------------------------
BeginEsdlNamespace()


class TLibrary: public IEObject, public ABasicLibrary  {
  TSStrPObjList<olxstr,ABasicFunction*, true> Functions, Macros;
  olxstr LibraryName;
  TSStrPObjList<olxstr,TLibrary*, true> Libraries;
  ALibraryContainer* LibraryOwner;
  TLibrary* ParentLibrary;
protected:
  // these are the helper functions to find func/macro in this library only using
  // unqulified names
  ABasicFunction* LocateFunction( const olxstr& fuctionName, unsigned int argc = 0 );
  ABasicFunction* LocateMacro( const olxstr& macroName, unsigned int argc = 0 );

  size_t LocateLocalFunctions( const olxstr& name, TBasicFunctionPList& store);
  size_t LocateLocalMacros( const olxstr& name, TBasicFunctionPList& store);
  void SetParentLibrary( TLibrary* lib )  {  ParentLibrary = lib;  }
public:
  /* it important to pass the owner for the special program state checks to be performed */
  TLibrary(const olxstr& libraryName, ALibraryContainer* owner = NULL);
  virtual ~TLibrary();

  virtual const olxstr& GetName()  const        {  return LibraryName;  }
  virtual ABasicLibrary* GetParentLibrary() const {  return ParentLibrary;  }
  // implementation of the state checker
  virtual bool CheckProgramState(unsigned int state);
  virtual ALibraryContainer* GetOwner() const     {  return LibraryOwner;  }

  inline int FunctionCount()  const  {  return Functions.Count(); }
  inline ABasicFunction* GetFunctionByIndex(int i)  const {  return Functions.GetObject(i);  }

  inline int MacroCount()  const  {  return Macros.Count(); }
  inline ABasicFunction* GetMacroByIndex(int i)  const {  return Macros.GetObject(i);  }


  TLibrary* AddLibrary(const olxstr& name, ALibraryContainer* owner = NULL );
  // not that the library will be deleted upon destruction
  void AttachLibrary( TLibrary* lib );

  inline int LibraryCount()  const  {  return Libraries.Count();  }
  inline TLibrary* GetLibraryByName( const olxstr& name ) const  {  return Libraries[name];  }
  inline TLibrary* GetLibraryByIndex( int index )           const  {  return Libraries.GetObject( index );  }

  template <class BaseClass>
    void RegisterFunction( TFunction<BaseClass>* func, bool replace = false )  {
      TIntList list;
      Functions.GetIndexes( func->GetName(), list );
      for(int i=0; i < list.Count(); i++ )  {
        unsigned int argc = Functions.GetObject(list[i])->GetArgStateMask();
        if( func->GetArgStateMask() & argc )  {
          if( replace )  {
            delete Functions.GetObject(list[i]);
            Functions.Delete(list[i]);
            break;
          }
          throw TDuplicateEntry(__OlxSourceInfo, olxstr("function (same number of args)") << func->GetName(), "function");
        }
      }
      func->SetParentLibrary( *this );
      Functions.Add(func->GetName(), func);
    }

  void RegisterStaticFunction( TStaticFunction* func, bool replace = false );
  void RegisterStaticMacro( TStaticMacro* func, bool replace = false );

  template <class BaseClass>
    void RegisterMacro( TMacro<BaseClass>* macro, bool replace = false )  {
      TIntList list;
      Macros.GetIndexes( macro->GetName(), list );
      for(int i=0; i < list.Count(); i++ )  {
        unsigned int argc = Macros.GetObject(list[i])->GetArgStateMask();
        if( macro->GetArgStateMask() & argc )  {
          if( replace )  {
            delete Macros.GetObject(list[i]);
            Macros.Delete(list[i]);
            break;
          }
          throw TDuplicateEntry(__OlxSourceInfo, olxstr("macro (same number of args)") << macro->GetName(), "macro");
        }
      }
      macro->SetParentLibrary( *this );
      Macros.Add(macro->GetName(), macro);
    }
  /* if function name is no qualified, current lib is searched only, for quailified
    function names like, html.home, the library will be located and searched
  */
  ABasicFunction* FindFunction(const olxstr& name, unsigned int argc = 0);
  ABasicFunction* FindMacro(const olxstr& name, unsigned int argc = 0);

  // finds similar macros and puts them to the list, returns the number of added entries
  size_t FindSimilarMacros(const olxstr& name, TBasicFunctionPList& store);
  // finds similar functions and puts them to the list, returns the number of added entries
  size_t FindSimilarFunctions(const olxstr& name, TBasicFunctionPList& store);
  // finds similar library names and puts them to the list, returns the number of added entries
  size_t FindSimilarLibraries(const olxstr& name, TBasicLibraryPList& store);

  /* the functions do search sublibrraies too and return the list of available functions
    the return value is the number of found functions
  */
  size_t LocateFunctions(const olxstr& name, TBasicFunctionPList& store);
  size_t LocateMacros(const olxstr& name, TBasicFunctionPList& store);
  size_t LocateSimilarFunctions(const olxstr& name, TBasicFunctionPList& store);
  size_t LocateSimilarMacros(const olxstr& name, TBasicFunctionPList& store);
  size_t LocateSimilarLibraries(const olxstr& name, TBasicLibraryPList& store);

  void ListAllFunctions(TBasicFunctionPList& store);
  void ListAllMacros(TBasicFunctionPList& store);

  class TDuplicateEntry : public TBasicException  {
  public:
    TDuplicateEntry(const olxstr& location, const olxstr& entry, const olxstr& entryType) :
      TBasicException(location, olxstr("Duplicate ") << entryType << '-' << entry)
      {  ;  }
    virtual IEObject* Replicate()  const  {  return new TDuplicateEntry(*this);  }
  };

  class TLibraryNotFound : public TBasicException  {
  public:
    TLibraryNotFound(const olxstr& location, const olxstr& libName) :
      TBasicException(location, olxstr("Library ") << libName << " not found")
      {  ;  }
    virtual IEObject* Replicate()  const  {  return new TLibraryNotFound(*this);  }
  };
};

EndEsdlNamespace()

#endif
