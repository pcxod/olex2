/* Structure factor utilities 
 (c) O Dolomanov, 2008
*/
#ifndef __structure_factor_h
#define __structure_factor_h

#include "xapp.h"
#include "emath.h"
#include "fastsymm.h"
#include "symmlib.h"
#include "cif.h"
#include "chemdata.h"

BeginXlibNamespace()

struct StructureFactor  {
  vec3i hkl;  // hkl indexes
  double ps;  // phase shift
  compd val; // value
};

class ISF_expansion {
public:
  virtual void Expand(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, TArrayList<StructureFactor>& out) const = 0;
  virtual int GetSGOrder() const = 0;
};

template <class sg> class SF_expansion : public ISF_expansion {
public:
  virtual void Expand(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, TArrayList<StructureFactor>& out) const {
    TArrayList<vec3i> rv(sg::size);
    TArrayList<double> ps(sg::size);
    const double pi_2 = M_PI*2;
    const int hkl_cnt = hkl.Count();
    for( int i=0; i < hkl_cnt; i++ )  {
      sg::GenHkl(hkl[i], rv, ps);
      const int off = i*sg::size;
      for( int j=0; j < sg::size; j++ )  {
        const int ind = j+off;
        out[ind].hkl = rv[j];
        out[ind].ps = ps[j];
        double ca = 1, sa = 0;
        if( ps[j] != 0 )  {
          SinCos(pi_2*ps[j], &sa, &ca);
          out[ind].val = F[i]*compd(ca, sa);
        }
        else
          out[ind].val = F[i];
      }
    }
  }
  virtual int GetSGOrder() const {  return sg::size;  }
};

class ISF_calculation {
public:
  /* atoms[i]->Tag() must be index of the corresponding scatterer. U has 6 elements of Ucif or Uiso for each 
    atom
  */
  virtual void Calculate(double waveLength, const TRefList& refs, TArrayList<compd>& F, 
      const TPtrList<cm_Element> scatterers, const TCAtomPList& atoms, double* U) const = 0;
};

template <class sg> class SF_calculation : public ISF_calculation {
public:
  virtual void Calculate(double waveLength, const TRefList& refs, TArrayList<compd>& F, 
      const TPtrList<cm_Element> scatterers, const TCAtomPList& atoms, double* U) const {
    TArrayList<vec3i> rv(sg::size);
    TArrayList<double> ps(sg::size);
    TArrayList<compd> fo(scatterers.Count()), fpfdp(scatterers.Count());
    const mat3d& hkl2c = atoms[0]->GetParent()->GetHklToCartesian();
    static const double T_PI = M_PI*2;
    static const double ev_angstrom  = 6626.0755 * 2.99792458 / 1.60217733;
    for( int i=0; i < scatterers.Count(); i++ )  {
      fpfdp[i] = scatterers[i]->CalcFpFdp(ev_angstrom/waveLength);
      fpfdp[i] -= scatterers[i]->z;
    }
    for( int i=0; i < refs.Count(); i++ )  {
      const TReflection& ref = refs[i];
      vec3i hkl(ref.GetH(), ref.GetK(), ref.GetL());
      sg::GenHkl(hkl, rv, ps);
      compd ir;
      for( int j=0; j < sg::size; j++ )  {
        const vec3i& o_hkl = rv[j];
        vec3d hkl(o_hkl[0]*hkl2c[0][0] + o_hkl[1]*hkl2c[1][0] + o_hkl[2]*hkl2c[2][0],
                  o_hkl[0]*hkl2c[0][1] + o_hkl[1]*hkl2c[1][1] + o_hkl[2]*hkl2c[2][1],
                  o_hkl[0]*hkl2c[0][2] + o_hkl[1]*hkl2c[1][2] + o_hkl[2]*hkl2c[2][2]);
        const double d_s2 = hkl.QLength()*0.25;
        for( int k=0; k < scatterers.Count(); k++)  {
          fo[k] = scatterers[k]->gaussians->calc_sq(d_s2);
          fo[k] += fpfdp[k];
        }
        compd l;
        for( int k=0; k < atoms.Count(); k++ )  {
          double tv =  T_PI*hkl.DotProd(atoms[k]->ccrd());  // scattering vector + phase shift
          double ca, sa;
          SinCos(tv, &sa, &ca);
          if( atoms[j]->GetEllpId() != -1 )  {
            const double* Q = &U[k*6];  // pick up the correct ellipsoid
            double B = (Q[0]*hkl[0]*hkl[0] + Q[1]*hkl[1]*hkl[1] + Q[2]*hkl[2]*hkl[2] + 
              Q[3]*hkl[1]*hkl[2] + Q[4]*hkl[0]*hkl[2] + Q[5]*hkl[0]*hkl[1]);
            B = exp( B );
            l.Re() += B*ca;
            l.Im() += B*sa;
          }
          else  {
            const double Uiso = exp( U[j*6]*d_s2 );
            l.Re() += Uiso*ca;
            l.Im() += Uiso*sa;
          }
          l *= atoms[j]->GetOccp();
          l *= fo[ atoms[j]->GetTag() ];
          ir += l;
        }
      }
      F[i] = ir;
    }
  }
};

namespace SFUtil {
  static const short mapTypeDiff = 0,  // map type
                     mapTypeObs  = 1,
                     mapTypeCalc = 2,
                     mapType2OmC = 3;
  static const short scaleSimple = 0,  // scale for difference map
                     scaleRegression = 1;
  static const short sfOriginFcf = 0,  // structure factor origin
                     sfOriginOlex2 = 1;
  void ExpandToP1(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, const TSpaceGroup& sg, TArrayList<StructureFactor>& out);
  void FindMinMax(const TArrayList<StructureFactor>& F, vec3i& min, vec3i& max);
  // prepares the list of hkl and structure factors, return error message or empty string
  olxstr GetSF(TRefList& refs, TArrayList<compd>& F, 
    short mapType, short sfOrigin = sfOriginOlex2, short scaleType = scaleSimple);
  void CalcSF(TXFile& xfile, const TRefList& refs, TArrayList<compd>& F, bool useFpDfp);
};

EndXlibNamespace()
#endif
