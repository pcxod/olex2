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
    bool CellFound, DependentNonH, SetNextPivot;
    int Part, Afix;
    TStack< AnAssociation3<int,TCAtom*, bool> > DependentAtoms;  // number of atoms (left), pivot, Hydrogens or not
    double PartOccu;
    TCAtom* Last;
    TAsymmUnit::TResidue* Resi;
    // SAME instructions and the first atom after it/them
    TTypeList< AnAssociation2<TStrList,TCAtom*> > Same;
    ParseContext() : Resi(NULL), Last(NULL)  {
      SetNextPivot = DependentNonH = CellFound = false;
      PartOccu = 0;
      Afix = Part = 0;
    }
  };
private:
  TStrPObjList< olxstr, TInsList* > Ins;  // instructions
  TStrList Skipped,
           Disp;  // this should be treated specially as their position is after SFAC and between UNIT
  olxstr Unit, Sfac;
  void HypernateIns(const olxstr &InsName, const olxstr &Ins, TStrList &Res);
  void HypernateIns(const olxstr &Ins, TStrList &Res);
  double   Error, R1;    // mean error of cell parameters. Can be used for estimation of other lengths
  bool     LoadQPeaks;// true if Q-peaks should be loaded
  evecd FVars; // contains variables, used for calculation right UNIT
  evecd FWght;    // could be up to six parameters
  evecd FWght1;   // could be up to six parameters
  eveci FLS;      // up to four params
  evecd FPLAN;    // up to three params
  olxstr HKLF,
           RefinementMethod,  // L.S. or CGLS
           SolutionMethod;
  double   Radiation;
protected:
  void _SaveSfac(TStrList& list, int pos);
  TCAtom* _ParseAtom(TStrList& toks, ParseContext& cx, TCAtom* atom = NULL);
  olxstr _AtomToString(TCAtom* CA, int SfacIndex);
  olxstr _CellToString();
  olxstr _ZerrToString();
  void _SaveFVar(TStrList& SL);
  void _SaveSymm(TStrList& SL);
  void _SaveHklSrc(TStrList& SL);
  void _SaveRefMethod(TStrList& SL);
  void _ProcessAfix(TCAtom& a, ParseContext& cx);
  // if atoms is saved, its Tag is added to the index (if not NULL) 
  void _SaveAtom(TCAtom& a, int& part, int& afix, TStrPObjList<olxstr,TBasicAtomInfo*>* sfac, TStrList& sl, TIntList* index=NULL, bool checkSame=true);
  void _ProcessSame(ParseContext& cx);
  // initialises the unparsed instruction list
  void _FinishParsing();
public:
  TIns(TAtomsInfo *S);
  virtual ~TIns();

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

  inline evecd& Wght()   {  return FWght;  }
  inline evecd& Wght1()  {  return FWght1;  }
  inline evecd& Vars()   {  return FVars;  }
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
  void UpdateAtomsFromStrings(TCAtomPList& CAtoms, const TIntList& index, TStrList& SL, TStrList& Instructions);
  /* saves some atoms to a plain ins format with no headers etc; to be used with
    UpdateAtomsFromStrings. index is initialised with the order in which atoms saved
    this must be passed to UpdateAtomsFromString
  */
  bool SaveAtomsToStrings(const TCAtomPList& CAtoms, TIntList& index, TStrList& SL, TSimpleRestraintPList* processed);
  void SaveRestraints(TStrList& SL, const TCAtomPList* atoms, TSimpleRestraintPList* processed, TAsymmUnit* au);
  template <class StrLst> void ParseRestraints(StrLst& SL, TAsymmUnit* au)  {
      if( au == NULL )  au = &GetAsymmUnit();
      TStrList Toks;
      olxstr resi;
      TSRestraintList* srl;
      double DefEsd, DefEsd1, DefVal, Esd1Mult;
      double* Vals[3];
      bool AcceptsAll; // all atoms
      short AcceptsParams, RequiredParams;
      for( int i =0; i < SL.Count(); i++ )  {
        RequiredParams = AcceptsParams = 1;
        AcceptsAll = false;
        Esd1Mult = DefVal = DefEsd = DefEsd1 = 0;
        Vals[0] = &DefVal;  Vals[1] = &DefEsd;  Vals[2] = &DefEsd1;
        Toks.Clear();
        Toks.Strtok( SL[i], ' ');
        int resi_ind = Toks[0].IndexOf('_');
        if( resi_ind != -1 )  {
          resi = Toks[0].SubStringFrom(resi_ind+1);
          Toks[0] = Toks[0].SubStringTo(resi_ind);
        }
        else
          resi = EmptyString;
        if( Toks[0].Comparei("EQIV") == 0 )  {
          if( Toks.Count() > 2 )  {
            srl = NULL;
            olxstr Tmp = Toks.String(1).SubStringFrom(1);  // $1 -> 1
            if( !Tmp.IsNumber() )
              throw TInvalidArgumentException(__OlxSourceInfo,
              olxstr("A number is expected, \'") << Tmp << "\' is provided");
            Toks.Delete(0);
            Toks.Delete(0);
            smatd* SymM = new smatd;
            TSymmParser::SymmToMatrix(Toks.Text(EmptyString), *SymM);
            au->AddUsedSymm(*SymM);
            au->AddUsedSymm( *SymM );
            delete SymM;
            SL[i] = EmptyString;
          }
        }
        else if( Toks[0].Comparei("DFIX") == 0 )  {
          srl = &au->RestrainedDistances();
          RequiredParams = 1;  AcceptsParams = 2;
          DefEsd = 0.02;
        }
        else if( Toks[0].Comparei("DANG") == 0 )  {
          srl = &au->RestrainedAngles();
          RequiredParams = 1;  AcceptsParams = 2;
          DefEsd = 0.04;
        }
        else if( Toks[0].Comparei("SADI") == 0 )  {
          srl = &au->SimilarDistances();
          RequiredParams = 0;  AcceptsParams = 1;
          DefEsd = 0.02;
        }
        else if( Toks[0].Comparei("CHIV") == 0 )  {
          srl = &au->RestrainedVolumes();
          RequiredParams = 1;  AcceptsParams = 2;
          DefEsd = 0.1;
        }
        else if( Toks[0].Comparei("FLAT") == 0 )  {
          srl = &au->RestrainedPlanarity();
          DefEsd = 0.1;
          RequiredParams = 0;  AcceptsParams = 1;
        }
        else if( Toks[0].Comparei("DELU") == 0 )  {
          srl = &au->RigidBonds();
          DefEsd = 0.01;  DefEsd1 = 0.01;
          Esd1Mult = 1;
          RequiredParams = 0;  AcceptsParams = 2;
          Vals[0] = &DefEsd;  Vals[1] = &DefEsd1;
          AcceptsAll = true;
        }
        else if( Toks[0].Comparei("SIMU") == 0 )  {
          srl = &au->SimilarU();
          DefEsd = 0.04;  DefEsd1 = 0.08;
          Esd1Mult = 2;
          DefVal = 1.7;
          RequiredParams = 0;  AcceptsParams = 3;
          Vals[0] = &DefEsd;  Vals[1] = &DefEsd1;  Vals[2] = &DefVal;
          AcceptsAll = true;
        }
        else if( Toks[0].Comparei("ISOR") == 0 )  {
          srl = &au->RestranedUaAsUi();
          DefEsd = 0.1;  DefEsd1 = 0.2;
          Esd1Mult = 2;
          RequiredParams = 0;  AcceptsParams = 2;
          Vals[0] = &DefEsd;  Vals[1] = &DefEsd1;
          AcceptsAll = true;
        }
        else if( Toks[0].Comparei("EADP") == 0 )  {
          srl = &au->EquivalentU();
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
          sr.SetValue( DefVal );
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
            try  {  aref.Expand(*au, agroup, resi, atomAGroup);  }
            catch( const TExceptionBase& ex )  {
              TBasicApp::GetLog().Exception( ex.GetException()->GetError() );
              continue;
            }
            if( sr.GetListType() == rltBonds && (agroup.Count() == 0 || (agroup.Count()%2)!=0 ) )  {
              TBasicApp::GetLog().Error( olxstr("Wrong restraint parameters list: ") << SL[i] );
              continue;
            }
            if( Toks[0].Comparei("FLAT") == 0 )  {  // a special case again...
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
    template <class StrLst> bool _ParseIns(const StrLst& Toks)  {
      if( Toks[0].Comparei("FVAR") == 0 )  {
        int VC = FVars.Count();
        FVars.Resize(VC+Toks.Count()-1);  // if the instruction is too long - can be dublicated
        for( int j=1; j < Toks.Count(); j++ )
          FVars[VC+j-1] = Toks[j].ToDouble();
      }
      else if( Toks[0].Comparei("WGHT") == 0 )  {
        if( FWght.Count() != 0 )  {
          FWght1.Resize(Toks.Count()-1);
          for( int j=1; j < Toks.Count(); j++ )
            FWght1[j-1] = Toks[j].ToDouble();
        }
        else  {
          FWght.Resize(Toks.Count()-1);
          for( int j=1; j < Toks.Count(); j++ )
            FWght[j-1] = Toks[j].ToDouble();
          FWght1 = FWght;
        }
      }
      else if( Toks[0].Comparei("TITL") == 0 )
        FTitle = Toks.Text(' ', 1);
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
          GetAsymmUnit().SetZ( (short)Toks[1].ToInt() );
          GetAsymmUnit().Axes()[0].E() = Toks[2].ToDouble();
          GetAsymmUnit().Axes()[1].E() = Toks[3].ToDouble();
          GetAsymmUnit().Axes()[2].E() = Toks[4].ToDouble();
          GetAsymmUnit().Angles()[0].E() = Toks[5].ToDouble();
          GetAsymmUnit().Angles()[1].E() = Toks[6].ToDouble();
          GetAsymmUnit().Angles()[2].E() = Toks[7].ToDouble();
          int j = 0;
          if( GetAsymmUnit().Axes()[0].GetE() != 0 )  {  Error += GetAsymmUnit().Axes()[0].GetE()/GetAsymmUnit().Axes()[0].GetV(); j++; }
          if( GetAsymmUnit().Axes()[1].GetE() != 0 )  {  Error += GetAsymmUnit().Axes()[1].GetE()/GetAsymmUnit().Axes()[1].GetV(); j++; }
          if( GetAsymmUnit().Axes()[2].GetE() != 0 )  {  Error += GetAsymmUnit().Axes()[2].GetE()/GetAsymmUnit().Axes()[2].GetV(); j++; }
          if( GetAsymmUnit().Angles()[0].GetE() != 0 ){  Error += GetAsymmUnit().Angles()[0].GetE()/GetAsymmUnit().Angles()[0].GetV(); j++; }
          if( GetAsymmUnit().Angles()[1].GetE() != 0 ){  Error += GetAsymmUnit().Angles()[1].GetE()/GetAsymmUnit().Angles()[1].GetV(); j++; }
          if( GetAsymmUnit().Angles()[2].GetE() != 0 ){  Error += GetAsymmUnit().Angles()[2].GetE()/GetAsymmUnit().Angles()[2].GetV(); j++; }
          if( j != 0 )
            Error /= j;
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

  void AddVar(float val);
  bool InsExists(const olxstr &Name);
  inline int InsCount()  const                {  return Ins.Count();  }
  inline const olxstr& InsName(int i) const {  return Ins.String(i);  }
  inline const TInsList& InsParams(int i)     {  return *Ins.Object(i); }
  void DelIns(int i);
  void FixUnit();
  void DeleteAtom(TCAtom *CA);

  virtual IEObject* Replicate()  const {  return new TIns(AtomsInfo);  }
};

EndXlibNamespace()
#endif
