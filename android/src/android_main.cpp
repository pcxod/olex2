/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

/* Android entry point. Qt's loader calls main(); xglapp.cpp uses
IMPLEMENT_APP_NO_MAIN on Android so this one runs instead of wx's.

Olex2 expects a writable base directory with its GUI files. The APK carries
them in assets/olex2 (android/assemble_assets.py) together with files.txt:
line 1 is a content stamp, every other line a path relative to assets/olex2.
The NDK asset API cannot list sub-directories, hence the list. The files are
extracted once per stamp into <filesDir>/olex2.
*/
#include "wx/app.h"
#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#include <QtCore/qnativeinterface.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

static void olx_log(const std::string &s) {
  __android_log_print(ANDROID_LOG_INFO, "olex2", "%s", s.c_str());
}

static bool read_asset(AAssetManager *am, const std::string &name,
  std::string &out)
{
  AAsset *a = AAssetManager_open(am, name.c_str(), AASSET_MODE_STREAMING);
  if (a == nullptr) {
    return false;
  }
  out.clear();
  char buf[65536];
  int n;
  while ((n = AAsset_read(a, buf, sizeof(buf))) > 0) {
    out.append(buf, n);
  }
  AAsset_close(a);
  return n == 0;
}

// returns false only when the assets are unusable
static bool extract_assets(AAssetManager *am, const fs::path &dst) {
  std::string list;
  if (!read_asset(am, "olex2/files.txt", list)) {
    olx_log("assets/olex2/files.txt missing - APK built without assets");
    return false;
  }
  std::istringstream in(list);
  std::string stamp, line;
  std::getline(in, stamp);
  const fs::path stamp_file = dst / ".asset-stamp";
  {
    std::ifstream sf(stamp_file);
    std::string old;
    if (sf && std::getline(sf, old) && old == stamp) {
      return true;
    }
  }
  olx_log("extracting assets to " + dst.string());
  std::error_code ec;
  size_t cnt = 0;
  std::string data;
  while (std::getline(in, line)) {
    if (line.empty()) {
      continue;
    }
    if (!read_asset(am, "olex2/" + line, data)) {
      olx_log("cannot read asset " + line);
      return false;
    }
    const fs::path p = dst / fs::u8path(line);
    fs::create_directories(p.parent_path(), ec);
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    if (!f.write(data.data(), data.size())) {
      olx_log("cannot write " + p.string());
      return false;
    }
    cnt++;
  }
  // the stamp goes last: an interrupted extraction is redone next start
  std::ofstream(stamp_file) << stamp << '\n';
  olx_log("extracted " + std::to_string(cnt) + " files");
  return true;
}

int main(int argc, char **argv) {
  QJniObject ctx = QNativeInterface::QAndroidApplication::context();
  const std::string files_dir = ctx
    .callObjectMethod("getFilesDir", "()Ljava/io/File;")
    .callObjectMethod("getAbsolutePath", "()Ljava/lang/String;")
    .toString().toStdString();
  QJniObject jam = ctx.callObjectMethod("getAssets",
    "()Landroid/content/res/AssetManager;");
  QJniEnvironment env;
  AAssetManager *am = AAssetManager_fromJava(env.jniEnv(), jam.object());

  const fs::path base = fs::path(files_dir) / "olex2";
  if (am == nullptr || !extract_assets(am, base)) {
    olx_log("no usable GUI files; Olex2 will start without them");
  }
  std::error_code ec;
  fs::create_directories(fs::path(files_dir) / "data", ec);
  fs::create_directories(fs::path(files_dir) / "config", ec);
  // first start: Android defaults (rundir-overlay); later edits are the user's
  fs::copy_file(base / "android.options",
    fs::path(files_dir) / "config" / ".options",
    fs::copy_options::skip_existing, ec);
  // read by xglapp.cpp (base/config dir, GL attributes) and patchapi.cpp
  setenv("OLEX2_DIR", base.c_str(), 1);
  setenv("OLEX2_DATADIR", (fs::path(files_dir) / "data").c_str(), 1);
  setenv("OLEX2_DATADIR_STATIC", "TRUE", 1);
  setenv("OLEX2_CONFIGDIR", (fs::path(files_dir) / "config").c_str(), 1);
  setenv("OLEX2_GL_STEREO", "false", 1);
  setenv("OLEX2_GL_MULTISAMPLE", "false", 1);
  return wxEntry(argc, argv);
}
