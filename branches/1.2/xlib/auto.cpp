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

// variations
const double LengthVar = 0.03,
             AngleVar  = 5.4;

//..............................................................................
void TAutoDBRegistry::SaveToStream(IDataOutputStream& output) const {
  output << (uint32_t)entries.Count();
  for( size_t i=0; i < entries.Count(); i++ )
    output << TUtf8::Encode(entries.GetKey(i));
}
//..............................................................................
void TAutoDBRegistry::LoadFromStream(IDataInputStream& input) {
  uint32_t fc;
  input >> fc;
  entries.SetCapacity(fc);
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
  entries.SetCapacity(fc);
  olxcstr tmp;
  for( uint32_t i=0; i < fc; i++ )  {
    input >> tmp;
    size_t idx = entries.IndexOf(TUtf8::Decode(tmp));
    input >> tmp;
    if (idx != InvalidIndex)
      entries.GetValue(idx)->reference = TUtf8::Decode(tmp);
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
TAttachedNode::TAttachedNode(IDataInputStream& in)  {
  uint32_t ind;
  in >> ind;
  Element = &XElementLib::GetByIndex(ind);
  float val;
  in >> val;  FCenter[0] = val;
  in >> val;  FCenter[1] = val;
  in >> val;  FCenter[2] = val;
}
//..............................................................................
void TAttachedNode::SaveToStream( IDataOutputStream& output ) const {
  output << (int32_t)Element->index;
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

TAutoDBNode::TAutoDBNode(TSAtom& sa,
  TTypeList<olx_pair_t<TCAtom*, vec3d> >* atoms)
{
  Center = sa.crd();
  Element = &sa.GetType();
  const TCAtom& ca = sa.CAtom();
  const TUnitCell& uc = sa.GetNetwork().GetLattice().GetUnitCell();
  const TAsymmUnit& au = sa.GetNetwork().GetLattice().GetAsymmUnit();
  //TArrayList<olx_pair_t<const TCAtom *, vec3d> > res;
  //uc.FindInRangeAC(sa.ccrd(), 2, res);
  //for (size_t i=0; i < res.Count(); i++) {
  //  if (res[i].GetB().QDistanceTo(sa.crd()) < 1e-3 ) {
  //    continue;
  //  }
  //  bool unique = true;
  //  for ( size_t j=0; j < AttachedNodes.Count(); j++) {
  //    if (AttachedNodes[j].GetCenter().QDistanceTo(res[i].GetB()) < 1e-3) {
  //      unique = false;
  //      break;
  //    }
  //  }
  //  if (!unique) continue;
  //  if( atoms != NULL )
  //    atoms->AddNew<TCAtom*, vec3d>(
  //    const_cast<TCAtom*>(res[i].GetA()), res[i].GetB());
  //  AttachedNodes.Add(new TAttachedNode(&res[i].GetA()->GetType(), res[i].GetB()));
  //
  //}
  for( size_t i=0; i < ca.AttachedSiteCount(); i++ )  {
    const TCAtom::Site& site = ca.GetAttachedSite(i);
    if( ca.IsDeleted() || site.atom->GetType() == iHydrogenZ )  continue;
    const smatd m = sa.GetMatrix().IsFirst()
      ? site.matrix : uc.MulMatrix(site.matrix, sa.GetMatrix());
    const vec3d p = au.Orthogonalise(m*site.atom->ccrd());
    if( atoms != NULL )
      atoms->AddNew<TCAtom*, vec3d>(site.atom, p);
    AttachedNodes.Add(new TAttachedNode(&site.atom->GetType(), p));
  }
  TAutoDBNode::SortCenter = sa.crd();
  QuickSorter::SortSF(AttachedNodes, SortMetricsFunc);
  if( atoms != NULL )
    QuickSorter::SortSF(*atoms, SortCAtomsFunc);
  _PreCalc();
}
//..............................................................................
void TAutoDBNode::_PreCalc()  {
  Params.Resize( (AttachedNodes.Count()+1)*AttachedNodes.Count()/2);
  size_t index = AttachedNodes.Count();
  for( size_t i=0; i < AttachedNodes.Count(); i++ )  {
    Params[i] = CalcDistance(i);
    for( size_t j=i+1; j < AttachedNodes.Count(); j++ )  {
      Params[index] = CalcAngle(i,j);
      index ++;
    }
  }
}
//..............................................................................
double TAutoDBNode::CalcAngle(size_t i, size_t j)  const {
  vec3d a(AttachedNodes[i].GetCenter()-Center),
        b(AttachedNodes[j].GetCenter()-Center);
  if( a.QLength()*b.QLength() == 0 )  {
    TBasicApp::NewLogEntry(logError) <<  "Overlapping atoms encountered";
    return 0;
  }
  double ca = a.CAngle(b);
  if( ca < -1 )  ca = -1;
  if( ca > 1 )  ca = 1;
  return acos(ca)*180.0/M_PI;
}
//..............................................................................
void TAutoDBNode::SaveToStream(IDataOutputStream& output) const {
  output << (uint32_t)Element->index;
  output << (float)Center[0];
  output << (float)Center[1];
  output << (float)Center[2];
  output << (int)AttachedNodes.Count();
  for( size_t i=0; i < AttachedNodes.Count(); i++ )
    AttachedNodes[i].SaveToStream(output);
}
//..............................................................................
void TAutoDBNode::LoadFromStream(IDataInputStream& in)  {
  Element = &XElementLib::GetByIndex(in.Read<uint32_t>());
  float val;
  Center[0] = (in >> val);
  Center[1] = (in >> val);
  Center[2] = (in >> val);
  uint32_t cnt = in.Read<uint32_t>();
  for( uint32_t i=0; i < cnt; i++ )
    AttachedNodes.Add(*(new TAttachedNode(in)));
  TAutoDBNode::SortCenter = Center;
  QuickSorter::SortSF(AttachedNodes, SortMetricsFunc);
  _PreCalc();
}
//..............................................................................
const olxstr& TAutoDBNode::ToString() const {
  olxstr& tmp = TEGC::New<olxstr>(EmptyString(), 100);
  tmp << Element->symbol << '{';
  for( size_t i=0; i < AttachedNodes.Count(); i++ )  {
    tmp << AttachedNodes[i].GetType().symbol;
    if( (i+1) < AttachedNodes.Count() )
      tmp << ',';
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
  size_t mc = (AttachedNodes.Count() > 4 ) ? AttachedNodes.Count()
    : Params.Count();
  for( size_t i=0; i < mc; i++ ) {
    double diff = Params[i] - dbn.Params[i];
    if( i < AttachedNodes.Count() )  {
      if( olx_abs(diff) > LengthVar )
        return diff;
    }
    else  {
      if( olx_abs(diff) > AngleVar )
        return diff/180;
      diff /= 180;
    }
    _fom += diff*diff;
  }
  if( fom != NULL )
    *fom += _fom/Params.Count();
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
  double diff = Element->z - dbn.Element->z;
  if( diff == 0 )  {
    diff = olx_cmp(AttachedNodes.Count(), dbn.AttachedNodes.Count());
    if( diff == 0 )  {
      for( size_t i=0; i < AttachedNodes.Count(); i++ )  {
        diff = AttachedNodes[i].GetType().z - dbn.AttachedNodes[i].GetType().z;
        if( diff != 0 )  return (int)diff;
      }
      for( size_t i=0; i < Params.Count(); i++ )  {
        diff = Params[i] - dbn.Params[i];
        if( diff < 0 )  return -1;
        if( diff > 0 )  return 1;
      }
      return 0;
    }
    else
      return (int)diff;
  }
  else
    return (int)diff;
}
//..............................................................................
bool TAutoDBNode::IsSameType(const TAutoDBNode& dbn) const {
  int diff = Element->z - dbn.Element->z;
  if( diff != 0 )  return false;
  diff = olx_cmp(AttachedNodes.Count(), dbn.AttachedNodes.Count());
  if( diff != 0 )  return false;
  for( size_t i=0; i < AttachedNodes.Count(); i++ )  {
    diff = AttachedNodes[i].GetType().z - dbn.AttachedNodes[i].GetType().z;
    if( diff != 0 )  return false;
  }
  return true;
}
//..............................................................................
bool TAutoDBNode::IsSimilar(const TAutoDBNode& dbn) const {
  double diff = Element->z - dbn.Element->z;
  if( diff == 0 )  {
    diff = olx_cmp(AttachedNodes.Count(), dbn.AttachedNodes.Count());
    if( diff == 0 )  {
      // check types
      for( size_t i=0; i < AttachedNodes.Count(); i++ )  {
        diff = AttachedNodes[i].GetType().z -
               dbn.AttachedNodes[i].GetType().z;
        if( diff != 0 )  return false;
      }
      // check distance and angles
      for( size_t i=0; i < Params.Count(); i++ )  {
        diff = olx_abs(Params[i] - dbn.Params[i]);
        if( i < AttachedNodes.Count() )  {
          if( diff > 0.005 ) return false;
        }
        else
          if( diff > 4 ) return false;
      }
      return true;
    }
    else
      return false;
  }
  else
    return false;
}
//..............................................................................
bool TAutoDBNode::IsMetricSimilar(const TAutoDBNode& dbn, double& fom) const {
  if( AttachedNodes.Count() != dbn.AttachedNodes.Count() )  return false;
  // check distance and angles
  double _fom = 0;
  size_t mc = (AttachedNodes.Count() > 4 )
    ? AttachedNodes.Count() : Params.Count();
  for( size_t i=0; i < mc; i++ ) {
//  for( size_t i=0; i < Params.Count(); i++ ) {
    double diff = olx_abs(Params[i] - dbn.Params[i]);
    if( i < AttachedNodes.Count() )  {
      if( diff > LengthVar )  return false;
    }
    /* 5.7 degrees give about 0.1 deviation in distance for 1A bonds
    (b^2+a^2-2abcos)^1/2
    */
    else  {
      if( diff > AngleVar ) return false;
      diff = diff/180.0;
    }
    _fom += diff*diff;
  }
  fom += _fom/Params.Count();
  return true;
}
//..............................................................................
//..............................................................................
//..............................................................................
bool TAutoDBNetNode::IsMetricSimilar(const TAutoDBNetNode& nd, double& cfom,
  uint16_t* cindexes, bool ExtraLevel) const
{
  if( nd.AttachedNodes.Count() != AttachedNodes.Count() )  return false;
  // have to do a full comparison, as the node order is unknown ...
  const uint16_t con =
    (uint16_t)olx_min(MaxConnectivity, AttachedNodes.Count());
  TAutoDBNetNode* nodes[MaxConnectivity];
  for( uint16_t i=0; i < con; i++ )
    nodes[i] = AttachedNodes[i];
  uint16_t indexes[MaxConnectivity];
  for( uint16_t i=0; i < nd.AttachedNodes.Count(); i++ )  {
    for( uint16_t j=0; j < con; j++ )  {
      if( nodes[j] == NULL ) continue;
      if( nodes[j]->FCenter->IsMetricSimilar(
        *nd.AttachedNodes[i]->FCenter, cfom) )
      {
        nodes[j] = NULL;
        indexes[i] = j;
        if( cindexes != NULL )
          cindexes[i] = j;
        break;
      }
    }
  }
  int ndcnt = 0;
  for( uint16_t i=0; i < con; i++ )
    if( nodes[i] != NULL )  ndcnt++;

  if( !ExtraLevel )  {
    if( ndcnt != 0 )  return false;
  }
  else  {
    if( ndcnt != 0 )  return false;
    else  {
      for( uint16_t i=0; i < con; i++ )
        if( !nd.AttachedNodes[i]->IsMetricSimilar(
            *AttachedNodes[indexes[i]], cfom, NULL, false) )
          return false;
    }
  }
  return true;
}
//..............................................................................
bool TAutoDBNetNode::IsSameType(const TAutoDBNetNode& dbn, bool ExtraLevel)
  const
{
  if( !FCenter->IsSameType(*dbn.FCenter) )  return false;
  TPtrList<TAutoDBNetNode> nodes;
  nodes.AddList(AttachedNodes);
  TSizeList indexes;
  indexes.SetCapacity( nodes.Count() );
  for( size_t i=0; i < dbn.AttachedNodes.Count(); i++ )  {
    for(size_t j=0; j < nodes.Count(); j++ )  {
      if( nodes[j] == NULL ) continue;
      if( nodes[j]->FCenter->IsSameType(*dbn.AttachedNodes[i]->FCenter) )  {
        nodes[j] = NULL;
        indexes.Add(j);
        break;
      }
    }
  }
  nodes.Pack();
  if( !ExtraLevel )
    return nodes.IsEmpty();
  else  {
    if( nodes.Count() != 0 )  return false;
    for( size_t i=0; i < dbn.AttachedNodes.Count(); i++ )  {
      if( !dbn.AttachedNodes[i]->IsSameType(
          *AttachedNodes[indexes[i]], false) )
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
  for( size_t i=0; i < AttachedNodes.Count(); i++ )
    output << AttachedNodes[i]->GetId();
}
void TAutoDBNetNode::LoadFromStream(IDataInputStream& input)  {
#ifdef __GNUC__  // dunno how it is implemented, but need 8 bits here
  unsigned char cnt;
#else
  uint8_t cnt;
#endif
  int32_t ind;
  input >> ind;
  FCenter = TAutoDB::GetInstance().Node(ind);
  input >> cnt;
  for( size_t i=0; i < cnt; i++ )  {
    input >> ind;
    AttachedNodes.Add(&TAutoDBNet::GetCurrentlyLoading().Node(ind));
  }
}
//..............................................................................
const olxstr& TAutoDBNetNode::ToString(int level) const  {
  olxstr& tmp = TEGC::New<olxstr>(EmptyString(), 256);
  tmp << FCenter->ToString();
  if( level == 1 )  {
    tmp << '{';
    for( size_t i=0; i < AttachedNodes.Count(); i++ )  {
      tmp << AttachedNodes[i]->FCenter->ToString();
      if( (i+1) < AttachedNodes.Count() )
        tmp << ',';
    }
    tmp << '}';
  }
  else if( level == 2 )  {
    tmp << '[';
    for( size_t i=0; i < AttachedNodes.Count(); i++ )  {
      tmp << AttachedNodes[i]->ToString(1);
      if( (i+1) < AttachedNodes.Count() )
        tmp << ',';
    }
    tmp << ']';
  }
  return tmp;
}
//..............................................................................
//..............................................................................
//..............................................................................
TAutoDBNet* TAutoDBNet::CurrentlyLoading = NULL;
//..............................................................................
void TAutoDBNet::SaveToStream(IDataOutputStream& output) const {
  output << (uint32_t)FReference->id;
  output << (uint16_t)Nodes.Count();
  for( size_t i=0; i < Nodes.Count(); i++ )
    Nodes[i].SetId((int32_t)i);
  for( size_t i=0; i < Nodes.Count(); i++ )
    Nodes[i].SaveToStream(output);
}
void TAutoDBNet::LoadFromStream(IDataInputStream& input)  {
  TAutoDBNet::CurrentlyLoading = this;
  uint32_t ind;
  uint16_t cnt;
  input >> ind;
  FReference = &TAutoDB::GetInstance().Reference(ind);
  input >> cnt;
  Nodes.SetCapacity(cnt);
  for( uint16_t i=0; i < cnt; i++ )
    Nodes.Add(new TAutoDBNetNode(NULL));
  for( uint16_t i=0; i < cnt; i++ )
    Nodes[i].LoadFromStream(input);
  // build index
  for( uint16_t i=0; i < cnt; i++ )
    Nodes[i].Center()->AddParent(this,i);
  TAutoDBNet::CurrentlyLoading = NULL;
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
  if( v < 0 )  return -1;
  if( v > 0 )  return 1;
  return 0;
}
//..............................................................................
//..............................................................................
TAutoDB* TAutoDB::Instance = NULL;


TAutoDB::TAutoDB(TXFile& xfile, ALibraryContainer& lc) : XFile(xfile)  {
  if( Instance != NULL )  {
    throw TFunctionFailedException(__OlxSourceInfo,
      "duplicated object instance");
  }
  Instance = this;
  for( uint16_t i=0; i < MaxConnectivity-1; i++ )
    Nodes.AddNew();
  BAIDelta = -1;
  URatio = 1.5;
  EnforceFormula = false;
  lc.GetLibrary().AttachLibrary(ExportLibrary());
}
//..............................................................................
TAutoDB::~TAutoDB()  {
  for( size_t i=0; i < Nodes.Count(); i++ )
    for( size_t j=0; j < Nodes[i].Count(); j++ )
      delete Nodes[i][j];
  Instance = NULL;
  delete &XFile;
}
//..............................................................................
void TAutoDB::PrepareForSearch()  {
  for( size_t i=0; i < Nodes.Count(); i++ )
    QuickSorter::SortSF(Nodes[i], SearchCompareFunc);
}
//..............................................................................
void TAutoDB::ProcessFolder(const olxstr& folder)  {
  if( !TEFile::Exists(folder) )  return;
  olxstr uf = TEFile::TrimPathDelimeter(folder);
  TFileTree ft(uf);
  ft.Expand();
  TStrList files;
  ft.GetRoot().ListFiles(files, "*.cif");

  TOnProgress progress;
  progress.SetMax(files.Count());
  for( size_t i=0; i < Nodes.Count(); i++ )  {
    Nodes[i].SetCapacity(Nodes[i].Count() + files.Count()*100);
    Nodes[i].SetIncrement(64*1024);
  }
  for( size_t i=0; i < files.Count(); i++ )  {
    progress.SetPos(i);
    TBasicApp::NewLogEntry() << "Processing: " << files[i];
    olxstr digest;
    try  {
      TEFile f(files[i], "rb");
      digest = MD5::Digest(f);
    }
    catch(...) {
      TBasicApp::NewLogEntry(logError) << "Reading failed, skipping";
      continue;
    }
    if (registry.Contains(digest)) {
      TBasicApp::NewLogEntry(logError) << "Duplicate entry, skipping";
      continue;
    }
    try  {
      XFile.LoadFromFile(files[i]);
      TCif& cif = XFile.GetLastLoader<TCif>();
      olxstr r1 = cif.GetParamAsString("_refine_ls_R_factor_gt");
      if( r1.Length() && r1.ToDouble() > 5 )  {
        TBasicApp::NewLogEntry() << "Skipped: r1=" << r1;
        continue;
      }
      olxstr shift = cif.GetParamAsString("_refine_ls_shift/su_max");
      if( shift.Length() && shift.ToDouble() > 0.05 )  {
        TBasicApp::NewLogEntry() << "Skipped: shift=" << shift;
        continue;
      }
      olxstr gof = cif.GetParamAsString("_refine_ls_goodness_of_fit_ref");
      if( gof.Length() && olx_abs(1-gof.ToDouble()) > 0.1 )  {
        TBasicApp::NewLogEntry() << "Skipped: GOF=" << gof;
        continue;
      }
      bool has_parts = false;
      TAsymmUnit &au = XFile.GetAsymmUnit();
      for (size_t ai=0; ai < au.AtomCount(); ai++) {
        if (au.GetAtom(ai).GetPart() != 0) {
          has_parts = true;
          break;
        }
      }
      if( has_parts )  {
        TBasicApp::NewLogEntry() << "Skipped: contains disorder";
        continue;
      }
      XFile.GetLattice().CompaqAll();
      TAutoDBIdObject &adf = registry.Add(digest, files[i]);
      for( size_t j=0; j < XFile.GetLattice().FragmentCount(); j++ )
        ProcessNodes(&adf, XFile.GetLattice().GetFragment(j));
    }
    catch( const TExceptionBase& e)  {
      TBasicApp::NewLogEntry(logError) << "Failed to process: " << files[i];
      TBasicApp::NewLogEntry(logExceptionTrace) << e;
    }
  }
  PrepareForSearch();
  try {
    TEFile tf(src_file + ".tmp", "wb+");
    SaveToStream(tf);
    tf.Close();
    TEFile::Rename(src_file + ".tmp", src_file);
    tf.Open(src_file + ".tmp", "wb+");
    registry.SaveMap(tf);
    tf.Close();
    TEFile::Rename(src_file + ".tmp", src_file + ".map");
  }
  catch(const TExceptionBase &e) {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
}
//..............................................................................
struct TTmpNetData  {
  TSAtom* Atom;
  TAutoDBNode* Node;
  TTypeList<olx_pair_t<TCAtom*, vec3d> >* neighbours;
};
void TAutoDB::ProcessNodes(TAutoDBIdObject* currentFile, TNetwork& net)  {
  if( net.NodeCount() == 0 )  return;
  TTypeList< TTmpNetData* > netMatch;
  TTmpNetData* netItem;
  for( size_t i=0; i < net.NodeCount(); i++ )
    net.Node(i).SetTag(1);
  for( size_t i=0; i < net.NodeCount(); i++ )  {
    if (net.Node(i).GetType().z < 2 ||net.Node(i).IsDeleted())
      continue;
    netItem = new TTmpNetData;
    netItem->neighbours = new TTypeList<olx_pair_t<TCAtom*, vec3d> >;
    netItem->Atom = &net.Node(i);
    TAutoDBNode* dbn = new TAutoDBNode(net.Node(i), netItem->neighbours);
    /* instead of MaxConnectivity we use Nodes.Count() to comply with db
    format */
    if( dbn->NodeCount() < 1 || dbn->NodeCount() > Nodes.Count() )  {
      delete dbn;
      delete netItem->neighbours;
      delete netItem;
    }
    else  {
      TPtrList<TAutoDBNode>& segment = Nodes[dbn->NodeCount()-1];
      for( size_t j=0; j < segment.Count(); j++ )  {
        if( segment[j]->IsSimilar(*dbn) )  {
          netItem->Node = segment[j];
          delete dbn;
          dbn = NULL;
          break;
        }
      }
      if( dbn != NULL )  {
        netItem->Node = dbn;
        Nodes[dbn->NodeCount()-1].Add(dbn);
      }
      netMatch.AddCopy(netItem);
    }
  }
  // construct the network
  if( netMatch.Count() > 1 )  {
    /*this gives a one-to-one match between CAtoms and net nodes with
    CAtom->GetId(). However there is a problems since the fragments represent
    current content of the asymmetric unit and therefore some atoms might be
    attached to CAtoms of other fragments. We avoid this situation by
    considering only atoms of this fragment
    */
    net.GetLattice().GetAsymmUnit().GetAtoms().ForEach(
      ACollectionItem::TagSetter(-1));
    for( size_t i=0; i < netMatch.Count(); i++ )
      netMatch[i]->Atom->CAtom().SetTag(i);
    TAutoDBNet& net = Nets.AddNew(currentFile);
    // precreate nodes
    for( uint32_t i=0; i < netMatch.Count(); i++ )  {
      net.NewNode(netMatch[i]->Node);
      netMatch[i]->Node->AddParent(&net, i); // build index
    }
    // build connectivity
    for( size_t i=0; i < netMatch.Count(); i++ )  {
      for( size_t j=0; j < netMatch[i]->neighbours->Count(); j++ )  {
        if( netMatch[i]->neighbours->GetItem(j).GetA()->GetTag() < 0 )
          continue;
        net.Node(i).AttachNode(
          &net.Node(netMatch[i]->neighbours->GetItem(j).GetA()->GetTag()));
      }
    }
    for( size_t i=0; i < netMatch.Count(); i++ ) {
      delete netMatch[i]->neighbours;
      delete netMatch[i];
    }
  }
//  for( size_t i=0; i < Nodes.Count(); i++ )
//    Nodes[i]->Average();

  return;
}
//..............................................................................
TAutoDBNet* TAutoDB::BuildSearchNet(TNetwork& net, TSAtomPList& cas)  {
  if( net.NodeCount() == 0 )  return NULL;
  TPtrList<TTmpNetData> netMatch;
  for( size_t i=0; i < net.NodeCount(); i++ )
    net.Node(i).SetTag(1);
  for( size_t i=0; i < net.NodeCount(); i++ )  {
//    if( net.Node(i).GetType() != iQPeakZ &&
    if( net.Node(i).GetType() != iHydrogenZ ) {
      TTmpNetData *netItem = new TTmpNetData;
      netItem->neighbours = new TTypeList<olx_pair_t<TCAtom*, vec3d> >;
      netItem->Atom = &net.Node(i);
      TAutoDBNode* dbn = new TAutoDBNode(net.Node(i), netItem->neighbours);
      if( dbn->NodeCount() < 1 || dbn->NodeCount() > 12 )  {
        delete dbn;
        delete netItem->neighbours;
        delete netItem;
      }
      else  {
        netItem->Node = dbn;
        netMatch.Add(netItem);
      }
    }
  }
  // construct the network
  if( netMatch.Count() > 0 )  {
    for( size_t i=0; i < net.GetLattice().GetAsymmUnit().AtomCount(); i++ )
      net.GetLattice().GetAsymmUnit().GetAtom(i).SetTag(-1);
    TAutoDBNet* dbnet = new TAutoDBNet(NULL);
    for( size_t i=0; i < netMatch.Count(); i++ )  {
      dbnet->NewNode(netMatch[i]->Node);
      cas.Add(netMatch[i]->Atom);
      netMatch[i]->Atom->CAtom().SetTag(i);
    }
    for( size_t i=0; i < netMatch.Count(); i++ )  {
      for( size_t j=0; j < netMatch[i]->neighbours->Count(); j++ )  {
        if( netMatch[i]->neighbours->GetItem(j).GetA()->GetTag() < 0 )
          continue;
        dbnet->Node(i).AttachNode(
          &dbnet->Node(netMatch[i]->neighbours->GetItem(j).GetA()->GetTag()));
      }
    }
    for( size_t i=0; i < netMatch.Count(); i++ ) {
      delete netMatch[i]->neighbours;
      delete netMatch[i];
    }
    return dbnet;
  }
  return NULL;
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
  for( uint32_t i=0; i < Nodes.Count(); i++ )  {
    output << (uint32_t)Nodes[i].Count();
    for( uint32_t j=0; j < Nodes[i].Count(); j++ )  {
      Nodes[i][j]->SetId(nodeCount+j);
      Nodes[i][j]->SaveToStream(output);
    }
    nodeCount += (uint32_t)Nodes[i].Count();
  }

  output << (uint32_t)Nets.Count();
  for( uint32_t i=0; i < Nets.Count(); i++ )
    Nets[i].SaveToStream(output);
}
//..............................................................................
void TAutoDB::LoadFromStream(IDataInputStream& input)  {
  // validation of the file
  char fileSignature[FileSignatureLength+1];
  input.Read( fileSignature, FileSignatureLength );
  fileSignature[FileSignatureLength] = '\0';
  if( olxstr(fileSignature) != FileSignature )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file signature");
  uint16_t fileVersion;
  input >> fileVersion;
  if( fileVersion != FileVersion )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file version");
  // read file flags
  input >> fileVersion;
  // end of the file validation
  registry.LoadFromStream(input);
  uint32_t ind, listCount, nodeCount=0;
  input >> listCount;  // nt MaxConnectivity is overriden!
  Nodes.Clear();
  Nodes.SetCapacity(listCount);

  TOnProgress pg;
  pg.SetAction("Loading database...");
  pg.SetMax(listCount);
  TBasicApp::GetInstance().OnProgress.Enter(NULL, &pg);

  for( uint32_t i=0; i < listCount; i++ )  {
    Nodes.AddNew();
    input >> ind;
    Nodes[i].SetCapacity(ind);
    for( uint32_t j=0; j < ind; j++ )  {
      Nodes[i].Add(new TAutoDBNode(input));
      Nodes[i][j]->SetId(nodeCount + j);
    }
    nodeCount += ind;
    pg.SetPos(i);
    TBasicApp::GetInstance().OnProgress.Execute(NULL, &pg);
  }

  input >> ind;
  Nets.SetCapacity(ind);
  for( uint32_t i=0; i < ind; i++ )
    Nets.Add(new TAutoDBNet(input));

  PrepareForSearch();
  pg.SetPos(pg.GetMax());
  TBasicApp::GetInstance().OnProgress.Execute(NULL, &pg);
  TBasicApp::GetInstance().OnProgress.Exit(NULL, &pg);
}
//..............................................................................
void TAutoDB::AnalyseNode(TSAtom& sa, TStrList& report)  {
  TSAtomPList cas;
  TAutoDBNet* sn = BuildSearchNet(sa.GetNetwork(), cas);
  if( sn == NULL )  return;
  olxstr tmp;
  size_t index = InvalidIndex;
  for( size_t i=0; i < cas.Count(); i++ )  {
    if( cas[i] == &sa )  {
      index = i;
      break;
    }
  }
  if( index == InvalidIndex )  return;
  TAutoDBNetNode& node = sn->Node(index);
  if( node.Count() < 1 || node.Count() > Nodes.Count() ) return;
  TPtrList< TAutoDBNode >& segment = Nodes[node.Count() - 1];
  TTypeList< olx_pair_t<TAutoDBNode*, int> > S1Match;
  TTypeList< AnAssociation3<TAutoDBNetNode*, int, TAutoDBIdPList*> > S2Match,
    S3Match;
  for( size_t i=0; i < segment.Count(); i++ )  {
    double fom = 0;
    if( segment[i]->IsMetricSimilar(*node.Center(), fom) )  {
      //
      bool found = false;
      for( size_t j=0; j < S1Match.Count(); j++ )  {
        if( S1Match[j].GetA()->IsSameType(*segment[i]) )  {
          S1Match[j].b ++;
          found = true;
          break;
        }
      }
      if( !found )  S1Match.AddNew<TAutoDBNode*,int>(segment[i], 1);
      //
      for( size_t j=0; j < segment[i]->ParentCount(); j++ )  {
        double cfom = 0;
        TAutoDBNetNode& netnd = segment[i]->GetParent(j)->Node(
          segment[i]->GetParentIndex(j));
        //TBasicApp::NewLogEntry(logInfo) << Nodes[i]->GetParent(j)->Reference()->GetName();
        if( netnd.IsMetricSimilar(node, cfom, NULL, false) )  {
          //
          found = false;
          for( size_t k=0; k < S2Match.Count(); k++ )  {
            if( S2Match[k].GetA()->IsSameType(netnd, false) )  {
              S2Match[k].b ++;
              S2Match[k].c->Add(segment[i]->GetParent(j)->Reference());
              found = true;
              break;
            }
          }
          if( !found )  {
            S2Match.AddNew<TAutoDBNetNode*,int,TAutoDBIdPList*>(
              &netnd, 1, new TAutoDBIdPList);
            S2Match[S2Match.Count()-1].c->Add(
              segment[i]->GetParent(j)->Reference());
          }
          //
          if( netnd.IsMetricSimilar(node, cfom, NULL, true) )  {
            //
            found = false;
            for( size_t k=0; k < S3Match.Count(); k++ )  {
              if( S3Match[k].GetA()->IsSameType(netnd, false) )  {
                S3Match[k].b ++;
                S3Match[k].c->Add(segment[i]->GetParent(j)->Reference());
                found = true;
                break;
              }
            }
            if( !found )  {
              S3Match.AddNew<TAutoDBNetNode*,int,TAutoDBIdPList*>(
                &netnd, 1, new TAutoDBIdPList);
              S3Match[S3Match.Count()-1].c->Add(
                segment[i]->GetParent(j)->Reference());
            }
            //
          }
        }
      }
    }
  }
  if( !S1Match.IsEmpty() )  {
    report.Add( "S1 matches:" );
    for( size_t i=0; i < S1Match.Count(); i++ )  {
      report.Add( olxstr("   ") << S1Match[i].GetA()->ToString() <<
        ' ' << S1Match[i].GetB() << " hits" );
    }
    if( !S2Match.IsEmpty() )  {
      report.Add( "S2 matches:" );
      for( size_t i=0; i < S2Match.Count(); i++ )  {
        report.Add( olxstr("   ") << S2Match[i].GetA()->ToString(1) <<
          ' ' << S2Match[i].GetB() << " hits" );
        olxstr tmp("Refs [");
        for( size_t j=0; j < S2Match[i].GetC()->Count(); j++ )  {
          tmp << S2Match[i].GetC()->GetItem(j)->reference << ';';
        }
        report.Add( tmp << ']' );
        delete S2Match[i].GetC();
      }
      if( !S3Match.IsEmpty() )  {
        report.Add( "S3 matches:" );
        for( size_t i=0; i < S3Match.Count(); i++ )  {
          report.Add( olxstr("   ") << S3Match[i].GetA()->ToString(2) <<
            ' ' << S3Match[i].GetB() << " hits" );
          olxstr tmp("Refs [");
          for( size_t j=0; j < S3Match[i].GetC()->Count(); j++ )  {
            tmp << S3Match[i].GetC()->GetItem(j)->reference << ';';
          }
          report.Add( tmp << ']' );
          delete S3Match[i].GetC();
        }
      }
    }
  }

  for( size_t i=0; i < sn->Count(); i++ )
    delete sn->Node(i).Center();
  delete sn;
  return;
}
//..............................................................................
void TAutoDB::AnalyseStructure(const olxstr& lastFileName, TLattice& latt,
  TAtomTypePermutator* permutator, TAutoDB::AnalysisStat& stat,
  ElementPList* proposed_atoms)
{
  LastFileName = lastFileName;
  stat.Clear();
  stat.FormulaConstrained = (proposed_atoms != NULL);
  stat.AtomDeltaConstrained = (BAIDelta != -1);
  Uisos.Clear();
  for( size_t i=0; i < latt.FragmentCount(); i++ )  {
    AnalyseNet(latt.GetFragment(i), permutator,
      Uisos.Add(0), stat, proposed_atoms);
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
  if( segment.IsEmpty() )  return InvalidIndex;
  if( from == InvalidIndex ) from = 0;
  if( to == InvalidIndex )  to = segment.Count()-1;
  if( to == from )  return to;
  if( (to-from) == 1 )  return from;
  int resfrom = SearchCompareFunc(*segment[from], *nd),
      resto   = SearchCompareFunc(*segment[to], *nd);
  if( resfrom == 0 )  return from;
  if( resto == 0 )    return to;
  if( resfrom < 0 && resto > 0 )  {
    size_t index = (to+from)/2;
    int res = SearchCompareFunc(*segment[index], *nd);
    if( res < 0 )  {  return LocateDBNodeIndex(segment, nd, index, to);  }
    if( res > 0 )  {  return LocateDBNodeIndex(segment, nd, from, index);  }
    if( res == 0 )  {  return index;  }
  }
  return InvalidIndex;
}
//..............................................................................
void TAutoDB::TAnalyseNetNodeTask::Run(size_t index )  {
  const TAutoDBNetNode& nd = Network.Node(index);
  if( nd.Count() < 1 || nd.Count() > Nodes.Count() ) return;
  const TPtrList< TAutoDBNode >& segment = Nodes[nd.Count() - 1];
  //long ndind = LocateDBNodeIndex(segment, nd.Center());
  //if( ndind == -1 )  return;
  //long position = ndind, inc = 1;
  TGuessCount& gc = Guesses[index];
  bool found;
  double fom, cfom; //, var;
  for( size_t i=0; i < segment.Count(); i++ )  {
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
    if( nd.Center()->IsMetricSimilar(segnd, fom) )  {
      for( size_t j=0; j < segnd.ParentCount(); j++ )  {
        TAutoDBNetNode& netnd = segnd.GetParent(j)->Node(
          segnd.GetParentIndex(j));
        cfom = 0;
        if( nd.IsMetricSimilar(netnd, cfom, NULL, false) )  {
          found = false;
          for( size_t k=0; k < gc.list2.Count(); k++ )  {
            if( *gc.list2[k].Type == netnd.Center()->GetType() )  {
              gc.list2[k].hits.AddNew(&netnd, cfom);
              found = true;
              break;
            }
          }
          if( !found )  {
            gc.list2.AddNew(netnd.Center()->GetType(), &netnd, cfom);
          }
          if( nd.IsMetricSimilar(netnd, cfom, NULL, true) )  {
            found = false;
            for( size_t k=0; k < gc.list3.Count(); k++ )  {
              if( *gc.list3[k].Type == netnd.Center()->GetType() )  {
                gc.list3[k].hits.AddNew(&netnd, cfom);
                found = true;
                break;
              }
            }
            if( !found )
              gc.list3.AddNew(netnd.Center()->GetType(), &netnd, cfom);
          }
        }
      }
      found = false;
      for( size_t j=0; j < gc.list1.Count(); j++ )  {
        if( *gc.list1[j].Type == segnd.GetType() )  {
          gc.list1[j].hits.AddNew(&segnd, fom);
          found = true;
          break;
        }
      }
      if( !found )
        gc.list1.AddNew(segnd.GetType(), &segnd, fom);
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
void TAutoDB::A2Pemutate(TCAtom& a1, TCAtom& a2, const cm_Element& e1,
  const cm_Element& e2, double threshold)
{
  short v = CheckA2Pemutate(a1, a2, e1, e2, threshold);
  if ((v&9) != 0 ) {
    if (!a1.IsFixedType()) {
      TBasicApp::NewLogEntry() << "Skipping fixed type atoms '" <<
        a1.GetLabel() << '\'';
      return;
    }
  }
  if ((v&6) != 0 ) {
    if (!a1.IsFixedType()) {
      TBasicApp::NewLogEntry() << "Skipping fixed type atoms '" <<
        a2.GetLabel() << '\'';
      return;
    }
  }
  if( (v&1) != 0 )  {
    TBasicApp::NewLogEntry(logInfo) << "A2 assignment: " <<
      a1.GetLabel() << " -> " << e1.symbol;
    a1.SetType(e1);
  }
  if( (v&2) != 0 )  {
    TBasicApp::NewLogEntry(logInfo) << "A2 assignment: " <<
      a2.GetLabel() << " -> " << e2.symbol;
    a2.SetType(e2);
  }
  if( (v&4) != 0 )  {
    TBasicApp::NewLogEntry(logInfo) << "A2 assignment: " <<
      a2.GetLabel() << " -> " << e1.symbol;
    a2.SetType(e1);
  }
  if( (v&8) != 0 )  {
    TBasicApp::NewLogEntry(logInfo) << "A2 assignment: " <<
      a1.GetLabel() << " -> " << e2.symbol;
    a1.SetType(e2);
  }
}
//..............................................................................
short TAutoDB::CheckA2Pemutate(TCAtom& a1, TCAtom& a2, const cm_Element& e1,
  const cm_Element& e2, double threshold)
{
  const double ratio = a1.GetUiso()/(olx_abs(a2.GetUiso())+0.001);
  short res = 0;
  if( ratio > (1.0 + threshold) )  {
    if( a1.GetType() != e1 )
      res |= 1;
    if( a2.GetType() != e2 )
      res |= 2;
  }
  else if( ratio < (1.0-threshold) )  {
    if( a2.GetType() != e1 )
      res |= 4;
    if( a1.GetType() != e2 )
      res |= 8;
  }
  return res;
}
//..............................................................................
ConstTypeList<TAutoDB::TAnalysisResult> TAutoDB::AnalyseStructure(TLattice& latt)
{
  TTypeList<TAutoDB::TAnalysisResult> res;
  for( size_t i=0; i < latt.FragmentCount(); i++ )
    res.AddList(AnalyseNet(latt.GetFragment(i)));
  return res;
}
//..............................................................................
ConstTypeList<TAutoDB::TAnalysisResult> TAutoDB::AnalyseNet(TNetwork& net)  {
  TSAtomPList cas;
  TTypeList<TAutoDB::TAnalysisResult> res;
  TTypeList<TAutoDB::TGuessCount> guesses;
  TAutoDBNet* sn = BuildSearchNet(net, cas);
  for( size_t i=0; i < net.NodeCount(); i++ )
    net.Node(i).SetTag(-1);
  if( sn == NULL )  return res;
  const size_t sn_count = sn->Count();
  // for two atoms we cannot decide which one is which, for one - no reason at all :)
  res.SetCapacity(sn_count);
  for( size_t i=0; i < sn_count; i++ )  {
    sn->Node(i).SetTag(-1);
    sn->Node(i).SetId(0);
    res.AddNew().atom = &cas[i]->CAtom();
    TGuessCount& gc = guesses.AddNew();
    gc.list1.SetCapacity(12);
    gc.list2.SetCapacity(6);
    gc.list3.SetCapacity(3);
    gc.atom = &cas[i]->CAtom();
  }
  if( sn_count < 3 )  {
    if( sn_count == 2 )  { // C-O or C-N?
      if( sn->Node(0).Center()->GetDistance(0) < 1.3 )  { // C-N
        short mv = CheckA2Pemutate(cas[0]->CAtom(), cas[1]->CAtom(),
          XElementLib::GetByIndex(iCarbonIndex),
          XElementLib::GetByIndex(iNitrogenIndex), 0.2);
        if( (mv&1) != 0 )  {
          res[0].enforced.AddNew(1,
            XElementLib::GetByIndex(iCarbonIndex));
        }
        if( (mv&2) != 0 )  {
          res[1].enforced.AddNew(1,
            XElementLib::GetByIndex(iNitrogenIndex));
        }
      }
      else  { // C-O
        short mv = CheckA2Pemutate(cas[0]->CAtom(), cas[1]->CAtom(),
          XElementLib::GetByIndex(iCarbonIndex),
          XElementLib::GetByIndex(iOxygenIndex), 0.2);
        if( (mv&1) != 0 )  {
          res[0].enforced.AddNew(1,
            XElementLib::GetByIndex(iCarbonIndex));
        }
        if( (mv&2) != 0 )  {
          res[1].enforced.AddNew(1,
            XElementLib::GetByIndex(iOxygenIndex));
        }
      }
    }
    for( size_t i=0; i < sn_count; i++ )
      delete sn->Node(i).Center();
    delete sn;
    return res;
  }
  TAnalyseNetNodeTask analyseNetNodeTask(Nodes, *sn, guesses);
  OlxListTask::Run(analyseNetNodeTask, sn_count, tLinearTask, 0);
  uint16_t cindexes[MaxConnectivity];
  TArrayList<short> node_lists(sn_count);
  for( size_t i=0; i < sn_count; i++ )  {
    if( !guesses[i].list3.IsEmpty() )
      node_lists[i] = 2;
    else if ( !guesses[i].list2.IsEmpty() )
      node_lists[i] = 1;
    else
      node_lists[i] = 0;
    sn->Node(i).SetId((uint32_t)i);
  }
  // analysis of "confident", L3 and L2 atom types and Uiso
  for( size_t i=0; i < sn_count; i++ )  {
    // copy the results
    for( size_t j=0; j < guesses[i].list1.Count(); j++ )
      res[i].list1.AddNew(guesses[i].list1[j].MeanFom(), *guesses[i].list1[j].Type);
    for( size_t j=0; j < guesses[i].list2.Count(); j++ )
      res[i].list2.AddNew(guesses[i].list2[j].MeanFom(), *guesses[i].list2[j].Type);
    for( size_t j=0; j < guesses[i].list3.Count(); j++ )
      res[i].list3.AddNew(guesses[i].list3[j].MeanFom(), *guesses[i].list3[j].Type);

    if( node_lists[i] == 0 )  continue;
    TTypeList< THitList<TAutoDBNetNode> > &guessN =
      !guesses[i].list3.IsEmpty() ? guesses[i].list3 : guesses[i].list2;
    for( size_t j=0; j < guessN.Count(); j++ )
      guessN.GetItem(j).Sort();
    QuickSorter::SortSF(guessN, THitList<TAutoDBNetNode>::SortByFOMFunc);
    double cfom = 0;
    // just for sorting
    sn->Node(i).IsMetricSimilar(
      *guessN[0].hits[0].Node, cfom, cindexes, false);
    for( size_t j=0; j < sn->Node(i).Count(); j++ )  {
      if( sn->Node(i).Node(j)->GetTag() == -1 && node_lists[i] == 0 )  {
        const cm_Element &t = guessN[0].hits[0].Node->Node(
            cindexes[j])->Center()->GetType();
        res[sn->Node(i).Node(j)->GetId()].enforced.AddNew(
          guessN[0].hits[0].Fom, t);
        sn->Node(i).Node(j)->SetTag(t.index);
      }
      else  {
        const cm_Element &to = guessN[0].hits[0].Node->Node(
          cindexes[j])->Center()->GetType();
        TTypeList<Type> &en =
          res[sn->Node(i).Node(j)->GetId()].enforced;
        bool uniq = true;
        for( size_t k=0; k < en.Count(); k++ )  {
          if( en[k].type == to )  {
            uniq = false;
            en[k].fom /= 2; // VERY simple promotion
            break;
          }
        }
        if( uniq )
          en.AddNew(guessN[0].hits[0].Fom, to);
      }
    }
  }
  for( size_t i=0; i < sn_count; i++ )
    delete sn->Node(i).Center();
  delete sn;
  return res;
}
//..............................................................................
bool TAutoDB::ChangeType(TCAtom &a, const cm_Element &e) {
  if (a.GetType() == e || e == iHydrogenZ ) return false;
  if (a.IsFixedType()) {
    TBasicApp::NewLogEntry() << "Skipping fixed type atoms '" <<
      a.GetLabel() << '\'';
    return false;
  }
  bool return_any = !alg::check_connectivity(a, a.GetType());
  if (return_any || alg::check_connectivity(a, e)) {
    // extra checks for high jumpers
    if (olx_abs(a.GetType().z-e.z) > 3 ) {
      if (!alg::check_geometry(a, e))
        return false;
    }
    a.SetType(e);
    a.SetLabel(e.symbol, false);
    return true;
  }
  return false;
}
//..............................................................................
void TAutoDB::AnalyseNet(TNetwork& net, TAtomTypePermutator* permutator,
  double& Uiso, TAutoDB::AnalysisStat& stat, ElementPList* proposed_atoms)
{
  TSAtomPList cas;
  TAutoDBNet* sn = BuildSearchNet(net, cas);
  olxstr tmp;
  for( size_t i=0; i < net.NodeCount(); i++ )
    net.Node(i).SetTag(-1);
  if( sn == NULL )  return;
  const size_t sn_count = sn->Count();
  // for two atoms we cannot decide which one is which, for one - no reason at all :)
  if( sn_count < 3 )  {
    if( sn_count == 2 && // C-O or C-N?
      cas[0]->CAtom().AttachedSiteCount() == 1 &&
      cas[1]->CAtom().AttachedSiteCount() == 1)
    {
      if( sn->Node(0).Center()->GetDistance(0) < 1.3 )  { // C-N
        if (proposed_atoms == NULL ||
            (proposed_atoms->Contains(XElementLib::GetByIndex(iCarbonIndex)) &&
             proposed_atoms->Contains(XElementLib::GetByIndex(iNitrogenIndex))))
        {
        A2Pemutate(cas[0]->CAtom(), cas[1]->CAtom(),
          XElementLib::GetByIndex(iCarbonIndex),
          XElementLib::GetByIndex(iNitrogenIndex), 0.2);
        }
      }
      else  { // C-O
        if (proposed_atoms == NULL ||
            (proposed_atoms->Contains(XElementLib::GetByIndex(iCarbonIndex)) &&
             proposed_atoms->Contains(XElementLib::GetByIndex(iOxygenIndex))))
        {
          A2Pemutate(cas[0]->CAtom(), cas[1]->CAtom(),
            XElementLib::GetByIndex(iCarbonIndex),
            XElementLib::GetByIndex(iOxygenIndex), 0.2);
        }
      }
    }
    for( size_t i=0; i < sn_count; i++ )
      delete sn->Node(i).Center();
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
  TAnalyseNetNodeTask analyseNetNodeTask(Nodes, *sn, guesses);
  OlxListTask::Run(analyseNetNodeTask, sn_count, tLinearTask, 0);
  //TListIteratorManager<TAnalyseNetNodeTask> nodesAnalysis(analyseNetNodeTask, sn->Count(), tQuadraticTask);
  uint16_t cindexes[MaxConnectivity];
  uint16_t UisoCnt = 0;
  for( size_t i=0; i < sn_count; i++ )  {
    if( !guesses[i].list3.IsEmpty() )
      sn->Node(i).SetId(2);
    else if ( !guesses[i].list2.IsEmpty() )
      sn->Node(i).SetId(1);
    else
      sn->Node(i).SetId(0);
  }
  // analysis of "confident", L3 and L2 atom types and Uiso
  for( size_t i=0; i < sn_count; i++ )  {
    if( sn->Node(i).GetId() == 0 )  continue;
    tmp.SetLength(0);
    TTypeList< THitList<TAutoDBNetNode> > &guessN =
      !guesses[i].list3.IsEmpty() ? guesses[i].list3 : guesses[i].list2;
    for( size_t j=0; j < guessN.Count(); j++ )
      guessN.GetItem(j).Sort();
    QuickSorter::SortSF(guessN, THitList<TAutoDBNetNode>::SortByFOMFunc);
    tmp << guesses[i].atom->GetLabel() << ' ';
    double cfom = 0;
    // just for sorting
    sn->Node(i).IsMetricSimilar(
      *guessN[0].hits[0].Node, cfom, cindexes, false);
    for( size_t j=0; j < sn->Node(i).Count(); j++ )  {
      if( sn->Node(i).Node(j)->GetTag() == -1 &&
          sn->Node(i).Node(j)->GetId() == 0 )
      {
        sn->Node(i).Node(j)->SetTag(
          guessN[0].hits[0].Node->Node(
            cindexes[j])->Center()->GetType().index);
      }
      else  {
        int from = sn->Node(i).Node(j)->GetTag();
        int to = guessN[0].hits[0].Node->Node(
          cindexes[j])->Center()->GetType().index;
        if( from != -1 && from != to )
          TBasicApp::NewLogEntry(logInfo) << "Oups ...";
      }
    }
    if( sn->Node(i).Count() == 1 )  // normally wobly
      Uiso += guesses[i].atom->GetUiso()*3./4.;
    else
      Uiso += guesses[i].atom->GetUiso();
    UisoCnt ++;
    stat.ConfidentAtomTypes++;
  }
  if( UisoCnt != 0 )  Uiso /= UisoCnt;
  if( Uiso < 0.015 || Uiso > 0.075 )  {  // override silly values if happens
    Uiso = 0;
    UisoCnt = 0;
  }
  if( UisoCnt != 0 )  {
    TBasicApp::NewLogEntry(logInfo) <<
    "Mean Uiso for confident atom types is " << olxstr::FormatFloat(3,Uiso);
  }
  else
    TBasicApp::NewLogEntry(logInfo) << "Could not locate confident atom types";
  // assigning atom types according to L3 and L2 and printing stats
  sorted::PointerPointer<TAutoDBNetNode> processed;
  for( size_t i=0; i < sn_count; i++ )  {
    if( sn->Node(i).GetId() == 0 )  {
      if (UisoCnt==0 && proposed_atoms==NULL && !guesses[i].list1.IsEmpty()) {
        if (alg::check_connectivity(*guesses[i].atom, *guesses[i].list1[0].Type)) {
          sn->Node(i).SetTag(guesses[i].list1[0].Type->index);
        }
        continue;
      }
    }
    TTypeList<THitList<TAutoDBNetNode> > &guessN =
      !guesses[i].list3.IsEmpty() ? guesses[i].list3 : guesses[i].list2;
    const cm_Element* type = NULL;
    if (sn->Node(i).GetId() != 0) {
      tmp.SetLength(0);
      type = guessN[0].Type;
      sn->Node(i).SetTag(type->index);
      for( size_t j=0; j < guessN.Count(); j++ )  {
        tmp << guessN[j].Type->symbol << '(' <<
          olxstr::FormatFloat(2,1.0/(guessN[j].MeanFomN(1)+0.001)) << ")";
        if( (j+1) < guessN.Count() )
          tmp << ',';
      }
      }
    if( permutator == NULL || !permutator->IsActive() )  {
      // have to do it here too!
      bool searchHeavier = false, searchLighter = false;
      if( UisoCnt != 0 && Uiso != 0 )  {
        double uiso = sn->Node(i).Count() == 1
          ? guesses[i].atom->GetUiso()*3./4. : guesses[i].atom->GetUiso();
        double scale = uiso / Uiso;
        if( scale > URatio )
          searchLighter = true;
        else if( scale < 1./URatio )
          searchHeavier = true;
      }
      // use max.min 'absolute' values for elements after Si
      else if (guesses[i].atom->GetType().z > 14) {
        if (guesses[i].atom->GetUiso() > 0.05)
          searchLighter = true;
        else if (guesses[i].atom->GetUiso() < 0.005)
          searchHeavier = true;
      }
      if( searchLighter || searchHeavier )  {
        if (AnalyseUiso(*guesses[i].atom, guessN, stat, searchHeavier,
          searchLighter, proposed_atoms))
        {
          sn->Node(i).SetTag(guesses[i].atom->GetType().index);
          processed.AddUnique(&sn->Node(i));
        }
      }
      else  {
        if( type != NULL && *type != guesses[i].atom->GetType() )  {
          if( proposed_atoms != NULL )  {
            if( proposed_atoms->IndexOf(type) != InvalidIndex )  {
              if (ChangeType(*guesses[i].atom, *type))
                stat.AtomTypeChanges++;
            }
          }
          else if( BAIDelta != -1 )  {
            if( abs(type->z - guesses[i].atom->GetType().z) < BAIDelta )  {
              if (ChangeType(*guesses[i].atom, *type))
                stat.AtomTypeChanges++;
            }
          }
          else  {
            if (ChangeType(*guesses[i].atom, *type))
              stat.AtomTypeChanges++;
          }
        }
      }
    }
    TBasicApp::NewLogEntry(logInfo) << tmp;
  }
  for( size_t i=0; i < sn_count; i++ )  {
    if (sn->Node(i).GetTag() != -1 &&
        guesses[i].atom->GetType() != sn->Node(i).GetTag() &&
        processed.IndexOf(&sn->Node(i)) == InvalidIndex)
    {
      int change_evt = -1;
      cm_Element* l_elm = &XElementLib::GetByIndex(sn->Node(i).GetTag());
      // change to only provided atoms if in the guess list
      if( proposed_atoms != NULL )  {
        if( proposed_atoms->IndexOf(l_elm) != InvalidIndex )  {
          double delta_z = guesses[i].atom->GetType().z - l_elm->z;
          double ref_val = guesses[i].atom->GetUiso() - 0.0025*delta_z;
          if(  ref_val > 0.01 && ref_val < 0.075)  {
            if (ChangeType(*guesses[i].atom, *l_elm))
              change_evt = 0;
          }
        }
      }
      else  if( BAIDelta != -1 )  { // consider atom types within BAIDelta only
        if( olx_abs(guesses[i].atom->GetType().z-sn->Node(i).GetTag()) <
            BAIDelta )
        {
          if (ChangeType(*guesses[i].atom, *l_elm))
            change_evt = 1;
        }
      }
      else  {  // unrestrained assignment
        if (ChangeType(*guesses[i].atom, *l_elm))
          change_evt = 2;
      }
      if( change_evt != -1 )  {
        TBasicApp::NewLogEntry(logInfo) << "SN[" << change_evt <<
          "] assignment " << guesses[i].atom->GetLabel() << " to " <<
          l_elm->symbol;
        stat.AtomTypeChanges++;
        stat.SNAtomTypeAssignments++;
      }
    }
  }
  for( size_t i=0; i < sn_count; i++ )  {
    if( sn->Node(i).GetTag() == -1 )  {
      bool searchHeavier = false, searchLighter = false;
      if( UisoCnt != 0 && Uiso != 0 )  {
        double scale = guesses[i].atom->GetUiso() / Uiso;
        if( scale > URatio )          searchLighter = true;
        else if( scale < 1./URatio )  searchHeavier = true;
      }
      //else if( Uiso == 0 )
      //  searchLighter = true;
      if( permutator != NULL && permutator->IsActive() )
        permutator->InitAtom( guesses[i] );
      tmp.SetLength(0);
      for( size_t j=0; j < guesses[i].list1.Count(); j++ )
        guesses[i].list1[j].Sort();
      QuickSorter::SortSF(guesses[i].list1,
        THitList<TAutoDBNode>::SortByFOMFunc);
      if( !guesses[i].list1.IsEmpty() )  {
        tmp << guesses[i].atom->GetLabel() << ' ';
        const cm_Element* type = &guesses[i].atom->GetType();
        if( searchHeavier )  {
          TBasicApp::NewLogEntry(logInfo) << "Searching element heavier for "
            << guesses[i].atom->GetLabel();
          for( size_t j=0; j < guesses[i].list1.Count(); j++ )  {
            if( guesses[i].list1[j].Type->z > type->z )  {
              if( proposed_atoms != NULL )  {
                if( proposed_atoms->IndexOf(guesses[i].list1[j].Type) !=
                    InvalidIndex )
                {
                  type = guesses[i].list1[j].Type;
                  break;
                }
              }
              else if( BAIDelta != -1 )  {
                if( guesses[i].list1[j].Type->z-guesses[i].atom->GetType().z <
                    BAIDelta )
                {
                  type = guesses[i].list1[j].Type;
                  break;
                }
              }
//              else  {
//              }
            }
          }
        }
        else if( searchLighter )  {
          TBasicApp::NewLogEntry(logInfo) << "Searching element lighter for "
            << guesses[i].atom->GetLabel();
          for( size_t j=0; j < guesses[i].list1.Count(); j++ )  {
            if( guesses[i].list1[j].Type->z < type->z )  {
              if( proposed_atoms != NULL )  {
                if( proposed_atoms->IndexOf(guesses[i].list1[j].Type) !=
                    InvalidIndex )
                {
                  type = guesses[i].list1[j].Type;
                  break;
                }
              }
              else if( BAIDelta != -1 )  {
                if( guesses[i].atom->GetType().z-guesses[i].list1[j].Type->z <
                    BAIDelta )
                {
                  type = guesses[i].list1[j].Type;
                  break;
                }
              }
            }
          }
        }
        else  {
         type = NULL;  //guesses[i].list1->Item(0).BAI;
        }
        for( size_t j=0; j < guesses[i].list1.Count(); j++ )  {
          tmp << guesses[i].list1[j].Type->symbol << '(' <<
            olxstr::FormatFloat(3,1.0/(guesses[i].list1[j].MeanFom()+0.001))
            << ")" << guesses[i].list1[j].hits[0].Fom;
          if( (j+1) < guesses[i].list1.Count() )
            tmp << ',';
        }
        if( permutator == NULL || !permutator->IsActive() )  {
          if( type == NULL || *type == guesses[i].atom->GetType() )  continue;
          bool change = true;
          if( proposed_atoms != NULL )
            change = proposed_atoms->Contains(type);
          else if( BAIDelta != -1 )
            change = (olx_abs(type->z-guesses[i].atom->GetType().z) < BAIDelta);
          if (change) {
            if (ChangeType(*guesses[i].atom, *type)) {
              olx_analysis::helper::reset_u(*guesses[i].atom);
              stat.AtomTypeChanges++;
            }
          }
        }
        TBasicApp::NewLogEntry(logInfo) << tmp;
      }
    }
  }
  for( size_t i=0; i < sn_count; i++ )
    delete sn->Node(i).Center();
  delete sn;
}
//..............................................................................
void TAutoDB::ValidateResult(const olxstr& fileName, const TLattice& latt,
  TStrList& report)
{
  olxstr cifFN = TEFile::ChangeFileExt(fileName, "cif");
  report.Add("Starting analysis of ").quote() << cifFN << " on " <<
    TETime::FormatDateTime(TETime::Now());
  if( !TEFile::Exists(cifFN) )  {
    report.Add( olxstr("The cif file does not exist") );
    return;
  }
  try  {
    XFile.LoadFromFile(cifFN);
    XFile.GetLattice().CompaqAll();
  }
  catch( const TExceptionBase& exc )  {
    report.Add("Failed to load due to ").quote() <<
      exc.GetException()->GetError();
    return;
  }
  TSpaceGroup &sga = TSymmLib::GetInstance().FindSG(latt.GetAsymmUnit());
  TSpaceGroup &sgb = TSymmLib::GetInstance().FindSG(XFile.GetAsymmUnit());
  if( &sga != &sgb )  {
    report.Add("Inconsistent space group. Changed from ").quote() <<
      sgb.GetName() << " to '" << sga.GetName() << '\'';
    return;
  }
  report.Add(olxstr("Current space group is ") << sga.GetName() );
  // have to locate possible translation using 'hard' method
  TTypeList< olx_pair_t<vec3d,TCAtom*> > alist, blist;
  TTypeList< TSymmTestData > vlist;
  latt.GetUnitCell().GenereteAtomCoordinates(alist, false);
  XFile.GetUnitCell().GenereteAtomCoordinates(blist, false);
  smatd mI;
  mI.r.I();
  TSymmTest::TestDependency(alist, blist, vlist, mI, 0.01);
  vec3d thisCenter, atomCenter;
  if( vlist.Count() != 0 )  {
    TBasicApp::NewLogEntry(logInfo) << vlist[vlist.Count()-1].Count();
    if( vlist[vlist.Count()-1].Count() > (alist.Count()*0.75) )  {
      thisCenter = vlist[vlist.Count()-1].Center;
      if( !sga.IsCentrosymmetric() )
        thisCenter *= 2;
    }
  }

  int extraAtoms = 0, missingAtoms = 0;

  TAsymmUnit& au = latt.GetAsymmUnit();

  for( size_t i=0; i < XFile.GetAsymmUnit().AtomCount(); i++ )
    XFile.GetAsymmUnit().GetAtom(i).SetTag(-1);

  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetType().GetMr() < 3 )
      continue;
    atomCenter = au.GetAtom(i).ccrd();
    atomCenter -= thisCenter;
    TCAtom* ca = XFile.GetUnitCell().FindCAtom( atomCenter );
    if( ca == NULL )  {
      report.Add("Extra atom ").quote() << au.GetAtom(i).GetLabel();
      extraAtoms++;
      continue;
    }
    ca->SetTag(i);
    if( ca->GetType() != au.GetAtom(i).GetType() )  {
      report.Add("Atom type changed from ").quote() << ca->GetLabel() <<
        " to '" << au.GetAtom(i).GetLabel() << '\'';
    }
  }
  for( size_t i=0; i < XFile.GetAsymmUnit().AtomCount(); i++ )  {
    if( XFile.GetAsymmUnit().GetAtom(i).GetTag() == -1 &&
        XFile.GetAsymmUnit().GetAtom(i).GetType() != iHydrogenZ )  {
          report.Add("Missing atom ").quote() <<
            XFile.GetAsymmUnit().GetAtom(i).GetLabel();
      missingAtoms++;
    }
  }
  report.Add( olxstr("------Analysis complete with ") << extraAtoms << " extra atoms and " <<
    missingAtoms << " missing atoms-----" );
  report.Add(EmptyString());
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAtomTypePermutator::Init(ElementPList* typeRestraints)  {
  Atoms.Clear();
  TypeRestraints.Clear();
  if( typeRestraints != NULL )
    TypeRestraints.AddList(*typeRestraints);
}
//..............................................................................
void TAtomTypePermutator::ReInit(const TAsymmUnit& au)  {
  // restore old atoms, remove deleted
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    Atoms[i].Atom = au.GetLattice().GetUnitCell().FindCAtom(Atoms[i].AtomCenter);
    if( Atoms[i].Atom == NULL )
      Atoms.NullItem(i);
  }
  Atoms.Pack();
}
void TAtomTypePermutator::InitAtom(TAutoDB::TGuessCount& guess)  {
  TTypeList< TAutoDB::THitList<TAutoDBNetNode> > &list =
    (!guess.list3.IsEmpty() ? guess.list3 : guess.list2);
  if( !list.IsEmpty() || !guess.list1.IsEmpty() )  {
    TPermutation* pm = NULL;
    for( size_t i=0; i < Atoms.Count(); i++ )  {
      if( Atoms[i].Atom == guess.atom )  {
        pm = &Atoms[i];
        break;
      }
    }
    if( list.Count() == 1 || (list.IsEmpty() && guess.list1.Count() == 1) )  {
      const cm_Element* elm = (list.Count() == 1) ? list[0].Type
        : guess.list1[0].Type;
      if( guess.atom->GetType() != *elm )
        guess.atom->SetLabel(elm->symbol);
      if( pm != NULL && pm->Tries.Count() )  {
        //Atoms.Delete(pmIndex);
        pm->Tries.Clear();
        TBasicApp::NewLogEntry(logInfo) << "Converged " << guess.atom->GetLabel();
      }
      return;
    }
    if( pm == NULL )  {
      pm = &Atoms.AddNew();
      pm->AtomCenter = guess.atom->ccrd();
      pm->Atom = guess.atom;
    }
    else  {  // check if converged
      if( pm->Tries.IsEmpty() )  return;
    }
    if( list.Count() > 1 )  {
      for( size_t i=0; i < list.Count(); i++ )  {
        bool found = false;
        for( size_t j=0; j < pm->Tries.Count(); j++ )  {
          if( pm->Tries[j].GetA() == list[i].Type )  {
            pm->Tries[j].c = list[i].MeanFom();
            if( list[i].Type == &guess.atom->GetType() )
              pm->Tries[j].b = guess.atom->GetUiso();
            found = true;
            break;
          }
        }
        if( !found )  {
          pm->Tries.AddNew<const cm_Element*,double,double>(
            list[i].Type, -1, list[i].MeanFom());
          if( *list[i].Type == guess.atom->GetType() )
            pm->Tries[pm->Tries.Count()-1].b = guess.atom->GetUiso();
        }
      }
    }
    else if( guess.list1.Count() > 1 ) {
      for( size_t i=0; i < guess.list1.Count(); i++ )  {
        bool found = false;
        for( size_t j=0; j < pm->Tries.Count(); j++ )  {
          if( pm->Tries[j].GetA() == guess.list1[i].Type )  {
            pm->Tries[j].c = guess.list1[i].MeanFom();
            if( guess.list1[i].Type == &guess.atom->GetType() )
              pm->Tries[j].b = guess.atom->GetUiso();
            found = true;
            break;
          }
        }
        if( !found )  {
          pm->Tries.AddNew<const cm_Element*,double,double>(
            guess.list1[i].Type, -1, guess.list1[i].MeanFom());
          if( guess.list1[i].Type == &guess.atom->GetType() )
            pm->Tries[pm->Tries.Count()-1].b = guess.atom->GetUiso();
        }
      }
    }
  }
}
//..............................................................................
void TAtomTypePermutator::Permutate()  {
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    bool permuted = false;
    for( size_t j=0; j < Atoms[i].Tries.Count(); j++ )  {
      if( Atoms[i].Tries[j].GetB() == -1 )  {
        Atoms[i].Atom->SetLabel(olxstr(Atoms[i].Tries[j].GetA()->symbol), false);
        Atoms[i].Atom->SetType(*Atoms[i].Tries[j].GetA());
        permuted = true;
        break;
      }
    }
    if( permuted )  {
      TBasicApp::NewLogEntry(logInfo) << Atoms[i].Atom->GetLabel() <<
        " permutated";
    }
    else  {
      const cm_Element* type = NULL;
      double minDelta = 1;
      for( size_t j=0; j < Atoms[i].Tries.Count(); j++ )  {
        if( olx_sqr(Atoms[i].Tries[j].GetB()-0.025) < minDelta )  {
          type = Atoms[i].Tries[j].GetA();
          minDelta = olx_sqr(Atoms[i].Tries[j].GetB()-0.025);
        }
        TBasicApp::NewLogEntry(logInfo) << Atoms[i].Atom->GetLabel() <<
          " permutation to " << Atoms[i].Tries[j].GetA()->symbol <<
          " leads to Uiso = " << Atoms[i].Tries[j].GetB();
      }
      if( type != NULL )  {
        TBasicApp::NewLogEntry(logInfo) << "Most probable type is " <<
          type->symbol;
        if( &Atoms[i].Atom->GetType() != type )  {
          Atoms[i].Atom->SetLabel(type->symbol, false);
          Atoms[i].Atom->SetType(*type);
        }
        Atoms[i].Tries.Clear();
      }
    }
  }
}
//..............................................................................
TAutoDB &TAutoDB::GetInstance()  {
  if( Instance == NULL )  {
    TXApp &app = TXApp::GetInstance();
    olxstr fn,
      bd_fn = app.GetBaseDir() + "acidb.db";
    if (app.IsBaseDirWriteable() || !app.HasSharedDir())
      fn = bd_fn;
    else {
      fn = app.GetSharedDir() + "acidb.db";
      if (!TEFile::Exists(fn) && TEFile::Exists(bd_fn))
        TEFile::Copy(bd_fn, fn);
    }
    TEGC::AddP(Instance=new TAutoDB(*((TXFile*)app.XFile().Replicate()), app));
    if( TEFile::Exists(fn) )  {
      TEFile dbf(fn, "rb");
      Instance->LoadFromStream(dbf);
      olxstr map_fn = fn + ".map";
      if (TEFile::Exists(map_fn)) {
        dbf.Open(map_fn, "rb");
        Instance->registry.LoadMap(dbf);
      }
    }
    Instance->src_file = fn;
  }
  return *Instance;
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAutoDB::LibBAIDelta(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(BAIDelta);
  else
    BAIDelta = Params[0].ToInt();
}
//..............................................................................
void TAutoDB::LibURatio(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(URatio);
  else
    URatio = Params[0].ToDouble();
}
//..............................................................................
void TAutoDB::LibEnforceFormula(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(EnforceFormula);
  else
    EnforceFormula = Params[0].ToBool();
}
//..............................................................................
TLibrary* TAutoDB::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("ata") : name);
  lib->Register(
    new TFunction<TAutoDB>(this,  &TAutoDB::LibBAIDelta, "BAIDelta",
      fpNone|fpOne,
      "Returns/sets maximum difference between element types to promote")
  );
  lib->Register(
    new TFunction<TAutoDB>(this,  &TAutoDB::LibURatio, "URatio", fpNone|fpOne,
      "Returns/sets a ration between atom U and mean U of the confident atoms to"
      " consider promotion")
  );
  lib->Register(
    new TFunction<TAutoDB>(this,  &TAutoDB::LibEnforceFormula, "EnforceFormula",
      fpNone|fpOne,
      "Returns/sets user formula enforcement option")
  );
  return lib;
}
//..............................................................................
