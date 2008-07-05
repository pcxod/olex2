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
  virtual void CreationProtection()  {  };
protected:
  TBasicException(const TBasicException& toReplicate) {
    this->Message = toReplicate.Message;
    this->Location = toReplicate.Location;
    if( toReplicate.Cause != NULL )
      this->Cause = (TBasicException*)toReplicate.Cause->Replicate();
    else
      this->Cause = NULL;
  }
public:
  TBasicException()  {
    Cause = NULL;
  }

  TBasicException(const olxstr& location, const TExceptionBase& cause, const olxstr& msg=EmptyString)  {
    if( msg.IsEmpty() )  Message = "Inherited exception";
    else                 Message = msg;
    Location = location;
    Cause = (TBasicException*)cause.GetException()->Replicate();
  }
  /* caution: the expeceted object is an instance from a call to Replicate() !
    and will be deleted
  */
  TBasicException(const olxstr& location, IEObject* cause)  {
    Message = "Inherited exception";
    Location = location;
    Cause = (TBasicException*)cause;
  }

  TBasicException(const olxstr& location, const olxstr& Msg)  {
      Message = Msg;
      Location = location;
      Cause = NULL;
  }

  virtual ~TBasicException()  {
    if( Cause != NULL )  delete Cause;
  }

  inline const olxstr& GetError()         const {  return Message;  }
  inline const olxstr& GetLocation()      const {  return Location;  }
  inline TBasicException* GetCause()         const {  return Cause;  }
  // traces back to the original cause
  TBasicException* GetSource()         const;
  virtual IEObject* Replicate()  const = 0;
  template <class SC, class T>
    void GetStackTrace( TTStrList<SC,T>& output )  const  {
      TBasicException const* cause = this;
      //TPtrList<TBasicException const> list;
      while( cause != NULL )  {
//        if( cause->GetCause() != NULL )
//          output.Insert(0, cause->GetLocation() );
//        else
        output.Insert(0, cause->GetFullMessage() );
        cause = cause->GetCause();
      }
    }
  olxstr GetFullMessage()  const;
};

class TIndexOutOfRangeException: public TBasicException  {
  long Index, Min, Max;
public:
  TIndexOutOfRangeException(const TIndexOutOfRangeException& toReplicate) :
      TBasicException(toReplicate)  {
    this->Index = toReplicate.Index;
    this->Min = toReplicate.Min;
    this->Max = toReplicate.Max;
  }

  TIndexOutOfRangeException(const olxstr& location, long index, long min, long max):
      TBasicException(location, olxstr("[") << min << ".." << max << "]: " << index )  {
    Index = index;
    Min = min;
    Max = max;
  }

  static inline void ValidateRange(const olxstr& location, long index, long min, long max)  {
    if( index < min || index >= max )
      throw TIndexOutOfRangeException(location, index, min, max);
  }

  inline long GetIndex() const  {  return Index;  }
  inline long GetMin() const  {  return Min;  }
  inline long GetMax() const  {  return Max;  }

  virtual IEObject* Replicate()  const    {  return new TIndexOutOfRangeException(*this);  }
};

class TFunctionFailedException: public TBasicException {
public:
  TFunctionFailedException(const olxstr& location, const olxstr& msg) :
    TBasicException(location, msg )  {    }
  TFunctionFailedException(const olxstr& location, const TExceptionBase& cause, const olxstr& msg=EmptyString) :
    TBasicException(location, cause, msg )  {    }
  TFunctionFailedException(const olxstr& location, IEObject* cause) :
    TBasicException(location, cause )  {    }

  virtual IEObject* Replicate()  const    {  return new TFunctionFailedException(*this);  }
};

class TInvalidArgumentException: public TBasicException  {
public:
  TInvalidArgumentException(const olxstr& location, const olxstr& argName):
    TBasicException(location, argName )  {  }

  inline const olxstr& GetArgumentName()  const  {  return GetError();  }
  virtual IEObject* Replicate()  const    {  return new TInvalidArgumentException(*this);  }

};

class TNotImplementedException: public TBasicException  {
public:
  TNotImplementedException(const olxstr& location) :
    TBasicException(location, EmptyString )  {    }

  virtual IEObject* Replicate()  const    {  return new TNotImplementedException(*this);  }
};


class TIOExceptionBase: public TBasicException  {
public:
  TIOExceptionBase(const olxstr& location, const olxstr &msg):
    TBasicException(location, msg )  {    }
};

class TFileExceptionBase: public TIOExceptionBase  {
  olxstr FileName;
public:
  TFileExceptionBase(const TFileExceptionBase& toReplicate) :
      TIOExceptionBase(toReplicate)  {
    this->FileName = toReplicate.FileName;
  }

  TFileExceptionBase(const olxstr& location, const olxstr& fileName, const olxstr& reason) :
      TIOExceptionBase(location, olxstr(fileName) << ' ' << reason)  {
    FileName = fileName;
  }

  inline const olxstr& GetFileName()  const  {  return FileName;  }

  virtual IEObject* Replicate()  const    {  return new TFileExceptionBase(*this);  }
};

class TFileDoesNotExistException: public TFileExceptionBase  {
public:
  TFileDoesNotExistException(const olxstr& location, const olxstr& fileName) :
    TFileExceptionBase(location, fileName, EmptyString )  {    }

  virtual IEObject* Replicate()  const  {  return new TFileDoesNotExistException(*this);  }
};

class TEmptyFileException: public TFileExceptionBase  {
public:
  TEmptyFileException(const olxstr& location, const olxstr& fileName) :
    TFileExceptionBase(location, fileName, EmptyString )  {    }

  virtual IEObject* Replicate()  const  {  return new TEmptyFileException(*this);  }
};

class TMathExceptionBase: public TBasicException  {
public:
  TMathExceptionBase(const olxstr& location, const olxstr& msg):
    TBasicException(location, msg )  {    }

  virtual IEObject* Replicate()  const  {  return new TMathExceptionBase(*this);  }
};

class TDivException: public TMathExceptionBase {
public:
  TDivException(const olxstr& location):
    TMathExceptionBase(location, EmptyString )  {  }

  virtual IEObject* Replicate()  const  {  return new TDivException(*this);  }
};

class TOutOfMemoryException: public TBasicException  {
public:
  TOutOfMemoryException(const olxstr& location) :
    TBasicException(location, EmptyString )  {    }

  virtual IEObject* Replicate()  const  {  return new TOutOfMemoryException(*this);  }
};

EndEsdlNamespace()
#endif
