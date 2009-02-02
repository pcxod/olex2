/* Structure factor utilities 
 (c) O Dolomanov, 2008
*/
#ifndef __structure_factor_h
#define __structure_factor_h

#include "xapp.h"
#include "emath.h"
#include "fastsymm.h"
#include "symmlib.h"
#include "chemdata.h"

BeginXlibNamespace()

namespace SFUtil {

  static const short mapTypeDiff = 0,  // map type
    mapTypeObs  = 1,
    mapTypeCalc = 2,
    mapType2OmC = 3;
  static const short scaleSimple = 0,  // scale for difference map
    scaleRegression = 1;
  static const short sfOriginFcf = 0,  // structure factor origin
    sfOriginOlex2 = 1;
  static const double T_PI = M_PI*2;
  const static double EQ_PI = 8*M_PI*M_PI;
  const static double TQ_PI = 2*M_PI*M_PI;
  
  static inline double GetReflectionF(const TReflection* r) {  return r->GetI() <= 0 ? 0 : sqrt(r->GetI());  }
  static inline double GetReflectionF(const TReflection& r) {  return r.GetI() <=0 ? 0 : sqrt(r.GetI());  }
  static inline double GetReflectionF(const double& r) {  return r;  }
  static inline double GetReflectionF2(const TReflection* r) {  return r->GetI();  }
  static inline double GetReflectionF2(const TReflection& r) {  return r.GetI();  }
  static inline double GetReflectionF2(const double& r) {  return r;  }


  struct StructureFactor  {
    vec3i hkl;  // hkl indexes
    double ps;  // phase shift
    compd val; // value
  };
  // for internal use
  void PrepareCalcSF(const TAsymmUnit& au, double* U, TPtrList<cm_Element>& scatterers, TCAtomPList& alist); 
  /* calculates the scale sum(Fc)/sum(Fo) Fc = k*Fo. Can accept a list of doubles (Fo) */
  template <class RefList>
  static double CalcFScale(const TArrayList<compd>& F, const RefList& refs)  {
    double sF2o = 0, sF2c = 0;
    const int f_cnt = F.Count();
    for( int i=0; i < f_cnt; i++ )  {
      sF2o += GetReflectionF(refs[i]);
      sF2c += F[i].mod();
    }
    return sF2c/sF2o;
  }
  /* calculates the scale sum(Fc^2)/sum(Fo^2) Fc^2 = k*Fo^2. Can accept a list of doubles (Fo^2) */
  template <class RefList> double CalcF2Scale(const TArrayList<compd>& F, const RefList& refs)  {
    double sF2o = 0, sF2c = 0;
    const int f_cnt = F.Count();
    for( int i=0; i < f_cnt; i++ )  {
      sF2o += GetReflectionF2(refs[i]);
      sF2c += F[i].qmod();
    }
    return sF2c/sF2o;
  }
  /* calculates a best line scale : Fc = k*Fo + a. Can accept a list of doubles (Fo) */
  template <class RefList> void CalcFScale(const TArrayList<compd>& F, const RefList& refs, double& k, double& a)  {
    double sx = 0, sy = 0, sxs = 0, sxy = 0;
    const int f_cnt = F.Count();
    for( int i=0; i < f_cnt; i++ )  {
      const double I = GetReflectionF(refs[i]);
      const double qm = F[i].mod();
      sx += I;
      sy += qm;
      sxy += I*qm;
      sxs += I*I;
    }
    k = (sxy - sx*sy/f_cnt)/(sxs - sx*sx/f_cnt);
    a = (sy - k*sx)/f_cnt;
  }
  /* calculates a best line scale : Fc^2 = k*Fo^2 + a. Can accept a list of doubles (Fo^2) */
  template <class RefList> void CalcF2Scale(const TArrayList<compd>& F, const RefList& refs, double& k, double& a)  {
    double sx = 0, sy = 0, sxs = 0, sxy = 0;
    const int f_cnt = F.Count();
    for( int i=0; i < f_cnt; i++ )  {
      const double I = GetReflectionF2(refs[i]);
      const double qm = F[i].qmod();
      sx += I;
      sy += qm;
      sxy += I*qm;
      sxs += I*I;
    }
    k = (sxy - sx*sy/f_cnt)/(sxs - sx*sx/f_cnt);
    a = (sy - k*sx)/f_cnt;
  }
  // expands structure factors to P1 for given space group
  void ExpandToP1(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, const TSpaceGroup& sg, TArrayList<StructureFactor>& out);
  // find minimum and maximum values of the miller indexes of the structure factor
  void FindMinMax(const TArrayList<StructureFactor>& F, vec3i& min, vec3i& max);
  // prepares the list of hkl and structure factors, return error message or empty string
  olxstr GetSF(TRefList& refs, TArrayList<compd>& F, 
    short mapType, short sfOrigin = sfOriginOlex2, short scaleType = scaleSimple);
  // calculates the structure factors for given reflections
  void CalcSF(const TXFile& xfile, const TRefList& refs, TArrayList<compd>& F, bool useFpFdp);
  // calculates the structure factors for given reflections
  void CalcSF(const TXFile& xfile, const TRefPList& refs, TArrayList<compd>& F, bool useFpFdp);

}; // SFUtil namespace

  class ISF_Util {
  public:
    // expands indexes to P1
    virtual void Expand(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, TArrayList<SFUtil::StructureFactor>& out) const = 0;
    /* atoms[i]->Tag() must be index of the corresponding scatterer. U has 6 elements of Ucif or Uiso for each 
    atom  */
    virtual void Calculate(double eV, const TRefList& refs, const mat3d& hkl2c, TArrayList<compd>& F, 
      const TPtrList<cm_Element>& scatterers, const TCAtomPList& atoms, const double* U, bool useFpFdp) const = 0;
    virtual void Calculate(double eV, const TRefPList& refs, const mat3d& hkl2c, TArrayList<compd>& F, 
      const TPtrList<cm_Element>& scatterers, const TCAtomPList& atoms, const double* U, bool useFpFdp) const = 0;
    virtual int GetSGOrder() const = 0;
  };

  template <class sg> class SF_Util : public ISF_Util {
  protected:
    template <class RefList>
    static void CalculateWithFpFdp( double eV, const RefList& refs, const mat3d& hkl2c, TArrayList<compd>& F, 
                                    const TPtrList<cm_Element>& scatterers, const TCAtomPList& atoms, 
                                    const double* U)
    {
      TArrayList<vec3i> rv(sg::size);
      TArrayList<double> ps(sg::size);
      TArrayList<compd> fo(scatterers.Count()), fpfdp(scatterers.Count());
      for( int i=0; i < scatterers.Count(); i++ )  {
        fpfdp[i] = scatterers[i]->CalcFpFdp(eV);
        fpfdp[i] -= scatterers[i]->z;
      }
      for( int i=0; i < refs.Count(); i++ )  {
        const TReflection& ref = TReflection::GetRef(refs[i]);
        const double d_s2 = ref.ToCart(hkl2c).QLength()*0.25;
        sg::GenHkl(ref.GetHkl(), rv, ps);
        for( int j=0; j < scatterers.Count(); j++)  {
          fo[j] = scatterers[j]->gaussians->calc_sq(d_s2);
          fo[j] += fpfdp[j];
        }
        compd ir;
        for( int j=0; j < atoms.Count(); j++ )  {
          compd l;
          for( int k=0; k < sg::size; k++ )  {
            double tv =  SFUtil::T_PI*(atoms[j]->ccrd().DotProd(rv[k])+ps[k]);  // scattering vector + phase shift
            double ca, sa;
            SinCos(tv, &sa, &ca);
            if( atoms[j]->GetEllpId() != -1 )  {
              const double* Q = &U[j*6];  // pick up the correct ellipsoid
              const double B = exp(
                (Q[0]*rv[k][0]+Q[4]*rv[k][2]+Q[5]*rv[k][1])*rv[k][0] + 
                (Q[1]*rv[k][1]+Q[3]*rv[k][2])*rv[k][1] + 
                (Q[2]*rv[k][2])*rv[k][2] );
              l.Re() += ca*B;
              l.Im() += sa*B;
            }
            else  {
              l.Re() += ca;
              l.Im() += sa;
            }
          }
          if( atoms[j]->GetEllpId() == -1 )
            l *= exp( U[j*6]*d_s2 );
          l *= atoms[j]->GetOccu();
          l *= fo[ atoms[j]->GetTag() ];
          ir += l;
        }
        F[i] = ir;
      }
    }
    template <class RefList>
    static void CalculateWithoutFpFdp(const RefList& refs, const mat3d& hkl2c, TArrayList<compd>& F, 
                                      const TPtrList<cm_Element>& scatterers, const TCAtomPList& atoms, 
                                      const double* U) 
    {
      TArrayList<vec3i> rv(sg::size);
      TArrayList<double> ps(sg::size);
      TArrayList<double> fo(scatterers.Count());
      for( int i=0; i < refs.Count(); i++ )  {
        const TReflection& ref = TReflection::GetRef(refs[i]);
        const double d_s2 = ref.ToCart(hkl2c).QLength()*0.25;
        sg::GenHkl(ref.GetHkl(), rv, ps);
        for( int j=0; j < scatterers.Count(); j++)
          fo[j] = scatterers[j]->gaussians->calc_sq(d_s2);

        compd ir;
        for( int j=0; j < atoms.Count(); j++ )  {
          compd l;
          for( int k=0; k < sg::size; k++ )  {
            double tv =  SFUtil::T_PI*(atoms[j]->ccrd().DotProd(rv[k])+ps[k]);  // scattering vector + phase shift
            double ca, sa;
            SinCos(tv, &sa, &ca);
            if( atoms[j]->GetEllpId() != -1 )  {
              const double* Q = &U[j*6];  // pick up the correct ellipsoid
              const double B = exp(
                (Q[0]*rv[k][0]+Q[4]*rv[k][2]+Q[5]*rv[k][1])*rv[k][0] + 
                (Q[1]*rv[k][1]+Q[3]*rv[k][2])*rv[k][1] + 
                (Q[2]*rv[k][2])*rv[k][2] );
              l.Re() += ca*B;
              l.Im() += sa*B;
            }
            else  {
              l.Re() += ca;
              l.Im() += sa;
            }
          }
          if( atoms[j]->GetEllpId() == -1 )
            l *= exp( U[j*6]*d_s2 );
          l *= atoms[j]->GetOccu();
          l *= fo[ atoms[j]->GetTag() ];
          ir += l;
        }
        F[i] = ir;
      }
    }
  public:
    virtual void Expand(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, TArrayList<SFUtil::StructureFactor>& out) const {
      TArrayList<vec3i> rv(sg::size);
      TArrayList<double> ps(sg::size);
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
            SinCos(SFUtil::T_PI*ps[j], &sa, &ca);
            out[ind].val = F[i]*compd(ca, sa);
          }
          else
            out[ind].val = F[i];
        }
      }
    }
    virtual void Calculate( double eV, const TRefList& refs, const mat3d& hkl2c, TArrayList<compd>& F, 
                            const TPtrList<cm_Element>& scatterers, const TCAtomPList& atoms, 
                            const double* U, bool useFpFdp) const 
    {
      if( useFpFdp )
        CalculateWithFpFdp<TRefList>(eV, refs, hkl2c, F, scatterers, atoms, U);
      else
        CalculateWithoutFpFdp<TRefList>(refs, hkl2c, F, scatterers, atoms, U);
    }
    virtual void Calculate( double eV, const TRefPList& refs, const mat3d& hkl2c, TArrayList<compd>& F, 
                            const TPtrList<cm_Element>& scatterers, const TCAtomPList& atoms, 
                            const double* U, bool useFpFdp) const 
    {
      if( useFpFdp )
        CalculateWithFpFdp<TRefPList>(eV, refs, hkl2c, F, scatterers, atoms, U);
      else
        CalculateWithoutFpFdp<TRefPList>(refs, hkl2c, F, scatterers, atoms, U);
    }
    virtual int GetSGOrder() const {  return sg::size;  }
  };

EndXlibNamespace()
#endif
