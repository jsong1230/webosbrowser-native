# Getting Started - webOS Browser Native

프로젝트 빌드 및 실행 가이드입니다.

## 1. 개발 환경 요구사항

### 필수 도구

- **Qt 5.15+**: GUI 프레임워크
- **CMake 3.16+**: 빌드 시스템
- **C++ 컴파일러**: Clang 또는 GCC (C++17 지원)

### macOS 설치

```bash
# Qt 5 설치
brew install qt@5

# PATH 설정
echo 'export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc

# 설치 확인
qmake --version
# 출력 예시: QMake version 3.1, Using Qt version 5.15.x

# CMake 설치
brew install cmake
cmake --version
# 출력 예시: cmake version 3.x.x
```

### webOS Native SDK (실제 디바이스 배포용)

webOS 프로젝터에 배포하려면 webOS Native SDK 필요:
- 다운로드: https://webostv.developer.lge.com/develop/tools/sdk-download
- 개발 환경 빌드에는 불필요

## 2. 빌드 및 실행

### 빌드 (macOS)

```bash
# 1. 프로젝트 디렉토리로 이동
cd /Users/jsong/dev/jsong1230-github/webosbrowser-native

# 2. 빌드 디렉토리 생성
mkdir -p build && cd build

# 3. CMake 설정
cmake ..

# 4. 컴파일 (24개 소스 파일)
make

# 5. 빌드 성공 확인
ls -lh bin/webosbrowser
# 출력 예시: -rwxr-xr-x  1 user  staff   1.1M  webosbrowser
```

### 실행

```bash
# build 디렉토리에서
./bin/webosbrowser
```

**정상 실행 시 콘솔 출력:**
```
webOS Browser Native App 시작...
SecurityIndicator: Secure 아이콘 로드 실패
SecurityIndicator: Insecure 아이콘 로드 실패
SecurityIndicator: Local 아이콘 로드 실패
URLBar: 생성 중...
```

⚠️ **아이콘 로드 실패는 정상입니다** (리소스 파일이 빌드 디렉토리에 복사되지 않음)

### 현재 빌드 제한사항

**WebView 스텁 사용**
- Qt5 WebEngine이 Homebrew에서 제공되지 않음
- 실제 웹 렌더링 기능 없음 (UI 구조만 동작)
- `[WebView] 스텁 구현 - 실제 웹 렌더링 기능 없음` 로그 출력

**다운로드 기능 제외**
- F-12 다운로드 관리는 WebEngine 의존성으로 빌드에서 제외
- 설계는 완료되었으나 실제 webOS 환경에서만 구현 가능

**북마크/히스토리 임시 저장**
- 현재 QSettings 사용 (메모리 기반)
- 실제 webOS에서는 LS2 DB8 API로 교체 필요

## 3. 프로젝트 현황

### 구현 완료 (15/15 기능)

**MS-1: 핵심 기능**
- F-01: 프로젝트 초기 설정
- F-02: 웹뷰 통합 (스텁)
- F-03: URL 입력 UI
- F-04: 페이지 탐색 컨트롤
- F-05: 로딩 인디케이터

**MS-2: 편의 기능**
- F-06: 탭 관리 시스템
- F-07: 북마크 관리
- F-08: 히스토리 관리
- F-09: 검색 엔진 통합
- F-10: 에러 처리

**MS-3: 고급 기능**
- F-11: 설정 화면
- F-12: 다운로드 관리 (설계 완료, 빌드 제외)
- F-13: 리모컨 단축키
- F-14: HTTPS 보안 표시
- F-15: 즐겨찾기 홈 화면

### 문서 구조

```
docs/
├── project/               # 프로젝트 기획
│   ├── prd.md
│   ├── features.md
│   └── roadmap.md
├── specs/                 # 15개 기능 설계서
│   ├── {기능명}/
│   │   ├── requirements.md
│   │   ├── design.md
│   │   └── plan.md
├── api/                   # API 스펙
├── db/                    # DB 스키마
├── components/            # 컴포넌트 문서
├── test-reports/          # 테스트 리포트
└── dev-log.md            # 개발 진행 로그
```

---

## 4. webOS 디바이스 배포

실제 webOS 프로젝터에 배포하려면 다음 구현이 필요합니다.

### 필요한 실제 구현

**1. WebView API 통합**
```cpp
// src/browser/WebView_stub.cpp → src/browser/WebView.cpp
// webOS WebView API 사용
#include <WebOSWebView>

void WebView::load(const QUrl &url) {
    // WAM(Web Application Manager) 연동
    webosWebView_->loadUrl(url.toString());
}
```

**2. LS2 API 통합**
```cpp
// src/services/StorageService.cpp
// QSettings → LS2 DB8 API
#include <luna-service2/lunaservice.h>

void StorageService::putData(...) {
    // LS2 call to com.webos.service.db/put
}
```

**3. 다운로드 기능 복원**
```cpp
// CMakeLists.txt에 다시 추가:
// src/services/DownloadManager.cpp
// src/ui/DownloadPanel.cpp
```

### IPK 패키징

```bash
# 1. webOS용 빌드 (크로스 컴파일)
ares-package build/

# 2. 디바이스 등록
ares-setup-device

# 3. 설치
ares-install --device projector \
  com.jsong.webosbrowser.native_1.0.0_arm64.ipk

# 4. 실행
ares-launch --device projector \
  com.jsong.webosbrowser.native

# 5. 로그 확인
ares-log --device projector -f com.jsong.webosbrowser.native
```

### webos-meta/appinfo.json 확인

```json
{
  "id": "com.jsong.webosbrowser.native",
  "version": "1.0.0",
  "vendor": "jsong",
  "type": "native",
  "main": "webosbrowser",
  "title": "webOS Browser",
  "icon": "icon.png",
  "largeIcon": "largeIcon.png"
}
```

---

## 5. 참고 자료

### 공식 문서
- [webOS Native API](https://webostv.developer.lge.com/develop/native-apps/native-app-overview)
- [Qt for webOS](https://webostv.developer.lge.com/develop/native-apps/webos-qt-overview)
- [LS2 API Guide](https://webostv.developer.lge.com/develop/native-apps/using-ls2-api)
- [Qt 5 Documentation](https://doc.qt.io/qt-5/)

### 프로젝트 문서
- Web App PoC: https://github.com/jsong1230/webosbrowser
- 설계 문서: `docs/specs/`
- 개발 로그: `docs/dev-log.md`
- 변경 이력: `CHANGELOG.md`

---

**빌드 완료!** 개발 환경에서 UI 구조는 확인 가능합니다.
실제 웹 렌더링은 webOS 디바이스에서 WebView API 통합 후 가능합니다.
