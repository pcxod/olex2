#include "hall.h"

char HallSymbol::LattSymbols[] = "PIRFABC";  // [abs(latt)-1]
TTypeList<AnAssociation2<vec3d, olxstr> > HallSymbol::trans;
TTypeList<AnAssociation2<int,olxstr> >
  HallSymbol::rotx,
  HallSymbol::roty,
  HallSymbol::rotz,
  HallSymbol::rotx1,
  HallSymbol::roty1,
  HallSymbol::rotz1,
  HallSymbol::rot3;
//..........................................................................................

void HallSymbol::init()  {
  if( trans.IsEmpty() )  {
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
  }
}
//..........................................................................................
olxstr HallSymbol::FindT(const vec3d& t, int order)  {
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
  return olxstr(" (") << t[0]*m << ',' << t[1]*m << ',' << t[2]*m << ')';
  //throw TFunctionFailedException(__OlxSourceInfo, olxstr("Failed to encode translation: ") << t.ToString());
}
//..........................................................................................
olxstr HallSymbol::FindTR(const vec3d& t, int order)  {
  const double v = t[0] != 0 ? t[0] : (t[1] != 0 ? t[1] : t[2]);
  if( v == 0 )  return EmptyString;
  bool processed = false;
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
//..........................................................................................
int HallSymbol::FindR(olxstr& hs, TTypeList<symop>& matrs,
    const TTypeList<AnAssociation2<int,olxstr> >& rot, bool full)
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
      else if( rot[i].GetA() == rotation_id::invert(so.rot_id) )  {
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
//..........................................................................................
olxstr HallSymbol::Evaluate(int latt, const smatd_list& matrices)  {
  init();
  olxstr hs;
  if( latt > 0 )  hs << '-';
  hs << GetLatticeSymbol(latt);
  if( matrices.IsEmpty() )
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
olxstr HallSymbol::FindCentering(smatd_list& matrices)  {
  olxdict<int, TPtrList<smatd>, TPrimitiveComparator> groups;
  for( size_t i=0; i < matrices.Count(); i++ )  {
    TPtrList<smatd>& l = groups.Add(rotation_id::get(matrices[i].r));
    l.Add(matrices[i]);
  }
  size_t min_a_group=100;
  for( size_t i=0; i < groups.Count(); i++ )  {
    if( groups.GetValue(i).Count() < min_a_group )
      min_a_group = groups.GetValue(i).Count();
  }
  if( min_a_group <=  1 )
    return 'P';
  if( min_a_group == 2 )  {
    vec3d t = groups.GetValue(0)[1]->t - groups.GetValue(0)[0]->t;
    t -= t.Floor<int>();
    if( t[0] == 0.5 )  {
      if( t[1] == 0.5 )  {
        if( t[2] == 0.5 )
          return 'I';
        if( t[2] == 0 )
          return 'C';
      }
      else if( t[1] == 0 && t[2] == 0.5 )
        return 'B';

    }
    else if( t[0] == 0 && t[1] == 0.5 && t[2] == 0.5 )
      return 'A';
  }
  else if( min_a_group == 3 )  {
    vec3d t1 = groups.GetValue(0)[1]->t - groups.GetValue(0)[0]->t;
    vec3d t2 = groups.GetValue(0)[2]->t - groups.GetValue(0)[0]->t;
    t1 -= t1.Floor<int>();
    t2 -= t2.Floor<int>();
    if( olx_abs(t1[0]-2./3) < 1e-6 && olx_abs(t1[1]-1./3) < 1e-6 && olx_abs(t1[2]-1./3) < 1e-6  &&
        olx_abs(t2[0]-1./3) < 1e-6 && olx_abs(t2[1]-2./3) < 1e-6 && olx_abs(t2[2]-2./3) < 1e-6 )
      return 'R';
    if( olx_abs(t1[0]-1./3) < 1e-6 && olx_abs(t1[1]-1./3) < 1e-6 && olx_abs(t1[2]-2./3) < 1e-6  &&
        olx_abs(t2[0]-2./3) < 1e-6 && olx_abs(t2[1]-2./3) < 1e-6 && olx_abs(t2[2]-1./3) < 1e-6 )
      return 'S';
    if( olx_abs(t1[0]-1./3) < 1e-6 && olx_abs(t1[1]-2./3) < 1e-6 && olx_abs(t1[2]-1./3) < 1e-6  &&
        olx_abs(t2[0]-2./3) < 1e-6 && olx_abs(t2[1]-1./3) < 1e-6 && olx_abs(t2[2]-2./3) < 1e-6 )
      return 'T';
    return EmptyString;
  }
  else if( min_a_group )
    return 'F';
  return EmptyString;
}
//..........................................................................................
