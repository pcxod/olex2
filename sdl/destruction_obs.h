/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#ifndef __olx_sdl_desobs_H
#define __olx_sdl_desobs_H

struct StaticDestructionObserver : public ADestructionObserver {
  void(*des_obs)(APerishable* obj);
  StaticDestructionObserver(void(*des_obs)(APerishable* obj))
    : des_obs(des_obs)
  {}
  virtual void call(APerishable* obj) const { (*des_obs)(obj); }
  virtual bool operator == (const ADestructionObserver *p_) const {
    const StaticDestructionObserver *p =
      dynamic_cast<const StaticDestructionObserver *>(p_);
    return (p && p->des_obs == des_obs);
  }
  ADestructionObserver *clone() const {
    return new StaticDestructionObserver(des_obs);
  }
};
template <class base>
struct MemberDestructionObserver : public ADestructionObserver {
  void (base::*des_obs)(APerishable* obj);
  olx_vptr<base> instance;
  MemberDestructionObserver(const olx_vptr<base> &inst,
    void (base::*des_obs)(APerishable* obj))
    : instance(inst),
      des_obs(des_obs)
  {}
  MemberDestructionObserver(base *inst,
    void (base::*des_obs)(APerishable* obj))
    : instance(inst),
    des_obs(des_obs)
  {}
  virtual void call(APerishable* obj) const {
    ((*(const_cast<MemberDestructionObserver *>(this)->instance)).*des_obs)(obj);
  }
  virtual bool operator == (const ADestructionObserver *p_) const {
    const MemberDestructionObserver *p =
      dynamic_cast<const MemberDestructionObserver *>(p_);
    return (p && instance == p->instance &&
      p->des_obs == des_obs);
  }
  ADestructionObserver *clone() const {
    return new MemberDestructionObserver(instance, des_obs);
  }
};

struct DestructionObserver {
  template <class T> static MemberDestructionObserver<T> Make(T* inst,
    void (T::*f)(APerishable* obj))
  {
    return MemberDestructionObserver<T>(inst, f);
  }

  static StaticDestructionObserver Make(void(*f)(APerishable *)) {
    return StaticDestructionObserver(f);
  }
  template <class T> static MemberDestructionObserver<T> &MakeNew(T* inst,
    void (T::*f)(APerishable* obj))
  {
    return *(new MemberDestructionObserver<T>(inst, f));
  }

  static StaticDestructionObserver &MakeNew(void(*f)(APerishable *)) {
    return *(new StaticDestructionObserver(f));
  }
};

#endif
