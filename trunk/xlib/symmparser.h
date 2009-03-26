#ifndef symmparserH
#define symmparserH

#include "xbase.h"
#include "symmat.h"
#include "emath.h"

BeginXlibNamespace()

class TSymmParser  {
  struct sml_converter {  // adaptor to provide AsymmUnit or UnitCell interface for a list
    const smatd_list& ml;
    sml_converter(const smatd_list& _ml) : ml(_ml) {}
    int MatrixCount() const {  return ml.Count();  }
    const smatd& GetMatrix(int i) const {  return ml[i];  }
  };
  // compares p with values in array axes. Used in SymmToMatrix function
  static short IsAxis(const olxstr& p) {
    if( p.IsEmpty() )  return -1;
    for( int i=0; i < 3; i++ )
      if( Axis[i] == p.CharAt(0) )  
        return i;
    return -1;
  }
  
  template <typename vc> 
  static vc& ExtractTranslation(const olxstr& str, vc& t)  {
    if( str.Length() == 3 )  {
      t[0] += (int)(str.CharAt(0)-'5');
      t[1] += (int)(str.CharAt(1)-'5');
      t[2] += (int)(str.CharAt(2)-'5');
    }
    else if( str.Length() == 6 ) {
      t[0] += (str.SubString(0, 2).ToInt()-55);
      t[1] += (str.SubString(2, 2).ToInt()-55);
      t[2] += (str.SubString(4, 2).ToInt()-55);
    }
    else
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong translation code: ") << str);
    return t;
  }
  static olxstr FormatFloatEx(double f)  {
    olxstr rv;
    if( f < 0 )
      rv << '-';
    int v = abs(Round(f*12)), base = 12;
    int denom = esdl::gcd(v, base);
    if( denom != 1 )  {
      v /= denom;
      base /= denom;
    }
    if( base == 1 )  return rv << v;
    else             return rv << v << '/' << base;
  }
  template <class SM>
  static olxstr _MatrixToSymm(const SM& M, bool fraction)  {
    olxstr T, T1;
    for( int j=0; j < 3; j ++ )  {
      if( j != 0 )
        T << ',';
      for( int i=0; i < 3; i ++ )  {
        if( i == j )  {
          if( M.r[j][i] != 0 )  {
            T1 << CharSign(M.r[j][i]);
            T1 << Axis[j];
          }
          continue;
        }
        if( M.r[j][i] != 0 )  {
          T1.Insert(Axis[i], 0);
          T1.Insert(CharSign(M.r[j][i]), 0);
        }
      }
      if( M.t[j] != 0 )  {
        if( !fraction )
          T1.Insert(olxstr::FormatFloat(3, M.t[j], false).TrimFloat(), 0);
        else
          T1.Insert(FormatFloatEx(M.t[j]), 0);
      }
      T << T1;
      T1 = EmptyString;
    }
    return T;
  }
  // can tace both TAsymmUnit or TUnitCell
  template <class MC>
  static smatd _SymmCodeToMatrix(const MC& au, const olxstr& Code, int* index=NULL)  {
    TStrList Toks(Code, '_');
    smatd mSymm;
    if( Toks.Count() == 1 )  {  // standard XP symm code like 3444
      if( Toks[0].Length() >= 4 )  {
        Toks.Add( Toks[0].SubStringFrom(Toks[0].Length()-3) );
        Toks[0].SetLength(Toks[0].Length()-3);
      }
      else  {
        int isymm = Toks[0].ToInt()-1;
        if( isymm < 0 || isymm >= au.MatrixCount() )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix index: ") << isymm);
        return au.GetMatrix(isymm);
      }
    }
    if( Toks.Count() != 2 )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong code: ") << Code);
    int isymm = Toks[0].ToInt()-1;
    if( isymm < 0 || isymm >= au.MatrixCount() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix index: ") << isymm);
    mSymm = au.GetMatrix(isymm);
    if( index != NULL )
      *index = isymm;
    ExtractTranslation(Toks[1], mSymm.t);
    return mSymm;
  }

  static const char Axis[];
public:
    // Transforms matrix to standard SYMM operation (INS, CIF files)
  static olxstr MatrixToSymm(const smatd& M)  {
    return _MatrixToSymm(M, false);
  }
    // Transforms matrix to standard SYMM operation (INS, CIF files), using fractions of 12 for translations
  static olxstr MatrixToSymmEx(const smatd& M)  {
    return _MatrixToSymm(M, true);
  }
    // Transforms matrix to standard SYMM operation (INS, CIF files), using fractions of 12 for translations
  static olxstr MatrixToSymmEx(const mat3i& M);
    // Transforms standard SYMM operation (INS, CIF files) to matrix
  static bool SymmToMatrix(const olxstr& symm, smatd& M);
  // return a matrix representation of 1_555 or 1_555555 code for the unit cell
  static smatd SymmCodeToMatrixU(const class TUnitCell& UC, const olxstr& Code);
  // return a matrix representation of 1_555 or 1_555555 code for the asymmetric unit
  static smatd SymmCodeToMatrixA(const class TAsymmUnit& AU, const olxstr& Code);
  // return a matrix representation of 1_555 or 1_555555 code for the the list of matrices
  static smatd SymmCodeToMatrix(const smatd_list& ml, const olxstr& Code)  {
    int index = -1;
    smatd rv = _SymmCodeToMatrix(sml_converter(ml), Code, &index);
    rv.SetTag(index);
    return rv;
  }
  // return a string representation of a matrix like 1_555 or 1_555555 code in dependence on
  // the length of translations; Matrix->Tag must be set to the index of the matrix in the Unit cell!!!
  static olxstr MatrixToSymmCode(const TUnitCell& UC, const smatd& M);
  static olxstr MatrixToSymmCode(const smatd_list& ml, const smatd& M);
};


EndXlibNamespace()
#endif

