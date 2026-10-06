# Builds the glprobe APK for one ABI (debug-signed, installs directly):
#   pwsh -File android/tests/glprobe/build.ps1 -Abi x86_64 [-SelfInit] [-BuildDir <dir>]
param([ValidateSet('x86_64', 'arm64-v8a')][string]$Abi = 'x86_64',
      [switch]$SelfInit, [string]$BuildDir = '')
$ErrorActionPreference = 'Stop'
$env:JAVA_HOME = 'D:/Android/jdk17'
$env:ANDROID_SDK_ROOT = 'D:/Android/sdk'
$qtabi = $Abi -replace '-', '_'
if (-not $BuildDir) { $BuildDir = "D:/Android/build/glprobe-$Abi" + $(if ($SelfInit) { '-selfinit' } else { '' }) }
& C:/Qt/Tools/CMake_64/bin/cmake.exe -S $PSScriptRoot -B $BuildDir -G Ninja `
  -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/Ninja/ninja.exe -DCMAKE_BUILD_TYPE=Debug `
  "-DCMAKE_TOOLCHAIN_FILE=C:/Qt/6.11.1/android_$qtabi/lib/cmake/Qt6/qt.toolchain.cmake" `
  -DQT_HOST_PATH=C:/Qt/6.11.1/mingw_64 "-DANDROID_ABI=$Abi" -DANDROID_PLATFORM=android-28 `
  -DANDROID_STL=c++_shared -DANDROID_SDK_ROOT=D:/Android/sdk `
  -DANDROID_NDK_ROOT=D:/Android/sdk/ndk/27.2.12479018 `
  "-DGLPROBE_SELF_INIT=$(if ($SelfInit) { 'ON' } else { 'OFF' })"
if ($LASTEXITCODE) { throw "configure failed" }
& C:/Qt/Tools/CMake_64/bin/cmake.exe --build $BuildDir --target apk -j 6
if ($LASTEXITCODE) { throw "build failed" }
Get-ChildItem -Recurse "$BuildDir/android-build/build/outputs/apk" -Filter *.apk | Select-Object FullName, Length
