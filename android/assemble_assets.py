"""Build assets/olex2 for the APK from an installed Olex2 rundir plus
android/rundir-overlay.

The rundir is only read. Only allowlisted paths are copied, and closed or
platform binaries are refused even when an allowlisted directory holds them.
files.txt lists every copied file after a content stamp line; android_main.cpp
extracts by that list, because the NDK asset API cannot list directories.

  python assemble_assets.py --rundir D:/devel/rundir-py3 \
      --overlay android/rundir-overlay --out <build>/package/assets/olex2
  python assemble_assets.py --self-test
"""
import argparse, fnmatch, hashlib, pathlib, shutil, sys, tempfile

# relative to the rundir; a directory is copied recursively
ALLOW = [
    "etc", "sample_data",
    "LICENCE.txt", "licence.rtf", "version.txt", "olex2.tag", "mirrors.txt",
    "macro.xld", "macrox.xld", "help.xld", "settings.xld", "symmlib.xld",
    "dictionary.txt", "ptablex.dat", "splash.jpg", "index.ind", "acidb.db",
    "gui.params", "params.phil", "metacif.phil",
]
# S1 runs without Python, so its scripts stay out with the binaries
REFUSE = ["*.key", "*.key_*", "*ac7*", "*.exe", "*.dll", "*.pyd", "*.so",
          "*.bak", "*.py", "*.pyc", "*.bat", "*.sh", ".git", ".svn",
          "__pycache__"]


def refused(rel):
    return any(fnmatch.fnmatch(part.lower(), p)
               for part in rel.parts for p in REFUSE)


def collect(rundir, overlay):
    """{relative path: source file}; the overlay wins over the rundir."""
    files = {}
    for name in ALLOW:
        p = rundir / name
        if p.is_file():
            files[pathlib.PurePosixPath(name)] = p
        elif p.is_dir():
            for f in sorted(p.rglob("*")):
                if f.is_file():
                    files[pathlib.PurePosixPath(f.relative_to(rundir).as_posix())] = f
        else:
            print("not in rundir, skipped:", name)
    if overlay is not None:
        for f in sorted(overlay.rglob("*")):
            if f.is_file():
                files[pathlib.PurePosixPath(f.relative_to(overlay).as_posix())] = f
    return {r: f for r, f in files.items() if not refused(r)}


def assemble(rundir, overlay, out):
    files = collect(rundir, overlay)
    if out.exists():
        shutil.rmtree(out)
    h = hashlib.sha256()
    total = 0
    for rel in sorted(files):
        dst = out / rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(files[rel], dst)
        data = dst.read_bytes()
        h.update(str(rel).encode() + b"\0" + hashlib.sha256(data).digest())
        total += len(data)
    lines = [h.hexdigest()[:16]] + [str(r) for r in sorted(files)]
    (out / "files.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("assets: %d files, %.1f MB -> %s" % (len(files), total / 1e6, out))
    return files


def self_test():
    with tempfile.TemporaryDirectory() as t:
        t = pathlib.Path(t)
        rd, ov, out = t / "rd", t / "ov", t / "out"
        for p in ["etc/gui/a.htm", "etc/bin/start.exe", "etc/s/x.py",
                  "AC7.key", "macro.xld", "olex2.exe", "util/u.htm",
                  "etc/.svn/entries"]:
            (rd / p).parent.mkdir(parents=True, exist_ok=True)
            (rd / p).write_text(p)
        (ov / "etc/gui").mkdir(parents=True)
        (ov / "etc/gui/a.htm").write_text("overlay")
        (ov / "android.options").write_text("gl_touch=true")
        files = assemble(rd, ov, out)
        got = sorted(str(r) for r in files)
        assert got == ["android.options", "etc/gui/a.htm", "macro.xld"], got
        assert (out / "etc/gui/a.htm").read_text() == "overlay"
        lst = (out / "files.txt").read_text().split()
        assert len(lst[0]) == 16 and lst[1:] == got, lst
    print("self-test passed")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rundir", type=pathlib.Path)
    ap.add_argument("--overlay", type=pathlib.Path)
    ap.add_argument("--out", type=pathlib.Path)
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args()
    if a.self_test:
        return self_test()
    if a.rundir is None or a.out is None:
        ap.error("--rundir and --out are required")
    if not a.rundir.is_dir():
        sys.exit("no rundir at %s" % a.rundir)
    assemble(a.rundir, a.overlay, a.out)


if __name__ == "__main__":
    main()
