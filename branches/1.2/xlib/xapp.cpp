/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xapp.h"
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
#include "twinning.h"
#include "sfutil.h"
#include "datafile.h"

TXApp::TXApp(const olxstr &basedir, bool)
  : TBasicApp(basedir), Library(EmptyString(), this)
{
  min_hbond_angle = 120;
  min_hbond_angle_i = false;
  preserve_fvars = false;
  preserve_fvars_i = false;
  safe_afix = true;
  safe_afix_i = false;
  interactions_i = false;
  max_label_length = 0;
}
//..............................................................................
TXApp::TXApp(const olxstr &basedir, ASObjectProvider* objectProvider,
  ASelectionOwner* selOwner)
  : TBasicApp(basedir), Library(EmptyString(), this)
{
  Init(objectProvider, selOwner);
}
//..............................................................................
void TXApp::Init(ASObjectProvider* objectProvider, ASelectionOwner* selOwner) {
  SelectionOwner = selOwner;
  try  {
    if( !TSymmLib::IsInitialised() )
      TEGC::AddP(new TSymmLib(GetBaseDir() + "symmlib.xld"));
  }
  catch( const TIOException& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  Files.Add((objectProvider == NULL ? new SObjectProvider()
    : objectProvider)->CreateXFile());

  DefineState(psFileLoaded, "Loaded file is expected");
  DefineState(psCheckFileTypeIns, "INS file is expected");
  DefineState(psCheckFileTypeCif, "CIF file is expected");
  DefineState(psCheckFileTypeP4P, "P4P file is expected");
  DefineState(psCheckFileTypeCRS, "CRS file is expected");

  CifTemplatesDir = GetBaseDir() + "etc/CIF/";

  XLibMacros::Export(Library);
}
//..............................................................................
TXApp::~TXApp() {
  Instance_() = 0;
}
//..............................................................................
bool TXApp::CheckProgramState(unsigned int specialCheck)  {
 if( specialCheck & psFileLoaded )
   return XFile().HasLastLoader();
 if ((specialCheck&psCheckFileTypeIns) != 0 &&
   (XFile().HasLastLoader() && (EsdlInstanceOf(*XFile().LastLoader(), TIns) ||
   XFile().LastLoader()->IsNative())))
   return true;
 if ((specialCheck&psCheckFileTypeP4P) != 0 &&
   (XFile().HasLastLoader() && EsdlInstanceOf(*XFile().LastLoader(), TP4PFile)))
   return true;
 if ((specialCheck&psCheckFileTypeCRS) != 0 &&
   (XFile().HasLastLoader() && EsdlInstanceOf(*XFile().LastLoader(), TCRSFile)))
   return true;
 if ((specialCheck&psCheckFileTypeCif) != 0 &&
   (XFile().HasLastLoader() && EsdlInstanceOf(*XFile().LastLoader(), TCif)))
   return true;
 return false;
}
//..............................................................................
void TXApp::CalcSF(const TRefList& refs, TArrayList<TEComplex<double> >& F)
  const
{
  XFile().UpdateAsymmUnit();
  TSpaceGroup* sg = NULL;
  try { sg = &XFile().GetLastLoaderSG(); }
  catch (...) {
    throw TFunctionFailedException(__OlxSourceInfo, "unknown spacegroup");
  }
  smatd_list ml;
  sg->GetMatrices(ml, mattAll ^ (mattCentering));
  int sg_order = (int)sg->GetLattice().GetVectors().Count() + 1;
  TSAtomPList atoms;
  TIObjectProvider<TSAtom> &atoms_ = XFile().GetLattice().GetObjects().atoms;
  atoms.SetCapacity(atoms_.Count());
  for (size_t i = 0; i < atoms_.Count(); i++) {
    TSAtom &a = atoms_[i];
    if (a.IsDeleted() || a.GetType() == iQPeakZ || !a.IsAUAtom()) {
      continue;
    }
    atoms.Add(a);
  }
  CalcSFEx(refs, F, atoms, ml, sg_order);
}
//..............................................................................
  void TXApp::CalcSFEx(const TRefList& refs, TArrayList<TEComplex<double> >& F,
  const TSAtomPList &atoms, const smatd_list &ml, int sg_order) const
{
  // initialise newly created atoms
  TAsymmUnit& au = XFile().GetAsymmUnit();
  const mat3d hkl2c = au.GetHklToCartesian();
  evecd quad(6);
  const static double EQ_PI = 8*M_PI*M_PI;
  const static double T_PI = 2*M_PI;
  const static double TQ_PI = 2.0*M_PI*M_PI;
  const double r_e = XFile().GetRM().expl.GetRadiationEnergy();

  // the thermal ellipsoid scaling factors
  double BM[6] = {hkl2c[0].Length(), hkl2c[1].Length(), hkl2c[2].Length(),
    0, 0, 0};
  BM[3] = 2*BM[1]*BM[2];
  BM[4] = 2*BM[0]*BM[2];
  BM[5] = 2*BM[0]*BM[1];
  BM[0] *= BM[0];
  BM[1] *= BM[1];
  BM[2] *= BM[2];

  ElementPList bais;
  olx_array_ptr<double> Ucifs(6*atoms.Count() + 1);
  TTypeList< AnAssociation3<const cm_Element*, compd, compd> > scatterers;
  for (size_t i = 0; i < atoms.Count(); i++) {
    TSAtom& a = *atoms[i];
    size_t ind = bais.IndexOf(a.GetType());
    if (ind == InvalidIndex) {
      scatterers.AddNew<const cm_Element*, compd, compd>(&a.GetType(), 0, 0);
      bais.Add(a.GetType());
      scatterers.GetLast().c =
        scatterers.GetLast().GetA()->CalcFpFdp(r_e);
      scatterers.GetLast().c -= scatterers.GetLast().GetA()->z;
      ind = scatterers.Count() - 1;
    }
    a.SetTag(ind);
    ind = i * 6;
    TEllipsoid* elp = a.GetEllipsoid();
    if (elp != NULL) {
      elp->GetShelxQuad(quad);  // default is Ucart
      au.UcartToUcif(quad);
      for (int k = 0; k < 6; k++) {
        Ucifs[ind + k] = -TQ_PI*quad[k] * BM[k];
      }
    }
    else {
      Ucifs[ind] = -EQ_PI*a.CAtom().GetUiso();
    }
  }

  const size_t a_cnt = atoms.Count(),
            m_cnt = ml.Count();
  for( size_t i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    const double d_s2 = ref.ToCart(hkl2c).QLength()*0.25;
    for( size_t j=0; j < scatterers.Count(); j++)  {
      scatterers[j].b = scatterers[j].GetA()->gaussians->calc_sq(d_s2);
      scatterers[j].b += scatterers[j].GetC();
    }
    compd ir;
    for( size_t j=0; j < a_cnt; j++ )  {
      compd l;
      for( size_t k=0; k < m_cnt; k++ )  {
        const vec3d hkl = ref.GetHkl()*ml[k].r;
        double tv = T_PI*(atoms[j]->ccrd().DotProd(hkl) +
          ml[k].t.DotProd(ref.GetHkl()));  // scattering vector + phase shift
        double ca, sa;
        olx_sincos(tv, &sa, &ca);
        if( atoms[j]->GetEllipsoid() != NULL )  {
          const double* Q = &Ucifs[j*6];  // pick up the correct ellipsoid
          double B = (hkl[0]*(Q[0]*hkl[0]+Q[4]*hkl[2]+Q[5]*hkl[1]) +
                      hkl[1]*(Q[1]*hkl[1]+Q[3]*hkl[2]) +
                      hkl[2]*(Q[2]*hkl[2]));
          B = exp(B);
          l.Re() += B*ca;
          l.Im() += B*sa;
        }
        else  {
          l.Re() += ca;
          l.Im() += sa;
        }
      }
      compd scv = scatterers[atoms[j]->GetTag()].GetB();
      if (atoms[j]->GetEllipsoid() == NULL) {
        scv *= exp(Ucifs[j * 6] * d_s2);
      }

      scv *= atoms[j]->CAtom().GetOccu();
      scv *= l;
      ir += scv;
    }
    F[i] = ir*sg_order;
  }
}
//..............................................................................
RefinementModel::HklStat TXApp::CalcFsq(TRefList &refs, evecd &Fsq,
  bool scale) const
{
  RefinementModel::HklStat rv;
  RefinementModel& rm = XFile().GetRM();
  TUnitCell::SymmSpace sp = XFile().GetUnitCell().GetSymmSpace();
  const TDoubleList basf = rm.GetBASFAsDoubleList();
  SymmSpace::InfoEx info_ex = SymmSpace::Compact(sp);
  double exti = rm.Vars.HasEXTI() ? rm.Vars.GetEXTI().GetValue() : 0;
  const double l = rm.expl.GetRadiation(),
    l_sq_4 = olx_sqr(rm.expl.GetRadiation())/4,
    s_l_q_e = 0.0005*exti*l*l*l;
  if (!basf.IsEmpty()) {
    if (rm.GetHKLF() >= 5) {
      int tn = rm.HasTWIN() ? rm.GetTWIN_n() : 0;
      twinning::general twin(info_ex, rm.GetReflections(),
        RefUtil::ResolutionAndSigmaFilter(rm), basf,
        mat3d::Transpose(rm.GetTWIN_mat()), tn);
      TArrayList<compd> F(twin.unique_indices.Count());
      SFUtil::CalcSF(XFile(), twin.unique_indices, F);
      twin.calc_fsq(F, Fsq);
      refs.TakeOver(twin.reflections);
      rv = twin.ms;
    }
    else  {
      rv = rm.GetRefinementRefList<
          TUnitCell::SymmSpace,RefMerger::ShelxMerger>(sp, refs);
      if (rv.FriedelOppositesMerged)
        info_ex.centrosymmetric = true;
      TArrayList<compd> F(refs.Count());
      SFUtil::CalcSF(XFile(), refs, F, rv.MERG != 4);
      twinning::merohedral twin(
        info_ex, refs, rv, basf, mat3d::Transpose(rm.GetTWIN_mat()),
        rm.GetTWIN_n());
      twin.calc_fsq(F, Fsq);
    }
  }
  else  {
    rv = rm.GetRefinementRefList<
        TUnitCell::SymmSpace,RefMerger::ShelxMerger>(sp, refs);
    if (rv.FriedelOppositesMerged)
      info_ex.centrosymmetric = true;
    TArrayList<compd> F(refs.Count());
    Fsq.Resize(refs.Count());
    SFUtil::CalcSF(XFile(), refs, F, !info_ex.centrosymmetric);
    if (exti != 0) {
      for (size_t i=0; i < F.Count(); i++) {
        double x = sp.HklToCart(refs[i].GetHkl()).QLength()*l_sq_4;
        double F2 = F[i].qmod();
        Fsq[i] = F2/sqrt(1+F2*s_l_q_e/sqrt(olx_max(0,x*(1-x))));
      }
      exti = 0;
    }
    else {
      for (size_t i=0; i < F.Count(); i++)
        Fsq[i] = F[i].qmod();
    }
  }
  if (exti != 0) {
    for (size_t i=0; i < Fsq.Count(); i++) {
      double x = sp.HklToCart(refs[i].GetHkl()).QLength()*l_sq_4;
      Fsq[i] /= sqrt(1+Fsq[i]*s_l_q_e/sqrt(olx_max(0,x*(1-x))));
    }
  }
  if (scale) {
    double scale_k = 1./olx_sqr(rm.Vars.GetVar(0).GetValue());
    rv.MaxI = 100;
    rv.MaxI = -100;
    for (size_t i=0; i < refs.Count(); i++) {
      refs[i].SetI(refs[i].GetI()*scale_k);
      if (refs[i].GetI() > rv.MaxI)
        rv.MaxI = refs[i].GetI();
      else if (refs[i].GetI() < rv.MinI)
        rv.MinI = refs[i].GetI();
      refs[i].SetS(refs[i].GetS()*scale_k);
    }
  }
  return rv;
}
//..............................................................................
void TXApp::NameHydrogens(TSAtom& SA, TUndoData* ud)  {
  TNameUndo* nu = static_cast<TNameUndo*>(ud);
  int lablInc = 0;
  olx_pdict<int,TSAtomPList> parts;
  olxstr Name(
    SA.GetLabel().StartsFromi(SA.GetType().symbol) ?
      SA.GetLabel().SubStringFrom(SA.GetType().symbol.Length())
    :
      EmptyString()
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
        Labl.SetLength( al.Count() > 1 ? 3 : 4);
      else if( Labl.Length() < 3 && parts.Count() > 1 )
        Labl << (char)('a'+i);  // part ID
      if( al.Count() > 1 )
        Labl << (char)('a' + lablInc++);
      if (true ) {
        if (lablInc > 25) {
          Labl = al[j]->CAtom().GetParent()->CheckLabel(&al[j]->CAtom(), Labl);
          continue;
        }
        olxstr appx;
        if (SA.CAtom().GetResiId() != 0) {
          appx << '_' << SA.CAtom().GetParent()->GetResidue(
            SA.CAtom().GetResiId()).GetNumber();
        }
        TCAtom* CA;
        while ((CA = XFile().GetAsymmUnit().FindCAtom(Labl+appx)) != NULL) {
          if (CA == &al[j]->CAtom() || CA->IsDeleted() || CA->GetTag() < 0)
            break;
          Labl = al[j]->GetType().symbol + Name;
          if (Labl.Length() >= 4)
            Labl.SetLength(3);
          else if (Labl.Length() < 3 && parts.Count() > 1)
            Labl << (char)('a'+i);
          const char next_ch = 'a' + lablInc++;
          if (next_ch > 'z') {
            Labl = CA->GetParent()->CheckLabel(&al[j]->CAtom(), Labl);
            break;
          }
          else
            Labl << next_ch;
        }
      }
      if (al[j]->GetLabel() != Labl) {
        if (nu != NULL)
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
  TNameUndo *undo = new TNameUndo(
    new TUndoActionImplMF<TXApp>(this, &TXApp::undoName));
  olx_pdict<int,TSAtomPList> frags;
  TIntList frag_id;
  TSAtomPList satoms;
  FindSAtoms(EmptyString(), satoms, false, true);  //the selection might be returned
  if (!satoms.IsEmpty()) {
    for (size_t i=0; i < satoms.Count(); i++) {
      if (!satoms[i]->IsAUAtom())  continue;
      if (frag_id.IndexOf(satoms[i]->CAtom().GetFragmentId()) == InvalidIndex)
        frag_id.Add(satoms[i]->CAtom().GetFragmentId());
    }
  }
  ASObjectProvider& objects = XFile().GetLattice().GetObjects();
  const size_t ac = objects.atoms.Count();
  for (size_t i=0; i < ac; i++) {
    TSAtom& sa = objects.atoms[i];
    if (!sa.CAtom().IsAvailable() || sa.GetType() == iQPeakZ ||
      !sa.IsAUAtom())
    {
      sa.SetTag(-1);
      continue;
    }
    if (sa.GetType() == iHydrogenZ) {
      sa.SetTag(-2);  // mark as unpocessed
      sa.CAtom().SetTag(-2);
      sa.CAtom().SetLabel(EmptyString(), false);
      continue;
    }
    if (frag_id.IsEmpty() ||
        frag_id.IndexOf(sa.CAtom().GetFragmentId()) != InvalidIndex)
    {
      frags.Add(sa.CAtom().GetFragmentId()).Add(sa);
    }
  }
  if (frag_id.IsEmpty()) {
    for (size_t i=0; i < frags.Count(); i++) {
      TSAtomPList& al = frags.GetValue(i);
      for (size_t j = 0; j < al.Count(); j++) {
        if (!XElementLib::IsMetal(al[j]->GetType()))
          NameHydrogens(*al[j], undo);
      }
    }
  }
  else {
    for (size_t i=0; i < frag_id.Count(); i++) {
      TSAtomPList& al = frags.Get(frag_id[i]);
      for (size_t j = 0; j < al.Count(); j++) {
        if (!XElementLib::IsMetal(al[j]->GetType()))
          NameHydrogens(*al[j], undo);
      }
    }
  }
  // check if there are any standalone h atoms left...
  for (size_t i=0; i < ac; i++) {
    TSAtom& sa = objects.atoms[i];
    if (!sa.CAtom().IsAvailable() || !sa.IsAUAtom())  continue;
    if (sa.GetType() == iHydrogenZ && sa.CAtom().GetTag() == -2)
      NameHydrogens(sa, undo);
  }
  return undo;
}
//..............................................................................
bool RingsEq(const TSAtomPList& r1, const TSAtomPList& r2 )  {
  for( size_t i=0; i < r1.Count(); i++ )  {
    bool found = false;
    for( size_t j=0; j < r2.Count(); j++ )  {
      if( r2[j]->GetOwnerId() == r1[i]->GetOwnerId() )  {
        found = true;
        break;
      }
    }
    if( !found )
      return false;
  }
  return true;
}
void TXApp::RingContentFromStr(const olxstr& Condition,
  ElementPList& ringDesc)
{
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
      count.SetLength(0);
    }
    else if( Condition[i] <= 'z' && Condition[i] >= 'a' )  {
      symbol << Condition[i];
      count.SetLength(0);
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
    if( elm == NULL ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("Unknown element: ") << toks[i]);
    }
    ringDesc.Add(elm);
  }
}
void TXApp::FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings) {
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
bool TXApp::FindSAtoms(const olxstr& condition, TSAtomPList& res,
  bool ReturnAll, bool ClearSelection)
{
  if( SelectionOwner != NULL )
    SelectionOwner->SetDoClearSelection(ClearSelection);
  TSAtomPList atoms;
  // try the selection first
  if (condition.IsEmpty() || condition.Equals("sel"))  {
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
      // should not be here, but the parser will choke on it
      else if( toks[i].IsNumber() )  {
        toks.Delete(i);
        i--;
      }
    }
    olxstr new_c = toks.Text(' ');
    if( !new_c.IsEmpty() )  {
      TCAtomGroup ag;
      TAtomReference ar(toks.Text(' '), SelectionOwner);
      size_t atomAGroup;
      ar.Expand(XFile().GetRM(), ag, EmptyString(), atomAGroup);
      if( !ag.IsEmpty() )  {
        atoms.SetCapacity( atoms.Count() + ag.Count() );
        TAsymmUnit& au = XFile().GetAsymmUnit();
        for( size_t i=0; i < au.AtomCount(); i++ )
          au.GetAtom(i).SetTag(au.GetAtom(i).GetId());
        for( size_t i=0; i < ag.Count(); i++ )  {
          if( (size_t)ag[i].GetAtom()->GetTag() != ag[i].GetAtom()->GetId() )
            continue;
          for( size_t j=0; j < objects.atoms.Count(); j++ )  {
            TSAtom& sa = objects.atoms[j];
            if( !sa.CAtom().IsAvailable() )  continue;
            if( sa.CAtom().GetTag() != ag[i].GetAtom()->GetTag() )  continue;
            // get an atom from the asymm unit
            if( ag[i].GetMatrix() == NULL )  {
              if( sa.IsAUAtom() )
                atoms.Add(sa);
            }
            else  {
              if( sa.IsGenerator(*ag[i].GetMatrix()) )  {
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
ConstPtrList<SObject> TXApp::GetSelected(bool unselect) const {
  TPtrList<SObject> rv;
  if (SelectionOwner != NULL) {
    SelectionOwner->SetDoClearSelection(unselect);
    rv = SelectionOwner->GetSelected();
  }
  return rv;
}
//..............................................................................
void TXApp::ProcessRingAfix(TSAtomPList& ring, int afix, bool pivot_last)  {
  olxstr info("Processing");
  size_t pivot = (pivot_last ? ring.Count()-1 : 0);
  for( size_t i=0; i < ring.Count(); i++ )
    info << ' ' << ring[i]->GetLabel();
  TBasicApp::NewLogEntry() << info << ". Chosen pivot atom is " <<
    ring[pivot]->GetLabel();
  if( ring[pivot]->CAtom().GetDependentAfixGroup() != NULL )
    ring[pivot]->CAtom().GetDependentAfixGroup()->Clear();
  TAfixGroup& ag = XFile().GetRM().AfixGroups.New( &ring[pivot]->CAtom(), afix);
  for( size_t i=0; i < ring.Count(); i++ )  {
    if (i == pivot)  continue;  // do not want to delete just created!
    TCAtom& ca = ring[i]->CAtom();
    // if used in case to change order
    if (ca.GetParentAfixGroup() != NULL) {
      TBasicApp::NewLogEntry() << "Removing intersecting AFIX group: ";
      TBasicApp::NewLogEntry() << ca.GetParentAfixGroup()->ToString();
      ca.GetParentAfixGroup()->Clear();
    }
    if( ca.GetDependentAfixGroup() != NULL &&
      ca.GetDependentAfixGroup()->GetAfix() == afix )
    {
      TBasicApp::NewLogEntry() << "Removing potentially intersecting AFIX"
        " group: ";
      TBasicApp::NewLogEntry() << ca.GetDependentAfixGroup()->ToString();
      ca.GetDependentAfixGroup()->Clear();
    }
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
  TBasicApp::NewLogEntry() <<
    "Automatically locating suitable targets for AFIX " << afix;
  int m = TAfixGroup::GetM(afix);
  if( m == 5 || m ==6 || m == 7 || m == 10 )  {  // special case
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
      catch( const TExceptionBase& exc )  {
        throw TFunctionFailedException(__OlxSourceInfo, exc);
      }
      TNetwork::RingInfo ri;
      for( size_t i=0; i < rings.Count(); i++ )  {
        if( m != 11 && !TNetwork::IsRingRegular(rings[i]) )  continue;
        // find the pivot (with heaviest atom attached)
        TNetwork::AnalyseRing( rings[i], ri.Clear() );
        if( ri.HasAfix || !olx_is_valid_index(ri.HeaviestSubsIndex) )  continue;
        if( m != 10 && ri.Substituted.Count() > 1 )  continue;
        if( m == 10 && ri.Substituted.Count() != 5 )  continue; // Cp*
        // do not allign to pivot for Cp*
        size_t shift = (m == 10 ? 0 : ri.HeaviestSubsIndex+1);
        rings[i].ShiftL(shift);  // pivot is the last one now
        if( m == 11 )  {  // validate and rearrange to figure of 8
          if( ri.Alpha.IndexOf(shift - 1) == InvalidIndex ) continue;
          if( ri.Ternary.Count() != 2 )  continue;
          // counter-clockwise direction
          if( ri.Ternary.IndexOf(
              shift >= 2 ? shift-2 : rings[i].Count()-shift ) != InvalidIndex )
          {
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
      // need to rearrage the ring to fit shelxl requirements as fihure of 8
      if( m == 11 )  {
        if( ri.Alpha.IndexOf(rings[0].Count() - 1) == InvalidIndex )  {
          NewLogEntry() << "the alpha substituted atom is expected";
          return;
        }
        if( ri.Ternary.Count() != 2 )  {
          NewLogEntry() << "naphtalene ring should have two ternary atoms";
          return;
        }
        // counter-clockwise direction to revert
        if( ri.Ternary.IndexOf(rings[0].Count()-2) != InvalidIndex )  {
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
      /* make sure that there is only one atom in the envi and it has proper
      Uiso
      */
      if( ni != InvalidIndex && sa.Node(ni).CAtom().GetUisoOwner() == NULL )  {
        rm.Vars.FreeParam(sa.CAtom(), catom_var_name_Uiso);
        sa.CAtom().SetUisoOwner(&sa.Node(ni).CAtom());
        sa.CAtom().SetUisoScale(olx_abs(val));
        sa.CAtom().SetUiso(olx_abs(val)*sa.Node(ni).CAtom().GetUiso());
      }
      else
        throw TInvalidArgumentException(__OlxSourceInfo, "U owner");
    }
    else {
      if (val > 1 && int(val / 10) >= rm.Vars.VarCount()) {
        rm.Vars.NewVar(0.025);
      }
      rm.Vars.SetParam(sa.CAtom(), catom_var_name_Uiso, val);
      sa.CAtom().SetUisoOwner(NULL);
    }
  }
}
//..............................................................................
void TXApp::GetSymm(smatd_list& ml) const {
  if (!XFile().HasLastLoader()) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "a loaded file is expected");
  }
  const TUnitCell& uc = XFile().GetUnitCell();
  ml.SetCapacity(ml.Count() + uc.MatrixCount());
  for (size_t i=0; i < uc.MatrixCount(); i++)
    ml.AddCopy(uc.GetMatrix(i));
}
//..............................................................................
void TXApp::ToDataItem(TDataItem& item) const {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TXApp::FromDataItem(TDataItem& item)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
olxstr TXApp::InitVcoV(VcoVContainer& vcovc) const {
  olxstr fn = XFile().GetFileName();
  if (XFile().LastLoader()->IsNative()) {
    olxstr md = XFile().LastLoader()->GetRM().GetModelSource();
    if (!md.IsEmpty()) {
      fn = TEFile::ExtractFilePath(XFile().GetFileName());
      TEFile::AddPathDelimeterI(fn) << md;
    }
  }
  const olxstr shelx_fn = TEFile::ChangeFileExt(fn, "mat");
  const olxstr smtbx_fn = TEFile::ChangeFileExt(fn, "vcov");
  bool shelx_exists = TEFile::Exists(shelx_fn),
    smtbx_exists = TEFile::Exists(smtbx_fn);
  olxstr src_mat;
  if (shelx_exists && smtbx_exists) {
    if (TEFile::FileAge(shelx_fn) > TEFile::FileAge(smtbx_fn)) {
      vcovc.ReadShelxMat(shelx_fn);
      src_mat = "shelxl";
    }
    else  {
      src_mat = "smtbx";
      vcovc.ReadSmtbxMat(smtbx_fn);
    }
  }
  else if (shelx_exists) {
    vcovc.ReadShelxMat(shelx_fn);
    src_mat = "shelxl";
  }
  else if (smtbx_exists) {
    vcovc.ReadSmtbxMat(smtbx_fn);
    src_mat = "smtbx";
  }
  else if (CheckFileType<TCif>()) {
    vcovc.FromCIF();
    src_mat = "CIF!!!";
  }
  else {
    return "NO (just cell e.s.d)";
  }
  return src_mat;
}
//..............................................................................
void TXApp::UpdateRadii(const olxstr &fn, const olxstr &rtype, bool log) {
  if (fn.EndsWithi(".xld")) {
    TDataFile df;
    df.LoadFromXLFile(fn);
    TDataItem &elements = df.Root().GetItemByName("elements");
    for (size_t i = 0; i < elements.ItemCount(); i++) {
      TDataItem &e = elements.GetItemByIndex(i);
      cm_Element* cme = XElementLib::FindBySymbol(e.GetName());
      if (cme == NULL) {
        TBasicApp::NewLogEntry(logError) << "Undefined element: '" <<
          e.GetName() << '\'';
        continue;
      }
      for (size_t j = 0; j < e.FieldCount(); j++) {
        if (e.GetFieldName(j).Equals("sfil"))
          cme->r_sfil = e.GetFieldByIndex(j).ToDouble();
        else if (e.GetFieldName(j).Equals("pers"))
          cme->r_pers = e.GetFieldByIndex(j).ToDouble();
        else if (e.GetFieldName(j).Equals("vdw"))
          cme->r_vdw = e.GetFieldByIndex(j).ToDouble();
        else if (e.GetFieldName(j).Equals("bonding"))
          cme->r_bonding = e.GetFieldByIndex(j).ToDouble();
      }
    }
    if (log) {
      TBasicApp::NewLogEntry() <<
        "Custom radii file loaded: '" << fn << '\'';
    }
  }
  else {
    olxstr_dict<double> radii;
    TStrList sl = TEFile::ReadLines(fn),
      changed;
    // parse the file and fill the dictionary
    for (size_t i = 0; i < sl.Count(); i++)  {
      size_t idx = sl[i].IndexOf(' ');
      if (idx != InvalidIndex) {
        radii(sl[i].SubStringTo(idx), sl[i].SubStringFrom(idx + 1).ToDouble());
      }
    }
    // end the file parsing
    if (rtype.Equalsi("sfil")) {
      for (size_t i = 0; i < radii.Count(); i++) {
        cm_Element* cme = XElementLib::FindBySymbol(radii.GetKey(i));
        if (cme != NULL && cme->r_sfil != radii.GetValue(i)) {
          cme->r_sfil = radii.GetValue(i);
          changed << cme->symbol;
        }
      }
    }
    else if (rtype.Equalsi("pers")) {
      for (size_t i = 0; i < radii.Count(); i++) {
        cm_Element* cme = XElementLib::FindBySymbol(radii.GetKey(i));
        if (cme != NULL && cme->r_pers != radii.GetValue(i)) {
          cme->r_pers = radii.GetValue(i);
          changed << cme->symbol;
        }
      }
    }
    else if (rtype.Equalsi("vdw")) {
      for (size_t i = 0; i < radii.Count(); i++) {
        cm_Element* cme = XElementLib::FindBySymbol(radii.GetKey(i));
        if (cme != NULL && cme->r_vdw != radii.GetValue(i)) {
          cme->r_vdw = radii.GetValue(i);
          changed << cme->symbol;
        }
      }
    }
    else if (rtype.Equalsi("bonding")) {
      for (size_t i = 0; i < radii.Count(); i++) {
        cm_Element* cme = XElementLib::FindBySymbol(radii.GetKey(i));
        if (cme != NULL && cme->r_bonding != radii.GetValue(i)) {
          cme->r_bonding = radii.GetValue(i);
          changed << cme->symbol;
        }
      }
    }
    else {
      TBasicApp::NewLogEntry(logError) <<
        (olxstr("Undefined radii name: ").quote() << rtype);
    }
    if (log && !changed.IsEmpty()) {
      TBasicApp::NewLogEntry() << "Using user defined " << rtype <<
        " radii for:";
      olxstr all = changed.Text(' ');
      changed.Clear();
      TBasicApp::NewLogEntry() << changed.Hyphenate(all, 80, true);
    }
  }
}
//..............................................................................
ElementRadii::const_dict_type TXApp::ReadRadii(const olxstr& fileName) {
  ElementRadii radii;
  if (TEFile::Exists(fileName)) {
    TStrList sl = TEFile::ReadLines(fileName);
    for (size_t i = 0; i < sl.Count(); i++) {
      TStrList toks(sl[i], ' ');
      if (toks.Count() == 2) {
        cm_Element* elm = XElementLib::FindBySymbol(toks[0]);
        if (elm == NULL) {
          TBasicApp::NewLogEntry(logError) << "Invalid atom type: " << toks[0];
          continue;
        }
        radii.Add(elm, toks[1].ToDouble(), true);
      }
    }
  }
  return radii;
}
//..............................................................................
void TXApp::PrintRadii(int which, const ElementRadii& radii,
  const ContentList& au_cont)
{
  if (au_cont.IsEmpty())  return;
  TBasicApp::NewLogEntry() << "Using the following element radii:";
  if (which == 4) {
    TBasicApp::NewLogEntry() <<
      "(Default radii source: http://www.ccdc.cam.ac.uk/products/csd/radii)";
  }
  for (size_t i = 0; i < au_cont.Count(); i++) {
    const size_t ei = radii.IndexOf(&au_cont[i].element);
    if (ei == InvalidIndex) {
      double r = 0;
      switch (which) {
      case 0: r = au_cont[i].element.r_bonding; break;
      case 1: r = au_cont[i].element.r_pers; break;
      case 2: r = au_cont[i].element.r_cov; break;
      case 3: r = au_cont[i].element.r_sfil; break;
      case 4: r = au_cont[i].element.r_vdw; break;
      case 5: r = au_cont[i].element.r_custom; break;
      }
      TBasicApp::NewLogEntry() << au_cont[i].element.symbol << '\t' <<
        r;
    }
    else {
      TBasicApp::NewLogEntry() << au_cont[i].element.symbol << '\t' <<
        radii.GetValue(ei);
    }
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
  TArrayList<olx_pair_t<vec3d, double> > crds(atoms.Count());
  for( size_t i=0; i < atoms.Count(); i++ )  {
    crds[i].a = atoms[i]->crd();
    crds[i].b = (*weight_calculator)(*atoms[i]);
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
double TXApp::GetMinHBondAngle()  {
  TXApp &a = GetInstance();
  if (a.min_hbond_angle_i)
    return a.min_hbond_angle;
  else {
    a.min_hbond_angle = TBasicApp::GetInstance().GetOptions()
    .FindValue("hbond_min_angle", "120").ToDouble();
    a.min_hbond_angle_i = true;
    return a.min_hbond_angle;
  }
}
//..............................................................................
bool TXApp::DoPreserveFVARs() {
  TXApp &a = GetInstance();
  if (a.preserve_fvars_i)
    return a.preserve_fvars;
  else {
    a.preserve_fvars = TBasicApp::GetInstance().GetOptions()
    .FindValue("preserve_fvars", FalseString()).ToBool();
    a.preserve_fvars_i = true;
    return a.preserve_fvars;
  }
}
//..............................................................................
bool TXApp::DoUseSafeAfix() {
  TXApp &a = GetInstance();
  if (a.safe_afix_i) {
    return a.safe_afix;
  }
  else {
    a.safe_afix = TBasicApp::GetInstance().GetOptions()
    .FindValue("safe_afix", TrueString()).ToBool();
    a.safe_afix_i = true;
    return a.safe_afix;
  }
}
//..............................................................................
size_t TXApp::GetMaxLabelLength() {
  TXApp &a = GetInstance();
  if (a.max_label_length != 0) {
    return a.max_label_length;
  }
  else {
    try {
      a.max_label_length = TBasicApp::GetInstance().GetOptions()
        .FindValue("max_label_length", "4").ToSizeT();
    }
    catch (...) {
      a.max_label_length = 4;
    }
    return a.max_label_length;
  }
}
//..............................................................................
void TXApp::InitInteractions() {
  if (interactions_i)
    return;
  TStrList toks(TBasicApp::GetInstance().GetOptions()
    .FindValue("interactions_from", "H"), ',');
  for (size_t i=0; i < toks.Count(); i++) {
    cm_Element *e = XElementLib::FindBySymbol(toks[i]);
    if (e == NULL) {
      TBasicApp::NewLogEntry() << "interactions_from, invalid symbol: " <<
        toks[i];
    }
    else
      interactions_from.AddUnique(e->z);
  }
  toks.Clear();
  toks.Strtok(TBasicApp::GetInstance().GetOptions()
    .FindValue("interactions_to", "N,O,F,Cl,S,Br,Se"), ',');
  for (size_t i=0; i < toks.Count(); i++) {
    cm_Element *e = XElementLib::FindBySymbol(toks[i]);
    if (e == NULL) {
      TBasicApp::NewLogEntry() << "interactions_to, invalid symbol: " <<
        toks[i];
    }
    else
      interactions_to.AddUnique(e->z);
  }
  interactions_i = true;
}
//..............................................................................
SortedObjectList<int, TPrimitiveComparator>& TXApp::GetInteractionsFrom() {
  TXApp &a = GetInstance();
  a.InitInteractions();
  return a.interactions_from;
}
//..............................................................................
SortedObjectList<int, TPrimitiveComparator>& TXApp::GetInteractionsTo() {
  TXApp &a = GetInstance();
  a.InitInteractions();
  return a.interactions_to;
}
//..............................................................................
const_strlist TXApp::BangList(const TSAtom& A)  {
  TStrList L;
  for( size_t i=0; i < A.BondCount(); i++ )  {
    const TSBond &B = A.Bond(i);
    olxstr& T = L.Add(A.GetLabel());
    T << '-'  << B.Another(A).GetLabel();
    T << ": " << olxstr::FormatFloat(3, B.Length());
  }
  for( size_t i=0; i < A.BondCount(); i++ )  {
    const TSBond &B = A.Bond(i);
    for( size_t j=i+1; j < A.BondCount(); j++ )  {
      const TSBond &B1 = A.Bond(j);
      olxstr& T = L.Add(B.Another(A).GetLabel());
      T << '-' << A.GetLabel() << '-';
      T << B1.Another(A).GetLabel() << ": ";
      const vec3d V = B.Another(A).crd() - A.crd();
      const vec3d V1 = B1.Another(A).crd() - A.crd();
      if( V.QLength()*V1.QLength() != 0 )  {
        double angle = V.CAngle(V1);
        angle = acos(angle)*180/M_PI;
        T << olxstr::FormatFloat(3, angle);
      }
    }
  }
  return L;
}
//..............................................................................
void TXApp::BangTable(const TSAtom& A, TTTable<TStrList>& Table) {
  if( A.BondCount() == 0 )  return;
  Table.Resize(A.BondCount(), A.BondCount());
  Table.ColName(0) = A.GetLabel();
  for( size_t i=0; i < A.BondCount()-1; i++ )
    Table.ColName(i+1) = A.Bond(i).Another(A).GetLabel();
  for( size_t i=0; i < A.BondCount(); i++ )  {
    const TSBond &B = A.Bond(i);
    Table[i][0] = olxstr::FormatFloat(3, B.Length());
    Table.RowName(i) = B.Another(A).GetLabel();
    for( size_t j=0; j < A.BondCount()-1; j++ )  {
      const TSBond& B1 = A.Bond(j);
      if( i == j )  { Table[i][j+1] = '-'; continue; }
      if( i <= j )  { Table[i][j+1] = '-'; continue; }
      const vec3d V = B.Another(A).crd() - A.crd();
      const vec3d V1 = B1.Another(A).crd() - A.crd();
      if( V.QLength()*V1.QLength() != 0 )  {
        double angle = V.CAngle(V1);
        angle = acos(angle)*180/M_PI;
        Table[i][j+1] = olxstr::FormatFloat(3, angle);
      }
      else
        Table[i][j+1] = '-';
    }
  }
}
//..............................................................................
double TXApp::Tang(TSBond *B1, TSBond *B2, TSBond *Middle, olxstr *Sequence) {
  // right parameters should be passed, e.g. the bonds should be connecetd like
  // B1-Middle-B2 or B2-Middle->B1, otherwise the result is almost meaningless!
  if( Middle->A() == B1->A() || Middle->A() == B1->B() )
    ;
  else  {
    olx_swap(B1, B2);
  }
  // using next scheme : A1-A2-A3-A4
  TSAtom &A2 = Middle->A();
  TSAtom &A3 = Middle->B();
  TSAtom &A1 = B1->Another(A2);
  TSAtom &A4 = B2->Another(A3);
  const double angle = olx_dihedral_angle_signed(
    A1.crd(), A2.crd(), A3.crd(), A4.crd());
  if (Sequence != NULL) {
    *Sequence = A1.GetLabel();
    *Sequence << '-' << A2.GetLabel() <<
                 '-' << A3.GetLabel() <<
                 '-' << A4.GetLabel();
  }
  return angle;
}
const_strlist TXApp::TangList(TSBond *XMiddle)  {
  TStrList L;
  TSBondPList BondsA, BondsB;
  size_t maxl=0;
  TSBond *B, *Middle = XMiddle;
  TSAtom *A = &Middle->A();
  for( size_t i=0; i < A->BondCount(); i++ )  {
    B = &A->Bond(i);
    if( B != Middle ) BondsA.Add(B);
  }
  A = &Middle->B();
  for( size_t i=0; i < A->BondCount(); i++ )  {
    B = &A->Bond(i);
    if( B != Middle ) BondsB.Add(B);
  }
  for( size_t i=0; i < BondsA.Count(); i++ )  {
    for( size_t j=0; j < BondsB.Count(); j++ )  {
      olxstr& T = L.Add();
      const double angle = Tang( BondsA[i], BondsB[j], Middle, &T);
      T << ':' << ' ';
      if( T.Length() > maxl ) maxl = T.Length();  // to format the string later
      T << olxstr::FormatFloat(3, angle);
    }
  }
  for( size_t i=0; i < L.Count(); i++ )  {
    size_t j = L[i].IndexOf(':');
    L[i].Insert(' ', j, maxl-j);
  }
  return L;
}
//..............................................................................
olxstr TXApp::GetPlatformString_(bool full) const {
  olxstr rv = TBasicApp::GetPlatformString_(full);
  if (!full) {
    return rv;
  }
#ifdef _PYTHON
  rv << ", Python: " << PY_VERSION;
#endif
  return rv;
}
//..............................................................................
