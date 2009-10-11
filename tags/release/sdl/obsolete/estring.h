//----------------------------------------------------------------------------//
// namespace TEObjects: TEString
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef estringH
#define estringH
//---------------------------------------------------------------------------
#include "ebase.h"
#include <stdlib.h>

#if defined(__GNUC__) && defined(Bool)
  #undef Bool
#endif

BeginEsdlNamespace()

template <typename> class TVector;
class TInvalidArgumentException;	

class TEString;

  extern const TEString& EmptyString;
  extern const TEString& NullString;
  extern const TEString& TrueString;
  extern const TEString& FalseString;

class TEString: public IEObject  {
  char *FData;
  int FLength;
  int FCapacity, FIncrement;
  inline void Allocate(char* data)  {
    FData = (char*)realloc((void*)data, FCapacity);
  }
public:
  TEString();
  // creates a copy of the string but with the capacity trimmed to the length
  TEString(const TEString &E, int defcapacity=-1);
  TEString(const char* S, int defcapacity=-1);
  TEString(bool C, int defcapacity=-1);
  /* if defcapacity is greater than 1 the string is initialised with defcapacity
    chars and length is set to the defcapacity, works same as CharStr
  */
  TEString(char C, int defcapacity=-1);
#if defined(__BORLANDC__) || defined(__GNUC__)  // have no idea how they implement long...
  TEString(long V, int defcapacity=-1);
  TEString(unsigned long V, int defcapacity=-1);
#endif
  TEString(int16_t V, int defcapacity=-1);
  TEString(int32_t V, int defcapacity=-1);
  TEString(int64_t V, int defcapacity=-1);
  TEString(uint16_t V, int defcapacity=-1);
  TEString(uint32_t V, int defcapacity=-1);
  TEString(uint64_t V, int defcapacity=-1);
  TEString(float V, int defcapacity=-1);
  TEString(double V, int defcapacity=-1);
  TEString(long double V, int defcapacity=-1);
  // causes too many problems, during other overrided operators
  //operator const char*() const { return FData; }

  template <typename T>
    TEString(const TVector<T>& V)  {
      FIncrement = 5;
      FLength = FCapacity = 0;
      FData = NULL;
      TEString res;
      for( int i=0; i < V.Count(); i++ )  {
        res << V[i];
        if( (i+1) < V.Count() )  res << ',' << ' ';
      }
      *this = res;
    }
  virtual ~TEString();

  inline char* Data()    const {  return FData; }
  inline char* c_str()   const {  return FData; }
  inline int Length()    const {  return FLength;  }
  inline bool IsEmpty()  const {  return FLength == 0;  }
  // changes the number of extra bytes allocated by the string each time the buffer
  // allocated by the string comes to the end. Use to optimise subsequent add, insert
  // operations when the final size of the string is unknown, if it is known
  // then use SetCapacity instead
  void SetIncrement(int i)     {  FIncrement = i; }
  // expands the buffer allocated by the string; use to optimise subsequent addition
  //operations when the final size of the string is known, otherwise use SetIncrement function
  void SetCapacity(int i);
  virtual olxstr ToString()  const  {  return olxstr(this->c_str());  }
  virtual IEObject* Replicate() const  {  return new TEString(*this);  }
  // allows to truncate a string; if s > Length, expands the string with spaces
  void SetLength(int s);
#ifdef bool_defined
  inline bool Bool() const  {  return this->operator == (TrueString);  }
#endif
  int Int()       const;
  int HexInt()    const;
  long Long()     const;
  int64_t Int64() const;
  double Double() const;
  float Float() const;
#if defined(__BORLANDC__)  // could not locate in similar functions in MSVC
  long double LDouble() const;
#endif
public:
  // transforms the string into integer according to the radix
  template <class T>
    T RadInt(unsigned short Rad=10)  const  {
        int sts = 0; // string start
        while( FData[sts] == ' ' && ++sts < FLength )
        if( sts >= FLength )
          throw TInvalidArgumentException(__OlxSourceInfo, "invalid integer format");
        T val=0;
        bool Negative = false;
        if( FData[sts] == '-' )  {  Negative = true;  sts++;  }
        else if( FData[sts] == '+' )  sts++;
        if( sts == FLength )
          throw TInvalidArgumentException(__OlxSourceInfo, "invalid ineteger format");
        // test for any particluar format specifier, here just '0x', for hexadecimal
        if( FLength > sts+1 && FData[sts] == '0' && FData[sts+1] == 'x' )  {
          Rad = 16;
          sts += 2;
        }
        if( Rad > 10 )  {
          for( int i=sts; i < FLength; i++ )  {
            T pv = -1;
            if( FData[i] <= '9' && FData[i] >= '0' )  // the order is important, chars are rearer
              pv = FData[i] - '0';
            else if( FData[i] <= 'Z' && FData[i] >= 'A' )
              pv = FData[i] - 'A' + 10;
            else if( FData[i] <= 'z' && FData[i] >= 'a' )
              pv = FData[i] - 'a' + 10;
            else
              break;
//              throw TInvalidArgumentException(__OlxSourceInfo, TEString("Incorrect integer '") << *this << "' for base " << Rad);
            val = val*Rad + pv;
          }
        }
        else  {
          for( int i=sts; i < FLength; i++ )  {
            if( FData[i] <= '9' && FData[i] >= '0' )
              val = val*Rad + (FData[i] - '0');
            else
              break;
//              throw TInvalidArgumentException(__OlxSourceInfo, TEString("Incorrect base 10 integer '") << *this << '\'');
          }
        }
        return Negative ? -val : val;
      }
  // returns true if the string possibly represent a number and false otherwise;
  // the function counts the number of digits and other "number-relevant" characters
  // such as '.', ',' and '+' and '-';   the number of charachters should be lequal
  //to 4 and at least one digit should present: -1.1e-5
  bool IsNumber() const;
  // a normal comparison: a2 < a10; IC - insensitive to case
  int CompareStr(const TEString &Str, bool IC) const ;
  // a call strcmp - case sensitive compare
  int Compare(const TEString &Str) const ;
  // a call strcmp - case sensitive/incensitive comparison
  int Compare(const TEString &Str, bool IC) const ;
  // a call strcmpi - case insensitive compare
  int CompareCI(const TEString &Str) const;
#ifdef _OLX_DEBUG
  char& operator [] (int index) const;
#else
  inline char& operator [] (int index) const {
    return FData[index];
  }
#endif

  inline bool operator >  (const TEString &C) const  {  return (Compare(C) > 0);  }
  inline bool operator <  (const TEString &C) const  {  return (Compare(C) < 0);  }

  TEString operator +  (const TEString& C) const;
  TEString operator +  (const char *C) const;
  // this causes a lot of problems with ints etc being casted to char ...
  //TEString operator +  (char C) const  {  return TEString(*this) += C;  }
  inline bool operator =  (bool C)       {  TEString A(C);  *this = A; return C; }
  inline TEString& operator += (bool C)  {  TEString A(C);  *this += A; return *this; }
  inline TEString& operator << (bool C)  {  return this->operator +=(C); }
  char operator =  (char C);
  TEString& operator += (char C);
  inline TEString& operator << (char C)  {  return this->operator +=(C);  }
  inline int16_t operator =  (int16_t C)     {  TEString A(C);  *this = A; return C; }
  inline TEString& operator += (int16_t C) {  TEString A(C);  *this += A; return *this; }
  inline TEString& operator << (int16_t C) {  return this->operator +=(C); }
  inline uint16_t operator =  (uint16_t C)      {  TEString A(C);  *this = A; return C; }
  inline TEString& operator += (uint16_t C)           {  TEString A(C);  *this += A; return *this; }
  inline TEString& operator << (uint16_t C)           {  return this->operator +=(C); }
#if defined(__BORLANDC__) || defined(__GNUC__)  // have no idea how they implement long...
  inline long operator =  (long C)        {  *this = TEString(C);  return C; }
  inline TEString& operator += (long C)   {  *this += TEString(C); return *this; }
  inline TEString& operator << (long C)   {  return this->operator +=(C); }
  inline unsigned long operator =  (unsigned long C)        {  *this = TEString(C);  return C; }
  inline TEString& operator += (unsigned long C)   {  *this += TEString(C); return *this; }
  inline TEString& operator << (unsigned long C)   {  return this->operator +=(C); }
#endif
  inline int32_t operator =  (int32_t C)         {  TEString A(C);  *this = A; return C; }
  inline TEString& operator += (int32_t C)   {  TEString A(C);  *this += A; return *this; }
  inline TEString& operator << (int32_t C)   {  return this->operator +=(C); }
  inline uint32_t operator =  (uint32_t C)          {  TEString A(C);  *this = A; return C; }
  inline TEString& operator += (uint32_t C)             {  TEString A(C);  *this += A; return *this; }
  inline TEString& operator << (uint32_t C)             {  return this->operator +=(C); }
  inline int64_t operator =  (int64_t C)       {  TEString A(C);  *this = A; return C; }
  inline TEString& operator += (int64_t C)  {  TEString A(C);  *this += A; return *this; }
  inline TEString& operator << (int64_t C)  {  return this->operator +=(C); }
  inline uint64_t operator =  (uint64_t C)  {  TEString A(C);  *this = A; return C; }
  inline TEString& operator += (uint64_t C)         {  TEString A(C);  *this += A; return *this; }
  inline TEString& operator << (uint64_t C)         {  return this->operator +=(C); }
  inline float operator =  (float C)     {  TEString A(C);  *this = A; return C; }
  inline TEString& operator += (float C) {  TEString A(C);  *this += A; return *this; }
  inline TEString& operator << (float C) {  return this->operator +=(C); }
  inline double operator =  (double C)   {  TEString A(C);  *this = A; return C; }
  inline TEString& operator += (double C){  TEString A(C);  *this += A; return *this; }
  inline TEString& operator << (double C){  return this->operator +=(C); }
  inline long double operator =  (long double C)   {  TEString A(C);  *this = A; return C; }
  inline TEString& operator += (long double C)     {  TEString A(C);  *this += A; return *this; }
  inline TEString& operator << (long double C)     {  return this->operator +=(C); }

  const TEString& operator =  (const TEString &S);
  TEString& operator += (const TEString &S);
  inline TEString& operator << (const TEString &S) {  return this->operator +=(S);  }
  const char* operator =  (const char *S);
  TEString& operator += (const char *S);
  inline TEString& operator << (const char *S) {  return this->operator +=(S);  }

// case sensitive comparison functions like in Compare{
  bool operator == (const TEString &S)  const;
  bool operator == (const char *S) const;
  inline bool operator != (const TEString& S) const{  return !(*this == S); }
  inline bool operator != (const char*S ) const{  return !(*this == S); }

  // inserts count number of symbols c at pos
  void Insert(char c, int pos, int count);
  // inserts one symbols c at pos
  void Insert(char c, int pos);
  // inserts a string at pos
  void Insert(const TEString &S, int pos);
  // deletes a count number of characters starting from index position
  void Delete( int index, int count );
  // returns last index of a character starting from pos; if pos is not specified
  // the search is started from the end of the string
  int LastIndexOf(char C, int pos=-1) const;
  // returns first index of a character starting from pos; if pos is not specified
  // the search is started from the beginning of the string
  int FirstIndexOf(char C, int pos=-1)const;
  // returns the position of a substring within current string
  int IndexOf(char C ) const {  return FirstIndexOf(C, 0); };

  // returns last index of a string starting from pos; if pos is not specified
  // the search is started from the end of the string
  int LastIndexOf(const TEString &What, int pos=-1, bool IC=false) const;
  // returns first index of a string starting from pos; if pos is not specified
  // the search is started from the beginning of the string
  int FirstIndexOf(const TEString &What, int pos=-1, bool IC=false)const;
  // returns the position of a substring within current string
  int IndexOf(const TEString &SubString) const {  return FirstIndexOf(SubString, 0); };

  // returns a substring of current string
  TEString SubString(int index, int count) const;
  // returns a substring of current string starting from index to the end - indexfromend
  TEString SubStringFrom(int index, int indexfromend=0) const;
  /* returns a substring of current string starting from the indexfromstart
   of current string to index */
  TEString SubStringTo(int index, int indexfromstart=0) const;
  // replaces some substring of the string withj other substring
  //function returns the number of the substitutions
  int Replace(const TEString &What, const TEString &With, bool IC=false);
  // returns an uppercase representtaion of current string
  TEString UpperCase() const;
  // returns an lowercase representtaion of current string
  TEString LowerCase() const;
  bool StartsFrom(const TEString &St) const;
  bool EndsWith(const TEString &St)  const;
  // returns number of leading chars C in the string
  int LeadingCharCount(char C) const;
  // returns number of chars C in the string
  int CharCount(char C) const;
  /* returns a new string with spaces removed in the beginning and at the end*/
  TEString Trim(char C = ' ') const;
// static members

  // returns a string of number of chars
  static TEString CharStr(char Char, int Number);
  // returns a copy of S with all C omitted
  static TEString RemoveChars(const TEString &S, char C);
  //returns a copy of S with multiple spaces replaced by single space
  // the spaces are removed from the beginning of the string and optionally from
  // the end (set RemoveTrailing to false to leave trailing spaces)
  static TEString RemoveMultiSpaces(const TEString &S, bool RemoveTrailing=true);
  // adds a number of Sep (the size of which is assumed equal to 1!) to increase
  //the size of the string S to 'count'. Use Right to specify the alignment
  // note that the string passed as paraemeter is being modified, nothing is returned!
  static TEString& FormatString(TEString &S, int count, bool Right, const TEString &Sep);
  // returns a string common to A and B; thus if A = "abcqwert" and B = "abyuyw",
  // the function returns "ab"
  static TEString CommonString(const TEString &A, const TEString &B);
  // formats float point value so that only NumberOfDigits significant digits is shown
  static TEString FormatFloat(int NumberOfDigits, double v, bool Exponent=false);
  // hypernates a string and puts the result into 'SL'
  // returns true if the string was  forced to hypernate
  static int SearchArrayIC(char *Array, int ALength, char *SearchStr);
  static int SearchArray(char *Array, int ALength, char *SearchStr);

  static class TLibrary*  ExportLibrary(const TEString &name);
};

  template <bool CaseInsensetive>  class TEStringComparator  {
  public:
    template <class comparable>
    static inline int Compare(const TEString& A, const TEString& B )  {
      if( CaseInsensetive )  return A.CompareCI( B );
      else                   return A.Compare( B );
    }
};

EndEsdlNamespace()
#endif
