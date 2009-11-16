//---------------------------------------------------------------------------//
#ifndef insH
#define insH

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

#ifdef AdAtom
  #undef AddAtom
#endif

BeginXlibNamespace()

  typedef TStrPObjList<olxstr,TCAtom*> TInsList;

class TIns: public TBasicCFile  {
  // parsing context state and varables
  struct ParseContext {
    TStrList Symm;
    TStrPObjList<olxstr, TBasicAtomInfo*>  BasicAtoms;  // SFAC container
    bool CellFound, SetNextPivot, End;
    int Part;
    esdl::TStack< AnAssociation3<int,TAfixGroup*, bool> > AfixGroups;  // number of atoms (left), pivot, Hydrogens or not
    double PartOccu;
    TCAtom* Last, 
      *LastWithU, *LastNonH;  // thi sis used to evaluate riding H Uiso coded like -1.5
    TResidue* Resi;
    TAsymmUnit& au;
    RefinementModel& rm;
    // SAME instructions and the first atom after it/them
    TTypeList< AnAssociation2<TStrList,TCAtom*> > Same;
    ParseContext(RefinementModel& _rm) : rm(_rm), au(_rm.aunit), 
      Resi(NULL), Last(NULL), LastWithU(NULL), LastNonH(NULL)  {
      End = SetNextPivot = CellFound = false;
      PartOccu = 0;
      Part = 0;
    }
  };
private:
  TStrPObjList< olxstr, TInsList* > Ins;  // instructions
  TStrList Skipped,
           Disp;  // this should be treated specially as their position is after SFAC and between UNIT
  void HyphenateIns(const olxstr &InsName, const olxstr &Ins, TStrList &Res);
  void HyphenateIns(const olxstr &Ins, TStrList &Res);
  double   R1;    // mean error of cell parameters. Can be used for estimation of other lengths
  bool     LoadQPeaks;// true if Q-peaks should be loaded
  olxstr Sfac, Unit;
protected:
  void _SaveSfac(TStrList& list, size_t pos);
  TCAtom* _ParseAtom(TStrList& toks, ParseContext& cx, TCAtom* atom = NULL);
  olxstr _AtomToString(RefinementModel& rm, TCAtom& CA, index_t SfacIndex);
  olxstr _CellToString();
  olxstr _ZerrToString();
  void _SaveFVar(RefinementModel& rm, TStrList& SL);
  void _SaveSymm(TStrList& SL);
  void _SaveSizeTemp(TStrList& SL);
  // if solution specified, only OMIT's and HKLSrc are saved 
  void _SaveHklInfo(TStrList& SL, bool solution);
  void _SaveRefMethod(TStrList& SL);
  void _ProcessAfix(TCAtom& a, ParseContext& cx);
  // validates existing AFIX'es and clears the stack
  void _ProcessAfix0(ParseContext& cx);
  // if atoms is saved, its Tag is added to the index (if not NULL) 
  void _SaveAtom(RefinementModel& rm, TCAtom& a, int& part, int& afix, 
    TStrPObjList<olxstr,TBasicAtomInfo*>* sfac, TStrList& sl, TIndexList* index=NULL, bool checkSame=true);
  void _ProcessSame(ParseContext& cx);
  // initialises the unparsed instruction list
  void _FinishParsing(ParseContext& cx);
  // processes CONN, FREE and BIND, called from _FinishParsing
  void __ProcessConn(ParseContext& cx);
public:
  TIns();
  virtual ~TIns();

  void Clear();

  DefPropBIsSet(LoadQPeaks)
  
  // this is -1 if not in the file like REM R1 = ...
  inline double GetR1() const {  return R1;  }

  /* olex does not use this - they are just for a record, however they can be changed
    using fixunit command to take the actual values from the asymmetric unit
  */
  DefPropC(olxstr, Sfac)
  DefPropC(olxstr, Unit)
  // created sfac/unit form a string like C37H41P2BRhClO
  void SetSfacUnit(const olxstr& su);

  /* updates all instructions */
  void UpdateParams();
  void SaveForSolution(const olxstr& FileName, const olxstr& Method, const olxstr& comments, bool rems=true);
  void SavePattSolution( const olxstr& FileName, const TTypeList<class TPattAtom>& atoms,
                         const olxstr& comments );
  /* reads a file containing just lines of atoms and updates the to the
   provided Atoms, whic means that the number of atoms should be the same
   as in SaveAtomsToFile and the order should be the same too
   Instructions are initialised with all unrecognised commands
   @retutn error message or an empty string
  */
  void UpdateAtomsFromStrings(RefinementModel& rm, TCAtomPList& CAtoms, const TIndexList& index, TStrList& SL, TStrList& Instructions);
  /* saves some atoms to a plain ins format with no headers etc; to be used with
    UpdateAtomsFromStrings. index is initialised with the order in which atoms saved
    this must be passed to UpdateAtomsFromString
  */
  bool SaveAtomsToStrings(RefinementModel& rm, const TCAtomPList& CAtoms, TIndexList& index, TStrList& SL, 
    RefinementModel::ReleasedItems* processed);
  void ValidateRestraintsAtomNames(RefinementModel& rm);
  bool ParseRestraint(RefinementModel& rm, const TStrList& toks);
  void SaveRestraints(TStrList& SL, const TCAtomPList* atoms, 
    RefinementModel::ReleasedItems* processed, RefinementModel& rm);
  template <class StrLst> void ParseRestraints(RefinementModel& rm, StrLst& SL)  {
      TStrList Toks;
      for( size_t i =0; i < SL.Count(); i++ )  {
        Toks.Clear();
        Toks.Strtok(SL[i], ' ');
        if( ParseRestraint(rm, Toks) )
          SL[i] = EmptyString;
      }
    }
//..............................................................................
  /* parses a single line instruction, which does not depend on context (as SYMM) 
    this is used internally by ParseIns and AddIns    */
    template <class StrLst> bool _ParseIns(RefinementModel& rm, const StrLst& Toks)  {
      if( Toks[0].Equalsi("FVAR") )
        rm.Vars.AddFVAR( Toks.SubListFrom(1) );
      else if( Toks[0].Equalsi("SUMP") )
        rm.Vars.AddSUMP( Toks.SubListFrom(1) );
      else if( Toks[0].Equalsi("WGHT") )  {
        if( rm.used_weight.Count() != 0 )  {
          rm.proposed_weight.SetCount(Toks.Count()-1);
          for( size_t j=1; j < Toks.Count(); j++ )
            rm.proposed_weight[j-1] = Toks[j].ToDouble();
        }
        else  {
          rm.used_weight.SetCount(Toks.Count()-1);
          for( size_t j=1; j < Toks.Count(); j++ )
            rm.used_weight[j-1] = Toks[j].ToDouble();
          rm.proposed_weight = rm.used_weight;
        }
      }
      else if( Toks[0].Equalsi("TITL") )
        Title = Toks.Text(' ', 1);
      else if( Toks[0].Equalsi("MERG") && Toks.Count() == 2 )
        rm.SetMERG( Toks[1].ToInt() );
      else if( Toks[0].Equalsi("SIZE") && (Toks.Count() == 4) )
        rm.expl.SetCrystalSize(Toks[1].ToDouble(), Toks[2].ToDouble(), Toks[3].ToDouble() );
      else if( Toks[0].Equalsi("BASF") && (Toks.Count() > 1) )
        rm.SetBASF( Toks.SubListFrom(1) );
      else if( Toks[0].Equalsi("OMIT") )
        rm.AddOMIT( Toks.SubListFrom(1) );
      else if( Toks[0].Equalsi("SHEL") )
        rm.SetSHEL( Toks.SubListFrom(1) );
      else if( Toks[0].Equalsi("TWIN") )
        rm.SetTWIN( Toks.SubListFrom(1) );
      else if( Toks[0].Equalsi("TEMP") && Toks.Count() == 2 )
        rm.expl.SetTemperature( Toks[1].ToDouble() );
      else if( Toks[0].Equalsi("HKLF") && (Toks.Count() > 1) )
        rm.SetHKLF( Toks.SubListFrom(1) );
      else if( Toks[0].Equalsi("L.S.") || Toks[0].Equalsi("CGLS") )  {
        rm.SetRefinementMethod(Toks[0]);
        rm.LS.SetCount( Toks.Count() - 1 );
        for( size_t i=1; i < Toks.Count(); i++ )
          rm.LS[i-1] = Toks[i].ToInt();
      }
      else if( Toks[0].Equalsi("PLAN") )  {
        rm.PLAN.SetCount( Toks.Count() - 1 );
        for( size_t i=1; i < Toks.Count(); i++ )
          rm.PLAN[i-1] = Toks[i].ToDouble();
      }
      else if( Toks[0].Equalsi("LATT") && (Toks.Count() > 1))
        rm.aunit.SetLatt( (short)Toks[1].ToInt() );
      else if( Toks[0].Equalsi("UNIT") )
        Unit = Toks.Text(' ', 1);
      else if( Toks[0].Equalsi("ZERR") )  {
        if( Toks.Count() == 8 )  {
          rm.aunit.SetZ( (short)Toks[1].ToDouble() );
          rm.aunit.Axes()[0].E() = Toks[2].ToDouble();
          rm.aunit.Axes()[1].E() = Toks[3].ToDouble();
          rm.aunit.Axes()[2].E() = Toks[4].ToDouble();
          rm.aunit.Angles()[0].E() = Toks[5].ToDouble();
          rm.aunit.Angles()[1].E() = Toks[6].ToDouble();
          rm.aunit.Angles()[2].E() = Toks[7].ToDouble();
        }
        else
          throw TInvalidArgumentException(__OlxSourceInfo, "ZERR");
      }
      else
        return false;
      return true;
    }

  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *XF);

  TInsList* FindIns(const olxstr &Name);
  void ClearIns();
  /* AddIns require refinement model as if the instruction is parsed, the result goes to that RM,
    otherwise an un-parsed instruction will be added to this file
  */
  bool AddIns(const olxstr& ins, RefinementModel& rm);
  // the instruction name is Toks[0]
  bool AddIns(const TStrList& Params, RefinementModel& rm, bool CheckUniq=true);
  // a convinience method
  template <class StrLst> bool AddIns(const olxstr& name, const StrLst& Params, RefinementModel& rm, bool CheckUniq=true)  {
    TStrList lst(Params);
    lst.Insert(0, name);
    return AddIns(lst, rm, CheckUniq);
  }
protected:
  // index will be automatically incremented if more then one line is parsed
  bool ParseIns(const TStrList& ins, const TStrList& toks, ParseContext& cx, size_t& index);
public:
  // spits out all instructions, including CELL, FVAR, etc
  void SaveHeader(TStrList& out, bool ValidateRestraintNames);
  // Parses all instructions, exclusing atoms, throws if fails
  void ParseHeader(const TStrList& in);

  bool InsExists(const olxstr &Name);
  inline size_t InsCount() const {  return Ins.Count();  }
  inline const olxstr& InsName(size_t i) const {  return Ins[i];  }
  inline const TInsList& InsParams(size_t i)  {  return *Ins.GetObject(i); }
  void DelIns(size_t i);
  void DeleteAtom(TCAtom *CA);

  virtual IEObject* Replicate()  const {  return new TIns;  }
};

EndXlibNamespace()
#endif
