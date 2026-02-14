# 웹뷰 통합 — 요구사항 분석서 (Native App)

## 1. 개요

### 기능명
F-02: 웹뷰 통합 (WebView Integration)

### 목적
webOS Native SDK에서 제공하는 WebView API를 Qt/C++ 환경에 통합하여 외부 웹 페이지를 앱 내에서 안전하고 효율적으로 렌더링합니다. Web App PoC의 iframe 제약을 극복하여 실제 외부 사이트(YouTube, Netflix 등)를 로딩할 수 있는 브라우저 핵심 기능을 구현합니다.

### 대상 사용자
- 프로젝터로 웹 페이지를 탐색하려는 일반 사용자
- 스트리밍 서비스(YouTube, Netflix)를 이용하려는 사용자
- 뉴스, 블로그, 포털 사이트를 리모컨으로 탐색하려는 사용자

### 요청 배경
- **Web App PoC 제약 극복**: React/Enact 기반 Web App은 iframe의 Same-Origin Policy로 외부 사이트 렌더링 제약
- **F-01(프로젝트 초기 설정)** 완료로 Qt/CMake 기반 프로젝트 구조 확립
- **webOS Native API 활용**: webOS Native SDK의 WebView API는 실제 Chromium 엔진 기반으로 iframe 제약 없음
- 이후 F-04(페이지 탐색), F-05(로딩 인디케이터), F-06(탭 관리) 등이 이 기능에 의존
- PRD에서 정의한 "주요 사이트 렌더링 성공률 95% 이상" 달성의 기초

## 2. 유저 스토리

### US-1: 기본 웹 페이지 렌더링
- **As a** 프로젝터 사용자
- **I want** URL을 입력하면 해당 웹 페이지가 화면에 표시되기를
- **So that** 리모컨만으로 웹 콘텐츠를 즐길 수 있다

### US-2: 로딩 상태 확인
- **As a** 사용자
- **I want** 페이지가 로딩 중인지, 완료되었는지 알 수 있기를
- **So that** 페이지가 준비되기를 기다리며 불필요한 조작을 하지 않을 수 있다

### US-3: 로딩 실패 감지
- **As a** 사용자
- **I want** 페이지 로딩이 실패했을 때 에러 메시지를 보고 싶다
- **So that** 문제를 인지하고 재시도하거나 다른 URL을 시도할 수 있다

### US-4: 페이지 네비게이션 추적
- **As a** 사용자
- **I want** 웹 페이지 내에서 링크를 클릭했을 때 브라우저가 이를 인지하기를
- **So that** 뒤로 가기 버튼이나 URL 바가 자동으로 업데이트될 수 있다

### US-5: 안전한 웹뷰 종료
- **As a** 사용자
- **I want** 웹 페이지를 닫을 때 메모리가 제대로 해제되기를
- **So that** 여러 페이지를 탐색해도 앱이 느려지거나 멈추지 않는다

## 3. 기능 요구사항 (Functional Requirements)

### FR-1: webOS Native WebView API 조사 및 선택
- **설명**: webOS Native SDK에서 제공하는 웹 페이지 렌더링 API를 조사하고 적합한 방식 선택
- **우선순위**: Must
- **세부 요구사항**:
  - webOS Native SDK의 WebView 관련 API 조사:
    - `WebOSWebView` 클래스 (공식 문서 확인)
    - Qt WebEngineView와의 차이점 분석
    - webOS LS2 API 연동 가능성
  - 표준 Qt WebEngineView와 비교 분석:
    - Qt WebEngineView: 크로스플랫폼, Chromium 기반
    - webOS WebView: webOS 전용, LS2 API 연동, 메모리 최적화
  - 각 방식의 제약사항 파악:
    - 보안 정책 (CORS, CSP)
    - 리소스 접근 제한 (로컬 파일, webOS API)
    - 이벤트 시그널 지원 여부
    - 메모리/성능 차이
  - 선택 기준:
    - webOS 공식 지원 여부 (장기 유지보수)
    - 페이지 로딩 이벤트 시그널 수신 가능 여부
    - 네비게이션 제어 API (뒤로/앞으로/새로고침)
    - 에러 핸들링 시그널 지원 여부
  - 최종 선택 결과 문서화 (design.md에 반영)

### FR-2: WebView 래퍼 클래스 구현
- **설명**: 선택한 API를 캡슐화한 재사용 가능한 Qt 위젯 클래스 구축
- **우선순위**: Must
- **세부 요구사항**:
  - 클래스 위치: `src/browser/WebView.h`, `src/browser/WebView.cpp`
  - 클래스 정의:
    ```cpp
    namespace webosbrowser {

    class WebView : public QWidget {
        Q_OBJECT

    public:
        explicit WebView(QWidget *parent = nullptr);
        ~WebView();

        // URL 로딩 API
        void load(const QUrl &url);
        void reload();
        void stop();

        // 네비게이션 API (F-04에서 사용)
        bool canGoBack() const;
        bool canGoForward() const;
        void goBack();
        void goForward();

        // 상태 조회 API
        QUrl url() const;
        QString title() const;
        LoadingState state() const;

    signals:
        // 로딩 이벤트 시그널
        void loadStarted();
        void loadProgress(int progress); // 0~100
        void loadFinished(bool success);
        void titleChanged(const QString &title);

        // 네비게이션 이벤트 시그널
        void urlChanged(const QUrl &url);

        // 에러 이벤트 시그널
        void loadError(int errorCode, const QString &errorMessage, const QUrl &url);

    private:
        class WebViewPrivate;
        QScopedPointer<WebViewPrivate> d_ptr;
        Q_DECLARE_PRIVATE(WebView)
    };

    } // namespace webosbrowser
    ```
  - PIMPL 패턴 사용 (구현 세부사항 숨김)
  - Qt 시그널/슬롯 메커니즘 활용
  - 스마트 포인터 사용 (메모리 안전성)

### FR-3: 페이지 로딩 이벤트 처리
- **설명**: 웹 페이지 로딩의 각 단계를 감지하고 시그널로 전달
- **우선순위**: Must
- **세부 요구사항**:
  - 로딩 시작 이벤트 (`loadStarted` 시그널):
    - URL 로드 시작 시점 감지
    - 로딩 시작 시각 기록 (성능 모니터링용)
  - 로딩 진행률 이벤트 (`loadProgress` 시그널):
    - 진행률(0~100) 계산
    - 프로그레스바 표시를 위한 데이터 제공 (F-05 연계)
    - 주기적 업데이트 (100ms 간격)
  - 로딩 완료 이벤트 (`loadFinished` 시그널):
    - 페이지 렌더링 완료 감지
    - 성공/실패 플래그 전달
    - 로딩 소요 시간 로깅
  - 제목 변경 이벤트 (`titleChanged` 시그널):
    - 페이지 제목(document.title) 추출
    - 동적 제목 변경 감지 (SPA 대응)
    - 탭/북마크 표시용 데이터 제공

### FR-4: 네비게이션 이벤트 처리
- **설명**: 웹 페이지 내 링크 클릭 등으로 URL이 변경될 때 이를 추적
- **우선순위**: Must
- **세부 요구사항**:
  - URL 변경 감지 (`urlChanged` 시그널):
    - 링크 클릭 시 새 URL 감지
    - HTTP 리다이렉트 감지 (예: http → https)
    - Form 제출로 인한 URL 변경 감지
  - 변경된 URL을 상위 컴포넌트에 전달:
    - URLBar 업데이트용 (F-03 연계)
    - 히스토리 기록용 (F-08 연계)
  - 네비게이션 가능 여부 조회:
    - `canGoBack()`, `canGoForward()` API 구현
    - 뒤로/앞으로 버튼 활성화 상태 결정용 (F-04 연계)

### FR-5: 에러 처리
- **설명**: 페이지 로딩 실패 시 에러 정보를 수집하고 시그널로 전달
- **우선순위**: Must
- **세부 요구사항**:
  - 에러 타입 분류:
    - 네트워크 에러 (연결 실패, 타임아웃)
    - HTTP 에러 (404, 500 등)
    - SSL/TLS 에러 (인증서 오류)
    - 렌더링 에러 (잘못된 HTML, 스크립트 오류)
  - 에러 정보 구조:
    - `errorCode` (int): HTTP 상태 코드 또는 시스템 에러 코드
    - `errorMessage` (QString): 사용자에게 표시할 메시지
    - `url` (QUrl): 실패한 URL
  - 에러 시그널 발생:
    - `loadError(errorCode, errorMessage, url)` 시그널 emit
  - ErrorPage 표시 연계 (F-13)

### FR-6: 웹뷰 상태 관리
- **설명**: WebView의 현재 상태를 추적하고 관리
- **우선순위**: Must
- **세부 요구사항**:
  - 상태 열거형 정의 (LoadingState enum):
    ```cpp
    enum class LoadingState {
        Idle,      // 초기 상태 (URL 로드 전)
        Loading,   // 로딩 중
        Loaded,    // 로딩 완료
        Error      // 로딩 실패
    };
    ```
  - 상태 전환 규칙:
    - `Idle` → `Loading`: URL 로드 시작
    - `Loading` → `Loaded`: 로딩 성공
    - `Loading` → `Error`: 로딩 실패
    - `Loaded` → `Loading`: 새 URL 로드 시작
  - 상태 기반 UI 제어 준비:
    - `Loading` 시 로딩 인디케이터 표시 (F-05)
    - `Error` 시 에러 페이지 표시 (F-13)
    - `Loaded` 시 웹 콘텐츠 표시

### FR-7: 메모리 관리 및 리소스 해제
- **설명**: WebView 인스턴스의 생명주기를 관리하여 메모리 누수 방지
- **우선순위**: Must
- **세부 요구사항**:
  - 소멸자 구현:
    - 진행 중인 네트워크 요청 취소
    - WebView 내부 리소스 정리
    - 이벤트 핸들러 해제
  - URL 변경 시:
    - 이전 페이지 리소스 해제 (API 지원 시)
    - 캐시 정리 옵션 제공 (설정 연계)
  - 메모리 모니터링 (개발 모드):
    - 로딩 전후 메모리 사용량 로깅
    - 탭당 메모리 사용량 추적 준비 (F-06 연계)
  - 스마트 포인터 활용:
    - `QScopedPointer` 또는 `std::unique_ptr`로 PIMPL 구현
    - Raw 포인터 사용 최소화

### FR-8: BrowserWindow에 WebView 통합
- **설명**: 구현한 WebView 클래스를 메인 브라우저 창에 통합
- **우선순위**: Must
- **세부 요구사항**:
  - `src/browser/BrowserWindow.h`, `src/browser/BrowserWindow.cpp` 수정:
    - WebView 멤버 변수 추가
    - 초기 URL 설정 (테스트용, 예: https://www.google.com)
    - 로딩 이벤트 슬롯 구현 (qDebug() 로그)
  - 레이아웃 구성:
    - 상단: URL Bar 영역 (F-03에서 구현 예정, 현재는 QLabel 플레이스홀더)
    - 중앙: WebView (화면 대부분 차지)
    - 하단: Navigation Bar 영역 (F-04에서 구현 예정, 현재는 비어 있음)
  - Qt 시그널/슬롯 연결:
    ```cpp
    connect(webView, &WebView::loadStarted, this, [this]() {
        qDebug() << "Page loading started";
    });
    connect(webView, &WebView::loadProgress, this, [this](int progress) {
        qDebug() << "Loading progress:" << progress << "%";
    });
    connect(webView, &WebView::loadFinished, this, [this](bool success) {
        qDebug() << "Loading finished:" << (success ? "success" : "failed");
    });
    ```
  - 테스트 시나리오:
    - 앱 실행 → 초기 URL 자동 로드
    - 로딩 상태 콘솔 로그 출력
    - 페이지 완료 시 웹 콘텐츠 표시

## 4. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **페이지 로딩 시간**:
  - 일반 웹 페이지: 5초 이내 (PRD 기준)
  - 경량 페이지 (Google 홈): 2초 이내
  - 측정 기준: `loadStarted` ~ `loadFinished` 시그널 시간 차이
- **메모리 사용량**:
  - 단일 웹뷰 최대 200MB (PRD 기준)
  - 메모리 누수 없음 (1시간 연속 사용 시 메모리 증가 10% 이내)
- **이벤트 응답 속도**:
  - 로딩 이벤트 시그널 발생 지연: 100ms 이내
  - URL 변경 감지 지연: 200ms 이내

### UX (리모컨 최적화)
- **포커스 관리**:
  - WebView 내 링크/버튼에 리모컨 십자키로 포커스 이동 가능
  - Qt 키 이벤트 핸들링으로 리모컨 입력 처리
  - WebView 외부 UI(URL Bar, Navigation Bar)와 포커스 전환 매끄럽게
- **가독성**:
  - 웹 페이지 확대/축소 금지 (대화면 전제, viewport 제어)
  - 기본 폰트 크기 유지 (웹 페이지 자체 스타일 존중)
- **에러 복구**:
  - 에러 발생 시 명확한 메시지 표시 (F-13 연계)
  - 재시도 옵션 제공 (F-13 연계)

### 보안
- **HTTPS 우선**:
  - HTTP URL 입력 시 HTTPS로 자동 업그레이드 시도 (선택, F-14 연계)
  - 비보안 사이트 접속 시 경고 표시 준비 (F-14에서 구현)
- **쿠키 관리**:
  - WebView의 쿠키 저장소 격리 (다른 앱과 공유 방지)
  - 설정에서 쿠키 삭제 기능 준비 (F-10 연계)
- **SSL/TLS 검증**:
  - 인증서 오류 감지 및 경고
  - 자체 서명 인증서 차단 (보안 우선)

### 호환성
- **주요 사이트 렌더링 성공**:
  - 목표: 95% 이상 (PRD 기준)
  - 테스트 대상:
    - YouTube (https://www.youtube.com)
    - Netflix (https://www.netflix.com)
    - Google (https://www.google.com)
    - Naver (https://www.naver.com)
    - Wikipedia (https://www.wikipedia.org)
- **HTML5/CSS3 지원**:
  - webOS Chromium 엔진 기반 (버전에 따라 ES6+ 지원)
  - Canvas, Video, Audio 태그 동작 확인
  - WebGL 지원 여부 확인 (일부 게임 사이트)
- **webOS 버전**:
  - webOS 6.x 이상 지원 (LG 프로젝터 HU715QW 기준)
  - Qt 5.15+ 호환성

### 안정성
- **크래시 방지**:
  - 잘못된 URL (예: `invalid://url`) 입력 시에도 앱 크래시 없음
  - 메모리 부족 시 Graceful degradation (에러 메시지 표시)
  - QApplication 예외 처리
- **장시간 사용**:
  - 1시간 연속 사용 시 웹뷰 정상 동작
  - 여러 페이지 탐색 후에도 성능 저하 없음
- **에러 격리**:
  - 웹 페이지 스크립트 오류가 브라우저 앱에 영향 주지 않음
  - Qt 이벤트 루프 안정성 유지

### 확장성
- **다중 웹뷰 지원 준비**:
  - 현 단계: 단일 웹뷰
  - 향후(F-06): 탭별 웹뷰 인스턴스 관리
  - 설계 시 다중 인스턴스를 고려한 구조
- **WebView 설정 확장 가능**:
  - JavaScript on/off (향후 설정 기능)
  - User-Agent 커스터마이징 (향후 필요 시)
  - 캐시 정책 설정 (메모리/디스크 캐시)

## 5. 제약조건

### 플랫폼 제약
- **webOS Native WebView API 제한**:
  - 일부 웹 API가 제한될 수 있음 (예: Geolocation, WebRTC 제약)
  - CORS 정책이 일반 브라우저와 다를 수 있음 (webOS 보안 정책)
  - 파일 업로드/다운로드 기능 제약 (F-09에서 다루지만 API 지원 여부 확인 필요)
- **리모컨 입력**:
  - 웹 페이지의 마우스 이벤트를 리모컨 십자키 이벤트로 변환해야 함
  - Qt 키 이벤트 핸들링 필요 (QKeyEvent)
  - 일부 웹 페이지는 터치/마우스 전제로 설계되어 조작 어려움

### 기술 제약
- **메모리 제한**:
  - webOS 디바이스(프로젝터)의 RAM 제약 (일반적으로 1~2GB)
  - 탭당 200MB 이하로 메모리 사용 제한 (PRD)
- **네트워크**:
  - Wi-Fi 연결 필수 (유선랜 대안)
  - 네트워크 불안정 시 타임아웃 처리 필요 (기본 30초)
  - webOS 네트워크 스택 사용 (HTTPS/TLS 1.2+ 지원 확인)
- **Qt/webOS 통합**:
  - Qt 5.15+ 버전 사용 (C++17 표준)
  - webOS Native SDK와 Qt 라이브러리 충돌 방지
  - Qt 이벤트 루프와 webOS LS2 API 비동기 처리 호환

### 개발 환경 제약
- **webOS Native SDK 제약**:
  - macOS 개발 환경 (Linux 지원 가능)
  - 크로스 컴파일 필요 (ARM 타겟)
  - webOS CLI 도구 버전 호환성 (ares-package, ares-install)
- **디버깅 제약**:
  - 웹 페이지 내부 디버깅은 Chrome DevTools 원격 디버깅 필요
  - Qt Creator 디버거와 webOS 디바이스 연동 설정 복잡도 높음
  - qDebug() 로그는 webOS 로그 시스템에 통합

### 의존성
- **선행 기능**:
  - F-01 (프로젝트 초기 설정) 완료 필요
- **후속 기능**:
  - F-03 (URL 입력 UI) - WebView에 URL 전달
  - F-04 (페이지 탐색 컨트롤) - WebView 네비게이션 API 호출
  - F-05 (로딩 인디케이터) - WebView 로딩 시그널 구독
  - F-06 (탭 관리) - 다중 WebView 인스턴스 관리
  - F-08 (히스토리 관리) - WebView URL 변경 시그널 저장
  - F-13 (에러 처리) - WebView 에러 시그널로 에러 페이지 표시

## 6. 용어 정의

| 용어 | 정의 |
|------|------|
| WebView | 앱 내에서 웹 콘텐츠를 렌더링하는 UI 위젯 (네이티브 브라우저 엔진 사용) |
| Qt WebEngineView | Qt 프레임워크의 Chromium 기반 WebView 위젯 (크로스플랫폼) |
| webOS WebView | webOS Native SDK의 WebView API (webOS 전용, 최적화) |
| PIMPL | Pointer to Implementation, C++ 디자인 패턴 (구현 세부사항 숨김) |
| CORS | Cross-Origin Resource Sharing, 다른 도메인의 리소스 접근 보안 정책 |
| CSP | Content Security Policy, 웹 페이지 콘텐츠 보안 정책 (스크립트 실행 제한 등) |
| SPA | Single Page Application, 페이지 전환 없이 동적으로 콘텐츠를 변경하는 웹 앱 |
| LS2 API | Luna Service 2 API, webOS의 시스템 서비스 API (비동기 메시지 버스) |
| Qt 시그널/슬롯 | Qt의 이벤트 처리 메커니즘 (옵저버 패턴) |
| User-Agent | HTTP 요청 헤더, 브라우저/앱 식별 정보 (서버가 모바일/데스크톱 구분) |

## 7. 범위 외 (Out of Scope)

### 이번 단계에서 하지 않는 것
- **URL 입력 UI**: F-03에서 구현 (현 단계는 하드코딩된 초기 URL 사용)
- **뒤로/앞으로 버튼 UI**: F-04에서 구현 (현 단계는 WebView API만 준비)
- **프로그레스바 UI**: F-05에서 구현 (현 단계는 qDebug() 로그만)
- **다중 탭**: F-06에서 구현 (현 단계는 단일 WebView)
- **북마크/히스토리 저장**: F-07, F-08에서 구현 (현 단계는 URL 변경 감지만)
- **다운로드 처리**: F-09에서 구현
- **HTTPS 보안 표시**: F-14에서 구현
- **에러 페이지 UI**: F-13에서 구현 (현 단계는 loadError 시그널만)
- **설정 기능**: F-10에서 구현 (JavaScript on/off, User-Agent 등)

### 추후 확장 가능 기능
- **광고 차단**: 현재 범위 외, 향후 플러그인으로 추가 가능
- **스크린샷**: webOS API 지원 시 구현 가능
- **페이지 내 검색** (Ctrl+F 같은): 리모컨 조작 복잡도 높아 보류
- **시크릿 모드**: 쿠키/히스토리 저장 안 함 (추후 확장)
- **개발자 도구**: Chrome DevTools 통합 (디버그 빌드만)

## 8. 완료 기준 (Acceptance Criteria)

### AC-1: webOS Native WebView API 조사 완료
- [ ] webOS Native SDK의 WebView API 문서 확인
- [ ] Qt WebEngineView와 비교 분석 문서 작성
- [ ] 최종 선택 결정 및 근거 문서화 (design.md에 포함)

### AC-2: WebView 클래스 구현 완료
- [ ] `src/browser/WebView.h` 헤더 파일 작성
- [ ] `src/browser/WebView.cpp` 소스 파일 작성
- [ ] PIMPL 패턴 구현 (WebViewPrivate 클래스)
- [ ] 공개 API 정의 (load, reload, stop, goBack, goForward 등)
- [ ] 주석 추가 (한국어, Doxygen 스타일)

### AC-3: 로딩 이벤트 시그널 구현 완료
- [ ] `loadStarted` 시그널 emit 확인 (qDebug() 로그)
- [ ] `loadProgress` 진행률(0~100) 시그널 emit 확인
- [ ] `loadFinished` 시그널 emit 및 성공/실패 플래그 확인
- [ ] `titleChanged` 시그널 emit 및 페이지 제목 추출 확인
- [ ] 로딩 소요 시간 계산 및 로깅

### AC-4: 네비게이션 이벤트 시그널 구현 완료
- [ ] 페이지 내 링크 클릭 시 `urlChanged` 시그널 emit 확인
- [ ] 새 URL 정상 전달 확인
- [ ] 리다이렉트 감지 확인 (예: http → https)
- [ ] `canGoBack()`, `canGoForward()` API 동작 확인

### AC-5: 에러 처리 완료
- [ ] 네트워크 에러 감지 (Wi-Fi 끄고 테스트)
- [ ] 404 에러 감지 (존재하지 않는 URL)
- [ ] SSL/TLS 에러 감지 (자체 서명 인증서)
- [ ] `loadError` 시그널 emit 및 에러 정보(errorCode, errorMessage, url) 확인

### AC-6: 상태 관리 완료
- [ ] `LoadingState` enum 정의
- [ ] 상태 전이 구현 (Idle → Loading → Loaded/Error)
- [ ] `state()` API로 현재 상태 조회 가능
- [ ] qDebug()로 상태 변경 로그 출력

### AC-7: 메모리 관리 확인
- [ ] 소멸자에서 리소스 해제 확인 (qDebug() 로그)
- [ ] 10개 페이지 연속 로드 후 메모리 사용량 확인 (200MB 이하)
- [ ] 1시간 연속 사용 후 메모리 누수 확인 (10% 이내 증가)
- [ ] 스마트 포인터 사용 (QScopedPointer, std::unique_ptr)

### AC-8: BrowserWindow 통합 완료
- [ ] `src/browser/BrowserWindow.h`, `BrowserWindow.cpp` 수정
- [ ] WebView 멤버 변수 추가 및 레이아웃 배치
- [ ] 초기 URL (https://www.google.com) 자동 로드
- [ ] 로딩 이벤트 슬롯 구현 (qDebug() 로그)
- [ ] 페이지 렌더링 확인 (Google 홈페이지 표시)

### AC-9: 주요 사이트 렌더링 테스트
- [ ] YouTube (https://www.youtube.com) 정상 렌더링
- [ ] Netflix (https://www.netflix.com) 정상 렌더링 (DRM 제약 확인)
- [ ] Google (https://www.google.com) 정상 렌더링
- [ ] Naver (https://www.naver.com) 정상 렌더링
- [ ] Wikipedia (https://www.wikipedia.org) 정상 렌더링
- [ ] 렌더링 성공률: 5개 중 최소 4개 (80% 이상, 목표는 95%)

### AC-10: 리모컨 조작 확인
- [ ] WebView 포커스 시 십자키로 링크 간 이동 가능
- [ ] Enter 키로 링크 클릭 가능
- [ ] Back 키로 뒤로 가기 동작 (F-04에서 구현, 현 단계는 API만)

### AC-11: 성능 기준 충족
- [ ] Google 홈페이지 로딩 시간: 2초 이내
- [ ] 일반 페이지 로딩 시간: 5초 이내 (예: Naver 홈)
- [ ] 이벤트 시그널 지연: 100ms 이내

### AC-12: 실제 디바이스 테스트
- [ ] LG 프로젝터 HU715QW에서 앱 실행 성공
- [ ] WebView 정상 렌더링 확인
- [ ] 리모컨으로 페이지 조작 확인

### AC-13: 빌드 및 패키징
- [ ] CMake 빌드 성공 (`cmake .. && make`)
- [ ] IPK 패키지 생성 성공 (`ares-package`)
- [ ] 프로젝터 설치 성공 (`ares-install`)

### AC-14: 문서화
- [ ] WebView 클래스 주석 (한국어, Doxygen) 추가
- [ ] 공개 API 문서화 (시그널, 슬롯, 메서드)
- [ ] 사용 예시 추가 (BrowserWindow 코드)
- [ ] API 선택 근거 문서 (design.md에 포함)

## 9. 우선순위 및 복잡도

- **우선순위**: Must (브라우저 앱의 핵심 기능)
- **예상 복잡도**: High
  - webOS Native API 학습 곡선
  - Qt/webOS 통합 복잡도
  - 이벤트 시그널 처리 구현 복잡도
  - 리모컨 입력 처리 (Qt 키 이벤트)
  - 실제 디바이스 테스트 필요성
- **예상 소요 시간**: 2~3일
  - API 조사 및 선택: 4~6시간
  - WebView 클래스 구현: 6~8시간
  - BrowserWindow 통합: 2~3시간
  - 테스트 및 디버깅: 4~6시간
  - 디바이스 테스트: 2~3시간
  - 문서화: 2~3시간

## 10. 리스크

| 리스크 | 확률 | 영향도 | 대응 방안 |
|--------|------|--------|-----------|
| webOS Native WebView API 문서 부족 | 중 | 고 | webOS 개발자 포럼, LG 공식 지원 활용. 최악의 경우 Qt WebEngineView fallback |
| 주요 사이트 렌더링 실패 (예: Netflix DRM) | 중 | 중 | 사이트별 호환성 테스트, DRM 지원 여부 확인. 불가능한 사이트는 사용자에게 안내 |
| 리모컨 포커스와 웹 페이지 내 포커스 충돌 | 고 | 중 | Qt 키 이벤트 핸들링 세밀 조정, WebView 내부 포커스는 웹 표준 탐색 사용 |
| 메모리 누수 발생 | 중 | 고 | 철저한 리소스 해제 구현, Qt 스마트 포인터 활용, 프로파일링 도구 사용 |
| 네트워크 불안정 시 타임아웃 처리 미흡 | 중 | 중 | 타임아웃 설정 (30초), 재시도 옵션 제공 (F-13 연계) |
| 실제 디바이스에서만 발생하는 버그 | 고 | 중 | 조기 실제 디바이스 테스트, 로그 수집 메커니즘 구축 (qDebug → webOS 로그) |
| Qt/webOS 버전 호환성 문제 | 중 | 고 | webOS Native SDK 공식 지원 Qt 버전 사용 (Qt 5.15), CMake 설정 주의 |

## 11. 의존성

### 선행 기능
- **F-01 (프로젝트 초기 설정)**: 완료 필요

### 후속 기능
- **F-03 (URL 입력 UI)**: WebView에 동적 URL 전달
- **F-04 (페이지 탐색 컨트롤)**: WebView 네비게이션 API 호출 (뒤로/앞으로)
- **F-05 (로딩 인디케이터)**: WebView 로딩 시그널 구독
- **F-06 (탭 관리)**: 다중 WebView 인스턴스 관리
- **F-08 (히스토리 관리)**: WebView URL 변경 시그널 저장
- **F-13 (에러 처리)**: WebView 에러 시그널로 에러 페이지 표시

## 12. 참고 자료

### webOS Native API 문서
- webOS Native App 개발 가이드: https://webostv.developer.lge.com/develop/native-apps/native-app-overview
- webOS Qt API 참조: https://webostv.developer.lge.com/develop/native-apps/webos-qt-overview
- Luna Service 2 (LS2) API: https://webostv.developer.lge.com/develop/references/luna-service2-api/

### Qt 문서
- Qt 5 Documentation: https://doc.qt.io/qt-5/
- Qt WebEngineView: https://doc.qt.io/qt-5/qwebengineview.html
- Qt Signals and Slots: https://doc.qt.io/qt-5/signalsandslots.html
- Qt Smart Pointers: https://doc.qt.io/qt-5/smart-pointers.html

### 프로젝트 문서
- PRD: `/Users/jsong/dev/jsong1230-github/webosbrowser-native/docs/project/prd.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser-native/CLAUDE.md`
- F-01 요구사항: `/Users/jsong/dev/jsong1230-github/webosbrowser-native/docs/specs/project-setup/requirements.md`
- F-01 설계서: `/Users/jsong/dev/jsong1230-github/webosbrowser-native/docs/specs/project-setup/design.md`

### 기술 참고
- C++17 스마트 포인터: https://en.cppreference.com/w/cpp/memory
- PIMPL 패턴: https://en.cppreference.com/w/cpp/language/pimpl
- Qt 이벤트 시스템: https://doc.qt.io/qt-5/eventsandfilters.html

## 13. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초기 요구사항 분석서 작성 (Native App 관점) | product-manager |
| 2026-02-14 | Web App PoC 문서 기반으로 Qt/C++ 기술 스택 업데이트 | product-manager |
