# 에러 처리 — 기술 설계서

## 1. 참조
- 요구사항 분석서: docs/specs/error-handling/requirements.md

---

## 2. 아키텍처 결정

### 결정 1: ErrorPage 위젯 위치 및 표시 방식
- **선택지**:
  - A) QStackedLayout으로 WebView 위에 전환 표시
  - B) 투명 QWidget 오버레이를 WebView 위에 배치
  - C) QDialog/QMessageBox 팝업으로 표시
- **결정**: A) QStackedLayout으로 WebView 위에 전환 표시
- **근거**:
  - QStackedLayout은 Qt의 표준 위젯 전환 메커니즘
  - setCurrentWidget()으로 간단한 전환 가능
  - Z-order 관리 불필요 (자동 관리)
  - BookmarkPanel과 달리 전체 화면을 덮어야 하므로 오버레이보다 전환이 적합
  - QDialog는 별도 윈도우 생성으로 리모컨 포커스 관리 복잡
- **트레이드오프**:
  - 포기: WebView와 ErrorPage 동시 표시 불가 (덮어쓰기 방식)
  - 얻음: 단순한 구조, 명확한 포커스 관리, 적은 메모리 사용

### 결정 2: 에러 타입 구조체 vs enum
- **선택지**:
  - A) enum class ErrorType + ErrorInfo 구조체 분리
  - B) enum만 사용하고 메시지는 showError()에서 생성
  - C) ErrorInfo 구조체만 사용 (errorCode로 구분)
- **결정**: A) enum class ErrorType + ErrorInfo 구조체 분리
- **근거**:
  - 타입 안정성: enum class는 강타입 열거형 (암시적 형변환 방지)
  - 확장성: 향후 에러 타입 추가 시 enum에만 추가
  - 가독성: ErrorType::NetworkError가 정수 코드보다 명확
  - ErrorInfo는 에러 발생 시점의 상세 정보 (URL, 타임스탬프) 보관
- **트레이드오프**:
  - 포기: 단순한 정수 코드 사용
  - 얻음: 타입 안정성, 코드 가독성, 컴파일 타임 에러 검출

### 결정 3: HTTP 상태 코드 감지 방식
- **선택지**:
  - A) Qt WebEngineView의 제한적 정보만 사용 (현재)
  - B) QWebEngineUrlRequestInterceptor로 HTTP 헤더 감지
  - C) JavaScript injection으로 서버 응답 파싱
- **결정**: A) Qt WebEngineView의 제한적 정보만 사용
- **근거**:
  - Qt WebEngineView는 loadFinished(false)와 에러 문자열만 제공
  - HTTP 상태 코드(404, 500)는 페이지가 정상 렌더링되므로 에러로 감지 안 됨
  - QWebEngineUrlRequestInterceptor는 요청 단계만 가로채기 가능 (응답 불가)
  - JavaScript injection은 보안 정책(CSP) 위반 가능 및 복잡도 증가
  - 현 단계에서는 일반적인 네트워크 에러/타임아웃만 처리
- **트레이드오프**:
  - 포기: 정확한 HTTP 상태 코드별 분류
  - 얻음: 단순한 구조, Qt 표준 API 사용, 안정성

### 결정 4: 애니메이션 구현 방식
- **선택지**:
  - A) QPropertyAnimation (Qt Animation Framework)
  - B) QGraphicsOpacityEffect + QTimer
  - C) 애니메이션 없이 즉시 표시/숨김
- **결정**: A) QPropertyAnimation
- **근거**:
  - Qt Animation Framework의 표준 방식
  - windowOpacity 속성 애니메이션으로 페이드인/아웃 구현
  - 하드웨어 가속 지원 (부드러운 전환)
  - DeleteWhenStopped 옵션으로 자동 메모리 관리
- **트레이드오프**:
  - 포기: 즉시 표시의 단순함
  - 얻음: 부드러운 UX, 사용자 경험 개선

### 결정 5: 메모리 관리 전략
- **선택지**:
  - A) ErrorPage를 BrowserWindow 생성자에서 미리 생성 (사전 할당)
  - B) 에러 발생 시 동적 생성, 숨김 시 삭제
  - C) QScopedPointer로 지연 초기화
- **결정**: A) ErrorPage를 BrowserWindow 생성자에서 미리 생성
- **근거**:
  - 에러는 빈번히 발생 가능 (네트워크 불안정)
  - 동적 생성/삭제는 메모리 파편화 및 성능 저하
  - ErrorPage는 가벼운 위젯 (QLabel, QPushButton만)
  - Qt parent-child 관계로 자동 메모리 해제 (BrowserWindow 소멸 시)
- **트레이드오프**:
  - 포기: 초기 메모리 절약
  - 얻음: 빠른 에러 화면 표시, 안정적 메모리 관리

---

## 3. 클래스 설계

### ErrorPage 클래스

#### 헤더 (src/ui/ErrorPage.h)

```cpp
#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QUrl>

namespace webosbrowser {

/**
 * @enum ErrorType
 * @brief 에러 타입 열거형
 */
enum class ErrorType {
    NetworkError = -1,   ///< 네트워크 연결 실패 (DNS, 연결 거부 등)
    Timeout = -2,        ///< 타임아웃 (30초 초과)
    CorsError = -3,      ///< CORS 정책 위반
    UnknownError = -99   ///< 알 수 없는 에러
};

/**
 * @struct ErrorInfo
 * @brief 에러 상세 정보
 */
struct ErrorInfo {
    ErrorType type;          ///< 에러 타입
    QString errorMessage;    ///< 사용자 표시용 메시지
    QUrl url;                ///< 실패한 URL
    QDateTime timestamp;     ///< 에러 발생 시각

    ErrorInfo()
        : type(ErrorType::UnknownError)
        , timestamp(QDateTime::currentDateTime())
    {}
};

/**
 * @class ErrorPage
 * @brief 에러 화면 위젯 (WebView 로딩 실패 시 표시)
 *
 * QWidget 기반, WebView와 QStackedLayout으로 전환 표시.
 * 리모컨 포커스 관리 및 재시도/홈 이동 기능 제공.
 */
class ErrorPage : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param parent 부모 위젯 (BrowserWindow)
     */
    explicit ErrorPage(QWidget *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~ErrorPage() override;

    // 복사 생성자 및 대입 연산자 삭제
    ErrorPage(const ErrorPage&) = delete;
    ErrorPage& operator=(const ErrorPage&) = delete;

    /**
     * @brief 에러 화면 표시
     * @param type 에러 타입
     * @param errorMessage 에러 메시지
     * @param url 실패한 URL
     */
    void showError(ErrorType type, const QString &errorMessage, const QUrl &url);

    /**
     * @brief 에러 정보로 화면 표시 (오버로드)
     * @param errorInfo 에러 정보 구조체
     */
    void showError(const ErrorInfo &errorInfo);

signals:
    /**
     * @brief 재시도 버튼 클릭 시그널
     */
    void retryRequested();

    /**
     * @brief 홈으로 이동 버튼 클릭 시그널
     */
    void homeRequested();

protected:
    /**
     * @brief 키 이벤트 핸들러 (리모컨 입력)
     * @param event 키 이벤트
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 표시 이벤트 핸들러 (페이드인 애니메이션)
     * @param event 표시 이벤트
     */
    void showEvent(QShowEvent *event) override;

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 스타일시트 적용 (QSS)
     */
    void applyStyles();

    /**
     * @brief 에러 타입에 따른 아이콘 경로 반환
     * @param type 에러 타입
     * @return 아이콘 파일 경로
     */
    QString getErrorIcon(ErrorType type) const;

    /**
     * @brief URL 문자열 truncate (최대 50자)
     * @param url URL
     * @return truncate된 URL 문자열
     */
    QString truncateUrl(const QUrl &url) const;

    /**
     * @brief 페이드인 애니메이션 시작
     */
    void startFadeInAnimation();

private:
    // UI 컴포넌트
    QVBoxLayout *mainLayout_;       ///< 메인 레이아웃
    QLabel *iconLabel_;             ///< 에러 아이콘
    QLabel *titleLabel_;            ///< 에러 제목
    QLabel *messageLabel_;          ///< 에러 메시지
    QLabel *urlLabel_;              ///< 실패한 URL
    QPushButton *retryButton_;      ///< 재시도 버튼
    QPushButton *homeButton_;       ///< 홈으로 이동 버튼

    // 현재 에러 정보
    ErrorInfo currentError_;        ///< 현재 표시 중인 에러 정보
};

} // namespace webosbrowser
```

#### 구현 (src/ui/ErrorPage.cpp)

**주요 메서드 설계**:

1. **setupUI()**
   - QVBoxLayout으로 수직 배치
   - 아이콘: QLabel (80x80px, QPixmap)
   - 제목: QLabel (48px, Bold)
   - 메시지: QLabel (28px, WordWrap)
   - URL: QLabel (22px, 회색)
   - 버튼: QHBoxLayout (재시도, 홈)
   - 중앙 정렬: layout->setAlignment(Qt::AlignCenter)

2. **applyStyles()**
   - 배경: `rgba(0, 0, 0, 230)` (반투명 검은색)
   - 텍스트: 흰색
   - 버튼: 파란색 배경, 흰색 텍스트, 최소 크기 200x60px
   - 포커스: `QPushButton:focus { border: 4px solid #FFD700; }`
   - QSS 스타일시트로 일괄 적용

3. **showError(ErrorType, QString, QUrl)**
   - ErrorInfo 구조체 생성
   - 에러 타입별 제목 설정 (switch-case)
   - URL truncate (50자)
   - UI 업데이트 (setText)
   - retryButton에 자동 포커스 (`setFocus(Qt::OtherFocusReason)`)
   - show() 호출

4. **keyPressEvent(QKeyEvent*)**
   - Qt 탭 오더 시스템이 좌/우 방향키 처리
   - Back 키 무시 (`event->ignore()`)
   - 에러 화면에서 벗어나지 못하도록 강제

5. **showEvent(QShowEvent*)**
   - QPropertyAnimation 생성
   - windowOpacity: 0 → 1, 300ms
   - EasingCurve: QEasingCurve::OutCubic
   - DeleteWhenStopped

6. **getErrorIcon(ErrorType)**
   - 에러 타입별 아이콘 파일 경로 반환
   - 기본값: `":/icons/error.png"`
   - 향후 확장: 네트워크 에러, 타임아웃 전용 아이콘

---

## 4. BrowserWindow 통합 설계

### 레이아웃 구조

```
BrowserWindow (QMainWindow)
├─ centralWidget (QWidget)
│  ├─ mainLayout (QVBoxLayout)
│  │  ├─ URLBar
│  │  ├─ NavigationBar
│  │  ├─ LoadingIndicator
│  │  ├─ contentWidget (QWidget) ← 새로 추가
│  │  │  └─ stackedLayout (QStackedLayout) ← 새로 추가
│  │  │     ├─ webView (기본 표시)
│  │  │     └─ errorPage (에러 시 전환)
│  ├─ bookmarkPanel (오버레이, 우측 고정)
│  └─ historyPanel (오버레이, 우측 고정)
```

### BrowserWindow.h 수정 사항

```cpp
#include <QStackedLayout>
#include "../ui/ErrorPage.h"

class BrowserWindow : public QMainWindow {
    Q_OBJECT

private:
    QWidget *contentWidget_;         // 새로 추가: WebView/ErrorPage 컨테이너
    QStackedLayout *stackedLayout_;  // 새로 추가: 전환 레이아웃
    ErrorPage *errorPage_;           // 새로 추가: 에러 화면

private slots:
    /**
     * @brief WebView 로딩 에러 핸들러
     * @param errorString 에러 메시지
     */
    void onLoadError(const QString &errorString);

    /**
     * @brief WebView 타임아웃 핸들러
     */
    void onLoadTimeout();

    /**
     * @brief ErrorPage 재시도 핸들러
     */
    void onRetryRequested();

    /**
     * @brief ErrorPage 홈 이동 핸들러
     */
    void onHomeRequested();
};
```

### BrowserWindow.cpp 수정 사항

**생성자 초기화 리스트 추가**:
```cpp
BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent)
    , contentWidget_(new QWidget(centralWidget_))  // 추가
    , stackedLayout_(new QStackedLayout(contentWidget_))  // 추가
    , errorPage_(new ErrorPage(contentWidget_))  // 추가
    , webView_(new WebView(contentWidget_))
    // ... 나머지 동일
```

**setupUI() 수정**:
```cpp
void BrowserWindow::setupUI() {
    // ... URLBar, NavigationBar, LoadingIndicator 추가 (기존 동일)

    // WebView/ErrorPage 컨테이너 설정 (새로 추가)
    stackedLayout_->setStackingMode(QStackedLayout::StackOne);
    stackedLayout_->addWidget(webView_);
    stackedLayout_->addWidget(errorPage_);
    stackedLayout_->setCurrentWidget(webView_);  // 기본값: WebView 표시

    // 컨테이너를 메인 레이아웃에 추가
    mainLayout_->addWidget(contentWidget_, 1);  // stretch=1

    // ... 나머지 동일
}
```

**setupConnections() 추가**:
```cpp
void BrowserWindow::setupConnections() {
    // ... 기존 연결 유지

    // WebView 에러 시그널 연결 (기존 로그만 찍던 것 수정)
    connect(webView_, &WebView::loadError, this, &BrowserWindow::onLoadError);
    connect(webView_, &WebView::loadTimeout, this, &BrowserWindow::onLoadTimeout);

    // ErrorPage 시그널 연결
    connect(errorPage_, &ErrorPage::retryRequested, this, &BrowserWindow::onRetryRequested);
    connect(errorPage_, &ErrorPage::homeRequested, this, &BrowserWindow::onHomeRequested);

    // WebView 로딩 성공 시 WebView로 전환 (에러 화면 숨김)
    connect(webView_, &WebView::loadFinished, this, [this](bool success) {
        if (success) {
            stackedLayout_->setCurrentWidget(webView_);
        }
        // 실패 시는 onLoadError에서 처리
    });
}
```

**슬롯 구현**:
```cpp
void BrowserWindow::onLoadError(const QString &errorString) {
    // ErrorInfo 생성
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::NetworkError;  // 기본값 (Qt가 타입 구분 안 해줌)
    errorInfo.errorMessage = errorString;
    errorInfo.url = webView_->url();
    errorInfo.timestamp = QDateTime::currentDateTime();

    // ErrorPage 표시
    errorPage_->showError(errorInfo);
    stackedLayout_->setCurrentWidget(errorPage_);

    // 로그 기록
    qCritical() << "Page load error:"
                << "message=" << errorString
                << "url=" << errorInfo.url.toString();
}

void BrowserWindow::onLoadTimeout() {
    ErrorInfo errorInfo;
    errorInfo.type = ErrorType::Timeout;
    errorInfo.errorMessage = "페이지 로딩 시간이 초과되었습니다 (30초)";
    errorInfo.url = webView_->url();
    errorInfo.timestamp = QDateTime::currentDateTime();

    errorPage_->showError(errorInfo);
    stackedLayout_->setCurrentWidget(errorPage_);

    qCritical() << "Page load timeout:" << errorInfo.url.toString();
}

void BrowserWindow::onRetryRequested() {
    // ErrorPage 페이드아웃 (QPropertyAnimation)
    QPropertyAnimation *fadeOut = new QPropertyAnimation(errorPage_, "windowOpacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    // 애니메이션 완료 후 WebView로 전환 및 재시도
    connect(fadeOut, &QPropertyAnimation::finished, this, [this]() {
        stackedLayout_->setCurrentWidget(webView_);
        webView_->reload();
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

void BrowserWindow::onHomeRequested() {
    // 페이드아웃 (onRetryRequested와 동일)
    QPropertyAnimation *fadeOut = new QPropertyAnimation(errorPage_, "windowOpacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    connect(fadeOut, &QPropertyAnimation::finished, this, [this]() {
        stackedLayout_->setCurrentWidget(webView_);
        // 홈 URL은 하드코딩 (향후 SettingsService 연동)
        webView_->load(QUrl("https://www.google.com"));
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}
```

---

## 5. 시그널/슬롯 연결 다이어그램

```
┌─────────────────────────────────────────────────────────────┐
│                       BrowserWindow                          │
│                                                              │
│  ┌──────────────┐        ┌──────────────┐                  │
│  │   WebView    │        │  ErrorPage   │                  │
│  │              │        │              │                  │
│  │  loadError() ├───────▶│              │                  │
│  │  loadTimeout()───────▶│ showError()  │                  │
│  │              │        │              │                  │
│  │              │◀───────┤ retryRequested()                │
│  │  reload()    │        │              │                  │
│  │              │◀───────┤ homeRequested()                 │
│  │  load(home)  │        │              │                  │
│  │              │        │              │                  │
│  │  loadFinished(true)──▶│ (WebView로 전환)                │
│  └──────────────┘        └──────────────┘                  │
│         │                       │                           │
│         │  QStackedLayout       │                           │
│         │   setCurrentWidget()  │                           │
│         └───────────┬───────────┘                           │
│                     │                                       │
│            ┌────────▼────────┐                              │
│            │  contentWidget   │                              │
│            │  (QWidget)       │                              │
│            └──────────────────┘                              │
└─────────────────────────────────────────────────────────────┘

시그널/슬롯 흐름:
1. 에러 발생: WebView::loadError() → BrowserWindow::onLoadError() → ErrorPage::showError() → QStackedLayout::setCurrentWidget(errorPage_)
2. 타임아웃: WebView::loadTimeout() → BrowserWindow::onLoadTimeout() → ErrorPage::showError() → 전환
3. 재시도: ErrorPage::retryRequested() → BrowserWindow::onRetryRequested() → 페이드아웃 → WebView::reload() → 전환
4. 홈 이동: ErrorPage::homeRequested() → BrowserWindow::onHomeRequested() → 페이드아웃 → WebView::load(home) → 전환
5. 로딩 성공: WebView::loadFinished(true) → lambda → QStackedLayout::setCurrentWidget(webView_)
```

---

## 6. UI 레이아웃 구조 (ErrorPage)

### 와이어프레임

```
┌────────────────────────────────────────────────────────┐
│                                                        │
│                   (반투명 검은 배경)                   │
│                                                        │
│                                                        │
│                    [에러 아이콘]                       │
│                      80x80px                           │
│                                                        │
│               에러 제목 (48px, Bold)                   │
│                                                        │
│         에러 메시지 (28px, 줄바꿈 지원)                │
│                                                        │
│          실패한 URL (22px, 회색, 최대 50자)            │
│                                                        │
│                                                        │
│       ┌─────────────┐       ┌─────────────┐           │
│       │  재시도     │       │  홈으로     │           │
│       │  버튼       │       │  이동       │           │
│       │ 200x60px    │       │ 200x60px    │           │
│       └─────────────┘       └─────────────┘           │
│                                                        │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### QSS 스타일 정의

```cpp
void ErrorPage::applyStyles() {
    setStyleSheet(R"(
        ErrorPage {
            background-color: rgba(0, 0, 0, 230);
        }
    )");

    titleLabel_->setStyleSheet(R"(
        QLabel {
            font-size: 48px;
            font-weight: bold;
            color: #FFFFFF;
        }
    )");

    messageLabel_->setStyleSheet(R"(
        QLabel {
            font-size: 28px;
            color: #FFFFFF;
        }
    )");

    urlLabel_->setStyleSheet(R"(
        QLabel {
            font-size: 22px;
            color: #999999;
        }
    )");

    // 버튼 공통 스타일
    QString buttonStyle = R"(
        QPushButton {
            background-color: #1E90FF;
            color: #FFFFFF;
            font-size: 24px;
            border: 2px solid #1E90FF;
            border-radius: 8px;
            padding: 10px 20px;
            min-width: 200px;
            min-height: 60px;
        }
        QPushButton:hover {
            background-color: #4169E1;
        }
        QPushButton:pressed {
            background-color: #104E8B;
        }
        QPushButton:focus {
            border: 4px solid #FFD700;
        }
    )";

    retryButton_->setStyleSheet(buttonStyle);
    homeButton_->setStyleSheet(buttonStyle);

    // 포커스 정책 설정 (리모컨 포커스 필수)
    retryButton_->setFocusPolicy(Qt::StrongFocus);
    homeButton_->setFocusPolicy(Qt::StrongFocus);

    // 탭 오더 설정 (좌우 방향키 전환)
    setTabOrder(retryButton_, homeButton_);
    setTabOrder(homeButton_, retryButton_);  // 순환
}
```

---

## 7. 에러 감지 및 처리 플로우

### 에러 감지 시퀀스

```
사용자 → URLBar → WebView → Qt WebEngineView → 네트워크
                                │
                    ┌───────────┴───────────┐
                    │                       │
                 (성공)                  (실패)
                    │                       │
                    ▼                       ▼
            loadFinished(true)     loadFinished(false)
                    │                       │
                    │                   loadError()
                    │                   emit signal
                    │                       │
                    ▼                       ▼
              WebView 표시        BrowserWindow::onLoadError()
                                            │
                                            ▼
                                  ErrorPage::showError()
                                            │
                                            ▼
                              QStackedLayout::setCurrentWidget(errorPage_)
                                            │
                                            ▼
                                    ErrorPage 표시
                                    (페이드인 애니메이션)
```

### 타임아웃 처리 (WebView 내부)

```cpp
// WebView.cpp (기존 구현)
void WebViewPrivate::onLoadStarted() {
    // 타임아웃 타이머 시작 (30초)
    timeoutTimer_->start(30000);
}

void WebViewPrivate::onLoadFinished(bool ok) {
    // 타임아웃 타이머 정지
    timeoutTimer_->stop();

    if (!ok) {
        emit q->loadError("페이지 로딩 실패");
    }
}

void WebViewPrivate::onTimeout() {
    // 30초 초과
    webEngineView_->stop();
    emit q->loadTimeout();
}
```

### 재시도 플로우

```
ErrorPage (표시 중)
    │
    │ 사용자 리모컨 Enter 키
    ▼
retryButton_->clicked()
    │
    ▼
ErrorPage::retryRequested() signal
    │
    ▼
BrowserWindow::onRetryRequested()
    │
    ├─ QPropertyAnimation (페이드아웃 200ms)
    │
    └─ (애니메이션 완료 후)
       │
       ├─ QStackedLayout::setCurrentWidget(webView_)
       │
       └─ webView_->reload()
              │
              ▼
       WebView::loadStarted()
              │
              ▼
       LoadingIndicator 표시
       타임아웃 타이머 재시작
              │
       ┌─────┴─────┐
       │           │
    (성공)      (실패)
       │           │
       ▼           ▼
   WebView    ErrorPage
   유지       다시 표시
```

---

## 8. 데이터 모델

### 에러 타입별 메시지 맵핑

```cpp
// ErrorPage.cpp
void ErrorPage::showError(ErrorType type, const QString &errorMessage, const QUrl &url) {
    // 에러 정보 저장
    currentError_.type = type;
    currentError_.errorMessage = errorMessage;
    currentError_.url = url;
    currentError_.timestamp = QDateTime::currentDateTime();

    // 에러 타입별 제목 및 아이콘 설정
    QString title;
    QString icon;

    switch (type) {
        case ErrorType::NetworkError:
            title = "네트워크 연결 실패";
            icon = getErrorIcon(ErrorType::NetworkError);
            break;

        case ErrorType::Timeout:
            title = "로딩 시간 초과";
            icon = getErrorIcon(ErrorType::Timeout);
            break;

        case ErrorType::CorsError:
            title = "보안 정책 오류";
            icon = getErrorIcon(ErrorType::CorsError);
            break;

        case ErrorType::UnknownError:
        default:
            title = "페이지 로딩 오류";
            icon = getErrorIcon(ErrorType::UnknownError);
            break;
    }

    // UI 업데이트
    iconLabel_->setPixmap(QPixmap(icon));
    titleLabel_->setText(title);
    messageLabel_->setText(errorMessage);
    urlLabel_->setText(truncateUrl(url));

    // 재시도 버튼에 포커스
    retryButton_->setFocus(Qt::OtherFocusReason);
}

QString ErrorPage::getErrorIcon(ErrorType type) const {
    // 향후 에러 타입별 아이콘 분리 가능
    return ":/icons/error.png";  // 현재는 공통 아이콘
}

QString ErrorPage::truncateUrl(const QUrl &url) const {
    QString urlString = url.toString();
    if (urlString.length() > 50) {
        return urlString.left(50) + "...";
    }
    return urlString;
}
```

### ErrorInfo 구조체 활용

```cpp
// BrowserWindow.cpp
void BrowserWindow::onLoadError(const QString &errorString) {
    // 에러 메시지 파싱으로 타입 추론 (Qt가 세부 타입 안 줌)
    ErrorType type = ErrorType::NetworkError;

    if (errorString.contains("timeout", Qt::CaseInsensitive)) {
        type = ErrorType::Timeout;
    } else if (errorString.contains("cors", Qt::CaseInsensitive) ||
               errorString.contains("cross-origin", Qt::CaseInsensitive)) {
        type = ErrorType::CorsError;
    }

    ErrorInfo errorInfo;
    errorInfo.type = type;
    errorInfo.errorMessage = errorString;
    errorInfo.url = webView_->url();
    errorInfo.timestamp = QDateTime::currentDateTime();

    errorPage_->showError(errorInfo);
    stackedLayout_->setCurrentWidget(errorPage_);

    // 로그 기록
    Logger::error(QString("Page load error: type=%1, message=%2, url=%3")
                  .arg(static_cast<int>(type))
                  .arg(errorString)
                  .arg(errorInfo.url.toString()));
}
```

---

## 9. 메모리 관리 (Qt Parent-Child)

### 객체 생명주기

```
BrowserWindow (QMainWindow)
    │
    ├─ centralWidget_ (자동 삭제)
    │  └─ contentWidget_ (자동 삭제)
    │     ├─ webView_ (자동 삭제)
    │     └─ errorPage_ (자동 삭제)
    │        ├─ mainLayout_ (자동 삭제)
    │        ├─ iconLabel_ (자동 삭제)
    │        ├─ titleLabel_ (자동 삭제)
    │        ├─ messageLabel_ (자동 삭제)
    │        ├─ urlLabel_ (자동 삭제)
    │        ├─ retryButton_ (자동 삭제)
    │        └─ homeButton_ (자동 삭제)
    │
    └─ stackedLayout_ (contentWidget_에 속함, 자동 삭제)
```

### 스마트 포인터 사용 금지 이유

- Qt parent-child 시스템이 자동 메모리 관리
- QWidget을 QScopedPointer로 감싸면 parent 설정 불가
- 명시적 `new`로 생성, parent 전달로 자동 해제
- 예외: PIMPL 패턴의 private 구현 포인터만 QScopedPointer 사용 (WebView 참고)

### 애니메이션 객체 메모리 관리

```cpp
void BrowserWindow::onRetryRequested() {
    QPropertyAnimation *fadeOut = new QPropertyAnimation(errorPage_, "windowOpacity");
    fadeOut->setDuration(200);
    // ...
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);  // 자동 삭제
}
```

- `DeleteWhenStopped` 플래그로 애니메이션 완료 후 자동 delete
- parent 설정 불필요 (임시 객체)

---

## 10. 성능 고려사항

### 에러 감지 속도 최적화

- **목표**: 에러 발생 후 500ms 이내 에러 화면 표시
- **구현**:
  - ErrorPage는 BrowserWindow 생성자에서 미리 생성 (동적 생성 X)
  - QStackedLayout::setCurrentWidget()은 O(1) 시간 복잡도
  - 페이드인 애니메이션 300ms (하드웨어 가속)
  - 총 시간: 시그널 전달(~10ms) + 위젯 전환(~1ms) + 애니메이션(300ms) ≈ 311ms ✅

### 메모리 사용량

- **ErrorPage 위젯 크기**:
  - QLabel 5개 × ~200 bytes = 1KB
  - QPushButton 2개 × ~500 bytes = 1KB
  - QVBoxLayout 등 레이아웃 = 0.5KB
  - 총합: ~2.5KB (무시 가능한 수준)
- **사전 할당 전략**: 항상 메모리에 상주하지만 숨김 상태 (렌더링 비용 없음)

### 애니메이션 성능

- **QPropertyAnimation + windowOpacity**:
  - Qt Animation Framework는 GPU 가속 지원
  - 프로젝터 환경에서도 부드러운 60fps 목표
  - EasingCurve로 자연스러운 가속/감속
- **대안 (성능 문제 시)**: 애니메이션 비활성화 옵션 추가 (SettingsService 연동)

### 리모컨 포커스 전환 속도

- **Qt 포커스 시스템**: 네이티브 이벤트 루프 기반, 지연 없음
- **탭 오더 설정**: setTabOrder()는 단순 포인터 연결, 오버헤드 없음
- **포커스 스타일**: QSS 스타일시트는 캐시됨, 재계산 없음

---

## 11. 영향 범위 분석

### 수정 필요한 기존 파일

| 파일 경로 | 변경 내용 | 이유 |
|----------|----------|------|
| `src/browser/BrowserWindow.h` | - `QStackedLayout *stackedLayout_` 추가<br>- `QWidget *contentWidget_` 추가<br>- `ErrorPage *errorPage_` 추가<br>- 4개 슬롯 추가 | ErrorPage 통합 |
| `src/browser/BrowserWindow.cpp` | - 생성자에 3개 멤버 초기화<br>- setupUI()에 QStackedLayout 설정<br>- setupConnections()에 에러 시그널 연결<br>- 4개 슬롯 구현 | ErrorPage 통합 |
| `src/CMakeLists.txt` | - `src/ui/ErrorPage.cpp` 추가<br>- `src/ui/ErrorPage.h` 추가 | 새 파일 빌드 |

### 새로 생성할 파일

| 파일 경로 | 역할 | 의존성 |
|----------|------|--------|
| `src/ui/ErrorPage.h` | ErrorPage 클래스 헤더 | `<QWidget>`, `<QLabel>`, `<QPushButton>` |
| `src/ui/ErrorPage.cpp` | ErrorPage 클래스 구현 | ErrorPage.h, WebView.h (시그널 연결용) |
| `resources/icons/error.png` | 기본 에러 아이콘 (80x80px) | 없음 |
| `tests/unit/ErrorPageTest.cpp` | ErrorPage 단위 테스트 (선택 사항) | Google Test, Qt Test |

### 영향 받는 기존 기능

| 기능명 | 영향 내용 | 대응 방안 |
|--------|----------|----------|
| F-02 (웹뷰 통합) | `loadError`, `loadTimeout` 시그널이 BrowserWindow에서 사용됨 | 기존 시그널 유지, 추가 구독만 |
| F-04 (페이지 탐색) | 에러 상태에서 뒤로/앞으로 버튼 비활성화 필요 (선택 사항) | NavigationBar에 `setErrorState(bool)` 추가 (향후) |
| F-05 (로딩 인디케이터) | 재시도 시 LoadingIndicator 자동 표시됨 (기존 시그널 유지) | 변경 없음, 기존 연결 활용 |
| F-06 (탭 관리) | 탭 상태에 `isError: bool` 추가 필요 | TabManager에 `setTabError(tabId, bool)` 추가 (향후) |
| F-07 (북마크 관리) | 영향 없음 | 없음 |
| F-08 (히스토리 관리) | 에러 페이지는 히스토리 기록 안 됨 (loadFinished(true)만 기록) | 변경 없음, 기존 조건문 활용 |

---

## 12. 기술적 주의사항

### 에러 감지 제약사항

1. **HTTP 상태 코드 제한**:
   - Qt WebEngineView는 404, 500 등 HTTP 에러 페이지를 정상 로딩으로 처리
   - loadFinished(true)로 emit되므로 에러 감지 불가
   - 대응: 일반적인 네트워크 에러/타임아웃만 처리 (현 단계)

2. **에러 메시지 상세도**:
   - Qt가 제공하는 errorString은 간단한 문자열만 ("Connection refused", "Host not found" 등)
   - 에러 코드는 제공되지 않음
   - 대응: 문자열 파싱으로 타입 추론 (완벽하지 않음)

3. **타임아웃 구현 위치**:
   - WebView 클래스 내부에 QTimer로 이미 구현됨
   - ErrorPage는 타임아웃 감지 로직 없음 (시그널만 구독)

### 리모컨 포커스 관리

1. **Back 키 처리**:
   - ErrorPage::keyPressEvent()에서 Back 키 무시
   - 에러 상태에서는 반드시 재시도 또는 홈 이동만 가능
   - 사용자가 에러 화면에서 벗어나지 못함 (의도된 동작)

2. **포커스 초기화**:
   - ErrorPage::showError()에서 retryButton에 자동 포커스
   - 사용자는 즉시 Enter 키로 재시도 가능
   - 홈 버튼으로 이동 시 좌/우 방향키 사용

3. **포커스 스타일 테스트**:
   - 실제 webOS 프로젝터에서 QSS 포커스 테두리 확인 필요
   - 일부 Qt 빌드는 포커스 스타일이 플랫폼 의존적

### 애니메이션 호환성

1. **windowOpacity 제약**:
   - QWidget::windowOpacity는 최상위 윈도우에서만 동작 (일부 플랫폼)
   - QStackedLayout 내부 위젯은 동작 안 할 수 있음
   - 대응: showEvent()에서 애니메이션 실패 시 즉시 표시

2. **성능 저하 시**:
   - 프로젝터 GPU가 약한 경우 애니메이션이 끊길 수 있음
   - 대응: SettingsService에 애니메이션 비활성화 옵션 추가 (향후)

### 메모리 누수 방지

1. **QPropertyAnimation 자동 삭제**:
   - `DeleteWhenStopped` 플래그 필수
   - parent 설정 금지 (임시 객체)
   - connect()의 람다는 애니메이션 객체 캡처 금지

2. **시그널/슬롯 연결 해제**:
   - Qt는 객체 소멸 시 자동 연결 해제
   - 명시적 disconnect() 불필요 (parent-child 관계 유지 시)

### 로그 기록

1. **qCritical() 사용**:
   - webOS pmloglib로 자동 통합
   - 로그 레벨: Critical (에러 발생 시)
   - 로그 포맷: `[2026-02-14 20:45:00] Page load error: message=..., url=...`

2. **민감 정보 보호**:
   - URL은 로그에 기록되지만, POST 파라미터는 로그 안 됨 (Qt 기본 동작)
   - 사용자 입력 비밀번호 등은 WebView 내부에서 처리되므로 노출 안 됨

---

## 13. 확장 가능성

### 향후 추가 가능한 기능

1. **에러 타입별 복구 제안**:
   - 네트워크 에러 → "Wi-Fi 연결을 확인하세요"
   - 타임아웃 → "잠시 후 다시 시도하세요"
   - 구현: ErrorPage에 suggestionLabel 추가

2. **에러 통계 수집**:
   - 에러 발생 빈도, 타입별 분포 수집
   - SettingsService에 통계 저장
   - 대시보드 UI 제공 (F-11 연계)

3. **오프라인 모드**:
   - 네트워크 단절 시 로컬 캐시된 페이지 표시
   - Service Worker 연동 (향후 Chromium 기능 활용)

4. **에러 히스토리**:
   - 이전 에러 목록 별도 저장
   - 디버깅 및 사용자 지원용
   - StorageService에 errorHistory 테이블 추가

5. **원격 에러 리포팅**:
   - 에러 발생 시 원격 서버로 전송 (사용자 동의 하에)
   - 앱 품질 개선 데이터 수집

---

## 14. 테스트 계획

### 단위 테스트 (Google Test + Qt Test)

```cpp
// tests/unit/ErrorPageTest.cpp
TEST(ErrorPageTest, ShowNetworkError) {
    ErrorPage errorPage;
    errorPage.showError(ErrorType::NetworkError, "Connection refused", QUrl("https://example.com"));

    // 제목 확인
    EXPECT_EQ(errorPage.titleLabel_->text(), "네트워크 연결 실패");

    // 메시지 확인
    EXPECT_EQ(errorPage.messageLabel_->text(), "Connection refused");

    // URL truncate 확인
    EXPECT_TRUE(errorPage.urlLabel_->text().contains("example.com"));
}

TEST(ErrorPageTest, RetryButtonFocus) {
    ErrorPage errorPage;
    errorPage.showError(ErrorType::Timeout, "Timeout", QUrl("https://slow.com"));

    // 재시도 버튼에 포커스 확인
    EXPECT_TRUE(errorPage.retryButton_->hasFocus());
}

TEST(ErrorPageTest, RetrySignalEmit) {
    ErrorPage errorPage;
    QSignalSpy spy(&errorPage, &ErrorPage::retryRequested);

    errorPage.retryButton_->click();

    EXPECT_EQ(spy.count(), 1);
}
```

### 통합 테스트 (BrowserWindow + WebView + ErrorPage)

```cpp
// tests/integration/ErrorHandlingTest.cpp
TEST(ErrorHandlingIntegrationTest, NetworkErrorFlow) {
    BrowserWindow window;

    // 잘못된 URL 로드
    window.webView_->load(QUrl("https://invalid-domain-12345.com"));

    // 에러 시그널 대기 (최대 5초)
    QSignalSpy errorSpy(window.webView_, &WebView::loadError);
    ASSERT_TRUE(errorSpy.wait(5000));

    // ErrorPage가 표시되었는지 확인
    EXPECT_EQ(window.stackedLayout_->currentWidget(), window.errorPage_);

    // 재시도 버튼 클릭
    window.errorPage_->retryButton_->click();

    // WebView로 전환 확인 (애니메이션 완료 후)
    QTest::qWait(300);  // 페이드아웃 200ms + 여유 100ms
    EXPECT_EQ(window.stackedLayout_->currentWidget(), window.webView_);
}
```

### 실제 디바이스 테스트 (LG 프로젝터 HU715QW)

1. **테스트 케이스**:
   - [ ] 네트워크 단절 상태에서 페이지 로드 → 에러 화면 표시
   - [ ] 30초 타임아웃 → 에러 화면 표시
   - [ ] 재시도 버튼 클릭 → 페이지 재로드
   - [ ] 홈 버튼 클릭 → 홈페이지 이동
   - [ ] 리모컨 좌/우 방향키 → 버튼 포커스 전환
   - [ ] 리모컨 Back 키 → 에러 화면 유지 (무시)
   - [ ] 3m 거리에서 에러 메시지 가독성 확인
   - [ ] 페이드인/아웃 애니메이션 부드러움 확인

2. **성능 측정**:
   - 에러 발생 → 화면 표시: 500ms 이내 목표
   - 재시도 클릭 → 로딩 시작: 300ms 이내 목표

---

## 15. 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | F-10 기술 설계 |

---

## 다음 단계

이 기술 설계서를 기반으로 product-manager 에이전트가 다음 문서를 작성합니다:
- `docs/specs/error-handling/plan.md` (구현 계획서)

**product-manager에게 전달할 사항**:
- ErrorPage 클래스 구현: 1~1.5일 예상
- BrowserWindow 통합: QStackedLayout 설정 간단
- 의존성: F-02 (WebView) 완료 필요 ✅
- 충돌 위험: 낮음 (새 파일 생성 위주)
- 테스트 우선순위: 실제 디바이스 리모컨 포커스 테스트 필수
