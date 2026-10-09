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

Builds against Qt 6 (API 28+) and Qt 5.15 (the API 21 line). No
std::filesystem: NDK r21's libc++ has none, plain POSIX calls do instead.
*/
#include "wx/app.h"
#include <QtCore/qglobal.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#include <QtCore/qcoreapplication_platform.h>  // QAndroidApplication
static QJniObject android_context() {
  return QNativeInterface::QAndroidApplication::context();
}
#else
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QtAndroid>
using QJniObject = QAndroidJniObject;
using QJniEnvironment = QAndroidJniEnvironment;
static QJniObject android_context() { return QtAndroid::androidContext(); }
#endif
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

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

// mkdir -p of the directory part of path (up to the last '/')
static void make_parents(const std::string &path) {
  for (size_t i = path.find('/', 1); i != std::string::npos;
    i = path.find('/', i + 1))
  {
    mkdir(path.substr(0, i).c_str(), 0700);
  }
}

// returns false only when the assets are unusable
static bool extract_assets(AAssetManager *am, const std::string &dst) {
  std::string list;
  if (!read_asset(am, "olex2/files.txt", list)) {
    olx_log("assets/olex2/files.txt missing - APK built without assets");
    return false;
  }
  std::istringstream in(list);
  std::string stamp, line;
  std::getline(in, stamp);
  const std::string stamp_file = dst + "/.asset-stamp";
  {
    std::ifstream sf(stamp_file);
    std::string old;
    if (sf && std::getline(sf, old) && old == stamp) {
      return true;
    }
  }
  olx_log("extracting assets to " + dst);
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
    const std::string p = dst + '/' + (line.back() == '-'
      ? line.substr(0, line.size() - 1) : line);
    make_parents(p);
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    if (!f.write(data.data(), data.size())) {
      olx_log("cannot write " + p);
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
  QJniObject ctx = android_context();
  const std::string files_dir = ctx
    .callObjectMethod("getFilesDir", "()Ljava/io/File;")
    .callObjectMethod("getAbsolutePath", "()Ljava/lang/String;")
    .toString().toStdString();
  QJniObject jam = ctx.callObjectMethod("getAssets",
    "()Landroid/content/res/AssetManager;");
  QJniEnvironment env;
  AAssetManager *am = AAssetManager_fromJava(env.operator->(), jam.object());
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

  const std::string base = files_dir + "/olex2",
    config_dir = files_dir + "/config";
  if (am == nullptr || !extract_assets(am, base)) {
    olx_log("no usable GUI files; Olex2 will start without them");
  }
  mkdir(config_dir.c_str(), 0700);
  /* Optional <config_dir>/env.txt, one KEY=VALUE per line, '#' comments:
  device tuning without a rebuild, e.g. LIBGL_NOHIGHP=1 (gl4es, Mali-400
  fragment shaders are mediump only) or OMP_NUM_THREADS. Wins over the
  defaults below that are set without overwrite. Written with
  adb shell run-as <package> (the legacy APK is debuggable)
  */
  {
    std::ifstream ef(config_dir + "/env.txt");
    std::string l;
    while (std::getline(ef, l)) {
      if (!l.empty() && l.back() == '\r') {
        l.pop_back();
      }
      const size_t eq = l.find('=');
      if (l.empty() || l[0] == '#' || eq == std::string::npos || eq == 0) {
        continue;
      }
      setenv(l.substr(0, eq).c_str(), l.c_str() + eq + 1, 1);
      olx_log("env.txt: " + l);
    }
  }
  // first start: Android defaults (rundir-overlay); later edits are the user's
  const std::string options = config_dir + "/.options";
  if (access(options.c_str(), F_OK) != 0) {
    std::ifstream in(base + "/android.options", std::ios::binary);
    std::ofstream(options, std::ios::binary) << in.rdbuf();
  }
  /* env.txt may move the data dir, e.g. to the SD card's app dir
  /storage/<uuid>/Android/data/<package>/files/data (no permission needed)
  */
  setenv("OLEX2_DATADIR", (files_dir + "/data").c_str(), 0);
  const std::string data_dir = getenv("OLEX2_DATADIR");
  make_parents(data_dir + '/');
  // read by xglapp.cpp (base/config dir, GL attributes) and patchapi.cpp
  setenv("OLEX2_DIR", base.c_str(), 1);
  setenv("OLEX2_DATADIR_STATIC", "TRUE", 1);
  setenv("OLEX2_CONFIGDIR", config_dir.c_str(), 1);
  setenv("OLEX2_GL_STEREO", "false", 1);
  setenv("OLEX2_GL_MULTISAMPLE", "false", 1);
  /* Embedded CPython: the stdlib, lib-dynload and site-packages are assets
  under base/python; libpython and every other lib*.so are in the APK's
  native library dir, where the linker finds them by soname. cctbx is found
  by initpy.py under base/cctbx.
  */
  setenv("PYTHONHOME", (base + "/python").c_str(), 1);
  // C's measurements: one BLAS thread is fastest on every ABI, the OpenMP
  // loops of smtbx take all cores. Affinity off: libomp's topology probe
  // aborts under ndk_translation and binding buys nothing on big.LITTLE
  setenv("OPENBLAS_NUM_THREADS", "1", 0);
  setenv("KMP_AFFINITY", "disabled", 0);
  // FLINT's trials: the adapter's default of cores - 1 leaves the 8th of 8
  // trials for a second round on the 8-core tablet (52.6 -> 46.5 s on water);
  // the GUI thread only polls meanwhile
  setenv("SMTBX_SOLVE_THREADS",
    std::to_string(sysconf(_SC_NPROCESSORS_CONF)).c_str(), 0);
  // Samsung's hotplug keeps cores 4-7 offline at idle, so Python's
  // cpu_count() (online cores) says 4 on the 8-core tablet and the
  // NoSpherA2 CPU list and refinement thread defaults follow it.
  // Python 3.13 reads this at start-up instead
  setenv("PYTHON_CPU_COUNT",
    std::to_string(sysconf(_SC_NPROCESSORS_CONF)).c_str(), 0);
  /* NoSpherA2 is a PIE executable packaged as libNoSpherA2.so so that it is
  installed executable; files under filesDir cannot be exec'd (W^X, API 29+).
  NoSpherA2.py finds <basedir>/NoSpherA2 and its basis_sets and occ/share
  next to it, so a symlink stands in. The native dir changes with every
  install, hence relinked on every start. pTB (optional) the same way.
  */
  {
    const std::string nld = ctx
      .callObjectMethod("getApplicationInfo",
        "()Landroid/content/pm/ApplicationInfo;")
      .getObjectField<jstring>("nativeLibraryDir").toString().toStdString();
    for (const char *n : { "NoSpherA2", "ptb" }) {
      const std::string link = base + "/" + n, so = nld + "/lib" + n + ".so";
      unlink(link.c_str());
      if (access(so.c_str(), F_OK) == 0 && symlink(so.c_str(), link.c_str()) != 0) {
        olx_log(std::string("cannot link ") + n);
      }
    }
  }
  /* ptb is a static glibc binary: glibc registers rseq before any of its
  constructors run, and the app seccomp filter answers that with SIGSYS (the
  other new syscalls sigsys.c turns into ENOSYS). Inherited by the child
  */
  setenv("GLIBC_TUNABLES", "glibc.pthread.rseq=0", 0);
  /* SALTED models stay out of the APK: assets are extracted, so the 844 MB
  combo model would be stored twice. Each volume's
  Android/data/<package>/files/salted (created here, fillable over USB, no
  permission needed) is registered once as a model folder; the last phil
  assignment wins and the GUI's add/remove edits the list from then on
  */
  {
    const std::string up = data_dir + "/user.phil";
    std::stringstream ss;
    ss << std::ifstream(up).rdbuf();
    if (ss.str().find("salted_models_list") == std::string::npos) {
      QJniObject dirs = ctx.callObjectMethod("getExternalFilesDirs",
        "(Ljava/lang/String;)[Ljava/io/File;",
        QJniObject::fromString("salted").object());
      jobjectArray da = static_cast<jobjectArray>(dirs.object());
      std::string list;
      for (jsize i = 0, n = da ? env->GetArrayLength(da) : 0; i < n; i++) {
        QJniObject d = QJniObject::fromLocalRef(env->GetObjectArrayElement(da, i));
        if (d.isValid()) {  // null for a volume that is not mounted
          list += ';' + d.callObjectMethod("getAbsolutePath",
            "()Ljava/lang/String;").toString().toStdString();
        }
      }
      std::ofstream(up, std::ios::app)
        << "\nuser.NoSpherA2.salted_models_list = \"" << list << "\"\n";
      olx_log("SALTED model folders" + list);
    }
  }
  // argv[0] is the .so path, /data/app/~~<base64>==/..., and TBasicApp
  // reads any argument holding '=' as an option: Olex2 gets its base dir
  // from OLEX2_DIR anyway. No file picker before S4: with no
  // applicationArguments open the bundled sucrose (mainform.cpp loads
  // argv[1] when the file exists)
  std::string exe = base + "/olex2",
    sample = base + "/sample_data/sucrose/sucrose.res";
  argv[0] = exe.data();
  char *args[] = { argv[0], sample.data(), nullptr };
  if (argc < 2) {
    argc = 2;
    argv = args;
  }
  return wxEntry(argc, argv);
}
