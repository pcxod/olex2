# Builds the glprobe APK for one ABI (debug-signed, installs directly):
#   pwsh -File android/tests/glprobe/build.ps1 -Abi x86_64 [-SelfInit] [-BuildDir <dir>] [-Config Release]
# -Config Release optimises the native code; the package stays debug-signed
# (QT_ANDROID_DEPLOYMENT_TYPE=Debug) so it installs without a release keystore.
param([ValidateSet('x86_64', 'arm64-v8a', 'armeabi-v7a', 'x86')][string]$Abi = 'x86_64',
      [switch]$SelfInit, [string]$BuildDir = '',
      [ValidateSet('Debug', 'Release')][string]$Config = 'Debug', [int]$Jobs = 6)
$ErrorActionPreference = 'Stop'
$env:JAVA_HOME = 'D:/Android/jdk17'
$env:ANDROID_SDK_ROOT = 'D:/Android/sdk'
$kit = @{ 'x86_64' = 'android_x86_64'; 'arm64-v8a' = 'android_arm64_v8a';
          'armeabi-v7a' = 'android_armv7'; 'x86' = 'android_x86' }[$Abi]
if (-not $BuildDir) { $BuildDir = "D:/Android/build/glprobe-$Abi" + $(if ($SelfInit) { '-selfinit' } else { '' }) }
& C:/Qt/Tools/CMake_64/bin/cmake.exe -S $PSScriptRoot -B $BuildDir -G Ninja `
  -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/Ninja/ninja.exe "-DCMAKE_BUILD_TYPE=$Config" `
  -DQT_ANDROID_DEPLOYMENT_TYPE=Debug `
  "-DCMAKE_TOOLCHAIN_FILE=C:/Qt/6.11.1/$kit/lib/cmake/Qt6/qt.toolchain.cmake" `
  -DQT_HOST_PATH=C:/Qt/6.11.1/mingw_64 "-DANDROID_ABI=$Abi" -DANDROID_PLATFORM=android-28 `
  -DANDROID_STL=c++_shared -DANDROID_SDK_ROOT=D:/Android/sdk `
  -DANDROID_NDK_ROOT=D:/Android/sdk/ndk/27.2.12479018 `
  "-DGLPROBE_SELF_INIT=$(if ($SelfInit) { 'ON' } else { 'OFF' })"
if ($LASTEXITCODE) { throw "configure failed" }
& C:/Qt/Tools/CMake_64/bin/cmake.exe --build $BuildDir --target apk -j $Jobs
if ($LASTEXITCODE) { throw "build failed" }
Get-ChildItem -Recurse "$BuildDir/android-build/build/outputs/apk" -Filter *.apk | Select-Object FullName, Length
