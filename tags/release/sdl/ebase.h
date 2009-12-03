//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef ebaseH
#define ebaseH
//---------------------------------------------------------------------------

#include <typeinfo>
#include <string.h>
#include <stdlib.h>

  #define EsdlClassNameT(class)  typeid(class).name()
  #define EsdlObjectNameT(object)  typeid(object).name()
  #define EsdlClassName(class)  olxstr(typeid(class).name())
  #define EsdlObjectName(object)  olxstr(typeid(object).name())

  #define EsdlInstanceOf( class, className )  (typeid(class) == typeid(className))


// defines a primitive type property
#define DefPropP(Type, Name) \
public:\
  Type Get##Name() const {  return Name;  }\
  void Set##Name(Type MaCV) {  Name = MaCV;  }
// defines a boolean type property as Is/Set
#define DefPropBIsSet(Name) \
public:\
  bool Is##Name() const {  return Name;  }\
  void Set##Name(bool MaCV) {  Name = MaCV;  }
// defines a boolean type property as Is/Set
#define DefPropBHasSet(Name) \
public:\
  bool Has##Name() const {  return Name;  }\
  void Set##Name(bool MaCV) {  Name = MaCV;  }
// defines a boolean type property as a bit mask as Is/Set
#define DefPropBFIsSet(Name, VarName, BitMask) \
public:\
  bool Is##Name() const {  return (VarName & BitMask) != 0;  }\
  void Set##Name(bool v)   { \
      if( v )  VarName |= BitMask; \
      else     VarName &= ~BitMask;  }
// defines a boolean type property as a bit mask as Has/Set
#define DefPropBFHasSet(Name, VarName, BitMask) \
public:\
  bool Has##Name() const {  return (VarName & BitMask) != 0;  }\
  void Set##Name(bool v)   { \
      if( v )  VarName |= BitMask; \
      else     VarName &= ~BitMask;  }
// defines a complex (class) type property
#define DefPropC(Type, Name) \
public:\
  const Type& Get##Name()    const {  return Name;  }\
  void Set##Name(const Type& MaCV) {  Name = MaCV;  }

#define BeginEsdlNamespace()  namespace esdl {
#define EndEsdlNamespace()  };\
  using namespace esdl;
#define UseEsdlNamespace()  using namespace esdl;
#define GlobalEsdlFunction( fun )     esdl::fun
#define EsdlObject( obj )     esdl::obj

#include "defs.h"

#ifdef __WIN32__
  #if !defined(_WIN32_WINNT) && _MSC_VER < 1500
    #define _WIN32_WINNT 0x400
  #endif
  #include <windows.h>
#endif
// there is a mistery how it manages to disapper!!!!
#ifdef __BORLANDC__
  #if !defined(__MT__)
    #define __MT__
  #endif
#endif

BeginEsdlNamespace()

static const size_t InvalidIndex = (size_t)(~0);
static const size_t InvalidSize = (size_t)(~0);
// validates if unsigned number is valid... since the move to size_t etc...
template <typename int_t> bool olx_is_valid_index(const int_t& v)  {  return v != (int_t)~0;  }
template <typename int_t> bool olx_is_valid_size(const int_t& v)  {  return v != (int_t)~0;  }
// string base
template <class T> class TTIString {
public:
  static const unsigned short CharSize;
  struct Buffer  {
    T *Data;
    unsigned int RefCnt;
    size_t Length;
    Buffer(size_t len, const T *data=NULL, size_t tocopy=0)  {
      Data = ((len != 0) ? (T*)malloc(len*CharSize) : (T*)NULL);
      if( data != NULL )
        memcpy(Data, data, tocopy*CharSize);
      RefCnt = 1;
      Length = len;
    }
    // creates object from external array, must be created with new
    Buffer(T *data, size_t len)  {
      Data = data;
      RefCnt = 1;
      Length = len;
    }
    ~Buffer()  {  if( Data != NULL )  free(Data);  }
    void SetCapacity(size_t newlen)  {
      if( newlen > Length )  {
        Data = (T*)realloc(Data, newlen*CharSize);
        Length = newlen;
      }
    }
  };
protected:
  mutable Buffer *SData;  // do not have much choice with c_str ...
  void IncLength(size_t len)  {  _Length += len;  }
  void DecLength(size_t len)  {  _Length -= len;  }
  size_t GetCapacity()  const {  return SData->Length;  }
  size_t _Increment, _Length;
  mutable size_t _Start;
  TTIString() {}
  void checkBufferForModification(size_t newSize) const {
    if( SData == NULL )
      SData = new Buffer(newSize + _Increment);
    else if( SData->RefCnt > 1 )  {
      SData->RefCnt--;
      Buffer *newData = new Buffer(newSize + _Increment, &SData->Data[_Start], olx_min(_Length, newSize));
      SData = newData;
      _Start = 0;
    }
    else if( SData->RefCnt == 1 && _Start != 0 )  {
      if( _Length != 0 )
        memmove(SData->Data, &SData->Data[_Start], _Length*CharSize);
      _Start = 0;
    }
    if( SData->Length < newSize )
      SData->SetCapacity( (size_t)(newSize*1.5)+_Increment );
  }
public:
  TTIString(const TTIString& str) {
    SData = str.SData;
    if( SData != NULL )  SData->RefCnt++;
    _Length = str._Length;
    _Start = str._Start;
    _Increment = 8;
  }
  virtual ~TTIString() {
    if( SData != NULL && --SData->RefCnt == 0 )
      delete SData;
  }
  // might not have '\0' char, to be used with Length or RawLen (char count)
  T * raw_str() const { return ((SData == NULL) ? NULL : &SData->Data[_Start]);  }
  size_t RawLen() const { return _Length*CharSize;  }
  size_t Length() const { return _Length;  }
  // standard api requires terminating '\0'; the use of raw_str and Length() is preferable
  const T * u_str() const {
    if( SData == NULL ) return NULL;
    if( (SData->Length == (_Start+_Length)) || (SData->Data[_Start+_Length] != '\0') )  {
      checkBufferForModification(_Length + 1);
      SData->Data[_Start+_Length] = '\0';
    }
    return &SData->Data[_Start];
  }
  T Data(size_t i) const { return SData->Data[_Start + i];  }
  bool IsEmpty() const { return _Length == 0;  }
  T CharAt(size_t i) const { return SData->Data[_Start + i];  }
  T operator[] (size_t i) const { return SData->Data[_Start + i];  }
  /* reads content of the string to external buffer, which must be able to accommodate
   string length + 1 for the end of string char  */
  T *Read(T *v)  {
    if( SData == NULL ) return NULL;
    memcpy(v, &SData->Data[_Start], _Length*CharSize);
    v[_Length] = L'\0';
    return v;
  }
  // this does not help in GCC, 
  template <typename,typename> friend class TTSString;
#ifdef __GNUC__
  Buffer* GetBuffer() const {  return SData;  }  // it is mutable
  size_t Start() const { return _Start; }
#endif
};

template <typename T> const unsigned short TTIString<T>::CharSize = sizeof(T);

typedef TTIString<olxch> TIString;

// implementation of basic object, providing usefull information about a class
class IEObject  {
  // this function, if set, will be called from the destructor - useful for garbage collector...
  struct a_destruction_handler  {
    a_destruction_handler *next;
    a_destruction_handler(a_destruction_handler* _prev) : next(NULL) {
      if( _prev != NULL )
        _prev->next = this;
    }
    virtual ~a_destruction_handler() {}
    virtual void call(IEObject* obj) const = 0;
    virtual const void* get_identifier() const = 0;
  };
  struct static_destruction_handler : public a_destruction_handler {
    void (*destruction_handler)(IEObject* obj);
    static_destruction_handler(
      a_destruction_handler* prev, 
      void (*_destruction_handler)(IEObject* obj)) :
        a_destruction_handler(prev),
        destruction_handler(_destruction_handler) {}
    virtual void call(IEObject* obj) const {  (*destruction_handler)(obj);  }
    virtual const void* get_identifier() const {  return (const void*)destruction_handler;  }
  };
  template <class base>
  struct member_destruction_handler : public a_destruction_handler {
    void (base::*destruction_handler)(IEObject* obj);
    base& instance;
    member_destruction_handler(
      a_destruction_handler* prev, 
      base& base_instance,
      void (base::*_destruction_handler)(IEObject* obj)) :
        a_destruction_handler(prev),
        instance(base_instance),
        destruction_handler(_destruction_handler) {}
    virtual void call(IEObject* obj) const {  (instance.*destruction_handler)(obj);  }
    virtual const void* get_identifier() const {  return &instance;  }
  };
  
  a_destruction_handler *dsh_head, *dsh_tail;
  
  void _RemoveDestructionHandler(const void* identifier)  {
    a_destruction_handler *cr = dsh_head, *prev=NULL;
    while( cr != NULL )  {
      if( cr->get_identifier() == identifier )  {
        if( prev != NULL )  prev->next = cr->next;
        if( cr == dsh_tail )  dsh_tail = prev;
        if( dsh_head == cr )  dsh_head = NULL;
        delete cr;
        break;
      }
      prev = cr;
      cr = cr->next;
    }
  }
public:
  IEObject() : dsh_head(NULL), dsh_tail(NULL) {}
  virtual ~IEObject()  {
    while( dsh_head != NULL )  {
      dsh_head->call(this);
      a_destruction_handler *dsh = dsh_head;
      dsh_head = dsh_head->next;
      delete dsh;
  }
  }
  // throws an exception
  virtual TIString ToString() const;
  // throws an exception if not implemented
  virtual IEObject* Replicate() const;
  void AddDestructionHandler(void (*func)(IEObject*))  {
    if( dsh_head == NULL )
      dsh_head = dsh_tail = new static_destruction_handler(NULL, func);
    else
      dsh_tail = new static_destruction_handler(dsh_tail, func);
  }
  template <class base>
  void AddDestructionHandler(base& instance, void (base::*func)(IEObject*))  {
    if( dsh_head == NULL )
      dsh_head = dsh_tail = new member_destruction_handler<base>(NULL, instance, func);
    else
      dsh_tail = new member_destruction_handler<base>(dsh_tail, instance, func);
  }
  template <class T> void RemoveDestructionHandler(const T& indetifier)  {
    _RemoveDestructionHandler((const void*)indetifier);
  }
};

  extern const IEObject& NullObject;
  extern const char* NewLineSequence;
  extern const short NewLineSequenceLength;

// implements data for collection item
class ACollectionItem : public IEObject  {
  index_t CollectionItemTag;
public:
  ACollectionItem()  {  CollectionItemTag = -1;  }
  virtual ~ACollectionItem()  {  ;  }
  index_t GetTag() const {  return CollectionItemTag;  }
  void SetTag(index_t v) { CollectionItemTag = v;  }
  index_t IncTag()  {  return ++CollectionItemTag;  }
  index_t DecTag()  {  return --CollectionItemTag;  }
};

// an association template; association of any complexity can be build from this :)
// but for more flexibility still Association3 is provided
template <class Ac, class Bc> class AnAssociation2  {
  Ac a;
  Bc b;
public:
  AnAssociation2()  {  ;  }
  AnAssociation2( const Ac& a )  {  this->a = a;  }
  AnAssociation2( const Ac& a, const Bc& b )  {
    this->a = a;
    this->b = b;
  }
  AnAssociation2( const AnAssociation2& an )  {
    this->a = an.GetA();
    this->b = an.GetB();
  }
  virtual ~AnAssociation2()  {  }

  const AnAssociation2& operator = (const AnAssociation2& an)  {
    SetA( an.GetA() );
    SetB( an.GetB() );
    return an;
  }

  Ac& A()  {  return a;  }
  Bc& B()  {  return b;  }
  const Ac& GetA() const  {  return a;  }
  const Bc& GetB() const {  return b;  }
  void SetA( const Ac& a )  {  this->a = a;  }
  void SetB( const Bc& b )  {  this->b = b;  }
};
template <class Ac, class Bc, class Cc> class AnAssociation3  {
  Ac a;
  Bc b;
  Cc c;
public:
  AnAssociation3()  {  ;  }
  AnAssociation3( const Ac& a )  {  this->a = a;  }
  AnAssociation3( const Ac& a, const Bc& b )  {
    this->a = a;
    this->b = b;
  }
  AnAssociation3( const Ac& a, const Bc& b, const Cc& c )  {
    this->a = a;
    this->b = b;
    this->c = c;
  }
  AnAssociation3( const AnAssociation3& an )  {
    this->a = an.GetA();
    this->b = an.GetB();
    this->c = an.GetC();
  }
  virtual ~AnAssociation3()  {  }

  const AnAssociation3& operator = (const AnAssociation3& an)  {
    SetA( an.GetA() );
    SetB( an.GetB() );
    SetC( an.GetC() );
    return an;
  }

  Ac& A()  {  return a;  }
  Bc& B()  {  return b;  }
  Cc& C()  {  return c;  }
  const Ac& GetA() const {  return a;  }
  const Bc& GetB() const {  return b;  }
  const Cc& GetC() const {  return c;  }
  void SetA( const Ac& a )  {  this->a = a;  }
  void SetB( const Bc& b )  {  this->b = b;  }
  void SetC( const Cc& c )  {  this->c = c;  }
};
template <class Ac, class Bc, class Cc, class Dc> class AnAssociation4  {
  Ac a;
  Bc b;
  Cc c;
  Dc d;
public:
  AnAssociation4()  {  ;  }
  AnAssociation4( const Ac& a )  {  this->a = a;  }
  AnAssociation4( const Ac& a, const Bc& b )  {
    this->a = a;
    this->b = b;
  }
  AnAssociation4( const Ac& a, const Bc& b, const Cc& c )  {
    this->a = a;
    this->b = b;
    this->c = c;
  }
  AnAssociation4( const Ac& a, const Bc& b, const Cc& c, const Dc& d )  {
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
  }
  AnAssociation4( const AnAssociation4& an )  {
    this->a = an.GetA();
    this->b = an.GetB();
    this->c = an.GetC();
    this->d = an.GetD();
  }
  virtual ~AnAssociation4()  {  }

  const AnAssociation4& operator = (const AnAssociation4& an)  {
    SetA( an.GetA() );
    SetB( an.GetB() );
    SetC( an.GetC() );
    SetD( an.GetD() );
    return an;
  }

  Ac& A()  {  return a;  }
  Bc& B()  {  return b;  }
  Cc& C()  {  return c;  }
  Dc& D()  {  return d;  }
  const Ac& GetA() const  {  return a;  }
  const Bc& GetB() const  {  return b;  }
  const Cc& GetC() const  {  return c;  }
  const Dc& GetD() const  {  return d;  }
  void SetA( const Ac& a )  {  this->a = a;  }
  void SetB( const Bc& b )  {  this->b = b;  }
  void SetC( const Cc& c )  {  this->c = c;  }
  void SetD( const Dc& d )  {  this->d = d;  }
};
// an interface for a referencible object
class AReferencible : public IEObject  {
  short This_RefCount;
public:
  AReferencible()  {  This_RefCount = 0;  }
  virtual ~AReferencible();

  short GetRefCount() const {  return This_RefCount;  }
  short DecRef()  {  return --This_RefCount;  }
  short IncRef()  {  return ++This_RefCount;  }
};

// we need this class to throw exceptions from string with gcc ...
class TExceptionBase : public IEObject  {
protected:
  /* to prevent creation this class directly. All instances must be of the TBasicExceptionClass
  defined in exception.h */
  virtual void CreationProtection() = 0;  
public:
  static void ThrowFunctionFailed(const char* file, const char* function, int line, const char* msg);
  // returns recasted this, or throws exception if dynamic_cast fails
  const class TBasicException* GetException() const; 
};

#ifdef _WIN64
#  define olx_cmp_size_t(a,b) (a) < (b) ? -1 : ((a) > (b) ? 1 : 0)
#else
#  define olx_cmp_size_t(a,b) (a) - (b)
#endif

EndEsdlNamespace()

#include "istream.h"
#include "smart/ostring.h"
#endif


