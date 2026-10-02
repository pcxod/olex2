/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#pragma once

#include "structure_generator.h"
#include "bonds_eq_search.h"

namespace test {
  // using some of the occtbx borrowed tests...
  void bond_eq_test(OlxTests& t) {
    t.description = __OlxSrcInfo;
    TSpaceGroup* sg = TSymmLib::GetInstance().FindGroupByName("C2/c");
    TStopWatch sw(__OlxSrcInfo);
    sw.start("Generating structure");
    olx_object_ptr<RandomStructure::structure> str =
      RandomStructure::create_structure(vec3d(40, 62, 50), vec3d(90, 102, 90), *sg);
    RandomStructure rs(0);
    rs.generate_atoms(str->latt, 1500);
    RandomStructure::structure str1, str2;
    str1.RefMod.Assign(str->RefMod, true);
    str2.RefMod.Assign(str->RefMod, true);
    sw.stop();
    BondsSymmEqTaskFactory::default_type() = BondsSymmEqTaskDirect;
    sw.start("Direct");
    str->latt.Init();
    BondsSymmEqTaskFactory::default_type() = BondsSymmEqTaskCellList;
    sw.start("Cell list");
    str1.latt.Init();
    sw.stop();
    RandomStructure::compare_conn(
      str->latt.GetAsymmUnit(), str1.latt.GetAsymmUnit(), "cell list");
    sw.start("Shells");
    BondsSymmEqTaskFactory::default_type() = BondsSymmEqTaskShells;
    str2.latt.Init();
    sw.stop();
    RandomStructure::compare_conn(
      str->latt.GetAsymmUnit(), str2.latt.GetAsymmUnit(), "shells");
    
  }
}
