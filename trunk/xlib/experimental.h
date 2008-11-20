#ifndef _olx_experimantal_H
#define _olx_experimantal_H
#include "xbase.h"
#include "threex3.h"
BeginXlibNamespace()

class ExperimentalDetails {
  double Radiation;
  double Temperature;
  vec3d CrystalSize;
public:
  ExperimentalDetails() : Radiation(0.71073), Temperature(150) {}
  ExperimentalDetails(const ExperimentalDetails& ed)  {
    *this = ed;
  }
  ExperimentalDetails& operator = (const ExperimentalDetails& ed) {
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