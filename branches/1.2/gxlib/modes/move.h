/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_MOVE_MODE_H
#define __OLX_MOVE_MODE_H

class TMoveMode : public AMode  {
  bool Copy;
  vec3d Center;
protected:
public:
  TMoveMode(size_t id) : AMode(id)  {}
  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    Copy = Options.Contains('c');
    TXAtomPList Atoms = gxapp.FindXAtoms(Cmds.Text(' '), true);
    for( size_t i=0; i < Atoms.Count(); i++ )
      Center += Atoms[i]->ccrd();
    if( Atoms.Count() != 0 )
      Center /= Atoms.Count();
    olex2.processMacro("cursor(hand)");
    return true;
  }
  void Finalise_()  {}
  virtual bool OnObject_(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      gxapp.MoveFragment(Center, &(TXAtom&)obj, Copy);
      return true;
    }
    return false;
  }
};

#endif
