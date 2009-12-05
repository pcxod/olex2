#ifndef __olx_expvalue_H
#define __olx_expvalue_H
#include "../ebase.h"

BeginEsdlNamespace()

namespace exparse  {
  struct cast_result  {
    const void* value;
    bool temporary;  // the value must be deleted
    cast_result(const cast_result& cr) : value(cr.value), temporary(cr.temporary)  {}
    cast_result(const void* val, bool tmp) : value(val), temporary(tmp) {} 
  };

  template <class T, class OT> struct val_wrapper  {
    T* val;
    mutable OT* origin;
    mutable bool do_delete;
    val_wrapper(const val_wrapper& v) : val(v.val), do_delete(v.do_delete), origin(v.origin)  {
      v.origin = NULL;
      v.do_delete = false;
    }  
    val_wrapper(const val_wrapper& v, OT* _origin) : val(v.val),
      do_delete(v.do_delete), origin(_origin)
    {
      origin->inc_ref();
      v.do_delete = false;
    }  
    val_wrapper(const cast_result& cr) : val((T*)cr.value), do_delete(cr.temporary), origin(NULL)  {}
    ~val_wrapper()  {  
      if( do_delete )
        delete val;  
      if( origin != NULL && origin->dec_ref() == 0 )
        delete origin;
    }
    operator const T& ()  {  return *val;  }
  };

  template <class T, class OT> struct val_wrapper<const T&,OT>  {
    T* val;
    mutable OT* origin;
    mutable bool do_delete;
    val_wrapper(const val_wrapper& v) : val(v.val), do_delete(v.do_delete), origin(v.origin)  {
      v.origin = NULL;
      v.do_delete = false;
    }  
    val_wrapper(const val_wrapper& v, OT* _origin) : val(v.val),
      do_delete(v.do_delete), origin(_origin)
    {
      origin->inc_ref();
      v.do_delete = false;
    }  
    val_wrapper(const cast_result& cr) : val((T*)cr.value), do_delete(cr.temporary), origin(NULL)  {}
    ~val_wrapper()  {  
      if( do_delete )
        delete val;  
      if( origin != NULL && origin->dec_ref() == 0 )
        delete origin;
    }
    operator const T& ()  {  return *val;  }
  };
};  // end namespace exparse

EndEsdlNamespace()

#endif

