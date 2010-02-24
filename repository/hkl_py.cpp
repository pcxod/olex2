#include "hkl_py.h"


PyMethodDef hkl_py::Methods[] = {
  {"Read", hkl_py::Read, METH_VARARGS, "reads an HKL file and returns a tuple of the reflections"},
  {"Write", hkl_py::Write, METH_VARARGS, "writes provided tuple to an hkl file"},
  {NULL, NULL, 0, NULL}
};

void hkl_py::PyInit()  {
  Py_InitModule( "olex_hkl", Methods );
}

//..................................................................................................
PyObject* hkl_py::Read(PyObject* self, PyObject* args)  {
  olxstr fn;
  if( !PythonExt::ParseTuple(args, "w", &fn) )
    return PythonExt::PyNone();
  if( !TEFile::Exists(fn) )  {
    PythonExt::SetErrorMsg(PyExc_IOError, olxstr("File does not exist: ") << fn);
    return PythonExt::PyNone();
  }
  THklFile hkl;
  bool res = false;
  olxstr error;
  try  {  res = hkl.LoadFromFile(fn);  }
  catch( const TExceptionBase& e )  {  error = e.GetException()->GetError();  }
  if( !res )  {
    PythonExt::SetErrorMsg(PyExc_IOError, olxstr("Invalid HKL file: ") << fn << '\n' << error);
    return PythonExt::PyNone();
  }
  PyObject* rv = PyTuple_New( hkl.RefCount() );
  for( size_t i=0; i < hkl.RefCount(); i++ )  {
    PyObject* ref = PyTuple_New(7);
    PyTuple_SetItem(rv, i, ref);
    PyTuple_SetItem(ref, 0, Py_BuildValue("i", hkl[i].GetH()) );
    PyTuple_SetItem(ref, 1, Py_BuildValue("i", hkl[i].GetK()) );
    PyTuple_SetItem(ref, 2, Py_BuildValue("i", hkl[i].GetL()) );
    PyTuple_SetItem(ref, 3, Py_BuildValue("d", hkl[i].GetI()) );
    PyTuple_SetItem(ref, 4, Py_BuildValue("d", hkl[i].GetS()) );
    PyTuple_SetItem(ref, 5, Py_BuildValue("i", hkl[i].GetFlag()) );
    PyTuple_SetItem(ref, 6, Py_BuildValue("b", hkl[i].GetTag() >= 0) );
  }
  return rv;
}
//..................................................................................................
PyObject* hkl_py::Write(PyObject* self, PyObject* args)  {
  olxstr fn;
  PyObject* in;
  if( !PythonExt::ParseTuple(args, "wO", &fn, &in) )
    return PythonExt::PyNone();
  if( !PyList_CheckExact(in) )  {
    PythonExt::SetErrorMsg(PyExc_RuntimeError, "A list is expected");
    return PythonExt::PyNone();
  }
  size_t sz = PyList_Size(in);
  TRefList rf;
  int h, k, l, flag, test_flag = -1;
  double I, S;
  for( size_t i=0; i < sz; i++ )  {
    PyObject* prf = PyList_GetItem(in, i);
    if( !PyTuple_CheckExact(prf) || PyTuple_Size(prf) < 6)  {
      PythonExt::SetErrorMsg(PyExc_RuntimeError, "A tuple of 6 items is expected");
      return PythonExt::PyNone();
    }
    if( !PyArg_ParseTuple(prf, "iiiddi", &h, &k, &l, &I, &S, &flag) )  {
      PythonExt::SetErrorMsg(PyExc_RuntimeError, "Failed to parse the (iiiddi) tuple");
      return PythonExt::PyNone();
    }
    if( test_flag == -1 )  
       test_flag = flag;
    else if( test_flag == NoFlagSet && flag != NoFlagSet )  {
      PythonExt::SetErrorMsg(PyExc_IOError,
        "Error: reflections with and without batch numbers are provided");
      return PythonExt::PyNone();
    }
    rf.Add(new TReflection(h, k, l, I, S, flag)).SetTag(1);
  }
  bool res = false;
  olxstr error;
  try  {  res = THklFile::SaveToFile(fn, rf);  }
  catch( const TExceptionBase& e )  {  error = e.GetException()->GetError();  }
  if( !res )  {
    PythonExt::SetErrorMsg(PyExc_IOError,
      olxstr("Failed to save the HKL file: ") << fn << '\n' << error);
  }
  return PythonExt::PyNone();
}
