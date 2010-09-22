//---------------------------------------------------------------------------//
// CIF and related data management procedures
// (c) Oleg V. Dolomanov, 2004-2010
//---------------------------------------------------------------------------//
#include "cif.h"
#include "dataitem.h"
#include "catom.h"
#include "satom.h"
#include "symmparser.h"
#include "unitcell.h"
#include "ellipsoid.h"
#include "bapp.h"
#include "log.h"
#include "symmlib.h"
#include "etime.h"

using namespace exparse::parser_util;
using namespace cif_dp;

//----------------------------------------------------------------------------//
// TCif function bodies
//----------------------------------------------------------------------------//
TCif::TCif() : block_index(InvalidIndex)  {  }
//..............................................................................
TCif::~TCif()  {  Clear();  }
//..............................................................................
void TCif::Clear()  {
  WeightA.SetLength(0);
  WeightB.SetLength(0);
  GetRM().Clear(rm_clear_ALL);
  GetAsymmUnit().Clear();
  DataManager.Clear();
  Matrices.Clear();
  MatrixMap.Clear();
}
//..............................................................................
void TCif::LoadFromStrings(const TStrList& Strings)  {
  block_index = 0;
  data_provider.LoadFromStrings(Strings);
  for( size_t i=0; i < data_provider.Count(); i++ )  {
    CifBlock& cb = data_provider[i];
    if( cb.param_map.IndexOf("_cell_length_a") == InvalidIndex )
      continue;
    bool valid = false;
    for( size_t j=0; j < cb.table_map.Count(); j++ )  {
      if( cb.table_map.GetKey(j).StartsFrom("_atom_site") )  {
        valid = true;
        break;
      }
    }
    if( valid )  {
      block_index = i;
      break;
    }
  }
  //_LoadCurrent();
}
//..............................................................................
void TCif::_LoadCurrent()  {
  if( block_index == InvalidIndex )  {
    if( data_provider.Count() > 1 || data_provider.Count() == 0 )
      throw TFunctionFailedException(__OlxSourceInfo, "could not locate required data");
    block_index = 0;
    return;  // nothing to initialise anyway... must be a dummy CIF
  }
  CifBlock& cif_data = data_provider[block_index];
  Clear();
  /*search for the weigting sceme*************************************/
  const size_t ws_i = cif_data.param_map.IndexOf("_refine_ls_weighting_details");
  if( ws_i != InvalidIndex )  {
    IStringCifEntry* ci = dynamic_cast<IStringCifEntry*>(cif_data.param_map.GetValue(ws_i));
    if( ci != NULL && ci->Count() == 1 )  {
      const olxstr& tmp = (*ci)[0];
      for( size_t k=0; k < tmp.Length(); k++ )  {
        if( tmp[k] == '+' )  {
          if( WeightA.IsEmpty() )  {
            const size_t st = k+2;
            while( tmp[k] != ')' && ++k < tmp.Length() )  ;
            WeightA = tmp.SubString(st, --k-st);
          }
          else if( WeightB.IsEmpty() )  {
            const size_t st = k;
            while( tmp[k] != ']' && ++k < tmp.Length() )  ;
            WeightB = tmp.SubString(st, k-st-1);
          }
          else
            break;
        }
      }
    }
  }
  Initialize();
}
//..............................................................................
void TCif::SaveToStrings(TStrList& Strings)  {
  if( block_index == InvalidIndex )  return;
  GetAsymmUnit().ComplyToResidues();
  for( size_t i=0; i < data_provider[block_index].table_map.Count(); i++ )
    data_provider[block_index].table_map.GetValue(i)->Sort();
  data_provider[block_index].Sort(TStrList(), TStrList());
  data_provider.SaveToStrings(Strings);
}
//..............................................................................
olxstr TCif::GetParamAsString(const olxstr &Param) const {
  if( block_index == InvalidIndex )  return EmptyString;
  IStringCifEntry* ce = dynamic_cast<IStringCifEntry*>(
    data_provider[block_index].param_map.Find(Param, NULL));
  if( ce == NULL || ce->Count() == 0 )
    return EmptyString;
  olxstr rv = (*ce)[0];
  for( size_t i = 1; i < ce->Count(); i++ )
    rv << '\n' << (*ce)[i];
  return rv;
}
//..............................................................................
void TCif::SetParam(const olxstr& name, const ICifEntry& value)  {
  if( block_index == InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
  data_provider[block_index].Add(value.Replicate());
}
//..............................................................................
void TCif::ReplaceParam(const olxstr& old_name, const olxstr& new_name, const ICifEntry& value)  {
  if( block_index == InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
  data_provider[block_index].Remove(old_name);
  data_provider[block_index].Add(value.Replicate());
}
//..............................................................................
void TCif::Rename(const olxstr& old_name, const olxstr& new_name)  {
  data_provider[block_index].Rename(old_name, new_name);
}
//..............................................................................
void TCif::Initialize()  {
  olxstr Param;
  cetTable *ALoop, *Loop;
  double Q[6], E[6]; // quadratic form of ellipsoid
  TEValueD EValue;
  try  {
    try  {
      const olxstr mx = GetParamAsString("_exptl_crystal_size_max");
      if( mx.IsNumber() )  {
        const olxstr md = GetParamAsString("_exptl_crystal_size_mid"),
          mn = GetParamAsString("_exptl_crystal_size_min");
        if( md.IsNumber() && mn.IsNumber() )
          GetRM().expl.SetCrystalSize(mx.ToDouble(), md.ToDouble(), mn.ToDouble());
      }
      const olxstr temp = GetParamAsString("_diffrn_ambient_temperature");
      if( temp != '?' )
        GetRM().expl.SetTempValue(TEValueD(temp));
      const olxstr radiation = GetParamAsString("_diffrn_radiation_wavelength");
      if( radiation != '?' )
        GetRM().expl.SetRadiation(radiation.ToDouble());
    }
    catch(...)  {}
    
    GetAsymmUnit().Axes()[0] = GetParamAsString("_cell_length_a");
    GetAsymmUnit().Axes()[1] = GetParamAsString("_cell_length_b");
    GetAsymmUnit().Axes()[2] = GetParamAsString("_cell_length_c");

    GetAsymmUnit().Angles()[0] = GetParamAsString("_cell_angle_alpha");
    GetAsymmUnit().Angles()[1] = GetParamAsString("_cell_angle_beta");
    GetAsymmUnit().Angles()[2] = GetParamAsString("_cell_angle_gamma");
    if( ParamExists("_cell_formula_units_Z") )
      GetAsymmUnit().SetZ((short)olx_round(GetParamAsString("_cell_formula_units_Z").ToDouble()));
  }
  catch(...)  {
    TBasicApp::GetLog().Error("Given CIF data block does not contain cell parameters");
    return;
  }
  // check if the cif file contains valid parameters
  if( GetAsymmUnit().CalcCellVolume() == 0 )
    return;

  GetAsymmUnit().InitMatrices();

  Loop = FindLoop("_space_group_symop");
  if( Loop == NULL )
    Loop = FindLoop("_space_group_symop_operation_xyz");
  if( Loop != NULL  )  {
    size_t sindex = Loop->ColIndex("_space_group_symop_operation_xyz");
    size_t iindex = Loop->ColIndex("_space_group_symop_id");
    if( sindex != InvalidIndex )  {
      for( size_t i=0; i < Loop->RowCount(); i++ )  {
        if( !TSymmParser::SymmToMatrix(Loop->Get(i, sindex).GetStringValue(), Matrices.AddNew()) )
          throw TFunctionFailedException(__OlxSourceInfo, "could not process symmetry matrix");
        if( iindex == InvalidIndex )
          MatrixMap.Add(i+1, i);
        else
          MatrixMap.Add(Loop->Get(i, iindex).GetStringValue(), i);
      }
    }
  }
  else  {
    Loop = FindLoop("_symmetry_equiv_pos");
    if( Loop == NULL )
      Loop = FindLoop("_symmetry_equiv_pos_as_xyz");
    if( Loop != NULL  )  {
      cetTable& symop_tab = AddLoopDef("_space_group_symop_id,_space_group_symop_operation_xyz");
      size_t sindex = Loop->ColIndex("_symmetry_equiv_pos_as_xyz");
      size_t iindex = Loop->ColIndex("_symmetry_equiv_pos_site_id");
      if( sindex != InvalidIndex )  {
        for( size_t i=0; i < Loop->RowCount(); i++ )  {
          if( !TSymmParser::SymmToMatrix(Loop->Get(i, sindex).GetStringValue(), Matrices.AddNew()) )
            throw TFunctionFailedException(__OlxSourceInfo, "could not process symmetry matrix");
          CifRow& row = symop_tab.AddRow();
          if( iindex == InvalidIndex )  {
            MatrixMap.Add(i+1, i);
            row[0] = new cetString(i+1);
          }
          else  {
            MatrixMap.Add(Loop->Get(i, iindex).GetStringValue(), i);
            row[0] = Loop->Get(i, iindex).Replicate();
          }
          row[1] = Loop->Get(i, sindex).Replicate();
        }
      }
      // remove obsolete loop
      data_provider[block_index].Remove(*Loop);
    }
  }
  TSpaceGroup* sg = TSymmLib::GetInstance().FindSymSpace(Matrices);
  if( sg != NULL )
    GetAsymmUnit().ChangeSpaceGroup(*sg);
  else   {
    GetAsymmUnit().ChangeSpaceGroup(*TSymmLib::GetInstance().FindGroup("P1"));
    //throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  }
  
  try  {
    GetRM().SetUserFormula(olxstr::DeleteChars(GetParamAsString("_chemical_formula_sum"), ' '));
  }
  catch(...)  {  }
  
  this->Title = GetDataName().ToUpperCase();
  this->Title << " OLEX2: imported from CIF";

  ALoop = FindLoop("_atom_site");
  if( ALoop == NULL )  return;

  size_t ALabel = ALoop->ColIndex("_atom_site_label");
  const size_t ACi[] = {
    ALoop->ColIndex("_atom_site_fract_x"),
    ALoop->ColIndex("_atom_site_fract_y"),
    ALoop->ColIndex("_atom_site_fract_z")
  };
  const size_t ACUiso =  ALoop->ColIndex("_atom_site_U_iso_or_equiv");
  const size_t ASymbol = ALoop->ColIndex("_atom_site_type_symbol");
  const size_t APart   = ALoop->ColIndex("_atom_site_disorder_group");
  const size_t SiteOccu = ALoop->ColIndex("_atom_site_occupancy");
  const size_t Degen = ALoop->ColIndex("_atom_site_symmetry_multiplicity");
  const size_t Part = ALoop->ColIndex("_atom_site_disorder_group");
  if( (ALabel|ACi[0]|ACi[1]|ACi[2]|ASymbol) == InvalidIndex )  {
    TBasicApp::GetLog().Error("Failed to locate required fields in atoms loop");
    return;
  }
  for( size_t i=0; i < ALoop->RowCount(); i++ )  {
    TCAtom& A = GetAsymmUnit().NewAtom();
    A.SetLabel(ALoop->Get(i, ALabel).GetStringValue(), false);
    cm_Element* type = XElementLib::FindBySymbol(ALoop->Get(i, ASymbol).GetStringValue());
    if( type == NULL )  {
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Undefined element: ") <<
        ALoop->Get(i, ASymbol).GetStringValue());
    }
    A.SetType(*type);
    for( int j=0; j < 3; j++ )  {
      EValue = ALoop->Get(i, ACi[j]).GetStringValue();
      A.ccrd()[j] = EValue.GetV();  A.ccrdEsd()[j] = EValue.GetE();
      if( EValue.GetE() == 0 )
        GetRM().Vars.FixParam(A, catom_var_name_X+j);
    }
    if( ACUiso != InvalidIndex )    {
      EValue = ALoop->Get(i, ACUiso).GetStringValue();
      A.SetUisoEsd(EValue.GetE());
      A.SetUiso(EValue.GetV());
      if( EValue.GetE() == 0 )  GetRM().Vars.FixParam(A, catom_var_name_Uiso);
    }
    if( APart != InvalidIndex && ALoop->Get(i, APart).GetStringValue().IsNumber() )
      A.SetPart(ALoop->Get(i, APart).GetStringValue().ToInt());
    if( SiteOccu != InvalidIndex )  {
      EValue = ALoop->Get(i, SiteOccu).GetStringValue();
      A.SetOccu(EValue.GetV());
      A.SetOccuEsd(EValue.GetE());
      if( EValue.GetE() == 0 )  GetRM().Vars.FixParam(A, catom_var_name_Sof);
    }
    if( Degen != InvalidIndex )
      A.SetOccu(A.GetOccu()/ALoop->Get(i, Degen).GetStringValue().ToDouble());
    ALoop->Set(i, ALabel, new AtomCifEntry(A));
    if( Part != InvalidIndex )
      ALoop->Set(i, Part, new AtomPartCifEntry(A));
  }
  for( size_t i=0; i < LoopCount(); i++ )  {
    if( &GetLoop(i) == ALoop )  continue;
    cetTable& tab = GetLoop(i);
    for( size_t j=0; j < tab.ColCount(); j++ )  {
      if(  tab.ColName(j).IndexOf("atom_site") != InvalidIndex &&
        tab.ColName(j).IndexOf("label") != InvalidIndex )
      {
        for( size_t k=0; k < tab.RowCount(); k++ )  {
          TCAtom* ca = GetAsymmUnit().FindCAtom(tab[k][j]->GetStringValue());
          if( ca != NULL )
            tab.Set(k, j, new AtomCifEntry(*ca));
        }
      }
    }
  }

  ALoop = FindLoop("_atom_site_aniso");
  if( ALoop == NULL )  return;
  ALabel =  ALoop->ColIndex("_atom_site_aniso_label");
  const size_t Ui[] = {
    ALoop->ColIndex("_atom_site_aniso_U_11"),
    ALoop->ColIndex("_atom_site_aniso_U_22"),
    ALoop->ColIndex("_atom_site_aniso_U_33"),
    ALoop->ColIndex("_atom_site_aniso_U_23"),
    ALoop->ColIndex("_atom_site_aniso_U_13"),
    ALoop->ColIndex("_atom_site_aniso_U_12")
  };
  if( (ALabel|Ui[0]|Ui[1]|Ui[2]|Ui[3]|Ui[4]|Ui[5]) != InvalidIndex )  {
    for( size_t i=0; i < ALoop->RowCount(); i++ )  {
      TCAtom* A = GetAsymmUnit().FindCAtom(ALoop->Get(i, ALabel).GetStringValue());
      if( A == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("wrong atom in the aniso loop ") << ALabel);
      for( int j=0; j < 6; j++ )  {
        EValue = ALoop->Get(i, Ui[j]).GetStringValue();
        Q[j] = EValue.GetV();  E[j] = EValue.GetE();
        if( EValue.GetE() == 0 )
          GetRM().Vars.FixParam(*A, catom_var_name_U11+j);
      }
      GetAsymmUnit().UcifToUcart(Q);
      A->AssignEllp(&GetAsymmUnit().NewEllp().Initialise(Q, E));
    }
  }
  // geometric parameters
  ALoop = FindLoop("_geom_bond");
  if( ALoop != NULL )  {
    const size_t ALabel =  ALoop->ColIndex("_geom_bond_atom_site_label_1");
    const size_t ALabel1 = ALoop->ColIndex("_geom_bond_atom_site_label_2");
    const size_t BD =      ALoop->ColIndex("_geom_bond_distance");
    const size_t SymmA = ALoop->ColIndex("_geom_bond_site_symmetry_2");
    if( (ALabel|ALabel1|BD|SymmA) != InvalidIndex )  {
      TEValueD ev;
      for( size_t i=0; i < ALoop->RowCount(); i++ )  {
        const CifRow& Row = (*ALoop)[i];
        ACifValue* cv = NULL;
        ev = Row[BD]->GetStringValue();
        if( Row[SymmA]->GetStringValue() == '.' )  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]->GetStringValue()),
            ev);
        }
        else  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]->GetStringValue()),
            SymmCodeToMatrix(Row[SymmA]->GetStringValue()),
            ev);
        }
        DataManager.AddValue(cv);
      }
    }
  }
  ALoop = FindLoop("_geom_hbond");
  if( ALoop != NULL )  {
    const size_t ALabel =  ALoop->ColIndex("_geom_hbond_atom_site_label_D");
    const size_t ALabel1 = ALoop->ColIndex("_geom_hbond_atom_site_label_A");
    const size_t BD =      ALoop->ColIndex("_geom_hbond_distance_DA");
    const size_t SymmA =   ALoop->ColIndex("_geom_hbond_site_symmetry_A");
    if( (ALabel|ALabel1|BD|SymmA) != InvalidIndex )  {
      TEValueD ev;
      for( size_t i=0; i < ALoop->RowCount(); i++ )  {
        const CifRow& Row = (*ALoop)[i];
        ACifValue* cv = NULL;
        ev = Row[BD]->GetStringValue();
        if( Row[SymmA]->GetStringValue() == '.' )  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]->GetStringValue()),
            ev);
        }
        else  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]->GetStringValue()),
            SymmCodeToMatrix(Row[SymmA]->GetStringValue()),
            ev);
        }
        DataManager.AddValue(cv);
      }
    }
  }
  ALoop = FindLoop("_geom_angle");
  if( ALoop != NULL )  {
    const size_t ind_l =  ALoop->ColIndex("_geom_angle_atom_site_label_1");
    const size_t ind_m =  ALoop->ColIndex("_geom_angle_atom_site_label_2");
    const size_t ind_r =  ALoop->ColIndex("_geom_angle_atom_site_label_3");
    const size_t ind_a =  ALoop->ColIndex("_geom_angle");
    const size_t ind_sl = ALoop->ColIndex("_geom_angle_site_symmetry_1");
    const size_t ind_sr = ALoop->ColIndex("_geom_angle_site_symmetry_3");
    if( (ind_l|ind_m|ind_r|ind_a|ind_sl|ind_sr) != InvalidIndex )  {
      TEValueD ev;
      smatd im;
      im.I();
      for( size_t i=0; i < ALoop->RowCount(); i++ )  {
        const CifRow& Row = (*ALoop)[i];
        ACifValue* cv = NULL;
        ev = Row[ind_a]->GetStringValue();
        if( Row[ind_sl]->GetStringValue() == '.' && Row[ind_sr]->GetStringValue() == '.' )  {
          cv = new CifAngle(
            *GetAsymmUnit().FindCAtom(Row[ind_l]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ind_m]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ind_r]->GetStringValue()),
            ev);
        }
        else  {
          cv = new CifAngle(
            *GetAsymmUnit().FindCAtom(Row[ind_l]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ind_m]->GetStringValue()),
            *GetAsymmUnit().FindCAtom(Row[ind_r]->GetStringValue()),
            Row[ind_sl]->GetStringValue() == '.' ? im : SymmCodeToMatrix(Row[ind_sl]->GetStringValue()),
            Row[ind_sr]->GetStringValue() == '.' ? im : SymmCodeToMatrix(Row[ind_sr]->GetStringValue()),
            ev);
        }
        DataManager.AddValue(cv);
      }
    }
  }
}
//..............................................................................
cetTable* TCif::LoopFromDef(CifBlock& dp, const TStrList& col_names)  {
  cetTable* tab = new cetTable();
  for( size_t i=0; i < col_names.Count(); i++ )
    tab->AddCol(col_names[i]);
  return &(cetTable&)dp.Add(tab);
}
//..............................................................................
cetTable& TCif::AddLoopDef(const olxstr& col_names)  {
  TStrList toks(col_names, ',');
  olxstr name = cetTable::GenerateName(toks);
  cetTable *CF = FindLoop(name);
  if( CF != NULL )  {
    for( size_t i=0; i < toks.Count(); i++ )
      if( CF->ColIndex(toks[i]) == InvalidIndex )
        CF->AddCol(toks[i]);
    return *CF;
  }
  return *LoopFromDef(data_provider[block_index], toks);
}
//..............................................................................
cetTable& TCif::GetPublicationInfoLoop()  {
  const static olxstr publ_ln( "_publ_author" ), publ_jn("_publ_requested_journal");
  cetTable *CF = FindLoop(publ_ln);
  if( CF != NULL )  return *CF;
  // to make the automatic grouping to work ...
  if( !ParamExists(publ_jn) )
    data_provider[block_index].Add(new cetNamedString(publ_jn, "?"));
  return *LoopFromDef(data_provider[block_index],
    "_publ_author_name,_publ_author_email,_publ_author_address");
}
//..............................................................................
bool TCif::Adopt(TXFile& XF)  {
  Clear();
  double Q[6], E[6];  // quadratic form of s thermal ellipsoid
  GetRM().Assign(XF.GetRM(), true);
  GetAsymmUnit().SetZ((short)XF.GetLattice().GetUnitCell().MatrixCount());
  Title = TEFile::ChangeFileExt(TEFile::ExtractFileName(XF.GetFileName()), EmptyString);

  block_index = 0;
  data_provider.Add(Title);
  SetParam("_audit_creation_method", "OLEX2", true);
  SetParam("_chemical_name_systematic", "?", true);
  SetParam("_chemical_name_common", "?", true);
  SetParam("_chemical_melting_point", "?", false);
  SetParam("_chemical_formula_moiety", XF.GetLattice().CalcMoiety(), true);
  SetParam("_chemical_formula_sum", GetAsymmUnit().SummFormula(' ', false), true);
  SetParam("_chemical_formula_weight", olxstr::FormatFloat(2, GetAsymmUnit().MolWeight()), false);

  SetParam("_cell_length_a", GetAsymmUnit().Axes()[0].ToString(), false);
  SetParam("_cell_length_b", GetAsymmUnit().Axes()[1].ToString(), false);
  SetParam("_cell_length_c", GetAsymmUnit().Axes()[2].ToString(), false);

  SetParam("_cell_angle_alpha", GetAsymmUnit().Angles()[0].ToString(), false);
  SetParam("_cell_angle_beta",  GetAsymmUnit().Angles()[1].ToString(), false);
  SetParam("_cell_angle_gamma", GetAsymmUnit().Angles()[2].ToString(), false);
  SetParam("_cell_volume", XF.GetUnitCell().CalcVolumeEx().ToString(), false);
  SetParam("_cell_formula_units_Z", XF.GetAsymmUnit().GetZ(), false);

  SetParam("_diffrn_ambient_temperature",
    XF.GetRM().expl.IsTemperatureSet() ? XF.GetRM().expl.GetTempValue().ToString() : olxstr('?'), false);
  SetParam("_diffrn_radiation_wavelength", XF.GetRM().expl.GetRadiation(), false);
  if( XF.GetRM().expl.GetCrystalSize().QLength() > 1.e-6 )  {
    SetParam("_exptl_crystal_size_max", XF.GetRM().expl.GetCrystalSize()[0], false);
    SetParam("_exptl_crystal_size_mid", XF.GetRM().expl.GetCrystalSize()[1], false);
    SetParam("_exptl_crystal_size_min", XF.GetRM().expl.GetCrystalSize()[2], false);
  }
  // HKL section
  const RefinementModel::HklStat& hkl_stat = XF.GetRM().GetMergeStat();
  SetParam("_diffrn_reflns_number", hkl_stat.TotalReflections-hkl_stat.SystematicAbsentcesRemoved, false);
  SetParam("_reflns_number_total", hkl_stat.UniqueReflections, false);
  const char* hkl = "hkl";
  for( int i=0; i < 3; i++ )  {
    SetParam(olxstr("_diffrn_reflns_limit_") << hkl[i] << "_min", hkl_stat.FileMinInd[i], false);
    SetParam(olxstr("_diffrn_reflns_limit_") << hkl[i] << "_max", hkl_stat.FileMaxInd[i], false);
  }
  if( hkl_stat.MaxD > 0 )
    SetParam("_diffrn_reflns_theta_min",
      olxstr::FormatFloat(2, asin(XF.GetRM().expl.GetRadiation()/(2*hkl_stat.MaxD))*180/M_PI), false);
  if( hkl_stat.MinD > 0 )
    SetParam("_diffrn_reflns_theta_max",
      olxstr::FormatFloat(2, asin(XF.GetRM().expl.GetRadiation()/(2*hkl_stat.MinD))*180/M_PI), false);
  SetParam("_diffrn_reflns_av_R_equivalents", olxstr::FormatFloat(4, hkl_stat.Rint), false);
  SetParam("_diffrn_reflns_av_sigmaI/netI", olxstr::FormatFloat(4, hkl_stat.Rsigma), false);
  if( XF.GetAsymmUnit().IsQPeakMinMaxInitialised() )
    SetParam("_refine_diff_density_max", XF.GetAsymmUnit().GetMaxQPeak(), false);
  TSpaceGroup& sg = XF.GetLastLoaderSG();
  SetParam("_space_group_crystal_system", sg.GetBravaisLattice().GetName().ToLowerCase(), true);
  SetParam("_space_group_name_H-M_alt", sg.GetFullName(), true);
  SetParam("_space_group_name_Hall", sg.GetHallSymbol(), true);
  SetParam("_space_group_IT_number", sg.GetNumber(), false);
  {
    cetTable& Loop = AddLoopDef("_space_group_symop_id,_space_group_symop_operation_xyz");
    sg.GetMatrices(Matrices, mattAll);
    for( size_t i=0; i < Matrices.Count(); i++ )  {
      CifRow& row = Loop.AddRow();
      row[0] = new cetString(i+1);
      row[1] = new cetString(TSymmParser::MatrixToSymm(Matrices[i]));
    }
  }

  SetParam("_computing_structure_solution", "?", true);
  SetParam("_computing_molecular_graphics", "?", true);
  SetParam("_computing_publication_material", "?", true);

  SetParam("_atom_sites_solution_primary", "?", false);

  cetTable& atom_loop = AddLoopDef(
    "_atom_site_label,_atom_site_type_symbol,_atom_site_fract_x,"
    "_atom_site_fract_y,_atom_site_fract_z,_atom_site_U_iso_or_equiv,"
    "_atom_site_adp_type,_atom_site_occupancy,_atom_site_refinement_flags_posn,"
    "_atom_site_symmetry_multiplicity,_atom_site_disorder_group");

  cetTable& u_loop = AddLoopDef(
    "_atom_site_aniso_label,_atom_site_aniso_U_11,"
    "_atom_site_aniso_U_22,_atom_site_aniso_U_33,_atom_site_aniso_U_23,"
    "_atom_site_aniso_U_13,_atom_site_aniso_U_12");

  for( size_t i = 0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& A = GetAsymmUnit().GetAtom(i);
    if( A.IsDeleted() || A.GetType() == iQPeakZ )  continue;
    CifRow& Row = atom_loop.AddRow();
    Row[0] = new cetString(A.GetLabel());
    Row[1] = new cetString(A.GetType().symbol);
    for( int j=0; j < 3; j++ )
      Row.Set(j+2, new cetString(TEValueD(A.ccrd()[j], A.ccrdEsd()[j]).ToString()));
    Row.Set(5, new cetString(TEValueD(A.GetUiso(), A.GetUisoEsd()).ToString()));
    Row.Set(6, new cetString(A.GetEllipsoid() == NULL ? "Uiso" : "Uani"));
    Row.Set(7, new cetString(TEValueD(A.GetOccu()*A.GetDegeneracy(), A.GetOccuEsd()).ToString()));
    if( A.GetParentAfixGroup() != NULL && A.GetParentAfixGroup()->IsRiding() )
      Row.Set(8, new cetString("R"));
    else
      Row.Set(8, new cetString('.'));
    Row.Set(9, new cetString(A.GetDegeneracy()));
    // process part as well
    if( A.GetPart() != 0 )
      Row[10] = new cetString((int)A.GetPart());
    else
      Row[10] = new cetString('.');
    if( A.GetEllipsoid() != NULL )  {
      A.GetEllipsoid()->GetQuad(Q, E);
      GetAsymmUnit().UcartToUcif(Q);
      CifRow& Row1 = u_loop.AddRow();
      Row1[0] = new AtomCifEntry(A);
      for( int j=0; j < 6; j++ )
        Row1.Set(j+1, new cetString(TEValueD(Q[j], E[j]).ToString()));
    }
  }
  return true;
}
//..............................................................................
smatd TCif::SymmCodeToMatrix(const olxstr &Code) const {
  size_t ui = Code.LastIndexOf('_');
  if( ui == InvalidIndex )
    return GetMatrixById(Code);
  smatd mSymm = GetMatrixById(Code.SubStringTo(ui));
  olxstr str_t = Code.SubStringFrom(ui+1);
  if( str_t.Length() != 3 )
    return mSymm;
  mSymm.t[0] += (int)(str_t.CharAt(0)-'5');
  mSymm.t[1] += (int)(str_t.CharAt(1)-'5');
  mSymm.t[2] += (int)(str_t.CharAt(2)-'5');
  return mSymm;
}
//..............................................................................
bool TCif::ResolveParamsFromDictionary(TStrList &Dic, olxstr &String,
 olxch Quote,
 olxstr (*ResolveExternal)(const olxstr& valueName),
 bool DoubleTheta) const
{
  size_t start, end;
  for( size_t i=0; i < String.Length(); i++ )  {
    if( String.CharAt(i) == Quote )  {
      if( (i+1) < String.Length() && String.CharAt(i+1) == Quote )  {
        String.Delete(i, 1);
        continue;
      }
      if( i > 0 && String.CharAt(i-1) == '\\' )  // escaped?
        continue;
      olxstr Val;
      if( (i+1) < String.Length() &&
          (String.CharAt(i+1) == '$' || String.CharAt(i+1) == '_' ||
          olxstr::o_isdigit(String.CharAt(i+1))) )
      {
        start = i;
        while( ++i < String.Length() )  {
          if( String.CharAt(i) == Quote )  {
            if( (i+1) < String.Length() && String.CharAt(i+1) == Quote )  {
              String.Delete(i, 1);
              Val << Quote;
              continue;
            }
            else if( String.CharAt(i-1) == '\\' ) // escaped?
              ;
            else  {
              end = i;  
              break;
            }
          }
          Val << String.CharAt(i);
        }
      }
      if( !Val.IsEmpty() )  {
        if( !Val.IsNumber() )  {
          if( Val.CharAt(0) == '$' )  {
            if( ResolveExternal != NULL )  {
              String.Delete(start, end-start+1);
              Val.Replace("\\%", '%');
              ResolveParamsFromDictionary(Dic, Val, Quote, ResolveExternal);
              olxstr Tmp = ResolveExternal(Val);
              ResolveParamsFromDictionary(Dic, Tmp, Quote, ResolveExternal);
              String.Insert(Tmp, start);
              i = start + Tmp.Length() - 1;
            }
          }
          else if( Val.CharAt(0) == '_' )  {
            IStringCifEntry* Params = FindParam<IStringCifEntry>(Val);
            olxstr Tmp = 'N';
            if( Params != NULL && Params->Count() != 0 )  
              Tmp = (*Params)[0];
            String.Delete(start, end-start+1);
            String.Insert(Tmp, start);
            i = start + Tmp.Length() - 1;
          }
          else
            TBasicApp::GetLog() << olxstr("A number or function starting from '$' or '_' is expected");
          continue;
        }
        size_t index = Val.ToSizeT();
        // Not much use if not for personal use :D
        /*
        if( index >= 73 )  {  //direct insert
          // 73 - crystals handling
          // 74 - other programs
          // 75 - solved and refined by...
          // 76 - collected for
          // 77 - anisotropic atoms
          // ....
          if( (index > Dic.Count()) || (index <= 0) )
            TBasicApp::GetLog()->Info( olxstr("Wrong parameter index ") << index);
          else  {
            String.Delete(start, end-start+1);
            String.Insert(Dic.String(index-1), start);
            i = start + Dic.String(index-1).Length() - 1;
          }
          continue;
        }
        */
        if( (index > Dic.Count()) || (index <= 0) )
          TBasicApp::GetLog().Error(olxstr("Wrong parameter index ") << index);
        else  {  // resolve indexes
          String.Delete(start, end-start+1);
          olxstr SVal = Dic[index-1];
          olxstr value;
          if( !SVal.IsEmpty() )  {
            if( SVal.Equalsi("date") )
              value = TETime::FormatDateTime(TETime::Now());
            else if( SVal.Equalsi("sg_number") )  {
              TSpaceGroup* sg = TSymmLib::GetInstance().FindSG(GetAsymmUnit());
              if( sg != NULL )
                value = sg->GetNumber();
              else
                value = "unknown";
            }
            else if( SVal.Equalsi("data_name") )
              value = GetDataName();
            else if( SVal.Equalsi("weighta") )
              value = WeightA;
            else if( SVal.Equalsi("weightb") )
              value = WeightB;
            else {
              IStringCifEntry* Params = FindParam<IStringCifEntry>(SVal);
              if( Params == NULL )  {
                TBasicApp::GetLog().Info(olxstr("The parameter \'") << SVal << "' is not found");
                value = 'N';
              }
              else if( Params->Count() == 0 )  {
                TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not found");
                  value = "none";
              }
              else if( Params->Count() == 1 )  {
                if( (*Params)[0].IsEmpty() )  {
                  TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not found");
                  value = "none";
                }
                else if( (*Params)[0].CharAt(0) == '?' )  {
                  TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not defined");
                  value = '?';
                }
                else  {
                  if( (index == 13 || index == 14 || index == 30) && DoubleTheta )
                    value = (*Params)[0].ToDouble()*2;
                  else
                    value = (*Params)[0];
                }
              }
              else  {
                value = (*Params)[0];
                for( size_t sti=1; sti < Params->Count(); sti++ )  {
                  value << ' ' << (*Params)[sti];
                }
              }
            }
            String.Insert(value, start);
            i = start + value.Length() - 1;
          }
        }
      }
    }
  }
  return true;
}
//..............................................................................
void TCif::MultValue(olxstr &Val, const olxstr &N)  {
  Val = (TEValue<double>(Val) *= N.ToDouble()).ToString();
}
//..............................................................................
bool TCif::CreateTable(TDataItem *TD, TTTable<TStrList> &Table, smatd_list& SymmList) const {
  int RowDeleted=0, ColDeleted=0;
  SymmList.Clear();
  const CifTable* LT = NULL;
  for( size_t i=0; i < LoopCount(); i++ )  {
    LT = &GetLoop(i).GetData();
    if( LT->ColCount() < TD->ItemCount() )  continue;
    size_t defcnt = 0;
    for( size_t j=0; j < LT->ColCount(); j++ )  {
      if( TD->FindItemi(LT->ColName(j)) != NULL )
        defcnt ++;
    }
    if( defcnt == TD->ItemCount() )  break;
    else  LT = NULL;
  }
  if( LT == NULL )  {
    TBasicApp::GetLog().Info(olxstr("Could not find loop for table definition: ") << TD->GetName());
    return false;
  }
  Table.Resize(LT->RowCount(), LT->ColCount());
  for( size_t i =0; i < Table.ColCount(); i++ )  {
    Table.ColName(i) = LT->ColName(i);
    for( size_t j=0; j < Table.RowCount(); j++ )
      Table[j][i] = (*LT)[j][i]->GetStringValue();
  }
  // process rows
  for( size_t i=0; i < LT->RowCount(); i++ )  {
    bool AddRow = true;
    for( size_t j=0; j < LT->ColCount(); j++ )  {
      TDataItem *DI = TD->FindItemi(LT->ColName(j));
      if( LT->ColName(j).StartsFrom("_geom_") && 
        LT->ColName(j).IndexOf("site_symmetry") != InvalidIndex)
      {
        if( (*LT)[i][j]->GetStringValue() != '.' )  {  // 1_555
          olxstr tmp = LT->ColName(j).SubStringFrom(LT->ColName(j).LastIndexOf('_')+1);
          //if( !tmp.IsNumber() ) continue;
          olxstr Tmp = "label_";
          Tmp << tmp;
          smatd SymmMatr = SymmCodeToMatrix((*LT)[i][j]->GetStringValue());
          size_t matIndex = SymmList.IndexOf(SymmMatr);
          if( matIndex == InvalidIndex )  {
            SymmList.AddCCopy(SymmMatr);
            matIndex = SymmList.Count()-1;
          }
          for( size_t k=0; k < Table.ColCount(); k++ )  {
            if( Table.ColName(k).EndsWith(Tmp) )  {
              Table[i-RowDeleted][k] << "<sup>" << (matIndex+1) << "</sup>";
              break;
            }
          }
        }
      }
      if( DI == NULL )  continue;
      olxstr Val = (*LT)[i][j]->GetStringValue();
      olxstr Tmp = DI->GetFieldValue("mustequal", EmptyString);
      TStrList Toks(Tmp, ';');
      if( !Tmp.IsEmpty() && (Toks.IndexOfi(Val) == InvalidIndex) ) // equal to
      {  AddRow = false;  break;  }

      Tmp = DI->GetFieldValue("atypeequal", EmptyString);
      if( !Tmp.IsEmpty() )  {  // check for atom type equals to
        ICifEntry* CD = (*LT)[i][j];
        if( CD != NULL && EsdlInstanceOf(*CD, AtomCifEntry) )
          if( !((AtomCifEntry*)CD)->data.GetType().symbol.Equalsi(Tmp) )  {
            AddRow = false;
            break;
          }
      }
      Tmp = DI->GetFieldValue("atypenotequal", EmptyString);
      if( !Tmp.IsEmpty() )  {  // check for atom type equals to
        ICifEntry* CD = (*LT)[i][j];
        if( CD != NULL && EsdlInstanceOf(*CD, AtomCifEntry) )
          if( ((AtomCifEntry*)CD)->data.GetType().symbol.Equalsi(Tmp) )  {
            AddRow = false;
            break;
          }
      }
      Tmp = DI->GetFieldValue("mustnotequal", EmptyString);
      Toks.Clear();
      Toks.Strtok(Tmp, ';');
      if( !Tmp.IsEmpty() && (Toks.IndexOfi(Val) != InvalidIndex) ) // not equal to
      {  AddRow = false;  break;  }

      Tmp = DI->GetFieldValue("multiplier", EmptyString);
      if( !Tmp.IsEmpty() )  {  // Multiply
        Val = Table[i-RowDeleted][j];
        MultValue(Val, Tmp);
        Table[i-RowDeleted][j] = Val;
      }
    }
    if( !AddRow )  {
      Table.DelRow(i-RowDeleted);
      RowDeleted++;
    }
  }
  // process columns
  for( size_t i=0; i < LT->ColCount(); i++ )  {
    TDataItem *DI = TD->FindItemi(LT->ColName(i));
    if( DI != NULL )  {
      Table.ColName(i-ColDeleted) = DI->GetFieldValueCI("caption");
      if( !DI->GetFieldValueCI("visible", FalseString).ToBool() )  {
        Table.DelCol(i-ColDeleted);
        ColDeleted++;
      }
    }
    else  {
      Table.DelCol(i-ColDeleted);
      ColDeleted++;
    }
  }
  return true;
}
//..............................................................................


