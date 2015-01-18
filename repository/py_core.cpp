/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "py_core.h"
#ifndef _CONSOLE
#include "olex2app.h"
#else
#include "xapp.h"
#endif
#include "efile.h"
#include "settingsfile.h"
#include "url.h"
#include "httpfs.h"
#include "integration.h"
#include "olxvar.h"
#include "symmlib.h"
#include "cdsfs.h"
#include "label_corrector.h"
#include "symmparser.h"
#include "updateapi.h"
#undef GetObject

using namespace olex2;
#ifdef _PYTHON

PyObject* pyVarValue(PyObject* self, PyObject* args)  {
  olxstr varName;
  PyObject* defVal = NULL;
  if( !PythonExt::ParseTuple(args, "w|O", &varName, &defVal) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w|O");
  size_t i = TOlxVars::VarIndex(varName);
  if( i == InvalidIndex )  {
    if( defVal != NULL )  {
      TOlxVars::SetVar(varName, defVal);
      Py_IncRef( defVal );
      return TOlxPyVar::ObjectValue(defVal);
    }
    else {
      return PythonExt::SetErrorMsg(PyExc_KeyError, __OlxSourceInfo,
        "Undefined key name");
    }
  }
  return TOlxVars::GetVarValue(i);
}
//..............................................................................
PyObject* pyVarObject(PyObject* self, PyObject* args)  {
  olxstr varName;
  PyObject* defVal = NULL;
  if( !PythonExt::ParseTuple(args, "w|O", &varName, &defVal) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w|O");
  size_t i = TOlxVars::VarIndex(varName);
  if( i == InvalidIndex )  {
    if( defVal != NULL )  {
      TOlxVars::SetVar(varName, defVal);
      Py_IncRef( defVal );
      return defVal;
    }
    else {
      return PythonExt::SetErrorMsg(PyExc_KeyError, __OlxSourceInfo,
        "Undefined key name");
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
  if( !PythonExt::ParseTuple(args, "w", &varName) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  return Py_BuildValue("b", TOlxVars::IsVar(varName));
}
//..............................................................................
PyObject* pyVarCount(PyObject* self, PyObject* args)  {
  return Py_BuildValue("i", TOlxVars::VarCount());
}
//..............................................................................
PyObject* pyGetVar(PyObject* self, PyObject* args)  {
  int varIndex;
  if( !PyArg_ParseTuple(args, "i", &varIndex) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "i");
  return TOlxVars::GetVarValue(varIndex);
}
//..............................................................................
PyObject* pyGetVarName(PyObject* self, PyObject* args)  {
  int varIndex;
  if( !PyArg_ParseTuple(args, "i", &varIndex) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "i");
  return PythonExt::BuildString(TOlxVars::GetVarName(varIndex));
}
//..............................................................................
PyObject* pyFindGetVarName(PyObject* self, PyObject* args)  {
  PyObject *val;
  if( !PyArg_ParseTuple(args, "O", &val) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "O");
  return PythonExt::BuildString(TOlxVars::FindVarName(val));
}
//..............................................................................
PyObject* pySetVar(PyObject* self, PyObject* args)  {
  olxstr varName;
  PyObject *varValue = NULL;
  if( !PythonExt::ParseTuple(args, "wO", &varName, &varValue) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "wO");
  TOlxVars::SetVar(varName, varValue);
  return PythonExt::PyNone();
}
//..............................................................................
PyObject* pyUnsetVar(PyObject* self, PyObject* args)  {
  olxstr varName;
  if( !PythonExt::ParseTuple(args, "w", &varName) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  return TOlxVars::UnsetVar(varName) ? PythonExt::PyTrue()
    : PythonExt::PyFalse();
}
//..............................................................................
PyObject* pyExpFun(PyObject* self, PyObject* args)  {
  TBasicFunctionPList functions;
  IOlex2Processor::GetInstance()->GetLibrary().ListAllFunctions(functions);
  PyObject* af = PyTuple_New(functions.Count()), *f;
  for( size_t i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    f = PyTuple_New(3);
    PyTuple_SetItem(af, i, f );

    PyTuple_SetItem(f, 0, PythonExt::BuildString(func->GetQualifiedName()));
    PyTuple_SetItem(f, 1, PythonExt::BuildString(func->GetSignature()));
    PyTuple_SetItem(f, 2, PythonExt::BuildString(func->GetDescription()));
  }
  return af;
}
//..............................................................................
PyObject* pyExpMac(PyObject* self, PyObject* args)  {
  TBasicFunctionPList functions;
  IOlex2Processor::GetInstance()->GetLibrary().ListAllMacros(functions);
  PyObject* af = PyTuple_New( functions.Count() ), *f, *s;
  for( size_t i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    f = PyTuple_New(4);
    PyTuple_SetItem(af, i, f );
    PyTuple_SetItem(f, 0, PythonExt::BuildString(func->GetQualifiedName()));
    PyTuple_SetItem(f, 1, PythonExt::BuildString(func->GetSignature()));
    PyTuple_SetItem(f, 2, PythonExt::BuildString(func->GetDescription()));
    s = PyDict_New();
    PyTuple_SetItem(f, 3, s );
    for( size_t j=0; j < func->GetOptions().Count(); j++ )  {
      PythonExt::SetDictItem(s, func->GetOptions().GetKey(j).c_str(),
        PythonExt::BuildString(func->GetOptions().GetValue(j)));
    }
  }
  return af;
}
//..............................................................................
PyObject* pyGetPlugins(PyObject* self, PyObject* args)  {
  TStrList rv;
  olxstr fn = TBasicApp::GetBaseDir()+"plugins.xld";
#ifdef _CONSOLE
  if (TEFile::Exists(fn)) {
    TDataFile df;
    df.LoadFromXLFile(fn);
    TDataItem *pi = df.Root().FindItem("Plugin");
    if (pi != NULL) {
      for (size_t i=0; i < pi->ItemCount(); i++)
        rv.Add(pi->GetItemByIndex(i).GetName());
    }
  }
#else
  if (!AOlex2App::HasInstance())
    return PythonExt::PyNone();
  rv = AOlex2App::GetInstance().GetPluginList();
#endif
  PyObject* af = PyTuple_New(rv.Count());
  for( size_t i=0; i < rv.Count(); i++ )
    PyTuple_SetItem(af, i, PythonExt::BuildString(rv[i]));
  return af;
}
//..............................................................................
PyObject* pyTranslate(PyObject* self, PyObject* args)  {
  olxstr str;
  if( !PythonExt::ParseTuple(args, "w", &str) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
#ifdef _CONSOLE
  return PythonExt::BuildString(str);
#else
  if (!AOlex2App::HasInstance())
    return PythonExt::BuildString(str);
  return PythonExt::BuildString(
    AOlex2App::GetInstance().TranslateString(str));
#endif
}
//..............................................................................
PyObject* pyDescRef(PyObject* self, PyObject* args)  {
  TStrList rv = TXApp::GetInstance().XFile().GetRM().Describe();
  return PythonExt::BuildString(rv.Text('\n'));
}
//..............................................................................
PyObject* pyRefModel(PyObject* self, PyObject* args)  {
  bool calc_connectivity = false;
  if( PyTuple_Size(args) != 0 )  {
    if( !PyArg_ParseTuple(args, "b", &calc_connectivity) )
      return PythonExt::InvalidArgumentException(__OlxSourceInfo, "b");
  }
  TAsymmUnit& au = TXApp::GetInstance().XFile().GetAsymmUnit();
  /* make the labels unique globally. This needs to be done since the number of
  files containing atom labels (like VcV) and information may be miss
  -represented
  */
  TStrList o_labels(au.AtomCount());
  LabelCorrector lc;
  for (size_t i=0; i < au.AtomCount(); i++) {
    lc.Correct(au.GetAtom(i));
    o_labels[i] = au.GetAtom(i).GetLabel();
    au.GetAtom(i).SetLabel(au.GetAtom(i).GetResiLabel(), false);
  }
  PyObject *rv = TXApp::GetInstance().XFile().GetRM().PyExport(calc_connectivity);
  // restore labels
  for (size_t i=0; i < au.AtomCount(); i++) {
    au.GetAtom(i).SetLabel(o_labels[i], false);
  }
  return rv;
}
//..............................................................................
PyObject* pySGInfo(PyObject* self, PyObject* args)  {
  TSpaceGroup* sg = NULL;
  bool include_lattice = true;
  if( PyTuple_Size(args) == 0 )  {
    try  {  sg = &TXApp::GetInstance().XFile().GetLastLoaderSG();  }
    catch(...)  {
      return PythonExt::PyNone();
    }
  }
  else  {
    olxstr sg_name;
    if( !PythonExt::ParseTuple(args, "w|b", &sg_name, &include_lattice) )
      return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w|b");
    sg = TSymmLib::GetInstance().FindGroupByName(sg_name);
    if( sg == NULL )
      return PythonExt::PyNone();
  }
  PyObject* out = PyDict_New();
  PythonExt::SetDictItem(out, "Number",
    Py_BuildValue("i", sg->GetNumber()));
  PythonExt::SetDictItem(out, "Centrosymmetric",
    Py_BuildValue("b", sg->IsCentrosymmetric()));
  PythonExt::SetDictItem(out, "ShortName",
    PythonExt::BuildString(sg->GetName()));
  PythonExt::SetDictItem(out, "FullName",
    PythonExt::BuildString(sg->GetFullName()));
  PythonExt::SetDictItem(out, "System",
    PythonExt::BuildString(sg->GetBravaisLattice().GetName()));
  PythonExt::SetDictItem(out, "Center",
    Py_BuildValue("(d,d,d)",
      sg->GetInversionCenter()[0],
      sg->GetInversionCenter()[1],
      sg->GetInversionCenter()[2]));
  PythonExt::SetDictItem(out, "PointGroup",
    PythonExt::BuildString(sg->GetPointGroup().GetBareName()));
  PythonExt::SetDictItem(out, "LaueClass",
    PythonExt::BuildString(sg->GetLaueClass().GetBareName()));
  PythonExt::SetDictItem(out, "HallSymbol"
    , PythonExt::BuildString(sg->GetHallSymbol()));
    PyObject* latt_out;
    PythonExt::SetDictItem(out, "Lattice", (latt_out=PyDict_New()));
    const TCLattice& latt = sg->GetLattice();
    PythonExt::SetDictItem(latt_out, "Name",
      PythonExt::BuildString(latt.GetName()));
    PythonExt::SetDictItem(latt_out, "Centering",
      PythonExt::BuildString(latt.GetSymbol()));
    PythonExt::SetDictItem(latt_out, "InsCode",
      Py_BuildValue("i", latt.GetLatt()));
    PyObject* latt_vec_out = PyTuple_New(latt.GetVectors().Count());
    for( size_t i=0; i < latt.GetVectors().Count(); i++ )  {
      PyTuple_SetItem(latt_vec_out, i,
        Py_BuildValue("(ddd)",
          latt.GetVectors()[i][0],
          latt.GetVectors()[i][1],
          latt.GetVectors()[i][2]));
    }
    PythonExt::SetDictItem(latt_out, "Translations", latt_vec_out);
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
    PythonExt::SetDictItem(out, "Matrices", matr_out);
    smatd_list ml;
    sg->GetMatrices(ml, include_lattice ? mattAll : mattAll^mattCentering);
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
    PythonExt::SetDictItem(out, "MatricesAll", matr_out);

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
          Py_BuildValue("(iiid)(iiid)(iiid)",
            m.r[0][0], m.r[0][1], m.r[0][2], m.t[0],
            m.r[1][0], m.r[1][1], m.r[1][2], m.t[1],
            m.r[2][0], m.r[2][1], m.r[2][2], m.t[2]));
      }
      PyTuple_SetItem(sysabs_out, i, Py_BuildValue("(OO)",
        PythonExt::BuildString(sg_elm[i]->GetName()), matr_out));
    }
    PythonExt::SetDictItem(out, "SysAbs", sysabs_out);
    PythonExt::SetDictItem(out, "Axis", PythonExt::BuildString(sg->GetAxis()));
  return out;
}
//..............................................................................
PyObject* pyMatrixToString(PyObject* self, PyObject* args)  {
  PyObject *m[3], *tp;
  bool normalise_t=true;
  if (!PyArg_ParseTuple(args, "O|b", &tp, &normalise_t))
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "O");
  if (!PyArg_ParseTuple(tp, "OOO", &m[0], &m[1], &m[2]))
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "OOO");
  smatd matr;
  for (int i=0; i < 3; i++) {
    int x, y, z;
    double t;
    if( !PyArg_ParseTuple(m[i], "iiid", &x, &y, &z, &t) )
      return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iiif");
    matr.r[i][0] = x; matr.r[i][1] = y; matr.r[i][2] = z; matr.t[i] = t;
  }
  return PythonExt::BuildString(normalise_t ? TSymmParser::MatrixToSymmEx(matr)
    : TSymmParser::MatrixToSymm(matr));
}
//..............................................................................
PyObject* pyHklStat(PyObject* self, PyObject* args)  {
  try  {
    TXApp& xapp = TXApp::GetInstance();
    RefinementModel::HklStat hs = xapp.XFile().GetRM().GetMergeStat();
    PyObject* out = PyDict_New();
    PythonExt::SetDictItem(out, "TotalReflections",
      Py_BuildValue("i", hs.TotalReflections));
    PythonExt::SetDictItem(out, "UniqueReflections",
      Py_BuildValue("i", hs.UniqueReflections));
    PythonExt::SetDictItem(out, "DataCount",
      Py_BuildValue("i", hs.DataCount));
    PythonExt::SetDictItem(out, "FriedelOppositesMerged",
      Py_BuildValue("b", hs.FriedelOppositesMerged));
    PythonExt::SetDictItem(out, "InconsistentEquivalents",
      Py_BuildValue("i", hs.InconsistentEquivalents));
    PythonExt::SetDictItem(out, "SystematicAbsencesRemoved",
      Py_BuildValue("i", hs.SystematicAbsencesRemoved));
    PythonExt::SetDictItem(out, "MinD", Py_BuildValue("d", hs.MinD));
    PythonExt::SetDictItem(out, "MaxD", Py_BuildValue("d", hs.MaxD));
    PythonExt::SetDictItem(out, "LimDmin", Py_BuildValue("d", hs.LimDmin));
    PythonExt::SetDictItem(out, "LimDmax", Py_BuildValue("d", hs.LimDmax));
    PythonExt::SetDictItem(out, "FilteredOff",
      Py_BuildValue("i", hs.FilteredOff));
    PythonExt::SetDictItem(out, "OmittedByUser",
      Py_BuildValue("i", hs.OmittedByUser));
    PythonExt::SetDictItem(out, "OmittedReflections",
      Py_BuildValue("i", hs.OmittedReflections));
    PythonExt::SetDictItem(out, "IntensityTransformed",
      Py_BuildValue("i", hs.IntensityTransformed));
    PythonExt::SetDictItem(out, "Rint", Py_BuildValue("d", hs.Rint));
    PythonExt::SetDictItem(out, "Rsigma", Py_BuildValue("d", hs.Rsigma));
    PythonExt::SetDictItem(out, "MeanIOverSigma",
      Py_BuildValue("d", hs.MeanIOverSigma));
    PythonExt::SetDictItem(out, "Completeness",
      Py_BuildValue("d", hs.Completeness));
    PythonExt::SetDictItem(out, "MaxIndexes",
      Py_BuildValue("(iii)", hs.MaxIndexes[0],
        hs.MaxIndexes[1], hs.MaxIndexes[2]));
    PythonExt::SetDictItem(out, "MinIndexes",
      Py_BuildValue("(iii)", hs.MinIndexes[0],
        hs.MinIndexes[1], hs.MinIndexes[2]));
    PythonExt::SetDictItem(out, "FileMaxIndexes", Py_BuildValue("(iii)",
      hs.FileMaxInd[0], hs.FileMaxInd[1], hs.FileMaxInd[2]));
    PythonExt::SetDictItem(out, "FileMinIndexes", Py_BuildValue("(iii)",
      hs.FileMinInd[0], hs.FileMinInd[1], hs.FileMinInd[2]));
    PythonExt::SetDictItem(out, "ReflectionAPotMax",
      Py_BuildValue("i", hs.ReflectionAPotMax));
    PythonExt::SetDictItem(out, "FriedelPairCount",
      Py_BuildValue("i", xapp.XFile().GetRM().GetFriedelPairCount()));

    const TIntList& redInfo = xapp.XFile().GetRM().GetRedundancyInfo();
    PyObject* red = PyTuple_New(redInfo.Count());
    for( size_t i=0; i < redInfo.Count(); i++ )
      PyTuple_SetItem(red, i, Py_BuildValue("i", redInfo[i]));
    PythonExt::SetDictItem(out, "Redundancy", red);
    return out;
  }
  catch(const TExceptionBase& e)  {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
    return PythonExt::SetErrorMsg(PyExc_Exception, __OlxSourceInfo,
      e.GetException()->GetFullMessage());
  }
}
//..............................................................................
PyObject* pyUpdateRepository(PyObject* self, PyObject* args)  {
  olxstr index, index_fn, repos, dest, proxy;
  if( !PythonExt::ParseTuple(args, "ww", &index, &dest) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "ww");
  olxstr SettingsFile = updater::UpdateAPI::GetSettingsFileName();
  if( TEFile::Exists(SettingsFile) )  {
    const TSettingsFile settings(SettingsFile);
    proxy = settings["proxy"];
  }
  size_t lsi = index.LastIndexOf('/');
  if( lsi == InvalidIndex ) {
    return PythonExt::SetErrorMsg(PyExc_AttributeError, __OlxSourceInfo,
      "Invalid index file");
  }
  dest = TBasicApp::GetBaseDir() + dest;
  if( !TEFile::MakeDirs(dest) ) {
    return PythonExt::SetErrorMsg(PyExc_AttributeError, __OlxSourceInfo,
      "Could not create destination folder");
  }
  index_fn = index.SubStringFrom(lsi+1);
  repos = index.SubStringTo(lsi+1);
  TUrl url(repos);
  if( !proxy.IsEmpty() )
    url.SetProxy(TUrl(proxy));
  TSocketFS httpFS(url);
  TOSFileSystem osFS(dest);
  TFSIndex fsIndex(httpFS);
  TStrList properties;
  try  {  fsIndex.Synchronise(osFS, properties, NULL, NULL, index_fn);  }
  catch( const TExceptionBase& exc )  {
    return PythonExt::SetErrorMsg(PyExc_TypeError, __OlxSourceInfo,
      exc.GetException()->GetFullMessage());
  }
  return Py_BuildValue("b", true);
}
//..............................................................................
PyObject* pyGetVdWRadii(PyObject* self, PyObject* args)  {
  ElementRadii radii;
  olxstr radii_fn;
  if( !PythonExt::ParseTuple(args, "|w", &radii_fn) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "|w");
  radii = TXApp::ReadRadii(radii_fn);
  ContentList content =
    TXApp::GetInstance().XFile().GetAsymmUnit().GetContentList();
  PyObject* dict = PyDict_New();
  for( size_t i=0; i < content.Count(); i++ )  {
    const size_t ei = radii.IndexOf(&content[i].element);
    const double r = (ei == InvalidIndex ? content[i].element.r_vdw
      : radii.GetValue(ei));
    PythonExt::SetDictItem(dict,
      PythonExt::BuildString(content[i].element.symbol),
      Py_BuildValue("f", r));
  }
  return dict;
}
//..............................................................................
PyObject* pySetBadReflections(PyObject* self, PyObject* args)  {
  PyObject *r, *l;
  if( !PythonExt::ParseTuple(args, "O", &l) || !PyIter_Check(l) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "O");
  TTypeList<RefinementModel::BadReflection> bad_refs;
  while (r = PyIter_Next(l)) {
    RefinementModel::BadReflection br;
    if (!PythonExt::ParseTuple(r, "iiiddd",
          &br.index[0], &br.index[1], &br.index[2],
          &br.Fo, &br.Fc, &br.esd))
    {
      Py_DECREF(r);
      return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iiiddd");
    }
    Py_DECREF(r);
    bad_refs.AddCopy(br).UpdateFactor();
  }
  TXApp::GetInstance().XFile().GetRM().SetBadReflectionList(bad_refs);

  return PythonExt::PyNone();
}
//..............................................................................
#if defined(__WIN32__)
struct olxProcessWindow {
  HWND hwnd;
  DWORD pid;
  olxProcessWindow(DWORD pid) : hwnd(0), pid(pid) {}
};
BOOL CALLBACK olx_EnumWindowsProc(HWND hwnd, LPARAM lParam) {
  olxch bf[32];
  GetWindowText(hwnd, &bf[0], 32);
  olxstr wn(&bf[0]);
  if (!(wn.Equalsi("P.L.A.T.O.N") || wn.Equalsi("P.L.U.T.O.N"))) {
    return TRUE;
  }
  DWORD pid;
  GetWindowThreadProcessId(hwnd, &pid);
  olxProcessWindow *pw = (olxProcessWindow*)lParam;
  if (pid == pw->pid) {
    pw->hwnd = hwnd;
    return FALSE;
  }
  return TRUE;
}
#endif
PyObject* pyOnPlatonRun(PyObject* self, PyObject* args) {
#if defined(__WIN32__)
  DWORD pid;
  if (!PythonExt::ParseTuple(args, "i", &pid)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "i");
  }
  HANDLE ph = OpenProcess(PROCESS_QUERY_INFORMATION, false, pid);
  if (ph == NULL) {
    return PythonExt::PyFalse();
  }
  olxProcessWindow pwt(pid);
  size_t cnt = 0;
  while (true) {
    EnumWindows(&olx_EnumWindowsProc, (LPARAM)&pwt);
    if (pwt.hwnd != NULL) {
      break;
    }
    DWORD ec;
    if (GetExitCodeProcess(ph, &ec) == TRUE) {
      if (ec != STILL_ACTIVE) {
        return PythonExt::PyFalse();
      }
    }
    else {
      return PythonExt::PyFalse();
    }
    olx_sleep(50);
    if (++cnt >= 100) {
      return PythonExt::PyFalse();
    }
  }
  HMENU  hMenu = GetSystemMenu(pwt.hwnd, FALSE);
  if (hMenu != NULL) {
    EnableMenuItem(hMenu, SC_CLOSE,
      MF_BYCOMMAND | (MF_DISABLED | MF_GRAYED));
  }
  return PythonExt::PyTrue();
#endif
  return PythonExt::PyFalse();
}
//..............................................................................
static PyMethodDef CORE_Methods[] = {
  {"UpdateRepository", pyUpdateRepository, METH_VARARGS,
  "Updates specified local repository from the http one. Takes the following "
  "arguments: the index file name, destination folder (relative to the basedir)"
  },
  {"GetPluginList", pyGetPlugins, METH_VARARGS,
  "Returns a list of installed plugins"},
  {"Translate", pyTranslate, METH_VARARGS,
  "Returns translated version of provided string"},
  {"ExportFunctionList", pyExpFun, METH_VARARGS,
  "Exports a list of Olex2 functions and their description"},
  {"ExportMacroList", pyExpMac, METH_VARARGS,
  "Exports a list of Olex2 macros and their description"},
  {"IsVar", pyIsVar, METH_VARARGS,
  "Returns boolean value if specified variable exists"},
  {"VarCount", pyVarCount, METH_VARARGS, "Returns the number of variables"},
  {"VarValue", pyGetVar, METH_VARARGS, "Returns specified variable value"},
  {"SetVar", pySetVar, METH_VARARGS, "Sets value of specified variable"},
  {"UnsetVar", pyUnsetVar, METH_VARARGS,
  "Unsets specified variable. Returns True if the variable existed, "
  "False if it did not exist and None if an error occured"},
  {"FindValue", pyVarValue, METH_VARARGS,
  "Returns value of specified variable or empty string"},
  {"FindObject", pyVarObject, METH_VARARGS,
  "Returns value of specified variable as an object"},
  {"VarName", pyGetVarName, METH_VARARGS,
  "Returns name of specified variable"},
  {"FindVarName", pyFindGetVarName, METH_VARARGS,
  "Returns name of variable name corresponding to provided object"},
  {"DescribeRefinement", pyDescRef, METH_VARARGS,
  "Returns a string describing current refinement model"},
  {"GetRefinementModel", pyRefModel, METH_VARARGS,
  "Returns refinement model as python object"},
  {"GetHklStat", pyHklStat, METH_VARARGS,
  "Returns HKL statistics"},
  {"SGInfo", pySGInfo, METH_VARARGS,
  "Returns current/given space group information"},
  {"GetVdWRadii", pyGetVdWRadii, METH_VARARGS,
  "Returns Van der Waals radii for elements of current model"},
  {"SetBadReflections", pySetBadReflections, METH_VARARGS,
    "Sets a list of bad reflections as iterable of (h k l Fo Fc esd)"},
  {"MatrixToString", pyMatrixToString, METH_VARARGS,
    "Converts a symmetry operation into string representation"},
  { "OnPlatonRun", pyOnPlatonRun, METH_VARARGS,
    "Deals with orphaned Platon processes" },
  { NULL, NULL, 0, NULL }
   };

void OlexPyCore::PyInit()  {
  Py_InitModule("olex_core", CORE_Methods);
}
#endif
