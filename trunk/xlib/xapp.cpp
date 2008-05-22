#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "xapp.h"
#include "hkl.h"
#include "ins.h"
#include "cif.h"
#include "p4p.h"
#include "crs.h"

#include "egc.h"
#include "log.h"

#include "xmacro.h"
#include "symmlib.h"
#include "sr_fft.h"

TXApp::TXApp(const olxstr &basedir) : TBasicApp(basedir), Library(EmptyString, this)  {
  try  {
    FAtomsInfo = new TAtomsInfo( BaseDir() + "ptablex.dat" );
  }
  catch( const TIOExceptionBase &exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  FXFile = new TXFile(FAtomsInfo);

  DefineState( psFileLoaded, "Loaded file is expected");
  DefineState( psCheckFileTypeIns, "INS file is expected");
  DefineState( psCheckFileTypeCif, "CIF file is expected");
  DefineState( psCheckFileTypeP4P, "P4P file is expected");
  DefineState( psCheckFileTypeCRS, "CRS file is expected");

  XLibMacros::Export(Library);
}
//..............................................................................
TXApp::~TXApp()  {
  delete FXFile;
  delete FAtomsInfo;
}
//..............................................................................
const olxstr& TXApp::LocateHklFile()  {
  if( XFile().GetLastLoader() == NULL )  return EmptyString;

  olxstr &HklFN = TEGC::New<olxstr>( XFile().GetLastLoader()->GetHKLSource() );
  if( TEFile::FileExists( HklFN ) )  return HklFN;
  HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkl");
  if( TEFile::FileExists( HklFN ) )  return HklFN;
  HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "raw");
  if( TEFile::FileExists(HklFN) )  {
    THklFile Hkl;
    Hkl.LoadFromFile(HklFN);
    HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkl");
    for( int i=0; i < Hkl.RefCount(); i++ )  {
      Hkl[i].SetI( (double)Round(Hkl[i].GetI())/100.0 );
      Hkl[i].SetS( (double)Round(Hkl[i].GetS())/100.0 );
    }
    Hkl.SaveToFile( HklFN );
    GetLog().Info("The scaled hkl file is prepared");
    return HklFN;
  }
  else  {  // check for stoe format
    HklFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkl");
    olxstr HkcFN = TEFile::ChangeFileExt(XFile().GetFileName(), "hkc");
    if( TEFile::FileExists( HkcFN ) )  {
      TEFile::Copy( HkcFN, HklFN );
      return HklFN;
    }
  }
  return EmptyString;
}
//..............................................................................
bool TXApp::CheckProgramState(unsigned int specialCheck)  {
 if( specialCheck & psFileLoaded )
   return XFile().GetLastLoader() != NULL;
 else if( specialCheck & psCheckFileTypeIns )
   return (XFile().GetLastLoader() == NULL) ? false : EsdlInstanceOf(*XFile().GetLastLoader(), TIns);
 else if( specialCheck & psCheckFileTypeP4P )
   return (XFile().GetLastLoader() == NULL) ? false : EsdlInstanceOf(*XFile().GetLastLoader(), TP4PFile);
 else if( specialCheck & psCheckFileTypeCRS )
   return (XFile().GetLastLoader() == NULL) ? false : EsdlInstanceOf(*XFile().GetLastLoader(), TCRSFile);
 else if( specialCheck & psCheckFileTypeCif )
   return (XFile().GetLastLoader() == NULL) ? false : EsdlInstanceOf(*XFile().GetLastLoader(), TCif);
  throw TFunctionFailedException(__OlxSourceInfo, "undefined state");
}
//..............................................................................
void TXApp::CalcSF(const TRefList& refs, TArrayList<TEComplex<double> >& F)  {
  // initialise newly created atoms
  XFile().UpdateAsymmUnit();
  TAsymmUnit& au = XFile().GetAsymmUnit();
  const TMatrixD& hkl2c = au.GetHklToCartesian();
  TMatrixD abc2xyz( au.GetCellToCartesian()), xyz2abc( au.GetCartesianToCell());
  abc2xyz.Transpose();
  xyz2abc.Transpose();
  // space group matrix list
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(au);
  if( sg == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "unknown spacegroup");
  TMatrixDList ml, allm;
  sg->GetMatrices(ml, mattAll);

  allm.SetCapacity(ml.Count()*2);
  // prepare list of M and Mt for ellispoids calculations
  for( int i=0; i < ml.Count(); i++ )  {
    TMatrixD& m  = allm.AddNew(3,3);
    TMatrixD& mt = allm.AddNew(3,3);
    for( int j=0; j < 3; j++ )
      for( int k=0; k < 3; k++ )
        mt[k][j] = m[j][k]  = ml[i][j][k];
//    m  = abc2xyz*m*xyz2abc;
//    for( int j=0; j < 3; j++ )
//      for( int k=0; k < 3; k++ )
//        mt[k][j] = m[j][k];
  }
  TVectorD quad(6);
  TMatrixD ElpM(3,3), SymM(3,3);
  const static double EQ_PI = 8*QRT(M_PI);
  const static double T_PI = 2*M_PI;
  const static double TQ_PI = 2*QRT(M_PI);

  // the thermal ellipsoid scaling factors
  double BM[6] = {hkl2c[0].Length(), hkl2c[1].Length(), hkl2c[2].Length(), 0, 0, 0};
  BM[3] = BM[1]*BM[2];
  BM[4] = BM[0]*BM[2];
  BM[5] = BM[0]*BM[1];
  BM[0] *= BM[0];
  BM[1] *= BM[1];
  BM[2] *= BM[2];

  TScattererLib scat_lib(9);
  TPtrList<TBasicAtomInfo> bais;
  TPtrList<TCAtom> alist;
  double *Ucifs = new double[6*ml.Count()*au.AtomCount() + 1];
  TTypeList< AnAssociation2<TLibScatterer*, double> > scatterers;
  for( int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  continue;
    int ind = bais.IndexOf( &ca.GetAtomInfo() );
    if( ind == -1 )  {
      if( ca.GetAtomInfo() == iDeuteriumIndex ) // treat D as H
        scatterers.AddNew<TLibScatterer*, double>(scat_lib.Find('H'), 0.0 );
      else 
        scatterers.AddNew<TLibScatterer*, double>(scat_lib.Find(ca.GetAtomInfo().GetSymbol()), 0.0 );
      
      ind = scatterers.Count() -1;
      if( scatterers[ind].GetA() == NULL ) {
        delete [] Ucifs;
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("could not locate scatterer: ") << ca.GetAtomInfo().GetSymbol() );
      }
      bais.Add( &ca.GetAtomInfo() );
    }
    ca.SetTag(ind);
    alist.Add(&ca); 
    TEllipsoid* elp = ca.GetEllipsoid();
    if( elp != NULL )  {
      elp->GetQuad(quad);
      au.UcartToUcif(quad);
      SymM[0][0] = quad[0];  SymM[1][1] = quad[1];  SymM[2][2] = quad[2];
      SymM[1][2] = SymM[2][1] = quad[3];
      SymM[0][2] = SymM[2][0] = quad[4];
      SymM[0][1] = SymM[1][0] = quad[5];
      int ind = (alist.Count()-1)*ml.Count()*6;
      for( int j=0; j < allm.Count(); j+=2 )  {
        ElpM = allm[j]*SymM*allm[j+1];
        quad[0] = ElpM[0][0];  quad[1] = ElpM[1][1];  quad[2] = ElpM[2][2];
        quad[3] = ElpM[1][2];  quad[4] = ElpM[0][2];  quad[5] = ElpM[0][1];
        for( int k=0; k < 6; k++ )  {
          Ucifs[ind+k] = quad[k];
          Ucifs[ind+k] *= BM[k];
          Ucifs[ind+k] *= -TQ_PI;
        }
        ind += 6;
      }
    }
    else  {
      const int ind = (alist.Count()-1)*ml.Count()*6;
      if( ca.GetAtomInfo() == iHydrogenIndex && ca.GetUisoVar() < 0 )  {
        int ai = ca.GetAfixAtomId();
        if( ai == -1 )  {
          delete Ucifs;
          throw TFunctionFailedException(__OlxSourceInfo, "bad Uiso");
        }
        Ucifs[ind] = fabs(au.GetAtom(ai).GetUiso()*ca.GetUisoVar());
      }
      else
        Ucifs[ind] = ca.GetUiso();
      //Ucifs[ind] *= Ucifs[ind]; 
      Ucifs[ind] *= -EQ_PI;
    }
  }
  
  TVPointD hkl;
  for( int i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    ref.MulHkl(hkl, hkl2c);
    const double d_s2 = hkl.QLength()*0.25;
    for( int j=0; j < scatterers.Count(); j++)
      scatterers[j].B() = scatterers[j].GetA()->Calc_sq(d_s2);
    double a = 0, b = 0;
    for( int j=0; j < alist.Count(); j++ )  {
      const TVPointD& crd = alist[j]->GetCCenter();
      double la=0, lb=0;
      for( int k=0; k < ml.Count(); k++ )  {
        const TMatrixD& mt = ml[k];
        ref.MulHklT(hkl, mt);
        double tv =  hkl[0]*mt[0][3];  // scattering vector + phase shift
               tv += hkl[1]*mt[1][3];
               tv += hkl[2]*mt[2][3];
               tv += hkl[0]*crd[0];
               tv += hkl[1]*crd[1];
               tv += hkl[2]*crd[2];
        tv *= T_PI;
        double ca, sa;
        SinCos(tv, &sa, &ca);
        if( alist[j]->GetEllipsoid() != NULL )  {
          double* Q = &Ucifs[(j*ml.Count()+k)*6];  // pick up the correct ellipsoid
          double B = (Q[0]*hkl[0]*hkl[0] + Q[1]*hkl[1]*hkl[1] + Q[2]*hkl[2]*hkl[2] + 
            2*Q[3]*hkl[1]*hkl[2] + 2*Q[4]*hkl[0]*hkl[2] + 2*Q[5]*hkl[0]*hkl[1]);
          B = exp( B );
          la += B*ca;
          lb += B*sa;
        }
        else  {
          la += ca;
          lb += sa;
        }
      }
      double scv = scatterers[ alist[j]->GetTag() ].GetB();
      if( alist[j]->GetEllipsoid() == NULL )  {
        scv *= exp( Ucifs[j*ml.Count()*6]*d_s2 );
      }
      scv *= alist[j]->GetOccp();
      a += scv * la;
      b += scv * lb;
    }
    F[i].Re() = a;
    F[i].Im() = b;
  }
  delete [] Ucifs;
  au.InitAtomIds();
}

