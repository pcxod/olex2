/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "protein.h"
#include "residue.h"

BeginXlibNamespace()

namespace protein {

const double default_break_distance = 4.5;

namespace {
  // ideal peptide geometry, in Angstroms
  const double ideal_c_o = 1.231, ideal_ca_c = 1.525, ideal_n_ca = 1.458;
  /* Generous search gates. They only bound the search; the scoring below is
  what discriminates, so widening them costs time but not correctness.
  */
  const double max_c_o = 1.60, max_ca_c = 1.80, max_n_ca = 1.75;
  const double max_peptide_c_n = 1.55;
  /* Worst total deviation still accepted, summed over the three distances.
  Loosening this saturates rather than helping: on haemoglobin 0.55 leaves 30
  residues unassigned and 1.5 leaves 27, so what remains has no N-CA-C=O
  candidate at all and is not a threshold problem.
  */
  const double max_deviation = 0.9;
  // credit for carrying the next residue's nitrogen, in the same units
  const double peptide_bond_credit = 0.35;

  int ResidueCmp(const BackboneResidue &a, const BackboneResidue &b) {
    return olx_cmp(a.number, b.number);
  }

  const double ideal_ca_ca = 3.8;
  /* Bounds for a recovered CA, and the widest mean deviation accepted. Wider
  than a peptide step in both directions: the residues this runs on are the ones
  the classifier could not place, which is usually a sign the geometry is poor.
  */
  const double min_rec_ca_ca = 2.8, max_rec_ca_ca = 4.7,
    max_rec_deviation = 0.55;
  // credit for carrying a nitrogen at CA-N distance, as the peptide credit above
  const double amine_credit = 0.25;
  const double max_ca_n = 1.65;
  /* Numbered gap over which the break test is widened. Beyond this the missing
  stretch is long enough that a ribbon drawn across it would be an invention
  rather than an interpolation, whatever the distance says.
  */
  const int max_bridge_residues = 3;

  /* finds the CA of a residue the classifier rejected, from its traced
  neighbours. One unplaced residue splits a chain, so a 5% failure rate over
  574 residues costs thirty spurious breaks rather than 5% coverage
  */
  size_t RecoverCA(const TTypeList<ProteinAtom> &atoms,
    const vec3d *prev_ca, const vec3d *next_ca)
  {
    if (prev_ca == 0 && next_ca == 0) {
      return InvalidIndex;
    }
    size_t best = InvalidIndex;
    double best_score = 0;
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i].z != 6 || atoms[i].foreign) {
        continue;
      }
      double score = 0;
      int n = 0;
      bool rejected = false;
      const vec3d *flank[2] = { prev_ca, next_ca };
      for (int k = 0; k < 2; k++) {
        if (flank[k] == 0) {
          continue;
        }
        const double d = atoms[i].crd.DistanceTo(*flank[k]);
        if (d < min_rec_ca_ca || d > max_rec_ca_ca) {
          rejected = true;
          break;
        }
        score += olx_abs(d - ideal_ca_ca);
        n++;
      }
      if (rejected || n == 0) {
        continue;
      }
      score /= n;
      // a CA carries the residue nitrogen; most sidechain carbons do not
      for (size_t k = 0; k < atoms.Count(); k++) {
        if (atoms[k].z == 7 && !atoms[k].foreign &&
          atoms[i].crd.DistanceTo(atoms[k].crd) <= max_ca_n)
        {
          score -= amine_credit;
          break;
        }
      }
      if (best == InvalidIndex || score < best_score) {
        best_score = score;
        best = i;
      }
    }
    return (best != InvalidIndex && best_score <= max_rec_deviation)
      ? best : InvalidIndex;
  }

  // ideal CA-only descriptors, from P-SEA (Labesse 1997), and their spreads
  struct SSIdeal {
    double d2, d2s, d3, d3s, d4, d4s, tau, taus, alpha, alphas;
  };
  const SSIdeal helix_ideal = {
    5.5, 0.5, 5.3, 0.5, 6.4, 0.6, 89, 12, 50, 20
  };
  const SSIdeal strand_ideal = {
    6.7, 0.6, 9.9, 0.9, 12.4, 1.1, 124, 14, -170, 45
  };

  double Bell(double x, double m, double s) {
    const double t = (x - m)/s;
    return exp(-0.5*t*t);
  }
  // the same, on a circle, so 179 and -179 degrees are near neighbours
  double BellDeg(double x, double m, double s) {
    double d = x - m;
    while (d > 180) { d -= 360; }
    while (d < -180) { d += 360; }
    return exp(-0.5*(d/s)*(d/s));
  }

  double VirtualAngle(const vec3d &a, const vec3d &b, const vec3d &c) {
    vec3d u = a - b, v = c - b;
    const double lu = u.Length(), lv = v.Length();
    if (lu < 1e-6 || lv < 1e-6) {
      return 0;
    }
    double cs = u.DotProd(v)/(lu*lv);
    cs = olx_max(-1.0, olx_min(1.0, cs));
    return acos(cs)*180/M_PI;
  }

  double VirtualTorsion(const vec3d &a, const vec3d &b, const vec3d &c,
    const vec3d &d)
  {
    const vec3d b1 = b - a, b2 = c - b, b3 = d - c;
    const vec3d n1 = b1.XProdVec(b2), n2 = b2.XProdVec(b3);
    const double l1 = n1.Length(), l2 = n2.Length(), lb2 = b2.Length();
    if (l1 < 1e-6 || l2 < 1e-6 || lb2 < 1e-6) {
      return 0;
    }
    const double x = n1.DotProd(n2)/(l1*l2);
    const double y = n1.XProdVec(n2).DotProd(b2)/(l1*l2*lb2);
    return atan2(y, olx_max(-1.0, olx_min(1.0, x)))*180/M_PI;
  }
}

//.............................................................................
BackboneRoles FindBackbone(const TTypeList<ProteinAtom> &atoms) {
  BackboneRoles best;
  double best_score = 0;
  bool have_best = false;
  const size_t ac = atoms.Count();

  for (size_t ci = 0; ci < ac; ci++) {
    if (atoms[ci].z != 6 || atoms[ci].foreign) {
      continue;
    }
    // does this carbon carry a nitrogen of another residue?
    bool has_peptide_n = false;
    for (size_t k = 0; k < ac; k++) {
      if (atoms[k].z == 7 && atoms[k].foreign
        && atoms[ci].crd.DistanceTo(atoms[k].crd) <= max_peptide_c_n)
      {
        has_peptide_n = true;
        break;
      }
    }
    for (size_t oi = 0; oi < ac; oi++) {
      if (atoms[oi].z != 8 || atoms[oi].foreign) {
        continue;
      }
      const double d_co = atoms[ci].crd.DistanceTo(atoms[oi].crd);
      if (d_co > max_c_o) {
        continue;
      }
      for (size_t ai = 0; ai < ac; ai++) {
        if (ai == ci || atoms[ai].z != 6 || atoms[ai].foreign) {
          continue;
        }
        const double d_cac = atoms[ci].crd.DistanceTo(atoms[ai].crd);
        if (d_cac > max_ca_c) {
          continue;
        }
        for (size_t ni = 0; ni < ac; ni++) {
          if (atoms[ni].z != 7 || atoms[ni].foreign) {
            continue;
          }
          const double d_nca = atoms[ai].crd.DistanceTo(atoms[ni].crd);
          if (d_nca > max_n_ca) {
            continue;
          }
          double score = olx_abs(d_co - ideal_c_o)
            + olx_abs(d_cac - ideal_ca_c)
            + olx_abs(d_nca - ideal_n_ca);
          if (has_peptide_n) {
            score -= peptide_bond_credit;
          }
          if (!have_best || score < best_score) {
            best_score = score;
            have_best = true;
            best.n = ni;  best.ca = ai;  best.c = ci;  best.o = oi;
          }
        }
      }
    }
  }
  if (!have_best || best_score > max_deviation) {
    return BackboneRoles();
  }
  return best;
}
//.............................................................................
namespace {
  /* The twenty, alphabetically by three-letter code. Alphabetical rather than
  grouped by property: the other two axes are what the properties are for, and a
  reader looking a residue up in a twenty-row key finds it by name.
  */
  const char *standard_residues[] = {
    "ALA", "ARG", "ASN", "ASP", "CYS", "GLN", "GLU", "GLY", "HIS", "ILE",
    "LEU", "LYS", "MET", "PHE", "PRO", "SER", "THR", "TRP", "TYR", "VAL"
  };
  const size_t standard_residue_count =
    sizeof(standard_residues)/sizeof(standard_residues[0]);

  // upper case, trimmed, and with the variants folded onto their parent
  olxstr normalise_residue(const olxstr &class_name) {
    olxstr n = olxstr(class_name).Trim(' ').UpperCase();
    if (n == "CYX" || n == "CSO") { return olxstr("CYS"); }
    if (n == "HID" || n == "HIE" || n == "HIP") { return olxstr("HIS"); }
    if (n == "ASH") { return olxstr("ASP"); }
    if (n == "GLH") { return olxstr("GLU"); }
    if (n == "LYN") { return olxstr("LYS"); }
    if (n == "MSE") { return olxstr("MET"); }
    return n;
  }
}
//.............................................................................
size_t StandardResidueCount() {
  return standard_residue_count;
}
//.............................................................................
olxstr StandardResidueName(size_t index) {
  return index < standard_residue_count
    ? olxstr(standard_residues[index]) : EmptyString();
}
//.............................................................................
size_t ResidueIndex(const olxstr &class_name) {
  const olxstr n = normalise_residue(class_name);
  if (n.IsEmpty()) {
    return InvalidIndex;
  }
  for (size_t i = 0; i < standard_residue_count; i++) {
    if (n == standard_residues[i]) {
      return i;
    }
  }
  return InvalidIndex;
}
//.............................................................................
ResiduePolarity ClassifyPolarity(const olxstr &class_name) {
  const olxstr n = normalise_residue(class_name);
  if (n.IsEmpty()) {
    return rpUnknown;
  }
  if (n == "ALA" || n == "VAL" || n == "LEU" || n == "ILE" || n == "MET" ||
    n == "PHE" || n == "TRP" || n == "CYS")
  {
    return rpLipophilic;
  }
  if (n == "SER" || n == "THR" || n == "ASN" || n == "GLN" || n == "TYR" ||
    n == "ASP" || n == "GLU" || n == "LYS" || n == "ARG" || n == "HIS" ||
    n == "GLY" || n == "PRO")
  {
    return rpHydrophilic;
  }
  return rpUnknown;
}
//.............................................................................
ResidueCharge ClassifyCharge(const olxstr &class_name) {
  const olxstr n = normalise_residue(class_name);
  if (n.IsEmpty()) {
    return rcUnknown;
  }
  if (n == "ASP" || n == "GLU") {
    return rcAcidic;
  }
  if (n == "ARG" || n == "LYS" || n == "HIS") {
    return rcBasic;
  }
  // everything else that is an amino acid at all
  return ResidueIndex(n) == InvalidIndex ? rcUnknown : rcNeutral;
}
//.............................................................................
size_t ResidueAtomCount(const olxstr &class_name) {
  olxstr n = olxstr(class_name).Trim(' ').UpperCase();
  if (n.IsEmpty()) {
    return 0;
  }
  /* Non-hydrogen atoms, backbone included. Hydrogen is left out because a
  protein model refined from diffraction data usually has none, and a count
  that assumed them would be wrong by more than the differences between the
  residues themselves.
  */
  if (n == "GLY") { return 4; }
  if (n == "ALA") { return 5; }
  if (n == "SER" || n == "CYS" || n == "CYX" || n == "CSO") { return 6; }
  if (n == "THR" || n == "VAL" || n == "PRO") { return 7; }
  if (n == "ASN" || n == "ASP" || n == "ASH" || n == "ILE" || n == "LEU" ||
    n == "MET" || n == "MSE")
  {
    return 8;
  }
  if (n == "GLN" || n == "GLU" || n == "GLH" || n == "LYS" || n == "LYN") {
    return 9;
  }
  if (n == "HIS" || n == "HID" || n == "HIE" || n == "HIP") { return 10; }
  if (n == "PHE" || n == "ARG") { return 11; }
  if (n == "TYR") { return 12; }
  if (n == "TRP") { return 14; }
  return 0;
}
//.............................................................................
bool ResidueHydrogens(const olxstr &class_name, const olxstr &atom_name,
  int &h, int &heavy)
{
  struct Site { const char *resi, *atom; short h, heavy; };
  static const Site sites[] = {
    // the variants, before they are folded onto their parent
    {"HID", "ND1", 1, 2}, {"HID", "NE2", 0, 2},
    {"HIE", "ND1", 0, 2}, {"HIE", "NE2", 1, 2},
    {"HIP", "ND1", 1, 2}, {"HIP", "NE2", 1, 2},
    {"ASH", "OD2", 1, 1}, {"GLH", "OE2", 1, 1}, {"CYX", "SG", 0, 2},
    {"GLY", "CA", 2, 2}, {"PRO", "N", 0, 3},
    {"ALA", "CB", 3, 1},
    {"ARG", "CB", 2, 2}, {"ARG", "CG", 2, 2}, {"ARG", "CD", 2, 2},
    {"ARG", "NE", 1, 2}, {"ARG", "CZ", 0, 3}, {"ARG", "NH1", 2, 1},
    {"ARG", "NH2", 2, 1},
    {"ASN", "CB", 2, 2}, {"ASN", "CG", 0, 3}, {"ASN", "OD1", 0, 1},
    {"ASN", "ND2", 2, 1},
    {"ASP", "CB", 2, 2}, {"ASP", "CG", 0, 3}, {"ASP", "OD1", 0, 1},
    {"ASP", "OD2", 0, 1},
    {"CYS", "CB", 2, 2}, {"CYS", "SG", 1, 1},
    {"GLN", "CB", 2, 2}, {"GLN", "CG", 2, 2}, {"GLN", "CD", 0, 3},
    {"GLN", "OE1", 0, 1}, {"GLN", "NE2", 2, 1},
    {"GLU", "CB", 2, 2}, {"GLU", "CG", 2, 2}, {"GLU", "CD", 0, 3},
    {"GLU", "OE1", 0, 1}, {"GLU", "OE2", 0, 1},
    {"HIS", "CB", 2, 2}, {"HIS", "CG", 0, 3}, {"HIS", "ND1", -1, 2},
    {"HIS", "CD2", 1, 2}, {"HIS", "CE1", 1, 2}, {"HIS", "NE2", -1, 2},
    {"ILE", "CB", 1, 3}, {"ILE", "CG1", 2, 2}, {"ILE", "CG2", 3, 1},
    {"ILE", "CD1", 3, 1},
    {"LEU", "CB", 2, 2}, {"LEU", "CG", 1, 3}, {"LEU", "CD1", 3, 1},
    {"LEU", "CD2", 3, 1},
    {"LYS", "CB", 2, 2}, {"LYS", "CG", 2, 2}, {"LYS", "CD", 2, 2},
    {"LYS", "CE", 2, 2}, {"LYS", "NZ", 3, 1},
    {"MET", "CB", 2, 2}, {"MET", "CG", 2, 2}, {"MET", "SD", 0, 2},
    {"MET", "SE", 0, 2}, {"MET", "CE", 3, 1},
    {"PHE", "CB", 2, 2}, {"PHE", "CG", 0, 3}, {"PHE", "CD1", 1, 2},
    {"PHE", "CD2", 1, 2}, {"PHE", "CE1", 1, 2}, {"PHE", "CE2", 1, 2},
    {"PHE", "CZ", 1, 2},
    {"PRO", "CB", 2, 2}, {"PRO", "CG", 2, 2}, {"PRO", "CD", 2, 2},
    {"SER", "CB", 2, 2}, {"SER", "OG", 1, 1},
    {"THR", "CB", 1, 3}, {"THR", "OG1", 1, 1}, {"THR", "CG2", 3, 1},
    {"TRP", "CB", 2, 2}, {"TRP", "CG", 0, 3}, {"TRP", "CD1", 1, 2},
    {"TRP", "CD2", 0, 3}, {"TRP", "NE1", 1, 2}, {"TRP", "CE2", 0, 3},
    {"TRP", "CE3", 1, 2}, {"TRP", "CZ2", 1, 2}, {"TRP", "CZ3", 1, 2},
    {"TRP", "CH2", 1, 2},
    {"TYR", "CB", 2, 2}, {"TYR", "CG", 0, 3}, {"TYR", "CD1", 1, 2},
    {"TYR", "CD2", 1, 2}, {"TYR", "CE1", 1, 2}, {"TYR", "CE2", 1, 2},
    {"TYR", "CZ", 0, 3}, {"TYR", "OH", 1, 1},
    {"VAL", "CB", 1, 3}, {"VAL", "CG1", 3, 1}, {"VAL", "CG2", 3, 1},
    // crystallisation anions: bond lengths read their oxygens as OH
    {"SO4", "*", 0, 0}, {"ACT", "CH3", 3, 1}, {"ACT", "*", 0, 0},
    {"FMT", "C", 1, 2}, {"FMT", "*", 0, 0},
    // the backbone of all of the above, last so GLY CA and PRO N win
    {"*", "N", 1, 2}, {"*", "CA", 1, 3}, {"*", "C", 0, 3}, {"*", "O", 0, 1},
    {"*", "OXT", 0, 1}
  };
  const olxstr raw = olxstr(class_name).Trim(' ').UpperCase(),
    parent = normalise_residue(raw),
    name = olxstr(atom_name).Trim(' ').UpperCase();
  // LYN is a neutral NH2, which no riding group of one heavy neighbour fits
  if (raw == "LYN" && name == "NZ") {
    return false;
  }
  const bool amino_acid = ResidueIndex(parent) != InvalidIndex;
  for (size_t i = 0; i < sizeof(sites)/sizeof(sites[0]); i++) {
    const Site &s = sites[i];
    if ((s.resi[0] == '*' ? amino_acid : (raw == s.resi || parent == s.resi)) &&
      (s.atom[0] == '*' || name == s.atom))
    {
      h = s.h;
      heavy = s.heavy;
      return true;
    }
  }
  return false;
}
//.............................................................................
bool IsContinuous(const BackboneResidue &prev, const BackboneResidue &next,
  double break_distance)
{
  int gap = next.number - prev.number;
  if (gap < 1) {
    gap = 1;      // duplicate numbering, from a dropped insertion code
  }
  if (gap > max_bridge_residues) {
    return false;
  }
  const double limit = break_distance*gap;
  return next.ca.QDistanceTo(prev.ca) <= limit*limit;
}
//.............................................................................
namespace {
  /* Removes runs shorter than the minimum, and fills a single-residue hole
  inside a run. Both directions matter: a lone helix residue in a loop is noise,
  and a lone loop residue inside a helix is a ribbon cut in half.
  */
  void SmoothRuns(TArrayList<short> &ss, short kind, size_t min_len) {
    const size_t n = ss.Count();
    // fill a one-residue hole
    for (size_t i = 1; i + 1 < n; i++) {
      if (ss[i] != kind && ss[i-1] == kind && ss[i+1] == kind) {
        ss[i] = kind;
      }
    }
    size_t i = 0;
    while (i < n) {
      if (ss[i] != kind) {
        i++;
        continue;
      }
      size_t j = i;
      while (j < n && ss[j] == kind) {
        j++;
      }
      if (j - i < min_len) {
        for (size_t k = i; k < j; k++) {
          ss[k] = ss_coil;
        }
      }
      i = j;
    }
  }
}
//.............................................................................
void AssignSecondaryStructure(const ChainSegment &seg, TArrayList<short> &out) {
  const size_t n = seg.residues.Count();
  out.SetCount(n);
  for (size_t i = 0; i < n; i++) {
    out[i] = ss_coil;
  }
  if (n < 5) {
    return;      // too short for any window to say anything
  }

  TArrayList<double> h(n), s(n), w(n);
  for (size_t i = 0; i < n; i++) {
    h[i] = s[i] = w[i] = 0;
  }

  /* Every five-residue window votes for all five of the residues it spans, so
  a residue in the middle of a helix collects five consistent votes and one at
  the end collects fewer. That is the smoothing, and it falls out of the
  evidence rather than being applied afterwards.
  */
  for (size_t i = 0; i + 4 < n; i++) {
    const vec3d &c0 = seg.residues[i].ca;
    const double d2 = c0.DistanceTo(seg.residues[i+2].ca),
      d3 = c0.DistanceTo(seg.residues[i+3].ca),
      d4 = c0.DistanceTo(seg.residues[i+4].ca);
    const double tau = VirtualAngle(c0, seg.residues[i+1].ca,
      seg.residues[i+2].ca);
    const double alpha = VirtualTorsion(c0, seg.residues[i+1].ca,
      seg.residues[i+2].ca, seg.residues[i+3].ca);

    const double wh = (
      Bell(d2, helix_ideal.d2, helix_ideal.d2s) +
      Bell(d3, helix_ideal.d3, helix_ideal.d3s) +
      Bell(d4, helix_ideal.d4, helix_ideal.d4s) +
      Bell(tau, helix_ideal.tau, helix_ideal.taus) +
      BellDeg(alpha, helix_ideal.alpha, helix_ideal.alphas))/5;
    const double ws = (
      Bell(d2, strand_ideal.d2, strand_ideal.d2s) +
      Bell(d3, strand_ideal.d3, strand_ideal.d3s) +
      Bell(d4, strand_ideal.d4, strand_ideal.d4s) +
      Bell(tau, strand_ideal.tau, strand_ideal.taus) +
      BellDeg(alpha, strand_ideal.alpha, strand_ideal.alphas))/5;
    for (size_t k = 0; k <= 4; k++) {
      h[i+k] += wh;
      s[i+k] += ws;
      w[i+k] += 1;
    }
  }

  /* The winner needs both a floor and a margin. Without the floor a residue in
  a loop is assigned whichever of the two it resembles slightly more; without
  the margin the boundary between a helix and the coil leaving it flickers.
  */
  const double min_evidence = 0.30, margin = 1.15;
  for (size_t i = 0; i < n; i++) {
    if (w[i] == 0) {
      continue;
    }
    const double hv = h[i]/w[i], sv = s[i]/w[i];
    if (hv >= min_evidence && hv > sv*margin) {
      out[i] = ss_helix;
    }
    else if (sv >= min_evidence && sv > hv*margin) {
      out[i] = ss_strand;
    }
  }
  // one turn of an alpha helix is 3.6 residues; a beta bridge is two
  SmoothRuns(out, ss_helix, 4);
  SmoothRuns(out, ss_strand, 3);

  /* a strand is only a strand if it is in a sheet. The descriptors above see
  the same local shape in an extended loop, and on their own report 5% strand
  in haemoglobin, which has no sheet, and run ubiquitin's strands past their
  ends. What separates the real ones is a partner about 5 A away, from
  elsewhere in the chain - the one place the assignment is not local
  */
  const double max_pairing = 5.6, max_pairing_sq = max_pairing*max_pairing;
  const size_t min_separation = 4;
  TArrayList<bool> paired(n);
  for (size_t i = 0; i < n; i++) {
    paired[i] = false;
  }
  for (size_t i = 0; i < n; i++) {
    if (out[i] != ss_strand) {
      continue;
    }
    for (size_t j = 0; j < n; j++) {
      if (out[j] != ss_strand ||
        (j > i ? j - i : i - j) < min_separation)
      {
        continue;
      }
      if (seg.residues[i].ca.QDistanceTo(seg.residues[j].ca) <= max_pairing_sq) {
        paired[i] = true;
        break;
      }
    }
  }
  /* trim unpaired residues off the ends rather than dropping the run: the
  common error is a strand running too far into the loop, and a run with no
  partner trims to nothing anyway. An interior one is kept - a beta bulge
  */
  size_t i = 0;
  while (i < n) {
    if (out[i] != ss_strand) {
      i++;
      continue;
    }
    size_t j = i;
    while (j < n && out[j] == ss_strand) {
      j++;
    }
    size_t a = i, b = j;
    while (a < b && !paired[a]) { a++; }
    while (b > a && !paired[b-1]) { b--; }
    for (size_t k = i; k < a; k++) { out[k] = ss_coil; }
    for (size_t k = b; k < j; k++) { out[k] = ss_coil; }
    i = j;
  }
  SmoothRuns(out, ss_strand, 3);
}
//.............................................................................
void ExtractSegments(const TAsymmUnit &au, const TSAtomPList &atoms,
  TTypeList<ChainSegment> &out, double break_distance)
{
  /* group atoms by residue, keyed also by symmetry matrix so a generated copy
  is its own chain. Not by walking TResidue::Next(): NextResidue passes an int
  that converts to olxstr and searches the no-chain bucket only
  */
  olxdict<uint64_t, TSAtomPList, TPrimitiveComparator> by_residue;
  TArrayList<uint64_t> order;

  for (size_t i = 0; i < atoms.Count(); i++) {
    TSAtom *sa = atoms[i];
    if (sa == 0 || sa->IsDeleted()) {
      continue;
    }
    const size_t resi_id = sa->CAtom().GetResiId();
    // residue 0 collects everything with no RESI: not polymer
    if (resi_id == 0 || resi_id == InvalidIndex) {
      continue;
    }
    const uint64_t key = (uint64_t(resi_id) << 32)
      | uint64_t(sa->GetMatrix().GetId());
    const size_t idx = by_residue.IndexOf(key);
    if (idx == InvalidIndex) {
      by_residue.Add(key, TSAtomPList()).Add(sa);
      order.Add(key);
    }
    else {
      by_residue.GetValue(idx).Add(sa);
    }
  }

  olxdict<uint64_t, TTypeList<BackboneResidue>, TPrimitiveComparator> by_chain;
  /* Residues the classifier could not place, kept per chain with their geometry
  so a second pass can try to recover a CA once the chain around them is known.
  */
  olxdict<uint64_t, TArrayList<size_t>, TPrimitiveComparator> unplaced;
  TTypeList<TTypeList<ProteinAtom> > res_atoms;
  TArrayList<size_t> res_id(order.Count());
  TArrayList<int> res_num(order.Count());

  for (size_t i = 0; i < order.Count(); i++) {
    const uint64_t key = order[i];
    const size_t resi_id = size_t(key >> 32);
    const uint32_t matrix_id = uint32_t(key & 0xffffffff);
    TSAtomPList &ra = by_residue.Get(key);

    /* The residue's own atoms, plus any bonded atom of another residue marked
    foreign. The peptide nitrogen is what distinguishes a backbone carbonyl
    from a sidechain C-OH, so it has to be visible to the classifier.
    */
    TTypeList<ProteinAtom> pa;
    for (size_t j = 0; j < ra.Count(); j++) {
      pa.AddNew(ra[j]->GetType().z, ra[j]->crd(), false);
    }
    for (size_t j = 0; j < ra.Count(); j++) {
      TSAtom &sa = *ra[j];
      for (size_t k = 0; k < sa.NodeCount(); k++) {
        TSAtom &nb = sa.Node(k);
        if (nb.IsDeleted() || nb.CAtom().GetResiId() == resi_id) {
          continue;
        }
        pa.AddNew(nb.GetType().z, nb.crd(), true);
      }
    }

    const TResidue &r = au.GetResidue(resi_id);
    const uint64_t ckey = (uint64_t(uint32_t(r.GetChainId())) << 32)
      | uint64_t(matrix_id);
    res_id[i] = resi_id;
    res_num[i] = r.GetNumber();
    res_atoms.AddCopy(pa);

    const BackboneRoles roles = FindBackbone(pa);
    if (!roles.IsValid()) {
      /* Not an amino acid - water, ion, ligand - or an amino acid whose
      geometry is too poor to place. The second pass tells those apart, using
      the chain that forms around them.
      */
      const size_t ui = unplaced.IndexOf(ckey);
      (ui == InvalidIndex ? unplaced.Add(ckey, TArrayList<size_t>())
        : unplaced.GetValue(ui)).Add(i);
      continue;
    }
    const size_t ci = by_chain.IndexOf(ckey);
    TTypeList<BackboneResidue> &lst = (ci == InvalidIndex)
      ? by_chain.Add(ckey, TTypeList<BackboneResidue>())
      : by_chain.GetValue(ci);

    BackboneResidue &br = lst.AddNew();
    br.number = res_num[i];
    br.resi_id = resi_id;
    br.ca = pa[roles.ca].crd;
    if (roles.o != InvalidIndex) {
      br.o = pa[roles.o].crd;
      br.has_o = true;
    }
    /* the roles index pa, whose first ra.Count() entries are this residue's
    own atoms in order and the rest the foreign neighbours. A backbone atom is
    never one of those, so an index past the range belongs to no atom here
    */
    struct id_of {
      static size_t get(const TSAtomPList &ra, size_t role) {
        return (role != InvalidIndex && role < ra.Count())
          ? ra[role]->CAtom().GetId() : InvalidIndex;
      }
    };
    br.n_id = id_of::get(ra, roles.n);
    br.ca_id = id_of::get(ra, roles.ca);
    br.c_id = id_of::get(ra, roles.c);
    br.o_id = id_of::get(ra, roles.o);
  }

  /* second pass: place the residues the classifier rejected from the CA of
  their traced neighbours. Only those in a numbered gap of a chain qualify,
  which keeps waters and ligands out. Recovered residues do not help each
  other, so two consecutive failures leave a hole for the bridging below
  */
  for (size_t i = 0; i < unplaced.Count(); i++) {
    const uint64_t ckey = unplaced.GetKey(i);
    const size_t ci = by_chain.IndexOf(ckey);
    if (ci == InvalidIndex) {
      continue;      // nothing of this chain was traced: not a polymer
    }
    TTypeList<BackboneResidue> &lst = by_chain.GetValue(ci);
    QuickSorter::SortSF(lst, &ResidueCmp);
    const TArrayList<size_t> &cands = unplaced.GetValue(i);
    TTypeList<BackboneResidue> recovered;
    for (size_t j = 0; j < cands.Count(); j++) {
      const size_t ri = cands[j];
      const int num = res_num[ri];
      const vec3d *prev = 0, *next = 0;
      for (size_t k = 0; k < lst.Count(); k++) {
        if (lst[k].number < num && num - lst[k].number <= 1) {
          prev = &lst[k].ca;
        }
        else if (lst[k].number > num && lst[k].number - num <= 1) {
          next = &lst[k].ca;
          break;
        }
      }
      const size_t ca = RecoverCA(res_atoms[ri], prev, next);
      if (ca == InvalidIndex) {
        continue;
      }
      BackboneResidue &br = recovered.AddNew();
      br.number = num;
      br.resi_id = res_id[ri];
      br.ca = res_atoms[ri][ca].crd;
    }
    for (size_t j = 0; j < recovered.Count(); j++) {
      lst.AddCopy(recovered[j]);
    }
  }

  // order by residue number, then split wherever the backbone is broken
  for (size_t i = 0; i < by_chain.Count(); i++) {
    TTypeList<BackboneResidue> &lst = by_chain.GetValue(i);
    if (lst.Count() < 2) {
      continue;
    }
    QuickSorter::SortSF(lst, &ResidueCmp);
    const olxch chain = olxch(uint32_t(by_chain.GetKey(i) >> 32));
    const uint32_t mid = uint32_t(by_chain.GetKey(i) & 0xffffffff);

    ChainSegment *seg = 0;
    for (size_t j = 0; j < lst.Count(); j++) {
      if (seg != 0 && !seg->residues.IsEmpty()) {
        if (!IsContinuous(seg->residues.GetLast(), lst[j], break_distance)) {
          seg = 0;
        }
      }
      if (seg == 0) {
        seg = &out.AddNew();
        seg->chain_id = chain;
        seg->matrix_id = mid;
      }
      seg->residues.AddCopy(lst[j]);
    }
  }

  // a single residue cannot be splined
  for (size_t i = out.Count(); i > 0; i--) {
    if (out[i-1].residues.Count() < 2) {
      out.Delete(i-1);
    }
  }
}
//.............................................................................

} // namespace protein

EndXlibNamespace()
