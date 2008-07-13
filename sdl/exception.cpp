//---------------------------------------------------------------------------
#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "exception.h"
#include "estrlist.h"
//#include "bapp.h"
//#include "log.h"

UseEsdlNamespace()

//TBasicException(const olxstr& location, const TExceptionBase& cause, const olxstr& msg=EmptyString)  {
//    if( msg.IsEmpty() )  Message = "Inherited exception";
//    else                 Message = msg;
//    Location = location;
//    Cause = (TBasicException*)cause.GetException()->Replicate();
//  }
//  /* caution: the expeceted object is an instance from a call to Replicate() !
//    and will be deleted
//  */
//  TBasicException(const olxstr& location, IEObject* cause)  {
//    Message = "Inherited exception";
//    Location = location;
//    Cause = (TBasicException*)cause;
//  }
//
//  TBasicException(const olxstr& location, const olxstr& Msg)  {
//      Message = Msg;
//      Location = location;
//      Cause = NULL;
//  }
//
//TBasicException* TBasicException::GetSource() const  {
//
//  TBasicException* cause = const_cast<TBasicException*>(this);
//  while( cause->GetCause() != NULL )
//    cause = cause->GetCause();
//  return cause;
//}


olxstr TBasicException::GetFullMessage()  const  {
  olxstr rv(EsdlClassName(*this));
  rv << ' ' << Message << " occured at " << Location;
  return rv;
}

