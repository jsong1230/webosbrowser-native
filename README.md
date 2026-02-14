# webOS Browser Native

> webOS Native App (C++/Qt) 기반 웹 브라우저

## 프로젝트 정보

- **버전**: 0.2.0
- **기술 스택**: C++, Qt 5.x, webOS Native SDK
- **타겟 플랫폼**: webOS 6.x (LG HU715QW 프로젝터)
- **전신 프로젝트**: [webosbrowser](https://github.com/jsong1230/webosbrowser) (Web App PoC)

## 프로젝트 배경

이 프로젝트는 webOS Web App의 기술적 제약(iframe 외부 사이트 로드 불가)을 극복하기 위해 Native App으로 전환한 버전입니다.

Web App PoC에서 작성된 설계 문서(`docs/specs/`)를 기반으로 실제 웹 브라우저 기능을 구현합니다.

## 개발 환경 요구사항

### 필수 도구

- **Qt 5.x**: GUI 프레임워크
- **CMake**: 빌드 시스템
- **webOS Native SDK**: Native App 빌드 및 배포
- **C++ 컴파일러**: Clang 또는 GCC

### 설치

```bash
# Qt 설치
brew install qt@5

# CMake 설치
brew install cmake

# webOS Native SDK
# LG 공식 사이트에서 다운로드:
# https://webostv.developer.lge.com/develop/tools/sdk-download
```

## 프로젝트 구조

```
webosbrowser-native/
├── src/               # C++ 소스 코드
│   ├── main.cpp
│   ├── browser/       # 브라우저 코어
│   ├── ui/            # Qt UI 컴포넌트
│   └── services/      # 비즈니스 로직
├── include/           # 헤더 파일
├── resources/         # 리소스 (아이콘, QML 등)
├── webos-meta/        # webOS 메타데이터
├── docs/              # 설계 문서 (Web App PoC에서 복사)
│   ├── project/       # PRD, 기능 백로그, 로드맵
│   └── specs/         # 15개 기능 설계서
├── CMakeLists.txt     # CMake 빌드 설정
└── README.md
```

## 개발 현황

### 완료된 기능
- ✅ **F-01**: 프로젝트 초기 설정 (CMake, Qt 프로젝트 구조)
- ✅ **F-02**: 웹뷰 통합 (WebView 클래스, PIMPL 패턴, BrowserWindow 통합)

### 진행 중인 기능
- 🔄 **F-03**: URL 입력 UI (예정)
- 🔄 **F-04**: 페이지 탐색 컨트롤 (뒤로/앞으로 버튼, 예정)
- 🔄 **F-05**: 로딩 인디케이터 (프로그레스바, 예정)

### 계획 중인 기능
- **F-06~F-15**: 탭 관리, 북마크, 히스토리, 다운로드, 설정, 검색, 단축키, 에러 처리, 보안, 홈 화면

## F-02: 웹뷰 통합 개요

webOS Native SDK의 Qt WebEngineView를 활용하여 Chromium 기반 웹 페이지 렌더링을 구현했습니다.

### 주요 기능
- **WebView 클래스**: PIMPL 패턴으로 QWebEngineView 캡슐화
- **로딩 이벤트**: loadStarted, loadProgress, loadFinished 시그널
- **네비게이션**: goBack(), goForward() API
- **타임아웃 메커니즘**: 30초 초과 시 로딩 중단 및 에러 시그널
- **상태 관리**: LoadingState enum (Idle, Loading, Loaded, Error)

### 빌드 및 실행
```bash
# 빌드
mkdir -p build && cd build
cmake ..
make

# 실행 (macOS)
./webosbrowser
```

초기 URL(https://www.google.com)이 자동으로 로드됩니다.

### 다음 단계

1. **F-03 URL 입력 UI**: WebView::load(QUrl) API를 활용한 사용자 입력 UI
2. **F-04 페이지 탐색**: WebView::goBack()/goForward() API를 활용한 네비게이션 UI
3. **F-05 로딩 인디케이터**: WebView::loadProgress 시그널을 활용한 프로그레스바

## 참고 자료

- [webOS Native API 가이드](https://webostv.developer.lge.com/develop/native-apps/native-app-overview)
- [Qt for webOS](https://webostv.developer.lge.com/develop/native-apps/webos-qt-overview)
- [Web App PoC 리포지토리](https://github.com/jsong1230/webosbrowser)

---

**© 2026 webOS Browser Native Project**
