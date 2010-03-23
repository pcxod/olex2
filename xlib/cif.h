#ifndef __olx_xl_cif_H
#define __olx_xl_cif_H
#include "xfiles.h"
#include "etable.h"
#include "estrlist.h"
#include "symmparser.h"

BeginXlibNamespace()

class TCif;
class TLinkedLoopTable;
struct TCifData  {
  TStrList *Data;
  bool Quoted;
};

struct ICifCell  {
  virtual ~ICifCell()  {}
  virtual const TCAtom* GetAtomRef() const {  return NULL;  }
  virtual bool IsQuoted() const {  return false;  }
};

struct AtomCifCell : public ICifCell {
  TCAtom* data;
  virtual const TCAtom* GetAtomRef() const {  return data;  }
  AtomCifCell(TCAtom* _data=NULL) : data(_data)  {}
};

struct StringCifCell : public ICifCell {
  bool data;
  virtual bool IsQuoted() const {  return data;  }
  StringCifCell(bool _data=false) : data(_data)  {}
};

typedef TStrPObjList<olxstr,ICifCell*> TCifRow;
typedef TTTable<TCifRow> TCifLoopTable;
//---------------------------------------------------------------------------
class TCifLoop: public IEObject  {
  TCifLoopTable FTable;
  olxstr FComments;
//  int FRowCount, FColCount;
public:
  TCifLoop();
  virtual ~TCifLoop();

  void Clear(); // Clears the content of the loop
  /* Parses strings and initialises the table. The strings should represent a valid loop data.
   If any field of the table is recognised as a valid atom label, that atom is assigned the
   corresponding data object (TCifLoopData). If labels have changed, call to the UpdateTable
   to update labels in the table. */
  void Format(TStrList& Data);
  void SaveToStrings(TStrList& Strings) const; // Saves the loop to a stringlist
  /* Returns a string, which is common for all columns. For example for the following loop:
   loop_
   _atom_site_label
   _atom_site_type_symbol,  ... the function returns "_atom_site". */
  olxstr GetLoopName() const;
  /* Returns the column name minus the the loop name (see above). Thus for "_atom_site_label",
    the name is "label". */
  olxstr ShortColumnName(size_t col_i) const;
  // removes rows havibng reference to the given atom
  void DeleteAtom(TCAtom* atom);
  // Updates the content of the table; changes labels of the atoms
  void UpdateTable(const TCif& parent);
  //Returns the table representing the loop data. Use it to make tables or iterate through data.
  TCifLoopTable& GetTable()  {  return FTable;  }
  const TCifLoopTable& GetTable() const {  return FTable;  }
  void SetData(size_t i, size_t j, ICifCell* data)  {
    if( FTable[i].GetObject(j) != NULL )
      delete FTable[i].GetObject(j);
    FTable[i].GetObject(j) = data;
  }
  // row sorter struct
  struct CifLoopSorter  {
  public:
    static int Compare(const TCifLoopTable::TableSort& r1, const TCifLoopTable::TableSort& r2);
  };
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TCifValue : public IEObject  {
  TTypeList<vec3d> MatrixTranslations;
  TIntList MatrixIndexes;
  TCAtomPList Atoms;
  TEValueD Value;
protected:
  olxstr FormatTranslation(const vec3d& v);
public:
  TCifValue()  {  }
  virtual ~TCifValue()  {  }

  void AddAtom(TCAtom& A, const vec3d& Translation, int SymmIndex = 0 )  {
    Atoms.Add(A);
    MatrixIndexes.Add( SymmIndex );
    MatrixTranslations.AddCCopy( Translation );
  }
  inline size_t Count() const {  return Atoms.Count();  }
  inline int GetMatrixIndex(size_t index) const {  return MatrixIndexes[index];  }
  inline TCAtom& GetAtom(size_t index) const {  return *Atoms[index];  }
  inline const TEValueD& GetValue() const {  return Value;  }
  void SetValue(const olxstr& v)  {  Value = v;  }
  olxstr Format()  const;
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TCifDataManager  {
  TTypeList<TCifValue> Items;
public:
  TCifDataManager()  {}
  virtual ~TCifDataManager()  {}
  TCifValue& NewValue()  {  return Items.AddNew();  }
  // finds a cif value for a list of TSATOMS(!)
  TCifValue* Match(const TSAtomPList& Atoms) const;
  void Clear()  {  Items.Clear();  }
  size_t Count() const {  return Items.Count();  }
  const TCifValue& Item(size_t index) const {  return Items[index];  }
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TCif: public TBasicCFile  {
private:
  TStrPObjList<olxstr,TCifData*> Lines, Parameters;
  void Format();
  olxstr FDataName, FWeightA, FWeightB;
  TStrPObjList<olxstr,TCifLoop*> Loops; // LoopName + CifLoop
//  void SetDataName(const olxstr& S);
  bool FDataNameUpperCase;
  void Initialize();
  TCifDataManager DataManager;
  smatd_list Matrices;
  olxdict<olxstr, size_t, olxstrComparator<true> > MatrixMap;
public:
  TCif();
  virtual ~TCif();

  void Clear();  // Empties the content of current file
  //............................................................................
  //Load the object from a file.
  virtual void LoadFromStrings(const TStrList& Strings);
  //Saves the data to a file and returns true if successful and false in the case of failure
  virtual void SaveToStrings(TStrList& Strings);
  //Adopts the content of a file (asymmetric unit, loops, etc) to a specified source file
  virtual bool Adopt(TXFile& XF);
  /*Adds a new parameter to the CIF. The Params is copied to a new object, so it
    should be deleted within the main body of a program  */
  bool AddParam(const olxstr& Param, TCifData* Params);
  /*Adds a new parameter to the CIF. If the parameter is a string object, then set
    the String parameter to true.  */
  bool AddParam(const olxstr& Param, const olxstr& Params, bool String);
  TCifData *FindParam(const olxstr& Param) const; //Returns full value of a parameter
 /*Returns the first string of the TCifData objects associated with a given parameter.
   Note that the data is stores in a olxstrList, which may contain more than one string.
   To get full information, use GetParam function instead.  */
  const olxstr& GetSParam(const olxstr& Param) const;
  bool ParamExists(const olxstr& Param);  //Returns true if a specified parameter exists
  bool SetParam(const olxstr& Param, TCifData* Params);
  // returns the number of parameters
  inline size_t ParamCount() const {  return Parameters.Count(); };
  // returns the name of a specified parameter
  const olxstr& Param(size_t index) const {  return Parameters[index]; };
  // returns the name of a specified parameter
  TCifData* ParamValue(size_t index)  {  return Parameters.GetObject(index); };
  // matrics access functions
  size_t MatrixCount() const {  return Matrices.Count();  }
  const smatd& GetMatrix(size_t i) const {  return Matrices[i];  }
  const smatd& GetMatrixById(const olxstr& id) const {
    size_t id_ind = MatrixMap.IndexOf(id);
    if( id_ind == InvalidIndex )
      throw TInvalidArgumentException(__OlxSrcInfo, "matrix id");
    return Matrices[MatrixMap.GetValue(id_ind)];
  }
  // special for CIF dues to the MatrixMap...
  smatd SymmCodeToMatrix(const olxstr& code) const;
  /*Transforms a symmetry code written like "22_565" to the symmetry operation
   corresponding to the code in a SYMM like view "x, 1+y, z" */
  olxstr SymmCodeToSymm(const olxstr& Code) const {
    return TSymmParser::MatrixToSymm(SymmCodeToMatrix(Code));
  }
  //............................................................................
  //Returns the data name of the file (data_XXX, returns XXX in this case)
  inline const olxstr& GetDataName() const {  return FDataName; }
  /*Set the data name. You should specify only the data name, not data_DATANAME.
    The function is not affected by DataNameUpperCase function, and, hence, specify
    the character's case manually, if necessary. */
  void SetDataName(const olxstr& D);
  /*Shows if the data name will appear in upper case or in a default case when
    current object is loaded from a file  */
  inline bool IsDataNameUpperCase() const { return FDataNameUpperCase;  }
  /*Allows changing the case of the data name. The change takes place only when a
    file is being loaded. Use SetDataName function to change the data name  */
  inline void SetDataNameUpperCase(bool v)  { FDataNameUpperCase = v; }
  //............................................................................
  inline const olxstr& GetWeightA() const {  return FWeightA;  }
  inline const olxstr& GetWeightB() const {  return FWeightB;  }
  //............................................................................
  //Returns a loop specified by index
  TCifLoop& Loop(size_t i);
  //Returns a loop specified by name
  TCifLoop* FindLoop(const olxstr& L);
  //Returns the name of a loop specified by the index
  inline const olxstr& GetLoopName(size_t i) const {  return Loops[i];  }
  // Returns the number of loops
  inline size_t LoopCount() const { return Loops.Count(); }
  // Adds a loop to current  file
  TCifLoop& AddLoop(const olxstr& Name);
  /* this is the only loop, which is not automatically created from structure data!
   If the loop does not exist it is automatically created
  */
  TCifLoop& GetPublicationInfoLoop();
protected:
  static void MultValue(olxstr& Val, const olxstr& N);
public:
  bool ResolveParamsFromDictionary(
    TStrList &Dic,   // the dictionary containing the cif fields
    olxstr& String,    // the string in which the parameters are stores
    olxch Quote,           // %10%, #10#, ...
    olxstr (*ResolveExternal)(const olxstr& valueName) = NULL,
    bool DoubleTheta = true) const;
  bool CreateTable(TDataItem* TableDefinitions, TTTable<TStrList>& Table, smatd_list& SymmList) const;
  void Group();
  const TCifDataManager& GetDataManager() const {  return DataManager;  }

  virtual IEObject* Replicate() const {  return new TCif;  }

};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TMultiCifManager  {
  struct cif_content  {
    olxstr name;
    TStrList content;
  };
  olxstr FileName;
  TTypeList<cif_content> data;
public:
  TMultiCifManager()  {}
  void LoadFromFile(const olxstr& file_name);
  void Save();

  size_t Count() const {  return data.Count();  }
  const TStrList& GetContent(size_t i) const {  return data[i].content;  }
  const olxstr& GetName(size_t i) const {  return data[i].name;  }
  void Update(size_t i, TCif& cif)  {
    data[i].content.Clear();
    cif.SaveToStrings(data[i].content);
    data[i].name = cif.GetDataName();
  }
  size_t IndexOf(const olxstr& data_name) const {
    for( size_t i=0; i < data.Count(); i++ )
      if( data[i].name.Equalsi(data_name) )
        return i;
    return InvalidIndex;
  }

};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
struct TLAtom; // for internal use by TLinkedLoopTable
struct TLBond;
struct TLAngle;
class  TLinkedLoop;
struct TLAtom  {
  olxstr Label;
  TPtrList<TLBond> Bonds;
  TPtrList<TLAngle> Angles;
  TCAtom &CA;
  TLAtom(TCAtom& ca) : CA(ca)  {}
  bool operator == (const TLAtom& a) const {  return this == &a;  }
};
struct TLBond  {
  olxstr Value, S2;
  TLAtom &A1, &A2;
  TLBond(TLAtom& a1, TLAtom& a2) : A1(a1), A2(a2)  {}
  const TLAtom& Another(TLAtom& A) const;
  bool operator == (const TLBond &B) const;
};
struct TLAngle  {
  olxstr Value, S1, S3;
  TLAtom &A1, &A2, &A3;
  TLAngle(TLAtom& a1, TLAtom& a2, TLAtom& a3) :A1(a1), A2(a2), A3(a3) {}
  bool Contains(const TLAtom& A) const;
  bool FormedBy(const TLBond& B, const TLBond& B1) const;
};
//---------------------------------------------------------------------------
class TLLTBondSort  {
public:
  short SortType;
  TLAtom &Atom; // must be initilaised before the call
  const TStrList &Symmetry;
  TLLTBondSort(TLAtom& atom, const TStrList& symm, short sort_type) :
    Atom(atom), Symmetry(symm), SortType(sort_type)  {}
  int Compare(const TLBond *I, const TLBond *I1);
};
//---------------------------------------------------------------------------
class TLinkedLoopTable: public IEObject  {
  TPtrList<TLAtom> FAtoms;
  TPtrList<TLBond> FBonds;
  TPtrList<TLAngle> FAngles;
  TCif *FCif;
protected:
  TLAtom& AtomByName(const olxstr& Name);
  TTTable< TStrList > Table;
public:
  TLinkedLoopTable(TCif *C);
  virtual ~TLinkedLoopTable();

  size_t AtomCount() const {  return FAtoms.Count(); }
  TLAtom *Atom(size_t index) {  return FAtoms[index];  };
  /* Returns a table constructed for Atom. The Atom should represent a valid atom
   label in Cif->AsymmUnit. */
  TTTable<TStrList>* MakeTable(const olxstr& Atom);
};
EndXlibNamespace()

#endif
