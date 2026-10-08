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
import argparse, fnmatch, hashlib, pathlib, runpy, shutil, struct, subprocess, sys, tempfile, zipfile

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
    # Exynos 7870 tablet: threaded normal-equation builds pay off from ~10x
    # less work than desktop (ZP2 5404x340: build 0.83 -> 0.24 s, Flack
    # 2.55 -> 0.77 s; never slower on small cells), so lower both thresholds
    ("cctbx/cctbx_sources/smtbx/refinement/least_squares.py",
     "blas_2_parallel_work_threshold = 2e5",
     "blas_2_parallel_work_threshold = 2e4  # Android: /10, see assemble_assets.py"),
    ("cctbx/cctbx_sources/smtbx/refinement/least_squares.py",
     "blas_3_parallel_work_threshold = 2e6", "blas_3_parallel_work_threshold = 2e5"),
    # Android parks idle cores offline, so cpu_count() says 4 of the tablet's 8 and the
    # default refinement gets 3 threads; ZP2 Flack 1.21 -> 0.83 s, LS build 0.47 -> 0.25 s
    # at 6 (= 3/4 of the possible cores), no gain at 8
    ("util/pyUtil/olexFunctions.py",
     "        v = max(1, int(os.cpu_count() *3/4))",
     "        v = max(1, int((int(open('/sys/devices/system/cpu/possible').read().strip()"
     ".split('-')[-1]) + 1) *3/4))"),
    # no scipy on Android: refinement.py needs it only for one LU (lazy import,
    # fails there alone), cubes_maps.py only for inv/det, which numpy has. These
    # replace the whole-file copies in the stage overlay that went stale on rebase
    ("util/pyUtil/CctbxLib/refinement.py",
     "import scipy.linalg", "# import scipy.linalg  # Android: lazy, see assemble_assets.py"),
    ("util/pyUtil/CctbxLib/refinement.py",
     "l,u = scipy.linalg.lu(lineareq, permute_l=True)",
     "import scipy.linalg; l,u = scipy.linalg.lu(lineareq, permute_l=True)"),
    ("util/pyUtil/NoSpherA2/cubes_maps.py",
     "from scipy import linalg",
     "from numpy import linalg  # Android has no scipy; only inv/det are used here"),
    # SHELX weight optimiser: each bin's 9x9 a/b grid in one numpy expression
    # instead of 81 closure calls (ZP2 776 -> 232 ms on the tablet); a, b and all
    # grid cells bitwise equal on armv7. Proposed for cctbx, carried here until then
    ("cctbx/cctbx_sources/smtbx/refinement/weighting_schemes.py", """\
    def mainstream_shelx_weighting_compute_chi_sq(fo_sq, fc_sq, a,b):
      weighting.a = a
      weighting.b = b
      weights = weighting(
        fo_sq.data(), fo_sq.sigmas(), fc_sq.data(), fo_sq.indices(), scale_factor)
      return (flex.sum(
        weights * flex.pow2(fo_sq.data() - scale_factor * fc_sq.data())))

""", ""),
    ("cctbx/cctbx_sources/smtbx/refinement/weighting_schemes.py", """\
    # search on a 9x9 grid to determine best values of a and b
    gridding = flex.grid(9,9)
    while (a_step > 1e-4 and b_step > 5e-3):
      tmp = flex.double(gridding, 0)
      binned_chi_sq = [tmp.deep_copy() for i in range(n_bins)]
      start_a = max(start_a, 4*a_step) - 4*a_step
      start_b = max(start_b, 4*b_step) - 4*b_step
      for i_bin in range(n_bins):
        sel = flex.size_t_range(bin_limits[i_bin], bin_limits[i_bin+1])
        fc2 = fc_sq.select(sel)
        fo2 = fo_sq.select(sel)
        b = start_b
        for j in range(9):
          a = start_a
          b += b_step
          for k in range(9):
            a += a_step
            binned_chi_sq[i_bin][j,k] += mainstream_shelx_weighting_compute_chi_sq(fo2, fc2, a, b)
""", """\
    # the a,b-independent parts of each bin: sigma^2, P and (Fo^2 - K Fc^2)^2
    bins = []
    for i_bin in range(n_bins):
      sel = flex.size_t_range(bin_limits[i_bin], bin_limits[i_bin+1])
      fo = fo_sq.data().select(sel).as_numpy_array()
      s = fo_sq.sigmas().select(sel).as_numpy_array()
      fc = fc_sq.data().select(sel).as_numpy_array()
      p = (np.maximum(fo, 0.) + 2 * scale_factor * fc) / 3.
      bins.append((s * s, p, (fo - scale_factor * fc)**2))

    # search on a 9x9 grid to determine best values of a and b
    gridding = flex.grid(9,9)
    while (a_step > 1e-4 and b_step > 5e-3):
      start_a = max(start_a, 4*a_step) - 4*a_step
      start_b = max(start_b, 4*b_step) - 4*b_step
      # accumulated step by step as before, so every grid value is bitwise the same
      a_grid, b_grid = [], []
      a, b = start_a, start_b
      for k in range(9):
        a += a_step
        b += b_step
        a_grid.append(a)
        b_grid.append(b)
      a_grid, b_grid = np.array(a_grid), np.array(b_grid)
      binned_chi_sq = []
      for s2, p, r2 in bins:
        # w[j,k] = 1/(s^2 + (a_k P)^2 + b_j K P) in the operation order of
        # mainstream_shelx_weighting::compute; cumsum keeps flex.sum's sequential order
        ap = a_grid[:, None] * p
        w = 1. / ((s2 + ap * ap)[None, :, :] + ((b_grid * scale_factor)[:, None] * p)[:, None, :])
        chi = np.cumsum(w * r2, axis=2)[:, :, -1] if p.size else np.zeros((9, 9))
        g = flex.double(np.ascontiguousarray(chi).ravel())
        g.reshape(gridding)
        binned_chi_sq.append(g)
"""),
    # CONF dihedrals (generate_all): python copies of the pair table and the sites instead of
    # boost lookups in the 4-deep loop; ZP2 0.45 -> 0.19 s, sucrose 0.47 -> 0.21 s per refine on
    # the tablet, every dihedral, rt_mx and su bitwise equal. Proposed for cctbx, carried here until then
    ("cctbx/cctbx_sources/cctbx/crystal/__init__.py", """\
    table = self.pair_asu_table.table()
    for i_seq,i_asu_dict in enumerate(table):
      rt_mx_i_inv = asu_mappings.get_rt_mx(i_seq, 0).inverse()
      i_site_frac = self.sites_frac[i_seq]
      for j_seq,j_sym_groups in i_asu_dict.items():
        rt_mx_j0_inv = asu_mappings.get_rt_mx(j_seq, 0).inverse()
        for j_sym_group in j_sym_groups:
          for j_sym_idx,j_sym in enumerate(j_sym_group):
            rt_mx_j = rt_mx_i_inv.multiply(asu_mappings.get_rt_mx(j_seq, j_sym))
            j_site_frac = rt_mx_j * self.sites_frac[j_seq]
            if j_site_frac == i_site_frac:
              continue
            for k_seq, k_sym_groups in table[j_seq].items():
              if (self.conformer_indices is not None and
                  self.conformer_indices[i_seq] !=
                  self.conformer_indices[k_seq]):
                continue
              rt_mx_k0_inv = asu_mappings.get_rt_mx(k_seq, 0).inverse()
              rt_mx_kj = rt_mx_j.multiply(rt_mx_j0_inv)
              for k_sym_group in k_sym_groups:
                for k_sym_idx,k_sym in enumerate(k_sym_group):
                  rt_mx_k = rt_mx_kj.multiply(asu_mappings.get_rt_mx(k_seq, k_sym))
                  k_site_frac = rt_mx_k *  self.sites_frac[k_seq]
                  if k_site_frac in (i_site_frac, j_site_frac):
                    continue
                  if unit_cell.distance(j_site_frac, k_site_frac) > self.max_d:
                    continue
                  if unit_cell.angle(i_site_frac, j_site_frac, k_site_frac) > self.max_angle:
                    continue
                  rt_mx_lk = rt_mx_k.multiply(rt_mx_k0_inv)
                  for l_seq, l_sym_groups in table[k_seq].items():
                    if (self.conformer_indices is not None and
                        self.conformer_indices[j_seq] !=
                        self.conformer_indices[l_seq]):
                      continue
                    for l_sym_group in l_sym_groups:
                      for l_sym_idx, l_sym in enumerate(l_sym_group):
                        rt_mx_l = rt_mx_lk.multiply(asu_mappings.get_rt_mx(l_seq, l_sym))
                        l_site_frac = rt_mx_l *  self.sites_frac[l_seq]
""", """\
    # python copies of the pair table and the sites: the loops below re-fetched them
    # through boost on every pass (2.4x faster on an A53, same dihedrals bit for bit)
    table = [[(j, [list(g) for g in groups]) for j, groups in d.items()]
             for d in self.pair_asu_table.table()]
    sites_frac = list(self.sites_frac)
    for i_seq,i_asu_dict in enumerate(table):
      rt_mx_i_inv = asu_mappings.get_rt_mx(i_seq, 0).inverse()
      i_site_frac = sites_frac[i_seq]
      for j_seq,j_sym_groups in i_asu_dict:
        rt_mx_j0_inv = asu_mappings.get_rt_mx(j_seq, 0).inverse()
        for j_sym_group in j_sym_groups:
          for j_sym_idx,j_sym in enumerate(j_sym_group):
            rt_mx_j = rt_mx_i_inv.multiply(asu_mappings.get_rt_mx(j_seq, j_sym))
            j_site_frac = rt_mx_j * sites_frac[j_seq]
            if j_site_frac == i_site_frac:
              continue
            for k_seq, k_sym_groups in table[j_seq]:
              if (self.conformer_indices is not None and
                  self.conformer_indices[i_seq] !=
                  self.conformer_indices[k_seq]):
                continue
              rt_mx_k0_inv = asu_mappings.get_rt_mx(k_seq, 0).inverse()
              rt_mx_kj = rt_mx_j.multiply(rt_mx_j0_inv)
              for k_sym_group in k_sym_groups:
                for k_sym_idx,k_sym in enumerate(k_sym_group):
                  rt_mx_k = rt_mx_kj.multiply(asu_mappings.get_rt_mx(k_seq, k_sym))
                  k_site_frac = rt_mx_k *  sites_frac[k_seq]
                  if k_site_frac in (i_site_frac, j_site_frac):
                    continue
                  if unit_cell.distance(j_site_frac, k_site_frac) > self.max_d:
                    continue
                  if unit_cell.angle(i_site_frac, j_site_frac, k_site_frac) > self.max_angle:
                    continue
                  rt_mx_lk = rt_mx_k.multiply(rt_mx_k0_inv)
                  for l_seq, l_sym_groups in table[k_seq]:
                    if (self.conformer_indices is not None and
                        self.conformer_indices[j_seq] !=
                        self.conformer_indices[l_seq]):
                      continue
                    for l_sym_group in l_sym_groups:
                      for l_sym_idx, l_sym in enumerate(l_sym_group):
                        rt_mx_l = rt_mx_lk.multiply(asu_mappings.get_rt_mx(l_seq, l_sym))
                        l_site_frac = rt_mx_l *  sites_frac[l_seq]
"""),
]
# tablet-sized GUI: layout patches to etc/gui and util/pyUtil
PATCHES += runpy.run_path(str(pathlib.Path(__file__).with_name("gui_tablet_patches.py")))["PATCHES"]


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


def pylib(payload):
    """python/lib/python3.X of the payload: 3.14 on the API 28 line, 3.13 on
    the API 21 (Qt 5) line"""
    d = sorted((payload / "python/lib").glob("python3.*"))
    if len(d) != 1:
        sys.exit("expected one python/lib/python3.* in %s, found %s" % (payload, d))
    return d[0].relative_to(payload).as_posix()


def wheel_overlay(whl, tmp, payload):
    """a pure-Python wheel as an overlay of the payload's site-packages"""
    root = tmp / whl.stem
    zipfile.ZipFile(whl).extractall(root / pylib(payload) / "site-packages")
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
        # pTB, a static glibc executable (ptb-android/build.sh); optional
        if (payload / "bin/libptb.so").is_file():
            libs["libptb.so"] = payload / "bin/libptb.so"
        py = pylib(payload)
        for src, dst in [(py, py)] + list(PAYLOAD.items()):
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
        # surrogateescape: trunk refinement.py carries a cp1252 dash; keep it byte-exact
        s = p.read_bytes().decode("utf-8", "surrogateescape")
        if s.count(old) != 1:
            sys.exit("patch does not apply to %s: %r" % (rel, old))
        p.write_bytes(s.replace(old, new).encode("utf-8", "surrogateescape"))


def elf_dynsyms(path):
    """(DT_NEEDED names, {defined dynamic symbol: (Elf_Sym offset, binding)})
    of a little-endian ELF32/ELF64 shared library, from its section headers"""
    b = path.read_bytes()
    if b[:4] != b"\x7fELF":
        return [], {}
    e64 = b[4] == 2
    shoff, = struct.unpack_from("<Q" if e64 else "<I", b, 0x28 if e64 else 0x20)
    shentsize, shnum = struct.unpack_from("<HH", b, 0x3A if e64 else 0x2E)
    sh = [struct.unpack_from("<IIQQQQIIQQ" if e64 else "<IIIIIIIIII", b, shoff + i * shentsize)
          for i in range(shnum)]
    def strtab(i, o):
        return b[sh[i][4] + o:b.index(b"\0", sh[i][4] + o)].decode()
    needed, syms = [], {}
    for _, typ, _, _, off, size, link, _, _, ent in sh:
        if typ == 6:  # SHT_DYNAMIC
            for o in range(off, off + size, ent):
                tag, val = struct.unpack_from("<qQ" if e64 else "<iI", b, o)
                if tag == 1:  # DT_NEEDED
                    needed.append(strtab(link, val))
        elif typ == 11:  # SHT_DYNSYM
            for o in range(off, off + size, ent):
                if e64:
                    name, info, _, shndx = struct.unpack_from("<IBBH", b, o)
                else:
                    name, _, _, info, _, shndx = struct.unpack_from("<IIIBBH", b, o)
                if shndx:
                    syms[strtab(link, name)] = (o, info >> 4)
    return needed, syms


def share_typeinfo(sos):
    """Below API 23 bionic looks a symbol up in the library itself before its
    DT_NEEDED, so a cctbx extension and its library each keep their own weak
    copy of a typeinfo (asu_parameter: smtbx_refinement_constraints_ext.so
    and libsmtbx_refinement_constraints.so), and dynamic_cast across them
    fails: Boost.Python cannot pass a tertiary_xh_site as asu_parameter* and
    every refinement with riding hydrogens stops. Make such a weak typeinfo
    undefined where a direct DT_NEEDED defines it, so the library's copy
    is the only one (what API 23+ does by itself). Returns the count."""
    info = {p.name: (p, *elf_dynsyms(p)) for p in sos}
    n = 0
    for p, needed, syms in info.values():
        theirs = set()
        for d in needed:
            if d in info:
                theirs.update(info[d][2])
        mine = [o for s, (o, bind) in syms.items()
                if bind == 2 and s.startswith("_ZTI") and s in theirs]  # STB_WEAK
        if mine:
            b = bytearray(p.read_bytes())
            e64 = b[4] == 2
            for o in mine:
                struct.pack_into("<H", b, o + (6 if e64 else 14), 0)  # st_shndx = SHN_UNDEF
            p.write_bytes(bytes(b))
            n += len(mine)
    return n


def compile_pyc(out, python):
    """unchecked-hash pycs: no source stat on import, no first-start compile"""
    dirs = [str(out / d) for d in ("python", "util", "cctbx") if (out / d).is_dir()]
    if dirs:
        # syntax errors in py2-only test files are expected, hence no check
        subprocess.run([python, "-m", "compileall", "-q", "-j", "6",
                        "--invalidation-mode", "unchecked-hash",
                        "-s", str(out), "-p", "/"] + dirs,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def assemble(rundir, overlays, out, payload=None, jnilibs=None, python=None,
             typeinfo=False):
    files, libs = collect(rundir, overlays, payload)
    if out.exists():
        shutil.rmtree(out)
    for rel, src in files.items():
        dst = out / asset_name(rel)
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(src, dst)
    apply_patches(out)
    if jnilibs is not None:
        if jnilibs.exists():
            shutil.rmtree(jnilibs)
        jnilibs.mkdir(parents=True)
        for n, f in libs.items():
            shutil.copyfile(f, jnilibs / n)
        print("jniLibs: %d libraries -> %s" % (len(libs), jnilibs))
        if typeinfo:
            n = share_typeinfo(sorted(jnilibs.glob("*.so")) + sorted(out.rglob("*.so")))
            print("typeinfo: %d weak copies now bind to their DT_NEEDED" % n)
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
                  "bin/libptb.so", "python/lib/python3.14/os.py",
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
        wh = wheel_overlay(t / "w-1.0-py3-none-any.whl", t, pl)
        names, libs = assemble(rd, [wh, ov], out, pl, jl)
        assert names == [
            "android.options", "cctbx/cctbx_build/lib/scitbx_ext.so",
            "cctbx/cctbx_sources/iotbx/d.dic.gz-", "etc/gui/a.htm",
            "macro.xld", "occ/share/basis/x",
            "python/lib/python3.14/lib-dynload/_ssl.cpython-314.so",
            "python/lib/python3.14/os.py",
            "python/lib/python3.14/site-packages/w/__init__.py", "util/s/x.py"], names
        assert sorted(libs) == ["libNoSpherA2.so", "libcctbx.so",
                                "libptb.so", "libpython3.14.so"], libs
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
    ap.add_argument("--pyc", help="host Python of the payload's version, precompiles the .py")
    ap.add_argument("--stale", type=pathlib.Path,
                    help="file to delete when the assets change (the APK: "
                         "androiddeployqt's depfile does not list them)")
    ap.add_argument("--share-typeinfo", action="store_true",
                    help="minSdk < 23: one typeinfo per type across the payload's libraries")
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
        wheels = [wheel_overlay(w, pathlib.Path(t), a.payload) for w in a.wheel]
        assemble(a.rundir, wheels + a.overlay, a.out, a.payload, a.jnilibs, a.pyc,
                 a.share_typeinfo)
    if a.stale and old != lst.read_text().splitlines()[0]:
        a.stale.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
