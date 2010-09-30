//---------------------------------------------------------------------------//
// namespace TEObjects
// TEString - a dynamic array implementation of a string object
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
  #pragma hdrstop
#endif
#ifdef _MSC_VER
  #include <memory.h>
  #define strcasecmp _stricmp
  //#define strcpy strcpy_s
  //#define sprintf sprintf_s
#elif __BORLANDC__
  #define strcasecmp stricmp
#else
  #include <strings.h>
#endif

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "estring.h"
#include "estrlist.h"
#include "exception.h"
#include "typelist.h"

#include "library.h"

  const int AaDiff = 'A'-'a';

UseEsdlNamespace()
  const TEString& esdl::NullString = (const TEString&)(*(TEString*)NULL);
  const TEString& esdl::EmptyString = "";
  const TEString& esdl::TrueString = "true";
  const TEString& esdl::FalseString = "false";

//---------------------------------------------------------------------------
TEString::TEString()  {
  FIncrement = 5;
  FLength = 0;
  FCapacity = FLength + FIncrement;
  Allocate(NULL);
  FData[0] = '\0';
}
//..............................................................................
TEString::TEString(const TEString &E, int defcapacity)  {
  FIncrement = E.FIncrement;
  FLength = E.FLength;

  FCapacity = olx_max(FLength, defcapacity) + 1;

  Allocate(NULL);
  memcpy(FData, E.FData, FLength+1);
}
//..............................................................................
TEString::TEString(const char *S, int defcapacity)  {
  FIncrement = 5;
  FLength = strlen(S);
  FCapacity = olx_max(FLength + FIncrement, defcapacity);
  Allocate(NULL);
  strcpy(FData, S);
}
//..............................................................................
TEString::TEString(char C, int defcapacity)  {
  FIncrement = 5;
  if( defcapacity <=0 )  {
    FCapacity = olx_max(FIncrement+1, defcapacity);
    FLength = 1;
    Allocate(NULL);
    FData[0] = C;
  }
  else  {
    FCapacity = olx_max(FIncrement+1, defcapacity+FIncrement);
    FLength = defcapacity;
    Allocate(NULL);
    memset(FData, C, defcapacity);
  }
  FData[FLength] = '\0';
}
//..............................................................................
TEString::TEString(float V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%.6f", V);
  int c = strlen(FData);
  for( int i=c-1; i > 0; i-- )  { // remove trailing nulls
    if( FData[i] == '0' )  {
      if( FData[i-1] != '.' )
        FData[i] = '\0';
    }
    else
      break;
  }
  FLength = strlen(FData);
}
//..............................................................................
TEString::TEString(double V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%.6lf", V);
  int c = strlen(FData);
  for( int i=c-1; i > 0; i-- )  { // remove trailing nils
    if( FData[i] == '0' )  {
      if( FData[i-1] != '.' )
        FData[i] = '\0';
    }
    else
      break;
  }
  FLength = strlen(FData);
}
//..............................................................................
TEString::TEString(long double V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%.6Lf", V);
  int c = strlen(FData);
  for( int i=c-1; i > 0; i-- )  { // remove trailing nils
    if( FData[i] == '0' )  {
      if( FData[i-1] != '.' )
        FData[i] = '\0';
    }
    else
      break;
  }
  FLength = strlen(FData);
}
//..............................................................................
TEString::TEString(int16_t V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%d", V);
  FLength = strlen(FData);
}
//..............................................................................
TEString::TEString(uint16_t V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%u", V);
  FLength = strlen(FData);
}
//..............................................................................
#if defined(__BORLANDC__) || defined(__GNUC__)
TEString::TEString(long V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%ld", V);
  FLength = strlen(FData);
}
TEString::TEString(unsigned long V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%lu", V);
  FLength = strlen(FData);
}
#endif
//..............................................................................
TEString::TEString(int32_t V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%d", V);
  FLength = strlen(FData);
}
//..............................................................................
TEString::TEString(uint32_t V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%u", V);
  FLength = strlen(FData);
}
//..............................................................................
TEString::TEString(int64_t V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%Ld", V);
  FLength = strlen(FData);
}
//..............................................................................
TEString::TEString(uint64_t V, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(33, defcapacity);
  Allocate(NULL);
  sprintf(FData, "%Lu", V);
  FLength = strlen(FData);
}
//..............................................................................
TEString::TEString(bool C, int defcapacity)  {
  FIncrement = 5;
  FCapacity = olx_max(6, defcapacity);
  Allocate(NULL);
  if( C )  {
    strcpy(FData, TrueString.FData);
    FLength = 4;
  }
  else  {
    strcpy(FData, FalseString.FData);
    FLength = 5;
  }
}
//..............................................................................
//..............................................................................
TEString::~TEString()  {
  free(FData);
}
//..............................................................................
int TEString::CompareStr(const TEString &Str, bool IC) const  {
  int minl = olx_min(FLength, Str.Length());
  int diff, diff1;
  for( int i = 0; i < minl; i++ )  {
    diff = FData[i] - Str[i];
    if( FData[i] >= '0' && FData[i] <= '9' )  {
      if( Str[i] >= '0' && Str[i] <= '9' )  {
        TEString S, S1;
        int j = i, k = i;
        while( (j < FLength) && (FData[j] >= '0' && FData[j] <= '9') )  {
          S += FData[j];  j++;
        }
        while( (k < Str.Length()) && (Str[k] >= '0' && Str[k] <= '9') )  {
          S1 += Str[k];  k++;
        }
        diff1 = S.Int() - S1.Int();
        if( diff1 != 0 )  return diff1;
        // if the number of digits different - diff1 != 0, so now k=j
        i = k-1;
      }
    }
    else  {
      if( IC && ( ((Str[i] >= 'a' && Str[i] <= 'z') ||
                   (Str[i] >= 'A' && Str[i] <= 'Z')) &&
                  ((FData[i] >= 'a' && FData[i] <= 'z') ||
                   (FData[i] >= 'A' && FData[i] <= 'Z'))  ) )  {
        diff1 = abs(diff);
        if( diff1 != AaDiff && diff1 != 0 )  return diff;
      }
      else  {
        if( diff  != 0 )
          return diff;
      }
    }
  }
  return 0;
}
//..............................................................................
int TEString::Compare(const TEString &Str) const  {
  return strcmp(FData, Str.FData);
};
//..............................................................................
int TEString::Compare(const TEString &Str, bool IC) const  {
  return (!IC) ? strcmp(FData, Str.FData) : strcasecmp(FData, Str.FData);
}
//..............................................................................
int TEString::CompareCI(const TEString &Str) const  {
  return strcasecmp(FData, Str.FData);
};
//..............................................................................
double TEString::Double() const  {
  if( FLength >= 3 && FData[0] == '0' && FData[1] == 'x' )
    return (double)RadInt<int>();
  if( (FData[0] == '-' || FData[0]=='+') && (FLength > 3 && FData[1] == '0' && FData[2] == 'x') )
    return (double)RadInt<int>();

  return atof(FData);
}
//..............................................................................
float TEString::Float() const  {
  if( FLength >= 3 && FData[0] == '0' && FData[1] == 'x' )
    return (float)RadInt<int>();
  if( (FData[0] == '-' || FData[0]=='+') && (FLength > 3 && FData[1] == '0' && FData[2] == 'x') )
    return (float)RadInt<int>();

  return (float)atof(FData);
}
//..............................................................................
#ifdef __BORLANDC__
long double TEString::LDouble() const  {
  if( FLength >= 3 && FData[0] == '0' && FData[1] == 'x' )
    return (double)RadInt<int>();
  if( (FData[0] == '-' || FData[0]=='+') && (FLength > 3 && FData[1] == '0' && FData[2] == 'x') )
    return (double)RadInt<int>();

  return _strtold(FData, NULL);
}
#endif
//..............................................................................
int TEString::Int() const {
  return RadInt<int>();
}
//..............................................................................
long TEString::Long() const  {
  return RadInt<long>();
}
//..............................................................................
int64_t TEString::Int64() const  {
  return RadInt<int64_t>();
}
//..............................................................................
int TEString::HexInt()  const {
  return RadInt<int>(16);
}
//..............................................................................
bool TEString::IsNumber() const  {
  if( FLength == 0 )  return false;
  // hex notation ...
  if( FLength >= 3 && FData[0] == '0' && FData[1] == 'x' )  {
    for( int i=2; i < FLength; i++ )
      if( !( (FData[i] >= '0' && FData[i] <= '9') ||
             (FData[i] >= 'a' && FData[i] <= 'f') ||
             (FData[i] >= 'A' && FData[i] <= 'F')
        )  )  return false;
    return true;
  }
  if( (FData[0] == '-' || FData[0]=='+') && (FLength > 3 && FData[1] == '0' && FData[2] == 'x') )  {
    for( int i=3; i < FLength; i++ )
      if( !( (FData[i] >= '0' && FData[i] <= '9') ||
             (FData[i] >= 'a' && FData[i] <= 'f') ||
             (FData[i] >= 'A' && FData[i] <= 'F')
        )  )  return false;
    return true;
  }
  short digits = 0, chars = 0;
  for( int i=0; i < FLength; i++ )  {
    char c = FData[i];
    if( ( c >= '0' && c <= '9') || (c == '.') || (c == ',') ||
        (c == '-') || (c == '+') || (c == 'e') || (c == 'E'))  {
      if( ( c >= '0' && c <= '9')  )
        digits++;
      else
        chars++;
    }
    else
      return false;
  }
  if( chars <= 4 && digits >= 1 )
    return true;
  return false;
}
//..............................................................................
void TEString::SetCapacity(int i)  {
  if( i <= FCapacity )  return;
  FCapacity = i;
  Allocate(FData);
//  char *Bf = new char [i];
//  if( FLength )  memcpy(Bf, FData, FLength);
//  Bf[FLength] = '\0';
//  if( FData )  delete [] FData;
//  FData = Bf;
}
//..............................................................................
void TEString::SetLength(int s)  {
  if( s >= FLength )  {
    if( FCapacity < s + 2 )  SetCapacity(s + FIncrement);
    for( int i=FLength; i < s; i++ )  FData[i] = ' ';
    FLength = s;
    FData[s] = '\0';
    return;
  }
  FLength = s;
  FData[s] = '\0';
}
//..............................................................................
#ifdef _OLX_DEBUG
char& TEString::operator [] (int index) const {
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FLength);
  return FData[index];
}
#endif
//..............................................................................
char TEString::operator = (char C)  {
  FLength = 1;
  FCapacity = FLength + FIncrement;
  Allocate(FData);
  FData[0] = C;  FData[1] = '\0';
  return C;
}
//..............................................................................
TEString& TEString::operator += (char C)  {
  if( FCapacity < FLength + 2 )  SetCapacity(FLength*1.5 + FIncrement);
  FData[FLength] = C;
  FLength ++;
  FData[FLength] = '\0';
  return *this;
}
//..............................................................................
const char* TEString::operator = (const char* S)  {
  FLength = (S == NULL) ? 0 : strlen(S);
  FCapacity = FLength + FIncrement;

  Allocate(FData);
  if( FLength !=0 )
    strcpy(FData, S);
  else
    FData[0] = '\0';
  return S;
}
//..............................................................................
TEString& TEString::operator += (const char *S)  {
  int sl = strlen(S);
  if( FCapacity < FLength + sl + 2 )  SetCapacity(FLength*1.5 + FIncrement + sl);
  strcpy(&FData[FLength], S);
  FLength += sl;
  return *this;
}
//..............................................................................
const TEString& TEString::operator = (const TEString &S)  {
  SetCapacity(S.FLength + FIncrement );
  memcpy(FData, S.FData, S.FLength);
  FLength = S.FLength;
  FData[FLength] = '\0';
  return S;
}
//..............................................................................
TEString& TEString::operator += (const TEString& S)  {
  *this += S.c_str();
  return *this;
}
//..............................................................................
bool TEString::operator == (const TEString& S) const {
  return strcmp(FData, S.c_str()) == 0;
}
//..............................................................................
bool TEString::operator == (const char *S) const  {
  return strcmp(FData, S) == 0;
}
//..............................................................................
/*void TEString::operator += (int S)
{
  TEString Str(S);
  *this += Str;
}*/
//..............................................................................
TEString TEString::operator + (const TEString &S) const  {
  TEString C( *this, S.Length() + FLength + 1);
  C += S;
  return C;
}
//..............................................................................
TEString TEString::operator +  (const char *S) const  {
  TEString C( *this, (S==NULL) ? (FLength+1) : (strlen(S)+FLength+1) );
  if( S != NULL )
    C += S;
  return C;
}
//..............................................................................
int TEString::LastIndexOf(char C, int pos) const  {
  if( FLength == 0 )  return -1;
  if( pos == -1 )     pos = FLength-1;
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, pos, 0, FLength);
#endif
  for( int i=pos; i >= 0; i-- )
    if( FData[i] == C )
      return i;
  return -1;
}
//..............................................................................
int TEString::FirstIndexOf(char C, int pos) const  {
  if( FLength == 0 )  return -1;
  if( pos == -1 )     pos = 0;
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, pos, 0, FLength);
#endif
  for( int i=pos; i < FLength; i++ )
    if( FData[i] == C )  return i;
  return -1;
}
//..............................................................................
TEString TEString::SubString(int index, int count) const  {
  if( count == 0 || FLength == 0 )  return EmptyString;
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index + count - 1, 0, FLength);
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, FLength);
#endif
  TEString S;
  S.SetCapacity(count + 1);
  memcpy(S.FData, &FData[index], count);
  S.FData[count] = '\0';
  S.FLength = count;
  return S;
}
//..............................................................................
TEString TEString::SubStringFrom(int index, int indexfromend) const  {
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index-1, 0, FLength);
#endif
  return SubString(index, FLength-index-indexfromend);
}
//..............................................................................
TEString TEString::SubStringTo(int index, int indexfromstart) const  {
#ifdef _OLX_DEBUG
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index-1, 0, FLength);
#endif
  return SubString(indexfromstart, index-indexfromstart);
}
//..............................................................................
bool TEString::StartsFrom(const TEString &St) const  {
  int sl = St.Length();
  if( sl > Length() ) return false;
  for( int i=0; i < sl; i++ )
    if( FData[i] != St[i] )  return false;
  return true;
}
//..............................................................................
bool TEString::EndsWith(const TEString &St) const  {
  int sl = St.Length();
  if( sl > Length() ) return false;
  for( int i=1; i <= sl; i++ )
    if( FData[FLength-i] != St[sl-i] )  return false;
  return true;
}
//..............................................................................
int TEString::FirstIndexOf(const TEString &C, int From, bool IC) const  {
  if( From < 0 )  From = 0;
  if( (From+C.FLength) > FLength )  return -1;
  int index=-1, j=0;
  if( !IC )  {
    for( int i=From; i < FLength; i++ )  {
      if( (FLength - i) < C.FLength && index == -1 )  return -1;
      if( FData[i] == C.FData[j] )  {
        index = i;
        while( C.FData[j] == FData[i+j] )  {
          j++;
          if( j == C.FLength )  break;
        }
        if( j == C.FLength )
          return index;
        else  {
         index = -1;
         j = 0;
        }
      }
    }
    return -1;
  }
  else  {
    for( int i=From; i < FLength; i++ )  {
      if( (FLength - i) < C.FLength && index == -1 )  return -1;
      char ac = FData[i],
           sc = C.FData[j];
      if( ac >= 'a' && ac <= 'z' ) ac += AaDiff;
      if( sc >= 'a' && sc <= 'z' ) sc += AaDiff;
      /* have to keep in mind that case sensitivity matters only for characters*/
      if( ac == sc )  {
        index = i;
        ac = FData[i+j];
        sc = C.FData[j];
        if( ac >= 'a' && ac <= 'z' ) ac += AaDiff;
        if( sc >= 'a' && sc <= 'z' ) sc += AaDiff;
        while( ac == sc )  {
          j++;
          if( j == C.FLength )  break;
          ac = FData[i+j];
          sc = C.FData[j];
          if( ac >= 'a' && ac <= 'z' ) ac += AaDiff;
          if( sc >= 'a' && sc <= 'z' ) sc += AaDiff;
        }
        if( j == C.FLength )
          return index;
        else  {
          index = -1;
          j = 0;
        }
      }
    }
    return -1;
  }
}
//..............................................................................
int TEString::LastIndexOf(const TEString &C, int From, bool IC) const  {
  if( From < 0 )  From = FLength;
  if( (From+C.FLength) > FLength )  return -1;
  int index=-1, j=C.FLength-1;
  if( !IC )  {
    for( int i=From; i >= 0; i-- )  {
      if( (FLength - i) < C.FLength && index == -1 )  return -1;
      if( FData[i] == C.FData[j] )  {
        index = i;
        while( C.FData[j] == FData[i-j] )  {
          j--;
          if( j == -1 ) break;
        }
        if( j == -1 )  return index;
        else  {
          index = -1;
          j = C.FLength-1;
        }
      }
    }
    return -1;
  }
  else  {
    for( int i=From; i >= 0; i-- )    {
      if( (FLength - i) < C.FLength && index == -1 )  return -1;
      char ac = FData[i],
           sc = C.FData[j];
      if( ac >= 'a' && ac <= 'z' ) ac += AaDiff;
      if( sc >= 'a' && sc <= 'z' ) sc += AaDiff;
      if( ac == sc  )  {
        index = i;
        ac = FData[i-j];
        sc = C.FData[j];
        if( ac >= 'a' && ac <= 'z' ) ac += AaDiff;
        if( sc >= 'a' && sc <= 'z' ) sc += AaDiff;
        while( ac == sc )  {
          j--;
          if( j == -1 ) break;
          ac = FData[i-j];
          sc = C.FData[j];
          if( ac >= 'a' && ac <= 'z' ) ac += AaDiff;
          if( sc >= 'a' && sc <= 'z' ) sc += AaDiff;
        }
        if( j == -1 )  return index;
        else  {
          index = -1;
          j = C.FLength-1;
        }
      }
    }
    return -1;
  }
}
//..............................................................................
void TEString::Insert(char c, int pos)  {
  Insert(c, pos, 1);
}
//..............................................................................
void TEString::Insert(char c, int pos, int count)  {
#ifdef _OLX_DEBUG
  // zero is a valid index, and the FLength is valid index, so check pos-1 instead
  if( pos )
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, pos-1, 0, FLength);
#endif
  if( FCapacity < FLength + count + 2 )  SetCapacity(FLength + FIncrement + count);
  memmove(&FData[pos+count], &FData[pos], FLength-pos);
  memset( &FData[pos], c, count);
//  for( int i=pos; i < pos+count; i++ )  FData[i] = c;
  FLength += count;
  FData[FLength] = '\0';
}
//..............................................................................
void TEString::Insert(const TEString& S, int pos)  {
  if( S.IsEmpty() )  return;

#ifdef _OLX_DEBUG
  // zero is a valid index, and the FLength is valid index, so check pos-1 instead
  if( pos )
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, pos-1, 0, FLength);
#endif
  int count = S.Length(),
      cap = FLength + FIncrement + count;
  if( cap > FCapacity )  {
    FCapacity = cap;
    Allocate(FData);
  }
  memmove(&FData[pos+count], &FData[pos], FLength-pos);
  memcpy(&FData[pos], S.c_str(), count);
  FLength += count;
  FData[FLength] = '\0';
}
//..............................................................................
void TEString::Delete( int index, int count )  {
#ifdef _OLX_DEBUG                                              
  TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index + count - 1, 0, FLength);
#endif
  memmove( &FData[index], &FData[index+count], FLength-index-count);
  FLength -= count;
  FData[FLength] = '\0';
}
//..............................................................................
TEString TEString::UpperCase() const  {
  TEString S = *this;
  char *Dt = S.Data();
  int c = Length();
  for( int i=0; i < c; i++ )
    if( Dt[i] >= 'a' && Dt[i] <= 'z' )
      Dt[i] = (char)(Dt[i] + AaDiff);
  return S;
}
//..............................................................................
TEString TEString::LowerCase() const{
  TEString S = *this;
  char *Dt = S.Data();
  int c = Length();
  for( int i=0; i < c; i++ )
    if( Dt[i] >= 'A' && Dt[i] <= 'Z' )
      Dt[i] = (char)(Dt[i] - AaDiff);
  return S;
}
//..............................................................................
int TEString::LeadingCharCount(char C) const  {
  int index=0;
  while( index < FLength && FData[index] == C )
    index ++;
  return index;
}
//..............................................................................
int TEString::CharCount(char C) const  {
  int count=0;
  for( int i=0; i < FLength; i++ )
    if( FData[i] == C )
      count++;
  return count;
}
////////////////////////////////////////////////////////////////////////////////
// ststic members
TEString TEString::RemoveChars(const TEString &S, char C)  {
  TEString s;
  s.SetCapacity( S.FLength );
  for( int i=0; i < S.FLength; i++ )
    if( S[i] != C )
      s += S[i];
  return s;
}
//..............................................................................
TEString TEString::Trim(char C) const  {
  int count = FLength;
  int charcount = 0, i, startfrom = 0;
  TEString Res;
  for( i=count-1; i >=0; i-- )  {  // to remove spaces from the end
    if( FData[i] == C )
      charcount ++;
    else
      break;
  }
  while( startfrom < count && FData[startfrom] == C )
    startfrom++;

  Res.SetLength(olx_max(count - charcount - startfrom, 0));
  for(i=startfrom; i < count - charcount; i++ )
    Res[i-startfrom] = FData[i];  
  return Res;
}
//..............................................................................
TEString TEString::RemoveMultiSpaces(const TEString &S, bool RemoveTrailing)  {
  TEString s;
  bool spaceadded=true; // remove spaces from the beginning
  int count = S.Length();
  s.SetCapacity( count );
  int spacescount = 0;
  if( RemoveTrailing )  {
    for( int i=count-1; i >=0; i-- ) // to remove spaces from the end
      if( S[i] == ' ' )      spacescount ++;
      else                       break;
  }
  for( int i=0; i < count - spacescount; i++ )  {
    if (S[i] != ' ')  {
      spaceadded = false;
      s += S[i];
    }
    else  {
      if( !spaceadded )  {
        s += ' ';
        spaceadded = true;
      }
    }
  }
  return s;
}
//..............................................................................
TEString& TEString::FormatString(TEString &S, int count, bool Right, const TEString &Sep)  {
  int l = S.FLength;
  if( l > count )  {
    S += Sep;
    return S;
  }
  S.SetCapacity( S.FLength + count );
  if( Right )
    for( int i=0; i < count-l; i++ )
      S += Sep;
  else  {
    int cnt = count - l;
    for( int i=0; i < cnt; i++ )
      S.Insert(Sep, 0);
  }
  return S;
}
//..............................................................................
TEString TEString::CommonString(const TEString &A, const TEString &B)  {
  TEString C;
  int count = olx_min(A.Length(), B.Length());
  C.SetCapacity( count );
  for( int i=0; i < count ; i++ )  {
    if( A[i] == B[i] )
      C += A[i];
    else
      break;
  }
  return C;
}
//..............................................................................
TEString TEString::FormatFloat(int NumberOfDigits, double v, bool Exponent)  {
  TEString F;
  if( NumberOfDigits < 0 )  {
    NumberOfDigits = -NumberOfDigits;
    if( v >= 0 )
      F = ' ';
  }

  F << "%." << NumberOfDigits << ( (Exponent) ? 'e' : 'f');

  char bf[100];
  sprintf(bf, F.c_str(), v);
  F = bf;
  return F;
}
//..............................................................................
int TEString::Replace(const TEString &What, const TEString &With, bool IC)  {
  int rep=0;
  int ind = FirstIndexOf(What, 0, IC);
  while( ind >=0 )  {
    Delete(ind, What.FLength);
    Insert(With, ind);
    ind = FirstIndexOf(What, ind+With.FLength, IC);
    rep ++;
  }
  return rep;
}
//..............................................................................
int TEString::SearchArrayIC(char *Array, int ALength, char *SearchStr)  {
  int Length = strlen(SearchStr);
  if( Length > ALength )  return -1;
  int
    index=-1,
    j=0;
  for( int i=0; i < ALength; i++ )  {
    if( (ALength - i) < Length && index == -1 )  return -1;
    char ac = Array[i],
         sc = SearchStr[j];
    if( ac >= 'a' && ac <= 'z' ) ac += AaDiff;
    if( sc >= 'a' && sc <= 'z' ) sc += AaDiff;
    if( ac == sc )  {
      index = i;
      ac = Array[i];
      sc = SearchStr[j];
      if( ac >= 'a' && ac <= 'z' ) ac += AaDiff;
      if( sc >= 'a' && sc <= 'z' ) sc += AaDiff;
      while( ac == sc )  {
        j++;
        if( j == Length )  break;
        ac = Array[i+j];
        sc = SearchStr[j];
        if( ac >= 'a' && ac <= 'z' ) ac += AaDiff;
        if( sc >= 'a' && sc <= 'z' ) sc += AaDiff;
      }
      if( j == Length )
        return index;
      else  {
        index = -1;
        j = 0;
      }
    }
  }
  return -1;
}
//..............................................................................
int TEString::SearchArray(char *Array, int ALength, char *SearchStr)  {
  int Length = strlen(SearchStr);
  if( Length > ALength )  return -1;
  int index=-1, j=0;
  for( int i=0; i < ALength; i++ )  {
    if( (ALength - i) < Length && index == -1 )  return -1;
    if( Array[i] == SearchStr[j] )  {
      index = i;
      while( SearchStr[j] == Array[i+j] )  {
        j++;
        if( j == Length )  break;
      }
      if( j == Length )
        return index;
      else  {
        index = -1;
        j = 0;
      }
    }
  }
  return -1;
}
//..............................................................................
TEString TEString::CharStr(char C, int Number)  {
  TEString s(EmptyString, Number + 1 );
  for( int i=0; i < Number; i++ )
    s += C;
  return s;
}
//..............................................................................

////////////////////////////////////////////////////////////////////////////////
////////////////////EXTERNAL LIBRRAY FUNCTIONS//////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void Strcat(const TStrObjList& Params, TMacroError& E)
{  E.SetRetVal( Params.String(0)+Params.String(1) );  }

void Compare(const TStrObjList& Params, TMacroError& E)
{  E.SetRetVal( (bool)(Params.String(0) == Params.String(1)) );  }

void Replace(const TStrObjList& Params, TMacroError& E)
{
  TEString res = Params.String(0);
  res.Replace( Params.String(1), Params.String(2) );
  E.SetRetVal( res );
}

void UpperCase(const TStrObjList& Params, TMacroError& E)
{  E.SetRetVal( Params.String(0).UpperCase() );  }

void LowerCase(const TStrObjList& Params, TMacroError& E)
{  E.SetRetVal( Params.String(0).LowerCase() );  }

void Length(const TStrObjList& Params, TMacroError& E)
{  E.SetRetVal( Params.String(0).Length() );  }

TLibrary*  TEString::ExportLibrary(const TEString& name=EmptyString)  {
  TLibrary* lib = new TLibrary( name.IsEmpty() ? TEString("string") : name);
  lib->RegisterStaticFunction( new TStaticFunction( ::Strcat, "Strcat", fpTwo,
"Concatenates two strings ito one") );
  lib->RegisterStaticFunction( new TStaticFunction( ::Compare, "Cmp", fpTwo,
"Compares two strings and returns true or false") );
  lib->RegisterStaticFunction( new TStaticFunction( ::Replace, "Replace", fpThree,
"Replaces string passed as second parameter with the string passed as third parameter\
 in the string passed as first parameter") );
  lib->RegisterStaticFunction( new TStaticFunction( ::UpperCase, "UpperCase", fpOne,
"Returns uppercase representtaion of provided string") );
  lib->RegisterStaticFunction( new TStaticFunction( ::LowerCase, "LowerCase", fpOne,
"Returns lowercase representtaion of provided string") );
  lib->RegisterStaticFunction( new TStaticFunction( ::Length, "Length", fpOne,
"Returns length of the string") );
  return lib;
}
