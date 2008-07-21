#ifndef symmparserH
#define symmparserH

#include "xbase.h"
#include "symmat.h"

BeginXlibNamespace()

class TSymmParser  {
  // compares p with values in array axes. Used in SymmToMatrix function
  static short IsAxis(const char* axes, const olxstr& p);
public:
    // Transforms matrix to standard SYMM operation (INS, CIF files)
  static olxstr  MatrixToSymm(const symmd& M);
    // Transforms standard SYMM operation (INS, CIF files) to matrix
  static bool SymmToMatrix(const olxstr& symm, symmd& M);
  // return a matrix representation of 1_555 or 1_555555 code for the unit cell
  static symmd SymmCodeToMatrixU(const class TUnitCell& UC, const olxstr &Code);
  // return a matrix representation of 1_555 or 1_555555 code for the asymmetric unit
  static symmd SymmCodeToMatrixA(const class TAsymmUnit& AU, const olxstr &Code);
  // return a matrix representation of 1_555 or 1_555555 code for the the list of matrices
  static symmd SymmCodeToMatrix(const symmd_list& ml, const olxstr &Code);
  // return a string representation of a matrix like 1_555 or 1_555555 code in dependence on
  // the length of translations; Matrix->Tag must be set to the index of the matrix in the Unit cell!!!
  static olxstr MatrixToSymmCode(const TUnitCell& UC, const symmd& M);
  static olxstr MatrixToSymmCode(const symmd_list& ml, const symmd& M);
};


EndXlibNamespace()
#endif

