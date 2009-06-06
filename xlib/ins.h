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
#include "asymmunit.h"
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
    TAsymmUnit::TResidue* Resi;
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
  void HypernateIns(const olxstr &InsName, const olxstr &Ins, TStrList &Res);
  void HypernateIns(const olxstr &Ins, TStrList &Res);
  double   R1;    // mean error of cell parameters. Can be used for estimation of other lengths
  bool     LoadQPeaks;// true if Q-peaks should be loaded
  olxstr Sfac, Unit;
protected:
  void _SaveSfac(TStrList& list, int pos);
  TCAtom* _ParseAtom(TStrList& toks, ParseContext& cx, TCAtom* atom = NULL);
  olxstr _AtomToString(RefinementModel& rm, TCAtom& CA, int SfacIndex);
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
    TStrPObjList<olxstr,TBasicAtomInfo*>* sfac, TStrList& sl, TIntList* index=NULL, bool checkSame=true);
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
  void SaveToRefine(const olxstr& FileName, const olxstr& Method, const olxstr& comments);
  void SavePattSolution( const olxstr& FileName, const TTypeList<class TPattAtom>& atoms,
                         const olxstr& comments );
  /* reads a file containing just lines of atoms and updates the to the
   provided Atoms, whic means that the number of atoms should be the same
   as in SaveAtomsToFile and the order should be the same too
   Instructions are initialised with all unrecognised commands
   @retutn error message or an empty string
  */
  void UpdateAtomsFromStrings(RefinementModel& rm, TCAtomPList& CAtoms, const TIntList& index, TStrList& SL, TStrList& Instructions);
  /* saves some atoms to a plain ins format with no headers etc; to be used with
    UpdateAtomsFromStrings. index is initialised with the order in which atoms saved
    this must be passed to UpdateAtomsFromString
  */
  bool SaveAtomsToStrings(RefinementModel& rm, const TCAtomPList& CAtoms, TIntList& index, TStrList& SL, 
    RefinementModel::ReleasedItems* processed);
  void ValidateRestraintsAtomNames(RefinementModel& rm);
  void SaveRestraints(TStrList& SL, const TCAtomPList* atoms, 
    RefinementModel::ReleasedItems* processed, RefinementModel& rm);
  template <class StrLst> void ParseRestraints(StrLst& SL, ParseContext& cx)  {
      TStrList Toks;
      olxstr resi;
      TSRestraintList* srl;
      double DefEsd, DefEsd1, DefVal, Esd1Mult;
      double* Vals[3];
      bool AcceptsAll; // all atoms
      short AcceptsParams, RequiredParams;
      for( int i =0; i < SL.Count(); i++ )  {
        Toks.Clear();
        Toks.Strtok( SL[i], ' ');
        if( Toks[0].Equalsi("EQIV") && Toks.Count() >= 3 )  {
          smatd SymM;
          TSymmParser::SymmToMatrix(Toks.Text(EmptyString, 2), SymM);
          cx.rm.AddUsedSymm(SymM, Toks[1]);
          SL[i] = EmptyString;
          continue;
        }
        if( (Toks[0].StartsFromi("HTAB") || Toks[0].StartsFromi("RTAB")) && Toks.Count() > 2 )  {
          cx.rm.AddInfoTab( Toks );
          SL[i] = EmptyString;
          continue;
        }
        srl = NULL;
        RequiredParams = AcceptsParams = 1;
        AcceptsAll = false;
        Esd1Mult = DefVal = DefEsd = DefEsd1 = 0;
        Vals[0] = &DefVal;  Vals[1] = &DefEsd;  Vals[2] = &DefEsd1;
        int resi_ind = Toks[0].IndexOf('_');
        if( resi_ind != -1 )  {
          resi = Toks[0].SubStringFrom(resi_ind+1);
          Toks[0] = Toks[0].SubStringTo(resi_ind);
        }
        else
          resi = EmptyString;

        if( Toks[0].Equalsi("EXYZ") )  {
          cx.rm.AddEXYZ( Toks.SubListFrom(1) );
          SL[i] = EmptyString;
          continue;
        }
        else if( Toks[0].Equalsi("DFIX") )  {
          srl = &cx.rm.rDFIX;
          RequiredParams = 1;  AcceptsParams = 2;
          DefEsd = 0.02;
          Vals[0] = &DefVal;  Vals[1] = &DefEsd;
        }
        else if( Toks[0].Equalsi("DANG") )  {
          srl = &cx.rm.rDANG;
          RequiredParams = 1;  AcceptsParams = 2;
          DefEsd = 0.04;
          Vals[0] = &DefVal;  Vals[1] = &DefEsd;
        }
        else if( Toks[0].Equalsi("SADI") )  {
          srl = &cx.rm.rSADI;
          RequiredParams = 0;  AcceptsParams = 1;
          DefEsd = 0.02;
          Vals[0] = &DefEsd;
        }
        else if( Toks[0].Equalsi("CHIV") )  {
          srl = &cx.rm.rCHIV;
          RequiredParams = 1;  AcceptsParams = 2;
          DefEsd = 0.1;
          Vals[0] = &DefEsd;  Vals[1] = &DefVal;
        }
        else if( Toks[0].Equalsi("FLAT") )  {
          srl = &cx.rm.rFLAT;
          DefEsd = 0.1;
          RequiredParams = 0;  AcceptsParams = 1;
          Vals[0] = &DefEsd; ;
        }
        else if( Toks[0].Equalsi("DELU") )  {
          srl = &cx.rm.rDELU;
          DefEsd = 0.01;  DefEsd1 = 0.01;
          Esd1Mult = 1;
          RequiredParams = 0;  AcceptsParams = 2;
          Vals[0] = &DefEsd;  Vals[1] = &DefEsd1;
          AcceptsAll = true;
        }
        else if( Toks[0].Equalsi("SIMU") )  {
          srl = &cx.rm.rSIMU;
          DefEsd = 0.04;  DefEsd1 = 0.08;
          Esd1Mult = 2;
          DefVal = 1.7;
          RequiredParams = 0;  AcceptsParams = 3;
          Vals[0] = &DefEsd;  Vals[1] = &DefEsd1;  Vals[2] = &DefVal;
          AcceptsAll = true;
        }
        else if( Toks[0].Equalsi("ISOR") )  {
          srl = &cx.rm.rISOR;
          DefEsd = 0.1;  DefEsd1 = 0.2;
          Esd1Mult = 2;
          RequiredParams = 0;  AcceptsParams = 2;
          Vals[0] = &DefEsd;  Vals[1] = &DefEsd1;
          AcceptsAll = true;
        }
        else if( Toks[0].Equalsi("EADP") )  {
          srl = &cx.rm.rEADP;
          RequiredParams = 0;  AcceptsParams = 0;
        }
        else
          srl = NULL;
        if( srl != NULL )  {
          TSimpleRestraint& sr = srl->AddNew();
          int index = 1;
          if( Toks.Count() > 1 && Toks[1].IsNumber() )  {
            if( Toks.Count() > 2 && Toks[2].IsNumber() )  {
              if( Toks.Count() > 3 && Toks[3].IsNumber() )  {  // three numerical params
                if( AcceptsParams < 3 )  
                  throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical parameters");
                *Vals[0] = Toks[1].ToDouble();
                *Vals[1] = Toks[2].ToDouble();
                *Vals[2] = Toks[3].ToDouble();
                index = 4; 
              }
              else  {  // two numerical params
                if( AcceptsParams < 2 )  
                  throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical parameters");
                *Vals[0] = Toks[1].ToDouble();
                *Vals[1] = Toks[2].ToDouble();
                index = 3; 
              }
            }
            else  {
              if( AcceptsParams < 1 )  
                throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical parameters");
              *Vals[0] = Toks[1].ToDouble();
              index = 2; 
            }
          }
          cx.rm.Vars.SetParam(sr, 0, DefVal );
          sr.SetEsd( DefEsd );
          if( Vals[0] == &DefEsd )
            sr.SetEsd1( (index <= 2) ? DefEsd*Esd1Mult : DefEsd1 );
          else
            sr.SetEsd1( DefEsd1 );
          if( AcceptsAll && Toks.Count() <= index )  {
            sr.SetAllNonHAtoms(true);
          }
          else  {
            TAtomReference aref(Toks.Text(' ', index));
            TCAtomGroup agroup;
            int atomAGroup;
            try  {  aref.Expand(cx.rm, agroup, resi, atomAGroup);  }
            catch( const TExceptionBase& ex )  {
              TBasicApp::GetLog().Exception( ex.GetException()->GetError() );
              continue;
            }
            if( sr.GetListType() == rltBonds && (agroup.Count() == 0 || (agroup.Count()%2)!=0 ) )  {
              TBasicApp::GetLog().Error( olxstr("Wrong restraint parameters list: ") << SL[i] );
              continue;
            }
            if( Toks[0].Equalsi("FLAT") )  {  // a special case again...
              TSimpleRestraint* sr1 = &sr;
              for( int j=0; j < agroup.Count(); j += atomAGroup )  {
                for( int k=0; k < atomAGroup; k++ )
                  sr1->AddAtom( *agroup[j+k].GetAtom(), agroup[j+k].GetMatrix() );
                if( j != 0 )
                  srl->ValidateRestraint(*sr1);
                sr1 = &srl->AddNew();
                sr1->SetEsd( sr.GetEsd() );
                sr1->SetEsd1( sr.GetEsd1() );
                sr1->SetValue( sr.GetValue() );
              }
            }
            else
              sr.AddAtoms(agroup);
          }
          srl->ValidateRestraint(sr);
          SL[i] = EmptyString;
        }
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
          for( int j=1; j < Toks.Count(); j++ )
            rm.proposed_weight[j-1] = Toks[j].ToDouble();
        }
        else  {
          rm.used_weight.SetCount(Toks.Count()-1);
          for( int j=1; j < Toks.Count(); j++ )
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
        for( int i=1; i < Toks.Count(); i++ )
          rm.LS[i-1] = Toks[i].ToInt();
      }
      else if( Toks[0].Equalsi("PLAN") )  {
        rm.PLAN.SetCount( Toks.Count() - 1 );
        for( int i=1; i < Toks.Count(); i++ )
          rm.PLAN[i-1] = Toks[i].ToDouble();
      }
      else if( Toks[0].Equalsi("LATT") && (Toks.Count() > 1))
        rm.aunit.SetLatt( (short)Toks[1].ToInt() );
      else if( Toks[0].Equalsi("UNIT") )
        Unit = Toks.Text(' ', 1);
      else if( Toks[0].Equalsi("ZERR") )  {
        if( Toks.Count() == 8 )  {
          rm.aunit.SetZ( (short)Toks[1].ToInt() );
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
  // index will be automatically imcremented if more then one line is parsed
  bool ParseIns(const TStrList& ins, const TStrList& toks, ParseContext& cx, int& index);
public:
  // spits out all instructions, including CELL, FVAR, etc
  void SaveHeader(TStrList& out, bool ValidateRestraintNames);
  // Parses all instructions, exclusing atoms, throws if fails
  void ParseHeader(const TStrList& in);

  bool InsExists(const olxstr &Name);
  inline int InsCount()  const              {  return Ins.Count();  }
  inline const olxstr& InsName(int i) const {  return Ins[i];  }
  inline const TInsList& InsParams(int i)   {  return *Ins.GetObject(i); }
  void DelIns(int i);
  void DeleteAtom(TCAtom *CA);

  virtual IEObject* Replicate()  const {  return new TIns;  }
};

EndXlibNamespace()
#endif
