//----------------------------------------------------------------------------//
// namespace TEObjects: lists
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef sstrlistH
#define sstrlistH
//---------------------------------------------------------------------------
#include "ebase.h"
#include "string.h"
#include "estrlist.h"

BeginEsdlNamespace()

class TSStringList: public TEStringList
{
  bool FCI;
protected:
  // insert functions break the functionality of the object, so they are here
  void InsertObject(int index, const TEString &S, void *Object);
  void Insert(int index, const TEString &S);
  void Swap( int i, int j);
  const void * ObjectByNameCI(const TEString &Name) const;
  int IndexOfStringCI(const TEString &C) const;

  int FindInsertIndex(const TEString &Str, int from=-1, int to=-1) const;
  int SortedIndexOf(const TEString &Str, int from=-1, int to=-1) const;
  virtual int FindIndexOf(const TEString &Str, bool IC) const;
public:
  TSStringList(bool CaseInsensitive);
  virtual ~TSStringList();
  // population functions - do automatic sorting
  virtual void Assign(const TEStringList &S);
  virtual void AddList(const TEStringList &S);
  virtual void AddString(const TEString &S);
  virtual void AddString(const char *S);
  virtual void AddObject(const TEString &S, void *Object);
  int IndexOfString(const TEString &C) const   {  return FindIndexOf(C, FCI);  }
  TIntList IndexesOfString(const TEString &C) const;
  const void * ObjectByName(const TEString &Name) const
  {  return FCI?TEStringList::ObjectByNameCI(Name) : TEStringList::ObjectByName(Name);  }

  BeginIEObjectImplementation()
};

EndEsdlNamespace()
#endif
