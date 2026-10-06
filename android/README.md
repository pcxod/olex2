# Olex2 on Android (wxWidgets GUI, wxQt backend)

This directory builds the original wxWidgets Olex2 (`olex/`, `glib/`, `gxlib/`
and the HTML GUI) as an Android APK. It is not a Qt rewrite. wxWidgets runs on
its Qt port (wxQt), so Qt is only the platform layer below wx. No Olex2 code
uses Qt directly.

> **Status (7 Oct 2026): stage S1 runs.** Four APKs (arm64-v8a, armeabi-v7a,
> x86_64, x86) build without Python. On the emulators Olex2 starts, loads the
> bundled sucrose, renders it, and selects, rotates and zooms by touch; see
> [Results](#results-s1-emulators). Lines marked [U] are unverified.

## What is here

| File | Role |
|---|---|
| `CMakeLists.txt` | One `qt_add_executable(olex2)` over the same source globs as the root build, minus unirun and the Windows-only `fsindex`/`zip`/`unzip`. Stage S1 has no `_PYTHON`, `_OPENSSL` or cctbx. |
| `CMakePresets.json` | One preset per ABI: `android-arm64`, `android-armv7`, `android-x86_64`, `android-x86`. Release (`-O3 -DNDEBUG`), Qt 6.11.1 Android kits, NDK 27.2.12479018, API 28, 6 jobs, build trees in `D:/Android/build/olex2-<preset>`. One APK per ABI, no fat APK. |
| `cmake/abi_flags.cmake` | Per-ABI code generation (`OLX_TUNE`, default ON, also adds ThinLTO): x86_64 `-march=x86-64-v2`, x86 `-mssse3`, armeabi-v7a `-mthumb -mfpu=neon` (no VFPv4: Cortex-A9), arm64-v8a the armv8-a baseline. Anything beyond needs runtime dispatch. `build_deps.py` applies the same file to the dependencies. |
| `build_deps.py` | Builds the GUI base libraries per ABI into `D:/Android/prefix/<abi>`: wxWidgets 3.3.3 (wxQt, static, GL on), gl4es 1.1.6 (`libGL.a`) and ptitSeb GLU (`libGLU.a`), with `patches/<name>/*.patch` applied. |
| `cmake/deps.cmake` | Imports those prebuilt libraries (`OLX_ANDROID_PREFIX`) as `gl4es_gl`, `olx_glu` and `wxWidgets::*`. wx does not export `wxBUILD_DEBUG_LEVEL`; the file looks for `wxTheAssertHandler` in the wx archive and sets `wxDEBUG_LEVEL=0` when it is missing, or Olex2 fails to link (`undefined symbol: wxOnAssert`). |
| `tests/glprobe` | Small wx GL test APK: fixed-function drawing through gl4es in a `wxGLCanvas`, and a log of every touch and gesture event. |
| `src/android_main.cpp` | `main()`. It copies Java's environment (`Os.environ`) into the native one, extracts `assets/olex2` once per content stamp, sets the `OLEX2_*` environment, then calls `wxEntry`. `olex/xglapp.cpp` uses `IMPLEMENT_APP_NO_MAIN` on Android. The copy matters under a native bridge (an ARM APK on an x86 emulator or Chromebook): Qt's loader sets `QT_PLUGIN_PATH` and friends in the host libc only, and Qt then aborts with "Could not find the Qt platform plugin". |
| `AndroidManifest.xml` | `org.olex2.android`, landscape, `extractNativeLibs`, INTERNET, GLES 2. |
| `assemble_assets.py` | Copies an allowlist of files from an installed rundir (read only) plus `rundir-overlay/`. It refuses keys, AC7 files, binaries and Python. `--stale <file>` deletes that file when the content stamp changes; the build uses it so androiddeployqt repackages after an asset change. |
| `rundir-overlay/custom.xld` | Replaces the `reap` macro: the stock one calls `spy.*`, and without Python every load ends in an error. Remove it when Python arrives (S2). |
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

- Qt 6.11.1 `android_arm64_v8a`, `android_armv7`, `android_x86_64`, `android_x86` and `mingw_64` (host tools) under `C:/Qt/6.11.1`.
- CMake 3.30 and Ninja under `C:/Qt/Tools`.
- Android SDK under `D:/Android/sdk`, with platform 36, build-tools 36.0.0 and NDK 27.2.12479018.
- JDK 17 under `D:/Android/jdk17`.
- The AVD `olex2_tablet` (Pixel Tablet, android-36 google_apis x86_64; runs arm64 APKs through ndk_translation).
- The AVD `olex2_arm32` (android-30 google_apis x86, 2560x1600; runs armeabi-v7a through ndk_translation).
- On the Mac: the AVD `olex2_tablet_arm64` (arm64 image, native).

Unpacked under `D:/Android/src` (`--src`) and built by `build_deps.py`:

| Package | Official source | SHA256 | Licence |
|---|---|---|---|
| wxWidgets 3.3.3 | https://github.com/wxWidgets/wxWidgets/releases/download/v3.3.3/wxWidgets-3.3.3.tar.bz2 | `81b09d6dd9f1ed9301f8c55a968a488d0491f264dc2bab19a7e407ac67009482` | wxWindows |
| gl4es 1.1.6 | https://github.com/ptitSeb/gl4es/archive/v1.1.6.tar.gz | `dca1d897e492a0cb163a3390f273fbd4cc7ab2367d236d93dc2b321ce108ed5c` | MIT |
| ptitSeb GLU 2fed2bda | https://github.com/ptitSeb/GLU/archive/2fed2bda.tar.gz | `8a016d32fc1fed742f10ba8e4bc32151598f6273a0dbabec15ba47c44151c879` | SGI Free B |

```bash
cd D:\git\olex2-android; python android/build_deps.py
```

Without `--abi` it builds all four ABIs. wx is built with
`wxBUILD_DEBUG_LEVEL=0` (no asserts). The 64-bit prefixes on FLOWOFFICE
still date from before that change (debug level 1); `deps.cmake` copes with
both, but rebuild them for consistency and speed.

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

Run these in PowerShell. Replace `x86_64` with `arm64`, `armv7` or `x86`.

```bash
cd D:\git\olex2-android\android; $env:JAVA_HOME = 'D:/Android/jdk17'; cmake --preset android-x86_64
```

```bash
cd D:\git\olex2-android\android; $env:JAVA_HOME = 'D:/Android/jdk17'; cmake --build --preset android-x86_64
```

The code is a Release build; the package is debug-signed
(`QT_ANDROID_DEPLOYMENT_TYPE=Debug`) so it installs directly:
`D:/Android/build/olex2-android-x86_64/android-build/build/outputs/apk/debug/android-build-debug.apk`.
Gradle strips the `.so` files. `-DOLX_TUNE=OFF` gives the untuned baseline.

The incremental packager leaves holes in a repackaged APK (72 MB instead of
60 MB). For a release-size APK delete
`android-build/build/outputs/apk/debug/android-build-debug.apk` and
`android-build/olex2.apk` in the build tree, then build again.

Assets come from `-DOLEX2_RUNDIR=<path>`, default `D:/devel/rundir-py3`. That
directory is only read. A dry run copies about 529 files (31 MB):

```bash
python D:\git\olex2-android\android\assemble_assets.py --self-test
```

## Emulator (x86_64)

gl4es needs a GLES2 GPU: the AVD uses `hw.gpu.mode = host`.

```bash
$env:ANDROID_AVD_HOME = 'D:/Android/avd'; $env:ANDROID_SDK_ROOT = 'D:/Android/sdk'; D:\Android\sdk\emulator\emulator.exe -avd olex2_tablet -no-snapshot-save
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

## Results (S1, emulators)

Debug-signed, release-compiled APKs, clean-packaged. Start-up is `am start`
to the `<pid>.ready` file `TMainForm::StartupInit` writes (first start
extracts the assets). Frame time is `1000 / fps()`, each `fps()` the mean of
10 draws of sucrose, read by an xld macro (`user_onstartup` in
`<config>/custom.xld`) because `adb input` keystrokes do not reach Olex2.
Emulator numbers on FLOWOFFICE and the Mac; real hardware will differ.

| APK | Size | Device | First start | Warm start (n = 5) | Frame time (n = 5) | Touch |
|---|---|---|---|---|---|---|
| x86_64 tuned | 60 MB | olex2_tablet, native | 4.8 s | 4.3-5.7 s | 28.8 ms (34.7 fps) | tap select, double tap, drag rotate, pinch, twist, long-press menu |
| x86_64 `OLX_TUNE=OFF` | 61 MB | olex2_tablet, native | 6.2 s | 4.4-5.4 s | 31.6 ms (31.7 fps) | not repeated |
| arm64-v8a | 59 MB | olex2_tablet, ndk_translation | 9.6 s | 8.6-11.7 s | 41-44 ms | tap select, double tap clear |
| arm64-v8a | 59 MB | Mac `olex2_tablet_arm64`, native (Apple Silicon) | 2.8 s | 1.6-1.8 s | 9.4-9.7 ms (103-106 fps; first sample 12.7 ms) | start-up screenshot only |
| x86 | 62 MB | olex2_arm32, native | 4.3 s | 3.3-3.6 s | 27-45 ms (mean 30.8 fps) | tap select, drag rotate, long-press menu; no `uinput` on API 30, so no pinch |
| armeabi-v7a | 54 MB | olex2_arm32, ndk_translation | 10.1 s | 9.5-9.7 s | - | crashes in the translator, see below |

Tuning gains about 3 fps (9 %, 95 % interval about +-1.1 fps) on x86_64.

## Known open points (S1)

- armeabi-v7a is untested on hardware. Under the API-30 x86 image's
  ndk_translation it dies about 0.1 s after start-up, tuned or untuned:
  `CHECK failed` in `ndk_translation/base/mmap_posix.cc` (an mmap for its own
  translation tables fails) or a SIGSEGV inside `ExecuteGuest`, never in the
  guest code. A 32-bit translator process runs out of address space; a real
  ARMv7 device runs the code natively.
- On some starts the HTML panel comes up on the right instead of the left,
  which moves the molecule on screen.

- wxQt on Android is not an officially supported wx target [U]. The first
  experiment is wx's `samples/opengl/cube`, built with these presets.
- Olex2 without `_PYTHON` has never been built [U]. The panel GUI is generated
  partly by Python. S1 aims for the 3-D view and the macro console; Python
  (CPython for Android) comes in S2.
- A wxQt top-level frame on Android stays at 400x250 DIP unless `Maximize()`d.
- After a resize Qt recreates the canvas FBO and leaves texture 0 bound on the
  active unit; `gl4es_resyncState` does not restore texture bindings.
