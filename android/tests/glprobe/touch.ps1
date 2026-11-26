# Replays the touch set on a running glprobe and saves the glprobe logcat
# lines plus a screenshot per gesture to <Out>_touch_*.{txt,png}:
#   pwsh -File touch.ps1 -Out D:/Android/evidence/A/x86_64_stock [-Gestures <dir>]
# <dir> holds the g_*.json scripts from gestures.py (multi-touch via uinput).
param([string]$Out, [string]$Gestures = 'D:/Android/build/gestures',
      [string]$Serial = 'emulator-5554')
$adb = 'D:/Android/sdk/platform-tools/adb.exe'
function A { & $adb -s $Serial @args }
function Step([string]$name, [scriptblock]$act) {
  A logcat -c
  & $act
  Start-Sleep 2
  A logcat -d -v time -s glprobe:* > "${Out}_touch_$name.txt"
  A shell screencap -p /sdcard/s.png | Out-Null
  A pull /sdcard/s.png "${Out}_touch_$name.png" | Out-Null
  # the gesture boundaries and mouse buttons; the full stream is in the file
  "== $name"; Get-Content "${Out}_touch_$name.txt" |
    Select-String 'START|END|longpress|left_|right_|mode' | Select-Object -ExpandProperty Line
}
Get-ChildItem $Gestures -Filter "g_*.json" | ForEach-Object { A push $_.FullName /data/local/tmp/ | Out-Null }
Step 'tap'       { A shell input tap 640 900 }
Step 'swipe'     { A shell input swipe 640 900 1240 1100 600 }
Step 'dtap'      { A shell input tap 640 900; A shell input tap 640 900 }
Step 'longpress' { A shell uinput /data/local/tmp/g_longpress.json }
Step 'pinch'     { A shell uinput /data/local/tmp/g_pinch.json }
Step 'rotate'    { A shell uinput /data/local/tmp/g_rotate.json }
Step 'pan2'      { A shell uinput /data/local/tmp/g_pan2.json }
