#ifndef cmodelH
#define cmodelH
#include "estrlist.h"
#include "estlist.h"
//#include "estlist.h"

// forward references
class CTemplate;
class CType;
class CFunction;
class CStruct;
class CNamespace;
class CModel;

/* primitive type, like char, int etc */
class CPType  {
  TEString TypeName;
public:
  CPType(const TEString& name) : TypeName(name)  {}
  virtual ~CPType()  {}
  inline const TEString& GetTypeName() const {  return TypeName;  }
  virtual bool IsPrimitive() const {  return true;  }
  inline bool IsComplex()    const {  return !IsPrimitive();  }
  virtual bool IsTemplate()  const {  return false;  }
};

/* template type */
class CTType : public CPType  {
  const CPType* RealType; // only defined for templates
public:
  CTType(const TEString& name) : CPType(name)  { RealType = NULL; }
  inline void UseType(const CPType* type)  {  RealType = type;  }
  inline const CPType* GetRealType() const {  return RealType;  }
  virtual bool IsTemplate()  const {  return true;  }
};

class CMember {
  const CPType& Type;
  TEString Name, DefValue;
public:
  CMember(const CPType& atype, const TEString &name) : Type(atype), Name(name) {}
  CMember(const CPType& atype, const TEString &name, const TEString &defVal) :
    Type(atype), Name(name), DefValue(defVal) {}
  inline const CPType& GetType()     const {  return Type;  }
  inline const TEString& GetName()   const {  return Name;  }
  inline const TEString& GetDefVal() const {  return Name;  }
};

class CFunction  {
  TEString Name;
protected:
  TPtrList<CMember> Args;
  const CPType* RetType;
public:
  CFunction(const CPType* rettype, const TEString& name) : Name(name)  {  RetType = rettype;  }
  virtual ~CFunction() {
  }
  inline void AddArg(const CPType& atype, const TEString &name)  { Args.Add( new CMember(atype, name) ); }
  inline void AddArg(const CPType& atype, const TEString &name, const TEString &defVal)  {
    Args.Add( new CMember(atype, name, defVal) );
  }
  inline int ArgCount(   )            const {  return Args.Count();  }
  inline const CMember& GetArg(int i) const {  return *Args[i];  }
  inline const CPType* GetRetType()   const {  return RetType;  }
  inline const TEString& GetName()    const {  return Name;  }
};


//class ACType : public CPType  {
//public:
//  ACType(const TEString& typeName) : CPType(typeName) {}
//  virtual ACType* NewInstance(const TEString& name, CStruct* parent
//};

class CStruct: public CPType  {
  TSStrPObjList<CFunction*, false> Functions;
  TSStrPObjList<CStruct*, false> Types; // all object declarations: namespaces, structs, classes
  TSStrPObjList<CMember*, false> Members; // primitive and complex
  TEString Name;
  CStruct* Parent;
public:
  CStruct(const TEString &name, CStruct* parent = NULL, const TEString &typeName="struct") :
    CPType(typeName), Name(name)  {  Parent = parent;  }
  virtual ~CStruct() {
    for( int i=0; i < Functions.Count(); i++ )  delete Functions.Object(i);
    for( int i=0; i < Types.Count(); i++ )      delete Types.Object(i);
    for( int i=0; i < Members.Count(); i++ )    delete Members.Object(i);
  }
  void AddFunc(CFunction* func)  {  Functions.Add(func->GetName(), func);  }
  void AddMember(CMember* mbr)   {  Members.Add(mbr->GetName(), mbr);  }
  void AddType(CStruct* typ)     {  Types.Add(typ->GetName(), typ); }

  inline int TypeCount()                 const {  return Types.Count();  }
  inline const CStruct& GetType(int i)   const {  return *Types.GetObject(i);  }
  inline const CStruct* FindType(const TEString &name) const {
    return Types[name];
  }

  inline int FuncCount()                 const {  return Functions.Count();  }
  inline const CFunction& GetFunc(int i) const {  return *Functions.GetObject(i); }
  inline const CFunction* FindFunc(const TEString &name) const {
    return Functions[name];
  }

  inline int MemberCount()               const {  return Members.Count();  }
  inline const CMember& GetMember(int i) const {  return *Members.GetObject(i);  }
  inline const CMember* FindMember(const TEString &name) const {
    return Members[name];
  }

  inline const TEString& GetName()       const {  return Name;  }
  TEString GetQualifiedName() const {
    TEString qn(Name, 128);
    CStruct* pr = Parent;
    while( pr != NULL )  {
      qn.Insert( Parent->GetName() + "::", 0);
      pr = pr->Parent;
    }
    return qn;
  }
};

class CNamespace: public CStruct  {
public:
  CNamespace(const TEString &name, CNamespace* parent=NULL) : CStruct(name, parent, "namespace") {}
};

/* a complex type. It could be a struct or a class */
class CType : public CStruct {
  TSStrPObjList<CPType*, false> ProtectedTypes;
  TSStrPObjList<CPType*, false> PrivateTypes;
  TSStrPObjList<CPType*, false> ProtectedMembers;
  TSStrPObjList<CPType*, false> PrivateMembers;
  TSStrPObjList<CFunction*, false> ProtectedFunctions;
  TSStrPObjList<CFunction*, false> PrivateFunctions;
  TStrPObjList<CType*> Ancestors;  // cannot inherit from a primitive type
  CTemplate* Template;
public:
  CType(const TEString& name, CStruct* parent, CTemplate* templ) : CStruct(name, parent, "class")  {
    Template = templ;
  }

  virtual bool IsPrimitive()  const {  return false;  }
  virtual bool IsTemplate()  const  {  return Template != NULL;  }
};

/* template */
class CTemplate {
public:
  class CTArg {
    CTType* Type;
    TEString Name, DefValue;
  public:
    CTArg(CTType* atype) : Type(atype) {}
    CTArg(CTType* atype, const TEString &name) : Type(atype), Name(name) {}
    CTArg(CTType* atype, const TEString &name, const TEString &defVal) :
      Type(atype), Name(name), DefValue(defVal) {}
    virtual ~CTArg()  {  delete Type;  }
    inline const CTType& GetType()   const {  return *Type;  }
    inline const TEString& GetName() const {  return Name;  }
    inline const TEString& GetDefVal() const {  return Name;  }
  };
protected:
  TPtrList<CTArg> Args;
  CTemplate* Parent; // used to resolve missing types
public:
  CTemplate()  {
    Parent = NULL;
  }
  virtual ~CTemplate()  {
    for( int i=0; i < Args.Count(); i++ )  delete Args[i];
  }
  void AddArg(const TEString& typeName)  {
    Args.Add( new CTArg( new CTType(typeName) ) );
  }
  void AddArg(const TEString& typeName, const TEString& argName)  {
    Args.Add( new CTArg( new CTType(typeName), argName ) );
  }
  void AddArg(const TEString& typeName, const TEString& argName, const TEString& argVal)  {
    Args.Add( new CTArg( new CTType(typeName), argName, argVal ) );
  }
  void Parse(const TEString& line);
};

class CModel  {
  CNamespace* nsGlobal;
//  TSStrPObjList<CMacro*, false> Typedefs;
//  TSStrPObjList<CMacro*, false> Typedefs;
  TEString ProjectName;
public:
  CModel(const TEString &projectName) : ProjectName(projectName)  {
    nsGlobal = new CNamespace(EmptyString);
  }
  ~CModel()  {  delete nsGlobal;  }

  inline CNamespace& Global()  {  return *nsGlobal;  }
  /* if namespace already exists - returns it, otherwise returns a newly created one
  */
  CNamespace& GetNamespace(const TEString& name, CNamespace* current=NULL)  {
    if( current == NULL ) current = nsGlobal;
    CNamespace* ns = (CNamespace*)current->FindType(name);
    if( ns == NULL ) ns = new CNamespace(name);
    else return *ns;
    current->AddType(ns);
    return *ns;
  }
  inline const TEString& GetProjectName()  const { return ProjectName; }
};

#endif

