#include "egc.h"
#include "xmacro.h"

#include "hkl.h"

#include "symmtest.h"
#include "symmparser.h"
#include "symmlib.h"

//..............................................................................
bool NormalisevectorView(TVectorD& v ) {
  double tol = 0.05;
  bool res = true;
  for( int j=0; j < v.Count(); j++ )  {
    if( v[j] < tol )  v[j] = 0;
    else if( (1-v[j]) < tol )  v[j] = 0;
    else if( fabs(9./12-v[j]) < tol )  v[j] = 9./12;
    else if( fabs(8./12-v[j]) < tol )  v[j] = 8./12;
    else if( fabs(6./12-v[j]) < tol )  v[j] = 6./12;
    else if( fabs(4./12-v[j]) < tol )  v[j] = 4./12;
    else if( fabs(3./12-v[j]) < tol )  v[j] = 3./12;
    else if( fabs(2./12-v[j]) < tol )  v[j] = 2./12;
    else
      res = false;
  }
  return res;
}
//..............................................................................
void ElimateSGFromList(TPtrList<TSpaceGroup>& sglist, TMatrixD& symm, TVPointDList& trans, bool present)  {
  TMatrixDList sgm;
  TVPointD diff, nm;
  for( int i=0; i < sglist.Count(); i++ )  {
    bool found = false;
    sgm.Clear();
    sglist[i]->GetMatrices(sgm, mattAll);
    for( int j=0; j < sgm.Count(); j++ )  {
      TMatrixD& m = sgm[j];
      if( m[0][0] == symm[0][0] && m[0][1] == symm[0][1] && m[0][2] == symm[0][2] &&
          m[1][0] == symm[1][0] && m[1][1] == symm[1][1] && m[1][2] == symm[1][2] &&
          m[2][0] == symm[2][0] && m[2][1] == symm[2][1] && m[2][2] == symm[2][2] )  {
        if( trans.Count() == 0 )  {  found = true;  break;  }
        for( int k=0; k < trans.Count(); k++ )  {
          for( int l=0; l < 3; l++ )  {
            if( trans[k][l] > 0.51 )  trans[k][l] = 1.0 - trans[k][l];
            while( m[l][3] < 0 )  m[l][3] += 1;
            if( m[l][3] > 0.51 )   nm[l] = 1.0 - m[l][3];
            else nm[l] = m[l][3];
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
  XApp.GetLog() << ( olxstr("\nCenter of gravity: ") << st.GetGravityCenter().ToString() << '\n');
  double tol = Options.FindValue('e', "0.04").ToDouble();
  double confth = 75; // %
  TPtrList<TSpaceGroup> sglist;
  TTypeList<TBravaisLatticeRef> bravtypes;
  sglist.SetCapacity( TSymmLib::GetInstance()->SGCount() );
  TSymmLib::GetInstance()->FindBravaisLattices(au, bravtypes);
  for( int i=0; i < TSymmLib::GetInstance()->SGCount(); i++ )  {
    bool found = false;
    for( int j=0; j < bravtypes.Count(); j++ )  {
      if( bravtypes[j].GetA() == &TSymmLib::GetInstance()->GetGroup(i).GetBravaisLattice() )  {
        found = true;
        break;
      }
    }
    if( found )
      sglist.Add( &TSymmLib::GetInstance()->GetGroup(i) );
  }

  TMatrixDList presentSymm;
  TTOStringList<olxstr, TMatrixD, TObjectStrListData<olxstr,TMatrixD> > toTest;
  TMatrixD a(3,4), r3(3,4), r4(3,4), r6(3,4), res(3,4);
  TMatrixD nxx(3,4), xnx(3,4), xxn(3,4);
  TMatrixD xr2x(3,4), r2xx(3,4), xxr2(3,4);
  TVPointD trans, itrans;
  TVPointDList translations;
  a.E();
  xr2x.E();  xr2x[1][1] = -1;
  r2xx.E();  r2xx[0][0] = -1;
  xxr2.E();  xxr2[2][2] = -1;
  r3[0][1] = -1;  r3[1][0] = 1; r3[1][1] = -1; r3[2][2] = 1;
  r4[0][1] = -1;  r4[1][0] = 1; r4[2][2] = 1;
  r6[0][0] = 1;  r6[0][1] = -1; r6[1][0] = 1; r6[2][2] = 1;

  nxx.E();   nxx[1][1] = nxx[2][2] = -1;
  xnx.E();   xnx[0][0] = xnx[2][2] = -1;
  xxn.E();   xxn[0][0] = xxn[1][1] = -1;

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
  for( int i=0; i < toTest.Count(); i++ )  {
    XApp.GetLog() << ( olxstr("Testing ") << toTest[i] << "...\n");
    XApp.Update();
    try  {  st.TestMatrix( toTest.Object(i), tol );  }
    catch( const TExceptionBase& exc )  {
      XApp.GetLog() << ( olxstr("Test failed because of ") << exc.GetException()->GetError()  << '\n');
      continue;
    }
    if( st.GetResults().Count() > 0 )  {
      int ind = st.GetResults().Count()-1;
      double match = st.GetResults()[ind].Count()*400/((st.AtomCount()-2)*st.AtomCount());
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
      res = toTest.Object(i);
      res *= -1;  // special treatment of inversion
      translations.Clear();
      if( match >= confth )  {
        int cutoff = (int)(st.GetResults()[st.GetResults().Count()-1].Count()*0.75);
        for( int j=olx_max(0,st.GetResults().Count()-8); j < st.GetResults().Count(); j++ )  {
          if( st.GetResults()[j].Count() < cutoff )
            continue;
          trans = st.GetResults()[j].Center;
          if( i != 0 && !NormalisevectorView(trans) )  match = 0;  //delete
          if( i != 0 )
            translations.AddCCopy( trans );
          XApp.GetLog() << ( olxstr(st.GetResults()[j].Count())  << '\t' << trans.ToString() ) << '\n';
        }
      }

      if( match < confth )  {
        ElimateSGFromList( sglist, res, translations, false);
        continue;
      }

      presentSymm.AddACopy( toTest.Object(i) );
      ElimateSGFromList( sglist, res, translations, true);

      if( match >= confth )  {
        XApp.GetLog() << ( olxstr("Related matrix: ") << TSymmParser::MatrixToSymm(res) << '\n');
        XApp.GetLog() << ( olxstr(match) << "%: with origin at " << trans.ToString() << '\n');
        XApp.GetLog() << '\n';
      }
    }
  }
  olxstr line;
  for( int i=0; i < sglist.Count(); i++ )  {
    line << sglist[i]->GetName();
    if( (i+1) < sglist.Count() )
      line << ", ";
  }
  XApp.GetLog() << (line << '\n');
}


