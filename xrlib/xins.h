//---------------------------------------------------------------------------//
#ifndef insH
#define insH

#include "xbase.h"
#include "estrlist.h"
#include "estlist.h"

#include "bapp.h"
#include "log.h"
#include "symmparser.h"
#include "xexp_parser.h"
#include "restraints.h"
#ifdef AddAtom
  #undef AddAtom
#endif

BeginXlibNamespace()

class XShelxIns : public IEObject {
  // parsing context state and varables
  struct ParseContext {
    TStrList Symm;
    TStrPObjList<olxstr, TBasicAtomInfo*>  BasicAtoms;  // SFAC container
    bool CellFound;
    int Afix, Part;
    double PartOccu;
    XResidue* Resi;
    XModel& rm;
    ParseContext(XModel& xm) : rm(xm)  {
      CellFound = false;
      PartOccu = 0;
      Afix = Part = 0;
      Resi = NULL;
      Defs[0] = 0.02;  Defs[1] = 0.1;  Defs[2] = 0.01;  Defs[3] = 0.04;  Defs[4] = 1; 
    }
    double Defs[5];
  };
private:
  TStrList Skipped;
  olxstr Unit, Sfac, Title, HklSrc;
  void HypernateIns(const olxstr &InsName, const olxstr &Ins, TStrList &Res);
  void HypernateIns(const olxstr &Ins, TStrList &Res);
  double   Error, R1;    // mean error of cell parameters. Can be used for estimation of other lengths
  bool     LoadQPeaks;// true if Q-peaks should be loaded
  evecd FWght1;   // could be up to six parameters
  eveci FLS;      // up to four params
  evecd FPLAN;    // up to three params
  olxstr HKLF,
           RefinementMethod,  // L.S. or CGLS
           SolutionMethod;
  double   Radiation;
protected:
  void _SaveSfac(TStrList& list, int pos);
  XScatterer* _ParseAtom(TStrList& toks, ParseContext& cx, XScatterer* scatterer = NULL);
  olxstr _ScattererToString(XScatterer* sc, int SfacIndex);
  olxstr _CellToString();
  olxstr _ZerrToString();
  void _SaveFVar(TStrList& SL);
  void _SaveSymm(TStrList& SL);
  void _SaveHklSrc(TStrList& SL);
  void _SaveRefMethod(TStrList& SL);
  // initialises the unparsed instruction list
  void _FinishParsing();
public:
  XShelxIns();
  virtual ~XShelxIns();

  void Clear();

  DefPropP(double, Error)
  DefPropP(double, Radiation)

  DefPropC(olxstr, RefinementMethod)
  DefPropC(olxstr, SolutionMethod)
  DefPropB(LoadQPeaks)
  
  int GetIterations() const  {  
    if( FLS.Count() == 0 )
      throw TFunctionFailedException(__OlxSourceInfo, "undefined number of iterations");
    return FLS[0];  
  }
  int GetPlan()       const  {  
    if( FPLAN.Count() == 0 )
      throw TFunctionFailedException(__OlxSourceInfo, "undefined number of Fourier peaks");
    return Round(FPLAN[0]);  
  }
  const eveci& GetLSV() const {  return FLS;  }
  void SetIterations( int v ) {  
    if( FLS.Count() == 0 ) FLS.Resize(1);
    FLS[0] = v;  
  }
  void SetPlan(int v)        {  
    if( FPLAN.Count() == 0 )  FPLAN.Resize(1);
    FPLAN[0] = v;  
  }
  const evecd& GetPlanV() const {  return FPLAN;  }

  inline evecd& Wght1()  {  return FWght1;  }
  inline olxstr& Hklf()  { return HKLF;  }
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
  void UpdateFromStrings(XScattererPList& CAtoms, TStrList& SL, TStrList& Instructions);
  /* saves some atoms to a plain ins format with no headers etc; to be used with
    UpdateAtomsFromFile
  */
  bool SaveAtomsToStrings(const XScattererPList& Scatterers, TStrList& SL, IRefinementModel& rm);
  //void SaveRestraints(TStrList& SL, const TCAtomPList* atoms, TSimpleRestraintPList* processed, TAsymmUnit* au);
  template <class StrLst> void ParseRestraints(StrLst& SL, ParseContext& xc)  {
      TStrList Toks;
      olxstr resi, insn;
      for( int i =0; i < SL.Count(); i++ )  {
        RequiredParams = AcceptsParams = 1;
        Toks.Clear();
        Toks.Strtok( SL[i], ' ');
        insn = Toks[0];
        Toks.Delete(0);
        int resi_ind = insn.IndexOf('_');
        if( resi_ind != -1 )  {
          resi = insn.SubStringFrom(resi_ind+1);
          insn = insn.SubStringTo(resi_ind);
        }
        else
          resi = EmptyString;
        if( Toks[0].Comparei("EQIV") == 0 )  {
          if( Toks.Count() > 2 )  {
            srl = NULL;
            olxstr Tmp = Toks[1].SubStringFrom(1);  // $1 -> 1
            if( !Tmp.IsNumber() )
              throw TInvalidArgumentException(__OlxSourceInfo,
              olxstr("A number is expected, \'") << Tmp << "\' is provided");
            Toks.Delete(0);
            Toks.Delete(0);
            smatd SymM;
            TSymmParser::SymmToMatrix(Toks.Text(EmptyString), SymM);
            xc.rm.AddUsedSymm(SymM);
            SL[i] = EmptyString;
          }
        }
        else if( insn.Comparei("DFIX") == 0 )  {
          xc.rm.DFIX.AddNew(rm, xc.Defs, Toks);
          SL[i] = EmptyString;
        }
        else if( insn.Comparei("DANG") == 0 )  {
          xc.rm.DANG.AddNew(rm, xc.Defs, Toks);
          SL[i] = EmptyString;
        }
        else if( insn.Comparei("SADI") == 0 )  {
          xc.rm.SADI.AddNew(rm, xc.Defs, Toks);
          SL[i] = EmptyString;
        }
        else if( insn.Comparei("CHIV") == 0 )  {
          xc.rm.CHIV.AddNew(rm, xc.Defs, Toks);
          SL[i] = EmptyString;
        }
        else if( insn.Comparei("FLAT") == 0 )  {
          xc.rm.FLAT.AddNew(rm, xc.Defs, Toks);
          SL[i] = EmptyString;
        }
        else if( insn.Comparei("DELU") == 0 )  {
          xc.rm.DELU.AddNew(rm, xc.Defs, Toks);
          SL[i] = EmptyString;
        }
        else if( insn.Comparei("SIMU") == 0 )  {
          xc.rm.SIMU.AddNew(rm, xc.Defs, Toks);
          SL[i] = EmptyString;
        }
        else if( insn.Comparei("ISOR") == 0 )  {
          xc.rm.ISOR.AddNew(rm, xc.Defs, Toks);
          SL[i] = EmptyString;
        }
        else if( insn.Comparei("EADP") == 0 )  {
          xc.rm.ShareTDP(Toks);
          SL[i] = EmptyString;
        }
        else if( insn.Comparei("EXYZ") == 0 )  {
          xc.rm.ShareSite(Toks);
          SL[i] = EmptyString;
        }
      }
    }
//..............................................................................
  /* parses a single line instruction, which does not depend on context (as SYMM) 
    this is used internally by ParseIns and AddIns    */
    template <class StrLst> bool _ParseIns(const StrLst& Toks, XModel& rm)  {
      if( Toks[0].Comparei("FVAR") == 0 )  {
        rm.Variables.SetCapacity(rm.Variables.Count()+Toks.Count());
        for( int j=0; j < Toks.Count(); j++ )
          rm.Variables.AddNew(Toks[j].ToDouble(), var_type_None);
      }
      else if( Toks[0].Comparei("WGHT") == 0 )  {
        if( rm.Weight.Count() != 0 )  {
          FWght1.Resize(Toks.Count()-1);
          for( int j=1; j < Toks.Count(); j++ )
            FWght1[j-1] = Toks[j].ToDouble();
        }
        else  {
          rm.Weight.Resize(Toks.Count()-1);
          for( int j=1; j < Toks.Count(); j++ )
            rm.Weight[j-1] = Toks[j].ToDouble();
          FWght1 = rm.Weight;
        }
      }
      else if( Toks[0].Comparei("TITL") == 0 )
        Title = Toks.Text(' ', 1);
      else if( Toks[0].Comparei("HKLF") == 0 && (Toks.Count() > 1) )
        HKLF = Toks.Text(' ', 1);
      else if( Toks[0].Comparei("L.S.") == 0  || Toks[0].Comparei("CGLS") == 0 )  {
        SetRefinementMethod(Toks[0]);
        FLS.Resize( Toks.Count() - 1 );
        for( int i=1; i < Toks.Count(); i++ )
          FLS[i-1] = Toks[i].ToInt();
      }
      else if( Toks[0].Comparei("PLAN") == 0  )  {
        FPLAN.Resize( Toks.Count() - 1 );
        for( int i=1; i < Toks.Count(); i++ )
          FPLAN[i-1] = Toks[i].ToDouble();
      }
      else if( Toks[0].Comparei("LATT") == 0 && (Toks.Count() > 1))
        GetAsymmUnit().SetLatt( (short)Toks[1].ToInt() );
      else if( Toks[0].Comparei("UNIT") == 0 )  // can look like an atom !
        Unit = Toks.Text(' ', 1);
      else if( Toks[0].Comparei("ZERR") == 0 )  {
        if( Toks.Count() == 8 )  {
          rm.Cell.SetSigmas(Toks[2].ToDouble(), Toks[3].ToDouble(), Toks[4].ToDouble(),
                            Toks[5].ToDouble(), Toks[6].ToDouble(), Toks[7].ToDouble());
          GetAsymmUnit().SetZ( (short)Toks[1].ToInt() );
        }
        else
          throw TInvalidArgumentException(__OlxSourceInfo, "ZERR");
      }
      else
        return false;
      return true;
    }

  virtual void SaveToStrings(TStrList& Strings, XModel& xm);
  virtual void LoadFromStrings(const TStrList& Strings, XModel& xm);

  void ClearIns();
  bool AddIns(const olxstr& Name);
  // the instruction name is Toks[0]
  bool AddIns(const TStrList& Params, bool CheckUniq=true);
  // a convinience method
  template <class StrLst> bool AddIns(const olxstr& name, const StrLst& Params, bool CheckUniq=true)  {
    TStrList lst(Params);
    lst.Insert(0, name);
    return AddIns(lst, CheckUniq);
  }
protected:
  // index will be automatically imcremented if more then one line is parsed
  bool ParseIns(const TStrList& ins, const TStrList& toks, ParseContext& cx, int& index);
public:
  // spits out all instructions, including CELL, FVAR, etc
  void SaveHeader(TStrList& out, int* SfacIndex=NULL, int* UnitIndex=NULL);
  // Parses all instructions, exclusing atoms, throws if fails
  void ParseHeader(const TStrList& in);

  void FixUnit();

  virtual IEObject* Replicate()  const {  return new XShelxIns();  }
};

EndXlibNamespace()
#endif
