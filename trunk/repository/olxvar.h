#ifndef olxvarH
#define olxvarH

#include "estlist.h"
#include "actions.h"
#include "bapp.h"

#ifndef _NO_PYTHON
  #include "pyext.h"
// python object types
const short potNone        = 0x0000,
            potBool        = 0x0001,
            potInt         = 0x0002,
            potFloat       = 0x0004,
            potString      = 0x0008,
            potComplex     = 0x0010,
            potUndefined   = 0x0080,
            potHasGetter   = 0x0100,
            potHasSetter   = 0x0200;

class TOlxPyVar {
  olxstr *Str;
  PyObject *Obj;
  short Type;
  static  char svmn[], gvmn[];
  inline bool HasGet() const {  return (Type&potHasGetter)!=0;  }
  inline bool HasSet() const {  return (Type&potHasSetter)!=0;  }
  void InitObject(PyObject* obj);
public:
  TOlxPyVar()  {
    Type = potNone;
    Str = NULL;
    Obj = NULL;
  }

  TOlxPyVar(const TOlxPyVar& oo);

  TOlxPyVar(const olxstr& val)  {
    Str = new olxstr(val);
    Type = potString;
    Obj = NULL;
  }

  TOlxPyVar(PyObject* obj);

  inline TOlxPyVar& operator = (const TOlxPyVar& oo)  {
    if( Str != NULL )  delete Str;
    if( Obj != NULL ) Py_DecRef(Obj);
    Type = oo.Type;
    Str = (oo.Str == NULL) ? NULL : new olxstr(*oo.Str);
    Obj = oo.Obj;
    if( Obj != NULL )  Py_INCREF(Obj);
    return *this;
  }
  
  ~TOlxPyVar();
  inline PyObject* GetObj()  {  return Obj;  }
  PyObject* GetObjVal();
  // returns wrapped value of the object or the object if of primitive type
  static PyObject* ObjectValue(PyObject* obj);

  inline olxstr* GetStr()  {  return Str;  }
  void Set(PyObject* obj);
  void Set(const olxstr& str);
};

struct TOlxVarChangeData : public IEObject  {
  const olxstr& str_val, var_name;
  PyObject* py_val;
  TOlxVarChangeData(const olxstr& v_name, const olxstr& v_val, PyObject* p_val) :
    var_name(v_name), str_val(v_val), py_val(p_val) {}
};
   
class TOlxVars : public IEObject  {
  static TOlxVars* Instance;
  TSStrObjList<olxstr,TOlxPyVar, true> Vars;

  template <class T>
  inline void _SetVar(const T& name, const olxstr& value)  {
    const size_t ind = Vars.IndexOf(name);
    try  {
      if( ind != InvalidIndex )
        Vars.GetObject(ind).Set(value);
      else
        Vars.Add(name, value);
    }
    catch( const TExceptionBase& exc)  {
      throw TFunctionFailedException(__OlxSourceInfo, exc,
        olxstr("Exception occured while setting variable '") << name <<'\'');
    }
    TOlxVarChangeData vcd(name, value, NULL);
    OnVarChange->Execute(NULL, &vcd);
  }
  template <class T>
  inline void _SetVar(const T& name, PyObject* value)  {
    const size_t ind = Vars.IndexOf(name);
    try  {
      if( ind != InvalidIndex )
        Vars.GetObject(ind).Set(value);
      else
        Vars.Add(name, value);
    }
    catch( const TExceptionBase& exc)  {
      throw TFunctionFailedException(__OlxSourceInfo, exc,
        olxstr("Exception occured while setting variable '") << name <<'\'');
    }
    TOlxVarChangeData vcd(name, EmptyString(), value);
    OnVarChange->Execute(NULL, &vcd);
  }

  inline const olxstr& _FindName(PyObject* value)  {
    for( size_t i=0; i < Vars.Count(); i++ )
      if( Vars.GetObject(i).GetObj()  == value )
        return Vars.GetString(i);
    return EmptyString();
  }

  template <class T>
  inline bool _UnsetVar(const T& name)  {
    const size_t ind = Vars.IndexOf(name);
    if( ind != InvalidIndex )  {
      Vars.Delete(ind);
      return true;
    }
    return false;
  }
  TOlxVars();
  ~TOlxVars()  {  Instance = NULL;  }
  TActionQList Actions;
public:

  TActionQueue *OnVarChange;

  static inline TOlxVars* GetInstance()  {  return Instance;  }
  static inline TOlxVars& Init()  {  return *(new TOlxVars());  }
  static inline void Finalise()   {  if( Instance != NULL )  delete Instance;  }

  static inline size_t VarCount()  {  return Instance != NULL ? Instance->Vars.Count() : 0;  }

  static inline PyObject* GetVarValue(size_t index) {
    return Instance->Vars.GetObject(index).GetObjVal();
  }
  static inline PyObject* GetVarWrapper(size_t index) {
    return Instance->Vars.GetObject(index).GetObj();
  }
  static const olxstr& GetVarStr(size_t index);

  static inline const olxstr& GetVarName(size_t index) {
    return Instance->Vars.GetKey(index);
  }

  template <class T>
  static inline void SetVar(const T& name, const olxstr& value)  {
    if( Instance == NULL )  new TOlxVars();
    Instance->_SetVar(name, value);
  }

  template <class T>
  static inline bool UnsetVar(const T& name)  {
    if( Instance == NULL )  return false;
    return Instance->_UnsetVar(name);
  }

  template <class T>
  static inline void SetVar(const T& name, PyObject *value)  {
    if( Instance == NULL )  new TOlxVars();
    Instance->_SetVar(name, value);
  }

  template <class T>
  static inline bool IsVar(const T& name) {
    return (Instance == NULL) ? false : Instance->Vars.IndexOf(name) != InvalidIndex;
  }
  template <class T>
  static inline size_t VarIndex(const T& name) {
    return (Instance == NULL) ? InvalidIndex : Instance->Vars.IndexOf(name);
  }
  static inline const olxstr& FindVarName(PyObject *pyObj) {
    return (Instance == NULL) ? EmptyString() : Instance->_FindName(pyObj);
  }
};
#else  // _NO_PYTHON
// use a very thin implementation with no python...
class TOlxVars : public IEObject  {
  static TOlxVars* Instance;
  TSStrObjList<olxstr,olxstr, true> Vars;

  template <class T>
  inline void _SetVar(const T& name, const olxstr& value)  {
    const size_t ind = Vars.IndexOf(name);
    if( ind != InvalidIndex )
      Vars.GetObject(ind) = value;
    else
      Vars.Add(name, value);
  }
  template <class T>
  inline void _UnsetVar(const T& name)  {
    const size_t ind = Vars.IndexOf(name);
    if( ind != InvalidIndex )  Vars.Delete(ind);
  }
public:
  static inline TOlxVars* GetInstance()  {  return Instance;  }
  static inline TOlxVars& Init()  {  return *(new TOlxVars());  }
  static inline void Finalise()  {  if( Instance != NULL )  delete Instance;  }

  static inline size_t VarCount()  {  return Instance != NULL ? Instance->Vars.Count() : 0;  }

  static inline const olxstr& GetVarName(size_t index) {
    return Instance->Vars.GetKey(index);
  }
  static inline const olxstr& GetVarStr(size_t index) {
    return Instance->Vars.GetObject(index);
  }

  template <class T>
  static inline void SetVar(const T& name, const olxstr& value)  {
    if( Instance == NULL )  new TOlxVars();
    Instance->_SetVar(name, value);
  }
  template <class T>
  static inline void UnsetVar(const T& name)  {
    if( Instance == NULL )  return;
    Instance->_UnsetVar(name);
  }
  template <class T>
  static inline bool IsVar(const T& name) {
    return (Instance == NULL) ? false : Instance->Vars.IndexOf(name) != InvalidIndex;
  }
  template <class T>
  static inline size_t VarIndex(const T& name) {
    return (Instance == NULL) ? InvalidIndex : Instance->Vars.IndexOf(name);
  }
};
#endif  // _NO_PYTHON
/* a convinience object: sets var withi given name on the creation and unsets on destruction
thread safe - synchronised */
class OlxStateVar  {
  olxstr name, prev_val;
  bool was_set, restored;
public:
  OlxStateVar(const olxstr& _name) : name(_name), restored(false)  {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    size_t i = TOlxVars::VarIndex(_name);
    if( i != InvalidIndex )  {
      prev_val = TOlxVars::GetVarStr(i);
      was_set = true;
    }
    TOlxVars::SetVar(name, TrueString());
  }
  ~OlxStateVar()  {  Restore();  }
  void Restore()  {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    if( restored )  return;
    if( was_set )
      TOlxVars::SetVar(name, prev_val);
    else
      TOlxVars::UnsetVar(name);
    restored = true;
  }
};
#endif
