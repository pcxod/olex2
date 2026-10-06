# Olex2 on Android (wxWidgets GUI, wxQt backend)

This directory builds the original wxWidgets Olex2 (`olex/`, `glib/`, `gxlib/`
and the HTML GUI) as an Android APK. It is not a Qt rewrite. wxWidgets runs on
its Qt port (wxQt), so Qt is only the platform layer below wx. No Olex2 code
uses Qt directly.

> **Status: wiring only, never built.** Nothing in this directory has been
> configured or compiled yet: the third-party sources below have not been
> downloaded. Lines marked [U] are unverified.

## What is here

| File | Role |
|---|---|
| `CMakeLists.txt` | One `qt_add_executable(olex2)` over the same source globs as the root build, minus unirun and the Windows-only `fsindex`/`zip`/`unzip`. Stage S1 has no `_PYTHON`, `_OPENSSL` or cctbx. |
| `CMakePresets.json` | `android-arm64` (tablet) and `android-x86_64` (emulator). Each uses the Qt 6.11.1 Android toolchain, NDK 27.2.12479018, API 28, and at most 6 build jobs. |
| `cmake/deps.cmake` | FetchContent for wxWidgets 3.3 (`wxBUILD_TOOLKIT=qt`, static), gl4es (GL 1.x on GLES2), and the GLU `libutil` subset. |
| `src/android_main.cpp` | `main()`. It extracts `assets/olex2` once per content stamp, sets the `OLEX2_*` environment, then calls `wxEntry`. `olex/xglapp.cpp` uses `IMPLEMENT_APP_NO_MAIN` on Android. |
| `AndroidManifest.xml` | `org.olex2.android`, landscape, `extractNativeLibs`, INTERNET, GLES 2. |
| `assemble_assets.py` | Copies an allowlist of files from an installed rundir (read only) plus `rundir-overlay/`. It refuses keys, AC7 files, binaries and Python. |
| `rundir-overlay/android.options` | Android defaults, copied to `<config>/.options` on the first start: no GL selection, no multisampling, no stereo, a 12 px tap threshold, `gl_touch=true`. |

Touch input is shared with the desktop build. `olex/touchnav.cpp` maps wx
gesture events onto the `TGlMouse` view paths:

| Gesture | Action |
|---|---|
| Pinch | Zoom |
| Two-finger pan | Translate |
| Twist | Rotate about Z |
| Long press | Context menu |
| Double tap | Centre |
| One-finger drag and tap | Arrive as left-mouse events, so they rotate and select |

The option `gl_touch` turns this on. It defaults to true on Android and false
elsewhere. The desktop test is `tests/touch` (ctest `touch_gestures`).

## Requirements (download and install yourself)

Already on FLOWOFFICE:

- Qt 6.11.1 `android_arm64_v8a`, `android_x86_64` and `mingw_64` (host tools) under `C:/Qt/6.11.1`.
- CMake 3.30 and Ninja under `C:/Qt/Tools`.
- Android SDK under `D:/Android/sdk`, with platform 36, build-tools 36.0.0 and NDK 27.2.12479018.
- JDK 17 under `D:/Android/jdk17`.
- The AVD `olex2_tablet` (Pixel Tablet, android-36 google_apis x86_64).

Fetched by `cmake/deps.cmake` at configure time. Each one needs its SHA256:

| Package | Official source | Approx. size | Licence |
|---|---|---|---|
| wxWidgets 3.3.1 [U version] | https://github.com/wxWidgets/wxWidgets/releases/download/v3.3.1/wxWidgets-3.3.1.tar.bz2 | 45-60 MB unpacked, ~25 MB archive [U] | wxWindows |
| gl4es (tag `OLX_GL4ES_REF`, default `v1.1.6` [U tag]) | https://github.com/ptitSeb/gl4es | 5-10 MB [U] | MIT |
| Mesa GLU 9.0.3 | https://archive.mesa3d.org/glu/glu-9.0.3.tar.xz | ~1 MB [U] | SGI Free B |

Each hash is passed once. Compute it from a file you downloaded and checked
yourself (`Get-FileHash <file> -Algorithm SHA256`):

```bash
cmake --preset android-arm64 -DOLX_WX_SHA256=<sha> -DOLX_GL4ES_SHA256=<sha> -DOLX_GLU_SHA256=<sha>
```

You can also point at a tree you already unpacked:
`-DFETCHCONTENT_SOURCE_DIR_WXWIDGETS=D:/src/wxWidgets-3.3.1`. The same works
for `GL4ES` and `GLU`. Patches in `patches/<name>/*.patch` are applied only to
downloaded trees. There are none yet. Expected work for `patches/wxwidgets`,
all [U] until the cube experiment below:

- The wxQt GL canvas binds gl4es to `QOpenGLWidget::defaultFramebufferObject()` after `makeCurrent`.
- Pinch gives a cumulative zoom factor.
- Twist produces `wxEVT_GESTURE_ROTATE`.
- Long press fires when the hold triggers, not when the finger lifts.

## Build

Run these in PowerShell from the repository root. The build directories are
`build/android-arm64` and `build/android-x86_64`. They are not ignored by git,
so do not commit them.

```bash
cd D:\git\olex2-android\android; cmake --preset android-x86_64 -DOLX_WX_SHA256=<sha> -DOLX_GL4ES_SHA256=<sha> -DOLX_GLU_SHA256=<sha>
```

```bash
cd D:\git\olex2-android\android; cmake --build --preset android-x86_64
```

The APK is written to
`build/android-x86_64/android-build/build/outputs/apk/release/android-build-release-unsigned.apk`
[U path]. Use `android-arm64` for the tablet. For a debug-signed APK that
installs directly, configure with `-DCMAKE_BUILD_TYPE=Debug`, which gives
`.../apk/debug/android-build-debug.apk` [U path].

Assets come from `-DOLEX2_RUNDIR=<path>`, default `D:/devel/rundir-py3`. That
directory is only read. A dry run copies about 529 files (31 MB):

```bash
python D:\git\olex2-android\android\assemble_assets.py --self-test
```

## Emulator (x86_64)

The AVD currently has `hw.gpu.enabled = no`, and gl4es needs a GLES2 GPU.
Change these lines in `D:\Android\avd\olex2_tablet.avd\config.ini` first:
`hw.gpu.enabled = yes` and `hw.gpu.mode = host`.

```bash
D:\Android\sdk\emulator\emulator.exe -avd olex2_tablet -gpu host
```

```bash
D:\Android\sdk\platform-tools\adb.exe install -r D:\git\olex2-android\build\android-x86_64\android-build\build\outputs\apk\debug\android-build-debug.apk
```

```bash
D:\Android\sdk\platform-tools\adb.exe logcat -s olex2:* Qt:* libEGL:*
```

## Tablet (arm64-v8a)

1. Enable Developer options and USB debugging on the tablet.
2. Connect it and accept the RSA prompt there.
3. Check that it is listed:

```bash
D:\Android\sdk\platform-tools\adb.exe devices
```

```bash
D:\Android\sdk\platform-tools\adb.exe install -r D:\git\olex2-android\build\android-arm64\android-build\build\outputs\apk\debug\android-build-debug.apk
```

The first start extracts the GUI files to the app's private files directory,
`files/olex2`. Later starts extract again only when the asset stamp changes.
The log tag is `olex2`.

## Known open points (S1)

- wxQt on Android is not an officially supported wx target [U]. The first
  experiment is wx's `samples/opengl/cube`, built with these presets.
- Olex2 without `_PYTHON` has never been built [U]. The panel GUI is generated
  partly by Python. S1 aims for the 3-D view and the macro console; Python
  (CPython for Android) comes in S2.
- GLU `libutil/error.c` may reference the tessellator and NURBS error tables
  [U]. If it does, add `libtess` or stub those two symbols.
- gl4es CMake option names (`NOX11`, `NOEGL`, `STATICLIB`) and the target name
  `GL` are [U] for the chosen tag.
- wxQt sends no rotate or two-finger pan events, and it sends long press only at
  the end [U]. Until the wx patch exists, the touch mapping depends on what wxQt
  delivers.
