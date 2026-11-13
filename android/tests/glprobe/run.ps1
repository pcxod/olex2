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
# -W: ActivityManager's launch timing (TotalTime, ms) into <Out>_amstart.txt
A shell am start -W -n org.olex2.glprobe/org.qtproject.qt.android.bindings.QtActivity > "${Out}_amstart.txt"
Get-Content "${Out}_amstart.txt"
Start-Sleep 6
Shot 'start'
Start-Sleep 2
Shot 'start_plus2s'   # timer frames advance only if drawing outside paint shows
# the frame-time bench runs at the 6th timer tick; wait for its line (max 90 s)
for ($i = 0; $i -lt 45; $i++) {
  if (A logcat -d -s glprobe:* | Select-String 'bench:') { break }
  Start-Sleep 2
}
Shot 'after_bench'
A logcat -d -v time -s glprobe:* Qt:* libEGL:* AndroidRuntime:* DEBUG:* gl4es:* > "${Out}_logcat.txt"
Select-String -Path "${Out}_logcat.txt" -Pattern 'bench:|started'
A shell wm size
A shell wm density
