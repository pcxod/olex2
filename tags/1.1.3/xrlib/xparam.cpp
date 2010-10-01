/*

class IParameter  {
public:
  virtual double Get() const = 0;
  virtual void Set(double v) = 0;
};

template <class BC, typename Type> class TParamWrapper: public IParamWrapper {
  BC* Instance;
  Type (BC::*Getter)() const;
  void (BC::*Setter)(Type v);
public:
  TParamWrapper(BC *instance, Type (BC::*getter)() const, void (BC::*setter)(Type) )  {
    Instance = instance;
    Getter = getter;
    Setter = setter; 
  }
)
  virtual double Get()  const {  return (double)(Instance::->*Getter)();  }
  virtual void Set(double v)  {  (Instance::->*Getter)((Type)v);  }
};

struct _Refinables {
  short Tag;
  TEString Name;
};


enum _Refs {  refX, refY, refZ, refO, refUi, refUa };

_Refinables Refinables[] = { {refX, 'X'},
                             {refY, 'Y'},
                             {refZ, 'Z'},
                             {refO, 'O'},
                             {refUi, "Ui"},
                             {refUa, "Ua"} };

class TRefinable  {
  short Tag;
  bool Fixed;
  double Value, Coefficient;
public:
  TRefinable()
};

class TLEquation  {
  double Value;
  TVectorD Coefficients;

};

TScatterer  {
  RelativeOccupancy
  U
  TScattererLib* Scatterer;
  
};

class TAtomicPosition  {
  BasicAtominfo* Element;
  List< TScatterer >
  X,Y,Z;
  SiteOccupancy
  
};

*/

