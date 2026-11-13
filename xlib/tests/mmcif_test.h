/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_mmcif_test_H
#define __olx_xlib_mmcif_test_H
#include "cif.h"

namespace test {

/* A minimal file in the macromolecular dictionary, written out here rather
than read from disk so the test needs nothing but itself. It carries one of
each thing that is decided in code: two chains, an alternate location, an
insertion code, a residue whose sequence number is not a number, an atom of a
second model, and an anisotropic displacement keyed by atom id rather than by
row order.
*/
static const char *mmcif_fixture =
"data_TEST\n"
"_cell.entry_id           TEST\n"
"_cell.length_a           10.000\n"
"_cell.length_b           20.000\n"
"_cell.length_c           40.000\n"
"_cell.angle_alpha        90.00\n"
"_cell.angle_beta         90.00\n"
"_cell.angle_gamma        90.00\n"
"_cell.Z_PDB              4\n"
"_symmetry.space_group_name_H-M   'P 1 21 1'\n"
"loop_\n"
"_atom_site.group_PDB\n"
"_atom_site.id\n"
"_atom_site.type_symbol\n"
"_atom_site.label_atom_id\n"
"_atom_site.label_alt_id\n"
"_atom_site.label_comp_id\n"
"_atom_site.label_asym_id\n"
"_atom_site.label_seq_id\n"
"_atom_site.pdbx_PDB_ins_code\n"
"_atom_site.Cartn_x\n"
"_atom_site.Cartn_y\n"
"_atom_site.Cartn_z\n"
"_atom_site.occupancy\n"
"_atom_site.B_iso_or_equiv\n"
"_atom_site.auth_seq_id\n"
"_atom_site.auth_comp_id\n"
"_atom_site.auth_asym_id\n"
"_atom_site.pdbx_PDB_model_num\n"
"ATOM   1 N N   . THR A 1 ? 1.000 2.000  4.000 1.00 15.79 1  THR A 1\n"
"ATOM   2 C CA  . THR A 1 ? 2.000 4.000  8.000 1.00 15.79 1  THR A 1\n"
"ATOM   3 C CA  A SER A 2 ? 3.000 6.000 12.000 0.60 20.00 2  SER A 1\n"
"ATOM   4 C CA  B SER A 2 ? 3.100 6.100 12.100 0.40 20.00 2  SER A 1\n"
"ATOM   5 C CA  . GLY B 1 ? 4.000 8.000 16.000 1.00 10.00 1  GLY B 1\n"
"ATOM   6 C CA  . ALA B 2 A 5.000 9.000 18.000 1.00 10.00 2  ALA B 1\n"
"HETATM 7 O O   . HOH C . ? 6.000 1.000  2.000 1.00 30.00 .  HOH C 1\n"
"ATOM   8 C CA  . THR A 1 ? 9.000 9.000  9.000 1.00 15.79 1  THR A 2\n"
"loop_\n"
"_atom_site_anisotrop.id\n"
"_atom_site_anisotrop.U[1][1]\n"
"_atom_site_anisotrop.U[2][2]\n"
"_atom_site_anisotrop.U[3][3]\n"
"_atom_site_anisotrop.U[1][2]\n"
"_atom_site_anisotrop.U[1][3]\n"
"_atom_site_anisotrop.U[2][3]\n"
"2 0.2000 0.2000 0.2000 0.0000 0.0000 0.0000\n"
;

void MMCifDictionaryTest(OlxTests &) {
  TCif cif;
  cif.LoadFromStrings(TStrList(olxstr(mmcif_fixture), '\n'));
  if (!cif.IsMMCif()) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "the macromolecular dictionary was not recognised");
  }
  // and the core dictionary must not be mistaken for it
  TCif core;
  core.LoadFromStrings(TStrList(olxstr(
    "data_CORE\n"
    "_cell_length_a 10\n"
    "_cell_length_b 10\n"
    "_cell_length_c 10\n"
    "loop_\n_atom_site_label\n_atom_site_fract_x\nC1 0.1\n"), '\n'));
  if (core.IsMMCif()) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "a core CIF was taken for a macromolecular one");
  }
}
//.............................................................................
void MMCifLoadTest(OlxTests &) {
  TCif cif;
  cif.LoadFromStrings(TStrList(olxstr(mmcif_fixture), '\n'));
  /* LoadFromStrings only parses; the asymmetric unit is built on block choice.
  InvalidIndex, not 0, because that is what TBasicCFile::LoadFromFile passes
  for a file named without a block - which is every file opened normally, and
  so the only path worth testing.
  */
  cif.SetCurrentBlock(InvalidIndex);
  TAsymmUnit &au = cif.GetAsymmUnit();

  if (olx_abs(au.GetAxes()[0] - 10) > 1e-6 ||
    olx_abs(au.GetAxes()[2] - 40) > 1e-6 ||
    olx_abs(au.GetAngles()[1] - 90) > 1e-6)
  {
    throw TFunctionFailedException(__OlxSourceInfo, "cell");
  }
  /* Seven of the eight rows: the eighth is model 2 and must not be loaded, or
  an NMR ensemble arrives as a pile of superimposed copies.
  */
  if (au.AtomCount() != 7) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("expected 7 atoms, got ") << au.AtomCount());
  }
  /* Coordinates are orthogonal angstroems in this dictionary, so the loader
  has to fractionalise them; with this cell that is a division by the axes.
  */
  const vec3d &c = au.GetAtom(0).ccrd();
  if (olx_abs(c[0] - 0.1) > 1e-6 || olx_abs(c[1] - 0.1) > 1e-6 ||
    olx_abs(c[2] - 0.1) > 1e-6)
  {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("cartesian coordinates were not fractionalised: ") << c.ToString());
  }
  // B, not U
  if (olx_abs(au.GetAtom(0).GetUiso() - 15.79/(8*M_PI*M_PI)) > 1e-6) {
    throw TFunctionFailedException(__OlxSourceInfo, "B was not converted to U");
  }
  // an alternate location is a part
  if (au.GetAtom(2).GetPart() != 1 || au.GetAtom(3).GetPart() != 2) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "label_alt_id did not become a part");
  }
  if (au.GetAtom(0).GetPart() != 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "an atom with no alternate location was given a part");
  }
  /* Residues: THR and SER in chain A, GLY and ALA in chain B, and the water
  has no sequence number so it belongs to none. The two chains both number
  from 1, so a lookup that ignored the chain would merge them.
  */
  // ResidueCount() counts MainResidue at index 0, so four named residues is 5
  if (au.ResidueCount() != 5) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("expected 4 named residues, got ") << (au.ResidueCount() - 1));
  }
  if (au.GetAtom(6).GetResiId() != 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "a residue was invented for an atom that names none");
  }
  // the two alternate locations are one residue, not two
  if (au.GetAtom(2).GetResiId() != au.GetAtom(3).GetResiId()) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "alternate locations were split across residues");
  }
  // chain A residue 1 and chain B residue 1 are different residues
  if (au.GetAtom(0).GetResiId() == au.GetAtom(4).GetResiId()) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "residues of different chains were merged");
  }
  /* The insertion code makes ALA a residue of its own rather than a second
  residue 2 of chain B, which the registry could not hold.
  */
  if (au.GetAtom(5).GetResiId() == 0 ||
    au.GetAtom(5).GetResiId() == au.GetAtom(4).GetResiId())
  {
    throw TFunctionFailedException(__OlxSourceInfo,
      "an insertion code did not produce its own residue");
  }
  // the anisotropic row names atom id 2, which is the second atom
  if (au.GetAtom(1).GetEllipsoid() == 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "_atom_site_anisotrop was not applied");
  }
  if (au.GetAtom(0).GetEllipsoid() != 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "an ellipsoid reached an atom the loop does not name");
  }
}
//.............................................................................
/* Real entries, if any have been put where OLEX2_MMCIF_TEST_DIR points. A
synthetic fixture only ever tests the author's reading of the dictionary twice;
what the PDB actually writes is the thing worth checking. Skipped silently when
the variable is unset, so this stays a test rather than a local script.
*/
void MMCifRealFileTest(OlxTests &) {
  olxstr dir = olx_getenv("OLEX2_MMCIF_TEST_DIR");
  if (dir.IsEmpty() || !TEFile::Exists(dir)) {
    return;
  }
  TStrList files = TEFile::ListDir(dir, "*.cif", sefFile);
  for (size_t i = 0; i < files.Count(); i++) {
    const olxstr fn = TEFile::AddPathDelimeter(dir) << files[i];
    TCif cif;
    // through LoadFromFile, which chooses the block itself, as opening does
    cif.LoadFromFile(fn);
    if (!cif.IsMMCif()) {
      continue;      // a small-molecule CIF in the same folder is not an error
    }
    TAsymmUnit &au = cif.GetAsymmUnit();
    if (au.AtomCount() == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("no atoms read from ") << files[i]);
    }
    if (au.GetAxes()[0] <= 0 || au.GetAngles()[0] <= 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("no cell read from ") << files[i]);
    }
    /* Fractional coordinates of a deposited entry lie within a couple of cells
    of the origin. Anything beyond that means the orthogonal coordinates were
    taken as fractional, which is the mistake this dictionary invites.
    */
    for (size_t j = 0; j < au.AtomCount(); j++) {
      const vec3d &c = au.GetAtom(j).ccrd();
      for (int k = 0; k < 3; k++) {
        if (olx_abs(c[k]) > 5) {
          throw TFunctionFailedException(__OlxSourceInfo,
            olxstr("implausible fractional coordinate in ") << files[i] <<
            ": " << c.ToString());
        }
      }
    }
    TBasicApp::NewLogEntry(logInfo) << "mmCIF " << files[i] << ": " <<
      au.AtomCount() << " atoms, " << (au.ResidueCount() - 1) << " residues";
  }
}
//.............................................................................
void MMCifTests(OlxTests &t) {
  t.Add(test::MMCifDictionaryTest);
  t.Add(test::MMCifLoadTest);
  t.Add(test::MMCifRealFileTest);
}

};  //namespace test
#endif
