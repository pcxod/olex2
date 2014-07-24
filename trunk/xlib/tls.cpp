/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

//---------------------------------------------------------------------------//
// (c) James J Haestier, 2007
// (c) Oleg V Dolomanov, 2012
//---------------------------------------------------------------------------//
#include "tls.h"
#include "satom.h"
#include "asymmunit.h"
#include "etable.h"
#include "math/align.h"
#include "math/dmatrix.h"

using namespace math;
typedef linear_from_sym<mat3d> from_sym3d;
typedef linear_to_sym<mat3d> to_sym3d;


TLS::TLS(const TSAtomPList &atoms_) : atoms(atoms_)
{
  if (atoms.IsEmpty())
    throw TInvalidArgumentException(__OlxSourceInfo, "atom list");
  TLSfreeParameters = 21;
  RtoLaxes.I();
  //Use an atom as origin (reducing numerical errors on distances)
  origin = olx_mean(atoms,
    FunctionAccessor::MakeConst<vec3d, TSAtom>(&TSAtom::crd));
  /*
  Calculate errors on Uij input - to be replaced be VcV when available from
  Olex2
  */
  ematd designM, weights;
  evecd UijCol;
  UijErrors(weights);
  /*
  currently unit weights
  Create design matrix and find TLS matrices using SVD
  dm is function of atoms' coords in Cartesian.
  */
  createDM(designM, UijCol);
  // UijCol create wrt same coord system
  // TLS wrt same frame/origin as UijCol
  calcTLS(designM, UijCol, weights);
  // Rotate TLS tensor to L principle axes: diagonalise L
  RotateLaxes();
  symS();  //Origin shift to symmetrise the S
  newElps = calcUijEllipse(atoms);
  FoM = FigOfMerit(newElps, weights);
  if (false) { // diagonalise S?
    mat3d splitAxes, Tmatrix, Smatrix;
    diagS(splitAxes,Tmatrix,Smatrix);
  }
}

void TLS::printTLS(const olxstr &title) const {
  TTTable<TStrList> tab(12, 3);
  tab[0][0] = "T";  tab[4][0] = "L";  tab[8][0] = "S";
  int sacc [9] = {0, 1, 2, 1, 3, 4, 2, 4, 5};
  for (int i=0; i < 3; i++ )  {
    for (int j=0; j < 3; j++) {
      int idx = i*3+j;
      tab[1+j][i] =
        TEValueD(GetT()[i][j], sqrt(TLS_VcV[sacc[idx]][sacc[idx]])).ToString();
      tab[5+j][i] =
        TEValueD(GetL()[i][j], sqrt(TLS_VcV[sacc[idx]+6][sacc[idx]+6])).ToString();
      tab[9+j][i] =
        TEValueD(GetS()[i][j], sqrt(TLS_VcV[idx+12][idx+12])).ToString();
    }
  }
  TBasicApp::NewLogEntry() << tab.CreateTXTList(title, false, false, ' ');
}

void TLS::printDiff(const olxstr &title) const {
  TTTable<TStrList> tab(atoms.Count()*2, 10);
  tab.ColName(0) = "Atom";
  tab.ColName(1) = "U11";
  tab.ColName(2) = "U22";
  tab.ColName(3) = "U33";
  tab.ColName(4) = "U23";
  tab.ColName(5) = "U31";
  tab.ColName(6) = "U12";
  tab.ColName(7) = "dV/A^3";
  tab.ColName(8) = "V/A^3";
  tab.ColName(9) = "100*dV/V";
  evecd Q(6);
  double UobsVolSum=0, UobsVolSum_sq=0, R1=0, R2=0;
  for (size_t i=0; i < atoms.Count(); i++) {
    const double v = atoms[i]->GetEllipsoid()->CalcVolume();
    UobsVolSum += v;
    UobsVolSum_sq += v*v;
    size_t idx = i*2;
    tab[idx][0] = atoms[i]->GetGuiLabel();
    atoms[i]->GetEllipsoid()->GetShelxQuad(Q);
    for (size_t j=0; j < 6; j++)
      tab[idx][j+1] = olxstr::FormatFloat(-3, Q[j], true);
    tab[idx+1][0] = "Utls";
    Q = GetElpList()[i];
    double v_tls = TEllipsoid(Q).CalcVolume();
    double dV = (v - v_tls);
    R1 += olx_abs(dV);
    R2 += dV*dV;
    for (size_t j=0; j < 6; j++)
      tab[idx+1][j+1] = olxstr::FormatFloat(-3, Q[j], true);
    tab[idx][7] = olxstr::FormatFloat(-3, dV, true);
    tab[idx][8] = olxstr::FormatFloat(3, v, true);
    tab[idx][9] = olxstr::FormatFloat(-3, 100*dV/v);
    tab[idx + 1][8] = olxstr::FormatFloat(3, v_tls, true);
  }
  TBasicApp::NewLogEntry() << tab.CreateTXTList(title, true, false, ' ');
  TBasicApp::NewLogEntry() << "R1(vol)=" <<
    olxstr::FormatFloat(2, 100*R1/UobsVolSum) << " %";
  TBasicApp::NewLogEntry() << "R2(vol)=" <<
    olxstr::FormatFloat(2, 100*sqrt(R2/UobsVolSum_sq)) << " %";
  TBasicApp::NewLogEntry() << "sum(V)=" <<
    olxstr::FormatFloat(3, 100*UobsVolSum, true) << " A^3";
}

void TLS::printFOM() const {
  TBasicApp::NewLogEntry() <<
    "R1, %: " << olxstr::FormatFloat(2, GetFoM()[0]*100) << ' ' <<
    "R2', %: " << olxstr::FormatFloat(2, GetFoM()[1]*100);
}

void TLS::UijErrors(ematd &weights){
// Unit weights  - should be replaced by VcV matrix when available
  size_t dm = 6*atoms.Count();
  int wght[6] = {1, 2, 2, 1, 2, 1};
  weights.Resize(dm, dm).Null();
  for (size_t i= 0; i< dm; i++) {
    weights(i,i) = wght[i%6];
  }

  //Calculating errors on Uij from errors on cellParameters.
  //This should be replaced when Variance-Covariance matrix is available from Olex

  //Algorithm:
  //  1) Create Vector of cell errors; rotate to Cartesian frame.
  //  2)
/*
  &err_a = cellParameters[6],
  &err_b = cellParameters[7],
  &err_c = cellParameters[8],
  &err_alpha = cellParameters[9]*M_PI/180,
  &err_beta = cellParameters[10]*M_PI/180,
  &err_gamma = cellParameters[11]*M_PI/180;

  ematd errors(3);
  errors[0] = err_a;errors[1] = err_b;errors[2] = err_c;
  errors =  Rcart*errors; //Errors on Cart. cell axes to 1st order (errors on angles are 2nd order)
  std::cout << "\nErrors: " << errors[0] << ", "
    << errors[1] << ", " << errors[2] <<"\n";
  Rcart.Print();*/
}

void TLS::createDM(ematd &dm, evecd &UijC) {
  dm.Resize(6*atoms.Count(),TLSfreeParameters).Null();
  UijC.Resize(6*atoms.Count()).Null();
  evecd quad(6);
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if ( atoms[i]->GetEllipsoid() == NULL ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "Isotropic atom: invalid TLS input");
    }
    vec3d r = atoms[i]->crd() - origin;
    mat3d Atls(0, r[2], -r[1], -r[2], 0, r[0], r[1], -r[0], 0);
    mat3d AtlsT = mat3d::Transpose(Atls);
    size_t idx = i*6;
  //  Utls = Tmat + Atls*Smat + mat3d::Transpose(Smat)*AtlsT + Atls*Lmat*AtlsT;
    for (int j=0; j < 6; j++)  {
      dm(idx+j, j) = 1;  // T
      from_sym3d m =
        dmat::M_x_OneSym_x_Mt(Atls, to_sym3d::get_i(j), to_sym3d::get_j(j));
      for (int k=0; k < 6; k++)
        dm(idx+k, 6+j) = m(k);
    }
    for (int j=0; j < 9; j++) {
      from_sym3d m =
        dmat::M_x_One(Atls, j/3, j%3) + dmat::One_x_M(AtlsT, j%3, j/3);
      for (int  k=0; k < 6; k++)
        dm(idx+k, 12+j) = m(k);
    }
    /************************************************************/
    atoms[i]->GetEllipsoid()->GetShelxQuad(quad);
    for (int j=0; j < 6; j++)
      UijC[idx+j]  = quad[TEllipsoid::shelx_to_linear(j)];
  }
}

bool TLS::calcTLS(const ematd &designM, const evecd &UijC,
  const ematd &weights)
{
  const size_t nColumns = designM.Elements();  //No. of columns
  const size_t mRows = designM.Vectors();  //No. of Rows

  if (TLSfreeParameters!= nColumns)
    throw TFunctionFailedException(__OlxSourceInfo,
    "TLS Failed: Number of free TLS parameters not same as design matrix width");

  if (nColumns> mRows) {
    throw TFunctionFailedException(__OlxSourceInfo,
    "TLS Failed: more tls parameters than equations."
    "\nTry adding more atoms or constraints.");
  }

  ematd m = weights.IsEmpty() ? designM : (weights*designM);
  evecd b = weights.IsEmpty() ? UijC : weights*UijC;
  math::SVD<double> svd;
  bool svdRes = svd.Decompose(m, 2, 2);
  ematd D(TLSfreeParameters, svd.u.ColCount()),
    Ds(TLSfreeParameters, TLSfreeParameters);
  for (int i = 0; i < TLSfreeParameters; i++) {
    if (svd.w(i)) {
      D(i,i) = 1.0/svd.w(i);
      Ds(i,i) = olx_sqr(D(i,i));
    }
  }
  TLS_VcV = svd.vt*Ds*ematd::Transpose(svd.vt);
  // Compute tls elements from decomposition matrices
  svd.vt.Transpose();
  svd.u.Transpose();
  ematd Ap = svd.vt*D*svd.u; // pseudo inverse
  evecd tlsElements = Ap*b;
  // normalise the VcV matrix by S(Np-Ndof)
  double ssr=0;
  for (size_t i=0; i < UijC.Count(); i++)
    ssr += olx_sqr(UijC[i] - designM[i].DotProd(tlsElements));
  TLS_VcV *= ssr/(UijC.Count()-TLSfreeParameters);
  to_sym3d tm = Tmat, lm = Lmat;
  for (size_t i=0; i < 6; i++) {
    tm(i) = tlsElements[i];
    lm(i) = tlsElements[i+6];
  }
  for (size_t i=0; i < 9; i++) {
    Smat[i/3][i%3] = tlsElements[i+12];
  }
  return svdRes;
}

void TLS::RotateLaxes() {
  //Rotates TLS tensors to L principle axes
  Lmat.EigenValues(Lmat, RtoLaxes.I());
  origin = RtoLaxes*origin;
  mat3d tm = mat3d::Transpose(RtoLaxes);
  Tmat = RtoLaxes * Tmat * tm;
  Smat = RtoLaxes * Smat * tm;
  // in general the transformation is new = J*old*JT, J - the Jacobian
  ematd J(TLSfreeParameters, TLSfreeParameters);
  for (int i=0; i < 6; i++) {
    from_sym3d m = dmat::M_x_OneSym_x_Mt(RtoLaxes,
      to_sym3d::get_i(i), to_sym3d::get_j(i));
    for (int j=0; j < 6; j++)
      J(j, i) = J(j+6, i+6) = m(j); // dTdT, // dSdS
  }
  for (int i=0; i < 9; i++) {
    mat3d m = dmat::M_x_One_x_Mt(RtoLaxes, i/3, i%3);
    for (int j=0; j < 9; j++)
      J(12+j, 12+i) = m[j/3][j%3]; // dSdS
  }
  TLS_VcV = J*TLS_VcV*ematd::Transpose(J);
}

ConstTypeList<evecd> TLS::calcUijEllipse (const TSAtomPList &atoms) {
  /* For each atom, calc U_ij from current TLS matrices */
  evecd_list Ellipsoids(atoms.Count());
  mat3d RtoLaxesT = mat3d::Transpose(RtoLaxes);  //inverse
  for( size_t i=0; i < atoms.Count(); i++ )  {
    mat3d UtlsLaxes = calcUijCart(RtoLaxes*atoms[i]->crd());
    mat3d UtlsCell = RtoLaxesT * UtlsLaxes * RtoLaxes;
    ShelxQuad(UtlsCell, Ellipsoids[i].Resize(6));
  }
  return Ellipsoids;
}

mat3d TLS::calcUijCart(const vec3d &position) {
  vec3d r = position - origin;
  mat3d Atls(0, r[2], -r[1], -r[2], 0, r[0], r[1], -r[0], 0);
  mat3d AtlsT = mat3d::Transpose(Atls);
  return Tmat + Atls*Smat + mat3d::Transpose(Smat)*AtlsT + Atls*Lmat*AtlsT;
}

vec3d TLS::FigOfMerit(const evecd_list &Elps, const ematd &weights) {
  //sets FoM = {R1,R2}
  // R1 = sum {i<j=0..3} |Uobs - Utls| / sum |Uobs|
  // R2 = Sqrt[ sum {i,j=0..3} (Uobs - Utls)^2 / sum{Uobs^2}  ]
  double sumUobs=0;
  double R1=0, R2=0, sumUobsSq=0;
  for (size_t i=0; i < atoms.Count(); i++) {
    for (int j=0; j < 6; j++) {
      double k = (j > 2 ? 2 : 1);
      double e = atoms[i]->GetEllipsoid()->GetQuad(j);
      double d = olx_abs(e - Elps[i][j]);
      R1 += d;
      sumUobs += olx_abs(e);
      R2 += k*olx_sqr(d);
      sumUobsSq += k*olx_sqr(e);
    }
  }
  R1 = R1/sumUobs;
  double chiSq = R2 / (6*atoms.Count() - TLSfreeParameters);
  R2 = sqrt(R2/sumUobsSq);
  return vec3d(R1, R2, sqrt(chiSq));
}

void TLS::symS() {
  vec3d r; //shifts of origin
  r[0] = (Smat[1][2] - Smat[2][1])/(Lmat[1][1] + Lmat[2][2]);
  r[1] = (Smat[2][0] - Smat[0][2])/(Lmat[2][2] + Lmat[0][0]);
  r[2] = (Smat[0][1] - Smat[1][0])/(Lmat[0][0] + Lmat[1][1]);
  mat3d P(0, r[2], -r[1], -r[2], 0, r[0], r[1], -r[0], 0);
  mat3d Pt = mat3d::Transpose(P);

  Tmat = Tmat + P*Lmat*Pt + P*Smat + mat3d::Transpose(Smat)*Pt;
  Smat = Smat + Lmat*Pt;
  origin = origin + r; //Origin wrt L axes
  // update VcV
  ematd J(TLSfreeParameters, TLSfreeParameters);
  for (int i=0; i < 9; i++) {
    if (i < 6)
      J(i,i) = J(6+i, 6+i) = 1; // dTdT, dLdL
    J(12+i, 12+i) = 1; // dSdS
  }
  for (size_t i=0; i < 6; i++) {
    from_sym3d dTdL =
      dmat::M_x_OneSym_x_Mt(P, to_sym3d::get_i(i), to_sym3d::get_j(i));
    mat3d dSdL
      = dmat::OneSym_x_M(Pt, to_sym3d::get_i(i), to_sym3d::get_j(i));
    for (int j=0; j < 9; j++) {
      if (j < 6)
        J(j, i+6) = dTdL(j);
      J(j+12, i+6) = dSdL[j/3][j%3];
    }
  }
  for (int i=0; i < 9; i++) {
    mat3d dTdS = dmat::M_x_One(P, i/3, i%3) + dmat::One_x_M(Pt, i%3, i/3);
    for (int j=0; j < 9; j++)
      J(j, i+12) = dTdS[j/3][j%3];
  }
  TLS_VcV = J*TLS_VcV*ematd::Transpose(J);
}

void TLS::diagS(mat3d &split, mat3d &Tmatrix, mat3d &Smatrix){
  // Calc split of L-principle axes
  for (int i=0; i<3; i++) {
    for (int j=0; j<3 ; j++) {
      for (int k =0; k <3; k++)
        split[i][j] = epsil(i,j,k)*Smat[j][k]/Lmat[j][j];
    }
  }
  // Calc change to T matrix
  mat3d temp;
  for (int i=0; i<3; i++){
    for(int j=0; j<3 ; j++){
      for (int k =0; k <3; k++){
        if(i!=j)
          temp[i][j] = temp[i][j]+ Smat[k][i]*Smat[k][j]/Lmat[k][k];
        else if(i!=k) //i==j !=k
          temp[i][j] = temp[i][j]+ Smat[k][i]*Smat[k][j]/Lmat[k][k];
        //no contrib from i=j=k
      }
      Tmatrix[i][j] = Tmat[i][j] - temp[i][j];
      temp[i][j]=0; // reset for Smat transf
    }
  }
  //Change to S matrix
  for (int i=0; i<3; i++) {
    for (int j=0; j<3 ; j++) {
      for (int m =0; m <3; m++) {
        for (int n =0; n <3; n++) {
          if( i!=j )
            temp[i][j] += epsil(m,n,j)*Lmat[i][m]*split[n][i];
             // sum ( epsil * L *split ) see TLS notes
        }
      }
    }
  }
  Smatrix = Smat + temp;
}

int TLS::epsil(int i, int j, int k) const{
  //antiSym tensor Epsilon_(i,j,k)
  //    = +1, even perms of 0,1,2
  //    = -1, odd perms
  //    = 0 for repeated index, eg 0,0,k
  if( (i==0 && j==1 && k==2) ||
      (i==1 && j==2 && k==0) ||
      (i==2 && j==0 && k==1) )
    return 1;
  else if(
      (i==1 && j==0 && k==2) ||
      (i==2 && j==1 && k==0) ||
      (i==0 && j==2 && k==1) )
    return -1;
  else if (i==j || i==k || k==j)
    return 0;  // returns zero eg for i==j > 2;
  else {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "epsilon_i,j,k not permutation of 0,1,2");
  }
}

TEValueD TLS::BondCorrect(const TSAtom &atom1, const TSAtom &atom2){
  vec3d vec = (RtoLaxes*(atom1.crd()-atom2.crd())).Abs();
   //TLS correction
  int acc[] = {6+0,6+3,6+5};
  TEValueD rv(vec.Length(), 0);
  rv.V() += (vec[0]*(Lmat[1][1]+Lmat[2][2])
           + vec[1]*(Lmat[0][0]+Lmat[2][2])
           + vec[2]*(Lmat[1][1]+Lmat[0][0]))/2;

  vec3d dBdL; //derivative: d BondCorrection / d L[i][i], i = 1,2,3
  dBdL[0] = (1./2.)*(vec[1] + vec[2]);
  dBdL[1] = (1./2.)*(vec[0] + vec[2]);
  dBdL[2] = (1./2.)*(vec[1] + vec[0]);

  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      rv.E() += dBdL[i]*dBdL[j]*TLS_VcV[acc[i]][acc[j]];
  rv.E() = sqrt(rv.GetE());
  return rv;
}

evecd TLS::extrapolate(const TSAtom &atom) {
  mat3d UtlsLaxes = calcUijCart(RtoLaxes*atom.crd());
  mat3d UtlsCell= mat3d::Transpose(RtoLaxes) * UtlsLaxes * RtoLaxes;
  return ShelxQuad(UtlsCell);
}

evecd &TLS::ShelxQuad(const mat3d &m, evecd &vec) {
  vec[0] = m[0][0];
  vec[1] = m[1][1];
  vec[2] = m[2][2];
  vec[3] = m[2][1];
  vec[4] = m[2][0];
  vec[5] = m[0][1];
  return vec;
}
