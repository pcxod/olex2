/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_association_H
#define __olx_sdl_association_H

// an association template; association of any complexity can be build from this :)
// but for more flexibility still Association3 is provided
template <class Ac, class Bc>
class AnAssociation2  {
  Ac a;
  Bc b;
public:
  AnAssociation2()  {}
  AnAssociation2(const Ac& _a) : a(_a)  {}
  AnAssociation2(const Ac& _a, const Bc& _b) : a(_a), b(_b)  {}
  AnAssociation2(const AnAssociation2& an) : a(an.GetA()), b(an.GetB())  {}
  AnAssociation2& operator = (const AnAssociation2& an)  {
    SetA(an.GetA());
    SetB(an.GetB());
    return *this;
  }
  Ac& A()  {  return a;  }
  Bc& B()  {  return b;  }
  const Ac& GetA() const  {  return a;  }
  const Bc& GetB() const {  return b;  }
  void SetA(const Ac& a)  {  this->a = a;  }
  void SetB(const Bc& b)  {  this->b = b;  }
};

template <class Ac, class Bc, class Cc>
class AnAssociation3 : public AnAssociation2<Ac,Bc>  {
  Cc c;
  typedef AnAssociation2<Ac,Bc> Parent;
public:
  AnAssociation3()  {}
  AnAssociation3(const Ac& a ) : Parent(a)  {}
  AnAssociation3(const Ac& a, const Bc& b) : Parent(a, b)  {}
  AnAssociation3(const Ac& a, const Bc& b, const Cc& _c) : Parent(a,b), c(_c)  {}
  AnAssociation3(const AnAssociation3& an) : Parent(an), c(an.GetC())  {}
  AnAssociation3& operator = (const AnAssociation3& an)  {
    Parent::operator = (an);
    SetC(an.GetC());
    return *this;
  }
  Cc& C()  {  return c;  }
  const Cc& GetC() const {  return c;  }
  void SetC(const Cc& c)  {  this->c = c;  }
};

template <class Ac, class Bc, class Cc, class Dc>
class AnAssociation4 : public AnAssociation3<Ac,Bc,Cc> {
  Dc d;
  typedef AnAssociation3<Ac,Bc,Cc> Parent;
public:
  AnAssociation4()  {}
  AnAssociation4(const Ac& a) : Parent(a)  {}
  AnAssociation4(const Ac& a, const Bc& b) : Parent(a, b)  {}
  AnAssociation4(const Ac& a, const Bc& b, const Cc& c) : Parent(a, b, c)  {}
  AnAssociation4(const Ac& a, const Bc& b, const Cc& c, const Dc& _d) : Parent(a, b, c), d(_d)  {}
  AnAssociation4(const AnAssociation4& an) : Parent(an), d(an.GetD())  {}
  AnAssociation4& operator = (const AnAssociation4& an)  {
    Parent::operator = (an);
    SetD(an.GetD());
    return *this;
  }
  Dc& D()  {  return d;  }
  const Dc& GetD() const  {  return d;  }
  void SetD(const Dc& d)  {  this->d = d;  }
};

struct Association  {
  template <class Ac, class Bc>
  static AnAssociation2<Ac,Bc> Create(const Ac& a, const Bc& b)  {
    return AnAssociation2<Ac,Bc>(a, b);
  }
  template <class Ac, class Bc>
  static AnAssociation2<Ac,Bc> *New(const Ac& a, const Bc& b)  {
    return new AnAssociation2<Ac,Bc>(a, b);
  }
  template <class Ac, class Bc, class Cc>
  static AnAssociation3<Ac,Bc,Cc> Create(const Ac& a, const Bc& b, const Cc& c)  {
    return AnAssociation3<Ac,Bc,Cc>(a, b, c);
  }
  template <class Ac, class Bc, class Cc>
  static AnAssociation3<Ac,Bc,Cc> *New(const Ac& a, const Bc& b, const Cc& c)  {
    return new AnAssociation3<Ac,Bc,Cc>(a, b, c);
  }
  template <class Ac, class Bc, class Cc, class Dc>
  static AnAssociation4<Ac,Bc,Cc,Dc>
  Create(const Ac& a, const Bc& b, const Cc& c, const Dc& d)  {
    return AnAssociation4<Ac,Bc,Cc,Dc>(a, b, c, d);
  }
  template <class Ac, class Bc, class Cc, class Dc>
  static AnAssociation4<Ac,Bc,Cc,Dc> *
  New(const Ac& a, const Bc& b, const Cc& c, const Dc& d)  {
    return new AnAssociation4<Ac,Bc,Cc,Dc>(a, b, c, d);
  }
};

#endif
