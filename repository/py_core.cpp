#undef GetObject

#include "py_core.h"

#include "xapp.h"
#include "efile.h"
#include "settingsfile.h"
#include "url.h"
#include "httpfs.h"
#include "integration.h"
#include "olxvar.h"

using namespace olex;

PyObject* pyVarValue(PyObject* self, PyObject* args)  {
  olxstr varName;
  PyObject* defVal = NULL;
  if( !PythonExt::ParseTuple(args, "w|O", &varName, &defVal) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  int i = TOlxVars::VarIndex(varName);
  if( i == -1 )  {
    if( defVal != NULL )  {
      TOlxVars::SetVar(varName, defVal);
      Py_IncRef( defVal );
      return TOlxPyVar::ObjectValue(defVal);
    }
    else  {
      PyErr_SetObject(PyExc_KeyError, PythonExt::BuildString("undefined key name"));
      Py_INCREF(Py_None);
      return Py_None;
    }
  }
  return TOlxVars::GetVarValue(i);
}
//..............................................................................
PyObject* pyVarObject(PyObject* self, PyObject* args)  {
  olxstr varName;
  PyObject* defVal = NULL;
  if( !PythonExt::ParseTuple(args, "w|O", &varName, &defVal) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  int i = TOlxVars::VarIndex(varName);
  if( i == -1 )  {
    if( defVal != NULL )  {
      TOlxVars::SetVar(varName, defVal);
      Py_IncRef( defVal );
      return defVal;
    }
    else  {
      PyErr_SetObject(PyExc_KeyError, PythonExt::BuildString("undefined key name"));
      Py_INCREF(Py_None);
      return Py_None;
    }
  }
  PyObject *rv = TOlxVars::GetVarWrapper(i);
  if( rv == NULL )  rv = Py_None;
  Py_IncRef(rv);
  return rv;
}
//..............................................................................
PyObject* pyIsVar(PyObject* self, PyObject* args)  {
  olxstr varName;
  if( !PythonExt::ParseTuple(args, "w", &varName) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return Py_BuildValue("b", TOlxVars::IsVar(varName) );
}
//..............................................................................
PyObject* pyVarCount(PyObject* self, PyObject* args)  {
  return Py_BuildValue("i", TOlxVars::VarCount() );
}
//..............................................................................
PyObject* pyGetVar(PyObject* self, PyObject* args)  {
  int varIndex;
  if( !PyArg_ParseTuple(args, "i", &varIndex) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return TOlxVars::GetVarValue(varIndex);
}
//..............................................................................
PyObject* pyGetVarName(PyObject* self, PyObject* args)  {
  int varIndex;
  if( !PyArg_ParseTuple(args, "i", &varIndex) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return PythonExt::BuildString( TOlxVars::GetVarName(varIndex) );
}
//..............................................................................
PyObject* pyFindGetVarName(PyObject* self, PyObject* args)  {
  PyObject *val;
  if( !PyArg_ParseTuple(args, "O", &val) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return PythonExt::BuildString( TOlxVars::FindVarName(val) );
}
//..............................................................................
PyObject* pySetVar(PyObject* self, PyObject* args)  {
  olxstr varName;
  PyObject *varValue = NULL;
  if( !PythonExt::ParseTuple(args, "wO", &varName, &varValue) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  TOlxVars::SetVar(varName, varValue);
  Py_INCREF(Py_None);
  return Py_None;
}
//..............................................................................
PyObject* pyUnsetVar(PyObject* self, PyObject* args)  {
  olxstr varName;
  if( !PythonExt::ParseTuple(args, "w", &varName) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  if( TOlxVars::UnsetVar(varName) )  {
    Py_INCREF(Py_True);
    return Py_True;
  }
  else  {
    Py_INCREF(Py_False);
    return Py_False;
  }
}
//..............................................................................
PyObject* pyGetPlugins(PyObject* self, PyObject* args)  {
  TStrList rv(IOlexProcessor::GetInstance()->GetPluginList());
  PyObject* af = PyTuple_New( rv.Count() );
  for( int i=0; i < rv.Count(); i++ )
    PyTuple_SetItem(af, i, PythonExt::BuildString(rv[i]) );
  return af;
}
//..............................................................................
PyObject* pyExpFun(PyObject* self, PyObject* args)  {
  TBasicFunctionPList functions;
  IOlexProcessor::GetInstance()->GetLibrary().ListAllFunctions( functions );
  PyObject* af = PyTuple_New( functions.Count() ), *f;
  for( int i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    f = PyTuple_New(3);
    PyTuple_SetItem(af, i, f );

    PyTuple_SetItem(f, 0, PythonExt::BuildString(func->GetQualifiedName()) );
    PyTuple_SetItem(f, 1, PythonExt::BuildString(func->GetSignature()) );
    PyTuple_SetItem(f, 2, PythonExt::BuildString(func->GetDescription()) );
  }
  return af;
}
//..............................................................................
PyObject* pyExpMac(PyObject* self, PyObject* args)  {
  TBasicFunctionPList functions;
  IOlexProcessor::GetInstance()->GetLibrary().ListAllMacros( functions );
  PyObject* af = PyTuple_New( functions.Count() ), *f, *s;
  for( int i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    f = PyTuple_New(4);
    PyTuple_SetItem(af, i, f );
    PyTuple_SetItem(f, 0, PythonExt::BuildString(func->GetQualifiedName()) );
    PyTuple_SetItem(f, 1, PythonExt::BuildString(func->GetSignature()) );
    PyTuple_SetItem(f, 2, PythonExt::BuildString(func->GetDescription()) );
    s = PyDict_New();
    PyTuple_SetItem(f, 3, s );
    for(int j=0; j < func->GetOptions().Count(); j++ )  {
      PyDict_SetItem(s, PythonExt::BuildString(func->GetOptions().GetComparable(j)),
                        PythonExt::BuildString(func->GetOptions().GetObject(j)) );
    }
  }
  return af;
}
//..............................................................................
PyObject* pyTranslate(PyObject* self, PyObject* args)  {
  olxstr str;
  if( !PythonExt::ParseTuple(args, "w", &str) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  return PythonExt::BuildString(IOlexProcessor::GetInstance()->TranslateString(str));
}
//..............................................................................
PyObject* pyDescRef(PyObject* self, PyObject* args)  {
  TStrList rv;
  TXApp::GetInstance().XFile().GetRM().Describe(rv);
  return PythonExt::BuildString(rv.Text('\n'));
}
//..............................................................................
PyObject* pyRefModel(PyObject* self, PyObject* args)  {
  bool calc_connectivity = false;
  if( PyTuple_Size(args) != 0 )  {
    if( !PyArg_ParseTuple(args, "b", &calc_connectivity) )  {
      Py_INCREF(Py_None);
      return Py_None;
    }
  }
  return TXApp::GetInstance().XFile().GetRM().PyExport(calc_connectivity);
}
//..............................................................................
PyObject* pyHklStat(PyObject* self, PyObject* args)  {
  TXApp& xapp = TXApp::GetInstance();
  RefinementModel::HklStat hs = xapp.XFile().GetRM().GetMergeStat();
  PyObject* out = PyDict_New();
  PyDict_SetItemString(out, "TotalReflections", Py_BuildValue("i", hs.TotalReflections));
  PyDict_SetItemString(out, "UniqueReflections", Py_BuildValue("i", hs.UniqueReflections));
  PyDict_SetItemString(out, "FriedelOppositesMerged", Py_BuildValue("b", hs.FriedelOppositesMerged));
  PyDict_SetItemString(out, "InconsistentEquivalents", Py_BuildValue("i", hs.InconsistentEquivalents));
  PyDict_SetItemString(out, "SystematicAbsencesRemoved", Py_BuildValue("i", hs.SystematicAbsentcesRemoved));
  PyDict_SetItemString(out, "MinD", Py_BuildValue("d", hs.MinD));
  PyDict_SetItemString(out, "MaxD", Py_BuildValue("d", hs.MaxD));
  PyDict_SetItemString(out, "LimDmin", Py_BuildValue("d", hs.LimDmin));
  PyDict_SetItemString(out, "LimDmax", Py_BuildValue("d", hs.LimDmax));
  PyDict_SetItemString(out, "FilteredOff", Py_BuildValue("i", hs.FilteredOff));
  PyDict_SetItemString(out, "OmittedByUser", Py_BuildValue("i", hs.OmittedByUser));
  PyDict_SetItemString(out, "OmittedReflections", Py_BuildValue("i", hs.OmittedReflections));
  PyDict_SetItemString(out, "IntensityTransformed", Py_BuildValue("i", hs.IntensityTransformed));
  PyDict_SetItemString(out, "Rint", Py_BuildValue("d", hs.Rint));
  PyDict_SetItemString(out, "Rsigma", Py_BuildValue("d", hs.Rsigma));
  PyDict_SetItemString(out, "MeanIOverSigma", Py_BuildValue("d", hs.MeanIOverSigma));
  PyDict_SetItemString(out, "MaxIndexes", Py_BuildValue("(iii)", hs.MaxIndexes[0], hs.MaxIndexes[1], hs.MaxIndexes[2]) );
  PyDict_SetItemString(out, "MinIndexes", Py_BuildValue("(iii)", hs.MinIndexes[0], hs.MinIndexes[1], hs.MinIndexes[2]) );
  PyDict_SetItemString(out, "ReflectionAPotMax", Py_BuildValue("i", hs.ReflectionAPotMax));
  PyDict_SetItemString(out, "FriedelPairCount", Py_BuildValue("i", xapp.XFile().GetRM().GetFriedelPairCount()));

  const TIntList& redInfo = xapp.XFile().GetRM().GetRedundancyInfo();
  PyObject* red = PyTuple_New(redInfo.Count());
  for( int i=0; i < redInfo.Count(); i++ )
    PyTuple_SetItem(red, i, Py_BuildValue("i", redInfo[i]));
  PyDict_SetItemString(out, "Redundancy", red);
  return out;
}
//..............................................................................
PyObject* pyUpdateRepository(PyObject* self, PyObject* args)  {
  olxstr index, index_fn, repos, dest, proxy;
  if( !PythonExt::ParseTuple(args, "ww", &index, &dest) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  olxstr SettingsFile( TBasicApp::GetBaseDir() + "usettings.dat" );
  if( TEFile::Exists(SettingsFile) )  {
    const TSettingsFile settings(SettingsFile);
    proxy = settings["proxy"];
  }
  int lsi = index.LastIndexOf('/');
  if( lsi == -1 )  {
    PyErr_SetObject(PyExc_AttributeError, PythonExt::BuildString("Invalid index file") );
    return Py_BuildValue("b", false);
  }
  dest = TBasicApp::GetBaseDir() + dest;
  if( !TEFile::MakeDirs(dest) )  {
    PyErr_SetObject(PyExc_AttributeError, PythonExt::BuildString("Could not create distination folder") );
    return Py_BuildValue("b", false);
  }
  index_fn = index.SubStringFrom(lsi+1);
  repos = index.SubStringTo(lsi+1);
  TUrl url(repos);
  if( !proxy.IsEmpty() )  
    url.SetProxy( TUrl(proxy) );
  THttpFileSystem httpFS( url );
  TOSFileSystem osFS(dest);
  TFSIndex fsIndex( httpFS );
  TStrList properties;
  try  {  fsIndex.Synchronise(osFS, properties, NULL, NULL, NULL, index_fn);  }
  catch( const TExceptionBase& exc )  {
    PyErr_SetObject(PyExc_TypeError, PythonExt::BuildString(exc.GetException()->GetFullMessage()) );
    return Py_BuildValue("b", false);
  }
  return Py_BuildValue("b", true);
}
//..............................................................................
static PyMethodDef CORE_Methods[] = {
  {"UpdateRepository", pyUpdateRepository, METH_VARARGS, "Updates specified local repository from the http one. Takes the following arguments: \
the index file name, destination folder (relative to the basedir)"},
  {"GetPluginList", pyGetPlugins, METH_VARARGS, "returns a list of installed plugins"},
  {"ExportFunctionList", pyExpFun, METH_VARARGS, "exports a list of olex functions and their description"},
  {"ExportMacroList", pyExpMac, METH_VARARGS, "exports a list of olex macros and their description"},
  {"IsVar", pyIsVar, METH_VARARGS, "returns boolean value if specified variable exists"},
  {"VarCount", pyVarCount, METH_VARARGS, "returns the number of variables"},
  {"VarValue", pyGetVar, METH_VARARGS, "returns specified variable value"},
  {"SetVar", pySetVar, METH_VARARGS, "sets value of specified variable"},
  {"UnsetVar", pyUnsetVar, METH_VARARGS, "unsets specified variable. Returns True if the variable existed,\
 False if it did not xust and None if an error occured"},
  {"FindValue", pyVarValue, METH_VARARGS, "returns value of specified variable or empty string"},
  {"FindObject", pyVarObject, METH_VARARGS, "returns value of specified variable as an object"},
  {"VarName", pyGetVarName, METH_VARARGS, "returns name of specified variable"},
  {"FindVarName", pyFindGetVarName, METH_VARARGS, "returns name of variable name corresponding to provided object"},
  {"Translate", pyTranslate, METH_VARARGS, "returns translated version of provided string"},
  {"DescribeRefinement", pyDescRef, METH_VARARGS, "Returns a string describing current refinement model"},
  {"GetRefinementModel", pyRefModel, METH_VARARGS, "Returns refinement model as python object"},
  {"GetHklStat", pyHklStat, METH_VARARGS, "Returns HKL statistics"},
  {NULL, NULL, 0, NULL}
   };

void OlexPyCore::PyInit()  {
  Py_InitModule( "olex_core", CORE_Methods );
}
