# F-15: 즐겨찾기 홈 화면 — 기술 설계서

## 1. 참조
- **요구사항 분석서**: `docs/specs/favorites-home/requirements.md`
- **프로젝트**: webOS Browser Native (C++17, Qt 5.15+)

---

## 2. 아키텍처 개요

### 2.1 컴포넌트 다이어그램

```
┌────────────────────────────────────────────────────────┐
│                   BrowserWindow                        │
│  ┌──────────────────────────────────────────────────┐ │
│  │ QStackedLayout (stackedLayout_)                  │ │
│  │  ├─ WebView (기존)                               │ │
│  │  ├─ ErrorPage (기존)                             │ │
│  │  └─ HomePage (신규) ◀──────────┐                 │ │
│  └────────────────────────────────┼─────────────────┘ │
│                                   │                    │
│  NavigationBar (홈 버튼) ─────────┘                    │
│     │                                                  │
│     └─ SettingsService::homepage() 확인               │
└────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────┐
│              HomePage (QWidget)                      │
│  ┌────────────────────────────────────────────────┐ │
│  │ QVBoxLayout (mainLayout_)                      │ │
│  │  ├─ QLabel (타이틀: "즐겨찾기")                │ │
│  │  ├─ QScrollArea (contentScrollArea_)           │ │
│  │  │   └─ QWidget (contentContainer_)            │ │
│  │  │       ├─ QGridLayout (gridLayout_) [4열]    │ │
│  │  │       │   ├─ BookmarkCard #1                │ │
│  │  │       │   ├─ BookmarkCard #2                │ │
│  │  │       │   └─ ...                             │ │
│  │  │       └─ QWidget (emptyStateWidget_)        │ │
│  │  │           ├─ QLabel (아이콘 + 메시지)        │ │
│  │  │           └─ QPushButton (액션 버튼)        │ │
│  │  └─ QLabel (푸터: 단축키 안내)                  │ │
│  └────────────────────────────────────────────────┘ │
│                                                      │
│  BookmarkService* bookmarkService_                   │
└──────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────┐
│          BookmarkCard (QWidget)                      │
│  ┌────────────────────────────────────────────────┐ │
│  │ QVBoxLayout (layout_)                          │ │
│  │  ├─ QLabel (faviconLabel_) [64×64px]          │ │
│  │  ├─ QLabel (titleLabel_) [Bold 20px]          │ │
│  │  └─ QLabel (urlLabel_) [14px, Optional]       │ │
│  └────────────────────────────────────────────────┘ │
│                                                      │
│  포커스 시 2px 파란 테두리 (#0078D7)                  │
└──────────────────────────────────────────────────────┘
```

### 2.2 데이터 플로우

#### 홈 버튼 클릭 플로우
```
사용자 리모컨 홈 버튼
    │
    ↓
NavigationBar::onHomeClicked()
    │
    ↓
SettingsService::homepage() == "about:favorites" ?
    │
    ├─ YES → emit homeRequested("about:favorites")
    │           │
    │           ↓
    │       BrowserWindow::onHomeRequested(url)
    │           │
    │           ↓
    │       BrowserWindow::showHomePage()
    │           ├─ stackedLayout_->setCurrentWidget(homePage_)
    │           ├─ urlBar_->setText("about:favorites")
    │           └─ homePage_->refreshBookmarks()
    │                   │
    │                   ↓
    │               BookmarkService::getAllBookmarks()
    │                   │
    │                   ↓
    │               HomePage::onBookmarksLoaded()
    │                   │
    │                   ↓
    │               displayBookmarks() or displayEmptyState()
    │
    └─ NO → webView_->load(url)
```

#### 북마크 선택 플로우
```
사용자 Enter 키
    │
    ↓
BookmarkCard::keyPressEvent(Enter)
    │
    ↓
emit clicked(url)
    │
    ↓
HomePage::onCardClicked(url)
    │
    ↓
emit bookmarkSelected(url)
    │
    ↓
BrowserWindow::onBookmarkSelected(url)
    │
    ↓
hideHomePage() → webView_->load(url)
```

---

## 3. 클래스 설계

### 3.1 HomePage (메인 홈 화면)

```cpp
/**
 * @file HomePage.h
 * @brief 즐겨찾기 홈 화면 컴포넌트
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QVector>
#include <QKeyEvent>
#include "../models/Bookmark.h"

namespace webosbrowser {

class BookmarkService;
class BookmarkCard;

/**
 * @class HomePage
 * @brief 즐겨찾기 홈 화면 (about:favorites)
 *
 * 북마크를 4열 그리드로 표시하며 리모컨 방향키로 탐색합니다.
 * 북마크 0개 시 빈 상태 UI를 표시합니다.
 */
class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(BookmarkService *bookmarkService, QWidget *parent = nullptr);
    ~HomePage() override;

    HomePage(const HomePage&) = delete;
    HomePage& operator=(const HomePage&) = delete;

    /**
     * @brief 북마크 목록 새로고침
     *
     * BookmarkService에서 최신 데이터를 조회하여 그리드를 재구성합니다.
     */
    void refreshBookmarks();

signals:
    /**
     * @brief 북마크 선택 시그널
     * @param url 선택된 북마크 URL
     */
    void bookmarkSelected(const QString &url);

    /**
     * @brief 북마크 추가 요청 시그널 (Red 버튼)
     */
    void bookmarkAddRequested();

    /**
     * @brief 설정 요청 시그널 (Menu 버튼)
     */
    void settingsRequested();

protected:
    /**
     * @brief 키 입력 이벤트 (리모컨 단축키)
     * @param event 키 이벤트
     *
     * 방향키: 그리드 포커스 이동
     * Enter: 북마크 선택
     * Back/Escape: 이전 페이지
     * Red(0x193): 북마크 추가
     * Menu(Qt::Key_Menu): 설정
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 위젯 표시 이벤트
     * @param event 표시 이벤트
     *
     * HomePage가 표시될 때마다 자동으로 refreshBookmarks() 호출
     */
    void showEvent(QShowEvent *event) override;

private slots:
    /**
     * @brief 북마크 추가됨 슬롯
     * @param bookmark 추가된 북마크
     */
    void onBookmarkAdded(const Bookmark &bookmark);

    /**
     * @brief 북마크 삭제됨 슬롯
     * @param id 북마크 ID
     */
    void onBookmarkDeleted(const QString &id);

    /**
     * @brief 북마크 업데이트됨 슬롯
     * @param id 북마크 ID
     */
    void onBookmarkUpdated(const QString &id);

    /**
     * @brief 북마크 데이터 로드 완료 슬롯
     * @param success 성공 여부
     * @param bookmarks 북마크 리스트
     */
    void onBookmarksLoaded(bool success, const QVector<Bookmark> &bookmarks);

    /**
     * @brief BookmarkCard 클릭 슬롯
     * @param url 북마크 URL
     */
    void onCardClicked(const QString &url);

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

    /**
     * @brief 스타일시트 적용
     */
    void applyStyles();

    /**
     * @brief 그리드 뷰 재구성
     *
     * 기존 카드를 모두 삭제하고 bookmarks_를 기반으로 새 카드 생성
     */
    void rebuildGrid();

    /**
     * @brief 북마크 표시 (그리드 뷰)
     * @param bookmarks 표시할 북마크 목록 (최대 12개)
     */
    void displayBookmarks(const QVector<Bookmark> &bookmarks);

    /**
     * @brief 빈 상태 UI 표시
     */
    void displayEmptyState();

    /**
     * @brief 그리드 포커스 이동 (왼쪽)
     */
    void moveFocusLeft();

    /**
     * @brief 그리드 포커스 이동 (오른쪽)
     */
    void moveFocusRight();

    /**
     * @brief 그리드 포커스 이동 (위)
     */
    void moveFocusUp();

    /**
     * @brief 그리드 포커스 이동 (아래)
     */
    void moveFocusDown();

    /**
     * @brief 현재 포커스된 카드 활성화 (Enter 키)
     */
    void activateCurrentCard();

    // 서비스
    BookmarkService *bookmarkService_;

    // UI 컴포넌트
    QVBoxLayout *mainLayout_;             // 메인 레이아웃
    QLabel *titleLabel_;                  // 타이틀 "즐겨찾기"
    QScrollArea *contentScrollArea_;      // 스크롤 영역 (M4 대비)
    QWidget *contentContainer_;           // 스크롤 컨테이너
    QGridLayout *gridLayout_;             // 4열 그리드
    QWidget *emptyStateWidget_;           // 빈 상태 UI
    QLabel *footerLabel_;                 // 푸터 (단축키 안내)

    // 북마크 카드 목록
    QVector<BookmarkCard*> cards_;

    // 데이터 캐시
    QVector<Bookmark> bookmarks_;

    // 포커스 관리
    int currentFocusIndex_;

    // 상수
    static constexpr int MAX_BOOKMARKS = 12;  // 최대 표시 개수
    static constexpr int GRID_COLUMNS = 4;    // 그리드 열 개수
};

} // namespace webosbrowser
```

### 3.2 BookmarkCard (개별 북마크 카드)

```cpp
/**
 * @file BookmarkCard.h
 * @brief 홈 화면의 북마크 카드 UI
 */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>

namespace webosbrowser {

/**
 * @class BookmarkCard
 * @brief 홈 화면의 개별 북마크 카드
 *
 * 파비콘(기본 아이콘), 제목, URL을 표시하며
 * 클릭/Enter 키로 페이지를 엽니다.
 */
class BookmarkCard : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param id 북마크 ID
     * @param title 북마크 제목
     * @param url 북마크 URL
     * @param parent 부모 위젯
     */
    explicit BookmarkCard(const QString &id, const QString &title,
                          const QString &url, QWidget *parent = nullptr);
    ~BookmarkCard() override;

    BookmarkCard(const BookmarkCard&) = delete;
    BookmarkCard& operator=(const BookmarkCard&) = delete;

    /**
     * @brief 북마크 ID 조회
     */
    QString id() const { return id_; }

    /**
     * @brief 북마크 URL 조회
     */
    QString url() const { return url_; }

signals:
    /**
     * @brief 카드 클릭/Enter 키 시그널
     * @param url 북마크 URL
     */
    void clicked(const QString &url);

protected:
    /**
     * @brief 마우스 클릭 이벤트
     * @param event 마우스 이벤트
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 키 입력 이벤트 (Enter 키)
     * @param event 키 이벤트
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 포커스 인 이벤트
     * @param event 포커스 이벤트
     */
    void focusInEvent(QFocusEvent *event) override;

    /**
     * @brief 포커스 아웃 이벤트
     * @param event 포커스 이벤트
     */
    void focusOutEvent(QFocusEvent *event) override;

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 스타일 적용
     * @param focused 포커스 여부
     */
    void updateStyle(bool focused);

    /**
     * @brief 제목 텍스트를 말줄임표 처리
     * @param text 원본 텍스트
     * @param maxLength 최대 길이
     * @return 말줄임표 처리된 텍스트
     */
    QString elideText(const QString &text, int maxLength);

    // 데이터
    QString id_;
    QString title_;
    QString url_;

    // UI 컴포넌트
    QVBoxLayout *layout_;
    QLabel *faviconLabel_;  // 파비콘 (기본 아이콘)
    QLabel *titleLabel_;    // 제목
    QLabel *urlLabel_;      // URL (선택사항)

    // 상태
    bool isFocused_;
};

} // namespace webosbrowser
```

---

## 4. BrowserWindow 통합 설계

### 4.1 BrowserWindow 수정사항

#### 헤더 파일 변경 (BrowserWindow.h)
```cpp
class BrowserWindow : public QMainWindow {
    Q_OBJECT

public:
    // ... (기존 코드)

private slots:
    /**
     * @brief 홈 요청 핸들러 (NavigationBar에서 발생)
     * @param url 홈페이지 URL
     *
     * "about:favorites"이면 HomePage 표시, 아니면 WebView 로드
     */
    void onHomeRequested(const QString &url);

    /**
     * @brief HomePage에서 북마크 선택 핸들러
     * @param url 선택된 북마크 URL
     */
    void onBookmarkSelected(const QString &url);

    // ... (기존 슬롯)

private:
    /**
     * @brief HomePage 표시
     *
     * stackedLayout_에서 HomePage로 전환하고 URLBar 업데이트
     */
    void showHomePage();

    /**
     * @brief HomePage 숨김 (WebView로 전환)
     */
    void hideHomePage();

    // UI 컴포넌트
    // ... (기존 멤버)
    HomePage *homePage_;  // 홈 화면 컴포넌트 (신규)

    // ... (기존 멤버)
};
```

#### 구현 변경사항

1. **생성자에 HomePage 초기화 추가**
```cpp
BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent)
    // ... (기존 초기화)
    , homePage_(nullptr)
{
    // ... (기존 코드)

    // HomePage 생성 (BookmarkService 생성 후)
    homePage_ = new HomePage(bookmarkService_, contentWidget_);

    // ... (setupUI 호출)
}
```

2. **setupUI()에 HomePage 추가**
```cpp
void BrowserWindow::setupUI() {
    // ... (기존 코드)

    // stackedLayout_에 위젯 추가
    stackedLayout_->addWidget(webView_);      // index 0
    stackedLayout_->addWidget(errorPage_);    // index 1
    stackedLayout_->addWidget(homePage_);     // index 2 (신규)

    // 초기 WebView 표시
    stackedLayout_->setCurrentWidget(webView_);

    // ... (기존 코드)
}
```

3. **setupConnections()에 시그널 연결 추가**
```cpp
void BrowserWindow::setupConnections() {
    // ... (기존 연결)

    // NavigationBar 홈 요청 시그널 연결
    connect(navigationBar_, &NavigationBar::homeRequested,
            this, &BrowserWindow::onHomeRequested);

    // HomePage 시그널 연결
    connect(homePage_, &HomePage::bookmarkSelected,
            this, &BrowserWindow::onBookmarkSelected);
    connect(homePage_, &HomePage::bookmarkAddRequested,
            this, &BrowserWindow::onBookmarkButtonClicked);
    connect(homePage_, &HomePage::settingsRequested,
            this, [this]() {
                if (settingsPanel_) {
                    settingsPanel_->show();
                    settingsPanel_->raise();
                }
            });
}
```

4. **showHomePage() / hideHomePage() 구현**
```cpp
void BrowserWindow::showHomePage() {
    qDebug() << "BrowserWindow: HomePage 표시";

    // stackedLayout_에서 HomePage로 전환
    stackedLayout_->setCurrentWidget(homePage_);

    // URLBar 업데이트
    urlBar_->setText("about:favorites");

    // HomePage 새로고침
    homePage_->refreshBookmarks();
}

void BrowserWindow::hideHomePage() {
    qDebug() << "BrowserWindow: HomePage 숨김 → WebView 전환";

    // stackedLayout_에서 WebView로 전환
    stackedLayout_->setCurrentWidget(webView_);
}
```

5. **onHomeRequested() 구현**
```cpp
void BrowserWindow::onHomeRequested(const QString &url) {
    qDebug() << "BrowserWindow: 홈 요청 - URL:" << url;

    if (url == "about:favorites") {
        // 즐겨찾기 홈 화면 표시
        showHomePage();
    } else {
        // 일반 웹 페이지 로드
        hideHomePage();
        webView_->load(url);
    }
}
```

6. **onBookmarkSelected() 구현**
```cpp
void BrowserWindow::onBookmarkSelected(const QString &url) {
    qDebug() << "BrowserWindow: 북마크 선택 - URL:" << url;

    // HomePage 숨김 → WebView 전환
    hideHomePage();

    // 웹 페이지 로드
    webView_->load(url);

    // 방문 횟수 증가 (비동기)
    // TODO: Bookmark ID를 HomePage에서 전달받아 incrementVisitCount() 호출
}
```

### 4.2 NavigationBar 수정사항

#### 헤더 파일 변경 (NavigationBar.h)
```cpp
class NavigationBar : public QWidget {
    Q_OBJECT

public:
    // ... (기존 코드)

signals:
    /**
     * @brief 홈 요청 시그널
     * @param url 홈페이지 URL
     *
     * "about:favorites" 또는 설정된 홈페이지 URL
     */
    void homeRequested(const QString &url);

    // ... (기존 시그널)

private slots:
    /**
     * @brief 홈페이지 설정 변경 슬롯
     * @param url 새 홈페이지 URL
     */
    void onHomepageChanged(const QString &url);

    // ... (기존 슬롯)
};
```

#### 구현 변경사항 (NavigationBar.cpp)

1. **onHomeClicked() 수정**
```cpp
void NavigationBar::onHomeClicked() {
    qDebug() << "NavigationBar: 홈 버튼 클릭 - 홈페이지:" << homepage_;

    // 홈 요청 시그널 발생 (BrowserWindow에서 처리)
    emit homeRequested(homepage_);
}
```

2. **onHomepageChanged() 슬롯 추가**
```cpp
void NavigationBar::onHomepageChanged(const QString &url) {
    setHomepage(url);
}
```

3. **BrowserWindow에서 SettingsService 시그널 연결**
```cpp
// BrowserWindow::setupConnections()에 추가
connect(settingsService_, &SettingsService::homepageChanged,
        navigationBar_, &NavigationBar::onHomepageChanged);
```

---

## 5. 리모컨 네비게이션 설계

### 5.1 키 이벤트 처리 (HomePage::keyPressEvent)

```cpp
void HomePage::keyPressEvent(QKeyEvent *event) {
    int keyCode = event->key();
    qDebug() << "HomePage: 키 입력 - keyCode:" << keyCode;

    // 방향키 처리 (Qt 기본 포커스 시스템 대신 수동 관리)
    if (keyCode == Qt::Key_Left) {
        moveFocusLeft();
        event->accept();
        return;
    } else if (keyCode == Qt::Key_Right) {
        moveFocusRight();
        event->accept();
        return;
    } else if (keyCode == Qt::Key_Up) {
        moveFocusUp();
        event->accept();
        return;
    } else if (keyCode == Qt::Key_Down) {
        moveFocusDown();
        event->accept();
        return;
    }

    // Enter 키 (북마크 선택)
    if (keyCode == Qt::Key_Return || keyCode == Qt::Key_Enter) {
        activateCurrentCard();
        event->accept();
        return;
    }

    // Back 키 (이전 페이지)
    if (keyCode == Qt::Key_Escape || keyCode == Qt::Key_Back) {
        // BrowserWindow에서 처리하도록 이벤트 전파
        event->ignore();
        return;
    }

    // Red 버튼 (북마크 추가) - KeyCodeConstants::KEY_RED = 0x193
    if (keyCode == 0x193) {
        emit bookmarkAddRequested();
        event->accept();
        return;
    }

    // Menu 버튼 (설정)
    if (keyCode == Qt::Key_Menu) {
        emit settingsRequested();
        event->accept();
        return;
    }

    // 그 외 키는 부모로 전파
    QWidget::keyPressEvent(event);
}
```

### 5.2 그리드 포커스 이동 로직

```cpp
void HomePage::moveFocusLeft() {
    if (cards_.isEmpty()) return;

    int newIndex = currentFocusIndex_ - 1;
    if (newIndex < 0) {
        newIndex = 0;  // 최소값 제한
    }

    if (newIndex != currentFocusIndex_) {
        currentFocusIndex_ = newIndex;
        cards_[currentFocusIndex_]->setFocus();
    }
}

void HomePage::moveFocusRight() {
    if (cards_.isEmpty()) return;

    int newIndex = currentFocusIndex_ + 1;
    if (newIndex >= cards_.size()) {
        newIndex = cards_.size() - 1;  // 최대값 제한
    }

    if (newIndex != currentFocusIndex_) {
        currentFocusIndex_ = newIndex;
        cards_[currentFocusIndex_]->setFocus();
    }
}

void HomePage::moveFocusUp() {
    if (cards_.isEmpty()) return;

    int newIndex = currentFocusIndex_ - GRID_COLUMNS;
    if (newIndex < 0) {
        newIndex = 0;  // 최소값 제한
    }

    if (newIndex != currentFocusIndex_) {
        currentFocusIndex_ = newIndex;
        cards_[currentFocusIndex_]->setFocus();
    }
}

void HomePage::moveFocusDown() {
    if (cards_.isEmpty()) return;

    int newIndex = currentFocusIndex_ + GRID_COLUMNS;
    if (newIndex >= cards_.size()) {
        newIndex = cards_.size() - 1;  // 최대값 제한
    }

    if (newIndex != currentFocusIndex_) {
        currentFocusIndex_ = newIndex;
        cards_[currentFocusIndex_]->setFocus();
    }
}

void HomePage::activateCurrentCard() {
    if (cards_.isEmpty() || currentFocusIndex_ < 0 || currentFocusIndex_ >= cards_.size()) {
        return;
    }

    // 현재 포커스된 카드의 URL 전달
    QString url = cards_[currentFocusIndex_]->url();
    qDebug() << "HomePage: 북마크 활성화 - URL:" << url;
    emit bookmarkSelected(url);
}
```

---

## 6. UI 레이아웃 설계

### 6.1 HomePage 레이아웃 구조

```
┌────────────────────────────────────────────────────────┐
│ ┌────────────────────────────────────────────────────┐ │
│ │ "즐겨찾기" (QLabel, 32px Bold)                     │ │ ← titleLabel_
│ └────────────────────────────────────────────────────┘ │
│                                                          │
│ ┌────────────────────────────────────────────────────┐ │
│ │ QScrollArea (contentScrollArea_)                   │ │
│ │ ┌────────────────────────────────────────────────┐ │ │
│ │ │ QGridLayout (4열 × N행)                        │ │ │
│ │ │                                                 │ │ │
│ │ │  [Card1] [Card2] [Card3] [Card4]               │ │ │ ← Row 1
│ │ │  [Card5] [Card6] [Card7] [Card8]               │ │ │ ← Row 2
│ │ │  [Card9] [Card10] [Card11] [Card12]            │ │ │ ← Row 3
│ │ │                                                 │ │ │
│ │ └────────────────────────────────────────────────┘ │ │
│ └────────────────────────────────────────────────────┘ │
│                                                          │
│ ┌────────────────────────────────────────────────────┐ │
│ │ "Red: 북마크 추가 | Menu: 설정" (QLabel, 16px)    │ │ ← footerLabel_
│ └────────────────────────────────────────────────────┘ │
└────────────────────────────────────────────────────────┘
```

### 6.2 BookmarkCard 레이아웃 구조

```
┌─────────────────────────┐
│                         │
│     [Favicon Icon]      │  64×64px (기본 아이콘)
│                         │
│   사이트 제목 텍스트      │  20px Bold, 1줄 (ellipsis)
│   www.example.com       │  14px, 회색 (선택사항)
│                         │
└─────────────────────────┘
  200px × 180px
  Padding: 12px
```

### 6.3 빈 상태 레이아웃

```
┌────────────────────────────────────────────────────────┐
│                                                          │
│                                                          │
│                    [북마크 아이콘]                        │  128×128px
│                                                          │
│          북마크를 추가하여 자주 가는 사이트에              │  24px
│              빠르게 접근하세요                           │
│                                                          │
│          ┌──────────────────────────┐                   │
│          │ 북마크 추가 (Red 버튼)   │                   │  QPushButton
│          └──────────────────────────┘                   │
│                                                          │
│          ┌──────────────────────────┐                   │
│          │ 설정 (Menu 버튼)         │                   │  QPushButton
│          └──────────────────────────┘                   │
│                                                          │
└────────────────────────────────────────────────────────┘
```

---

## 7. 스타일링 설계 (QSS)

### 7.1 HomePage 스타일

```cpp
void HomePage::applyStyles() {
    QString styleSheet = R"(
        HomePage {
            background-color: #1E1E1E;  /* 다크 모드 배경 */
        }

        QLabel#titleLabel {
            color: #FFFFFF;
            font-size: 32px;
            font-weight: bold;
            padding: 20px;
        }

        QLabel#footerLabel {
            color: #888888;
            font-size: 16px;
            padding: 10px;
            background-color: #2A2A2A;
        }

        QScrollArea {
            border: none;
            background-color: transparent;
        }
    )";

    setStyleSheet(styleSheet);
}
```

### 7.2 BookmarkCard 스타일

```cpp
void BookmarkCard::updateStyle(bool focused) {
    QString styleSheet;

    if (focused) {
        styleSheet = R"(
            BookmarkCard {
                border: 2px solid #0078D7;  /* 파란색 테두리 */
                border-radius: 8px;
                padding: 10px;
                background-color: #3A3A3A;
            }
        )";
    } else {
        styleSheet = R"(
            BookmarkCard {
                border: 2px solid transparent;
                border-radius: 8px;
                padding: 10px;
                background-color: #2A2A2A;
            }
        )";
    }

    setStyleSheet(styleSheet);
}
```

### 7.3 빈 상태 스타일

```cpp
// emptyStateWidget_ 스타일
QString emptyStateStyle = R"(
    QWidget#emptyState {
        background-color: transparent;
    }

    QLabel#emptyIcon {
        qproperty-alignment: AlignCenter;
    }

    QLabel#emptyMessage {
        color: #888888;
        font-size: 24px;
        qproperty-alignment: AlignCenter;
    }

    QPushButton#actionButton {
        min-width: 300px;
        min-height: 60px;
        font-size: 18px;
        background-color: #0078D7;
        color: white;
        border: none;
        border-radius: 5px;
    }

    QPushButton#actionButton:focus {
        border: 3px solid #00AAFF;
    }
)";
```

---

## 8. 데이터 연동 설계

### 8.1 북마크 로드 플로우

```cpp
void HomePage::refreshBookmarks() {
    qDebug() << "HomePage: 북마크 새로고침";

    // BookmarkService에서 비동기 조회
    bookmarkService_->getAllBookmarks([this](bool success, const QVector<Bookmark> &bookmarks) {
        onBookmarksLoaded(success, bookmarks);
    });
}

void HomePage::onBookmarksLoaded(bool success, const QVector<Bookmark> &bookmarks) {
    if (!success) {
        qWarning() << "HomePage: 북마크 로드 실패";
        displayEmptyState();
        return;
    }

    qDebug() << "HomePage: 북마크 로드 성공 - 개수:" << bookmarks.size();

    // 데이터 캐시 업데이트
    bookmarks_ = bookmarks;

    // 최대 12개만 표시
    if (bookmarks_.size() > MAX_BOOKMARKS) {
        bookmarks_ = bookmarks_.mid(0, MAX_BOOKMARKS);
    }

    // 그리드 재구성
    if (bookmarks_.isEmpty()) {
        displayEmptyState();
    } else {
        displayBookmarks(bookmarks_);
    }
}
```

### 8.2 그리드 재구성 로직

```cpp
void HomePage::rebuildGrid() {
    qDebug() << "HomePage: 그리드 재구성";

    // 기존 카드 삭제
    for (BookmarkCard *card : cards_) {
        gridLayout_->removeWidget(card);
        card->deleteLater();
    }
    cards_.clear();

    // 빈 상태 위젯 숨김
    if (emptyStateWidget_) {
        emptyStateWidget_->hide();
    }

    // 북마크 없으면 빈 상태 표시
    if (bookmarks_.isEmpty()) {
        displayEmptyState();
        return;
    }

    // 새 카드 생성
    for (int i = 0; i < bookmarks_.size(); ++i) {
        const Bookmark &bookmark = bookmarks_[i];

        BookmarkCard *card = new BookmarkCard(
            bookmark.id, bookmark.title, bookmark.url, contentContainer_
        );

        // 시그널 연결
        connect(card, &BookmarkCard::clicked,
                this, &HomePage::onCardClicked);

        // 그리드에 추가 (4열)
        int row = i / GRID_COLUMNS;
        int col = i % GRID_COLUMNS;
        gridLayout_->addWidget(card, row, col);

        cards_.append(card);
    }

    // 첫 번째 카드에 포커스
    if (!cards_.isEmpty()) {
        currentFocusIndex_ = 0;
        cards_[0]->setFocus();
    }
}

void HomePage::displayBookmarks(const QVector<Bookmark> &bookmarks) {
    bookmarks_ = bookmarks;
    rebuildGrid();
}

void HomePage::displayEmptyState() {
    qDebug() << "HomePage: 빈 상태 표시";

    // 기존 카드 삭제
    for (BookmarkCard *card : cards_) {
        gridLayout_->removeWidget(card);
        card->deleteLater();
    }
    cards_.clear();

    // 빈 상태 위젯 표시
    if (emptyStateWidget_) {
        emptyStateWidget_->show();
    }
}
```

### 8.3 북마크 변경 자동 새로고침

```cpp
void HomePage::setupConnections() {
    // BookmarkService 시그널 연결
    connect(bookmarkService_, &BookmarkService::bookmarkAdded,
            this, &HomePage::onBookmarkAdded);
    connect(bookmarkService_, &BookmarkService::bookmarkDeleted,
            this, &HomePage::onBookmarkDeleted);
    connect(bookmarkService_, &BookmarkService::bookmarkUpdated,
            this, &HomePage::onBookmarkUpdated);
}

void HomePage::onBookmarkAdded(const Bookmark &bookmark) {
    Q_UNUSED(bookmark);
    qDebug() << "HomePage: 북마크 추가됨 - 새로고침";
    refreshBookmarks();
}

void HomePage::onBookmarkDeleted(const QString &id) {
    Q_UNUSED(id);
    qDebug() << "HomePage: 북마크 삭제됨 - 새로고침";
    refreshBookmarks();
}

void HomePage::onBookmarkUpdated(const QString &id) {
    Q_UNUSED(id);
    qDebug() << "HomePage: 북마크 업데이트됨 - 새로고침";
    refreshBookmarks();
}
```

---

## 9. 구현 상세 설계

### 9.1 HomePage 구현 핵심 로직

#### setupUI()
```cpp
void HomePage::setupUI() {
    // 메인 레이아웃
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(40, 40, 40, 40);
    mainLayout_->setSpacing(20);

    // 타이틀 라벨
    titleLabel_ = new QLabel("즐겨찾기", this);
    titleLabel_->setObjectName("titleLabel");
    titleLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout_->addWidget(titleLabel_);

    // 스크롤 영역 (M4 대비)
    contentScrollArea_ = new QScrollArea(this);
    contentScrollArea_->setWidgetResizable(true);
    contentScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    contentScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    contentScrollArea_->setFrameShape(QFrame::NoFrame);

    // 스크롤 컨테이너
    contentContainer_ = new QWidget();
    contentScrollArea_->setWidget(contentContainer_);

    // 그리드 레이아웃 (4열)
    gridLayout_ = new QGridLayout(contentContainer_);
    gridLayout_->setContentsMargins(0, 0, 0, 0);
    gridLayout_->setSpacing(16);  // 카드 간격 16px
    gridLayout_->setColumnStretch(0, 1);
    gridLayout_->setColumnStretch(1, 1);
    gridLayout_->setColumnStretch(2, 1);
    gridLayout_->setColumnStretch(3, 1);

    // 빈 상태 위젯 생성
    emptyStateWidget_ = createEmptyStateWidget();
    gridLayout_->addWidget(emptyStateWidget_, 0, 0, 1, 4);  // 4열 병합
    emptyStateWidget_->hide();  // 초기 숨김

    mainLayout_->addWidget(contentScrollArea_, 1);  // stretch=1

    // 푸터 라벨
    footerLabel_ = new QLabel("Red: 북마크 추가  |  Menu: 설정", this);
    footerLabel_->setObjectName("footerLabel");
    footerLabel_->setAlignment(Qt::AlignCenter);
    mainLayout_->addWidget(footerLabel_);

    // 포커스 정책
    setFocusPolicy(Qt::StrongFocus);
}
```

#### createEmptyStateWidget()
```cpp
QWidget* HomePage::createEmptyStateWidget() {
    QWidget *widget = new QWidget(contentContainer_);
    widget->setObjectName("emptyState");

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(30);

    // 아이콘 (기본 북마크 아이콘)
    QLabel *iconLabel = new QLabel(widget);
    iconLabel->setObjectName("emptyIcon");
    QPixmap iconPixmap(":/resources/icons/bookmark-empty.png");  // 128×128px
    iconLabel->setPixmap(iconPixmap);
    iconLabel->setFixedSize(128, 128);
    layout->addWidget(iconLabel);

    // 메시지
    QLabel *messageLabel = new QLabel(
        "북마크를 추가하여 자주 가는 사이트에\n빠르게 접근하세요",
        widget
    );
    messageLabel->setObjectName("emptyMessage");
    messageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(messageLabel);

    // 북마크 추가 버튼
    QPushButton *addButton = new QPushButton("북마크 추가 (Red 버튼)", widget);
    addButton->setObjectName("actionButton");
    addButton->setFocusPolicy(Qt::StrongFocus);
    connect(addButton, &QPushButton::clicked, this, [this]() {
        emit bookmarkAddRequested();
    });
    layout->addWidget(addButton);

    // 설정 버튼
    QPushButton *settingsButton = new QPushButton("홈페이지 설정 변경 (Menu 버튼)", widget);
    settingsButton->setObjectName("actionButton");
    settingsButton->setFocusPolicy(Qt::StrongFocus);
    connect(settingsButton, &QPushButton::clicked, this, [this]() {
        emit settingsRequested();
    });
    layout->addWidget(settingsButton);

    return widget;
}
```

### 9.2 BookmarkCard 구현 핵심 로직

#### setupUI()
```cpp
void BookmarkCard::setupUI() {
    // 메인 레이아웃
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(12, 12, 12, 12);
    layout_->setSpacing(8);
    layout_->setAlignment(Qt::AlignCenter);

    // 파비콘 (기본 아이콘)
    faviconLabel_ = new QLabel(this);
    QPixmap faviconPixmap(":/resources/icons/default-favicon.png");  // 64×64px
    faviconLabel_->setPixmap(faviconPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    faviconLabel_->setFixedSize(64, 64);
    faviconLabel_->setAlignment(Qt::AlignCenter);
    layout_->addWidget(faviconLabel_);

    // 제목
    titleLabel_ = new QLabel(elideText(title_, 20), this);
    titleLabel_->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFFFFF;");
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setWordWrap(false);
    layout_->addWidget(titleLabel_);

    // URL (선택사항, 초기 버전은 표시하지 않음)
    // urlLabel_ = new QLabel(elideText(url_, 30), this);
    // urlLabel_->setStyleSheet("font-size: 14px; color: #888888;");
    // urlLabel_->setAlignment(Qt::AlignCenter);
    // layout_->addWidget(urlLabel_);

    // 카드 크기 설정
    setFixedSize(200, 180);

    // 포커스 정책
    setFocusPolicy(Qt::StrongFocus);

    // 초기 스타일 적용
    updateStyle(false);
}
```

#### 이벤트 핸들러
```cpp
void BookmarkCard::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        qDebug() << "BookmarkCard: 마우스 클릭 - URL:" << url_;
        emit clicked(url_);
    }
    QWidget::mousePressEvent(event);
}

void BookmarkCard::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        qDebug() << "BookmarkCard: Enter 키 - URL:" << url_;
        emit clicked(url_);
        event->accept();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void BookmarkCard::focusInEvent(QFocusEvent *event) {
    isFocused_ = true;
    updateStyle(true);
    QWidget::focusInEvent(event);
}

void BookmarkCard::focusOutEvent(QFocusEvent *event) {
    isFocused_ = false;
    updateStyle(false);
    QWidget::focusOutEvent(event);
}
```

---

## 10. CMakeLists.txt 업데이트

```cmake
# 소스 파일 목록
set(SOURCES
    # ... (기존 소스)
    src/ui/HomePage.cpp          # 신규
    src/ui/BookmarkCard.cpp      # 신규
)

# 헤더 파일 목록
set(HEADERS
    # ... (기존 헤더)
    src/ui/HomePage.h            # 신규
    src/ui/BookmarkCard.h        # 신규
)
```

---

## 11. 아키텍처 결정 사항

### 결정 1: HomePage를 QStackedLayout에 통합

**선택지**:
- A) HomePage를 독립 윈도우로 구현 (QDialog)
- B) HomePage를 BrowserWindow의 QStackedLayout에 통합
- C) HomePage를 오버레이 위젯으로 구현

**결정**: **B) QStackedLayout 통합**

**근거**:
- WebView, ErrorPage와 동일한 패턴으로 일관성 유지
- 메모리 효율성 (단일 위젯 인스턴스)
- 전환 애니메이션 추가 용이 (M4)
- URL Bar에 "about:favorites" 표시 가능

**트레이드오프**:
- 얻은 것: 일관된 아키텍처, 메모리 효율
- 포기한 것: HomePage 독립 실행 (불필요한 기능)

### 결정 2: 그리드 포커스 수동 관리

**선택지**:
- A) Qt 기본 포커스 시스템 사용 (QGridLayout + Tab Order)
- B) 수동 포커스 관리 (keyPressEvent 오버라이드)
- C) QML ListView 사용

**결정**: **B) 수동 포커스 관리**

**근거**:
- 리모컨 방향키의 4열 그리드 동작을 정확히 제어
- Qt 기본 포커스 순서가 직관적이지 않음
- 포커스 인덱스를 명시적으로 관리하여 디버깅 용이

**트레이드오프**:
- 얻은 것: 리모컨 UX 최적화, 정확한 네비게이션
- 포기한 것: Qt 기본 시스템의 자동 처리

### 결정 3: 북마크 최대 12개 제한

**선택지**:
- A) 무제한 표시 + 스크롤
- B) 최대 12개 제한 + "더 보기" 버튼
- C) 최대 12개 제한 (M3 단순 버전)

**결정**: **C) 최대 12개 제한**

**근거**:
- 초기 버전 단순화 (M3 목표)
- 4×3 그리드가 1080p 화면에 최적
- 스크롤 UX는 M4에서 추가 가능

**트레이드오프**:
- 얻은 것: 단순한 구현, 빠른 렌더링
- 포기한 것: 12개 초과 북마크 표시 (M4로 연기)

### 결정 4: 파비콘 기본 아이콘 사용

**선택지**:
- A) 실제 파비콘 다운로드 (비동기)
- B) 기본 아이콘 사용
- C) 웹사이트 썸네일 사용

**결정**: **B) 기본 아이콘 사용**

**근거**:
- 초기 버전 단순화
- BookmarkService에 favicon 필드는 존재하나 현재 미사용
- 네트워크 의존성 제거 (오프라인 동작 보장)

**트레이드오프**:
- 얻은 것: 빠른 구현, 안정성
- 포기한 것: 시각적 차별화 (M4로 연기)

### 결정 5: 빈 상태 UI 필수 구현

**선택지**:
- A) 빈 상태 UI 생략 (빈 화면만 표시)
- B) 안내 메시지만 표시
- C) 안내 메시지 + 액션 버튼 표시

**결정**: **C) 안내 메시지 + 액션 버튼**

**근거**:
- 첫 사용자 경험 개선 (북마크 추가 유도)
- 홈페이지 설정 변경 유도 (일반 URL로 전환 가능)
- 요구사항 FR-4에서 Must 우선순위

**트레이드오프**:
- 얻은 것: 우수한 UX, 사용자 가이드
- 포기한 것: 약간의 구현 복잡도 증가

---

## 12. 시퀀스 다이어그램

### 시나리오 1: 홈 버튼 클릭 → 홈 화면 표시

```
사용자     NavigationBar    SettingsService    BrowserWindow    HomePage    BookmarkService
  │              │                  │                  │            │               │
  │  홈 버튼 클릭 │                  │                  │            │               │
  │─────────────▶│                  │                  │            │               │
  │              │ setHomepage()    │                  │            │               │
  │              │ (초기화 시 설정)  │                  │            │               │
  │              │                  │                  │            │               │
  │              │ onHomeClicked()  │                  │            │               │
  │              │──────────────────┼─────────────────▶│            │               │
  │              │ emit homeRequested("about:favorites")            │               │
  │              │                  │                  │            │               │
  │              │                  │ onHomeRequested()│            │               │
  │              │                  │                  │ url == "about:favorites" ? │
  │              │                  │                  │  ├─ YES    │               │
  │              │                  │                  │  │ showHomePage()          │
  │              │                  │                  │  │─────────▶               │
  │              │                  │                  │  │ refreshBookmarks()      │
  │              │                  │                  │  │          │──────────────▶
  │              │                  │                  │  │          │ getAllBookmarks()
  │              │                  │                  │  │          │               │
  │              │                  │                  │  │          │◀──────────────
  │              │                  │                  │  │          │ callback()    │
  │              │                  │                  │  │          │               │
  │              │                  │                  │  │ onBookmarksLoaded()     │
  │              │                  │                  │  │          │ displayBookmarks()
  │◀─────────────────────────────────────────────────────┴──────────┴───────────────
  │  (화면에 HomePage 표시)
```

### 시나리오 2: 북마크 선택 → 페이지 열기

```
사용자     HomePage     BookmarkCard    BrowserWindow    WebView
  │            │              │                │            │
  │ 방향키 이동 │              │                │            │
  │───────────▶│              │                │            │
  │            │ moveFocusRight()             │            │
  │            │ cards_[newIndex]->setFocus() │            │
  │            │─────────────▶│                │            │
  │            │              │ focusInEvent() │            │
  │            │              │ updateStyle(true)           │
  │            │              │ (테두리 강조)  │            │
  │            │              │                │            │
  │ Enter 키   │              │                │            │
  │───────────▶│              │                │            │
  │            │ activateCurrentCard()        │            │
  │            │ emit bookmarkSelected(url)   │            │
  │            │──────────────────────────────▶│            │
  │            │                               │ onBookmarkSelected()
  │            │                               │ hideHomePage()
  │            │                               │ load(url)  │
  │            │                               │───────────▶│
  │◀───────────────────────────────────────────────────────┴────────
  │  (WebView에 페이지 로딩 시작)
```

### 시나리오 3: 북마크 추가 → 자동 새로고침

```
사용자     HomePage     BrowserWindow    BookmarkPanel    BookmarkService
  │            │              │                │                  │
  │ Red 버튼   │              │                │                  │
  │───────────▶│              │                │                  │
  │            │ keyPressEvent(Red)           │                  │
  │            │ emit bookmarkAddRequested()  │                  │
  │            │─────────────▶│                │                  │
  │            │              │ onBookmarkButtonClicked()         │
  │            │              │ bookmarkPanel_->show()            │
  │            │              │────────────────▶                  │
  │◀──────────────────────────────────────────┘                  │
  │  (BookmarkPanel 슬라이드 인)                                  │
  │                                                               │
  │  (사용자가 북마크 추가 완료)                                   │
  │──────────────────────────────────────────────────────────────▶
  │                                            │ addBookmark()    │
  │                                            │ emit bookmarkAdded()
  │                                            │─────────────────▶│
  │            │ onBookmarkAdded()            │                  │
  │            │◀─────────────────────────────────────────────────
  │            │ refreshBookmarks()           │                  │
  │            │─────────────────────────────────────────────────▶
  │            │                              │ getAllBookmarks()│
  │            │◀─────────────────────────────────────────────────
  │            │ rebuildGrid()                │                  │
  │◀───────────┘                              │                  │
  │  (홈 화면에 새 북마크 카드 표시)
```

---

## 13. 에러 처리 설계

### 13.1 북마크 로드 실패

```cpp
void HomePage::onBookmarksLoaded(bool success, const QVector<Bookmark> &bookmarks) {
    if (!success) {
        qWarning() << "HomePage: 북마크 로드 실패";

        // 에러 메시지 표시 (빈 상태 위젯 재활용)
        if (emptyStateWidget_) {
            QLabel *messageLabel = emptyStateWidget_->findChild<QLabel*>("emptyMessage");
            if (messageLabel) {
                messageLabel->setText("북마크를 불러올 수 없습니다.\n잠시 후 다시 시도해주세요.");
            }
        }

        displayEmptyState();
        return;
    }

    // 정상 처리
    // ...
}
```

### 13.2 잘못된 URL 처리

```cpp
void BookmarkCard::setupUI() {
    // URL 검증 (URLValidator 사용)
    if (!URLValidator::isValid(url_)) {
        qWarning() << "BookmarkCard: 잘못된 URL - " << url_;

        // 경고 아이콘 표시 (파비콘 대신)
        QPixmap warningPixmap(":/resources/icons/warning.png");
        faviconLabel_->setPixmap(warningPixmap.scaled(64, 64));

        // 타이틀에 경고 표시
        titleLabel_->setText(title_ + " (잘못된 URL)");
        titleLabel_->setStyleSheet("font-size: 20px; font-weight: bold; color: #FF6B6B;");
    }
}
```

### 13.3 BookmarkService nullptr 체크

```cpp
HomePage::HomePage(BookmarkService *bookmarkService, QWidget *parent)
    : QWidget(parent)
    , bookmarkService_(bookmarkService)
{
    if (!bookmarkService_) {
        qCritical() << "HomePage: BookmarkService가 nullptr입니다!";
        // 에러 상태 표시
        setupUI();  // UI는 생성하되 데이터 로드 생략
        return;
    }

    // 정상 초기화
    setupUI();
    setupConnections();
    applyStyles();
}
```

---

## 14. 테스트 전략

### 14.1 단위 테스트 (Google Test)

```cpp
// tests/unit/HomePageTest.cpp

TEST(HomePageTest, DisplayEmptyStateWhenNoBookmarks) {
    // Given
    MockBookmarkService mockService;
    EXPECT_CALL(mockService, getAllBookmarks(_))
        .WillOnce(Invoke([](auto callback) {
            callback(true, QVector<Bookmark>());
        }));

    // When
    HomePage homePage(&mockService);
    homePage.refreshBookmarks();

    // Then
    EXPECT_TRUE(homePage.findChild<QWidget*>("emptyState")->isVisible());
}

TEST(HomePageTest, DisplayGridWhenBookmarksExist) {
    // Given
    MockBookmarkService mockService;
    QVector<Bookmark> bookmarks = createTestBookmarks(5);
    EXPECT_CALL(mockService, getAllBookmarks(_))
        .WillOnce(Invoke([bookmarks](auto callback) {
            callback(true, bookmarks);
        }));

    // When
    HomePage homePage(&mockService);
    homePage.refreshBookmarks();

    // Then
    EXPECT_EQ(homePage.findChildren<BookmarkCard*>().size(), 5);
}

TEST(HomePageTest, MaxBookmarksLimitIs12) {
    // Given
    MockBookmarkService mockService;
    QVector<Bookmark> bookmarks = createTestBookmarks(20);  // 20개 생성
    EXPECT_CALL(mockService, getAllBookmarks(_))
        .WillOnce(Invoke([bookmarks](auto callback) {
            callback(true, bookmarks);
        }));

    // When
    HomePage homePage(&mockService);
    homePage.refreshBookmarks();

    // Then
    EXPECT_EQ(homePage.findChildren<BookmarkCard*>().size(), 12);  // 12개만 표시
}
```

### 14.2 통합 테스트 (Qt Test)

```cpp
// tests/integration/HomePageIntegrationTest.cpp

void HomePageIntegrationTest::testHomeButtonNavigation() {
    // Given
    BrowserWindow window;
    SettingsService *settingsService = window.findChild<SettingsService*>();
    settingsService->setHomepage("about:favorites");

    // When
    NavigationBar *navBar = window.findChild<NavigationBar*>();
    QTest::mouseClick(navBar->findChild<QPushButton*>("homeButton"), Qt::LeftButton);

    // Then
    QStackedLayout *stackedLayout = window.findChild<QStackedLayout*>();
    QCOMPARE(stackedLayout->currentWidget()->metaObject()->className(), "HomePage");
}

void HomePageIntegrationTest::testBookmarkSelection() {
    // Given
    BrowserWindow window;
    HomePage *homePage = window.findChild<HomePage*>();
    homePage->show();
    QTest::qWaitForWindowExposed(homePage);

    // When
    BookmarkCard *firstCard = homePage->findChildren<BookmarkCard*>().first();
    QTest::keyClick(firstCard, Qt::Key_Enter);

    // Then
    WebView *webView = window.findChild<WebView*>();
    QCOMPARE(webView->url().toString(), firstCard->url());
}
```

### 14.3 수동 테스트 체크리스트

- [ ] 홈 버튼 클릭 시 HomePage 표시 (about:favorites 설정)
- [ ] 홈 버튼 클릭 시 일반 URL 로드 (https://www.google.com 설정)
- [ ] 북마크 0개 시 빈 상태 UI 표시
- [ ] 북마크 1~12개 시 그리드 뷰 표시
- [ ] 북마크 12개 초과 시 최대 12개만 표시
- [ ] 방향키 (Left/Right/Up/Down)로 포커스 이동
- [ ] Enter 키로 북마크 선택 및 페이지 로드
- [ ] Red 버튼으로 BookmarkPanel 열기
- [ ] Menu 버튼으로 SettingsPanel 열기
- [ ] 북마크 추가 시 HomePage 자동 새로고침
- [ ] 북마크 삭제 시 HomePage 자동 새로고침
- [ ] 포커스된 카드 테두리 강조 (파란색 #0078D7)
- [ ] URLBar에 "about:favorites" 표시

---

## 15. 성능 고려사항

### 15.1 메모리 사용량

- **HomePage 인스턴스**: 약 20KB
- **BookmarkCard × 12**: 약 12KB (각 1KB)
- **총 메모리**: 약 32KB (충분히 작음)

### 15.2 렌더링 성능

- **그리드 재구성 시간**: 목표 200ms 이내
- **북마크 로드 시간**: 목표 500ms 이내 (비동기)
- **포커스 이동 반응 시간**: 즉시 (keyPressEvent 동기 처리)

### 15.3 최적화 전략

1. **카드 재사용** (M4): rebuildGrid() 시 기존 카드를 재사용하여 new/delete 오버헤드 감소
2. **비동기 로드**: BookmarkService::getAllBookmarks()는 이미 비동기이므로 UI 블로킹 없음
3. **레이지 로드** (M4): 스크롤 시 보이는 카드만 렌더링

---

## 16. 보안 고려사항

### 16.1 URL 검증

```cpp
void HomePage::displayBookmarks(const QVector<Bookmark> &bookmarks) {
    for (const Bookmark &bookmark : bookmarks) {
        // URL 유효성 검증
        if (!URLValidator::isValid(bookmark.url)) {
            qWarning() << "HomePage: 잘못된 북마크 URL - " << bookmark.url;
            continue;  // 잘못된 북마크는 표시하지 않음
        }

        // 정상 북마크만 카드 생성
        // ...
    }
}
```

### 16.2 XSS 방지

```cpp
void BookmarkCard::setupUI() {
    // 제목에 HTML 태그 이스케이프
    QString safeTitle = title_.toHtmlEscaped();
    titleLabel_->setText(elideText(safeTitle, 20));

    // QLabel은 기본적으로 평문 표시 (setTextFormat(Qt::PlainText))
    titleLabel_->setTextFormat(Qt::PlainText);
}
```

---

## 17. 확장성 고려사항 (M4 이후)

### 17.1 파비콘 다운로드

```cpp
// M4에서 구현 예정
void BookmarkCard::loadFavicon(const QString &faviconUrl) {
    if (faviconUrl.isEmpty()) {
        // 기본 아이콘 사용
        return;
    }

    // QNetworkAccessManager로 비동기 다운로드
    // 완료 시 faviconLabel_->setPixmap() 호출
}
```

### 17.2 무한 스크롤

```cpp
// M4에서 구현 예정
void HomePage::setupUI() {
    // QScrollArea의 verticalScrollBar() 시그널 연결
    connect(contentScrollArea_->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &HomePage::onScrollValueChanged);
}

void HomePage::onScrollValueChanged(int value) {
    // 스크롤 끝에 도달 시 추가 북마크 로드
    if (value >= contentScrollArea_->verticalScrollBar()->maximum() - 100) {
        loadMoreBookmarks();
    }
}
```

### 17.3 드래그 앤 드롭 순서 변경

```cpp
// M4에서 구현 예정
void BookmarkCard::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        // 드래그 시작
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData();
        mimeData->setText(id_);  // 북마크 ID 전달
        drag->setMimeData(mimeData);
        drag->exec(Qt::MoveAction);
    }
}
```

---

## 18. 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | F-15 즐겨찾기 홈 화면 기술 설계서 초안 작성 | 요구사항 분석서 기반 |

---

## 19. 참고 자료

### 19.1 관련 문서
- **요구사항 분석서**: `docs/specs/favorites-home/requirements.md`
- **F-07 북마크 관리 설계서**: `docs/specs/bookmark-management/design.md`
- **F-11 설정 화면 설계서**: `docs/specs/settings/design.md`
- **F-13 리모컨 단축키 설계서**: `docs/specs/remote-shortcuts/design.md`

### 19.2 Qt 문서
- **QStackedLayout**: https://doc.qt.io/qt-5/qstackedlayout.html
- **QGridLayout**: https://doc.qt.io/qt-5/qgridlayout.html
- **QWidget Focus Policy**: https://doc.qt.io/qt-5/qwidget.html#focusPolicy-prop

### 19.3 참고 구현
- **Chrome 새 탭 페이지**: 자주 방문한 사이트 그리드 뷰
- **Firefox about:home**: Top Sites 그리드 레이아웃
- **Opera Speed Dial**: 북마크 그리드 + 썸네일

---

## 20. 구현 우선순위

### P0 (Must, M3에서 필수)
- [ ] HomePage 클래스 구현 (헤더 + 소스)
- [ ] BookmarkCard 클래스 구현
- [ ] BrowserWindow 통합 (QStackedLayout)
- [ ] NavigationBar 수정 (homeRequested 시그널)
- [ ] 그리드 레이아웃 (4열 × 3행)
- [ ] 북마크 데이터 연동 (BookmarkService)
- [ ] 빈 상태 UI
- [ ] 리모컨 키 이벤트 처리 (방향키, Enter, Red, Menu)

### P1 (Should, M3에서 권장)
- [ ] 포커스 스타일 (테두리 강조)
- [ ] 푸터 단축키 안내 라벨
- [ ] 북마크 추가/삭제 자동 새로고침
- [ ] CMakeLists.txt 업데이트

### P2 (Could, M4로 연기 가능)
- [ ] 파비콘 실제 다운로드
- [ ] 12개 초과 북마크 스크롤
- [ ] 북마크 순서 변경 (드래그 앤 드롭)
- [ ] 썸네일 이미지 표시
- [ ] 전환 애니메이션

---

## 21. 주의사항

### 21.1 개발 시 주의사항
- HomePage는 BrowserWindow의 자식 위젯이므로 메모리 자동 관리됨 (delete 불필요)
- BookmarkCard는 HomePage가 소유하므로 cards_ 벡터에서 제거 시 deleteLater() 호출
- about:favorites는 특수 URL이므로 WebView에서 로드 시도하면 안 됨
- 포커스 인덱스는 0부터 시작 (cards_.size()-1이 최대값)

### 21.2 테스트 시 주의사항
- 실제 webOS 프로젝터에서 리모컨 방향키 동작 확인 필수
- BookmarkService 모킹 필요 (단위 테스트)
- 북마크 0개, 1개, 12개, 13개 케이스 모두 테스트
- 포커스 순환 동작 확인 (그리드 경계에서 방향키 입력)

### 21.3 코드 리뷰 체크리스트
- [ ] C++17 표준 준수
- [ ] Qt 5.15+ API 사용
- [ ] 스마트 포인터 사용 (메모리 누수 방지)
- [ ] 시그널/슬롯 연결 확인
- [ ] 한국어 주석 작성
- [ ] QDebug 로깅 추가
- [ ] 에러 처리 (nullptr 체크)

---

**설계 완료**: 2026-02-14
**설계자**: architect (Claude Code Agent)
**검토자**: TBD (product-manager)
