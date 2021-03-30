/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "auto.h"
#include "bapp.h"
#include "log.h"
#include "actions.h"
#include "symmtest.h"
#include "symmlib.h"
#include "cif.h"
#include "etime.h"
#include "egc.h"
#include "eutf8.h"
#include "filetree.h"
#include "md5.h"

#undef GetObject

using namespace olx_analysis;
const uint16_t MaxConnectivity = 12;

// file header: Signature,Version,Flags
// Version 1 to version 2: TAutoDBFolders changes with single TAutoDBRegistry
const int16_t FileVersion = 0x0002;
// file flags
const int16_t ffZipped  = 0x0001;

const char FileSignature [] = "OADB";  // olex auto .. db
const long FileSignatureLength = 4;  // must not change ever!

//..............................................................................
void TAutoDBRegistry::SaveToStream(IDataOutputStream& output) const {
  output << (uint32_t)entries.Count();
  for (size_t i = 0; i < entries.Count(); i++) {
    output << TUtf8::Encode(entries.GetKey(i));
  }
}
//..............................................................................
void TAutoDBRegistry::LoadFromStream(IDataInputStream& input) {
  uint32_t fc;
  input >> fc;
  entries.SetCapacity(entries.Count() + fc);
  olxcstr tmp;
  for( uint32_t i=0; i < fc; i++ )  {
    input >> tmp;
    entries.Add(TUtf8::Decode(tmp), new TAutoDBIdObject(i));
  }
}
//..............................................................................
void TAutoDBRegistry::SaveMap(IDataOutputStream& output) {
  output << (uint32_t)entries.Count();
  for( size_t i=0; i < entries.Count(); i++ ) {
    output << TUtf8::Encode(entries.GetKey(i)) <<
      TUtf8::Encode(entries.GetValue(i)->reference);
  }
}
//..............................................................................
void TAutoDBRegistry::LoadMap(IDataInputStream& input) {
  uint32_t fc;
  input >> fc;
  olxcstr tmp;
  for( uint32_t i=0; i < fc; i++ )  {
    input >> tmp;
    size_t idx = entries.IndexOf(TUtf8::Decode(tmp));
    input >> tmp;
    if (idx != InvalidIndex) {
      entries.GetValue(idx)->reference = TUtf8::Decode(tmp);
    }
  }
}
//..............................................................................
void TAutoDBRegistry::Clear() {
  for (size_t i = 0; i < entries.Count(); i++) {
    delete entries.GetValue(i);
  }
  entries.Clear();
}

//..............................................................................
//..............................................................................
//..............................................................................
TAttachedNode::TAttachedNode(IDataInputStream& in) {
  uint32_t ind;
  in >> ind;
  Element = &XElementLib::GetByIndex(ind);
  float val;
  in >> val;  FCenter[0] = val;
  in >> val;  FCenter[1] = val;
  in >> val;  FCenter[2] = val;
}
//..............................................................................
void TAttachedNode::SaveToStream(IDataOutputStream& output) const {
  output << (int32_t)Element->GetIndex();
  output << (float)FCenter[0];
  output << (float)FCenter[1];
  output << (float)FCenter[2];
}
//..............................................................................
//..............................................................................
//..............................................................................
vec3d TAutoDBNode::SortCenter;

static const vec3d ZAxis(0,0,1);

int TAutoDBNode::SortMetricsFunc(const TAttachedNode &a,
  const TAttachedNode &b)
{
  return olx_cmp(TAutoDBNode::SortCenter.DistanceTo(b.GetCenter()),
                TAutoDBNode::SortCenter.DistanceTo(a.GetCenter()));
}
int TAutoDBNode::SortCAtomsFunc(const olx_pair_t<TCAtom*, vec3d> &a,
                                const olx_pair_t<TCAtom*, vec3d> &b)  {
  return olx_cmp(TAutoDBNode::SortCenter.DistanceTo(b.GetB()),
                TAutoDBNode::SortCenter.DistanceTo(a.GetB()));
}
//..............................................................................
void TAutoDBNode::FromCAtom(const TCAtom& ca, const smatd &m_,
  TTypeList<olx_pair_t<TCAtom*, vec3d> >* atoms)
{
  const TAsymmUnit& au = *ca.GetParent();
  const TUnitCell &uc = au.GetLattice().GetUnitCell();
  Center = au.Orthogonalise(m_*ca.ccrd());
  Element = &ca.GetType();

  for (size_t i = 0; i < ca.AttachedSiteCount(); i++) {
    const TCAtom::Site& site = ca.GetAttachedSite(i);
    if (ca.IsDeleted() || site.atom->GetType() == iHydrogenZ) {
      continue;
    }
    const smatd m = m_.IsFirst()
      ? site.matrix : uc.MulMatrix(site.matrix, m_);
    const vec3d p = au.Orthogonalise(m*site.atom->ccrd());
    if (atoms != 0) {
      atoms->AddNew<TCAtom*, vec3d>(site.atom, p);
    }
    AttachedNodes.Add(new TAttachedNode(&site.atom->GetType(), p));
  }
  TAutoDBNode::SortCenter = Center;
  QuickSorter::SortSF(AttachedNodes, SortMetricsFunc);
  if (atoms != 0) {
    QuickSorter::SortSF(*atoms, SortCAtomsFunc);
  }
  _PreCalc();
}
//..............................................................................
void TAutoDBNode::_PreCalc() {
  Params.Resize((AttachedNodes.Count() + 1) * AttachedNodes.Count() / 2);
  size_t index = AttachedNodes.Count();
  for (size_t i = 0; i < AttachedNodes.Count(); i++) {
    Params[i] = CalcDistance(i);
    for (size_t j = i + 1; j < AttachedNodes.Count(); j++) {
      Params[index] = CalcAngle(i, j);
      index++;
    }
  }
}
//..............................................................................
double TAutoDBNode::CalcAngle(size_t i, size_t j)  const {
  vec3d a(AttachedNodes[i].GetCenter() - Center),
    b(AttachedNodes[j].GetCenter() - Center);
  if (a.QLength() * b.QLength() == 0) {
    TBasicApp::NewLogEntry(logError) << "Overlapping atoms encountered";
    return 0;
  }
  double ca = a.CAngle(b);
  if (ca < -1) {
    ca = -1;
  }
  if (ca > 1) {
    ca = 1;
  }
  return acos(ca) * 180.0 / M_PI;
}
//..............................................................................
void TAutoDBNode::SaveToStream(IDataOutputStream& output) const {
  output << (uint32_t)Element->GetIndex();
  output << (float)Center[0];
  output << (float)Center[1];
  output << (float)Center[2];
  output << (int)AttachedNodes.Count();
  for (size_t i = 0; i < AttachedNodes.Count(); i++) {
    AttachedNodes[i].SaveToStream(output);
  }
}
//..............................................................................
void TAutoDBNode::LoadFromStream(IDataInputStream& in) {
  Element = &XElementLib::GetByIndex(in.Read<uint32_t>());
  float val;
  Center[0] = (in >> val);
  Center[1] = (in >> val);
  Center[2] = (in >> val);
  uint32_t cnt = in.Read<uint32_t>();
  for (uint32_t i = 0; i < cnt; i++) {
    AttachedNodes.Add(*(new TAttachedNode(in)));
  }
  TAutoDBNode::SortCenter = Center;
  QuickSorter::SortSF(AttachedNodes, SortMetricsFunc);
  _PreCalc();
}
//..............................................................................
olxstr TAutoDBNode::ToString() const {
  olxstr tmp = Element->symbol;
  tmp << '{';
  for (size_t i=0; i < AttachedNodes.Count(); i++) {
    tmp << AttachedNodes[i].GetType().symbol;
    if ((i + 1) < AttachedNodes.Count()) {
      tmp << ',';
    }
  }
  tmp << '}'; /* << '[';
  for( size_t i=0; i < Params.Count(); i++ )  {
    tmp << olxstr::FormatFloat(3, GetDistance(i));
    if( (i+1) < Params.Count() )  tmp << ',';
  }
  tmp << ']';   */
  return tmp;
}
//..............................................................................
double TAutoDBNode::SearchCompare(const TAutoDBNode& dbn, double* fom) const {
  double _fom = 0;
  size_t mc = (AttachedNodes.Count() > 4) ? AttachedNodes.Count()
    : Params.Count();
  // variations
  const double LengthVar = TAutoDB::GetInstance_()->GetLengthVar(),
    AngleVar = TAutoDB::GetInstance_()->GetAngleVar();
  for (size_t i = 0; i < mc; i++) {
    double diff = Params[i] - dbn.Params[i];
    if (i < AttachedNodes.Count()) {
      if (olx_abs(diff) > LengthVar) {
        return diff;
      }
    }
    else {
      if (olx_abs(diff) > AngleVar) {
        return diff / 180;
      }
      diff /= 180;
    }
    _fom += diff * diff;
  }
  if (fom != 0) {
    *fom += _fom / Params.Count();
  }
  return 0;
  /*
    for( size_t i=0; i < Params.Count(); i++ )  {
      double diff = Params[i] - dbn.Params[i];
      if( diff < 0 )  return -1;
      if( diff > 0 )  return 1;
    }
    return 0;
  */
}
//..............................................................................
int TAutoDBNode::UpdateCompare(const TAutoDBNode& dbn) const {
  int z_diff = Element->z - dbn.Element->z;
  if (z_diff == 0) {
    size_t c_diff = olx_cmp(AttachedNodes.Count(), dbn.AttachedNodes.Count());
    if (c_diff == 0) {
      for (size_t i = 0; i < AttachedNodes.Count(); i++) {
        z_diff = AttachedNodes[i].GetType().z - dbn.AttachedNodes[i].GetType().z;
        if (z_diff != 0) {
          return z_diff;
        }
      }
      for (size_t i = 0; i < Params.Count(); i++) {
        double p_diff = Params[i] - dbn.Params[i];
        if (p_diff < 0) {
          return -1;
        }
        if (p_diff > 0) {
          return 1;
        }
      }
      return 0;
    }
    else {
      return (int)c_diff;
    }
  }
  else {
    return z_diff;
  }
}
//..............................................................................
bool TAutoDBNode::IsSameType(const TAutoDBNode& dbn) const {
  int diff = Element->z - dbn.Element->z;
  if (diff != 0) {
    return false;
  }
  diff = olx_cmp(AttachedNodes.Count(), dbn.AttachedNodes.Count());
  if (diff != 0) {
    return false;
  }
  for (size_t i = 0; i < AttachedNodes.Count(); i++) {
    diff = AttachedNodes[i].GetType().z - dbn.AttachedNodes[i].GetType().z;
    if (diff != 0) {
      return false;
    }
  }
  return true;
}
//..............................................................................
bool TAutoDBNode::IsSimilar(const TAutoDBNode& dbn) const {
  int z_diff = (int)Element->z - dbn.Element->z;
  if (z_diff == 0) {
    size_t c_diff = olx_cmp(AttachedNodes.Count(), dbn.AttachedNodes.Count());
    if (c_diff == 0) {
      // check types
      for (size_t i = 0; i < AttachedNodes.Count(); i++) {
        z_diff = AttachedNodes[i].GetType().z -
          dbn.AttachedNodes[i].GetType().z;
        if (z_diff != 0) {
          return false;
        }
      }
      // check distance and angles
      for (size_t i = 0; i < Params.Count(); i++) {
        double p_diff = olx_abs(Params[i] - dbn.Params[i]);
        if (i < AttachedNodes.Count()) {
          if (p_diff > 0.005) {
            return false;
          }
        }
        else
          if (p_diff > 4) {
            return false;
          }
      }
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}
//..............................................................................
bool TAutoDBNode::IsMetricSimilar(const TAutoDBNode& dbn, double& fom) const
{
  if (AttachedNodes.Count() != dbn.AttachedNodes.Count()) {
    return false;
  }
  // check distance and angles
  double _fom = 0;
  size_t mc = (AttachedNodes.Count() > 4)
    ? AttachedNodes.Count() : Params.Count();
  const double len_var = TAutoDB::GetInstance_()->GetLengthVar(),
    ang_var = TAutoDB::GetInstance_()->GetAngleVar();
  for (size_t i = 0; i < mc; i++) {
    //  for( size_t i=0; i < Params.Count(); i++ ) {
    double diff = olx_abs(Params[i] - dbn.Params[i]);
    if (i < AttachedNodes.Count()) {
      if (diff > len_var) {
        return false;
      }
    }
    /* 5.7 degrees give about 0.1 deviation in distance for 1A bonds
    (b^2+a^2-2abcos)^1/2
    */
    else {
      if (diff > ang_var) {
        return false;
      }
      diff = diff / 180.0;
    }
    _fom += diff * diff;
  }
  fom += _fom / Params.Count();
  return true;
}
//..............................................................................
//..............................................................................
//..............................................................................
bool TAutoDBNetNode::IsMetricSimilar(const TAutoDBNetNode& nd, double& cfom,
  uint16_t* cindexes, bool ExtraLevel) const
{
  if (nd.AttachedNodes.Count() != AttachedNodes.Count()) {
    return false;
  }
  // have to do a full comparison, as the node order is unknown ...
  const uint16_t con =
    (uint16_t)olx_min(MaxConnectivity, AttachedNodes.Count());
  TAutoDBNetNode* nodes[MaxConnectivity];
  for (uint16_t i = 0; i < con; i++) {
    nodes[i] = AttachedNodes[i];
  }
  uint16_t indexes[MaxConnectivity];
  for (uint16_t i = 0; i < nd.AttachedNodes.Count(); i++) {
    for (uint16_t j = 0; j < con; j++) {
      if (nodes[j] == 0) {
        continue;
      }
      if (nodes[j]->FCenter->IsMetricSimilar(
        *nd.AttachedNodes[i]->FCenter, cfom))
      {
        nodes[j] = 0;
        indexes[i] = j;
        if (cindexes != 0) {
          cindexes[i] = j;
        }
        break;
      }
    }
  }
  int ndcnt = 0;
  for (uint16_t i = 0; i < con; i++) {
    if (nodes[i] != 0) {
      ndcnt++;
    }
  }

  if (!ExtraLevel) {
    if (ndcnt != 0) {
      return false;
    }
  }
  else {
    if (ndcnt != 0) {
      return false;
    }
    else {
      for (uint16_t i = 0; i < con; i++) {
        if (!nd.AttachedNodes[i]->IsMetricSimilar(
          *AttachedNodes[indexes[i]], cfom, 0, false))
        {
          return false;
        }
      }
    }
  }
  return true;
}
//..............................................................................
bool TAutoDBNetNode::IsSameType(const TAutoDBNetNode& dbn, bool ExtraLevel)
const
{
  if (!FCenter->IsSameType(*dbn.FCenter)) {
    return false;
  }
  TPtrList<TAutoDBNetNode> nodes;
  nodes.AddAll(AttachedNodes);
  TSizeList indexes;
  indexes.SetCapacity(nodes.Count());
  for (size_t i = 0; i < dbn.AttachedNodes.Count(); i++) {
    for (size_t j = 0; j < nodes.Count(); j++) {
      if (nodes[j] == 0) {
        continue;
      }
      if (nodes[j]->FCenter->IsSameType(*dbn.AttachedNodes[i]->FCenter)) {
        nodes[j] = 0;
        indexes.Add(j);
        break;
      }
    }
  }
  nodes.Pack();
  if (!ExtraLevel) {
    return nodes.IsEmpty();
  }
  else {
    if (nodes.Count() != 0) {
      return false;
    }
    for (size_t i = 0; i < dbn.AttachedNodes.Count(); i++) {
      if (!dbn.AttachedNodes[i]->IsSameType(
        *AttachedNodes[indexes[i]], false))
      {
        return false;
      }
    }
    return true;
  }
}
//..............................................................................
void TAutoDBNetNode::SaveToStream(IDataOutputStream& output) const {
  output << FCenter->GetId();
  output << (uint8_t)AttachedNodes.Count();
  for (size_t i = 0; i < AttachedNodes.Count(); i++) {
    output << AttachedNodes[i]->GetId();
  }
}
void TAutoDBNetNode::LoadFromStream(IDataInputStream& input) {
#ifdef __GNUC__  // dunno how it is implemented, but need 8 bits here
  unsigned char cnt;
#else
  uint8_t cnt;
#endif
  int32_t ind;
  input >> ind;
  FCenter = TAutoDB::GetInstance_()->Node(ind);
  input >> cnt;
  for (size_t i = 0; i < cnt; i++) {
    input >> ind;
    AttachedNodes.Add(&TAutoDBNet::GetCurrentlyLoading().Node(ind));
  }
}
//..............................................................................
olxstr TAutoDBNetNode::ToString(int level) const {
  olxstr tmp = FCenter->ToString();
  if (level == 1) {
    tmp << '{';
    for (size_t i = 0; i < AttachedNodes.Count(); i++) {
      tmp << AttachedNodes[i]->FCenter->ToString();
      if ((i + 1) < AttachedNodes.Count()) {
        tmp << ',';
      }
    }
    tmp << '}';
  }
  else if (level == 2) {
    tmp << '[';
    for (size_t i = 0; i < AttachedNodes.Count(); i++) {
      tmp << AttachedNodes[i]->ToString(1);
      if ((i + 1) < AttachedNodes.Count()) {
        tmp << ',';
      }
    }
    tmp << ']';
  }
  return tmp;
}
//..............................................................................
//..............................................................................
//..............................................................................
TAutoDBNet* TAutoDBNet::CurrentlyLoading = 0;
//..............................................................................
void TAutoDBNet::SaveToStream(IDataOutputStream& output) const {
  output << (uint32_t)FReference->id;
  output << (uint16_t)Nodes.Count();
  for (size_t i = 0; i < Nodes.Count(); i++) {
    Nodes[i].SetId((int32_t)i);
  }
  for (size_t i = 0; i < Nodes.Count(); i++) {
    Nodes[i].SaveToStream(output);
  }
}
void TAutoDBNet::LoadFromStream(IDataInputStream& input) {
  TAutoDBNet::CurrentlyLoading = this;
  uint32_t ind;
  uint16_t cnt;
  input >> ind;
  FReference = &TAutoDB::GetInstance_()->Reference(ind);
  input >> cnt;
  Nodes.SetCapacity(cnt);
  for (uint16_t i = 0; i < cnt; i++) {
    Nodes.Add(new TAutoDBNetNode(0));
  }
  for (uint16_t i = 0; i < cnt; i++) {
    Nodes[i].LoadFromStream(input);
  }
  // build index
  for (uint16_t i = 0; i < cnt; i++) {
    Nodes[i].Center()->AddParent(this, i);
  }
  TAutoDBNet::CurrentlyLoading = 0;
}
//..............................................................................
//..............................................................................
//..............................................................................
int UpdateNodeSortFunc(const TAutoDBNode* a, const TAutoDBNode* b)  {
  return a->UpdateCompare(*b);
}
//..............................................................................
int SearchCompareFunc(const TAutoDBNode &a, const TAutoDBNode &b)  {
  double v = a.SearchCompare(b);
  return v < 0 ? -1 : (v > 0 ? 1 : 0);
}
//..............................................................................
//..............................................................................

TAutoDB::TAutoDB(TXFile& xfile, ALibraryContainer& lc)
  : XFile(xfile)
{
  if (GetInstance_() != 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "duplicated object instance");
  }
  GetInstance_() = this;
  for (uint16_t i = 0; i < MaxConnectivity - 1; i++) {
    Nodes.AddNew();
  }
  BAIDelta = -1;
  URatio = 1.5;
  LengthVar = 0.03;
  AngleVar = 5.4;
  EnforceFormula = false;
}
//..............................................................................
TAutoDB::~TAutoDB() {
  for (size_t i = 0; i < Nodes.Count(); i++) {
    for (size_t j = 0; j < Nodes[i].Count(); j++) {
      delete Nodes[i][j];
    }
  }
  GetInstance_() = 0;
  delete& XFile;
}
//..............................................................................
void TAutoDB::Clear() {
  for (size_t i = 0; i < Nodes.Count(); i++) {
    for (size_t j = 0; j < Nodes[i].Count(); j++) {
      delete Nodes[i][j];
    }
  }
  registry.Clear();
  Nodes.Clear();
  Nodes.SetCapacity(MaxConnectivity);
  for (uint16_t i = 0; i < MaxConnectivity - 1; i++) {
    Nodes.AddNew();
  }
}
//..............................................................................
void TAutoDB::PrepareForSearch() {
  for (size_t i = 0; i < Nodes.Count(); i++) {
    QuickSorter::SortSF(Nodes[i], SearchCompareFunc);
  }
}
//..............................................................................
void TAutoDB::ProcessFolder(const olxstr& folder, bool allow_disorder,
  double max_r, double max_shift_over_esd, double max_GoF_dev,
  const olxstr &dest)
{
  if (!TEFile::Exists(folder)) {
    return;
  }
  olxstr uf = TEFile::TrimPathDelimeter(folder);
  TFileTree ft(uf);
  ft.Expand();
  TStrList files;
  ft.GetRoot().ListFiles(files, "*.cif");

  TOnProgress progress;
  progress.SetMax(files.Count());
  for (size_t i = 0; i < Nodes.Count(); i++) {
    Nodes[i].SetCapacity(Nodes[i].Count() + files.Count() * 100);
    Nodes[i].SetIncrement(64 * 1024);
  }
  for (size_t i = 0; i < files.Count(); i++) {
    progress.SetPos(i);
    TBasicApp::NewLogEntry() << "Processing: " << files[i];
    olxstr digest;
    try {
      TEFile f(files[i], "rb");
      digest = MD5::Digest(f);
    }
    catch (...) {
      TBasicApp::NewLogEntry(logError) << "Reading failed, skipping";
      continue;
    }
    if (registry.Contains(digest)) {
      TBasicApp::NewLogEntry(logError) << "Duplicate entry, skipping";
      continue;
    }
    try {
      XFile.LoadFromFile(files[i]);
      TCif& cif = XFile.GetLastLoader<TCif>();
      olxstr r1 = cif.GetParamAsString("_refine_ls_R_factor_gt");
      if (r1.Length() && r1.ToDouble() > max_r) {
        TBasicApp::NewLogEntry() << "Skipped: r1=" << r1;
        continue;
      }
      olxstr shift = cif.GetParamAsString("_refine_ls_shift/su_max");
      if (shift.Length() && shift.ToDouble() > max_shift_over_esd) {
        TBasicApp::NewLogEntry() << "Skipped: shift=" << shift;
        continue;
      }
      olxstr gof = cif.GetParamAsString("_refine_ls_goodness_of_fit_ref");
      if (gof.Length() && olx_abs(1 - gof.ToDouble()) > max_GoF_dev) {
        TBasicApp::NewLogEntry() << "Skipped: GoF=" << gof;
        continue;
      }
      if (!allow_disorder) {
        bool has_parts = false;
        TAsymmUnit& au = XFile.GetAsymmUnit();
        for (size_t ai = 0; ai < au.AtomCount(); ai++) {
          if (au.GetAtom(ai).GetPart() != 0) {
            has_parts = true;
            break;
          }
        }
        if (has_parts) {
          TBasicApp::NewLogEntry() << "Skipped: contains disorder";
          continue;
        }
      }
      XFile.GetLattice().CompaqAll();
      TAutoDBIdObject& adf = registry.Add(digest, files[i]);
      for (size_t j = 0; j < XFile.GetLattice().FragmentCount(); j++) {
        ProcessNodes(&adf, XFile.GetLattice().GetFragment(j));
      }
    }
    catch (const TExceptionBase& e) {
      TBasicApp::NewLogEntry(logError) << "Failed to process: " << files[i];
      TBasicApp::NewLogEntry(logExceptionTrace) << e;
    }
  }
  PrepareForSearch();
  SafeSave(dest.IsEmpty() ? src_file : dest);
}
//..............................................................................
void TAutoDB::SafeSave(const olxstr& file_name) {
  try {
    TEFile tf(file_name + ".tmp", "wb+");
    SaveToStream(tf);
    tf.Close();
    TEFile::Rename(file_name + ".tmp", file_name);
    tf.Open(file_name + ".tmp", "wb+");
    registry.SaveMap(tf);
    tf.Close();
    TEFile::Rename(file_name + ".tmp", file_name + ".map");
  }
  catch (const TExceptionBase& e) {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
}
//..............................................................................
struct TTmpNetData {
  TSAtom* Atom;
  TAutoDBNode* Node;
  TTypeList<olx_pair_t<TCAtom*, vec3d> >* neighbours;
};
void TAutoDB::ProcessNodes(TAutoDBIdObject* currentFile, TNetwork& net) {
  if (net.NodeCount() == 0) {
    return;
  }
  TTypeList< TTmpNetData* > netMatch;
  TTmpNetData* netItem;
  for (size_t i = 0; i < net.NodeCount(); i++) {
    net.Node(i).SetTag(1);
  }
  for (size_t i = 0; i < net.NodeCount(); i++) {
    if (net.Node(i).GetType().z < 2 || net.Node(i).IsDeleted()) {
      continue;
    }
    netItem = new TTmpNetData;
    netItem->neighbours = new TTypeList<olx_pair_t<TCAtom*, vec3d> >;
    netItem->Atom = &net.Node(i);
    TAutoDBNode* dbn = new TAutoDBNode(net.Node(i), netItem->neighbours);
    /* instead of MaxConnectivity we use Nodes.Count() to comply with db
    format */
    if (dbn->NodeCount() < 1 || dbn->NodeCount() > Nodes.Count()) {
      delete dbn;
      delete netItem->neighbours;
      delete netItem;
    }
    else {
      TPtrList<TAutoDBNode>& segment = Nodes[dbn->NodeCount() - 1];
      for (size_t j = 0; j < segment.Count(); j++) {
        if (segment[j]->IsSimilar(*dbn)) {
          netItem->Node = segment[j];
          delete dbn;
          dbn = 0;
          break;
        }
      }
      if (dbn != 0) {
        netItem->Node = dbn;
        Nodes[dbn->NodeCount() - 1].Add(dbn);
      }
      netMatch.AddCopy(netItem);
    }
  }
  // construct the network
  if (netMatch.Count() > 1) {
    /*this gives a one-to-one match between CAtoms and net nodes with
    CAtom->GetId(). However there is a problems since the fragments represent
    current content of the asymmetric unit and therefore some atoms might be
    attached to CAtoms of other fragments. We avoid this situation by
    considering only atoms of this fragment
    */
    net.GetLattice().GetAsymmUnit().GetAtoms().ForEach(
      ACollectionItem::TagSetter(-1));
    for (size_t i = 0; i < netMatch.Count(); i++) {
      netMatch[i]->Atom->CAtom().SetTag(i);
    }
    TAutoDBNet& net = Nets.AddNew(currentFile);
    // precreate nodes
    for (uint32_t i = 0; i < netMatch.Count(); i++) {
      net.NewNode(netMatch[i]->Node);
      netMatch[i]->Node->AddParent(&net, i); // build index
    }
    // build connectivity
    for (size_t i = 0; i < netMatch.Count(); i++) {
      for (size_t j = 0; j < netMatch[i]->neighbours->Count(); j++) {
        if (netMatch[i]->neighbours->GetItem(j).GetA()->GetTag() < 0) {
          continue;
        }
        net.Node(i).AttachNode(
          &net.Node(netMatch[i]->neighbours->GetItem(j).GetA()->GetTag()));
      }
    }
    for (size_t i = 0; i < netMatch.Count(); i++) {
      delete netMatch[i]->neighbours;
      delete netMatch[i];
    }
  }
  //  for( size_t i=0; i < Nodes.Count(); i++ )
  //    Nodes[i]->Average();

  return;
}
//..............................................................................
TAutoDBNet* TAutoDB::BuildSearchNet(TNetwork& net, TSAtomPList& cas) {
  TStopWatch sw(__FUNC__);
  if (net.NodeCount() == 0) {
    return 0;
  }
  TPtrList<TTmpNetData> netMatch;
  for (size_t i = 0; i < net.NodeCount(); i++) {
    net.Node(i).SetTag(1);
  }
  for (size_t i = 0; i < net.NodeCount(); i++) {
    //    if( net.Node(i).GetType() != iQPeakZ &&
    if (net.Node(i).GetType() != iHydrogenZ) {
      TTmpNetData* netItem = new TTmpNetData;
      netItem->neighbours = new TTypeList<olx_pair_t<TCAtom*, vec3d> >;
      netItem->Atom = &net.Node(i);
      TAutoDBNode* dbn = new TAutoDBNode(net.Node(i), netItem->neighbours);
      if (dbn->NodeCount() < 1 || dbn->NodeCount() > 12) {
        delete dbn;
        delete netItem->neighbours;
        delete netItem;
      }
      else {
        netItem->Node = dbn;
        netMatch.Add(netItem);
      }
    }
  }
  // construct the network
  if (netMatch.Count() > 0) {
    for (size_t i = 0; i < net.GetLattice().GetAsymmUnit().AtomCount(); i++)
      net.GetLattice().GetAsymmUnit().GetAtom(i).SetTag(-1);
    TAutoDBNet* dbnet = new TAutoDBNet(0);
    for (size_t i = 0; i < netMatch.Count(); i++) {
      dbnet->NewNode(netMatch[i]->Node);
      cas.Add(netMatch[i]->Atom);
      netMatch[i]->Atom->CAtom().SetTag(i);
    }
    for (size_t i = 0; i < netMatch.Count(); i++) {
      for (size_t j = 0; j < netMatch[i]->neighbours->Count(); j++) {
        if (netMatch[i]->neighbours->GetItem(j).GetA()->GetTag() < 0) {
          continue;
        }
        dbnet->Node(i).AttachNode(
          &dbnet->Node(netMatch[i]->neighbours->GetItem(j).GetA()->GetTag()));
      }
    }
    for (size_t i = 0; i < netMatch.Count(); i++) {
      delete netMatch[i]->neighbours;
      delete netMatch[i];
    }
    return dbnet;
  }
  return 0;
}
//..............................................................................
void TAutoDB::SaveToStream(IDataOutputStream& output) const {
  output.Write(FileSignature, FileSignatureLength);
  output << FileVersion;
  output << (uint16_t)0;  // file flags - flat for now
  registry.AssignIds();
  registry.SaveToStream(output);
  uint32_t nodeCount = 0;
  output << (uint32_t)Nodes.Count();
  for (uint32_t i = 0; i < Nodes.Count(); i++) {
    output << (uint32_t)Nodes[i].Count();
    for (uint32_t j = 0; j < Nodes[i].Count(); j++) {
      Nodes[i][j]->SetId(nodeCount + j);
      Nodes[i][j]->SaveToStream(output);
    }
    nodeCount += (uint32_t)Nodes[i].Count();
  }

  output << (uint32_t)Nets.Count();
  for (uint32_t i = 0; i < Nets.Count(); i++) {
    Nets[i].SaveToStream(output);
  }
}
//..............................................................................
void TAutoDB::LoadFromStream(IDataInputStream& input) {
  TStopWatch sw(__FUNC__);
  // validation of the file
  char fileSignature[FileSignatureLength + 1];
  input.Read(fileSignature, FileSignatureLength);
  fileSignature[FileSignatureLength] = '\0';
  if (olxstr(fileSignature) != FileSignature) {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file signature");
  }
  uint16_t fileVersion;
  input >> fileVersion;
  if (fileVersion != FileVersion) {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file version");
  }
  // read file flags
  input >> fileVersion;
  // end of the file validation
  registry.LoadFromStream(input);
  uint32_t ind, listCount, nodeCount = 0;
  input >> listCount;  // nt MaxConnectivity is overriden!
  Nodes.SetCapacity(Nodes.Count() + listCount);
  for (uint32_t i = 0; i < listCount; i++) {
    Nodes.AddNew();
    input >> ind;
    Nodes[i].SetCapacity(ind);
    for (uint32_t j = 0; j < ind; j++) {
      Nodes[i].Add(new TAutoDBNode(input));
      Nodes[i][j]->SetId(nodeCount + j);
    }
    nodeCount += ind;
  }

  input >> ind;
  Nets.SetCapacity(ind);
  for (uint32_t i = 0; i < ind; i++) {
    Nets.Add(new TAutoDBNet(input));
  }
  PrepareForSearch();
  TBasicApp::NewLogEntry(logInfo) << "Loaded " << Nets.Count() << " graphs with"
    " max connectivity of " << Nodes.Count();
}
//..............................................................................
void TAutoDB::AnalyseNode(TSAtom& sa, TStrList& report) {
  TSAtomPList cas;
  TAutoDBNet* sn = BuildSearchNet(sa.GetNetwork(), cas);
  if (sn == 0) {
    return;
  }
  olxstr tmp;
  size_t index = InvalidIndex;
  for (size_t i = 0; i < cas.Count(); i++) {
    if (cas[i] == &sa) {
      index = i;
      break;
    }
  }
  if (index == InvalidIndex) {
    return;
  }
  TAutoDBNetNode& node = sn->Node(index);
  if (node.Count() < 1 || node.Count() > Nodes.Count()) {
    return;
  }
  TPtrList< TAutoDBNode >& segment = Nodes[node.Count() - 1];
  TTypeList< olx_pair_t<TAutoDBNode*, int> > S1Match;
  TTypeList< AnAssociation3<TAutoDBNetNode*, int, TAutoDBIdPList*> > S2Match,
    S3Match;
  for (size_t i = 0; i < segment.Count(); i++) {
    double fom = 0;
    if (segment[i]->IsMetricSimilar(*node.Center(), fom)) {
      //
      bool found = false;
      for (size_t j = 0; j < S1Match.Count(); j++) {
        if (S1Match[j].GetA()->IsSameType(*segment[i])) {
          S1Match[j].b++;
          found = true;
          break;
        }
      }
      if (!found) {
        S1Match.AddNew<TAutoDBNode*, int>(segment[i], 1);
      }
      //
      for (size_t j = 0; j < segment[i]->ParentCount(); j++) {
        double cfom = 0;
        TAutoDBNetNode& netnd = segment[i]->GetParent(j)->Node(
          segment[i]->GetParentIndex(j));
        //TBasicApp::NewLogEntry(logVerbose) << Nodes[i]->GetParent(j)->Reference()->GetName();
        if (netnd.IsMetricSimilar(node, cfom, 0, false)) {
          //
          found = false;
          for (size_t k = 0; k < S2Match.Count(); k++) {
            if (S2Match[k].GetA()->IsSameType(netnd, false)) {
              S2Match[k].b++;
              S2Match[k].c->Add(segment[i]->GetParent(j)->Reference());
              found = true;
              break;
            }
          }
          if (!found) {
            S2Match.AddNew<TAutoDBNetNode*, int, TAutoDBIdPList*>(
              &netnd, 1, new TAutoDBIdPList);
            S2Match[S2Match.Count() - 1].c->Add(
              segment[i]->GetParent(j)->Reference());
          }
          //
          if (netnd.IsMetricSimilar(node, cfom, 0, true)) {
            //
            found = false;
            for (size_t k = 0; k < S3Match.Count(); k++) {
              if (S3Match[k].GetA()->IsSameType(netnd, false)) {
                S3Match[k].b++;
                S3Match[k].c->Add(segment[i]->GetParent(j)->Reference());
                found = true;
                break;
              }
            }
            if (!found) {
              S3Match.AddNew<TAutoDBNetNode*, int, TAutoDBIdPList*>(
                &netnd, 1, new TAutoDBIdPList);
              S3Match[S3Match.Count() - 1].c->Add(
                segment[i]->GetParent(j)->Reference());
            }
            //
          }
        }
      }
    }
  }
  if (!S1Match.IsEmpty()) {
    report.Add("S1 matches:");
    for (size_t i = 0; i < S1Match.Count(); i++) {
      report.Add(olxstr("   ") << S1Match[i].GetA()->ToString() <<
        ' ' << S1Match[i].GetB() << " hits");
    }
    if (!S2Match.IsEmpty()) {
      report.Add("S2 matches:");
      for (size_t i = 0; i < S2Match.Count(); i++) {
        report.Add(olxstr("   ") << S2Match[i].GetA()->ToString(1) <<
          ' ' << S2Match[i].GetB() << " hits");
        olxstr tmp("Refs [");
        for (size_t j = 0; j < S2Match[i].GetC()->Count(); j++) {
          tmp << S2Match[i].GetC()->GetItem(j)->reference << ';';
        }
        report.Add(tmp << ']');
        delete S2Match[i].GetC();
      }
      if (!S3Match.IsEmpty()) {
        report.Add("S3 matches:");
        for (size_t i = 0; i < S3Match.Count(); i++) {
          report.Add(olxstr("   ") << S3Match[i].GetA()->ToString(2) <<
            ' ' << S3Match[i].GetB() << " hits");
          olxstr tmp("Refs [");
          for (size_t j = 0; j < S3Match[i].GetC()->Count(); j++) {
            tmp << S3Match[i].GetC()->GetItem(j)->reference << ';';
          }
          report.Add(tmp << ']');
          delete S3Match[i].GetC();
        }
      }
    }
  }

  for (size_t i = 0; i < sn->Count(); i++) {
    delete sn->Node(i).Center();
  }
  delete sn;
  return;
}
//..............................................................................
void TAutoDB::AnalyseStructure(const olxstr& lastFileName, TLattice& latt,
  TAtomTypePermutator* permutator, TAutoDB::AnalysisStat& stat, bool dry_run,
  ElementPList* proposed_atoms)
{
  LastFileName = lastFileName;
  stat.Clear();
  stat.FormulaConstrained = (proposed_atoms != 0);
  stat.AtomDeltaConstrained = (BAIDelta != -1);
  Uisos.Clear();
  for (size_t i = 0; i < latt.FragmentCount(); i++) {
    AnalyseNet(latt.GetFragment(i), permutator,
      Uisos.Add(0), stat, dry_run, proposed_atoms);
  }
  LastStat = stat;
}
//..............................................................................
int SortGuessListByCount(
  const AnAssociation3<double, const cm_Element*, int>& a,
  const AnAssociation3<double, const cm_Element*, int>& b)
{
  return a.GetC() - b.GetC();
}
//..............................................................................
size_t TAutoDB::TAnalyseNetNodeTask::LocateDBNodeIndex(
  const TPtrList<TAutoDBNode>& segment, TAutoDBNode* nd,
  size_t from, size_t to)
{
  if (segment.IsEmpty()) {
    return InvalidIndex;
  }
  if (from == InvalidIndex) {
    from = 0;
  }
  if (to == InvalidIndex) {
    to = segment.Count() - 1;
  }
  if (to == from) {
    return to;
  }
  if ((to - from) == 1) {
    return from;
  }
  int resfrom = SearchCompareFunc(*segment[from], *nd),
    resto = SearchCompareFunc(*segment[to], *nd);
  if (resfrom == 0) {
    return from;
  }
  if (resto == 0) {
    return to;
  }
  if (resfrom < 0 && resto > 0) {
    size_t index = (to + from) / 2;
    int res = SearchCompareFunc(*segment[index], *nd);
    if (res < 0) {
      return LocateDBNodeIndex(segment, nd, index, to);
    }
    if (res > 0) {
      return LocateDBNodeIndex(segment, nd, from, index);
    }
    if (res == 0) {
      return index;
    }
  }
  return InvalidIndex;
}
//..............................................................................
void TAutoDB::TAnalyseNetNodeTask::Run(size_t index) {
  const TAutoDBNetNode& nd = Network.Node(index);
  if (nd.Count() < 1 || nd.Count() > Nodes.Count()) {
    return;
  }
  const TPtrList< TAutoDBNode >& segment = Nodes[nd.Count() - 1];
  //long ndind = LocateDBNodeIndex(segment, nd.Center());
  //if( ndind == -1 )  return;
  //long position = ndind, inc = 1;
  TGuessCount& gc = Guesses[index];
  bool found;
  double fom, cfom; //, var;
  for (size_t i = 0; i < segment.Count(); i++) {
    //    if( position >= segment.Count() )  {
    //      inc = -1;
    //      position = ndind - 1;
    //    }
    //    if( position < 0 )  return;
    fom = 0;
    TAutoDBNode& segnd = *segment[i];
    //    TAutoDBNode& segnd = *segment[position];
    //    var = nd.Center()->SearchCompare( segnd, &fom );
    //    if( var == 0 )  {
    if (nd.Center()->IsMetricSimilar(segnd, fom)) {
      for (size_t j = 0; j < segnd.ParentCount(); j++) {
        TAutoDBNetNode& netnd = segnd.GetParent(j)->Node(
          segnd.GetParentIndex(j));
        cfom = 0;
        if (nd.IsMetricSimilar(netnd, cfom, 0, false)) {
          found = false;
          for (size_t k = 0; k < gc.list2.Count(); k++) {
            if (*gc.list2[k].Type == netnd.Center()->GetType()) {
              gc.list2[k].hits.AddNew(&netnd, cfom);
              found = true;
              break;
            }
          }
          if (!found) {
            gc.list2.AddNew(netnd.Center()->GetType(), &netnd, cfom);
          }
          if (nd.IsMetricSimilar(netnd, cfom, 0, true)) {
            found = false;
            for (size_t k = 0; k < gc.list3.Count(); k++) {
              if (*gc.list3[k].Type == netnd.Center()->GetType()) {
                gc.list3[k].hits.AddNew(&netnd, cfom);
                found = true;
                break;
              }
            }
            if (!found) {
              gc.list3.AddNew(netnd.Center()->GetType(), &netnd, cfom);
            }
          }
        }
      }
      found = false;
      for (size_t j = 0; j < gc.list1.Count(); j++) {
        if (*gc.list1[j].Type == segnd.GetType()) {
          gc.list1[j].hits.AddNew(&segnd, fom);
          found = true;
          break;
        }
      }
      if (!found) {
        gc.list1.AddNew(segnd.GetType(), &segnd, fom);
      }
    }
    //    if( var == 0 || olx_abs(var) <= LengthVar*4 )  {
    //      position += inc;
    //    }
    //    else  {  // no match
    //      if( inc == 1 )  {  // start in opposit direction
    //        inc = -1;
    //        position = ndind - 1;
    //      }
    //      else   // finsished the opposit direction too
    //        return;
    //    }
  }
}
//..............................................................................
bool TAutoDB::A2Pemutate(TCAtom& a1, TCAtom& a2, const cm_Element& e1,
  const cm_Element& e2, bool dry_run, double threshold)
{
  short v = CheckA2Pemutate(a1, a2, e1, e2, threshold);
  if ((v & 9) != 0) {
    if (!a1.IsFixedType()) {
      if (!dry_run) {
        TBasicApp::NewLogEntry(logVerbose) << "Skipping fixed type atom '" <<
          a1.GetLabel() << '\'';
      }
      return false;
    }
  }
  if ((v & 6) != 0) {
    if (!a1.IsFixedType()) {
      if (!dry_run) {
        TBasicApp::NewLogEntry(logVerbose) << "Skipping fixed type atom '" <<
          a2.GetLabel() << '\'';
      }
      return false;
    }
  }
  bool changes = false;
  if ((v & 1) != 0) {
    if (!dry_run) {
      TBasicApp::NewLogEntry(logVerbose) << "A2 assignment: " <<
        a1.GetLabel() << " -> " << e1.symbol;
      a1.SetType(e1);
    }
    changes = true;
  }
  if ((v & 2) != 0) {
    if (!dry_run) {
      TBasicApp::NewLogEntry(logVerbose) << "A2 assignment: " <<
        a2.GetLabel() << " -> " << e2.symbol;
      a2.SetType(e2);
    }
    changes = true;
  }
  if ((v & 4) != 0) {
    if (!dry_run) {
      TBasicApp::NewLogEntry(logVerbose) << "A2 assignment: " <<
        a2.GetLabel() << " -> " << e1.symbol;
      a2.SetType(e1);
    }
    changes = true;
  }
  if ((v & 8) != 0) {
    if (!dry_run) {
      TBasicApp::NewLogEntry(logVerbose) << "A2 assignment: " <<
        a1.GetLabel() << " -> " << e2.symbol;
      a1.SetType(e2);
    }
    changes = true;
  }
  return changes;
}
//..............................................................................
short TAutoDB::CheckA2Pemutate(TCAtom& a1, TCAtom& a2, const cm_Element& e1,
  const cm_Element& e2, double threshold)
{
  const double ratio = a1.GetUiso() / (olx_abs(a2.GetUiso()) + 0.001);
  short res = 0;
  if (ratio > (1.0 + threshold)) {
    if (a1.GetType() != e1) {
      res |= 1;
    }
    if (a2.GetType() != e2) {
      res |= 2;
    }
  }
  else if (ratio < (1.0 - threshold)) {
    if (a2.GetType() != e1) {
      res |= 4;
    }
    if (a1.GetType() != e2) {
      res |= 8;
    }
  }
  return res;
}
//..............................................................................
ConstTypeList<TAutoDB::TAnalysisResult> TAutoDB::AnalyseStructure(TLattice& latt)
{
  TTypeList<TAutoDB::TAnalysisResult> res;
  for (size_t i = 0; i < latt.FragmentCount(); i++) {
    res.AddAll(AnalyseNet(latt.GetFragment(i)));
  }
  return res;
}
//..............................................................................
ConstTypeList<TAutoDB::TAnalysisResult> TAutoDB::AnalyseNet(TNetwork& net) {
  TStopWatch sw(__FUNC__);
  TSAtomPList cas;
  TTypeList<TAutoDB::TAnalysisResult> res;
  TTypeList<TAutoDB::TGuessCount> guesses;
  TAutoDBNet* sn = BuildSearchNet(net, cas);
  for (size_t i = 0; i < net.NodeCount(); i++) {
    net.Node(i).SetTag(-1);
  }
  if (sn == 0) {
    return res;
  }
  const size_t sn_count = sn->Count();
  // for two atoms we cannot decide which one is which, for one - no reason at all :)
  res.SetCapacity(sn_count);
  for (size_t i = 0; i < sn_count; i++) {
    sn->Node(i).SetTag(-1);
    sn->Node(i).SetId(0);
    res.AddNew().atom = &cas[i]->CAtom();
    TGuessCount& gc = guesses.AddNew();
    gc.list1.SetCapacity(12);
    gc.list2.SetCapacity(6);
    gc.list3.SetCapacity(3);
    gc.atom = &cas[i]->CAtom();
  }
  if (sn_count < 3) {
    if (sn_count == 2) { // C-O or C-N?
      if (sn->Node(0).Center()->GetDistance(0) < 1.3) { // C-N
        short mv = CheckA2Pemutate(cas[0]->CAtom(), cas[1]->CAtom(),
          XElementLib::GetByIndex(iCarbonIndex),
          XElementLib::GetByIndex(iNitrogenIndex), 0.2);
        if ((mv & 1) != 0) {
          res[0].enforced.AddNew(1,
            XElementLib::GetByIndex(iCarbonIndex));
        }
        if ((mv & 2) != 0) {
          res[1].enforced.AddNew(1,
            XElementLib::GetByIndex(iNitrogenIndex));
        }
      }
      else { // C-O
        short mv = CheckA2Pemutate(cas[0]->CAtom(), cas[1]->CAtom(),
          XElementLib::GetByIndex(iCarbonIndex),
          XElementLib::GetByIndex(iOxygenIndex), 0.2);
        if ((mv & 1) != 0) {
          res[0].enforced.AddNew(1,
            XElementLib::GetByIndex(iCarbonIndex));
        }
        if ((mv & 2) != 0) {
          res[1].enforced.AddNew(1,
            XElementLib::GetByIndex(iOxygenIndex));
        }
      }
    }
    for (size_t i = 0; i < sn_count; i++) {
      delete sn->Node(i).Center();
    }
    delete sn;
    return res;
  }
  sw.start("Creating analysis task");
  TAnalyseNetNodeTask analyseNetNodeTask(Nodes, *sn, guesses);
  OlxListTask::Run(analyseNetNodeTask, sn_count, tLinearTask, 0);
  sw.stop();
  uint16_t cindexes[MaxConnectivity];
  TArrayList<short> node_lists(sn_count);
  for (size_t i = 0; i < sn_count; i++) {
    if (!guesses[i].list3.IsEmpty()) {
      node_lists[i] = 2;
    }
    else if (!guesses[i].list2.IsEmpty()) {
      node_lists[i] = 1;
    }
    else {
      node_lists[i] = 0;
    }
    sn->Node(i).SetId((uint32_t)i);
  }
  // analysis of "confident", L3 and L2 atom types and Uiso
  for (size_t i = 0; i < sn_count; i++) {
    // copy the results
    for (size_t j = 0; j < guesses[i].list1.Count(); j++) {
      res[i].list1.AddNew(guesses[i].list1[j].MeanFom(), *guesses[i].list1[j].Type);
    }
    for (size_t j = 0; j < guesses[i].list2.Count(); j++) {
      res[i].list2.AddNew(guesses[i].list2[j].MeanFom(), *guesses[i].list2[j].Type);
    }
    for (size_t j = 0; j < guesses[i].list3.Count(); j++) {
      res[i].list3.AddNew(guesses[i].list3[j].MeanFom(), *guesses[i].list3[j].Type);
    }

    if (node_lists[i] == 0) {
      continue;
    }
    TTypeList< THitList<TAutoDBNetNode> >& guessN =
      !guesses[i].list3.IsEmpty() ? guesses[i].list3 : guesses[i].list2;
    for (size_t j = 0; j < guessN.Count(); j++) {
      guessN.GetItem(j).Sort();
    }
    QuickSorter::SortSF(guessN, THitList<TAutoDBNetNode>::SortByFOMFunc);
    double cfom = 0;
    // just for sorting
    sn->Node(i).IsMetricSimilar(
      *guessN[0].hits[0].Node, cfom, cindexes, false);
    for (size_t j = 0; j < sn->Node(i).Count(); j++) {
      if (sn->Node(i).Node(j)->GetTag() == -1 && node_lists[i] == 0) {
        const cm_Element& t = guessN[0].hits[0].Node->Node(
          cindexes[j])->Center()->GetType();
        res[sn->Node(i).Node(j)->GetId()].enforced.AddNew(
          guessN[0].hits[0].Fom, t);
        sn->Node(i).Node(j)->SetTag(t.GetIndex());
      }
      else {
        const cm_Element& to = guessN[0].hits[0].Node->Node(
          cindexes[j])->Center()->GetType();
        TTypeList<Type>& en =
          res[sn->Node(i).Node(j)->GetId()].enforced;
        bool uniq = true;
        for (size_t k = 0; k < en.Count(); k++) {
          if (en[k].type == to) {
            uniq = false;
            en[k].fom /= 2; // VERY simple promotion
            break;
          }
        }
        if (uniq) {
          en.AddNew(guessN[0].hits[0].Fom, to);
        }
      }
    }
  }
  for (size_t i = 0; i < sn_count; i++) {
    delete sn->Node(i).Center();
  }
  delete sn;
  return res;
}
//..............................................................................
bool TAutoDB::ChangeType(TCAtom &a, const cm_Element &e, bool dry_run) {
  if (a.GetType() == e || e == iHydrogenZ) {
    return false;
  }
  if (a.IsFixedType()) {
    if (!dry_run) {
      TBasicApp::NewLogEntry(logVerbose) << "Skipping fixed type atom '" <<
        a.GetLabel() << '\'';
    }
    return false;
  }
  bool return_any = !alg::check_connectivity(a, a.GetType());
  if (return_any || alg::check_connectivity(a, e)) {
    // extra checks for high jumpers
    if (olx_abs(a.GetType().z-e.z) > 3 ) {
      if (!alg::check_geometry(a, e)) {
        return false;
      }
    }
    if (!dry_run) {
      a.SetType(e);
      a.SetLabel(e.symbol, false);
    }
    return true;
  }
  return false;
}
//..............................................................................
void TAutoDB::AnalyseNet(TNetwork& net, TAtomTypePermutator* permutator,
  double& Uiso, TAutoDB::AnalysisStat& stat, bool dry_run,
  ElementPList* proposed_atoms)
{
  TStopWatch sw(__FUNC__);
  TSAtomPList cas;
  TAutoDBNet* sn = BuildSearchNet(net, cas);
  TStrList log;
  for (size_t i = 0; i < net.NodeCount(); i++) {
    net.Node(i).SetTag(-1);
  }
  if (sn == 0) {
    return;
  }
  const size_t sn_count = sn->Count();
  // for two atoms we cannot decide which one is which, for one - no reason at all :)
  if (sn_count < 3) {
    if (sn_count == 2 && // C-O or C-N?
        cas[0]->CAtom().AttachedSiteCount() == 1 &&
        cas[1]->CAtom().AttachedSiteCount() == 1)
    {
      if (sn->Node(0).Center()->GetDistance(0) < 1.3) { // C-N
        if (proposed_atoms == 0 ||
            (proposed_atoms->Contains(XElementLib::GetByIndex(iCarbonIndex)) &&
             proposed_atoms->Contains(XElementLib::GetByIndex(iNitrogenIndex))))
        {
          if (A2Pemutate(cas[0]->CAtom(), cas[1]->CAtom(),
            XElementLib::GetByIndex(iCarbonIndex),
            XElementLib::GetByIndex(iNitrogenIndex), dry_run, 0.2))
          {
            stat.AtomTypeChanges++;
          }
        }
      }
      else { // C-O
        if (proposed_atoms == 0 ||
            (proposed_atoms->Contains(XElementLib::GetByIndex(iCarbonIndex)) &&
             proposed_atoms->Contains(XElementLib::GetByIndex(iOxygenIndex))))
        {
          if (A2Pemutate(cas[0]->CAtom(), cas[1]->CAtom(),
            XElementLib::GetByIndex(iCarbonIndex),
            XElementLib::GetByIndex(iOxygenIndex), dry_run, 0.2))
          {
            stat.AtomTypeChanges++;
          }
        }
      }
    }
    for (size_t i = 0; i < sn_count; i++) {
      delete sn->Node(i).Center();
    }
    delete sn;
    return;
  }

  TTypeList< TGuessCount > guesses;
  guesses.SetCapacity(sn_count);
  for( size_t i=0; i < sn_count; i++ )  {
    sn->Node(i).SetTag(-1);
    sn->Node(i).SetId(0);
    TGuessCount& gc = guesses.AddNew();
    gc.list1.SetCapacity(12);
    gc.list2.SetCapacity(6);
    gc.list3.SetCapacity(3);
    gc.atom = &cas[i]->CAtom();
  }
  
  sw.start("Start node analysis task");
  TAnalyseNetNodeTask analyseNetNodeTask(Nodes, *sn, guesses);
  OlxListTask::Run(analyseNetNodeTask, sn_count, tLinearTask, 0);
  sw.start("Post analysis");
  uint16_t cindexes[MaxConnectivity];
  uint16_t UisoCnt = 0;
  for (size_t i=0; i < sn_count; i++) {
    if (!guesses[i].list3.IsEmpty()) {
      sn->Node(i).SetId(2);
    }
    else if (!guesses[i].list2.IsEmpty()) {
      sn->Node(i).SetId(1);
    }
    else {
      sn->Node(i).SetId(0);
    }
  }
  sw.start("Analysing confident atom types");
  // analysis of "confident", L3 and L2 atom types and Uiso
  for (size_t i=0; i < sn_count; i++) {
    if (sn->Node(i).GetId() == 0) {
      continue;
    }
    TTypeList< THitList<TAutoDBNetNode> > &guessN =
      !guesses[i].list3.IsEmpty() ? guesses[i].list3 : guesses[i].list2;
    for (size_t j = 0; j < guessN.Count(); j++) {
      guessN.GetItem(j).Sort();
    }
    QuickSorter::SortSF(guessN, THitList<TAutoDBNetNode>::SortByFOMFunc);
    double cfom = 0;
    // just for sorting
    sn->Node(i).IsMetricSimilar(*guessN[0].hits[0].Node, cfom, cindexes,
      false);
    for (size_t j=0; j < sn->Node(i).Count(); j++) {
      if (sn->Node(i).Node(j)->GetTag() == -1 &&
          sn->Node(i).Node(j)->GetId() == 0)
      {
        sn->Node(i).Node(j)->SetTag(guessN[0].hits[0].Node->Node(
          cindexes[j])->Center()->GetType().GetIndex());
      }
      else {
        index_t from = sn->Node(i).Node(j)->GetTag();
        index_t to = guessN[0].hits[0].Node->Node(
          cindexes[j])->Center()->GetType().GetIndex();
        if (from != -1 && from != to) {
          log.Add("Oups ...");
        }
      }
    }
    if (sn->Node(i).Count() == 1) { // normally wobly
      Uiso += guesses[i].atom->GetUiso() * 3. / 4.;
    }
    else {
      Uiso += guesses[i].atom->GetUiso();
    }
    UisoCnt ++;
    stat.ConfidentAtomTypes++;
  }
  if (UisoCnt != 0) {
    Uiso /= UisoCnt;
  }
  if( Uiso < 0.015 || Uiso > 0.075 )  {  // override silly values if happens
    Uiso = 0;
    UisoCnt = 0;
  }
  if( UisoCnt != 0 )  {
    log.Add("Mean Uiso for confident atom types is ") <<
      olxstr::FormatFloat(3,Uiso);
  }
  else {
    log.Add("Could not locate confident atom types");
  }
  // assigning atom types according to L3 and L2 and printing stats
  sw.start("Assigning confident atom types");
  sorted::PointerPointer<TAutoDBNetNode> processed;
  for (size_t i=0; i < sn_count; i++) {
    if (sn->Node(i).GetId() == 0) {
      if (UisoCnt==0 && proposed_atoms==0 && !guesses[i].list1.IsEmpty()) {
        if (alg::check_connectivity(*guesses[i].atom,
            *guesses[i].list1[0].Type))
        {
          sn->Node(i).SetTag(guesses[i].list1[0].Type->GetIndex());
        }
        continue;
      }
    }
    TTypeList<THitList<TAutoDBNetNode> > &guessN =
      !guesses[i].list3.IsEmpty() ? guesses[i].list3 : guesses[i].list2;
    const cm_Element* type = 0;
    if (sn->Node(i).GetId() != 0) {
      type = guessN[0].Type;
      sn->Node(i).SetTag(type->GetIndex());
      for (size_t j=0; j < guessN.Count(); j++) {
        log.Add(guessN[j].Type->symbol) << '(' <<
          olxstr::FormatFloat(2,1.0/(guessN[j].MeanFomN(1)+0.001)) << ")";
        if ((j + 1) < guessN.Count()) {
          log.GetLastString() << ',';
        }
      }
    }
    if (permutator == 0 || !permutator->IsActive()) {
      // have to do it here too!
      bool searchHeavier = false, searchLighter = false;
      if( UisoCnt != 0 && Uiso != 0 )  {
        double uiso = sn->Node(i).Count() == 1
          ? guesses[i].atom->GetUiso()*3./4. : guesses[i].atom->GetUiso();
        double scale = uiso / Uiso;
        if (scale > URatio) {
          searchLighter = true;
        }
        else if (scale < 1. / URatio) {
          searchHeavier = true;
        }
      }
      // use max.min 'absolute' values for elements after Si
      else if (guesses[i].atom->GetType().z > 14) {
        if (guesses[i].atom->GetUiso() > 0.05) {
          searchLighter = true;
        }
        else if (guesses[i].atom->GetUiso() < 0.005) {
          searchHeavier = true;
        }
      }
      if (searchLighter || searchHeavier) {
        if (AnalyseUiso(*guesses[i].atom, guessN, stat, dry_run, searchHeavier,
          searchLighter, proposed_atoms))
        {
          sn->Node(i).SetTag(guesses[i].atom->GetType().GetIndex());
          processed.AddUnique(&sn->Node(i));
        }
      }
      else {
        if (type != 0 && *type != guesses[i].atom->GetType()) {
          if (proposed_atoms != 0) {
            if (proposed_atoms->IndexOf(type) != InvalidIndex) {
              if (ChangeType(*guesses[i].atom, *type, dry_run)) {
                stat.AtomTypeChanges++;
              }
            }
          }
          else if (BAIDelta != -1) {
            if (abs(type->z - guesses[i].atom->GetType().z) < BAIDelta) {
              if (ChangeType(*guesses[i].atom, *type, dry_run)) {
                stat.AtomTypeChanges++;
              }
            }
          }
          else {
            if (ChangeType(*guesses[i].atom, *type, dry_run)) {
              stat.AtomTypeChanges++;
            }
          }
        }
      }
    }
  }
  sw.start("Analysing the rest");
  for( size_t i=0; i < sn_count; i++ )  {
    if (sn->Node(i).GetTag() != -1 &&
        guesses[i].atom->GetType() != sn->Node(i).GetTag() &&
        !processed.Contains(&sn->Node(i)))
    {
      int change_evt = -1;
      cm_Element* l_elm = &XElementLib::GetByIndex(sn->Node(i).GetTag());
      // change to only provided atoms if in the guess list
      if( proposed_atoms != 0 )  {
        if( proposed_atoms->IndexOf(l_elm) != InvalidIndex )  {
          double delta_z = guesses[i].atom->GetType().z - l_elm->z;
          double ref_val = guesses[i].atom->GetUiso() - 0.0025*delta_z;
          if(  ref_val > 0.01 && ref_val < 0.075)  {
            if (ChangeType(*guesses[i].atom, *l_elm, dry_run)) {
              change_evt = 0;
            }
          }
        }
      }
      else  if( BAIDelta != -1 )  { // consider atom types within BAIDelta only
        if( olx_abs(guesses[i].atom->GetType().z-sn->Node(i).GetTag()) <
            BAIDelta )
        {
          if (ChangeType(*guesses[i].atom, *l_elm, dry_run)) {
            change_evt = 1;
          }
        }
      }
      else  {  // unrestrained assignment
        if (ChangeType(*guesses[i].atom, *l_elm, dry_run)) {
          change_evt = 2;
        }
      }
      if( change_evt != -1 )  {
        log.Add("SN[") << change_evt << "] assignment " <<
          guesses[i].atom->GetLabel() << " to " << l_elm->symbol;
        stat.AtomTypeChanges++;
        stat.SNAtomTypeAssignments++;
      }
    }
  }
  sw.start("Analysing promotions");
  for (size_t i = 0; i < sn_count; i++)  {
    if (sn->Node(i).GetTag() == -1) {
      bool searchHeavier = false, searchLighter = false;
      if (UisoCnt != 0 && Uiso != 0) {
        double scale = guesses[i].atom->GetUiso() / Uiso;
        if (scale > URatio) {
          searchLighter = true;
        }
        else if (scale < 1. / URatio) {
          searchHeavier = true;
        }
      }
      //else if( Uiso == 0 )
      //  searchLighter = true;
      if (permutator != 0 && permutator->IsActive()) {
        permutator->InitAtom(guesses[i]);
      }
      for (size_t j = 0; j < guesses[i].list1.Count(); j++) {
        guesses[i].list1[j].Sort();
      }
      QuickSorter::SortSF(guesses[i].list1,
        THitList<TAutoDBNode>::SortByFOMFunc);
      olxstr tmp;
      if (!guesses[i].list1.IsEmpty()) {
        tmp << guesses[i].atom->GetLabel() << ' ';
        const cm_Element* type = &guesses[i].atom->GetType();
        if( searchHeavier )  {
          log.Add("Searching element heavier for ") <<
            guesses[i].atom->GetLabel();
          for (size_t j=0; j < guesses[i].list1.Count(); j++) {
            if (guesses[i].list1[j].Type->z > type->z) {
              if (proposed_atoms != 0) {
                if (proposed_atoms->Contains(guesses[i].list1[j].Type)) {
                  type = guesses[i].list1[j].Type;
                  break;
                }
              }
              else if (BAIDelta != -1) {
                if( guesses[i].list1[j].Type->z-guesses[i].atom->GetType().z <
                    BAIDelta )
                {
                  type = guesses[i].list1[j].Type;
                  break;
                }
              }
            }
          }
        }
        else if (searchLighter) {
          log.Add("Searching element lighter for ") <<
            guesses[i].atom->GetLabel();
          for (size_t j=0; j < guesses[i].list1.Count(); j++) {
            if (guesses[i].list1[j].Type->z < type->z) {
              if (proposed_atoms != 0) {
                if (proposed_atoms->Contains(guesses[i].list1[j].Type)) {
                  type = guesses[i].list1[j].Type;
                  break;
                }
              }
              else if (BAIDelta != -1) {
                if (guesses[i].atom->GetType().z-guesses[i].list1[j].Type->z <
                    BAIDelta)
                {
                  type = guesses[i].list1[j].Type;
                  break;
                }
              }
            }
          }
        }
        else {
          type = 0;  //guesses[i].list1->Item(0).BAI;
        }
        for (size_t j=0; j < guesses[i].list1.Count(); j++) {
          tmp << guesses[i].list1[j].Type->symbol << '(' <<
            olxstr::FormatFloat(3,1.0/(guesses[i].list1[j].MeanFom()+0.001))
            << ")" << guesses[i].list1[j].hits[0].Fom;
          if ((j + 1) < guesses[i].list1.Count()) {
            tmp << ',';
          }
        }
        if (permutator == 0 || !permutator->IsActive()) {
          if (type == 0 || *type == guesses[i].atom->GetType()) {
            continue;
          }
          bool change = true;
          if (proposed_atoms != 0) {
            change = proposed_atoms->Contains(type);
          }
          if (change && BAIDelta != -1) {
            change = (olx_abs(type->z - guesses[i].atom->GetType().z) < BAIDelta);
          }
          if (change) {
            if (ChangeType(*guesses[i].atom, *type, dry_run)) {
              if (!dry_run) {
                olx_analysis::helper::reset_u(*guesses[i].atom);
              }
              stat.AtomTypeChanges++;
            }
          }
        }
        log.Add(tmp);
      }
    }
  }
  for (size_t i = 0; i < sn_count; i++) {
    delete sn->Node(i).Center();
  }
  delete sn;
  if (!dry_run) {
    TBasicApp::NewLogEntry(logVerbose) << log;
  }
}
//..............................................................................
TStrList::const_list_type TAutoDB::ValidateResult(const olxstr& fileName,
  const TLattice& latt)
{
  TStrList report;
  olxstr cifFN = TEFile::ChangeFileExt(fileName, "cif");
  report.Add("Starting analysis of ").quote() << cifFN << " on " <<
    TETime::FormatDateTime(TETime::Now());
  if (!TEFile::Exists(cifFN)) {
    report.Add(olxstr("The cif file does not exist"));
    return report;
  }
  try {
    XFile.LoadFromFile(cifFN);
    XFile.GetLattice().CompaqAll();
  }
  catch (const TExceptionBase& exc) {
    report.Add("Failed to load due to ").quote() <<
      exc.GetException()->GetError();
    return report;
  }
  TSpaceGroup& sga = TSymmLib::GetInstance().FindSG(latt.GetAsymmUnit());
  TSpaceGroup& sgb = TSymmLib::GetInstance().FindSG(XFile.GetAsymmUnit());
  if (&sga != &sgb) {
    report.Add("Inconsistent space group. Changed from ").quote() <<
      sgb.GetName() << " to '" << sga.GetName() << '\'';
    return report;
  }
  report.Add(olxstr("Current space group is ") << sga.GetName());
  // have to locate possible translation using 'hard' method
  TTypeList< olx_pair_t<vec3d, TCAtom*> > alist, blist;
  TTypeList< TSymmTestData > vlist;
  latt.GetUnitCell().GenereteAtomCoordinates(alist, false);
  XFile.GetUnitCell().GenereteAtomCoordinates(blist, false);
  smatd mI;
  mI.r.I();
  TSymmTest::TestDependency(alist, blist, vlist, mI, 0.01);
  vec3d thisCenter, atomCenter;
  if (vlist.Count() != 0) {
    TBasicApp::NewLogEntry(logInfo) << vlist[vlist.Count() - 1].Count();
    if (vlist[vlist.Count() - 1].Count() > (alist.Count() * 0.75)) {
      thisCenter = vlist[vlist.Count() - 1].Center;
      if (!sga.IsCentrosymmetric()) {
        thisCenter *= 2;
      }
    }
  }

  int extraAtoms = 0, missingAtoms = 0;

  TAsymmUnit& au = latt.GetAsymmUnit();

  for (size_t i = 0; i < XFile.GetAsymmUnit().AtomCount(); i++)
    XFile.GetAsymmUnit().GetAtom(i).SetTag(-1);

  for (size_t i = 0; i < au.AtomCount(); i++) {
    if (au.GetAtom(i).GetType().z < 2) {
      continue;
    }
    atomCenter = au.GetAtom(i).ccrd();
    atomCenter -= thisCenter;
    TCAtom* ca = XFile.GetUnitCell().FindCAtom(atomCenter);
    if (ca == 0) {
      report.Add("Extra atom ").quote() << au.GetAtom(i).GetLabel();
      extraAtoms++;
      continue;
    }
    ca->SetTag(i);
    if (ca->GetType() != au.GetAtom(i).GetType()) {
      report.Add("Atom type changed from ").quote() << ca->GetLabel() <<
        " to '" << au.GetAtom(i).GetLabel() << '\'';
    }
  }
  for (size_t i = 0; i < XFile.GetAsymmUnit().AtomCount(); i++) {
    if (XFile.GetAsymmUnit().GetAtom(i).GetTag() == -1 &&
      XFile.GetAsymmUnit().GetAtom(i).GetType() != iHydrogenZ)
    {
      report.Add("Missing atom ").quote() <<
        XFile.GetAsymmUnit().GetAtom(i).GetLabel();
      missingAtoms++;
    }
  }
  report.Add(olxstr("------Analysis complete with ") << extraAtoms
    << " extra atoms and " << missingAtoms << " missing atoms-----");
  report.Add(EmptyString());
  return report;
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAtomTypePermutator::Init(ElementPList* typeRestraints) {
  Atoms.Clear();
  TypeRestraints.Clear();
  if (typeRestraints != 0) {
    TypeRestraints.AddAll(*typeRestraints);
  }
}
//..............................................................................
void TAtomTypePermutator::ReInit(const TAsymmUnit& au) {
  // restore old atoms, remove deleted
  for (size_t i = 0; i < Atoms.Count(); i++) {
    Atoms[i].Atom = au.GetLattice().GetUnitCell().FindCAtom(Atoms[i].AtomCenter);
    if (Atoms[i].Atom == 0) {
      Atoms.NullItem(i);
    }
  }
  Atoms.Pack();
}
void TAtomTypePermutator::InitAtom(TAutoDB::TGuessCount& guess) {
  TTypeList< TAutoDB::THitList<TAutoDBNetNode> >& list =
    (!guess.list3.IsEmpty() ? guess.list3 : guess.list2);
  if (!list.IsEmpty() || !guess.list1.IsEmpty()) {
    TPermutation* pm = 0;
    for (size_t i = 0; i < Atoms.Count(); i++) {
      if (Atoms[i].Atom == guess.atom) {
        pm = &Atoms[i];
        break;
      }
    }
    if (list.Count() == 1 || (list.IsEmpty() && guess.list1.Count() == 1)) {
      const cm_Element* elm = (list.Count() == 1) ? list[0].Type
        : guess.list1[0].Type;
      if (guess.atom->GetType() != *elm)
        guess.atom->SetLabel(elm->symbol);
      if (pm != 0 && pm->Tries.Count()) {
        //Atoms.Delete(pmIndex);
        pm->Tries.Clear();
        TBasicApp::NewLogEntry(logVerbose) << "Converged " << guess.atom->GetLabel();
      }
      return;
    }
    if (pm == 0) {
      pm = &Atoms.AddNew();
      pm->AtomCenter = guess.atom->ccrd();
      pm->Atom = guess.atom;
    }
    else {  // check if converged
      if (pm->Tries.IsEmpty()) {
        return;
      }
    }
    if (list.Count() > 1) {
      for (size_t i = 0; i < list.Count(); i++) {
        bool found = false;
        for (size_t j = 0; j < pm->Tries.Count(); j++) {
          if (pm->Tries[j].GetA() == list[i].Type) {
            pm->Tries[j].c = list[i].MeanFom();
            if (list[i].Type == &guess.atom->GetType()) {
              pm->Tries[j].b = guess.atom->GetUiso();
            }
            found = true;
            break;
          }
        }
        if (!found) {
          pm->Tries.AddNew<const cm_Element*, double, double>(
            list[i].Type, -1, list[i].MeanFom());
          if (*list[i].Type == guess.atom->GetType()) {
            pm->Tries[pm->Tries.Count() - 1].b = guess.atom->GetUiso();
          }
        }
      }
    }
    else if (guess.list1.Count() > 1) {
      for (size_t i = 0; i < guess.list1.Count(); i++) {
        bool found = false;
        for (size_t j = 0; j < pm->Tries.Count(); j++) {
          if (pm->Tries[j].GetA() == guess.list1[i].Type) {
            pm->Tries[j].c = guess.list1[i].MeanFom();
            if (guess.list1[i].Type == &guess.atom->GetType()) {
              pm->Tries[j].b = guess.atom->GetUiso();
            }
            found = true;
            break;
          }
        }
        if (!found) {
          pm->Tries.AddNew<const cm_Element*, double, double>(
            guess.list1[i].Type, -1, guess.list1[i].MeanFom());
          if (guess.list1[i].Type == &guess.atom->GetType()) {
            pm->Tries[pm->Tries.Count() - 1].b = guess.atom->GetUiso();
          }
        }
      }
    }
  }
}
//..............................................................................
void TAtomTypePermutator::Permutate() {
  for (size_t i = 0; i < Atoms.Count(); i++) {
    bool permuted = false;
    for (size_t j = 0; j < Atoms[i].Tries.Count(); j++) {
      if (Atoms[i].Tries[j].GetB() == -1) {
        Atoms[i].Atom->SetLabel(olxstr(Atoms[i].Tries[j].GetA()->symbol), false);
        Atoms[i].Atom->SetType(*Atoms[i].Tries[j].GetA());
        permuted = true;
        break;
      }
    }
    if (permuted) {
      TBasicApp::NewLogEntry(logVerbose) << Atoms[i].Atom->GetLabel() <<
        " permutated";
    }
    else {
      const cm_Element* type = 0;
      double minDelta = 1;
      for (size_t j = 0; j < Atoms[i].Tries.Count(); j++) {
        if (olx_sqr(Atoms[i].Tries[j].GetB() - 0.025) < minDelta) {
          type = Atoms[i].Tries[j].GetA();
          minDelta = olx_sqr(Atoms[i].Tries[j].GetB() - 0.025);
        }
        TBasicApp::NewLogEntry(logVerbose) << Atoms[i].Atom->GetLabel() <<
          " permutation to " << Atoms[i].Tries[j].GetA()->symbol <<
          " leads to Uiso = " << Atoms[i].Tries[j].GetB();
      }
      if (type != 0) {
        TBasicApp::NewLogEntry(logVerbose) << "Most probable type is " <<
          type->symbol;
        if (&Atoms[i].Atom->GetType() != type) {
          Atoms[i].Atom->SetLabel(type->symbol, false);
          Atoms[i].Atom->SetType(*type);
        }
        Atoms[i].Tries.Clear();
      }
    }
  }
}
//..............................................................................
void TAutoDB::DoInitialise() {
  TStopWatch sw(__FUNC__);
  TXApp& app = TXApp::GetInstance();
  olxstr fn = app.GetSharedDir() + "acidb.db";
  if (!TEFile::Exists(fn)) {
    TEFile::Copy(app.GetBaseDir() + "acidb.db", fn);
  }
  if (TEFile::Exists(fn)) {
    TEFile dbf(fn, "rb");
    GetInstance_()->LoadFromStream(dbf);
    olxstr map_fn = fn + ".map";
    if (TEFile::Exists(map_fn)) {
      dbf.Open(map_fn, "rb");
      GetInstance_()->registry.LoadMap(dbf);
    }
  }
  GetInstance_()->src_file = fn;
}
//..............................................................................
TAutoDB& TAutoDB::GetInstance(bool init) {
  if (GetInstance_() == 0) {
    TXApp& app = TXApp::GetInstance();
    TEGC::AddP(GetInstance_() = new TAutoDB(
      *(dynamic_cast<TXFile*>(app.XFile().Replicate())), app));
    if (init) {
      GetInstance_()->DoInitialise();
    }
  }
  else {
    if (GetInstance_()->src_file.IsEmpty()) {
      GetInstance_()->DoInitialise();
    }
  }
  return *GetInstance_();
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAutoDB::LibBAIDelta(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(BAIDelta);
  }
  else {
    BAIDelta = Params[0].ToInt();
  }
}
//..............................................................................
void TAutoDB::LibURatio(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(URatio);
  }
  else {
    URatio = Params[0].ToDouble();
  }
}
//..............................................................................
void TAutoDB::LibEnforceFormula(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(EnforceFormula);
  }
  else {
    EnforceFormula = Params[0].ToBool();
  }
}
//..............................................................................
void TAutoDB::LibTolerance(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(olxstr() << LengthVar << ',' << AngleVar);
  }
  else {
    LengthVar = Params[0].ToDouble();
    AngleVar = Params[1].ToDouble();
  }
}
//..............................................................................
void TAutoDB::LibClear(TStrObjList& Cmds, const TParamList& Options, TMacroData& E) {
  Clear();
  src_file = EmptyString();
}
//..............................................................................
void TAutoDB::LibLoad(TStrObjList& Cmds, const TParamList& Options, TMacroData& E) {
  olxstr fn;
  if (Cmds.IsEmpty()) {
    TXApp& app = TXApp::GetInstance();
    fn = app.GetSharedDir() + "acidb.db";
    if (!TEFile::Exists(fn)) {
      TEFile::Copy(app.GetBaseDir() + "acidb.db", fn);
    }
  }
  else {
    fn = Cmds[0];
  }
  TEFile in(fn, "rb");
  LoadFromStream(in);
  olxstr map_fn = fn + ".map";
  if (TEFile::Exists(map_fn)) {
    TEFile dbf(map_fn, "rb");
    GetInstance_()->registry.LoadMap(dbf);
  }
  src_file = fn;
}
//..............................................................................
void TAutoDB::LibSave(TStrObjList& Cmds, const TParamList& Options, TMacroData& E) {
  bool overwrite = Options.GetBoolOption('f');
  if (!overwrite && TEFile::Exists(Cmds[0])) {
    return;
  }
  SafeSave(Cmds[0]);
}
//..............................................................................
void TAutoDB::LibDigest(TStrObjList& Cmds, const TParamList& Options, TMacroData& E) {
  if (Cmds.Count() == 2) {
    Clear();
  }
  ProcessFolder(Cmds[0],
    Options.GetBoolOption('d'),
    Options.FindValue('r', "5").ToDouble(),
    Options.FindValue('s', "0.05").ToDouble(),
    Options.FindValue('f', "0.1").ToDouble(),
    Cmds.Count() == 2 ? Cmds[1] : EmptyString());
  if (Cmds.Count() == 2 && !GetInstance_()->src_file.IsEmpty() &&
    GetInstance_()->src_file != Cmds[1])
  {
    TEFile in(GetInstance_()->src_file, "rb");
    LoadFromStream(in);
    olxstr map_fn = src_file + ".map";
    if (TEFile::Exists(map_fn)) {
      TEFile dbf(map_fn, "rb");
      GetInstance_()->registry.LoadMap(dbf);
    }
  }
}
//..............................................................................
void TAutoDB::LibLock(TStrObjList& Cmds, const TParamList& Options, TMacroData& E) {
  bool unlock = Options.GetBoolOption('u');
  TXApp& app = TXApp::GetInstance();
  TSAtomPList atoms = app.FindSAtoms(Cmds);
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i]->CAtom().SetFixedType(!unlock);
  }
}
//..............................................................................
TLibrary* TAutoDB::ExportLibrary(const olxstr& name) {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("ata") : name);
  lib->Register(
    new TFunction<TAutoDB>(this, &TAutoDB::LibBAIDelta, "BAIDelta",
      fpNone | fpOne,
      "Returns/sets maximum difference between element types to promote")
  );
  lib->Register(
    new TFunction<TAutoDB>(this, &TAutoDB::LibURatio, "URatio", fpNone | fpOne,
      "Returns/sets a ration between atom U and mean U of the confident atoms to"
      " consider promotion")
  );
  lib->Register(
    new TFunction<TAutoDB>(this, &TAutoDB::LibEnforceFormula, "EnforceFormula",
      fpNone | fpOne,
      "Returns/sets user formula enforcement option")
  );
  lib->Register(
    new TFunction<TAutoDB>(this, &TAutoDB::LibTolerance, "Tolerance",
      fpNone | fpTwo,
      "Returns/sets maximum deviations for lengths and angles")
  );
  lib->Register(
    new TMacro<TAutoDB>(this, &TAutoDB::LibLoad, "Load",
      EmptyString(),
      fpNone|fpOne,
      "Loads ACIDB from the given file merging with current data."
      " Loads the default if no arguments are given")
  );
  lib->Register(
    new TMacro<TAutoDB>(this, &TAutoDB::LibClear, "Clear",
      EmptyString(),
      fpNone,
      "Clears current data")
  );
  lib->Register(
    new TMacro<TAutoDB>(this, &TAutoDB::LibSave, "Save",
      "f-overwrite the file if exists [false]",
      fpOne,
      "Saves ACIDB to the given file")
  );
  lib->Register(
    new TMacro<TAutoDB>(this, &TAutoDB::LibDigest, "Digest",
      "d-allow disorder [false]&;"
      "r-max R1 [5]&;"
      "s-max shift/esd [0.05]&;"
      "f-max deviation of GoF from 1 [0.1]&;"
      ,
      fpOne|fpTwo,
      "Digests CIFs from the given folder and updates the ACIDB."
      " Destination DB can be specified as second parameter.")
  );
  lib->Register(
    new TMacro<TAutoDB>(this, &TAutoDB::LibLock, "Lock",
      "u-unlock [false]",
      fpAny,
      "Locks/unlocks atoms for ata")
  );
  return lib;
}
//..............................................................................
