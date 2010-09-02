#ifndef __olx_xl_cif_H
#define __olx_xl_cif_H
#include "xfiles.h"
#include "estrlist.h"
#include "symmparser.h"
#include "cifdata.h"
#include "cifloop.h"
#include "ciftab.h"
#include "cifdp.h"
BeginXlibNamespace()

class TCif: public TBasicCFile  {
private:
  cif_dp::TCifDP data_provider;
  size_t block_index;
  olxstr FWeightA, FWeightB;
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
  void Clear();
  //............................................................................
  //Load the object from a file.
  virtual void LoadFromStrings(const TStrList& Strings);
  //Saves the data to a file and returns true if successful and false in the case of failure
  virtual void SaveToStrings(TStrList& Strings);
  //Adopts the content of a file (asymmetric unit, loops, etc) to a specified source file
  virtual bool Adopt(TXFile& XF);
  //Finds a value by name
  cif_dp::ICifEntry* FindEntry(const olxstr& name) const {
    return (block_index == InvalidIndex) ? false :
      data_provider[block_index].param_map.Find(name, NULL);
  }
  template <class Entry> Entry* FindParam(const olxstr& name) const {
    return dynamic_cast<Entry*>(FindEntry(name));
  }
 /* Returns the value of the given param as a string. Mght have '\n' as lines separator */
  olxstr GetParamAsString(const olxstr& name) const;
  //Returns true if a specified parameter exists
  bool ParamExists(const olxstr& name) const;
  //Adds/Sets given parameter a value
  void SetParam(const olxstr& name, const cif_dp::ICifEntry& value);
  void SetParam(const olxstr& name, const olxstr& value, bool quoted)  {
    if( quoted )
      SetParam(name, cif_dp::cetNamedString(name, olxstr('\'') << value << '\''));
    else
      SetParam(name, cif_dp::cetNamedString(name, value));
  }
  void ReplaceParam(const olxstr& olx_name, const olxstr& new_name, const cif_dp::ICifEntry& value);
  void Rename(const olxstr& olx_name, const olxstr& new_name);
  // returns the number of parameters
  inline size_t ParamCount() const {
    return (block_index == InvalidIndex) ? 0 : data_provider[block_index].param_map.Count();
  }
  // returns the name of a specified parameter
  const olxstr& ParamName(size_t i) const {  return data_provider[block_index].param_map.GetKey(i);  }
  // returns the value of a specified parameter
  cif_dp::ICifEntry& ParamValue(size_t i) const {
    return *data_provider[block_index].param_map.GetValue(i);
  }
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
  inline const olxstr& GetDataName() const {
    return (block_index == InvalidIndex) ? EmptyString : data_provider[block_index].GetName();
  }
  /*Set the data name. You should specify only the data name, not data_DATANAME.
    The function is not affected by DataNameUpperCase function, and, hence, specify
    the character's case manually, if necessary. */
  //void SetDataName(const olxstr& D);
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
  TCifLoop& GetLoop(size_t i) const {  return *Loops.GetObject(i);  }
  //Returns a loop specified by name
  TCifLoop* FindLoop(const olxstr& L) const {
    const size_t i = Loops.IndexOf(L);
    return (i == InvalidIndex) ? NULL : Loops.GetObject(i);
}
  //Returns the name of a loop specified by the index
  inline const olxstr& GetLoopName(size_t i) const {  return Loops[i];  }
  // Returns the number of loops
  inline size_t LoopCount() const { return Loops.Count(); }
  // Adds a loop to current  file
  TCifLoop& AddLoop(const olxstr& name);
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
EndXlibNamespace()
#endif
