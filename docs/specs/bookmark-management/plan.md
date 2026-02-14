# 북마크 관리 — 구현 계획서

## 1. 참조 문서
- **요구사항 분석서**: `docs/specs/bookmark-management/requirements.md`
- **기술 설계서**: `docs/specs/bookmark-management/design.md`
- **프로젝트 가이드**: `CLAUDE.md`
- **기능 백로그**: `docs/project/features.md`

---

## 2. 개요

### 2.1 기능 요약
F-07 북마크 관리 기능은 사용자가 자주 방문하는 웹사이트를 북마크로 저장하고, 폴더별로 구조화하여 관리할 수 있는 기능입니다. C++/Qt Native App으로 구현하며, webOS LS2 API(DB8)를 통해 데이터를 영속적으로 저장합니다. 리모컨 전용 입력 환경에 최적화된 UX를 제공합니다.

### 2.2 구현 목표
- 북마크 CRUD (추가/조회/편집/삭제) 기능 완전 구현
- 1단계 서브폴더 지원 (폴더 생성/이름변경/삭제)
- 리모컨 방향키 최적화 UI (QTreeWidget 기반 계층 구조)
- LS2 API 비동기 호출로 메인 스레드 블로킹 방지
- 메모리 캐싱 전략으로 북마크 조회 성능 최적화 (1초 이내)
- Qt Signal/Slot 시스템으로 느슨한 결합 아키텍처 구현

### 2.3 핵심 산출물
- **데이터 모델**: `src/models/Bookmark.h` (헤더 온리)
- **서비스 계층**: `src/services/StorageService.h/.cpp`, `src/services/BookmarkService.h/.cpp`
- **UI 계층**: `src/ui/BookmarkPanel.h/.cpp`, `src/ui/BookmarkDialog.h/.cpp`
- **통합**: `src/browser/BrowserWindow.h/.cpp`, `src/ui/NavigationBar.h/.cpp` 수정

---

## 3. 구현 Phase

### Phase 1: 데이터 모델 및 StorageService 기반 구축
**담당**: cpp-dev
**예상 기간**: 2일
**의존성**: F-01 완료 필요

#### 태스크
- [ ] **Task 1.1**: `src/models/Bookmark.h` 생성
  - Bookmark 구조체 정의 (id, title, url, folderId, favicon, createdAt, updatedAt, visitCount)
  - Folder 구조체 정의 (id, name, parentId, createdAt)
  - toJson(), fromJson() 메서드 구현
  - 기본 생성자, 편의 생성자 구현
- [ ] **Task 1.2**: `src/services/StorageService.h/.cpp` 생성
  - LS2 API 래퍼 클래스 구현 (put, find, del 메서드)
  - 비동기 호출 시그널/슬롯 (putSuccess, findSuccess, delSuccess, error)
  - luna-service2 메시지 버스 연동
  - JSON 직렬화/역직렬화 (QJsonObject, QJsonArray)
- [ ] **Task 1.3**: `webos-meta/appinfo.json` 권한 추가
  - requiredPermissions: ["db8.find", "db8.put", "db8.del"]

#### 산출물
- `src/models/Bookmark.h`
- `src/services/StorageService.h`
- `src/services/StorageService.cpp`
- `webos-meta/appinfo.json` (수정)

#### 완료 기준
- Bookmark/Folder 구조체가 JSON 직렬화/역직렬화 가능
- StorageService::put() 호출 시 LS2 API로 데이터 저장 성공
- StorageService::find() 호출 시 LS2 API에서 데이터 조회 성공
- 비동기 시그널/슬롯 동작 확인 (putSuccess, findSuccess 시그널 emit)

---

### Phase 2: BookmarkService 비즈니스 로직 구현
**담당**: cpp-dev
**예상 기간**: 3일
**의존성**: Phase 1 완료

#### 태스크
- [ ] **Task 2.1**: `src/services/BookmarkService.h/.cpp` 생성
  - 클래스 기본 구조 구현 (생성자, 소멸자, Q_OBJECT)
  - StorageService 의존성 주입 (생성자 파라미터)
- [ ] **Task 2.2**: 북마크 CRUD 메서드 구현
  - `addBookmark(title, url, folderId)`: 중복 체크 후 추가
  - `updateBookmark(bookmarkId, title, url, folderId)`: 기존 북마크 업데이트
  - `deleteBookmark(bookmarkId)`: 북마크 삭제
  - `getBookmark(bookmarkId)`: 북마크 조회 (std::optional 반환)
  - `getAllBookmarks()`: 모든 북마크 조회 (QVector<Bookmark> 반환)
  - `getBookmarksByFolder(folderId)`: 폴더별 북마크 조회
  - `exists(url)`: URL 중복 체크 (대소문자 무시)
- [ ] **Task 2.3**: 폴더 관리 메서드 구현
  - `createFolder(name, parentId)`: 폴더 생성
  - `renameFolder(folderId, newName)`: 폴더 이름 변경
  - `deleteFolder(folderId, moveToRoot)`: 폴더 삭제 (하위 북마크 처리)
  - `getFolder(folderId)`: 폴더 조회 (std::optional 반환)
  - `getAllFolders()`: 모든 폴더 조회 (QVector<Folder> 반환)
- [ ] **Task 2.4**: 메모리 캐싱 구현
  - `QMap<QString, Bookmark> bookmarks_` 멤버 변수
  - `QMap<QString, Folder> folders_` 멤버 변수
  - `initialize()`: 앱 시작 시 LS2 API에서 데이터 로드 → 메모리 캐시
  - `loadBookmarksFromStorage()`, `loadFoldersFromStorage()` 프라이빗 메서드
- [ ] **Task 2.5**: 시그널/슬롯 구현
  - 시그널: bookmarkAdded, bookmarkUpdated, bookmarkDeleted, folderAdded, folderUpdated, folderDeleted, initialized, error
  - 슬롯: onFindSuccess, onPutSuccess, onDelSuccess, onStorageError
  - StorageService 시그널과 BookmarkService 슬롯 연결

#### 산출물
- `src/services/BookmarkService.h`
- `src/services/BookmarkService.cpp`

#### 완료 기준
- BookmarkService::addBookmark() 호출 시 LS2 API 저장 + 메모리 캐시 업데이트
- BookmarkService::getAllBookmarks() 호출 시 메모리 캐시에서 즉시 반환 (1ms 이내)
- BookmarkService::exists() 호출 시 중복 URL 정확히 검출 (대소문자 무시)
- BookmarkService::deleteFolder() 호출 시 하위 북마크 cascade 삭제 또는 루트 이동 정상 동작
- BookmarkService::initialized 시그널 emit 확인 (앱 시작 시)

---

### Phase 3: BookmarkPanel UI 구현
**담당**: cpp-dev
**예상 기간**: 4일
**의존성**: Phase 2 완료

#### 태스크
- [ ] **Task 3.1**: `src/ui/BookmarkPanel.h/.cpp` 생성
  - 클래스 기본 구조 구현 (생성자, 소멸자, Q_OBJECT)
  - BookmarkService 의존성 주입 (생성자 파라미터)
- [ ] **Task 3.2**: UI 레이아웃 구성
  - `QVBoxLayout *mainLayout_`: 메인 레이아웃
  - `QHBoxLayout *topLayout_`: 상단 레이아웃 (검색, 새 폴더 버튼)
  - `QLineEdit *searchEdit_`: 검색 입력 필드
  - `QPushButton *newFolderButton_`: 새 폴더 버튼
  - `QTreeWidget *treeWidget_`: 북마크/폴더 트리 위젯 (계층 구조)
  - `QLabel *emptyLabel_`: 빈 목록 안내 라벨
  - `QPushButton *closeButton_`: 닫기 버튼
- [ ] **Task 3.3**: 북마크 목록 로딩
  - `loadBookmarks()`: BookmarkService에서 북마크 조회 → QTreeWidget 표시
  - `loadFolders()`: BookmarkService에서 폴더 조회 → QTreeWidget 루트 아이템 표시
  - `createBookmarkItem(Bookmark)`: QTreeWidgetItem 생성 (아이콘, 제목, URL)
  - `createFolderItem(Folder)`: QTreeWidgetItem 생성 (폴더 아이콘, 이름)
  - 폴더 구조 시각화 (1단계 서브폴더, QTreeWidgetItem::addChild())
- [ ] **Task 3.4**: 리모컨 키 이벤트 처리
  - `keyPressEvent(QKeyEvent)` 오버라이드
  - Qt::Key_Up/Down: 북마크 목록 스크롤 (QTreeWidget::setCurrentItem())
  - Qt::Key_Left: 폴더 접기 (QTreeWidget::collapseItem())
  - Qt::Key_Right: 폴더 펼치기 (QTreeWidget::expandItem())
  - Qt::Key_Return/Enter: 북마크 실행 (emit bookmarkSelected(url))
  - Qt::Key_Escape/Back: 패널 닫기 (hide())
  - Qt::Key_Delete: 북마크 삭제 (onDeleteBookmark())
  - Qt::Key_Menu/Options: 컨텍스트 메뉴 (onContextMenuRequested())
- [ ] **Task 3.5**: 컨텍스트 메뉴 구현
  - `createContextMenu(QTreeWidgetItem, QPoint)`: QMenu 생성
  - 메뉴 항목: "페이지 열기", "새 탭에서 열기", "편집", "삭제", "폴더 이름 변경", "폴더 삭제"
  - 북마크 항목/폴더 항목에 따라 메뉴 항목 동적 표시
- [ ] **Task 3.6**: 검색 필터링
  - `onSearchTextChanged(QString)`: QLineEdit::textChanged 시그널 연결
  - `filterBookmarks(QString)`: 제목/URL 일치 북마크 필터링 (QString::contains())
  - 필터링된 결과만 QTreeWidget 표시 (QTreeWidgetItem::setHidden())
- [ ] **Task 3.7**: 슬라이드 애니메이션
  - `show()`: QPropertyAnimation으로 슬라이드 인 (x 위치 -600 → 0)
  - `hide()`: QPropertyAnimation으로 슬라이드 아웃 (x 위치 0 → -600)
- [ ] **Task 3.8**: Qt Stylesheet 적용
  - `applyStyles()`: 다크 테마 스타일 (배경: #2b2b2b, 텍스트: #ffffff)
  - 포커스 스타일 (테두리 3px, 색상: #0078d7)
  - 폰트 크기 (제목 20px, URL 16px)
- [ ] **Task 3.9**: BookmarkService 시그널 연결
  - bookmarkAdded → onBookmarkAdded: QTreeWidget에 항목 추가
  - bookmarkUpdated → onBookmarkUpdated: QTreeWidget 항목 업데이트
  - bookmarkDeleted → onBookmarkDeleted: QTreeWidget 항목 제거
  - folderAdded → onFolderAdded: QTreeWidget에 폴더 추가
  - folderUpdated → onFolderUpdated: QTreeWidget 폴더 업데이트
  - folderDeleted → onFolderDeleted: QTreeWidget 폴더 제거

#### 산출물
- `src/ui/BookmarkPanel.h`
- `src/ui/BookmarkPanel.cpp`

#### 완료 기준
- BookmarkPanel::show() 호출 시 슬라이드 인 애니메이션 동작
- 리모컨 방향키로 북마크 목록 스크롤 가능 (포커스 이동)
- Enter 키로 북마크 선택 시 bookmarkSelected(url) 시그널 emit
- 컨텍스트 메뉴 (Options 키)로 편집/삭제 액션 실행 가능
- 검색 입력 시 제목/URL 필터링 실시간 동작
- BookmarkService::bookmarkAdded 시그널 수신 시 QTreeWidget 증분 업데이트 (전체 새로고침 없음)

---

### Phase 4: BookmarkDialog UI 구현
**담당**: cpp-dev
**예상 기간**: 2일
**의존성**: Phase 2 완료 (Phase 3과 병렬 가능)

#### 태스크
- [ ] **Task 4.1**: `src/ui/BookmarkDialog.h/.cpp` 생성
  - 클래스 기본 구조 구현 (생성자, 소멸자, Q_OBJECT)
  - Mode enum (Add, Edit)
  - BookmarkService 의존성 주입 (생성자 파라미터)
- [ ] **Task 4.2**: UI 레이아웃 구성
  - `QVBoxLayout *mainLayout_`: 메인 레이아웃
  - `QFormLayout *formLayout_`: 폼 레이아웃
  - `QLineEdit *titleEdit_`: 제목 입력
  - `QLineEdit *urlEdit_`: URL 입력
  - `QComboBox *folderComboBox_`: 폴더 선택 드롭다운
  - `QTextEdit *descriptionEdit_`: 설명 입력 (선택)
  - `QHBoxLayout *buttonLayout_`: 버튼 레이아웃
  - `QPushButton *acceptButton_`: 확인 버튼
  - `QPushButton *rejectButton_`: 취소 버튼
  - `QLabel *errorLabel_`: 에러 메시지 라벨
- [ ] **Task 4.3**: 폴더 드롭다운 로딩
  - `loadFolders()`: BookmarkService::getAllFolders() 호출 → QComboBox에 추가
  - "루트 폴더" 기본 항목 추가 (folderId == "")
- [ ] **Task 4.4**: URL 유효성 검증
  - `onUrlTextChanged(QString)`: QLineEdit::textChanged 시그널 연결
  - URLValidator::isValid() 호출 (유효하지 않으면 QLineEdit 배경색 빨강)
  - `validate()`: 제목 비어있지 않음, URL 유효함 체크
  - 유효하지 않으면 acceptButton_ 비활성화 (QPushButton::setEnabled(false))
- [ ] **Task 4.5**: 확인/취소 액션
  - `onAcceptClicked()`: validate() → BookmarkService::addBookmark() 또는 updateBookmark() 호출
  - 성공 시 QDialog::accept() → 다이얼로그 닫기
  - 실패 시 errorLabel_ 표시 ("이미 북마크에 추가된 페이지입니다")
  - `onRejectClicked()`: QDialog::reject() → 다이얼로그 닫기
- [ ] **Task 4.6**: 리모컨 키 이벤트 처리
  - `keyPressEvent(QKeyEvent)` 오버라이드
  - Qt::Key_Up/Down: 입력 필드 간 포커스 이동 (titleEdit_ → urlEdit_ → folderComboBox_)
  - Qt::Key_Return/Enter: 확인 버튼 클릭 (onAcceptClicked())
  - Qt::Key_Escape/Back: 취소 버튼 클릭 (onRejectClicked())
- [ ] **Task 4.7**: Qt Stylesheet 적용
  - `applyStyles()`: 다크 테마 스타일
  - 입력 필드 포커스 스타일 (테두리 3px, 색상: #0078d7)
  - 에러 메시지 라벨 (색상: 빨강, 폰트 크기: 14px)

#### 산출물
- `src/ui/BookmarkDialog.h`
- `src/ui/BookmarkDialog.cpp`

#### 완료 기준
- Add 모드: 제목/URL 입력 → 확인 버튼 클릭 시 BookmarkService::addBookmark() 호출
- Edit 모드: 기존 제목/URL 자동 입력 → 수정 후 확인 버튼 클릭 시 BookmarkService::updateBookmark() 호출
- URL 유효성 검증: 잘못된 URL 입력 시 acceptButton_ 비활성화
- 중복 URL 입력 시 errorLabel_ 표시 ("이미 북마크에 추가된 페이지입니다")
- 리모컨 방향키로 입력 필드 간 포커스 이동 가능

---

### Phase 5: BrowserWindow/NavigationBar 통합
**담당**: cpp-dev
**예상 기간**: 1일
**의존성**: Phase 3, Phase 4 완료

#### 태스크
- [ ] **Task 5.1**: `src/browser/BrowserWindow.h/.cpp` 수정
  - `StorageService *storageService_` 멤버 변수 추가
  - `BookmarkService *bookmarkService_` 멤버 변수 추가
  - `BookmarkPanel *bookmarkPanel_` 멤버 변수 추가
  - 생성자에서 StorageService, BookmarkService, BookmarkPanel 생성
  - `setupConnections()`에서 시그널/슬롯 연결:
    - BookmarkPanel::bookmarkSelected → BrowserWindow::onBookmarkSelected
    - BrowserWindow::onBookmarkSelected → WebView::load(QUrl)
- [ ] **Task 5.2**: `src/ui/NavigationBar.h/.cpp` 수정
  - `QPushButton *bookmarkButton_` 멤버 변수 추가
  - NavigationBar 레이아웃에 북마크 버튼 추가 (아이콘: ⭐)
  - 북마크 버튼 클릭 시 emit showBookmarkPanel() 시그널
  - BrowserWindow에서 NavigationBar::showBookmarkPanel → BookmarkPanel::show() 연결
- [ ] **Task 5.3**: `src/ui/NavigationBar.h/.cpp` 북마크 추가 버튼
  - `QPushButton *addBookmarkButton_` 멤버 변수 추가 (또는 bookmarkButton_ 우클릭)
  - NavigationBar 레이아웃에 북마크 추가 버튼 배치 (아이콘: ⭐+)
  - 북마크 추가 버튼 클릭 시:
    - WebView::title(), WebView::url() 조회
    - BookmarkDialog 생성 (Mode::Add)
    - titleEdit_, urlEdit_ 자동 입력
    - BookmarkDialog::exec() → 모달 다이얼로그 표시
- [ ] **Task 5.4**: 리모컨 컬러 버튼 매핑
  - BrowserWindow::keyPressEvent() 오버라이드
  - 빨강 버튼 (0x193): 북마크 추가 다이얼로그 표시
  - 파랑 버튼 (0x195): 북마크 패널 표시 (BookmarkPanel::show())
  - webOS 리모컨 키코드 → Qt::Key_* 매핑 확인
- [ ] **Task 5.5**: CMakeLists.txt 업데이트
  - BookmarkService.cpp, BookmarkPanel.cpp, BookmarkDialog.cpp 추가
  - Bookmark.h 헤더 경로 추가 (include_directories)
  - Qt5::Widgets 모듈 링크 (이미 있으면 생략)

#### 산출물
- `src/browser/BrowserWindow.h` (수정)
- `src/browser/BrowserWindow.cpp` (수정)
- `src/ui/NavigationBar.h` (수정)
- `src/ui/NavigationBar.cpp` (수정)
- `CMakeLists.txt` (수정)

#### 완료 기준
- NavigationBar 북마크 버튼 클릭 시 BookmarkPanel 표시
- NavigationBar 북마크 추가 버튼 클릭 시 BookmarkDialog 표시 (제목/URL 자동 입력)
- BookmarkPanel에서 북마크 선택 시 WebView에서 페이지 로드
- 리모컨 빨강 버튼으로 북마크 추가 다이얼로그 즉시 표시
- 리모컨 파랑 버튼으로 북마크 패널 즉시 표시

---

### Phase 6: 단위 테스트 및 통합 테스트
**담당**: test-runner
**예상 기간**: 2일
**의존성**: Phase 5 완료

#### 태스크
- [ ] **Task 6.1**: BookmarkService 단위 테스트 (Google Test)
  - `tests/unit/BookmarkServiceTest.cpp` 생성
  - addBookmark() 테스트: 정상 추가, 중복 URL 체크
  - updateBookmark() 테스트: 기존 북마크 업데이트
  - deleteBookmark() 테스트: 북마크 삭제
  - getAllBookmarks() 테스트: 북마크 조회
  - exists() 테스트: 중복 URL 검증 (대소문자 무시)
  - createFolder(), renameFolder(), deleteFolder() 테스트
  - StorageService 모킹 (MockStorageService 클래스)
- [ ] **Task 6.2**: BookmarkPanel UI 테스트 (Qt Test)
  - `tests/integration/BookmarkPanelTest.cpp` 생성
  - QTreeWidget 표시 테스트: loadBookmarks() 호출 후 항목 개수 확인
  - 키 이벤트 테스트: Qt::Key_Down → currentItem() 변경 확인
  - bookmarkSelected 시그널 테스트: Enter 키 → 시그널 emit 확인
  - QTestEventList로 리모컨 입력 시뮬레이션
- [ ] **Task 6.3**: BookmarkDialog UI 테스트 (Qt Test)
  - `tests/integration/BookmarkDialogTest.cpp` 생성
  - Add 모드 테스트: titleEdit_, urlEdit_ 입력 → addBookmark() 호출 확인
  - Edit 모드 테스트: 기존 값 자동 입력 → updateBookmark() 호출 확인
  - URL 유효성 검증 테스트: 잘못된 URL → acceptButton_ 비활성화
- [ ] **Task 6.4**: 통합 테스트 (전체 플로우)
  - `tests/integration/BookmarkFlowTest.cpp` 생성
  - 북마크 추가 → 목록 조회 → 실행 → 페이지 로드 전체 플로우
  - 폴더 생성 → 북마크 이동 → 폴더 삭제 플로우
  - LS2 API 모킹 (MockStorageService 사용)

#### 산출물
- `tests/unit/BookmarkServiceTest.cpp`
- `tests/integration/BookmarkPanelTest.cpp`
- `tests/integration/BookmarkDialogTest.cpp`
- `tests/integration/BookmarkFlowTest.cpp`
- `tests/mocks/MockStorageService.h/.cpp` (LS2 API 모킹)

#### 완료 기준
- BookmarkService 단위 테스트 통과율 100%
- BookmarkPanel, BookmarkDialog UI 테스트 통과율 100%
- 통합 테스트 (전체 플로우) 통과
- 테스트 커버리지 80% 이상 (gcov, lcov)

---

### Phase 7: 코드 리뷰 및 문서화
**담당**: code-reviewer, doc-writer
**예상 기간**: 1일
**의존성**: Phase 6 완료

#### 태스크
- [ ] **Task 7.1**: code-reviewer 에이전트 리뷰
  - C++ 코딩 컨벤션 준수 확인 (CLAUDE.md 기준)
  - 메모리 관리 적절성 확인 (Qt 부모-자식 메모리 관리, 스마트 포인터)
  - 에러 처리 완전성 확인 (LS2 API 에러, 중복 URL, 잘못된 입력)
  - Qt Signal/Slot 연결 올바름 확인
  - 로깅 적절성 확인 (Logger::info(), Logger::error())
- [ ] **Task 7.2**: 리뷰 피드백 반영
  - code-reviewer 지적사항 수정
  - 테스트 재실행 (회귀 테스트)
- [ ] **Task 7.3**: API 문서 작성 (Doxygen 주석)
  - BookmarkService 클래스 공개 메서드 Doxygen 주석 추가
  - BookmarkPanel, BookmarkDialog 클래스 Doxygen 주석 추가
  - Bookmark.h 구조체 Doxygen 주석 추가
- [ ] **Task 7.4**: 개발 로그 및 CHANGELOG 업데이트
  - doc-writer 에이전트 호출
  - `docs/dev-log.md`: F-07 구현 과정 기록
  - `CHANGELOG.md`: F-07 완료 항목 추가 (버전, 날짜, 변경 내용)
- [ ] **Task 7.5**: README 업데이트
  - F-07 기능 완료 명시
  - 북마크 관리 기능 사용법 간략 설명

#### 산출물
- 코드 리뷰 피드백 문서 (comment 또는 PR 형태)
- Doxygen 주석 추가된 소스 코드
- `docs/dev-log.md` (업데이트)
- `CHANGELOG.md` (업데이트)
- `README.md` (업데이트)

#### 완료 기준
- code-reviewer 승인 완료
- 모든 public 메서드에 Doxygen 주석 추가
- dev-log.md, CHANGELOG.md, README.md 업데이트 완료

---

## 4. 파일 작업 목록

### 4.1 새로 생성할 파일

| 파일 경로 | 역할 | Phase |
|-----------|------|-------|
| `src/models/Bookmark.h` | 북마크/폴더 구조체 정의 (헤더 온리) | Phase 1 |
| `src/services/StorageService.h` | LS2 API 래퍼 헤더 | Phase 1 |
| `src/services/StorageService.cpp` | LS2 API 래퍼 구현 | Phase 1 |
| `src/services/BookmarkService.h` | 북마크 서비스 헤더 | Phase 2 |
| `src/services/BookmarkService.cpp` | 북마크 서비스 구현 | Phase 2 |
| `src/ui/BookmarkPanel.h` | 북마크 패널 헤더 | Phase 3 |
| `src/ui/BookmarkPanel.cpp` | 북마크 패널 구현 | Phase 3 |
| `src/ui/BookmarkDialog.h` | 북마크 다이얼로그 헤더 | Phase 4 |
| `src/ui/BookmarkDialog.cpp` | 북마크 다이얼로그 구현 | Phase 4 |
| `tests/unit/BookmarkServiceTest.cpp` | BookmarkService 단위 테스트 | Phase 6 |
| `tests/integration/BookmarkPanelTest.cpp` | BookmarkPanel UI 테스트 | Phase 6 |
| `tests/integration/BookmarkDialogTest.cpp` | BookmarkDialog UI 테스트 | Phase 6 |
| `tests/integration/BookmarkFlowTest.cpp` | 전체 플로우 통합 테스트 | Phase 6 |
| `tests/mocks/MockStorageService.h` | LS2 API 모킹 헤더 | Phase 6 |
| `tests/mocks/MockStorageService.cpp` | LS2 API 모킹 구현 | Phase 6 |

### 4.2 수정할 기존 파일

| 파일 경로 | 변경 내용 | Phase |
|-----------|-----------|-------|
| `webos-meta/appinfo.json` | LS2 API 권한 추가 (db8.find, db8.put, db8.del) | Phase 1 |
| `src/browser/BrowserWindow.h` | BookmarkPanel, BookmarkService 포인터 추가 | Phase 5 |
| `src/browser/BrowserWindow.cpp` | BookmarkPanel/BookmarkService 생성, 시그널 연결 | Phase 5 |
| `src/ui/NavigationBar.h` | 북마크 버튼, 북마크 추가 버튼 추가 | Phase 5 |
| `src/ui/NavigationBar.cpp` | 버튼 클릭 이벤트 핸들러, 시그널 emit | Phase 5 |
| `CMakeLists.txt` | 새 소스 파일 추가 (BookmarkService.cpp, BookmarkPanel.cpp, BookmarkDialog.cpp) | Phase 5 |
| `docs/dev-log.md` | F-07 구현 과정 기록 | Phase 7 |
| `CHANGELOG.md` | F-07 완료 항목 추가 | Phase 7 |
| `README.md` | F-07 기능 완료 명시 | Phase 7 |

---

## 5. 의존성 체크

### 5.1 기능 의존성
- **F-01 (프로젝트 초기 설정)**: ✅ 완료 확인 (Qt/C++ 프로젝트 구조 존재)
- **F-02 (웹뷰 통합)**: ✅ 완료 확인 (WebView 클래스 존재, 북마크 실행 시 WebView::load() 호출 필요)
- **F-03 (URL 입력 UI)**: ✅ 완료 확인 (URLBar 존재, URLValidator 재사용 가능)

### 5.2 라이브러리 의존성
- **Qt 5.15+**: Core, Gui, Widgets 모듈
  - QObject, QString, QVector, QMap, QJsonObject, QJsonArray (Qt Core)
  - QIcon, QKeyEvent (Qt Gui)
  - QWidget, QTreeWidget, QDialog, QPushButton, QLineEdit, QComboBox (Qt Widgets)
- **C++17 표준 라이브러리**: std::optional
- **webOS LS2 API**: luna-service2 (com.webos.service.db)
  - put, find, del 메서드 사용
  - appinfo.json 권한 필요

### 5.3 서비스 간 의존성
```
StorageService (LS2 API 래퍼)
    │
    ▼ (의존)
BookmarkService (비즈니스 로직)
    │
    ▼ (의존)
BookmarkPanel, BookmarkDialog (UI)
    │
    ▼ (통합)
BrowserWindow, NavigationBar
```

---

## 6. 병렬 실행 판단

### 6.1 판단 결과
**순차 실행 권장 (Agent Team 사용 안함)**

### 6.2 근거
- **Phase 1 → Phase 2 의존성**: BookmarkService는 StorageService에 의존 (순차 필수)
- **Phase 2 → Phase 3, 4 의존성**: BookmarkPanel, BookmarkDialog는 BookmarkService에 의존 (순차 필수)
- **Phase 3, 4 병렬 가능성**: BookmarkPanel과 BookmarkDialog는 독립적으로 구현 가능하나, 단일 개발자 작업 시 병렬 효과 미미
- **Phase 5 통합**: 모든 Phase 완료 후 통합 필요 (순차 필수)
- **코드 충돌 위험**: BrowserWindow, NavigationBar 수정 시 Phase 3, 4, 5 간 충돌 가능

### 6.3 병렬 가능한 부분 (선택적)
만약 Agent Team을 사용한다면:
- **Task Group 1** (cpp-dev-1): Phase 1 → Phase 2 → Phase 3 (BookmarkPanel)
- **Task Group 2** (cpp-dev-2): Phase 4 (BookmarkDialog, Phase 2 완료 후 시작)
- **합류**: Phase 5 통합 작업

**권장하지 않는 이유**:
- Phase 4는 독립적이지만 Phase 2 완료 대기 필요 (병렬 효과 제한)
- 통합 시점(Phase 5)에서 충돌 가능성 높음 (BrowserWindow, NavigationBar 동시 수정)
- 단일 cpp-dev 에이전트로 순차 진행이 더 안전하고 명확함

---

## 7. 태스크 의존성 다이어그램

```
Phase 1: 데이터 모델 + StorageService
  ├─ Task 1.1: Bookmark.h
  ├─ Task 1.2: StorageService.h/.cpp
  └─ Task 1.3: appinfo.json 권한
          │
          ▼
Phase 2: BookmarkService
  ├─ Task 2.1: 클래스 기본 구조
  ├─ Task 2.2: 북마크 CRUD
  ├─ Task 2.3: 폴더 관리
  ├─ Task 2.4: 메모리 캐싱
  └─ Task 2.5: 시그널/슬롯
          │
          ├─────────────────────────────┐
          ▼                             ▼
Phase 3: BookmarkPanel          Phase 4: BookmarkDialog
  ├─ Task 3.1: 클래스 구조        ├─ Task 4.1: 클래스 구조
  ├─ Task 3.2: UI 레이아웃        ├─ Task 4.2: UI 레이아웃
  ├─ Task 3.3: 목록 로딩          ├─ Task 4.3: 폴더 드롭다운
  ├─ Task 3.4: 키 이벤트          ├─ Task 4.4: URL 검증
  ├─ Task 3.5: 컨텍스트 메뉴      ├─ Task 4.5: 확인/취소 액션
  ├─ Task 3.6: 검색 필터링        ├─ Task 4.6: 키 이벤트
  ├─ Task 3.7: 슬라이드 애니메    └─ Task 4.7: Stylesheet
  ├─ Task 3.8: Stylesheet
  └─ Task 3.9: 시그널 연결
          │                             │
          └─────────────────────────────┘
                      │
                      ▼
Phase 5: BrowserWindow/NavigationBar 통합
  ├─ Task 5.1: BrowserWindow 수정
  ├─ Task 5.2: NavigationBar 북마크 버튼
  ├─ Task 5.3: NavigationBar 추가 버튼
  ├─ Task 5.4: 리모컨 컬러 버튼
  └─ Task 5.5: CMakeLists.txt 업데이트
          │
          ▼
Phase 6: 테스트
  ├─ Task 6.1: BookmarkService 단위 테스트
  ├─ Task 6.2: BookmarkPanel UI 테스트
  ├─ Task 6.3: BookmarkDialog UI 테스트
  └─ Task 6.4: 통합 테스트
          │
          ▼
Phase 7: 코드 리뷰 및 문서화
  ├─ Task 7.1: code-reviewer 리뷰
  ├─ Task 7.2: 피드백 반영
  ├─ Task 7.3: Doxygen 주석
  ├─ Task 7.4: dev-log, CHANGELOG
  └─ Task 7.5: README 업데이트
```

---

## 8. 위험 요소 및 대응 방안

### 8.1 LS2 API 연동 실패

**위험 요소**:
- luna-service2 라이브러리 미설치 또는 권한 부족
- LS2 API 응답 타임아웃
- JSON 응답 파싱 실패

**대응 방안**:
1. **Phase 1에서 조기 검증**: StorageService::put() 단위 테스트로 LS2 API 연동 확인
2. **에러 시그널 처리**: StorageService::error(QString) 시그널 → BookmarkService::onStorageError 슬롯
3. **타임아웃 설정**: LS2 API 호출 후 5초 이내 응답 없으면 에러 시그널 emit
4. **폴백 처리**: LS2 API 실패 시 메모리 캐시 유지 (세션 중 데이터 유지)
5. **사용자 알림**: QMessageBox로 에러 메시지 표시 ("북마크 저장 실패: 네트워크 오류"), 재시도 버튼 제공

### 8.2 UI 복잡도 관리

**위험 요소**:
- BookmarkPanel UI 복잡도 높음 (QTreeWidget, 컨텍스트 메뉴, 검색, 애니메이션)
- 리모컨 키 이벤트 처리 복잡도

**대응 방안**:
1. **UI 로직 분리**: setupUI(), setupConnections(), applyStyles() 메서드로 관심사 분리
2. **키 이벤트 단순화**: keyPressEvent()에서 switch-case로 명확히 분기
3. **프라이빗 헬퍼 메서드**: createBookmarkItem(), createFolderItem(), filterBookmarks() 등으로 코드 모듈화
4. **Qt Test 활용**: QTestEventList로 키 이벤트 시뮬레이션, 자동화 테스트
5. **단계적 구현**: 기본 목록 표시 → 키 이벤트 → 컨텍스트 메뉴 → 검색 → 애니메이션 순서로 순차 구현

### 8.3 테스트 환경 제약

**위험 요소**:
- webOS 프로젝터 실제 디바이스 접근 제한
- LS2 API 모킹 복잡도

**대응 방안**:
1. **MockStorageService 구현**: LS2 API를 모킹하여 단위 테스트 환경 구성
2. **Qt Test 활용**: Qt Creator에서 로컬 테스트 실행 가능
3. **Google Test 통합**: CMakeLists.txt에 GTest 링크, CTest로 자동화
4. **수동 테스트 계획**: 실제 디바이스 배포 전 단위 테스트 100% 통과 필수
5. **CI/CD 파이프라인**: GitHub Actions로 빌드 + 테스트 자동화 (향후)

### 8.4 메모리 누수

**위험 요소**:
- Qt 객체 메모리 관리 실수 (QTreeWidgetItem, QDialog)
- 시그널/슬롯 연결 해제 누락

**대응 방안**:
1. **Qt 부모-자식 메모리 관리**: 모든 QObject 생성 시 parent 전달
2. **QScopedPointer 사용**: StorageService Private 구현 (PIMPL 패턴)
3. **QTreeWidgetItem 자동 소유권**: QTreeWidget::addTopLevelItem() 호출 시 자동 소유권 이전 (수동 delete 불필요)
4. **Valgrind 검증**: 테스트 실행 시 메모리 누수 체크
5. **코드 리뷰**: code-reviewer 에이전트가 메모리 관리 적절성 확인

### 8.5 중복 URL 동시 추가 (Race Condition)

**위험 요소**:
- 사용자가 동시에 같은 URL 북마크 추가 시도 (드문 케이스)
- LS2 API 응답 지연으로 인한 중복 체크 실패

**대응 방안**:
1. **BookmarkService::exists() 선행 체크**: addBookmark() 호출 전 중복 URL 검증
2. **LS2 API 응답 후 캐시 업데이트**: 낙관적 업데이트 금지 (putSuccess 시그널 수신 후 메모리 캐시 업데이트)
3. **DB8 인덱스 활용**: url_index로 중복 쿼리 성능 최적화
4. **사용자 피드백**: 중복 URL 발견 시 QMessageBox 경고 ("이미 북마크에 추가된 페이지입니다")
5. **버튼 비활성화**: 북마크 추가 진행 중 addBookmarkButton_ 비활성화 (중복 클릭 방지)

---

## 9. 테스트 전략

### 9.1 단위 테스트 (BookmarkService)

**도구**: Google Test
**파일**: `tests/unit/BookmarkServiceTest.cpp`

**테스트 케이스**:
1. **addBookmark() 테스트**:
   - 정상 추가: 반환값 true, bookmarkAdded 시그널 emit 확인
   - 중복 URL 추가: 반환값 false, 시그널 emit 안됨
2. **updateBookmark() 테스트**:
   - 기존 북마크 업데이트: 반환값 true, bookmarkUpdated 시그널 emit
   - 존재하지 않는 ID 업데이트: 반환값 false
3. **deleteBookmark() 테스트**:
   - 기존 북마크 삭제: 반환값 true, bookmarkDeleted 시그널 emit
   - 존재하지 않는 ID 삭제: 반환값 false
4. **getAllBookmarks() 테스트**:
   - 북마크 3개 추가 후 조회: 반환 배열 크기 3
   - 빈 상태 조회: 반환 배열 크기 0
5. **exists() 테스트**:
   - 중복 URL (대소문자 무시): 반환값 true
   - 새 URL: 반환값 false
6. **createFolder() 테스트**:
   - 루트 폴더 생성: 반환값 true, folderAdded 시그널 emit
   - 서브폴더 생성 (parentId 지정): 반환값 true
7. **deleteFolder() 테스트**:
   - 빈 폴더 삭제: 반환값 true, folderDeleted 시그널 emit
   - 하위 북마크 있는 폴더 삭제 (moveToRoot=true): 북마크의 folderId가 "" 로 변경
   - 하위 북마크 있는 폴더 삭제 (moveToRoot=false): 폴더 + 하위 북마크 모두 삭제

**모킹**:
- MockStorageService: put(), find(), del() 메서드 모킹
- QSignalSpy로 시그널 emit 확인

### 9.2 통합 테스트 (BookmarkPanel)

**도구**: Qt Test
**파일**: `tests/integration/BookmarkPanelTest.cpp`

**테스트 케이스**:
1. **북마크 목록 표시 테스트**:
   - BookmarkService에 북마크 3개 추가 → BookmarkPanel::show() 호출
   - QTreeWidget::topLevelItemCount() == 3 확인
2. **키 이벤트 테스트**:
   - Qt::Key_Down → QTreeWidget::currentItem() 변경 확인
   - Qt::Key_Right → QTreeWidget::isItemExpanded() == true (폴더 펼치기)
   - Qt::Key_Return → bookmarkSelected 시그널 emit 확인 (QSignalSpy)
3. **컨텍스트 메뉴 테스트**:
   - Qt::Key_Menu → QMenu::exec() 호출 확인 (QTest::mouseClick으로 메뉴 항목 클릭)
4. **검색 필터링 테스트**:
   - searchEdit_->setText("youtube") → QTreeWidget에 "YouTube" 항목만 표시
   - QTreeWidget::topLevelItemCount() == 1 확인

**모킹**:
- MockBookmarkService 사용 (또는 실제 BookmarkService + MockStorageService)

### 9.3 통합 테스트 (BookmarkDialog)

**도구**: Qt Test
**파일**: `tests/integration/BookmarkDialogTest.cpp`

**테스트 케이스**:
1. **Add 모드 테스트**:
   - titleEdit_->setText("Test"), urlEdit_->setText("https://test.com")
   - acceptButton_->click() → BookmarkService::addBookmark() 호출 확인 (QSignalSpy)
2. **Edit 모드 테스트**:
   - 기존 북마크 ID 전달 → titleEdit_, urlEdit_ 자동 입력 확인
   - titleEdit_->setText("Updated") → acceptButton_->click() → updateBookmark() 호출 확인
3. **URL 유효성 검증 테스트**:
   - urlEdit_->setText("invalid-url") → acceptButton_->isEnabled() == false
   - urlEdit_->setText("https://valid.com") → acceptButton_->isEnabled() == true
4. **중복 URL 테스트**:
   - BookmarkService에 "https://test.com" 추가 → 다시 추가 시도
   - errorLabel_->text() == "이미 북마크에 추가된 페이지입니다" 확인

### 9.4 전체 플로우 테스트

**도구**: Qt Test
**파일**: `tests/integration/BookmarkFlowTest.cpp`

**테스트 케이스**:
1. **북마크 추가 → 목록 조회 → 실행 → 페이지 로드**:
   - NavigationBar::addBookmarkButton_->click() → BookmarkDialog 표시
   - 제목/URL 입력 → acceptButton_->click() → BookmarkService::addBookmark() 호출
   - BookmarkPanel::show() → QTreeWidget에 북마크 표시 확인
   - 북마크 선택 (Enter 키) → bookmarkSelected 시그널 emit → WebView::load() 호출 확인
2. **폴더 생성 → 북마크 이동 → 폴더 삭제**:
   - BookmarkPanel::newFolderButton_->click() → QInputDialog 표시 → "Test Folder" 입력
   - BookmarkService::createFolder() 호출 → QTreeWidget에 폴더 추가 확인
   - 북마크 편집 → 폴더 선택 "Test Folder" → updateBookmark() 호출
   - 폴더 삭제 (moveToRoot=true) → 북마크의 folderId == "" 확인

**모킹**:
- MockWebView: load() 호출 확인
- MockStorageService: LS2 API 모킹

---

## 10. 완료 기준

### 10.1 Phase별 완료 기준

#### Phase 1 완료 기준
- ✅ Bookmark/Folder 구조체가 QJsonObject와 상호 변환 가능
- ✅ StorageService::put() 호출 시 putSuccess 시그널 emit (LS2 API 연동 확인)
- ✅ StorageService::find() 호출 시 findSuccess 시그널 emit (데이터 조회 성공)
- ✅ appinfo.json에 db8 권한 추가 완료

#### Phase 2 완료 기준
- ✅ BookmarkService::addBookmark() 호출 시 LS2 API 저장 + 메모리 캐시 업데이트
- ✅ BookmarkService::getAllBookmarks() 호출 시 메모리 캐시에서 1ms 이내 반환
- ✅ BookmarkService::exists() 호출 시 중복 URL 정확 검출 (대소문자 무시)
- ✅ BookmarkService::deleteFolder() 호출 시 cascade 삭제 또는 루트 이동 정상 동작
- ✅ BookmarkService::initialized 시그널 emit 확인

#### Phase 3 완료 기준
- ✅ BookmarkPanel::show() 호출 시 슬라이드 인 애니메이션 동작
- ✅ 리모컨 방향키로 북마크 목록 스크롤 가능
- ✅ Enter 키로 북마크 선택 시 bookmarkSelected(url) 시그널 emit
- ✅ 컨텍스트 메뉴 (Options 키)로 편집/삭제 액션 실행 가능
- ✅ 검색 입력 시 제목/URL 필터링 실시간 동작
- ✅ BookmarkService::bookmarkAdded 시그널 수신 시 QTreeWidget 증분 업데이트

#### Phase 4 완료 기준
- ✅ Add 모드: 제목/URL 입력 → 확인 버튼 클릭 시 addBookmark() 호출
- ✅ Edit 모드: 기존 값 자동 입력 → 수정 후 확인 버튼 클릭 시 updateBookmark() 호출
- ✅ URL 유효성 검증: 잘못된 URL 입력 시 acceptButton_ 비활성화
- ✅ 중복 URL 입력 시 errorLabel_ 표시
- ✅ 리모컨 방향키로 입력 필드 간 포커스 이동 가능

#### Phase 5 완료 기준
- ✅ NavigationBar 북마크 버튼 클릭 시 BookmarkPanel 표시
- ✅ NavigationBar 북마크 추가 버튼 클릭 시 BookmarkDialog 표시 (제목/URL 자동 입력)
- ✅ BookmarkPanel에서 북마크 선택 시 WebView에서 페이지 로드
- ✅ 리모컨 빨강 버튼으로 북마크 추가 다이얼로그 즉시 표시
- ✅ 리모컨 파랑 버튼으로 북마크 패널 즉시 표시

#### Phase 6 완료 기준
- ✅ BookmarkService 단위 테스트 통과율 100%
- ✅ BookmarkPanel, BookmarkDialog UI 테스트 통과율 100%
- ✅ 전체 플로우 통합 테스트 통과
- ✅ 테스트 커버리지 80% 이상

#### Phase 7 완료 기준
- ✅ code-reviewer 승인 완료
- ✅ 모든 public 메서드에 Doxygen 주석 추가
- ✅ dev-log.md, CHANGELOG.md, README.md 업데이트 완료

### 10.2 전체 기능 완료 기준 (DoD)

아래 모든 항목 체크 시 F-07 완료:

#### 기능 완성도
- [ ] 북마크 추가/편집/삭제 기능 정상 동작 (LS2 API 저장/조회/삭제)
- [ ] 폴더 생성/이름 변경/삭제 기능 정상 동작 (1단계 서브폴더 지원)
- [ ] 북마크 목록 조회 및 폴더 구조 트리 뷰 표시
- [ ] 북마크 선택 시 WebView에서 페이지 로드
- [ ] 리모컨 방향키 및 단축키로 모든 기능 사용 가능
- [ ] 앱 재시작 후 저장된 북마크 데이터 정상 로드

#### 테스트 통과
- [ ] 단위 테스트 (BookmarkService) 100% 통과
- [ ] 통합 테스트 (BookmarkPanel, BookmarkDialog) 100% 통과
- [ ] 전체 플로우 테스트 통과
- [ ] 수동 테스트 (실제 webOS 프로젝터): 북마크 추가 → 목록 조회 → 실행 → 페이지 로드 플로우
- [ ] 리모컨 방향키/컬러 버튼 매핑 테스트

#### 코드 품질
- [ ] C++ 코딩 컨벤션 준수 (CLAUDE.md 기준)
- [ ] 스마트 포인터 사용 (Qt 부모-자식 메모리 관리)
- [ ] 에러 처리 (LS2 API 에러, 중복 URL, 잘못된 입력) 완전 구현
- [ ] 로깅 (Logger::info(), Logger::error()) 적절히 사용
- [ ] 메모리 누수 없음 (Valgrind 검증)

#### 성능 기준
- [ ] 북마크 로딩 시간 1초 이내 (100개 북마크 기준)
- [ ] 북마크 추가/편집/삭제 반응 속도 0.5초 이내
- [ ] 목록 스크롤 60fps 유지 (100개 이상 북마크)
- [ ] 메모리 사용량 50MB 이하 (BookmarkPanel + BookmarkService)

#### 문서화
- [ ] requirements.md, design.md, plan.md 완성
- [ ] API 문서 (Doxygen 주석) 완성
- [ ] dev-log.md, CHANGELOG.md 업데이트
- [ ] README 업데이트 (F-07 기능 완료 명시)

#### 코드 리뷰
- [ ] code-reviewer 승인 완료
- [ ] 리뷰 피드백 반영 완료

---

## 11. 리스크

| 리스크 | 영향도 | 발생 확률 | 대응 방안 |
|--------|--------|----------|-----------|
| LS2 API 연동 실패 | 높음 | 중간 | Phase 1에서 조기 검증, MockStorageService 구현, 에러 시그널 처리 |
| UI 복잡도 관리 | 중간 | 중간 | UI 로직 분리, 프라이빗 헬퍼 메서드, 단계적 구현 |
| 테스트 환경 제약 (webOS 디바이스 접근 제한) | 중간 | 높음 | MockStorageService 구현, Qt Test 로컬 실행, 수동 테스트 계획 |
| 메모리 누수 | 높음 | 낮음 | Qt 부모-자식 메모리 관리, Valgrind 검증, 코드 리뷰 |
| 중복 URL 동시 추가 (Race Condition) | 낮음 | 낮음 | exists() 선행 체크, LS2 API 응답 후 캐시 업데이트, 버튼 비활성화 |
| 폴더 삭제 시 하위 북마크 고아 데이터 | 중간 | 낮음 | deleteFolder(moveToRoot) 메서드, cascade 삭제 또는 루트 이동, 사용자 확인 다이얼로그 |
| 리모컨 키 매핑 충돌 | 낮음 | 낮음 | keyPressEvent() 명확한 분기, webOS 리모컨 키코드 문서 참조 |
| 예상 일정 초과 (Phase 3 UI 복잡도) | 중간 | 중간 | Phase 3 태스크 우선순위 조정 (검색/애니메이션 후순위), 병목 조기 식별 |

---

## 12. 예상 작업 시간

| Phase | 예상 기간 | 의존성 |
|-------|----------|--------|
| Phase 1: 데이터 모델 + StorageService | 2일 | F-01 |
| Phase 2: BookmarkService | 3일 | Phase 1 |
| Phase 3: BookmarkPanel | 4일 | Phase 2 |
| Phase 4: BookmarkDialog | 2일 | Phase 2 (Phase 3과 병렬 가능) |
| Phase 5: BrowserWindow/NavigationBar 통합 | 1일 | Phase 3, 4 |
| Phase 6: 테스트 | 2일 | Phase 5 |
| Phase 7: 코드 리뷰 및 문서화 | 1일 | Phase 6 |
| **총 예상 기간** | **15일** (순차 진행 시) | — |

**병렬 실행 시 예상 기간**: 13일 (Phase 3과 4 병렬 실행, 2일 단축)

---

## 13. 다음 단계

### 13.1 Phase 1 시작 조건
- ✅ requirements.md 승인 완료
- ✅ design.md 승인 완료
- ✅ plan.md 승인 완료 (현재 문서)
- ✅ F-01 (프로젝트 초기 설정) 완료 확인

### 13.2 Phase 1 착수 명령
```bash
# cpp-dev 에이전트 호출 예시
/agent cpp-dev "Phase 1: 데이터 모델 및 StorageService 기반 구축 시작.
Task 1.1: src/models/Bookmark.h 생성,
Task 1.2: src/services/StorageService.h/.cpp 생성,
Task 1.3: webos-meta/appinfo.json 권한 추가.
참조: docs/specs/bookmark-management/plan.md Phase 1"
```

### 13.3 이후 진행 순서
1. Phase 1 완료 후 product-manager에게 보고
2. product-manager 검증 후 Phase 2 착수 승인
3. Phase 2 ~ Phase 7 순차 진행
4. 각 Phase 완료 시 product-manager에게 보고
5. Phase 7 완료 시 F-07 기능 완료 선언

---

## 14. 참고 자료

### 14.1 프로젝트 문서
- **PRD**: `docs/project/prd.md`
- **기능 백로그**: `docs/project/features.md`
- **로드맵**: `docs/project/roadmap.md`
- **프로젝트 가이드**: `CLAUDE.md`

### 14.2 관련 기능 설계서
- **F-02 (웹뷰 통합)**: `docs/specs/webview-integration/` (북마크 실행 시 WebView::load() 연동)
- **F-03 (URL 입력 UI)**: `docs/specs/url-input-ui/` (URLValidator 재사용)
- **F-06 (탭 관리)**: `docs/specs/tab-management/` ("새 탭에서 열기" 컨텍스트 메뉴 연동)

### 14.3 기술 문서
- **Qt QTreeWidget**: https://doc.qt.io/qt-5/qtreewidget.html
- **Qt Signal/Slot**: https://doc.qt.io/qt-5/signalsandslots.html
- **Qt Stylesheet**: https://doc.qt.io/qt-5/stylesheet.html
- **webOS LS2 API (DB8)**: https://webostv.developer.lge.com/develop/native-apps/native-service-overview
- **Qt JSON**: https://doc.qt.io/qt-5/json.html

---

## 15. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초기 작성 (requirements.md, design.md 기반) | product-manager |
| 2026-02-14 | Web App → Native App 전환으로 전체 재작성 (C++/Qt, LS2 API) | product-manager |
