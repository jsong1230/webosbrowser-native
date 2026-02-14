# F-15: 즐겨찾기 홈 화면 — 요구사항 분석서

## 1. 개요

### 기능명
즐겨찾기 홈 화면 (Favorites Home Page)

### 기능 ID
F-15

### 목적
저장된 북마크를 그리드 뷰로 표시하는 홈 화면을 제공하여, 사용자가 자주 방문하는 사이트에 빠르게 접근할 수 있도록 합니다. NavigationBar의 홈 버튼 클릭 시 홈페이지 설정에 따라 즐겨찾기 그리드 뷰 또는 특정 URL로 이동합니다.

### 대상 사용자
- 자주 방문하는 웹사이트(YouTube, Netflix, 뉴스 사이트 등)에 빠르게 접근하고 싶은 사용자
- 북마크를 시각적으로 탐색하고 선택하고 싶은 사용자
- URL 입력 없이 리모컨 방향키만으로 사이트에 접근하고 싶은 사용자
- 브라우저 시작 시 자주 가는 사이트들을 한눈에 보고 싶은 사용자

### 요청 배경
- 북마크 패널(BookmarkPanel)은 리스트 뷰로 북마크를 표시하지만, 홈 화면에서는 더 시각적이고 빠른 접근이 필요함
- 데스크톱 브라우저(Chrome, Firefox)의 새 탭 페이지와 같은 그리드 뷰 제공
- F-11 (설정 화면)에서 홈페이지 설정 기능이 구현되었으며, "about:favorites" 특수 URL로 홈 화면 표시 가능
- 리모컨 환경에서 방향키로 그리드 아이템을 탐색하는 것이 사용자 경험에 적합함
- 앱 시작 시 또는 새 탭 생성 시 홈 화면을 표시하여 사용성 향상

---

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: 홈 화면 표시 조건

- **설명**: NavigationBar의 홈 버튼 클릭 시 홈페이지 설정에 따라 적절한 화면을 표시합니다.
- **유저 스토리**: As a 사용자, I want 홈 버튼을 클릭했을 때 내가 설정한 홈페이지로 이동하고 싶다, so that 자주 가는 페이지에 빠르게 접근할 수 있다.
- **우선순위**: Must
- **상세 요구사항**:
  - **홈 버튼 클릭 시 동작**:
    1. `SettingsService::homepage()` 호출하여 홈페이지 URL 조회
    2. URL이 "about:favorites"이면 → HomePage 컴포넌트 표시 (그리드 뷰)
    3. URL이 그 외 값이면 → `WebView::load(url)` 호출 (일반 웹 페이지 로드)
  - **새 탭 생성 시 동작**:
    - TabManager에서 새 탭 생성 시 (Blue 버튼 또는 "+" 버튼)
    - 홈페이지 설정에 따라 자동으로 HomePage 또는 URL 표시
    - 기본값: "about:favorites" (설정되지 않은 경우 즐겨찾기 홈 화면)
  - **앱 시작 시 동작**:
    - BrowserWindow 초기화 시 첫 탭 생성
    - 첫 탭은 홈페이지 설정에 따라 표시
  - **about:favorites URL**:
    - 특수 URL로 WebView 대신 HomePage QWidget 표시
    - URLBar에는 "about:favorites" 문자열 표시
    - 뒤로/앞으로 히스토리에 "about:favorites" 기록

### FR-2: 그리드 뷰 UI 레이아웃

- **설명**: 북마크를 4열 × N행 그리드 레이아웃으로 표시하여 리모컨 방향키로 탐색할 수 있습니다.
- **유저 스토리**: As a 사용자, I want 북마크를 그리드 형태로 보고 싶다, so that 리모컨 방향키로 쉽게 탐색할 수 있다.
- **우선순위**: Must
- **상세 요구사항**:
  - **그리드 레이아웃**: QGridLayout 사용
    - 4열 고정 (리모컨 좌/우 키로 이동 최적화)
    - N행 (북마크 개수에 따라 동적, 최대 12개 북마크 표시 = 3행)
    - 그리드 아이템 간격: 16px (margin)
  - **그리드 아이템 구성**: 각 북마크는 BookmarkCard QWidget으로 표시
    - **파비콘** (QLabel + QPixmap):
      - 크기: 64x64px
      - 기본 아이콘: `:resources/icons/default-favicon.png` (파비콘 없을 시)
      - 위치: 카드 상단 중앙
    - **사이트 제목** (QLabel):
      - 폰트: 20px, Bold
      - 색상: 테마별 (다크 모드 #FFFFFF, 라이트 모드 #000000)
      - 최대 1줄 표시, 길면 말줄임표(...) 처리 (Qt::TextElideMode::ElideRight)
      - 위치: 파비콘 아래
    - **URL** (QLabel, 선택사항):
      - 폰트: 14px, Regular
      - 색상: 회색 (#888888)
      - 최대 1줄 표시, 말줄임표 처리
      - 위치: 제목 아래
    - **카드 배경**:
      - 기본: 투명 또는 연한 회색 (#2D2D2D 다크, #F5F5F5 라이트)
      - 호버/포커스: 강조 배경색 (#3C3C3C 다크, #E0E0E0 라이트)
      - 테두리: 포커스 시 2px 파란색 (#0078D7)
  - **카드 크기**:
    - 너비: 200px (4열 = 800px + 마진)
    - 높이: 180px (3행 = 540px + 마진)
  - **최대 표시 개수**: 12개 북마크 (4열 × 3행)
    - 12개 초과 북마크는 표시하지 않음 (M4에서 스크롤 또는 페이지네이션 추가 가능)
    - 표시 우선순위: 북마크 추가 순서 (최신 12개) 또는 방문 횟수 순서

### FR-3: 북마크 데이터 연동

- **설명**: BookmarkService에서 북마크 데이터를 조회하여 그리드에 표시합니다.
- **유저 스토리**: As a 사용자, I want 저장된 북마크가 홈 화면에 자동으로 표시되기를 원한다, so that 북마크 추가 후 바로 홈 화면에서 볼 수 있다.
- **우선순위**: Must
- **상세 요구사항**:
  - **데이터 로드**:
    - HomePage 초기화 시 `BookmarkService::getAllBookmarks()` 호출
    - 비동기 콜백으로 북마크 데이터 수신
    - 최대 12개 북마크만 표시 (QVector::mid(0, 12))
  - **북마크 변경 시 자동 새로고침**:
    - `BookmarkService::bookmarkAdded` 시그널 → `HomePage::onBookmarkAdded()` 슬롯
    - `BookmarkService::bookmarkDeleted` 시그널 → `HomePage::onBookmarkDeleted()` 슬롯
    - `BookmarkService::bookmarkUpdated` 시그널 → `HomePage::onBookmarkUpdated()` 슬롯
    - 슬롯에서 그리드 뷰 재구성 (`rebuildGrid()` 메서드)
  - **정렬 옵션** (초기 버전은 단순 정렬, M4에서 사용자 커스터마이징 추가):
    - 옵션 1: 추가 순서 역순 (최신 북마크 먼저)
    - 옵션 2: 방문 횟수 역순 (자주 가는 사이트 먼저)
    - 기본값: 옵션 1 (추가 순서 역순)

### FR-4: 빈 상태 처리

- **설명**: 북마크가 없을 때 안내 메시지와 액션 버튼을 표시합니다.
- **유저 스토리**: As a 사용자, I want 북마크가 없을 때 어떻게 추가하는지 안내받고 싶다, so that 첫 사용자도 쉽게 북마크를 추가할 수 있다.
- **우선순위**: Must
- **상세 요구사항**:
  - **빈 상태 UI**:
    - 중앙에 큰 아이콘 표시 (북마크 아이콘, 128x128px)
    - 메시지 QLabel:
      - "북마크를 추가하여 자주 가는 사이트에 빠르게 접근하세요"
      - 폰트: 24px, 중앙 정렬
      - 색상: 회색 (#888888)
    - 액션 버튼 (QPushButton):
      - **"북마크 추가" 버튼**: Red 버튼 또는 클릭 시 BookmarkPanel 열기
        - 버튼 텍스트: "북마크 추가 (Red 버튼)"
        - 클릭 시 `emit bookmarkAddRequested()` 시그널
        - BrowserWindow에서 시그널 수신 → BookmarkPanel 열기
      - **"설정" 버튼**: Menu 버튼 또는 클릭 시 SettingsPanel 열기
        - 버튼 텍스트: "홈페이지 설정 변경 (Menu 버튼)"
        - 클릭 시 `emit settingsRequested()` 시그널
        - BrowserWindow에서 시그널 수신 → SettingsPanel 열기
    - 레이아웃: QVBoxLayout, 중앙 정렬 (Qt::AlignCenter)
  - **빈 상태 전환**:
    - 북마크 개수 0개 → 빈 상태 UI 표시
    - 북마크 1개 이상 → 그리드 뷰 표시
    - `rebuildGrid()` 메서드에서 자동 전환

### FR-5: 그리드 아이템 선택 및 페이지 열기

- **설명**: 리모컨 방향키로 그리드 아이템을 탐색하고 Enter 키로 페이지를 엽니다.
- **유저 스토리**: As a 사용자, I want 리모컨 방향키로 북마크를 선택하고 Enter 키로 열고 싶다, so that 빠르게 사이트에 접근할 수 있다.
- **우선순위**: Must
- **상세 요구사항**:
  - **포커스 시스템**:
    - 각 BookmarkCard는 `QWidget::setFocusPolicy(Qt::StrongFocus)` 설정
    - 리모컨 방향키 (Up/Down/Left/Right)로 그리드 아이템 간 포커스 이동
    - QGridLayout에서 자동 포커스 순서 (좌→우, 상→하)
    - 포커스된 카드는 테두리 강조 (2px 파란색 #0078D7)
  - **Enter 키로 페이지 열기**:
    - 포커스된 BookmarkCard에서 Enter 키 입력 시:
      1. `emit bookmarkSelected(bookmark.url)` 시그널 발생
      2. BrowserWindow::onBookmarkSelected(url) 슬롯에서 `WebView::load(url)` 호출
      3. 페이지 로드 시작 → LoadingIndicator 표시 (F-05 연동)
      4. HomePage 자동 닫기 또는 뒤로 가기 히스토리에 추가
  - **마우스 클릭 지원** (선택사항):
    - BookmarkCard::mousePressEvent() 오버라이드
    - 클릭 시 동일하게 `emit bookmarkSelected(url)` 시그널

### FR-6: 리모컨 단축키 연동

- **설명**: F-13에서 정의된 리모컨 단축키를 HomePage에서 지원합니다.
- **유저 스토리**: As a 사용자, I want 홈 화면에서도 리모컨 단축키를 사용하고 싶다, so that 일관된 UX로 빠르게 기능을 실행할 수 있다.
- **우선순위**: Must
- **상세 요구사항**:
  - **Red 버튼 (0x193)**: 북마크 추가
    - HomePage에서 Red 버튼 입력 시 `emit bookmarkAddRequested()` 시그널
    - BrowserWindow에서 시그널 수신 → BookmarkPanel 열기 (F-13과 동일)
  - **Back 키 (Qt::Key_Back, Qt::Key_Escape)**: 이전 페이지로
    - HomePage 표시 중 Back 키 입력 시:
      - WebView 히스토리가 있으면 → 브라우저 뒤로 가기
      - 히스토리가 없으면 → 아무 동작 없음 (또는 앱 종료 확인 다이얼로그)
  - **Menu 버튼 (Qt::Key_Menu)**: 설정 패널 열기
    - HomePage에서 Menu 버튼 입력 시 `emit settingsRequested()` 시그널
    - BrowserWindow에서 시그널 수신 → SettingsPanel 열기 (F-13과 동일)
  - **방향키 (Up/Down/Left/Right)**: 그리드 아이템 간 포커스 이동
    - Qt 기본 포커스 시스템 사용
  - **Enter 키 (Qt::Key_Return, Qt::Key_Enter)**: 북마크 선택 및 페이지 열기
    - FR-5에서 정의

### FR-7: 홈페이지 설정 연동

- **설명**: SettingsService의 홈페이지 설정 변경 시 HomePage 동작을 업데이트합니다.
- **유저 스토리**: As a 사용자, I want 홈페이지 설정을 변경하면 홈 버튼 동작이 즉시 바뀌기를 원한다, so that 설정이 즉시 반영된다.
- **우선순위**: Must
- **상세 요구사항**:
  - **설정 변경 감지**:
    - `SettingsService::homepageChanged` 시그널 → `NavigationBar::onHomepageChanged()` 슬롯
    - NavigationBar::setHomepage(url) 호출하여 내부 homepage_ 변수 업데이트
  - **홈 버튼 클릭 시 동작**:
    - NavigationBar::onHomeClicked() 메서드:
      ```cpp
      void NavigationBar::onHomeClicked() {
          QString homepageUrl = homepage_;  // SettingsService에서 동기화된 값
          if (homepageUrl == "about:favorites") {
              emit showFavoritesPage();  // BrowserWindow에서 HomePage 표시
          } else {
              if (webView_) {
                  webView_->load(QUrl(homepageUrl));
              }
          }
      }
      ```
  - **시그널/슬롯 연결**:
    - BrowserWindow에서 `NavigationBar::showFavoritesPage` 시그널 → `BrowserWindow::showHomePage()` 슬롯
    - showHomePage() 메서드에서 HomePage 컴포넌트 표시 (WebView 숨기기, HomePage 표시)

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **북마크 로드 시간**: HomePage 초기화 시 BookmarkService에서 북마크 로드 500ms 이내
  - 비동기 호출이므로 UI 블로킹 없음
  - 로드 중 스피너 표시 (선택사항)
- **그리드 렌더링 속도**: 12개 BookmarkCard 생성 및 레이아웃 100ms 이내
  - QGridLayout은 빠르므로 최적화 불필요
  - 파비콘 로드는 비동기 (현재는 기본 아이콘 사용)
- **북마크 변경 반응 속도**: 북마크 추가/삭제 시 그리드 재구성 200ms 이내
  - `rebuildGrid()` 메서드에서 기존 카드 삭제 후 재생성

### 접근성
- **리모컨 최적화**: 모든 그리드 아이템에 방향키로 접근 가능
  - 4열 그리드 (좌/우 키로 이동 명확)
  - 포커스 표시 명확 (2px 테두리 강조)
- **고대비 테마**: 다크 모드, 라이트 모드 모두 WCAG 2.1 AA 수준 대비 비율 준수
  - 다크 모드: 배경 #1E1E1E, 텍스트 #FFFFFF
  - 라이트 모드: 배경 #FFFFFF, 텍스트 #000000
- **포커스 가시성**: 포커스된 카드는 명확히 구분
  - 테두리 색상 + 배경색 변경

### 보안
- **북마크 URL 검증**: 북마크 로드 시 URLValidator::isValid() 호출 (선택사항)
  - 잘못된 URL은 표시하지 않거나 경고 아이콘 표시
- **XSS 방지**: 북마크 제목에 HTML 태그 이스케이프 처리
  - `QString::toHtmlEscaped()` 사용 (QLabel은 기본적으로 평문 표시)

### 확장성
- **파비콘 로드**: 초기 버전은 기본 아이콘 사용, M4에서 실제 파비콘 다운로드 기능 추가 가능
  - BookmarkService에 favicon 필드 이미 존재
  - 비동기 파비콘 다운로드 → QPixmap으로 표시
- **북마크 개수 확장**: 현재 12개 제한, M4에서 스크롤 또는 페이지네이션 추가 가능
  - QScrollArea로 래핑
  - 또는 "더 보기" 버튼 추가
- **폴더 필터링**: M4에서 특정 폴더의 북마크만 표시하는 기능 추가 가능
  - 예: "홈 화면에 표시" 플래그가 있는 북마크만 표시

### 메모리 사용량
- **HomePage 컴포넌트**: 약 20KB (QWidget + 12개 BookmarkCard)
- **BookmarkCard**: 각 약 1KB (QLabel + QPixmap)
- **총 메모리**: 약 32KB (충분히 작음)

---

## 4. 데이터 구조

### HomePage 클래스

```cpp
// src/ui/HomePage.h

#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include "../models/Bookmark.h"

namespace webosbrowser {

class BookmarkService;
class BookmarkCard;

/**
 * @class HomePage
 * @brief 즐겨찾기 홈 화면 컴포넌트
 *
 * 북마크를 그리드 뷰로 표시하고 리모컨 방향키로 탐색할 수 있습니다.
 * about:favorites URL로 표시됩니다.
 */
class HomePage : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(BookmarkService *bookmarkService, QWidget *parent = nullptr);
    ~HomePage() override;

    HomePage(const HomePage&) = delete;
    HomePage& operator=(const HomePage&) = delete;

    /**
     * @brief 그리드 뷰 새로고침 (북마크 변경 시 호출)
     */
    void refresh();

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
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief 북마크 추가됨 슬롯
     */
    void onBookmarkAdded(const Bookmark &bookmark);

    /**
     * @brief 북마크 삭제됨 슬롯
     */
    void onBookmarkDeleted(const QString &id);

    /**
     * @brief 북마크 업데이트됨 슬롯
     */
    void onBookmarkUpdated(const QString &id);

    /**
     * @brief 북마크 데이터 로드 완료 슬롯
     */
    void onBookmarksLoaded(bool success, const QVector<Bookmark> &bookmarks);

    /**
     * @brief BookmarkCard 클릭 슬롯
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
     * @brief 그리드 뷰 재구성 (북마크 변경 시)
     */
    void rebuildGrid();

    /**
     * @brief 빈 상태 UI 표시
     */
    void showEmptyState();

    /**
     * @brief 그리드 UI 표시
     */
    void showGrid();

    BookmarkService *bookmarkService_;
    QGridLayout *gridLayout_;
    QWidget *emptyStateWidget_;  // 빈 상태 UI
    QVector<BookmarkCard*> cards_;  // 북마크 카드 리스트
    QVector<Bookmark> bookmarks_;  // 북마크 데이터 캐시

    static constexpr int MAX_BOOKMARKS = 12;  // 최대 표시 개수
    static constexpr int GRID_COLUMNS = 4;    // 그리드 열 개수
};

} // namespace webosbrowser
```

### BookmarkCard 클래스

```cpp
// src/ui/BookmarkCard.h

#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

namespace webosbrowser {

/**
 * @class BookmarkCard
 * @brief 홈 화면의 북마크 카드 UI
 *
 * 파비콘, 제목, URL을 표시하고 클릭/Enter 키로 페이지를 엽니다.
 */
class BookmarkCard : public QWidget {
    Q_OBJECT

public:
    explicit BookmarkCard(const QString &id, const QString &title, const QString &url, QWidget *parent = nullptr);
    ~BookmarkCard() override;

    BookmarkCard(const BookmarkCard&) = delete;
    BookmarkCard& operator=(const BookmarkCard&) = delete;

    QString id() const { return id_; }
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
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 키 입력 이벤트 (Enter 키)
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 포커스 인 이벤트 (테두리 강조)
     */
    void focusInEvent(QFocusEvent *event) override;

    /**
     * @brief 포커스 아웃 이벤트
     */
    void focusOutEvent(QFocusEvent *event) override;

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 스타일 적용 (포커스 상태에 따라)
     */
    void updateStyle(bool focused);

    QString id_;
    QString title_;
    QString url_;

    QVBoxLayout *layout_;
    QLabel *faviconLabel_;  // 파비콘 (기본 아이콘)
    QLabel *titleLabel_;    // 제목
    QLabel *urlLabel_;      // URL (선택사항)
};

} // namespace webosbrowser
```

---

## 5. 제약조건

### 기술 제약사항
- **QGridLayout 포커스 순서**: Qt 기본 포커스 순서는 좌→우, 상→하이지만, 리모컨 방향키 동작이 직관적이지 않을 수 있음
  - 해결책: `QWidget::setTabOrder()` 수동 설정 또는 keyPressEvent() 커스터마이징
- **about:favorites URL**: 특수 URL이므로 WebView에서 로드할 수 없음
  - BrowserWindow에서 URL 체크하여 HomePage 표시로 분기 필요
- **파비콘 로드**: 초기 버전은 기본 아이콘 사용, 실제 파비콘은 M4 이후 구현
  - BookmarkService에 favicon 필드는 존재하나 현재 사용하지 않음

### webOS 플랫폼 제약사항
- **리모컨 방향키 이벤트**: QGridLayout의 기본 포커스 이동이 리모컨 환경에서 테스트 필요
  - 실제 webOS 프로젝터 환경에서 수동 테스트 필수
- **화면 해상도**: 4K UHD (3840x2160) 기본, 그리드 카드 크기 조정 필요
  - 카드 크기: 200x180px × 12개 = 화면 중앙 배치 (여백 고려)

### 의존성
- **F-07 (북마크 관리)**: BookmarkService::getAllBookmarks() (데이터 조회)
- **F-11 (설정 화면)**: SettingsService::homepage() (홈페이지 설정)
- **F-04 (페이지 탐색 컨트롤)**: NavigationBar::onHomeClicked() (홈 버튼 동작)
- **F-13 (리모컨 단축키)**: Red 버튼, Back 키, Menu 버튼 (keyPressEvent)
- **F-05 (로딩 인디케이터)**: 페이지 로드 시 로딩 표시

### 범위 외 (Out of Scope)
- **파비콘 자동 다운로드**: M4 이후 구현 (초기 버전은 기본 아이콘 사용)
- **북마크 순서 변경**: M4 이후 구현 (드래그 앤 드롭 또는 설정)
- **폴더 필터링**: M4 이후 구현 (특정 폴더의 북마크만 홈 화면에 표시)
- **커스터마이징**: M4 이후 구현 (사용자가 홈 화면에 표시할 북마크 선택)
- **썸네일 이미지**: M4 이후 구현 (웹 페이지 스크린샷 썸네일)
- **무한 스크롤**: M4 이후 구현 (12개 초과 북마크 표시)

---

## 6. 용어 정의

| 용어 | 정의 |
|------|------|
| **홈 화면** | 북마크를 그리드 뷰로 표시하는 특수 페이지 (about:favorites URL) |
| **about:favorites** | 홈 화면을 표시하는 특수 URL (WebView 대신 HomePage 표시) |
| **BookmarkCard** | 그리드 뷰의 각 북마크 카드 UI (파비콘 + 제목 + URL) |
| **HomePage** | 홈 화면 컴포넌트 (QWidget 기반) |
| **그리드 뷰** | 북마크를 4열 × N행으로 표시하는 레이아웃 (QGridLayout) |
| **빈 상태** | 북마크가 0개일 때 표시되는 안내 메시지 및 액션 버튼 UI |
| **홈페이지 설정** | NavigationBar 홈 버튼 클릭 시 이동할 URL (SettingsService에서 관리) |
| **파비콘** | 웹사이트의 작은 아이콘 (16x16px 또는 32x32px, 홈 화면에서는 64x64px) |

---

## 7. UI/UX 플로우

### 시나리오 1: 홈 버튼 클릭 → 홈 화면 표시

```
사용자 → NavigationBar 홈 버튼 클릭 → 홈페이지 설정 확인 → HomePage 표시
   │                  │                        │                        │
   │  리모컨 선택 버튼  │                        │                        │
   │─────────────────▶│                        │                        │
   │                  │ onHomeClicked()        │                        │
   │                  │───────────────────────▶│                        │
   │                  │                        │ SettingsService::homepage()
   │                  │                        │ == "about:favorites"   │
   │                  │                        │───────────────────────▶│
   │                  │                        │ emit showFavoritesPage()
   │                  │                        │───────────────────────▶BrowserWindow
   │                  │                        │                        │
   │                  │                        │ BrowserWindow::showHomePage()
   │                  │                        │ WebView 숨기기, HomePage 표시
   │                  │                        │                        │
   │                  │                        │ BookmarkService::getAllBookmarks()
   │                  │                        │───────────────────────▶│
   │                  │                        │ 12개 북마크 로드        │
   │                  │                        │◀───────────────────────│
   │                  │                        │ HomePage::rebuildGrid()
   │                  │                        │ 그리드 뷰 렌더링 완료   │
   │◀─────────────────────────────────────────────────────────────────────│
   │  (화면에 4×3 그리드 표시)
```

### 시나리오 2: 그리드 아이템 선택 → 페이지 열기

```
사용자 → HomePage (그리드 뷰) → 방향키로 북마크 선택 → Enter 키 → 페이지 로드
   │            │                        │                  │           │
   │  방향키 이동 │                        │                  │           │
   │───────────▶│                        │                  │           │
   │            │ 포커스 이동 (BookmarkCard 간)              │           │
   │            │───────────────────────▶│                  │           │
   │            │                        │ 테두리 강조 (#0078D7)         │
   │            │                        │                  │           │
   │  Enter 키  │                        │                  │           │
   │───────────▶│                        │                  │           │
   │            │ BookmarkCard::keyPressEvent(Enter)        │           │
   │            │───────────────────────▶│                  │           │
   │            │                        │ emit clicked(url)│           │
   │            │                        │─────────────────▶│           │
   │            │                        │                  │ HomePage::onCardClicked(url)
   │            │                        │                  │ emit bookmarkSelected(url)
   │            │                        │                  │──────────▶BrowserWindow
   │            │                        │                  │           │
   │            │                        │                  │ WebView::load(url)
   │            │                        │                  │ HomePage 숨기기
   │            │                        │                  │ WebView 표시
   │◀──────────────────────────────────────────────────────────────────────│
   │  (웹 페이지 로딩 시작)
```

### 시나리오 3: 북마크 없음 → 북마크 추가

```
사용자 → HomePage (빈 상태) → "북마크 추가" 버튼 클릭 → BookmarkPanel 열기
   │            │                      │                        │
   │            │ 북마크 0개 확인      │                        │
   │            │ showEmptyState()     │                        │
   │            │ "북마크를 추가하여..." 메시지 표시            │
   │            │                      │                        │
   │  Red 버튼  │                      │                        │
   │───────────▶│                      │                        │
   │            │ keyPressEvent(Red)   │                        │
   │            │─────────────────────▶│                        │
   │            │                      │ emit bookmarkAddRequested()
   │            │                      │───────────────────────▶BrowserWindow
   │            │                      │                        │
   │            │                      │ BrowserWindow::showBookmarkPanel()
   │            │                      │ BookmarkPanel 슬라이드 인
   │◀──────────────────────────────────────────────────────────────│
   │  (BookmarkPanel 표시, 북마크 추가 다이얼로그)
   │
   │  (사용자가 북마크 추가 완료)
   │─────────────────────────────────────────────────────────────▶BookmarkService
   │                      │                        │
   │                      │ emit bookmarkAdded()   │
   │                      │───────────────────────▶HomePage
   │                      │                        │
   │                      │ HomePage::onBookmarkAdded()
   │                      │ rebuildGrid() → 그리드 뷰 표시
   │◀──────────────────────────────────────────────────────────────│
   │  (홈 화면에 북마크 카드 1개 표시)
```

### 시나리오 4: 앱 시작 → 홈 화면 자동 표시

```
앱 시작 → BrowserWindow 초기화 → SettingsService::homepage() → 첫 탭에 HomePage 표시
   │              │                        │                        │
   │ main()       │                        │                        │
   │─────────────▶│                        │                        │
   │              │ BrowserWindow::BrowserWindow()                  │
   │              │ TabManager::createTab()                         │
   │              │───────────────────────▶│                        │
   │              │                        │ SettingsService::homepage()
   │              │                        │ == "about:favorites"   │
   │              │                        │───────────────────────▶│
   │              │                        │ WebView 대신 HomePage 표시
   │              │                        │ BookmarkService::getAllBookmarks()
   │              │                        │ 그리드 뷰 렌더링        │
   │◀──────────────────────────────────────────────────────────────────│
   │  (앱 화면에 홈 화면 표시)
```

---

## 8. 완료 기준 (Acceptance Criteria)

### AC-1: 홈 화면 표시 조건
- [ ] NavigationBar 홈 버튼 클릭 시 SettingsService::homepage() 확인
- [ ] 홈페이지 설정이 "about:favorites"이면 HomePage 표시
- [ ] 홈페이지 설정이 그 외 URL이면 WebView::load(url) 호출
- [ ] 새 탭 생성 시 홈페이지 설정에 따라 HomePage 또는 URL 로드
- [ ] 앱 시작 시 첫 탭은 홈페이지 설정에 따라 표시

### AC-2: 그리드 뷰 UI
- [ ] BookmarkService에서 최대 12개 북마크 조회
- [ ] 북마크를 4열 × 3행 그리드로 표시 (QGridLayout)
- [ ] 각 BookmarkCard에 파비콘(기본 아이콘), 제목, URL 표시
- [ ] 카드 크기: 200x180px, 간격 16px
- [ ] 포커스된 카드는 테두리 강조 (2px 파란색 #0078D7)

### AC-3: 북마크 데이터 연동
- [ ] HomePage 초기화 시 BookmarkService::getAllBookmarks() 호출
- [ ] 북마크 추가 시 자동 새로고침 (bookmarkAdded 시그널)
- [ ] 북마크 삭제 시 자동 새로고침 (bookmarkDeleted 시그널)
- [ ] 북마크 업데이트 시 자동 새로고침 (bookmarkUpdated 시그널)

### AC-4: 빈 상태 처리
- [ ] 북마크 0개일 때 빈 상태 UI 표시
- [ ] "북마크를 추가하여..." 안내 메시지 표시
- [ ] "북마크 추가" 버튼 (Red 버튼 안내) 표시
- [ ] "설정" 버튼 (Menu 버튼 안내) 표시
- [ ] 북마크 추가 후 그리드 뷰로 자동 전환

### AC-5: 그리드 아이템 선택 및 페이지 열기
- [ ] 리모컨 방향키 (Up/Down/Left/Right)로 그리드 아이템 간 포커스 이동
- [ ] Enter 키로 포커스된 북마크 페이지 열기
- [ ] emit bookmarkSelected(url) 시그널 → BrowserWindow::onBookmarkSelected(url)
- [ ] WebView::load(url) 호출 → 페이지 로드 시작
- [ ] 로드 시작 시 LoadingIndicator 표시 (F-05 연동)

### AC-6: 리모컨 단축키 연동
- [ ] Red 버튼 (0x193) 입력 시 emit bookmarkAddRequested()
- [ ] BrowserWindow에서 시그널 수신 → BookmarkPanel 열기
- [ ] Back 키 입력 시 브라우저 뒤로 가기 (히스토리 있을 경우)
- [ ] Menu 버튼 입력 시 emit settingsRequested()
- [ ] BrowserWindow에서 시그널 수신 → SettingsPanel 열기

### AC-7: 홈페이지 설정 연동
- [ ] SettingsService::homepageChanged 시그널 → NavigationBar::onHomepageChanged() 슬롯
- [ ] NavigationBar::setHomepage(url) 호출하여 homepage_ 변수 업데이트
- [ ] 홈 버튼 클릭 시 업데이트된 홈페이지 설정 반영
- [ ] about:favorites URL 설정 시 HomePage 표시

### AC-8: 회귀 테스트
- [ ] 기존 NavigationBar, WebView, TabManager 동작 정상
- [ ] F-07 북마크 관리 기능 정상 (BookmarkPanel)
- [ ] F-11 설정 화면 기능 정상 (SettingsPanel)
- [ ] F-13 리모컨 단축키와 충돌 없음

---

## 9. 참고 사항

### 관련 문서
- **PRD**: `docs/project/prd.md`
- **기능 백로그**: `docs/project/features.md`
- **F-07 요구사항 분석서**: `docs/specs/bookmark-management/requirements.md`
- **F-11 요구사항 분석서**: `docs/specs/settings/requirements.md`
- **F-13 요구사항 분석서**: `docs/specs/remote-shortcuts/requirements.md`
- **F-04 요구사항 분석서**: `docs/specs/page-navigation-controls/requirements.md`
- **CLAUDE.md**: 프로젝트 규칙 및 코딩 컨벤션

### 참고 구현
- **Chrome 새 탭 페이지**: `chrome://newtab/`
  - 자주 방문한 사이트 8개 그리드 표시
  - 썸네일 이미지 + 사이트 제목
  - 마우스/키보드로 선택 및 열기
- **Firefox 홈 화면**: `about:home`
  - 상위 사이트(Top Sites) 그리드 표시
  - 파비콘 + 사이트 제목
  - 커스터마이징 가능 (사이트 고정, 제거)
- **Opera Speed Dial**: 즐겨찾기 그리드 표시
  - 썸네일 이미지 + 제목
  - 드래그 앤 드롭으로 순서 변경

### Qt 위젯 선택 가이드
- **그리드 레이아웃**: QGridLayout (간단하고 빠름)
- **북마크 카드**: QWidget (커스텀 카드 UI)
- **파비콘**: QLabel + QPixmap (64x64px)
- **제목/URL**: QLabel (텍스트 표시)
- **빈 상태**: QVBoxLayout + QLabel + QPushButton

### 구현 권장사항
- **HomePage 초기화**: BrowserWindow에서 HomePage 인스턴스를 미리 생성하여 캐싱
  - 홈 버튼 클릭 시 즉시 표시 (생성 지연 없음)
  - 메모리 사용량 약 32KB로 충분히 작음
- **그리드 재구성 최적화**: rebuildGrid() 메서드에서 기존 카드 삭제 → 새 카드 생성
  - QGridLayout::clear() 또는 QLayoutItem::removeWidget() 사용
  - 카드 재사용 고려 (M4 최적화)
- **포커스 순서**: QWidget::setTabOrder() 수동 설정 권장
  - QGridLayout 기본 포커스 순서가 직관적이지 않을 수 있음
  - 좌→우, 상→하 순서로 명시적 설정

### 우선순위 조정 가능성
- FR-6 (리모컨 단축키 연동)의 Red 버튼은 F-13과 중복이므로 Should 우선순위로 조정 가능
- FR-3 (북마크 데이터 연동)의 정렬 옵션은 Could 우선순위로 조정 가능 (기본 정렬만 M3 구현)
- FR-2 (그리드 뷰 UI)의 URL 표시는 선택사항 (파비콘 + 제목만 표시 가능)

---

## 10. 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|-----------|--------|
| 2026-02-14 | 1.0 | F-15 즐겨찾기 홈 화면 요구사항 분석서 초안 작성 | product-manager |
