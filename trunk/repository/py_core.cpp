#undef GetObject

#include "py_core.h"

#include "xapp.h"
#include "efile.h"
#include "settingsfile.h"
#include "url.h"
#include "httpfs.h"
#include "integration.h"
#include "olxvar.h"
#include "symmlib.h"

using namespace olex;

PyObject* pyVarValue(PyObject* self, PyObject* args)  {
  olxstr varName;
  PyObject* defVal = NULL;
  if( !PythonExt::ParseTuple(args, "w|O", &varName, &defVal) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  size_t i = TOlxVars::VarIndex(varName);
  if( i == InvalidIndex )  {
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
  size_t i = TOlxVars::VarIndex(varName);
  if( i == InvalidIndex )  {
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
  for( size_t i=0; i < rv.Count(); i++ )
    PyTuple_SetItem(af, i, PythonExt::BuildString(rv[i]) );
  return af;
}
//..............................................................................
PyObject* pyExpFun(PyObject* self, PyObject* args)  {
  TBasicFunctionPList functions;
  IOlexProcessor::GetInstance()->GetLibrary().ListAllFunctions( functions );
  PyObject* af = PyTuple_New( functions.Count() ), *f;
  for( size_t i=0; i < functions.Count(); i++ )  {
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
  for( size_t i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    f = PyTuple_New(4);
    PyTuple_SetItem(af, i, f );
    PyTuple_SetItem(f, 0, PythonExt::BuildString(func->GetQualifiedName()) );
    PyTuple_SetItem(f, 1, PythonExt::BuildString(func->GetSignature()) );
    PyTuple_SetItem(f, 2, PythonExt::BuildString(func->GetDescription()) );
    s = PyDict_New();
    PyTuple_SetItem(f, 3, s );
    for( size_t j=0; j < func->GetOptions().Count(); j++ )  {
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
PyObject* pySGInfo(PyObject* self, PyObject* args)  {
  TSpaceGroup* sg = NULL;
  if( PyTuple_Size(args) == 0 )  {
    try  {  sg = &TXApp::GetInstance().XFile().GetLastLoaderSG();  }
    catch(...)  {
      Py_INCREF(Py_None);
      return Py_None;
    }
  }
  else  {
    olxstr sg_name;
    if( !PythonExt::ParseTuple(args, "w", &sg_name) )  {
      Py_INCREF(Py_None);
      return Py_None;
    }
    sg = TSymmLib::GetInstance().FindGroup(sg_name);
    if( sg == NULL )  {
      Py_INCREF(Py_None);
      return Py_None;
    }
  }
  PyObject* out = PyDict_New();
  PyDict_SetItemString(out, "Number", Py_BuildValue("i", sg->GetNumber()));
  PyDict_SetItemString(out, "Centrosymmetric", Py_BuildValue("b", sg->IsCentrosymmetric()));
  PyDict_SetItemString(out, "ShortName", PythonExt::BuildString(sg->GetName()));
  PyDict_SetItemString(out, "FullName", PythonExt::BuildString(sg->GetFullName()));
  PyDict_SetItemString(out, "System", PythonExt::BuildString(sg->GetBravaisLattice().GetName()));
  PyDict_SetItemString(out, "Center", 
    Py_BuildValue("(d,d,d)", sg->GetInversionCenter()[0], sg->GetInversionCenter()[1], sg->GetInversionCenter()[2]));
  PyDict_SetItemString(out, "PointGroup", PythonExt::BuildString(sg->GetPointGroup().GetBareName()));
  PyDict_SetItemString(out, "LaueClass", PythonExt::BuildString(sg->GetLaueClass().GetBareName()));
  PyDict_SetItemString(out, "HallSymbol", PythonExt::BuildString(sg->GetHallSymbol()));
    PyObject* latt_out;
    PyDict_SetItemString(out, "Lattice", (latt_out=PyDict_New()));
    TCLattice& latt = sg->GetLattice();
    PyDict_SetItemString(latt_out, "Name", PythonExt::BuildString(latt.GetName()));
    PyDict_SetItemString(latt_out, "Centering", PythonExt::BuildString(latt.GetSymbol()));
    PyDict_SetItemString(latt_out, "InsCode", Py_BuildValue("i", latt.GetLatt()));
    PyObject* latt_vec_out = PyTuple_New(latt.VectorCount());
    for( size_t i=0; i < latt.VectorCount(); i++ ) 
      PyTuple_SetItem(latt_vec_out, i, Py_BuildValue("(ddd)", latt.GetVector(i)[0], latt.GetVector(i)[1], latt.GetVector(i)[2]));
    PyDict_SetItemString(latt_out, "Translations", latt_vec_out);
    PyObject* matr_out = PyTuple_New(sg->MatrixCount());
    for( size_t i=0; i < sg->MatrixCount(); i++ )  {
      const smatd& m = sg->GetMatrix(i);
      PyTuple_SetItem(matr_out, i, 
        Py_BuildValue("(iiid)(iiid)(iiid)", 
        m.r[0][0], m.r[0][1], m.r[0][2], m.t[0],
        m.r[1][0], m.r[1][1], m.r[1][2], m.t[1],
        m.r[2][0], m.r[2][1], m.r[2][2], m.t[2]
      ));
    }
    PyDict_SetItemString(out, "Matrices", matr_out);
    smatd_list ml;
    sg->GetMatrices(ml, mattAll);
    matr_out=PyTuple_New(ml.Count());
    for( size_t i=0; i < ml.Count(); i++ )  {
      const smatd& m = ml[i];
      PyTuple_SetItem(matr_out, i, 
        Py_BuildValue("(iiid)(iiid)(iiid)", 
          m.r[0][0], m.r[0][1], m.r[0][2], m.t[0],
          m.r[1][0], m.r[1][1], m.r[1][2], m.t[1],
          m.r[2][0], m.r[2][1], m.r[2][2], m.t[2]
      ));
    }
    PyDict_SetItemString(out, "MatricesAll", matr_out);

    TPtrList<TSymmElement> ref, sg_elm;
    for( size_t i=0; i < TSymmLib::GetInstance().SymmElementCount(); i++ )
      ref.Add(TSymmLib::GetInstance().GetSymmElement(i));
    sg->SplitIntoElements(ref, sg_elm);
    PyObject* sysabs_out = PyTuple_New(sg_elm.Count());
    for( size_t i=0; i < sg_elm.Count(); i++ )  {
      matr_out = PyTuple_New(sg_elm[i]->MatrixCount());
      for( size_t j=0; j < sg_elm[i]->MatrixCount(); j++ )  {
        const smatd& m = sg_elm[i]->GetMatrix(j);
        PyTuple_SetItem(matr_out, j, 
          Py_BuildValue("((iiid)(iiid)(iiid)))",
            m.r[0][0], m.r[0][1], m.r[0][2], m.t[0],
            m.r[1][0], m.r[1][1], m.r[1][2], m.t[1],
            m.r[2][0], m.r[2][1], m.r[2][2], m.t[2]));
      }
      PyTuple_SetItem(sysabs_out, i, Py_BuildValue("(OO)", PythonExt::BuildString(sg_elm[i]->GetName()), matr_out) );
    }
    PyDict_SetItemString(out, "SysAbs", sysabs_out);
  return out;
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
  for( size_t i=0; i < redInfo.Count(); i++ )
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
  size_t lsi = index.LastIndexOf('/');
  if( lsi == InvalidIndex )  {
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
  {"SGInfo", pySGInfo, METH_VARARGS, "Returns current/give space group information"},
  {NULL, NULL, 0, NULL}
   };

void OlexPyCore::PyInit()  {
  Py_InitModule( "olex_core", CORE_Methods );
}
