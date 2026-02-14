# F-15: 즐겨찾기 홈 화면 — 구현 계획서

## 1. 참조 문서
- **요구사항 분석서**: `docs/specs/favorites-home/requirements.md`
- **기술 설계서**: `docs/specs/favorites-home/design.md`
- **프로젝트 규칙**: `CLAUDE.md`

---

## 2. 구현 Phase

### Phase 1: BookmarkCard 컴포넌트 구현

**담당**: general-purpose (C++/Qt 구현)

**작업 내역**:
- [ ] Task 1-1: `src/ui/BookmarkCard.h` 헤더 작성
  - 클래스 선언 (QWidget 상속)
  - 생성자/소멸자, 복사 생성자 삭제
  - 시그널 (clicked), 슬롯 정의
  - 이벤트 핸들러 (mousePressEvent, keyPressEvent, focusInEvent, focusOutEvent)
  - private 멤버 (layout_, faviconLabel_, titleLabel_, urlLabel_, id_, title_, url_, isFocused_)

- [ ] Task 1-2: `src/ui/BookmarkCard.cpp` 구현 - UI 초기화
  - 생성자에서 데이터 초기화 (id_, title_, url_)
  - setupUI() 구현: QVBoxLayout, QLabel (파비콘, 제목, URL) 생성
  - 파비콘 기본 아이콘 로드 (`:resources/icons/default-favicon.png`)
  - 제목 말줄임표 처리 (elideText 메서드)
  - 카드 크기 고정 (200×180px)
  - 포커스 정책 설정 (Qt::StrongFocus)

- [ ] Task 1-3: `src/ui/BookmarkCard.cpp` 구현 - 스타일링
  - updateStyle(bool focused) 구현
  - 포커스 시 테두리 강조 (2px 파란색 #0078D7)
  - 비포커스 시 기본 스타일 (투명 테두리, 배경 #2A2A2A)
  - QSS 스타일시트 적용

- [ ] Task 1-4: `src/ui/BookmarkCard.cpp` 구현 - 이벤트 핸들링
  - mousePressEvent() 구현: 좌클릭 시 `emit clicked(url_)`
  - keyPressEvent() 구현: Enter 키 시 `emit clicked(url_)`
  - focusInEvent() 구현: `isFocused_ = true`, `updateStyle(true)` 호출
  - focusOutEvent() 구현: `isFocused_ = false`, `updateStyle(false)` 호출

**예상 산출물**:
- `src/ui/BookmarkCard.h` (약 100줄)
- `src/ui/BookmarkCard.cpp` (약 200줄)

**완료 기준**:
- BookmarkCard 인스턴스 생성 시 파비콘 + 제목 표시
- Enter 키 입력 시 `clicked(url)` 시그널 발생 확인 (qDebug 로그)
- 포커스 시 테두리 색상 변경 확인

**예상 복잡도**: Medium
**예상 시간**: 1.5시간

---

### Phase 2: HomePage 컴포넌트 구현 - 기본 구조

**담당**: general-purpose (C++/Qt 구현)

**작업 내역**:
- [ ] Task 2-1: `src/ui/HomePage.h` 헤더 작성
  - 클래스 선언 (QWidget 상속)
  - BookmarkService* 멤버
  - UI 멤버 (mainLayout_, titleLabel_, contentScrollArea_, contentContainer_, gridLayout_, emptyStateWidget_, footerLabel_)
  - 데이터 멤버 (cards_, bookmarks_, currentFocusIndex_)
  - 시그널 (bookmarkSelected, bookmarkAddRequested, settingsRequested)
  - private 슬롯 (onBookmarkAdded, onBookmarkDeleted, onBookmarkUpdated, onBookmarksLoaded, onCardClicked)
  - private 메서드 (setupUI, setupConnections, applyStyles, rebuildGrid, displayBookmarks, displayEmptyState, moveFocus[Left/Right/Up/Down], activateCurrentCard)
  - 상수 (MAX_BOOKMARKS=12, GRID_COLUMNS=4)

- [ ] Task 2-2: `src/ui/HomePage.cpp` 구현 - setupUI()
  - mainLayout_ 생성 (QVBoxLayout, 40px 마진)
  - titleLabel_ 생성 ("즐겨찾기", 32px Bold)
  - contentScrollArea_ 생성 (QScrollArea, 스크롤바 정책 설정)
  - contentContainer_ 생성 (QWidget)
  - gridLayout_ 생성 (QGridLayout, 4열, 16px 간격)
  - emptyStateWidget_ 생성 (createEmptyStateWidget 호출)
  - footerLabel_ 생성 ("Red: 북마크 추가 | Menu: 설정", 16px)
  - 레이아웃 배치 (타이틀 → 스크롤 영역 → 푸터)

- [ ] Task 2-3: `src/ui/HomePage.cpp` 구현 - createEmptyStateWidget()
  - QWidget 생성 (emptyState 오브젝트명)
  - QVBoxLayout 생성 (중앙 정렬)
  - 아이콘 QLabel (`:resources/icons/bookmark-empty.png`, 128×128px)
  - 메시지 QLabel ("북마크를 추가하여..." 안내 메시지)
  - "북마크 추가" QPushButton (클릭 시 `emit bookmarkAddRequested()`)
  - "홈페이지 설정 변경" QPushButton (클릭 시 `emit settingsRequested()`)
  - QSS 스타일 적용

- [ ] Task 2-4: `src/ui/HomePage.cpp` 구현 - setupConnections()
  - BookmarkService 시그널 연결 (bookmarkAdded, bookmarkDeleted, bookmarkUpdated)
  - 각 시그널을 HomePage 슬롯 (onBookmarkAdded, onBookmarkDeleted, onBookmarkUpdated)에 연결

- [ ] Task 2-5: `src/ui/HomePage.cpp` 구현 - applyStyles()
  - QSS 스타일시트 작성 (다크 모드 배경 #1E1E1E)
  - titleLabel, footerLabel, QScrollArea 스타일 적용
  - setStyleSheet() 호출

**예상 산출물**:
- `src/ui/HomePage.h` (약 150줄)
- `src/ui/HomePage.cpp` (setupUI, createEmptyStateWidget, setupConnections, applyStyles 부분, 약 250줄)

**완료 기준**:
- HomePage 인스턴스 생성 시 타이틀, 푸터 표시
- 초기 상태에서 빈 상태 위젯 표시 (북마크 0개 가정)
- BookmarkService 시그널 연결 확인 (qDebug 로그)

**예상 복잡도**: Medium
**예상 시간**: 2시간

---

### Phase 3: HomePage 컴포넌트 구현 - 북마크 데이터 연동

**담당**: general-purpose (C++/Qt 구현)

**작업 내역**:
- [ ] Task 3-1: `src/ui/HomePage.cpp` 구현 - refreshBookmarks()
  - BookmarkService::getAllBookmarks() 비동기 호출
  - 콜백 람다에서 onBookmarksLoaded() 슬롯 호출
  - qDebug 로그 추가

- [ ] Task 3-2: `src/ui/HomePage.cpp` 구현 - onBookmarksLoaded()
  - success 체크 (실패 시 displayEmptyState() 호출)
  - bookmarks_ 데이터 캐시 업데이트
  - 북마크 개수 MAX_BOOKMARKS(12) 제한 (QVector::mid(0, 12))
  - 북마크 0개 시 displayEmptyState() 호출
  - 북마크 1개 이상 시 displayBookmarks() 호출

- [ ] Task 3-3: `src/ui/HomePage.cpp` 구현 - displayBookmarks()
  - bookmarks_ 데이터 캐시 업데이트
  - rebuildGrid() 호출

- [ ] Task 3-4: `src/ui/HomePage.cpp` 구현 - displayEmptyState()
  - 기존 cards_ 삭제 (deleteLater())
  - cards_.clear()
  - emptyStateWidget_->show()

- [ ] Task 3-5: `src/ui/HomePage.cpp` 구현 - rebuildGrid()
  - 기존 cards_ 삭제 (gridLayout_->removeWidget, deleteLater())
  - cards_.clear()
  - emptyStateWidget_->hide()
  - 북마크 0개 시 displayEmptyState() 호출 후 종료
  - bookmarks_ 순회하며 BookmarkCard 생성
  - BookmarkCard::clicked 시그널을 HomePage::onCardClicked 슬롯에 연결
  - gridLayout_에 카드 추가 (row = i / GRID_COLUMNS, col = i % GRID_COLUMNS)
  - cards_ 벡터에 추가
  - 첫 번째 카드에 포커스 (currentFocusIndex_ = 0)

- [ ] Task 3-6: `src/ui/HomePage.cpp` 구현 - onCardClicked()
  - qDebug 로그
  - `emit bookmarkSelected(url)` 시그널 발생

- [ ] Task 3-7: `src/ui/HomePage.cpp` 구현 - 북마크 변경 슬롯
  - onBookmarkAdded(): refreshBookmarks() 호출
  - onBookmarkDeleted(): refreshBookmarks() 호출
  - onBookmarkUpdated(): refreshBookmarks() 호출

- [ ] Task 3-8: `src/ui/HomePage.cpp` 구현 - showEvent()
  - QWidget::showEvent() 오버라이드
  - HomePage 표시 시 자동으로 refreshBookmarks() 호출

**예상 산출물**:
- `src/ui/HomePage.cpp` (refreshBookmarks, onBookmarksLoaded, displayBookmarks, displayEmptyState, rebuildGrid, onCardClicked, onBookmark* 슬롯, showEvent, 약 300줄 추가)

**완료 기준**:
- refreshBookmarks() 호출 시 BookmarkService에서 데이터 조회
- 북마크 0개 시 빈 상태 UI 표시
- 북마크 1~12개 시 그리드 뷰 표시 (4열)
- 북마크 13개 이상 시 12개만 표시
- 북마크 추가/삭제 시 HomePage 자동 새로고침 확인

**예상 복잡도**: Medium
**예상 시간**: 1.5시간

---

### Phase 4: HomePage 컴포넌트 구현 - 리모컨 네비게이션

**담당**: general-purpose (C++/Qt 구현)

**작업 내역**:
- [ ] Task 4-1: `src/ui/HomePage.cpp` 구현 - keyPressEvent()
  - Qt::Key_Left → moveFocusLeft()
  - Qt::Key_Right → moveFocusRight()
  - Qt::Key_Up → moveFocusUp()
  - Qt::Key_Down → moveFocusDown()
  - Qt::Key_Return / Qt::Key_Enter → activateCurrentCard()
  - Qt::Key_Escape / Qt::Key_Back → event->ignore() (부모로 전파)
  - 0x193 (Red 버튼) → `emit bookmarkAddRequested()`
  - Qt::Key_Menu → `emit settingsRequested()`
  - event->accept() 호출하여 이벤트 처리 완료 표시

- [ ] Task 4-2: `src/ui/HomePage.cpp` 구현 - moveFocusLeft()
  - cards_.isEmpty() 체크
  - newIndex = currentFocusIndex_ - 1
  - newIndex < 0이면 0으로 제한 (경계 체크)
  - currentFocusIndex_ 업데이트
  - cards_[currentFocusIndex_]->setFocus() 호출

- [ ] Task 4-3: `src/ui/HomePage.cpp` 구현 - moveFocusRight()
  - cards_.isEmpty() 체크
  - newIndex = currentFocusIndex_ + 1
  - newIndex >= cards_.size()이면 cards_.size()-1로 제한
  - currentFocusIndex_ 업데이트
  - cards_[currentFocusIndex_]->setFocus() 호출

- [ ] Task 4-4: `src/ui/HomePage.cpp` 구현 - moveFocusUp()
  - cards_.isEmpty() 체크
  - newIndex = currentFocusIndex_ - GRID_COLUMNS (4)
  - newIndex < 0이면 0으로 제한
  - currentFocusIndex_ 업데이트
  - cards_[currentFocusIndex_]->setFocus() 호출

- [ ] Task 4-5: `src/ui/HomePage.cpp` 구현 - moveFocusDown()
  - cards_.isEmpty() 체크
  - newIndex = currentFocusIndex_ + GRID_COLUMNS (4)
  - newIndex >= cards_.size()이면 cards_.size()-1로 제한
  - currentFocusIndex_ 업데이트
  - cards_[currentFocusIndex_]->setFocus() 호출

- [ ] Task 4-6: `src/ui/HomePage.cpp` 구현 - activateCurrentCard()
  - cards_.isEmpty() 또는 currentFocusIndex_ 범위 체크
  - cards_[currentFocusIndex_]->url() 조회
  - qDebug 로그
  - `emit bookmarkSelected(url)` 시그널 발생

**예상 산출물**:
- `src/ui/HomePage.cpp` (keyPressEvent, moveFocus[Left/Right/Up/Down], activateCurrentCard, 약 150줄 추가)

**완료 기준**:
- 방향키 (Left/Right/Up/Down) 입력 시 그리드 아이템 간 포커스 이동
- 4열 그리드에서 좌/우/위/아래 이동 로직 정확성 확인
- Enter 키 입력 시 `bookmarkSelected(url)` 시그널 발생
- Red 버튼 입력 시 `bookmarkAddRequested()` 시그널 발생
- Menu 버튼 입력 시 `settingsRequested()` 시그널 발생
- 그리드 경계에서 방향키 입력 시 포커스 유지 (경계 제한)

**예상 복잡도**: High (4열 그리드 포커스 로직 복잡)
**예상 시간**: 2시간

---

### Phase 5: BrowserWindow 통합

**담당**: general-purpose (C++/Qt 구현)

**작업 내역**:
- [ ] Task 5-1: `src/browser/BrowserWindow.h` 헤더 수정
  - HomePage* 멤버 추가 (`homePage_`)
  - private 슬롯 추가:
    - `void onHomeRequested(const QString &url);`
    - `void onBookmarkSelected(const QString &url);`
  - private 메서드 추가:
    - `void showHomePage();`
    - `void hideHomePage();`

- [ ] Task 5-2: `src/browser/BrowserWindow.cpp` 수정 - 생성자
  - HomePage 인스턴스 생성 (`new HomePage(bookmarkService_, contentWidget_)`)
  - homePage_ 멤버 초기화
  - setupUI() 이전에 생성

- [ ] Task 5-3: `src/browser/BrowserWindow.cpp` 수정 - setupUI()
  - stackedLayout_에 HomePage 추가 (`stackedLayout_->addWidget(homePage_);`)
  - WebView (index 0), ErrorPage (index 1), HomePage (index 2) 순서
  - 초기 WebView 표시 유지

- [ ] Task 5-4: `src/browser/BrowserWindow.cpp` 수정 - setupConnections()
  - NavigationBar::homeRequested 시그널 → BrowserWindow::onHomeRequested 슬롯 연결
  - HomePage::bookmarkSelected 시그널 → BrowserWindow::onBookmarkSelected 슬롯 연결
  - HomePage::bookmarkAddRequested 시그널 → BrowserWindow::onBookmarkButtonClicked 슬롯 연결 (기존 F-07 연동)
  - HomePage::settingsRequested 시그널 → SettingsPanel::show() 연결 (람다)

- [ ] Task 5-5: `src/browser/BrowserWindow.cpp` 구현 - showHomePage()
  - qDebug 로그
  - stackedLayout_->setCurrentWidget(homePage_) 호출
  - urlBar_->setText("about:favorites") 호출
  - homePage_->refreshBookmarks() 호출

- [ ] Task 5-6: `src/browser/BrowserWindow.cpp` 구현 - hideHomePage()
  - qDebug 로그
  - stackedLayout_->setCurrentWidget(webView_) 호출

- [ ] Task 5-7: `src/browser/BrowserWindow.cpp` 구현 - onHomeRequested()
  - qDebug 로그 (url 출력)
  - url == "about:favorites" 체크
    - YES → showHomePage() 호출
    - NO → hideHomePage() + webView_->load(url) 호출

- [ ] Task 5-8: `src/browser/BrowserWindow.cpp` 구현 - onBookmarkSelected()
  - qDebug 로그 (url 출력)
  - hideHomePage() 호출
  - webView_->load(url) 호출
  - TODO 주석: BookmarkService::incrementVisitCount() 호출 (Bookmark ID 필요)

**예상 산출물**:
- `src/browser/BrowserWindow.h` (private 슬롯/메서드 추가, 약 10줄)
- `src/browser/BrowserWindow.cpp` (생성자, setupUI, setupConnections, showHomePage, hideHomePage, onHomeRequested, onBookmarkSelected, 약 80줄 추가)

**완료 기준**:
- BrowserWindow에 HomePage 인스턴스 생성 확인
- stackedLayout_에 HomePage 추가 확인
- NavigationBar, HomePage 시그널 연결 확인
- showHomePage() 호출 시 stackedLayout_이 HomePage로 전환
- onHomeRequested("about:favorites") 호출 시 HomePage 표시
- onHomeRequested("https://www.google.com") 호출 시 WebView 로드
- onBookmarkSelected(url) 호출 시 WebView 로드 및 HomePage 숨김

**예상 복잡도**: Medium
**예상 시간**: 1.5시간

---

### Phase 6: NavigationBar 수정

**담당**: general-purpose (C++/Qt 구현)

**작업 내역**:
- [ ] Task 6-1: `src/ui/NavigationBar.h` 헤더 수정
  - 시그널 추가:
    - `void homeRequested(const QString &url);`
  - private 슬롯 추가:
    - `void onHomepageChanged(const QString &url);`

- [ ] Task 6-2: `src/ui/NavigationBar.cpp` 수정 - onHomeClicked()
  - 기존 코드 수정
  - qDebug 로그 (homepage_ 출력)
  - `emit homeRequested(homepage_);` 시그널 발생
  - 기존 webView_->load(url) 호출 제거 (BrowserWindow에서 처리)

- [ ] Task 6-3: `src/ui/NavigationBar.cpp` 구현 - onHomepageChanged()
  - setHomepage(url) 호출
  - qDebug 로그

- [ ] Task 6-4: `src/browser/BrowserWindow.cpp` 수정 - setupConnections()
  - SettingsService::homepageChanged 시그널 → NavigationBar::onHomepageChanged 슬롯 연결
  - 기존 setupConnections() 메서드에 추가

**예상 산출물**:
- `src/ui/NavigationBar.h` (시그널/슬롯 추가, 약 5줄)
- `src/ui/NavigationBar.cpp` (onHomeClicked 수정, onHomepageChanged 구현, 약 20줄 수정)
- `src/browser/BrowserWindow.cpp` (setupConnections 추가, 약 5줄)

**완료 기준**:
- NavigationBar::onHomeClicked() 호출 시 `homeRequested(homepage_)` 시그널 발생
- SettingsService::homepageChanged 시그널 발생 시 NavigationBar::homepage_ 업데이트
- BrowserWindow에서 homeRequested 시그널 수신 확인

**예상 복잡도**: Low
**예상 시간**: 0.5시간

---

### Phase 7: 빈 상태 및 리모컨 단축키 테스트

**담당**: general-purpose (C++/Qt 구현)

**작업 내역**:
- [ ] Task 7-1: 빈 상태 UI 최종 확인
  - 북마크 0개 시 HomePage에 빈 상태 위젯 표시 확인
  - 아이콘, 메시지, "북마크 추가" 버튼, "홈페이지 설정 변경" 버튼 표시 확인
  - 버튼 클릭 시 시그널 발생 확인

- [ ] Task 7-2: 리모컨 단축키 동작 확인
  - Red 버튼 (0x193) 입력 시 BookmarkPanel 열기 확인 (F-13 연동)
  - Menu 버튼 (Qt::Key_Menu) 입력 시 SettingsPanel 열기 확인 (F-11 연동)
  - Back 버튼 (Qt::Key_Back, Qt::Key_Escape) 입력 시 BrowserWindow로 이벤트 전파 확인
  - 방향키 (Up/Down/Left/Right) 입력 시 그리드 포커스 이동 확인
  - Enter 키 입력 시 북마크 선택 및 페이지 로드 확인

- [ ] Task 7-3: HomePage와 기존 기능 통합 확인
  - F-07 (북마크 관리) 연동 확인: 북마크 추가/삭제 시 HomePage 자동 새로고침
  - F-11 (설정 화면) 연동 확인: 홈페이지 설정 변경 시 홈 버튼 동작 변경
  - F-13 (리모컨 단축키) 연동 확인: Red, Menu 버튼 동작

- [ ] Task 7-4: 빈 상태 버튼 포커스 관리
  - 빈 상태에서 "북마크 추가" 버튼에 초기 포커스 설정
  - Tab 키로 "홈페이지 설정 변경" 버튼으로 포커스 이동
  - Enter 키로 버튼 활성화 확인

**예상 산출물**:
- 빈 상태 UI 동작 확인 (수동 테스트)
- 리모컨 단축키 동작 확인 (수동 테스트)
- 기존 기능 통합 확인 (수동 테스트)

**완료 기준**:
- 북마크 0개 시 빈 상태 UI 표시
- "북마크 추가" 버튼 클릭 시 BookmarkPanel 열기
- "홈페이지 설정 변경" 버튼 클릭 시 SettingsPanel 열기
- Red 버튼 입력 시 BookmarkPanel 열기
- Menu 버튼 입력 시 SettingsPanel 열기
- Back 버튼 입력 시 이전 페이지 또는 아무 동작 없음
- 기존 F-07, F-11, F-13 기능 정상 동작

**예상 복잡도**: Low
**예상 시간**: 1시간

---

### Phase 8: CMakeLists.txt 및 빌드 설정

**담당**: general-purpose (C++/Qt 구현)

**작업 내역**:
- [ ] Task 8-1: CMakeLists.txt 소스 파일 추가
  - set(SOURCES ...) 목록에 추가:
    - `src/ui/HomePage.cpp`
    - `src/ui/BookmarkCard.cpp`

- [ ] Task 8-2: CMakeLists.txt 헤더 파일 추가
  - set(HEADERS ...) 목록에 추가:
    - `src/ui/HomePage.h`
    - `src/ui/BookmarkCard.h`

- [ ] Task 8-3: 빌드 테스트
  - `mkdir build && cd build` (기존 빌드 디렉토리 사용)
  - `cmake ..` 실행
  - `make` 실행
  - 빌드 에러 확인 및 수정
  - 링크 에러 확인 및 수정

- [ ] Task 8-4: 리소스 파일 확인
  - `:resources/icons/default-favicon.png` 파일 존재 확인 (없으면 추가)
  - `:resources/icons/bookmark-empty.png` 파일 존재 확인 (없으면 추가)
  - Qt 리소스 파일 (`.qrc`) 업데이트 필요 시 수정

**예상 산출물**:
- `CMakeLists.txt` (SOURCES, HEADERS 목록 업데이트)
- 빌드 성공 확인
- 리소스 파일 확인 및 추가

**완료 기준**:
- CMakeLists.txt에 HomePage, BookmarkCard 소스/헤더 추가
- `cmake ..` 실행 성공
- `make` 실행 성공 (빌드 에러 없음)
- 리소스 파일 경로 확인 (qrc 파일 업데이트)

**예상 복잡도**: Low
**예상 시간**: 0.5시간

---

### Phase 9: 테스트

**담당**: test-runner (단위 테스트, 통합 테스트)

**작업 내역**:
- [ ] Task 9-1: HomePage 단위 테스트 작성
  - `tests/unit/HomePageTest.cpp` 생성
  - 테스트 케이스:
    - `DisplayEmptyStateWhenNoBookmarks`: 북마크 0개 시 빈 상태 UI 표시
    - `DisplayGridWhenBookmarksExist`: 북마크 1~12개 시 그리드 뷰 표시
    - `MaxBookmarksLimitIs12`: 북마크 13개 이상 시 12개만 표시
    - `RefreshOnBookmarkAdded`: 북마크 추가 시 자동 새로고침
    - `RefreshOnBookmarkDeleted`: 북마크 삭제 시 자동 새로고침
  - MockBookmarkService 사용

- [ ] Task 9-2: BookmarkCard 단위 테스트 작성
  - `tests/unit/BookmarkCardTest.cpp` 생성
  - 테스트 케이스:
    - `EmitClickedSignalOnMouseClick`: 마우스 클릭 시 clicked 시그널 발생
    - `EmitClickedSignalOnEnterKey`: Enter 키 입력 시 clicked 시그널 발생
    - `FocusStyleChange`: 포커스 시 스타일 변경 확인
    - `ElideTextWhenTitleTooLong`: 제목 길 시 말줄임표 처리

- [ ] Task 9-3: 통합 테스트 작성
  - `tests/integration/HomePageIntegrationTest.cpp` 생성
  - 테스트 케이스:
    - `HomeButtonNavigation`: 홈 버튼 클릭 시 HomePage 표시
    - `BookmarkSelection`: Enter 키로 북마크 선택 시 WebView 로드
    - `RedButtonOpensBookmarkPanel`: Red 버튼 입력 시 BookmarkPanel 열기
    - `MenuButtonOpensSettingsPanel`: Menu 버튼 입력 시 SettingsPanel 열기
    - `FocusNavigationInGrid`: 방향키로 그리드 아이템 간 포커스 이동

- [ ] Task 9-4: 리모컨 네비게이션 E2E 테스트 (수동)
  - 실제 webOS 프로젝터 또는 에뮬레이터에서 테스트
  - 체크리스트:
    - [ ] 홈 버튼 클릭 시 HomePage 표시
    - [ ] 방향키로 그리드 아이템 포커스 이동
    - [ ] Enter 키로 북마크 선택 및 페이지 로드
    - [ ] Red 버튼으로 BookmarkPanel 열기
    - [ ] Menu 버튼으로 SettingsPanel 열기
    - [ ] 포커스된 카드 테두리 강조 확인 (파란색)
    - [ ] 북마크 추가 시 HomePage 자동 새로고침
    - [ ] 북마크 삭제 시 HomePage 자동 새로고침

- [ ] Task 9-5: 회귀 테스트
  - 기존 F-07, F-11, F-13 기능 테스트 재실행
  - NavigationBar, WebView, TabManager 동작 확인

**예상 산출물**:
- `tests/unit/HomePageTest.cpp` (약 200줄)
- `tests/unit/BookmarkCardTest.cpp` (약 150줄)
- `tests/integration/HomePageIntegrationTest.cpp` (약 200줄)
- E2E 테스트 체크리스트 (수동)
- 회귀 테스트 결과

**완료 기준**:
- 모든 단위 테스트 통과 (Google Test)
- 모든 통합 테스트 통과 (Qt Test)
- E2E 테스트 체크리스트 모두 완료
- 회귀 테스트 통과 (기존 기능 정상 동작)

**예상 복잡도**: Medium
**예상 시간**: 1시간

---

## 3. 태스크 의존성

```
Phase 1 (BookmarkCard)
  │
  ├──▶ Phase 2 (HomePage 기본 구조)
  │      │
  │      └──▶ Phase 3 (HomePage 데이터 연동)
  │             │
  │             └──▶ Phase 4 (HomePage 리모컨 네비게이션)
  │                    │
  │                    ├──▶ Phase 5 (BrowserWindow 통합)
  │                    │      │
  │                    │      └──▶ Phase 6 (NavigationBar 수정)
  │                    │             │
  │                    │             └──▶ Phase 7 (빈 상태 및 단축키)
  │                    │                    │
  │                    │                    └──▶ Phase 8 (빌드 설정)
  │                    │                           │
  │                    │                           └──▶ Phase 9 (테스트)
  │                    │
  └────────────────────┘
```

**의존성 설명**:
- Phase 1은 독립적으로 시작 가능 (BookmarkCard 단독 개발)
- Phase 2는 Phase 1 완료 후 시작 (HomePage에서 BookmarkCard 사용)
- Phase 3은 Phase 2 완료 후 시작 (setupUI 필요)
- Phase 4는 Phase 3 완료 후 시작 (데이터 연동 필요)
- Phase 5는 Phase 4 완료 후 시작 (HomePage 완성 필요)
- Phase 6은 Phase 5와 병렬 가능하나 순차 권장 (BrowserWindow 통합 후 NavigationBar 수정)
- Phase 7은 Phase 6 완료 후 시작 (통합 완료 후 테스트)
- Phase 8은 Phase 7 완료 후 시작 (빌드 설정 최종 업데이트)
- Phase 9는 Phase 8 완료 후 시작 (빌드 성공 후 테스트)

---

## 4. 병렬 실행 판단

### 분석 결과

**Phase 독립성**:
- Phase 1 (BookmarkCard)는 독립적이나 Phase 2에서 사용하므로 순차 권장
- Phase 2-4 (HomePage)는 강한 의존성 (순차 필수)
- Phase 5-6 (통합)은 BrowserWindow, NavigationBar 수정으로 순차 권장

**파일 충돌**:
- Phase 5, 6에서 BrowserWindow.h/cpp, NavigationBar.h/cpp 동시 수정 → 충돌 위험
- Phase 1-4는 새 파일 생성이므로 충돌 없음

**태스크 수**:
- 전체 태스크: 약 40개 (Phase 1-9 합계)
- 20개 이상이나 순차 의존성이 강해 병렬 실행 부적합

### 결론: **순차 실행 권장**

**이유**:
1. **강한 의존성**: Phase 2-4는 순차 진행 필수 (HomePage 클래스 완성 필요)
2. **파일 충돌 위험**: Phase 5-6에서 BrowserWindow, NavigationBar 동시 수정 시 충돌
3. **단일 UI 컴포넌트**: HomePage는 단일 컴포넌트로 병렬 개발 이점 미미
4. **테스트 단계**: Phase 9는 모든 Phase 완료 후 실행 필수

**Agent Team 사용 권장 여부**: **No**
- 순차 파이프라인 (single general-purpose agent) 권장
- 병렬 팀 파이프라인은 불필요 (의존성 체인이 명확)

---

## 5. 예상 복잡도 및 소요 시간

| Phase | 복잡도 | 예상 시간 | 산출물 |
|-------|--------|-----------|--------|
| Phase 1 (BookmarkCard) | Medium | 1.5시간 | BookmarkCard.h/cpp (300줄) |
| Phase 2 (HomePage 기본 구조) | Medium | 2시간 | HomePage.h (150줄), HomePage.cpp (250줄) |
| Phase 3 (HomePage 데이터 연동) | Medium | 1.5시간 | HomePage.cpp (300줄 추가) |
| Phase 4 (HomePage 리모컨 네비게이션) | High | 2시간 | HomePage.cpp (150줄 추가) |
| Phase 5 (BrowserWindow 통합) | Medium | 1.5시간 | BrowserWindow.h/cpp (90줄 추가) |
| Phase 6 (NavigationBar 수정) | Low | 0.5시간 | NavigationBar.h/cpp (30줄 수정) |
| Phase 7 (빈 상태 및 단축키) | Low | 1시간 | 수동 테스트 확인 |
| Phase 8 (빌드 설정) | Low | 0.5시간 | CMakeLists.txt 업데이트 |
| Phase 9 (테스트) | Medium | 1시간 | 단위/통합/E2E 테스트 (550줄) |
| **총계** | **Medium-High** | **11.5시간** | **약 1,500줄** |

**주요 복잡도 요인**:
- **Phase 4 (리모컨 네비게이션)**: 4열 그리드 포커스 이동 로직 복잡 (경계 체크, 인덱스 계산)
- **Phase 2-3 (HomePage 데이터 연동)**: BookmarkService 비동기 호출, 그리드 재구성 로직
- **Phase 5 (BrowserWindow 통합)**: QStackedLayout 통합, 시그널/슬롯 연결 다수

---

## 6. 위험 요소 및 대응 방안

### 위험 1: 4열 그리드 포커스 이동 로직 복잡성

**설명**:
- moveFocusLeft/Right/Up/Down 메서드에서 인덱스 계산 오류 가능
- 그리드 경계에서 방향키 입력 시 포커스 이탈 또는 앱 크래시 위험

**대응 방안**:
- BookmarkPanel 구현 참고 (F-07에서 리스트 뷰 포커스 관리 경험)
- 포커스 인덱스 계산 명확히 (row = index / 4, col = index % 4)
- 경계 체크 강화 (newIndex < 0 → 0, newIndex >= size → size-1)
- 단위 테스트 작성 (FocusNavigationInGrid)

**영향도**: High (리모컨 UX 핵심)
**발생 가능성**: Medium

---

### 위험 2: QStackedLayout에 HomePage 추가 시 WebView 기능 영향

**설명**:
- stackedLayout_에 HomePage 추가 시 기존 WebView, ErrorPage 동작에 회귀 가능
- 위젯 전환 시 포커스 관리 문제 (예: HomePage에서 WebView로 전환 시 URLBar 포커스 이탈)

**대응 방안**:
- 기존 WebView, ErrorPage와 동일한 패턴 사용 (stackedLayout_->addWidget)
- showHomePage(), hideHomePage() 메서드에서 명시적 위젯 전환
- 회귀 테스트 실행 (F-02, F-10 기능 확인)
- 포커스 관리: HomePage 표시 시 첫 번째 BookmarkCard에 자동 포커스

**영향도**: High (기존 기능 영향)
**발생 가능성**: Low (기존 패턴 재사용)

---

### 위험 3: NavigationBar 시그널 변경으로 인한 F-11 회귀

**설명**:
- NavigationBar::onHomeClicked() 수정 시 기존 F-11 (설정 화면) 동작에 영향 가능
- homeRequested 시그널 추가 시 기존 코드와 충돌 가능

**대응 방안**:
- F-11 설정 화면 테스트 재실행 (SettingsPanel 열기, 홈페이지 설정 변경)
- onHomeClicked() 수정 시 기존 webView_->load(url) 제거 (BrowserWindow에서 처리)
- SettingsService::homepageChanged 시그널 연결 확인

**영향도**: Medium (F-11 기능 영향)
**발생 가능성**: Low (단순 시그널 추가)

---

### 위험 4: 북마크 데이터 로드 실패 시 에러 처리

**설명**:
- BookmarkService::getAllBookmarks() 호출 실패 시 HomePage에 빈 화면 또는 에러 표시 필요
- 네트워크 오류, LS2 API 오류 등으로 데이터 조회 실패 가능

**대응 방안**:
- onBookmarksLoaded(bool success, ...) 메서드에서 success 체크
- 실패 시 displayEmptyState() 호출 + 에러 메시지 표시 (빈 상태 위젯 재활용)
- qWarning 로그 추가 ("HomePage: 북마크 로드 실패")

**영향도**: Low (비보안 사이트 경고 수준)
**발생 가능성**: Low (BookmarkService 이미 안정적)

---

## 7. 의존성 체크리스트

- [x] **F-07 (북마크 관리) 완료** → BookmarkService::getAllBookmarks() 사용 가능
  - `BookmarkService::getAllBookmarks(callback)` API 사용
  - `BookmarkService::bookmarkAdded/Deleted/Updated` 시그널 사용

- [x] **F-11 (설정 화면) 완료** → SettingsService::homepage() 사용 가능
  - `SettingsService::homepage()` API로 홈페이지 URL 조회
  - `SettingsService::homepageChanged` 시그널로 변경 감지

- [x] **F-04 (페이지 탐색 컨트롤) 완료** → NavigationBar::onHomeClicked() 수정 가능
  - NavigationBar::homeRequested 시그널 추가
  - onHomeClicked() 메서드 수정

- [x] **F-13 (리모컨 단축키) 완료** → Red, Back, Menu 버튼 사용 가능
  - 0x193 (Red 버튼) → BookmarkPanel 열기
  - Qt::Key_Menu → SettingsPanel 열기
  - Qt::Key_Back / Qt::Key_Escape → 이전 페이지

- [x] **F-06 (탭 관리) 완료** → TabManager 연동 (새 탭 생성 시 HomePage 표시)
  - 새 탭 생성 시 SettingsService::homepage() 확인
  - "about:favorites"이면 HomePage 표시

---

## 8. 인수 기준 (Acceptance Criteria)

### AC-1: 홈 화면 표시 조건
- [ ] NavigationBar 홈 버튼 클릭 시 SettingsService::homepage() 확인
- [ ] 홈페이지 설정이 "about:favorites"이면 HomePage 표시
- [ ] 홈페이지 설정이 그 외 URL이면 WebView::load(url) 호출
- [ ] 새 탭 생성 시 홈페이지 설정에 따라 HomePage 또는 URL 로드
- [ ] 앱 시작 시 첫 탭은 홈페이지 설정에 따라 표시
- [ ] URLBar에 "about:favorites" 표시

### AC-2: 그리드 뷰 UI
- [ ] BookmarkService에서 최대 12개 북마크 조회
- [ ] 북마크를 4열 × N행 그리드로 표시 (QGridLayout)
- [ ] 각 BookmarkCard에 파비콘(기본 아이콘), 제목 표시
- [ ] 카드 크기: 200×180px, 간격 16px
- [ ] 포커스된 카드는 테두리 강조 (2px 파란색 #0078D7)

### AC-3: 북마크 데이터 연동
- [ ] HomePage 초기화 시 BookmarkService::getAllBookmarks() 호출
- [ ] 북마크 추가 시 자동 새로고침 (bookmarkAdded 시그널)
- [ ] 북마크 삭제 시 자동 새로고침 (bookmarkDeleted 시그널)
- [ ] 북마크 업데이트 시 자동 새로고침 (bookmarkUpdated 시그널)

### AC-4: 빈 상태 처리
- [ ] 북마크 0개일 때 빈 상태 UI 표시
- [ ] "북마크를 추가하여..." 안내 메시지 표시
- [ ] "북마크 추가 (Red 버튼)" 버튼 표시
- [ ] "홈페이지 설정 변경 (Menu 버튼)" 버튼 표시
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
- [ ] Back 키 입력 시 BrowserWindow로 이벤트 전파 (브라우저 뒤로 가기)
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

## 9. 리스크 매트릭스

| 리스크 | 영향도 | 발생 가능성 | 대응 방안 | 책임자 |
|--------|--------|-------------|-----------|--------|
| 4열 그리드 포커스 로직 복잡성 | High | Medium | BookmarkPanel 참고, 단위 테스트 강화 | general-purpose |
| QStackedLayout 통합 시 WebView 영향 | High | Low | 기존 패턴 재사용, 회귀 테스트 | general-purpose |
| NavigationBar 시그널 변경 회귀 | Medium | Low | F-11 테스트 재실행 | test-runner |
| 북마크 데이터 로드 실패 | Low | Low | 에러 처리 추가, 빈 상태 재활용 | general-purpose |

---

## 10. 다음 단계

### 이 계획서가 승인되면:

1. **순차 실행**: general-purpose 에이전트 (C++/Qt 구현)로 진행
2. **실행 명령**: `/fullstack-feature F-15: 즐겨찾기 홈 화면 구현`
3. **파이프라인**:
   - Phase 1 → Phase 2 → Phase 3 → Phase 4 → Phase 5 → Phase 6 → Phase 7 → Phase 8 → Phase 9 (순차)
4. **진행 로그**: `docs/dev-log.md`에 자동 기록
5. **CHANGELOG**: `CHANGELOG.md`에 F-15 완료 기록

### 직접 구현 시작 (수동):

1. Phase 1부터 순차적으로 Task 실행
2. 각 Phase 완료 후 git commit (Conventional Commits)
3. Phase 9 완료 후 code-reviewer에게 리뷰 요청
4. 리뷰 통과 후 `features.md`에서 F-15 상태를 ✅ 완료로 변경

---

## 11. 예상 산출물 요약

### 신규 파일
- `src/ui/HomePage.h` (150줄)
- `src/ui/HomePage.cpp` (700줄)
- `src/ui/BookmarkCard.h` (100줄)
- `src/ui/BookmarkCard.cpp` (200줄)
- `tests/unit/HomePageTest.cpp` (200줄)
- `tests/unit/BookmarkCardTest.cpp` (150줄)
- `tests/integration/HomePageIntegrationTest.cpp` (200줄)

### 수정 파일
- `src/browser/BrowserWindow.h` (+10줄)
- `src/browser/BrowserWindow.cpp` (+80줄)
- `src/ui/NavigationBar.h` (+5줄)
- `src/ui/NavigationBar.cpp` (+20줄)
- `CMakeLists.txt` (+4줄)

### 총 코드 라인 수
- 신규: 약 1,500줄
- 수정: 약 120줄
- **총계: 약 1,620줄**

---

## 12. 참고 자료

### 관련 문서
- **요구사항 분석서**: `docs/specs/favorites-home/requirements.md`
- **기술 설계서**: `docs/specs/favorites-home/design.md`
- **F-07 북마크 관리**: `docs/specs/bookmark-management/`
- **F-11 설정 화면**: `docs/specs/settings/`
- **F-13 리모컨 단축키**: `docs/specs/remote-shortcuts/`
- **CLAUDE.md**: 프로젝트 규칙 및 코딩 컨벤션

### Qt 문서
- **QStackedLayout**: https://doc.qt.io/qt-5/qstackedlayout.html
- **QGridLayout**: https://doc.qt.io/qt-5/qgridlayout.html
- **QWidget Focus Policy**: https://doc.qt.io/qt-5/qwidget.html#focusPolicy-prop

### 참고 구현
- **Chrome 새 탭 페이지**: 자주 방문한 사이트 그리드 뷰
- **Firefox about:home**: Top Sites 그리드 레이아웃
- **Opera Speed Dial**: 북마크 그리드 + 썸네일

---

## 13. 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|-----------|--------|
| 2026-02-15 | 1.0 | F-15 즐겨찾기 홈 화면 구현 계획서 초안 작성 | product-manager |

---

**계획 완료**: 2026-02-15
**계획자**: product-manager (Claude Code Agent)
**검토자**: TBD (architect, code-reviewer)
**승인자**: TBD (사용자)
