/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "cifdata.h"

bool CifBond::DoesMatch(const TSAtom& a, const TSAtom& b) const {
  if( a.CAtom().GetId() == base.GetId() )  {
    if( b.CAtom().GetId() != to.GetId() )  return false;
    if( a.IsAUAtom() )  {
      if( mat.GetContainerId() == 0 )
        return b.IsAUAtom();
      for( size_t i=0; i < b.MatrixCount(); i++ )
        if( b.GetMatrix(i) == mat )
          return true;
      return false;
    }
    else  {
      for( size_t i=0; i < a.MatrixCount(); i++ )  {
        const smatd tm = mat*a.GetMatrix(i);
        for( size_t j=0; j < b.MatrixCount(); j++ )  {
          if( b.GetMatrix(j) == tm )
            return true;
        }
      }
    }
  }
  return false;
}

bool CifAngle::DoesMatch(const TSAtom& l, const TSAtom& m, const TSAtom& r) const {
  if( l.CAtom().GetId() == left.GetId() )  {
    if( m.CAtom().GetId() != middle.GetId() )  return false;
    if( r.CAtom().GetId() != right.GetId() )  return false;
    if( m.IsAUAtom() )  {
      if( mat_l.GetContainerId() == 0 && !l.IsAUAtom() )
        return false;
      if( mat_r.GetContainerId() == 0 && !r.IsAUAtom() )
        return false;
      bool found = false;
      for( size_t i=0; i < l.MatrixCount(); i++ )  {
        if( l.GetMatrix(i) == mat_l )  {
          found = true;
          break;
        }
      }
      if( !found )  return false;
      found = false;
      for( size_t i=0; i < r.MatrixCount(); i++ )  {
        if( r.GetMatrix(i) == mat_r )  {
          found = true;
          break;
        }
      }
      return found;
    }
    else  {
      for( size_t i=0; i < m.MatrixCount(); i++ )  {
        const smatd tm_l = mat_l*m.GetMatrix(i);
        bool found = false;
        for( size_t j=0; j < l.MatrixCount(); j++ )  {
          if( l.GetMatrix(j) == tm_l )  {
            found = true;
            break;
          }
        }
        if( !found )  continue;
        const smatd tm_r = mat_r*m.GetMatrix(i);
        found = false;
        for( size_t j=0; j < r.MatrixCount(); j++ )  {
          if( r.GetMatrix(j) == tm_r )  {
            found = true;
            break;
          }
        }
        if( !found )  continue;
        return true;
      }
    }
  }
  return false;
}
