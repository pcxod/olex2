#ifndef lstH
#define lstH

#include "xbase.h"
#include "symmat.h"
#include "bitarray.h"
#include "estrlist.h"

BeginXlibNamespace()

const short slstReflections = 1,
            slslRefineData  = 2;
struct TLstRef  {
  short H, K, L;
  double DF, Res, Fo, Fc;
  bool Deleted;
};

struct TLstSplitAtom  {
  olxstr AtomName;
  vec3d PositionA, PositionB;
};

struct TTrefTry  {
  double CFOM, RAlpha, NQual, SigmaM1, Mabs;
  long Try;
  TEBitArray Semivariants;
};

class TPattAtom {
  olxstr Name;
  vec3d Crd;
  double Occup;
public:
  TPattAtom()  {  Occup = 0;  }
  TPattAtom(const TPattAtom& A)  {
    Name = A.Name;
    Crd = A.Crd;
    Occup = A.Occup;
  }
  TPattAtom& operator = (const TPattAtom& A)  {
    Name = A.Name;
    Crd = A.Crd;
    Occup = A.Occup;
    return *this;
  }

  inline const olxstr& GetName() const {  return Name;  }
  inline vec3d& GetCrd()              {  return Crd;   }
  inline double GetOccup()  const        {  return Occup; }

  inline void SetName(const olxstr& n)  { Name = n;   }
  inline void SetOccup(double v)          { Occup = v;  }

};

class TLst: public IEObject  {
protected:
  double FR1, FR1a, FwR2, FS, FRS, FRint, FRsig;
  int FParams, FTotalRefs, FUniqRefs, FRefs4sig;
  double FPeak, FHole;
  TEValue<double> FlackParam;
  TTypeList<TLstRef> FDRefs;
  TTypeList<TTrefTry> TrefTries;
  TTypeList< TTypeList<TPattAtom> > PattSolutions;
  TTypeList<TLstSplitAtom> FSplitAtoms;
  TTypeList< AnAssociation2<olxstr, olxstr> > ErrorMsgs;
  bool FLoaded, _HasFlack;
public:
  TLst();
  void Clear();
  virtual ~TLst();
  bool LoadFromFile(const olxstr &FN);
  void SynchroniseOmits(class RefinementModel& rm);
  bool ExportHTML( const short Param, TStrList &Html, bool TableDef=true);
  inline int DRefCount() const {  return FDRefs.Count(); }
  inline TLstRef& DRef(int i)  {  return FDRefs[i]; }
  inline void DelRef(int i)    {  FDRefs.Delete(i);  }
  inline int SplitAtomCount() const       {  return FSplitAtoms.Count();  }
  inline TLstSplitAtom& SplitAtom(int i)  {  return FSplitAtoms[i]; }

  inline int TrefTryCount()  const {  return TrefTries.Count(); }
  inline TTrefTry& TrefTry(int i)  {  return TrefTries[i]; }

  inline int PattSolutionCount()  const {  return PattSolutions.Count(); }
  inline TTypeList<TPattAtom>& PattSolution(int i) {  return PattSolutions[i];  }

  inline bool IsLoaded()     const {  return FLoaded;  }
  inline int ErrMsgCount()   const {  return ErrorMsgs.Count();  }
  inline const olxstr& GetError(int i) const  {  return ErrorMsgs[i].GetA();  }
  inline const olxstr& GetCause(int i) const  {  return ErrorMsgs[i].GetB();  }

  inline double Rint() const   {  return FRint; }
  inline double Rsigma() const {  return FRsig; }
  inline double R1() const     {  return FR1; }
  inline double R1a() const    {  return FR1a; }
  inline double wR2() const    {  return FwR2; }
  inline double S() const      {  return FS; }
  inline double RS() const     {  return FRS; }
  inline int Params() const   {  return FParams; }
  inline int TotalRefs()const {  return FTotalRefs; }
  inline int UniqRefs() const {  return FUniqRefs; }
  inline int Refs4sig() const {  return FRefs4sig; }
  inline double Peak() const   {  return FPeak; }
  inline double Hole() const   {  return FHole; }
  inline bool HasFlack() const {  return _HasFlack; }
  inline const TEValue<double>& Flack() const {  return FlackParam;  }
};

EndXlibNamespace()
#endif
