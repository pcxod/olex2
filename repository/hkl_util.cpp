#include "hkl_util.h"

using namespace hkl_util;

olx_object_ptr<Ref> hkl_util::str2ref(const olxcstr& s) {
  bool ref = s.SubString(0, 4).IsInt() &&
    s.SubString(4, 4).IsInt() &&
    s.SubString(8, 4).IsInt() &&
    s.SubString(12, 8).IsNumber() &&
    s.SubString(20, 8).IsNumber();
  if (ref) {
    int h = s.SubString(0, 4).ToInt(),
      k = s.SubString(4, 4).ToInt(),
      l = s.SubString(8, 4).ToInt();
    if (h == 0 && k == 0 && l == 0) {
      return 0;
    }
    return new Ref(h, k, l, s.SubString(12, 8).ToDouble(), s.SubString(20, 8).ToDouble());
  }
  return 0;
}

TTypeList<Ref>::const_list_type hkl_util::lines2refs(const TCStrList& lines,
  size_t l_len)
{
  TTypeList<Ref> refs;
  if (lines.IsEmpty()) {
    return refs;
  }
  for (size_t i = 0; i < lines.Count(); i++) {
    if (lines[i].Length() == l_len) {
      olx_object_ptr<Ref> r = str2ref(lines[i]);
      if (r.ok()) {
        refs.Add(r.release());
      }
    }
  }
  return refs;
}

olxcstr hkl_util::fingerprint2str(const hkl_util::fingerprint_t& fp, bool use_N) {
  olxcstr_buf txt;
  for (size_t i = 0; i < fp.Count(); i++) {
    if (fp[i].b == 0) {
      continue;
    }
    if (use_N) {
      txt << fp[i].b << '(' << olxcstr::FormatFloat(1, fp[i].a / fp[i].b) << ')';
    }
    else {
      txt << olx_round(fp[i].a*10 / fp[i].b);
    }
  }
  return olxcstr(txt);
}

double hkl_util::corelate(const fingerprint_t& f1, const fingerprint_t& f2) {
  if (f1.Count() != f2.Count()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "fingerprint size");
  }
  double diff = 0;
  for (size_t i = 0; i < f1.Count(); i++) {
    double v1 = f1[i].b == 0 ? 0 : f1[i].a / f1[i].b;
    double v2 = f2[i].b == 0 ? 0 : f2[i].a / f2[i].b;
    diff += olx_abs(v1 - v2) * 2 /(f1[i].b + f2[i].b + 1);
  }
  return diff / f1.Count();
}

