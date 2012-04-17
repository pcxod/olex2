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
  V. Schomaker, Acta Cryst 1968, B24, 63
*/
class TLS {
public:
  TLS(const TSAtomPList &atoms, const double *cellParameters);
  //accessors
  const mat3d& GetT() const { return Tmat; }
  const mat3d& GetL() const { return Lmat; }
  const mat3d& GetS() const { return Smat; }
  const vec3d& GetOrigin() const { return origin; }
  const vec3d& GetFoM() const { return FoM; }
  const evecd_list& GetElpList() const { return newElps; }
  const mat3d& GetRtoLaxes() const { return RtoLaxes; }
  const ematd& GetVcV() const { return TLS_VcV; }
  void printTLS(const olxstr &title="TLS matrices");
  
  ConstTypeList<evecd> calcUijEllipse(const TSAtomPList &atoms);
  bool calcTLS(const ematd& designM, const evecd& UijC, const ematd &weigts);
  //returns t
  TEValueD BondCorrect(const TSAtom &atom1, const TSAtom &anAtom2);
  // Extapolate TLS motion to an atom
  evecd extrapolate(const TSAtom &atom);

private:
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
  void UijErrors(const TSAtomPList &atoms, ematd &weights);
  void createDM(ematd &designM, evecd &UijC ,const TSAtomPList &atoms);
  mat3d calcUijCart (const vec3d &atomPosition);
  void RotateLaxes();
  vec3d FigOfMerit(const TSAtomPList &atoms, const evecd_list &Elps,
    const evecd &UijCol, const ematd &weights);
  void symS(); // Shifts origin - makes S symmetric
  void diagS(mat3d &split, mat3d &Tmatrix, mat3d &Smatrix); //Splits L axes to make S diagonal

  //helper mathods:
  int epsil(int i, int j, int k) const;
};

EndXlibNamespace()
#endif
