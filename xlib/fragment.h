#ifndef __olx_ins_frag_H
#define __olx_ins_frag_H
#include "xbase.h"
BeginXlibNamespace()

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
    return (olxstr(label).Format(5, true, ' ') << "-1  "
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
  void BuildMatrix(double a, double b, double c, double alpha, double beta, double gamma)
  {
    double cG = cos(gamma/180*M_PI),
      cB = cos(beta/180*M_PI),
      cA = cos(alpha/180*M_PI),
      sG = sin(gamma/180*M_PI),
      sB = sin(beta/180*M_PI),
      sA = sin(alpha/180*M_PI);
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
  Fragment(const Fragment& frag) : Atoms(frag.Atoms), Code(frag.Code)  {  }
  Fragment& operator = (const Fragment& f)  {  
    Atoms = f.Atoms;  
    Code = f.Code;
    return *this;
  }

  void Reset(double a, double b, double c, double alpha, double beta, double gamma)  {
    BuildMatrix(a, b, c, alpha, beta, gamma);
    Atoms.Clear();
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

  template <class slist> void ToStrings(slist& lst) const {
    lst.Add("FRAG ") << Code;
    for( size_t i=0; i < Atoms.Count(); i++ )
      lst.Add( Atoms[i].ToString() ); 
    lst.Add("FEND");
  }
};

EndXlibNamespace()
#endif

