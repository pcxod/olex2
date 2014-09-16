/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "fracmask.h"
#include "bitarray.h"
#include "pers_util.h"

void FractMask::Init(const vec3d& _min, const vec3d& _max, const vec3d& norms, double resolution)  {
  if( Mask != NULL )  {
    delete Mask;
    Mask = NULL; // in case of the exception
  }
  Norm = norms/resolution;
  const TVector3<index_t> min_((_min*Norm).Round<index_t>()), max_((_max*Norm).Round<index_t>());
  if( min_[0] >= max_[0] || min_[1] >= max_[1] || min_[2] >= max_[2] )
    throw TInvalidArgumentException(__OlxSourceInfo, "mask size");

  Mask = new TArray3D<bool>(
    min_[0], max_[0], 
    min_[1], max_[1], 
    min_[2], max_[2]
  );
  Mask->FastInitWith(0);
}
//..................................................................................................
void FractMask::ToDataItem(TDataItem& di, IOutputStream& os) const {
  TEBitArray ba(Mask->Length1()*Mask->Length2()*Mask->Length3());
  di.AddField("norm", PersUtil::VecToStr(Norm));
  di.AddField("min", PersUtil::VecToStr(TVector3<index_t>(Mask->GetMin1(), Mask->GetMin2(), Mask->GetMin3())));
  di.AddField("max", 
    PersUtil::VecToStr(TVector3<index_t>(Mask->Length1()+Mask->GetMin1()-1, 
      Mask->Length2()+Mask->GetMin2()-1, Mask->Length3()+Mask->GetMin3()-1))
  );
  di.AddField("char_count", ba.CharCount());
  for( size_t x=0; x < Mask->Length1(); x++ )  {
    const size_t offx = x*Mask->Length2();
    for( size_t y=0; y < Mask->Length2(); y++ )  {
      const size_t off = (y+offx)*Mask->Length3();
      for( size_t z=0; z < Mask->Length3(); z++ )  {
        ba.Set(z + off, Mask->Data[x][y][z]);
      }
    }
  }
  os.Write(ba.GetData(), ba.CharCount());
}
//..................................................................................................
void FractMask::FromDataItem(const TDataItem& di, IInputStream& is)  {
  PersUtil::VecFromStr(di.GetFieldByName("norm"), Norm);
  vec3i _min = PersUtil::VecFromStr<vec3i>(di.GetFieldByName("min"));
  vec3i _max = PersUtil::VecFromStr<vec3i>(di.GetFieldByName("max"));
  size_t cc = di.GetFieldByName("char_count").ToSizeT();
  if( Mask != NULL )
    delete Mask;
  Mask = new TArray3D<bool>(_min[0], _max[0], _min[1], _max[1], _min[2], _max[2]);
  unsigned char* data = new unsigned char[cc];
  is.Read(data, cc);
  TEBitArray ba(data, cc, true);
  for( size_t x=0; x < Mask->Length1(); x++ )  {
    const size_t offx = x*Mask->Length2();
    for( size_t y=0; y < Mask->Length2(); y++ )  {
      const size_t off = (y+offx)*Mask->Length3();
      for( size_t z=0; z < Mask->Length3(); z++ )  {
        Mask->Data[x][y][z] = ba[z + off];
      }
    }
  }
}
