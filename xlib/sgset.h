/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olxs_sg_set_H
#define __olxs_sg_set_H
#include "symmlib.h"
BeginXlibNamespace()

class AxisInfo {
  olxstr axis;
public:
  AxisInfo(const TSpaceGroup& sg, const olxstr _axis = EmptyString())
    : axis(_axis.IsEmpty() ? sg.GetAxis() : _axis)
  {
    if (axis.IsEmpty() && sg.GetBravaisLattice().GetName().Equalsi("Orthorhombic")) {
      axis = "abc";
    }
  }
  bool HasCellChoice() const {
    if (axis.IsEmpty()) {
      return false;
    }
    olxch lc = axis.GetLast();
    return (lc >= '1' && lc <= '3');
  }
  short GetCellChoice() const {
    return HasCellChoice() ? axis.GetLast() - '0' : -1;
  }
  void ChangeCellChoice(short v) {
    if (HasCellChoice()) {
      axis[axis.Length() - 1] = v + '0';
    }
    else {
      axis << (olxch)(v + '0');
    }
  }
  void ChangeMonoclinicAxis(const olxstr& a) {
    if (HasCellChoice()) {
      short cc = GetCellChoice();
      axis = a;
      axis << (olxch)(cc + '0');
    }
    else {
      axis = a;
    }
  }
  bool HasMonoclinicAxis() const {
    if (axis.IsEmpty()) {
      return false;
    }
    return ((axis.Length() == 3 && axis.CharAt(0) == '-' && olxstr::o_islatin(axis.CharAt(1))) ||
      (axis.Length() < 3 && olxstr::o_islatin(axis.CharAt(0))));
  }
  olxstr GetMonoclinicAxis() const {
    return HasMonoclinicAxis()
      ? (HasCellChoice() ? axis.SubStringTo(axis.Length() - 1) : axis)
      : EmptyString();
  }
  const olxstr GetAxis() const { return axis; }
  static olxstr ComposeAxisInfo(const olxstr& mon_axis, short cell_choice = -1) {
    olxstr rv(mon_axis);
    return cell_choice != -1 ? rv << cell_choice : rv;
  }
};
class SGSettings {
  TSpaceGroup& sg;
  olxch ExtractAxis(const olxstr& axis) const {
    return axis.Length() == 2 ? axis.CharAt(1) : axis.CharAt(0);
  }
public:
  SGSettings(TSpaceGroup& _sg) : sg(_sg), axisInfo(_sg) {
    //static const mat3d I_to_P(-0.5, 0.5, 0.5, 0.5, -0.5, -0.5), P_to_I(0, 1, 1, 0, 1, 0);
    //static const mat3d F_to_P(0, 0.5, 0.5, 0, 0.5, 0), P_to_F(-1, 1, 1, -1, 1, 1);
  }
  AxisInfo axisInfo;

  // returns transformation for abc (I), cb-a etc notation
  static olx_object_ptr<mat3d> GetABCTransformation(const olxstr& t) {
    if (t.Length() < 3 || t.Length() > 6) {
      return 0;
    }
    olx_object_ptr<mat3d> rv = new mat3d();
    int sig = 1;
    size_t pos = 0;
    for (size_t i = 0; i < t.Length(); i++) {
      if (t.CharAt(i) == '-') {
        sig = -1;
        continue;
      }
      int r_n = t.CharAt(i) - 'a';
      if (r_n < 0 || r_n > 2) {
        return 0;
      }
      (*rv)[pos++][r_n] = sig;
      sig = 1;
    }
    return pos == 3 ? rv : 0;
  }

  bool GetTrasformation(const AxisInfo& ai, mat3d& m) const {
    mat3d rv;
    if (axisInfo.HasMonoclinicAxis() && ai.HasMonoclinicAxis()) {  // is monoclinic
      rv.I();
      olxstr a_from(axisInfo.GetMonoclinicAxis()),
        a_to(ai.GetMonoclinicAxis());
      olxch a_f = ExtractAxis(a_from), a_t = ExtractAxis(a_to);
      int a_d = a_t - a_f;
      if (a_d < 0) {
        a_d += 3;
      }
      if (a_d < 0 || a_d > 2) {
        return false;
      }
      if (a_d > 0) {
        mat3d a_tm(0, 1, 0, 0, 0, 1, 1, 0, 0);
        for (int i = 0; i < a_d; i++) {
          rv *= a_tm;
        }
      }
      if (a_from.Length() != a_to.Length()) {  // inversion of the axis
        mat3d ai_tm(1.0, 1.0, 1.0);
        int r_n = a_t - 'a';
        if (r_n >= 0 && r_n <= 3) {
          ai_tm[r_n][r_n] = -1;
        }
        else {
          throw TInvalidArgumentException(__OlxSourceInfo, "axis");
        }
        rv *= ai_tm;
      }
      if (axisInfo.HasCellChoice() && ai.HasCellChoice()) {  // change cell choice
        int cc_f = axisInfo.GetCellChoice(), cc_t = ai.GetCellChoice();
        int cc_d = cc_t - cc_f;
        if (cc_d < 0) {
          cc_d += 3;
        }
        if (cc_d < 0 || cc_d > 2) {
          return false;
        }
        if (cc_d > 0) {
          mat3d c_tm;
          if (a_t == 'a') {
            c_tm = mat3d(1, 0, 0, 0, 0, -1, 0, 1, -1);
          }
          else if (a_t == 'b') {
            c_tm = mat3d(-1, 0, 1, 0, 1, 0, -1, 0, 0);
          }
          else if (a_t == 'c') {
            c_tm = mat3d(0, -1, 0, 1, -1, 0, 0, 0, 1);
          }
          for (int i = 0; i < cc_d; i++) {
            rv *= c_tm;
          }
        }
      }
      m = rv;
      return !rv.IsI();
    }
    olx_object_ptr<mat3d> that_m = GetABCTransformation(ai.GetAxis());
    olx_object_ptr<mat3d> this_m = GetABCTransformation(axisInfo.GetAxis());
    if (this_m.ok() && that_m.ok()) {
      m = this_m->GetInverse() * (*that_m);
      return true;
    }
    return false;
  }
};

EndXlibNamespace()
#endif
