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
        ca.CCenter().Value(0).GetV(),
        ca.CCenter().Value(1).GetV(),
        ca.CCenter().Value(2).GetV() );
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

olxstr ProcessRow(const TVectorD& vec, const olxstr& var_type, int ind)  {
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
olxstr ProcessRowInPlace(const TVectorD& vec, int ind)  {
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
olxstr GenName(const TMatrixD& m)  {
  olxstr rv;
  for( int i=0; i < 3; i++ )  {
    for( int j=0; j < 3; j++ )  {
      if( m[i][j] == -1 )  rv << 0; 
      else if( m[i][j] == 0 )  rv << 1;
      else if( m[i][j] == 1 )  rv << 2;
    }
  }
  return rv;
 }
bool IsDiagonale(const TMatrixD& m )  {
  for( int i=0; i < 3; i++ )  {
    for( int j=0; j < 3; j++ )  {
      if( i != j && m[i][j] != 0 )  return false;
    }
  }
  return true;
}
void copymatr(TMatrixD& dest, const TMatrixD& src, bool transpose)  {
  if( !transpose )  {
  for( int i=0; i < 3; i++ )
    for( int j=0; j < 3; j++ )
      dest[i][j] = src[i][j];
  }
  else  {
    for( int i=0; i < 3; i++ )
      for( int j=0; j < 3; j++ )
        dest[j][i] = src[i][j];
  }
}
void CodeGen(TMatrixD& m, const olxstr& d_type, const olxstr&var_type, const olxstr& Suffix, TStrList& out)  {
  out.Add("  static inline void Mult") << Suffix << '(' << d_type << " v)  {";
  if( m.IsE() )  {
    out.Last().String() << "  }";
    return;
  }
  if( IsDiagonale(m) )  {
    out.Add( EmptyString );
    if( m[0][0] == -1 )
      out.Last().String() << "    v[0] = -v[0];";
    if( m[1][1] == -1 )
      out.Last().String() << "    v[1] = -v[1];";
    if( m[2][2] == -1 )
      out.Last().String() << "    v[2] = -v[2];";
    out.Add("  }");
    return;
  }
  bool xmixed = false, ymixed = false, zmixed = false;
  bool xymixed = false, xzmixed = false, yzmixed = false;
  bool yxmixed = false, zxmixed = false, zymixed = false;
  if( m[0][1] != 0 || m[0][2] != 0 )  {
    if( m[0][1] != 0 )  xymixed = true;
    if( m[0][2] != 0 )  xzmixed = true;
    xmixed = true;
  }
  if( m[1][0] != 0 || m[1][2] != 0 )  {
    if( m[1][0] != 0 )  yxmixed = true;
    if( m[1][2] != 0 )  yzmixed = true;
    ymixed = true;
  }
  if( m[2][0] != 0 || m[2][1] != 0 )  {
    if( m[2][0] != 0 )  zxmixed = true;
    if( m[2][1] != 0 )  zymixed = true;
    zmixed = true;
  }

  if( !xmixed )  {
    if( m[0][0] == -1 )  {
      out << "    v[0] = -v[0];";
      if( yxmixed )  m[1][0] *= -1;
      if( zxmixed )  m[2][0] *= -1;
    }
  }
  else  {
    if( (xymixed || xzmixed) && (!xzmixed & !yxmixed && !zxmixed) )  {
      out.Add( ProcessRowInPlace(m[0], 0) );
      xmixed = false;
    }
    else 
      out.Add( ProcessRow(m[0], var_type, 0) );
  }
  if( !ymixed )  {
    if( m[1][1] == -1 )  {
      out << "    v[1] = -v[1];";
      if( zymixed )  m[2][1] *= -1;
    }
  }
  else  {
    if( (yxmixed || yzmixed) && (!zymixed) )  {
      out.Add( ProcessRowInPlace(m[1], 1) );
      ymixed = false;
    }
    else 
      out.Add( ProcessRow(m[1], var_type, 1) );
  }
  if( !zmixed )  {
    if( m[2][2] == -1 )
      out << "    v[2] = -v[2];";
  }
  else  {
    if( zymixed || zxmixed )  {
      out.Add( ProcessRowInPlace(m[2], 2) );
      zmixed = false;
    }
    else
      out.Add( ProcessRow(m[2], var_type, 2) );
  }
  if( xmixed )  out.Add( "    v[0] = a0;" );
  if( ymixed )  out.Add( "    v[1] = a1;" );
  if( zmixed )  out.Add( "    v[2] = a2;" );
  out.Add("  }");
}

void ExportSymmLib()  {
  TSymmLib& sl = *TSymmLib::GetInstance();
  TMatrixDList ml, uniqm;
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
      TMatrixD& m = ml[j];
      for( int k=0; k < uniqm.Count(); k++ )  {
        const TMatrixD& n = uniqm[k];
        if( m[0][0] == n[0][0] &&
            m[0][1] == n[0][1] &&
            m[0][2] == n[0][2] &&
            m[1][0] == n[1][0] &&
            m[1][1] == n[1][1] &&
            m[1][2] == n[1][2] &&
            m[2][0] == n[2][0] &&
            m[2][1] == n[2][1] &&
            m[2][2] == n[2][2] )  {
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
  TMatrixD matr(3,3);
  for( int i=0; i < uniqm.Count(); i++ )  {
    uniqm[i][0][3] = 0;
    uniqm[i][1][3] = 0;
    uniqm[i][2][3] = 0;
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
            
