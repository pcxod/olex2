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
void RefinementModel::Clear(uint32_t clear_mask) {
  for( size_t i=0; i < SfacData.Count(); i++ )
    delete SfacData.GetValue(i);
  SfacData.Clear();
  UserContent.Clear();
  for( size_t i=0; i < Frags.Count(); i++ )
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
  ExyzGroups.Clear();
  if( (clear_mask & rm_clear_SAME) != 0 )
    rSAME.Clear();
  //ExyzGroups.Clear();
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
  PLAN.Clear();
  LS.Clear();
  if( (clear_mask & rm_clear_AFIX) != 0 )
    AfixGroups.Clear();
  if( (clear_mask & rm_clear_VARS) != 0 )
    Vars.ClearAll();
}
//....................................................................................................
void RefinementModel::ClearVarRefs() {
  for( size_t i=0; i < RefContainers.Count(); i++ )  {
    IXVarReferencerContainer* rc = RefContainers.GetValue(i);
    for( size_t j=0; j < rc->ReferencerCount(); j++ )  {
      IXVarReferencer& vr = rc->GetReferencer(j);
      for( size_t k=0; k < vr.VarCount(); k++ )
        vr.SetVarRef(k, NULL);
    }
  }
}
//....................................................................................................
const smatd& RefinementModel::AddUsedSymm(const smatd& matr, const olxstr& id)  {
  for( size_t i=0;  i < UsedSymm.Count(); i++ )  {
    if( UsedSymm.GetValue(i).symop == matr )  {
      UsedSymm.GetValue(i).ref_cnt++;
      return UsedSymm.GetValue(i).symop;
    }
  }
  return UsedSymm.Add(
    id.IsEmpty() ? (olxstr("$") << (UsedSymm.Count()+1)) : id, RefinementModel::Equiv(matr)).symop;
}
//....................................................................................................
void RefinementModel::UpdateUsedSymm(const class TUnitCell& uc)  {
  for( size_t i=0;  i < UsedSymm.Count(); i++ )
    uc.InitMatrixId(UsedSymm.GetValue(i).symop);
}
//....................................................................................................
void RefinementModel::RemUsedSymm(const smatd& matr)  {
  for( size_t i=0;  i < UsedSymm.Count(); i++ )  {
    if( UsedSymm.GetValue(i).symop == matr )  {
      if( UsedSymm.GetValue(i).ref_cnt > 0 )
        UsedSymm.GetValue(i).ref_cnt--;
      return;
    }
  }
  throw TInvalidArgumentException(__OlxSourceInfo, "matrix is not in the list");
}
//....................................................................................................
size_t RefinementModel::UsedSymmIndex(const smatd& matr) const {
  for( size_t i=0; i < UsedSymm.Count(); i++ )
    if( UsedSymm.GetValue(i).symop == matr )
      return i;
  return InvalidIndex;
}
//....................................................................................................
RefinementModel& RefinementModel::Assign(const RefinementModel& rm, bool AssignAUnit) {
  Clear(rm_clear_ALL);
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
  for( size_t i=0; i < BASF.Count(); i++ )
    BASF_Vars.Add(NULL);
  HKLSource = rm.HKLSource;
  RefinementMethod = rm.RefinementMethod;
  SolutionMethod = rm.SolutionMethod;

  for( size_t i=0; i < rm.Frags.Count(); i++ )
    Frags(rm.Frags.GetKey(i), new Fragment( *rm.Frags.GetValue(i) ) );

  if( AssignAUnit )
    aunit.Assign(rm.aunit);
  // need to copy the ID's before any restraints or info tabs use them or all gets broken... !!! 
  for( size_t i=0; i < rm.UsedSymm.Count(); i++ )
    AddUsedSymm(rm.UsedSymm.GetValue(i).symop, rm.UsedSymm.GetKey(i));

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
  // restraunts have to be copied first, as some may refer to vars
  Vars.Assign( rm.Vars );

  Conn.Assign(rm.Conn);
  aunit._UpdateConnInfo();

  for( size_t i=0; i < rm.InfoTables.Count(); i++ )  {
    if( rm.InfoTables[i].IsValid() )
      InfoTables.Add( new InfoTab(*this, rm.InfoTables[i]) );
  }

  for( size_t i=0; i < rm.SfacData.Count(); i++ )
    SfacData(rm.SfacData.GetKey(i), new XScatterer(*rm.SfacData.GetValue(i)));
  UserContent = rm.UserContent;
  // check if all EQIV are used
  for( size_t i=0; i < UsedSymm.Count(); i++ )  {
    if( UsedSymm.GetValue(i).ref_cnt == 0 )
      UsedSymm.Delete(i--);
  }
  
  return *this;
}
//....................................................................................................
olxstr RefinementModel::GetBASFStr() const {
  olxstr rv;
  for( size_t i=0; i < BASF.Count(); i++ )  {
    rv << Vars.GetParam(*this, (short)i);
    if( (i+1) < BASF.Count() )
      rv << ' ';
  }
  return rv;
}
//....................................................................................................
olxstr RefinementModel::GetTWINStr() const {
  olxstr rv;
  for( size_t i=0; i < 9; i++ )  {
    if( TWIN_mat[i/3][i%3] == 0 )
      rv << "0 ";
    else
      rv << TWIN_mat[i/3][i%3] << ' ';
  }
  return rv << TWIN_n;
}
//....................................................................................................
void RefinementModel::SetIterations(int v)  {  
  if( LS.IsEmpty() ) 
    LS.Add(v);
  else
    LS[0] = v;  
}
//....................................................................................................
void RefinementModel::SetPlan(int v)  {  
  if( PLAN.IsEmpty() )  
    PLAN.Add(v);
  else
    PLAN[0] = v;  
}
//....................................................................................................
void RefinementModel::AddSfac(XScatterer& sc)  {
  const size_t i = SfacData.IndexOf(sc.GetLabel());
  if( i != InvalidIndex )  {
    SfacData.GetEntry(i).val->Merge(sc);
    delete &sc;
  }
  else
    SfacData.Add(sc.GetLabel(), &sc);
}
//....................................................................................................
InfoTab& RefinementModel::AddHTAB() {
  return InfoTables.Add(new InfoTab(*this, infotab_htab));
}
//....................................................................................................
InfoTab& RefinementModel::AddRTAB(const olxstr& codename, const olxstr& resi) {
  return InfoTables.Add(new InfoTab(*this, infotab_rtab, codename, resi));
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
  for( size_t i=0; i < InfoTables.Count(); i++ )  {
    if( InfoTables[i].HasDeletedAtom() )
      InfoTables.Delete(i--);
  }
}
//....................................................................................................
bool RefinementModel::ValidateInfoTab(const InfoTab& it)  {
  size_t it_ind = InvalidIndex;
  bool unique = true;
  for( size_t i=0; i < InfoTables.Count(); i++ )  {
    if( &InfoTables[i] == &it )
      it_ind = i;
    else  {
      if( unique && (InfoTables[i] == it) )  
        unique = false;
    }
  }
  if( !unique || !it.IsValid() )  {
    if( it_ind != InvalidIndex )
      InfoTables.Delete(it_ind);
    return false;
  }
  return true;
}
//....................................................................................................
void RefinementModel::AddInfoTab(const TStrList& l)  {
  size_t atom_start = 1;
  size_t resi_ind = l[0].IndexOf('_');
  olxstr tab_name = (resi_ind == InvalidIndex ? l[0] : l[0].SubStringTo(resi_ind));
  olxstr resi_name = (resi_ind == InvalidIndex ? EmptyString : l[0].SubStringFrom(resi_ind+1));
  if( tab_name.Equalsi("HTAB") )
    InfoTables.Add( new InfoTab(*this, infotab_htab, EmptyString, resi_name) );
  else if( tab_name.Equalsi("RTAB") )
    InfoTables.Add( new InfoTab(*this, infotab_rtab, l[atom_start++], resi_name) );
  else if( tab_name.Equalsi("MPLA") )  {
    if( l[atom_start].IsNumber() )
      InfoTables.Add(new InfoTab(*this, infotab_mpla, l[atom_start++], resi_name));
    else
      InfoTables.Add(new InfoTab(*this, infotab_mpla, EmptyString, resi_name));
  }
  else
    throw TInvalidArgumentException(__OlxSourceInfo, "unknown information table name");

  TAtomReference ar(l.Text(' ', atom_start));
  TCAtomGroup ag;
  size_t atomAGroup;
  try  {  ar.Expand( *this, ag, resi_name, atomAGroup);  }
  catch( const TExceptionBase& ex )  {
    TBasicApp::NewLogEntry(logError) << "Invalid info table atoms: " << l.Text(' ');
    TBasicApp::NewLogEntry(logError) << ex.GetException()->GetFullMessage();
    InfoTables.Delete( InfoTables.Count()-1 );
    return;
  }
  InfoTables.GetLast().AssignAtoms(ag);
  if( !InfoTables.GetLast().IsValid() )  {
    TBasicApp::NewLogEntry(logError) << "Invalid info table: " << l.Text(' ');
    InfoTables.Delete( InfoTables.Count()-1 );
    return;
  }
  for( size_t i=0; i < InfoTables.Count()-1; i++ )  {
    if( InfoTables[i] == InfoTables.GetLast() )  {
      TBasicApp::NewLogEntry(logError) << "Duplicate info table: " << l.Text(' ');
      InfoTables.Delete( InfoTables.Count()-1 );
      return;
    }
  }
}
//....................................................................................................
double RefinementModel::FindRestrainedDistance(const TCAtom& a1, const TCAtom& a2)  {
  for(size_t i=0; i < rDFIX.Count(); i++ )  {
    for( size_t j=0; j < rDFIX[i].AtomCount(); j+=2 )  {
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
    _HklStat.FileMinInd = vec3i(100,100,100);
    _HklStat.FileMaxInd = vec3i(-100,-100,-100);
    TArray3D<TRefPList*> hkl3d(minInd, maxInd);
    hkl3d.FastInitWith(0);
    HklFileID = hkl_src_id;
    const size_t hkl_cnt = hf.RefCount();
    size_t maxRedundancy = 0;
    _Reflections.Clear();
    _FriedelPairs.Clear();
    _Redundancy.Clear();
    _FriedelPairCount = 0;
    _Reflections.SetCapacity(hkl_cnt);
    for( size_t i=0; i < hkl_cnt; i++ )  {
      if( HKLF == 5 )  {
        if( hf[i].GetFlag() < 0 )  {
          while( ++i < hkl_cnt && hf[i].GetFlag() < 0 )
            ;
          i--;
          continue;
        }
      }
      if( hf[i].GetTag() < 0 )  // is after (0, 0, 0) ?
        continue;
      TReflection& r = _Reflections.AddNew(hf[i]);
      vec3i::UpdateMinMax(r.GetHkl(), _HklStat.FileMinInd, _HklStat.FileMaxInd);
      TRefPList* rl = hkl3d(hf[i].GetHkl());
      if(  rl == NULL )
        hkl3d(hf[i].GetHkl()) = rl = new TRefPList;
      rl->Add(&r);
      if( rl->Count() > maxRedundancy )
        maxRedundancy = rl->Count();
    }
    _Redundancy.SetCount(maxRedundancy);
    for( size_t i=0; i < maxRedundancy; i++ )
      _Redundancy[i] = 0;
    for( int h=minInd[0]; h <= maxInd[0]; h++ )  {
      for( int k=minInd[1]; k <= maxInd[1]; k++ )  {
        for( int l=minInd[2]; l <= maxInd[2]; l++ )  {
          TRefPList* rl1 = hkl3d(h, k, l);
          if(  rl1 == NULL )  continue;
          const vec3i ind(-h,-k,-l);
          if( vec3i::IsInRangeInc(ind, minInd, maxInd) )  {
            TRefPList* rl2 = hkl3d(ind);
            if( rl2 != NULL )  {
              _FriedelPairs.AddList(*rl2);
              _FriedelPairs.AddList(*rl1);
              _FriedelPairCount++;
              _Redundancy[rl2->Count()-1]++;
              delete rl2;
              hkl3d(ind) = NULL;
            }
          }
          _Redundancy[rl1->Count()-1]++;
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
      TRefList refs;
      FilterHkl(refs, _HklStat);
      TUnitCell::SymSpace sp = aunit.GetLattice().GetUnitCell().GetSymSpace();
      if( MERG != 0 && HKLF != 5 )  {
        bool mergeFP = (MERG == 4 || MERG == 3) && !sp.IsCentrosymmetric();
        _HklStat = RefMerger::DryMerge<TUnitCell::SymSpace,RefMerger::ShelxMerger>(
          sp, refs, Omits, mergeFP);
      }
      else
        _HklStat = RefMerger::DrySGFilter(sp, refs, Omits);
    }
  }
  catch(const TExceptionBase& e)  {
    _HklStat.SetDefaults();
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
  return _HklStat;
}
//....................................................................................................
RefinementModel::HklStat& RefinementModel::FilterHkl(TRefList& out, RefinementModel::HklStat& stats)  {
  const TRefList& all_refs = GetReflections();
  const mat3d& hkl2c = aunit.GetHklToCartesian();
  // swap the values if in wrong order
  if( SHEL_hr > SHEL_lr )
    olx_swap(SHEL_hr, SHEL_lr);
  const double h_o_s = 0.5*OMIT_s, two_sin_2t = 2*sin(OMIT_2t*M_PI/360.0);
  double min_d = expl.GetRadiation()/( two_sin_2t == 0 ? 1e-6 : two_sin_2t);
  if( SHEL_set && SHEL_hr > min_d )
    min_d = SHEL_hr;
  const double min_qd = min_d*min_d;  // maximum d squared
  const double max_qd = SHEL_lr*SHEL_lr;
  const bool transform_hkl = !HKLF_mat.IsI();

  const size_t ref_cnt = all_refs.Count();
  out.SetCapacity(ref_cnt);
  stats.MinD = 100;
  stats.MaxD = -100;
  stats.MaxI = -100;
  stats.MinI = 100;
  //apply OMIT transformation and filtering and calculate spacing limits
  for( size_t i=0; i < ref_cnt; i++ )  {
    const TReflection& r = all_refs[i];
    if( r.GetTag() < 0 )  {
      _HklStat.OmittedReflections++;
      continue;
    }
    vec3i chkl = r.GetHkl();
    if( transform_hkl )
      chkl = (HKLF_mat*vec3d(chkl)).Round<int>();

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
      if( new_ref.GetI() > stats.MaxI )  stats.MaxI = new_ref.GetI();
      if( new_ref.GetI() < stats.MinI )  stats.MinI = new_ref.GetI();
      new_ref.SetHkl(chkl);
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
RefinementModel::HklStat& RefinementModel::AdjustIntensity(TRefList& out,
  RefinementModel::HklStat& stats) const
{
  const double h_o_s = 0.5*OMIT_s;
  const size_t ref_cnt = out.Count();
  for( size_t i=0; i < ref_cnt; i++ )  {
    TReflection& r = out[i];
    if( r.GetI() < h_o_s*r.GetS() )  {
      r.SetI(h_o_s*r.GetS());
      stats.IntensityTransformed++;
    }
    //if( r.GetI() < 0 )
    //  r.SetI(0);
  }
  return stats;
}
//....................................................................................................
size_t RefinementModel::ProcessOmits(TRefList& refs)  {
  if( Omits.IsEmpty() )  return 0;
  size_t processed = 0;
  const size_t ref_c = refs.Count();
  for( size_t i=0; i < ref_c; i++ )  {
    const TReflection& r = refs[i];
    const size_t omit_cnt = Omits.Count();
    for( size_t j=0; j < omit_cnt; j++ )  {
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
    for( size_t i=0; i < rDFIX.Count(); i++ )  {
      TSimpleRestraint& sr = rDFIX[i];
      if( b_res != NULL )  b_res->Add(&sr);
      olxstr& str = lst.Add(EmptyString);
      for( size_t j=0; j < sr.AtomCount(); j+=2 )  {
        str << sr.GetAtom(j).GetFullLabel(*this) << '-' << sr.GetAtom(j+1).GetFullLabel(*this);
        if( (j+2) < sr.AtomCount() )
          str << " = ";
      }
      str << ": " << sr.GetValue() << " with sigma of " << sr.GetEsd();
    }
    for( size_t i=0; i < rDANG.Count(); i++ )  {
      TSimpleRestraint& sr = rDANG[i];
      if( b_res != NULL )  b_res->Add(&sr);
      olxstr& str = lst.Add(EmptyString);
      for( size_t j=0; j < sr.AtomCount(); j+=2 )  {
        str << sr.GetAtom(j).GetFullLabel(*this) << '-' << sr.GetAtom(j+1).GetFullLabel(*this);
        if( (j+2) < sr.AtomCount() )
          str << " = ";
      }
      str << ": " << sr.GetValue() << " with sigma of " << sr.GetEsd();
    }
    for( size_t i=0; i < rSADI.Count(); i++ )  {
      TSimpleRestraint& sr = rSADI[i];
      if( b_res != NULL )  b_res->Add(&sr);
      olxstr& str = lst.Add(EmptyString);
      for( size_t j=0; j < sr.AtomCount(); j+=2 )  {
        str << sr.GetAtom(j).GetFullLabel(*this) << '-' << sr.GetAtom(j+1).GetFullLabel(*this);
        if( (j+2) < sr.AtomCount() )
          str << " ~ ";
      }
      str << ": with sigma of " << sr.GetEsd();
    }
  }
  if( rCHIV.Count() != 0 )  {
    lst.Add(olxstr(++sec_num)) << ". Restrained atomic chiral volume";
    for( size_t i=0; i < rCHIV.Count(); i++ )  {
      TSimpleRestraint& sr = rCHIV[i];
      olxstr& str = lst.Add(EmptyString);
      for( size_t j=0; j < sr.AtomCount(); j++ )  {
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
    for( size_t i=0; i < rFLAT.Count(); i++ )  {
      TSimpleRestraint& sr = rFLAT[i];
      olxstr& str = lst.Add(EmptyString);
      for( size_t j=0; j < sr.AtomCount(); j++ )  {
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
    for( size_t i=0; i < rDELU.Count(); i++ )  {
      TSimpleRestraint& sr = rDELU[i];
      if( sr.GetEsd() == 0 || sr.GetEsd1() == 0 )  continue;
      if( b_res != NULL )  b_res->Add(&sr);
      olxstr& str = lst.Add(EmptyString);
      if( sr.IsAllNonHAtoms() )  {
        str << "All non-hydrogen atoms";
      }
      else {
        for( size_t j=0; j < sr.AtomCount(); j++ )  {
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
    for( size_t i=0; i < rSIMU.Count(); i++ )  {
      TSimpleRestraint& sr = rSIMU[i];
      olxstr& str = lst.Add(EmptyString);
      if( sr.IsAllNonHAtoms() )  {
        str << "All non-hydrogen atoms";
      }
      else {
        for( size_t j=0; j < sr.AtomCount(); j++ )  {
          if( a_res != NULL && sr.GetAtom(j).GetMatrix() == NULL )  a_res->Add( sr.GetAtom(j).GetAtom() ); 
          str << "U(" << sr.GetAtom(j).GetFullLabel(*this) << ')';
          if( (j+2) < sr.AtomCount() )
            str << " ~ ";
        }
      }
      str << ": within " << sr.GetValue() << "A with sigma of " << sr.GetEsd() << 
        " and sigma for terminal atoms of " << sr.GetEsd1();
    }
    for( size_t i=0; i < rISOR.Count(); i++ )  {
      TSimpleRestraint& sr = rISOR[i];
      olxstr& str = lst.Add(EmptyString);
      if( sr.IsAllNonHAtoms() )  {
        str << "All non-hydrogen atoms";
      }
      else {
        for( size_t j=0; j < sr.AtomCount(); j++ )  {
          if( sr.GetAtom(j).GetAtom()->GetEllipsoid() == NULL )  continue;
          if( a_res != NULL && sr.GetAtom(j).GetMatrix() == NULL )  a_res->Add( sr.GetAtom(j).GetAtom() ); 
          str << "Uanis(" << sr.GetAtom(j).GetFullLabel(*this) << ") ~ Uiso";
          if( (j+1) < sr.AtomCount() )
            str << ", ";
        }
      }
      str << ": with sigma of " << sr.GetEsd() << " and sigma for terminal atoms of " << sr.GetEsd1();
    }
    for( size_t i=0; i < rEADP.Count(); i++ )  {
      TSimpleRestraint& sr = rEADP[i];
      olxstr& str = lst.Add(EmptyString);
      for( size_t j=0; j < sr.AtomCount(); j++ )  {
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
    for( size_t i=0; i < ExyzGroups.Count(); i++ )  {
      TExyzGroup& sr = ExyzGroups[i];
      olxstr& str = lst.Add('{');
      for( size_t j=0; j < sr.Count(); j++ )  {
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
    for( size_t i=0; i < rSAME.Count(); i++ )  {
      TSameGroup& sg = rSAME[i];
      if( sg.DependentCount() == 0 || !sg.IsValidForSave() )
        continue;
      for( size_t j=0; j < sg.DependentCount(); j++ )  {
        if( !sg.GetDependent(j).IsValidForSave() )
          continue;
        olxstr& str = lst.Add('{');
        str << sg.GetDependent(j)[0].GetLabel();
        for( size_t k=1; k < sg.GetDependent(j).Count(); k++ )
          str << ", " << sg.GetDependent(j)[k].GetLabel();
        str << '}';
      }
      lst.Add("as");
      olxstr& str = lst.Add('{');
      str << sg[0].GetLabel();
      for( size_t j=1; j < sg.Count(); j++ )
        str << ", " << sg[j].GetLabel();
      str << '}';
    }
  }
  TStrList vars;
  Vars.Describe(vars);
  if( !vars.IsEmpty() )  {
    lst.Add(++sec_num) << ". Others";
    lst.AddList(vars);
  }
}
//....................................................................................................
void RefinementModel::ProcessFrags()  {
  // generate missing atoms for the AFIX 59, 66
  olxdict<int, TPtrList<TAfixGroup>, TPrimitiveComparator> a_groups;
  olxdict<int, Fragment*, TPrimitiveComparator> frags;
  for( size_t i=0; i < AfixGroups.Count(); i++ )  {
    TAfixGroup& ag = AfixGroups[i];
    int m = ag.GetM();
    if( !ag.IsFitted() &&  m < 16 )  continue;
    if( m == 7 )  m = 6;
    bool generate = false;
    for( size_t j=0; j < ag.Count(); j++ )  {
      if( ag[j].ccrd().IsNull() )  {
        generate = true;
        a_groups.Add(ag.GetAfix()).Add(ag)->SetAfix(m*10+ag.GetN());
        break;
      }
    }
    if( generate )  {
      if( frags.IndexOf(m) == InvalidIndex )  {
        vec3d_list crds;
        if( m == 5 )
          Fragment::GenerateFragCrds(frag_id_cp, crds);
        else if( m == 6 )
          Fragment::GenerateFragCrds(frag_id_ph, crds);
        else if( m == 10 )
          Fragment::GenerateFragCrds(frag_id_cp_star, crds);
        else if( m == 11 )
          Fragment::GenerateFragCrds(frag_id_naphthalene, crds);
        Fragment& f = AddFrag(m);
        const olxstr label("C");
        for( size_t i=0; i < crds.Count(); i++ )
          f.Add(label, crds[i]);
        frags.Add(m, &f);
      }
    }
  }
  for( size_t i=0; i < Frags.Count(); i++ )  {
    Fragment* frag = Frags.GetValue(i);
    for( size_t j=0; j < AfixGroups.Count(); j++ )  {
      TAfixGroup& ag = AfixGroups[j];
      if( ag.GetM() == frag->GetCode() && (ag.Count()+1) == frag->Count() )  {
        TTypeList<AnAssociation3<TCAtom*, const cm_Element*, bool> > atoms;
        vec3d_list crds;
        TCAtomPList all_atoms(ag.Count()+1);
        all_atoms[0] = &ag.GetPivot();
        for( size_t k=0; k < ag.Count(); k++ )
          all_atoms[k+1] = &ag[k];
        for( size_t k=0; k < all_atoms.Count(); k++ )  {
          atoms.AddNew(all_atoms[k], (const cm_Element*)NULL, all_atoms[k]->ccrd().QLength() > 1e-6);
          crds.AddCCopy((*frag)[k].crd);
        }
        aunit.FitAtoms(atoms, crds, false);
        ag.SetAfix(ag.GetN());
      }
    }
  }
  for( size_t i=0; i < a_groups.Count(); i++ )  {
    TPtrList<TAfixGroup>& gs = a_groups.GetValue(i);
    for( size_t j=0; j < gs.Count(); j++ )
      gs[j]->SetAfix(a_groups.GetKey(i));
  }
  // remove the 'special' frags
  for( size_t i=0; i < frags.Count(); i++ )  {
    const size_t ind = Frags.IndexOf(frags.GetKey(i));
    if( ind == InvalidIndex )  continue;  // ?
    delete Frags.GetValue(ind);
    Frags.Delete(ind);
  }
}
//....................................................................................................
void RefinementModel::ToDataItem(TDataItem& item) {
  // fields
  item.AddField("RefOutArg", PersUtil::NumberListToStr(PLAN));
  item.AddField("Weight", PersUtil::NumberListToStr(used_weight));
  item.AddField("ProposedWeight", PersUtil::NumberListToStr(proposed_weight));
  item.AddField("HklSrc", HKLSource);
  item.AddField("RefMeth", RefinementMethod);
  item.AddField("SolMeth", SolutionMethod);
  item.AddField("BatchScales", PersUtil::NumberListToStr(BASF));
  item.AddField("RefInArg", PersUtil::NumberListToStr(LS));

  // save used equivalent positions
  TArrayList<uint32_t> mat_tags(UsedSymm.Count());
  TDataItem& eqiv = item.AddItem("EQIV");
  for( size_t i=0; i < UsedSymm.Count(); i++ )  {
    eqiv.AddItem(UsedSymm.GetKey(i), TSymmParser::MatrixToSymmEx(UsedSymm.GetValue(i).symop));
    mat_tags[i] = UsedSymm.GetValue(i).symop.GetId();
    UsedSymm.GetValue(i).symop.SetRawId((uint32_t)i);
  }
  
  Vars.ToDataItem(item.AddItem("LEQS"));
  expl.ToDataItem(item.AddItem("EXPL"));  

  AfixGroups.ToDataItem(item.AddItem("AFIX"));
  ExyzGroups.ToDataItem(item.AddItem("EXYZ"));
  rSAME.ToDataItem(item.AddItem("SAME"));
  rDFIX.ToDataItem(item.AddItem("DFIX"));
  rDANG.ToDataItem(item.AddItem("DANG"));
  rSADI.ToDataItem(item.AddItem("SADI"));
  rCHIV.ToDataItem(item.AddItem("CHIV"));
  rFLAT.ToDataItem(item.AddItem("FLAT"));
  rDELU.ToDataItem(item.AddItem("DELU"));
  rSIMU.ToDataItem(item.AddItem("SIMU"));
  rISOR.ToDataItem(item.AddItem("ISOR"));
  rEADP.ToDataItem(item.AddItem("EADP"));
  
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
  Conn.ToDataItem(item.AddItem("CONN"));
  item.AddField("UserContent", GetUserContentStr());
  // restore matrix tags
  for( size_t i=0; i < UsedSymm.Count(); i++ )
    UsedSymm.GetValue(i).symop.SetRawId(mat_tags[i]);
  if( !SfacData.IsEmpty() )  {
    TDataItem& sfacs = item.AddItem("SFAC");
    for( size_t i=0; i < SfacData.Count(); i++ )
      SfacData.GetValue(i)->ToDataItem(sfacs);      
  }
}
//....................................................................................................
void RefinementModel::FromDataItem(TDataItem& item) {
  Clear(rm_clear_ALL);
  PersUtil::FloatNumberListFromStr(item.GetRequiredField("RefOutArg"), PLAN);
  PersUtil::FloatNumberListFromStr(item.GetRequiredField("Weight"), used_weight);
  PersUtil::FloatNumberListFromStr(item.GetRequiredField("ProposedWeight"), proposed_weight);
  HKLSource = item.GetRequiredField("HklSrc");
  RefinementMethod = item.GetRequiredField("RefMeth");
  SolutionMethod = item.GetRequiredField("SolMeth");
  PersUtil::FloatNumberListFromStr(item.GetRequiredField("BatchScales"), BASF);
  for( size_t i=0; i < BASF.Count(); i++ )
    BASF_Vars.Add(NULL);
  PersUtil::IntNumberListFromStr(item.GetRequiredField("RefInArg"), LS);

  TDataItem& eqiv = item.FindRequiredItem("EQIV");
  for( size_t i=0; i < eqiv.ItemCount(); i++ )
    TSymmParser::SymmToMatrix(eqiv.GetItem(i).GetValue(), UsedSymm.Add(eqiv.GetItem(i).GetName()).symop);
  

  expl.FromDataItem(item.FindRequiredItem("EXPL"));  

  AfixGroups.FromDataItem(item.FindRequiredItem("AFIX"));
  ExyzGroups.FromDataItem(item.FindRequiredItem("EXYZ"));
  rSAME.FromDataItem(item.FindRequiredItem("SAME"));
  rDFIX.FromDataItem(item.FindRequiredItem("DFIX"));
  rDANG.FromDataItem(item.FindRequiredItem("DANG"));
  rSADI.FromDataItem(item.FindRequiredItem("SADI"));
  rCHIV.FromDataItem(item.FindRequiredItem("CHIV"));
  rFLAT.FromDataItem(item.FindRequiredItem("FLAT"));
  rDELU.FromDataItem(item.FindRequiredItem("DELU"));
  rSIMU.FromDataItem(item.FindRequiredItem("SIMU"));
  rISOR.FromDataItem(item.FindRequiredItem("ISOR"));
  rEADP.FromDataItem(item.FindRequiredItem("EADP"));

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

  // restraints and BASF may use some of the vars...  
  Vars.FromDataItem( item.FindRequiredItem("LEQS") );
  Conn.FromDataItem( item.FindRequiredItem("CONN") );
  SetUserFormula(item.GetFieldValue("UserContent"), false);
  TDataItem* sfac = item.FindItem("SFAC");
  if( sfac != NULL )  {
    for( size_t i=0; i < sfac->ItemCount(); i++ )  {
      XScatterer* sc = new XScatterer(EmptyString);
      sc->FromDataItem(sfac->GetItem(i));
      SfacData.Add(sc->GetLabel(), sc);
    }
  }
  aunit._UpdateConnInfo();
}
//....................................................................................................
#ifndef _NO_PYTHON
PyObject* RefinementModel::PyExport(bool export_connectivity)  {
  PyObject* main = PyDict_New(), 
    *hklf = PyDict_New(), 
    *eq = PyTuple_New(UsedSymm.Count());
  TPtrList<PyObject> atoms, equivs;
  PythonExt::SetDictItem(main, "aunit", aunit.PyExport(atoms));
  TArrayList<uint32_t> mat_tags(UsedSymm.Count());
  for( size_t i=0; i < UsedSymm.Count(); i++ )  {
    smatd& m = UsedSymm.GetValue(i).symop;
    PyTuple_SetItem(eq, i, 
      equivs.Add(
      Py_BuildValue("(iii)(iii)(iii)(ddd)", m.r[0][0], m.r[0][1], m.r[0][2],
      m.r[1][0], m.r[1][1], m.r[1][2],
      m.r[2][0], m.r[2][1], m.r[2][2],
      m.t[0], m.t[1], m.t[2]
    )) );
    mat_tags[i] = m.GetId();
    m.SetRawId((uint32_t)i);
  }
  PythonExt::SetDictItem(main, "equivalents", eq);

  PythonExt::SetDictItem(main, "variables", Vars.PyExport(atoms));
  PythonExt::SetDictItem(main, "exptl", expl.PyExport());
  PythonExt::SetDictItem(main, "afix", AfixGroups.PyExport(atoms));
  PythonExt::SetDictItem(main, "exyz", ExyzGroups.PyExport(atoms));
  PythonExt::SetDictItem(main, "same", rSAME.PyExport(atoms));
  PythonExt::SetDictItem(main, "dfix", rDFIX.PyExport(atoms, equivs));
  PythonExt::SetDictItem(main, "dang", rDANG.PyExport(atoms, equivs));
  PythonExt::SetDictItem(main, "sadi", rSADI.PyExport(atoms, equivs));
  PythonExt::SetDictItem(main, "chiv", rCHIV.PyExport(atoms, equivs));
  PythonExt::SetDictItem(main, "flat", rFLAT.PyExport(atoms, equivs));
  PythonExt::SetDictItem(main, "delu", rDELU.PyExport(atoms, equivs));
  PythonExt::SetDictItem(main, "simu", rSIMU.PyExport(atoms, equivs));
  PythonExt::SetDictItem(main, "isor", rISOR.PyExport(atoms, equivs));
  PythonExt::SetDictItem(main, "eadp", rEADP.PyExport(atoms, equivs));

  PythonExt::SetDictItem(hklf, "value", Py_BuildValue("i", HKLF));
  PythonExt::SetDictItem(hklf, "s", Py_BuildValue("d", HKLF_s));
  PythonExt::SetDictItem(hklf, "m", Py_BuildValue("d", HKLF_m));
  PythonExt::SetDictItem(hklf, "wt", Py_BuildValue("d", HKLF_wt));
  PythonExt::SetDictItem(hklf, "matrix", 
    Py_BuildValue("(ddd)(ddd)(ddd)", HKLF_mat[0][0], HKLF_mat[0][1], HKLF_mat[0][2],
    HKLF_mat[1][0], HKLF_mat[1][1], HKLF_mat[1][2],
    HKLF_mat[2][0], HKLF_mat[2][1], HKLF_mat[2][2]));
  PythonExt::SetDictItem(main, "hklf", hklf );
  {
    PyObject* uweight = PyTuple_New(used_weight.Count());
    PyObject* pweight = PyTuple_New(proposed_weight.Count());
    for( size_t i=0; i < used_weight.Count(); i++ )
      PyTuple_SetItem(uweight, i, Py_BuildValue("d", used_weight[i]));
    for( size_t i=0; i < proposed_weight.Count(); i++ )
      PyTuple_SetItem(pweight, i, Py_BuildValue("d", proposed_weight[i]));
    PythonExt::SetDictItem(main, "weight", uweight);
    PythonExt::SetDictItem(main, "proposed_weight", pweight);
  }
  {
    PyObject* omit;
    PythonExt::SetDictItem(main, "omit", omit = PyDict_New() );
    PythonExt::SetDictItem(omit, "s", Py_BuildValue("d", OMIT_s));
    PythonExt::SetDictItem(omit, "2theta", Py_BuildValue("d", OMIT_2t));
    if( !Omits.IsEmpty() )  {
      PyObject* omits = PyTuple_New(Omits.Count());
      for( size_t i=0; i < Omits.Count(); i++ )
        PyTuple_SetItem(omits, i, Py_BuildValue("(iii)", Omits[i][0], Omits[i][1], Omits[i][2]) );

      PythonExt::SetDictItem(omit, "hkl", omits);
    }
    PythonExt::SetDictItem(main, "merge", Py_BuildValue("i", MERG));
  }
  if( TWIN_set )  {
    PyObject* twin = PyDict_New(), 
      *basf = PyTuple_New(BASF.Count());
    PythonExt::SetDictItem(twin, "n", Py_BuildValue("i", TWIN_n));
    PythonExt::SetDictItem(twin, "matrix", 
      Py_BuildValue("(ddd)(ddd)(ddd)", TWIN_mat[0][0], TWIN_mat[0][1], TWIN_mat[0][2],
      TWIN_mat[1][0], TWIN_mat[1][1], TWIN_mat[1][2],
      TWIN_mat[2][0], TWIN_mat[2][1], TWIN_mat[2][2]));
    for( size_t i=0; i < BASF.Count(); i++ )
      PyTuple_SetItem(basf, i, Py_BuildValue("d", BASF[i]) );
    PythonExt::SetDictItem(twin, "basf", basf);
    PythonExt::SetDictItem(main, "twin", twin );
  }
  if( SHEL_set )  {
    PyObject* shel;
    PythonExt::SetDictItem(main, "shel", shel = PyDict_New() );
    PythonExt::SetDictItem(shel, "low", Py_BuildValue("d", SHEL_lr));
    PythonExt::SetDictItem(shel, "high", Py_BuildValue("d", SHEL_hr));
  }
  PythonExt::SetDictItem(main, "conn", Conn.PyExport());
  // attach the connectivity...
  if( export_connectivity )  {
    TAtomEnvi ae;
    TLattice& lat = aunit.GetLattice();
    TUnitCell& uc = aunit.GetLattice().GetUnitCell();
    ASObjectProvider& objects = lat.GetObjects();
    for( size_t i=0; i < objects.atoms.Count(); i++ )  {
      TSAtom& sa = objects.atoms[i];
      if( sa.IsDeleted() || sa.GetType() == iQPeakZ )  continue;
      // make sure that only AU atoms go to 
      if( !sa.IsAUAtom() )  continue;
      uc.GetAtomEnviList(sa, ae);
      if( PyDict_GetItemString(atoms[sa.CAtom().GetTag()], "neighbours") != NULL )
        continue;
      PythonExt::SetDictItem(atoms[sa.CAtom().GetTag()], "neighbours", ae.PyExport(atoms) );
      ae.Clear();
    }
  }
  //
  // restore matrix tags
  for( size_t i=0; i < UsedSymm.Count(); i++ )
    UsedSymm.GetValue(i).symop.SetRawId(mat_tags[i]);
  if( !SfacData.IsEmpty() )  {
    PyObject* sfac = PyDict_New();
    for( size_t i=0; i < SfacData.Count(); i++ )
      PythonExt::SetDictItem(sfac, SfacData.GetKey(i).c_str(),
        SfacData.GetValue(i)->PyExport());
    PythonExt::SetDictItem(main, "sfac", sfac);
  }
  return main;
}
#endif
//..............................................................................
bool RefinementModel::Update(const RefinementModel& rm)  {
  if( aunit.GetAngles().DistanceTo(rm.aunit.GetAngles()) > 1e-6 ||
    aunit.GetAxes().DistanceTo(rm.aunit.GetAxes()) > 1e-6 ||
    VarCount() != rm.VarCount() ||
    BASF.Count() != rm.BASF.Count() ||
    aunit.EllpCount() != rm.aunit.EllpCount() ||
    Vars.VarCount() != rm.Vars.VarCount() )
  {
    return false;
  }
  size_t ac = 0;
  aunit.ComplyToResidues();
  for( size_t i=0; i < aunit.AtomCount(); i++ )  {
    TCAtom &this_a = aunit.GetAtom(i);
    if( this_a.IsDeleted() )  continue;
    TCAtom *that_a = &rm.aunit.GetAtom(ac);
    while( that_a->IsDeleted() )  {
      if( ++ac >= rm.aunit.AtomCount() )
        return false;
      that_a = &rm.aunit.GetAtom(ac);
    }
    if( that_a->GetType() != this_a.GetType() )
      return false;
    ac++;
    if( ac >= rm.aunit.AtomCount() )
      return false;
  }
  // need to implement addition of  new atoms, like q-peaks
  ac = 0;
  for( size_t i=0; i < aunit.AtomCount(); i++ )  {
    TCAtom &this_a = aunit.GetAtom(i);
    if( this_a.IsDeleted() )  continue;
    TCAtom &that_a = rm.aunit.GetAtom(ac);
    this_a.ccrd() = that_a.ccrd();
    this_a.ccrdEsd() = that_a.ccrdEsd();
    this_a.SetOccu(that_a.GetOccu());
    this_a.SetOccuEsd(that_a.GetOccuEsd());
    this_a.SetUiso(that_a.GetUiso());
    this_a.SetUisoEsd(that_a.GetUisoEsd());
    ac++;
  }
  for( size_t i=0; i < aunit.EllpCount(); i++ )
    aunit.GetEllp(i) = rm.aunit.GetEllp(i);

  used_weight = rm.used_weight;
  proposed_weight = rm.proposed_weight;
  BASF = rm.BASF;
  for( size_t i=0; i < Vars.VarCount(); i++ )
    Vars.GetVar(i).SetValue(rm.Vars.GetVar(i).GetValue());
  return true;
}
//..............................................................................
//..............................................................................
//..............................................................................
void RefinementModel::LibOSF(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(Vars.VarCount() == 0 ? 0.0 : Vars.GetVar(0).GetValue());
  else  {
    if( Vars.VarCount() == 0 )
      Vars.NewVar(Params[0].ToDouble());
    else
      Vars.GetVar(0).SetValue(Params[0].ToDouble());
  }
}
//..............................................................................
void RefinementModel::LibFVar(const TStrObjList& Params, TMacroError& E)  {
  size_t i = Params[0].ToSizeT();
  if( Vars.VarCount() <= i )  {
    E.ProcessingError(__OlxSrcInfo, "FVar index out of bounds");
    return;
  }
  if( Params.Count() == 1 )
    E.SetRetVal(Vars.GetVar(i).GetValue());
  else
    Vars.GetVar(i).SetValue(Params[1].ToDouble());
}
//..............................................................................
TLibrary* RefinementModel::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("rm") : name);
  lib->RegisterFunction<RefinementModel>(
    new TFunction<RefinementModel>(this, &RefinementModel::LibOSF, "OSF", fpNone|fpOne,
"Returns/sets OSF") );
  lib->RegisterFunction<RefinementModel>(
    new TFunction<RefinementModel>(this, &RefinementModel::LibFVar, "FVar", fpOne|fpTwo,
"Returns/sets FVAR referred by index") );
  return lib;
}
