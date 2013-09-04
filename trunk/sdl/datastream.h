/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_DATA_STREAM_H
#define __OLX_DATA_STREAM_H
#include "ebase.h"
//disable 'partial virtual function override warning
#ifdef __INTEL_COMPILER
 #pragma warning( disable : 654 )
#endif
BeginEsdlNamespace()

class IDataInputStream;
class IDataOutputStream;

class IDataInputStream: public IInputStream {
  template <class T> inline T& readType(T& v) {
    Read(&v, sizeof(T));
    return v;
  }
public:
  virtual ~IDataInputStream() {}
  virtual void Read(void *Data, size_t size) = 0;
  virtual inline void Read(olxcstr& whr, size_t size) {
    whr.AppendFromStream(*this, size);
  }
  virtual void Read(olxwstr& whr, size_t size) {
    whr.AppendFromStream(*this, size);
  }
  template <class T> inline T Read() {
    T v;
    Read(&v, sizeof(T));
    return v;
  }
  olxcstr ReadLine() {
    char c;
    olxcstr rv;
    rv.SetIncrement(64);
    while( GetAvailableSizeT() > 0 ) {
      if( readType(c) == '\r' || c == '\n' ) {
        if( c == '\r' && GetAvailableSizeT() > 0 ) { // check what is next
          if( readType(c) != '\n' ) // rewind
            SetPosition(GetPosition()-1);
        }
        return rv;
      }
      rv << c;
    }
    return rv;
  }
  inline char& operator >> (char& v) { return readType(v); }
  inline unsigned char& operator >> (unsigned char& v) { return readType(v); }
  inline short int& operator >> (short int& v) { return readType(v); }
  inline unsigned short int& operator >> (unsigned short int& v) {
    return readType(v);
  }
  inline int& operator >> (int& v) { return readType(v); }
  inline unsigned int& operator >> (unsigned int& v) { return readType(v); }
  inline long int& operator >> (long int& v) { return readType(v); }
  inline unsigned long int& operator >> (unsigned long int& v) {
    return readType(v);
  }
  inline long long int& operator >> (long long int& v) {
    return readType(v);
  }
  inline unsigned long long int& operator >> (unsigned long long int& v) {
    return readType(v);
  }
  inline float& operator >> (float& v) { return readType(v); }
  inline double& operator >> (double& v) { return readType(v); }
  inline IDataOutputStream& operator >> (IDataOutputStream& v) {
    IInputStream::operator >> (v);
    return v;
  }
  inline olxcstr& operator >> (olxcstr& v) {
    v.FromBinaryStream(*this);
    return v;
  }
  inline olxwstr& operator >> (olxwstr& v) {
    v.FromBinaryStream(*this); return v;
  }
};

class IDataOutputStream : public IOutputStream {
  template <class T> IDataOutputStream& writeType(const T& v) {
    Write(&v, sizeof(T));
    return *this;
  }
protected:
  // stream's underlying new line...
  virtual inline size_t WritelnFor(const TIWString&) {
    return Write(WNewLineSequence());
  }
  virtual inline size_t WritelnFor(const wchar_t*) {
    return Write(WNewLineSequence());
  }
  virtual inline size_t WritelnFor(const TICString&) {
    return Write(CNewLineSequence());
  }
  virtual inline size_t WritelnFor(const char*) {
    return Write(CNewLineSequence());
  }
public:
  virtual ~IDataOutputStream() {}

  virtual size_t Write(const void *Data, size_t size) = 0;
  template <class T> inline size_t Write(const T& data) {
    return Write(data);
  }
  inline size_t Writecln(const void *Data, size_t size) {
    const size_t cnt = Write(Data, size);
    return cnt + Write(CNewLineSequence());
  }
  inline size_t Writecln() { return Write(CNewLineSequence()); }
  inline size_t Writewln(const void *Data, size_t size) {
    const size_t cnt = Write(Data, size);
    return cnt + Write(WNewLineSequence());
  }
  inline size_t Writewln() { return Write(WNewLineSequence()); }
  inline size_t Writeuln(const void *Data, size_t size) {
    const size_t cnt = Write(Data, size);
    return cnt + Write(NewLineSequence());
  }
  inline size_t Writeuln() { return Write(NewLineSequence()); }

  virtual inline size_t Write(const wchar_t* str, size_t sz) {
    return Write((const void *)str, sz*sizeof(wchar_t));
  }
  virtual size_t Write(const TIWString& str) {
    return Write((const void *)str.raw_str(), str.RawLen());
  }
  virtual size_t Write(const olxwstr& str) {
    return Write((const TIWString&)str);
  }

  virtual inline size_t Write(const char* str, size_t sz) {
    return Write((const void *)str, sz);
  }
  virtual size_t Write(const TICString& str) {
    return Write((const void *)str.raw_str(), str.RawLen());
  }
  virtual size_t Write(const olxcstr& str) {
    return Write((const TICString&)str);
  }

  inline size_t Write(const char* str) {
    return Write(str, olxstr::o_strlen(str));
  }
  inline size_t Write(const wchar_t* str) {
    return Write(str, olxstr::o_strlen(str));
  }

  inline size_t Writeln(const olxwstr& str) {
    return Writeln((const TIWString&)str);
  }
  inline size_t Writeln(const TIWString& str) {
    const size_t cnt = Write(str);
    return cnt + Write(WNewLineSequence());
  }
  inline size_t Writeln(const wchar_t *Data) {
    const size_t cnt = Write(Data, olxstr::o_strlen(Data));
    return cnt + WritelnFor(Data);
  }

  inline size_t Writeln(const olxcstr& str) {
    return Writeln((const TICString&)str);
  }
  inline size_t Writeln(const TICString& str) {
    const size_t cnt = Write(str);
    return cnt + WritelnFor(str);
  }
  inline size_t Writeln(const char* Data) {
    const size_t cnt = Write(Data, olxstr::o_strlen(Data));
    return cnt + WritelnFor(Data);
  }

  virtual void Flush() { }

  inline IDataOutputStream& operator << (char v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (unsigned char v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (short int v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (unsigned short int v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (int v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (unsigned int v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (long int v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (unsigned long int v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (long long int v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (unsigned long long int v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (float v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (double v) {
    return writeType(v);
  }
  inline IDataOutputStream& operator << (IInputStream& v) {
    IOutputStream::operator << (v);
    return *this;
  }
  inline IDataOutputStream& operator << (const olxcstr& v) {
    v.ToBinaryStream(*this);
    return *this;
  }
  inline IDataOutputStream& operator << (const olxwstr& v) {
    v.ToBinaryStream(*this);
    return *this;
  }
};

EndEsdlNamespace()
#endif
