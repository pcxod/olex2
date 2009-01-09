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
void ExperimentalDetails::FromDataItem(const TDataItem& item) {
  SetRadiation(item.GetRequiredField("radiation").ToDouble());
  Temperature = item.GetRequiredField("temperature").ToDouble();
  CrystalSize = PersUtil::FloatVecFromStr( item.GetRequiredField("crystal_size") );
}
//..................................................................................................
