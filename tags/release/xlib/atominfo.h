#ifndef atominfoH
#define atominfoH
#include "chembase.h"
#include "estrlist.h"
#include "tptrlist.h"
#include "ecomplex.h"

BeginChemNamespace()

const  short iHydrogenIndex    = 0,
             iBoronIndex       = 4,
             iCarbonIndex      = 5,
             iNitrogenIndex    = 6,
             iOxygenIndex      = 7,
             iFluorineIndex    = 8,
             iSodiumIndex      = 10,
             iMagnesiumIndex   = 11,
             iSiliconIndex     = 13,
             iPhosphorusIndex  = 14,
             iSulphurIndex     = 15,
             iChlorineIndex    = 16,
             iPotassiumIndex   = 18,
             iCalciumIndex     = 19,
             iQPeakIndex       = 104,
             iDeuteriumIndex   = 105;

class TIsotope: public ACollectionItem  {
  double Mr, W;
public:
  TIsotope()  {  Mr = W = 0;  }
  TIsotope(double mr, double w)  {  Mr = mr;  W = w;  }

  DefPropP(double, Mr)
  DefPropP(double, W)
};

typedef TTypeList<TIsotope> TIsotopeList;

class TBasicAtomInfo: public ACollectionItem  {
  TIsotopeList* Isotopes;  // list of isotopes
  olxstr Symbol,  // atom symbol, e.g. Mn
           Name;    // atom name e.g. carbon
  double Mr,    // molecular weight
         Rad,    // radius for bonds determination (WVan der Waals)
         Rad1,
         Rad2;
  int    DefColor;   // default colour
  short  Index, // index of the record in the list; for internal use
    Z;          // atomic number
  double Summ;
public:
  TBasicAtomInfo() : Isotopes(NULL) {}
  virtual ~TBasicAtomInfo() {  
    if( Isotopes != NULL ) 
      delete Isotopes;
  }

  inline size_t IsotopeCount()  const {  return (Isotopes==NULL) ? 0 : Isotopes->Count(); }
  inline TIsotope& GetIsotope(size_t index)  const  {  return Isotopes->Item(index);  }
  inline TIsotope& NewIsotope() {  return (Isotopes == NULL ? (Isotopes = new TIsotopeList) : Isotopes)->AddNew();  }

  inline bool operator == (const TBasicAtomInfo& bai)  const {  return Index == bai.Index;  }
  inline bool operator == (const short index)           const {  return Index == index;  }
  inline bool operator != (const TBasicAtomInfo& bai)  const {  return Index != bai.Index;  }
  inline bool operator != (const short index)           const {  return Index != index;  }
  inline bool operator < (const TBasicAtomInfo& bai)  const {  return Index < bai.Index;  }
  inline bool operator < (const short index)           const {  return Index < index;  }
  inline bool operator <= (const TBasicAtomInfo& bai)  const {  return Index <= bai.Index;  }
  inline bool operator <= (const short index)           const {  return Index <= index;  }
  inline bool operator > (const TBasicAtomInfo& bai)  const {  return Index > bai.Index;  }
  inline bool operator > (const short index)           const {  return Index > index;  }
  inline bool operator >= (const TBasicAtomInfo& bai)  const {  return Index >= bai.Index;  }
  inline bool operator >= (const short index)           const {  return Index >= index;  }

  DefPropC(olxstr, Symbol)
  DefPropC(olxstr, Name)

  DefPropP(double, Mr)
  DefPropP(double, Rad)
  DefPropP(double, Rad1)
  DefPropP(double, Rad2)

  DefPropP(short, Index)
  DefPropP(short, Z)
  DefPropP(int, DefColor)

  DefPropP(double, Summ)

  olxstr StrRepr() const;
};
typedef TPtrList<TBasicAtomInfo> TBAIPList;
typedef TTypeList<AnAssociation2<olxstr, double> > ContentList;
//---------------------------------------------------------------------------
class TAtomsInfo: public IEObject  {
private:
  TTypeList<TBasicAtomInfo> Data;
  // parses a string like BRhClO intp B, Rh, Cl and O; works for capitalised symbols only
  // and will not know how to treat BRH, but BRh
  void ParseSimpleElementStr(const olxstr& str, TStrList& toks) const;
  static TAtomsInfo* Instance;
public:
  // 21.06.2008, table translated into the code, so the fileName is not used
  TAtomsInfo(const olxstr& filename=EmptyString);
  virtual ~TAtomsInfo();
  size_t Count() const {  return Data.Count();  }
  void SaveToFile(const olxstr& fileName) const;
  // returns correpondinag value of the AtomsInfo list
  inline TBasicAtomInfo& GetAtomInfo(size_t index) const {  return Data[index];  }
  // returns correpondinag value of the AtomsInfo list
  TBasicAtomInfo* FindAtomInfoBySymbol(const olxstr& Symbol) const;
  // returns correpondinag value of the AtomsInfo list, Str can be "C10a" etc
  TBasicAtomInfo* FindAtomInfoEx(const olxstr& Str) const;
  // a simple check exact match is expected (case insesitive)
  bool IsElement(const olxstr& S) const {  return (FindAtomInfoBySymbol(S) != NULL);  }
  // checks if p is an element symbol, will correctly distinguis "C " and "Cd"
  bool IsAtom(const olxstr &C) const {  return (FindAtomInfoEx(C) != NULL);  }
  static bool IsHAtom(const TBasicAtomInfo& bai)  {
    return bai == iHydrogenIndex || bai == iDeuteriumIndex;
  }
  // checks if p is an element symbol, will correctly distinguis "C " and "Cd"
  static bool IsShortcut(const olxstr &c) {  
    return c.Equalsi("Ph") || c.Equalsi("Cp") || c.Equalsi("Me") ||
      c.Equalsi("Et") || c.Equalsi("Bu") || 
      c.Equalsi("Py") || c.Equalsi("Tf");  
  }
  static void ExpandShortcut(const olxstr& sh, ContentList& res, double cnt=1.0)  {
    TTypeList<AnAssociation2<olxstr, double> > shc;
    if( sh.Equalsi("Ph") )  {
      shc.AddNew("C", 6);
      shc.AddNew("H", 5);
    }
    else if( sh.Equalsi("Py") )  {
      shc.AddNew("C", 5);
      shc.AddNew("N", 1);
      shc.AddNew("H", 4);
    }
    else if( sh.Equalsi("Tf") )  {
      shc.AddNew("C", 1);
      shc.AddNew("S", 1);
      shc.AddNew("O", 2);
      shc.AddNew("F", 3);
    }
    else if( sh.Equalsi("Cp") )  {
      shc.AddNew("C", 5);
      shc.AddNew("H", 5);
    }
    else if( sh.Equalsi("Me") )  {
      shc.AddNew("C", 1);
      shc.AddNew("H", 3);
    }
    else if( sh.Equalsi("Et") )  {
      shc.AddNew("C", 2);
      shc.AddNew("H", 5);
    }
    else if( sh.Equalsi("Bu") )  {
      shc.AddNew("C", 4);
      shc.AddNew("H", 9);
    }
    else    // just add whatever is provided
      shc.AddNew(sh, 1);
    
    for( size_t i=0; i < shc.Count(); i++ )  {
      shc[i].B() *= cnt;
      bool found = false;
      for( size_t j=0; j < res.Count(); j++ )  {
        if( res[j].GetA().Equalsi(shc[i].GetA()) )  {
          res[j].B() += shc[i].GetB();
          found = true;
          break;
        }
      }
      if( !found )
        res.AddCCopy(shc[i]);
    }
  }
  /* parses a string like C37H41P2BRhClO into a list of element names and theur
    count
  */
  void ParseElementString(const olxstr& su, ContentList& res) const;
  inline static TAtomsInfo& GetInstance() {
    if( Instance == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "object is not initialised");
    return *Instance;
  }
  // combines dublicate types and expands shortcuts such as Me, Ph and Cp
  olxstr& NormaliseAtomString(olxstr& str) const;
};

EndChemNamespace()
#endif
