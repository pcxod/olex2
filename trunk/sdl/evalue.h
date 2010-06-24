//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifndef __olx_sdl_evalue_H
#define __olx_sdl_evalue_H
#include <math.h>
#include "exception.h"
#include "evalue.h"
#include "emath.h"

BeginEsdlNamespace()

template <typename> class TEVPoint;
//---------------------------------------------------------------------------
template <class EType> class TEValue: public IEObject  {
  EType FV, FE;
public:

  TEValue() : FV(0), FE(0) {}

  TEValue(const TEValue& obj) : FV(obj.FV), FE(obj.FE)  {}

  template <class T>
    TEValue(const TEValue<T>& obj)  {
      FV = (EType)obj.V;
      FE = (EType)obj.E;
    }

  TEValue(EType Value, EType Error) : FV(Value), FE(Error) {}

  TEValue(const olxstr& str) : FV(0), FE(0) {  *this = str;  }

  EType& V()  {  return  FV;  }
  EType& E()  {  return  FE;  }

  const EType& GetV() const {  return  FV;  }
  const EType& GetE() const {  return  FE;  }


  TEValue Sqrt() const {
    TEValue<EType> Val;
    if( FV == 0 )
      throw TDivException(__OlxSourceInfo);
    Val.FV = (EType)sqrt(FV);
    Val.FE = (EType)(FE/Val.FV)/2;
    return Val;
  }

  TEValue& SelfSqrt()  {
    if( FV == 0 )
      throw TDivException(__OlxSourceInfo);
    FE = (EType)(FE/FV)/2;
    FV = (EType)sqrt(FV);
    return *this;
  }

  const TEValue& operator = (const TEValue& p)  {
    FV = p.FV;
    FE = p.FE;
    return p;
  }

  template <class AType>
    const TEValue<AType>& operator = (const TEValue<AType>& p)  {
      FV = (EType)p.FV;
      FE = (EType)p.FE;
      return p;
    }

  const olxstr& operator = (const olxstr& S)  {
    size_t i = S.LastIndexOf('(');
    if( i != InvalidIndex && i > 0 )  {
      FV = (EType)S.SubStringTo(i).ToDouble();
      size_t j = S.LastIndexOf(')'),
             k = S.FirstIndexOf('.');
      double po=1;
      if( j != InvalidIndex && j > i )  {
        if( k != InvalidIndex && k < i )
          for( size_t l=0; l < i-k-1; l++ )
            po *= 10;
        FE = (EType)(S.SubString(i+1,j-i-1).ToDouble()/po);
      }
    }
    else  {  // no error
      FV = (EType)S.ToDouble();
      FE = 0;
    }
    return S;
  }

  template <class AType>
    TEValue& operator += (const TEValue<AType>& S)  {
      FV += S.FV;
      FE = (EType)sqrt(S.FE*S.FE+FE*FE);
     return *this;
    }

  template <class AType>
    TEValue& operator *= (const TEValue<AType>& S)  {
      if( FV == 0 || S.FV == 0 )
        throw TDivException(__OlxSourceInfo);
      if( FE != 0 || S.FE != 0 )
        FE = (EType)(sqr(S.FE/S.FV) + olx_sqr(FE/FV));
      FV *= S.FV;
      return *this;
    }

  template <class AType>
    TEValue& operator /= (const TEValue<AType>& S)  {
      if( FV == 0 || S.FV == 0 )
        throw TDivException(__OlxSourceInfo);
      if( FE != 0 || S.FE != 0 )
        FE = (EType)(sqr(S.FE/S.FV) + olx_sqr(FE/FV));
      FV /= S.FV;
      return *this;
    }

  template <class AType>
    TEValue& operator -= (const TEValue<AType>& S)  {
      FV -= S.FV;
      FE = (EType)sqrt(sqr(S.FE)+sqr(FE));
      return *this;
    }

  TEValue& operator += (EType S)  {
    FV += S;
    return *this;
  }

  TEValue& operator *= (EType S)  {
    FV *= S;
    FE *= S;
    return *this;
  }

  TEValue& operator /= (EType S)  {
    if( S == 0 )
      throw TDivException(__OlxSourceInfo);
    FV /= S;
    FE /= S;
    return *this;
  }

  TEValue& operator -= (EType S)  {
    FV -= S;
    return *this;
  }

  TEValue operator + (EType S) const {
    return TEValue<EType>(FV+S, FE);
  }

  TEValue operator - (EType S) const {
    return TEValue<EType>(FV-S, FE);
  }

  TEValue operator * (EType S) const {
    return TEValue<EType>(FV*S, FE*S);
  }

  TEValue operator / (EType S) const {
    if( S == 0 )
      throw TDivException(__OlxSourceInfo);
    return TEValue<EType>(FV/S, FE/S);
  }

  template <class AType>
    TEValue operator + (const TEValue<AType> &S) const {
      TEValue<EType> P(*this);
      return (P += S);
    }

  template <class AType>
    TEValue operator - (const TEValue<AType> &S) const {
      TEValue<EType> P(*this);
      return (P -= S);
    }

  template <class AType>
    TEValue operator * (const TEValue<AType> &S) const {
      TEValue<EType> P(*this);
      return (P *= S);
    }

  template <class AType>
    TEValue operator / (const TEValue<AType> &S) const {
      TEValue<EType> P(*this);
      return (P /= S);
    }

  template <class SC> SC StrRepr() const {
    if( FE != 0 )  {
      SC strv, stre;
      int pr = 0;
      double po = 1;
      while( olx_abs(FE*po) < 10 )  {
        po *= 10;
        pr++;
      }
      int iv = olx_round(FE*po);
	  if( iv > 20 )  {
		  iv /= 10;
		  pr--;
	  }
      if( pr != 0 )  {
        strv = olxstr::FormatFloat(pr, FV);
        stre = iv;
      }
      else  {
        strv = (int)FV;
        stre << (int)olx_round(FE);
      }
      while( stre.EndsWith('0') && strv.EndsWith('0') )  {
        stre.SetLength(stre.Length()-1);
        strv.SetLength(strv.Length()-1);
      }
      if( strv.EndsWith('.') )
        strv.SetLength(strv.Length()-1);
      return (SC(strv) << '(' << stre << ')');
    }
    else
      return SC(FV);
  }
  inline virtual TIString ToString() const {  return StrRepr<olxstr>();  }
  inline olxcstr ToCStr() const {  return StrRepr<olxcstr>();  }
  inline olxwstr ToWStr() const {  return StrRepr<olxwstr>();  }
};

  typedef TEValue<float>  TEValueF;
  typedef TEValue<double>  TEValueD;

EndEsdlNamespace()
#endif
