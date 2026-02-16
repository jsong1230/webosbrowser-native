# webOS 프로젝터 배포 가이드

## 📦 IPK 패키징

### 1. 빌드 파일 복사

```bash
# webOS 배포 디렉토리 생성
mkdir -p webos-deploy/bin

# 실행 파일 복사
cp build/bin/webosbrowser webos-deploy/bin/

# 메타데이터 복사
cp -r webos-meta/* webos-deploy/

# 권한 설정
chmod +x webos-deploy/bin/webosbrowser
```

### 2. IPK 생성

```bash
# IPK 패키지 생성
ares-package webos-deploy/ \
  --outdir ./dist \
  --pkgid com.jsong.webosbrowser.native

# 결과: dist/com.jsong.webosbrowser.native_1.0.0_all.ipk
```

## 🎯 프로젝터 설치

### 1. 디바이스 설정

```bash
# 프로젝터 추가 (처음 한 번만)
ares-setup-device

# 디바이스 이름: projector
# IP 주소: [프로젝터 IP]
# 포트: 9922
# 사용자: prisoner
# 비밀번호: [공백]
```

### 2. 앱 설치

```bash
# IPK 설치
ares-install --device projector \
  dist/com.jsong.webosbrowser.native_1.0.0_all.ipk

# 설치 확인
ares-install --device projector --list
```

### 3. 앱 실행

```bash
# 앱 실행
ares-launch --device projector com.jsong.webosbrowser.native

# 실행 로그 확인
ares-launch --device projector com.jsong.webosbrowser.native --inspect
```

## 🔍 Qt WebEngine 확인

### 프로젝터에서 Qt WebEngine 사용 가능 여부 확인

```bash
# 프로젝터 SSH 접속
ares-shell -d projector

# Qt 라이브러리 확인
ls /usr/lib/libQt5WebEngine*

# 있으면: 실제 웹 렌더링 가능 ✅
# 없으면: 스텁만 작동 ⚠️
```

## ⚙️ 트러블슈팅

### Qt WebEngine이 없는 경우

프로젝터에 Qt WebEngine이 없다면:

**옵션 A: Qt WebView 사용 (경량)**
- Qt WebView는 시스템 브라우저 사용
- 더 가벼운 대안

**옵션 B: webOS Web App으로 전환**
- Native app 대신 Web App 사용
- 전신 프로젝트: https://github.com/jsong1230/webosbrowser

### 권한 오류

```bash
# 권한 추가 (appinfo.json)
"requiredPermissions": [
  "network.operation",
  "storage.operation",
  "applications.operation",
  "webos.service"  # 추가 필요 시
]
```

### 빌드 오류

프로젝터에서 직접 빌드 필요 시:

```bash
# 프로젝터 SSH 접속
ares-shell -d projector

# CMake 빌드
cd /tmp
git clone [repository]
cd webosbrowser-native
mkdir build && cd build
cmake ..
make
```

## 📊 테스트

### 기본 테스트

1. 앱 실행 확인
2. URL 입력 (google.com)
3. 웹 페이지 로딩 확인
4. 탐색 버튼 테스트 (뒤로/앞으로)
5. 북마크 추가/삭제
6. 히스토리 확인

### 성능 테스트

- YouTube 재생
- Naver 검색
- 다중 탭 전환
- 장시간 사용 (메모리 누수 확인)

## 🚀 빠른 배포 스크립트

```bash
#!/bin/bash
# deploy.sh

echo "=== webOS 브라우저 배포 ==="

# 1. 빌드
echo "1. 빌드 중..."
mkdir -p webos-deploy/bin
cp build/bin/webosbrowser webos-deploy/bin/
cp -r webos-meta/* webos-deploy/
chmod +x webos-deploy/bin/webosbrowser

# 2. 패키징
echo "2. IPK 생성 중..."
mkdir -p dist
ares-package webos-deploy/ --outdir ./dist

# 3. 설치
echo "3. 프로젝터에 설치 중..."
ares-install --device projector dist/*.ipk

# 4. 실행
echo "4. 앱 실행..."
ares-launch --device projector com.jsong.webosbrowser.native

echo "=== 배포 완료 ==="
```

사용법:
```bash
chmod +x deploy.sh
./deploy.sh
```

## 📝 배포 체크리스트

- [ ] 빌드 성공 확인
- [ ] appinfo.json 버전 확인
- [ ] 아이콘 파일 준비
- [ ] 프로젝터 네트워크 연결
- [ ] IPK 생성
- [ ] 앱 설치
- [ ] 실행 테스트
- [ ] Qt WebEngine 동작 확인
- [ ] 주요 사이트 로딩 테스트
- [ ] 성능 모니터링

## 🎉 성공 기준

✅ 앱이 정상 실행됨
✅ Google.com 로딩 성공
✅ URL 입력 및 탐색 정상 동작
✅ 북마크/히스토리 기능 정상
✅ 리모컨 조작 가능
✅ 10분 이상 안정적 동작

---

**문제 발생 시**: GitHub Issues에 보고
**저장소**: https://github.com/jsong1230/webosbrowser-native
