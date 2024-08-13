/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "htmlprep.h"
#include "htmlswitch.h"
#include "olex2app.h"
#include "integration.h"
#include "wxzipfs.h"
#include "eutf8.h"

void THtmlPreprocessor::CheckForSwitches(THtmlSwitch& Sender, bool isZip) {
  TStringToList<olxstr, THtmlSwitch*>& Lst = Sender.GetStrings();
  TStrList Toks;
  using namespace exparse::parser_util;
  static const olxstr Tag = "<!-- #include ",
    Tag1 = "<!-- #includeif ",
    Tag2 = "<!-- #include",
    comment_open = "<!--", comment_close = "-->";
  olex2::AOlex2App* app = 0;
  try { app = &olex2::AOlex2App::GetInstance(); }
  catch (...) {}
  olex2::IOlex2Processor* op = olex2::IOlex2Processor::GetInstance();
  for (size_t i = 0; i < Lst.Count(); i++) {
    Lst[i].Replace("~popup_name~", PopupName);
    olxstr tmp = olxstr(Lst[i]).TrimWhiteChars();
    // skip comments
    if (tmp.StartsFrom(comment_open) && !tmp.StartsFrom(Tag2)) {
      //tag_parse_info tpi = skip_tag(Lst, Tag2, Tag3, i, 0);
      if (tmp.EndsWith(comment_close))  continue;
      bool tag_found = false;
      while (++i < Lst.Count()) {
        if (olxstr(Lst[i]).TrimWhiteChars().EndsWith(comment_close)) {
          tag_found = true;
          break;
        }
      }
      if (tag_found) {
        continue;
      }
      else {
        break;
      }
    }
    // TRANSLATION START
    if (app != 0) {
      Lst[i] = app->TranslateString(Lst[i]);
      size_t bs = Lst[i].IndexOf("$+");
      if (bs != InvalidIndex) {
        TStrList blk;
        blk.Add(Lst[i].SubStringFrom(bs + 2).TrimWhiteChars(true, false));
        Lst[i].Delete(bs, Lst[i].Length() - bs);
        size_t bi = i;
        while (++bi < Lst.Count()) {
          size_t be = Lst[bi].IndexOf("$-");
          if (be != InvalidIndex) {
            blk.Add(Lst[bi].SubStringTo(be).TrimWhiteChars(true, false));
            Lst[bi].Delete(0, be + 2);
            break;
          }
          else {
            blk.Add(Lst[bi].TrimWhiteChars(true, false));
          }
        }
        Lst.DeleteRange(i + 1, blk.Count() - 2);
        Lst.Insert(i + 1, olxstr('$') << blk.Text(EmptyString()), 0);
      }
      if (Lst[i].IndexOf("$") != InvalidIndex && op != 0) {
        op->processFunction(Lst[i],
          olxstr(Sender.GetCurrentFile()) << '#' << (i + 1));
      }
    }
    // TRANSLATION END
    int stm = (Lst[i].StartsFrom(Tag1) ? 1 : 0);
    if (stm == 0) {
      stm = (Lst[i].StartsFrom(Tag) ? 2 : 0);
    }
    if (stm != 0) {
      tmp = Lst[i].SubStringFrom(stm == 1 ? Tag1.Length() : Tag.Length());
      Toks.Clear();
      Toks.Strtok(tmp, ' ', false); // extract item name
      if ((stm == 1 && Toks.Count() < 4) ||
        (stm == 2 && Toks.Count() < 3)) {
        TBasicApp::NewLogEntry(logError) << "Wrong #include[if] syntax: " << tmp;
        continue;
      }
      if (stm == 1) {
        tmp = Toks[0];
        if (op != 0 && op->processFunction(tmp)) {
          if (!tmp.ToBool()) {
            continue;
          }
        }
        else {
          continue;
        }
        Toks.Delete(0);
      }
      THtmlSwitch* Sw = &Sender.NewSwitch();
      Lst.GetObject(i) = Sw;
      Sw->SetName(Toks[0]);
      tmp = Toks.Text(' ', 1, Toks.Count() - 1);
      Toks.Clear();
      TParamList::StrtokParams(tmp, ';', Toks); // extract arguments
      if (Toks.Count() < 2) { // must be at least 2 for filename and status
        TBasicApp::NewLogEntry(logError) <<
          "Wrong defined switch (not enough data)" << Sw->GetName();
        continue;
      }

      for (size_t j = 0; j < Toks.Count() - 1; j++) {
        if (Toks[j].FirstIndexOf('=') == InvalidIndex) {
          if (isZip && !TZipWrapper::IsZipFile(Toks[j])) {
            if (Toks[j].StartsFrom('\\') || Toks[j].StartsFrom('/')) {
              tmp = Toks[j].SubStringFrom(1);
            }
            else {
              tmp = TZipWrapper::ComposeFileName(
                Sender.GetFile(Sender.GetFileIndex()), Toks[j]);
            }
          }
          else {
            tmp = Toks[j];
          }
          if (op != 0) {
            op->processFunction(tmp);
          }
          Sw->AddFile(tmp);
        }
        else {
          // check for parameters
          if (Toks[j].IndexOf('#') != InvalidIndex) {
            tmp = Toks[j];
            for (size_t k = 0; k < Sender.GetParams().Count(); k++) {
              olxstr tmp1 = olxstr().Allocate(
                Sender.GetParams().GetName(k).Length() + 2) <<
                '#' << Sender.GetParams().GetName(k);
              tmp.Replace(tmp1, Sender.GetParams().GetValue(k));
            }
            Sw->AddParam(tmp);
          }
          else {
            Sw->AddParam(Toks[j]);
          }
        }
      }

      size_t switchState = SwitchStates.Find(Sw->GetName(), size_t(-2)),
        index = InvalidIndex;
      if (switchState == UnknownSwitchState) {
        index_t iv = Toks.GetLastString().RadInt<index_t>();
        if (iv < 0) {
          Sw->SetUpdateSwitch(false);
        }
        index = olx_abs(iv) - 1;
      }
      else {
        index = switchState;
      }
      Sw->SetFileIndex(index);
    }
  }
}
//.............................................................................
void THtmlPreprocessor::SetSwitchState(THtmlSwitch& sw, size_t state) {
  size_t ind = SwitchStates.IndexOf(sw.GetName());
  if (ind == InvalidIndex) {
    SwitchStates.Add(sw.GetName(), state);
  }
  else {
    SwitchStates.GetValue(ind) = state;
  }
}
//.............................................................................
void THtmlPreprocessor::UpdateSwitchState(THtmlSwitch& Switch, olxstr& String) {
  if (!Switch.IsToUpdateSwitch()) {
    return;
  }
  olxstr Tmp = "<!-- #include ";
  Tmp << Switch.GetName() << ' ';
  for (size_t i = 0; i < Switch.FileCount(); i++) {
    Tmp << Switch.GetFile(i) << ';';
  }
  for (size_t i = 0; i < Switch.GetParams().Count(); i++) {
    Tmp << Switch.GetParams().GetName(i) << '=';
    if (Switch.GetParams().GetValue(i).FirstIndexOf(' ') == InvalidIndex) {
      Tmp << Switch.GetParams().GetValue(i);
    }
    else {
      Tmp << '\'' << Switch.GetParams().GetValue(i) << '\'';
    }
    Tmp << ';';
  }

  Tmp << Switch.GetFileIndex() + 1 << ';' << " -->";
  String = Tmp;
}
//.............................................................................
olxstr THtmlPreprocessor::Preprocess(const olxstr& html) {
  try {
    olxcstr cstr = TUtf8::Encode(html);
    olxstr tmp_file_name = "#html_preprocessor!";
    TFileHandlerManager::AddMemoryBlock(tmp_file_name, cstr.c_str(), cstr.Length(), 0);
    THtmlSwitch root(this, 0);
    root.AddFile(tmp_file_name);
    root.SetFileIndex(0);
    root.UpdateFileIndex();
    TStrList res;
    root.ToStrings(res, false);
    TFileHandlerManager::Remove(tmp_file_name);
    return res.Text('\n');
  }
  catch (const TExceptionBase& e) {
    return e.GetException()->GetFullMessage();
  }
}
//.............................................................................
