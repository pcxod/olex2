#ifndef __olx_sdl_strbuf_H
#define __olx_sdl_strbuf_H
// no namespace - to be included into
/* string buffer, reuses memory allocated by any posted TCString entry */
template <typename char_t, class str_t>
class TTStrBuffer {
  struct Entry {
    typename str_t::Buffer* Data;
    size_t Start, Length;
    Entry* Next;
  };
  size_t _Length;
  Entry *Head, *Tail;
public:
  TTStrBuffer()  {
    Tail = Head = NULL;
    _Length = 0;
  }
  TTStrBuffer(const str_t& v)  {
    Tail = Head = new Entry;
    Tail->Data = v.Data_();
    Tail->Start = v.Start_();
    _Length = Tail->Length = v.Length();
    Tail->Next = NULL;
    Tail->Data->RefCnt++;
  }
  virtual ~TTStrBuffer()  {
    Entry* en = Head;
    while( en != NULL )  {
      Head = en->Next;
      if( --en->Data->RefCnt == 0 )
        delete en->Data;
      delete en;
      en = Head;
    }
  }

  TTStrBuffer& operator << (const str_t& v)  {
    if( v.Data_() == NULL )  return *this;
    if( Head == NULL )  {
      Tail = Head = new Entry;
    }
    else  {
      Tail->Next = new Entry;
      Tail = Tail->Next;
      Tail->Next = NULL;
    }
    Tail->Data = v.Data_();
    Tail->Start = v.Start_();
    _Length += (Tail->Length = v.Length());
    Tail->Next = NULL;
    Tail->Data->RefCnt++;
    return *this;
  }
  // writes the EOL - size of v should be Length()+1
  char_t *Read(char_t *v) const {
    if( Head == NULL )
      v[0] = L'\0';
    else  {
      Entry* en = Head;
      size_t read = 0;
      while( en != NULL )  {
        olx_memcpy(&v[read], &en->Data->Data[en->Start], en->Length);
        read += en->Length;
        en = en->Next;
      }
      v[read] = L'\0';
    }
    return v;
  }
  // writes the EOL - size of v should be CalcSize()+1
  char_t *ReverseRead(char_t *v) const {
    if( Head != NULL )  {
      Entry* en = Head;
      size_t x = _Length-1;
      while( en != NULL )  {
        for( size_t i=en->Length; i != 0; i--, x-- )
          v[x] = en->Data->Data[i-1];
        en = en->Next;
      }
    }
    v[_Length] = L'\0';
    return v;
  }
  size_t Length() const {  return _Length;  }
  Entry *GetHead() {  return Head;  }
  const Entry *GetHead() const {  return Head;  }
};
#endif
