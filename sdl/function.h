/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_function_H
#define __olx_sdl_function_H

#define DefMacro(macroName) void mac##macroName(TStrObjList &Cmds, const TParamList &Options, TMacroError &E);
#define DefFunc(funcName) void fun##funcName(const TStrObjList &Cmds, TMacroError &E);

#define InitFunc(library, baseClass, funcName, argc) \
  (library).RegisterFunction( new TFunction<baseClass>(this, &baseClass::fun##funcName, #funcName, argc))
#define InitFuncA(library, baseClass, funcName, funcNameA, argc) \
  (library).RegisterFunction( new TFunction<baseClass>(this, &baseClass::fun##funcName, #funcNameA, argc))
#define InitMacro(library, baseClass, macroName, validOptions, argc)\
  (library).RegisterMacro( new TMacro<baseClass>(this, &baseClass::mac##macroName, #macroName, #validOptions, argc))
#define InitMacroA(library, baseClass, realMacroName, macroName, validOptions, argc)\
  (library).RegisterMacro( new TMacro<baseClass>(this, &baseClass::mac##realMacroName, #macroName, #validOptions, argc))

#define InitFuncD(library, baseClass, funcName, argc, desc) \
  (library).RegisterFunctionD( new TFunction<baseClass>(this, &baseClass::fun##funcName, #funcName, argc, desc))
#define InitFuncAD(library, baseClass, funcName, funcNameA, argc, desc) \
  (library).RegisterFunction( new TFunction<baseClass>(this, &baseClass::fun##funcName, #funcNameA, argc, desc))
#define InitMacroD(library, baseClass, macroName, validOptions, argc, desc)\
  (library).RegisterMacro( new TMacro<baseClass>(this, &baseClass::mac##macroName, #macroName, validOptions, argc, desc))
#define InitMacroAD(library, baseClass, realMacroName, macroName, validOptions, argc, desc)\
  (library).RegisterMacro( new TMacro<baseClass>(this, &baseClass::mac##realMacroName, #macroName, validOptions, argc, desc))

#include "estlist.h"
#include "paramlist.h"

#include "macroerror.h"
#include "tptrlist.h"
BeginEsdlNamespace()

/* these constans specify the number of parameters (bytes [0..3]
   and any special checks for the state in which func/macro can be exexuted
   in bytes [4..7]. For example some functions can be executed only if a file
   is loaded...
*/
const uint32_t
  fpNone  = 0x00000001,
  fpOne   = 0x00000002,
  fpTwo   = 0x00000004,
  fpThree = 0x00000008,
  fpFour  = 0x00000010,
  fpFive  = 0x00000020,
  fpSix   = 0x00000040,
  fpSeven = 0x00000080,
  fpEight = 0x00000100,
  fpNine  = 0x00000200,
  fpTen   = 0x00000400,
  fpAny   = 0x0000ffff,

  fpSpecialCheckA = 0x00010000,
  fpSpecialCheckB = 0x00020000,
  fpSpecialCheckC = 0x00040000,
  fpSpecialCheckD = 0x00080000,
  fpSpecialCheckE = 0x00100000,
  fpSpecialCheckF = 0x00200000
  // .... 1 << n
  ;

class ALibraryContainer: public IEObject  {
private:
  struct TProgramStateDescriptor {
    uint32_t StateBit;
    olxstr StateDescription;
  };
  TPtrList<TProgramStateDescriptor> ProgramStates;
  TProgramStateDescriptor* FindState(uint32_t stateBit)  {
    for( size_t i=0; i < ProgramStates.Count(); i++ )
      if( ProgramStates[i]->StateBit == stateBit )
        return ProgramStates[i];
    return NULL;
  }
protected:
  void DefineState(uint32_t specialCheck, const olxstr& description)  {
    TProgramStateDescriptor* ps = new TProgramStateDescriptor;
    ps->StateBit = specialCheck;
    ps->StateDescription = description;
    ProgramStates.Add( ps );
  }
public:
  virtual ~ALibraryContainer()  {
    for( size_t i=0; i < ProgramStates.Count(); i++ )
      delete ProgramStates[i];
  }
  olxstr GetStateName(uint32_t specialCheck)  {
    olxstr stateDescr;
    for( int i=16; i < 32; i++ )  {
      if( specialCheck & (1 << i) )  {
        TProgramStateDescriptor* ps = FindState( 1 << i );
        if( ps != NULL )
          stateDescr << '[' << ps->StateDescription << ']';
        else
          throw TFunctionFailedException(__OlxSourceInfo, "unregistered state");
      }
    }
    return stateDescr;
  }

  virtual class TLibrary&  GetLibrary() = 0;
  virtual bool CheckProgramState(uint32_t specialCheck) = 0;
};

class ABasicLibrary  {
public:
  virtual ~ABasicLibrary()  {}
  virtual bool CheckProgramState(uint32_t state) = 0;
  virtual ALibraryContainer* GetOwner() const = 0;
  virtual ABasicLibrary* GetParentLibrary() const = 0;
  virtual const olxstr& GetName() const = 0;
  olxstr GetQualifiedName() const;
};


class ABasicFunction: public IEObject  {
  ABasicLibrary* ParentLibrary;
  olxstr Name;
  olxstr Description;
protected:
  void SetName(const olxstr& n)  {  Name = n;  }
  void ParseOptions(const olxstr& Options, TCSTypeList<olxstr,olxstr>& list);
  olxstr OptionsToString(const TCSTypeList<olxstr,olxstr>& list) const;
  uint32_t ArgStateMask;
  olxstr RunSignature;
  olxstr SubstituteArgs(const olxstr &arg, const TStrList &argv) const;
public:
  virtual ~ABasicFunction()  {}
  virtual void Run(const TStrObjList& Params, TMacroError& E) = 0;
  virtual void Run(TStrObjList& Params, const TParamList& options,
    TMacroError& E) = 0;
  virtual void MacroRun(const TStrObjList& Params, TMacroError& E,
    const TStrList &argv);
  virtual void MacroRun(TStrObjList& Params, const TParamList& options,
    TMacroError& E, const TStrList &argv);
  DefPropP(uint32_t, ArgStateMask)
  const olxstr& GetName() const {  return Name;  }
  const olxstr& GetDescription() const {  return Description;  }
  void SetDescription(const olxstr& d) {  Description = d;  }
  olxstr GetQualifiedName() const;
  void SetParentLibrary(ABasicLibrary& parent) {  ParentLibrary = &parent;  }
  ABasicLibrary* GetParentLibrary()  const {  return ParentLibrary;  }
  virtual olxstr GetSignature() const;
  virtual bool HasOptions() const = 0;
  virtual const TCSTypeList<olxstr,olxstr>& GetOptions() const = 0;
  virtual void SetOptions(const TCSTypeList<olxstr,olxstr>&)  {
    if (!HasOptions())
      throw TNotImplementedException(__OlxSourceInfo);
  }
  const olxstr& GetRuntimeSignature() const { return RunSignature; }
  virtual ABasicFunction* Replicate() const = 0;
  bool ValidateState(const TStrObjList &Params, TMacroError &E)  {
    const size_t argC = Params.Count(),
      arg_m = (0x0001 << argC);
    if( (ArgStateMask&fpAny) != fpAny && (ArgStateMask&arg_m) == 0)  {
      E.WrongArgCount(*this, argC);
      return false;
    }
    // the special checks are in the high word
    if( (ArgStateMask&0xFFFF0000) &&
        !GetParentLibrary()->CheckProgramState(ArgStateMask) )
    {
      E.WrongState(*this);
      return false;
    }
    return true;
  }
};
//------------------------------------------------------------------------------
class AFunction: public ABasicFunction  {
protected:
  virtual void DoRun(const TStrObjList &Params, TMacroError& E) = 0;
public:
  AFunction(const olxstr& funcName, uint32_t argc,
    const olxstr& desc=EmptyString())
  {
    ArgStateMask = argc;
    SetName(funcName);
    SetDescription(desc);
  }
  virtual void Run(TStrObjList&, const TParamList&, TMacroError&)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }

  virtual bool HasOptions() const { return false; }
  virtual const TCSTypeList<olxstr,olxstr>& GetOptions() const {
    throw TNotImplementedException(__OlxSourceInfo);
  }

  virtual void Run(const TStrObjList &Params, class TMacroError& E)  {
    if( !ValidateState(Params, E) )  return;
    const size_t argC = Params.Count();
    try  {
      RunSignature = olxstr(GetName(), 128);
      RunSignature << '(';
      for( size_t i=0; i < argC; i++ )  {
        RunSignature << '[' << Params[i] << ']';
        if( i < (argC-1) )  RunSignature << ", ";
      }
      RunSignature << ')';
      DoRun(Params, E);
    }
    catch( TExceptionBase& exc )  {
      E.ProcessingException(*this, exc);
    }
  };
};
//------------------------------------------------------------------------------
template <class Base>
class TFunction: public AFunction  {
  Base* BaseClassInstance;
  void (Base::*Func)(const TStrObjList& Params, TMacroError& E);
public:
  TFunction(Base* baseClassInstance,
    void (Base::*func)(const TStrObjList& Params, TMacroError& E),
    const olxstr& funcName, uint32_t argc,
    const olxstr& desc=EmptyString())
    : AFunction(funcName, argc, desc)
  {
    BaseClassInstance = baseClassInstance;
    Func = func;
  }
  virtual ABasicFunction* Replicate() const  {
    return new TFunction<Base>(BaseClassInstance, Func, GetName(),
      ArgStateMask, GetDescription());
  }
protected:
  virtual void DoRun(const TStrObjList &Params, TMacroError& E)  {
    (BaseClassInstance->*Func)(Params, E);
  };
};
//------------------------------------------------------------------------------
class TStaticFunction: public AFunction  {
  void (*Func)(const TStrObjList& Params, TMacroError& E);
public:
  TStaticFunction(void (*func)(const TStrObjList& Params, TMacroError& E),
    const olxstr& funcName, uint32_t argc,
    const olxstr& desc=EmptyString())
    : AFunction(funcName, argc, desc), Func(func)
  {}

  virtual ABasicFunction* Replicate() const {
    return new TStaticFunction(Func, GetName(), ArgStateMask, GetDescription());
  }
protected:
  virtual void DoRun(const TStrObjList &Params, TMacroError& E)  {
    Func(Params, E);
  };
};
//------------------------------------------------------------------------------
class AMacro: public ABasicFunction  {
protected:
  TCSTypeList<olxstr,olxstr> ValidOptions;
  virtual void DoRun(TStrObjList &Params, const TParamList &Options,
    TMacroError& E) = 0;
public:
  AMacro(const olxstr& macroName, const olxstr& validOptions,
    uint32_t argc, const olxstr& desc=EmptyString())
  {
    ArgStateMask = argc;
    SetName(macroName);
    SetDescription(desc);
    ParseOptions(validOptions, ValidOptions);
  }
  virtual bool HasOptions() const { return true; }
  virtual const TCSTypeList<olxstr,olxstr>& GetOptions() const {
    return ValidOptions;
  }
  virtual void SetOptions(const TCSTypeList<olxstr,olxstr>& opts)  {
    ValidOptions = opts;
  }
  virtual void Run(const TStrObjList& Params, TMacroError& E)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual void Run(TStrObjList &Params, const TParamList &Options,
    TMacroError& E)
  {
    if( !ValidateState(Params, E) )  return;
    const size_t argC = Params.Count();
    for( size_t i=0; i < Options.Count(); i++ )  {
      if( ValidOptions.IndexOf(Options.GetName(i)) == InvalidIndex )  {
        E.WrongOption(*this, Options.GetName(i) );
        return;
      }
    }
    try  {
      RunSignature = olxstr(GetName(), 128);
      RunSignature << ' ';
      for( size_t i=0; i < argC; i++ )  {
        RunSignature << '[' << Params[i] << ']';
        if( i < (argC-1) )  RunSignature << ", ";
      }
      RunSignature << ' ';
      for( size_t i=0; i < Options.Count(); i++ )  {
        RunSignature << '{' << Options.GetName(i) << '=' <<
          Options.GetValue(i) << '}';
      }
      DoRun(Params, Options, E);
    }
    catch( TExceptionBase& exc )  {
      E.ProcessingException(*this, exc);
    }
  };
  virtual olxstr GetSignature() const {
    if( ValidOptions.Count() )  {
      olxstr res = ABasicFunction::GetSignature();
      res << "; valid options - ";
      for( size_t i=0; i < ValidOptions.Count(); i++ )
        res << ValidOptions.GetKey(i)  << ';';
      return res;
    }
    else
      return ABasicFunction::GetSignature();
  }
};
//------------------------------------------------------------------------------
template <class Base>
class TMacro: public AMacro {
  Base* BaseClassInstance;
  void (Base::*Macro)(TStrObjList& Params, const TParamList &Options,
    TMacroError& E);
public:
  TMacro(Base* baseClassInstance,
    void (Base::*macro)(TStrObjList& Params,
    const TParamList &Options, TMacroError& E),
    const olxstr& macroName, const olxstr& validOptions,
    uint32_t argc, const olxstr& desc=EmptyString())
    : AMacro(macroName, validOptions, argc, desc)
  {
    BaseClassInstance = baseClassInstance;
    Macro = macro;
  }

  virtual ABasicFunction* Replicate() const {
    return new TMacro<Base>(
      BaseClassInstance, Macro, GetName(),
      OptionsToString(ValidOptions), ArgStateMask,
      GetDescription());
  }
  Base &GetBaseInstance() const { return *BaseClassInstance; }
protected:
  virtual void DoRun(TStrObjList &Params, const TParamList &Options,
    TMacroError& E)
  {
    (BaseClassInstance->*Macro)(Params, Options, E);
  }
};
//------------------------------------------------------------------------------
template <class Base>
class TMacroMacro: public AMacro {
  Base* BaseClassInstance;
  void (Base::*Macro)(TStrObjList& Params, const TParamList &Options,
    TMacroError& E, const TStrList &argv);
public:
  TMacroMacro(Base* baseClassInstance,
    void (Base::*macro)(TStrObjList& Params,
      const TParamList &Options, TMacroError& E, const TStrList &argv),
    const olxstr& macroName, const olxstr& validOptions,
    uint32_t argc, const olxstr& desc=EmptyString())
    : AMacro(macroName, validOptions, argc, desc)
  {
    BaseClassInstance = baseClassInstance;
    Macro = macro;
  }

  virtual ABasicFunction* Replicate() const {
    return new TMacroMacro<Base>(
      BaseClassInstance, Macro, GetName(),
      OptionsToString(ValidOptions), ArgStateMask,
      GetDescription());
  }
  virtual void MacroRun(TStrObjList &Params, const TParamList &Options,
    TMacroError& E, const TStrList &argv)
  {
    if( !ValidateState(Params, E) )  return;
    const size_t argC = Params.Count();
    for( size_t i=0; i < Options.Count(); i++ )  {
      if( ValidOptions.IndexOf(Options.GetName(i)) == InvalidIndex )  {
        E.WrongOption(*this, Options.GetName(i) );
        return;
      }
    }
    try  {
      RunSignature = olxstr(GetName(), 128);
      RunSignature << ' ';
      for( size_t i=0; i < argC; i++ )  {
        RunSignature << '[' << Params[i] << ']';
        if( i < (argC-1) )  RunSignature << ", ";
      }
      RunSignature << ' ';
      for( size_t i=0; i < Options.Count(); i++ )  {
        RunSignature << '{' << Options.GetName(i) << '=' <<
          Options.GetValue(i) << '}';
      }
      (BaseClassInstance->*Macro)(Params, Options, E, argv);
    }
    catch( TExceptionBase& exc )  {
      E.ProcessingException(*this, exc);
    }
  }
//.............................................................................
protected:
  virtual void DoRun(TStrObjList &Params, const TParamList &Options,
    TMacroError& E)
  {
    (BaseClassInstance->*Macro)(Params, Options, E, TStrList());
  }
};
//------------------------------------------------------------------------------
class TStaticMacro: public AMacro {
  void (*Macro)(TStrObjList& Params, const TParamList &Options,
    TMacroError& E);
public:
  TStaticMacro(void (*macro)(TStrObjList& Params,
    const TParamList &Options, TMacroError& E),
    const olxstr& macroName, const olxstr& validOptions,
    uint32_t argc, const olxstr& desc=EmptyString())
    : AMacro(macroName, validOptions, argc, desc), Macro(macro)
  {}

  virtual ABasicFunction* Replicate() const {
    return new TStaticMacro(Macro, GetName(), OptionsToString(ValidOptions),
      ArgStateMask, GetDescription());
  }
protected:
  virtual void DoRun(TStrObjList &Params, const TParamList &Options,
    TMacroError& E)
  {
    (*Macro)(Params, Options, E);
  };
};

typedef TPtrList<ABasicFunction> TBasicFunctionPList;
typedef TPtrList<ABasicLibrary> TBasicLibraryPList;

class FunctionChainer {
  TBasicFunctionPList functions;
public:
  FunctionChainer() {}
  ~FunctionChainer() { functions.DeleteItems(false); }
  FunctionChainer &Add(ABasicFunction *f) {
    functions.Add(f);
    return *this;
  }
  void RunMacro(TStrObjList &Params, const TParamList &Options,
    TMacroError& E);
  void RunFunction(const TStrObjList &Params, TMacroError& E);
  void Update(TMacro<FunctionChainer> &m);
};

EndEsdlNamespace()
#endif
