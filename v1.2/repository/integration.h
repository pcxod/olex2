#ifndef Olex_Integration_H_
#define Olex_Integration_H_

#ifdef HAVE_GCCVISIBILITYPATCH
  #define DllExport __attribute__ ((visibility("default")))
#else
  #ifdef _MSC_VER
    #define DllImport   __declspec( dllimport )
    #define DllExport   __declspec( dllexport )
  #endif
  #ifdef __BORLANDC__
    #define DllExport __export
  #endif
  #ifdef __GNUC__
    #define DllExport 
  #endif
#endif



#include "library.h"

namespace olex
{
  const short mtNone      = 0,
              mtInfo      = 1,
              mtWarning   = 2,
              mtError     = 3,
              mtException = 4;

  class IOlexProcessor  {
    static IOlexProcessor* Instance;
  public:
    IOlexProcessor()  {  Instance = this;  }
    virtual ~IOlexProcessor()  {  ;  }
    virtual bool executeMacro(const olxstr& cmdLine) = 0;
    virtual void print(const olxstr& Text, const short MessageType = mtNone) = 0;
    virtual bool executeFunction(const olxstr& funcName, olxstr& retValue) = 0;
    // returns a value, which should be deleted, of the TPType <> type
    virtual IEObject* executeFunction(const olxstr& funcName) = 0;
    virtual TLibrary&  GetLibrary() = 0;
    virtual bool registerCallbackFunc(const olxstr& cbEvent, ABasicFunction* fn) = 0;
    virtual void unregisterCallbackFunc(const olxstr& cbEvent, const olxstr& funcName) = 0;

    virtual const olxstr& getDataDir() const = 0;
    virtual const olxstr& getVar(const olxstr &name, const olxstr &defval=NullString) const = 0;
    virtual void setVar(const olxstr &name, const olxstr &val) const = 0;

    static const olxstr SGListVarName;
    static DllExport IOlexProcessor* GetInstance()  {  return Instance;  }
  };

  class IOlexRunnable : public IEObject  {
  public:
    virtual ~IOlexRunnable()  {  ;  }
    virtual bool Run( IOlexProcessor& olexProcessor ) = 0;
  };
  class OlexPort : public IEObject  {
    static IOlexRunnable *OlexRunnable;
  protected:
  public:
    OlexPort()  {  OlexRunnable = NULL;  }
    ~OlexPort()  {
      if( OlexRunnable )  delete OlexRunnable;
    }
  //............................................................................
    static void SetOlexRunnable( IOlexRunnable* o_r )  {  OlexRunnable = o_r;  }
  //............................................................................
    static DllExport IOlexRunnable* GetOlexRunnable();
  //............................................................................
  };

};  // end namespace olex

#endif
 
