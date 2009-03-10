#ifndef settingsfileH
#define settingsfileH

#include "estlist.h"
//---------------------------------------------------------------------------
BeginEsdlNamespace()

class TSettingsFile: public IEObject  {
  TSStrStrList<olxstr,true> Params;
protected:
  void Clear();
public:
  TSettingsFile(){  ;  }
  TSettingsFile( const olxstr& fileName );
  void LoadSettings(const olxstr& fileName);
  void SaveSettings(const olxstr& fileName);
  virtual ~TSettingsFile();

  inline int ParamCount() const {  return Params.Count();  }
  inline const olxstr& ParamValue(const olxstr& paramName, const olxstr &defVal = EmptyString ) const {
    int ind = Params.IndexOfComparable(paramName);
    if( ind == -1 ) return defVal;
    const olxstr &rv = Params.GetObject(ind);
    return rv.IsEmpty() ? defVal : rv;

  }
  inline bool ParamExists(const olxstr& paramName)  const
               {  return (Params.IndexOfComparable(paramName) != -1);  }
  // updates or creates new parameter
  void UpdateParam(const olxstr& paramName, const olxstr& val);
};

EndEsdlNamespace()
#endif
