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
#undef AddAtom
BeginXlibNamespace()

  typedef TStrPObjList<olxstr,TCAtom*> TInsList;

class TIns: public TBasicCFile  {
private:
  TStrPObjList< olxstr, TInsList* > Ins;  // instructions
  olxstr Unit, Sfac;
  void HypernateIns(const olxstr &InsName, const olxstr &Ins, TStrList &Res);
  void HypernateIns(const olxstr &Ins, TStrList &Res);
  double   Error;    // mean error of cell parameters. Can be used for estimation of other lengths
  bool     LoadQPeaks;// true if Q-peaks should be loaded
  TVectorD FVars; // contains variables, used for calculation right UNIT
  TVectorD FWght;    // could be up to six parameters
  TVectorD FWght1;   // could be up to six parameters
  short    Iterations,    // number of refinement cycles
           Plan;    // number of Q-peaks from fourier
  olxstr HKLF,
           RefinementMethod,  // L.S. or CGLS
           SolutionMethod;
  double   Radiation;
protected:
  void SaveSfac(TStrList& list, int pos);
  TCAtom* ParseAtom(TStrList& toks, double partOccu, TAsymmUnit::TResidue* resi, TCAtom* atom = NULL);
  olxstr AtomToString(TCAtom* CA, int SfacIndex);
  olxstr CellToString();
  olxstr ZerrToString();
  void SaveHklSrc(TStrList& SL);
public:
  TIns(TAtomsInfo *S);
  virtual ~TIns();

  void Clear();

  DefPropP(double, Error)
  DefPropP(double, Radiation)

  DefPropC(olxstr, RefinementMethod)
  DefPropC(olxstr, SolutionMethod)
  DefPropP(short, Iterations)
  DefPropP(short, Plan)
  DefPropB(LoadQPeaks)

  inline TVectorD& Wght()   {  return FWght;  }
  inline TVectorD& Wght1()  {  return FWght1;  }
  inline TVectorD& Vars()   {  return FVars;  }
  inline olxstr& Hklf()     { return HKLF;  }

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
  void UpdateAtomsFromStrings(TCAtomPList& CAtoms, TStrList& SL, TStrList& Instructions);
  /* saves some atoms to a plain ins format with no headers etc; to be used with
    UpdateAtomsFromFile
  */
  bool SaveAtomsToStrings(const TCAtomPList& CAtoms, TStrList& SL, TSimpleRestraintPList* processed);
  void SaveRestraints(TStrList& SL, const TCAtomPList* atoms, TSimpleRestraintPList* processed, TAsymmUnit* au);
  template <class SC, class T>
    void ParseRestraints(TTStrList<SC,T>& SL, TAsymmUnit* au)  {
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
            TMatrixD* SymM = new TMatrixD(3,4);
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
            if( sr.GetListType() == rltBonds && (agroup.Count() == 0 || (agroup.Count()%2)!=0 ) )
              throw TInvalidArgumentException(__OlxSourceInfo, "wrong restraint parameters list");
            if( Toks[0].Comparei("FLAT") == 0 )  {  // a secial case again...
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

  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *XF);

  TInsList* FindIns(const olxstr &Name);
  void ClearIns();
  bool AddIns(const olxstr& Name);
  bool AddIns(const olxstr& Name, const TStrList& Params);
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
