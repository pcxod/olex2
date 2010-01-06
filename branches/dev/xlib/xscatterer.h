#ifndef __olx_x_scatterer_H
#define __olx_x_scatterer_H
#include "chemdata.h"

BeginXlibNamespace()
struct XDispersion  {
  olxstr label;
  compd fpfdp;
  double mu;
  XDispersion() : mu(-1)  {  }
  XDispersion(const olxstr& _label, const compd& _fpfdp, double _mu=-1) :
    label(_label), fpfdp(_fpfdp), mu(_mu)  {  }
  XDispersion(const olxstr& _label, double _fp, double _fdp, double _mu=-1) :
    label(_label), fpfdp(_fp, _fdp), mu(_mu)  {  }
  XDispersion(const XDispersion& d) :
    label(d.label), fpfdp(d.fpfdp), mu(d.mu)  {  }
  olxstr ToInsString() const {
    olxstr rv(label, 50);
    rv << ' ' << fpfdp.GetRe() << ' ' << fpfdp.GetIm();
    if( mu >= 0 )
      rv << ' ' << mu;
    return rv;
  }
};
/* scatterer wrapping to allow user defined values. */
class XScatterer {
private:
  cm_Gaussians gaussians;
  double mu, wt, r;
  compd fpfdp;
  const cm_Element* source;
  olxstr Label;
  bool builtIn;
public:
  // creates a dummy scatterer
  XScatterer() : mu(0), r(0), wt(0), source(NULL), gaussians(0,0,0,0,0,0,0,0,0), builtIn(false) {  }
  
  // creates scatterer from the library
  XScatterer(const cm_Element& src, double energy) : mu(0), builtIn(true), gaussians(0,0,0,0,0,0,0,0,0)  {  
    SetSource(src, energy);
  }
  // searches for the scatterer in the library and initialises data. If scatterer no found, throws exception
  XScatterer(const olxstr& symbol, double energy) : mu(0), builtIn(true), gaussians(0,0,0,0,0,0,0,0,0)  {  
    cm_Element* src = XElementLib::FindBySymbol(symbol);
    if( src == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "unknown scatterer symbol");
    SetSource(*src, energy);
  }
  // copy constructor
  XScatterer(const XScatterer& sc) : 
    gaussians(sc.gaussians), 
    mu(sc.mu),
    wt(sc.wt),
    r(sc.r),
    Label(sc.Label),
    source(sc.source),
    builtIn(sc.builtIn),
    fpfdp(sc.fpfdp)  {  }
  // initialises data from the provided library element
  void SetSource(const cm_Element& src, double energy)  {
    if( src.gaussians == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "given scatterer is only partially initialised");
    gaussians = *src.gaussians;
    Label = src.symbol;
    wt = src.GetMr();
    r = src.r_bonding;
    fpfdp = src.CalcFpFdp(energy) - src.z;
    source = &src;
    builtIn = true;
  }
  // sets custom fp and fdp
  void SetFpFdp(const compd& v) {
    if( v != fpfdp )  {
      fpfdp = v;
      builtIn = false;
    }
  }
  const compd& GetFpFdp() const {  return fpfdp;  }
  // sets custom bonding radius
  void SetR(double v)  {
    if( r != v )  {
      r = v;
      builtIn = false;
    }
  }
  double GetR() const {  return r;  }
  // sets custom molecular weight
  void SetWeight(double v)  {
    if( v != wt )  {
      wt = v;
      builtIn = false;
    }
  }
  double GetWeight() const {  return wt;  }
  // sets custom adsorption coefficient
  void SetAdsorptionCoefficient(double v)  {
    if( v != mu )  {
      mu = v;
      builtIn = false;
    }
  }
  double GetAdsorptionCoefficient() const {  return mu;  }
  // sets custom gaussians
  void SetGaussians(double a1, double a2, double a3, double a4, double b1, double b2, double b3, double b4, double c)  {
    b1 = -b1;  b2 = -b2;  b3 = -b3;  b4 = -b4;
    if( gaussians.a1 != a1 )  {  gaussians.a1 = a1;  builtIn = false;  }
    if( gaussians.a2 != a2 )  {  gaussians.a2 = a2;  builtIn = false;  }
    if( gaussians.a3 != a3 )  {  gaussians.a3 = a3;  builtIn = false;  }
    if( gaussians.a4 != a4 )  {  gaussians.a4 = a4;  builtIn = false;  }
    if( gaussians.b1 != b1 )  {  gaussians.b1 = b1;  builtIn = false;  }
    if( gaussians.b2 != b2 )  {  gaussians.b2 = b2;  builtIn = false;  }
    if( gaussians.b3 != b3 )  {  gaussians.b3 = b3;  builtIn = false;  }
    if( gaussians.b4 != b4 )  {  gaussians.b4 = b4;  builtIn = false;  }
    if( gaussians.c != c )    {  gaussians.c = c;  builtIn = false;  }
  }
  DefPropC(olxstr, Label)
  // return an INS file string representation
  olxstr ToInsString() const {
    olxstr rv(Label, 100);
    rv << ' ' << gaussians.a1 << ' ' << gaussians.a2 << ' ' << gaussians.a3 << ' ' << gaussians.a4 <<
          ' ' << -gaussians.b1 << ' ' << -gaussians.b2 << ' ' << -gaussians.b3 << ' ' << -gaussians.b4 <<
          ' ' << gaussians.c << 
          ' ' << fpfdp.GetRe() << ' ' << fpfdp.GetIm() << ' ' << mu << ' ' << r << ' ' << wt;
    return rv;
  }
  inline double calc_sq(double sqv) const {
    return gaussians.calc_sq(sqv);
  }
  inline compd calc_sq_anomalous(double sqv) const {
    return compd(gaussians.calc_sq(sqv) + fpfdp.GetRe(), fpfdp.GetIm());
  }
};


EndXlibNamespace()

#endif
