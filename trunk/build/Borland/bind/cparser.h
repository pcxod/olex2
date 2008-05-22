#ifndef cparserH
#define cparserH
#include "pbase.h"
#include "estrlist.h"
#include "estlist.h"
#include "syntaxp.h"

#include "etime.h"
#include "cmodel.h"
#include "egc.h"

class CMacro  {
  TEString &Value;
  TStrList &Args;
public:
  CMacro(TStrList* args, TEString* value) : Value(*value), Args(*args)  {  }
  ~CMacro()  {
    delete &Value;
    if( &Args != NULL )  delete &Args;
  }
  const TEString& Expand(TStrList* args) const {
    if( args != NULL )  {
      if( args->Count() != Args.Count() )
        throw TInvalidArgumentException(__OlxSourceInfo, "wrong number of macro arguments");
      TEString& rv = TEGC::New<TEString>(Value);
      for(int i=0; i < Args.Count(); i++ )  {
        int ai = rv.FirstIndexOf( Args[i] );
        while( ai != -1 )  {
          if( ai > 0 )  {
            if( rv[ai-1] == '#' )  {
              if( ai > 1 && rv[ai-2] == '#' )  {  // simple concatenation
                rv.Delete(ai-2, Args[i].Length()+2);
                rv.Insert(args->String(i), ai-2);
                ai += (2+args->String(i).Length());
              }
              else  { // whatever to string
                rv.Delete(ai-1, Args[i].Length()+1);
                rv.Insert(TEString('"', args->String(i).Length()+3) << args->String(i) << '"', ai-1);
                ai += (1+args->String(i).Length());
              }
            }
            else  {  // do not replace an occurence in a longer word
              if( (rv[ai-1] >= 'a' && rv[ai-1] <= 'z') ||
                  (rv[ai-1] >= 'A' && rv[ai-1] <= 'Z') ||
                  (rv[ai-1] >= '0' && rv[ai-1] <= '9') || rv[ai-1] == '_' )  {  ai += Args[i].Length();  }
              else  { // it is a "standalone" word, replace it
                rv.Delete(ai, Args[i].Length());
                rv.Insert(args->String(i), ai);
                ai += (args->String(i).Length());
              }
            }
          }
          else  {
            rv.Delete(ai, Args[i].Length());
            rv.Insert(args->String(i), ai);
            ai += (args->String(i).Length());
          }
          ai = rv.FirstIndexOf( Args[i], ai );
        }
      }
      return rv;
    }
    else
      return Value;
  }
  TEString ArgSignature() const {
    if( &Args == NULL )  return "()";
    TEString rv('(');
    for( int i=0; i < Args.Count(); i++ )  {
      rv << Args[i];
      if( (i+1) < Args.Count() )
        rv << ',';
    }
    rv << ')';
    return rv;
  }
  const TEString& GetValue() const { return Value;  }
};

class CPP  {
  TSStrPObjList<CMacro*, false> Macros;
  TSStrObjList<TEString, false> Defines;
  static const TEString defstr, incstr, ifdefstr, ifndefstr, ifstr, endifstr,
                        elsestr, elifstr, undefstr, linestr;
  TSyntaxParser* Parser;
  class CParser* CP;
protected:
  bool ValidateIf(const TEString& val);
  void ProcessIf(const TStrList& lines, int &index, TStrList& output);
  void Process(TStrList& lines, TStrList& output);
  void WriteOutput(const TEString& line, TStrList& output);
  void AnalyseDef(TEString& line);
  /* removes spaces from the beginnig and the end, replaces multiple spaces with */
  //TEString PreprocessLine(const TEString& line);
public:
  CPP(CParser* cp);
  void Parse(TStrList& input, TStrList& output);
  virtual ~CPP();
  bool IsDefined(const TEString& name) const {
    return  (Defines.IndexOf(name) != -1 || Macros.IndexOf(name) != -1);
  }
  const TEString& GetValue(const TEString& name) const {
    int ind = Defines.IndexOf(name);
    if( ind == -1 )
      throw TInvalidArgumentException(__OlxSourceInfo, TEString("undefined ") << name);
    return Defines.GetObject(ind);
  }
protected:

  class DefEvaluatorFactory: public IEvaluatorFactory, public IDataProvider {
    // the list of all evaluators
    TPtrList<IEvaluable> Evaluables;
    TPtrList<IEvaluator> Evaluators;
    CPP* Parent;
  protected:
    class DefEvaluable: public IEvaluable  {
      DefEvaluatorFactory *Parent;
      TEString DefName;
    public:
      DefEvaluable(DefEvaluatorFactory* parent, const TEString& defn)  {
        Parent = parent;
        DefName = defn;
      }
      DefEvaluable()  {  ;  }
      bool Evaluate() const {  return Parent->IsDefined(DefName);  }
    };
    class DefEvaluator: public IDoubleEvaluator  {
      DefEvaluatorFactory *Parent;
      TEString DefName;
    public:
      DefEvaluator(DefEvaluatorFactory* parent, const TEString& defn)  {
        Parent = parent;
        DefName = defn;
      }
      virtual const TEString& EvaluateString()  const  {
        return Parent->GetValue(DefName);
      }
      virtual double EvaluateDouble()  const  {
        return Parent->GetValue(DefName).Double();
      }
      IEvaluator *NewInstance(IDataProvider *dp)  {  return new DefEvaluator( (DefEvaluatorFactory*)dp, EmptyString);  }
      DefEvaluator()  {  ;  }
      //bool Evaluate() const {  return Parent->IsDefined(DefName);  }
    };
  public:
    IEvaluator *Evaluator(const TEString &propertyName)  {
      DefEvaluator* de = new DefEvaluator(this, propertyName);
      Evaluators.Add(de);
      return de;
    }
    IEvaluable *Evaluable(const TEString &propertyName)  {
      DefEvaluable* de = new DefEvaluable(this, propertyName);
      Evaluables.Add(de);
      return de;
    }
    IEvaluator* Evaluator(const TEString& name, const TEString &Val)  {
      DefEvaluator* de = new DefEvaluator(this, Val);
      Evaluators.Add(de);
      return de;
    }
    IEvaluable* Evaluable(const TEString& name, const TEString &Val)  {
      DefEvaluable* de = new DefEvaluable(this, Val);
      Evaluables.Add(de);
      return de;
    }
    ~DefEvaluatorFactory()  {
      for( int i=0; i < Evaluators.Count(); i++ )
        delete Evaluators[i];
      for( int i=0; i < Evaluables.Count(); i++ )
        delete Evaluables[i];
    }
    bool IsDefined(const TEString& name) const {  return Parent->IsDefined(name);  }
    const TEString& GetValue(const TEString& name) const {  return Parent->GetValue(name);  }
    DefEvaluatorFactory(CPP *parent)  {
      Parent = parent;
    }
  };
private:
  CPP::DefEvaluatorFactory* Factory;
public:
  inline int MacroCount() const {  return Macros.Count();  }
  inline const CMacro& MacroValue(int i)  const  {  return *Macros.GetObject(i);  }
  inline const TEString& MacroName(int i)  const  {  return Macros.GetString(i);  }
  TEString&  ExpandMacros(TEString& line);
  inline int DefineCount() const {  return Defines.Count();  }
  inline const TEString& DefineName(int i)  const  {  return Defines.GetString(i);  }
  inline const TEString& DefineValue(int i)  const  {  return Defines.GetObject(i);  }
  void Define( const TEString& name, const TEString& val)  {
    int ind = Defines.IndexOfComparable(name);
    if( ind != -1 )  Defines.Object(ind) = val;
    else
      Defines.Add(name, val);
  }
  void StartFileProcessing(const TEString& fn)  {
    Define("__FILE__", TEString("\"") << fn << '\"');
    Define("__DATE__", TEString("\"") << TETime::FormatDateTime("yyyy.MM.dd", TETime::Now()) << '\"');
    Define("__TIME__", TEString("\"") << TETime::FormatDateTime("hh:mm:ss", TETime::Now()) << '\"' );
  }

};

class CParser  {
  // preprocessor...
  TStrList Paths, Result;
  CModel* Model;
  TEString StrCode;
  TStrList* Input;
  const static TEString templatestr, namespacestr, classstr, structstr;
//  void ParseObject(const TEString& code, int &ind, CStruct* cst);
  void _CParse(const TEString &parent, int start, int end);
  void CParse(TStrList& code);
protected:
  bool IsNextWord(int &ind, int end, const TEString& word);
  void ExtractExpression(int &i, int end, TEString& exp);
  void ExtractNext(int &i, int end, TEString& exp);
//  void ExtractRev(int &i, int start, TEString& exp);

public:
  CParser()  { Model = NULL;  }
  ~CParser()  {
    if( Model != NULL )
      delete Model;
  }

  TStrList& Parse(const TEString& filename);
  TEString LocateFile(const TEString& fn) const;
  void AddPath( const TEString& path)  {
    if( Paths.IndexOf(path) == -1 )
      Paths.Add(path);
  }
};
#endif

