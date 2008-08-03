#ifndef __olx_xcell
#define __olx_xcell
#include "xbase.h"
#include "threex3.h"

BeginXlibNamespace()
struct XCell {
  double a, b, c, alpha, beta, gamma, V;
  double sigA, sigB, sigC, sigAlpha, sigBeta, sigGamma;
  mat3d Cell2Cartesian, Cartesian2Cell, Hkl2Cartesian,
        UcifToUxyz, UcifToUxyzT,
        UxyzToUcif, UxyzToUcifT;
  XCell(const double cell[6]) : 
        a(cell[0]), b(cell[1]), c(cell[2]), alpha(cell[3]), beta(cell[4]), gamma(cell[5]),
        sigA(0), sigB(0), sigC(0), sigAlpha(0), sigBeta(0), sigGamma(0)  {  
    Init();
  }
  XCell(double _a, double _b, double _c, double _alpha, double _beta, double _gamma) :
        a(_a), b(_b), c(_c), alpha(_alpha), beta(_beta), gamma(_gamma),
        sigA(0), sigB(0), sigC(0), sigAlpha(0), sigBeta(0), sigGamma(0)  {
    Init();
  }
  XCell() : a(0), b(0), c(0), alpha(0), beta(0), gamma(0), V(0), 
            sigA(0), sigB(0), sigC(0), sigAlpha(0), sigBeta(0), sigGamma(0)  {  }
  void Init(double _a, double _b, double _c, double _alpha, double _beta, double _gamma)  {
    a = _a;  b = _b;  c = _c;
    alpha = _alpha;  beta = _beta;  gamma = _gamma;
    Init();
  }
  void SetSigmas(double _a, double _b, double _c, double _alpha, double _beta, double _gamma)  {
    sigA = _a;  sigB = _b;  sigC = _c;
    sigAlpha = _alpha;  sigBeta = _beta;  sigGamma = _gamma;
  }
  void Init()  {
    double cG = cos(gamma/180*M_PI),
      cB = cos(beta/180*M_PI),
      cA = cos(alpha/180*M_PI),
      sG = sin(gamma/180*M_PI),
      sB = sin(beta/180*M_PI),
      sA = sin(alpha/180*M_PI);
    V = a*b*c*sqrt( (1-cA*cA-cB*cB-cG*cG) + 2*(cA*cB*cG));

    double cGs = (cA*cB-cG)/(sA*sB),
      cBs = (cA*cG-cB)/(sA*sG),
      cAs = (cB*cG-cA)/(sB*sG),
      as = b*c*sA/V,
      bs = a*c*sB/V,
      cs = a*b*sG/V;
    Cell2Cartesian[0][0] = a;
    Cell2Cartesian[1][0] = b*cG;
    Cell2Cartesian[2][0] = c*cB;
    Cell2Cartesian[1][1] = b*sG;
    Cell2Cartesian[2][1] = -c*(cB*cG-cA)/sG;
    Cell2Cartesian[2][2] = 1./cs;

    Cartesian2Cell[0][0] =  1./a;
    Cartesian2Cell[1][0] = -cG/(sG*a);
    Cartesian2Cell[2][0] = as*cBs;
    Cartesian2Cell[1][1] = 1./(sG*b);
    Cartesian2Cell[2][1] = bs*cAs;
    Cartesian2Cell[2][2] = cs;

    mat3d m( Cell2Cartesian );
    vec3d v1(m[0]), v2(m[1]), v3(m[2]);

    Hkl2Cartesian[0] = v2.XProdVec(v3)/V;
    Hkl2Cartesian[1] = v3.XProdVec(v1)/V;
    Hkl2Cartesian[2] = v1.XProdVec(v2)/V;

    // init Uaniso traformation matices
    m.Null();
    m[0][0] = Hkl2Cartesian[0].Length();
    m[1][1] = Hkl2Cartesian[1].Length();
    m[2][2] = Hkl2Cartesian[2].Length();

    UcifToUxyz = Cell2Cartesian * m;
    UcifToUxyz.Transpose(UcifToUxyz, UcifToUxyzT);

    m[0][0] = 1./Hkl2Cartesian[0].Length();
    m[1][1] = 1./Hkl2Cartesian[1].Length();
    m[2][2] = 1./Hkl2Cartesian[2].Length();

    UxyzToUcif = m*Cartesian2Cell;
    UxyzToUcif.Transpose(UxyzToUcif, UxyzToUcifT);
  }
  template <class T> T& UcifToUcart(T& v) const { //Q-form
    mat3d M(v[0], v[5], v[4], 
            v[5], v[1], v[3], 
            v[4], v[3], v[2]);
    M = UcifToUxyz*M*UcifToUxyzT;
    v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
    v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
    return v;
  }
  //..............................................................................
  template <class T> T& UcartToUcif(T& v) const {  //Q-form
    mat3d M(v[0], v[5], v[4], 
            v[5], v[1], v[3], 
            v[4], v[3], v[2]);
    M = UxyzToUcif*M*UxyzToUcifT;
    v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
    v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
  }
};
EndXlibNamespace()

#endif
