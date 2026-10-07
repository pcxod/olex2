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
#include <QtCore/qcoreapplication_platform.h>  // QAndroidApplication
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
    // assemble_assets.py appends '-' to *.gz (and *-) names: aapt would
    // decompress them
    const fs::path p = dst / fs::u8path(line.back() == '-'
      ? line.substr(0, line.size() - 1) : line);
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
  /* Qt's Java loader hands QT_PLUGIN_PATH, HOME, TMPDIR, fonts and style over
  with Os.setenv. Under a native bridge (arm64 APK on an x86_64 emulator or
  Chromebook) that sets the host libc's environment, not this one, and Qt
  finds no platform plugin: copy whatever is missing here
  */
  QJniObject jenv = QJniObject::callStaticObjectMethod("android/system/Os",
    "environ", "()[Ljava/lang/String;");
  jobjectArray ea = static_cast<jobjectArray>(jenv.object());
  for (jsize i = 0, n = ea ? env->GetArrayLength(ea) : 0; i < n; i++) {
    const std::string kv = QJniObject::fromLocalRef(
      env->GetObjectArrayElement(ea, i)).toString().toStdString();
    const size_t eq = kv.find('=');
    if (eq != std::string::npos && eq > 0) {
      setenv(kv.substr(0, eq).c_str(), kv.c_str() + eq + 1, 0);
    }
  }

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
  /* Embedded CPython: the stdlib, lib-dynload and site-packages are assets
  under base/python; libpython and every other lib*.so are in the APK's
  native library dir, where the linker finds them by soname. cctbx is found
  by initpy.py under base/cctbx.
  */
  setenv("PYTHONHOME", (base / "python").c_str(), 1);
  // C's measurements: one BLAS thread is fastest on every ABI, the OpenMP
  // loops of smtbx take all cores. Affinity off: libomp's topology probe
  // aborts under ndk_translation and binding buys nothing on big.LITTLE
  setenv("OPENBLAS_NUM_THREADS", "1", 0);
  setenv("KMP_AFFINITY", "disabled", 0);
  /* NoSpherA2 is a PIE executable packaged as libNoSpherA2.so so that it is
  installed executable; files under filesDir cannot be exec'd (W^X, API 29+).
  NoSpherA2.py finds <basedir>/NoSpherA2 and its basis_sets and occ/share
  next to it, so a symlink stands in. The native dir changes with every
  install, hence relinked on every start.
  */
  {
    const std::string nld = ctx
      .callObjectMethod("getApplicationInfo",
        "()Landroid/content/pm/ApplicationInfo;")
      .getObjectField<jstring>("nativeLibraryDir").toString().toStdString();
    const fs::path link = base / "NoSpherA2";
    fs::remove(link, ec);
    fs::create_symlink(fs::path(nld) / "libNoSpherA2.so", link, ec);
    if (ec) {
      olx_log("cannot link NoSpherA2: " + ec.message());
    }
  }
  // argv[0] is the .so path, /data/app/~~<base64>==/..., and TBasicApp
  // reads any argument holding '=' as an option: Olex2 gets its base dir
  // from OLEX2_DIR anyway. No file picker before S4: with no
  // applicationArguments open the bundled sucrose (mainform.cpp loads
  // argv[1] when the file exists)
  std::string exe = (base / "olex2").string(),
    sample = (base / "sample_data/sucrose/sucrose.res").string();
  argv[0] = exe.data();
  char *args[] = { argv[0], sample.data(), nullptr };
  if (argc < 2) {
    argc = 2;
    argv = args;
  }
  return wxEntry(argc, argv);
}
