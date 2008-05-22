// sym_bark.cpp : Defines the entry point for the console application.
//

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
#include "etime.h"

#include <iostream>
#include "simple_math.h"

using namespace std;




int main(int argc, char* argv[])  {
  const int olc = 100000, ilc = 5000;
  double v[3];
  double m[3][3] = { {-1.0, 0.0, 0.0}, {1.0,0.0,-1.0}, {0.0, 0.0, -1.0}};
  double **nm = new double*[3];  nm[0] = new double[3];  nm[1] = new double[3];  nm[2] = new double[3];  
  nm[0][0] = -1;  nm[0][1] = nm[0][2] = 0;
  nm[1][0] = 1;   nm[1][1] = 0;  nm[1][2] = -1;
  nm[2][0] = 0;  nm[2][1] = 0;  nm[2][2] = -1;
  double *pm = &m[0][0];
  v[0] = v[1] = v[2] = 10.5;
  cout << "Performing " << (__int64)olc*ilc << " operations\n";
  cout << "Simple multiplication with inline function, passing m[][]\n";
  uint64_t st = TETime::msNow();
  double rv = 0;
  for( int i=0; i < olc; i++ )  {
    for( int j=0; j < ilc; j++ )  {
      SimpleMath::mul_i(v, m);
    }
  }
  st = TETime::msNow() - st;
  cout << st << "ms\n";

  cout << "Simple multiplication with inline function, passing &m[0][0]\n";
  st = TETime::msNow();
  for( int i=0; i < olc; i++ )  {
    for( int j=0; j < ilc; j++ )  {
      SimpleMath::mul_i(v, pm);
    }
  }
  st = TETime::msNow() - st;
  cout << st << "ms\n";

  cout << "Simple multiplication with inline function, passing m** created with new\n";
  st = TETime::msNow();
  for( int i=0; i < olc; i++ )  {
    for( int j=0; j < ilc; j++ )  {
      SimpleMath::mul_i(v, nm);
    }
  }
  st = TETime::msNow() - st;
  cout << st << "ms\n";

  cout << "Simple multiplication with 'for' loop function, passing m[][]\n";
  st = TETime::msNow();
  for( int i=0; i < olc; i++ )  {
    for( int j=0; j < ilc; j++ )  {
      SimpleMath::mul_f(v, m);
    }
  }
  st = TETime::msNow() - st;
  cout << st << "ms\n";

  cout << "Simple multiplication with 'for' l0op function, passing &m[0][0]\n";
  st = TETime::msNow();
  for( int i=0; i < olc; i++ )  {
    for( int j=0; j < ilc; j++ )  {
      SimpleMath::mul_f(v, pm);
    }
  }
  st = TETime::msNow() - st;
  cout << st << "ms\n";

  cout << "Simple multiplication with 'for' l0op function, passing m** created with new\n";
  st = TETime::msNow();
  for( int i=0; i < olc; i++ )  {
    for( int j=0; j < ilc; j++ )  {
      SimpleMath::mul_f(v, nm);
    }
  }
  st = TETime::msNow() - st;
  cout << st << "ms\n";
  
  cout << "Using inline fastsymm with the same matrix\n";
  st = TETime::msNow();
  for( int i=0; i < olc; i++ )  {
    for( int j=0; j < ilc; j++ )  {
      MultDA101201110(v);
    }
  }
  st = TETime::msNow() - st;
  cout << st << "ms\n";
  typedef void (*DA)(register double* v);
  DA fsf = &MultDA101201110;
  cout << "Using fastsymm function pointer with the same matrix\n";
  st = TETime::msNow();
  for( int i=0; i < olc; i++ )  {
    for( int j=0; j < ilc; j++ )  {
      fsf(v);
    }
  }
  st = TETime::msNow() - st;
  cout << st << "ms\n";

  TFSymmI* fsi = new TFSymmImp;
  cout << "Using fastsymm virtual function implementaton with the same matrix\n";
  st = TETime::msNow();
  for( int i=0; i < olc; i++ )  {
    for( int j=0; j < ilc; j++ )  {
      fsi->Mult(v);
    }
  }
  st = TETime::msNow() - st;
  cout << st << "ms\n";

  cin.get();
  cout << v[0] << ';' << v[1] << ';' << v[2];
  delete [] nm[0];
  delete [] nm[1];
  delete [] nm[2];
  delete [] nm;
  delete fsi;
	return 0;
}

