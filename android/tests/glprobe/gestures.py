"""Write uinput scripts that replay multi-touch gestures on an emulator or
device. /system/bin/uinput (Android 12+) registers a virtual multi-touch
screen through /dev/uinput, which the adb shell user may open (group uhid);
sendevent on the emulator's own touch device is refused by SELinux.

  python gestures.py <outdir> [--w 2560 --h 1600]
  adb push <outdir>/g_pinch.json /data/local/tmp/
  adb shell uinput /data/local/tmp/g_pinch.json

The axes span the display in pixels, so the values are screen pixels.
"""
import argparse, json, math
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("out")
ap.add_argument("--w", type=int, default=2560)
ap.add_argument("--h", type=int, default=1600)
a = ap.parse_args()
EV_SYN, EV_KEY, EV_ABS, BTN_TOUCH = 0, 1, 3, 0x14a
SLOT, X, Y, TID = 0x2f, 0x35, 0x36, 0x39
# uinput ioctl numbers as the tool names them (UI_SET_EVBIT = 100, ...)
REGISTER = {
    "id": 1, "command": "register", "name": "glprobe touch", "vid": 0x18d1,
    "pid": 0x7a01, "bus": "usb",
    "configuration": [
        {"type": 100, "data": [EV_KEY, EV_ABS]},
        {"type": 101, "data": [BTN_TOUCH]},
        {"type": 103, "data": [SLOT, X, Y, TID]},
        {"type": 110, "data": [1]}],  # INPUT_PROP_DIRECT: a touch screen
    "abs_info": [
        {"code": SLOT, "info": {"value": 0, "minimum": 0, "maximum": 9}},
        {"code": X, "info": {"value": 0, "minimum": 0, "maximum": a.w - 1}},
        {"code": Y, "info": {"value": 0, "minimum": 0, "maximum": a.h - 1}},
        {"code": TID, "info": {"value": 0, "minimum": 0, "maximum": 65535}}]}


def script(frames, holds):
    """frames: list of {slot: (x, y) or None}; None lifts that finger.
    holds[i]: milliseconds to wait after frame i."""
    cmds = [REGISTER, {"id": 1, "command": "delay", "duration": 1500}]
    down = set()
    for fr, hold in zip(frames, holds):
        ev = []
        for s, p in fr.items():
            ev += [EV_ABS, SLOT, s]
            if p is None:
                ev += [EV_ABS, TID, -1]
                down.discard(s)
                continue
            if s not in down:
                ev += [EV_ABS, TID, 100 + s]
                if not down:
                    ev += [EV_KEY, BTN_TOUCH, 1]
                down.add(s)
            ev += [EV_ABS, X, round(p[0]), EV_ABS, Y, round(p[1])]
        if not down:
            ev += [EV_KEY, BTN_TOUCH, 0]
        ev += [EV_SYN, 0, 0]
        cmds.append({"id": 1, "command": "inject", "events": ev})
        cmds.append({"id": 1, "command": "delay", "duration": hold})
    cmds.append({"id": 1, "command": "delay", "duration": 500})
    return "\n".join(json.dumps(c) for c in cmds) + "\n"


cx, cy, n = a.w / 2, a.h / 2 + 100, 20
g = {}
# pinch out: finger distance 300 -> 900 px, horizontal, about the centre
g["pinch"] = [{0: (cx - r / 2, cy), 1: (cx + r / 2, cy)}
              for r in (300 + 600 * i / n for i in range(n + 1))]
# rotate: 600 px apart, counter-clockwise on screen by 60 degrees
g["rotate"] = [{0: (cx - 300 * math.cos(t), cy + 300 * math.sin(t)),
                1: (cx + 300 * math.cos(t), cy - 300 * math.sin(t))}
               for t in (math.radians(60) * i / n for i in range(n + 1))]
# two-finger pan: 400 px right, 200 px down
g["pan2"] = [{0: (cx - 150 + 400 * i / n, cy + 200 * i / n),
              1: (cx + 150 + 400 * i / n, cy + 200 * i / n)} for i in range(n + 1)]
# one-finger long press, held 1.5 s
g["longpress"] = [{0: (cx - 400, cy - 200)}]
holds = {"longpress": [1500]}

Path(a.out).mkdir(parents=True, exist_ok=True)
for k, fr in g.items():
    fr = fr + [{s: None for s in fr[0]}]
    h = holds.get(k, [30] * (len(fr) - 1)) + [30]
    Path(a.out, f"g_{k}.json").write_text(script(fr, h), newline="\n")
    print(k, len(fr), "frames")
