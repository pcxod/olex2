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

class TIns: public TBasicCFile  {
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
    esdl::TStack< AnAssociation3<int,TAfixGroup*, bool> > AfixGroups;
    double PartOccu, SPEC;
    TResidue* Resi;
    TCAtom* Last,
      // this are used to evaluate riding H Uiso coded like -1.5
      *LastWithU, *LastRideable;
    TIns* ins;
    // SAME instructions and the first atom after it/them
    TTypeList< olx_pair_t<TStrList,TCAtom*> > Same;
    ParseContext(RefinementModel& _rm) : rm(_rm), au(_rm.aunit),
      Resi(NULL), Last(NULL), LastWithU(NULL), LastRideable(NULL)
    {
      End = SetNextPivot = CellFound = false;
      SPEC = PartOccu = 0;
      ToAnis = Part = 0;
      ins = NULL;
    }
  };
private:
  TStringToList<olxstr, TInsList*> Ins;  // instructions
  TStrList Skipped;
  // mean error of cell parameters. Can be used for estimation of other lengths
  double R1;
  bool LoadQPeaks;// true if Q-peaks should be loaded
  TLst Lst;
protected:
  static TCAtom* _ParseAtom(TStrList& toks, ParseContext& cx,
    TCAtom* atom = NULL);
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
    TIndexList* index=NULL, bool checkSame=true, bool checkResi=true);
  static void _DrySaveAtom(TCAtom& a, TSizeList &indices,
    bool checkSame = true, bool checkResi = true);

  static void _ProcessSame(ParseContext& cx, const TIndexList *atomIndex=0);
  // initialises the unparsed instruction list
  void _FinishParsing(ParseContext& cx);
  // processes CONN, FREE and BIND, called from _FinishParsing
  void __ProcessConn(ParseContext& cx);
  // also updates the RM user content if any of the types missing
  void FixTypeListAndLabels();
public:
  TIns();
  virtual ~TIns();

  void Clear();

  DefPropBIsSet(LoadQPeaks)

  // this is -1 if not in the file like REM R1 = ...
  double GetR1() const {  return R1;  }
  TLst& GetLst()  {  return Lst;  }
  const TLst& GetLst() const {  return Lst;  }
  /* updates all instructions */
  void UpdateParams();
  void SaveForSolution(const olxstr& FileName, const olxstr& Method,
    const olxstr& comments, bool rems=true, bool save_atoms=false);
  void SavePattSolution(const olxstr& FileName,
    const TTypeList<class TPattAtom>& atoms, const olxstr& comments);
  /* reads a file containing just lines of atoms and updates the provided
   Atoms by index in the AU.
   Instructions are initialised with all unrecognised commands
  */
  static void UpdateAtomsFromStrings(RefinementModel& rm,
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
    bool report=true);
  static bool ParseRestraint(RefinementModel& rm, const TStrList& toks);
  static void SaveRestraints(TStrList& SL, const TCAtomPList* atoms,
    RefinementModel::ReleasedItems* processed, RefinementModel& rm);
  static void SaveExtras(TStrList& SL, const TCAtomPList* atoms,
    RefinementModel::ReleasedItems* processed, RefinementModel& rm);

  template <class StrLst> static
  void ParseRestraints(RefinementModel& rm, StrLst& SL) {
    bool preserve = DoPreserveInvalid();
    for (size_t i = 0; i < SL.Count(); i++) {
      TStrList Toks(SL[i], ' ');
      try {
        if (ParseRestraint(rm, Toks)) {
          SL[i].SetLength(0);
        }
      }
      catch (const TExceptionBase &e) {
        TBasicApp::NewLogEntry(logExceptionTrace) << e;
        if (preserve) {
          SL[i] = olxstr("REM ") << SL[i];
        }
        else {
          SL[i].SetLength(0);
        }
      }
    }
  }
//..............................................................................
  /* parses a single line instruction, which does not depend on context
  (as SYMM) this is used internally by ParseIns and AddIns
  */
    template <class StrLst>
    static bool _ParseIns(RefinementModel& rm, const StrLst& Toks) {
      if (Toks[0].Equalsi("FVAR")) {
        rm.Vars.AddFVAR(Toks.SubListFrom(1));
      }
      else if (Toks[0].Equalsi("WGHT")) {
        if (rm.used_weight.Count() != 0) {
          rm.proposed_weight.SetCount(Toks.Count() - 1);
          for (size_t j = 1; j < Toks.Count(); j++) {
            rm.proposed_weight[j - 1] = Toks[j].IsNumber() ? Toks[j].ToDouble()
              : 0.1;
          }
        }
        else {
          /* shelxl proposes wght in .0000 but print in .000000 format, need
          to check for multiple values in tokens
          */
          TStrList toks(Toks);
          for (size_t j = 1; j < toks.Count(); j++) {
            if (toks[j].CharCount('.') > 1) {
              const size_t fp = toks[j].IndexOf('.');
              if (toks[j].Length() - fp >= 6) {
                // 'recursive' run
                toks.Insert(j + 1, toks[j].SubStringFrom(fp + 7));
                toks[j].SetLength(fp + 6);
              }
            }
          }
          rm.used_weight.SetCount(toks.Count() - 1);
          for (size_t j = 1; j < toks.Count(); j++) {
            if (!toks[j].IsNumber() && toks[j].EndsWith('*')) {
              if (!toks[j].TrimR('*').IsNumber()) {
                rm.used_weight[j - 1] = 0.1;
                continue;
              }
            }
            rm.used_weight[j - 1] = toks[j].ToDouble();
          }
          rm.proposed_weight = rm.used_weight;
        }
      }
      else if (Toks[0].Equalsi("MERG") && Toks.Count() == 2) {
        rm.SetMERG(Toks[1].ToInt());
      }
      else if (Toks[0].Equalsi("EXTI")) {
        TEValueD ev;
        if (Toks.Count() > 1) {
          ev = Toks[1];
        }
        rm.Vars.SetEXTI(ev.GetV(), ev.GetE());
      }
      else if (Toks[0].Equalsi("SIZE") && (Toks.Count() == 4)) {
        rm.expl.SetCrystalSize(Toks[1].ToDouble(), Toks[2].ToDouble(),
          Toks[3].ToDouble());
      }
      else if (Toks[0].Equalsi("BASF") && (Toks.Count() > 1)) {
        rm.Vars.SetBASF(Toks.SubListFrom(1));
      }
      else if (Toks[0].Equalsi("DEFS") && (Toks.Count() > 1)) {
        rm.SetDEFS(Toks.SubListFrom(1));
      }
      else if (Toks[0].Equalsi("SHEL")) {
        rm.SetSHEL(Toks.SubListFrom(1));
      }
      else if (Toks[0].Equalsi("TWIN")) {
        rm.SetTWIN(Toks.SubListFrom(1));
      }
      else if (Toks[0].Equalsi("TEMP") && Toks.Count() == 2) {
        rm.expl.SetTemp(Toks[1]);
      }
      else if (Toks[0].Equalsi("HKLF") && (Toks.Count() > 1)) {
        rm.SetHKLF(Toks.SubListFrom(1));
      }
      else if (Toks[0].Equalsi("L.S.") || Toks[0].Equalsi("CGLS")) {
        rm.SetRefinementMethod(Toks[0]);
        rm.LS.SetCount(Toks.Count() - 1);
        for (size_t i = 1; i < Toks.Count(); i++) {
          rm.LS[i - 1] = Toks[i].ToInt();
        }
      }
      else if (Toks[0].Equalsi("PLAN")) {
        rm.PLAN.SetCount(Toks.Count() - 1);
        for (size_t i = 1; i < Toks.Count(); i++) {
          rm.PLAN[i - 1] = Toks[i].ToDouble();
        }
      }
      else if (Toks[0].Equalsi("LATT") && (Toks.Count() > 1)) {
        rm.aunit.SetLatt((short)Toks[1].ToInt());
      }
      else if (Toks[0].Equalsi("UNIT")) {
        rm.SetUserContentSize(Toks.SubListFrom(1));
      }
      else if (Toks[0].Equalsi("ZERR")) {
        if (Toks.Count() == 8) {
          rm.aunit.SetZ(Toks[1].ToDouble());
          rm.aunit.GetAxisEsds() = vec3d(Toks[2].ToDouble(),
            Toks[3].ToDouble(), Toks[4].ToDouble());
          rm.aunit.GetAngleEsds() = vec3d(Toks[5].ToDouble(),
            Toks[6].ToDouble(), Toks[7].ToDouble());
        }
        else {
          throw TInvalidArgumentException(__OlxSourceInfo, "ZERR");
        }
      }
      else {
        return false;
      }
      return true;
    }

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
  bool AddIns(const TStrList& Params, RefinementModel& rm, bool CheckUniq=true);
  // a convinience method
  template <class StrLst> bool AddIns(const olxstr& name, const StrLst& Params,
    RefinementModel& rm, bool CheckUniq=true)
  {
    TStrList lst(Params);
    lst.Insert(0, name);
    return AddIns(lst, rm, CheckUniq);
  }
  // utility functions
  static olxstr AtomToString(RefinementModel& rm, TCAtom& CA,
    index_t SfacIndex);
  static void HyphenateIns(const olxstr &InsName, const olxstr &Ins,
    TStrList &Res, int sz=80);
  static void HyphenateIns(const olxstr &Ins, TStrList &Res, int sz=80);
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
  template <class List> static List& Preprocess(List& l)  {
    // combine lines
    for( size_t i=0; i < l.Count(); i++ )  {
      if( l[i].EndsWith('=') &&
          (!l[i].StartsFromi("REM") ||
            l[i].IndexOf("olex2.restraint.") != InvalidIndex ||
            l[i].IndexOf("olex2.constraint.") != InvalidIndex ))
      {
        l[i].SetLength(l[i].Length()-1);
        if( (i+1) < l.Count() )  {
          l[i] << l[i+1];
          l.Delete(1+i--);
        }
      }
    }
    // include files
    for( size_t i=0; i < l.Count(); i++ )  {
      if( l[i].StartsFrom('+') )  {
        olxstr fn = l[i].SubStringFrom(1);
        if( !TEFile::Exists(fn) )  {
          TBasicApp::NewLogEntry(logError) << "Included file missing: " << fn;
          continue;
        }
        TStrList lst = TEFile::ReadLines(fn);
        l.Delete(i);
        l.Insert(i, lst);
        i += lst.Count();
      }
    }
    return l;
  }
  /* spits out all instructions, including CELL, FVAR, etc, returns the list
  of SFAC
  */
  TStrList::const_list_type SaveHeader(TStrList& out,
    bool ValidateRestraintNames);
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

  struct RCInfo  {
    short has_value,  // +1 - true, goes first, -1 - true, goes last, 0 - false
      esd_cnt, // 0,1,2
      atom_limit; // -1, N, if atom count is fewer than value - skipped
    bool full_label;
    RCInfo(short _has_value, short _esd_cnt, short _atom_limit,
      bool _full_label=true)
      : has_value(_has_value), esd_cnt(_esd_cnt), atom_limit(_atom_limit),
        full_label(_full_label)  {}
  };
  static olxstr RestraintToString(const TSimpleRestraint &r, const RCInfo &ri,
    const TCAtomPList *atoms=NULL);

  bool InsExists(const olxstr &Name);
  inline size_t InsCount() const {  return Ins.Count();  }
  inline const olxstr& InsName(size_t i) const {  return Ins[i];  }
  inline const TInsList& InsParams(size_t i)  {  return *Ins.GetObject(i); }
  void DelIns(size_t i);
  static bool DoPreserveInvalid() {
    return TBasicApp::GetInstance().GetOptions()
      .FindValue("preserve_invalid_ins", FalseString()).ToBool();
  }
  virtual IOlxObject* Replicate() const {  return new TIns;  }
};

EndXlibNamespace()
#endif
