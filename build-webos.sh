#!/bin/bash
# Docker로 webOS ARMv7 바이너리 빌드
# 타겟: LG 프로젝터 HU715QW (armv7l, Realtek rtd2875)
set -e

PROJ_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJ_DIR/build_webos"
mkdir -p "$BUILD_DIR"

echo "======================================"
echo "webOS ARMv7 크로스 컴파일 빌드 시작"
echo "======================================"
echo "프로젝트 경로: $PROJ_DIR"
echo "빌드 출력:    $BUILD_DIR"
echo ""

echo "[1/3] Docker 이미지 빌드..."
# macOS ARM64(Apple Silicon)에서도 동작하도록 --platform linux/amd64 강제
# amd64 Ubuntu에서 armhf 멀티아크 크로스 컴파일이 올바르게 동작함
docker build --platform linux/amd64 -t webosbrowser-builder -f "$PROJ_DIR/Dockerfile.webos" "$PROJ_DIR"

echo ""
echo "[2/3] 크로스 컴파일..."
docker run --rm --platform linux/amd64 \
  -v "$PROJ_DIR:/src:ro" \
  -v "$BUILD_DIR:/build" \
  webosbrowser-builder \
  bash -c "
    set -e
    echo '--- 툴체인 확인 ---'
    arm-linux-gnueabi-g++ --version
    echo ''

    echo '--- cmake 구성 ---'
    cd /build && \
    cmake /src \
      -DCMAKE_TOOLCHAIN_FILE=/src/toolchain/webos-arm.cmake \
      -DWEBOS_TARGET=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_VERBOSE_MAKEFILE=ON \
      2>&1

    echo ''
    echo '--- 빌드 ---'
    make -j\$(nproc) 2>&1
  "

echo ""
if [ -f "$BUILD_DIR/bin/webosbrowser" ]; then
    echo "[3/3] 완료!"
    echo "========================================"
    echo "바이너리: $BUILD_DIR/bin/webosbrowser"
    file "$BUILD_DIR/bin/webosbrowser"
    echo "========================================"
else
    echo "[3/3] 경고: 바이너리를 찾을 수 없습니다"
    echo "빌드 출력 디렉토리 확인:"
    ls -la "$BUILD_DIR/" 2>/dev/null || echo "(비어 있음)"
    exit 1
fi
