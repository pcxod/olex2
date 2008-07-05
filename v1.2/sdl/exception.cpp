//---------------------------------------------------------------------------
#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "exception.h"
#include "estrlist.h"

UseEsdlNamespace()

TBasicException* TBasicException::GetSource() const  {

  TBasicException* cause = const_cast<TBasicException*>(this);
  while( cause->GetCause() != NULL )
    cause = cause->GetCause();
  return cause;
}


olxstr TBasicException::GetFullMessage()  const  {
  olxstr rv(EsdlClassName(*this));
  rv << ' ' << Message << " occured at " << Location;
  return rv;
}

