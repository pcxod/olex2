/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "ehashed.h"
#include "encodings.h"
#include "etime.h"

namespace test {

  void basic_set_test(OlxTests& t) {
    typedef TEHashSet<olxstr, olxstrComparator<false> > set_t;
    set_t test;
    olxstr test_set[4] = {
    "1",
    "2",
    "hello",
    "test"};
    for (size_t i = 0; i < 4; i++) {
      test.Add(test_set[i]);
    }
    for (size_t i = 0; i < 4; i++) {
      if (!test.Contains(test_set[i])) {
        throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
      }
    }
    if (!test.Remove(test_set[2])) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
    }
    set_t::iterator_t itr = test.Iterate();
    set_t::basket_t* b;
    size_t cnt = 0;
    while ((b = itr.Next()) != 0) {
      cnt += b->Count();
    }
    if (cnt != 3) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
    }

  }
  //...........................................................................................
  void basic_dict_test(OlxTests& t) {
    TEHashMap<olxstr, olxstr, olxstrComparator<true> > test;
    olxstr test_set[4] = {
    "1",
    "2",
    "hello",
    "test" };
    for (size_t i = 0; i < 4; i++) {
      test.Add(test_set[i], test_set[i]);
    }
    for (size_t i = 0; i < 4; i++) {
      if (!test.Contains(test_set[i])) {
        throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
      }
    }
  }
  //...........................................................................................
  olxstr rnd_str_85(size_t max_l=128) {
    const char* b85 = encoding::data::base85();
    size_t l = (size_t)rand() % max_l;
    olxstr rv(olx_reserve(l));
    for (size_t i = 0; i < l; i++) {
      rv << b85[rand() % 85];
    }
    return rv;
  }
  void perf_test(OlxTests& t) {
    size_t max_str_c = 450000;
    bool test_binary = max_str_c < 1000000; // takes too long with > 1m recs
    TStrList strings(max_str_c);
    for (size_t i = 0; i < max_str_c; i++) {
      strings[i] = rnd_str_85();
    }

    olxstr_set<> binary_set(olx_reserve(test_binary ? max_str_c : 1));
    typedef TEHashSet<olxstr, olxstrComparator<false>, 64, 2> set_t;
    set_t has_set;

    size_t bin_st = TETime::msNow();
    if (test_binary) {
      for (size_t i = 0; i < max_str_c; i++) {
        binary_set.Add(strings[i]);
      }
    }
    size_t hash_st = TETime::msNow();
    for (size_t i = 0; i < max_str_c; i++) {
      has_set.Add(strings[i]);
    }
    size_t hash_end = TETime::msNow();
    // hash set is about 20 times faster
    TBasicApp::NewLogEntry() << "Binary set building time (ms): " <<
      hash_st - bin_st;
    TBasicApp::NewLogEntry() << "Hash set building time (ms): " <<
      hash_end - hash_st;

    bin_st = TETime::msNow();
    if (test_binary) {
      for (size_t i = 0; i < max_str_c; i++) {
        if (!binary_set.Contains(strings[i])) {
          throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
        }
      }
    }
    hash_st = TETime::msNow();
    for (size_t i = 0; i < max_str_c; i++) {
      if (!has_set.Contains(strings[i])) {
        throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
      }
    }
    hash_end = TETime::msNow();
    // hash set is about 3 times faster for 64/2, 32/3
    TBasicApp::NewLogEntry() << "Binary set contains time (ms): " <<
      hash_st - bin_st;
    TBasicApp::NewLogEntry() << "Hash set contains time (ms): " <<
      hash_end - hash_st;
  }
  //...........................................................................................
  void HashedTests(OlxTests& t) {
    t.Add(&basic_set_test)
      .Add(&basic_dict_test)
      .Add(&perf_test);
  }
};  //namespace test

