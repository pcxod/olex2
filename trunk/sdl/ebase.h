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
  inline Type Get##Name() const {  return Name;  }\
  inline void Set##Name(Type MaCV) {  Name = MaCV;  }
// defines a boolean type property as Is/Set
#define DefPropBIsSet(Name) \
public:\
  inline bool Is##Name() const {  return Name;  }\
  inline void Set##Name(bool MaCV) {  Name = MaCV;  }
// defines a boolean type property as Is/Set
#define DefPropBHasSet(Name) \
public:\
  inline bool Has##Name() const {  return Name;  }\
  inline void Set##Name(bool MaCV) {  Name = MaCV;  }
// defines a boolean type property as a bit mask as Is/Set
#define DefPropBFIsSet(Name, VarName, BitMask) \
public:\
  inline bool Is##Name() const {  return (VarName & BitMask) != 0;  }\
  inline void Set##Name(bool v)   { \
      if( v )  VarName |= BitMask; \
      else     VarName &= ~BitMask;  }
// defines a boolean type property as a bit mask as Has/Set
#define DefPropBFHasSet(Name, VarName, BitMask) \
public:\
  inline bool Has##Name() const {  return (VarName & BitMask) != 0;  }\
  inline void Set##Name(bool v)   { \
      if( v )  VarName |= BitMask; \
      else     VarName &= ~BitMask;  }
// defines a complex (class) type property
#define DefPropC(Type, Name) \
public:\
  inline const Type& Get##Name()    const {  return Name;  }\
  inline void Set##Name(const Type& MaCV) {  Name = MaCV;  }

#define BeginEsdlNamespace()  namespace esdl {
#define EndEsdlNamespace()  };\
  using namespace esdl;
#define UseEsdlNamespace()  using namespace esdl;
#define GlobalEsdlFunction( fun )     esdl::fun
#define EsdlObject( obj )     esdl::obj

#include "defs.h"

// there is a mistery how it manages to disapper!!!!
#ifdef __BORLANDC__
  #if !defined(__MT__)
    #define __MT__
  #endif
#endif

BeginEsdlNamespace()
// immutable string
template <class T> class TTIString {
public:
  static const short CharSize;
  struct Buffer  {
    T *Data;
    unsigned RefCnt;
    size_t Length;
    Buffer(size_t len, const T *data=NULL, size_t tocopy=0)  {
      Data = ((len != 0) ? (T*)malloc(len*CharSize) : (T*)NULL);
      if( data != NULL )
        memcpy(Data, data, tocopy*CharSize);
      RefCnt = 1;
      Length = len;
    }
    ~Buffer()  {  if( Data != NULL )  free(Data);  }
    inline void SetCapacity(size_t newlen)  {
      if( newlen > Length )  {
        Data = (T*)realloc(Data, newlen*CharSize);
        Length = newlen;
      }
    }
  };
protected:
  mutable Buffer *SData;  // do not have much choice with c_str ...
  inline void IncLength(size_t len)  {  _Length += len;  }
  inline void DecLength(size_t len)  {  _Length -= len;  }
  inline size_t GetCapacity()  const {  return SData->Length;  }
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
  inline T * raw_str()                   const { return ((SData == NULL) ? NULL : &SData->Data[_Start]);  }
  inline int RawLen()                    const { return _Length*CharSize;  }
  inline int Length()                    const { return _Length;  }
  // standard api requires terminating '\0'; the use of raw_str and Length() is preferable
  inline const T * u_str()               const {
    if( SData == NULL ) return NULL;
    if( (SData->Length == (_Start+_Length)) || (SData->Data[_Start+_Length] != '\0') )  {
      checkBufferForModification(_Length + 1);
      SData->Data[_Start+_Length] = '\0';
    }
    return &SData->Data[_Start];
  }
  inline T Data(size_t i)                const { return SData->Data[_Start + i];  }
  inline bool IsEmpty()                  const { return _Length == 0;  }
  inline T CharAt(size_t i)              const { return SData->Data[_Start + i];  }
  inline T operator[] (size_t i)         const { return SData->Data[_Start + i];  }
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
  inline Buffer* GetBuffer() const {  return SData;  }  // it is mutable
  inline size_t Start() const { return _Start; }
#endif
};

  template <typename T> const short TTIString<T>::CharSize = sizeof(T);

typedef TTIString<olxch> TIString;
// implementation of basic object, providing usefull information about a class
class IEObject  {
  // this function, if set, will be called from the destructor - useful for garbage collector...
  void (*AtDestruct_Function)(IEObject* obj);
public:
  IEObject()  {
    AtDestruct_Function = NULL;
  }
  virtual ~IEObject()  {
    if( AtDestruct_Function != NULL )
      AtDestruct_Function(this);
  }
  // throws an exception
  virtual TIString ToString() const;
  // throws an exception if not implemented
  virtual IEObject* Replicate() const;
  inline void SetAtDestruct( void (*fun)(IEObject*) )  {
    AtDestruct_Function = fun;
  }
};

  extern const IEObject& NullObject;
  extern const char* NewLineSequence;
  extern const short NewLineSequenceLength;


// implements data for collection item
class ACollectionItem : public IEObject  {
  int CollectionItemTag;
public:
  ACollectionItem()  {  CollectionItemTag = -1;  }
  virtual ~ACollectionItem()  {  ;  }
  inline  int GetTag() const {  return CollectionItemTag;  }
  inline void SetTag(int v) { CollectionItemTag = v;  }
  inline int IncTag()  {  return ++CollectionItemTag;  }
  inline int DecTag()  {  return --CollectionItemTag;  }
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

  inline Ac& A()  {  return a;  }
  inline Bc& B()  {  return b;  }
  inline const Ac& GetA() const  {  return a;  }
  inline const Bc& GetB() const {  return b;  }
  inline void SetA( const Ac& a )  {  this->a = a;  }
  inline void SetB( const Bc& b )  {  this->b = b;  }
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

  inline Ac& A()  {  return a;  }
  inline Bc& B()  {  return b;  }
  inline Cc& C()  {  return c;  }
  inline const Ac& GetA() const {  return a;  }
  inline const Bc& GetB() const {  return b;  }
  inline const Cc& GetC() const {  return c;  }
  inline void SetA( const Ac& a )  {  this->a = a;  }
  inline void SetB( const Bc& b )  {  this->b = b;  }
  inline void SetC( const Cc& c )  {  this->c = c;  }
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

  inline Ac& A()  {  return a;  }
  inline Bc& B()  {  return b;  }
  inline Cc& C()  {  return c;  }
  inline Dc& D()  {  return d;  }
  inline const Ac& GetA() const  {  return a;  }
  inline const Bc& GetB() const  {  return b;  }
  inline const Cc& GetC() const  {  return c;  }
  inline const Dc& GetD() const  {  return d;  }
  inline void SetA( const Ac& a )  {  this->a = a;  }
  inline void SetB( const Bc& b )  {  this->b = b;  }
  inline void SetC( const Cc& c )  {  this->c = c;  }
  inline void SetD( const Dc& d )  {  this->d = d;  }
};
// class factories

// a very simple factory
template <class ObjectClass>
  class ISObjectFactory  {
  public:
    virtual ~ISObjectFactory()  {  ;  }
    virtual ObjectClass* NewInstance() = 0;
  };
// implementation of the simple object factory
template <class ObjectBase, class ObjectImplementation>
  class TSFactory: public ISObjectFactory<ObjectBase>
  {
  public:
    virtual ~TSFactory()  {  ;  }
    virtual ObjectBase *NewInstance()  {  return new ObjectImplementation();  }
  };
// an interface for a referencible object
class AReferencible : public IEObject  {
  short This_RefCount;
public:
  AReferencible()  {  This_RefCount = 0;  }
  virtual ~AReferencible();

  inline short GetRefCount() const {  return This_RefCount;  }
  inline short DecRef()            {  return --This_RefCount;  }
  inline short IncRef()            {  return ++This_RefCount;  }
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

EndEsdlNamespace()

#include "istream.h"
#include "smart/ostring.h"
#endif


