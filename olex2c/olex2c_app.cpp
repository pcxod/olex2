/******************************************************************************
* Copyright (c) 2004-2019 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* OlexSys Proprietary file                                                    *
******************************************************************************/
#include "olex2c.h"
#include <iostream>
#if !defined(__WIN32__) && defined(__WXWIDGETS__)
#include "wx/wx.h"
#include "wx/app.h"
#endif
#ifndef __WIN32__
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif


bool OnCommand(const olxstr &cmd) {
  if (cmd.Equalsi("quit")) {
    return false;
  }
  else {
    try { TOlex2c::GetInstance().processMacro(cmd); }
    catch (TExceptionBase& exc)  {
      TBasicApp::NewLogEntry(logException) << exc.GetException()->GetError();
    }
  }
  if (TOlex2c::GetInstance().TerminateSignal) {
    return false;
  }
#ifdef __WIN32__
  std::cout << ">>";
#else
  //std::cout << ">>";
#endif
  return true;
}

#if !defined(__WIN32__)  // dummy stuff for wxWidgets...
class MyApp : public wxAppConsole {
  virtual bool OnInit() {
    //Bind(wxEVT_CHAR, &MyApp::OnChar, this);
    return true;
  }
  virtual int OnRun() { 
    return 0;
  }
};
IMPLEMENT_APP_NO_MAIN(MyApp)
#else
struct Console {
  TStrList commands;
  olxstr cmd;
  HANDLE in, out;
  CONSOLE_SCREEN_BUFFER_INFO si;
  size_t ip; // string insertion position
  size_t cp; // command position
  Console() : ip(0), cp(0) {
    in = GetStdHandle(STD_INPUT_HANDLE);
    out = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(out, &si);
  }
  void replace_cmd(const olxstr &ncmd) {
    DWORD written;
    SetConsoleCursorPosition(out, si.dwCursorPosition);
    WriteConsole(out, olxstr::CharStr(' ', cmd.Length()).u_str(),
      cmd.Length(), &written, NULL);
    cmd = ncmd;
    ip = cmd.Length();
  }
  olxstr expand_command(const olxstr &Cmd, bool inc_files) {
    if (Cmd.IsEmpty())  return Cmd;
    olxstr FullCmd(Cmd.ToLowerCase());
    TStrList all_cmds;
    if (inc_files) {
      TStrList names;
      olxstr path = TEFile::ExpandRelativePath(Cmd, TEFile::CurrentDir());
      if (!path.IsEmpty() && TEFile::IsAbsolutePath(path)) {
        size_t lsi = path.LastIndexOf(TEFile::GetPathDelimeter());
        if (lsi != InvalidIndex) {
          olxstr dir_name = path.SubStringTo(lsi + 1);
          if (TEFile::Exists(dir_name)) {
            TEFile::ListDir(dir_name, names, olxstr(path.SubStringFrom(lsi + 1)) <<
              '*', sefReadWrite);
            for (size_t i = 0; i < names.Count(); i++) {
              all_cmds.Add(dir_name + names[i]);
            }
          }
        }
      }
      else {
        TEFile::ListCurrentDir(all_cmds, olxstr(Cmd) << '*', sefReadWrite);
      }
    }
    TBasicLibraryPList libs;
    TLibrary &lib = TOlex2c::GetInstance().GetLibrary();
    lib.FindSimilarLibraries(Cmd, libs);
    TBasicFunctionPList bins;  // builins
    lib.FindSimilarMacros(Cmd, bins);
    lib.FindSimilarFunctions(Cmd, bins);
    for (size_t i = 0; i < bins.Count(); i++) {
      all_cmds.Add(bins[i]->GetQualifiedName());
    }
    for (size_t i = 0; i < libs.Count(); i++) {
      all_cmds.Add(libs[i]->GetQualifiedName());
    }
    if (all_cmds.Count() > 1) {
      olxstr cmn_str = all_cmds[0].ToLowerCase();
      olxstr line(all_cmds[0], 80);
      for (size_t i = 1; i < all_cmds.Count(); i++)  {
        cmn_str = all_cmds[i].ToLowerCase().CommonString(cmn_str);
        if (line.Length() + all_cmds[i].Length() > 79) {
        }
        else {
          line << ' ' << all_cmds[i];
        }
      }
      FullCmd = cmn_str;
      if (!line.IsEmpty())  {
      }
    }
    else if (all_cmds.Count() == 1) {
      return all_cmds[0];
    }
    return FullCmd;
  }
  bool process(int state, int key, olxch ch, size_t cnt) {
    bool clear_last = false;
    switch (key) {
    case VK_TAB: {
      TStrList toks(cmd, ' ');
      if (!toks.IsEmpty()) {
        olxstr c = expand_command(toks.GetLastString(), true);
        if (c != toks.GetLastString()) {
          toks[toks.Count() - 1] = c;
          replace_cmd(toks.Text(' '));
        }
      }
      break;
    }
    case VK_ESCAPE: {
      replace_cmd(EmptyString());
      break;
    }
    case VK_LEFT: {
      if (ip > 0) {
        ip--;
      }
      break;
    }
    case VK_RIGHT: {
      if (ip < cmd.Length()) {
        ip++;
      }
      break;
    }
    case VK_DOWN: {
      if (cp < commands.Count()) {
        replace_cmd(commands[cp++]);
      }
      break;
    }
    case VK_UP: {
      if (cp > 0) {
        replace_cmd(commands[--cp]);
      }
      break;
    }
    case VK_HOME: {
      ip = 0;
      break;
    }
    case VK_END: {
      ip = cmd.Length();
      break;
    }
    case VK_BACK: {
      if (ip > 0) {
        cmd.Delete(--ip, 1);
        clear_last = true;
      }
      break;
    }
    case VK_RETURN: {
      if (commands.IsEmpty() || commands.GetLastString() != cmd) {
        commands << cmd;
        cp = commands.Count();
      }
      CONSOLE_SCREEN_BUFFER_INFO csi;
      GetConsoleScreenBufferInfo(out, &csi);
      csi.dwCursorPosition.X = 0;
      csi.dwCursorPosition.Y++;
      SetConsoleCursorPosition(out, csi.dwCursorPosition);
      if (!OnCommand(cmd)) {
        return false;
      }
      reset();
      break;
    }
    case VK_DELETE: {
      if (ip + 1 <= cmd.Length()) {
        cmd.Delete(ip, 1);
        clear_last = true;
      }
      break;
    }
    default: {
      if ((state&RIGHT_CTRL_PRESSED) || (state&LEFT_CTRL_PRESSED)) {
        if (key == 'v' || key == 'V') {
          if (OpenClipboard(NULL)) {
            olxstr t;
            HANDLE h = GetClipboardData(CF_UNICODETEXT);
            if (h == NULL) {
              h = GetClipboardData(CF_TEXT);
              if (h) {
                t = (char *)h;
              }
            }
            else {
              t = (wchar_t *)h;
            }
            CloseClipboard();
            if (!t.IsEmpty()) {
              cmd.Insert(t, ip++);
              ip += (t.Length() - 1);
            }
          }
        }
      }
      else if (ch != 0) {
        cmd.Insert(ch, ip++, cnt);
        ip += (cnt - 1);
      }
    }
    }
    COORD crp = si.dwCursorPosition;
    crp.X += ip;
    if (crp.X >= si.dwSize.X) {
      int inc = crp.X / si.dwSize.X;
      crp.Y += inc;
      crp.X -= inc*si.dwSize.X;
    }
    DWORD written;
    if (clear_last) {
      COORD ccp = si.dwCursorPosition;
      ccp.X += cmd.Length();
      if (ccp.X >= si.dwSize.X) {
        int inc = ccp.X / si.dwSize.X;
        ccp.Y += inc;
        ccp.X -= inc*si.dwSize.X;
      }
      FillConsoleOutputCharacter(out, L' ', 1, ccp, &written);
      FillConsoleOutputAttribute(out, si.wAttributes, 1, ccp, &written);
    }
    SetConsoleCursorPosition(out, si.dwCursorPosition);
    WriteConsole(out, cmd.u_str(), cmd.Length(), &written, NULL);
    SetConsoleCursorPosition(out, crp);
    return true;
  }

  void reset() {
    GetConsoleScreenBufferInfo(out, &si);
    cmd.SetLength(0);
    ip = 0;
  }
};
#endif

int main(int argc, char* argv[]) {
  olx_object_ptr<TStopWatch> sw(new TStopWatch(__FUNC__));
  TEGC::Initialise();
#ifndef __WIN32__  // dummy stuff for wxWidgets...
  MyApp wx_app;
  wxAppConsole::SetInstance(&wx_app);
  wx_app.SetAppName(wxT("olex2c"));
  rl_readline_name = argv[0];
  //struct termios new_settings, stored_settings;
  //tcgetattr(0,&stored_settings);
  //new_settings = stored_settings;
  //new_settings.c_lflag &= (~(ICANON|ECHO));
  //new_settings.c_cc[VTIME] = 0;
  //tcgetattr(0,&stored_settings);
  //new_settings.c_cc[VMIN] = 1;
  //tcsetattr(0,TCSANOW,&new_settings);
#endif
  olxstr bd(TBasicApp::GuessBaseDir(argv[0], "OLEX2_DIR"));
  olxstr base_dir = TEFile::AddPathDelimeter(
    TEFile::ExtractFilePath(bd));
  olx_setenv("PATH", TEFile::TrimPathDelimeter(base_dir) << olx_env_sep()
    << olx_getenv("PATH"));
  TOlex2c olex(bd);
  TBasicApp::GetInstance().InitArguments(argc, argv);

#ifdef __WIN32__
  SetConsoleTitle(olxT("Olex2 Console"));
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  SetConsoleMode(hStdin, ENABLE_WINDOW_INPUT);
  HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
  TLibrary &Library = olex.GetLibrary();
  std::cout << "Welcome to Olex2 console\n";
  std::cout << "GUI basedir is: " << TBasicApp::GetBaseDir().c_str() << '\n';
  olex.processMacro("echo Compilation information: GetCompilationInfo(full)");
#ifdef __WIN32__  // readline prints one itself...
  std::cout << ">>";
#endif
  if (TBasicApp::GetInstance().GetArguments().Count() >= 2 &&
    TBasicApp::GetInstance().GetArguments()[1].EndsWith(".autochem"))
  {
    olxstr sf(TEFile::ChangeFileExt(
      TBasicApp::GetInstance().GetArguments()[1], EmptyString()));
    olex.processMacro(olxstr("start_autochem ").quote() << sf);
  }
  else if (TBasicApp::GetInstance().GetArguments().Count() >= 2 &&
    TBasicApp::GetInstance().GetArguments()[1].EndsWith(".rpac"))
  {
    olxstr sf(TEFile::ChangeFileExt(
      TBasicApp::GetInstance().GetArguments()[1], EmptyString()));
    olex.processMacro(olxstr("start_rpac ").quote() << sf);
  }
  else if (TBasicApp::GetInstance().GetArguments().Count() > 1 &&
    TBasicApp::GetInstance().GetArguments()[1].Equalsi("sisyphos"))
  {
    olex.processMacro(olxstr("sisyphos"));
  }
    else if (TBasicApp::GetInstance().GetArguments().Count() > 1 &&
    TBasicApp::GetInstance().GetArguments()[1].Equalsi("DataGrabber"))
  {
    olex.processMacro(olxstr("DataGrabber"));
  }
  else if (TBasicApp::GetInstance().GetArguments().Count() > 1 &&
    TBasicApp::GetInstance().GetArguments()[1].Equalsi("datagrabber"))
  {
    olex.processMacro(olxstr("DataGrabber"));
  }
  else if (TBasicApp::GetInstance().GetArguments().Count() > 1 &&
    TBasicApp::GetInstance().GetArguments()[1].Equalsi("server"))
  {
    const TStrList &args = TBasicApp::GetInstance().GetArguments();
    if (args.Count() > 2 && args[2].IsNumber()) {
      olex.processMacro(olxstr("server start -p=") << args[2]);
#if defined(__WIN32__) && !defined(DEBUG_)
      FreeConsole();
#endif
    }
    else {
      olex.processMacro("server start");
    }
  }
  else {
#ifdef __WIN32__
    MSG msg;
    BOOL rv;
    olx_array_ptr<INPUT_RECORD> bf(16);
    Console cl;
    while ((rv = GetMessage(&msg, 0, 0, 0)) != 0) {
      if (rv == -1) {
        break;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      DWORD read;
      if (!PeekConsoleInput(hStdin, bf, 16, &read) || read == 0) {
        continue;
      }
      bool stop = false;
      ReadConsoleInput(hStdin, bf, read, &read);
      for (int i = 0; i < read; i++) {
        INPUT_RECORD &r = bf[i];
        if (r.EventType != KEY_EVENT || !r.Event.KeyEvent.bKeyDown) {
          continue;
        }
        if (!cl.process(
          r.Event.KeyEvent.dwControlKeyState,
          r.Event.KeyEvent.wVirtualKeyCode,
          r.Event.KeyEvent.uChar.UnicodeChar,
          r.Event.KeyEvent.wRepeatCount))
        {
          stop = true;
          break;
        }
      }
      if (stop) {
        break;
      }
    }
#else
    std::ios::sync_with_stdio();
    while (true) {
      TBasicApp::GetInstance().OnIdle.Execute(0);
      char* _cmd = readline(">>");
      if (_cmd == 0) {
        break;
      }
      add_history(_cmd);
      olxstr cmd = _cmd;
      free(_cmd);
      if (!OnCommand(cmd)) {
        break;
      }
    }
#endif
  }
  delete sw.release();
  TBasicApp::GetInstance().OnIdle.Execute(0);
  return 0;
}

