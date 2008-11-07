#ifndef __olxs_v_co_v_h
#define __olxs_v_co_v_h
#include "asymmunit.h"
#include "lattice.h"
#include "ecast.h"
//#include "ematrix.h"
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
      if( Index[i].GetC() == a.GetLoaderId() )
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
  void FindVcoV(const TPtrList<const TCAtom>& atoms, mat3d_list& m ) const;
  // for tests
  double Find(const olxstr& atom, const short va, const short vy) const;
};

class VcoVContainer {
  VcoVMatrix vcov;
protected:
  void ProcessSymmetry(const TPtrList<const TSAtom>& atoms, mat3d_list& ms)  {
    mat3d_list left(atoms.Count()), right(atoms.Count());
    int mc = 0;
    for( int i=0; i < atoms.Count(); i++ )  {
      const mat3d& m = atoms[i]->GetMatrix(0).r;
      if( m.IsI() )  continue;
      left.SetACopy(i, m); 
      right.SetACopy(i, m);
      right[i].Transpose(); 
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
public:
  VcoVContainer()  {
    weights = NULL;
  }
  void ReadShelxMat(const olxstr& fileName, TAsymmUnit& au) {
    vcov.ReadShelxMat(fileName, au);
  }
  TEValue<double> CalcDistance(const TSAtom& a1, const TSAtom& a2) {
    mat3d_list m;
    TPtrList<const TCAtom> catoms;
    TPtrList<const TSAtom> satoms;
    satoms.Add(&a1);
    satoms.Add(&a2);
    TListCaster::POP(satoms, catoms);
    vcov.FindVcoV(catoms, m);
    ProcessSymmetry(satoms, m);
    mat3d vcov = m[0] - m[1] - m[2] + m[3];
    mat3d c2f( a1.CAtom().GetParent()->GetCellToCartesian() );
    mat3d c2f_t( mat3d::Transpose(a1.CAtom().GetParent()->GetCellToCartesian()) );
    vcov = c2f_t*vcov*c2f;
    vec3d v = a1.crd() - a2.crd();
    double val = v.Length();
    double esd = sqrt(v.ColMul(vcov).DotProd(v))/val;
    return TEValue<double>(val,esd);
  }
  TEValue<double> CalcCentroid(const TSAtomPList& atoms) {
    return TEValue<double>(0,0);
  }
  TEValue<double> CalcDistanceToCentroid(const TSAtomPList& cent, const TSAtom& a) {
    mat3d_list m;
    TPtrList<const TCAtom> catoms;
    TPtrList<const TSAtom> satoms;
    satoms.SetCapacity(cent.Count()+1);
    for( int i=0; i < cent.Count(); i++ )
      satoms.Add( cent[i] );
    satoms.Add(&a);
    TListCaster::POP(satoms, catoms);
    vcov.FindVcoV(catoms, m);
    ProcessSymmetry(satoms, m);
    mat3d c2f( a.CAtom().GetParent()->GetCellToCartesian() );
    mat3d c2f_t( mat3d::Transpose(a.CAtom().GetParent()->GetCellToCartesian()) );
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
    // to cartesian frame
    vcov = c2f_t*vcov*c2f;
    double val = center.Length();
    double esd = sqrt(center.ColMul(vcov).DotProd(center))/val;
    return TEValue<double>(val, esd);
  }

  TEValue<double> CalcAngle(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3) {
    mat3d_list m;
    TPtrList<const TCAtom> catoms;
    TPtrList<const TSAtom> satoms;
    satoms.Add(&a2);
    satoms.Add(&a1);
    satoms.Add(&a3);
    TListCaster::POP(satoms, catoms);
    vcov.FindVcoV(catoms, m);
    ProcessSymmetry(satoms, m);
    mat3d c2f( a1.CAtom().GetParent()->GetCellToCartesian() );
    mat3d c2f_t( mat3d::Transpose(a1.CAtom().GetParent()->GetCellToCartesian()) );
    for( int i=0; i < m.Count(); i++ )
      m[i] = c2f_t*m[i]*c2f;
    vec3d v1(satoms[0]->crd() - satoms[1]->crd()),
          v2(satoms[1]->crd() - satoms[2]->crd()),
          v3(satoms[2]->crd() - satoms[0]->crd());
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
  // Acta A30, 848, Uri Shmuelli
  TEValue<double> CalcTAng(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    mat3d_list m;
    TPtrList<const TCAtom> catoms;
    TPtrList<const TSAtom> satoms;
    satoms.Add(&a1);  satoms.Add(&a2);  satoms.Add(&a3);  satoms.Add(&a4);
    TListCaster::POP(satoms, catoms);
    vcov.FindVcoV(catoms, m);
    ProcessSymmetry(satoms, m);
    mat3d c2f( a1.CAtom().GetParent()->GetCellToCartesian() );
    mat3d c2f_t( mat3d::Transpose(a1.CAtom().GetParent()->GetCellToCartesian()) );
    for( int i=0; i < m.Count(); i++ )
      m[i] = c2f_t*m[i]*c2f;
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
protected:
  TDoubleList weights;
  vec3d plane_params, plane_center;
  double CalcPlane(const vec3d_list& Points) {
    mat3d m, vecs;
    vec3d t;
    double mass = 0;
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
        plane_params = vecs[0];
        return m[0][0];
      }
      else  {
        plane_params = vecs[2];
        return m[2][2];
      }
    }
    else  {
      if( m[1][1] < m[2][2] )  {
        plane_params = vecs[1];
        return m[1][1];
      }
      else  {
        plane_params = vecs[2];
        return m[2][2];
      }
    }
  }
  double CalcTHV(const vec3d_list& points) const {
    return TetrahedronVolume(points[0], points[1], points[2], points[3]);
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
public:
  TEValue<double> Plane(const TSAtomPList& atoms) {
    mat3d_list m;
    TPtrList<const TCAtom> catoms;
    TPtrList<const TSAtom> satoms;
    vec3d_list points;
    TDoubleList dl(atoms.Count()*3);
    weights.SetCount(atoms.Count());
    points.SetCapacity(atoms.Count());
    satoms.SetCapacity(atoms.Count());
    for( int i=0; i < atoms.Count(); i++ )  {
      points.AddNew(atoms[i]->crd());
      satoms.Add(atoms[i]);
      weights[i] = 1.0;
    }
    TListCaster::POP(satoms, catoms);
    vcov.FindVcoV(catoms, m);
    ProcessSymmetry(satoms, m);
    mat3d c2f( atoms[0]->CAtom().GetParent()->GetCellToCartesian() );
    mat3d c2f_t( mat3d::Transpose(atoms[0]->CAtom().GetParent()->GetCellToCartesian()) );
    for( int i=0; i < m.Count(); i++ )
      m[i] = c2f_t*m[i]*c2f;
    CalcDiff(points, dl, &VcoVContainer::CalcPlane);
    double esd = CalcEsd(atoms.Count(), m, dl);
    return TEValue<double>(CalcPlane(points), sqrt(esd));
  }
  TEValue<double> CalcTetrahedronVolume(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4) {
    mat3d_list m;
    TPtrList<const TCAtom> catoms;
    TPtrList<const TSAtom> satoms;
    vec3d_list points;
    satoms.Add(&a1);  satoms.Add(&a2);  satoms.Add(&a3);  satoms.Add(&a4);
    points.AddCCopy(a1.crd());
    points.AddCCopy(a2.crd());
    points.AddCCopy(a3.crd());
    points.AddCCopy(a4.crd());
    TDoubleList dl(satoms.Count()*3);
    TListCaster::POP(satoms, catoms);
    vcov.FindVcoV(catoms, m);
    ProcessSymmetry(satoms, m);
    mat3d c2f( a1.CAtom().GetParent()->GetCellToCartesian() );
    mat3d c2f_t( mat3d::Transpose(a1.CAtom().GetParent()->GetCellToCartesian()) );
    for( int i=0; i < m.Count(); i++ )
      m[i] = c2f_t*m[i]*c2f;
    vec3d cnt, p;
    CalcDiff(points, dl, &VcoVContainer::CalcTHV);
    double esd = CalcEsd(4, m, dl);
    return TEValue<double>(TetrahedronVolume(points[0], points[1], points[2], points[3]), sqrt(esd));
  }
  const VcoVMatrix& GetMatrix() const {  return vcov;  }
};

EndXlibNamespace()
#endif
