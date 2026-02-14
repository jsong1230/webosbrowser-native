# 로딩 인디케이터 — 기술 설계서

## 1. 참조
- 요구사항 분석서: docs/specs/loading-indicator/requirements.md
- WebView 헤더: src/browser/WebView.h
- BrowserWindow 헤더: src/browser/BrowserWindow.h
- PRD: docs/project/prd.md

## 2. 아키텍처 개요

### 전체 구조
웹 페이지 로딩 중 시각적 피드백을 제공하는 Qt Widgets 기반 UI 컴포넌트입니다. QWebEngineView의 실제 로딩 진행률(0~100%)을 QProgressBar로 표시하고, 화면 중앙에 스피너 애니메이션을 오버레이로 표시합니다.

```
BrowserWindow
┌────────────────────────────────────────────────┐
│ ┌────────────────────────────────────────────┐ │
│ │  LoadingIndicator (상단 프로그레스바)      │ │  ← QProgressBar (화면 상단)
│ │  ████████████░░░░░░░░░░░░░░░░░░░░░  75%    │ │
│ └────────────────────────────────────────────┘ │
│                                                  │
│   WebView                                       │
│   ┌────────────────────────────────────────┐   │
│   │         [로딩 스피너 오버레이]          │   │  ← QLabel + QMovie (중앙)
│   │              ⟳ (회전)                   │   │
│   │         "페이지 로딩 중..."             │   │  ← QLabel (텍스트)
│   │                                          │   │
│   │         www.youtube.com                 │   │  ← QLabel (URL, 선택적)
│   └────────────────────────────────────────┘   │
│                                                  │
│   [statusBar: "로딩 중... 75%"]                 │  ← 기존 상태바 연동
└────────────────────────────────────────────────┘

에러 시 (1초간 표시)
┌────────────────────────────────────────────────┐
│ ┌────────────────────────────────────────────┐ │
│ │  ████████████████████████████████  (빨간색) │ │  ← 프로그레스바 빨간색
│ └────────────────────────────────────────────┘ │
│   ┌────────────────────────────────────────┐   │
│   │              ⚠️                          │   │  ← 에러 아이콘
│   │         "페이지 로딩 실패"              │   │
│   └────────────────────────────────────────┘   │
└────────────────────────────────────────────────┘
```

### 핵심 설계 원칙
1. **Qt 표준 위젯 활용**: QProgressBar, QLabel, QMovie 등 Qt 기본 위젯 우선 사용
2. **실제 진행률 표시**: QWebEngineView의 loadProgress(int) 시그널 기반
3. **오버레이 방식**: WebView 위에 z-order 높게 배치 (QStackedWidget 또는 raise())
4. **하드웨어 가속**: Qt OpenGL 기반 애니메이션 (webOS 최적화)
5. **시그널/슬롯 기반 통신**: WebView와 느슨한 결합

## 3. 아키텍처 결정

### 결정 1: 프로그레스바 위치 및 스타일
- **선택지**:
  - A) BrowserWindow 상단 (QMainWindow::addToolBar에 추가)
  - B) WebView 내부 상단 (WebView 자식 위젯)
  - C) BrowserWindow 레이아웃에 별도 위젯으로 추가
- **결정**: C) BrowserWindow 레이아웃에 별도 위젯
- **근거**:
  - BrowserWindow::mainLayout_에 QProgressBar 추가 (WebView 위에 배치)
  - QVBoxLayout 순서: QProgressBar → WebView → statusBar
  - 독립적인 위젯으로 재사용 가능 (향후 탭별 로딩 인디케이터)
  - QMainWindow::addToolBar는 툴바 스타일로 표시되어 프로그레스바에 부적합
- **트레이드오프**: BrowserWindow.cpp 수정 필요 (레이아웃 재구성)

### 결정 2: 스피너 애니메이션 방식
- **선택지**:
  - A) QLabel + QMovie (GIF 애니메이션 파일)
  - B) QProgressBar (BusyIndicator 스타일, QSS로 무한 회전)
  - C) QPainter + QTimer (커스텀 회전 애니메이션 직접 그리기)
  - D) QML Loader + BusyIndicator (QML 컴포넌트 사용)
- **결정**: A) QLabel + QMovie (GIF 애니메이션 파일)
- **근거**:
  - 가장 간단하고 안정적인 방법 (Qt 표준 기능)
  - GIF 파일은 디자이너가 커스터마이징 가능 (스피너 스타일 변경 용이)
  - QMovie는 하드웨어 가속 지원 (OpenGL 렌더링)
  - QPainter는 구현 복잡도 높고 성능 최적화 필요
  - QML은 Qt Widgets와 혼용 시 추가 의존성
- **트레이드오프**: GIF 파일 리소스 추가 필요 (resources/icons/spinner.gif)

### 결정 3: 오버레이 배치 방식
- **선택지**:
  - A) QStackedWidget (BrowserWindow에서 WebView와 LoadingOverlay 관리)
  - B) WebView 자식 위젯으로 추가 (WebView::addChild)
  - C) LoadingIndicator를 독립 QWidget으로 만들고 QWidget::raise() 사용
- **결정**: C) LoadingIndicator를 독립 QWidget, raise() 사용
- **근거**:
  - 가장 단순한 방법 (QStackedWidget은 전체 화면 전환용)
  - WebView 내부에 추가하면 WebView가 LoadingIndicator를 관리해야 함 (결합도 증가)
  - raise()는 z-order만 조정하여 오버레이 효과 제공
  - 페이드인/아웃 시 show()/hide() 호출만으로 제어 가능
- **트레이드오프**: 레이아웃 관리가 약간 복잡해짐 (수동으로 위치 조정)

### 결정 4: 로딩 진행률 업데이트 빈도
- **선택지**:
  - A) WebView의 loadProgress(int) 시그널을 그대로 반영 (무제한)
  - B) QTimer로 100ms마다 업데이트 (10fps)
  - C) 진행률 변화가 5% 이상일 때만 업데이트
- **결정**: B) QTimer로 100ms마다 업데이트 (10fps)
- **근거**:
  - QWebEngineView는 loadProgress 시그널을 매우 빈번하게 emit (50fps 이상)
  - 과도한 repaint는 CPU 사용률 증가 및 UI 버벅임 유발
  - 사람 눈은 10fps 이상이면 부드럽게 인지 (프로그레스바는 30fps 불필요)
  - QTimer 쓰로틀링으로 성능과 부드러움 균형
- **구현**: LoadingIndicator 내부에 QTimer (100ms interval) + 진행률 캐싱
- **트레이드오프**: 진행률 업데이트가 최대 100ms 지연 (무시할 수 있는 수준)

### 결정 5: 로딩 타임아웃 처리
- **선택지**:
  - A) WebView 내부에서 QTimer(30초) 관리, loadTimeout() 시그널 emit
  - B) LoadingIndicator에서 QTimer(30초) 관리, 30초 후 경고 표시
  - C) BrowserWindow에서 타임아웃 관리, LoadingIndicator는 UI만 담당
- **결정**: A) WebView 내부에서 타임아웃 관리
- **근거**:
  - 요구사항 분석서에서 WebView가 loadTimeout() 시그널 제공 (이미 정의됨)
  - LoadingIndicator는 순수 UI 컴포넌트로 유지 (타이머 로직 없음)
  - 타임아웃은 WebView의 로딩 상태와 밀접하게 연관 (단일 책임 원칙)
  - BrowserWindow는 타임아웃 시그널을 받아 QMessageBox 표시
- **구현**: WebView::loadStarted()에서 QTimer::singleShot(30000) 시작
- **트레이드오프**: WebView가 타이머 관리 책임 추가 (로딩 로직과 함께 관리)

### 결정 6: 에러 상태 시각적 피드백
- **선택지**:
  - A) 프로그레스바 색상만 변경 (녹색 → 빨간색)
  - B) 스피너를 에러 아이콘으로 교체 + 프로그레스바 빨간색
  - C) 별도의 에러 다이얼로그 표시 (QMessageBox)
- **결정**: B) 스피너를 에러 아이콘으로 교체 + 프로그레스바 빨간색
- **근거**:
  - 명확한 시각적 구분 (정상 로딩 vs 에러)
  - 1초간 표시 후 F-10 에러 페이지로 전환 (점진적 전환)
  - QMessageBox는 모달 다이얼로그로 사용자 클릭 필요 (UX 저해)
  - QSS 동적 변경으로 프로그레스바 색상 전환 가능
- **구현**:
  - `loadError(QString)` 시그널 수신 → setStyleSheet("...background: red...")
  - QLabel 아이콘 변경 → QPixmap(":/icons/error.png") 또는 유니코드 "⚠️"
  - QTimer::singleShot(1000) 후 hide() 및 에러 페이지 전환
- **트레이드오프**: 에러 상태 처리 로직 추가 (상태 머신 복잡도 증가)

### 결정 7: 페이드아웃 애니메이션
- **선택지**:
  - A) QPropertyAnimation (opacity 0.0 → 1.0)
  - B) QGraphicsOpacityEffect + QPropertyAnimation
  - C) 애니메이션 없이 즉시 hide()
- **결정**: B) QGraphicsOpacityEffect + QPropertyAnimation
- **근거**:
  - Qt Widgets에서 opacity 애니메이션은 QGraphicsOpacityEffect 필요
  - 부드러운 전환 효과로 UX 향상
  - QPropertyAnimation은 하드웨어 가속 지원
  - 즉시 hide()는 갑작스러운 화면 전환으로 불편
- **구현**:
  ```cpp
  QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(this);
  setGraphicsEffect(effect);
  QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity");
  anim->setDuration(500); // 0.5초
  anim->setStartValue(1.0);
  anim->setEndValue(0.0);
  anim->start(QAbstractAnimation::DeleteWhenStopped);
  ```
- **트레이드오프**: 애니메이션 코드 추가 (복잡도 증가)

## 4. 클래스 설계

### LoadingIndicator 클래스

#### 헤더 파일: src/ui/LoadingIndicator.h

```cpp
#pragma once

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QMovie>
#include <QScopedPointer>

namespace webosbrowser {

/**
 * @class LoadingIndicator
 * @brief 웹 페이지 로딩 중 시각적 피드백 제공 위젯
 *
 * QProgressBar (상단) + 스피너 오버레이 (중앙) 조합.
 * WebView의 loadProgress 시그널과 연동하여 실시간 진행률 표시.
 */
class LoadingIndicator : public QWidget {
    Q_OBJECT

public:
    explicit LoadingIndicator(QWidget *parent = nullptr);
    ~LoadingIndicator() override;

    LoadingIndicator(const LoadingIndicator&) = delete;
    LoadingIndicator& operator=(const LoadingIndicator&) = delete;

    /**
     * @brief 로딩 인디케이터 표시 (페이드인)
     */
    void show();

    /**
     * @brief 로딩 인디케이터 숨김 (페이드아웃)
     * @param immediate true면 즉시 숨김 (애니메이션 없음)
     */
    void hide(bool immediate = false);

    /**
     * @brief 진행률 업데이트
     * @param progress 진행률 (0~100)
     */
    void setProgress(int progress);

    /**
     * @brief 로딩 중인 URL 표시
     * @param url URL 문자열 (최대 50자, 말줄임 처리)
     */
    void setLoadingUrl(const QString &url);

    /**
     * @brief 에러 상태로 전환
     * @param errorMessage 에러 메시지
     */
    void showError(const QString &errorMessage);

    /**
     * @brief 타임아웃 경고 표시 (30초 초과)
     */
    void showTimeoutWarning();

private:
    void setupUI();
    void setupAnimations();
    void startFadeIn();
    void startFadeOut();
    void resetToNormalState();

private slots:
    void onUpdateThrottled();

private:
    // UI 컴포넌트
    QProgressBar *progressBar_;       ///< 상단 프로그레스바
    QWidget *overlayWidget_;          ///< 중앙 오버레이 컨테이너
    QVBoxLayout *overlayLayout_;      ///< 오버레이 레이아웃
    QLabel *spinnerLabel_;            ///< 스피너 아이콘 (QMovie)
    QLabel *statusLabel_;             ///< "페이지 로딩 중..." 텍스트
    QLabel *urlLabel_;                ///< URL 표시 (선택적)

    // 애니메이션
    QMovie *spinnerMovie_;            ///< 스피너 GIF 애니메이션
    QGraphicsOpacityEffect *opacityEffect_;  ///< 페이드인/아웃 효과

    // 상태 관리
    QTimer *updateThrottleTimer_;     ///< 진행률 업데이트 쓰로틀링 (100ms)
    int cachedProgress_;              ///< 캐싱된 진행률 (쓰로틀링용)
    bool isErrorState_;               ///< 에러 상태 플래그
};

} // namespace webosbrowser
```

#### 구현 파일: src/ui/LoadingIndicator.cpp (핵심 로직)

```cpp
#include "LoadingIndicator.h"
#include <QPropertyAnimation>
#include <QDebug>

namespace webosbrowser {

LoadingIndicator::LoadingIndicator(QWidget *parent)
    : QWidget(parent)
    , progressBar_(new QProgressBar(this))
    , overlayWidget_(new QWidget(this))
    , overlayLayout_(new QVBoxLayout(overlayWidget_))
    , spinnerLabel_(new QLabel(overlayWidget_))
    , statusLabel_(new QLabel("페이지 로딩 중...", overlayWidget_))
    , urlLabel_(new QLabel(overlayWidget_))
    , spinnerMovie_(new QMovie(":/resources/icons/spinner.gif", QByteArray(), this))
    , opacityEffect_(new QGraphicsOpacityEffect(this))
    , updateThrottleTimer_(new QTimer(this))
    , cachedProgress_(0)
    , isErrorState_(false)
{
    setupUI();
    setupAnimations();
    hide(true);  // 초기 상태: 숨김
}

LoadingIndicator::~LoadingIndicator() {
    qDebug() << "LoadingIndicator: 소멸";
}

void LoadingIndicator::setupUI() {
    // 프로그레스바 설정
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(false);
    progressBar_->setFixedHeight(8);
    progressBar_->setStyleSheet(
        "QProgressBar {"
        "  border: none;"
        "  background-color: rgba(0, 0, 0, 0.1);"
        "}"
        "QProgressBar::chunk {"
        "  background-color: #00C851;"  // 녹색 (정상)
        "}"
    );

    // 오버레이 위젯 설정
    overlayWidget_->setStyleSheet(
        "QWidget {"
        "  background-color: rgba(0, 0, 0, 0.5);"  // 반투명 검정 배경
        "  border-radius: 12px;"
        "}"
    );

    // 스피너 설정
    spinnerLabel_->setMovie(spinnerMovie_);
    spinnerLabel_->setAlignment(Qt::AlignCenter);
    spinnerLabel_->setFixedSize(80, 80);

    // 상태 텍스트 설정
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setStyleSheet(
        "QLabel {"
        "  font-size: 24px;"
        "  color: white;"
        "  background-color: transparent;"
        "}"
    );

    // URL 라벨 설정
    urlLabel_->setAlignment(Qt::AlignCenter);
    urlLabel_->setStyleSheet(
        "QLabel {"
        "  font-size: 16px;"
        "  color: rgba(255, 255, 255, 0.8);"
        "  background-color: transparent;"
        "}"
    );

    // 오버레이 레이아웃 구성
    overlayLayout_->setSpacing(12);
    overlayLayout_->setContentsMargins(24, 24, 24, 24);
    overlayLayout_->addWidget(spinnerLabel_, 0, Qt::AlignCenter);
    overlayLayout_->addWidget(statusLabel_, 0, Qt::AlignCenter);
    overlayLayout_->addWidget(urlLabel_, 0, Qt::AlignCenter);

    // 포커스 정책 (리모컨 포커스 받지 않음)
    setFocusPolicy(Qt::NoFocus);
    progressBar_->setFocusPolicy(Qt::NoFocus);
    overlayWidget_->setFocusPolicy(Qt::NoFocus);
}

void LoadingIndicator::setupAnimations() {
    // 스피너 애니메이션 시작
    spinnerMovie_->start();

    // 페이드 효과 설정
    setGraphicsEffect(opacityEffect_);
    opacityEffect_->setOpacity(1.0);

    // 쓰로틀 타이머 설정 (100ms = 10fps)
    updateThrottleTimer_->setInterval(100);
    connect(updateThrottleTimer_, &QTimer::timeout,
            this, &LoadingIndicator::onUpdateThrottled);
}

void LoadingIndicator::show() {
    QWidget::show();
    raise();  // z-order 최상위
    startFadeIn();
    updateThrottleTimer_->start();
    qDebug() << "LoadingIndicator: 표시";
}

void LoadingIndicator::hide(bool immediate) {
    updateThrottleTimer_->stop();

    if (immediate) {
        QWidget::hide();
    } else {
        startFadeOut();
    }

    resetToNormalState();
    qDebug() << "LoadingIndicator: 숨김 (immediate:" << immediate << ")";
}

void LoadingIndicator::setProgress(int progress) {
    cachedProgress_ = qBound(0, progress, 100);
}

void LoadingIndicator::setLoadingUrl(const QString &url) {
    QString displayUrl = url;
    if (displayUrl.length() > 50) {
        displayUrl = displayUrl.left(47) + "...";
    }
    urlLabel_->setText(displayUrl);
}

void LoadingIndicator::showError(const QString &errorMessage) {
    isErrorState_ = true;

    // 프로그레스바 빨간색으로 변경
    progressBar_->setStyleSheet(
        "QProgressBar {"
        "  border: none;"
        "  background-color: rgba(0, 0, 0, 0.1);"
        "}"
        "QProgressBar::chunk {"
        "  background-color: #ff4444;"  // 빨간색 (에러)
        "}"
    );

    // 스피너를 에러 아이콘으로 교체
    spinnerMovie_->stop();
    spinnerLabel_->setPixmap(QPixmap(":/resources/icons/error.png"));

    // 상태 텍스트 변경
    statusLabel_->setText("페이지 로딩 실패");
    urlLabel_->setText(errorMessage);

    // 1초 후 숨김
    QTimer::singleShot(1000, this, [this]() {
        hide(false);
    });

    qDebug() << "LoadingIndicator: 에러 표시 -" << errorMessage;
}

void LoadingIndicator::showTimeoutWarning() {
    statusLabel_->setText("로딩이 오래 걸리고 있습니다");
    urlLabel_->setText("네트워크를 확인해주세요");
    qDebug() << "LoadingIndicator: 타임아웃 경고";
}

void LoadingIndicator::onUpdateThrottled() {
    progressBar_->setValue(cachedProgress_);
}

void LoadingIndicator::startFadeIn() {
    QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect_, "opacity", this);
    anim->setDuration(200);  // 0.2초
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void LoadingIndicator::startFadeOut() {
    QPropertyAnimation *anim = new QPropertyAnimation(opacityEffect_, "opacity", this);
    anim->setDuration(500);  // 0.5초
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    // 애니메이션 완료 후 hide() 호출
    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        QWidget::hide();
    });
}

void LoadingIndicator::resetToNormalState() {
    isErrorState_ = false;
    cachedProgress_ = 0;
    progressBar_->setValue(0);

    // 프로그레스바 녹색 복원
    progressBar_->setStyleSheet(
        "QProgressBar {"
        "  border: none;"
        "  background-color: rgba(0, 0, 0, 0.1);"
        "}"
        "QProgressBar::chunk {"
        "  background-color: #00C851;"  // 녹색 (정상)
        "}"
    );

    // 스피너 복원
    spinnerLabel_->setMovie(spinnerMovie_);
    spinnerMovie_->start();

    // 텍스트 복원
    statusLabel_->setText("페이지 로딩 중...");
    urlLabel_->clear();
}

} // namespace webosbrowser
```

### BrowserWindow 수정 (LoadingIndicator 통합)

#### BrowserWindow.h 수정

```cpp
// 기존 코드에 추가
#include "LoadingIndicator.h"

private:
    // UI 컴포넌트
    LoadingIndicator *loadingIndicator_;  ///< 로딩 인디케이터 (추가)
```

#### BrowserWindow.cpp 수정

```cpp
// setupUI() 수정
void BrowserWindow::setupUI() {
    // ... 기존 코드 ...

    // LoadingIndicator 생성
    loadingIndicator_ = new LoadingIndicator(centralWidget_);

    // 메인 레이아웃에 추가 (WebView 위에 오버레이)
    // 주의: mainLayout_에 추가하지 않고, centralWidget_의 자식으로만 설정
    // 위치는 resizeEvent에서 수동 조정

    // ... 기존 코드 ...
}

// setupConnections() 수정
void BrowserWindow::setupConnections() {
    // WebView 로딩 시작 이벤트
    connect(webView_, &WebView::loadStarted, this, [this]() {
        loadingIndicator_->show();
        loadingIndicator_->setProgress(0);
        statusLabel_->setText("로딩 중...");
        setWindowTitle("로딩 중... - webOS Browser");
        qDebug() << "BrowserWindow: 페이지 로딩 시작";
    });

    // WebView 로딩 진행률 이벤트
    connect(webView_, &WebView::loadProgress, this, [this](int progress) {
        loadingIndicator_->setProgress(progress);
        statusLabel_->setText(QString("로딩 중... %1%").arg(progress));
    });

    // WebView 로딩 완료 이벤트
    connect(webView_, &WebView::loadFinished, this, [this](bool success) {
        if (success) {
            loadingIndicator_->hide(false);  // 페이드아웃
            statusLabel_->setText("완료");
            QString title = webView_->title();
            if (title.isEmpty()) {
                title = webView_->url().toString();
            }
            setWindowTitle(title + " - webOS Browser");
            qDebug() << "BrowserWindow: 페이지 로딩 완료";
        } else {
            // 에러는 loadError 시그널에서 처리
            qDebug() << "BrowserWindow: 페이지 로딩 실패";
        }
    });

    // WebView URL 변경 이벤트
    connect(webView_, &WebView::urlChanged, this, [this](const QUrl &url) {
        loadingIndicator_->setLoadingUrl(url.toString());
        statusLabel_->setText(url.toString());
        qDebug() << "BrowserWindow: URL 변경 -" << url.toString();
    });

    // WebView 에러 이벤트 (추가)
    connect(webView_, &WebView::loadError, this, [this](const QString &errorString) {
        loadingIndicator_->showError(errorString);
        statusLabel_->setText("에러: " + errorString);
        qDebug() << "BrowserWindow: 로딩 에러 -" << errorString;
    });

    // WebView 타임아웃 이벤트 (추가)
    connect(webView_, &WebView::loadTimeout, this, [this]() {
        loadingIndicator_->showTimeoutWarning();
        statusLabel_->setText("타임아웃: 30초 초과");
        qDebug() << "BrowserWindow: 로딩 타임아웃";
    });

    // ... 기존 코드 ...
}

// resizeEvent 오버라이드 (오버레이 위치 조정)
void BrowserWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    // LoadingIndicator 오버레이 위치 조정
    QRect webViewGeometry = webView_->geometry();
    QSize overlaySize(400, 200);
    QPoint overlayPos(
        webViewGeometry.center().x() - overlaySize.width() / 2,
        webViewGeometry.center().y() - overlaySize.height() / 2
    );
    loadingIndicator_->setGeometry(QRect(overlayPos, overlaySize));
}
```

## 5. 시퀀스 흐름

### 주요 시나리오 1: 일반 페이지 로딩

```
사용자 → BrowserWindow → WebView → LoadingIndicator
  │                          │                │
  │  (URLBar Enter)          │                │
  │──────────────────────▶   │                │
  │                          │  load(url)     │
  │                          │───────────▶    │
  │                          │                │
  │                          │  loadStarted() │
  │                          │◀───────────    │
  │                          │                │  show()
  │                          │                │──────────▶
  │                          │                │  [프로그레스바 0%]
  │                          │                │  [스피너 표시]
  │                          │                │
  │                          │  loadProgress(25)
  │                          │───────────────────────────▶
  │                          │                │  setProgress(25)
  │                          │                │  [프로그레스바 25%]
  │                          │                │
  │                          │  loadProgress(50)
  │                          │───────────────────────────▶
  │                          │                │  setProgress(50)
  │                          │                │  [프로그레스바 50%]
  │                          │                │
  │                          │  loadProgress(100)
  │                          │───────────────────────────▶
  │                          │                │  setProgress(100)
  │                          │                │  [프로그레스바 100%]
  │                          │                │
  │                          │  loadFinished(true)
  │                          │◀───────────    │
  │                          │                │  hide(false)
  │                          │                │──────────▶
  │                          │                │  [페이드아웃 0.5초]
  │                          │                │  [숨김]
  │  [페이지 표시]           │                │
  │◀──────────────────────   │                │
```

### 주요 시나리오 2: 로딩 에러

```
사용자 → BrowserWindow → WebView → LoadingIndicator
  │                          │                │
  │  load(invalid-url)       │                │
  │──────────────────────▶   │                │
  │                          │  loadStarted() │
  │                          │───────────────────────────▶
  │                          │                │  show()
  │                          │                │  [프로그레스바 0%]
  │                          │                │
  │                          │  loadProgress(30)
  │                          │───────────────────────────▶
  │                          │                │  setProgress(30)
  │                          │                │
  │                          │  loadError("ERR_NAME_NOT_RESOLVED")
  │                          │◀───────────    │
  │                          │                │  showError()
  │                          │                │──────────▶
  │                          │                │  [프로그레스바 빨간색]
  │                          │                │  [스피너 → ⚠️]
  │                          │                │  [텍스트: "페이지 로딩 실패"]
  │                          │                │
  │                          │                │  (1초 대기)
  │                          │                │  hide(false)
  │                          │                │──────────▶
  │  [에러 페이지 표시]      │                │
  │◀──────────────────────   │                │
```

### 주요 시나리오 3: 로딩 타임아웃 (30초 초과)

```
사용자 → BrowserWindow → WebView → LoadingIndicator
  │                          │                │
  │  load(slow-url)          │                │
  │──────────────────────▶   │                │
  │                          │  loadStarted() │
  │                          │───────────────────────────▶
  │                          │                │  show()
  │                          │                │  [프로그레스바 0%]
  │                          │                │
  │                          │  (30초 경과)   │
  │                          │                │
  │                          │  loadTimeout() │
  │                          │◀───────────    │
  │                          │                │  showTimeoutWarning()
  │                          │                │──────────▶
  │                          │                │  [텍스트: "로딩이 오래 걸리고 있습니다"]
  │                          │                │  [텍스트: "네트워크를 확인해주세요"]
  │                          │                │  [프로그레스바 계속 표시]
  │                          │                │
  │  (Back 버튼)             │                │
  │──────────────────────▶   │  stop()        │
  │                          │───────────▶    │
  │                          │                │  hide(false)
  │                          │                │──────────▶
```

## 6. 영향 범위 분석

### 수정 필요한 기존 파일

| 파일 경로 | 변경 내용 | 이유 |
|---------|---------|------|
| `src/browser/BrowserWindow.h` | `LoadingIndicator *loadingIndicator_` 멤버 추가 | LoadingIndicator 통합 |
| `src/browser/BrowserWindow.cpp` | `setupUI()`, `setupConnections()`, `resizeEvent()` 수정 | LoadingIndicator 시그널 연결 및 오버레이 배치 |
| `src/browser/WebView.cpp` | `loadTimeout()` 시그널 구현 (QTimer 30초) | 타임아웃 기능 추가 |

### 새로 생성할 파일

| 파일 경로 | 역할 |
|---------|------|
| `src/ui/LoadingIndicator.h` | LoadingIndicator 클래스 헤더 |
| `src/ui/LoadingIndicator.cpp` | LoadingIndicator 클래스 구현 |
| `resources/icons/spinner.gif` | 스피너 애니메이션 GIF (80x80px) |
| `resources/icons/error.png` | 에러 아이콘 PNG (80x80px) |
| `resources/resources.qrc` | Qt 리소스 파일 (아이콘 등록) |
| `docs/components/LoadingIndicator.md` | 컴포넌트 문서 (사용법, API) |

### 영향 받는 기존 기능

| 기능 | 영향 내용 | 대응 방안 |
|-----|---------|----------|
| F-02 (WebView) | `loadStarted()`, `loadProgress()`, `loadFinished()` 시그널 사용 | 이미 정의되어 있으므로 영향 없음 |
| F-04 (네비게이션) | 뒤로/앞으로 이동 시에도 로딩 인디케이터 동작 | 시그널 기반이므로 자동 동작 |
| F-10 (에러 처리) | 에러 시 LoadingIndicator 1초 표시 후 에러 페이지로 전환 | LoadingIndicator의 showError()에서 1초 타이머 처리 |

## 7. 리소스 설계

### GIF 애니메이션 스펙 (spinner.gif)

| 속성 | 값 |
|-----|-----|
| 크기 | 80x80px |
| 프레임 수 | 12프레임 (30도 회전) |
| 프레임 레이트 | 12fps (1000ms / 12 = ~83ms/프레임) |
| 배경 | 투명 (알파 채널) |
| 색상 | 흰색 (#FFFFFF) 또는 브랜드 컬러 |
| 파일 크기 | 50KB 이하 |

### 에러 아이콘 스펙 (error.png)

| 속성 | 값 |
|-----|-----|
| 크기 | 80x80px |
| 배경 | 투명 (알파 채널) |
| 아이콘 | 삼각형 경고 표시(⚠️) |
| 색상 | 빨간색 (#ff4444) |
| 파일 크기 | 10KB 이하 |

### Qt 리소스 파일 (resources/resources.qrc)

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource>
        <file>icons/spinner.gif</file>
        <file>icons/error.png</file>
    </qresource>
</RCC>
```

### CMakeLists.txt 수정

```cmake
# Qt 리소스 컴파일 추가
qt5_add_resources(RESOURCES
    resources/resources.qrc
)

# 소스 파일에 LoadingIndicator 추가
set(SOURCES
    # ... 기존 소스 ...
    src/ui/LoadingIndicator.cpp
    ${RESOURCES}  # 리소스 추가
)
```

## 8. QSS 스타일 시트 설계

### 정상 상태 (녹색 프로그레스바)

```css
QProgressBar {
    border: none;
    background-color: rgba(0, 0, 0, 0.1);
    min-height: 8px;
    max-height: 8px;
}

QProgressBar::chunk {
    background-color: #00C851;  /* 녹색 (LG webOS 브랜드 컬러 고려) */
    border-radius: 4px;
}
```

### 에러 상태 (빨간색 프로그레스바)

```css
QProgressBar {
    border: none;
    background-color: rgba(0, 0, 0, 0.1);
    min-height: 8px;
    max-height: 8px;
}

QProgressBar::chunk {
    background-color: #ff4444;  /* 빨간색 */
    border-radius: 4px;
}
```

### 오버레이 위젯

```css
QWidget#overlayWidget {
    background-color: rgba(0, 0, 0, 0.5);  /* 반투명 검정 */
    border-radius: 12px;
}

QLabel#spinnerLabel {
    background-color: transparent;
}

QLabel#statusLabel {
    font-size: 24px;
    font-weight: bold;
    color: white;
    background-color: transparent;
}

QLabel#urlLabel {
    font-size: 16px;
    color: rgba(255, 255, 255, 0.8);
    background-color: transparent;
}
```

## 9. 기술적 주의사항

### 성능 최적화
1. **프로그레스바 업데이트 쓰로틀링**: QTimer 100ms 간격으로 repaint 최소화 (CPU 사용률 5% 이하 유지)
2. **하드웨어 가속 활용**: Qt OpenGL 렌더링 (webOS 기본 지원)
3. **메모리 관리**: QScopedPointer 사용으로 자동 메모리 해제, 3MB 이하 유지
4. **애니메이션 최적화**: QPropertyAnimation은 Qt Graphics Framework 사용 (GPU 가속)

### 리모컨 최적화
1. **포커스 정책**: LoadingIndicator는 `Qt::NoFocus` 설정 (리모컨 포커스 받지 않음)
2. **Back 버튼 처리**: BrowserWindow에서 Back 버튼 이벤트 수신 시 `WebView::stop()` 호출 → 로딩 취소
3. **포커스 유지**: 로딩 중에도 WebView가 포커스 유지 (스크롤 등 리모컨 조작 가능)

### webOS 플랫폼 고려사항
1. **메모리 제약**: 프로젝터 하드웨어 메모리 제한 (LoadingIndicator는 경량 위젯으로 유지)
2. **무선 네트워크**: 네트워크 속도 변동성이 크므로 타임아웃 경고 중요
3. **OpenGL 지원**: QMovie, QPropertyAnimation 모두 OpenGL 가속 지원 확인

### 동시성 고려사항
1. **시그널/슬롯 스레드 안전성**: Qt는 기본적으로 스레드 안전한 시그널/슬롯 제공
2. **QTimer 주기 충돌 방지**: updateThrottleTimer_와 애니메이션 타이머가 다른 주기 사용
3. **에러 상태 경합 방지**: isErrorState_ 플래그로 중복 에러 처리 방지

### 테스트 시나리오 고려사항
1. **빠른 로딩 (1초 이내)**: 프로그레스바가 0% → 100% 즉시 이동 후 페이드아웃 (깜박임 없음)
2. **매우 느린 로딩 (30초 초과)**: 타임아웃 경고 표시되는지 확인
3. **연속 로딩**: 이전 로딩 인디케이터가 완전히 정리되고 새 로딩 시작하는지 확인
4. **에러 복구**: 에러 후 다시 정상 로딩 시 녹색 프로그레스바로 복원되는지 확인

## 10. 단위 테스트 계획

### 테스트 파일: tests/unit/LoadingIndicatorTest.cpp

```cpp
#include <gtest/gtest.h>
#include "ui/LoadingIndicator.h"
#include <QTest>

using namespace webosbrowser;

class LoadingIndicatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        indicator_ = new LoadingIndicator();
    }

    void TearDown() override {
        delete indicator_;
    }

    LoadingIndicator *indicator_;
};

TEST_F(LoadingIndicatorTest, InitialState) {
    EXPECT_FALSE(indicator_->isVisible());
}

TEST_F(LoadingIndicatorTest, ShowHide) {
    indicator_->show();
    EXPECT_TRUE(indicator_->isVisible());

    indicator_->hide(true);  // 즉시 숨김
    EXPECT_FALSE(indicator_->isVisible());
}

TEST_F(LoadingIndicatorTest, SetProgress) {
    indicator_->setProgress(50);
    // 100ms 대기 (쓰로틀링)
    QTest::qWait(150);
    // 진행률 확인은 private 멤버 접근 불가로 생략
}

TEST_F(LoadingIndicatorTest, ErrorState) {
    indicator_->show();
    indicator_->showError("Test Error");

    // 1초 후 숨김 확인
    QTest::qWait(1100);
    EXPECT_FALSE(indicator_->isVisible());
}

TEST_F(LoadingIndicatorTest, ProgressBounds) {
    indicator_->setProgress(-10);  // 음수
    indicator_->setProgress(150);  // 100 초과
    // qBound로 0~100 범위 보장 (내부 검증)
}
```

### 통합 테스트: tests/integration/WebViewLoadingTest.cpp

```cpp
// WebView와 LoadingIndicator 통합 테스트
TEST_F(BrowserWindowTest, LoadingIndicatorIntegration) {
    // WebView 로딩 시작
    webView_->load("https://www.google.com");

    // LoadingIndicator 표시 확인
    EXPECT_TRUE(loadingIndicator_->isVisible());

    // 로딩 완료 대기 (최대 10초)
    QSignalSpy spy(webView_, &WebView::loadFinished);
    ASSERT_TRUE(spy.wait(10000));

    // LoadingIndicator 숨김 확인 (페이드아웃 500ms 대기)
    QTest::qWait(600);
    EXPECT_FALSE(loadingIndicator_->isVisible());
}
```

## 11. 컴포넌트 문서 작성 계획

### docs/components/LoadingIndicator.md

```markdown
# LoadingIndicator 컴포넌트

## 개요
웹 페이지 로딩 중 시각적 피드백을 제공하는 Qt Widgets 기반 UI 컴포넌트.

## 주요 기능
- 실시간 로딩 진행률 표시 (QProgressBar)
- 스피너 애니메이션 (QMovie)
- 로딩 상태 텍스트 및 URL 표시
- 에러 상태 시각적 피드백 (빨간색 프로그레스바 + 에러 아이콘)
- 타임아웃 경고 (30초 초과)
- 페이드인/아웃 애니메이션

## API

### Public Methods
- `void show()`: 로딩 인디케이터 표시 (페이드인)
- `void hide(bool immediate)`: 로딩 인디케이터 숨김 (페이드아웃 또는 즉시)
- `void setProgress(int progress)`: 진행률 업데이트 (0~100)
- `void setLoadingUrl(const QString &url)`: 로딩 중인 URL 표시
- `void showError(const QString &errorMessage)`: 에러 상태 표시
- `void showTimeoutWarning()`: 타임아웃 경고 표시

### Signals
(없음 - 순수 UI 컴포넌트)

## 사용 예제

\`\`\`cpp
// BrowserWindow에서 사용
LoadingIndicator *indicator = new LoadingIndicator(this);

// WebView 시그널 연결
connect(webView_, &WebView::loadStarted, [indicator]() {
    indicator->show();
});

connect(webView_, &WebView::loadProgress, [indicator](int progress) {
    indicator->setProgress(progress);
});

connect(webView_, &WebView::loadFinished, [indicator](bool success) {
    indicator->hide(false);  // 페이드아웃
});
\`\`\`

## 스타일 커스터마이징
QSS를 통해 프로그레스바 색상 변경 가능.
```

## 12. 향후 확장 가능성

### F-06 (탭 관리)와의 연동
- 각 탭마다 독립적인 LoadingIndicator 인스턴스 관리
- 백그라운드 탭 로딩 시: 프로그레스바 숨김, 탭 아이콘에 작은 스피너만 표시
- TabManager에서 활성 탭 변경 시 LoadingIndicator 전환
- 구현: `QMap<int, LoadingIndicator*> tabLoadingIndicators_;`

### F-11 (설정 화면)과의 연동
- 애니메이션 비활성화 옵션 (모션 감도가 높은 사용자)
- 프로그레스바 스타일 선택 (슬림 4px / 표준 8px / 굵게 12px)
- 스피너 스타일 선택 (회전 원형 / 점 3개 / 막대)
- QSettings로 설정 영속화
- 구현: `LoadingIndicator::applySettings(const QSettings &settings)`

### 성능 모니터링 (F-08 히스토리 연동)
- 페이지별 평균 로딩 시간 측정 (QElapsedTimer)
- HistoryService에 로딩 시간 저장
- 설정 화면에서 "평균 로딩 시간 통계" 표시
- 구현: `QElapsedTimer loadingTimer_` 추가

### 고급 UX 기능
- 프리로딩 인디케이터: 백그라운드 탭 미리 로딩 시 작은 프로그레스바
- 네트워크 속도 표시: "빠름 🟢 / 보통 🟡 / 느림 🔴" 아이콘
- 예상 완료 시간: 이전 로딩 패턴 기반 예측 (ML 모델 또는 단순 평균)

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | Native App 관점 전면 재작성 | Qt/C++ 기술 스택으로 마이그레이션 |
| 2026-02-14 | QMovie + GIF 방식 선택 | 단순성과 안정성 (GIF 리소스 사용) |
| 2026-02-14 | 쓰로틀링 100ms 결정 | 성능과 부드러움 균형 (10fps) |
| 2026-02-14 | raise() 오버레이 방식 채택 | QStackedWidget 불필요, 단순성 우선 |
