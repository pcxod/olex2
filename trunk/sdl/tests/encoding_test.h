/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "md5.h"
#include "sha.h"
#include "../encodings.h"
#include <locale>
namespace test {
  void MD5Test(OlxTests& t)  {
    t.description = __FUNC__;
    olxcstr msg("The quick brown fox jumps over the lazy dog"),
      res("9e107d9d372bb6826bd81d3542a419d6"),
      res1("e4d909c290d0fb1ca068ffaddf22cbd0"),
      res3("d41d8cd98f00b204e9800998ecf8427e");

    if (!MD5::Digest(CEmptyString()).Equalsi(res3))
      throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
    if (!MD5::Digest(msg).Equalsi(res))
      throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
    if (!MD5::Digest(msg << '.').Equalsi(res1))
      throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  }
  //...................................................................................................
  void SHA1Test(OlxTests& t)  {
    t.description = __FUNC__;
    olxcstr msg("The quick brown fox jumps over the lazy dog"),
      res("2fd4e1c6 7a2d28fc ed849ee1 bb76e739 1b93eb12"),
      res1("da39a3ee 5e6b4b0d 3255bfef 95601890 afd80709");

    if (!SHA1::Digest(CEmptyString()).Equalsi(res1))
      throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
    if (!SHA1::Digest(msg).Equalsi(res))
      throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  }
  //...................................................................................................
  void SHA2Test(OlxTests& t) {
    t.description = __FUNC__;
    olxcstr msg("The quick brown fox jumps over the lazy dog"),
      res256_0("d7a8fbb3 07d78094 69ca9abc b0082e4f 8d5651e4 6d3cdb76 2d02d0bf 37c9e592"),
      res256_1("e3b0c442 98fc1c14 9afbf4c8 996fb924 27ae41e4 649b934c a495991b 7852b855"),
      res224_0("730e109b d7a8a32b 1cb9d9a0 9aa2325d 2430587d dbc0c38b ad911525"),
      res224_1("d14a028c 2a3a2bc9 476102bb 288234c4 15a2b01f 828ea62a c5b3e42f");

    if (!SHA256::Digest(CEmptyString()).Equalsi(res256_1))
      throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
    if (!SHA256::Digest(msg).Equalsi(res256_0))
      throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
    if (!SHA224::Digest(CEmptyString()).Equalsi(res224_1))
      throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
    if (!SHA224::Digest(msg).Equalsi(res224_0))
      throw TFunctionFailedException(__OlxSourceInfo, "Wrong digest message");
  }
  //...................................................................................................
  void HashingTests(OlxTests& t) {
    t.Add(&test::MD5Test)
      .Add(&test::SHA1Test)
      .Add(&test::SHA2Test);
  }
  //...................................................................................................
  //...................................................................................................
  //...................................................................................................
  void BaseEncodingTest(OlxTests& t) {
    t.description = __FUNC__;
    olxcstr msg("The quick brown fox jumps over the lazy dog");
    {
      olxcstr res = encoding::base16::encode(msg);
      if (encoding::base16::decode(res) != msg) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "Non reversble encoding");
      }
    }
    {
      olxcstr res = encoding::base64::encode(msg);
      if (encoding::base64::decode(res) != msg) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "Non reversble encoding");
      }
    }
  }
  //...................................................................................................
  void PercentEncodingTest(OlxTests& t) {
    t.description = __FUNC__;
    olxstr msgs[] = { L"The quick brown fox",
      L"%% %%% % \u2045" };
    for (int i = 0; i < 2; i++) {
      olxcstr eres = encoding::percent::encode(msgs[i]);
      olxstr dres = encoding::percent::decode(eres);
      if (dres != msgs[i]) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "Non reversble encoding");
      }
    }
  }
  //...................................................................................................
  void Base85Test(OlxTests& t) {
    t.description = __FUNC__;
    olxstr msgs[] = { L"The quick brown fox",
      L"%% %%% % \u2045" };
    uint8_t test_data_1[8] = {0x86, 0x4F, 0xD2, 0x6F, 0xB5, 0x59, 0xF7, 0x5B};
    olxcstr x1 = encoding::base85::decode("HelloWorld");
    if (x1.Length() != 8) {
      throw TFunctionFailedException(__OlxSourceInfo, "encoding failed");
    }
    for (int i = 0; i < 8; i++) {
      if (test_data_1[i] != (uint8_t)x1.CharAt(i)) {
        throw TFunctionFailedException(__OlxSourceInfo, "encoding failed");
      }
    }
    olxcstr x = encoding::base85::encode(&test_data_1[0], 8);
    if (x != "HelloWorld") {
      throw TFunctionFailedException(__OlxSourceInfo, "encoding failed");
    }
    for (int i = 0; i < 2; i++) {
      olxcstr toe = TUtf8::Encode(msgs[i]);
      olxcstr eres = encoding::base85::encode(toe);
      olxcstr tod = encoding::base85::decode(eres);
      olxstr dres = TUtf8::Decode(tod);
      if (dres != msgs[i]) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "Non reversble encoding");
      }
    }
  }
  //...................................................................................................
  void MBStest(OlxTests &t) {
    t.description = __FUNC__;
    olxstr cm[] = { L"The quick brown fox",
      L"\u00e9 \u00df \u00de "
    },
      fm[] = {L"%% \u2045",  L"\u06de \u20a9"};
    for (int i = 0; i < 2; i++) {
      olxcstr e = cm[i].ToMBStr();
      olxstr d = e.ToWCStr();
      if (d != cm[i]) {
        throw TFunctionFailedException(__OlxSourceInfo, "MBS-WStr conversion failed");
      }
    }
    for (int i = 0; i < 2; i++) {
      try {
        olxcstr e = fm[i].ToMBStr();
        olxstr d = e.ToWCStr();
        throw TFunctionFailedException(__OlxSourceInfo, "MBS-WStr did not fail but should have");
      }
      catch (const TExceptionBase &e) {
      }
    }
    for (int i = 0; i < 2; i++) {
      olxcstr e = fm[i].ToUTF8(), e1;
      e1 = e;
      if (!e.IsUTF8() || !e.SubString(0, 1).IsUTF8() || !olxcstr(e).IsUTF8() || !e1.IsUTF8()) {
        throw TFunctionFailedException(__OlxSourceInfo, "UTF8 flag is not set or not copied over");
      }
      olxstr d = e.ToWCStr();
      if (d != fm[i] || d != TUtf8::Decode(e)) {
        throw TFunctionFailedException(__OlxSourceInfo, "UTF8 conversion failed");
      }
    }
  }
  //...................................................................................................
  void EncodingTests(OlxTests& t) {
    t.Add(&test::BaseEncodingTest)
      .Add(&test::PercentEncodingTest)
      .Add(&test::Base85Test).
      Add(&test::MBStest);
  }
  //...................................................................................................
};  //namespace test
