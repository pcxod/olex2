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
| `build_deps.py` | Builds the GUI base libraries per ABI into `D:/Android/prefix/<abi>`: wxWidgets 3.3.3 (wxQt, static, GL on), gl4es 1.1.6 (`libGL.a`) and ptitSeb GLU (`libGLU.a`), with `patches/<name>/*.patch` applied. |
| `cmake/deps.cmake` | Imports those prebuilt libraries (`OLX_ANDROID_PREFIX`) as `gl4es_gl`, `olx_glu` and `wxWidgets::*`. |
| `tests/glprobe` | Small wx GL test APK: fixed-function drawing through gl4es in a `wxGLCanvas`, and a log of every touch and gesture event. |
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

Unpacked under `D:/Android/src` (`--src`) and built by `build_deps.py`:

| Package | Official source | SHA256 | Licence |
|---|---|---|---|
| wxWidgets 3.3.3 | https://github.com/wxWidgets/wxWidgets/releases/download/v3.3.3/wxWidgets-3.3.3.tar.bz2 | `81b09d6dd9f1ed9301f8c55a968a488d0491f264dc2bab19a7e407ac67009482` | wxWindows |
| gl4es 1.1.6 | https://github.com/ptitSeb/gl4es/archive/v1.1.6.tar.gz | `dca1d897e492a0cb163a3390f273fbd4cc7ab2367d236d93dc2b321ce108ed5c` | MIT |
| ptitSeb GLU 2fed2bda | https://github.com/ptitSeb/GLU/archive/2fed2bda.tar.gz | `8a016d32fc1fed742f10ba8e4bc32151598f6273a0dbabec15ba47c44151c879` | SGI Free B |

```bash
cd D:\git\olex2-android; python android/build_deps.py --abi x86_64 --abi arm64-v8a
```

Each tree is copied once to `D:/Android/build/<name>-src` and patched there
(stamp `.olx-patched`; delete it to re-patch). Everything is static, so the
libraries end up inside the app's `.so` and androiddeployqt has nothing extra
to package. wx is configured with `wxUSE_LIBICONV=OFF`: bionic's iconv makes
`wxConvLocal` loop forever on E2BIG inside `wxEntry`.

What the patches do:

- `wxwidgets/0001`: the GL canvas initialises gl4es after the first
  `makeCurrent`, binds it to `QOpenGLWidget::defaultFramebufferObject()` on
  every `SetCurrent` (the FBO changes on resize) and resyncs gl4es's state
  caches; `SwapBuffers` flushes gl4es and schedules a repaint when called
  outside `paintGL`. Gesture positions are client coordinates; pinch zoom and
  rotation are cumulative and sent on every change and at the end; the
  one-finger pan recognizer is off on Android (pan needs two fingers); a
  double tap gives one `wxEVT_LEFT_DCLICK`, not two.
- `wxwidgets/0002`: bionic has no `getservbyname_r`.
- `gl4es/0001`: lets the host set the main framebuffer (`gl4es_setMainFBO`).
- `gl4es/0002`: `glext.h` typedef clash with the NDK headers.
- `gl4es/0003`: `gl4es_resyncState()`. `QOpenGLWidget` resets the program,
  array buffer, blend and viewport before `paintGL` behind gl4es's back; without
  the resync every paint after the first fails with `GL_INVALID_OPERATION`.

Touch as wx delivers it after the patches (AVD, `tests/glprobe/touch.ps1`):
long press arrives once, after a 1 s hold, flagged END only; rotation comes as
`wxEVT_GESTURE_ROTATE` in cumulative radians, negative = counter-clockwise;
during two-finger gestures finger 0 also produces left down/up.

## Build

Run these in PowerShell from the repository root. The build directories are
`build/android-arm64` and `build/android-x86_64`. They are not ignored by git,
so do not commit them.

```bash
cd D:\git\olex2-android\android; cmake --preset android-x86_64
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
- A wxQt top-level frame on Android stays at 400x250 DIP unless `Maximize()`d.
- After a resize Qt recreates the canvas FBO and leaves texture 0 bound on the
  active unit; `gl4es_resyncState` does not restore texture bindings.
