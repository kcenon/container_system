#!/usr/bin/env python3
"""Generate a human-readable dependency report for SBOM artifacts."""

from __future__ import annotations

import json
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
MANIFEST_PATH = REPO_ROOT / "vcpkg.json"
GRPC_CMAKE_PATH = REPO_ROOT / "grpc" / "CMakeLists.txt"


def format_dependency(dep: str | dict) -> tuple[str, str, str]:
    if isinstance(dep, str):
        return dep, "-", "-"

    name = dep.get("name", "unknown")
    version = dep.get("version>=", "-")
    features = ", ".join(dep.get("features", [])) or "-"
    return name, version, features


def print_table(rows: list[tuple[str, str, str]]) -> None:
    print("| Package | Version Constraint | Features |")
    print("|---------|-------------------|----------|")
    for name, version, features in rows:
        print(f"| {name} | {version} | {features} |")


def main() -> int:
    manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    overrides = {item["name"]: item["version"] for item in manifest.get("overrides", [])}

    print("# Dependency Inventory")
    print()
    print(f"**Project**: {manifest.get('name', 'N/A')}")
    print(f"**Version**: {manifest.get('version', 'N/A')}")
    print()
    print("## Root Package Dependencies")
    print()

    direct_rows = [format_dependency(dep) for dep in manifest.get("dependencies", [])]
    if direct_rows:
        print_table(direct_rows)
    else:
        print("The root package has no direct third-party dependencies.")

    features = manifest.get("features", {})
    if features:
        print()
        print("## Feature Dependencies")
        print()
        for feature_name, feature_data in features.items():
            print(f"### {feature_name}")
            print()
            rows = [format_dependency(dep) for dep in feature_data.get("dependencies", [])]
            if rows:
                print_table(rows)
            else:
                print("No additional package dependencies.")
            print()

    print("## Module-Scoped Optional Deliverables")
    print()
    if GRPC_CMAKE_PATH.exists():
        print("| Module | Built by Root Project | Dependencies | Activation |")
        print("|--------|-----------------------|--------------|------------|")
        grpc_version = overrides.get("grpc", "unversioned")
        protobuf_version = overrides.get("protobuf", "unversioned")
        print(
            "| `grpc/` | No | "
            f"`protobuf` {protobuf_version}, `gRPC` {grpc_version}, system threads | "
            "Configure `cmake -S grpc -B build-grpc` |"
        )
        print()
        print(
            "When the isolated `grpc/` subproject is built, review these module-scoped "
            "dependencies alongside the root manifest because they are outside the "
            "default package dependency set."
        )
    else:
        print("No module-scoped optional deliverables detected.")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
