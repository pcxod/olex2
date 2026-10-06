# Installs and starts glprobe on the running emulator/device, then saves
# screenshots and the logcat lines to <Out>_*.png / <Out>_logcat.txt.
#   pwsh -File run.ps1 -Apk <apk> -Out D:/Android/evidence/A/x86_64_stock [-Serial emulator-5554]
param([string]$Apk, [string]$Out, [string]$Serial = 'emulator-5554')
$adb = 'D:/Android/sdk/platform-tools/adb.exe'
function A { & $adb -s $Serial @args }
function Shot([string]$name) {
  A shell screencap -p /sdcard/s.png | Out-Null
  A pull /sdcard/s.png "${Out}_$name.png" | Out-Null
}
A install -r -g $Apk
A shell am force-stop org.olex2.glprobe
A logcat -c
A shell am start -n org.olex2.glprobe/org.qtproject.qt.android.bindings.QtActivity
Start-Sleep 6
Shot 'start'
Start-Sleep 2
Shot 'start_plus2s'   # timer frames advance only if drawing outside paint shows
A logcat -d -v time -s glprobe:* Qt:* libEGL:* AndroidRuntime:* DEBUG:* gl4es:* > "${Out}_logcat.txt"
A shell wm size
A shell wm density
