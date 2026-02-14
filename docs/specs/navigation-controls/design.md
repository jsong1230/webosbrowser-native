# 페이지 탐색 컨트롤 — 기술 설계서 (Native App)

## 1. 참조
- 요구사항 분석서: docs/specs/navigation-controls/requirements.md
- F-02 설계서: docs/specs/webview-integration/design.md
- WebView 헤더: src/browser/WebView.h
- BrowserWindow 헤더: src/browser/BrowserWindow.h
- CLAUDE.md: /Users/jsong/dev/jsong1230-github/webosbrowser-native/CLAUDE.md

## 2. 아키텍처 개요

### 전체 구조
Qt Widgets 기반의 NavigationBar 컴포넌트로 브라우저 네비게이션 UI를 구현합니다. WebView의 히스토리 API와 연동하여 뒤로/앞으로/새로고침/홈 버튼 기능을 제공하고, webOS 리모컨 입력에 최적화된 포커스 네비게이션을 구현합니다.

```
┌───────────────────────────────────────────────────────────────┐
│                  BrowserWindow (QMainWindow)                   │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │                  URLBar (F-03)                            │ │
│  └──────────────────────────────────────────────────────────┘ │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │              NavigationBar (QWidget)                      │ │
│  │  ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐            │ │
│  │  │  Back  │ │Forward │ │ Reload │ │  Home  │  QPushButton│ │
│  │  │   ←    │ │   →    │ │   ↻    │ │   ⌂    │            │ │
│  │  └────────┘ └────────┘ └────────┘ └────────┘            │ │
│  │  [ QHBoxLayout ]                                          │ │
│  └──────────────────────────────────────────────────────────┘ │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │                  WebView (QWidget)                        │ │
│  │  - canGoBack(), canGoForward()                            │ │
│  │  - goBack(), goForward(), reload()                        │ │
│  │  - signals: urlChanged(), loadStarted(), loadFinished()   │ │
│  └──────────────────────────────────────────────────────────┘ │
└───────────────────────────────────────────────────────────────┘

리모컨 방향키 (Qt::Key_Left, Qt::Key_Right)
    ↓
Qt Focus Chain (setTabOrder)
    ↓
버튼 포커스 이동 (흰색 테두리)
    ↓
리모컨 선택 (Qt::Key_Enter, Qt::Key_Select)
    ↓
버튼 클릭 → WebView API 호출

WebView::urlChanged(QUrl)
    ↓
NavigationBar::updateButtonStates()
    ↓
canGoBack()/canGoForward() 체크
    ↓
setEnabled(bool) → 버튼 활성/비활성 업데이트
```

### 핵심 설계 원칙
1. **Qt Widgets 표준 사용**: QPushButton, QHBoxLayout으로 간단한 UI 구성
2. **시그널/슬롯 연동**: WebView의 시그널로 버튼 상태 자동 동기화
3. **포커스 기반 네비게이션**: Qt Focus Policy와 Tab Order로 리모컨 입력 처리
4. **스마트 포인터 불필요**: 버튼은 QWidget의 자식으로 자동 메모리 관리
5. **중앙 집중 키 이벤트**: BrowserWindow::keyPressEvent()에서 리모컨 Back 키 처리

## 3. 아키텍처 결정

### 결정 1: 버튼 위젯 타입
- **선택지**:
  - A) QPushButton (범용 버튼)
  - B) QToolButton (아이콘 전용 버튼, 툴바 용도)
  - C) 커스텀 QWidget (직접 페인팅)
- **결정**: A) QPushButton
- **근거**:
  - **표준 동작**: QPushButton은 Qt의 가장 기본적인 버튼 위젯으로 클릭, 포커스, 키보드 이벤트를 모두 지원
  - **스타일시트 지원**: QSS로 쉽게 스타일링 가능 (포커스 테두리, 비활성 상태 등)
  - **아이콘 + 텍스트**: setIcon(), setText()로 아이콘과 라벨 모두 설정 가능 (향후 확장성)
  - **프로젝터 UI**: 큰 버튼이 필요한 프로젝터 환경에서 QPushButton이 적합 (QToolButton은 작은 아이콘용)
- **트레이드오프**:
  - **포기**: QToolButton의 툴바 특화 기능 (팝업 메뉴, 딜레이 등) 불필요
  - **얻음**: 단순한 구조, 명확한 의도

### 결정 2: 버튼 레이아웃
- **선택지**:
  - A) QHBoxLayout (가로 배치)
  - B) QGridLayout (2x2 그리드)
  - C) QToolBar (툴바 컨테이너)
- **결정**: A) QHBoxLayout
- **근거**:
  - **단순성**: 4개 버튼을 좌→우로 일렬 배치가 가장 직관적
  - **리모컨 포커스**: 좌/우 방향키만으로 모든 버튼 접근 가능 (2차원 네비게이션 불필요)
  - **PRD 요구사항**: "리모컨 최적화 UX"는 단순한 포커스 흐름을 요구
  - **확장성**: spacing, stretch로 간격 조정 용이
- **트레이드오프**: 그리드 레이아웃의 공간 활용도는 떨어지지만, UX는 우수

### 결정 3: 버튼 상태 동기화 메커니즘
- **선택지**:
  - A) WebView::urlChanged() 시그널에 슬롯 연결
  - B) 주기적 폴링 (QTimer로 100ms마다 확인)
  - C) WebView에 별도 시그널 추가 (historyChanged())
- **결정**: A) WebView::urlChanged() 시그널 사용
- **근거**:
  - **이미 구현됨**: WebView.h에 urlChanged(QUrl) 시그널이 존재 (F-02 완료)
  - **즉시 반응**: URL 변경 시 즉시 버튼 상태 업데이트 (폴링보다 효율적)
  - **표준 Qt 패턴**: 시그널/슬롯은 Qt의 표준 이벤트 메커니즘
  - **추가 구현 불필요**: WebView 수정 없이 기존 API 활용
- **트레이드오프**: urlChanged() 외에 loadStarted(), loadFinished()도 연결하여 더 정확한 상태 업데이트 가능

### 결정 4: 홈페이지 URL 관리
- **선택지**:
  - A) 하드코딩 (private const QString)
  - B) SettingsService에서 조회 (F-11 의존)
  - C) 환경 변수 또는 설정 파일 읽기
- **결정**: A) 하드코딩 (현 단계), B) 마이그레이션 계획 (F-11)
- **근거**:
  - **현 단계 범위**: F-04는 F-11(설정 화면)에 의존하지 않음 (features.md 확인)
  - **기본값 제공**: "https://www.google.com"을 private 멤버로 정의
  - **향후 확장**: F-11 구현 시 SettingsService::getHomePage()로 교체
- **구현 방식**:
  ```cpp
  // NavigationBar.h
  private:
      const QString DEFAULT_HOME_URL = "https://www.google.com";

  // F-11 구현 후:
  QString homeUrl = settingsService_->getHomePage();
  webView_->load(homeUrl);
  ```
- **트레이드오프**: 초기 단계는 유연성 부족, 하지만 의존성 제거로 개발 속도 향상

### 결정 5: 리모컨 Back 키 처리 위치
- **선택지**:
  - A) NavigationBar::keyPressEvent() 오버라이드
  - B) BrowserWindow::keyPressEvent() 중앙 처리
  - C) Qt Event Filter 설치
- **결정**: B) BrowserWindow::keyPressEvent() (F-02 설계서 따름)
- **근거**:
  - **중앙 집중**: 리모컨 키 이벤트는 BrowserWindow 레벨에서 포커스 컨텍스트에 따라 분기 처리
  - **포커스 인식**: WebView에 포커스 있을 때만 goBack() 호출 (NavigationBar 포커스 시 event->ignore())
  - **F-02 설계서 결정**: "결정 5: 리모컨 키 이벤트 처리 - B) BrowserWindow에서 중앙 집중 처리"
- **구현 위치**:
  - **BrowserWindow.cpp**: keyPressEvent()에서 Qt::Key_Backspace 감지
  - **NavigationBar**: 버튼 클릭으로만 goBack() 호출 (키 이벤트 직접 처리 안 함)
- **트레이드오프**: NavigationBar의 자율성은 떨어지지만, 전체 포커스 흐름이 명확

### 결정 6: 비활성 버튼 스타일
- **선택지**:
  - A) Qt 기본 disabled 스타일 (OS 의존)
  - B) 커스텀 QSS 스타일시트 (opacity: 0.5)
  - C) 아이콘 교체 (그레이 아이콘)
- **결정**: B) 커스텀 QSS 스타일시트 + A) 기본 스타일 보조
- **근거**:
  - **일관성**: webOS 전체 앱에서 통일된 비활성 스타일 필요
  - **명확성**: opacity: 0.5로 비활성 상태가 직관적으로 보임
  - **WCAG 준수**: 배경색과 대비 4.5:1 유지하면서 비활성 표시
- **구현**:
  ```cpp
  QString styleSheet = R"(
      QPushButton {
          min-width: 100px;
          min-height: 80px;
          font-size: 16pt;
          background-color: #333333;
          color: white;
          border: 2px solid transparent;
      }
      QPushButton:focus {
          border: 2px solid white;
      }
      QPushButton:disabled {
          opacity: 0.5;
          color: #888888;
      }
  )";
  ```
- **트레이드오프**: Qt 기본 스타일보다 유지보수 부담 증가, 하지만 UX 품질 향상

### 결정 7: 버튼 아이콘 vs 유니코드 텍스트
- **선택지**:
  - A) QIcon (PNG/SVG 이미지 파일)
  - B) 유니코드 문자 (U+2190, U+21BB 등)
  - C) 아이콘 폰트 (Font Awesome 등)
- **결정**: B) 유니코드 문자 (초기 구현), A) 마이그레이션 계획
- **근거**:
  - **단순성**: 이미지 파일 관리 불필요, 코드만으로 UI 완성
  - **확장성**: 유니코드 → QIcon 교체 시 setText() → setIcon()만 변경
  - **PRD 요구사항**: 아이콘 32px x 32px는 Qt 기본 폰트 크기로도 달성 가능
- **유니코드 심볼**:
  - 뒤로: ← (U+2190) 또는 ◀ (U+25C0)
  - 앞으로: → (U+2192) 또는 ▶ (U+25B6)
  - 새로고침: ↻ (U+21BB) 또는 ⟲ (U+27F2)
  - 홈: ⌂ (U+2302) 또는 🏠 (Emoji, webOS 지원 확인 필요)
- **F-15 구현 시**: resources/icons/ 폴더에 SVG 추가 후 QIcon::fromTheme() 사용
- **트레이드오프**: 프로덕션 품질은 이미지 아이콘이 우수, 하지만 PoC 단계는 유니코드로 충분

## 4. NavigationBar 클래스 설계

### 클래스 구조
```
src/ui/
├── NavigationBar.h       # 헤더 (기존 스켈레톤 파일 확장)
├── NavigationBar.cpp     # 구현
```

### NavigationBar.h (확장)
```cpp
/**
 * @file NavigationBar.h
 * @brief 네비게이션 바 컴포넌트 - 브라우저 네비게이션 버튼 UI
 */

#pragma once

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

namespace webosbrowser {

// Forward declaration
class WebView;

/**
 * @class NavigationBar
 * @brief 뒤로/앞으로/새로고침/홈 버튼을 제공하는 네비게이션 바
 *
 * WebView와 시그널/슬롯으로 연동하여 버튼 상태를 자동 동기화합니다.
 * Qt Focus Policy로 리모컨 방향키 네비게이션을 지원합니다.
 */
class NavigationBar : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param parent 부모 위젯
     */
    explicit NavigationBar(QWidget *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~NavigationBar() override;

    // 복사 생성자 및 대입 연산자 삭제
    NavigationBar(const NavigationBar&) = delete;
    NavigationBar& operator=(const NavigationBar&) = delete;

    /**
     * @brief WebView 인스턴스 설정 (시그널/슬롯 연결)
     * @param webView WebView 포인터 (nullptr 가능, 연결 해제 시)
     */
    void setWebView(WebView *webView);

public slots:
    /**
     * @brief 버튼 상태 업데이트 (WebView 히스토리 상태 기반)
     */
    void updateButtonStates();

private slots:
    /**
     * @brief 뒤로 버튼 클릭 핸들러
     */
    void onBackClicked();

    /**
     * @brief 앞으로 버튼 클릭 핸들러
     */
    void onForwardClicked();

    /**
     * @brief 새로고침 버튼 클릭 핸들러
     */
    void onReloadClicked();

    /**
     * @brief 홈 버튼 클릭 핸들러
     */
    void onHomeClicked();

private:
    /**
     * @brief UI 초기화 (버튼 생성, 레이아웃 설정)
     */
    void setupUI();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

    /**
     * @brief 스타일시트 적용 (QSS)
     */
    void applyStyles();

    /**
     * @brief 포커스 순서 설정 (리모컨 네비게이션)
     */
    void setupFocusOrder();

private:
    // UI 컴포넌트
    QHBoxLayout *layout_;           ///< 가로 레이아웃
    QPushButton *backButton_;       ///< 뒤로 버튼
    QPushButton *forwardButton_;    ///< 앞으로 버튼
    QPushButton *reloadButton_;     ///< 새로고침 버튼
    QPushButton *homeButton_;       ///< 홈 버튼

    // 데이터
    WebView *webView_;              ///< WebView 인스턴스 (약한 참조)
    const QString DEFAULT_HOME_URL = "https://www.google.com";  ///< 기본 홈페이지 URL
};

} // namespace webosbrowser
```

### NavigationBar.cpp (주요 메서드)

#### setupUI() - UI 초기화
```cpp
void NavigationBar::setupUI() {
    // 레이아웃 생성
    layout_ = new QHBoxLayout(this);
    layout_->setContentsMargins(10, 10, 10, 10);
    layout_->setSpacing(20);  // 버튼 간격 20px (NFR-2)

    // 버튼 생성
    backButton_ = new QPushButton("←", this);     // 뒤로 (U+2190)
    forwardButton_ = new QPushButton("→", this);  // 앞으로 (U+2192)
    reloadButton_ = new QPushButton("↻", this);   // 새로고침 (U+21BB)
    homeButton_ = new QPushButton("⌂", this);     // 홈 (U+2302)

    // 버튼 크기 설정 (NFR-2: 100x80px 이상)
    QSize minSize(100, 80);
    backButton_->setMinimumSize(minSize);
    forwardButton_->setMinimumSize(minSize);
    reloadButton_->setMinimumSize(minSize);
    homeButton_->setMinimumSize(minSize);

    // 포커스 정책 설정 (FR-5: 리모컨 포커스 네비게이션)
    backButton_->setFocusPolicy(Qt::StrongFocus);
    forwardButton_->setFocusPolicy(Qt::StrongFocus);
    reloadButton_->setFocusPolicy(Qt::StrongFocus);
    homeButton_->setFocusPolicy(Qt::StrongFocus);

    // 접근성 설정 (NFR-3)
    backButton_->setAccessibleName("뒤로 가기");
    backButton_->setAccessibleDescription("이전 페이지로 이동합니다");
    forwardButton_->setAccessibleName("앞으로 가기");
    forwardButton_->setAccessibleDescription("다음 페이지로 이동합니다");
    reloadButton_->setAccessibleName("새로고침");
    reloadButton_->setAccessibleDescription("현재 페이지를 다시 로드합니다");
    homeButton_->setAccessibleName("홈");
    homeButton_->setAccessibleDescription("홈페이지로 이동합니다");

    // 레이아웃에 버튼 추가
    layout_->addWidget(backButton_);
    layout_->addWidget(forwardButton_);
    layout_->addWidget(reloadButton_);
    layout_->addWidget(homeButton_);
    layout_->addStretch();  // 우측 공간 확보

    // 초기 상태: WebView 없으므로 뒤로/앞으로 비활성
    backButton_->setEnabled(false);
    forwardButton_->setEnabled(false);
}
```

#### setupConnections() - 시그널/슬롯 연결
```cpp
void NavigationBar::setupConnections() {
    // 버튼 클릭 시그널 연결
    connect(backButton_, &QPushButton::clicked, this, &NavigationBar::onBackClicked);
    connect(forwardButton_, &QPushButton::clicked, this, &NavigationBar::onForwardClicked);
    connect(reloadButton_, &QPushButton::clicked, this, &NavigationBar::onReloadClicked);
    connect(homeButton_, &QPushButton::clicked, this, &NavigationBar::onHomeClicked);
}
```

#### applyStyles() - QSS 스타일시트
```cpp
void NavigationBar::applyStyles() {
    // QSS 스타일 정의 (NFR-2: 리모컨 최적화 UX)
    QString styleSheet = R"(
        QPushButton {
            min-width: 100px;
            min-height: 80px;
            font-size: 32pt;          /* 아이콘 크기 (유니코드) */
            background-color: #333333; /* 어두운 배경 */
            color: white;
            border: 2px solid transparent;
            border-radius: 8px;
        }
        QPushButton:hover {
            background-color: #444444;
        }
        QPushButton:pressed {
            background-color: #555555;
        }
        QPushButton:focus {
            border: 2px solid white;  /* 포커스 테두리 (NFR-2) */
        }
        QPushButton:disabled {
            opacity: 0.5;              /* 비활성 버튼 (결정 6) */
            color: #888888;
        }
    )";

    this->setStyleSheet(styleSheet);
}
```

#### setupFocusOrder() - 포커스 순서 설정
```cpp
void NavigationBar::setupFocusOrder() {
    // Qt Focus Chain 설정 (FR-5: 좌→우 순서)
    QWidget::setTabOrder(backButton_, forwardButton_);
    QWidget::setTabOrder(forwardButton_, reloadButton_);
    QWidget::setTabOrder(reloadButton_, homeButton_);
}
```

#### setWebView() - WebView 연동
```cpp
void NavigationBar::setWebView(WebView *webView) {
    // 기존 WebView 연결 해제
    if (webView_) {
        disconnect(webView_, nullptr, this, nullptr);
    }

    webView_ = webView;

    // 새 WebView 시그널 연결 (FR-6: 버튼 상태 동기화)
    if (webView_) {
        connect(webView_, &WebView::urlChanged, this, &NavigationBar::updateButtonStates);
        connect(webView_, &WebView::loadStarted, this, &NavigationBar::updateButtonStates);
        connect(webView_, &WebView::loadFinished, this, &NavigationBar::updateButtonStates);

        // 초기 상태 업데이트
        updateButtonStates();
    } else {
        // WebView 없으면 모든 버튼 비활성
        backButton_->setEnabled(false);
        forwardButton_->setEnabled(false);
        reloadButton_->setEnabled(false);  // 새로고침도 비활성 (WebView 없음)
        homeButton_->setEnabled(false);    // 홈도 비활성 (WebView 없음)
    }
}
```

#### updateButtonStates() - 버튼 상태 업데이트
```cpp
void NavigationBar::updateButtonStates() {
    // nullptr 체크 (NFR-4: 신뢰성)
    if (!webView_) {
        backButton_->setEnabled(false);
        forwardButton_->setEnabled(false);
        return;
    }

    // WebView 히스토리 상태 조회 (FR-6: 버튼 상태 동기화)
    bool canGoBack = webView_->canGoBack();
    bool canGoForward = webView_->canGoForward();

    backButton_->setEnabled(canGoBack);
    forwardButton_->setEnabled(canGoForward);

    // 새로고침, 홈 버튼은 항상 활성 (FR-3, FR-4)
    reloadButton_->setEnabled(true);
    homeButton_->setEnabled(true);
}
```

#### 버튼 클릭 핸들러
```cpp
void NavigationBar::onBackClicked() {
    if (webView_ && webView_->canGoBack()) {
        webView_->goBack();  // FR-1: 뒤로 가기
        qDebug() << "[NavigationBar] Back button clicked, navigating to previous page";
    }
}

void NavigationBar::onForwardClicked() {
    if (webView_ && webView_->canGoForward()) {
        webView_->goForward();  // FR-2: 앞으로 가기
        qDebug() << "[NavigationBar] Forward button clicked, navigating to next page";
    }
}

void NavigationBar::onReloadClicked() {
    if (webView_) {
        webView_->reload();  // FR-3: 새로고침
        qDebug() << "[NavigationBar] Reload button clicked";
    }
}

void NavigationBar::onHomeClicked() {
    if (webView_) {
        webView_->load(DEFAULT_HOME_URL);  // FR-4: 홈페이지 로드
        qDebug() << "[NavigationBar] Home button clicked, loading" << DEFAULT_HOME_URL;
    }
}
```

## 5. BrowserWindow 통합

### BrowserWindow.h 수정
```cpp
// Forward declaration 추가
class NavigationBar;

// 멤버 변수 추가
private:
    NavigationBar *navBar_;  ///< 네비게이션 바
```

### BrowserWindow.cpp 수정

#### setupUI() - NavigationBar 추가
```cpp
void BrowserWindow::setupUI() {
    centralWidget_ = new QWidget(this);
    mainLayout_ = new QVBoxLayout(centralWidget_);

    // URLBar는 F-03에서 추가 예정

    // NavigationBar 생성 및 추가
    navBar_ = new NavigationBar(this);
    mainLayout_->addWidget(navBar_);

    // WebView 생성
    webView_ = new WebView(this);
    mainLayout_->addWidget(webView_, 1);  // stretch factor 1 (주요 영역)

    // StatusLabel
    statusLabel_ = new QLabel("Ready", this);
    mainLayout_->addWidget(statusLabel_);

    setCentralWidget(centralWidget_);
}
```

#### setupConnections() - NavigationBar ↔ WebView 연결
```cpp
void BrowserWindow::setupConnections() {
    // NavigationBar에 WebView 설정
    navBar_->setWebView(webView_);

    // WebView 시그널 → StatusLabel 업데이트 (기존 코드 유지)
    connect(webView_, &WebView::urlChanged, this, [this](const QUrl &url) {
        statusLabel_->setText(url.toString());
    });
}
```

#### keyPressEvent() - 리모컨 Back 키 처리 (FR-7)
```cpp
void BrowserWindow::keyPressEvent(QKeyEvent *event) {
    // 리모컨 Back 키 처리 (결정 5)
    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Escape) {
        // WebView에 포커스가 있을 때만 브라우저 뒤로 가기
        if (webView_->hasFocus() && webView_->canGoBack()) {
            webView_->goBack();
            qDebug() << "[BrowserWindow] Remote Back key pressed, navigating back";
            event->accept();
            return;
        } else {
            // NavigationBar나 URLBar에 포커스가 있을 때는 포커스 이탈
            event->ignore();
        }
    }

    // 기본 키 이벤트 처리 (Qt Focus Chain)
    QMainWindow::keyPressEvent(event);
}
```

## 6. 시퀀스 흐름

### 주요 시나리오: 뒤로 버튼 클릭
```
사용자 → 리모컨 → BrowserWindow → NavigationBar → WebView
  │      좌/우키      │                 │              │
  │  ───────────────▶ │                 │              │
  │                    │   Focus Chain   │              │
  │                    │  ─────────────▶ │              │
  │                    │                 │  (버튼 포커스)
  │      선택 키       │                 │              │
  │  ───────────────▶ │                 │              │
  │                    │                 │  clicked()   │
  │                    │                 │  ──────────▶ │
  │                    │                 │  onBackClicked()
  │                    │                 │  canGoBack()?
  │                    │                 │  ───────────▶
  │                    │                 │              │ true
  │                    │                 │              │ goBack()
  │                    │                 │              │ ────────▶
  │                    │                 │              │ (히스토리 스택 pop)
  │                    │                 │  urlChanged()│
  │                    │                 │ ◀────────────│
  │                    │  updateButtonStates()         │
  │                    │                 │ ────────────▶│
  │                    │                 │  canGoForward() = true
  │                    │                 │  setEnabled(true) on Forward
  │                    │                 │  (UI 업데이트)
```

### 시나리오: WebView URL 변경 → 버튼 상태 동기화
```
WebView → NavigationBar
  │            │
  │ urlChanged(QUrl)
  │ ──────────▶│
  │            │ updateButtonStates()
  │            │ ────────────────────▶
  │            │ canGoBack() 호출
  │ canGoBack()│
  │◀───────────│
  │ true/false │
  │───────────▶│
  │            │ setEnabled(bool) on Back Button
  │            │
  │            │ canGoForward() 호출
  │canGoForward()
  │◀───────────│
  │ true/false │
  │───────────▶│
  │            │ setEnabled(bool) on Forward Button
  │            │ (UI 즉시 업데이트)
```

### 에러 시나리오: WebView nullptr
```
NavigationBar::onBackClicked()
  │
  │ if (!webView_) → 조용히 리턴 (NFR-4)
  │ qDebug() << "WebView is null, ignoring click"
  │
  └─ (버튼 클릭 무시, 에러 메시지 없음)
```

### 시나리오: 리모컨 Back 키 → 브라우저 뒤로 가기
```
사용자 → BrowserWindow → WebView
  │ Back 키       │           │
  │ ────────────▶ │           │
  │               │ keyPressEvent(Key_Backspace)
  │               │ webView_->hasFocus()?
  │               │ ──────────▶
  │               │           │ true
  │               │ canGoBack()?
  │               │ ──────────▶
  │               │           │ true
  │               │ goBack()  │
  │               │ ──────────▶
  │               │           │ (히스토리 스택 pop)
  │               │ event->accept()
  │               │ (이벤트 소비, 앱 종료 방지)
```

## 7. 영향 범위 분석

### 수정 필요한 기존 파일
| 파일 경로 | 변경 내용 | 이유 |
|----------|----------|------|
| `src/browser/BrowserWindow.h` | - Forward declaration 추가: `class NavigationBar;`<br>- 멤버 변수 추가: `NavigationBar *navBar_;`<br>- keyPressEvent() 메서드 선언 추가 | NavigationBar 통합 |
| `src/browser/BrowserWindow.cpp` | - #include "ui/NavigationBar.h" 추가<br>- setupUI()에서 navBar_ 생성 및 레이아웃 추가<br>- setupConnections()에서 navBar_->setWebView() 호출<br>- keyPressEvent() 구현 (리모컨 Back 키) | NavigationBar 인스턴스 생성 및 연결 |

### 새로 생성할 파일
| 파일 경로 | 역할 | 크기 |
|----------|------|------|
| `src/ui/NavigationBar.cpp` | NavigationBar 클래스 구현 | ~300줄 |
| (NavigationBar.h는 이미 스켈레톤 존재) | 확장 필요 | 기존 파일 수정 |

### 영향 받는 기존 기능
| 기능명 | 영향 내용 | 대응 방안 |
|-------|----------|----------|
| F-02 (WebView 통합) | NavigationBar가 WebView의 시그널 구독 | 이미 구현된 시그널 활용, WebView 수정 불필요 |
| F-03 (URL 입력 UI) | BrowserWindow 레이아웃에 URLBar 추가 시 순서 고려 | URLBar → NavigationBar → WebView 순서로 배치 |
| F-06 (탭 관리) | 탭 전환 시 NavigationBar의 WebView 포인터 업데이트 | setWebView() 메서드 재호출로 대응 |

## 8. 기술적 주의사항

### 메모리 안전성
- **WebView 포인터**: NavigationBar는 WebView를 **약한 참조**(weak reference)로 관리
  - NavigationBar가 WebView를 소유하지 않음 (BrowserWindow가 소유)
  - WebView 소멸 시 NavigationBar는 nullptr 체크 필요
  - BrowserWindow 소멸 시 Qt parent-child 메커니즘으로 자동 정리

### 시그널/슬롯 연결 해제
- `setWebView(nullptr)` 호출 시 이전 WebView의 시그널 연결 해제 필수
  - 메모리 누수 방지 (WebView 소멸 시 댕글링 포인터)
  - `disconnect(webView_, nullptr, this, nullptr)` 사용

### 리모컨 Back 키 동작
- **WebView 포커스 시**: 브라우저 뒤로 가기 (goBack())
- **NavigationBar 포커스 시**: event->ignore()로 포커스 이탈
- **첫 페이지에서 Back 키**: canGoBack() == false → 이벤트 무시 (앱 종료 안 함)
  - F-15(즐겨찾기 홈 화면) 구현 시 홈 화면으로 이동하도록 변경 가능

### Qt 시그널/슬롯 성능
- WebView의 urlChanged() 시그널은 동기 호출 (Qt 기본)
  - updateButtonStates()는 즉시 실행 (0.1초 이내, NFR-1)
  - 버튼 상태 업데이트는 UI 스레드에서 처리 (Qt 이벤트 루프)

### 포커스 정책
- `Qt::StrongFocus`: 탭 키와 클릭 모두로 포커스 가능
- `setTabOrder()`: 명시적 포커스 순서 설정 (좌→우)
- **주의**: BrowserWindow::setupUI()에서 URLBar, NavigationBar, WebView 간 전체 포커스 체인 설정 필요

### QSS 스타일 우선순위
- 인라인 스타일 > 위젯 스타일시트 > 부모 스타일시트 > 앱 전역 스타일
- NavigationBar::applyStyles()는 버튼에 개별 스타일시트 설정
- BrowserWindow에서 전역 스타일 설정 시 우선순위 고려

## 9. 테스트 계획

### 단위 테스트 (Google Test + Qt Test)
```cpp
// tests/unit/NavigationBarTest.cpp

TEST_F(NavigationBarTest, InitialState) {
    NavigationBar navBar;
    // WebView 설정 전: 모든 버튼 비활성
    EXPECT_FALSE(navBar.backButton()->isEnabled());
    EXPECT_FALSE(navBar.forwardButton()->isEnabled());
}

TEST_F(NavigationBarTest, UpdateButtonStates) {
    NavigationBar navBar;
    MockWebView webView;

    EXPECT_CALL(webView, canGoBack()).WillOnce(Return(true));
    EXPECT_CALL(webView, canGoForward()).WillOnce(Return(false));

    navBar.setWebView(&webView);
    navBar.updateButtonStates();

    EXPECT_TRUE(navBar.backButton()->isEnabled());
    EXPECT_FALSE(navBar.forwardButton()->isEnabled());
}

TEST_F(NavigationBarTest, BackButtonClick) {
    NavigationBar navBar;
    MockWebView webView;

    EXPECT_CALL(webView, canGoBack()).WillOnce(Return(true));
    EXPECT_CALL(webView, goBack()).Times(1);

    navBar.setWebView(&webView);
    QTest::mouseClick(navBar.backButton(), Qt::LeftButton);
}

TEST_F(NavigationBarTest, NullWebViewSafety) {
    NavigationBar navBar;
    navBar.setWebView(nullptr);

    // 클릭 시 크래시 없이 조용히 무시
    EXPECT_NO_THROW(QTest::mouseClick(navBar.backButton(), Qt::LeftButton));
}
```

### 통합 테스트 (Qt Test)
```cpp
// tests/integration/BrowserWindowTest.cpp

TEST_F(BrowserWindowTest, NavigationBarIntegration) {
    BrowserWindow window;
    window.show();

    // 초기 로드
    window.loadUrl("https://example.com");
    QTest::qWait(2000);  // 로딩 대기

    // 뒤로 버튼 비활성 확인
    EXPECT_FALSE(window.navBar()->backButton()->isEnabled());

    // 링크 클릭 시뮬레이션
    window.webView()->load("https://example.com/page2");
    QTest::qWait(2000);

    // 뒤로 버튼 활성 확인
    EXPECT_TRUE(window.navBar()->backButton()->isEnabled());

    // 뒤로 버튼 클릭
    QTest::mouseClick(window.navBar()->backButton(), Qt::LeftButton);
    QTest::qWait(2000);

    // URL 확인
    EXPECT_EQ(window.webView()->url().toString(), "https://example.com");
}

TEST_F(BrowserWindowTest, RemoteBackKeyHandling) {
    BrowserWindow window;
    window.show();

    window.loadUrl("https://example.com");
    QTest::qWait(2000);
    window.webView()->load("https://example.com/page2");
    QTest::qWait(2000);

    // WebView에 포커스
    window.webView()->setFocus();

    // 리모컨 Back 키 시뮬레이션
    QTest::keyPress(&window, Qt::Key_Backspace);
    QTest::qWait(2000);

    // URL 확인
    EXPECT_EQ(window.webView()->url().toString(), "https://example.com");
}
```

### 수동 테스트 (LG 프로젝터 HU715QW)
| 테스트 케이스 | 절차 | 예상 결과 |
|-------------|------|----------|
| TC-1: 뒤로 버튼 동작 | 1. Google 로드<br>2. "YouTube" 검색<br>3. 뒤로 버튼 클릭 | Google 홈페이지로 복귀, 앞으로 버튼 활성화 |
| TC-2: 리모컨 포커스 이동 | 1. 리모컨 좌/우 방향키<br>2. 각 버튼 포커스 확인 | 흰색 테두리 표시, 순서대로 이동 |
| TC-3: 새로고침 동작 | 1. YouTube 로드<br>2. 새로고침 버튼 클릭 | 페이지 재로드, 로딩 인디케이터 표시 (F-05) |
| TC-4: 홈 버튼 동작 | 1. 임의 사이트 로드<br>2. 홈 버튼 클릭 | Google 홈페이지로 이동 |
| TC-5: 리모컨 Back 키 | 1. 여러 페이지 탐색<br>2. WebView 포커스<br>3. Back 키 | 브라우저 뒤로 가기 |
| TC-6: 비활성 버튼 스타일 | 1. 첫 페이지에서 뒤로 버튼 확인 | opacity 0.5, 클릭 안 됨 |

## 10. 성능 고려사항

### 메모리 사용
- NavigationBar: 4개 QPushButton + 1개 QHBoxLayout = 약 10KB
- QSS 스타일시트: 약 1KB (메모리 캐시)
- **총합**: 50KB 이하 (NFR-1 만족)

### 응답 시간
- 버튼 클릭 → WebView API 호출: Qt 시그널/슬롯 = 약 0.01ms
- WebView::goBack() → 페이지 로드 시작: Qt WebEngineView = 약 50-100ms
- **총합**: 0.3초 이내 (NFR-1 만족)

### UI 업데이트
- WebView::urlChanged() → updateButtonStates(): 동기 호출 = 약 0.1ms
- QPushButton::setEnabled(): Qt 이벤트 루프 = 약 1ms
- **총합**: 0.1초 이내 (NFR-1 만족)

## 11. 향후 확장 계획

### F-11 (설정 화면) 연동
```cpp
// NavigationBar.cpp
void NavigationBar::onHomeClicked() {
    QString homeUrl = settingsService_->getHomePage();  // SettingsService 주입
    webView_->load(homeUrl);
}
```

### F-15 (즐겨찾기 홈 화면) 연동
- 첫 페이지에서 리모컨 Back 키 → 즐겨찾기 홈 화면 표시
- BrowserWindow::keyPressEvent()에서 분기 처리

### 아이콘 교체 (프로덕션)
```cpp
backButton_->setIcon(QIcon(":/icons/back.svg"));
backButton_->setIconSize(QSize(32, 32));
```

### 버튼 애니메이션 (선택 사항)
```cpp
// 클릭 시 버튼 회전 효과 (Qt Property Animation)
QPropertyAnimation *anim = new QPropertyAnimation(reloadButton_, "rotation");
anim->setDuration(300);
anim->setStartValue(0);
anim->setEndValue(360);
anim->start(QAbstractAnimation::DeleteWhenStopped);
```

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 (Native App 기반) | F-04 기능 설계, Web App에서 Native App으로 전환 |
| 2026-02-14 | 유니코드 아이콘 사용 결정 (결정 7) | 초기 구현 단순화, 향후 SVG 마이그레이션 계획 |
| 2026-02-14 | 홈페이지 URL 하드코딩 결정 (결정 4) | F-11 의존성 제거, SettingsService 없이 구현 가능 |
