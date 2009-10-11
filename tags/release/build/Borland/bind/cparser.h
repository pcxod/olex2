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
  olxstr &Value;
  TStrList &Args;
public:
  CMacro(TStrList* args, olxstr* value) : Value(*value), Args(*args)  {  }
  ~CMacro()  {
    delete &Value;
    if( &Args != NULL )  delete &Args;
  }
  const olxstr& Expand(TStrList* args) const {
    if( args != NULL )  {
      if( args->Count() != Args.Count() )
        throw TInvalidArgumentException(__OlxSourceInfo, "wrong number of macro arguments");
      olxstr& rv = TEGC::New<olxstr>(Value);
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
                rv.Insert(olxstr('"', args->String(i).Length()+3) << args->String(i) << '"', ai-1);
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
  olxstr ArgSignature() const {
    if( &Args == NULL )  return "()";
    olxstr rv('(');
    for( int i=0; i < Args.Count(); i++ )  {
      rv << Args[i];
      if( (i+1) < Args.Count() )
        rv << ',';
    }
    rv << ')';
    return rv;
  }
  const olxstr& GetValue() const { return Value;  }
};

class CPP  {
  TSStrPObjList<olxstr, CMacro*, false> Macros;
  TSStrObjList<olxstr, olxstr, false> Defines;
  static const olxstr defstr, incstr, ifdefstr, ifndefstr, ifstr, endifstr,
                        elsestr, elifstr, undefstr, linestr;
  TSyntaxParser* Parser;
  class CParser* CP;
protected:
  bool ValidateIf(const olxstr& val);
  void ProcessIf(const TStrList& lines, int &index, TStrList& output);
  void Process(TStrList& lines, TStrList& output);
  void WriteOutput(const olxstr& line, TStrList& output);
  void AnalyseDef(olxstr& line);
  /* removes spaces from the beginnig and the end, replaces multiple spaces with */
  //olxstr PreprocessLine(const olxstr& line);
public:
  CPP(CParser* cp);
  void Parse(TStrList& input, TStrList& output);
  virtual ~CPP();
  bool IsDefined(const olxstr& name) const {
    return  (Defines.IndexOf(name) != -1 || Macros.IndexOf(name) != -1);
  }
  const olxstr& GetValue(const olxstr& name) const {
    int ind = Defines.IndexOf(name);
    if( ind == -1 )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("undefined ") << name);
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
      olxstr DefName;
    public:
      DefEvaluable(DefEvaluatorFactory* parent, const olxstr& defn)  {
        Parent = parent;
        DefName = defn;
      }
      DefEvaluable()  {  ;  }
      bool Evaluate() const {  return Parent->IsDefined(DefName);  }
    };
    class DefEvaluator: public IDoubleEvaluator  {
      DefEvaluatorFactory *Parent;
      olxstr DefName;
    public:
      DefEvaluator(DefEvaluatorFactory* parent, const olxstr& defn)  {
        Parent = parent;
        DefName = defn;
      }
      virtual const olxstr& Evaluaolxstr()  const  {
        return Parent->GetValue(DefName);
      }
      virtual double EvaluateDouble()  const  {
        return Parent->GetValue(DefName).ToDouble();
      }
      IEvaluator *NewInstance(IDataProvider *dp)  {  return new DefEvaluator( (DefEvaluatorFactory*)dp, EmptyString);  }
      DefEvaluator()  {  ;  }
      //bool Evaluate() const {  return Parent->IsDefined(DefName);  }
    };
  public:
    IEvaluator *Evaluator(const olxstr &propertyName)  {
      DefEvaluator* de = new DefEvaluator(this, propertyName);
      Evaluators.Add(de);
      return de;
    }
    IEvaluable *Evaluable(const olxstr &propertyName)  {
      DefEvaluable* de = new DefEvaluable(this, propertyName);
      Evaluables.Add(de);
      return de;
    }
    IEvaluator* Evaluator(const olxstr& name, const olxstr &Val)  {
      DefEvaluator* de = new DefEvaluator(this, Val);
      Evaluators.Add(de);
      return de;
    }
    IEvaluable* Evaluable(const olxstr& name, const olxstr &Val)  {
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
    bool IsDefined(const olxstr& name) const {  return Parent->IsDefined(name);  }
    const olxstr& GetValue(const olxstr& name) const {  return Parent->GetValue(name);  }
    DefEvaluatorFactory(CPP *parent)  {
      Parent = parent;
    }
  };
private:
  CPP::DefEvaluatorFactory* Factory;
public:
  inline int MacroCount() const {  return Macros.Count();  }
  inline const CMacro& MacroValue(int i)  const  {  return *Macros.GetObject(i);  }
  inline const olxstr& MacroName(int i)  const  {  return Macros.GetString(i);  }
  olxstr&  ExpandMacros(olxstr& line);
  inline int DefineCount() const {  return Defines.Count();  }
  inline const olxstr& DefineName(int i)  const  {  return Defines.GetString(i);  }
  inline const olxstr& DefineValue(int i)  const  {  return Defines.GetObject(i);  }
  void Define( const olxstr& name, const olxstr& val)  {
    int ind = Defines.IndexOfComparable(name);
    if( ind != -1 )  Defines.Object(ind) = val;
    else
      Defines.Add(name, val);
  }
  void StartFileProcessing(const olxstr& fn)  {
    Define("__FILE__", olxstr("\"") << fn << '\"');
    Define("__DATE__", olxstr("\"") << TETime::FormatDateTime("yyyy.MM.dd", TETime::Now()) << '\"');
    Define("__TIME__", olxstr("\"") << TETime::FormatDateTime("hh:mm:ss", TETime::Now()) << '\"' );
  }

};

class CParser  {
  // preprocessor...
  TStrList Paths, Result;
  CModel* Model;
  olxstr StrCode;
  TStrList* Input;
  const static olxstr templatestr, namespacestr, classstr, structstr;
//  void ParseObject(const olxstr& code, int &ind, CStruct* cst);
  void _CParse(const olxstr &parent, int start, int end);
  void CParse(TStrList& code);
protected:
  bool IsNextWord(int &ind, int end, const olxstr& word);
  void ExtractExpression(int &i, int end, olxstr& exp);
  void ExtractNext(int &i, int end, olxstr& exp);
//  void ExtractRev(int &i, int start, olxstr& exp);

public:
  CParser()  { Model = NULL;  }
  ~CParser()  {
    if( Model != NULL )
      delete Model;
  }

  TStrList& Parse(const olxstr& filename);
  olxstr LocateFile(const olxstr& fn) const;
  void AddPath( const olxstr& path)  {
    if( Paths.IndexOf(path) == -1 )
      Paths.Add(path);
  }
};
#endif

