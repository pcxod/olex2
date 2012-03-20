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
#include "math/mmath.h"
#include "etable.h"

/*
TLS module:

*/

TLS::TLS(const TSAtomPList &atoms, const double *cellParameters) {
  if (atoms.IsEmpty())
    throw TInvalidArgumentException(__OlxSourceInfo, "atom list");
  /* Calculates TLS fit for group of atoms including 
    - intialise designMatrix for SVD
    - TLS matrices optimised by SVD to fit UijColumn
  // Input: 
  //   atoms is a list of atoms in rigid body (postion, adp)
  //   cellParameters[12], (a,b,c,alpha,beta,gamma, "6 errors in same order") in Ang/degrees
  //   atomsTLS = copy of atoms to be overwritten with Uij calc from TLS
  // Output: atomsTLS with Uij calc from TLS.

  //std::cout << "\n a is: " << cellParameters[0] << std::endl; //verbose check

  */

  TLSfreeParameters = 21;
  
  RtoLaxes.I();

  //Use an atom as origin (reducing numerical errors on distances)
  for (size_t i=0; i < atoms.Count(); i++)
    origin += atoms[i]->crd();
  origin /= atoms.Count();


  /*
  Calculate errors on Uij input - to be replaced be VcV when available from
  Olex2
  */
  ematd designM, weights;
  evecd UijCol;
  //UijErrors(atoms, weights);
  /*
  currently unit weights
  Create design matrix and find TLS matrices using SVD
  dm is function of atoms' coords in Cartesian.
  */
  createDM(designM, UijCol, atoms);
  // UijCol create wrt same coord system
  // TLS wrt same frame/origin as UijCol
  calcTLS(designM, UijCol, weights);
  /* Analysis:
    1) rotate to L- principle axes
    2) shift origin; S symmetrix
    3) split axes; S diagonal
  */
  // Rotate TLS tensor to L principle axes: diagonalise L
  RotateLaxes();
  //Origin shift
  symS();
  //Initialise newElps to TLS for atoms
  newElps = calcUijEllipse(atoms);
  //Independant of coordinates, calculated with Uij in cartesian
  FigOfMerit (atoms, newElps, UijCol, weights);

  //{
  //  mat3d splitAxes, Tmatrix, Smatrix;
  //  diagS(splitAxes,Tmatrix,Smatrix);
  //}
}
void TLS::printTLS(const olxstr &title){
  TTTable<TStrList> tab(12, 3);
  tab[0][0] = "T";  tab[4][0] = "L";  tab[8][0] = "S";
  for( size_t i=0; i < 3; i++ )  {
    for( size_t j=0; j < 3; j++ )  {
      if (olx_abs(GetT()[i][j]) < 1e-16) Tmat[i][j] = 0;
      if (olx_abs(GetL()[i][j]) < 1e-16) Lmat[i][j] = 0;
      if (olx_abs(GetS()[i][j]) < 1e-16) Smat[i][j] = 0;
    }
  }
  for( size_t i=0; i < 3; i++ )  {
    tab[1][i]  = olxstr::FormatFloat(-3, GetT()[i][0], true);
    tab[2][i]  = olxstr::FormatFloat(-3, GetT()[i][1], true);
    tab[3][i]  = olxstr::FormatFloat(-3, GetT()[i][2], true);
    tab[5][i]  = olxstr::FormatFloat(-3, GetL()[i][0], true);
    tab[6][i]  = olxstr::FormatFloat(-3, GetL()[i][1], true);
    tab[7][i]  = olxstr::FormatFloat(-3, GetL()[i][2], true);
    tab[9][i]  = olxstr::FormatFloat(-3, GetS()[i][0], true);
    tab[10][i] = olxstr::FormatFloat(-3, GetS()[i][1], true);
    tab[11][i] = olxstr::FormatFloat(-3, GetS()[i][2], true);
  }
  TBasicApp::NewLogEntry() << tab.CreateTXTList(title, false, false, ' ');
}
void TLS::UijErrors(const TSAtomPList &atoms, ematd &weights){
// Unit weights  - should be replaced by VcV matrix when available
  size_t dm = 6*atoms.Count();
  weights.Resize(dm, dm).Null();
  for (size_t i= 0; i< dm; i++)
    weights(i,i) = 10000; //weight = 1/sigma^2, sigma =0.01 Ang. 

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

void TLS::createDM(ematd &dm, evecd &UijC , const TSAtomPList &atoms) {
  //TLS (6n x 21) design matrix
  dm.Resize(6*atoms.Count(),TLSfreeParameters).Null();
  //6n vector of 'observed' Uij
  UijC.Resize(6*atoms.Count()).Null();
  evecd quad(6);
  for( size_t i=0; i < atoms.Count(); i++ )  {  
    if ( atoms[i]->GetEllipsoid() == NULL ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "Isotropic atom: invalid TLS input"); 
    }
    double x = atoms[i]->crd()[0] - origin[0];
    double y = atoms[i]->crd()[1] - origin[1];
    double z = atoms[i]->crd()[2] - origin[2];
    size_t idx = i*6;
    for (size_t j=0; j < 6; j++)  dm(idx+j, j) = 1;  // T
    dm[idx][10] = 2*z;      // 2z
    dm[idx][14] = z*z;      // z^2
    dm[idx][15] = -2*y;      // -2y
    dm[idx][19] = -2*y*z;    // -2yz  
    dm[idx][20] = y*y;      // y^2
    //U12
    dm[idx+1][6] = -z;    //-z
    dm[idx+1][11] = z;    //z
    dm[idx+1][13] = -z*z;  //-z^2
    dm[idx+1][15] = x;    //x
    dm[idx+1][16] = -y;    //-y
    dm[idx+1][18] = y*z;  //yz
    dm[idx+1][19] = x*z;  //xz
    dm[idx+1][20] = -x*y;  //-xy
    //U22
    dm[idx+2][7] = -2*z;  //-2z
    dm[idx+2][9] = z*z;    //z^2
    dm[idx+2][16] = 2*x;  //2x
    dm[idx+2][18] = -2*x*z; //-2xz  
    dm[idx+2][20] = x*x;  //x^2
    //U13
    dm[idx+3][6] = y;      //y
    dm[idx+3][10] = -x;      //-x
    dm[idx+3][12] = z;      //z
    dm[idx+3][13] = y*z;    //yz
    dm[idx+3][14] = -x*z;    //-xz
    dm[idx+3][17] = -y;      //-y
    dm[idx+3][18] = -y*y;    //-y^2
    dm[idx+3][19] = x*y;    //xy
    //U23
    dm[idx+4][7] = y;      //y
    dm[idx+4][8] = -z;      //-z
    dm[idx+4][9] = -y*z;    //-yz
    dm[idx+4][11] = -x;      //-x
    dm[idx+4][13] = x*z;    //xz
    dm[idx+4][17] = x;      //x
    dm[idx+4][18] = x*y;    //xy
    dm[idx+4][19] = -x*x;    //-x^2
    //U33
    dm[idx+5][8] = 2*y;      //2y
    dm[idx+5][9] = y*y;      //y^2
    dm[idx+5][12] = -2*x;    //-2x
    dm[idx+5][13] = -2*x*y;    //-2xy  
    dm[idx+5][14] = x*x;    //x^2
    /************************************************************/
    atoms[i]->GetEllipsoid()->GetQuad(quad);
    UijC[idx]   = quad[0];  //U11 in cartesian
    UijC[idx+1] = quad[5];  //U21
    UijC[idx+2] = quad[1];  //U22
    UijC[idx+3] = quad[4];  //U31
    UijC[idx+4] = quad[3];  //U32
    UijC[idx+5] = quad[2];  //U33
      
  } //i loop
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
  ematd V = svd.vt;
  calcTLS_VcV(svd.w, V.Transpose());
  // Compute tls elements from decomposition matrices
  svd.vt.Transpose();
  svd.u.Transpose();
  ematd D(TLSfreeParameters, svd.u.ColCount());
  for (int i = 0; i < TLSfreeParameters; i++){
    if ( svd.w(i) ) 
      D(i,i) = 1.0/svd.w(i);
  }
  ematd Ap = svd.vt*D*svd.u; // pseudo inverse
  evecd tlsElements = Ap*b;
  //(t11, t12,t22,t13,t23,t33,s11,s12,s13,l11,s21,s22,s23,l12,l22,s31,s32,s33,l13,l23,l33)
  Tmat[0][0] = tlsElements[0];
  Tmat[1][0] = Tmat[0][1] = tlsElements[1];
  Tmat[0][2] = Tmat[2][0] = tlsElements[3];
  Tmat[1][1] = tlsElements[2];
  Tmat[1][2] = Tmat[2][1] = tlsElements[4];
  Tmat[2][2] = tlsElements[5];

  Lmat[0][0] = tlsElements[9];
  Lmat[1][0] = Lmat[0][1] = tlsElements[13];
  Lmat[0][2] = Lmat[2][0] = tlsElements[18];
  Lmat[1][1] = tlsElements[14];
  Lmat[1][2] = Lmat[2][1] = tlsElements[19];
  Lmat[2][2] = tlsElements[20];

  Smat[0][0] =  tlsElements[6];
  Smat[0][1] = tlsElements[7];
  Smat[0][2] = tlsElements[8];
  Smat[1][0] = tlsElements[10];
  Smat[1][1] =  tlsElements[11];
  Smat[1][2] = tlsElements[12];
  Smat[2][0] = tlsElements[15];
  Smat[2][1] = tlsElements[16];
  Smat[2][2] =  tlsElements[17];
  return svdRes;
}

void TLS::RotateLaxes(){
  //Rotates TLS tensors to L principle axes and records
  //rotation matrix, RtoLaxes.
  Lmat.EigenValues(Lmat, RtoLaxes.I()); //diagonalise Lmatrix
  //compute L variance-covariance matrix
  calcL_VcV();
  origin = RtoLaxes*origin;
  mat3d tm = mat3d::Transpose(RtoLaxes);
  Tmat = RtoLaxes * Tmat * tm;
  Smat = RtoLaxes * Smat * tm;
}

void TLS::calcL_VcV(){
// VcV_L = sum_(m,n,p,q) d L_Laxes(i,i) / d L_cart(m,n) * d L_Laxes(j,j) / d L_cart(p,q) *VcVLCart(mn,pq)

  //compute VcV_L_cart from TLS_VcV_cart  
  int l_acc [] = {9, 14, 20, 19, 18, 13};
  ematd VcV(6,6); //order: l11,l22,l33,l23,l13,l12 (shelx)
  for (int i=0; i<6; i++) {
    for (int j=0; j<6; j++) {
      VcV[i][j] = TLS_VcV[l_acc[i]][l_acc[j]];
    }
  }
  ematd diff(3,6);
  //differiential matrix,diff[i][j] dL_laxes[i][i]/dLcartesian[j], j= 11,22,33,32,31,21 (shelx)
  for(short i=0; i<3; i++){  //ith eigenvalue
      mat3d temp;
      for(short m=0; m<3; m++){
        for(short n=m; n<3; n++){ //symmetric - only calc 6 elements
          temp[m][n] = RtoLaxes[i][m]*RtoLaxes[i][n]; 
        }
      }
      diff[i][0]= temp[0][0];
      diff[i][1]= temp[1][1];
      diff[i][2]= temp[2][2];
      diff[i][3]= temp[1][2];
      diff[i][4]= temp[0][2];
      diff[i][5]= temp[0][1];
  }  

  mat3d VcVtemp; //Not 6x6: Only 3 diagonal L_Laxes values
  for(short i=0; i<3; i++){
    for(short j=0; j<3; j++){ // for Covariance [L(i,i),L(j,j)]
      for(short m=0; m<6; m++){
        for(short n=0; n<6; n++){ 
//VCV (L(i),L_L(j))= dL_L(i)/dL_Cart(m) * dL_L(j)/ d LCart(n) *VCV(Lcart,Lcart)
          VcVtemp[i][j] =  VcVtemp[i][j] + diff[i][m]*diff[j][n] *VcV[m][n]; 
        }
      }
      if (i!=j){
        VcVtemp[i][j] = 2*VcVtemp[i][j]; 
        // off-diag elements count twice due to symmetry, [i][j] = [j][i]
      }
    }
  }
  LVcV=VcVtemp;
}
ConstTypeList<evecd>  TLS::calcUijEllipse (const TSAtomPList &atoms) {
  /* For each atom, calc U_ij from current TLS matrices */
  evecd_list Ellipsoids;
  mat3d RtoLaxesInv = RtoLaxes.Inverse();  //inverse
  mat3d RtoLaxesInvT = mat3d::Transpose(RtoLaxesInv); //inverse Transposed
  for( size_t i=0; i < atoms.Count(); i++ )  {  
    // for each atom, calculate Utls in Cart
    
    TSAtom* theAtom = atoms[i];
    //Atom coords wrt origin, as fraction of unit cell axes

    vec3d position = RtoLaxes*atoms[i]->crd();
    mat3d UtlsLaxes = calcUijCart(position);
    
// intially RtoLaxesInv = 1 before TLS matrixes are rotated
    mat3d UtlsCell = RtoLaxesInv * UtlsLaxes * RtoLaxesInvT;

    /*
    std::cout << "\nUtlsLaxes";
    UtlsLaxes.Print();
    */

    //Add to ellipse list
    evecd vec(6);
    vec[0]=  UtlsCell[0][0];
    vec[1]=  UtlsCell[1][1];
    vec[2]=  UtlsCell[2][2];
    vec[3]=  UtlsCell[2][1];
    vec[4]=  UtlsCell[2][0];
    vec[5]=  UtlsCell[0][1];
    Ellipsoids.AddCopy(vec);
  }
  return Ellipsoids;
}

mat3d TLS::calcUijCart(const vec3d &position){
  vec3d p = position - origin;
  mat3d Atls;
  Atls[0][1] = p[2];  Atls[1][0] = -p[2];
  Atls[0][2] = -p[1]; Atls[2][0] = p[1];
  Atls[1][2] = p[0];  Atls[2][1] = -p[0];
  mat3d AtlsT = mat3d::Transpose(Atls);
  return Tmat + Atls*Smat + mat3d::Transpose(Smat)*AtlsT + Atls*Lmat*AtlsT;
}

void TLS::calcTLS_VcV(evecd &w, ematd &v){
  //takes references to SVD matrix, w ,V
  //calculates Variance-covariance of output vector (TLS elements)
  ematd VcV(TLSfreeParameters,TLSfreeParameters);
  for (int j = 0; j < TLSfreeParameters; j++){
    for (int k = 0; k < TLSfreeParameters; k++){
      for (int i = 0; i < TLSfreeParameters; i++){
        if (w[i]) { //sum_i v(j,i) v(k,i)/ w(i)^2
          VcV[j][k] += (v[j][i]*v[k][i]/(w[i]*w[i]));
        }
      }
    }
  }
  TLS_VcV = VcV;
}
void TLS::FigOfMerit(const TSAtomPList &atoms, const evecd_list &Elps,
  const evecd &UijCol, const ematd &weights)
{  
  //sets FoM = {R1,R2,Sqrt[chi^2]}
  // R1 = sum |Uobs - Utls| / sum |Uobs|
  // R2 = Sqrt[ sum {weight_Uobs (Uobs - Utls)^2} / sum{weigh_Uobs Uobs^2}  ]
  // Sqrt[chi^2] = Sqrt[ sum {weight_Uobs (Uobs - Utls)^2}/ n ], n dof

  evecd diff(6* atoms.Count());
  for (size_t i =0; i < atoms.Count(); i++){
    diff[6*i+0] = UijCol[6*i+0] - Elps[i][0];  // Uobs - Utls in Cartesian
    diff[6*i+1] = UijCol[6*i+1] - Elps[i][5];
    diff[6*i+2] = UijCol[6*i+2] - Elps[i][1];
    diff[6*i+3] = UijCol[6*i+3] - Elps[i][4];
    diff[6*i+4] = UijCol[6*i+4] - Elps[i][3];
    diff[6*i+5] = UijCol[6*i+5] - Elps[i][2];
  }

  //R1
  double sumUobs=0;
  double R1 = 0;
  for (size_t i=0; i<6*atoms.Count(); i++){
    R1 += olx_abs(diff[i]);
    sumUobs += olx_abs(UijCol[i]);
  }
  R1 = R1/sumUobs;

  //R2
  double R2=0;
  double sumUobsSq = 0;
  for (size_t i =0; i<6*atoms.Count(); i++){
    double wght = weights.IsEmpty() ? 1.0 : weights[i][i];
    R2 += wght*olx_sqr(diff[i]);
    sumUobsSq += wght*olx_sqr(UijCol[i]);
  }

  double chiSq = R2 / (6*atoms.Count() - TLSfreeParameters);

  R2 = sqrt(R2/sumUobsSq) ;

  evecd FoMtemp(3);
  FoMtemp[0] = R1;
  FoMtemp[1] = R2;
  FoMtemp[2] = sqrt(chiSq);
  FoM = FoMtemp;
}

void TLS::symS(){
  vec3d rho; //shifts of origin
  
  for(int i =0; i<3; i++){
    int j,k;
    if(i==0){ j=1;  k=2;   }
    else if(i==1){   j=2;k=0; }
    else if(i==2){ j=0; k=1; }
    rho[i] = (Smat[j][k] - Smat[k][j]) / (Lmat[j][j] + Lmat[k][k]);
  }

  mat3d P;
  P[0][1] = rho[2]; P[1][0]=-rho[2];
  P[1][2] = rho[0]; P[2][1]=-rho[0];
  P[2][0] = rho[1]; P[0][2]=-rho[1];
  mat3d Pt = mat3d::Transpose(P);

  Tmat = Tmat + P*Smat + mat3d::Transpose(Smat)*Pt + P*Lmat*Pt;
  Smat = Smat + Lmat*Pt;
  origin = origin + rho; //Origin wrt L axes
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
      rv.E() += dBdL[i]*dBdL[j]*LVcV[i][j];
  rv.E() = sqrt(rv.GetE());
  return rv;
}

evecd TLS::extrapolate(const TSAtom &atom) {
  mat3d UtlsLaxes = calcUijCart(RtoLaxes*atom.crd());
  mat3d UtlsCell= mat3d::Transpose(RtoLaxes) * UtlsLaxes * RtoLaxes;
  evecd vec(6);
  vec[0]=  UtlsCell[0][0];
  vec[1]=  UtlsCell[1][1];
  vec[2]=  UtlsCell[2][2];
  vec[3]=  UtlsCell[2][1];
  vec[4]=  UtlsCell[2][0];
  vec[5]=  UtlsCell[0][1];
  return vec;
}

//int TLS::tlsToShelx(int i) const {
//  /*recall TLS elements order:  
//  (t11,t12,t22,t13,t23,t33,
//   s11,s12,s13,l11,s21,s22,
//   s33,l12,l22,s31,s32,s33,
//   l13,l23,l33) 
//  //shelx order: l11,l22,l33,l32,l31,l21
//  */
//  switch(i){
//  case 0: 
//    return 9; //l11
//  case 1:
//    return 14; //l22
//  case 2:
//    return 20; //l33      
//    break;
//  case 3:
//    return 19;
//  case 4:
//    return 18;
//  case 5:
//    return 13;
//  default:
//    throw TInvalidArgumentException(__OlxSourceInfo, 
//      "TLS parameters (l11,l22,l33,l23,l13,l12 etc) conversion to shelx convention: failed");   
//  }
//}
