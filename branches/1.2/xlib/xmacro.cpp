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

#ifdef _SVN_REVISION_AVAILABLE
#  include "../svn_revision.h"
#endif

#define xlib_InitMacro(macroName, validOptions, argc, desc)\
  lib.RegisterStaticMacro( new TStaticMacro(&XLibMacros::mac##macroName, #macroName, (validOptions), argc, desc))
#define xlib_InitMacroA(macroName, amacroName, validOptions, argc, desc)\
  lib.RegisterStaticMacro( new TStaticMacro(&XLibMacros::mac##macroName, #amacroName, (validOptions), argc, desc))
#define xlib_InitFunc(funcName, argc, desc) \
  lib.RegisterStaticFunction( new TStaticFunction(&XLibMacros::fun##funcName, #funcName, argc, desc))

using namespace cif_dp;
const olxstr XLibMacros::NoneString("none");
const olxstr XLibMacros::NAString("n/a");
olxstr XLibMacros::CurrentDir;
TActionQList XLibMacros::Actions;
TActionQueue& XLibMacros::OnDelIns = XLibMacros::Actions.New("OnDelIns");
TActionQueue& XLibMacros::OnAddIns = XLibMacros::Actions.New("OnAddIns");

void XLibMacros::Export(TLibrary& lib)  {
  xlib_InitMacro(Run, EmptyString(), fpAny^fpNone, "Runs provided macros (combined by '>>')");
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
  xlib_InitMacro(SGE, EmptyString(), fpNone|fpOne|psFileLoaded,
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
    "q-adds Q-peaks to the list&;h-adds hydrogen atoms to the list&;cs-leaves"
    " selection unchanged",
    fpNone|fpOne|fpTwo,
    "This macro prints environment of any particular atom. Default search "
    "radius is 2.7A.");
//_____________________________________________________________________________
  xlib_InitMacro(AddSE, EmptyString(), (fpAny^fpNone)|psFileLoaded,
    "Tries to add a new symmetry element to current space group to form a new "
    "one. [-1] is for center of symmetry");
//_____________________________________________________________________________
  xlib_InitMacro(Fuse, "f-removes symmetrical equivalents",
    fpNone|fpOne|psFileLoaded,
    "Re-initialises the connectivity list. If a number is provided, atoms of "
    "the same type connected by bonds shorter than the provided number are "
    "merged into one atom with center at the centroid formed by all removed "
    "atoms");
  xlib_InitMacro(Flush, EmptyString(), fpNone|fpOne, "Flushes log streams");
//_____________________________________________________________________________
  xlib_InitMacro(EXYZ,
    "eadp-sets the equivalent anisotropic parameter constraint for the shared "
    "site",
    fpAny|psCheckFileTypeIns,
    "Adds a new element to the given/selected site. Takes one selected atom "
    "and element types as any subsequent argument. Alternatively can take a "
    "few selected aoms of different type to be modelled as the type swapping "
    "disorder."
);
//_____________________________________________________________________________
  xlib_InitMacro(EADP, EmptyString(), fpAny|psCheckFileTypeIns,
"Forces EADP/Uiso of provided atoms to be constrained the same");
//_____________________________________________________________________________
  xlib_InitMacro(Cif2Doc, "n-output file name", fpNone|fpOne|psFileLoaded,
    "converts cif to a document");
  xlib_InitMacro(Cif2Tab, "n-output file name&;t-table definition file",
    fpAny|psFileLoaded,
    "creates a table from a cif");
  xlib_InitMacro(CifMerge,
    "u-updates atom treatment if the asymmetric units of currently loaded file"
    " and of the CIF file match",
    fpAny|psFileLoaded,
  "Merges loaded or provided as first argument cif with other cif(s)");
  xlib_InitMacro(CifExtract, EmptyString(), fpTwo|psFileLoaded,
    "extract a list of items from one cif to another");
  xlib_InitMacro(CifCreate, EmptyString(), fpNone|psFileLoaded,
    "Creates cif from current file, variance-covariance matrix should be "
    "available");
  xlib_InitMacro(FcfCreate, "scale-[external],simple or regression",
    (fpAny^fpNone)|psFileLoaded,
    "Creates fcf from current file. Expects a number as in the shelx list "
    "number as the first argument, the second argument is the output file name"
    " filename().fcf is default"
    );
//_____________________________________________________________________________
  xlib_InitMacro(VoidE, EmptyString(), fpNone|psFileLoaded,
    "calculates number of electrons in the voids area");
//_____________________________________________________________________________
  xlib_InitMacro(ChangeSG, EmptyString(), (fpAny^fpNone)|psFileLoaded,
    "[shift] SG. Changes space group of current structure, applying given shit"
    " prior (if provided) to the change of symmetry of the unit cell");
//_____________________________________________________________________________
  xlib_InitMacro(Htab,
    "t-adds extra elements (comma separated -t=Br,I) to the donor list. "
    "Defaults are [N,O,F,Cl,S]&;g-generates found interactions",
    fpNone|fpOne|fpTwo|psCheckFileTypeIns, 
    "Adds HTAB instructions to the ins file, maximum bond length [2.9] and "
    "minimal angle [150] might be provided");
//_____________________________________________________________________________
  xlib_InitMacro(HAdd, EmptyString(), fpAny|psCheckFileTypeIns,
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
    "f-generates full SFAC instructions&;n-for neutron data",
    fpNone|fpOne|psFileLoaded,
    "Generates anisotropic dispertion parameters for current radiation "
    "wavelength");
//_____________________________________________________________________________
  xlib_InitMacro(AddIns,EmptyString(), (fpAny^fpNone)|psCheckFileTypeIns,
    "Adds an instruction to the INS file");
  xlib_InitMacro(DelIns, EmptyString(), fpOne|psCheckFileTypeIns,
    "A number or the name (will remove all accurances) can be provided");
  xlib_InitMacro(LstIns, EmptyString(), fpNone|psCheckFileTypeIns,
    "Lists all instructions of currently loaded Ins file");
  xlib_InitMacro(FixHL, EmptyString(), fpNone|psFileLoaded,
    "Fixes hydrogen atom labels");
  xlib_InitMacro(Fix, EmptyString(), (fpAny^fpNone)|psCheckFileTypeIns,
    "Fixes specified parameters of atoms: XYZ, Uiso, Occu");
  xlib_InitMacro(Free, EmptyString(), (fpAny^fpNone)|psCheckFileTypeIns,
    "Frees specified parameters of atoms: XYZ, Uiso, Occu");
  xlib_InitMacro(Isot, "npd-makes all NPD atoms isotropic",
    fpAny|psFileLoaded,
    "Makes provided atoms isotropic, if no arguments provided, current "
    "selection or all atoms become isotropic");
  xlib_InitMacro(Anis,"h-adds hydrogen atoms" , (fpAny) | psFileLoaded, 
    "Makes provided atoms anisotropic if no arguments provided current "
    "selection or all atoms are considered");
  xlib_InitMacro(File, "s-sort the main residue of the asymmetric unit",
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
  xlib_InitMacro(Sort, EmptyString(), fpAny^psFileLoaded,
  "Sorts atoms of the default residue. Atom sort arguments: "
  "\n\tm - atomic weight"
  "\n\tl - label, considering numbers"
  "\n\tp - part, 0 is first followed by all positive parts in ascending order "
  "and then negative ones"
  "\n\th - to treat hydrogen atoms independent of the pivot atom"
  "\n\ts - non-numerical labels suffix"
  "\n\tz - number after the atom symbol"
  "\n Moiety sort arguments:"
  "\n\ts - size"
  "\n\th - by heaviest atom"
  "\n\tm - molecular weight"
  "\nUsage: sort [+atom_sort_type] or [Atoms] [moiety [+moety sort type] "
  "[moiety atoms]]. If just 'moiety' is provided - the atoms will be split "
  "into the moieties without sorting."
  "\nExample: sort +ml F2 F1 moiety +s - will sort atoms by atomic weight and "
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
  xlib_InitMacro(Omit, EmptyString(), fpOne|fpTwo|fpThree|psCheckFileTypeIns, 
    "Removes any particular reflection from the refinement list. If a single "
    "number is provided, all reflections with delta(F^2)/esd greater than "
    "given number are omitted");
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
    "Analysis of the pi-pi interactions (experimental). The procedure searches"
    " for flat reqular C6 or NC5 rings and prints information for the ones "
    "where the centroid-centroid distance is smaller than [4] A and the shift "
    "is smaller than [3] A. These two parameters can be customised.");
  xlib_InitMacro(MolInfo, "g-generation of the triangluation [5]&;s-source "
    "([o]ctahedron, (t)etrahedron) &;o-use occupancy of the atoms in the "
    "integration",
    fpAny|psFileLoaded,
    "Prints molecular volume, surface area and other information for "
    "visible/selected atoms");
  xlib_InitMacro(RTab, EmptyString(), (fpAny^fpNone)|psCheckFileTypeIns,
    "Adds RTAB with givn name for provided atoms/selection");
  xlib_InitMacro(HklMerge, "z-zero negative intensity", fpAny|psFileLoaded,
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
  xlib_InitMacroA(Update, @update, EmptyString(), fpAny,
    "Reads given file and if the atoms list of loaded file matches the atom "
    "list of the given file the atomic coordinates, FVAR and BASF values are "
    "updated.");
//_____________________________________________________________________________
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
    "Checks type of currently loaded file [ins,res,ires,cif,mol,xyz]");
//_____________________________________________________________________________
  xlib_InitFunc(BaseDir, fpNone|fpOne, "Returns the startup folder");
  xlib_InitFunc(HKLSrc, fpNone|fpOne|psFileLoaded,
    "Returns/sets hkl source for currently loaded file");
//_____________________________________________________________________________
  xlib_InitFunc(LSM, fpNone|psCheckFileTypeIns,
    "Return current refinement method, L.S. or CGLS currently.");
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
}
//..............................................................................
void XLibMacros::macTransform(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  smatd tm;
  if( !Parse(Cmds, "mivd", &tm.r, &tm.t) )  {
    Error.ProcessingError(__OlxSrcInfo, "invalid transformation matrix");
    return;
  }
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms;
  xapp.FindSAtoms(Cmds.Text(' '), atoms, true);
  xapp.XFile().GetLattice().TransformFragments(atoms, tm);
}
//..............................................................................
void XLibMacros::macPush(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  vec3d pnt;
  if( !Parse(Cmds, "vd", &pnt) )  {
    Error.ProcessingError(__OlxSrcInfo, "invalid translation");
    return;
  }
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms;
  xapp.FindSAtoms(Cmds.Text(' '), atoms, true);
  smatd tm;
  tm.I();
  tm.t = pnt;
  xapp.XFile().GetLattice().TransformFragments(atoms, tm);
}
//..............................................................................
void XLibMacros::macInv(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool Force = Options.Contains("f");  // forces inversion for sg without center of inversion
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    Error.ProcessingError(__OlxSrcInfo, "unknown file space group");
    return;
  }
  if( !sg->IsCentrosymmetric() &&  !Force )  {
    Error.ProcessingError(__OlxSrcInfo, "non-centrosymmetric space group, use -f to force");
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
  specials.Add("P4132", "P4321");
  // remap the reverse
  size_t s_c = specials.Count();
  for( size_t i=0; i < s_c; i++ )
    specials.Add(specials.GetValue(i), specials.GetKey(i));

  TSAtomPList atoms;
  xapp.FindSAtoms(Cmds.Text(' '), atoms, true);
  smatd tm;
  tm.I() *= -1;
  tm.t = sg->GetInversionCenter()*(-2);
  xapp.XFile().GetLattice().TransformFragments(atoms, tm);
  s_c = specials.IndexOf(sg->GetName());
  if( s_c != InvalidIndex )  {
    TBasicApp::NewLogEntry() << "Changing spacegroup from "
      << specials.GetKey(s_c) << " to " << specials.GetValue(s_c);
    sg = TSymmLib::GetInstance().FindGroupByName(specials.GetValue(s_c));
    if( sg == NULL )  {
      Error.ProcessingError(__OlxSrcInfo, "Failed to locate necessary space group");
      return;
    }
    xapp.XFile().GetAsymmUnit().ChangeSpaceGroup(*sg);
    xapp.XFile().LastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
    xapp.XFile().EndUpdate();
  }
}
//..............................................................................
void XLibMacros::macSAInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
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
    TPSTypeList<size_t, TSpaceGroup*> hits, bl_hits;
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
        if( &hits.GetObject(j)->GetBravaisLattice() == &sl.GetBravaisLattice(i) )
          bl_hits.Add(hits.GetKey(j), hits.GetObject(j));
      }
      if( bl_hits.IsEmpty() )  continue;
      TTTable<TStrList> tab( bl_hits.Count()/5+((bl_hits.Count()%5) != 0 ? 1 : 0), 5);
      olxstr tmp;
      for( size_t j=0; j < bl_hits.Count(); j++ )  {
        tmp = bl_hits.GetObject(j)->GetName();
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
        hits.GetObject(i)->SplitIntoElements(all_elm, sg_elm);
        bool exact = true;
        for( size_t j=0; j < sg_elm.Count(); j++ )  {
          if( ref.IndexOf(sg_elm[j]) == InvalidIndex )  {
            exact = false;
            break;
          }
        }
        if( exact )  exact_match << '[';
        exact_match << hits.GetObject(i)->GetName();
        if( exact )  exact_match << ']';
        sg_elm.Clear();
      }
    }
    output.Hyphenate(exact_match, 80);
    log.NewEntry() << output;
    log.NewEntry() << "Space groups inclosed in [] have exact match to the provided elements";
  }
}
//..............................................................................
void XLibMacros::macSGInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    TPtrList<TSpaceGroup> sgList;
    TSymmLib& symlib = TSymmLib::GetInstance();
    for( size_t i=0; i < symlib.BravaisLatticeCount(); i++ )  {
      TBravaisLattice& bl = symlib.GetBravaisLattice(i);
      bl.FindSpaceGroups(sgList);
      TBasicApp::NewLogEntry() << "------------------- " << bl.GetName() << " --- "  << sgList.Count();
      olxstr tmp, tmp1;
      TPSTypeList<int, TSpaceGroup*> SortedSG;
      for( size_t j=0; j < sgList.Count(); j++ )
        SortedSG.Add(sgList[j]->GetNumber(), sgList[j]);
      for( size_t j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetObject(j)->GetName() << "(#" << SortedSG.GetKey(j) << ')';
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
      E.ProcessingError(__OlxSrcInfo, "Could not find specified space group/Laue class/Point group: ") << Cmds[0];
      return;
    }
    LaueClassPG = true;
  }
  if( LaueClassPG )  {
    TPtrList<TSpaceGroup> sgList;
    TPSTypeList<int, TSpaceGroup*> SortedSG;
    if( &sg->GetLaueClass() == sg )  {
      TBasicApp::NewLogEntry() << "Space groups of the Laue class " << sg->GetBareName();
      TSymmLib::GetInstance().FindLaueClassGroups( *sg, sgList);
      for( size_t j=0; j < sgList.Count(); j++ )
        SortedSG.Add(sgList[j]->GetNumber(), sgList[j]);
      olxstr tmp, tmp1;
      for( size_t j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetObject(j)->GetName() << "(#" << SortedSG.GetKey(j) << ')';
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
      TBasicApp::NewLogEntry() << "Space groups of the point group " << sg->GetBareName();
      TSymmLib::GetInstance().FindPointGroupGroups(*sg, sgList);
      TPSTypeList<int, TSpaceGroup*> SortedSG;
      for( size_t j=0; j < sgList.Count(); j++ )
        SortedSG.Add(sgList[j]->GetNumber(), sgList[j]);
      for( size_t j=0; j < SortedSG.Count(); j++ )  {
        tmp1 << SortedSG.GetObject(j)->GetName() << "(#" << SortedSG.GetKey(j) << ')';
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

  TBasicApp::NewLogEntry() << (sg->IsCentrosymmetric() ? "Centrosymmetric" : "Non centrosymmetric");
  TBasicApp::NewLogEntry() << "Hall symbol: " << sg->GetHallSymbol();

  TSymmLib::GetInstance().GetGroupByNumber(sg->GetNumber(), AllGroups);
  if( AllGroups.Count() > 1 )  {
    TBasicApp::NewLogEntry() << "Alternative settings:";
    olxstr tmp;
    for( size_t i=0; i < AllGroups.Count(); i++ )  {
      if( AllGroups[i] == sg )  continue;
      tmp << AllGroups[i]->GetName() << '(' << AllGroups[i]->GetFullName() <<  ") ";
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
//..............................................................................
void XLibMacros::macSort(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp::GetInstance().XFile().Sort(TStrList(Cmds));
}
//..............................................................................
void XLibMacros::macRun(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "this function requires Olex2 processor implementation");
  TStrList allCmds(Cmds.Text(' '), ">>");
  for( size_t i=0; i < allCmds.Count(); i++ )  {
    op->executeMacroEx(allCmds[i], Error);
    if( !Error.IsSuccessful() )  {
      if( (i+1) < allCmds.Count() )
        op->print("Not all macros in the provided list were executed", olex::mtError);
      break;
    }
  }
}
//..............................................................................
void XLibMacros::macHklStat(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr hklSrc = xapp.LocateHklFile();
  if( !TEFile::Exists( hklSrc ) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not find hkl file: ") << hklSrc;
    return;
  }
  if( Cmds.IsEmpty() )  {
    RefinementModel::HklStat hs = xapp.XFile().GetRM().GetMergeStat();
    TTTable<TStrList> tab(21, 2);
    tab[0][0] << "Total reflections (after filtering)";   tab[0][1] << hs.TotalReflections;
    tab[1][0] << "Unique reflections";            tab[1][1] << hs.UniqueReflections;
    tab[2][0] << "Centric reflections";           tab[2][1] << hs.CentricReflections;
    tab[3][0] << "Friedel pairs merged";          tab[3][1] << hs.FriedelOppositesMerged;
    tab[4][0] << "Inconsistent equivalents";     tab[4][1] << hs.InconsistentEquivalents;
    tab[5][0] << "Systematic absences removed";   tab[5][1] << hs.SystematicAbsentcesRemoved;
    tab[6][0] << "Min d";                         tab[6][1] << olxstr::FormatFloat(3, hs.MinD);
    tab[7][0] << "Max d";                         tab[7][1] << olxstr::FormatFloat(3, hs.MaxD);
    tab[8][0] << "Limiting d min (SHEL)";         tab[8][1] << olxstr::FormatFloat(3, hs.LimDmin);
    tab[9][0] << "Limiting d max (SHEL/OMIT_2t)";    tab[9][1] << hs.LimDmax;
    tab[10][0] << "Filtered off reflections (SHEL/OMIT_s/OMIT_2t)";  tab[10][1] << hs.FilteredOff;
    tab[11][0] << "Reflections omitted by user (OMIT_hkl)";   tab[11][1] << hs.OmittedByUser;
    tab[12][0] << "Reflections skipped (after 0 0 0)";        tab[12][1] << hs.OmittedReflections;
    tab[13][0] << "Intensity transformed for (OMIT_s)";       tab[13][1] << hs.IntensityTransformed << " reflections";
    tab[14][0] << "Rint, %";                         tab[14][1] << olxstr::FormatFloat(2, hs.Rint*100);
    tab[15][0] << "Rsigma, %";                       tab[15][1] << olxstr::FormatFloat(2, hs.Rsigma*100);
    tab[16][0] << "Mean I/sig";                   tab[16][1] << olxstr::FormatFloat(3, hs.MeanIOverSigma);
    tab[17][0] << "HKL range (refinement)";                    
    tab[17][1] << "h=[" << hs.MinIndexes[0] << ',' << hs.MaxIndexes[0] << "] "
               << "k=[" << hs.MinIndexes[1] << ',' << hs.MaxIndexes[1] << "] "
               << "l=[" << hs.MinIndexes[2] << ',' << hs.MaxIndexes[2] << "] ";
    tab[18][0] << "HKL range (file)";                    
    tab[18][1] << "h=[" << hs.FileMinInd[0] << ',' << hs.FileMaxInd[0] << "] "
               << "k=[" << hs.FileMinInd[1] << ',' << hs.FileMaxInd[1] << "] "
               << "l=[" << hs.FileMinInd[2] << ',' << hs.FileMaxInd[2] << "] ";
    tab[19][0] << "Maximum redundancy (+symm eqivs)";    tab[19][1] << hs.ReflectionAPotMax;
    tab[20][0] << "Average redundancy (+symm eqivs)";    tab[20][1] << olxstr::FormatFloat(2, (double)hs.TotalReflections/hs.UniqueReflections);

    TStrList Output;
    tab.CreateTXTList(Output, olxstr("Refinement reflection statistsics"), true, false, "  ");
    xapp.NewLogEntry() << Output;
    const TIntList& redInfo = xapp.XFile().GetRM().GetRedundancyInfo();
    int red_cnt = 0;
    for( size_t i=0; i < redInfo.Count(); i++ )
      if( redInfo[i] != 0 )
        red_cnt++;
    tab.Resize(red_cnt, 2);
    tab.ColName(0) = "Times measured";
    tab.ColName(1) = "Count";
    red_cnt = 0;
    for( size_t i=0; i < redInfo.Count(); i++ )  {
      if( redInfo[i] == 0 )  continue;
      tab[red_cnt][0] = i+1;
      tab[red_cnt++][1] = redInfo[i];
    }
    Output.Clear();
    tab.CreateTXTList(Output, olxstr("All reflection statistics"), true, false, "  ");
    xapp.NewLogEntry() << Output;
    //const vec3i_list empty_omits;
    //MergeStats fr_ms = RefMerger::DryMergeInP1<RefMerger::UnitMerger>(xapp.XFile().GetRM().GetFriedelPairs(), empty_omits);
    xapp.NewLogEntry() << "Friedel pairs measured (in P1): " << xapp.XFile().GetRM().GetFriedelPairCount();
    return;
  }
  bool list = Options.Contains("l"), 
       merge = Options.Contains("m");
  TRefList Refs;
  if( merge ) {
    xapp.XFile().GetRM().GetRefinementRefList<
      TUnitCell::SymmSpace,RefMerger::StandardMerger>(
        xapp.XFile().GetUnitCell().GetSymmSpace(), Refs);
  }
  else
    xapp.XFile().GetRM().GetFilteredP1RefList<RefMerger::StandardMerger>(Refs);
  evecd_list con;

  for( size_t i=0; i < Cmds.Count(); i++ )  {
    size_t obi = Cmds[i].FirstIndexOf('[');
    if( obi == InvalidIndex || !Cmds[i].EndsWith(']') )  {
      Error.ProcessingError(__OlxSrcInfo, "incorrect construct: ") << Cmds[i];
      return;
    }
    con.AddNew(4);
    con[i][3] = Cmds[i].SubStringTo(obi).ToInt();
    olxstr tmp = Cmds[i].SubString(obi+1, Cmds[i].Length() - obi - 2);
    int hkli=-1;
    for( size_t j=tmp.Length()-1; j != InvalidIndex; j-- ) {
      if( tmp.CharAt(j) == 'l' )  hkli = 2;
      else if( tmp.CharAt(j) == 'k' )  hkli = 1;
      else if( tmp.CharAt(j) == 'h' )  hkli = 0;
      if( hkli == -1 )  {
        Error.ProcessingError(__OlxSrcInfo, "incorrect construct: ") << Cmds[i];
        return;

      }
      j--;
      olxstr strV;
      while( j != InvalidIndex && !(tmp.CharAt(j) >= 'a' && tmp.CharAt(j) <= 'z' ) )  {
        strV.Insert((olxch)tmp[j], 0);
        j--;
      }
      if( !strV.IsEmpty() && !(strV == "+") && !(strV == "-") )
        con[i][hkli] = strV.ToDouble();
      else  {
        if( !strV.IsEmpty() && strV == "-" )
          con[i][hkli] = -1.0;
        else
          con[i][hkli] = 1.0;
      }
      if( con[i][hkli] == 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "illigal value: ") << Cmds[i];
        return;
      }
      j++;
    }
  }
  double SI = 0, SE = 0;
  size_t count = 0;
  for( size_t i=0; i < Refs.Count(); i++ )  {
    bool fulfilled = true;
    const TReflection& ref = Refs[i];
    for( size_t j=0; j < Cmds.Count(); j ++ )  {
      int v = olx_round(ref.GetH()*con[j][0] +
                    ref.GetK()*con[j][1] +
                    ref.GetL()*con[j][2]);
      if( con[j][3] == 0 )  {
        if( v != 0 ) {
          fulfilled = false;
          break;
        }
      }
      else if( con[j][3] < 0 )  {
        if( (v%(int)con[j][3]) == 0 )  {
          fulfilled = false;
          break;
        }
      }
      else if( con[j][3] > 0 )  {
        if( (v%(int)con[j][3]) != 0 )  {
          fulfilled = false;
          break;
        }
      }
    }
    if( !fulfilled )  continue;
    count ++;
    SI += ref.GetI();
    SE += olx_sqr(ref.GetS());
    if( list )  {
      TBasicApp::NewLogEntry() << ref.ToString();
    }
  }
  if( count == 0 )  {
    TBasicApp::NewLogEntry() << "Could not find any reflections fulfilling given condition";
    return;
  }
  SI /= count;
  SE = sqrt(SE/count);

  xapp.NewLogEntry() << "Found " << count << " reflections fulfilling given condition";
  xapp.NewLogEntry() << "I(s) is " << olxstr::FormatFloat(3, SI) << '(' << olxstr::FormatFloat(3, SE) << ")";

}
//..............................................................................
void XLibMacros::macHtab(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if( TXApp::GetInstance().XFile().GetLattice().IsGenerated() )  {
    E.ProcessingError(__OlxSrcInfo,
      "operation is not applicable to the grown structure");
    return;
  }
  double max_d = 2.9, min_ang = TXApp::GetMinHBondAngle();
  size_t cnt = XLibMacros::Parse(Cmds, "dd", &max_d, &min_ang);
  if( cnt == 1 )  {
    if( max_d > 10 )  {
      min_ang = max_d;
      max_d = 2.9;
    }
  }
  if( max_d > 5 )  {
    if( min_ang < 5 )
      olx_swap(max_d, min_ang);
    else
      max_d = 2.9;
  }
  TAsymmUnit& au = TXApp::GetInstance().XFile().GetAsymmUnit();
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  TStrList current;
  for( size_t i=0; i < rm.InfoTabCount(); i++ )  {
    InfoTab& it = rm.GetInfoTab(i);
    if( it.IsValid() && it.GetType() == infotab_htab )
      current.Add(it.InsStr()) << " d=" << olx_round(au.Orthogonalise(
      it.GetAtom(0).ccrd()-it.GetAtom(1).ccrd()).Length(), 1000);
  }
  if( !current.IsEmpty() )  {
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
  TBasicApp::NewLogEntry() << "Processing HTAB with max D-A distance " <<
    max_d << " and minimum angle " << min_ang;
  min_ang = cos(min_ang*M_PI/180.0);
  if( Options.Contains('t') )  {
    TStrList elm(Options.FindValue('t'), ',');
    for( size_t i=0; i < elm.Count(); i++ )  {
      cm_Element* e = XElementLib::FindBySymbol(elm[i]);
      if( e == NULL )
        TBasicApp::NewLogEntry() << "Unknown element type: " << elm[i];
      else if( bais.IndexOf(e->z) == InvalidIndex )
        bais.Add(e->z);
    }
  }
  TUnitCell& uc = TXApp::GetInstance().XFile().GetUnitCell();
  TLattice& lat = TXApp::GetInstance().XFile().GetLattice();
  TArrayList< AnAssociation2<TCAtom const*, smatd> > all;
  size_t h_indexes[4];
  const ASObjectProvider& objects = lat.GetObjects();
  for( size_t i=0; i < objects.atoms.Count(); i++ )  {
    TSAtom& sa = objects.atoms[i];
    const cm_Element& elm = sa.GetType();
    if( elm.GetMr() < 3.5 )  // H,D,Q
      continue;
    size_t hc = 0;
    for( size_t j=0; j < sa.NodeCount(); j++ )  {
      const cm_Element& elm1 = sa.Node(j).GetType();
      if( elm1 == iHydrogenZ )  {
        h_indexes[hc] = j;
        hc++;
        if( hc >= 4 )
          break;
      }
    }
    if( hc == 0 || hc >= 4 )  continue;
    all.Clear();
    uc.FindInRangeAM(sa.ccrd(), max_d+elm.r_bonding-0.6, all);
    for( size_t j=0; j < all.Count(); j++ )  {
      const TCAtom& ca = *all[j].GetA();
      const cm_Element& elm1 = ca.GetType();
      if(  bais.IndexOf(elm1.z) == InvalidIndex )  continue;
      vec3d cvec(all[j].GetB()*ca.ccrd()),
            bond(cvec - sa.ccrd());
      const double d = au.CellToCartesian(bond).Length();
      if( d < (elm.r_bonding + elm1.r_bonding + 0.4) ) // coval bond
        continue;  
      // analyse angles
      for( size_t k=0; k < hc; k++ )  {
        vec3d base = sa.Node(h_indexes[k]).ccrd();
        const vec3d v1 = au.Orthogonalise(sa.ccrd() - base);
        const vec3d v2 = au.Orthogonalise(cvec - base);
        const double c_a = v1.CAngle(v2);
        if( c_a < min_ang )  {  // > 150 degrees
          // NOTE: false!
          if( sa.GetType() == iCarbonZ && false )  {
            InfoTab& it_d = rm.AddRTAB(
              sa.GetType().symbol + ca.GetType().symbol);
            it_d.AddAtom(&sa.CAtom(), NULL);
            const smatd* mt = (!(all[j].GetB().t.IsNull() &&
              all[j].GetB().r.IsI()) ? &all[j].GetB() : NULL);
            if( mt != NULL && transforms.IndexOf(*mt) == InvalidIndex )
              transforms.AddCopy(*mt);
            it_d.AddAtom(const_cast<TCAtom*>(&ca), mt);
            if( rm.ValidateInfoTab(it_d) ) {
              TBasicApp::NewLogEntry() << it_d.InsStr() << " d=" <<
                olxstr::FormatFloat(3, d);
            }

            InfoTab& it_a = rm.AddRTAB(
              sa.GetType().symbol + ca.GetType().symbol);
            it_a.AddAtom(&sa.CAtom(), NULL);
            it_a.AddAtom(&sa.Node(h_indexes[k]).CAtom(), NULL);
            it_a.AddAtom(const_cast<TCAtom*>(&ca), mt);
            if( rm.ValidateInfoTab(it_a) ) {
              TBasicApp::NewLogEntry() << it_a.InsStr() << " a=" <<
                olxstr::FormatFloat(3, acos(c_a)*180.0/M_PI);
            }
          }
          else  {
            InfoTab& it = rm.AddHTAB();
            it.AddAtom(&sa.CAtom(), NULL);
            const smatd* mt = (!(all[j].GetB().t.IsNull() &&
              all[j].GetB().r.IsI()) ? &all[j].GetB() : NULL);
            if( mt != NULL && transforms.IndexOf(*mt) == InvalidIndex )
              transforms.AddCopy(*mt);
            it.AddAtom(const_cast<TCAtom*>(&ca), mt);
            if( rm.ValidateInfoTab(it) ) {
              TBasicApp::NewLogEntry() << it.InsStr() << " d=" <<
                olxstr::FormatFloat(3, d);
            }
          }
        }
      }
    }
  }
  if( Options.Contains('g') && !transforms.IsEmpty() )  {
    TLattice& xlatt = TXApp::GetInstance().XFile().GetLattice();
    const TUnitCell& uc = xlatt.GetUnitCell();
    const ASObjectProvider& objects = xlatt.GetObjects();
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
//..............................................................................
void XLibMacros::macHAdd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp &XApp = TXApp::GetInstance();
  if( XApp.XFile().GetLattice().IsGenerated() )  {
    Error.ProcessingError(__OlxSrcInfo, "not applicable to grown structures");
    return;
  }
  int Hfix = 0;
  if( !Cmds.IsEmpty() && Cmds[0].IsNumber() )  {
    Hfix = Cmds[0].ToInt();
    Cmds.Delete(0);
  }
  TAsymmUnit &au = XApp.XFile().GetAsymmUnit();
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom &ca = au.GetAtom(i);
    if( ca.GetType() == iHydrogenZ )
      ca.SetDetached(false);
  }
  TActionQueueLock q_draw(XApp.FindActionQueue(olxappevent_GL_DRAW));
  try  {
    TSAtomPList satoms;
    XApp.FindSAtoms(Cmds.Text(' '), satoms, true);
    // find atoms first, or selection gets lost...
    XApp.XFile().GetLattice().UpdateConnectivity();
    TXlConGen xlConGen(XApp.XFile().GetRM());
    if( Hfix == 0 ) 
      XApp.XFile().GetLattice().AnalyseHAdd(xlConGen, satoms);
    else  {
      RefinementModel& rm = XApp.XFile().GetRM();
      for( size_t aitr=0; aitr < satoms.Count(); aitr++ )  {
        TIntList parts;
        TDoubleList occu;
        TAtomEnvi AE;
        XApp.XFile().GetUnitCell().GetAtomEnviList(*satoms[aitr], AE);
        for( size_t i=0; i < AE.Count(); i++ )  {
          if( AE.GetCAtom(i).GetPart() != 0 &&
              AE.GetCAtom(i).GetPart() != AE.GetBase().CAtom().GetPart() )
          {
            if( parts.IndexOf(AE.GetCAtom(i).GetPart()) == InvalidIndex )  {
              parts.Add(AE.GetCAtom(i).GetPart());
              occu.Add(rm.Vars.GetParam(AE.GetCAtom(i), catom_var_name_Sof));
            }
          }
        }
        if( parts.Count() < 2 )  {
          int afix = TXlConGen::ShelxToOlex(Hfix, AE);
          if( afix != -1 )  {
            TCAtomPList generated;
            xlConGen.FixAtom(AE, afix, XElementLib::GetByIndex(iHydrogenIndex),
              NULL, &generated);
            if( !generated.IsEmpty() &&  // hack to get desired Hfix...
                generated[0]->GetParentAfixGroup() != NULL )
            {
              generated[0]->GetParentAfixGroup()->SetAfix(Hfix);
            }
          }
          else  {
            XApp.NewLogEntry() << "Failed to translate HFIX code for " <<
              satoms[aitr]->GetLabel() << " with " << AE.Count() << " bonds";
          }
        }
        else  {
          XApp.NewLogEntry() << "Processing " << parts.Count() << " parts";
          for( size_t i=0; i < parts.Count(); i++ )  {
            AE.Clear();
            XApp.XFile().GetUnitCell().GetAtomEnviList(*satoms[aitr], AE,
              false, parts[i]);
            /*consider special case where the atom is bound to itself but
            very long bond > 1.6 A */
            smatd* eqiv = NULL;
            for( size_t j=0; j < AE.Count(); j++ )  {
              if( &AE.GetCAtom(j) == &AE.GetBase().CAtom() )  {
                const double d = AE.GetCrd(j).DistanceTo(AE.GetBase().crd());
                if( d > 1.6 )  {
                  eqiv = new smatd(AE.GetMatrix(j));
                  AE.Delete(j);
                  break;
                }
              }
            }
            if( eqiv != NULL )  {
              const smatd& e = rm.AddUsedSymm(*eqiv);
              rm.Conn.RemBond(satoms[aitr]->CAtom(), satoms[aitr]->CAtom(),
                NULL, &e, true);
              XApp.NewLogEntry() << "The atom" << satoms[aitr]->GetLabel() << 
                " is connected to itself through symmetry, removing the"
                " symmetry generated bond";
              delete eqiv;
            }
            //
            int afix = TXlConGen::ShelxToOlex(Hfix, AE);
            if( afix != -1 )  {
              TCAtomPList generated;
              xlConGen.FixAtom(AE, afix,
                XElementLib::GetByIndex(iHydrogenIndex), NULL, &generated);
              for( size_t j=0; j < generated.Count(); j++ )  {
                generated[j]->SetPart(parts[i]);
                rm.Vars.SetParam(*generated[j], catom_var_name_Sof, occu[i]);
              }
              if( !generated.IsEmpty() &&
                  generated[0]->GetParentAfixGroup() != NULL )
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
  q_draw.Unlock();
  XApp.XFile().GetLattice().Init();
  delete XApp.FixHL();
}
//..............................................................................
void XLibMacros::macHImp(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TXApp& XApp = TXApp::GetInstance();
  if( XApp.XFile().GetLattice().IsGenerated() )  {
    Error.ProcessingError(__OlxSrcInfo,
      "The procedure is not applicable for the grown structure");
    return;
  }
  bool increase = false, 
    decrease = false;
  if( !Cmds[0].IsNumber() )  {
    Error.ProcessingError(__OlxSrcInfo,
      "first arument should be a number or +/- number");
    return;
  }
  double val = Cmds[0].ToDouble();
  if( Cmds[0].CharAt(0) == '+' )
    increase = true;
  else if( Cmds[0].CharAt(0) == '-' )
    decrease = true;
  Cmds.Delete(0);

  TSAtomPList satoms;
  XApp.FindSAtoms(Cmds.Text(' '), satoms, true);
  const double delta = XApp.XFile().GetLattice().GetDelta();
  for( size_t i=0; i < satoms.Count(); i++ )  {
    if( satoms[i]->GetType() != iHydrogenZ )
      continue;
    TSAtom& h = *satoms[i], *attached = NULL;
    size_t ac = 0;
    for( size_t j=0; j < h.NodeCount(); j++ )  {
      TSAtom& n = h.Node(j);
      if( !(n.IsDeleted() || n.GetType().z < 2) )  {
        ac++;
        attached = &n;
      }
    }
    if( ac > 1 || ac == 0 )  {
      XApp.NewLogEntry() << "Skipping " << h.GetLabel();
      continue;
    }
    vec3d v(h.crd() - attached->crd());
    if( increase || decrease )
      v.NormaliseTo(v.Length() + val);
    else
      v.NormaliseTo(val);
    v += attached->crd();
    double qd1 = v.QDistanceTo(attached->crd());
    double qd2 =  attached->GetType().r_bonding + h.GetType().r_bonding + delta;
    qd2 *= qd2;
    if( qd1 >= qd2-0.01 )  {
      XApp.NewLogEntry() << "Skipping " << h.GetLabel();
      continue;
    }
    h.crd() = v;
    XApp.XFile().GetAsymmUnit().CartesianToCell(v);
    h.CAtom().ccrd() = v;
    h.ccrd() = v;
  }
  XApp.XFile().GetLattice().UpdateConnectivity();
  //XApp.XFile().EndUpdate();
}
//..............................................................................
void XLibMacros::macAnis(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TSAtomPList atoms;
  if( !TXApp::GetInstance().FindSAtoms(Cmds.Text(' '), atoms, true) )  return;
  TCAtomPList catoms(atoms,
    FunctionAccessor::MakeConst(&TSAtom::CAtom));
  if( !Options.Contains("h") )
    catoms.Pack(TCAtom::TypeAnalyser(iHydrogenZ));
  catoms.Pack(TCAtom::TypeAnalyser(iQPeakZ));
  TXApp::GetInstance().XFile().GetLattice().SetAnis(catoms, true);
}
//..............................................................................
void XLibMacros::macIsot(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if (Options.Contains("npd")) {
    TAsymmUnit &au = TXApp::GetInstance().XFile().GetAsymmUnit();
    TCAtomPList catoms;
    for (size_t i=0; i < au.AtomCount(); i++) {
      TCAtom &a = au.GetAtom(i);
      if (a.GetEllipsoid() != NULL && a.GetEllipsoid()->IsNPD())
        catoms.Add(a);
    }
    TXApp::GetInstance().XFile().GetLattice().SetAnis(catoms, false);
    return;
  }
  TSAtomPList atoms;
  if( !TXApp::GetInstance().FindSAtoms(Cmds.Text(' '), atoms, true) )  return;
  TCAtomPList catoms(atoms,
    FunctionAccessor::MakeConst(&TSAtom::CAtom));
  TXApp::GetInstance().XFile().GetLattice().SetAnis(catoms, false);
}
//..............................................................................
void XLibMacros::macFix(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  olxstr vars(Cmds[0]);
  Cmds.Delete(0);
  double var_val = 0;
  if( !Cmds.IsEmpty() && Cmds[0].IsNumber() )  {
    var_val = Cmds[0].ToDouble();
    Cmds.Delete(0);
  }
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList atoms;
  if( !xapp.FindSAtoms(Cmds.Text(' '), atoms, true, true) )  
    return;

  if( vars.Equalsi( "XYZ" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      for( short j=0; j < 3; j++ )
        xapp.XFile().GetRM().Vars.FixParam(atoms[i]->CAtom(), catom_var_name_X+j);
    }
  }
  else if( vars.Equalsi( "UISO" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetEllipsoid() == NULL )  // isotropic atom
        xapp.SetAtomUiso(*atoms[i], var_val);
      else  {
        for( short j=0; j < 6; j++ )
          xapp.XFile().GetRM().Vars.FixParam(atoms[i]->CAtom(), catom_var_name_U11+j);
      }
    }
  }
  else if( vars.Equalsi( "OCCU" ) )  {
    const ASObjectProvider& objects = xapp.XFile().GetLattice().GetObjects();
    objects.atoms.ForEach(ACollectionItem::TagSetter(0));
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetTag() != 0 )  continue;
      TSAtomPList neighbours;
      neighbours.Add(atoms[i]);
      for( size_t j=0; j < atoms[i]->NodeCount(); j++ )  {
        TSAtom& n = atoms[i]->Node(j);
        if( n.IsDeleted() || n.GetType() != iHydrogenZ || n.GetTag() != 0 )  continue;
        neighbours.Add(n);
      }
      for( size_t j=0; j < neighbours.Count(); j++ )  {
        if( neighbours[j]->CAtom().GetPart() != atoms[i]->CAtom().GetPart() )
          continue;
        neighbours[j]->SetTag(1);
        xapp.XFile().GetRM().Vars.FixParam(neighbours[j]->CAtom(), catom_var_name_Sof);
        if( var_val == 0 )  {
          if( neighbours[j]->CAtom().GetPart() == 0 )  // else leave as it is
            neighbours[j]->CAtom().SetOccu(1./neighbours[j]->CAtom().GetDegeneracy());
        }
        else
          neighbours[j]->CAtom().SetOccu(var_val/neighbours[j]->CAtom().GetDegeneracy());
      }
    }
  }
}
//..............................................................................
void XLibMacros::macFree(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  olxstr vars = Cmds[0];
  Cmds.Delete(0);
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.FindSAtoms(Cmds.Text(' '), atoms, true, true) )  return;
  if( vars.Equalsi( "XYZ" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      for( short j=0; j < 3; j++ )
        xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(), catom_var_name_X+j);
    }
  }
  else if( vars.Equalsi( "UISO" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->CAtom().GetEllipsoid() == NULL )  {  // isotropic atom
        xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(), catom_var_name_Uiso);
      }
      else  {
        for( short j=0; j < 6; j++ )
          xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(), catom_var_name_U11+j);
      }
      if( atoms[i]->CAtom().GetUisoOwner() != NULL )  {
        TAfixGroup *ag = atoms[i]->CAtom().GetParentAfixGroup();
        if( ag != NULL && ag->GetAfix() == -1 )
          ag->RemoveDependent(atoms[i]->CAtom());
        atoms[i]->CAtom().SetUisoOwner(NULL);
      }
    }
  }
  else if( vars.Equalsi( "OCCU" ) )  {
    for( size_t i=0; i < atoms.Count(); i++ ) 
      xapp.XFile().GetRM().Vars.FreeParam(atoms[i]->CAtom(), catom_var_name_Sof);
  }
}
//..............................................................................
void XLibMacros::macFixHL(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp & xapp = TXApp::GetInstance();
  TAsymmUnit &au = xapp.XFile().GetAsymmUnit();
  TEBitArray detached(au.AtomCount());
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom &ca = au.GetAtom(i);
    detached.Set(i, ca.IsDetached());
    if( ca.GetType() == iQPeakZ )
      ca.SetDetached(true);
    else if( ca.GetType().GetMr() < 3.5 )
      ca.SetDetached(false);
  }
  TActionQueueLock q_draw(xapp.FindActionQueue(olxappevent_GL_DRAW));
  xapp.XFile().GetLattice().UpdateConnectivity();
  delete TXApp::GetInstance().FixHL();
  for( size_t i=0; i < au.AtomCount(); i++ )
    au.GetAtom(i).SetDetached(detached[i]);
  xapp.XFile().GetLattice().UpdateConnectivity();
}
//..............................................................................
// http://www.minsocam.org/ammin/AM78/AM78_1104.pdf
int macGraphPD_Sort(const AnAssociation2<double,double> &a1,
  const AnAssociation2<double,double> &a2)
{
  return olx_cmp(a1.GetA(), a2.GetA());
}
void XLibMacros::macGraphPD(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TRefList refs;
  double res = Options.FindValue("r", "0.5").ToDouble();
  TArrayList<compd > F;
  olxstr err(SFUtil::GetSF(refs, F, SFUtil::mapTypeObs, 
    Options.Contains("fcf") ? SFUtil::sfOriginFcf : SFUtil::sfOriginOlex2, 
    (Options.FindValue("s", "r").ToLowerCase().CharAt(0) == 'r') ? SFUtil::scaleRegression : SFUtil::scaleSimple));
  if( !err.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, err);
    return;
  }
  TEFile out( TEFile::ExtractFilePath(xapp.XFile().GetFileName()) << "olx_pd_calc.csv", "w+b");
  const mat3d& hkl2c = xapp.XFile().GetAsymmUnit().GetHklToCartesian();
  const double half_lambda = xapp.XFile().GetRM().expl.GetRadiation()/2.0;
  //refs.Clear();
  //xapp.XFile().GetRM().GetFourierRefList<TUnitCell::SymSpace, RefMerger::ShelxMerger>(
  //  xapp.XFile().GetUnitCell().GetSymmSpace(), refs);
  TTypeList< AnAssociation2<double,double> > gd;
  gd.SetCapacity(refs.Count());
  double max_2t = 0, min_2t=180;
  for( size_t i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    vec3d hkl = ref.ToCart(hkl2c);
    const double theta_2 = 2*asin(half_lambda*hkl.Length());
    const double lp = 1;//(1.0+olx_sqr(cos(theta_2)))/(olx_sqr(sin(theta_2/2))*cos(theta_2/2));
    gd.AddNew(theta_2*180/M_PI, ref.GetI()*ref.GetMultiplicity()*lp);
  }
  QuickSorter::SortSF(gd, macGraphPD_Sort);
  min_2t = gd[0].GetA();
  max_2t = gd.GetLast().GetA();
  const double sig_0 = 1./80. + (max_2t-min_2t)/800.0;
  const size_t ref_cnt = refs.Count();
  for( double s = min_2t; s <= max_2t; s += res )  {
    double y = 0.0001;  
    for( size_t i=0; i < ref_cnt; i++ )  { 
      const double sig = sig_0*(1.0+gd[i].GetA()/140.0);
      const double qsig = sig*sig;
      y += gd[i].GetB()*exp(-olx_sqr(s-gd[i].GetA())/(2*qsig))/sig;
    }
    out.Writeln(olxcstr(s, 40) << ',' << y);
  }
}
//..............................................................................
void XLibMacros::macFile(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& XApp = TXApp::GetInstance();
  olxstr Tmp;
  if( Cmds.IsEmpty() )  {  // res -> Ins rotation if ins file
    if( XApp.CheckFileType<TIns>() )  {
      Tmp = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "ins");
    }
    else
      Tmp = XApp.XFile().GetFileName();
  }
  else
    Tmp = Cmds[0];

  bool Sort = Options.Contains('s');

  if( TEFile::ExtractFilePath(Tmp).IsEmpty() )
    Tmp = TEFile::AddPathDelimeter(CurrentDir) + Tmp;
  TEBitArray removedSAtoms, removedCAtoms;
  if( TEFile::ExtractFileExt(Tmp).Equalsi("ins"))  {  // kill Q peak in the ins file
    ASObjectProvider& objects = XApp.XFile().GetLattice().GetObjects();
    removedSAtoms.SetSize(objects.atoms.Count());
    for( size_t i=0; i < objects.atoms.Count(); i++ )  {
      TSAtom& sa = objects.atoms[i];
      if( sa.GetType() == iQPeakZ && !sa.IsDeleted() )  {
        sa.SetDeleted(true);
        removedSAtoms.SetTrue(i);
      }
    }
    TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
    removedCAtoms.SetSize(au.AtomCount());
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      TCAtom& ca = au.GetAtom(i);
      if( ca.GetType() == iQPeakZ && !ca.IsDeleted() )  {
        ca.SetDeleted(true);
        removedCAtoms.SetTrue(i);
      }
    }
  }
  
  XApp.XFile().SaveToFile(Tmp, Sort);
  if( XApp.XFile().HasLastLoader() )  {
    olxstr fd = TEFile::ExtractFilePath(Tmp);
    if( !fd.IsEmpty() && !fd.Equalsi(CurrentDir) )  {
      if( !TEFile::ChangeDir(fd) )
        TBasicApp::NewLogEntry(logError) << "Cannot change current folder...";
      else
        CurrentDir = fd;
    }
  }
  else  if( !Sort )  {
    Sort = true;  // forse reading the file
  }
  if( !removedSAtoms.IsEmpty() )  {  // need to restore, a bit of mess here...
    ASObjectProvider& objects = XApp.XFile().GetLattice().GetObjects();
    for( size_t i=0; i < objects.atoms.Count(); i++ )  {
      if( removedSAtoms.Get(i) )
        objects.atoms[i].SetDeleted(false);
    }
    TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      if( removedCAtoms[i] )
          au.GetAtom(i).SetDeleted(false);
    }
  }
  if( Sort )  {
    olex::IOlexProcessor* op = olex::IOlexProcessor::GetInstance();
      if( op != NULL )
        op->executeMacro(olxstr("reap \'") << Tmp << '\'');
  }
}
//..............................................................................
void XLibMacros::macFuse(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Cmds.Count() == 1 && Cmds[0].IsNumber() )  {
    const double th = Cmds[0].ToDouble();
    TLattice& latt = TXApp::GetInstance().XFile().GetLattice();
    ASObjectProvider& objects = latt.GetObjects();
    for( size_t i=0; i < objects.atoms.Count(); i++ )  {
      TSAtom& sa = objects.atoms[i];
      if( sa.IsDeleted() )  continue;
      if( sa.BondCount() == 0 )  continue;
      sa.SortBondsByLengthAsc();
      vec3d cnt(sa.crd());
      size_t ac = 1;
      for( size_t j=0; j < sa.BondCount(); j++ )  {
        if( sa.Bond(j).Length() < th )  {
          TSAtom& asa = sa.Bond(j).Another(sa);
          if( asa.GetType() != sa.GetType() )
            continue;
          ac++;
          cnt += asa.crd();
          asa.CAtom().SetDeleted(true);
          asa.SetDeleted(true);
        }    
        else
          break;
      }
      if( ac > 1 )  {
        cnt /= ac;
        sa.CAtom().ccrd() = latt.GetAsymmUnit().CartesianToCell(cnt);
      }
    }
    TXApp::GetInstance().XFile().GetLattice().Uniq(true);
  }
  else
    TXApp::GetInstance().XFile().GetLattice().Uniq(Options.Contains("f"));
}
//..............................................................................
void XLibMacros::macLstIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
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
//..............................................................................
void XLibMacros::macAddIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  // if instruction is parsed, it goes to current model, otherwise i stays in the ins file
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  if( !Ins.AddIns(TStrList(Cmds), TXApp::GetInstance().XFile().GetRM()) )  {
    Error.ProcessingError(__OlxSrcInfo, olxstr("could not add instruction: ") << Cmds.Text(' '));
    return;
  }
}
//..............................................................................
void XLibMacros::macDelIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  if( Cmds[0].IsNumber() )  {
    int insIndex = Cmds[0].ToInt();
    Ins.DelIns(insIndex);
  }
  else  {
    if( Cmds[0].Equalsi("OMIT") )
      TXApp::GetInstance().XFile().GetRM().ClearOmits();
    else if( Cmds[0].Equalsi("TWIN") )
      TXApp::GetInstance().XFile().GetRM().RemoveTWIN();
    else if( Cmds[0].Equalsi("BASF") )
      TXApp::GetInstance().XFile().GetRM().ClearBASF();
    else  {
      for( size_t i=0; i < Ins.InsCount(); i++ )  {
        if( Ins.InsName(i).Equalsi(Cmds[0]) )  {
          Ins.DelIns(i--);
          continue;
        }
      }
    }
  }
  OnDelIns.Exit(NULL, &Cmds[0]);
}
//..............................................................................
void XLibMacros::macLS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  int ls = -1;
  XLibMacros::Parse(Cmds, "i", &ls);
  if( ls != -1 )  
    TXApp::GetInstance().XFile().GetRM().SetIterations((int)ls);
  if( !Cmds.IsEmpty() )
    TXApp::GetInstance().XFile().GetRM().SetRefinementMethod(Cmds[0]);
}
//..............................................................................
void XLibMacros::macUpdateWght(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  if( rm.proposed_weight.Count() == 0 )  return;
  if( Cmds.IsEmpty() )  
    rm.used_weight = rm.proposed_weight;
  else  {
    rm.used_weight.SetCount(Cmds.Count());
    for( size_t i=0; i < Cmds.Count(); i++ )  
      rm.used_weight[i] = Cmds[i].ToDouble();
  }
}
//..............................................................................
void XLibMacros::macUser(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    TBasicApp::NewLogEntry() << TEFile::CurrentDir();
  }
  else if( !TEFile::ChangeDir(Cmds[0]) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not change current folder");
  }
  else
    CurrentDir = Cmds[0]; 
}
//..............................................................................
void XLibMacros::macDir(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
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
//..............................................................................
void XLibMacros::macLstMac(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
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
//..............................................................................
void XLibMacros::macLstFun(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
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
//..............................................................................
void XLibMacros::ChangeCell(const mat3d& tm, const TSpaceGroup& new_sg, const olxstr& resHKL_FN)  {
  TXApp& xapp = TXApp::GetInstance();
  TBasicApp::NewLogEntry() << "Cell choice trasformation matrix:" << NewLineSequence() <<
    tm[0].ToString() << NewLineSequence() <<
    tm[1].ToString() << NewLineSequence() <<
    tm[2].ToString();
  TBasicApp::NewLogEntry() << "New space group: " << new_sg.GetName();
  const mat3d tm_t(mat3d::Transpose(tm));
  xapp.XFile().UpdateAsymmUnit();
  TAsymmUnit& au = xapp.XFile().LastLoader()->GetAsymmUnit();
  const mat3d i_tm(tm.Inverse());
  const mat3d f2c = mat3d::Transpose((mat3d::Transpose(au.GetCellToCartesian())*tm));
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
  au.GetAngles()[0] = acos(f2c[1].CAngle(f2c[2]))*180.0/M_PI;  au.GetAngleEsds()[0] = sqrt(an_err[0][0]);
  au.GetAngles()[1] = acos(f2c[0].CAngle(f2c[2]))*180.0/M_PI;  au.GetAngleEsds()[1] = sqrt(an_err[1][1]);
  au.GetAngles()[2] = acos(f2c[0].CAngle(f2c[1]))*180.0/M_PI;  au.GetAngleEsds()[2] = sqrt(an_err[2][2]);
  const mat3d old_cac = au.GetCartesianToCell();
  au.InitMatrices();
  const mat3d elptm = mat3d::Transpose(au.GetCellToCartesian())*i_tm*mat3d::Transpose(old_cac);
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    ca.ccrd() = i_tm * ca.ccrd();
    if( ca.GetEllipsoid() != NULL )
      ca.GetEllipsoid()->MultMatrix(elptm);
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
  if( !resHKL_FN.IsEmpty() )  {
    olxstr hkl_fn(xapp.LocateHklFile());
    if( !hkl_fn.IsEmpty() )  {
      THklFile hklf;
      hklf.LoadFromFile(hkl_fn);
      for( size_t i=0; i < hklf.RefCount(); i++ )
        hklf[i].SetHkl((tm_t * vec3d(hklf[i].GetHkl())).Round<int>());
      hklf.SaveToFile(resHKL_FN);
      xapp.XFile().GetRM().SetHKLSource(resHKL_FN);
      save = true;
    }
    else
      TBasicApp::NewLogEntry(logError) << "Could not locate source HKL file";
  }
  else  {
    const mat3d hklf_mat = tm_t*xapp.XFile().LastLoader()->GetRM().GetHKLF_mat();
    xapp.XFile().LastLoader()->GetRM().SetHKLF_mat(hklf_mat);
  }
  au.ChangeSpaceGroup(new_sg);
  au.InitMatrices();
  xapp.XFile().LastLoaderChanged();
  if( save )
    xapp.XFile().SaveToFile(xapp.XFile().GetFileName(), false);
}
//..............................................................................
TSpaceGroup* XLibMacros_macSGS_FindSG(TPtrList<TSpaceGroup>& sgs, const olxstr& axis)  {
  for( size_t i=0; i < sgs.Count(); i++ )
    if( sgs[i]->GetAxis().Compare(axis) == 0 )
      return sgs[i];
  return NULL;
}
olxstr XLibMacros_macSGS_SgInfo(const olxstr& caxis)  {
  if( caxis.IsEmpty() )
    return "standard";
  else  {
    if( caxis.Length() == 3 && caxis.CharAt(0) == '-' )    // -axis + cell choice
      return olxstr("axis: -") << caxis.CharAt(1) << ", cell choice " << caxis.CharAt(2);
    else if( caxis.Length() == 2 )    // axis + cell choice
      return olxstr("axis: ") << caxis.CharAt(0) << ", cell choice " << caxis.CharAt(1);
    else  
      return olxstr("axis: ") << caxis;
  }
}
void XLibMacros::macSGS(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TXApp& xapp = TXApp::GetInstance();
  olxstr hkl_fn;
  if( Cmds.Count() > 1 && Cmds.GetLastString().EndsWithi(".hkl") )  {
    hkl_fn = Cmds.GetLastString();
    Cmds.Delete(Cmds.Count()-1);
  }
  if( Cmds.Count() == 10 )  {  // transformation provided?
    TSpaceGroup* sg = TSymmLib::GetInstance().FindGroupByName(Cmds[9]);
    if( sg == NULL )  {
      E.ProcessingError(__OlxSrcInfo, "undefined space group");
      return;
    }
    mat3d tm;
    for( int i=0; i < 9; i++ )
      tm[i/3][i%3] = Cmds[i].ToDouble();
    if( !tm.IsI() )  {
      ChangeCell(tm, *sg, hkl_fn);
      TBasicApp::NewLogEntry() << "The cell, atomic coordinates and ADP's are "
        "transformed using user transform";
    }
    return;
  }
  TSpaceGroup& sg = Cmds.Count() == 1 ? xapp.XFile().GetLastLoaderSG() :
    *TSymmLib::GetInstance().FindGroupByName(Cmds[1]);
  if( &sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "undefined space group");
    return;
  }
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
//..............................................................................
void XLibMacros::macLstVar(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
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
#ifndef _NO_PYTHON_
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
#ifndef _NO_PYTHON
    if( TOlxVars::GetVarWrapper(i) != NULL )
      tab[rowsCount][2] = TOlxVars::GetVarWrapper(i)->ob_refcnt;
    else
      tab[rowsCount][2] = NAString;
#endif
    rowsCount++;
  }
  tab.SetRowCount(rowsCount);
  TBasicApp::NewLogEntry() <<
    tab.CreateTXTList("Variables list", true, true, ' ');
}
//..............................................................................
void XLibMacros::macLstFS(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
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
//..............................................................................
void XLibMacros::macPlan(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  int plan = Cmds[0].ToInt();
  if( plan == -1 )  return; // leave like it is
  TXApp::GetInstance().XFile().GetRM().SetPlan(plan);
}
//..............................................................................
void XLibMacros::macFixUnit(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  double Zp = Cmds.IsEmpty() ? 1 : Cmds[0].ToDouble();
  if( Zp <= 0 )  Zp = 1;
  TXApp::GetInstance().XFile().UpdateAsymmUnit();
  TAsymmUnit& au = TXApp::GetInstance().XFile().GetAsymmUnit();
  TUnitCell& uc = TXApp::GetInstance().XFile().GetUnitCell();
  ContentList content = au.GetContentList();
  const int Z_sg = (int)uc.MatrixCount();
  int Z = olx_max(olx_round(Z_sg*Zp), 1);
  au.SetZ(Z);
  olxstr n_c;
  for( size_t i=0; i < content.Count(); i++ )  {
    n_c << content[i].element.symbol <<
      olxstr::FormatFloat(3,content[i].count/Zp).TrimFloat();
    if( (i+1) < content.Count() )
      n_c << ' ';
    content[i].count *= Z_sg;
  }
  TBasicApp::NewLogEntry() << "New content is: " << n_c;
  TXApp::GetInstance().XFile().GetRM().SetUserContent(content);
}
//..............................................................................
void XLibMacros::macGenDisp(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  const bool neutron = Options.Contains('n');
  const bool full = Options.Contains('f') || neutron;
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  const ContentList& content = rm.GetUserContent();
  const double en = rm.expl.GetRadiationEnergy();
  if( !full )  {
    for( size_t i=0; i < content.Count(); i++ )  {
      XScatterer* sc = new XScatterer(content[i].element.symbol);
      sc->SetFpFdp(content[i].element.CalcFpFdp(en) - content[i].element.z);
      rm.AddSfac(*sc);
    }
  }
  else  {
    cm_Absorption_Coefficient_Reg ac;
    for( size_t i=0; i < content.Count(); i++ )  {
      XScatterer* sc = new XScatterer(content[i].element, en);
      sc->SetFpFdp(content[i].element.CalcFpFdp(en) - content[i].element.z);
      try  {
        double absorpc =
          ac.CalcMuOverRhoForE(en, *ac.locate(content[i].element.symbol));
        CXConnInfo& ci = rm.Conn.GetConnInfo(content[i].element);
        sc->SetMu(absorpc*content[i].element.GetMr()/0.6022142);
        sc->SetR(ci.r);
        sc->SetWeight(content[i].element.GetMr());
        delete &ci;
      }
      catch(...)  {
        TBasicApp::NewLogEntry() << "Could not locate absorption data for: " <<
          content[i].element.symbol;
      }
      if( neutron )  {
        if( content[i].element.neutron_scattering == NULL )  {
          TBasicApp::NewLogEntry() << "Could not locate neutron data for: " <<
            content[i].element.symbol;
        }
        else  {
          sc->SetGaussians(
            cm_Gaussians(0,0,0,0,0,0,0,0,
              content[i].element.neutron_scattering->coh.GetRe()));
        }
      }
      rm.AddSfac(*sc);
    }
  }
}
//..............................................................................
void XLibMacros::macEXYZ(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.FindSAtoms(EmptyString(), atoms, false, true) )  {
    E.ProcessingError(__OlxSrcInfo, "No atoms provided");
    return;
  }
  if( atoms.Count() == 1 && Cmds.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "Please specify additional atom type(s)");
    return;
  }
  if( atoms.Count() > 1 )  {
    if( !Cmds.IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "No arguments is expected");
      return;
    }
    else  {
      ElementDict elms;
      for( size_t i=0; i < atoms.Count(); i++ )  {
        if( elms.IndexOf(&atoms[i]->GetType()) != InvalidIndex )  {
          E.ProcessingError(__OlxSrcInfo,
            "Atoms of different type are expected");
          return;
        }
      }
    }
  }
  const bool set_eadp = Options.Contains("eadp");
  TCAtomPList processed;
  TPtrList<TExyzGroup> groups;
  RefinementModel& rm = xapp.XFile().GetRM();
  TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  if( atoms.Count() == 1 )  {
    ElementPList elms;
    if( atoms[0]->CAtom().GetExyzGroup() != NULL &&
      !atoms[0]->CAtom().GetExyzGroup()->IsEmpty() )
    {
      groups.Add(atoms[0]->CAtom().GetExyzGroup());
      elms.Add(atoms[0]->GetType());
    }
    else  {
      groups.Add(rm.ExyzGroups.New())->Add(atoms[0]->CAtom());
    }
    for( size_t i=0; i < Cmds.Count(); i++ )  {
      cm_Element* elm = XElementLib::FindBySymbol(Cmds[i]);
      if( elm == NULL )  {
        xapp.NewLogEntry(logError) << "Unknown element: " << Cmds[i];
        continue;
      }
      if( elms.IndexOf(elm) == InvalidIndex )  {
        TCAtom& ca = au.NewAtom();
        ca.ccrd() = atoms[0]->CAtom().ccrd();
        ca.SetLabel(elm->symbol + atoms[0]->GetLabel().SubStringFrom( 
          atoms[0]->GetType().symbol.Length()), false);
        ca.SetType(*elm);
        ca.AssignEquivs(atoms[0]->CAtom());
        rm.Vars.FixParam(ca, catom_var_name_Sof);
        groups[0]->Add(ca);
        ca.SetUiso(atoms[0]->CAtom().GetUiso());
      }
    }
    processed.Add(atoms[0]->CAtom());
  }
  else  {  // model type swapping disorder
    const int part = au.GetNextPart();
    for( size_t i=0; i < atoms.Count(); i++ )  {
      groups.Add(rm.ExyzGroups.New())->Add(atoms[i]->CAtom());
      for( size_t j=1; j < atoms.Count(); j++ )  {
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
      for( size_t j=0; j < groups[i]->Count(); j++ )
        (*groups[i])[j].SetPart((int8_t)(part+j));
      processed.Add(atoms[i]->CAtom());
    }
  }
  // special case - cross-link 4 atoms by one variable
  if( groups.Count() == 2 && groups[0]->Count() == 2 && groups[1]->Count() )  {
    XVar& vr = rm.Vars.NewVar();
    rm.Vars.AddVarRef(vr,
      (*groups[0])[0], catom_var_name_Sof, relation_AsVar, 1.0);
    rm.Vars.AddVarRef(vr,
      (*groups[0])[1], catom_var_name_Sof, relation_AsOneMinusVar, 1.0);
    rm.Vars.AddVarRef(vr,
      (*groups[1])[0], catom_var_name_Sof, relation_AsVar, 1.0); 
    rm.Vars.AddVarRef(vr,
      (*groups[1])[1], catom_var_name_Sof, relation_AsOneMinusVar, 1.0); 
    if( set_eadp )  {
      rm.rEADP.AddNew()
        .AddAtom((*groups[0])[0], NULL)
        .AddAtom((*groups[0])[1], NULL);
      rm.rEADP.AddNew()
        .AddAtom((*groups[1])[0], NULL)
        .AddAtom((*groups[1])[1], NULL);
    }
  }
  else  {
    for( size_t i=0; i < groups.Count(); i++ )  {
      if( groups[i]->Count() > 1 )  {
        TSimpleRestraint* sr = set_eadp ? &rm.rEADP.AddNew() : NULL;
        XLEQ* leq = NULL;
        if( groups[i]->Count() == 2 )  {
          XVar& vr = rm.Vars.NewVar();
          rm.Vars.AddVarRef(vr,
            (*groups[i])[0], catom_var_name_Sof, relation_AsVar, 1.0); 
          rm.Vars.AddVarRef(vr,
            (*groups[i])[1], catom_var_name_Sof, relation_AsOneMinusVar, 1.0); 
        }
        else
          leq = &rm.Vars.NewEquation();
        for( size_t j=0; j < groups[i]->Count(); j++ )  {
          if( (*groups[i])[j].IsDeleted() )  continue;
          if( leq != NULL )  {
            XVar& vr = rm.Vars.NewVar(1./groups[i]->Count());
            rm.Vars.AddVarRef(vr,
              (*groups[i])[j], catom_var_name_Sof, relation_AsVar, 1.0); 
            leq->AddMember(vr);
          }
          if( sr != NULL )
            sr->AddAtom((*groups[i])[j], NULL);
        }
      }
    }
  }
  // force the split atom to become isotropic
  xapp.XFile().GetLattice().SetAnis(processed, false);
}
//..............................................................................
void XLibMacros::macEADP(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  xapp.FindSAtoms(Cmds.Text(' '), atoms, false, true);
  if( atoms.Count() < 2 )  {
    E.ProcessingError(__OlxSrcInfo, "not enough atoms provided");
    return;
  }
  // validate that atoms of the same type
  bool allIso = (atoms[0]->GetEllipsoid() == NULL);
  for( size_t i=1; i < atoms.Count(); i++ )  {
    if( (atoms[i]->GetEllipsoid() == NULL) != allIso )  {
      E.ProcessingError(__OlxSrcInfo, "mixed atoms types (aniso and iso)");
      return;
    }
  }
  TSimpleRestraint& sr = xapp.XFile().GetRM().rEADP.AddNew();
  for( size_t i=0; i < atoms.Count(); i++ )
    sr.AddAtom(atoms[i]->CAtom(), NULL);
  xapp.XFile().GetRM().rEADP.ValidateRestraint(sr);
}
//..............................................................................
void XLibMacros::macAddSE(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TXApp& xapp = TXApp::GetInstance();
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  if( au.AtomCount() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "Empty asymmetric unit");
    return;
  }
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "Could not identify current space group");
    return;
  }
  if( sg->IsCentrosymmetric() && Cmds.Count() == 1 )  {
    E.ProcessingError(__OlxSrcInfo, "Centrosymmetric space group");
    return;
  }
  if( Cmds.Count() == 1 )  {
    SymmSpace::Info si = sg->GetInfo();
    si.centrosymmetric = true;
    sg = TSymmLib::GetInstance().FindGroupByHallSymbol(
      HallSymbol::Evaluate(si), sg);
  }
  else if( Cmds.Count() == 2 )  {
    TSAtomPList atoms;
    xapp.FindSAtoms(EmptyString(), atoms);
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      for( size_t j=i+1; j < au.AtomCount(); j++ )  {
        if( au.GetAtom(j).IsDeleted() )  continue;
        double d = uc.FindClosestDistance(au.GetAtom(i), au.GetAtom(j));
        if( d < 0.5 )  {
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
  if( !st.GetResults().IsEmpty() )  {
    size_t ind = st.GetResults().Count()-1;
    double match = (double)(st.GetResults()[ind].Count()*200/st.AtomCount());
    while( match > 125 && tol > 1e-4 )  {
      tol /= 4;
      st.TestMatrix(m, tol);
      ind = st.GetResults().Count()-1;
      match = st.GetResults().IsEmpty() ? 0.0
        : st.GetResults()[ind].Count()*200/st.AtomCount();
      continue;
    }
    if( st.GetResults().IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "ooops...");
      return;
    }
    vec3d trans = st.GetResults()[ind].Center;
    //TVectorD trans = st.GetGravityCenter();
    trans /= 2;
    trans *= -1;
    m.t = trans;
    TSAtomPList atoms;
    xapp.FindSAtoms(EmptyString(), atoms);
    xapp.XFile().GetLattice().TransformFragments(atoms, m);
    au.ChangeSpaceGroup(*sg);
    xapp.XFile().LastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
    latt.Init();
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      for( size_t j=i+1; j < au.AtomCount(); j++ )  {
        if( au.GetAtom(j).IsDeleted() )  continue;
        double d = uc.FindClosestDistance(au.GetAtom(i), au.GetAtom(j));
        if( d < 0.5 )  {
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
  else  {
    E.ProcessingError(__OlxSrcInfo, "could not find interatomic relations");
  }
}
//..............................................................................
void XLibMacros::macCompaq(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  if( Options.Contains('a') )  
    TXApp::GetInstance().XFile().GetLattice().CompaqAll();
  else if( Options.Contains('c') )  
    TXApp::GetInstance().XFile().GetLattice().CompaqClosest();
  else if( Options.Contains('q') )  
    TXApp::GetInstance().XFile().GetLattice().CompaqQ();
  else if( Options.Contains('m') )  {
    TXFile &xf = TXApp::GetInstance().XFile();
    TAsymmUnit &au = xf.GetAsymmUnit();
    TIntList modified(au.AtomCount());
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      TCAtom &a = au.GetAtom(i);
      modified[i] = -100;
      if( XElementLib::IsMetal(a.GetType()) )  {
        modified[i] = a.GetConnInfo().maxBonds;
        a.GetConnInfo().maxBonds = 0;
      }
      else if( a.GetType() == iQPeakZ && !a.IsDetached() )  {
        modified[i] = -101;
        a.SetDetached(true);
      }
    }
    xf.GetLattice().Init();
    xf.GetLattice().CompaqAll();
    xf.GetLattice().CompaqClosest();
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      if( modified[i] == -101 )
        au.GetAtom(i).SetDetached(false);
      else if( modified[i] != -100 )
        au.GetAtom(i).GetConnInfo().maxBonds = modified[i];
    }
    xf.GetLattice().Init();
    xf.GetLattice().CompaqQ();
  }
  else
    TXApp::GetInstance().XFile().GetLattice().Compaq();
}
//..............................................................................
void XLibMacros::macEnvi(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  double r = 2.7;
  Parse(Cmds, "d", &r);
  if( r < 1 || r > 10 )  {
    E.ProcessingError(__OlxSrcInfo, "radius must be within [1;10] range");
    return;
  }
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.FindSAtoms(Cmds.Text(' '), atoms, false, false) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  ElementPList Exceptions;
  Exceptions.Add(XElementLib::GetByIndex(iQPeakIndex));
  Exceptions.Add(XElementLib::GetByIndex(iHydrogenIndex));
  if( Options.Contains('q') )
    Exceptions.Remove(XElementLib::GetByIndex(iQPeakIndex));
  if( Options.Contains('h') )
    Exceptions.Remove(XElementLib::GetByIndex(iHydrogenIndex));

  TSAtom& SA = *atoms[0];
  TLattice& latt = TXApp::GetInstance().XFile().GetLattice();
  TAsymmUnit& au = latt.GetAsymmUnit();
  vec3d V;
  smatd_list* L;
  TArrayList< AnAssociation3<TCAtom*, vec3d, smatd> > rowData;
  TCAtomPList allAtoms;

  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    bool skip = false;
    for( size_t j=0; j < Exceptions.Count(); j++ )  {
      if( au.GetAtom(i).GetType() == *Exceptions[j] )
      {  skip = true;  break;  }
    }
    if( !skip )  allAtoms.Add(au.GetAtom(i));
  }
  for( size_t i=0; i < allAtoms.Count(); i++ )  {
    if( SA.CAtom().GetId() == allAtoms[i]->GetId() ) {
      L = latt.GetUnitCell().GetInRange(
        SA.ccrd(), allAtoms[i]->ccrd(), r, false);
    }
    else {
      L = latt.GetUnitCell().GetInRange(
        SA.ccrd(), allAtoms[i]->ccrd(), r, true);
    }
    if( !L->IsEmpty() )  {
      for( size_t j=0; j < L->Count(); j++ )  {
        const smatd& m = L->GetItem(j);
        V = m * allAtoms[i]->ccrd() - SA.ccrd();
        au.CellToCartesian(V);
        if( V.Length() == 0 )  // symmetrical equivalent?
          continue;
        rowData.Add(AnAssociation3<TCAtom*, vec3d, smatd>(allAtoms[i], V, m));
      }
    }
    delete L;
  }
  TTTable<TStrList> table(rowData.Count(), rowData.Count()+2); // +SYM + LEN
  table.ColName(0) = SA.GetLabel();
  table.ColName(1) = "SYMM";
  BubbleSorter::Sort(rowData, XLibMacros::TEnviComparator());
  for( size_t i=0; i < rowData.Count(); i++ )  {
    const AnAssociation3<TCAtom*, vec3d, smatd>& rd = rowData[i];
    table.RowName(i) = rd.GetA()->GetLabel();
    table.ColName(i+2) = table.RowName(i);
    if( rd.GetC().r.IsI() && rd.GetC().t.IsNull() )
     table[i][1] = 'I';  // identity
    else {
      table[i][1] = TSymmParser::MatrixToSymmCode(
        xapp.XFile().GetUnitCell().GetSymmSpace(), rd.GetC());
    }
    table[i][0] = olxstr::FormatFloat(2, rd.GetB().Length());
    for( size_t j=0; j < rowData.Count(); j++ )  {
      if( i == j )  { table[i][j+2] = '-'; continue; }
      if( i < j )   { table[i][j+2] = '-'; continue; }
      const AnAssociation3<TCAtom*, vec3d, smatd>& rd1 = rowData[j];
      if( rd.GetB().Length() != 0 && rd1.GetB().Length() != 0 )  {
        double angle = rd.GetB().CAngle(rd1.GetB());
        angle = acos(angle)*180/M_PI;
        table[i][j+2] = olxstr::FormatFloat(1, angle);
      }
      else
        table[i][j+2] = '-';
    }
  }
  TBasicApp::NewLogEntry() <<
    table.CreateTXTList(EmptyString(), true, true, ' ');
}
//..............................................................................
void XLibMacros::funRemoveSE(const TStrObjList &Params, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "could not identify current space group");
    return;
  }
  if( Params[0] == "-1" )  {
    if( !sg->IsCentrosymmetric() )  {
      E.SetRetVal(sg->GetName());
      return;
    }
    smatd_list ml;
    sg->GetMatrices(ml, mattAll^mattInversion);
    TPSTypeList<double, TSpaceGroup*> sglist;
    for( size_t i=0; i < TSymmLib::GetInstance().SGCount(); i++ )  {
      double st=0;
      if( TSymmLib::GetInstance().GetGroup(i).Compare(ml, st) )
        sglist.Add(st, &TSymmLib::GetInstance().GetGroup(i));
    }
    E.SetRetVal(sglist.IsEmpty() ? sg->GetName() : sglist.GetObject(0)->GetName());
  }
}
//..............................................................................
void XLibMacros::funFileName(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    Tmp = TEFile::ExtractFileName(Params[0]);
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      Tmp = TEFile::ExtractFileName(TXApp::GetInstance().XFile().GetFileName());
    else
      Tmp = NoneString;
  }
  E.SetRetVal(TEFile::ChangeFileExt(Tmp, EmptyString()));
}
//..............................................................................
void XLibMacros::funFileExt(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    E.SetRetVal(TEFile::ExtractFileExt(Params[0]));
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() ) {
      E.SetRetVal(
        TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()));
    }
    else
      E.SetRetVal(NoneString);
  }
}
//..............................................................................
void XLibMacros::funFilePath(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    Tmp = TEFile::ExtractFilePath(Params[0]);
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      Tmp = TEFile::ExtractFilePath(TXApp::GetInstance().XFile().GetFileName());
    else
      Tmp = NoneString;
  }
  // see notes in funBaseDir
  TEFile::TrimPathDelimeterI(Tmp);
  E.SetRetVal(Tmp);
}
//..............................................................................
void XLibMacros::funFileDrive(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    E.SetRetVal(TEFile::ExtractFileDrive(Params[0]));
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() ) {
      E.SetRetVal(
        TEFile::ExtractFileDrive(TXApp::GetInstance().XFile().GetFileName()));
    }
    else
      E.SetRetVal(NoneString);
  }
}
//..............................................................................
void XLibMacros::funFileFull(const TStrObjList &Params, TMacroError &E)  {
  if( TXApp::GetInstance().XFile().HasLastLoader() )
    E.SetRetVal(TXApp::GetInstance().XFile().GetFileName());
  else
    E.SetRetVal(NoneString);
}
//..............................................................................
void XLibMacros::funIsFileLoaded(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal(TXApp::GetInstance().XFile().HasLastLoader());
}
//..............................................................................
void XLibMacros::funTitle(const TStrObjList& Params, TMacroError &E)  {
  if( !TXApp::GetInstance().XFile().HasLastLoader() )  {
    if( Params.IsEmpty() )
      E.SetRetVal(olxstr("File is not loaded"));
    else
      E.SetRetVal(Params[0]);
  }
  else
    E.SetRetVal(TXApp::GetInstance().XFile().LastLoader()->GetTitle());
}
//..............................................................................
void XLibMacros::funIsFileType(const TStrObjList& Params, TMacroError &E) {
  if( Params[0].Equalsi("ins") )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() && 
      TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()).Equalsi("ins"));
  }
  else if( Params[0].Equalsi("res") )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() && 
      TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()).Equalsi("res"));
  }
  else if( Params[0].Equalsi("ires") )  {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TIns>());
  }
  else if( Params[0].Equalsi("cif") )  {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TCif>());
  }
  else if( Params[0].Equalsi("p4p") )  {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TP4PFile>());
  }
  else if( Params[0].Equalsi("mol") )  {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TMol>());
  }
  else if( Params[0].Equalsi("xyz") )  {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TMol>());
  }
  else if( Params[0].Equalsi("crs") )  {
    E.SetRetVal(TXApp::GetInstance().CheckFileType<TCRSFile>());
  }
  else
    E.SetRetVal(false);
}
//..............................................................................
void XLibMacros::funBaseDir(const TStrObjList& Params, TMacroError &E)  {
  olxstr tmp(TBasicApp::GetBaseDir());
  // remove the trailing backslash, as it causes a lot of problems with
  // passing parameters to other programs:
  // windows parser assumes that \" is " and does wrong parsing...
  if( !tmp.IsEmpty() )  tmp.SetLength(tmp.Length()-1);
  E.SetRetVal(tmp);
}
//..............................................................................
void XLibMacros::funLSM(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal(TXApp::GetInstance().XFile().GetRM().GetRefinementMethod());
}
//..............................................................................
void XLibMacros::funRun(const TStrObjList& Params, TMacroError &E) {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "this function requires Olex2 processor implementation");
  TStrList allCmds(Params.Text(' '), ">>");
  for( size_t i=0; i < allCmds.Count(); i++ )  {
    op->executeMacroEx(allCmds[i], E);
    if( !E.IsSuccessful() )  {
      if( (i+1) < allCmds.Count() )
        op->print("Not all macros in the provided list were executed", olex::mtError);
      break;
    }
  }
  //E.SetRetVal(E.IsSuccessful());
  // we do not care about result, but nothing should be printed on the html...
  E.SetRetVal(EmptyString());
}
//..............................................................................
void XLibMacros::funIns(const TStrObjList& Params, TMacroError &E)  {
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  olxstr tmp;
  if( Params[0].Equalsi("weight") || Params[0].Equalsi("wght") )  {
    for( size_t j=0; j < rm.used_weight.Count(); j++ )  {
      tmp << rm.used_weight[j];
      if( (j+1) < rm.used_weight.Count() )  tmp << ' ';
    }
    E.SetRetVal(tmp.IsEmpty() ? NAString : tmp);
  }
  else if( Params[0].Equalsi("weight1") )  {
    for( size_t j=0; j < rm.proposed_weight.Count(); j++ )  {
      tmp << rm.proposed_weight[j];
      if( (j+1) < rm.proposed_weight.Count() )  tmp << ' ';
    }
    E.SetRetVal(tmp.IsEmpty() ? NAString : tmp);
  }
  else if( Params[0].Equalsi("L.S.") || Params[0].Equalsi("CGLS")  )  {
    for( size_t i=0; i < rm.LS.Count(); i++ )  {
      tmp << rm.LS[i];
      if( (i+1) < rm.LS.Count() )  tmp << ' ';
    }
    E.SetRetVal(rm.LS.Count() == 0 ? NAString : tmp);
  }
  else if( Params[0].Equalsi("ls") )
    E.SetRetVal(rm.LS.Count() == 0 ? NAString : olxstr(rm.LS[0]));
  else if( Params[0].Equalsi("plan") )  {
    for( size_t i=0; i < rm.PLAN.Count(); i++ )  {
      tmp << ((i < 1) ? olx_round(rm.PLAN[i]) : rm.PLAN[i]);
      if( (i+1) < rm.PLAN.Count() )  tmp << ' ';
    }
    E.SetRetVal(rm.PLAN.Count() == 0 ? NAString : tmp);
  }
  else if( Params[0].Equalsi("qnum") )
    E.SetRetVal(rm.PLAN.Count() == 0 ? NAString : olxstr(rm.PLAN[0]));
  else  {
    TIns& I = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
    if( Params[0].Equalsi("R1") )
      E.SetRetVal(I.GetR1() < 0 ? NAString : olxstr(I.GetR1()));
    if( !I.InsExists(Params[0]) )  {
      E.SetRetVal(NAString);
      return;
    }
    //  xapp.XFile().UpdateAsymmUnit();
    //  I->UpdateParams();

    TInsList* insv = I.FindIns(Params[0]);
    if( insv != 0 )
      E.SetRetVal(insv->Text(' '));
    else
      E.SetRetVal(EmptyString());
  }
}
//..............................................................................
void XLibMacros::funSSM(const TStrObjList& Params, TMacroError &E) {
  RefinementModel& rm  = TXApp::GetInstance().XFile().GetRM();
  if( rm.GetSolutionMethod().IsEmpty() && Params.Count() == 1 )
    E.SetRetVal(Params[0]);
  else
    E.SetRetVal(rm.GetSolutionMethod());
}
//..............................................................................
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
olxstr XLibMacros_funSGNameToHtmlX(const olxstr& name)  {
  TStrList toks(name, ' ');
  olxstr res;
  for( size_t i=0; i < toks.Count(); i++ )  {
    if( toks[i].Length() >= 2 && XLibMacros_funSGNameIsNextSub(toks[i], 0) )
      res << toks[i].CharAt(0) << "<sub>" << toks[i].CharAt(1) << "</sub>" << toks[i].SubStringFrom(2);
    else
      res << toks[i];
  }
  return res;
}
void XLibMacros::funSG(const TStrObjList &Cmds, TMacroError &E)  {
  TSpaceGroup* sg = NULL;
  try  { sg = &TXApp::GetInstance().XFile().GetLastLoaderSG();  }
  catch(...)  {}
  if( sg != NULL )  {
    olxstr Tmp;
    if( Cmds.IsEmpty() )  {
      Tmp = sg->GetName();
      if( !sg->GetFullName().IsEmpty() )  {
        Tmp << " (" << sg->GetFullName() << ')';
      }
      Tmp << " #" << sg->GetNumber();
    }
    else  {
      Tmp = Cmds[0];
      Tmp.Replace("%#", olxstr(sg->GetNumber()) ).\
        Replace("%n", sg->GetName()).\
        Replace("%N", sg->GetFullName()).\
        Replace("%HS", sg->GetHallSymbol()).\
        Replace("%s", sg->GetBravaisLattice().GetName());
        Tmp.Replace("%H", XLibMacros_funSGNameToHtmlX(sg->GetFullName()));
        if( sg->GetName() == olxstr::DeleteChars(sg->GetFullName(), ' ') ) 
          Tmp.Replace("%h", XLibMacros_funSGNameToHtmlX(sg->GetFullName()));
        else
          Tmp.Replace("%h", XLibMacros_funSGNameToHtml(sg->GetName()));
    }
    E.SetRetVal(Tmp);
  }
  else  {
    E.SetRetVal(NAString);
//    E.ProcessingError(__OlxSrcInfo, "could not find space group for the file");
    return;
  }
}
//..............................................................................
void XLibMacros::funSGS(const TStrObjList &Cmds, TMacroError &E)  {
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
//..............................................................................
void XLibMacros::funHKLSrc(const TStrObjList& Params, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  if( Params.Count() == 1 )
    xapp.XFile().GetRM().SetHKLSource(Params[0]);
  else  {
    olxstr fn = xapp.XFile().GetRM().GetHKLSource();
    if( TEFile::Exists(fn) )  {  // check the format...
      TEFile f(fn, "rb");
      if( !THklFile::IsHKLFileLine(f.ReadLine()) )  {
        f.Close();
        fn = TEFile::AddPathDelimeter(TEFile::ExtractFilePath(fn)) << MD5::Digest(fn) << ".hkl";
        if( !TEFile::Exists(fn) )  {
          TBasicApp::NewLogEntry() << "Creating HKL file...";
          THklFile::SaveToFile(fn, xapp.XFile().GetRM().GetReflections());
        }
      }
    }
    E.SetRetVal(fn);
  }
}
//..............................................................................
void XLibMacros::macCif2Doc(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
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
    Error.ProcessingError(__OlxSrcInfo, "template for CIF does not exist: ") << Cmds[0];
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

  SL.LoadFromFile(TN);
  Dic.LoadFromFile(CifDictionaryFile);
  for( size_t i=0; i < SL.Count(); i++ )
    Cif->ResolveParamsFromDictionary(Dic, SL[i], '%', &XLibMacros::CifResolve);
  TUtf8File::WriteLines(RF, SL, false);
  TBasicApp::NewLogEntry(logInfo) << "Document name: " << RF;
}
//..............................................................................
void XLibMacros::macCif2Tab(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  olxstr CifTablesFile = Options.FindValue('t', "tables.xlt");
  if (!TEFile::IsAbsolutePath(CifTablesFile))
    CifTablesFile = xapp.GetCifTemplatesDir() + CifTablesFile;
  if( !TEFile::Exists(CifTablesFile) )  {
    Error.ProcessingError(__OlxSrcInfo, "tables definition file is not found");
    return;
  }

  olxstr CifDictionaryFile(xapp.GetCifTemplatesDir() + "cifindex.dat");
  if( Cmds.IsEmpty() )  {
    TDataFile DF;
    TStrList SL;
    TDataItem *Root;
    olxstr Tmp;
    DF.LoadFromXLFile(CifTablesFile, &SL);
    Root = DF.Root().FindItemi("Cif_Tables");
    if( Root != NULL )  {
      xapp.NewLogEntry(logInfo) << "Found table definitions:";
      for( size_t i=0; i < Root->ItemCount(); i++ )  {
        Tmp = "Table "; 
        Tmp << Root->GetItem(i).GetName()  << '(' << " #" << (int)i+1 <<  "): caption <---";
        xapp.NewLogEntry(logInfo) << Tmp;
        xapp.NewLogEntry(logInfo) << Root->GetItem(i).GetFieldValueCI("caption");
        xapp.NewLogEntry(logInfo) << "--->";
      }
    }
    else  {
      Error.ProcessingError(__OlxSrcInfo, "tables definition node is not found");
      return;
    }
    return;
  }
  TCif *Cif, Cif1;

  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "cif");
    if( TEFile::Exists(cifFN ) )  {
      Cif1.LoadFromFile(cifFN);
    }
    else
        throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif1;
  }

  TStrList SL, SL1, Dic, 
    CLA, // cell attributes
    THA;  // header (th) attributes
  TDataFile DF;
  TTTable<TStrList> DT;
  DF.LoadFromXLFile(CifTablesFile, NULL);
  Dic.LoadFromFile(CifDictionaryFile);
  olxstr RF(Options.FindValue('n'));
  if( RF.IsEmpty() )  {
    RF = TEFile::ChangeFileExt(Cif->GetFileName(), EmptyString());
    RF << "_tables";
  }
  TDataItem* Root = DF.Root().FindItemi("Cif_Tables");
  if (Root == NULL) {
    Error.ProcessingError(__OlxSrcInfo, "wrong root");
    return;
  }
  { // guess format
    const olxstr str_format = Root->GetFieldValue("format", "html");
    const bool html = str_format.Equalsi("html");
    RF = TEFile::ChangeFileExt(RF, html ? "html" : "tex");
  }
  smatd_list SymmList;
  size_t tab_count = 1;
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    TDataItem* TD = NULL;
    if( Cmds[i].IsNumber() )  {
      size_t index = Cmds[i].ToSizeT();
      if( index < Root->ItemCount() )
        TD = &Root->GetItem(index);
    }
    if( TD == NULL  )
      TD = Root->FindItem(Cmds[i]);
    if( TD == NULL )  {
      xapp.NewLogEntry(logWarning) << "Could not find table definition: " << Cmds[i];
      continue;
    }
    if( TD->GetName().Equalsi("footer") || TD->GetName().Equalsi("header") )  {
      olxstr fn = TD->GetFieldValue("source");
      if( fn.IndexOf('$') != InvalidIndex )
        ProcessExternalFunction(fn);
      if( !TEFile::IsAbsolutePath(fn) )
        fn = xapp.GetCifTemplatesDir() + fn;
      SL1.LoadFromFile(fn);
      for( size_t j=0; j < SL1.Count(); j++ )  {
        Cif->ResolveParamsFromDictionary(Dic, SL1[j], '%', &XLibMacros::CifResolve);
        SL.Add(SL1[j]);
      }
      continue;
    }
    if( Cif->CreateTable(TD, DT, SymmList) )  {
      olxstr Tmp = "Table ";
      Tmp << ++tab_count << ' ' << TD->GetFieldValueCI("caption");
      Tmp.Replace("%DATA_NAME%", Cif->GetDataName());
      if( Tmp.IndexOf('$') != InvalidIndex )
        ProcessExternalFunction(Tmp);
      // attributes of the row names ...
      CLA.Clear();
      THA.Clear();
      CLA.Add(TD->GetFieldValue("tha"));
      THA.Add(TD->GetFieldValue("tha"));
      for( size_t j=0; j < TD->ItemCount(); j++ )  {
        CLA.Add(TD->GetItem(j).GetFieldValue("cola"));
        THA.Add(TD->GetItem(j).GetFieldValue("tha"));
      }

      olxstr footer;
      for( size_t i=0; i < SymmList.Count(); i++ )  {
        footer << "<sup>" << (i+1) << "</sup>" <<
          TSymmParser::MatrixToSymmEx(SymmList[i]);
        if( (i+1) < SymmList.Count() )
          footer << "; ";
      }
      if( tab_count > 1 )
        SL.Add("<p>&nbsp;</p>");
      DT.CreateHTMLList(
        SL,
        Tmp,
        footer,
        true, false,
        TD->GetFieldValue("tita"),  // title paragraph attributes
        TD->GetFieldValue("foota"),  // footer paragraph attributes
        TD->GetFieldValue("taba"),  //const olxstr& tabAttr,
        TD->GetFieldValue("rowa"),  //const olxstr& rowAttr,
        THA, // header attributes
        CLA, // cell attributes,
        true,
        TD->GetFieldValue("coln", "1").ToInt(),
        TD->GetFieldValue("colsa")
      ); //bool Format) const  {
      //DT.CreateHTMLList(SL, Tmp, true, false, true);
    }
  }
  TUtf8File::WriteLines(RF, SL, false);
  TBasicApp::NewLogEntry(logInfo) << "Tables file: " << RF;
}
//..............................................................................
olxstr XLibMacros::CifResolve(const olxstr& func)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )  return func;
  olxstr rv;
  if( op->executeFunction(func, rv) )
    return rv;
  return func;
}
//..............................................................................
bool XLibMacros::ProcessExternalFunction(olxstr& func)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )  return false;
  olxstr rv;
  if( op->executeFunction(func, rv) )  {
    func = rv;
    return true;
  }
  return false;
}
//..............................................................................
void XLibMacros::macCifMerge(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TCif *Cif, Cif2;
  cif_dp::TCifDP src;
  const size_t _Translation_count = 5;
  static const olxstr _Translations[2*_Translation_count] = {
    "_symmetry_cell_setting", "_space_group_crystal_system",
    "_symmetry_space_group_name_Hall", "_space_group_name_Hall",
    "_symmetry_space_group_name_H-M", "_space_group_name_H-M_alt",
    "_symmetry_Int_Tables_number-M", "_space_group_IT_number",
    "_diffrn_reflns_av_sigmaI/netI", "_diffrn_reflns_av_unetI/netI"
  };
  TTypeList<AnAssociation2<olxstr,olxstr> > Translations;
  olxstr CifCustomisationFN(xapp.GetCifTemplatesDir() + "customisation.xlt");
  if( TEFile::Exists(CifCustomisationFN) )  {
    try  {
      TDataFile df;
      if( !df.LoadFromXLFile(CifCustomisationFN) )  {
        Error.ProcessingError(__OlxSrcInfo, "falied to load CIF customisation file");
        return;
      }
      TDataItem& di = df.Root().FindRequiredItem("cif_customisation").FindRequiredItem("translation");
      for( size_t i=0; i < di.ItemCount(); i++ )
        Translations.AddNew(di.GetItem(i).GetRequiredField("from"), di.GetItem(i).GetRequiredField("to"));
    }
    catch(const TExceptionBase& e)  {
      throw TFunctionFailedException(__OlxSourceInfo, e);
    }
  }
  else  {
    for( size_t i=0; i < _Translation_count; i++ )
      Translations.AddNew(_Translations[i*2], _Translations[i*2+1]);
  }
  TStrList _loop_names_to_skip("_atom_site;_geom;_space_group", ';');
  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "cif");
    if( TEFile::Exists(cifFN) )
      Cif2.LoadFromFile(cifFN);
    else
      throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif2;
  }
  // normalise
  for( size_t i=0; i < Translations.Count(); i++ )
    Cif->Rename(Translations[i].GetA(), Translations[i].GetB());
  // update the atom_site loop and H treatment if AU match
  if( Options.Contains('u') )  {
    if( xapp.CheckFileType<TCif>() )  {
      TBasicApp::NewLogEntry() <<
        "Cannot update the CIF - the real refinement model is unknown";
    }
    else  {
      const TAsymmUnit &au = xapp.XFile().GetAsymmUnit();
      const TAsymmUnit &tau = Cif->GetAsymmUnit();
      bool match = (tau.AtomCount() <= au.AtomCount()), has_parts = false;
      if( match )  {
        size_t ind=0;
        for( size_t i=0; i < tau.AtomCount(); i++, ind++ )  {
          TCAtom& ta = tau.GetAtom(i);
          while( ind < au.AtomCount() &&
            (au.GetAtom(ind).IsDeleted() || au.GetAtom(ind).GetType() == iQPeakZ) )
          {
            ind++;
          }
          if( ind >= au.AtomCount() )  {
            match = false;
            break;
          }
          TCAtom &a = au.GetAtom(ind);
          if( !a.GetLabel().Equalsi(ta.GetLabel()) ||
            a.ccrd().QDistanceTo(ta.ccrd()) > 1e-3 )
          {
            match = false;
            break;
          }
          if( a.GetPart() != 0 )
            has_parts = true;
        }
      }
      if( !match )  {
        TBasicApp::NewLogEntry() << "Could not update the atom information loop - "
          "the asymmetric units mismatch";
      }
      else  {
        cetTable* tab = Cif->FindLoop("_atom_site");
        if( tab == NULL || tab->RowCount() != tau.AtomCount() )  {
          TBasicApp::NewLogEntry() << "Could not locate the atom_site loop or"
           " its content mismatches the asymmetric unit";
        }
        else  {
          size_t rf_ind = tab->ColIndex("_atom_site_refinement_flags_posn");
          if( rf_ind == InvalidIndex )  {
            tab->AddCol("_atom_site_refinement_flags_posn");
            rf_ind = tab->ColCount()-1;
          }
          size_t dg_ind = tab->ColIndex("_atom_site_disorder_group");
          if( dg_ind == InvalidIndex && has_parts )  {
            tab->AddCol("_atom_site_disorder_group");
            dg_ind = tab->ColCount()-1;
          }
          TIntList h_t;
          size_t ri=0;
          for( size_t i=0; i < au.AtomCount(); i++, ri++ )  {
            while( i < au.AtomCount() &&
              (au.GetAtom(i).IsDeleted() || au.GetAtom(i).GetType() == iQPeakZ) )
            {
              i++;
            }
            // last condition must not ever happen
            if( i >= au.AtomCount() || ri >= tab->RowCount() )
              break;
            TCAtom &a = au.GetAtom(i);
            if( a.GetType() == iHydrogenZ )  {
              int& h = h_t.Add(0);
              if( a.GetUisoOwner() != NULL )  {  // u constrained
                h |= 0x0001;
              }
              if( a.GetParentAfixGroup() != NULL &&  // coordinates constrained
                a.GetParentAfixGroup()->GetAfix() > 0 )
              {
                if( !a.GetParentAfixGroup()->IsRefinable() )
                  h |= 0x0002;
                else
                  h |= 0x0004;
              }
            }
            olxstr f;
            if( a.GetParentAfixGroup() != NULL )  {
              if( a.GetParentAfixGroup()->IsRefinable() )  {
                f << 'G';
              }
              else if( a.GetParentAfixGroup()->IsRiding() )
                f << 'R';
            }
            if( a.GetDegeneracy() != 1 )
              f << 'S';
            if( f.IsEmpty() )
              tab->Set(ri, rf_ind, new cetString('.'));
            else
              tab->Set(ri, rf_ind, new cetString(f));
            if( has_parts )  {
              if( a.GetPart() == 0 )
                tab->Set(ri, dg_ind, new cetString('.'));
              else
                tab->Set(ri, dg_ind, new cetString((int)a.GetPart()));
            }
          }
          if( h_t.IsEmpty() )
            Cif->SetParam("_refine_ls_hydrogen_treatment", "undef", false);
          else  {
            int v = h_t[0];
            bool all_same = true;
            for( size_t i=1; i < h_t.Count(); i++ )  {
              if( h_t[i] != v )  {
                all_same = false;
                break;
              }
            }
            if( all_same )  {
              if( v == 1 )
                Cif->SetParam("_refine_ls_hydrogen_treatment", "refxyz", false);
              else if( v == 2 )
                Cif->SetParam("_refine_ls_hydrogen_treatment", "refU", false);
              else if( v == 4 )
                Cif->SetParam("_refine_ls_hydrogen_treatment", "noref", false);
              else
                Cif->SetParam("_refine_ls_hydrogen_treatment", "constr", false);
            }
            else
              Cif->SetParam("_refine_ls_hydrogen_treatment", "mixed", false);
          }
        }
      }
    }
  }
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    try {
      IInputStream *is = TFileHandlerManager::GetInputStream(Cmds[i]);
      if( is == NULL )  {
        TBasicApp::NewLogEntry(logError) << "Could not find file: " << Cmds[i];
        continue;
      }
      TStrList sl;
      sl.LoadFromTextStream(*is);
      delete is;
      src.LoadFromStrings(sl);
    }
    catch( ... )  {    }  // most like the cif does not have cell, so pass it
    if( src.Count() == 0 )  continue;
    // normalise
    for( size_t i=0; i < Translations.Count(); i++ )
      src[0].Rename(Translations[i].GetA(), Translations[i].GetB());
    for( size_t j=0; j < src[0].param_map.Count(); j++ )  {
      const cif_dp::ICifEntry& e = *src[0].param_map.GetValue(j);
      if( !e.HasName() )  continue;
      bool skip = false;
      for( size_t k=0; k < _loop_names_to_skip.Count(); k++ )  {
        olxstr i_name = e.GetName();
        if(i_name.StartsFromi(_loop_names_to_skip[k]) &&
          (i_name.Length()>_loop_names_to_skip[k].Length() &&
           i_name.CharAt(_loop_names_to_skip[k].Length()) == '_'))
        {
          skip = true;
          break;
        }
      }
      if( !skip )
        Cif->SetParam(src[0].param_map.GetKey(j), e);
    }
  }
  // generate moiety string if does not exist
  const olxstr cif_moiety = Cif->GetParamAsString("_chemical_formula_moiety");
  if (cif_moiety.IsEmpty() || cif_moiety == '?') {
    Cif->SetParam("_chemical_formula_moiety",
      xapp.XFile().GetLattice().CalcMoiety(), true);
  }
  Cif->SetParam("_cell_formula_units_Z",
    xapp.XFile().GetAsymmUnit().GetZ(), false);
  TSpaceGroup &sg = TSymmLib::GetInstance().FindSG(Cif->GetAsymmUnit());
  Cif->SetParam("_space_group_crystal_system",
    sg.GetBravaisLattice().GetName().ToLowerCase(), true);
  Cif->SetParam("_space_group_name_Hall", sg.GetHallSymbol(), true);
  Cif->SetParam("_space_group_name_H-M_alt", sg.GetFullName(), true);
  Cif->SetParam("_space_group_IT_number", sg.GetNumber(), false);
  if( !sg.IsCentrosymmetric() &&
    !Cif->ParamExists("_chemical_absolute_configuration") )
  {
    bool flack_used = false;
    if( xapp.CheckFileType<TIns>() )  {
      const TIns& ins = xapp.XFile().GetLastLoader<TIns>();
      const TLst& lst = ins.GetLst();
      if( lst.IsLoaded() && lst.HasFlack() )  {
        TEValue<double> fv = lst.Flack();
        if( fv.GetE() < 0.2 )  {
          Cif->SetParam("_chemical_absolute_configuration", "ad", false);
          flack_used = true;
        }
      }
    }
    if( !flack_used )
      Cif->SetParam("_chemical_absolute_configuration", "unk", false);
  }
  Cif->SaveToFile(Cif->GetFileName());
}
//..............................................................................
void XLibMacros::macCifExtract(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr Dictionary = Cmds[0];
  if( !TEFile::Exists(Dictionary) )  {  // check if the dictionary exists
    Dictionary = xapp.GetCifTemplatesDir();  Dictionary << Cmds[0];
    if( !TEFile::Exists(Dictionary) )  {
      Error.ProcessingError(__OlxSrcInfo, "dictionary file does not exists");
      return;
    }
  }
  TCif In,  Out, *Cif, Cif1;
  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "cif");
    if( TEFile::Exists(cifFN) )  {
      Cif1.LoadFromFile(cifFN);
    }
    else
      throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif1;
  }
  // dictionary does not have cell etc - so it should fail to initialise
  try  {  In.LoadFromFile(Dictionary);  }
  catch( TExceptionBase& )  {}

  for( size_t i=0; i < In.ParamCount(); i++ )  {
    cif_dp::ICifEntry* CifData = Cif->FindParam<cif_dp::ICifEntry>(In.ParamName(i));
    if( CifData != NULL )
      Out.SetParam(In.ParamName(i), *CifData);
  }
  try  {  Out.SaveToFile(Cmds[1]);  }
  catch( TExceptionBase& )  {
    Error.ProcessingError(__OlxSrcInfo, "could not save file: ") << Cmds[1];
    return;
  }
}
//..............................................................................
void XLibMacros::macCifCreate(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  VcoVContainer vcovc(xapp.XFile().GetAsymmUnit());
  try  {
    olxstr src_mat = xapp.InitVcoV(vcovc);
    xapp.NewLogEntry() << "Using " << src_mat << " matrix for the calculation";
  }
  catch(TExceptionBase& e)  {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }

  TAsymmUnit& _au = xapp.XFile().GetAsymmUnit();
  for( size_t i=0; i < _au.AtomCount(); i++ )  {
    TCAtom& a = _au.GetAtom(i);
    if( a.GetEllipsoid() != NULL )  {
      TEllipsoid& elp = *a.GetEllipsoid();
      a.SetUiso(elp.GetUeq());
      if( a.GetUisoEsd() == 0 )  {
        double esd = 0;
        for( int j=0; j < 3; j++ )
          esd += olx_sqr(elp.GetEsd(j));
        a.SetUisoEsd(sqrt(esd)/3.);
      }
    }
    else if( a.GetType() == iHydrogenZ && a.GetUisoEsd() == 0 )  {
      long val = olx_round(a.GetUiso()*100000);
      a.SetUiso((double)val/100000);
    }
  }
  TCif cif;
  cif.Adopt(xapp.XFile());
  TAsymmUnit& au = cif.GetAsymmUnit();
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetType() == iQPeakZ )
      au.GetAtom(i).SetDeleted(true);
  }
  TLattice latt(*(new SObjectProvider));
  latt.GetAsymmUnit().SetRefMod(&xapp.XFile().GetRM());
  latt.GetAsymmUnit().Assign(xapp.XFile().GetAsymmUnit());
  for( size_t i=0; i < latt.GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& a = latt.GetAsymmUnit().GetAtom(i);
    if( a.IsDetached() )
      a.SetDetached(false);
    if( a.IsMasked() )
      a.SetMasked(false);
  }
  latt.GetAsymmUnit()._UpdateConnInfo();
  latt.GetAsymmUnit().DetachAtomType(iQPeakZ, true);
  latt.Init();
  latt.CompaqAll();
  ASObjectProvider& objects = latt.GetObjects();

  latt.GrowFragments(false, NULL);

  cif_dp::cetTable& bonds = cif.AddLoopDef(
    "_geom_bond_atom_site_label_1,_geom_bond_atom_site_label_2,_geom_bond_distance,"
    "_geom_bond_site_symmetry_2,_geom_bond_publ_flag");
  for( size_t i=0; i < objects.bonds.Count(); i++ )  {
    TSBond& b = objects.bonds[i];
    if( b.A().GetType().GetMr() < 3 || b.A().IsDeleted() )  {
      b.SetTag(0);
      continue;
    }
    if( b.B().GetType().GetMr() < 3 || b.B().IsDeleted() )  {
      b.SetTag(0);
      continue;
    }
    b.SetTag(-1);
  }
  for( size_t i=0; i < objects.atoms.Count(); i++ )  {
    TSAtom& a = objects.atoms[i];
    if( a.GetType().GetMr()  < 3 || a.IsDeleted() || !a.IsAUAtom() )  continue;
    for( size_t j=0; j < a.BondCount(); j++ )  {
      TSBond& b = a.Bond(j);
      if( b.GetTag() == 0 || !b.A().IsAUAtom() )  continue;
      b.SetTag(0);
      cif_dp::CifRow& row = bonds.AddRow();
      row.Set(0, new AtomCifEntry(b.A().CAtom()));
      row.Set(1, new AtomCifEntry(b.B().CAtom()));
      row[2] = new cetString(vcovc.CalcDistance(b.A(), b.B()).ToString());
      if( !b.B().IsAUAtom() )
        row[3] = new cetString(TSymmParser::MatrixToSymmCode(xapp.XFile().GetUnitCell().GetSymmSpace(),
          b.B().GetMatrix(0)));
      else
        row[3] = new cetString('.');
      row[4] = new cetString('?');
    }
  }
  cif_dp::cetTable& angles = cif.AddLoopDef(
    "_geom_angle_atom_site_label_1,_geom_angle_atom_site_label_2,_geom_angle_atom_site_label_3,"
    "_geom_angle,_geom_angle_site_symmetry_1,_geom_angle_site_symmetry_3,_geom_angle_publ_flag");
  for( size_t i=0; i < objects.atoms.Count(); i++ )  {
    TSAtom& a = objects.atoms[i];
    if( a.GetType().GetMr()  < 3 || a.IsDeleted() || !a.IsAUAtom() )  continue;
    for( size_t j=0; j < a.NodeCount(); j++ )  {
      TSAtom& b = a.Node(j);
      if( b.IsDeleted() || b.GetType().GetMr() < 3 )
        continue;
      for( size_t k=j+1; k < a.NodeCount(); k++ )  {
        TSAtom& c = a.Node(k);      
        if( c.IsDeleted() || c.GetType().GetMr() < 3 )
          continue;
        TSAtom& _b = (b.CAtom().GetId() <= c.CAtom().GetId() ? b : c);
        TSAtom& _c = (b.CAtom().GetId() > c.CAtom().GetId() ? b : c);
        cif_dp::CifRow& row = angles.AddRow();
        row.Set(0, new AtomCifEntry(_b.CAtom()));
        row.Set(1, new AtomCifEntry(a.CAtom()));
        row.Set(2, new AtomCifEntry(_c.CAtom()));
        row[3] = new cetString(vcovc.CalcAngleA(_b, a, _c).ToString());
        if( !_b.IsAUAtom() )
          row[4] = new cetString(TSymmParser::MatrixToSymmCode(xapp.XFile().GetUnitCell().GetSymmSpace(),
            _b.GetMatrix(0)));
        else
          row[4] = new cetString('.');
        if( !_c.IsAUAtom() )
          row[5] = new cetString(TSymmParser::MatrixToSymmCode(xapp.XFile().GetUnitCell().GetSymmSpace(),
            _c.GetMatrix(0)));
        else
          row[5] = new cetString('.');
        row[6] = new cetString('?');
      }
    }
  }
  RefinementModel& rm = xapp.XFile().GetRM();
  if( rm.InfoTabCount() != 0 )  {
    cif_dp::cetTable& hbonds = cif.AddLoopDef(
      "_geom_hbond_atom_site_label_D,_geom_hbond_atom_site_label_H,_geom_hbond_atom_site_label_A,"
      "_geom_hbond_distance_DH,_geom_hbond_distance_HA,_geom_hbond_distance_DA,"
      "_geom_hbond_angle_DHA,_geom_hbond_site_symmetry_A");
    smatd I;
    I.I().SetId(0);
    TAtomEnvi envi;
    for( size_t i=0; i < rm.InfoTabCount(); i++ )  {
      InfoTab& it = rm.GetInfoTab(i);
      if( it.GetType() != infotab_htab || !it.IsValid() )  continue;
      TGroupCAtom *d = NULL, *a = NULL;
      for( size_t j=0; j < it.Count(); j++ )  {
        if( it.GetAtom(j).GetAtom()->IsDeleted() )  continue;
        if( d == NULL )
          d = &it.GetAtom(j);
        else  {
          a = &it.GetAtom(j);
          break;
        }
      }
      TSAtom* dsa = xapp.XFile().GetLattice().FindSAtom(*d->GetAtom());
      if( dsa == NULL )  continue;  //eh?
      envi.Clear();
      xapp.XFile().GetUnitCell().GetAtomEnviList(*dsa, envi);
      for( size_t j=0; j < envi.Count(); j++ )  {
        if( envi.GetType(j) != iHydrogenZ)  continue;
        // check if the D-H-A angle makes sense...
        double ang = (au.Orthogonalise(d->ccrd())-envi.GetCrd(j)).CAngle(
          au.Orthogonalise(a->ccrd())-envi.GetCrd(j));
        if( ang >= 0 )  // <= 90
          continue;
        CifRow& row = hbonds.AddRow();
        row.Set(0, new AtomCifEntry(*d->GetAtom()));
        row.Set(1, new AtomCifEntry(envi.GetCAtom(j)));
        row.Set(2, new AtomCifEntry(*a->GetAtom()));
        TSAtom da(NULL), aa(NULL);
        da.CAtom(*d->GetAtom());
        da.AddMatrix(&I);
        au.CellToCartesian(da.ccrd(), da.crd());
        aa.CAtom(*a->GetAtom());
        smatd am;
        if( a->GetMatrix() == NULL )  {
          am.I();
          am.SetId(0);
        }
        else  {
          am = *a->GetMatrix();
          xapp.XFile().GetUnitCell().InitMatrixId(am);
        }
        aa.AddMatrix(&am);
        aa.ccrd() = am*aa.ccrd();
        au.CellToCartesian(aa.ccrd(), aa.crd());
        row[3] = new cetString(olxstr::FormatFloat(2, envi.GetCrd(j).DistanceTo(da.crd())));
        row[4] = new cetString(olxstr::FormatFloat(2, envi.GetCrd(j).DistanceTo(aa.crd())));
        row[5] = new cetString(vcovc.CalcDistance(da, aa).ToString());
        row[6] = new cetString(olxstr::FormatFloat(1, olx_angle(da.crd(), envi.GetCrd(j), aa.crd())));
        if( a->GetMatrix() == NULL )
          row[7] = new cetString('.');
        else
          row[7] = new cetString(
            TSymmParser::MatrixToSymmCode(xapp.XFile().GetUnitCell().GetSymmSpace(),
              aa.GetMatrix(0)));
      }
    }
  }
  cif.SaveToFile(TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "cif"));
}
//..............................................................................
void XLibMacros::macFcfCreate(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  const int list_n = Cmds[0].ToInt();
  const olxstr fn = (Cmds.Count() > 1 ? Cmds.Text(' ', 1) :
    TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fcf"));
  RefinementModel::HklStat ms;
  TUnitCell::SymmSpace sp = xapp.XFile().GetUnitCell().GetSymmSpace();
  TRefList refs;
  olxstr col_names = "_refln_index_h,_refln_index_k,_refln_index_l,";
  if( list_n == 4 )  {
    ms = xapp.XFile().GetRM().GetRefinementRefList<
      TUnitCell::SymmSpace,RefMerger::ShelxMerger>(sp, refs);
    col_names << "_refln_F_squared_calc,_refln_F_squared_meas,"
      "_refln_F_squared_sigma,_refln_observed_status";
  }
  else if( list_n == 3 )  {
    ms = xapp.XFile().GetRM().GetFourierRefList<
      TUnitCell::SymmSpace,RefMerger::ShelxMerger>(sp, refs);
    col_names << "_refln_F_meas,_refln_F_sigma,_refln_A_calc,_refln_B_calc";
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, olxstr("unsupported list number: ") << list_n);
    return;
  }
  TArrayList<compd> F;
  F.SetCount(refs.Count());
  //xapp.CalcSF(refs, F);
  //SFUtil::CalcSF(xapp.XFile(), refs, F, xapp.XFile().GetRM().GetMERG() != 4);
  SFUtil::CalcSF(xapp.XFile(), refs, F);
  double scale_k = 1, scale_a = 0;
  olxstr scale_str = Options.FindValue("scale", "external");
  if( scale_str.Equalsi("external") )
    scale_k = 1./olx_sqr(xapp.XFile().GetRM().Vars.GetVar(0).GetValue());
  else if( scale_str.Equalsi("simple") )
    scale_k = SFUtil::CalcF2Scale(F, refs);
  else if( scale_str.Equalsi("regression") )
    SFUtil::CalcF2Scale(F, refs, scale_k, scale_a);
  else  {
    Error.ProcessingError(__OlxSrcInfo, olxstr("unsupported scale: ") << scale_str);
    return;
  }
  TCifDP fcf_dp;
  CifBlock& cif_data = fcf_dp.Add(
    TEFile::ExtractFileName(fn).Replace(' ', EmptyString()));
  cif_data.Add(new cetNamedString("_olex2_title",
    xapp.XFile().LastLoader()->GetTitle()));
  cif_data.Add(new cetNamedString("_shelx_refln_list_code", list_n));

  const TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  cif_data.Add(new cetNamedString("_cell_length_a",
    TEValueD(au.GetAxes()[0], au.GetAxisEsds()[0]).ToString()));
  cif_data.Add(new cetNamedString("_cell_length_b",
    TEValueD(au.GetAxes()[1], au.GetAxisEsds()[1]).ToString()));
  cif_data.Add(new cetNamedString("_cell_length_c",
    TEValueD(au.GetAxes()[2], au.GetAxisEsds()[2]).ToString()));
  cif_data.Add(new cetNamedString("_cell_angle_alpha",
    TEValueD(au.GetAngles()[0], au.GetAngleEsds()[0]).ToString()));
  cif_data.Add(new cetNamedString("_cell_angle_beta",
    TEValueD(au.GetAngles()[1], au.GetAngleEsds()[1]).ToString()));
  cif_data.Add(new cetNamedString("_cell_angle_gamma",
    TEValueD(au.GetAngles()[2], au.GetAngleEsds()[2]).ToString()));

  const TUnitCell& uc = xapp.XFile().GetUnitCell();
  cetTable* sym_tab = new cetTable(
    "_space_group_symop_id,_space_group_symop_operation_xyz");
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    CifRow& r = sym_tab->AddRow();
    r[0] = new cetString(i+1);
    r[1] = new cetString(TSymmParser::MatrixToSymmEx(uc.GetMatrix(i)));
  }
  cif_data.Add(sym_tab);

  cetTable* ref_tab = new cetTable(col_names);
  cif_data.Add(ref_tab);

  for( size_t i=0; i < refs.Count(); i++ )  {
    TReflection& r = refs[i];
    const double Fo2 = r.GetI()*scale_k + scale_a;
    const double sigFo2 = r.GetS()*scale_k;
    CifRow& row = ref_tab->AddRow();
    row[0] = new cetString(r.GetH());
    row[1] = new cetString(r.GetK());
    row[2] = new cetString(r.GetL());
    if( list_n == 3 )  {
      double Fo, s_m;
      //http://www.iucr.org/__data/iucr/cif/software/xtal/xtal372htmlman/html/refcal-desc.html
      if( Fo2 <= 0 )  {
        Fo = 0;
        s_m = sqrt(sigFo2); // xtal 3.7.2
      }
      else  {
        Fo = sqrt(Fo2);
        s_m = sqrt(Fo2+sigFo2) - Fo; // xtal 3.7.2
        //s_m = sigFo2/(Fo + sqrt(sigFo2+Fo2));  // sxtal 3.7
        //s_m = Fo2 < sigFo2 ? sigFo2 : sigFo2/(2*Fo); // crystals
      }
      row[3] = new cetString(Fo);
      row[4] = new cetString(s_m);
      row[5] = new cetString(F[i].Re());
      row[6] = new cetString(sp.IsCentrosymmetric() ? 0.0 : F[i].Im());
    }
    else if( list_n == 4 )  {
      row[3] = new cetString(olxstr::FormatFloat(2, F[i].qmod()));
      row[4] = new cetString(olxstr::FormatFloat(2, Fo2));
      row[5] = new cetString(olxstr::FormatFloat(2, sigFo2));
      row[6] = new cetString('o');
    }
  }
  TCStrList(fcf_dp.SaveToStrings()).SaveToFile(fn);
}
//..............................................................................
struct XLibMacros_StrF  {
  int h, k, l;
  double ps;
  TEComplex<double> v;
};
void XLibMacros::macVoidE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
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
//    TStrPObjList<olxstr,TCifLoopData*>& row = hklLoop->GetTable()[i];
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
//			mapY = (int)au.Axes()[1].GetV()*3,
//			mapZ = (int)au.Axes()[2].GetV()*3;
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

//..............................................................................
void XLibMacros::macChangeSG(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  if( au.AtomCount() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "Empty asymmetric unit");
    return;
  }
  if( xapp.XFile().GetRM().UsedSymmCount() != 0 )  {
    E.ProcessingError(__OlxSrcInfo,
      "Please remove used symmetry (EQIV) instructions before using this operation");
    return;
  }
  TSpaceGroup& from_sg = xapp.XFile().GetLastLoaderSG();
  TSpaceGroup* sg = TSymmLib::GetInstance().FindGroupByName(Cmds.GetLastString());
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Could not identify given space group");
    return;
  }
  // change centering?
  if( from_sg.GetName().SubStringFrom(1) == sg->GetName().SubStringFrom(1) )  {
    olxch from = from_sg.GetLattice().GetSymbol()[0],
          to = sg->GetLattice().GetSymbol()[0];
    mat3d tm;
    tm.I();
    if( from == 'I' )  {
      if( to == 'P' )
        tm = mat3d(-0.5, 0.5, 0.5, -0.5, 0.5, -0.5);
    }
    else if( from == 'P' )  {
      if( to == 'I' )
        tm = mat3d(0, 1, 1, 0, 1, 0);
      else if( to == 'C' )  {
        tm = mat3d(0, 1, 1, 0, 1, 0);  // P->I
        tm *= mat3d(-1, 0, 1, 0, 1, 0, -1, 0, 0);  // I->C, uniq axis b
      }
      else if( to == 'F' )
        tm = mat3d(-1, 1, 1, -1, 1, 1);
    }
    else if( from == 'C' )  {
      if( to == 'P' )  {
        tm = mat3d(0, 0, -1, 0, 1, 0, 1, 0, -1);  // C->I, uniq axis b
        tm *= mat3d(-0.5, 0.5, 0.5, -0.5, 0.5, -0.5);  // I->P 
      }
    }
    else if( from == 'F')  {
      if( to == 'P' )
        tm = mat3d(0, 0.5, 0.5, 0, 0.5, 0);
    }
    if( !tm.IsI() )  {
      TBasicApp::NewLogEntry() << "EXPERIMENTAL: transformations considering b unique";
      ChangeCell(tm, *sg, EmptyString());
    }
    else  {
      TBasicApp::NewLogEntry() << "The transformation is not supported";
    }
    return;
  }
  smatd_list ml;
  sg->GetMatrices(ml, mattAll);
  TTypeList<AnAssociation3<vec3d,TCAtom*, int> > list;
  uc.GenereteAtomCoordinates(list, true);
  if( Cmds.Count() == 4 )  {
    vec3d trans(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble());
    for( size_t i=0; i < list.Count(); i++ )  {
      list[i].A() += trans;
      list[i].SetC(1);
    }
  }
  else   {
    for( size_t i=0; i < list.Count(); i++ )
      list[i].SetC(1);
  }
  for( size_t i=0; i < list.Count(); i++ )  {
    if( list[i].GetC() == 0 )  continue;
    for( size_t j=i+1; j < list.Count(); j++ )  {
      if( list[j].GetC() == 0 )  continue;
      for( size_t k=1; k < ml.Count(); k++ )  {
        vec3d v = ml[k] * list[i].GetA();
        v -= list[j].GetA();
        v -= v.Round<int>();
        au.CellToCartesian(v);
        if( v.QLength() < 0.01 )  {
          list[i].C() ++;
          list[j].SetC(0);
        }
      }
    }
  }
  for( size_t i=0; i < au.AtomCount(); i++ )
    au.GetAtom(i).SetTag(0);
  TCAtomPList newAtoms;
  for( size_t i=0; i < list.Count(); i++ )  {
    if( list[i].GetC() == 0 )  continue;
    TCAtom* ca;
    if( list[i].GetB()->GetTag() > 0 )  {
      ca = &au.NewAtom();
      ca->Assign(*list[i].GetB());
    }
    else  {
      ca = list[i].GetB();
      ca->SetTag(ca->GetTag() + 1);
    }
    ca->ccrd() = list[i].GetA();
    ca->AssignEllp(NULL);
  }
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetTag() == 0 )
      au.GetAtom(i).SetDeleted(true);
  }
  au.ChangeSpaceGroup(*sg);
  xapp.XFile().LastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
  latt.Init();
  latt.CompaqAll();
}
//..............................................................................
void XLibMacros::macFlush(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TBasicApp::GetLog().Flush();
}
//..............................................................................
void XLibMacros::macSGE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  using namespace olex;
  TXApp& xapp = TXApp::GetInstance();
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "this function requires Olex2 processor implementation");
  TSpaceGroup* sg = NULL;
  if( EsdlInstanceOf(*xapp.XFile().LastLoader(), TCRSFile) && 
    ((TCRSFile*)xapp.XFile().LastLoader())->HasSG() )  
  {
    sg = &xapp.XFile().GetLastLoaderSG();
    TBasicApp::NewLogEntry() << "Choosing CRS file space group: " << sg->GetName();
  }
  else  {
    TPtrList<TSpaceGroup> sgs;
    E.SetRetVal(&sgs);
    op->executeMacroEx("SG", E);
    E.SetRetVal<bool>(false);
    if( sgs.IsEmpty() )  {
      TBasicApp::NewLogEntry(logError) <<  "Could not find any suitable space group. Terminating ... ";
      return;
    }
    else if( sgs.Count() == 1 )  {
      sg = sgs[0];
      TBasicApp::NewLogEntry() << "Univocal space group choice: " << sg->GetName();
    }
    else  {
      olxstr cmd = "Wilson";
      if( !Cmds.IsEmpty() )
        cmd << " '" << Cmds[0] << '\'';
      op->executeMacroEx(cmd, E);
      bool centro = E.GetRetVal().ToBool();
      TBasicApp::NewLogEntry() << "Searching for centrosymmetric group: " << centro;
      for( size_t i=0; i < sgs.Count(); i++ )  {
        if( centro )  {
          if( sgs[i]->IsCentrosymmetric() )  {
            sg = sgs[i];
            break;
          }
        }
        else  {
          if( !sgs[i]->IsCentrosymmetric() )  {
            sg = sgs[i];
            break;
          }
        }
      }
      if( sg == NULL )  {  // no match to centre of symmetry found
        sg = sgs[0];
        TBasicApp::NewLogEntry() << "Could not match, choosing: " << sg->GetName();
      }
      else  {
        TBasicApp::NewLogEntry() << "Chosen: " << sg->GetName();
      }
    }
  }
  olxstr fn(Cmds.IsEmpty() ? TEFile::ChangeFileExt(TXApp::GetInstance().XFile().GetFileName(), "ins") : Cmds[0]);
  op->executeMacroEx(olxstr("reset -s=") << sg->GetName() << " -f='" << fn << '\'', E);
  if( E.IsSuccessful() )  {
    op->executeMacroEx(olxstr("reap '") << fn << '\'', E);
    if( E.IsSuccessful() )  { 
      OlxStateVar _var(VarName_ResetLock());
      op->executeMacroEx(olxstr("solve"), E);
      // this will reset zoom!
      op->executeMacroEx(olxstr("fuse"), E);
    }
    E.SetRetVal<bool>(E.IsSuccessful());
  }
}
//..............................................................................
void XLibMacros::macASR(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup& sg = xapp.XFile().GetLastLoaderSG();
  if( sg.IsCentrosymmetric() )  {
    E.ProcessingError(__OlxSrcInfo, "not applicable to centrosymmetric space groups");
    return;
  }
  if( xapp.XFile().GetRM().GetHKLF() == 5 || xapp.XFile().GetRM().GetHKLF() == 6 )  {
    E.ProcessingError(__OlxSrcInfo, "not applicable to HKLF 5/6 data format");
    return;
  }
  if( xapp.XFile().GetRM().GetBASF().IsEmpty() )  {
    xapp.XFile().GetRM().AddBASF(0.2);
    xapp.NewLogEntry() << "BASF 0.2 is added";
  }
  if( !xapp.XFile().GetRM().HasTWIN() )  {
    xapp.XFile().GetRM().SetTWIN_n(2);
    xapp.NewLogEntry() << "TWIN set to 2 components";
  }
  if( xapp.XFile().GetRM().HasMERG() && xapp.XFile().GetRM().GetMERG() == 4 )
    xapp.NewLogEntry() << "Please note, that currently Friedel pairs are merged";
  xapp.NewLogEntry() << "Done";
}
//..............................................................................
void XLibMacros::macDescribe(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TStrList lst, out;
  xapp.XFile().GetRM().Describe(lst);
  for( size_t i=0; i < lst.Count(); i++ )
    out.Hyphenate(lst[i], 80, true);
  xapp.NewLogEntry() << out; 
}
//..............................................................................
void XLibMacros::macCalcCHN(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.XFile().HasLastLoader() && Cmds.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "Nor file is loaded neither formula is provided");
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
    TBasicApp::NewLogEntry() << "Full composition:" << NewLineSequence() << chn.Composition();
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
  TBasicApp::NewLogEntry() << "Full composition:" << NewLineSequence() << chn.Composition();
}
//..............................................................................
void XLibMacros::macCalcMass(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.XFile().HasLastLoader() && Cmds.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "Nor file is loaded neither formula is provided");
    return;
  }
  TIPattern ip;
  if( Cmds.Count() == 1 )  {
    olxstr err;
    if( !ip.Calc(Cmds[0], err, true, 0.5) )  {
      Error.ProcessingError(__OlxSrcInfo, "could not parse the given expression: ") << err;
      return;
    }
  }
  else  {
    olxstr err;
    if( !ip.Calc(xapp.XFile().GetAsymmUnit().SummFormula(EmptyString()), err, true, 0.5) )  {
      Error.ProcessingError(__OlxSrcInfo, "could not parse the given expression: ") << err;
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
  TBasicApp::NewLogEntry() << "    -- NOTE THAT NATURAL DISTRIBUTION OF ISOTOPES IS ASSUMED --";
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
//..............................................................................
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
void XLibMacros_fit_chn_process(TTypeList<XLibMacros_ChnFitData>& list, const ematd& chn,
                                const evecd& p,
                                const olxstr names[4],
                                const olxdict<short, double, TPrimitiveComparator>& obs,
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
void XLibMacros::macFitCHN(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TCHNExp chne;
  chne.LoadFromExpression(Cmds[0]);
  TStrList solvents;
  olxdict<short, double, TPrimitiveComparator> obs, calc;
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
      obs(elm->index, Cmds[i].SubStringFrom(si+1).ToDouble()/100);
      calc(elm->index, 0);
    }
  }
  if( solvents.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "a space separated list of solvents is expected");
    return;
  }
  TTypeList<XLibMacros_ChnFitData> list;
  const double Mw = chne.CHN(calc);
  olxstr names[4] = {chne.SummFormula(EmptyString()), EmptyString(), EmptyString(), EmptyString()};
  ematd m(obs.Count(), 3), chn(4, obs.Count()+1);
  evecd p(obs.Count());
  olxstr fit_info_from = "Fitting ", fit_info_to;
  for( size_t i=0; i < obs.Count(); i++ )  {
    p[i] = calc.GetValue(i) - obs.GetValue(i)*Mw;
    chn[0][i] = calc.GetValue(i);
    fit_info_from << XElementLib::GetByIndex(calc.GetKey(i)).symbol << ':' << olxstr::FormatFloat(2, calc.GetValue(i)*100/Mw);
    fit_info_to << XElementLib::GetByIndex(obs.GetKey(i)).symbol << ':' << olxstr::FormatFloat(2, obs.GetValue(i)*100);
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
//..............................................................................
void XLibMacros::macStandardise(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
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
//..............................................................................
void XLibMacros::macOmit(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  static olxstr sig("OMIT");
  TXApp &app = TXApp::GetInstance();
  RefinementModel& rm = app.XFile().GetRM();
  if( Cmds.Count() == 1 )  {
    if( Cmds[0].IsNumber() )  {
      const double th = Cmds[0].ToDouble();
      const TTypeList<RefinementModel::BadReflection> &bad_refs =
        rm.GetBadReflectionList();
      for( size_t i=0; i < bad_refs.Count(); i++ )  {
        if( rm.GetOmits().IndexOf(bad_refs[i].index) == InvalidIndex &&
          olx_abs(bad_refs[i].Fc-bad_refs[i].Fo)/bad_refs[i].esd >= th )
        {
          rm.Omit(bad_refs[i].index);
        }
      }
    }
  }
  else if( Cmds.Count() == 2 )  {
    rm.SetOMIT_s(Cmds[0].ToDouble());
    rm.SetOMIT_2t(Cmds[1].ToDouble());
  }
  else 
    rm.AddOMIT(Cmds);
  OnAddIns.Exit(NULL, &sig);
}
//..............................................................................
void XLibMacros::funLst(const TStrObjList &Cmds, TMacroError &E)  {
  const TIns& ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  const TLst& Lst = ins.GetLst();
  if( !Lst.IsLoaded() )
    E.SetRetVal(NAString);
  else if( Cmds[0].Equalsi("rint") )
    E.SetRetVal(Lst.Rint());
  else if( Cmds[0].Equalsi("rsig") )
    E.SetRetVal(Lst.Rsigma());
  else if( Cmds[0].Equalsi("r1") )
    E.SetRetVal(Lst.R1());
  else if( Cmds[0].Equalsi("r1a") )
    E.SetRetVal(Lst.R1a());
  else if( Cmds[0].Equalsi("wr2") )
    E.SetRetVal(Lst.wR2());
  else if( Cmds[0].Equalsi("s") )
    E.SetRetVal(Lst.S());
  else if( Cmds[0].Equalsi("rs") )
    E.SetRetVal(Lst.RS());
  else if( Cmds[0].Equalsi("params") )
    E.SetRetVal(Lst.Params());
  else if( Cmds[0].Equalsi("rtotal") )
    E.SetRetVal(Lst.TotalRefs());
  else if( Cmds[0].Equalsi("runiq") )
    E.SetRetVal(Lst.UniqRefs());
  else if( Cmds[0].Equalsi("r4sig") )
    E.SetRetVal(Lst.Refs4sig());
  else if( Cmds[0].Equalsi("peak") )
    E.SetRetVal(Lst.Peak());
  else if( Cmds[0].Equalsi("hole") )
    E.SetRetVal(Lst.Hole());
  else if( Cmds[0].Equalsi("F000") )
    E.SetRetVal(Lst.F000());
  else if( Cmds[0].Equalsi("Rho") )
    E.SetRetVal(Lst.Rho());
  else if( Cmds[0].Equalsi("Mu") )
    E.SetRetVal(Lst.Mu());
  else if( Cmds[0].Equalsi("flack") )  {
    if( Lst.HasFlack() )
      E.SetRetVal(Lst.Flack().ToString());
    else
      E.SetRetVal(NAString);
  }
  else
    E.SetRetVal(NAString);
}
//..............................................................................
void XLibMacros::macReset(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  TXApp& xapp = TXApp::GetInstance();
  if( !(xapp.CheckFileType<TIns>() ||
        xapp.CheckFileType<TP4PFile>() ||
        xapp.CheckFileType<TCRSFile>()  )  )  return;
  if( TOlxVars::IsVar(VarName_InternalTref()) || TOlxVars::IsVar(VarName_ResetLock()))
    return;
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  olxstr newSg(Options.FindValue('s')), 
         content( olxstr::DeleteChars(Options.FindValue('c'), ' ')),
         fileName(Options.FindValue('f'));
  xapp.XFile().UpdateAsymmUnit();
  TIns *Ins = (TIns*)xapp.XFile().FindFormat("ins");
  if( xapp.CheckFileType<TP4PFile>() )  {
    if( newSg.IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "please specify a space group with -s=SG switch");
      return;
    }
    Ins->Adopt(xapp.XFile());
  }
  else if( xapp.CheckFileType<TCRSFile>() )  {
    TSpaceGroup* sg = xapp.XFile().GetLastLoader<TCRSFile>().GetSG();
    if( newSg.IsEmpty() )  {
      if( sg == NULL )  {
        E.ProcessingError(__OlxSrcInfo, "please specify a space group with -s=SG switch");
        return;
      }
      else  {
        TBasicApp::NewLogEntry() << "The CRS file format space group is: " << sg->GetName();
      }
    }
    Ins->Adopt(xapp.XFile());
  }
  if( !content.IsEmpty() )
    Ins->GetRM().SetUserFormula(content);
  if( Ins->GetRM().GetUserContent().IsEmpty() )  {
    if( op != NULL )  {
      content = "getuserinput(1, \'Please, enter structure composition\', \'C1\')";
      if( op->executeFunction(content, content) )
        Ins->GetRM().SetUserFormula(content);
      if( Ins->GetRM().GetUserContent().IsEmpty() )  {
        E.ProcessingError(__OlxSrcInfo, "empty SFAC instruction, please use -c=Content to specify");
        return;
      }
    }
  }
  if( !newSg.IsEmpty() )  {
    TSpaceGroup* sg = TSymmLib::GetInstance().FindGroupByName(newSg);
    if( !sg )  {
      E.ProcessingError(__OlxSrcInfo, "could not find space group: ") << newSg;
      return;
    }
    Ins->GetAsymmUnit().ChangeSpaceGroup(*sg);
    newSg.SetLength(0);
    newSg <<  " reset to " << sg->GetName() << " #" << sg->GetNumber();
    olxstr titl(TEFile::ChangeFileExt(TEFile::ExtractFileName(xapp.XFile().GetFileName()), EmptyString()));
    Ins->SetTitle(titl << " in " << sg->GetName() << " #" << sg->GetNumber());
  }
  if( fileName.IsEmpty() )
    fileName = xapp.XFile().GetFileName();
  olxstr FN(TEFile::ChangeFileExt(fileName, "ins"));
  olxstr lstFN(TEFile::ChangeFileExt(fileName, "lst"));

  Ins->SaveForSolution(FN, Cmds.Text(' '), newSg, !Options.Contains("rem"),
    Options.Contains("atoms"));
  if( TEFile::Exists(lstFN) )  {
    olxstr lstTmpFN(lstFN);
    lstTmpFN << ".tmp";
    TEFile::Rename(lstFN, lstTmpFN);
  }
  if( op != NULL )  {
    op->executeMacroEx(olxstr("@reap \'") << FN << '\'', E);
    if( E.IsSuccessful() ) {
      ABasicFunction *uf = op->GetLibrary().FindMacro("html.Update");
      if (uf != 0)
        op->executeMacro("html.Update");
    }
  }
}
//..............................................................................
void XLibMacros::macDegen(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TSAtomPList atoms;
  TXApp::GetInstance().FindSAtoms(Cmds.Text(' '), atoms, true, !Options.Contains("cs"));
  for( size_t i=0; i < atoms.Count(); i++ ) 
    atoms[i]->CAtom().SetTag(i);
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( (size_t)atoms[i]->CAtom().GetTag() != i ||
        atoms[i]->CAtom().GetDegeneracy() == 1)
    {
      continue;
    }
    olxstr str(atoms[i]->CAtom().GetLabel());
    TBasicApp::NewLogEntry() << str.RightPadding(6, ' ', true) <<
      atoms[i]->CAtom().GetDegeneracy();
    for( size_t j=0; j < atoms[i]->CAtom().EquivCount(); j++ )  {
      TBasicApp::NewLogEntry() << '\t' <<
        TSymmParser::MatrixToSymmEx(atoms[i]->CAtom().GetEquiv(j));
    }
    SiteSymmCon ssc = atoms[i]->CAtom().GetSiteConstraints();
    TBasicApp::GetLog() << "\tSite constraints: "; 
    if( ssc.IsConstrained() )
      TBasicApp::NewLogEntry() << ssc.ToString(); 
    else
      TBasicApp::NewLogEntry() << "none"; 
  }
}
//..............................................................................
void XLibMacros::macClose(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp::GetInstance().XFile().Close();
}
//..............................................................................
void XLibMacros::macPiPi(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
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
    const double rms = TSPlane::CalcRMS(rings[i]);
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
    TTypeList<AnAssociation2<TSAtom*,double> > ring_atoms;
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
              TTypeList<AnAssociation2<vec3d, double> > points;
              vec3d plane_params, plane_center;
              for( size_t pi=0; pi < planes[j].Count(); pi++ )  {
                points.AddNew(mat*planes[j].GetAtom(pi).ccrd(), 1.0);
                au.CellToCartesian(points.GetLast().A());
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
                if( transforms.IndexOf(mat) == InvalidIndex )
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
//..............................................................................
void XLibMacros::funCrd(const TStrObjList& Params, TMacroError &E) {
  TSAtomPList Atoms;
  if( !TXApp::GetInstance().FindSAtoms(Params.Text(' '), Atoms, true, true) ) {
    E.ProcessingError(__OlxSrcInfo, "could not find any atoms");
    return;
  }
  vec3d center;
  for( size_t i=0; i < Atoms.Count(); i++ )
    center += Atoms[i]->crd();
  center /= Atoms.Count();
  E.SetRetVal(olxstr::FormatFloat(3, center[0]) << ' ' <<
              olxstr::FormatFloat(3, center[1]) << ' ' <<
              olxstr::FormatFloat(3, center[2]));
}
//..............................................................................
void XLibMacros::funCCrd(const TStrObjList& Params, TMacroError &E)  {
  TSAtomPList Atoms;
  if( !TXApp::GetInstance().FindSAtoms(Params.Text(' '), Atoms, true, true) ) {
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
//..............................................................................
void XLibMacros::macMolInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TSAtomPList atoms;
  TXApp &app = TXApp::GetInstance();
  if( !app.FindSAtoms(Cmds.Text(' '), atoms, true, true) )
    return;
  typedef double float_type; // for generation >= 8, double ,ust be used...
  typedef TVector3<float_type> vec_type;
  TTypeList<TVector3<float_type> > verts;
  TTypeList<IndexTriangle> triags;
  const size_t generation = olx_min(10, Options.FindValue('g', '5').ToSizeT());
  if( Options.FindValue('s') == 't' )
    OlxSphere<float_type, TetrahedronFP<vec_type> >::Generate(1.0, generation, verts, triags);
  else
    OlxSphere<float_type, OctahedronFP<vec_type> >::Generate(1.0, generation, verts, triags);

  float_type volume_p = 0, volume_a = 0, area_p = 0, area_a = 0;
  TArrayList<int8_t> t_map(atoms.Count()*triags.Count());
  app.PrintVdWRadii(ElementRadii(), app.XFile().GetAsymmUnit().GetContentList());
  for( size_t i=0; i < atoms.Count(); i++ )  {
    const float_type r = (float_type)atoms[i]->GetType().r_vdw;
    volume_p += (float_type)olx_sphere_volume(r);
    area_p += (float_type)(4*M_PI*r*r);
    const size_t off = i*triags.Count();
    for( size_t j=0; j < triags.Count(); j++ )  {
      t_map[j+off] = 3;
      volume_a += olx_abs((verts[triags[j].vertices[0]]*r).DotProd(
        (verts[triags[j].vertices[1]]*r).XProdVec(
        (verts[triags[j].vertices[2]]*r))));
      area_a += ((verts[triags[j].vertices[1]]-verts[triags[j].vertices[0]])*r).XProdVec(
        ((verts[triags[j].vertices[2]]-verts[triags[j].vertices[0]])*r)).Length();
    }
  }
  volume_a /= 6;
  area_a /= 2;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    const TSAtom& a1 = *atoms[i];
    const float_type r_sq = (float_type)olx_sqr(a1.GetType().r_vdw);
    const vec_type center1 = a1.crd();
    for( size_t j=0; j < atoms.Count(); j++ )  {
      if( i == j )  continue;
      const TSAtom& a2 = *atoms[j];
      const float_type dist = (float_type)a1.crd().DistanceTo(a2.crd());
      if( dist >= (a1.GetType().r_vdw+a2.GetType().r_vdw) )  continue;
      const float_type r = (float_type)a2.GetType().r_vdw;
      const vec_type center2 = a2.crd();
      const size_t off = triags.Count()*j;
      for( size_t k=0; k < triags.Count(); k++ )  {
        if( t_map[k+off] == 0 )  continue;
        const float_type d[] = {
          (verts[triags[k].vertices[0]]*r+center2).QDistanceTo(center1),
          (verts[triags[k].vertices[1]]*r+center2).QDistanceTo(center1),
          (verts[triags[k].vertices[2]]*r+center2).QDistanceTo(center1)
        };
        if( d[0] < r_sq && d[1] < r_sq && d[2] < r_sq )
          t_map[k+off] = 0;
        else if( (d[0] < r_sq && (d[1] < r_sq || d[2] < r_sq)) || (d[1] < r_sq && d[2] < r_sq) )
          t_map[k+off] = 1;
        else if( d[0] < r_sq || d[1] < r_sq || d[2] < r_sq )
          t_map[k+off] = 2;
        
      }
    }
  }
  TVector3<float_type> mol_vol;
  float_type mol_area = 0;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    const size_t off = triags.Count()*i;
    const float_type r = (float_type)atoms[i]->GetType().r_vdw;
    const vec_type center = atoms[i]->crd();
    for( size_t j=0; j < triags.Count(); j++ )  {
      if( t_map[off+j] == 0 )  continue;
      const vec_type
        v1 = verts[triags[j].vertices[0]]*r,
        v2 = verts[triags[j].vertices[1]]*r,
        v3 = verts[triags[j].vertices[2]]*r;
      vec_type dp = (v2-v1).XProdVec(v3-v1);
      const float_type m = (float_type)(1.0/2*(float_type)t_map[off+j]/3.0);
      mol_area += m*dp.Length();
      const float_type dx21 = v2[0]-v1[0],
        dx31 = v3[0]-v1[0],
        dy21 = v2[1]-v1[1],
        dy31 = v3[1]-v1[1],
        dz21 = v2[2]-v1[2],
        dz31 = v3[2]-v1[2];
      const TVector3<float_type> dv(
        (float_type)((1./3*(v1[0]+v2[0]+v3[0])+center[0])*(dy21*dz31-dz21*dy31)),
        (float_type)((1./3*(v1[1]+v2[1]+v3[1])+center[1])*(dz21*dx31-dx21*dz31)),
        (float_type)((1./3*(v1[2]+v2[2]+v3[2])+center[2])*(dx21*dy31-dy21*dx31)));
      mol_vol += dv*m;
    }
  }
  TBasicApp::NewLogEntry() << "Approximating spheres by " << triags.Count() <<
    " triangles";
  TBasicApp::NewLogEntry() << "Volume approximation error, %: " <<
    olxstr::FormatFloat(3, olx_abs(volume_p-volume_a)*100.0/volume_p);
  TBasicApp::NewLogEntry() << "Surface area approximation error, %: " <<
    olxstr::FormatFloat(3, olx_abs(area_p-area_a)*100.0/area_p);
  TBasicApp::NewLogEntry() << "Molecular surface area, A^2: " <<
    olxstr::FormatFloat(2, mol_area);
  TBasicApp::NewLogEntry() << "Molecular volume, A^3: " <<
    olxstr::FormatFloat(2, mol_vol.AbsSum()/3);
}
//..............................................................................
void XLibMacros::macRTab(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TSAtomPList atoms;
  olxstr name = Cmds[0];
  if( !TXApp::GetInstance().FindSAtoms(Cmds.Text(' ', 1), atoms, true, true) )
    return;
  if( atoms.Count() >= 1 && atoms.Count() <= 4 )  {
    RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
    InfoTab& it = rm.AddRTAB(name);
    for( size_t i=0; i < atoms.Count(); i++ ) {
      it.AddAtom(&atoms[i]->CAtom(), atoms[i]->GetMatrix(0).IsFirst() ? NULL
       : &atoms[i]->GetMatrix(0));
    }
  }
  else
    Error.ProcessingError(__OlxSrcInfo, "1 to 3 atoms is expected");
}
//..............................................................................
void XLibMacros::macHklMerge(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TXFile& xf = TXApp::GetInstance().XFile();
  TRefList refs;
  RefinementModel::HklStat ms =
    xf.GetRM().GetRefinementRefList<
    TUnitCell::SymmSpace,RefMerger::StandardMerger>(
      xf.GetUnitCell().GetSymmSpace(), refs);
  if (Options.Contains('z')) {
    for (size_t i=0; i < refs.Count(); i++) {
      if (refs[i].GetI() < 0)
        refs[i].SetI(0);
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
    tab[3][1] << ms.SystematicAbsentcesRemoved;
  tab[4][0] << "Rint";
    tab[4][1] << ms.Rint;
  tab[5][0] << "Rsigma";
    tab[5][1] << ms.Rsigma;
  TBasicApp::NewLogEntry() <<
    tab.CreateTXTList("Merging statistics ", true, false, "  ");
  olxstr hklFileName = xf.GetRM().GetHKLSource();
  THklFile::SaveToFile(Cmds.IsEmpty() ? hklFileName : Cmds.Text(' '), refs);
}
//..............................................................................
void XLibMacros::macHklAppend(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
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

  const olxstr hklSrc = TXApp::GetInstance().LocateHklFile();
  if( !TEFile::Exists( hklSrc ) )  {
    E.ProcessingError(__OlxSrcInfo, "could not find hkl file: ") << hklSrc;
    return;
  }
  THklFile Hkl;
  size_t c = 0;
  Hkl.LoadFromFile(hklSrc);
  if( Options.IsEmpty() )  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].GetTag() < 0 )  {
        Hkl[i].SetTag(-Hkl[i].GetTag());
        c++;
      }
    }
  }
  else if( combine )  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].GetTag() < 0 )  {
        if( !h.IsEmpty() && h.IndexOf(Hkl[i].GetH()) == InvalidIndex ) continue;
        if( !k.IsEmpty() && k.IndexOf(Hkl[i].GetK()) == InvalidIndex ) continue;
        if( !l.IsEmpty() && l.IndexOf(Hkl[i].GetL()) == InvalidIndex ) continue;
        Hkl[i].SetTag(-Hkl[i].GetTag());
        c++;
      }
    }
  }
  else  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].GetTag() < 0 )  {
        if( h.IndexOf(Hkl[i].GetH()) != InvalidIndex ||
            k.IndexOf(Hkl[i].GetK()) != InvalidIndex ||
            l.IndexOf(Hkl[i].GetL()) != InvalidIndex )  {
          Hkl[i].SetTag(-Hkl[i].GetTag());
          c++;
        }
      }
    }
  }
  Hkl.SaveToFile(hklSrc);
  TBasicApp::NewLogEntry() << c << " reflections appended";
}
//..............................................................................
void XLibMacros::macHklExclude(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TIntList h, k, l;
  const bool combine = Options.FindValue("c", TrueString()).ToBool();
  TStrList toks(Options.FindValue('h', EmptyString()), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    h.Add(toks[i].ToInt());
  toks.Clear();
  toks.Strtok(Options.FindValue('k', EmptyString()), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    k.Add(toks[i].ToInt());
  toks.Clear();
  toks.Strtok(Options.FindValue('l', EmptyString()), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    l.Add(toks[i].ToInt());

  const olxstr hklSrc(TXApp::GetInstance().LocateHklFile());
  if( !TEFile::Exists(hklSrc) )  {
    E.ProcessingError(__OlxSrcInfo, "could not find hkl file: ") << hklSrc;
    return;
  }
  if( h.IsEmpty() && k.IsEmpty() && l.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "please provide a condition");
    return;
  }
  THklFile Hkl;
  size_t c = 0;
  Hkl.LoadFromFile(hklSrc);
  if( combine )  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].GetTag() > 0 )  {
        if( !h.IsEmpty() && h.IndexOf(Hkl[i].GetH()) == InvalidIndex) continue;
        if( !k.IsEmpty() && k.IndexOf(Hkl[i].GetK()) == InvalidIndex) continue;
        if( !l.IsEmpty() && l.IndexOf(Hkl[i].GetL()) == InvalidIndex) continue;
        Hkl[i].SetTag(-Hkl[i].GetTag());
        c++;
      }
    }
  }
  else  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].GetTag() > 0 )  {
        if( (!h.IsEmpty() && h.IndexOf(Hkl[i].GetH()) != InvalidIndex) ||
            (!k.IsEmpty() && k.IndexOf(Hkl[i].GetK()) != InvalidIndex) ||
            (!l.IsEmpty() && l.IndexOf(Hkl[i].GetL()) != InvalidIndex) )
        {
          Hkl[i].SetTag(-Hkl[i].GetTag());
          c++;
        }
      }
    }
  }
  Hkl.SaveToFile(hklSrc);
  TBasicApp::NewLogEntry() << c << " reflections excluded";
}
//..............................................................................
void XLibMacros::macHklImport(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TStrList lines;
  lines.LoadFromFile(Cmds[0]);
  const olxstr out_name = Cmds.GetLastString();
  Cmds.Delete(Cmds.Count()-1);
  if( Cmds[1].Equalsi("fixed") )  {
    TSizeList format;
    for( size_t i=2; i < Cmds.Count(); i++ )
      format.Add(Cmds[i].ToSizeT());
    if( format.Count() < 5 || format.Count() > 6 )  {
      E.ProcessingError(__OlxSrcInfo, "5 or 6 numbers are expected");
      return;
    }
    TRefList refs;
    for( size_t i=0; i < lines.Count(); i++ )  {
      TStrList toks;
      if( toks.StrtokF(lines[i], format) < format.Count() )
        continue;
      TReflection& r = refs.AddNew(
        toks[0].ToInt(), toks[1].ToInt(), toks[2].ToInt()
        );
      r.SetI(toks[3].ToDouble());
      r.SetS(toks[4].ToDouble());
      if( format.Count() == 6 )
        r.SetBatch(toks[5].ToInt());
    }
    THklFile::SaveToFile(out_name, refs);
  }
  else if( Cmds[1].Equalsi("separator") )  {
    if( Cmds[2].IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "non-empty separator is expected");
      return;
    }
    const bool has_batch = Options.Contains("batch");
    TRefList refs;
    const olxch sep = Cmds[2].CharAt(0);
    for( size_t i=0; i < lines.Count(); i++ )  {
      TStrList toks(lines[i], sep);
      if( toks.Count() < 5 )  continue;
      TReflection& r = refs.AddNew(
        toks[0].ToInt(), toks[1].ToInt(), toks[2].ToInt()
        );
      r.SetI(toks[3].ToDouble());
      r.SetS(toks[4].ToDouble());
      if( has_batch && toks.Count() >= 6 )
        r.SetBatch(toks[5].ToInt());
    }
    THklFile::SaveToFile(out_name, refs);
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined keyword: ") << Cmds[1]);
  }
}
//..............................................................................
void XLibMacros::macUpdate(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TXApp& app = TXApp::GetInstance();
  olx_object_ptr<TXFile> xf((TXFile*)app.XFile().Replicate());
  xf.p->p->LoadFromFile(Cmds.Text(' '));
  RefinementModel &this_rm = app.XFile().GetRM(),
    &that_rm = xf.p->p->GetRM();
  if( !this_rm.Update(that_rm) )  {
    E.ProcessingError(__OlxSrcInfo, "Asymmetric units do not match");
    return;
  }
  app.XFile().EndUpdate();
}
//..............................................................................
void XLibMacros::funCalcR(const TStrObjList& Params, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TRefList refs;
  evecd Fsq;
  TUnitCell::SymmSpace sp = xapp.XFile().GetUnitCell().GetSymmSpace();
  RefinementModel& rm = xapp.XFile().GetRM();
  const TDoubleList& basf = rm.GetBASF();
  SymmSpace::InfoEx info_ex = SymmSpace::Compact(sp);
  if( !basf.IsEmpty() )  {
    if( rm.GetHKLF() >= 5 )  {
      size_t tn = rm.HasTWIN() ? rm.GetTWIN_n() : 0;
      twinning::general twin(info_ex, rm.GetReflections(),
        RefUtil::ResolutionAndSigmaFilter(rm), basf,
        mat3d::Transpose(rm.GetTWIN_mat()), tn);
      TArrayList<compd> F(twin.unique_indices.Count());
      SFUtil::CalcSF(xapp.XFile(), twin.unique_indices, F);
      twin.calc_fsq(F, Fsq);
      refs = twin.reflections;
    }
    else  {
      RefinementModel::HklStat ms =
        rm.GetRefinementRefList<
          TUnitCell::SymmSpace,RefMerger::ShelxMerger>(sp, refs);
      if (ms.FriedelOppositesMerged)
        info_ex.centrosymmetric = true;
      TArrayList<compd> F(refs.Count());
      SFUtil::CalcSF(xapp.XFile(), refs, F, ms.MERG != 4);
      twinning::merohedral twin(
        info_ex, refs, ms, basf, mat3d::Transpose(rm.GetTWIN_mat()),
        rm.GetTWIN_n());
      twin.calc_fsq(F, Fsq);
    }
  }
  else  {
    RefinementModel::HklStat ms =
      rm.GetRefinementRefList<
        TUnitCell::SymmSpace,RefMerger::ShelxMerger>(sp, refs);
    if (ms.FriedelOppositesMerged)
      info_ex.centrosymmetric = true;
    TArrayList<compd> F(refs.Count());
    Fsq.Resize(refs.Count());
    SFUtil::CalcSF(xapp.XFile(), refs, F);
    for( size_t i=0; i < F.Count(); i++ )
      Fsq[i] = F[i].qmod();
  }
  double scale_k = 1./olx_sqr(rm.Vars.GetVar(0).GetValue());
  if (!Params.IsEmpty() && Params[0].IndexOfi("scale") != InvalidIndex) {
    double sup=0, sdn=0;
    for( size_t i=0; i < refs.Count(); i++ )  {
      if (refs[i].GetI() < 3*refs[i].GetS()) continue;
      sup += refs[i].GetI();
      sdn += Fsq[i];
    }
    scale_k = sdn/sup;
  }
  double wR2u=0, wR2d=0, R1u=0, R1d=0, R1up = 0, R1dp = 0;
  size_t r1p_cnt=0;
  TDoubleList wght = rm.used_weight;
  while( wght.Count() < 6 )
    wght.Add(0);
  wght[5] = 1./3;
  const double exti = rm.HasEXTI() ? rm.GetEXTI() : 0;
  for( size_t i=0; i < refs.Count(); i++ )  {
    TReflection& r = refs[i];
    double Fc2 = Fsq[i];
    if( exti != 0 )  {
      const double l = rm.expl.GetRadiation();
      double x = sp.HklToCart(r.GetHkl()).QLength()*l*l/4;
      Fc2 /= sqrt(1+0.0005*exti*Fc2*l*l*l/sqrt(olx_max(0,x*(1-x))));
    }
    const double Fc = sqrt(olx_abs(Fc2));
    double Fo2 = olx_abs(r.GetI()*scale_k);
    const double Fo = sqrt(Fo2);
    const double sigFo2 = r.GetS()*scale_k;
    const double P = wght[5]*olx_max(0, Fo2) + (1.0-wght[5])*Fc2;
    const double w =
      1./(olx_sqr(sigFo2) + olx_sqr(wght[0]*P) + wght[1]*P + wght[2]);
    wR2u += w*olx_sqr(Fo2-Fc2);
    wR2d += w*olx_sqr(Fo2);
    R1u += olx_abs(Fo-Fc);
    R1d += Fo;
    if( Fo2 >= 2*sigFo2 )  {
      R1up += olx_abs(Fo-Fc);
      R1dp += Fo;
      r1p_cnt++;
    }
  }
  double wR2 = sqrt(wR2u/wR2d);
  double R1 = R1u/R1d;
  double R1p = R1up/R1dp;
  if( !Params.IsEmpty() && Params[0].IndexOfi("print") != InvalidIndex )  {
    xapp.NewLogEntry() << "R1 (All, " << refs.Count() << ") = " <<
      olxstr::FormatFloat(4, R1);
    xapp.NewLogEntry() << "R1 (I/sig >= 2, " << r1p_cnt << ") = " <<
      olxstr::FormatFloat(4, R1p);
    xapp.NewLogEntry() << "wR2 = " << olxstr::FormatFloat(4, wR2);
  }
  E.SetRetVal(olxstr(R1) << ',' << R1p << ',' << wR2);
}
//..............................................................................
void XLibMacros::funGetCompilationInfo(const TStrObjList& Params,
  TMacroError &E)
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
      olxstr rv = timestamp;
      if( !revision.IsEmpty() )  rv << " svn.r" << revision;
#ifdef _MSC_FULL_VER
      rv << " MSC:" << _MSC_FULL_VER;
#elif __INTEL_COMPILER
      rv << " Intel:" << __INTEL_COMPILER;
#elif __GNUC__
      rv << " GCC:" << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__;
#endif
      rv << " on " << TBasicApp::GetPlatformString();
#ifdef _CUSTOM_BUILD_
      rv << " for " << CustomCodeBase::GetName();
#endif
      E.SetRetVal(rv);
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
//..............................................................................
