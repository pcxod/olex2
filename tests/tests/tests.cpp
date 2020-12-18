/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include <stdio.h>
#include <iostream>

#include "testsuit.h"
#include "xapp.h"
#include "outstream.h"
#include "edict.h"
#include "olxth.h"
#include "symmat.h"
#include "filetree.h"
#include "cif.h"
//..............................
#include "tests/alg_test.h"
#include "tests/container_test.h"
#include "tests/sort_test.h"
#include "tests/encoding_test.h"
#include "tests/roman_test.h"
#include "tests/string_test.h"
#include "tests/file_test.h"
#include "tests/sys_test.h"
#include "tests/spline_test.h"
#include "tests/matrix_test.h"
#include "tests/tensor_test.h"
#include "tests/mat_id_test.h"
#include "tests/symmparser_test.h"
#include "tests/vcov_test.h"
#include "tests/irange_test.h"
#include "tests/reflection_test.h"
#include "tests/evalue_test.h"
#include "tests/cell_reduction_test.h"
#include "tests/hall_test.h"
#include "tests/formula_test.h"
#include "tests/smat_test.h"
#include "tests/exparse_test.h"
#include "../repository/tests/shellutil_test.h"
#include "tests/olx_func_test.h"
#include "tests/ptr_test.h"
#include "tests/binding_test.h"

class Listener : public AActionHandler {
public:
  virtual bool Execute(const IOlxObject *Sender, const IOlxObject *Data) {
    if (Data != 0 && Data->Is<TOnProgress>()) {
      TBasicApp::GetLog() << '\r' << ((TOnProgress*)Data)->GetAction() << "     ";
      return true;
    }
    return false;
  }
  virtual bool OnExit(const IOlxObject *Sender, const IOlxObject *Data) {
    TBasicApp::NewLogEntry();
    return true;
  }
};

int main(int argc, char* argv[]) {
  TXApp xapp(TXApp::GuessBaseDir(argv[0]));
  xapp.XFile().RegisterFileFormat(new TIns, "ins");
  xapp.GetLog().AddStream(new TOutStream, true);
  xapp.GetLog().AddStream(TUtf8File::Create(xapp.GetBaseDir()+"log.out", false), true);
  OlxTests tests;
  tests.Add(&test::exparse::ExparseTests);
  tests.Add(&test::StringTests);
  tests.Add(&test::ContainerTests);
  tests.Add(&test::HashingTests).
    Add(&test::EncodingTests);
  tests.Add(new test::CriticalSectionTest(true), &test::CriticalSectionTest::DoTest).
    Add(new test::CriticalSectionTest(false), &test::CriticalSectionTest::DoTest);
  tests.Add(&test::FileTests);
  tests.Add(new test::SortTest, &test::SortTest::DoTest);
  tests.Add(&test::TestSVD).
    Add(&test::TestInvert).
    Add(&test::TestLU).
    Add(&test::TestQR).
    Add(&test::TestCholesky).
    Add(&test::TestEigenDecomposition).
    Add(&test::TestMatrixDiff);
  tests.Add(&test::spline_test);
  tests.Add(&test::MatIdTests);
  tests.Add(&test::symm_parser_tests);
  tests.Add(&test::SymmMatTests);
  tests.Add(&test::vcov_test);
  tests.Add(&test::IndexRangeTest);
  tests.Add(&test::reflection_tests);
  tests.Add(&test::EValueTests);
  tests.Add(&test::cell_reduction_test);
  tests.Add(&test::AlgTests);
  tests.Add(&test::HallTests);
  tests.Add(&test::FormulaTests);
  tests.Add(&test::ShellUtilTests);
  tests.Add(&test::TestRoman);
  tests.Add(&test::FuncTests);
  tests.Add(&test::PtrTests);
  tests.Add(&test::BindingTests);
  tests.Add(&test::TensorTransform);

  tests.run();
  if( argc > 1 )  {
    olxstr data_dir = argv[1];
    if( !TEFile::Exists(data_dir) || !TEFile::IsDir(data_dir) )  {
      xapp.NewLogEntry() << "Skipping IO tests - no valid folder is given";
      return 0;
    }
    TFileTree ft(data_dir);
    Listener lsnr;
    ft.OnExpand.Add(&lsnr);
    ft.Expand();
    ft.OnExpand.Remove(&lsnr);
    TStrList files;
    ft.GetRoot().ListFiles(files, "*.cif");
    for( size_t i=0; i < files.Count(); i++ )  {
      try  {
        TCif cif;
        cif.LoadFromFile(files[i]);
        bool report = false;
        for( size_t ai=0; ai < cif.GetAsymmUnit().AtomCount(); ai++ )  {
          const TCAtom& ca = cif.GetAsymmUnit().GetAtom(ai);
          vec3d crd_esd = ca.ccrdEsd()*10000;
          if( crd_esd[0] >= 20 || crd_esd[1] >= 20 || crd_esd[2] >= 20 ||
            ca.GetUisoEsd()*1000 >= 20 )
          {
            report = true;
            break;
          }
        }
        if( report && false )  {
          xapp.NewLogEntry() << "This file is potentially affected by the esd formatting error: '" <<
            files[i] << '\'';
        }
      }
      catch(const TExceptionBase& e)  {
        xapp.NewLogEntry().nl() << files[i] << ':';
        xapp.NewLogEntry(logExceptionTrace) << e;
      }
    }
    files.Clear();
    ft.GetRoot().ListFiles(files, "*.ins;*.res");
    for( size_t i=0; i < files.Count(); i++ )  {
      try  {
        xapp.XFile().LoadFromFile(files[i]);
      }
      catch(const TExceptionBase& e)  {
        xapp.NewLogEntry().nl() << files[i] << ':';
        xapp.NewLogEntry(logExceptionTrace) << e;
      }
    }
  }
  xapp.NewLogEntry() << "Finished...";
  std::cin.get();
  return 0;
}
