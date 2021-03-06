/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_alg_H
#define __olx_gl_alg_H
#include "glbase.h"
#include "bitarray.h"
#include "esphere.h"
#include "olxmps.h"
BeginGlNamespace()

template <typename float_type=double>
struct gl_alg {
  typedef TVector3<float_type> vec_t;
  /* General reference:
  http://paulbourke.net/geometry/pointlineplane/
  */

  /* v1, v2 - define the vector, N(ormal) and p (point on the plane)
  */
  static bool VectorPlaneIntersection(const vec_t& v1, const vec_t& v2,
    const vec_t& N, const vec_t& p, vec_t &out)
  {
    vec_t vd = v2 - v1;
    float_type dp = N.DotProd(vd);
    if (dp == 0) {
      return false;
    }
    float_type u = N.DotProd(p - v1) / dp;
    out = v1 + vd * u;
    return true;
  }
  // for zero based vector
  static bool VectorPlaneIntersection(const vec_t& v,
    const vec_t& N, const vec_t& p, vec_t &out)
  {
    float_type dp = N.DotProd(v);
    if (dp == 0) {
      return false;
    }
    float_type u = N.DotProd(p) / dp;
    out = v * u;
    return true;
  }
  /*
  checks if the line segment intersects with the given plane
  */
  static bool DoLineAndPlaneIntersect(const vec_t& v1, const vec_t& v2,
    const vec_t& N, const vec_t& p)
  {
    vec_t vd = v2 - v1;
    float_type dp = N.DotProd(vd);
    if (dp == 0) {
      return false;
    }
    float_type u = N.DotProd(p - v1) / dp;
    return u >= 0 && u <= 1;
  }

  static bool IsPointInTriangle(const vec_t& p,
    const vec_t& a, const vec_t& b, const vec_t& c)
  {
    // a-b-c
    if (((a - b).XProdVec(p - b)).DotProd((c - b).XProdVec(p - b)) > 0)
      return false;
    // c-a-b
    if (((c - a).XProdVec(p - a)).DotProd((b - a).XProdVec(p - a)) > 0)
      return false;
    // b-c-a
    if (((b - c).XProdVec(p - c)).DotProd((a - c).XProdVec(p - c)) > 0)
      return false;
    return true;
  }

  /*
  ao - line a origin, ad - line a direction
  bo - line b origin, bd - line b direction
  output: pa - point on line a, pb - point on line b defining the shortest
  segment
  */
  static bool LineLineIntersect(const vec_t &ao, const vec_t& ad,
    const vec_t& bo, const vec_t& bd,
    vec_t &pa, vec_t &pb)
  {
    vec_t ab = ao - bo;
    float_type d1343 = ab.DotProd(bd);
    float_type d4321 = bd.DotProd(ad);
    float_type d1321 = ab.DotProd(ad);
    float_type d2121 = ad.QLength();
    float_type d4343 = bd.QLength();
    float_type denom = d2121 * d4343 - d4321 * d4321;
    if (olx_abs(denom) < 1e-6) {
      return false;
    }
    float_type numer = d1343 * d4321 - d1321 * d4343;
    float_type mua = numer / denom;
    float_type mub = (d1343 + d4321 * mua) / d4343;
    pa = ao + ad * mua;
    pb = bo + bd * mub;
    return true;
  }
  /*
  ao - a point on plane a, an - normal for plane a,
  bo - a point on plane b, an - normal for plane b,
  output: lo - a point on the intersection line, ld - direction
  */
  static bool PlanePlaneIntersect(const vec_t &ao, const vec_t &an,
    const vec_t &bo, const vec_t &bn, vec_t &lo, vec_t &ld)
  {
    float_type np = an.DotProd(bn);
    if (np == 0) {
      return false;
    }
    float_type an_ql = an.QLength(),
      bn_ql = bn.QLength(),
      adn = an.DotProd(ao),
      bdn = bn.DotProd(bo);
    float_type dv = an_ql * bn_ql - olx_sqr(np);
    float_type c1 = (adn * bn_ql - bdn * np) / dv,
      c2 = (bdn * an_ql - adn * np) / dv;
    lo = an * c1 + bn * c2;
    ld = an.XProdVec(bn);
    return true;
  }
protected:
  struct DiffTask {
    const TTypeList<vec_t>& v1, & v2;
    const TTypeList<IndexTriangle>& triags;
    const TArrayList<float_type>& QL;
    TEBitArray& used, & used1;
    typedef gl_alg<float_type> alg;
    DiffTask(const TTypeList<vec_t>& v1, const TTypeList<vec_t>& v2,
      const TTypeList<IndexTriangle>& triags,
      const TArrayList<float_type>& QL,
      TEBitArray& used,
      TEBitArray& used1)
      : v1(v1), v2(v2), triags(triags),
      QL(QL), used(used), used1(used1)
    {}
    void Run(size_t i) {
      float_type ql = v1[i].QLength();
      for (size_t j = 0; j < triags.Count(); j++) {
        const IndexTriangle& t = triags[j];
        if (used1[j] || (ql > QL[t[0]] && ql > QL[t[1]] && ql > QL[t[2]]))
          continue;
        vec_t p = alg::VectorPlaneIntersection(v1[i],
          (v2[t[0]] - v2[t[1]]).XProdVec(v2[t[2]] - v2[t[1]]).Normalise(),
          v2[t[2]]);
        if (alg::IsPointInTriangle(p, v2[t[0]], v2[t[1]], v2[t[2]])) {
          used.SetTrue(i);
          used1.SetTrue(j);
        }
      }
    }
    DiffTask* Replicate() const {
      return new DiffTask(v1, v2, triags, QL, used, used1);
    }
  };
public:
  /*given a list of vectors v1 and v2 and lists of the triangles forming a
  shape, this procedure calculates the difference shape consrtcuted from (000)
  at the origin
  */
  static void Difference(const TTypeList<vec_t>& v1,
    const TTypeList<vec_t>& v2,
    const TTypeList<IndexTriangle>& triags,
    TTypeList<IndexTriangle>& out1,
    TTypeList<IndexTriangle>& out2)
  {
    TArrayList<float_type> QL(v2.Count());
    TEBitArray used(v1.Count()), used1(triags.Count());
    for (size_t i = 0; i < v2.Count(); i++)
      QL[i] = v2[i].QLength();
    DiffTask dt(v1, v2, triags, QL, used, used1);
    OlxListTask::Run(dt, v1.Count(), tLinearTask, 1000);
    out1.SetCapacity(out1.Count() + triags.Count());
    out2.SetCapacity(out2.Count() + triags.Count());
    for (size_t i = 0; i < triags.Count(); i++) {
      const IndexTriangle& t = triags[i];
      if (used[t[0]] || used[t[1]] || used[t[2]]) {
        out1.AddCopy(t);
      }
      if (used1[t[0]] || used1[t[1]] || used1[t[2]]) {
        out2.AddCopy(t);
      }
    }
  }

};

EndGlNamespace()
#endif
