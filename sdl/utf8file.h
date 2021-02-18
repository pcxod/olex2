/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_utf8file_H
#define __olx_sdl_utf8file_H
#include "efile.h"
#include "eutf8.h"
#include "estrlist.h"
BeginEsdlNamespace()

class TUtf8File : public TEFile {
protected:
  static bool validate_stream(IInputStream& io, bool check_header) {
    io.SetPosition(0);
    if (io.GetSize() >= 3) {
      uint32_t header = 0;
      io.Read(&header, 3);
      if (header != TUtf8::GetFileSignature()) {
        if (check_header) return false;
        else
          io.SetPosition(0);
      }
    }
    return true;
  }
  TUtf8File(const olxstr& fn, const olxstr& attr, bool CheckHeader = true)
    : TEFile(fn, attr)
  {
    if (!validate_stream(*this, CheckHeader)) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "invalid UTF8 file signature");
    }
  }
  virtual size_t WritelnFor(const TIWString&) {
    return Write(CNewLineSequence());
  }
  virtual size_t WritelnFor(const wchar_t*) {
    return Write(CNewLineSequence());
  }
  virtual size_t WritelnFor(const TICString&) {
    return Write(CNewLineSequence());
  }
  virtual size_t WritelnFor(const char*) {
    return Write(CNewLineSequence());
  }

  static olxcstr GetlnFor(const TICString& l) {
    return CNewLineSequence();
  }

  static olxcstr GetlnFor(const TIWString& l) {
    return WNewLineSequence();
  }
public:
  // pointer must be deleted
  static TUtf8File* Create(const olxstr& name, bool write_header = true) {
    TUtf8File* file = new TUtf8File(name, "w+b");
    if (write_header) {
      ((TEFile*)file)->Write(&TUtf8::GetFileSignature(), 3);
    }
    return file;
  }

  // creates/opens, positioned at the begining
  static olx_object_ptr<TUtf8File> Open(
    const olxstr& name, const olxstr& attr, bool CheckHeader)
  {
    TUtf8File* file = new TUtf8File(name, attr, CheckHeader);
    return file;
  }
  // creates a file and writes data to it, closes it
  static void Create(const olxstr& name, const TIString& data,
    bool write_header = true)
  {
    TUtf8File file(name, "w+b");
    if (write_header)
      ((TEFile&)file).Write(&TUtf8::GetFileSignature(), 3);
    file.Write(data);
  }

  static bool IsUtf8File(const olxstr& fn) {
    try {
      TEFile file(fn, "rb");
      return validate_stream(file, true);
    }
    catch (...) {
      return false;
    }
  }

  size_t Write(const olxwstr& S) {
    uint64_t p = GetPosition();
    return static_cast<size_t>(TUtf8::Encode(S, *this).GetPosition() - p);
  }
  virtual size_t Write(const TIWString& S) {
    uint64_t p = GetPosition();
    return static_cast<size_t>(TUtf8::Encode(S, *this).GetPosition() - p);
  }
  size_t Write(const wchar_t* bf, size_t size) {
    uint64_t p = GetPosition();
    return static_cast<size_t>(TUtf8::Encode(bf, *this, size).GetPosition() - p);
  }
  virtual size_t Write(const void* bf, size_t size) {
    return TEFile::Write(bf, size);
  }

  static const_strlist ReadLines(IInputStream& io, bool CheckHeader = true) {
    if (!validate_stream(io, CheckHeader)) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "invalid UTF8 stream");
    }
    const size_t fl = io.GetAvailableSizeT();
    olx_array_ptr<char> bf(new char[fl + 1]);
    io.Read(bf, fl);
    TStrList list;
    list.Strtok(TUtf8::Decode(bf, fl), '\n', false);
    for (size_t i = 0; i < list.Count(); i++) {
      if (list[i].EndsWith('\r')) {
        list[i].SetLength(list[i].Length() - 1);
      }
    }
    return list;
  }

  static const_strlist ReadLines(const olxstr& fn, bool CheckHeader = true) {
    TEFile file(fn, "rb");
    return ReadLines(file, CheckHeader);
  }

  // returns one long string
  static olxwstr ReadAsString(IInputStream& io, bool CheckHeader = true) {
    if (!validate_stream(io, CheckHeader)) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "invalid UTF8 stream");
    }
    const size_t fl = io.GetAvailableSizeT();
    olx_array_ptr<char> bf(fl + 1);
    io.Read(bf, fl);
    olxwstr rv = TUtf8::Decode(bf, fl);
    return rv;
  }
  static olxwstr ReadAsString(const olxstr& fn, bool CheckHeader = true) {
    TEFile file(fn, "rb");
    return ReadAsString(file, CheckHeader);
  }

  template <class T>
  static void WriteLines(const olxstr& fn, const T& list,
    bool WriteHeader = false)
  {
    TUtf8File file(fn, "w+b");
    if (WriteHeader) {
      ((TEFile&)file).Write(&TUtf8::GetFileSignature(), 3);
    }
    if (list.IsEmpty()) {
      return;
    }
    const size_t sz = 16 * 1024;
    TEMemoryStream ms(sz);
    olxcstr ln = GetlnFor(list[0]);
    for (size_t i = 0; i < list.Count(); i++) {
      TUtf8::Encode(list[i], ms);
      ms.Write(ln.c_str(), ln.Length());
      if (ms.GetSize() > sz) {
        ms.SetPosition(0);
        file << ms;
        ms.Clear();
      }
    }
    if (ms.GetSize() > 0) {
      ms.SetPosition(0);
      file << ms;
    }
  }

};

EndEsdlNamespace()
#endif
