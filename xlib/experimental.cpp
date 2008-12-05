#include "experimental.h"
#include "dataitem.h"

//..................................................................................................
void ExperimentalDetails::ToDataItem(TDataItem& item) const {
  item.AddField("radiation", Radiation);
  item.AddField("temparature", Temperature);
  item.AddField("crystal_size", olxstr(CrystalSize[0]) << 'x' << 
    CrystalSize[1] << 'x' << CrystalSize[2]);
}
//..................................................................................................
void ExperimentalDetails::FromDataItem(const TDataItem& item) {
  SetRadiation(item.GetFieldValue("radiation", "0.71073").ToDouble());
  Temperature = item.GetFieldValue("temperature", "0").ToDouble();
  const olxstr& cs = item.GetFieldValue("crystal_size");
  if( !cs.IsEmpty() )  {
    TStrList toks(cs, 'x');
    if( toks.Count() != 3 )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid crysta size format");
    CrystalSize[0] = toks[0].ToDouble();
    CrystalSize[1] = toks[1].ToDouble();
    CrystalSize[2] = toks[2].ToDouble();
  }
}
//..................................................................................................
