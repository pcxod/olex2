#include "experimental.h"
#include "dataitem.h"
#include "pers_util.h"

//..................................................................................................
void ExperimentalDetails::ToDataItem(TDataItem& item) const {
  item.AddField("radiation", Radiation);
  item.AddField("temperature", Temperature);
  item.AddField("crystal_size", PersUtil::VecToStr(CrystalSize));
}
//..................................................................................................
#ifndef _NO_PYTHON
PyObject* ExperimentalDetails::PyExport() {
  PyObject* main = PyDict_New();
  PyDict_SetItemString(main, "radiation", Py_BuildValue("d", Radiation) );
  PyDict_SetItemString(main, "temperature", Py_BuildValue("d", Temperature) );
  PyDict_SetItemString(main, "size", Py_BuildValue("(ddd)", CrystalSize[0], CrystalSize[1], CrystalSize[2]) );
  return main;
}
#endif
//..................................................................................................
void ExperimentalDetails::FromDataItem(const TDataItem& item) {
  SetRadiation(item.GetRequiredField("radiation").ToDouble());
  Temperature = item.GetRequiredField("temperature").ToDouble();
  CrystalSize = PersUtil::FloatVecFromStr( item.GetRequiredField("crystal_size") );
}
//..................................................................................................
