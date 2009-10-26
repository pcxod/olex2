
#include "outstream.h"

#include <stdio.h>

void outstream_putc(char ch)  {  putchar(ch);  }
void outstream_putc(wchar_t ch)  {  putwchar(ch);  }

size_t TOutStream::Write(const olxstr& str)  {
  if( SkipPost )  {
    SkipPost = false;
    return 0;
  }
  for( size_t i=0; i < str.Length(); i++ )
    outstream_putc( str[i] );
  return str.Length();
}
//..............................................................................
size_t TOutStream::Writenl(const olxstr& str)  {
  if( SkipPost )  {
    SkipPost = false;
    return 0;
  }
  Write(str);
#ifdef _UNICODE  // un Linux unicode and mbs sreams are separated!
  outstream_putc(L'\n');
#else  
  outstream_putc('\n');
#endif
  return str.Length() + 1;
}
//..............................................................................

