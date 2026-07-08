#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd -P)"
SDK_DIR="$ROOT_DIR/3rdparty/smrcore_sdk"
SDK_CONFIG="$SDK_DIR/lib/cmake/smrcore_sdk/smrcore_sdkConfig.cmake"

if [ ! -f "$SDK_CONFIG" ]; then
  "$ROOT_DIR/scripts/download.sh"
fi
if [ ! -f "$SDK_CONFIG" ]; then
  echo "未找到 SDK CMake 配置: $SDK_CONFIG" >&2
  echo "请检查下载包是否包含 lib/cmake/smrcore_sdk/smrcore_sdkConfig.cmake" >&2
  exit 1
fi

ROS_DISTRO="${ROS_DISTRO:-humble}"
ROS_SETUP="/opt/ros/${ROS_DISTRO}/setup.bash"
if [ ! -f "$ROS_SETUP" ]; then
  echo "未找到 ROS 环境脚本: $ROS_SETUP" >&2
  exit 1
fi

# shellcheck source=/dev/null
set +u
source "$ROS_SETUP"
set -u

EXTRA_PREFIX="$SDK_DIR"
if [ -n "${CMAKE_PREFIX_PATH:-}" ]; then
  EXTRA_PREFIX="${SDK_DIR};${CMAKE_PREFIX_PATH}"
elif [ -n "${AMENT_PREFIX_PATH:-}" ]; then
  EXTRA_PREFIX="${SDK_DIR};${AMENT_PREFIX_PATH}"
else
  EXTRA_PREFIX="${SDK_DIR};/opt/ros/${ROS_DISTRO}"
fi

colcon --log-base "$ROOT_DIR/log" build \
  --base-paths "$ROOT_DIR/ros2" \
  --build-base "$ROOT_DIR/build" \
  --install-base "$ROOT_DIR/install" \
  --symlink-install \
  --cmake-args -DCMAKE_PREFIX_PATH="$EXTRA_PREFIX"
