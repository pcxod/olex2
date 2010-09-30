#ifndef __olx_xl_symmat_H
#define __olx_xl_symmat_H
#include "threex3.h"
#include "testsuit.h"

template <class MC, class VC> class TSymmMat {
  uint32_t Id;
public:
  TSymmMat() : Id(~0) {  }
  // copy constructor
  TSymmMat(const TSymmMat<MC,VC>& v) : 
    r(v.r), 
    t(v.t),
    Id(v.Id) {  }
  template <typename AMC, typename AVC> 
  TSymmMat(const TSymmMat<AMC,AVC>& v) : 
    r(v.r), 
    t(v.t),
    Id(~0) {  }
  // composing constructor
  template <typename AMC, typename AVC> 
  TSymmMat(const TMatrix33<AMC>& m, const TVector3<AVC>& v) :
    r(m), t(v), Id(~0) {  }

  template <class AT> 
  inline TVector3<VC> operator * (const TVector3<AT>& a) const {
    return TVector3<VC>(r*a).operator +=(t);
  }
  
  inline TSymmMat<MC,VC> operator * (const TSymmMat<MC,VC>& v) const {
    return TSymmMat<MC,VC>(v.r*r, v*t);
  }

  inline TSymmMat<MC,VC>& operator *= (const TSymmMat<MC,VC>& v)  {
    r *= v.r;
    t = v * t;
    return *this;
  }

  inline bool operator == (const TSymmMat<MC,VC>& v) const {
    return (r == v.r && t == v.t);
  }
  /* compares rotational part directly, but does distance comparison for translation 
  to prevent rounding errors influence*/
  bool EqualExt(const TSymmMat<MC,VC>& v) const {
    return (r == v.r && t.QDistanceTo(v.t) < 1e-6);
  }
  
  inline TSymmMat& operator = (const TSymmMat& sm)  {
    t = sm.t;
    r = sm.r;
    Id = sm.Id;
    return *this;
  }

  template <class AMC, class AVC> 
  inline TSymmMat& operator = (const TSymmMat<AMC,AVC>& sm)  {
    t = sm.t;
    r = sm.r;
    Id = sm.Id;
    return *this;
  }
  
  template <class AT> inline void operator *= (AT v) {
    r *= v;
    t *= v;
  }
  
  inline TSymmMat<MC,VC>& I()  {
    r.I();
    t.Null();
    return *this;
  }
  inline bool IsI() const  {
    return (r.IsI() && t.QLength() < 1e-6);
  }
  
  inline TSymmMat<MC,VC>& Null()  {
    r.Null();
    t.Null();
    return *this;
  }
  
  inline TSymmMat<MC,VC> Inverse() const  {
    TSymmMat<MC,VC> rv(r.Inverse(), t*-1);
    rv.t = rv.r * rv.t;
    return rv;
  }
  
  static inline TSymmMat<MC,VC>& Inverse(TSymmMat<MC,VC>& m)  {
    m.r = m.r.Inverse();
    m.t = ((m.r*m.t) *= -1);
    return m;
  }

  TVector3<VC> t;
  TMatrix33<MC> r;

  uint32_t GetId() const { return Id;  }
  void SetId(uint8_t id)  {  Id = ((uint32_t)id << 24)|(0x00808080);  }
  void SetRawId(uint32_t id)  {  Id = id;  }
  void SetId(uint8_t id, int8_t ta, int8_t tb, int8_t tc)  {
    Id = ((uint32_t)id << 24)|
         ((uint32_t)(0x80-ta) << 16)|
         ((uint32_t)(0x80-tb) << 8)|
         (uint32_t)(0x80-tc);
  }

  bool IsFirst() const {  return Id == 0x00808080;  }
  uint8_t GetContainerId() const {  return (uint8_t)(Id >> 24);  }
  static uint8_t GetContainerId(uint32_t id) {  return (uint8_t)(id >> 24);  }
  static int8_t GetTx(uint32_t id) {  return (int8_t)(128-(int8_t)((id&0x00FF0000) >> 16));  }
  static int8_t GetTy(uint32_t id) {  return (int8_t)(128-(int8_t)((id&0x0000FF00) >> 8));  }
  static int8_t GetTz(uint32_t id) {  return (int8_t)(128-(int8_t)(id&0x000000FF));  }
  static vec3i GetT(uint32_t id)  {  return vec3i(GetTx(id), GetTy(id), GetTz(id));  }
  static TSymmMat<MC,VC> FromId(uint32_t id, const TSymmMat<MC,VC>& ref)  {
    TSymmMat<MC,VC> rv(ref);
    rv.t[0] += GetTx(id);
    rv.t[1] += GetTy(id);
    rv.t[2] += GetTz(id);
    rv.Id = id;
    return rv;
  }
  static uint32_t GenerateId(uint8_t id, int8_t ta, int8_t tb, int8_t tc) {
    return ((uint32_t)id<<24)|((uint32_t)(0x80-ta)<<16)|((uint32_t)(0x80-tb)<<8)|(uint32_t)(0x80-tc);
  }
  static uint32_t GenerateId(uint8_t id, const vec3i& t) {
    return ((uint32_t)id<<24)|((uint32_t)(0x80-t[0])<<16)|((uint32_t)(0x80-t[1])<<8)|(uint32_t)(0x80-t[2]);
  }
  static uint32_t GenerateId(uint8_t container_id, const TSymmMat<MC,VC>& m, const TSymmMat<MC,VC>& ref)  {
    vec3i t(ref.t - m.t);
    uint32_t rv = ((uint32_t)container_id << 24);
    rv |= ((uint32_t)(0x80-t[0]) << 16);
    rv |= ((uint32_t)(0x80-t[1]) << 8);
    rv |= (uint32_t)(0x80-t[2]);
    return rv;
  }
  static void Tests(OlxTests& t)  {
    t.description = __OlxSourceInfo;
    const uint32_t id_1 = GenerateId(0, -56, -43, -21);
    const uint32_t id_2 = GenerateId(0, vec3i(-56, -43, -21));
    if( id_1 != id_2 )
      throw TFunctionFailedException(__OlxSourceInfo, "ID generation mismatch");
    if( GetContainerId(id_1) != 0 )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid ID");
    if( GetT(id_1)[0] != -56 || GetT(id_1)[1] != -43 || GetT(id_1)[2] != -21 )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid T");
    if( GetT(id_1)[0] != GetTx(id_1) || GetT(id_1)[1] != GetTy(id_1) || GetT(id_1)[2] != GetTz(id_1) )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid T");
  }
};

typedef TSymmMat<int,double>   smatd;
typedef TPtrList<smatd>    smatd_plist;
typedef TTypeList<smatd>   smatd_list;
typedef TSymmMat<int,double>   smatid;
typedef TSymmMat<double,double>   smatdd;

#endif
