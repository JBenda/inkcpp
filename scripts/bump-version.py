#!/usr/bin/env python3
"""bump-version.py — Bump the inkcpp project version or add a new Unreal Engine version.

Usage:
    bump-version.py inkcpp <new_version>
    bump-version.py ue <new_version>

Examples:
    python scripts/bump-version.py inkcpp 0.1.11
    python scripts/bump-version.py ue 5.8

inkcpp subcommand updates:
    - CMakeLists.txt                       (project(inkcpp VERSION ...))
    - setup.py                             (version="...")

ue subcommand (adds new version, keeps all prior versions in CI):
    - unreal/CMakeLists.txt                (default INKCPP_UNREAL_TARGET_VERSION + comment)
    - .github/workflows/build.yml          (Install UE run block + new Upload step)
    - .github/workflows/release.yml        (download / zip / release artifact lists)
    Also warns if Documentation/unreal/InkCPP_DEMO.zip targets a different UE version.
"""

import json
import re
import sys
import zipfile

from docopt import docopt
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ARGS = docopt(__doc__, version="1.0")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _ok(rel_path: str, detail: str = "") -> None:
    suffix = f"  ({detail})" if detail else ""
    print(f"  [OK]    {rel_path}{suffix}")


def _warn(rel_path: str, msg: str) -> None:
    print(f"  [WARN]  {rel_path}  — {msg}")


def _info(msg: str) -> None:
    print(f"  [INFO]  {msg}")


def _read(rel: str) -> tuple[Path, str]:
    path = REPO_ROOT / rel
    return path, path.read_text(encoding="utf-8")


def _write(path: Path, text: str) -> None:
    path.write_text(text, encoding="utf-8")


# ---------------------------------------------------------------------------
# inkcpp version bump
# ---------------------------------------------------------------------------

INKCPP_FILES = [
    (
        "CMakeLists.txt",
        r"(project\(inkcpp VERSION\s+)\d+\.\d+\.\d+",
        r"\g<1>{v}",
    ),
    (
        "setup.py",
        r'(version=")[^"]+(")',
        r"\g<1>{v}\2",
    ),
]


def current_inkcpp_version() -> str:
    _, text = _read("CMakeLists.txt")
    m = re.search(r"project\(inkcpp VERSION\s+(\d+\.\d+\.\d+)", text)
    if not m:
        raise RuntimeError("Could not parse inkcpp version from CMakeLists.txt")
    return m.group(1)


def bump_inkcpp(new_ver: str) -> None:
    if not re.fullmatch(r"\d+\.\d+\.\d+", new_ver):
        sys.exit(f"ERROR: inkcpp version must be MAJOR.MINOR.PATCH, got: {new_ver!r}")

    old_ver = current_inkcpp_version()
    if old_ver == new_ver:
        print(f"inkcpp version is already {old_ver} — nothing to do.")
        return

    print(f"Bumping inkcpp  {old_ver}  →  {new_ver}\n")
    changed = []

    for rel, pattern, tmpl in INKCPP_FILES:
        path, original = _read(rel)
        replacement = tmpl.replace("{v}", new_ver)
        updated, count = re.subn(pattern, replacement, original)
        if count == 0:
            _warn(rel, "pattern not matched — skipped")
        else:
            _write(path, updated)
            _ok(rel, f"{count} replacement{'s' if count != 1 else ''}")
            changed.append(rel)

    print()
    if changed:
        files = " ".join(changed)
        print("Suggested commit:")
        print(f"  git add {files}")
        print(f'  git commit -m "chore: bump inkcpp version {old_ver} → {new_ver}"')


# ---------------------------------------------------------------------------
# UE version bump
# ---------------------------------------------------------------------------


def current_ue_version() -> str:
    _, text = _read("unreal/CMakeLists.txt")
    m = re.search(r'set\(INKCPP_UNREAL_TARGET_VERSION\s+"(\d+\.\d+)"', text)
    if not m:
        raise RuntimeError("Could not parse UE version from unreal/CMakeLists.txt")
    return m.group(1)


def _update_unreal_cmake(old_ue: str, new_ue: str) -> bool:
    rel = "unreal/CMakeLists.txt"
    path, text = _read(rel)

    # Update cached default value
    text, n1 = re.subn(
        r'(set\(INKCPP_UNREAL_TARGET_VERSION\s+")' + re.escape(old_ue) + r'"',
        r"\g<1>" + new_ue + '"',
        text,
    )
    # Update the e.g. comment so it shows the previous default (old_ue)
    text, n2 = re.subn(
        r'(CACHE STRING "[^"]*e\.g: )[\d.]+(")',
        r"\g<1>" + old_ue + r"\2",
        text,
    )

    if n1 == 0:
        _warn(rel, f"default version {old_ue!r} not found — skipped")
        return False
    _write(path, text)
    _ok(rel, f"default {old_ue} → {new_ue}" + (", comment updated" if n2 else ""))
    return True


def _update_build_yml(old_ue: str, new_ue: str) -> bool:
    rel = ".github/workflows/build.yml"
    path, text = _read(rel)

    old_u = old_ue.replace(".", "_")
    new_u = new_ue.replace(".", "_")

    # Guard: already present?
    if f'INKCPP_UNREAL_TARGET_VERSION="{new_ue}"' in text:
        _warn(rel, f"UE {new_ue} already present — skipped")
        return False

    # 1. Prepend new cmake build+install pair before the existing first pair
    old_cmake_pair = (
        f'        cmake $GITHUB_WORKSPACE -DINKCPP_UNREAL_TARGET_VERSION="{old_ue}" -DINKCPP_UNREAL=ON\n'
        f"        cmake --install . --config $BUILD_TYPE --prefix comp_unreal_{old_u} --component unreal"
    )
    new_cmake_pair = (
        f'        cmake $GITHUB_WORKSPACE -DINKCPP_UNREAL_TARGET_VERSION="{new_ue}" -DINKCPP_UNREAL=ON\n'
        f"        cmake --install . --config $BUILD_TYPE --prefix comp_unreal_{new_u} --component unreal\n"
    )
    if old_cmake_pair not in text:
        _warn(
            rel, f"cmake install lines for {old_ue} not found — run block not updated"
        )
        return False
    text = text.replace(old_cmake_pair, new_cmake_pair + old_cmake_pair, 1)

    # 2. Insert new Upload step immediately before the existing Upload UE {old_ue} step
    old_upload_anchor = f"    - name: Upload UE {old_ue}\n"
    new_upload_block = (
        f"    - name: Upload UE {new_ue}\n"
        f"      if: ${{{{ matrix.unreal }}}}\n"
        f"      uses: actions/upload-artifact@v4\n"
        f"      with:\n"
        f"        name: unreal_{new_u}\n"
        f"        path: build/comp_unreal_{new_u}/\n"
    )
    if old_upload_anchor not in text:
        _warn(rel, f"'Upload UE {old_ue}' step not found — upload step not inserted")
        return False
    text = text.replace(old_upload_anchor, new_upload_block + old_upload_anchor, 1)

    _write(path, text)
    _ok(rel, f"added cmake pair + Upload UE {new_ue} step")
    return True


def _update_release_yml(old_ue: str, new_ue: str) -> bool:
    rel = ".github/workflows/release.yml"
    path, text = _read(rel)

    old_u = old_ue.replace(".", "_")
    new_u = new_ue.replace(".", "_")

    # Guard: already present?
    if f"unreal_{new_u}" in text:
        _warn(rel, f"unreal_{new_u} already present — skipped")
        return False

    ok = True

    # 1. Download step: -n unreal_OLD → -n unreal_NEW -n unreal_OLD
    old_flag = f"-n unreal_{old_u}"
    if old_flag not in text:
        _warn(rel, f"download flag '-n unreal_{old_u}' not found")
        ok = False
    else:
        text = text.replace(old_flag, f"-n unreal_{new_u} {old_flag}", 1)

    # 2. Zip for-loop: unreal_OLD → unreal_NEW unreal_OLD  (only inside the for line)
    for_pat = r"(for f in [^\n]*?)(" + re.escape(f"unreal_{old_u}") + r")"
    text, n = re.subn(for_pat, r"\1unreal_" + new_u + r" \2", text, count=1)
    if n == 0:
        _warn(rel, f"unreal_{old_u} not found in zip for-loop")
        ok = False

    # 3. gh release create asset list: "unreal_OLD.zip" → "unreal_NEW.zip" "unreal_OLD.zip"
    old_asset = f'"unreal_{old_u}.zip"'
    if old_asset not in text:
        _warn(rel, f"release asset {old_asset} not found")
        ok = False
    else:
        text = text.replace(old_asset, f'"unreal_{new_u}.zip" {old_asset}', 1)

    if not ok:
        return False

    _write(path, text)
    _ok(rel, "download / zip / release lists updated")
    return True


def _check_demo_zip(new_ue: str) -> None:
    zip_rel = "Documentation/unreal/InkCPP_DEMO.zip"
    zip_path = REPO_ROOT / zip_rel
    if not zip_path.exists():
        _info(f"{zip_rel} not found locally — skipping demo check")
        return

    try:
        with zipfile.ZipFile(zip_path) as zf:
            uproject = next((n for n in zf.namelist() if n.endswith(".uproject")), None)
            if not uproject:
                _warn(zip_rel, "no .uproject file found inside the zip")
                return
            data = json.loads(zf.read(uproject).decode("utf-8"))
            engine_assoc = data.get("EngineAssociation", "<missing>")
    except Exception as exc:
        _warn(zip_rel, f"could not inspect zip: {exc}")
        return

    if engine_assoc == new_ue:
        _ok(zip_rel, f"EngineAssociation already {engine_assoc!r}")
    else:
        print(f"  [WARN]  {zip_rel}")
        print(
            f"            EngineAssociation is {engine_assoc!r} but new default is {new_ue!r}."
        )
        print(f"            Action: open the project in UE {new_ue}, let it convert,")
        print("            re-export it, and replace the zip.")


def bump_ue(new_ue: str) -> None:
    if not re.fullmatch(r"\d+\.\d+", new_ue):
        sys.exit(f"ERROR: UE version must be MAJOR.MINOR (e.g. 5.8), got: {new_ue!r}")

    old_ue = current_ue_version()
    if old_ue == new_ue:
        print(f"UE default is already {old_ue} — nothing to do.")
        return

    print(f"Adding UE {new_ue}  (new default; {old_ue} and earlier remain in CI)\n")

    changed = []
    if _update_unreal_cmake(old_ue, new_ue):
        changed.append("unreal/CMakeLists.txt")
    if _update_build_yml(old_ue, new_ue):
        changed.append(".github/workflows/build.yml")
    if _update_release_yml(old_ue, new_ue):
        changed.append(".github/workflows/release.yml")

    print()
    print("Demo project check:")
    _check_demo_zip(new_ue)

    if changed:
        files = " ".join(changed)
        print()
        print("Suggested commit:")
        print(f"  git add {files}")
        print(
            f'  git commit -m "chore: add UE {new_ue} support (default; {old_ue} still built)"'
        )


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    if ARGS["inkcpp"]:
        bump_inkcpp(ARGS["<new_version>"])
    elif ARGS["ue"]:
        bump_ue(ARGS["<new_version>"])
