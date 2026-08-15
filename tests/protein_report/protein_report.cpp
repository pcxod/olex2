/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

/* What the cartoon makes of a structure, without opening the GUI.

Runs the shipped topology and secondary-structure code over a file and prints
the chain segments it found and the assignment along each one. That is the only
way to check the assignment against a known fold - ubiquitin's five-stranded
sheet, crambin's two - as data rather than by eye, and it is how a regression in
either would be noticed.

It runs the real functions from xlib/protein.h, not a copy, so what it reports
is what the ribbon will draw.

  cl /O2 /EHsc /I..\..\sdl /I..\..\xlib protein_report.cpp
    sdl.lib xlib-np.lib Iphlpapi.lib

  protein_report <file.ins|.res> [more files...]
*/

#include "xapp.h"
#include "ins.h"
#include "outstream.h"
#include "protein.h"
#include "residue.h"

#include <stdio.h>

//.............................................................................
static void Report(TXApp &xapp, const olxstr &fn) {
  printf("\n%s\n", olxcstr(TEFile::ExtractFileName(fn)).c_str());
  /* Timed in two halves on purpose. The question this answers is whether a
  large structure is slow to open because of the display or because of the
  model, and only splitting it can say.
  */
  const uint64_t t0 = TETime::msNow();
  xapp.XFile().LoadFromFile(fn);
  const uint64_t t1 = TETime::msNow();
  TLattice &latt = xapp.XFile().GetLattice();
  const TAsymmUnit &au = xapp.XFile().GetAsymmUnit();

  TSAtomPList atoms;
  atoms.SetCapacity(latt.GetObjects().atoms.Count());
  for (size_t i = 0; i < latt.GetObjects().atoms.Count(); i++) {
    TSAtom &a = latt.GetObjects().atoms[i];
    if (!a.IsDeleted()) {
      atoms.Add(a);
    }
  }

  const uint64_t t2 = TETime::msNow();
  TTypeList<xlib::protein::ChainSegment> segments;
  xlib::protein::ExtractSegments(au, atoms, segments);
  const uint64_t t3 = TETime::msNow();

  printf("  %u atoms, %u residues, %u chain segment(s)\n",
    (unsigned)atoms.Count(), (unsigned)au.ResidueCount(),
    (unsigned)segments.Count());
  printf("  read and lattice %.1f s, topology %.1f s\n",
    (t1 - t0)/1000.0, (t3 - t2)/1000.0);

  size_t total = 0, n_h = 0, n_s = 0;
  for (size_t i = 0; i < segments.Count(); i++) {
    const xlib::protein::ChainSegment &seg = segments[i];
    TArrayList<short> ss;
    xlib::protein::AssignSecondaryStructure(seg, ss);
    printf("  chain %c  %u residues, %d to %d\n",
      (char)seg.chain_id, (unsigned)seg.residues.Count(),
      seg.residues[0].number, seg.residues.GetLast().number);
    /* H helix, E strand, dash coil: the one-letter alphabet every viewer of a
    DSSP output already reads, so the string can be compared by eye with a
    deposited assignment.
    */
    olxcstr line;
    for (size_t j = 0; j < ss.Count(); j++) {
      line << (ss[j] == xlib::protein::ss_helix ? 'H'
        : (ss[j] == xlib::protein::ss_strand ? 'E' : '-'));
      if (ss[j] == xlib::protein::ss_helix) { n_h++; }
      else if (ss[j] == xlib::protein::ss_strand) { n_s++; }
    }
    total += ss.Count();
    for (size_t j = 0; j < line.Length(); j += 60) {
      printf("    %4u %s\n", (unsigned)(seg.residues[j].number),
        olxcstr(line.SubStringFrom(j,
          (j + 60 < line.Length()) ? line.Length() - j - 60 : 0)).c_str());
    }
    // strand runs, which is what a sheet shows up as
    size_t runs = 0;
    for (size_t j = 0; j < ss.Count(); j++) {
      if (ss[j] == xlib::protein::ss_strand &&
        (j == 0 || ss[j-1] != xlib::protein::ss_strand))
      {
        runs++;
      }
    }
    if (runs != 0) {
      printf("    %u strand run(s)\n", (unsigned)runs);
    }
  }
  if (total != 0) {
    printf("  traced %u residues: %u%% helix, %u%% strand, %u%% coil\n",
      (unsigned)total, (unsigned)(100*n_h/total), (unsigned)(100*n_s/total),
      (unsigned)(100*(total - n_h - n_s)/total));
  }
}
//.............................................................................
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: protein_report <file.ins|.res> [more files...]\n");
    return 1;
  }
  TXApp xapp(TXApp::GuessBaseDir(argv[0]));
  xapp.XFile().RegisterFileFormat(new TIns, "ins");
  xapp.XFile().RegisterFileFormat(new TIns, "res");
  xapp.GetLog().AddStream(new TOutStream, true);
  for (int i = 1; i < argc; i++) {
    try {
      Report(xapp, argv[i]);
    }
    catch (const TExceptionBase &e) {
      printf("  FAILED: %s\n", olxcstr(e.GetException()->GetError()).c_str());
    }
  }
  return 0;
}
