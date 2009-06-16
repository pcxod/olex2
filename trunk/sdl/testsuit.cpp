#include "testsuit.h"
#include "bapp.h"
#include "log.h"

//...................................................................................................
void IsNumberTest(OlxTests& t)  {
  t.description = __FUNC__;
  olxstr valid_str[] = { "0", " 0 ", " 0", "0 ", " 0", " 0", " .0 ", " 0.0 ", " 0.e0 ", 
    "  0.e-1  ", "  0xffa  ", "  0xffa", " 0", " -0. ", " +0. ", "+0e-5", "-.e-5"  };
  olxstr invalid_str[] = { EmptyString, "  0xffx", " 0a", " -.", "0e-a" };
  for( int i=0; i < sizeof(valid_str)/sizeof(valid_str[0]); i++ )  {
    if( !valid_str[i].IsNumber() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("valid number is not recognised: '") << valid_str[i] << '\'');
  }
  for( int i=0; i < sizeof(invalid_str)/sizeof(invalid_str[0]); i++ )  {
    if( invalid_str[i].IsNumber() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid number is not recognised: '") << invalid_str[i] << '\'');
  }
}
//...................................................................................................
void _PrepareList(TTypeList<int>& il)  {
  for( int i=0; i < 10; i++ )
    il.AddCCopy(i);
}
void _PrepareList(TArrayList<int>& il)  {
  for( int i=0; i < 10; i++ )
    il.Add(i);
}
template <class List>
void ListTests(OlxTests& t)  {
  int valid_move []   = {1,2,3,4,5,6,7,8,9,0};
  int valid_shift1 [] = {8,9,0,1,2,3,4,5,6,7};
  int valid_shift2 [] = {7,8,9,0,1,2,3,4,5,6};
  int valid_shift3 [] = {0,1,2,3,4,5,6,7,8,9};
  int valid_shift4 [] = {1,2,3,4,5,6,7,8,9,0};
  t.description = __FUNC__;
  t.description << ' ' << EsdlClassName(List);
  List il;
  _PrepareList(il);
  il.Move(0,9);
  for( int i=0; i < 10; i++ )
    if( il[i] != valid_move[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "Move failed");
  il.ShiftR(3);
  for( int i=0; i < 10; i++ )
    if( il[i] != valid_shift1[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "ShiftR failed");
  il.ShiftR(1);
  for( int i=0; i < 10; i++ )
    if( il[i] != valid_shift2[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "ShiftR failed");
  il.ShiftL(3);
  for( int i=0; i < 10; i++ )
    if( il[i] != valid_shift3[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "ShiftL failed");
  il.ShiftL(1);
  for( int i=0; i < 10; i++ )
    if( il[i] != valid_shift4[i] )
      throw TFunctionFailedException(__OlxSourceInfo, "ShiftL failed");
  il.Move(9,0);
  for( int i=0; i < 10; i++ )
    if( il[i] != i )
      throw TFunctionFailedException(__OlxSourceInfo, "Consistency failed");
}
//...................................................................................................
//...................................................................................................
//...................................................................................................
OlxTests::OlxTests() {
  Add(&IsNumberTest);
  Add(&ListTests< TArrayList<int> >);
  Add(&ListTests< TTypeList<int> >);
}
//...................................................................................................
void OlxTests::run()  {
  int failed_cnt = 0;
  for( int i=0; i < tests.Count(); i++ )  {
    try  { 
      description = EmptyString;
      tests[i].run(*this);  
      TBasicApp::GetLog() << (olxstr("Running test ") << i+1 << '/' << tests.Count() << ": " << description << '\n');
      TBasicApp::GetLog() << "Done\n";
    }
    catch( TExceptionBase& exc )  {
      TBasicApp::GetLog() << (olxstr("Running test ") << i+1 << '/' << tests.Count() << ": " << description << '\n');
      TBasicApp::GetLog().Error( exc.GetException()->GetFullMessage() );
      TBasicApp::GetLog() << "Failed\n";
      failed_cnt++;
    }
  }
  if( failed_cnt != 0 )
    TBasicApp::GetLog() << (olxstr(failed_cnt) << '/' << tests.Count() << " have failed\n");
}
