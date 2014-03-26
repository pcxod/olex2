/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "envilist.h"
#include "lattice.h"

//.............................................................................
void TAtomEnvi::ApplySymm(const smatd& sym) {
  for (size_t i=0; i < Envi.Count(); i++) {
    Envi[i].b *= sym;
    Envi[i].c = Envi[i].GetB() * Envi[i].GetA()->ccrd();
  }
}
//.............................................................................
size_t TAtomEnvi::CountCovalent() const {
  if (Envi.IsEmpty() || Base == NULL) return 0;
  size_t cnt = 0;
  const double delta = Base->GetNetwork().GetLattice().GetDelta();
  for (size_t i=0; i < Envi.Count(); i++) {
    if (TNetwork::BondExistsQ(Base->CAtom(), *Envi[i].GetA(),
      Envi[i].GetC().QDistanceTo(Base->crd()), delta))
    {
      cnt++;
    }
  }
  return cnt;
}
//.............................................................................
#ifdef _PYTHON
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
