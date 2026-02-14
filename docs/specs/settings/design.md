# F-11: 설정 화면 — 기술 설계서

## 1. 참조
- 요구사항 분석서: docs/specs/settings/requirements.md
- F-09 검색 엔진 통합 설계서: docs/specs/search-engine-integration/design.md
- F-07 북마크 관리 설계서: docs/specs/bookmark-management/design.md
- F-08 히스토리 관리 설계서: docs/specs/history-management/design.md
- F-13 리모컨 단축키 요구사항 분석서: docs/specs/remote-shortcuts/requirements.md
- CLAUDE.md: 프로젝트 규칙 및 코딩 컨벤션

---

## 2. 아키텍처 개요

### 컴포넌트 다이어그램

```
┌───────────────────────────────────────────────────────────────┐
│                        BrowserWindow                           │
│  ┌─────────────────┐  ┌──────────────────────────────────┐   │
│  │ NavigationBar   │  │    SettingsPanel (오버레이)       │   │
│  │  - 설정 버튼    │  │  ┌────────────────────────────┐  │   │
│  └─────────────────┘  │  │  검색 엔진 선택             │  │   │
│                       │  │  (QRadioButton Group)      │  │   │
│  ┌─────────────────┐  │  ├────────────────────────────┤  │   │
│  │ Menu 버튼       │  │  │  홈페이지 설정             │  │   │
│  │ (keyPressEvent) │──▶  │  (QLineEdit)               │  │   │
│  └─────────────────┘  │  ├────────────────────────────┤  │   │
│                       │  │  테마 선택                  │  │   │
│                       │  │  (QRadioButton Group)      │  │   │
│                       │  ├────────────────────────────┤  │   │
│                       │  │  브라우징 데이터 삭제       │  │   │
│                       │  │  (QPushButton × 2)         │  │   │
│                       │  └────────────────────────────┘  │   │
│                       └──────────────────────────────────┘   │
└───────────────────────────────────────────────────────────────┘
                                │
                    ┌───────────┴───────────┐
                    ▼                       ▼
        ┌────────────────────┐   ┌──────────────────────┐
        │  SettingsService   │   │  SearchEngine        │
        │  (설정 관리)        │   │  (정적 클래스)        │
        │  - searchEngine_   │──▶│  - getAllEngines()   │
        │  - homepage_       │   │  - setDefaultEngine()│
        │  - theme_          │   └──────────────────────┘
        └────────────────────┘
                    │
                    ▼
        ┌────────────────────┐   ┌──────────────────────┐
        │  StorageService    │   │  BookmarkService     │
        │  (LS2 API 래퍼)    │   │  HistoryService      │
        │  - putData()       │   │  - deleteAllData()   │
        │  - getData()       │   └──────────────────────┘
        └────────────────────┘
```

### 데이터 플로우 (설정 변경)

```
사용자 입력 (검색 엔진 선택: Naver)
      │
      ▼
┌─────────────────────┐
│ SettingsPanel::     │
│ onSearchEngine      │
│ RadioToggled()      │
└─────────────────────┘
      │
      ▼
┌─────────────────────┐
│ SettingsService::   │
│ setSearchEngine()   │
│ ("naver")           │
└─────────────────────┘
      │
      ├─ 메모리 캐시 업데이트 (searchEngine_ = "naver")
      │
      ├─ emit searchEngineChanged("naver")  ──▶  SearchEngine::setDefaultSearchEngine()
      │                                    └──▶  URLBar (검색 시 반영)
      │
      └─ StorageService::putData(DataKind::Settings, "browser_settings", json)
               │
               ▼
         LS2 API 비동기 저장
         (com.webos.service.db/put)
```

### 데이터 플로우 (앱 시작 시 설정 로드)

```
BrowserWindow::BrowserWindow()
      │
      ▼
SettingsService::loadSettings()
      │
      ▼
StorageService::getData(DataKind::Settings, "browser_settings")
      │
      ├─ 성공: JSON 파싱 → 메모리 캐시 초기화
      │         └─ emit settingsLoaded()
      │                  │
      │                  ├─ SearchEngine::setDefaultSearchEngine(engine)
      │                  ├─ NavigationBar::setHomepage(homepage)
      │                  └─ BrowserWindow::applyTheme(theme)
      │
      └─ 실패: 기본값 사용
               └─ searchEngine_ = "google"
               └─ homepage_ = "https://www.google.com"
               └─ theme_ = "dark"
```

---

## 3. 아키텍처 결정

### 결정 1: SettingsService 설계 방식
- **선택지**:
  - A) 싱글톤 패턴 (전역 접근)
  - B) BrowserWindow에서 소유하고 다른 컴포넌트에 주입
  - C) 정적 메서드만 사용 (SearchEngine 방식)
- **결정**: B) BrowserWindow에서 소유하고 의존성 주입
- **근거**:
  - SettingsService는 상태를 가짐 (searchEngine_, homepage_, theme_ 캐시)
  - 시그널/슬롯 기반 이벤트 통지 필요 (QObject 상속)
  - BrowserWindow는 이미 StorageService, BookmarkService 등을 소유하는 컨테이너 역할
  - 의존성 주입으로 테스트 용이성 확보 (Mock 주입 가능)
  - 싱글톤은 전역 상태 문제, 테스트 어려움
- **트레이드오프**:
  - 포기: 전역 접근의 편리함
  - 획득: 명확한 소유권, 생명주기 관리, 테스트 용이성

### 결정 2: 설정 저장 시점
- **선택지**:
  - A) 항목 변경 즉시 저장 (라디오 버튼 클릭 시)
  - B) "저장" 버튼 클릭 시 일괄 저장
  - C) 패널 닫기 시 자동 저장
- **결정**: A) 항목 변경 즉시 저장
- **근거**:
  - 모바일/TV UX 패턴 (Android Settings, Apple Settings)과 일치
  - 사용자가 변경 즉시 효과 확인 가능 (테마 변경 → 즉시 UI 업데이트)
  - "저장하지 않고 닫기" 개념이 없어 혼란 방지
  - LS2 API는 비동기이므로 성능 영향 미미
  - 에러 발생 시 즉시 피드백 가능
- **트레이드오프**:
  - 포기: 변경 취소 기능 (되돌리기 없음)
  - 획득: 단순한 UX, 즉각적 피드백, 사용자 혼란 감소

### 결정 3: 테마 시스템 구현 방식
- **선택지**:
  - A) QSS 파일 외부화 (resources/styles/dark.qss, light.qss)
  - B) C++ 코드에 QString으로 하드코딩
  - C) QPalette 사용 (Qt 기본 팔레트 변경)
- **결정**: A) QSS 파일 외부화 (QRC 리소스 시스템 사용)
- **근거**:
  - QSS는 CSS와 유사하여 디자이너 협업 용이
  - 런타임에 파일 로드로 테마 동적 전환 가능
  - C++ 코드에서 스타일 로직 분리 (관심사 분리)
  - QRC 리소스 시스템으로 바이너리에 임베드 (파일 경로 문제 없음)
  - QPalette는 제한적 (Qt Widgets 일부만 적용)
- **트레이드오프**:
  - 포기: 코드에서 스타일 직접 제어
  - 획득: 유지보수성, 확장성 (새 테마 추가 쉬움), 디자이너 협업

### 결정 4: SettingsPanel UI 타입
- **선택지**:
  - A) QDialog (모달 다이얼로그)
  - B) QWidget 오버레이 (BrowserWindow 위에 슬라이드 인)
  - C) 독립 창 (QMainWindow)
- **결정**: B) QWidget 오버레이
- **근거**:
  - BookmarkPanel, HistoryPanel과 일관된 UX (슬라이드 인/아웃 애니메이션)
  - QDialog는 모달로 백그라운드 블로킹, 리모컨 Back 키 처리 복잡
  - 독립 창은 TV/프로젝터 환경에서 부자연스러움
  - QPropertyAnimation으로 부드러운 슬라이드 애니메이션 구현 가능
  - BrowserWindow의 자식 위젯으로 생명주기 관리 단순
- **트레이드오프**:
  - 포기: QDialog의 간편한 모달 처리
  - 획득: 일관된 UX, 애니메이션 제어, 리모컨 키 처리 단순화

### 결정 5: 홈페이지 URL 설정 vs NavigationBar 연동
- **선택지**:
  - A) NavigationBar가 SettingsService를 직접 참조
  - B) 시그널/슬롯으로 간접 연동 (SettingsService → BrowserWindow → NavigationBar)
  - C) NavigationBar에 setHomepage() public 메서드 추가
- **결정**: B + C) 시그널/슬롯 + public 메서드
- **근거**:
  - SettingsService가 homepageChanged 시그널 발생
  - BrowserWindow에서 시그널 수신 후 NavigationBar::setHomepage() 호출
  - NavigationBar는 SettingsService 의존성 없음 (낮은 결합도)
  - 테스트 시 NavigationBar 단독 테스트 가능
  - NavigationBar의 현재 DEFAULT_HOME_URL 상수를 제거하고 동적 할당
- **트레이드오프**:
  - 포기: NavigationBar에서 설정 직접 읽기
  - 획득: 낮은 결합도, 단방향 데이터 플로우, 테스트 용이성

---

## 4. 클래스 설계

### 4.1 SettingsService (서비스 계층)

#### 파일 위치
- 헤더: `src/services/SettingsService.h`
- 구현: `src/services/SettingsService.cpp`

#### 클래스 인터페이스

```cpp
/**
 * @file SettingsService.h
 * @brief 브라우저 설정 관리 서비스
 */

#pragma once

#include <QObject>
#include <QString>

namespace webosbrowser {

class StorageService;

/**
 * @class SettingsService
 * @brief 브라우저 설정 관리 서비스
 *
 * 검색 엔진, 홈페이지, 테마 등의 설정을 LS2 API로 저장/로드합니다.
 * 설정 변경 시 시그널을 발생시켜 관련 컴포넌트에 통지합니다.
 */
class SettingsService : public QObject {
    Q_OBJECT

public:
    explicit SettingsService(StorageService *storageService, QObject *parent = nullptr);
    ~SettingsService() override;

    SettingsService(const SettingsService&) = delete;
    SettingsService& operator=(const SettingsService&) = delete;

    // 설정 로드/저장
    bool loadSettings();

    // Getters
    QString searchEngine() const;
    QString homepage() const;
    QString theme() const;

    // Setters (LS2 저장 포함)
    bool setSearchEngine(const QString &engineId);
    bool setHomepage(const QString &url);
    bool setTheme(const QString &themeId);

signals:
    void searchEngineChanged(const QString &engineId);
    void homepageChanged(const QString &url);
    void themeChanged(const QString &themeId);
    void settingsLoaded();
    void settingsLoadFailed(const QString &errorMessage);

private:
    bool saveToStorage();

    StorageService *storageService_;
    QString searchEngine_;
    QString homepage_;
    QString theme_;

    static constexpr const char* DEFAULT_SEARCH_ENGINE = "google";
    static constexpr const char* DEFAULT_HOMEPAGE = "https://www.google.com";
    static constexpr const char* DEFAULT_THEME = "dark";
    static constexpr const char* STORAGE_ID = "browser_settings";
    static constexpr const char* KEY_SEARCH_ENGINE = "searchEngine";
    static constexpr const char* KEY_HOMEPAGE = "homepage";
    static constexpr const char* KEY_THEME = "theme";
};

} // namespace webosbrowser
```

#### 주요 메서드 구현

```cpp
bool SettingsService::loadSettings() {
    if (!storageService_) {
        emit settingsLoadFailed("StorageService가 초기화되지 않았습니다.");
        return false;
    }

    QJsonObject data = storageService_->getData(DataKind::Settings, STORAGE_ID);

    if (data.isEmpty()) {
        // 첫 실행 또는 설정 없음 → 기본값 사용
        searchEngine_ = DEFAULT_SEARCH_ENGINE;
        homepage_ = DEFAULT_HOMEPAGE;
        theme_ = DEFAULT_THEME;
        saveToStorage();
        emit settingsLoaded();
        return true;
    }

    // JSON 파싱
    searchEngine_ = data.value(KEY_SEARCH_ENGINE).toString(DEFAULT_SEARCH_ENGINE);
    homepage_ = data.value(KEY_HOMEPAGE).toString(DEFAULT_HOMEPAGE);
    theme_ = data.value(KEY_THEME).toString(DEFAULT_THEME);

    emit settingsLoaded();
    return true;
}

bool SettingsService::setSearchEngine(const QString &engineId) {
    // 유효성 검증 (SearchEngine::getAllSearchEngines()로 확인)
    QList<SearchEngineInfo> engines = SearchEngine::getAllSearchEngines();
    bool valid = false;
    for (const auto &engine : engines) {
        if (engine.id == engineId) {
            valid = true;
            break;
        }
    }

    if (!valid) return false;

    searchEngine_ = engineId;
    SearchEngine::setDefaultSearchEngine(engineId);
    bool success = saveToStorage();
    if (success) {
        emit searchEngineChanged(engineId);
    }
    return success;
}

bool SettingsService::setHomepage(const QString &url) {
    // URL 검증 (URLValidator 사용)
    if (!URLValidator::isValid(url) && !url.startsWith("about:")) {
        return false;
    }

    // 위험한 프로토콜 차단
    if (url.contains("javascript:", Qt::CaseInsensitive) || url.startsWith("file://")) {
        return false;
    }

    homepage_ = url;
    bool success = saveToStorage();
    if (success) {
        emit homepageChanged(url);
    }
    return success;
}

bool SettingsService::setTheme(const QString &themeId) {
    if (themeId != "dark" && themeId != "light") {
        return false;
    }

    theme_ = themeId;
    bool success = saveToStorage();
    if (success) {
        emit themeChanged(themeId);
    }
    return success;
}

bool SettingsService::saveToStorage() {
    if (!storageService_) return false;

    QJsonObject data;
    data[KEY_SEARCH_ENGINE] = searchEngine_;
    data[KEY_HOMEPAGE] = homepage_;
    data[KEY_THEME] = theme_;

    return storageService_->putData(DataKind::Settings, STORAGE_ID, data);
}
```

### 4.2 SettingsPanel (UI 계층)

#### 파일 위치
- 헤더: `src/ui/SettingsPanel.h`
- 구현: `src/ui/SettingsPanel.cpp`

#### 클래스 인터페이스

```cpp
/**
 * @file SettingsPanel.h
 * @brief 설정 패널 UI
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QPropertyAnimation>

namespace webosbrowser {

class SettingsService;
class BookmarkService;
class HistoryService;

/**
 * @class SettingsPanel
 * @brief 설정 패널 UI (오버레이)
 *
 * 검색 엔진 선택, 홈페이지 설정, 테마 변경, 브라우징 데이터 삭제 UI 제공.
 * 리모컨 방향키로 항목 간 네비게이션, 슬라이드 인/아웃 애니메이션 지원.
 */
class SettingsPanel : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPanel(SettingsService *settingsService,
                           BookmarkService *bookmarkService,
                           HistoryService *historyService,
                           QWidget *parent = nullptr);
    ~SettingsPanel() override;

    SettingsPanel(const SettingsPanel&) = delete;
    SettingsPanel& operator=(const SettingsPanel&) = delete;

    void showPanel();
    void hidePanel();

signals:
    void panelClosed();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onSearchEngineRadioToggled(bool checked);
    void onHomepageEditingFinished();
    void onThemeRadioToggled(bool checked);
    void onClearBookmarksClicked();
    void onClearHistoryClicked();
    void onCloseButtonClicked();
    void onSettingsLoaded();

private:
    void setupUI();
    void setupConnections();
    void setupFocusOrder();
    void loadCurrentSettings();
    bool showConfirmDialog(const QString &message);
    void showToast(const QString &message);

    SettingsService *settingsService_;
    BookmarkService *bookmarkService_;
    HistoryService *historyService_;

    // UI 컴포넌트
    QVBoxLayout *mainLayout_;
    QWidget *contentWidget_;
    QVBoxLayout *contentLayout_;
    QFormLayout *formLayout_;
    QPushButton *closeButton_;

    // 검색 엔진 섹션
    QRadioButton *googleRadio_;
    QRadioButton *naverRadio_;
    QRadioButton *bingRadio_;
    QRadioButton *duckduckgoRadio_;
    QButtonGroup *searchEngineGroup_;

    // 홈페이지 섹션
    QLineEdit *homepageInput_;
    QLabel *homepageErrorLabel_;

    // 테마 섹션
    QRadioButton *darkThemeRadio_;
    QRadioButton *lightThemeRadio_;
    QButtonGroup *themeGroup_;

    // 브라우징 데이터 삭제 섹션
    QPushButton *clearBookmarksButton_;
    QPushButton *clearHistoryButton_;

    // 애니메이션
    QPropertyAnimation *slideAnimation_;
};

} // namespace webosbrowser
```

#### 주요 메서드 구현

```cpp
void SettingsPanel::showPanel() {
    show();
    raise();

    // 슬라이드 인 애니메이션 (오른쪽에서 왼쪽으로)
    QRect startGeometry(parentWidget()->width(), 0,
                        contentWidget_->width(), parentWidget()->height());
    QRect endGeometry(parentWidget()->width() - contentWidget_->width() - 20, 80,
                      contentWidget_->width(), parentWidget()->height() - 160);

    slideAnimation_->setStartValue(startGeometry);
    slideAnimation_->setEndValue(endGeometry);
    slideAnimation_->start();

    googleRadio_->setFocus();
}

void SettingsPanel::hidePanel() {
    QRect startGeometry = contentWidget_->geometry();
    QRect endGeometry(parentWidget()->width(), startGeometry.y(),
                      contentWidget_->width(), startGeometry.height());

    slideAnimation_->setStartValue(startGeometry);
    slideAnimation_->setEndValue(endGeometry);
    slideAnimation_->start();

    connect(slideAnimation_, &QPropertyAnimation::finished, this, [this]() {
        hide();
        emit panelClosed();
    }, Qt::SingleShotConnection);
}

void SettingsPanel::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Back) {
        hidePanel();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void SettingsPanel::onSearchEngineRadioToggled(bool checked) {
    if (!checked || !settingsService_) return;

    QString engineId;
    if (sender() == googleRadio_) engineId = "google";
    else if (sender() == naverRadio_) engineId = "naver";
    else if (sender() == bingRadio_) engineId = "bing";
    else if (sender() == duckduckgoRadio_) engineId = "duckduckgo";

    if (!engineId.isEmpty()) {
        settingsService_->setSearchEngine(engineId);
    }
}

void SettingsPanel::onHomepageEditingFinished() {
    if (!settingsService_) return;

    QString url = homepageInput_->text().trimmed();

    if (!URLValidator::isValid(url) && !url.startsWith("about:")) {
        homepageErrorLabel_->setText("유효하지 않은 URL입니다.");
        homepageErrorLabel_->setVisible(true);
        return;
    }

    bool success = settingsService_->setHomepage(url);
    if (success) {
        homepageErrorLabel_->setVisible(false);
        showToast("홈페이지가 설정되었습니다.");
    } else {
        homepageErrorLabel_->setText("설정 저장에 실패했습니다.");
        homepageErrorLabel_->setVisible(true);
    }
}

void SettingsPanel::onClearBookmarksClicked() {
    if (!bookmarkService_) return;

    bool confirmed = showConfirmDialog(
        "모든 북마크를 삭제하시겠습니까?\n이 작업은 되돌릴 수 없습니다."
    );
    if (!confirmed) return;

    bookmarkService_->getAllBookmarks([this](bool success, const QVector<Bookmark> &bookmarks) {
        if (!success) return;
        for (const auto &bookmark : bookmarks) {
            bookmarkService_->deleteBookmark(bookmark.id, [](bool) {});
        }
        showToast("북마크가 삭제되었습니다.");
    });
}
```

---

## 5. BrowserWindow 통합

### BrowserWindow.h 수정

```cpp
// Forward declarations에 추가
class SettingsService;
class SettingsPanel;

// private: 멤버 변수에 추가
SettingsService *settingsService_;
SettingsPanel *settingsPanel_;

// private slots: 에 추가
void onSearchEngineChanged(const QString &engineId);
void onHomepageChanged(const QString &url);
void onThemeChanged(const QString &themeId);

// private: 메서드에 추가
void applyTheme(const QString &themeId);
```

### BrowserWindow.cpp 수정

```cpp
BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent)
    , settingsService_(nullptr)
    , settingsPanel_(nullptr)
{
    storageService_ = new StorageService(this);
    storageService_->initialize();

    // SettingsService 생성
    settingsService_ = new SettingsService(storageService_, this);
    settingsService_->loadSettings();

    setupUI();

    bookmarkService_ = new BookmarkService(storageService_, this);
    historyService_ = new HistoryService(storageService_, this);

    // SettingsPanel 생성
    settingsPanel_ = new SettingsPanel(settingsService_, bookmarkService_,
                                       historyService_, this);

    setupConnections();

    // 초기 테마 적용
    applyTheme(settingsService_->theme());
}

void BrowserWindow::setupConnections() {
    // ... 기존 연결 ...

    if (settingsService_) {
        connect(settingsService_, &SettingsService::searchEngineChanged,
                this, &BrowserWindow::onSearchEngineChanged);
        connect(settingsService_, &SettingsService::homepageChanged,
                this, &BrowserWindow::onHomepageChanged);
        connect(settingsService_, &SettingsService::themeChanged,
                this, &BrowserWindow::onThemeChanged);
    }
}

void BrowserWindow::handleMenuButton(int keyCode) {
    Q_UNUSED(keyCode);
    if (settingsPanel_->isVisible()) {
        settingsPanel_->hidePanel();
    } else {
        settingsPanel_->showPanel();
    }
}

void BrowserWindow::onHomepageChanged(const QString &url) {
    if (navigationBar_) {
        navigationBar_->setHomepage(url);
    }
}

void BrowserWindow::onThemeChanged(const QString &themeId) {
    applyTheme(themeId);
}

void BrowserWindow::applyTheme(const QString &themeId) {
    QString qssFilePath = QString(":/styles/%1.qss").arg(themeId);
    QFile qssFile(qssFilePath);

    if (qssFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(qssFile.readAll());
        qApp->setStyleSheet(styleSheet);
    } else {
        qWarning() << "테마 파일 로드 실패:" << qssFilePath;
    }
}
```

---

## 6. NavigationBar 수정 (홈페이지 동적 설정)

### NavigationBar.h 수정

```cpp
public:
    void setHomepage(const QString &url);

private:
    QString homepage_;  // const QString DEFAULT_HOME_URL 삭제
```

### NavigationBar.cpp 수정

```cpp
NavigationBar::NavigationBar(QWidget *parent)
    : QWidget(parent)
    , webView_(nullptr)
    , homepage_("https://www.google.com")  // 기본값
{
    setupUI();
    setupConnections();
    applyStyles();
    setupFocusOrder();
}

void NavigationBar::setHomepage(const QString &url) {
    homepage_ = url;
}

void NavigationBar::onHomeClicked() {
    if (webView_) {
        webView_->load(QUrl(homepage_));  // 동적 홈페이지 사용
    }
}
```

---

## 7. 테마 시스템 (QSS 파일)

### 디렉토리 구조

```
resources/
├── styles/
│   ├── dark.qss
│   └── light.qss
└── resources.qrc
```

### resources/styles/dark.qss

```css
/* 다크 모드 */

QMainWindow {
    background-color: #1E1E1E;
}

QWidget#NavigationBar, QWidget#URLBar {
    background-color: #2D2D2D;
    border-bottom: 1px solid #444444;
}

QPushButton {
    background-color: #3C3C3C;
    color: #FFFFFF;
    border: 1px solid #555555;
    border-radius: 4px;
    padding: 8px 16px;
    font-size: 14px;
}

QPushButton:hover {
    background-color: #4A4A4A;
}

QPushButton:focus {
    border: 2px solid #0078D7;
    outline: none;
}

QPushButton:disabled {
    background-color: #2D2D2D;
    color: #888888;
}

QLineEdit {
    background-color: #2D2D2D;
    color: #FFFFFF;
    border: 1px solid #555555;
    border-radius: 4px;
    padding: 6px 10px;
}

QLineEdit:focus {
    border: 2px solid #0078D7;
}

QRadioButton {
    color: #FFFFFF;
}

QRadioButton::indicator:checked {
    background-color: #0078D7;
    border: 2px solid #0078D7;
    border-radius: 8px;
}

QRadioButton::indicator:unchecked {
    background-color: transparent;
    border: 2px solid #888888;
    border-radius: 8px;
}

QLabel {
    color: #FFFFFF;
}

QListWidget {
    background-color: #2D2D2D;
    color: #FFFFFF;
    border: 1px solid #444444;
}

QListWidget::item:selected {
    background-color: #0078D7;
}
```

### resources/styles/light.qss

```css
/* 라이트 모드 */

QMainWindow {
    background-color: #FFFFFF;
}

QWidget#NavigationBar, QWidget#URLBar {
    background-color: #F5F5F5;
    border-bottom: 1px solid #CCCCCC;
}

QPushButton {
    background-color: #E0E0E0;
    color: #000000;
    border: 1px solid #CCCCCC;
    border-radius: 4px;
    padding: 8px 16px;
}

QPushButton:hover {
    background-color: #D0D0D0;
}

QPushButton:focus {
    border: 2px solid #0078D7;
}

QLineEdit {
    background-color: #FFFFFF;
    color: #000000;
    border: 1px solid #CCCCCC;
    border-radius: 4px;
    padding: 6px 10px;
}

QLineEdit:focus {
    border: 2px solid #0078D7;
}

QRadioButton {
    color: #000000;
}

QRadioButton::indicator:checked {
    background-color: #0078D7;
    border: 2px solid #0078D7;
    border-radius: 8px;
}

QLabel {
    color: #000000;
}

QListWidget {
    background-color: #FFFFFF;
    color: #000000;
    border: 1px solid #CCCCCC;
}

QListWidget::item:selected {
    background-color: #0078D7;
    color: #FFFFFF;
}
```

### resources/resources.qrc

```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource prefix="/styles">
        <file>styles/dark.qss</file>
        <file>styles/light.qss</file>
    </qresource>
</RCC>
```

---

## 8. CMakeLists.txt 업데이트

```cmake
# 소스 파일 추가
set(SOURCES
    # ... (기존 소스)
    src/services/SettingsService.cpp
    src/ui/SettingsPanel.cpp
)

# 헤더 파일 추가
set(HEADERS
    # ... (기존 헤더)
    src/services/SettingsService.h
    src/ui/SettingsPanel.h
)

# QRC 리소스 추가
set(RESOURCES
    resources/resources.qrc
)

# 실행 파일 생성 (RESOURCES 추가)
add_executable(webosbrowser ${SOURCES} ${HEADERS} ${RESOURCES})
```

---

## 9. 데이터 구조

### LS2 저장 형식

```json
{
  "_id": "browser_settings",
  "_kind": "com.jsong.webosbrowser:1.settings",
  "searchEngine": "google",
  "homepage": "https://www.google.com",
  "theme": "dark"
}
```

---

## 10. 시그널/슬롯 연결

```
SettingsPanel
      │
      │ onSearchEngineRadioToggled()
      └──▶ SettingsService::setSearchEngine()
                │
                ├─ 메모리 캐시 업데이트
                ├─ StorageService::putData()
                └─ emit searchEngineChanged()
                        │
                        ├──▶ SearchEngine::setDefaultSearchEngine()
                        └──▶ BrowserWindow::onSearchEngineChanged()

SettingsPanel
      │
      │ onHomepageEditingFinished()
      └──▶ SettingsService::setHomepage()
                │
                ├─ URLValidator::isValid()
                ├─ 메모리 캐시 업데이트
                ├─ StorageService::putData()
                └─ emit homepageChanged()
                        │
                        └──▶ BrowserWindow::onHomepageChanged()
                                  │
                                  └──▶ NavigationBar::setHomepage()

SettingsPanel
      │
      │ onThemeRadioToggled()
      └──▶ SettingsService::setTheme()
                │
                ├─ 메모리 캐시 업데이트
                ├─ StorageService::putData()
                └─ emit themeChanged()
                        │
                        └──▶ BrowserWindow::onThemeChanged()
                                  │
                                  └──▶ applyTheme()
                                        │
                                        └─ qApp->setStyleSheet(qss)
```

---

## 11. 리모컨 네비게이션

### Tab Order

```
googleRadio_ → naverRadio_ → bingRadio_ → duckduckgoRadio_
      │
      └─→ homepageInput_
               │
               └─→ darkThemeRadio_ → lightThemeRadio_
                        │
                        └─→ clearBookmarksButton_ → clearHistoryButton_
                                 │
                                 └─→ closeButton_
```

### 키 이벤트 처리

```cpp
void SettingsPanel::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Escape:
        case Qt::Key_Back:
            hidePanel();
            break;
        default:
            QWidget::keyPressEvent(event);  // Qt 기본 Tab Order 처리
            break;
    }
}
```

---

## 12. 에러 처리

### 설정 로드 실패
- LS2 API 에러 → 기본값 사용
- 기본값 즉시 저장하여 다음 로드 성공

### 잘못된 URL 입력
- URLValidator로 검증
- 에러 메시지: homepageErrorLabel_ 표시

### 브라우징 데이터 삭제 실패
- 비동기 콜백에서 에러 체크
- 실패 시 QMessageBox::critical

---

## 13. 테스트 시나리오

### 단위 테스트

```cpp
TEST(SettingsServiceTest, LoadDefaultSettings) {
    StorageService storage;
    SettingsService settings(&storage);
    EXPECT_TRUE(settings.loadSettings());
    EXPECT_EQ(settings.searchEngine(), "google");
}

TEST(SettingsServiceTest, SetSearchEngine) {
    StorageService storage;
    SettingsService settings(&storage);
    settings.loadSettings();
    EXPECT_TRUE(settings.setSearchEngine("naver"));
    EXPECT_EQ(settings.searchEngine(), "naver");
}

TEST(SettingsServiceTest, InvalidSearchEngine) {
    StorageService storage;
    SettingsService settings(&storage);
    settings.loadSettings();
    EXPECT_FALSE(settings.setSearchEngine("invalid"));
}
```

### E2E 테스트

1. Menu 버튼 → 설정 패널 표시
2. 검색 엔진 변경 → URLBar 검색 반영
3. 홈페이지 설정 → 홈 버튼 클릭 시 이동
4. 테마 변경 → UI 즉시 업데이트
5. 앱 재시작 → 설정 유지

---

## 14. 성능 최적화

### LS2 API 비동기 처리
- 메모리 캐시로 즉시 반영
- UI 블로킹 없음

### 테마 변경 최적화
- QRC 바이너리 임베드 (파일 I/O 없음)
- qApp->setStyleSheet() 빠름 (0.1초 미만)

### 슬라이드 애니메이션
- QPropertyAnimation (GPU 가속 가능)
- 300ms, OutCubic easing

---

## 15. 보안 고려사항

### 홈페이지 URL 검증
- javascript:, file://, data: 차단
- 허용: http://, https://, about:

### LS2 API 데이터 격리
- webOS 앱별 샌드박스
- 다른 앱 접근 불가

---

## 16. 확장성

### 새 설정 항목 추가
```cpp
// SettingsService에 메서드 추가
bool enableJavaScript() const;
bool setEnableJavaScript(bool enabled);

// SettingsPanel에 UI 추가
QCheckBox *enableJavaScriptCheckbox_;
```

### 새 테마 추가
1. `resources/styles/high-contrast.qss` 생성
2. QRC에 추가
3. SettingsPanel에 QRadioButton 추가

---

## 17. 알려진 제약사항

### QSS 제약
- QMessageBox 일부 속성 스타일링 불가
- QRadioButton::indicator 크기 고정

### webOS 리모컨 키 코드
- Menu 버튼 keyCode 기기별 다름 (457 또는 18)
- 둘 다 처리 필요

### LS2 API 비동기 지연
- 저장 완료 시점 보장 안 됨
- 메모리 캐시로 완화

---

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | F-11 설정 화면 기술 설계 (Native App) |
