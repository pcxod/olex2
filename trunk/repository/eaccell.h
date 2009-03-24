#ifndef eaccellH
#define eaccellH
#include "estlist.h"
#include "dataitem.h"
//---------------------------------------------------------------------------

template <class T>
  class TAccellList : public IEObject
  {
    TPSTypeList<int32_t, T> Entries;
  protected:         
    int32_t LastId;
  public:
    TAccellList()  {  LastId = 0;  }
    virtual ~TAccellList()  {  }

    inline void Clear()   {  Entries.Clear();  }

    T& GetValue( int32_t key )  {
      int ind = Entries.IndexOfComparable( key );
      if( ind == -1 )  throw TInvalidArgumentException(__OlxSourceInfo, "id");
      return Entries.GetObject(ind);
    }

    bool ValueExists( int32_t key )  {
      return (Entries.IndexOfComparable( key ) == -1) ? false : true;
    }

    void AddAccell(int32_t id, const T& value)  {
      int ind = Entries.IndexOfComparable( id );
      if( ind != -1 )
        throw TInvalidArgumentException(__OlxSourceInfo, "duplicate ids");
      Entries.Add( id, value);
      LastId = id;
    }
    void RemoveAccell( int32_t id ) {
      int ind = Entries.IndexOfComparable( id );
      if( ind == -1 )
        throw TInvalidArgumentException(__OlxSourceInfo, "id");
      Entries.Remove(ind);
    }

    inline int32_t GetLastId()  const  {  return LastId;  }
};
#endif
