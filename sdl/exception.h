#ifndef olx_exceptionH
#define olx_exceptionH
#include "ebase.h"

BeginEsdlNamespace()
// forward reference
template <typename,typename> class TTStrList;

class TBasicException: public TExceptionBase  {
  olxstr Message,
         Location;
  TBasicException* Cause;
  virtual void CreationProtection()  {}
protected:
  TBasicException(const TBasicException& toReplicate) {
    this->Message = toReplicate.Message;
    this->Location = toReplicate.Location;
    this->Cause = toReplicate.Cause != NULL ? (TBasicException*)toReplicate.Cause->Replicate() : NULL;
  }
public:
  TBasicException() : Cause(NULL) {}

  TBasicException(const olxstr& location, const TExceptionBase& cause, const olxstr& msg=EmptyString) :
    Message(msg), Location(location), Cause((TBasicException*)cause.GetException()->Replicate())  {}
  /* caution: the expeceted object is an instance from a call to Replicate() !
    and will be deleted
  */
  TBasicException(const olxstr& location, IEObject* cause) :
    Location(location), Cause((TBasicException*)cause) {}

    TBasicException(const olxstr& location, const olxstr& Msg) :
    Location(location), Message(Msg), Cause(NULL)  {}

  virtual ~TBasicException()  {
    if( Cause != NULL )
      delete Cause;
  }
  inline const olxstr& GetError() const {  return Message;  }
  inline const olxstr& GetLocation() const {  return Location;  }
  inline TBasicException* GetCause() const {  return Cause;  }
  virtual const char* GetNiceName() const {  return NULL;  }
  // traces back to the original cause
  TBasicException* GetSource() const;
  virtual IEObject* Replicate() const = 0;
  template <class List>
    List& GetStackTrace( List& output ) const {
      TBasicException const* cause = this;
      while( cause != NULL )  {
        output.Insert(0, cause->GetFullMessage() );
        cause = cause->GetCause();
      }
      return output;
    }
  olxstr GetFullMessage() const;
};

class TIndexOutOfRangeException: public TBasicException  {
  size_t Index, Min, Max;
public:
  TIndexOutOfRangeException(const TIndexOutOfRangeException& toReplicate) :
      TBasicException(toReplicate)  {
    this->Index = toReplicate.Index;
    this->Min = toReplicate.Min;
    this->Max = toReplicate.Max;
  }

  TIndexOutOfRangeException(const olxstr& location, size_t index, size_t min, size_t max):
      TBasicException(location, olxstr("[") << min << ".." << max << "]: " << index )
  {
    Index = index;
    Min = min;
    Max = max;
  }
  // use __POlxSourceInfo to provide source exception, this is to avoid HUGE overhead when creating olxstr
  static inline void ValidateRange(const char* file, const char* function, int line, size_t index, size_t min, size_t max)  {
    if( index < min || index >= max )
      throw TIndexOutOfRangeException(TExceptionBase::FormatSrc(file, function, line), index, min, max);
  }

  inline size_t GetIndex() const {  return Index;  }
  inline size_t GetMin() const {  return Min;  }
  inline size_t GetMax() const {  return Max;  }

  virtual const char* GetNiceName() const {  return "Invalid index";  }
  virtual IEObject* Replicate() const {  return new TIndexOutOfRangeException(*this);  }
};

class TFunctionFailedException: public TBasicException {
public:
  TFunctionFailedException(const olxstr& location, const olxstr& msg) :
    TBasicException(location, msg )  {}
  TFunctionFailedException(const olxstr& location, const TExceptionBase& cause, const olxstr& msg=EmptyString) :
    TBasicException(location, cause, msg )  {}
  TFunctionFailedException(const olxstr& location, IEObject* cause) :
    TBasicException(location, cause )  {}

  virtual const char* GetNiceName() const {  return "Failed";  }
  virtual IEObject* Replicate() const {  return new TFunctionFailedException(*this);  }
};

class TInvalidArgumentException: public TBasicException {
public:
  TInvalidArgumentException(const olxstr& location, const olxstr& argName):
    TBasicException(location, argName)  {}
  inline const olxstr& GetArgumentName() const {  return GetError();  }
  virtual IEObject* Replicate() const {  return new TInvalidArgumentException(*this);  }

};

class TNotImplementedException: public TBasicException {
public:
  TNotImplementedException(const olxstr& location) :
    TBasicException(location, EmptyString)  {}
  virtual const char* GetNiceName() const {  return "Not implemented";  }
  virtual IEObject* Replicate() const {  return new TNotImplementedException(*this);  }
};
//...........................................................................................
class TIOException: public TBasicException {
public:
  TIOException(const olxstr& location, const olxstr &msg):
    TBasicException(location, msg )  {    }
};

class TFileException: public TIOException {
  olxstr FileName;
public:
  TFileException(const TFileException& toReplicate) : TIOException(toReplicate)  {
    this->FileName = toReplicate.FileName;
  }
  TFileException(const olxstr& location, const olxstr& fileName, const olxstr& reason) :
    TIOException(location, olxstr(fileName) << ' ' << reason)
  {
    FileName = fileName;
  }
  inline const olxstr& GetFileName() const {  return FileName;  }
  virtual IEObject* Replicate() const {  return new TFileException(*this);  }
};

class TFileDoesNotExistException: public TFileException {
public:
  TFileDoesNotExistException(const olxstr& location, const olxstr& fileName) :
    TFileException(location, fileName, EmptyString )  {}
  virtual const char* GetNiceName() const {  return "File does not exist";  }
  virtual IEObject* Replicate() const {  return new TFileDoesNotExistException(*this);  }
};

class TEmptyFileException: public TFileException {
public:
  TEmptyFileException(const olxstr& location, const olxstr& fileName) :
    TFileException(location, fileName, EmptyString )  {}
  virtual const char* GetNiceName() const {  return "Empty file";  }
  virtual IEObject* Replicate() const {  return new TEmptyFileException(*this);  }
};
//...........................................................................................
class TMathException: public TBasicException {
public:
  TMathException(const olxstr& location, const olxstr& msg):
    TBasicException(location, msg)  {}
  virtual IEObject* Replicate() const {  return new TMathException(*this);  }
};

class TDivException: public TMathException {
public:
  TDivException(const olxstr& location) : TMathException(location, EmptyString)  {}
  virtual const char* GetNiceName() const {  return "Division by zero";  }
  virtual IEObject* Replicate() const {  return new TDivException(*this);  }
};
//.........................................................................................
class TInvalidNumberException: public TBasicException {
public:
  TInvalidNumberException(const olxstr& location, const olxstr& msg):
    TBasicException(location, msg)  {}
  virtual IEObject* Replicate() const {  return new TInvalidNumberException(*this);  }
};

class TInvalidIntegerNumberException : public TInvalidNumberException {
public:
  TInvalidIntegerNumberException(const olxstr& location, const olxstr& str):
    TInvalidNumberException(location, str)  {}
  virtual const char* GetNiceName() const {  return "Invalid integer format";  }
  virtual IEObject* Replicate() const {  return new TInvalidIntegerNumberException(*this);  }
};

class TInvalidFloatNumberException : public TInvalidNumberException {
public:
  TInvalidFloatNumberException(const olxstr& location, const olxstr& str):
    TInvalidNumberException(location, str)  {}
  virtual const char* GetNiceName() const {  return "Invalid float format";  }
  virtual IEObject* Replicate() const {  return new TInvalidFloatNumberException(*this);  }
};
//.........................................................................................

class TOutOfMemoryException: public TBasicException {
public:
  TOutOfMemoryException(const olxstr& location) :
    TBasicException(location, EmptyString )  {}
  virtual const char* GetNiceName() const {  return "Out of memory";  }
  virtual IEObject* Replicate() const {  return new TOutOfMemoryException(*this);  }
};

EndEsdlNamespace()
#endif
