#include "testsuit.h"
#include "bapp.h"
#include "log.h"
#include "md5.h"
#include "sha.h"
#include "olxth.h"
#include "ellist.h"
#include "efile.h"

//...................................................................................................
void IsNumberTest(OlxTests& t)  {
  t.description = __FUNC__;
  olxstr valid_num_str[] = { "0", " 0 ", " 0", "0 ", " 0", " 0", " .0 ", " 0.0 ", " 0.e0 ", 
    "  0.e-1  ", "  0xffa  ", "  0xffa", " 0", " -0. ", " +0. ", "+0e-5", "-.e-5", "07777"  };
  olxstr invalid_num_str[] = { EmptyString, "  0xffx", " 0a", " -.", "0e-a", "08" };
  olxstr valid_int_str[] = {"  0xff ", "0xa", "0777", "-3", " +3 " };
  olxstr invalid_int_str[] = {"  0xffx", " 0xa.", " 0778", "-3.", " +3. " };
  olxstr valid_uint_str[] = {"  0xff", " 0xa", " 0777", "3", " +3 " };
  olxstr invalid_uint_str[] = {"-3", " -0" };
  for( size_t i=0; i < sizeof(valid_num_str)/sizeof(valid_num_str[0]); i++ )  {
    if( !valid_num_str[i].IsNumber() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("valid number is not recognised: '") << valid_num_str[i] << '\'');
  }
  for( size_t i=0; i < sizeof(invalid_num_str)/sizeof(invalid_num_str[0]); i++ )  {
    if( invalid_num_str[i].IsNumber() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid number is not recognised: '") << invalid_num_str[i] << '\'');
  }
  for( size_t i=0; i < sizeof(valid_int_str)/sizeof(valid_int_str[0]); i++ )  {
    if( !valid_int_str[i].IsInt() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("valid number is not recognised: '") << valid_int_str[i] << '\'');
  }
  for( size_t i=0; i < sizeof(invalid_int_str)/sizeof(invalid_int_str[0]); i++ )  {
    if( invalid_int_str[i].IsInt() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid number is not recognised: '") << invalid_int_str[i] << '\'');
  }
  for( size_t i=0; i < sizeof(valid_uint_str)/sizeof(valid_uint_str[0]); i++ )  {
    if( !valid_uint_str[i].IsUInt() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("valid number is not recognised: '") << valid_uint_str[i] << '\'');
  }
  for( size_t i=0; i < sizeof(invalid_uint_str)/sizeof(invalid_uint_str[0]); i++ )  {
    if( invalid_int_str[i].IsUInt() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid number is not recognised: '") << invalid_uint_str[i] << '\'');
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
void MD5Test(OlxTests& t)  {
  t.description = __FUNC__;
  olxcstr msg("The quick brown fox jumps over the lazy dog"),
    res("9e107d9d372bb6826bd81d3542a419d6"),
    res1("e4d909c290d0fb1ca068ffaddf22cbd0"),
    res3("d41d8cd98f00b204e9800998ecf8427e");

  if( !MD5::Digest(CEmptyString).Equalsi(res3) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !MD5::Digest(msg).Equalsi(res) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !MD5::Digest(msg << '.').Equalsi(res1) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
}
//...................................................................................................
void SHA1Test(OlxTests& t)  {
  t.description = __FUNC__;
  olxcstr msg("The quick brown fox jumps over the lazy dog"),
    res("2fd4e1c6 7a2d28fc ed849ee1 bb76e739 1b93eb12"),
    res1("da39a3ee 5e6b4b0d 3255bfef 95601890 afd80709");

  if( !SHA1::Digest(CEmptyString).Equalsi(res1) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !SHA1::Digest(msg).Equalsi(res) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
}
//...................................................................................................
void RelativePathTest(OlxTests& t)  {
  t.description = __FUNC__;
  olxstr base1="c:\\windows\\test",
         base2="c:\\windows\\drivers\\test",
         base3="/tmp/test";
  olxstr path1="c:\\windows\\sys32",
         path2="c:\\test",
         path3="/tmp/test/test/tmp";
  olxstr rel1=TEFile::RelativePathTo(base1,path1),
         rel2=TEFile::RelativePathTo(base1,path2),
         rel3=TEFile::RelativePathTo(base2,path1),
         rel4=TEFile::RelativePathTo(base2,path2),
         rel5=TEFile::RelativePathTo(base3,path3);
  if( TEFile::WinPath(rel1) != "..\\sys32" )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");
  if( TEFile::WinPath(rel2) != "..\\..\\test" )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");
  if( TEFile::WinPath(rel3) != "..\\..\\sys32" )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");
  if( TEFile::WinPath(rel4) != "..\\..\\..\\test" )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");
  if( TEFile::UnixPath(rel5) != "./test/tmp" )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");
  if( TEFile::WinPath(TEFile::RelativePathTo("d:\\test",path2)) != path2 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");

  if( TEFile::WinPath(TEFile::AbsolutePathTo(base1,rel1)) != path1 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to expand relative path");
  if( TEFile::WinPath(TEFile::AbsolutePathTo(base1,rel2)) != path2 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to expand relative path");
  if( TEFile::WinPath(TEFile::AbsolutePathTo(base2,rel3)) != path1 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to expand relative path");
  if( TEFile::WinPath(TEFile::AbsolutePathTo(base2,rel4)) != path2 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to expand relative path");
  if( TEFile::UnixPath(TEFile::AbsolutePathTo(base3,rel5)) != path3 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to expand relative path");
}
//...................................................................................................
void SHA2Test(OlxTests& t)  {
  t.description = __FUNC__;
  olxcstr msg("The quick brown fox jumps over the lazy dog"),
    res256_0("d7a8fbb3 07d78094 69ca9abc b0082e4f 8d5651e4 6d3cdb76 2d02d0bf 37c9e592"),
    res256_1("e3b0c442 98fc1c14 9afbf4c8 996fb924 27ae41e4 649b934c a495991b 7852b855"),
    res224_0("730e109b d7a8a32b 1cb9d9a0 9aa2325d 2430587d dbc0c38b ad911525"),
    res224_1("d14a028c 2a3a2bc9 476102bb 288234c4 15a2b01f 828ea62a c5b3e42f");

  if( !SHA256::Digest(CEmptyString).Equalsi(res256_1) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !SHA256::Digest(msg).Equalsi(res256_0) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !SHA224::Digest(CEmptyString).Equalsi(res224_1) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  if( !SHA224::Digest(msg).Equalsi(res224_0) )
    throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
}
//...................................................................................................
void DirectionalListTest(OlxTests& t)  {
  TUDTypeList<int> test;
  t.description = __FUNC__;
  for( int i=0; i < 10; i++ )
    test.Add(i);
  for( int i=0; i < 10; i++ )  {
    if( test[i] != i )
      throw TFunctionFailedException(__OlxSourceInfo, "Indexing is broken");
  }
  if( (test[5] = 7) != 7 )
    throw TFunctionFailedException(__OlxSourceInfo, "Assignment is broken");

  TUDTypeList<int*, NewCleanup<int> > test1;
  for( int i=0; i < 10; i++ )
    test1.Add(new int(i));
  for( int i=0; i < 10; i++ )  {
    if( *test1[i] != i )
      throw TFunctionFailedException(__OlxSourceInfo, "Indexing is broken");
  }
  if( (*test1[5] = 7) != 7 )
    throw TFunctionFailedException(__OlxSourceInfo, "Assignment is broken");
}
//...................................................................................................
class CriticalSectionTest {
  int i, j, k, l;
  bool use_cs;
  olx_critical_section cs;
  class TestTh : public AOlxThread  {
    CriticalSectionTest& inst;
  public:
    TestTh(CriticalSectionTest& _inst) : inst(_inst)  {  Detached = false;  }
    virtual int Run()  {
      for( int _i=0; _i < 100000; _i++ )  {
        if( inst.use_cs == true )
          inst.cs.enter();
        inst.i++; inst.j++; inst.k++; inst.l++;
        if( inst.use_cs == true )
          inst.cs.leave();
      }
      return 0;
    }
  };
public:
  CriticalSectionTest(bool _use_cs) : use_cs(_use_cs), i(0), j(0), k(0), l(0) {}
  void DoTest(OlxTests& t)  {
    t.description << __FUNC__ << " using CS: " << use_cs;
    TestTh* ths[10];
    for( int _i=0; _i < 10; _i++ )  {
      ths[_i] = new TestTh(*this);
      ths[_i]->Start();
    }
    for( int _i=0; _i < 10; _i++ )  {
      ths[_i]->Join();
      delete ths[_i];
    }
    if( i != 1000000 || j != 1000000 || k != 1000000 || l != 1000000 )  {
      if( use_cs )
        throw TFunctionFailedException(__OlxSourceInfo, "critical section test has failed");
    }
    else if( !use_cs )  
      throw TFunctionFailedException(__OlxSourceInfo, "critical section test is ambiguous (it is possible)");
  }
};
//...................................................................................................
//...................................................................................................
//...................................................................................................
OlxTests::OlxTests() {
  Add(&IsNumberTest);
  Add(&ListTests< TArrayList<int> >);
  Add(&ListTests< TTypeList<int> >);
  Add(&MD5Test);
  Add(&SHA1Test);
  Add(&SHA2Test);
  Add(new CriticalSectionTest(true), &CriticalSectionTest::DoTest);  // the instance gets deleted
  Add(new CriticalSectionTest(false), &CriticalSectionTest::DoTest);  // the instance gets deleted
  Add(&DirectionalListTest);
  Add(&RelativePathTest);
}
//...................................................................................................
void OlxTests::run()  {
  int failed_cnt = 0;
  for( size_t i=0; i < tests.Count(); i++ )  {
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
