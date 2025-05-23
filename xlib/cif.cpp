/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "cif.h"
#include "datafile.h"
#include "catom.h"
#include "satom.h"
#include "symmparser.h"
#include "unitcell.h"
#include "ellipsoid.h"
#include "xapp.h"
#include "log.h"
#include "symmlib.h"
#include "etime.h"
#include "integration.h"
#include "label_corrector.h"

using namespace exparse::parser_util;
using namespace cif_dp;

TCif::TCif()
  : block_index(InvalidIndex), has_duplicate_labels(false)
{}
//..............................................................................
TCif::~TCif()
{}
//..............................................................................
void TCif::Clear() {
  WeightA.SetLength(0);
  WeightB.SetLength(0);
  GetRM().Clear(rm_clear_ALL);
  GetAsymmUnit().Clear();
  DataManager.Clear();
  Matrices.Clear();
  MatrixMap.Clear();
}
//..............................................................................
void TCif::LoadFromStrings(const TStrList& Strings) {
  block_index = InvalidIndex;
  data_provider.LoadFromStrings(Strings);
  for (size_t i=0; i < data_provider.Count(); i++) {
    CifBlock& cb = data_provider[i];
    if (!cb.param_map.HasKey("_cell_length_a")) {
      continue;
    }
    bool valid = false;
    for (size_t j = 0; j < cb.table_map.Count(); j++) {
      if (cb.table_map.GetKey(j).StartsFrom("_atom_site")) {
        valid = true;
        break;
      }
    }
    if (valid) {
      block_index = i;
      break;
    }
  }
  has_duplicate_labels = false;
  //_LoadCurrent();
}
//..............................................................................
void TCif::_LoadCurrent() {
  if (data_provider.Count() == 0) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "Empty/Invalid CIF");
  }
  if (block_index == InvalidIndex) {
    if (data_provider.Count() == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "no data available");
    }
    for (size_t i = 0; i < data_provider.Count(); i++) {
      CifBlock& cb = data_provider[i];
      if (cb.param_map.IndexOf("_cell_length_a") != InvalidIndex) {
        block_index = i;
        break;
      }
    }
    if (block_index == InvalidIndex) {
      return;  // nothing to initialise anyway... must be a dummy CIF
    }
  }
  // undo the changes
  for (size_t i = 0; i < data_provider.Count(); i++) {
    CifBlock& d = data_provider[i];
    for (size_t j = 0; j < d.table_map.Count(); j++) {
      cetTable& tab = *d.table_map.GetValue(j);
      for (size_t k = 0; k < tab.ColCount(); k++) {
        if (tab.ColName(k).IndexOf("atom_site") != InvalidIndex &&
          tab.ColName(k).IndexOf("label") != InvalidIndex)
        {
          for (size_t l = 0; l < tab.RowCount(); l++) {
            if (tab.Get(l, k).Is<AtomCifEntry>() ||
              tab.Get(l, k).Is<AtomPartCifEntry>())
            {
              tab.Set(l, k, new cetString(tab.Get(l, k).GetStringValue()));
            }
          }
        }
      }
    }
  }
  CifBlock& cif_data = data_provider[block_index];
  Clear();
  /*search for the weigting scheme*************************************/
  const size_t ws_i = cif_data.param_map.IndexOf("_refine_ls_weighting_details");
  if (ws_i != InvalidIndex) {
    IStringCifEntry* ci = dynamic_cast<IStringCifEntry*>(cif_data.param_map.GetValue(ws_i));
    if (ci != NULL && ci->Count() == 1) {
      const olxstr& tmp = (*ci)[0];
      for (size_t k = 0; k < tmp.Length(); k++) {
        if (tmp[k] == '+') {
          if (WeightA.IsEmpty()) {
            const size_t st = k + 2;
            while (tmp[k] != ')' && ++k < tmp.Length())
              ;
            WeightA = tmp.SubString(st, --k - st);
          }
          else if (WeightB.IsEmpty()) {
            const size_t st = k;
            while (tmp[k] != ']' && ++k < tmp.Length());
            WeightB = tmp.SubString(st, k - st - 1);
          }
          else {
            break;
          }
        }
      }
    }
  }
  Initialize();
}
//..............................................................................
void TCif::SaveToStrings(TStrList& Strings)  {
  TStopWatch sw(__FUNC__);
  static olxstr def_pivots(
    "_audit_creation,_publ,_chemical_name,_chemical_formula,_chemical,_atom_type,"
    "_space_group,_space_group_symop,_symmetry,"
    "_cell_length,_cell_angle,_cell_volume,_cell_formula,_cell,"
    "_exptl_,"
    "_diffrn_reflns,_diffrn,"
    "_reflns,"
    "_computing,"
    "_refine,"
    "_atom_sites,_atom_site,_atom_site_aniso,"
    "_geom_special,_geom_bond,_geom_angle,_geom,"
    "_smtbx"
    );
  static olxstr def_endings(
    "_h_min,_h_max,_k_min,k_max,_l_min,_l_max,_min,_max"
    );
  if (block_index == InvalidIndex) {
    return;
  }
  TStrList pivots, endings;
  TXApp& xapp = TXApp::GetInstance();
  olxstr CifCustomisationFN(xapp.GetCifTemplatesDir() + "customisation.xlt");
  if (TEFile::Exists(CifCustomisationFN)) {
    try {
      TDataFile df;
      if (!df.LoadFromXLFile(CifCustomisationFN)) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "failed to load CIF customisation file");
      }
      df.Include(NULL);
      const TDataItem& ist = df.Root().GetItemByName("cif_customisation")
        .GetItemByName("sorting");
      const TDataItem& ipv = ist.GetItemByName("pivots");
      for (size_t i=0; i < ipv.ItemCount(); i++)
        pivots.Add(ipv.GetItemByIndex(i).GetValue());
      const TDataItem& ied = ist.GetItemByName("endings");
      for (size_t i=0; i < ied.ItemCount(); i++)
        pivots.Add(ied.GetItemByIndex(i).GetValue());
    }
    catch(const TExceptionBase& e) {
      throw TFunctionFailedException(__OlxSourceInfo, e);
    }
  }
  else {
    pivots.Strtok(def_pivots, ',');
    endings.Strtok(def_endings, ',');
  }
  sw.start("Fixing labels");
  for (size_t i=0; i < GetAsymmUnit().AtomCount(); i++) {
    TCAtom &ca = GetAsymmUnit().GetAtom(i);
    olxstr lb = ca.GetLabel();
    lb.Replace('\t' ,' ').Replace('#', ' ');
    if (lb.Contains(' ')) {
      TBasicApp::NewLogEntry(logError) << "Changing invalid atom labels for" <<
        (olxstr(' ').quote() << ca.GetLabel());
      ca.SetLabel(lb.DeleteChars(' '), false);
    }
  }
  // fix parts if needed
  if (!TXApp::DoRenameParts()) {
    olxset<TCAtom *, TPointerComparator> as = GetAsymmUnit().GetAtomsNeedingPartInLabel();
    if (!as.IsEmpty()) {
      for (size_t i = 0; i < data_provider[block_index].table_map.Count(); i++) {
        cetTable &t = *data_provider[block_index].table_map.GetValue(i);
        for (size_t j = 0; j < t.RowCount(); j++) {
          for (size_t k = 0; k < t.ColCount(); k++) {
            AtomCifEntry *i = dynamic_cast<AtomCifEntry *>(t[j][k]);
            if (i != 0) {
              i->save_part = as.Contains(&i->data);
            }
          }
        }
      }
    }
  }
  GetAsymmUnit().ComplyToResidues();
  sw.start("Sorting");
  for (size_t i = 0; i < data_provider[block_index].table_map.Count(); i++) {
    data_provider[block_index].table_map.GetValue(i)->Sort();
  }
  data_provider[block_index].Sort(pivots, endings);
  sw.start("Saving");
  data_provider.SaveToStrings(Strings);
}
//..............................................................................
olxstr TCif::GetParamAsString(const olxstr &Param, size_t block_idx) const {
  size_t bix = block_idx == InvalidIndex ? block_index : block_idx;
  if (bix == InvalidIndex || bix >= data_provider.Count()) {
    return EmptyString();
  }
  ICifEntry* ce = data_provider[bix].param_map.Find(Param, 0);
  if (ce == 0) {
    return EmptyString();
  }
  IStringCifEntry* ci = dynamic_cast<IStringCifEntry*>(ce);
  if (ci == 0) {
    TStrList rv;
    ce->ToStrings(rv);
    return rv.Text('\n');
  }
  if (ci->Count() == 0) {
    return EmptyString();
  }
  olxstr rv = (*ci)[0];
  for (size_t i = 1; i < ci->Count(); i++) {
    rv << '\n' << (*ci)[i];
  }
  return rv;
}
//..............................................................................
void TCif::SetParam(const ICifEntry& value)  {
  if (block_index == InvalidIndex) {
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
  }
  data_provider[block_index].Add(value.Replicate());
}
//..............................................................................
void TCif::ReplaceParam(const olxstr& name, const ICifEntry& value) {
  if (block_index == InvalidIndex) {
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
  }
  data_provider[block_index].Remove(name);
  data_provider[block_index].Add(value.Replicate());
}
//..............................................................................
void TCif::Rename(const olxstr& old_name, const olxstr& new_name)  {
  data_provider[block_index].Rename(old_name, new_name);
}
//..............................................................................
ConstPtrList<TCAtom> TCif::FindAtoms(const TStrList &names) {
  TCAtomPList atoms;
  for (size_t i=0; i < names.Count(); i++) {
    if (atoms.Add(GetAsymmUnit().FindCAtom(names[i])) == 0) {
      TBasicApp::NewLogEntry(logError) <<
        (olxstr("Undefined atom: ").quote() << names[i]);
      atoms.Clear();
      break;
    }
  }
  return atoms;
}
//..............................................................................
void TCif::Initialize() {
  has_duplicate_labels = false;
  olx_array_ptr<double> Q(6), E(6); // quadratic form of ellipsoid
  TEValueD EValue;
  try {
    try {
      const olxstr mx = GetParamAsString("_exptl_crystal_size_max");
      if (mx.IsNumber()) {
        const olxstr md = GetParamAsString("_exptl_crystal_size_mid"),
          mn = GetParamAsString("_exptl_crystal_size_min");
        if (md.IsNumber() && mn.IsNumber())
          GetRM().expl.SetCrystalSize(mx.ToDouble(), md.ToDouble(), mn.ToDouble());
      }
      const olxstr temp = GetParamAsString("_diffrn_ambient_temperature");
      if (!temp.IsEmpty() && temp != '?') {
        TEValueD t_v(temp);
        t_v.V() -= 273.15;
        GetRM().expl.SetTempValue(t_v);
      }
      const olxstr radiation = GetParamAsString("_diffrn_radiation_wavelength");
      if (!radiation.IsEmpty() && radiation != '?')
        GetRM().expl.SetRadiation(radiation.ToDouble());
    }
    catch (...) {}
    EValue = GetParamAsString("_cell_length_a");
    GetAsymmUnit().GetAxes()[0] = EValue.GetV();
    GetAsymmUnit().GetAxisEsds()[0] = EValue.GetE();
    EValue = GetParamAsString("_cell_length_b");
    GetAsymmUnit().GetAxes()[1] = EValue.GetV();
    GetAsymmUnit().GetAxisEsds()[1] = EValue.GetE();
    EValue = GetParamAsString("_cell_length_c");
    GetAsymmUnit().GetAxes()[2] = EValue.GetV();
    GetAsymmUnit().GetAxisEsds()[2] = EValue.GetE();

    EValue = GetParamAsString("_cell_angle_alpha");
    GetAsymmUnit().GetAngles()[0] = EValue.GetV();
    GetAsymmUnit().GetAngleEsds()[0] = EValue.GetE();
    EValue = GetParamAsString("_cell_angle_beta");
    GetAsymmUnit().GetAngles()[1] = EValue.GetV();
    GetAsymmUnit().GetAngleEsds()[1] = EValue.GetE();
    EValue = GetParamAsString("_cell_angle_gamma");
    GetAsymmUnit().GetAngles()[2] = EValue.GetV();
    GetAsymmUnit().GetAngleEsds()[2] = EValue.GetE();
    if (ParamExists("_cell_formula_units_Z"))
      GetAsymmUnit().SetZ(
      (short)olx_round(GetParamAsString("_cell_formula_units_Z").ToDouble()));
  }
  catch (...) {
    GetAsymmUnit().GetAxes() = vec3d(0);
    GetAsymmUnit().GetAngles() = vec3d(0);
    TBasicApp::NewLogEntry(logInfo) <<
      "CIF initialising failed: unknown cell parameters";
    return;
  }
  // check if the cif file contains valid parameters
  if (GetAsymmUnit().CalcCellVolume() == 0) {
    TBasicApp::NewLogEntry(logInfo) << "CIF initialising failed: zero cell volume";
    return;
  }
  {
    olxstr qp = GetParamAsString("_refine_diff_density_max");
    if (qp.IsNumber()) {
      GetAsymmUnit().SetMaxQPeak(qp.ToDouble());
    }
  }

  GetAsymmUnit().InitMatrices();
  bool sg_initialised = false;
  cetTable *Loop = FindLoop("_space_group_symop");
  if (Loop == 0) {
    Loop = FindLoop("_space_group_symop_operation_xyz");
  }
  if (Loop != 0) {
    size_t sindex = Loop->ColIndex("_space_group_symop_operation_xyz");
    size_t iindex = Loop->ColIndex("_space_group_symop_id");
    if (sindex != InvalidIndex) {
      for (size_t i = 0; i < Loop->RowCount(); i++) {
        try {
          Matrices.AddCopy(
            TSymmParser::SymmToMatrix(Loop->Get(i, sindex).GetStringValue()));
        }
        catch (const TExceptionBase &e) {
          throw TFunctionFailedException(__OlxSourceInfo, e,
            "could not process symmetry matrix");
        }
        if (iindex == InvalidIndex) {
          MatrixMap.Add(i + 1, i);
        }
        else {
          MatrixMap.Add(Loop->Get(i, iindex).GetStringValue(), i);
        }
      }
    }
  }
  else {
    Loop = FindLoop("_symmetry_equiv_pos");
    if (Loop == 0) {
      Loop = FindLoop("_symmetry_equiv_pos_as_xyz");
    }
    if (Loop != 0) {
      cetTable& symop_tab = AddLoopDef(
        "_space_group_symop_id,_space_group_symop_operation_xyz");
      size_t sindex = Loop->ColIndex("_symmetry_equiv_pos_as_xyz");
      size_t iindex = Loop->ColIndex("_symmetry_equiv_pos_site_id");
      if (sindex != InvalidIndex) {
        for (size_t i = 0; i < Loop->RowCount(); i++) {
          try {
            Matrices.AddCopy(
              TSymmParser::SymmToMatrix(Loop->Get(i, sindex).GetStringValue()));
          }
          catch (const TExceptionBase &e) {
            TStrList l;
            l << "to process symmetry element:";
            Loop->Get(i, sindex).ToStrings(l);
            throw TFunctionFailedException(__OlxSourceInfo, e,
              l.Text(NewLineSequence()) << NewLineSequence());
          }
          CifRow& row = symop_tab.AddRow();
          if (iindex == InvalidIndex) {
            MatrixMap.Add(i + 1, i);
            row[0] = new cetString(i + 1);
          }
          else {
            MatrixMap.Add(Loop->Get(i, iindex).GetStringValue(), i);
            row[0] = Loop->Get(i, iindex).Replicate();
          }
          row[1] = Loop->Get(i, sindex).Replicate();
        }
      }
      if (!Matrices.IsEmpty()) {
        if (!Matrices[0].IsI()) {
          TBasicApp::NewLogEntry(logError) << "The CIF symmetry first matrix "
            "element is not identity! This may cause CIF processing errors - "
            "please fix.";
          size_t idx = InvalidIndex;
          for (size_t i = 1; i < Matrices.Count(); i++) {
            if (Matrices[i].IsI()) {
              idx = i;
              break;
            }
          }
          if (idx != InvalidIndex) {
            Matrices.Swap(0, idx);
            size_t i0 = InvalidIndex, i1 = InvalidIndex;
            for (size_t i = 0; i < MatrixMap.Count(); i++) {
              if (MatrixMap.GetValue(i) == 0) {
                i0 = i;
              }
              else if (MatrixMap.GetValue(i) == idx) {
                i1 = i;
              }
              if (i0 != InvalidIndex && i1 != InvalidIndex) {
                MatrixMap.GetValue(i0) = idx;
                MatrixMap.GetValue(i1) = 0;
                break;
              }
            }
          }
        }
      }
      // remove obsolete loop
      data_provider[block_index].Remove(*(ICifEntry*)Loop);
    }
    // no sym ops, check hall symbol
    else {
      olxstr hs = GetParamAsString("_space_group_name_Hall");
      if (hs.IsEmpty()) {
        hs = GetParamAsString("_symmetry_space_group_name_Hall");
      }
      if (!hs.IsEmpty()) {
        TSpaceGroup *sg = TSymmLib::GetInstance().FindGroupByHallSymbol(hs);
        if (sg != 0) {
          GetAsymmUnit().ChangeSpaceGroup(*sg);
          sg_initialised = true;
          TBasicApp::NewLogEntry(logWarning) <<
            "Note the symmetry operators were deduced from the Hall symbol and"
            " may differ from the intendent ones.";
        }
        else {
          try {
            GetAsymmUnit().ChangeSpaceGroup(
              TSymmLib::GetInstance().CreateNew(hs));
            sg_initialised = true;
          }
          catch (...) {
            TBasicApp::NewLogEntry() << "Failed to expand Hall symbol";
          }
        }
      }
    }
  }
  if (!sg_initialised) {
    try {
      if (Matrices.IsEmpty()) {
        GetAsymmUnit().ChangeSpaceGroup(
          *TSymmLib::GetInstance().FindGroupByName("P1"));
      }
      else {
        GetAsymmUnit().ChangeSpaceGroup(
          TSymmLib::GetInstance().FindSymSpace(Matrices));
      }
    }
    catch (const TExceptionBase &e) {
      TStrList out;
      e.GetException()->GetStackTrace(out);
      TBasicApp::NewLogEntry(logInfo) << out;
      GetAsymmUnit().ChangeSpaceGroup(
        *TSymmLib::GetInstance().FindGroupByName("P1"));
    }
  }
  try {
    TStrList frm(GetParamAsString("_chemical_formula_sum"), ' ');
    for (size_t i = 0; i < frm.Count(); i++) {
      if ((frm[i].Length() == 1 && olxstr::o_isalpha(frm[i].CharAt(0))) ||
        (frm[i].Length() == 2 && olxstr::o_isalpha(frm[i].CharAt(0)) &&
          olxstr::o_isalpha(frm[i].CharAt(1))))
      {
        frm[i] << '1';
      }
    }
    GetRM().SetUserFormula(frm.Text(EmptyString()));
  }
  catch (...) {}

  this->Title = GetDataName().ToUpperCase();
  this->Title << " OLEX2: imported from CIF";

  cetTable *ALoop = FindLoop("_atom_site");
  if (ALoop == 0) {
    return;
  }

  size_t ALabel = ALoop->ColIndex("_atom_site_label");
  const size_t ACi[] = {
    ALoop->ColIndex("_atom_site_fract_x"),
    ALoop->ColIndex("_atom_site_fract_y"),
    ALoop->ColIndex("_atom_site_fract_z")
  };
  const size_t ACUiso = ALoop->ColIndex("_atom_site_U_iso_or_equiv");
  const size_t ASymbol = ALoop->ColIndex("_atom_site_type_symbol");
  const size_t APart = ALoop->ColIndex("_atom_site_disorder_group");
  const size_t SiteOccu = ALoop->ColIndex("_atom_site_occupancy");
  size_t Degen = ALoop->ColIndex("_atom_site_symmetry_multiplicity");
  /* 0 - unknown, 1 - position degenaracy, 2 - position multiplicity
  */
  int DegenFunction = 0;
  if (Degen == InvalidIndex) {
    Degen = ALoop->ColIndex("_atom_site_site_symmetry_multiplicity");
    if (Degen != InvalidIndex) {
      DegenFunction = 2;
    }
  }
  const size_t Part = ALoop->ColIndex("_atom_site_disorder_group");
  if ((ALabel | ACi[0] | ACi[1] | ACi[2]) == InvalidIndex) {
    TBasicApp::NewLogEntry(logError) <<
      "Failed to locate required fields in atoms loop";
    return;
  }
  // need to put Identity first as it might not be!
  smatd_list MatrixList = this->Matrices;
  for (size_t i = 0; i < MatrixList.Count(); i++) {
    if (MatrixList[i].IsI()) {
      MatrixList.Delete(i);
      break;
    }
  }
  MatrixList.Insert(0, new smatd()).I();
  //
  olxstr_dict<size_t> label_occurrence;
  for (size_t i = 0; i < ALoop->RowCount(); i++) {
    olxstr label = ALoop->Get(i, ALabel).GetStringValue();
    label_occurrence.Add(label, 0)++;
    TResidue *resi = 0;
    int part = 0;
    if (Part != InvalidIndex) {
      olxstr p_str = ALoop->Get(i, Part).GetStringValue();
      if (p_str.IsInt()) {
        part = p_str.ToInt();
        olxstr sfx = (olxch)('a' + olx_abs(part) - 1);
        if (label.EndsWith(olxstr("_") << sfx) || label.EndsWith(olxstr("^") << sfx)) {
          label.SetLength(label.Length() - 2);
        }
      }
    }
    {
      size_t ui = label.IndexOf('_');
      if (ui != InvalidIndex) {
        size_t p_idx = label.FirstIndexOf('^', ui);
        if (p_idx != InvalidIndex && p_idx + 1 < label.Length()) {
          part = label.CharAt(p_idx + 1) - 'a' + 1;
          label.SetLength(p_idx);
        }
        olxstr rn = label.SubStringFrom(ui + 1);
        if (TResidue::IsValidNumber(rn)) {
          olxch chain_id = TResidue::NoChainId();
          int r_num = TResidue::NoResidue;
          size_t cidx = rn.IndexOf(':');
          if (cidx == 1) {
            chain_id = rn.CharAt(0);
            r_num = rn.SubStringFrom(cidx + 1).ToInt();
          }
          else if (rn.IsInt()) {
            r_num = rn.ToInt();
          }
          if (r_num != TResidue::NoResidue) {
            resi = GetAsymmUnit().FindResidue(chain_id, r_num);
            if (resi == 0) {
              resi = &GetAsymmUnit().NewResidue(
                olxstr("unk") << GetAsymmUnit().ResidueCount(), r_num, r_num, chain_id);
            }
            label.SetLength(ui);
          }
        }
      }
    }
    TCAtom& A = GetAsymmUnit().NewAtom(resi);
    A.SetLabel(label, false);
    A.SetPart(part);
    const cm_Element* type = 0;
    if (ASymbol != InvalidIndex) {
      type = XElementLib::FindBySymbolEx(ALoop->Get(i, ASymbol).GetStringValue());
    }
    else {
      type = XElementLib::FindBySymbolEx(ALoop->Get(i, ALabel).GetStringValue());
    }
    if (type == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("Undefined element: ").quote() <<
        ALoop->Get(i, ASymbol != InvalidIndex ? ASymbol : ALabel).GetStringValue());
    }
    A.SetType(*type);
    for (int j = 0; j < 3; j++) {
      EValue = ALoop->Get(i, ACi[j]).GetStringValue();
      A.ccrd()[j] = EValue.GetV();  A.ccrdEsd()[j] = EValue.GetE();
      if (EValue.GetE() == 0) {
        GetRM().Vars.FixParam(A, catom_var_name_X + j);
      }
    }
    if (ACUiso != InvalidIndex) {
      EValue = ALoop->Get(i, ACUiso).GetStringValue();
      A.SetUisoEsd(EValue.GetE());
      A.SetUiso(EValue.GetV());
      if (EValue.GetE() == 0) {
        GetRM().Vars.FixParam(A, catom_var_name_Uiso);
      }
    }
    if (APart != InvalidIndex &&
      ALoop->Get(i, APart).GetStringValue().IsNumber())
    {
      A.SetPart(ALoop->Get(i, APart).GetStringValue().ToInt());
    }
    if (SiteOccu != InvalidIndex) {
      EValue = ALoop->Get(i, SiteOccu).GetStringValue();
      A.SetOccu(EValue.GetV());
      A.SetOccuEsd(EValue.GetE());
      if (EValue.GetE() == 0) {
        GetRM().Vars.FixParam(A, catom_var_name_Sof);
      }
    }
    size_t site_mult = InvalidIndex;
    // try to guess what it is
    if (Degen != InvalidIndex && DegenFunction == 0) {
      double a = ALoop->Get(i, Degen).GetStringValue().ToDouble();
      site_mult = TUnitCell::GetPositionMultiplicity(MatrixList, A.ccrd());
      double b = static_cast<double>(site_mult);
      if (olx_abs(a - b) < 1e-3) {
        DegenFunction = 1;
        // check if both DegenFunction is the same for 1 and 2!
        if ((olx_abs((double)MatrixCount() / b - a) < 1e-3)) {
          DegenFunction = 0;
        }
      }
      else if (olx_abs((double)MatrixCount() / b - a) < 1e-3) {
        DegenFunction = 2;
      }
      else if (olx_abs(b - 1. / a) < 1e-3) {
        DegenFunction = 3;
      }
    }
    if (DegenFunction == 0) {
      double degen = static_cast<double>(site_mult == InvalidIndex ?
        TUnitCell::GetPositionMultiplicity(MatrixList, A.ccrd()) : site_mult);
      if (degen != 1) {
        A.SetOccu(A.GetOccu() / degen);
        A.SetOccuEsd(A.GetOccuEsd() / degen);
      }
    }
    else {
      double degen = ALoop->Get(i, Degen).GetStringValue().ToDouble();
      if (DegenFunction == 2) {
        degen = (double)MatrixCount() / degen;
      }
      else if (DegenFunction == 3) {
        degen = 1. / degen;
      }
      if (degen != 1) {
        A.SetOccu(A.GetOccu() / degen);
        A.SetOccuEsd(A.GetOccuEsd() / degen);
      }
    }
    ALoop->Set(i, ALabel, new AtomCifEntry(A));
    if (Part != InvalidIndex) {
      ALoop->Set(i, Part, new AtomPartCifEntry(A));
    }
  }
  // warn if any duplicate labels
  {
    olxstr_buf duplicates;
    for (size_t i = 0; i < label_occurrence.Count(); i++) {
      if (label_occurrence.GetValue(i) > 1) {
        duplicates << ' ' << label_occurrence.GetKey(i);
      }
    }
    if (!duplicates.IsEmpty()) {
      TBasicApp::NewLogEntry(logWarning) << "WARNING: CIF contains duplicate atom labels. "
        "Do not use Olex2 to process it.";
      has_duplicate_labels = true;
      TBasicApp::NewLogEntry(logWarning) << duplicates;
    }
  }
  for (size_t i = 0; i < LoopCount(); i++) {
    if (&GetLoop(i) == ALoop) {
      continue;
    }
    cetTable& tab = GetLoop(i);
    for (size_t j = 0; j < tab.ColCount(); j++) {
      if (tab.ColName(j).Contains("atom_site") &&
        tab.ColName(j).Contains("label"))
      {
        for (size_t k = 0; k < tab.RowCount(); k++) {
          TCAtom* ca = GetAsymmUnit().FindCAtom(
            tab[k][j]->GetStringValue());
          if (ca != 0) {
            tab.Set(k, j, new AtomCifEntry(*ca));
          }
        }
      }
    }
  }
  if ((ALoop = FindLoop("_atom_site_aniso")) != 0) {
    ALabel = ALoop->ColIndex("_atom_site_aniso_label");
    double scale;
    TSizeList Ui;
    Ui.SetCapacity(6);
    size_t x = ALoop->ColIndex("_atom_site_aniso_U_11");
    if (x == InvalidIndex) {
      x = ALoop->ColIndex("_atom_site_aniso_B_11");
      if (x != InvalidIndex) {
        scale = 1. / (8 * M_PI*M_PI);
        Ui << ALoop->ColIndex("_atom_site_aniso_B_11")
          << ALoop->ColIndex("_atom_site_aniso_B_22")
          << ALoop->ColIndex("_atom_site_aniso_B_33")
          << ALoop->ColIndex("_atom_site_aniso_B_23")
          << ALoop->ColIndex("_atom_site_aniso_B_13")
          << ALoop->ColIndex("_atom_site_aniso_B_12");
      }
    }
    else {
      scale = 1;
      Ui << ALoop->ColIndex("_atom_site_aniso_U_11")
        << ALoop->ColIndex("_atom_site_aniso_U_22")
        << ALoop->ColIndex("_atom_site_aniso_U_33")
        << ALoop->ColIndex("_atom_site_aniso_U_23")
        << ALoop->ColIndex("_atom_site_aniso_U_13")
        << ALoop->ColIndex("_atom_site_aniso_U_12");
    }

    if (ALabel != InvalidIndex && Ui.Count() == 6 &&
      (Ui[0] | Ui[1] | Ui[2] | Ui[3] | Ui[4] | Ui[5]) != InvalidIndex)
    {
      for (size_t i = 0; i < ALoop->RowCount(); i++) {
        AtomCifEntry * ci = dynamic_cast<AtomCifEntry *>((*ALoop)[i][ALabel]);
        if (ci == 0) {
          TBasicApp::NewLogEntry(logError) <<
            (olxstr("Wrong atom in the aniso loop ").quote() <<
              ALoop->Get(i, ALabel).GetStringValue() << " removing");
          ALoop->DelRow(i--);
          continue;
        }
        for (int j = 0; j < 6; j++) {
          EValue = ALoop->Get(i, Ui[j]).GetStringValue();
          Q[j] = EValue.GetV()*scale;
          E[j] = EValue.GetE()*scale;
          if (EValue.GetE() == 0) {
            GetRM().Vars.FixParam(ci->data, catom_var_name_U11 + j);
          }
        }
        GetAsymmUnit().UcifToUcart(Q);
        ci->data.AssignEllp(&GetAsymmUnit().NewEllp().Initialise(Q, E));
      }
    }
  }
  // geometric parameters
  if ((ALoop = FindLoop("_geom_bond")) != 0) {
    try {
      const size_t ALabel = ALoop->ColIndex("_geom_bond_atom_site_label_1");
      const size_t ALabel1 = ALoop->ColIndex("_geom_bond_atom_site_label_2");
      const size_t BD = ALoop->ColIndex("_geom_bond_distance");
      const size_t SymmA = ALoop->ColIndex("_geom_bond_site_symmetry_2");
      if ((ALabel | ALabel1 | BD | SymmA) != InvalidIndex) {
        TEValueD ev;
        for (size_t i = 0; i < ALoop->RowCount(); i++) {
          const CifRow& Row = (*ALoop)[i];
          ev = Row[BD]->GetStringValue();
          AtomCifEntry * ci1 = dynamic_cast<AtomCifEntry *>(Row[ALabel]);
          AtomCifEntry * ci2 = dynamic_cast<AtomCifEntry *>(Row[ALabel1]);
          if (ci1 != 0 && ci2 != 0) {
            ACifValue* cv = 0;
            if (Row[SymmA]->GetStringValue() == '.') {
              cv = new CifBond(ci1->data, ci2->data, ev);
            }
            else {
              cv = new CifBond(ci1->data, ci2->data,
                SymmCodeToMatrix(Row[SymmA]->GetStringValue()), ev);
            }
            DataManager.AddValue(cv);
          }
        }
      }
    }
    catch (const TExceptionBase &e) {
      TBasicApp::NewLogEntry(logException) << "Failed to read the '" <<
        ALoop->GetName() << " loop: " << e.GetException()->GetFullMessage();
    }
  }
  if ((ALoop = FindLoop("_geom_hbond")) != 0) {
    try {
      const size_t ALabel = ALoop->ColIndex("_geom_hbond_atom_site_label_D");
      const size_t ALabel1 = ALoop->ColIndex("_geom_hbond_atom_site_label_A");
      const size_t ALabel2 = ALoop->ColIndex("_geom_hbond_atom_site_label_H");
      const size_t BD = ALoop->ColIndex("_geom_hbond_distance_DA");
      const size_t SymmA = ALoop->ColIndex("_geom_hbond_site_symmetry_A");
      if ((ALabel | ALabel1 | ALabel2 | BD | SymmA) != InvalidIndex) {
        TEValueD ev;
        for (size_t i = 0; i < ALoop->RowCount(); i++) {
          const CifRow& Row = (*ALoop)[i];
          ev = Row[BD]->GetStringValue();
          AtomCifEntry * ci1 = dynamic_cast<AtomCifEntry *>(Row[ALabel]);
          AtomCifEntry * ci2 = dynamic_cast<AtomCifEntry *>(Row[ALabel1]);
          AtomCifEntry * ha = dynamic_cast<AtomCifEntry *>(Row[ALabel2]);
          if (ci1 != 0 && ci2 != 0 && ha != 0) {
            ACifValue* cv;
            if (Row[SymmA]->GetStringValue() == '.') {
              cv = new CifHBond(ci1->data, ha->data, ci2->data, ev);
            }
            else {
              cv = new CifHBond(ci1->data, ha->data, ci2->data,
                SymmCodeToMatrix(Row[SymmA]->GetStringValue()), ev);
            }
            DataManager.AddValue(cv);
          }
        }
      }
    }
    catch (const TExceptionBase &e) {
      TBasicApp::NewLogEntry(logException) << "Failed to read the '" <<
        ALoop->GetName() << " loop: " << e.GetException()->GetFullMessage();
    }
  }
  if ((ALoop = FindLoop("_geom_angle")) != 0) {
    try {
      const size_t ind_l = ALoop->ColIndex("_geom_angle_atom_site_label_1");
      const size_t ind_m = ALoop->ColIndex("_geom_angle_atom_site_label_2");
      const size_t ind_r = ALoop->ColIndex("_geom_angle_atom_site_label_3");
      const size_t ind_a = ALoop->ColIndex("_geom_angle");
      const size_t ind_sl = ALoop->ColIndex("_geom_angle_site_symmetry_1");
      const size_t ind_sr = ALoop->ColIndex("_geom_angle_site_symmetry_3");
      if ((ind_l | ind_m | ind_r | ind_a | ind_sl | ind_sr) != InvalidIndex) {
        TEValueD ev;
        smatd im;
        im.I();
        for (size_t i = 0; i < ALoop->RowCount(); i++) {
          const CifRow& Row = (*ALoop)[i];
          ev = Row[ind_a]->GetStringValue();
          AtomCifEntry * ci1 = dynamic_cast<AtomCifEntry *>(Row[ind_l]);
          AtomCifEntry * ci2 = dynamic_cast<AtomCifEntry *>(Row[ind_m]);
          AtomCifEntry * ci3 = dynamic_cast<AtomCifEntry *>(Row[ind_r]);
          if (ci1 != 0 && ci2 != 0 && ci3 != 0) {
            ACifValue* cv = 0;
            if (Row[ind_sl]->GetStringValue() == '.' &&
              Row[ind_sr]->GetStringValue() == '.')
            {
              cv = new CifAngle(ci1->data, ci2->data, ci3->data, ev);
            }
            else {
              cv = new CifAngle(ci1->data, ci2->data, ci3->data,
                Row[ind_sl]->GetStringValue() == '.' ? im
                : SymmCodeToMatrix(Row[ind_sl]->GetStringValue()),
                Row[ind_sr]->GetStringValue() == '.' ? im
                : SymmCodeToMatrix(Row[ind_sr]->GetStringValue()),
                ev);
            }
            DataManager.AddValue(cv);
          }
        }
      }
    }
    catch (const TExceptionBase &e) {
      TBasicApp::NewLogEntry(logException) << "Failed to read the '" <<
        ALoop->GetName() << " loop: " << e.GetException()->GetFullMessage();
    }
  }
  // read in the dispersion values for types
  if ((ALoop = FindLoop("_atom_type")) != 0) {
    try {
      const size_t ind_s = ALoop->ColIndex("_atom_type_symbol");
      const size_t ind_r = ALoop->ColIndex("_atom_type_scat_dispersion_real");
      const size_t ind_i = ALoop->ColIndex("_atom_type_scat_dispersion_imag");
      if ((ind_s | ind_r | ind_i) != InvalidIndex) {
        for (size_t i = 0; i < ALoop->RowCount(); i++) {
          const CifRow& r = (*ALoop)[i];
          XScatterer* sc = new XScatterer(r[ind_s]->GetStringValue());
          TEValueD fp = r[ind_r]->GetStringValue();
          TEValueD fdp = r[ind_i]->GetStringValue();
          sc->SetFpFdp(compd(fp.GetV(), fdp.GetV()));
          GetRM().AddSfac(*sc);
        }
      }
    }
    catch (const TExceptionBase &e) {
      throw TFunctionFailedException(__OlxSourceInfo, e,
        olxstr("while reading ").quote() << ALoop->GetName() << " loop");
    }
  }
  // read in the dispersion values for sites
  if ((ALoop = FindLoop("_atom_site_dispersion")) != 0) {
    try {
      const size_t ind_l = ALoop->ColIndex("_atom_site_dispersion_label");
      const size_t ind_r = ALoop->ColIndex("_atom_site_dispersion_real");
      const size_t ind_i = ALoop->ColIndex("_atom_site_dispersion_imag");
      if ((ind_l | ind_r | ind_i) != InvalidIndex) {
        for (size_t i = 0; i < ALoop->RowCount(); i++) {
          const CifRow& r = (*ALoop)[i];
          AtomCifEntry* ci = dynamic_cast<AtomCifEntry*>(r[ind_l]);
          if (ci == 0) {
            TBasicApp::NewLogEntry(logError) <<
              (olxstr("Wrong atom in the disp loop ").quote() <<
                ALoop->Get(i, ALabel).GetStringValue() << " removing");
            ALoop->DelRow(i--);
            continue;
          }
          TEValueD fp = r[ind_r]->GetStringValue();
          TEValueD fdp = r[ind_i]->GetStringValue();
          ci->data.GetDisp() = new Disp(compd(fp.GetV(), fdp.GetV()));
          ci->data.GetDisp()->fp_su = fp.GetE();
          ci->data.GetDisp()->fdp_su = fdp.GetE();
        }
      }
    }
    catch (const TExceptionBase& e) {
      throw TFunctionFailedException(__OlxSourceInfo, e,
        olxstr("while reading ").quote() << ALoop->GetName() << " loop");
    }

  }
  // identify EXYZ/EADP
  {
    TAsymmUnit &au = GetAsymmUnit();
    sorted::PrimitiveAssociation<long, TCAtom*> atom_map;
    for (size_t i = 0; i < au.AtomCount(); i++) {
      TCAtom &a = au.GetAtom(i);
      atom_map.Add(olx_round(a.ccrd().Prod() * 1000), &a);
      a.SetTag(0);
    }
    for (size_t i = 0; i < au.AtomCount(); i++) {
      TCAtom &a = au.GetAtom(i);
      if (a.GetTag() != 0) {
        continue;
      }
      long av = olx_round(a.ccrd().Prod() * 1000);
      TSizeList idx;
      atom_map.GetIndices(av, idx);
      if (idx.IsEmpty()) {
        throw TFunctionFailedException(__OlxSourceInfo, "assert");
      }
      if (idx.Count() < 2) {
        continue;
      }
      for (size_t j = 0; j < idx.Count(); j++) {
        if (!a.ccrd().Equals(atom_map.GetValue(idx[j])->ccrd(), 1e-2)) {
          idx.Delete(j--);
        }
      }
      if (idx.Count() < 2) {
        continue;
      }
      TExyzGroup &g = GetRM().ExyzGroups.New();
      for (size_t j = 0; j < idx.Count(); j++) {
        g.Add(*atom_map.GetValue(idx[j])).SetTag(1);
      }
    }
  }
}
//..............................................................................
cetTable* TCif::LoopFromDef(CifBlock& dp, const TStrList& col_names)  {
  cetTable* tab = new cetTable();
  for (size_t i = 0; i < col_names.Count(); i++) {
    tab->AddCol(col_names[i]);
  }
  return &(cetTable&)dp.Add(tab);
}
//..............................................................................
cetTable& TCif::AddLoopDef(const olxstr& col_names, bool replace) {
  TStrList toks(col_names, ',');
  olxstr name = cetTable::GenerateName(toks);
  cetTable *CF = FindLoop(name);
  if (CF != 0) {
    if (!replace) {
      if (toks.Count() != CF->ColCount()) {
        replace = true;
      }
      if (!replace) {
        for (size_t i = 0; i < toks.Count(); i++) {
          if (CF->ColIndex(toks[i]) == InvalidIndex) {
            replace = true;
            break;
          }
        }
      }
    }
    if (replace) {
      data_provider[block_index].Remove(name);
      return *LoopFromDef(data_provider[block_index], toks);
    }
    return *CF;
  }
  return *LoopFromDef(data_provider[block_index], toks);
}
//..............................................................................
bool TCif::Add(const cetTable& tab, bool update_atom_deps) {
  if (tab.RowCount() == 0) {
    return true;
  }
  cetTable *t = FindLoop(tab.GetName());
  if (t == 0) {
    data_provider[block_index].Add(tab.Replicate());
    return true;
  }
  if (t->RowCount() == 0) {
    return t->Add(tab, false, false);
  }
  TEBitArray to_hash(t->ColCount());
  for (size_t j = 0; j < t->ColCount(); j++) {
    bool do_hash = t->ColName(j).Contains("atom_site_label") ||
      t->ColName(j).Contains("site_symmetry");
    to_hash.Set(j, do_hash);
  }
  return t->Add(tab, true, update_atom_deps,
    to_hash.CountTrue() == 0 ? 0 : &to_hash);
}
//..............................................................................
cetTable& TCif::GetPublicationInfoLoop() {
  const static olxstr publ_ln("_publ_author"), publ_jn("_publ_requested_journal");
  cetTable *CF = FindLoop(publ_ln);
  if (CF != NULL)  return *CF;
  // to make the automatic grouping to work ...
  if (!ParamExists(publ_jn)) {
    data_provider[block_index].Add(new cetString(publ_jn, "?"));
  }
  return *LoopFromDef(data_provider[block_index],
    "_publ_author_name,_publ_author_email,_publ_author_address");
}
//..............................................................................
bool TCif::Adopt(TXFile& XF, int flags) {
  Clear();
  double Q[6], E[6];  // quadratic form of thermal ellipsoid
  GetRM().Assign(XF.GetRM(), true);
  if (flags != 0) {
    ASObjectProvider& objects = XF.GetLattice().GetObjects();
    for (size_t i = 0; i < objects.atoms.Count(); i++) {
      TSAtom& a = objects.atoms[i];
      if (!a.IsAvailable() || a.IsAUAtom()) {
        continue;
      }
      TCAtom& ca = AsymmUnit.NewAtom();
      ca.SetLabel(a.GetLabel(), false);
      ca.SetPart(a.CAtom().GetPart());
      ca.SetType(a.GetType());
      ca.ccrd() = a.ccrd();
      if (a.IsAUAtom()) {
        ca.ccrdEsd() = a.CAtom().ccrdEsd();
      }
      ca.SetOccu(a.CAtom().GetOccu());
      ca.SetOccuEsd(a.CAtom().GetOccuEsd());
      ca.SetUiso(a.CAtom().GetUiso());
      ca.SetUisoEsd(a.CAtom().GetUisoEsd());
      if (a.GetEllipsoid() != 0) {
        TEllipsoid& e = AsymmUnit.NewEllp();
        e = *a.GetEllipsoid();
        ca.SetEllpId(AsymmUnit.EllpCount() - 1);
      }
      for (size_t ei = 0; ei < a.CAtom().EquivCount(); ei++) {
        ca.AddEquiv(a.CAtom().GetEquiv(ei));
      }
    }
  }
  {
    LabelCorrector lc(GetAsymmUnit(), TXApp::GetMaxLabelLength(),
      TXApp::DoRenameParts());
    for (size_t i = 0; i < AsymmUnit.AtomCount(); i++) {
      lc.CorrectGlobal(AsymmUnit.GetAtom(i));
    }
  }

  Title = TEFile::ChangeFileExt(
    TEFile::ExtractFileName(XF.GetFileName()), EmptyString());
  data_provider.Clear();
  data_provider.Add(Title.Replace(' ', "%20"));
  block_index = 0;
  {
    olex2::IOlex2Processor* op = olex2::IOlex2Processor::GetInstance();
    olxstr ad = "Olex2";
    if (op != 0) {
      olxstr ci = "GetCompilationInfo('full')";
      if (op->processFunction(ci, EmptyString(), true)) {
        ad << ": " << ci;
      }
    }
    SetParam("_audit_creation_method", ad, true);
  }
  SetParam("_chemical_name_systematic", "?", true);
  SetParam("_chemical_name_common", "?", true);
  SetParam("_chemical_melting_point", "?", false);
  SetParam("_chemical_formula_moiety",
    XF.GetLattice().CalcMoietyStr(false), true);
  SetParam("_chemical_formula_sum", GetAsymmUnit()._SummFormula(' ',
    1. / olx_max(GetAsymmUnit().GetZPrime(), 0.01)), true);
  SetParam("_chemical_formula_weight",
    olxstr::FormatFloat(2, GetAsymmUnit().MolWeight()), false);
  const TAsymmUnit& au = GetAsymmUnit();
  SetParam("_cell_length_a",
    TEValueD(au.GetAxes()[0], au.GetAxisEsds()[0]).ToString(), false);
  SetParam("_cell_length_b",
    TEValueD(au.GetAxes()[1], au.GetAxisEsds()[1]).ToString(), false);
  SetParam("_cell_length_c",
    TEValueD(au.GetAxes()[2], au.GetAxisEsds()[2]).ToString(), false);

  SetParam("_cell_angle_alpha",
    TEValueD(au.GetAngles()[0], au.GetAngleEsds()[0]).ToString(), false);
  SetParam("_cell_angle_beta",
    TEValueD(au.GetAngles()[1], au.GetAngleEsds()[1]).ToString(), false);
  SetParam("_cell_angle_gamma",
    TEValueD(au.GetAngles()[2], au.GetAngleEsds()[2]).ToString(), false);
  SetParam("_cell_volume", XF.GetUnitCell().CalcVolumeEx().ToString(), false);
  SetParam("_cell_formula_units_Z", XF.GetAsymmUnit().GetZ(), false);

  TEValueD temp_v = XF.GetRM().expl.GetTempValue();
  temp_v.V() += 273.15;
  SetParam("_diffrn_ambient_temperature",
    XF.GetRM().expl.IsTemperatureSet() ? temp_v.ToString() : olxstr('?'), false);
  SetParam("_diffrn_radiation_wavelength", XF.GetRM().expl.GetRadiation(), false);
  if (XF.GetRM().expl.GetCrystalSize().QLength() > 1.e-6) {
    SetParam("_exptl_crystal_size_max", XF.GetRM().expl.GetCrystalSize()[0], false);
    SetParam("_exptl_crystal_size_mid", XF.GetRM().expl.GetCrystalSize()[1], false);
    SetParam("_exptl_crystal_size_min", XF.GetRM().expl.GetCrystalSize()[2], false);
  }
  // HKL section
  try {
    const RefinementModel::HklStat& hkl_stat = XF.GetRM().GetMergeStat();
    if (hkl_stat.TotalReflections != 0) {
      SetParam("_diffrn_reflns_number",
        hkl_stat.TotalReflections - hkl_stat.SystematicAbsencesRemoved, false);
      SetParam("_reflns_number_total", hkl_stat.UniqueReflections, false);
      const char* hkl = "hkl";
      for (int i = 0; i < 3; i++) {
        SetParam(olxstr("_diffrn_reflns_limit_") << hkl[i] << "_min",
          hkl_stat.FileMinInd[i], false);
        SetParam(olxstr("_diffrn_reflns_limit_") << hkl[i] << "_max",
          hkl_stat.FileMaxInd[i], false);
      }
      if (hkl_stat.MaxD > 0)
        SetParam("_diffrn_reflns_theta_min",
          olxstr::FormatFloat(2,
            asin(XF.GetRM().expl.GetRadiation() / (2 * hkl_stat.MaxD)) * 180 / M_PI),
          false);
      if (hkl_stat.MinD > 0)
        SetParam("_diffrn_reflns_theta_max",
          olxstr::FormatFloat(2,
            asin(XF.GetRM().expl.GetRadiation() / (2 * hkl_stat.MinD)) * 180 / M_PI),
          false);
      SetParam("_diffrn_reflns_av_R_equivalents",
        olxstr::FormatFloat(4, hkl_stat.Rint), false);
      SetParam("_diffrn_reflns_av_unetI/netI",
        olxstr::FormatFloat(4, hkl_stat.Rsigma), false);
    }
  }
  catch (const TExceptionBase&) {
    TBasicApp::NewLogEntry() << __OlxSrcInfo << ": failed to update HKL "
      "statistics section of the CIF";
  }
  if (XF.GetAsymmUnit().IsQPeakMinMaxInitialised()) {
    SetParam("_refine_diff_density_max", XF.GetAsymmUnit().GetMaxQPeak(),
      false);
  }
  TSpaceGroup& sg = XF.GetLastLoaderSG();
  SetParam("_space_group_crystal_system",
    sg.GetBravaisLattice().GetName().ToLowerCase(), true);
  SetParam("_space_group_name_H-M_alt", sg.GetFullName(), true);
  SetParam("_space_group_name_Hall", sg.GetHallSymbol(), true);
  SetParam("_space_group_IT_number", sg.GetNumber(), false);
  {
    cetTable& Loop = AddLoopDef("_space_group_symop_id,"
      "_space_group_symop_operation_xyz");
    sg.GetMatrices(Matrices, mattAll);
    for (size_t i = 0; i < Matrices.Count(); i++) {
      CifRow& row = Loop.AddRow();
      row[0] = new cetString(i + 1);
      row[1] = new cetString(TSymmParser::MatrixToSymmEx(Matrices[i]));
    }
  }

  cetTable& atom_loop = AddLoopDef(
    "_atom_site_label,_atom_site_type_symbol,_atom_site_fract_x,"
    "_atom_site_fract_y,_atom_site_fract_z,_atom_site_U_iso_or_equiv,"
    "_atom_site_adp_type,_atom_site_occupancy,_atom_site_refinement_flags_posn,"
    "_atom_site_site_symmetry_order,_atom_site_disorder_group");

  cetTable& u_loop = AddLoopDef(
    "_atom_site_aniso_label,_atom_site_aniso_U_11,"
    "_atom_site_aniso_U_22,_atom_site_aniso_U_33,_atom_site_aniso_U_23,"
    "_atom_site_aniso_U_13,_atom_site_aniso_U_12");

  for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
    TCAtom& A = GetAsymmUnit().GetAtom(i);
    if (A.IsDeleted() || A.GetType() == iQPeakZ)  continue;
    CifRow& Row = atom_loop.AddRow();
    Row[0] = new cetString(A.GetResiLabel());
    Row[1] = new cetString(A.GetType().symbol);
    for (size_t j = 0; j < 3; j++) {
      Row.Set(j + 2,
        new cetString(TEValueD(A.ccrd()[j], A.ccrdEsd()[j]).ToString()));
    }
    Row.Set(5, new cetString(TEValueD(A.GetUiso(), A.GetUisoEsd()).ToString()));
    Row.Set(6, new cetString(A.GetEllipsoid() == 0 ? "Uiso" : "Uani"));
    Row.Set(7, new cetString(TEValueD(olx_round(A.GetChemOccu(), 1000),
      A.GetOccuEsd() * A.GetDegeneracy()).ToString()));
    if (A.GetParentAfixGroup() != 0 && A.GetParentAfixGroup()->IsRiding()) {
      Row.Set(8, new cetString("R"));
    }
    else {
      Row.Set(8, new cetString('.'));
    }
    Row.Set(9, new cetString(A.GetDegeneracy()));
    // process part as well
    if (A.GetPart() != 0) {
      Row[10] = new cetString((int)A.GetPart());
    }
    else {
      Row[10] = new cetString('.');
    }
    if (A.GetEllipsoid() != 0) {
      A.GetEllipsoid()->GetShelxQuad(Q, E);
      GetAsymmUnit().UcartToUcif(Q);
      CifRow& Row1 = u_loop.AddRow();
      Row1[0] = new AtomCifEntry(A);
      for (size_t j = 0; j < 6; j++) {
        Row1.Set(j + 1, new cetString(TEValueD(Q[j], E[j]).ToString()));
      }
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
olxstr TCif::MatrixToCode(const smatd& m) const {
  return TSymmParser::MatrixToSymmCode(Matrices, m);
}
//..............................................................................
const_strlist CIF_ParseArgumemnts(const olxstr &arg, olxch q) {
  TStrList args;
  size_t s = 0;
  for (size_t i = 0; i < arg.Length(); i++) {
    if (arg[i] == q) {
      while (++i < arg.Length() && (arg[i] != q || is_escaped(arg, i)))
        ;
    }
    else if (arg[i] == ',' && !is_escaped(arg, i)) {
      args.Add(arg.SubString(s, i - s));
      s = i + 1;
    }
  }
  if (s < arg.Length()) {
    args.Add(arg.SubStringFrom(s));
  }
  return args;
}

bool TCif::ResolveParamsFromDictionary(TStrList &Dic, olxstr &String,
 olxch Quote,
 olxstr (*ResolveExternal_)(const olxstr& valueName),
 bool DoubleTheta) const
{
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  size_t start, end;
  for (size_t i=0; i < String.Length(); i++) {
    if (String.CharAt(i) == Quote) {
      if ((i+1) < String.Length() && String.CharAt(i+1) == Quote) {
        String.Delete(i, 1);
        continue;
      }
      if (i > 0 && String.CharAt(i-1) == '\\')  // escaped?
        continue;
      olxstr Val;
      if( (i+1) < String.Length() &&
          (String.CharAt(i+1) == '$' || String.CharAt(i+1) == '_' ||
          olxstr::o_isdigit(String.CharAt(i+1))) )
      {
        start = i;
        while (++i < String.Length()) {
          if (String.CharAt(i) == Quote) {
            if ((i+1) < String.Length() && String.CharAt(i+1) == Quote) {
              String.Delete(i, 1);
              Val << Quote;
              continue;
            }
            else if (String.CharAt(i-1) == '\\') // escaped?
              ;
            else {
              end = i;
              break;
            }
          }
          Val << String.CharAt(i);
        }
      }
      if (!Val.IsEmpty()) {
        if (!Val.IsNumber()) {
          if (Val.StartsFrom('$')) {
            if (op != NULL ) {
              String.Delete(start, end-start+1);
              Val.Replace("\\%", '%');
              olxstr Tmp;
              size_t ob = Val.FirstIndexOf('('),
                cb = Val.LastIndexOf(')');
              if (ob < cb && cb != InvalidIndex) {
                TStrList args = CIF_ParseArgumemnts(
                  Val.SubString(ob + 1, cb - ob - 1), Quote);
                for (size_t ai = 0; ai < args.Count(); ai++) {
                  ResolveParamsFromDictionary(Dic, args[ai], Quote, ResolveExternal_);
                  op->processFunction(args[ai]);
                  args[ai] = unquote(args[ai]);
                }
                ABasicFunction *f = op->GetLibrary().FindFunction(Val.SubString(1, ob-1));
                if (f != NULL) {
                  TMacroData e;
                  f->Run(TStrObjList(args), e);
                  if (e.IsSuccessful())
                    Tmp = e.GetRetVal();
                }
                else {
                  TBasicApp::NewLogEntry(logInfo) << "Undefined function '" <<
                    Val.SubString(1, ob - 1) << '\'';
                }
              }
              else if (ResolveExternal_ != NULL) {
                ResolveParamsFromDictionary(Dic, Val, Quote, ResolveExternal_);
                Tmp = ResolveExternal_(Val);
              }
              ResolveParamsFromDictionary(Dic, Tmp, Quote, ResolveExternal_);
              String.Insert(Tmp, start);
              i = start + Tmp.Length() - 1;
            }
          }
          else if (Val.CharAt(0) == '_') {
            olxstr val_name = Val, new_line = ' ';
            const size_t c_i = Val.IndexOf(',');
            if (c_i != InvalidIndex) {
              val_name = Val.SubStringTo(c_i);
              new_line = Val.SubStringFrom(c_i+1);
            }
            IStringCifEntry* Params = FindParam<IStringCifEntry>(val_name);
            olxstr Tmp = "<font color=red>N/A</font>";
            if (Params != NULL && Params->Count() != 0) {
              Tmp = (*Params)[0];
              for (size_t pi=1; pi < Params->Count(); pi++)
                Tmp << new_line << (*Params)[pi];
            }
            String.Delete(start, end-start+1);
            String.Insert(Tmp, start);
            i = start + Tmp.Length() - 1;
          }
          else {
            TBasicApp::NewLogEntry() <<
              "A number or function starting from '$' or '_' is expected, "
              "found: '" << Val << '\'';
          }
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
        if ((index > Dic.Count()) || (index <= 0))
          TBasicApp::NewLogEntry(logError) << "Wrong parameter index " << index;
        else {  // resolve indexes
          String.Delete(start, end-start+1);
          olxstr SVal = Dic[index-1];
          olxstr value;
          if (!SVal.IsEmpty()) {
            if (SVal.Equalsi("date"))
              value = TETime::FormatDateTime(TETime::Now());
            else if (SVal.Equalsi("sg_number")) {
              TSpaceGroup &sg = TSymmLib::GetInstance().FindSG(GetAsymmUnit());
              if (sg.GetNumber() > 0)
                value = sg.GetNumber();
              else
                value = "unknown";
            }
            else if (SVal.Equalsi("data_name"))
              value = GetDataName();
            else if (SVal.Equalsi("weighta"))
              value = WeightA;
            else if (SVal.Equalsi("weightb"))
              value = WeightB;
            else {
              IStringCifEntry* Params = FindParam<IStringCifEntry>(SVal);
              if (Params == NULL) {
                TBasicApp::NewLogEntry(logInfo) << "The parameter \'" <<
                  SVal << "' is not found";
                value = "<font color=red>N/A</font>";
              }
              else if (Params->Count() == 0) {
                TBasicApp::NewLogEntry(logInfo) << "Value of parameter \'" <<
                  SVal << "' is not found";
                  value = "none";
              }
              else if (Params->Count() == 1) {
                if ((*Params)[0].IsEmpty()) {
                  TBasicApp::NewLogEntry(logInfo) << "Value of parameter \'" <<
                    SVal << "' is not found";
                  value = "none";
                }
                else if ((*Params)[0].CharAt(0) == '?') {
                  TBasicApp::NewLogEntry(logInfo) << "Value of parameter \'" <<
                    SVal << "' is not defined";
                  value = '?';
                }
                else {
                  if (DoubleTheta &&
                      (index == 13 || index == 14 || index == 30 ||
                       index == 61 || index == 62))
                  {
                    value = (*Params)[0].ToDouble() * 2;
                  }
                  else {
                  value = (*Params)[0];
                  }
                }
              }
              else {
                value = (*Params)[0];
                for (size_t sti=1; sti < Params->Count(); sti++) {
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
//..............................................................................
bool Cif_ValidateColumn(const TDataItem &col, const ICifEntry &entry) {
  olxstr val = entry.GetStringValue();
  if (val.IsNumber())
    val.TrimFloat();
  olxstr Tmp = col.FindField("mustequal", EmptyString());
  TStrList Toks(Tmp, ';');
   // equal to
  if (!Tmp.IsEmpty() && (Toks.IndexOfi(val) == InvalidIndex))
    return false;
  Tmp = col.FindField("mustnotequal", EmptyString());
  Toks.Clear();
  Toks.Strtok(Tmp, ';');
  if (!Tmp.IsEmpty() && (Toks.IndexOfi(val) != InvalidIndex) ) // not equal to
    return false;

  Tmp = col.FindField("atypeequal", EmptyString());
  if (!Tmp.IsEmpty()) {  // check for atom type equals to
    if (entry.Is<AtomCifEntry>())
      if (!((AtomCifEntry&)entry).data.GetType().symbol.Equalsi(Tmp)) {
        return false;
      }
  }
  Tmp = col.FindField("atypenotequal", EmptyString());
  if (!Tmp.IsEmpty()) {  // check for atom type equals to
    if (entry.Is<AtomCifEntry>())
      if (((AtomCifEntry&)entry).data.GetType().symbol.Equalsi(Tmp)) {
        return false;
      }
  }
  return true;
}
bool TCif::CreateTable(TDataItem *TD, TTTable<TStrList> &Table,
  smatd_list& SymmList, int label_options) const
{
  int RowDeleted=0, ColDeleted=0;
  SymmList.Clear();
  const CifTable* LT = NULL;
  for (size_t i=0; i < LoopCount(); i++) {
    LT = &GetLoop(i).GetData();
    if (LT->ColCount() < TD->ItemCount()) {
      LT = NULL;
      continue;
    }
    size_t defcnt = 0;
    for (size_t j=0; j < LT->ColCount(); j++) {
      if (TD->FindItemi(LT->ColName(j)) != NULL) {
        defcnt++;
      }
    }
    if (defcnt == TD->ItemCount()) {
      break;
    }
    else {
      LT = NULL;
    }
  }
  if (LT == NULL || LT->RowCount() == 0) {
    return false;
  }
  Table.Resize(LT->RowCount(), LT->ColCount());
  for (size_t i =0; i < Table.ColCount(); i++) {
    Table.ColName(i) = LT->ColName(i);
    for (size_t j = 0; j < Table.RowCount(); j++) {
      Table[j][i] = (*LT)[j][i]->GetStringValue();
      if (label_options == 0) {
        continue;
      }
      AtomCifEntry *ae = dynamic_cast<AtomCifEntry *>((*LT)[j][i]);
      if (ae == 0) {
        continue;
      }
      size_t ls = ae->data.GetType().GetSymbol().Length();
      const olxstr al = ae->data.GetLabel();
      olxstr sf = al.Length() > ls ? al.SubStringFrom(ls) : EmptyString();
      if (sf.IsEmpty()) {
        continue;
      }
      if ((label_options & 1) == 1) {
        sf = olxstr('(') << sf << ')';
      }
      if (ae->data.GetResiId() != 0) {
        sf << '_' << ae->data.GetResiId();
      }
      if ((label_options & 2) == 2) {
        Table[j][i] = olxstr(ae->data.GetType().GetSymbol()) <<
          "<sub>" << sf << "</sub>";
      }
      else if ((label_options & 4) == 4) {
        Table[j][i] = olxstr(ae->data.GetType().GetSymbol()) <<
          "<sup>" << sf << "</sup>";
      }
      else {
        Table[j][i] = olxstr(ae->data.GetType().GetSymbol()) << sf;
      }
    }
  }
  // process rows
  for (size_t i=0; i < LT->RowCount(); i++) {
    bool AddRow = true;
    for (size_t j=0; j < LT->ColCount(); j++) {
      TDataItem *DI = TD->FindItemi(LT->ColName(j));
      if (LT->ColName(j).StartsFrom("_geom_") &&
          LT->ColName(j).Contains("site_symmetry"))
      {
        if ((*LT)[i][j]->GetStringValue() != '.') {  // 1_555
          olxstr tmp = LT->ColName(j).SubStringFrom(
            LT->ColName(j).LastIndexOf('_')+1);
          //if( !tmp.IsNumber() ) continue;
          olxstr Tmp = "label_";
          Tmp << tmp;
          smatd SymmMatr = SymmCodeToMatrix((*LT)[i][j]->GetStringValue());
          size_t matIndex = SymmList.IndexOf(SymmMatr);
          if (matIndex == InvalidIndex) {
            SymmList.AddCopy(SymmMatr);
            matIndex = SymmList.Count()-1;
          }
          for (size_t k=0; k < Table.ColCount(); k++) {
            if (Table.ColName(k).EndsWith(Tmp)) {
              Table[i-RowDeleted][k] << "<sup>" << (matIndex+1) << "</sup>";
              break;
            }
          }
        }
      }
      if (DI == NULL) {
        continue;
      }
      if (!Cif_ValidateColumn(*DI, *(*LT)[i][j])) {
        AddRow = false;
        break;
      }
      olxstr Tmp = DI->FindField("multiplier", EmptyString());
      if (!Tmp.IsEmpty()) {  // Multiply
        olxstr Val = Table[i-RowDeleted][j];
        MultValue(Val, Tmp);
        Table[i-RowDeleted][j] = Val;
      }
      for (size_t sc=0; sc < DI->ItemCount(); sc++) {
        size_t ci = LT->ColIndex(DI->GetItemByIndex(sc).GetName());
        if (ci == InvalidIndex) continue;
        if (Cif_ValidateColumn(DI->GetItemByIndex(sc), *(*LT)[i][ci])) {
          Table[i - RowDeleted][j] << DI->GetItemByIndex(sc).FindField("before")
            << (*LT)[i][ci]->GetStringValue()
            << DI->GetItemByIndex(sc).FindField("after");
        }
        else {
          AddRow = false;
          break;
        }
      }
    }
    if (!AddRow) {
      Table.DelRow(i-RowDeleted);
      RowDeleted++;
    }
  }
  // process columns
  olex2::IOlex2Processor *ip = olex2::IOlex2Processor::GetInstance();
  for (size_t i=0; i < LT->ColCount(); i++) {
    TDataItem *DI = TD->FindItemi(LT->ColName(i));
    if (DI != NULL) {
      Table.ColName(i-ColDeleted) = DI->FindField("caption");
      olxstr v = DI->FindField("visible", FalseString());
      if (!v.IsBool() && ip != NULL)
        ip->processFunction(v);
      if (!v.ToBool()) {
        Table.DelCol(i-ColDeleted);
        ColDeleted++;
      }
    }
    else {
      Table.DelCol(i-ColDeleted);
      ColDeleted++;
    }
  }
  return true;
}
//..............................................................................
void TCif::ToDataItem(TDataItem &di_) const {
  TDataItem &di = di_.AddItem("data");
  olxstr_dict<olxstr> out_map;
  for (size_t i = 0; i < data_provider.Count(); i++)  {
    CifBlock& cb = data_provider[i];
    olxstr block_name = cb.GetName();
    if (block_name.IsEmpty() || block_name.IsNumber()) {
      block_name = out_map(block_name, olxstr("data_") << block_name);
    }
    TDataItem &block = di.AddItem(block_name);
    for (size_t j = 0; j < cb.params.Count(); j++) {
      if (!cb.params.GetObject(j)->HasName()) {
        block.AddItem("comment", cb.params.GetObject(j)->GetStringValue());
        continue;
      }
      olxstr item_name = cb.params[j];
      if (item_name.ContainAnyOf("/%")) {
        out_map(cb.params[j],
          item_name.Replace('/', "_over_")
          .Replace('%', "percent")
          );
      }
      TStrList toks(item_name, '_');
      for (size_t ti = 0; ti < toks.Count(); ti++) {
        if (ti > 0) {
          if (olxstr::o_isdigit(toks[ti].CharAt(0))) {
            toks[ti - 1] << '_' << toks[ti];
            toks.Delete(ti--);
          }
        }
      }
      olxstr tab_name;
      ICifEntry *ice = cb.params.GetObject(j);
      IStringCifEntry *ise = dynamic_cast<IStringCifEntry*>(ice);
      cetTable *tab = dynamic_cast<cetTable*>(ice);
      if (tab != NULL) {
        tab_name = toks.GetLastString();
        toks.Delete(toks.Count() - 1);
      }
      size_t idx = 0;
      TDataItem *p = block.FindItem(toks[idx]);
      while (p != NULL) {
        if (++idx >= toks.Count()) break;
        TDataItem *p1 = p->FindItem(toks[idx]);
        if (p1 == 0) break;
        p = p1;
      }
      if (p == 0) p = &block;
      for (size_t k = idx; k < toks.Count(); k++) {
        p = &p->AddItem(toks[k]);
      }
      if (ise != 0) {
        TStrList cnt;
        if (ise->HasComment())
          cnt.Add('#') << ise->GetComment();
        for (size_t li = 0; li < ise->Count(); li++)
          cnt << (*ise)[li];
          p->SetValue(cnt.Text(NewLineSequence()));
      }
      else if (tab != NULL) {
        p = &p->AddItem("table");
        const size_t nf = tab->GetName().IsEmpty() ? 0
          : (tab->GetName().Length()+1);
        for (size_t tr = 0; tr < tab->RowCount(); tr++) {
          TDataItem &tri = p->AddItem(tab_name);
          for (size_t td = 0; td < tab->ColCount(); td++) {
            TStrList cnt;
            IStringCifEntry *tce = dynamic_cast<IStringCifEntry *>(
              (*tab)[tr][td]);
            for (size_t li = 0; li < tce->Count(); li++)
              cnt << (*tce)[li];
            olxstr name;
            if (tab->ColName(td).Length() <= nf) {
              name = "value";
            }
            else
              name = tab->ColName(td).SubStringFrom(nf);
            tri.AddItem(name, cnt.Text(NewLineSequence()));
          }
        }
      }
    }
  }
  if (!out_map.IsEmpty()) {
    TDataItem &tr = di_.AddItem("translation");
    for (size_t i = 0; i < out_map.Count(); i++) {
      tr.AddItem(out_map.GetValue(i), out_map.GetKey(i));
    }
  }
}
//..............................................................................
void DataItemToCifEntry(const TDataItem &root, const TDataItem &di,
  olxstr_dict<olxstr> &translations, CifBlock &out)
{
  if (di.ItemCount() == 0) {
    if (di.GetName().Equals("comment")) {
      out.Add(new cetComment(di.GetValue()));
    }
    else {
      olxstr name = olxstr('_') << di.GetFullName('_', &root);
      name = translations.Find(name, name);
      if (di.GetValue().Contains('\n')) {
        cetStringList *sl = new cetStringList(name);
        sl->lines.Strtok(di.GetValue(), '\n', false);
        for (size_t i = 0; i < sl->lines.Count(); i++) {
          sl->lines[i].TrimR('\r');
        }
        out.Add(sl);
      }
      else {
        out.Add(new cetString(name, di.GetValue()));
      }
    }
  }
  else if (di.GetName().Equals("table")) {
    if (di.ItemCount() == 0) return;
    olxstr cols;
    for (size_t i = 0; i < di.GetItemByIndex(0).ItemCount(); i++) {
      olxstr cn = olxstr('_') <<
        di.GetItemByIndex(0).GetItemByIndex(i).GetFullName('_', &root);
      if (cn.EndsWith("_value")) {
        cn.SetLength(cn.Length()-6);
      }
      cols << cn.Replace("_table", EmptyString()) << ',';
    }
    cols.SetLength(cols.Length() - 1);
    cetTable *tab = new cetTable(cols, di.ItemCount());
    for (size_t i = 0; i < di.ItemCount(); i++) {
      for (size_t j = 0; j < tab->ColCount(); j++) {
        tab->Set(i, j,
          new cetString(di.GetItemByIndex(i).GetItemByIndex(j).GetValue()));
      }
    }
    out.Add(tab);
  }
  else {
    for (size_t i = 0; i < di.ItemCount(); i++) {
      DataItemToCifEntry(root, di.GetItemByIndex(i), translations, out);
    }
  }
}
//..............................................................................
void TCif::FromDataItem(const TDataItem &di_) {
  olxstr_dict<olxstr> translations;
  {
    TDataItem *t = di_.FindItem("translation");
    if (t != 0) {
      for (size_t i = 0; i < t->ItemCount(); i++) {
        const TDataItem &ti = t->GetItemByIndex(i);
        translations.Add(ti.GetName(), ti.GetValue());
      }
    }
  }
  data_provider.Clear();
  TDataItem &di = di_.GetItemByName("data");
  for (size_t i = 0; i < di.ItemCount(); i++) {
    TDataItem &data = di.GetItemByIndex(i);
    CifBlock &cb = data_provider.Add(
      translations.Find(data.GetName(), data.GetName()));
    DataItemToCifEntry(data, data, translations, cb);
  }
}
//..............................................................................
olx_object_ptr<olx_pair_t<cif_dp::cetTable*, size_t> >
  TCif::FindLoopItem(const olxstr &name, size_t block_idx)
{
  size_t bix = get_bix(block_idx);
  for (size_t i = 0; i < data_provider[bix].table_map.Count(); i++) {
    cetTable *tab = data_provider[bix].table_map.GetValue(i);
    size_t ci = tab->ColIndex(name);
    if (ci != InvalidIndex) {
      return olx_pair::New(tab, ci);
    }
  }
  return 0;
}
//..............................................................................
//..............................................................................
//..............................................................................
void AtomCifEntry::ToStrings(TStrList& list) const {
  olxstr al = data.GetResiLabel(save_part);
  if (list.IsEmpty() ||
    (list.GetLastString().Length() + al.Length() + 1 > 80))
  {
    list.Add(' ') << al;
  }
  else {
    list.GetLastString() << ' ' << al;
  }
}
//..............................................................................
olxstr AtomCifEntry::GetStringValue() const {
  return data.GetResiLabel(save_part);
}
//..............................................................................
//..............................................................................
//..............................................................................
void SymmCifEntry::ToStrings(TStrList& list) const {
  olxstr al = GetStringValue();
  if (list.IsEmpty() ||
    (list.GetLastString().Length() + al.Length() + 1 > 80))
  {
    list.Add(' ') << al;
  }
  else {
    list.GetLastString() << ' ' << al;
  }
}
//..............................................................................
