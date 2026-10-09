# Legacy-line APK (Qt 5.15.2, NDK r21e, minSdk 21), one ABI per call:
#   pwsh -File android/build_legacy.ps1 -Preset legacy-armv7 [-TargetSdk 30]
# Needs build_deps.py --legacy --abi <abi> first and, unless the preset is
# *-nopy, the CPython 3.13 + cctbx + NoSpherA2 payload in D:/Android/stage-legacy.
# Release native code, the APK signed with the SDK debug key, copied to
# D:/Android/stage-legacy/apk/olex2-legacy-<abi>[-nopy].apk.
param([Parameter(Mandatory)][ValidateSet('legacy-armv7', 'legacy-arm64', 'legacy-x86',
        'legacy-x86_64', 'legacy-armv7-nopy')][string]$Preset,
      [int]$TargetSdk = 30, [string]$OutDir = 'D:/Android/stage-legacy/apk')
$ErrorActionPreference = 'Stop'
$cmake = 'C:/Qt/Tools/CMake_64/bin/cmake.exe'
$qt = 'D:/Android/Qt5/5.15.2/android'
$ndk = 'D:/Android/sdk/ndk/21.4.7075529'
$env:JAVA_HOME = 'D:/Android/jdk17'
$env:ANDROID_SDK_ROOT = 'D:/Android/sdk'
$abi = @{ 'legacy-armv7' = 'armeabi-v7a'; 'legacy-arm64' = 'arm64-v8a'; 'legacy-x86' = 'x86';
          'legacy-x86_64' = 'x86_64'; 'legacy-armv7-nopy' = 'armeabi-v7a' }[$Preset]
$bd = "D:/Android/build/olex2-$Preset"
Push-Location $PSScriptRoot
& $cmake --preset $Preset
if ($LASTEXITCODE) { Pop-Location; throw "configure failed" }
& $cmake --build --preset $Preset
$rc = $LASTEXITCODE; Pop-Location
if ($rc) { throw "build failed" }
# Qt 5.15.2's json template drops the min/target SDK keys androiddeployqt reads
$js = "$bd/android_deployment_settings.json"
$j = Get-Content $js -Raw | ConvertFrom-Json
$j | Add-Member -Force 'android-min-sdk-version' '21'
$j | Add-Member -Force 'android-target-sdk-version' "$TargetSdk"
$j | ConvertTo-Json | Set-Content $js
# Pass 1: androiddeployqt lays out android-build (Qt libs, manifest, assets,
# gradle.properties); its own gradle run then stops on the
# android.bundle.enableUncompressedNativeLibs line AGP 8.1 removed. Pass 2:
# drop that line and run gradle (release, debug key) here.
$ab = "$bd/android-build"
Remove-Item -Recurse -Force "$ab/assets", "$ab/build" -ErrorAction SilentlyContinue
& "$qt/bin/androiddeployqt.exe" --input $js --output $ab --android-platform android-36 `
  --jdk $env:JAVA_HOME --gradle --release
$gp = "$ab/gradle.properties"
if (-not (Select-String -Quiet 'qtMinSdkVersion' $gp)) { throw "androiddeployqt failed before gradle" }
# Blank lines go too: androiddeployqt doubles them on every run (5 M lines
# after ~22 builds, minutes of Get-Content here)
(Get-Content $gp) -notmatch '^(android\.bundle\.enableUncompressedNativeLibs|\s*$)' | Set-Content $gp
# Qt 5.15.2's prebuilt .so files keep .symtab and .debug_*: strip them. Not
# the payload's (package/libs: patchelf'ed, stripped already, llvm-strip
# would misalign their segments) and not libolex2 (linked with -s).
$keep = @(Get-ChildItem "$bd/package/libs/$abi/*.so" -ErrorAction SilentlyContinue |
  ForEach-Object Name) + "libolex2_$abi.so"
Get-ChildItem "$ab/libs/$abi/*.so" | Where-Object { $keep -notcontains $_.Name } | ForEach-Object {
  & "$ndk/toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-strip.exe" --strip-all $_.FullName
  if ($LASTEXITCODE) { throw "strip failed: $($_.Name)" }
}
Push-Location $ab
& ./gradlew.bat --no-daemon assembleRelease
$rc = $LASTEXITCODE; Pop-Location
if ($rc) { throw "gradle failed" }
New-Item -ItemType Directory -Force $OutDir | Out-Null
$name = "olex2-legacy-$abi" + $(if ($Preset -like '*-nopy') { '-nopy' } else { '' }) + '.apk'
Copy-Item "$ab/build/outputs/apk/release/android-build-release.apk" "$OutDir/$name"
Get-Item "$OutDir/$name" | Select-Object FullName, Length
