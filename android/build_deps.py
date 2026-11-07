"""Build the GUI base libraries for the Android APK into a per-ABI prefix.

  gl4es 1.1.6 (desktop GL 1.x on GLES2, static libGL.a)  -> <prefix>/<abi>/gl4es
  ptitSeb GLU (libutil+libtess, static libGLU.a)           -> <prefix>/<abi>/gl4es
  wxWidgets 3.3.3, wxQt on Qt 6, static, wxGLCanvas on      -> <prefix>/<abi>/wx

Sources are the unpacked upstream release archives (see android/README.md for
URLs and SHA256). Each one is copied once to <build>/<name>-src and the patches
in android/patches/<name>/*.patch are applied there with git apply, so the
unpacked trees stay pristine.

  python android/build_deps.py [--abi x86_64 --abi arm64-v8a] [--only wx]

--legacy builds the same recipe for the API 21 line (Qt 5.15.2, NDK r21e,
android-21) into D:/Android/prefix-legacy, the prefix of the legacy-* presets:
  - Qt 5.15.2 has no qt.toolchain.cmake: the NDK toolchain with the multi-ABI
    Qt 5 kit as find root
  - NDK r21e for everything: Qt 5.15.2 was built with it and wx shares its
    libc++_shared, so no r27 compiler-rt builtins end up in an r21 link
  - wx at debug level 1, and without what bionic lacks below API 23-26
"""
import argparse, os, shutil, subprocess, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ABIS = ["x86_64", "arm64-v8a", "x86", "armeabi-v7a"]
# Qt names its kits differently from the NDK ABIs
QT_KIT = {"x86_64": "x86_64", "arm64-v8a": "arm64_v8a", "x86": "x86",
          "armeabi-v7a": "armv7"}

ap = argparse.ArgumentParser()
ap.add_argument("--legacy", action="store_true", help="Qt 5.15.2 + NDK r21e, API 21")
ap.add_argument("--abi", action="append", choices=ABIS)
ap.add_argument("--only", action="append", choices=["gl4es", "glu", "wx"])
ap.add_argument("--src", default="D:/Android/src")
ap.add_argument("--build")
ap.add_argument("--prefix")
ap.add_argument("--qt")
ap.add_argument("--sdk", default=os.environ.get("ANDROID_SDK_ROOT", "D:/Android/sdk"))
ap.add_argument("--ndk")
ap.add_argument("--cmake", default="C:/Qt/Tools/CMake_64/bin/cmake.exe")
ap.add_argument("--ninja", default="C:/Qt/Tools/Ninja/ninja.exe")
ap.add_argument("-j", "--jobs", type=int, default=6)
a = ap.parse_args()
for k, modern, legacy in [
        ("build", "D:/Android/build", "D:/Android/build/legacy"),
        ("prefix", "D:/Android/prefix", "D:/Android/prefix-legacy"),
        ("qt", "C:/Qt/6.11.1", "D:/Android/Qt5/5.15.2/android"),
        ("ndk", "D:/Android/sdk/ndk/27.2.12479018", "D:/Android/sdk/ndk/21.4.7075529")]:
    if getattr(a, k) is None:
        setattr(a, k, legacy if a.legacy else modern)
api = 21 if a.legacy else 28
abis = a.abi or ABIS
only = a.only or ["gl4es", "glu", "wx"]
src, build, prefix = Path(a.src), Path(a.build), Path(a.prefix)

TREES = {"gl4es": "gl4es-1.1.6", "glu": "GLU-2fed2bda", "wxwidgets": "wxWidgets-3.3.3"}


def run(*cmd, cwd=None):
    print("+", " ".join(map(str, cmd)), flush=True)
    subprocess.run([str(c) for c in cmd], cwd=cwd, check=True)


def patched(name):
    """Pristine copy of the unpacked tree with our patches applied (once)."""
    dst = build / f"{name}-src"
    stamp = dst / ".olx-patched"
    if stamp.exists():
        return dst
    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src / TREES[name], dst)
    for p in sorted((HERE / "patches" / name).glob("*.patch")):
        run("git", "apply", "--whitespace=nowarn", p, cwd=dst)
    stamp.write_text("ok")
    return dst


def cmake(srcdir, bdir, toolchain, abi, *defs):
    run(a.cmake, "-S", srcdir, "-B", bdir, "-G", "Ninja",
        f"-DCMAKE_MAKE_PROGRAM={a.ninja}", "-DCMAKE_BUILD_TYPE=Release",
        f"-DCMAKE_TOOLCHAIN_FILE={toolchain}", f"-DANDROID_ABI={abi}",
        f"-DANDROID_PLATFORM=android-{api}", "-DANDROID_STL=c++_shared",
        f"-DANDROID_SDK_ROOT={a.sdk}", f"-DANDROID_NDK_ROOT={a.ndk}",
        f"-DANDROID_NDK={a.ndk}",
        f"-DCMAKE_PROJECT_INCLUDE={(HERE / 'cmake' / 'abi_flags.cmake').as_posix()}",
        # r21 puts -Oz into armeabi-v7a (Thumb) Release flags: this -O2 wins
        *(["-DCMAKE_C_FLAGS_RELEASE=-O2 -DNDEBUG", "-DCMAKE_CXX_FLAGS_RELEASE=-O2 -DNDEBUG"]
          if a.legacy else []), *defs)
    run(a.cmake, "--build", bdir, "-j", a.jobs)


ndk_tc = f"{a.ndk}/build/cmake/android.toolchain.cmake"
for abi in abis:
    gl = prefix / abi / "gl4es"
    if "gl4es" in only:
        s = patched("gl4es")
        b = build / f"gl4es-{abi}"
        # gl4es writes its library to <source>/lib whatever the build dir; clear
        # it so one ABI never picks up the other's archive
        shutil.rmtree(s / "lib", ignore_errors=True)
        # NO_INIT_CONSTRUCTOR: the default state reads GL_VIEWPORT from the
        # current context, which exists only after the first makeCurrent; the
        # wxQt glcanvas patch calls initialize_gl4es() then
        cmake(s, b, ndk_tc, abi, "-DNOX11=ON", "-DNOEGL=ON", "-DSTATICLIB=ON",
              "-DNO_GBM=ON", "-DNO_INIT_CONSTRUCTOR=ON", "-DUSE_ANDROID_LOG=ON",
              "-DCMAKE_POLICY_VERSION_MINIMUM=3.5")
        (gl / "lib").mkdir(parents=True, exist_ok=True)
        shutil.copy2(s / "lib" / "libGL.a", gl / "lib" / "libGL.a")
        shutil.copytree(s / "include", gl / "include", dirs_exist_ok=True)
    if "glu" in only:
        cmake(HERE / "cmake" / "glu", build / f"glu-{abi}", ndk_tc, abi,
              f"-DGLU_SOURCE_DIR={patched('glu')}", f"-DGL_INCLUDE_DIR={gl}/include",
              f"-DCMAKE_INSTALL_PREFIX={gl}")
        run(a.cmake, "--install", build / f"glu-{abi}")
    if "wx" in only:
        b = build / f"wx-{abi}"
        if a.legacy:
            qt = [f"-DCMAKE_FIND_ROOT_PATH={a.qt}", f"-DCMAKE_PREFIX_PATH={a.qt}",
                  # Qt 5's config links Threads::Threads without finding it
                  f"-DCMAKE_PROJECT_INCLUDE={(HERE / 'cmake' / 'qt5_threads.cmake').as_posix()}",
                  # nl_langinfo() is API 26+
                  "-DHAVE_LANGINFO_H=0",
                  # sckaddr.cpp pairs gethostbyname_r with gethostbyaddr_r (API
                  # 23+): the mutex-guarded non-_r fallback instead
                  "-DHAVE_FUNC_GETHOSTBYNAME_R_6=0", "-DHAVE_FUNC_GETHOSTBYNAME_R_5=0",
                  # 32-bit bionic hides fseeko/ftello under _FILE_OFFSET_BITS=64
                  # below API 24: 32-bit off_t there (2 GB files)
                  "-DwxBUILD_LARGEFILE_SUPPORT=" +
                  ("OFF" if abi in ("armeabi-v7a", "x86") else "ON")]
            tc = ndk_tc
        else:
            qt = [f"-DQT_HOST_PATH={a.qt}/mingw_64",
                  # release: no wxASSERT checks (cmake/deps.cmake follows suit)
                  "-DwxBUILD_DEBUG_LEVEL=0"]
            tc = f"{a.qt}/android_{QT_KIT[abi]}/lib/cmake/Qt6/qt.toolchain.cmake"
        cmake(patched("wxwidgets"), b, tc, abi, *qt,
              f"-DCMAKE_INSTALL_PREFIX={prefix / abi / 'wx'}",
              "-DwxBUILD_TOOLKIT=qt", "-DwxBUILD_SHARED=OFF",
              "-DwxBUILD_SAMPLES=OFF", "-DwxBUILD_TESTS=OFF", "-DwxBUILD_DEMOS=OFF",
              "-DwxUSE_OPENGL=ON",
              # find_package(OpenGL) has nothing to find in the NDK: point it
              # at gl4es, or wx silently turns wxUSE_OPENGL off
              f"-DOPENGL_INCLUDE_DIR={gl}/include", f"-DOPENGL_gl_LIBRARY={gl}/lib/libGL.a",
              f"-DOPENGL_glu_LIBRARY={gl}/lib/libGLU.a",
              "-DwxUSE_REGEX=builtin", "-DwxUSE_ZLIB=builtin", "-DwxUSE_EXPAT=builtin",
              "-DwxUSE_LIBJPEG=builtin", "-DwxUSE_LIBPNG=builtin", "-DwxUSE_LIBTIFF=builtin",
              "-DwxUSE_NANOSVG=builtin",
              "-DwxUSE_WEBVIEW=OFF", "-DwxUSE_MEDIACTRL=OFF", "-DwxUSE_LIBSDL=OFF",
              "-DwxUSE_SECRETSTORE=OFF", "-DwxUSE_LIBWEBP=OFF", "-DwxUSE_LIBLZMA=OFF",
              "-DwxUSE_LIBMSPACK=OFF", "-DwxUSE_LIBGNOMEVFS=OFF", "-DwxUSE_GLCANVAS_EGL=OFF",
              # Bionic iconv fails wx's UTF-32LE probe, and wxConvLocal then
              # spins forever on E2BIG in wxEntry (argv conversion), before
              # OnInit; wx's own UTF-8/UTF-32 converters do the job
              "-DwxUSE_LIBICONV=OFF")
        run(a.cmake, "--install", b)
print("done:", ", ".join(only), "for", ", ".join(abis))
