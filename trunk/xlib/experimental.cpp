#include "experimental.h"
#include "dataitem.h"
#include "pers_util.h"
#include "library.h"
#include "macroerror.h"

//..................................................................................................
bool ExperimentalDetails::SetTemp(const olxstr& t)  {
  if( t.IsEmpty() )  return false;
  if( !olxstr::o_isdigit(t.Last()) )  {
    TemperatureChar = (char)t.Last();
    Temperature = t.SubStringTo(t.Length()-1).ToDouble();
  }
  else  {
    TemperatureChar = 'C';
    Temperature = t.ToDouble();
  }
  return true;
}
//..................................................................................................
bool ExperimentalDetails::SetSize(const olxstr& s)  {
  TStrList toks(s, 'x');
  if( toks.Count() != 3 )
    return false;;
  for( int i=0; i < 3; i++ )
    CrystalSize[i] = toks[i].ToDouble();
  return true;
}
//..................................................................................................
bool ExperimentalDetails::SetWL(const olxstr& wl)  {
  Radiation = wl.ToDouble();
  return true;
}
//..................................................................................................
void ExperimentalDetails::ToDataItem(TDataItem& item) const {
  item.AddField("radiation", Radiation);
  item.AddField("temperature", olxstr(Temperature) << TemperatureChar);
  item.AddField("crystal_size", PersUtil::VecToStr(CrystalSize));
}
//..................................................................................................
#ifndef _NO_PYTHON
PyObject* ExperimentalDetails::PyExport() {
  PyObject* main = PyDict_New();
  PyDict_SetItemString(main, "radiation", Py_BuildValue("d", Radiation) );
  PyDict_SetItemString(main, "temperature", Py_BuildValue("d", Temperature) );
  PyDict_SetItemString(main, "temperature_char", Py_BuildValue("s#", &TemperatureChar, 1) );
  PyDict_SetItemString(main, "size", Py_BuildValue("(ddd)", CrystalSize[0], CrystalSize[1], CrystalSize[2]) );
  return main;
}
#endif
//..................................................................................................
void ExperimentalDetails::FromDataItem(const TDataItem& item) {
  Radiation = item.GetRequiredField("radiation").ToDouble();
  SetTemp(item.GetRequiredField("temperature"));
  CrystalSize = PersUtil::FloatVecFromStr( item.GetRequiredField("crystal_size") );
}
//..................................................................................................
void ExperimentalDetails::LibTemperature(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( olxstr(Temperature) << TemperatureChar);
  else
    SetTemp(Params[0]);
}
//..................................................................................................
void ExperimentalDetails::LibRadiation(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( olxstr(Radiation) );
  else
    SetWL(Params[0]);
}
//..................................................................................................
void ExperimentalDetails::LibSize(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( olxstr(CrystalSize[0]) << 'x' << CrystalSize[1] << 'x' << CrystalSize[2]);
  else
    SetSize(Params[0]);
}
//..................................................................................................
TLibrary* ExperimentalDetails::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("exptl") : name );
  lib->RegisterFunction<ExperimentalDetails>(
    new TFunction<ExperimentalDetails>(this,  &ExperimentalDetails::LibTemperature, "Temperature", fpNone|fpOne,
"Returns/sets experiment temperature. Returns/accepts strings like 120K, 10C, 10F. Default scale is C") );
  lib->RegisterFunction<ExperimentalDetails>(
    new TFunction<ExperimentalDetails>(this,  &ExperimentalDetails::LibRadiation, "Radiation", fpNone|fpOne,
"Returns/sets experiment wavelength in angstrems") );
  lib->RegisterFunction<ExperimentalDetails>(
    new TFunction<ExperimentalDetails>(this,  &ExperimentalDetails::LibSize, "Size", fpNone|fpOne,
"Returns/sets crystal size. Returns/accepts strings line 0.5x0.5x0.5 (in mm)") );
  return lib;
}
//..................................................................................................
