/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
// TLS procedure, (c) J Haestier, 2007
#ifndef __olx_xlib_tls_H
#define __olx_xlib_tls_H
#include "xbase.h"
#include "ematrix.h"
#include "satom.h"
#include "ellipsoid.h"
#include "evalue.h"
#include "math/mmath.h"

BeginXlibNamespace()
/* References:
  V. Schomaker & K. N. Trueblood, Acta Cryst 1968, B24, 63
  B54, 507-514
*/
class TLS {
  TSAtomPList atoms;
public:
  TLS(const TSAtomPList &atoms);
  //accessors
  const mat3d& GetT() const { return Tmat; }
  const mat3d& GetL() const { return Lmat; }
  const mat3d& GetS() const { return Smat; }
  const vec3d& GetOrigin() const { return origin; }
  const vec3d& GetFoM() const { return FoM; }
  void RotateElps(const mat3d &basis) {
    mat3d basis_t = mat3d::Transpose(basis);
    for (size_t i = 0; i < newElps.Count(); i++) {
      ShelxQuad(basis*TEllipsoid::ExpandShelxQuad(newElps[i])*basis_t,
        newElps[i]);
    }
  }
  const evecd_list& GetElpList() const { return newElps; }
  const mat3d& GetRtoLaxes() const { return RtoLaxes; }
  const ematd& GetVcV() const { return TLS_VcV; }
  void printTLS(const olxstr &title="TLS matrices") const;
  void printDiff(const olxstr &title="Uobs vs Utls") const;
  void printFOM() const;

  ConstTypeList<evecd> calcUijEllipse(const TSAtomPList &atoms);
  bool calcTLS(const ematd& designM, const evecd& UijC, const ematd &weigts);
  //returns t
  TEValueD BondCorrect(const TSAtom &atom1, const TSAtom &anAtom2);
  // Extapolate TLS motion to an atom
  evecd extrapolate(const TSAtom &atom);

private:
  static evecd &ShelxQuad(const mat3d &m, evecd &dest);
  static evecd ShelxQuad(const mat3d &m) {
    evecd v(6);
    return ShelxQuad(m, v);
  }
  vec3d origin;
  /* Rotation matrix, transforms TLS to L-principle axes. Nb.
  Inverse = Transpose since orthog
  */
  mat3d RtoLaxes;
  //TLS wrt current frame: Updated through analysis
  mat3d Tmat, Lmat, Smat;

  evecd_list newElps;  //Ellipsoids calculated from TLS
  unsigned short TLSfreeParameters; // 21, To be reduced by 1 per constraint
                    //(unless enforced with Lagrange multipliers?).

  vec3d FoM;    // {R1,R2'}
  // (6T, 6L, 9S) x (6T, 6L, 9S)
  ematd TLS_VcV;
  // NOTE: To be replace when VcV matrix is available
  void UijErrors(ematd &weights);
  void createDM(ematd &designM, evecd &UijC);
  mat3d calcUijCart (const vec3d &atomPosition);
  void RotateLaxes();
  vec3d FigOfMerit(const evecd_list &Elps, const ematd &weights);
  void symS(); // Shifts origin - makes S symmetric
  void diagS(mat3d &split, mat3d &Tmatrix, mat3d &Smatrix); //Splits L axes to make S diagonal

  //helper mathods:
  int epsil(int i, int j, int k) const;
};

EndXlibNamespace()
#endif
