#ifdef __BORLANDC__
  #pragma hdrstop
#endif
#include "etime.h"
#include "efile.h"
#include "cparser.h"

const olxstr CPP::defstr("#define");
const olxstr CPP::incstr("#include");
const olxstr CPP::ifdefstr("#ifdef");
const olxstr CPP::ifndefstr("#ifndef");
const olxstr CPP::ifstr("#if");
const olxstr CPP::endifstr("#endif");
const olxstr CPP::elsestr("#else");
const olxstr CPP::elifstr("#elif");
const olxstr CPP::undefstr("#undef");
const olxstr CPP::linestr("__LINE__");
const olxstr CParser::templatestr("template");
const olxstr CParser::namespacestr("namespace");
const olxstr CParser::classstr("class");
const olxstr CParser::structstr("struct");

olxstr PreprocessLine(const olxstr& line)  {
  if( line.Length() == 0 )  return line;

  olxstr rv(EmptyString, line.Length() );
  int si = 0, ei = line.Length()-1;
  while( (line[si] == ' ' || line[si] == '\t') && si <= ei )  si++;
  while( (line[ei] == ' ' || line[ei] == '\t') && ei >= si )  ei--;
  if( ei < si )  return EmptyString;
  for( int i=si; i <= ei; i++ )  {
    if( line[i] == '\'' || line[i] == '\"' )  {
      char quote = line[i];
      rv << quote;
      while( ++i <= ei && line[i] != quote )  rv << line[i];
    }
    if( i == si && line[i] == '#')  {
      rv << '#';
      while( ++i < ei && (line[i] == ' ' || line[i] == '\t') )  ;
      i--;
      continue;
    }
    if( (line[i] == ' ' || line[i] == '\t') && i < ei )  {
      if( line[i+1] == ' ' || line[i+1] == '\t' )  continue;  // multiple spaces
      else if( line[i+1] == ')' )  continue;  // space after args
      else if( line[i+1] == ';' )  continue;  //
      else if( line[i+1] == '{' )  continue;  //
      else if( line[i+1] == '}' )  continue;  //
      else if( line[i+1] == ']' )  continue;  //
    }
    else if( line[i] == '(' && i < ei )  {
      if( line[i+1] == ' ' || line[i+1] == '\t' )  {
        rv << '(';  i++;
        continue;
      }
    }
    else if( line[i] == '[' && i < ei )  {
      if( line[i+1] == ' ' || line[i+1] == '\t' )  {
        rv << '[';  i++;
        continue;
      }
    }
    rv << ((line.CharAt(i) != '\t') ? line.CharAt(i) : ' ');
  }
  return rv;
}
void CPP::AnalyseDef(olxstr& line)  {
  int obi = line.FirstIndexOf('(');
  if( obi == -1 || (obi > 0 && line[obi-1] == ' ') )  {  // just define?
    obi = line.FirstIndexOf(' ');
    olxstr macroName = (obi==-1) ? line : line.SubStringTo(obi);
    olxstr macroValue = (obi==-1) ? EmptyString : line.SubStringFrom(obi+1);
    //ExpandMacros(macroValue);
    int ind = Defines.IndexOf(macroName);
    if(  ind != -1 )  {
      //throw TFunctionFailedException(__OlxSourceInfo, olxstr("Macro redifinition") << macroName );
      Defines.Object(ind) = macroValue;
    }
    else
      Defines.Add(macroName,  macroValue);
    line = "// variable definition ";
    line << macroName;
    return;
  }
  else  { // a "function" macro
    int bc = 1;
    for( int i=obi+1; i < line.Length(); i++ )  {
      if( line[i] == ')' )  bc--;
      else if( line[i] == '(' )  bc++;

      if( bc == 0 )  {  // now we know where the arguments are and the macro body
        olxstr macroName = line.SubStringTo(obi);
        olxstr* macroBody;
        TStrList* args = NULL;
        if( obi == i-1 )  {  // empty args
          macroBody = new olxstr( line.SubStringFrom(i+1).Trim(' ') );
        }
        else  {
          olxstr strArgs( line.SubString(obi+1, i-obi-1) );
          macroBody = new olxstr( line.SubStringFrom(i+1) );
          args = new TStrList(olxstr::DeleteChars(strArgs, ' '), ',');
        }
        //ExpandMacros(*macroBody);
        int ind = Macros.IndexOf(macroName);
        if( ind == -1 )
          Macros.Add( macroName, new CMacro(args, macroBody) );
        else  {
          delete Macros.Object(ind);
          Macros.Object(ind) = new CMacro(args, macroBody);
        }
        line = "// macro definition ";
        line << macroName;
        return;
      }
    }
  }
  throw TFunctionFailedException(__OlxSourceInfo, olxstr("Invalid macro definition - ") << line);
}

void CPP::ProcessIf(const TStrList& lines, int &index, TStrList& output)  {
  bool condition;
  if( lines[index].StartsFrom(ifdefstr) )
    condition = IsDefined(lines[index].SubStringFrom( ifdefstr.Length()+1 ));
  else if( lines[index].StartsFrom(ifndefstr) )
    condition = !IsDefined(lines[index].SubStringFrom( ifndefstr.Length()+1 ));
  else if( lines[index].StartsFrom(ifstr) )
    condition = ValidateIf( lines[index].SubStringFrom( ifstr.Length()+1 ) );
  else  {
    if( !lines[index][0] == '#' )
      WriteOutput( ExpandMacros(lines[index]), output);
    return;
  }

  int oifc = 1;  // open ifs count;
  index ++;

  TStrList next;

  for( ; index < lines.Count(); index++ )  {
    if( lines[index].IsEmpty() )  continue;

    //if( lines[index][0] != '#' )  continue;

    if( lines[index].StartsFrom(endifstr) ) {
      oifc--;
      if( oifc == 0 )  break; // should not add this to output :)
    }
    else if( lines[index].StartsFrom(ifstr) ) oifc++;
    else if( lines[index].StartsFrom(ifdefstr) ) oifc++;
    else if( lines[index].StartsFrom(ifndefstr) ) oifc++;
    else if( oifc == 1 && lines[index].StartsFrom(elsestr) ) {
      if( !condition )  condition = true;
      else  {
        while( oifc != 0 && ++index < lines.Count() )  {
          if( lines[index].StartsFrom(endifstr) )  oifc--;
          else if( lines[index].StartsFrom(ifstr) ) oifc++;
          else if( lines[index].StartsFrom(ifdefstr) ) oifc++;
          else if( lines[index].StartsFrom(ifndefstr) ) oifc++;
        }
        if( oifc != 0 )
          throw TFunctionFailedException(__OlxSourceInfo, "could not locate matching #endif");
        break;
      }
      continue;  // skip adding this line in any case
    }
    else if( oifc == 1 && lines[index].StartsFrom(elifstr) ) {
      condition = ValidateIf( lines[index].SubStringFrom(elifstr.Length()+1) );
      if( condition )  {
        while( oifc != 0 && ++index < lines.Count() )  {
          if( condition && lines[index].StartsFrom(elifstr) )  condition = false;
          else if( condition && lines[index].StartsFrom(elsestr) )  condition = false;
          else if( lines[index].StartsFrom(endifstr) )  oifc--;
          else if( lines[index].StartsFrom(ifstr) ) oifc++;
          else if( lines[index].StartsFrom(ifdefstr) ) oifc++;
          else if( lines[index].StartsFrom(ifndefstr) ) oifc++;
          if( condition )  next << lines[index];
        }
        break;
      }
      continue;  // skip adding this line in any case
    }
    if( condition )  next << lines[index];
    if( oifc == 0 )      break;
  }
  Process(next, output);
}

int LocateName( const olxstr& line, const olxstr& what )  {
  if( line.Length() < what.Length() )  return -1;
  int al = line.Length(), bl = what.Length();
  const char *a = line.c_str();
  const char *b = what.c_str();
  for( int i=0; i < al; i++ )  {
    if( i + bl > al )  return -1;
    bool found = true;
    for( int j=0; j < bl; j++ )  {
      if( a[i+j] != b[j] )  {  found = false;  break;  }
      if( j == 0 && i > 0 )  {
        if( (a[i-1] >= 'a' && a[i-1] <= 'z') ||
            (a[i-1] >= 'A' && a[i-1] <= 'Z') ||
            (a[i-1] >= '0' && a[i-1] <= '9') || a[i-1] == '_' )  {  found = false;  break;  }
      }
    }
    if( found )  {
      if( (i+bl+1) < al )  {
        if( (a[i+bl] >= 'a' && a[i+bl] <= 'z') ||
            (a[i+bl] >= 'A' && a[i+bl] <= 'Z') ||
            (a[i+bl] >= '0' && a[i+bl] <= '9') || a[i+bl] == '_' )  {  found = false;  }
      }
      if( found )  return i;
    }
  }
  return -1;
}
olxstr& CPP::ExpandMacros(olxstr& line)  {
  if( line.IsEmpty() )  return line;
  if( line.Length() >= 2 && line[0] == '/' && line[1] == '/' )  return line;

  TStrList args;
  olxstr strArgs;
  bool changes = true, changed = false;
  while( changes )  {
    changes = false;
    for( int i=0; i < MacroCount(); i++ )  {
      int sind = LocateName(line, MacroName(i) );
      if( sind == -1 ) continue;
      int eind = sind + MacroName(i).Length();
      strArgs.SetLength(0);
      if( (eind+1) < line.Length() && line[eind] == '(' )  {
        int bc = 1;
        while( ++eind < line.Length() )  {
          if( line[eind] == '(' )  bc++;
          else if( line[eind] == ')' )  bc--;
          if( bc == 0 )  break;
          strArgs << line[eind];
        }
        line.Delete( sind, eind-sind+1 );

        args.Clear();
        args.Strtok(olxstr::DeleteChars(strArgs, ' '), ',');
        line.Insert((args.Count() != 0) ? MacroValue(i).Expand(&args) : MacroValue(i).Expand(NULL), sind );
        changed = changes = true;
      }
    }
  } // this will operate on the macro expansion too
  changes = true;
  while( changes )  {
    changes = false;
    for( int i=0; i < DefineCount(); i++ )  {
      int sind = LocateName(line, DefineName(i) );
      if( sind == -1 ) continue;
      int eind = sind + DefineName(i).Length();
      changed = changes = true;
      line.Delete( sind, eind-sind );
      line.Insert(DefineValue(i), sind );
    }
  }
  if( changed )  line = PreprocessLine(line);
  return line;
}

void CPP::Process(TStrList& lines, TStrList& output)  {
  olxstr line, fn;
  TStrList toks;
  for( int i=0; i < lines.Count(); i++ )  {
    if( lines[i].IsEmpty() )  continue;

    olxstr &lref = lines[i];
    // skip comments ...
    bool openquote = false;
    for( int j=0; j < lref.Length(); j++ )  {
      if( lref[j] == '\'' || lref[j] == '\"' )  {
        if( j > 0 && lref[j-1] == '\\' )  // escaped quotation
          continue;
        openquote = !openquote;
      }
      else if( !openquote && lref[j] == '/' && (j+1) < lref.Length() )  {
        if( lref[j+1] == '*' )  { // mulitline comment ...
          // search the end on the same line ...
          int endindex = -1;
          for( int k=j+2; k < lref.Length(); k++ )  {
            if( lref[k] == '*' && (k+1) < lref.Length() && lref[k+1] == '/' )  {
              endindex = k;
              break;
            }
          }
          if( endindex != -1 )  {
            lref.Delete(j, endindex-j+2);
          }
          else  {  // search on other lines then
            lref.SetLength(j+1);
            while( ++i < lines.Count() )  {
              olxstr &cref = lines[i];
              for( int k=0; k < cref.Length(); k++ )  {
                if( cref[k] == '*' && (k+1) < cref.Length() && cref[k+1] == '/' )  {
                  endindex = k;
                  break;
                }
              }
              if( endindex == -1 )  cref.SetLength(0);
              else  {
                cref.Delete(0, endindex+2);
                break;
              }
            }
            if( endindex == -1 )
              throw TFunctionFailedException(__OlxSourceInfo, "could not locate th eend of the comment");
          }
        }
        else if( lref[j+1] == '/' )  {  // single line comment
          lref.SetLength( j );
        }
      }
    }

    Defines[linestr] = (olxstr('"') << i << '"');
//    line = PreprocessLine(lines[i]);
//    if( line.IsEmpty() )  continue;

    if( lines[i][0] != '#' )  {
      WriteOutput( ExpandMacros(lines[i]), output );
      continue;
    }
    if( lines[i].StartsFrom(defstr) ) {
      line = lines[i].SubStringFrom(defstr.Length()+1);
      AnalyseDef( line );
      //output << line;
    }
    else if( lines[i].StartsFrom(undefstr) ) {
      line = lines[i].SubStringFrom(undefstr.Length()+1);
      int di = Defines.IndexOf(line);
      if( di != -1 )  Defines.Delete(di);
      //output.Add("// undefine ") << line;
    }
    else if( lines[i].StartsFrom(incstr) )  {
      line = lines[i];
      ExpandMacros(line);
      line = line.SubStringFrom( incstr.Length()+1 ).Trim(' ');
      if( line.Length() < 3 )  {
        output.Add("// FAILED ON ") << lines[i];
        continue;
        //throw TFunctionFailedException(__OlxSourceInfo, "empty include macro");
      }
      line = line.SubString(1, line.Length()-2);
      fn = CP->LocateFile(line);
      if( !TEFile::Exists(fn) )  {
        output.Add("// FAILED ON Include ") << line;
        output.Add("// >> ") << lines[i];
        continue;
        //throw TFunctionFailedException(__OlxSourceInfo, olxstr("could not locate included file - ") << line);
      }
      TStrList fl;
      fl.LoadFromFile(fn);
      for( int j=0; j < fl.Count(); j++ )
        fl[j] = PreprocessLine(fl[j]);
      fl.CombineLines('\\');
      //output.Add("// #include ") << fn;
      StartFileProcessing( fn );
      Process(fl, output );
      //output.Add("// end #include ") << fn;
    }
    else  {
      ProcessIf(lines, i, output);
    }
  }
}

void CPP::WriteOutput(const olxstr& line, TStrList& output)  {
  if( line.IsEmpty() )  return;
  output << line;
/*
  static CNamespace* CurrentNS = &Model->Global();
  static CType* CurrentObject = NULL;
  //static CFunction* CurrentObject = NULL;
  if( line.StartsFrom(namespacestr) )  {
    olxstr nsname;
    if( line[line.Length()-1] == '{') nsname = line.SubStringFrom(namespacestr.Length()+1, 1);
    else                              nsname = line.SubStringFrom(namespacestr.Length()+1, 0);
    CurrentNS = &Model->GetNamespace(nsname, CurrentNS);
  }
  else if( line.StartsFrom(classstr) )  {
    if( line[line.Length()-1] == ';' )  ;
    else if( line[line.Length()-1] == '{' )  {
      CurrentObject = new CType( line.SubStringFrom(classstr.Length()+1, 1), CurrentNS, NULL);
      CurrentNS->AddType( CurrentObject );
    }
  }
*/  
}


CPP::CPP(CParser* cp)  {
  Factory = new DefEvaluatorFactory(this);
  Parser = new TSyntaxParser(Factory);
  Defines.Add(linestr, EmptyString);
  CP = cp;
}
void CPP::Parse(TStrList& input, TStrList& output)  {
  for( int i=0; i < input.Count(); i++ )
    input[i] = PreprocessLine(input[i]);
  input.CombineLines('\\');
  Process(input, output );
}
CPP::~CPP()  {
  delete Parser;
  delete Factory;
  for( int i=0; i < Macros.Count(); i++ )
    delete Macros.Object(i);
}

bool CPP::ValidateIf(const olxstr& val)  {
  Parser->Parse(val);
  if( Parser->GetRoot() == 0 )
    return IsDefined(val);
  return Parser->Evaluate();
}


TStrList& CParser::Parse(const olxstr& filename)  {
  // model initialisation
  if( Model != NULL )  delete Model;
  Model = new CModel(filename);

  TStrList strings;
  Paths.Add( TEFile::ExtractFilePath(filename) );
  try  {  strings.LoadFromFile(filename);  }
  catch( const TExceptionBase& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  CPP PP(this);
  PP.Define("__BORLANDC__", __BORLANDC__);
  PP.Define("__TEMPLATES__", __TEMPLATES__);
  PP.Define("__WIN32__", __WIN32__);
  PP.Define("_Windows", _Windows);
  PP.Define("__cplusplus", __cplusplus);
#ifdef _M_IX86
  PP.Define("_M_IX86", _M_IX86);
#endif
#ifdef __TURBOC__
  PP.Define("__TURBOC__", __TURBOC__);
#endif
#ifdef _CPPUNWIND
  PP.Define("_CPPUNWIND", _CPPUNWIND);
#endif
#ifdef _DEBUG
  PP.Define("_DEBUG", EmptyString);
#endif
//#ifdef _STLP_NO_EXCEPTION_HEADER
  PP.Define("_STLP_NO_EXCEPTION_HEADER", "1");
  PP.Define("_STLP_NO_EXCEPTION_SPEC", "1");
//#endif
#ifdef _STLP_USE_EXCEPTIONS
  PP.Define("_STLP_USE_EXCEPTIONS", "1");
#endif

  PP.StartFileProcessing(filename);

  try  {  PP.Parse(strings, Result);  }
  catch(const TExceptionBase& exc) {
    //MessageBox( NULL, exc.GetFullMessage().c_str(), EsdlClassName(exc).c_str(), 0 );
  }

//  olxstr test("_STLP_BEGIN_NAMESPACE");
//  LocateName(test, test);
//  PP.ExpandMacros(test);

//  olxstr testa("what() const ;");
//  PreprocessLine(testa);
//  PP.ExpandMacros(test);

//  Result.Pack();


  Result.Add("Current defined values:");
  for( int i=0; i < PP.DefineCount(); i++ )
    Result.Add( PP.DefineName(i) ) << '=' << PP.DefineValue(i);

  CParse(Result);

  return Result;
}
olxstr CParser::LocateFile(const olxstr& fn) const {
  if( TEFile::IsAbsolutePath(fn) )  return fn;
  if( fn.IndexOf("..") != -1 )  {
    for( int i=0; i < Paths.Count(); i++ )  {
      olxstr f( TEFile::AbsolutePathTo(Paths[i], fn) );
      if( TEFile::Exists(f) )  return f;
    }
  }
  else  {
    for( int i=0; i < Paths.Count(); i++ )  {
      olxstr f( Paths[i]);
      f << fn;
      if( TEFile::Exists(f) )  return f;
    }
  }
  return EmptyString;
}

bool CParser::IsNextWord(int &ind, int end, const olxstr& word)  {
  if( ind + word.Length() > end )  return false;

  while( StrCode[ind] == ' ' ) ind++;

  for( int i=0; i < word.Length(); i++ )
    if( StrCode[ind+i] != word[i] )  return false;
  ind += word.Length();
  return true;
}

void CParser::ExtractExpression(int &i, int end, olxstr& exp)  {
  const char oc = exp[0], cc = exp[1];
  exp.SetLength(0);
  while( i < end && StrCode[i] != oc )  i++;
  int ob = 1;
  while( ++i < end )  {
    if( StrCode[i] == oc )      ob ++;
    else if( StrCode[i] == cc ) ob --;
    if( ob == 0 )  {  i++;  break;  }
    exp << StrCode[i];
  }
  return;
}

void CParser::ExtractNext(int &i, int end, olxstr& exp)  {
  while( StrCode[i] == ' ' ) i++;
  for(; i < end; i++ )  {
    if( (StrCode[i] >= 'a' && StrCode[i] <= 'z') ||
        (StrCode[i] >= 'A' && StrCode[i] <= 'Z') ||
        (StrCode[i] >= '0' && StrCode[i] <= '9') || StrCode[i] == '_' )
      exp << StrCode[i];
    else
      break;
  }
  // scroll to the next char
  while( StrCode[i] == ' ' ) i++;
  return;
}

void CParser::_CParse(const olxstr &parent, int start, int end)  {
  olxstr name, type, tmp;
  for( int i=start; i < end; i++ )  {
    if( IsNextWord(i, end, templatestr) )  {
      type = "<>";
      ExtractExpression(i, end, type); // xtract template arguments
      if( IsNextWord(i, end, classstr) || IsNextWord(i, end, structstr))  {
        name.SetLength(0);
        ExtractNext(i, end, name);
        if( type.IsEmpty() )  {  // explicit template type
          tmp = "<>";
          ExtractExpression(i, end, tmp);
          type << tmp;
        }
        if( i < StrCode.Length() && (StrCode[i] == ':' || StrCode[i] == '{') )  {
          int sti = i, ob=1;
          olxstr der;
          if( StrCode[i] == ':' )
            while( ++i < StrCode.Length() && StrCode[i] != '{' )  der << StrCode[i];
          // scrol the class template declaration
          while( ++i < StrCode.Length() )  {
            if( StrCode[i] == '{' )  ob++;
            else if( StrCode[i] == '}' )  ob --;
            if( ob == 0 )  break;
          }
          if( der.Length() != 0 )
            Input->Add( olxstr("def template ") << parent << name << '<' << type << '>' << ':' << der);
          else
            Input->Add( olxstr("def template ") << parent << name << '<' << type << '>');

          if( parent.IsEmpty() )
            _CParse( name << "::", sti, i);
          else
            _CParse( olxstr(parent) << name << "::", sti, i);
        }
      }
      else  {  // function then?
        name.SetLength(0);
        while( i < end && StrCode[i] != '(' )  {
          name << StrCode[i]; i++;
        }
//        ExtractNext(i, end, type);
        tmp = "()";  // extract function args
        ExtractExpression(i, end, tmp);
        //scroll function modifiers, as const
        while(i < StrCode.Length() && !(StrCode[i] == ';' || StrCode[i] == '{') ) i++;

        if( i < StrCode.Length() )  {
          int sti = i;
          olxstr der;
          if( StrCode[i] == '{' )  {  // scrol the function body
            int ob = 1;
            while( ++i < StrCode.Length() )  {
              if( StrCode[i] == '{' )  ob++;
              else if( StrCode[i] == '}' )  ob --;
              if( ob == 0 )  break;
            }
          }
          Input->Add( olxstr("def function ") << parent << name << '(' << tmp << ')');
        }
      }
    }
    else if( IsNextWord(i, end, classstr) )  {
      name.SetLength(0);
      ExtractNext(i, end, name);
      if( i < StrCode.Length() && (StrCode[i] == ':' || StrCode[i] == '{') )  {
        int sti = i, ob=1;
        olxstr der;
        if( StrCode[i] == ':' )
          while( ++i < StrCode.Length() && StrCode[i] != '{' )  der << StrCode[i];
        // scrol the class template declaration
        while( ++i < StrCode.Length() )  {
          if( StrCode[i] == '{' )  ob++;
          else if( StrCode[i] == '}' )  ob --;
          if( ob == 0 )  break;
        }
        if( der.Length() != 0 )
          Input->Add( olxstr("def class ") << parent << name << ':' << der);
        else
          Input->Add( olxstr("def class ") << parent << name);

        if( parent.IsEmpty() )
          _CParse( name << "::", sti, i);
        else
          _CParse( olxstr(parent) << name << "::", sti, i);
      }
    }
    else if( IsNextWord(i, end, structstr) )  {
      name.SetLength(0);
      ExtractNext(i, end, name);
      if( i < StrCode.Length() && (StrCode[i] == ':' || StrCode[i] == '{') )  {
        int sti = i, ob=1;
        olxstr der;
        if( StrCode[i] == ':' )
          while( ++i < StrCode.Length() && StrCode[i] != '{' )  der << StrCode[i];
        // scrol the class template declaration
        while( ++i < StrCode.Length() )  {
          if( StrCode[i] == '{' )  ob++;
          else if( StrCode[i] == '}' )  ob --;
          if( ob == 0 )  break;
        }
        if( der.Length() != 0 )
          Input->Add( olxstr("def struct ") << parent << name << ':' << der);
        else
          Input->Add( olxstr("def struct ") << parent << name);

        if( parent.IsEmpty() )
          _CParse( name << "::", sti, i);
        else
          _CParse( olxstr(parent) << name << "::", sti, i);
      }
    }
    else if( IsNextWord(i, end, namespacestr) )  {
      name.SetLength(0);
      ExtractNext(i, end, name);
      if( i < StrCode.Length() && StrCode[i] == '{' )  {
        int sti = i, ob=1;
        olxstr der;
        // scrol the class template declaration
        while( ++i < StrCode.Length() )  {
          if( StrCode[i] == '{' )  ob++;
          else if( StrCode[i] == '}' )  ob --;
          if( ob == 0 )  break;
        }

        Input->Add( olxstr("def namespace ") << parent << name);

        if( parent.IsEmpty() )
          _CParse( name << "::", sti, i);
        else
          _CParse( olxstr(parent) << name << "::", sti, i);
      }
    }
  }

}

void CParser::CParse(TStrList& code)  {
  StrCode = code.Text(EmptyString);
  Input = &code;
  _CParse(EmptyString, 0, StrCode.Length());
}


#ifdef __BORLANDC__
  #pragma package(smart_init)
#endif

