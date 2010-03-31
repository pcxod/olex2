#ifndef __olx_xl_xifloop_H
#define __olx_xl_xifloop_H
#include "xbase.h"
#include "etable.h"
#include "catom.h"
BeginXlibNamespace()

class TCif;

struct ICifCell  {
  virtual ~ICifCell()  {}
  virtual const TCAtom* GetAtomRef() const {  return NULL;  }
  virtual bool IsQuoted() const {  return false;  }
  virtual bool IsBlock() const {  return false;  }
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

struct StringListCifCell : public ICifCell {
  virtual bool IsBlock() const {  return true;  }
};

typedef TStrPObjList<olxstr,ICifCell*> TCifRow;
typedef TTTable<TCifRow> TCifLoopTable;
//---------------------------------------------------------------------------
class TCifLoop: public IEObject  {
  TCifLoopTable FTable;
  olxstr FComments;
public:
  TCifLoop()  {}
  virtual ~TCifLoop()  {  Clear();  }

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
  TCifRow& operator [] (size_t i)  {  return FTable[i];  }
  const TCifRow& operator [] (size_t i) const {  return FTable[i];  }

  // row sorter struct
  struct CifLoopSorter  {
  public:
    static int Compare(const TCifLoopTable::TableSort& r1, const TCifLoopTable::TableSort& r2);
  };
};

EndXlibNamespace()
#endif
