#ifndef __olx_xl_xifloop_H
#define __olx_xl_xifloop_H
#include "xbase.h"
#include "etable.h"
#include "catom.h"
#include "cifdp.h"
BeginXlibNamespace()

class TCif;

struct AtomCifEntry : public cif_dp::IStringCifEntry {
  TCAtom* data;
  AtomCifEntry(const AtomCifEntry& v) : data(v.data)  {}
  AtomCifEntry(TCAtom* _data) : data(_data)  {}
  virtual size_t Count() const {  return 1;  }
  virtual const olxstr& operator [] (size_t i) const {  return  data->GetLabel();  }
  virtual const olxstr& GetComment() const {  return EmptyString;  }
  virtual cif_dp::ICifEntry* Replicate() const {  return new AtomCifEntry(*this);  }
  virtual void ToStrings(TStrList& list) const {
    if( list.IsEmpty() || (list.Last().String.Length() + data->GetLabel().Length() + 1 > 80) )
      list.Add(' ') << data->GetLabel();
    else
      list.Last().String << ' ' << data->GetLabel();
  }
  virtual olxstr GetStringValue() const {  return data->GetLabel();  }
};
//---------------------------------------------------------------------------
class TCifLoop: public IEObject  {
  cif_dp::cetTable& Table;
public:
  TCifLoop(cif_dp::cetTable& table) : Table(table)  {}
  virtual ~TCifLoop()  {}
  void Clear()  {  Table.Clear();  }
  olxstr GetLoopName() const {  return Table.GetName();  }
  // removes rows referencing to the given atom
  void DeleteAtom(TCAtom* atom);
  // Updates the content of the table; changes labels of the atoms
  void UpdateTable(const TCif& parent);
  //Returns the table representing the loop data. Use it to make tables or iterate through data.
  cif_dp::cetTable& GetDataProvider()  {  return Table;  }
  const cif_dp::cetTable& GetDataProvider() const {  return Table;  }
  const cif_dp::CifTable& GetTable() const {  return Table.GetData();  }
  void SetData(size_t i, size_t j, cif_dp::ICifEntry* data)  {
    Table.Set(i,j,data);
  }
  const cif_dp::CifRow& operator [] (size_t i) const {  return Table.GetData()[i];  }

  // row sorter struct
  struct CifLoopSorter  {
  public:
    static int Compare(const cif_dp::CifTable::TableSort& r1, const cif_dp::CifTable::TableSort& r2);
  };
};

EndXlibNamespace()
#endif
