// testa.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

//#include "conio.h"

#include "exception.h"
#include "efile.h"
#include "estrlist.h"
#include "xapp.h"
#include "log.h"

#include "ins.h"
#include "asymmunit.h"
#include "catom.h"

#include "symmlib.h"
#include "library.h"
#include "outstream.h"
#include "estlist.h"
#include "fastsymm.h"

#include <iostream>

void ExportSymmLib();
void ExportSymmLibA();
void ExportSymmLibB();  // MSVC crashes on this code...
void ExportSymmLibC();
void ExportBAI(TAtomsInfo& ais);



int main(int argc, char* argv[])  {
  try  {
    olxstr bd( TEFile::ExtractFilePath( argv[0] ) );
    if( bd.IsEmpty() )
      bd = TEFile::CurrentDir();

    TEFile::AddTrailingBackslashI( bd );

    TXApp XApp(bd);
    XApp.GetLog().AddStream( new TOutStream(), true );
    olxstr dataFolder = TEFile::AbsolutePathTo( bd, "../../../data");
    olxstr sampleFolder = TEFile::AbsolutePathTo( bd, "../../../sampleData");

    TAtomsInfo ai(dataFolder + "/ptablex.dat");
    TSymmLib sl( dataFolder + "/symmlib.xld" );

    TIns* ins = new TIns(&ai);
    XApp.XFile().RegisterFileFormat( ins, "ins" );
    XApp.XFile().RegisterFileFormat( ins, "res" );
    XApp.XFile().LoadFromFile(sampleFolder + "/05srv085.ins");

    for( int i=0; i < XApp.XFile().GetAsymmUnit().AtomCount(); i++ )  {
      TCAtom &ca = XApp.XFile().GetAsymmUnit().GetAtom(i);
      printf("%s \t %f \t %f \t %f\n", ca.Label().c_str(),
        ca.ccrd()[0],
        ca.ccrd()[1],
        ca.ccrd()[2] );
    }
    TSpaceGroup* sg = sl.FindSG( XApp.XFile().GetAsymmUnit() );
    if( sg != NULL )  {
      printf("File space group: %s", sg->GetName().c_str() ); 
      TFastSymmLib fsl;
      FastSymm* fs = fsl.FindSymm(sg->GetName() );
    }
    ABasicFunction* sgm = XApp.GetLibrary().FindMacro("SG");
    if( sgm !=  NULL )  {
      TStrObjList args;
      TParamList opts;
      TMacroError me;
      sgm->Run(args, opts, me);
      if( !me.IsSuccessful() )
        printf("\n%s", me.GetInfo().c_str() );
    }
    //ExportSymmLib();
    //ExportSymmLibA();
    ExportBAI( *XApp.AtomsInfo() );
    //ExportSymmLibC();
  }
  catch( TExceptionBase& exc )  {
    printf("An exception occured: %s\n", EsdlObjectName(exc).c_str() );
    printf("details: %s\n", exc.GetException()->GetFullMessage().c_str() );
  }
  printf("\n...");
  std::cin.get();
  return 0;
}
// mixes
const short mxX = 0x001,
            mxY = 0x002,
            mxZ = 0x004;

olxstr ProcessRow(const vec3d& vec, const olxstr& var_type, int ind)  {
  olxstr rv("    ");
  rv << var_type << " a" << ind << " = ";
  if( vec[0] == 1 )
    rv << "v[0]";
  else if( vec[0] == -1 )
    rv << "-v[0]";

  if( vec[1] == 1 )
    rv << ((vec[0] != 0) ? "+v[1]" : "v[1]");
  else if( vec[1] == -1 )
    rv << "-v[1]";

  if( vec[2] == 1 )
    rv << ((vec[0] != 0 || vec[1] != 0) ? "+v[2]" : "v[2]");
  else if( vec[2] == -1 )
    rv << "-v[2]";
  rv << ';';
 return rv;
}
olxstr ProcessRowInPlace(const vec3d& vec, int ind)  {
  olxstr rv("    v[");
  rv << ind << "] = ";
  if( vec[0] == 1 )
    rv << "v[0]";
  else if( vec[0] == -1 )
    rv << "-v[0]";

  if( vec[1] == 1 )
    rv << ((vec[0] != 0) ? "+v[1]" : "v[1]");
  else if( vec[1] == -1 )
    rv << "-v[1]";

  if( vec[2] == 1 )
    rv << ((vec[0] != 0 || vec[1] != 0) ? "+v[2]" : "v[2]");
  else if( vec[2] == -1 )
    rv << "-v[2]";
  rv << ';';
 return rv;
}
olxstr GenName(const smatd& m)  {
  olxstr rv;
  for( int i=0; i < 3; i++ )  {
    for( int j=0; j < 3; j++ )  {
      if( m.r[i][j] == -1 )  rv << 0; 
      else if( m.r[i][j] == 0 )  rv << 1;
      else if( m.r[i][j] == 1 )  rv << 2;
    }
  }
  return rv;
 }
bool IsDiagonale(const smatd& m )  {
  for( int i=0; i < 3; i++ )  {
    for( int j=0; j < 3; j++ )  {
      if( i != j && m.r[i][j] != 0 )  return false;
    }
  }
  return true;
}
void copymatr(smatd& dest, const smatd& src, bool transpose)  {
  if( !transpose )
    dest.r = src.r;
  else 
    dest.r = src.r.Transpose(src.r);
}
void CodeGen(smatd& m, const olxstr& d_type, const olxstr&var_type, const olxstr& Suffix, TStrList& out)  {
  out.Add("  static inline void Mult") << Suffix << '(' << d_type << " v)  {";
  if( m.r.IsI() )  {
    out.Last().String() << "  }";
    return;
  }
  if( IsDiagonale(m) )  {
    out.Add( EmptyString );
    if( m.r[0][0] == -1 )
      out.Last().String() << "    v[0] = -v[0];";
    if( m.r[1][1] == -1 )
      out.Last().String() << "    v[1] = -v[1];";
    if( m.r[2][2] == -1 )
      out.Last().String() << "    v[2] = -v[2];";
    out.Add("  }");
    return;
  }
  bool xmixed = false, ymixed = false, zmixed = false;
  bool xymixed = false, xzmixed = false, yzmixed = false;
  bool yxmixed = false, zxmixed = false, zymixed = false;
  if( m.r[0][1] != 0 || m.r[0][2] != 0 )  {
    if( m.r[0][1] != 0 )  xymixed = true;
    if( m.r[0][2] != 0 )  xzmixed = true;
    xmixed = true;
  }
  if( m.r[1][0] != 0 || m.r[1][2] != 0 )  {
    if( m.r[1][0] != 0 )  yxmixed = true;
    if( m.r[1][2] != 0 )  yzmixed = true;
    ymixed = true;
  }
  if( m.r[2][0] != 0 || m.r[2][1] != 0 )  {
    if( m.r[2][0] != 0 )  zxmixed = true;
    if( m.r[2][1] != 0 )  zymixed = true;
    zmixed = true;
  }

  if( !xmixed )  {
    if( m.r[0][0] == -1 )  {
      out << "    v[0] = -v[0];";
      if( yxmixed )  m.r[1][0] *= -1;
      if( zxmixed )  m.r[2][0] *= -1;
    }
  }
  else  {
    if( (xymixed || xzmixed) && (!xzmixed & !yxmixed && !zxmixed) )  {
      out.Add( ProcessRowInPlace(m.r[0], 0) );
      xmixed = false;
    }
    else 
      out.Add( ProcessRow(m.r[0], var_type, 0) );
  }
  if( !ymixed )  {
    if( m.r[1][1] == -1 )  {
      out << "    v[1] = -v[1];";
      if( zymixed )  m.r[2][1] *= -1;
    }
  }
  else  {
    if( (yxmixed || yzmixed) && (!zymixed) )  {
      out.Add( ProcessRowInPlace(m.r[1], 1) );
      ymixed = false;
    }
    else 
      out.Add( ProcessRow(m.r[1], var_type, 1) );
  }
  if( !zmixed )  {
    if( m.r[2][2] == -1 )
      out << "    v[2] = -v[2];";
  }
  else  {
    if( zymixed || zxmixed )  {
      out.Add( ProcessRowInPlace(m.r[2], 2) );
      zmixed = false;
    }
    else
      out.Add( ProcessRow(m.r[2], var_type, 2) );
  }
  if( xmixed )  out.Add( "    v[0] = a0;" );
  if( ymixed )  out.Add( "    v[1] = a1;" );
  if( zmixed )  out.Add( "    v[2] = a2;" );
  out.Add("  }");
}
int CodeGen(TSpaceGroup& sg, TStrList& out)  {
  smatd_list ml;
  sg.GetMatrices(ml, mattAll);
  out.Add("  template <class TT, class AT> static inline void DoGeneratePos(const TT& v, AT& res)  {");
  for( int i=0; i < ml.Count(); i++ )  {
    smatd& m = ml[i];
    for( int j=0; j < 3; j++ )  {
      olxstr& str = out.Add("    res[") << i << "][" << j << "] = ";
      bool added = false;
      for( int k=0; k < 3; k++ )  {
        if( m.r[j][k] != 0 )  {
          str << ((m.r[j][k] > 0) ? (added ? (olxstr("+v[") << k << ']') : (olxstr("v[") << k << ']')) : (olxstr("-v[") << k << ']'));
          added = true;
        }
      }
      if( m.t[j] != 0 )  {
        int v = Round(m.t[j]*12), base = 12;
        int denom = esdl::gcd(v, base);
        if( denom != 1 )  {
          v /= denom;
          base /= denom;
        }
        str << '+' << v << "./" << base;
      }
      str << ';';
    }
  }
  out.Add("  }");
  out.Add("  template <class TT, class AT, class SAT> static inline void DoGenerateHkl(TT& v, AT& res, SAT& phase)  {");
  for( int i=0; i < ml.Count(); i++ )  {
    smatd& m = ml[i];
    for( int j=0; j < 3; j++ )  {
      olxstr& str = out.Add("    res[") << i << "][" << j << "] = ";
      bool added = false;
      for( int k=0; k < 3; k++ )  {
        if( m.r[k][j] != 0 )  {  // transposed form for hkl
          str << ((m.r[k][j] > 0) ? (added ? (olxstr("+v[") << k << ']') : (olxstr("v[") << k << ']')) : (olxstr("-v[") << k << ']'));
          added = true;
        }
      }
      str << ';';
    }
    olxstr& str = out.Add("    phase[") << i << "] = ";
    bool added = false;
    for( int j=0; j < 3; j++ )  {
      if( m.t[j] != 0 )  {  // transposed form for hkl
        int v = Round(m.t[j]*12), base = 12;
        int denom = esdl::gcd(v, base);
        if( denom != 1 )  {
          v /= denom;
          base /= denom;
        }
        olxstr mult(v);  mult << "./"  << base;
        str << ((m.t[j] > 0) ? (added ? (olxstr("+v[") << j << ']') : (olxstr("v[") << j << ']')) : (olxstr("-v[") << j << ']'));
        str << '*' << mult;
        added = true;
      }
    }
    if( !added )  str << " 0";
    str << ';';
  }
  out.Add("  }");
  return ml.Count();
}

void ExportSymmLib()  {
  TSymmLib& sl = *TSymmLib::GetInstance();
  smatd_list ml, uniqm;
  TStrList sgout, sgout1;
  olxstr matrname;
  sgout.Add("  TIntList mind(256);");
  for( int i=0; i < sl.SGCount(); i++ )  {
    TSpaceGroup& sg = sl.GetGroup(i);
    ml.Clear();
    sg.GetMatrices( ml, mattAll);
    if( i == 0 )
      sgout.Add("  FastSymm* fs = new FastSymm(") << ml.Count() << ");";
    else
      sgout.Add("  fs = new FastSymm(") << ml.Count() << ");";
    sgout.Add("  list.Add(\"") << sg.GetName() << "\", fs);";
    for( int j=0; j < ml.Count(); j ++ )  {
      bool uniq = true;
      smatd& m = ml[j];
      for( int k=0; k < uniqm.Count(); k++ )  {
        if( m.r == uniqm[k].r )  {
          uniq = false;
          m.SetTag(k);
          break;
        }
      }
      if( uniq )  {
        m.SetTag( uniqm.Count() );
        uniqm.AddCCopy( m );
      }
      sgout.Add("  mind[") << j << "] = " << m.GetTag() << ';';
    }
    sgout.Add("  processSymm(mind, ") << ml.Count() << ", *fs);";
  }
  sgout1.Add("void processSymm(const TIntList& mind, int count, FastSymm& fs)  {");
    sgout1.Add("  for(int i=0; i < count; i++ )  {");
    sgout1.Add("    fs.DVs[i]  = MultDV[mind[i]];");
    sgout1.Add("    fs.FVs[i]  = MultFV[mind[i]];");
    sgout1.Add("    fs.IVs[i]  = MultIV[mind[i]];");
    sgout1.Add("    fs.DVTs[i] = MultDVT[mind[i]];");
    sgout1.Add("    fs.FVTs[i] = MultFVT[mind[i]];");
    sgout1.Add("    fs.IVTs[i] = MultIVT[mind[i]];");
    sgout1.Add("    fs.DAs[i]  = MultDA[mind[i]];");
    sgout1.Add("    fs.FAs[i]  = MultFA[mind[i]];");
    sgout1.Add("    fs.IAs[i]  = MultIA[mind[i]];");
    sgout1.Add("    fs.DATs[i] = MultDAT[mind[i]];");
    sgout1.Add("    fs.FATs[i] = MultFAT[mind[i]];");
    sgout1.Add("    fs.IATs[i] = MultIAT[mind[i]];");
    sgout1.Add("  }");
    sgout1.Add("}");
  sgout1.Add("  FastSymm::DV MultDV[") << uniqm.Count() << "], MultDVT[" << uniqm.Count() << "];";
  sgout1.Add("  FastSymm::FV MultFV[") << uniqm.Count() << "], MultFVT[" << uniqm.Count() << "];";
  sgout1.Add("  FastSymm::IV MultIV[") << uniqm.Count() << "], MultIVT[" << uniqm.Count() << "];";
  sgout1.Add("  FastSymm::DA MultDA[") << uniqm.Count() << "], MultDAT[" << uniqm.Count() << "];";
  sgout1.Add("  FastSymm::FA MultFA[") << uniqm.Count() << "], MultFAT[" << uniqm.Count() << "];";
  sgout1.Add("  FastSymm::IA MultIA[") << uniqm.Count() << "], MultIAT[" << uniqm.Count() << "];";
  TStrList out;
  out.Add("struct FastMatrices {");
  smatd matr;
  for( int i=0; i < uniqm.Count(); i++ )  {
    uniqm[i].t.Null();
    matrname = GenName(uniqm[i]);
    out.Add("// ")  << TSymmParser::MatrixToSymm(uniqm[i]);
    copymatr(matr, uniqm[i], false);
    CodeGen(matr, "TVector<double>&", "double", olxstr("DV") << matrname, out);
    copymatr(matr, uniqm[i], false);
    CodeGen(matr, "double*", "double", olxstr("DA") << matrname, out);
    copymatr(matr, uniqm[i], false);
    CodeGen(matr, "TVector<float>&", "float", olxstr("FV") << matrname, out);
    copymatr(matr, uniqm[i], false);
    CodeGen(matr, "float*", "float", olxstr("FA") << matrname, out);
    copymatr(matr, uniqm[i], false);
    CodeGen(matr, "TVector<int>&", "int", olxstr("IV") << matrname, out);
    copymatr(matr, uniqm[i], false);
    CodeGen(matr, "int*", "int", olxstr("IA") << matrname, out);

    copymatr(matr, uniqm[i], true);
    CodeGen(matr, "TVector<double>&", "double", olxstr("DVT") << matrname, out);
    copymatr(matr, uniqm[i], true);
    CodeGen(matr, "double*", "double", olxstr("DAT") << matrname, out);
    copymatr(matr, uniqm[i], true);
    CodeGen(matr, "TVector<float>&", "float", olxstr("FVT") << matrname, out);
    copymatr(matr, uniqm[i], true);
    CodeGen(matr, "float*", "float", olxstr("FAT") << matrname, out);
    copymatr(matr, uniqm[i], true);
    CodeGen(matr, "TVector<int>&", "int", olxstr("IVT") << matrname, out);
    copymatr(matr, uniqm[i], true);
    CodeGen(matr, "int*", "int", olxstr("IAT") << matrname, out);

    sgout1.Add("  MultDV[") << i << "]  = &FastMatrices::MultDV" << matrname << ';';
    sgout1.Add("  MultFV[") << i << "]  = &FastMatrices::MultFV" << matrname << ';';
    sgout1.Add("  MultIV[") << i << "]  = &FastMatrices::MultIV" << matrname << ';';
    sgout1.Add("  MultDVT[") << i << "] = &FastMatrices::MultDVT" << matrname << ';';
    sgout1.Add("  MultFVT[") << i << "] = &FastMatrices::MultFVT" << matrname << ';';
    sgout1.Add("  MultIVT[") << i << "] = &FastMatrices::MultIVT" << matrname << ';';
    sgout1.Add("  MultDA[") << i << "]  = &FastMatrices::MultDA" << matrname << ';';
    sgout1.Add("  MultFA[") << i << "]  = &FastMatrices::MultFA" << matrname << ';';
    sgout1.Add("  MultIA[") << i << "]  = &FastMatrices::MultIA" << matrname << ';';
    sgout1.Add("  MultDAT[") << i << "] = &FastMatrices::MultDAT" << matrname << ';';
    sgout1.Add("  MultFAT[") << i << "] = &FastMatrices::MultFAT" << matrname << ';';
    sgout1.Add("  MultIAT[") << i << "] = &FastMatrices::MultIAT" << matrname << ';';
  }
  out.Add("};");
  out.SaveToFile("e:/tmp/sgout.h");
  sgout1.AddList( sgout );
  sgout1.SaveToFile("e:/tmp/sgout.cpp");
}
            
void ExportSymmLibA()  {
  TSymmLib& sl = *TSymmLib::GetInstance();
  TStrList out;
  for( int i=0; i < sl.SGCount(); i++ )  {
    TSpaceGroup& sg = sl.GetGroup(i);
    out.Add("//") << sg.GetName() << " #" << sg.GetNumber() << " axis:" << sg.GetAxis();
    out.Add("class TSpaceGroup_") << i << " : public ISGGroup  {";
    out.Add("public: ");
    int mc = CodeGen(sg, out);
    out.Add("  virtual inline void GeneratePos(const evecd& v, TArrayList<evecd>& res) const {  DoGeneratePos(v,res);  }");
    out.Add("  virtual inline void GeneratePos(const evecd& v, TArrayList<TVPointD>& res) const {  DoGeneratePos(v,res);  }");
    out.Add("  virtual inline int GetSize() const {  return ") << mc << ";  }";
    out.Add("};");
  }
  out.SaveToFile("e:/tmp/sgoutx.h");
}

void ExportBAI(TAtomsInfo& ais)  {
  TStrList out;
  char bf[10];
  for( int i=0; i < ais.Count(); i++ )  {
    TBasicAtomInfo& bai = ais.GetAtomInfo(i);
    out.Add("    bai = &Data.AddNew();");
    out.Add("    bai->SetSymbol(\"") << bai.GetSymbol() << "\");";
    out.Add("    bai->SetName(\"") << bai.GetName() << "\");";
    out.Add("    bai->SetMr(") << bai.GetMr() << ");";
    sprintf(bf, "0x%X", bai.GetDefColor());
    out.Add("    bai->SetDefColor(") << bf << ");";
    out.Add("    bai->SetRad(") << bai.GetRad() << ");";
    out.Add("    bai->SetRad1(") << bai.GetRad1() << ");";
    out.Add("    bai->SetRad2(") << bai.GetRad2() << ");";
    out.Add("    bai->SetIndex(") << bai.GetIndex() << ");";
    for( int j=0; j < bai.IsotopeCount(); j++ )  {
      out.Add("      ai = &bai->NewIsotope();");
      out.Add("      ai->SetMr(") << bai.GetIsotope(j).GetMr() << ");";
      out.Add("      ai->SetW(") << bai.GetIsotope(j).GetW() << ");";
    }
  }
  out.SaveToFile("e:/tmp/baiout.h");
}
struct SGSM {
  double m[3];
};
struct SG  {
  char* name, *full_name, *hall_symbol, *axis;
  int number, latt;
  char* matrices;
};
void ExportSymmLibC()  {
  TSymmLib& sl = *TSymmLib::GetInstance();
  TStrList out;
  out.Add("olx_SGDef olx_SG[") << sl.SGCount() << "]={";
  for( int i=0; i < sl.SGCount(); i++ )  {
    TSpaceGroup& sg = sl.GetGroup(i);
    olxstr& s = out.Add("{\"") << sg.GetName() << "\", \"" << sg.GetFullName() << 
      "\", \"" << sg.GetHallSymbol().Trim(' ') << "\", \"" << sg.GetAxis() << "\", " << sg.GetNumber() << 
      ", " << sg.GetLattice().GetLatt()*( sg.IsCentrosymmetric() ? 1 : -1) << ", \"";
    for( int j=0; j < sg.MatrixCount(); j++ )  {
      s << TSymmParser::MatrixToSymm(sg.GetMatrix(j));
      if( (j+1) < sg.MatrixCount() ) s << ';';
      else s << '"';
    }
    if( sg.MatrixCount() == 0 )  s << '"';
    if( (i+1) < sl.SGCount() ) 
      s << "}, ";
    else
      s << "}};";
  }
  out.SaveToFile("e:/tmp/sgoutsx.h");
}
void ExportSymmLibB()  {
  TSymmLib& sl = *TSymmLib::GetInstance();
  TStrList out;
  for( int i=0; i < sl.SGCount(); i++ )  {
    TSpaceGroup& sg = sl.GetGroup(i);

    out.Add("  sg = new TSpaceGroup(\"") << sg.GetName() << "\", \"" << sg.GetFullName() << 
      "\", \"" << sg.GetHallSymbol().Trim(' ') << "\", \"" << sg.GetAxis() << "\", " << sg.GetNumber() << 
      ", GetLattice(" << abs(sg.GetLattice().GetLatt()-1) << "), " << 
      sg.IsCentrosymmetric() << ");";
    for( int j=0; j < sg.MatrixCount(); j++ )  {
      smatd& m = sg.GetMatrix(j);
      out.Add("    sg->AddMatrix(") << m.r[0][0] << ", " << m.r[0][1] << ", " << m.r[0][2] << ", " <<
        m.r[1][0] << ", " << m.r[1][1] << ", " << m.r[1][2] << ", " <<
        m.r[2][0] << ", " << m.r[2][1] << ", " << m.r[2][2] << ", " <<
        m.t[0]    << ", " << m.t[1]    << ", " << m.t[2] << ");";
    }
    out.Add("  SpaceGroups.Add(\"") << sg.GetName() << "\", sg);";
  }
  out.SaveToFile("e:/tmp/sgouts.h");
}