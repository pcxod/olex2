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

///////////////////////////////////////////////////////////////////////
void ExportSymmLib();
void ExportSymmLibB();  // MSVC crashes on this code...
//void ExportSymmLibC();
void ExportSymmLibD();  // new fastsymm output
void ExportBAI(TAtomsInfo& ais);
void ExportBAIA(TAtomsInfo& ais, TScattererLib& scl);



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
    TScattererLib scl(9);
    TSymmLib& sl = *TSymmLib::GetInstance();
    if( &sl == NULL ) 
      throw TFunctionFailedException(__OlxSourceInfo, "symmlib is not initialised");
    TIns* ins = new TIns;
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
    //ExportBAI( *XApp.AtomsInfo() );
    //ExportBAIA( *XApp.AtomsInfo(), scl );
    //ExportSymmLibC();
    ExportSymmLibD();
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
void HallSymbolTranslation(const smatd& m, olxstr& hs)  {
  static TTypeList<AnAssociation2<vec3d, olxstr> > trans;
  if( trans.IsEmpty() )  {
    trans.AddNew( vec3d(0.5, 0, 0), "a");
    trans.AddNew( vec3d(0, 0.5, 0), "b");
    trans.AddNew( vec3d(0, 0, 0.5), "c");
    trans.AddNew( vec3d(0.5, 0.5, 0.5), "n");
    trans.AddNew( vec3d(0.25, 0, 0), "u");
    trans.AddNew( vec3d(0, 0.25, 0), "v");
    trans.AddNew( vec3d(0, 0, 0.25), "w");
    trans.AddNew( vec3d(0.25, 0.25, 0.25), "d");
  }
  for( int j=0; j < trans.Count(); j++ )  {
    if( trans[j].GetA() == m.t )  {
      hs << trans[j].GetB();
      return;
    }
  }
  for( int j=0; j < trans.Count(); j++ )  {
    for( int k=j+1; k < trans.Count(); k++ )  {
      if( trans[j].GetA()+trans[k].GetA() == m.t )  {
        hs << trans[j].GetB() << trans[k].GetB();
        return;
      }
    }
  }
  for( int j=0; j < trans.Count(); j++ )  {
    for( int k=j+1; k < trans.Count(); k++ )  {
      for( int l=k+1; l < trans.Count(); l++ )  {
        if( trans[j].GetA()+trans[k].GetA()+trans[l].GetA() == m.t )  {
          hs << trans[j].GetB() << trans[k].GetB() << trans[l].GetB();
          return;
        }
      }
    }
  }
  TBasicApp::GetLog() << "t_fail: " << m.t.ToString() << '\n';
}
void HallSymbolTranslationR(const smatd& m, olxstr& hs, int order)  {
  double v = m.t[0] != 0 ? m.t[0] : (m.t[1] != 0 ? m.t[1] : m.t[2]);
  if( v == 0 )  return;
  bool processed = false;
  if( order <= 2 || m.t.Length() != v )  {
    HallSymbolTranslation(m, hs);
    return;
  }
  if( order == 3 )  {
    if( fabs(v - 1./3) < 0.05 )  {
      hs << '1';
      processed = true;
    }
    else if( fabs(v - 2./3) < 0.05 )  {
      hs << '2';
      processed = true;
    }
  }
  else if( order == 4 )  {
    if( fabs(v - 1./4) < 0.05 )  {
      hs << '1';
      processed = true;
    }
    else if( fabs(v - 3./4) < 0.05 )  {
      hs << '3';
      processed = true;
    }
  }
  else if( order == 6 )  {
    if( fabs(v - 1./6) < 0.05 )  {
      hs << '1';
      processed = true;
    }
    else if( fabs(v - 2./6) < 0.05 )  {
      hs << '2';
      processed = true;
    }
    else if( fabs(v - 4./6) < 0.05 )  {
      hs << '4';
      processed = true;
    }
    else if( fabs(v - 5./6) < 0.05 )  {
      hs << '5';
      processed = true;
    }
  }
  if( !processed )
    HallSymbolTranslation(m, hs);
}
int HallSymbolFindR(olxstr& hs, TPtrList<const smatd>& matrs, const TTypeList<AnAssociation2<mat3d, olxstr> >& rot, bool full)  {
  int previous = 0;
  for( int i=0; i < rot.Count(); i++ )  {
    for( int j=0; j < matrs.Count(); j++ )  {
      if( matrs[j] == NULL )  continue;
      const smatd& m = *matrs[j];
      if( rot[i].GetA() == m.r )  {
        hs << ' ';
        if( full )  hs << rot[i].GetB();
        else        hs << rot[i].GetB().CharAt(0);
        previous = rot[i].GetB().CharAt(0)-'0';
        HallSymbolTranslationR(m, hs, previous);
        matrs[j] = NULL;
        break;
      }
      else if( rot[i].GetA() == -m.r )  {
        hs << " -";
        if( full )  hs << rot[i].GetB();
        else        hs << rot[i].GetB().CharAt(0);
        previous = rot[i].GetB().CharAt(0)-'0';
        HallSymbolTranslationR(m, hs, previous);
        matrs[j] = NULL;
        break;
      }
    }
    if( previous != 0 )  break;
  }
  if( previous != 0 )
    matrs.Pack();
  return previous;
}
olxstr HallSymbol(const TSpaceGroup& sg)  {
  olxstr hs( sg.IsCentrosymmetric() ? olxstr('-') << sg.GetLattice().GetSymbol() : sg.GetLattice().GetSymbol());
  if( sg.MatrixCount() == 0 )  {
    hs << ' ' << '1';
  }
  else  {
    static TTypeList<AnAssociation2<mat3d, olxstr> > rotx, roty, rotz, 
      rotx1, roty1, rotz1, rot3;
    if( rotx.IsEmpty() )  {
      rotx.AddNew( mat3d( 1, 0, 0,   0,-1, 0,   0, 0,-1), "2x" );
      rotx.AddNew( mat3d( 1, 0, 0,   0, 0,-1,   0, 1,-1), "3x" );
      rotx.AddNew( mat3d( 1, 0, 0,   0, 0,-1,   0, 1, 0), "4x" );
      rotx.AddNew( mat3d( 1, 0, 0,   0, 1,-1,   0, 1, 0), "6x" );

      roty.AddNew( mat3d( 0, 0, 1,   0, 1, 0,  -1, 0, 1), "6y" );
      roty.AddNew( mat3d( 0, 0, 1,   0, 1, 0,  -1, 0, 0), "4y" );
      roty.AddNew( mat3d(-1, 0, 1,   0, 1, 0,  -1, 0, 0), "3y" );
      roty.AddNew( mat3d(-1, 0, 0,   0, 1, 0,   0, 0,-1), "2y" );

      rotz.AddNew( mat3d( 1,-1, 0,   1, 0, 0,   0, 0, 1), "6z" );
      rotz.AddNew( mat3d( 0,-1, 0,   1, 0, 0,   0, 0, 1), "4z" );
      rotz.AddNew( mat3d( 0,-1, 0,   1,-1, 0,   0, 0, 1), "3z" );
      rotz.AddNew( mat3d(-1, 0, 0,   0,-1, 0,   0, 0, 1), "2z" );
      // x
      rotx1.AddNew( mat3d(-1, 0,  0,   0,  0, -1,   0,  -1,  0), "2" );
      rotx1.AddNew( mat3d(-1, 0,  0,   0,  0,  1,   0,   1,  0), "2\"" );
      // y
      roty1.AddNew( mat3d( 0, 0, -1,   0, -1,  0,  -1,   0,  0), "2" );
      roty1.AddNew( mat3d( 0, 0,  1,   0, -1,  0,   1,   0,  0), "2\"" );
      // z
      rotz1.AddNew( mat3d( 0,-1,  0,  -1,  0,  0,   0,   0, -1), "2" );
      rotz1.AddNew( mat3d( 0, 1,  0,   1,  0,  0,   0,   0, -1), "2\"" );

      rot3.AddNew( mat3d( 0, 0,  1,   1,  0,  0,   0,   1,  0), "3*" );
    }
    TPtrList<const smatd> matrs;
    for( int i=0; i < sg.MatrixCount(); i++ )
      matrs.Add( &sg.GetMatrix(i) );
    
    int rz = HallSymbolFindR(hs, matrs, rotz, false);
    // c direction
    if( rz != 0 )  {
      int rx = HallSymbolFindR(hs, matrs, rotx, false);
      if( rx != 0 )  {
        if( HallSymbolFindR(hs, matrs, rot3, false) == 0 )
          HallSymbolFindR(hs, matrs, rotx1, true);
      }
      else
        HallSymbolFindR(hs, matrs, rotz1, true);
    }
    else  { // no c direction
      HallSymbolFindR(hs, matrs, rotx, true);
      HallSymbolFindR(hs, matrs, roty, true);
      if( HallSymbolFindR(hs, matrs, rot3, true) != 0 )
        HallSymbolFindR(hs, matrs, rotz1, true);
    }
  }
  return hs;
}
int CodeGen(TSpaceGroup& sg, TStrList& out, olxstr& hs)  {
  hs = HallSymbol(sg);
  if( !(sg.GetHallSymbol() == hs) )  {
    TBasicApp::GetLog() << (olxstr(sg.GetNumber()) << ":" << sg.GetName()).Format(15, true, ' ') <<  
      hs << " vs. " << sg.GetHallSymbol() << '\n';
  }
  smatd_list ml;
  sg.GetMatrices(ml, mattAll);
  out.Add("  template <class pt, class vt> static inline void GenPos(const pt& v, vt& res)  {");
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
  //////////////////////////////
  out.Add("  template <class pt, class vt> static inline void GenHkl(const pt& v, vt& res)  {");
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
  }
  out.Add("  }");
  ///////////////////////
  out.Add("  template <class pt, class vt, class vt1> static inline void GenHkl(const pt& v, vt& res, vt1& phase)  {");
  for( int i=0; i < ml.Count(); i++ )  {
    smatd& m = ml[i];
    for( int j=0; j < 3; j++ )  {
      olxstr& str = out.Add("    res[") << i << "][" << j << "] = ";
      bool added = false;
      for( int k=0; k < 3; k++ )  {
        if( m.r[j][k] != 0 )  {  // normal form here for hkl
          str << ((m.r[j][k] > 0) ? (added ? (olxstr("+v[") << k << ']') : (olxstr("v[") << k << ']')) : (olxstr("-v[") << k << ']'));
          added = true;
        }
      }
      str << ';';
    }
    olxstr& str = out.Add("    phase[") << i << "] = ";
    bool added = false;
    olxstr m_str[3];
    for( int j=0; j < 3; j++ )  {
      if( m.t[j] != 0 )  {  // transposed form for hkl
        int v = Round(m.t[j]*12), base = 12;
        int denom = esdl::gcd(v, base);
        if( denom != 1 )  {
          v /= denom;
          base /= denom;
        }
        m_str[j] << v << "./"  << base;
      }
    }
    if( m_str[0] == m_str[1] && m_str[0] == m_str[2] && !m_str[0].IsEmpty() )  {
      added = true;
      if( m_str[0].CharAt(0) == '-' )  
        str <<  "-(res[#][0]+res[#][1]+res[#][2])*" << m_str[0].SubStringFrom(1);
      else
        str <<  "(res[#][0]+res[#][1]+res[#][2])*" << m_str[0];
    }
    else if( m_str[0] == m_str[1] && !m_str[0].IsEmpty() )  {
      added = true;
      if( m_str[0].CharAt(0) == '-' )  
        str << "-(res[#][0]+res[#][1])*" << m_str[0].SubStringFrom(1);
      else
        str << "(res[#][0]+res[#][1])*" << m_str[0];
      if( !m_str[2].IsEmpty() )  {
        if( m_str[2].CharAt(0) == '-' )  
          str <<  "-res[#][2]*" << m_str[2].SubStringFrom(1);
        else
          str <<  "+res[#][2]*" << m_str[2];
      }
    }
    else if( m_str[0] == m_str[2] && !m_str[0].IsEmpty() )  {
      added = true;
      if( m_str[0].CharAt(0) == '-' )  
        str << "-(res[#][0]+res[#][2])*" << m_str[0].SubStringFrom(1);
      else
        str << "(res[#][0]+res[#][2])*" << m_str[0];
      if( !m_str[1].IsEmpty() )  {
        if( m_str[1].CharAt(0) == '-' )  
          str <<  "-res[#][1]*" << m_str[1].SubStringFrom(1);
        else
          str <<  "+res[#][1]*" << m_str[1];
      }
    }
    else if( m_str[1] == m_str[2] && !m_str[1].IsEmpty() )  {
      added = true;
      if( !m_str[0].IsEmpty() )  {
        if( m_str[0].CharAt(0) == '-' )  
          str <<  "-res[#][0]*" << m_str[0].SubStringFrom(1);
        else
          str <<  "res[#][0]*" << m_str[0];
      }
      if( m_str[1].CharAt(0) == '-' )  
        str << "-(res[#][1]+res[#][2])*" << m_str[1].SubStringFrom(1);
      else  {
        if( m_str[0].IsEmpty() )  
          str << "(res[#][1]+res[#][2])*" << m_str[1];
        else  
          str << "+(res[#][1]+res[#][2])*" << m_str[1];
      }
    }
    if( !added && (m.t[0] != 0 || m.t[1] != 0 || m.t[2] != 0) )  {  // generic case
      for( int j=0; j < 3; j++ )  {
        if( m.t[j] != 0 )  {  // transposed form for hkl
          int v = Round(m.t[j]*12), base = 12;
          int denom = esdl::gcd(v, base);
          if( denom != 1 )  {
            v /= denom;
            base /= denom;
          }
          olxstr mult(v);  mult << "./"  << base;
          str << ((m.t[j] > 0) ? (added ? (olxstr("+res[#][") << j << ']') : (olxstr("res[#][") << j << ']')) : (olxstr("-res[#][") << j << ']'));
          str << '*' << mult;
          added = true;
        }
      }
    }
    if( !added )  str << " 0";
    else          str.Replace('#', olxstr(i));
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
            
struct chem_lib_Neutron_Scattering {
  compd coh;
  double xs; 
  chem_lib_Neutron_Scattering(double _coh_re, double _coh_im, double _xs) :
  coh(_coh_re, _coh_im), xs(_xs)  {  }
};
struct chem_lib_Isotope {  
  double Mr, W;  
  const chem_lib_Neutron_Scattering* neutron_scattering;
};
struct chem_lib_Anomalous_Henke {  double energy, fp, fdp;  };
struct chem_lib_Gaussians {  
  double a1, a2, a3, a4, b1, b2, b3, b4, c;  
  chem_lib_Gaussians(double _a1, double _a2, double _a3, double _a4, 
    double _b1, double _b2, double _b3, double _b4, double _c) :
    a1(_a1), a2(_a2), a3(_a3), a4(_a4), 
      b1(-_b1), b2(-_b2), b3(-_b3), b4(-_b4), c(_c)  {  }

  inline double calc_sq(double sqv) const {
    return a1*exp(b1*sqv) + a2*exp(b2*sqv) + a3*exp(b3*sqv) + a4*exp(b4*sqv) + c;
  }
};

/*
struct Label_Z_Henke  {
  const char* label;
  short z;
  const chem_lib_Anomalous_Henke* henke;
};

#include "henke.h"

struct chem_lib_Element {
  const char* symbol, *name;
  const chem_lib_Gaussians* gaussians;  // 9 elements = 4 gaussians + const
  const chem_lib_Isotope* isotopes;
  const chem_lib_Anomalous_Henke* henke_data; 
  const chem_lib_Neutron_Scattering* neutron_scattering;
  unsigned int def_color;
  short z, isotope_count, henke_count;
  double r_pers, r_bonding, r_sfil;
  chem_lib_Element(const char* _symbol, const char* _name, unsigned int _def_color, short _z, 
    double _r_pers, double _r_bonding, double _r_sfil, const chem_lib_Gaussians* _gaussians, 
    const chem_lib_Isotope* _isotopes, short _isotope_count, 
    const chem_lib_Anomalous_Henke* _henke_data, short _henke_count) :
    symbol(_symbol), name(_name), def_color(_def_color), z(_z), r_pers(_r_pers), 
    r_bonding(_r_bonding), r_sfil(_r_sfil), gaussians(_gaussians), 
    isotopes(_isotopes), isotope_count(_isotope_count),
    henke_data(_henke_data), henke_count(_henke_count){  }

  compd CalcFpFdp(double energy) const  {
    return compd(0);
  }
};

chem_lib_Neutron_Scattering neutron_H(1, 1, 1);
chem_lib_Gaussians gaussians_H(1, 1, 1, 1, 1, 1, 1, 1, 1);
chem_lib_Isotope Isotope_H[] = {{1.0,1.0, &neutron_H}, {1,1, &neutron_H}};
chem_lib_Element Hydrogen("H", "Hydrogen", 6, 1, 0.12, 0.32, 0.82, &gaussians_H, Isotope_H, 2, NULL, 0 );
chem_lib_Element dd[1] = { Hydrogen };

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

struct neutron_sc {
  TBasicAtomInfo* bai;
  int Z;
  double occ, coh_re, coh_im, xs;
  neutron_sc() : Z(0), occ(0), coh_re(0), coh_im(0), xs(0) {}
};
void ParseNumberEx(const olxstr& s, double& re, double &im)  {
  if( s.CharAt(s.Length()-1) == 'i' )  {
    int im_ind = s.LastIndexOf('-');
    im = s.SubStringFrom(im_ind, 1).ToDouble();
    re = s.SubStringTo(im_ind).ToDouble();
  }
  else {
    im = 0;
    int b_ind = s.FirstIndexOf('(');
    if( b_ind >= 0 )
      re = s.ToDouble();
    else
      re = s.SubStringTo(b_ind).ToDouble();
  }
}
void ParseNeutron(const TStrList& l, TTypeList<neutron_sc>& res)  {
  TStrList toks;
  olxstr str_z;
  for( int i=1; i < l.Count(); i++ )  {
    toks.Clear();
    toks.Strtok(l[i], '\t');
    for( int j=0; j < toks.Count(); j++ )
      toks[j] = toks[j].Trim(' ');
    TBasicAtomInfo* bai = TAtomsInfo::GetInstance()->FindAtomInfoBySymbol(toks[0]);
    int z = 0;
    if( bai == 0 )  {
      int j=0;
      str_z = EmptyString;
      while( j < toks[0].Length() && toks[0].CharAt(j) <= '9' && toks[0].CharAt(j) >= '0' )  {
        str_z << toks[0].CharAt(j);
        j++;
      }
      toks[0] = toks[0].SubStringFrom(j);
      bai = TAtomsInfo::GetInstance()->FindAtomInfoBySymbol(toks[0]);
      if( bai == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "assert");
      z = str_z.ToInt();
    }
    neutron_sc& r = res.AddNew();
    r.bai = bai;
    r.Z = z;
    if( toks[1].IsNumber() )
      r.occ = toks[1].ToDouble();
    ParseNumberEx(toks[2], r.coh_re, r.coh_im);
    int b_ind = toks[3].FirstIndexOf('(');
    if( b_ind >= 0 )
      r.xs = toks[3].SubStringTo(b_ind).ToDouble();
    else
      r.xs = toks[3].ToDouble();
  }
}
void ExportBAIA(TAtomsInfo& ais, TScattererLib& scl)  {
  TStrList out, isotopes, elements, gaussians, henke, neutron, arr;
  TStrList neutron_l_str;
  TTypeList<neutron_sc> neutron_l;
  neutron_l_str.LoadFromFile("e:/tmp/neutron_scatt.txt");
  ParseNeutron(neutron_l_str, neutron_l);
  char bf[80], bf1[80];
  _set_output_format(_TWO_DIGIT_EXPONENT);
  arr.Add("chem_lib_Element chem_lib_Elements[") << ais.Count() << "] = {";
  for( int i=0; i < ais.Count(); i++ )  {
    TBasicAtomInfo& bai = ais.GetAtomInfo(i);
    TLibScatterer* sc = scl.Find( bai.GetSymbol() );
    chem_lib_Anomalous_Henke const* henke_data = NULL;
    int henke_count = 0;
    neutron_sc* neutron_data = NULL;
    for( int j =0; HenkeTables[j].label != 0; j++ )  {
      if( bai.GetSymbol().Comparei( HenkeTables[j].label) == 0 )  {
        henke_data = HenkeTables[j].henke;
        for( int k=0; HenkeTables[j].henke[k].energy != 0; k++ )
          henke_count++;
        break;
      }
    }
    for( int j=0; j < neutron_l.Count(); j++ )  {
      if( *neutron_l[j].bai == bai && neutron_l[j].Z == 0 )  {
        neutron_data = &neutron_l[j];
        break;
      }
    }
    sprintf(bf, "0x%X", bai.GetDefColor());
    elements.Add("static chem_lib_Element _chem_lib_element_") << bai.GetSymbol() 
      << "(\"" << bai.GetSymbol() << "\", \"" << bai.GetName() << "\", " << bf << ", " << (i+1)  <<
      ", " << bai.GetRad() << ", " << bai.GetRad1() << ", " << bai.GetRad2() << 
      ((sc != NULL) ? (olxstr(", &_chem_lib_gaussians_") << bai.GetSymbol()) : olxstr(", NULL"))  <<
      ((bai.IsotopeCount() != 0) ? (olxstr(", _chem_lib_isotope_") << bai.GetSymbol()) : olxstr(", NULL"))
      << ", " << bai.IsotopeCount() << 
      ((henke_data != NULL) ? (olxstr(", _chem_lib_henke_") << bai.GetSymbol()) : olxstr(", NULL"))
      << ", " << henke_count <<
      ((neutron_data != NULL) ? (olxstr(", &_chem_lib_neutron_") << bai.GetSymbol()) : olxstr(", NULL"))
      << ");";
    if( neutron_data != NULL )  {
      neutron.Add("static const chem_lib_Neutron_Scattering _chem_lib_neutron_") << bai.GetSymbol() << '(' <<
        neutron_data->coh_re << ", " << neutron_data->coh_im << ", " << neutron_data->xs << ");";
    }
    if( bai.IsotopeCount() != 0 )  { 
      isotopes.Add("static const chem_lib_Isotope _chem_lib_isotope_") << bai.GetSymbol() << "[] = {";
      for( int j=0; j < bai.IsotopeCount(); j++ )  {
        neutron_sc* i_n_data = NULL;
        for( int k=0; k < neutron_l.Count(); k++ )  {
          if( *neutron_l[k].bai == bai && neutron_l[k].Z == Round(bai.GetIsotope(j).GetMr()) )  {
            i_n_data = &neutron_l[k];
            break;
          }
        }
        isotopes.Add("  {") << bai.GetIsotope(j).GetMr() << ", " << bai.GetIsotope(j).GetW() 
          << ", " <<
          ( (i_n_data == NULL) ? olxstr("NULL") : (olxstr("&_chem_lib_neutron_") << bai.GetSymbol() << i_n_data->Z) )
          << '}';
        if( (j+1) < bai.IsotopeCount() )
          isotopes.Last().String() << ", ";
        if( i_n_data != NULL )  {
          neutron.Add("static const chem_lib_Neutron_Scattering _chem_lib_neutron_") << bai.GetSymbol() << i_n_data->Z << '(' <<
            i_n_data->coh_re << ", " << i_n_data->coh_im << ", " << i_n_data->xs << ");";
        }
      }
      isotopes.Last().String() << "};";
    }
    if( sc != NULL )  {
      gaussians.Add("static const chem_lib_Gaussians _chem_lib_gaussians_") << bai.GetSymbol() << "(";
      for( int j=0; j < 9; j++ )  {
        gaussians.Last().String() << sc->GetData()[j];
        if( j < 8 )  gaussians.Last().String() << ", ";
      }
      gaussians.Last().String() << ");";
    }
    if( henke_data != NULL )  {
      henke.Add("static const chem_lib_Anomalous_Henke _chem_lib_henke_") << bai.GetSymbol() << "[] = {";
      for( int j=0; j < henke_count; j++ )  {
        if( henke_data[j].fp == NOVAL )
          strcpy(bf, "NOVAL");
        else if( fabs(henke_data[j].fp) < 1e-1 )
          sprintf(bf, "%.5e", henke_data[j].fp);
        else
          sprintf(bf, "%f", henke_data[j].fp);
        if( henke_data[j].fdp == NOVAL )  
          strcpy(bf1, "NOVAL");
        else if( fabs(henke_data[j].fdp) < 1e-1 ) 
          sprintf(bf1, "%.5e", henke_data[j].fdp);
        else
          sprintf(bf1, "%f", henke_data[j].fdp);
        henke.Add("  {") << henke_data[j].energy << ", " << olxstr(bf).TrimFloat() << ", " << olxstr(bf1).TrimFloat() << '}';
        if( (j+1) < henke_count )
          henke.Last().String() << ", ";
      }
      henke.Add("};");
    }
    arr.Add("  _chem_lib_element_") << bai.GetSymbol();
    if( (i+1) < ais.Count() )
      arr.Last().String() << ',';
  }
  arr.Add("};");
  out << neutron << isotopes << gaussians << henke << elements << arr;
  out.SaveToFile("e:/tmp/baiouta.h");
}
*/
/*
#include "bricks.h"
struct SGSM {
  double m[3];
};
struct SG  {
  char* name, *full_name, *hall_symbol, *axis;
  int number, latt;
  char* matrices;
  //int brick[3][2];
};
void ExportSymmLibC()  {
  TSymmLib& sl = *TSymmLib::GetInstance();
  TStrList out;
  out.Add("olx_SGDef olx_SG[") << sl.SGCount() << "]={";
  for( int i=0; i < sl.SGCount(); i++ )  {
    TSpaceGroup& sg = sl.GetGroup(i);
    //brick const * br = NULL;
    for(int j=0; btable[j].sg_number != 0; j++ )  {
      if( sg.GetHallSymbol() == olxstr(btable[j].hall_symbol).Trim(' ') )  {
        br = &btable[j];
        break;
      }
    }
    //if( br == NULL )
    //  TBasicApp::GetLog() << "hmm";
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
*/
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

//#define sg_factory_new(base, clazz, sgName) \
//  (sgName == "p21") ? (base*)(new clazz<ns_P21>) : \
//  (sgName == "p31") ? (base*)(new clazz<ns_P31>) : \
//  (base*)(NULL);


void ExportSymmLibD()  {
  TSymmLib& sl = *TSymmLib::GetInstance();
  TStrList out, def_out, cppout;
  olxstr sg_name, hs;
  def_out.Add("#define FSymmFactory(base, clazz) base* fs_factory##base(const olxstr& sgName) {\\");
  for( int i=0; i < sl.SGCount(); i++ )  {
    sg_name = sl.GetGroup(i).GetName();
    sg_name.Replace('-', '_');
    sg_name.Replace('/', '_');
    sg_name.Replace(':', '_');
    if( i == 0 )  
      def_out.Add("  if( sgName == \"") << sl.GetGroup(i).GetName() << "\" ) return new clazz<FastSG_" << sg_name << ">;\\";
    else
      def_out.Add("  else if( sgName == \"") << sl.GetGroup(i).GetName() << "\" ) return new clazz<FastSG_" << sg_name << ">;\\";
    out.Add("struct FastSG_") << sg_name << " {";
    int mc = CodeGen(sl.GetGroup(i), out, hs);
    out.Add("  static const short size=") << mc << ';'; 
    out.Add("  static const char lattice='") << sl.GetGroup(i).GetLattice().GetSymbol().CharAt(0) << "';"; 
    out.Add("  static const olxstr fullName, hallSymbol;");
    cppout.Add("const olxstr FastSG_") << sg_name << "::fullName(\"" << sl.GetGroup(i).GetFullName() << "\");";
    hs.Replace('"', "\\\"");
    cppout.Add("const olxstr FastSG_") << sg_name << "::hallSymbol(\"" << hs << "\");";
    out.Add("};");
  }
  def_out.Add("  else return NULL);\\");
  def_out.Add("}");
  def_out << out;
  def_out.SaveToFile("e:/tmp/sgouts_x.h");
  cppout.SaveToFile("e:/tmp/sgouts_x.cpp");
}


