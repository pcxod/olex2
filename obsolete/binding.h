//---------------------------------------------------------------------------

//#ifdef __COOOO

#ifndef bindingH
#define bindingH

#include "xatom.h"
#include "xbond.h"
#include "sparser.h"
//---------------------------------------------------------------------------
template <class U>
  class TXAtomPropertyEvaluator: public TPropertyEvaluator<TXAtom*, U>  {
    IEvaluator *NewInstance(IDataProvider *dp)  {  return NULL;  }
  };
template <class U>
  class TXBondPropertyEvaluator: public TPropertyEvaluator<TXBond*, U>  {
    IEvaluator *NewInstance(IDataProvider *dp)  {  return NULL;  }
  };
template <class U>
  class TSAtomPropertyEvaluator: public TPropertyEvaluator<TSAtom*, U>  {
    IEvaluator *NewInstance(IDataProvider *dp)  {  return NULL;  }
  };
template <class U>
  class TSBondPropertyEvaluator: public TPropertyEvaluator<TSBond *, U>  {
    IEvaluator *NewInstance(IDataProvider *dp)  {  return NULL;  }
  };
template <class U>
  class TGlGroupPropertyEvaluator: public TPropertyEvaluator<TGlGroup*, U>  {
    IEvaluator *NewInstance(IDataProvider *dp)  {  return NULL;  }
  };
template <class U>
  class TBAInfoPropertyEvaluator: public TPropertyEvaluator<TBasicAtomInfo*, U>  {
    IEvaluator *NewInstance(IDataProvider *dp)  {  return NULL;  }
  };

template <class CollectionProviderClass, class EvaluatorClass>
  class TXAtomCollectionEvaluator:
    public TCollectionPropertyEvaluator<CollectionProviderClass, TXAtom*, EvaluatorClass>
  {
  public:
    TXAtomCollectionEvaluator(ACollection<CollectionProviderClass, TXAtom*> *C,
      IEvaluator *itr, TXAtomPropertyEvaluator<EvaluatorClass> *PE):
      TCollectionPropertyEvaluator<CollectionProviderClass, TXAtom*, EvaluatorClass>(C, itr, PE)  {  ;  }
  };

template <class CollectionProviderClass, class EvaluatorClass>
  class TSAtomCollectionEvaluator:
    public TCollectionPropertyEvaluator<CollectionProviderClass, TSAtom*, EvaluatorClass>
  {
  public:
    TSAtomCollectionEvaluator(ACollection<CollectionProviderClass, TSAtom*> *C,
      IEvaluator *itr, TSAtomPropertyEvaluator<EvaluatorClass> *PE):
      TCollectionPropertyEvaluator<CollectionProviderClass, TSAtom*, EvaluatorClass>(C, itr, PE)  {  ;  }
  };

template <class CollectionProviderClass, class EvaluatorClass>
  class TSBondCollectionEvaluator:
    public TCollectionPropertyEvaluator<CollectionProviderClass, TSBond*, EvaluatorClass>
  {
  public:
    TSBondCollectionEvaluator(ACollection<CollectionProviderClass, TSBond*> *C,
      IEvaluator *itr, TSBondPropertyEvaluator<EvaluatorClass> *PE):
      TCollectionPropertyEvaluator<CollectionProviderClass, TSBond*, EvaluatorClass>(C, itr, PE)  {  ;  }
  };

/******************************************************************************/
// object collections
template <class CollectionProviderClass>
  class TSBondCollection: public ACollection<CollectionProviderClass, TSBond*>  {};

class TSAtomSBondCollection: public TSBondCollection<TSAtom*>  {
public:
  TSBond* Item(size_t index)  {  return &FData->Bond(index);  }
  virtual size_t Size() const {  return FData->BondCount();  }
};

class TXAtomSBondCollection: public TSBondCollection<TXAtom*>  {
public:
  TSBond* Item(size_t index)  {  return &FData->Atom().Bond(index);  }
  virtual size_t Size() const {  return FData->Atom().BondCount();  }
};

template <class CollectionProviderClass>
  class TSAtomCollection: public ACollection<CollectionProviderClass, TSAtom*>  {};

class TSAtomSAtomCollection: public TSAtomCollection<TSAtom*>  {
public:
  TSAtom* Item(size_t index)  {  return &FData->Node(index);  }
  virtual size_t Size() const {  return FData->NodeCount();  }
};

class TXAtomSAtomCollection: public TSAtomCollection<TXAtom*>  {
public:
  TSAtom* Item(size_t index)  {  return &FData->Atom().Node(index);  }
  virtual size_t Size() const {  return FData->Atom().NodeCount();  }
};
/******************************************************************************/
// TXAtom property evaluators
// label
class TXAtomLabelEvaluator: public TXAtomPropertyEvaluator<IStringEvaluator>  {
public:
  virtual const olxstr& EvaluateString() const {  return  FData->Atom().GetLabel();  }
};
// type
class TXAtomTypeEvaluator: public TXAtomPropertyEvaluator<IStringEvaluator>  {
public:
  virtual const olxstr& EvaluateString() const {  return  FData->Atom().GetAtomInfo().GetSymbol();  }
};
// part
class TXAtomPartEvaluator: public TXAtomPropertyEvaluator<IIntEvaluator>  {
public:
  virtual int EvaluateInt() const {  return  FData->Atom().CAtom().GetPart();  }
};
// afix
class TXAtomAfixEvaluator: public TXAtomPropertyEvaluator<IIntEvaluator>  {
public:
  virtual int EvaluateInt() const {  return  FData->Atom().CAtom().GetPart();  }
};
// Uiso
class TXAtomUisoEvaluator: public TXAtomPropertyEvaluator<IDoubleEvaluator>  {
public:
  virtual double EvaluateDouble() const {  return  FData->Atom().CAtom().GetUiso();  }
};
// peak
class TXAtomPeakEvaluator: public TXAtomPropertyEvaluator<IDoubleEvaluator>  {
public:
  virtual double EvaluateDouble() const {  return  FData->Atom().CAtom().GetQPeak();  }
};
// bond count
class TXAtomBCEvaluator: public TXAtomPropertyEvaluator<IIntEvaluator>  {
public:
  virtual int EvaluateInt()  const {  return  FData->Atom().BondCount();  }
};
// selected
class TXAtomSelectedEvaluator: public TXAtomPropertyEvaluator<IBoolEvaluator>  {
public:
  virtual bool EvaluateBool() const {  return  FData->IsSelected();  }
};
/******************************************************************************/
// TSAtom property evaluators
// label
class TSAtomLabelEvaluator: public TSAtomPropertyEvaluator<IStringEvaluator>  {
public:
  virtual const olxstr& EvaluateString() const {  return  FData->GetLabel();  }
};
// type
class TSAtomTypeEvaluator: public TSAtomPropertyEvaluator<IStringEvaluator>  {
public:
  virtual const olxstr& EvaluateString() const {  return  FData->GetAtomInfo().GetSymbol();  }
};
// part
class TSAtomPartEvaluator: public TSAtomPropertyEvaluator<IIntEvaluator>  {
public:
  virtual int EvaluateInt() const {  return  FData->CAtom().GetPart();  }
};
// afix
class TSAtomAfixEvaluator: public TSAtomPropertyEvaluator<IIntEvaluator>  {
public:
  virtual int EvaluateInt() const {  return  FData->CAtom().GetPart();  }
};
// Uiso
class TSAtomUisoEvaluator: public TSAtomPropertyEvaluator<IDoubleEvaluator>  {
public:
  virtual double EvaluateDouble() const {  return  FData->CAtom().GetUiso();  }
};
// peak
class TSAtomPeakEvaluator: public TSAtomPropertyEvaluator<IDoubleEvaluator>  {
public:
  virtual double EvaluateDouble()  const {  return  FData->CAtom().GetQPeak();  }
};
// bond count
class TSAtomBCEvaluator: public TSAtomPropertyEvaluator<IIntEvaluator>  {
public:
  virtual int EvaluateInt()  const {  return  FData->BondCount();  }
};
/******************************************************************************/
// TXBond property evaluators
// selected
class TXBondSelectedEvaluator: public TXBondPropertyEvaluator<IBoolEvaluator>  {
public:
  virtual bool EvaluateBool() const {  return  FData->IsSelected();  }
};
// length
class TXBondLengthEvaluator: public TXBondPropertyEvaluator<IDoubleEvaluator>  {
public:
  virtual double EvaluateDouble() const {  return  FData->Bond().Length();  }
};
// type
class TXBondTypeEvaluator: public TXBondPropertyEvaluator<IIntEvaluator>  {
public:
  virtual int EvaluateInt() const {  return  FData->Bond().GetType();  }
};
/******************************************************************************/
// TSBond property evaluators
// length
class TSBondLengthEvaluator: public TSBondPropertyEvaluator<IDoubleEvaluator>  {
public:
  virtual double EvaluateDouble() const {  return  FData->Length();  }
};
// type
class TSBondTypeEvaluator: public TSBondPropertyEvaluator<IIntEvaluator>  {
public:
  virtual int EvaluateInt() const {  return  FData->GetType();  }
};

/******************************************************************************/
// TSAtom collection property evaluators
// label
template <class CollectionProviderClass>
  class TSAtomCollectionLabelEvaluator:
    public TSAtomCollectionEvaluator<CollectionProviderClass, IStringEvaluator>
  {
    TSAtomLabelEvaluator LE;
  public:
    TSAtomCollectionLabelEvaluator(ACollection<CollectionProviderClass, TSAtom*> *C, IEvaluator *itr):
      TSAtomCollectionEvaluator<CollectionProviderClass, IStringEvaluator>(C, itr, &LE)  {  ;  }
  };
// type
template <class CollectionProviderClass>
  class TSAtomCollectionTypeEvaluator:
    public TSAtomCollectionEvaluator<CollectionProviderClass, IStringEvaluator>
  {
    TSAtomTypeEvaluator TE;
  public:
    TSAtomCollectionTypeEvaluator(ACollection<CollectionProviderClass, TSAtom*> *C, IEvaluator *itr):
      TSAtomCollectionEvaluator<CollectionProviderClass, IStringEvaluator>(C, itr, &TE)  {  ;  }
  };
// part
template <class CollectionProviderClass>
  class TSAtomCollectionPartEvaluator:
    public TSAtomCollectionEvaluator<CollectionProviderClass, IIntEvaluator>
  {
    TSAtomPartEvaluator LE;
  public:
    TSAtomCollectionPartEvaluator(ACollection<CollectionProviderClass, TSAtom*> *C, IEvaluator *itr):
      TSAtomCollectionEvaluator<CollectionProviderClass, IIntEvaluator>(C, itr, &LE)  {  ;  }
  };
// afix
template <class CollectionProviderClass>
  class TSAtomCollectionAfixEvaluator:
    public TSAtomCollectionEvaluator<CollectionProviderClass, IIntEvaluator>
  {
    TSAtomAfixEvaluator AE;
  public:
    TSAtomCollectionAfixEvaluator(ACollection<CollectionProviderClass, TSAtom*> *C, IEvaluator *itr):
      TSAtomCollectionEvaluator<CollectionProviderClass, IIntEvaluator>(C, itr, &AE)  {  ;  }
  };
// Uiso
template <class CollectionProviderClass>
  class TSAtomCollectionUisoEvaluator:
    public TSAtomCollectionEvaluator<CollectionProviderClass, IDoubleEvaluator>
  {
    TSAtomUisoEvaluator UE;
  public:
    TSAtomCollectionUisoEvaluator(ACollection<CollectionProviderClass, TSAtom*> *C, IEvaluator *itr):
      TSAtomCollectionEvaluator<CollectionProviderClass, IDoubleEvaluator>(C, itr, &UE)  {  ;  }
  };
// Peak
template <class CollectionProviderClass>
  class TSAtomCollectionPeakEvaluator:
    public TSAtomCollectionEvaluator<CollectionProviderClass, IDoubleEvaluator>
  {
    TSAtomPeakEvaluator PE;
  public:
    TSAtomCollectionPeakEvaluator(ACollection<CollectionProviderClass, TSAtom*> *C, IEvaluator *itr):
      TSAtomCollectionEvaluator<CollectionProviderClass, IDoubleEvaluator>(C, itr, &PE)  {  ;  }
  };
// bond count
template <class CollectionProviderClass>
  class TSAtomCollectionBCEvaluator:
    public TSAtomCollectionEvaluator<CollectionProviderClass, IIntEvaluator>
  {
    TSAtomBCEvaluator AE;
  public:
    TSAtomCollectionBCEvaluator(ACollection<CollectionProviderClass, TSAtom*> *C, IEvaluator *itr):
      TSAtomCollectionEvaluator<CollectionProviderClass, IIntEvaluator>(C, itr, &AE)  {  ;  }
  };
/******************************************************************************/
// TSBond collection property evaluators
// Length
template <class CollectionProviderClass>
  class TSBondCollectionLengthEvaluator:
    public TSBondCollectionEvaluator<CollectionProviderClass, IDoubleEvaluator>
  {
    TSBondLengthEvaluator LE;
  public:
    TSBondCollectionLengthEvaluator(ACollection<CollectionProviderClass, TSBond*> *C, IEvaluator *itr):
      TSBondCollectionEvaluator<CollectionProviderClass, IDoubleEvaluator>(C, itr, &LE)  {  ;  }
  };
// Type
template <class CollectionProviderClass>
  class TSBondCollectionTypeEvaluator:
    public TSBondCollectionEvaluator<CollectionProviderClass, IShortEvaluator>
  {
    TSBondTypeEvaluator TE;
  public:
    TSBondCollectionTypeEvaluator(ACollection<CollectionProviderClass, TSBond*> *C, IEvaluator *itr):
      TSBondCollectionEvaluator<CollectionProviderClass, IShortEvaluator>(C, itr, &TE)  {  ;  }
  };

class TSAtomClass  {
  TSAtomLabelEvaluator  FLabelEvaluator;
  TSAtomTypeEvaluator   FTypeEvaluator;
  TSAtomPartEvaluator   FPartEvaluator;
  TSAtomAfixEvaluator   FAfixEvaluator;
  TSAtomUisoEvaluator   FUisoEvaluator;
  TSAtomPeakEvaluator   FPeakEvaluator;
  TSAtomBCEvaluator     FBCEvaluator;
  TSAtomSAtomCollection FSAtomsCollection;
  TSAtomSBondCollection FSBondsCollection;
public:
  TSAtomClass();
  ~TSAtomClass()  {  }//FLabelEvaluator.;  }

};

class TXAtomClass  {
  TXAtomLabelEvaluator    FLabelEvaluator;
  TXAtomTypeEvaluator     FTypeEvaluator;
  TXAtomPartEvaluator     FPartEvaluator;
  TXAtomAfixEvaluator     FAfixEvaluator;
  TXAtomUisoEvaluator     FUisoEvaluator;
  TXAtomPeakEvaluator     FPeakEvaluator;
  TXAtomSelectedEvaluator FSelectedEvaluator;
  TXAtomBCEvaluator       FBCEvaluator;
  TXAtomSAtomCollection   FSAtomsCollection;
  TXAtomSBondCollection   FSBondsCollection;
public:
  TXAtomClass();
  ~TXAtomClass()  {  ;  }

};

class TXBondClass  {
  TXBondLengthEvaluator   FLengthEvaluator;
  TXBondTypeEvaluator     FTypeEvaluator;
  TXBondSelectedEvaluator FSelectedEvaluator;
public:
  TXBondClass();
  ~TXBondClass()  {  ;  }
};
class TSBondClass  {
  TSBondLengthEvaluator   FLengthEvaluator;
  TSBondTypeEvaluator     FTypeEvaluator;
public:
  TSBondClass();
  ~TSBondClass()  {  ;  }
};
#endif
//#endif
