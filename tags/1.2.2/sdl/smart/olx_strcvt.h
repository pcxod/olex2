/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

// no namespace or headers, to be included in
// generic
template <typename T> struct olx_str_to_num {
  template <class ST, typename TC>
  T operator ()(const TTSString<ST,TC> &s) const {  return (T)s.ToDouble();  }
};
// int8_t
template <> struct olx_str_to_num<int8_t> {
  template <class ST, typename TC>
  int8_t operator ()(const TTSString<ST,TC> &s) const {  return s.ToInt();  }
};
template <> struct olx_str_to_num<uint8_t> {
  template <class ST, typename TC>
  uint8_t operator ()(const TTSString<ST,TC> &s) const {  return s.ToUInt();  }
};
// int16_t
template <> struct olx_str_to_num<int16_t> {
  template <class ST, typename TC>
  int16_t operator ()(const TTSString<ST,TC> &s) const {  return s.ToInt();  }
};
template <> struct olx_str_to_num<uint16_t> {
  template <class ST, typename TC>
  uint16_t operator ()(const TTSString<ST,TC> &s) const {  return s.ToUInt();  }
};
// int32_t
template <> struct olx_str_to_num<int32_t> {
  template <class ST, typename TC>
  int32_t operator ()(const TTSString<ST,TC> &s) const {  return s.ToInt();  }
};
template <> struct olx_str_to_num<uint32_t> {
  template <class ST, typename TC>
  uint32_t operator ()(const TTSString<ST,TC> &s) const {  return s.ToUInt();  }
};
// int64_t
template <> struct olx_str_to_num<int64_t> {
  template <class ST, typename TC>
  int64_t operator ()(const TTSString<ST,TC> &s) const {  return s.ToInt();  }
};
template <> struct olx_str_to_num<uint64_t> {
  template <class ST, typename TC>
  uint64_t operator ()(const TTSString<ST,TC> &s) const {  return s.ToUInt();  }
};
// float
template <> struct olx_str_to_num<float> {
  template <class ST, typename TC>
  float operator ()(const TTSString<ST,TC> &s) const {  return s.ToFloat();  }
};
template <> struct olx_str_to_num<double> {
  template <class ST, typename TC>
  double operator ()(const TTSString<ST,TC> &s) const {  return s.ToDouble();  }
};
