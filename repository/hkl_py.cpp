/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifdef _PYTHON
#include "hkl_py.h"
#include "hkl_util.h"

PyMethodDef hkl_py::Methods[] = {
  {"Read", hkl_py::Read, METH_VARARGS, "reads an HKL file and returns a tuple of the reflections"},
  {"Write", hkl_py::Write, METH_VARARGS, "writes provided tuple to an hkl file"},
  {"Fingerprint", hkl_py::Fingerprint, METH_VARARGS,
    "computes fingerprint for HKL file or a list of intensities"},
  {"FormatFingerprint", hkl_py::FormatFingerprint, METH_VARARGS,
    "formats fingerprint as string"},
  {"Corelate", hkl_py::Corelate, METH_VARARGS,
    "corelates fingerprints"},
  {NULL, NULL, 0, NULL}
};

olxcstr &hkl_py::ModuleName() {
  static olxcstr mn = "olex_hkl";
  return mn;
}

PyObject *hkl_py::PyInit() {
  return PythonExt::init_module(ModuleName(), Methods);
}
//..................................................................................................
PyObject* hkl_py::Read(PyObject* self, PyObject* args)  {
  olxstr fn;
  if (!PythonExt::ParseTuple(args, "w", &fn)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  }
  if( !TEFile::Exists(fn) ) {
    return PythonExt::SetErrorMsg(PyExc_IOError, __OlxSourceInfo,
      olxstr("File does not exist: ") << fn);
  }
  THklFile hkl;
  olxstr error;
  try  {  hkl.LoadFromFile(fn, false);  }
  catch (const TExceptionBase& e)  {
    return PythonExt::SetErrorMsg(PyExc_IOError, __OlxSourceInfo,
      olxstr("Invalid HKL file: ") << fn << '\n' <<
      e.GetException()->GetError());
  }
  PyObject* rv = PyTuple_New(hkl.RefCount());
  for (size_t i=0; i < hkl.RefCount(); i++) {
    PyObject* ref = PyTuple_New(7);
    PyTuple_SetItem(rv, i, ref);
    PyTuple_SetItem(ref, 0, Py_BuildValue("i", hkl[i].GetH()));
    PyTuple_SetItem(ref, 1, Py_BuildValue("i", hkl[i].GetK()));
    PyTuple_SetItem(ref, 2, Py_BuildValue("i", hkl[i].GetL()));
    PyTuple_SetItem(ref, 3, Py_BuildValue("d", hkl[i].GetI()));
    PyTuple_SetItem(ref, 4, Py_BuildValue("d", hkl[i].GetS()));
    PyTuple_SetItem(ref, 5, Py_BuildValue("i", hkl[i].GetBatch()));
    PyTuple_SetItem(ref, 6, Py_BuildValue("b", !hkl[i].IsOmitted()));
  }
  return rv;
}
//..................................................................................................
PyObject* hkl_py::Write(PyObject* self, PyObject* args) {
  olxstr fn;
  PyObject* in;
  if (!PythonExt::ParseTuple(args, "wO", &fn, &in)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "wO");
  }
  if (!PyList_CheckExact(in)) {
    return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
      "A list is expected");
  }
  size_t sz = PyList_Size(in);
  TRefList rf;
  int h, k, l, flag, test_flag = -1;
  double I, S;
  for (size_t i = 0; i < sz; i++) {
    PyObject* prf = PyList_GetItem(in, i);
    if (!PyTuple_CheckExact(prf) || PyTuple_Size(prf) < 6) {
      return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
        "A tuple of 6 items is expected");
    }
    if (!PyArg_ParseTuple(prf, "iiiddi", &h, &k, &l, &I, &S, &flag)) {
      return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
        "Failed to parse the (iiiddi) tuple");
    }
    if (test_flag == -1) {
      test_flag = flag;
    }
    else if (test_flag == TReflection::NoBatchSet &&
      flag != TReflection::NoBatchSet)
    {
      return PythonExt::SetErrorMsg(PyExc_IOError, __OlxSourceInfo,
        "Error: reflections with and without batch numbers are provided");
    }
    rf.Add(new TReflection(h, k, l, I, S, flag)).SetTag(1);
  }
  olxstr error;
  try { THklFile::SaveToFile(fn, rf); }
  catch (const TExceptionBase& e) {
    error = e.GetException()->GetError();
  }
  if (!error.IsEmpty()) {
    return PythonExt::SetErrorMsg(PyExc_IOError, __OlxSourceInfo,
      olxstr("Failed to save the HKL file: ") << fn << '\n' << error);
  }
  return PythonExt::PyNone();
}
//..................................................................................................
PyObject* Fingerprint2Py(const hkl_util::fingerprint_t& fp) {
  PyObject* rv = PyTuple_New(fp.Count());
  for (size_t i = 0; i < fp.Count(); i++) {
    PyTuple_SetItem(rv, i, Py_BuildValue("(di)", fp[i].a, fp[i].b));
  }
  return rv;
}

PyObject* hkl_py::Fingerprint(PyObject* self, PyObject* args) {
  PyObject* in;
  int fp_sz = 12;
  if (!PythonExt::ParseTuple(args, "O|i", &in, &fp_sz)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "O|i");
  }
  if (fp_sz < 10) {
    fp_sz = 10;
  }
  if (PyList_CheckExact(in)) {
    size_t sz = PyList_Size(in);
    TTypeList<hkl_util::Ref> rf;
    double I;
    for (size_t i = 0; i < sz; i++) {
      PyObject* prf = PyList_GetItem(in, i);
      if (!PyArg_Parse(prf, "d", &I)) {
        return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
          "Failed to parse [d]");
      }
      rf.Add(new hkl_util::Ref(I));
    }
    hkl_util::fingerprint_t fp = hkl_util::calc_fingerprint(rf, fp_sz);
    return Fingerprint2Py(fp);
  }
  else {
    olxstr fn = PythonExt::ParseStr(in);
    THklFile hkl;
    olxstr error;
    try {
      hkl.LoadFromFile(fn, false);
    }
    catch (const TExceptionBase& e) {
      return PythonExt::SetErrorMsg(PyExc_IOError, __OlxSourceInfo,
        olxstr("Invalid HKL file: ") << fn << '\n' <<
        e.GetException()->GetError());
    }
    hkl_util::fingerprint_t fp = hkl_util::calc_fingerprint(hkl.RefList(), fp_sz);
    return Fingerprint2Py(fp);
  }
}
//..................................................................................................
hkl_util::fingerprint_t::const_list_type Py2Fingerprint2Py(PyObject* fp) {
  hkl_util::fingerprint_t rv;
  size_t sz = PyTuple_Size(fp);
  rv.SetCapacity(sz);
  double I;
  int c;
  for (size_t i = 0; i < sz; i++) {
    if (!PyArg_ParseTuple(PyTuple_GetItem(fp, i), "di", &I, &c)) {
      throw TInvalidArgumentException(__OlxSourceInfo, "tuple format (di)");
    }
    rv.Add(new olx_pair_t<double, size_t>(I, c));
  }
  return rv;
}
//..................................................................................................
PyObject* hkl_py::FormatFingerprint(PyObject* self, PyObject* args) {
  PyObject* py_fp;
  bool use_N = false;
  if (!PythonExt::ParseTuple(args, "O|b", &py_fp, &use_N)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "O|b");
  }
  try {
    hkl_util::fingerprint_t fp = Py2Fingerprint2Py(py_fp);
    return PythonExt::BuildCString(hkl_util::fingerprint2str(fp, use_N));
  }
  catch (const TInvalidArgumentException& e) {
    return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
      "Invalid Fingerprint");
  }
}
//..................................................................................................
PyObject* hkl_py::Corelate(PyObject* self, PyObject* args) {
  PyObject* py_fp1, * py_fp2;
  if (!PythonExt::ParseTuple(args, "OO", &py_fp1, &py_fp2)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "OO");
  }
  try {
    hkl_util::fingerprint_t fp1 = Py2Fingerprint2Py(py_fp1);
    hkl_util::fingerprint_t fp2 = Py2Fingerprint2Py(py_fp2);
    double v = hkl_util::corelate(fp1, fp2);
    return Py_BuildValue("d", v);
  }
  catch (const TInvalidArgumentException& e) {
    return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
      "Invalid Fingerprint(s)/size");
  }
}

//..................................................................................................

#endif
