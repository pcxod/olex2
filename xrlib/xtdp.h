#ifndef __olx_tdp
#define __olx_tdp
#include "xmodel.h"
BeginXlibNamespace()
class XIsotropicTDP : public ATDP {
public:
  XRefinable Uiso; 
  XIsotropicTDP() {
    Uiso.Name = "Uiso";  Uiso.Owner = this;
  }
  virtual double GetUisoVal() const {  return Uiso.Value;  }
  virtual bool IsAniso()      const {  return false;  }
  virtual ATDP& GetInstance()       {  return *this;  }
  virtual void SetRefinable(bool v) {  Uiso.Refinable = v;  }
  virtual bool IsRefinable()  const {  return Uiso.Refinable;  }
  virtual bool IsProxy()      const {  return false;  }
};

class XAnisotropicTDP : public ATDP {
public:
  XRefinable Uani[6]; 
  enum Uind { U11=0, U22, U33, U23, U13, U12 };
  XAnisotropicTDP() {
    Uani[0].Name = "U11";  Uani[0].Owner = this;
    Uani[1].Name = "U22";  Uani[1].Owner = this;
    Uani[2].Name = "U33";  Uani[2].Owner = this;
    Uani[3].Name = "U23";  Uani[3].Owner = this;
    Uani[4].Name = "U13";  Uani[4].Owner = this;
    Uani[5].Name = "U12";  Uani[5].Owner = this;
  }
  virtual double GetUisoVal() const {  return (Uani[0].Value+Uani[1].Value+Uani[2].Value)/3;  }
  virtual bool IsAniso()      const {  return true;  }
  virtual ATDP& GetInstance()       {  return *this;  }
  virtual void SetRefinable(bool v) {  
    Uani[0].Refinable = v;  Uani[1].Refinable = v;  Uani[2].Refinable = v;
    Uani[3].Refinable = v;  Uani[4].Refinable = v;  Uani[5].Refinable = v;
  }
  virtual bool IsRefinable()  const {  return Uani[0].Refinable;  }
  virtual bool IsProxy()      const {  return false;  }
};
//Proxy to be used for riding atoms, where Uiso depends on the pivot atom
class XTDPProxy : public ATDP {
  ITDP& Instance;
  double Scale;
public:
  XTDPProxy(ITDP& instance, double scale) : Instance(instance), Scale(scale)  {  }
  virtual double GetUisoVal() const {  return Instance.GetUisoVal()*Scale;  }
  virtual bool IsAniso()      const {  return Instance.IsAniso();  }
  virtual ATDP& GetInstance()       {  return Instance;  }
  virtual void SetRefinable(bool v) {  Instance.SetRefinable(v);  }
  virtual bool IsRefinable()  const {  return false;  }
  virtual bool IsProxy()      const {  return true;  }
};

EndXlibNamespace()

#endif
