#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "xapp.h"
#include "hkl.h"
#include "ins.h"
#include "cif.h"
#include "p4p.h"
#include "crs.h"

#include "egc.h"
#include "log.h"

#include "xmacro.h"
#include "symmlib.h"
#include "unitcell.h"
#include "chemdata.h"

TXApp::TXApp(const olxstr &basedir) : TBasicApp(basedir), Library(EmptyString, this)  {
  try  {
    FAtomsInfo = new TAtomsInfo( BaseDir() + "ptablex.dat" );
    if( TSymmLib::GetInstance() == NULL )
      TEGC::AddP( new TSymmLib(BaseDir() + "symmlib.xld") );
  }
  catch( const TIOExceptionBase &exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  FXFile = new TXFile(FAtomsInfo);

  DefineState( psFileLoaded, "Loaded file is expected");
  DefineState( psCheckFileTypeIns, "INS file is expected");
  DefineState( psCheckFileTypeCif, "CIF file is expected");
  DefineState( psCheckFileTypeP4P, "P4P file is expected");
  DefineState( psCheckFileTypeCRS, "CRS file is expected");
  
  CifTemplatesDir = BaseDir() + "etc/CIF/";

  XLibMacros::Export(Library);
}
//..............................................................................
TXApp::~TXApp()  {
  delete FXFile;
  delete FAtomsInfo;
}
//..............................................................................
const olxstr& TXApp::LocateHklFile()  {
  if( XFile().GetLastLoader() == NULL )  return EmptyString;

  olxstr &HklFN = TEGC::New<olxstr>( XFile().GetLastLoader()->GetHKLSource() );
  if( TEFile::FileExists( HklFN ) )  return HklFN;
  HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkl");
  if( TEFile::FileExists( HklFN ) )  return HklFN;
  HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "raw");
  if( TEFile::FileExists(HklFN) )  {
    THklFile Hkl;
    Hkl.LoadFromFile(HklFN);
    HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkl");
    for( int i=0; i < Hkl.RefCount(); i++ )  {
      Hkl[i].SetI( (double)Round(Hkl[i].GetI())/100.0 );
      Hkl[i].SetS( (double)Round(Hkl[i].GetS())/100.0 );
    }
    Hkl.SaveToFile( HklFN );
    GetLog().Info("The scaled hkl file is prepared");
    return HklFN;
  }
  else  {  // check for stoe format
    HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkl");
    olxstr HkcFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkc");
    if( TEFile::FileExists( HkcFN ) )  {
      TEFile::Copy( HkcFN, HklFN );
      return HklFN;
    }
  }
  return EmptyString;
}
//..............................................................................
bool TXApp::CheckProgramState(unsigned int specialCheck)  {
 if( specialCheck & psFileLoaded )
   return XFile().GetLastLoader() != NULL;
 else if( specialCheck & psCheckFileTypeIns )
   return (XFile().GetLastLoader() == NULL) ? false : EsdlInstanceOf(*XFile().GetLastLoader(), TIns);
 else if( specialCheck & psCheckFileTypeP4P )
   return (XFile().GetLastLoader() == NULL) ? false : EsdlInstanceOf(*XFile().GetLastLoader(), TP4PFile);
 else if( specialCheck & psCheckFileTypeCRS )
   return (XFile().GetLastLoader() == NULL) ? false : EsdlInstanceOf(*XFile().GetLastLoader(), TCRSFile);
 else if( specialCheck & psCheckFileTypeCif )
   return (XFile().GetLastLoader() == NULL) ? false : EsdlInstanceOf(*XFile().GetLastLoader(), TCif);
  throw TFunctionFailedException(__OlxSourceInfo, "undefined state");
}
//..............................................................................
void TXApp::CalcSF(const TRefList& refs, TArrayList<TEComplex<double> >& F)  {
  // initialise newly created atoms
  XFile().UpdateAsymmUnit();
  TAsymmUnit& au = XFile().GetAsymmUnit();
  const mat3d& hkl2c = au.GetHklToCartesian();
  // space group matrix list
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(au);
  if( sg == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "unknown spacegroup");
  smatd_list ml, allm;
  sg->GetMatrices(ml, mattAll);

  evecd quad(6);
  const static double EQ_PI = 8*QRT(M_PI);
  const static double T_PI = 2*M_PI;
  const static double TQ_PI = 2*QRT(M_PI);
  static const double ev_angstrom  = 6626.0755 * 2.99792458 / 1.60217733;
  double WaveLength = 0.71073;

  // the thermal ellipsoid scaling factors
  double BM[6] = {hkl2c[0].Length(), hkl2c[1].Length(), hkl2c[2].Length(), 0, 0, 0};
  BM[3] = 2*BM[1]*BM[2];
  BM[4] = 2*BM[0]*BM[2];
  BM[5] = 2*BM[0]*BM[1];
  BM[0] *= BM[0];
  BM[1] *= BM[1];
  BM[2] *= BM[2];
  
  cm_Element* carb = XElementLib::FindBySymbol("C");
  compd carb_fpfdp = carb->CalcFpFdp(ev_angstrom/WaveLength);
  TPtrList<TBasicAtomInfo> bais;
  TPtrList<TCAtom> alist;
  double *Ucifs = new double[6*au.AtomCount() + 1];
  TTypeList< AnAssociation3<cm_Element*, compd, compd> > scatterers;
  for( int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  continue;
    int ind = bais.IndexOf( &ca.GetAtomInfo() );
    if( ind == -1 )  {
      if( ca.GetAtomInfo() == iDeuteriumIndex ) // treat D as H
        scatterers.AddNew<cm_Element*,compd,compd>(XElementLib::FindBySymbol("H"), 0, 0);
      else 
        scatterers.AddNew<cm_Element*,compd,compd>(XElementLib::FindBySymbol(ca.GetAtomInfo().GetSymbol()), 0, 0);
     
      if( scatterers.Last().GetA() == NULL ) {
        delete [] Ucifs;
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("could not locate scatterer: ") << ca.GetAtomInfo().GetSymbol() );
      }
      bais.Add( &ca.GetAtomInfo() );
      scatterers.Last().C() = scatterers.Last().GetA()->CalcFpFdp(ev_angstrom/WaveLength);
      scatterers.Last().C() -= scatterers.Last().GetA()->z;
      ind = scatterers.Count() - 1;
    }
    ca.SetTag(ind);
    ind = alist.Count()*6;
    alist.Add(&ca); 
    TEllipsoid* elp = ca.GetEllipsoid();
    if( elp != NULL )  {
      elp->GetQuad(quad);  // default is Ucart
      au.UcartToUcif(quad);
      for( int k=0; k < 6; k++ )
        Ucifs[ind+k] = -TQ_PI*quad[k]*BM[k];
    }
    else  {
      //if( ca.GetAtomInfo() == iHydrogenIndex && ca.GetUisoVar() < 0 )  {
      //  int ai = ca.GetAfixAtomId();
      //  if( ai == -1 )  {
      //    delete [] Ucifs;
      //    throw TFunctionFailedException(__OlxSourceInfo, "bad Uiso");
      //  }
      //  Ucifs[ind] = fabs(au.GetAtom(ai).GetUiso()*ca.GetUisoVar());
      //}
      //else
      Ucifs[ind] = ca.GetUiso();
      //Ucifs[ind] *= Ucifs[ind]; 
      Ucifs[ind] *= -EQ_PI;
    }
  }
  
  vec3d hkl, crd;
  for( int i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    ref.MulHkl(hkl, hkl2c);
    const double d_s2 = hkl.QLength()*0.25;
    for( int j=0; j < scatterers.Count(); j++)  {
      scatterers[j].B() = scatterers[j].GetA()->gaussians->calc_sq(d_s2);
      //scatterers[j].B() += scatterers[j].GetC();
    }
    compd ir;
    for( int j=0; j < alist.Count(); j++ )  {
      crd = alist[j]->ccrd();
      compd l;
      for( int k=0; k < ml.Count(); k++ )  {
        const smatd& mt = ml[k];
        ref.MulHkl(hkl, mt);
        double tv =  T_PI*hkl.DotProd(mt.t+crd);  // scattering vector + phase shift
        double ca, sa;
        SinCos(tv, &sa, &ca);
        if( alist[j]->GetEllipsoid() != NULL )  {
          double* Q = &Ucifs[j*6];  // pick up the correct ellipsoid
          double B = (Q[0]*hkl[0]*hkl[0] + Q[1]*hkl[1]*hkl[1] + Q[2]*hkl[2]*hkl[2] + 
            Q[3]*hkl[1]*hkl[2] + Q[4]*hkl[0]*hkl[2] + Q[5]*hkl[0]*hkl[1]);
          B = exp( B );
          l.Re() += B*ca;
          l.Im() += B*sa;
        }
        else  {
          l.Re() += ca;
          l.Im() += sa;
        }
      }
      compd scv = scatterers[ alist[j]->GetTag() ].GetB();
      if( alist[j]->GetEllipsoid() == NULL )
        scv *= exp( Ucifs[j*6]*d_s2 );
      
      scv *= alist[j]->GetOccp();
      scv *= l;
      ir += scv;
    }
    F[i] = ir;
  }
  delete [] Ucifs;
  au.InitAtomIds();
}
//..............................................................................
void TXApp::NameHydrogens(TSAtom& SA, TUndoData* ud, bool CheckLabel)  {
  TNameUndo* nu = static_cast<TNameUndo*>(ud);
  olxstr Labl;
  int hcount=0, allH = 0, processedH = 0;
  olxstr Name( SA.GetLabel().SubStringFrom( SA.GetAtomInfo().GetSymbol().Length() ) );
  TCAtom* CA;
  for( int i=0; i < SA.NodeCount(); i++ )  {
    const TSAtom& sa = SA.Node(i);
    if( !sa.IsDeleted() && sa.GetAtomInfo() == iHydrogenIndex )
      allH ++;
  }

  for( int i=0; i < SA.NodeCount(); i++ )  {
    TSAtom& SA1 = SA.Node(i);
    if( SA1.IsDeleted() )  continue;
    if( SA1.GetAtomInfo().GetIndex() == iHydrogenIndex )  {
      Labl = SA1.GetAtomInfo().GetSymbol() + Name;
      // the suffix matters only for multiple hydrogen atoms attached
      if( allH > 1 )  {
        if( Labl.Length() >= 4 )  Labl.SetLength(3);
        Labl << (char)('a' + hcount);
      }
      if(  CheckLabel )  {
        while( (CA = XFile().GetAsymmUnit().FindCAtom(Labl)) != NULL )  {
          if( CA == &SA1.CAtom() || CA->IsDeleted() )  break;
          hcount++;
          Labl = SA1.GetAtomInfo().GetSymbol()+Name;
          if( Labl.Length() >= 4 )  Labl.SetLength(3);
          Labl << (char)('a' + hcount);
        }
      }
      if( SA1.GetLabel() != Labl )  {
        if( nu != NULL )  nu->AddAtom(SA1, SA1.GetLabel() );
        SA1.CAtom().Label() = Labl;
      }
      hcount++;
      processedH++;
      if( processedH >= allH )  break;
    }
  }
}
//..............................................................................
void TXApp::undoName(TUndoData *data)  {
  TNameUndo *undo = static_cast<TNameUndo*>(data);
  TSAtomPList sal;
  for( int i=0; i < undo->AtomCount(); i++ )  {
    TBasicAtomInfo* bai = FAtomsInfo->FindAtomInfoEx( undo->GetLabel(i) );
    if( undo->GetAtom(i).GetAtomInfo() != *bai )
      sal.Add( &undo->GetAtom(i) );
    undo->GetAtom(i).SetLabel( undo->GetLabel(i) );
  }
}
//..............................................................................
TUndoData* TXApp::FixHL()  {
  TNameUndo *undo = new TNameUndo( new TUndoActionImpl<TXApp>(this, &TXApp::undoName));
  for( int i=0; i < XFile().GetLattice().AtomCount(); i++ )  {
    TSAtom& sa = XFile().GetLattice().GetAtom(i);
    if( sa.IsDeleted() || (sa.GetAtomInfo() == iQPeakIndex) || (sa.GetAtomInfo() == iHydrogenIndex)  )
      continue;
    NameHydrogens(sa, undo, false);
  }
  for( int i=0; i < XFile().GetLattice().AtomCount(); i++ )  {
    TSAtom& sa = XFile().GetLattice().GetAtom(i);
    if( sa.IsDeleted() || (sa.GetAtomInfo() == iQPeakIndex) || (sa.GetAtomInfo() == iHydrogenIndex)  )
      continue;
    NameHydrogens(sa, undo, true);
  }
  return undo;
}
//..............................................................................
bool RingsEq(const TSAtomPList& r1, const TSAtomPList& r2 )  {
  for( int i=0; i < r1.Count(); i++ )  {
    bool found = false;
    for( int j=0; j < r2.Count(); j++ )  {
      if( r2[j]->GetLatId() == r1[i]->GetLatId() )  {
        found = true;
        break;
      }
    }
    if( !found )
      return false;
  }
  return true;
}
void TXApp::FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings)  {
  TPtrList<TBasicAtomInfo> ring;
  // external ring connectivity
  TTypeList< TPtrList<TBasicAtomInfo> > extRing;
  TStrList toks;
  olxstr symbol, count;
  for( int i=0; i < Condition.Length(); i++ )  {
    if( Condition[i] <= 'Z' && Condition[i] >= 'A' )  {
      if( !symbol.IsEmpty() )  {
        if( !count.IsEmpty() )  {
          int c = count.ToInt();
          for( int j=0; j < c; j++ )
            toks.Add( symbol );
        }
        else
          toks.Add( symbol );
      }
      symbol = Condition[i];
      count = EmptyString;
    }
    else if( Condition[i] <= 'z' && Condition[i] >= 'a' )  {
      symbol << Condition[i];
      count  = EmptyString;
    }
    else if( Condition[i] <= '9' && Condition[i] >= '0' )  {
      count << Condition[i];
    }
  }
  if( !symbol.IsEmpty() )  {
    if( !count.IsEmpty() )  {
      int c = count.ToInt();
      for( int j=0; j < c; j++ )
        toks.Add( symbol );
    }
    else
      toks.Add( symbol );
  }

  if( toks.Count() < 3 )  return;

  for( int i=0; i < toks.Count(); i++ )  {
    TBasicAtomInfo* bai = FAtomsInfo->FindAtomInfoBySymbol( toks[i] );
    if( bai == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Unknown element: ") << toks.String(i) );
    ring.Add( bai );
  }

  for( int i=0; i < XFile().GetLattice().FragmentCount(); i++ )  {
    XFile().GetLattice().GetFragment(i).FindRings(ring, rings);
  }

  for( int i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )  continue;
    for( int j= i+1; j < rings.Count(); j++ ) {
      if( rings.IsNull(j) )  continue;
      if( RingsEq( rings[i], rings[j]) )
        rings.NullItem(j);
    }
  }
  rings.Pack();
}
//..............................................................................
bool TXApp::FindSAtoms(const olxstr& condition, TSAtomPList& res)  {
  if( !condition.IsEmpty() )  {
    TAtomReference ar(condition);      
    TCAtomGroup ag;
    int atomAGroup;
    ar.Expand(XFile().GetAsymmUnit(), ag, EmptyString, atomAGroup);
    res.SetCapacity( ag.Count() );
    for( int i=0; i < XFile().GetAsymmUnit().AtomCount(); i++ )
      XFile().GetAsymmUnit().GetAtom(i).SetTag(0);
    for( int i=0; i < ag.Count(); i++ )
      ag[i].GetAtom()->SetTag(1);
    for( int i=0; i < XFile().GetLattice().AtomCount(); i++ )  {
      TSAtom* sa = &XFile().GetLattice().GetAtom(i);
      if( sa->CAtom().GetTag() == 1 )
        res.Add( sa );
    }
  }
  else  {
    res.SetCapacity( XFile().GetLattice().AtomCount() );
    for( int i=0; i < XFile().GetLattice().AtomCount(); i++ )
      res.Add( &XFile().GetLattice().GetAtom(i) );
  }
  return !res.IsEmpty();
}
//..............................................................................
short TXApp::CalcVoid(TArray3D<short>& map, double extraR, short val, long* structurePoints, vec3d& voidCenter)  {
  short*** D = map.Data;
  XFile().GetLattice().GetUnitCell().BuildStructureMap(map, extraR, val, structurePoints);
  int mapX = map.Length1(), mapY = map.Length2(), mapZ = map.Length3();
  int level = 0, MaxLevel = 0;
  while( true )  {
    bool levelUsed = false;
    for(int i=0; i < mapX; i++ )  {
      for(int j=0; j < mapY; j++ )  {
        for(int k=0; k < mapZ; k++ )  {
          // neigbouring points analysis
          bool inside = true;
          for(int ii = -1; ii <= 1; ii++)  {
            for(int jj = -1; jj <= 1; jj++)  {
              for(int kk = -1; kk <= 1; kk++)  {
                int iind = i+ii, jind = j+jj, kind = k+kk;
                // index "rotation" step
                if( iind < 0 )  iind += mapX;
                if( jind < 0 )  jind += mapY;
                if( kind < 0 )  kind += mapZ;
                if( iind >= mapX )  iind -= mapX;
                if( jind >= mapY )  jind -= mapY;
                if( kind >= mapZ )  kind -= mapZ;
                // main condition
                if( D[iind][jind][kind] < level )  {
                  inside = false;
                  break;
                }
              }
              if( !inside )  break;
            }
            if( !inside )  break;
          }
          if( inside )  {
            D[i][j][k] = level + 1;
            levelUsed = true;
            MaxLevel = level;
            voidCenter[0] = (double)i/mapX;
            voidCenter[1] = (double)j/mapY;
            voidCenter[2] = (double)k/mapZ;
          }
        }
      }
    }
    if( !levelUsed ) // reached the last point
      break;
    level ++;
  }
  return MaxLevel;
}
//..............................................................................
