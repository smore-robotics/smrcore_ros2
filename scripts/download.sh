#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd -P)"
SDK_VERSION="${SDK_VERSION:-${VERSION:-}}"
if [ -z "$SDK_VERSION" ] && [ -f "$ROOT_DIR/.sdk-version" ]; then
  SDK_VERSION="$(tr -d '[:space:]' < "$ROOT_DIR/.sdk-version")"
fi
if [ -z "$SDK_VERSION" ]; then
  echo "download: 请设置 SDK_VERSION/VERSION，或提供 .sdk-version" >&2
  exit 1
fi
if [ "$SDK_VERSION" = "latest" ]; then
  LATEST_URL="$(curl -Ls -o /dev/null -w '%{url_effective}' "https://github.com/smore-robotics/smrcore_sdk/releases/latest")"
  SDK_VERSION="${LATEST_URL##*/}"
fi
SDK_VERSION="${SDK_VERSION#v}"
if ! printf '%s\n' "$SDK_VERSION" | grep -Eq '^[0-9]+\.[0-9]+\.[0-9]+$'; then
  echo "download: SDK_VERSION 必须为 x.y.z，或使用 VERSION=latest；当前为 ${SDK_VERSION}" >&2
  exit 1
fi

SDK_DIR="$ROOT_DIR/3rdparty/smrcore_sdk"
ARCHIVE="$ROOT_DIR/3rdparty/smrcore_sdk-cpp-linux-x86_64-v${SDK_VERSION}.tar.gz"
SDK_RELEASE_TAG="${SDK_RELEASE_TAG:-v${SDK_VERSION}}"
URL="https://github.com/smore-robotics/smrcore_sdk/releases/download/${SDK_RELEASE_TAG}/smrcore_sdk-cpp-linux-x86_64-v${SDK_VERSION}.tar.gz"

mkdir -p "$ROOT_DIR/3rdparty"
rm -rf "$SDK_DIR"

echo "下载 SMRcore SDK v${SDK_VERSION}"
curl -L --fail --show-error "$URL" -o "$ARCHIVE"
mkdir -p "$SDK_DIR"
tar -xzf "$ARCHIVE" -C "$SDK_DIR"
rm -f "$ARCHIVE"
echo "SDK 已安装到 $SDK_DIR"
