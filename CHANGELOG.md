# 변경 이력

모든 주요 프로젝트 변경 사항이 이 파일에 기록됩니다.

이 프로젝트는 [Semantic Versioning](https://semver.org/lang/ko/)을 따릅니다.

---

## [0.2.0] - 2026-02-14

### F-02: 웹뷰 통합 (WebView Integration) 완료

#### Added (새로 추가됨)

- **WebView 클래스 (PIMPL 패턴)**
  - `src/browser/WebView.h`: 공개 인터페이스 (5.2KB)
  - `src/browser/WebView.cpp`: PIMPL 구현 (12KB)
  - LoadingState enum (Idle, Loading, Loaded, Error)

- **웹뷰 API**
  - `load(QUrl)`, `load(QString)`: 페이지 로딩
  - `reload()`: 새로고침
  - `stop()`: 로딩 중지
  - `goBack()`, `goForward()`: 네비게이션
  - `canGoBack()`, `canGoForward()`: 네비게이션 가능 여부
  - `url()`: 현재 URL 조회
  - `title()`: 페이지 제목 조회
  - `loadingState()`: 현재 로딩 상태 조회
  - `loadProgress()`: 로딩 진행률 조회

- **웹뷰 시그널 (이벤트)**
  - `loadStarted()`: 페이지 로딩 시작
  - `loadProgress(int)`: 로딩 진행률 변경 (0~100)
  - `loadFinished(bool)`: 페이지 로딩 완료 또는 실패
  - `titleChanged(QString)`: 페이지 제목 변경
  - `urlChanged(QUrl)`: URL 변경 (링크 클릭, 리다이렉트 등)
  - `loadError(QString)`: 로딩 에러 발생
  - `loadTimeout()`: 로딩 타임아웃 (30초 초과)

- **타임아웃 메커니즘**
  - 30초 이상 로딩 중단 감지
  - loadTimeout() 시그널 emit

- **BrowserWindow 통합**
  - WebView를 중앙 영역에 배치
  - 초기 URL 자동 로드 (https://www.google.com)
  - 웹뷰 이벤트 로깅 (qDebug)

#### Changed (변경됨)

- **CMakeLists.txt**
  - Qt5::WebEngineWidgets 모듈 의존성 추가
  - WebView.cpp 소스 파일 추가

- **BrowserWindow 클래스**
  - `WebView *webView_` 멤버 변수 추가
  - 레이아웃에 WebView 추가 (중앙 영역, stretch=1)

#### Improved (개선됨)

- 모든 공개 API에 Doxygen 주석 추가 (한국어)
- PIMPL 패턴으로 구현 세부사항 캡슐화
- Qt 시그널/슬롯 메커니즘으로 느슨한 결합 구현
- 스마트 포인터(QScopedPointer) 사용으로 메모리 안전성 보장

#### Test (테스트)

- ✅ CMake 빌드 성공 (macOS 개발 환경)
- ✅ 로컬 실행 성공
- ✅ Google 홈페이지 렌더링 확인
- ✅ 로딩 이벤트 시그널 정상 작동
- ⏳ 실제 디바이스 테스트 대기 (LG 프로젝터 HU715QW)

---

## [0.1.0] - 2026-02-14

### 초기 프로젝트 설정 (F-01)

#### Added

- 프로젝트 초기 구조 및 디렉토리 생성
- CMakeLists.txt 기본 설정 (Qt, C++17)
- BrowserWindow 스켈레톤 구현
- main.cpp 진입점
- README.md 프로젝트 개요
- CLAUDE.md 코딩 컨벤션 및 개발 규칙
- docs/project/ - 프로젝트 계획 문서
  - prd.md: 제품 요구사항 정의
  - features.md: 15개 기능 목록
  - roadmap.md: 개발 로드맵
- docs/specs/ - 15개 기능 설계서
- .claude/ - Claude Code 자동화 설정

#### Documentation

- Web App PoC 설계를 Native App으로 전환
- CLAUDE.md: Git 규칙, 코딩 컨벤션, 에이전트 라우팅 규칙 문서화

---

## 주요 마일스톤

### 완료 (✅)
- F-01: 프로젝트 초기 설정
- F-02: 웹뷰 통합

### 진행 중 (🔄)
- F-03: URL 입력 UI
- F-04: 페이지 탐색 컨트롤
- F-05: 로딩 인디케이터

### 계획 중 (📅)
- F-06: 탭 관리
- F-07: 북마크 관리
- F-08: 히스토리 관리
- F-09: 다운로드 관리
- F-10: 설정 기능
- F-11: 페이지 내 검색
- F-12: 리모컨 단축키
- F-13: 에러 처리
- F-14: 보안 표시
- F-15: 홈 화면

---

## 버전 전략

- **0.x.y**: 베타 단계 (기능 개발 진행 중)
- **1.0.0**: 최초 정식 릴리스 (15개 기능 모두 완료)

### 버전 번호 규칙
- **MAJOR** (x): 대규모 기능 추가 또는 API 변경
- **MINOR** (y): 기능 추가, 하위 호환 유지
- **PATCH** (z): 버그 수정, 성능 개선

---

## 참고

- **프로젝트 저장소**: https://github.com/jsong1230/webosbrowser-native
- **전신 프로젝트**: https://github.com/jsong1230/webosbrowser (Web App PoC)
- **개발 환경**: macOS, Qt 5.15+, C++17, CMake 3.16+
- **타겟 플랫폼**: webOS 6.x (LG HU715QW 프로젝터)
