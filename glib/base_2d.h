#include "exception.h"
#include "emath.h"
#include "threex3.h"

template <class T> class TVector2  {
  T data[2];
public:
  TVector2() {
    data[0] = data[1] = 0;
  }
  TVector2(T x, T y) {
    data[0] = x;  data[1] = y;
  }
  TVector2(const TVector2& v) {
    data[0] = v.data[0];
    data[1] = v.data[1];
  }
  template <class AT> TVector2(const AT& v) {
    data[0] = v[0];
    data[1] = v[1];
  }
  inline T QLength() const {  return data[0]*data[0] + data[1]*data[1];  }
  inline T Length() const {  return sqrt(QLemgth());  }
  template <class T1> inline T QDistanceTo(const TVector2<T1>& v) const {
    return sqr(data[0]-v[0]) + sqr(data[1]-v[1]);
  }
  template <class T1> inline T DistanceTo(const TVector2<T1>& v) const {
    return sqrt( QDistanceTo(v) );
  }
  template <class T1> inline T CAngle(const TVector2<T1>& v) const {
    T l = QLength()*v.QLength();
    if( l == 0 )  throw TDivException(__OlxSourceInfo);
    l = (T)((data[0]*v[0]+data[1]*v[1])/sqrt(l));
    return l > 1.0 ? 1.0 : (l < -1.0 ? -1.0 : l);
  }
  inline T& operator [] (int i) {  return data[i];  }
  inline const T& operator [] (int i) const {  return data[i];  }
  
  inline TVector2& Normalise() {
    const double ln = Length();
    data[0] /= ln;  data[1] /= ln;
    return *this;
  }
  inline TVector2& Null() {
    data[0] = data[1] = 0;
    return *this;
  }

  inline TVector2& operator = (const TVector2& v) {
    data[0] = v[0];  data[1] = v[1];
    return *this;
  }
  template <class AV> inline TVector2& operator = (const AV& v) {
    data[0] = v[0];  data[1] = v[1];
    return *this;
  }

  template <typename T1> inline TVector2& operator = (const TVector2<T1>& v) {
    data[0] = v[0];  data[1] = v[1];
    return *this;
  }
  template <typename T1> inline TVector2& operator += (const TVector2<T1>& v) {
    data[0] += v[0];  data[1] += v[1];
    return *this;
  }
  template <typename T1> inline TVector2& operator -= (const TVector2<T1>& v) {
    data[0] -= v[0];  data[1] -= v[1];
    return *this;
  }
  template <typename T1> inline TVector2& operator *= (const TVector2<T1>& v) {
    data[0] *= v[0];  data[1] *= v[1];
    return *this;
  }
  template <typename T1> inline TVector2& operator /= (const TVector2<T1>& v) {
    data[0] /= v[0];  data[1] /= v[1];
    return *this;
  }
  template <typename T1> inline TVector2 operator + (const TVector2<T1>& v) const {
    return TVector2(data[0]+v[0], data[1]+v[1]);
  }
  template <typename T1> inline TVector2 operator - (const TVector2<T1>& v) const {
    return TVector2(data[0]-v[0], data[1]-v[1]);
  }
  template <typename T1> inline TVector2 operator * (const TVector2<T1>& v) const {
    return TVector2(data[0]*v[0], data[1]*v[1]);
  }
  template <typename T1> inline TVector2 operator / (const TVector2<T1>& v) const {
    return TVector2(data[0]/v[0], data[1]/v[1]);
  }

  template <typename T1> inline TVector2& operator = (const TVector3<T1>& v) {
    data[0] = v[0];  data[1] = v[1];
    return *this;
  }
  template <typename T1> inline TVector2& operator += (const TVector3<T1>& v) {
    data[0] += v[0];  data[1] += v[1];
    return *this;
  }
  template <typename T1> inline TVector2& operator -= (const TVector3<T1>& v) {
    data[0] -= v[0];  data[1] -= v[1];
    return *this;
  }
  template <typename T1> inline TVector2& operator *= (const TVector3<T1>& v) {
    data[0] *= v[0];  data[1] *= v[1];
    return *this;
  }
  template <typename T1> inline TVector2& operator /= (const TVector3<T1>& v) {
    data[0] /= v[0];  data[1] /= v[1];
    return *this;
  }
  template <typename T1> inline TVector2 operator + (const TVector3<T1>& v) const {
    return TVector2(data[0]+v[0], data[1]+v[1]);
  }
  template <typename T1> inline TVector2 operator - (const TVector3<T1>& v) const {
    return TVector2(data[0]-v[0], data[1]-v[1]);
  }
  template <typename T1> inline TVector2 operator * (const TVector3<T1>& v) const {
    return TVector2(data[0]*v[0], data[1]*v[1]);
  }
  template <typename T1> inline TVector2 operator / (const TVector3<T1>& v) const {
    return TVector2(data[0]/v[0], data[1]/v[1]);
  }

  template <typename T1> inline TVector2& operator += (const T1& v) {
    data[0] += v;  data[1] += v;
    return *this;
  }
  template <typename T1> inline TVector2& operator -= (const T1& v) {
    data[0] -= v;  data[1] -= v;
    return *this;
  }
  template <typename T1> inline TVector2& operator *= (const T1& v) {
    data[0] *= v;  data[1] *= v;
    return *this;
  }
  template <typename T1> inline TVector2& operator /= (const T1& v) {
    data[0] /= v;  data[1] /= v;
    return *this;
  }
  template <typename T1> inline TVector2 operator + (const T1& v) const {
    return TVector2(data[0]+v, data[1]+v);
  }
  template <typename T1> inline TVector2 operator - (const T1& v) const {
    return TVector2(data[0]-v, data[1]-v);
  }
  template <typename T1> inline TVector2 operator * (const T1& v) const {
    return TVector2(data[0]*v, data[1]*v);
  }
  template <typename T1> inline TVector2 operator / (const T1& v) const {
    return TVector2(data[0]/v, data[1]/v);
  }

};

typedef TVector2<double> vec2d;
typedef TVector2<int> vec2i;

class TArc2D {
  double R, Angle;
  vec2d Center;
  bool Valid;
  vec2d From, To;
protected:
  void Init()  {
    Angle = R = 0;
    Valid = false;
  }
public:
  TArc2D()  {  Init();  }
  TArc2D(const vec2d& p1, const vec2d& p2, const vec2d& p3)  {
    Init();
    Initialise(p1, p2, p3);
  }
  //http://local.wasp.uwa.edu.au/~pbourke/geometry/circlefrom3/
  bool Initialise(const vec2d& p1, const vec2d& p2, const vec2d& p3)  {
    const double dx1 = p2[0] - p1[0],
                 dx2 = p3[0] - p2[0];
    if( dx1 != 0 && dx2 != 0 )  {
      const double ma = (p2[1] - p1[1])/dx1,
                   mb = (p3[1] - p2[1])/dx2;
      if( ma != mb )  {
        Center[0] = (ma*mb*(p1[1]-p3[1]) + mb*(p1[0]+p2[0]) -ma*(p2[0]+p3[0]))/(2*(mb-ma));
        Center[1] = ((p1[0]+p2[0])/2-Center[0])/ma + (p1[1]+p2[1])/2;
        R = Center.DistanceTo(p1);
        vec2d r1 = p1 - Center,
              r2 = p3 - Center;
        Angle = acos(r1.CAngle(r2));
        bool swap = false;
        double cw_test = r1[0]*r2[1] - r2[0]*r1[1];  // triangle "area", xprod in 2d
        if( cw_test < 0 )  {
          From = p3;
          To = p1;
        }
        else  {
          From = p1;
          To = p3;
        }
        Valid = true;
      }
    }
    return Valid;
  }
  inline bool IsValid()         const {  return Valid;  }
  const vec2d& GetFrom()        const {  return From;  }
  const vec2d& GetTo()          const {  return To;  }
  const vec2d& GetCenter()      const {  return Center;  }
  const double& GetAngle()      const {  return Angle;  }
  const double& GetRadius()     const {  return R;  }
};

class TEllipse2D  {
  TArc2D arcs[6];
public:
  TEllipse2D(const mat3d& matrix, const mat3d& proj_matr)  {
    mat3d pm(matrix), nm;
    for( int i=0; i < 3; i++ )  {
      if( pm[i].CAngle(proj_matr[2]) < 0 )
        pm[i] *= -1;
      nm[i] = -pm[i]*proj_matr;
      pm[i] *= proj_matr;
    }
    arcs[0].Initialise(nm[0], pm[1], pm[0]);
    arcs[1].Initialise(nm[0], nm[1], pm[0]);
    arcs[2].Initialise(nm[0], pm[2], pm[0]);
    arcs[3].Initialise(nm[0], nm[2], pm[0]);

    arcs[4].Initialise(nm[1], pm[2], pm[1]);
    arcs[5].Initialise(nm[1], nm[2], pm[1]);
  }
  const TArc2D& GetArc(int i) const {  return arcs[i];  }
};