# 웹뷰 통합 — 기술 설계서 (Native App)

## 1. 참조
- 요구사항 분석서: docs/specs/webview-integration/requirements.md
- PRD: docs/project/prd.md
- F-01 설계서: docs/specs/project-setup/design.md
- CLAUDE.md: /Users/jsong/dev/jsong1230-github/webosbrowser-native/CLAUDE.md

## 2. 아키텍처 개요

### 전체 구조
webOS Native SDK의 WebView API를 Qt/C++ 환경에 통합하여 Chromium 기반의 실제 웹 페이지 렌더링을 구현합니다. Web App PoC의 iframe 제약을 극복하여 YouTube, Netflix 등 외부 사이트를 로딩할 수 있는 브라우저 핵심 기능을 구축합니다.

```
┌─────────────────────────────────────────────────────────┐
│                    BrowserWindow (QMainWindow)           │
│  ┌────────────────────────────────────────────────────┐ │
│  │              QLabel (URLBar Placeholder)            │ │
│  │                    (F-03에서 구현)                   │ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │             WebView (QWidget)                       │ │
│  │  ┌──────────────────────────────────────────────┐  │ │
│  │  │    QWebEngineView (Chromium 엔진)             │  │ │
│  │  │    - url: QUrl                                │  │ │
│  │  │    - signals: loadStarted, loadProgress,      │  │ │
│  │  │               loadFinished, titleChanged      │  │ │
│  │  └──────────────────────────────────────────────┘  │ │
│  │  [ WebViewPrivate: PIMPL 구현 클래스 ]             │ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │             QWidget (NavBar Placeholder)            │ │
│  │                    (F-04에서 구현)                   │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### 핵심 설계 원칙
1. **Qt WebEngineView 활용**: Qt의 Chromium 기반 WebView 위젯 사용 (webOS 네이티브 지원)
2. **PIMPL 패턴**: 구현 세부사항을 숨기고 인터페이스 안정성 확보
3. **시그널/슬롯 아키텍처**: Qt의 이벤트 메커니즘으로 느슨한 결합 구현
4. **스마트 포인터**: RAII 원칙으로 메모리 안전성 보장
5. **리모컨 키 이벤트 처리**: QKeyEvent로 webOS 리모컨 입력 처리

## 3. 아키텍처 결정

### 결정 1: Qt WebEngineView vs webOS 전용 WebView API
- **선택지**:
  - A) Qt WebEngineView (Qt 5.15 표준 컴포넌트)
  - B) webOS Native WebView API (공식 문서 확인 필요)
  - C) 커스텀 Chromium 래퍼 (직접 구현)
- **결정**: A) Qt WebEngineView
- **근거**:
  - **공식 지원**: Qt for webOS는 Qt WebEngineView를 공식 지원하며 webOS 플랫폼에 최적화됨
  - **크로스플랫폼**: 로컬 개발 환경(macOS)에서도 동일한 API로 테스트 가능
  - **풍부한 API**: loadStarted, loadProgress, loadFinished, titleChanged 등 모든 필수 이벤트 제공
  - **안정성**: Qt 5.15는 장기 지원(LTS) 버전으로 버그 수정 지원
  - **문서화**: Qt 공식 문서가 상세하고 커뮤니티 활성화
- **트레이드오프**:
  - **포기**: webOS 전용 API의 플랫폼 특화 최적화 (존재 시)
  - **대응**: Qt WebEngineView가 webOS에서 충분히 성능을 제공하며, 추후 필요 시 프로파일링 후 최적화

### 결정 2: PIMPL 패턴 사용
- **선택지**:
  - A) PIMPL 패턴 (Pointer to Implementation)
  - B) 직접 멤버 변수 노출
  - C) Qt의 Q_D/Q_Q 매크로 사용
- **결정**: A) PIMPL 패턴 (QScopedPointer 사용)
- **근거**:
  - **ABI 안정성**: 구현 변경 시 헤더 파일을 수정하지 않아 재컴파일 범위 최소화
  - **캡슐화**: QWebEngineView 등 내부 구현을 완전히 숨겨 의존성 제거
  - **Qt 모범 사례**: Qt 자체도 PIMPL 패턴을 널리 사용 (QObject의 d 포인터)
  - **메모리 안전성**: QScopedPointer로 자동 메모리 관리
- **트레이드오프**: 약간의 코드 복잡도 증가, 하지만 유지보수성과 확장성에서 이득

### 결정 3: 상태 관리 메커니즘
- **선택지**:
  - A) enum class LoadingState + 멤버 변수
  - B) QStateMachine (Qt 상태 머신)
  - C) 상태 없이 시그널만 사용
- **결정**: A) enum class LoadingState + 멤버 변수
- **근거**:
  - **단순성**: 4가지 상태(Idle, Loading, Loaded, Error)만 관리하므로 enum으로 충분
  - **명확성**: state() API로 현재 상태를 명시적으로 조회 가능
  - **성능**: QStateMachine의 오버헤드 불필요
- **트레이드오프**: 복잡한 상태 전이가 필요하면 QStateMachine으로 리팩토링 가능 (현 단계는 불필요)

### 결정 4: 에러 처리 전략
- **선택지**:
  - A) QWebEngineView의 loadFinished(bool ok) 시그널만 사용
  - B) QWebEnginePage::certificateError, renderProcessTerminated 등 세밀한 에러 처리
  - C) 타임아웃 기반 에러 감지 추가
- **결정**: A) + C) 조합 (B는 F-13에서 구현)
- **근거**:
  - **현 단계 범위**: loadFinished(false)로 기본 에러 감지 가능
  - **타임아웃**: 30초 이상 로딩 시 QTimer로 에러 판단
  - **세밀한 처리**: SSL 에러, 프로세스 크래시 등은 F-13(에러 처리 기능)에서 구현
- **에러 코드 정의**:
  - `-1`: 네트워크 에러 (일반)
  - `-2`: 타임아웃 에러 (30초 초과)
  - HTTP 상태 코드: 404, 500 등 (QNetworkReply에서 추출 가능, F-13에서 구현)

### 결정 5: 리모컨 키 이벤트 처리
- **선택지**:
  - A) WebView 클래스에서 keyPressEvent 오버라이드
  - B) BrowserWindow에서 중앙 집중 키 이벤트 처리
  - C) Qt Event Filter 사용
- **결정**: B) BrowserWindow에서 중앙 집중 처리 (F-04, F-12에서 구현)
- **근거**:
  - **현 단계 범위**: WebView는 웹 콘텐츠 렌더링만 담당
  - **포커스 관리**: 리모컨 뒤로 버튼 등은 BrowserWindow에서 URLBar↔WebView↔NavBar 포커스 전환 제어
  - **확장성**: F-12(리모컨 단축키)에서 Home, Menu 등 추가 키 처리 시 한 곳에서 관리
- **트레이드오프**: WebView 내부 키 이벤트는 웹 페이지 자체 처리 (tabindex, accesskey)

### 결정 6: 메모리 관리 전략
- **선택지**:
  - A) QWebEngineView를 deleteLater()로 지연 삭제
  - B) QScopedPointer로 즉시 삭제
  - C) QWebEnginePage::deleteLater() + QWebEngineView 재사용
- **결정**: B) QScopedPointer로 즉시 삭제 (PIMPL 패턴과 결합)
- **근거**:
  - **RAII 원칙**: WebView 소멸자에서 자동으로 WebViewPrivate → QWebEngineView 삭제
  - **명확성**: 객체 생명주기가 명확하여 메모리 누수 방지
  - **Qt 이벤트 루프**: QScopedPointer 삭제는 이벤트 루프와 안전하게 통합됨
- **주의사항**: QWebEngineView는 Qt 이벤트 루프가 동작 중일 때만 안전하게 삭제 가능 (앱 종료 시 정상 종료 필수)

## 4. WebView 클래스 설계

### 클래스 구조
```
src/browser/
├── WebView.h           # 공개 인터페이스 (PIMPL 패턴)
├── WebView.cpp         # 구현 + WebViewPrivate 클래스 정의
```

### WebView.h (공개 인터페이스)
```cpp
/**
 * @file WebView.h
 * @brief webOS WebView API 래퍼 클래스 (Qt WebEngineView 기반)
 */

#pragma once

#include <QWidget>
#include <QUrl>
#include <QScopedPointer>

namespace webosbrowser {

/**
 * @enum LoadingState
 * @brief 웹뷰 로딩 상태
 */
enum class LoadingState {
    Idle,      ///< 초기 상태 (URL 로드 전)
    Loading,   ///< 로딩 중
    Loaded,    ///< 로딩 완료
    Error      ///< 로딩 실패
};

/**
 * @class WebView
 * @brief Qt WebEngineView를 캡슐화한 브라우저 웹뷰 컴포넌트
 *
 * PIMPL 패턴으로 구현 세부사항을 숨기고, Qt 시그널/슬롯으로
 * 로딩 이벤트를 상위 컴포넌트에 전달합니다.
 */
class WebView : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param parent 부모 위젯 (기본값: nullptr)
     */
    explicit WebView(QWidget *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~WebView() override;

    // 복사 생성자 및 대입 연산자 삭제 (QObject 규칙)
    WebView(const WebView&) = delete;
    WebView& operator=(const WebView&) = delete;

    /**
     * @brief URL 로딩
     * @param url 로드할 URL
     *
     * 로딩 시작 시 loadStarted 시그널이 emit되며,
     * 완료 시 loadFinished(true) 또는 실패 시 loadFinished(false) emit.
     */
    void load(const QUrl &url);

    /**
     * @brief 현재 페이지 새로고침
     */
    void reload();

    /**
     * @brief 로딩 중단
     */
    void stop();

    /**
     * @brief 뒤로 가기 가능 여부
     * @return 뒤로 갈 히스토리가 있으면 true
     */
    bool canGoBack() const;

    /**
     * @brief 앞으로 가기 가능 여부
     * @return 앞으로 갈 히스토리가 있으면 true
     */
    bool canGoForward() const;

    /**
     * @brief 뒤로 가기
     */
    void goBack();

    /**
     * @brief 앞으로 가기
     */
    void goForward();

    /**
     * @brief 현재 URL 조회
     * @return 현재 로드된 URL
     */
    QUrl url() const;

    /**
     * @brief 페이지 제목 조회
     * @return 페이지 제목 (document.title)
     */
    QString title() const;

    /**
     * @brief 현재 로딩 상태 조회
     * @return LoadingState enum 값
     */
    LoadingState state() const;

signals:
    /**
     * @brief 로딩 시작 시그널
     *
     * load() 호출 또는 웹 페이지 내 링크 클릭 시 발생
     */
    void loadStarted();

    /**
     * @brief 로딩 진행률 시그널
     * @param progress 진행률 (0~100)
     *
     * 페이지 리소스(HTML, 이미지, 스크립트) 로딩 진행 상황 전달
     */
    void loadProgress(int progress);

    /**
     * @brief 로딩 완료 시그널
     * @param success 성공 여부 (true: 성공, false: 실패)
     *
     * 성공 시 웹 콘텐츠가 표시되며, 실패 시 loadError 시그널도 발생
     */
    void loadFinished(bool success);

    /**
     * @brief 페이지 제목 변경 시그널
     * @param title 새 제목
     *
     * document.title이 변경될 때 발생 (SPA 대응)
     */
    void titleChanged(const QString &title);

    /**
     * @brief URL 변경 시그널
     * @param url 새 URL
     *
     * 웹 페이지 내 링크 클릭, HTTP 리다이렉트 시 발생
     */
    void urlChanged(const QUrl &url);

    /**
     * @brief 로딩 에러 시그널
     * @param errorCode 에러 코드 (-1: 네트워크, -2: 타임아웃, HTTP 상태 코드)
     * @param errorMessage 사용자에게 표시할 에러 메시지
     * @param url 실패한 URL
     *
     * 네트워크 에러, 타임아웃, HTTP 에러 시 발생
     */
    void loadError(int errorCode, const QString &errorMessage, const QUrl &url);

private:
    class WebViewPrivate;  ///< PIMPL 구현 클래스
    QScopedPointer<WebViewPrivate> d_ptr;  ///< Private 구현 포인터
    Q_DECLARE_PRIVATE(WebView)  ///< Qt PIMPL 매크로
};

} // namespace webosbrowser
```

### WebView.cpp (구현 + Private 클래스)
```cpp
/**
 * @file WebView.cpp
 * @brief WebView 구현 (PIMPL 패턴)
 */

#include "WebView.h"
#include <QWebEngineView>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>

namespace webosbrowser {

/**
 * @class WebView::WebViewPrivate
 * @brief WebView의 Private 구현 클래스 (PIMPL 패턴)
 */
class WebView::WebViewPrivate {
public:
    WebViewPrivate(WebView *q)
        : q_ptr(q)
        , webEngineView(new QWebEngineView(q))
        , currentState(LoadingState::Idle)
        , loadTimeout(30000)  // 30초 타임아웃
        , loadStartTime(0)
    {
        // QTimer 초기화 (타임아웃 감지용)
        timeoutTimer = new QTimer(q);
        timeoutTimer->setSingleShot(true);
    }

    ~WebViewPrivate() {
        // QScopedPointer 자동 삭제
    }

    WebView *q_ptr;                     ///< 공개 인터페이스 포인터 (Q_Q 매크로용)
    QWebEngineView *webEngineView;      ///< Qt WebEngineView 인스턴스
    LoadingState currentState;          ///< 현재 로딩 상태
    QTimer *timeoutTimer;               ///< 타임아웃 타이머
    int loadTimeout;                    ///< 타임아웃 시간 (밀리초)
    qint64 loadStartTime;               ///< 로딩 시작 시각 (밀리초)

    Q_DECLARE_PUBLIC(WebView)  ///< Qt PIMPL 매크로
};

// 생성자
WebView::WebView(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new WebViewPrivate(this))
{
    Q_D(WebView);

    qDebug() << "[WebView] 생성 중...";

    // 레이아웃 설정 (QWebEngineView를 전체 화면에 배치)
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->webEngineView);
    setLayout(layout);

    // 시그널/슬롯 연결
    connect(d->webEngineView, &QWebEngineView::loadStarted,
            this, &WebView::onLoadStarted);
    connect(d->webEngineView, &QWebEngineView::loadProgress,
            this, &WebView::onLoadProgress);
    connect(d->webEngineView, &QWebEngineView::loadFinished,
            this, &WebView::onLoadFinished);
    connect(d->webEngineView, &QWebEngineView::titleChanged,
            this, &WebView::titleChanged);  // 직접 전달
    connect(d->webEngineView, &QWebEngineView::urlChanged,
            this, &WebView::urlChanged);    // 직접 전달

    // 타임아웃 타이머 연결
    connect(d->timeoutTimer, &QTimer::timeout,
            this, &WebView::onLoadTimeout);

    qDebug() << "[WebView] 생성 완료";
}

// 소멸자
WebView::~WebView() {
    Q_D(WebView);
    qDebug() << "[WebView] 소멸";

    // 타임아웃 타이머 정지
    if (d->timeoutTimer->isActive()) {
        d->timeoutTimer->stop();
    }

    // QScopedPointer가 자동으로 d_ptr 삭제 → WebViewPrivate 소멸
    // → QWebEngineView는 QObject 부모-자식 관계로 자동 삭제됨
}

// URL 로딩
void WebView::load(const QUrl &url) {
    Q_D(WebView);
    qDebug() << "[WebView] URL 로드 시작:" << url.toString();

    d->webEngineView->load(url);
    // onLoadStarted 슬롯에서 상태 변경 및 시그널 emit
}

// 새로고침
void WebView::reload() {
    Q_D(WebView);
    qDebug() << "[WebView] 새로고침";
    d->webEngineView->reload();
}

// 로딩 중단
void WebView::stop() {
    Q_D(WebView);
    qDebug() << "[WebView] 로딩 중단";

    d->webEngineView->stop();

    // 타임아웃 타이머 정지
    if (d->timeoutTimer->isActive()) {
        d->timeoutTimer->stop();
    }

    // 상태를 Idle로 복원
    d->currentState = LoadingState::Idle;
}

// 뒤로 가기 가능 여부
bool WebView::canGoBack() const {
    Q_D(const WebView);
    return d->webEngineView->history()->canGoBack();
}

// 앞으로 가기 가능 여부
bool WebView::canGoForward() const {
    Q_D(const WebView);
    return d->webEngineView->history()->canGoForward();
}

// 뒤로 가기
void WebView::goBack() {
    Q_D(WebView);
    qDebug() << "[WebView] 뒤로 가기";
    d->webEngineView->back();
}

// 앞으로 가기
void WebView::goForward() {
    Q_D(WebView);
    qDebug() << "[WebView] 앞으로 가기";
    d->webEngineView->forward();
}

// 현재 URL 조회
QUrl WebView::url() const {
    Q_D(const WebView);
    return d->webEngineView->url();
}

// 페이지 제목 조회
QString WebView::title() const {
    Q_D(const WebView);
    return d->webEngineView->title();
}

// 현재 상태 조회
LoadingState WebView::state() const {
    Q_D(const WebView);
    return d->currentState;
}

// Private 슬롯: 로딩 시작
void WebView::onLoadStarted() {
    Q_D(WebView);
    qDebug() << "[WebView] 로딩 시작 (onLoadStarted)";

    // 상태 변경
    d->currentState = LoadingState::Loading;
    d->loadStartTime = QDateTime::currentMSecsSinceEpoch();

    // 타임아웃 타이머 시작 (30초)
    d->timeoutTimer->start(d->loadTimeout);

    // 시그널 emit
    emit loadStarted();
}

// Private 슬롯: 로딩 진행률
void WebView::onLoadProgress(int progress) {
    qDebug() << "[WebView] 로딩 진행률:" << progress << "%";
    emit loadProgress(progress);
}

// Private 슬롯: 로딩 완료
void WebView::onLoadFinished(bool ok) {
    Q_D(WebView);

    // 타임아웃 타이머 정지
    if (d->timeoutTimer->isActive()) {
        d->timeoutTimer->stop();
    }

    // 로딩 소요 시간 계산
    qint64 duration = QDateTime::currentMSecsSinceEpoch() - d->loadStartTime;
    qDebug() << "[WebView] 로딩 완료:" << (ok ? "성공" : "실패")
             << "/ 소요 시간:" << duration << "ms";

    if (ok) {
        // 성공
        d->currentState = LoadingState::Loaded;
        emit loadFinished(true);
    } else {
        // 실패
        d->currentState = LoadingState::Error;

        // 에러 시그널 emit (기본 네트워크 에러)
        emit loadError(-1, "페이지를 로드할 수 없습니다", d->webEngineView->url());
        emit loadFinished(false);
    }
}

// Private 슬롯: 타임아웃
void WebView::onLoadTimeout() {
    Q_D(WebView);

    // 아직 로딩 중인 경우에만 타임아웃 처리
    if (d->currentState == LoadingState::Loading) {
        qDebug() << "[WebView] 로딩 타임아웃 (30초 초과)";

        // 로딩 중단
        d->webEngineView->stop();

        // 상태 변경
        d->currentState = LoadingState::Error;

        // 에러 시그널 emit
        emit loadError(-2, "페이지 로딩 시간이 초과되었습니다 (30초)", d->webEngineView->url());
        emit loadFinished(false);
    }
}

} // namespace webosbrowser
```

**주의**: 위 코드의 `onLoadStarted`, `onLoadProgress`, `onLoadFinished`, `onLoadTimeout`은 private 슬롯으로, WebView.h에 다음과 같이 추가 필요:
```cpp
private slots:
    void onLoadStarted();
    void onLoadProgress(int progress);
    void onLoadFinished(bool ok);
    void onLoadTimeout();
```

## 5. BrowserWindow 통합 설계

### BrowserWindow.h 수정
```cpp
// BrowserWindow.h에 추가
#include "WebView.h"

class BrowserWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = nullptr);
    ~BrowserWindow() override;

private:
    void setupUI();
    void setupConnections();

    // WebView 이벤트 핸들러 (슬롯)
private slots:
    void onWebViewLoadStarted();
    void onWebViewLoadProgress(int progress);
    void onWebViewLoadFinished(bool success);
    void onWebViewTitleChanged(const QString &title);
    void onWebViewUrlChanged(const QUrl &url);
    void onWebViewLoadError(int errorCode, const QString &errorMessage, const QUrl &url);

private:
    QWidget *centralWidget_;
    QVBoxLayout *mainLayout_;
    QLabel *urlBarPlaceholder_;  // URLBar 플레이스홀더 (F-03에서 대체)
    WebView *webView_;           // WebView 인스턴스
    QWidget *navBarPlaceholder_; // NavBar 플레이스홀더 (F-04에서 대체)
};
```

### BrowserWindow.cpp 수정
```cpp
#include "BrowserWindow.h"
#include "WebView.h"
#include <QDebug>

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget_(new QWidget(this))
    , mainLayout_(new QVBoxLayout(centralWidget_))
    , urlBarPlaceholder_(new QLabel("URL: https://www.google.com", centralWidget_))
    , webView_(new WebView(centralWidget_))
    , navBarPlaceholder_(new QWidget(centralWidget_))
{
    qDebug() << "[BrowserWindow] 생성 중...";

    setupUI();
    setupConnections();

    // 초기 URL 로드 (테스트용)
    webView_->load(QUrl("https://www.google.com"));

    qDebug() << "[BrowserWindow] 생성 완료";
}

BrowserWindow::~BrowserWindow() {
    qDebug() << "[BrowserWindow] 소멸";
}

void BrowserWindow::setupUI() {
    // 윈도우 기본 설정
    setWindowTitle("webOS Browser");
    resize(1920, 1080);  // webOS 기본 해상도

    // 중앙 위젯 설정
    setCentralWidget(centralWidget_);

    // URLBar 플레이스홀더 스타일
    urlBarPlaceholder_->setFixedHeight(60);
    urlBarPlaceholder_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    urlBarPlaceholder_->setStyleSheet(
        "QLabel {"
        "  font-size: 24px;"
        "  padding: 10px 20px;"
        "  background-color: #2a2a2a;"
        "  color: #ffffff;"
        "}"
    );

    // NavBar 플레이스홀더 스타일
    navBarPlaceholder_->setFixedHeight(80);
    navBarPlaceholder_->setStyleSheet("background-color: #2a2a2a;");

    // 레이아웃 설정
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);
    mainLayout_->addWidget(urlBarPlaceholder_);
    mainLayout_->addWidget(webView_, 1);  // stretch factor = 1 (남은 공간 차지)
    mainLayout_->addWidget(navBarPlaceholder_);

    qDebug() << "[BrowserWindow] UI 설정 완료";
}

void BrowserWindow::setupConnections() {
    // WebView 시그널 → BrowserWindow 슬롯 연결
    connect(webView_, &WebView::loadStarted,
            this, &BrowserWindow::onWebViewLoadStarted);
    connect(webView_, &WebView::loadProgress,
            this, &BrowserWindow::onWebViewLoadProgress);
    connect(webView_, &WebView::loadFinished,
            this, &BrowserWindow::onWebViewLoadFinished);
    connect(webView_, &WebView::titleChanged,
            this, &BrowserWindow::onWebViewTitleChanged);
    connect(webView_, &WebView::urlChanged,
            this, &BrowserWindow::onWebViewUrlChanged);
    connect(webView_, &WebView::loadError,
            this, &BrowserWindow::onWebViewLoadError);

    qDebug() << "[BrowserWindow] 시그널/슬롯 연결 완료";
}

// WebView 이벤트 핸들러 구현
void BrowserWindow::onWebViewLoadStarted() {
    qDebug() << "[BrowserWindow] 페이지 로딩 시작";
    // TODO: F-05에서 로딩 인디케이터 표시
}

void BrowserWindow::onWebViewLoadProgress(int progress) {
    qDebug() << "[BrowserWindow] 로딩 진행률:" << progress << "%";
    // TODO: F-05에서 프로그레스바 업데이트
}

void BrowserWindow::onWebViewLoadFinished(bool success) {
    qDebug() << "[BrowserWindow] 페이지 로딩 완료:" << (success ? "성공" : "실패");
    if (success) {
        qDebug() << "[BrowserWindow] 페이지 제목:" << webView_->title();
        qDebug() << "[BrowserWindow] 페이지 URL:" << webView_->url().toString();
    }
}

void BrowserWindow::onWebViewTitleChanged(const QString &title) {
    qDebug() << "[BrowserWindow] 페이지 제목 변경:" << title;
    // TODO: F-06에서 탭 제목 업데이트
}

void BrowserWindow::onWebViewUrlChanged(const QUrl &url) {
    qDebug() << "[BrowserWindow] URL 변경:" << url.toString();
    // TODO: F-03에서 URLBar 업데이트
    // TODO: F-08에서 히스토리 저장
}

void BrowserWindow::onWebViewLoadError(int errorCode, const QString &errorMessage, const QUrl &url) {
    qDebug() << "[BrowserWindow] 로딩 에러:"
             << "Code=" << errorCode
             << ", Message=" << errorMessage
             << ", URL=" << url.toString();
    // TODO: F-13에서 에러 페이지 표시
}
```

## 6. 시퀀스 흐름 설계

### 주요 시나리오: 정상 페이지 로딩
```
사용자       BrowserWindow      WebView         QWebEngineView    Chromium
  │                │                │                   │               │
  │  앱 실행        │                │                   │               │
  │───────────────▶│                │                   │               │
  │                │  setupUI()     │                   │               │
  │                │  생성           │                   │               │
  │                │───────────────▶│  new              │               │
  │                │                │──────────────────▶│               │
  │                │                │                   │               │
  │                │  load(url)     │                   │               │
  │                │───────────────▶│  setUrl(url)      │               │
  │                │                │──────────────────▶│  HTTP GET     │
  │                │                │                   │──────────────▶│
  │                │                │  loadStarted()    │               │
  │                │  onLoadStarted │◀──────────────────│               │
  │                │◀───────────────│  emit signal      │               │
  │                │  [log] 로딩 시작│                   │               │
  │                │                │  start timer(30s) │               │
  │                │                │                   │  HTML 응답    │
  │                │                │                   │◀──────────────│
  │                │                │  loadProgress(25) │               │
  │                │  onProgress(25)│◀──────────────────│               │
  │                │◀───────────────│                   │               │
  │                │                │  loadProgress(50) │               │
  │                │  onProgress(50)│◀──────────────────│               │
  │                │◀───────────────│                   │               │
  │                │                │  loadProgress(100)│               │
  │                │  onProgress(100)│◀─────────────────│               │
  │                │◀───────────────│                   │               │
  │                │                │  loadFinished(true)│              │
  │                │  onLoadFinished│◀──────────────────│               │
  │                │◀───────────────│  stop timer       │               │
  │                │  [log] 성공    │  emit signal      │               │
  │                │                │                   │               │
  │  화면에 웹 콘텐츠 표시            │                   │               │
  │◀───────────────────────────────────────────────────────────────────│
```

### 에러 시나리오 1: 네트워크 에러 (404)
```
BrowserWindow      WebView         QWebEngineView
  │                │                   │
  │  load(url)     │                   │
  │───────────────▶│  setUrl(url)      │
  │                │──────────────────▶│  HTTP GET
  │                │                   │─────────────▶
  │                │                   │  404 Not Found
  │                │                   │◀─────────────
  │                │  loadFinished(false)
  │  onLoadFinished│◀──────────────────│
  │◀───────────────│  emit loadError(-1, "페이지를 로드할 수 없습니다", url)
  │  onLoadError   │                   │
  │◀───────────────│                   │
  │  [log] 에러    │                   │
  │  Code=-1       │                   │
```

### 에러 시나리오 2: 타임아웃 (30초 초과)
```
BrowserWindow      WebView         QTimer          QWebEngineView
  │                │                   │                   │
  │  load(url)     │                   │                   │
  │───────────────▶│  start timer(30s) │                   │
  │                │──────────────────▶│                   │
  │                │                   │  ... 30초 경과 ... │
  │                │  onLoadTimeout()  │                   │
  │                │◀──────────────────│                   │
  │                │  stop()           │                   │
  │                │──────────────────────────────────────▶│
  │                │  emit loadError(-2, "타임아웃", url)   │
  │  onLoadError   │                   │                   │
  │◀───────────────│                   │                   │
  │  [log] 타임아웃│                   │                   │
```

## 7. CMakeLists.txt 업데이트

### Qt WebEngine 모듈 추가
```cmake
# CMakeLists.txt에 추가

# Qt 패키지 찾기 (WebEngineWidgets 모듈 추가)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets WebEngineWidgets)

# Qt 라이브러리 링크 (WebEngineWidgets 추가)
target_link_libraries(webosbrowser
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::WebEngineWidgets
)
```

## 8. 리모컨 입력 처리 준비

### 키 이벤트 매핑 (F-04, F-12에서 구현)
| 리모컨 키 | Qt Key Event | WebView 동작 | BrowserWindow 동작 |
|-----------|-------------|-------------|-------------------|
| 십자키 (↑↓←→) | Qt::Key_Up/Down/Left/Right | 웹 페이지 내 포커스 이동, 스크롤 | 포커스가 WebView에 없으면 UI 간 이동 |
| 선택 (OK) | Qt::Key_Enter / Qt::Key_Return | 링크 클릭, 버튼 활성화 | — |
| 뒤로 (Back) | Qt::Key_Backspace | 웹 페이지 뒤로 가기 (history.back) | 포커스 이탈 → NavBar로 |
| 홈 (Home) | Qt::Key_Home | — | 홈 화면 이동 (F-15) |

### QKeyEvent 핸들링 (BrowserWindow에서 구현 예정)
```cpp
// BrowserWindow.cpp에 추가 (F-04에서 구현)
void BrowserWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Backspace:
        // 뒤로 버튼: WebView의 뒤로 가기 또는 포커스 이탈
        if (webView_->hasFocus() && webView_->canGoBack()) {
            webView_->goBack();
        } else {
            // 포커스를 NavBar로 이동 (F-04에서 구현)
        }
        break;
    case Qt::Key_Home:
        // 홈 버튼: 홈 화면 이동 (F-15에서 구현)
        break;
    default:
        QMainWindow::keyPressEvent(event);  // 기본 처리
    }
}
```

## 9. 메모리 관리 전략

### RAII 원칙
- **WebView 소멸 시**:
  1. `~WebView()` 호출
  2. `d_ptr`의 QScopedPointer 소멸자 자동 호출
  3. `WebViewPrivate` 소멸
  4. `QWebEngineView`는 QObject 부모-자식 관계로 Qt가 자동 삭제
  5. `QTimer`도 QObject 부모-자식 관계로 자동 삭제

### 타임아웃 타이머 정리
- 로딩 완료 또는 중단 시 `timeoutTimer->stop()` 호출
- 소멸자에서 타이머 활성 상태 확인 후 정지

### QWebEngineView 리소스 해제
- `stop()` 호출로 네트워크 요청 취소
- `setUrl(QUrl())` 또는 빈 페이지 로드로 메모리 해제 (F-06 탭 전환 시 고려)

## 10. 성능 및 메모리 모니터링

### 로딩 시간 측정
```cpp
// WebViewPrivate에 추가
qint64 loadStartTime = QDateTime::currentMSecsSinceEpoch();

// onLoadFinished에서 계산
qint64 duration = QDateTime::currentMSecsSinceEpoch() - d->loadStartTime;
qDebug() << "[WebView] 로딩 소요 시간:" << duration << "ms";
```

### 메모리 사용량 로깅 (개발 모드)
```cpp
// utils/Logger.cpp에 추가 (F-01에서 생성 예정)
#ifdef Q_OS_LINUX  // webOS는 Linux 기반
#include <sys/resource.h>

void logMemoryUsage(const QString &context) {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    qDebug() << "[Memory]" << context << ":"
             << (usage.ru_maxrss / 1024) << "MB";  // KB → MB
}
#endif
```

## 11. 영향 범위 분석

### 수정 필요한 기존 파일
- **src/browser/BrowserWindow.h**:
  - WebView 멤버 변수 추가
  - WebView 이벤트 핸들러 슬롯 선언
- **src/browser/BrowserWindow.cpp**:
  - WebView 인스턴스 생성 및 레이아웃 배치
  - 시그널/슬롯 연결
  - 초기 URL 로드
- **CMakeLists.txt**:
  - Qt5::WebEngineWidgets 모듈 추가

### 새로 생성할 파일
- **src/browser/WebView.h**: 공개 인터페이스 (PIMPL 패턴)
- **src/browser/WebView.cpp**: 구현 + WebViewPrivate 클래스

### 영향 받는 기존 기능
없음 (F-02가 첫 번째 실질적 기능 구현)

### 후속 기능에 제공하는 API
- **F-03 (URL 입력 UI)**: `WebView::load(QUrl)` 호출
- **F-04 (페이지 탐색)**: `goBack()`, `goForward()`, `canGoBack()`, `canGoForward()` 사용
- **F-05 (로딩 인디케이터)**: `loadProgress(int)` 시그널 구독
- **F-06 (탭 관리)**: 탭별 WebView 인스턴스 생성
- **F-08 (히스토리)**: `urlChanged(QUrl)` 시그널 구독하여 히스토리 저장
- **F-13 (에러 처리)**: `loadError(...)` 시그널 구독하여 에러 페이지 표시

## 12. 기술적 주의사항

### Qt WebEngineView 제약사항
- **프로세스 모델**: Chromium 멀티 프로세스 아키텍처 (렌더러 프로세스 별도 실행)
- **메모리 사용**: 단일 WebView도 100~200MB 메모리 사용 (Chromium 엔진)
- **이벤트 루프**: QWebEngineView는 Qt 이벤트 루프가 실행 중일 때만 동작
- **쿠키/캐시**: 기본적으로 `~/.local/share/webosbrowser/` 경로에 저장 (QWebEngineProfile로 제어)

### webOS 플랫폼 제약
- **Chromium 버전**: webOS 6.x는 Chromium 79 기반 (비교적 최신)
- **하드웨어 가속**: GPU 가속 활성화 필요 (webOS 설정 확인)
- **메모리 제한**: 프로젝터 하드웨어 제약 (1~2GB RAM) → 탭 개수 제한 필요 (F-06)

### CORS 및 보안 정책
- **Same-Origin Policy**: 웹 표준 CORS 정책 준수
- **Mixed Content**: HTTPS 페이지 내 HTTP 리소스 차단 (Chromium 기본)
- **SSL/TLS**: 인증서 오류 감지 (F-13에서 세밀한 에러 처리)

### 리모컴 포커스 관리
- **WebView 내부**: 웹 페이지의 tabindex, accesskey로 포커스 관리
- **WebView 외부**: BrowserWindow에서 QKeyEvent로 URLBar↔WebView↔NavBar 포커스 전환 (F-04)

## 13. 테스트 전략

### 단위 테스트 (F-02 범위 외, 추후 구현)
- WebView 클래스의 공개 API 테스트 (load, reload, goBack 등)
- 시그널 emit 검증 (QSignalSpy 사용)

### 로컬 테스트 (개발 환경)
1. CMake 빌드 성공 확인
2. 앱 실행 → Google 홈페이지 자동 로드
3. 콘솔 로그 확인 (로딩 시작, 진행률, 완료)
4. 웹 콘텐츠 표시 확인

### 실제 디바이스 테스트
1. IPK 패키지 생성
2. 프로젝터 설치 및 실행
3. 리모컨으로 웹 페이지 조작 확인 (스크롤, 링크 클릭)
4. 주요 사이트 렌더링 테스트 (YouTube, Naver, Wikipedia)

### 성능 테스트
- Google 홈페이지 로딩 시간: 2초 이내
- 일반 페이지 (Naver 홈): 5초 이내
- 메모리 사용량: 단일 WebView 200MB 이하

## 14. 구현 순서

### Phase 1: WebView 클래스 스켈레톤 (1~2시간)
1. `src/browser/WebView.h` 생성 (공개 인터페이스 + enum)
2. `src/browser/WebView.cpp` 생성 (WebViewPrivate 스켈레톤)
3. 생성자/소멸자 구현 (qDebug 로그)
4. CMakeLists.txt에 WebEngineWidgets 추가
5. 빌드 성공 확인

### Phase 2: QWebEngineView 통합 (2~3시간)
1. WebViewPrivate에 QWebEngineView 멤버 추가
2. 레이아웃 설정 (QVBoxLayout)
3. `load()`, `reload()`, `stop()` 구현
4. 시그널/슬롯 연결 (loadStarted, loadProgress, loadFinished)
5. 기본 로깅 추가

### Phase 3: 상태 관리 및 타임아웃 (2~3시간)
1. LoadingState enum 구현
2. QTimer로 30초 타임아웃 구현
3. 상태 전이 로직 (Idle → Loading → Loaded/Error)
4. `onLoadTimeout` 슬롯 구현

### Phase 4: 네비게이션 API (1~2시간)
1. `canGoBack()`, `canGoForward()` 구현
2. `goBack()`, `goForward()` 구현
3. `url()`, `title()`, `state()` 구현

### Phase 5: 에러 처리 (1~2시간)
1. `loadError` 시그널 정의
2. `onLoadFinished(false)` 시 에러 시그널 emit
3. 타임아웃 에러 처리

### Phase 6: BrowserWindow 통합 (2~3시간)
1. BrowserWindow.h에 WebView 멤버 추가
2. BrowserWindow.cpp에서 WebView 생성 및 레이아웃 배치
3. 시그널/슬롯 연결 (6개 이벤트 핸들러)
4. 초기 URL 로드 (https://www.google.com)

### Phase 7: 로컬 테스트 (1~2시간)
1. CMake 빌드 및 실행
2. 콘솔 로그 확인 (로딩 이벤트)
3. Google 홈페이지 렌더링 확인
4. 여러 사이트 테스트 (YouTube, Naver)

### Phase 8: 실제 디바이스 테스트 (2~3시간)
1. IPK 생성 및 프로젝터 설치
2. 리모컨으로 웹 페이지 조작 확인
3. 주요 사이트 렌더링 성공률 확인 (5개 중 4개 이상)
4. 성능 측정 (로딩 시간, 메모리 사용량)

### Phase 9: 문서화 (1~2시간)
1. WebView.h에 Doxygen 주석 추가
2. 코드 리뷰 및 주석 보완
3. README.md 업데이트

**총 예상 소요 시간**: 13~20시간 (2~3일)

## 15. 리스크 및 완화 전략

| 리스크 | 확률 | 영향도 | 완화 전략 |
|--------|------|--------|-----------|
| Qt WebEngineWidgets 미지원 (webOS) | 낮음 | 치명적 | webOS Native SDK 문서 확인, Qt for webOS 공식 지원 확인. 최악의 경우 Qt WebView로 fallback |
| Chromium 메모리 과다 사용 | 중간 | 높음 | QWebEngineProfile로 캐시 크기 제한, 탭 개수 제한 (F-06) |
| 리모컨 포커스 충돌 | 높음 | 중간 | BrowserWindow에서 키 이벤트 중앙 관리, 웹 페이지는 자체 포커스 사용 |
| 주요 사이트 렌더링 실패 (DRM) | 중간 | 중간 | Netflix DRM은 Widevine 지원 확인 필요, 불가능한 경우 사용자 안내 |
| 타임아웃 30초가 너무 길거나 짧음 | 낮음 | 낮음 | 실제 테스트 후 조정 (20~40초 범위) |
| QWebEngineView 프로세스 크래시 | 중간 | 높음 | F-13에서 renderProcessTerminated 시그널 처리 |

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 (Native App 관점) | Web App PoC의 iframe 설계를 Qt/C++ 기반으로 전환 |
| 2026-02-14 | Qt WebEngineView 선택 | webOS Native SDK의 Qt 지원 확인 및 크로스플랫폼 호환성 |
