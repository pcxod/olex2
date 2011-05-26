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
  if( a.DeleteChars('a') != "bcbc" )
    throw TFunctionFailedException(__OlxSourceInfo, EmptyString());
  if( a.DeleteCharSet("abc") != "" )
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
}
//...................................................................................................
void StringTests(OlxTests& t)  {
  t.Add(&ReplaceTest).Add(&IsNumberTest);
}
};  //namespace test
