//---------------------------------------------------------------------------
#ifndef macroerrorH
#define macroerrorH

//#include "estring.h"
#include "exception.h"
#include "ptypes.h"
#include "estack.h"

//---------------------------------------------------------------------------
BeginEsdlNamespace()

class ABasicFunction;

const unsigned short  peNonexistingFunction = 0x0001,
                      peProcessingError     = 0x0002,
                      peProcessingException = 0x0004,
                      peInvalidOption       = 0x0008,
                      peInvalidArgCount     = 0x0010,
                      peIllegalState          = 0x0020;

class TMacroError: public IEObject  {
  unsigned short ProcessError;
  bool DeleteObject;
  olxstr ErrorInfo, Location;
  IEObject* RetValue;
  TEStack<olxstr>* Stack;
public:
  TMacroError();
  virtual ~TMacroError()  {
    if( DeleteObject )  delete RetValue;
  }
  
  void operator = (const TMacroError& ME);
  olxstr& ProcessingError(const olxstr& location, const olxstr& errMsg);
  void NonexitingMacroError(const olxstr& macroName);
  void WrongArgCount(const ABasicFunction& caller, unsigned int provided);
  void WrongOption(const ABasicFunction& caller, const olxstr& option);
  void WrongState(const ABasicFunction& caller);

  void Reset()  {
    ProcessError = 0;
    ErrorInfo = EmptyString;
    if( DeleteObject )  delete RetValue;
    DeleteObject = false;
    RetValue = (IEObject*)NULL;
    Stack = NULL;
  }

  void ProcessingException(const ABasicFunction& caller, const TExceptionBase& exc);

  inline bool IsSuccessful()           const  {  return (ProcessError == 0);  }
  inline bool DoesFunctionExist()      const {  return (ProcessError&peNonexistingFunction) == 0;  }
  inline bool IsProcessingError()      const {  return (ProcessError&peProcessingError) != 0;  }
  inline bool IsProcessingException()  const {  return (ProcessError&peProcessingException) != 0;  }
  inline bool IsInvalidOption()        const {  return (ProcessError&peInvalidOption) != 0;  }
  inline bool IsInvalidArguments()     const {  return (ProcessError&peInvalidArgCount) != 0;  }
  inline bool IsIllegalState()         const {  return (ProcessError&peIllegalState) != 0;  }

  inline const olxstr& GetInfo()     const {  return ErrorInfo;  }
  inline const olxstr& GetLocation() const {  return Location;  }
  olxstr GetRetVal()  const;

  inline bool HasRetVal()     const {  return RetValue != NULL;  }
  inline IEObject* RetObj()  const  {  return RetValue;  }
  
  TEStack<olxstr>* GetStack() const {  return Stack;  }
  void SetStack(TEStack<olxstr>& stack)  {  Stack = &stack;  }

  // the type is validated
  template <class EObj>
    EObj* GetRetObj()  {
      if( !EsdlInstanceOf(*RetValue, EObj) )
        throw TCastException(__OlxSourceInfo, EsdlObjectName(*RetValue), EsdlClassName(EObj) );
      return (EObj*)RetValue;
    }

  template <class PT>
    void SetRetVal(const PT& val)  {
      if( DeleteObject )  delete RetValue;
      DeleteObject = true;
      RetValue = new TEPType<PT>(val);
  }

  template <class PT>
    void SetRetVal(PT* val)  {
      if( DeleteObject )  delete RetValue;
      DeleteObject = false;
      RetValue = val;
  }

  class TCastException : public TBasicException  {
    public: TCastException(const olxstr& location, const olxstr& from, const olxstr& to ):
      TBasicException(location, olxstr("Cannot cast '") << from << "' to '"  << to << '\'')  {  }
    virtual IEObject* Replicate()  const    {  return new TCastException(*this);  }
  };

};

EndEsdlNamespace()
#endif
