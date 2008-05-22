#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "function.h"
#include "egc.h"

UseEsdlNamespace()

//..............................................................................
olxstr ABasicFunction::GetSignature() const {
  olxstr res(GetName());
  res << " arguments [";
  unsigned int ArgC = GetArgStateMask();
  short argc = 0, currentArg = 0;
  for( short i=0; i < 16; i++ )  if( ArgC & (1 << i) )  argc++;
  if( argc  > 5 )  {
    if( argc == 16 )  res << "any";
    else  {
      argc = 0;
      for( short i=0; i < 16; i++ )  if( !(ArgC & (1 << i)) )  argc++;
      res << "any except ";
     for( short i=0; i < 16; i++ )  {
        if( !(ArgC & (0x001 << i)) )  {
          if( !i )  res << "none";
          else      res << i;
          if( currentArg < (argc - 2) )    res << ", ";
          if( ++currentArg == (argc - 1) ) res << " or ";
        }
      }
    }
  }
  else  {
    for( short i=0; i < 16; i++ )  {
      if( ArgC & (0x001 << i) )  {
        if( !i )  res << "none";
        else      res << (i);
        if( currentArg < (argc - 2) )    res << ", ";
        if( ++currentArg == (argc - 1) ) res << " or ";
      }
    }
  }
  if( (ArgC & 0xffff0000) )  {
    res << "] states - ";
    if( this->GetParentLibrary()->GetOwner() )
      res << this->GetParentLibrary()->GetOwner()->GetStateName( ArgC );
    else
      res << "illegal states, program might crash if this function is called";
  }
  else
    res << ']';
  return res;
}
//..............................................................................
olxstr ABasicFunction::GetQualifiedName() const  {
  olxstr res = GetName();
  IBasicLibrary* lib = this->GetParentLibrary();
  while( lib && lib->GetParentLibrary() )
  {
    res.Insert(lib->GetName() + '.', 0 );
    lib = lib->GetParentLibrary();
  }
  return res;
}
//..............................................................................
void ABasicFunction::ParseOptions(const olxstr& Options, TCSTypeList<olxstr,olxstr>& list)  {
  if( Options.IsEmpty() )  return;
  TStrList toks(Options, "&;");
  for( int i=0; i < toks.Count(); i++ )  {
    int mi = toks.String(i).IndexOf('-');
    if( mi != -1 )
      list.Add( toks.String(i).SubStringTo(mi), olxstr(toks.String(i).SubStringFrom(mi+1)));
    else
      list.Add( toks.String(i), EmptyString);
  }
}
//..............................................................................
olxstr ABasicFunction::OptionsToString(const TCSTypeList<olxstr,olxstr>& list) const {
  olxstr rv;
  for( int i=0; i < list.Count(); i++ )  {
    rv << list.GetComparable(i);
    if( !list.GetObject(i).IsEmpty() )
      rv << '-' << list.GetObject(i);
    rv << "&;";
  }
  return rv;
}
//..............................................................................

