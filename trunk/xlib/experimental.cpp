#include "experimental.h"
#include "dataitem.h"
#include "pers_util.h"

//..................................................................................................
void ExperimentalDetails::ToDataItem(TDataItem& item) const {
  item.AddCodedField("radiation", Radiation);
  item.AddCodedField("temperature", Temperature);
  item.AddCodedField("crystal_size", PersUtil::VecToStr(CrystalSize));
}
//..................................................................................................
void ExperimentalDetails::FromDataItem(const TDataItem& item) {
  SetRadiation(item.GetRequiredField("radiation").ToDouble());
  Temperature = item.GetRequiredField("temperature").ToDouble();
  CrystalSize = PersUtil::FloatVecFromStr( item.GetRequiredField("crystal_size") );
}
//..................................................................................................
