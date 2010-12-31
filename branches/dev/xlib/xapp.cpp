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
#include "vcov.h"

TXApp::TXApp(const olxstr &basedir, bool) : TBasicApp(basedir), Library(EmptyString, this)  {}
//..............................................................................
TXApp::TXApp(const olxstr &basedir, ASObjectProvider* objectProvider, ASelectionOwner* selOwner) :
  TBasicApp(basedir), Library(EmptyString, this)
{
  Init(objectProvider, selOwner);
}
//..............................................................................
void TXApp::Init(ASObjectProvider* objectProvider, ASelectionOwner* selOwner)  {
  SelectionOwner = selOwner;
  try  {
    if( !TSymmLib::IsInitialised() )
      TEGC::AddP(new TSymmLib(GetBaseDir() + "symmlib.xld"));
  }
  catch( const TIOException& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  FXFile = new TXFile(*(objectProvider == NULL ? new SObjectProvider : objectProvider));

  DefineState(psFileLoaded, "Loaded file is expected");
  DefineState(psCheckFileTypeIns, "INS file is expected");
  DefineState(psCheckFileTypeCif, "CIF file is expected");
  DefineState(psCheckFileTypeP4P, "P4P file is expected");
  DefineState(psCheckFileTypeCRS, "CRS file is expected");
  
  CifTemplatesDir = GetBaseDir() + "etc/CIF/";

  XLibMacros::Export(Library);
}
//..............................................................................
TXApp::~TXApp()  {
  delete FXFile;
}
//..............................................................................
olxstr TXApp::LocateHklFile()  {
  if( !XFile().HasLastLoader() )  return EmptyString;
  olxstr HklFN = XFile().GetRM().GetHKLSource();
  if( TEFile::Existsi(olxstr(HklFN), HklFN) )  
    return HklFN;
  HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkl");
  if( TEFile::Existsi( olxstr(HklFN), HklFN ) )  
    return HklFN;
  HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "raw");
  if( TEFile::Existsi(olxstr(HklFN), HklFN) )  {
    THklFile Hkl;
    Hkl.LoadFromFile(HklFN);
    HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkl");
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      Hkl[i].SetI((double)olx_round(Hkl[i].GetI())/100.0 );
      Hkl[i].SetS((double)olx_round(Hkl[i].GetS())/100.0 );
    }
    Hkl.SaveToFile( HklFN );
    NewLogEntry(logInfo) << "The scaled hkl file is prepared";
    return HklFN;
  }
  else  {  // check for stoe format
    HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkl");
    olxstr HkcFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkc");
    if( TEFile::Existsi(olxstr(HkcFN), HkcFN) )  {
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
  ElementPList bais;
  TPtrList<TCAtom> alist;
  double *Ucifs = new double[6*au.AtomCount() + 1];
  TTypeList< AnAssociation3<const cm_Element*, compd, compd> > scatterers;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() || ca.GetType() == iQPeakZ )  continue;
    size_t ind = bais.IndexOf(ca.GetType());
    if( ind == InvalidIndex )  {
      scatterers.AddNew<const cm_Element*,compd,compd>(&ca.GetType(), 0, 0);
      bais.Add(ca.GetType());
      scatterers.GetLast().C() = scatterers.GetLast().GetA()->CalcFpFdp(ev_angstrom/WaveLength);
      scatterers.GetLast().C() -= scatterers.GetLast().GetA()->z;
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
    else
      Ucifs[ind] = -EQ_PI*ca.GetUiso();
  }
  
  const size_t a_cnt = alist.Count(),
            m_cnt = ml.Count();
  for( size_t i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    const double d_s2 = ref.ToCart(hkl2c).QLength()*0.25;
    for( size_t j=0; j < scatterers.Count(); j++)  {
      scatterers[j].B() = scatterers[j].GetA()->gaussians->calc_sq(d_s2);
      scatterers[j].B() += scatterers[j].GetC();
    }
    compd ir;
    for( size_t j=0; j < a_cnt; j++ )  {
      compd l;
      for( size_t k=0; k < m_cnt; k++ )  {
        const vec3d hkl = ref.GetHkl()*ml[k].r;
        double tv =  T_PI*(alist[j]->ccrd().DotProd(hkl) + ml[k].t.DotProd(ref.GetHkl()));  // scattering vector + phase shift
        double ca, sa;
        olx_sincos(tv, &sa, &ca);
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
}
//..............................................................................
void TXApp::NameHydrogens(TSAtom& SA, TUndoData* ud, bool CheckLabel)  {
  TNameUndo* nu = static_cast<TNameUndo*>(ud);
  int lablInc = 0; 
  olxdict<int,TSAtomPList,TPrimitiveComparator> parts;
  olxstr Name( 
    SA.GetLabel().StartsFromi(SA.GetType().symbol) ? 
      SA.GetLabel().SubStringFrom(SA.GetType().symbol.Length())
    :
      EmptyString
  );
  // is H atom under consideration?
  if( SA.GetType() == iHydrogenZ && SA.GetTag() == -2 )
    parts.Add(SA.CAtom().GetPart()).Add(SA);
  for( size_t i=0; i < SA.NodeCount(); i++ )  {
    TSAtom& sa = SA.Node(i);
    if( sa.GetType() == iHydrogenZ && sa.GetTag() == -2 && sa.IsAUAtom() )
      parts.Add(sa.CAtom().GetPart()).Add(sa);
  }
  for( size_t i=0; i < parts.Count(); i++ )  {
    const TSAtomPList& al = parts.GetValue(i);
    for( size_t j=0; j < al.Count(); j++ )  {
      olxstr Labl = al[j]->GetType().symbol + Name;
      if( Labl.Length() >= 4 )
        Labl.SetLength(3);
      else if( Labl.Length() < 3 && parts.Count() > 1 )
        Labl << (char)('a'+i);  // part ID
      if( al.Count() > 1 )
        Labl << (char)('a' + lablInc++);      
      if( CheckLabel )  {
        TCAtom* CA;
        while( (CA = XFile().GetAsymmUnit().FindCAtom(Labl)) != NULL )  {
          if( CA == &al[j]->CAtom() || CA->IsDeleted() || CA->GetTag() < 0 )  break;
          Labl = al[j]->GetType().symbol + Name;
          if( Labl.Length() >= 4 )  
            Labl.SetLength(3);
          else if( Labl.Length() < 3 && parts.Count() > 1 )
            Labl << (char)('a'+i);
          const char next_ch = 'a' + lablInc++;
          if( next_ch > 'z' )
            Labl = CA->GetParent()->CheckLabel(NULL, Labl);
          else
            Labl << next_ch;      
        }
      }
      if( al[j]->GetLabel() != Labl )  {
        if( nu != NULL )  
          nu->AddAtom(al[j]->CAtom(), al[j]->GetLabel());
        al[j]->CAtom().SetLabel(Labl, false);
      }
      al[j]->CAtom().SetTag(0);
      al[j]->SetTag(0);
    }
  }
}
//..............................................................................
void TXApp::undoName(TUndoData *data)  {
  TNameUndo *undo = static_cast<TNameUndo*>(data);
  TAsymmUnit& au = XFile().GetAsymmUnit();
  for( size_t i=0; i < undo->AtomCount(); i++ )  {
    if( undo->GetCAtomId(i) >= au.AtomCount() )  // would definetely be an error
      continue;
    TCAtom& ca = au.GetAtom(undo->GetCAtomId(i));
    ca.SetLabel(undo->GetLabel(i), false);
    ca.SetType(undo->GetElement(i));
    if( ca.GetType() == iQPeakZ )
      ca.SetQPeak(undo->GetPeakHeight(i));
  }
}
//..............................................................................
TUndoData* TXApp::FixHL()  {
  TNameUndo *undo = new TNameUndo( new TUndoActionImplMF<TXApp>(this, &TXApp::undoName));
  olxdict<int,TSAtomPList,TPrimitiveComparator> frags;
  TIntList frag_id;
  TSAtomPList satoms;
  FindSAtoms(EmptyString, satoms, false, true);  //the selection might be returned
  if( !satoms.IsEmpty() )  {
    for( size_t i=0; i < satoms.Count(); i++ )  {
      if( !satoms[i]->IsAUAtom() )  continue;
      if( frag_id.IndexOf(satoms[i]->CAtom().GetFragmentId()) == InvalidIndex )
        frag_id.Add(satoms[i]->CAtom().GetFragmentId());
    }
  }
  ASObjectProvider& objects = XFile().GetLattice().GetObjects();
  const size_t ac = objects.atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = objects.atoms[i];
    if( !sa.CAtom().IsAvailable() || sa.GetType() == iQPeakZ || !sa.IsAUAtom() )  {
      sa.SetTag(-1);
      continue;
    }
    if( sa.GetType() == iHydrogenZ )  {
      sa.SetTag(-2);  // mark as unpocessed
      sa.CAtom().SetTag(-2);
      sa.CAtom().SetLabel(EmptyString, false);
      continue;
    }
    if( frag_id.IsEmpty() || frag_id.IndexOf(sa.CAtom().GetFragmentId()) != InvalidIndex )
      frags.Add(sa.CAtom().GetFragmentId()).Add(sa);
  }
  if( frag_id.IsEmpty() )  {
    for( size_t i=0; i < frags.Count(); i++ )  {
      TSAtomPList& al = frags.GetValue(i);
      for( size_t j=0; j < al.Count(); j++ )
        NameHydrogens(*al[j], undo, true);
    }
  }
  else  {
    for( size_t i=0; i < frag_id.Count(); i++ )  {
      TSAtomPList& al = frags[frag_id[i]];
      for( size_t j=0; j < al.Count(); j++ )
        NameHydrogens(*al[j], undo, true);
    }
  }
  // check if there are any standalone h atoms left...
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = objects.atoms[i];
    if( !sa.CAtom().IsAvailable() || !sa.IsAUAtom() )  continue;
    if( sa.GetType() == iHydrogenZ && sa.CAtom().GetTag() == -2 )
      NameHydrogens(sa, undo, true);
  }
  return undo;
}
//..............................................................................
bool RingsEq(const TSAtomPList& r1, const TSAtomPList& r2 )  {
  for( size_t i=0; i < r1.Count(); i++ )  {
    bool found = false;
    for( size_t j=0; j < r2.Count(); j++ )  {
      if( r2[j]->GetLattId() == r1[i]->GetLattId() )  {
        found = true;
        break;
      }
    }
    if( !found )
      return false;
  }
  return true;
}
void TXApp::RingContentFromStr(const olxstr& Condition, ElementPList& ringDesc)  {
  TStrList toks;
  olxstr symbol, count;
  for( size_t i=0; i < Condition.Length(); i++ )  {
    if( Condition[i] <= 'Z' && Condition[i] >= 'A' )  {
      if( !symbol.IsEmpty() )  {
        if( !count.IsEmpty() )  {
          const size_t c = count.ToSizeT();
          for( size_t j=0; j < c; j++ )
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
      const size_t c = count.ToSizeT();
      for( size_t j=0; j < c; j++ )
        toks.Add(symbol);
    }
    else
      toks.Add(symbol);
  }

  if( toks.Count() < 3 )  return;

  for( size_t i=0; i < toks.Count(); i++ )  {
    cm_Element* elm = XElementLib::FindBySymbol(toks[i]);
    if( elm == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Unknown element: ") << toks[i]);
    ringDesc.Add(elm);
  }
}
void TXApp::FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings)  {
  ElementPList ring;
  // external ring connectivity
  TTypeList< ElementPList > extRing;
  RingContentFromStr(Condition, ring);
  for( size_t i=0; i < XFile().GetLattice().FragmentCount(); i++ )  {
    XFile().GetLattice().GetFragment(i).FindRings(ring, rings);
  }

  for( size_t i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )  continue;
    for( size_t j= i+1; j < rings.Count(); j++ ) {
      if( rings.IsNull(j) )  continue;
      if( RingsEq(rings[i], rings[j]) )
        rings.NullItem(j);
    }
  }
  rings.Pack();
}
//..............................................................................
bool TXApp::FindSAtoms(const olxstr& condition, TSAtomPList& res, bool ReturnAll, bool ClearSelection)  {
  if( SelectionOwner != NULL )
    SelectionOwner->SetDoClearSelection(ClearSelection);
  TSAtomPList atoms;
  if( condition.IsEmpty() )  {  // try the selection first
    if( SelectionOwner != NULL )
      SelectionOwner->ExpandSelectionEx(atoms);
  }
  if( !condition.IsEmpty() )  {
    TStrList toks(condition, ' ');
    //TLattice& latt = XFile().GetLattice();
    ASObjectProvider& objects = XFile().GetLattice().GetObjects();
    for( size_t i=0; i < toks.Count(); i++ )  {
      if( toks[i].StartsFrom("#s") )  {  // TSAtom.LattId
        const size_t lat_id = toks[i].SubStringFrom(2).ToSizeT();
        if( lat_id >= objects.atoms.Count() )
          throw TInvalidArgumentException(__OlxSourceInfo, "satom id");
        if( objects.atoms[lat_id].CAtom().IsAvailable() )
          res.Add(objects.atoms[lat_id]);
        toks.Delete(i);
        i--;
      }
      else if( toks[i].IsNumber() )  {  // should not be here, but the parser will choke on it
        toks.Delete(i);
        i--;
      }
    }
    olxstr new_c = toks.Text(' ');
    if( !new_c.IsEmpty() )  {
      TCAtomGroup ag;
      TAtomReference ar(toks.Text(' '), SelectionOwner);      
      size_t atomAGroup;
      ar.Expand(XFile().GetRM(), ag, EmptyString, atomAGroup);
      if( !ag.IsEmpty() )  {
        atoms.SetCapacity( atoms.Count() + ag.Count() );
        TAsymmUnit& au = XFile().GetAsymmUnit();
        for( size_t i=0; i < au.AtomCount(); i++ )
          au.GetAtom(i).SetTag(au.GetAtom(i).GetId());
        TLattice& latt = XFile().GetLattice();
        for( size_t i=0; i < ag.Count(); i++ )  {
          if( ag[i].GetAtom()->GetTag() != ag[i].GetAtom()->GetId() )
            continue;
          for( size_t j=0; j < objects.atoms.Count(); j++ )  {
            TSAtom& sa = objects.atoms[j];
            if( !sa.CAtom().IsAvailable() )  continue;
            if( sa.CAtom().GetTag() != ag[i].GetAtom()->GetTag() )  continue;
            if( ag[i].GetMatrix() == NULL )  {  // get an atom from the asymm unit
              if( sa.IsAUAtom() )
                atoms.Add(sa);
            }
            else  {
              if( sa.ContainsMatrix(ag[i].GetMatrix()->GetId()) )  {
                atoms.Add(sa);
              }
            }
          }
        }
      }
    }
  }
  else if( atoms.IsEmpty() && ReturnAll ) {
    ASObjectProvider& objects = XFile().GetLattice().GetObjects();
    const size_t ac = objects.atoms.Count();
    atoms.SetCapacity(ac);
    for( size_t i=0; i < ac; i++ )
      if( objects.atoms[i].CAtom().IsAvailable() )
        atoms.Add(objects.atoms[i]);
  }
  res.AddList(atoms);
  return !atoms.IsEmpty();
}
//..............................................................................
void TXApp::ProcessRingAfix(TSAtomPList& ring, int afix, bool pivot_last)  {
  olxstr info("Processing");
  size_t pivot = (pivot_last ? ring.Count()-1 : 0);
  for( size_t i=0; i < ring.Count(); i++ )
    info << ' ' << ring[i]->GetLabel();
  TBasicApp::NewLogEntry() << info << ". Chosen pivot atom is " << ring[pivot]->GetLabel();
  if( ring[pivot]->CAtom().GetDependentAfixGroup() != NULL )
    ring[pivot]->CAtom().GetDependentAfixGroup()->Clear();
  TAfixGroup& ag = FXFile->GetRM().AfixGroups.New( &ring[pivot]->CAtom(), afix);
  for( size_t i=0; i < ring.Count(); i++ )  {  
    if( i == pivot )  continue;  // do not want to delete just created!
    TCAtom& ca = ring[i]->CAtom();
    if( ca.GetDependentAfixGroup() != NULL && ca.GetDependentAfixGroup()->GetAfix() == afix )  // if used in case to change order
      ca.GetDependentAfixGroup()->Clear();
  }
  if( pivot_last )  {
    for( size_t i=ring.Count()-2; i != InvalidIndex; i-- )
      ag.AddDependent(ring[i]->CAtom());
  }
  else  {
    for( size_t i=1; i < ring.Count(); i++ )
      ag.AddDependent(ring[i]->CAtom());
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
      for( size_t i=0; i < rings.Count(); i++ )  {
        if( m != 11 && !TNetwork::IsRingRegular(rings[i]) )  continue;
        // find the pivot (with heaviest atom attached)
        TNetwork::AnalyseRing( rings[i], ri.Clear() );
        if( ri.HasAfix || !olx_is_valid_index(ri.HeaviestSubsIndex) )  continue;
        if( m != 10 && ri.Substituted.Count() > 1 )  continue;
        if( m == 10 && ri.Substituted.Count() != 5 )  continue; // Cp*
        size_t shift = (m == 10 ? 0 : ri.HeaviestSubsIndex+1); // do not allign to pivot for Cp* 
        rings[i].ShiftL(shift);  // pivot is the last one now
        if( m == 11 )  {  // validate and rearrange to figure of 8
          if( ri.Alpha.IndexOf(shift - 1) == InvalidIndex ) continue;
          if( ri.Ternary.Count() != 2 )  continue;
          if( ri.Ternary.IndexOf( shift >= 2 ? shift-2 : rings[i].Count()-shift ) != InvalidIndex )  { // counter-clockwise direction
            for( size_t j=0; j < (rings[i].Count()-1)/2; j++ )  {
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
          for( size_t j=0; j < ri.Substituents.Count(); j++ )
            rings[i].Add(ri.Substituents[j][0] );
        }
        ProcessRingAfix(rings[i], afix, m != 10);
      }
    }
    else  {  // sa != NULL
      ElementPList ring;
      TTypeList< TSAtomPList > rings;
      if( sa->GetType() != iCarbonZ)
        ring.Add(sa->GetType());
      if( m == 6 || m == 7)
        RingContentFromStr(ring.IsEmpty() ? "C6" :"C5", ring);
      else if( m == 5 )
        RingContentFromStr(ring.IsEmpty() ? "C5" :"C4", ring);
      else if( m == 10 )
        RingContentFromStr(ring.IsEmpty() ? "C5" :"C4", ring);
      else if( m == 11 ) 
        RingContentFromStr(ring.IsEmpty() ? "C10" :"C9", ring);
      if( ring.IsEmpty() )  {
        NewLogEntry() << "Unable to derive ring size";
        return;
      }

      sa->GetNetwork().FindAtomRings(*sa, ring, rings);
      if( rings.IsEmpty() )  {
        NewLogEntry() << "no suitable rings have been found";
        return;
      }
      else if( rings.Count() > 1 )  {
        NewLogEntry() << "the atom is shared by several rings";
        return;
      }
      TNetwork::RingInfo ri;
      TNetwork::AnalyseRing( rings[0], ri );
      if( m == 11 )  {  // need to rearrage the ring to fit shelxl requirements as fihure of 8
        if( ri.Alpha.IndexOf(rings[0].Count() - 1) == InvalidIndex )  {
          NewLogEntry() << "the alpha substituted atom is expected";
          return;
        }
        if( ri.Ternary.Count() != 2 )  {
          NewLogEntry() << "naphtalene ring should have two ternary atoms";
          return;
        }
        if( ri.Ternary.IndexOf(rings[0].Count()-2) != InvalidIndex )  { // countr-clockwise direction to revert
          for( size_t i=0; i < (rings[0].Count()-1)/2; i++ )  {
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
          NewLogEntry() << "Could not locate Cp* ring";
          return;
        }
        for( size_t j=0; j < ri.Substituents.Count(); j++ )
          rings[0].Add(ri.Substituents[j][0] );
      }
      else  {
        if( ri.Substituents.GetLast().Count() == 0 )  {
          NewLogEntry() << "A substituted atom is expected";
          return;
        }
      }
      if( ri.Substituted.Count() > 1 && m != 10 )  
        NewLogEntry() << "The selected ring has more than one substituent";
      ProcessRingAfix(rings[0], afix, m!=10);
    }
  }
}
//..............................................................................
void TXApp::SetAtomUiso(TSAtom& sa, double val) {
  RefinementModel& rm = *sa.CAtom().GetParent()->GetRefMod();
  if( sa.CAtom().GetEllipsoid() == NULL )  {
    if( val <= -0.5 )  {
      size_t ni = InvalidIndex;
      for( size_t i=0; i < sa.NodeCount(); i++ ) {
        TSAtom& nd = sa.Node(i);
        if( nd.IsDeleted() || nd.GetType() == iQPeakZ )
          continue;
        if( ni != InvalidIndex )  {  // to many bonds
          ni = InvalidIndex;
          break;
        }
        ni = i;
      }
      // make sure that there is only one atom in the envi and it has proper Uiso
      if( ni != InvalidIndex && sa.Node(ni).CAtom().GetUisoOwner() == NULL )  {
        rm.Vars.FreeParam(sa.CAtom(), catom_var_name_Uiso);
        sa.CAtom().SetUisoOwner(&sa.Node(ni).CAtom());
        sa.CAtom().SetUisoScale(olx_abs(val));
        sa.CAtom().SetUiso(olx_abs(val)*sa.Node(ni).CAtom().GetUiso());
      }
      else
        throw TInvalidArgumentException(__OlxSourceInfo, "U owner");
    }
    else  {
      rm.Vars.SetParam(sa.CAtom(), catom_var_name_Uiso, val);
      sa.CAtom().SetUisoOwner(NULL);
    }
  }
}
//..............................................................................
void TXApp::GetSymm(smatd_list& ml) const {
  if( !FXFile->HasLastLoader() )
    throw TFunctionFailedException(__OlxSourceInfo, "a loaded file is expected");
  const TUnitCell& uc = FXFile->GetUnitCell();
  ml.SetCapacity( ml.Count() + uc.MatrixCount() );
  for( size_t i=0; i < uc.MatrixCount(); i++ )
    ml.AddCCopy(uc.GetMatrix(i));
}
//..............................................................................
void TXApp::ToDataItem(TDataItem& item) const  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TXApp::FromDataItem(TDataItem& item)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
olxstr TXApp::InitVcoV(VcoVContainer& vcovc) const {
  const olxstr shelx_fn = TEFile::ChangeFileExt(XFile().GetFileName(), "mat");
  const olxstr smtbx_fn = TEFile::ChangeFileExt(XFile().GetFileName(), "vcov");
  bool shelx_exists = TEFile::Exists(shelx_fn),
    smtbx_exists = TEFile::Exists(smtbx_fn);
  olxstr src_mat;
  if( shelx_exists && smtbx_exists )  {
    if( TEFile::FileAge(shelx_fn) > TEFile::FileAge(smtbx_fn) )  {
      vcovc.ReadShelxMat(shelx_fn);
      src_mat = "shelxl";
    }
    else  {
      src_mat = "smtbx";
      vcovc.ReadSmtbxMat(smtbx_fn);
    }
  }
  else if( shelx_exists )  {
    vcovc.ReadShelxMat(shelx_fn);
    src_mat = "shelxl";
  }
  else if( smtbx_exists )  {
    vcovc.ReadSmtbxMat(smtbx_fn);
    src_mat = "smtbx";
  }
  else
    throw TFunctionFailedException(__OlxSourceInfo, "could not find a variance-covariance matrix");
  return src_mat;
}
//..............................................................................
ElementRadii TXApp::ReadVdWRadii(const olxstr& fileName)  {
  ElementRadii radii;
  if( TEFile::Exists(fileName) )  {
    TStrList sl;
    sl.LoadFromFile(fileName);
    for( size_t i=0; i < sl.Count(); i++ )  {
      TStrList toks(sl[i], ' ');
      if( toks.Count() == 2 )  {
        cm_Element* elm = XElementLib::FindBySymbol(toks[0]);
        if( elm == NULL )  {
          TBasicApp::NewLogEntry(logError) << "Invalid atom type: " << toks[0];
          continue;
        }
        const size_t b_i = radii.IndexOf(elm);
        if( b_i == InvalidIndex )
          radii.Add(elm, toks[1].ToDouble());
        else
          radii.GetValue(b_i) = toks[1].ToDouble();
      }
    }
  }
  return radii;
}
//..............................................................................
void TXApp::PrintVdWRadii(const ElementRadii& radii, const ContentList& au_cont)  {
  if( au_cont.IsEmpty() )  return;
  TBasicApp::NewLogEntry() << "Using the following element radii:";
  TBasicApp::NewLogEntry() << "(Default radii source: http://www.ccdc.cam.ac.uk/products/csd/radii)";
  for( size_t i=0; i < au_cont.Count(); i++ )  {
    const size_t ei = radii.IndexOf(&au_cont[i].element);
    if( ei == InvalidIndex )
      TBasicApp::NewLogEntry() << au_cont[i].element.symbol << '\t' << au_cont[i].element.r_vdw;
    else
      TBasicApp::NewLogEntry() << au_cont[i].element.symbol << '\t' << radii.GetValue(ei);
  }
}
//..............................................................................
TXApp::CalcVolumeInfo TXApp::CalcVolume(const ElementRadii* radii)  {
  ASObjectProvider& objects = XFile().GetLattice().GetObjects();
  const size_t ac = objects.atoms.Count();
  const size_t bc = objects.bonds.Count();
  for( size_t i=0; i < bc; i++ )
    objects.bonds[i].SetTag(0);
  double Vi=0, Vt=0;
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& SA = objects.atoms[i];
    if( SA.IsDeleted() || !SA.CAtom().IsAvailable() )  continue;
    if( SA.GetType() == iQPeakZ )  continue;
    const double R1 = GetVdWRadius(SA, radii);
    Vt += M_PI*(R1*R1*R1)*4.0/3;
    for( size_t j=0; j < SA.BondCount(); j++ )  {
      TSBond& SB = SA.Bond(j);
      if( SB.GetTag() != 0 )  continue;
      const TSAtom& OA = SB.Another(SA);
      SB.SetTag(1);
      if( OA.IsDeleted() || !OA.CAtom().IsAvailable() )  continue;
      if( OA.GetType() == iQPeakZ )  continue;
      const double d = SB.Length();
      const double R2 = GetVdWRadius(OA, radii);
      const double h2 = (R1*R1 - (R2-d)*(R2-d))/(2*d);
      const double h1 = (R1+R2-d-h2);
      Vi += M_PI*( h1*h1*(R1-h1/3) + h2*h2*(R2-h2/3));
      //Vt += M_PI*(R1*R1*R1 + R2*R2*R2)*4.0/3;
    }
  }
  return CalcVolumeInfo(Vt, Vi);
}
//..............................................................................
WBoxInfo TXApp::CalcWBox(const TSAtomPList& atoms, const TDoubleList* radii,
                        double (*weight_calculator)(const TSAtom&))
{
  if( radii != NULL && atoms.Count() != radii->Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "radii count");
  TArrayList<AnAssociation2<vec3d, double> > crds(atoms.Count());
  for( size_t i=0; i < atoms.Count(); i++ )  {
    crds[i].A() = atoms[i]->crd();
    crds[i].B() = (*weight_calculator)(*atoms[i]);
  }

  WBoxInfo rv;
  vec3d rms;
  TSPlane::CalcPlanes(crds, rv.normals, rms, rv.center);  
  for( int i=0; i < 3; i++ )  {
    rv.d[i] = rv.normals[i].DotProd(rv.center)/rv.normals[i].Length();
    rv.normals[i].Normalise();
    for( size_t j=0; j < crds.Count(); j++ )  {
      const double d = crds[j].GetA().DotProd(rv.normals[i]) - rv.d[i];
      if( radii != NULL )  {
        const double d1 = d - radii->GetItem(j);
        if( d1 < rv.r_from[i] )
          rv.r_from[i] = d1;
        const double d2 = d + radii->GetItem(j);
        if( d2 > rv.r_to[i] )
          rv.r_to[i] = d2;
      }
      const double d1 = d - atoms[j]->GetType().r_sfil;
      if( d1 < rv.s_from[i] )
        rv.s_from[i] = d1;
      const double d2 = d + atoms[j]->GetType().r_sfil;
      if( d2 > rv.s_to[i] )
        rv.s_to[i] = d2;
    }
  }
  if( radii == NULL )  {
    rv.r_from = rv.s_from;
    rv.r_to = rv.s_to;
  }
  return rv;
}
//..............................................................................
