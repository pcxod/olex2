/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../protein.h"

namespace test {

/* The backbone classifier is exercised directly, on geometry built here, so
these cases document what it must survive rather than what one test file
happens to contain.

The measured behaviour on real structures, for reference: 100% of residues in
crambin (1EJG), ubiquitin and the 8015 residues of GroEL (1AON); 94% in
haemoglobin (4HHB), where the remainder has no N-CA-C=O candidate at all.
*/
namespace protein_test_data {
  using namespace xlib::protein;

  // ideal-ish residue: N-CA-C=O plus a CB, and the next residue's N
  void AddIdealResidue(TTypeList<ProteinAtom> &l, bool with_peptide_n) {
    l.AddNew(7, vec3d(0.000, 0.000, 0.000));           // N
    l.AddNew(6, vec3d(1.458, 0.000, 0.000));           // CA
    l.AddNew(6, vec3d(2.010, 1.420, 0.000));           // C
    l.AddNew(8, vec3d(3.180, 1.700, 0.000));           // O, 1.203 from C
    l.AddNew(6, vec3d(2.000, -0.780, 1.200));          // CB
    if (with_peptide_n) {
      // N of the following residue, 1.33 from C
      l.AddNew(7, vec3d(1.240, 2.510, 0.000), true);
    }
  }
}

//.............................................................................
void ProteinBackboneTest(OlxTests &t) {
  using namespace xlib::protein;
  t.description = __FUNC__;

  // a plain residue must resolve, and to the right atoms
  {
    TTypeList<ProteinAtom> l;
    protein_test_data::AddIdealResidue(l, true);
    BackboneRoles r = FindBackbone(l);
    if (!r.IsValid()) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "backbone not found in an ideal residue");
    }
    if (r.n != 0 || r.ca != 1 || r.c != 2 || r.o != 3) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("wrong backbone roles: n=") << r.n << " ca=" << r.ca
        << " c=" << r.c << " o=" << r.o);
    }
  }

  /* The serine trap. A sidechain hydroxyl at 1.42 A from CB is close enough to
  a carbonyl to fool any fixed distance window wide enough for a 1.7 A
  structure's real backbone, which is how an earlier version misread serine and
  threonine in haemoglobin. The scored classifier must still prefer the true
  backbone.
  */
  {
    TTypeList<ProteinAtom> l;
    protein_test_data::AddIdealResidue(l, true);
    // OG on the CB at index 4, at hydroxyl distance
    l.AddNew(8, vec3d(2.000, -0.780, 2.620));
    BackboneRoles r = FindBackbone(l);
    if (!r.IsValid() || r.ca != 1 || r.c != 2 || r.o != 3) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "a sidechain hydroxyl was mistaken for the backbone carbonyl");
    }
  }

  // the C-terminal residue has no following nitrogen and must still resolve
  {
    TTypeList<ProteinAtom> l;
    protein_test_data::AddIdealResidue(l, false);
    BackboneRoles r = FindBackbone(l);
    if (!r.IsValid() || r.ca != 1) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "a residue without a peptide bond was rejected");
    }
  }

  // water, and an isolated ion, must be rejected rather than traced
  {
    TTypeList<ProteinAtom> l;
    l.AddNew(8, vec3d(0, 0, 0));
    if (FindBackbone(l).IsValid()) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "a water was classified as an amino acid");
    }
    TTypeList<ProteinAtom> l2;
    l2.AddNew(26, vec3d(0, 0, 0));
    l2.AddNew(8, vec3d(2.1, 0, 0));
    if (FindBackbone(l2).IsValid()) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "a metal site was classified as an amino acid");
    }
  }

  /* Labels are never consulted, so a SHELX-labelled model must behave exactly
  like a PDB-labelled one. This is the case that motivated the whole approach:
  crambin's first residue is N3, C6, C7, O3. Geometry is identical here, so an
  identical result is the assertion.
  */
  {
    TTypeList<ProteinAtom> a, b;
    protein_test_data::AddIdealResidue(a, true);
    protein_test_data::AddIdealResidue(b, true);
    BackboneRoles ra = FindBackbone(a), rb = FindBackbone(b);
    if (ra.ca != rb.ca || ra.c != rb.c || ra.o != rb.o || ra.n != rb.n) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "classification depends on something other than geometry");
    }
  }
}
//.............................................................................
void ResidueKindTest(OlxTests &t) {
  using namespace xlib::protein;
  t.description = __FUNC__;

  struct { const char *name; ResiduePolarity p; ResidueCharge c; } cases[] = {
    {"ASP", rpHydrophilic, rcAcidic}, {"GLU", rpHydrophilic, rcAcidic},
    {"ARG", rpHydrophilic, rcBasic}, {"LYS", rpHydrophilic, rcBasic},
    {"HIS", rpHydrophilic, rcBasic},
    {"SER", rpHydrophilic, rcNeutral}, {"GLN", rpHydrophilic, rcNeutral},
    {"TYR", rpHydrophilic, rcNeutral},
    {"ALA", rpLipophilic, rcNeutral}, {"TRP", rpLipophilic, rcNeutral},
    // the three that are borderline, and the calls made for them
    {"GLY", rpHydrophilic, rcNeutral}, {"PRO", rpHydrophilic, rcNeutral},
    {"CYS", rpLipophilic, rcNeutral},
    // selenomethionine reads as methionine
    {"MSE", rpLipophilic, rcNeutral},
    // the protonation variants force-field files use follow their parent
    {"HID", rpHydrophilic, rcBasic}, {"ASH", rpHydrophilic, rcAcidic},
    {"LYN", rpHydrophilic, rcBasic},
    // not amino acids, and not to be guessed at
    {"HOH", rpUnknown, rcUnknown}, {"NAG", rpUnknown, rcUnknown},
    {"ZN", rpUnknown, rcUnknown}, {"", rpUnknown, rcUnknown}
  };
  for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
    if (ClassifyPolarity(cases[i].name) != cases[i].p) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("wrong polarity for '") << cases[i].name << '\'');
    }
    if (ClassifyCharge(cases[i].name) != cases[i].c) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("wrong charge for '") << cases[i].name << '\'');
    }
  }
  /* Class names arrive from a file, so case and padding are not guaranteed.
  This is the one place labels are trusted, and only because a converted SHELX
  model keeps RESI THR even when its atoms are called N3 and C6.
  */
  if (ClassifyPolarity(" thr ") != rpHydrophilic) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "class names are not normalised for case or padding");
  }
  /* The identity axis. Every standard name must round-trip through its index,
  and a variant must land on its parent rather than on a slot of its own, or a
  key of twenty rows would silently be a key of twenty-six.
  */
  if (StandardResidueCount() != 20) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "the standard amino acids are twenty");
  }
  for (size_t i = 0; i < StandardResidueCount(); i++) {
    if (ResidueIndex(StandardResidueName(i)) != i) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("residue index does not round-trip at ") << i);
    }
  }
  if (ResidueIndex("MSE") != ResidueIndex("MET") ||
    ResidueIndex("CYX") != ResidueIndex("CYS") ||
    ResidueIndex("HIP") != ResidueIndex("HIS"))
  {
    throw TFunctionFailedException(__OlxSourceInfo,
      "a residue variant does not fold onto its parent");
  }
  if (ResidueIndex("HOH") != InvalidIndex) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "water is not an amino acid");
  }
}
//.............................................................................
void ChainContinuityTest(OlxTests &t) {
  using namespace xlib::protein;
  t.description = __FUNC__;

  struct MakeResidue {
    static BackboneResidue At(int number, double x) {
      BackboneResidue r;
      r.number = number;
      r.ca = vec3d(x, 0, 0);
      return r;
    }
  };

  // consecutive and at peptide spacing: plainly one chain
  if (!IsContinuous(MakeResidue::At(1, 0), MakeResidue::At(2, 3.8))) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "a normal peptide step was treated as a break");
  }
  /* One residue missing from the numbering, its two neighbours at twice the
  peptide step. This is the case that produced dozens of spurious breaks: it
  must bridge.
  */
  if (!IsContinuous(MakeResidue::At(1, 0), MakeResidue::At(3, 7.6))) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "a single missing residue was treated as a break");
  }
  // two missing, still within reach
  if (!IsContinuous(MakeResidue::At(1, 0), MakeResidue::At(4, 11.0))) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "a two-residue gap at plausible spacing was treated as a break");
  }
  /* The rule must not become "consecutive numbers are enough". Two residues
  numbered one apart but 12 A away from each other are two different pieces,
  and joining them draws a ribbon through empty space.
  */
  if (IsContinuous(MakeResidue::At(1, 0), MakeResidue::At(2, 12.0))) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "two distant residues were joined because their numbers are adjacent");
  }
  // nor may a long unmodelled stretch be spanned, however close the ends land
  if (IsContinuous(MakeResidue::At(1, 0), MakeResidue::At(40, 3.8))) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "a gap of 39 residues was bridged because its ends happen to be close");
  }
  // duplicate numbering, from a dropped insertion code, falls back to distance
  if (!IsContinuous(MakeResidue::At(5, 0), MakeResidue::At(5, 3.8)) ||
    IsContinuous(MakeResidue::At(5, 0), MakeResidue::At(5, 8.0)))
  {
    throw TFunctionFailedException(__OlxSourceInfo,
      "duplicate residue numbers are not falling back to the distance test");
  }
}
//.............................................................................
/* Secondary structure, against geometry built to the textbook numbers rather
than to the classifier's own ideals, so the test can fail.
*/
void SecondaryStructureTest(OlxTests &t) {
  using namespace xlib::protein;
  t.description = __FUNC__;

  // an ideal alpha helix: 2.3 A radius, 1.5 A rise, 100 degrees per residue
  ChainSegment helix;
  for (size_t i = 0; i < 20; i++) {
    BackboneResidue r;
    r.number = (int)(i + 1);
    const double a = 100*M_PI/180*i;
    r.ca = vec3d(2.3*cos(a), 2.3*sin(a), 1.5*i);
    helix.residues.AddCopy(r);
  }
  /* A two-stranded antiparallel sheet. Each strand is a zigzag at 3.3 A rise
  and 0.9 A amplitude, which puts CA-CA at 3.76 A and CA(i) to CA(i+4) at
  13.2 A; the two lie 4.8 A apart, which is the separation that makes them a
  sheet rather than two pieces of extended chain.
  */
  ChainSegment sheet;
  const size_t strand_len = 10;
  for (size_t i = 0; i < strand_len; i++) {
    BackboneResidue r;
    r.number = (int)(i + 1);
    r.ca = vec3d(3.3*i, 0, (i % 2) ? 0.9 : -0.9);
    sheet.residues.AddCopy(r);
  }
  for (size_t i = 0; i < 3; i++) {          // the turn
    BackboneResidue r;
    r.number = (int)(strand_len + i + 1);
    r.ca = vec3d(3.3*(strand_len - 1) + 2.0, 1.2*(i + 1), 0);
    sheet.residues.AddCopy(r);
  }
  for (size_t i = 0; i < strand_len; i++) { // back the other way, 4.8 A across
    BackboneResidue r;
    r.number = (int)(strand_len + 4 + i);
    r.ca = vec3d(3.3*(strand_len - 1 - i), 4.8,
      ((strand_len - 1 - i) % 2) ? 0.9 : -0.9);
    sheet.residues.AddCopy(r);
  }

  /* The helix. Its ends are genuinely ambiguous, fewer windows spanning them,
  so the interior is what is asserted rather than every residue.
  */
  {
    TArrayList<short> ss;
    AssignSecondaryStructure(helix, ss);
    if (ss.Count() != helix.residues.Count()) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "one assignment per residue is expected");
    }
    for (size_t i = 4; i + 4 < ss.Count(); i++) {
      if (ss[i] != ss_helix) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("residue ") << i << " of an ideal helix was not assigned"
          " helix");
      }
    }
    for (size_t i = 0; i < ss.Count(); i++) {
      if (ss[i] == ss_strand) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "an ideal helix contains a strand residue");
      }
    }
  }

  // the sheet: both strands found, the turn between them not called strand
  {
    TArrayList<short> ss;
    AssignSecondaryStructure(sheet, ss);
    size_t n_e = 0;
    for (size_t i = 0; i < ss.Count(); i++) {
      if (ss[i] == ss_strand) {
        n_e++;
      }
      if (ss[i] == ss_helix) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "a beta sheet contains a helix residue");
      }
    }
    if (n_e < 12) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("only ") << n_e << " of 20 strand residues in a two-stranded"
        " sheet were assigned strand");
    }
    for (size_t i = strand_len; i < strand_len + 3; i++) {
      if (ss[i] == ss_strand) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "the turn between two strands was assigned strand");
      }
    }
  }

  /* A single strand with nothing alongside it is extended chain, not a sheet,
  and must not be drawn as an arrow. This is the case that motivated the
  pairing test: without it the distance criteria report strand in haemoglobin,
  which has no sheet at all, and run ubiquitin's strands past their ends.
  */
  {
    ChainSegment lone;
    for (size_t i = 0; i < 20; i++) {
      BackboneResidue r;
      r.number = (int)(i + 1);
      r.ca = vec3d(3.3*i, 0, (i % 2) ? 0.9 : -0.9);
      lone.residues.AddCopy(r);
    }
    TArrayList<short> ss;
    AssignSecondaryStructure(lone, ss);
    for (size_t i = 0; i < ss.Count(); i++) {
      if (ss[i] != ss_coil) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "an unpaired extended chain was assigned secondary structure");
      }
    }
  }

  /* The negative control: a wide, gently curving arc at peptide spacing. It is
  neither helix nor strand and must come out coil.

  The first version of this case was a planar zigzag at 3.9 A steps, which the
  assignment called strand - correctly. That is strand geometry: its CA(i) to
  CA(i+2) distance is 7.4 A against the ideal 6.7. A negative control has to be
  something genuinely unlike both, not something merely built by hand.
  */
  {
    ChainSegment arc;
    const double radius = 20;
    const double step = 2*asin(1.9/radius);   // 3.8 A chord
    for (size_t i = 0; i < 12; i++) {
      BackboneResidue r;
      r.number = (int)(i + 1);
      r.ca = vec3d(radius*cos(step*i), radius*sin(step*i), 0);
      arc.residues.AddCopy(r);
    }
    TArrayList<short> ss;
    AssignSecondaryStructure(arc, ss);
    size_t assigned = 0;
    for (size_t i = 0; i < ss.Count(); i++) {
      if (ss[i] != ss_coil) {
        assigned++;
      }
    }
    if (assigned != 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("a gently curving arc was assigned structure for ") << assigned
        << " of " << ss.Count() << " residues");
    }
  }

  // too short to say anything, and it must say nothing rather than throw
  {
    ChainSegment tiny;
    for (size_t i = 0; i < 3; i++) {
      BackboneResidue r;
      r.number = (int)(i + 1);
      r.ca = vec3d(3.8*i, 0, 0);
      tiny.residues.AddCopy(r);
    }
    TArrayList<short> ss;
    AssignSecondaryStructure(tiny, ss);
    if (ss.Count() != 3) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "a short segment did not produce one assignment per residue");
    }
    for (size_t i = 0; i < ss.Count(); i++) {
      if (ss[i] != ss_coil) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "a three-residue segment was assigned secondary structure");
      }
    }
  }
}
//.............................................................................
void ProteinTests(OlxTests &t) {
  t.Add(test::ProteinBackboneTest);
  t.Add(test::ResidueKindTest);
  t.Add(test::ChainContinuityTest);
  t.Add(test::SecondaryStructureTest);
}
};  //namespace test
