#include "xapp.h"
#include "outstream.h"
#include "hkl.h"
#include <iostream>
using namespace std;

int sortHKl(const TReflection* r1, const TReflection* r2)  {
  int d = r1->GetH() - r2->GetH();
  if( d != 0 )  return d;
  d = r1->GetK() - r2->GetK();
  if( d != 0 )  return d;
  return r1->GetL() - r2->GetL();
}

int main(int argc, char* argv[])  {
  try  {
    olxstr bd( TEFile::ExtractFilePath( argv[0] ) );
    if( bd.IsEmpty() )
      bd = TEFile::CurrentDir();

    TEFile::AddTrailingBackslashI( bd );

    TXApp XApp(bd);
    XApp.GetLog().AddStream( new TOutStream(), true );
    if( argc < 3 )  {
      TBasicApp::GetLog() << "Please provide an input and output hkl file names\n";
      return 0;
    }
    THklFile hkl;
    hkl.LoadFromFile(argv[1]);
    TRefList new_refs;
    TRefPList refs;
    new_refs.SetCapacity( hkl.RefCount()*3);
    refs.SetCapacity( hkl.RefCount());
    for( int i=0; i < hkl.RefCount(); i++ )
      refs.Add(&hkl[i]);
    refs.QuickSorter.SortSF(refs, sortHKl);
    for( int i=0; i < refs.Count(); i++ )  {
      const TReflection& r = *refs[i];
      if( (hkl[i].GetH()|hkl[i].GetK()||hkl[i].GetL()) == 0 )  continue;
      if( i == 0 )  {
        new_refs.Add( 
          new TReflection(r.GetH(), r.GetK(), r.GetL()-1, 0, 0, -3) ).SetTag(1);
      }
      else  {
          new_refs.Add( 
            new TReflection(r.GetH(), r.GetK(), r.GetL()-1, r.GetI(), r.GetS(), -3) ).SetTag(1);
      }
      if( (i+1) < refs.Count() )  {
          new_refs.Add( 
            new TReflection(r.GetH(), r.GetK(), r.GetL()+1, r.GetI(), r.GetS(), -2) ).SetTag(1);
      }
      else  {
          new_refs.Add( 
            new TReflection(r.GetH(), r.GetK(), r.GetL()+1, 0, 0, -2) ).SetTag(1);
      }
      new_refs.Add( 
        new TReflection(r.GetH(), r.GetK(), r.GetL(), r.GetI(), r.GetS(), 1) ).SetTag(1);
    }
    hkl.SaveToFile(argv[2], new_refs);
    TBasicApp::GetLog() << "new file has bee created\n";
  }
  catch( TExceptionBase& exc )  {
    printf("An exception occured: %s\n", EsdlObjectName(exc).c_str() );
    printf("details: %s\n", exc.GetException()->GetFullMessage().c_str() );
  }
  printf("\n...");
  std::cin.get();
  return 0;
}