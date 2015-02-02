/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "egc.h"
#include "xmacro.h"
#include "hkl.h"
#include "symmtest.h"
#include "symmparser.h"
#include "symmlib.h"

bool NormalisevectorView(vec3d& v ) {
  double tol = 0.05;
  bool res = true;
  for( int j=0; j < 3; j++ )  {
    if( v[j] < tol )  v[j] = 0;
    else if( (1-v[j]) < tol )  v[j] = 0;
    else if( olx_abs(9./12-v[j]) < tol )  v[j] = 9./12;
    else if( olx_abs(8./12-v[j]) < tol )  v[j] = 8./12;
    else if( olx_abs(6./12-v[j]) < tol )  v[j] = 6./12;
    else if( olx_abs(4./12-v[j]) < tol )  v[j] = 4./12;
    else if( olx_abs(3./12-v[j]) < tol )  v[j] = 3./12;
    else if( olx_abs(2./12-v[j]) < tol )  v[j] = 2./12;
    else
      res = false;
  }
  return res;
}
//..............................................................................
void ElimateSGFromList(TPtrList<TSpaceGroup>& sglist, smatd& symm, vec3d_list& trans, bool present)  {
  smatd_list sgm;
  vec3d diff, nm;
  for( size_t i=0; i < sglist.Count(); i++ )  {
    bool found = false;
    sgm.Clear();
    sglist[i]->GetMatrices(sgm, mattAll);
    for( size_t j=0; j < sgm.Count(); j++ )  {
      smatd& m = sgm[j];
      if( m.r == symm.r )  {
        if( trans.Count() == 0 )  {  found = true;  break;  }
        for( size_t k=0; k < trans.Count(); k++ )  {
          for( int l=0; l < 3; l++ )  {
            if( trans[k][l] > 0.51 )  trans[k][l] = 1.0 - trans[k][l];
            while( m.t[l] < 0 )  m.t[l] += 1;
            if( m.t[l] > 0.51 )  nm[l] = 1.0 - m.t[l];
            else nm[l] = m.t[l];
            diff[l] = nm[l] - trans[k][l];
            while( diff[l] < 0 )  diff[l] += 1;
            if( diff[l] < 0.01 )  diff[l] = 0;
            else if( diff[l] > 0.99 )  diff[l] = 0;
          }
          if( diff[0] == 0 && diff[1] == 0 && diff[2] == 0 )  {
            found = true;
            break;
          }
        }
        if( found )  break;
      }
    }
    if( present )  {
      if( found )  continue;
      sglist[i] = NULL;
    }
    else  {
      if( !found )  continue;
      sglist[i] = NULL;
    }
  }
  sglist.Pack();
}
//..............................................................................
void XLibMacros::macTestSymm(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp &XApp = TXApp::GetInstance();

  TUnitCell& uc = XApp.XFile().GetLattice().GetUnitCell();
  TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
  TSymmTest st(uc);
  XApp.NewLogEntry() << NewLineSequence() << "Center of gravity: " << st.GetGravityCenter().ToString();
  double tol = Options.FindValue('e', "0.04").ToDouble();
  double confth = 75; // %
  TPtrList<TSpaceGroup> sglist;
  TTypeList<TBravaisLatticeRef> bravtypes;
  sglist.SetCapacity(TSymmLib::GetInstance().SGCount());
  TSymmLib::GetInstance().FindBravaisLattices(au, bravtypes);
  for( size_t i=0; i < TSymmLib::GetInstance().SGCount(); i++ )  {
    bool found = false;
    for( size_t j=0; j < bravtypes.Count(); j++ )  {
      if( bravtypes[j].GetA() == &TSymmLib::GetInstance().GetGroup(i).GetBravaisLattice() )  {
        found = true;
        break;
      }
    }
    if( found )
      sglist.Add(TSymmLib::GetInstance().GetGroup(i));
  }

  smatd_list presentSymm;
  TStringToList<olxstr, smatd> toTest;
  smatd a, r3, r4, r6, res;
  smatd nxx, xnx, xxn;
  smatd xr2x, r2xx, xxr2;
  vec3d trans, itrans;
  vec3d_list translations;
  a.r.I();
  xr2x.r.I();  xr2x.r[1][1] = -1;
  r2xx.r.I();  r2xx.r[0][0] = -1;
  xxr2.r.I();  xxr2.r[2][2] = -1;
  r3.r[0][1] = -1;  r3.r[1][0] = 1;  r3.r[1][1] = -1; r3.r[2][2] = 1;
  r4.r[0][1] = -1;  r4.r[1][0] = 1;  r4.r[2][2] = 1;
  r6.r[0][0] = 1;   r6.r[0][1] = -1; r6.r[1][0] = 1;  r6.r[2][2] = 1;

  nxx.r.I();   nxx.r[1][1] = nxx.r[2][2] = -1;
  xnx.r.I();   xnx.r[0][0] = xnx.r[2][2] = -1;
  xxn.r.I();   xxn.r[0][0] = xxn.r[1][1] = -1;

  toTest.Add( "Inversion", a);
  toTest.Add( "n--", nxx);
  toTest.Add( "-n-", xnx);
  toTest.Add( "--n", xxn);
  toTest.Add( "2--", r2xx);
  toTest.Add( "-2-", xr2x);
  toTest.Add( "--2", xxr2);
  toTest.Add( "-3-", r3);
  toTest.Add( "-4-", r4);
  toTest.Add( "-6-", r6);
  for( size_t i=0; i < toTest.Count(); i++ )  {
    XApp.NewLogEntry() << "Testing " << toTest[i] << "...";
    XApp.Update();
    try  {  st.TestMatrix( toTest.GetObject(i), tol );  }
    catch( const TExceptionBase& exc )  {
      XApp.NewLogEntry() << "Test failed because of " << exc.GetException()->GetError();
      continue;
    }
    if( st.GetResults().Count() > 0 )  {
      size_t ind = st.GetResults().Count()-1;
      double match = (double)(st.GetResults()[ind].Count()*400/((st.AtomCount()-2)*st.AtomCount()));
      trans = st.GetResults()[ind].Center;
      NormalisevectorView(trans);
      if( i == 0 )  {
        if( match > confth )  {
          itrans = trans;
          itrans /= 2;
          itrans *= -1;
        }
        st.Push( itrans);
      }
      res = toTest.GetObject(i);
      res *= -1;  // special treatment of inversion
      translations.Clear();
      if( match >= confth )  {
        const size_t cutoff = (size_t)(st.GetResults()[st.GetResults().Count()-1].Count()*0.75);
        for( size_t j=olx_max(0,st.GetResults().Count()-8); j < st.GetResults().Count(); j++ )  {
          if( st.GetResults()[j].Count() < cutoff )
            continue;
          trans = st.GetResults()[j].Center;
          if( i != 0 && !NormalisevectorView(trans) )  match = 0;  //delete
          if( i != 0 )
            translations.AddCopy( trans );
          XApp.NewLogEntry() << st.GetResults()[j].Count()  << '\t' << trans.ToString();
        }
      }

      if( match < confth )  {
        ElimateSGFromList(sglist, res, translations, false);
        continue;
      }

      presentSymm.AddCopy(toTest.GetObject(i));
      ElimateSGFromList(sglist, res, translations, true);

      if( match >= confth )  {
        XApp.NewLogEntry() << "Related matrix: " << TSymmParser::MatrixToSymm(res);
        XApp.NewLogEntry() << match << "%: with origin at " << trans.ToString();
      }
    }
  }
  olxstr line;
  for( size_t i=0; i < sglist.Count(); i++ )  {
    line << sglist[i]->GetName();
    if( (i+1) < sglist.Count() )
      line << ", ";
  }
  XApp.NewLogEntry() << line;
}
