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
struct olx_pair_t {
  Ac a;
  Bc b;
  olx_pair_t() {}
  olx_pair_t(const Ac& a)
    : a(a)
  {}
  olx_pair_t(const Ac& a, const Bc& b)
    : a(a), b(b)
  {}
  olx_pair_t(const olx_pair_t& an)
    : a(an.GetA()), b(an.GetB())
  {}
  template <typename T>
  olx_pair_t(const T& an)
    : a(an.GetA()), b(an.GetB())
  {}
  template <typename T>
  olx_pair_t& operator = (const T& an) {
    a = an.GetA();
    b = an.GetB();
    return *this;
  }
  template <typename T>
  bool operator == (const T &an) const {
    return (a == an.GetA() && b == an.GetB());
  }
  const Ac& GetA() const { return a; }
  const Bc& GetB() const { return b; }
  void SetA(const Ac& a) { this->a = a; }
  void SetB(const Bc& b) { this->b = b; }
};

namespace olx_pair {
  template <class Ac, class Bc>
  olx_pair_t<Ac, Bc> Make(const Ac& a, const Bc& b) {
    return olx_pair_t<Ac, Bc>(a, b);
  }
  template <class Ac, class Bc>
  olx_pair_t<Ac, Bc> *New(const Ac& a, const Bc& b) {
    return new olx_pair_t<Ac, Bc>(a, b);
  }
}

template <class Ac, class Bc, class Cc>
struct AnAssociation3 : public olx_pair_t<Ac, Bc> {
  Cc c;
  typedef olx_pair_t<Ac, Bc> parent_t;
  AnAssociation3() {}
  AnAssociation3(const Ac& a)
    : parent_t(a)
  {}
  AnAssociation3(const Ac& a, const Bc& b)
    : parent_t(a, b)
  {}
  AnAssociation3(const Ac& a, const Bc& b, const Cc& c)
    : parent_t(a, b), c(c)
  {}
  AnAssociation3(const AnAssociation3& an)
    : parent_t(an), c(an.GetC())
  {}
  template <typename T>
  AnAssociation3(const T& an)
    : parent_t(an), c(an.GetC())
  {}
  template <typename T>
  AnAssociation3& operator = (const T& an) {
    parent_t::operator= (an);
    c = an.GetC();
    return *this;
  }
  template <typename T>
  bool operator == (const T &an) const {
    return (parent_t::operator ==(an) && c == an.GetC());
  }
  const Cc& GetC() const { return c; }
  void SetC(const Cc& c) { this->c = c; }
};

template <class Ac, class Bc, class Cc, class Dc>
struct AnAssociation4 : public AnAssociation3<Ac,Bc,Cc> {
  Dc d;
  typedef AnAssociation3<Ac, Bc, Cc> parent_t;
  AnAssociation4() {}
  AnAssociation4(const Ac& a)
    : parent_t(a)
  {}
  AnAssociation4(const Ac& a, const Bc& b)
    : parent_t(a, b)
  {}
  AnAssociation4(const Ac& a, const Bc& b, const Cc& c)
    : parent_t(a, b, c)
  {}
  AnAssociation4(const Ac& a, const Bc& b, const Cc& c, const Dc& _d)
    : parent_t(a, b, c), d(_d)
  {}
  AnAssociation4(const AnAssociation4& an)
    : parent_t(an), d(an.GetD())
  {}
  template <class T>
  AnAssociation4(const T& an)
    : parent_t(an), d(an.GetD())
  {}
  template <class T>
  AnAssociation4& operator = (const T& an) {
    parent_t::operator = (an);
    d = an.GetD();
    return *this;
  }
  template <typename T>
  bool operator == (const T &an) const {
    return (parent_t::operator ==(an) && d == an.GetD());
  }
  const Dc& GetD() const { return d; }
  void SetD(const Dc& d) { this->d = d; }
};

struct Association {
  template <class Ac, class Bc>
  static olx_pair_t<Ac,Bc> Create(const Ac& a, const Bc& b)  {
    return olx_pair_t<Ac,Bc>(a, b);
  }
  template <class Ac, class Bc>
  static olx_pair_t<Ac,Bc> *New(const Ac& a, const Bc& b)  {
    return new olx_pair_t<Ac,Bc>(a, b);
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
