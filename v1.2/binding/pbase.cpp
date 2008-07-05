#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "pbase.h"

BObject::~BObject()  {
  for(int i=0; i < Functions.Count(); i++ )  {
    if( Functions.Object(i)->DecRef() == 0 )
      delete Functions.Object(i);
  }
  if( Template != NULL && Template->DecRef() == 0 )
    delete Template;
}

void BObject::AddFunction(BFunction* func)  {
  Functions.Add(func->GetName(), func);
  func->IncRef();
}

void BObject::WriteDefinition(TStrList& out) const {
  TEString line("class ");
  out.Add( line << GetType() << ' ' << '{' );
  for(int i=0; i < Functions.Count(); i++ )  {
    line = "  ";
    out.Add(  line << Functions.GetObject(i)->ToCHString() );
  }
  out.Add("};");
}

BFunction::BFunction(BObject* retVal, const TEString& name, BTemplate* templ, short modifiers) : Name(name)  {
  Modifiers = modifiers;
  Template = templ;
  if( Template != NULL )  Template->IncRef();
  RetVal = retVal;
  RetVal->IncRef();
}

BObjects::BObjects()  {
  VoidType = NULL;
}
BObjects::~BObjects()  {
  for( int i=0; i < Objects.Count(); i++ )
    if( Objects.Object(i)->DecRef() == 0 )
      delete Objects.Object(i);
  for( int i=0; i < Functions.Count(); i++ )
    if( Functions.Object(i)->DecRef() == 0 )
      delete Functions.Object(i);
}
void BObjects::PyBind(BObject* obj, TStrList& out)  {
  static const TEString ident = "  ", ident2 = "    ";
  TStrList methods;
  if( VoidType == NULL )
    VoidType = FindObject<void>();
  if( VoidType == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate built in type");
  TEString line("class ");
  out.Add( line << "py_" << obj->GetType() << ' ' << '{' );
  line = ident;
  line << obj->GetType() << "* Instance;";
  out.Add( line );
  out.Add( "public:" );
  //create constructor
  line = ident;
  line << "py_" << obj->GetType() << '(' << obj->GetType() << "* inst)  {  Instance = inst;  }";
  out.Add( line );
  for(int i=0; i < obj->FunctionCount(); i++ )  {
    line = ident;
    line << obj->Function(i).Declare();
    out.Add( line );
    line = ident2;
    if( &obj->Function(i).GetRetVal() != VoidType )
      line << "return ";
    line << "Instance->" << obj->Function(i).CallStr() << ';';
    out.Add( line );
    out.Add( ident + '}' );

  }
  out.Add("};");
}

#ifdef __BORLANDC__
  #pragma package(smart_init)
#endif
