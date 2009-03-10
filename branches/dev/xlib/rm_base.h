// need this file to resolve includes and dependencies... not to be included first :)
#ifndef __olx_rm_base_H
#define __olx_rm_base_H
BeginXlibNamespace()

class IXVarReferencer;
class IXVarReferencerContainer;

class IXVarReferencer {
public:
  virtual short VarCount() const = 0;
  virtual const struct XVarReference* GetVarRef(short i) const = 0;
  virtual XVarReference* GetVarRef(short i) = 0;
  virtual olxstr GetVarName(short i) const = 0;
  virtual void SetVarRef(short i, XVarReference* var_ref) = 0;
  virtual double GetValue(short var_index) const = 0;
  virtual void SetValue(short var_index, const double& val) = 0;
  virtual IXVarReferencerContainer& GetParentContainer() = 0;
  virtual bool IsValid() const = 0;
  virtual int GetReferencerId() const;  // defined in leg.cpp ...
  virtual olxstr GetIdName() const = 0;
};

class IXVarReferencerContainer  {
public:
  // returns an object id and the name of id, like 'atom', 'dfix', 'basf', etc
  virtual olxstr GetIdName() const = 0;
  virtual int GetReferencerId(const IXVarReferencer& vr) const = 0;
  virtual int ReferencerCount() const = 0;
  virtual IXVarReferencer* GetReferencer(int id) = 0;
};

EndXlibNamespace()
#endif
