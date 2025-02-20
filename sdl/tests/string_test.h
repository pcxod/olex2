/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

namespace test {

void ReplaceTest(OlxTests& t)  {
  t.description = __FUNC__;
  olxstr a("abcdef");
  if( a.Replace("abc", "cba") != "cbadef" )
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  if( a.Replace('a', "aa") != "cbaadef" )
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  if( a.Replace('a', "aa") != "cbaaaadef" )
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  if( a.Replace("aa", "h") != "cbhhdef" )
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  if( a.Replace("h", EmptyString()) != "cbdef" )
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  a = "abcwwwbcawww";
  if( a.DeleteStrings("www") != "abcbca" )
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  a = "abcawwwbcawww";
  if (a.DeleteStrings("ca") != "abwwwbwww")
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  if( a.DeleteChars('a') != "bwwwbwww" )
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  if( a.DeleteCharSet("bw") != "" )
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  if (olxstr("bw_bw_").Replace("bw", "wbw") != "wbw_wbw_")
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  if (olxstr("bw_bw_bw").Replace("bw", "wbw") != "wbw_wbw_wbw")
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  if (olxstr("_xw_bw_xw").Replace("xw", "w_w") != "_w_w_bw_w_w")
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
}

void IsNumberTest(OlxTests& t)  {
  t.description = __FUNC__;
  olxstr valid_num_str[] = { "0", " 0 ", " 0", "0 ", " 0", " 0", " .0 ", " 0.0 ", " 0.e0 ",
    "  0.e-1  ", "  0xffa  ", "  0xffa", " 0", " -0. ", " +0. ", "+0e-5", "-.e-5", "07777"  };
  olxstr invalid_num_str[] = { EmptyString(), "  0xffx", " 0a", " -.", "0e-a", "08", "..", "0.0.",
    "1e-5e-4" };
  olxstr valid_int_str[] = {"  0xff ", "0xa", "0777", "-3", " +3 " };
  olxstr invalid_int_str[] = {"  0xffx", " 0xa.", " 0778", "-3.", " +3. " };
  olxstr valid_uint_str[] = {"  0xff", " 0xa", " 0777", "3", " +3 " };
  olxstr invalid_uint_str[] = {"-3", " -0" };
  for( size_t i=0; i < sizeof(valid_num_str)/sizeof(valid_num_str[0]); i++ )  {
    if( !valid_num_str[i].IsNumber() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("valid number is not recognised: '") << valid_num_str[i] << '\'');
  }
  for( size_t i=0; i < sizeof(invalid_num_str)/sizeof(invalid_num_str[0]); i++ )  {
    if( invalid_num_str[i].IsNumber() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid number is not recognised: '") << invalid_num_str[i] << '\'');
  }
  for( size_t i=0; i < sizeof(valid_int_str)/sizeof(valid_int_str[0]); i++ )  {
    if( !valid_int_str[i].IsInt() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("valid number is not recognised: '") << valid_int_str[i] << '\'');
  }
  for( size_t i=0; i < sizeof(invalid_int_str)/sizeof(invalid_int_str[0]); i++ )  {
    if( invalid_int_str[i].IsInt() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid number is not recognised: '") << invalid_int_str[i] << '\'');
  }
  for( size_t i=0; i < sizeof(valid_uint_str)/sizeof(valid_uint_str[0]); i++ )  {
    if( !valid_uint_str[i].IsUInt() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("valid number is not recognised: '") << valid_uint_str[i] << '\'');
  }
  for( size_t i=0; i < sizeof(invalid_uint_str)/sizeof(invalid_uint_str[0]); i++ )  {
    if( invalid_int_str[i].IsUInt() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid number is not recognised: '") << invalid_uint_str[i] << '\'');
  }
  olxstr ws = "   ";
  if (!ws.TrimWhiteChars().IsEmpty()) {
    throw TFunctionFailedException(__OlxSourceInfo, "trimming is not working");
  }
  ws = "a   ";
  if (ws.TrimWhiteChars() != "a") {
    throw TFunctionFailedException(__OlxSourceInfo, "trimming is not working");
  }
  ws = " a   ";
  if (ws.TrimWhiteChars() != "a") {
    throw TFunctionFailedException(__OlxSourceInfo, "trimming is not working");
  }
  ws = " a   ";
  if (olxstr(ws).TrimWhiteChars(true,false) != "a   ") {
    throw TFunctionFailedException(__OlxSourceInfo, "trimming is not working");
  }
  ws = " a   ";
  if (olxstr(ws).TrimWhiteChars(false, true) != " a") {
    throw TFunctionFailedException(__OlxSourceInfo, "trimming is not working");
  }
  if (olxstr(ws).TrimL(' ') != "a   ") {
    throw TFunctionFailedException(__OlxSourceInfo, "trimming is not working");
  }
  if (olxstr(ws).TrimR(' ') != " a") {
    throw TFunctionFailedException(__OlxSourceInfo, "trimming is not working");
  }
  if (olxstr(ws).TrimWhiteChars(false, true) != " a") {
    throw TFunctionFailedException(__OlxSourceInfo, "trimming is not working");
  }
  ws = "  a";
  if (ws.TrimWhiteChars() != "a") {
    throw TFunctionFailedException(__OlxSourceInfo, "trimming is not working");
  }
}
//...................................................................................................
void SubstringTest(OlxTests& t)  {
  t.description = __FUNC__;
  olxstr x("abcd>>");
  if (x.IsSubStringAt(">>", 3))
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (x.IsSubStringAt(">>", 5))
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (!x.IsSubStringAt(">>", 4))
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  for (size_t i=0; i < x.Length()-1; i++) {
    if (!x.IsSubStringAt(x.SubStringFrom(i), i))
      throw TFunctionFailedException(__OlxSourceInfo, "assert");
  }
  if (x.SubString(0, 1) != 'a')
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (x.SubString(0, 2) != "ab")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (x.SubStringFrom(2) != "cd>>")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (x.SubStringTo(2) != "ab")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (x.IndexOf(">>") != 4)
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
}
//...................................................................................................
void PrintTest(OlxTests& t_) {
  t_.description = __FUNC__;
  if (olx_print("%%%%") != "%%")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (olx_print("%%%d", 10) != "%10")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (olx_print("%s", "string") != "string")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (olx_print("%ls", L"lstring") != "lstring")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (olx_print("%lf", 10.1) != "10.1")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (olx_print("%llu", (uint64_t)10) != "10")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  olxstr t="abc";
  if (olx_print("%lf %% %w", 10.1, &t) != "10.1 % abc")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
}
//...................................................................................................
void TrimTest(OlxTests& t_) {
  t_.description = __FUNC__;
  if (olxstr("lu").TrimR('u') != "l")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (olxstr("u").TrimR('u') != "")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
}
//...................................................................................................
void StringTests(OlxTests& t)  {
  t.Add(&ReplaceTest).Add(&IsNumberTest)
    .Add(&SubstringTest)
    .Add(&PrintTest)
    .Add(&TrimTest);
}
};  //namespace test
