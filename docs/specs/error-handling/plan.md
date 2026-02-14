# 에러 처리 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: `docs/specs/error-handling/requirements.md`
- 기술 설계서: `docs/specs/error-handling/design.md`

---

## 2. 구현 Phase

### Phase 1: ErrorPage 클래스 기본 구현
**담당**: cpp-dev
**예상 소요 시간**: 4~5시간

- [ ] Task 1-1: ErrorPage 헤더 파일 작성 (`src/ui/ErrorPage.h`)
  - ErrorType enum class 정의 (NetworkError, Timeout, CorsError, UnknownError)
  - ErrorInfo 구조체 정의 (type, errorMessage, url, timestamp)
  - ErrorPage 클래스 선언 (QWidget 상속)
  - 시그널 정의: `retryRequested()`, `homeRequested()`
  - 메서드 정의: `showError()`, `keyPressEvent()`, `showEvent()`
  - private 멤버: UI 컴포넌트 (QLabel, QPushButton), currentError_

- [ ] Task 1-2: ErrorPage 기본 UI 구현 (`src/ui/ErrorPage.cpp`)
  - 생성자: UI 컴포넌트 생성 및 초기화
  - `setupUI()`: QVBoxLayout 기반 레이아웃 구성
    - 아이콘 QLabel (80x80px)
    - 제목 QLabel (48px, Bold)
    - 메시지 QLabel (28px, WordWrap)
    - URL QLabel (22px, 회색)
    - QHBoxLayout: 재시도 버튼, 홈 버튼 (200x60px)
  - 중앙 정렬: `layout->setAlignment(Qt::AlignCenter)`

- [ ] Task 1-3: ErrorPage 스타일 구현
  - `applyStyles()`: QSS 스타일시트 적용
    - 배경: `rgba(0, 0, 0, 230)` (반투명 검은색)
    - 텍스트: 흰색
    - 버튼: 파란색 배경 (#1E90FF), 흰색 텍스트
    - 포커스: 4px 노란색 테두리 (#FFD700)
  - 포커스 정책: `retryButton->setFocusPolicy(Qt::StrongFocus)`
  - 탭 오더 설정: `setTabOrder(retryButton_, homeButton_)` (순환)

**예상 산출물**:
- `src/ui/ErrorPage.h`
- `src/ui/ErrorPage.cpp` (기본 UI 및 스타일)

**완료 기준**:
- ErrorPage 위젯을 단독으로 생성하고 표시 가능
- 스타일시트가 정상 적용됨 (배경, 텍스트, 버튼)
- 버튼 포커스가 리모컨 방향키로 전환 가능 (탭 오더)
- 컴파일 에러 없음

---

### Phase 2: ErrorPage 로직 구현
**담당**: cpp-dev
**예상 소요 시간**: 3~4시간

- [ ] Task 2-1: `showError()` 메서드 구현
  - ErrorType → 제목 문자열 맵핑 (switch-case)
  - UI 업데이트: `setText()` 호출
  - URL truncate (50자 제한): `truncateUrl()` 헬퍼 메서드
  - 재시도 버튼에 자동 포커스: `setFocus(Qt::OtherFocusReason)`
  - ErrorInfo 구조체 저장: `currentError_`

- [ ] Task 2-2: 에러 타입별 아이콘 처리
  - `getErrorIcon(ErrorType)`: 에러 타입에 따른 아이콘 경로 반환
  - 현재는 공통 아이콘 사용: `":/icons/error.png"`
  - 향후 확장 대비 분기 구조 준비

- [ ] Task 2-3: 키 이벤트 핸들러 구현
  - `keyPressEvent(QKeyEvent*)` 오버라이드
  - Back 키 무시: `event->ignore()`
  - 나머지 키는 Qt 기본 동작 (탭 오더 시스템)

- [ ] Task 2-4: 페이드인 애니메이션 구현
  - `showEvent(QShowEvent*)` 오버라이드
  - QPropertyAnimation 생성 (windowOpacity: 0 → 1, 300ms)
  - EasingCurve: `QEasingCurve::OutCubic`
  - `DeleteWhenStopped` 플래그로 자동 메모리 해제
  - `startFadeInAnimation()` 헬퍼 메서드

- [ ] Task 2-5: 시그널 연결
  - 재시도 버튼 `clicked` → `retryRequested()` 시그널
  - 홈 버튼 `clicked` → `homeRequested()` 시그널

**예상 산출물**:
- `src/ui/ErrorPage.cpp` (로직 완성)
- `resources/icons/error.png` (80x80px 에러 아이콘)

**완료 기준**:
- `showError()`를 호출하면 에러 타입별 제목이 표시됨
- URL이 50자 초과 시 truncate되어 표시됨 ("...")
- 페이드인 애니메이션이 부드럽게 동작 (300ms)
- 버튼 클릭 시 시그널이 정상 emit됨 (QSignalSpy 테스트)

---

### Phase 3: BrowserWindow 통합
**담당**: cpp-dev
**예상 소요 시간**: 3~4시간
**의존성**: Phase 2 완료 필요

- [ ] Task 3-1: BrowserWindow.h 수정
  - 헤더 추가: `#include <QStackedLayout>`, `#include "../ui/ErrorPage.h"`
  - private 멤버 추가:
    - `QWidget *contentWidget_` (WebView/ErrorPage 컨테이너)
    - `QStackedLayout *stackedLayout_` (전환 레이아웃)
    - `ErrorPage *errorPage_` (에러 화면)
  - private slots 추가:
    - `void onLoadError(const QString &errorString)`
    - `void onLoadTimeout()`
    - `void onRetryRequested()`
    - `void onHomeRequested()`

- [ ] Task 3-2: BrowserWindow.cpp 생성자 수정
  - 초기화 리스트에 추가:
    - `contentWidget_(new QWidget(centralWidget_))`
    - `stackedLayout_(new QStackedLayout(contentWidget_))`
    - `errorPage_(new ErrorPage(contentWidget_))`

- [ ] Task 3-3: `setupUI()` 메서드 수정
  - QStackedLayout 설정:
    - `stackedLayout_->addWidget(webView_)`
    - `stackedLayout_->addWidget(errorPage_)`
    - `stackedLayout_->setCurrentWidget(webView_)` (기본값)
  - contentWidget을 mainLayout에 추가:
    - `mainLayout_->addWidget(contentWidget_, 1)` (stretch=1)

- [ ] Task 3-4: `setupConnections()` 메서드 수정
  - WebView 에러 시그널 연결:
    - `connect(webView_, &WebView::loadError, this, &BrowserWindow::onLoadError)`
    - `connect(webView_, &WebView::loadTimeout, this, &BrowserWindow::onLoadTimeout)`
  - ErrorPage 시그널 연결:
    - `connect(errorPage_, &ErrorPage::retryRequested, this, &BrowserWindow::onRetryRequested)`
    - `connect(errorPage_, &ErrorPage::homeRequested, this, &BrowserWindow::onHomeRequested)`
  - WebView 로딩 성공 시 전환:
    - `connect(webView_, &WebView::loadFinished, [this](bool success) { ... })`

**예상 산출물**:
- `src/browser/BrowserWindow.h` (수정)
- `src/browser/BrowserWindow.cpp` (수정)

**완료 기준**:
- BrowserWindow가 컴파일 에러 없이 빌드됨
- WebView와 ErrorPage가 QStackedLayout으로 전환됨 (기본값: WebView)
- 시그널 연결이 정상 동작 (에러 발생 시 ErrorPage 표시)

---

### Phase 4: BrowserWindow 슬롯 구현
**담당**: cpp-dev
**예상 소요 시간**: 3~4시간
**의존성**: Phase 3 완료 필요

- [ ] Task 4-1: `onLoadError()` 슬롯 구현
  - ErrorInfo 구조체 생성
  - 에러 메시지 파싱으로 ErrorType 추론:
    - "timeout" → ErrorType::Timeout
    - "cors" / "cross-origin" → ErrorType::CorsError
    - 기타 → ErrorType::NetworkError
  - `errorPage_->showError(errorInfo)` 호출
  - `stackedLayout_->setCurrentWidget(errorPage_)` 호출
  - `qCritical()` 로그 기록

- [ ] Task 4-2: `onLoadTimeout()` 슬롯 구현
  - ErrorInfo 생성 (type: Timeout)
  - 메시지: "페이지 로딩 시간이 초과되었습니다 (30초)"
  - `errorPage_->showError(errorInfo)` 호출
  - `stackedLayout_->setCurrentWidget(errorPage_)` 호출
  - `qCritical()` 로그 기록

- [ ] Task 4-3: `onRetryRequested()` 슬롯 구현
  - QPropertyAnimation 생성 (windowOpacity: 1 → 0, 200ms)
  - EasingCurve: `QEasingCurve::InCubic`
  - 애니메이션 완료 후 (람다):
    - `stackedLayout_->setCurrentWidget(webView_)`
    - `webView_->reload()`
  - `DeleteWhenStopped` 플래그 설정

- [ ] Task 4-4: `onHomeRequested()` 슬롯 구현
  - QPropertyAnimation 생성 (onRetryRequested와 동일)
  - 애니메이션 완료 후:
    - `stackedLayout_->setCurrentWidget(webView_)`
    - `webView_->load(QUrl("https://www.google.com"))`
  - 향후 SettingsService 연동 대비 주석 추가

**예상 산출물**:
- `src/browser/BrowserWindow.cpp` (슬롯 구현 완성)

**완료 기준**:
- 잘못된 URL 로드 시 에러 화면이 자동 표시됨
- 재시도 버튼 클릭 시 페이지 재로드 및 페이드아웃 애니메이션
- 홈 버튼 클릭 시 홈페이지 이동 및 페이드아웃 애니메이션
- 로그에 에러 정보가 기록됨 (`qCritical()`)

---

### Phase 5: CMakeLists.txt 업데이트 및 빌드
**담당**: cpp-dev
**예상 소요 시간**: 1시간
**의존성**: Phase 4 완료 필요

- [ ] Task 5-1: CMakeLists.txt 수정
  - `src/ui/ErrorPage.cpp` 추가
  - `src/ui/ErrorPage.h` 추가 (헤더만 포함 시)
  - 빌드 타겟 확인

- [ ] Task 5-2: 전체 빌드 검증
  - `mkdir build && cd build`
  - `cmake ..`
  - `make`
  - 컴파일 에러 없음 확인
  - 링크 에러 없음 확인

- [ ] Task 5-3: 리소스 파일 준비
  - `resources/icons/error.png` (80x80px) 생성 또는 임시 이미지 사용
  - Qt 리소스 시스템 (qrc) 확인

**예상 산출물**:
- `src/CMakeLists.txt` (수정)
- `resources/icons/error.png`
- 빌드 성공한 실행 파일

**완료 기준**:
- 전체 프로젝트가 에러 없이 빌드됨
- 실행 시 ErrorPage 관련 심볼 에러 없음

---

### Phase 6: 통합 테스트 (수동)
**담당**: test-runner
**예상 소요 시간**: 2~3시간
**의존성**: Phase 5 완료 필요

- [ ] Task 6-1: 로컬 환경 테스트
  - 잘못된 URL 입력 (`https://invalid-domain-12345.com`) → 에러 화면 표시 확인
  - 재시도 버튼 클릭 → 페이지 재로드 확인
  - 홈 버튼 클릭 → 홈페이지 이동 확인
  - 키보드 Tab 키 → 버튼 포커스 전환 확인
  - Esc 키 (Back 키 대용) → 에러 화면 유지 확인

- [ ] Task 6-2: 타임아웃 테스트
  - 매우 느린 웹사이트 접속 또는 타임아웃 시뮬레이션
  - 30초 후 타임아웃 에러 화면 표시 확인
  - 재시도 시 타이머 재시작 확인

- [ ] Task 6-3: 애니메이션 테스트
  - 에러 화면 페이드인 (300ms) 부드러움 확인
  - 재시도 시 페이드아웃 (200ms) 부드러움 확인
  - windowOpacity 애니메이션 동작 여부 확인

- [ ] Task 6-4: 로그 확인
  - 터미널에서 `qCritical()` 로그 출력 확인
  - 에러 메시지, URL 정보 포함 확인

**예상 산출물**:
- 테스트 결과 문서 (간단한 체크리스트)

**완료 기준**:
- 모든 테스트 케이스가 PASS
- 치명적인 버그 없음
- 로그가 정상 기록됨

---

### Phase 7: 실제 디바이스 테스트 (LG 프로젝터 HU715QW)
**담당**: test-runner
**예상 소요 시간**: 2~3시간
**의존성**: Phase 6 완료 필요

- [ ] Task 7-1: webOS 빌드 및 배포
  - `ares-package build/`로 IPK 생성
  - `ares-install --device projector {ipk파일}` 설치
  - `ares-launch --device projector com.jsong.webosbrowser.native` 실행

- [ ] Task 7-2: 리모컨 포커스 테스트
  - 에러 화면 표시 시 재시도 버튼에 자동 포커스 확인
  - 좌/우 방향키로 재시도 ↔ 홈 버튼 전환 확인
  - 포커스 테두리 (4px 노란색) 가시성 확인
  - Enter 키로 버튼 클릭 확인
  - Back 키 무시 확인 (에러 화면 유지)

- [ ] Task 7-3: 대화면 가독성 테스트
  - 3m 거리에서 에러 제목 (48px) 가독 가능 확인
  - 에러 메시지 (28px) 가독 가능 확인
  - 버튼 텍스트 (24px) 가독 가능 확인
  - URL 텍스트 (22px) 가독 가능 확인

- [ ] Task 7-4: 성능 측정
  - 에러 발생 → 화면 표시 시간 (목표: 500ms 이내)
  - 재시도 클릭 → 로딩 시작 시간 (목표: 300ms 이내)
  - 애니메이션 프레임 드랍 여부 (60fps 목표)

- [ ] Task 7-5: 실제 네트워크 에러 테스트
  - Wi-Fi 비활성화 후 페이지 로드 → 에러 화면 확인
  - Wi-Fi 활성화 후 재시도 → 성공 확인
  - 타임아웃 시나리오 (느린 서버) 확인

**예상 산출물**:
- 실제 디바이스 테스트 리포트 (`docs/specs/error-handling/test-report.md`)
- 스크린샷 (에러 화면, 포커스 상태)

**완료 기준**:
- 리모컨 조작이 모든 시나리오에서 정상 동작
- 3m 거리 가독성 충족
- 성능 목표 달성 (에러 화면 500ms, 재시도 300ms)
- 애니메이션이 부드럽게 동작 (60fps 또는 최소 30fps)

---

### Phase 8: 코드 리뷰 및 문서화
**담당**: code-reviewer, doc-writer
**예상 소요 시간**: 2~3시간
**의존성**: Phase 7 완료 필요

- [ ] Task 8-1: 코드 리뷰 (code-reviewer)
  - ErrorPage 클래스 구조 검증
    - enum class 사용 적절성
    - ErrorInfo 구조체 설계
    - 메모리 관리 (Qt parent-child)
  - BrowserWindow 통합 검증
    - QStackedLayout 사용 적절성
    - 시그널/슬롯 연결 정확성
    - 애니메이션 메모리 관리 (DeleteWhenStopped)
  - 코딩 컨벤션 준수 확인
    - 주석 언어 (한국어)
    - 네이밍 (camelCase, PascalCase)
    - 들여쓰기 (4 스페이스)
  - 보안 이슈 확인
    - URL truncate (XSS 방지)
    - 에러 메시지 민감 정보 노출 여부

- [ ] Task 8-2: 리팩토링 제안 (code-reviewer)
  - 중복 코드 제거 (onRetryRequested, onHomeRequested)
  - 상수 추출 (애니메이션 시간, URL 길이 제한 등)
  - 헬퍼 메서드 추가 (필요시)

- [ ] Task 8-3: 문서화 (doc-writer)
  - `docs/dev-log.md` 업데이트
    - Phase별 작업 내용 기록
    - 이슈 및 해결 방법 기록
  - `CHANGELOG.md` 업데이트
    - `feat: F-10 에러 처리 기능 구현` 항목 추가
    - 세부 기능 목록 (ErrorPage 클래스, BrowserWindow 통합, 리모컨 포커스, 애니메이션)
  - 코드 주석 보완
    - ErrorPage.h에 Doxygen 스타일 주석 확인
    - BrowserWindow 슬롯 메서드에 주석 추가

- [ ] Task 8-4: 설계 문서 검증 (code-reviewer)
  - requirements.md와 실제 구현 일치 확인
  - design.md의 클래스 구조와 실제 코드 일치 확인
  - 미구현 기능 (Out of Scope) 확인

**예상 산출물**:
- 코드 리뷰 리포트 (간단한 체크리스트 또는 주석)
- `docs/dev-log.md` (업데이트)
- `CHANGELOG.md` (업데이트)

**완료 기준**:
- 코드 리뷰에서 치명적인 이슈 없음
- 모든 파일에 적절한 주석 존재
- 문서가 최신 상태로 업데이트됨

---

## 3. 태스크 의존성

```
Phase 1 (ErrorPage 기본 UI)
    │
    ▼
Phase 2 (ErrorPage 로직)
    │
    ▼
Phase 3 (BrowserWindow 통합)
    │
    ▼
Phase 4 (BrowserWindow 슬롯)
    │
    ▼
Phase 5 (빌드 검증)
    │
    ▼
Phase 6 (통합 테스트)
    │
    ▼
Phase 7 (디바이스 테스트)
    │
    ▼
Phase 8 (코드 리뷰 및 문서화)
```

**순차적 의존성**: 모든 Phase가 순차적으로 진행되어야 합니다. 병렬 실행 불가능.

---

## 4. 병렬 실행 판단

### 병렬 가능한 태스크
**없음**

이유:
- Phase 1~2는 ErrorPage 클래스의 기본 구현 및 로직이며, 서로 의존적입니다.
- Phase 3~4는 BrowserWindow 통합 및 슬롯 구현이며, ErrorPage 완성 후 진행 가능합니다.
- Phase 5는 전체 빌드 검증으로, 모든 코드 구현 완료 후 진행됩니다.
- Phase 6~7은 테스트 단계로, 빌드 완료 후 순차적으로 진행됩니다.
- Phase 8은 최종 검증으로, 모든 테스트 완료 후 진행됩니다.

### Agent Team 사용 권장 여부
**권장하지 않음 (단일 에이전트 사용)**

**근거**:
1. **단일 컴포넌트**: ErrorPage 클래스 1개 + BrowserWindow 수정만 필요
2. **순차적 의존성**: Phase 1~8이 모두 순차적으로 진행되어야 함
3. **낮은 복잡도**: 설계서에서 예상 소요 시간 1~1.5일로, 단일 개발자가 처리 가능
4. **적은 파일 수**:
   - 새 파일: 2개 (ErrorPage.h, ErrorPage.cpp)
   - 수정 파일: 2개 (BrowserWindow.h, BrowserWindow.cpp)
   - 총 4개 파일만 다룸
5. **충돌 위험 낮음**: ErrorPage는 독립적인 새 파일, BrowserWindow 수정도 명확한 영역
6. **테스트 의존성**: 통합 테스트는 전체 구현 완료 후에만 가능

### Agent Team이 유용한 경우 (참고용)
만약 다음 조건이라면 Agent Team을 고려할 수 있습니다:
- ErrorPage 외에 추가로 3개 이상의 독립 컴포넌트 구현 필요 (예: OfflinePage, LoadErrorDialog 등)
- 프론트엔드/백엔드가 분리된 구조 (Native App은 단일 레이어)
- 병렬로 작업 가능한 독립적인 모듈 (예: 에러 통계 수집 서비스 + UI)

### 권장 실행 방식
```
/fullstack-feature F-10 에러 처리
```

또는 수동 단계별 실행:
```
/dev-cycle F-10
```

---

## 5. 리스크 및 대응 방안

| 리스크 | 영향도 | 발생 확률 | 대응 방안 |
|--------|--------|----------|----------|
| **Qt windowOpacity 애니메이션 미작동** | 중 | 중 | QStackedLayout 내부 위젯은 windowOpacity가 동작하지 않을 수 있음. 대응: QGraphicsOpacityEffect 사용 또는 애니메이션 비활성화 |
| **리모컨 포커스 스타일 미표시** | 중 | 중 | webOS Qt 빌드에서 QSS focus 스타일이 플랫폼 의존적일 수 있음. 대응: QPalette 기반 스타일 또는 커스텀 포커스 인디케이터 추가 |
| **HTTP 상태 코드 감지 불가** | 낮음 | 높음 | Qt WebEngineView는 404, 500 등을 정상 로딩으로 처리함. 대응: 현 단계에서는 네트워크 에러/타임아웃만 처리 (범위 외로 명시) |
| **타임아웃 시간 부적절** | 낮음 | 중 | 30초 타임아웃이 너무 길거나 짧을 수 있음. 대응: SettingsService에서 설정 가능하도록 향후 확장 |
| **에러 메시지 파싱 오류** | 낮음 | 중 | Qt가 제공하는 errorString이 일관되지 않을 수 있음. 대응: 기본값 NetworkError로 처리, 향후 세분화 |
| **애니메이션 성능 저하** | 중 | 낮음 | 프로젝터 GPU가 약한 경우 애니메이션 끊김. 대응: EasingCurve 단순화 또는 애니메이션 비활성화 옵션 추가 |
| **메모리 누수 (애니메이션 객체)** | 높음 | 낮음 | QPropertyAnimation 객체가 자동 삭제 안 될 수 있음. 대응: DeleteWhenStopped 플래그 필수, parent 설정 금지 |
| **BrowserWindow 레이아웃 충돌** | 중 | 낮음 | 기존 레이아웃(BookmarkPanel 등)과 충돌 가능. 대응: QStackedLayout을 별도 contentWidget에 격리 |

---

## 6. 예상 총 소요 시간

| Phase | 소요 시간 |
|-------|----------|
| Phase 1: ErrorPage 기본 UI | 4~5시간 |
| Phase 2: ErrorPage 로직 | 3~4시간 |
| Phase 3: BrowserWindow 통합 | 3~4시간 |
| Phase 4: BrowserWindow 슬롯 | 3~4시간 |
| Phase 5: 빌드 검증 | 1시간 |
| Phase 6: 통합 테스트 | 2~3시간 |
| Phase 7: 디바이스 테스트 | 2~3시간 |
| Phase 8: 코드 리뷰 및 문서화 | 2~3시간 |
| **총합** | **20~27시간 (2.5~3.5일)** |

**주의**: 설계서에서는 1~1.5일로 예상했으나, 테스트 및 문서화 포함 시 2.5~3.5일 소요 예상.

---

## 7. 완료 체크리스트

### 기능 완성도
- [ ] ErrorPage 클래스가 정상 동작 (에러 화면 표시)
- [ ] 에러 타입별 제목 및 메시지 표시
- [ ] 재시도 버튼으로 페이지 재로드
- [ ] 홈 버튼으로 홈페이지 이동
- [ ] 페이드인/아웃 애니메이션 동작
- [ ] 리모컨 포커스 관리 (방향키, Enter, Back)
- [ ] 에러 로깅 (`qCritical()`)

### 통합 검증
- [ ] BrowserWindow에 QStackedLayout으로 통합
- [ ] WebView의 `loadError`, `loadTimeout` 시그널 연결
- [ ] 에러 발생 시 자동으로 ErrorPage 표시
- [ ] 로딩 성공 시 자동으로 WebView로 전환

### 테스트
- [ ] 로컬 환경에서 모든 시나리오 PASS
- [ ] 실제 디바이스에서 리모컨 조작 PASS
- [ ] 3m 거리 가독성 확인
- [ ] 성능 목표 달성 (500ms, 300ms)

### 문서화
- [ ] `docs/dev-log.md` 업데이트
- [ ] `CHANGELOG.md` 업데이트
- [ ] 코드 주석 충분
- [ ] 테스트 리포트 작성

---

## 8. 변경 이력

| 날짜 | 변경 내용 | 작성자 | 사유 |
|------|-----------|--------|------|
| 2026-02-13 | 초안 작성 (Web App 기반) | product-manager | F-10 구현 계획 수립 |
| 2026-02-14 | Native App 관점으로 전면 업데이트 | product-manager | React/Enact → C++/Qt 기술 스택 변경 |

---

## 다음 단계

구현 계획서가 승인되면 다음과 같이 진행합니다:

1. **cpp-dev 에이전트 호출** (Phase 1~5):
   ```
   /fullstack-feature F-10 에러 처리
   ```

2. **test-runner 에이전트 호출** (Phase 6~7):
   - 로컬 테스트 실행
   - 실제 디바이스 테스트 실행

3. **code-reviewer 에이전트 호출** (Phase 8):
   - 코드 리뷰 수행
   - 리팩토링 제안

4. **doc-writer 에이전트 호출** (Phase 8):
   - 개발 로그 업데이트
   - CHANGELOG 업데이트

5. **완료 확인**:
   - 모든 Phase 완료
   - 체크리스트 검증
   - 다음 기능 (F-11 설정 화면) 진행

---

## 부록: 에러 시나리오별 동작 흐름

### 시나리오 1: 네트워크 연결 실패
```
사용자: 잘못된 URL 입력 (https://invalid-domain.com)
   ↓
URLBar: load(url) → WebView
   ↓
WebView: loadStarted() (타임아웃 타이머 시작)
   ↓
Qt WebEngineView: DNS 실패
   ↓
WebView: loadFinished(false) → loadError("Host not found")
   ↓
BrowserWindow: onLoadError()
   ↓
ErrorPage: showError(NetworkError, "네트워크 연결을 확인해주세요", url)
   ↓
QStackedLayout: setCurrentWidget(errorPage_)
   ↓
ErrorPage: 페이드인 애니메이션 (300ms)
   ↓
사용자: 재시도 버튼 클릭
   ↓
ErrorPage: retryRequested() signal
   ↓
BrowserWindow: onRetryRequested()
   ↓
QPropertyAnimation: 페이드아웃 (200ms)
   ↓
WebView: reload()
   ↓
(성공 시) QStackedLayout: setCurrentWidget(webView_)
```

### 시나리오 2: 타임아웃
```
사용자: 느린 웹사이트 접속 (https://very-slow-server.com)
   ↓
WebView: loadStarted() (타임아웃 타이머 30초 시작)
   ↓
Qt WebEngineView: 로딩 중... (30초 경과)
   ↓
WebView: timeout 타이머 트리거 → loadTimeout() signal
   ↓
BrowserWindow: onLoadTimeout()
   ↓
ErrorPage: showError(Timeout, "페이지 로딩 시간이 초과되었습니다 (30초)", url)
   ↓
QStackedLayout: setCurrentWidget(errorPage_)
   ↓
사용자: 홈 버튼 클릭
   ↓
ErrorPage: homeRequested() signal
   ↓
BrowserWindow: onHomeRequested()
   ↓
QPropertyAnimation: 페이드아웃 (200ms)
   ↓
WebView: load("https://www.google.com")
   ↓
QStackedLayout: setCurrentWidget(webView_)
```

---

## 부록: QStackedLayout vs 오버레이 비교

| 항목 | QStackedLayout | QWidget 오버레이 |
|------|---------------|-----------------|
| **구현 복잡도** | 간단 (setCurrentWidget) | 복잡 (Z-order, 위치 계산) |
| **메모리 사용** | 낮음 (하나만 활성) | 중간 (둘 다 활성) |
| **포커스 관리** | 명확 (하나만 포커스) | 복잡 (오버레이 포커스 제어) |
| **애니메이션** | 가능 (위젯 전환) | 가능 (투명도 조절) |
| **사용 사례** | 전체 화면 전환 | 부분 오버레이 (BookmarkPanel) |

**결정**: ErrorPage는 전체 화면을 덮어야 하므로 QStackedLayout 선택.

---

## 부록: 코드 예시 (핵심 메서드)

### ErrorPage::showError() 구현 예시

```cpp
void ErrorPage::showError(ErrorType type, const QString &errorMessage, const QUrl &url) {
    // 에러 정보 저장
    currentError_.type = type;
    currentError_.errorMessage = errorMessage;
    currentError_.url = url;
    currentError_.timestamp = QDateTime::currentDateTime();

    // 에러 타입별 제목 설정
    QString title;
    switch (type) {
        case ErrorType::NetworkError:
            title = "네트워크 연결 실패";
            break;
        case ErrorType::Timeout:
            title = "로딩 시간 초과";
            break;
        case ErrorType::CorsError:
            title = "보안 정책 오류";
            break;
        case ErrorType::UnknownError:
        default:
            title = "페이지 로딩 오류";
            break;
    }

    // UI 업데이트
    iconLabel_->setPixmap(QPixmap(getErrorIcon(type)));
    titleLabel_->setText(title);
    messageLabel_->setText(errorMessage);
    urlLabel_->setText(truncateUrl(url));

    // 재시도 버튼에 포커스
    retryButton_->setFocus(Qt::OtherFocusReason);

    // 위젯 표시
    show();
}
```

### BrowserWindow::onLoadError() 구현 예시

```cpp
void BrowserWindow::onLoadError(const QString &errorString) {
    // 에러 타입 추론
    ErrorType type = ErrorType::NetworkError;
    if (errorString.contains("timeout", Qt::CaseInsensitive)) {
        type = ErrorType::Timeout;
    } else if (errorString.contains("cors", Qt::CaseInsensitive)) {
        type = ErrorType::CorsError;
    }

    // ErrorPage 표시
    errorPage_->showError(type, errorString, webView_->url());
    stackedLayout_->setCurrentWidget(errorPage_);

    // 로그 기록
    qCritical() << "Page load error:"
                << "type=" << static_cast<int>(type)
                << "message=" << errorString
                << "url=" << webView_->url().toString();
}
```

---

## 부록: 파일 체크리스트

### 새로 생성할 파일
- [ ] `src/ui/ErrorPage.h`
- [ ] `src/ui/ErrorPage.cpp`
- [ ] `resources/icons/error.png`

### 수정할 파일
- [ ] `src/browser/BrowserWindow.h`
- [ ] `src/browser/BrowserWindow.cpp`
- [ ] `src/CMakeLists.txt`

### 문서 파일
- [ ] `docs/dev-log.md` (업데이트)
- [ ] `CHANGELOG.md` (업데이트)
- [ ] `docs/specs/error-handling/test-report.md` (새로 작성)

---

이상으로 F-10 에러 처리 기능의 구현 계획서를 완료합니다.
