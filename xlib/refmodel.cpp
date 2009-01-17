#include "refmodel.h"
#include "lattice.h"
#include "symmparser.h"
#include "hkl.h"
#include "symmlib.h"
#include "pers_util.h"
#include "unitcell.h"

RefinementModel::RefinementModel(TAsymmUnit& au) : rDFIX(*this, rltBonds), rDANG(*this, rltBonds), 
  rSADI(*this, rltBonds), rCHIV(*this, rltAtoms), rFLAT(*this, rltGroup), rDELU(*this, rltAtoms), 
  rSIMU(*this, rltAtoms), rISOR(*this, rltAtoms), rEADP(*this, rltAtoms), ExyzGroups(*this), 
  AfixGroups(*this), rSAME(*this),
  aunit(au), HklStatFileID(EmptyString, 0, 0), Vars(au)  {
  SetDefaults();
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
  HKLF_set = MERG_set = OMIT_set = TWIN_set = false;
  TWIN_n = def_TWIN_n;
  TWIN_mat.I() *= -1;
}
//....................................................................................................
void RefinementModel::Clear() {
  for( int i=0; i < SfacData.Count(); i++ )
    delete SfacData.Object(i);
  SfacData.Clear();
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
  UsedSymm.Clear();
  used_weight.Clear();
  proposed_weight.Clear();
  RefinementMethod = "L.S.";
  SolutionMethod = EmptyString;
  HKLSource = EmptyString;
  Omits.Clear();
  BASF.Clear();
  SetDefaults();
  expl.Clear();
  Vars.Clear();
}
//....................................................................................................
const smatd& RefinementModel::AddUsedSymm(const smatd& matr) {
  int ind = UsedSymm.IndexOf(matr);
  smatd* rv = NULL;
  if( ind == -1 )  {
    rv = &UsedSymm.Add( *(new smatd(matr)) );
    rv->SetTag(1);
  }
  else  {
    UsedSymm[ind].IncTag();
    rv = &UsedSymm[ind];
  }
  return *rv;
}
//....................................................................................................
void RefinementModel::RemUsedSymm(const smatd& matr)  {
  int ind = UsedSymm.IndexOf(matr);
  if( ind == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "matrix is not in the list");
  UsedSymm[ind].DecTag();
  if( UsedSymm[ind].GetTag() == 0 )
    UsedSymm.Delete(ind);
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
  Omits = rm.Omits;
  TWIN_mat = rm.TWIN_mat;
  TWIN_n = rm.TWIN_n;
  TWIN_set = rm.TWIN_set;
  BASF = rm.BASF;
  HKLSource = rm.HKLSource;
  RefinementMethod = rm.RefinementMethod;
  SolutionMethod = rm.SolutionMethod;
  if( AssignAUnit )
    aunit.Assign(rm.aunit);
  Vars.Assign( rm.Vars );
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

  for( int i=0; i < rm.UsedSymm.Count(); i++ )
    UsedSymm.AddCCopy( rm.UsedSymm[i] );

  for( int i=0; i < rm.SfacData.Count(); i++ )
    SfacData.Add(rm.SfacData.GetComparable(i), new XScatterer( *rm.SfacData.GetObject(i)) );
  
  
  return *this;
}
//....................................................................................................
void RefinementModel::AddNewSfac(const olxstr& label,
                  double a1, double a2, double a3, double a4,
                  double b1, double b2, double b3, double b4,
                  double c, double mu, double r, double wt)  {
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
  SfacData.Add(label, sc);
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
const RefinementModel::HklStat& RefinementModel::GetMergeStat() {
  // we need to take into the account MERG, HKLF and OMIT things here...
  try {
    TEFile::FileID hkl_src_id = TEFile::GetFileID(HKLSource);
    if( hkl_src_id == HklStatFileID &&
      _HklStat.OMIT_s == OMIT_s &&
      _HklStat.OMIT_2t == OMIT_2t &&
      _HklStat.MERG == MERG )  
    {
      _HklStat.OmittedByUser = Omits.Count();  // this might change beyond the HKL file!
      return _HklStat;
    }
    else  {
      THklFile hf;
      hf.LoadFromFile(HKLSource);
      HklStatFileID = hkl_src_id;
      _HklStat.SetDefaults();

      TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(aunit);
      if( sg == NULL )  // will not be seen outside though
        throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
      TRefPList refs;
      refs.SetCapacity( hf.RefCount() );
      const mat3d& hkl2c = aunit.GetHklToCartesian();
      const double h_o_s = 0.5*OMIT_s;
      double max_d = 2*sin(OMIT_2t*M_PI/360.0)/expl.GetRadiation();
      const double max_qd = QRT(max_d);  // maximum d squared
      const bool transform_hkl = !HKLF_mat.IsI();
      const int ref_cnt = hf.RefCount();
      _HklStat.MinD = 100;
      _HklStat.MaxD = -100;
      TRefPList Refs;
      Refs.SetCapacity(hf.RefCount());
      vec3d new_hkl;
      //apply OMIT transformation and filtering and calculate spacing limits
      for( int i=0; i < ref_cnt; i++ )  {
        TReflection& r = hf[i];
        if( r.GetI() < h_o_s*r.GetS()*r.GetI() )  {
          r.SetI( -h_o_s*r.GetI() );
          _HklStat.IntensityTransformed++;
        }
        if( transform_hkl )  {
          r.MulHkl(new_hkl, HKLF_mat);
          r.SetH( Round(new_hkl[0]) );
          r.SetK( Round(new_hkl[1]) );
          r.SetL( Round(new_hkl[2]) );
        }
        vec3d hkl(r.GetH()*hkl2c[0][0],
                  r.GetH()*hkl2c[0][1] + r.GetK()*hkl2c[1][1],
                  r.GetH()*hkl2c[0][2] + r.GetK()*hkl2c[1][2] + r.GetL()*hkl2c[2][2]);
        const double qd = hkl.QLength();
        if( qd < max_qd ) 
          Refs.Add(&r);
        else
          _HklStat.FilteredOff++;
        if( qd > _HklStat.MaxD ) 
          _HklStat.MaxD = qd;
        if( qd < _HklStat.MinD ) 
          _HklStat.MinD = qd;
      }
      _HklStat.LimD = sqrt(max_qd);
      _HklStat.MaxD = sqrt(_HklStat.MaxD);
      _HklStat.MinD = sqrt(_HklStat.MinD);
      _HklStat.OmittedByUser = Omits.Count();
      _HklStat.MERG = MERG;
      _HklStat.OMIT_s = OMIT_s;
      _HklStat.OMIT_2t = OMIT_2t;
      smatd_list ml;
      TRefList output;
      sg->GetMatrices(ml, mattAll^mattIdentity);
      if( MERG == 4 && !sg->IsCentrosymmetric() )  {  // merge all
        const int mc = ml.Count();
        for( int i=0; i < mc; i++ )
          ml.AddNew( ml[i] ).r *= -1;
        ml.AddNew().r.I() *= -1;
      }
      _HklStat = RefMerger::Merge<TSimpleMerger>(ml, Refs, output);
    }
  }
  catch(TExceptionBase&)  {
    _HklStat.SetDefaults();
  }
  return _HklStat;
}
//....................................................................................................
void RefinementModel::Describe(TStrList& lst) {
  Validate();
  int sec_num = 0;
  if( (rDFIX.Count()|rDANG.Count()|rSADI.Count()) != 0 )  {
    sec_num++;
    lst.Add(olxstr(sec_num)) << ". Restrained distances";
    for( int i=0; i < rDFIX.Count(); i++ )  {
      TSimpleRestraint& sr = rDFIX[i];
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
    sec_num++;
    lst.Add(olxstr(sec_num)) << ". Restrained atomic chiral volume";
    for( int i=0; i < rCHIV.Count(); i++ )  {
      TSimpleRestraint& sr = rCHIV[i];
      olxstr& str = lst.Add(EmptyString);
      for( int j=0; j < sr.AtomCount(); j++ )  {
        str << sr.GetAtom(j).GetFullLabel(*this);
        if( (j+1) < sr.AtomCount() )
          str << ", ";
      }
      str << ": fixed at " << sr.GetValue() << " with sigma of " << sr.GetEsd();
    }
  }
  if( rDELU.Count() != 0 )  {
    sec_num++;
    lst.Add(olxstr(sec_num)) << ". Rigid bond restraints";
    for( int i=0; i < rDELU.Count(); i++ )  {
      TSimpleRestraint& sr = rDELU[i];
      if( sr.GetEsd() == 0 || sr.GetEsd1() == 0 )  continue;
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
    sec_num++;
    lst.Add(olxstr(sec_num)) << ". Uiso/Uaniso restraints and constraints";
    for( int i=0; i < rSIMU.Count(); i++ )  {
      TSimpleRestraint& sr = rSIMU[i];
      olxstr& str = lst.Add(EmptyString);
      if( sr.IsAllNonHAtoms() )  {
        str << "All non-hydrogen atoms";
      }
      else {
        for( int j=0; j < sr.AtomCount(); j++ )  {
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
        if( sr.GetAtom(j).GetAtom()->GetEllipsoid() == NULL )  continue;
        str << "U(" << sr.GetAtom(j).GetFullLabel(*this) << ')';
        if( (j+1) < sr.AtomCount() )
          str << " = ";
      }
    }
  }
  if( ExyzGroups.Count() != 0 )  {
    sec_num++;
    lst.Add(olxstr(sec_num)) << ". Shared sites";
    for( int i=0; i < ExyzGroups.Count(); i++ )  {
      TExyzGroup& sr = ExyzGroups[i];
      olxstr& str = lst.Add('{');
      for( int j=0; j < sr.Count(); j++ )  {
        str << sr[j].GetLabel();
        if( (j+1) < sr.Count() )
          str << ", ";
      }
      str << '}';
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
  TDataItem& eqiv = item.AddItem("eqiv");
  for( int i=0; i < UsedSymm.Count(); i++ )  
    eqiv.AddItem(i, TSymmParser::MatrixToSymmEx(UsedSymm[i]));
  
  Vars.ToDataItem(item.AddItem("leqs"));
  expl.ToDataItem(item.AddItem("expl"));  

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

  TDataItem& eqiv = item.FindRequiredItem("eqiv");
  for( int i=0; i < eqiv.ItemCount(); i++ )  
    TSymmParser::SymmToMatrix( eqiv.GetItem(i).GetValue(), UsedSymm.AddNew());

  Vars.FromDataItem( item.FindRequiredItem("leqs") );
  expl.FromDataItem(item.FindRequiredItem("expl"));  

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
}
//....................................................................................................
#ifndef _NO_PYTHON
PyObject* RefinementModel::PyExport(bool export_connectivity)  {
  PyObject* main = PyDict_New(), 
    *hklf = PyDict_New(), 
    *eq = PyTuple_New(UsedSymm.Count());
  TPtrList<PyObject> atoms, equivs;
  PyDict_SetItemString(main, "aunit", aunit.PyExport(atoms) );

  for( int i=0; i < UsedSymm.Count(); i++ )  {
    const smatd& m = UsedSymm[i];
    PyTuple_SetItem(eq, i, 
      equivs.Add(
        Py_BuildValue("(ddd)(ddd)(ddd)(ddd)", m.r[0][0], m.r[0][1], m.r[0][2],
          m.r[1][0], m.r[1][1], m.r[1][2],
          m.r[2][0], m.r[2][1], m.r[2][2],
          m.t[0], m.t[1], m.t[2]
      )) );
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
  return main;
}
#endif
