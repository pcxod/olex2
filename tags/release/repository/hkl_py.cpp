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
  if( !PythonExt::ParseTuple(args, "w", &fn) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  if( !TEFile::Exists(fn) )  {
    PyErr_SetObject(PyExc_IOError, 
      PythonExt::BuildString( olxstr("File does not exist: ") << fn) );
    Py_INCREF(Py_None);
    return Py_None;
  }
  THklFile hkl;
  bool res = false;
  olxstr error;
  try  {  res = hkl.LoadFromFile(fn);  }
  catch( const TExceptionBase& e )  {  error = e.GetException()->GetError();  }
  if( !res )  {
    PyErr_SetObject(PyExc_IOError, 
      PythonExt::BuildString( olxstr("Invalid HKL file: ") << fn << '\n' << error ) );
    Py_INCREF(Py_None);
    return Py_None;
  }
  PyObject* rv = PyTuple_New( hkl.RefCount() );
  for( int i=0; i < hkl.RefCount(); i++ )  {
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
  if( !PythonExt::ParseTuple(args, "wO", &fn, &in) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  if( !PyList_CheckExact(in) )  {
    PyErr_SetObject(PyExc_RuntimeError, PythonExt::BuildString("A list is expected"));
    Py_INCREF(Py_None);
    return Py_None;
  }
  size_t sz = PyList_Size(in);
  TRefList rf;
  int h, k, l, flag, test_flag = -1;
  double I, S;
  for( size_t i=0; i < sz; i++ )  {
    PyObject* prf = PyList_GetItem(in, i);
    if( !PyTuple_CheckExact(prf) || PyTuple_Size(prf) < 6)  {
      PyErr_SetObject(PyExc_RuntimeError, PythonExt::BuildString("A tuple of 6 items is expected"));
      Py_INCREF(Py_None);
      return Py_None;
    }
    if( !PyArg_ParseTuple(prf, "iiiddi", &h, &k, &l, &I, &S, &flag) )  {
      PyErr_SetObject(PyExc_RuntimeError, PythonExt::BuildString("Failed to parse the (iiiddi) tuple"));
      Py_INCREF(Py_None);
      return Py_None;
    }
    if( test_flag == -1 )  
       test_flag = flag;
    else if( test_flag == NoFlagSet && flag != NoFlagSet )  {
      PyErr_SetObject(PyExc_IOError, 
        PythonExt::BuildString( "Error: reflections with and without batch numbers are provided") );
      Py_INCREF(Py_None);
      return Py_None;
    }
    rf.Add(new TReflection(h, k, l, I, S, flag)).SetTag(1);
  }
  bool res = false;
  olxstr error;
  try  {  res = THklFile::SaveToFile(fn, rf);  }
  catch( const TExceptionBase& e )  {  error = e.GetException()->GetError();  }
  if( !res )  {
    PyErr_SetObject(PyExc_IOError, 
      PythonExt::BuildString( olxstr("Failed to save the HKL file: ") << fn << '\n' << error ) );
  }
  Py_INCREF(Py_None);
  return Py_None;
}
