#ifndef __olx_sdl_listCast_H
#define __olx_sdl_listCast_H

struct ListCaster  {
  template <class To> struct SimpleCast  {
    template <class From> static inline To OnItem(From o)  {  return (To)o;  }
  };
  struct CopyCast  {
    template <class From, class To> static inline To& OnItem(From o)  {  return *(new To(o));  }
  };
  template <class To> struct AssignCast  {
    template <class From> static inline To OnItem(From o)  {  return *(new To) = o;  }
  };
  template <typename To, class accessor> struct AccessorCast  {
    template <class From> static inline To OnItem(From o)  {  return accessor::Access(o);  }
  };

  template <typename ListA, typename ListB, class Caster>
  static void Cast(const ListA& from, ListB& to, const Caster& caster)  {
    to.SetCapacity(to.Count()+from.Count());
    for( size_t i=0; i < from.Count(); i++ )
      to.Add(caster.OnItem(from[i]));
  }
};

#endif
