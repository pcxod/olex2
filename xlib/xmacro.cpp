/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xmacro.h"
#include "xapp.h"
#include "p4p.h"
#include "mol.h"
#include "crs.h"
#include "ins.h"
#include "cif.h"
#include "hkl.h"
#include "unitcell.h"
#include "symmtest.h"
#include "integration.h"
#include "utf8file.h"
#include "datafile.h"
#include "dataitem.h"
#include "fsext.h"
#include "xlcongen.h"
#include "bitarray.h"
#include "olxvar.h"
#include "strmask.h"
#include "sgset.h"
#include "sfutil.h"
#include "infotab.h"
#include "idistribution.h"
#include "ipattern.h"
#include "chnexp.h"
#include "maputil.h"
#include "vcov.h"
#include "esphere.h"
#include "symmcon.h"
#include "md5.h"
#include "absorpc.h"
#include "twinning.h"
#include "refutil.h"
#include "analysis.h"
#include "tls.h"
#include "math/plane.h"

#ifdef _CUSTOM_BUILD_
  #include "custom_base.h"
#endif

#ifdef _SVN_REVISION_AVAILABLE
#  include "../svn_revision.h"
#endif

#define xlib_InitMacro(macroName, validOptions, argc, desc)\
  lib.Register( new TStaticMacro(&XLibMacros::mac##macroName, #macroName, (validOptions), argc, desc))
#define xlib_InitMacroA(macroName, amacroName, validOptions, argc, desc)\
  lib.Register( new TStaticMacro(&XLibMacros::mac##macroName, #amacroName, (validOptions), argc, desc))
#define xlib_InitFunc(funcName, argc, desc) \
  lib.Register(new TStaticFunction(&XLibMacros::fun##funcName, #funcName, argc, desc))

using namespace cif_dp;
void HKLCreate(TStrObjList &Cmds, const TParamList &Options, TMacroData &E);

void XLibMacros::Export(TLibrary& lib)  {
  xlib_InitMacro(Run, EmptyString(), fpAny^fpNone,
    "Runs provided macros (combined by '>>')");
  xlib_InitMacro(HklStat,
    "l-list the reflections&;m-merge reflection in current space group",
    fpAny|psFileLoaded,
    "If no arguments provided, prints the statistics on all reflections as "
    "well as the ones used in the refinement. If an expressions (condition) "
    "is given in the following form: x[ahbkcl], meaning that x=ah+bk+cl for x"
    " equals 0, ((ah+bk+cl) mod x) equals 0 for positove x and not equals 0 "
    "for negative x; the subsequent expressions are combined using logical "
    "'and' operator. For instance 2[l] expression means: to find all "
    "reflections where l is even, the expression -2[l] means to find all "
    "reflections with odd l, 0[h-l] - reflections where h equals to l etc. The"
    " function operates on all P1 merged reflections after filtering by SHEL "
    "and OMIT, -m option merges the reflections in current space group"
 );
  xlib_InitMacro(HklBrush, "f-consider Friedel law", fpAny, "for high redundancy\
 data sets, removes equivalents with high sigma");
//_____________________________________________________________________________
  xlib_InitMacro(SG, "a", fpNone|fpOne, "suggest space group");
  xlib_InitMacro(SGE,
    "f-use the current loader space group",
    fpNone|fpOne|psFileLoaded,
    "Extended spacegroup determination. Internal use");
//_____________________________________________________________________________
  xlib_InitMacro(GraphSR, "b-number of bins", fpNone|fpOne|psFileLoaded,
    "Prints a scale vs resolution graph for current file (fcf file must exist "
    "in current folder)");
  xlib_InitMacro(GraphPD,
    "r-resolution in degrees [0.5]&;fcf-take structure factors from the FCF "
    "file, otherwise calculate from current model&;s-use simple scale when "
    "calculating structure factors from the mode, otherwise regression scaling"
    " will be used",
    fpNone|psFileLoaded,
    "Prints a intensity vs. 2 theta graph");
  xlib_InitMacro(Wilson,
    "b-number of bins&;p-uses linear bins for picture, otherwise uses spherical"
    " bins",
    fpNone|fpOne|psFileLoaded,
    "Prints Wilson plot data");
//_____________________________________________________________________________
  xlib_InitMacro(TestSymm, "e-tolerance limit", fpNone|psFileLoaded,
    "Tests current structure for missing symmetry");
//_____________________________________________________________________________
  xlib_InitMacro(VATA, EmptyString(), fpAny|psFileLoaded,
    "Compares current model with the cif file and write the report to provided"
    " file (appending)");
  xlib_InitMacro(Clean,
    "npd-promotes at maximum given number of atoms a call [0]&;"
    "f-does not run 'fuse' after the completion&;"
    "aq-disables analysis of the Q-peaks based on thresholds&;"
    "at-disables lonely atom types assignment to O and Cl&;"
    "d-before 'blown up' atoms, a possibility to demote will be checked"
    ,
    fpNone,
    "Tidies up current model");
//_____________________________________________________________________________
  xlib_InitMacro(AtomInfo, EmptyString(), fpAny|psFileLoaded,
"Searches information for given atoms in the database");
//_____________________________________________________________________________
  xlib_InitMacro(Compaq,
    "a-assembles broken fragments&;c-similar as with no options, but considers "
    "atom-to-atom distances&;q-moves Q-peaks to the atoms, atoms are not "
    "affected&;m-assembles non-metallic parts of the structure first, then "
    "moves metals to the closest atom",
    fpNone|psFileLoaded,
    "Moves all atoms or fragments of the asymmetric unit as close to each "
    "other as possible. If no options provided, all fragments are assembled "
    "around the largest one.");
//_____________________________________________________________________________
  xlib_InitMacro(Envi,
    "q-adds Q-peaks to the list&;h-adds hydrogen atoms to the list&;"
    "cs-leaves selection unchanged&;"
    "p-print out precision [2], the angle printing precision will be half of"
    " that for the distances&;"
    "c-prints just the connectivity information",
    fpNone|fpOne|fpTwo,
    "This macro prints environment of any particular atom. Default search "
    "radius is 2.7A.");
//_____________________________________________________________________________
  xlib_InitMacro(AddSE, EmptyString(), (fpAny^fpNone)|psFileLoaded,
    "Tries to add a new symmetry element to current space group to form a new "
    "one. [-1] is for center of symmetry");
//_____________________________________________________________________________
  xlib_InitMacro(Fuse, EmptyString(),
    fpNone|fpOne|psFileLoaded,
    "Re-initialises the connectivity list. If a number is provided, atoms of "
    "the same type connected by bonds shorter than the provided number are "
    "merged into one atom with center at the centroid formed by all removed "
    "atoms");
  xlib_InitMacro(Flush, EmptyString(), fpNone|fpOne, "Flushes log streams");
//_____________________________________________________________________________
  xlib_InitMacro(EXYZ,
    "eadp-does not set the equivalent ADP constraint for the shared site",
    fpAny|psCheckFileTypeIns,
    "Adds a new element to the given/selected site. Takes one selected atom "
    "and element types as any subsequent argument. Alternatively can take a "
    "few selected atoms of different type to be modelled as the type swapping "
    "disorder or a set of atoms of the same type and new element type on the "
    "command line."
);
//_____________________________________________________________________________
  xlib_InitMacro(EADP, EmptyString(), fpAny|psCheckFileTypeIns,
"Forces EADP/Uiso of provided atoms to be constrained the same");
//_____________________________________________________________________________
  xlib_InitMacro(Cif2Doc, "n-output file name", fpNone|fpOne|psFileLoaded,
    "converts cif to a document");
  xlib_InitMacro(Cif2Tab,
    "n-output file name&;"
    "t-table definition file&;"
    "l-label option flag: 0 - as is, 1 - round brackets, 2 - subscript, "
    "4 - superscript, this can be combined",
    fpAny|psFileLoaded,
    "creates a table from a cif");
  xlib_InitMacro(CifMerge,
    "u-updates atom treatment if the asymmetric units of currently loaded file"
    " and of the CIF file match&;"
    "f-creates final CIF with embedded RES file and HKL loop&;"
    "dn-new data name for the merged CIF&;"
    "resolve-allows using skip_merged items to resolve empty items&;"
    "fcf-[false] - use _refln loop vs _diffrn_refln when embedding HKL",
    fpAny|psFileLoaded,
  "Merges loaded or provided as first argument cif with other cif(s)");
  xlib_InitMacro(CifExtract,
    "i-a custom CIF with items to extract, like 'Basedir()/etc/CIF/extract.cif'"
    "; by default [metacif] the 'Basedir()/etc/CIF/customisation.xld'#"
    "cif_customisation#export_metacif item is used as the extract template",
    fpNone|fpOne|fpTwo|psFileLoaded,
    "Extract a list of items from one cif to another. If no argument is given "
    "the command creates, if does not exist, 'StrDir()/FileName().metacif' "
    "file using default template file. If arguments are given the first one is"
    " used as the input CIF, the second, which defaults to the metacif file - "
    "as output.");
  xlib_InitMacro(CifCreate, EmptyString(), fpNone|psFileLoaded,
    "Creates cif from current file, variance-covariance matrix should be "
    "available");
  xlib_InitMacro(FcfCreate,
    "scale-[external],simple, regression or none&;"
    "c-[false] converts current fcf to the given list format",
    (fpAny^fpNone)|psFileLoaded,
    "Creates fcf from current file. Expects a number as in the shelx list "
    "number as the first argument, the second argument is the output file name"
    " filename().fcf is default"
    );
//_____________________________________________________________________________
  xlib_InitMacro(VoidE, EmptyString(), fpNone|psFileLoaded,
    "calculates number of electrons in the voids area");
//_____________________________________________________________________________
  xlib_InitMacro(ChangeSG,
    "c-apply cell change according to the centering change (experimental!)",
    (fpAny^fpNone)|psFileLoaded,
    "[shift] SG. Changes space group of current structure, applying given shift"
    " prior (if provided) to the change of symmetry of the unit cell");
//_____________________________________________________________________________
  xlib_InitMacro(Htab,
    "t-adds extra elements (comma separated -t=Se,I) to the donor list. "
    "Defaults are [N,O,F,Cl,S,Br]&;"
    "g-generates found interactions&;"
    "c-add carbon in the atom list",
    fpNone|fpOne|fpTwo|psFileLoaded,
    "Adds HTAB instructions to the ins file, maximum bond length [2.9] and "
    "minimal angle [150] might be provided. If the default length is changed"
    " 'fuse' may be required to show the newly available bonds");
//_____________________________________________________________________________
  xlib_InitMacro(HAdd,
    "r-use restraints vs constraints for water molecules [False]&;"
    "nr3-disable H placement for NR3 [False]&;"
    "p-put added H atoms to given part (sometimes needed when add/del bonds) "
    "are used in conjunction with parts&;"
    "a-changes AFIX to the given value (like 3 is needed sometimes for complex)"
    " connectivity",
    fpAny,
    "Adds hydrogen atoms to all or provided atoms, however the ring atoms are "
    "treated separately and added all the time");
  xlib_InitMacro(HImp, EmptyString(), (fpAny^fpNone)|psFileLoaded,
    "Increases, decreases length of H-bonds. Arguments: value [H atoms]. Value"
    " might be +/- to specify to increase/decrease current value");
//_____________________________________________________________________________
  xlib_InitMacro(FixUnit, EmptyString(), fpNone|fpOne|psFileLoaded,
    "Sets SFAc and UNIT to current content of the asymmetric unit. Takes Z', "
    "with default value of 1.");
  xlib_InitMacro(GenDisp,
    "f-generates full SFAC instructions&;"
    "n-for neutron data&;"
    "force-forces rewriting SFAC for existing items[false]&;"
    "source-use cctbx's Sasaki or Henke tables as source",
    fpNone|fpOne|psFileLoaded,
    "Generates anisotropic dispertion parameters for current radiation "
    "wavelength");
//_____________________________________________________________________________
  xlib_InitMacro(AddIns,
    "q-quiet if has not added",
    (fpAny^fpNone)|psCheckFileTypeIns,
    "Adds an instruction to the INS file");
  xlib_InitMacro(DelIns, EmptyString(), fpOne|psCheckFileTypeIns,
    "A number or the name (will remove all accurances) can be provided");
  xlib_InitMacro(LstIns, EmptyString(), fpNone|psCheckFileTypeIns,
    "Lists all instructions of currently loaded Ins file");
  xlib_InitMacro(FixHL, EmptyString(), fpNone|psFileLoaded,
    "Fixes hydrogen atom labels");
  xlib_InitMacro(Fix, EmptyString(), (fpAny^fpNone)|psCheckFileTypeIns,
    "Fixes specified parameters of atoms: XYZ, Uiso, Occu");
  xlib_InitMacro(Free, EmptyString(), (fpAny^fpNone)|psFileLoaded,
    "Frees specified parameters of atoms: XYZ, Uiso, Occu");
  xlib_InitMacro(Isot, "npd-makes all NPD atoms isotropic",
    fpAny|psFileLoaded,
    "Makes provided atoms isotropic, if no arguments provided, current "
    "selection or all atoms become isotropic");
  xlib_InitMacro(Anis,
    "h-adds hydrogen atoms&;"
    "a-also adds anharmonic part",
    (fpAny) | psFileLoaded,
    "Makes provided atoms anisotropic if no arguments provided current "
    "selection or all atoms are considered");
  xlib_InitMacro(File,
    "s-sort the main residue of the asymmetric unit&;"
    "p-coordinate precision for files supporting this option&;"
    "a-save asymmetric unit only ",
    fpNone|fpOne|psFileLoaded,
    "Saves current model to a file. By default an ins file is saved and "
    "loaded");
  xlib_InitMacro(LS, EmptyString(), fpOne|fpTwo|psCheckFileTypeIns,
    "Sets refinement method and/or the number of iterations.");
  xlib_InitMacro(Plan, EmptyString(), fpOne|psCheckFileTypeIns,
    "Sets the number of Fourier peaks to be found from the difference map");
  xlib_InitMacro(UpdateWght, EmptyString(), fpAny|psCheckFileTypeIns,
    "Copies proposed weight to current");
  xlib_InitMacro(User, EmptyString(), fpNone|fpOne, "Changes current folder");
  xlib_InitMacro(Dir, EmptyString(), fpNone|fpOne,
    "Lists current folder. A file name mask may be provided");
  xlib_InitMacro(LstVar, EmptyString(), fpAny,
    "Lists all defined variables. Accepts * based masks");
  xlib_InitMacro(LstMac, "h-Shows help", fpAny,
    "Lists all defined macros. Accepts * based masks");
  xlib_InitMacro(LstFun, "h-Shows help", fpAny,
    "Lists all defined functions. Accepts * based masks");
  xlib_InitMacro(LstFS, EmptyString(), fpAny,
    "Prints out detailed content of virtual file system. Accepts * based "
    "masks");
  xlib_InitMacro(SGS, EmptyString(), (fpAny^fpNone)|psFileLoaded,
    "Changes current space group settings using provided cell setting (if "
    "applicable) and axis, or 9 transformation matrix elements and the space "
    "group symbol. If the transformed HKL file is required, it should be "
    "provided as the last argument (like test.hkl)");
  xlib_InitMacro(ASR, EmptyString(), fpNone^psFileLoaded,
    "Absolute structure refinement: adds TWIN and BASF to current model in the"
    " case of non-centrosymmetric structure");
  xlib_InitMacro(Describe, EmptyString(), fpNone^psFileLoaded,
    "Describes current refinement in a human readable form");
  xlib_InitMacro(Sort,
    "r-sort for atoms inside residues [true]&;"
    "rn-sort residues by number [true]&;"
    ,
    fpAny^psFileLoaded,
  "Sorts atoms of the default residue. Atom sort arguments: "
  "\n\tm - atomic mass"
  "\n\tz - atomic number"
  "\n\tl - label, considering numbers"
  "\n\tp - part, 0 is first followed by all positive parts in ascending order "
  "and then negative ones"
  "\n\th - to treat hydrogen atoms independent of the pivot atom"
  "\n\ts - non-numerical labels suffix"
  "\n\tn - number after the atom symbol"
  "\n\tx - atom moiety size"
  "\n\tf - when a list of atoms provided the atoms will be ordered after the "
  "first given name - otherwise at the position of the firstly found atom in "
  "current list"
  "\n\tw - rather than ordering atoms by given sequence this will swap their "
  "positions"
  "\n Moiety sort arguments:"
  "\n\tl - label"
  "\n\ts - size"
  "\n\th - by heaviest atom"
  "\n\tm - molecular mass"
  "\nUsage: sort [+atom_sort_type] or [Atoms] [moiety [+moety sort type] "
  "[moiety atoms]]. If just 'moiety' is provided - the atoms will be split "
  "into the moieties without sorting."
  "\nExample: sort +ml F2 F1 moiety +s - will sort atoms by atomic mass and "
  "label, put F1 after F2 and form moieties sorted by size. Note that when "
  "sorting atoms, any subsequent sort type operates inside the groups created "
  "by the preceeding sort types."
 );
  xlib_InitMacro(SGInfo,
    "c-include lattice centering matrices&;i-include inversion generated "
    "matrices if any",
    fpNone|fpOne,
    "Prints space group information.");
  xlib_InitMacro(SAInfo, EmptyString(), fpAny,
    "Finds and prints space groups which include any of the provided "
    "systematic absences in the form 'b~~', '~b~' or '~~b'");
  xlib_InitMacro(Inv, "f-force inversion for non-centrosymmetric space groups",
    fpAny|psFileLoaded,
    "Inverts whole structure or provided fragments of the structure");
  xlib_InitMacro(Push, EmptyString(),
    (fpAny^(fpNone|fpOne|fpTwo))|psFileLoaded,
    "Shifts the sctructure (or provided fragments) by the provided "
    "translation");
  xlib_InitMacro(Transform, EmptyString(), fpAny|psFileLoaded,
    "Transforms the structure or provided fragments according to the given "
    "matrix (a11, a12, a13, a21, a22, a23, a31, a32, a33, t1, t2, t3)");
  xlib_InitMacro(Standardise, EmptyString(), fpNone|fpOne|psFileLoaded,
    "Standardises atom coordinates (similar to HKL standardisation procedure)."
    " If '0' is provided as argument, the asymmetric unit content is arranged "
    "as close to (0,0,0), while being inside the unit cell as possible");
  xlib_InitMacro(FitCHN, EmptyString(), (fpAny^(fpNone|fpOne)),
    "Fits CHN analysis for given formula and observed data given a lits of "
    "possible solvents. A mixture of up to 3 solvents only considered, however"
    " any number of observed elements can be provided. Example: FitCHN "
    "C12H22O11 C:40.1 H:6 N:0 H2O CCl3H");
  xlib_InitMacro(CalcCHN, EmptyString(), fpNone|fpOne,
    "Calculates CHN composition of current structure or for provided formula");
  xlib_InitMacro(CalcMass, EmptyString(), fpNone|fpOne,
    "Calculates Mass spectrum of current structure or for provided formula");
  xlib_InitMacro(Omit,
    "u-puts the omitted reflection back (only for OMIT lines)",
    fpOne|fpTwo|fpThree|psCheckFileTypeIns,
    "Removes any particular reflection from the refinement list. If a single "
    "number is provided, all reflections with delta(F^2)/esd greater than "
    "given number are omitted");
  xlib_InitMacro(Shel,
    EmptyString(),
    fpTwo | psCheckFileTypeIns,
    "Adds SHEL command to trim reflections");
  xlib_InitMacro(Reset, "s-space group&;c-content&;f-alternative file name&;"
    "rem-exclude remarks&;atoms-saves the atom list alongside",
    fpAny|psFileLoaded,
    "Resets current structure for the solution with ShelX");
  xlib_InitMacro(Degen, "cs-clear selection", fpAny|psFileLoaded,
    "Prints how many symmetry operators put given atom to the same site");
  xlib_InitMacroA(Close, @Close, EmptyString(), fpNone|psFileLoaded,
    "Closes currently loaded file");
  xlib_InitMacro(PiPi, "g-generates using found symmetry operations"
    "&;r-ring content [C6,NC5]",
    fpNone|fpTwo|psFileLoaded,
    "Analysis of the pi-pi interactions. The procedure searches"
    " for flat reqular C6 or NC5 rings and prints information for the ones "
    "where the centroid-centroid distance is smaller than [4] A and the shift "
    "is smaller than [3] A. These two parameters can be customised.");
  xlib_InitMacro(PiSig, "g-generates using found symmetry operations"
    "&;r-ring content [C6,NC5]",
    fpNone | fpTwo | psFileLoaded,
    "Analysis of the pi-sigma interactions (experimental). The procedure searches"
    " for flat reqular 4 and 6 membored rings. Then it searches for atoms within "
    "[4] A from the ring centre and with less than [25] angle between the plane "
    "normal and the (plane center-atom) vectors.");
  xlib_InitMacro(MolInfo,
    "g-generation of the triangluation [5]&;"
    "s-source ([o]ctahedron, (t)etrahedron) &;"
    "o-use occupancy of the atoms in the integration",
    fpAny|psFileLoaded,
    "Prints molecular volume, surface area and other information for "
    "visible/selected atoms");
  xlib_InitMacro(RTab, EmptyString(), fpAny^(fpNone)|psCheckFileTypeIns,
    "Adds RTAB with given name (first argument) for provided atoms/selection");
  xlib_InitMacro(HklMerge,
    "m-merger [shelx], standard, unit&;"
    "z-zero negative intensity",
    fpAny|psFileLoaded,
    "Merges current HKL file (ehco HKLSrc()) to given file name. "
    "Warning: if no arguments provided, the current file is overwritten");
  xlib_InitMacro(HklAppend, "h&;k&;l&;c", fpAny,
    "moves reflection back into the refinement list. See excludeHkl for more "
    "details");
  xlib_InitMacro(HklExclude,
    "h-semicolon separated list of indexes&;k&;l&;c-true/false to use provided"
    " indexes in any reflection. The default is in any one reflection",
    fpAny,
    "Excludes reflections with give indexes from the hkl file -h=1;2 : all "
    "reflections where h=1 or 2");
  xlib_InitMacro(HklImport,
    "batch-for separator formatted file specifies that there is a batch "
    "number",
    (fpAny^(fpNone|fpOne|fpTwo|fpThree)),
    "Creates a Shelx compatible 44488(4) file format from given source. Valid"
    " arguments: fixed and separator. For example:"
    "\n\t'HklImport in.hkl fixed 7 7 7 9 9 out.hkl' or 'HklImport in.hkl "
    "separator \' \'' out.hkl"
   );
  xlib_InitMacro(HklSplit,
    "b-creates an HKLF 5 file (*_h5) with batches 1 and -2; only applicable "
    "when second parameter is 'a'",
    (fpTwo),
    "Split an HKL file according to the Fc^2/esd or the value of |Fc^2-Fo^2|/esd"
    ". The threshold value is the first argument. If it ends with '%' - the "
    "pecentage of the merged reflections is taken into the account. The second"
    " argument is the splitting criterion - 'i'  for intensity or 'a' for the "
    "agreeability. Unless -b option is provided, the 'agreeable' reflections "
    "end up in the *_a.hkl file,  'disagreeable' - in the *_d.hkl file; weaker"
    " reflections end up in *_w.hkl and stronger reflections - in the *_s.hkl "
    "files."
   );
  lib.Register(new TStaticMacro(
    &HKLCreate, "HKLCreate", EmptyString(), fpThree,
    "Creates calculated HKL file with given name followed by wavelength "
    "and resolution")
  );
  xlib_InitMacroA(Update, @update, EmptyString(), fpAny,
    "Reads given file and if the atoms list of loaded file matches the atom "
    "list of the given file the atomic coordinates, FVAR and BASF values are "
    "updated.");
  xlib_InitMacro(Move,
    "cs-leaves selection unchanged&;"
    "c-copy moved atom",
    fpNone|fpTwo,
    "Moves two atoms as close to each other as possible; if no atoms given, "
    "moves all fragments as close to the cell center as possible and makes "
    "sure that the center of fragments gravity stays inside the cell.");

  xlib_InitMacro(Fvar, EmptyString(), fpAny|psCheckFileTypeIns,
    "Assigns/release occupancy for given atoms. Examples:"
    "\n-'fvar' if nothing is selected will print current values of the "
    "variables. For a selection of even number atoms, will create a new "
    "variable and link occupancies of the first half of the selection to "
    "occupancy the other half of the selection. "
    "\n-'fvar 0' - makes occupancy of provided atoms refineable"
    "\n-'fvar 1' - fixes occupancy of provided atoms at current value"
    "\n-'fvar 1 1' - fixes occupancy of provided atoms at chemical occupancy "
    "of 1"
    "\n-'fvar 2' will link occupancy of the given atoms to the value of the "
    "2nd FVAR multiplied by current value of the occupancy of the given atoms,"
    " or, if occupancy already linked to a variable - it will replace the "
    "variable index."
    "\n-'fvar 2 0.5' will link occupancy of the given atoms to the value of "
    "the 2nd FVAR multiplied by 0.5.");
  xlib_InitMacro(Sump,
    EmptyString(),
    (fpAny^fpNone)|psCheckFileTypeIns,
    "Adds a linear equaltion into the refinement");
  xlib_InitMacro(Part,
    "p-number of parts&;lo-link ocupancy of given atoms through FVAR's&;"
    "c-creates a copy of all grown atoms to which applied in the asymmetric unit and "
    "automatically links occupancies with the original atoms&;"
    "f-do not 'fuse' the structure to be used with -c option [false]",
    fpAny|psFileLoaded,
    "Sets part(s) to given atoms, also if -lo is given and -p > 1 allows linking "
    "occupancy of given atoms through FVAR and/or SUMP in cases when -p > 2");
  xlib_InitMacro(Spec, EmptyString(),
    fpAny | psFileLoaded,
    "Sets SPEC (special position eforcing) command for given atoms with default"
    " deviation from the special position 0.2 A");
  xlib_InitMacro(Afix,
    "n-to accept N atoms in the rings for afix 66&;"
    "s-sorts atoms for 5 and 6 membered rings [true]&;"
    "c-changes the afix (if any) for the given atoms&;"
    ,
    (fpAny^fpNone)|psCheckFileTypeIns,
    "Sets atoms AFIX, special cases are 56,69,66,69,76,79,106,109,116 and "
    "119");
  xlib_InitMacro(Dfix,
    "i-[false] places implicit restraint&;"
    "cs-do not clear selection",
    fpAny,
    "Restrains distancesto the given value");
  xlib_InitMacro(Tria,
    "i-[false] places implicit restraint&;"
    "cs-do not clear selection",
    fpAny,
    "Adds a distance restraint for bonds and 'angle' restraint for the angle."
    "Takes bond pairs or atom triplets.");
  xlib_InitMacro(Dang,
    "i-[false] places implicit restraint&;"
    "cs-do not clear selection",
    fpAny|psCheckFileTypeIns,
    "Adds a ShelX compatible angle restraint");
  xlib_InitMacro(Sadi,
    "i-[false] makes an implicit restaraint&;"
    "r-[true] when two atoms selected creates a rotor restraint, can be false"
    " when -i is used&;"
    "cs-do not clear selection",
    fpAny|psCheckFileTypeIns,
    "Similar distances restraint");
  xlib_InitMacro(RRings,
    "cs-do not clear selection" ,
    fpAny,
    "Makes all provided rings [like C6 or NC5] regular (flat and all distances"
    " similar). If a selection is given - the whole rings must be selected");
  xlib_InitMacro(Flat,
    "cs-do not clear selection&;"
    "i-place implicit restraint"
    ,
    fpAny|psCheckFileTypeIns,
    "Flat group restraint for at least 4 provided atoms");
  xlib_InitMacro(Chiv,
    "i-[false] places implicit restraint&;"
    "cs- do not clear selection",
    fpAny|psCheckFileTypeIns,
    "Restrains chiral volume of atom(s) to '0' or provided value");
  xlib_InitMacro(DELU,
    "i-[false] places implicit restraint&;"
    "cs-do not clear selection",
    fpAny|psCheckFileTypeIns,
    "Rigid bond constraint. If no atoms provided, all non-H atoms considered");
  xlib_InitMacro(SIMU,
    "i-[false] places implicit restraint&;"
    "cs-do not clear selection",
    fpAny|psCheckFileTypeIns,
    "Similarity restraint for Uij of provided atoms. If no atoms provided, "
    "all non-H atoms considered");
  xlib_InitMacro(ISOR,
    "i-[false] places implicit restraint&;"
    "cs-do not clear selection",
    fpAny|psCheckFileTypeIns,
    "Forses Uij of provided atoms to behave in isotropic manner. If no atoms "
    "provided, all non-H atoms considered");
  xlib_InitMacro(Delta,
    EmptyString(),
    fpNone|fpOne,
    "Prints/sets current delta fir the covalent bonds");
  xlib_InitMacro(DeltaI,
    EmptyString(),
    fpNone|fpOne,
    "Prints/sets current delta for short interactions");
  xlib_InitMacroA(Export, @Export,
    EmptyString(),
    fpNone|fpOne|psCheckFileTypeCif,
    "Exports reflections file and RES if present in the loaded CIF");
  xlib_InitMacro(AddBond,
    EmptyString(),
    fpAny,
    "Adds specified bond to the connectivity table");
  xlib_InitMacro(DelBond,
    EmptyString(),
    fpAny,
    "Removes specified bond from the connectivity table");
  xlib_InitMacro(Sgen,
    "m-move the atoms instead of generating copies&;",
    (fpAny^fpNone)|psFileLoaded,
    "Grows the structure using provided atoms (all if none provided) and "
    "symmetry code");
  xlib_InitMacro(LstSymm,
    EmptyString(),
    fpNone|psFileLoaded,
    "Prints symmetry codes for current structure");
  xlib_InitMacro(Same,
    "i-if one atom is given - generates an implicit restraint for the atom's "
    "residue class, for 2 atoms - invert the graphs when doing automatic "
    "matching&;"
    "e-expand SAME into the list of SADI&;"
    "s-generates 'self' same restrain so that the fragment matches itself in "
    "reverse order&;"
    "all-find fragments matching the selection and applies SAME to them",
    fpAny|psFileLoaded,
    "Creates SAME instruction for two fragments (two selected atoms or two "
    "atoms provided) or number_of_groups and groups following each another "
    "(or selection)");
  xlib_InitMacro(RIGU,
    "i-[false] places implicit restraint&;"
    ,
    fpAny|psFileLoaded,
    "Creates rigid bond (RIGU) restraint for a group of provided atoms"
    "(or selection)");
  xlib_InitMacro(RESI,
    "a-alias&;"
    "all-searches selected subfragment and creates residues",
    (fpAny^fpNone)|psFileLoaded,
    "Creates residue with given class name and optionally number and adds "
    "selected or provided atoms into the residue. If provided residue class "
    "name is 'none', provided atoms are removed from their current residues");
  xlib_InitMacro(Restrain,
    EmptyString(),
    (fpAny^fpNone)|psFileLoaded,
    "Creates a restraint");
  xlib_InitMacro(Constrain,
    EmptyString(),
    (fpAny^fpNone)|psFileLoaded,
    "Creates a constraint");
  xlib_InitMacro(Conn,
    EmptyString(),
    fpAny^fpNone,
    "Changes provided atom(s) connectivity (only until next connectivity "
    "modifying operation for now)."
    "\nUsage: conn max_bond bonding_radius [selection/atom(s)/$type]"
    "\nUsage: conn max_bond [selection/atom(s)/$type]"
    "\nUsage: conn bonding_radius [selection/atom(s)/$type] - note the radius"
    " should have floating point");
  xlib_InitMacro(Tolman,
    "mpd-M to P distance [2.28]&;"
    "chd-C to H distance [0.96]",
    fpNone|fpFive|psFileLoaded,
    "Calculates Tolman angle for the structure");
  xlib_InitMacro(Split,
    "r-EADP,ISOR,SIMU,DELU,SAME to be placed for the split atoms",
    fpAny|psCheckFileTypeIns,
    "Splits provided atoms along the longest axis of the ADP");
  xlib_InitMacro(Bang,
    "c-copy info to the clipboard",
    fpAny|psFileLoaded,
    "Prints bonds and angles table for selected/given atoms");
  xlib_InitMacro(CalcVol,
    "n-normalises bonds before the calculation&;"
    "cs-do not clear the selection",
    fpNone|fpOne,
    "Calculates tetrahedron or bipyramidal shape volume for given (selected) "
    "atom");
  xlib_InitMacro(TLS,
    "a-apply the TLS ADP to the atoms&;"
    "q-quiet&;",
    fpAny|psFileLoaded,
    "TLS procedure. The TLS is calculated for the given atoms and then the "
    "matrices rotated to the L axes (making L diagonal) and shifted to make S "
    "symmetric. The printed R1 is calculated for ADPs in the L axes and is:\n"
    "R1=sum(i=1..3,j=i..3)(|Uobs_ij-Utls_ij|)/sum(i=1..3, j=i..3)(|Uobs_ij|)"
    "\nR2' is invariant under the rotation and is calculated as\n"
    "R2'=sum(i=1..3,j=1..3)((Uobs_ij-Utls_ij)^2)/sum(i=1..3,j=1..3)(Uobs_ij^2)"
    );
  xlib_InitMacro(RSA,
    EmptyString(),
    fpAny|psFileLoaded,
    "Identifies chiral centres and prints R/S their stereo configuration");
  xlib_InitMacro(CONF,
    "a-finds angles which made up of all given/selected atoms [true]",
    fpAny|psFileLoaded,
    "Adds dihedral angle calculation instructions to create corresponding "
    "tables in the CIF");
  xlib_InitMacro(D2CG,
    "c-copies the values to the Clipboard",
    fpAny | psFileLoaded,
    "Calculates distacne from first atom to the unit weight-centroid formed by"
    " the rest. If the variance-covariance matrix existsm also calculates the "
    "esd.");
  xlib_InitMacro(TestR,
    "s-update the scale&;"
    "t-tolerance [0.01A]",
    fpAny | psFileLoaded,
    "Test for twinning.");
  xlib_InitMacro(HKLF5,
    EmptyString(),
    fpAny | psFileLoaded,
    "HKLF5 utils.");
  xlib_InitMacro(CalcVars,
    EmptyString(),
    fpAny | psFileLoaded,
    "Calculates previously defined variables and stores the named values in "
    "olex2.calculated.* variables");
  xlib_InitMacro(Pack,
    "c-specifies if current lattice content should not be deleted&;"
    "a-generate atoms vs asymmetric units",
    fpAny | psFileLoaded,
    "Packs structure within default or given volume(6 or 2 values for "
    "parallelepiped "
    "or 1 for sphere). If atom names/types are provided it only packs the "
    "provided atoms.");
  xlib_InitMacro(Grow,
    "s-grow shells vs fragments&;"
    "w-grows the rest of the structure, using already applied generators&;"
    "t-grows only provided atoms/atom types&;",
    fpAny | psFileLoaded,
    "Grows whole structure or provided atoms only");
  xlib_InitMacro(Convert,
    EmptyString(),
    fpTwo,
    "Converts one file to the other without loading it into the GUI");
  xlib_InitMacro(SetCharge,
    EmptyString(),
    fpAny^fpNone,
    "Sets charge to the provided (selected) atoms");
  xlib_InitMacro(Lowdin,
    EmptyString(),
    fpSix|fpOne|fpNone,
    "Calculates orthogonalised unit cell axis lengths using Lowdin method."
    "Does the calculation for the curretly loaded file, 6 cell parameters or"
    "for a file that has space-separated lines starting with Id followed by"
    " the 6 cell parameters");
  //_____________________________________________________________________________

  xlib_InitFunc(FileName, fpNone|fpOne,
    "If no arguments provided, returns file name of currently loaded file, for "
    "one argument returns extracted file name");
//_____________________________________________________________________________
  xlib_InitFunc(FileExt, fpNone|fpOne,
    "Returns file extension. If no arguments provided - of currently loaded "
    "file");
  xlib_InitFunc(FilePath, fpNone|fpOne,
    "Returns file path. If no arguments provided - of currently loaded file");
  xlib_InitFunc(FileFull, fpNone, "Returns full path of currently loaded file");
  xlib_InitFunc(FileDrive, fpNone|fpOne,
    "Returns file drive. If no arguments provided - of currently loaded file");
  xlib_InitFunc(Title, fpNone|fpOne,
    "If the file is loaded, returns it's title else if a parameter passed, it "
    "is returned");
  xlib_InitFunc(IsFileLoaded, fpNone, "Returns true/false");
  xlib_InitFunc(IsFileType, fpOne,
    "Checks type of currently loaded file [ins,res,ires,cif,cmf,mol,xyz]");
//_____________________________________________________________________________
  xlib_InitFunc(BaseDir, fpNone, "Returns the startup folder");
  xlib_InitFunc(DataDir, fpNone, "Returns the location of user data");
  xlib_InitFunc(HKLSrc, fpNone|fpOne|psFileLoaded,
    "Returns/sets hkl source for currently loaded file");
//_____________________________________________________________________________
  xlib_InitFunc(LSM, fpNone|fpOne|psCheckFileTypeIns,
    "Return/sets current refinement method, L.S. or CGLS currently. The method"
    " can also be set using LS macro");
  xlib_InitFunc(SSM, fpNone|fpOne,
    "Return current structure solution method, TREF or PATT currently. If "
    "current method is unknown and an argument is provided, that argument is"
    " returned");
  xlib_InitFunc(Ins, fpOne|psCheckFileTypeIns,
    "Returns instruction value (all data after the instruction). In case the "
    "instruction does not exist it return 'n/a' string");
  xlib_InitFunc(SG, fpNone|fpOne,
    "Returns space group of currently loaded file. Also takes a string "
    "template, where %# is replaced with SG number, %n - short name, %N - full"
    " name, %h - html representation of the short name, %H - same as %h for "
    "full name, %s - syngony, %HS -Hall symbol");
  xlib_InitFunc(SGS, fpNone|fpOne|psFileLoaded, "Returns current space settings");
//_____________________________________________________________________________
  xlib_InitFunc(ATA, fpAny|psFileLoaded,
    "Test current structure against database. (Atom Type Assignment). Returns "
    "true if any atom type changed");
//_____________________________________________________________________________
  xlib_InitFunc(FATA, fpAny|psFileLoaded,
    "Calculates the diff Fourier map and integrates it to find artifacts "
    "around atoms. (Fourier Atom Type Analysis). Returns true if any atom type"
    " changed");
//_____________________________________________________________________________
  xlib_InitFunc(VSS, fpOne|psFileLoaded,
    "Validate Structure or Solution. Takes a boolean value. If value is true, "
    "the number of tested atoms is limited by the 18A rule. Returns proportion"
    " of known atom types to the all atoms number.");
//_____________________________________________________________________________
  xlib_InitFunc(RemoveSE, fpOne|psFileLoaded,
    "Returns a new space group name without provided element");
//_____________________________________________________________________________
  xlib_InitFunc(Run, fpOne,
    "Same as the macro, executes provided commands (separated by >>) returns "
    "true if succeded");
//_____________________________________________________________________________
  xlib_InitFunc(Lst, fpOne|psCheckFileTypeIns,
    "returns a value from the Lst file");
//_____________________________________________________________________________
  xlib_InitFunc(Crd, fpAny|psFileLoaded,
    "Returns center of given (selected) atoms in cartesian coordinates");
  xlib_InitFunc(CCrd, fpAny|psFileLoaded,
    "Returns center of given (selected) atoms in fractional coordinates");
  xlib_InitFunc(CalcR, fpNone|fpOne|psFileLoaded,
    "Calculates R1, R1 for I/sig >2 and wR2. If 'print' is provided - prints "
    "detailed info");
  xlib_InitFunc(GetCompilationInfo, fpAny,
    "Returns compilation info");
  xlib_InitFunc(Cell, fpOne|psFileLoaded,
    "Returns value of teh given parameter: a, b, c, alpha, beta, gamma, volume"
  );
  xlib_InitFunc(Cif, fpOne| fpTwo |psCheckFileTypeCif,
    "Returns instruction value (all data after the instruction). In case the "
    "instruction does not exist it return 'n/a' string.");
  xlib_InitFunc(P4p, fpOne|psCheckFileTypeP4P,
    "Returns instruction value (all data after the instruction). In case the "
    "instruction does not exist it return 'n/a' string");
  xlib_InitFunc(Crs, fpOne|psCheckFileTypeCRS,
    "Returns instruction value (all data after the instruction). In case the "
    "instruction does not exist it return 'n/a' string");
  xlib_InitFunc(VVol, fpNone|fpOne|psFileLoaded,
    "A simplistic procedure to calculate molecular volume - only pairwise "
    "overlapping of atoms is considered");
  xlib_InitFunc(Env, fpOne|psFileLoaded,
    "Returns immediate atom environment");
  xlib_InitFunc(SGList, fpNone|psFileLoaded,
    "Returns result of the last call to the space group determination "
    "procedure");
  xlib_InitFunc(HKLF, fpAny | psFileLoaded,
    "If no arguments given - returns current HKLF value, otherwise if the given"
    " value 0 - sets HKLF to 4 else - sets HKLF to 5 and adds the given number "
    "of BASF parameters");
  xlib_InitFunc(StrDir, fpNone | psFileLoaded,
    "Returns location of the folder, where Olex2 stores structure related "
    "data");
  xlib_InitFunc(HAddCount, fpNone | psFileLoaded,
    "calculates the number of H atoms HAdd will add");
}
//.............................................................................
void XLibMacros::macTransform(TStrObjList &Cmds,
  const TParamList &Options, TMacroData &Error)
{
  smatd tm;
  if (!Parse(Cmds, "mivd", &tm.r, &tm.t)) {
    Error.ProcessingError(__OlxSrcInfo, "invalid transformation matrix");
    return;
  }
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms = xapp.FindSAtoms(Cmds, true);
  xapp.XFile().GetLattice().TransformFragments(atoms, tm);
}
//.............................................................................
void XLibMacros::macPush(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  vec3d pnt;
  if (!Parse(Cmds, "vd", &pnt)) {
    Error.ProcessingError(__OlxSrcInfo, "invalid translation");
    return;
  }
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms = xapp.FindSAtoms(Cmds, true);
  smatd tm;
  tm.I();
  tm.t = pnt;
  xapp.XFile().GetLattice().TransformFragments(atoms, tm);
}
//.............................................................................
void XLibMacros::macInv(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  // forces inversion for sg without center of inversion
  bool Force = Options.Contains("f");
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup* sg = NULL;
  try { sg = &xapp.XFile().GetLastLoaderSG(); }
  catch (...) {
    Error.ProcessingError(__OlxSrcInfo, "unknown file space group");
    return;
  }
  if (!sg->IsCentrosymmetric() && !Force) {
    Error.ProcessingError(__OlxSrcInfo,
      "non-centrosymmetric space group, use -f to force");
    return;
  }
  olxstr_dict<olxstr> specials;
  specials.Add("P31", "P32");
  specials.Add("P3121", "P3221");
  specials.Add("P3112", "P3212");
  specials.Add("P61", "P65");
  specials.Add("P62", "P64");
  specials.Add("P6122", "P6522");
  specials.Add("P6222", "P6422");
  specials.Add("P41", "P43");
  specials.Add("P4122", "P4322");
  specials.Add("P41212", "P43212");
  specials.Add("P4132", "P4332");
  // re-map the reverse
  {
    const olxstr_dict<olxstr> tmp = specials;
    for (size_t i = 0; i < tmp.Count(); i++)
      specials.Add(tmp.GetValue(i), tmp.GetKey(i));
  }

  TSAtomPList atoms = xapp.FindSAtoms(Cmds, true);
  SortedObjectList<const TNetwork*, TPointerComparator> frags;
  if (!sg->IsCentrosymmetric()) {
    for (size_t i = 0; i < atoms.Count(); i++) {
      frags.AddUnique(&atoms[i]->GetNetwork());
    }
  }
  smatd tm;
  tm.I() *= -1;
  tm.t = sg->GetInversionCenter()*(-2);
  xapp.XFile().GetLattice().TransformFragments(atoms, tm);
  size_t s_c = specials.IndexOf(sg->GetName());
  if (s_c != InvalidIndex) {
    TBasicApp::NewLogEntry() << "Changing spacegroup from "
      << specials.GetKey(s_c) << " to " << specials.GetValue(s_c);
    sg = TSymmLib::GetInstance().FindGroupByName(specials.GetValue(s_c));
    if (sg == 0) {
      Error.ProcessingError(__OlxSrcInfo, "Failed to locate required space group");
      return;
    }
    xapp.XFile().GetAsymmUnit().ChangeSpaceGroup(*sg);
    xapp.XFile().LastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
    xapp.XFile().EndUpdate();
  }
  if (!sg->IsCentrosymmetric() &&
    frags.Count() != xapp.XFile().GetLattice().FragmentCount())
  {
    TBasicApp::NewLogEntry() << "Please note that only visible fragments were "
      "transformed. Reload the file to undo the transformation and use 'fmol' "
      "to show all fragments.";
  }
}
//.............................................................................
void XLibMacros::macSAInfo(TStrObjList &Cmds, const TParamList &Options, TMacroData &E)  {
  const TSymmLib& sl = TSymmLib::GetInstance();
  TLog& log = TBasicApp::GetLog();
  TPtrList<TSymmElement> ref, sg_elm;
  if( Cmds.IsEmpty() )  {
    TPtrList<TSpaceGroup> hits, bl_hits;
    for( size_t i=0; i < sl.SymmElementCount(); i++ )
      ref.Add(sl.GetSymmElement(i));
    for( size_t i=0; i < sl.SGCount(); i++ )  {
      sl.GetGroup(i).SplitIntoElements(ref, sg_elm);
      if( sg_elm.IsEmpty() )
        hits.Add(&sl.GetGroup(i));
      else
        sg_elm.Clear();
    }
    TStrList output;
    for( size_t i=0; i < sl.BravaisLatticeCount(); i++ )  {
      for( size_t j=0; j < hits.Count(); j++ )  {
        if( &hits[j]->GetBravaisLattice() == &sl.GetBravaisLattice(i) )
          bl_hits.Add(hits[j]);
      }
      if( bl_hits.IsEmpty() )  continue;
      TTTable<TStrList> tab(bl_hits.Count()/6+((bl_hits.Count()%6) != 0 ? 1 : 0), 6);
      for( size_t j=0; j < bl_hits.Count(); j++ )
        tab[j/6][j%6] = bl_hits[j]->GetName();
      tab.CreateTXTList(output, sl.GetBravaisLattice(i).GetName(), false, false, "  ");
      log.NewEntry() << output;
      output.Clear();
      bl_hits.Clear();
    }
  }
  else  {
    sorted::PrimitiveAssociation<size_t, TSpaceGroup*> hits, bl_hits;
    for( size_t i=0; i < Cmds.Count(); i++ )  {
      TSymmElement* se = sl.FindSymmElement(olxstr(Cmds[i]).Replace('~', '-'));
      if( se == NULL )  {
        E.ProcessingError(__OlxSrcInfo, olxstr("Unknown symmetry element: ") << Cmds[i]);
        return;
      }
      ref.Add(se);
    }
    for( size_t i=0; i < sl.SGCount(); i++ )  {
      sl.GetGroup(i).SplitIntoElements(ref, sg_elm);
      if( !sg_elm.IsEmpty() )  {
        hits.Add(sg_elm.Count(), &sl.GetGroup(i));
        sg_elm.Clear();
      }
    }
    TStrList output;
    for( size_t i=0; i < sl.BravaisLatticeCount(); i++ )  {
      for( size_t j=0; j < hits.Count(); j++ )  {
        if( &hits.GetValue(j)->GetBravaisLattice() == &sl.GetBravaisLattice(i) )
          bl_hits.Add(hits.GetKey(j), hits.GetValue(j));
      }
      if( bl_hits.IsEmpty() )  continue;
      TTTable<TStrList> tab( bl_hits.Count()/5+((bl_hits.Count()%5) != 0 ? 1 : 0), 5);
      olxstr tmp;
      for( size_t j=0; j < bl_hits.Count(); j++ )  {
        tmp = bl_hits.GetValue(j)->GetName();
        tmp.RightPadding(10, ' ');
        tab[j/5][j%5] << tmp << ' ' << bl_hits.GetKey(j) << '/' << ref.Count();
      }
      tab.CreateTXTList(output, sl.GetBravaisLattice(i).GetName(), false, false, "  ");
      log.NewEntry() << output;
      output.Clear();
      bl_hits.Clear();
    }
    log.NewEntry() << "Exact matche(s)";
    TPtrList<TSymmElement> all_elm;
    for( size_t i=0; i < sl.SymmElementCount(); i++ )
      all_elm.Add(sl.GetSymmElement(i));
    olxstr exact_match;
    for( size_t i=0; i < hits.Count(); i++ )  {
      if( hits.GetKey(i) == ref.Count() )  {
        if( !exact_match.IsEmpty() )
          exact_match << ", ";
        hits.GetValue(i)->SplitIntoElements(all_elm, sg_elm);
        bool exact = true;
        for( size_t j=0; j < sg_elm.Count(); j++ )  {
          if( ref.IndexOf(sg_elm[j]) == InvalidIndex )  {
            exact = false;
            break;
          }
        }
        if( exact )  exact_match << '[';
        exact_match << hits.GetValue(i)->GetName();
        if( exact )  exact_match << ']';
        sg_elm.Clear();
      }
    }
    output.Hyphenate(exact_match, 80);
    log.NewEntry() << output;
    log.NewEntry() << "Space groups inclosed in [] have exact match to the provided elements";
  }
}
//.............................................................................
void XLibMacros::macSGInfo(TStrObjList &Cmds, const TParamList &Options, TMacroData &E)  {
  if( Cmds.IsEmpty() )  {
    TPtrList<TSpaceGroup> sgList;
    TSymmLib& symlib = TSymmLib::GetInstance();
    for( size_t i=0; i < symlib.BravaisLatticeCount(); i++ )  {
      TBravaisLattice& bl = symlib.GetBravaisLattice(i);
      bl.FindSpaceGroups(sgList);
      TBasicApp::NewLogEntry() << "------------------- " << bl.GetName() <<
        " --- "  << sgList.Count();
      olxstr tmp, tmp1;
      sorted::PrimitiveAssociation<int, TSpaceGroup*> SortedSG;
      for( size_t j=0; j < sgList.Count(); j++ )
        SortedSG.Add(sgList[j]->GetNumber(), sgList[j]);
      for( size_t j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetValue(j)->GetName() << "(#" << SortedSG.GetKey(j) << ')';
        tmp << tmp1.RightPadding(15, ' ', true);
        tmp1.SetLength(0);
        if( tmp.Length() > 60 )  {
          TBasicApp::NewLogEntry() << tmp;
          tmp.SetLength(0);
        }
      }
      TBasicApp::NewLogEntry() << tmp;
      sgList.Clear();
    }
    return;
  }
  const bool Inversion = Options.Contains("i"),
    Centering = Options.Contains("c");
  TSpaceGroup* sg = TSymmLib::GetInstance().FindGroupByName(Cmds[0]);
  bool LaueClassPG = false;
  if( sg == NULL )  {
    sg = TSymmLib::GetInstance().FindGroupByName(olxstr("P") << Cmds[0]);
    if( !sg )  {
      E.ProcessingError(__OlxSrcInfo,
        "Could not find specified space group/Laue class/Point group: ") <<
        Cmds[0];
      return;
    }
    LaueClassPG = true;
  }
  if( LaueClassPG )  {
    TPtrList<TSpaceGroup> sgList;
    sorted::PrimitiveAssociation<int, TSpaceGroup*> SortedSG;
    if( &sg->GetLaueClass() == sg )  {
      TBasicApp::NewLogEntry() << "Space groups of the Laue class " <<
        sg->GetBareName();
      TSymmLib::GetInstance().FindLaueClassGroups( *sg, sgList);
      for( size_t j=0; j < sgList.Count(); j++ )
        SortedSG.Add(sgList[j]->GetNumber(), sgList[j]);
      olxstr tmp, tmp1;
      for( size_t j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetValue(j)->GetName() << "(#" << SortedSG.GetKey(j)
          << ')';
        tmp << tmp1.RightPadding(15, ' ', true);
        tmp1.SetLength(0);
        if( tmp.Length() > 60 )  {
          TBasicApp::NewLogEntry() << tmp;
          tmp.SetLength(0);
        }
      }
      TBasicApp::NewLogEntry() << tmp;
    }
    if( &sg->GetPointGroup() == sg )  {
      sgList.Clear();
      SortedSG.Clear();
      olxstr tmp, tmp1;
      TBasicApp::NewLogEntry() << "Space groups of the point group " <<
        sg->GetBareName();
      TSymmLib::GetInstance().FindPointGroupGroups(*sg, sgList);
      sorted::PrimitiveAssociation<int, TSpaceGroup*> SortedSG;
      for( size_t j=0; j < sgList.Count(); j++ )
        SortedSG.Add(sgList[j]->GetNumber(), sgList[j]);
      for( size_t j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetValue(j)->GetName() << "(#" << SortedSG.GetKey(j)
          << ')';
        tmp << tmp1.RightPadding(15, ' ', true);
        tmp1.SetLength(0);
        if( tmp.Length() > 60 )  {
          TBasicApp::NewLogEntry() << tmp;
          tmp.SetLength(0);
        }
      }
      TBasicApp::NewLogEntry() << tmp;
    }
    return;
  }
  TPtrList<TSpaceGroup> AllGroups;
  smatd_list SGMatrices;

  TBasicApp::NewLogEntry() << (sg->IsCentrosymmetric() ? "Centrosymmetric"
    : "Non centrosymmetric");
  TBasicApp::NewLogEntry() << "Hall symbol: " << sg->GetHallSymbol();

  TSymmLib::GetInstance().GetGroupByNumber(sg->GetNumber(), AllGroups);
  if( AllGroups.Count() > 1 )  {
    TBasicApp::NewLogEntry() << "Alternative settings:";
    olxstr tmp;
    for( size_t i=0; i < AllGroups.Count(); i++ )  {
      if( AllGroups[i] == sg )  continue;
      tmp << AllGroups[i]->GetName() << '(' << AllGroups[i]->GetFullName()
        <<  ") ";
    }
    TBasicApp::NewLogEntry() << tmp;
  }
  TBasicApp::NewLogEntry() << "Space group number: " << sg->GetNumber();
  TBasicApp::NewLogEntry() << "Crystal system: " << sg->GetBravaisLattice().GetName();
  TBasicApp::NewLogEntry() << "Laue class: " << sg->GetLaueClass().GetBareName();
  TBasicApp::NewLogEntry() << "Point group: " << sg->GetPointGroup().GetBareName();
  short Flags = mattAll;
  if( !Centering )  Flags ^= mattCentering;
  if( !Inversion )  Flags ^= mattInversion;
  sg->GetMatrices(SGMatrices, Flags);

  TTTable<TStrList> tab(SGMatrices.Count(), 2);

  for( size_t i=0; i < SGMatrices.Count(); i++ )
    tab[i][0] = TSymmParser::MatrixToSymmEx(SGMatrices[i]);

  TStrList Output;
  tab.CreateTXTList(Output, "Symmetry operators", true, true, ' ');
  if( !sg->GetInversionCenter().IsNull() )  {
    const vec3d& ic = sg->GetInversionCenter();
    TBasicApp::NewLogEntry() << "Inversion center position: " << olxstr::FormatFloat(3, ic[0])
      << ", " << olxstr::FormatFloat(3, ic[1]) << ", " << olxstr::FormatFloat(3, ic[2]);
  }
  // possible systematic absences
  Output.Add("Elements causing systematic absences: ");
  TPtrList<TSymmElement> ref, sg_elm;
  for( size_t i=0; i < TSymmLib::GetInstance().SymmElementCount(); i++ )
    ref.Add(TSymmLib::GetInstance().GetSymmElement(i));
  sg->SplitIntoElements(ref, sg_elm);
  if( sg_elm.IsEmpty() )
    Output.GetLastString() << "none";
  else  {
    for( size_t i=0; i < sg_elm.Count(); i++ )
      Output.GetLastString() << sg_elm[i]->GetName() << ' ';
  }
  TBasicApp::NewLogEntry() << Output;
}
//.............................................................................
void XLibMacros::macSort(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TStrList cmds = (Cmds.IsEmpty() ? TStrList("+ml moiety", ' ')
    : TStrList(Cmds));
  TXApp::GetInstance().XFile().Sort(cmds, Options);
  TBasicApp::NewLogEntry() << "Atom order after sorting :";
  olxstr_buf atoms;
  olxstr ws = ' ';
  const TAsymmUnit &au = TXApp::GetInstance().XFile().GetAsymmUnit();
  for (size_t i=0; i < au.ResidueCount(); i++) {
    TResidue &r = au.GetResidue(i);
    for (size_t j = 0; j < r.Count(); j++) {
      if (r[j].IsDeleted()) {
        continue;
      }
      olxstr l = r[j].GetResiLabel();
      if (atoms.Length() + l.Length() >= 80) {
        TBasicApp::NewLogEntry() << olxstr(atoms);
        atoms.Clear();
      }
      atoms << ws << l;
    }
  }
  if (!atoms.IsEmpty()) {
    TBasicApp::NewLogEntry() << olxstr(atoms);
  }
}
//.............................................................................
void XLibMacros::macRun(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error)  {
  using namespace olex2;
  IOlex2Processor* op = IOlex2Processor::GetInstance();
  if( op == NULL ) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "this function requires Olex2 processor implementation");
  }
  TStrList allCmds = TParamList::StrtokLines(Cmds.Text(' '), ">>");
  for( size_t i=0; i < allCmds.Count(); i++ )  {
    if( !op->processMacro(allCmds[i]) )  {
      if( (i+1) < allCmds.Count() ) {
        TBasicApp::NewLogEntry(logError) <<
          "Not all macros in the provided list were executed";
      }
      break;
    }
  }
}
//.............................................................................
void XLibMacros::macHklStat(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  if (xapp.XFile().GetRM().GetReflections().IsEmpty()) {
    Error.ProcessingError(__OlxSrcInfo, "Empty reflection list");
    return;
  }
  if (Cmds.IsEmpty()) {
    RefinementModel::HklStat hs = xapp.XFile().GetRM().GetMergeStat();
    TTTable<TStrList> tab(22, 2);
    tab[0][0] << "Total reflections (after filtering)";
    tab[0][1] << hs.TotalReflections;
    tab[1][0] << "Data/Unique reflections";
    tab[1][1] << hs.DataCount << '/' << hs.UniqueReflections;
    tab[2][0] << "Centric reflections";
    tab[2][1] << hs.CentricReflections;
    tab[3][0] << "Friedel pairs merged";
    tab[3][1] << hs.FriedelOppositesMerged;
    tab[4][0] << "Inconsistent equivalents";
    tab[4][1] << hs.InconsistentEquivalents;
    tab[5][0] << "Systematic absences removed (all/unique)";
    tab[5][1] << hs.SystematicAbsencesRemoved << '/' <<
      hs.UniqueSystematicAbsencesRemoved;
    tab[6][0] << "Min d";
    tab[6][1] << olxstr::FormatFloat(3, hs.MinD);
    tab[7][0] << "Max d";
    tab[7][1] << olxstr::FormatFloat(3, hs.MaxD);
    tab[8][0] << "Limiting d min (SHEL)";
    tab[8][1] << (hs.LimDmin == 0 ? NAString()
      : olxstr::FormatFloat(3, hs.LimDmin));
    tab[9][0] << "Limiting d max (SHEL/OMIT_2t)";
    tab[9][1] << (hs.LimDmax == 100 ? NAString()
      : olxstr::FormatFloat(3, hs.LimDmax));
    tab[10][0] << "Filtered off reflections (SHEL/OMIT_s/OMIT_2t)";
    tab[10][1] << hs.FilteredOff;
    tab[11][0] << "Reflections omitted by user (OMIT_hkl)";
    tab[11][1] << hs.OmittedByUser;
    tab[12][0] << "Reflections skipped (after 0 0 0)";
    tab[12][1] << hs.OmittedReflections;
    tab[13][0] << "Intensity transformed for (OMIT_s)";
    tab[13][1] << hs.IntensityTransformed << " reflections";
    tab[14][0] << "Rint, %";
    tab[14][1] << (hs.Rint < 0 ? NAString()
      : olxstr::FormatFloat(2, hs.Rint * 100));
    tab[15][0] << "Rsigma, %";
    tab[15][1] << (hs.Rsigma < 0 ? NAString()
      : olxstr::FormatFloat(2, hs.Rsigma * 100));
    tab[16][0] << "Completeness, [d_min-d_max] %";
    tab[16][1] << olxstr::FormatFloat(2, hs.Completeness * 100);
    tab[17][0] << "Mean I/sig";
    tab[17][1] << olxstr::FormatFloat(3, hs.MeanIOverSigma);
    tab[18][0] << "HKL range (refinement)";
    tab[18][1] << "h=[" << hs.MinIndexes[0] << ',' << hs.MaxIndexes[0] << "] "
      << "k=[" << hs.MinIndexes[1] << ',' << hs.MaxIndexes[1] << "] "
      << "l=[" << hs.MinIndexes[2] << ',' << hs.MaxIndexes[2] << "] ";
    tab[19][0] << "HKL range (file)";
    tab[19][1] << "h=[" << hs.FileMinInd[0] << ',' << hs.FileMaxInd[0] << "] "
      << "k=[" << hs.FileMinInd[1] << ',' << hs.FileMaxInd[1] << "] "
      << "l=[" << hs.FileMinInd[2] << ',' << hs.FileMaxInd[2] << "] ";
    tab[20][0] << "Maximum redundancy (+symm eqivs)";
    if (hs.HKLF >= 5) {
      tab[20][1] = NAString();
    }
    else {
      tab[20][1] << hs.ReflectionAPotMax;
    }
    tab[21][0] << "Average redundancy (+symm eqivs)";
    if (hs.HKLF >= 5) {
      tab[20][1] = NAString();
    }
    else {
      tab[21][1] << olxstr::FormatFloat(2,
        (double)hs.TotalReflections / hs.UniqueReflections);
    }

    TStrList Output = tab.CreateTXTList(
      "Refinement reflection statistsics", true, false, "  ");
    const TIntList& redInfo = xapp.XFile().GetRM().GetRedundancyInfo();
    int red_cnt = 0;
    for (size_t i = 0; i < redInfo.Count(); i++) {
      if (redInfo[i] != 0) {
        red_cnt++;
      }
    }
    tab.Resize(red_cnt, 2);
    tab.ColName(0) = "Times measured";
    tab.ColName(1) = "Count";
    red_cnt = 0;
    for (size_t i = 0; i < redInfo.Count(); i++) {
      if (redInfo[i] == 0) {
        continue;
      }
      tab[red_cnt][0] = i + 1;
      tab[red_cnt++][1] = redInfo[i];
    }
    Output << tab.CreateTXTList(
      "All reflection statistics", true, false, "  ");
    xapp.NewLogEntry() << Output;
    xapp.NewLogEntry() << "Friedel pairs measured (in P1): " <<
      xapp.XFile().GetRM().GetFriedelPairCount();
    if (hs.HKLF >= 5) {
      xapp.NewLogEntry() << "Note that the merging stats are given for batch "
        "#1 only";
    }
    return;
  }
  bool list = Options.Contains("l"),
    merge = Options.Contains("m");
  TRefList Refs;
  if (merge) {
    xapp.XFile().GetRM().GetRefinementRefList<
      TUnitCell::SymmSpace, RefMerger::StandardMerger>(
        xapp.XFile().GetUnitCell().GetSymmSpace(), Refs);
  }
  else {
    xapp.XFile().GetRM().GetFilteredP1RefList<RefMerger::StandardMerger>(Refs);
  }
  evecd_list con;

  for (size_t i = 0; i < Cmds.Count(); i++) {
    size_t obi = Cmds[i].FirstIndexOf('[');
    if (obi == InvalidIndex || !Cmds[i].EndsWith(']')) {
      Error.ProcessingError(__OlxSrcInfo, "incorrect construct: ") << Cmds[i];
      return;
    }
    con.AddNew(4);
    con[i][3] = Cmds[i].SubStringTo(obi).ToInt();
    olxstr tmp = Cmds[i].SubString(obi + 1, Cmds[i].Length() - obi - 2);
    int hkli = -1;
    for (size_t j = tmp.Length() - 1; j != InvalidIndex; j--) {
      if (tmp.CharAt(j) == 'l')  hkli = 2;
      else if (tmp.CharAt(j) == 'k') {
        hkli = 1;
      }
      else if (tmp.CharAt(j) == 'h') {
        hkli = 0;
      }
      if (hkli == -1) {
        Error.ProcessingError(__OlxSrcInfo, "incorrect construct: ") << Cmds[i];
        return;

      }
      j--;
      olxstr strV;
      while (j != InvalidIndex && !(tmp.CharAt(j) >= 'a' && tmp.CharAt(j) <= 'z')) {
        strV.Insert((olxch)tmp[j], 0);
        j--;
      }
      if (!strV.IsEmpty() && !(strV == "+") && !(strV == "-"))
        con[i][hkli] = strV.ToDouble();
      else {
        if (!strV.IsEmpty() && strV == "-") {
          con[i][hkli] = -1.0;
        }
        else {
          con[i][hkli] = 1.0;
        }
      }
      if (con[i][hkli] == 0) {
        Error.ProcessingError(__OlxSrcInfo, "illegal value: ") << Cmds[i];
        return;
      }
      j++;
    }
  }
  double SI = 0, SE = 0;
  size_t count = 0;
  for (size_t i = 0; i < Refs.Count(); i++) {
    bool fulfilled = true;
    const TReflection& ref = Refs[i];
    for (size_t j = 0; j < Cmds.Count(); j++) {
      int v = olx_round(ref.GetH()*con[j][0] +
        ref.GetK()*con[j][1] +
        ref.GetL()*con[j][2]);
      if (con[j][3] == 0) {
        if (v != 0) {
          fulfilled = false;
          break;
        }
      }
      else if (con[j][3] < 0) {
        if ((v % (int)con[j][3]) == 0) {
          fulfilled = false;
          break;
        }
      }
      else if (con[j][3] > 0) {
        if ((v % (int)con[j][3]) != 0) {
          fulfilled = false;
          break;
        }
      }
    }
    if (!fulfilled) {
      continue;
    }
    count++;
    SI += ref.GetI();
    SE += olx_sqr(ref.GetS());
    if (list) {
      TBasicApp::NewLogEntry() << ref.ToString();
    }
  }
  if (count == 0) {
    TBasicApp::NewLogEntry() <<
      "Could not find any reflections fulfilling given condition";
    return;
  }
  SI /= count;
  SE = sqrt(SE / count);

  xapp.NewLogEntry() << "Found " << count <<
    " reflections fulfilling given condition";
  xapp.NewLogEntry() << "I(s) is " << olxstr::FormatFloat(3, SI) << '(' <<
    olxstr::FormatFloat(3, SE) << ")";

}
//.............................................................................
void XLibMacros::macHtab(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (TXApp::GetInstance().XFile().GetLattice().IsGenerated()) {
    E.ProcessingError(__OlxSrcInfo,
      "operation is not applicable to the grown structure");
    return;
  }
  const bool use_c = Options.GetBoolOption("c");
  double def_max_d = 2.9,
    def_min_ang = TXApp::GetMinHBondAngle();
#ifdef _PYTHON
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  if (op != 0) {
    olxstr f = "spy.GetParam('snum.cif.htab_max_d')";
    if (op->processFunction(f) && f.IsNumber()) {
      def_max_d = f.ToDouble();
    }
    if (op->processFunction(f = "spy.GetParam('snum.cif.htab_min_angle')") &&
      f.IsNumber())
    {
      def_min_ang = f.ToDouble();
    }
  }
#endif
  double max_d = def_max_d, min_ang = def_min_ang;
  size_t cnt = XLibMacros::Parse(Cmds, "dd", &max_d, &min_ang);
  if (cnt == 1) {
    if (max_d > 10) {
      min_ang = max_d;
      max_d = 2.9;
    }
  }
  if (max_d > 5) {
    if (min_ang < 5)
      olx_swap(max_d, min_ang);
    else
      max_d = 2.9;
  }
#ifdef _PYTHON
  if (op != 0) {
    op->processMacro(olx_print("spy.SetParam('snum.cif.htab_max_d', %lf)", max_d));
    op->processMacro(olx_print("spy.SetParam('snum.cif.htab_min_angle', %lf)", min_ang));
  }
#endif
  TAsymmUnit& au = TXApp::GetInstance().XFile().GetAsymmUnit();
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  TStrList current;
  for (size_t i = 0; i < rm.InfoTabCount(); i++) {
    InfoTab& it = rm.GetInfoTab(i);
    if (it.IsValid() && it.GetType() == infotab_htab)
      current.Add(it.ToString());
  }
  if (!current.IsEmpty()) {
    TBasicApp::NewLogEntry() << "List of current HTAB instructions: ";
    TBasicApp::NewLogEntry() << current;
  }
  smatd_list transforms;
  TIntList bais;
  bais.Add(iNitrogenZ);
  bais.Add(iOxygenZ);
  bais.Add(iFluorineZ);
  bais.Add(iChlorineZ);
  bais.Add(iSulphurZ);
  bais.Add(iBromineZ);
  TBasicApp::NewLogEntry() << "Processing HTAB with max D-A distance " <<
    max_d << " and minimum angle " << min_ang;
  min_ang = cos(min_ang*M_PI / 180.0);
  if (Options.Contains('t')) {
    TStrList elm(Options.FindValue('t'), ',');
    for (size_t i = 0; i < elm.Count(); i++) {
      cm_Element* e = XElementLib::FindBySymbol(elm[i]);
      if (e == 0) {
        TBasicApp::NewLogEntry() << "Unknown element type: " << elm[i];
      }
      else if (bais.IndexOf(e->z) == InvalidIndex) {
        bais.Add(e->z);
      }
    }
  }
  TUnitCell& uc = TXApp::GetInstance().XFile().GetUnitCell();
  TLattice& lat = TXApp::GetInstance().XFile().GetLattice();
  const ASObjectProvider& objects = lat.GetObjects();
  for (size_t i = 0; i < objects.atoms.Count(); i++) {
    TSAtom& sa = objects.atoms[i];
    const cm_Element& elm = sa.GetType();
    if (elm.z < 2) { // H,D,Q
      continue;
    }
    TSizeList h_indexes;
    for (size_t j = 0; j < sa.NodeCount(); j++) {
      const cm_Element& elm1 = sa.Node(j).GetType();
      if (elm1 == iHydrogenZ) {
        h_indexes << j;
      }
    }
    if (h_indexes.IsEmpty()) {
      continue;
    }
    TArrayList<olx_pair_t<TCAtom const*, smatd> > all;
    uc.FindInRangeAM(sa.ccrd(), elm.r_bonding + 0.4, max_d, all);
    for (size_t j = 0; j < all.Count(); j++) {
      const TCAtom& ca = *all[j].GetA();
      if (!TNetwork::IsBondAllowed(sa, ca, all[j].GetB())) {
        continue;
      }
      const cm_Element& elm1 = ca.GetType();
      if (bais.IndexOf(elm1.z) == InvalidIndex) {
        continue;
      }
      vec3d cvec(all[j].GetB()*ca.ccrd()),
        bond(cvec - sa.ccrd());
      const double d = au.CellToCartesian(bond).Length();
      if (d < (elm.r_bonding + elm1.r_bonding + 0.4)) { // coval bond
        continue;
      }
      // analyse angles
      for (size_t k = 0; k < h_indexes.Count(); k++) {
        vec3d base = sa.Node(h_indexes[k]).ccrd();
        const vec3d v1 = au.Orthogonalise(sa.ccrd() - base);
        const vec3d v2 = au.Orthogonalise(cvec - base);
        const double c_a = v1.CAngle(v2);
        if (c_a < min_ang) {  // > 150 degrees
          if (sa.GetType() == iCarbonZ && use_c) {
            InfoTab& it_d = rm.AddRTAB(
              sa.GetType().symbol + ca.GetType().symbol);
            it_d.AddAtom(sa.CAtom(), 0);
            const smatd* mt = (!(all[j].GetB().t.IsNull() &&
              all[j].GetB().r.IsI()) ? &all[j].GetB() : 0);
            if (mt != 0 && transforms.IndexOf(*mt) == InvalidIndex) {
              transforms.AddCopy(*mt);
            }
            it_d.AddAtom(*const_cast<TCAtom*>(&ca), mt);
            if (rm.ValidateInfoTab(it_d)) {
              TBasicApp::NewLogEntry() << it_d.InsStr() << " d=" <<
                olxstr::FormatFloat(3, d);
            }

            InfoTab& it_a = rm.AddRTAB(
              sa.GetType().symbol + ca.GetType().symbol);
            it_a.AddAtom(sa.CAtom(), 0);
            it_a.AddAtom(sa.Node(h_indexes[k]).CAtom(), 0);
            it_a.AddAtom(*const_cast<TCAtom*>(&ca), mt);
            if (rm.ValidateInfoTab(it_a)) {
              TBasicApp::NewLogEntry() << it_a.InsStr() << " a=" <<
                olxstr::FormatFloat(3, acos(c_a)*180.0 / M_PI);
            }
          }
          else {
            InfoTab& it = rm.AddHTAB();
            it.AddAtom(sa.CAtom(), 0);
            const smatd* mt = (!(all[j].GetB().t.IsNull() &&
              all[j].GetB().r.IsI()) ? &all[j].GetB() : 0);
            if (mt != 0 && transforms.IndexOf(*mt) == InvalidIndex) {
              transforms.AddCopy(*mt);
            }
            it.AddAtom(*const_cast<TCAtom*>(&ca), mt);
            it.UpdateResi();
            if (rm.ValidateInfoTab(it)) {
              TBasicApp::NewLogEntry() << it.ToString();
            }
          }
        }
      }
    }
  }
  if (Options.GetBoolOption('g') && !transforms.IsEmpty()) {
    TLattice& xlatt = TXApp::GetInstance().XFile().GetLattice();
    const TUnitCell& uc = xlatt.GetUnitCell();
    const ASObjectProvider& objects = xlatt.GetObjects();
    for (size_t i = 0; i < transforms.Count(); i++)
      uc.InitMatrixId(transforms[i]);
    TCAtomPList iatoms;
    for (size_t i = 0; i < objects.atoms.Count(); i++) {
      TSAtom& sa = objects.atoms[i];
      if (sa.IsDeleted()) {
        continue;
      }
      if (sa.IsAUAtom()) {
        iatoms.Add(sa.CAtom());
      }
    }
    xlatt.GrowAtoms(iatoms, transforms);
  }
}
//.............................................................................
void XLibMacros::macHAdd(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  int Hfix = 0;
  if (!Cmds.IsEmpty() && Cmds[0].IsNumber()) {
    Hfix = Cmds[0].ToInt();
    Cmds.Delete(0);
  }
  TXApp &XApp = TXApp::GetInstance();
  if (XApp.XFile().GetLattice().IsGenerated() && Hfix >= 0) {
    Error.ProcessingError(__OlxSrcInfo, "not applicable to grown structures");
    return;
  }
  TAsymmUnit &au = XApp.XFile().GetAsymmUnit();
  for (size_t i=0; i < au.AtomCount(); i++) {
    TCAtom &ca = au.GetAtom(i);
    if (ca.GetType() == iHydrogenZ) {
      ca.SetDetached(false);
    }
  }
  TActionQueueLock q_draw(XApp.FindActionQueue(olxappevent_GL_DRAW));
  try {
    TLattice &latt = XApp.XFile().GetLattice();
    TSAtomPList satoms = XApp.FindSAtoms(Cmds, true);
    // find atoms first, or selection gets lost...
    latt.UpdateConnectivity();
    RefinementModel &rm = XApp.XFile().GetRM();
    TXlConGen xlConGen(rm);
    xlConGen.Options = Options;
    xlConGen.SetUseRestrains(Options.GetBoolOption('r'));
    TUnitCell &uc = XApp.XFile().GetUnitCell();
    if (Hfix == 0) {
      TSAtomPList c, n;
      satoms.ForEach(ACollectionItem::TagSetter(0));
      for (size_t i = 0; i < satoms.Count(); i++) {
        if (satoms[i]->GetType() == iCarbonZ) {
          c << satoms[i];
          satoms[i]->SetTag(1);
        }
        else if(satoms[i]->GetType() == iNitrogenZ) {
          n << satoms[i];
          satoms[i]->SetTag(1);
        }
      }
      TSAtomPList others = satoms.Filter(ACollectionItem::TagAnalyser(0));
      latt.AnalyseHAdd(xlConGen, c, false);
      // the rest of the atoms
      latt.AnalyseHAdd(xlConGen, others, false);
      latt.AnalyseHAdd(xlConGen, n, false);
    }
    else if (Hfix < 0) {
      if (satoms.Count() > 1) {
        TAtomEnvi AE(*satoms[0]);
        for (size_t ei = 1; ei < satoms.Count(); ei++) {
          TSAtom &a = *satoms[ei];
          AE.Add(a.CAtom(), a.GetMatrix(), a.crd());
        }
        int afix = TXlConGen::ShelxToOlex(-Hfix, AE);
        if (afix != -1) {
          TCAtomPList generated;
          xlConGen.FixAtom(AE, afix, XElementLib::GetByIndex(iHydrogenIndex),
            0, &generated);
          if (!generated.IsEmpty()) {
            int n_afix = Options.FindValue("a", "3").ToInt();
            if (generated[0]->GetParentAfixGroup() != 0) {
              generated[0]->GetParentAfixGroup()->SetAfix(n_afix);
            }
            double occu = rm.Vars.GetParam(satoms[1]->CAtom(),
              catom_var_name_Sof);
            for (size_t hi = 0; hi < generated.Count(); hi++) {
              generated[hi]->SetPart(satoms[1]->CAtom().GetPart());
              rm.Vars.SetParam(*generated[hi], catom_var_name_Sof, occu);
            }
            olxstr str_part = Options.FindValue('p', EmptyString());
            if (!str_part.IsEmpty()) {
              int part = str_part.ToInt();
              for (size_t hi = 0; hi < generated.Count(); hi++) {
                generated[hi]->SetPart(part);
              }
            }
          }
        }
        else {
          XApp.NewLogEntry() << "Failed to translate HFIX code for " <<
            satoms[0]->GetLabel() << " with " << AE.Count() << " bonds";
        }
      }
      else {
        Error.ProcessingError(__OlxSrcInfo, "at least 2 atoms are expected");
        return;
      }
    }
    else {
      for (size_t aitr=0; aitr < satoms.Count(); aitr++) {
        TIntList parts;
        TDoubleList occu;
        TAtomEnvi AE = uc.GetAtomEnviList(*satoms[aitr], false, DefNoPart, true);
        for (size_t i = 0; i < AE.Count(); i++) {
          if (AE.GetCAtom(i).GetPart() != 0 &&
            AE.GetCAtom(i).GetPart() != AE.GetBase().CAtom().GetPart())
          {
            if (parts.IndexOf(AE.GetCAtom(i).GetPart()) == InvalidIndex) {
              parts.Add(AE.GetCAtom(i).GetPart());
              occu.Add(rm.Vars.GetParam(AE.GetCAtom(i), catom_var_name_Sof));
            }
          }
        }
        if (parts.Count() < 2) {
          TCAtomPList generated;
          // special for symmetry generated disorder cases
          int afix = TXlConGen::ShelxToOlex(Hfix, AE);
          if (afix != -1) {
            xlConGen.FixAtom(AE, afix, XElementLib::GetByIndex(iHydrogenIndex),
              0, &generated);
            if (!generated.IsEmpty()) {
              // hack to get desired Hfix...
              if (generated[0]->GetParentAfixGroup() != NULL) {
                generated[0]->GetParentAfixGroup()->SetAfix(
                  Options.FindValue('a', Hfix).ToInt());
              }
              olxstr str_part = Options.FindValue('p', EmptyString());
              if (!str_part.IsEmpty()) {
                int part = str_part.ToInt();
                for (size_t hi = 0; hi < generated.Count(); hi++) {
                  generated[hi]->SetPart(part);
                }
              }
            }
          }
          else {
            XApp.NewLogEntry() << "Failed to translate HFIX code for " <<
              satoms[aitr]->GetLabel() << " with " << AE.Count() << " bonds";
          }
        }
        else {
          XApp.NewLogEntry() << "Processing " << parts.Count() << " parts";
          for (size_t i=0; i < parts.Count(); i++) {
            AE = uc.GetAtomEnviList(*satoms[aitr], false, parts[i], true);
            /*consider special case where the atom is bound to itself but
            very long bond > 1.6 A */
            smatd* eqiv = 0;
            for (size_t j=0; j < AE.Count(); j++) {
              if (&AE.GetCAtom(j) == &AE.GetBase().CAtom()) {
                const double d = AE.GetCrd(j).DistanceTo(AE.GetBase().crd());
                if (d > 1.6) {
                  eqiv = new smatd(AE.GetMatrix(j));
                  AE.Delete(j);
                  break;
                }
              }
            }
            if (eqiv != 0) {
              const smatd& e = rm.AddUsedSymm(*eqiv);
              rm.Conn.RemBond(satoms[aitr]->CAtom(), satoms[aitr]->CAtom(),
                0, &e, true);
              XApp.NewLogEntry() << "The atom" << satoms[aitr]->GetLabel() <<
                " is connected to itself through symmetry, removing the"
                " symmetry generated bond";
              delete eqiv;
            }
            //
            int afix = TXlConGen::ShelxToOlex(Hfix, AE);
            if (afix != -1) {
              TCAtomPList generated;
              xlConGen.FixAtom(AE, afix,
                XElementLib::GetByIndex(iHydrogenIndex), NULL, &generated);
              for (size_t j=0; j < generated.Count(); j++) {
                generated[j]->SetPart(parts[i]);
                rm.Vars.SetParam(*generated[j], catom_var_name_Sof, occu[i]);
              }
              if (!generated.IsEmpty() &&
                  generated[0]->GetParentAfixGroup() != 0)
              {
                // a hack again
                generated[0]->GetParentAfixGroup()->SetAfix(Hfix);
              }
            }
            else  {
              XApp.NewLogEntry() << "Failed to translate HFIX code for " <<
                satoms[aitr]->GetLabel() << " with " << AE.Count() << " bonds";
            }
          }
        }
      }
    }
  }
  catch(const TExceptionBase& e)  {
    Error.ProcessingError(__OlxSrcInfo, e.GetException()->GetError());
  }
  XApp.XFile().GetLattice().Init();
  // look at ring N...
  if (Hfix == 0) {
    using namespace olx_analysis;
    TAsymmUnit &au = XApp.XFile().GetAsymmUnit();
    au.DetachAtomType(iQPeakZ, true);
    TTypeList<fragments::fragment> frags = fragments::extract(au);
    for (size_t i = 0; i < frags.Count(); i++) {
      TCAtomPList r_atoms;
      frags[i].breadth_first_tags(InvalidIndex, &r_atoms);
      TTypeList<fragments::ring> rings = frags[i].get_rings(r_atoms);
      for (size_t j = 0; j < rings.Count(); j++) {
        rings[j].atoms.ForEach(TCAtom::FlagSetter(catom_flag_RingAtom, true));
      }
    }
    for (size_t i = 0; i < au.AtomCount(); i++) {
      TCAtom &a = au.GetAtom(i);
      if (a.IsDeleted() || a.GetType() != iHydrogenZ) {
        continue;
      }
      for (size_t j = 0; j < a.AttachedSiteCount(); j++) {
        a.GetAttachedAtom(j).SetHAttached(true);
      }
    }
    for (size_t i = 0; i < au.AtomCount(); i++) {
      TCAtom &a = au.GetAtom(i);
      if (a.IsDeleted() || a.GetType() != iNitrogenZ) {
        continue;
      }
      TCAtom *h_atom = 0;
      for (size_t j = 0; j < a.AttachedSiteCount(); j++) {
        TCAtom &aa = a.GetAttachedAtom(j);
        if (!aa.IsDeleted() && aa.GetType() == iHydrogenZ) {
          h_atom = &aa;
          break;
        }
      }
      if (h_atom == 0) {
        continue;
      }
      bool conflict = false;
      for (size_t j = 0; j < a.AttachedSiteICount(); j++) {
        TCAtom::Site &as = a.GetAttachedSiteI(j);
        if (as.atom->GetType() == iOxygenZ) {
          if(as.atom->IsHAttached()) {
            conflict = true;
            break;
          }
        }
      }
      if (conflict) {
        h_atom->SetDeleted(true);
        TBasicApp::NewLogEntry(logInfo) << "Removing H from ring N atom: " << a.GetLabel();
      }
    }
    au.DetachAtomType(iQPeakZ, false);
  }

  olx_del_obj(XApp.FixHL());
  if (TXApp::DoUseSafeAfix()) {
    XApp.GetUndo().Push(
      XApp.XFile().GetLattice().ValidateHGroups(true, true));
  }
}
//.............................................................................
void XLibMacros::macHImp(TStrObjList& Cmds, const TParamList& Options,
  TMacroData& Error)
{
  TXApp& XApp = TXApp::GetInstance();
  if (XApp.XFile().GetLattice().IsGenerated()) {
    Error.ProcessingError(__OlxSrcInfo,
      "The procedure is not applicable for the grown structure");
    return;
  }
  bool increase = false,
    decrease = false;
  if (!Cmds[0].IsNumber()) {
    Error.ProcessingError(__OlxSrcInfo,
      "first arument should be a number or +/- number");
    return;
  }
  double val = Cmds[0].ToDouble();
  if (Cmds[0].CharAt(0) == '+') {
    increase = true;
  }
  else if (Cmds[0].CharAt(0) == '-') {
    decrease = true;
  }
  Cmds.Delete(0);

  TSAtomPList satoms = XApp.FindSAtoms(Cmds, true);
  const double delta = XApp.XFile().GetLattice().GetDelta();
  for (size_t i = 0; i < satoms.Count(); i++) {
    if (satoms[i]->GetType() != iHydrogenZ)
      continue;
    TSAtom& h = *satoms[i], * attached = 0;
    size_t ac = 0;
    for (size_t j = 0; j < h.NodeCount(); j++) {
      TSAtom& n = h.Node(j);
      if (!(n.IsDeleted() || n.GetType().z < 2)) {
        ac++;
        attached = &n;
      }
    }
    if (ac > 1 || ac == 0) {
      XApp.NewLogEntry() << "Skipping " << h.GetLabel();
      continue;
    }
    vec3d v(h.crd() - attached->crd());
    if (increase || decrease)
      v.NormaliseTo(v.Length() + val);
    else
      v.NormaliseTo(val);
    v += attached->crd();
    double qd1 = v.QDistanceTo(attached->crd());
    double qd2 = attached->GetType().r_bonding + h.GetType().r_bonding + delta;
    qd2 *= qd2;
    if (qd1 >= qd2 - 0.01) {
      XApp.NewLogEntry() << "Skipping " << h.GetLabel();
      continue;
    }
    h.crd() = v;
    XApp.XFile().GetAsymmUnit().CartesianToCell(v);
    h.CAtom().ccrd() = v;
    h.ccrd() = v;
    if (h.CAtom().GetParentAfixGroup() != 0) {
      h.CAtom().GetParentAfixGroup()->SetD(sqrt(qd1));
    }
  }
  XApp.XFile().GetLattice().UpdateConnectivity();
  //XApp.XFile().EndUpdate();
}
//.............................................................................
void XLibMacros::macAnis(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TSAtomPList atoms = TXApp::GetInstance().FindSAtoms(Cmds, true);
  if (atoms.IsEmpty()) {
    return;
  }
  TCAtomPList catoms(atoms,
    FunctionAccessor::MakeConst(&TSAtom::CAtom));
  if (!Options.GetBoolOption('h')) {
    catoms.Pack(TCAtom::TypeAnalyser(iHydrogenZ));
  }
  catoms.Pack(TCAtom::TypeAnalyser(iQPeakZ));
  TXApp::GetInstance().XFile().GetLattice().SetAnis(
    catoms, true, Options.GetBoolOption('a'));
}
//.............................................................................
void XLibMacros::macIsot(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (Options.Contains("npd")) {
    TAsymmUnit &au = TXApp::GetInstance().XFile().GetAsymmUnit();
    TCAtomPList catoms;
    for (size_t i = 0; i < au.AtomCount(); i++) {
      TCAtom &a = au.GetAtom(i);
      if (a.GetEllipsoid() != NULL && a.GetEllipsoid()->IsNPD())
        catoms.Add(a);
    }
    TXApp::GetInstance().XFile().GetLattice().SetAnis(catoms, false);
    return;
  }
  TSAtomPList atoms = TXApp::GetInstance().FindSAtoms(Cmds, true);
  TCAtomPList catoms(atoms,
    FunctionAccessor::MakeConst(&TSAtom::CAtom));
  TXApp::GetInstance().XFile().GetLattice().SetAnis(catoms, false);
}
//.............................................................................
void XLibMacros::macFix(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olxstr vars(Cmds[0]);
  Cmds.Delete(0);
  double var_val = 0;
  if (!Cmds.IsEmpty() && Cmds[0].IsNumber()) {
    var_val = Cmds[0].ToDouble();
    Cmds.Delete(0);
  }
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms = xapp.FindSAtoms(Cmds, true, true);
  if (atoms.IsEmpty()) {
    return;
  }

  if (vars.Equalsi("XYZ")) {
    for (size_t i = 0; i < atoms.Count(); i++) {
      for (short j = 0; j < 3; j++) {
        xapp.XFile().GetRM().Vars.FixParam(atoms[i]->CAtom(), catom_var_name_X + j);
      }
    }
  }
  else if (vars.Equalsi("UISO")) {
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i]->GetEllipsoid() == 0) {// isotropic atom
        xapp.SetAtomUiso(*atoms[i], var_val);
        xapp.XFile().GetRM().Vars.FixParam(atoms[i]->CAtom(), catom_var_name_Uiso);
      }
      else {
        for (short j = 0; j < 6; j++) {
          xapp.XFile().GetRM().Vars.FixParam(atoms[i]->CAtom(), catom_var_name_U11 + j);
        }
      }
    }
  }
  else if (vars.Equalsi("OCCU")) {
    const ASObjectProvider& objects = xapp.XFile().GetLattice().GetObjects();
    objects.atoms.ForEach(ACollectionItem::TagSetter(0));
    XVar *var = 0;
    int rel = relation_None;
    if (olx_abs(var_val) > 10) {
      rel = var_val < 0 ? relation_AsOneMinusVar : relation_AsVar;
      int var_n = (int)(olx_abs(var_val) / 10) - 1;
      var_val = olx_abs(var_val) - var_n * 10;
      var = &xapp.XFile().GetRM().Vars.GetReferencedVar(var_n);
    }
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i]->GetTag() != 0) {
        continue;
      }
      TSAtomPList neighbours;
      neighbours.Add(atoms[i]);
      for (size_t j = 0; j < atoms[i]->NodeCount(); j++) {
        TSAtom& n = atoms[i]->Node(j);
        if (n.IsDeleted() || n.GetType() != iHydrogenZ || n.GetTag() != 0) {
          continue;
        }
        neighbours.Add(n);
      }
      for (size_t j = 0; j < neighbours.Count(); j++) {
        if (neighbours[j]->CAtom().GetPart() != atoms[i]->CAtom().GetPart()) {
          continue;
        }
        neighbours[j]->SetTag(1);
        xapp.XFile().GetRM().Vars.FixParam(neighbours[j]->CAtom(), catom_var_name_Sof);
        if (var_val == 0) {
          if (neighbours[j]->CAtom().GetPart() == 0)  // else leave as it is
            neighbours[j]->CAtom().SetOccu(1. / neighbours[j]->CAtom().GetDegeneracy());
        }
        else {
          if (var == 0) {
            neighbours[j]->CAtom().SetOccu(
              var_val / neighbours[j]->CAtom().GetDegeneracy());
          }
          else {
            xapp.XFile().GetRM().Vars.AddVarRef(*var, neighbours[j]->CAtom(),
              catom_var_name_Sof, rel, var_val);
          }
        }
      }
    }
  }
}
//.............................................................................
void XLibMacros::macFree(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olxstr vars = Cmds[0];
  Cmds.Delete(0);
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms = xapp.FindSAtoms(Cmds, true, true);
  if (atoms.IsEmpty()) {
    return;
  }
  if (vars.Containsi("XYZ")) {
    for (size_t i=0; i < atoms.Count(); i++) {
      for (short j = 0; j < 3; j++) {
        xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(), catom_var_name_X + j);
      }
    }
  }
  if (vars.Containsi("UISO")) {
    for (size_t i=0; i < atoms.Count(); i++) {
      if (atoms[i]->CAtom().GetEllipsoid() == NULL) {  // isotropic atom
        xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(),
          catom_var_name_Uiso);
      }
      else {
        for (short j = 0; j < 6; j++) {
          xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(),
            catom_var_name_U11 + j);
        }
      }
      if (atoms[i]->CAtom().GetUisoOwner() != NULL) {
        TAfixGroup *ag = atoms[i]->CAtom().GetParentAfixGroup();
        if (ag != NULL && ag->GetAfix() == -1) {
          ag->RemoveDependent(atoms[i]->CAtom());
        }
        atoms[i]->CAtom().SetUisoOwner(NULL);
      }
    }
  }
  if (vars.Containsi( "OCCU")) {
    xapp.XFile().GetAsymmUnit().GetAtoms().ForEach(
      ACollectionItem::TagSetter(0));
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i]->CAtom().GetTag() != 0) {
        continue;
      }
      xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(),
        catom_var_name_Sof);
      atoms[i]->CAtom().SetTag(1);
    }
    // now check if the H atoms are freed
    for (size_t i = 0; i < atoms.Count(); i++) {
      TCAtom &a = atoms[i]->CAtom();
      TCAtomPList hs;
      for (size_t j = 0; j < a.AttachedSiteCount(); j++) {
        TCAtom &aa = a.GetAttachedAtom(j);
        if (aa.GetType() == iHydrogenZ && aa.GetTag() == 0) {
          if (aa.GetParentAfixGroup() != NULL &&
            aa.GetParentAfixGroup()->GetPivot() == a)
          {
            hs << aa;
          }
        }
      }
      if (!hs.IsEmpty()) {
        RefinementModel &rm = xapp.XFile().GetRM();
        XVar &v = rm.Vars.NewVar(atoms[i]->CAtom().GetOccu());
        rm.Vars.AddVarRef(v, atoms[i]->CAtom(),
          catom_var_name_Sof, relation_AsVar, 1);
        for (size_t j = 0; j < hs.Count(); j++) {
          rm.Vars.AddVarRef(v, *hs[j], catom_var_name_Sof, relation_AsVar, 1);
        }
      }
    }
  }
}
//.............................................................................
void XLibMacros::macFixHL(TStrObjList &Cmds, const TParamList &Options, TMacroData &E)  {
  TXApp & xapp = TXApp::GetInstance();
  TAsymmUnit &au = xapp.XFile().GetAsymmUnit();
  TEBitArray detached(au.AtomCount());
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom &ca = au.GetAtom(i);
    detached.Set(i, ca.IsDetached());
    if (ca.GetType() == iQPeakZ) {
      ca.SetDetached(true);
    }
    else if (ca.GetType().z < 2) {
      ca.SetDetached(false);
    }
  }
  TActionQueueLock q_draw(xapp.FindActionQueue(olxappevent_GL_DRAW));
  xapp.XFile().GetLattice().UpdateConnectivity();
  delete TXApp::GetInstance().FixHL();
  for (size_t i = 0; i < au.AtomCount(); i++) {
    au.GetAtom(i).SetDetached(detached[i]);
  }
  xapp.XFile().GetLattice().UpdateConnectivity();
}
//.............................................................................
// http://www.minsocam.org/ammin/AM78/AM78_1104.pdf
int macGraphPD_Sort(const olx_pair_t<double,double> &a1,
  const olx_pair_t<double,double> &a2)
{
  return olx_cmp(a1.GetA(), a2.GetA());
}
void XLibMacros::macGraphPD(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& xapp = TXApp::GetInstance();
  RefinementModel &rm = xapp.XFile().GetRM();
  const double half_lambda = rm.expl.GetRadiation() / 2.0;
  double res = Options.FindValue("r", "0.1").ToDouble(),
    tt = 50;
  TUnitCell::SymmSpace sp =
    rm.aunit.GetLattice().GetUnitCell().GetSymmSpace();
  SymmSpace::InfoEx info_ex = SymmSpace::Compact(sp);
  double d = half_lambda / sin(tt*M_PI / 360),
    ds_sq = olx_sqr(1. / d);
  olx_pair_t<vec3i, vec3i> range = rm.CalcIndicesToD(d,
    &info_ex);
  mat3d h2c = xapp.XFile().GetAsymmUnit().GetHklToCartesian();
  TRefList refs;
  for (int h = range.a[0]; h <= range.b[0]; h++) {
    for (int k = range.a[1]; k <= range.b[1]; k++) {
      for (int l = range.a[2]; l <= range.b[2]; l++) {
        if (h == 0 && k == 0 && l == 0) continue;
        vec3i hkl(h, k, l);
        vec3i shkl = TReflection::Standardise(hkl, info_ex);
        if (shkl != hkl) continue;
        if (TReflection::IsAbsent(hkl, info_ex)) {
          continue;
        }
        double qd = TReflection::ToCart(hkl, h2c).QLength();
        if (qd <= ds_sq)
          refs.AddCopy(shkl);
      }
    }
  }

  TArrayList<compd> F(refs.Count());
  SFUtil::CalcSF(xapp.XFile(), MillerIndexList<TRefList>(refs), F);
  TEFile out(TEFile::ExtractFilePath(xapp.XFile().GetFileName()) <<
    "olx_pd_calc.csv", "w+b");
  TTypeList< olx_pair_t<double,double> > gd;
  gd.SetCapacity(refs.Count());
  double min_2t = 1000;
  double max_2t = 0;
  for (size_t i = 1; i < refs.Count(); i++)  {
    vec3d hkl = refs[i].ToCart(h2c);
    const double theta = asin(half_lambda*hkl.Length()),
      theta_2 = theta*2;
    const double lp = (1.0 + olx_sqr(cos(theta_2))) /
      (olx_sqr(sin(theta))*cos(theta));
    olx_update_min_max(gd.AddNew(theta_2 * 180 / M_PI,
      F[i].qmod()*lp).a,
      min_2t, max_2t);
  }
  const double sig_0 = 1./80. + (max_2t-min_2t)/800.0;
  const size_t ref_cnt = gd.Count();
  for (double s = 0; s <= max_2t; s += res) {
    if (s < min_2t) {
      out.Writeln(olxcstr(s, 40) << ',' << "0.001");
      continue;
    }
    double y = 0.0001;
    for (size_t i=0; i < ref_cnt; i++) {
      const double sig = sig_0*(1.0+gd[i].GetA()/140.0);
      y += gd[i].GetB()*exp(-olx_sqr((s-gd[i].GetA())/sig)/2)/sig;
    }
    out.Writeln(olxcstr(s, 40) << ',' << y);
  }
}
//.............................................................................
void XLibMacros::macFile(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& XApp = TXApp::GetInstance();
  olxstr Tmp;
  if (Cmds.IsEmpty()) {  // res -> Ins rotation if ins file
    if (XApp.CheckFileType<TIns>()) {
      Tmp = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "ins");
    }
    else {
      Tmp = XApp.XFile().GetFileName();
    }
  }
  else {
    Tmp = Cmds[0];
  }

  if (!TEFile::IsAbsolutePath(Tmp)) {
    olxstr root = (CurrentDir().IsEmpty() ? TEFile::CurrentDir()
      : CurrentDir());
    Tmp = TEFile::ExpandRelativePath(Tmp, root);
  }
  TEBitArray removedSAtoms, removedCAtoms;
  // kill Q peak in the ins file
  if (TEFile::ExtractFileExt(Tmp).Equalsi("ins")) {
    ASObjectProvider& objects = XApp.XFile().GetLattice().GetObjects();
    removedSAtoms.SetSize(objects.atoms.Count());
    for (size_t i = 0; i < objects.atoms.Count(); i++) {
      TSAtom& sa = objects.atoms[i];
      if (sa.GetType() == iQPeakZ && !sa.IsDeleted()) {
        sa.SetDeleted(true);
        removedSAtoms.SetTrue(i);
      }
    }
    TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
    removedCAtoms.SetSize(au.AtomCount());
    for (size_t i = 0; i < au.AtomCount(); i++) {
      TCAtom& ca = au.GetAtom(i);
      if (ca.GetType() == iQPeakZ && !ca.IsDeleted()) {
        ca.SetDeleted(true);
        removedCAtoms.SetTrue(i);
      }
    }
  }
  if (Options.GetBoolOption('s')) {
    olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
    if (op != 0) {
      op->processMacro("sort +ml moiety");
    }
  }
  olxstr op = Options.FindValue('p');
  if (op.IsInt()) {
    TOlxVars::SetVar("file_output_precision", op);
  }
  if (Tmp.EndsWith(".xml") || Tmp.EndsWith(".xld")) {
    if (!XApp.CheckFileType<TCif>()) {
      Error.ProcessingError(__OlxSourceInfo, "CIF is expected");
      return;
    }
    TDataFile df;
    XApp.XFile().GetLastLoader<TCif>().ToDataItem(df.Root().AddItem("CIF"));
    if (Tmp.EndsWith(".xml")) {
      df.SaveToXMLFile(Tmp);
    }
    else {
      df.SaveToXLFile(Tmp);
    }
  }
  else {
    XApp.XFile().SaveToFile(Tmp, Options.GetBoolOption('a') ? 0 : 1);
  }
  if (!removedSAtoms.IsEmpty()) {  // need to restore, a bit of mess here...
    ASObjectProvider& objects = XApp.XFile().GetLattice().GetObjects();
    for (size_t i = 0; i < objects.atoms.Count(); i++) {
      if (removedSAtoms.Get(i)) {
        objects.atoms[i].SetDeleted(false);
      }
    }
    TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
    for (size_t i = 0; i < au.AtomCount(); i++) {
      if (removedCAtoms[i]) {
        au.GetAtom(i).SetDeleted(false);
      }
    }
  }
}
//.............................................................................
void XLibMacros::macFuse(TStrObjList &Cmds, const TParamList &Options, TMacroData &E) {
  if (Cmds.Count() == 1 && Cmds[0].IsNumber()) {
    const double th = Cmds[0].ToDouble();
    TLattice& latt = TXApp::GetInstance().XFile().GetLattice();
    ASObjectProvider& objects = latt.GetObjects();
    for (size_t i = 0; i < objects.atoms.Count(); i++) {
      TSAtom& sa = objects.atoms[i];
      if (sa.IsDeleted()) {
        continue;
      }
      if (sa.BondCount() == 0) {
        continue;
      }
      sa.SortBondsByLengthAsc();
      vec3d cnt(sa.crd());
      size_t ac = 1;
      for (size_t j = 0; j < sa.BondCount(); j++) {
        if (sa.Bond(j).Length() < th) {
          TSAtom& asa = sa.Bond(j).Another(sa);
          if (asa.GetType() != sa.GetType()) {
            continue;
          }
          ac++;
          cnt += asa.crd();
          asa.CAtom().SetDeleted(true);
          asa.SetDeleted(true);
        }
        else {
          break;
        }
      }
      if (ac > 1) {
        cnt /= ac;
        sa.CAtom().ccrd() = latt.GetAsymmUnit().CartesianToCell(cnt);
      }
    }
    TXApp::GetInstance().XFile().GetLattice().Uniq();
  }
  else {
    TXApp::GetInstance().XFile().GetLattice().Uniq();
  }
}
//.............................................................................
void XLibMacros::macLstIns(TStrObjList &Cmds, const TParamList &Options, TMacroData &E) {
  bool remarks = Options.Contains("r");
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  TBasicApp::NewLogEntry() << "List of current instructions:";
  olxstr Tmp;
  for( size_t i=0; i < Ins.InsCount(); i++ )  {
    if( !remarks && Ins.InsName(i).Equalsi("REM") )  continue;
    Tmp = i;
    Tmp.RightPadding(3, ' ', true);
    Tmp << Ins.InsName(i) << ' ' << Ins.InsParams(i).Text(' ');
    TBasicApp::NewLogEntry() << Tmp;
  }
}
//.............................................................................
void XLibMacros::macAddIns(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (!TXApp::GetInstance().CheckFileType<TIns>()) {
    return;
  }
  /* if instruction is parsed, it goes to current model, otherwise it stays in
  the ins file
  */
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  if (!Ins.AddIns(TStrList(Cmds), TXApp::GetInstance().XFile().GetRM()) &&
    !Options.GetBoolOption('q'))
  {
    Error.ProcessingError(__OlxSrcInfo,
      olxstr("could not add instruction: ") << Cmds.Text(' '));
    return;
  }
}
//.............................................................................
void XLibMacros::macDelIns(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (!TXApp::GetInstance().CheckFileType<TIns>()) return;
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  if (Cmds[0].IsNumber()) {
    int insIndex = Cmds[0].ToInt();
    Ins.DelIns(insIndex);
  }
  else {
    RefinementModel &rm = TXApp::GetInstance().XFile().GetRM();
    if (Cmds[0].Equalsi("OMIT")) {
      rm.ClearOmits();
      rm.ClearOmit();
    }
    else if (Cmds[0].Equalsi("SHEL")) {
      rm.ClearShell();
    }
    else if (Cmds[0].Equalsi("TWIN")) {
      rm.RemoveTWIN();
    }
    else if (Cmds[0].Equalsi("BASF")) {
      rm.Vars.ClearBASF();
    }
    else if (Cmds[0].Equalsi("EXTI")) {
      rm.Vars.ClearEXTI();
    }
    else if (Cmds[0].Equalsi("HTAB")) {
      rm.ClearInfoTab("HTAB");
    }
    else if (Cmds[0].Equalsi("RTAB")) {
      rm.ClearInfoTab("RTAB");
    }
    else if (Cmds[0].Equalsi("BOND")) {
      rm.ClearInfoTab("BOND");
    }
    else if (Cmds[0].Equalsi("MPLA")) {
      rm.ClearInfoTab("MPLA");
    }
    else if (Cmds[0].Equalsi("CONF")) {
      rm.ClearInfoTab("CONF");
    }
    for (size_t i = 0; i < Ins.InsCount(); i++) {
      if (Ins.InsName(i).Equalsi(Cmds[0])) {
        Ins.DelIns(i--);
        continue;
      }
    }
  }
  OnDelIns().Exit(NULL, &Cmds[0]);
}
//.............................................................................
void XLibMacros::macLS(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  int ls = -1;
  XLibMacros::Parse(Cmds, "i", &ls);
  if (ls != -1)
    TXApp::GetInstance().XFile().GetRM().SetIterations((int)ls);
  if (!Cmds.IsEmpty())
    TXApp::GetInstance().XFile().GetRM().SetRefinementMethod(Cmds[0]);
}
//.............................................................................
void XLibMacros::macUpdateWght(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  if (Cmds.IsEmpty() && rm.proposed_weight.IsEmpty()) return;
  if (Cmds.IsEmpty())
    rm.used_weight = rm.proposed_weight;
  else {
    rm.used_weight.SetCount(Cmds.Count());
    for (size_t i=0; i < Cmds.Count(); i++)
      rm.used_weight[i] = Cmds[i].ToDouble();
  }
}
//.............................................................................
void XLibMacros::macUser(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error)  {
  if (Cmds.IsEmpty()) {
    TBasicApp::NewLogEntry() << TEFile::CurrentDir();
  }
  else if (!TEFile::ChangeDir(Cmds[0])) {
    Error.ProcessingError(__OlxSrcInfo, "could not change current folder");
  }
  else
    CurrentDir() = Cmds[0];
}
//.............................................................................
void XLibMacros::macDir(TStrObjList &Cmds, const TParamList &Options, TMacroData &Error)  {
  olxstr Filter(Cmds.IsEmpty() ? olxstr("*.*")  : Cmds[0]);
  TFileList fl;
  TEFile::ListCurrentDirEx(fl, Filter, sefFile|sefDir);
  TTTable<TStrList> tab(fl.Count(), 4);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Size";
  tab.ColName(2) = "Last Modified";
  tab.ColName(3) = "Attributes";

  TFileListItem::SortListByName(fl);

  for( size_t i=0; i < fl.Count(); i++ )  {
    tab[i][0] = fl[i].GetName();
    if( (fl[i].GetAttributes() & sefDir) != 0 )
      tab[i][1] = "Folder";
    else
      tab[i][1] = fl[i].GetSize();
    tab[i][2] = TETime::FormatDateTime("yyyy.MM.dd hh:mm:ss", fl[i].GetModificationTime());
    if( (fl[i].GetAttributes() & sefReadOnly) != 0 )
      tab[i][3] << 'r';
    if( (fl[i].GetAttributes() & sefWriteOnly) != 0 )
      tab[i][3] << 'w';
    if( (fl[i].GetAttributes() & sefHidden) != 0 )
      tab[i][3] << 'h';
    if( (fl[i].GetAttributes() & sefSystem) != 0 )
      tab[i][3] << 's';
    if( (fl[i].GetAttributes() & sefExecute) != 0 )
      tab[i][3] << 'e';
  }
  TBasicApp::NewLogEntry() << tab.CreateTXTList("Directory list", true, true, ' ');
}
//.............................................................................
void XLibMacros::macLstMac(TStrObjList &Cmds, const TParamList &Options, TMacroData &E)  {
  // create masks
  TTypeList<TStrMask> masks;
  for( size_t i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString());
  // end masks creation
  TBasicFunctionPList macros;
  TXApp::GetInstance().GetLibrary().ListAllMacros(macros);
  for( size_t i=0; i < macros.Count(); i++ )  {
    ABasicFunction* func = macros[i];
    bool add = false;
    olxstr fn = func->GetQualifiedName();
    for( size_t j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(fn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    TBasicApp::NewLogEntry() << fn << " - " << func->GetSignature();
  }
}
//.............................................................................
void XLibMacros::macLstFun(TStrObjList &Cmds, const TParamList &Options, TMacroData &E)  {
  // create masks
  TTypeList<TStrMask> masks;
  for( size_t i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString());
  // end masks creation
  TBasicFunctionPList functions;
  TXApp::GetInstance().GetLibrary().ListAllFunctions(functions);
  for( size_t i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    bool add = false;
    olxstr fn = func->GetQualifiedName();
    for( size_t j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(fn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    TBasicApp::NewLogEntry() << fn << " - " << func->GetSignature();
  }
}
//.............................................................................
void XLibMacros::ChangeCell(const mat3d& tm, const TSpaceGroup& new_sg,
  const olxstr& resHKL_FN)
{
  TXApp& xapp = TXApp::GetInstance();
  TBasicApp::NewLogEntry() << "Cell choice trasformation matrix:" <<
    NewLineSequence() << tm[0].ToString() << NewLineSequence() <<
    tm[1].ToString() << NewLineSequence() <<
    tm[2].ToString();
  TBasicApp::NewLogEntry() << "New space group: " << new_sg.GetName();
  const mat3d tm_t(mat3d::Transpose(tm));
  xapp.XFile().UpdateAsymmUnit();
  TAsymmUnit& au = xapp.XFile().LastLoader()->GetAsymmUnit();
  const mat3d i_tm(tm.Inverse());
  const mat3d f2c = mat3d::Transpose((
    mat3d::Transpose(au.GetCellToCartesian())*tm));
  mat3d ax_err;
  ax_err[0] = vec3d::Qrt(au.GetAxisEsds());
  ax_err[1] = ax_err[0];  ax_err[2] = ax_err[0];
  mat3d an_err;
  an_err[0] = vec3d::Qrt(au.GetAngleEsds());
  an_err[1] = an_err[0];  an_err[2] = an_err[0];
  // prepare positive matrix for error estimation
  mat3d tm_p(tm);
  for( size_t i=0; i < 3; i++ )
    for( size_t j=0; j < 3; j++ )
      tm_p[i][j] = olx_abs(tm_p[i][j]);
  ax_err *= tm_p;
  an_err *= tm_p;
  au.GetAxes()[0] = f2c[0].Length();  au.GetAxisEsds()[0] = sqrt(ax_err[0][0]);
  au.GetAxes()[1] = f2c[1].Length();  au.GetAxisEsds()[1] = sqrt(ax_err[1][1]);
  au.GetAxes()[2] = f2c[2].Length();  au.GetAxisEsds()[2] = sqrt(ax_err[2][2]);
  au.GetAngles()[0] = acos(f2c[1].CAngle(f2c[2]))*180.0/M_PI;
  au.GetAngleEsds()[0] = sqrt(an_err[0][0]);
  au.GetAngles()[1] = acos(f2c[0].CAngle(f2c[2]))*180.0/M_PI;
  au.GetAngleEsds()[1] = sqrt(an_err[1][1]);
  au.GetAngles()[2] = acos(f2c[0].CAngle(f2c[1]))*180.0/M_PI;
  au.GetAngleEsds()[2] = sqrt(an_err[2][2]);
  const mat3d old_cac = au.GetCartesianToCell();
  au.InitMatrices();
  const mat3d elptm = mat3d::Transpose(au.GetCellToCartesian())
    *i_tm*mat3d::Transpose(old_cac);
  ematd J = TEllipsoid::GetTransformationJ(elptm),
    Jt = ematd::Transpose(J);
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    ca.ccrd() = i_tm * ca.ccrd();
    if( ca.GetEllipsoid() != NULL )
      ca.GetEllipsoid()->Mult(elptm, J, Jt);
  }
  TBasicApp::NewLogEntry() << "New cell: " <<
    TEValueD(au.GetAxes()[0], au.GetAxisEsds()[0]).ToString() << ' ' <<
    TEValueD(au.GetAxes()[1], au.GetAxisEsds()[1]).ToString() << ' ' <<
    TEValueD(au.GetAxes()[2], au.GetAxisEsds()[2]).ToString() << ' '  <<
    TEValueD(au.GetAngles()[0], au.GetAngleEsds()[0]).ToString() << ' '  <<
    TEValueD(au.GetAngles()[1], au.GetAngleEsds()[1]).ToString() << ' '  <<
    TEValueD(au.GetAngles()[2], au.GetAngleEsds()[2]).ToString();
  TBasicApp::NewLogEntry(logError) << "Cell esd's are estimated!";
  bool save = false;
  if (!resHKL_FN.IsEmpty()) {
    olxstr hkl_fn(xapp.XFile().LocateHklFile());
    if (!hkl_fn.IsEmpty()) {
      THklFile hklf;
      hklf.LoadFromFile(hkl_fn, false);
      for (size_t i=0; i < hklf.RefCount(); i++)
        hklf[i].SetHkl((tm_t * vec3d(hklf[i].GetHkl())).Round<int>());
      hklf.SaveToFile(resHKL_FN);
      xapp.XFile().GetRM().SetHKLSource(resHKL_FN);
      save = true;
    }
    else {
      TBasicApp::NewLogEntry(logError) << "Could not locate source HKL file";
    }
  }
  else  {
    const mat3d hklf_mat = tm_t*xapp.XFile().LastLoader()->GetRM().GetHKLF_mat();
    xapp.XFile().LastLoader()->GetRM().SetHKLF_mat(hklf_mat);
  }
  au.ChangeSpaceGroup(new_sg);
  au.InitMatrices();
  xapp.XFile().LastLoaderChanged();
  if (save)
    xapp.XFile().SaveToFile(xapp.XFile().GetFileName());
}
//.............................................................................
TSpaceGroup* XLibMacros_macSGS_FindSG(TPtrList<TSpaceGroup>& sgs,
  const olxstr& axis)
{
  for (size_t i = 0; i < sgs.Count(); i++) {
    if (sgs[i]->GetAxis().Compare(axis) == 0) {
      return sgs[i];
    }
  }
  return 0;
}
olxstr XLibMacros_macSGS_SgInfo(const olxstr& caxis)  {
  if( caxis.IsEmpty() )
    return "standard";
  else  {
    // -axis + cell choice
    if (caxis.Length() == 3 && caxis.CharAt(0) == '-') {
      return olxstr("axis: -") << caxis.CharAt(1) << ", cell choice "
        << caxis.CharAt(2);
    }
    // axis + cell choice
    else if (caxis.Length() == 2) {
      return olxstr("axis: ") << caxis.CharAt(0) << ", cell choice "
        << caxis.CharAt(1);
    }
    else {
      return olxstr("axis: ") << caxis;
    }
  }
}
void XLibMacros::macSGS(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& xapp = TXApp::GetInstance();
  olxstr hkl_fn;
  if (Cmds.Count() > 1 && Cmds.GetLastString().EndsWithi(".hkl")) {
    hkl_fn = Cmds.GetLastString();
    Cmds.Delete(Cmds.Count()-1);
  }
  if (Cmds.Count() == 10) {  // transformation provided?
    TSpaceGroup* sg = TSymmLib::GetInstance().FindGroupByName(Cmds[9]);
    if (sg == NULL) {
      E.ProcessingError(__OlxSrcInfo, "undefined space group");
      return;
    }
    mat3d tm;
    for (int i=0; i < 9; i++)
      tm[i/3][i%3] = Cmds[i].ToDouble();
    if (!tm.IsI()) {
      ChangeCell(tm, *sg, hkl_fn);
      TBasicApp::NewLogEntry() << "The cell, atomic coordinates and ADP's are "
        "transformed using user transform";
    }
    return;
  }
  TSpaceGroup* sg_ = Cmds.Count() == 1 ? &xapp.XFile().GetLastLoaderSG() :
    TSymmLib::GetInstance().FindGroupByName(Cmds[1]);
  if (sg_ == NULL)  {
    E.ProcessingError(__OlxSrcInfo, "undefined space group");
    return;
  }
  TSpaceGroup &sg = *sg_;
  SGSettings sg_set(sg);
  olxstr axis = sg_set.axisInfo.GetAxis();
  TBasicApp::NewLogEntry() << "Current setting: " <<
    XLibMacros_macSGS_SgInfo(axis);
  if( axis.IsEmpty() )  {
    TBasicApp::NewLogEntry() << "Nothing to do";
    return;
  }
  const TSymmLib& sl = TSymmLib::GetInstance();
  TPtrList<TSpaceGroup> sgs;
  sl.GetGroupByNumber(sg.GetNumber(), sgs);
  for( size_t i=0; i < sgs.Count(); i++ )  {
    if( &sg != sgs[i] ) {
      TBasicApp::NewLogEntry() << "Possible: " <<
        XLibMacros_macSGS_SgInfo(sgs[i]->GetAxis());
    }
  }
  AxisInfo n_ai(sg, Cmds[0]);
  if( sg_set.axisInfo.HasMonoclinicAxis() && !n_ai.HasMonoclinicAxis() )
    n_ai.ChangeMonoclinicAxis(sg_set.axisInfo.GetMonoclinicAxis());
  if( sg_set.axisInfo.HasCellChoice() && !n_ai.HasCellChoice() )
    n_ai.ChangeCellChoice(sg_set.axisInfo.GetCellChoice());
  if( sg_set.axisInfo.GetAxis() == n_ai.GetAxis() )  {
    TBasicApp::NewLogEntry() << "Nothing to change";
    return;
  }
  mat3d tm;
  if( sg_set.GetTrasformation(n_ai, tm) )  {
    TSpaceGroup* new_sg = XLibMacros_macSGS_FindSG(sgs, n_ai.GetAxis());
    if( new_sg == NULL && n_ai.GetAxis() == "abc" )
      new_sg = XLibMacros_macSGS_FindSG(sgs, EmptyString());
    if( new_sg == NULL )  {
      E.ProcessingError(__OlxSrcInfo,
        "Could not locate space group for given settings");
      return;
    }
    ChangeCell(tm, *new_sg, hkl_fn);
  }
  else  {
    E.ProcessingError(__OlxSrcInfo,
      "could not find appropriate transformation");
  }
}
//.............................................................................
void XLibMacros::macLstVar(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if( TOlxVars::VarCount() == 0 )  return;
  // create masks
  TTypeList<TStrMask> masks;
  for( size_t i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString());
  // end masks creation
  TTTable<TStrList> tab(TOlxVars::VarCount(), 3);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Value";
#ifdef _PYTHON_
  tab.ColName(2) = "RefCnt";
#endif
  size_t rowsCount = 0;
  for( size_t i=0; i < TOlxVars::VarCount(); i++ )  {
    bool add = false;
    const olxstr& vn = TOlxVars::GetVarName(i);
    for( size_t j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(vn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    tab[rowsCount][0] = vn;
    tab[rowsCount][1] = TOlxVars::GetVarStr(i);
#ifdef _PYTHON
    if( TOlxVars::GetVarWrapper(i) != NULL )
      tab[rowsCount][2] = TOlxVars::GetVarWrapper(i)->ob_refcnt;
    else
      tab[rowsCount][2] = NAString();
#endif
    rowsCount++;
  }
  tab.SetRowCount(rowsCount);
  TBasicApp::NewLogEntry() <<
    tab.CreateTXTList("Variables list", true, true, ' ');
}
//.............................................................................
void XLibMacros::macLstFS(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  // create masks
  TTypeList<TStrMask> masks;
  for( size_t i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString());
  // end masks creation
  double tc = 0;
  TTTable<TStrList> tab(TFileHandlerManager::Count(), 4);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Size";
  tab.ColName(2) = "Timestamp";
  tab.ColName(3) = "Persistent";
  size_t rowsAdded = 0;
  for( size_t i=0; i < TFileHandlerManager::Count(); i++ )  {
    bool add = false;
    const olxstr& bn = TFileHandlerManager::GetBlockName(i);
    for( size_t j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(bn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    tab[rowsAdded][0] = bn;
    tab[rowsAdded][1] = TFileHandlerManager::GetBlockSize(i);
    tab[rowsAdded][2] = TFileHandlerManager::GetBlockDateTime(i);
    tab[rowsAdded][3] = TFileHandlerManager::GetPersistenceId(i);
    tc += TFileHandlerManager::GetBlockSize(i);
    rowsAdded++;
  }
  tc /= (1024*1024);
  tab.SetRowCount(rowsAdded);
  TBasicApp::NewLogEntry() <<
    tab.CreateTXTList(olxstr("Virtual FS content"), true, false, "|");
  TBasicApp::NewLogEntry() << "Content size is " <<
    olxstr::FormatFloat(3, tc)  << "Mb";
}
//.............................................................................
void XLibMacros::macPlan(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  int plan = Cmds[0].ToInt();
  if( plan == -1 )  return; // leave like it is
  TXApp::GetInstance().XFile().GetRM().SetPlan(plan);
}
//.............................................................................
void XLibMacros::macFixUnit(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  double Zp = Cmds.IsEmpty() ? 1
    : (double)olx_round(Cmds[0].ToDouble() * 192) / 192;
  if (Zp <= 0) {
    Zp = 1;
  }
  TXFile &xf = TXApp::GetInstance().XFile();
  xf.UpdateAsymmUnit();
  TAsymmUnit& au = xf.GetAsymmUnit();
  TUnitCell& uc = xf.GetUnitCell();
  ContentList content = au.GetContentList();
  if (content.IsEmpty()) {
    content = xf.GetRM().GetUserContent();
  }
  const int Z_sg = (int)uc.MatrixCount();
  au.SetZ(Z_sg*Zp);
  olxstr n_c;
  for (size_t i = 0; i < content.Count(); i++) {
    n_c << ' ' << ElementCount::ToString(*content[i].element,
      content[i].count/Zp, content[i].charge);
    content[i].count = olx_round(content[i].count * Z_sg, 100);
  }
  TBasicApp::NewLogEntry() << "New content is:" << n_c;
  xf.GetRM().SetUserContent(content);
}
//.............................................................................
void XLibMacros::macGenDisp(TStrObjList& Cmds, const TParamList& Options,
  TMacroData& Error)
{
  const bool neutron = Options.GetBoolOption('n');
  const bool full = Options.GetBoolOption('f') || neutron;
  const bool force = Options.GetBoolOption("force");

  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  const ContentList& content = rm.GetUserContent();
  if (!force) {
    size_t cnt = 0;
    for (size_t i = 0; i < content.Count(); i++) {
      XScatterer* s = rm.FindSfacData(content[i].element->symbol);
      if (s != 0 && s->IsSet(XScatterer::setDispersion | XScatterer::setMu)) {
        continue;
      }
      cnt++;
    }
    if (cnt == 0) {
      return;
    }
  }

  const double en = rm.expl.GetRadiationEnergy();
  cm_Absorption_Coefficient_Reg ac;
  olxstr_dict<TDoubleList> ext_registry;
  olxstr source = Options.FindValue("source");
  if (!source.IsEmpty()) {
    olxstr fn = "spy.sfac.generate_DISP(";
    if (source.Equalsi("sasaki")) {
      fn << "sasaki)";
    }
    else if (source.Equalsi("henke")) {
      fn << "henke)";
    }
    else if (source.Equalsi("auto")) {
      fn << "auto)";
    }
    else {
      Error.ProcessingError(__OlxSrcInfo, "Unknown source");
      return;
    }
    if (!olex2::IOlex2Processor::GetInstance()->processFunction(fn, true)) {
      Error.ProcessingError(__OlxSrcInfo, "Failed to extract data from cctbx");
      return;
    }
    TStrList e_lines(fn, ';');
    for (size_t i = 0; i < e_lines.Count(); i++) {
      TStrList e_toks(e_lines[i], ',');
      if (e_toks.Count() >= 3) {
        TDoubleList& dl = ext_registry.Add(e_toks[0]);
        dl.Add(e_toks[1].ToDouble());
        dl.Add(e_toks[2].ToDouble());
        if (e_toks.Count() > 3) {
          dl.Add(e_toks[3].ToDouble());
        }
      }
    }
  }
  if (!full) {
    if (rm.SfacCount() > 0 && rm.GetSfacData(0).IsNeutron()) {
      TBasicApp::NewLogEntry() << "Skipping DISP generation for neutron data";
      return;
    }
    for (size_t i = 0; i < content.Count(); i++) {
      if (!force) {
        XScatterer* s = rm.FindSfacData(content[i].element->symbol);
        if (s != 0 && s->IsSet(XScatterer::setDispersion|XScatterer::setMu)) {
          continue;
        }
      }
      XScatterer* sc = new XScatterer(content[i].element->symbol);
      size_t ext_idx = ext_registry.IndexOf(content[i].element->symbol);
      bool mu_set = false;
      if (ext_idx != InvalidIndex) {
        const TDoubleList& dl = ext_registry.GetValue(ext_idx);
        sc->SetFpFdp(compd(dl[0], dl[1]));
        if (dl.Count() > 2) {
          sc->SetMu(dl[2]);
          mu_set = true;
        }
      }
      else {
        sc->SetFpFdp(content[i].element->CalcFpFdp(en) - content[i].element->z);
      }
      if (!mu_set) {
        try {
          double absorpc =
            ac.CalcMuOverRhoForE(en, ac.get(content[i].element->symbol));
          sc->SetMu(absorpc * content[i].element->GetMr() / 0.6022142);
        }
        catch (...) {
          TBasicApp::NewLogEntry() << "Could not locate absorption data for: " <<
            content[i].element->symbol;
        }
      }
      rm.AddSfac(*sc);
    }
  }
  else {
    for (size_t i = 0; i < content.Count(); i++) {
      if (!force) {
        XScatterer* s = rm.FindSfacData(content[i].element->symbol);
        if (s != 0 && s->IsSet(XScatterer::setAll)) {
          continue;
        }
      }
      XScatterer* sc = new XScatterer(*content[i].element, en);
      bool mu_set = false;
      if (neutron) {
        sc->SetFpFdp(compd(0, 0));
      }
      else {
        size_t ext_idx = ext_registry.IndexOf(content[i].element->symbol);
        if (ext_idx != InvalidIndex) {
          const TDoubleList& dl = ext_registry.GetValue(ext_idx);
          sc->SetFpFdp(compd(dl[0], dl[1]));
          if (dl.Count() > 2) {
            sc->SetMu(dl[2]);
            mu_set = true;
          }
        }
        else {
          sc->SetFpFdp(content[i].element->CalcFpFdp(en) - content[i].element->z);
        }
      }
      try {
        double absorpc =
          ac.CalcMuOverRhoForE(en, ac.get(content[i].element->symbol));
        CXConnInfo& ci = rm.Conn.GetConnInfo(*content[i].element);
        if (!mu_set) {
          sc->SetMu(absorpc * content[i].element->GetMr() / 0.6022142);
        }
        sc->SetR(ci.r);
        sc->SetWeight(content[i].element->GetMr());
        delete& ci;
      }
      catch (...) {
        TBasicApp::NewLogEntry() << "Could not locate absorption data for: " <<
          content[i].element->symbol;
      }
      if (neutron) {
        if (content[i].element->neutron_scattering == 0) {
          TBasicApp::NewLogEntry() << "Could not locate neutron data for: " <<
            content[i].element->symbol;
        }
        else {
          sc->SetGaussians(
            cm_Gaussians(0, 0, 0, 0, 0, 0, 0, 0,
              content[i].element->neutron_scattering->coh.GetRe()));
        }
      }
      rm.AddSfac(*sc);
    }
  }
}
//.............................................................................
void XLibMacros::macEXYZ(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms = xapp.FindSAtoms(TStrList(), false, true);
  if (atoms.IsEmpty()) {
    E.ProcessingError(__OlxSrcInfo, "No atoms provided");
    return;
  }
  if (atoms.Count() == 1 && Cmds.IsEmpty()) {
    E.ProcessingError(__OlxSrcInfo, "Please specify additional atom type(s)");
    return;
  }
  const bool set_eadp = Options.GetBoolOption("eadp", false, true);
  TCAtomPList processed;
  TPtrList<TExyzGroup> groups;
  RefinementModel& rm = xapp.XFile().GetRM();
  TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  TPtrList<cm_Element> elements;
  for (size_t i=0; i < Cmds.Count(); i++) {
    cm_Element* elm = XElementLib::FindBySymbol(Cmds[i]);
    if (elm == 0) {
      xapp.NewLogEntry(logError) << "Unknown element: " << Cmds[i];
      continue;
    }
    elements.AddUnique(elm);
  }
  if (atoms.Count() == 1) {
    elements.Remove(atoms[0]->GetType());
    if (elements.IsEmpty()) {
      E.ProcessingError(__OlxSrcInfo, "At least one more element type is expected");
      return;
    }
    if (atoms[0]->CAtom().GetExyzGroup() != 0) {
      atoms[0]->CAtom().GetExyzGroup()->Clear();
    }
    groups.Add(rm.ExyzGroups.New())->Add(atoms[0]->CAtom());
    for (size_t i=0; i < elements.Count(); i++) {
      TCAtom& ca = au.NewAtom();
      ca.ccrd() = atoms[0]->CAtom().ccrd();
      ca.SetLabel(elements[i]->symbol + atoms[0]->GetLabel().SubStringFrom(
        atoms[0]->GetType().symbol.Length()), false);
      ca.SetType(*elements[i]);
      ca.AssignEquivs(atoms[0]->CAtom());
      rm.Vars.FixParam(ca, catom_var_name_Sof);
      groups[0]->Add(ca);
      ca.SetUiso(atoms[0]->CAtom().GetUiso());
    }
    processed.Add(atoms[0]->CAtom());
  }
  // special case of atoms close to each other
  else if (atoms.Count() == 2 && Cmds.IsEmpty() &&
    atoms[0]->crd().DistanceTo(atoms[1]->crd()) < 1)
  {
    groups.Add(rm.ExyzGroups.New())->Add(atoms[0]->CAtom());
    groups.GetLast()->Add(atoms[1]->CAtom());
  }
  else if (Cmds.IsEmpty()) {  // model type swapping disorder
    sorted::PointerPointer<const cm_Element> group_types;
    for (size_t i=0; i < atoms.Count(); i++) {
      if (!group_types.AddUnique(&atoms[i]->GetType()).b) {
        E.ProcessingError(__OlxSrcInfo, "Unique atom types are expected");
        return;
      }
    }
    for (size_t i=0; i < atoms.Count(); i++) {
      if (atoms[i]->CAtom().GetExyzGroup() != NULL)
        atoms[i]->CAtom().GetExyzGroup()->Clear();
      groups.Add(rm.ExyzGroups.New())->Add(atoms[i]->CAtom());
      for (size_t j=1; j < atoms.Count(); j++) {
        TCAtom& ref = atoms[(i+j)%atoms.Count()]->CAtom();
        TCAtom& ca = au.NewAtom();
        ca.ccrd() = atoms[i]->ccrd();
        ca.SetLabel(ref.GetType().symbol + atoms[i]->GetLabel().SubStringFrom(
          atoms[i]->GetType().symbol.Length()), false);
        ca.SetType(ref.GetType());
        ca.AssignEquivs(ref);
        rm.Vars.FixParam(ca, catom_var_name_Sof);
        ca.SetUiso(ref.GetUiso());
        groups[i]->Add(ca);
      }
      processed.Add(atoms[i]->CAtom());
    }
  }
  // several atoms and types are provided
  else {
    const cm_Element *e = &atoms[0]->GetType();
    for (size_t i=1; i < atoms.Count(); i++) {
      if (e != &atoms[i]->GetType()) {
        E.ProcessingError(__OlxSrcInfo, "Atoms of the same type are expected");
        return;
      }
    }
    elements.Remove(e);
    if (elements.IsEmpty()) {
      E.ProcessingError(__OlxSrcInfo, "No new element types is provided");
      return;
    }
    for (size_t i=0; i < atoms.Count(); i++) {
      if (atoms[i]->CAtom().GetExyzGroup() != NULL)
        atoms[i]->CAtom().GetExyzGroup()->Clear();
      groups.Add(rm.ExyzGroups.New())->Add(atoms[i]->CAtom());
      for (size_t j=0; j < elements.Count(); j++) {
        TCAtom& ca = au.NewAtom();
        ca.ccrd() = atoms[i]->ccrd();
        ca.SetLabel(elements[j]->symbol + atoms[i]->GetLabel().SubStringFrom(
          atoms[i]->GetType().symbol.Length()), false);
        ca.SetType(*elements[j]);
        ca.AssignEquivs(atoms[i]->CAtom());
        rm.Vars.FixParam(ca, catom_var_name_Sof);
        ca.SetUiso(atoms[i]->CAtom().GetUiso());
        groups[i]->Add(ca);
      }
      processed.Add(atoms[i]->CAtom());
    }
  }
  bool groups_equal=true;
  const size_t group0_sz = groups[0]->Count();
  for (size_t i=1; i < groups.Count(); i++) {
    if (groups[i]->Count() != group0_sz) {
      groups_equal = false;
      break;
    }
  }
  if (groups_equal && group0_sz > 1) {
    if (group0_sz == 2) {
      XVar& vr = rm.Vars.NewVar();
      for (size_t i=0; i < groups.Count(); i++) {
        double k = 1.0 / (*groups[i])[0].GetDegeneracy();
          rm.Vars.AddVarRef(vr,
          (*groups[i])[0], catom_var_name_Sof, relation_AsVar, k);
        rm.Vars.AddVarRef(vr,
          (*groups[i])[1], catom_var_name_Sof, relation_AsOneMinusVar, k);
      }
    }
    else {
      XLEQ &leq = rm.Vars.NewEquation();
      for (size_t i=0; i < group0_sz; i++) {
        XVar& vr = rm.Vars.NewVar(1./group0_sz);
        leq.AddMember(vr);
        for (size_t j=0; j < groups.Count(); j++) {
          rm.Vars.AddVarRef(vr,
            (*groups[j])[i], catom_var_name_Sof, relation_AsVar,
            1.0/ (*groups[j])[i].GetDegeneracy());
        }
      }
    }
    if (set_eadp) {
      for (size_t i=0; i < groups.Count(); i++) {
        TSimpleRestraint& sr = rm.rEADP.AddNew();
        for (size_t j=0; j < groups[i]->Count(); j++)
          sr.AddAtom((*groups[i])[j], 0);
      }
    }
  }
  const int part = au.GetNextPart();
  for (size_t i=0; i < groups.Count(); i++) {
    for (size_t j=0; j < groups[i]->Count(); j++)
      (*groups[i])[j].SetPart((int8_t)(part+j));
  }
  // force the split atom to become isotropic
  xapp.XFile().GetLattice().SetAnis(processed, false);
  xapp.XFile().EndUpdate();
}
//.............................................................................
void XLibMacros::macEADP(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms = xapp.FindSAtoms(Cmds, false, true);
  if (atoms.Count() < 2) {
    E.ProcessingError(__OlxSrcInfo, "not enough atoms provided");
    return;
  }
  // validate that atoms of the same type
  bool allIso = (atoms[0]->GetEllipsoid() == NULL);
  for (size_t i = 1; i < atoms.Count(); i++) {
    if ((atoms[i]->GetEllipsoid() == NULL) != allIso) {
      E.ProcessingError(__OlxSrcInfo, "mixed atoms types (aniso and iso)");
      return;
    }
  }
  TSimpleRestraint& sr = xapp.XFile().GetRM().rEADP.AddNew();
  for (size_t i = 0; i < atoms.Count(); i++) {
    sr.AddAtom(atoms[i]->CAtom(), NULL);
  }
  xapp.XFile().GetRM().rEADP.ValidateRestraint(sr);
}
//.............................................................................
void XLibMacros::macAddSE(TStrObjList& Cmds, const TParamList& Options,
  TMacroData& E)
{
  TXApp& xapp = TXApp::GetInstance();
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  if (au.AtomCount() == 0) {
    E.ProcessingError(__OlxSrcInfo, "Empty asymmetric unit");
    return;
  }
  TSpaceGroup* sg = NULL;
  try { sg = &xapp.XFile().GetLastLoaderSG(); }
  catch (...) {
    E.ProcessingError(__OlxSrcInfo, "Could not identify current space group");
    return;
  }
  if (sg->IsCentrosymmetric() && Cmds.Count() == 1) {
    E.ProcessingError(__OlxSrcInfo, "Centrosymmetric space group");
    return;
  }
  if (Cmds.Count() == 1) {
    SymmSpace::Info si = sg->GetInfo();
    si.centrosymmetric = true;
    sg = TSymmLib::GetInstance().FindGroupByHallSymbol(
      HallSymbol::Evaluate(si), sg);
  }
  else if (Cmds.Count() == 2) {
    TSAtomPList atoms = xapp.FindSAtoms(TStrList());
    for (size_t i = 0; i < au.AtomCount(); i++) {
      for (size_t j = i + 1; j < au.AtomCount(); j++) {
        if (au.GetAtom(j).IsDeleted())  continue;
        double d = uc.FindClosestDistance(au.GetAtom(i), au.GetAtom(j));
        if (d < 0.5) {
          au.GetAtom(i).SetDeleted(true);
          break;
        }
      }
    }
    latt.Init();
    latt.Compaq();
    latt.CompaqAll();
    return;
  }
  TSymmTest st(uc);
  smatd m;
  m.r.I();
  double tol = 0.1;
  st.TestMatrix(m, tol);
  if (!st.GetResults().IsEmpty()) {
    size_t ind = st.GetResults().Count() - 1;
    double match = (double)(st.GetResults()[ind].Count() * 200 / st.AtomCount());
    while (match > 125 && tol > 1e-4) {
      tol /= 4;
      st.TestMatrix(m, tol);
      ind = st.GetResults().Count() - 1;
      match = st.GetResults().IsEmpty() ? 0.0
        : st.GetResults()[ind].Count() * 200 / st.AtomCount();
      continue;
    }
    if (st.GetResults().IsEmpty()) {
      E.ProcessingError(__OlxSrcInfo, "ooops...");
      return;
    }
    vec3d trans = st.GetResults()[ind].Center;
    //TVectorD trans = st.GetGravityCenter();
    trans /= 2;
    trans *= -1;
    m.t = trans;
    TSAtomPList atoms = xapp.FindSAtoms(TStrList());
    xapp.XFile().GetLattice().TransformFragments(atoms, m);
    au.ChangeSpaceGroup(*sg);
    xapp.XFile().LastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
    latt.Init();
    for (size_t i = 0; i < au.AtomCount(); i++) {
      for (size_t j = i + 1; j < au.AtomCount(); j++) {
        if (au.GetAtom(j).IsDeleted()) {
          continue;
        }
        double d = uc.FindClosestDistance(au.GetAtom(i), au.GetAtom(j));
        if (d < 0.5) {
          au.GetAtom(i).SetDeleted(true);
          break;
        }
      }
    }
    //    latt.OnStructureGrow->SetEnabled(false);
    //    latt.OnStructureUniq->SetEnabled(false);
    latt.Init();
    latt.Compaq();
    //    latt.OnStructureGrow->SetEnabled(true);
    //    latt.OnStructureUniq->SetEnabled(true);
    latt.CompaqAll();
  }
  else {
    E.ProcessingError(__OlxSrcInfo, "could not find interatomic relations");
  }
}
//.............................................................................
void XLibMacros::macCompaq(TStrObjList& Cmds, const TParamList& Options,
  TMacroData& E)
{
  TXFile& xf = TXApp::GetInstance().XFile();
  if (xf.GetLattice().IsGenerated() && !Options.Contains('q')) {
    TBasicApp::NewLogEntry(logError) <<
      "Cannot perform this operation on grown structure";
    return;
  }
  if (Options.Contains('a')) {
    xf.GetLattice().CompaqAll();
  }
  else if (Options.Contains('c')) {
    xf.GetLattice().CompaqClosest();
  }
  else if (Options.Contains('q')) {
    xf.GetLattice().CompaqQ();
  }
  else if (Options.Contains('m')) {
    TAsymmUnit& au = xf.GetAsymmUnit();
    TIntList modified(au.AtomCount());
    for (size_t i = 0; i < au.AtomCount(); i++) {
      TCAtom& a = au.GetAtom(i);
      modified[i] = -100;
      if (XElementLib::IsMetal(a.GetType())) {
        modified[i] = a.GetConnInfo().maxBonds;
        a.GetConnInfo().maxBonds = 0;
      }
      else if (a.GetType() == iQPeakZ && !a.IsDetached()) {
        modified[i] = -101;
        a.SetDetached(true);
      }
    }
    xf.GetLattice().Init();
    xf.GetLattice().CompaqAll();
    xf.GetLattice().CompaqClosest();
    for (size_t i = 0; i < au.AtomCount(); i++) {
      if (modified[i] == -101) {
        au.GetAtom(i).SetDetached(false);
      }
      else if (modified[i] != -100) {
        au.GetAtom(i).GetConnInfo().maxBonds = modified[i];
      }
    }
    xf.GetLattice().Init();
    xf.GetLattice().CompaqQ();
  }
  else {
    TXApp::GetInstance().XFile().GetLattice().Compaq();
  }
}
//.............................................................................
void XLibMacros::macEnvi(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& xapp = TXApp::GetInstance();
  double r = 2.7;
  Parse(Cmds, "d", &r);
  if (r < 1) {
    r = 1;
  }
  TSAtomPList atoms = xapp.FindSAtoms(Cmds, true, !Options.Contains("cs"));
  if (atoms.IsEmpty()) {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  // print calculated connectivity and SYMM
  if (Options.Contains('c')) {
    TStrList out;
    const TAsymmUnit &au = xapp.XFile().GetAsymmUnit();
    const TUnitCell &uc = xapp.XFile().GetUnitCell();
    for (size_t i = 0; i < atoms.Count(); i++) {
      out.Add(atoms[i]->GetLabel()) << " [" <<
        TSymmParser::MatrixToSymmEx(atoms[i]->GetMatrix()) << ']';
      for (size_t j = 0; j < atoms[i]->CAtom().AttachedSiteCount(); j++) {
        TCAtom::Site &s = atoms[i]->CAtom().GetAttachedSite(j);
        smatd m = uc.MulMatrix(s.matrix, atoms[i]->GetMatrix());
        double d = atoms[i]->crd().DistanceTo(
          au.Orthogonalise(m*s.atom->ccrd()));
        out.Add(s.atom->GetLabel()).LeftPadding(5, ' ').RightPadding(20, ' ') <<
          TSymmParser::MatrixToSymmEx(m).RightPadding(20, ' ') << ": " <<
          olxstr::FormatFloat(3, d);
      }
    }
    TBasicApp::NewLogEntry() << out;
    return;
  }
  int prc = Options.FindValue('p', 2).ToInt(),
    aprc = prc / 2;
  ElementPList Exceptions;
  Exceptions.Add(XElementLib::GetByIndex(iQPeakIndex));
  Exceptions.Add(XElementLib::GetByIndex(iHydrogenIndex));
  if (Options.Contains('q'))
    Exceptions.Remove(XElementLib::GetByIndex(iQPeakIndex));
  if (Options.Contains('h'))
    Exceptions.Remove(XElementLib::GetByIndex(iHydrogenIndex));

  TLattice& latt = xapp.XFile().GetLattice();
  TAsymmUnit& au = latt.GetAsymmUnit();
  TCAtomPList allAtoms;
  for (size_t i = 0; i < au.AtomCount(); i++) {
    if (!au.GetAtom(i).IsDeleted() &&
      !Exceptions.Contains(au.GetAtom(i).GetType()))
    {
      allAtoms.Add(au.GetAtom(i));
    }
  }
  olex2::IOlex2Processor::GetInstance()->processMacro("Freeze true",
    __OlxSrcInfo, true);
  for (size_t i = 0; i < atoms.Count(); i++) {
    TTypeList<AnAssociation3<TCAtom*, smatd, vec3d> > envi;
    latt.GetUnitCell().FindInRangeAMC(atoms[i]->ccrd(),
      1e-3, r, envi, &allAtoms);
    // remove self equivalents
    for (size_t j = 0; j < envi.Count(); j++) {
      if (j > 0 && !envi.IsNull(j - 1) &&
        envi[j - 1].GetA()->GetId() == envi[j].GetA()->GetId() &&
        envi[j - 1].GetB().Equals(envi[j].GetB()))
      {
        envi.NullItem(j - 1);
      }
      if (envi[j].GetA()->GetId() == atoms[i]->CAtom().GetId() &&
        envi[j].GetC().Equals(atoms[i]->crd(), 1e-3))
      {
        envi.NullItem(j);
      }
      else {
        envi[j].c -= atoms[i]->crd();
      }
    }
    envi.Pack();
    TTTable<TStrList> table(envi.Count(), envi.Count() + 2); // +SYM + LEN
    table.ColName(0) = atoms[i]->GetLabel();
    table.ColName(1) = "SYMM";
    BubbleSorter::Sort(envi, XLibMacros::TEnviComparator());
    for (size_t j = 0; j < envi.Count(); j++) {
      const AnAssociation3<TCAtom*, smatd, vec3d>& rd = envi[j];
      table.RowName(j) = rd.GetA()->GetLabel();
      table.ColName(j + 2) = table.RowName(j);
      if (rd.GetB().IsI()) {
        table[j][1] = 'I';  // identity
      }
      else {
        table[j][1] = TSymmParser::MatrixToSymmCode(
          xapp.XFile().GetUnitCell().GetSymmSpace(), rd.GetB());
      }
      table[j][0] = olxstr::FormatFloat(prc, rd.GetC().Length());
      for (size_t k = 0; k < envi.Count(); k++) {
        if (j <= k) {
          table[j][k + 2] = '-';
          continue;
        }
        const AnAssociation3<TCAtom*, smatd, vec3d>& rd1 = envi[k];
        if (!rd.GetC().IsNull() && !rd1.GetC().IsNull()) {
          double angle = rd.GetC().CAngle(rd1.GetC());
          angle = acos(angle) * 180 / M_PI;
          table[j][k + 2] = olxstr::FormatFloat(aprc, angle);
        }
        else {
          table[j][k + 2] = '-';
        }
      }
    }
    TBasicApp::NewLogEntry() <<
      table.CreateTXTList(EmptyString(), true, true, ' ');
  }
  olex2::IOlex2Processor::GetInstance()->processMacro("Freeze false",
    __OlxSrcInfo, true);
}
//.............................................................................
void XLibMacros::funRemoveSE(const TStrObjList &Params, TMacroData &E) {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup& sg = xapp.XFile().GetLastLoaderSG();
  if (Params[0] == "-1") {
    if (!sg.IsCentrosymmetric()) {
      E.SetRetVal(sg.GetName());
      return;
    }
    smatd_list ml;
    sg.GetMatrices(ml, mattAll^mattInversion);
    sorted::PrimitiveAssociation<double, TSpaceGroup*> sglist;
    for (size_t i = 0; i < TSymmLib::GetInstance().SGCount(); i++) {
      double st = 0;
      if (TSymmLib::GetInstance().GetGroup(i).Compare(ml).ok()) {
        sglist.Add(st, &TSymmLib::GetInstance().GetGroup(i));
      }
    }
    E.SetRetVal(sglist.IsEmpty() ? sg.GetName()
      : sglist.GetValue(0)->GetName());
  }
}
//.............................................................................
void XLibMacros::funFileName(const TStrObjList &Params, TMacroData &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    Tmp = TEFile::ExtractFileName(Params[0]);
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      Tmp = TEFile::ExtractFileName(TXApp::GetInstance().XFile().GetFileName());
    else
      Tmp = NoneString();
  }
  E.SetRetVal(TEFile::ChangeFileExt(Tmp, EmptyString()));
}
//.............................................................................
void XLibMacros::funFileExt(const TStrObjList &Params, TMacroData &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    E.SetRetVal(TEFile::ExtractFileExt(Params[0]));
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() ) {
      E.SetRetVal(
        TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()));
    }
    else
      E.SetRetVal(NoneString());
  }
}
//.............................................................................
void XLibMacros::funFilePath(const TStrObjList &Params, TMacroData &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    Tmp = TEFile::ExtractFilePath(Params[0]);
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      Tmp = TEFile::ExtractFilePath(TXApp::GetInstance().XFile().GetFileName());
    else
      Tmp = NoneString();
  }
  // see notes in funBaseDir
  TEFile::TrimPathDelimeterI(Tmp);
  E.SetRetVal(Tmp);
}
//.............................................................................
void XLibMacros::funFileDrive(const TStrObjList &Params, TMacroData &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    E.SetRetVal(TEFile::ExtractFileDrive(Params[0]));
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() ) {
      E.SetRetVal(
        TEFile::ExtractFileDrive(TXApp::GetInstance().XFile().GetFileName()));
    }
    else
      E.SetRetVal(NoneString());
  }
}
//.............................................................................
void XLibMacros::funFileFull(const TStrObjList &Params, TMacroData &E)  {
  if( TXApp::GetInstance().XFile().HasLastLoader() )
    E.SetRetVal(TXApp::GetInstance().XFile().GetFileName());
  else
    E.SetRetVal(NoneString());
}
//.............................................................................
void XLibMacros::funIsFileLoaded(const TStrObjList& Params, TMacroData &E) {
  E.SetRetVal(TXApp::GetInstance().XFile().HasLastLoader());
}
//.............................................................................
void XLibMacros::funTitle(const TStrObjList& Params, TMacroData &E)  {
  if( !TXApp::GetInstance().XFile().HasLastLoader() )  {
    if( Params.IsEmpty() )
      E.SetRetVal(olxstr("File is not loaded"));
    else
      E.SetRetVal(Params[0]);
  }
  else
    E.SetRetVal(TXApp::GetInstance().XFile().LastLoader()->GetTitle());
}
//.............................................................................
void XLibMacros::funIsFileType(const TStrObjList& Params, TMacroData &E) {
  if (Params[0].Equalsi("ins")) {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TIns>() &&
      TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName())
        .Equalsi("ins"));
  }
  else if (Params[0].Equalsi("res")) {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TIns>() &&
      TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName())
        .Equalsi("res"));
  }
  else if (Params[0].Equalsi("ires")) {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TIns>());
  }
  else if (Params[0].Equalsi("cif")) {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TCif>());
  }
  else if (Params[0].Equalsi("cmf")) {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TCif>());
  }
  else if (Params[0].Equalsi("p4p")) {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TP4PFile>());
  }
  else if (Params[0].Equalsi("mol")) {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TMol>());
  }
  else if (Params[0].Equalsi("xyz")) {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TMol>());
  }
  else if (Params[0].Equalsi("crs")) {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TCRSFile>());
  }
  else if (Params[0].Equalsi("oxm")) {
    E.SetRetVal(TXApp::GetInstance().XFile().HasLastLoader() &&
      TXApp::GetInstance().XFile().LastLoader()->IsNative());
  }
  else
    E.SetRetVal(false);
}
//.............................................................................
void XLibMacros::funBaseDir(const TStrObjList& Params, TMacroData &E)  {
  olxstr tmp(TBasicApp::GetBaseDir());
  // remove the trailing backslash, as it causes a lot of problems with
  // passing parameters to other programs:
  // windows parser assumes that \" is " and does wrong parsing...
  if( !tmp.IsEmpty() )  tmp.SetLength(tmp.Length()-1);
  E.SetRetVal(tmp);
}
//.............................................................................
void XLibMacros::funDataDir(const TStrObjList& Params, TMacroData &E)  {
  E.SetRetVal(TBasicApp::GetInstanceDir().SubStringFrom(0, 1));
}
//.............................................................................
void XLibMacros::funLSM(const TStrObjList& Params, TMacroData &E) {
  if (Params.IsEmpty())
    E.SetRetVal(TXApp::GetInstance().XFile().GetRM().GetRefinementMethod());
  else
    TXApp::GetInstance().XFile().GetRM().SetRefinementMethod(Params[0]);
}
//.............................................................................
void XLibMacros::funRun(const TStrObjList& Params, TMacroData &E) {
  using namespace olex2;
  IOlex2Processor* op = IOlex2Processor::GetInstance();
  if( op == NULL ) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "this function requires Olex2 processor implementation");
  }
  TStrList allCmds = TParamList::StrtokLines(Params.Text(' '), ">>");
  for( size_t i=0; i < allCmds.Count(); i++ )  {
    if( !op->processMacro(allCmds[i]) )  {
      if( (i+1) < allCmds.Count() ) {
        TBasicApp::NewLogEntry(logError) <<
          "Not all macros in the provided list were executed";
      }
      break;
    }
  }
  //E.SetRetVal(E.IsSuccessful());
  // we do not care about result, but nothing should be printed on the html...
  E.SetRetVal(EmptyString());
}
//.............................................................................
void XLibMacros::funIns(const TStrObjList& Params, TMacroData &E) {
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  olxstr tmp;
  if (Params[0].Equalsi("weight") || Params[0].Equalsi("wght")) {
    for (size_t j = 0; j < rm.used_weight.Count(); j++) {
      tmp << ' ' << rm.used_weight[j];
    }
    E.SetRetVal(tmp.IsEmpty() ? NAString() : tmp.SubStringFrom(1));
  }
  else if (Params[0].Equalsi("weight1")) {
    for (size_t j = 0; j < rm.proposed_weight.Count(); j++) {
      tmp << ' ' << rm.proposed_weight[j];
    }
    E.SetRetVal(tmp.IsEmpty() ? NAString() : tmp.SubStringFrom(1));
  }
  else if (Params[0].Equalsi("L.S.") || Params[0].Equalsi("CGLS")) {
    for (size_t i = 0; i < rm.LS.Count(); i++) {
      tmp << ' ' << rm.LS[i];
    }
    E.SetRetVal(tmp.IsEmpty() ? NAString() : tmp.SubStringFrom(1));
  }
  else if (Params[0].Equalsi("ls")) {
    E.SetRetVal(rm.LS.Count() == 0 ? NAString() : olxstr(rm.LS[0]));
  }
  else if (Params[0].Equalsi("plan")) {
    for (size_t i = 0; i < rm.PLAN.Count(); i++) {
      tmp << ' ' << ((i == 1) ? olx_round(rm.PLAN[i]) : rm.PLAN[i]);
    }
    E.SetRetVal(tmp.IsEmpty() ? NAString() : tmp.SubStringFrom(1));
  }
  else if (Params[0].Equalsi("qnum")) {
    E.SetRetVal(rm.PLAN.Count() == 0 ? NAString() : olxstr(rm.PLAN[0]));
  }
  else if (Params[0].Equalsi("shel")) {
    E.SetRetVal(rm.HasSHEL() ? rm.GetSHELStr() : NAString());
  }
  else if (Params[0].Equalsi("omit")) {
    E.SetRetVal(rm.HasOMIT() ? rm.GetOMITStr() : NAString());
  }
  else if (Params[0].Equalsi("conf")) {
    size_t cnt = 0;
    for (size_t i = 0; i < rm.InfoTabCount(); i++) {
      if (rm.GetInfoTab(i).GetType() == infotab_conf) {
        cnt++;
      }
    }
    E.SetRetVal(cnt == 0 ? NAString() : cnt);
  }
  else if (TXApp::GetInstance().CheckFileType<TIns>()) {
    TIns& I = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
    if (Params[0].Equalsi("R1")) {
      E.SetRetVal(I.GetR1() < 0 ? NAString() : olxstr(I.GetR1()));
    }
    if (!I.InsExists(Params[0])) {
      E.SetRetVal(NAString());
      return;
    }
    TInsList* insv = I.FindIns(Params[0]);
    if (insv != 0) {
      E.SetRetVal(insv->Text(' '));
    }
    else {
      E.SetRetVal(EmptyString());
    }
  }
  else {
    E.SetRetVal(NAString());
  }
}
//.............................................................................
void XLibMacros::funSSM(const TStrObjList& Params, TMacroData &E) {
  RefinementModel& rm  = TXApp::GetInstance().XFile().GetRM();
  if( rm.GetSolutionMethod().IsEmpty() && Params.Count() == 1 )
    E.SetRetVal(Params[0]);
  else
    E.SetRetVal(rm.GetSolutionMethod());
}
//.............................................................................
bool XLibMacros_funSGNameIsNextSub(const olxstr& name, size_t i)  {
  if( (i+1) < name.Length() )  {
    if( olxstr::o_isdigit(name[i]) && olxstr::o_isdigit(name[i+1]) )  {
      if( name[i] != '1' && name[i] > name[i+1] )
        return true;
    }
  }
  return false;
}
olxstr XLibMacros_funSGNameToHtml(const olxstr& name)  {
  olxstr res;
  res.SetCapacity(name.Length() + 20);
  for( size_t i=0; i < name.Length(); i++ )  {
    if( XLibMacros_funSGNameIsNextSub(name, i+1) )  {
      res << name[i];
      continue;
    }
    if( XLibMacros_funSGNameIsNextSub(name, i) )  {
      res << name[i] << "<sub>" << name[i+1] << "</sub>";
      i++;
      continue;
    }
    res << name[i];
  }
  return res;
}
olxstr XLibMacros_funSGNameToHtmlX(const olxstr& name) {
  TStrList toks(name, ' ');
  olxstr res;
  for( size_t i=0; i < toks.Count(); i++ )  {
    if (toks[i].Length() >= 2 && XLibMacros_funSGNameIsNextSub(toks[i], 0)) {
      res << toks[i].CharAt(0) << "<sub>" << toks[i].CharAt(1) << "</sub>" <<
        toks[i].SubStringFrom(2);
    }
    else
      res << toks[i];
  }
  return res;
}
void XLibMacros::funSG(const TStrObjList &Cmds, TMacroData &E) {
  TSpaceGroup* sg = NULL;
  try {
    if (TXApp::GetInstance().XFile().HasLastLoader()) {
      sg = &TXApp::GetInstance().XFile().GetLastLoaderSG();
    }
  }
  catch(...)  {}
  if (sg != NULL) {
    olxstr Tmp;
    if (Cmds.IsEmpty()) {
      Tmp = sg->GetName();
      if (!sg->GetFullName().IsEmpty()) {
        Tmp << " (" << sg->GetFullName() << ')';
      }
      Tmp << " #" << sg->GetNumber();
    }
    else {
      Tmp = Cmds[0];
      Tmp.Replace("%#", olxstr(sg->GetNumber())).\
        Replace("%n", sg->GetName()).\
        Replace("%N", sg->GetFullName()).\
        Replace("%HS", sg->GetHallSymbol()).\
        Replace("%s", sg->GetBravaisLattice().GetName());
        Tmp.Replace("%H", XLibMacros_funSGNameToHtmlX(sg->GetFullName()));
        if (sg->GetName() == olxstr::DeleteChars(sg->GetFullName(), ' '))
          Tmp.Replace("%h", XLibMacros_funSGNameToHtmlX(sg->GetFullName()));
        else
          Tmp.Replace("%h", XLibMacros_funSGNameToHtml(sg->GetName()));
        Tmp.Replace("%c", (sg->IsCentrosymmetric() ? TrueString() : FalseString()));
    }
    E.SetRetVal(Tmp);
  }
  else {
    E.SetRetVal(NAString());
    return;
  }
}
//.............................................................................
void XLibMacros::funSGS(const TStrObjList &Cmds, TMacroData &E) {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup& sg = xapp.XFile().GetLastLoaderSG();
  const olxstr& axis =  sg.GetAxis();
  if( axis.IsEmpty() )
    E.SetRetVal<olxstr>("standard");
  else  {
    if( axis.Length() == 2 )  {  // axis + cell choice
      E.SetRetVal(olxstr(axis.CharAt(0)) << ": cell choice " << axis.CharAt(1));
    }
    else  {
      E.SetRetVal(olxstr("axis: ") << axis);
    }
  }
}
//.............................................................................
void XLibMacros::funHKLSrc(const TStrObjList& Params, TMacroData &E) {
  TXApp& xapp = TXApp::GetInstance();
  if (Params.Count() == 1) {
    xapp.XFile().GetRM().SetHKLSource(Params[0]);
  }
  else {
    olxstr fn = xapp.XFile().GetRM().GetHKLSource();
    if (TEFile::Exists(fn) && !TEFile::IsDir(fn)) {  // check the format...
      static TEFile::FileID fid;
      TEFile::FileID tfid = TEFile::GetFileID(fn);
      if (tfid != fid) {
        fid = tfid;
        TEFile f(fn, "rb");
        if (!THklFile::IsHKLFileLine(f.ReadLine())) {
          f.Close();
          fn = TEFile::AddPathDelimeter(TEFile::ExtractFilePath(fn)) <<
            MD5::Digest(fn) << ".hkl";
          if (!TEFile::Exists(fn)) {
            TBasicApp::NewLogEntry() << "Creating HKL file...";
            THklFile::SaveToFile(fn, xapp.XFile().GetRM().GetReflections());
            xapp.XFile().GetRM().SetHKLSource(fn);
          }
        }
      }
    }
    E.SetRetVal(fn);
  }
}
//.............................................................................
void XLibMacros::macCif2Doc(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  olxstr CifDictionaryFile(xapp.GetCifTemplatesDir() + "cifindex.dat");
  if( Cmds.IsEmpty() )  {
    TStrList Output;
    olxstr CDir = TEFile::CurrentDir();
    TEFile::ChangeDir(xapp.GetCifTemplatesDir());
    TEFile::ListCurrentDir(Output, "*.rtf;*html;*.htm", sefFile);
    TEFile::ChangeDir(CDir);
    xapp.NewLogEntry() << "Templates found:";
    xapp.NewLogEntry() << Output;
    return;
  }

  olxstr TN = Cmds[0];
  if( !TEFile::Exists(TN) )
    TN = xapp.GetCifTemplatesDir() + TN;
  if( !TEFile::Exists(TN) )  {
    Error.ProcessingError(__OlxSrcInfo, "template for CIF does not exist: ") <<
      Cmds[0];
    return;
  }
  // resolvind the index file
  if( !TEFile::Exists(CifDictionaryFile) )  {
    Error.ProcessingError(__OlxSrcInfo, "CIF dictionary does not exist");
    return;
  }

  TCif *Cif, Cif1;
  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "cif");
    if( TEFile::Exists(cifFN) )
      Cif1.LoadFromFile(cifFN);
    else  {
      Error.ProcessingError(__OlxSrcInfo, "existing cif is expected");
      return;
    }
    Cif = &Cif1;
  }

  TStrList SL, Dic;
  olxstr RF(Options.FindValue("n", EmptyString()));
  if( RF.IsEmpty() )  {
    RF = TEFile::ChangeFileExt(Cif->GetFileName(), EmptyString());
    RF << "_doc";
  }
  RF = TEFile::ChangeFileExt(RF, TEFile::ExtractFileExt(TN));

  SL = TEFile::ReadLines(TN);
  Dic = TEFile::ReadLines(CifDictionaryFile);
  for( size_t i=0; i < SL.Count(); i++ )
    Cif->ResolveParamsFromDictionary(Dic, SL[i], '%', &XLibMacros::CifResolve);
  TUtf8File::WriteLines(RF, SL, false);
  TBasicApp::NewLogEntry(logInfo) << "Document name: " << RF;
}
//.............................................................................
void XLibMacros::macCif2Tab(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  olxstr CifTablesFile = Options.FindValue('t', "tables.xlt");
  olxstr TablesAlignmentTemplate = Options.FindValue("ta");
  if (!TEFile::IsAbsolutePath(CifTablesFile))
    CifTablesFile = xapp.GetCifTemplatesDir() + CifTablesFile;
  if (!TEFile::Exists(CifTablesFile)) {
    Error.ProcessingError(__OlxSrcInfo, "tables definition file is not found");
    return;
  }

  olxstr CifDictionaryFile(xapp.GetCifTemplatesDir() + "cifindex.dat");
  if (Cmds.IsEmpty()) {
    TDataFile DF;
    TStrList SL;
    TDataItem *Root;
    olxstr Tmp;
    DF.LoadFromXLFile(CifTablesFile, &SL);
    Root = DF.Root().FindItemi("Cif_Tables");
    if (Root != NULL) {
      xapp.NewLogEntry(logInfo) << "Found table definitions:";
      for (size_t i = 0; i < Root->ItemCount(); i++) {
        Tmp = "Table ";
        Tmp << Root->GetItemByIndex(i).GetName() << '(' << " #" << (int)i + 1 <<
          "): caption <---";
        xapp.NewLogEntry(logInfo) << Tmp;
        xapp.NewLogEntry(logInfo) << Root->GetItemByIndex(i).FindField("caption");
        xapp.NewLogEntry(logInfo) << "--->";
      }
    }
    else {
      Error.ProcessingError(__OlxSrcInfo, "tables definition node is not found");
      return;
    }
    return;
  }
  TCif *Cif;
  olx_object_ptr<TCif> Cif1(new TCif());
  if (xapp.CheckFileType<TCif>()) {
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  }
  else {
    olxstr cifFN = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "cif");
    if (TEFile::Exists(cifFN)) {
      Cif1->LoadFromFile(cifFN);
    }
    else {
      throw TFunctionFailedException(__OlxSourceInfo, "existing CIF is expected");
    }
    Cif = &Cif1;
  }

  TStrList SL, Dic;
  TDataFile DF;
  TTTable<TStrList> DT;
  DF.LoadFromXLFile(CifTablesFile, 0);
  Dic = TEFile::ReadLines(CifDictionaryFile);
  olxstr RF = Options.FindValue('n');
  if (RF.IsEmpty()) {
    RF = TEFile::ChangeFileExt(Cif->GetFileName(), EmptyString());
    RF << "_tables";
  }
  TDataItem* Root = DF.Root().FindItemi("Cif_Tables");
  if (Root == 0) {
    Error.ProcessingError(__OlxSrcInfo, "wrong root for table definitions");
    return;
  }
  { // guess format
    const olxstr str_format = Root->FindField("format", "html");
    const char* ext = str_format.Equalsi("html") ? "html" : "tex";
    if (!RF.EndsWithi(ext)) {
      RF << '.' << ext;
    }
  }
  smatd_list SymmList;
  size_t tab_count = 1;
  for (size_t i = 0; i < Cmds.Count(); i++) {
    TDataItem* TD = 0;
    if (Cmds[i].IsNumber()) {
      size_t index = Cmds[i].ToSizeT();
      if (index < Root->ItemCount()) {
        TD = &Root->GetItemByIndex(index);
      }
    }
    if (TD == 0) {
      TD = Root->FindItem(Cmds[i]);
    }
    if (TD == 0) {
      xapp.NewLogEntry(logWarning) << "Could not find table definition: " <<
        Cmds[i];
      continue;
    }
    if (TD->GetName().Equalsi("footer") || TD->GetName().Equalsi("header")
      || TD->GetName().StartsFromi("include"))
    {
      olxstr fn = TD->FindField("source");
      if (fn.Contains('$')) {
        ProcessExternalFunction(fn);
      }
      if (!TEFile::IsAbsolutePath(fn)) {
        fn = xapp.GetCifTemplatesDir() + fn;
      }
      TStrList SL1 = TEFile::ReadLines(fn);
      for (size_t j = 0; j < SL1.Count(); j++) {
        Cif->ResolveParamsFromDictionary(Dic, SL1[j], '%', &XLibMacros::CifResolve);
      }
      SL.AddAll(SL1);
      continue;
    }
    if (Cif->CreateTable(TD, DT, SymmList, Options.FindValue('l', "0").ToInt())
      && DT.RowCount() > 0)
    {
      olxstr Tmp = "Table ";
      Tmp << ++tab_count << ' ' << TD->FindField("caption");
      Tmp.Replace("%DATA_NAME%", Cif->GetDataName());
      if (Tmp.Contains('$')) {
        ProcessExternalFunction(Tmp);
      }
      // attributes
      TStrList CLA, THA;
      CLA.Add(TD->FindField("tha"));
      THA.Add(TD->FindField("tha"));
      for (size_t j = 0; j < TD->ItemCount(); j++) {
        CLA.Add(TD->GetItemByIndex(j).FindField("cola"));
        THA.Add(TD->GetItemByIndex(j).FindField("tha"));
      }
      olxstr footer;
      for (size_t i = 0; i < SymmList.Count(); i++) {
        footer << "<sup>" << (i + 1) << "</sup>" <<
          TSymmParser::MatrixToSymmEx(SymmList[i]);
        if ((i + 1) < SymmList.Count()) {
          footer << "; ";
        }
      }
      SL.Add("<p>&nbsp;</p>");
      DT.CreateHTMLList(
        SL,
        Tmp,
        footer,
        true, false,
        TD->FindField("tita"),  // title paragraph attributes
        TD->FindField("foota"),  // footer paragraph attributes
        TD->FindField("taba"),  //const olxstr& tabAttr,
        TD->FindField("rowa"),  //const olxstr& rowAttr,
        THA, // header attributes
        CLA, // cell attributes,
        true,
        TD->FindField("coln", "1").ToInt(),
        TD->FindField("colsa"),
        TD->FindField("across", FalseString()).ToBool()
      );
    }
  }
  TUtf8File::WriteLines(RF, SL, false);
  TBasicApp::NewLogEntry(logInfo) << "Tables file: " << RF;
}
//.............................................................................
olxstr XLibMacros::CifResolve(const olxstr& func)  {
  using namespace olex2;
  IOlex2Processor* op = IOlex2Processor::GetInstance();
  if( op == NULL )  return func;
  olxstr rv = func;
  op->processFunction(rv);
  return rv;
}
//.............................................................................
bool XLibMacros::ProcessExternalFunction(olxstr& func)  {
  using namespace olex2;
  IOlex2Processor* op = IOlex2Processor::GetInstance();
  if( op == NULL )  return false;
  olxstr rv = func;
  if( op->processFunction(rv) )  {
    func = rv;
    return true;
  }
  return false;
}
//.............................................................................
//.............................................................................
//.............................................................................
class SIMU_analyser : public TUnitCell::IAtomAnalyser {
public:
  const TCAtom &origin;
  mutable const TCAtom *match;
  bool allow_h;
  SIMU_analyser(const TCAtom &o, bool allow_h_)
    : origin(o),
    match(0),
    allow_h(allow_h_)
  {}
  bool Matches(const TCAtom &a, double d) const {
    if (d < 1e-6 && a.GetId() == origin.GetId()) {
      return false;
    }
    if (a.GetEllipsoid() != 0) {
      if (!allow_h && a.GetType().z < 2) {
        return false;
      }
      match = &a;
      return true;
    }
    return false;
  }
};
//.............................................................................
void CifMerge_UpdateAtomLoop(TCif &Cif) {
  TXApp &xapp = TXApp::GetInstance();
  if (xapp.CheckFileType<TCif>()) {
    TBasicApp::NewLogEntry() <<
      "Cannot update the CIF - the real refinement model is unknown";
  }
  else {
    TStopWatch sw(__FUNC__);
    sw.start("Processing -u option");
    olxstr shelxl_version_number =
      Cif.GetParamAsString("_shelxl_version_number");
    if (shelxl_version_number.IsEmpty()) { // 2014-7?
      shelxl_version_number =
        Cif.GetParamAsString("_shelx_SHELXL_version_number");
    }
    const bool shelxl2014 = shelxl_version_number.StartsFrom("201"); //3/4/5...
    const TAsymmUnit &au = xapp.XFile().GetAsymmUnit();
    const TAsymmUnit &tau = Cif.GetAsymmUnit();
    bool match = (tau.AtomCount() <= au.AtomCount()),
      has_parts = false,
      has_special_positions = false;
    if (match) {
      size_t ind = 0;
      for (size_t i = 0; i < tau.AtomCount(); i++, ind++) {
        TCAtom& ta = tau.GetAtom(i);
        while (ind < au.AtomCount() &&
          (au.GetAtom(ind).IsDeleted() || au.GetAtom(ind).GetType() == iQPeakZ))
        {
          ind++;
        }
        if (ind >= au.AtomCount()) {
          match = false;
          break;
        }
        TCAtom &a = au.GetAtom(ind);
        if (!a.GetLabel().Equalsi(ta.GetLabel())) {
          // negative parts do not produce generated labels
          if (shelxl2014 && a.GetPart() > 0) {
            olxstr tl = olxstr(a.GetLabel()) << '_' <<
              olxch('a' + a.GetPart() - 1);
            if (!ta.GetLabel().Equalsi(tl)) {
              match = false;
              break;
            }
          }
        }
        if (a.ccrd().QDistanceTo(ta.ccrd()) > 1e-3) {
          match = false;
          break;
        }
        if (a.GetPart() != 0) {
          has_parts = true;
        }
        if (a.GetDegeneracy() != 1) {
          has_special_positions = true;
        }
      }
    }
    if (!match) {
      TBasicApp::NewLogEntry() << "Could not update the atom information loop - "
        "the asymmetric units mismatch";
    }
    else {
      const RefinementModel &rm = xapp.XFile().GetRM();
      SortedObjectList<const TCAtom *, TPointerComparator> rD,
        rU;
      for (size_t rs_i = 0; rs_i < rm.rDFIX.Count(); rs_i++) {
        TTypeList<ExplicitCAtomRef> r =
          rm.rDFIX[rs_i].GetAtoms().ExpandList(rm, 2);
        for (size_t ai = 0; ai < r.Count(); ai++) {
          rD.AddUnique(&r[ai].GetAtom());
        }
      }
      for (size_t rs_i = 0; rs_i < rm.rDANG.Count(); rs_i++) {
        TTypeList<ExplicitCAtomRef> r =
          rm.rDANG[rs_i].GetAtoms().ExpandList(rm, 2);
        for (size_t ai = 0; ai < r.Count(); ai++) {
          rD.AddUnique(&r[ai].GetAtom());
        }
      }
      for (size_t rs_i = 0; rs_i < rm.rSADI.Count(); rs_i++) {
        TTypeList<ExplicitCAtomRef> r =
          rm.rSADI[rs_i].GetAtoms().ExpandList(rm, 2);
        for (size_t ai = 0; ai < r.Count(); ai++) {
          rD.AddUnique(&r[ai].GetAtom());
        }
      }
      for (size_t rs_i = 0; rs_i < rm.rISOR.Count(); rs_i++) {
        TTypeList<ExplicitCAtomRef> r =
          rm.rISOR[rs_i].GetAtoms().ExpandList(rm);
        for (size_t ai = 0; ai < r.Count(); ai++) {
          rU.AddUnique(&r[ai].GetAtom());
        }
      }
      for (size_t rs_i = 0; rs_i < rm.rSAME.Count(); rs_i++) {
        TTypeList<ExplicitCAtomRef> r =
          rm.rSAME[rs_i].GetAtoms().ExpandList(rm);
        for (size_t ai = 0; ai < r.Count(); ai++) {
          rD.AddUnique(&r[ai].GetAtom());
        }
      }
      {
        TPtrList<const TSRestraintList> Urs;
        Urs << rm.rDELU << rm.rRIGU;
        au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
        for (size_t rrs_i = 0; rrs_i < Urs.Count(); rrs_i++) {
          const TSRestraintList &rl = *Urs[rrs_i];
          for (size_t rs_i = 0; rs_i < rl.Count(); rs_i++) {
            if (rl[rs_i].IsAllNonHAtoms()) {
              for (size_t ai = 0; ai < au.AtomCount(); ai++) {
                if (au.GetAtom(ai).GetTag() == 0 &&
                  au.GetAtom(ai).GetType().z > 1)
                {
                  rU.AddUnique(&au.GetAtom(ai));
                  au.GetAtom(ai).SetTag(1);
                }
              }
            }
            else {
              TTypeList<ExplicitCAtomRef> r =
                rl[rs_i].GetAtoms().ExpandList(rm);
              for (size_t ai = 0; ai < r.Count(); ai++) {
                if (r[ai].GetAtom().GetTag() == 0) {
                  rU.AddUnique(&r[ai].GetAtom());
                  r[ai].GetAtom().SetTag(1);
                }
              }
            }
          }
        }
        // now all atoms in rU have tag == 1
        for (size_t rs_i = 0; rs_i < rm.rSIMU.Count(); rs_i++) {
          double d = rm.rSIMU[rs_i].GetValue();
          if (rm.rSIMU[rs_i].IsAllNonHAtoms()) {
            for (size_t ai = 0; ai < au.AtomCount(); ai++) {
              if (au.GetAtom(ai).GetTag() == 1 ||
                au.GetAtom(ai).GetType().z < 2 ||
                au.GetAtom(ai).GetEllipsoid() == 0)
              {
                continue;
              }
              SIMU_analyser a(au.GetAtom(ai), false);
              if (xapp.XFile().GetUnitCell().HasInRange(
                au.GetAtom(ai).ccrd(), d, a))
              {
                rU.AddUnique(&au.GetAtom(ai));
                au.GetAtom(ai).SetTag(1);
                if (a.match->GetTag() == 0) {
                  rU.AddUnique(a.match);
                  au.GetAtom(a.match->GetId()).SetTag(1);
                }
              }
            }
          }
          else {
            TTypeList<ExplicitCAtomRef> r =
              rm.rSIMU[rs_i].GetAtoms().ExpandList(rm);
            for (size_t ai = 0; ai < r.Count(); ai++) {
              if (r[ai].GetAtom().GetEllipsoid() == 0 ||
                r[ai].GetAtom().GetTag() == 1)
              {
                continue;
              }
              SIMU_analyser a(r[ai].GetAtom(), true);
              if (xapp.XFile().GetUnitCell().HasInRange(
                r[ai].GetAtom().ccrd(), d, a))
              {
                rU.AddUnique(&r[ai].GetAtom());
                r[ai].GetAtom().SetTag(1);
              }
            }
          }
        }
      }
      cetTable* tab = Cif.FindLoop("_atom_site");
      if (tab == NULL || tab->RowCount() != tau.AtomCount()) {
        TBasicApp::NewLogEntry() << "Could not locate the atom_site loop or"
          " its content mismatches the asymmetric unit";
      }
      else {
        bool need_new_site_m =
          tab->RemoveCol("_atom_site_symmetry_multiplicity");
        if (!need_new_site_m) {
          need_new_site_m = (
            (tab->ColIndex("_atom_site_site_symmetry_multiplicity") == InvalidIndex)
            &&
            (tab->ColIndex("_atom_site_site_symmetry_order") == InvalidIndex));
        }
        size_t st_order_ind = InvalidIndex;
        if (need_new_site_m && has_special_positions) {
          tab->AddCol("_atom_site_site_symmetry_order");
          st_order_ind = tab->ColCount() - 1;
        }
        tab->RemoveCol("_atom_site_refinement_flags");
        size_t rf_pos_ind = tab->ColIndex("_atom_site_refinement_flags_posn");
        if (rf_pos_ind == InvalidIndex) {
          tab->AddCol("_atom_site_refinement_flags_posn");
          rf_pos_ind = tab->ColCount() - 1;
        }
        // re-evaluate col indices!
        if (st_order_ind != InvalidIndex) {
          st_order_ind = tab->ColIndex("_atom_site_site_symmetry_order");
        }
        // no more removals - so indices will stick
        size_t rf_adp_ind = tab->ColIndex("_atom_site_refinement_flags_adp");
        if ((!rU.IsEmpty() || has_special_positions) &&
          rf_adp_ind == InvalidIndex)
        {
          tab->AddCol("_atom_site_refinement_flags_adp");
          rf_adp_ind = tab->ColCount() - 1;
        }
        size_t rf_occu_ind = tab->ColIndex("_atom_site_refinement_flags_occupancy");
        if (has_special_positions && rf_occu_ind == InvalidIndex) {
          tab->AddCol("_atom_site_refinement_flags_occupancy");
          rf_occu_ind = tab->ColCount() - 1;
        }
        size_t dg_ind = tab->ColIndex("_atom_site_disorder_group");
        if (dg_ind == InvalidIndex && has_parts) {
          tab->AddCol("_atom_site_disorder_group");
          dg_ind = tab->ColCount() - 1;
        }

        TIntList h_t;
        size_t ri = 0;
        for (size_t i = 0; i < au.AtomCount(); i++, ri++) {
          while (i < au.AtomCount() &&
            (au.GetAtom(i).IsDeleted() || au.GetAtom(i).GetType() == iQPeakZ))
          {
            i++;
          }
          // last condition must not ever happen
          if (i >= au.AtomCount() || ri >= tab->RowCount())
            break;
          TCAtom &a = au.GetAtom(i);
          if (a.GetType() == iHydrogenZ) {
            int& h = h_t.Add(0);
            /*
            0 - all refined
            1 - Uiso constrained
            2 - U/Uiso - fixed
            4 - xyz constrained
            8 - xyz fixed
            *16 - occu fixed - NOT USED
            */
            if (a.GetUisoOwner() != 0) {  // u constrained
              h |= 0x0001;
            }
            else {
              if (a.GetEllipsoid() == 0) {
                XVarReference *r = a.GetVarRef(catom_var_name_Uiso);
                if (r != 0 && r->relation_type == relation_None) {
                  h |= 0x0002;
                }
              }
              else {
                bool all_fixed = true;
                for (short vi = catom_var_name_U11; vi <= catom_var_name_U33; vi++) {
                  XVarReference *r = a.GetVarRef(vi);
                  if (r == 0 || r->relation_type != relation_None) {
                    all_fixed = false;
                    break;
                  }
                }
                if (all_fixed) {
                  h |= 0x0002;
                }
              }
            }
            // position
            {
              bool all_fixed = true;
              for (short vi = catom_var_name_X; vi <= catom_var_name_Z; vi++) {
                XVarReference *r = a.GetVarRef(vi);
                if (r == 0 || r->relation_type != relation_None) {
                  all_fixed = false;
                  break;
                }
              }
              if (all_fixed) {
                h |= 0x0008;
              }
              else {
                if (a.GetParentAfixGroup() != 0 &&  // coordinates constrained
                  a.GetParentAfixGroup()->GetAfix() > 0)
                {
                  h |= 0x0004;
                }
              }
            }
            // occupancy
            //{
            //  XVarReference *r = a.GetVarRef(catom_var_name_Sof);
            //  if (r != NULL && r->relation_type == relation_None)
            //    h |= 0x0010;
            //}
          }
          olxstr pos_t, adp_t;
          if (rD.Contains(&a)) {
            pos_t << 'D';
          }
          if (a.GetParentAfixGroup() != NULL) {
            if (a.GetParentAfixGroup()->IsRefinable()) {
              pos_t << 'G';
            }
            if (a.GetParentAfixGroup()->IsRiding()) {
              pos_t << 'R';
            }
          }
          if (a.GetDegeneracy() != 1) {
            pos_t << 'S';
            adp_t << 'T';
          }
          if (rU.Contains(&a)) {
            adp_t << 'U';
          }
          if (pos_t.IsEmpty()) {
            tab->Set(ri, rf_pos_ind, new cetString('.'));
          }
          else {
            tab->Set(ri, rf_pos_ind, new cetString(pos_t));
          }

          if (rf_adp_ind != InvalidIndex) {
            if (adp_t.IsEmpty()) {
              tab->Set(ri, rf_adp_ind, new cetString('.'));
            }
            else {
              tab->Set(ri, rf_adp_ind, new cetString(adp_t));
            }
          }
          if (has_parts) {
            if (a.GetPart() == 0) {
              tab->Set(ri, dg_ind, new cetString('.'));
            }
            else {
              tab->Set(ri, dg_ind, new cetString((int)a.GetPart()));
            }
          }
          if (has_special_positions) {
            if (a.GetDegeneracy() == 1) {
              tab->Set(ri, rf_occu_ind, new cetString('.'));
            }
            else {
              tab->Set(ri, rf_occu_ind, new cetString('P'));
            }
          }
          if (st_order_ind != InvalidIndex) {
            tab->Set(ri, st_order_ind, new cetString(a.GetDegeneracy()));
          }
        }
        bool force_update = shelxl_version_number.IsEmpty();
        if (force_update ||
          !Cif.ParamExists("_refine_ls_hydrogen_treatment"))
        {
          if (h_t.IsEmpty())
            Cif.SetParam("_refine_ls_hydrogen_treatment", "undef", false);
          else {
            int v = h_t[0];
            bool all_same = true;
            for (size_t i = 1; i < h_t.Count(); i++) {
              if (h_t[i] != v) {
                all_same = false;
                break;
              }
            }
            if (all_same) {
              if (v == 0) {
                Cif.SetParam("_refine_ls_hydrogen_treatment", "refall", false);
              }
              else if (v == 2) { // fixed U
                Cif.SetParam("_refine_ls_hydrogen_treatment", "refxyz", false);
              }
              else if (v == 8) { // fixed xyz
                Cif.SetParam("_refine_ls_hydrogen_treatment", "refU", false);
              }
              else if (v == 10) { // fixed U and xyz
                Cif.SetParam("_refine_ls_hydrogen_treatment", "noref", false);
              }
              else if (v == 1 || v == 4 || v == 5) { // constrained U, xyz or both
                Cif.SetParam("_refine_ls_hydrogen_treatment", "constr", false);
              }
              else {
                Cif.SetParam("_refine_ls_hydrogen_treatment", "mixed", false);
              }
            }
            else {
              Cif.SetParam("_refine_ls_hydrogen_treatment", "mixed", false);
            }
          }
        }
      }
    }
  }
}

void CifMerge_EmbeddData(TCif &Cif, bool insert, bool fcf_format) {
  TStopWatch sw(__FUNC__);
  sw.start("Embedding data");
  TXApp &xapp = TXApp::GetInstance();
  const bool use_md5 = xapp.GetOptions()
    .FindValue("cif.use_md5", FalseString()).ToBool();
  // emmbedding the RES file into the CIF
  Cif.Remove("_shelx_res_file");
  Cif.Remove("_shelx_res_checksum");
  Cif.Remove("_iucr_refine_instructions_details");
  Cif.Remove("_shelx_hkl_file");
  Cif.Remove("_shelx_fab_file");
  Cif.Remove("_shelx_hkl_checksum");
  Cif.Remove("_shelx_fab_checksum");
  if (use_md5) {
    Cif.Remove("_olex2_res_file_MD5");
    Cif.Remove("_olex2_hkl_file_MD5");
    Cif.Remove("_olex2_fab_file_MD5");
  }
  Cif.Remove("_refln");
  if (insert) {
    olxstr res_fn = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "res");
    if (TEFile::Exists(res_fn)) {
      cetStringList res("_iucr_refine_instructions_details");
      TEFile res_f(res_fn, "rb");
      res.lines.LoadFromTextStream(res_f);
      Cif.SetParam(res);
      if (use_md5) {
        res_f.SetPosition(0);
        size_t as = res_f.GetAvailableSizeT();
        olx_array_ptr<char> bf(as);
        res_f.Read(&bf, as);
        olxcstr s = olxcstr::FromExternal(bf.release(), as);
        s.DeleteCharSet("\n\r\t ");
        Cif.SetParam("_olex2_res_file_MD5", MD5::Digest(s), false);
      }
    }
    // embedd HKL
    olxstr hkl_src = xapp.XFile().LocateHklFile();
    if (TEFile::Exists(hkl_src)) {
      THklFile hkl;
      hkl.LoadFromFile(hkl_src, false);
      cetTable *t;
      if (fcf_format) {
        t = THklFile::ExperimentalToFCF(hkl.RefList(), Cif);
      }
      else {
        t = THklFile::ExperimentalToCIF(hkl.RefList(), Cif);
      }
      if (use_md5) {
        olxstr_buf bf;
        for (size_t i = 0; i < t->RowCount(); i++) {
          for (size_t j = 0; j < t->ColCount(); j++) {
            bf << (*t)[i][j]->GetStringValue();
          }
        }
        Cif.SetParam("_olex2_hkl_file_MD5",
          MD5::Digest(olxcstr(olxstr(bf).DeleteChars(' '))), false);
      }
    }
    // try inserting FAB file
    if (xapp.CheckFileType<TIns>()) {
      TIns &ins = xapp.XFile().GetLastLoader<TIns>();
      if (ins.InsExists("ABIN")) {
        if (Cif.FindEntry("_shelx_fab_file") == 0) {
          olxstr fab_name = TEFile::ChangeFileExt(xapp.XFile().LocateHklFile(),
            "fab");
          if (!TEFile::Exists(fab_name)) {
            TBasicApp::NewLogEntry(logError) << "FAB file is missing";
          }
          else {
            cetStringList fab("_shelx_fab_file");
            fab.lines = TEFile::ReadLines(fab_name);
            for (size_t fi = 0; fi < fab.lines.Count(); fi++) {
              olxstr &l = fab.lines[fi];
              if (l.StartsFrom("loop_") || l.StartsFrom('_')) {
                try {
                  cif_dp::TCifDP fabc;
                  fabc.LoadFromStrings(fab.lines.SubListFrom(fi).GetObject());
                  for (size_t bc = 0; bc < fabc.Count(); bc++) {
                    for (size_t fj = 0; fj < fabc[bc].param_map.Count(); fj++) {
                      Cif.SetParam(*fabc[bc].param_map.GetValue(fj));
                    }
                  }
                  fab.lines.SetCount(fi);
                  break;
                }
                catch (const TExceptionBase &e) {
                  break;
                }
              }
              else {
                l.DeleteSequencesOf(' ');
              }
            }
            Cif.SetParam(fab);
            if (use_md5) {
              olxcstr s = fab.lines.Text(EmptyString()).c_str();
              s.DeleteCharSet("\n\r\t ");
              Cif.SetParam("_olex2_fab_file_MD5", MD5::Digest(s), false);
            }
          }
        }
      }
    }
  }
}
//.............................................................................
void XLibMacros::macCifMerge(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TStopWatch sw(__FUNC__);
  TXApp& xapp = TXApp::GetInstance();
  const bool resolve = Options.GetBoolOption("resolve", true, false);
  cif_dp::TCifDP src;
  TTypeList<olx_pair_t<olxstr,olxstr> > Translations;
  olxstr CifCustomisationFN = xapp.GetCifTemplatesDir() + "customisation.xlt";
  typedef SortedObjectList<olxstr, olxstrComparator<true> > SortedStrList;
  WildcardList to_skip, to_merge;
  sw.start("Reading customisation");
  if (TEFile::Exists(CifCustomisationFN)) {
    try {
      TDataFile df;
      if (!df.LoadFromXLFile(CifCustomisationFN)) {
        Error.ProcessingError(__OlxSrcInfo,
          "falied to load CIF customisation file");
        return;
      }
      df.Include(0);
      TDataItem& di = df.Root().GetItemByName(
        "cif_customisation").GetItemByName("translation");
      for (size_t i=0; i < di.ItemCount(); i++) {
        Translations.AddNew(di.GetItemByIndex(i).GetFieldByName("from"),
          di.GetItemByIndex(i).GetFieldByName("to"));
      }
      {
        TDataItem *si = df.Root().GetItemByName(
          "cif_customisation").FindItem("skip_merge");
        if (si != 0) {
          for (size_t i=0; i < si->ItemCount(); i++) {
            to_skip.Add(si->GetItemByIndex(i).GetName());
          }
        }
      }
      {
        TDataItem *mi = df.Root().GetItemByName(
          "cif_customisation").FindItem("do_merge");
        if (mi != 0) {
          for (size_t i = 0; i < mi->ItemCount(); i++) {
            to_merge.Add(mi->GetItemByIndex(i).GetName());
          }
        }
      }
    }
    catch (const TExceptionBase& e) {
      throw TFunctionFailedException(__OlxSourceInfo, e);
    }
  }
  TStrList _loop_names_to_skip("_atom;_geom;_space_group", ';');
  TCif *Cif;
  olx_object_ptr<TCif> Cif2(new TCif);
  if (xapp.CheckFileType<TCif>()) {
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  }
  else {
    olxstr cifFN = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "cif");
    if (TEFile::Exists(cifFN)) {
      sw.start("Loading CIF");
      Cif2->LoadFromFile(cifFN);
    }
    else {
      throw TFunctionFailedException(__OlxSourceInfo,
        "existing cif is expected");
    }
    Cif = &Cif2;
  }
  if (Cif->HasDuplicateLabels()) {
    TBasicApp::NewLogEntry(logWarning) << "Sorry - the CIF contains duplicate "
      "atom labels and Olex2 cannot correctly process it. Skipping CifMerge.";
      return;
  }
  // normalise
  for (size_t i = 0; i < Translations.Count(); i++) {
    Cif->Rename(Translations[i].GetA(), Translations[i].GetB());
  }
  // update the atom_site loop and H treatment if AU match
  if (Options.GetBoolOption('u')) {
    CifMerge_UpdateAtomLoop(*Cif);
  }
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  sw.start("Do the merging");
  for (size_t i=0; i < Cmds.Count(); i++) {
    bool force_merge = false;
    try {
      TStrList toks(Cmds[i], '&');
      olxstr fn = toks[0];
      olx_object_ptr<IDataInputStream> is = TFileHandlerManager::GetInputStream(fn);
      if (!is.ok()) {
        TBasicApp::NewLogEntry(logError) << "Could not find file: " << fn;
        continue;
      }
      for (size_t i=1; i < toks.Count(); i++) {
        size_t idx = toks[i].IndexOf('=');
        if (idx != InvalidIndex) {
          if (toks[i].SubStringTo(idx) == "force") {
            force_merge = toks[i].SubStringFrom(idx + 1).ToBool();
            break;
          }
        }
      }
      src.LoadFromStream(is);
    }
    catch(...) {}  // most like the cif does not have cell, so pass it
    if (src.Count() == 0) {
      continue;
    }
    // normalise
    for (size_t i = 0; i < Translations.Count(); i++) {
      src[0].Rename(Translations[i].GetA(), Translations[i].GetB());
    }
    for (size_t j = 0; j < src[0].param_map.Count(); j++) {
      const cif_dp::ICifEntry& e = *src[0].param_map.GetValue(j);
      if (!e.HasName()) {
        continue;
      }
      bool skip = false;
      if (!force_merge) {
        bool contains = false;
        {
          ICifEntry *ie = Cif->FindEntry(e.GetName());
          if (ie != 0) {
            try {
              olxstr v = ie->GetStringValue();
              if (!(v.IsEmpty() || v == '?' || v == '.')) {
                contains = true;
              }
            }
            catch (...) {
            }
          }
        }
        if (contains || !resolve) {
          if (to_skip.DoesMatch(e.GetName()) &&
            !to_merge.DoesMatch(e.GetName()))
          {
            TBasicApp::NewLogEntry(logInfo) << "Skipping '" << e.GetName() << '\'';
            continue;
          }
          if (e.Is<cetTable>()) {
            for (size_t k = 0; k < _loop_names_to_skip.Count(); k++) {
              const olxstr &i_name = e.GetName();
              if (i_name.StartsFromi(_loop_names_to_skip[k]) &&
                (i_name.Length() > _loop_names_to_skip[k].Length() &&
                  i_name.CharAt(_loop_names_to_skip[k].Length()) == '_'))
              {
                skip = true;
                break;
              }
            }
          }
        }
      }
      if (!skip) {
        bool processed = false;
        if (op != 0 && e.Is<cif_dp::cetString>()) {
          olxstr sv = e.GetStringValue();
          if (sv.StartsFrom('$')) {
            try {
              if (op->processFunction(sv)) {
                Cif->SetParam(cif_dp::cetString(e.GetName(), sv));
                processed = true;
              }
            }
            catch (TExceptionBase &e) {
              TBasicApp::NewLogEntry(logInfo) << e;
            }
          }
        }
        if (!processed) {
          Cif->SetParam(e);
        }
      }
      else {
        TBasicApp::NewLogEntry() << "Skipping '" << e.GetName() << '\'';
      }
    }
  }
  if (Options.Contains('f')) {
    olxstr i_v = Options.FindValue('f');
    bool insert = i_v.IsEmpty() ? true : i_v.ToBool();
    CifMerge_EmbeddData(*Cif, insert, Options.GetBoolOption("fcf"));
  }
  sw.start("Updating commonly mising parameters");
  // generate moiety string if does not exist
  const olxstr cif_moiety = Cif->GetParamAsString("_chemical_formula_moiety");
  if (cif_moiety.IsEmpty() || cif_moiety == '?') {
    Cif->SetParam("_chemical_formula_moiety",
      xapp.XFile().GetLattice().CalcMoietyStr(false), true);
  }
  Cif->SetParam("_cell_formula_units_Z",
    xapp.XFile().GetAsymmUnit().GetZ(), false);
  TSpaceGroup &sg = TSymmLib::GetInstance().FindSG(Cif->GetAsymmUnit());
  Cif->SetParam("_space_group_crystal_system",
    sg.GetBravaisLattice().GetName().ToLowerCase(), true);
  const olxstr hall_symbol = Cif->GetParamAsString("_space_group_name_Hall");
  if (hall_symbol.IsEmpty() || hall_symbol == '?') {
    Cif->SetParam("_space_group_name_Hall", sg.GetHallSymbol(), true);
  }
  Cif->SetParam("_space_group_name_H-M_alt", sg.GetFullName(), true);
  Cif->SetParam("_space_group_IT_number", sg.GetNumber(), false);
  if( !sg.IsCentrosymmetric() &&
    !Cif->ParamExists("_chemical_absolute_configuration") )
  {
    bool flack_used = false;
    if( xapp.CheckFileType<TIns>() )  {
      const TIns& ins = xapp.XFile().GetLastLoader<TIns>();
      const TLst& lst = ins.GetLst();
      olxstr flack = lst.params.Find("flack", EmptyString());
      if( !flack.IsEmpty() )  {
        try {
          TEValue<double> fv(flack);
          if( fv.GetE() < 0.2 )  {
            Cif->SetParam("_chemical_absolute_configuration", "ad", false);
            flack_used = true;
          }
        }
        catch(...) {}
      }
    }
    if( !flack_used )
      Cif->SetParam("_chemical_absolute_configuration", "unk", false);
  }
  // checking some possibly missing bits
  if (Cif->FindEntry("_diffrn_reflns_limit_h_max") == NULL) {
    RefinementModel::HklStat hs = xapp.XFile().GetRM().GetMergeStat();
    Cif->SetParam("_diffrn_reflns_limit_h_min", hs.MinIndexes[0], false);
    Cif->SetParam("_diffrn_reflns_limit_k_min", hs.MinIndexes[1], false);
    Cif->SetParam("_diffrn_reflns_limit_l_min", hs.MinIndexes[2], false);
    Cif->SetParam("_diffrn_reflns_limit_h_max", hs.MaxIndexes[0], false);
    Cif->SetParam("_diffrn_reflns_limit_k_max", hs.MaxIndexes[1], false);
    Cif->SetParam("_diffrn_reflns_limit_l_max", hs.MaxIndexes[2], false);
  }
  // batch scales
  if (xapp.XFile().GetRM().Vars.HasBASF()) {
    olx_object_ptr<olx_pair_t<cetTable *, size_t> > tw_lip =
      Cif->FindLoopItem("_twin_individual_mass_fraction_refined");
    cetTable *l = 0;
    size_t id_col_idx, fraction_col_idx;
    const XVarManager &vm = xapp.XFile().GetRM().Vars;
    if (tw_lip.ok()) {
      fraction_col_idx = tw_lip->b;
      id_col_idx = tw_lip->a->ColIndex("_twin_individual_id");
      if (id_col_idx != InvalidIndex) {
        l = tw_lip->a;
      }
    }
    if (l == 0 || l->RowCount() != (vm.GetBASFCount()+1)) {
      if (l != 0) {
        Cif->Remove(l->GetName());
        TBasicApp::NewLogEntry(logWarning) << "The TWIN definition loop does not"
          " match the refinement model - removing, but some merged information "
          "may be still invalid.";
      }
      l = &Cif->AddLoopDef("_twin_individual_id,"
        "_twin_individual_mass_fraction_refined");
      id_col_idx = 0;
      fraction_col_idx = 1;
    }
    double esd = 0, sum = 0;
    for (size_t i = 0; i < vm.GetBASFCount(); i++) {
      sum += vm.GetBASF(i).GetValue();
      esd += olx_sqr(vm.GetBASF(i).GetEsd());
    }
    for (size_t i = 0; i <= vm.GetBASFCount(); i++) {
      olxstr id_val = olxstr(i + 1);
      size_t row_id = InvalidIndex;
      for (size_t j = 0; j < l->RowCount(); j++) {
        if ((*l)[j][id_col_idx]->GetStringValue().Equals(id_val)) {
          row_id = j;
          break;
        }
      }
      if (row_id == InvalidIndex) {
        row_id = l->RowCount();
        l->AddRow()[id_col_idx] = new cetString(id_val);
      }
      if (i == 0) {
        l->Set(row_id, fraction_col_idx, 
          new cetString(TEValueD(1 - sum, sqrt(esd)).ToString()));
      }
      else {
        l->Set(row_id, fraction_col_idx,
          new cetString(vm.GetBASF(i-1).ToString()));
      }
    }
  }
  // update the refinement description
  cetStringList description("_olex2_refinement_description");
  TStrList ri = xapp.XFile().GetRM().Describe();
  for (size_t i=0; i < ri.Count(); i++) {
    description.lines.Hyphenate(ri[i].Replace(" ~ ", " \\\\sim "), 80, true);
  }
  Cif->SetParam(description);
  sw.start("Processing selected geometric measuremenets");
  xapp.XFile().GetRM().GetSelectedTableRows().Process(*Cif);
  sw.start("Saving the result");
  {
    olxstr new_dn = Options.FindValue("dn", EmptyString());
    if (Cif->GetBlockIndex() != InvalidIndex && !new_dn.IsEmpty()) {
      Cif->RenameCurrentBlock(new_dn);
    }
  }
  Cif->SaveToFile(Cif->GetFileName());
}
//.............................................................................
void XLibMacros::macCifExtract(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& xapp = TXApp::GetInstance();
  bool use_items = Options.Contains('i');
  olxstr items_file = (use_items ? Options.FindValue("i") : EmptyString());
  olx_object_ptr<TCif> external_cif;
  TCif *cif = 0;
  bool export_metacif = (!use_items || items_file.Equalsi("metacif"));
  if (Cmds.IsEmpty()) {
    if (!xapp.CheckFileType<TCif>()) {
      E.ProcessingError(__OlxSrcInfo, "invalid loaded file type");
      return;
    }
    else {
      cif = &xapp.XFile().GetLastLoader<TCif>();
      export_metacif = true;
    }
  }
  else {
    external_cif = new TCif;
    try {
      external_cif->LoadFromFile(Cmds[0]);
      cif = &external_cif;
    }
    catch (const TExceptionBase &e) {
      throw TFunctionFailedException(__OlxSrcInfo, e);
    }
  }
  olxstr dest;
  if (Cmds.Count() == 2) {
    dest = Cmds[1];
  }
  else {
    dest = (xapp.XFile().GetStructureDataFolder() +
      cif->GetDataName()) << ".metacif";
  }
  
  if (export_metacif) {
    olxstr CifCustomisationFN = xapp.GetCifTemplatesDir() +
      "customisation.xlt";
    TTypeList<Wildcard> to_extract, to_skip;
    if (TEFile::Exists(CifCustomisationFN)) {
      try {
        TDataFile df;
        if (!df.LoadFromXLFile(CifCustomisationFN)) {
          E.ProcessingError(__OlxSrcInfo,
            "falied to load CIF customisation file");
          return;
        }
        df.Include(NULL);
        TDataItem* di = df.Root().GetItemByName(
          "cif_customisation").FindItem("export_metacif");
        if (di == 0) {
          E.ProcessingError(__OlxSrcInfo, "the metacif definition is missing");
        }
        else {
          for (size_t i = 0; i < di->ItemCount(); i++) {
            if (di->GetItemByIndex(i).FindField("skip", FalseString()).
              ToBool())
            {
              to_skip.AddNew(di->GetItemByIndex(i).GetName());
            }
            else {
              to_extract.AddNew(di->GetItemByIndex(i).GetName());
            }
          }
        }
      }
      catch (const TExceptionBase& e) {
        throw TFunctionFailedException(__OlxSourceInfo, e);
      }
    }
    try {
      TCif mcf;
      mcf.SetCurrentBlock(cif->GetDataName(), true);
      const CifBlock &cb = cif->GetBlock(cif->GetBlockIndex());
      for (size_t i = 0; i < cb.params.Count(); i++) {
        bool skip = false;
        for (size_t j = 0; j < to_skip.Count(); j++) {
          if (to_skip[j].DoesMatch(cb.params[i])) {
            skip = true;
            break;
          }
        }
        if (skip) continue;
        for (size_t j = 0; j < to_extract.Count(); j++) {
          if (to_extract[j].DoesMatch(cb.params[i])) {
            try {
              olxstr s = cb.params.GetObject(i)->GetStringValue();
              if (s.IsEmpty() || s == '?') {
                continue;
              }
            }
            catch (const TExceptionBase &) {}
            mcf.SetParam(*cb.params.GetObject(i));
          }
        }
      }
      mcf.SaveToFile(dest);
    }
    catch (const TExceptionBase &e) {
      TBasicApp::NewLogEntry(logError) << "Failed to retrieve structure "
        "folder - skipping metacif export";
    }
  }
  else {
    // check if the dictionary exists
    if (items_file.IsEmpty() || !TEFile::Exists(items_file)) {
      items_file = xapp.GetCifTemplatesDir() + "extract.cif";
      if (!TEFile::Exists(items_file)) {
        E.ProcessingError(__OlxSrcInfo, "dictionary file does not exists");
        return;
      }
    }
    TCif items;
    items.LoadFromFile(items_file);
    using namespace cif_dp;
    items.RenameCurrentBlock("extract");
    cif_dp::TCifDP out_dp;
    CifBlock& out = out_dp.Add(cif->GetDataName());
    for (size_t i = 0; i < items.ParamCount(); i++) {
      ICifEntry* CifData =
        cif->FindParam<cif_dp::ICifEntry>(items.ParamName(i));
      if (CifData != NULL)
        out.Add(CifData->Replicate());
    }
    TStrList content;
    TUtf8File::WriteLines(Cmds[Cmds.Count() == 1 ? 0 : 1],
      out_dp.SaveToStrings(content), false);
  }
}
//.............................................................................
void XLibMacros::macCifCreate(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  VcoVContainer vcovc(xapp.XFile().GetAsymmUnit());
  try {
    olxstr src_mat = xapp.InitVcoV(vcovc);
    xapp.NewLogEntry() << "Using " << src_mat << " matrix for the calculation";
  }
  catch (TExceptionBase& e) {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }

  TAsymmUnit& _au = xapp.XFile().GetAsymmUnit();
  for (size_t i = 0; i < _au.AtomCount(); i++) {
    TCAtom& a = _au.GetAtom(i);
    if (a.GetEllipsoid() != NULL) {
      TEllipsoid& elp = *a.GetEllipsoid();
      a.SetUiso(elp.GetUeq());
      if (a.GetUisoEsd() == 0) {
        double esd = 0;
        for (int j = 0; j < 3; j++) {
          esd += olx_sqr(elp.GetEsd(j));
        }
        a.SetUisoEsd(sqrt(esd) / 3.);
      }
    }
    else if (a.GetType() == iHydrogenZ && a.GetUisoEsd() == 0) {
      a.SetUiso(olx_round(a.GetUiso(), 100000));
    }
  }
  TCif cif;
  cif.Adopt(xapp.XFile(), 0);
  TAsymmUnit& au = cif.GetAsymmUnit();
  for (size_t i = 0; i < au.AtomCount(); i++) {
    if (au.GetAtom(i).GetType() == iQPeakZ) {
      au.GetAtom(i).SetDeleted(true);
    }
  }
  TLattice latt(*(new SObjectProvider));
  latt.GetAsymmUnit().SetRefMod(&xapp.XFile().GetRM());
  latt.GetAsymmUnit().Assign(xapp.XFile().GetAsymmUnit());
  for (size_t i = 0; i < latt.GetAsymmUnit().AtomCount(); i++) {
    TCAtom& a = latt.GetAsymmUnit().GetAtom(i);
    if (a.IsDetached()) {
      a.SetDetached(false);
    }
    if (a.IsMasked()) {
      a.SetMasked(false);
    }
  }
  latt.GetAsymmUnit()._UpdateConnInfo();
  latt.GetAsymmUnit().DetachAtomType(iQPeakZ, true);
  latt.Init();
  latt.CompaqAll();
  ASObjectProvider& objects = latt.GetObjects();

  latt.GrowFragments(false, NULL);

  cif_dp::cetTable& bonds = cif.AddLoopDef(
    "_geom_bond_atom_site_label_1,_geom_bond_atom_site_label_2,"
    "_geom_bond_distance,_geom_bond_site_symmetry_2,_geom_bond_publ_flag");
  for (size_t i = 0; i < objects.bonds.Count(); i++) {
    TSBond& b = objects.bonds[i];
    if (b.A().GetType().GetMr() < 3 || b.A().IsDeleted()) {
      b.SetTag(0);
      continue;
    }
    if (b.B().GetType().GetMr() < 3 || b.B().IsDeleted()) {
      b.SetTag(0);
      continue;
    }
    b.SetTag(-1);
  }
  for (size_t i = 0; i < objects.atoms.Count(); i++) {
    TSAtom& a = objects.atoms[i];
    if (a.GetType().GetMr() < 3 || a.IsDeleted() || !a.IsAUAtom()) {
      continue;
    }
    for (size_t j = 0; j < a.BondCount(); j++) {
      TSBond& b = a.Bond(j);
      if (b.GetTag() == 0 || !b.A().IsAUAtom()) {
        continue;
      }
      b.SetTag(0);
      cif_dp::CifRow& row = bonds.AddRow();
      row.Set(0, new AtomCifEntry(b.A().CAtom()));
      row.Set(1, new AtomCifEntry(b.B().CAtom()));
      row[2] = new cetString(vcovc.CalcDistance(b.A(), b.B()).ToString());
      if (!b.B().IsAUAtom()) {
        row[3] = new cetString(TSymmParser::MatrixToSymmCode(
          xapp.XFile().GetUnitCell().GetSymmSpace(), b.B().GetMatrix()));
      }
      else {
        row[3] = new cetString('.');
      }
      row[4] = new cetString('?');
    }
  }
  cif_dp::cetTable& angles = cif.AddLoopDef(
    "_geom_angle_atom_site_label_1,_geom_angle_atom_site_label_2,"
    "_geom_angle_atom_site_label_3,_geom_angle,_geom_angle_site_symmetry_1,"
    "_geom_angle_site_symmetry_3,_geom_angle_publ_flag");
  for (size_t i = 0; i < objects.atoms.Count(); i++) {
    TSAtom& a = objects.atoms[i];
    if (a.GetType().GetMr() < 3 || a.IsDeleted() || !a.IsAUAtom()) {
      continue;
    }
    for (size_t j = 0; j < a.NodeCount(); j++) {
      TSAtom& b = a.Node(j);
      if (b.IsDeleted() || b.GetType().z < 2) {
        continue;
      }
      for (size_t k = j + 1; k < a.NodeCount(); k++) {
        TSAtom& c = a.Node(k);
        if (c.IsDeleted() || c.GetType().z < 2) {
          continue;
        }
        TSAtom& _b = (b.CAtom().GetId() <= c.CAtom().GetId() ? b : c);
        TSAtom& _c = (b.CAtom().GetId() > c.CAtom().GetId() ? b : c);
        cif_dp::CifRow& row = angles.AddRow();
        row.Set(0, new AtomCifEntry(_b.CAtom()));
        row.Set(1, new AtomCifEntry(a.CAtom()));
        row.Set(2, new AtomCifEntry(_c.CAtom()));
        row[3] = new cetString(vcovc.CalcAngleA(_b, a, _c).ToString());
        if (!_b.IsAUAtom()) {
          row[4] = new cetString(TSymmParser::MatrixToSymmCode(
            xapp.XFile().GetUnitCell().GetSymmSpace(), _b.GetMatrix()));
        }
        else
          row[4] = new cetString('.');
        if (!_c.IsAUAtom()) {
          row[5] = new cetString(TSymmParser::MatrixToSymmCode(
            xapp.XFile().GetUnitCell().GetSymmSpace(), _c.GetMatrix()));
        }
        else {
          row[5] = new cetString('.');
        }
        row[6] = new cetString('?');
      }
    }
  }
  RefinementModel& rm = xapp.XFile().GetRM();
  if (rm.InfoTabCount() != 0) {
    cif_dp::cetTable& hbonds = cif.AddLoopDef(
      "_geom_hbond_atom_site_label_D,_geom_hbond_atom_site_label_H,"
      "_geom_hbond_atom_site_label_A,_geom_hbond_distance_DH,"
      "_geom_hbond_distance_HA,_geom_hbond_distance_DA,"
      "_geom_hbond_angle_DHA,_geom_hbond_site_symmetry_A");
    smatd I;
    I.I().SetId(0);
    for (size_t i = 0; i < rm.InfoTabCount(); i++) {
      InfoTab& it = rm.GetInfoTab(i);
      if (it.GetType() != infotab_htab || !it.IsValid()) {
        continue;
      }
      TTypeList<ExplicitCAtomRef> ta = it.GetAtoms().ExpandList(rm);
      TSAtom* dsa = xapp.XFile().GetLattice().FindSAtom(ta[0].GetAtom());
      if (dsa == 0) { //eh?
        continue;
      }
      TAtomEnvi envi = xapp.XFile().GetUnitCell().GetAtomEnviList(*dsa);
      for (size_t j = 0; j < envi.Count(); j++) {
        if (envi.GetType(j) != iHydrogenZ) {
          continue;
        }
        // check if the D-H-A angle makes sense...
        double ang = (au.Orthogonalise(ta[0].GetAtom().ccrd()) - envi.GetCrd(j))
          .CAngle(au.Orthogonalise(ta[1].GetAtom().ccrd()) - envi.GetCrd(j));
        if (ang >= 0) { // <= 90
          continue;
        }
        CifRow& row = hbonds.AddRow();
        row.Set(0, new AtomCifEntry(ta[0].GetAtom()));
        row.Set(1, new AtomCifEntry(envi.GetCAtom(j)));
        row.Set(2, new AtomCifEntry(ta[1].GetAtom()));
        TSAtom da(0), aa(0);
        da.CAtom(ta[0].GetAtom());
        da._SetMatrix(I);
        da.crd() = au.Orthogonalise(da.ccrd());
        aa.CAtom(ta[1].GetAtom());
        smatd am;
        if (ta[1].GetMatrix() == 0) {
          am.I();
          am.SetId(0);
        }
        else {
          am = *ta[1].GetMatrix();
          xapp.XFile().GetUnitCell().InitMatrixId(am);
        }
        aa._SetMatrix(am);
        aa.ccrd() = am*aa.ccrd();
        aa.crd() = au.Orthogonalise(aa.ccrd());
        row[3] = new cetString(olxstr::FormatFloat(2, envi.GetCrd(j).
          DistanceTo(da.crd())));
        row[4] = new cetString(olxstr::FormatFloat(2, envi.GetCrd(j).
          DistanceTo(aa.crd())));
        row[5] = new cetString(vcovc.CalcDistance(da, aa).ToString());
        row[6] = new cetString(olxstr::FormatFloat(1, olx_angle(da.crd(),
          envi.GetCrd(j), aa.crd())));
        if (ta[1].GetMatrix() == NULL)
          row[7] = new cetString('.');
        else {
          row[7] = new cetString(
            TSymmParser::MatrixToSymmCode(
              xapp.XFile().GetUnitCell().GetSymmSpace(), aa.GetMatrix()));
        }
      }
    }
  }
  cif.SaveToFile(TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "cif"));
}
//.............................................................................
void XLibMacros::macFcfCreate(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  const int list_n = Cmds[0].ToInt();
  const olxstr fn = (Cmds.Count() > 1 ? Cmds.Text(' ', 1) :
    TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fcf"));
  TUnitCell::SymmSpace sp = xapp.XFile().GetUnitCell().GetSymmSpace();
  TRefList refs;
  TArrayList<compd> F;
  bool convert = Options.GetBoolOption('c');
  if (convert) {
    olxstr err = SFUtil::GetSF(refs, F, SFUtil::mapTypeCalc, SFUtil::sfOriginFcf,
      SFUtil::scaleExternal, 1);
    if (!err.IsEmpty()) {
      Error.ProcessingError(__OlxSrcInfo, err);
      return;
    }
  }
  olxstr col_names = "_refln_index_h,_refln_index_k,_refln_index_l,";
  if (list_n == 4) {
    if (!convert) {
      xapp.XFile().GetRM().GetRefinementRefList<
        TUnitCell::SymmSpace, RefMerger::ShelxMerger>(sp, refs);
    }
    col_names << "_refln_F_squared_calc,_refln_F_squared_meas,"
      "_refln_F_squared_sigma,_refln_observed_status";
  }
  else if (list_n == 6) {
    if (!convert) {
      xapp.XFile().GetRM().GetRefinementRefList<
        TUnitCell::SymmSpace, RefMerger::ShelxMerger>(sp, refs);
    }
    col_names << "_refln_F_squared_meas,_refln_F_squared_sigma,"
      "_refln_F_calc,_refln_phase_calc";
  }
  else if (list_n == 3) {
    if (!convert) {
      xapp.XFile().GetRM().GetFourierRefList<
        TUnitCell::SymmSpace, RefMerger::ShelxMerger>(sp, refs);
    }
    col_names << "_refln_F_meas,_refln_F_sigma,_refln_A_calc,_refln_B_calc";
  }
  else {
    Error.ProcessingError(__OlxSrcInfo, "unsupported list number: ") <<
      list_n;
    return;
  }
  if (!convert) {
    F.SetCount(refs.Count());
    SFUtil::CalcSF(xapp.XFile(), refs, F);
  }
  double scale_k = 1, scale_a = 0;
  olxstr scale_str = Options.FindValue("scale", convert ? "none" : "regression");
  if (scale_str.Equalsi("none")) {
  }
  else if (scale_str.Equalsi("external")) {
    scale_k = 1. / olx_sqr(xapp.XFile().GetRM().Vars.GetVar(0).GetValue());
  }
  else if (scale_str.Equalsi("simple")) {
    scale_k = SFUtil::CalcF2Scale(F, refs,
      TReflection::SigmaWeightCalculator<2>(),
      TReflection::IoverSigmaFilter(3));
  }
  else if (scale_str.Equalsi("regression")) {
    SFUtil::CalcF2Scale(F, refs, scale_k, scale_a);
  }
  else {
    Error.ProcessingError(__OlxSrcInfo, olxstr("unsupported scale: ") << scale_str);
    return;
  }

  TCifDP fcf_dp;
  CifBlock& cif_data = fcf_dp.Add(
    TEFile::ExtractFileName(fn).Replace(' ', EmptyString()));
  cif_data.Add(cetString::NewNamedString("_olex2_title",
    xapp.XFile().LastLoader()->GetTitle()));
  cif_data.Add(cetString::NewNamedString("_shelx_refln_list_code", list_n));

  const TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  cif_data.Add(cetString::NewNamedString("_cell_length_a",
    TEValueD(au.GetAxes()[0], au.GetAxisEsds()[0]).ToString()));
  cif_data.Add(cetString::NewNamedString("_cell_length_b",
    TEValueD(au.GetAxes()[1], au.GetAxisEsds()[1]).ToString()));
  cif_data.Add(cetString::NewNamedString("_cell_length_c",
    TEValueD(au.GetAxes()[2], au.GetAxisEsds()[2]).ToString()));
  cif_data.Add(cetString::NewNamedString("_cell_angle_alpha",
    TEValueD(au.GetAngles()[0], au.GetAngleEsds()[0]).ToString()));
  cif_data.Add(cetString::NewNamedString("_cell_angle_beta",
    TEValueD(au.GetAngles()[1], au.GetAngleEsds()[1]).ToString()));
  cif_data.Add(cetString::NewNamedString("_cell_angle_gamma",
    TEValueD(au.GetAngles()[2], au.GetAngleEsds()[2]).ToString()));

  const TUnitCell& uc = xapp.XFile().GetUnitCell();
  cetTable* sym_tab = new cetTable(
    "_space_group_symop_id,_space_group_symop_operation_xyz");
  for (size_t i = 0; i < uc.MatrixCount(); i++) {
    CifRow& r = sym_tab->AddRow();
    r[0] = new cetString(i + 1);
    r[1] = new cetString(TSymmParser::MatrixToSymmEx(uc.GetMatrix(i)));
  }
  cif_data.Add(sym_tab);

  cetTable* ref_tab = new cetTable(col_names);
  cif_data.Add(ref_tab);
  const RefinementModel &rm = xapp.XFile().GetRM();
  RefinementModel::EXTI::Shelxl cr = rm.GetShelxEXTICorrector();
  double exti = cr.IsValid();
  for (size_t i = 0; i < refs.Count(); i++) {
    TReflection& r = refs[i];
    double Fo2 = r.GetI()*scale_k + scale_a;
    double sigFo2 = r.GetS()*scale_k;
    double Fc_sq = F[i].qmod();
    if (exti != 0) {
      double k = cr.CalcForF2(refs[i].GetHkl(), Fc_sq);
      Fo2 *= k;
      sigFo2 *= k;
    }
    CifRow& row = ref_tab->AddRow();
    row[0] = new cetString(r.GetH());
    row[1] = new cetString(r.GetK());
    row[2] = new cetString(r.GetL());
    if (list_n == 3) {
      double Fo, s_m;
      //http://www.iucr.org/__data/iucr/cif/software/xtal/xtal372htmlman/html/refcal-desc.html
      if (Fo2 <= 0) {
        Fo = 0;
        s_m = sqrt(sigFo2); // xtal 3.7.2
      }
      else {
        Fo = sqrt(Fo2);
        s_m = sqrt(Fo2 + sigFo2) - Fo; // xtal 3.7.2
        //s_m = sigFo2/(Fo + sqrt(sigFo2+Fo2));  // sxtal 3.7
        //s_m = Fo2 < sigFo2 ? sigFo2 : sigFo2/(2*Fo); // crystals
      }
      row[3] = new cetString(Fo);
      row[4] = new cetString(s_m);
      if (sp.IsCentrosymmetric()) {
        row[5] = new cetString(olx_sign(F[i].Re()) * F[i].mod());
        row[6] = new cetString(0.0);
      }
      else {
        row[5] = new cetString(F[i].Re());
        row[6] = new cetString("0");
      }
    }
    else if (list_n == 4) {
      row[3] = new cetString(olxstr::FormatFloat(2, Fc_sq));
      row[4] = new cetString(olxstr::FormatFloat(2, Fo2));
      row[5] = new cetString(olxstr::FormatFloat(2, sigFo2));
      row[6] = new cetString('o');
    }
    else if (list_n == 6) {
      row[3] = new cetString(olxstr::FormatFloat(2, Fo2));
      row[4] = new cetString(olxstr::FormatFloat(2, sigFo2));
      row[5] = new cetString(olxstr::FormatFloat(2, F[i].mod()));
      row[6] = new cetString(olxstr::FormatFloat(2, F[i].arg()*180/M_PI));
    }
  }
  TEFile::WriteLines(fn, TCStrList(fcf_dp.SaveToStrings().GetObject()));
}
//.............................................................................
struct XLibMacros_StrF  {
  int h, k, l;
  double ps;
  TEComplex<double> v;
};
void XLibMacros::macVoidE(TStrObjList &Cmds, const TParamList &Options, TMacroData &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
//  TXApp& XApp = TXApp::GetInstance();
//  double F000 = 0;
//  double factor = 2;
//  TRefList refs;
//  TArrayList<TEComplex<double> > F;
//  TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
//  const TUnitCell& uc = XApp.XFile().GetUnitCell();
//  // space group matrix list
//  TSpaceGroup* sg = NULL;
//  try  { sg = &XApp.XFile().GetLastLoaderSG();  }
//  catch(...)  {
//    E.ProcessingError(__OlxSrcInfo, "could not locate space group");
//    return;
//  }
//  smatd_list ml;
//  sg->GetMatrices(ml, mattAll^mattInversion);
//  for( size_t i=0; i < au.AtomCount(); i++ )  {
//    TCAtom& ca = au.GetAtom(i);
//    if( ca.IsDeleted() || ca.GetType() == iQPeakZ )
//      continue;
//    F000 += ca.GetType().z*uc.MatrixCount()*ca.GetOccu();
//  }
//  olxstr fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fcf");
//  if( !TEFile::Exists(fcffn) )  {
//    fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fco");
//    if( !TEFile::Exists(fcffn) )  {
//      E.ProcessingError(__OlxSrcInfo, "please load fcf file or make sure the one exists in current folder");
//      return;
//    }
//  }
//  TCif cif;
//  cif.LoadFromFile(fcffn);
////  F000 = cif.GetSParam("_exptl_crystal_F_000").ToDouble();
//  TCifLoop* hklLoop = cif.FindLoop("_refln");
//  if( hklLoop == NULL )  {
//    E.ProcessingError(__OlxSrcInfo, "no hkl loop found");
//    return;
//  }
//  size_t hInd = hklLoop->GetTable().ColIndex("_refln_index_h");
//  size_t kInd = hklLoop->GetTable().ColIndex("_refln_index_k");
//  size_t lInd = hklLoop->GetTable().ColIndex("_refln_index_l");
//  // list 3, F
//  size_t mfInd = hklLoop->GetTable().ColIndex("_refln_F_meas");
//  size_t sfInd = hklLoop->GetTable().ColIndex("_refln_F_sigma");
//  size_t aInd = hklLoop->GetTable().ColIndex("_refln_A_calc");
//  size_t bInd = hklLoop->GetTable().ColIndex("_refln_B_calc");
//
//  if( (hInd|kInd|lInd|mfInd|sfInd|aInd|bInd) == InvalidIndex ) {
//      E.ProcessingError(__OlxSrcInfo, "list 3 fcf file is expected");
//      return;
//  }
//  refs.SetCapacity(hklLoop->GetTable().RowCount());
//  F.SetCount(hklLoop->GetTable().RowCount());
//  for( size_t i=0; i < hklLoop->GetTable().RowCount(); i++ )  {
//    TStringToList<olxstr,TCifLoopData*>& row = hklLoop->GetTable()[i];
//    TReflection& ref = refs.AddNew(row[hInd].ToInt(), row[kInd].ToInt(),
//      row[lInd].ToInt(), row[mfInd].ToDouble(), row[sfInd].ToDouble());
//    if( ref.GetH() < 0 )
//      factor = 4;
////    const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
////    F[i] = TEComplex<double>::polar(ref.GetI(), rv.arg());
////    F[i].A() = row[aInd].ToDouble();
////    F[i].B() = row[bInd].ToDouble();
//      const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
//      double dI = (ref.GetI() - rv.mod());
//      F[i] = TEComplex<double>::polar(dI, rv.arg());
//  }
//  olxstr hklFileName = XApp.LocateHklFile();
//  if( !TEFile::Exists(hklFileName) )  {
//    E.ProcessingError(__OlxSrcInfo, "could not locate hkl file");
//    return;
//  }
//  double vol = XApp.XFile().GetLattice().GetUnitCell().CalcVolume();
//  int minH = 100,  minK = 100,  minL = 100;
//  int maxH = -100, maxK = -100, maxL = -100;
//
//  vec3d hkl;
//  TArrayList<XLibMacros_StrF> AllF(refs.Count()*ml.Count());
//  int index = 0;
//  double f000 = 0;
//  for( size_t i=0; i < refs.Count(); i++ )  {
//    const TReflection& ref = refs[i];
//    for( size_t j=0; j < ml.Count(); j++, index++ )  {
//      ref.MulHkl(hkl, ml[j]);
//      if( hkl[0] < minH )  minH = (int)hkl[0];
//      if( hkl[1] < minK )  minK = (int)hkl[1];
//      if( hkl[2] < minL )  minL = (int)hkl[2];
//      if( hkl[0] > maxH )  maxH = (int)hkl[0];
//      if( hkl[1] > maxK )  maxK = (int)hkl[1];
//      if( hkl[2] > maxL )  maxL = (int)hkl[2];
//      AllF[index].h = (int)hkl[0];
//      AllF[index].k = (int)hkl[1];
//      AllF[index].l = (int)hkl[2];
//      AllF[index].ps = hkl[0]*ml[j].t[0] + hkl[1]*ml[j].t[1] + hkl[2]*ml[j].t[2];
//      AllF[index].v = F[i];
//      AllF[index].v *= TEComplex<double>::polar(1, 2*M_PI*AllF[index].ps);
//    }
//  }
//// init map, 0.1A for now
//  const int mapX = (int)au.Axes()[0].GetV()*3,
//                      mapY = (int)au.Axes()[1].GetV()*3,
//                      mapZ = (int)au.Axes()[2].GetV()*3;
//  double mapVol = mapX*mapY*mapZ;
//  TArray3D<double> fMap(0, mapX-1, 0, mapY-1, 0, mapZ-1);
////////////////////////////////////////////////////////////////////////////////////////////
//  TEComplex<double> ** S, *T;
//  int kLen = maxK-minK+1, hLen = maxH-minH+1, lLen = maxL-minL+1;
//  S = new TEComplex<double>*[kLen];
//  for( int i=0; i < kLen; i++ )
//    S[i] = new TEComplex<double>[lLen];
//  T = new TEComplex<double>[lLen];
//  const double T_PI = 2*M_PI;
//// precalculations
//  int minInd = olx_min(minH, minK);
//  if( minL < minInd )  minInd = minL;
//  int maxInd = olx_max(maxH, maxK);
//  if( maxL > maxInd )  maxInd = maxL;
//  int iLen = maxInd - minInd + 1;
//  int mapMax = olx_max(mapX, mapY);
//  if( mapZ > mapMax )  mapMax = mapZ;
//  TEComplex<double>** sin_cosX = new TEComplex<double>*[mapX],
//                      **sin_cosY, **sin_cosZ;
//  for( int i=0; i < mapX; i++ )  {
//    sin_cosX[i] = new TEComplex<double>[iLen];
//    for( int j=minInd; j <= maxInd; j++ )  {
//      double rv = (double)(i*j)/mapX, ca, sa;
//      rv *= T_PI;
//      olx_sincos(-rv, &sa, &ca);
//      sin_cosX[i][j-minInd].SetRe(ca);
//      sin_cosX[i][j-minInd].SetIm(sa);
//    }
//  }
//  if( mapX == mapY )  {
//    sin_cosY = sin_cosX;
//  }
//  else  {
//    sin_cosY = new TEComplex<double>*[mapY];
//    for( int i=0; i < mapY; i++ )  {
//      sin_cosY[i] = new TEComplex<double>[iLen];
//      for( int j=minInd; j <= maxInd; j++ )  {
//        double rv = (double)(i*j)/mapY, ca, sa;
//        rv *= T_PI;
//        olx_sincos(-rv, &sa, &ca);
//        sin_cosY[i][j-minInd].SetRe(ca);
//        sin_cosY[i][j-minInd].SetIm(sa);
//      }
//    }
//  }
//  if( mapX == mapZ )  {
//    sin_cosZ = sin_cosX;
//  }
//  else if( mapY == mapZ )  {
//    sin_cosZ = sin_cosY;
//  }
//  else  {
//    sin_cosZ = new TEComplex<double>*[mapZ];
//    for( int i=0; i < mapZ; i++ )  {
//      sin_cosZ[i] = new TEComplex<double>[iLen];
//      for( int j=minInd; j <= maxInd; j++ )  {
//        double rv = (double)(i*j)/mapZ, ca, sa;
//        rv *= T_PI;
//        olx_sincos(-rv, &sa, &ca);
//        sin_cosZ[i][j-minInd].SetRe(ca);
//        sin_cosZ[i][j-minInd].SetIm(sa);
//      }
//    }
//  }
//  TEComplex<double> R;
//  double maxMapV = -1000, minMapV = 1000;
//  for( int ix=0; ix < mapX; ix++ )  {
//    for( size_t i=0; i < AllF.Count(); i++ )  {
//      const XLibMacros_StrF& sf = AllF[i];
//      S[sf.k-minK][sf.l-minL] += sf.v*sin_cosX[ix][sf.h-minInd];
//    }
//    for( int iy=0; iy < mapY; iy++ )  {
//      for( int i=minK; i <= maxK; i++ )  {
//        for( int j=minL; j <= maxL; j++ )  {
//          T[j-minL] += S[i-minK][j-minL]*sin_cosY[iy][i-minInd];
//        }
//      }
//      for( int iz=0; iz < mapZ; iz++ )  {
//        R.Null();
//        for( int i=minL; i <= maxL; i++ )  {
//          R += T[i-minL]*sin_cosZ[iz][i-minInd];
//        }
//        double val = factor*R.Re()/vol;
//        if( val > maxMapV )  maxMapV = val;
//        if( val < minMapV )  minMapV = val;
//        fMap.Data[ix][iy][iz] = val;
//      }
//      for( int i=0; i < lLen; i++ )
//        T[i].Null();
//    }
//    for( int i=0; i < kLen; i++ )
//      for( int j=0; j < lLen; j++ )
//        S[i][j].Null();
//  }
//  TBasicApp::GetLog() << (olxstr("Map max val ") << olxstr::FormatFloat(3, maxMapV) << " min val " << olxstr::FormatFloat(3, minMapV) << '\n');
////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//  // calculate the map
//  double surfdis = Options.FindValue("d", "1.0").ToDouble();
//  size_t structurePoints = 0;
//  vec3d voidCenter;
//  TArray3D<short> maskMap(0, mapX-1, 0, mapY-1, 0, mapZ-1);
//  short MaxLevel = XApp.CalcVoid(maskMap, surfdis, -101, &structurePoints, voidCenter, NULL);
//  XApp.GetLog() << ( olxstr("Cell volume (A^3) ") << olxstr::FormatFloat(3, vol) << '\n');
//  XApp.GetLog() << ( olxstr("Max level reached ") << MaxLevel << '\n');
//  XApp.GetLog() << ( olxstr("Largest spherical void is (A^3) ") << olxstr::FormatFloat(3, MaxLevel*MaxLevel*MaxLevel*4*M_PI/(3*mapVol)*vol) << '\n');
//  XApp.GetLog() << ( olxstr("Structure occupies (A^3) ") << olxstr::FormatFloat(3, structurePoints*vol/mapVol) << '\n');
//  int minLevel = olx_round(pow( 6*mapVol*3/(4*M_PI*vol), 1./3));
//  XApp.GetLog() << ( olxstr("6A^3 level is ") << minLevel << '\n');
//  // calculate new structure factors
//  double Re = 0, Te=0, F0 = 0;
//  int RePointCount = 0, TePointCount = 0;
////  for( int i=0; i < refs.Count(); i++ )  {
////    TReflection& ref = refs[i];
////    double A = 0, B = 0;
//    for( int ix=0; ix < mapX; ix++ )  {
//      for( int iy=0; iy < mapY; iy++ )  {
//        for( int iz=0; iz < mapZ; iz++ )  {
//          if( maskMap.Data[ix][iy][iz] <= 0  )  {
////            double tv =  (double)ref.GetH()*ix/mapX;
////            tv += (double)ref.GetK()*iy/mapY;
////            tv += (double)ref.GetL()*iz/mapZ;
////            tv *= T_PI;
////            double ca, sa;
////            olx_sincos(tv, &sa, &ca);
////            A += fMap.Data[ix][iy][iz]*ca;
////            B += fMap.Data[ix][iy][iz]*sa;
////            if( i == 0 )  {
//              Te += fMap.Data[ix][iy][iz];
//              TePointCount++;
////            }
//          }
//          else   {
////            if( i == 0 )  {
//              Re += fMap.Data[ix][iy][iz];
//              RePointCount++;
////            }
//          }
////          if( i == 0 )  {
//            F0 += fMap.Data[ix][iy][iz];
////          }
//        }
//      }
//    }
////    ref.SetI(sqrt(A*A+B*B)/100);
////  }
////  TCStrList sl;
////  for( int i=0;  i < refs.Count(); i++ )
////    sl.Add(refs[i].ToString());
////  sl.SaveToFile("test.hkl");
//  XApp.GetLog() << "Voids         " << Re*vol/(mapVol) << "e-\n";
////  XApp.GetLog() << "F000 calc     " << Te*vol/(mapVol) << "e-\n";
//  XApp.GetLog() << "F000 (formula)" << F000 << "e-\n";
////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//  for( int i=0; i < kLen; i++ )
//    delete [] S[i];
//  delete [] S;
//  delete [] T;
//  if( sin_cosY == sin_cosX )  sin_cosY = NULL;
//  if( sin_cosZ == sin_cosX || sin_cosZ == sin_cosY )  sin_cosZ = NULL;
//  for( int i=0; i < mapX; i++ )
//    delete [] sin_cosX[i];
//  delete [] sin_cosX;
//  if( sin_cosY != NULL )  {
//    for( int i=0; i < mapY; i++ )
//      delete [] sin_cosY[i];
//    delete [] sin_cosY;
//  }
//  if( sin_cosZ != NULL )  {
//    for( int i=0; i < mapZ; i++ )
//      delete [] sin_cosZ[i];
//    delete [] sin_cosZ;
//  }
}

//.............................................................................
void XLibMacros::macChangeSG(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& xapp = TXApp::GetInstance();
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  TSpaceGroup& from_sg = xapp.XFile().GetLastLoaderSG();
  TSpaceGroup* sg;
  if (Cmds.GetLastString().Contains(' ')) {
    sg = &TSymmLib::GetInstance().CreateNew(Cmds.GetLastString());
  }
  else {
    sg = TSymmLib::GetInstance().FindGroupByName(Cmds.GetLastString());
  }
  if (sg == NULL) {
    E.ProcessingError(__OlxSrcInfo, "Could not identify given space group");
    return;
  }
  // change centering?
  if (Options.GetBoolOption("c") &&
    from_sg.GetName().SubStringFrom(1) == sg->GetName().SubStringFrom(1))
  {
    olxch from = from_sg.GetLattice().GetSymbol()[0],
          to = sg->GetLattice().GetSymbol()[0];
    mat3d tm;
    tm.I();
    if (from == 'I') {
      if (to == 'P')
        tm = mat3d(-0.5, 0.5, 0.5, -0.5, 0.5, -0.5);
    }
    else if (from == 'P') {
      if (to == 'I')
        tm = mat3d(0, 1, 1, 0, 1, 0);
      else if (to == 'C') {
        tm = mat3d(0, 1, 1, 0, 1, 0);  // P->I
        tm *= mat3d(-1, 0, 1, 0, 1, 0, -1, 0, 0);  // I->C, uniq axis b
      }
      else if (to == 'F')
        tm = mat3d(-1, 1, 1, -1, 1, 1);
    }
    else if (from == 'C') {
      if (to == 'P') {
        tm = mat3d(0, 0, -1, 0, 1, 0, 1, 0, -1);  // C->I, uniq axis b
        tm *= mat3d(-0.5, 0.5, 0.5, -0.5, 0.5, -0.5);  // I->P
      }
    }
    else if (from == 'F') {
      if (to == 'P')
        tm = mat3d(0, 0.5, 0.5, 0, 0.5, 0);
    }
    if (!tm.IsI()) {
      TBasicApp::NewLogEntry() << "EXPERIMENTAL: transformations considering b unique";
      ChangeCell(tm, *sg, EmptyString());
    }
    else {
      TBasicApp::NewLogEntry() << "The transformation is not supported";
    }
    return;
  }
  smatd_list ml;
  sg->GetMatrices(ml, mattAll);
  TTypeList<AnAssociation3<vec3d,TCAtom*, int> > list;
  uc.GenereteAtomCoordinates(list, true);
  if (Cmds.Count() == 4) {
    vec3d trans(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble());
    for (size_t i=0; i < list.Count(); i++) {
      list[i].a += trans;
      list[i].SetC(1);
    }
  }
  else {
    for (size_t i=0; i < list.Count(); i++)
      list[i].SetC(1);
  }
  for (size_t i=0; i < list.Count(); i++) {
    if (list[i].GetC() == 0) continue;
    for (size_t j=i+1; j < list.Count(); j++) {
      if (list[j].GetC() == 0) continue;
      for (size_t k=1; k < ml.Count(); k++) {
        vec3d v = ml[k] * list[i].GetA();
        v -= list[j].GetA();
        v -= v.Round<int>();
        au.CellToCartesian(v);
        if (v.QLength() < 0.01 ) {
          list[i].c ++;
          list[j].SetC(0);
        }
      }
    }
  }
  for (size_t i=0; i < au.AtomCount(); i++)
    au.GetAtom(i).SetTag(0);
  TCAtomPList newAtoms;
  for (size_t i=0; i < list.Count(); i++) {
    if (list[i].GetC() == 0) continue;
    TCAtom* ca;
    if (list[i].GetB()->GetTag() > 0) {
      ca = &au.NewAtom();
      ca->Assign(*list[i].GetB());
    }
    else {
      ca = list[i].GetB();
      ca->SetTag(ca->GetTag() + 1);
    }
    ca->ccrd() = list[i].GetA();
    ca->AssignEllp(NULL);
  }
  for (size_t i=0; i < au.AtomCount(); i++) {
    if (au.GetAtom(i).GetTag() == 0)
      au.GetAtom(i).SetDeleted(true);
  }
  au.ChangeSpaceGroup(*sg);
  xapp.XFile().LastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
  latt.Init();
  latt.CompaqAll();
}
//.............................................................................
void XLibMacros::macFlush(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (Cmds.IsEmpty() || Cmds[0].Equalsi("log"))
    TBasicApp::GetLog().Flush();
}
//.............................................................................
void XLibMacros::macSGE(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  using namespace olex2;
  TXApp& xapp = TXApp::GetInstance();
  IOlex2Processor* op = IOlex2Processor::GetInstance();
  if (op == 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "this function requires Olex2 processor implementation");
  }
  TSpaceGroup* sg = 0;
  if (Options.GetBoolOption("f") ||
    (xapp.CheckFileType<TCRSFile>() &&
    ((TCRSFile*)xapp.XFile().LastLoader())->HasSG()))
  {
    sg = &xapp.XFile().GetLastLoaderSG();
    TBasicApp::NewLogEntry() << "Choosing CRS file space group: " <<
      sg->GetName();
  }
  if (sg == 0) {
    TPtrList<TSpaceGroup> sgs;
    E.SetRetVal(&sgs);
    op->processMacroEx("SG", E);
    E.SetRetVal<bool>(false);
    if (sgs.IsEmpty()) {
      TBasicApp::NewLogEntry(logError) <<
        "Could not find any suitable space group. Terminating ... ";
      return;
    }
    else if (sgs.Count() == 1) {
      sg = sgs[0];
      TBasicApp::NewLogEntry() << "Univocal space group choice: " <<
        sg->GetName();
    }
    else {
      olxstr cmd = "Wilson";
      if (!Cmds.IsEmpty()) {
        cmd << " '" << Cmds[0] << '\'';
      }
      op->processMacroEx(cmd, E);
      bool centro = E.GetRetVal().ToBool();
      TBasicApp::NewLogEntry() << "Searching for centrosymmetric group: " <<
        centro;
      for (size_t i = 0; i < sgs.Count(); i++) {
        if (centro) {
          if (sgs[i]->IsCentrosymmetric()) {
            sg = sgs[i];
            break;
          }
        }
        else {
          if (!sgs[i]->IsCentrosymmetric()) {
            sg = sgs[i];
            break;
          }
        }
      }
      if (sg == 0) {  // no match to centre of symmetry found
        sg = sgs[0];
        TBasicApp::NewLogEntry() << "Could not match, choosing: " <<
          sg->GetName();
      }
      else {
        TBasicApp::NewLogEntry() << "Chosen: " << sg->GetName();
      }
    }
  }
  olxstr fn = (Cmds.IsEmpty() ? TEFile::ChangeFileExt(
    TXApp::GetInstance().XFile().GetFileName(), "ins") : Cmds[0]);
  op->processMacroEx(olxstr("reset -s=") << sg->GetName() << " -f='" <<
    fn << '\'', E);
  if (E.IsSuccessful()) {
    op->processMacroEx(olxstr("reap '") << fn << '\'', E);
    if (E.IsSuccessful()) {
      OlxStateVar _var(VarName_ResetLock());
      op->processMacroEx(olxstr("solve"), E);
      // this will reset zoom!
      op->processMacroEx(olxstr("fuse"), E);
    }
    E.SetRetVal<bool>(E.IsSuccessful());
  }
}
//.............................................................................
void XLibMacros::macASR(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup& sg = xapp.XFile().GetLastLoaderSG();
  if (sg.IsCentrosymmetric()) {
    E.ProcessingError(__OlxSrcInfo,
      "not applicable to centrosymmetric space groups");
    return;
  }
  if (xapp.XFile().GetRM().GetHKLF() == 5 ||
    xapp.XFile().GetRM().GetHKLF() == 6)
  {
    E.ProcessingError(__OlxSrcInfo, "not applicable to HKLF 5/6 data format");
    return;
  }
  if (!xapp.XFile().GetRM().Vars.HasBASF()) {
    xapp.XFile().GetRM().Vars.SetBASF(TStrList() << "0.2");
    xapp.NewLogEntry() << "BASF 0.2 is added";
  }
  if (!xapp.XFile().GetRM().HasTWIN()) {
    xapp.XFile().GetRM().SetTWIN_n(2);
    xapp.NewLogEntry() << "TWIN set to 2 components";
  }
  if (xapp.XFile().GetRM().HasMERG() && xapp.XFile().GetRM().GetMERG() == 4) {
    xapp.NewLogEntry() << "Please note, that currently Friedel pairs are merged";
  }
  xapp.NewLogEntry() << "Done";
}
//.............................................................................
void XLibMacros::macDescribe(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& xapp = TXApp::GetInstance();
  TStrList lst =xapp.XFile().GetRM().Describe(),
    out;
  for( size_t i=0; i < lst.Count(); i++ )
    out.Hyphenate(lst[i], 80, true);
  xapp.NewLogEntry() << out;
}
//.............................................................................
void XLibMacros::macCalcCHN(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.XFile().HasLastLoader() && Cmds.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo,
      "Nor file is loaded neither formula is provided");
    return;
  }
  TCHNExp chn;
  double C=0, H=0, N=0, Mr=0;
  if( Cmds.Count() == 1 )  {
    chn.LoadFromExpression(Cmds[0]);
    chn.CHN(C, H, N, Mr);
    TBasicApp::NewLogEntry() << "Molecular weight: " << Mr;
    olxstr Msg("C: ");
    Msg << olxstr::FormatFloat(3, C*100./Mr) <<
      " H: " << olxstr::FormatFloat(3, H*100./Mr) <<
      " N: " << olxstr::FormatFloat(3, N*100./Mr);
    TBasicApp::NewLogEntry() << Msg << NewLineSequence();
    TBasicApp::NewLogEntry() << "Full composition:" <<
      NewLineSequence() << chn.Composition();
    return;
  }
  chn.LoadFromExpression(xapp.XFile().GetAsymmUnit().SummFormula(EmptyString()));
  chn.CHN(C, H, N, Mr);
  TBasicApp::NewLogEntry() << "Molecular weight: " << Mr;
  olxstr Msg("C: ");
  Msg << olxstr::FormatFloat(3, C*100./Mr) <<
    " H: " << olxstr::FormatFloat(3, H*100./Mr) <<
    " N: " << olxstr::FormatFloat(3, N*100./Mr);
  TBasicApp::NewLogEntry() << Msg;
  TBasicApp::NewLogEntry() << "Full composition:" <<
    NewLineSequence() << chn.Composition();
}
//.............................................................................
void XLibMacros::macCalcMass(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.XFile().HasLastLoader() && Cmds.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo,
      "Nor file is loaded neither formula is provided");
    return;
  }
  TIPattern ip;
  if( Cmds.Count() == 1 )  {
    olxstr err;
    if( !ip.Calc(Cmds[0], err, true, 0.5) )  {
      Error.ProcessingError(__OlxSrcInfo,
        "could not parse the given expression: ") << err;
      return;
    }
  }
  else  {
    olxstr err;
    if( !ip.Calc(xapp.XFile().GetAsymmUnit().SummFormula(
      EmptyString()), err, true, 0.5) )
    {
      Error.ProcessingError(__OlxSrcInfo,
        "could not parse the given expression: ") << err;
      return;
    }
  }
  for( size_t i=0; i < ip.PointCount(); i++ )  {
    const TSPoint& point = ip.Point(i);
    if( point.Y < 0.001 )  break;
    olxstr Msg = point.X;
    Msg.RightPadding(11, ' ');
    Msg << ": " << point.Y;
    xapp.NewLogEntry() << Msg;
  }
  TBasicApp::NewLogEntry() <<
    "    -- NOTE THAT NATURAL DISTRIBUTION OF ISOTOPES IS ASSUMED --";
  TBasicApp::NewLogEntry() << "******* ******* SPECTRUM ******* ********";
  ip.SortDataByMolWeight();
  for( size_t i=0; i < ip.PointCount(); i++ )  {
    const TSPoint& point = ip.Point(i);
    if( point.Y < 1 )  continue;
    olxstr Msg = point.X;
    Msg.RightPadding(11, ' ');
    Msg << "|";
    long yVal = olx_round(point.Y/2);
    for( long j=0; j < yVal; j++ )
      Msg << '-';
    xapp.NewLogEntry() << Msg;
  }
}
//.............................................................................
evecd XLibMacros_fit_chn_calc(const ematd& m, const evecd& p, size_t cnt)  {
  ematd _m(m.Vectors(), cnt), _mt(cnt, m.Vectors());
  evecd _v(m.Vectors());
  for( size_t i=0; i < m.Vectors(); i++ )  {
    for( size_t j=0; j < cnt; j++ )  {
      _m[i][j] = m[i][j];
      _mt[j][i] = m[i][j];
    }
    _v[i] = p[i];
  }
  ematd nm = _mt*_m;
  evecd res = _mt*_v;
  if( cnt == 1 )  {
    if( nm[0][0] == 0 )
      res[0] = -1;
    else
      res[0] = res[0]/nm[0][0];
  }
  else  {
    try  {  ematd::GaussSolve(nm, res);  }
    catch(...)  {
      for( size_t i=0; i < res.Count(); i++ )
        res[i] = -1.0;
    }
  }
  return res;
}
struct XLibMacros_ChnFitData  {
  double dev;
  olxstr formula;
  int Compare(const XLibMacros_ChnFitData& v) const {
    const double df = dev - v.dev;
    return df < 0 ? -1 : (df > 0 ? 1 : 0);
  }
};
void XLibMacros_fit_chn_process(TTypeList<XLibMacros_ChnFitData>& list,
  const ematd& chn,
  const evecd& p,
  const olxstr names[4],
  const olx_pdict<short, double>& obs,
  size_t cnt)
{
  for( size_t i=0; i < cnt; i++ )  {
    if( p[i] < 0 || fabs(p[i]) < 0.05 || p[i] > 5 )  return;
  }
  double mw = chn[0][obs.Count()];
  evecd e(obs.Count());
  for( size_t i=0; i < obs.Count(); i++ )
    e[i] = chn[0][i];
  olxstr name = names[0];
  for( size_t i=0; i < cnt; i++ )  {
    mw += p[i]*chn[i+1][obs.Count()];
    for( size_t j=0; j < obs.Count(); j++ )
      e[j] += p[i]*chn[i+1][j];
    name << " (" << names[i+1] << ')';
    name << olxstr::FormatFloat(2, p[i]);
  }
  XLibMacros_ChnFitData& cfd = list.AddNew();
  cfd.formula = name;
  double dev = 0;
  for( size_t i=0; i < obs.Count(); i++ )  {
    dev += olx_sqr(obs.GetValue(i)-e[i]/mw);
  }
  cfd.dev = sqrt(dev);
}
void XLibMacros::macFitCHN(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TCHNExp chne;
  chne.LoadFromExpression(Cmds[0]);
  TStrList solvents;
  olx_pdict<short, double> obs, calc;
  for( size_t i=1; i < Cmds.Count(); i++ )  {
    size_t si = Cmds[i].IndexOf(':');
    if( si == InvalidIndex )
      solvents.Add(Cmds[i]);
    else  {
      cm_Element* elm = XElementLib::FindBySymbol(Cmds[i].SubStringTo(si));
      if( elm == NULL )  {
        Error.ProcessingError(__OlxSrcInfo, olxstr("Invalid element: ") << Cmds[i]);
        return;
      }
      obs(elm->GetIndex(), Cmds[i].SubStringFrom(si+1).ToDouble()/100);
      calc(elm->GetIndex(), 0);
    }
  }
  if( solvents.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo,
      "a space separated list of solvents is expected");
    return;
  }
  TTypeList<XLibMacros_ChnFitData> list;
  const double Mw = chne.CHN(calc);
  olxstr names[4] = {chne.SummFormula(EmptyString()), EmptyString(),
    EmptyString(), EmptyString()};
  ematd m(obs.Count(), 3), chn(4, obs.Count()+1);
  evecd p(obs.Count());
  olxstr fit_info_from = "Fitting ", fit_info_to;
  for( size_t i=0; i < obs.Count(); i++ )  {
    p[i] = calc.GetValue(i) - obs.GetValue(i)*Mw;
    chn[0][i] = calc.GetValue(i);
    fit_info_from << XElementLib::GetByIndex(calc.GetKey(i)).symbol << ':'
      << olxstr::FormatFloat(2, calc.GetValue(i)*100/Mw);
    fit_info_to << XElementLib::GetByIndex(obs.GetKey(i)).symbol << ':' <<
      olxstr::FormatFloat(2, obs.GetValue(i)*100);
    if( (i+1) < calc.Count() )  {
       fit_info_to << ' ';
       fit_info_from << ' ';
    }
  }
  TBasicApp::NewLogEntry() << fit_info_from << " to " << fit_info_to;
  chn[0][obs.Count()] = Mw;
  for( size_t i=0; i < solvents.Count(); i++ )  {
    chne.LoadFromExpression(solvents[i]);
    const double mw = chne.CHN(calc);
    names[1] = chne.SummFormula(EmptyString());
    for( size_t i1=0; i1 < obs.Count(); i1++ )  {
      m[i1][0] = obs.GetValue(i1)*mw - calc.GetValue(i1);
      chn[1][i1] = calc.GetValue(i1);
    }
    chn[1][obs.Count()] = mw;
    evecd res = XLibMacros_fit_chn_calc(m, p, 1);
    XLibMacros_fit_chn_process(list, chn, res, names, obs, 1);
    for( size_t j = i+1; j < solvents.Count(); j++ )  {
      chne.LoadFromExpression(solvents[j]);
      const double mw1 = chne.CHN(calc);
      names[2] = chne.SummFormula(EmptyString());
      for( size_t i1=0; i1 < obs.Count(); i1++ )  {
        m[i1][1] = obs.GetValue(i1)*mw1 - calc.GetValue(i1);
        chn[2][i1] = calc.GetValue(i1);
      }
      chn[2][obs.Count()] = mw1;
      evecd res = XLibMacros_fit_chn_calc(m, p, 2);
      XLibMacros_fit_chn_process(list, chn, res, names, obs, 2);
      for( size_t k=j+1; k < solvents.Count(); k++ )  {
        chne.LoadFromExpression(solvents[k]);
        const double mw2 = chne.CHN(calc);
        names[3] = chne.SummFormula(EmptyString());
        for( size_t i1=0; i1 < obs.Count(); i1++ )  {
          m[i1][2] = obs.GetValue(i1)*mw2 - calc.GetValue(i1);
          chn[3][i1] = calc.GetValue(i1);
        }
        chn[3][obs.Count()] = mw2;
        evecd res = XLibMacros_fit_chn_calc(m, p, 3);
        XLibMacros_fit_chn_process(list, chn, res, names, obs, 3);
      }
    }
  }
  if( list.IsEmpty() )
    TBasicApp::NewLogEntry() << "Could not fit provided data";
  else  {
    QuickSorter::Sort(list);
    TETable tab(list.Count(), 3);
    tab.ColName(0) = "Formula";
    tab.ColName(1) = "CHN";
    tab.ColName(2) = "Deviation";
    for( size_t i=0; i < list.Count(); i++ )  {
      tab[i][0] = list[i].formula;
      chne.LoadFromExpression(list[i].formula);
      const double M = chne.CHN(calc);
      for( size_t j=0; j < calc.Count(); j++ )  {
        tab[i][1] << XElementLib::GetByIndex(calc.GetKey(j)).symbol << ':' <<
          olxstr::FormatFloat(2, calc.GetValue(j)*100/M);
        if( (j+1) < calc.Count() )
          tab[i][1] << ' ';
      }
      tab[i][2] = olxstr::FormatFloat(2, list[i].dev*100);
    }
    TBasicApp::NewLogEntry() << tab.CreateTXTList("Summary", true, false, ' ');
  }
}
//.............................................................................
void XLibMacros::macStandardise(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  TSpaceGroup& sg = xapp.XFile().GetLastLoaderSG();
  smatd_list sm;
  sg.GetMatrices(sm, mattAll^mattIdentity);
  if( Cmds.IsEmpty() )  {
    for( size_t i=0; i < au.AtomCount(); i++ ) {
      au.GetAtom(i).ccrd() = smatd::StandardiseFractional(
        au.GetAtom(i).ccrd(), sm);
    }
  }
  else  {
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      TCAtom& ca = au.GetAtom(i);
      vec3d v = ca.ccrd() - ca.ccrd().Floor<int>();
      double d = au.Orthogonalise(v).QLength();
      for( size_t j=0; j < sm.Count(); j++ )  {
        vec3d tmp = sm[j]*ca.ccrd();
        tmp -= tmp.Floor<int>();
        const double _d = au.Orthogonalise(tmp).QLength();
        if( _d < d )  {
          d = _d;
          v = tmp;
        }
      }
      ca.ccrd() = v;
    }
  }
  xapp.XFile().GetLattice().Init();
  xapp.XFile().GetLattice().Uniq();
}
//.............................................................................
void XLibMacros::macOmit(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  static olxstr sig("OMIT");
  TXApp &app = TXApp::GetInstance();
  RefinementModel& rm = app.XFile().GetRM();
  bool processed = false;
  if (Cmds.Count() == 1) {
    if (Cmds[0].IsNumber()) {
      const double th = Cmds[0].ToDouble();
      const TTypeList<RefinementModel::BadReflection> &bad_refs =
        rm.GetBadReflectionList();
      for (size_t i=0; i < bad_refs.Count(); i++) {
        if (rm.GetOmits().IndexOf(bad_refs[i].index) == InvalidIndex &&
          olx_abs(bad_refs[i].Fc-bad_refs[i].Fo)/bad_refs[i].esd >= th)
        {
          rm.Omit(bad_refs[i].index);
        }
      }
      processed = true;
    }
  }
  else if (Cmds.Count() == 2 && olx_list_and(Cmds, &olxstr::IsNumber)) {
    rm.AddOMIT(TStrList(Cmds));
    processed = true;
  }
  if (!processed) {
    if (Options.GetBoolOption('u')) {
      if (Cmds.Count() == 3) {
        rm.DelOMIT(Cmds);
      }
      else {
        Error.ProcessingError(__OlxSrcInfo, "3 integers are expected");
      }
      return;
    }
    rm.AddOMIT(Cmds);
  }
  OnAddIns().Exit(NULL, &sig);
}
//.............................................................................
void XLibMacros::macShel(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp &app = TXApp::GetInstance();
  RefinementModel& rm = app.XFile().GetRM();
  rm.SetSHEL(TStrList(Cmds));
}
//.............................................................................
void XLibMacros::funLst(const TStrObjList &Cmds, TMacroData &E) {
  const TIns& ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  const TLst& Lst = ins.GetLst();
  if (!Lst.IsLoaded()) {
    E.SetRetVal(NAString());
  }
  E.SetRetVal(Lst.params.Find(Cmds[0], NAString()));
}
//.............................................................................
void XLibMacros::macReset(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& xapp = TXApp::GetInstance();
  if (!(xapp.CheckFileType<TIns>() ||
    xapp.CheckFileType<TP4PFile>() ||
    xapp.CheckFileType<TCRSFile>()))
  {
    return;
  }
  if (TOlxVars::IsVar(VarName_InternalTref()) ||
      TOlxVars::IsVar(VarName_ResetLock()))
  {
    return;
  }
  using namespace olex2;
  IOlex2Processor* op = IOlex2Processor::GetInstance();
  olxstr newSg = Options.FindValue('s'),
         content = olxstr::DeleteChars(Options.FindValue('c'), ' '),
         fileName = Options.FindValue('f');
  xapp.XFile().UpdateAsymmUnit();
  TIns ins;
  ins.Adopt(xapp.XFile(), 0);
  if (xapp.CheckFileType<TP4PFile>()) {
    if (newSg.IsEmpty()) {
      E.ProcessingError(__OlxSrcInfo,
        "please specify a space group with -s=SG switch");
      return;
    }
  }
  else if (xapp.CheckFileType<TCRSFile>()) {
    TSpaceGroup* sg = xapp.XFile().GetLastLoader<TCRSFile>().GetSG();
    if (newSg.IsEmpty()) {
      if (sg == NULL) {
        E.ProcessingError(__OlxSrcInfo,
          "please specify a space group with -s=SG switch");
        return;
      }
      else {
        TBasicApp::NewLogEntry() << "The CRS file format space group is: "
          << sg->GetName();
      }
    }
  }
  if (!content.IsEmpty()) {
    ins.GetRM().SetUserFormula(content);
  }
  if (ins.GetRM().GetUserContent().IsEmpty()) {
    if (op != NULL) {
      content = "getuserinput(1, \'Please, enter structure composition\', \'C1\')";
      if (op->processFunction(content)) {
        ins.GetRM().SetUserFormula(content);
      }
      if (ins.GetRM().GetUserContent().IsEmpty()) {
        E.ProcessingError(__OlxSrcInfo,
          "empty SFAC instruction, please use -c=Content to specify");
        return;
      }
    }
  }
  if (!newSg.IsEmpty()) {
    TStrList sg_toks(newSg, '~', false);
    TSpaceGroup* sg = NULL;
    if (sg_toks.Count() == 1) {
      sg = TSymmLib::GetInstance().FindGroupByName(sg_toks[0]);
    }
    else {
      if (sg_toks.Count() == 5) {
        TStrList toks(sg_toks[0], ' ');
        SymmSpace::Info sg_info;
        for (size_t i = 0; i < toks.Count(); i++) {
          sg_info.matrices.AddCopy(TSymmParser::SymmToMatrix(toks[i]));
        }
        short latt = sg_toks[1].ToInt();
        sg_info.latt = olx_abs(latt);
        sg_info.centrosymmetric = latt > 0;
        sg = &TSymmLib::GetInstance().CreateNew(sg_info);
        TStrList cell_toks(sg_toks[2], ' '),
          esd_toks(sg_toks[3], ' '),
          hklf_toks(sg_toks[4], ' ');
        if (cell_toks.Count() != 6 || esd_toks.Count() != 6) {
          throw TInvalidArgumentException(__OlxSourceInfo, "CELL");
        }
        if (hklf_toks.Count() != 9) {
          throw TInvalidArgumentException(__OlxSourceInfo, "HKLF");
        }
        for (size_t i=0; i < 3; i++) {
          ins.GetAsymmUnit().GetAxes()[i] = cell_toks[i].ToDouble();
          ins.GetAsymmUnit().GetAngles()[i] = cell_toks[i+3].ToDouble();
          ins.GetAsymmUnit().GetAxisEsds()[i] = esd_toks[i].ToDouble();
          ins.GetAsymmUnit().GetAngleEsds()[i] = esd_toks[i+3].ToDouble();
        }
        mat3d m;
        for (size_t i = 0; i < 9; i++) {
          m[i/3][i%3] = hklf_toks[i].ToDouble();
        }
        // assume absolute value!
        ins.GetRM().SetHKLF_mat(m);
        //ins.GetRM().SetHKLF_mat(ins.GetRM().GetHKLF_mat()*m);
      }
    }
    if (sg == 0) {
      E.ProcessingError(__OlxSrcInfo, "could not find space group: ") << newSg;
      return;
    }
    ins.GetAsymmUnit().ChangeSpaceGroup(*sg);
    newSg.SetLength(0);
    newSg <<  " reset to " << sg->GetName() << " #" << sg->GetNumber();
    olxstr titl(TEFile::ChangeFileExt(
      TEFile::ExtractFileName(xapp.XFile().GetFileName()), EmptyString()));
    ins.SetTitle(titl << " in " << sg->GetName() << " #" << sg->GetNumber());
  }
  if (fileName.IsEmpty()) {
    fileName = xapp.XFile().GetFileName();
  }
  olxstr FN = TEFile::ChangeFileExt(fileName, "ins"),
    lstFN = TEFile::ChangeFileExt(fileName, "lst");

  ins.SaveForSolution(FN, Cmds.Text(' '), newSg, !Options.Contains("rem"),
    Options.Contains("atoms"));
  if (TEFile::Exists(lstFN)) {
    olxstr lstTmpFN(lstFN);
    lstTmpFN << ".tmp";
    TEFile::Rename(lstFN, lstTmpFN);
  }
  if (op != 0) {
    olxstr hkl_src = xapp.XFile().GetRM().GetHKLSource();
    op->processMacroEx(olxstr("@reap \'") << FN << '\'', E);
    if (E.IsSuccessful()) {
      TActionQueue *q =
        TBasicApp::GetInstance().FindActionQueue(olxappevent_UPDATE_GUI);
      if (q != 0) {
        q->Execute(0);
      }
      xapp.XFile().GetRM().SetHKLSource(hkl_src);
    }
  }
}
//.............................................................................
void XLibMacros::macDegen(TStrObjList& Cmds, const TParamList& Options,
  TMacroData& E)
{
  TSAtomPList atoms = TXApp::GetInstance().FindSAtoms(
    Cmds, true, !Options.Contains("cs"));
  TUnitCell& uc = TXApp::GetInstance().XFile().GetUnitCell();
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i]->CAtom().GetDegeneracy() == 1) {
      continue;
    }
    TStrList out;
    out.Add(atoms[i]->CAtom().GetLabel()) << " [" <<
      TSymmParser::MatrixToSymmEx(atoms[i]->GetMatrix()) << "] " <<
      atoms[i]->CAtom().GetDegeneracy();
    for (size_t j = 0; j < atoms[i]->CAtom().EquivCount(); j++) {
      smatd m = uc.MulMatrix(
        atoms[i]->CAtom().GetEquiv(j), atoms[i]->GetMatrix());
      out.Add('\t') << TSymmParser::MatrixToSymmEx(m);
    }
    TBasicApp::NewLogEntry() << out;
    SiteSymmCon ssc = atoms[i]->CAtom().GetSiteConstraints();
    TBasicApp::GetLog() << "\tSite constraints: ";
    if (ssc.IsConstrained()) {
      TBasicApp::NewLogEntry() << ssc.ToString();
    }
    else {
      TBasicApp::NewLogEntry() << "none";
    }
  }
}
//.............................................................................
void XLibMacros::macClose(TStrObjList &Cmds, const TParamList &Options, TMacroData &E)  {
  TXApp::GetInstance().XFile().Close();
}
//.............................................................................
void XLibMacros::macPiPi(TStrObjList &Cmds, const TParamList &Options, TMacroData &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TLattice latt(*(new SObjectProvider));
  RefinementModel rm(latt.GetAsymmUnit());
  latt.GetAsymmUnit().SetRefMod(&rm);
  rm.Assign(xapp.XFile().GetRM(), true);
  latt.GetAsymmUnit()._UpdateConnInfo();
  latt.GetAsymmUnit().DetachAtomType(iQPeakZ, true);
  latt.GetAsymmUnit().DetachAtomType(iHydrogenZ, true);
  latt.Init();
  latt.GrowFragments(false, NULL);

  TTypeList<ElementPList> ring_cont;
  //C6_ring(6), NC5_ring(6);
  ring_cont.AddNew(6);  // C6
  ring_cont.AddNew(6);  // NC5
  ring_cont[0][0] = &XElementLib::GetByIndex(iCarbonIndex);
  ring_cont[1][0] = &XElementLib::GetByIndex(iNitrogenIndex);
  for( int i=1; i < 6; i++ )
    ring_cont[0][i] = ring_cont[1][i] = ring_cont[0][0];
  olxstr str_rings = Options.FindValue('r');
  if( !str_rings.IsEmpty() )  {
    TStrList toks(str_rings, ',');
    for( size_t i=0; i < toks.Count(); i++ )  {
      ElementPList* rc = new ElementPList;
      try {  xapp.RingContentFromStr(toks[i], *rc);  }
      catch(...)  {
        TBasicApp::NewLogEntry(logError) << "Invalid ring definition: " << toks[i];
        delete rc;
        continue;
      }
      ring_cont.Add(rc);
    }
  }
  TTypeList<TSAtomPList> rings;
  for( size_t i=0; i < latt.FragmentCount(); i++ )  {
    TNetwork& frag = latt.GetFragment(i);
    if( frag.NodeCount() < 5 )  continue;
    for( size_t j=0; j < ring_cont.Count(); j++ )
      frag.FindRings(ring_cont[j], rings);
  }
  size_t plance_cnt = 0;
  for( size_t i=0; i < rings.Count(); i++ )  {
    const double rms = TSPlane::CalcRMSD(rings[i]);
    if( rms > 0.05 || !TNetwork::IsRingRegular(rings[i]) )  {
      olxstr rc = "Plane #";
      rc << ++plance_cnt << NewLineSequence();
      for( size_t j=0; j < rings[i].Count(); j++ )  {
        rc << rings[i][j]->GetGuiLabel();
        if( j < 5 )
          rc << ' ';
      }
      TBasicApp::NewLogEntry() << rc;
      rings.NullItem(i);
      continue;
    }
    bool identity_based = false;
    for( size_t j=0; j < rings[i].Count(); j++ )  {
      if( rings[i][j]->IsAUAtom() )  {
        identity_based = true;
        break;
      }
    }
    if( !identity_based )  {
      rings.NullItem(i);
      continue;
    }
    olxstr rc = "Plane #";
    rc << ++plance_cnt << NewLineSequence();
    for( size_t j=0; j < rings[i].Count(); j++ )  {
      rc << rings[i][j]->GetGuiLabel();
      if( j < 5 )
        rc << ' ';
    }
    TBasicApp::NewLogEntry() << rc;
  }
  rings.Pack();
  if( rings.IsEmpty() )  {
    TBasicApp::NewLogEntry() << "No C6 or NC5 or user specified regular rings could be found";
    return;
  }
  double max_d = 4, max_shift = 3;
  if( Cmds.Count() == 2 )  {
    max_d = Cmds[0].ToDouble();
    max_shift = Cmds[1].ToDouble();
  }
  TTypeList<TSPlane> planes(rings.Count(), false);
  TArrayList<vec3d> plane_centres(rings.Count());
  const TUnitCell& uc = latt.GetUnitCell();
  const TAsymmUnit& au = latt.GetAsymmUnit();
  for( size_t i=0; i < rings.Count(); i++ )  {
    TSPlane* sp = new TSPlane(&latt.GetNetwork());
    TTypeList<olx_pair_t<TSAtom*,double> > ring_atoms;
    for( size_t j=0; j < rings[i].Count(); j++ )
      ring_atoms.AddNew(rings[i][j],1.0);
    sp->Init(ring_atoms);
    planes.Set(i, sp);
    plane_centres[i] = sp->GetCenter();
    au.CartesianToCell(plane_centres[i]);
  }
  smatd_list transforms;
  for( size_t i=0; i < planes.Count(); i++ )  {
    TBasicApp::NewLogEntry() << "Considering plane #" << (i+1);
    size_t int_cnt = 0;
    for( size_t j=i; j < planes.Count(); j++ )  {
      for( size_t k=0; k < uc.MatrixCount(); k++ )  {
        const smatd& __mat = uc.GetMatrix(k);
        const vec3i tv = (__mat*plane_centres[j] - plane_centres[i]).Round<int>();
        smatd _mat = __mat;
        _mat.t -= tv;
        for( int x=-2; x <= 2; x++ )  {
          for( int y=-2; y <= 2; y++ )  {
            for( int z=-2; z <= 2; z++ )  {
              smatd mat = _mat;
              mat.t += vec3d(x,y,z);
              TTypeList<olx_pair_t<vec3d, double> > points;
              vec3d plane_params, plane_center;
              for( size_t pi=0; pi < planes[j].Count(); pi++ )  {
                points.AddNew(mat*planes[j].GetAtom(pi).ccrd(), 1.0);
                au.CellToCartesian(points.GetLast().a);
              }
              TSPlane::CalcPlane(points, plane_params, plane_center);
              const double pccd = planes[i].GetCenter().DistanceTo(plane_center);
              if( pccd < 1 || pccd > max_d )  continue;
              const double pcpd = olx_abs(planes[i].DistanceTo(plane_center));
              if( pcpd < 1 )  continue;   // ajacent planes?
              //const double plane_d = plane_params.DotProd(plane_center)/plane_params.Length()
              //plane_params.Normalise();
              const double shift = sqrt(olx_max(0, pccd*pccd - pcpd*pcpd));
              if( shift < max_shift )  {
                int_cnt++;
                TBasicApp::NewLogEntry() << '#' << (j+1) << '@' <<
                  TSymmParser::MatrixToSymmCode(uc.GetSymmSpace(), mat) <<
                  " (" << TSymmParser::MatrixToSymmEx(mat) << ")";
                TBasicApp::NewLogEntry() << "angle: " <<
                  olxstr::FormatFloat(3, planes[i].Angle(plane_params)) <<
                  ", centroid-centroid distance: " << olxstr::FormatFloat(3, pccd) <<
                  ", shift distance " << olxstr::FormatFloat(3, shift);
                if (!transforms.Contains(mat))
                  transforms.AddCopy(mat);
              }
            }
          }
        }
      }
    }
    if( int_cnt == 0 )
      TBasicApp::NewLogEntry() << "No interactions found";
  }
  if( Options.Contains('g') && !transforms.IsEmpty() )  {
    TLattice& xlatt = xapp.XFile().GetLattice();
    ASObjectProvider& objects = xlatt.GetObjects();
    const TUnitCell& uc = xlatt.GetUnitCell();
    for( size_t i=0; i < transforms.Count(); i++ )
      uc.InitMatrixId(transforms[i]);
    TCAtomPList iatoms;
    for( size_t i=0; i < objects.atoms.Count(); i++ )  {
      TSAtom& sa = objects.atoms[i];
      if( sa.IsDeleted() )  continue;
      if( sa.IsAUAtom() )
        iatoms.Add(sa.CAtom());
    }
    xlatt.GrowAtoms(iatoms, transforms);
  }
}
//.............................................................................
void XLibMacros::macPiSig(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double maxd = 4, maxa = 25;
  if (Cmds.Count() == 2) {
    if ((maxd = Cmds[0].ToDouble()) > 6 || maxd < 0) {
      maxd = 4;
    }
    maxa = Cmds[1].ToDouble();
  }
  TXApp& xapp = TXApp::GetInstance();
  TAsymmUnit &au = xapp.XFile().GetAsymmUnit();
  TUnitCell &uc = xapp.XFile().GetUnitCell();
  using namespace olx_analysis;
  TEBitArray masks(au.AtomCount());
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    if (a.IsMasked()) {
      masks.SetTrue(i);
    }
    a.SetMasked(a.GetType().z < 2);
  }
  smatd_list transforms;
  TTypeList<fragments::fragment> fs = fragments::extract(au);
  for (size_t i = 0; i < fs.Count(); i++) {
    TCAtomPList r_atoms;
    fs[i].breadth_first_tags(InvalidIndex, &r_atoms);
    TTypeList<fragments::ring> rs = fs[i].get_rings(r_atoms);
    for (size_t j = 0; j < rs.Count(); j++) {
      if (rs[j].atoms.Count() < 4 || rs[j].atoms.Count() > 6) {
        continue;
      }
      fragments::cart_ring ring = rs[j].to_cart();
      if (!ring.is_regular()) {
        continue;
      }
      TCAtomPList found;
      fragments::cart_plane cp = ring.calc_plane();
      if (cp.rmsd > 0.25) {
        continue;
      }
      TArrayList<AnAssociation3<TCAtom*, smatd, vec3d> > res;
      uc.FindInRangeAMC(au.Fractionalise(cp.center), 2.0, maxd, res);
      for (size_t k = 0; k < res.Count(); k++) {
        double ang = cp.angle(res[k].GetC() - cp.center);
        if (ang < maxa) {
          found << res[k].GetA();
          if (!transforms.Contains(res[k].GetB())) {
            transforms.AddCopy(res[k].GetB());
          }
        }
      }
      if (!found.IsEmpty()) {
        for (size_t k = 0; k < ring.atoms.Count(); k++) {
          if (!transforms.Contains(ring[k].matrix)) {
            transforms.AddCopy(ring[k].matrix);
          }
        }
        olxstr l = (alg::label(rs[j].atoms, '-') << ": ");
        for (size_t k = 0; k < found.Count(); k++) {
          l << ' ' << found[k]->GetLabel();
        }
        TBasicApp::NewLogEntry() << l;
      }
    }
  }
  for (size_t i = 0; i < au.AtomCount(); i++) {
    au.GetAtom(i).SetMasked(masks[i]);
  }
  if (Options.GetBoolOption('g') && !transforms.IsEmpty()) {
    TLattice& xlatt = xapp.XFile().GetLattice();
    ASObjectProvider& objects = xlatt.GetObjects();
    const TUnitCell& uc = xlatt.GetUnitCell();
    for (size_t i = 0; i < transforms.Count(); i++)
      uc.InitMatrixId(transforms[i]);
    TCAtomPList iatoms;
    for (size_t i = 0; i < objects.atoms.Count(); i++) {
      TSAtom& sa = objects.atoms[i];
      if (sa.IsDeleted()) {
        continue;
      }
      if (sa.IsAUAtom()) {
        iatoms.Add(sa.CAtom());
      }
    }
    xlatt.GrowAtoms(iatoms, transforms);
  }
}
//.............................................................................
void XLibMacros::funCrd(const TStrObjList& Params, TMacroData &E) {
  TSAtomPList Atoms = TXApp::GetInstance().FindSAtoms(Params, true, true);
  if (Atoms.IsEmpty()) {
    E.ProcessingError(__OlxSrcInfo, "could not find any atoms");
    return;
  }
  vec3d center;
  for (size_t i = 0; i < Atoms.Count(); i++) {
    center += Atoms[i]->crd();
  }
  center /= Atoms.Count();
  E.SetRetVal(olxstr::FormatFloat(3, center[0]) << ' ' <<
              olxstr::FormatFloat(3, center[1]) << ' ' <<
              olxstr::FormatFloat(3, center[2]));
}
//.............................................................................
void XLibMacros::funCCrd(const TStrObjList& Params, TMacroData &E)  {
  TSAtomPList Atoms = TXApp::GetInstance().FindSAtoms(Params, true, true);
  if (Atoms.IsEmpty()) {
    E.ProcessingError(__OlxSrcInfo, "could not find any atoms");
    return;
  }
  vec3d ccenter;
  for( size_t i=0; i < Atoms.Count(); i++ )
    ccenter += Atoms[i]->ccrd();
  ccenter /= Atoms.Count();
  E.SetRetVal(olxstr::FormatFloat(3, ccenter[0]) << ' ' <<
              olxstr::FormatFloat(3, ccenter[1]) << ' ' <<
              olxstr::FormatFloat(3, ccenter[2]));
}
//.............................................................................
void XLibMacros::macMolInfo(TStrObjList& Cmds, const TParamList& Options, TMacroData& Error) {
  TXApp& app = TXApp::GetInstance();
  TSAtomPList atoms = app.FindSAtoms(Cmds, true, true);
  if (atoms.IsEmpty()) {
    return;
  }
  typedef double float_type; // for generation >= 8, double ,ust be used...
  typedef TVector3<float_type> vec_type;
  TTypeList<TVector3<float_type> > verts;
  TTypeList<IndexTriangle> triags;
  const size_t generation = olx_min(10, Options.FindValue('g', '5').ToSizeT());
  if (Options.FindValue('s') == 't') {
    OlxSphere<float_type, TetrahedronFP<vec_type> >::Generate(1.0, generation, verts, triags);
  }
  else {
    OlxSphere<float_type, OctahedronFP<vec_type> >::Generate(1.0, generation, verts, triags);
  }

  float_type volume_p = 0, volume_a = 0, area_p = 0, area_a = 0;
  TArrayList<int8_t> t_map(atoms.Count() * triags.Count());
  app.PrintVdWRadii(ElementRadii(), app.XFile().GetAsymmUnit().GetContentList());
  for (size_t i = 0; i < atoms.Count(); i++) {
    const float_type r = (float_type)atoms[i]->GetType().r_vdw;
    volume_p += (float_type)olx_sphere_volume(r);
    area_p += (float_type)(4 * M_PI * r * r);
    const size_t off = i * triags.Count();
    for (size_t j = 0; j < triags.Count(); j++) {
      t_map[j + off] = 3;
      volume_a += olx_abs((verts[triags[j].vertices[0]] * r).DotProd(
        (verts[triags[j].vertices[1]] * r).XProdVec(
          (verts[triags[j].vertices[2]] * r))));
      area_a += ((verts[triags[j].vertices[1]] - verts[triags[j].vertices[0]]) * r).XProdVec(
        ((verts[triags[j].vertices[2]] - verts[triags[j].vertices[0]]) * r)).Length();
    }
  }
  volume_a /= 6;
  area_a /= 2;
  for (size_t i = 0; i < atoms.Count(); i++) {
    const TSAtom& a1 = *atoms[i];
    const float_type r_sq = (float_type)olx_sqr(a1.GetType().r_vdw);
    const vec_type center1 = a1.crd();
    for (size_t j = 0; j < atoms.Count(); j++) {
      if (i == j) {
        continue;
      }
      const TSAtom& a2 = *atoms[j];
      const float_type dist = (float_type)a1.crd().DistanceTo(a2.crd());
      if (dist >= (a1.GetType().r_vdw + a2.GetType().r_vdw)) {
        continue;
      }
      const float_type r = (float_type)a2.GetType().r_vdw;
      const vec_type center2 = a2.crd();
      const size_t off = triags.Count() * j;
      for (size_t k = 0; k < triags.Count(); k++) {
        if (t_map[k + off] == 0) {
          continue;
        }
        const float_type d[] = {
          (verts[triags[k].vertices[0]] * r + center2).QDistanceTo(center1),
          (verts[triags[k].vertices[1]] * r + center2).QDistanceTo(center1),
          (verts[triags[k].vertices[2]] * r + center2).QDistanceTo(center1)
        };
        if (d[0] < r_sq && d[1] < r_sq && d[2] < r_sq) {
          t_map[k + off] = 0;
        }
        else if ((d[0] < r_sq && (d[1] < r_sq || d[2] < r_sq)) || (d[1] < r_sq && d[2] < r_sq)) {
          t_map[k + off] = 1;
        }
        else if (d[0] < r_sq || d[1] < r_sq || d[2] < r_sq) {
          t_map[k + off] = 2;
        }

      }
    }
  }
  TVector3<float_type> mol_vol;
  float_type mol_area = 0;
  for (size_t i = 0; i < atoms.Count(); i++) {
    const size_t off = triags.Count() * i;
    const float_type r = (float_type)atoms[i]->GetType().r_vdw;
    const vec_type center = atoms[i]->crd();
    for (size_t j = 0; j < triags.Count(); j++) {
      if (t_map[off + j] == 0) {
        continue;
      }
      const vec_type
        v1 = verts[triags[j].vertices[0]] * r,
        v2 = verts[triags[j].vertices[1]] * r,
        v3 = verts[triags[j].vertices[2]] * r;
      vec_type dp = (v2 - v1).XProdVec(v3 - v1);
      const float_type m = (float_type)(1.0 / 2 * (float_type)t_map[off + j] / 3.0);
      mol_area += m * dp.Length();
      const float_type dx21 = v2[0] - v1[0],
        dx31 = v3[0] - v1[0],
        dy21 = v2[1] - v1[1],
        dy31 = v3[1] - v1[1],
        dz21 = v2[2] - v1[2],
        dz31 = v3[2] - v1[2];
      const TVector3<float_type> dv(
        (float_type)((1. / 3 * (v1[0] + v2[0] + v3[0]) + center[0]) * (dy21 * dz31 - dz21 * dy31)),
        (float_type)((1. / 3 * (v1[1] + v2[1] + v3[1]) + center[1]) * (dz21 * dx31 - dx21 * dz31)),
        (float_type)((1. / 3 * (v1[2] + v2[2] + v3[2]) + center[2]) * (dx21 * dy31 - dy21 * dx31)));
      mol_vol += dv * m;
    }
  }
  TBasicApp::NewLogEntry() << "Approximating spheres by " << triags.Count() <<
    " triangles";
  TBasicApp::NewLogEntry() << "Volume approximation error, %: " <<
    olxstr::FormatFloat(3, olx_abs(volume_p - volume_a) * 100.0 / volume_p);
  TBasicApp::NewLogEntry() << "Surface area approximation error, %: " <<
    olxstr::FormatFloat(3, olx_abs(area_p - area_a) * 100.0 / area_p);
  TBasicApp::NewLogEntry() << "Molecular surface area, A^2: " <<
    olxstr::FormatFloat(2, mol_area);
  TBasicApp::NewLogEntry() << "Molecular volume, A^3: " <<
    olxstr::FormatFloat(2, mol_vol.AbsSum() / 3);
}
//.............................................................................
void XLibMacros::macRTab(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  olxstr name = Cmds[0];
  TSAtomPList atoms = TXApp::GetInstance().FindSAtoms(Cmds.SubListFrom(1), true, true);
  if (atoms.IsEmpty()) {
    return;
  }
  if ((atoms.Count() >= 1 && atoms.Count() <= 4) ||
    (atoms.Count() > 1 && name.Equalsi("D2CG")))
  {
    RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
    InfoTab& it = rm.AddRTAB(name);
    for (size_t i = 0; i < atoms.Count(); i++) {
      it.AddAtom(atoms[i]->CAtom(), atoms[i]->GetMatrix().IsFirst() ? 0
        : &atoms[i]->GetMatrix());
    }
  }
  else {
    Error.ProcessingError(__OlxSrcInfo, "1 to 4 atoms is expected");
  }
}
//.............................................................................
void XLibMacros::macHklMerge(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olxstr merger = Options.FindValue("m");
  TXFile& xf = TXApp::GetInstance().XFile();
  TRefList refs;

  RefinementModel::HklStat ms;
  if (merger.Equals("standard")) {
    ms = xf.GetRM().GetRefinementRefList<
      TUnitCell::SymmSpace, RefMerger::StandardMerger>(
        xf.GetUnitCell().GetSymmSpace(), refs);
  }
  else if (merger.Equals("unit")) {
    ms = xf.GetRM().GetRefinementRefList<
      TUnitCell::SymmSpace, RefMerger::UnitMerger>(
        xf.GetUnitCell().GetSymmSpace(), refs);
  }
  else { // default
    ms = xf.GetRM().GetRefinementRefList<
      TUnitCell::SymmSpace, RefMerger::ShelxMerger>(
        xf.GetUnitCell().GetSymmSpace(), refs);
  }
  if (Options.Contains('z')) {
    for (size_t i=0; i < refs.Count(); i++) {
      if (refs[i].GetI() < 0) {
        refs[i].SetI(0);
      }
    }
  }
  TTTable<TStrList> tab(6, 2);
  tab[0][0] << "Total reflections";
    tab[0][1] << ms.GetReadReflections();
  tab[1][0] << "Unique reflections";
    tab[1][1] << ms.UniqueReflections;
  tab[2][0] << "Inconsistent equaivalents";
    tab[2][1] << ms.InconsistentEquivalents;
  tab[3][0] << "Systematic absences removed";
    tab[3][1] << ms.SystematicAbsencesRemoved;
  tab[4][0] << "Rint";
    tab[4][1] << ms.Rint;
  tab[5][0] << "Rsigma";
    tab[5][1] << ms.Rsigma;
  TBasicApp::NewLogEntry() <<
    tab.CreateTXTList("Merging statistics ", true, false, "  ");
  olxstr hklFileName = xf.GetRM().GetHKLSource();
  THklFile::SaveToFile(Cmds.IsEmpty() ? hklFileName : Cmds.Text(' '), refs);
}
//.............................................................................
void XLibMacros::macHklAppend(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TIntList h, k, l;
  bool combine = Options.FindValue("c", TrueString()).ToBool();
  TStrList toks( Options.FindValue('h', EmptyString()), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    h.Add(toks[i].ToInt());
  toks.Clear();
  toks.Strtok( Options.FindValue('k', EmptyString()), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    k.Add(toks[i].ToInt());
  toks.Clear();
  toks.Strtok( Options.FindValue('l', EmptyString()), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    l.Add(toks[i].ToInt());

  const olxstr hklSrc = TXApp::GetInstance().XFile().LocateHklFile();
  if( !TEFile::Exists( hklSrc ) )  {
    E.ProcessingError(__OlxSrcInfo, "could not find hkl file: ") << hklSrc;
    return;
  }
  THklFile Hkl;
  size_t c = 0;
  Hkl.LoadFromFile(hklSrc, false);
  if( Options.IsEmpty() )  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].IsOmitted() )  {
        Hkl[i].SetOmitted(false);
        c++;
      }
    }
  }
  else if( combine )  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].IsOmitted() )  {
        if( !h.IsEmpty() && h.IndexOf(Hkl[i].GetH()) == InvalidIndex ) continue;
        if( !k.IsEmpty() && k.IndexOf(Hkl[i].GetK()) == InvalidIndex ) continue;
        if( !l.IsEmpty() && l.IndexOf(Hkl[i].GetL()) == InvalidIndex ) continue;
        Hkl[i].SetOmitted(false);
        c++;
      }
    }
  }
  else  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].IsOmitted() )  {
        if( h.IndexOf(Hkl[i].GetH()) != InvalidIndex ||
            k.IndexOf(Hkl[i].GetK()) != InvalidIndex ||
            l.IndexOf(Hkl[i].GetL()) != InvalidIndex )  {
          Hkl[i].SetOmitted(false);
          c++;
        }
      }
    }
  }
  Hkl.SaveToFile(hklSrc);
  TBasicApp::NewLogEntry() << c << " reflections appended";
}
//.............................................................................
void XLibMacros::macHklExclude(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TIntList h, k, l;
  const bool combine = Options.FindValue("c", TrueString()).ToBool();
  TStrList toks(Options.FindValue('h', EmptyString()), ';');
  for (size_t i = 0; i < toks.Count(); i++) {
    h.Add(toks[i].ToInt());
  }
  toks.Clear();
  toks.Strtok(Options.FindValue('k', EmptyString()), ';');
  for (size_t i = 0; i < toks.Count(); i++) {
    k.Add(toks[i].ToInt());
  }
  toks.Clear();
  toks.Strtok(Options.FindValue('l', EmptyString()), ';');
  for (size_t i = 0; i < toks.Count(); i++) {
    l.Add(toks[i].ToInt());
  }

  const olxstr hklSrc = TXApp::GetInstance().XFile().LocateHklFile();
  if (!TEFile::Exists(hklSrc)) {
    E.ProcessingError(__OlxSrcInfo, "could not find hkl file: ") << hklSrc;
    return;
  }
  if (h.IsEmpty() && k.IsEmpty() && l.IsEmpty()) {
    E.ProcessingError(__OlxSrcInfo, "please provide a condition");
    return;
  }
  THklFile Hkl;
  size_t c = 0;
  Hkl.LoadFromFile(hklSrc, false);
  if (combine) {
    for (size_t i = 0; i < Hkl.RefCount(); i++) {
      if (!Hkl[i].IsOmitted()) {
        if (!h.IsEmpty() && h.IndexOf(Hkl[i].GetH()) == InvalidIndex) continue;
        if (!k.IsEmpty() && k.IndexOf(Hkl[i].GetK()) == InvalidIndex) continue;
        if (!l.IsEmpty() && l.IndexOf(Hkl[i].GetL()) == InvalidIndex) continue;
        Hkl[i].SetOmitted(true);
        c++;
      }
    }
  }
  else {
    for (size_t i = 0; i < Hkl.RefCount(); i++) {
      if (!Hkl[i].IsOmitted()) {
        if ((!h.IsEmpty() && h.IndexOf(Hkl[i].GetH()) != InvalidIndex) ||
          (!k.IsEmpty() && k.IndexOf(Hkl[i].GetK()) != InvalidIndex) ||
          (!l.IsEmpty() && l.IndexOf(Hkl[i].GetL()) != InvalidIndex))
        {
          Hkl[i].SetOmitted(true);
          c++;
        }
      }
    }
  }
  Hkl.SaveToFile(hklSrc);
  TBasicApp::NewLogEntry() << c << " reflections excluded";
}
//.............................................................................
void XLibMacros::macHklImport(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TStrList lines = TEFile::ReadLines(Cmds[0]);
  const olxstr out_name = Cmds.GetLastString();
  Cmds.Delete(Cmds.Count() - 1);
  if (Cmds[1].Equalsi("fixed")) {
    TSizeList format;
    for (size_t i = 2; i < Cmds.Count(); i++)
      format.Add(Cmds[i].ToSizeT());
    if (format.Count() < 5 || format.Count() > 6) {
      E.ProcessingError(__OlxSrcInfo, "5 or 6 numbers are expected");
      return;
    }
    TRefList refs;
    for (size_t i = 0; i < lines.Count(); i++) {
      TStrList toks;
      if (toks.StrtokF(lines[i], format) < format.Count())
        continue;
      TReflection& r = refs.AddNew(
        toks[0].ToInt(), toks[1].ToInt(), toks[2].ToInt()
      );
      r.SetI(toks[3].ToDouble());
      r.SetS(toks[4].ToDouble());
      if (format.Count() == 6)
        r.SetBatch(toks[5].ToInt());
    }
    THklFile::SaveToFile(out_name, refs);
  }
  else if (Cmds[1].Equalsi("separator")) {
    if (Cmds[2].IsEmpty()) {
      E.ProcessingError(__OlxSrcInfo, "non-empty separator is expected");
      return;
    }
    const bool has_batch = Options.Contains("batch");
    TRefList refs;
    const olxch sep = Cmds[2].CharAt(0);
    for (size_t i = 0; i < lines.Count(); i++) {
      TStrList toks(lines[i], sep);
      if (toks.Count() < 5) {
        continue;
      }
      TReflection& r = refs.AddNew(
        toks[0].ToInt(), toks[1].ToInt(), toks[2].ToInt()
      );
      r.SetI(toks[3].ToDouble());
      r.SetS(toks[4].ToDouble());
      if (has_batch && toks.Count() >= 6) {
        r.SetBatch(toks[5].ToInt());
      }
    }
    THklFile::SaveToFile(out_name, refs);
  }
  else {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined keyword: ") << Cmds[1]);
  }
}
//.............................................................................
void XLibMacros::macHklSplit(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double percents = -1, th = -1;
  if (Cmds[0].EndsWith('%')) {
    percents = Cmds[0].SubStringFrom(0, 1).ToDouble();
    if (percents <= 0 || percents >= 100) {
      E.ProcessingError(__OlxSrcInfo, "(0..100)% Is expected");
      return;
    }
  }
  else {
    th = Cmds[0].ToDouble();
    if (th < 0) {
      E.ProcessingError(__OlxSrcInfo, "A positive number is expected");
      return;
    }
  }
  if (!(Cmds[1].Equalsi('i') || Cmds[1].Equalsi('a'))) {
    E.ProcessingError(__OlxSrcInfo, "Second argument can be only 'i' or 'a'");
    return;
  }

  TXApp &app = TXApp::GetInstance();
  TUnitCell::SymmSpace sp =
    app.XFile().GetLattice().GetUnitCell().GetSymmSpace();
  SymmSpace::InfoEx info_ex = SymmSpace::Compact(sp);

  TRefList refs;
  evecd Fsq;
  RefinementModel::HklStat stats = app.CalcFsq(refs, Fsq, true);
  TArrayList<double> fracts(refs.Count());
  if (Cmds[1].Equalsi('a')) {
    for (size_t i = 0; i < refs.Count(); i++) {
      fracts[i] = olx_abs(refs[i].GetI() - Fsq[i]) / (refs[i].GetS() + 1e-6);
    }
  }
  else { // I/esd
    for (size_t i = 0; i < refs.Count(); i++) {
      fracts[i] = refs[i].GetI() / (refs[i].GetS() + 1e-6);
    }
  }

  TArray3D<size_t> hkl3d(stats.MinIndexes, stats.MaxIndexes);
  hkl3d.FastInitWith(0);
  for (size_t i = 0; i < refs.Count(); i++) {
    hkl3d(refs[i].GetHkl()) = i + 1;
  }
  TRefList all_refs;
  RefinementModel::HklStat stats1;
  app.XFile().GetRM().FilterHkl(all_refs, stats1);
  sorted::PrimitiveAssociation<double, TReflection*> sorted;
  sorted.SetCapacity(all_refs.Count());
  for (size_t i = 0; i < all_refs.Count(); i++) {
    if (TReflection::IsAbsent(all_refs[i].GetHkl(), info_ex)) {
      stats1.SystematicAbsencesRemoved++;
      all_refs.NullItem(i);
      continue;
    }
    vec3i hkl = TReflection::Standardise(all_refs[i].GetHkl(), info_ex);
    if (hkl3d.IsInRange(hkl)) {
      size_t idx = hkl3d(hkl);
      if (idx != 0) {
        sorted.Add(fracts[idx - 1], &all_refs[i]);
      }
      else {
        throw TFunctionFailedException(__OlxSourceInfo,
          "Miller arrays mismatch");
      }
    }
    else {
      throw TFunctionFailedException(__OlxSourceInfo,
        "Miller arrays mismatch");
    }
  }
  if (stats1.SystematicAbsencesRemoved != 0) {
    all_refs.Pack();
    TBasicApp::NewLogEntry() << stats1.SystematicAbsencesRemoved <<
      " Systematic absences is removed.";
  }
  if (percents > 0) {
    th = sorted.GetKey(olx_round(all_refs.Count()*percents / 100));
    TBasicApp::NewLogEntry() << "Threshold calculated for " << percents <<
      "% is " << olxstr::FormatFloat(2, th);
  }
  if (all_refs.IsEmpty()) {
    return;
  }
  olxstr fn = TEFile::ChangeFileExt(app.XFile().GetRM().GetHKLSource(),
    EmptyString());
  size_t cnt = 0;
  if (Options.GetBoolOption('b')) {
    for (size_t i = 0; i < sorted.Count(); i++) {
      if (sorted.GetKey(i) < th) {
        sorted.GetValue(i)->SetBatch(1);
        cnt++;
      }
      else {
        sorted.GetValue(i)->SetBatch(2);
      }
    }
    fn = olx_print("%w_hf5.hkl", &fn);
    THklFile::SaveToFile(fn, all_refs);
    TBasicApp::NewLogEntry() << cnt << " Reflection is assigned batch 1 and " <<
      (sorted.Count() - cnt) << " reflection assigned batch 2 and written to '" <<
      TEFile::ExtractFileName(fn) << " file.";
  }
  else {
    const size_t ref_str_len = all_refs[0].ToString().Length();
    const size_t bf_sz = ref_str_len + 10;
    olx_array_ptr<char> ref_bf(new char[bf_sz]);
    char ffs = Cmds[1].Equalsi('i') ? 'w' : 'a',
      sfs = Cmds[1].Equalsi('i') ? 's' : 'd';
    TEFile a(olx_print("%w_%c.hkl", &fn, ffs), "w+b"),
      d(olx_print("%w_%c.hkl", &fn, sfs), "w+b");
    for (size_t i = 0; i < sorted.Count(); i++) {
      if (sorted.GetKey(i) < th) {
        a.Writecln(sorted.GetValue(i)->ToCBuffer(ref_bf, bf_sz, 1), ref_str_len);
        cnt++;
      }
      else {
        d.Writecln(sorted.GetValue(i)->ToCBuffer(ref_bf, bf_sz, 1), ref_str_len);
      }
    }
    TBasicApp::NewLogEntry() << cnt << " Reflection is written to '" <<
      TEFile::ExtractFileName(a.GetName()) << "' and " << (sorted.Count() - cnt) <<
      " reflection is written to '" << TEFile::ExtractFileName(d.GetName()) <<
      " files.";
  }
}
//.............................................................................
void HKLCreate(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olxstr out_name = Cmds[0];
  if (!out_name.EndsWithi(".hkl")) {
    out_name << ".hkl";
  }
  double wl = Cmds[1].ToDouble(),
    r = Cmds[2].ToDouble();
  if (r < wl / 2) {
    r = wl / 2;
  }
  TXApp& app = TXApp::GetInstance();
  vec3i idx = app.XFile().GetRM().CalcMaxHklIndexForD(r);
  MillerIndexArray ma = MillerIndexArray(-idx, idx);
  TArrayList<compd> F(ma.Count());
  SFUtil::CalcSF(app.XFile(), ma, F);
  TRefList refs(F.Count(), false);
  for (size_t i = 0; i < F.Count(); i++) {
    refs.Set(i, new TReflection(ma[i], F[i].qmod(), 0.0));
    refs[i].SetS((rand()%10)*refs[i].GetI()/100 + 0.01);
  }
  THklFile hkl;
  hkl.Append(refs);
  hkl.SaveToFile(out_name);
}
//.............................................................................
void XLibMacros::macUpdate(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp& app = TXApp::GetInstance();
  olx_object_ptr<TXFile> xf(dynamic_cast<TXFile*>(app.XFile().Replicate()));
  xf->LoadFromFile(Cmds.Text(' '));
  RefinementModel &this_rm = app.XFile().GetRM(),
    &that_rm = xf->GetRM();
  if( !this_rm.Update(that_rm) )  {
    E.ProcessingError(__OlxSrcInfo, "Asymmetric units do not match");
    return;
  }
  app.XFile().EndUpdate();
}
//.............................................................................
void XLibMacros::funCalcR(const TStrObjList& Params, TMacroData &E) {
  TXApp& xapp = TXApp::GetInstance();
  RefUtil::Stats rstat(!Params.IsEmpty() && Params[0].Contains("scale"));
  bool print = !Params.IsEmpty() && Params[0].Containsi("print");
  if (print) {
    xapp.NewLogEntry() << "R1 (All, " << rstat.refs.Count() << ") = "
      << olxstr::FormatFloat(4, rstat.R1);
    xapp.NewLogEntry() << "R1 (I/sig >= 2, " << rstat.partial_R1_cnt << ") = "
      << olxstr::FormatFloat(4, rstat.R1_partial);
    xapp.NewLogEntry() << "wR2 = " << olxstr::FormatFloat(4, rstat.wR2);
  }
  E.SetRetVal(
    olxstr(rstat.R1) << ',' << rstat.R1_partial << ',' << rstat.wR2);
  if (!print) {
    return;
  }
  // some test code for Flack calculation
  TRefPList pos_, neg_;
  TRefList refs;
  vec3i mini(1000), maxi(-1000);
  double th = 0;
  for (size_t i = 0; i < rstat.refs.Count(); i++) {
//    if (rstat.refs[i].GetI() < rstat.refs[i].GetS() * th) {
//      continue;
//    }
    refs.AddCopy(rstat.refs[i]);
    vec3i::UpdateMinMax(rstat.refs[i].GetHkl(), mini, maxi);
  }
  RefUtil::GetBijovetPairs(refs, mini, maxi, pos_, neg_,
    xapp.XFile().GetLastLoaderSG().GetMatrices(mattAll).GetObject());
  double up = 0, down = 0;
  for (int p = 0; p <= 1; p++) {
    TRefPList *pos = p == 0 ? &pos_ : &neg_,
      *neg = p == 0 ? &neg_ : &pos_;
    for (size_t i = 0; i < pos_.Count(); i++) {
      double w = rstat.weights[(*pos)[i]->GetTag()];
      double D1 = (*pos)[i]->GetI() - rstat.Fsq[(*pos)[i]->GetTag()];
      double D2 = rstat.Fsq[(*neg)[i]->GetTag()] - rstat.Fsq[(*pos)[i]->GetTag()];
      up += w * D1 * D2;
      down += w * D2 * D2;
    }
  }
  double k = up / down, obj = 0, obj_d = 0;
  for (int p = 0; p <= 1; p++) {
    TRefPList* pos = p == 0 ? &pos_ : &neg_,
      * neg = p == 0 ? &neg_ : &pos_;
    for (size_t i = 0; i < pos_.Count(); i++) {
      double w = rstat.weights[(*pos)[i]->GetTag()];
      double D1 = (*pos)[i]->GetI() - rstat.Fsq[(*pos)[i]->GetTag()];
      double D2 = rstat.Fsq[(*neg)[i]->GetTag()] - rstat.Fsq[(*pos)[i]->GetTag()];
      obj += w*olx_sqr((D1 - k*D2));
      obj_d += w*D2 * D2;
    }
  }
  double esd = obj / ((pos_.Count() - 1)*obj_d);

  TBasicApp::NewLogEntry() << "X: " << TEValueD(k, sqrt(esd)).ToString() << " for "
    << pos_.Count() << " pairs";
}
//.............................................................................
olxstr XLibMacros::GetCompilationInfo() {
  olxstr timestamp(olxstr(__DATE__) << ' ' << __TIME__), revision;
#ifdef _SVN_REVISION_AVAILABLE
  timestamp = compile_timestamp;
  revision = svn_revision_number;
#endif
  olxstr rv = timestamp;
  if (!revision.IsEmpty()) {
    rv << " svn.r" << revision;
  }
#ifdef _MSC_FULL_VER
  rv << " MSC:" << _MSC_FULL_VER;
#elif __INTEL_COMPILER
  rv << " Intel:" << __INTEL_COMPILER;
#elif __GNUC__
  rv << " GCC:" << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__;
#endif
  rv << " on " << TBasicApp::GetPlatformString(true);
#ifdef _CUSTOM_BUILD_
  rv << " for " << CustomCodeBase::GetName();
#endif
  return rv;
}
//.............................................................................
void XLibMacros::funGetCompilationInfo(const TStrObjList& Params,
  TMacroData &E)
{
  olxstr timestamp(olxstr(__DATE__) << ' ' << __TIME__), revision;
#ifdef _SVN_REVISION_AVAILABLE
  timestamp = compile_timestamp;
  revision = svn_revision_number;
#endif
  if( Params.IsEmpty() )  {
    if( revision.IsEmpty() )
      E.SetRetVal(timestamp);
    else
      E.SetRetVal(timestamp << " svn.r" << revision);
  }
  else  {
    if( Params.Count() == 1 && Params[0].Equalsi("full") )  {
      E.SetRetVal(GetCompilationInfo());
    }
    else  {
      try {
        time_t date = TETime::ParseDate(__DATE__);
        time_t time = TETime::ParseTime(__TIME__);
        E.SetRetVal<olxstr>(TETime::FormatDateTime(Params[0], date+time));
      }
      catch( TExceptionBase& ) {
        E.SetRetVal(timestamp);
      }
    }
  }
}
//.............................................................................
void XLibMacros::macMove(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  TSAtomPList atoms =
    app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  if (atoms.IsEmpty()) {
    app.XFile().GetLattice().MoveToCenter();
  }
  else if (atoms.Count() == 2) {
    if (Options.Contains('c'))
      app.XFile().GetLattice().MoveFragmentG(*atoms[0], *atoms[1]);
    else
      app.XFile().GetLattice().MoveFragment(*atoms[0], *atoms[1]);
  }
  else
    E.ProcessingError(__OlxSrcInfo, "Two or none atoms is expected");
}
//.............................................................................
void XLibMacros::macFvar(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double fvar = -1101, k = -1;
  XLibMacros::Parse(Cmds, "dd", &fvar, &k);
  if( k < 0 && olx_abs(fvar-olx_round(fvar)) > 1e-1 )  {
    double fvv = int(fvar)/10;
    k = olx_abs(fvar - fvv*10);
    fvar = fvv;
  }
  TXApp &app = TXApp::GetInstance();
  RefinementModel& rm = app.XFile().GetRM();
  TCAtomPList atoms(
    app.FindSAtoms(Cmds, false, !Options.Contains("cs")),
    FunctionAccessor::MakeConst(&TSAtom::CAtom));
  ACollectionItem::Unify(atoms);
  atoms.ForEach(ACollectionItem::TagSetter(0));
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( atoms[i]->DependentHfixGroupCount() == 1 )  {
      TAfixGroup &ag = atoms[i]->GetDependentHfixGroup(0);
      for( size_t j=0; j < ag.Count(); j++ )
        ag[j].SetTag(1);
    }
  }
  atoms.Pack(olx_alg::olx_not(ACollectionItem::TagAnalyser(0)));
  if( fvar == -1101 && ((atoms.Count()%2) != 0 || atoms.IsEmpty()) )  {
    rm.Vars.Validate();
    TBasicApp::NewLogEntry() << "Free variables: " << rm.Vars.GetFVARStr();
    return;
  }
  if( (atoms.Count()%2)==0 && fvar == -1101 )  {
    if( !atoms.IsEmpty() )  {
      XVar& xv = rm.Vars.NewVar();
      for( size_t i=0; i < atoms.Count()/2; i++ )  {
        TCAtom& a = *atoms[i];
        if( a.DependentHfixGroupCount() == 1 )  {
          TAfixGroup &ag = a.GetDependentHfixGroup(0);
          for( size_t j=0; j < ag.Count(); j++ )  {
            rm.Vars.AddVarRef(xv, ag[j], catom_var_name_Sof,
              relation_AsVar, 1.0/ag[j].GetDegeneracy());
          }
        }
        rm.Vars.AddVarRef(xv, a, catom_var_name_Sof, relation_AsVar,
          1.0/a.GetDegeneracy());
        TCAtom& b = *atoms[atoms.Count()/2+i];
        if( b.DependentHfixGroupCount() == 1 )  {
          TAfixGroup &ag = b.GetDependentHfixGroup(0);
          for( size_t j=0; j < ag.Count(); j++ )  {
            rm.Vars.AddVarRef(xv, ag[j], catom_var_name_Sof,
              relation_AsOneMinusVar, 1.0/ag[j].GetDegeneracy());
          }
        }
        rm.Vars.AddVarRef(xv, b, catom_var_name_Sof, relation_AsOneMinusVar,
          1.0/b.GetDegeneracy());
      }
    }
  }
  else  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      double val;
      if( k < 0 )  {
        val = atoms[i]->GetOccu();
      }
      else
        val = k/atoms[i]->GetDegeneracy();
      rm.Vars.SetParam(*atoms[i], catom_var_name_Sof,
        olx_sign(fvar)*(olx_abs(fvar)*10+val));
      if( atoms[i]->DependentHfixGroupCount() == 1 )  {
        TAfixGroup &ag = atoms[i]->GetDependentHfixGroup(0);
        for( size_t j=0; j < ag.Count(); j++ )  {
          XVarReference *v_ref = ag[j].GetVarRef(catom_var_name_Sof);
          if (k < 0) {
            val = ag[j].GetOccu();
          }
          else {
            val = k / ag[j].GetDegeneracy();
          }
          rm.Vars.SetParam(ag[j], catom_var_name_Sof,
            olx_sign(fvar)*(olx_abs(fvar)*10+val));
        }
      }
    }
  }
}
//.............................................................................
void XLibMacros::macSump(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  RefinementModel& rm = app.XFile().GetRM();
  double val = 1, esd = 0.01;
  XLibMacros::Parse(Cmds, "dd", &val, &esd);
  TSAtomPList xatoms = app.FindSAtoms(Cmds, false, true);
  // create a list of unique catoms
  for (size_t i = 0; i < xatoms.Count(); i++) {
    xatoms[i]->CAtom().SetTag(i);
  }
  TCAtomPList catoms;
  size_t p_cnt = 0, v_cnt = 0;
  for (size_t i = 0; i < xatoms.Count(); i++) {
    if (xatoms[i]->CAtom().GetTag() == i) {
      TCAtom &a = xatoms[i]->CAtom();
      catoms.Add(a);
      if (a.GetVarRef(catom_var_name_Sof) == 0 ||
        a.GetVarRef(catom_var_name_Sof)->relation_type == relation_None)
      {
        v_cnt++;
      }
      if (a.GetPart() != 0) {
        p_cnt++;
      }
    }
  }
  if (catoms.Count() < 2) {
    E.ProcessingError(__OlxSrcInfo,
      "at least two unique atoms should be provided");
    return;
  }
  XLEQ& xeq = rm.Vars.NewEquation(val, esd);
  // special case: all of the FVAR are set, use them
  if (v_cnt == 0) {
    olxdict<XVar*, int, TPointerComparator> vars;
    for (size_t i = 0; i < catoms.Count(); i++) {
      XVar &v = catoms[i]->GetVarRef(catom_var_name_Sof)->Parent;
      size_t v_idx = vars.IndexOf(&v);
      if (v_idx == InvalidIndex) {
        vars.Add(&v, 1);
      }
      else {
        vars.GetValue(v_idx)++;
      }
    }
    for (size_t i = 0; i < vars.Count(); i++) {
      xeq.AddMember(*vars.GetKey(i), vars.GetValue(i));
    }
  }
  // special case - all in parts
  else if (p_cnt == catoms.Count()) {
    typedef olx_pair_t<XVar*, int> var_t;
    olx_pdict<int, var_t> vars;
    // collect and analyse FVARs if any
    for (size_t i = 0; i < catoms.Count(); i++) {
      XVar *v;
      XVarReference *vr = catoms[i]->GetVarRef(catom_var_name_Sof);
      if (vr != 0 && vr->relation_type == relation_None) {
        vr = 0;
      }
      size_t p_idx = vars.IndexOf(catoms[i]->GetPart());
      if (p_idx == InvalidIndex) {
        if (vr != 0) {
          v = vars.Add(catoms[i]->GetPart(), var_t(&vr->Parent, 1)).a;
        }
      }
      else {
        var_t &vr_ = vars.GetValue(p_idx);
        if (vr != 0) {
          if (&vr->Parent != vr_.a) {
            E.ProcessingError(__OlxSrcInfo,
              "Inconsistent use of parts and variables, aborting");
            return;
          }
        }
        vr_.b++;
      }
    }
    // create new vars as needed
    for (size_t i = 0; i < catoms.Count(); i++) {
      XVar *v;
      size_t p_idx = vars.IndexOf(catoms[i]->GetPart());
      if (p_idx == InvalidIndex) {
        v = &rm.Vars.NewVar(1. / catoms.Count());
        v = vars.Add(catoms[i]->GetPart(), var_t(v, 1)).a;
      }
      else {
        var_t &vr = vars.GetValue(p_idx);
        vr.b++;
        v = vr.a;
      }
      rm.Vars.AddVarRef(*v, *catoms[i], catom_var_name_Sof, relation_AsVar,
        1.0 / catoms[i]->GetDegeneracy());
    }
    for (size_t i = 0; i < vars.Count(); i++) {
      var_t &vr = vars.GetValue(i);
      xeq.AddMember(*vr.a, vr.b);
    }
  }
  // generic case, disregard parts, create FVARs if not set
  else {
    olxdict<XVar*, int, TPointerComparator> vars;
    for (size_t i = 0; i < catoms.Count(); i++) {
      XVar *xv;
      if (catoms[i]->GetVarRef(catom_var_name_Sof) == 0 ||
        catoms[i]->GetVarRef(catom_var_name_Sof)->relation_type == relation_None)
      {
        xv = &rm.Vars.NewVar(1. / catoms.Count());
        rm.Vars.AddVarRef(*xv, *catoms[i], catom_var_name_Sof, relation_AsVar,
          1.0 / catoms[i]->GetDegeneracy());
      }
      else {
        xv = &catoms[i]->GetVarRef(catom_var_name_Sof)->Parent;
      }
      size_t v_idx = vars.IndexOf(xv);
      if (v_idx == InvalidIndex) {
        vars.Add(xv, 1);
      }
      else {
        vars.GetValue(v_idx)++;
      }
    }
    for (size_t i = 0; i < vars.Count(); i++) {
      xeq.AddMember(*vars.GetKey(i), vars.GetValue(i));
    }
  }
}
//.............................................................................
void XLibMacros::macPart(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  RefinementModel& rm = app.XFile().GetRM();
  int part = DefNoPart;
  double occu = 0;
  XLibMacros::Parse(Cmds, "id", &part, &occu);
  const size_t partCount = Options.FindValue("p", "1").ToSizeT();
  const bool linkOccu = Options.GetBoolOption("lo");
  const bool copy = Options.GetBoolOption("c");

  TSAtomPList Atoms = app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  if (partCount == 0 || (Atoms.Count() % partCount) != 0) {
    E.ProcessingError(__OlxSrcInfo, "wrong number of parts");
    return;
  }
  if (copy) {
    if (part == DefNoPart) {
      part = rm.aunit.GetNextPart(true);
    }
    XVar& xv = rm.Vars.NewVar(0.5);
    for (size_t i = 0; i < Atoms.Count(); i++) {
      if (!Atoms[i]->GetMatrix().IsFirst()) {
        TCAtom& ca = rm.aunit.NewAtom();
        ca.Assign(Atoms[i]->CAtom());
        ca.ccrd() = Atoms[i]->ccrd();
        ca.SetPart(part);
        rm.Vars.AddVarRef(xv, Atoms[i]->CAtom(), catom_var_name_Sof,
          relation_AsVar, 1.0 / Atoms[i]->CAtom().GetDegeneracy());
        rm.Vars.AddVarRef(xv, ca, catom_var_name_Sof, relation_AsVar,
          1.0 / Atoms[i]->CAtom().GetDegeneracy());
      }
      Atoms[i]->CAtom().SetPart(part);
    }
    app.XFile().EndUpdate();
    if (Options.GetBoolOption('f', false, true)) {
      olex2::IOlex2Processor::GetInstance()->processMacro("fuse 0.05",
        __OlxSrcInfo);
    }
    return;
  }
  XVar* xv = 0;
  XLEQ* leq = 0;
  if (linkOccu) {
    // -21 -> 21
    if (partCount == 2) {
      xv = &rm.Vars.NewVar(0.5);
    }
    // SUMP
    if (partCount > 2) {
      leq = &rm.Vars.NewEquation(1.0, 0.01);
    }
  }

  if (part == DefNoPart) {
    part = rm.aunit.GetNextPart();
  }

  for (size_t i = 0; i < partCount; i++) {
    if (partCount > 2) {
      xv = &rm.Vars.NewVar(1. / partCount);
      leq->AddMember(*xv);
    }
    for (size_t j = (Atoms.Count() / partCount)*i;
      j < (Atoms.Count() / partCount)*(i + 1); j++)
    {
      sorted::PointerPointer<TCAtom> atoms;
      atoms.Add(&Atoms[j]->CAtom());
      if (Atoms[j]->GetType() > 1) {
        for (size_t k = 0; k < Atoms[j]->CAtom().AttachedSiteCount(); k++) {
          TCAtom &aa = Atoms[j]->CAtom().GetAttachedAtom(k);
          if (aa.GetType() == iHydrogenZ) {
            if (aa.GetParentAfixGroup() != 0 &&
              aa.GetParentAfixGroup()->GetPivot() != Atoms[j]->CAtom())
            {
              continue;
            }
            atoms.AddUnique(&aa);
          }
        }
      }
      for (size_t k = 0; k < atoms.Count(); k++) {
        atoms[k]->SetPart(part);
        if (linkOccu) {
          if (partCount == 2) {
            if (i != 0) {
              rm.Vars.AddVarRef(*xv, *atoms[k], catom_var_name_Sof,
                relation_AsVar, 1.0 / atoms[k]->GetDegeneracy());
            }
            else {
              rm.Vars.AddVarRef(*xv, *atoms[k], catom_var_name_Sof,
                relation_AsOneMinusVar, 1.0 / atoms[k]->GetDegeneracy());
            }
          }
          if (partCount > 2) {
            rm.Vars.AddVarRef(*xv, *atoms[k], catom_var_name_Sof,
              relation_AsVar, 1.0 / atoms[k]->GetDegeneracy());
          }
        }
        else if (occu != 0) {
          app.XFile().GetRM().Vars.SetParam(*atoms[k], catom_var_name_Sof, occu);
        }
      }
    }
    part++;
  }
  app.XFile().GetLattice().UpdateConnectivityInfo();
}
//.............................................................................
void XLibMacros::macSpec(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double spec = 0.2;
  XLibMacros::Parse(Cmds, "d", &spec);
  TXApp &app = TXApp::GetInstance();
  TSAtomPList atoms = app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i]->CAtom().SetSpecialPositionDeviation(spec);
  }
}
//.............................................................................
void XLibMacros::macAfix(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  int afix = -1;
  XLibMacros::Parse(Cmds, "i", &afix);
  if (afix == -1) {
    E.ProcessingError(__OlxSrcInfo, "afix should be specified");
    return;
  }
  olxstr positions;
  if (Cmds.Count() > 0 && Cmds[0].Contains(',')) {
    positions = Cmds[0];
    Cmds.Delete(0);
  }
  TXApp &app = TXApp::GetInstance();
  RefinementModel& rm = app.XFile().GetRM();
  TSAtomPList Atoms = app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  const int m = TAfixGroup::GetM(afix);
  if (TAfixGroup::IsFittedRing(afix)) {  // special case
    // yet another special case
    if (!positions.IsEmpty()) {
      TBasicApp::NewLogEntry() <<
        "Generating missing atoms and applying the AFIX";
      TStrList toks(positions, ',');
      if (toks.Count() < 3) {
        E.ProcessingError(__OlxSrcInfo, "at least 3 atoms should be provided");
        return;
      }
      if (toks.Count() != Atoms.Count()) {
        E.ProcessingError(__OlxSrcInfo,
          "mismatching number of positions and given atoms");
        return;
      }
      TArrayList<int> pos(toks.Count());
      vec3d_list crds;
      if (m == 5) {
        Fragment::GenerateFragCrds(frag_id_cp, crds);
      }
      else if (m == 6 || m == 7) {
        Fragment::GenerateFragCrds(frag_id_ph, crds);
      }
      else if (m == 10) {
        Fragment::GenerateFragCrds(frag_id_cp_star, crds);
      }
      else if (m == 11) {
        Fragment::GenerateFragCrds(frag_id_naphthalene, crds);
      }
      for (size_t i = 0; i < toks.Count(); i++) {
        const int v = toks[i].ToInt() - 1;
        if (i > 0 && pos[i - 1] >= v) {
          E.ProcessingError(__OlxSrcInfo,
            "please provide position in the ascending order");
          return;
        }
        if (v < 0 || v >= (int)crds.Count()) {
          E.ProcessingError(__OlxSrcInfo, "invalid ring position");
          return;
        }
        pos[i] = v;
      }
      TTypeList<AnAssociation3<TCAtom*, const cm_Element*, bool> > atoms;
      const cm_Element& carb = XElementLib::GetByIndex(iCarbonIndex);
      for (size_t i = 0; i < pos.Count(); i++) {
        while (pos[i] > 0 && (pos[i] > (int)atoms.Count())) {
          atoms.Add(new AnAssociation3<TCAtom*, const cm_Element*, bool>(
            0, &carb, false));
        }
        atoms.Add(new AnAssociation3<TCAtom*, const cm_Element*, bool>(
          &Atoms[i]->CAtom(), (const cm_Element*)NULL, true));
      }
      while (atoms.Count() < crds.Count()) {
        atoms.Add(new AnAssociation3<TCAtom*, const cm_Element*, bool>(
          0, &carb, false));
      }
      app.XFile().GetAsymmUnit().FitAtoms(atoms, crds, false);
      TAfixGroup& ag = app.XFile().GetRM().AfixGroups.New(
        atoms[pos[0]].a, afix);
      for (size_t i = pos[0] + 1; i < atoms.Count(); i++) {
        ag.AddDependent(*atoms[i].a);
      }
      for (int i = 0; i < pos[i]; i++) {
        ag.AddDependent(*atoms[i].a);
      }
      app.XFile().EndUpdate();
      return;
    }
    if (Atoms.IsEmpty()) {
      app.AutoAfixRings(afix, 0, Options.Contains('n'));
    }
    else if (Atoms.Count() == 1) {
      app.AutoAfixRings(afix, Atoms[0], Options.Contains('n'));
    }
    else {
      if ((m == 5) && Atoms.Count() != 5) {
        E.ProcessingError(__OlxSrcInfo, "please provide 5 atoms exactly");
        return;
      }
      else if ((m == 6 || m == 7) && Atoms.Count() != 6) {
        E.ProcessingError(__OlxSrcInfo, "please provide 6 atoms exactly");
        return;
      }
      else if ((m == 10 || m == 11) && Atoms.Count() != 10) {
        E.ProcessingError(__OlxSrcInfo, "please provide 10 atoms exactly");
        return;
      }
      if (!olx_analysis::alg::are_all_I(Atoms)) {
        TBasicApp::NewLogEntry() <<
          "Skipping - asymmetric unit atoms are expected";
      }
      else {
        if (Options.GetBoolOption("s", false, true) && (m == 5 || m == 6)) {
          vec3d normal, center;
          TSAtomPList atoms = Atoms;
          TSPlane::CalcPlane(atoms, normal, center);
          olx_plane::Sort(atoms, FunctionAccessor::MakeConst(
            (const vec3d& (TSAtom::*)() const)&TSAtom::crd), center, normal);
          if (atoms[0] != Atoms[0]) {
            size_t idx = atoms.IndexOf(Atoms[0]);
            atoms.ShiftL(idx);
          }
          bool match = true;
          for (size_t i = 0; i < Atoms.Count(); i++) {
            if (atoms[i] != Atoms[i]) {
              match = false;
              break;
            }
          }
          if (!match) {
            TBasicApp::NewLogEntry() << "The atom order has been changed to "
              "follow the ring direction. Consider using -s option to disable "
              "this.";
          }
          Atoms = atoms;
        }
        TBasicApp::NewLogEntry() << "Applying AFIX " << afix << " to " <<
          olx_analysis::alg::label(Atoms, true, ' ');
        if (Atoms[0]->CAtom().GetDependentAfixGroup() != 0) {
          Atoms[0]->CAtom().GetDependentAfixGroup()->Clear();
        }
        TAfixGroup& ag = rm.AfixGroups.New(&Atoms[0]->CAtom(), afix);
        for (size_t i = 1; i < Atoms.Count(); i++) {
          TCAtom& ca = Atoms[i]->CAtom();
          if (ca.GetParentAfixGroup() != 0) {
            TBasicApp::NewLogEntry() << "Removing intersecting AFIX group: ";
            TBasicApp::NewLogEntry() << ca.GetParentAfixGroup()->ToString();
            ca.GetParentAfixGroup()->Clear();
          }
          if (ca.GetDependentAfixGroup() != 0 &&
            ca.GetDependentAfixGroup()->GetAfix() == afix)
          {
            TBasicApp::NewLogEntry() << "Removing potentially intersecting AFIX"
              " group: ";
            TBasicApp::NewLogEntry() << ca.GetDependentAfixGroup()->ToString();
            ca.GetDependentAfixGroup()->Clear();
          }
        }
        for (size_t i = 1; i < Atoms.Count(); i++) {
          ag.AddDependent(Atoms[i]->CAtom());
        }
      }
    }
  }
  else {
    if (afix == 0) {
      for (size_t i = 0; i < Atoms.Count(); i++) {
        TCAtom& ca = Atoms[i]->CAtom();
        if (ca.GetAfix() == -1) {
          continue;
        }
        if (ca.GetAfix() == 1 || ca.GetAfix() == 2) {
          if (ca.GetDependentAfixGroup() != 0) {
            ca.GetDependentAfixGroup()->Clear();
          }
          continue;
        }
        if (ca.GetDependentAfixGroup() != 0) {
          ca.GetDependentAfixGroup()->Clear();
        }
        else if (ca.DependentHfixGroupCount() != 0) {
          for (size_t j = 0; j < ca.DependentHfixGroupCount(); j++) {
            if (ca.GetDependentHfixGroup(j).GetAfix() != -1) {
              ca.GetDependentHfixGroup(j).Clear();
            }
          }
        }
        else if (ca.GetParentAfixGroup() != 0) {
          ca.GetParentAfixGroup()->Clear();
        }
      }
    }
    else if (!Atoms.IsEmpty()) {
      // simply override the AFIX number
      if (Options.GetBoolOption('c')) {
        for (size_t i = 0; i < Atoms.Count(); i++) {
          if (Atoms[i]->CAtom().GetParentAfixGroup() != 0) {
            Atoms[i]->CAtom().GetParentAfixGroup()->SetAfix(afix);
          }
        }
        return;
      }
      
      if (Atoms[0]->CAtom().GetUisoOwner() != 0) {
        TBasicApp::NewLogEntry(logError) << "Cannot use '" <<
          Atoms[0]->GetLabel() << "' as a pivot for the AFIX group";
      }
      else {
        if (Atoms[0]->CAtom().GetDependentAfixGroup() != 0) {
          Atoms[0]->CAtom().GetDependentAfixGroup()->Clear();
        }
        TAfixGroup& ag = rm.AfixGroups.New(&Atoms[0]->CAtom(), afix);
        for (size_t i = 1; i < Atoms.Count(); i++) {
          ag.AddDependent(Atoms[i]->CAtom());
        }
      }
    }
  }
}
//.............................................................................
bool XLibMacros::ParseResParam(TStrObjList &Cmds, double& esd, double* len,
  double* len1, double* ang)
{
  bool esd_set = false;
  for (size_t i=0; i < Cmds.Count(); i++) {
    if (Cmds[i].IsNumber()) {
      double v = Cmds[i].ToDouble();
      if (olx_abs(v) > 0.25) {
        if (olx_abs(v) < 5) {
          if (len == NULL) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "too many numerical arguments, length");
          }
          else {
            if (*len == 0) {
              *len = v;
              if( len1 != NULL )  *len1 = v;
            }
            else if (len1 != NULL)
              *len1 = v;
            else {
              throw TInvalidArgumentException(__OlxSourceInfo,
                "too many numerical arguments, length");
            }
          }
        }
        else if (olx_abs(v) >= 15 && olx_abs(v) <= 180) {  // looks line an angle?
          if (ang != NULL)  *ang = v;
          else {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "too many numerical arguments, angle");
          }
        }
        else
          throw TInvalidArgumentException(__OlxSourceInfo, Cmds[i]);
      }
      else  {  // v < 0.25
        esd = v;
        esd_set = true;
      }
      Cmds.Delete(i);
      i--;
    }
  }
  return esd_set;
}
//.............................................................................
XLibMacros::MacroInput XLibMacros::ExtractSelection(const TStrObjList &Cmds_,
    bool unselect)
{
  TStrObjList Cmds = Cmds_;
  TSAtomPList atoms;
  TSBondPList bonds;
  TSPlanePList planes;
  TXApp &app = TXApp::GetInstance();
  if (!Cmds.IsEmpty()) {
    atoms = app.FindSAtoms(Cmds, false, unselect);
  }
  else {
    SObjectPtrList sel = app.GetSelected(unselect);
    for( size_t i=0; i < sel.Count(); i++ )  {
      TSBond* sb = dynamic_cast<TSBond*>(sel[i]);
      if (sb != NULL) {
        bonds << sb;
        continue;
      }
      TSAtom* sa = dynamic_cast<TSAtom*>(sel[i]);
      if (sa != NULL)
        atoms << sa;
      TSPlane* sp = dynamic_cast<TSPlane*>(sel[i]);
      if (sp != NULL)
        planes << sp;
    }
  }
  return XLibMacros::MacroInput(atoms, bonds, planes);
}
//.............................................................................
void XLibMacros::macDfix(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double fixLen = 0, esd = 0.02;  // length and esd for dfix
  bool esd_set = ParseResParam(Cmds, esd, &fixLen);
  if (fixLen == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "please specify the distance to restrain to");
    return;
  }
  TXApp &app = TXApp::GetInstance();
  MacroInput mi = ExtractSelection(Cmds, !Options.Contains("cs"));
  TSAtomPList Atoms;
  if (!mi.atoms.IsEmpty())
    Atoms = mi.atoms;
  else {
    for (size_t i=0; i < mi.bonds.Count(); i++)
      Atoms << mi.bonds[i]->A() << mi.bonds[i]->B();
  }
  if (Atoms.IsEmpty()) return;
  TSimpleRestraint &dfix = app.XFile().GetRM().rDFIX.AddNew();
  dfix.SetValue(fixLen);
  if (esd_set) dfix.SetEsd(esd);
  esd = dfix.GetEsd();
  if (Atoms.Count() == 1) {  // special case
    TSAtom* A = Atoms[0];
    for (size_t i=0; i < A->NodeCount(); i++) {
      TSAtom* SA = &A->Node(i);
      if (SA->IsDeleted() || SA->GetType() == iQPeakZ)
        continue;
      dfix.AddAtomPair(
        A->CAtom(), &A->GetMatrix(),
        SA->CAtom(), &SA->GetMatrix());
    }
  }
  else if (Atoms.Count() == 3) {  // special case
    dfix.AddAtomPair(
      Atoms[0]->CAtom(), &Atoms[0]->GetMatrix(),
      Atoms[1]->CAtom(), &Atoms[1]->GetMatrix());
    dfix.AddAtomPair(
      Atoms[1]->CAtom(), &Atoms[1]->GetMatrix(),
      Atoms[2]->CAtom(), &Atoms[2]->GetMatrix());
  }
  else {
    if ((Atoms.Count()%2) != 0) {
      E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected");
      return;
    }
    for (size_t i=0; i < Atoms.Count(); i+=2) {
      dfix.AddAtomPair(
        Atoms[i]->CAtom(), &Atoms[i]->GetMatrix(),
        Atoms[i+1]->CAtom(), &Atoms[i+1]->GetMatrix());
    }
  }
  dfix.UpdateResi();
  if (Options.GetBoolOption('i')) {
    dfix.ConvertToImplicit();
  }
  app.XFile().GetRM().rDFIX.ValidateRestraint(dfix);
  TBasicApp::NewLogEntry() << dfix.ToString();
}
//.............................................................................
void XLibMacros::macDang(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double fixLen = 0, esd = 0.04;  // length and esd for dang
  bool esd_set = ParseResParam(Cmds, esd, &fixLen);
  if (fixLen == 0) {
    E.ProcessingError(__OlxSrcInfo,
      "please specify the distance to restrain to");
    return;
  }
  TXApp &app = TXApp::GetInstance();
  TSAtomPList Atoms = app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  if (Atoms.IsEmpty()) {
    E.ProcessingError(__OlxSrcInfo, "no atoms or bonds provided");
    return;
  }
  TSimpleRestraint &dang = app.XFile().GetRM().rDANG.AddNew();
  dang.SetValue(fixLen);
  if (esd_set) dang.SetEsd(esd);
  esd = dang.GetEsd();
  if ((Atoms.Count()%2) != 0 ) {
    E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected");
    return;
  }
  dang.SetAtoms(Atoms);
  if (Options.GetBoolOption('i')) {
    dang.ConvertToImplicit();
  }
  app.XFile().GetRM().rDANG.ValidateRestraint(dang);
  TBasicApp::NewLogEntry() << dang.ToString();
}
//.............................................................................
void XLibMacros::macTria(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double dfixLenA = 0, dfixLenB = 0,
        esd = 0.02,  // esd for dfix, for dang will be doubled
        angle = 0;
  bool esd_set = ParseResParam(Cmds, esd, &dfixLenA, &dfixLenB, &angle);
  if (angle == 0) {
    E.ProcessingError(__OlxSrcInfo, "please provide the angle to restrain to");
    return;
  }
  MacroInput mi = ExtractSelection(Cmds, !Options.Contains("cs"));
  TXApp &app = TXApp::GetInstance();
  TSAtomPList atoms;
  if (!mi.atoms.IsEmpty())
    atoms = mi.atoms;
  else {
    if (mi.bonds.Count() > 1) {
      for (size_t i=0; i < mi.bonds.Count(); i += 2) {
        TSBond *ba = mi.bonds[i];
        TSBond *bb = mi.bonds[i+1];
        TSAtom* a = (&ba->A() == &bb->A() || &ba->B() == &bb->A() ) ? &bb->A()
          : ((&ba->A() == &bb->B() || &ba->B() == &bb->B()) ? &bb->B() : NULL);
        if( a == NULL )  {
          E.ProcessingError(__OlxSrcInfo, "some bonds do not share atom");
          return;
        }
        atoms.Add(ba->Another(*a));
        atoms.Add(a);
        atoms.Add(bb->Another(*a));
      }
    }
  }
  if ((atoms.Count() % 3) != 0) {
    E.ProcessingError(__OlxSrcInfo, "Please provide atom triplets");
    return;
  }
  TPtrList<TSimpleRestraint> restraints;
  for (size_t i=0; i < atoms.Count(); i+=3) {
    if (dfixLenA == 0) {
      dfixLenA = app.XFile().GetRM().FindRestrainedDistance(
        atoms[i]->CAtom(), atoms[i+1]->CAtom());
      dfixLenB = app.XFile().GetRM().FindRestrainedDistance(
        atoms[i+1]->CAtom(), atoms[i+2]->CAtom());
      if( dfixLenA == -1 || dfixLenB == -1 )  {
        TBasicApp::NewLogEntry(logError) <<
          "Tria: please fix or provided distance(s)";
        dfixLenA = dfixLenB = 0;
        continue;
      }
    }
    TSimpleRestraint *dfix = &app.XFile().GetRM().rDFIX.AddNew();
    dfix->SetValue(dfixLenA);
    if (esd_set) {
      dfix->SetEsd(esd);
    }
    esd = dfix->GetEsd();
    dfix->AddAtomPair(atoms[i]->CAtom(), &atoms[i]->GetMatrix(),
      atoms[i+1]->CAtom(), &atoms[i+1]->GetMatrix());
    if (dfixLenB != dfixLenA) {
      app.XFile().GetRM().rDFIX.ValidateRestraint(*dfix);
      if (!dfix->IsEmpty()) {
        restraints << dfix;
      }
      dfix = &app.XFile().GetRM().rDFIX.AddNew();
      dfix->SetEsd(esd);
    }
    dfix->AddAtomPair(atoms[i+1]->CAtom(), &atoms[i+1]->GetMatrix(),
      atoms[i+2]->CAtom(), &atoms[i+2]->GetMatrix());
    app.XFile().GetRM().rDFIX.ValidateRestraint(*dfix);
    if (!dfix->IsEmpty()) {
      restraints << dfix;
    }
    TSimpleRestraint &dang = app.XFile().GetRM().rDANG.AddNew();
    dang.SetValue(
      olx_round(
        sqrt(olx_sqr(dfixLenA) + olx_sqr(dfixLenB) -
          2*dfixLenA*dfixLenB*cos(angle*M_PI/180)),
        1000)
      );
    dang.SetEsd(esd*2);
    dang.AddAtom(atoms[i]->CAtom(), &atoms[i]->GetMatrix());
    dang.AddAtom(atoms[i+2]->CAtom(), &atoms[i+2]->GetMatrix());
    app.XFile().GetRM().rDANG.ValidateRestraint(dang);
    if (!dang.IsEmpty()) {
      restraints << dang;
    }
  }
  const bool impl = Options.GetBoolOption('i');
  TBasicApp::NewLogEntry() << "Placing the following restraints: ";
  for (size_t i = 0; i < restraints.Count(); i++) {
    if (impl) {
      restraints[i]->ConvertToImplicit();
    }
    TBasicApp::NewLogEntry() << restraints[i]->ToString();
  }
}
//.............................................................................
void XLibMacros::macSadi(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double esd = 0.02;  // esd for sadi
  bool esd_set = ParseResParam(Cmds, esd);
  MacroInput mi = ExtractSelection(Cmds, !Options.Contains("cs"));
  TXApp &app = TXApp::GetInstance();
  TSAtomPList Atoms;
  if (!mi.atoms.IsEmpty())
    Atoms = mi.atoms;
  else {
    for (size_t i=0; i < mi.bonds.Count(); i++)
      Atoms << mi.bonds[i]->A() << mi.bonds[i]->B();
  }
  TPtrList<TSimpleRestraint> restraints;
  TSimpleRestraint &sr = app.XFile().GetRM().rSADI.AddNew();
  restraints << sr;
  if (esd_set) sr.SetEsd(esd);
  esd = sr.GetEsd();
  if (Atoms.Count() == 1) {  // special case
    TSimpleRestraint &sr1 = app.XFile().GetRM().rSADI.AddNew();
    restraints << sr1;
    sr1.SetEsd(esd);
    sr.SetEsd(esd*2);
    TSAtom* A = Atoms[0];
    double td = 0;
    for (size_t i=0; i < A->NodeCount(); i++) {
      TSAtom& SA = A->Node(i);
      if (SA.IsDeleted() || SA.GetType() == iQPeakZ) continue;
      sr1.AddAtomPair(
        A->CAtom(), &A->GetMatrix(),
        SA.CAtom(), &SA.GetMatrix());
      if (td == 0)  // need this one to remove opposite atoms from restraint
        td = A->crd().DistanceTo(SA.crd()) * 2;
      for (size_t j=i+1; j < A->NodeCount(); j++) {
        TSAtom& SA1 = A->Node(j);
        if (SA1.IsDeleted() || SA1.GetType() == iQPeakZ ||
           (SA.CAtom().GetPart() != SA1.CAtom().GetPart()))
        {
         continue;
        }
        const double d = SA.crd().DistanceTo(SA1.crd());
        if( d/td > 0.85 ) {
          TBasicApp::NewLogEntry() << "Skipping " << SA.GetGuiLabel() << '-' <<
            SA1.GetGuiLabel() << ": high deviation from the rest of the bonds";
          continue;
        }
        sr.AddAtomPair(
          SA.CAtom(), &SA.GetMatrix(),
          SA1.CAtom(), &SA1.GetMatrix());
      }
    }
  }
  else if (Atoms.Count() == 2) {  // 'rotor;
    if (Options.GetBoolOption('r')) {
      for (size_t i = 0; i < Atoms[0]->NodeCount(); i++) {
        TSAtom& n = Atoms[0]->Node(i);
        if (n.IsDeleted() || n.GetType() == iQPeakZ || &n == Atoms[1])
          continue;
        sr.AddAtomPair(
          Atoms[0]->CAtom(), &Atoms[0]->GetMatrix(),
          n.CAtom(), &n.GetMatrix());
      }
      TSimpleRestraint &sr1 = app.XFile().GetRM().rSADI.AddNew();
      if (esd_set) sr1.SetEsd(esd);
      restraints << sr1;
      olx_pdict<int, TSAtomPList> parts;
      for (size_t i = 0; i < Atoms[0]->NodeCount(); i++) {
        TSAtom& n = Atoms[0]->Node(i);
        if (n.IsDeleted() || n.GetType() == iQPeakZ || &n == Atoms[1])
          continue;
        parts.Add(n.CAtom().GetPart()).Add(n);
        sr1.AddAtomPair(
          Atoms[1]->CAtom(), &Atoms[1]->GetMatrix(),
          n.CAtom(), &n.GetMatrix());
      }
      TSimpleRestraint &sr2 = app.XFile().GetRM().rSADI.AddNew();
      if (esd_set) sr2.SetEsd(esd);
      restraints << sr2;
      for (size_t i = 0; i < parts.Count(); i++) {
        TSAtomPList &atoms = parts.GetValue(i);
        vec3d normal, center;
        TSPlane::CalcPlane(atoms, normal, center);
        olx_plane::Sort(atoms, FunctionAccessor::MakeConst(
          (const vec3d& (TSAtom::*)() const)&TSAtom::crd),
          center, normal);
        for (size_t j = 1; j < atoms.Count(); j++) {
          sr2.AddAtomPair(
            atoms[j-1]->CAtom(), &atoms[j-1]->GetMatrix(),
            atoms[j]->CAtom(), &atoms[j]->GetMatrix());
        }
        if (atoms.Count() > 2) {
          sr2.AddAtomPair(
            atoms[0]->CAtom(), &atoms[0]->GetMatrix(),
            atoms.GetLast()->CAtom(), &atoms.GetLast()->GetMatrix());
        }
      }
    }
    else {
      if (!Options.GetBoolOption('i')) {
        E.ProcessingError(__OlxSrcInfo, "not applicable to explicit restraints");
        return;
      }
      TSimpleRestraint &sr1 = app.XFile().GetRM().rSADI.AddNew();
      if (esd_set) sr1.SetEsd(esd);
      restraints << sr1;
      sr1.AddAtomPair(Atoms[0]->CAtom(), 0, Atoms[1]->CAtom(), 0);
    }
  }
  else if (Atoms.Count() == 3) {  // special case
    sr.AddAtomPair(
      Atoms[0]->CAtom(), &Atoms[0]->GetMatrix(),
      Atoms[1]->CAtom(), &Atoms[1]->GetMatrix());
    sr.AddAtomPair(
      Atoms[1]->CAtom(), &Atoms[1]->GetMatrix(),
      Atoms[2]->CAtom(), &Atoms[2]->GetMatrix());
  }
  else {
    if ((Atoms.Count()%2) != 0 ) {
      E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected");
      return;
    }
    for (size_t i=0; i < Atoms.Count(); i += 2 ) {
      sr.AddAtomPair(Atoms[i]->CAtom(), &Atoms[i]->GetMatrix(),
        Atoms[i+1]->CAtom(), &Atoms[i+1]->GetMatrix());
    }
  }
  bool to_impl = Options.GetBoolOption('i');
  TBasicApp::NewLogEntry() << "Placing the following restraints: ";
  for (size_t i = 0; i < restraints.Count(); i++) {
    if (to_impl)
      restraints[i]->ConvertToImplicit();
    TBasicApp::NewLogEntry() << restraints[i]->ToString();
  }
}
//.............................................................................
void XLibMacros::macFlat(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double esd = 0.1;  // esd for flat
  bool esd_set = ParseResParam(Cmds, esd);
  TXApp &app = TXApp::GetInstance();
  TSAtomPList Atoms = app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  if (Atoms.IsEmpty()) return;
  TSimpleRestraint& sr = app.XFile().GetRM().rFLAT.AddNew();
  if (esd_set) sr.SetEsd(esd);
  for (size_t i=0; i < Atoms.Count(); i++)
    sr.AddAtom(Atoms[i]->CAtom(), &Atoms[i]->GetMatrix());
  app.XFile().GetRM().rFLAT.ValidateRestraint(sr);
  if (!sr.IsEmpty()) {
    if (Options.GetBoolOption('i')) {
      sr.ConvertToImplicit();
    }
    TBasicApp::NewLogEntry() << "Placing the following restraints: ";
    TBasicApp::NewLogEntry() << sr.ToString();
  }
}
//.............................................................................
void XLibMacros::macSIMU(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double esd1 = 0.04, esd2 = 0.08, val = 1.7;  // esd
  size_t cnt = XLibMacros::Parse(Cmds, "ddd", &esd1, &esd2, &val);
  if (cnt == 1) esd2 = esd1 * 2;
  TXApp &app = TXApp::GetInstance();
  TSAtomPList Atoms = app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  // validate that atoms of the same type
  TSimpleRestraint& sr = app.XFile().GetRM().rSIMU.AddNew();
  sr.SetAllNonHAtoms(Atoms.IsEmpty());
  if (cnt > 0) {
    sr.SetEsd(esd1);
    sr.SetEsd1(esd2);
    if (cnt == 3)
      sr.SetValue(val);
  }
  for (size_t i = 0; i < Atoms.Count(); i++) {
    sr.AddAtom(Atoms[i]->CAtom(), NULL);
  }
  app.XFile().GetRM().rSIMU.ValidateRestraint(sr);
  if (!sr.IsEmpty()) {
    if (Options.GetBoolOption('i')) {
      sr.ConvertToImplicit();
    }
    TBasicApp::NewLogEntry() << "Placing the following restraints: ";
    TBasicApp::NewLogEntry() << sr.ToString();
  }
}
//.............................................................................
void XLibMacros::macDELU(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double esd1 = 0.01, esd2=0.01;  // esd
  size_t cnt = XLibMacros::Parse(Cmds, "dd", &esd1, &esd2);
  if (cnt == 1) esd2 = esd1;
  TXApp &app = TXApp::GetInstance();
  TSAtomPList Atoms = app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  // validate that atoms of the same type
  TSimpleRestraint& sr = app.XFile().GetRM().rDELU.AddNew();
  if (cnt > 0) {
    sr.SetEsd(esd1);
    sr.SetEsd1(esd2);
  }
  sr.SetAllNonHAtoms(Atoms.IsEmpty());
  for (size_t i = 0; i < Atoms.Count(); i++) {
    sr.AddAtom(Atoms[i]->CAtom(), NULL);
  }
  app.XFile().GetRM().rDELU.ValidateRestraint(sr);
  if (!sr.IsEmpty()) {
    if (Options.GetBoolOption('i')) {
      sr.ConvertToImplicit();
    }
    TBasicApp::NewLogEntry() << "Placing the following restraints: ";
    TBasicApp::NewLogEntry() << sr.ToString();
  }
}
//.............................................................................
void XLibMacros::macISOR(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double esd1 = 0.1, esd2=0.2;  // esd
  size_t cnt = XLibMacros::Parse(Cmds, "dd", &esd1, &esd2);
  if (cnt == 1) esd2 = 2*esd1;
  TXApp &app = TXApp::GetInstance();
  TSAtomPList Atoms = app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  if (Atoms.IsEmpty()) return;
  // validate that atoms of the same type
  TSimpleRestraint& sr = app.XFile().GetRM().rISOR.AddNew();
  if (cnt > 0) {
    sr.SetEsd(esd1);
    sr.SetEsd1(esd2);
  }
  sr.SetAllNonHAtoms(Atoms.IsEmpty());
  for (size_t i = 0; i < Atoms.Count(); i++) {
    sr.AddAtom(Atoms[i]->CAtom(), NULL);
  }
  app.XFile().GetRM().rISOR.ValidateRestraint(sr);
  if (!sr.IsEmpty()) {
    if (Options.GetBoolOption('i')) {
      sr.ConvertToImplicit();
    }
    TBasicApp::NewLogEntry() << "Placing the following restraints: ";
    TBasicApp::NewLogEntry() << sr.ToString();
  }
}
//.............................................................................
void XLibMacros::macChiv(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double esd = 0.1, val=0;
  size_t cnt = XLibMacros::Parse(Cmds, "dd", &val, &esd);
  TXApp &app = TXApp::GetInstance();
  TSAtomPList Atoms = app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  if (Atoms.IsEmpty()) return;
  TSimpleRestraint& sr = app.XFile().GetRM().rCHIV.AddNew();
  sr.SetValue(val);
  if (cnt == 2) sr.SetEsd(esd);
  for (size_t i = 0; i < Atoms.Count(); i++) {
    sr.AddAtom(Atoms[i]->CAtom(), &Atoms[i]->GetMatrix());
  }
  app.XFile().GetRM().rCHIV.ValidateRestraint(sr);
  if (!sr.IsEmpty()) {
    if (Options.GetBoolOption('i')) {
      sr.ConvertToImplicit();
    }
    TBasicApp::NewLogEntry() << "Placing the following restraints: ";
    TBasicApp::NewLogEntry() << sr.ToString();
  }
}
//.............................................................................
void XLibMacros::macRRings(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double l = 1.39, e = 0.01;
  XLibMacros::Parse(Cmds, "dd", &l, &e);
  TTypeList<TSAtomPList> rings;
  TXApp &app = TXApp::GetInstance();
  olxstr cmd = Cmds.IsEmpty() ? olxstr("sel") : Cmds[0];
  try  {  app.FindRings(cmd, rings);  }
  catch(const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  for (size_t i=0; i < rings.Count(); i++) {
    TBasicApp::NewLogEntry() << "Processing ring: " <<
      olx_analysis::alg::label(rings[i], true, '-');
    if (!olx_analysis::alg::has_I(rings[i])) {
      TBasicApp::NewLogEntry() << "Skipping - no atoms of the asymmetric unit";
      continue;
    }
    TSimpleRestraint& dr12 = l < 0 ? app.XFile().GetRM().rSADI.AddNew()
      : app.XFile().GetRM().rDFIX.AddNew();
    TSimpleRestraint& dr13 = app.XFile().GetRM().rSADI.AddNew();
    if (l > 0) dr12.SetValue(l);
    dr12.SetEsd(e);
    dr13.SetEsd(e*2);
    TSimpleRestraint& flat = app.XFile().GetRM().rFLAT.AddNew();
    flat.SetEsd(0.1);
    for (size_t j=0; j < rings[i].Count(); j++) {
      flat.AddAtom(rings[i][j]->CAtom(), &rings[i][j]->GetMatrix());
      if ((j+1) < rings[i].Count()) {
        dr12.AddAtomPair(rings[i][j]->CAtom(), &rings[i][j]->GetMatrix(),
          rings[i][j+1]->CAtom(), &rings[i][j+1]->GetMatrix());
      }
      else {
        dr12.AddAtomPair(rings[i][j]->CAtom(), &rings[i][j]->GetMatrix(),
          rings[i][0]->CAtom(), &rings[i][0]->GetMatrix());
      }
      if ((j+2) < rings[i].Count()) {
        dr13.AddAtomPair(rings[i][j]->CAtom(), &rings[i][j]->GetMatrix(),
          rings[i][j+2]->CAtom(), &rings[i][j+2]->GetMatrix());
      }
      else {
        size_t idx = (j+2) - rings[i].Count();
        dr13.AddAtomPair(rings[i][j]->CAtom(), &rings[i][j]->GetMatrix(),
          rings[i][idx]->CAtom(), &rings[i][idx]->GetMatrix());
      }
    }
    TBasicApp::NewLogEntry() << "Placing the following restraints: ";
    TBasicApp::NewLogEntry() << dr12.ToString();
    TBasicApp::NewLogEntry() << dr13.ToString();
    TBasicApp::NewLogEntry() << flat.ToString() << NewLineSequence();
  }
}
//.............................................................................
void XLibMacros::macDelta(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  if (Cmds.Count() == 1) {
    double delta = Cmds[0].ToDouble();
    if (delta < 0.1 || delta > 0.9)
      delta = 0.5;
    app.XFile().GetLattice().SetDelta(delta);
  }
  else {
    TBasicApp::NewLogEntry() << "Current delta (covalent bonds) is: " <<
      app.XFile().GetLattice().GetDelta();
  }
}
//.............................................................................
void XLibMacros::macDeltaI(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  if (Cmds.Count() == 1) {
    double deltai = Cmds[0].ToDouble();
    if (deltai < 0.9) deltai = 0.9;
    else if (deltai > 3)
      deltai = 3;
    app.XFile().GetLattice().SetDeltaI(deltai);
  }
  else  {
    TBasicApp::NewLogEntry() << "Current delta (short interactions bonds) is: "
      << app.XFile().GetLattice().GetDeltaI();
  }
}
//.............................................................................
void XLibMacros::macExport(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  const bool use_md5 = app.GetOptions().FindValue("cif.use_md5", FalseString()).ToBool();
  TCif& C = app.XFile().GetLastLoader<TCif>();
  olxstr hkl_name = (!Cmds.IsEmpty() ? Cmds[0]: C.GetDataName() + ".hkl");
  if (!Cmds.IsEmpty()) {
    if (TEFile::ExtractFileExt(hkl_name).IsEmpty()) {
      hkl_name << ".hkl";
    }
  }
  if (TEFile::Exists(hkl_name)) {
    E.ProcessingError(__OlxSrcInfo, "the hkl file already exists");
    return;
  }
  cif_dp::cetTable* hklLoop = C.FindLoop("_refln");
  if (hklLoop == 0) {
    hklLoop = C.FindLoop("_diffrn_refln");
  }
  if (hklLoop == 0) {
    cif_dp::cetStringList *ci = dynamic_cast<cif_dp::cetStringList *>(
      C.FindEntry("_shelx_hkl_file"));
    if (ci == 0) {
      ci = dynamic_cast<cif_dp::cetStringList *>(
        C.FindEntry("_iucr_refine_reflections_details"));
    }
    if (ci == 0) {
      TBasicApp::NewLogEntry() << "No hkl loop or data found";
    }
    else {
      TCStrList lines(ci->lines);
      TEFile::WriteLines(hkl_name, lines);
      ci = dynamic_cast<cif_dp::cetStringList *>(
        C.FindEntry("_shelx_fab_file"));
      if (ci != 0) {
        TEFile::WriteLines(TEFile::ChangeFileExt(hkl_name, "fab"),
          TCStrList(ci->lines));
      }
    }
  }
  else {
    olx_object_ptr<THklFile::ref_list> refs = THklFile::FromCifTable(*hklLoop, mat3d().I());
    if (refs.ok()) {
      olxstr md5;
      cif_dp::ICifEntry * md5i = C.FindEntry("_olex2_hkl_file_MD5");
      if (md5i != 0) {
        md5 = md5i->GetStringValue();
        olxstr_buf bf;
        for (size_t i = 0; i < hklLoop->RowCount(); i++) {
          for (size_t j = 0; j < hklLoop->ColCount(); j++) {
            bf << (*hklLoop)[i][j]->GetStringValue();
          }
        }
        if (MD5::Digest(olxcstr(olxstr(bf).DeleteCharSet("\n\r\t "))) != md5) {
          TBasicApp::NewLogEntry(logWarning) << "HKL MD5: mismatch";
        }
        else {
          TBasicApp::NewLogEntry() << "HKL MD5: OK";
        }
      }
      else if (use_md5) {
        TBasicApp::NewLogEntry(logWarning) << "HKL MD5: missing";
      }
      THklFile::SaveToFile(hkl_name, refs->a);
      if (!refs->b) {
        C.GetRM().SetHKLF(3);
        app.XFile().GetRM().SetHKLF(3);
      }
    }
  }
  // extract FAB if any
  {
    cif_dp::cetStringList *ci = dynamic_cast<cif_dp::cetStringList *>(
      C.FindEntry("_shelx_fab_file"));
    if (ci != 0) {
      TEFile::WriteLines(TEFile::ChangeFileExt(hkl_name, "fab"),
        TCStrList(ci->lines));
    }
  }
  // extract SQUEEZE info, if any
  {
    cif_dp::cetTable *sqf = C.FindLoop("_smtbx_masks_void");
    TPtrList<cif_dp::ICifEntry> details;
    if (sqf != 0) {
      cif_dp::ICifEntry *e = C.FindEntry("_smtbx_masks_special_details");
      if (e != 0) {
        details.Add(e);
      }
    }
    else {
      sqf = C.FindLoop("_platon_squeeze_void");
      if (sqf != 0) {
        cif_dp::ICifEntry *e = C.FindEntry("_platon_squeeze_details");
        if (e != 0) {
          details.Add(e);
        }
        e = C.FindEntry("_platon_squeeze_void_probe_radius");
        if (e != 0) {
          details.Add(e);
        }
      }
    }
    if (sqf != 0) {
      TCif cf;
      cf.SetCurrentBlock(C.GetDataName(), true);
      cf.SetParam(*sqf);
      for (size_t i = 0; i < details.Count(); i++) {
        cf.SetParam(*details[i]);
      }
      cf.SaveToFile(TEFile::ChangeFileExt(hkl_name, "sqf"));
    }
  }
  // check if the res file is there
  olxstr res_name = TEFile::ChangeFileExt(hkl_name, "res");
  if (!TEFile::Exists(res_name)) {
    cif_dp::cetStringList *ci = dynamic_cast<cif_dp::cetStringList *>(
      C.FindEntry("_shelx_res_file"));
    bool check_md5 = false;
    if (ci == 0) {
      check_md5 = true;
      ci = dynamic_cast<cif_dp::cetStringList *>(
        C.FindEntry("_iucr_refine_instructions_details"));
    }
    if (ci != 0) {
      TCStrList lines(ci->lines);
      TEFile::WriteLines(res_name, lines);
      if (check_md5) {
        olxstr md5;
        cif_dp::ICifEntry * md5i = C.FindEntry("_olex2_res_file_MD5");
        if (md5i != 0) {
          md5 = md5i->GetStringValue();
          if (MD5::Digest(lines.Text(CEmptyString()).DeleteCharSet("\n\r\t ")) != md5) {
            TBasicApp::NewLogEntry(logWarning) << "RES MD5: mismatch";
          }
          else {
            TBasicApp::NewLogEntry() << "RES MD5: OK";
          }
        }
        else if (use_md5) {
          TBasicApp::NewLogEntry(logWarning) << "RES MD5: missing";
        }
      }
      olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
      if (op != 0) {
        op->processMacro("CifExtract");
      }
    }
  }
}
//..............................................................................
void XLibMacros::macLstSymm(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp &app = TXApp::GetInstance();
  TUnitCell& uc = app.XFile().GetLattice().GetUnitCell();
  TTTable<TStrList> tab(uc.MatrixCount(), 2);
  tab.ColName(0) = "Code";
  tab.ColName(1) = "Symm";
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    tab[i][0] =
      TSymmParser::MatrixToSymmCode(uc.GetSymmSpace(), uc.GetMatrix(i));
    tab[i][1] = TSymmParser::MatrixToSymm(uc.GetMatrix(i));
  }
  TBasicApp::NewLogEntry() <<
    tab.CreateTXTList("Unit cell symmetry list", true, true, ' ');
}
//..............................................................................
void XLibMacros::macSgen(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp &app = TXApp::GetInstance();
  bool move = Options.GetBoolOption('m');
  if (move && app.XFile().GetLattice().IsGenerated()) {
    Error.ProcessingError(__OlxSrcInfo,
      "moving is not applicable to grown structures");
    return;
  }
  // check if a single full matrix is given
  if (Cmds.Count() == 12 && olx_list_and(Cmds, &olxstr::IsNumber)) {
    smatdd m;
    for (int i = 0; i < 9; i++) {
      m.r[i / 3][i % 3] = Cmds[i].ToDouble();
    }
    for (int i = 0; i < 3; i++) {
      m.t[i] = Cmds[9 + i].ToDouble();
    }
    TLattice & latt = app.XFile().GetLattice();
    TSAtomPList atoms = app.FindSAtoms(TStrList());
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (move) {
        vec3d c = m * atoms[i]->ccrd();
        atoms[i]->CAtom().ccrd() = c;
        if (atoms[i]->CAtom().GetEllipsoid() != 0) {
          atoms[i]->CAtom().GetEllipsoid()->Mult(m.r);
        }
      }
      else {
        vec3d c = m * atoms[i]->ccrd();
        TSAtom &a = latt.NewAtom(c);
        a.CAtom().SetType(atoms[i]->CAtom().GetType());
      }
    }
    latt.Init();
    return;
  }
  smatd_list symm;
  smatd matr;
  for (size_t i = 0; i < Cmds.Count(); i++) {
    bool validSymm = false;
    if (TSymmParser::IsRelSymm(Cmds[i])) {
      try {
        matr = TSymmParser::SymmCodeToMatrix(
          app.XFile().GetLattice().GetUnitCell(), Cmds[i]);
        validSymm = true;
      }
      catch (const TExceptionBase &) {}
    }
    if (!validSymm) {
      try {
        matr = TSymmParser::SymmToMatrix(Cmds[i]);
        validSymm = true;
      }
      catch (const TExceptionBase &) {}
    }
    if (validSymm) {
      Cmds.Delete(i--);
      symm.AddCopy(app.XFile().GetUnitCell().InitMatrixId(matr));
    }
  }
  if (symm.IsEmpty()) {
    Error.ProcessingError(__OlxSrcInfo, "no symm code(s) provided");
    return;
  }
  if (move) {
    if (app.XFile().GetLattice().IsGenerated() && symm.Count() > 1) {
      Error.ProcessingError(__OlxSrcInfo, "single operator is expected for move");
      return;
    }
    TSAtomPList atoms = app.FindSAtoms(Cmds, true, true);
    for (size_t i = 0; i < atoms.Count(); i++) {
      vec3d c = symm[0] * atoms[i]->ccrd();
      atoms[i]->CAtom().ccrd() = c;
      if (atoms[i]->CAtom().GetEllipsoid() != 0) {
        atoms[i]->CAtom().GetEllipsoid()->Mult(symm[0].r);
      }
    }
    app.XFile().GetLattice().Init();
    return;
  }
  TCAtomPList atoms(app.FindSAtoms(Cmds, true, true),
    FunctionAccessor::MakeConst(&TSAtom::CAtom));
  app.XFile().GetLattice().GrowAtoms(atoms, symm);
}
//..............................................................................
void XLibMacros::macConn(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  try  {
    TStrList lst(Cmds);
    if ((Cmds.Count() == 1 || Cmds.Count() == 2) &&
      olx_list_and(Cmds, &olxstr::IsNumber))
    {
      MacroInput mi = ExtractSelection(TStrObjList(), true);
      for (size_t i=0; i < mi.atoms.Count(); i++)
        mi.atoms[i]->SetTag(mi.atoms[i]->CAtom().GetId());
      for (size_t i=0; i < mi.atoms.Count(); i++) {
        if (mi.atoms[i]->GetTag() != mi.atoms[i]->CAtom().GetId())
          continue;
        lst.Add("#c") << mi.atoms[i]->CAtom().GetId();
      }
    }
    app.XFile().GetRM().Conn.ProcessConn(lst);
    app.XFile().GetLattice().UpdateConnectivityInfo();
  }
  catch( const TExceptionBase& exc )  {
    E.ProcessingError(__OlxSrcInfo, exc.GetException()->GetError());
  }
}
//..............................................................................
void XLibMacros::macAddBond(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  TSAtomPList atoms = app.FindSAtoms(Cmds, false, true);
  if ((atoms.Count() % 2) != 0) {
    E.ProcessingError(__OlxSrcInfo, "even number if atoms is expected");
    return;
  }
  for (size_t i=0; i < atoms.Count(); i += 2) {
    app.XFile().GetRM().Conn.AddBond(
      atoms[i]->CAtom(), atoms[i+1]->CAtom(),
      atoms[i]->GetMatrix(),
      atoms[i+1]->GetMatrix()
    );
    TBasicApp::NewLogEntry() << "Adding bond " << atoms[i]->GetGuiLabel()
      << '-' << atoms[i+1]->GetGuiLabel();
  }
  app.XFile().GetLattice().UpdateConnectivityInfo();
}
//..............................................................................
void XLibMacros::macDelBond(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  MacroInput mi = ExtractSelection(Cmds, true);
  TSAtomPList pairs;
  for (size_t i=0; i < mi.bonds.Count(); i++)
    pairs << mi.bonds[i]->A() << mi.bonds[i]->B();
  if ((mi.atoms.Count()%2) == 0)
    pairs.AddAll(mi.atoms);

  TXApp &app = TXApp::GetInstance();
  const TUnitCell &uc = app.XFile().GetUnitCell();
  const smatd &fm = uc.GetMatrix(0);
  for( size_t i=0; i < pairs.Count(); i+=2 )  {
    TCAtom &a1 = pairs[i]->CAtom(),
      &a2 = pairs[i+1]->CAtom();
    for( size_t fasi=0; fasi <= a1.EquivCount(); fasi++ )  {
      const smatd m1 = uc.MulMatrix(pairs[i]->GetMatrix(),
        (fasi == a1.EquivCount() ? fm : a1.GetEquiv(fasi)));
      for( size_t sasi=0; sasi <= a2.EquivCount(); sasi++ )  {
        const smatd m2 = uc.MulMatrix(pairs[i+1]->GetMatrix(),
          (sasi == a2.EquivCount() ? fm : a2.GetEquiv(sasi)));
        app.XFile().GetRM().Conn.RemBond(a1, a2, m1, m2);
        TBasicApp::NewLogEntry() << "Removing bond " << pairs[i]->GetGuiLabel()
          << '-' << pairs[i+1]->GetGuiLabel();
      }
    }
  }
  app.XFile().GetLattice().UpdateConnectivityInfo();
}
//..............................................................................
void XLibMacros::macRestrain(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  RefinementModel &rm = app.XFile().GetRM();
  if (Cmds[0].Equalsi("ADP") && Cmds.Count() > 1) {
    olxstr target = Cmds[1];
    Cmds.DeleteRange(0, 2);
    double value = -1;
    if (Cmds.Count() > 0 && Cmds[0].IsNumber()) {
      value = Cmds[0].ToDouble();
      Cmds.Delete(0);
    }
    MacroInput mi = ExtractSelection(Cmds, true);
    TSimpleRestraint *r = 0;
    if (target.Equalsi("Ueq")) {
      if (mi.atoms.Count() < 2 && value < 0) {
        E.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
        return;
      }
      if (value > 0) {
        r = &rm.rFixedUeq.AddNew();
        r->SetValue(value);
      }
      else {
        r = &rm.rSimilarUeq.AddNew();
      }
    }
    else if (target.Equalsi("volume")) {
      if (mi.atoms.Count() < 2) {
        E.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
        return;
      }
      r = &rm.rSimilarAdpVolume.AddNew();
    }
    if (r != 0) {
      for (size_t i = 0; i < mi.atoms.Count(); i++) {
        r->AddAtom(mi.atoms[i]->CAtom(), 0);
      }
    }
  }
  else if (Cmds[0].Equalsi("bond")) {
    Cmds.Delete(0);
    MacroInput mi = ExtractSelection(Cmds, true);
    double val = -1, esd = 0.02;
    TSimpleRestraint *r = 0;
    size_t set_cnt = XLibMacros::Parse(Cmds, "dd", &val, &esd);
    TSAtomPList atoms = mi.atoms;
    if (atoms.IsEmpty()) {
      for (size_t i = 0; i < mi.bonds.Count(); i++) {
        atoms << mi.bonds[i]->A() << mi.bonds[i]->B();
      }
    }
    if ((atoms.Count() % 2) != 0) {
      E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected");
      return;
    }
    if ((set_cnt == 1 && val >= 0.5) || set_cnt == 2) {
      r = &rm.rDFIX.AddNew();
      r->SetValue(val);
      if (set_cnt == 2) {
        r->SetEsd(esd);
      }
    }
    else {
      r = &rm.rSADI.AddNew();
      if (set_cnt == 1) {
        r->SetEsd(val);
      }
    }
    for (size_t i = 0; i < atoms.Count(); i++) {
      r->AddAtom(atoms[i]->CAtom(), &atoms[i]->GetMatrix());
    }
  }
  else if (Cmds[0].Equalsi("angle") && Cmds.Count() > 1) {
    double val = Cmds[1].ToDouble();
    Cmds.DeleteRange(0, 2);
    MacroInput mi = ExtractSelection(Cmds, true);
    TSAtomPList atoms = mi.atoms;
    if (atoms.IsEmpty()) {
      if ((mi.bonds.Count() % 2) != 0) {
        E.ProcessingError(__OlxSrcInfo, "even number of bonds is expected");
        return;
      }
      for (size_t i = 0; i < mi.bonds.Count(); i += 2) {
        TSAtom *s = mi.bonds[i]->GetShared(*mi.bonds[i + 1]);
        if (s == 0) {
          TBasicApp::NewLogEntry(logError) << "No shared atom for: " <<
            mi.bonds[i]->A().GetLabel() << '-' << mi.bonds[i]->B().GetLabel()
            << " and " << mi.bonds[i + 1]->A().GetLabel() << '-' <<
            mi.bonds[i + 1]->B().GetLabel() << " skiping...";
        }
        else {
          atoms << mi.bonds[i]->Another(*s) << s << mi.bonds[i + 1]->Another(*s);
        }
      }
    }
    if ((atoms.Count() % 3) != 0) {
      E.ProcessingError(__OlxSrcInfo, "triplets of atoms are expected");
      return;
    }
    if (!atoms.IsEmpty()) {
      TSimpleRestraint &sr = rm.rAngle.AddNew();
      sr.SetValue(val);
      for (size_t i = 0; i < atoms.Count(); i++) {
        sr.AddAtom(atoms[i]->CAtom(), &atoms[i]->GetMatrix());
      }
    }
  }
  else if (Cmds[0].Equalsi("dihedral") && Cmds.Count() > 1) {
    double val = Cmds[1].ToDouble();
    Cmds.DeleteRange(0, 2);
    MacroInput mi = ExtractSelection(Cmds, true);
    TSAtomPList atoms = mi.atoms;
    TTypeList<TSAtomPList> quadruplets;
    if (atoms.IsEmpty()) {
      if ((mi.bonds.Count() % 2) != 0) {
        E.ProcessingError(__OlxSrcInfo, "even number of bonds is expected");
        return;
      }
      for (size_t i = 0; i < mi.bonds.Count(); i += 2) {
        ConstPtrList<TSAtom> dh = mi.bonds[i]->GetDihedral(*mi.bonds[i + 1]);
        if (dh.IsEmpty()) {
          TBasicApp::NewLogEntry(logError) << "Do not form dihedral: " <<
            mi.bonds[i]->A().GetLabel() << '-' << mi.bonds[i]->B().GetLabel()
            << " and " << mi.bonds[i + 1]->A().GetLabel()
            << '-' << mi.bonds[i + 1]->B().GetLabel() << " skiping...";
        }
        else
          quadruplets.AddNew(dh);
      }
    }
    else {
      if ((atoms.Count() % 4) != 0) {
        E.ProcessingError(__OlxSrcInfo, "quadruplets of atoms are expected");
        return;
      }
      for (size_t i = 0; i < atoms.Count(); i += 4) {
        TSAtomPList &l = quadruplets.AddNew(4);
        for (int j = 0; j < 4; j++) {
          l.Set(j, atoms[i + j]);
        }
      }
    }
    if (!quadruplets.IsEmpty()) {
      TSimpleRestraint &sr = rm.rDihedralAngle.AddNew();
      sr.SetValue(val);
      for (size_t i = 0; i < quadruplets.Count(); i++) {
        for (size_t j = 0; j < quadruplets[i].Count(); j++) {
          sr.AddAtom(quadruplets[i][j]->CAtom(),
            &quadruplets[i][j]->GetMatrix());
        }
      }
    }
  }
}
//..............................................................................
void XLibMacros::macConstrain(TStrObjList &Cmds,
  const TParamList &Options, TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  RefinementModel &rm = app.XFile().GetRM();
  if (Cmds[0].Equalsi("U") || Cmds[0].Equalsi("ADP")) { // EADP
    //Cmds.Delete(0);
    //TXAtomPList atoms = FindXAtoms(Cmds, false, true);
    //if( atoms.Count() < 2 )  {
    //  E.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
    //  return;
    //}
    //ACollectionItem::Unify(atoms,
    //  FunctionAccessor::MakeConst(&TXAtom::CAtom));
    //// search optimisation
    //SortedPtrList<TCAtom, TPointerComparator> s_catoms;
    //for( size_t i=0; i < atoms.Count(); i++ )
    //  s_catoms.AddUnique(&atoms[i]->CAtom());
    //TSimpleRestraint *e_sr = NULL;
    //for( size_t i=0; i < rm.rEADP.Count(); i++ )  {
    //  TSimpleRestraint &sr = rm.rEADP[i];
    //  for( size_t j=0; j < sr.AtomCount(); j++ )  {
    //    if( s_catoms.IndexOf(sr.GetAtom(j).GetAtom()) != InvalidIndex )  {
    //      e_sr = &sr;
    //      break;
    //    }
    //    if( e_sr != NULL )  break;
    //  }
    //  if( e_sr != NULL )  break;
    //}
    //if( e_sr == NULL )
    //  e_sr = &rm.rEADP.AddNew();
    //for( size_t i=0; i < atoms.Count(); i++ )
    //  e_sr->AddAtom(atoms[i]->CAtom(), NULL);
  }
  else if( Cmds[0].Equalsi("site") || Cmds[0].Equalsi("xyz"))  { // EXYZ
    Cmds.Delete(0);
    MacroInput mi = ExtractSelection(Cmds, true);
    TSAtomPList atoms = mi.atoms;
    if (atoms.Count() < 2) {
      E.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
      return;
    }
    ACollectionItem::Unify(atoms,
      FunctionAccessor::MakeConst(&TSAtom::CAtom));
    // search optimisation
    sorted::PointerPointer<TCAtom> s_catoms;
    for( size_t i=0; i < atoms.Count(); i++ )
      s_catoms.AddUnique(&atoms[i]->CAtom());
    TExyzGroup *e_g = NULL;
    for( size_t i=0; i < rm.rEADP.Count(); i++ )  {
      TExyzGroup &sr = rm.ExyzGroups[i];
      for( size_t j=0; j < sr.Count(); j++ )  {
        if( s_catoms.IndexOf(&sr[j]) != InvalidIndex )  {
          e_g = &sr;
          break;
        }
        if( e_g != NULL )  break;
      }
      if( e_g != NULL )  break;
    }
    if( e_g == NULL )
      e_g = &rm.ExyzGroups.New();
    for( size_t i=0; i < atoms.Count(); i++ )
      e_g->Add(atoms[i]->CAtom());
  }
  else if (Cmds[0].Equalsi("same") && // same group constraint
          (Cmds.Count() > 1 && Cmds[1].Equalsi("group")))
  {
    size_t n=InvalidSize;
    if (Cmds.Count() > 2 && Cmds[2].IsNumber()) {
      n = Cmds[2].ToSizeT();
      Cmds.DeleteRange(0, 3);
    }
    else
      Cmds.DeleteRange(0, 2);
    MacroInput mi = ExtractSelection(Cmds, true);
    TSAtomPList atoms = mi.atoms;
    if (n != InvalidSize && atoms.Count() > n*3) {
      if ((atoms.Count()%n) != 0) {
        E.ProcessingError(__OlxSrcInfo,
          "number of atoms do not match the number given of groups");
        return;
      }
      const size_t ag = atoms.Count()/n;
      TTypeList<TCAtomPList> groups(n);
      for (size_t i=0, ac=0; i < n; i++) {
        TCAtomPList &al = groups.Set(i, new TCAtomPList(ag));
        for (size_t j=0; j < ag; j++, ac++)
          al[j] = &atoms[ac]->CAtom();
      }
      app.XFile().GetRM().SameGroups.items.AddNew(
        ConstTypeList<TCAtomPList>(groups));
    }
    else if (atoms.Count() == 2) {
      TTypeList<olx_pair_t<size_t, size_t> > res;
      TNetwork &netA = atoms[0]->GetNetwork(),
        &netB = atoms[1]->GetNetwork();
      if (&netA == &netB) {
        E.ProcessingError(__OlxSrcInfo,
          "please select atoms from different fragments");
        return;
      }
      if (!netA.DoMatch(netB, res, false, &TSAtom::weight_unit)) {
        E.ProcessingError(__OlxSrcInfo, "fragments do not match");
        return;
      }
      TTypeList<olx_pair_t<TSAtom*,TSAtom*> > matoms(res.Count());
      for (size_t i=0; i < res.Count(); i++) {
        matoms.Set(i, new olx_pair_t<TSAtom*,TSAtom*>(
          &netA.Node(res[i].GetA()), &netB.Node(res[i].GetB())));
      }
      TNetwork::AlignInfo rv =
        TNetwork::GetAlignmentRMSD(matoms, false, &TSAtom::weight_unit);

      TTypeList<olx_pair_t<TSAtom*,TSAtom*> > imatoms(res.Count());
      res.Clear();
      netA.DoMatch(netB, res, true, &TSAtom::weight_unit);
      for (size_t i=0; i < res.Count(); i++) {
        imatoms.Set(i, new olx_pair_t<TSAtom*,TSAtom*>(
          &netA.Node(res[i].GetA()), &netB.Node(res[i].GetB())));
      }
      TNetwork::AlignInfo irv =
        TNetwork::GetAlignmentRMSD(imatoms, true, &TSAtom::weight_unit);
      TTypeList<olx_pair_t<TSAtom*,TSAtom*> > &ma =
        (rv.rmsd.GetV() < irv.rmsd.GetV() ? matoms : imatoms);
      TTypeList<TCAtomPList> groups;
      groups.AddNew(ma.Count());
      groups.AddNew(ma.Count());
      for (size_t i=0; i < ma.Count(); i++) {
        groups[0][i] = &ma[i].GetA()->CAtom();
        groups[1][i] = &ma[i].GetB()->CAtom();
      }
      app.XFile().GetRM().SameGroups.items.AddNew(
        ConstTypeList<TCAtomPList>(groups));
    }
  }
  else if (Cmds[0].Equalsi("same") && // same group constraint
          (Cmds.Count() > 1 && Cmds[1].Equalsi("adp")))
  {
    Cmds.DeleteRange(0, 2);
    ABasicFunction *bf =
      olex2::IOlex2Processor::GetInstance()->GetLibrary().FindMacro(
      "xf.rm.ShareADP");
    if (bf == NULL) {
      E.ProcessingError(__OlxSrcInfo, "could not locate required function");
      return;
    }
    else
      bf->Run(Cmds, Options, E);
  }
}
//..............................................................................
bool Tolman_MaskSub(TSAtom &a, int tag) {
  a.SetTag(tag);
  for (size_t i = 0; i < a.NodeCount(); i++) {
    if (a.Node(i).GetTag() == -1) {
      if (!Tolman_MaskSub(a.Node(i), tag)) {
        return false;
      }
    }
    else if (a.Node(i).GetTag() != 0 && a.Node(i).GetTag() != tag) {
      return false;
    }
  }
  return true;
}
void XLibMacros::macTolman(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  double mpd = Options.FindValue("mpd", "2.28").ToDouble();
  double chd = Options.FindValue("chd", "0.96").ToDouble();
  TSAtomPList atoms = app.FindSAtoms(TStrList() << "$P", false, false);
  for (size_t i = 0; i < atoms.Count(); i++) {
    TSAtom &sa = *atoms[i];
    size_t sc = 0, m_idx = InvalidIndex;
    vec3d s_c;
    for (size_t j = 0; j < sa.NodeCount(); j++) {
      if (sa.Node(j).IsDeleted() || sa.Node(j).GetType() == iQPeakZ) {
        continue;
      }
      if (XElementLib::IsMetal(sa.Node(j).GetType())) {
        if (m_idx != InvalidIndex) {
          sc = 0;
          break;
        }
        m_idx = j;
      }
      else {
        s_c += sa.Node(j).crd();
      }
      sc++;
    }
    vec3d v0, // M
      v1; // P
    double ang = 0;
    bool calculated = true;
    if (sc == 3 || (sc == 4 && m_idx != InvalidIndex)) {
      v1 = sa.crd();
      if (sc == 3) {
        v0 = v1 + (v1 - s_c/3).NormaliseTo(mpd);
      }
      else {
        v0 = v1 + (sa.Node(m_idx).crd() - v1).NormaliseTo(mpd);
      }
      ASObjectProvider &op = app.XFile().GetLattice().GetObjects();
      op.atoms.ForEach(ACollectionItem::TagSetter(-1));
      sa.SetTag(0);
      for (size_t j = 0; j < sa.NodeCount(); j++) {
        TSAtom &sa1 = sa.Node(j);
        if (sa1.IsDeleted() || sa1.GetType() == iQPeakZ) {
          continue;
        }
        if (m_idx == j) {
          continue;
        }
        if (!Tolman_MaskSub(sa1, (int)j)) {
          calculated = false;
          break;
        }
      }
      for (size_t j = 0; j < sa.NodeCount(); j++) {
        TSAtom &sa1 = sa.Node(j);
        if (sa1.IsDeleted() || sa1.GetType() == iQPeakZ) {
          continue;
        }
        if (m_idx == j) {
          continue;
        }
        double max_ang = 0;
        for (size_t k = 0; k < op.atoms.Count(); k++) {
          TSAtom& sa2 = op.atoms[k];
          if (sa2.IsDeleted() || sa2.GetType() == iQPeakZ ||
            sa2.GetTag() != j || sa2.GetType() != iHydrogenZ)
          {
            continue;
          }
          if (sa2.NodeCount() != 1) {
            calculated = false;
            break;
          }
          TSAtom &ba = sa2.Node(0);
          vec3d v = (ba.crd() + (sa2.crd()-ba.crd()).NormaliseTo(chd)) - v0;
          double a = acos(v.CAngle(v1-v0));
          a += asin(1.0/v.Length());
          if (a > max_ang) {
            max_ang = a;
          }
        }
        ang += max_ang;
      }
      if (calculated) {
        TBasicApp::NewLogEntry() << "Angle for " << sa.GetLabel() << ": "
          << olxstr::FormatFloat(3, 180 * (2 * ang / 3) / M_PI);
      }
    }
  }
  TBasicApp::NewLogEntry() << "Calculating Tolman cone angle: "
    "http://en.wikipedia.org/wiki/Tolman_cone_angle, Transition Met. Chem., 20, 533-539 (1995)";
  TBasicApp::NewLogEntry() << "Metal to P distance is: " << olxstr::FormatFloat(3, mpd);
  TBasicApp::NewLogEntry() << "C-H disance is: " << olxstr::FormatFloat(3, chd);
}

struct idx_pair_t : public olx_pair_t<size_t, size_t> {
  idx_pair_t(size_t a, size_t b)
    : olx_pair_t<size_t, size_t>(a, b)
  {
    if (a > b) {
      olx_swap(this->a, this->b);
    }
  }
  int Compare(const idx_pair_t &p) const {
    int d = olx_cmp(a, p.a);
    if (d == 0) {
      d = olx_cmp(b, p.b);
    }
    return d;
  }
};

struct PairListComparator {
  int Compare(const TTypeList<idx_pair_t> &a,
    const TTypeList<idx_pair_t> &b) const {
    int r = olx_cmp(a.Count(), b.Count());
    if (r == 0) {
      for (size_t i = 0; i < a.Count(); i++) {
        r = a[i].Compare(b[i]);
        if (r != 0) {
          break;
        }
      }
    }
    return r;
  }
};

template <class list_t, typename accessor_t>
void macSAME_expand(RefinementModel &rm, const list_t& groups,
  const accessor_t &acc)
{
  DistanceGenerator::atom_set_t atom_set;
  atom_set.SetCapacity(groups[0].Count());
  DistanceGenerator::atom_map_N_t atom_map;
  atom_map.SetCapacity(groups[0].Count());
  for (size_t i = 0; i < groups[0].Count(); i++) {
   const  TCAtom &a = olx_ref::get(acc(groups[0][i]));
    atom_set.Add(a.GetId());
    TSizeList &idl = atom_map.Add(a.GetId());
    for (size_t j = 1; j < groups.Count(); j++) {
      idl.Add(olx_ref::get(acc(groups[j][i])).GetId());
    }
  }
  DistanceGenerator d;
  d.Generate(rm.aunit, atom_set, true, true);
  d.GenerateSADI(rm, atom_map);
}
void XLibMacros::macSame(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  const bool invert = Options.Contains("i"),
    expand = Options.Contains("e"),
    self = Options.GetBoolOption('s'),
    all = Options.GetBoolOption("all");
  if (all && self) {
    TBasicApp::NewLogEntry(logError) << "Sorry the option combination is "
      "currently unsupported";
    return;
  }
  size_t groups_count = InvalidSize;
  if (!Cmds.IsEmpty() && Cmds[0].IsNumber()) {
    groups_count = Cmds[0].ToSizeT();
    Cmds.Delete(0);
  }
  TXApp &app = TXApp::GetInstance();
  TSAtomPList atoms = app.FindSAtoms(Cmds, false, true);
  TSameGroup *created = NULL;
  if (atoms.Count() == 1 && invert) {
    if (atoms[0]->CAtom().GetResiId() != 0) {
      TSameGroup& sg = app.XFile().GetRM().rSAME.New();
      TSAtomPList atms;
      TResidue &r = atoms[0]->CAtom().GetParent()->GetResidue(
        atoms[0]->CAtom().GetResiId());
      for (size_t i = 0; i < r.Count(); i++) {
        if (r[i].GetType().z > 1) {
          sg.GetAtoms().AddExplicit(r[i]);
        }
      }
      sg.GetAtoms().ConvertToImplicit();
    }
  }
  else if (atoms.Count() == 2) {
    TTypeList< olx_pair_t<size_t, size_t> > res;
    TNetwork &netA = atoms[0]->GetNetwork(),
      &netB = atoms[1]->GetNetwork();
    if (&netA == &netB) {
      E.ProcessingError(__OlxSrcInfo, "Please select different fragments");
      return;
    }
    if (!netA.DoMatch(netB, res, invert, &TSAtom::weight_occu)) {
      E.ProcessingError(__OlxSrcInfo, "Graphs do not match");
      return;
    }
    //got the pairs now...
    if (expand) {
      TArrayList<TSAtomPList> groups(2);
      groups[0].SetCapacity(res.Count());
      groups[1].SetCapacity(res.Count());
      for (size_t i = 0; i < res.Count(); i++) {
        if (netA.Node(res[i].GetA()).GetType().z < 2) {
          continue;
        }
        groups[0].Add(netA.Node(res[i].GetA()));
        groups[1].Add(netB.Node(res[i].GetB()));
      }
      macSAME_expand(app.XFile().GetRM(), groups,
        FunctionAccessor::MakeConst(&TSAtom::CAtom));
    }
    else {
      TSameGroup* sg = 0;
      for (size_t i = 0; i < netA.NodeCount(); i++) {
        netA.Node(i).SetTag(-1);
        netB.Node(i).SetTag(-1);
      }
      bool first = true;
      for (size_t i = 0; i < res.Count(); i++) {
        if (netA.Node(res[i].GetA()).GetType().z < 2 ||
          netA.Node(res[i].GetA()).GetTag() == 0)
        {
          continue;
        }
        if (first) {
          if (olx_is_valid_index(netA.Node(res[i].GetA()).CAtom().GetSameId())) {
            TSameGroup &asg = app.XFile().GetRM().rSAME[
              netA.Node(res[i].GetA()).CAtom().GetSameId()];
            if (asg.GetParentGroup() == 0) {
              sg = &asg;
              break;
            }
          }
          if (sg == 0) {
            sg = &app.XFile().GetRM().rSAME.New();
          }
          first = false;
        }
        sg->Add(netA.Node(res[i].GetA()).CAtom());
        netA.Node(res[i].GetA()).SetTag(0);
      }
      TBasicApp::NewLogEntry();
      TSameGroup& d_sg = app.XFile().GetRM().rSAME.NewDependent(*sg);
      for (size_t i = 0; i < res.Count(); i++) {
        if (netB.Node(res[i].GetB()).GetType().z < 2 ||
          netB.Node(res[i].GetB()).GetTag() == 0)
        {
          continue;
        }
        d_sg.Add(netB.Node(res[i].GetB()).CAtom());
        netB.Node(res[i].GetB()).SetTag(0);
        TBasicApp::GetLog() << netB.Node(res[i].GetB()).GetLabel() << ' ';
      }
      TBasicApp::NewLogEntry();
    }
  }
  else if (groups_count != InvalidSize && (atoms.Count() % groups_count) == 0) {
    const size_t cnt = atoms.Count() / groups_count;
    if (expand) {
      TArrayList<TSAtomPList> groups(groups_count);
      for (size_t i = 0; i < cnt; i++) {
        for (size_t j = 0; j < groups_count; j++) {
          groups[j].Add(atoms[cnt*j + i]);
        }
      }
      macSAME_expand(app.XFile().GetRM(), groups,
        FunctionAccessor::MakeConst(&TSAtom::CAtom));
    }
    else {
      TPtrList<TSameGroup> deps;
      TSameGroup* sg = 0;
      if (olx_is_valid_index(atoms[0]->CAtom().GetSameId())) {
        TSameGroup &asg = app.XFile().GetRM().rSAME[
          atoms[0]->CAtom().GetSameId()];
        if (asg.GetParentGroup() == 0) {
          sg = &asg;
        }
      }
      if (sg == 0) {
        sg = &app.XFile().GetRM().rSAME.New();
      }
      for (size_t i = 0; i < groups_count - 1; i++) {
        deps.Add(app.XFile().GetRM().rSAME.NewDependent(*sg));
      }
      if (!sg->GetAtoms().IsEmpty()) {
        sg = 0;
      }
      for (size_t i = 0; i < cnt; i++) {
        if (sg != 0) {
          sg->Add(atoms[i]->CAtom());
        }
        for (size_t j = 1; j < groups_count; j++) {
          deps[j - 1]->Add(atoms[cnt*j + i]->CAtom());
        }
      }
      TBasicApp::NewLogEntry() << "SAME instruction is added";
    }
  }
  else if (groups_count == InvalidIndex && atoms.Count() > 2 && self) {
    if (expand) {
      TArrayList<TSAtomPList> groups(2);
      if (atoms[0]->IsConnectedTo(*atoms.GetLast())) {
        for (size_t i = 0; i < atoms.Count(); i++) {
          groups[0].Add(atoms[i]);
          groups[1].Add(atoms[i == 0 ? 0 : atoms.Count() - i]);
        }
      }
      else {
        for (size_t i = 0; i < atoms.Count(); i++) {
          groups[0].Add(atoms[i]);
          groups[1].Add(atoms[atoms.Count() - i - 1]);
        }
      }
      macSAME_expand(app.XFile().GetRM(), groups,
        FunctionAccessor::MakeConst(&TSAtom::CAtom));
    }
    else {
      TSameGroup *sg = 0;
      if (olx_is_valid_index(atoms[0]->CAtom().GetSameId())) {
        TSameGroup &asg = app.XFile().GetRM().rSAME[
          atoms[0]->CAtom().GetSameId()];
        if (asg.GetParentGroup() == 0) {
          sg = &asg;
        }
      }
      if (sg == 0) {
        sg = &app.XFile().GetRM().rSAME.New();
      }
      TSameGroup &dep = app.XFile().GetRM().rSAME.NewDependent(*sg);
      if (!sg->GetAtoms().IsEmpty()) {
        sg = 0;
      }
      if (atoms[0]->IsConnectedTo(*atoms.GetLast())) {
        for (size_t i = 0; i < atoms.Count(); i++) {
          if (sg != 0) {
            sg->Add(atoms[i]->CAtom());
          }
          dep.Add(atoms[i == 0 ? 0 : atoms.Count() - i]->CAtom());
        }
      }
      else {
        for (size_t i = 0; i < atoms.Count(); i++) {
          if (sg != 0) {
            sg->Add(atoms[i]->CAtom());
          }
          dep.Add(atoms[atoms.Count() - i - 1]->CAtom());
        }
      }
      TBasicApp::NewLogEntry() << "SAME instruction is added";
    }
  }
  else if (groups_count == InvalidSize && atoms.Count() > 2 && all) {
    using namespace olx_analysis;
    TCAtomPList fa(atoms, FunctionAccessor::MakeConst(&TSAtom::CAtom));
    ACollectionItem::Unify(fa);
    fragments::fragment fr(fa);
    TTypeList<fragments::fragment> frags =
      fragments::extract(app.XFile().GetAsymmUnit(), fr);
    RefinementModel &rm = app.XFile().GetRM();
    if (!frags.IsEmpty()) {
      if (expand) {
        TArrayList<TCAtomPList> groups(frags.Count() + 1);
        for (size_t i = 0; i < frags.Count() + 1; i++) {
          fragments::fragment &f = (i == 0 ? fr : frags[i - 1]);
          for (size_t j = 0; j < f.count(); j++) {
            groups[i].Add(f[j]);
          }
        }
        macSAME_expand(app.XFile().GetRM(), groups,
          DummyAccessor());
      }
      else {
        TSameGroup &rg = rm.rSAME.New();
        for (size_t i = 0; i < fr.count(); i++) {
          if (fr[i].GetType().z > 1) {
            rg.Add(fr[i]);
          }
        }
        for (size_t fi = 0; fi < frags.Count(); fi++) {
          TSameGroup &dg = rm.rSAME.NewDependent(rg);
          for (size_t i = 0; i < frags[0].count(); i++) {
            if (fr[i].GetType().z > 1) {
              dg.Add(frags[fi][i]);
            }
          }
        }
      }
    }
    else {
      TBasicApp::NewLogEntry(logError) << "could not locate any matching "
        "fragments";
    }
  }
  else {
    E.ProcessingError(__OlxSrcInfo, "invalid input arguments");
    return;
  }
  app.XFile().GetRM().rSAME.FixIds();
}
//.............................................................................
void XLibMacros::macRIGU(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  double esd1 = 0.01, esd2=0.01;  // esd
  size_t cnt = XLibMacros::Parse(Cmds, "dd", &esd1, &esd2);
  if (cnt == 1) esd2 = esd1;
  TXApp &app = TXApp::GetInstance();
  TSAtomPList Atoms = app.FindSAtoms(Cmds, false, !Options.Contains("cs"));
  // validate that atoms of the same type
  TSimpleRestraint& sr = app.XFile().GetRM().rRIGU.AddNew();
  if (cnt > 0) {
    sr.SetEsd(esd1);
    sr.SetEsd1(esd2);
  }
  sr.SetAllNonHAtoms(Atoms.IsEmpty());
  for (size_t i = 0; i < Atoms.Count(); i++) {
    sr.AddAtom(Atoms[i]->CAtom(), 0);
  }
  app.XFile().GetRM().rRIGU.ValidateRestraint(sr);
  if (!sr.IsEmpty()) {
    if (Options.GetBoolOption('i')) {
      sr.ConvertToImplicit();
    }
    TBasicApp::NewLogEntry() << "Placing the following restraints: ";
    TBasicApp::NewLogEntry() << sr.ToString();
  }
}
//.............................................................................
void XLibMacros::macRESI(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  olxstr resi_class = Cmds[0];
  int resi_number = TResidue::NoResidue;
  if (resi_class.IsNumber()) {
    resi_number = resi_class.ToInt();
    resi_class.SetLength(0);
  }
  Cmds.Delete(0);
  if (resi_number == TResidue::NoResidue &&
    (Cmds.Count() > 0  && Cmds[0].IsNumber()))
  {
    resi_number = Cmds[0].ToInt();
    Cmds.Delete(0);
  }
  bool same = false;
  if (!Cmds.IsEmpty() && Cmds[0].Equalsi("SAME")) {
    same = true;
    Cmds.Delete(0);
  }
  TSAtomPList atoms = app.FindSAtoms(Cmds, false, true);
  if (atoms.IsEmpty()) {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  ACollectionItem::Unify(atoms);
  if (resi_class.Equalsi("none")) {
    TResidue& main_resi = au.GetResidue(0);
    for (size_t i=0; i < atoms.Count(); i++) {
      TCAtom& ca = atoms[i]->CAtom();
      if (olx_is_valid_index(ca.GetResiId())) {
        main_resi.Add(ca);
      }
    }
  }
  else {
    if (Options.GetBoolOption("all")) {
      using namespace olx_analysis;
      TCAtomPList fa(atoms, FunctionAccessor::MakeConst(&TSAtom::CAtom));
      ACollectionItem::Unify(fa);
      fragments::fragment fr(fa);
      TTypeList<fragments::fragment> frags =
        fragments::extract(app.XFile().GetAsymmUnit(), fr);
      for (size_t fi = 0; fi <= frags.Count(); fi++) {
        fragments::fragment *f = (fi == 0 ? &fr : &frags[fi - 1]);
        TResidue *resi_;
        if (resi_number == TResidue::NoResidue) {
          resi_ = &au.NewResidue(resi_class, TResidue::NoResidue,
            TResidue::NoResidue, TResidue::NoChainId());
          resi_number = resi_->GetNumber() + 1;
        }
        else {
          int rn = resi_number++;
          resi_ = &au.NewResidue(resi_class, rn, rn, TResidue::NoChainId());
        }
        TResidue &resi = *resi_;
        resi.SetCapacity(f->count());
        for (size_t i = 0; i < f->count(); i++) {
          TCAtom &a = (*f)[i];
          resi.Add(a);
          if (fi > 0) {
            a.SetLabel(fr[i].GetLabel(), false);
          }
        }
      }
      if (frags.Count() > 0 && same) {
        RefinementModel &rm = app.XFile().GetRM();
        TSameGroup &rg = rm.rSAME.New();
        for (size_t i = 0; i < fr.count(); i++) {
          if (fr[i].GetType().z > 1) {
            rg.Add(fr[i]);
          }
        }
        for (size_t fi = 0; fi < frags.Count(); fi++) {
          TSameGroup &dg = rm.rSAME.NewDependent(rg);
          for (size_t i = 0; i < frags[0].count(); i++) {
            if (fr[i].GetType().z > 1) {
              dg.Add(frags[fi][i]);
            }
          }
        }
      }
    }
    else {
      olxstr ch_id = Options.FindValue('c', TResidue::NoChainId());
      if (ch_id.IsEmpty()) {
        ch_id = TResidue::NoChainId();
      }
      TResidue& resi = au.NewResidue(resi_class, resi_number,
        Options.FindValue('a', resi_number).ToInt(),
        ch_id.CharAt(0));
      if (!resi.IsEmpty()) {
        TBasicApp::NewLogEntry(logWarning) <<
          "Warning - appending atoms to existing, non-empty residue!";
      }
      resi.SetCapacity(atoms.Count());
      for (size_t i = 0; i < atoms.Count(); i++) {
        resi.Add(atoms[i]->CAtom());
      }
    }
  }
}
//.............................................................................
void XLibMacros::macSplit(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TXApp &app = TXApp::GetInstance();
  TStrList restraints(Options.FindValue("r", EmptyString()).ToLowerCase(), ',');
  TCAtomPList Atoms(app.FindSAtoms(Cmds, false, true),
    FunctionAccessor::MakeConst(&TSAtom::CAtom));
  if (Atoms.IsEmpty()) {
    return;
  }
  bool same = restraints.Contains("same");
  DistanceGenerator::atom_map_1_t atom_map;
  DistanceGenerator::atom_set_t atom_set;
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  TLattice& latt = app.XFile().GetLattice();
  RefinementModel& rm = app.XFile().GetRM();
  TCAtomPList ProcessedAtoms;
  XVar& var = rm.Vars.NewVar();
  for (size_t i = 0; i < Atoms.Count(); i++) {
    TCAtom* CA = Atoms[i];
    if (CA->GetEllipsoid() == 0) {
      continue;
    }
    vec3d direction;
    double Length = 0;
    olxstr lbl = CA->GetLabel();
    if (CA->GetEllipsoid()->GetSX() > CA->GetEllipsoid()->GetSY()) {
      if (CA->GetEllipsoid()->GetSX() > CA->GetEllipsoid()->GetSZ()) {
        Length = CA->GetEllipsoid()->GetSX();
        direction = CA->GetEllipsoid()->GetMatrix()[0];
      }
      else {
        Length = CA->GetEllipsoid()->GetSZ();
        direction = CA->GetEllipsoid()->GetMatrix()[2];
      }
    }
    else {
      if (CA->GetEllipsoid()->GetSY() > CA->GetEllipsoid()->GetSZ()) {
        Length = CA->GetEllipsoid()->GetSY();
        direction = CA->GetEllipsoid()->GetMatrix()[1];
      }
      else {
        Length = CA->GetEllipsoid()->GetSZ();
        direction = CA->GetEllipsoid()->GetMatrix()[2];
      }
    }
    direction *= Length;
    direction /= 2;
    au.CartesianToCell(direction);
    const double sp = 1. / CA->GetDegeneracy();
    TAsymmUnit::TLabelChecker lc(au);
    TSAtom &A = latt.NewAtom(vec3d());
    TCAtom& CA1 = A.CAtom();
    CA1.Assign(*CA);
    CA1.SetSameId(~0);
    CA1.SetPart(1);
    CA1.ccrd() += direction;
    A.ccrd() = CA1.ccrd();
    CA1.SetLabel(lc.CheckLabel(CA1, lbl + 'a', 0, true), false);
    // link occupancies
    rm.Vars.AddVarRef(var, CA1, catom_var_name_Sof, relation_AsVar, 1);
    CA1.SetOccu(0.5*sp);
    ProcessedAtoms.Add(CA1);
    TCAtom& CA2 = *CA;
    CA2.SetPart(2);
    CA2.ccrd() -= direction;
    CA2.SetLabel(lc.CheckLabel(CA2, lbl + 'b', 0, true), false);
    // link occupancies
    rm.Vars.AddVarRef(var, CA2, catom_var_name_Sof, relation_AsOneMinusVar, 1);
    CA2.SetOccu(0.5*sp);
    ProcessedAtoms.Add(CA2);
    if (same) {
      atom_map.Add(CA2.GetId(), CA1.GetId());
      atom_set.Add(CA2.GetId());
    }
    if (restraints.Contains("eadp")) {
      rm.rEADP.AddNew().AddAtomPair(CA1, 0, CA2, 0);
    }
    if (restraints.Contains("isor")) {
      rm.rISOR.AddNew().AddAtomPair(CA1, 0, CA2, 0);
    }
    if (restraints.Contains("simu")) {
      rm.rSIMU.AddNew().AddAtomPair(CA1, 0, CA2, 0);
    }
    if (restraints.Contains("delu")) {
      rm.rDELU.AddNew().AddAtomPair(CA1, 0, CA2, 0);
    }
  }
  if (same) {
    DistanceGenerator dg;
    dg.Generate(au, atom_set, true, true);
    dg.GenerateSADI(rm, atom_map, 2);
  }

  latt.SetAnis(ProcessedAtoms, false);
  latt.Uniq();
  if (TXApp::DoUseSafeAfix()) {
    app.GetUndo().Push(
      app.XFile().GetLattice().ValidateHGroups(true, true));
  }
}
//.............................................................................
void XLibMacros::macTLS(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp &app = TXApp::GetInstance();
  TSAtomPList atoms = app.FindSAtoms(Cmds, true, true);
  for (size_t i=0; i < atoms.Count(); i++) {
    if (atoms[i]->GetEllipsoid() == 0) {
      atoms[i] = 0;
    }
  }
  if (atoms.Pack().Count() < 4) {
    Error.ProcessingError(__OlxSrcInfo,
      "at least 4 anisotropic atoms expected");
    return;
  }
  evecd Q(6);
  TAsymmUnit &au = app.XFile().GetAsymmUnit();
  xlib::TLS tls(atoms);
  if (!Options.GetBoolOption('q')) {
    olxstr ttitle = "TLS analysis for: ";
    tls.printTLS(ttitle << olx_analysis::alg::label(atoms, true, ' '));
    tls.printFOM();
    tls.printDiff();
  }
  if (Options.GetBoolOption('a') &&
      tls.GetElpList().Count() == atoms.Count())
  {
    for (size_t i=0; i < atoms.Count(); i++) {
      atoms[i]->GetEllipsoid()->GetShelxQuad(Q);
      atoms[i]->GetEllipsoid()->Initialise(tls.GetElpList()[i]);
      if (atoms[i]->GetMatrix().IsFirst()) {
        *atoms[i]->CAtom().GetEllipsoid() = tls.GetElpList()[i];
      }
    }
  }
}
//.............................................................................
void XLibMacros::funCell(const TStrObjList& Params, TMacroData &E)  {
  TXApp &app = TXApp::GetInstance();
  if( Params[0].Equalsi('a') )
    E.SetRetVal(app.XFile().GetAsymmUnit().GetAxes()[0]);
  else if( Params[0].Equalsi('b') )
    E.SetRetVal(app.XFile().GetAsymmUnit().GetAxes()[1]);
  else if( Params[0].Equalsi('c') )
    E.SetRetVal(app.XFile().GetAsymmUnit().GetAxes()[2]);
  else if( Params[0].Equalsi("alpha") )
    E.SetRetVal(app.XFile().GetAsymmUnit().GetAngles()[0]);
  else if( Params[0].Equalsi("beta") )
    E.SetRetVal(app.XFile().GetAsymmUnit().GetAngles()[1]);
  else if( Params[0].Equalsi("gamma") )
    E.SetRetVal(app.XFile().GetAsymmUnit().GetAngles()[2]);
  else if( Params[0].Equalsi("volume") )
    E.SetRetVal(olxstr::FormatFloat(2, app.XFile().GetUnitCell().CalcVolume()));
  else
    E.ProcessingError(__OlxSrcInfo, "invalid argument: ") << Params[0];
}
//..............................................................................
void XLibMacros::funCif(const TStrObjList& Params, TMacroData &E)  {
  TXApp &app = TXApp::GetInstance();
  TCif& cf = app.XFile().GetLastLoader<TCif>();
  if (cf.ParamExists(Params[0])) {
    E.SetRetVal(cf.GetParamAsString(Params[0]));
  }
  else {
    size_t idx = InvalidIndex;
    if (Params.Count() == 2) {
      idx = Params[1].ToSizeT();
    }
    olx_object_ptr<olx_pair_t<cif_dp::cetTable*, size_t> > t = cf.FindLoopItem(Params[0]);
    
    if (!t.ok() || (idx != InvalidIndex && t->a->RowCount() < idx)) {
      E.SetRetVal(XLibMacros::NAString());
      return;
    }
    // return all values
    if (idx == InvalidIndex) {
      TStrList rv;
      for (size_t i = 0; i < t->a->RowCount(); i++) {
        rv << (*t->a)[i][t->b]->GetStringValue();
      }
      E.SetRetVal(olxstr(",").Join(rv));
    }
    else {
      E.SetRetVal((*t->a)[idx][t->b]->GetStringValue());
    }
  }
}
//..............................................................................
void XLibMacros::funP4p(const TStrObjList& Params, TMacroData &E)  {
}
//.............................................................................
void XLibMacros::funCrs(const TStrObjList& Params, TMacroData& E) {
  TXApp& app = TXApp::GetInstance();
  TCRSFile& cf = app.XFile().GetLastLoader<TCRSFile>();
  if (Params[0].Equalsi("sg")) {
    E.SetRetVal(cf.GetSG() != 0 ? cf.GetSG()->GetName() : EmptyString());
  }
  else {
    E.SetRetVal(XLibMacros::NAString());
  }
}
//.............................................................................
class helper_Tetrahedron  {
  vec3d_list Points;
  olxstr Name;
  double Volume;
protected:
  double CalcVolume()  {
    return olx_tetrahedron_volume(Points[0], Points[1], Points[2], Points[3]);
  }
public:
  helper_Tetrahedron(const olxstr& name) : Name(name)  {
    Name = name;
    Volume = -1;
  }
  void AddPoint(const vec3d& p)  {
    Points.AddNew(p);
    if( Points.Count() == 4 )
      Volume = CalcVolume();
  }
  const olxstr& GetName() const {  return Name;  }
  const vec3d& operator [] (size_t i) const {  return Points[i];  }
  double GetVolume() const {  return Volume;  }
  int Compare(const helper_Tetrahedron& th) const {
    const double v = GetVolume() - th.GetVolume();
    return v < 0 ? -1 : (v > 0 ? 1 : 0);
  }
};
void XLibMacros::macCalcVol(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp &app = TXApp::GetInstance();
  TSAtomPList xatoms = app.FindSAtoms(Cmds, false, true);
  bool normalise = Options.GetBoolOption('n');
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    TSAtomPList atoms;
    for( size_t j=0; j < xatoms[i]->NodeCount(); j++ ) {
      TSAtom& A = xatoms[i]->Node(j);
      if( A.IsDeleted() || (A.GetType() == iQPeakZ) )
        continue;
      atoms.Add(A);
    }
    if( atoms.Count() < 3 ) continue;
    if( normalise ) {
      TBasicApp::NewLogEntry() << (olxstr("Current atom: ") <<
        xatoms[i]->GetLabel() << " (volumes for normalised bonds)");
    }
    else
      TBasicApp::NewLogEntry() << "Current atom: " << xatoms[i]->GetLabel();
    if( atoms.Count() == 3 )  {
      double sa = olx_angle(atoms[0]->crd(), xatoms[i]->crd(), atoms[1]->crd());
      sa += olx_angle(atoms[0]->crd(), xatoms[i]->crd(), atoms[2]->crd());
      sa += olx_angle(atoms[1]->crd(), xatoms[i]->crd(), atoms[2]->crd());
      TBasicApp::NewLogEntry() << "Sum of angles is " << olxstr::FormatFloat(3,sa);
      double v;
      if( normalise )  {
        v = olx_tetrahedron_volume(
        xatoms[i]->crd(),
        xatoms[i]->crd() + (atoms[0]->crd()-xatoms[i]->crd()).Normalise(),
        xatoms[i]->crd() + (atoms[1]->crd()-xatoms[i]->crd()).Normalise(),
        xatoms[i]->crd() + (atoms[2]->crd()-xatoms[i]->crd()).Normalise());
      }
      else {
        v = olx_tetrahedron_volume(xatoms[i]->crd(), atoms[0]->crd(),
          atoms[1]->crd(), atoms[2]->crd());
      }
      TBasicApp::NewLogEntry() << "The tetrahedron volume is " <<
        olxstr::FormatFloat(3,v);
    }
    else if( atoms.Count() == 4 )  {
      double v;
      if( normalise )  {
        v = olx_tetrahedron_volume(
        xatoms[i]->crd() + (atoms[0]->crd()-xatoms[i]->crd()).Normalise(),
        xatoms[i]->crd() + (atoms[1]->crd()-xatoms[i]->crd()).Normalise(),
        xatoms[i]->crd() + (atoms[2]->crd()-xatoms[i]->crd()).Normalise(),
        xatoms[i]->crd() + (atoms[3]->crd()-xatoms[i]->crd()).Normalise());
      }
      else {
        v = olx_tetrahedron_volume(atoms[0]->crd(), atoms[1]->crd(),
          atoms[2]->crd(), atoms[3]->crd());
      }
      TBasicApp::NewLogEntry() << "The tetrahedron volume is " <<
        olxstr::FormatFloat(3, v);
    }
    else  {
      TTypeList<helper_Tetrahedron> tetrahedra;
      for( size_t i1=0; i1 < atoms.Count(); i1++ ) {
        for( size_t i2=i1+1; i2 < atoms.Count(); i2++ ) {
          for( size_t i3=i2+1; i3 < atoms.Count(); i3++ ) {
            helper_Tetrahedron& th = tetrahedra.AddNew(
              olxstr(xatoms[i]->GetLabel()) << '-' <<
              atoms[i1]->GetLabel() << '-' << atoms[i2]->GetLabel() << '-' <<
              atoms[i3]->GetLabel());
            if( normalise )  {
              th.AddPoint(xatoms[i]->crd());
              th.AddPoint(xatoms[i]->crd() +
                (atoms[i1]->crd()-xatoms[i]->crd()).Normalise());
              th.AddPoint(xatoms[i]->crd() +
                (atoms[i2]->crd()-xatoms[i]->crd()).Normalise());
              th.AddPoint(xatoms[i]->crd() +
                (atoms[i3]->crd()-xatoms[i]->crd()).Normalise());
            }
            else  {
              th.AddPoint(xatoms[i]->crd());
              th.AddPoint(atoms[i1]->crd());
              th.AddPoint(atoms[i2]->crd());
              th.AddPoint(atoms[i3]->crd());
            }
          }
        }
      }
      const size_t thc = (atoms.Count()-2)*2;
      QuickSorter::Sort(tetrahedra);
      for( size_t j=0; j < tetrahedra.Count(); j++ )  {
        TBasicApp::NewLogEntry() << "Tetrahedron " << j+1 <<  ' ' <<
          tetrahedra[j].GetName() << " V = " << tetrahedra[j].GetVolume();
      }
      while(  tetrahedra.Count() > thc )
        tetrahedra.Delete(0);
      double v = 0;
      for( size_t j=0; j < tetrahedra.Count(); j++ )
        v += tetrahedra[j].GetVolume();
      TBasicApp::NewLogEntry() << "The sum of volumes of " << thc <<
        " largest tetrahedra is " << olxstr::FormatFloat(3, v);
    }
  }
}
//.............................................................................
void XLibMacros::funVVol(const TStrObjList& Params, TMacroData &E) {
  TXApp &app = TXApp::GetInstance();
  ElementRadii radii;
  if( Params.Count() == 1 && TEFile::Exists(Params[0]) )
    radii = TXApp::ReadRadii(Params[0]);
  TXApp::PrintVdWRadii(radii, app.XFile().GetAsymmUnit().GetContentList());

  const TXApp::CalcVolumeInfo vi = app.CalcVolume(&radii);
  olxstr report;
  E.SetRetVal(olxstr::FormatFloat(2, vi.total-vi.overlapping));
  TBasicApp::NewLogEntry(logWarning) << "Please note that this is a highly "
    "approximate procedure. Volume of current fragment is calculated using a "
    "maximum two overlaping spheres, to calculate packing indexes, use "
    "calcvoid or MolInfo instead";
  TBasicApp::NewLogEntry() << "Molecular volume (A): " <<
    olxstr::FormatFloat(2, vi.total-vi.overlapping);
  TBasicApp::NewLogEntry() << "Overlapping volume (A): " <<
    olxstr::FormatFloat(2, vi.overlapping);
}
//..............................................................................
void XLibMacros::funEnv(const TStrObjList& Params, TMacroData& E) {
  TXApp& app = TXApp::GetInstance();
  TSAtomPList atoms = app.FindSAtoms(TStrList() << Params[0], false, true);
  if (atoms.Count() != 1) {
    return;
  }
  olxstr tmp;
  for (size_t i = 0; i < atoms[0]->NodeCount(); i++) {
    tmp << atoms[0]->Node(i).GetLabel() << ' ';
  }
  E.SetRetVal(tmp);
}
//..............................................................................
void XLibMacros::macBang(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp &app = TXApp::GetInstance();
  TTTable<TStrList> Table;
  olxstr Tmp = Cmds.Text(' '), clipbrd;
  TSAtomPList Atoms = app.FindSAtoms(Cmds, false);
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    app.BangTable(*Atoms[i], Table);
    TStrList Output;
    TBasicApp::NewLogEntry() <<
      Table.CreateTXTList(Output, EmptyString(), true, true, ' ');
    clipbrd << Output.Text(NewLineSequence());
  }
  if (Options.GetBoolOption('c'))
    app.ToClipboard(clipbrd);
}
//..............................................................................
void XLibMacros::funSGList(const TStrObjList &, TMacroData &E) {
  E.SetRetVal(TXApp::GetInstance().GetLastSGResult());
}
//..............................................................................
int RSA_BondOrder(const TCAtom &a, const TCAtom ::Site &to) {
  double d = a.GetParent()->Orthogonalise(a.ccrd()-to.matrix*to.atom->ccrd())
    .Length();
  if (a.GetType().z == iCarbonZ || to.atom->GetType().z == iCarbonZ) {
    const cm_Element &other = (a.GetType().z == iCarbonZ ? to.atom->GetType()
      : a.GetType());
    if (other.z == iCarbonZ) { //C-C
      if (d < 1.2) return 3;
      if (d < 1.45) return 2;
      return 1;
    }
    if (other.z == iOxygenZ) { //C-O
      if (d < 1.3) return 2;
      return 1;
    }
    if (other.z == iNitrogenZ) { //C-N
      if (d < 1.2) return 3;
      if (d < 1.4) return 2;
      return 1;
    }
  }
  if (a.GetType().z == iNitrogenZ || to.atom->GetType().z == iNitrogenZ) {
    const cm_Element &other = (a.GetType().z == iCarbonZ ? to.atom->GetType()
      : a.GetType());
    if (other.z == iOxygenZ) {
      if (d < 1.2) return 2;
      return 1;
    }
  }
  return 1;
}

typedef olx_pair_t<const TCAtom::Site *, const cm_Element *> SiteInfo;
typedef TTypeList<SiteInfo > AtomEnvList;
int RSA_CompareSites(const SiteInfo &a, const SiteInfo &b) {
  return olx_cmp(a.GetB()->z, b.GetB()->z);
}
int RSA_GetAtomPriorityX(AtomEnvList &a, AtomEnvList &b) {
  size_t sz = a.Count();
  if (sz == 0) {
    return 0;
  }
  for (size_t i=0; i < sz; i++) {
    size_t ai = a.Count()-i-1;
    size_t bi = b.Count()-i-1;
    int res = RSA_CompareSites(a[ai], b[bi]);
    if (a[ai].GetA() != 0) {
      if (a[ai].a->atom->GetTag() == 0) {
        a[ai].a->atom->SetTag(2);
      }
    }
    if (b[bi].GetA() != 0) {
      if (b[bi].a->atom->GetTag() == 0) {
        b[bi].a->atom->SetTag(3);
      }
    }
    if (res != 0) {
      return res;
    }
  }
  // equal? expand further
  for (size_t i=0; i < sz; i++) {
    AtomEnvList aa, bb;
    if (a[i].GetA() != 0) {
      TCAtom &atomA = *a[i].GetA()->atom;
      for (size_t j=0; j < atomA.AttachedSiteCount(); j++) {
        TCAtom::Site &s = atomA.GetAttachedSite(j);
        if (s.atom->GetTag() != 0 ||
          s.atom->GetType() == iQPeakZ || s.atom->IsDeleted())
        {
          continue;
        }
        aa.Add(new SiteInfo(&s, &s.atom->GetType()));
        int bo = RSA_BondOrder(atomA, s);
        for (int k = 1; k < bo; k++) {
          aa.Add(new SiteInfo(0, &s.atom->GetType()));
        }
      }
    }
    if (b[i].GetA() != 0) {
      TCAtom &atomB = *b[i].GetA()->atom;
      for (size_t j=0; j < atomB.AttachedSiteCount(); j++) {
        TCAtom::Site &s = atomB.GetAttachedSite(j);
        if (s.atom->GetTag() != 0 ||
          s.atom->GetType() == iQPeakZ || s.atom->IsDeleted())
        {
          continue;
        }
        bb.Add(new SiteInfo(&s, &s.atom->GetType()));
        int bo = RSA_BondOrder(atomB, s);
        for (int k = 1; k < bo; k++) {
          bb.Add(new SiteInfo(0, &s.atom->GetType()));
        }
      }
    }
    // padd the branches
    while (aa.Count() < bb.Count()) {
      aa.Add(new SiteInfo(0, &XElementLib::GetByIndex(iQPeakIndex)));
    }
    while (bb.Count() < aa.Count()) {
      bb.Add(new SiteInfo(0, &XElementLib::GetByIndex(iQPeakIndex)));
    }
    BubbleSorter::SortSF(aa, &RSA_CompareSites);
    for (size_t ai = 0; ai < aa.Count(); ai++) {
      a.Add(aa[ai]);
    }
    aa.ReleaseAll();
    BubbleSorter::SortSF(bb, &RSA_CompareSites);
    for (size_t ai = 0; ai < bb.Count(); ai++) {
      b.Add(bb[ai]);
    }
    bb.ReleaseAll();
  }
  if (!a.IsEmpty()) {
    a.DeleteRange(0, sz);
  }
  if (!b.IsEmpty()) {
    b.DeleteRange(0, sz);
  }
  return RSA_GetAtomPriorityX(a, b);
}
struct RSA_EnviSorter {
  TCAtom &center;
  RSA_EnviSorter(TCAtom &center)
    : center(center)
  {}

  int Comparator(const TCAtom::Site &a, const TCAtom ::Site &b) const {
    a.atom->GetParent()->GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    center.SetTag(1);
    AtomEnvList ea, eb;
    ea.Add(new SiteInfo(&a, &a.atom->GetType()));
    eb.Add(new SiteInfo(&b, &b.atom->GetType()));
    return RSA_GetAtomPriorityX(ea, eb);
  }
};
void XLibMacros::macRSA(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp &app = TXApp::GetInstance();
  const TAsymmUnit &au = app.XFile().GetAsymmUnit();
  for (size_t i=0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    if (a.IsDeleted() || a.GetType() < 2) {
      continue;
    }
    TPtrList<TCAtom::Site> attached;
    for (size_t j=0; j < a.AttachedSiteCount(); j++) {
      TCAtom &aa = a.GetAttachedAtom(j);
      if (aa.IsDeleted() || aa.GetType() == iQPeakZ) {
        continue;
      }
      attached.Add(a.GetAttachedSite(j));
    }
    if (attached.Count() == 4) {
      RSA_EnviSorter es(a);
      BubbleSorter::SortMF(attached, es, &RSA_EnviSorter::Comparator);
      bool chiral=true;
      olxstr w;
      for (size_t j=0; j < attached.Count(); j++) {
        w << attached[j]->atom->GetLabel();
        if ((j + 1) < 4) {
          w << " < ";
        }
        if (j == 0) {
          continue;
        }
        if(es.Comparator(*attached[j-1], *attached[j]) == 0) {
          chiral = false;
          break;
        }
      }
      if (!chiral) {
        continue;
      }
      vec3d_alist crds(4);
      for (int j=0; j < 4; j++) {
        crds[j] = au.Orthogonalise(
          attached[j]->matrix*attached[j]->atom->ccrd());
      }
      vec3d cnt = (crds[1]+crds[2]+crds[3])/3;
      vec3d n = (crds[1]-crds[2]).XProdVec(crds[3]-crds[2]).Normalise();
      if ((crds[0] - cnt).DotProd(n) < 0) {
        n *= -1;
      }
      vec3d np = (crds[1]-cnt).XProdVec(n);
      olxstr lbl = a.GetLabel();
      lbl.RightPadding(5, ' ') << ':';
      if ((crds[3] - cnt).DotProd(np) < 0) { //clockwise
        lbl << " R";
      }
      else {
        lbl << " S";
      }
      TBasicApp::NewLogEntry() << lbl << " (" << w << ')';
    }
  }
}
//..............................................................................
ConstTypeList<TGroupCAtom> CONF_GetSites(const TLattice &latt,
  const TCAtom &cf, const smatd &fm,
  const TCAtom &ce, const smatd &em,
  olx_pdict<uint32_t, smatd*> &matrices,
  short min_z)
{
  const TUnitCell &uc = latt.GetUnitCell();
  TCAtomGroup rv;
  for (size_t i=0; i < cf.AttachedSiteCount(); i++) {
    TCAtom::Site &s = cf.GetAttachedSite(i);
    if (s.atom->GetType() < min_z) continue;
    const smatd m = fm.IsFirst() ? s.matrix : uc.MulMatrix(s.matrix, fm);
    bool skip = false;
    if (*s.atom == ce) {
      if (m.GetId() == em.GetId()) {
        skip = true;
      }
      if (!skip) {
        for (size_t j=0; j < s.atom->EquivCount(); j++) {
          uint32_t id = uc.MulMatrixId(s.atom->GetEquiv(j), m);
          if (id == em.GetId()) {
            skip = true;
            break;
          }
        }
      }
    }
    if (skip) continue;
    smatd *mp = 0;
    if (!m.IsFirst()) {
      mp = matrices.Find(m.GetId(), 0);
      if (mp == 0) {
        mp = matrices.Add(m.GetId(), new smatd(m));
      }
    }
    rv.Add(new TGroupCAtom(s.atom, mp));
  }
  return rv;
}
void CONF_Process(TCAtom &a, const smatd &am, TCAtom &b, const smatd &bm,
  olx_pdict<uint32_t, smatd*> &matrices, short min_z, const TDoubleList &args)
{
  if (a.GetType() < min_z || b.GetType() < min_z) {
    return;
  }
  TAsymmUnit &au = *a.GetParent();
  TLattice &latt = au.GetLattice();
  RefinementModel &rm = *au.GetRefMod();
  TCAtomGroup left = CONF_GetSites(latt, a, am, b, bm, matrices, min_z);
  TCAtomGroup right = CONF_GetSites(latt, b, bm, a, am, matrices, min_z);
  const vec3d ac = am*a.ccrd(), bc = bm*b.ccrd();
  for (size_t i=0; i < left.Count(); i++) {
    const vec3d lc = (left[i].GetMatrix() == 0 ? left[i].GetAtom()->ccrd() :
      (*left[i].GetMatrix())*left[i].GetAtom()->ccrd());
    for (size_t j=0; j < right.Count(); j++) {
      const vec3d rc = (right[j].GetMatrix() == 0 ? right[j].GetAtom()->ccrd() :
        (*right[j].GetMatrix())*right[j].GetAtom()->ccrd());
      // analyse if any of the three atoms are on the line
      try {
        double ca = au.Orthogonalise(lc-ac).CAngle(au.Orthogonalise(bc-ac));
        if (olx_abs(ca + 1) < 1e-6) {
          continue;
        }
        ca = au.Orthogonalise(rc-bc).CAngle(au.Orthogonalise(ac-bc));
        if (olx_abs(ca + 1) < 1e-6) {
          continue;
        }
      }
      catch(...) {
        continue;
      }
      InfoTab &it = rm.AddCONF();
      it.setArgs(args);
      bool reverse;
      if (left[i].GetAtom()->GetType() == right[j].GetAtom()->GetType()) {
        if (a.GetType() == b.GetType()) {
          reverse = left[i].GetAtom()->GetId() > right[j].GetAtom()->GetId();
        }
        else {
          reverse = a.GetType() > b.GetType();
        }
      }
      else {
        reverse = left[i].GetAtom()->GetType() > right[j].GetAtom()->GetType();
      }
      TCAtomGroup atoms;
      atoms.AddCopy(left[i]);
      atoms.Add(new TGroupCAtom(&a, am.IsFirst() ? 0 : &am));
      atoms.Add(new TGroupCAtom(&b, bm.IsFirst() ? 0 : &bm));
      atoms.AddCopy(right[j]);
      for (size_t k=0; k < atoms.Count(); k++) {
        size_t idx = (reverse ? (atoms.Count()-k-1) : k);
        it.AddAtom(*atoms[idx].GetAtom(), atoms[idx].GetMatrix());
      }
      rm.ValidateInfoTab(it);
    }
  }
}
void XLibMacros::macCONF(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  bool all = Options.GetBoolOption('a');
  TDoubleList args;
  for (size_t i = 0; i < Cmds.Count(); i++) {
    if (Cmds[i].IsNumber()) {
      args.Add(Cmds[i].ToDouble());
      Cmds.Delete(i--);
    }
  }
  MacroInput ma = ExtractSelection(Cmds, true);
  TXApp &app = TXApp::GetInstance();
  RefinementModel &rm = app.XFile().GetRM();
  const TLattice &latt = app.XFile().GetLattice();
  sorted::PointerPointer<TSAtom> atoms;
  olx_pdict<uint32_t, smatd*> matrices;
  short min_z = 2;
  if (ma.atoms.IsEmpty()) {
    if (all) {
      for (size_t i=0; i < ma.bonds.Count(); i++) {
        TSBond &b = *ma.bonds[i];
        atoms.AddUnique(&b.A());
        atoms.AddUnique(&b.B());
      }
    }
    else {
      for (size_t i=0; i < ma.bonds.Count(); i++) {
        TSBond &b = *ma.bonds[i];
        CONF_Process(b.A().CAtom(), b.A().GetMatrix(),
          b.B().CAtom(), b.B().GetMatrix(), matrices, min_z, args);
      }
    }
  }
  if (all) {
    for (size_t i=0; i < ma.atoms.Count(); i++) {
      atoms.AddUnique(ma.atoms[i]);
    }
    for (size_t i=0; i < atoms.Count(); i++) {
      TSAtom &a = *atoms[i];
      for (size_t j=0; j < a.CAtom().AttachedSiteCount(); j++) {
        TCAtom::Site &s = a.CAtom().GetAttachedSite(j);
        CONF_Process(a.CAtom(), a.GetMatrix(), *s.atom, s.matrix, matrices,
          min_z, args);
      }
    }
  }
  else if (!ma.atoms.IsEmpty()) {
    if (ma.atoms.Count() > 3) {
      InfoTab &it = rm.AddCONF();
      it.setArgs(args);
      for (size_t i = 0; i < ma.atoms.Count(); i++) {
        it.AddAtom(ma.atoms[i]->CAtom(),
          ma.atoms[i]->GetMatrix().IsFirst() ? NULL : &ma.atoms[i]->GetMatrix());
      }
    }
    else {
      Error.ProcessingError(__OlxSrcInfo, "at least 4 atoms are expected");
    }
  }
  for (size_t i = 0; i < matrices.Count(); i++) {
    delete matrices.GetValue(i);
  }
}
//..............................................................................
void XLibMacros::macD2CG(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  MacroInput ma = ExtractSelection(Cmds, true);
  if (!(ma.atoms.Count() > 1 ||
      (ma.atoms.Count() == 1 && ma.planes.Count() == 1)))
  {
    Error.ProcessingError(__OlxSrcInfo, "more than 1 atoms or a single atom "
      "and a plane are expected");
    return;
  }
  TXApp &app = TXApp::GetInstance();
  VcoVContainer vcovc(app.XFile().GetAsymmUnit());
  try  {
    olxstr src_mat = app.InitVcoV(vcovc);
    app.NewLogEntry() << "Using " << src_mat << " matrix for the calculation";
  }
  catch (TExceptionBase& e)  {
    Error.ProcessingError(__OlxSrcInfo, e.GetException()->GetError());
    return;
  }
  TSAtom &a = *ma.atoms[0];
  TSAtomCPList atoms;
  if (ma.atoms.Count() > 1) {
    atoms = ma.atoms.GetObject().SubListFrom(1);
  }
  else {
    atoms.SetCapacity(ma.planes[0]->Count());
    for (size_t i = 0; i < ma.planes[0]->Count(); i++) {
      atoms << ma.planes[0]->GetAtom(i);
    }
  }
  TStrList out;
  out.Add(a.GetLabel()) << " to centroid of " <<
    olx_analysis::alg::label(TCAtomPList(atoms,
    FunctionAccessor::MakeConst(&TSAtom::CAtom)), '-') << " distance: " <<
    vcovc.CalcPC2ADistance(atoms, a).ToString();
  if (atoms.Count() > 2) {
    out.Add(a.GetLabel()) << " to plane formed by " <<
      olx_analysis::alg::label(TCAtomPList(atoms,
      FunctionAccessor::MakeConst(&TSAtom::CAtom)), '-') <<
      " projection length: " << vcovc.CalcP2ADistance(atoms, a).ToString();
  }
  if (Options.GetBoolOption('c')) {
    app.ToClipboard(out);
  }
  TBasicApp::NewLogEntry() << out;
}
//.............................................................................
void XLibMacros::macCalcVars(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  xapp.XFile().GetRM().CVars.CalcAll();
}
//.............................................................................
void XLibMacros::funHKLF(const TStrObjList &args, TMacroData &E) {
  TXFile &xf = TXApp::GetInstance().XFile();
  if (args.IsEmpty()) {
    E.SetRetVal(xf.GetRM().GetHKLF());
  }
  else if (args.Count() == 1 && args[0].IsUInt()) {
    uint32_t bc = args[0].ToUInt();
    if (bc == 0) {
      xf.GetRM().Vars.ClearBASF();
      xf.GetRM().SetHKLF(4);
    }
    else {
      xf.GetRM().Vars.ClearBASF();
      xf.GetRM().SetHKLF(5);
      bc--;
      TStrList sl;
      olxstr v = 1. / (bc+1);
      for (size_t i = 0; i < bc; i++) {
        sl << v;
      }
      xf.GetRM().Vars.SetBASF(sl);
    }
  }
  else if (args.Count() >= 1 && args[0].Equalsi("remove")) {
    mat3d hklfm = xf.GetRM().GetHKLF_mat();
    if (hklfm.IsI()) {
      return;
    }
    olxstr hklfn = xf.GetRM().GetHKLSource();
    if (args.Count() >= 2) {
      if (!TEFile::IsAbsolutePath(args[1])) {
        hklfn = TEFile::ExtractFilePath(hklfn) + args[1];
      }
      else {
        hklfn = args[1];
      }
    }
    THklFile::SaveToFile(hklfn, xf.GetRM().GetReflections());
    xf.GetRM().SetHKLSource(hklfn);
    hklfm.I();
    xf.GetRM().SetHKLF_mat(hklfm);
  }
  else {
    xf.GetRM().Vars.ClearBASF();
    xf.GetRM().SetHKLF(5);
    xf.GetRM().Vars.SetBASF(args);
  }
}
//..............................................................................
void XLibMacros::macPack(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp &app = TXApp::GetInstance();
  const bool ClearCont = !Options.Contains("c");
  const bool cell = (Cmds.Count() > 0 && Cmds[0].Equalsi("cell"));
  if (cell) {
    Cmds.Delete(0);
  }
  TStopWatch sw(__FUNC__);
  if (cell) {
    app.XFile().GetLattice().GenerateCell();
  }
  else {
    vec3d From(-0.5), To(1.5);
    size_t number_count = 0;
    for (size_t i = 0; i < Cmds.Count(); i++)  {
      if (Cmds[i].IsNumber())  {
        if (!(number_count % 2)) {
          From[number_count / 2] = Cmds[i].ToDouble();
        }
        else {
          To[number_count / 2] = Cmds[i].ToDouble();
        }
        number_count++;
        Cmds.Delete(i--);
      }
    }

    if (number_count != 0 && !(number_count == 6 || number_count == 1 ||
      number_count == 2))
    {
      Error.ProcessingError(__OlxSrcInfo, "please provide 6, 2 or 1 number");
      return;
    }

    TCAtomPList TemplAtoms;
    if (!Cmds.IsEmpty()) {
      TSAtomPList atoms = app.FindSAtoms(Cmds);
      ACollectionItem::Unify(atoms,
        FunctionAccessor::MakeConst(&TSAtom::CAtom));
      TemplAtoms.AddAll(atoms, FunctionAccessor::MakeConst(&TSAtom::CAtom));
    }

    if (number_count == 6 || number_count == 0 || number_count == 2)  {
      if (number_count == 2)  {
        From[1] = From[2] = From[0];
        To[1] = To[2] = To[0];
      }
      app.XFile().GetLattice().Generate(From, To,
        &TemplAtoms, ClearCont,
        Options.GetBoolOption('a'));
    }
    else {
      TSAtomPList xatoms = app.FindSAtoms(Cmds, true, true);
      vec3d cent;
      double wght = 0;
      for (size_t i = 0; i < xatoms.Count(); i++)  {
        cent += xatoms[i]->crd()*xatoms[i]->CAtom().GetChemOccu();
        wght += xatoms[i]->CAtom().GetChemOccu();
      }
      if (wght != 0) {
        cent /= wght;
      }
      app.XFile().GetLattice().Generate(cent, From[0],
        &TemplAtoms, ClearCont,
        Options.GetBoolOption('a'));
    }
  }
}
//.............................................................................
void XLibMacros::macGrow(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp &app = TXApp::GetInstance();
  TLattice& latt = app.XFile().GetLattice();
  bool GrowShells = Options.Contains('s'),
    GrowContent = Options.Contains('w');
  TCAtomPList TemplAtoms;
  if (Options.Contains('t')) {
    TSAtomPList atoms = app.FindSAtoms(TStrList(olxstr(Options['t']), ','));
    ACollectionItem::Unify(atoms,
      FunctionAccessor::MakeConst(&TSAtom::CAtom));
    TemplAtoms.AddAll(atoms, FunctionAccessor::MakeConst(&TSAtom::CAtom));
  }
  if (Cmds.IsEmpty()) {  // grow fragments
    if (GrowContent) {
      latt.GenerateWholeContent(TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
    }
    else  {
      TSAtomPList atoms;
      app.XFile().GetLattice().GrowFragments(
        GrowShells, TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
      if (!GrowShells)  {
        smatd_list gm;
        /* check if next grow will not introduce simple translations */
        bool grow_next = true;
        while (grow_next)  {
          gm.Clear();
          latt.GetGrowMatrices(gm);
          if (gm.IsEmpty())  break;
          for (size_t i = 0; i < latt.MatrixCount(); i++)  {
            for (size_t j = 0; j < gm.Count(); j++)  {
              if (latt.GetMatrix(i).r == gm[j].r)  {
                const vec3d df = latt.GetMatrix(i).t - gm[j].t;
                if ((df - df.Round<int>()).QLength() < 1e-6)  {
                  grow_next = false;
                  break;
                }
              }
            }
            if (!grow_next)  break;
          }
          if (grow_next) {
            latt.GrowFragments(GrowShells,
              TemplAtoms.IsEmpty() ? 0 : &TemplAtoms);
          }
        }
      }
    }
  }
  else  {  // grow atoms
    if (GrowContent)
      latt.GenerateWholeContent(TemplAtoms.IsEmpty() ? 0 : &TemplAtoms);
    else {
      latt.GrowAtoms(app.FindSAtoms(Cmds), GrowShells,
        TemplAtoms.IsEmpty() ? 0 : &TemplAtoms);
    }

  }
}
//.............................................................................
void XLibMacros::funStrDir(const TStrObjList& Params, TMacroData &E) {
  olxstr f = TXApp::GetInstance().XFile().GetStructureDataFolder();
  E.SetRetVal(f.IsEmpty() ? EmptyString() : f.SubStringFrom(0, 1));
}
//..............................................................................
void XLibMacros::funHAddCount(const TStrObjList& Params, TMacroData &E) {
  TXApp &app = TXApp::GetInstance();
  TLattice &latt = app.XFile().GetLattice();
  TSAtomPList satoms = app.FindSAtoms(Params, true);
  latt.UpdateConnectivity();
  RefinementModel &rm = app.XFile().GetRM();
  TXlConGen xlConGen(rm);
  TUnitCell &uc = app.XFile().GetUnitCell();
  E.SetRetVal(app.XFile().GetLattice().AnalyseHAdd(xlConGen, satoms, true));
}
//..............................................................................
void XLibMacros::macConvert(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp &app = TXApp::GetInstance();
  TBasicCFile *inp_ =
    app.XFile().FindFormat(TEFile::ExtractFileExt(Cmds[0]));
  TBasicCFile *otp_ =
    app.XFile().FindFormat(TEFile::ExtractFileExt(Cmds[1]));
  if (inp_ == 0 || otp_ == 0) {
    Error.ProcessingError(__OlxSrcInfo, "unknown file(s) type");
    return;
  }
  TXFile xf(*(new SObjectProvider));
  xf.RegisterFileFormat(dynamic_cast<TBasicCFile *>(inp_->Replicate()),
    TEFile::ExtractFileExt(Cmds[0]));
  xf.RegisterFileFormat(dynamic_cast<TBasicCFile *>(otp_->Replicate()),
    TEFile::ExtractFileExt(Cmds[1]));
  xf.LoadFromFile(Cmds[0]);
  xf.SaveToFile(Cmds[1]);
}
//..............................................................................
void XLibMacros::macSetCharge(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  uint8_t ch = Cmds[0].ToInt();
  TXApp &app = TXApp::GetInstance();
  TSAtomPList atoms = app.FindSAtoms(Cmds.SubListFrom(1), true);
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i]->IsAUAtom()) {
      atoms[i]->CAtom().SetCharge(ch);
    }
  }
}
//..............................................................................
vec3d Orthogonalise(const vec3d &ax, const vec3d &an) {
  TAsymmUnit au(0);
  au.GetAxes() = ax;
  au.GetAngles() = an;
  au.InitMatrices();
  mat3d S = au.GetCellToCartesian()*mat3d::Transpose(au.GetCellToCartesian()),
    S_sq = S, I;
  mat3d::EigenValues(S_sq, I.I());
  S_sq[0][0] = 1. / sqrt(S_sq[0][0]);
  S_sq[1][1] = 1. / sqrt(S_sq[1][1]);
  S_sq[2][2] = 1. / sqrt(S_sq[2][2]);
  S_sq = mat3d::Transpose(I)*S_sq*I;
  return vec3d(
    S_sq[0].DotProd(S[0]), S_sq[1].DotProd(S[1]), S_sq[2].DotProd(S[2]));
}
//..............................................................................
//Acta Cryst. (1999). B55, 1099-1108
void XLibMacros::macLowdin(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  mat3d S, S_sq, I;
  if (Cmds.Count() == 6) {
    TAsymmUnit au(0);
    au.GetAxes() = vec3d(
      Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble());
    au.GetAngles() = vec3d(
      Cmds[3].ToDouble(), Cmds[4].ToDouble(), Cmds[5].ToDouble());
    au.InitMatrices();
    S = au.GetCellToCartesian();
  }
  else if (Cmds.Count() == 1) {
    TStrList lines = TEFile::ReadLines(Cmds[0]);
    for (size_t i = 0; i < lines.Count(); i++) {
      TStrList toks(lines[i], ' ');
      if (toks.Count() < 7) {
        TBasicApp::NewLogEntry(logError) << "Skipping line #" << (i + 1);
      }
      else {
        try {
          vec3d x = Orthogonalise(
            vec3d(toks[1].ToDouble(), toks[2].ToDouble(), toks[3].ToDouble()),
            vec3d(toks[4].ToDouble(), toks[5].ToDouble(), toks[6].ToDouble()));
          TBasicApp::NewLogEntry() << toks[0]
            << ": " << olxstr::FormatFloat(4, x[0])
            << " " << olxstr::FormatFloat(4, x[1])
            << " " << olxstr::FormatFloat(4, x[2]);
        }
        catch (const TExceptionBase &e) {
          TBasicApp::NewLogEntry(logError) << "Failed at line #" << (i+1);
        }
      }
    }
    return;
  }
  else {
    if (!TXApp::GetInstance().XFile().HasLastLoader()) {
      Error.ProcessingError(__OlxSrcInfo, "Loaded file is expected");
      return;
    }
    S = TXApp::GetInstance().XFile().GetAsymmUnit().GetCellToCartesian();
  }
  S_sq = S = S*mat3d::Transpose(S);
  mat3d::EigenValues(S_sq, I.I());
  S_sq[0][0] = 1. / sqrt(S_sq[0][0]);
  S_sq[1][1] = 1. / sqrt(S_sq[1][1]);
  S_sq[2][2] = 1. / sqrt(S_sq[2][2]);
  S_sq = mat3d::Transpose(I)*S_sq*I;
  TBasicApp::NewLogEntry() << "Orthogonalised cell axis lengths:";
  TBasicApp::NewLogEntry() << "(" << olxstr::FormatFloat(3, S_sq[0].DotProd(S[0]))
    << ", " << olxstr::FormatFloat(3, S_sq[1].DotProd(S[1]))
    << ", " << olxstr::FormatFloat(3, S_sq[2].DotProd(S[2])) << ')';
}
//..............................................................................
struct macHKLF5_sorter {
  const THklFile &hf;
  macHKLF5_sorter(const THklFile &hf)
    : hf(hf)
  {}

  template <typename pair_t>
  int Compare(const pair_t &a_, const pair_t &b_) const {
    const TReflection &ra = hf[olx_ref::get(a_).b];
    const TReflection &rb = hf[olx_ref::get(b_).b];
    return -olx_cmp(ra.GetHkl().QLength(), rb.GetHkl().QLength());
  }
};
void XLibMacros::macHKLF5(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (Cmds.IsEmpty()) {
    olxstr fof = "FileOpen('Choose an HKLF5 file', '*.hkl', FilePath())";
    if (!olex2::IOlex2Processor::GetInstance()->processFunction(fof)) {
      return;
    }
    Cmds.Add(fof);
  }
  THklFile hf;
  hf.LoadFromFile(Cmds[0], false);
  if (hf.RefCount() == 0 || !hf[0].IsBatchSet()) {
    Error.ProcessingError(__OlxSrcInfo, "HKLF5 file is expected");
    return;
  }
  typedef olx_pair_t<size_t, size_t> pair_t;
  olx_pdict<int16_t, TTypeList<pair_t> > batches;
  
  for (size_t i = 0; i < hf.RefCount(); i++) {
    if (hf[i].GetBatch() < 0) {
      size_t st = i;
      while (++i < hf.RefCount() && hf[i].GetBatch() < 0)
      {
      }
      if (i >= hf.RefCount()) {
        break;
      }
      if (hf[i].GetHkl().Prod() == 0) {
        continue;
      }
      for (size_t ri = st; ri < i; ri++) {
        TTypeList<pair_t> &data = batches.Add(hf[ri].GetBatch());
        data.Add(new pair_t(ri, i));
      }
    }
  }
  if (batches.IsEmpty()) {
    Error.ProcessingError(__OlxSrcInfo,
      "could not locate any negative batch numbers");
    return;
  }
  const TAsymmUnit &au = TXApp::GetInstance().XFile().GetAsymmUnit();
  mat3d hm = au.GetHklToCartesian();
  for (size_t bi = 0; bi < batches.Count(); bi++) {
    TTypeList<pair_t> &data = batches.GetValue(bi);
    if (data.Count() < 3) {
      TBasicApp::NewLogEntry(logWarning) << "Skipping batch "
        << batches.GetKey(bi) << " - not enough data";
      continue;
    }
    QuickSorter::Sort(data, macHKLF5_sorter(hf));
    size_t max_data = olx_min(10, data.Count());
    ematd dm(max_data * 3, 9);
    evecd right(max_data * 3);
    for (size_t hi = 0; hi < max_data; hi ++) {
      const TReflection &sr = hf[data[hi].b];
      const TReflection &dr = hf[data[hi].a];
      size_t off = hi * 3;
      double w = sr.GetHkl().QLength();
      for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 3; j++) {
          dm[off + i][i * 3 + j] = sr.GetHkl()[j] * w * w;
        }
        right[off + i] = dr.GetHkl()[i] * w * w;
      }
    }
    ematd dmt = ematd::Transpose(dm);
    ematd inm = dmt * dm;
    if (!math::LU::Invert(inm)) {
      TBasicApp::NewLogEntry(logWarning) << "Failed to invert the normal matrix";
      continue;
    }
    evecd b = dmt * right;
    b = inm * b;
    mat3d tm;
    TBasicApp::NewLogEntry() << "Twin rotation matrix for batch "
      << batches.GetKey(bi);
    for (size_t i = 0; i < 3; i++) {
      for (size_t j = 0; j < 3; j++) {
        tm[i][j] = b[i * 3 + j];
        if (olx_abs(tm[i][j]) < 1e-6) {
          tm[i][j] = 0;
        }
      }
      TBasicApp::NewLogEntry() << olx_print("%5.3lft %5.3lft %5.3lft",
        tm[i][0], tm[i][1], tm[i][2]);
    }
    // test
    double r_sq = 0;
    for (size_t hi = 0; hi < data.Count(); hi++) {
      const TReflection &sr = hf[data[hi].b];
      const TReflection &dr = hf[data[hi].a];
      vec3d res = tm * vec3d(sr.GetHkl());
      r_sq += TReflection::ToCart(res - dr.GetHkl(), hm).QLength();
    }
    TBasicApp::NewLogEntry() << olx_print("R fit: %.2le for %z data",
      sqrt(r_sq)/ data.Count(), data.Count());
  }

}
//..............................................................................
