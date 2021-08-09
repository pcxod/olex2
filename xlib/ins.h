/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_ins_H
#define __olx_xl_ins_H
#include "xbase.h"
#include "estrlist.h"
#include "xfiles.h"
#include "catom.h"
#include "estlist.h"
#include "bapp.h"
#include "log.h"
#include "symmparser.h"
#include "atomref.h"
#include "residue.h"
#include "estack.h"
#include "lst.h"

BeginXlibNamespace()

typedef TStringToList<olxstr, TCAtom*> TInsList;

class TIns : public TBasicCFile {
  // parsing context state and varables
  struct ParseContext {
    RefinementModel& rm;
    TAsymmUnit& au;
    TStrList Symm, Extras;
    TTypeList<TStrList> Sump;
    TStringToList<olxstr, const cm_Element*>  BasicAtoms;  // SFAC container
    bool CellFound, SetNextPivot, End;
    int Part, ToAnis;
    // number of atoms (left), pivot, Hydrogens or not
    esdl::TStack< AnAssociation3<int, TAfixGroup*, bool> > AfixGroups;
    double PartOccu, SPEC;
    TResidue* Resi;
    TCAtom* Last,
      // this are used to evaluate riding H Uiso coded like -1.5
      *LastWithU, *LastRideable;
    TIns* ins;
    // SAME instructions and the first atom after it/them
    TTypeList< olx_pair_t<TStrList, TCAtom*> > Same;
    ParseContext(RefinementModel& _rm)
      : rm(_rm), au(_rm.aunit),
      Resi(0), Last(0), LastWithU(0), LastRideable(0)
    {
      End = SetNextPivot = CellFound = false;
      SPEC = PartOccu = 0;
      ToAnis = Part = 0;
      ins = 0;
    }
  };
  TStringToList<olxstr, bool> included;
private:
  TStringToList<olxstr, TInsList*> Ins;  // instructions
  TStrList Skipped;
  // mean error of cell parameters. Can be used for estimation of other lengths
  double R1;
  bool LoadQPeaks;// true if Q-peaks should be loaded
  TLst Lst;
protected:
  static TCAtom* _ParseAtom(TStrList& toks, ParseContext& cx, TCAtom* atom = 0);
  olxstr _CellToString();
  olxstr _ZerrToString();
  static void _SaveFVar(RefinementModel& rm, TStrList& SL);
  void _SaveSymm(TStrList& SL);
  void _SaveSizeTemp(TStrList& SL);
  void _ReadExtras(TStrList &l, ParseContext &cx);
  // if solution specified, only OMIT's and HKLSrc are saved
  void _SaveHklInfo(TStrList& SL, bool solution);
  void _SaveRefMethod(TStrList& SL);
  static void _ProcessAfix(TCAtom& a, ParseContext& cx);
  // validates existing AFIX'es and clears the stack
  static void _ProcessAfix0(ParseContext& cx);
  // if atoms is saved, its Tag is added to the index (if not NULL)
  static void _SaveAtom(RefinementModel& rm, TCAtom& a, int& part, int& afix,
    double &spec, TStrList* sfac, TStrList& sl,
    TIndexList* index = 0, bool checkSame = true, bool checkResi = true);
  static void _DrySaveAtom(TCAtom& a, TSizeList &indices,
    bool checkSame = true, bool checkResi = true);

  static void _ProcessSame(ParseContext& cx, const TIndexList *atomIndex = 0);
  // initialises the unparsed instruction list
  void _FinishParsing(ParseContext& cx, bool header_only);
  // processes CONN, FREE and BIND, called from _FinishParsing
  void __ProcessConn(ParseContext& cx);
  // also updates the RM user content if any of the types missing
  void FixTypeListAndLabels();
  // deals with SFAC/DISP
  static bool ProcessSFAC(ParseContext& cx, const TStrList &toks, bool update_rm);
  enum InsType {
    insNone,
    insHeader,
    insFooter
  };
  /* determines if the instruction goes to the header or the footer
  (after HKLF). Always returns insNone if params is null or the Ins is in
  (REM, NEUT).
  */
  InsType GetInsType(const olxstr &ins, const TInsList *params) const;
public:
  TIns();
  virtual ~TIns();

  void Clear();

  DefPropBIsSet(LoadQPeaks)

    // this is -1 if not in the file like REM R1 = ...
    double GetR1() const { return R1; }
  TLst& GetLst() { return Lst; }
  const TLst& GetLst() const { return Lst; }
  /* updates all instructions */
  void UpdateParams();
  void SaveForSolution(const olxstr& FileName, const olxstr& Method,
    const olxstr& comments, bool rems = true, bool save_atoms = false);
  void SavePattSolution(const olxstr& FileName,
    const TTypeList<class TPattAtom>& atoms, const olxstr& comments);
  /* reads a file containing just lines of atoms and updates the provided
   Atoms by index in the AU.
   Instructions are initialised with all unrecognised commands
  */
  void UpdateAtomsFromStrings(RefinementModel& rm,
    const TIndexList& index, TStrList& SL, TStrList& Instructions);
  /* saves some atoms to a plain ins format with no headers etc; to be used
  with UpdateAtomsFromStrings. index is initialised with the order in which
  atom Ids this must be passed to UpdateAtomsFromString
  */
  static bool SaveAtomsToStrings(RefinementModel& rm, const TCAtomPList& CAtoms,
    TIndexList& index, TStrList& SL,
    RefinementModel::ReleasedItems* processed);
  /**/
  static TSizeList::const_list_type DrySave(const TAsymmUnit& au);
  static void ValidateRestraintsAtomNames(RefinementModel& rm,
    bool report = true);
  static bool ParseRestraint(RefinementModel &rm, const TStrList& toks,
    bool warnings = true, size_t r_position=InvalidIndex);
  static void SaveRestraints(TStrList& SL, const TCAtomPList* atoms,
    RefinementModel::ReleasedItems* processed, RefinementModel& rm);
  static void SaveExtras(TStrList& SL, const TCAtomPList* atoms,
    RefinementModel::ReleasedItems* processed, RefinementModel& rm);

  void ParseRestraints(RefinementModel& rm, const TStringToList<olxstr,
    TInsList*>& SL, bool warnings = true);
  //..............................................................................
    /* parses a single line instruction, which does not depend on context
    (as SYMM) this is used internally by ParseIns and AddIns
    */
  static bool _ParseIns(RefinementModel& rm, const TStrList& Toks);

  virtual void LoadFromFile(const olxstr& fileName);
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile &, int);

  TInsList* FindIns(const olxstr& name);
  void ClearIns();
  /* AddIns require refinement model as if the instruction is parsed, the
  result goes to that RM, otherwise an un-parsed instruction will be added to
  this file
  */
  bool AddIns(const olxstr& ins, RefinementModel& rm);
  // the instruction name is Toks[0]
  bool AddIns(const TStrList& Params, RefinementModel& rm, bool CheckUniq = true);
  // a convinience method
  template <class StrLst> bool AddIns(const olxstr& name, const StrLst& Params,
    RefinementModel& rm, bool CheckUniq = true)
  {
    TStrList lst(Params);
    lst.Insert(0, name);
    return AddIns(lst, rm, CheckUniq);
  }
  // utility functions
  static olxstr AtomToString(RefinementModel& rm, TCAtom& CA,
    index_t SfacIndex);
  static void HyphenateIns(const olxstr &InsName, const olxstr &Ins,
    TStrList &Res, int sz = 79);
  static void HyphenateIns(const olxstr &Ins, TStrList &Res, int sz = 79);
protected:
  // index will be automatically incremented if more then one line is parsed
  static bool ParseIns(const TStrList& ins, const TStrList& toks,
    ParseContext& cx, size_t& index);
public:
  // helper function...
  static TStrList::const_list_type SaveSfacUnit(const RefinementModel& rm,
    TStrList& list, size_t sfac_pos, bool save_disp);
  static TStrList::const_list_type SaveSfacUnit(const RefinementModel& rm,
    TStrList& list, size_t sfac_pos);
  TStrList& Preprocess(TStrList& l);

  /* spits out all instructions, including CELL, FVAR, but skipping "L N N N N"
  lines, returns the list
  of SFAC.
  */
  TStrList::const_list_type SaveHeader(TStrList& out,
    bool ValidateRestraintNames);
  /* retursn the remaning "unknown" instructions that did not end up in the
  header
  */
  TStrList::const_list_type GetFooter();
  // Parses all instructions, exclusing atoms, throws if fails
  void ParseHeader(const TStrList& in);
  /* parsed out from REMS if refined with Olex2, typically listed like:
  REM R1_all = 0.0509
  REM R1_gt = 0.0380
  REM wR_ref = 0.0946
  REM GOOF = 0.9447
  REM Shift_max = 0.0014
  REM Shift_mean = 0.0001
  REM Reflections_all = 5404
  REM Reflections_gt = 4352
  REM Parameters = 340
  REM Hole = -0.2418
  REM Peak = 0.3648
  REM Flack = -0.2(4)
  */
  olxstr_dict<olxstr> RefinementInfo;

  struct RCInfo {
    short has_value,  // +1 - true, goes first, -1 - true, goes last, 0 - false
      esd_cnt, // 0,1,2
      atom_limit; // -1, N, if atom count is fewer than value - skipped
    bool full_label;
    RCInfo(short _has_value, short _esd_cnt, short _atom_limit,
      bool _full_label = true)
      : has_value(_has_value), esd_cnt(_esd_cnt), atom_limit(_atom_limit),
      full_label(_full_label)
    {}
  };
  static olxstr RestraintToString(const TSimpleRestraint &r, const RCInfo &ri,
    const TCAtomPList *atoms = 0);

  bool InsExists(const olxstr &Name);
  inline size_t InsCount() const { return Ins.Count(); }
  inline const olxstr& InsName(size_t i) const { return Ins[i]; }
  inline const TInsList& InsParams(size_t i) { return *Ins.GetObject(i); }
  void DelIns(size_t i);
  static bool DoPreserveInvalid() {
    return TBasicApp::GetInstance().GetOptions()
      .FindValue("preserve_invalid_ins", FalseString()).ToBool();
  }
  const TStringToList<olxstr, bool>& getIncluded() const {
    return included;
  }
  virtual IOlxObject* Replicate() const { return new TIns; }
};

EndXlibNamespace()
#endif
