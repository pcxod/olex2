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
#include "ebtree.h"
#include "estopwatch.h"
#include <set>

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
    size_t max_str_c = 500000;
    bool test_binary = max_str_c < 1000000; // takes too long with > 1m recs
    TStrList strings(max_str_c);
    for (size_t i = 0; i < max_str_c; i++) {
      strings[i] = rnd_str_85(256);
    }

    olxstr_set<> binary_set(olx_reserve(test_binary ? max_str_c : 1));
    typedef TEHashSet<olxstr, olxstrComparator<false>, 4, 4> set_t;
    typedef TEHashTreeSet<olxstr, olxstrComparator<false>, 4, 4> hbt_t;

    typedef AVLTreeEntry<TreeSetEntry<olxstr> > avlt_entry_t;
    typedef AVLTree<avlt_entry_t, olxstrComparator<false> > bt_t;

    typedef RBTreeEntry<TreeSetEntry<olxstr> > rb_entry_t;
    typedef RBTree<rb_entry_t, olxstrComparator<false> > rbt_t;

    std::set<olxstr> std_set;
    bt_t bt;
    rbt_t rbt;
    set_t hash_set;
    hbt_t hbt;
    olxstr ns = "do you have me??";
    TStopWatch sw(__FUNC__);
    sw.start("Binary set building");
    if (test_binary) {
      for (size_t i = 0; i < max_str_c; i++) {
        binary_set.Add(strings[i]);
      }
    }
    sw.start("std::set building");
    for (size_t i = 0; i < max_str_c; i++) {
      std_set.insert(strings[i]);
    }
    sw.start("Hash set building");
    for (size_t i = 0; i < max_str_c; i++) {
      hash_set.Add(strings[i]);
    }
    sw.start("AVL binary tree building");
    for (size_t i = 0; i < max_str_c; i++) {
      bt.Add(bt_t::value_t(strings[i]));
    }
    sw.start("RB binary tree building");
    for (size_t i = 0; i < max_str_c; i++) {
      rbt.Add(bt_t::value_t(strings[i]));
    }

    sw.start("Hash binary tree building");
    for (size_t i = 0; i < max_str_c; i++) {
      hbt.Add(strings[i]);
    }

    sw.start("Binary set contains");
    if (test_binary) {
      for (size_t i = 0; i < max_str_c; i++) {
        if (!binary_set.Contains(strings[i])) {
          throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
        }
      }
      if (binary_set.Contains(ns)) {
        throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
      }
    }

    sw.start("std::set contains");
    for (size_t i = 0; i < max_str_c; i++) {
      if (std_set.find(strings[i]) == std_set.end()) {
        throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
      }
    }
    if (std_set.find(ns) != std_set.end()) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
    }

    sw.start("Hash set contains");
    for (size_t i = 0; i < max_str_c; i++) {
      if (!hash_set.Contains(strings[i])) {
        throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
      }
    }
    if (hash_set.Contains(ns)) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
    }

    sw.start("AVL binary tree contains");
    for (size_t i = 0; i < max_str_c; i++) {
      if (!bt.Contains(strings[i])) {
        throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
      }
    }
    if (bt.Contains(ns)) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
    }

    sw.start("RB binary tree contains");
    for (size_t i = 0; i < max_str_c; i++) {
      if (!rbt.Contains(strings[i])) {
        throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
      }
    }
    if (rbt.Contains(ns)) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
    }

    sw.start("Hash binary tree contains");
    for (size_t i = 0; i < max_str_c; i++) {
      if (!hbt.Contains(strings[i])) {
        throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
      }
    }
    if (hbt.Contains(ns)) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected");
    }
    sw.stop();

    olx_pdict<size_t,size_t> stats = hash_set.get_stats();
    TArrayList<olx_pair_t<size_t, size_t> > rv_stats(stats.Count());
    for (size_t i = 0; i < stats.Count(); i++) {
      //TBasicApp::NewLogEntry() << stats.GetKey(i) << " -> " << stats.GetValue(i);
      rv_stats[i].a = stats.GetValue(i);
      rv_stats[i].b = stats.GetKey(i);
    }
    QuickSorter::Sort(rv_stats,
      ReverseComparator::Make(
      ComplexComparator::Make(
        FunctionAccessor::MakeConst(&olx_pair_t<size_t, size_t>::GetA),
        TPrimitiveComparator())));

    for (size_t i = 0; i < olx_min(10, rv_stats.Count()); i++) {
      TBasicApp::NewLogEntry() << rv_stats[i].a << " -> " << rv_stats[i].b;
    }
  }
  //...........................................................................................
  void HashedTests(OlxTests& t) {
    t.Add(&basic_set_test)
      .Add(&basic_dict_test)
      .Add(&perf_test);
  }
};  //namespace test

