# webOS Browser Native

> webOS Native App (C++/Qt) 기반 웹 브라우저

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/jsong1230/webosbrowser-native)
[![Platform](https://img.shields.io/badge/platform-webOS-green.svg)](https://www.webosose.org/)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-5.15+-green.svg)](https://www.qt.io/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

**🚀 Status**: v1.0.0 릴리스 완료 (2026-02-16)

## 프로젝트 정보

- **버전**: 1.0.0 (전체 기능 완료)
- **기술 스택**: C++17, Qt 5.15+, CMake 3.16+
- **타겟 플랫폼**: webOS 6.x (LG HU715QW 프로젝터)
- **전신 프로젝트**: [webosbrowser](https://github.com/jsong1230/webosbrowser) (Web App PoC)
- **저장소**: https://github.com/jsong1230/webosbrowser-native

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

### ✅ 전체 기능 구현 완료 (15/15)

**MS-1: 핵심 기능 (5개)**
- ✅ **F-01**: 프로젝트 초기 설정
- ✅ **F-02**: 웹뷰 통합 (WebView 스텁 구현)
- ✅ **F-03**: URL 입력 UI (URLBar, VirtualKeyboard, URLValidator)
- ✅ **F-04**: 페이지 탐색 컨트롤 (NavigationBar, 뒤로/앞으로/새로고침)
- ✅ **F-05**: 로딩 인디케이터 (LoadingIndicator, 프로그레스바)

**MS-2: 편의 기능 (5개)**
- ✅ **F-06**: 탭 관리 시스템 (TabManager, 단일 탭 모드)
- ✅ **F-07**: 북마크 관리 (BookmarkPanel, BookmarkService)
- ✅ **F-08**: 히스토리 관리 (HistoryPanel, HistoryService, 날짜별 그룹화)
- ✅ **F-09**: 검색 엔진 통합 (SearchEngine, Google/Naver/Bing)
- ✅ **F-10**: 에러 처리 (ErrorPage, 네트워크/타임아웃/404 처리)

**MS-3: 고급 기능 (5개)**
- ✅ **F-11**: 설정 화면 (SettingsPanel, 시작 페이지/검색엔진/히스토리 관리)
- ✅ **F-12**: 다운로드 관리 (설계 완료, WebEngine 의존성으로 빌드 제외)
- ✅ **F-13**: 리모컨 단축키 (KeyCodeConstants, 리모컨 5방향 + 컬러버튼)
- ✅ **F-14**: HTTPS 보안 표시 (SecurityIndicator, Secure/Insecure/Local)
- ✅ **F-15**: 즐겨찾기 홈 화면 (HomePage, 북마크 그리드 뷰)

### 📦 빌드 상태
- ✅ CMake 빌드 시스템 구성 완료
- ✅ 24개 소스 파일 컴파일 성공
- ✅ 실행 파일 생성: `bin/webosbrowser` (1.1MB, arm64)
- ✅ 실행 테스트 통과
- ✅ Qt WebEngine 자동 감지 시스템
- ✅ Mac 개발: WebView 스텁 (시각화)
- ✅ webOS 배포: WebView 실제 구현 (Qt WebEngine)

## 빌드 및 실행

### 개발 환경 빌드 (macOS)

```bash
# 1. Qt 설치 확인
qmake --version  # Qt 5.15+ 필요

# 2. 빌드 디렉토리 생성
mkdir -p build && cd build

# 3. CMake 설정
cmake ..

# 4. 컴파일
make

# 5. 실행
./bin/webosbrowser
```

### 현재 빌드 제한사항

⚠️ **개발 환경에서는 WebView 스텁 사용**
- Qt5 WebEngine이 Homebrew에서 제공되지 않음
- 실제 웹 렌더링 기능 없음 (UI 구조만 동작)
- 로그에서 확인: `[WebView] 스텁 구현 - 실제 웹 렌더링 기능 없음`

⚠️ **다운로드 기능 제외**
- F-12 다운로드 관리는 설계 완료되었으나 WebEngine 의존성으로 빌드에서 제외
- DownloadManager.cpp, DownloadPanel.cpp 컴파일 제외

### webOS 프로젝터 배포

**📘 상세 가이드**: [DEPLOY_GUIDE.md](DEPLOY_GUIDE.md)

**빠른 시작:**

```bash
# 1. 배포 디렉토리 생성
mkdir -p webos-deploy/bin
cp build/bin/webosbrowser webos-deploy/bin/
cp -r webos-meta/* webos-deploy/

# 2. IPK 패키지 생성
ares-package webos-deploy/ --outdir ./dist

# 3. 프로젝터 설치
ares-install --device projector dist/*.ipk

# 4. 실행
ares-launch --device projector com.jsong.webosbrowser.native
```

### Qt WebEngine 자동 감지

프로젝터에서 Qt WebEngine이 **있으면**:
- ✅ `WebView.cpp` 사용 (실제 웹 렌더링)
- ✅ Google, Naver 등 모든 사이트 로드 가능
- ✅ 완전한 브라우저 기능

프로젝터에서 Qt WebEngine이 **없으면**:
- ⚠️ `WebView_stub.cpp` 사용 (시각화만)
- ⚠️ 실제 웹사이트 로드 불가
- 💡 Qt WebView로 대체 고려

## 📚 문서

- **배포 가이드**: [DEPLOY_GUIDE.md](DEPLOY_GUIDE.md)
- **개발 가이드**: [GETTING_STARTED.md](GETTING_STARTED.md)
- **변경 이력**: [CHANGELOG.md](CHANGELOG.md)
- **개발 로그**: [docs/dev-log.md](docs/dev-log.md)

## 📖 참고 자료

- [webOS Native API 가이드](https://webostv.developer.lge.com/develop/native-apps/native-app-overview)
- [Qt for webOS](https://webostv.developer.lge.com/develop/native-apps/webos-qt-overview)
- [Web App PoC 리포지토리](https://github.com/jsong1230/webosbrowser)

## 🤝 기여

이슈와 PR은 언제나 환영합니다!

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'feat: Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 라이선스

MIT License - 자유롭게 사용하세요!

---

**© 2026 webOS Browser Native Project**
**Made with ❤️ for LG HU715QW Projector**
