#include <stdio.h>
#include <iostream>

#include "testsuit.h"
#include "xapp.h"
#include "outstream.h"
#include "edict.h"
#include "olxth.h"
#include "ellist.h"
#include "symmat.h"
//..............................
#include "tests/sort_test.h"
#include "tests/encoding_test.h"
#include "tests/string_test.h"
#include "tests/file_test.h"
#include "tests/sys_test.h"
#include "tests/spline_test.h"
#include "tests/matrix_test.h"
#include "tests/mat_id_test.h"
#include "tests/symmparser_test.h"
#include "tests/vcov_test.h"

int main(int argc, char* argv[]) {
	TXApp xapp(argv[0]);
  xapp.GetLog().AddStream(new TOutStream, true);
  OlxTests tests;
  tests.Add(&test::IsNumberTest);
  tests.Add(&test::ListTests<TArrayList<int> >).
    Add(&test::ListTests<TTypeList<int> >).
    Add(&test::DirectionalListTest);
  tests.Add(&test::MD5Test).
    Add(&test::SHA1Test).
    Add(&test::SHA2Test);
  tests.Add(new test::CriticalSectionTest(true), &test::CriticalSectionTest::DoTest).
    Add(new test::CriticalSectionTest(false), &test::CriticalSectionTest::DoTest);
  tests.Add(&test::RelativePathTest);
  tests.Add(new test::SortTest, &test::SortTest::DoTest);
  tests.Add(&test::TestSVD).
    Add(&test::TestInvert).
    Add(&test::TestLU).
    Add(&test::TestQR);
  tests.Add(&test::spline_test);
  tests.Add(&full_smatd_id<>::Tests).Add(&test::rotation_id_test);
  tests.Add(&test::symm_parser_tests);
  tests.Add(&smatd::Tests);
  tests.Add(&test::vcov_test);
  tests.run();
  std::cin.get();
  return 0;
}

