# 페이지 탐색 컨트롤 — 구현 계획서

## 1. 참조 문서
- **요구사항 분석서**: `docs/specs/navigation-controls/requirements.md`
- **기술 설계서**: `docs/specs/navigation-controls/design.md`
- **F-02 설계서**: `docs/specs/webview-integration/design.md` (WebView API)
- **프로젝트 규칙**: `CLAUDE.md`

## 2. 구현 Phase

### Phase 1: NavigationBar 클래스 구현 (Core)
**목표**: NavigationBar 위젯의 핵심 UI 및 버튼 동작 구현

#### 태스크
- [ ] **Task 1.1**: NavigationBar.h 확장 (기존 스켈레톤 파일 수정)
  - 담당: cpp-dev
  - 작업 내용:
    - 멤버 변수 추가 (버튼 4개, 레이아웃, WebView 포인터)
    - 공개/비공개 메서드 시그니처 작성 (setWebView, updateButtonStates, 버튼 클릭 핸들러)
    - Qt 시그널/슬롯 매크로 설정 (Q_OBJECT, public slots, private slots)
  - 산출물: `src/ui/NavigationBar.h` (수정)
  - 완료 기준: 컴파일 통과 (헤더 단독), 모든 메서드 선언 포함

- [ ] **Task 1.2**: NavigationBar.cpp 기본 구조 구현
  - 담당: cpp-dev
  - 작업 내용:
    - 생성자/소멸자 구현 (setupUI, setupConnections 호출)
    - setupUI() 구현 (버튼 4개 생성, QHBoxLayout 구성, 최소 크기 설정)
    - setupFocusOrder() 구현 (setTabOrder로 좌→우 포커스 체인)
    - applyStyles() 구현 (QSS 스타일시트 정의 및 적용)
  - 산출물: `src/ui/NavigationBar.cpp` (신규)
  - 완료 기준: NavigationBar 인스턴스 생성 시 버튼 4개 표시, 리모컨 방향키로 포커스 이동 확인

- [ ] **Task 1.3**: 버튼 클릭 핸들러 구현
  - 담당: cpp-dev
  - 작업 내용:
    - onBackClicked() 구현 (WebView::goBack() 호출, nullptr 체크)
    - onForwardClicked() 구현 (WebView::goForward() 호출)
    - onReloadClicked() 구현 (WebView::reload() 호출)
    - onHomeClicked() 구현 (WebView::load(DEFAULT_HOME_URL) 호출)
    - setupConnections() 구현 (버튼 clicked 시그널 → 핸들러 슬롯 연결)
  - 산출물: `src/ui/NavigationBar.cpp` (추가)
  - 완료 기준: 각 버튼 클릭 시 콘솔 로그 출력 (`qDebug()`), WebView API 호출 확인

**예상 소요 시간**: 4시간

**완료 기준**:
- NavigationBar 인스턴스를 독립적으로 생성하여 버튼 UI 표시 가능
- 버튼 클릭 시 WebView 메서드 호출 (실제 페이지 이동은 Phase 2에서 검증)
- 리모컨 좌/우 방향키로 버튼 간 포커스 이동 동작
- QSS 스타일시트 적용 (포커스 테두리, 버튼 크기)

---

### Phase 2: WebView 연동 및 상태 동기화
**목표**: NavigationBar와 WebView의 시그널/슬롯 연결, 버튼 상태 자동 업데이트

#### 태스크
- [ ] **Task 2.1**: setWebView() 메서드 구현
  - 담당: cpp-dev
  - 작업 내용:
    - 기존 WebView 시그널 연결 해제 (`disconnect`)
    - 새 WebView 시그널 연결 (urlChanged, loadStarted, loadFinished → updateButtonStates)
    - nullptr 처리 (WebView 없을 시 모든 버튼 비활성화)
  - 산출물: `src/ui/NavigationBar.cpp` (추가)
  - 완료 기준: setWebView(nullptr) 호출 시 버튼 모두 비활성화, setWebView(validPtr) 호출 시 시그널 연결 확인

- [ ] **Task 2.2**: updateButtonStates() 메서드 구현
  - 담당: cpp-dev
  - 작업 내용:
    - WebView::canGoBack(), canGoForward() 호출
    - 결과 값으로 backButton_->setEnabled(), forwardButton_->setEnabled() 설정
    - 새로고침/홈 버튼은 항상 활성화 (WebView가 유효할 때만)
    - nullptr 체크 및 에러 처리
  - 산출물: `src/ui/NavigationBar.cpp` (추가)
  - 완료 기준:
    - WebView URL 변경 시 버튼 상태 자동 업데이트
    - 첫 페이지에서 뒤로 버튼 비활성화 (opacity 0.5 스타일)
    - 뒤로 간 후 앞으로 버튼 활성화

**예상 소요 시간**: 3시간

**의존성**: Phase 1 완료 (NavigationBar 기본 구현), F-02 완료 (WebView API)

**완료 기준**:
- NavigationBar::setWebView(webViewPtr) 호출 후 시그널 연결 확인
- 웹 페이지 이동 시 버튼 활성/비활성 상태 자동 변경
- 테스트 시나리오 실행: Google → YouTube 검색 → 뒤로 → 앞으로

---

### Phase 3: BrowserWindow 통합
**목표**: NavigationBar를 BrowserWindow에 통합, 리모컨 Back 키 처리

#### 태스크
- [ ] **Task 3.1**: BrowserWindow.h 수정 (NavigationBar 멤버 추가)
  - 담당: cpp-dev
  - 작업 내용:
    - Forward declaration 추가 (`class NavigationBar;`)
    - 멤버 변수 추가 (`NavigationBar *navBar_;`)
    - keyPressEvent() 메서드 선언 추가 (오버라이드)
  - 산출물: `src/browser/BrowserWindow.h` (수정)
  - 완료 기준: 컴파일 통과

- [ ] **Task 3.2**: BrowserWindow.cpp setupUI() 수정 (NavigationBar 생성)
  - 담당: cpp-dev
  - 작업 내용:
    - NavigationBar 인스턴스 생성 (`navBar_ = new NavigationBar(this);`)
    - mainLayout_에 NavigationBar 추가 (URLBar 다음, WebView 이전 위치)
    - 레이아웃 순서: URLBar (F-03 예정) → NavigationBar → WebView → StatusLabel
  - 산출물: `src/browser/BrowserWindow.cpp` (수정)
  - 완료 기준: BrowserWindow 실행 시 NavigationBar 표시 확인

- [ ] **Task 3.3**: BrowserWindow.cpp setupConnections() 수정 (WebView 연결)
  - 담당: cpp-dev
  - 작업 내용:
    - `navBar_->setWebView(webView_);` 호출 (WebView 인스턴스 전달)
  - 산출물: `src/browser/BrowserWindow.cpp` (수정)
  - 완료 기준: 앱 실행 후 페이지 이동 시 버튼 상태 자동 업데이트 확인

- [ ] **Task 3.4**: BrowserWindow::keyPressEvent() 구현 (리모컨 Back 키)
  - 담당: cpp-dev
  - 작업 내용:
    - keyPressEvent(QKeyEvent*) 메서드 오버라이드
    - Qt::Key_Backspace 또는 Qt::Key_Escape 감지
    - WebView에 포커스 있을 때: `webView_->goBack()` 호출
    - NavigationBar에 포커스 있을 때: `event->ignore()` (포커스 이탈)
    - canGoBack() == false일 때: 이벤트 무시 (앱 종료 방지)
  - 산출물: `src/browser/BrowserWindow.cpp` (추가)
  - 완료 기준:
    - WebView에 포커스 시 리모컨 Back 키로 브라우저 뒤로 가기
    - NavigationBar에 포커스 시 Back 키로 포커스 이탈
    - 첫 페이지에서 Back 키 눌러도 앱 종료 안 됨

**예상 소요 시간**: 3시간

**의존성**: Phase 2 완료 (NavigationBar ↔ WebView 연동)

**완료 기준**:
- BrowserWindow 실행 시 NavigationBar가 WebView 위에 표시
- 모든 버튼 동작 정상 (뒤로, 앞으로, 새로고침, 홈)
- 리모컨 Back 키 처리 정상 동작

---

### Phase 4: 스타일링 및 UX 최적화
**목표**: 프로젝터 환경에 맞는 버튼 크기, 아이콘, 포커스 스타일 적용

#### 태스크
- [ ] **Task 4.1**: QSS 스타일시트 최적화
  - 담당: cpp-dev
  - 작업 내용:
    - 버튼 크기 조정 (최소 100x80px, `setMinimumSize`)
    - 아이콘 폰트 크기 조정 (32pt, 유니코드 심볼 크기)
    - 포커스 테두리 스타일 (2px, white, border-radius 8px)
    - 비활성 버튼 스타일 (opacity 0.5, color #888888)
    - 색상 대비 검증 (WCAG 2.1 AA 기준 4.5:1)
  - 산출물: `src/ui/NavigationBar.cpp` (applyStyles() 수정)
  - 완료 기준: 3m 거리에서 버튼 아이콘 명확히 인식 가능

- [ ] **Task 4.2**: 접근성 설정
  - 담당: cpp-dev
  - 작업 내용:
    - 각 버튼에 setAccessibleName(), setAccessibleDescription() 설정
    - 한국어 접근성 레이블 추가 ("뒤로 가기", "앞으로 가기" 등)
  - 산출물: `src/ui/NavigationBar.cpp` (setupUI() 수정)
  - 완료 기준: webOS 접근성 도구로 레이블 확인 (선택 사항)

**예상 소요 시간**: 2시간

**의존성**: Phase 3 완료 (BrowserWindow 통합)

**완료 기준**:
- 버튼 크기, 간격, 아이콘 크기 NFR-2 요구사항 충족
- 포커스 테두리 명확히 표시 (흰색 2px)
- 비활성 버튼 시각적 구분 명확 (opacity 0.5)

---

### Phase 5: 단위 테스트 작성
**목표**: NavigationBar 클래스의 단위 테스트 작성 (Google Test + Qt Test)

#### 태스크
- [ ] **Task 5.1**: NavigationBar 단위 테스트 작성
  - 담당: test-runner
  - 작업 내용:
    - InitialState 테스트 (WebView 없을 시 버튼 비활성)
    - UpdateButtonStates 테스트 (MockWebView로 canGoBack/Forward 모킹)
    - BackButtonClick 테스트 (goBack() 호출 검증)
    - NullWebViewSafety 테스트 (nullptr 안전성)
    - FocusOrder 테스트 (setTabOrder 동작 확인)
  - 산출물: `tests/unit/NavigationBarTest.cpp` (신규)
  - 완료 기준: 모든 테스트 통과 (5개 이상)

**예상 소요 시간**: 3시간

**의존성**: Phase 4 완료 (NavigationBar 완전 구현)

**완료 기준**:
- 단위 테스트 커버리지 80% 이상
- CI/CD 파이프라인에서 자동 실행 가능

---

### Phase 6: 통합 테스트 및 디바이스 검증
**목표**: BrowserWindow와 NavigationBar의 통합 테스트, LG 프로젝터 실기 테스트

#### 태스크
- [ ] **Task 6.1**: BrowserWindow 통합 테스트 작성
  - 담당: test-runner
  - 작업 내용:
    - NavigationBarIntegration 테스트 (BrowserWindow에서 NavigationBar 정상 동작)
    - RemoteBackKeyHandling 테스트 (리모컨 Back 키 시뮬레이션)
    - ButtonStateSync 테스트 (WebView URL 변경 → 버튼 상태 동기화)
  - 산출물: `tests/integration/BrowserWindowTest.cpp` (추가)
  - 완료 기준: 모든 테스트 통과 (3개 이상)

- [ ] **Task 6.2**: LG 프로젝터 HU715QW 실기 테스트
  - 담당: test-runner
  - 작업 내용:
    - TC-1: 뒤로 버튼 동작 (Google → YouTube 검색 → 뒤로)
    - TC-2: 리모컨 포커스 이동 (좌/우 방향키)
    - TC-3: 새로고침 동작 (YouTube 페이지 재로드)
    - TC-4: 홈 버튼 동작 (Google 홈페이지 이동)
    - TC-5: 리모컨 Back 키 (WebView 포커스 시 뒤로 가기)
    - TC-6: 비활성 버튼 스타일 (opacity 0.5 확인)
  - 산출물: 테스트 리포트 (`docs/test-reports/F-04_device_test.md`)
  - 완료 기준: 모든 테스트 케이스 통과, 스크린샷 3장 이상

**예상 소요 시간**: 4시간

**의존성**: Phase 5 완료 (단위 테스트)

**완료 기준**:
- 통합 테스트 모두 통과
- LG 프로젝터에서 모든 버튼 정상 동작 확인
- 리모컨 포커스 네비게이션 자연스럽고 직관적

---

### Phase 7: 코드 리뷰 및 문서화
**목표**: 코드 품질 검증, 개발 로그 업데이트

#### 태스크
- [ ] **Task 7.1**: 코드 리뷰 (NavigationBar, BrowserWindow)
  - 담당: code-reviewer
  - 작업 내용:
    - 코딩 컨벤션 준수 확인 (CLAUDE.md 기준)
    - 메모리 안전성 검증 (스마트 포인터, nullptr 체크)
    - Qt 시그널/슬롯 연결 정확성 검증
    - QSS 스타일시트 문법 검증
  - 산출물: 리뷰 코멘트 (인라인 코멘트 또는 별도 문서)
  - 완료 기준: 모든 리뷰 이슈 해결

- [ ] **Task 7.2**: 개발 로그 업데이트
  - 담당: doc-writer
  - 작업 내용:
    - `docs/dev-log.md`에 F-04 개발 과정 기록
    - `CHANGELOG.md`에 F-04 기능 추가 항목 작성
    - 주요 아키텍처 결정 사항 요약 (설계서 → 개발 로그)
  - 산출물: `docs/dev-log.md`, `CHANGELOG.md` (업데이트)
  - 완료 기준: 개발 타임라인, 이슈, 해결 방안이 명확히 기록

**예상 소요 시간**: 2시간

**의존성**: Phase 6 완료 (통합 테스트)

**완료 기준**:
- 코드 리뷰 통과 (리뷰어 승인)
- 개발 문서 최신 상태 유지

---

## 3. 태스크 의존성

```
Phase 1 (Core)
  │
  ├─ Task 1.1 (헤더 확장)
  │     ↓
  ├─ Task 1.2 (기본 UI 구현)
  │     ↓
  └─ Task 1.3 (버튼 핸들러)
        ↓
Phase 2 (WebView 연동)
  │
  ├─ Task 2.1 (setWebView)
  │     ↓
  └─ Task 2.2 (updateButtonStates)
        ↓
Phase 3 (BrowserWindow 통합)
  │
  ├─ Task 3.1 (헤더 수정)
  │     ↓
  ├─ Task 3.2 (setupUI)
  │     ↓
  ├─ Task 3.3 (setupConnections)
  │     ↓
  └─ Task 3.4 (keyPressEvent)
        ↓
Phase 4 (스타일링)
  │
  ├─ Task 4.1 (QSS 최적화) ─┐
  │                          │
  └─ Task 4.2 (접근성) ──────┤
                             ↓
Phase 5 (단위 테스트)
  │
  └─ Task 5.1 (NavigationBar 테스트)
        ↓
Phase 6 (통합 테스트)
  │
  ├─ Task 6.1 (통합 테스트) ─┐
  │                          │
  └─ Task 6.2 (디바이스 테스트)
        ↓
Phase 7 (리뷰 및 문서화)
  │
  ├─ Task 7.1 (코드 리뷰) ───┐
  │                          │
  └─ Task 7.2 (개발 로그) ───┘
```

**병렬 실행 가능 태스크**:
- Phase 4: Task 4.1 (QSS) ↔ Task 4.2 (접근성) (독립적, 동시 작업 가능)
- Phase 6: Task 6.1 (통합 테스트) ↔ Task 6.2 (디바이스 테스트) (병렬 실행 권장)
- Phase 7: Task 7.1 (코드 리뷰) ↔ Task 7.2 (개발 로그) (병렬 실행 권장)

---

## 4. 병렬 실행 판단 (PG-1 그룹)

### PG-1 그룹: F-03 (URL 입력 UI), F-04 (페이지 탐색 컨트롤), F-05 (로딩 인디케이터)

#### 병렬 가능 여부
**제한적 병렬 가능** (Phase 1-2는 병렬, Phase 3는 순차)

#### 의존성 분석
| 기능 | 의존하는 기능 | 의존 내용 |
|------|-------------|----------|
| **F-04 (NavigationBar)** | F-02 (WebView) | WebView 시그널 (urlChanged, loadStarted, loadFinished), canGoBack/Forward API |
| **F-03 (URLBar)** | F-02 (WebView) | WebView::load(QUrl) 메서드, urlChanged 시그널 |
| **F-05 (LoadingIndicator)** | F-02 (WebView) | loadStarted, loadFinished, loadProgress 시그널 |

#### 병렬 실행 전략

##### 병렬 Phase (Phase 1-2): 독립적 구현
- **F-03 Phase 1-2**: URLBar 클래스 독립 구현 (BrowserWindow 연동 전)
- **F-04 Phase 1-2**: NavigationBar 클래스 독립 구현 (BrowserWindow 연동 전)
- **F-05 Phase 1-2**: LoadingIndicator 클래스 독립 구현 (BrowserWindow 연동 전)

**근거**: 세 컴포넌트 모두 WebView의 공개 API만 사용하므로, WebView가 완성되어 있으면 각자 독립적으로 개발 가능합니다. BrowserWindow에 통합하기 전까지는 서로 간섭하지 않습니다.

##### 순차 Phase (Phase 3): BrowserWindow 통합
- **순서**: F-03 → F-04 → F-05 (위에서 아래로 레이아웃 순서)
- **이유**: BrowserWindow의 mainLayout_에 위젯을 추가하는 순서가 중요합니다. URLBar가 가장 위, LoadingIndicator가 가장 아래(또는 WebView와 겹침)로 배치되어야 합니다.

#### Agent Team 사용 권장
**Yes** (Phase 1-2에 한하여)

**이유**:
- **병렬 개발 효율**: F-03, F-04, F-05의 Phase 1-2를 동시에 구현하면 개발 시간을 1/3로 단축 가능
- **독립적 테스트**: 각 컴포넌트를 독립적으로 테스트한 후 BrowserWindow 통합 시 통합 테스트만 진행
- **코드 충돌 최소화**: 각 기능이 별도 파일(URLBar.cpp, NavigationBar.cpp, LoadingIndicator.cpp)에서 작업하므로 Git 충돌 없음

**Agent Team 구성 (Phase 1-2)**:
- **Agent 1**: F-03 URLBar 구현 (src/ui/URLBar.cpp)
- **Agent 2**: F-04 NavigationBar 구현 (src/ui/NavigationBar.cpp)
- **Agent 3**: F-05 LoadingIndicator 구현 (src/ui/LoadingIndicator.cpp)

**순차 작업 (Phase 3 이후)**:
- Phase 3 (BrowserWindow 통합)부터는 단일 Agent 또는 순차 실행 권장
- BrowserWindow.cpp를 세 기능이 공유하므로 병렬 작업 시 충돌 위험

---

## 5. 파일 목록

### 수정할 기존 파일
| 파일 경로 | 변경 내용 | Phase |
|----------|----------|-------|
| `src/ui/NavigationBar.h` | 멤버 변수, 메서드 시그니처 추가 (기존 스켈레톤 확장) | Phase 1 |
| `src/browser/BrowserWindow.h` | NavigationBar 멤버 추가, keyPressEvent 선언 | Phase 3 |
| `src/browser/BrowserWindow.cpp` | NavigationBar 생성, 레이아웃 추가, keyPressEvent 구현 | Phase 3 |

### 새로 생성할 파일
| 파일 경로 | 역할 | Phase |
|----------|------|-------|
| `src/ui/NavigationBar.cpp` | NavigationBar 클래스 구현 (약 300줄) | Phase 1-2 |
| `tests/unit/NavigationBarTest.cpp` | NavigationBar 단위 테스트 | Phase 5 |
| `tests/integration/BrowserWindowTest.cpp` | BrowserWindow 통합 테스트 (NavigationBar 포함) | Phase 6 |
| `docs/test-reports/F-04_device_test.md` | LG 프로젝터 실기 테스트 리포트 | Phase 6 |

### 업데이트할 문서
| 파일 경로 | 업데이트 내용 | Phase |
|----------|--------------|-------|
| `docs/dev-log.md` | F-04 개발 과정, 이슈, 해결 방안 기록 | Phase 7 |
| `CHANGELOG.md` | F-04 기능 추가 항목 작성 | Phase 7 |

---

## 6. 예상 소요 시간

| Phase | 예상 시간 | 누적 시간 |
|-------|----------|----------|
| Phase 1: Core 구현 | 4시간 | 4시간 |
| Phase 2: WebView 연동 | 3시간 | 7시간 |
| Phase 3: BrowserWindow 통합 | 3시간 | 10시간 |
| Phase 4: 스타일링 | 2시간 | 12시간 |
| Phase 5: 단위 테스트 | 3시간 | 15시간 |
| Phase 6: 통합 테스트 | 4시간 | 19시간 |
| Phase 7: 리뷰 및 문서화 | 2시간 | 21시간 |

**총 예상 시간**: 21시간 (약 2.5일, 1인 기준)

**병렬 실행 시 단축 가능**:
- PG-1 그룹 (F-03, F-04, F-05) Phase 1-2 병렬 실행 시: 7시간 → 3시간 (4시간 단축)
- Phase 4 병렬 실행 시: 2시간 → 1시간 (1시간 단축)
- **최종 예상 시간**: 16시간 (약 2일)

---

## 7. 리스크

| 리스크 | 영향도 | 발생 확률 | 대응 방안 |
|--------|--------|----------|-----------|
| **R1: WebView 시그널 미동작** | 높음 | 낮음 | - F-02 WebView 구현 검증 완료 필요<br>- urlChanged 시그널이 emit되는지 테스트<br>- 연결 실패 시 QTimer로 주기적 폴링(대안) |
| **R2: 리모컨 Back 키 코드 불일치** | 중간 | 중간 | - webOS 4.x에서 Back 키가 Qt::Key_Backspace가 아닐 수 있음<br>- 디바이스 테스트 시 실제 키 코드 로그 출력<br>- 복수 키 코드 처리 (Backspace, Escape 등) |
| **R3: QSS 스타일 webOS에서 미적용** | 낮음 | 낮음 | - webOS Qt 5.15는 QSS 지원 확인됨<br>- 최악의 경우 Qt 기본 스타일 사용 (기능은 정상 동작) |
| **R4: 포커스 순서 비정상** | 중간 | 낮음 | - setTabOrder() 호출 순서 재확인<br>- BrowserWindow 레벨에서 전체 포커스 체인 설계 필요<br>- F-03 URLBar와의 포커스 연동 테스트 |
| **R5: 메모리 누수 (시그널 연결)** | 낮음 | 낮음 | - setWebView() 호출 시 disconnect() 필수<br>- Valgrind 또는 Qt 메모리 프로파일러로 검증<br>- QObject parent-child 메커니즘으로 자동 정리 |
| **R6: Phase 3 통합 시 F-03과 충돌** | 중간 | 중간 | - BrowserWindow.cpp 레이아웃 순서 명확히 정의<br>- F-03, F-04 통합 시 코드 리뷰 필수<br>- 통합 테스트로 충돌 조기 발견 |

---

## 8. 완료 기준 (전체)

### 기능 완료 기준
- [ ] NavigationBar 위젯이 BrowserWindow에 표시
- [ ] 뒤로/앞으로/새로고침/홈 버튼 4개 정상 동작
- [ ] WebView URL 변경 시 버튼 상태 자동 업데이트 (활성/비활성)
- [ ] 리모컨 좌/우 방향키로 버튼 간 포커스 이동
- [ ] 리모컨 Back 키로 브라우저 뒤로 가기 (WebView 포커스 시)
- [ ] 첫 페이지에서 뒤로 버튼 비활성화
- [ ] 뒤로 간 후 앞으로 버튼 활성화

### 성능 완료 기준
- [ ] 버튼 클릭 후 0.3초 이내에 페이지 네비게이션 시작
- [ ] WebView URL 변경 시 0.1초 이내에 버튼 상태 업데이트
- [ ] NavigationBar 메모리 사용량 50KB 이하

### UX 완료 기준
- [ ] 버튼 크기 100px x 80px 이상
- [ ] 버튼 간격 20px 이상
- [ ] 포커스 테두리 2px, 흰색
- [ ] 비활성 버튼 opacity 0.5 (시각적 구분 명확)
- [ ] 3m 거리에서 버튼 아이콘 명확히 인식 가능

### 테스트 완료 기준
- [ ] 단위 테스트 5개 이상 통과 (NavigationBarTest)
- [ ] 통합 테스트 3개 이상 통과 (BrowserWindowTest)
- [ ] LG 프로젝터 HU715QW 실기 테스트 6개 통과
- [ ] 테스트 커버리지 80% 이상

### 문서 완료 기준
- [ ] 코드 리뷰 통과 (리뷰어 승인)
- [ ] `docs/dev-log.md` 업데이트 (F-04 개발 과정 기록)
- [ ] `CHANGELOG.md` 업데이트 (F-04 기능 추가)
- [ ] 테스트 리포트 작성 (`docs/test-reports/F-04_device_test.md`)

---

## 9. 추가 고려사항

### F-03 (URLBar)와의 연동
- **레이아웃 순서**: URLBar → NavigationBar → WebView
- **포커스 흐름**: URLBar ↔ NavigationBar ↔ WebView (위/아래 방향키)
- **WebView 공유**: URLBar와 NavigationBar 모두 동일한 WebView 인스턴스 참조
- **대응 방안**: Phase 3에서 BrowserWindow::setupConnections()에서 두 컴포넌트에 동일한 webView_ 포인터 전달

### F-11 (설정 화면) 향후 연동
- **홈페이지 URL 동적 변경**:
  ```cpp
  // 현재 (하드코딩)
  const QString DEFAULT_HOME_URL = "https://www.google.com";

  // F-11 구현 후 (SettingsService 주입)
  QString homeUrl = settingsService_->getHomePage();
  webView_->load(homeUrl);
  ```
- **대응 방안**: NavigationBar.cpp 주석으로 마이그레이션 계획 기록, F-11 구현 시 onHomeClicked() 수정

### F-15 (즐겨찾기 홈 화면) 향후 연동
- **첫 페이지에서 Back 키 동작 변경**:
  ```cpp
  // 현재 (이벤트 무시)
  if (!webView_->canGoBack()) {
      event->ignore();  // 앱 종료 방지
  }

  // F-15 구현 후 (즐겨찾기 홈 화면 표시)
  if (!webView_->canGoBack()) {
      showFavoriteHomeScreen();  // 홈 화면 전환
      event->accept();
  }
  ```
- **대응 방안**: BrowserWindow::keyPressEvent() 주석으로 향후 확장 계획 기록

---

## 10. Phase별 검증 체크리스트

### Phase 1 검증
- [ ] NavigationBar.h 컴파일 통과
- [ ] NavigationBar 인스턴스 생성 가능
- [ ] 버튼 4개 표시 확인 (유니코드 심볼)
- [ ] 리모컨 좌/우 방향키로 포커스 이동

### Phase 2 검증
- [ ] setWebView(nullptr) 호출 시 버튼 비활성화
- [ ] setWebView(validPtr) 호출 시 시그널 연결 확인
- [ ] updateButtonStates() 호출 시 버튼 상태 업데이트
- [ ] WebView URL 변경 시 updateButtonStates() 자동 호출

### Phase 3 검증
- [ ] BrowserWindow 실행 시 NavigationBar 표시
- [ ] 버튼 클릭 시 WebView 페이지 이동 확인
- [ ] 리모컨 Back 키로 브라우저 뒤로 가기
- [ ] 포커스 흐름 정상 (URLBar ↔ NavigationBar ↔ WebView)

### Phase 4 검증
- [ ] 버튼 크기 100x80px 이상 확인
- [ ] 포커스 테두리 2px 흰색 표시
- [ ] 비활성 버튼 opacity 0.5 확인
- [ ] 색상 대비 4.5:1 이상 (WCAG 도구로 측정)

### Phase 5 검증
- [ ] 단위 테스트 모두 통과 (5개 이상)
- [ ] 테스트 커버리지 80% 이상

### Phase 6 검증
- [ ] 통합 테스트 모두 통과 (3개 이상)
- [ ] LG 프로젝터 실기 테스트 모두 통과 (6개)
- [ ] 테스트 리포트 작성 완료

### Phase 7 검증
- [ ] 코드 리뷰 통과
- [ ] 개발 문서 업데이트 완료

---

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | F-04 구현 계획서 작성 (Native App 기반) |
| 2026-02-14 | PG-1 병렬 실행 전략 추가 | F-03, F-04, F-05 병렬 개발 계획 |
| 2026-02-14 | Phase별 검증 체크리스트 추가 | 각 Phase 완료 기준 명확화 |
| 2026-02-14 | Web App 내용 제거, Native App 내용으로 전면 재작성 | React/Enact → C++/Qt 전환 |
