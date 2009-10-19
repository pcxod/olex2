//----------------------------------------------------------------------------//
// namespace TEObjects: Stream
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef __OLX_DATA_STREAM_H
#define __OLX_DATA_STREAM_H
#include "ebase.h"

//disable 'partial virtual function override warning
#ifdef __INTEL_COMPILER
  #pragma warning( disable : 654 )
#endif
 
//#include "estrlist.h"
//TODO: fix that header and then re-include it #include "edlist.h"
//---------------------------------------------------------------------------

BeginEsdlNamespace()

class IDataInputStream;
class IDataOutputStream;

class IDataInputStream: public IInputStream  {
  template <class T> inline T& readType( T& v )  {
//    IInputStream::operator >> (v);
    Read( &v, sizeof(T) );
    return v;
  }

public:
  virtual ~IDataInputStream() {  ;  }
  virtual void Read(void *Data, size_t size) = 0;
  virtual inline void Read(olxcstr& whr, size_t size)  {
    whr.AppendFromStream(*this, size);
  }
  virtual void Read(olxwstr& whr, size_t size)  {
    whr.AppendFromStream(*this, size);
  }
  inline char&                   operator >> ( char& v )                   {  return readType(v);  }
  inline unsigned char&          operator >> ( unsigned char& v )          {  return readType(v);  }
  inline short int&              operator >> ( short int& v )              {  return readType(v);  }
  inline unsigned short int&     operator >> ( unsigned short int& v )     {  return readType(v);  }
  inline int&                    operator >> ( int& v )                    {  return readType(v);  }
  inline unsigned int&           operator >> ( unsigned int& v )           {  return readType(v);  }
  inline long int&               operator >> ( long int& v )               {  return readType(v);  }
  inline unsigned long int&      operator >> ( unsigned long int& v )      {  return readType(v);  }
  inline long long int&          operator >> ( long long int& v )          {  return readType(v);  }
  inline unsigned long long int& operator >> ( unsigned long long int& v ) {  return readType(v);  }
  inline float&                  operator >> ( float& v )                  {  return readType(v);  }
  inline double&                 operator >> ( double& v )                 {  return readType(v);  }
  inline IDataOutputStream&      operator >> ( IDataOutputStream& v )      {  IInputStream::operator >> (v);  return v;  }
  inline olxcstr&                operator >> ( olxcstr& v )                {  v.FromBinaryStream(*this);  return v;  }
  inline olxwstr&                operator >> ( olxwstr& v )                {  v.FromBinaryStream(*this);  return v;  }
};

class IDataOutputStream: public IOutputStream  {
  template <class T> IDataOutputStream& writeType(const T& v)  {
    Write(&v, sizeof(T));
    return *this;
  }
public:
  virtual ~IDataOutputStream() {  ;  }
  // to let derived classes to handle unicode
  virtual size_t Write(const void *Data, size_t size) = 0;
  virtual inline size_t Writenl(const void *Data, size_t size)  {
    size_t w = Write(Data, size);
    w += Write(NewLineSequence, NewLineSequenceLength);
    return w;
  }

  virtual size_t Write(const olxwstr& str)  {  return Write(str.raw_str(), str.RawLen());  }
  virtual size_t Write(const olxcstr& str)  {  return Write(str.raw_str(), str.RawLen());  }
  virtual size_t Write(const TTIString<char>& str)  {  return Write(str.raw_str(), str.RawLen());  }
  virtual size_t Write(const TTIString<wchar_t>& str)  {  return Write(str.raw_str(), str.RawLen());  }

  virtual size_t Writenl(const olxwstr& str)  {  return Writenl(str.raw_str(), str.RawLen());  }
  virtual size_t Writenl(const olxcstr& str)  {  return Writenl(str.raw_str(), str.RawLen());  }
  virtual size_t Writenl(const TTIString<char>& str)  {  return Writenl(str.raw_str(), str.RawLen());  }
  virtual size_t Writenl(const TTIString<wchar_t>& str)  {  return Writenl(str.raw_str(), str.RawLen());  }
  virtual void Flush()  { }

  inline IDataOutputStream& operator << ( char v )                   {  return writeType(v);  }
  inline IDataOutputStream& operator << ( short int v )              {  return writeType(v);  }
  inline IDataOutputStream& operator << ( unsigned short int v )     {  return writeType(v);  }
  inline IDataOutputStream& operator << ( int v )                    {  return writeType(v);  }
  inline IDataOutputStream& operator << ( unsigned int v )           {  return writeType(v);  }
  inline IDataOutputStream& operator << ( long int v )               {  return writeType(v);  }
  inline IDataOutputStream& operator << ( unsigned long int v )      {  return writeType(v);  }
  inline IDataOutputStream& operator << ( long long int v )          {  return writeType(v);  }
  inline IDataOutputStream& operator << ( unsigned long long int v ) {  return writeType(v);  }
  inline IDataOutputStream& operator << ( float v )                  {  return writeType(v);  }
  inline IDataOutputStream& operator << ( double v )                 {  return writeType(v);  }
  inline IDataOutputStream& operator << ( IInputStream& v )          {  IOutputStream::operator << (v);  return *this;  }
  inline IDataOutputStream& operator << ( const olxcstr& v )       {  v.ToBinaryStream(*this);  return *this;  }
  inline IDataOutputStream& operator << ( const olxwstr& v )       {  v.ToBinaryStream(*this);  return *this;  }

  virtual inline size_t Write(const char* str)       {  return Write(str, olxstr::o_strlen(str));  }
  virtual inline size_t Write(const wchar_t* str)    {  return Write(str, olxstr::o_strlen(str));  }
  virtual inline size_t Writenl(const char* str)     {  return Writenl(str, olxstr::o_strlen(str));  }
  virtual inline size_t Writenl(const wchar_t* str)  {  return Writenl(str, olxstr::o_strlen(str));  }
};

EndEsdlNamespace()
#endif
