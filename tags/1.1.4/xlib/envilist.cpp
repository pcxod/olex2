#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "envilist.h"

void TAtomEnvi::ApplySymm(const smatd& sym)  {
  for( size_t i=0; i < Envi.Count(); i++ )  {
    Envi[i].B() *= sym;
    Envi[i].C() = Envi[i].GetB() * Envi[i].GetA()->ccrd();
  }
}

#ifndef _NO_PYTHON
PyObject* TAtomEnvi::PyExport(TPtrList<PyObject>& atoms)  {
  PyObject* neighbours = PyTuple_New( Envi.Count() );
  for( size_t i=0; i < Envi.Count(); i++ )  {
    const smatd& mat = Envi[i].GetB();
    const vec3d& crd = Envi[i].GetC();
    if( mat.r.IsI() && mat.t.IsNull() )  {
      PyTuple_SetItem(neighbours, i, Py_BuildValue("i", Envi[i].GetA()->GetTag()));
    }
    else  {
      PyTuple_SetItem(neighbours, i, 
        Py_BuildValue("OOO", Py_BuildValue("i", Envi[i].GetA()->GetTag()),
          Py_BuildValue("(ddd)", crd[0], crd[1], crd[2]),
            Py_BuildValue("(iii)(iii)(iii)(ddd)", 
              mat.r[0][0], mat.r[0][1], mat.r[0][2],
              mat.r[1][0], mat.r[1][1], mat.r[1][2],
              mat.r[2][0], mat.r[2][1], mat.r[2][2],
              mat.t[0], mat.t[1], mat.t[2] )
         )
      );
    }
  }
  return neighbours;
}
#endif

 
