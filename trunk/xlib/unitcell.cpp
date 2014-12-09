/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "unitcell.h"
#include "asymmunit.h"
#include "lattice.h"
#include "catom.h"
#include "ellipsoid.h"
#include "network.h"
#include "xapp.h"
#include "log.h"
#include "emath.h"
#include "olxmps.h"
#include "arrays.h"
#include "etable.h"
#include "infotab.h"
#include "estopwatch.h"

#undef GetObject

TUnitCell::TUnitCell(TLattice *L)  {  Lattice = L;  }
//..............................................................................
TUnitCell::~TUnitCell()  {  Clear();  }
//..............................................................................
void TUnitCell::ClearEllipsoids()  {
  for( size_t i=0; i < Ellipsoids.Count(); i++ )
    Ellipsoids[i].DeleteItems(true);
  Ellipsoids.Clear();
}
//..............................................................................
void TUnitCell::AddEllipsoid(size_t n)  {
  size_t st = Ellipsoids.Count();
  Ellipsoids.SetCount(Ellipsoids.Count() + n);
  for( size_t i=0; i < n; i++ )
    Ellipsoids[i+st].SetCount(Matrices.Count());
}
//..............................................................................
void TUnitCell::Clear()  {
  Matrices.Clear();
  ClearEllipsoids();
}
//..............................................................................
smatd& TUnitCell::InitMatrixId(smatd& m) const {
  for( size_t i=0; i < Matrices.Count(); i++ )  {
    if( Matrices[i].r == m.r )  {
      const vec3d dt = m.t-Matrices[i].t;
      const int8_t ta = (int8_t)(m.t[0]-Matrices[i].t[0]);
      const int8_t tb = (int8_t)(m.t[1]-Matrices[i].t[1]);
      const int8_t tc = (int8_t)(m.t[2]-Matrices[i].t[2]);
      if( olx_abs(dt[0]-ta) < 1e-6 &&
          olx_abs(dt[1]-tb) < 1e-6 &&
          olx_abs(dt[2]-tc) < 1e-6 )
      {
        m.SetId((uint8_t)i, ta, tb, tc);
        return m;
      }
    }
  }
  throw TInvalidArgumentException(__OlxSourceInfo, "could not locate matrix");
}
//..............................................................................
smatd_list TUnitCell::MulMatrices(const smatd_list& in,
  const smatd& transform) const
{
  smatd_list out(in.Count());
  for( size_t i=0; i < in.Count(); i++ )  {
    out.Set(i, new smatd(transform*in[i]));
    const uint8_t index = MulDest[transform.GetContainerId()][in[i].GetContainerId()];
    const int8_t ta = (int8_t)(out[i].t[0]-Matrices[index].t[0]);
    const int8_t tb = (int8_t)(out[i].t[1]-Matrices[index].t[1]);
    const int8_t tc = (int8_t)(out[i].t[2]-Matrices[index].t[2]);
    out[i].SetId(index, ta, tb, tc);
  }
  return out;
}
//..............................................................................
double TUnitCell::CalcVolume(const vec3d& sides, const vec3d& angles)  {
  static const double k = M_PI/180;
  const vec3d cs(cos(angles[0]*k), cos(angles[1]*k), cos(angles[2]*k));
  return sides.Prod()*sqrt(1-cs.QLength() + 2*cs.Prod());
}
//..............................................................................
double TUnitCell::CalcVolume() const {
  TAsymmUnit& au = GetLattice().GetAsymmUnit();
  return CalcVolume(au.GetAxes(), au.GetAngles());
}
//..............................................................................
TEValue<double> TUnitCell::CalcVolumeEx() const {
  return CalcVolumeEx(GetLattice().GetAsymmUnit());
}
//..............................................................................
TEValue<double> TUnitCell::CalcVolumeEx(const TAsymmUnit& au) {
  static const double k = M_PI/180;
  const vec3d ang = au.GetAngles()*k;
  const vec3d ange = au.GetAngleEsds()*k;
  const vec3d ax = au.GetAxes();
  const vec3d axe = au.GetAxisEsds();
  const vec3d cs(cos(ang[0]), cos(ang[1]), cos(ang[2]));
  const vec3d ss(sin(ang[0]), sin(ang[1]), sin(ang[2]));
  const double t = sqrt(1-cs.QLength() + 2*cs.Prod());
  const double r = ax.Prod();
  const double v = r*t;
  const double esd = sqrt(
    olx_sqr(v*axe[0]/ax[0]) + olx_sqr(v*axe[1]/ax[1]) + olx_sqr(v*axe[2]/ax[2]) +
    olx_sqr(r/t*ss[0]*(cs[0]-cs[1]*cs[2])*ange[0]) +
    olx_sqr(r/t*ss[1]*(cs[1]-cs[0]*cs[2])*ange[1]) +
    olx_sqr(r/t*ss[2]*(cs[2]-cs[0]*cs[1])*ange[2])
    );
  return  TEValue<double>(v, esd);
}
//..............................................................................
void  TUnitCell::InitMatrices()  {
  volatile TStopWatch sw(__FUNC__);
  MulDest.Clear();
  Matrices.Clear();
  GenerateMatrices(Matrices, GetLattice().GetAsymmUnit(),
    GetLattice().GetAsymmUnit().GetLatt());
  const size_t mc = Matrices.Count();
  MulDest.SetCapacity(mc);
  InvDest.SetCount(mc);
  for( size_t i=0; i < mc; i++ )  {
    Matrices[i].SetId((uint8_t)i);
    MulDest.Add(new TArrayList<uint8_t>(mc));
    for( size_t j=0; j < mc; j++ )  {
      smatd m = Matrices[i]*Matrices[j];
      size_t index = InvalidIndex;
      for( size_t k=0; k < mc; k++ )  {
        if( Matrices[k].r == m.r )  {
          const vec3d t = m.t - Matrices[k].t;
          if( (t-t.Round<int>()).QLength() < 1e-6 )  {
            index = k;
            break;
          }
        }
      }
      if( index == InvalidIndex ) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "assert: incomplete space group");
      }
      MulDest[i][j] = (uint8_t)index;
    }
    {
      const smatd m = Matrices[i].Inverse();
      size_t index = InvalidIndex;
      for( size_t k=0; k < mc; k++ )  {
        if( Matrices[k].r == m.r )  {
          const vec3d t = m.t - Matrices[k].t;
          if( (t-t.Round<int>()).QLength() < 1e-6 )  {
            index = k;
            break;
          }
        }
      }
      if( index == InvalidIndex ) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "assert: incomplete space group");
      }
      InvDest[i] = (uint8_t)index;
    }
  }
  UpdateEllipsoids();
#if defined(_DEBUG) && 0
  TTable m_tab(mc, mc), i_tab(1, mc);
  for( size_t i=0; i < mc; i++ )  {
    i_tab.ColName(i) = i;
    i_tab[0][i] = InvDest[i];
    for( size_t j=0; j < mc; j++ )  {
      m_tab[i][j] = MulDest[i][j];
    }
  }
  TBasicApp::NewLogEntry() <<
    m_tab.CreateTXTList("Space group multiplication table", false, false, ' ');
  TBasicApp::NewLogEntry() <<
    i_tab.CreateTXTList("Space inversion table", true, false, ' ');
#endif
}
//..............................................................................
void TUnitCell::UpdateEllipsoids()  {
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  const size_t ac = au.AtomCount();
  const size_t mc = Matrices.Count();
  const mat3d abc2xyz(mat3d::Transpose(au.GetCellToCartesian())),
              xyz2abc(mat3d::Transpose(au.GetCartesianToCell()));
  ClearEllipsoids();
  Ellipsoids.SetCount(ac);
  for (size_t i=0; i < mc; i++)  {
    mat3d tm = abc2xyz*Matrices[i].r*xyz2abc;
    ematd J = TEllipsoid::GetTransformationJ(tm),
      Jt = ematd::Transpose(J);
    for (size_t j=0; j < ac; j++)  {
      if (i == 0)
        Ellipsoids[j].SetCount(mc);
      const TCAtom& a = au.GetAtom(j);
      if( olx_is_valid_index(a.GetEllpId()) )  {
        TEllipsoid* E = new TEllipsoid(*a.GetEllipsoid());
        E->SetId(i*mc+a.GetId());
        E->Mult(tm, J, Jt);
        Ellipsoids[j][i] = E;
      }
      else
        Ellipsoids[j][i] = NULL;
    }
  }
}
//..............................................................................
TUnitCell::TSearchSymmEqTask::TSearchSymmEqTask(TPtrList<TCAtom>& atoms,
  const smatd_list& matrices)
  : Atoms(atoms), Matrices(matrices)
{
  AU = atoms[0]->GetParent();
  Latt = &AU->GetLattice();
}
//..............................................................................
void TUnitCell::TSearchSymmEqTask::Run(size_t ind) const {
  if (Atoms[ind]->IsDeleted())  return;
  const size_t ac = Atoms.Count();
  const size_t mc = Matrices.Count();
  for (size_t i=ind+1; i < ac; i++) {
    if (Atoms[i]->IsDeleted())  continue;
    if (Atoms[i]->GetExyzGroup() != NULL &&
      Atoms[i]->GetExyzGroup() == Atoms[ind]->GetExyzGroup())
    {
      continue;
    }
    for (size_t j=0; j < mc; j++) {
      vec3d v = Atoms[ind]->ccrd() - Matrices[j] * Atoms[i]->ccrd();
      const vec3i shift = v.Round<int>();
      // collect asymetric unit bonds
      if (j == 0 && shift.IsNull()) {  // I
        AU->CellToCartesian(v);
        const double qd = v.QLength();
        if (qd < 1e-4) {
          if (Atoms[i]->GetPart() != Atoms[ind]->GetPart())  continue;
          if (Atoms[ind]->GetType() == iQPeakZ) {
            Atoms[ind]->SetDeleted(true);
            break;
          }
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[i]->SetDeleted(true);
        }
        else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], qd,
          Latt->GetDelta()))  // covalent bond
        {
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[ind]->AttachSite(Atoms[i], Matrices[j]);
          Atoms[i]->AttachSite(Atoms[ind], Matrices[j]);
        }
        else if( TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], qd,
          Latt->GetDeltaI()) )  // interaction
        {
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[ind]->AttachSiteI(Atoms[i], Matrices[j]);
          Atoms[i]->AttachSiteI(Atoms[ind], Matrices[j]);
        }
        continue;
      }
      AU->CellToCartesian(v -= shift);
      const double qd = v.QLength();
      if (qd < 1e-4) {
        if (Atoms[ind]->GetType() == iQPeakZ) {
          Atoms[ind]->SetDeleted(true);
          break;
        }
        if (Atoms[i]->GetPart() != Atoms[ind]->GetPart() ||
          Atoms[i]->GetPart() < 0)
        {
          continue;
        }
        if (Atoms[i]->GetParentAfixGroup() == NULL) {
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[i]->SetDeleted(true);
        }
      }
      else {
        smatd matr = Matrices[j];
        matr.t += shift;
        matr.SetId((uint8_t)j, shift);
        if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
          Latt->GetDelta()))
        {
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[ind]->AttachSite(Atoms[i], matr);
          Atoms[i]->AttachSite(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
        }
        else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
          Latt->GetDeltaI()))
        {
          volatile olx_scope_cs cs_(GetCriticalSection());
          Atoms[ind]->AttachSiteI(Atoms[i], matr);
          Atoms[i]->AttachSiteI(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
        }
      }
    }
    for (int ii = -1; ii <= 1; ii++) {
      for (int ij = -1; ij <= 1; ij++) {
        for (int ik = -1; ik <= 1; ik++) {
          const vec3i shift(ii, ij, ik);
          const double qd = AU->Orthogonalise(
            Atoms[ind]->ccrd() - shift - Atoms[i]->ccrd()).QLength();
          smatd matr = Matrices[0];
          matr.t += shift;
          matr.SetId(0, shift);
          if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
            Latt->GetDelta()))
          {
            volatile olx_scope_cs cs_(GetCriticalSection());
            Atoms[ind]->AttachSite(Atoms[i], matr);
            if (i != ind)
              Atoms[i]->AttachSite(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
          }
          else if (TNetwork::BondExistsQ(*Atoms[ind], *Atoms[i], matr, qd,
            Latt->GetDeltaI()))
          {
            volatile olx_scope_cs cs_(GetCriticalSection());
            Atoms[ind]->AttachSiteI(Atoms[i], matr);
            if (i != ind)
              Atoms[i]->AttachSiteI(Atoms[ind], Latt->GetUnitCell().InvMatrix(matr));
          }
        }
      }
    }
  }
}
//..............................................................................
void TUnitCell::TSearchSymmEqTask::InitEquiv() const {
  const size_t ac = Atoms.Count();
  const size_t mc = Matrices.Count();
  for( size_t i=0; i < ac; i++ )  {
    if( Atoms[i]->IsDeleted() )  continue;
    for( size_t j=1; j < mc; j++ )  {
      vec3d v = Atoms[i]->ccrd() - Matrices[j] * Atoms[i]->ccrd();
      const vec3i shift = v.Round<int>();
      AU->CellToCartesian(v -= shift);
      const double qd = v.QLength();
      if( qd < 1e-4 )  {
        smatd eqm(Matrices[j]);
        eqm.t += shift;
        eqm.SetId((uint8_t)j, shift);
        Atoms[i]->AddEquiv(eqm);
      }
      else  {
        smatd matr = Matrices[j];
        matr.t += shift;
        matr.SetId((uint8_t)j, shift);
        if( TNetwork::BondExistsQ(*Atoms[i], *Atoms[i], matr, qd,
          Latt->GetDelta()) )
        {
          Atoms[i]->AttachSite(Atoms[i], matr);
        }
        else if( TNetwork::BondExistsQ(*Atoms[i], *Atoms[i], matr, qd,
          Latt->GetDeltaI()) )
        {
          Atoms[i]->AttachSiteI(Atoms[i], matr);
        }
      }
    }
  }
}
//..............................................................................
void TUnitCell::FindSymmEq() const {
  volatile TStopWatch sw(__FUNC__);
  TStrList report;
  TCAtomPList ACA;
  ACA.SetCapacity(GetLattice().GetAsymmUnit().AtomCount());
  for( size_t i=0; i < GetLattice().GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& A1 = GetLattice().GetAsymmUnit().GetAtom(i);
    if( A1.IsDeleted() )  continue;
    A1.ClearAttachedSites();
    A1.ClearEquivs();
    if (A1.GetConnInfo().maxBonds != 0)
      ACA.Add(A1)->SetTag(0);
  }
  // searching for symmetrical equivalents; the search could be optimised by
  // removing the translational equivalents in the firts order; however the task is not
  // very common, so it should be OK. (An identity (E) matrix is in the list
  // so translational equivalents will be removed too
  if( ACA.IsEmpty() )  return;
  if( olx_abs(CalcVolume()-1.0) < 1e-3 )  {  // cartesian coordinates?
    const double delta = GetLattice().GetDelta();
    for( size_t i=0; i < ACA.Count(); i++ )  {
      for( size_t j=i+1; j < ACA.Count(); j++ )  {
        const double qd = ACA[i]->ccrd().QDistanceTo(ACA[j]->ccrd());
        if( TNetwork::BondExistsQ(*ACA[i], *ACA[j], Matrices[0], qd, delta) )  {
          ACA[i]->AttachSite(ACA[j], Matrices[0]);
          ACA[j]->AttachSite(ACA[i], Matrices[0]);
        }
      }
    }
  }
  else  {
    TSearchSymmEqTask searchTask(ACA, Matrices);
    searchTask.InitEquiv();
    OlxListTask::Run(searchTask, ACA.Count(), tQuadraticTask, 1000);
    for( size_t i=0; i < ACA.Count(); i++ )
      ACA[i]->UpdateAttachedSites();
  }
  RefinementModel *rm = GetLattice().GetAsymmUnit().GetRefMod();
  if (rm != NULL) {
    for (size_t i=0; i < rm->InfoTabCount(); i++) {
      InfoTab &it = rm->GetInfoTab(i);
      if (it.GetType() != infotab_htab) continue;
      TTypeList<ExplicitCAtomRef> ta = it.GetAtoms().ExpandList(*rm, 2);
      for (size_t j=0; j < ta.Count(); j+=2) {
        smatd m;
        if (ta[j+1].GetMatrix() != NULL) {
          m = InvMatrix(*ta[j+1].GetMatrix());
          if (ta[j].GetMatrix() != NULL)
            m = MulMatrix(*ta[j].GetMatrix(), m);
        }
        else if (ta[j].GetMatrix() != NULL)
          m = *ta[j].GetMatrix();
        else
          m = Matrices[0];
        TCAtom &dn = ta[j].GetAtom();
        for (size_t k=0; k < dn.AttachedSiteCount(); k++) {
          TCAtom::Site &s = dn.GetAttachedSite(k);
          if (s.atom->GetType() != iHydrogenZ) continue;
          smatd m1 = MulMatrix(s.matrix, m);
          ta[j+1].GetAtom().AttachSiteI(s.atom, m1);
          if (ta[j+1].GetAtom() != s.atom)
            s.atom->AttachSiteI(&ta[j+1].GetAtom(), InvMatrix(m1));
        }
      }
    }
  }
}
//..............................................................................
smatd* TUnitCell::GetClosest(const vec3d& to, const vec3d& from,
  bool ConsiderOriginal, double* dist) const
{
  const smatd* minMatr = NULL;
  const TAsymmUnit &au = GetLattice().GetAsymmUnit();
  vec3i mint;
  smatd I = smatd::Identity();
  double minD = 10000;
  if (ConsiderOriginal) {
    minD = au.Orthogonalise(from-to).QLength();
    if (dist != NULL)
      *dist = minD;
  }
  for (size_t i=0; i < Matrices.Count(); i++) {
    const smatd& matr = Matrices[i];
    vec3d v = matr * from - to;
    const vec3i shift = -v.Round<int>();
    v += shift;
    // check for identity matrix
    if( i == 0 && shift.IsNull() )  continue;
    const double D = au.CellToCartesian(v).QLength();
    if (D < minD) {
      minD = D;
      minMatr = &matr;
      mint = shift;
    }
    else {
      if (D == minD && minMatr == NULL) {
        minMatr = &matr;
        mint = shift;
      }
    }
  }
  if (minMatr != NULL) {
    smatd* retVal = new smatd(*minMatr);
    retVal->t += mint;
    retVal->SetId(minMatr->GetContainerId(), mint);
    if (dist != NULL)
      *dist = sqrt(minD);
    return retVal;
  }
  return NULL;
}
//..............................................................................
smatd TUnitCell::GetRelation(const vec3d& to, const vec3d& from) const {
  if (to.Equals(from, 1e-3)) {
    return Matrices[0];
  }
  const TAsymmUnit &au = GetLattice().GetAsymmUnit();
  for (size_t i = 0; i < Matrices.Count(); i++) {
    const smatd& matr = Matrices[i];
    vec3d v = matr * from - to;
    const vec3i shift = -v.Round<int>();
    v += shift;
    const double D = au.CellToCartesian(v).QLength();
    if (D < 1e-3) {
      smatd retVal(matr);
      retVal.t += shift;
      retVal.SetId(matr.GetContainerId(), shift);
      return retVal;
    }
  }
  throw TFunctionFailedException(__OlxSourceInfo,
    "Could not locate symmetry relation");
}
//..............................................................................
void TUnitCell::UnifyAMCList(
  TTypeList<AnAssociation3<TCAtom*, smatd, vec3d> >& res)
{
  for (size_t i=0; i < res.Count(); i++) {
    if (res.IsNull(i) || res[i].GetA()->EquivCount() == 0) continue;
    for (size_t j=i+1; j < res.Count(); j++) {
      if (res.IsNull(j)) continue;
      if (res[i].GetA() == res[j].GetA() &&
          res[i].GetC().Equals(res[j].GetC(), 1e-3))
      {
        res.NullItem(j);
      }
    }
  }
  res.Pack();
}
//..............................................................................
ConstTypeList<smatd> TUnitCell::GetBinding(const TCAtom& toA,
  const TCAtom& fromA, const vec3d& to, const vec3d& from, bool IncludeI,
  bool IncludeHBonds) const
{
  smatd_list retVal;
  smatd Im;
  vec3d i_t;
  Im.I().SetId(0);
  for( size_t i=0; i < MatrixCount(); i++ )  {
    const smatd& matr = GetMatrix(i);
    vec3d v = matr * from - to;
    const vec3i shift = -v.Round<int>();
    if (i == 0) i_t = shift;
    v += shift;
    // check for identity matrix
    if( !IncludeI && i == 0 && shift.IsNull() )  continue;
    const double qD = GetLattice().GetAsymmUnit().CellToCartesian(v).QLength();
    if( GetLattice().GetNetwork().CBondExistsQ(toA, fromA, matr, qD) )  {
      smatd& newMatr = retVal.AddNew(matr);
      newMatr.t += shift;
      newMatr.SetId(matr.GetContainerId(), shift);
    }
    else if( IncludeHBonds )  {
      if( GetLattice().GetNetwork().HBondExistsQ(toA, fromA, matr, qD) )  {
        smatd& newMatr = retVal.AddNew(matr);
        newMatr.t += shift;
        newMatr.SetId(matr.GetContainerId(), shift);
      }
    }
  }
  for( int i=-1; i <= 1; i++ )  {
    for( int j=-1; j <= 1; j++ ) {
      for( int k=-1; k <= 1; k++ )  {
        if( i == 0 && j == 0 && k == 0 ) {
          if (i_t.IsNull(1e-3))
            continue;
        }
        vec3d v(from[0] + i - to[0], from[1] + j - to[1], from[2] + k - to[2]);
        Im.t = v;
        const double qD = GetLattice().GetAsymmUnit().CellToCartesian(v).QLength();
        if( GetLattice().GetNetwork().CBondExistsQ(toA, fromA, Im, qD) )  {
          smatd& retMatr = retVal.AddNew().I();
          retMatr.t[0] += i;
          retMatr.t[1] += j;
          retMatr.t[2] += k;
          retMatr.SetId(0, i, j, k);
        }
        else if( IncludeHBonds )  {
          if( GetLattice().GetNetwork().HBondExistsQ(toA, fromA, Im, qD) )  {
            smatd& retMatr = retVal.AddNew().I();
            retMatr.t[0] += i;
            retMatr.t[1] += j;
            retMatr.t[2] += k;
            retMatr.SetId(0, i, j, k);
          }
        }
      }
    }
  }
  return retVal;
}
//..............................................................................
ConstTypeList<smatd> TUnitCell::GetInRange(const vec3d& to, const vec3d& from,
  double R, bool IncludeI) const
{
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  const double ma = olx_min(olx_min(au.GetAxes()[0], au.GetAxes()[1]),
    au.GetAxes()[2]);
  const int pr = int(R / ma) + 1, mr = -pr;
  smatd_list retVal;
  R *= R;
  for (size_t i=0; i < MatrixCount(); i++) {
    const vec3i shift = -vec3d(Matrices[i] * from - to).Round<int>();
    for (int ii = mr; ii <= pr; ii++) {
      for (int ij = mr; ij <= pr; ij++) {
        for (int ik = mr; ik <= pr; ik++) {
          const vec3i shift1(shift[0] + ii, shift[1] + ij, shift[2] + ik);
          smatd tm = Matrices[i];
          tm.t += shift1;
          const double D = au.Orthogonalise(tm*from - to).QLength();
          if (D < R && D > 0.0001) {
            retVal.AddCopy(tm).SetId(Matrices[i].GetContainerId(), shift1);
          }
        }
      }
    }
  }
  return retVal;
}
//..............................................................................
void TUnitCell::_FindInRange(const vec3d& to, double R,
  TTypeList<AnAssociation3<TCAtom*,smatd, vec3d> >& res, bool find_deleted,
  const TCAtomPList* _atoms) const
{
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  const double ma = olx_min(olx_min(au.GetAxes()[0], au.GetAxes()[1]),
    au.GetAxes()[2]);
  const int pr = int(R / ma) + 1, mr = -pr;
  const TCAtomPList& atoms = (_atoms == NULL ? au.GetAtoms() : *_atoms);
  R *= R;
  const size_t ac = atoms.Count();
  const size_t mc = Matrices.Count();
  for (size_t i=0; i < ac; i++ ) {
    const TCAtom& a = *atoms[i];
    if (a.IsDeleted() && !find_deleted)  continue;
    for (size_t j = 0; j < mc; j++) {
      const vec3i shift = -vec3d(Matrices[j] * a.ccrd() - to).Round<int>();
      for (int ii = mr; ii <= pr; ii++) {
        for (int ij = mr; ij <= pr; ij++) {
          for (int ik = mr; ik <= pr; ik++) {
            const vec3i shift1(shift[0] + ii, shift[1] + ij, shift[2] + ik);
            smatd tm = Matrices[j];
            tm.t += shift1;
            const double D = au.Orthogonalise(tm*a.ccrd() - to).QLength();
            if (D < R && D > 0.0001) {
              res.AddNew(atoms[i], tm, au.Orthogonalise(tm*a.ccrd())).b
                .SetId(Matrices[j].GetContainerId(), shift1);
            }
          }
        }
      }
    }
  }
  UnifyAMCList(res);
}
//..............................................................................
void TUnitCell::_FindBinding(const TCAtom& to, const smatd& ctm, double delta,
  TTypeList<AnAssociation3<TCAtom*,smatd, vec3d> >& res, const TCAtomPList* _atoms) const
{
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  const TCAtomPList& atoms = (_atoms == NULL ? au.GetAtoms() : *_atoms);
  const size_t ac = atoms.Count();
  const size_t mc = Matrices.Count();
  smatd I;
  I.I().SetId(0);
  vec3d i_t;
  const vec3d to_center = ctm*to.ccrd();
  for (size_t i=0; i < ac; i++) {
    const TCAtom& a = *atoms[i];
    if (a.IsDeleted())  continue;
    for (size_t j = 0; j < mc; j++) {
      const vec3i shift = -vec3d(Matrices[j] * a.ccrd() - to_center).Round<int>();
      for (int ii = -1; ii <= 1; ii++)  {
        for (int ij = -1; ij <= 1; ij++) {
          for (int ik = -1; ik <= 1; ik++)  {
            const vec3i shift1(shift[0] + ii, shift[1] + ij, shift[2] + ik);
            smatd tm = Matrices[j];
            tm.t += shift1;
            const double qD = au.Orthogonalise(tm*a.ccrd() - to_center)
              .QLength();
            if (qD > 1e-6 && TNetwork::BondExistsQ(to, a, Matrices[j], qD, delta)) {
              res.AddNew(atoms[i], tm, au.Orthogonalise(tm*a.ccrd())).b
                .SetId(Matrices[j].GetContainerId(), shift1);
            }
          }
        }
      }
    }
  }
  UnifyAMCList(res);
}
//..............................................................................
void TUnitCell::GetAtomEnviList(TSAtom& atom, TAtomEnvi& envi, bool IncludeQ,
  int part) const
{
  if (atom.GetParent() != this->GetLattice()) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "atom must belong to the same lattice as the unit cell");
  }
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  envi.SetBase(atom);
  smatd I;
  I.I().SetId(0);
  TCAtom& ca = atom.CAtom();
  for( size_t i=0; i < ca.AttachedSiteCount(); i++ )  {
    const TCAtom::Site& site = ca.GetAttachedSite(i);
    if( site.atom->IsDeleted() ||
        (!IncludeQ && site.atom->GetType() == iQPeakZ) )
    {
      continue;
    }
    const smatd m = MulMatrix(site.matrix, atom.GetMatrix());
    if( part == DefNoPart ||
        (site.atom->GetPart() == 0 || site.atom->GetPart() == part) )
    {
      envi.Add(*site.atom, m, au.Orthogonalise(m*site.atom->ccrd()));
    }
  }
}
//..............................................................................
void TUnitCell::GetAtomQEnviList(TSAtom& atom, TAtomEnvi& envi)  {
  envi.SetBase(atom);
  smatd I;
  I.I().SetId(0);
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  for( size_t i=0; i < atom.CAtom().AttachedSiteCount(); i++ )  {
    TCAtom::Site& site = atom.CAtom().GetAttachedSite(i);
    if( site.atom->IsDeleted() || site.atom->GetType() != iQPeakZ )
      continue;
    const smatd m = MulMatrix(site.matrix, atom.GetMatrix());
    const vec3d v = au.Orthogonalise(m*site.atom->ccrd());
    envi.Add(*site.atom, m, v);
  }
}
//..............................................................................
void TUnitCell::GetAtomPossibleHBonds(const TAtomEnvi& ae, TAtomEnvi& envi)  {
  envi.SetBase(ae.GetBase());
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  const double D = 3.1, qD = D*D;
  const size_t ac = au.AtomCount();
  SortedObjectList<int, TPrimitiveComparator> types;
  types.Add(iNitrogenZ);
  types.Add(iOxygenZ);
  types.Add(iFluorineZ);
  types.Add(iChlorineZ);
  types.Add(iSulphurZ);
  types.Add(iBromineZ);
  for (size_t i=0; i < ac; i++) {
    TCAtom& A = au.GetAtom(i);
    if (A.IsDeleted() || !types.Contains(A.GetType().z)) continue;
    const bool considerI =  (A != ae.GetBase().CAtom());
    smatd_list ms = GetInRange(ae.GetBase().ccrd(), A.ccrd(), D, considerI);
    for (size_t j=0; j < ms.Count(); j++) {
      const vec3d a_crd = au.Orthogonalise(ms[j] * A.ccrd());
      const double qd = a_crd.QDistanceTo(ae.GetBase().crd());
      if (qd < 2*2 || qd > qD) continue;
      if (ae.Count() == 1) {
        // 80 - 150 degrees
        const double ca = (ae.GetCrd(0) - ae.GetBase().crd()).CAngle(
          a_crd - ae.GetBase().crd());
        if (ca > 0.174 || ca < -0.866) continue;
      }
      // make sure that atoms on center of symmetry are not counted twice
      bool add = true;
      for (size_t k=0; k < envi.Count(); k++) {
        if (envi.GetCAtom(k) == A && envi.GetCrd(k).Equals(a_crd, 1e-3)) {
          add = false;
          break;
        }
      }
      if (add)
        envi.Add(A, ms[j], a_crd);
    }
  }
}
//..............................................................................
void TUnitCell::FilterHBonds(TAtomEnvi& atom, TAtomEnvi& envi, bool move) {
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  for (size_t i=0; i < envi.Count(); i++) {
    TCAtom &a = envi.GetCAtom(i);
    for( size_t ai=0; ai < a.AttachedSiteCount(); ai++) {
      TCAtom::Site &s = a.GetAttachedSite(ai);
      if (s.atom->GetType() != iHydrogenZ) continue;
      smatd m = MulMatrix(s.matrix, envi.GetMatrix(i));
      vec3d h_crd = au.Orthogonalise(m*s.atom->ccrd());
      double ca = (envi.GetBase().crd()-h_crd).CAngle(envi.GetCrd(i)-h_crd);
      if (ca < -0.34) { // 109.9 -> 180
        if (move)
          atom.Add(a, envi.GetMatrix(i), envi.GetCrd(i));
        envi.Delete(i--);
        break;
      }
    }
  }
}
//..............................................................................
TCAtom* TUnitCell::FindOverlappingAtom(const vec3d& pos, bool find_deleted,
  double delta) const
{
  TTypeList< AnAssociation3<TCAtom*,smatd,vec3d> > res;
  _FindInRange(pos, delta, res, find_deleted);
  if( res.IsEmpty() )
    return NULL;
  if( res.Count() == 1 )
    return const_cast<TCAtom*>(res[0].a);
  else  {
    const TAsymmUnit& au = GetLattice().GetAsymmUnit();
    vec3d cpos = au.Orthogonalise(pos);
    double minQD = 1000;
    size_t ri = InvalidIndex;
    for( size_t i=0; i < res.Count(); i++ )  {
      const double qd = res[i].GetC().QDistanceTo(cpos);
      if( qd < minQD )  {
        ri = i;
        minQD = qd;
      }
    }
    if( ri == InvalidIndex )
      throw TFunctionFailedException(__OlxSourceInfo, "assert here");
    return res[ri].a;
  }
}
//..............................................................................
TCAtom* TUnitCell::FindCAtom(const vec3d& center) const  {
  TTypeList< AnAssociation3<TCAtom*, smatd, vec3d> > res;
  _FindInRange(center, 0.1, res);
  if( res.IsEmpty() )
    return NULL;
  if( res.Count() == 1 )
    return res[0].a;
  throw TFunctionFailedException(__OlxSourceInfo,
    "assert, too many atoms returned");
}
//..............................................................................
void TUnitCell::BuildStructureMap_Direct(TArray3D<short>& map, double delta,
  short val, ElementRadii* radii, const TCAtomPList* _template)
{
  TBasicApp::NewLogEntry() << "Building structure map...";
  TTypeList< AnAssociation3<vec3d,TCAtom*, double> > allAtoms;
  GenereteAtomCoordinates(allAtoms, true, _template);

  const size_t da = map.Length1(),
               db = map.Length2(),
               dc = map.Length3();
  map.FastInitWith(0);
  // expand atom list to +/-1/3 of UC
  ExpandAtomCoordinates(allAtoms, 1./2);
  // precalculate the sphere/ellipsoid etc coordinates for all distinct scatterers
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  double maxR = 0;
  sorted::PrimitiveAssociation<short, double> scatterers;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    const size_t ind = scatterers.IndexOf(au.GetAtom(i).GetType().GetIndex());
    if( ind != InvalidIndex )  continue;
    const double r = TXApp::GetVdWRadius(au.GetAtom(i), radii) + delta;
    scatterers.Add(au.GetAtom(i).GetType().GetIndex(), r);
  }
  for( size_t i=0; i < allAtoms.Count(); i++ )  {
    const double sr = scatterers.Find(allAtoms[i].GetB()->GetType().GetIndex());
    if( sr > maxR )
      maxR = sr;
    allAtoms[i].c = sr*sr;
    au.CellToCartesian(allAtoms[i].a);
  }
  const size_t ac = allAtoms.Count();
  for( size_t j = 0; j < da; j ++ )  {
    const double dx = (double)j/da;
    if( (j%5) == 0 )  {
      TBasicApp::GetLog() << '\r' << j*100/da << '%';
      TBasicApp::GetInstance().Update();
    }
    for( size_t k = 0; k < db; k ++ )  {
      const double dy = (double)k/db;
      for( size_t l = 0; l < dc; l ++ )  {
        vec3d p = au.Orthogonalise(vec3d(dx, dy, (double)l/dc));
        for( size_t i=0; i < ac; i++ )  {
          if( p.QDistanceTo(allAtoms[i].GetA()) <= allAtoms[i].GetC() )  {
            map.Data[j][k][l] = val;
            break;
          }
        }
      }
    }
  }
  TBasicApp::GetLog() << '\r' << "Done" << NewLineSequence();
  TBasicApp::GetInstance().Update();
}
//..................................................................................
olx_object_ptr<TArray3D<bool> > TUnitCell::BuildAtomMask(const vec3s& dim,
  double r) const
{
  // angstrem per pixel scale
  double capp = Lattice->GetAsymmUnit().GetAxes()[2]/dim[2],
         bapp = Lattice->GetAsymmUnit().GetAxes()[1]/dim[1],
         aapp = Lattice->GetAsymmUnit().GetAxes()[0]/dim[0];
  // precalculate the sphere/ellipsoid etc coordinates for all distinct scatterers
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  const double sin_a = sin(au.GetAngles()[0]*M_PI/180);
  const double sin_b = sin(au.GetAngles()[1]*M_PI/180);
  const double sin_g = sin(au.GetAngles()[2]*M_PI/180);
  const double sr = r*r;
  int ad = olx_round(olx_max(2*r/sin_b, 2*r/sin_g)/aapp)/2;
  int bd = olx_round(olx_max(2*r/sin_a, 2*r/sin_g)/bapp)/2;
  int cd = olx_round(olx_max(2*r/sin_a, 2*r/sin_b)/capp)/2;
  TArray3D<bool>* spm = new TArray3D<bool>(-ad, ad, -bd, bd, -cd, cd);
  for( int x=-ad; x <= ad; x ++ )  {
    for( int y=-bd; y <= bd; y ++ )  {
      for( int z=-cd; z <= cd; z ++ )  {
        vec3d v = au.Orthogonalise(vec3d((double)x/dim[0], (double)y/dim[1],
          (double)z/dim[2]));
        spm->Data[x+ad][y+bd][z+cd] = (v.QLength() <= sr);
      }
    }
  }
  return spm;
}
//..................................................................................
olx_pdict<short, TArray3D<bool>*>::const_dict_type
  TUnitCell::BuildAtomMasks(const vec3s& dim, ElementRadii* radii, double delta) const
{
  // angstrem per pixel scale
  double capp = Lattice->GetAsymmUnit().GetAxes()[2]/dim[2],
         bapp = Lattice->GetAsymmUnit().GetAxes()[1]/dim[1],
         aapp = Lattice->GetAsymmUnit().GetAxes()[0]/dim[0];
  // precalculate the sphere/ellipsoid etc coordinates for all distinct scatterers
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  const double sin_a = sin(au.GetAngles()[0]*M_PI/180);
  const double sin_b = sin(au.GetAngles()[1]*M_PI/180);
  const double sin_g = sin(au.GetAngles()[2]*M_PI/180);
  olx_pdict<short, TArray3D<bool>*> scatterers;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() || au.GetAtom(i).GetType() == iQPeakZ )  continue;
    size_t ind = scatterers.IndexOf(au.GetAtom(i).GetType().GetIndex());
    if( ind != InvalidIndex )  continue;
    const double r = TXApp::GetVdWRadius(au.GetAtom(i), radii) + delta;
    const double sr = r*r;
    int ad = olx_round(olx_max(2*r/sin_b, 2*r/sin_g)/aapp);
    int bd = olx_round(olx_max(2*r/sin_a, 2*r/sin_g)/bapp);
    int cd = olx_round(olx_max(2*r/sin_a, 2*r/sin_b)/capp);
    TArray3D<bool>* spm = new TArray3D<bool>(-ad, ad, -bd, bd, -cd, cd);
    for( int x=-ad; x <= ad; x ++ )  {
      for( int y=-bd; y <= bd; y ++ )  {
        for( int z=-cd; z <= cd; z ++ )  {
          vec3d v = au.Orthogonalise(vec3d((double)x/dim[0], (double)y/dim[1],
            (double)z/dim[2]));
          spm->Data[x+ad][y+bd][z+cd] = (v.QLength() <= sr);
        }
      }
    }
    scatterers.Add(au.GetAtom(i).GetType().GetIndex(), spm);
  }
  return scatterers;
}
//..................................................................................
void TUnitCell::BuildStructureMap_Masks(TArray3D<short>& map, double delta, short val,
  ElementRadii* radii, const TCAtomPList* _template) const
{
  TTypeList< olx_pair_t<vec3d,TCAtom*> > allAtoms;
  GenereteAtomCoordinates(allAtoms, true, _template);
  const vec3s dim = map.GetSize();
  // angstrem per pixel scale
  // precalculate the sphere/ellipsoid etc coordinates for all distinct scatterers
  olx_pdict<short, TArray3D<bool>*> scatterers = BuildAtomMasks(dim, radii, delta);
  vec3i aa[8];
  for( size_t i=0; i < allAtoms.Count(); i++ )  {
    TArray3D<bool>* spm = scatterers.Get(allAtoms[i].GetB()->GetType().GetIndex());
    vec3d center = allAtoms[i].GetA()*dim;
    const index_t ad = spm->Length1()/2;
    const index_t bd = spm->Length2()/2;
    const index_t cd = spm->Length3()/2;
    for( index_t j = -ad; j <= ad; j ++ )  {
      for( index_t k = -bd; k <= bd; k ++ )  {
        for( index_t l = -cd; l <= cd; l ++ )  {
          if( !spm->Data[j+ad][k+bd][l+cd] )  continue;
          aa[0] = vec3i(olx_round(center[0]+j), olx_round(center[1]+k), olx_round(center[2]+l)); //x,y,z
          aa[1] = vec3i((int)(center[0]+j), (int)(center[1]+k), (int)(center[2]+l));  //x',y',z'
          aa[2] = vec3i(aa[1][0], aa[0][1], aa[0][2]);  // x',y,z
          aa[3] = vec3i(aa[1][0], aa[1][1], aa[0][2]);  // x',y',z
          aa[4] = vec3i(aa[1][0], aa[0][1], aa[1][2]);  // x',y,z'
          aa[5] = vec3i(aa[0][0], aa[1][1], aa[0][2]);  // x,y',z
          aa[6] = vec3i(aa[0][0], aa[1][1], aa[1][2]);  // x,y',z'
          aa[7] = vec3i(aa[0][0], aa[0][1], aa[1][2]);  // x,y,z'
          for( int ci=0; ci < 8; ci++ )  {
            for( size_t m=0; m < 3; m++ )  {
              if( aa[ci][m] < 0 )
                aa[ci][m] += (int)dim[m];
              else if( aa[ci][m] >= (int)dim[m] )
                aa[ci][m] -= (int)dim[m];
            }
            map.Data[aa[ci][0]][aa[ci][1]][aa[ci][2]] = val;
          }
        }
      }
    }
  }
  for( size_t i=0; i < scatterers.Count(); i++ )
    delete scatterers.GetValue(i);
}
//..................................................................................
void TUnitCell::BuildDistanceMap_Direct(TArray3D<short>& _map, double delta, short val,
  const ElementRadii* _radii, const TCAtomPList* _template) const
{
  TTypeList< AnAssociation3<vec3f,TCAtom*, float> > allAtoms;
  GenereteAtomCoordinates(allAtoms, true, _template);
  ExpandAtomCoordinates(allAtoms, 1./2);
  const vec3s dims = _map.GetSize();
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  sorted::PrimitiveAssociation<short, float> radii;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    const size_t ind = radii.IndexOf(au.GetAtom(i).GetType().GetIndex());
    if( ind != InvalidIndex )  continue;
    const double r = TXApp::GetVdWRadius(au.GetAtom(i), _radii) + delta;
    radii.Add(au.GetAtom(i).GetType().GetIndex(), (float)r);
  }
  for( size_t i=0; i < allAtoms.Count(); i++ )  {
    allAtoms[i].c = radii.Find(allAtoms[i].GetB()->GetType().GetIndex());
    vec3d c = allAtoms[i].GetA();
    allAtoms[i].a = au.CellToCartesian(c);
  }
  mat3f tm = au.GetCellToCartesian();
  TArray3D<float> map(0, dims[0]-1, 0, dims[1]-1, 0, dims[2]-1);
  map.InitWith(10000);
  TBuildDistanceMapTask task(tm, map.Data, dims, allAtoms);
  OlxListTask::Run(task, dims[0], tLinearTask, 0);
  task.clear_loop_data();
  float scale = (float)dims[0]/(float)Lattice->GetAsymmUnit().GetAxes()[0];
  for( size_t i=0; i < dims[0]; i++ )  {
    for( size_t j=0; j < dims[1]; j++ )  {
      for( size_t k=0; k < dims[2]; k++ )  {
        if( map.Data[i][j][k] > 0 )
          _map.Data[i][j][k] = olx_round_t<short,float>(map.Data[i][j][k]*scale);
        else
          _map.Data[i][j][k] = val;
      }
    }
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////
void TUnitCell::TBuildDistanceMapTask::init_loop_data()  {
  loop_data = new float*[7];
  loop_data[0] = new float[dims[0]];
  loop_data[1] = new float[dims[1]];
  loop_data[2] = new float[dims[1]];
  loop_data[3] = new float[dims[2]];
  loop_data[4] = new float[dims[2]];
  loop_data[5] = new float[dims[2]];
  loop_data[6] = new float[atoms.Count()];
  for( size_t i=0; i < dims[0]; i++ )  {
    loop_data[0][i] = (float)i*tm[0][0]/dims[0];
  }
  for( size_t i=0; i < dims[1]; i++ )  {
    loop_data[1][i] = (float)i*tm[1][0]/dims[1];
    loop_data[2][i] = (float)i*tm[1][1]/dims[1];
  }
  for( size_t i=0; i < dims[2]; i++ )  {
    loop_data[3][i] = (float)i*tm[2][0]/dims[2];
    loop_data[4][i] = (float)i*tm[2][1]/dims[2];
    loop_data[5][i] = (float)i*tm[2][2]/dims[2];
  }
  QuickSorter::SortSF(atoms,
    &TUnitCell::AtomsSortByDistance<AnAssociation3<vec3f,TCAtom*,float> >);
  const size_t ac = atoms.Count();
  for( size_t i=0; i < ac; i++ )
    loop_data[6][i] = atoms[i].GetA().Length();
}
void TUnitCell::TBuildDistanceMapTask::clear_loop_data()  {
  for( int i=0; i < 7; i++ )
    delete [] loop_data[i];
  delete [] loop_data;
}
void TUnitCell::TBuildDistanceMapTask::Run(size_t ind) const {
  const size_t ac = atoms.Count();
  for( size_t i = 0; i < dims[1]; i++ )  {
    for( size_t j = 0; j < dims[2]; j++ )  {
      vec3f p(loop_data[0][ind] + loop_data[1][i] + loop_data[3][j],
        loop_data[2][i] + loop_data[4][j], loop_data[5][j]);
      const float pl = p.Length();
      for( size_t k=0; k < ac; k++ )  {
        const float d = p.DistanceTo(atoms[k].GetA())-atoms[k].GetC();
        if( map[ind][i][j] > d )
          map[ind][i][j] = d;
        //map[ind][i][j] += d;
        if( d < 0 )  // inside
          break;
        if( (loop_data[6][k]-map[ind][i][j]-3) > pl )  // 3 - max rad (?)
          break;
      }
    }
  }
}
//..................................................................................
void TUnitCell::BuildDistanceMap_Masks(TArray3D<short>& map, double delta, short val,
  const ElementRadii* radii, const TCAtomPList* _template) const
{
  const vec3s dims = map.GetSize();
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  double capp = au.GetAxes()[2]/dims[2],
         bapp = au.GetAxes()[1]/dims[1],
         aapp = au.GetAxes()[0]/dims[0];
  const double sin_a = sin(au.GetAngles()[0]*M_PI/180);
  const double sin_b = sin(au.GetAngles()[1]*M_PI/180);
  const double sin_g = sin(au.GetAngles()[2]*M_PI/180);
  const int shell_res = olx_round(1./aapp);
  const double max_r = 15;
  TArrayList<vec3i_list> shells(olx_round(max_r*shell_res)+1);
  const int ad = olx_round(olx_max(max_r/sin_b, max_r/sin_g)/aapp);
  const int bd = olx_round(olx_max(max_r/sin_a, max_r/sin_g)/bapp);
  const int cd = olx_round(olx_max(max_r/sin_a, max_r/sin_b)/capp);
  for( int x=-ad; x <= ad; x++ )  {
    const double dx = (double)x/dims[0];
    for( int y=-bd; y <= bd; y++ )  {
      const double dy = (double)y/dims[1];
      for( int z=-cd; z <= cd; z++ )  {
        vec3d v(dx, dy, (double)z/dims[2]);
        au.CellToCartesian(v);
        const double r = v.Length();
        if( r < max_r )  {
          short shell_index = (short)(r*shell_res);
          shells[shell_index].AddNew(x,y,z);
        }
      }
    }
  }
  TTypeList<AnAssociation3<vec3d,TCAtom*,short> > allAtoms;
  GenereteAtomCoordinates(allAtoms, true, _template);
  // Z->R
  olx_pdict<short, short> scatterers;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    size_t ind = scatterers.IndexOf(au.GetAtom(i).GetType().GetIndex());
    if( ind != InvalidIndex )  continue;
    const double r = TXApp::GetVdWRadius(au.GetAtom(i), radii) + delta;
    scatterers.Add(au.GetAtom(i).GetType().GetIndex(), short(r*shell_res));
  }
  vec3i aa[8];
  // this builds the structure map
  for( size_t i=0; i < allAtoms.Count(); i++ )  {
    allAtoms[i].a *= dims;
    short shell_cnt = scatterers.Get(allAtoms[i].GetB()->GetType().GetIndex());
    allAtoms[i].c = shell_cnt;
    for( short j=0; j < shell_cnt; j++ )  {
      for( size_t k=0; k < shells[j].Count(); k++ )  {
        aa[0] = (allAtoms[i].GetA() + shells[j][k]).Round<int>(); //x,y,z
        aa[1] = allAtoms[i].GetA() + shells[j][k];  //x',y',z'
        aa[2] = vec3i(aa[1][0], aa[0][1], aa[0][2]);  // x',y,z
        aa[3] = vec3i(aa[1][0], aa[1][1], aa[0][2]);  // x',y',z
        aa[4] = vec3i(aa[1][0], aa[0][1], aa[1][2]);  // x',y,z'
        aa[5] = vec3i(aa[0][0], aa[1][1], aa[0][2]);  // x,y',z
        aa[6] = vec3i(aa[0][0], aa[1][1], aa[1][2]);  // x,y',z'
        aa[7] = vec3i(aa[0][0], aa[0][1], aa[1][2]);  // x,y,z'
        for( int ci=0; ci < 8; ci++ )  {
          for( size_t m=0; m < 3; m++ )  {
            if( aa[ci][m] < 0 )
              aa[ci][m] += (int)dims[m];
            else if( aa[ci][m] >= (int)dims[m] )
              aa[ci][m] -= (int)dims[m];
          }
          map.Data[aa[ci][0]][aa[ci][1]][aa[ci][2]] = val;
        }
      }
    }
  }
  short shell_expansion=0;
  bool changes = true;
  while( changes )  {
    changes = false;
    const short map_val = shell_expansion;//*shell_res;
    size_t unused_atoms=0;
    for( size_t i=0; i < allAtoms.Count(); i++ )  {
      const short shell_cnt = allAtoms[i].GetC();
      const short shell_num = shell_cnt+shell_expansion;
      short atom_used = false;
      for( size_t j=0; j < shells[shell_num].Count(); j++ )  {
        vec3i crd = allAtoms[i].GetA() + shells[shell_num][j];
        for( size_t k=0; k < 3; k++ )  {
          // cannot use if/else here - in some silly cases (like one atom in the AU) this can cause crashes
          while( crd[k] < 0 )
            crd[k] += (int)dims[k];
          while( crd[k] >= (int)dims[k] )
            crd[k] -= (int)dims[k];
        }
        short& cv = map.Data[crd[0]][crd[1]][crd[2]];
        if( cv > map_val )  {
          cv = map_val;
          atom_used = changes = true;
        }
      }
      if( !atom_used )  {
        allAtoms.NullItem(i);
        unused_atoms++;
      }
    }
    if( unused_atoms > 0 )
      allAtoms.Pack();
    if( size_t(shell_expansion++) >= shells.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, "too large voids for this procedure");
  }
}
//..................................................................................
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void TUnitCell::LibVolumeEx(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(CalcVolumeEx().ToString());
}
//..............................................................................
void TUnitCell::LibCellEx(const TStrObjList& Params, TMacroData& E)  {
  const TAsymmUnit& au = Lattice->GetAsymmUnit();
  if( Params[0].Equalsi('a') )
    E.SetRetVal(TEValueD(au.GetAxes()[0], au.GetAxisEsds()[0]).ToString());
  else if( Params[0].Equalsi('b') )
    E.SetRetVal(TEValueD(au.GetAxes()[1], au.GetAxisEsds()[1]).ToString());
  else if( Params[0].Equalsi('c') )
    E.SetRetVal(TEValueD(au.GetAxes()[2], au.GetAxisEsds()[2]).ToString());
  else if( Params[0].Equalsi("alpha") )
    E.SetRetVal(TEValueD(au.GetAngles()[0], au.GetAngleEsds()[0]).ToString());
  else if( Params[0].Equalsi("beta") )
    E.SetRetVal(TEValueD(au.GetAngles()[1], au.GetAngleEsds()[1]).ToString());
  else if( Params[0].Equalsi("gamma") )
    E.SetRetVal(TEValueD(au.GetAngles()[2], au.GetAngleEsds()[2]).ToString());
  else
    E.ProcessingError(__OlxSrcInfo, "invalid argument");
}
//..............................................................................
void TUnitCell::LibMatrixCount(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(Matrices.Count());
}
//..............................................................................
//..............................................................................
//..............................................................................
IOlxObject *TUnitCell::VPtr::get_ptr() const {
  return &TXApp::GetInstance().XFile().GetUnitCell();
}
//..............................................................................
//..............................................................................
//..............................................................................
TLibrary* TUnitCell::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("uc") : name);
  olx_vptr<TUnitCell> thip(new VPtr);
  lib->Register(
    new TFunction<TUnitCell>(thip, &TUnitCell::LibVolumeEx, "VolumeEx",
      fpNone,
    "Returns unit cell volume with esd") );
  lib->Register(
    new TFunction<TUnitCell>(thip, &TUnitCell::LibCellEx, "CellEx",
      fpOne,
    "Returns unit cell side/angle with esd") );
  lib->Register(
    new TFunction<TUnitCell>(thip, &TUnitCell::LibMatrixCount, "MatrixCount",
      fpNone,
    "Returns the number of matrices in the unit cell") );
  return lib;
}
//..............................................................................
