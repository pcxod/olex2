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
    for( int i=0; i < Streams.Count(); i++ )
      Streams[i].A()->Writenl(str);
  }
  template <class SC, class T>
    void Add(const TTStrList<SC,T> &lst)  {
      for( int i=0; i < lst.Count(); i++ )
        for( int j=0; j < Streams.Count(); j++ )
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
    for( int i=0; i < Streams.Count(); i++ )
      if( Streams[i].GetA() == stream )  {
        Streams.Delete(i);
        return;
      }
  }
  // use this operators for unconditional 'printing'
  inline TLog& operator << (const olxstr &str)  {
    int ind = str.IndexOf('\n');
    olxstr tmp(str);
    while( ind >= 0 )  {
      for( int i=0; i < Streams.Count(); i++ )
        Streams[i].A()->Writenl(tmp.SubStringTo(ind));
      if( ind >= tmp.Length() )  break;
      tmp = tmp.SubStringFrom(ind+1);
      ind = tmp.IndexOf('\n');
    }
    if( !tmp.IsEmpty() )
      for( int i=0; i < Streams.Count(); i++ )
        Streams[i].A()->Write(str);
    return *this;
  }
  template <class SC, class T>
  inline TLog& operator << (const TTStrList<SC,T> &lst)  {
    Add(lst);
    return *this;
  }
//..............................................................................
  void Info(const olxstr &msg)  {
    if( !OnInfo->Execute(this, &TEGC::New<olxstr>(msg)) )
      Add(msg);
  }
//..............................................................................
  void Warning(const olxstr &msg)  {
    if( !OnWarning->Execute(this, &TEGC::New<olxstr>(msg)) )
      Add(msg);
  }
//..............................................................................
  void Error(const olxstr &msg)  {
    if( !OnError->Execute(this, &TEGC::New<olxstr>(msg)) )
      Add(msg);
  }
//..............................................................................
  void Exception(const olxstr &msg)  {
    if( !OnException->Execute(this, &TEGC::New<olxstr>(msg)) )
      Add(msg);
  }
//..............................................................................
  virtual void Flush()  {
    for( int i=0; i < Streams.Count(); i++ )
      Streams[i].GetA()->Flush();
  }

  class TActionQueue *OnInfo;
  class TActionQueue *OnWarning;
  class TActionQueue *OnError;
  class TActionQueue *OnException;
};

EndEsdlNamespace()
#endif
