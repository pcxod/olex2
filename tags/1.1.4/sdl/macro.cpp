//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "datafile.h"

/*
//..............................................................................
//..............................................................................
//---------------------------------------------------------------------------
// TArgList implementation
//---------------------------------------------------------------------------
TArgList::~TArgList()
{
  Clear();
}
//..............................................................................
void TArgList::Clear()
{
  int i;
  for(i=0; i < Count(); i++ )
    delete (TEString*)Object(i);
  TEStringList::Clear();
}
//..............................................................................
void TArgList::Add(const TEString &S, const TEString &S1)
{
  TEString *ES = new TEString;
  *ES = S1;
  AddObject(S, ES);
}
//..............................................................................
TEString TArgList::Value(const TEString &Param)
{
  int i = IndexOf(Param); // will throw an exception if something wrong
  if( i == -1 )  return "";
  return Value(i);
}
//..............................................................................
// takes a string like: fileopen filter=[All files|*.*] caption=[Select file]
TEString TArgList::LoadFromString(const TEString &Str)
{
  int sl, i;
  sl = Str.Length();
  TEString Cmd, Arg, ArgV;
  for(i=0; i < sl; i++ )
  {
    while( i < sl ) // skip spaces
    {
      if( Str[i] != ' ' )  break;
      i++;
    }
    if( (i+1) >= sl ) break;
    if( !Cmd.Length() )
    {
      while( i < sl )  // extract command name
      {
        if( Str[i] == ' ' )  break;
        Cmd += Str[i];
        i++;
      }
      continue;
    }
    if( Str[i] != ' ' )
    {
      while( i < sl )  // extract argument name
      {
        if( (Str[i] == '=') ){ i++;  break; }
        if( Str[i] != ' ')  Arg += Str[i];
        i++;
      }
      if( (i+1) >= sl ) break;
      while( i < sl ) // skip spaces
      {
        if( Str[i] != ' ' )  break;
        i++;
      }
      if( (i+1) >= sl ) break;
      if( Str[i] == '[' )  // complex string
      {
        i++;
        while( i < sl )  // extract argument name
        {
          if( (Str[i] == ']') )  break;
          ArgV += Str[i];
          i++;
        }
        while( i < sl )  // skip spaces after [
        {
          if( (Str[i] == ',') )  break;
          i++;
        }
      }
      else
      {
        while( i < sl )  // extract argument name
        {
          if( (Str[i] == ',') )  break;
          ArgV += Str[i];
          i++;
        }
      }
      Add(Arg, ArgV);
      ArgV = "";
      Arg = "";
    }
  }
  if( Arg.Length() )  Add(Arg, ArgV); // add last argument (if does not have a qualifier)
  return Cmd;
}
//..............................................................................
//---------------------------------------------------------------------------
// TVAriable implementation
//---------------------------------------------------------------------------
TVariable::TVariable(const TEString &name)
{ 
  FName = name; 
  FFields = new TEList;
  FReferences=0;
}
//..............................................................................
TVariable::~TVariable()
{  
  int i;
  for( i=0; i < FieldCount(); i++ )
  {
    delete Field(i);
  }
}
//..............................................................................
void TVariable::operator = (TVariable *V)
{
  int i;
  TVariable *OField, *NField;
  FValue = V->Value();
  for( i=0; i < V->FieldCount(); i++ )
  {
    NField = V->Field(i);
    OField = Field(NField->Name());
    if( OField )  *OField = *NField;
  }
}
//..............................................................................
TEString TVariable::Value(TEList *Args)  {  return FValue; }
//..............................................................................
void TVariable::Value(const TEString &V)
{ FValue = V; }
//..............................................................................
TVariable *TVariable::Field(const TEString &Name) const
{
  int i;
  for( i=0; i < FieldCount(); i++ )
  {
    if( Field(i)->Name() == Name )  return Field(i);
  }
  return NULL;
}
//..............................................................................
bool TVariable::FieldExists(const TEString &Name) const
{
  if( Field(Name) )  return true;
  return false;
}
//..............................................................................
//---------------------------------------------------------------------------
// TFunctioncall implementation
//---------------------------------------------------------------------------
TFunctionCall::TFunctionCall()
{
  FFunc = NULL;
  FArgs = new TEList;
}
//..............................................................................
TFunctionCall::~TFunctionCall()
{
  int i;
  TVariable *Var;
  for( i=0; i < FArgs->Count(); i++ )
  {
    Var = (TVariable*)FArgs->Item(i);
    if( Var->RefCount() == 1 )
    { delete Var;  }
    else
    {  Var->DecRefs(); }
  }
}
//..............................................................................
void TFunctionCall::AddArg(TVariable *V)
{
  FArgs->Add(V);
  V->IncRefs();
}
//..............................................................................
void TFunctionCall::Call()
{
  if( FFunc )  FFunc->Value(FArgs);
  else
  {  BasicApp->Log->Error("TFunctioncall:: NULL function! "); }
}
*/
