#ifndef xlcongenH
#define xlcongenH

#include "congen.h"
#include "ins.h"

BeginXlibNamespace()
class TXlConGen : public AConstraintGenerator {
  TIns* InsFile;
public:
  TXlConGen(TIns* ins );
  virtual bool FixParam(const short paramMask, TStrList& res, const TCAtomPList& atoms, const TFixedValueList& values);
  virtual bool FixAtom( TAtomEnvi& envi, const short Group, const TBasicAtomInfo& atomType, 
    TAtomEnvi* pivoting = NULL, TCAtomPList* generated = NULL);
  // translates shelxl AFIX, HFIX to olex2 notation
  static int OlexToShelx(short code, TAtomEnvi& envi, TAtomEnvi* pivot = NULL) {
    switch( code ) {
      case fgCH3:  
        return 137;
      case fgCH2: 
        if( envi.Count() == 1 )  
          return 93;
        else if( envi.Count() == 2 )  
          return 23;
        break;
      case fgCH1:
        if( envi.Count() == 1 ) 
          return 163;
        else if( envi.Count() == 2 )
          return 43;
        else if( envi.Count() == 3 )
          return 13;
        break;
      case fgSiH1:
        if( envi.Count() == 3 )
          return 13;
      case fgSH1:
      case fgOH1:
        return 147;
        break;
      case fgNH2:
        if( envi.Count() == 1 && pivot == NULL )
          return 93;
        else if( envi.Count() == 2 )
          return 23;
        break;
      case fgNH1:
        if( envi.Count() == 2 )
          return 43;
        else if( envi.Count() == 3 )
          return 13;
        break;
      case fgBH1:
        if( envi.Count() == 4 || envi.Count() == 5 )
          return 153;
        break;
    }
    return -1;
  }
  static short ShelxToOlex(int shelx_code, TAtomEnvi& envi, TAtomEnvi* pivot = NULL)  {
    switch( shelx_code ) {
      case 13:
        if( envi.Count() == 3 )  {
          if( envi.GetBase().GetAtomInfo() == iCarbonIndex )
            return fgCH1;
          else if( envi.GetBase().GetAtomInfo() == iNitrogenIndex )
            return fgNH1;
        }
        break;
      case 23:
        if( envi.Count() == 2 )  {
          if( envi.GetBase().GetAtomInfo() == iCarbonIndex )
            return fgCH2;
          else if( envi.GetBase().GetAtomInfo() == iNitrogenIndex )
            return fgNH2;
        }
        break;
      case 43:
        if( envi.Count() == 2 )  {
          if( envi.GetBase().GetAtomInfo() == iCarbonIndex )
            return fgCH1;
          else if( envi.GetBase().GetAtomInfo() == iNitrogenIndex )
            return fgNH1;
        }
        break;
      case 137:
        if( envi.Count() == 1 && envi.GetBase().GetAtomInfo() == iCarbonIndex )
          return fgCH3;
        break;
    }
    return -1;
  };
};

EndXlibNamespace()
#endif
