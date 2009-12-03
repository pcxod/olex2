//---------------------------------------------------------------------------//
// get a response
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifndef elogH
#define elogH

#include "tptrlist.h"
#include "egc.h"
#include "datastream.h"

BeginEsdlNamespace()

class TLog: public IEObject, public IDataOutputStream  {
  // stream, to delete at the end
  TArrayList<AnAssociation2<IDataOutputStream*, bool> > Streams;
  class TActionQList *FActions;
protected:
  virtual size_t Write(const void *Data, size_t size);
  virtual size_t Writenl(const void *Data, size_t size);
  virtual size_t GetSize() const {  return 0;  }
  virtual size_t GetPosition() const {  return 0;  }
  virtual void SetPosition(size_t newPos) {    }

  void Add(const olxstr &str)  {
    for( size_t i=0; i < Streams.Count(); i++ )
      Streams[i].A()->Writenl(str);
  }
  template <class SC, class T>
    void Add(const TTStrList<SC,T> &lst)  {
      for( size_t i=0; i < lst.Count(); i++ )
        for( size_t j=0; j < Streams.Count(); j++ )
          Streams[j].A()->Writenl( lst[i] );
    }
public:
  TLog();
  virtual ~TLog();
  // if own is true, the object will be deleted in the end
  inline void AddStream( IDataOutputStream* stream, bool own )  {
    Streams.Add(AnAssociation2<IDataOutputStream*, bool>(stream, own));
  }
  void RemoveStream( IDataOutputStream* stream )  {
    for( size_t i=0; i < Streams.Count(); i++ )
      if( Streams[i].GetA() == stream )  {
        Streams.Delete(i);
        return;
      }
  }
  // use this operators for unconditional 'printing'
  inline TLog& operator << (const olxstr &str)  {
    size_t ind = str.IndexOf('\n');
    olxstr tmp(str);
    while( ind != InvalidIndex )  {
      for( size_t i=0; i < Streams.Count(); i++ )
        Streams[i].A()->Writenl(tmp.SubStringTo(ind));
      if( (size_t)ind >= tmp.Length() )  break;
      tmp = tmp.SubStringFrom(ind+1);
      ind = tmp.IndexOf('\n');
    }
    if( !tmp.IsEmpty() )
      for( size_t i=0; i < Streams.Count(); i++ )
        Streams[i].A()->Write(tmp);
    return *this;
  }
  template <class SC, class T>
  inline TLog& operator << (const TTStrList<SC,T> &lst)  {
    Add(lst);
    return *this;
  }
//..............................................................................
  void Info(const olxstr &msg)  {
    if( !OnInfo->Enter(this, &TEGC::New<olxstr>(msg)) )
      Add(msg);
    OnInfo->Exit(this, NULL);
  }
//..............................................................................
  void Warning(const olxstr &msg)  {
    if( !OnWarning->Enter(this, &TEGC::New<olxstr>(msg)) )
      Add(msg);
    OnWarning->Exit(this, NULL);
  }
//..............................................................................
  void Error(const olxstr &msg)  {
    if( !OnError->Enter(this, &TEGC::New<olxstr>(msg)) )
      Add(msg);
    OnError->Exit(this, NULL);
  }
//..............................................................................
  void Exception(const olxstr &msg)  {
    if( !OnException->Enter(this, &TEGC::New<olxstr>(msg)) )
      Add(msg);
    OnException->Exit(this, NULL);
  }
//..............................................................................
  virtual void Flush()  {
    for( size_t i=0; i < Streams.Count(); i++ )
      Streams[i].GetA()->Flush();
  }

  class TActionQueue *OnInfo;
  class TActionQueue *OnWarning;
  class TActionQueue *OnError;
  class TActionQueue *OnException;
};

EndEsdlNamespace()
#endif
