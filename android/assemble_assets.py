"""Build assets/olex2 for the APK from an installed Olex2 rundir, overlays
and (S2/S3) the per-ABI Python/cctbx/NoSpherA2 payload.

The rundir is only read. Only allowlisted paths are copied, and closed or
platform binaries are refused even when an allowlisted directory holds them.
files.txt lists every asset after a content stamp line; android_main.cpp
extracts by that list, because the NDK asset API cannot list directories.

Shared libraries (lib*.so) of the payload go to --jnilibs instead: Android's
linker resolves DT_NEEDED by soname only from the APK's native library dir.
Python and cctbx extension modules stay assets, CPython dlopens them by path.

  python assemble_assets.py --rundir D:/devel/rundir-py3 \
      --overlay android/rundir-overlay --out <build>/package/assets/olex2 \
      [--payload D:/Android/stage/<abi> --jnilibs <build>/package/libs/<abi>]
  python assemble_assets.py --self-test
"""
import argparse, fnmatch, hashlib, pathlib, shutil, subprocess, sys, tempfile, zipfile

# relative to the rundir; a directory is copied recursively
ALLOW = [
    "etc", "sample_data", "util", "basis_sets",
    "LICENCE.txt", "licence.rtf", "version.txt", "olex2.tag", "mirrors.txt",
    "macro.xld", "macrox.xld", "help.xld", "settings.xld", "symmlib.xld",
    "dictionary.txt", "ptablex.dat", "splash.jpg", "index.ind", "acidb.db",
    "gui.params", "params.phil", "metacif.phil",
]
# closed modules (AC7/ACED ship as .pyc for another Python), keys, platform
# binaries and the zipped plugin copies
REFUSE = ["*.key", "*.key_*", "*ac7*", "*aced*", "*.exe", "*.dll", "*.pyd",
          "*.so", "*.bak", "*.pyc", "*.bat", "*.sh", "*.7z", "nosphera2.zip",
          ".*", "__pycache__"]  # .* also drops .gitignore (aapt does) and .token
# payload directory -> asset directory
PAYLOAD = {
    "python/lib/python3.14": "python/lib/python3.14",
    "cctbx/cctbx_build": "cctbx/cctbx_build",
    "cctbx/cctbx_sources": "cctbx/cctbx_sources",
    # NoSpherA2.py: OCC_DATA_PATH = <dir of the NoSpherA2 exe>/occ/share
    "share/occ": "occ/share",
}
# Qt's androiddeployqt ships its own, identical libc++_shared.so
SKIP_LIBS = {"libc++_shared.so"}
# (asset, old, new) source fixes for what Android lacks, applied to the copy
# only; each old text must occur exactly once (CRLF files: one-line olds)
PATCHES = [
    # cctbx is built without ccp4io (no iotbx_mtz_ext): MTZ is not read
    ("cctbx/cctbx_sources/iotbx/reflection_file_reader.py",
     "from iotbx import mtz\n",
     "try: from iotbx import mtz\n"
     "except ImportError: mtz = None  # Android: cctbx without ccp4io\n"),
    ("cctbx/cctbx_sources/iotbx/reflection_file_reader.py",
     "  try: content = mtz.object(file_name=file_name)\n  except RuntimeError: pass\n",
     "  try: content = mtz.object(file_name=file_name)\n"
     "  except (RuntimeError, AttributeError): pass\n"),
    # AC7 is closed bytecode and refused above: start without it
    ("util/pyUtil/initpy_funcs.py",
     'if "bad magic number" not in str(err):',
     'if "bad magic number" not in str(err) and err.name != "AC7":'),
]


def refused(rel):
    return any(fnmatch.fnmatch(part.lower(), p)
               for part in rel.parts for p in REFUSE)


def is_lib(name):
    return name.startswith("lib") and name.endswith(".so")


def asset_name(rel):
    """aapt decompresses *.gz assets: add a dash, android_main.cpp drops it
    (the CPython testbed does the same)"""
    s = str(rel)
    return s + "-" if s.endswith((".gz", "-")) else s


def tree(d):
    return [f for f in sorted(d.rglob("*")) if f.is_file()
            and not {"__pycache__", ".git", ".svn"} & set(f.parts)]


def wheel_overlay(whl, tmp):
    """a pure-Python wheel as an overlay of the payload's site-packages"""
    root = tmp / whl.stem
    zipfile.ZipFile(whl).extractall(root / "python/lib/python3.14/site-packages")
    return root


def collect(rundir, overlays, payload=None):
    """({relative path: source file}, {lib name: source file});
    overlays win over the payload, the payload over the rundir"""
    files, libs = {}, {}
    for name in ALLOW:
        p = rundir / name
        if p.is_file():
            files[pathlib.PurePosixPath(name)] = p
        elif p.is_dir():
            for f in tree(p):
                files[pathlib.PurePosixPath(f.relative_to(rundir).as_posix())] = f
        else:
            print("not in rundir, skipped:", name)
    files = {r: f for r, f in files.items() if not refused(r)}
    if payload is not None:
        for f in sorted((payload / "python/lib").glob("lib*.so")):
            libs[f.name] = f
        libs["libNoSpherA2.so"] = payload / "bin/libNoSpherA2.so"
        for src, dst in PAYLOAD.items():
            for f in tree(payload / src):
                if is_lib(f.name):
                    libs[f.name] = f
                else:
                    rel = f.relative_to(payload / src).as_posix()
                    files[pathlib.PurePosixPath(dst, rel)] = f
        for n in SKIP_LIBS:
            libs.pop(n, None)
        missing = [n for n, f in libs.items() if not f.is_file()]
        if missing:
            sys.exit("payload lacks %s" % missing)
    for ov in overlays:
        for f in tree(ov):
            files[pathlib.PurePosixPath(f.relative_to(ov).as_posix())] = f
    return files, libs


def apply_patches(out):
    for rel, old, new in PATCHES:
        p = out / rel
        if not p.is_file():  # S1: no payload
            continue
        s = p.read_bytes().decode("utf-8")
        if s.count(old) != 1:
            sys.exit("patch does not apply to %s: %r" % (rel, old))
        p.write_bytes(s.replace(old, new).encode("utf-8"))


def compile_pyc(out, python):
    """unchecked-hash pycs: no source stat on import, no first-start compile"""
    dirs = [str(out / d) for d in ("python", "util", "cctbx") if (out / d).is_dir()]
    if dirs:
        # syntax errors in py2-only test files are expected, hence no check
        subprocess.run([python, "-m", "compileall", "-q", "-j", "6",
                        "--invalidation-mode", "unchecked-hash",
                        "-s", str(out), "-p", "/"] + dirs,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def assemble(rundir, overlays, out, payload=None, jnilibs=None, python=None):
    files, libs = collect(rundir, overlays, payload)
    if out.exists():
        shutil.rmtree(out)
    for rel, src in files.items():
        dst = out / asset_name(rel)
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)
    apply_patches(out)
    if python:
        compile_pyc(out, python)
    # list what is in out, so the pycs are covered too
    h = hashlib.sha256()
    names, total = [], 0
    for f in tree(out) + sorted(out.rglob("__pycache__/*.pyc")):
        rel = f.relative_to(out).as_posix()
        data = f.read_bytes()
        h.update(rel.encode() + b"\0" + hashlib.sha256(data).digest())
        names.append(rel)
        total += len(data)
    # LF on Windows too: android_main.cpp splits on '\n' only
    (out / "files.txt").write_text("\n".join([h.hexdigest()[:16]] + names) + "\n",
                                   encoding="utf-8", newline="\n")
    print("assets: %d files, %.1f MB -> %s" % (len(names), total / 1e6, out))
    if jnilibs is not None:
        if jnilibs.exists():
            shutil.rmtree(jnilibs)
        jnilibs.mkdir(parents=True)
        for n, f in libs.items():
            shutil.copyfile(f, jnilibs / n)
        print("jniLibs: %d libraries -> %s" % (len(libs), jnilibs))
    return names, libs


def self_test():
    with tempfile.TemporaryDirectory() as t:
        t = pathlib.Path(t)
        rd, ov, pl, out, jl = t / "rd", t / "ov", t / "pl", t / "out", t / "jl"
        for p in ["etc/gui/a.htm", "etc/bin/start.exe", "util/s/x.py",
                  "util/ACED/a.pyc", "AC7.key", "macro.xld", "olex2.exe",
                  "etc/.svn/entries", "util/__pycache__/x.cpython-312.pyc",
                  "util/R/.token", "util/P/.gitignore"]:
            (rd / p).parent.mkdir(parents=True, exist_ok=True)
            (rd / p).write_text(p)
        for p in ["python/lib/libpython3.14.so", "bin/libNoSpherA2.so",
                  "python/lib/python3.14/os.py",
                  "python/lib/python3.14/lib-dynload/_ssl.cpython-314.so",
                  "cctbx/cctbx_build/lib/libcctbx.so",
                  "cctbx/cctbx_build/lib/libc++_shared.so",
                  "cctbx/cctbx_build/lib/scitbx_ext.so",
                  "cctbx/cctbx_sources/iotbx/d.dic.gz", "share/occ/basis/x"]:
            (pl / p).parent.mkdir(parents=True, exist_ok=True)
            (pl / p).write_text(p)
        (ov / "etc/gui").mkdir(parents=True)
        (ov / "etc/gui/a.htm").write_text("overlay")
        (ov / "android.options").write_text("gl_touch=true")
        with zipfile.ZipFile(t / "w-1.0-py3-none-any.whl", "w") as z:
            z.writestr("w/__init__.py", "")
        wh = wheel_overlay(t / "w-1.0-py3-none-any.whl", t)
        names, libs = assemble(rd, [wh, ov], out, pl, jl)
        assert names == [
            "android.options", "cctbx/cctbx_build/lib/scitbx_ext.so",
            "cctbx/cctbx_sources/iotbx/d.dic.gz-", "etc/gui/a.htm",
            "macro.xld", "occ/share/basis/x",
            "python/lib/python3.14/lib-dynload/_ssl.cpython-314.so",
            "python/lib/python3.14/os.py",
            "python/lib/python3.14/site-packages/w/__init__.py", "util/s/x.py"], names
        assert sorted(libs) == ["libNoSpherA2.so", "libcctbx.so",
                                "libpython3.14.so"], libs
        assert sorted(p.name for p in jl.iterdir()) == sorted(libs)
        assert (out / "etc/gui/a.htm").read_text() == "overlay"
        assert b"\r" not in (out / "files.txt").read_bytes()
        lst = (out / "files.txt").read_text().split()
        assert len(lst[0]) == 16 and lst[1:] == names, lst
    print("self-test passed")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rundir", type=pathlib.Path)
    ap.add_argument("--overlay", type=pathlib.Path, action="append", default=[],
                    help="repeatable; a later overlay wins")
    ap.add_argument("--out", type=pathlib.Path)
    ap.add_argument("--payload", type=pathlib.Path,
                    help="D:/Android/stage/<abi>: python, cctbx, NoSpherA2, occ")
    ap.add_argument("--jnilibs", type=pathlib.Path,
                    help="where the payload's lib*.so go (package/libs/<abi>)")
    ap.add_argument("--wheel", type=pathlib.Path, action="append", default=[],
                    help="repeatable; pure-Python wheel added to site-packages")
    ap.add_argument("--pyc", help="Python 3.14 to precompile the .py with")
    ap.add_argument("--stale", type=pathlib.Path,
                    help="file to delete when the assets change (the APK: "
                         "androiddeployqt's depfile does not list them)")
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args()
    if a.self_test:
        return self_test()
    if a.rundir is None or a.out is None:
        ap.error("--rundir and --out are required")
    if not a.rundir.is_dir():
        sys.exit("no rundir at %s" % a.rundir)
    lst = a.out / "files.txt"
    old = lst.read_text().splitlines()[0] if lst.exists() else None
    with tempfile.TemporaryDirectory() as t:
        wheels = [wheel_overlay(w, pathlib.Path(t)) for w in a.wheel]
        assemble(a.rundir, wheels + a.overlay, a.out, a.payload, a.jnilibs, a.pyc)
    if a.stale and old != lst.read_text().splitlines()[0]:
        a.stale.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
