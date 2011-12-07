/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ins_frag_H
#define __olx_ins_frag_H
#include "xbase.h"
BeginXlibNamespace()

enum {
  frag_id_cp = 0,
  frag_id_ph,
  frag_id_naphthalene,
  frag_id_cp_star
};
// coordinates are always cartesian
struct FragAtom {
  olxstr label;
  vec3d crd;
  FragAtom(const olxstr& labl, const vec3d& _crd) : label(labl), crd(_crd)  {  }
  FragAtom(const FragAtom& a) :
  label(a.label),
    crd(a.crd) {  }
  FragAtom& operator = (const FragAtom& a)  {
    label = a.label;
    crd = a.crd;
    return *this;
  }
  olxstr ToString() const {
    return (olxstr(label).RightPadding(5, ' ', true) << "-1  "
      << olxstr::FormatFloat(-5, crd[0]) << ' '
      << olxstr::FormatFloat(-5, crd[1]) << ' '
      << olxstr::FormatFloat(-5, crd[2]));
  }
};

class Fragment {
  int Code;
  TTypeList<FragAtom> Atoms;
  mat3d f2c;  // fractioal to cartesian 
protected:
  void BuildMatrix(double a, double b, double c, double alpha, double beta,
    double gamma)
  {
    double cG = cos(gamma/180*M_PI),
      cB = cos(beta/180*M_PI),
      cA = cos(alpha/180*M_PI),
      sG = sin(gamma/180*M_PI);
    double cs = sG/(c*sqrt( (1-cA*cA-cB*cB-cG*cG) + 2*(cA*cB*cG)));
    f2c[0][0] = a;
    f2c[1][0] = b*cG;
    f2c[2][0] = c*cB;
    f2c[1][1] = b*sG;
    f2c[2][1] = -c*(cB*cG-cA)/sG;
    f2c[2][2] = 1./cs;
  }
public:
  Fragment(int code, double a, double b, double c, 
    double alpha, double beta, double gamma) : Code(code)
  {  
    BuildMatrix(a, b, c, alpha, beta, gamma);
  }
  Fragment(const Fragment& frag) : Code(frag.Code), Atoms(frag.Atoms)  {}
  Fragment& operator = (const Fragment& f)  {  
    Atoms = f.Atoms;  
    Code = f.Code;
    return *this;
  }

  void Reset(double a, double b, double c, double alpha, double beta,
    double gamma)
  {
    BuildMatrix(a, b, c, alpha, beta, gamma);
    Atoms.Clear();
  }

  template <class VecType>
  FragAtom& Add(const olxstr& label, const VecType& v)  {
    return Add(label, v[0], v[1], v[2]);
  }
  
  FragAtom& Add(const olxstr& label, double a, double b, double c)  {
    vec3d crd( 
      a*f2c[0][0] + b*f2c[1][0] + c*f2c[2][0],
      b*f2c[1][1] + c*f2c[2][1],
      c*f2c[2][2]
    );
    return Atoms.AddNew( label, crd );
  }

  int GetCode() const {  return Code;  }

  bool IsEmpty() const {  return Atoms.IsEmpty();  }
  inline size_t Count() const {  return Atoms.Count();  }
  FragAtom& operator [] (size_t i)  {  return Atoms[i];  }
  const FragAtom& operator [] (size_t i) const {  return Atoms[i];  }
  void Delete(size_t i)  {  Atoms.Delete(i);  }
  template <class slist> void ToStrings(slist& lst) const {
    lst.Add("FRAG ") << Code;
    for( size_t i=0; i < Atoms.Count(); i++ )
      lst.Add( Atoms[i].ToString() ); 
    lst.Add("FEND");
  }
  ////
  template <class List>  // one of the type_list
  static List& GenerateRegularRing(size_t sides, double norm, List& rv)  {
    rv.SetCapacity(rv.Count()+sides);
    double sin_a, cos_a;
    olx_sincos(2*M_PI/sides, &sin_a, &cos_a);
    vec3d ps(cos_a, -sin_a, 0);
    for( size_t i=0; i < sides; i++ )  {
      rv.AddCopy(ps*norm);
      const double x = ps[0];
      ps[0] = (cos_a*x + sin_a*ps[1]);
      ps[1] = (cos_a*ps[1] - sin_a*x);
    }
    return rv;
  }
  template <class List>  // one of the type_list
  static List& GenerateFragCrds(int frag_id, List& rv)  {
    if( frag_id == frag_id_cp )
      GenerateRegularRing(5, 0.5*1.42/cos(54*M_PI/180), rv);
    else if( frag_id == frag_id_ph )
      GenerateRegularRing(6, 1.39, rv);
    else if( frag_id == frag_id_naphthalene )  {
      GenerateRegularRing(6, 1.39, rv);
      rv.AddCopy(rv[0]);
      for( size_t i=3; i < 6; i++ )
        rv.AddCopy(rv[i]);
      const vec3d t = (rv[4]+rv[5]).NormaliseTo(1.39*2*cos(M_PI/6));
      for( size_t i=6; i < rv.Count(); i++ )
        rv[i] += t;
      olx_swap(rv[9], rv[7]);
    }
    else if( frag_id == frag_id_cp_star )  {
      const double l = 0.5*1.42/cos(54*M_PI/180);
      GenerateRegularRing(5, l, rv);
      GenerateRegularRing(5, l+1.063, rv);
    }
    else {
      throw TInvalidArgumentException(
        __OlxSourceInfo, "invalid fragment identifier");
    }
    return rv;
  }
};

EndXlibNamespace()
#endif

