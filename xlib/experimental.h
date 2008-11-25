#ifndef _olx_experimantal_H
#define _olx_experimantal_H
#include "xbase.h"
#include "threex3.h"
BeginXlibNamespace()

class TExperimentalDetails {
  double Radiation;
  double Temperature;
  vec3d CrystalSize;
public:
  TExperimentalDetails() : Radiation(0.71073), Temperature(150) {}
  TExperimentalDetails(const ExperimentalDetails& ed)  {
    *this = ed;
  }
  TExperimentalDetails& operator = (const TExperimentalDetails& ed) {
    Radiation = ed.Radiation;
    Temperature = ed.Temperature;
    CrystalSize = ed.CrystalSize;
    return *this;
  }
  DefPropP(double, Radiation)
  DefPropP(double, Temperature)
  DefPropC(vec3d, CrystalSize)
  void SetCrystalSize(double x, double y, double z)  {
    CrystalSize = vec3d(x,y,z);
  }
};

EndXlibNamespace()
#endif