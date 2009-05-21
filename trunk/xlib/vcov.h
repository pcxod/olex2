#ifndef __olxs_v_co_v_h
#define __olxs_v_co_v_h
#include "asymmunit.h"
#include "lattice.h"
#include "ecast.h"
#include "bapp.h"
#include "log.h"
/*
  after several days if differentiation, I came across som stuff, which became to complecated and
  tried to use the numerical differentiation, as all the functions calculated here are "smooth". The
  results have shown that the resultiong esd is very similar to the exact expressions for angles and
  some othe parameters, so here is the way forward, no more complecated fomulas!
*/
BeginXlibNamespace()

const short // constants decribing the stored values
  vcoviX = 0,
  vcoviY = 1,
  vcoviZ = 2,
  vcoviO = 3;

// stores X,Y,Z,SOF for each atom and their correlations
class VcoVMatrix {
  double **data;
  int count;
  // atom label, 
  TTypeList< AnAssociation3<olxstr, short, int> > Index;
protected:
  void Allocate(int w) {
    Clear();
    count = w;
    data = new double*[w];
    for( int i=0; i < w; i++ ) // bottom diagonal agrrangement
      data[i] = new double[i+1];
  }
  int FindAtomIndex(const TCAtom& a) const {
    for( int i=0; i < Index.Count(); i++ )
      if( Index[i].GetC() == a.GetId() )
        return i;
    return -1;
  }
public:
  VcoVMatrix()  {
    data = NULL;
    count = 0;
  }
  ~VcoVMatrix() {  Clear();  }
  void Clear() {
    if( data == NULL )  return;
    Index.Clear();
    for( int i=0; i < count; i++ )
      delete [] data[i];
    delete [] data;
    data = NULL;
  }
  double operator () (int i, int j) const {
    return (j <= i ) ? data[i][j] : data[j][i];
  }
  double Get(int i, int j) const {
    return (j <= i ) ? data[i][j] : data[j][i];
  }
  // reads the shelxl VcoV matrix and initliases atom loader Ids'
  void ReadShelxMat(const olxstr& fileName, TAsymmUnit& au);
  // fills creates three matrices AA, AB, ... AX, BA, BB, ... BX, ...
  template <class list> void FindVcoV(const list& atoms, mat3d_list& m ) const {
    TIntList a_indexes;
    vec3i_list indexes;
    for( int i=0; i < atoms.Count(); i++ )  {
      a_indexes.Add(FindAtomIndex(atoms[i]->CAtom()));
      if( a_indexes.Last() == -1 )
        TBasicApp::GetLog().Error( olxstr("Unable to located provided atom: ") << atoms[i]->GetLabel());
      indexes.AddNew(-1,-1,-1);
    }
    for( int i=0; i < a_indexes.Count(); i++ )  {
      if( a_indexes[i] == -1 )  continue;
      for( int j=a_indexes[i]; j < Index.Count() && Index[j].GetC() == atoms[i]->CAtom().GetId(); j++ )  {
        if( Index[j].GetB() == vcoviX )
          indexes[i][0] = j;
        else if( Index[j].GetB() == vcoviY )
          indexes[i][1] = j;
        else if( Index[j].GetB() == vcoviZ )
          indexes[i][2] = j;
      }
    }
    for( int i=0; i < a_indexes.Count(); i++ )  {
      for( int j=0; j < a_indexes.Count(); j++ )  {
        mat3d& a = m.AddNew();
        for( int k=0; k < 3; k++ )  {
          for( int l=k; l < 3; l++ )  {
            if( indexes[i][k] != -1 && indexes[j][l] != -1 )  {
              a[k][l] = Get(indexes[i][k], indexes[j][l]);
              a[l][k] = a[k][l];
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
  TDoubleList weights1, weights2;
  vec3d plane_param1, plane_param2, plane_center1, plane_center2;
protected:
  template <class atom_list> void ProcessSymmetry(const atom_list& atoms, mat3d_list& ms)  {
    mat3d_list left(atoms.Count()), right(atoms.Count());
    int mc = 0;
    for( int i=0; i < atoms.Count(); i++ )  {
      const mat3d& m = atoms[i]->GetMatrix(0).r;
      if( m.IsI() )  continue;
      left.SetCCopy(i, m); 
      right.SetCCopy(i, m).Transpose();
      mc++;
    }
    if( mc == 0 )  return;
    for( int i=0; i < atoms.Count(); i++ )  {
      for( int j=0; j < atoms.Count(); j++ )  {
        if( !left.IsNull(i) )  {
          int l_ind = i*atoms.Count() + j;
          ms[l_ind] = left[i]*ms[l_ind];
        }
        if( !right.IsNull(i) )  {
          int r_ind = j*atoms.Count() + i;
          ms[r_ind] *= right[i];
        }
      }
    }
  }
  double _calcAngle(const vec3d_alist& points)  {
    return acos( (points[0]-points[1]).CAngle(points[2]-points[1]))*180.0/M_PI;
  }
  double _calcB2BAngle(const vec3d_alist& points)  {
    return acos( (points[1]-points[0]).CAngle(points[3]-points[2]))*180.0/M_PI;
  }
  double _calcTHV(const vec3d_alist& points) const {
    return TetrahedronVolume(points[0], points[1], points[2], points[3]);
  }
  double _calcTang(const vec3d_alist& points)  {
    vec3d u(points[1]-points[0]),
      v(points[2]-points[1]),
      w(points[3]-points[2]);
    double h11 = u.QLength(),
      h22 = v.QLength(),
      h33 = w.QLength(),
      h12 = u.DotProd(v),
      h13 = u.DotProd(w),
      h23 = v.DotProd(w);
    double A = h12*h23 - h13*h22;
    double B = h11*h22 - h12*h12;
    double C = h22*h33 - h23*h23;
    return acos(A/sqrt(B*C))*180.0/M_PI;
  }
  template <int k> double _calcPlane(const vec3d_alist& Points) {
    mat3d m, vecs;
    vec3d t;
    double mass = 0;
    TDoubleList& weights = (k == 1 ? weights1 : weights2);
    vec3d& pp = (k == 1 ? plane_param1 : plane_param2);
    vec3d& plane_center = (k == 1 ? plane_center1 : plane_center2);
    plane_center.Null();
    for( int i=0; i < Points.Count(); i++ )  {
      plane_center += Points[i]*weights[i];
      mass += weights[i];
    }
    plane_center /= mass;

    for( int i=0; i < Points.Count(); i++ )  {
      vec3d t( Points[i] - plane_center );
      const double wght = weights[i]*weights[i];
      m[0][0] += (t[0]*t[0]*wght);
      m[0][1] += (t[0]*t[1]*wght);
      m[0][2] += (t[0]*t[2]*wght);

      m[1][1] += (t[1]*t[1]*wght);
      m[1][2] += (t[1]*t[2]*wght);

      m[2][2] += t[2]*t[2]*wght;
    } // equ: d = s[0]*x + s[1]*y + s[2]*z
    m[1][0] = m[0][1];
    m[2][0] = m[0][2];
    m[2][1] = m[1][2];
    mat3d::EigenValues(m, vecs.I());
    if( m[0][0] < m[1][1] )  {
      if( m[0][0] < m[2][2] )  {  
        pp = vecs[0];
        return sqrt(m[0][0]/Points.Count());
      }
      else  {
        pp = vecs[2];
        return sqrt(m[2][2]/Points.Count());
      }
    }
    else  {
      if( m[1][1] < m[2][2] )  {
        pp = vecs[1];
        return sqrt(m[1][1]/Points.Count());
      }
      else  {
        pp = vecs[2];
        return sqrt(m[2][2]/Points.Count());
      }
    }
  }
  // plane to plane angle in degrees
  double _calcP2PAngle(const vec3d_alist& points, int fpc)  {
    vec3d_alist p1(fpc), p2(points.Count()-fpc);
    for( int i=0; i < points.Count(); i++ )  {
      if( i < fpc )
        p1[i] = points[i];
      else
        p2[i-fpc] = points[i];
    }
    _calcPlane<1>(p1);
    _calcPlane<2>(p2);
    return acos(plane_param1.CAngle(plane_param2))*180.0/M_PI;
  }
  // plane to plane centroid distance
  double _calcP2PCDistance(const vec3d_alist& points, int fpc)  {
    vec3d_alist p1(fpc);
    vec3d crd;
    for( int i=0; i < points.Count(); i++ )  {
      if( i < fpc )
        p1[i] = points[i];
      else
        crd += points[i];
    }
    crd /= (points.Count()-fpc);
    _calcPlane<1>(p1);
    double d = plane_param1.DotProd(plane_center1)/plane_param1.Length();
    plane_param1.Normalise();
    return crd.DotProd(plane_param1) - d;
  }
  // alignment RMSD
  double _calcAllignmentRMSD(const vec3d_alist& points)  {
    ematd evm(4,4), quaternions(4,4);
    vec3d cntA, cntB;
    const int ac = points.Count()/2;
    double wghta = 0, wghtb = 0;
    for( int i=0; i < ac; i++ )  {
      cntA += points[i]*weights1[i];
      cntB += points[ac+i]*weights1[ac+i];
      wghta += weights1[i];
      wghtb += weights1[ac+i];
    }
    cntA /= wghta;
    cntB /= wghtb;

    for( int i=0; i < ac; i++ )  {
      const vec3d v = points[i] - cntA;
      const double 
        xm = v[0] - (points[ac+i][0]-cntB[0]),
        xp = v[0] + (points[ac+i][0]-cntB[0]),
        yp = v[1] + (points[ac+i][1]-cntB[1]),
        ym = v[1] - (points[ac+i][1]-cntB[1]),
        zm = v[2] - (points[ac+i][2]-cntB[2]),
        zp = v[2] + (points[ac+i][2]-cntB[2]);
      evm[0][0] += (xm*xm + ym*ym + zm*zm);
      evm[0][1] += (yp*zm - ym*zp);
      evm[0][2] += (xm*zp - xp*zm);
      evm[0][3] += (xp*ym - xm*yp);
      evm[1][0] = evm[0][1];
      evm[1][1] += (yp*yp + zp*zp + xm*xm);
      evm[1][2] += (xm*ym - xp*yp);
      evm[1][3] += (xm*zm - xp*zp);
      evm[2][0] = evm[0][2];
      evm[2][1] = evm[1][2];
      evm[2][2] += (xp*xp + zp*zp + ym*ym);
      evm[2][3] += (ym*zm - yp*zp);
      evm[3][0] = evm[0][3];
      evm[3][1] = evm[1][3];
      evm[3][2] = evm[2][3];
      evm[3][3] += (xp*xp + yp*yp + zm*zm);
    }
    ematd::EigenValues(evm, quaternions.I());
    double rms = 1e5;
    for( int i=0; i < 4; i++ )
      if( evm[i][i] < rms )
        rms = evm[i][i];
    return rms < 0 ? 0 : sqrt(rms/ac);
  }
  // plane to bond angle in degrees
  double _calcP2BAngle(const vec3d_alist& points)  {
    vec3d_alist p1(points.Count()-2);
    for( int i=0; i < points.Count()-2; i++ ) 
      p1[i] = points[i];
    _calcPlane<1>(p1);
    vec3d v(points.Last() - points[points.Count()-2]);
    return acos(plane_param1.CAngle(v))*180.0/M_PI;
  }
  // plane to atom distance
  double _calcP2ADistance(const vec3d_alist& points)  {
    vec3d_alist p1(points.Count()-1);
    for( int i=0; i < points.Count()-1; i++ )
      p1[i] = points[i];
    _calcPlane<1>(p1);
    double d = plane_param1.DotProd(plane_center1)/plane_param1.Length();
    plane_param1.Normalise();
    return points.Last().DotProd(plane_param1) - d;
  }
  // plane centroid to plane centroid distance
  double _calcPC2PCDistance(const vec3d_alist& points, int fpc)  {
    vec3d c1, c2;
    for( int i=0; i < points.Count(); i++ )  {
      if( i < fpc )
        c1 += points[i];
      else
        c2 += points[i];
    }
    c1 /= fpc;
    c2 /= (points.Count()-fpc);
    return c1.DistanceTo(c2);
  }
  // plane centroid to atom distance
  double _calcPC2ADistance(const vec3d_alist& points)  {
    vec3d c1;
    for( int i=0; i < points.Count()-1; i++ ) 
      c1 += points[i];
    c1 /= (points.Count()-1);
    return c1.DistanceTo(points.Last());
  }
  // helper functions
  double CalcEsd(const int sz, const mat3d_list& m, const TDoubleList& df)  {
    double esd = 0;
    for( int i=0; i < sz; i++ )  {
      for( int j=0; j < sz; j++ )  {
        const int m_ind = i*sz+j;
        for( int k=0; k < 3; k++ )  {
          for( int l=0; l < 3; l++ )  {
            esd += m[m_ind][k][l]*df[i*3+k]*df[j*3+l];
          }
        }
      }
    }
    return esd;
  }
  // helper functions, matrices are in cartesian frame
  template <class atom_list> void GetVcoV(const atom_list& as, mat3d_list& m)  {
    vcov.FindVcoV(as, m);
    ProcessSymmetry(as, m);
    mat3d c2f( as[0]->CAtom().GetParent()->GetCellToCartesian() );
    mat3d c2f_t( mat3d::Transpose(as[0]->CAtom().GetParent()->GetCellToCartesian()) );
    for( int i=0; i < m.Count(); i++ )
      m[i] = c2f_t*m[i]*c2f;
  }
  // helper functions, matrices are in fractional frame
  template <class atom_list> void GetVcoVF(const atom_list& as, mat3d_list& m)  {
    vcov.FindVcoV(as, m);
    ProcessSymmetry(as, m);
  }
  // helper functions
  template <class list> void AtomsToPoints(const list& atoms, vec3d_alist& r)  {
    r.SetCount(atoms.Count());
    for( int i=0; i < atoms.Count(); i++ )
      r[i] = atoms[i]->crd();
  }
  template <class List, class Evaluator> 
  void CalcDiff(const List& points, TDoubleList& df, Evaluator e)  {
    const double delta=1.0e-10;
    for( int i=0; i < points.Count(); i++ )  {
      for( int j=0; j < 3; j++ )  {
        points[i][j] += 2*delta;
        double v1 = (this->*e)(points);
        points[i][j] -= delta;
        double v2 = (this->*e)(points);
        points[i][j] -= 2*delta;
        double v3 = (this->*e)(points);
        points[i][j] -= delta;
        double v4 = (this->*e)(points);
        df[i*3+j] = (-v1+8*v2-8*v3+v4)/(12*delta);
        points[i][j] += 2*delta;
      }
    }
  }
  template <class List, typename Evaluator, class extraParam> 
  void CalcDiff(const List& points, TDoubleList& df, Evaluator e, const extraParam& ep)  {
    const double delta=1.0e-10;
    for( int i=0; i < points.Count(); i++ )  {
      for( int j=0; j < 3; j++ )  {
        points[i][j] += 2*delta;
        double v1 = (this->*e)(points, ep);
        points[i][j] -= delta;
        double v2 = (this->*e)(points, ep);
        points[i][j] -= 2*delta;
        double v3 = (this->*e)(points, ep);
        points[i][j] -= delta;
        double v4 = (this->*e)(points, ep);
        df[i*3+j] = (-v1+8*v2-8*v3+v4)/(12*delta);
        points[i][j] += 2*delta;
      }
    }
  }
  template <class list, typename eval> 
  TEValue<double> DoCalcForAtoms(const list& atoms, eval e)  {
    mat3d_list m;
    GetVcoV(atoms, m);
    vec3d_alist points(atoms.Count());
    AtomsToPoints(atoms, points);
    return DoCalcForPoints(points, m, e);
  }
  template <class list, typename eval> 
  TEValue<double> DoCalcForPoints(const list& points, const mat3d_list& vcov, eval e)  {
    TDoubleList df(points.Count()*3);
    CalcDiff(points, df, e);
    return TEValue<double>((this->*e)(points),sqrt(CalcEsd(points.Count(), vcov, df)));
  }
  template <class list, typename eval, class extraParam> 
  TEValue<double> DoCalcExForAtoms(const list& atoms, eval e, const extraParam& ep)  {
    mat3d_list m;
    GetVcoV(atoms, m);
    vec3d_alist points(atoms.Count());
    AtomsToPoints(atoms, points);
    return DoCalcExForPoints(points, m, e, ep);
  }
  template <class list, typename eval, class extraParam> 
  TEValue<double> DoCalcExForPoints(const list& points, const mat3d_list& vcov, 
    eval e, const extraParam& ep)  
  {
    TDoubleList df(points.Count()*3);
    CalcDiff(points, df, e, ep);
    return TEValue<double>((this->*e)(points, ep),sqrt(CalcEsd(points.Count(), vcov, df)));
  }
public:
  VcoVContainer()  {  }
  void ReadShelxMat(const olxstr& fileName, TAsymmUnit& au) {
    vcov.ReadShelxMat(fileName, au);
  }
  // precise calculation
  TEValue<double> CalcDistance(const TSAtom& a1, const TSAtom& a2) {
    mat3d_list m;
    TSAtom const* as[] = {&a1,&a2};
    TSAtomPList satoms(2, as);
    GetVcoV(satoms, m);
    mat3d vcov = m[0] - m[1] - m[2] + m[3];
    vec3d v = a1.crd() - a2.crd();
    double val = v.Length();
    double esd = sqrt(v.ColMul(vcov).DotProd(v))/val;
    return TEValue<double>(val,esd);
  }
  // cartesian centroid
  TEVPoint<double> CalcCentroid(const TSAtomPList& atoms) {
    mat3d_list m;
    GetVcoV(atoms, m);
    mat3d vcov;
    vec3d center;
    for( int i=0; i < atoms.Count(); i++ )  {
      center += atoms[i]->crd();
      for( int j=0; j < atoms.Count(); j++ )
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
    for( int i=0; i < atoms.Count(); i++ )  {
      center += atoms[i]->ccrd();
      for( int j=0; j < atoms.Count(); j++ )
        vcov += m[i*atoms.Count()+j]; 
    }
    vcov *= 1./(atoms.Count()*atoms.Count());
    center /= atoms.Count();
    return TEVPoint<double>(center[0], center[1], center[2], sqrt(vcov[0][0]), sqrt(vcov[1][1]), sqrt(vcov[2][2]));
  }  // precise calculation
  TEValue<double> CalcPC2ADistanceP(const TSAtomPList& cent, const TSAtom& a) {
    mat3d_list m;
    TSAtomPList satoms(cent);
    satoms.Add(const_cast<TSAtom*>(&a));
    GetVcoV(satoms, m);
    mat3d vcov, nvcov;
    vec3d center;
    // var(atom(i)), cov(atom(i),atom(j))
    for( int i=0; i < cent.Count(); i++ )  {
      center += cent[i]->crd();
      for( int j=0; j < cent.Count(); j++ )
        vcov += m[i*satoms.Count()+j]; 
    }
    vcov *= 1./(cent.Count()*cent.Count());
    center /= cent.Count();
    center -= a.crd();
    // cov( atom(i), a)
    for( int i=0; i < cent.Count(); i++ )  {
      nvcov += m[i*satoms.Count()+cent.Count()];
      nvcov += m[m.Count()-cent.Count()-2+i];
    }
    nvcov *= 1./cent.Count();
    vcov -= nvcov;
    // var(a,a)
    vcov += m.Last();
    double val = center.Length();
    double esd = sqrt(center.ColMul(vcov).DotProd(center))/val;
    return TEValue<double>(val, esd);
  }
  // precise calculation, Sands
  TEValue<double> CalcAngleP(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3) {
    mat3d_list m;
    TSAtom const * as[] = {&a1,&a2,&a3};
    TSAtomPList satoms(3, as);
    GetVcoV(satoms, m);
    vec3d v1(satoms[1]->crd() - satoms[0]->crd()),
          v2(satoms[0]->crd() - satoms[2]->crd()),
          v3(satoms[2]->crd() - satoms[1]->crd());
    mat3d vcov(  v1.ColMul(m[0] - m[3] - m[1] + m[4]).DotProd(v1)/v1.QLength(), // var l1
      v1.ColMul(m[1] - m[4] - m[2] + m[5]).DotProd(v2)/(v1.Length()*v2.Length()), // cov(l1,l2) 
      v1.ColMul(m[2] - m[0] - m[5] + m[3]).DotProd(v3)/(v1.Length()*v3.Length()), // cov(l1,l3) 
      v2.ColMul(m[4] - m[7] - m[5] + m[2]).DotProd(v2)/v2.QLength(), //var l2
      v2.ColMul(m[5] - m[3] - m[8] + m[6]).DotProd(v3)/(v2.Length()*v3.Length()), // cov(l2,l3) 
      v3.ColMul(m[0] - m[2] - m[6] + m[8]).DotProd(v3)/v3.QLength()); //var l3
    double a = acos( (v1.QLength()+v3.QLength()-v2.QLength())/(2*v1.Length()*v3.Length()));
    double ca2 = (v1.QLength()+v2.QLength()-v3.QLength())/(2*v1.Length()*v2.Length());
    double ca3 = (v2.QLength()+v3.QLength()-v1.QLength())/(2*v2.Length()*v3.Length());
    double esd = ca2*ca2*vcov[0][0] - 2*ca2*vcov[0][1] + 2*ca2*ca3*vcov[0][2] + vcov[1][1] +
      2*ca3*vcov[1][2] + ca3*ca3*vcov[2][2];
    esd = sqrt(esd)/(v1.Length()*v3.Length()*sin(a)/v2.Length());
    return TEValue<double>(a*180.0/M_PI,esd*180/M_PI);
  }
  TEValue<double> CalcAngle(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3) {
    TSAtom const * as[] = {&a1,&a2,&a3};
    TSAtomPList satoms(3, as);
    return DoCalcForAtoms(satoms, &VcoVContainer::_calcAngle);
  }
  // tortion angl, precise esd, Acta A30, 848, Uri Shmuelli
  TEValue<double> CalcTAngP(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    mat3d_list m;
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    TSAtomPList satoms(4, as);
    vcov.FindVcoV(satoms, m);
    ProcessSymmetry(satoms, m);
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
    double tau = acos(A/sqrt(B*C));
    double K = -1.0/(sqrt(B*C)*sin(tau)), esd = 0;
    double dt[3][3];
    for( int i=0; i < 3; i++ )  {
      dt[i][0] = K*(v[i]*h23-w[i]*h22 - A/(2*B)*2*(u[i]*h22-v[i]*h12));
      dt[i][1] = K*(u[i]*h23+w[i]*h12-2*v[i]*h13 - A/(2*B)*2*(v[i]*h11-u[i]*h12) -A/(2*C)*2*(v[i]*h33-w[i]*h23));
      dt[i][2] = K*(v[i]*h12-u[i]*h22 - A/(2*C)*2*(w[i]*h22-v[i]*h23));
    }
    double dtx[4][3];
    for( int i=0; i < 3; i++ )  {
      dtx[0][i] = -dt[0][i];
      dtx[1][i] = dt[0][i]-dt[1][i];
      dtx[2][i] = dt[1][i]-dt[2][i];
      dtx[3][i] = dt[2][i];
    }
    for( int k=0; k < 4; k++ )  {
      for( int n=0; n < 4; n++ )  {
        for( int s=0; s < 3; s++ )  {
          for( int q=0; q < 3; q++ )  {
            esd += dtx[k][s]*dtx[n][q]*m[k*4+n][s][q];
          }
        }
      }
    }
    return TEValue<double>(tau*180.0/M_PI, sqrt(esd)*180.0/M_PI);
  }
  // torsion angle
  TEValue<double> CalcTAngle(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    mat3d_list m;
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    TSAtomPList satoms(4, as);
    return DoCalcForAtoms(satoms, &VcoVContainer::_calcTang);
  }
  // bond to bond angle
  TEValue<double> CalcB2BAngle(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    mat3d_list m;
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    TSAtomPList satoms(4, as);
    return DoCalcForAtoms(satoms, &VcoVContainer::_calcB2BAngle);
  }
  // returns rms for the best plane
  TEValue<double> CalcPlane(const TSAtomPList& atoms) {
    weights1.SetCount(atoms.Count());
    for( int i=0; i < atoms.Count(); i++ ) 
      weights1[i] = 1.0;
    return DoCalcForAtoms(atoms, &VcoVContainer::_calcPlane<1>);
  }
  // plane to atom distance
  TEValue<double> CalcP2ADistance(const TSAtomPList& atoms, const TSAtom& a) {
    TSAtomPList satoms(atoms);
    weights1.SetCount(atoms.Count());
    for( int i=0; i < atoms.Count(); i++ ) 
      weights1[i] = 1.0;
    satoms.Add(const_cast<TSAtom*>(&a));
    return DoCalcForAtoms(satoms, &VcoVContainer::_calcP2ADistance);
  }
  // plane centroid to atom distance
  TEValue<double> CalcPC2ADistance(const TSAtomPList& plane, const TSAtom& a) {
    TSAtomPList satoms(plane);
    satoms.Add(const_cast<TSAtom*>(&a));
    return DoCalcForAtoms(satoms, &VcoVContainer::_calcPC2ADistance);
  }
  // plane to a vector angle
  TEValue<double> CalcP2VAngle(const TSAtomPList& plane, const TSAtom& a1, const TSAtom& a2) {
    TSAtomPList satoms(plane);
    weights1.SetCount(satoms.Count());
    for( int i=0; i < satoms.Count(); i++ ) 
      weights1[i] = 1.0;
    satoms.Add(const_cast<TSAtom*>(&a1));
    satoms.Add(const_cast<TSAtom*>(&a2));
    return DoCalcForAtoms(satoms, &VcoVContainer::_calcP2BAngle);
  }
  // plane to plane angle
  TEValue<double> CalcP2PAngle(const TSAtomPList& p1, const TSAtomPList& p2) {
    weights1.SetCount(p1.Count());
    weights2.SetCount(p2.Count());
    for( int i=0; i < p1.Count(); i++ ) 
      weights1[i] = 1.0;
    for( int i=0; i < p2.Count(); i++ ) 
      weights2[i] = 1.0;
    TSAtomPList atoms(p1);
    atoms.AddList(p2);
    return DoCalcExForAtoms(atoms, &VcoVContainer::_calcP2PAngle, p1.Count());
  }
  //plane centroid to plane centroid distance
  TEValue<double> CalcPC2PCDistance(const TSAtomPList& p1, const TSAtomPList& p2) {
    weights1.SetCount(p1.Count());
    weights2.SetCount(p2.Count());
    for( int i=0; i < p1.Count(); i++ ) 
      weights1[i] = 1.0;
    for( int i=0; i < p2.Count(); i++ ) 
      weights2[i] = 1.0;
    TSAtomPList atoms(p1);
    atoms.AddList(p2);
    return DoCalcExForAtoms(atoms, &VcoVContainer::_calcPC2PCDistance, p1.Count());
  }
  //plane to another centroid centroid distance
  TEValue<double> CalcP2PCDistance(const TSAtomPList& p1, const TSAtomPList& p2) {
    weights1.SetCount(p1.Count());
    for( int i=0; i < p1.Count(); i++ ) 
      weights1[i] = 1.0;
    TSAtomPList atoms(p1);
    atoms.AddList(p2);
    return DoCalcExForAtoms(atoms, &VcoVContainer::_calcP2PCDistance, p1.Count());
  }
  // tetrahedron volume
  TEValue<double> CalcTetrahedronVolume(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    TSAtom const * as[] = {&a1,&a2,&a3,&a4};
    TSAtomPList satoms(4, as);
    return DoCalcForAtoms(satoms, &VcoVContainer::_calcTHV);
  }
  // alignment RMSD crds should be prepeared, i.e. inverted
  TEValue<double> CalcAlignmentRMSD(const TSAtomPList& atoms, const vec3d_alist& crds, 
    const TDoubleList& weights) 
  {
    mat3d_list m;
    GetVcoV(atoms, m);
    weights1 = weights;
    return DoCalcForPoints(crds, m, &VcoVContainer::_calcAllignmentRMSD);
  }
  const VcoVMatrix& GetMatrix() const {  return vcov;  }
};

EndXlibNamespace()
#endif
