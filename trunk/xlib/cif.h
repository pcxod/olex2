//---------------------------------------------------------------------------//
#ifndef cifH
#define cifH

#include "xfiles.h"
#include "etable.h"
#include "estrlist.h"

BeginXlibNamespace()

class TCif;
class TLinkedLoopTable;
struct TCifData  {
  TStrList *Data;
  bool String;  //specifies if the data is a string type, e.g. should be in commas
};
struct TCifLoopData  {
  bool String;  //specifies if the data is a string type, e.g. should be in commas
  TCAtom *CA;   ////contains a pointer to an atom if the cell is relevant to an atom label
  TCifLoopData()  {
    CA = NULL;
    String = false;
  }
  TCifLoopData(bool str)  {
    CA = NULL;
    String = str;
  }
  TCifLoopData(TCAtom* ca)  {
    CA = ca;
    String = false;
  }
};
  typedef TTTable< TStrPObjList<olxstr,TCifLoopData*> > TCifLoopTable;
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
  void SaveToStrings( TStrList& Strings ); // Saves the loop to a stringlist

  /* Returns a string, which is common for all columns. For example for the following loop:
   loop_
   _atom_site_label
   _atom_site_type_symbol,  ... the function returns "_atom_site". */
  olxstr GetLoopName();
  /* Returns the column name minus the the loop name (see above). Thus for "_atom_site_label",
    the name is "label". */
  olxstr ShortColumnName(int in);
  // Updates the content of the table; changes labels of the atoms
  void UpdateTable();
  //Returns the table representing the loop data. Use it to make tables or iterate through data.
  TCifLoopTable& Table()  {  return FTable; }
  void DeleteAtom( TCAtom *A );
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TCifValue : public IEObject  {
  TTypeList<vec3d> MatrixTranslations;
  TIntList MatrixIndexes;
  TCAtomPList   Atoms;
  TEValueD Value;
protected:
  olxstr FormatTranslation(const vec3d& v);
public:
  TCifValue()  {  ;  }
  virtual ~TCifValue()  { ;  }

  void AddAtom(TCAtom& A, const vec3d& Translation, int SymmIndex = 0 )  {
    Atoms.Add(&A);
    MatrixIndexes.Add( SymmIndex );
    MatrixTranslations.AddCCopy( Translation );
  }
  inline int Count()  const                      {  return Atoms.Count();  }
  inline int GetMatrixIndex(int index)  const    {  return MatrixIndexes[index];  }
  inline TCAtom& GetAtom(int index)  const       {  return *Atoms[index];  }
  inline const TEValueD& GetValue() const {  return Value;  }

  olxstr Format()  const;
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TCifDataManager  {
  TTypeList<TCifValue> Items;
public:
  TCifDataManager()  {  ;  }
  virtual ~TCifDataManager()  {  Clear();  }
  inline TCifValue& NewValue()  {  return Items.AddNew();  }

  // finds a cif value for a list of TSATOMS(!)
  TCifValue* Match( const TSAtomPList& Atoms );

  void Clear();

  inline int Count()  const  {  return Items.Count();  }
  inline const TCifValue& Item(int index)  const {  return Items[index];  }
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TCif: public TBasicCFile  {
private:
  TStrPObjList<olxstr,TCifData*> Lines,
                          Parameters;
  void Format();
  olxstr FDataName, FWeightA, FWeightB;
  TStrPObjList<olxstr,TCifLoop*> Loops; // LoopName + CifLoop
//  void SetDataName(const olxstr &S);
  bool FDataNameUpperCase;
  void Initialize();
  TCifDataManager DataManager;
public:
  TCif(TAtomsInfo *AI);
  virtual ~TCif();

  void Clear();  // Empties the content of current file
  //............................................................................
  //Load the object from a file. If the operation is successful, returns true and false otherwise
  virtual void LoadFromStrings(const TStrList& Strings);
  //Saves the data to a file and returns true if successful and false in the case of failure
  virtual void SaveToStrings(TStrList& Strings);
  //Adopts the content of a file (asymmetric unit, loops, etc) to a specified source file
  virtual bool Adopt(TXFile *XF);
  /*Adds a new parameter to the CIF. The Params is copied to a new object, so it
    should be deleted within the main body of a program  */
  bool AddParam(const olxstr &Param, TCifData *Params);
  /*Adds a new parameter to the CIF. If the parameter is a string object, then set
    the String parameter to true.  */
  bool AddParam(const olxstr &Param, const olxstr &Params, bool String);
  TCifData *FindParam(const olxstr &Param); //Returns full value of a parameter
 /*Returns the first string of the TCifData objects associated with a given parameter.
   Note that the data is stores in a olxstrList, which may contain more than one string.
   To get full information, use GetParam function instead.  */
  const olxstr& GetSParam(const olxstr &Param) const;
  bool ParamExists(const olxstr &Param);  //Returns true if a specified parameter exists
  bool SetParam(const olxstr &Param, TCifData *Params);
  // returns the number of parameters
  inline int ParamCount() const {  return Parameters.Count(); };
  // returns the name of a specified parameter
  const olxstr& Param(int index) const {  return Parameters.String(index); };
  // returns the name of a specified parameter
  TCifData* ParamValue(int index)        {  return Parameters.Object(index); };
  //............................................................................
  //Returns the data name of the file (data_XXX, returns XXX in this case)
  inline const olxstr& GetDataName() const {  return FDataName; }
  /*Set the data name. You should specify only the data name, not data_DATANAME.
    The function is not affected by DataNameUpperCase function, and, hence, specify
    the character's case manually, if necessary. */
  void SetDataName(const olxstr &D);
  /*Shows if the data name will appear in upper case or in a default case when
    current object is loaded from a file  */
  inline bool IsDataNameUpperCase()          const { return FDataNameUpperCase;  }
  /*Allows changing the case of the data name. The change takes place only when a
    file is being loaded. Use SetDataName function to change the data name  */
  inline void SetDataNameUpperCase(bool v)         { FDataNameUpperCase = v; }
  //............................................................................
  inline const olxstr& GetWeightA()        const {   return FWeightA;  }
  inline const olxstr& GetWeightB()        const {   return FWeightB;  }
  //............................................................................
  //Returns a loop specified by index
  TCifLoop& Loop(int i);
  //Returns a loop specified by name
  TCifLoop* FindLoop(const olxstr &L);
  //Returns the name of a loop specified by the index
  inline const olxstr& GetLoopName(int i)  const {  return Loops.String(i);  }
  // Returns the number of loops
  inline int LoopCount()                     const { return Loops.Count(); }
  // Adds a loop to current  file
  TCifLoop& AddLoop(const olxstr &Name);
  /* this is the only loop, which is not automatically created from structure data!
   If the loop does not exist it is automatically created
  */
  TCifLoop& PublicationInfoLoop();

  void DeleteAtom(TCAtom *A);
protected:
  void MultValue(olxstr &Val, const olxstr &N);
public:
  bool ResolveParamsFromDictionary(
    TStrList &Dic,   // the dictionary containing the cif fields
    olxstr &String,    // the string in which the parameters are stores
    char Open,           // OpenParamIndexClose: %10%, (10), ...
    char Close,
    olxstr (*ResolveExternal)(const olxstr& valueName) = NULL,
    bool DoubleTheta = true);
  bool CreateTable(TDataItem *TableDefinitions, TTTable<TStrList>& Table, smatd_list& SymmList);
  void Group();
  const TCifDataManager& GetDataManager()  const  {  return DataManager;  }

  virtual IEObject* Replicate()  const {  return new TCif(AtomsInfo);  }

};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
struct TLAtom; // for internal use by TLinkedLoopTable
struct TLBond;
struct TLAngle;
class  TLinkedLoop;
struct TLAtom  {
  olxstr Label;
  TEList *Bonds;
  TEList *Angles;
  TCAtom *CA;
};
struct TLBond  {
  olxstr Value, S2;
  TLAtom *A1, *A2;
  TLAtom *Another(TLAtom *A);
  bool operator == (TLBond * B);
};
struct TLAngle  {
  olxstr Value, S1, S3;
  TLAtom *A1, *A2, *A3;
  bool Contains(TLAtom *A);
  bool FormedBy(TLBond *B, TLBond *B1 );
};
//---------------------------------------------------------------------------
  //A class derived from the abstract class TListSort. Use the BondSort property SortType,
  //to change the way the tables produce with a call to MakeTable look like.
  //Possible values are:
  //slltLength: Length, Mr, Atom Label;
  //slltName: Atom Label, Length;
  //slltMw: Mr, Length, Atom Label
  //default value is slltLength. The sorting happens according to the sequence shown above.
class TLLTBondSort: public TListSort  {
public:
  short SortType;
  TLAtom *Atom; // must be initilaised before the call
  TStrList *Symmetry;
  int Compare(void *I, void *I1);
};
//---------------------------------------------------------------------------
class TLinkedLoopTable: public IEObject  {
  TEList *FAtoms, *FBonds, *FAngles;
  TCif *FCif;
protected:
  TLAtom *AtomByName(const olxstr &Name);
  TTTable< TStrList > Table;
  TLLTBondSort BondSort;
public:
  TLinkedLoopTable(TCif *C);
  virtual ~TLinkedLoopTable();

  inline int AtomCount() const   {  return FAtoms->Count(); }
  inline TLAtom *Atom(int index) {  return (TLAtom*)FAtoms->Item(index);  };
  /* Returns a table constructed for Atom. The Atom should represent a valid atom
   label in Cif->AsymmUnit. */
  TTTable<TStrList>* MakeTable(const olxstr &Atom);
  /*Transforms a symmetry code written like "22_565" to the symmetry operation
   corresponding to the code in a SYMM like view "x, 1+y, z" */
  static olxstr SymmCodeToSymm(TCif *Cif, const olxstr &Code);
};
EndXlibNamespace()

#endif
