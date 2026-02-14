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

## [0.3.0] - 2026-02-14

### F-03: URL 입력 UI 완료

#### Added (새로 추가됨)

- **URLValidator 유틸리티 클래스**
  - `src/utils/URLValidator.h/cpp`: URL 검증 및 자동 보완
  - 정적 메서드: `isValid()`, `autoComplete()`, `isSearchQuery()`, `isDomainFormat()`
  - QRegularExpression 기반 도메인 패턴 검증
  - 프로토콜 자동 추가 (http://, https://)
  - 다양한 TLD 지원 (.co.uk, .com.br, .gov.kr 등)

- **URLBar 컴포넌트 (URL 입력 필드)**
  - `src/ui/URLBar.h/cpp`: QWidget 상속 복합 위젯
  - 공개 메서드:
    - `text()`: 현재 입력된 URL 반환
    - `setText(url)`: URL 텍스트 설정
    - `setFocusToInput()`: 입력 필드에 포커스
    - `showError(message)`: 에러 메시지 표시
    - `hideError()`: 에러 메시지 숨김
  - 시그널:
    - `urlSubmitted(const QUrl &url)`: URL 제출 (Enter 키)
    - `editingCancelled()`: 입력 취소 (ESC 키)
  - 기능:
    - 포커스 시 3px 파란 테두리 하이라이트
    - Enter 키: URL 검증 → urlSubmitted 시그널 emit
    - ESC 키: 이전 URL 복원 → editingCancelled 시그널 emit
    - 유효하지 않은 URL: 에러 메시지 표시

- **VirtualKeyboard 컴포넌트 (리모컨 최적화 가상 키보드)**
  - `src/ui/VirtualKeyboard.h/cpp`: QWidget 상속 가상 키보드
  - QWERTY 레이아웃 (44개 키):
    - 행 0: 숫자 1-0 + `-`
    - 행 1: qwertyuiop + `/`
    - 행 2: asdfghjkl + `:` + `.`
    - 행 3: zxcvbnm + `?` + `&` + `=` + `_`
    - 제어 키: Space, Backspace, Enter, Cancel
  - 시그널:
    - `characterEntered(const QString &character)`: 문자 입력
    - `backspacePressed()`: 백스페이스
    - `enterPressed()`: Enter 키
    - `spacePressed()`: 스페이스
    - `closeRequested()`: 키보드 닫기 (ESC/Back)
  - 리모컨 최적화:
    - Qt::Key_Up/Down/Left/Right: 그리드 내 포커스 이동 (순환)
    - Qt::Key_Select: 버튼 클릭
    - Qt::Key_Escape/Back: 키보드 닫기
    - 포커스된 키: 3px 파란 테두리

- **URLBar + VirtualKeyboard 통합**
  - Select 키로 가상 키보드 표시
  - 가상 키보드에서 문자 입력 → URLBar 텍스트 필드에 반영
  - Backspace: 마지막 문자 삭제
  - Enter: URL 제출
  - ESC: 키보드 닫기, 입력 취소

- **BrowserWindow 통합**
  - URLBar를 BrowserWindow 상단에 배치 (WebView 위)
  - 시그널/슬롯 연결:
    - URLBar::urlSubmitted → WebView::load (URL 로드)
    - WebView::urlChanged → URLBar::setText (현재 URL 표시)
    - WebView::loadError → URLBar::showError (에러 메시지)

- **테스트 코드 (120개 테스트, 1,505줄)**
  - `tests/unit/URLValidatorTest.cpp`: 43개 테스트
    - URL 검증 (프로토콜, 경로, 쿼리 문자열)
    - 자동 보완 (프로토콜 추가, www, HTTPS 유지)
    - 검색어 판단 (공백, 단어, 특수문자)
    - 도메인 형식 검증
    - 엣지 케이스 (빈 문자열, 다중 TLD, URL 인코딩)
  - `tests/unit/URLBarTest.cpp`: 32개 테스트
    - 입력 필드 (setText, getText, 특수문자)
    - URL 제출 (urlSubmitted 시그널)
    - 에러 처리 (showError, hideError)
    - 입력 취소 (ESC/Back, 이전 URL 복원)
    - 포커스 관리 (setFocusToInput, focusInEvent, focusOutEvent)
    - 엣지 케이스 (긴 URL, 유니코드, 이모지, 반복 작업)
  - `tests/integration/BrowserWindowIntegrationTest.cpp`: 45개 테스트
    - URLBar ↔ WebView 시그널/슬롯 연결
    - 레이아웃 검증
    - URL 로드 시나리오 (유효 URL, 자동 보완, 에러)
    - 다중 URL 처리
    - 포커스 네비게이션
    - 성능 테스트 (100개 URL < 10초)
    - 안정성 테스트 (메모리 누수 없음)

#### Changed (변경됨)

- **CMakeLists.txt**
  - `src/ui/URLBar.cpp`, `src/ui/VirtualKeyboard.cpp` 추가
  - `src/utils/URLValidator.cpp` 추가
  - Qt5 Widgets 모듈 의존성 확인

- **BrowserWindow 클래스**
  - `URLBar *urlBar_` 멤버 변수 추가
  - `setupUI()`: URLBar 인스턴스 생성 및 레이아웃 추가
  - `setupConnections()`: URLBar → WebView 시그널/슬롯 연결

#### Improved (개선됨)

- URL 검증 로직: 강화된 정규표현식으로 다양한 도메인 형식 지원
  - 다중 서브도메인 (api.v1.example.com)
  - 다양한 TLD (.co.uk, .com.br, .gov.kr)
  - 하이픈, 숫자 포함 도메인
  - IP 주소 (192.168.1.1)
- VirtualKeyboard 포커스: 2D 배열 기반 순환 이동으로 직관적 네비게이션
- URLBar 에러 처리: 명확한 에러 메시지 표시, 3초 후 자동 숨김
- 포커스 경리: focusInEvent에서 previousUrl_ 저장하여 입력 취소 시 복원

#### Test (테스트)

- ✅ 120개 테스트 코드 작성 (1,505줄)
  - URLValidator: 43개 테스트 (URL 검증, 자동 보완, 엣지 케이스)
  - URLBar: 32개 테스트 (입력, 제출, 에러, 포커스)
  - BrowserWindow: 45개 테스트 (시그널, 시나리오, 성능)
- ⏳ CMake 빌드 및 테스트 실행 대기 (Qt 5.15+, Google Test 필요)
- ⏳ 실제 디바이스 테스트 대기 (LG 프로젝터 HU715QW)

#### Notes

- 자동완성 기능 (Phase 4): F-07, F-08 완료 후 추가 예정
  - searchAutocomplete() 스켈레톤 코드 작성
  - HistoryService, BookmarkService 주입 메서드 준비
- 향후 확장:
  - F-09 검색 엔진 통합: URLValidator::isSearchQuery() 활용
  - F-14 HTTPS 보안 표시: URLBar에 보안 아이콘 추가

---

## 주요 마일스톤

### 완료 (✅)
- F-01: 프로젝트 초기 설정
- F-02: 웹뷰 통합
- F-03: URL 입력 UI

### 진행 중 (🔄)
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
