/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef _olx_experimantal_H
#define _olx_experimantal_H
#include "xbase.h"
#include "threex3.h"
#include "chemdata.h"
#include "library.h"
#include "macroerror.h"
#include "evalue.h"

#ifdef _PYTHON
  #include "pyext.h"
#endif

BeginXlibNamespace()

enum ERadiationType {
  radiaotion_type_xray = 1,
  radiaotion_type_ed = 2,
  radiation_type_neut = 3,
};

class ExperimentalDetails : public IOlxObject {
  double Radiation, RadiationEnergy;
  TEValueD TempValue;  // always in C
  vec3d CrystalSize;
  ERadiationType RadiationType;
  bool SetSize(const olxstr& t);
  bool SetWL(const olxstr& t);
public:
  ExperimentalDetails()
    : TempValue(-1000), RadiationType(radiaotion_type_xray)
  {
    SetRadiation(0.71073);
  }
  ExperimentalDetails(const ExperimentalDetails& ed)  {
    *this = ed;
  }
  ExperimentalDetails& operator = (const ExperimentalDetails& ed) {
    SetRadiation(ed.Radiation);
    TempValue = ed.TempValue;
    CrystalSize = ed.CrystalSize;
    RadiationType = ed.RadiationType;
    return *this;
  }
  double GetRadiation() const {  return Radiation;  }
  void SetRadiation(double lambda) {
    Radiation = lambda;
    RadiationEnergy = XElementLib::Wavelength2eV(Radiation);
  }
  double GetRadiationEnergy() const {  return RadiationEnergy;  }
  bool IsTemperatureSet() const {  return TempValue.GetV() >= -273.15;  }
  // checks for scale type {F,C,K}
  bool SetTemp(const olxstr& t);
  DefPropC(TEValueD, TempValue);
  DefPropC(vec3d, CrystalSize);
  DefPropC(ERadiationType, RadiationType);
  void SetCrystalSize(double x, double y, double z) {
    CrystalSize = vec3d(x, y, z);
  }
  void Clear() {
    SetRadiation(0.71073);
    CrystalSize.Null();
    TempValue = -1000;
    RadiationType = radiaotion_type_xray;
  }
  void ToDataItem(class TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport();
#endif
  void FromDataItem(const TDataItem& item);

  void LibTemperature(const TStrObjList& Params, TMacroData& E);
  void LibRadiation(const TStrObjList& Params, TMacroData& E);
  void LibSize(const TStrObjList& Params, TMacroData& E);

  class TLibrary* ExportLibrary(const olxstr& name=EmptyString());
  struct VPtr : public olx_virtual_ptr<ExperimentalDetails> {
    virtual IOlxObject *get_ptr() const;
  };
};

EndXlibNamespace()
#endif
