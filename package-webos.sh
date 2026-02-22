#!/bin/bash
# webOS IPK 패키징 및 프로젝터 설치
# 전제 조건: ./build-webos.sh 실행 완료, ares-cli 설치
set -e

PROJ_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJ_DIR/build_webos"
BINARY="$BUILD_DIR/bin/webosbrowser"

echo "======================================"
echo "webOS IPK 패키징 및 배포"
echo "======================================"

# 바이너리 존재 확인
if [ ! -f "$BINARY" ]; then
    echo "ERROR: 바이너리를 찾을 수 없습니다: $BINARY"
    echo "먼저 ./build-webos.sh를 실행하세요"
    exit 1
fi

# 바이너리 아키텍처 확인
echo "바이너리 정보:"
file "$BINARY"
echo ""

echo "[1/3] IPK 패키징..."

# webos-meta/appinfo.json에서 앱 ID 읽기
APP_ID=$(python3 -c "import json; d=json.load(open('$PROJ_DIR/webos-meta/appinfo.json')); print(d['id'])")
APP_VER=$(python3 -c "import json; d=json.load(open('$PROJ_DIR/webos-meta/appinfo.json')); print(d.get('version','1.0.0'))")
echo "앱 ID:  $APP_ID"
echo "버전:   $APP_VER"

# 스테이징 디렉토리 구성
STAGE="$BUILD_DIR/ipk_stage/$APP_ID"
rm -rf "$BUILD_DIR/ipk_stage"
mkdir -p "$STAGE"

# webOS 메타데이터 복사
cp -r "$PROJ_DIR/webos-meta/"* "$STAGE/"

# 바이너리 복사 (appinfo.json의 "main": "webosbrowser"와 일치)
cp "$BINARY" "$STAGE/webosbrowser"
chmod +x "$STAGE/webosbrowser"

echo "스테이징 디렉토리 구성:"
ls -la "$STAGE/"
echo ""

# ares-package로 IPK 생성
IPK_OUT_DIR="$BUILD_DIR"
ares-package "$STAGE" -o "$IPK_OUT_DIR"

# 생성된 IPK 파일 찾기
IPK=$(ls "$IPK_OUT_DIR/"*.ipk 2>/dev/null | head -1)
if [ -z "$IPK" ]; then
    echo "ERROR: IPK 파일이 생성되지 않았습니다"
    exit 1
fi
echo "IPK 생성 완료: $IPK"
echo ""

echo "[2/3] 프로젝터에 설치..."
# ares-cli 프로파일: 'projector' (ares-setup-device로 사전 구성 필요)
ares-install -d projector "$IPK"
echo "설치 완료"
echo ""

echo "[3/3] 앱 실행..."
ares-launch -d projector "$APP_ID"
echo ""
echo "======================================"
echo "완료! 앱이 프로젝터에서 실행 중입니다"
echo "앱 ID: $APP_ID"
echo "======================================"
