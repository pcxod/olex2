#include "fracmask.h"
#include "bitarray.h"
#include "pers_util.h"

void FractMask::Init(const vec3d& _min, const vec3d& _max, const vec3f& norms, float resolution)  {
  if( Mask != NULL )  {
    delete Mask;
    Mask = NULL; // in case of the exception
  }
  Norm = norms/resolution;
  vec3d min = _min*Norm,
    max = _max*Norm;
  if( min[0] >= max[0] ||
    min[1] >= max[1] ||
    min[2] >= max[2] )
    throw TInvalidArgumentException(__OlxSourceInfo, "mask size");
  Mask = new TArray3D<bool>(
    (int)min[0], (int)max[0], 
    (int)min[1], (int)max[1], 
    (int)min[2], (int)max[2]
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
  Norm = PersUtil::FloatVecFromStr(di.GetRequiredField("norm"));
  vec3i _min = PersUtil::IntVecFromStr(di.GetRequiredField("min"));
  vec3i _max = PersUtil::IntVecFromStr(di.GetRequiredField("max"));
  size_t cc = di.GetRequiredField("char_count").ToSizeT();
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

