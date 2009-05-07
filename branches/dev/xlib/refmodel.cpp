#include "refmodel.h"
#include "lattice.h"
#include "symmparser.h"
#include "hkl.h"
#include "symmlib.h"
#include "pers_util.h"
#include "unitcell.h"
#include "xapp.h"
#include "refmerge.h"
#include "infotab.h"

RefinementModel::RefinementModel(TAsymmUnit& au) : 
  rDFIX(*this, rltBonds, "dfix"), 
  rDANG(*this, rltBonds, "dang"), 
  rSADI(*this, rltBonds, "sadi"), 
  rCHIV(*this, rltAtoms, "chiv"), 
  rFLAT(*this, rltGroup, "flat"), 
  rDELU(*this, rltAtoms, "delu"), 
  rSIMU(*this, rltAtoms, "simu"), 
  rISOR(*this, rltAtoms, "isor"), 
  rEADP(*this, rltAtoms, "eadp"), 
  ExyzGroups(*this), 
  AfixGroups(*this), 
  rSAME(*this),
  aunit(au), 
  HklStatFileID(EmptyString, 0, 0), 
  HklFileID(EmptyString, 0, 0), 
  Vars(*this),
  VarRefrencerId("basf"),
  Conn(*this)
{
  SetDefaults();
  RefContainers(rDFIX.GetIdName(), &rDFIX);
  RefContainers(rDANG.GetIdName(), &rDANG);
  RefContainers(TAsymmUnit::_GetIdName(), &aunit);
  //RefContainers(aunit.GetIdName(), &aunit);
  RefContainers(GetIdName(), this);
}
//....................................................................................................
void RefinementModel::SetDefaults() {
  HKLF = 4;
  HKLF_s = def_HKLF_s;
  HKLF_mat.I();
  HKLF_wt = def_HKLF_wt;
  HKLF_m = def_HKLF_m;
  MERG = def_MERG;
  OMIT_s = def_OMIT_s;
  OMIT_2t = def_OMIT_2t;
  OMITs_Modified = false;
  SHEL_hr = def_SHEL_hr;
  SHEL_lr = def_SHEL_lr;
  HKLF_set = MERG_set = OMIT_set = TWIN_set = SHEL_set = false;
  TWIN_n = def_TWIN_n;
  TWIN_mat.I() *= -1;
}
//....................................................................................................
void RefinementModel::Clear() {
  for( int i=0; i < SfacData.Count(); i++ )
    delete SfacData.GetValue(i);
  SfacData.Clear();

  for( int i=0; i < Frags.Count(); i++ )
    delete Frags.GetValue(i);
  Frags.Clear();

  rDFIX.Clear();
  rDANG.Clear();
  rSADI.Clear();
  rCHIV.Clear();
  rFLAT.Clear();
  rSIMU.Clear();
  rDELU.Clear();
  rISOR.Clear();
  rEADP.Clear();
  rSAME.Clear();
  ExyzGroups.Clear();
  //AfixGroups.Clear();
  InfoTables.Clear();
  UsedSymm.Clear();
  used_weight.Clear();
  proposed_weight.Clear();
  RefinementMethod = "L.S.";
  SolutionMethod = EmptyString;
  HKLSource = EmptyString;
  Omits.Clear();
  BASF.Clear();
  BASF_Vars.Clear();
  SetDefaults();
  expl.Clear();
  Vars.Clear();
  Conn.Clear();
}
//....................................................................................................
void RefinementModel::ClearVarRefs() {
  for( int i=0; i < RefContainers.Count(); i++ )  {
    IXVarReferencerContainer* rc = RefContainers.GetValue(i);
    for( int j=0; j < rc->ReferencerCount(); j++ )  {
      IXVarReferencer* vr = rc->GetReferencer(j);
      for( int k=0; k < vr->VarCount(); k++ )
        vr->SetVarRef(k, NULL);
    }
  }
}
//....................................................................................................
const smatd& RefinementModel::AddUsedSymm(const smatd& matr, const olxstr& id) {
  int ind = UsedSymm.IndexOfValue(matr);
  smatd* rv = NULL;
  if( ind == -1 )  {
    if( id.IsEmpty() ) 
      rv = &UsedSymm.Add( olxstr("$") << UsedSymm.Count(), matr );
    else
      rv = &UsedSymm.Add( id, matr );
    rv->SetTag(0); // do not lock it
  }
  else  {
    rv = &UsedSymm.GetValue(ind);
    rv->IncTag();
  }
  return *rv;
}
//....................................................................................................
void RefinementModel::RemUsedSymm(const smatd& matr)  {
  int ind = UsedSymm.IndexOfValue(matr);
  if( ind == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "matrix is not in the list");
  UsedSymm.GetValue(ind).DecTag();
  //if( UsedSymm.GetValue(ind).GetTag() == 0 )
  //  UsedSymm.Delete(ind);
}
//....................................................................................................
RefinementModel& RefinementModel::Assign(const RefinementModel& rm, bool AssignAUnit) {
  ClearAll();
  expl = rm.expl;
  used_weight = rm.used_weight;
  proposed_weight = rm.proposed_weight;
  LS = rm.LS;
  PLAN = rm.PLAN;
  HKLF = rm.HKLF;
  HKLF_s = rm.HKLF_s;
  HKLF_mat = rm.HKLF_mat;
  HKLF_wt = rm.HKLF_wt;
  HKLF_m = rm.HKLF_m;
  HKLF_set = rm.HKLF_set;
  MERG = rm.MERG;
  MERG_set = rm.MERG_set;
  OMIT_s = rm.OMIT_s;
  OMIT_2t = rm.OMIT_2t;
  OMIT_set = rm.OMIT_set;
  OMITs_Modified = rm.OMITs_Modified;
  SHEL_lr = rm.SHEL_lr;
  SHEL_hr = rm.SHEL_hr;
  SHEL_set = rm.SHEL_set;
  Omits = rm.Omits;
  TWIN_mat = rm.TWIN_mat;
  TWIN_n = rm.TWIN_n;
  TWIN_set = rm.TWIN_set;
  BASF = rm.BASF;
  for( int i=0; i < BASF.Count(); i++ )
    BASF_Vars.Add(NULL);
  HKLSource = rm.HKLSource;
  RefinementMethod = rm.RefinementMethod;
  SolutionMethod = rm.SolutionMethod;

  for( int i=0; i < rm.Frags.Count(); i++ )
    Frags(rm.Frags.GetKey(i), new Fragment( *rm.Frags.GetValue(i) ) );

  if( AssignAUnit )
    aunit.Assign(rm.aunit);
  rDFIX.Assign(rm.rDFIX);
  rDANG.Assign(rm.rDANG);
  rSADI.Assign(rm.rSADI);
  rCHIV.Assign(rm.rCHIV);
  rFLAT.Assign(rm.rFLAT);
  rSIMU.Assign(rm.rSIMU);
  rDELU.Assign(rm.rDELU);
  rISOR.Assign(rm.rISOR);
  rEADP.Assign(rm.rEADP);
  rSAME.Assign(aunit, rm.rSAME);
  ExyzGroups.Assign(rm.ExyzGroups);
  AfixGroups.Assign(rm.AfixGroups);
  Conn.Assign(rm.Conn);
  aunit._UpdateConnInfo();
  // restraunts have to be copied first, as some may refer to vars
  Vars.Assign( rm.Vars );

  for( int i=0; i < rm.UsedSymm.Count(); i++ )
    AddUsedSymm( rm.UsedSymm.GetValue(i), rm.UsedSymm.GetKey(i) );

  for( int i=0; i < rm.InfoTables.Count(); i++ )  {
    if( rm.InfoTables[i].IsValid() )
      InfoTables.Add( new InfoTab(*this, rm.InfoTables[i]) );
  }

  for( int i=0; i < rm.SfacData.Count(); i++ )
    SfacData(rm.SfacData.GetKey(i), new XScatterer( *rm.SfacData.GetValue(i)) );
  // check if all EQIV are used
  for( int i=0; i < UsedSymm.Count(); i++ )  {
    if( UsedSymm.GetValue(i).GetTag() <= 0 )
      UsedSymm.Delete(i--);
  }
  
  return *this;
}
//....................................................................................................
void RefinementModel::AddNewSfac(const olxstr& label,
                  double a1, double a2, double a3, double a4,
                  double b1, double b2, double b3, double b4,
                  double c, double fp, double fdp, double mu, double r, double wt)  {
  olxstr lb(label.CharAt(0) == '$' ? label.SubStringFrom(1) : label);
  cm_Element* src = XElementLib::FindBySymbolEx(lb);
  XScatterer* sc;
  if( src != NULL )
    sc = new XScatterer(*src, expl.GetRadiationEnergy());
  else
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate reference chemical element");
  sc->SetLabel(lb);
  sc->SetGaussians(a1, a2, a3, a4, b1, b2, b3, b4, c);
  sc->SetAdsorptionCoefficient(mu);
  sc->SetBondingR(r);
  sc->SetWeight(wt);
  sc->SetFpFdp( compd(fp, fdp) );
  SfacData.Add(label, sc);
}
//....................................................................................................
InfoTab& RefinementModel::AddHTAB() {
  return InfoTables.Add( new InfoTab(*this, infotab_htab) );
}
//....................................................................................................
InfoTab& RefinementModel::AddRTAB(const olxstr& codename, const olxstr& resi) {
  return InfoTables.Add( new InfoTab(*this, infotab_rtab, codename, resi) );
}
//....................................................................................................
void RefinementModel::Validate() {
  rDFIX.ValidateAll();
  rDANG.ValidateAll();
  rSADI.ValidateAll();
  rCHIV.ValidateAll();
  rFLAT.ValidateAll();
  rDELU.ValidateAll();
  rSIMU.ValidateAll();
  rISOR.ValidateAll();
  rEADP.ValidateAll();
  ExyzGroups.ValidateAll();
  AfixGroups.ValidateAll();
  Vars.Validate();
  for( int i=0; i < InfoTables.Count(); i++ )  {
    if( InfoTables[i].HasDeletedAtom() )
      InfoTables.Delete(i--);
  }
}
//....................................................................................................
bool RefinementModel::ValidateInfoTab(const InfoTab& it)  {
  int it_ind = -1;
  bool unique = true;
  for( int i=0; i < InfoTables.Count(); i++ )  {
    if( &InfoTables[i] == &it )
      it_ind = i;
    else  {
      if( unique && (InfoTables[i] == it) )  
        unique = false;
    }
  }
  if( !unique || !it.IsValid() )  {
    if( it_ind != -1 )
      InfoTables.Delete(it_ind);
    return false;
  }
  return true;
}
//....................................................................................................
void RefinementModel::AddInfoTab(const TStrList& l)  {
  int atom_start = 1;
  int resi_ind = l[0].IndexOf('_');
  olxstr tab_name = (resi_ind == -1 ? l[0] : l[0].SubStringTo(resi_ind));
  olxstr resi_name = (resi_ind == -1 ? EmptyString : l[0].SubStringFrom(resi_ind+1));
  if( tab_name.Comparei("HTAB") == 0 )
    InfoTables.Add( new InfoTab(*this, infotab_htab, EmptyString, resi_name) );
  else if( tab_name.Comparei("RTAB") == 0 )
    InfoTables.Add( new InfoTab(*this, infotab_rtab, l[atom_start++], resi_name) );
  else
    throw TInvalidArgumentException(__OlxSourceInfo, "unknown information table name");

  TAtomReference ar( l.Text(' ', atom_start) );
  TCAtomGroup ag;
  int atomAGroup;
  try  {  ar.Expand( *this, ag, resi_name, atomAGroup);  }
  catch( const TExceptionBase& ex )  {
    TBasicApp::GetLog().Error(olxstr("Invalid info table atoms: ") << l.Text(' '));
    TBasicApp::GetLog().Error(ex.GetException()->GetFullMessage());
    InfoTables.Delete( InfoTables.Count()-1 );
    return;
  }
  InfoTables.Last().AssignAtoms( ag );
  if( !InfoTables.Last().IsValid() )  {
    TBasicApp::GetLog().Error(olxstr("Invalid info table: ") << l.Text(' '));
    InfoTables.Delete( InfoTables.Count()-1 );
    return;
  }
  for( int i=0; i < InfoTables.Count()-1; i++ )  {
    if( InfoTables[i] == InfoTables.Last() )  {
      TBasicApp::GetLog().Error(olxstr("Duplicate info table: ") << l.Text(' '));
      InfoTables.Delete( InfoTables.Count()-1 );
      return;
    }
  }
}
//....................................................................................................
double RefinementModel::FindRestrainedDistance(const TCAtom& a1, const TCAtom& a2)  {
  for(int i=0; i < rDFIX.Count(); i++ )  {
    for( int j=0; j < rDFIX[i].AtomCount(); j+=2 )  {
      if( (rDFIX[i].GetAtom(j).GetAtom() == &a1 && rDFIX[i].GetAtom(j+1).GetAtom() == &a2) ||
          (rDFIX[i].GetAtom(j).GetAtom() == &a2 && rDFIX[i].GetAtom(j+1).GetAtom() == &a1) )  {
        return rDFIX[i].GetValue();
      }
    }
  }
  return -1;
}
//....................................................................................................
void RefinementModel::SetHKLSource(const olxstr& src) {
  if( HKLSource == src )  return;
  HKLSource = src;
}
//....................................................................................................
const TRefList& RefinementModel::GetReflections() const {
  try {
    TEFile::FileID hkl_src_id = TEFile::GetFileID(HKLSource);
    if( !_Reflections.IsEmpty() && hkl_src_id == HklFileID )
      return _Reflections;
    THklFile hf;
    hf.LoadFromFile(HKLSource);
    const vec3i minInd(hf.GetMinHkl()), maxInd(hf.GetMaxHkl());
    TArray3D<TRefPList*> hkl3d(
      minInd[0], maxInd[0],
      minInd[1], maxInd[1],
      minInd[2], maxInd[2]
    );
    hkl3d.FastInitWith(0);
    HklFileID = hkl_src_id;
    const int hkl_cnt = hf.RefCount();
    int maxRedundancy = 0;
    _Reflections.Clear();
    _FriedelPairs.Clear();
    _Redundancy.Clear();
    _FriedelPairCount = 0;
    _Reflections.SetCapacity(hkl_cnt);
    for( int i=0; i < hkl_cnt; i++ )  {
      TReflection& r = _Reflections.AddNew( hf[i] );
      TRefPList* rl = hkl3d(hf[i].GetHkl());
      if(  rl == NULL )
        hkl3d(hf[i].GetHkl()) = rl = new TRefPList;
      rl->Add(&r);
      if( rl->Count() > maxRedundancy )
        maxRedundancy = rl->Count();
    }
    _Redundancy.SetCount(maxRedundancy);
    for( int i=0; i < maxRedundancy; i++ )
      _Redundancy[i] = 0;
    for( int h=minInd[0]; h <= maxInd[0]; h++ )  {
      for( int k=minInd[1]; k <= maxInd[1]; k++ )  {
        for( int l=minInd[2]; l <= maxInd[2]; l++ )  {
          TRefPList* rl = hkl3d(h, k, l);
          if(  rl == NULL )  continue;
          if( (h|k|l) >= 0 )  {
            vec3i ind(-h,-k,-l);
            if( vec3i::IsInRangeInc(ind, minInd, maxInd) )  {
              if( hkl3d(ind) != NULL )  {
                _FriedelPairs.AddList(*hkl3d(ind));
                _FriedelPairs.AddList(*rl);
                _FriedelPairCount++;
              }
            }
          }
          _Redundancy[rl->Count()-1]++;
        }
      }
    }
    for( int h=minInd[0]; h <= maxInd[0]; h++ )  {
      for( int k=minInd[1]; k <= maxInd[1]; k++ )  {
        for( int l=minInd[2]; l <= maxInd[2]; l++ )  {
          if( hkl3d(h, k, l) != NULL )
            delete hkl3d(h, k, l);
        }
      }
    }
    return _Reflections;
  }
  catch(TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
}
//....................................................................................................
const RefinementModel::HklStat& RefinementModel::GetMergeStat() {
  // we need to take into the account MERG, HKLF and OMIT things here...
  try {
    const TRefList& all_refs = GetReflections();
    bool update = (HklStatFileID != HklFileID);
    if( !update && 
      _HklStat.OMIT_s == OMIT_s &&
      _HklStat.OMIT_2t == OMIT_2t &&
      _HklStat.SHEL_lr == SHEL_lr &&
      _HklStat.SHEL_hr == SHEL_hr &&
      _HklStat.MERG == MERG && !OMITs_Modified )  
    {
      return _HklStat;
    }
    else  {
      HklStatFileID = HklFileID;
      _HklStat.SetDefaults();
      // cannot use XFile, as we do not know which loader owns this object
      TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(aunit);
      if( sg == NULL )  // will not be seen outside though
        throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
      TRefList refs;
      FilterHkl(refs, _HklStat);
      if( MERG != 0 )  {
        smatd_list ml;
        sg->GetMatrices(ml, mattAll^mattIdentity);
        if( (MERG == 4 || MERG == 3) && !sg->IsCentrosymmetric() )  {  // merge all
          const int mc = ml.Count();
          for( int i=0; i < mc; i++ )
            ml.AddNew( ml[i] ) *= -1;
          ml.AddNew().I() *= -1;
        }
        _HklStat = RefMerger::DryMerge<RefMerger::ShelxMerger>(ml, refs, Omits);
      }
      else
        _HklStat = RefMerger::DryMergeInP1<RefMerger::ShelxMerger>(refs, Omits);
    }
  }
  catch(TExceptionBase&)  {
    _HklStat.SetDefaults();
  }
  return _HklStat;
}
//....................................................................................................
RefinementModel::HklStat& RefinementModel::FilterHkl(TRefList& out, RefinementModel::HklStat& stats)  {
  const TRefList& all_refs = GetReflections();
  const mat3d& hkl2c = aunit.GetHklToCartesian();
  // swap the values if in wrong order
  if( SHEL_hr > SHEL_lr )  {
    double tmp = SHEL_hr;
    SHEL_hr = SHEL_lr;
    SHEL_lr = tmp;
  }
  const double h_o_s = 0.5*OMIT_s, two_sin_2t = 2*sin(OMIT_2t*M_PI/360.0);
  double min_d = expl.GetRadiation()/( two_sin_2t == 0 ? 1e-6 : two_sin_2t);
  if( SHEL_set && SHEL_hr > min_d )
    min_d = SHEL_hr;
  const double min_qd = min_d*min_d;  // maximum d squared
  const double max_qd = SHEL_lr*SHEL_lr;
  const bool transform_hkl = !HKLF_mat.IsI();

  const int ref_cnt = all_refs.Count();
  out.SetCapacity( ref_cnt );
  stats.MinD = 100;
  stats.MaxD = -100;
  stats.MaxI = -100;
  stats.MinI = 100;
  //apply OMIT transformation and filtering and calculate spacing limits
  for( int i=0; i < ref_cnt; i++ )  {
    const TReflection& r = all_refs[i];
    if( r.GetTag() < 0 )  {
      _HklStat.OmittedReflections++;
      continue;
    }
    vec3i chkl(r.GetH(), r.GetK(), r.GetL());
    if( transform_hkl )
      r.MulHklR(chkl, HKLF_mat);

    vec3d hkl(chkl[0]*hkl2c[0][0],
      chkl[0]*hkl2c[0][1] + chkl[1]*hkl2c[1][1],
      chkl[0]*hkl2c[0][2] + chkl[1]*hkl2c[1][2] + chkl[2]*hkl2c[2][2]);
    const double qd = 1./hkl.QLength();
    if( qd > _HklStat.MaxD )  stats.MaxD = qd;
    if( qd < _HklStat.MinD )  stats.MinD = qd;
    // OMIT and SHEL filtering by res 
    if( h_o_s > 0 && r.GetI() < h_o_s*r.GetS() )  {
      stats.FilteredOff++;
      continue;
    }
    if( qd < max_qd && qd > min_qd )  {
      TReflection& new_ref = out.AddNew(r);
      if( r.GetI() < h_o_s*r.GetS() )  {
        new_ref.SetI( h_o_s*r.GetS() );
        stats.IntensityTransformed++;
      }
      if( new_ref.GetI() < 0 )
        new_ref.SetI(0);
      if( new_ref.GetI() > stats.MaxI )  stats.MaxI = new_ref.GetI();
      if( new_ref.GetI() < stats.MinI )  stats.MinI = new_ref.GetI();
    }
    else
      stats.FilteredOff++;
  }
  stats.LimDmax = sqrt(max_qd);
  stats.LimDmin = sqrt(min_qd);
  stats.MaxD = sqrt(stats.MaxD);
  stats.MinD = sqrt(stats.MinD);
  stats.MERG = MERG;
  stats.OMIT_s = OMIT_s;
  stats.OMIT_2t = OMIT_2t;
  stats.SHEL_lr = SHEL_lr;
  stats.SHEL_hr = SHEL_hr;
  stats.TotalReflections = out.Count();
  return stats;
}
//....................................................................................................
int RefinementModel::ProcessOmits(TRefList& refs)  {
  if( Omits.IsEmpty() )  return 0;
  int processed = 0;
  const int ref_c = refs.Count();
  for( int i=0; i < ref_c; i++ )  {
    const TReflection& r = refs[i];
    const int omit_cnt = Omits.Count();
    for( int j=0; j < omit_cnt; j++ )  {
      if( r.GetH() == Omits[j][0] && 
          r.GetK() == Omits[j][1] &&
          r.GetL() == Omits[j][2] ) 
      {
        refs.NullItem(i);
        processed++;
        break;
      }
    }
  }
  if( processed != 0 )
    refs.Pack();
  return processed;
}
//....................................................................................................
void RefinementModel::Describe(TStrList& lst, TPtrList<TCAtom>* a_res, TPtrList<TSimpleRestraint>* b_res) {
  Validate();
  int sec_num = 0;
  if( (rDFIX.Count()|rDANG.Count()|rSADI.Count()) != 0 )  {
    lst.Add(olxstr(++sec_num)) << ". Restrained distances";
    for( int i=0; i < rDFIX.Count(); i++ )  {
      TSimpleRestraint& sr = rDFIX[i];
      if( b_res != NULL )  b_res->Add(&sr);
      olxstr& str = lst.Add(EmptyString);
      for( int j=0; j < sr.AtomCount(); j+=2 )  {
        str << sr.GetAtom(j).GetFullLabel(*this) << '-' << sr.GetAtom(j+1).GetFullLabel(*this);
        if( (j+2) < sr.AtomCount() )
          str << " = ";
      }
      str << ": " << sr.GetValue() << " with sigma of " << sr.GetEsd();
    }
    for( int i=0; i < rDANG.Count(); i++ )  {
      TSimpleRestraint& sr = rDANG[i];
      if( b_res != NULL )  b_res->Add(&sr);
      olxstr& str = lst.Add(EmptyString);
      for( int j=0; j < sr.AtomCount(); j+=2 )  {
        str << sr.GetAtom(j).GetFullLabel(*this) << '-' << sr.GetAtom(j+1).GetFullLabel(*this);
        if( (j+2) < sr.AtomCount() )
          str << " = ";
      }
      str << ": " << sr.GetValue() << " with sigma of " << sr.GetEsd();
    }
    for( int i=0; i < rSADI.Count(); i++ )  {
      TSimpleRestraint& sr = rSADI[i];
      if( b_res != NULL )  b_res->Add(&sr);
      olxstr& str = lst.Add(EmptyString);
      for( int j=0; j < sr.AtomCount(); j+=2 )  {
        str << sr.GetAtom(j).GetFullLabel(*this) << '-' << sr.GetAtom(j+1).GetFullLabel(*this);
        if( (j+2) < sr.AtomCount() )
          str << " ~ ";
      }
      str << ": with sigma of " << sr.GetEsd();
    }
  }
  if( rCHIV.Count() != 0 )  {
    lst.Add(olxstr(++sec_num)) << ". Restrained atomic chiral volume";
    for( int i=0; i < rCHIV.Count(); i++ )  {
      TSimpleRestraint& sr = rCHIV[i];
      olxstr& str = lst.Add(EmptyString);
      for( int j=0; j < sr.AtomCount(); j++ )  {
        if( a_res != NULL && sr.GetAtom(j).GetMatrix() == NULL )  a_res->Add( sr.GetAtom(j).GetAtom() ); 
        str << sr.GetAtom(j).GetFullLabel(*this);
        if( (j+1) < sr.AtomCount() )
          str << ", ";
      }
      str << ": fixed at " << sr.GetValue() << " with sigma of " << sr.GetEsd();
    }
  }
  if( rFLAT.Count() != 0 )  {
    lst.Add(olxstr(++sec_num)) << ". Restrained planarity";
    for( int i=0; i < rFLAT.Count(); i++ )  {
      TSimpleRestraint& sr = rFLAT[i];
      olxstr& str = lst.Add(EmptyString);
      for( int j=0; j < sr.AtomCount(); j++ )  {
        if( a_res != NULL && sr.GetAtom(j).GetMatrix() == NULL )  a_res->Add( sr.GetAtom(j).GetAtom() ); 
        str << sr.GetAtom(j).GetFullLabel(*this);
        if( (j+1) < sr.AtomCount() )
          str << ", ";
      }
      str << ": with sigma of " << sr.GetEsd();
    }
  }
  if( rDELU.Count() != 0 )  {
    lst.Add(olxstr(++sec_num)) << ". Rigid bond restraints";
    for( int i=0; i < rDELU.Count(); i++ )  {
      TSimpleRestraint& sr = rDELU[i];
      if( sr.GetEsd() == 0 || sr.GetEsd1() == 0 )  continue;
      if( b_res != NULL )  b_res->Add(&sr);
      olxstr& str = lst.Add(EmptyString);
      if( sr.IsAllNonHAtoms() )  {
        str << "All non-hydrogen atoms";
      }
      else {
        for( int j=0; j < sr.AtomCount(); j++ )  {
          str << sr.GetAtom(j).GetFullLabel(*this);
          if( (j+1) < sr.AtomCount() )
            str << ", ";
        }
      }
      str << ": with sigma for 1-2 distances of " << sr.GetEsd() << " and sigma for 1-3 distances of " <<
        sr.GetEsd1();
    }
  }
  if( (rSIMU.Count()|rISOR.Count()|rEADP.Count()) != 0 )  {
    lst.Add(olxstr(++sec_num)) << ". Uiso/Uaniso restraints and constraints";
    for( int i=0; i < rSIMU.Count(); i++ )  {
      TSimpleRestraint& sr = rSIMU[i];
      olxstr& str = lst.Add(EmptyString);
      if( sr.IsAllNonHAtoms() )  {
        str << "All non-hydrogen atoms";
      }
      else {
        for( int j=0; j < sr.AtomCount(); j++ )  {
          if( a_res != NULL && sr.GetAtom(j).GetMatrix() == NULL )  a_res->Add( sr.GetAtom(j).GetAtom() ); 
          str << "U(" << sr.GetAtom(j).GetFullLabel(*this) << ')';
          if( (j+2) < sr.AtomCount() )
            str << " ~ ";
        }
      }
      str << ": within " << sr.GetValue() << "A with sigma of " << sr.GetEsd() << 
        " and sigma for terminal atoms of " << sr.GetEsd1();
    }
    for( int i=0; i < rISOR.Count(); i++ )  {
      TSimpleRestraint& sr = rISOR[i];
      olxstr& str = lst.Add(EmptyString);
      if( sr.IsAllNonHAtoms() )  {
        str << "All non-hydrogen atoms";
      }
      else {
        for( int j=0; j < sr.AtomCount(); j++ )  {
          if( sr.GetAtom(j).GetAtom()->GetEllipsoid() == NULL )  continue;
          if( a_res != NULL && sr.GetAtom(j).GetMatrix() == NULL )  a_res->Add( sr.GetAtom(j).GetAtom() ); 
          str << "Uanis(" << sr.GetAtom(j).GetFullLabel(*this) << ") ~ Uiso";
          if( (j+1) < sr.AtomCount() )
            str << ", ";
        }
      }
      str << ": with sigma of " << sr.GetEsd() << " and sigma for terminal atoms of " << sr.GetEsd1();
    }
    for( int i=0; i < rEADP.Count(); i++ )  {
      TSimpleRestraint& sr = rEADP[i];
      olxstr& str = lst.Add(EmptyString);
      for( int j=0; j < sr.AtomCount(); j++ )  {
        if( a_res != NULL && sr.GetAtom(j).GetMatrix() == NULL )  a_res->Add( sr.GetAtom(j).GetAtom() ); 
        if( sr.GetAtom(j).GetAtom()->GetEllipsoid() == NULL )
          str << "Uiso(";
        else 
          str << "Uanis(";
        str << sr.GetAtom(j).GetFullLabel(*this) << ')';
        if( (j+1) < sr.AtomCount() )
          str << " = ";
      }
    }
  }
  if( ExyzGroups.Count() != 0 )  {
    lst.Add(olxstr(++sec_num)) << ". Shared sites";
    for( int i=0; i < ExyzGroups.Count(); i++ )  {
      TExyzGroup& sr = ExyzGroups[i];
      olxstr& str = lst.Add('{');
      for( int j=0; j < sr.Count(); j++ )  {
        if( a_res != NULL )  a_res->Add( &sr[j] ); 
        str << sr[j].GetLabel();
        if( (j+1) < sr.Count() )
          str << ", ";
      }
      str << '}';
    }
  }
  if( rSAME.Count() != 0 )  {
    lst.Add(olxstr(++sec_num)) << ". Same fragments";
    for( int i=0; i < rSAME.Count(); i++ )  {
      TSameGroup& sg = rSAME[i];
      if( sg.DependentCount() == 0 || !sg.IsValidForSave() )
        continue;
      for( int j=0; j < sg.DependentCount(); j++ )  {
        if( !sg.GetDependent(j).IsValidForSave() )
          continue;
        olxstr& str = lst.Add('{');
        str << sg.GetDependent(j)[0].GetLabel();
        for( int k=1; k < sg.GetDependent(j).Count(); k++ )
          str << ", " << sg.GetDependent(j)[k].GetLabel();
        str << '}';
      }
      lst.Add("as");
      olxstr& str = lst.Add('{');
      str << sg[0].GetLabel();
      for( int j=1; j < sg.Count(); j++ )
        str << ", " << sg[j].GetLabel();
      str << '}';
    }
  }
  TStrList vars;
  Vars.Describe(vars);
  if( !vars.IsEmpty() )  {
    lst.Add(++sec_num) << ". Other restraints";
    lst.AddList(vars);
  }
}
//....................................................................................................
void RefinementModel::ProcessFrags()  {
  for( int i=0; i < Frags.Count(); i++ )  {
    Fragment* frag = Frags.GetValue(i);
    for( int j=0; j < AfixGroups.Count(); j++ )  {
      TAfixGroup& ag = AfixGroups[j];
      if( ag.GetM() == frag->GetCode() && (ag.Count()+1) == frag->Count() )  {
        TTypeList< AnAssociation2<vec3d, vec3d> > crds, icrds;
        TCAtomPList atoms;
        atoms.Add( &ag.GetPivot() );
        for( int k=0; k < ag.Count(); k++ )
          atoms.Add( &ag[k] );
        for( int k=0; k < atoms.Count(); k++ )  {
          if( atoms[k]->ccrd().QLength() > 0.00001 )  {
            crds.AddNew( (*frag)[k].crd, atoms[k]->ccrd() );
            icrds.AddNew((*frag)[k].crd );
          }
        }
        if( crds.Count() < 3 )
          throw TFunctionFailedException(__OlxSourceInfo, "Not enough atoms in fitted group");
        smatdd tm;
        vec3d tr, tri, t;
        for( int k=0; k < crds.Count(); k++ )  {
          icrds[k].B() = aunit.CellToCartesian( crds[k].B() );
          aunit.CartesianToCell( icrds[k].A() ) *= -1;
          aunit.CellToCartesian( icrds[k].A() );
          t += crds[k].B();
          tr += crds[k].GetA();
          tri += icrds[k].GetA();
        }
        t /= crds.Count();
        tr /= crds.Count();
        tri /= crds.Count();
        bool invert = false;
        double rms = TNetwork::FindAlignmentMatrix(crds, tr, t, tm);
        double irms = TNetwork::FindAlignmentMatrix(icrds, tri, t, tm);
        if( irms < rms && irms >= 0 )  {
          tr = tri;
          invert = true;
        }
        tm.r.Transpose();
        for( int k=0; k < atoms.Count(); k++ )  {
          vec3d v = (*frag)[k].crd;
          if( invert )  {
            aunit.CartesianToCell(v);
            v *= -1;
            aunit.CellToCartesian(v);
          }
          v = tm*(tr-v);
          atoms[k]->ccrd() = aunit.CartesianToCell(v);
        }
        ag.SetAfix( ag.GetN() );
      }
    }
  }
}
//....................................................................................................
void RefinementModel::ToDataItem(TDataItem& item) {
  // fields
  item.AddField("RefOutArg", PersUtil::NumberListToStr(PLAN));
  item.AddField("HklSrc", HKLSource);
  item.AddField("RefMeth", RefinementMethod);
  item.AddField("SolMeth", SolutionMethod);
  item.AddField("BatchScales", PersUtil::NumberListToStr(BASF));
  item.AddField("RefInArg", PersUtil::NumberListToStr(LS));

  // save used equivalent positions
  TIntList mat_tags(UsedSymm.Count());
  TDataItem& eqiv = item.AddItem("EQIV");
  for( int i=0; i < UsedSymm.Count(); i++ )  {
    eqiv.AddItem(UsedSymm.GetKey(i), TSymmParser::MatrixToSymmEx(UsedSymm.GetValue(i)));
    mat_tags[i] = UsedSymm.GetValue(i).GetTag();
    UsedSymm.GetValue(i).SetTag(i);
  }
  
  Vars.ToDataItem(item.AddItem("LEQS"));
  expl.ToDataItem(item.AddItem("EXPL"));  

  AfixGroups.ToDataItem(item.AddItem("afix"));
  ExyzGroups.ToDataItem(item.AddItem("exyz"));
  rSAME.ToDataItem(item.AddItem("same"));
  rDFIX.ToDataItem(item.AddItem("dfix"));
  rDANG.ToDataItem(item.AddItem("dang"));
  rSADI.ToDataItem(item.AddItem("sadi"));
  rCHIV.ToDataItem(item.AddItem("chiv"));
  rFLAT.ToDataItem(item.AddItem("flat"));
  rDELU.ToDataItem(item.AddItem("delu"));
  rSIMU.ToDataItem(item.AddItem("simu"));
  rISOR.ToDataItem(item.AddItem("isor"));
  rEADP.ToDataItem(item.AddItem("eadp"));
  
  TDataItem& hklf = item.AddItem("HKLF", HKLF);
  hklf.AddField("s", HKLF_s);
  hklf.AddField("wt", HKLF_wt);
  hklf.AddField("m", HKLF_m);
  hklf.AddField("mat", TSymmParser::MatrixToSymmEx(HKLF_mat));

  TDataItem& omits = item.AddItem("OMIT", OMIT_set);
  omits.AddField("s", OMIT_s);
  omits.AddField("2theta", OMIT_2t);
  omits.AddField("hkl", PersUtil::VecListToStr(Omits));
  item.AddItem("TWIN", TWIN_set).AddField("mat", TSymmParser::MatrixToSymmEx(TWIN_mat)).AddField("n", TWIN_n);
  item.AddItem("MERG", MERG_set).AddField("val", MERG);
  item.AddItem("SHEL", SHEL_set).AddField("high", SHEL_hr).AddField("low", SHEL_lr);
  Conn.ToDataItem( item.AddItem("CONN") );
  // restore matrix tags
  for( int i=0; i < UsedSymm.Count(); i++ )
    UsedSymm.GetValue(i).SetTag( mat_tags[i] );
}
//....................................................................................................
void RefinementModel::FromDataItem(TDataItem& item) {
  ClearAll();

  PersUtil::FloatNumberListFromStr(item.GetRequiredField("RefOutArg"), PLAN);
  HKLSource = item.GetRequiredField("HklSrc");
  RefinementMethod = item.GetRequiredField("RefMeth");
  SolutionMethod = item.GetRequiredField("SolMeth");
  PersUtil::FloatNumberListFromStr(item.GetRequiredField("BatchScales"), BASF);
  PersUtil::IntNumberListFromStr(item.GetRequiredField("RefInArg"), LS);

  TDataItem& eqiv = item.FindRequiredItem("EQIV");
  for( int i=0; i < eqiv.ItemCount(); i++ )
    TSymmParser::SymmToMatrix( eqiv.GetItem(i).GetValue(), UsedSymm.Add(eqiv.GetName()));
  

  expl.FromDataItem(item.FindRequiredItem("EXPL"));  

  AfixGroups.FromDataItem(item.FindRequiredItem("afix"));
  ExyzGroups.FromDataItem(item.FindRequiredItem("exyz"));
  rSAME.FromDataItem(item.FindRequiredItem("same"));
  rDFIX.FromDataItem(item.FindRequiredItem("dfix"));
  rDANG.FromDataItem(item.FindRequiredItem("dang"));
  rSADI.FromDataItem(item.FindRequiredItem("sadi"));
  rCHIV.FromDataItem(item.FindRequiredItem("chiv"));
  rFLAT.FromDataItem(item.FindRequiredItem("flat"));
  rDELU.FromDataItem(item.FindRequiredItem("delu"));
  rSIMU.FromDataItem(item.FindRequiredItem("simu"));
  rISOR.FromDataItem(item.FindRequiredItem("isor"));
  rEADP.FromDataItem(item.FindRequiredItem("eadp"));
  // restraints may use some of the vars...  
  Vars.FromDataItem( item.FindRequiredItem("LEQS") );

  TDataItem& hklf = item.FindRequiredItem("HKLF");
  HKLF = hklf.GetValue().ToInt();
  HKLF_s = hklf.GetRequiredField("s").ToDouble();
  HKLF_wt = hklf.GetRequiredField("wt").ToDouble();
  HKLF_m = hklf.GetRequiredField("m").ToDouble();
  smatd tmp_m;
  TSymmParser::SymmToMatrix(hklf.GetRequiredField("mat"), tmp_m);
  HKLF_mat = tmp_m.r;

  TDataItem& omits = item.FindRequiredItem("OMIT");
  OMIT_set = omits.GetValue().ToBool();
  OMIT_s = omits.GetRequiredField("s").ToDouble();
  OMIT_2t = omits.GetRequiredField("2theta").ToDouble();
  PersUtil::IntVecListFromStr(omits.GetRequiredField("hkl"), Omits);

  TDataItem& twin = item.FindRequiredItem("TWIN");
  TWIN_set = twin.GetValue().ToBool();
  TSymmParser::SymmToMatrix(twin.GetRequiredField("mat"), tmp_m);
  TWIN_mat = tmp_m.r;
  TWIN_n = twin.GetRequiredField("n").ToInt();

  TDataItem& merge = item.FindRequiredItem("MERG");
  MERG_set = merge.GetValue().ToBool();
  MERG = merge.GetRequiredField("val").ToInt();

  TDataItem& shel = *item.FindItem("SHEL");
  if( &shel != NULL )  {
    SHEL_set = shel.GetValue().ToBool();
    SHEL_lr = shel.GetRequiredField("low").ToDouble();
    SHEL_hr = shel.GetRequiredField("high").ToDouble();
  }

  Conn.FromDataItem( item.FindRequiredItem("CONN") );
  aunit._UpdateConnInfo();
}
//....................................................................................................
#ifndef _NO_PYTHON
PyObject* RefinementModel::PyExport(bool export_connectivity)  {
  PyObject* main = PyDict_New(), 
    *hklf = PyDict_New(), 
    *eq = PyTuple_New(UsedSymm.Count());
  TPtrList<PyObject> atoms, equivs;
  PyDict_SetItemString(main, "aunit", aunit.PyExport(atoms) );
  TIntList mat_tags(UsedSymm.Count());
  for( int i=0; i < UsedSymm.Count(); i++ )  {
    smatd& m = UsedSymm.GetValue(i);
    PyTuple_SetItem(eq, i, 
      equivs.Add(
        Py_BuildValue("(iii)(iii)(iii)(ddd)", m.r[0][0], m.r[0][1], m.r[0][2],
          m.r[1][0], m.r[1][1], m.r[1][2],
          m.r[2][0], m.r[2][1], m.r[2][2],
          m.t[0], m.t[1], m.t[2]
      )) );
    mat_tags[i] = m.GetTag();
    m.SetTag(i);
  }
  PyDict_SetItemString(main, "equivalents", eq);

  PyDict_SetItemString(main, "variables", Vars.PyExport(atoms) );
  PyDict_SetItemString(main, "exptl", expl.PyExport() );
  PyDict_SetItemString(main, "afix", AfixGroups.PyExport(atoms) );
  PyDict_SetItemString(main, "exyz", ExyzGroups.PyExport(atoms) );
  PyDict_SetItemString(main, "same", rSAME.PyExport(atoms) );
  PyDict_SetItemString(main, "dfix", rDFIX.PyExport(atoms, equivs) );
  PyDict_SetItemString(main, "dang", rDANG.PyExport(atoms, equivs) );
  PyDict_SetItemString(main, "sadi", rSADI.PyExport(atoms, equivs) );
  PyDict_SetItemString(main, "chiv", rCHIV.PyExport(atoms, equivs) );
  PyDict_SetItemString(main, "flat", rFLAT.PyExport(atoms, equivs) );
  PyDict_SetItemString(main, "delu", rDELU.PyExport(atoms, equivs) );
  PyDict_SetItemString(main, "simu", rSIMU.PyExport(atoms, equivs) );
  PyDict_SetItemString(main, "isor", rISOR.PyExport(atoms, equivs) );
  PyDict_SetItemString(main, "eadp", rEADP.PyExport(atoms, equivs) );

  PyDict_SetItemString(hklf, "value", Py_BuildValue("i", HKLF));
  PyDict_SetItemString(hklf, "s", Py_BuildValue("d", HKLF_s));
  PyDict_SetItemString(hklf, "m", Py_BuildValue("d", HKLF_m));
  PyDict_SetItemString(hklf, "wt", Py_BuildValue("d", HKLF_wt));
  PyDict_SetItemString(hklf, "matrix", 
    Py_BuildValue("(ddd)(ddd)(ddd)", HKLF_mat[0][0], HKLF_mat[0][1], HKLF_mat[0][2],
      HKLF_mat[1][0], HKLF_mat[1][1], HKLF_mat[1][2],
      HKLF_mat[2][0], HKLF_mat[2][1], HKLF_mat[2][2]));
  PyDict_SetItemString(main, "hklf", hklf );

    
  if( OMIT_set )  {
    PyObject* omit;
    PyDict_SetItemString(main, "omit", omit = PyDict_New() );
      PyDict_SetItemString(omit, "s", Py_BuildValue("d", OMIT_s));
      PyDict_SetItemString(omit, "2theta", Py_BuildValue("d", OMIT_2t));
  }
  if( TWIN_set )  {
    PyObject* twin = PyDict_New(), 
      *basf = PyTuple_New(BASF.Count());
      PyDict_SetItemString(twin, "n", Py_BuildValue("i", TWIN_n));
      PyDict_SetItemString(twin, "matrix", 
        Py_BuildValue("(ddd)(ddd)(ddd)", TWIN_mat[0][0], TWIN_mat[0][1], TWIN_mat[0][2],
          TWIN_mat[1][0], TWIN_mat[1][1], TWIN_mat[1][2],
          TWIN_mat[2][0], TWIN_mat[2][1], TWIN_mat[2][2]));
      for( int i=0; i < BASF.Count(); i++ )
        PyTuple_SetItem(basf, i, Py_BuildValue("d", BASF[i]) );
    PyDict_SetItemString(twin, "basf", basf);
    PyDict_SetItemString(main, "twin", twin );
  }
  if( SHEL_set )  {
    PyObject* shel;
    PyDict_SetItemString(main, "shel", shel = PyDict_New() );
      PyDict_SetItemString(shel, "low", Py_BuildValue("d", SHEL_lr));
      PyDict_SetItemString(shel, "high", Py_BuildValue("d", SHEL_hr));
  }
  // attach the connectivity...
  if( export_connectivity )  {
    TAtomEnvi ae;
    TLattice& lat = aunit.GetLattice();
    TUnitCell& uc = aunit.GetLattice().GetUnitCell();
    for( int i=0; i < lat.AtomCount(); i++ )  {
      TSAtom& sa = lat.GetAtom(i);
      if( sa.IsDeleted() || sa.GetAtomInfo() == iQPeakIndex )  continue;
      uc.GetAtomEnviList(sa, ae);
      if( PyDict_GetItemString(atoms[sa.CAtom().GetTag()], "neighbours") != NULL )
        continue;
      PyDict_SetItemString(atoms[sa.CAtom().GetTag()], "neighbours", ae.PyExport(atoms) );
      ae.Clear();
    }
  }
  //
  // restore matrix tags
  for( int i=0; i < UsedSymm.Count(); i++ )
    UsedSymm.GetValue(i).SetTag( mat_tags[i] );
  return main;
}
#endif
