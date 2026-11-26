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

// polymer topology: backbone atoms and how residues join into chains
BeginXlibNamespace()

namespace protein {

/* what the classifier needs of an atom, Cartesian; not TSAtom, so it is a pure
function of geometry, testable without a lattice
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

// indices into the atom list passed to FindBackbone, or InvalidIndex
struct BackboneRoles {
  size_t n, ca, c, o;
  BackboneRoles()
    : n(InvalidIndex), ca(InvalidIndex), c(InvalidIndex), o(InvalidIndex)
  {}
  bool IsValid() const { return ca != InvalidIndex; }
};

/* N, CA, C and O by geometry, not label (SHELX labels are N3, C6...): best
score against ideal peptide geometry, not the first in a window (Ser CB-OG can
pass for C=O); foreign atoms are bonded atoms of the neighbours
*/
BackboneRoles FindBackbone(const TTypeList<ProteinAtom> &atoms);

/* chemical character from the RESI class name (it survives where labels do
not), on independent axes: Asp is both acidic and hydrophilic
*/

// whether the sidechain prefers the protein's interior or the solvent
enum ResiduePolarity {
  rpUnknown = 0,
  rpLipophilic,
  rpHydrophilic
};
// borderline: Gly and Pro count as hydrophilic, Cys as lipophilic
ResiduePolarity ClassifyPolarity(const olxstr &class_name);

// acid/base character at neutral pH
enum ResidueCharge {
  rcUnknown = 0,
  rcAcidic,
  rcBasic,
  rcNeutral
};
// protonation variants (ASH, GLH, LYN) follow their parent
ResidueCharge ClassifyCharge(const olxstr &class_name);

/* index of the standard amino acid in alphabetical order, or InvalidIndex;
variants (CYX, CSO, HID/HIE/HIP, ASH, GLH, LYN, MSE) map onto their parent
*/
size_t ResidueIndex(const olxstr &class_name);
size_t StandardResidueCount();
// the three-letter code for an index from ResidueIndex, or an empty string
olxstr StandardResidueName(size_t index);

/* non-hydrogen atom count by RESI class, 0 if unknown (the caller then counts
the model's atoms); weights a ribbon residue in the model centre
*/
size_t ResidueAtomCount(const olxstr &class_name);

/* hydrogens of an atom at neutral pH by RESI class and PDB name, and its heavy
neighbour count; Lys NZ and N-terminus NH3+, carboxylates none, plain His
ND1/NE2 h=-1 (tautomer unknown), SO4/ACT/FMT as anions
*/
bool ResidueHydrogens(const olxstr &class_name, const olxstr &atom_name,
  int &h, int &heavy);

// a residue reduced to what a backbone trace needs
struct BackboneResidue {
  int number;
  // TResidue id, so a per-residue display mask can index back to the model
  size_t resi_id;
  vec3d ca, o;
  bool has_o;
  /* TCAtom ids of the backbone atoms, or InvalidIndex, so a sidechain drawn
  as sticks can leave out the atoms the ribbon stands for
  */
  size_t n_id, ca_id, c_id, o_id;
  BackboneResidue()
    : number(0), resi_id(InvalidIndex), has_o(false),
      n_id(InvalidIndex), ca_id(InvalidIndex),
      c_id(InvalidIndex), o_id(InvalidIndex)
  {}
};

// a continuous backbone run; a break starts a new one, no ribbon spans it
struct ChainSegment {
  olxch chain_id;
  // symmetry matrix id, so each generated copy is traced separately
  uint32_t matrix_id;
  TTypeList<BackboneResidue> residues;
  ChainSegment() : chain_id('~'), matrix_id(0) {}
};

/* CA-CA break distance: 3.8 A per peptide, 4.5 allows strain; numbering only
widens it across a gap, never joins on its own
*/
extern const double default_break_distance;

/* continuity of residues adjacent in numbering; the limit widens over a short
numbered gap (one missing residue leaves 7.6 A), not over a long one
*/
bool IsContinuous(const BackboneResidue &prev, const BackboneResidue &next,
  double break_distance = default_break_distance);

// secondary structure of one residue
const short
  ss_coil = 0,
  ss_helix = 1,
  ss_strand = 2;

/* secondary structure per residue from CA positions only, after P-SEA
(Labesse 1997): Gaussian scores, window votes, a winning margin; helices under
four and strands under three residues are dropped
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
