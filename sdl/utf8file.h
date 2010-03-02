#ifndef __OLX__UTF8_FILE__
#define __OLX__UTF8_FILE__
#include "efile.h"
#include "eutf8.h"
#include "estrlist.h"

class TUtf8File : public TEFile  {
protected:
  TUtf8File(const olxstr& fn, const olxstr& attr, bool CheckHeader=true) : TEFile(fn, attr)  {
    if( Length() >= 3 )  {
      uint32_t header = 0;
      TEFile::Read(&header, 3);
      if( header != TUtf8::FileSignature )  {
        if( CheckHeader )
          throw TFunctionFailedException(__OlxSourceInfo, "invalid UTF8 file signature");
        else
          TEFile::SetPosition(0);
      }
    }
  }
public:
  // pointer must be deleted
  static TUtf8File* Create(const olxstr& name)  {  
    TUtf8File* file = new TUtf8File(name, "wb+");  
    ((TEFile*)file)->Write( &TUtf8::FileSignature, 3);
    return file;
  }
  
  // pointer must be deleted
  static TUtf8File* Open(const olxstr& name, bool CheckHeader)  {
    TUtf8File* file = new TUtf8File(name, "wb", CheckHeader);  
    return file;
  }
  // creates a file and writes data to it, closes it
  static void Create(const olxstr& name, const TIString& data)  {  
    TUtf8File file(name, "wb+");  
    ((TEFile&)file).Write( &TUtf8::FileSignature, 3);
    file.Write(data);
  }

  static bool IsUtf8File(const olxstr& fn)  {
    TEFile file(fn, 'r');
    if( file.Length() < 3 )  return false;
    uint32_t header;
    file.Read(&header, 3);
    return (header == TUtf8::FileSignature);
  }

  virtual inline size_t Write(const olxwstr &S)    {  return TEFile::Write( TUtf8::Encode(S) );  }
  virtual inline size_t Writenl(const olxwstr &S)  {  return TEFile::Writenl( TUtf8::Encode(S) );  }

  virtual inline size_t Write(const TTIString<wchar_t> &S)    {  return TEFile::Write( TUtf8::Encode(S) );  }
  inline size_t Writenl(const TTIString<wchar_t> &S)  {  return TEFile::Writenl( TUtf8::Encode(S) );  }

  virtual inline size_t Write(const wchar_t* bf)   { return TEFile::Write(TUtf8::Encode(bf));  }
  inline size_t Writenl(const wchar_t* bf) { return TEFile::Writenl(TUtf8::Encode(bf));  }

  virtual inline size_t Write(const wchar_t* bf, size_t size)   { return TEFile::Write( TUtf8::Encode(bf, size) );  }
  virtual inline size_t Writenl(const wchar_t* bf, size_t size) { return TEFile::Writenl( TUtf8::Encode(bf, size) );  }
  virtual inline size_t Write(const void* bf, size_t size) { return TEFile::Write(bf, size);  }
  virtual inline size_t Writenl(const void* bf, size_t size) { return TEFile::Writenl(bf, size);  }

  template <class T>
  static void ReadLines(IInputStream& io, TTStrList<olxwstr,T>& list, bool CheckHeader=true)  {
    if( io.GetSize() >= 3 )  {
      uint32_t header = 0;
      io.Read(&header, 3);
      if( header != TUtf8::FileSignature )  {
        if( CheckHeader )
          throw TFunctionFailedException(__OlxSourceInfo, "invalid UTF8 stream");
        else
          io.SetPosition(0);
      }
    }
    size_t fl = io.GetAvailableSizeT();
    char * bf = new char [fl+1];
    io.Read(bf, fl);
    list.Clear();
    list.Strtok( TUtf8::Decode(bf, fl), '\n', false);
    delete [] bf;
    for( size_t i=0; i < list.Count(); i++ )
      if( list[i].EndsWith('\r') )  
        list[i].SetLength( list[i].Length() -1 );
  }
  // returns one long string
  static olxwstr ReadAsString(IInputStream& io, bool CheckHeader=true)  {
    if( io.GetSize() >= 3 )  {
      uint32_t header = 0;
      io.Read(&header, 3);
      if( header != TUtf8::FileSignature )  {
        if( CheckHeader )
          throw TFunctionFailedException(__OlxSourceInfo, "invalid UTF8 stream");
        else
          io.SetPosition(0);
      }
    }
    size_t fl = io.GetAvailableSizeT();
    char * bf = new char [fl+1];
    io.Read(bf, fl);
    olxwstr rv = TUtf8::Decode(bf, fl);
    delete [] bf;
    return rv;
  }
  template <class T>
  static void ReadLines(const olxstr& fn, TTStrList<olxwstr,T>& list, bool CheckHeader=true)  {
    TUtf8File file(fn, "rb", CheckHeader);
    size_t fl = file.GetAvailableSizeT();
    char * bf = new char [fl+1];
    file.Read(bf, fl);
    list.Clear();
    list.Strtok(TUtf8::Decode(bf, fl), '\n', false);
    delete [] bf;
    for( size_t i=0; i < list.Count(); i++ )
      if( list[i].EndsWith('\r') )  
        list[i].SetLength( list[i].Length() -1 );
  }
  // returns one long string
  static olxwstr ReadAsString(const olxstr& fn, bool CheckHeader=true)  {
    TUtf8File file(fn, "rb", CheckHeader);
    size_t fl = file.GetAvailableSizeT();
    char * bf = new char [fl+1];
    file.Read(bf, fl);
    olxwstr rv = TUtf8::Decode(bf, fl);
    delete [] bf;
    return rv;
  }
  template <class T>
  static void WriteLines(const olxstr& fn, const TTStrList<olxwstr,T>& list, bool WriteHeader=true)  {
    TUtf8File file(fn, "wb+");
    if( WriteHeader )
      ((TEFile&)file).Write(&TUtf8::FileSignature, 3);
    for( size_t i=0; i < list.Count(); i++ )
      if( i+1 < list.Count() )  file.Writenl( list[i] );
      else                      file.Write( list[i] );
  }
  template <class T>
  static void WriteLines(IDataOutputStream& out, const TTStrList<olxwstr,T>& list, bool WriteHeader=true)  {
    if( WriteHeader )
      out.Write(&TUtf8::FileSignature, 3);
    for( size_t i=0; i < list.Count(); i++ )
      if( i+1 < list.Count() )  out.Writenl( list[i] );
      else                      out.Write( list[i] );
  }
  template <class T>
  static void WriteLines(IDataOutputStream& out, const TTStrList<olxcstr,T>& list, bool WriteHeader=false)  {
    if( WriteHeader )
      out.Write(&TUtf8::FileSignature, 3);
    for( size_t i=0; i < list.Count(); i++ )
      if( i+1 < list.Count() )  out.Writenl( list[i] );
      else                      out.Write( list[i] );
  }
  template <class T>
  static void WriteLines(const olxstr& fn, const TTStrList<olxcstr,T>& list, bool WriteHeader=false)  {
    TUtf8File file(fn, "wb+");
    if( WriteHeader )
      ((TEFile&)file).Write( TUtf8::FileSignature, 3);
    for( size_t i=0; i < list.Count(); i++ )
      if( i+1 < list.Count() )  file.Writenl( list[i] );
      else                      file.Write( list[i] );
  }

};

#endif
