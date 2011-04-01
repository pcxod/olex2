#ifndef __olx_sdl_ptr_H
#define __olx_sdl_ptr_H

template <typename ptr> struct olx_ptr  {
  ptr* p;
  int ref_cnt;
  olx_ptr(ptr* _p) : p(_p), ref_cnt(1)  {}
  olx_ptr* inc_ref()  {  ref_cnt++;  return this;  }
};

template <typename ptr> struct olx_object_ptr  {
  olx_ptr<ptr>* p;
  olx_object_ptr(ptr* _p) {  p = new olx_ptr<ptr>(_p);  }
  olx_object_ptr(const olx_object_ptr& _p) : p(_p.p->inc_ref())  {}
  ~olx_object_ptr()  {
    if( --p->ref_cnt <= 0 )  {
      delete p->p;
      delete p;
    }
  }
  olx_object_ptr& operator = (const olx_object_ptr& _p)  {
    if( --p->ref_cnt <= 0 )  {
      delete p->p;
      delete p;
    }
    p = _p.p->inc_ref();
    return *this;
  }
  ptr& operator ()()  {  return *p->p;  }
  operator ptr& ()  {  return *p->p;  }
};

template <typename ptr> struct olx_array_ptr  {
  olx_ptr<ptr>* p;
  olx_array_ptr(ptr* _p) {  p = new olx_ptr<ptr>(_p);  }
  olx_array_ptr(const olx_array_ptr& _p) : p(_p.p->inc_ref())  {}
  ~olx_array_ptr()  {
    if( --p->ref_cnt <= 0 )  {
      delete [] p->p;
      delete p;
    }
  }
  olx_array_ptr& operator = (const olx_array_ptr& _p)  {
    if( --p->ref_cnt <= 0 )  {
      delete [] p->p;
      delete p;
    }
    p = _p.p->inc_ref();
    return *this;
  }
  ptr* operator ()()  {  return p->p;  }
  operator ptr* ()  {  return p->p;  }
};

template <class P> P& olx_get_ref(P* p)  {  return *p;  }
template <class P> const P& olx_get_ref(const P* p)  {  return *p;  }
template <class P> P& olx_get_ref(P& p)  {  return p;  }
template <class P> const P& olx_get_ref(const P& p)  {  return p;  }

template <class P> P* olx_get_ptr(P* p)  {  return p;  }
template <class P> const P* olx_get_ptr(const P* p)  {  return p;  }
template <class P> P* olx_get_ptr(P& p)  {  return &p;  }
template <class P> const P* olx_get_ptr(const P& p)  {  return &p;  }

#endif
