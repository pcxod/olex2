/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xfiles.h"
#include "efile.h"
#include "xapp.h"
#include "unitcell.h"
#include "catom.h"
#include "library.h"
#include "ins.h"
#include "crs.h"
#include "cif.h"
#include "hkl.h"
#include "cif.h"
#include "utf8file.h"
#include "atomsort.h"
#include "infotab.h"
#include "absorpc.h"
#include "analysis.h"
#include "estopwatch.h"

TBasicCFile::TBasicCFile()
  : RefMod(AsymmUnit), AsymmUnit(NULL)
{
  AsymmUnit.SetRefMod(&RefMod);
}
//..............................................................................
TBasicCFile::~TBasicCFile()  {
  /* this must be called, as the AU might get destroyed beforehand and then
  AfixGroups cause crash
  */
  RefMod.Clear(rm_clear_ALL);
}
//..............................................................................
void TBasicCFile::SaveToFile(const olxstr& fn)  {
  TStopWatch sw(__FUNC__);
  TStrList L;
  sw.start("Saving to strings...");
  SaveToStrings(L);
  sw.start("UTF8 encoding to file");
  TUtf8File::WriteLines(fn, L, false);
  FileName = fn;
};
//..............................................................................
void TBasicCFile::PostLoad() {
  /* fix labels for not native formats, will not help for FE1A, because it
  could come from Fe1A or from Fe1a ...
  */
  if (!IsNative()) {
    for (size_t i=0; i < AsymmUnit.AtomCount(); i++) {
      TCAtom& a = AsymmUnit.GetAtom(i);
      if (a.GetType().symbol.Length() == 2 &&
          a.GetLabel().StartsFromi(a.GetType().symbol))
      {
        a.SetLabel(a.GetType().symbol +
          a.GetLabel().SubStringFrom(a.GetType().symbol.Length()), false);
      }
    }
  }
}
//..............................................................................
void TBasicCFile::LoadFromStream(IInputStream &is, const olxstr& nameToken) {
  TStrList lines;
  lines.LoadFromTextStream(is);
  LoadStrings(lines, nameToken);
}
//..............................................................................
void TBasicCFile::LoadStrings(const TStrList &lines, const olxstr &nameToken) {
  FileName.SetLength(0);
  Title.SetLength(0);
  TXFile::NameArg file_n(nameToken);
  if (lines.IsEmpty()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "empty content");
  }
  try {
    FileName = file_n.file_name;
    LoadFromStrings(lines);
    if (EsdlInstanceOf(*this, TCif)) {
      if (!file_n.data_name.IsEmpty()) {
        if (file_n.is_index) {
          ((TCif*)this)->SetCurrentBlock(file_n.data_name.ToSizeT());
        }
        else {
          ((TCif*)this)->SetCurrentBlock(file_n.data_name);
        }
      }
      else {  // set first then
        ((TCif*)this)->SetCurrentBlock(InvalidIndex);
      }
    }
  }
  catch (const TExceptionBase& exc) {
    FileName.SetLength(0);
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  FileName = nameToken;
  PostLoad();
}
//..............................................................................
void TBasicCFile::LoadFromFile(const olxstr& _fn)  {
  TStopWatch(__FUNC__);
  TXFile::NameArg file_n(_fn);
  TEFile::CheckFileExists(__OlxSourceInfo, file_n.file_name);
  TStrList L = TEFile::ReadLines(file_n.file_name);
  if (L.IsEmpty()) {
    throw TEmptyFileException(__OlxSourceInfo, _fn);
  }
  try {
    LoadStrings(L, _fn);
  }
  catch (const TExceptionBase& exc) {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  FileName = file_n.file_name;
}
//..............................................................................
void TBasicCFile::GenerateCellForCartesianFormat() {
  TAsymmUnit &au = GetAsymmUnit();
  vec3d miv(1000), mav(-1000);
  for (size_t i = 0; i < au.AtomCount(); i++) {
    vec3d::UpdateMinMax(au.GetAtom(i).ccrd(), miv, mav);
  }
  au.GetAngles() = vec3d(90, 90, 90);
  au.GetAxes() = mav - miv;
  if (au.GetAxes().IsNull()) {
    au.GetAxes() = vec3d(1);
  }
  else {
    for (int i = 0; i < 3; i++) {
      if (au.GetAxes()[i] < 1e-3) {
        au.GetAxes()[i] = 1;
      }
    }
    au.GetAxes() += 2;
    for (size_t i = 0; i < au.AtomCount(); i++) {
      TCAtom &a = au.GetAtom(i);
      a.ccrd() /= au.GetAxes();
    }
  }
  au.InitMatrices();
}
//..............................................................................
void TBasicCFile::RearrangeAtoms(const TSizeList & new_indices) {
  if (new_indices.Count() != GetAsymmUnit().AtomCount()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "invalid list of indices");
  }
  GetRM().BeforeAUSort_();
  GetAsymmUnit().GetAtoms().Rearrange(new_indices);
  GetRM().Sort_();
  index_t idx = 0;
  for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
    TCAtom &a = GetAsymmUnit().GetAtom(i);
    if (a.GetType() == iHydrogenZ) {
      continue;
    }
    a.SetTag(idx++);
  }
  GetRM().AfterAUSort_();
}
//----------------------------------------------------------------------------//
// TXFile function bodies
//----------------------------------------------------------------------------//
TXFile::TXFile(ASObjectProvider& Objects) :
  Lattice(Objects),
  RefMod(Lattice.GetAsymmUnit()),
  OnFileLoad(Actions.New("XFILELOAD")),
  OnFileSave(Actions.New("XFILESAVE")),
  OnFileClose(Actions.New("XFILECLOSE"))
{
  Lattice.GetAsymmUnit().SetRefMod(&RefMod);
  olx_vptr<AEventsDispatcher> vptr(new VPtr);
  Lattice.GetAsymmUnit().OnSGChange.Add(vptr, XFILE_EVT_SG_Change);
  Lattice.OnStructureUniq.Add(vptr, XFILE_EVT_UNIQ);
  FLastLoader = 0;
  FSG = 0;
}
//..............................................................................
TXFile::~TXFile() {
// finding uniq objects and deleting them
  for (size_t i = 0; i < FileFormats.Count(); i++) {
    FileFormats.GetObject(i)->SetTag(i);
  }
  for (size_t i = 0; i < FileFormats.Count(); i++) {
    if ((size_t)FileFormats.GetObject(i)->GetTag() == i) {
      delete FileFormats.GetObject(i);
    }
  }
}
//..............................................................................
void TXFile::TakeOver(TXFile &f) {
  OnFileLoad.TakeOver(f.OnFileLoad);
  OnFileSave.TakeOver(f.OnFileSave);
  /* This should be left along as it is needed for ALL files
  */
  //GetLattice().OnDisassemble.TakeOver(
  //  f.GetLattice().OnDisassemble);
  GetLattice().OnStructureGrow.TakeOver(f.GetLattice().OnStructureGrow);
  GetLattice().OnStructureUniq.TakeOver(f.GetLattice().OnStructureUniq);
  GetLattice().OnAtomsDeleted.TakeOver(f.GetLattice().OnAtomsDeleted);
}
//..............................................................................
void TXFile::RegisterFileFormat(TBasicCFile *F, const olxstr &Ext)  {
  if (FileFormats.IndexOf(Ext) != InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo, "Ext");
  }
  FileFormats.Add(Ext.ToLowerCase(), F);
}
//..............................................................................
TBasicCFile *TXFile::FindFormat(const olxstr &Ext)  {
  const size_t i = FileFormats.IndexOf(Ext.ToLowerCase());
  if (i == InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo, "unknown file format");
  }
  return FileFormats.GetObject(i);
}
//..............................................................................
void TXFile::LastLoaderChanged() {
  if (FLastLoader == NULL) {
    GetRM().Clear(rm_clear_ALL);
    GetLattice().Clear(true);
    return;
  }
  FSG = &TSymmLib::GetInstance().FindSG(FLastLoader->GetAsymmUnit());
  OnFileLoad.Enter(this, &FLastLoader->GetFileName());
  GetRM().Clear(rm_clear_ALL);
  GetLattice().Clear(true);
  GetRM().Assign(FLastLoader->GetRM(), true);
  OnFileLoad.Execute(this);
  GetLattice().Init();
  OnFileLoad.Exit(this, &FLastLoader->GetFileName());
}
//..............................................................................
bool TXFile::Dispatch(int MsgId, short MsgSubId, const IOlxObject* Sender,
  const IOlxObject* Data, TActionQueue *)
{
  if (MsgId == XFILE_EVT_SG_Change) {
    if (Data == NULL || !EsdlInstanceOf(*Data, TSpaceGroup))
      throw TInvalidArgumentException(__OlxSourceInfo, "space group");
    FSG = const_cast<TSpaceGroup*>(dynamic_cast<const TSpaceGroup*>(Data));
    GetRM().ResetHklStats();
  }
  else if (MsgId == XFILE_EVT_UNIQ && MsgSubId == msiEnter) {
    //RefMod.Validate();
    //UpdateAsymmUnit();
    //GetAsymmUnit().PackAtoms();
    //if( !FLastLoader->IsNative() )  {
    //  FLastLoader->GetRM().Validate();
    //  FLastLoader->GetAsymmUnit().PackAtoms();
    //}
  }
  else {
    return false;
  }
  return true;
}
//..............................................................................
void TXFile::PostLoad(const olxstr &fn, TBasicCFile *Loader, bool replicated) {
  TStopWatch sw(__FUNC__);
  for (size_t i = 0; i < Loader->GetAsymmUnit().AtomCount(); i++) {
    TCAtom &a = Loader->GetAsymmUnit().GetAtom(i);
    if (olx_abs(a.ccrd()[0]) > 127 ||
      olx_abs(a.ccrd()[1]) > 127 ||
      olx_abs(a.ccrd()[2]) > 127)
    {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("atom coordinates for ").quote() << a.GetLabel());
    }
  }
  if (!Loader->IsNative()) {
    OnFileLoad.Enter(this, &fn);
    try {
      GetRM().Clear(rm_clear_ALL);
      GetLattice().Clear(true);
      GetRM().Assign(Loader->GetRM(), true);
      OnFileLoad.Execute(this);
      GetLattice().Init();
    }
    catch(const TExceptionBase& exc)  {
      OnFileLoad.Exit(this);
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    OnFileLoad.Exit(this);
  }
  TSpaceGroup &sg = TSymmLib::GetInstance().FindSG(Loader->GetAsymmUnit());;
  if (FSG != &sg) {
    GetRM().ResetHklStats();
    FSG = &sg;
  }
  if (replicated) {
    for (size_t i = 0; i < FileFormats.Count(); i++) {
      if (FileFormats.GetObject(i) == FLastLoader) {
        FileFormats.GetObject(i) = Loader;
      }
    }
    delete FLastLoader;
  }
  FLastLoader = Loader;
  if (GetRM().GetHKLSource().IsEmpty() ||
     !TEFile::Exists(GetRM().GetHKLSource()))
  {
    olxstr src = LocateHklFile();
    if (!src.IsEmpty() && !TEFile::Existsi(olxstr(src), src)) {
      src.SetLength(0);
    }
    GetRM().SetHKLSource(src);
    try {
      if (src.IsEmpty()) {
        GetRM().SetReflections(TRefList());
        if (EsdlInstanceOf(*FLastLoader, TCif)) {
          TCif &cif = GetLastLoader<TCif>();
          cif_dp::cetTable* hklLoop = cif.FindLoop("_refln");
          if (hklLoop == 0) {
            // sorting out tonto loop
            hklLoop = cif.FindLoop("_diffrn_refln");
          }
          if (hklLoop != 0) {
            try {
              olx_object_ptr<THklFile::ref_list> refs =
                THklFile::FromCifTable(*hklLoop);
              if (refs.is_valid()) {
                GetRM().SetReflections(refs().a);
              }
              //if (!refs.b) {
              //  GetRM().SetHKLF(3);
              //}
            }
            catch (TExceptionBase &) {}
          }
          else {
            cif_dp::cetStringList *ci = dynamic_cast<cif_dp::cetStringList *>(
              cif.FindEntry("_shelx_hkl_file"));
            if (ci != NULL) {
              THklFile hkf;
              hkf.LoadFromStrings(ci->lines, false);
              GetRM().SetReflections(hkf.RefList());
            }
          }
        }
      }
    }
    catch (const TExceptionBase &e) {
      TBasicApp::NewLogEntry(logWarning) << "Failed to extract HKL data from "
        "CIF";
    }
  }
  // try resolve the residues
  if (false && EsdlInstanceOf(*FLastLoader, TCif)) {
    using namespace olx_analysis;
    TAsymmUnit &au = GetAsymmUnit();
    TEBitArray processed(au.ResidueCount());
    for (size_t i = 1; i < au.ResidueCount(); i++) {
      if (processed[i]) {
        continue;
      }
      TResidue &r1 = au.GetResidue(i);
      fragments::fragment fr1(r1.GetAtomList());
      for (size_t j = i+1; j < au.ResidueCount(); j++) {
        if (processed[j]) {
          continue;
        }
        TResidue &r2 = au.GetResidue(j);
        if (r1.Count() != r2.Count()) {
          continue;
        }
        TTypeList<fragments::fragment> matching =
          fragments::extract(r2.GetAtomList(), fr1);
        if (!matching.IsEmpty()) {
          r2.GetAtomList().ForEach(ACollectionItem::IndexTagSetter());
          TSizeList new_order = TSizeList::FromList(
            matching[0].atoms(),
            FunctionAccessor::MakeConst<index_t,TCAtom>(&TCAtom::GetTag));
          r2.GetAtomList().Rearrange(new_order);
        }
      }
    }

  }
  TXApp::GetInstance().SetLastSGResult_(EmptyString());
}
//..............................................................................
void TXFile::LoadFromStrings(const TStrList& lines, const olxstr &nameToken) {
  TStopWatch(__FUNC__);
  // this thows an exception if the file format loader does not exist
  const NameArg file_n(nameToken);
  const olxstr ext(TEFile::ExtractFileExt(file_n.file_name));
  TBasicCFile* Loader = FindFormat(ext);
  bool replicated = false;
  if (FLastLoader == Loader) {
    Loader = dynamic_cast<TBasicCFile *>(Loader->Replicate());
    replicated = true;
  }
  try {
    Loader->LoadStrings(lines, nameToken);
  }
  catch (const TExceptionBase& exc) {
    if (replicated) {
      delete Loader;
    }
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  PostLoad(EmptyString(), Loader, replicated);
}
//..............................................................................
void TXFile::LoadFromStream(IInputStream& in, const olxstr &nameToken) {
  TStopWatch(__FUNC__);
  // this thows an exception if the file format loader does not exist
  const NameArg file_n(nameToken);
  const olxstr ext(TEFile::ExtractFileExt(file_n.file_name));
  TBasicCFile* Loader = FindFormat(ext);
  bool replicated = false;
  if (FLastLoader == Loader) {
    Loader = dynamic_cast<TBasicCFile *>(Loader->Replicate());
    replicated = true;
  }
  try {
    Loader->LoadFromStream(in, nameToken);
  }
  catch (const TExceptionBase& exc) {
    if (replicated) {
      delete Loader;
    }
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  PostLoad(EmptyString(), Loader, replicated);
}
//..............................................................................
void TXFile::LoadFromFile(const olxstr & _fn) {
  TStopWatch(__FUNC__);
  const NameArg file_n(_fn);
  const olxstr ext(TEFile::ExtractFileExt(file_n.file_name));
  // this thows an exception if the file format loader does not exist
  TBasicCFile* Loader = FindFormat(ext);
  bool replicated = false;
  if (FLastLoader == Loader) {
    Loader = dynamic_cast<TBasicCFile *>(Loader->Replicate());
    replicated = true;
  }
  try {
    Loader->LoadFromFile(_fn);
    olxstr hkl_src = Loader->GetRM().GetHKLSource();
    if (!hkl_src.IsEmpty() && !TEFile::IsAbsolutePath(hkl_src)) {
      hkl_src = TEFile::ExpandRelativePath(hkl_src,
        TEFile::ExtractFilePath(_fn));
      Loader->GetRM().SetHKLSource(hkl_src);
    }
  }
  catch (const TExceptionBase& exc) {
    if (replicated) {
      delete Loader;
    }
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  PostLoad(_fn, Loader, replicated);
}
//..............................................................................
void TXFile::UpdateAsymmUnit() {
  TBasicCFile* LL = FLastLoader;
  if (LL->IsNative()) {
    return;
  }
  GetLattice().UpdateAsymmUnit();
  LL->GetAsymmUnit().ClearEllps();
  for (size_t i = 0; i < GetAsymmUnit().EllpCount(); i++) {
    LL->GetAsymmUnit().NewEllp() = GetAsymmUnit().GetEllp(i);
  }
  for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
    TCAtom& CA = GetAsymmUnit().GetAtom(i);
    TCAtom& CA1 = LL->GetAsymmUnit().AtomCount() <= i ?
      LL->GetAsymmUnit().NewAtom() : LL->GetAsymmUnit().GetAtom(i);
    CA1.Assign(CA);
  }
  LL->GetAsymmUnit().AssignResidues(GetAsymmUnit());
  RefMod.Validate();
  ValidateTabs();
  LL->GetRM().Assign(RefMod, false);
  LL->GetAsymmUnit().SetZ(GetAsymmUnit().GetZ());
  LL->GetAsymmUnit().GetAxes() = GetAsymmUnit().GetAxes();
  LL->GetAsymmUnit().GetAxisEsds() = GetAsymmUnit().GetAxisEsds();
  LL->GetAsymmUnit().GetAngles() = GetAsymmUnit().GetAngles();
  LL->GetAsymmUnit().GetAngleEsds() = GetAsymmUnit().GetAngleEsds();
}
//..............................................................................
void TXFile::Sort(const TStrList& ins, const TParamList &options) {
  if (FLastLoader == 0) {
    return;
  }
  olxdict<TCAtom *, size_t, TPointerComparator> original_ids;
  if (!FLastLoader->IsNative()) {
    UpdateAsymmUnit();
    original_ids.SetCapacity(GetAsymmUnit().AtomCount());
    for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
      original_ids.Add(&GetAsymmUnit().GetAtom(i), i);
    }
  }
  const bool sort_resi_n = options.GetBoolOption("rn", false, true);
  GetRM().BeforeAUSort_();
  AtomSorter::CombiSort default_sorter;
  default_sorter.sequence.AddNew(&AtomSorter::atom_cmp_Mw);
  default_sorter.sequence.AddNew(&AtomSorter::atom_cmp_Label);
  TStrList labels;
  TCAtomPList &list = GetAsymmUnit().GetResidue(0).GetAtomList();
  size_t moiety_index = InvalidIndex, h_cnt = 0, del_h_cnt = 0, free_h_cnt = 0;
  bool keeph = true;
  for (size_t i = 0; i < list.Count(); i++) {
    if (list[i]->GetType() == iHydrogenZ) {
      if (!list[i]->IsDeleted()) {
        h_cnt++;
        if (list[i]->GetParentAfixGroup() == 0) {
          free_h_cnt++;
        }
      }
      else {
        del_h_cnt++;
      }
    }
  }
  if (h_cnt == 0 || del_h_cnt != 0) {
    keeph = false;
    if (del_h_cnt != 0 && free_h_cnt != 0) {
      TBasicApp::NewLogEntry(logError) << "Hydrogen atoms, which are not "
        "attached using AFIX will not be kept with pivot atom until the file "
        "is reloaded";
    }
  }
  try {
    AtomSorter::CombiSort acs;
    olxstr sort;
    for (size_t i = 0; i < ins.Count(); i++) {
      if (ins[i].CharAt(0) == '+') {
        sort << ins[i].SubStringFrom(1);
      }
      else if (ins[i].Equalsi("moiety")) {
        moiety_index = i;
        break;
      }
      else {
        labels.Add(ins[i]);
        }
    }
    bool insert_at_fisrt_label = false,
      label_swap = false;
    for (size_t i = 0; i < sort.Length(); i++) {
      if (sort.CharAt(i) == 'm') {
        acs.sequence.AddNew(&AtomSorter::atom_cmp_Mw);
      }
      else if (sort.CharAt(i) == 'z') {
        acs.sequence.AddNew(&AtomSorter::atom_cmp_Z);
      }
      else if (sort.CharAt(i) == 'l') {
        acs.sequence.AddNew(&AtomSorter::atom_cmp_Label);
      }
      else if (sort.CharAt(i) == 'p') {
        acs.sequence.AddNew(&AtomSorter::atom_cmp_Part);
      }
      else if (sort.CharAt(i) == 'h') {
        keeph = false;
      }
      else if (sort.CharAt(i) == 's') {
        acs.sequence.AddNew(&AtomSorter::atom_cmp_Suffix);
      }
      else if (sort.CharAt(i) == 'n') {
        acs.sequence.AddNew(&AtomSorter::atom_cmp_Number);
      }
      else if (sort.CharAt(i) == 'x') {
        acs.sequence.AddNew(&AtomSorter::atom_cmp_MoietySize);
      }
      else if (sort.CharAt(i) == 'f') {
        insert_at_fisrt_label = true;
      }
      else if (sort.CharAt(i) == 'w') {
        label_swap = true;
      }
    }
    if (!acs.sequence.IsEmpty()) {
      if (!labels.IsEmpty()) {
        for (size_t i = 0; i < acs.sequence.Count(); i++) {
          acs.sequence[i].AddExceptions(labels);
        }
      }
      AtomSorter::Sort(list, acs);
    }
    if (label_swap) {
      AtomSorter::ReorderByName(list, labels);
    }
    else {
      AtomSorter::SortByName(list, labels, insert_at_fisrt_label);
    }
    labels.Clear();
    if (moiety_index != InvalidIndex) {
      MoietySorter::CombiSort mcs;
      TTypeList<MoietySorter::moiety_t> moieties =
        MoietySorter::SplitIntoMoieties(list);
      sort.SetLength(0);
      if (moiety_index + 1 < ins.Count()) {
        for (size_t i = moiety_index + 1; i < ins.Count(); i++) {
          if (ins[i].CharAt(0) == '+') {
            sort << ins[i].SubStringFrom(1);
          }
          else {
            labels.Add(ins[i]);
          }
        }
        for (size_t i = 0; i < sort.Length(); i++) {
          if (sort.CharAt(i) == 'l') {
            mcs.sequence.Add(&MoietySorter::moiety_cmp_label);
          }
          if (sort.CharAt(i) == 's') {
            mcs.sequence.Add(&MoietySorter::moiety_cmp_size);
          }
          else if (sort.CharAt(i) == 'h') {
            mcs.sequence.Add(&MoietySorter::moiety_cmp_heaviest);
          }
          else if (sort.CharAt(i) == 'm') {
            mcs.sequence.Add(&MoietySorter::moiety_cmp_mass);
          }
        }
        if (!mcs.sequence.IsEmpty()) {
          QuickSorter::SortMF(moieties, mcs, &MoietySorter::CombiSort::moiety_cmp);
        }
      }
      if (!labels.IsEmpty()) {
        MoietySorter::ReoderByMoietyAtom(moieties, labels);
      }
      MoietySorter::UpdateList(list, moieties);
    }
    if (keeph) {
      // this will not work with SAME
      AtomSorter::KeepH(list, GetAsymmUnit(), &AtomSorter::atom_cmp_Label);
    }
  }
  catch (const TExceptionBase& exc) {
    TBasicApp::NewLogEntry(logError) << exc.GetException()->GetError();
  }
  
  if (sort_resi_n) {
    GetAsymmUnit().SortResidues();
    if (!FLastLoader->IsNative()) {
      FLastLoader->GetAsymmUnit().SortResidues();
    }
  }
  if (options.GetBoolOption("r", false, true)) {
    // apply default sorting to the residues
    for (size_t i = 1; i < GetAsymmUnit().ResidueCount(); i++) {
      AtomSorter::Sort(GetAsymmUnit().GetResidue(i).GetAtomList(),
        default_sorter);
    }
  }
  // comply to residues before dry-save
  {
    size_t ac = 0;
    for (size_t i = 0; i < GetAsymmUnit().ResidueCount(); i++) {
      TResidue& resi = GetAsymmUnit().GetResidue(i);
      for (size_t j = 0; j < resi.Count(); j++) {
        resi[j].SetTag(ac++);
      }
    }
    QuickSorter::Sort(GetAsymmUnit().GetAtoms(), ACollectionItem::TagComparator());
  }
  {
    TSizeList indices = TIns::DrySave(GetAsymmUnit());
    if (indices.Count() != GetAsymmUnit().AtomCount()) {
      throw TFunctionFailedException(__OlxSourceInfo, "assert");
    }
    GetAsymmUnit().GetAtoms().Rearrange(indices);
  }
  GetAsymmUnit().GetAtoms().ForEach(ACollectionItem::IndexTagSetter());
  GetRM().Sort_();
  index_t idx = 0;
  for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
    TCAtom &a = GetAsymmUnit().GetAtom(i);
    if (a.GetType() == iHydrogenZ) {
      continue;
    }
    a.SetTag(idx++);
  }
  GetRM().AfterAUSort_();
  if (!FLastLoader->IsNative()) {
    TSizeList new_indices(original_ids.Count(), olx_list_init::zero());
    for (size_t i = 0; i < GetAsymmUnit().AtomCount(); i++) {
      new_indices[i] = 
        original_ids[&GetAsymmUnit().GetAtom(i)];
    }
    FLastLoader->RearrangeAtoms(new_indices);
    FLastLoader->GetAsymmUnit().AssignResidues(GetAsymmUnit());
  }
  GetAsymmUnit()._UpdateAtomIds();
  // 2010.11.29, ASB bug fix for ADPS on H...
  GetUnitCell().UpdateEllipsoids();
  GetLattice().RestoreADPs(false);
}
//..............................................................................
void TXFile::UpdateAtomIds() {
  if (!FLastLoader->IsNative()) {
    UpdateAsymmUnit();
  }
  TAsymmUnit &au = GetAsymmUnit();
  TSizeList indices = TIns::DrySave(au);
  olxdict<TCAtom *, size_t, TPointerComparator> original_ids;
  if (!FLastLoader->IsNative()) {
    original_ids.SetCapacity(au.AtomCount());
    for (size_t i = 0; i < au.AtomCount(); i++) {
      original_ids.Add(&au.GetAtom(i), i);
    }
  }
  if (indices.Count() != au.AtomCount()) {
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  }
  bool uniform = true;
  for (size_t i = 0; i < indices.Count(); i++) {
    if (indices[i] != i) {
      uniform = false;
      break;
    }
  }
  if (uniform) {
    return;
  }
  GetRM().BeforeAUSort_();
  au.GetAtoms().Rearrange(indices);
  au.GetAtoms().ForEach(ACollectionItem::IndexTagSetter());
  GetRM().Sort_();
  au.ComplyToResidues();
  if (!FLastLoader->IsNative()) {
    TSizeList new_indices(original_ids.Count(), olx_list_init::zero());
    for (size_t i = 0; i < au.AtomCount(); i++) {
      new_indices[i] = original_ids[&GetAsymmUnit().GetAtom(i)];
    }
    FLastLoader->GetAsymmUnit().GetAtoms().Rearrange(new_indices);
    FLastLoader->GetAsymmUnit().ComplyToResidues();
  }
  index_t idx = 0;
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    if (a.GetType() == iHydrogenZ) {
      continue;
    }
    a.SetTag(idx++);
  }
  GetRM().AfterAUSort_();
}
//..............................................................................
void TXFile::ValidateTabs() {
  for (size_t i = 0; i < RefMod.InfoTabCount(); i++) {
    if (RefMod.GetInfoTab(i).GetType() != infotab_htab)
      continue;
    if (!RefMod.GetInfoTab(i).GetAtoms().IsExplicit()) continue;
    TTypeList<ExplicitCAtomRef> ta =
      RefMod.GetInfoTab(i).GetAtoms().ExpandList(GetRM(), 2);
    if (ta.IsEmpty()) continue;
    TSAtom* sa = NULL;
    InfoTab& it = RefMod.GetInfoTab(i);
    bool hasH = false;
    for (size_t j = 0; j < ta[0].GetAtom().AttachedSiteCount(); j++) {
      TCAtom &aa = ta[0].GetAtom().GetAttachedAtom(j);
      if (!aa.IsDeleted() && aa.GetType() == iHydrogenZ) {
        hasH = true;
        break;
      }
    }
    if (!hasH) {
      TBasicApp::NewLogEntry() << "Removing HTAB (donor has no H atoms): "
        << it.InsStr();
      RefMod.DeleteInfoTab(i--);
      continue;
    }
    // validate the distance makes sense
    const TAsymmUnit& au = *ta[0].GetAtom().GetParent();
    vec3d v1 = ta[0].GetAtom().ccrd();
    if (ta[0].GetMatrix() != 0) {
      v1 = *ta[0].GetMatrix()*v1;
    }
    vec3d v2 = ta[1].GetAtom().ccrd();
    if (ta[1].GetMatrix() != 0) {
      v2 = *ta[1].GetMatrix()*v2;
    }
    const double dis = au.CellToCartesian(v1).DistanceTo(au.CellToCartesian(v2));
    if (dis > 5) {
      TBasicApp::NewLogEntry() << "Removing HTAB (d > 5A): " << it.InsStr();
      RefMod.DeleteInfoTab(i--);
      continue;
    }
  }
}
//..............................................................................
void TXFile::SaveToFile(const olxstr& FN, int flags) {
  TStopWatch sw(__FUNC__);
  olxstr Ext = TEFile::ExtractFileExt(FN);
  TBasicCFile *Loader = FindFormat(Ext);
  TBasicCFile *LL = FLastLoader;
  if (!Loader->IsNative()) {
    if (LL != Loader) {
      if (!Loader->Adopt(*this, 1)) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "could not adopt specified file format");
      }
    }
    else {
      UpdateAsymmUnit();
    }
    //if (Sort)
    //  Loader->GetAsymmUnit().Sort();
  }
  OnFileSave.Enter(this);
  IOlxObject* Cause = NULL;
  try {
    if (!TBasicApp::GetInstance().GetOptions()
      .FindValue("absolute_hkl_path", FalseString()).ToBool())
    {
      olxstr hkl_src = Loader->GetRM().GetHKLSource();
      olxstr root = TEFile::ExtractFilePath(FN);
      if (root.IsEmpty()) {
        root = TEFile::CurrentDir();
      }
      olxstr hs = TEFile::CreateRelativePath(hkl_src, root);
      Loader->GetRM().SetHKLSource(hs);
      Loader->SaveToFile(FN);
      Loader->GetRM().SetHKLSource(hkl_src);
    }
    else {
      Loader->SaveToFile(FN);
    }
  }
  catch (const TExceptionBase& exc) {
    Cause = exc.Replicate();
  }
  OnFileSave.Exit(this);
  if (Cause != 0) {
    throw TFunctionFailedException(__OlxSourceInfo, Cause);
  }
}
//..............................................................................
void TXFile::Close() {
  OnFileClose.Enter(this, FLastLoader);
  FLastLoader = NULL;
  RefMod.Clear(rm_clear_ALL);
  Lattice.Clear(true);
  OnFileClose.Exit(this, NULL);
}
//..............................................................................
IOlxObject* TXFile::Replicate() const {
  TXFile* xf = ((SObjectProvider*)Lattice.GetObjects().Replicate())->
    CreateXFile();
  for (size_t i=0; i < FileFormats.Count(); i++) {
    xf->RegisterFileFormat(
      dynamic_cast<TBasicCFile *>(FileFormats.GetObject(i)->Replicate()),
      FileFormats[i]);
  }
  return xf;
}
//..............................................................................
void TXFile::EndUpdate() {
  OnFileLoad.Enter(this, &GetFileName());
  OnFileLoad.Execute(this);
  // we keep the asymmunit but clear the unitcell
  try {
    GetLattice().Init();
    OnFileLoad.Exit(this);
  }
  catch (const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
    Close();
  }
}
//..............................................................................
void TXFile::ToDataItem(TDataItem& item) {
  GetLattice().ToDataItem(item.AddItem("Lattice"));
  GetRM().ToDataItem(item.AddItem("RefModel"));
}
//..............................................................................
void TXFile::FromDataItem(const TDataItem& item) {
  GetRM().Clear(rm_clear_ALL);
  GetLattice().FromDataItem(item.GetItemByName("Lattice"));
  GetRM().FromDataItem(item.GetItemByName("RefModel"));
  GetLattice().FinaliseLoading();
}
//..............................................................................
const_strlist TXFile::ToJSON() const {
  TStrList out;
  vec3d center;
  out << '{';
  const ASObjectProvider &objects = GetLattice().GetObjects();
  for (size_t i=0; i < objects.atoms.Count(); i++)
    center += objects.atoms[i].crd();
  if (!objects.atoms.IsEmpty())
    center /= objects.atoms.Count();

  out << "\"atoms\": [";
  index_t cnt=0;
  for (size_t i=0; i < objects.atoms.Count(); i++) {
    TSAtom &a = objects.atoms[i];
    if (!a.IsAvailable()) continue;
    a.SetTag(cnt++);
    olxstr &l = (out.Add(" {\"type\":").quote('"') << a.GetType().symbol);
    l << ", \"label\":\"" << a.GetLabel() << "\", \"crd\":[";
    vec3d crd = a.crd() - center;
    l << crd[0] << ',' << crd[1] << ',' << crd[2] << "]";
    if (a.GetEllipsoid() != NULL) {
      l << ", \"matrix\":[";
      mat3d m = a.GetEllipsoid()->GetMatrix();
      m[0] *= a.GetEllipsoid()->GetSX();
      m[1] *= a.GetEllipsoid()->GetSY();
      m[2] *= a.GetEllipsoid()->GetSZ();
      for (int mi=0; mi < 3; mi++)
        for (int mj=0; mj < 3; mj++)
          l << m[mi][mj] << ',';
      l.SetLength(l.Length()-1); // strip last comma
      l << ']';
    }
    l << ", \"Uiso\":" << a.CAtom().GetUiso();
    if (a.CAtom().GetQPeak() != 0) {
      l << ", \"Peak\":" << a.CAtom().GetQPeak();
    }
    uint32_t cl = a.GetType().def_color;
    l << ", \"color\":[" <<
      (float)(cl&0xFF)/255 << ',' <<
      (float)((cl>>8)&0xFF)/255 << ',' <<
      (float)((cl>>16)&0xFF)/255 << ']';

    l << "},";
  }
  if (cnt > 0) {
    out.GetLastString().SetLength(out.GetLastString().Length()-1);
    cnt = 0;
  }
  out << ']'; // atoms

  out << ", \"bonds\": [";
  for (size_t i=0; i < objects.bonds.Count(); i++) {
    TSBond &b = objects.bonds[i];
    if (!b.A().IsAvailable() || !b.B().IsAvailable())
      continue;
    olxstr &l = out.Add();
    l << " {\"from\":" << b.A().GetTag() << ", \"to\":" << b.B().GetTag();
    if (b.GetType() == sotHBond)
      l << ", \"stippled\": true";
    l << "},";
    cnt++;
  }
  if (cnt > 0) {
    out.GetLastString().SetLength(out.GetLastString().Length()-1);
  }
  out << ']'; // bonds
  {
    const mat3d& ucm = GetAsymmUnit().GetCellToCartesian();
    olxstr &l = out.Add(", \"cell\": {");
    l << "\"o\":[" << -center[0] << ',' << -center[1] << ',' << -center[2] << ']';
    for (int i=0; i < 3; i++) {
      l << ", \"" << (olxch)('a'+i) << "\":[";
      for (int j=0; j < 3; j++) {
        l << (ucm[i][j]-center[j]);
        if (j != 2) l << ',';
      }
      l << ']';
    }
    l << '}'; // cell
  }
  return (out << '}');
}
//..............................................................................
//..............................................................................
//..............................................................................
IOlxObject *TXFile::VPtr::get_ptr() const {
  TXApp &app = TXApp::GetInstance();
  return app.XFiles().IsEmpty() ? 0 : &app.XFile();
}
//..............................................................................
//..............................................................................
//..............................................................................
void TXFile::LibGetFormula(const TStrObjList& Params, TMacroData& E)  {
  bool list = false, html = false, split = false, unit=false;
  int digits = -1;
  if( Params.Count() > 0 )  {
    if( Params[0].Equalsi("list") )
      list = true;
    else if( Params[0].Equalsi("html") )
      html = true;
    else if( Params[0].Equalsi("split") )
      split = true;
    else if( Params[0].Equalsi("unit") )
      unit = true;
  }
  if (Params.Count() == 2)
    digits = Params[1].ToInt();

  const ContentList& content = GetRM().GetUserContent();
  olxstr rv;
  for (size_t i=0; i < content.Count(); i++) {
    rv << content[i].element.symbol;
    if (list)  rv << ':';
    else if (split) rv << ' ';
    bool subAdded = false;
    const double dv = unit ? olx_round(content[i].count)
      : (content[i].count/GetAsymmUnit().GetZ());
    olxstr tmp = (digits > 0) ? olxstr::FormatFloat(digits, dv) : olxstr(dv);
    if (tmp.IndexOf('.') != InvalidIndex)
      tmp.TrimFloat();
    if (html) {
      if (olx_abs(dv-1) > 0.01 && olx_abs(dv) > 0.01) {
        rv << "<sub>" << tmp;
        subAdded = true;
      }
    }
    else
      rv << tmp;

    if ((i+1) <  content.Count()) {
      if (list)
        rv << ',';
      else
        if (html) {
          if( subAdded )
            rv << "</sub>";
        }
        else
          rv << ' ';
    }
    else  // have to close the html tag
      if (html && subAdded)
        rv << "</sub>";

  }
  E.SetRetVal(rv);
}
//..............................................................................
void TXFile::LibSetFormula(const TStrObjList& Params, TMacroData& E) {
  if( Params[0].IndexOf(':') == InvalidIndex )
    GetRM().SetUserFormula(Params[0]);
  else  {
    ContentList content;
    TStrList toks(Params[0], ',');
    for( size_t i=0; i < toks.Count(); i++ )  {
      size_t ind = toks[i].FirstIndexOf(':');
      if( ind == InvalidIndex )  {
        E.ProcessingError(__OlxSrcInfo, "invalid formula syntax" );
        return;
      }
      const cm_Element* elm =
        XElementLib::FindBySymbol(toks[i].SubStringTo(ind));
      if( elm == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, "element");
      content.AddNew(*elm,
        toks[i].SubStringFrom(ind+1).ToDouble()*GetAsymmUnit().GetZ());
    }
    if( content.IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "empty SFAC - check formula syntax");
      return;
    }
    GetRM().SetUserContent(content);
  }
}
//..............................................................................
void TXFile::LibEndUpdate(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  EndUpdate();
  if (Cmds.Count() == 1 && Cmds[0].ToBool()) {
    TIns * ins = dynamic_cast<TIns *>(FLastLoader);
    if (ins != NULL) {
      ins->GetLst().Clear();
    }
  }
}
//..............................................................................
void TXFile::LibSaveSolution(const TStrObjList& Params, TMacroData& E)  {
  TIns* oins = (TIns*)FLastLoader;
  TIns ins;
  // needs to be called to assign the loaderIds for new atoms
  UpdateAsymmUnit();
  ins.GetRM().Assign( GetRM(), true );
  ins.AddIns("FMAP 2", ins.GetRM());
  ins.GetRM().SetRefinementMethod("L.S.");
  ins.GetRM().SetIterations(4);
  ins.GetRM().SetPlan(20);
  ins.GetRM().SetUserContent(oins->GetRM().GetUserContent());
  ins.SaveToFile(Params[0]);
}
void TXFile::LibDataCount(const TStrObjList& Params, TMacroData& E)  {
  if (EsdlInstanceOf(*FLastLoader, TCif)) {
    E.SetRetVal(((TCif*)FLastLoader)->BlockCount());
  }
  else
    E.SetRetVal(1);
}
//..............................................................................
void TXFile::LibCurrentData(const TStrObjList& Params, TMacroData& E)  {
  if (EsdlInstanceOf(*FLastLoader, TCif)) {
    TCif &cif = *(TCif*)FLastLoader;
    if (Params.IsEmpty())
      E.SetRetVal(cif.GetBlockIndex());
    else
      cif.SetCurrentBlock(Params[0].ToInt());
  }
  else {
    if (Params.IsEmpty()) {
      E.SetRetVal(0);
    }
    else {
      E.ProcessingError(__OlxSrcInfo, "not-applicable");
    }
  }
}
//..............................................................................
void TXFile::LibDataName(const TStrObjList& Params, TMacroData& E) {
  int i = Params[0].ToInt();
  TCif &cif = *(TCif*)FLastLoader;
  if (i < 0) {
    E.SetRetVal(cif.GetDataName());
  }
  else {
    if ((size_t)i >= cif.BlockCount()) {
      throw TIndexOutOfRangeException(__OlxSourceInfo, i, 0, cif.BlockCount());
    }
    E.SetRetVal(cif.GetBlock(i).GetName());
  }
}
//..............................................................................
void TXFile::LibGetMu(const TStrObjList& Params, TMacroData& E) {
  cm_Absorption_Coefficient_Reg ac;
  ContentList cont = GetAsymmUnit().GetContentList();
  double mu = 0;
  for (size_t i = 0; i < cont.Count(); i++) {
    XScatterer *xs = GetRM().FindSfacData(cont[i].element.symbol);
    if (xs != NULL && xs->IsSet(XScatterer::setMu)) {
      mu += cont[i].count*xs->GetMu() / 10;
    }
    else {
      double v = ac.CalcMuOverRhoForE(
        GetRM().expl.GetRadiationEnergy(), *ac.locate(cont[i].element.symbol));
      mu += (cont[i].count*cont[i].element.GetMr())*v / 6.022142;
    }
  }
  mu *= GetAsymmUnit().GetZ() / GetAsymmUnit().CalcCellVolume() /
    GetAsymmUnit().GetZPrime();
  E.SetRetVal(olxstr::FormatFloat(3, mu));
}
//..............................................................................
void TXFile::LibRefinementInfo(const TStrObjList& Params, TMacroData& E) {
  TIns &ins = *(TIns*)FLastLoader;
  if (Params.IsEmpty()) {
    TStrList rv;
    for (size_t i = 0; i < ins.RefinementInfo.Count(); i++) {
      rv.Add(ins.RefinementInfo.GetKey(i)) << '=' <<
        ins.RefinementInfo.GetValue(i);
    }
    E.SetRetVal(rv.Text(';'));
  }
  else {
    ins.RefinementInfo.Clear();
    TStrList toks(Params[0].DeleteCharSet("\t \r\n"), ';');
    for (size_t i = 0; i < toks.Count(); i++) {
      size_t ei = toks[i].IndexOf('=');
      if (ei == InvalidIndex) {
        continue;
      }
      ins.RefinementInfo(toks[i].SubStringTo(ei),
        toks[i].SubStringFrom(ei + 1));
    }
  }
}
//..............................................................................
TLibrary* TXFile::ExportLibrary(const olxstr& name) {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("xf") : name);
  olx_vptr<TXFile> thip(new VPtr);
  lib->Register(
    new TFunction<TXFile>(thip, &TXFile::LibGetFormula, "GetFormula",
      fpNone|fpOne|fpTwo|psFileLoaded,
      "Returns a string for content of the asymmetric unit. Takes single or "
      "none parameters. If parameter equals 'html' and html formatted string is"
      " returned, for 'list' parameter a string like 'C:26,N:45' is returned. "
      "If no parameter is specified, just formula is returned")
   );

  lib->Register(
    new TFunction<TXFile>(thip, &TXFile::LibSetFormula, "SetFormula",
      fpOne|psCheckFileTypeIns|psCheckFileTypeP4P,
      "Sets formula for current file, takes a string of the following form "
      "'C:25,N:4'")
  );

  lib->Register(
    new TMacro<TXFile>(thip, &TXFile::LibEndUpdate, "EndUpdate",
      EmptyString(),
      fpNone|fpOne|psCheckFileTypeIns,
      "Must be called after the content of the asymmetric unit has changed - "
      "this function will update the program state. If true is passed as an "
      "argument - the loader related metainformation (like LST for INS) will "
      "be cleared too.")
  );

  lib->Register(
    new TFunction<TXFile>(thip, &TXFile::LibSaveSolution, "SaveSolution",
      fpOne|psCheckFileTypeIns,
      "Saves current Q-peak model to provided file (res-file)")
  );

  lib->Register(
    new TFunction<TXFile>(thip, &TXFile::LibDataCount, "DataCount",
      fpNone|psFileLoaded,
      "Returns number of available data sets")
  );

  lib->Register(
    new TFunction<TXFile>(thip, &TXFile::LibDataName, "DataName",
      fpOne|psCheckFileTypeCif,
      "Returns data name for given CIF block")
  );

  lib->Register(
    new TFunction<TXFile>(thip, &TXFile::LibCurrentData, "CurrentData",
      fpNone|fpOne|psFileLoaded,
      "Returns current data index or changes current data block within the CIF")
  );

  lib->Register(
    new TFunction<TXFile>(thip, &TXFile::LibGetMu, "GetMu",
      fpNone|psFileLoaded,
      "Returns absorption coefficient for current model.")
  );

  lib->Register(
    new TFunction<TXFile>(thip, &TXFile::LibRefinementInfo, "RefinementInfo",
    fpNone | fpOne | psCheckFileTypeIns,
    "Sets/returns refinement information.")
    );

  lib->AttachLibrary(Lattice.GetAsymmUnit().ExportLibrary());
  lib->AttachLibrary(Lattice.GetUnitCell().ExportLibrary());
  lib->AttachLibrary(Lattice.ExportLibrary());
  lib->AttachLibrary(RefMod.expl.ExportLibrary());
  lib->AttachLibrary(RefMod.ExportLibrary());
  lib->AttachLibrary(olx_analysis::Analysis::ExportLibrary());
  return lib;
}
//..............................................................................
void TXFile::NameArg::Parse(const olxstr& fn)  {
  this->file_name = fn;
  this->data_name.SetLength(0);
  this->is_index = false;
  const size_t hi = fn.LastIndexOf('#');
  const size_t ui = fn.LastIndexOf('$');
  if (hi == InvalidIndex && ui == InvalidIndex)
    return;
  const size_t di = fn.LastIndexOf('.');
  if (di != InvalidIndex) {
    if (hi != InvalidIndex && ui != InvalidIndex && di < hi && di < ui) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "only one data ID is allowed");
    }
    if (hi != InvalidIndex && di < hi) {
      this->data_name = fn.SubStringFrom(hi+1);
      this->file_name = fn.SubStringTo(hi);
      this->is_index = true;
    }
    else if (ui != InvalidIndex && di < ui) {
      this->data_name = fn.SubStringFrom(ui+1);
      this->file_name = fn.SubStringTo(ui);
      this->is_index = false;
    }
  }
  else {
    if (hi != InvalidIndex && ui != InvalidIndex) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "only one data ID is allowed");
    }
    if (hi != InvalidIndex) {
      this->data_name = fn.SubStringFrom(hi+1);
      this->file_name = fn.SubStringTo(hi);
      this->is_index = true;
    }
    else if (ui != InvalidIndex) {
      this->data_name = fn.SubStringFrom(ui+1);
      this->file_name = fn.SubStringTo(ui);
      this->is_index = false;
    }
  }
}
//..............................................................................
olxstr TXFile::NameArg::ToString() const {
  if (data_name.IsEmpty())
    return file_name;
  return olxstr(file_name) << (is_index ? '#' : '$') << data_name;
}
//..............................................................................
olxstr TXFile::LocateHklFile() {
  olxstr HklFN = GetRM().GetHKLSource();
  if (TEFile::Existsi(HklFN, HklFN))
    return HklFN;
  const olxstr fn = GetFileName();
  HklFN = TEFile::ChangeFileExt(fn, "hkl");
  if (TEFile::Existsi(HklFN, HklFN))
    return HklFN;
  HklFN = TEFile::ChangeFileExt(fn, "raw");
  if (TEFile::Existsi(HklFN, HklFN)) {
    THklFile Hkl;
    Hkl.LoadFromFile(HklFN, false);
    HklFN = TEFile::ChangeFileExt(fn, "hkl");
    for (size_t i=0; i < Hkl.RefCount(); i++) {
      Hkl[i].SetI((double)olx_round(Hkl[i].GetI())/100.0);
      Hkl[i].SetS((double)olx_round(Hkl[i].GetS())/100.0);
    }
    Hkl.SaveToFile(HklFN);
    TBasicApp::NewLogEntry() << "The scaled hkl file is prepared";
    return HklFN;
  }
  else {  // check for stoe format
    HklFN = TEFile::ChangeFileExt(fn, "hkl");
    olxstr HkcFN = TEFile::ChangeFileExt(fn, "hkc");
    if (TEFile::Existsi(olxstr(HkcFN), HkcFN)) {
      TEFile::Copy(HkcFN, HklFN);
      return HklFN;
    }
  }
  // last chance - get any hkl in the same folder (only if one!)
  TStrList hkl_files;
  olxstr dir = TEFile::ExtractFilePath(fn);
  TEFile::ListDir(dir, hkl_files, "*.hkl", sefFile);
  if (hkl_files.Count() == 1)
    return TEFile::AddPathDelimeterI(dir) << hkl_files[0];
  return EmptyString();
}
//..............................................................................
olxstr TXFile::GetStructureDataFolder() const {
  if (HasLastLoader()) {
    olxstr ofn = TEFile::ExtractFilePath(GetFileName());
    TEFile::AddPathDelimeterI(ofn) << ".olex/";
    if (!TEFile::Exists(ofn)) {
      if (!TEFile::MakeDir(ofn)) {
        throw TFunctionFailedException(__OlxSourceInfo, "cannot create folder");
      }
#ifdef __WIN32__
      SetFileAttributes(ofn.u_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
    }
    return ofn;
  }
  return EmptyString();
}
//..............................................................................
