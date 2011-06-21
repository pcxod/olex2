#include "function.h"
#include "egc.h"
UseEsdlNamespace()

olxstr ABasicLibrary::GetQualifiedName() const  {
  olxstr res = GetName();
  ABasicLibrary* lib = this->GetParentLibrary();
  while( lib && lib->GetParentLibrary() )  {
    res.Insert(lib->GetName() + '.', 0 );
    lib = lib->GetParentLibrary();
  }
  return res;
}
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
  ABasicLibrary* lib = this->GetParentLibrary();
  while( lib && lib->GetParentLibrary() )  {
    res.Insert(lib->GetName() + '.', 0 );
    lib = lib->GetParentLibrary();
  }
  return res;
}
//..............................................................................
void ABasicFunction::ParseOptions(const olxstr& Options, TCSTypeList<olxstr,olxstr>& list)  {
  if( Options.IsEmpty() )  return;
  TStrList toks(Options, "&;");
  for( size_t i=0; i < toks.Count(); i++ )  {
    size_t mi = toks[i].IndexOf('-');
    if( mi != InvalidIndex )
      list.Add( toks[i].SubStringTo(mi), olxstr(toks[i].SubStringFrom(mi+1)));
    else
      list.Add( toks[i], EmptyString());
  }
}
//..............................................................................
olxstr ABasicFunction::OptionsToString(const TCSTypeList<olxstr,olxstr>& list) const {
  olxstr rv;
  for( size_t i=0; i < list.Count(); i++ )  {
    rv << list.GetKey(i);
    if( !list.GetObject(i).IsEmpty() )
      rv << '-' << list.GetObject(i);
    rv << "&;";
  }
  return rv;
}
//..............................................................................
