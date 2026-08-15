/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_protein_H
#define __olx_xlib_protein_H
#include "xbase.h"
#include "typelist.h"
#include "satom.h"
#include "asymmunit.h"

/* Polymer topology: which atoms form the backbone, and how residues join into
chains.

This is model knowledge rather than graphics, so it lives in xlib. The cartoon
renderer is one consumer; residue selection and fragment work are others, and
none of them should carry their own copy of the rules.
*/
BeginXlibNamespace()

namespace protein {

/* The minimum an atom needs to be classified. Positions are Cartesian.

Deliberately not TSAtom: the classifier is then a pure function of geometry,
testable without a lattice, a file or a graphics context, which is what lets the
regression test exercise the shipped code rather than a copy of it.
*/
struct ProteinAtom {
  short z;
  vec3d crd;
  // true when the atom belongs to a different residue than the one being
  // classified; the peptide bond is the strongest evidence there is
  bool foreign;
  ProteinAtom() : z(0), foreign(false) {}
  ProteinAtom(short z, const vec3d &crd, bool foreign = false)
    : z(z), crd(crd), foreign(foreign)
  {}
};

/* Indices into the atom list passed to FindBackbone, or InvalidIndex. */
struct BackboneRoles {
  size_t n, ca, c, o;
  BackboneRoles()
    : n(InvalidIndex), ca(InvalidIndex), c(InvalidIndex), o(InvalidIndex)
  {}
  bool IsValid() const { return ca != InvalidIndex; }
};

/* Assigns N, CA, C and O within one residue, by geometry rather than by label.

Labels cannot be relied on. A PDB-derived file names them N, CA, C and O, but a
SHELX model refined from the same structure names them by element and sequence:
crambin's first residue is N3, C6, C7, O3. A label test finds no backbone at all
there while succeeding on rubredoxin. Geometry is the same in both.

Every N-CA-C=O candidate is scored against ideal peptide geometry and the best
wins, rather than the first inside a distance window. A window fails on older or
lower-resolution models: in haemoglobin at 1.74 A a serine CB-OG sits inside any
window wide enough to admit that structure's own backbone C=O, so the sidechain
hydroxyl is taken for the carbonyl. Scoring separates them because a real
backbone matches all three distances at once, and because the backbone carbonyl
carbon also carries the next residue's nitrogen, which a sidechain C-OH never
does.

`atoms` should hold the residue's own atoms plus, marked foreign, any atom of a
neighbouring residue close enough to be bonded. Returns an invalid result for
anything that is not an amino acid, which is how waters, ions and ligands are
excluded without a residue-name table.
*/
BackboneRoles FindBackbone(const TTypeList<ProteinAtom> &atoms);

/* Chemical character of a residue, from its RESI class name, along three
independent axes.

Class names survive where atom labels do not: a SHELX model converted from a PDB
entry keeps `RESI THR 1` even when the atoms inside are called N3, C6, C7. So
these are name lookups on purpose, unlike the backbone classifier above, and
each returns its unknown value rather than guessing when the name is not one it
knows.

Three axes rather than one list of kinds. A single classification had to choose,
for each residue, which of its properties to show, and so answered neither
question: aspartate is acidic *and* hydrophilic, and a key that offered "acidic,
basic, polar, hydrophobic" made those look like alternatives. Asked separately,
each axis has a short key that means one thing.
*/

/* Whether the sidechain prefers the protein's interior or the solvent. */
enum ResiduePolarity {
  rpUnknown = 0,
  rpLipophilic,
  rpHydrophilic
};
/* Glycine and proline count as hydrophilic and cysteine as lipophilic. All
three are borderline on any scale - glycine has no sidechain to speak of - but
this axis is about where a residue sits relative to the solvent, and a residue
left out of it would read as unknown, which is worse than a defensible call.
*/
ResiduePolarity ClassifyPolarity(const olxstr &class_name);

/* Acid/base character at neutral pH. */
enum ResidueCharge {
  rcUnknown = 0,
  rcAcidic,
  rcBasic,
  rcNeutral
};
/* The protonation variants follow their parent: ASH and GLH are the neutral
forms of aspartate and glutamate and LYN of lysine, but a reader looking at a
colour key wants to find the acidic and basic residues where they expect them,
and a model that names them separately has said something about protonation
rather than about chemistry.
*/
ResidueCharge ClassifyCharge(const olxstr &class_name);

/* Which of the twenty standard amino acids, as an index into a fixed
alphabetical order, or InvalidIndex for anything else.

Variants map onto their parent: CYX and CSO to cysteine, HID, HIE and HIP to
histidine, ASH to aspartate, GLH to glutamate, LYN to lysine and MSE, which is
common enough in real entries to be worth naming, to methionine.
*/
size_t ResidueIndex(const olxstr &class_name);
size_t StandardResidueCount();
/* The three-letter code for an index from ResidueIndex, or an empty string. */
olxstr StandardResidueName(size_t index);

/* How many non-hydrogen atoms a standard amino acid has, from its RESI class
name, or 0 when the name is not one this knows.

For weighting a residue drawn as ribbon rather than as atoms. The model centre
is an average over atom positions, so a chain replaced by a ribbon has to
re-enter that average carrying the weight its atoms would have had, or the
centre drifts towards whatever is still drawn as atoms. Tryptophan is three and
a half times glycine, which is too large a difference to paper over by counting
residues instead.

Zero is a real answer rather than a failure: the caller falls back to counting
the residue's atoms in the model, which is also the right thing to do for a
modified or non-standard residue, and for hydrogens if the model has them.

Hydrogen is excluded because a protein refined from diffraction data usually
carries none, so including it would be wrong by more than the differences
between the residues this is meant to distinguish.
*/
size_t ResidueAtomCount(const olxstr &class_name);

/* A residue reduced to what a backbone trace needs. */
struct BackboneResidue {
  int number;
  // TResidue id, so a per-residue display mask can index back to the model
  size_t resi_id;
  vec3d ca, o;
  bool has_o;
  /* TCAtom ids of the four backbone atoms, or InvalidIndex.

  The positions above are all a ribbon needs, but a display which draws the
  backbone as a ribbon has to know which *atoms* that ribbon stands for, or it
  cannot show a sidechain as sticks without drawing the backbone twice - once
  as the ribbon and once as the sticks running through it. Ids rather than
  labels because the classifier is topological: crambin's first residue is
  N3, C6, C7, O3, and a label test finds no backbone in it at all.
  */
  size_t n_id, ca_id, c_id, o_id;
  BackboneResidue()
    : number(0), resi_id(InvalidIndex), has_o(false),
      n_id(InvalidIndex), ca_id(InvalidIndex),
      c_id(InvalidIndex), o_id(InvalidIndex)
  {}
};

/* A run of residues whose backbone is actually continuous.

A break starts a new segment rather than a longer one carrying a gap flag:
drawing a ribbon across a chain break is the most visible way to get this wrong,
and splitting makes it impossible by construction.
*/
struct ChainSegment {
  olxch chain_id;
  // symmetry matrix id, so each generated copy is traced separately
  uint32_t matrix_id;
  TTypeList<BackboneResidue> residues;
  ChainSegment() : chain_id('~'), matrix_id(0) {}
};

/* CA-CA distance above which the backbone is treated as broken.

A peptide bond gives about 3.8 A between consecutive CA. 4.5 A allows for a
strained or poorly refined model without joining genuinely separate pieces.
Residue numbering is not trusted on its own for this: insertion codes are
dropped on PDB import and TER records are ignored, so consecutive numbers do
not imply a bond. It is used only to widen this limit across a numbered gap,
never to join two residues the distance rejects.
*/
extern const double default_break_distance;

/* Whether two residues, adjacent in a chain's numbering order, are one
continuous piece of backbone.

The limit is widened across a numbered gap rather than the gap being treated as
a break: one residue the classifier cannot place, or one simply absent from the
model, otherwise leaves its two neighbours 7.6 A apart and splits the chain in
two. Over a 574-residue structure with a 5% failure rate that is about thirty
spurious breaks, which is what a rainbow-coloured trace shows as dozens of short
runs instead of four long ones.

Numbering is never trusted on its own, only to widen a limit the distance must
still pass, and only over a few residues: beyond that a ribbon drawn across the
gap is an invention rather than an interpolation.
*/
bool IsContinuous(const BackboneResidue &prev, const BackboneResidue &next,
  double break_distance = default_break_distance);

/* Secondary structure of one residue. */
const short
  ss_coil = 0,
  ss_helix = 1,
  ss_strand = 2;

/* Assigns secondary structure along a segment, one entry per residue.

From CA positions alone, in the manner of P-SEA (Labesse 1997): the distances
CA(i) to CA(i+2), (i+3) and (i+4), the virtual angle at CA(i) and the virtual
torsion over four consecutive CA separate helix from strand cleanly, and every
one of them survives a model with no carbonyl oxygens, which the backbone
classifier shows is a real case here.

Evidence is weighted rather than thresholded, per the rule that earned itself on
the backbone classifier: each descriptor contributes a Gaussian score around its
ideal, every window votes for all the residues it spans, and the winner needs a
margin over the loser. Run-length hysteresis then removes helices shorter than
four residues and strands shorter than three, which is what stops a single
coincidental window from putting a two-residue helix in the middle of a loop.

A hydrogen-bond assignment in the manner of DSSP would be more faithful where
the oxygens are present, and is the obvious later refinement; this is uniform
across models that have them and models that do not.
*/
void AssignSecondaryStructure(const ChainSegment &seg, TArrayList<short> &out);

/* Groups atoms into chain segments, ordered by residue number.

Keyed by chain id and symmetry matrix, so a generated copy is traced separately
rather than spliced into the asymmetric-unit chain.
*/
void ExtractSegments(const TAsymmUnit &au, const TSAtomPList &atoms,
  TTypeList<ChainSegment> &out,
  double break_distance = default_break_distance);

} // namespace protein

EndXlibNamespace()
#endif
