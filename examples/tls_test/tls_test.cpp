// testa.cpp : Defines the entry point for the console application.

//#include "stdafx.h"
//#include "conio.h"

#include "exception.h"
#include "efile.h"
#include "estrlist.h"
#include "bapp.h"
#include "log.h"

#include "ins.h"
#include "asymmunit.h"
#include "catom.h"
#include "satom.h"
#include "ellipsoid.h"
#include "symmlib.h"
#include "xapp.h"
#include "svd.h"
#include "inv.h"
#include "tls.h"
#include "outstream.h"

#include <iostream>



int __cdecl main(int argc, char* argv[])
{
  try
  {
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
    TXFile& xf = XApp.XFile();

  double cellParameters[12];
  for ( short i = 0; i < 3; i++ ){
    cellParameters[i]= xf.GetAsymmUnit().Axes()[i].GetV();
    cellParameters[i+3]= xf.GetAsymmUnit().Angles()[i].GetV();
    cellParameters[i+6]= xf.GetAsymmUnit().Axes()[i].GetE();
    cellParameters[i+9]= xf.GetAsymmUnit().Angles()[i].GetE();
    std::cout << "Param " << i << " = " << cellParameters[i]
    << ", error = " << cellParameters[i+6]*M_PI/180 << std::endl;
  }
    std::cout << "\n";

  TStrList atoms;
  atoms.Add("C25");
  atoms.Add("C26");
  atoms.Add("C27");
  atoms.Add("C28");
  atoms.Add("C29");
  atoms.Add("C30");
  
  
  TSAtomPList atomList; 
  
  for( int i=0; i < xf.GetLattice().AtomCount(); i++ )  {
    for( int j=0; j < atoms.Count(); j++ )  {
      if( xf.GetLattice().GetAtom(i).GetLabel() == atoms.String(j) )  {
        atomList.Add( &xf.GetLattice().GetAtom(i));

        TSAtom &ca = xf.GetLattice().GetAtom(i);
        printf("%s \t %f \t %f \t %f\n", ca.GetLabel().c_str(),
          ca.CCenter()[0],
          ca.CCenter()[1],
          ca.CCenter()[2]);

      }
    }
  }
  
  std::cout << "\n Starting TLS Analysis.\n";
  TLS tls(atomList,cellParameters);
  std::cout << "\nTLS Analysis complete.\n" 
    << "Figures of Merit, R1, R2 root(chi^2): (" << tls.GetFoM()[0]
  << ", " << tls.GetFoM()[1]
  << ", " << tls.GetFoM()[2] << ")\n";

  std::cout << "\n" 
    << "Origin: (" << tls.GetOrigin()[0]
  << ", " << tls.GetOrigin()[1]
  << ", " << tls.GetOrigin()[2] << ")\n";


  for(int i = 0 ; i <atomList.Count(); i++){
    TVectorD quad(6);
    TMatrixD UijCell(3,3);

    std::cout << "\n-----------------------------\n\nAtom " 
          << i << "\nUij matrix from refinement";
    TMatrixD Uij(3,3);
    {
      TVectorD quad(6);
    
      TSAtom *anAtom = atomList[i];
      anAtom->GetEllipsoid()->GetQuad(quad);
      Uij[0][0] = quad[0];
      Uij[0][1] = Uij[1][0] = quad[5];
      Uij[0][2] = Uij[2][0] = quad[4];
      Uij[1][1] = quad[1];
      Uij[1][2] = Uij[2][1] = quad[3];
      Uij[2][2] = quad[2];
    }
    Uij.Print();

    /*anAtom->GetEllipsoid()->GetQuad(quad);

    UijCell[0][0] = quad[0];
    UijCell[0][1] = UijCell[1][0] = quad[5];
    UijCell[0][2] = UijCell[2][0] = quad[4];
    UijCell[1][1] = quad[1];
    UijCell[1][2] = UijCell[2][1] = quad[3];
    UijCell[2][2] = quad[2];

    UijCell.Print();*/

    std::cout << "\nUij matrix from TLS:";
    
    
    {
      TVectorD quad(6);
      tls.GetElpList()[i].GetQuad(quad); // .GetMatrix().Print();      
      Uij[0][0] = quad[0];
      Uij[0][1] = Uij[1][0] = quad[5];
      Uij[0][2] = Uij[2][0] = quad[4];
      Uij[1][1] = quad[1];
      Uij[1][2] = Uij[2][1] = quad[3];
      Uij[2][2] = quad[2];
    }
    Uij.Print();
    

    std::cout << "EigenVals: "
          << tls.GetElpList()[i].GetSX() << ", "
            << tls.GetElpList()[i].GetSY() << ", "
          << tls.GetElpList()[i].GetSZ() << "\n";

  }
  

  //TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( *xf.GetAsymmUnit() );
  //if( sg != NULL )
  //  printf("\nSpaceGroup: %s", sg->GetName().c_str() );
  }
  catch( TExceptionBase& exc )
  {
    printf("An exception occured: %s\n", EsdlObjectName(exc).c_str() );
    printf("details: %s\n", exc.GetException()->GetFullMessage().c_str() );
  }

  printf("\n\n Press return to exit...");
  std::cin.get();

  return 0;
}

/*
  TMatrixD rFractionToCart(3,3); 
  rFractionToCart[0][0] = a;
  rFractionToCart[0][1] = b*cos(gamma);
  rFractionToCart[1][1] = b*sin(gamma);
  rFractionToCart[0][2] = c*cos(beta);
  rFractionToCart[1][2] = c*(cos(alpha)- cos(beta)*cos(gamma))/sin(gamma);
  rFractionToCart[2][2] = c*sqrt(1 - (rFractionToCart[1][2]/c)*(rFractionToCart[1][2]/c)
    - cos(beta)*cos(beta)); // c * SqRoot( 1 - r12^2 -r02^2)
  
  std::cout << "rFracToCart:" << std::endl;
  rFractionToCart.Print();   // Olex can calc this - see rCart below
*/ 
/*
int n=0;
  bool res = inverse(testM,testM.gethighbound(n) - testM.getlowbound(n) +1 );
  std::cout << "Inverse funct call success = " << res << std::endl;
  
  std::cout << "\nInverse matrix:" <<std::endl;
  for (short i = 1; i <= 3; i++){
    for (short j=1 ; j<= 3; j++){
      std::cout << testM(i,j) <<",  ";
    }
    std::cout << "\n" <<std::endl;
  }
  
  */

/*
  
*/

