/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_strbuf_H
#define __olx_sdl_strbuf_H
// no namespace - to be included into
/* string buffer, reuses memory allocated by any posted olxstr entry */
template <typename char_t, class str_t>
class TTStrBuffer {
  struct Entry {
    typename str_t::Buffer* Data;
    size_t Start, Length;
    Entry* Next;
  };
  size_t _Length;
  Entry *Head, *Tail;
  void _Clear() {
    Entry* en = Head;
    while (en != 0) {
      Head = en->Next;
      if (--en->Data->RefCnt == 0) {
        delete en->Data;
      }
      delete en;
      en = Head;
    }
  }

public:
  TTStrBuffer() {
    Tail = Head = 0;
    _Length = 0;
  }

  TTStrBuffer(const str_t& v) {
    Tail = Head = new Entry();
    Tail->Data = v.Data_();
    Tail->Start = v.Start_();
    _Length = Tail->Length = v.Length();
    Tail->Next = 0;
    Tail->Data->RefCnt++;
  }

  TTStrBuffer(const TTStrBuffer& v) {
    Tail = Head = 0;
    _Length = 0;
    this->operator << (v);
  }

  virtual ~TTStrBuffer()  { _Clear(); }

  void Clear()  {
    _Clear();
    Head = Tail = 0;
    _Length = 0;
  }

  TTStrBuffer& operator << (const str_t& v) {
    if (v.Data_() == 0) {
      return *this;
    }
    if (Head == 0) {
      Tail = Head = new Entry;
    }
    else {
      Tail->Next = new Entry;
      Tail = Tail->Next;
      Tail->Next = 0;
    }
    Tail->Data = v.Data_();
    Tail->Start = v.Start_();
    _Length += (Tail->Length = v.Length());
    Tail->Next = 0;
    Tail->Data->RefCnt++;
    return *this;
  }

  TTStrBuffer& operator << (const TTStrBuffer& v) {
    Entry *en = v.Head;
    while (en != 0) {
      if (Head == 0)
        Tail = Head = new Entry;
      else {
        Tail->Next = new Entry;
        Tail = Tail->Next;
        Tail->Next = 0;
      }
      Tail->Data = en->Data;
      Tail->Start = en->Start;
      _Length += (Tail->Length = en->Length);
      en = en->Next;
      Tail->Next = 0;
      Tail->Data->RefCnt++;
    }
    return *this;
  }
  // writes the EOL - size of v should be Length()+1
  char_t* Read(char_t* v) const {
    if (Head == 0) {
      v[0] = L'\0';
    }
    else {
      Entry* en = Head;
      size_t read = 0;
      while (en != 0) {
        olx_memcpy(&v[read], &en->Data->Data[en->Start], en->Length);
        read += en->Length;
        en = en->Next;
      }
      v[read] = L'\0';
    }
    return v;
  }
  // writes the EOL - size of v should be CalcSize()+1
  char_t* ReverseRead(char_t* v) const {
    if (Head != 0) {
      Entry* en = Head;
      size_t x = _Length - 1;
      while (en != 0) {
        for (size_t i = en->Length; i != 0; i--, x--) {
          v[x] = en->Data->Data[en->Start + i - 1];
        }
        en = en->Next;
      }
    }
    v[_Length] = L'\0';
    return v;
  }
  size_t Length() const {  return _Length;  }
  bool IsEmpty() const { return _Length == 0; }
  Entry *GetHead() {  return Head;  }
  const Entry *GetHead() const {  return Head;  }
  // tries to decrease the length of the last entry by inc
  TTStrBuffer& TrimTail(size_t inc = 1) {
    if (Tail != 0) {
      if (Tail->Length >= inc) {
        Tail->Length -= inc;
      }
    }
    return *this;
  }
};
#endif
