# copy the static ptb builds into both payloads as bin/libptb.so
$ErrorActionPreference = 'Stop'
$out = (wsl -e bash -c 'echo ~/ptb-android/out').Trim()
foreach ($abi in 'armeabi-v7a', 'arm64-v8a', 'x86_64') {
  foreach ($st in 'D:/Android/stage-legacy', 'D:/Android/stage') {
    if (Test-Path "$st/$abi/bin") {
      $w = (wsl -e wslpath -w "$out/$abi/ptb").Trim()
      Copy-Item $w "$st/$abi/bin/libptb.so"
      "{0} {1:N1} MB" -f "$st/$abi/bin/libptb.so", ((Get-Item "$st/$abi/bin/libptb.so").Length / 1MB)
    }
  }
}
