#ifndef _olx_experimantal_H
#define _olx_experimantal_H
#include "xbase.h"
#include "threex3.h"
BeginXlibNamespace()

class ExperimentalDetails {
  double Radiation, RadiationEnergy;
  double Temperature;
  vec3d CrystalSize;
public:
  ExperimentalDetails() : Temperature(0) {
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

  DefPropP(double, Temperature)
  DefPropC(vec3d, CrystalSize)
  void SetCrystalSize(double x, double y, double z)  {
    CrystalSize = vec3d(x,y,z);
  }
  void Clear() {
    SetRadiation(0.71073);
    CrystalSize.Null();
    Temperature = 0;
  }
};

EndXlibNamespace()
#endif

