#ifndef __olxs_v_co_v_h
#define __olxs_v_co_v_h
#include "math/align.h"
#include "math/composite.h"
#include "asymmunit.h"
#include "lattice.h"
#include "bapp.h"
#include "log.h"
BeginXlibNamespace()

const short // constants decribing the stored values
  vcoviX = 0x0001,
  vcoviY = 0x0002,
  vcoviZ = 0x0004,
  vcoviO = 0x0008;
// stores X,Y,Z,SOF for each atom and their correlations
class VcoVMatrix {
  double **data;
  size_t count;
  TTypeList<AnAssociation3<olxstr, short, size_t> > Index;
  static TStrList U_annotations;
protected:
  void Allocate(size_t w) {
    Clear();
    count = w;
    data = new double*[w];
    for( size_t i=0; i < w; i++ ) // bottom diagonal agrrangement
      data[i] = new double[i+1];
  }
  size_t FindAtomIndex(const TCAtom& a) const {
    for( size_t i=0; i < Index.Count(); i++ )
      if( Index[i].GetC() == a.GetId() )
        return i;
    return InvalidIndex;
  }
public:
  VcoVMatrix();
  ~VcoVMatrix() {  Clear();  }
  void Clear() {
    if( data == NULL )  return;
    Index.Clear();
    for( size_t i=0; i < count; i++ )
      delete [] data[i];
    delete [] data;
    data = NULL;
  }
  double operator () (size_t i, size_t j) const {
    return (j <= i ) ? data[i][j] : data[j][i];
  }
  double Get(size_t i, size_t j) const {
    return (j <= i ) ? data[i][j] : data[j][i];
  }
  // reads the shelxl VcoV matrix
  void ReadShelxMat(const olxstr& fileName, TAsymmUnit& au);
  // reads the smtbx VcoV matrix
  void ReadSmtbxMat(const olxstr& fileName, TAsymmUnit& au);
  // creates matrices AA, AB, ... AX, BA, BB, ... BX, ...
  template <class list> void FindVcoV(const list& atoms, mat3d_list& m) const {
    TSizeList a_indexes;
    TTypeList<TVector3<size_t> > indexes;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( a_indexes.Add(FindAtomIndex(atoms[i]->CAtom())) == InvalidIndex )
        TBasicApp::GetLog().Error(olxstr("Unable to located given atom: ") << atoms[i]->GetLabel());
      indexes.AddNew(InvalidIndex,InvalidIndex,InvalidIndex);
    }
    for( size_t i=0; i < a_indexes.Count(); i++ )  {
      if( a_indexes[i] == InvalidIndex )  continue;
      for( size_t j=a_indexes[i]; j < Index.Count() && Index[j].GetC() == atoms[i]->CAtom().GetId(); j++ )  {
        if( (Index[j].GetB() & vcoviX) != 0 )
          indexes[i][0] = j;
        if( (Index[j].GetB() & vcoviY) != 0 )
          indexes[i][1] = j;
        if( (Index[j].GetB() & vcoviZ) != 0 )
          indexes[i][2] = j;
      }
    }
    for( size_t i=0; i < a_indexes.Count(); i++ )  {
      for( size_t j=0; j < a_indexes.Count(); j++ )  {
        mat3d& a = m.AddNew();
        for( short k=0; k < 3; k++ )  {
          for( short l=0; l < 3; l++ )  {
            if( indexes[i][k] != InvalidIndex && indexes[j][l] != InvalidIndex )  {
              a[k][l] = Get(indexes[i][k], indexes[j][l]);
            }
          }
        }
      }
    }
  }
  // for tests
  double Find(const olxstr& atom, const short va, const short vy) const;
};

class VcoVContainer {
  VcoVMatrix vcov;
  TDoubleList weights[3];
  vec3d plane_param[3], plane_center[3];
  evecd cell, celle;  // celle can be extended to matrix once covariances are available
  TAsymmUnit& au;
  struct TwoInts  {
    size_t a, b;
  };
public:
  template <class atom_list> void ProcessSymmetry(const atom_list& atoms, mat3d_list& ms)  {
    mat3d_list left(atoms.Count()), right(atoms.Count());
    size_t mc = 0;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      const mat3d& m = atoms[i]->GetMatrix(0).r;
      if( m.IsI() )  continue;
      left.SetCCopy(i, m); 
      right.SetCCopy(i, m).Transpose();
      mc++;
    }
    if( mc == 0 )  return;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      for( size_t j=0; j < atoms.Count(); j++ )  {
        if( !left.IsNull(i) )  {
          size_t l_ind = i*atoms.Count() + j;
          ms[l_ind] = left[i]*ms[l_ind];
        }
        if( !right.IsNull(i) )  {
          size_t r_ind = j*atoms.Count() + i;
          ms[r_ind] *= right[i];
        }
      }
    }
  }
  // helper functions, matrices are in cartesian frame
  template <class atom_list> void GetVcoV(const atom_list& as, mat3d_list& m)  {
    vcov.FindVcoV(as, m);
    ProcessSymmetry(as, m);
    mat3d c2f(as[0]->CAtom().GetParent()->GetCellToCartesian());
    mat3d c2f_t( mat3d::Transpose(as[0]->CAtom().GetParent()->GetCellToCartesian()) );
    for( size_t i=0; i < m.Count(); i++ )
      m[i] = c2f_t*m[i]*c2f;
  }
  // helper functions, matrices are in fractional frame
  template <class atom_list> void GetVcoVF(const atom_list& as, mat3d_list& m)  {
    vcov.FindVcoV(as, m);
    ProcessSymmetry(as, m);
  }
protected:
  struct CellEsd {
    VcoVContainer& base;
    vec3d_alist &points, fcrds;
    template <class Evaluator> struct Functor  {
      CellEsd& base;
      const Evaluator& eval;
      Functor(CellEsd& _base, const Evaluator& e) : base(_base), eval(e)  {}
      double calc()  {
        base.base.Orthogonalise(base.fcrds, base.points);
        return eval.calc();
      }
    };
    CellEsd(VcoVContainer& _base, vec3d_alist& _ps) : base(_base), points(_ps)  {}
    template <class Evaluator> double DoCalc(const Evaluator& e)  {
      evecd df(6);
      fcrds = base.Fractionalise(points);
      Functor<Evaluator> f(*this, e);
      base.CalcDiff(base.cell, df, f);
      return (base.celle*df).DotProd(df);
    }
  };
  vec3d_alist Fractionalise(const vec3d_alist& l) const {
    vec3d_alist rv(l.Count());
    for( size_t i=0; i < l.Count(); i++ )
      rv[i] = au.Fractionalise(l[i]);
    return rv;
  }
  vec3d_alist& Orthogonalise(const vec3d_alist& l, vec3d_alist& rv) const {
    const double
      cA = cos(cell[3]),
      cB = cos(cell[4]),
      cG = cos(cell[5]), sG = sin(cell[5]),
      m[5] = {
        cell[1]*cG, cell[2]*cB, cell[1]*sG, -cell[2]*(cB*cG-cA)/sG,
        cell[2]*sqrt(1-cA*cA-cB*cB-cG*cG+2*cA*cB*cG)/sG
      };
    for( size_t i=0; i < l.Count(); i++ )  {
      rv[i][0] = l[i][0]*cell[0] + l[i][1]*m[0] + l[i][2]*m[1];
      rv[i][1] = l[i][1]*m[2] + l[i][2]*m[3];
      rv[i][2] = l[i][2]*m[4];
    }
    return rv;
  }
  struct PlaneInfo  {
    double rmsd, d;
    vec3d center, normal;
  };
  template <class VT, class WT>
  static PlaneInfo CalcPlane(const VT& points, const WT& weights, short type=0)  {
    mat3d m, vecs;
    double mass = 0, qmass = 0;
    PlaneInfo rv;
    for( size_t i=0; i < points.Count(); i++ )  {
      rv.center += points[i]*weights[i];
      mass += weights[i];
      qmass += olx_sqr(weights[i]);
    }
    rv.center /= mass;
    for( size_t i=0; i < points.Count(); i++ )  {
      const vec3d t = (points[i] - rv.center)*weights[i];
      m[0][0] += (t[0]*t[0]);
      m[0][1] += (t[0]*t[1]);
      m[0][2] += (t[0]*t[2]);
      m[1][1] += (t[1]*t[1]);
      m[1][2] += (t[1]*t[2]);
      m[2][2] += (t[2]*t[2]);
    } // equ: d = s[0]*x + s[1]*y + s[2]*z
    m[1][0] = m[0][1];
    m[2][0] = m[0][2];
    m[2][1] = m[1][2];
    mat3d::EigenValues(m /= qmass, vecs.I());
    bool swaps = true;
    while( swaps )  {
      swaps = false;
      for( short i=0; i < 2; i++ )  {
        if( m[i][i] > m[i+1][i+1] )  {
          olx_swap(vecs[i], vecs[i+1]);
          olx_swap(m[i][i], m[i+1][i+1]);
          swaps = true;
        }
      }
    }
    rv.d = vecs[type].DotProd(rv.center)/vecs[type].Length();
    rv.normal = vecs[type].Normalise();
    rv.rmsd = (m[type][type] <= 0 ? 0 : sqrt(m[type][type]));
    return rv;
  }
  // plane to plane shift distance
  template <class pT, class cT> struct Centroid2CentriodShiftDistance  {
    const pT& p;
    const cT& c;
    Centroid2CentriodShiftDistance(const pT& _p, const cT& _c) : p(_p), c(_c)  {}
    double calc() const {
      const PlaneInfo pi = p.evaluate();
      const vec3d pc = c.evaluate();
      const double pcd = pc.DotProd(pi.normal) - pi.d;
      const double res = pc.QDistanceTo(pi.center) - pcd*pcd;
      return  res <= 0 ? 0 : sqrt(res); 
    }
  };
  // alignment RMSD
  double _calcAllignmentRMSD(const vec3d_alist& points)  {
    align::ListsToPairAdaptor<vec3d_alist, TDoubleList> l2p(points, weights[0]);
    align::out ao = align::FindAlignmentQuaternions(l2p);
    return align::CalcRMSD(l2p, ao);
  }
  // plane to atom distance
  template <class planeT, class pointT> struct PlaneToPointDistance  {
    const planeT& plane;
    const pointT& point;
    PlaneToPointDistance(const planeT& _plane, const pointT& _point) : plane(_plane), point(_point)  {}
    double calc() const {
      const PlaneInfo pi = plane.evaluate();
      return point.evaluate().DotProd(pi.normal) - pi.d;
    }
  };
  template <class VT, class WT> struct PlaneEvaluator  {
    const VT& values;
    const WT& weights;
    PlaneEvaluator(const VT& _v, const WT& _w) : values(_v), weights(_w)  {}
    PlaneInfo evaluate() const {  return CalcPlane(values, weights);  }
  };
  // plane centroid to plane centroid distance
  template <class VT, class WT> struct PointEvaluator  {
    const VT& values;
    const WT& weights;
    PointEvaluator(const VT& _v, const WT& _w) : values(_v), weights(_w)  {}
    vec3d evaluate() const {
      double weight = 0;
      vec3d point ;
      for( size_t i=0; i < values.Count(); i++ )  {
        point += values[i];
        weight += weights[i];
      }
      return (point /= weight);
    }
  };
  struct PointProxy  {
    vec3d& point;
    PointProxy(vec3d& v) : point(v)  {}
    const vec3d& evaluate() const {  return point;  }
  };
  struct VectorEvaluator  {
    const vec3d &a, &b;
    VectorEvaluator(const vec3d& _a, const vec3d& _b) : a(_a), b(_b)  {}
    vec3d evaluate() const {  return b-a;  }
  };
  template <class VT, class WT> struct NormalEvaluator  {
    const VT& values;
    const WT& weights;
    NormalEvaluator(const VT& _v, const WT& _w) : values(_v), weights(_w)  {}
    vec3d evaluate() const {  return CalcPlane(values, weights).normal;  }
  };
  template <class VT, class WT> struct Plane  {
    const VT& values;
    const WT& weights;
    Plane(const VT& _v, const WT& _w) : values(_v), weights(_w)  {}
    double calc() const {  return CalcPlane(values, weights).rmsd;  }
  };
  // torsion angle
  template <class aT, class bT, class cT, class dT> struct TorsionAngle  {
    const aT& a;
    const bT& b;
    const cT& c;
    const dT& d;
    TorsionAngle(const aT& _a, const bT& _b, const cT& _c, const dT& _d) : a(_a), b(_b), c(_c), d(_d)  {}
    double calc() const {
      return olx_dihedral_angle_signed(a.evaluate(), b.evaluate(), c.evaluate(), d.evaluate());
    }
  };
  // tetrahedron volume
  template <class aT, class bT, class cT, class dT> struct TetrahedronVolume  {
    const aT& a;
    const bT& b;
    const cT& c;
    const dT& d;
    TetrahedronVolume(const aT& _a, const bT& _b, const cT& _c, const dT& _d) : a(_a), b(_b), c(_c), d(_d)  {}
    double calc() const {
      return olx_tetrahedron_volume(a.evaluate(), b.evaluate(), c.evaluate(), d.evaluate());
    }
  };
  // angle between 3 points
  template <class aT, class bT, class cT> struct Angle3  {
    const aT& a;
    const bT& b;
    const cT& c;
    Angle3(const aT& _a, const bT& _b, const cT& _c) : a(_a), b(_b), c(_c)  {}
    double calc() const {
      const vec3d c2 = b.evaluate();
      const double ca = (a.evaluate()-c2).CAngle(c.evaluate()-c2);
      if( olx_abs(ca) >= 1.0-1e-16 )
        return ca < 0 ? 180.0 : 0.0;
      return acos(ca)*180.0/M_PI;
    }
  };
  // angle between 2 vectors
  template <class aT, class bT> struct Angle2  {
    const aT& a;
    const bT& b;
    Angle2(const aT& _a, const bT& _b) : a(_a), b(_b)  {}
    double calc() const {
      const double ca = a.evaluate().CAngle(b.evaluate());
      if( olx_abs(ca) >= 1.0-1e-16 )
        return ca < 0 ? 180.0 : 0.0;
      return acos(ca)*180.0/M_PI;
    }
  };
  // point centroid to point distance
  template <class aT, class bT> struct Distance  {
    const aT& a;
    const bT& b;
    Distance(const aT& _a, const bT& _b) : a(_a), b(_b)  {}
    double calc() const {  return a.evaluate().DistanceTo(b.evaluate());  }
  };
  // octahedral distortion (in degrees) using best line approach
  struct OctahedralDistortionBL  {
    const vec3d_alist& points;
    OctahedralDistortionBL(const vec3d_alist& _points) : points(_points)  {}
    double calc() const {
      // centroid for first face
      const vec3d c1 = (points[1] + points[3] + points[5])/3;
      // centroid for second face
      const vec3d c2 = (points[2] + points[4] + points[6])/3;
      vec3d_alist p1(3);
      TDoubleList weights(3);
      for( size_t i=0; i < weights.Count(); i++ )
        weights[i] = 1.0;
      p1[0] = c1;  p1[1] = points[0];  p1[2] = c2;
      const PlaneInfo pi = CalcPlane(p1, weights, 2);
      double sum = 0;
      for( short i=0; i < 3; i++ )  {
        vec3d v1 = points[i*2+1] - pi.center;
        vec3d v2 = points[i*2+2] - pi.center;
        v1 = v1 - pi.normal*v1.DotProd(pi.normal);
        v2 = v2 - pi.normal*v2.DotProd(pi.normal);
        sum += olx_abs(M_PI/3-acos(v1.CAngle(v2)));
      }
      return (sum*180/3)/M_PI;
    }
  };
  // octahedral distortion (in degrees), using best plane approach
  struct OctahedralDistortionBP  {
    const vec3d_alist& points;
    OctahedralDistortionBP(const vec3d_alist& _points) : points(_points)  {}
    double calc() const {
      // translation for first face
      const vec3d c1 = (points[1] + points[3] + points[5])/3;
      // translation for second face
      const vec3d c2 = (points[2] + points[4] + points[6])/3;
      vec3d_alist p1(6);
      TDoubleList weights(6);
      for( short i=0; i < 3; i++ )  {
        weights[i*2] = 1.0;  
        weights[i*2+1] = 1.0;
        p1[i*2] = (points[i*2+1] - c1).Normalise();
        p1[i*2+1] = (points[i*2+2] - c2).Normalise();
      }
      const PlaneInfo pi = CalcPlane(p1, weights, 0);
      double sum = 0;
      for( short i=0; i < 3; i++ )  {
        const vec3d v1 = p1[i*2] - pi.normal*p1[i*2].DotProd(pi.normal);
        const vec3d v2 = p1[i*2+1] - pi.normal*p1[i*2+1].DotProd(pi.normal);
        sum += olx_abs(M_PI/3-acos(v1.CAngle(v2)));
      }
      return (sum*180/3)/M_PI;
    }
  };
  // helper functions
  template <class VC> double CalcEsd(const size_t sz, const mat3d_list& m, const VC& df)  {
    double esd = 0;
    for( size_t i=0; i < sz; i++ )  {
      for( size_t j=0; j < sz; j++ )  {
        const size_t m_ind = i*sz+j;
        for( short k=0; k < 3; k++ )  {
          for( short l=0; l < 3; l++ )  {
            esd += m[m_ind].Get(k,l)*df[i*3+k]*df[j*3+l];
          }
        }
      }
    }
    return esd < 0 ? 0 : esd;
  }
  // helper functions
  template <class list> void AtomsToPoints(const list& atoms, vec3d_alist& r)  {
    r.SetCount(atoms.Count());
    for( size_t i=0; i < atoms.Count(); i++ )
      r[i] = atoms[i]->crd();
  }
  //http://en.wikipedia.org/wiki/Numerical_differentiation
  template <class VecT, class OutVecT, class Evaluator> 
  void CalcDiff(VecT& vars, OutVecT& df, Evaluator& e)  {
    static const double delta=sqrt(2.2e-16);
    for( size_t i=0; i < vars.Count(); i++ )  {
      vars[i] += 2*delta;
      const double v1 = e.calc();
      vars[i] -= delta;
      const double v2 = e.calc();
      vars[i] -= 2*delta;
      const double v3 = e.calc();
      vars[i] -= delta;
      const double v4 = e.calc();
      df[i] = (-v1+8*v2-8*v3+v4)/(12*delta);
      vars[i] += 2*delta;
    }
  }
  struct CalcHelper  {
    mat3d_list m;
    vec3d_alist points;
    VcoVContainer& base;
    template <class list> CalcHelper(VcoVContainer& _base, const list& atoms) :
      base(_base), points(atoms.Count())
    {
      base.GetVcoV(atoms, m);
      base.AtomsToPoints(atoms, points);
    }
    template <class Eval> TEValue<double> DoCalc(Eval& e)  {
      return base.DoCalcForPoints(points, m, e);
    }
  };
  struct CalcWHelper  {
    mat3d_list m;
    vec3d_alist points;
    TDoubleList weights;
    VcoVContainer& base;
    template <class list> CalcWHelper(VcoVContainer& _base, const list& atms) :
      base(_base), points(atms.Count()), weights(atms.Count())
    {
      base.GetVcoV(atms, m);
      base.AtomsToPoints(atms, points);
      for( size_t i=0; i < weights.Count(); i++ )
        weights[i] = 1.0;
    }
    template <class Eval> TEValue<double> DoCalc(Eval& e)  {
      return base.DoCalcForPoints(points, m, e);
    }
  };
  template <class list, typename eval> 
  TEValue<double> DoCalcForPoints(list& points, const mat3d_list& vcov, eval& e)  {
    TDoubleList df(points.Count()*3);
    CalcDiff(CompositeVector<list, double>(points), df, e);
    CellEsd ced(*this, points);
    return TEValue<double>(e.calc(),
      sqrt(CalcEsd(points.Count(), vcov, df) + ced.DoCalc(e)));
  }
public:
  VcoVContainer(TAsymmUnit& _au) : au(_au), cell(6), celle(6)  {
    static const double a2r = M_PI/180;
    cell[0] = au.GetAxes()[0].GetV();  celle[0] = olx_sqr(au.GetAxes()[0].GetE());
    cell[1] = au.GetAxes()[1].GetV();  celle[1] = olx_sqr(au.GetAxes()[1].GetE());
    cell[2] = au.GetAxes()[2].GetV();  celle[2] = olx_sqr(au.GetAxes()[2].GetE());
    cell[3] = au.GetAngles()[0].GetV()*a2r;  celle[3] = olx_sqr(au.GetAngles()[0].GetE()*a2r);
    cell[4] = au.GetAngles()[1].GetV()*a2r;  celle[4] = olx_sqr(au.GetAngles()[1].GetE()*a2r);
    cell[5] = au.GetAngles()[2].GetV()*a2r;  celle[5] = olx_sqr(au.GetAngles()[2].GetE()*a2r);
  }
  void ReadShelxMat(const olxstr& fileName) {  vcov.ReadShelxMat(fileName, au);  }
  void ReadSmtbxMat(const olxstr& fileName) {  vcov.ReadSmtbxMat(fileName, au);  }
  // precise calculation
  TEValue<double> CalcDistance(const TSAtom& a1, const TSAtom& a2) {
    mat3d_list m;
    TSAtom const* as[] = {&a1,&a2};
    TSAtomPList satoms(2, as);
    vec3d_alist pl(2);
    GetVcoV(satoms, m);
    AtomsToPoints(satoms, pl);
    mat3d vcov = m[0] - m[1] - m[2] + m[3];
    vec3d v = a1.crd() - a2.crd();
    const double val = v.Length();
    double qesd = (vcov*v).DotProd(v)/(val*val);
    typedef PointProxy pe_t;
    qesd += CellEsd(*this, pl).DoCalc(Distance<pe_t,pe_t>(pe_t(pl[0]), pe_t(pl[1])));
    return TEValue<double>(val, sqrt(qesd));
  }
  // cartesian centroid
  TEVPoint<double> CalcCentroid(const TSAtomPList& atoms) {
    mat3d_list m;
    GetVcoV(atoms, m);
    mat3d vcov;
    vec3d center;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      center += atoms[i]->crd();
      for( size_t j=0; j < atoms.Count(); j++ )
        vcov += m[i*atoms.Count()+j]; 
    }
    vcov *= 1./(atoms.Count()*atoms.Count());
    center /= atoms.Count();
    return TEVPoint<double>(center[0], center[1], center[2], sqrt(vcov[0][0]), sqrt(vcov[1][1]), sqrt(vcov[2][2]));
  }
  // fractional centroid
  TEVPoint<double> CalcCentroidF(const TSAtomPList& atoms) {
    mat3d_list m;
    GetVcoVF(atoms, m);
    mat3d vcov;
    vec3d center;
    for( size_t i=0; i < atoms.Count(); i++ )  {
      center += atoms[i]->ccrd();
      for( size_t j=0; j < atoms.Count(); j++ )
        vcov += m[i*atoms.Count()+j]; 
    }
    vcov *= 1./(atoms.Count()*atoms.Count());
    center /= atoms.Count();
    return TEVPoint<double>(center[0], center[1], center[2], sqrt(vcov[0][0]), sqrt(vcov[1][1]), sqrt(vcov[2][2]));
  }
  // precise calculation
  TEValue<double> CalcPC2ADistanceP(const TSAtomPList& cent, const TSAtom& a) {
    mat3d_list m;
    TSAtomPList satoms(cent);
    satoms.Add(const_cast<TSAtom*>(&a));
    GetVcoV(satoms, m);
    mat3d vcov, nvcov;
    vec3d center;
    // var(atom(i)), cov(atom(i),atom(j))
    for( size_t i=0; i < cent.Count(); i++ )  {
      center += cent[i]->crd();
      for( size_t j=0; j < cent.Count(); j++ )
        vcov += m[i*satoms.Count()+j]; 
    }
    vcov *= 1./(cent.Count()*cent.Count());
    center /= cent.Count();
    center -= a.crd();
    // cov( atom(i), a)
    for( size_t i=0; i < cent.Count(); i++ )  {
      nvcov += m[i*satoms.Count()+cent.Count()];
      nvcov += m[m.Count()-cent.Count()-2+i];
    }
    nvcov *= 1./cent.Count();
    vcov -= nvcov;
    // var(a,a)
    vcov += m.GetLast();
    double val = center.Length();
    double esd = sqrt((vcov*center).DotProd(center))/val;
    return TEValue<double>(val, esd);
  }
  // analytical, http://salilab.org/modeller/manual/node449.html#SECTION001331200000000000000 
  TEValue<double> CalcAngleA(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3) {
    mat3d_list m;
    TSAtom const * as[] = {&a1,&a2,&a3};
    TSAtomPList satoms(3, as);
    vec3d_alist pl(3);
    GetVcoV(satoms, m);
    AtomsToPoints(satoms, pl);
    vec3d ij = (satoms[0]->crd() - satoms[1]->crd());
    vec3d kj = (satoms[2]->crd() - satoms[1]->crd());
    vec3d_alist grad(3);
    const double ca = ij.CAngle(kj);
    if( olx_abs(ca) >= 1.0-1e-16 )
      return TEValue<double>(ca < 0 ? 180.0 : 0.0, 0);
    const double oos = 1./sqrt(1-ca*ca);
    const double ij_l = ij.Length(), kj_l = kj.Length();
    ij.Normalise();
    kj.Normalise();
    grad[0] = (ij*ca - kj)*oos/ij_l;
    grad[2] = (kj*ca - ij)*oos/kj_l;
    grad[1] = -(grad[0] + grad[2]);
    double qesd = CalcEsd(3, m, CompositeVector<vec3d_alist, double>(grad));
    typedef PointProxy pe_t;
    qesd += CellEsd(*this, pl).DoCalc(Angle3<pe_t,pe_t,pe_t>(pe_t(pl[0]), pe_t(pl[1]), pe_t(pl[2])))/olx_sqr(180/M_PI);
    const double a = acos(ca);
    return TEValue<double>(a,(qesd < 1e-15 ? 0 : sqrt(qesd)))*=180/M_PI;
  }
  TEValue<double> CalcAngle(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3) {
    TSAtom const * as[] = {&a1,&a2,&a3};
    TSAtomPList satoms(3, as);
    CalcHelper ch(*this, satoms);
    return ch.DoCalc(
      Angle3<PointProxy,PointProxy,PointProxy>(
        PointProxy(ch.points[0]),
        PointProxy(ch.points[1]),
        PointProxy(ch.points[2])));
  }
  // torsion angle, signless, precise esd, Acta A30, 848, Uri Shmuelli
  TEValue<double> CalcTAngleP(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    mat3d_list m;
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    TSAtomPList satoms(4, as);
    vec3d_alist pl(4);
    GetVcoV(satoms, m);
    AtomsToPoints(satoms, pl);
    vec3d u(a2.crd()-a1.crd()),
      v(a3.crd()-a2.crd()),
      w(a4.crd()-a3.crd());
    double h11 = u.QLength(),
      h22 = v.QLength(),
      h33 = w.QLength(),
      h12 = u.DotProd(v),
      h13 = u.DotProd(w),
      h23 = v.DotProd(w);
    double A = h12*h23 - h13*h22;
    double B = h11*h22 - h12*h12;
    double C = h22*h33 - h23*h23;
    const double ctau = A/sqrt(B*C);
    if( olx_abs(ctau) >= 1.0-1e-16 )
      return TEValue<double>(ctau < 0 ? 180.0 : 0.0, 0);
    double tau = acos(ctau);
    double K = -1.0/(sqrt(B*C)*sin(tau)), esd = 0;
    double dt[3][3];
    for( short i=0; i < 3; i++ )  {
      dt[i][0] = K*(v[i]*h23-w[i]*h22 - A/(2*B)*2*(u[i]*h22-v[i]*h12));
      dt[i][1] = K*(u[i]*h23+w[i]*h12-2*v[i]*h13 - A/(2*B)*2*(v[i]*h11-u[i]*h12) -A/(2*C)*2*(v[i]*h33-w[i]*h23));
      dt[i][2] = K*(v[i]*h12-u[i]*h22 - A/(2*C)*2*(w[i]*h22-v[i]*h23));
    }
    double dtx[4][3];
    for( short i=0; i < 3; i++ )  {
      dtx[0][i] = -dt[0][i];
      dtx[1][i] = dt[0][i]-dt[1][i];
      dtx[2][i] = dt[1][i]-dt[2][i];
      dtx[3][i] = dt[2][i];
    }
    for( short k=0; k < 4; k++ )  {
      for( short n=0; n < 4; n++ )  {
        for( short s=0; s < 3; s++ )  {
          for( short q=0; q < 3; q++ )  {
            esd += dtx[k][s]*dtx[n][q]*m[k*4+n][s][q];
          }
        }
      }
    }
    typedef PointProxy pe_t;
    esd += CellEsd(*this, pl).DoCalc(
      TorsionAngle<pe_t,pe_t,pe_t,pe_t>(pe_t(pl[0]), pe_t(pl[1]), pe_t(pl[2]), pe_t(pl[3])))/olx_sqr(180.0/M_PI);
    return TEValue<double>(tau, sqrt(esd))*=180.0/M_PI;
  }
  // analytical, http://salilab.org/modeller/manual/node449.html#SECTION001331200000000000000
  TEValue<double> CalcTAngleA(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    mat3d_list m;
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    TSAtomPList satoms(4, as);
    vec3d_alist pl(4);
    AtomsToPoints(satoms, pl);
    GetVcoV(satoms, m);
    const vec3d ij = a1.crd() - a2.crd();
    const vec3d kj = a3.crd() - a2.crd();
    const vec3d kl = a4.crd() - a3.crd();
    const vec3d mj = ij.XProdVec(kj);
    const vec3d nk = kj.XProdVec(kl);
    const double kj_ql = kj.QLength();
    const double kj_l = sqrt(kj_ql);
    vec3d_alist grad(4);
    grad[0] = mj*(kj_l/mj.QLength());
    grad[3] = -(nk*(kj_l/nk.QLength()));
    grad[1] = grad[0]*(ij.DotProd(kj)/kj_ql -1.0) - grad[3]*(kl.DotProd(kj)/kj_ql);
    grad[2] = grad[3]*(kl.DotProd(kj)/kj_ql -1.0) - grad[0]*(ij.DotProd(kj)/kj_ql);
    double esd = CalcEsd(4, m, CompositeVector<vec3d_alist, double>(grad));
    typedef PointProxy pe_t;
    esd += CellEsd(*this, pl).DoCalc(
      TorsionAngle<pe_t,pe_t,pe_t,pe_t>(pe_t(pl[0]), pe_t(pl[1]), pe_t(pl[2]), pe_t(pl[3])))/olx_sqr(180.0/M_PI);
    const double a = olx_dihedral_angle_signed(a1.crd(), a2.crd(), a3.crd(), a4.crd());
    return TEValue<double>(a, (esd < 1e-15 ? 0 : sqrt(esd))*180/M_PI);
  }
  // torsion angle
  TEValue<double> CalcTAngle(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    mat3d_list m;
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    TSAtomPList satoms(4, as);
    CalcHelper ch(*this, satoms);
    return ch.DoCalc(
      TorsionAngle<PointProxy,PointProxy,PointProxy,PointProxy>(
        PointProxy(ch.points[0]),
        PointProxy(ch.points[1]),
        PointProxy(ch.points[2]),
        PointProxy(ch.points[3])));
  }
  // bond to bond angle
  TEValue<double> CalcB2BAngle(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    mat3d_list m;
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    TSAtomPList satoms(4, as);
    CalcHelper ch(*this, satoms);
    return ch.DoCalc(
      Angle2<VectorEvaluator,VectorEvaluator>(
      VectorEvaluator(ch.points[0], ch.points[1]),
      VectorEvaluator(ch.points[2], ch.points[3])));
  }
  // returns rms for the best plane
  TEValue<double> CalcPlane(const TSAtomPList& atoms) {
    CalcWHelper ch(*this, atoms);
    return ch.DoCalc(Plane<vec3d_alist, TDoubleList>(ch.points, ch.weights));
  }
  // plane to atom distance
  TEValue<double> CalcP2ADistance(const TSAtomPList& atoms, const TSAtom& a) {
    TSAtomPList satoms(atoms);
    satoms.Add(const_cast<TSAtom*>(&a));
    CalcWHelper ch(*this, satoms);
    typedef VectorSlice<vec3d_alist,vec3d> crd_slice;
    return ch.DoCalc(
      PlaneToPointDistance<PlaneEvaluator<crd_slice,TDoubleList>, PointProxy>(
      PlaneEvaluator<crd_slice, TDoubleList>(
        crd_slice(ch.points, 0, atoms.Count()), ch.weights),
        PointProxy(ch.points.GetLast())));
  }
  // plane centroid to atom distance
  TEValue<double> CalcPC2ADistance(const TSAtomPList& plane, const TSAtom& a) {
    TSAtomPList satoms(plane);
    satoms.Add(const_cast<TSAtom*>(&a));
    CalcWHelper ch(*this, satoms);
    typedef VectorSlice<vec3d_alist,vec3d> crd_slice;
    return ch.DoCalc(
      Distance<PointEvaluator<crd_slice,TDoubleList>, PointProxy>(
      PointEvaluator<crd_slice, TDoubleList>(
        crd_slice(ch.points, 0, plane.Count()), ch.weights),
        PointProxy(ch.points.GetLast())));
  }
  // plane to a vector angle
  TEValue<double> CalcP2VAngle(const TSAtomPList& plane, const TSAtom& a1, const TSAtom& a2) {
    TSAtomPList satoms(plane);
    satoms.Add(const_cast<TSAtom*>(&a1));
    satoms.Add(const_cast<TSAtom*>(&a2));
    CalcWHelper ch(*this, satoms);
    typedef VectorSlice<vec3d_alist,vec3d> crd_slice;
    return ch.DoCalc(
      Angle2<NormalEvaluator<crd_slice,TDoubleList>, VectorEvaluator>(
      NormalEvaluator<crd_slice, TDoubleList>(
        crd_slice(ch.points, 0, plane.Count()), ch.weights),
        VectorEvaluator(ch.points[plane.Count()], ch.points[plane.Count()+1])));
  }
  // plane to plane angle
  TEValue<double> CalcP2PAngle(const TSAtomPList& p1, const TSAtomPList& p2) {
    TSAtomPList atoms(p1);
    atoms.AddList(p2);
    CalcWHelper ch(*this, atoms);
    typedef VectorSlice<vec3d_alist,vec3d> crd_slice;
    typedef VectorSlice<TDoubleList, double> weight_slice;
    return ch.DoCalc(
      Angle2<NormalEvaluator<crd_slice,weight_slice>, NormalEvaluator<crd_slice,weight_slice> >(
      NormalEvaluator<crd_slice,weight_slice>(
        crd_slice(ch.points, 0, p1.Count()), weight_slice(ch.weights, 0, p1.Count())),
      NormalEvaluator<crd_slice,weight_slice>(
        crd_slice(ch.points, p1.Count(), p2.Count()), weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  //plane centroid to plane centroid distance
  TEValue<double> CalcPC2PCDistance(const TSAtomPList& p1, const TSAtomPList& p2) {
    TSAtomPList atoms(p1);
    atoms.AddList(p2);
    CalcWHelper ch(*this, atoms);
    typedef VectorSlice<vec3d_alist,vec3d> crd_slice;
    typedef VectorSlice<TDoubleList, double> weight_slice;
    typedef PointEvaluator<crd_slice,weight_slice> point_e;
    return ch.DoCalc(
      Distance<point_e, point_e>(
      point_e(crd_slice(ch.points, 0, p1.Count()), weight_slice(ch.weights, 0, p1.Count())),
      point_e(crd_slice(ch.points, p1.Count(), p2.Count()),
        weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  // angle between 3 plane centroids
  TEValue<double> Calc3PCAngle(const TSAtomPList& p1, 
    const TSAtomPList& p2, const TSAtomPList& p3) 
  {
    TSAtomPList atoms(p1);
    atoms.AddList(p2);
    atoms.AddList(p3);
    CalcWHelper ch(*this, atoms);
    typedef VectorSlice<vec3d_alist,vec3d> crd_slice;
    typedef VectorSlice<TDoubleList, double> weight_slice;
    typedef PointEvaluator<crd_slice,weight_slice> point_e;
    return ch.DoCalc(
      Angle3<point_e, point_e, point_e>(
      point_e(crd_slice(ch.points, 0, p1.Count()), weight_slice(ch.weights, 0, p1.Count())),
      point_e(crd_slice(ch.points, p1.Count(), p2.Count()),
        weight_slice(ch.weights, p1.Count(), p2.Count())),
      point_e(crd_slice(ch.points, p1.Count()+p2.Count(), p3.Count()),
        weight_slice(ch.weights, p1.Count()+p2.Count(), p3.Count()))));
  }
  //plane to another plane centroid distance
  TEValue<double> CalcP2PCDistance(const TSAtomPList& p1, const TSAtomPList& p2) {
    TSAtomPList atoms(p1);
    atoms.AddList(p2);
    CalcWHelper ch(*this, atoms);
    typedef VectorSlice<vec3d_alist,vec3d> crd_slice;
    typedef VectorSlice<TDoubleList, double> weight_slice;
    typedef PointEvaluator<crd_slice,weight_slice> point_e;
    typedef PlaneEvaluator<crd_slice,weight_slice> plane_e;
    return ch.DoCalc(
      PlaneToPointDistance<plane_e, point_e>(
      plane_e(crd_slice(ch.points, 0, p1.Count()), weight_slice(ch.weights, 0, p1.Count())),
      point_e(crd_slice(ch.points, p1.Count(), p2.Count()),
        weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  //plane to another plane shift distance
  TEValue<double> CalcP2PShiftDistance(const TSAtomPList& p1, const TSAtomPList& p2) {
    TSAtomPList atoms(p1);
    atoms.AddList(p2);
    CalcWHelper ch(*this, atoms);
    typedef VectorSlice<vec3d_alist,vec3d> crd_slice;
    typedef VectorSlice<TDoubleList, double> weight_slice;
    typedef PointEvaluator<crd_slice,weight_slice> point_e;
    typedef PlaneEvaluator<crd_slice,weight_slice> plane_e;
    return ch.DoCalc(
      Centroid2CentriodShiftDistance<plane_e, point_e>(
      plane_e(crd_slice(ch.points, 0, p1.Count()), weight_slice(ch.weights, 0, p1.Count())),
      point_e(crd_slice(ch.points, p1.Count(), p2.Count()),
        weight_slice(ch.weights, p1.Count(), p2.Count()))));
  }
  // tetrahedron volume
  TEValue<double> CalcTetrahedronVolume(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    TSAtomPList satoms(4, as);
    CalcHelper ch(*this, satoms);
    typedef PointProxy point_t;
    return ch.DoCalc(
      TetrahedronVolume<point_t,point_t,point_t,point_t>(
      point_t(ch.points[0]), point_t(ch.points[1]),
      point_t(ch.points[2]), point_t(ch.points[3])));
  }
  // alignment RMSD crds should be prepeared, i.e. inverted
  TEValue<double> CalcAlignmentRMSD(const TSAtomPList& atoms, const vec3d_alist& crds, 
    const TDoubleList& _weights) 
  {
    //mat3d_list m;
    //GetVcoV(atoms, m);
    //weights[0] = _weights;
    //return DoCalcForPoints(crds, m, &VcoVContainer::_calcAllignmentRMSD);
    return 0;
  }
  // octahedral distortion, takes {Central Atom, a1, b1, a2, b2, a3, b3}, returns mean value
  TEValue<double> CalcOHDistortionBL(const TSAtomPList& atoms)  {
    //mat3d_list m;
    //return DoCalcForAtoms(atoms, &VcoVContainer::_calcOHDistortionBL);
    return 0;
  }
  TEValue<double> CalcOHDistortionBP(const TSAtomPList& atoms)  {
    //mat3d_list m;
    //return DoCalcForAtoms(atoms, &VcoVContainer::_calcOHDistortionBP);
    return 0;
  }
  const VcoVMatrix& GetMatrix() const {  return vcov;  }
};

EndXlibNamespace()
#endif
