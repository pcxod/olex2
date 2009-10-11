// testa.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

//#include "conio.h"

#include "exception.h"
#include "efile.h"
#include "estrlist.h"
#include "xapp.h"
#include "log.h"

#include "ins.h"
#include "asymmunit.h"
#include "catom.h"

#include "symmlib.h"
#include "library.h"
#include "outstream.h"

#include <iostream>

int main(int argc, char* argv[])  {
  try  {
  char bf[256];
  CString str("hello");
  CString str1(str);
  CString str2(str);

  str << 'a' << 10 << ',' << 100L << ',' << CString(1000LL) << str2[2];
  str1[2] = 'G';
  int ind = str.IndexOf("ll");
  printf("index = %d", ind);
  str1 = str.SubString(2,4);
  printf("%s\n", str.Read(bf));
  str2 << str.SubString(2,5);
  printf("%s\n", str1.Read(bf));
  str1 << 78.90;
  printf("%s\n", str1.Read(bf));
  str1.Delete(0, 2);
  printf("deleted two chars %s\n", str1.Read(bf));
  str1.Insert( "inserted ", 0);
  printf("deleted two chars %s\n", str1.Read(bf));

  TCStrBuffer sb(str);
  sb << str1 << str2;
  sb << 900.678;
  printf("%s\n", sb.Read(bf));
  str = "hello, hell, hel, he, h";
  TStrList toks(str, ", ");
  for( int i=0; i < toks.Count(); i++ )
    printf("%s\n", toks[i].Read(bf));

  CString dst("10.3454345655645");
  double d = dst.ToDouble();
  printf("got %s result=%lf\n", dst.Read(bf), d);
  dst = "10.001e-10";
  d = dst.ToDouble();
  printf("got %s result=%le\n", dst.Read(bf), d);
  dst = "-10.001e-10";
  d = dst.ToDouble();
  printf("got %s result=%le\n", dst.Read(bf), d);

  dst = "number -10.001e+100";
  d = dst.SubStringFrom(6).ToDouble();
  printf("got %s result=%lf\n", dst.SubStringFrom(6).Read(bf), d);

  dst = ".00134e+10";
  d = dst.ToDouble();
  printf("got %s result=%lf\n", dst.Read(bf), d);

  dst = "+156";
  int i = dst.RadInt<int>();
  printf("got %s result=%d\n", dst.Read(bf), i);

  dst = "0xFFFF";
  i = dst.SafeInt<int>();
  printf("got %s result=%d\n", dst.Read(bf), i);

  dst = "0xEFEF";
  short s = dst.SafeInt<short>();
  printf("got %s result=%hd\n", dst.Read(bf), s);

  // dodgy thing but works as pointer is the first member of class...
  dst = dst + "_h,mm";
  printf("got %s \n", dst.Read(bf));
  }
  catch( TExceptionBase& exc )  {
    printf("An exception occured: %s\n", EsdlObjectName(exc).c_str() );
    printf("details: %s\n", exc.GetException()->GetFullMessage().c_str() );
  }

  wchar_t wbf[256];

  int     x;
  char     *mbchar    = (char *)calloc(1, sizeof( char));
  wchar_t  wchar      = L'\x00EF';

  WString ws(wchar);
  ws << "test" << L" hmm";
  int stdiff = ws.o_memcmp(L"abc", L"abd", 3);
  stdiff = ws.o_memcmp(L"abc", L"abC", 3);
  stdiff = ws.o_memcmpi(L"abc", L"abC", 3);
  stdiff = ws.o_memcmp("abc", L"abC", 3);
  stdiff = ws.o_memcmp(L"abc", "abC", 3);
  ws = "ABCKLLKLKL";
  stdiff = ws.Compare(L"abc");
  stdiff = ws.Comparei(L"abc");
  stdiff = ws.Compare("abc");
  stdiff = ws.Comparei("abc");
  stdiff = ws.StartsFrom("abc");
  stdiff = ws.EndsWith("abc");

  ws = L"-101e5";
  int wsd = ws.ToDouble();
  printf("\nws: %ls", ws.Read(wbf) );

  wchar_t   *pwcnull = NULL;
  wchar_t  *pwchar    = (wchar_t *)calloc(1,  sizeof( wchar_t ));

  printf ("Convert a wide character to multibyte character:\n");
  x = wctomb( mbchar, wchar);
  printf( "\tCharacters converted: %u\n", x);
  printf( "\tMultibyte character: %x\n\n", mbchar);

  printf ("Convert multibyte character back to a wide character:\n");

  x = mbtowc( pwchar, mbchar, MB_CUR_MAX );
  printf( "\tBytes converted: %u\n", x);
  printf( "\tWide character: %x\n\n", pwchar);
  printf( "\tWide character: %lc\n\n", wchar);

  printf ("Atempt to convert when target is NULL\n" );
  printf (" returns the length of the multibyte character:\n" );
  x = mbtowc (pwcnull, mbchar, MB_CUR_MAX );
  printf ( "\tlength of multibyte character:%u\n\n", x );

  printf ("Attempt to convert a NULL pointer to a" );
  printf (" wide character:\n" );
  mbchar = NULL;
  x = mbtowc (pwchar, mbchar, MB_CUR_MAX);

  printf( "\tBytes converted: %u\n", x );

  printf("\n...");
  std::cin.get();

  return 0;
}




