/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_xlcongen_H
#define __olx_xl_xlcongen_H
#include "congen.h"
#include "ins.h"
BeginXlibNamespace()

class TXlConGen : public AConstraintGenerator {
public:
  TXlConGen(RefinementModel& rm) : AConstraintGenerator(rm) {}
  virtual bool FixParam(const short paramMask, TStrList& res,
    const TCAtomPList& atoms, const TFixedValueList& values);
  virtual bool FixAtom(TAtomEnvi& envi, const short Group,
    const cm_Element& atomType, 
    TAtomEnvi* pivoting = NULL, TCAtomPList* generated = NULL);
  virtual void AnalyseMultipart(const TAtomEnvi& envi,
    const TTypeList<TCAtomPList>& parts);
  // translates shelxl AFIX, HFIX to olex2 notation
  static int OlexToShelx(short code, const TAtomEnvi& envi, TAtomEnvi* pivot = NULL) {
    switch( code ) {
      case fgNH3:  
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
  static short ShelxToOlex(int shelx_code, const TAtomEnvi& envi)  {
    const int m = TAfixGroup::GetM(shelx_code);
    switch( m ) {
      case 1:
        if( envi.Count() == 3 )  {
          if( envi.GetBase().GetType() == iCarbonZ )
            return fgCH1;
          else if( envi.GetBase().GetType() == iNitrogenZ )
            return fgNH1;
          else if( envi.GetBase().GetType() == iSiliconZ )
            return fgSiH1;
        }
        break;
      case 2:
      case 9:
        if( envi.Count() == 2 || envi.Count() == 1 )  {
          if( envi.GetBase().GetType() == iCarbonZ )
            return fgCH2;
          else if( envi.GetBase().GetType() == iNitrogenZ )
            return fgNH2;
        }
        break;
      case 3:
      case 13:
        if( envi.Count() == 1 && envi.GetBase().GetType() == iCarbonZ )
          return fgCH3;
        else if( envi.Count() == 1 && envi.GetBase().GetType() == iNitrogenZ )
          return fgNH3;
        break;
      case 4:
        if( envi.Count() == 2 )  {
          if( envi.GetBase().GetType() == iCarbonZ )
            return fgCH1;
          else if( envi.GetBase().GetType() == iNitrogenZ )
            return fgNH1;
        }
        break;
      case 8:
      case 14:
        if( envi.Count() != 1 )
          break;
        if( envi.GetBase().GetType() == iOxygenZ )
          return fgOH1;
        else if( envi.GetBase().GetType() == iSulphurZ )
          return fgOH1;
        break;
      case 15:
        if( envi.GetBase().GetType() == iBoronZ &&
            (envi.Count() == 4 || envi.Count() == 5) )
        {
          return fgBH1;
        }
        break;
      case 16:
        if( envi.Count() == 1 && envi.GetBase().GetType() == iCarbonZ )
          return fgCH1;
        break;
    }
    return -1;
  };
};

EndXlibNamespace()
#endif
