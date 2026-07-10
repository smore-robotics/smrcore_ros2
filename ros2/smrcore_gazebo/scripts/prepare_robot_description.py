#!/usr/bin/env python3
"""Expand xacro to URDF and stage mesh files for Gazebo Classic.

Gazebo Classic cannot reliably load meshes when paths contain non-ASCII
characters or percent-encoded file:// URIs. Copy meshes to a temporary
Gazebo model path and reference them with model:// URIs in the generated URDF.
"""

import re
import shutil
import sys
from pathlib import Path
from xml.dom import Node

import xacro
from ament_index_python.packages import PackageNotFoundError, get_package_share_directory

_PACKAGE_URI_RE = re.compile(r'package://([^/"\s]+)/([^"\s]+)')
GAZEBO_MODEL_CACHE_ROOT = Path("/tmp/smrcore_gazebo_models")
MESH_MODEL_NAME = "smrcore_smri3_meshes"
MESH_CACHE_DIR = GAZEBO_MODEL_CACHE_ROOT / MESH_MODEL_NAME
MESH_CACHE_MESH_DIR = MESH_CACHE_DIR / "meshes"


def remove_comments(node):
    for child in list(node.childNodes):
        if child.nodeType == Node.COMMENT_NODE:
            node.removeChild(child)
        else:
            remove_comments(child)


def stage_smri3_meshes() -> Path:
    src_dir = Path(get_package_share_directory("smrcore_description")) / "meshes" / "smri3"
    if not src_dir.is_dir():
        raise RuntimeError(f"mesh directory not found: {src_dir}")

    MESH_CACHE_MESH_DIR.mkdir(parents=True, exist_ok=True)
    (MESH_CACHE_DIR / "model.config").write_text(
        f"""<?xml version="1.0"?>
<model>
  <name>{MESH_MODEL_NAME}</name>
  <version>1.0</version>
  <sdf version="1.6">model.sdf</sdf>
  <description>Temporary staged SMR-i3 meshes for Gazebo Classic.</description>
</model>
""",
        encoding="utf-8",
    )
    for src in sorted(src_dir.glob("*.STL")):
        dst = MESH_CACHE_MESH_DIR / src.name
        shutil.copy2(src.resolve(), dst, follow_symlinks=True)
    return MESH_CACHE_DIR


def local_file_uri(path: Path) -> str:
    return path.resolve().as_uri()


def resolve_package_uris(urdf_xml: str, mesh_cache_dir: Path) -> str:
    def repl(match):
        package_name = match.group(1)
        relative_path = match.group(2)
        if package_name == "smrcore_description" and relative_path.startswith("meshes/smri3/"):
            # Gazebo Classic's package:// resolver can fail under non-ASCII
            # install paths. Stage these meshes in an ASCII Gazebo model path
            # and use model:// so both gzserver and gzclient resolve them.
            return f"model://{mesh_cache_dir.name}/meshes/{Path(relative_path).name}"

        try:
            package_share = get_package_share_directory(package_name)
        except PackageNotFoundError as exc:
            raise RuntimeError(f"ROS package not found: {package_name}") from exc
        return local_file_uri(Path(package_share) / relative_path)

    return _PACKAGE_URI_RE.sub(repl, urdf_xml)


def main():
    if len(sys.argv) < 2:
        print(
            "usage: prepare_robot_description.py <file.xacro> [name:=value ...]",
            file=sys.stderr,
        )
        return 2

    mappings = {}
    for arg in sys.argv[2:]:
        if ":=" not in arg:
            print(f"invalid xacro argument: {arg}", file=sys.stderr)
            return 2
        name, value = arg.split(":=", 1)
        mappings[name] = value

    mesh_cache_dir = stage_smri3_meshes()
    doc = xacro.process_file(sys.argv[1], mappings=mappings)
    remove_comments(doc)
    urdf_xml = resolve_package_uris(doc.toxml(), mesh_cache_dir)
    sys.stdout.write(urdf_xml)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
