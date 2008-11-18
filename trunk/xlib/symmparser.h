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
  static olxstr  MatrixToSymm(const smatd& M);
    // Transforms matrix to standard SYMM operation (INS, CIF files), using fractions of 12 for translations
  static olxstr  MatrixToSymmEx(const smatd& M);
    // Transforms standard SYMM operation (INS, CIF files) to matrix
  static bool SymmToMatrix(const olxstr& symm, smatd& M);
  // return a matrix representation of 1_555 or 1_555555 code for the unit cell
  static smatd SymmCodeToMatrixU(const class TUnitCell& UC, const olxstr &Code);
  // return a matrix representation of 1_555 or 1_555555 code for the asymmetric unit
  static smatd SymmCodeToMatrixA(const class TAsymmUnit& AU, const olxstr &Code);
  // return a matrix representation of 1_555 or 1_555555 code for the the list of matrices
  static smatd SymmCodeToMatrix(const smatd_list& ml, const olxstr &Code);
  // return a string representation of a matrix like 1_555 or 1_555555 code in dependence on
  // the length of translations; Matrix->Tag must be set to the index of the matrix in the Unit cell!!!
  static olxstr MatrixToSymmCode(const TUnitCell& UC, const smatd& M);
  static olxstr MatrixToSymmCode(const smatd_list& ml, const smatd& M);
};


EndXlibNamespace()
#endif

