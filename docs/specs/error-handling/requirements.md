# 에러 처리 — 요구사항 분석서 (Native App)

## 1. 개요

### 기능명
F-10: 에러 처리 (Error Handling)

### 목적
webOS Native App 브라우저에서 웹 페이지 로딩 중 발생하는 다양한 에러(네트워크 연결 실패, HTTP 에러, 렌더링 오류, 타임아웃 등)를 감지하고, Qt 기반의 사용자 친화적인 에러 화면을 표시하여 사용자가 문제를 이해하고 쉽게 복구할 수 있도록 합니다.

### 대상 사용자
- LG 프로젝터 HU715QW에서 웹 브라우저를 사용하는 모든 사용자
- 네트워크 불안정 환경에서 웹 서핑하는 사용자
- 리모컨만으로 브라우저를 조작하는 사용자

### 요청 배경
- **네트워크 불안정**: 가정 환경에서 Wi-Fi 연결 불안정 또는 ISP 장애 발생 가능
- **HTTP 에러**: 사용자가 잘못된 URL 입력 또는 서버 오류 페이지 접속 (404, 500 등)
- **SSL/TLS 에러**: 인증서 오류, HTTPS 연결 실패
- **타임아웃**: 대용량 페이지 또는 느린 서버로 인한 30초 타임아웃
- **사용자 경험**: Qt WebEngineView 기본 에러 페이지는 리모컨 조작 불가 또는 가독성 낮음
- **F-02 (웹뷰 통합) 완료**: WebView 클래스의 `loadError` 시그널이 구현되어 있어 에러 감지 가능

현재 WebView 컴포넌트에 기본적인 에러 시그널(`loadError`, `loadTimeout`)이 구현되어 있으나, 에러 타입별 세분화된 처리와 전용 ErrorPage Qt 위젯이 필요합니다.

---

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: 에러 감지 (Qt 시그널 기반)
- **설명**: Qt WebEngineView의 `loadFinished(false)` 시그널 및 타임아웃 타이머로 에러를 감지합니다.
- **유저 스토리**: As a 사용자, I want 브라우저가 페이지 로딩 실패를 자동으로 감지하고, so that 무한 대기 상태에 빠지지 않습니다.
- **우선순위**: Must
- **세부 사항**:
  - WebView 클래스의 `loadError(const QString &errorString)` 시그널 구독
  - WebView 클래스의 `loadTimeout()` 시그널 구독 (30초 타임아웃, WebView 내부 구현)
  - 에러 발생 시 WebView 상태를 `LoadingState::Error`로 전환 (이미 WebView에 구현됨)
  - 에러 정보 구조체 생성:
    ```cpp
    struct ErrorInfo {
        int errorCode;           // HTTP 상태 코드 또는 시스템 에러 코드
        QString errorMessage;    // 사용자 표시용 메시지
        QUrl url;                // 실패한 URL
        QDateTime timestamp;     // 에러 발생 시각
    };
    ```

### FR-2: 에러 타입 분류
- **설명**: 에러를 타입별로 분류하여 각기 다른 메시지와 복구 옵션을 제공합니다.
- **유저 스토리**: As a 사용자, I want 에러 유형에 따라 적절한 안내 메시지를 보고, so that 문제 해결 방법을 이해할 수 있습니다.
- **우선순위**: Must
- **에러 타입**:
  | 에러 타입 | 에러 코드 | 원인 | 메시지 예시 |
  |----------|----------|------|------------|
  | 네트워크 에러 | `-1` | DNS 실패, 연결 거부, 네트워크 단절 | "네트워크 연결을 확인해주세요" |
  | 타임아웃 에러 | `-2` | 30초 초과 로딩 | "페이지 로딩 시간이 초과되었습니다 (30초)" |
  | 404 에러 | `404` | 페이지 없음 | "페이지를 찾을 수 없습니다" |
  | 5xx 에러 | `500`, `503` | 서버 오류 | "서버에 일시적인 문제가 발생했습니다" |
  | CORS 에러 | `-3` | Same-Origin Policy 위반 | "이 페이지는 보안 정책으로 인해 표시할 수 없습니다" |
  | 알 수 없는 에러 | `-99` | 기타 | "페이지 로딩 중 오류가 발생했습니다" |

### FR-3: 에러 화면 UI (Qt Widget)
- **설명**: 에러 발생 시 WebView 위에 오버레이 형태로 에러 화면을 표시합니다.
- **유저 스토리**: As a 사용자, I want 에러 발생 시 명확한 안내 화면을 보고, so that 현재 상황을 이해할 수 있습니다.
- **우선순위**: Must
- **구현 방식**:
  - ErrorPage 클래스: `QWidget` 상속
  - 위치: `src/ui/ErrorPage.h`, `src/ui/ErrorPage.cpp`
  - 레이아웃: `QVBoxLayout` 기반
  - WebView 위에 스택 레이아웃(`QStackedLayout`)으로 오버레이
- **UI 요소**:
  - 에러 아이콘: `QLabel` (QPixmap 사용, 80x80px)
  - 에러 제목: `QLabel` (폰트 크기 48px, 굵은 글꼴)
  - 에러 메시지: `QLabel` (폰트 크기 28px, 일반 글꼴, 줄바꿈 지원)
  - 실패한 URL: `QLabel` (폰트 크기 22px, 회색, 최대 50자)
  - 재시도 버튼: `QPushButton` (최소 크기 200x60px, 폰트 24px)
  - 홈으로 이동 버튼: `QPushButton` (선택 사항)
  - 배경: `QWidget` 배경색 `rgba(0, 0, 0, 0.9)` (반투명 검은색)

### FR-4: 재시도 기능
- **설명**: 재시도 버튼 클릭 시 같은 URL로 다시 로딩을 시도합니다.
- **유저 스토리**: As a 사용자, I want 에러 화면에서 재시도 버튼을 누르고, so that 네트워크 복구 후 페이지를 다시 로드할 수 있습니다.
- **우선순위**: Must
- **세부 사항**:
  - 재시도 버튼 (`QPushButton`) `clicked` 시그널 → WebView의 `reload()` 메서드 호출
  - WebView 상태를 `LoadingState::Error` → `LoadingState::Loading`으로 전환
  - 재시도 횟수 제한 없음 (사용자가 원하는 만큼 재시도 가능)
  - 재시도 시 에러 화면 즉시 숨김 (`ErrorPage::hide()`), LoadingBar 표시 (F-05 연계)
  - Qt 시그널/슬롯 연결:
    ```cpp
    connect(retryButton, &QPushButton::clicked, webView, &WebView::reload);
    connect(retryButton, &QPushButton::clicked, errorPage, &ErrorPage::hide);
    ```

### FR-5: 홈으로 이동 기능
- **설명**: 에러 화면에서 홈 버튼 클릭 시 홈페이지로 이동합니다.
- **유저 스토리**: As a 사용자, I want 에러 발생 시 홈페이지로 돌아가고, so that 다른 페이지를 탐색할 수 있습니다.
- **우선순위**: Should
- **세부 사항**:
  - 홈 버튼 (`QPushButton`) `clicked` 시그널 → WebView의 `load(homeUrl)` 메서드 호출
  - 홈 URL 기본값: `https://www.google.com` (SettingsService에서 가져올 예정, F-11 연계)
  - WebView 상태를 `LoadingState::Error` → `LoadingState::Loading`으로 전환
  - NavigationBar의 홈 버튼(F-04)과 동일한 동작
  - Qt 시그널/슬롯 연결:
    ```cpp
    connect(homeButton, &QPushButton::clicked, this, [this, webView]() {
        webView->load(QUrl("https://www.google.com")); // 또는 SettingsService::homeUrl()
        errorPage->hide();
    });
    ```

### FR-6: 에러 로깅 (Qt 로깅 시스템)
- **설명**: 에러 발생 시 로그를 기록하여 디버깅 및 모니터링에 활용합니다.
- **유저 스토리**: As a 개발자, I want 에러 발생 시 상세 로그를 확인하고, so that 문제 원인을 파악할 수 있습니다.
- **우선순위**: Must
- **로그 정보**:
  - 타임스탬프: `QDateTime::currentDateTime().toString(Qt::ISODate)`
  - 에러 코드: `errorCode` (int)
  - 에러 메시지: `errorMessage` (QString)
  - 실패한 URL: `url.toString()` (QString)
  - 로딩 시작 시간 및 소요 시간 (WebView 내부 타이머)
  - 사용자 환경 정보: webOS 버전, Qt 버전 (선택 사항)
- **로깅 구현**:
  - Logger 클래스 (기존 `src/utils/Logger.h`, `Logger.cpp`) 사용
  - `qCritical()` 매크로 사용:
    ```cpp
    qCritical() << "Page load error:"
                << "code=" << errorCode
                << "message=" << errorMessage
                << "url=" << url.toString();
    ```
  - webOS 로그 시스템에 자동 통합 (pmloglib)

### FR-7: 에러 이벤트 시그널
- **설명**: 에러 발생 시 부모 위젯(BrowserWindow)에 에러 정보를 전달합니다.
- **유저 스토리**: As a BrowserWindow, I want WebView에서 에러가 발생하면 알림을 받고, so that ErrorPage를 표시하고 탭 상태를 업데이트할 수 있습니다.
- **우선순위**: Must
- **시그널 인터페이스** (WebView 클래스):
  ```cpp
  signals:
      void loadError(const QString &errorString);
      void loadTimeout();
  ```
- **슬롯 구현** (BrowserWindow 클래스):
  ```cpp
  private slots:
      void onLoadError(const QString &errorString) {
          // ErrorInfo 생성 및 ErrorPage 표시
          errorPage->showError(errorCode, errorString, webView->url());
          qCritical() << "Load error:" << errorString;
      }

      void onLoadTimeout() {
          errorPage->showError(-2, "페이지 로딩 시간이 초과되었습니다 (30초)", webView->url());
          qCritical() << "Load timeout:" << webView->url();
      }
  ```
- **사용 사례**:
  - BrowserWindow에서 ErrorPage 표시 제어
  - 탭 상태 업데이트 (`isError: true`, F-06 연계)
  - 에러 통계 수집 (향후 분석용)

### FR-8: 리모컨 포커스 관리 (Qt 포커스 시스템)
- **설명**: 에러 화면의 버튼들은 Qt 포커스 시스템으로 리모컨 조작이 가능해야 합니다.
- **유저 스토리**: As a 사용자, I want 리모컨 방향키로 에러 화면의 버튼을 선택하고, so that 재시도 또는 홈으로 이동할 수 있습니다.
- **우선순위**: Must
- **포커스 흐름**:
  - 에러 화면 표시 시 재시도 버튼에 자동 포커스:
    ```cpp
    retryButton->setFocus(Qt::OtherFocusReason);
    ```
  - 좌/우 방향키: 재시도 ↔ 홈 버튼 전환 (Qt 탭 오더 설정)
    ```cpp
    setTabOrder(retryButton, homeButton);
    setTabOrder(homeButton, retryButton);
    ```
  - 선택 버튼 (Enter/Return): `clicked` 시그널 emit
  - Back 키: 에러 화면 유지 (`keyPressEvent` 오버라이드로 Back 키 무시)
- **포커스 스타일**:
  - 포커스된 버튼: 4px 노란색 테두리 (QPalette 또는 QSS 스타일시트)
    ```cpp
    retryButton->setStyleSheet(
        "QPushButton:focus { border: 4px solid #FFD700; }"
    );
    ```

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **에러 감지 속도**: 에러 발생 후 500ms 이내 에러 화면 표시
- **타임아웃 시간**: 30초 (WebView 내부 타이머 구현 유지)
- **재시도 응답 시간**: 버튼 클릭 후 300ms 이내 로딩 시작

### UX (리모컨 최적화)
- **대화면 가독성**:
  - 에러 제목: `QLabel` 폰트 크기 48px, 굵은 글꼴 (`QFont::Bold`)
  - 에러 메시지: 폰트 크기 28px, 일반 글꼴
  - URL: 폰트 크기 22px, 회색 텍스트 (`QPalette::Mid`)
  - 버튼: 최소 크기 200x60px (`setMinimumSize(200, 60)`), 폰트 크기 24px
- **고대비 디자인**:
  - 배경: 반투명 검은색 (QColor(0, 0, 0, 230), alpha=230/255≈0.9)
  - 텍스트: 흰색 (QColor(255, 255, 255))
  - 버튼: 파란색 배경 (QColor(30, 144, 255)), 흰색 텍스트
  - 포커스: 노란색 테두리 (4px solid #FFD700, QSS 또는 QPalette)
- **애니메이션** (Qt Animation Framework):
  - 에러 화면 페이드인: `QPropertyAnimation` (opacity: 0 → 1, 300ms)
  - 재시도 시 페이드아웃: `QPropertyAnimation` (opacity: 1 → 0, 200ms)
  - 구현 예시:
    ```cpp
    QPropertyAnimation *fadeIn = new QPropertyAnimation(errorPage, "windowOpacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    ```

### 접근성
- **리모컨 전용 조작**: 모든 버튼은 `QPushButton::setFocusPolicy(Qt::StrongFocus)` 설정
- **명확한 피드백**: 버튼 클릭 시 시각적 피드백 (QSS hover/pressed 스타일 또는 `QPushButton::setDown(true)`)
- **에러 메시지 국제화**: 향후 Qt Linguist 다국어 지원 대비 (현재는 한국어만, `tr()` 함수 준비)

### 보안
- **에러 정보 노출 최소화**: 에러 메시지에 서버 내부 정보(스택 트레이스, IP 등) 노출 금지
- **URL 검증**: 에러 화면에 표시할 URL은 최대 50자로 제한 (XSS 방지)
  ```cpp
  QString displayUrl = url.toString().left(50);
  if (url.toString().length() > 50) {
      displayUrl += "...";
  }
  ```

### 확장성
- **커스텀 에러 핸들러**: ErrorPage 클래스에 커스텀 에러 타입 및 메시지 전달 가능
  ```cpp
  void showError(int errorCode, const QString &errorMessage, const QUrl &url);
  ```
- **에러 리포팅**: 향후 원격 로그 수집 시스템 연동 가능 (현재는 로컬 로그만, webOS pmloglib 사용)

---

## 4. 제약조건

### Qt/webOS 플랫폼 제약
- **Qt WebEngineView 에러 이벤트**: `loadFinished(false)` 시그널만 제공, 세부 에러 코드는 제한적
- **HTTP 상태 코드 감지 제약**: Qt WebEngineView는 HTTP 상태 코드(404, 500 등) 직접 제공 안 함, 페이지 내용 파싱 필요
- **SSL/TLS 에러**: `QWebEngineCertificateError` 시그널로 인증서 오류 감지 가능
- **타임아웃 구현**: Qt WebEngineView 기본 타임아웃 없음, WebView 클래스 내부에 `QTimer`로 구현됨 (30초)

### 기존 시스템 연동
- **WebView 컴포넌트 연계**: 현재 WebView 클래스에 `loadError`, `loadTimeout` 시그널 구현됨 (F-02 완료)
- **BrowserWindow 레이아웃**: ErrorPage를 `QStackedLayout`으로 WebView 위에 오버레이
  ```cpp
  QStackedLayout *stackLayout = new QStackedLayout();
  stackLayout->addWidget(webView);
  stackLayout->addWidget(errorPage);
  stackLayout->setCurrentWidget(webView); // 기본값: WebView 표시
  ```
- **탭 상태 관리**: 탭의 `isError` 상태를 TabManager에 반영하여 탭 전환 시 에러 상태 유지 (F-06 연계)
- **NavigationBar 연동**: 에러 발생 시 NavigationBar의 뒤로/앞으로 버튼 비활성화 또는 에러 상태 표시 (F-04 연계)

### 디자인 제약
- **Qt Widgets 스타일**: QPalette, QSS(Qt Style Sheets)로 스타일 정의
- **리모컨 최적화**: 3m 거리 가독성 고려 (폰트 크기, 버튼 크기)
- **다크 모드 지원**: 향후 F-11 설정 화면에서 테마 전환 시 에러 화면도 다크/라이트 모드 적용 (QPalette 전환)

---

## 5. 용어 정의

| 용어 | 정의 |
|------|------|
| **에러 감지** | Qt WebEngineView의 `loadFinished(false)` 시그널 또는 타임아웃 타이머로 로딩 실패를 감지하는 프로세스 |
| **에러 타입** | 네트워크 에러, 타임아웃, HTTP 에러(404, 500 등), SSL/TLS 에러 등 에러의 원인 분류 |
| **에러 화면** | 에러 발생 시 WebView 위에 오버레이로 표시되는 UI (ErrorPage Qt 위젯) |
| **재시도** | 같은 URL로 페이지 로딩을 다시 시도하는 동작 (`WebView::reload()`) |
| **타임아웃** | 페이지 로딩 시작 후 30초 이내 완료되지 않으면 에러로 처리 (WebView 내부 QTimer) |
| **에러 로깅** | 에러 발생 시 로그 메시지를 기록하는 동작 (qCritical(), Logger 클래스, webOS pmloglib) |
| **에러 시그널** | WebView에서 BrowserWindow로 에러 정보를 전달하는 Qt 시그널 (`loadError`, `loadTimeout`) |
| **Qt 포커스 시스템** | Qt의 키보드 포커스 관리 메커니즘 (setFocus, setTabOrder, FocusPolicy) |
| **오버레이** | 기존 WebView 위에 반투명 배경으로 덮어 표시하는 레이어 (QStackedLayout 사용) |
| **QStackedLayout** | Qt 레이아웃, 여러 위젯을 겹쳐 놓고 하나만 표시 (스택처럼 전환) |
| **QPropertyAnimation** | Qt Animation Framework, 위젯 속성(opacity 등) 애니메이션 |
| **QSS (Qt Style Sheets)** | Qt의 CSS와 유사한 스타일 시트 언어 (위젯 스타일 정의) |

---

## 6. 범위 외 (Out of Scope)

### 이번 기능에서 하지 않는 것
- **HTTP 상태 코드 정밀 감지**: Qt WebEngineView 제약으로 인해 404, 500 등 HTTP 상태 코드를 직접 감지 어려움 (일부 페이지는 서버 에러 페이지로 정상 로딩)
- **자동 재시도**: 에러 발생 시 자동으로 재시도하지 않음 (사용자가 수동으로 재시도 버튼 클릭)
- **에러 통계**: 에러 발생 빈도, 원인 통계를 수집하지 않음 (향후 확장 가능)
- **오프라인 모드**: 네트워크 단절 시 오프라인 페이지 제공하지 않음 (단순 에러 화면만 표시)
- **에러 복구 제안**: "Wi-Fi를 켜세요", "라우터를 재시작하세요" 등의 구체적인 복구 제안 제공하지 않음
- **에러 히스토리**: 이전에 발생한 에러 기록을 별도로 저장하지 않음 (로그만 기록)
- **원격 로그 전송**: 현재는 로컬 로그만, 향후 원격 서버로 에러 리포팅 가능

---

## 7. 수용 기준 (Acceptance Criteria)

### AC-1: 네트워크 에러 감지 및 화면 표시
- **Given**: 사용자가 잘못된 URL을 입력하거나 네트워크가 단절된 상태
- **When**: WebView가 페이지 로딩 시도 시
- **Then**:
  - WebView의 `loadError` 시그널이 emit됨
  - BrowserWindow의 슬롯이 호출되어 ErrorInfo 생성
  - WebView 상태가 `LoadingState::Error`로 전환됨
  - ErrorPage가 `QStackedLayout::setCurrentWidget(errorPage)`로 표시됨
  - 에러 화면에 에러 아이콘, 제목, 메시지, URL, 재시도 버튼이 표시됨
  - `qCritical()` 로그가 기록됨

### AC-2: 타임아웃 에러 처리
- **Given**: 사용자가 매우 느린 웹사이트에 접속
- **When**: 페이지 로딩 시작 후 30초 경과
- **Then**:
  - WebView 내부 QTimer가 트리거되어 `loadTimeout` 시그널 emit
  - BrowserWindow 슬롯에서 ErrorPage 표시
  - 에러 화면에 "페이지 로딩 시간이 초과되었습니다 (30초)" 메시지 표시
  - 재시도 버튼 클릭 시 `webView->reload()` 호출되어 다시 30초 타이머 시작

### AC-3: 재시도 버튼 동작
- **Given**: 에러 화면이 표시된 상태
- **When**: 사용자가 리모컨으로 재시도 버튼을 선택하고 확인 버튼(Enter)을 누름
- **Then**:
  - 재시도 버튼의 `clicked` 시그널 emit
  - QPropertyAnimation으로 에러 화면 페이드아웃 (200ms)
  - `webView->reload()` 호출
  - WebView 상태가 `LoadingState::Loading`으로 전환됨
  - LoadingBar가 표시됨 (F-05 연계)
  - `QStackedLayout::setCurrentWidget(webView)` 호출
  - 성공 시 에러 화면 유지 숨김, 실패 시 다시 ErrorPage 표시

### AC-4: 홈으로 이동 버튼 동작
- **Given**: 에러 화면이 표시된 상태
- **When**: 사용자가 홈 버튼을 선택하고 확인 버튼을 누름
- **Then**:
  - 홈 버튼의 `clicked` 시그널 emit
  - QPropertyAnimation으로 에러 화면 페이드아웃 (200ms)
  - `webView->load(QUrl("https://www.google.com"))` 호출
  - WebView 상태가 `LoadingState::Loading`으로 전환됨
  - 홈페이지 로딩 시작
  - 성공 시 에러 화면 숨김

### AC-5: 리모컨 포커스 관리 (Qt 포커스)
- **Given**: 에러 화면이 표시된 상태
- **When**: 에러 화면이 처음 표시됨
- **Then**:
  - `retryButton->setFocus(Qt::OtherFocusReason)` 호출
  - 재시도 버튼에 노란색 테두리 표시 (QSS `QPushButton:focus` 스타일)
- **When**: 사용자가 좌/우 방향키를 누름
- **Then**:
  - Qt 탭 오더에 따라 재시도 버튼 ↔ 홈 버튼 간 포커스 전환
- **When**: 사용자가 백 키를 누름
- **Then**:
  - `ErrorPage::keyPressEvent()` 오버라이드로 Back 키 무시 (`event->ignore()`)
  - 에러 화면이 유지됨 (포커스 이탈 불가)

### AC-6: 에러 로깅 (Qt 로깅)
- **Given**: 에러가 발생한 상태
- **When**: 에러 화면이 표시됨
- **Then**:
  - `qCritical() << "Page load error:" << errorMessage << url;` 로그 기록
  - 로그에 타임스탬프 (자동), 에러 메시지, URL 포함
  - webOS pmloglib로 자동 통합됨

### AC-7: 탭 상태 반영 (F-06 연계)
- **Given**: WebView에서 에러가 발생함
- **When**: `loadError` 시그널이 emit됨
- **Then**:
  - BrowserWindow가 TabManager에 `setTabError(tabId, true)` 호출 (향후 F-06 구현)
  - 탭을 전환하고 다시 돌아와도 ErrorPage 상태 유지
  - 탭 UI에 에러 표시 (예: 빨간색 아이콘)

### AC-8: 대화면 가독성 (Qt 폰트)
- **Given**: 에러 화면이 표시된 상태
- **When**: 사용자가 3m 거리에서 프로젝터 화면을 봄
- **Then**:
  - 에러 제목 `QLabel` 폰트 크기 48px, `QFont::Bold`로 명확하게 읽힘
  - 에러 메시지 폰트 크기 28px로 명확하게 읽힘
  - 버튼 최소 크기 200x60px, 폰트 24px로 명확하게 구분됨
  - 포커스 테두리 4px 노란색 (QSS 스타일)으로 명확하게 보임

### AC-9: 에러 타입별 메시지
- **Given**: 각기 다른 타입의 에러가 발생함
- **When**: ErrorPage의 `showError(errorCode, errorMessage, url)` 호출
- **Then**:
  - 네트워크 에러 (-1): "네트워크 연결을 확인해주세요"
  - 타임아웃 (-2): "페이지 로딩 시간이 초과되었습니다 (30초)"
  - 알 수 없는 에러 (-99): "페이지 로딩 중 오류가 발생했습니다"
  - 에러 메시지 QLabel에 표시됨

### AC-10: 에러 화면 애니메이션 (QPropertyAnimation)
- **Given**: 에러가 발생함
- **When**: ErrorPage가 표시됨
- **Then**:
  - `QPropertyAnimation` (windowOpacity: 0 → 1, 300ms) 실행
  - 페이드인 효과로 에러 화면 표시
- **When**: 재시도 또는 홈 버튼을 누름
- **Then**:
  - `QPropertyAnimation` (windowOpacity: 1 → 0, 200ms) 실행
  - 페이드아웃 효과로 에러 화면 숨김

### AC-11: BrowserWindow 통합
- **Given**: BrowserWindow에 WebView와 ErrorPage가 QStackedLayout으로 구성됨
- **When**: 페이지 로딩 성공 시
- **Then**:
  - `stackLayout->setCurrentWidget(webView)` 호출되어 WebView 표시
- **When**: 에러 발생 시
- **Then**:
  - `stackLayout->setCurrentWidget(errorPage)` 호출되어 ErrorPage 표시

### AC-12: 실제 디바이스 테스트
- **Given**: LG 프로젝터 HU715QW에 앱 설치
- **When**: 잘못된 URL 입력 시
- **Then**:
  - ErrorPage가 리모컨 조작 가능하게 표시됨
  - 재시도 버튼 클릭 시 정상 동작
  - 3m 거리에서 에러 메시지 가독 가능

---

## 8. 우선순위 및 일정

- **기능 ID**: F-10
- **우선순위**: Must (브라우저 안정성 필수 기능)
- **의존성**:
  - F-02 (웹뷰 통합) ✅ 완료 - WebView 클래스의 `loadError`, `loadTimeout` 시그널 사용
  - F-04 (페이지 탐색 컨트롤) ✅ 완료 - NavigationBar 연동 (선택 사항)
  - F-05 (로딩 인디케이터) ✅ 완료 - 재시도 시 LoadingBar 표시 연계
- **충돌 영역**: ErrorPage 컴포넌트, WebView 컴포넌트 (시그널 구독)
- **마일스톤**: M2
- **예상 복잡도**: Low
  - ErrorPage Qt 위젯 구현: 단순 UI (QLabel, QPushButton)
  - BrowserWindow 통합: QStackedLayout 설정
  - 시그널/슬롯 연결: 표준 Qt 패턴
- **예상 소요 시간**: 1~1.5일
  - ErrorPage 클래스 구현: 4~5시간
  - BrowserWindow 통합: 2~3시간
  - 테스트 및 디버깅: 2~3시간
  - 문서화: 1~2시간

---

## 9. 변경 이력

| 날짜 | 변경 내용 | 작성자 | 사유 |
|------|-----------|--------|------|
| 2026-02-13 | 초안 작성 (Web App 기반) | product-manager | F-10 요구사항 분석 |
| 2026-02-14 | Native App 관점으로 전면 업데이트 | product-manager | React/Enact → C++/Qt 기술 스택 변경 |

---

## 다음 단계

이 요구사항 분석서를 기반으로 architect 에이전트가 다음 문서를 작성합니다:
- `docs/specs/error-handling/design.md` (기술 설계서)

**architect에게 전달할 사항**:
- WebView 클래스의 `loadError`, `loadTimeout` 시그널 인터페이스 확인 (`src/browser/WebView.h` 참조)
- ErrorPage 클래스: `QWidget` 상속, `showError(int, QString, QUrl)` 메서드
- BrowserWindow에 `QStackedLayout` 사용하여 WebView/ErrorPage 전환
- Qt 포커스 시스템 및 QPropertyAnimation 활용
