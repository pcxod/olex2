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
#include "maputil.h"

TXApp::TXApp(const olxstr &basedir, ASelectionOwner* selOwner) : 
    SelectionOwner(selOwner), TBasicApp(basedir), Library(EmptyString, this)  {
  try  {
    FAtomsInfo = new TAtomsInfo( BaseDir() + "ptablex.dat" );
    if( TSymmLib::GetInstance() == NULL )
      TEGC::AddP( new TSymmLib(BaseDir() + "symmlib.xld") );
  }
  catch( const TIOExceptionBase &exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  FXFile = new TXFile;

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
  if( !XFile().HasLastLoader() )  return EmptyString;

  olxstr &HklFN = TEGC::New<olxstr>( XFile().GetRM().GetHKLSource() );
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
   return XFile().HasLastLoader();
 else if( specialCheck & psCheckFileTypeIns )
   return (!XFile().HasLastLoader()) ? false : (
   EsdlInstanceOf(*XFile().LastLoader(), TIns) || 
   XFile().LastLoader()->IsNative()
   );
 else if( specialCheck & psCheckFileTypeP4P )
   return (!XFile().HasLastLoader()) ? false : EsdlInstanceOf(*XFile().LastLoader(), TP4PFile);
 else if( specialCheck & psCheckFileTypeCRS )
   return (!XFile().HasLastLoader()) ? false : EsdlInstanceOf(*XFile().LastLoader(), TCRSFile);
 else if( specialCheck & psCheckFileTypeCif )
   return (!XFile().HasLastLoader()) ? false : EsdlInstanceOf(*XFile().LastLoader(), TCif);
  throw TFunctionFailedException(__OlxSourceInfo, "undefined state");
}
//..............................................................................
void TXApp::CalcSF(const TRefList& refs, TArrayList<TEComplex<double> >& F)  {
  // initialise newly created atoms
  XFile().UpdateAsymmUnit();
  TAsymmUnit& au = XFile().GetAsymmUnit();
  const mat3d& hkl2c = au.GetHklToCartesian();
  // space group matrix list
  TSpaceGroup* sg = NULL;
  try  { sg = &XFile().GetLastLoaderSG();  }
  catch(...)  {
    throw TFunctionFailedException(__OlxSourceInfo, "unknown spacegroup");
  }
  smatd_list ml, allm;
  sg->GetMatrices(ml, mattAll);

  evecd quad(6);
  const static double EQ_PI = 8*M_PI*M_PI;
  const static double T_PI = 2*M_PI;
  const static double TQ_PI = 2.0*M_PI*M_PI;
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
      Ucifs[ind] = ca.GetUiso()*ca.GetUiso();
      Ucifs[ind] *= -EQ_PI;
    }
  }
  
  const int a_cnt = alist.Count(),
            m_cnt = ml.Count();
  for( int i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    const double d_s2 = ref.ToCart(hkl2c).QLength()*0.25;
    for( int j=0; j < scatterers.Count(); j++)  {
      scatterers[j].B() = scatterers[j].GetA()->gaussians->calc_sq(d_s2);
      scatterers[j].B() += scatterers[j].GetC();
    }
    compd ir;
    for( int j=0; j < a_cnt; j++ )  {
      compd l;
      for( int k=0; k < m_cnt; k++ )  {
        // it must not be the transposed form here!!!
        const vec3d hkl = ml[k].r*ref.GetHkl();
        double tv =  T_PI*hkl.DotProd(ml[k].t + alist[j]->ccrd());  // scattering vector + phase shift
        double ca, sa;
        SinCos(tv, &sa, &ca);
        if( alist[j]->GetEllipsoid() != NULL )  {
          const double* Q = &Ucifs[j*6];  // pick up the correct ellipsoid
          double B = (hkl[0]*(Q[0]*hkl[0]+Q[4]*hkl[2]+Q[5]*hkl[1]) + 
                      hkl[1]*(Q[1]*hkl[1]+Q[3]*hkl[2]) + 
                      hkl[2]*(Q[2]*hkl[2]) );
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
      
      scv *= alist[j]->GetOccu();
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
  olxstr Name( SA.GetLabel().StartsFromi(SA.GetAtomInfo().GetSymbol()) ? 
    SA.GetLabel().SubStringFrom( SA.GetAtomInfo().GetSymbol().Length() )
    :
    EmptyString
  );
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
void TXApp::RingContentFromStr(const olxstr& Condition, TPtrList<TBasicAtomInfo>& ringDesc)  {
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
    ringDesc.Add( bai );
  }
}
void TXApp::FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings)  {
  TPtrList<TBasicAtomInfo> ring;
  // external ring connectivity
  TTypeList< TPtrList<TBasicAtomInfo> > extRing;
  RingContentFromStr(Condition, ring);
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
bool TXApp::FindSAtoms(const olxstr& condition, TSAtomPList& res, bool ReturnAll, bool ClearSelection)  {
  if( SelectionOwner != NULL )
    SelectionOwner->SetDoClearSelection(ClearSelection);
  TCAtomGroup ag;
  if( condition.IsEmpty() )  {  // try the selection first
    if( SelectionOwner != NULL )
      SelectionOwner->ExpandSelection(ag);
  }
  if( !condition.IsEmpty() )  {
    TStrList toks(condition, ' ');
    TLattice& latt = XFile().GetLattice();
    for( int i=0; i < toks.Count(); i++ )  {
      if( toks[i].StartsFrom("#s") )  {  // TSAtom.LattId
        int lat_id = toks[i].SubStringFrom(2).ToInt();
        if( lat_id < 0 || lat_id >= latt.AtomCount() )
          throw TInvalidArgumentException(__OlxSourceInfo, "satom id");
        res.Add( &latt.GetAtom(lat_id) );
        toks.Delete(i);
        i--;
      }
      else if( toks[i].IsNumber() )  {  // should not be here, but the parser will choke on it
        toks.Delete(i);
        i--;
      }
    }
    TAtomReference ar(toks.Text(' '), SelectionOwner);      
    int atomAGroup;
    ar.Expand(XFile().GetRM(), ag, EmptyString, atomAGroup);
  }
  else if( ag.IsEmpty() && ReturnAll ) {
    res.SetCapacity( XFile().GetLattice().AtomCount() );
    for( int i=0; i < XFile().GetLattice().AtomCount(); i++ )
      res.Add( &XFile().GetLattice().GetAtom(i) );
  }
  if( !ag.IsEmpty() )  {
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
  return !res.IsEmpty();
}
//..............................................................................
short TXApp::CalcVoid(TArray3D<short>& map, double extraR, short val, size_t* structurePoints, 
                      vec3d& voidCenter, TPSTypeList<TBasicAtomInfo*, double>* radii)  {
  XFile().GetLattice().GetUnitCell().BuildStructureMap(map, extraR, val, structurePoints, radii);
  return MapUtil::AnalyseVoids(map.Data, map.Length1(), map.Length2(), map.Length3(), voidCenter);
}
//..............................................................................
void TXApp::ProcessRingAfix(TSAtomPList& ring, int afix, bool pivot_last)  {
  olxstr info("Processing");
  int pivot = (pivot_last ? ring.Count()-1 : 0);
  for( int i=0; i < ring.Count(); i++ )
    info << ' ' << ring[i]->GetLabel();
  TBasicApp::GetLog() << (info << ". Chosen pivot atom is " << ring[pivot]->GetLabel() << '\n');
  if( ring[pivot]->CAtom().GetDependentAfixGroup() != NULL )
    ring[pivot]->CAtom().GetDependentAfixGroup()->Clear();
  TAfixGroup& ag = FXFile->GetRM().AfixGroups.New( &ring[pivot]->CAtom(), afix);
  for( int i=0; i < ring.Count(); i++ )  {  
    if( i == pivot )  continue;  // do not want to delete just created!
    TCAtom& ca = ring[i]->CAtom();
    if( ca.GetDependentAfixGroup() != NULL && ca.GetDependentAfixGroup()->GetAfix() == afix )  // if used in case to change order
      ca.GetDependentAfixGroup()->Clear();
  }
  if( pivot_last )  {
    for( int i=ring.Count()-2; i >= 0; i-- )
      ag.AddDependent( ring[i]->CAtom() );
  }
  else  {
    for( int i=1; i < ring.Count(); i++ )
      ag.AddDependent( ring[i]->CAtom() );
  }

}
//..............................................................................
void TXApp::AutoAfixRings(int afix, TSAtom* sa, bool TryPyridine)  {
  int m = TAfixGroup::GetM(afix), n = TAfixGroup::GetN(afix);
  if( TAfixGroup::IsFitted(afix) && ( n == 6 || n == 9) )  {  // special case
    if( sa == NULL )  {
      TTypeList< TSAtomPList > rings;
      try  {  
        if( m == 6 || m == 7 )  {
          FindRings("C6", rings);  
          if( TryPyridine )  
            FindRings("NC5", rings);  
        }
        else if( m == 5 || m == 10 ) // Cp or Cp*
          FindRings("C5", rings);  
        else if( m == 11 )
          FindRings("C10", rings);  
      }
      catch( const TExceptionBase& exc )  {  throw TFunctionFailedException(__OlxSourceInfo, exc);  }
      TNetwork::RingInfo ri;
      for( int i=0; i < rings.Count(); i++ )  {
        if( m != 11 && !TNetwork::IsRingRegular(rings[i]) )  continue;
        // find the pivot (with heaviest atom attached)
        TNetwork::AnalyseRing( rings[i], ri.Clear() );
        if( ri.HasAfix || ri.HeaviestSubsIndex == -1 )  continue;
        if( m != 10 && ri.Substituted.Count() > 1 )  continue;
        if( m == 10 && ri.Substituted.Count() != 5 )  continue; // Cp*
        int shift = (m == 10 ? 0 : ri.HeaviestSubsIndex+1); // do not allign to pivot for Cp* 
        rings[i].ShiftL(shift);  // pivot is the last one now
        if( m == 11 )  {  // validate and rearrange to figure of 8
          if( ri.Alpha.IndexOf( shift -1 ) == -1 ) continue;
          if( ri.Ternary.Count() != 2 )  continue;
          if( ri.Ternary.IndexOf( shift >= 2 ? shift-2 : rings[i].Count()-shift ) != -1 )  { // counter-clockwise direction
            for( int j=0; j < (rings[i].Count()-1)/2; j++ )  {
              TSAtom* a = rings[i][j];
              rings[i][j] = rings[i][rings[i].Count()-j-2];
              rings[i][rings[i].Count()-j-2] = a;
            }
          }
          rings[i].Swap(0, 4);
          rings[i].Swap(1, 3);
        }
        else if( m == 10 )  {  // Cp*
          if( !ri.IsSingleCSubstituted() )  continue;
          for( int j=0; j < ri.Substituents.Count(); j++ )
            rings[i].Add(ri.Substituents[j][0] );
        }
        ProcessRingAfix(rings[i], afix, m != 10);
      }
    }
    else  {  // sa != NULL
      TPtrList<TBasicAtomInfo> ring;
      TTypeList< TSAtomPList > rings;
      if( sa->GetAtomInfo() != iCarbonIndex )
        ring.Add( &sa->GetAtomInfo() );
      if( m == 6 || m == 7)
        RingContentFromStr( ring.IsEmpty() ? "C6" :"C5", ring);
      else if( m == 5 )
        RingContentFromStr(ring.IsEmpty() ? "C5" :"C4", ring);
      else if( m == 10 )
        RingContentFromStr(ring.IsEmpty() ? "C5" :"C4", ring);
      else if( m == 11 ) 
        RingContentFromStr(ring.IsEmpty() ? "C10" :"C9", ring);

      sa->GetNetwork().FindAtomRings(*sa, ring, rings);
      if( rings.Count() == 0 )  {
        GetLog() << "no suitable rings have been found\n";
        return;
      }
      else if( rings.Count() > 1 )  {
        GetLog() << "the atom is shared by several rings\n";
        return;
      }
      TNetwork::RingInfo ri;
      TNetwork::AnalyseRing( rings[0], ri );
      if( m == 11 )  {  // need to rearrage the ring to fit shelxl requirements as fihure of 8
        if( ri.Alpha.IndexOf( rings[0].Count() -1 ) == -1 )  {
          GetLog() << "the alpha substituted atom is expected\n";
          return;
        }
        if( ri.Ternary.Count() != 2 )  {
          GetLog() << "naphtalene ring should have two ternary atoms\n";
          return;
        }
        if( ri.Ternary.IndexOf(rings[0].Count()-2) != -1 )  { // countr-clockwise direction to revert
          for( int i=0; i < (rings[0].Count()-1)/2; i++ )  {
            TSAtom* a = rings[0][i];
            rings[0][i] = rings[0][rings[0].Count()-i-2];
            rings[0][rings[0].Count()-i-2] = a;
          }
        }
        rings[0].Swap(0, 4);
        rings[0].Swap(1, 3);
      }
      else if( m == 10 )  {
        if( !ri.IsSingleCSubstituted() )  {
          TBasicApp::GetLog() << "Could not locate Cp* ring\n";
          return;
        }
        for( int j=0; j < ri.Substituents.Count(); j++ )
          rings[0].Add(ri.Substituents[j][0] );
      }
      else  {
        if( ri.Substituents.Last().Count() == 0 )  {
          TBasicApp::GetLog() << "A substituted atom is expected\n";
          return;
        }
      }
      if( ri.Substituted.Count() > 1 && m != 10 )  
        TBasicApp::GetLog() << "The selected ring has more than one substituent\n";
      ProcessRingAfix(rings[0], afix, m!=10);
    }
  }
}
//..............................................................................
void TXApp::SetAtomUiso(TSAtom& sa, double val) {
  RefinementModel& rm = *sa.CAtom().GetParent()->GetRefMod();
  if( sa.CAtom().GetEllipsoid() == NULL )  {
    if( val <= -0.5 )  {
      int ni = -1;
      for( int i=0; i < sa.NodeCount(); i++ ) {
        TSAtom& nd = sa.Node(i);
        if( nd.IsDeleted() || nd.GetAtomInfo() == iQPeakIndex )
          continue;
        if( ni != -1 )  {  // to many bonds
          ni = -1;
          break;
        }
        ni = i;
      }
      // make sure that there is only one atom in the envi and it has proper Uiso
      if( ni != -1 && sa.Node(ni).CAtom().GetUisoOwner() == NULL )  {
        rm.Vars.FreeAtomParam(sa.CAtom(), var_name_Uiso);
        sa.CAtom().SetUisoOwner(&sa.Node(ni).CAtom());
        sa.CAtom().SetUisoScale(olx_abs(val));
        sa.CAtom().SetUiso(olx_abs(val)*sa.Node(ni).CAtom().GetUiso());
      }
    }
    else
      rm.Vars.SetAtomParam(sa.CAtom(), var_name_Uiso, val);
  }
}
//..............................................................................
void TXApp::GetSymm(smatd_list& ml) const {
  if( !FXFile->HasLastLoader() )
    throw TFunctionFailedException(__OlxSourceInfo, "a loaded file is expected");
  const TUnitCell& uc = FXFile->GetUnitCell();
  ml.SetCapacity( ml.Count() + uc.MatrixCount() );
  for( int i=0; i < uc.MatrixCount(); i++ )
    ml.AddCCopy( uc.GetMatrix(i) );
}
//..............................................................................
void TXApp::ToDataItem(TDataItem& item) const  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TXApp::FromDataItem(TDataItem& item)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
