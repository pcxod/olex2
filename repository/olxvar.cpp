/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "olxvar.h"
#include "egc.h"
#include "efile.h"

TOlxVars* TOlxVars::Instance = NULL;
#ifdef _PYTHON
#include "pyext.h"
char TOlxPyVar::svmn[] = "setValue";
char TOlxPyVar::gvmn[] = "getValue";

TOlxPyVar::TOlxPyVar(PyObject* obj)  {
  Str = NULL;
  Obj = NULL;
  InitObject(obj);
}
//.............................................................................
TOlxPyVar::TOlxPyVar(const TOlxPyVar& oo)  {
  Type = oo.Type;
  Str = (oo.Str == NULL) ? NULL : new olxstr(*oo.Str);
  Obj = oo.Obj;
  if( Obj != NULL )  Py_INCREF(Obj);
}
//.............................................................................
TOlxPyVar::~TOlxPyVar()  {
  if( Str != NULL )  delete Str;
  if( Obj != NULL && Obj != Py_None)
    Py_DECREF(Obj);
}
//.............................................................................
void TOlxPyVar::InitObject(PyObject* obj)  {
  if( Obj != NULL )  Py_DECREF(Obj);
  Obj = obj;
  Py_INCREF(obj);

  Type = potNone;

  if( PyObject_HasAttrString(obj, gvmn) )  Type |= potHasGetter;
  if( PyObject_HasAttrString(obj, svmn) )  Type |= potHasSetter;

  PyObject* defv = NULL;
  PyTypeObject *ot;

  if( HasGet() )  {
    defv = PyObject_CallMethod(Obj, gvmn, NULL);
    ot = defv->ob_type;
  }
  else
    ot = obj->ob_type;

  if( ot == &PyInt_Type )          Type |= potInt;
  else if( ot == &PyBool_Type )    Type |= potBool;
  else if( ot == &PyFloat_Type )   Type |= potFloat;
  else if( ot == &PyString_Type )  Type |= potString;
  else if( ot == &PyUnicode_Type ) Type |= potString;
  else if( ot == &PyComplex_Type ) Type |= potComplex;
  else  Type |= potUndefined;

  if( defv != NULL )  Py_DECREF(defv);
}
//.............................................................................
PyObject* TOlxPyVar::ObjectValue(PyObject* obj)  {
  return PyObject_HasAttrString(obj, svmn)
    ? PyObject_CallMethod(obj, gvmn, NULL) : obj;
}
//.............................................................................
PyObject* TOlxPyVar::GetObjVal()  {
  if( Obj != NULL )  {
    PyObject *rv = HasGet() ? PyObject_CallMethod(Obj, gvmn, NULL) : Obj;
    if( !HasGet() )
      Py_INCREF(rv);
    return rv;
  }
  else  if( Str != NULL )
    return PythonExt::BuildString(*Str);
  else {
    return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
      "Uninitialised object");
  }
}
//.............................................................................
void TOlxPyVar::Set(PyObject* obj)  {
  if( Obj == NULL && obj == Py_None )  {
    Py_DECREF(PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo,
      "A valid object is expected"));
    return;
  }
  if( Obj != NULL )  {
    if( obj == Py_None )  {
      if( !HasSet() )  {
        Py_DECREF(PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo,
          "Missing set method"));
        return;
      }
    }
    if( Obj->ob_type == obj->ob_type )  {
      Py_DECREF(Obj);
      Py_INCREF(obj);
      Obj = obj;
    }
    else if( !HasSet() )  // replace object type
      InitObject(obj);
    else  {
      PyObject *rv = PyObject_CallMethod(Obj, svmn, "O", obj);
      if( rv != NULL )  Py_DECREF(rv);
    }
  }
  else  {
    if( obj->ob_type == &PyString_Type || obj->ob_type == &PyUnicode_Type )
      *Str = PythonExt::ParseStr(obj);
    else  {
      Py_DECREF(PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
        "Uninitialised object"));
      return;
    }
  }
}
//.............................................................................
void TOlxPyVar::Set(const olxstr& str)  {
  if( Obj == NULL )  {
    if( Str == NULL )  Str = new olxstr(str);
    else               *Str = str;
  }
  else  {
    if( HasSet() )  {
      PyObject *arg = PythonExt::BuildString(str);
      PyObject *rv = PyObject_CallMethod(Obj, svmn, "O", arg);
      if( rv != NULL )  Py_DECREF(rv);
      Py_DECREF(arg);
    }
    else  {  // replace a primitive value with new object
      PyObject * arg;
      if( (Type & potBool) != 0  )  {
        bool v = str.Equalsi(TrueString());
        if( !v && !str.Equalsi(FalseString()) )  {
          olxstr err(olxstr("Boolean is expected, got '") << str << '\'');
          Py_DECREF(
            PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo, err));
          throw TInvalidArgumentException(__OlxSourceInfo, err);
        }
        arg = PyBool_FromLong(v);
      }
      else if( (Type & potInt) != 0  )  {
        if( !str.IsNumber() )  {
          olxstr err(olxstr("Integer is expected, got '") << str << '\'');
          Py_DECREF(
            PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo, err));
          throw TInvalidArgumentException(__OlxSourceInfo, err);
        }
        arg = Py_BuildValue("i", str.ToInt());
      }
      else if( (Type & potFloat) != 0 )  {
        if( !str.IsNumber() )  {
          olxstr err(olxstr("Float is expected, got '") << str << '\'');
          Py_DECREF(
            PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo, err));
          throw TInvalidArgumentException(__OlxSourceInfo, err);
        }
        arg = Py_BuildValue("d", str.ToDouble());
      }
      else if( (Type & potString) != 0 )
        arg = PythonExt::BuildString(str);
      else  {
        olxstr err(olxstr("String is expected, got '") << str << '\'');
        Py_DECREF(
          PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo, err));
        throw TInvalidArgumentException(__OlxSourceInfo, err);
      }
      Py_DECREF(Obj);
      Obj = arg;
    }
  }
}
//.............................................................................
const olxstr& TOlxVars::GetVarStr(size_t index)  {
  volatile olx_scope_cs cs(CS());
  TOlxPyVar& oo = Instance->Vars.GetObject(index);
  if( oo.GetStr() != NULL )  return *oo.GetStr();
  PyObject *po = oo.GetObjVal();
  double fv;
  if( po->ob_type == &PyString_Type || po->ob_type == &PyUnicode_Type )
    return TEGC::New<olxstr>( PythonExt::ParseStr(po) );
  else if( po->ob_type == &PyFloat_Type && PyArg_Parse(po, "d", &fv) )
    return TEGC::New<olxstr>(fv);
  else
    return TEGC::New<olxstr>(PyObject_REPR(po));
}
//.............................................................................
TOlxVars::TOlxVars()  {
  if (Instance != NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  Instance = this;
  OnVarChange = &Actions.New("OnVarChange");
//  TEGC::AddP(this);  // we cannot do this, as Pyhon might be Finalised beforehand!!!
}
#endif // _PYTHON
//............................................................................
void olxvar_funUnsetVar(const TStrObjList& Params, TMacroError &E)  {
  TOlxVars::UnsetVar(Params[0]);
}
//.............................................................................
void olxvar_funGetVar(const TStrObjList& Params, TMacroError &E)  {
  const size_t ind = TOlxVars::VarIndex(Params[0]);
  if (ind == InvalidIndex) {
    if (Params.Count() == 2)
      E.SetRetVal(Params[1]);
    else {
      E.ProcessingError(__OlxSrcInfo,
        "Could not locate specified variable: ").quote() << Params[0];
    }
  }
  else
    E.SetRetVal(TOlxVars::GetVarStr(ind));
}
void olxvar_funSetVar(const TStrObjList& Params, TMacroError &E) {
  TOlxVars::SetVar(Params[0], Params[1]);
}
void olxvar_funIsVar(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal(TOlxVars::IsVar(Params[0]));
}
void olxvar_funFlush(const TStrObjList& Params, TMacroError &E) {
  TStrList settings;
  olxstr mask = (Params[1].IsEmpty() ? olxstr('*') : Params[1]);
  size_t from = Params.Count() == 3 ? Params[2].ToSizeT() : 0;
  Wildcard wc(Params[1]);
  for (size_t i=0; i < TOlxVars::VarCount(); i++) {
    olxstr vn = TOlxVars::GetVarName(i);
    if (!wc.DoesMatch(vn) || vn.Length() < from) continue;
    settings.Add(vn.SubStringFrom(from)) << '=' << TOlxVars::GetVarStr(i);
  }
  TEFile::WriteLines(Params[0], TCStrList(settings));
}
TLibrary *TOlxVars::ExportLibrary(const olxstr &name, TLibrary *_l) {
  TLibrary *l = _l == NULL ? new TLibrary(name) : _l;
  l->Register(
    new TStaticFunction(&olxvar_funSetVar,
      "SetVar", fpTwo, "Sets the value of the specified variable"));
  l->Register(
    new TStaticFunction(&olxvar_funGetVar,
      "GetVar", fpOne|fpTwo, "Gets the value of the specified variable. If the"
      " variable does not exist and no default value is provided - an error "
      "occurs"));
  l->Register(
    new TStaticFunction(&olxvar_funIsVar,
      "IsVar", fpOne, "Checks if the specified variable exists"));
  l->Register(
    new TStaticFunction(&olxvar_funUnsetVar,
      "UnsetVar", fpOne, "Removes the specified variable"));
  l->Register(
    new TStaticFunction(&olxvar_funFlush,
      "Flush", fpTwo|fpThree, "Saves variables to a file. The second argument is a "
      "mask in the form '*' 'settings.*' etc, if the 3rd argument [0] is "
      "specified the variable names used as substringFrom(3rd arg); note that "
      "if the name is shorter that the value of the 3rd argument - the value "
      "is not saved."));
  return l;
}

