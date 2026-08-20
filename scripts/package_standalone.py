#!/usr/bin/env python3

"""Package the minimum ReyEngine source tree needed for an offline build.

ReyEngine vendors every third-party dependency it needs (raylib, and GLFW
inside raylib's external/), so the archive this produces configures and builds
with no network access. The only dependency that is fetched at configure time
is yaml-cpp, and that is gated behind REY_ENABLE_YAML (OFF by default).

What is left out: the test/ and apps/ trees, IDE and VCS metadata, build
output directories, and vendored documentation.

Usage:
    scripts/package_standalone.py                       # -> ReyEngine_<tag-or-sha>.zip
    scripts/package_standalone.py -o /tmp/rey.zip       # override the name
    scripts/package_standalone.py --dry-run             # list what would be packaged
    scripts/package_standalone.py --verify              # unpack + cmake configure + build
    scripts/package_standalone.py --no-lua              # drop the vendored Lua (~900K)
    scripts/package_standalone.py --keep-docs           # keep vendored .md/CHANGELOG files
"""

import argparse
import fnmatch
import os
import re
import shutil
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path, PurePosixPath
from typing import List  # Backporting typing for compatibility

REPO_ROOT = Path(__file__).resolve().parent.parent

# Everything the build touches, rooted at the repo. Anything not under one of
# these is excluded by construction, so test/ and apps/ never get considered.
INCLUDE_ROOTS = [
    "CMakeLists.txt",
    "src",
    # copy_file() in the top-level CMakeLists reads these at *configure* time;
    # a missing font or icon is a hard configure error, not a runtime one.
    "reyengine/resources",
]

# Directory names dropped wherever they appear.
EXCLUDE_DIR_NAMES = {
    ".git", ".github", ".idea", ".vs", ".vscode",
    "__pycache__", "_deps", "node_modules",
}

# Directory paths (repo-relative, glob-matched) dropped along with their contents.
EXCLUDE_DIR_GLOBS = [
    "cmake-build*",
    "build",
    "src/thirdParty/raylib/projects",
    "src/thirdParty/raylib/examples",
    "src/thirdParty/raylib/src/external/glfw/docs",
    "src/thirdParty/raylib/src/external/glfw/tests",
    "src/thirdParty/raylib/src/external/glfw/examples",
]

EXCLUDE_FILE_GLOBS = [
    "*.o", "*.a", "*.so", "*.dll", "*.dylib", "*.exe", "*.obj", "*.lib",
    "*.pyc", "*.swp", "*~",
    ".DS_Store", "Thumbs.db",
]

# Vendored documentation. Dropped unless --keep-docs. LICENSE files are never
# dropped -- raylib, GLFW and Lua are all redistributed under their terms.
DOC_FILE_GLOBS = [
    "*.md", "CHANGELOG", "HISTORY", "ROADMAP", "*.html",
    "build.zig", "build.zig.zon",
]
LICENSE_NAMES = {"LICENSE", "LICENSE.md", "LICENSE.txt", "COPYING", "COPYING.txt", "NOTICE"}

# Doc-looking files that the build scripts actually read, so they cannot be
# pruned. raylib's PackConfigurations.cmake feeds README.md to CPack as both
# the package description and the welcome resource, and CPack hard-errors when
# either is missing -- even though we never invoke CPack ourselves.
KEEP_PATHS = {
    "src/thirdParty/raylib/README.md",
}

LUA_DIR = "src/thirdParty/lua"


DEFAULT_BASENAME = "ReyEngine"


def git_output(argv: List[str]):
    """Run a git command in the repo; return stripped stdout, or None on any failure."""
    try:
        proc = subprocess.run(
            ["git", "-C", str(REPO_ROOT)] + argv,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            universal_newlines=True)
    except OSError:
        return None  # git not installed
    if proc.returncode != 0:
        return None  # not a repo, no commits, no match
    return proc.stdout.strip() or None


def sanitize(label: str):
    """Reduce a git ref to something safe to embed in a filename.

    Tag names may legally contain '/' and other characters that are awkward or
    unsafe in a path, so anything outside [A-Za-z0-9._-] becomes a dash.
    """
    safe = "".join(c if (c.isalnum() or c in "._-") else "-" for c in label)
    return safe.strip("-.") or None


def tag_sort_key(tag: str):
    """Order tags by version, so 0.2.10 sorts above 0.2.9 (plain sort would not)."""
    parts = re.split(r"[._-]", tag.lstrip("vV"))
    return [(0, int(part)) if part.isdigit() else (1, part) for part in parts]


def version_label():
    """The tag on HEAD if there is one, else the first 5 chars of the sha.

    Returns None when git cannot tell us anything -- an export with no .git,
    or a machine with no git installed.
    """
    tags = git_output(["tag", "--points-at", "HEAD"])
    if tags:
        # Several tags can point at one commit; take the highest version.
        newest = sorted(tags.splitlines(), key=tag_sort_key)[-1].strip()
        label = sanitize(newest)
        if label:
            return label
    sha = git_output(["rev-parse", "--short=5", "HEAD"])
    if sha:
        return sanitize(sha)
    return None


def default_output(label) -> Path:
    suffix = label if label else "standalone"
    return REPO_ROOT / "{}_{}.zip".format(DEFAULT_BASENAME, suffix)


def rel_posix(path: Path) -> str:
    return PurePosixPath(path.relative_to(REPO_ROOT).as_posix()).as_posix()


def dir_excluded(rel: str, name: str, args: argparse.Namespace) -> bool:
    if name in EXCLUDE_DIR_NAMES:
        return True
    if any(fnmatch.fnmatch(rel, g) for g in EXCLUDE_DIR_GLOBS):
        return True
    if args.no_lua and rel == LUA_DIR:
        return True
    return False


def file_excluded(rel: str, name: str, args: argparse.Namespace) -> bool:
    if name in LICENSE_NAMES or rel in KEEP_PATHS:
        return False
    if any(fnmatch.fnmatch(name, g) for g in EXCLUDE_FILE_GLOBS):
        return True
    if not args.keep_docs and any(fnmatch.fnmatch(name, g) for g in DOC_FILE_GLOBS):
        return True
    return False


def collect(args: argparse.Namespace) -> List[Path]:
    """Walk INCLUDE_ROOTS and return the surviving files, sorted."""
    found = []  # Removed modern type annotation here to prevent runtime crash
    for root in INCLUDE_ROOTS:
        target = REPO_ROOT / root
        if not target.exists():
            sys.exit(f"error: required path is missing from the repo: {root}")
        if target.is_file():
            found.append(target)
            continue
        for dirpath, dirnames, filenames in os.walk(target):
            here = Path(dirpath)
            # Prune in place so os.walk does not descend into excluded trees.
            dirnames[:] = sorted(
                d for d in dirnames
                if not dir_excluded(rel_posix(here / d), d, args)
            )
            for fname in sorted(filenames):
                fpath = here / fname
                if fpath.is_symlink() or not fpath.is_file():
                    continue
                if not file_excluded(rel_posix(fpath), fname, args):
                    found.append(fpath)
    return sorted(set(found), key=rel_posix)


def build_manifest(files: List[Path], args: argparse.Namespace, label) -> str:
    total = sum(f.stat().st_size for f in files)
    lines = [
        "ReyEngine standalone source package",
        "",
        f"version: {label if label else 'unknown (no git metadata)'}",
        f"files: {len(files)}",
        f"uncompressed: {total / 1_048_576:.1f} MiB",
        f"lua included: {not args.no_lua}",
        f"vendored docs included: {args.keep_docs}",
        "",
        "Build (no network required):",
        "    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release",
        "    cmake --build build -j",
        "",
        "Notes:",
        "  * GLFW is vendored at src/thirdParty/raylib/src/external/glfw -- nothing to fetch.",
        "  * REY_ENABLE_YAML=ON fetches yaml-cpp over the network and will fail offline.",
        "  * REY_ENABLE_TESTS=ON will fail: the test/ tree is not part of this package.",
        "  * Linux also needs X11 dev headers (libx11, libxrandr, libxinerama,",
        "    libxcursor, libxi) and libgl1-mesa-dev; these come from the distro,",
        "    not from this archive. MinGW/Windows needs nothing extra.",
        "",
        "Contents:",
    ]
    lines += [f"  {rel_posix(f)}" for f in files]
    return "\n".join(lines) + "\n"


def write_zip(files: List[Path], out_path: Path, prefix: str, manifest: str) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    # Fixed timestamps keep the archive byte-identical across runs of an
    # unchanged tree, which makes it easy to tell whether anything moved.
    fixed_time = (1980, 1, 1, 0, 0, 0)
    with zipfile.ZipFile(str(out_path), "w", zipfile.ZIP_DEFLATED) as zf:
        for f in files:
            info = zipfile.ZipInfo(f"{prefix}/{rel_posix(f)}", date_time=fixed_time)
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o644 << 16
            zf.writestr(info, f.read_bytes())
        info = zipfile.ZipInfo(f"{prefix}/PACKAGE_MANIFEST.txt", date_time=fixed_time)
        info.compress_type = zipfile.ZIP_DEFLATED
        info.external_attr = 0o644 << 16
        zf.writestr(info, manifest)


def verify(out_path: Path, prefix: str) -> int:
    """Unpack into a scratch dir and run a real configure + build."""
    cmake = shutil.which("cmake")
    if not cmake:
        print("verify: cmake not found on PATH, skipping", file=sys.stderr)
        return 1
    with tempfile.TemporaryDirectory(prefix="reyengine-verify-") as tmp:
        tmpdir = Path(tmp)
        with zipfile.ZipFile(str(out_path)) as zf:
            zf.extractall(str(tmpdir))
        src = tmpdir / prefix
        build = tmpdir / "build"
        for step, cmd in (
            ("configure", [cmake, "-S", str(src), "-B", str(build),
                           "-DCMAKE_BUILD_TYPE=Release"]),
            ("build", [cmake, "--build", str(build), "-j", str(os.cpu_count() or 2)]),
        ):
            print(f"verify: {step}...", flush=True)
            # capture_output= and text= are 3.7+; these spellings are
            # equivalent and work on 3.6.
            proc = subprocess.run(cmd, stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE,
                                  universal_newlines=True)
            if proc.returncode != 0:
                print(proc.stdout[-4000:], file=sys.stderr)
                print(proc.stderr[-4000:], file=sys.stderr)
                print(f"verify: {step} FAILED", file=sys.stderr)
                return proc.returncode
            combined = proc.stdout + proc.stderr
            for marker in ("Downloading", "Cloning into", "FetchContent"):
                if marker in combined:
                    print(f"verify: WARNING - {step} mentions '{marker}'; "
                          "the tree may not be fully offline", file=sys.stderr)
        print("verify: OK - configured and built from the archive")
        return 0


def main() -> int:
    p = argparse.ArgumentParser(
        description="Zip the minimum ReyEngine tree needed for an offline build.")
    p.add_argument("-o", "--output", type=Path, default=None,
                   help="output zip path (default: ReyEngine_<tag-or-sha>.zip "
                        "in the repo root -- the tag on HEAD if there is one, "
                        "otherwise the first 5 characters of the sha)")
    p.add_argument("--prefix", default="ReyEngine",
                   help="top-level directory inside the zip (default: ReyEngine)")
    p.add_argument("--dry-run", action="store_true",
                   help="list the files that would be packaged and exit")
    p.add_argument("--verify", action="store_true",
                   help="after writing, unpack to a temp dir and cmake configure + build")
    p.add_argument("--no-lua", action="store_true",
                   help=f"omit {LUA_DIR} (only needed with REY_ENABLE_LUA=ON)")
    p.add_argument("--keep-docs", action="store_true",
                   help="keep vendored .md / CHANGELOG / HISTORY files")
    args = p.parse_args()

    label = version_label()
    if args.output is None:
        args.output = default_output(label)

    files = collect(args)
    if not files:
        sys.exit("error: nothing collected -- is this being run from the ReyEngine repo?")

    total = sum(f.stat().st_size for f in files)
    if args.dry_run:
        for f in files:
            print(rel_posix(f))
        print(f"\n{len(files)} files, {total / 1_048_576:.1f} MiB uncompressed",
              file=sys.stderr)
        return 0

    write_zip(files, args.output, args.prefix,
              build_manifest(files, args, label))
    packed = args.output.stat().st_size
    print(f"wrote {args.output}")
    print(f"  {len(files)} files, {total / 1_048_576:.1f} MiB -> "
          f"{packed / 1_048_576:.1f} MiB compressed")

    if args.verify:
        return verify(args.output, args.prefix)
    return 0


if __name__ == "__main__":
    sys.exit(main())
