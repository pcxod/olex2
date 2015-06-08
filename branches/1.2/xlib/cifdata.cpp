/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "cifdata.h"
#include "network.h"
#include "unitcell.h"
#include "lattice.h"

bool CifBond::DoesMatch(const TSAtom& a, const TSAtom& b) const {
  if( a.CAtom().GetId() == base.GetId() &&
      b.CAtom().GetId() == to.GetId() )
  {
    const TUnitCell & uc = a.GetNetwork().GetLattice().GetUnitCell();
    return b.IsGenerator(uc.MulMatrixId(a.GetMatrix(), mat));
  }
  return false;
}

bool CifAngle::DoesMatch(const TSAtom& l, const TSAtom& m, const TSAtom& r) const {
  if( l.CAtom().GetId() == left.GetId() &&
      m.CAtom().GetId() == middle.GetId() &&
      r.CAtom().GetId() == right.GetId() )
  {
    const TUnitCell & uc = l.GetNetwork().GetLattice().GetUnitCell();
    return l.IsGenerator(uc.MulMatrixId(m.GetMatrix(), mat_l)) &&
      r.IsGenerator(uc.MulMatrixId(m.GetMatrix(), mat_r));
  }
  return false;
}
