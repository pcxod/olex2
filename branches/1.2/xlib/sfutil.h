/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __structure_factor_h
#define __structure_factor_h
#include "xapp.h"
#include "emath.h"
#include "fastsymm.h"
#include "symmlib.h"
#include "chemdata.h"
#include "olxmps.h"
BeginXlibNamespace()

namespace SFUtil {

  static const short mapTypeDiff = 0,  // map type
    mapTypeObs = 1,
    mapTypeCalc = 2,
    mapType2OmC = 3;
  static const short scaleSimple = 0,  // scale for difference map
    scaleRegression = 1,
    scaleExternal = 2;
  static const short sfOriginFcf = 0,  // structure factor origin
    sfOriginOlex2 = 1;
  // merge Friedel pairs
  static const short fpDefault = 0,  // depending on SG
    fpMerge = 1,
    fpDoNotMerge = 2;
  static const double T_PI = M_PI * 2;
  static const double MT_PI = -M_PI * 2;
  const static double EQ_PI = 8 * M_PI*M_PI;
  const static double TQ_PI = 2 * M_PI*M_PI;

  struct StructureFactor {
    vec3i hkl;  // hkl indexes
    double ps;  // phase shift
    compd val; // value
  };
  // interface description...
  class ISF_Util {
  public:
    virtual ~ISF_Util() {}
    // expands indexes to P1
    virtual void Expand(const TArrayList<vec3i>& hkl,
      const TArrayList<compd>& F,
      TArrayList<StructureFactor>& out) const = 0;
    /* atoms[i]->Tag() must be index of the corresponding scatterer. U has 6
    elements of Ucif or Uiso for each atom
    */
    virtual void Calculate(const IMillerIndexList& refs,
      const mat3d& hkl2c, TArrayList<compd>& F,
      const ElementPList& scatterers, const TCAtomPList& atoms,
      const double* U,
      const TArrayList<compd> &fpfdp) const = 0;
    virtual size_t GetSGOrder() const = 0;
  };


  // for internal use
  void PrepareCalcSF(const TAsymmUnit& au, double* U,
    ElementPList& scatterers, TCAtomPList& alist);
  /* calculates the scale sum(Fc)/sum(Fo) Fc = k*Fo. Can accept a list of
  doubles (Fo)
  */
  template <class RefList>
  static double CalcFScale(const TArrayList<compd>& F, const RefList& refs) {
    double sF2o = 0, sF2c = 0;
    const size_t f_cnt = F.Count();
    for (size_t i = 0; i < f_cnt; i++) {
      sF2o += TReflection::GetF(refs[i]);
      sF2c += F[i].mod();
    }
    return sF2c / sF2o;
  }
  /* calculates the scale sum(Fc^2)/sum(Fo^2) Fc^2 = k*Fo^2. Can accept a list
  of doubles (Fo^2)
  */
  template <class RefList> double CalcF2Scale(const TArrayList<compd>& F,
    const RefList& refs)
  {
    double sF2o = 0, sF2c = 0;
    const size_t f_cnt = F.Count();
    for (size_t i = 0; i < f_cnt; i++) {
      sF2o += TReflection::GetFsq(refs[i]);
      sF2c += F[i].qmod();
    }
    return sF2c / sF2o;
  }
  /* calculates a best line scale : Fc = k*Fo + a. Can accept a list of doubles (Fo) */
  template <class RefList> void CalcFScale(const TArrayList<compd>& F,
    const RefList& refs, double& k, double& a)
  {
    double sx = 0, sy = 0, sxs = 0, sxy = 0;
    const size_t f_cnt = F.Count();
    for (size_t i = 0; i < f_cnt; i++) {
      const double I = TReflection::GetF(refs[i]);
      const double qm = F[i].mod();
      sx += I;
      sy += qm;
      sxy += I*qm;
      sxs += I*I;
    }
    k = (sxy - sx*sy / f_cnt) / (sxs - sx*sx / f_cnt);
    a = (sy - k*sx) / f_cnt;
  }
  /* calculates a best line scale : Fc^2 = k*Fo^2 + a. Can accept a list of
  doubles (Fo^2)
  */
  template <class RefList> void CalcF2Scale(const TArrayList<compd>& F,
    const RefList& refs, double& k, double& a)
  {
    double sx = 0, sy = 0, sxs = 0, sxy = 0;
    const size_t f_cnt = F.Count();
    for (size_t i = 0; i < f_cnt; i++) {
      const double I = TReflection::GetFsq(refs[i]);
      const double qm = F[i].qmod();
      sx += I;
      sy += qm;
      sxy += I*qm;
      sxs += I*I;
    }
    k = (sxy - sx*sy / f_cnt) / (sxs - sx*sx / f_cnt);
    a = (sy - k*sx) / f_cnt;
  }
  // expands structure factors to P1 for given space group
  void ExpandToP1(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F,
    const TSpaceGroup& sg, TArrayList<StructureFactor>& out);
  template <class RefList, class MatList>
  void ExpandToP1(const RefList& hkl_list, const TArrayList<compd>& F,
    const MatList& ml,
    TArrayList<StructureFactor>& out)
  {
    const size_t ml_cnt = ml.Count();
    out.SetCount(ml_cnt* hkl_list.Count());
    for (size_t i = 0; i < hkl_list.Count(); i++) {
      const size_t off = i*ml_cnt;
      for (size_t j = 0; j < ml_cnt; j++) {
        const smatd& m = ml[j];
        const size_t ind = off + j;
        vec3i hkl = TReflection::GetHkl(hkl_list[i]);
        out[ind].hkl = hkl*m.r;
        out[ind].ps = m.t.DotProd(hkl);
        if (out[ind].ps != 0) {
          double ca = 1, sa = 0;
          olx_sincos(-T_PI*out[ind].ps, &sa, &ca);
          out[ind].val = F[i] * compd(ca, sa);
        }
        else {
          out[ind].val = F[i];
        }
      }
    }
  }
  /* find minimum and maximum values of the miller indexes of the structure
 factor
 */
  void FindMinMax(const TArrayList<StructureFactor>& F,
    vec3i& min, vec3i& max);
  /* prepares the list of hkl and structure factors, return error message or
 empty string
 */
  olxstr GetSF(TRefList& refs, TArrayList<compd>& F,
    short mapType, short sfOrigin = sfOriginOlex2,
    short scaleType = scaleSimple,
    double scale = 0,
    short friedelPairs = fpDefault);
  // calculates the structure factors for given reflections
  void _CalcSF(const TXFile& xfile, const IMillerIndexList& refs,
    TArrayList<compd>& F, bool UseFpFdp);
  template <class IndexList>
  void CalcSF(const TXFile& xfile, const IndexList& refs,
    TArrayList<compd>& F, bool UseFdp = true)
  {
    _CalcSF(xfile, MillerIndexList<IndexList>(refs), F, UseFdp);
  }
  /* returns an instance according to __OLX_USE_FASTSYMM, must be deleted with
 delete
 */
  ISF_Util* GetSF_Util_Instance(const TSpaceGroup& sg);

#ifdef __OLX_USE_FASTSYMM
  template <class sg> class SF_Util : public ISF_Util {
#else
  struct SG_Impl {
    const size_t size, u_size;
    const smatd_list matrices, u_matrices;
    double u_multiplier;
    const bool centro;
    SG_Impl(const smatd_list& all_matrices, const smatd_list& u_matrices,
      bool _centro)
      : size(all_matrices.Count()), u_size(u_matrices.Count()),
      matrices(all_matrices), u_matrices(u_matrices),
      u_multiplier(double(size) / u_size), centro(_centro) {}
    void GenHkl(const vec3i& hkl, TArrayList<vec3i>& out,
      TArrayList<double>& ps) const
    {
      for (size_t i = 0; i < size; i++) {
        out[i] = hkl*matrices[i].r;
        ps[i] = matrices[i].t.DotProd(hkl);
      }
    }
    void GenUniqueHkl(const vec3i& hkl, TArrayList<vec3i>& out,
      TArrayList<double>& ps) const
    {
      for (size_t i = 0; i < u_matrices.Count(); i++) {
        out[i] = hkl*u_matrices[i].r;
        ps[i] = u_matrices[i].t.DotProd(hkl);
      }
    }
  };
  template <class sg> class SF_Util : public ISF_Util, public sg {
#endif
    bool centrosymmetric;
  protected:
    // proxying functions
    inline size_t _getusize() const { return sg::u_size; }
    inline double _getumult() const { return sg::u_multiplier; }
    inline void _generateu(const vec3i& hkl, TArrayList<vec3i>& out,
      TArrayList<double>& ps) const
    {
      sg::GenUniqueHkl(hkl, out, ps);
    }
    template <class RefList, bool centro> struct SFCalculateTask
      : public TaskBase
    {
      const SF_Util& parent;
      const IMillerIndexList& refs;
      const mat3d& hkl2c;
      TArrayList<compd>& F;
      const ElementPList& scatterers;
      const TCAtomPList& atoms;
      const double* U;
      TArrayList<vec3i> rv;
      TArrayList<double> ps;
      TArrayList<compd> fo;
      const TArrayList<compd>& fpfdp;
      SFCalculateTask(const SF_Util& _parent, const IMillerIndexList& _refs,
        const mat3d& _hkl2c,
        TArrayList<compd>& _F, const ElementPList& _scatterers,
        const TCAtomPList& _atoms, const double* _U,
        const TArrayList<compd>& _fpfdp)
        : parent(_parent), refs(_refs), hkl2c(_hkl2c), F(_F),
        scatterers(_scatterers), atoms(_atoms), U(_U),
        rv(_parent._getusize()), ps(_parent._getusize()),
        fo(_scatterers.Count()), fpfdp(_fpfdp)
      {}
      virtual ~SFCalculateTask() {}
      void Run(size_t i) {
        const vec3i hkl = refs[i];  //make a copy, safer
        const double d_s2 = TReflection::ToCart(hkl, hkl2c).QLength()*0.25;
        parent._generateu(hkl, rv, ps);
        for (size_t j = 0; j < scatterers.Count(); j++) {
          fo[j] = scatterers[j]->gaussians->calc_sq(d_s2);
          fo[j] += fpfdp[j];
        }
        if (centro) {
          compd ir = 0;
          for (size_t j = 0; j < atoms.Count(); j++) {
            double l = 0;
            for (size_t k = 0; k < parent._getusize(); k++) {
              // scattering vector + phase shift
              double ca = cos(SFUtil::T_PI*(atoms[j]->ccrd().DotProd(rv[k]) + ps[k]));
              if (olx_is_valid_index(atoms[j]->GetEllpId())) {
                const double* Q = &U[j * 6];  // pick up the correct ellipsoid
                const double B = exp(
                  (Q[0] * rv[k][0] + Q[4] * rv[k][2] + Q[5] * rv[k][1])*rv[k][0] +
                  (Q[1] * rv[k][1] + Q[3] * rv[k][2])*rv[k][1] +
                  (Q[2] * rv[k][2])*rv[k][2]);
                l += ca*B;
              }
              else {
                l += ca;
              }
            }
            double scv = fo[atoms[j]->GetTag()].Re();
            if (!olx_is_valid_index(atoms[j]->GetEllpId())) {
              scv *= exp(U[j * 6] * d_s2);
            }
            scv *= (l*atoms[j]->GetOccu());
            ir += scv;
          }
          F[i] = ir*parent._getumult();
        }
        else {
          compd ir;
          for (size_t j = 0; j < atoms.Count(); j++) {
            compd l;
            for (size_t k = 0; k < parent._getusize(); k++) {
              // scattering vector + phase shift
              const double tv = SFUtil::T_PI*(atoms[j]->ccrd().DotProd(rv[k]) + ps[k]);
              double ca, sa;
              olx_sincos(tv, &sa, &ca);
              if (olx_is_valid_index(atoms[j]->GetEllpId())) {
                const double* Q = &U[j * 6];  // pick up the correct ellipsoid
                const double B = exp(
                  (Q[0] * rv[k][0] + Q[4] * rv[k][2] + Q[5] * rv[k][1])*rv[k][0] +
                  (Q[1] * rv[k][1] + Q[3] * rv[k][2])*rv[k][1] +
                  (Q[2] * rv[k][2])*rv[k][2]);
                l.Re() += ca*B;
                l.Im() += sa*B;
              }
              else {
                l.Re() += ca;
                l.Im() += sa;
              }
            }
            compd scv = fo[atoms[j]->GetTag()];
            if (!olx_is_valid_index(atoms[j]->GetEllpId())) {
              scv *= exp(U[j * 6] * d_s2);
            }
            scv *= atoms[j]->GetOccu();
            scv *= l;
            ir += scv;
          }
          F[i] = ir*parent._getumult();
        }
      }
      SFCalculateTask* Replicate() const {
        return new SFCalculateTask(parent, refs, hkl2c, F, scatterers, atoms, U, fpfdp);
      }
    };
  public:
#ifndef __OLX_USE_FASTSYMM
    SF_Util(const smatd_list& all_mat, const smatd_list& unq_mat, bool centro) :
      sg(all_mat, unq_mat, centro), centrosymmetric(centro) {}
#endif
    virtual void Expand(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F,
      TArrayList<SFUtil::StructureFactor>& out) const
    {
      TArrayList<vec3i> rv(sg::size);
      TArrayList<double> ps(sg::size);
      const size_t hkl_cnt = hkl.Count();
      for (size_t i = 0; i < hkl_cnt; i++) {
        sg::GenHkl(hkl[i], rv, ps);
        const size_t off = i*sg::size;
        for (size_t j = 0; j < sg::size; j++) {
          const size_t ind = j + off;
          out[ind].hkl = rv[j];
          out[ind].ps = ps[j];
          double ca = 1, sa = 0;
          if (ps[j] != 0) {
            olx_sincos(-SFUtil::T_PI*ps[j], &sa, &ca);
            out[ind].val = F[i] * compd(ca, sa);
          }
          else {
            out[ind].val = F[i];
          }
        }
      }
    }
    virtual void Calculate(const IMillerIndexList& refs,
      const mat3d& hkl2c, TArrayList<compd>& F,
      const ElementPList& scatterers, const TCAtomPList& atoms,
      const double* U,
      const TArrayList<compd> &fpfdp_) const
    {
      TArrayList<compd> fpfdp(fpfdp_);
      fpfdp.SetCount(scatterers.Count(), olx_list_init::zero());
      if (centrosymmetric) {
        SFCalculateTask<TRefList, true> task(*this, refs, hkl2c, F, scatterers,
          atoms, U, fpfdp);
        OlxListTask::Run(task, refs.Count(), tLinearTask, 50);
      }
      else {
        SFCalculateTask<TRefList, false> task(*this, refs, hkl2c, F,
          scatterers, atoms, U, fpfdp);
        OlxListTask::Run(task, refs.Count(), tLinearTask, 50);
      }
    }
    virtual size_t GetSGOrder() const { return sg::size; }
  };

}; // SFUtil namespace

EndXlibNamespace()
#endif
