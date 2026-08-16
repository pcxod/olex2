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

/* polymer topology: which atoms form the backbone and how residues join into
chains. Model knowledge rather than graphics, so it lives here - the cartoon is
one consumer, residue selection and fragment work are others
*/
BeginXlibNamespace()

namespace protein {

/* the minimum an atom needs to be classified, positions Cartesian. Not TSAtom
on purpose: the classifier is then a pure function of geometry, testable
without a lattice, a file or a graphics context
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

/* assigns N, CA, C and O within a residue by geometry, not by label: a SHELX
model names them by element and sequence, so crambin's first residue is
N3, C6, C7, O3. Every candidate is scored against ideal peptide geometry and
the best wins, not the first inside a window - in haemoglobin at 1.74 A a
serine CB-OG fits any window wide enough for that structure's own C=O. atoms
holds the residue's own plus, marked foreign, bonded atoms of its neighbours
*/
BackboneRoles FindBackbone(const TTypeList<ProteinAtom> &atoms);

/* chemical character from the RESI class name, along three independent axes.
Name lookups on purpose, unlike the backbone classifier: class names survive
where atom labels do not, a converted model keeping RESI THR 1. Three axes
because one list would have to choose - aspartate is acidic and hydrophilic,
and offering those as alternatives answers neither question
*/

/* Whether the sidechain prefers the protein's interior or the solvent. */
enum ResiduePolarity {
  rpUnknown = 0,
  rpLipophilic,
  rpHydrophilic
};
/* glycine and proline count as hydrophilic, cysteine as lipophilic. All three
are borderline, but this axis is about the solvent and a residue left out would
read as unknown, which is worse than a defensible call
*/
ResiduePolarity ClassifyPolarity(const olxstr &class_name);

/* Acid/base character at neutral pH. */
enum ResidueCharge {
  rcUnknown = 0,
  rcAcidic,
  rcBasic,
  rcNeutral
};
/* the protonation variants follow their parent - ASH, GLH and LYN are neutral
forms, but a colour key should show the acidic and basic residues where a
reader expects them
*/
ResidueCharge ClassifyCharge(const olxstr &class_name);

/* which of the twenty standard amino acids, as an index into a fixed
alphabetical order, InvalidIndex otherwise. Variants map onto their parent:
CYX and CSO, HID/HIE/HIP, ASH, GLH, LYN, and MSE to methionine
*/
size_t ResidueIndex(const olxstr &class_name);
size_t StandardResidueCount();
/* The three-letter code for an index from ResidueIndex, or an empty string. */
olxstr StandardResidueName(size_t index);

/* non-hydrogen atom count of a standard amino acid from its RESI class name,
0 when the name is not known. For weighting a residue drawn as ribbon: the
model centre averages atom positions, so a chain replaced by a ribbon has to
carry the weight its atoms would have, and tryptophan is 3.5x glycine. Zero is
a real answer - the caller then counts the residue's atoms in the model
*/
size_t ResidueAtomCount(const olxstr &class_name);

/* A residue reduced to what a backbone trace needs. */
struct BackboneResidue {
  int number;
  // TResidue id, so a per-residue display mask can index back to the model
  size_t resi_id;
  vec3d ca, o;
  bool has_o;
  /* TCAtom ids of the four backbone atoms, or InvalidIndex. The positions are
  all a ribbon needs, but a display has to know which atoms it stands for or a
  sidechain drawn as sticks brings the backbone with it, inside the ribbon
  */
  size_t n_id, ca_id, c_id, o_id;
  BackboneResidue()
    : number(0), resi_id(InvalidIndex), has_o(false),
      n_id(InvalidIndex), ca_id(InvalidIndex),
      c_id(InvalidIndex), o_id(InvalidIndex)
  {}
};

/* a run of residues whose backbone is continuous. A break starts a new segment
rather than a gap flag on a longer one - drawing a ribbon across a break is the
most visible way to get this wrong, and splitting makes it impossible
*/
struct ChainSegment {
  olxch chain_id;
  // symmetry matrix id, so each generated copy is traced separately
  uint32_t matrix_id;
  TTypeList<BackboneResidue> residues;
  ChainSegment() : chain_id('~'), matrix_id(0) {}
};

/* CA-CA distance above which the backbone is broken. A peptide bond gives
about 3.8 A; 4.5 allows a strained model without joining separate pieces.
Numbering is not trusted on its own - insertion codes are dropped on import and
TER is ignored - only to widen this across a gap, never to join
*/
extern const double default_break_distance;

/* whether two residues adjacent in numbering are one continuous piece of
backbone. The limit widens across a numbered gap rather than the gap breaking
the chain: one residue missing leaves its neighbours 7.6 A apart, and at a 5%
failure rate over 574 residues that is thirty spurious breaks. Only over a few
residues - beyond that a ribbon across the gap is an invention
*/
bool IsContinuous(const BackboneResidue &prev, const BackboneResidue &next,
  double break_distance = default_break_distance);

/* Secondary structure of one residue. */
const short
  ss_coil = 0,
  ss_helix = 1,
  ss_strand = 2;

/* assigns secondary structure along a segment, one entry per residue, from CA
positions alone after P-SEA (Labesse 1997) - distances, virtual angle and
virtual torsion, all of which survive a model with no carbonyl oxygens.
Weighted rather than thresholded: each descriptor scores a Gaussian around its
ideal, every window votes for the residues it spans, and the winner needs a
margin. Hysteresis then drops helices under four residues and strands under
three
*/
void AssignSecondaryStructure(const ChainSegment &seg, TArrayList<short> &out);

/* groups atoms into chain segments ordered by residue number, keyed by chain
id and symmetry matrix so a generated copy is traced separately
*/
void ExtractSegments(const TAsymmUnit &au, const TSAtomPList &atoms,
  TTypeList<ChainSegment> &out,
  double break_distance = default_break_distance);

} // namespace protein

EndXlibNamespace()
#endif
