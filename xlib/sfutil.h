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
#include "xscatterer.h"
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

  static double GetFsq(double v) { return v; }
  static double GetFsq(const compd &v) { return v.qmod(); }
  static double GetF(double v) { return v; }
  static double GetF(const compd &v) { return v.mod(); }

  struct StructureFactor {
    vec3i hkl;  // hkl indexes
    double ps;  // phase shift
    compd val; // value
    int Compare(const StructureFactor &sf) const {
      return hkl.Compare(sf.hkl);
    }
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
      const TTypeList<XScatterer>& scatterers,
      const TCAtomPList& atoms,
      const double* U,
      const bool anom_only = false) const = 0;
    virtual size_t GetSGOrder() const = 0;
  };


  // for internal use
  void PrepareCalcSF(const TAsymmUnit& au, double* U,
    TTypeList<XScatterer>& scatterers,
    ElementPList &types,
    TCAtomPList& alist);
  /* calculates the scale sum(w*Fc*Fc)/sum(w*Fo^2) Fc = k*Fo.
  F - a list of doubles (F) or complex values
  refs - a list of reflections (Fo^2), sqrt of I will be taken
  */
  template <class FList, class RefList, class weight_t, class filter_t>
  static double CalcFScale(const FList& F, const RefList& refs,
    const weight_t &weights, const filter_t filter)
  {
    double sFo = 0, sFc = 0;
    for (size_t i = 0; i < F.Count(); i++) {
      if (filter.DoesPass(olx_ref::get(refs[i]))) {
        const TReflection &r = olx_ref::get(refs[i]);
        double Fo = r.GetI() <= 0 ? 0 : sqrt(r.GetI());
        double w = weights.Calculate(r);
        sFo += w * Fo * Fo;
        sFc += w* Fo * GetF(F[i]);
      }
    }
    return sFc / sFo;
  }
  /* calculates the scale sum(Fc^2*Fo^2)/sum(Fo^4) Fc^2 = k*Fo^2 for selected
  reflections.
  F - a list of doubles (Fsq) or complex values (F)
  refs - a list of reflections (Fo^2)
  */
  template <class FList, class RefList, class weight_t, class filter_t>
  double CalcF2Scale(const FList& F, const RefList& refs,
    const weight_t &weights, const filter_t filter)
  {
    double sF2o = 0, sF2c = 0;
    const size_t f_cnt = F.Count();
    for (size_t i = 0; i < f_cnt; i++) {
      if (filter.DoesPass(olx_ref::get(refs[i]))) {
        const TReflection &r = olx_ref::get(refs[i]);
        double F2o = r.GetI();
        double w = weights.Calculate(r);
        sF2o += w*F2o * F2o;
        sF2c += w*F2o * GetFsq(F[i]);
      }
    }
    return sF2c / sF2o;
  }
  /* calculates a best line scale : Fc = k*Fo + a.
  F - a list of doubles (F) or complex values (F)
  refs - a list of doubles (Fo) or reflections (Fo^2)
  */
  template <class FList, class RefList>
  void CalcFScale(const FList& F, const RefList& refs, double& k, double& a) {
    double sx = 0, sy = 0, sxs = 0, sxy = 0;
    const size_t f_cnt = F.Count();
    for (size_t i = 0; i < f_cnt; i++) {
      const double I = TReflection::GetF(refs[i]);
      const double qm = GetF(F[i]);
      sx += I;
      sy += qm;
      sxy += I*qm;
      sxs += I*I;
    }
    k = (sxy - sx*sy / f_cnt) / (sxs - sx*sx / f_cnt);
    a = (sy - k*sx) / f_cnt;
  }
  /* calculates a best line scale : Fc^2 = k*Fo^2 + a.
  F - a list of doubles (Fsq) or complex values (F)
  refs - a list of doubles (Fo^2) or reflections (Fo^2)
  */
  template <class FList, class RefList>
  void CalcF2Scale(const FList& F, const RefList& refs, double& k, double& a) {
    double sx = 0, sy = 0, sxs = 0, sxy = 0;
    const size_t f_cnt = F.Count();
    for (size_t i = 0; i < f_cnt; i++) {
      const double I = TReflection::GetFsq(refs[i]);
      const double qm = GetFsq(F[i]);
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
    TArrayList<StructureFactor>& out_)
  {
    const size_t ml_cnt = ml.Count();
    TTypeList<StructureFactor> tmp(ml_cnt * hkl_list.Count());
    for (size_t i = 0; i < hkl_list.Count(); i++) {
      const size_t off = i*ml_cnt;
      for (size_t j = 0; j < ml_cnt; j++) {
        const smatd& m = ml[j];
        const size_t ind = off + j;
        vec3i hkl = TReflection::GetHkl(hkl_list[i]);
        tmp[ind].hkl = hkl*m.r;
        tmp[ind].ps = m.t.DotProd(hkl);
        if (tmp[ind].ps != 0) {
          double ca = 1, sa = 0;
          olx_sincos(-T_PI*tmp[ind].ps, &sa, &ca);
          tmp[ind].val = F[i] * compd(ca, sa);
        }
        else {
          tmp[ind].val = F[i];
        }
      }
    }
    QuickSorter::Sort(tmp, TComparableComparator());
    for (size_t i = 0; i < tmp.Count(); i++) {
      size_t j = i;
      while (++j < tmp.Count() && tmp[i].hkl == tmp[j].hkl) {
        tmp.NullItem(j);
      }
      i = j - 1;
    }
    tmp.Pack();
    out_ = tmp;
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
    short friedelPairs = fpDefault,
    bool anom_only = false);
  // calculates the structure factors for given reflections
  void _CalcSF(const TXFile& xfile, const IMillerIndexList& refs,
    TArrayList<compd>& F, bool UseFpFdp, bool anom_only = false);
  template <class IndexList>
  void CalcSF(const TXFile& xfile, const IndexList& refs,
    TArrayList<compd>& F, bool UseFdp = true, bool anom_only = false)
  {
    _CalcSF(xfile, MillerIndexList<IndexList>(refs), F, UseFdp, anom_only);
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
    template <class RefList, bool centro, bool anom_only> struct SFCalculateTask
      : public TaskBase
    {
      const SF_Util& parent;
      const IMillerIndexList& refs;
      const mat3d& hkl2c;
      TArrayList<compd>& F;
      const TTypeList<XScatterer>& scatterers;
      const TCAtomPList& atoms;
      const double* U;
      TArrayList<vec3i> rv;
      TArrayList<double> ps;
      TArrayList<compd> fo;
      SFCalculateTask(const SF_Util& _parent, const IMillerIndexList& _refs,
        const mat3d& _hkl2c,
        TArrayList<compd>& _F,
        const TTypeList<XScatterer>& _scatterers,
        const TCAtomPList& _atoms, const double* _U,
        const bool anom_only = false)
        : parent(_parent), refs(_refs), hkl2c(_hkl2c), F(_F),
        scatterers(_scatterers), atoms(_atoms), U(_U),
        rv(_parent._getusize()), ps(_parent._getusize()),
        fo(_scatterers.Count())
      {}
      virtual ~SFCalculateTask() {}
      double calc_B(const double* Q, const vec3i &h) const {
        return exp(
          (Q[0] * h[0] + Q[4] * h[2] + Q[5] * h[1])*h[0] +
          (Q[1] * h[1] + Q[3] * h[2])*h[1] +
          (Q[2] * h[2])*h[2]);
      }
      void Run(size_t i) {
        const vec3i hkl = refs[i];  //make a copy, safer
        const double d_s2 = TReflection::ToCart(hkl, hkl2c).QLength()*0.25;
        parent._generateu(hkl, rv, ps);
        if (anom_only) {
          for (size_t j = 0; j < scatterers.Count(); j++) {
            fo[j] = scatterers[j].calc_anomalous();
          }
        }
        else {
         for (size_t j = 0; j < scatterers.Count(); j++) {
           fo[j] = scatterers[j].calc_sq_anomalous(d_s2);
         }
        }
        if (centro) {
          compd ir = 0;
          for (size_t j = 0; j < atoms.Count(); j++) {
            double l = 0;
            for (size_t k = 0; k < parent._getusize(); k++) {
              // scattering vector + phase shift
              const double tv = SFUtil::T_PI*(atoms[j]->ccrd().DotProd(rv[k]) + ps[k]);
              double ca, sa;
              olx_sincos(tv, &sa, &ca);
              if (olx_is_valid_index(atoms[j]->GetEllpId())) {
                const double B = calc_B(&U[j * 6], rv[k]);
                if (atoms[j]->GetEllipsoid()->IsAnharmonic()) {
                  l += (atoms[j]->GetEllipsoid()->
                    GetAnharmonicPart()->calculate(rv[k])*compd(ca*B, sa*B)).GetRe();
                }
                else {
                  l += ca * B;
                }
              }
              else {
                l += ca;
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
        else {
          compd ir = 0;
          for (size_t j = 0; j < atoms.Count(); j++) {
            compd l = 0;
            for (size_t k = 0; k < parent._getusize(); k++) {
              // scattering vector + phase shift
              const double tv = SFUtil::T_PI*(atoms[j]->ccrd().DotProd(rv[k]) + ps[k]);
              double ca, sa;
              olx_sincos(tv, &sa, &ca);
              if (olx_is_valid_index(atoms[j]->GetEllpId())) {
                const double B = calc_B(&U[j * 6], rv[k]);
                if (atoms[j]->GetEllipsoid()->IsAnharmonic()) {
                  l += atoms[j]->GetEllipsoid()->
                    GetAnharmonicPart()->calculate(rv[k])*compd(ca*B, sa*B);
                }
                else {
                  l.Re() += ca * B;
                  l.Im() += sa * B;
                }
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
        return new SFCalculateTask(parent, refs, hkl2c, F, scatterers, atoms, U);
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
      QuickSorter::Sort(out, TComparableComparator());
      for (size_t i = 0; i < out.Count(); i++) {
        size_t j = i;
        while (++j < out.Count() && out[i].hkl == out[j].hkl) {
          out.Delete(j--);
        }
      }
    }
    virtual void Calculate(const IMillerIndexList& refs,
      const mat3d& hkl2c, TArrayList<compd>& F,
      const TTypeList<XScatterer>& scatterers, const TCAtomPList& atoms,
      const double* U, const bool anom_only = false) const
    {
      if (centrosymmetric) {
        if(anom_only){
          SFCalculateTask<TRefList, true, true> task(*this, refs, hkl2c, F, scatterers,
            atoms, U);
          OlxListTask::Run(task, refs.Count(), tLinearTask, 50);
        }
        else{
          SFCalculateTask<TRefList, true, false> task(*this, refs, hkl2c, F, scatterers,
            atoms, U);
          OlxListTask::Run(task, refs.Count(), tLinearTask, 50);
        }
      }
      else {
        if (anom_only){
          SFCalculateTask<TRefList, false, true> task(*this, refs, hkl2c, F,
            scatterers, atoms, U);
          OlxListTask::Run(task, refs.Count(), tLinearTask, 50);
        }
        else{
          SFCalculateTask<TRefList, false, false> task(*this, refs, hkl2c, F,
            scatterers, atoms, U);
          OlxListTask::Run(task, refs.Count(), tLinearTask, 50);
        }
        
      }
    }
    virtual size_t GetSGOrder() const { return sg::size; }
  };

}; // SFUtil namespace

EndXlibNamespace()
#endif
