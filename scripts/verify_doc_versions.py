#!/usr/bin/env python3
"""Verify that documentation files cite the same dependency versions as vcpkg.json.

vcpkg.json is the single source of truth for pinned dependency versions. This
script scans a small whitelist of documentation files and fails if any of them
cite a different version for the tracked libraries (currently gRPC and
Protocol Buffers).

Intended to run in CI via .github/workflows/docs-verify.yml.
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

# Repository root, resolved from this file's location.
REPO_ROOT = Path(__file__).resolve().parent.parent

# Libraries to check. Name must match the "overrides" entry in vcpkg.json.
TRACKED_LIBS = ("grpc", "protobuf")

# Documentation files that must agree with vcpkg.json.
DOCS = (
    "README.md",
    "CLAUDE.md",
    "LICENSE-THIRD-PARTY",
    "docs/SOUP.md",
    "grpc/README.md",
)


def load_pinned_versions() -> dict[str, str]:
    """Return {lib_name: version} from vcpkg.json overrides."""
    with (REPO_ROOT / "vcpkg.json").open("r", encoding="utf-8") as fh:
        data = json.load(fh)
    overrides = {o["name"]: o["version"] for o in data.get("overrides", [])}
    missing = [lib for lib in TRACKED_LIBS if lib not in overrides]
    if missing:
        sys.stderr.write(
            f"vcpkg.json is missing overrides for: {', '.join(missing)}\n"
        )
        sys.exit(2)
    return {lib: overrides[lib] for lib in TRACKED_LIBS}


def scan_file(path: Path, lib: str, pinned: str) -> list[str]:
    """Return a list of offending "line: text" strings from `path`.

    A line is offending if `lib` and a version token (matching the same
    NUM.NUM.NUM shape as `pinned`) appear close together, and that version
    differs from `pinned`. We only consider versions that share the pinned
    version's component count to avoid matching IPs, ports, or unrelated
    decimal numbers.
    """
    component_count = pinned.count(".") + 1
    version_re = r"\d+" + r"\.\d+" * (component_count - 1) + r"(?![\.\d])"
    # Match "lib" followed by a version token within a small window on the
    # same line, with no other tracked library name intervening. The negative
    # lookahead keeps "gRPC 1.60.0 + protobuf 4.25.1" from cross-matching.
    other_libs = "|".join(re.escape(other) for other in TRACKED_LIBS if other.lower() != lib.lower())
    if other_libs:
        intervening = rf"(?:(?!{other_libs}|{re.escape(lib)}).)"
    else:
        intervening = r"."
    pattern = re.compile(
        rf"\b{re.escape(lib)}\b{intervening}{{0,40}}?({version_re})",
        re.IGNORECASE,
    )
    offenders: list[str] = []
    with path.open("r", encoding="utf-8") as fh:
        for lineno, line in enumerate(fh, 1):
            for match in pattern.finditer(line):
                version = match.group(1)
                if version != pinned:
                    offenders.append(
                        f"{lineno}: {line.rstrip()} "
                        f"(found {version}, expected {pinned})"
                    )
    return offenders


def main() -> int:
    pinned = load_pinned_versions()
    print("vcpkg.json pins:")
    for lib, version in pinned.items():
        print(f"  {lib} = {version}")

    failed = False
    for rel in DOCS:
        doc = REPO_ROOT / rel
        if not doc.exists():
            print(f"warning: {rel} not found, skipping")
            continue
        for lib, version in pinned.items():
            offenders = scan_file(doc, lib, version)
            if offenders:
                failed = True
                print(
                    f"::error file={rel}::Documentation drift for {lib} "
                    f"(expected {version}):"
                )
                for entry in offenders:
                    print(f"  {rel}:{entry}")

    if failed:
        print(
            "\nDocumentation version drift detected. Update the offending "
            "files to match vcpkg.json (single source of truth)."
        )
        return 1

    print("\nAll documented dependency versions match vcpkg.json.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
