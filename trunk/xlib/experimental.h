#ifndef _olx_experimantal_H
#define _olx_experimantal_H
#include "xbase.h"
#include "threex3.h"
#include "chemdata.h"

#ifndef _NO_PYTHON
  #include "pyext.h"
#endif

BeginXlibNamespace()

class ExperimentalDetails {
  double Radiation, RadiationEnergy;
  double Temperature;  // always in C
  vec3d CrystalSize;
  bool SetTemp(const olxstr& t);
  bool SetSize(const olxstr& t);
  bool SetWL(const olxstr& t);
public:
  ExperimentalDetails() : Temperature(-1000) {
    SetRadiation(0.71073);
  }
  ExperimentalDetails(const ExperimentalDetails& ed)  {
    *this = ed;
  }
  ExperimentalDetails& operator = (const ExperimentalDetails& ed) {
    Radiation = ed.Radiation;
    Temperature = ed.Temperature;
    CrystalSize = ed.CrystalSize;
    return *this;
  }
  double GetRadiation() const {  return Radiation;  }
  void SetRadiation(double lambda) {
    Radiation = lambda;
    RadiationEnergy = XElementLib::Wavelength2eV(Radiation);
  }
  double GetRadiationEnergy() const {  return RadiationEnergy;  }
  bool IsTemperatureSet() const {  return Temperature >= -273.15;  }

  DefPropP(double, Temperature)
  DefPropC(vec3d, CrystalSize)
  void SetCrystalSize(double x, double y, double z)  {
    CrystalSize = vec3d(x,y,z);
  }
  void Clear() {
    SetRadiation(0.71073);
    CrystalSize.Null();
    Temperature = -1000;
  }
  void ToDataItem(class TDataItem& item) const;
#ifndef _NO_PYTHON
  PyObject* PyExport();
#endif
  void FromDataItem(const TDataItem& item);

  void LibTemperature(const TStrObjList& Params, TMacroError& E);
  void LibRadiation(const TStrObjList& Params, TMacroError& E);
  void LibSize(const TStrObjList& Params, TMacroError& E);
  
  class TLibrary* ExportLibrary(const olxstr& name=EmptyString);
};

EndXlibNamespace()
#endif

