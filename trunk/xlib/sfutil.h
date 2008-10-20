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
};

EndXlibNamespace()
#endif
