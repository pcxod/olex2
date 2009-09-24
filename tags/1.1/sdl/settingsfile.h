#ifndef settingsfileH
#define settingsfileH

#include "edict.h"
#include "estrlist.h"
//---------------------------------------------------------------------------
BeginEsdlNamespace()
/* reads settings file, considering lines having no assignment operator or lines starting from
'#' as comments. The white spaces will be trimmed for parameter names and values. 
The file structure will be peserved upon saving (besides white spaces for param name/value).
If there are duplicate entries, the most recent value will be kept
(c) O Dolomanov, 2004-2009 */
class TSettingsFile: public IEObject  {
  olxdict<olxstr, olxstr, olxstrComparator<true> > Params;
  TTOStringList<olxstr,bool,TObjectStrListData<olxstr,bool> > Lines;
protected:
  void Clear()  {  
    Params.Clear();  
    Lines.Clear();
  }
public:
  TSettingsFile(){}
  TSettingsFile(const olxstr& fileName)  {  LoadSettings(fileName);  }
  void LoadSettings(const olxstr& fileName);
  void SaveSettings(const olxstr& fileName);
  virtual ~TSettingsFile() {}

  inline int ParamCount() const {  return Params.Count();  }
  // 
  template <class SC>
  inline const olxstr& GetParam(const SC& paramName, const olxstr& defVal=EmptyString ) const {
    int index = Params.IndexOf(paramName);
    return index == -1 ? defVal : Params.GetValue(index);
  }
  // convinience operator, same as GetParam
  template <class SC>
  inline const olxstr& operator [] (const SC& pn) const {  return GetParam(pn);  }
  // creates a param if does not exist
  template <class SC>
  inline olxstr& operator [] (const SC& paramName) {
    int index = Params.IndexOf(paramName);
    if( index != -1 )
      return Params.GetValue(index);
    else  {
      Lines.Add(paramName, true);
      return Params.Add(paramName, EmptyString);
    }
  }
  
  template <class SC>
  inline bool HasParam(const SC& paramName) const {  return Params.HasKey(paramName);  }
  // updates or creates new parameter
  template <class SC>
  void SetParam(const SC& paramName, const olxstr& val)  {
    int index = Params.IndexOf(paramName);
    if( index != -1 )
      Params.GetValue(index) = val;
    else  {
      Lines.Add(paramName, true);
      Params.Add(paramName, val);
    }
  }
};

EndEsdlNamespace()
#endif
