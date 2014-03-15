/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "hall.h"
#include "symmlib.h"
#include "symmparser.h"

//.............................................................................
void HallSymbol::init()  {
  if (trans.IsEmpty())  {
    trans.AddNew(vec3d(0.5, 0, 0), "a");
    trans.AddNew(vec3d(0, 0.5, 0), "b");
    trans.AddNew(vec3d(0, 0, 0.5), "c");
    trans.AddNew(vec3d(0.5, 0.5, 0.5), "n");
    trans.AddNew(vec3d(0.25, 0, 0), "u");
    trans.AddNew(vec3d(0, 0.25, 0), "v");
    trans.AddNew(vec3d(0, 0, 0.25), "w");
    trans.AddNew(vec3d(0.25, 0.25, 0.25), "d");

    rotx.AddNew(rotation_id::get(mat3d( 1, 0, 0,   0,-1, 0,   0, 0,-1)), "2x");
    rotx.AddNew(rotation_id::get(mat3d( 1, 0, 0,   0, 0,-1,   0, 1,-1)), "3x");
    rotx.AddNew(rotation_id::get(mat3d( 1, 0, 0,   0, 0,-1,   0, 1, 0)), "4x");
    rotx.AddNew(rotation_id::get(mat3d( 1, 0, 0,   0, 1,-1,   0, 1, 0)), "6x");

    roty.AddNew(rotation_id::get(mat3d( 0, 0, 1,   0, 1, 0,  -1, 0, 1)), "6y");
    roty.AddNew(rotation_id::get(mat3d( 0, 0, 1,   0, 1, 0,  -1, 0, 0)), "4y");
    roty.AddNew(rotation_id::get(mat3d(-1, 0, 1,   0, 1, 0,  -1, 0, 0)), "3y");
    roty.AddNew(rotation_id::get(mat3d(-1, 0, 0,   0, 1, 0,   0, 0,-1)), "2y");

    rotz.AddNew(rotation_id::get(mat3d( 1,-1, 0,   1, 0, 0,   0, 0, 1)), "6z");
    rotz.AddNew(rotation_id::get(mat3d( 0,-1, 0,   1, 0, 0,   0, 0, 1)), "4z");
    rotz.AddNew(rotation_id::get(mat3d( 0,-1, 0,   1,-1, 0,   0, 0, 1)), "3z");
    rotz.AddNew(rotation_id::get(mat3d(-1, 0, 0,   0,-1, 0,   0, 0, 1)), "2z");
    // x
    rotx1.AddNew(rotation_id::get(mat3d(-1, 0, 0,   0, 0,-1,   0,-1, 0)), "2");
    rotx1.AddNew(rotation_id::get(mat3d(-1, 0, 0,   0, 0, 1,   0, 1, 0)), "2\"");
    // y
    roty1.AddNew(rotation_id::get(mat3d( 0, 0,-1,   0,-1, 0,  -1, 0, 0)), "2");
    roty1.AddNew(rotation_id::get(mat3d( 0, 0, 1,   0,-1, 0,   1, 0, 0)), "2\"");
    // z
    rotz1.AddNew(rotation_id::get(mat3d( 0,-1, 0,  -1, 0, 0,   0, 0,-1)), "2");
    rotz1.AddNew(rotation_id::get(mat3d( 0, 1, 0,   1, 0, 0,   0, 0,-1)), "2\"");

    rot3.AddNew(rotation_id::get(mat3d( 0, 0, 1,   1, 0, 0,   0, 1, 0)), "3*");
    
    TPtrList<TTypeList<olx_pair_t<int,olxstr> > > rots;
    rots << rotx << roty << rotz << rot3;
    for (size_t i=0; i < rots.Count(); i++) {
      for (size_t j=0; j < rots[i]->Count(); j++)
        r_dict.Add((*rots[i])[j].GetB(), (*rots[i])[j].GetA());
    }
    r_dict.Add("1", rotation_id::get(mat3d().I()));
    // a-b
    r_dict.Add("2'", rotz1[0].GetA());
    for (size_t i=0; i < trans.Count(); i++)
      t_dict.Add(trans[i].GetB(), &trans[i].a);
  }
}
//.............................................................................
olxstr HallSymbol::FindT(const vec3d& t, int order) const {
  for( size_t j=0; j < trans.Count(); j++ )  {
    if( trans[j].GetA().QDistanceTo(t) < 1e-6 )
      return trans[j].GetB();
  }
  for( size_t j=0; j < trans.Count(); j++ )  {
    for( size_t k=j+1; k < trans.Count(); k++ )  {
      if( (trans[j].GetA()+trans[k].GetA()).QDistanceTo(t) < 1e-6 )
        return olxstr(trans[j].GetB()) << trans[k].GetB();
    }
  }
  for( size_t j=0; j < trans.Count(); j++ )  {
    for( size_t k=j+1; k < trans.Count(); k++ )  {
      for( size_t l=k+1; l < trans.Count(); l++ )  {
        if( (trans[j].GetA()+trans[k].GetA()+trans[l].GetA()).QDistanceTo(t) < 1e-6 )
          return olxstr(trans[j].GetB()) << trans[k].GetB() << trans[l].GetB();
      }
    }
  }
  const double m = 12./order;
  return olxstr(" (") << t[0]*m << ' ' << t[1]*m << ' ' << t[2]*m << ')';
}
//.............................................................................
olxstr HallSymbol::FindTR(const vec3d& t, int order) const {
  const double v = t[0] != 0 ? t[0] : (t[1] != 0 ? t[1] : t[2]);
  if( v == 0 )  return EmptyString();
  if( order <= 2 || t.Length() != v )
    return FindT(t, order);
  else if( order == 3 )  {
    if( fabs(v - 1./3) < 0.05 )
      return '1';
    if( fabs(v - 2./3) < 0.05 )
      return '2';
  }
  else if( order == 4 )  {
    if( fabs(v - 1./4) < 0.05 )
      return '1';
    if( fabs(v - 3./4) < 0.05 )
      return '3';
  }
  else if( order == 6 )  {
    if( fabs(v - 1./6) < 0.05 )
      return '1';
    if( fabs(v - 2./6) < 0.05 )
      return '2';
      if( fabs(v - 4./6) < 0.05 )
        return '4';
    if( fabs(v - 5./6) < 0.05 )
      return '5';
  }
  return FindT(t, order);
}
//.............................................................................
int HallSymbol::FindR(olxstr& hs, TTypeList<symop>& matrs,
    const TTypeList<olx_pair_t<int,olxstr> >& rot, bool full) const
{
  int previous = 0;
  for( size_t i=0; i < rot.Count(); i++ )  {
    for( size_t j=0; j < matrs.Count(); j++ )  {
      if( matrs.IsNull(j) )  continue;
      const symop& so = matrs[j];
      bool matched = false;
      if( rot[i].GetA() == so.rot_id )  {
        hs << ' ';
        matched = true;
      }
      else if( rot[i].GetA() == rotation_id::negate(so.rot_id) )  {
        hs << " -";
        matched = true;
      }
      if( matched )  {
        if( full )  hs << rot[i].GetB();
        else        hs << rot[i].GetB().CharAt(0);
        previous = rot[i].GetB().CharAt(0)-'0';
        hs << FindTR(so.t, previous);
        matrs.NullItem(j);
        break;
      }
    }
    if( previous != 0 )  break;
  }
  if( previous != 0 )
    matrs.Pack();
  return previous;
}
//.............................................................................
olxstr HallSymbol::EvaluateEx_(int latt_, const smatd_list& matrices_) const {
  smatd_list all_matrices;
  TSymmLib::GetInstance().ExpandLatt(all_matrices, matrices_, latt_);
  return Evaluate(SymmSpace::GetInfo(all_matrices));
}
//.............................................................................
olxstr HallSymbol::Evaluate_(int latt, const smatd_list& matrices) const {
  olxstr hs;
  if( latt > 0 )  hs << '-';
  hs << TCLattice::SymbolForLatt(olx_abs(latt));
  if( matrices.IsEmpty() || (matrices.Count() == 1 && matrices[0].IsI()))
    hs << ' ' << '1';
  else  {
    TTypeList<HallSymbol::symop> matrs;
    for( size_t i=0; i < matrices.Count(); i++ )  {
      symop& so = matrs.AddNew();
      so.rot_id = rotation_id::get(matrices[i].r);
      so.t = matrices[i].t;
    }

    int rz = FindR(hs, matrs, rotz, false);
    // c direction
    if( rz != 0 )  {
      int rx = FindR(hs, matrs, rotx, false);
      if( rx != 0 )  {
        if( FindR(hs, matrs, rot3, false) == 0 )
          FindR(hs, matrs, rotx1, true);
      }
      else
        FindR(hs, matrs, rotz1, true);
    }
    else  { // no c direction
      FindR(hs, matrs, rotx, true);
      FindR(hs, matrs, roty, true);
      if( FindR(hs, matrs, rot3, true) != 0 )
        FindR(hs, matrs, rotz1, true);
    }
  }
  return hs;
}
//..........................................................................................
olxstr HallSymbol::Evaluate_(const SymmSpace::Info& si) const {
  olxstr hs = Evaluate(si.centrosymmetric ? si.latt : -si.latt,
    si.matrices);
  // explicit inversion
  if (!si.inv_trans.IsNull(1e-3)) {
    if (si.centrosymmetric) // must be!
      hs = hs.SubStringFrom(1);
    hs << " -1" << FindT(si.inv_trans, 12);
  }
  return hs;
}
//.............................................................................
vec3d HallSymbol::get_screw_axis_t(int dir, int order) const {
  vec3d rv;
  rv[dir-1] = (double)order/12;
  return rv;
}
//.............................................................................
int HallSymbol::find_diagonal(int axis, olxch which) const {
  int index = which == '\'' ? 0 : 1;
  if (axis == 1)  return rotx1[index].GetA();
  if (axis == 2)  return roty1[index].GetA();
  if (axis == 3)  return rotz1[index].GetA();
  throw TInvalidArgumentException(__OlxSourceInfo, 
    olxstr("axis direction: ") << axis << ", which: " << which);
}
//.............................................................................
SymmSpace::Info HallSymbol::Expand_(const olxstr &_hs) const {
  smatdd change_of_basis;
  change_of_basis.r.I();
  SymmSpace::Info info;
  olxstr hs = _hs;
  // deal with the change of basis
  {
    size_t obi = _hs.IndexOf('(');
    if (obi != InvalidIndex) {
      size_t cbi = _hs.FirstIndexOf(')', obi);
      if (cbi == InvalidIndex) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "change of basis notation");
      }
      hs = _hs.SubStringTo(obi);
      olxstr cbs = _hs.SubString(obi+1, cbi-obi-1);
      if (cbs.IndexOf(',') != InvalidIndex) {
        try { change_of_basis = TSymmParser::SymmToMatrix(cbs); }
        catch (const TExceptionBase &e) {
          throw TFunctionFailedException(__OlxSourceInfo, e);
        }
      }
      else {
        TStrList toks(cbs, ' ');
        if (toks.Count() != 3) {
          throw TInvalidArgumentException(__OlxSourceInfo,
            "change of basis notation");
        }
        change_of_basis.t[0] = toks[0].ToDouble()/12;
        change_of_basis.t[1] = toks[1].ToDouble()/12;
        change_of_basis.t[2] = toks[2].ToDouble()/12;
      }
    }
    
  }
  TStrList toks(hs, ' ');
  if (toks.Count() < 1) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("Hall symbol: ").quote() << _hs);
  }
  info.centrosymmetric = toks[0].StartsFrom('-');
  if (info.centrosymmetric && toks[0].Length() == 1) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("Hall symbol: ").quote() << _hs);
  }
  const int ii_id = rotation_id::get(-mat3i().I());
  const int i_id = r_dict.Get('1');
  info.latt = TCLattice::LattForSymbol(
      toks[0].CharAt(info.centrosymmetric ? 1 : 0));
  int previous = 0, dir = 0;
  SortedObjectList<int, TPrimitiveComparator> r_hash;
  for (size_t i=1; i < toks.Count(); i++) {
    olxstr axis;
    bool neg = toks[i].StartsFrom('-');
    if (neg)
      toks[i] = toks[i].SubStringFrom(1);
    if (toks[i].Length() > 1 ) {
      axis = toks[i].SubStringTo(2);
      size_t ai = r_dict.IndexOf(axis);
      if (ai == InvalidIndex) {
        if (((axis.CharAt(1) == '\'') || (axis.CharAt(1) == '"')) && dir != 0) {
          previous = axis.CharAt(0)-'0';
          smatd& m = info.matrices.AddNew();
          m.r = rotation_id::get(find_diagonal(dir, axis.CharAt(1)));
          if (neg)
            m.r *= -1;
          toks[i] = toks[i].SubStringFrom(2);
        }
        else
          axis.SetLength(0);
      }
      else {
        previous = axis.CharAt(0)-'0';
        smatd& m = info.matrices.AddNew();
        m.r = rotation_id::get(r_dict.GetValue(ai));
        if (neg)
          m.r *= -1;
        toks[i] = toks[i].SubStringFrom(2);
      }
    }
    if (axis.IsEmpty()) {
      axis = toks[i].SubStringTo(1);
      size_t ai = r_dict.IndexOf(axis);
      int current = axis.CharAt(0)-'0';
      if (ai == InvalidIndex) {
        if (i == 1) {
          axis << 'z';
          dir = 3;
        }
        else if (i == 2) {
          if (current == 2) {
            if (previous == 2 || previous == 4) {
              dir = 1;
              axis << 'x';
            }
            else if (previous == 3 || previous == 6) {
              axis << '\'';
            }
          }
        }
        else if (i == 3) {
          if (current != 3) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              olxstr("Axis symbol: ").quote() << axis << " 3/3* is expected");
          }
          axis << '*';
        }
        ai = r_dict.IndexOf(axis);
        if (ai == InvalidIndex) {
          throw TInvalidArgumentException(__OlxSourceInfo,
            olxstr("Axis symbol: ").quote() << axis);
        }
      }
      smatd& m = info.matrices.AddNew();
      m.r = rotation_id::get(r_dict.GetValue(ai));
      if (neg)
        m.r *= -1;
      toks[i] = toks[i].SubStringFrom(1);
      previous = axis.CharAt(0)-'0';
    }
    vec3d t;
    for (size_t j=0; j < toks[i].Length(); j++) {
      olxch t_s = toks[i].CharAt(j);
      if (olxstr::o_isdigit(t_s)) {
        t += get_screw_axis_t(dir, (t_s-'0')*12/(axis.CharAt(0)-'0'));
      }
      else {
        size_t ti = t_dict.IndexOf(t_s);
        if (ti == InvalidIndex) {
          throw TInvalidArgumentException(__OlxSourceInfo,
            olxstr("Translation symbol: ").quote() << toks[i].CharAt(j));
        }
        t += *t_dict.GetValue(ti);
      }
    }
    info.matrices.GetLast().t += t;
    int m_id = rotation_id::get(info.matrices.GetLast().r);
    if (m_id == ii_id) {
      info.inv_trans = info.matrices.GetLast().t;
      info.centrosymmetric = true;
      info.matrices.Delete(info.matrices.Count()-1);
    }
    else if(m_id == i_id) {
      if (info.matrices.GetLast().t.IsNull(1e-3))
        info.matrices.Delete(info.matrices.Count()-1);
    }
    else
      r_hash.Add(m_id);
  }
  if (!change_of_basis.IsI()) {
    smatd cob_i = change_of_basis.Inverse();
    for (size_t i=0; i < info.matrices.Count(); i++) {
      smatdd m = info.matrices[i];
      m = change_of_basis*m*cob_i;
      info.matrices[i].t = m.t;
      for (int ii=0; ii < 3; ii++)
        for (int jj=0; jj < 3; jj++)
      info.matrices[i].r[ii][jj] = olx_round(m.r[ii][jj]);
    }
  }
  for (size_t i=0; i < info.matrices.Count(); i++) {
    for (size_t j=i; j < info.matrices.Count(); j++) {
      smatd m = info.matrices[i]*info.matrices[j];
      int id = rotation_id::get(m.r);
      if (i_id == id) continue;
      if (r_hash.IndexOf(id) == InvalidIndex) {
        r_hash.Add(id);
        info.matrices.AddCopy(m);
      }
    }
  }
  info.matrices.InsertNew(0).r.I();
  info.normalise(
    TSymmLib::GetInstance().GetLatticeByNumber(info.latt).GetVectors());
  return info;
}
