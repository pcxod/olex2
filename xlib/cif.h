/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_cif_H
#define __olx_xl_cif_H
#include "xfiles.h"
#include "estrlist.h"
#include "symmparser.h"
#include "cifdata.h"
#include "ciftab.h"
#include "cifdp.h"
BeginXlibNamespace()

class TCif : public TBasicCFile {
private:
  cif_dp::TCifDP data_provider;
  size_t block_index;
  olxstr WeightA, WeightB;
  void Initialize();
  TCifDataManager DataManager;
  smatd_list Matrices;
  olxstr_dict<size_t, true> MatrixMap;
  // to be used inernally for locating atoms in the tables
  ConstPtrList<TCAtom> FindAtoms(const TStrList& names);
  bool has_duplicate_labels;
protected:
  static cif_dp::cetTable* LoopFromDef(cif_dp::CifBlock& dp,
    const TStrList& col_names);
  static cif_dp::cetTable* LoopFromDef(cif_dp::CifBlock& dp,
    const olxstr& col_names)
  {
    return LoopFromDef(dp, TStrList(col_names, ','));
  }
  void _LoadCurrent();
  size_t get_bix(size_t block_idx) const {
    size_t bix = block_idx == InvalidIndex ? block_index : block_idx;
    if (bix == InvalidIndex || bix >= data_provider.Count()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "CIF block index");
    }
    return bix;
  }
public:
  TCif();
  virtual ~TCif();
  void Clear();
  //............................................................................
  /*Load the object from a file */
  virtual void LoadFromStrings(const TStrList& Strings);
  /* valid only after loading / changing data block */
  bool HasDuplicateLabels() {
    return has_duplicate_labels;
  }
  /* Saves the data to a file and returns true if successful and false in the
  case of failure
  */
  virtual void SaveToStrings(TStrList& Strings);
  /* Saves the content into a TDataItem object which can later be saved to XML
  or other formats
  */
  void ToDataItem(TDataItem& d) const;
  /* Loads content from a TDataItem object
  */
  void FromDataItem(const TDataItem& i);
  /* Adopts the content of a file (asymmetric unit, loops, etc) to a specified
  source file. If the second argument is 0 - the atoms are taken from the AU
  otherwise - from the latttice (allows saving grown structures)
  */
  virtual bool Adopt(TXFile&, int);
  //Finds a value by name
  cif_dp::ICifEntry* FindEntry(const olxstr& name) const {
    return (block_index == InvalidIndex) ? 0 :
      data_provider[block_index].param_map.Find(name, 0);
  }
  template <class Entry> Entry* FindParam(const olxstr& name) const {
    return dynamic_cast<Entry*>(FindEntry(name));
  }
  /* Returns the value of the given param as a string. Might have '\n' as lines
  separator
  */
  olxstr GetParamAsString(const olxstr& name, size_t block_idx=InvalidIndex) const;
  //Returns true if a specified parameter exists
  template <typename Str> bool ParamExists(const Str& name,
    size_t block_idx=InvalidIndex) const
  {
    return data_provider[get_bix(block_idx)].param_map.HasKey(name);
  }
  /*Adds/Sets given parameter a value, the value is replicated, so can be
  created on stack
  */
  void SetParam(const cif_dp::ICifEntry& value);
  void SetParam(const olxstr& name, const olxstr& value, bool quoted) {
    if (quoted) {
      SetParam(cif_dp::cetString(name, olxstr('\'') << value << '\''));
    }
    else {
      SetParam(cif_dp::cetString(name, value));
    }
  }
  // removes the parameter by name and add the new one
  void ReplaceParam(const olxstr& name, const cif_dp::ICifEntry& value);
  // renames a parameter
  void Rename(const olxstr& old_name, const olxstr& new_name);
  // returns the number of parameters
  size_t ParamCount() const {
    return (block_index == InvalidIndex) ? 0
      : data_provider[block_index].param_map.Count();
  }
  // returns the name of a specified parameter
  const olxstr& ParamName(size_t i) const {
    return data_provider[block_index].param_map.GetKey(i);
  }
  // removes a parameters or a table by name
  template <class SC>
  bool Remove(const SC& name) {
    return data_provider[block_index].Remove(name);
  }
  // returns the value of a specified parameter
  cif_dp::ICifEntry& ParamValue(size_t i) const {
    return *data_provider[block_index].param_map.GetValue(i);
  }
  // matrices access functions
  size_t MatrixCount() const { return Matrices.Count(); }
  const smatd& GetMatrix(size_t i) const { return Matrices[i]; }
  const smatd& GetMatrixById(const olxstr& id) const {
    size_t id_ind = MatrixMap.IndexOf(id);
    if (id_ind == InvalidIndex) {
      throw TInvalidArgumentException(__OlxSrcInfo,
        olxstr("matrix id: '") << id << '\'');
    }
    return Matrices[MatrixMap.GetValue(id_ind)];
  }
  const smatd_list& GetMatrices() const { return Matrices; }
  // special for CIF dues to the MatrixMap...
  smatd SymmCodeToMatrix(const olxstr& code) const;
  /*Transforms a symmetry code written like "22_565" to the symmetry operation
   corresponding to the code in a SYMM like view "x, 1+y, z"
   */
  olxstr SymmCodeToSymm(const olxstr& Code) const {
    return TSymmParser::MatrixToSymm(SymmCodeToMatrix(Code));
  }
  olxstr MatrixToCode(const smatd &m) const;
  //............................................................................
  //Returns the data name of the file (data_XXX, returns XXX in this case)
  const olxstr& GetDataName() const {
    return (block_index == InvalidIndex) ? EmptyString()
      : data_provider[block_index].GetName();
  }
  //............................................................................
  //Returns a loop specified by index
  cif_dp::cetTable& GetLoop(size_t i) const {
    return *data_provider[block_index].table_map.GetValue(i);
  }
  // Finds an item in any of the loops and returns the loop and the item index
  olx_object_ptr<olx_pair_t<cif_dp::cetTable*, size_t> > FindLoopItem(const olxstr& name,
    size_t block_idx=InvalidIndex);
  //Returns a loop specified by name or NULL
  template <class T>
  cif_dp::cetTable* FindLoop(const T& name) const {
    if (block_index == InvalidIndex) {
      return 0;
    }
    return data_provider[block_index].table_map.Find(name, 0);
  }
  // traverses the data blocks to find loop by name
  template <class T>
  cif_dp::cetTable* FindLoopGlobal(const T& name, bool set_current) {
    for (size_t i = 0; i < data_provider.Count(); i++) {
      cif_dp::cetTable* l = data_provider[i].table_map.Find(name, 0);
      if (l != 0) {
        if (set_current) {
          SetCurrentBlock(i);
        }
        return l;
      }
    }
    return 0;
  }
  //Returns the name of a loop specified by the index
  const olxstr& GetLoopName(size_t i) const {
    return data_provider[block_index].table_map.GetValue(i)->GetName();
  }
  // Returns the number of loops
  size_t LoopCount() const {
    return (block_index == InvalidIndex) ? 0
      : data_provider[block_index].table_map.Count();
  }
  // return number of blocks with atoms
  size_t BlockCount() const { return data_provider.Count(); }
  // changes current block index, i.e. loads structure from different block
  void SetCurrentBlock(size_t i) {
    block_index = i;
    _LoadCurrent();
  }
  /* Sets current block and creates if specified to in the case the block does
  not exist. When a block is created, parent can be used to create save_blocks
  */
  void SetCurrentBlock(const olxstr& block_name, bool create = false,
    cif_dp::CifBlock* parent = 0)
  {
    cif_dp::CifBlock* cb = data_provider.Find(block_name);
    if (cb == 0) {
      if (create) {
        cb = &data_provider.Add(block_name, parent);
        block_index = data_provider.Count() - 1;
        return;
      }
    }
    if (cb != 0) {
      block_index = data_provider.IndexOf(*cb);
    }
    _LoadCurrent();
  }
  // renames current block
  void RenameCurrentBlock(const olxstr& new_name) {
    data_provider.Rename(data_provider[block_index].GetName(), new_name);
  }
  // returns given block
  const cif_dp::CifBlock& GetBlock(size_t i) const {
    if (i > data_provider.Count()) {
      throw TIndexOutOfRangeException(__OlxSourceInfo,
        i, 0, data_provider.Count());
    }
    return data_provider[i];
  }
  // returns current block index, might be InvalidIndex
  size_t GetBlockIndex() const { return block_index; }
  /* creates a new loop from comma separated column names.
  * The table will be replaced disregarding "replace" flag if the columns do not
  match.
  */
  cif_dp::cetTable& AddLoopDef(const olxstr& col_names, bool replace=false);
  /* adds a new table or a table with matching cols */
  bool Add(const cif_dp::cetTable& tab);
  /* this is the only loop, which is not automatically created from structure
  data! If the loop does not exist it is automatically created
  */
  cif_dp::cetTable& GetPublicationInfoLoop();
protected:
  static void MultValue(olxstr& Val, const olxstr& N);
public:
  bool ResolveParamsFromDictionary(
    TStrList& Dic,   // the dictionary containing the cif fields
    olxstr& String,    // the string in which the parameters are stores
    olxch Quote,           // %10%, #10#, ...
    olxstr(*ResolveExternal)(const olxstr& valueName) = 0,
    bool DoubleTheta = true) const;
  /* creates a table from a loop using provided table definition. Fills the
  list of used symmetry. label_options specify if the label suffix should be
  placed in brackets (1) and/or placed a superscript (4) or subscript (2)
  */
  bool CreateTable(TDataItem* TableDefinitions, TTTable<TStrList>& Table,
    smatd_list& SymmList, int label_options = 0) const;
  const TCifDataManager& GetDataManager() const { return DataManager; }
  virtual IOlxObject* Replicate() const { return new TCif; }
};
//---------------------------------------------------------------------------
struct AtomCifEntry : public cif_dp::IStringCifEntry {
  TCAtom& data;
  mutable olxstr label;
  bool save_part;
  AtomCifEntry(const AtomCifEntry& v)
    : data(v.data), save_part(false)
  {}
  AtomCifEntry(TCAtom& _data)
    : data(_data), save_part(false)
  {}
  virtual size_t Count() const { return 1; }
  virtual bool IsSaveable() const { return !data.IsDeleted(); }
  virtual size_t GetCmpHash() const { return data.GetId(); }
  virtual const olxstr& operator [] (size_t) const {
    return  (label = data.GetResiLabel());
  }
  virtual const olxstr& GetComment() const { return EmptyString(); }
  virtual cif_dp::ICifEntry* Replicate() const {
    return new AtomCifEntry(*this);
  }
  virtual void ToStrings(TStrList& list) const;
  virtual olxstr GetStringValue() const;
};

struct AtomPartCifEntry : public cif_dp::IStringCifEntry {
  const TCAtom& data;
  mutable olxstr tmp_val;
  AtomPartCifEntry(const AtomPartCifEntry& v)
    : data(v.data)
  {}
  AtomPartCifEntry(const TCAtom& _data)
    : data(_data)
  {}
  virtual size_t Count() const { return 1; }
  virtual bool IsSaveable() const { return !data.IsDeleted(); }
  virtual const olxstr& operator [] (size_t) const {
    return  (tmp_val = (int)data.GetPart());
  }
  virtual const olxstr& GetComment() const { return EmptyString(); }
  virtual cif_dp::ICifEntry* Replicate() const {
    return new AtomPartCifEntry(*this);
  }
  virtual void ToStrings(TStrList& list) const {
    if (data.GetPart() == 0) {
      tmp_val = '.';
    }
    else {
      tmp_val = (int)data.GetPart();
    }
    if (list.IsEmpty() ||
      (list.GetLastString().Length() + data.GetLabel().Length() + 1 > 80))
    {
      list.Add(' ') << tmp_val;
    }
    else {
      list.GetLastString() << ' ' << tmp_val;
    }
  }
  virtual olxstr GetStringValue() const {
    return (tmp_val = (int)data.GetPart());
  }
};

struct SymmCifEntry : public cif_dp::IStringCifEntry {
  const TCif& parent;
  smatd data;
  mutable olxstr tmp_val;
  SymmCifEntry(const SymmCifEntry& v)
    : parent(v.parent), data(v.data)
  {}
  SymmCifEntry(const TCif& parent, const smatd &_data)
    : parent(parent), data(_data)
  {}
  virtual size_t Count() const { return 1; }
  virtual bool IsSaveable() const { return true; }
  virtual size_t GetCmpHash() const { return data.GetId(); }
  virtual const olxstr& operator [] (size_t) const {
    if (tmp_val.IsEmpty()) {
      if (data.IsFirst()) {
        return (tmp_val = ".");
      }
      return  (tmp_val = parent.MatrixToCode(data));
    }
    return tmp_val;
  }
  virtual const olxstr& GetComment() const { return EmptyString(); }
  virtual cif_dp::ICifEntry* Replicate() const {
    return new SymmCifEntry(*this);
  }
  virtual void ToStrings(TStrList& list) const;
  virtual olxstr GetStringValue() const {
    return (*this)[0];
  }
};

EndXlibNamespace()
#endif
