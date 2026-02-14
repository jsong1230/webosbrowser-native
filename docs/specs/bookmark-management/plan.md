# 북마크 관리 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/bookmark-management/requirements.md
- 기술 설계서: docs/specs/bookmark-management/design.md
- PRD: docs/project/prd.md
- 기능 백로그: docs/project/features.md

## 2. 구현 개요

### 목표
사용자가 자주 방문하는 웹사이트를 북마크로 저장하고 관리할 수 있는 기능을 구현합니다. IndexedDB로 영속 데이터를 저장하며, 리모컨 최적화 UI로 북마크 CRUD 및 폴더 관리를 제공합니다.

### 핵심 기능
- 북마크 추가/편집/삭제
- 폴더 구조 관리 (1단계 서브폴더 지원)
- 북마크 목록 조회 및 탐색 (리모컨 최적화)
- 북마크 실행 (페이지 열기)
- 북마크 검색 및 정렬 (선택적)
- IndexedDB 영속 저장

### 주요 컴포넌트
- BookmarkService (IndexedDB CRUD)
- BookmarkPanel (북마크 목록 UI)
- BookmarkDialog (추가/편집 다이얼로그)
- FolderDialog (폴더 관리 다이얼로그)
- BookmarkItem, FolderItem (목록 항목)

## 3. 구현 Phase

### Phase 1: 데이터 저장소 구현 (Backend 계층)
**담당**: frontend-dev
**우선순위**: Must (최우선)

- [ ] Task 1.1: IndexedDB 초기화 및 스키마 생성
  - 파일: `src/services/indexedDBService.js`
  - 내용:
    - `initDB()` 함수 (DB_NAME, DB_VERSION)
    - `onupgradeneeded` 이벤트에서 bookmarks, folders 오브젝트 스토어 생성
    - 인덱스 생성 (folderId, url, title, createdAt, parentId, name)
    - `promisifyRequest()` 유틸리티 함수
  - 완료 기준: IndexedDB 초기화 성공, 콘솔에서 스토어 확인 가능

- [ ] Task 1.2: BookmarkService 구현 (북마크 CRUD)
  - 파일: `src/services/bookmarkService.js`
  - 내용:
    - `getAllBookmarks()`: 모든 북마크 조회
    - `getBookmarksByFolder(folderId)`: 폴더별 북마크 조회
    - `getBookmarkById(id)`: 단일 북마크 조회
    - `getBookmarkByUrl(url)`: URL로 북마크 조회 (중복 체크)
    - `addBookmark({ title, url, folderId, description })`: 북마크 추가
    - `updateBookmark(id, updates)`: 북마크 수정
    - `deleteBookmark(id)`: 북마크 삭제
    - `incrementVisitCount(id)`: 방문 횟수 증가
  - 완료 기준: 각 함수 정상 동작, Jest 단위 테스트 통과

- [ ] Task 1.3: FolderService 구현 (폴더 관리)
  - 파일: `src/services/bookmarkService.js` (같은 파일)
  - 내용:
    - `getAllFolders()`: 모든 폴더 조회
    - `getFoldersByParent(parentId)`: 서브폴더 조회
    - `addFolder({ name, parentId })`: 폴더 추가
    - `updateFolder(id, { name })`: 폴더 이름 변경
    - `deleteFolder(id)`: 폴더 삭제 (하위 북마크 포함, 트랜잭션)
  - 완료 기준: 각 함수 정상 동작, 폴더 삭제 시 하위 북마크도 삭제 확인

- [ ] Task 1.4: UUID 생성 유틸리티
  - 파일: `src/utils/uuid.js`
  - 내용:
    - `generateUUID()`: crypto.randomUUID() 또는 폴백 구현
  - 완료 기준: 고유한 ID 생성 확인

**예상 산출물**:
- `src/services/indexedDBService.js`
- `src/services/bookmarkService.js`
- `src/utils/uuid.js`

**완료 기준**:
- IndexedDB 초기화 성공 (브라우저 개발자 도구에서 확인)
- 북마크 및 폴더 CRUD 함수 정상 동작
- 앱 재시작 후 데이터 영속성 확인

**예상 소요 시간**: 4~6시간

---

### Phase 2: 북마크 UI 컴포넌트 구현 (기본 구조)
**담당**: frontend-dev
**우선순위**: Must
**의존성**: Phase 1 완료 필요

- [ ] Task 2.1: BookmarkPanel 메인 컴포넌트
  - 파일: `src/components/BookmarkPanel/BookmarkPanel.js`
  - 내용:
    - BookmarkPanel 컴포넌트 구조 (Enact Panel)
    - Props 인터페이스 (visible, currentUrl, currentTitle, onClose, onBookmarkSelect)
    - 상태 관리 (bookmarks, folders, currentFolderId, showAddDialog, etc.)
    - useEffect로 IndexedDB 데이터 로드 (loadBookmarks, loadFolders)
    - 북마크 추가/편집/삭제 핸들러 (handleAddBookmark, handleEditBookmark, handleDeleteBookmark)
    - 폴더 추가/편집/삭제 핸들러
  - 완료 기준: BookmarkPanel이 visible prop에 따라 오버레이 표시, 백 버튼으로 닫기

- [ ] Task 2.2: BookmarkList 컴포넌트 (VirtualList)
  - 파일: `src/components/BookmarkPanel/BookmarkList.js`
  - 내용:
    - Enact VirtualList 사용
    - 폴더 + 북마크 결합 (폴더 우선 표시)
    - itemRenderer로 BookmarkItem, FolderItem 렌더링
    - 빈 목록 시 "저장된 북마크가 없습니다" 메시지
  - 완료 기준: 북마크 및 폴더 목록 표시, 리모컨 방향키로 스크롤 가능

- [ ] Task 2.3: BookmarkItem 컴포넌트
  - 파일: `src/components/BookmarkPanel/BookmarkItem.js`
  - 내용:
    - 북마크 정보 표시 (파비콘, 제목, URL)
    - 선택 버튼 클릭 시 onBookmarkClick 호출
    - 컨텍스트 메뉴 (옵션 버튼) 표시 (편집, 삭제)
    - Spotlightable 설정
  - 완료 기준: 북마크 항목 클릭 시 페이지 열기, 컨텍스트 메뉴 정상 동작

- [ ] Task 2.4: FolderItem 컴포넌트
  - 파일: `src/components/BookmarkPanel/FolderItem.js`
  - 내용:
    - 폴더 정보 표시 (폴더 아이콘, 폴더 이름)
    - 선택 버튼 클릭 시 onFolderClick 호출 (폴더 열기)
    - 방향키 우: 폴더 열기 (서브폴더 표시)
    - 컨텍스트 메뉴 (편집, 삭제)
  - 완료 기준: 폴더 클릭 시 하위 북마크 표시, 방향키 좌로 상위 폴더 이동

- [ ] Task 2.5: BookmarkPanel 스타일
  - 파일: `src/components/BookmarkPanel/BookmarkPanel.module.less`
  - 내용:
    - 패널 레이아웃 (오버레이, 우측 고정, 600px 너비)
    - 헤더, 액션바, 북마크 리스트 스타일
    - 북마크 항목 스타일 (포커스 상태, 호버 효과)
    - 폴더 항목 스타일
    - 대화면 가독성 (폰트 크기 20px+, 대비 4.5:1)
  - 완료 기준: 프로젝터에서 3m 거리에서 가독 가능, 포커스 표시 명확

- [ ] Task 2.6: BookmarkPanel Export
  - 파일: `src/components/BookmarkPanel/index.js`
  - 내용: BookmarkPanel 기본 export
  - 완료 기준: BrowserView에서 import 가능

**예상 산출물**:
- `src/components/BookmarkPanel/BookmarkPanel.js`
- `src/components/BookmarkPanel/BookmarkList.js`
- `src/components/BookmarkPanel/BookmarkItem.js`
- `src/components/BookmarkPanel/FolderItem.js`
- `src/components/BookmarkPanel/BookmarkPanel.module.less`
- `src/components/BookmarkPanel/index.js`

**완료 기준**:
- BookmarkPanel이 오버레이로 정상 표시
- 북마크 및 폴더 목록 조회 및 탐색 가능
- 리모컨 방향키로 목록 스크롤
- 백 버튼으로 패널 닫기

**예상 소요 시간**: 6~8시간

---

### Phase 3: 북마크 추가/편집 다이얼로그 구현
**담당**: frontend-dev
**우선순위**: Must
**의존성**: Phase 2 완료 필요

- [ ] Task 3.1: BookmarkDialog 컴포넌트
  - 파일: `src/components/BookmarkPanel/BookmarkDialog.js`
  - 내용:
    - Enact Dialog 사용
    - 추가 모드 vs 편집 모드 (bookmark prop이 있으면 편집)
    - 입력 필드: 제목 (Input), URL (Input, 편집 시 비활성화), 폴더 선택 (Dropdown)
    - 저장 버튼 클릭 시 onSave 호출
    - 취소 버튼 클릭 시 onCancel 호출
    - 가상 키보드 통합 (Input 포커스 시 표시)
  - 완료 기준: 북마크 추가/편집 다이얼로그 정상 동작, 저장 후 목록 갱신

- [ ] Task 3.2: FolderDialog 컴포넌트
  - 파일: `src/components/BookmarkPanel/FolderDialog.js`
  - 내용:
    - Enact Dialog 사용
    - 폴더 이름 입력 (Input)
    - 저장 버튼 클릭 시 onSave 호출
    - 취소 버튼 클릭 시 onCancel 호출
    - 가상 키보드 통합
  - 완료 기준: 폴더 추가/편집 다이얼로그 정상 동작, 저장 후 목록 갱신

- [ ] Task 3.3: ConfirmDialog 컴포넌트
  - 파일: `src/components/BookmarkPanel/ConfirmDialog.js`
  - 내용:
    - Enact Dialog 사용
    - 확인 메시지 표시 (예: "북마크를 삭제하시겠습니까?")
    - 확인 버튼 클릭 시 onConfirm 호출
    - 취소 버튼 클릭 시 onCancel 호출
  - 완료 기준: 북마크/폴더 삭제 시 확인 다이얼로그 표시, 확인 후 삭제

- [ ] Task 3.4: 다이얼로그 핸들러 통합
  - 파일: `src/components/BookmarkPanel/BookmarkPanel.js` (수정)
  - 내용:
    - handleAddBookmark: BookmarkService.addBookmark() 호출, 중복 체크
    - handleEditBookmark: BookmarkService.updateBookmark() 호출
    - handleDeleteBookmark: BookmarkService.deleteBookmark() 호출
    - handleAddFolder: BookmarkService.addFolder() 호출
    - handleEditFolder: BookmarkService.updateFolder() 호출
    - handleDeleteFolder: BookmarkService.deleteFolder() 호출 (하위 북마크 경고)
    - 성공 시 토스트 메시지 표시 (Enact Notification 또는 Alert)
  - 완료 기준: 북마크/폴더 CRUD 동작 정상, 에러 처리 완료

**예상 산출물**:
- `src/components/BookmarkPanel/BookmarkDialog.js`
- `src/components/BookmarkPanel/FolderDialog.js`
- `src/components/BookmarkPanel/ConfirmDialog.js`
- `src/components/BookmarkPanel/BookmarkPanel.js` (수정)

**완료 기준**:
- 북마크 추가/편집/삭제 전체 흐름 정상 동작
- 폴더 추가/편집/삭제 전체 흐름 정상 동작
- 중복 북마크 추가 시 에러 메시지 표시
- 폴더 삭제 시 하위 북마크 경고 표시

**예상 소요 시간**: 4~6시간

---

### Phase 4: BrowserView 통합 및 페이지 열기
**담당**: frontend-dev
**우선순위**: Must
**의존성**: Phase 3 완료 필요

- [ ] Task 4.1: NavigationBar에 북마크 버튼 추가
  - 파일: `src/components/NavigationBar/NavigationBar.js` (수정)
  - 내용:
    - 북마크 버튼 추가 (icon="star")
    - onBookmarkClick prop 추가
    - Spotlightable 설정
  - 완료 기준: NavigationBar에 북마크 버튼 표시, 클릭 시 BookmarkPanel 열기

- [ ] Task 4.2: BrowserView에서 BookmarkPanel 통합
  - 파일: `src/views/BrowserView.js` (수정)
  - 내용:
    - BookmarkPanel import
    - showBookmarkPanel 상태 추가
    - handleBookmarkClick 핸들러 (setShowBookmarkPanel(true))
    - handleBookmarkSelect 핸들러 (setCurrentUrl, setShowBookmarkPanel(false))
    - BookmarkService.incrementVisitCount() 호출 (방문 횟수 증가)
    - JSX에 `<BookmarkPanel>` 오버레이 추가
  - 완료 기준: 북마크 버튼 → BookmarkPanel 열기 → 북마크 선택 → 페이지 로드 전체 흐름 정상

- [ ] Task 4.3: 북마크 실행 시 로딩 인디케이터 표시
  - 파일: `src/views/BrowserView.js` (수정)
  - 내용:
    - handleBookmarkSelect에서 setIsLoading(true)
    - WebView onLoadEnd에서 setIsLoading(false)
    - LoadingBar 컴포넌트 표시 (F-05 연동)
  - 완료 기준: 북마크 실행 시 로딩 인디케이터 표시, 로드 완료 후 숨김

- [ ] Task 4.4: 북마크 추가 시 현재 페이지 정보 전달
  - 파일: `src/views/BrowserView.js` (수정)
  - 내용:
    - BookmarkPanel에 currentUrl, currentTitle prop 전달
    - WebView onLoadEnd에서 currentTitle 업데이트 (document.title)
  - 완료 기준: 북마크 추가 다이얼로그에서 현재 페이지 제목, URL 자동 입력

**예상 산출물**:
- `src/components/NavigationBar/NavigationBar.js` (수정)
- `src/views/BrowserView.js` (수정)

**완료 기준**:
- 북마크 버튼 클릭 → BookmarkPanel 열기
- 북마크 선택 → WebView에서 페이지 로드
- 현재 페이지 북마크 추가 가능
- 로딩 인디케이터 정상 동작

**예상 소요 시간**: 3~4시간

---

### Phase 5: 리모컨 키 매핑 및 Spotlight 최적화
**담당**: frontend-dev
**우선순위**: Must
**의존성**: Phase 4 완료 필요

- [ ] Task 5.1: BookmarkPanel Spotlight 설정
  - 파일: `src/components/BookmarkPanel/BookmarkPanel.js` (수정)
  - 내용:
    - useEffect에서 Spotlight.set() 호출 (spotlight-id="bookmark-panel")
    - defaultElement: BookmarkList
    - enterTo: 'default-element'
    - leaveFor: { left: '', right: '' } (상위 폴더 이동, 폴더 열기 제어)
    - visible 시 Spotlight.focus('bookmark-list')
  - 완료 기준: BookmarkPanel 열릴 때 자동으로 BookmarkList에 포커스

- [ ] Task 5.2: 리모컨 키 이벤트 핸들러
  - 파일: `src/components/BookmarkPanel/BookmarkPanel.js` (수정)
  - 내용:
    - handleKeyDown 이벤트 핸들러
    - 방향키 좌 (37): 상위 폴더로 이동 (setCurrentFolderId(null))
    - 방향키 우 (39): 폴더 열기 (FolderItem에서 처리)
    - 백 버튼 (27 또는 461): BookmarkPanel 닫기 (onClose)
    - 컬러 버튼 (빨강 403, 파랑 404, 노랑 405, 초록 406):
      - 빨강: 북마크 추가
      - 파랑: 북마크 편집
      - 노랑: 북마크 삭제
      - 초록: 새 폴더 생성
  - 완료 기준: 모든 리모컨 키 매핑 정상 동작

- [ ] Task 5.3: 포커스 표시 스타일 최적화
  - 파일: `src/components/BookmarkPanel/BookmarkPanel.module.less` (수정)
  - 내용:
    - :focus 스타일 강화 (outline: 3px solid var(--accent-color))
    - 포커스된 항목 배경색 변화 (rgba(255, 255, 255, 0.1))
    - 호버 효과 제거 (리모컨 전용)
  - 완료 기준: 포커스된 북마크/폴더 항목이 명확하게 구분됨

**예상 산출물**:
- `src/components/BookmarkPanel/BookmarkPanel.js` (수정)
- `src/components/BookmarkPanel/BookmarkPanel.module.less` (수정)

**완료 기준**:
- 리모컨 방향키로 북마크/폴더 탐색 가능
- 선택 버튼으로 북마크 실행, 폴더 열기
- 백 버튼으로 패널 닫기, 상위 폴더 이동
- 컬러 버튼으로 북마크 추가/편집/삭제, 폴더 생성
- 포커스 표시 명확

**예상 소요 시간**: 3~4시간

---

### Phase 6: 선택적 기능 (검색, 정렬)
**담당**: frontend-dev
**우선순위**: Could (선택적, 기본 기능 완료 후)
**의존성**: Phase 5 완료 필요

- [ ] Task 6.1: 북마크 검색 기능
  - 파일: `src/components/BookmarkPanel/BookmarkPanel.js` (수정)
  - 내용:
    - 검색 입력 필드 추가 (Enact Input)
    - searchQuery 상태 추가
    - BookmarkService.searchBookmarks(query) 호출
    - 검색 결과 실시간 업데이트
    - 검색어 비어있으면 전체 목록 표시
  - 완료 기준: 검색어 입력 시 일치하는 북마크 필터링

- [ ] Task 6.2: 북마크 정렬 기능
  - 파일: `src/components/BookmarkPanel/BookmarkPanel.js` (수정)
  - 내용:
    - 정렬 드롭다운 추가 (Enact Dropdown)
    - 정렬 옵션: 이름순, 추가된 날짜순, 방문 빈도순
    - sortBy 상태 추가
    - 북마크 목록 정렬 로직 추가
  - 완료 기준: 정렬 옵션 선택 시 북마크 목록 재정렬

**예상 산출물**:
- `src/components/BookmarkPanel/BookmarkPanel.js` (수정)
- `src/services/bookmarkService.js` (searchBookmarks 함수 추가)

**완료 기준**:
- 검색 기능 정상 동작
- 정렬 기능 정상 동작

**예상 소요 시간**: 2~3시간 (선택적)

---

### Phase 7: 테스트 및 검증
**담당**: test-runner
**우선순위**: Must
**의존성**: Phase 5 완료 필요 (Phase 6 선택적)

- [ ] Task 7.1: BookmarkService 단위 테스트
  - 파일: `src/services/bookmarkService.test.js`
  - 내용:
    - addBookmark 테스트 (정상, 중복 URL)
    - getBookmarksByFolder 테스트
    - updateBookmark 테스트
    - deleteBookmark 테스트
    - addFolder, deleteFolder 테스트 (하위 북마크 삭제 포함)
  - 완료 기준: Jest 테스트 전체 통과

- [ ] Task 7.2: BookmarkPanel 통합 테스트
  - 파일: `src/components/BookmarkPanel/BookmarkPanel.test.js`
  - 내용:
    - BookmarkPanel 렌더링 테스트
    - 북마크 추가 → 목록에 표시 → 실행 전체 흐름 테스트
    - 폴더 생성 → 북마크를 폴더에 추가 → 폴더 내 북마크 조회 테스트
    - 북마크 편집 후 변경 사항 목록 반영 테스트
    - 북마크 삭제 후 목록에서 제거 테스트
  - 완료 기준: Enact Testing Library 테스트 전체 통과

- [ ] Task 7.3: 회귀 테스트 (프로젝터 실제 환경)
  - 내용:
    - 북마크 추가 → 목록에 표시 → 실행 → 페이지 로드 전체 흐름 확인
    - 폴더 생성 → 북마크를 폴더에 추가 → 폴더 내 북마크 조회 확인
    - 북마크 편집 후 변경 사항 목록에 즉시 반영 확인
    - 북마크 삭제 후 목록에서 제거, 재시작 후에도 삭제 유지 확인
    - 앱 재시작 후 모든 북마크 및 폴더 데이터 정상 로드 확인
  - 완료 기준: 프로젝터에서 모든 시나리오 정상 동작

- [ ] Task 7.4: 성능 테스트
  - 내용:
    - 북마크 100개 추가 후 목록 로딩 시간 측정 (1초 이내)
    - 북마크 추가/편집/삭제 반응 속도 측정 (0.5초 이내)
    - 북마크 100개 이상일 때 스크롤 성능 측정 (30fps 이상)
  - 완료 기준: 성능 요구사항 충족

**완료 기준**:
- 단위 테스트 전체 통과
- 통합 테스트 전체 통과
- 프로젝터 실제 환경에서 회귀 테스트 통과
- 성능 요구사항 충족

**예상 소요 시간**: 4~6시간

---

### Phase 8: 코드 리뷰 및 문서화
**담당**: code-reviewer, doc-writer
**우선순위**: Must
**의존성**: Phase 7 완료 필요

- [ ] Task 8.1: 코드 리뷰
  - 담당: code-reviewer
  - 내용:
    - 코딩 컨벤션 준수 확인 (CLAUDE.md)
    - IndexedDB 비동기 처리 에러 핸들링 검증
    - PropTypes 정의 확인
    - CSS Modules 사용 확인
    - 리모컨 접근성 검증 (Spotlightable)
  - 완료 기준: 코드 리뷰 통과, 수정 사항 반영

- [ ] Task 8.2: 컴포넌트 문서 작성
  - 담당: frontend-dev
  - 파일:
    - `docs/components/BookmarkPanel.md`
    - `docs/components/BookmarkDialog.md`
  - 내용:
    - Props 인터페이스
    - 사용 예시
    - 리모컨 키 매핑
    - 상태 관리 방법
    - 주의사항
  - 완료 기준: 컴포넌트 문서 작성 완료

- [ ] Task 8.3: 운영 문서 업데이트
  - 담당: doc-writer
  - 파일:
    - `docs/dev-log.md` (F-07 완료 로그 추가)
    - `CHANGELOG.md` (v0.1.0 변경 사항 추가)
  - 완료 기준: 운영 문서 업데이트 완료

**완료 기준**:
- 코드 리뷰 통과
- 컴포넌트 문서 작성 완료
- 운영 문서 업데이트 완료

**예상 소요 시간**: 2~3시간

---

## 4. 태스크 의존성

```
Phase 1 (데이터 저장소)
  ├─ Task 1.1: IndexedDB 초기화
  ├─ Task 1.2: BookmarkService (북마크 CRUD)
  ├─ Task 1.3: FolderService (폴더 관리)
  └─ Task 1.4: UUID 유틸리티
      ↓
Phase 2 (북마크 UI 컴포넌트)
  ├─ Task 2.1: BookmarkPanel 메인
  ├─ Task 2.2: BookmarkList (VirtualList)
  ├─ Task 2.3: BookmarkItem
  ├─ Task 2.4: FolderItem
  ├─ Task 2.5: 스타일
  └─ Task 2.6: Export
      ↓
Phase 3 (다이얼로그)
  ├─ Task 3.1: BookmarkDialog
  ├─ Task 3.2: FolderDialog
  ├─ Task 3.3: ConfirmDialog
  └─ Task 3.4: 다이얼로그 핸들러 통합
      ↓
Phase 4 (BrowserView 통합)
  ├─ Task 4.1: NavigationBar 북마크 버튼
  ├─ Task 4.2: BrowserView 통합
  ├─ Task 4.3: 로딩 인디케이터
  └─ Task 4.4: 현재 페이지 정보 전달
      ↓
Phase 5 (리모컨 키 매핑)
  ├─ Task 5.1: Spotlight 설정
  ├─ Task 5.2: 리모컨 키 이벤트 핸들러
  └─ Task 5.3: 포커스 표시 스타일
      ↓
Phase 6 (선택적 기능) — 선택적
  ├─ Task 6.1: 검색 기능
  └─ Task 6.2: 정렬 기능
      ↓
Phase 7 (테스트)
  ├─ Task 7.1: 단위 테스트
  ├─ Task 7.2: 통합 테스트
  ├─ Task 7.3: 회귀 테스트
  └─ Task 7.4: 성능 테스트
      ↓
Phase 8 (코드 리뷰 및 문서화)
  ├─ Task 8.1: 코드 리뷰
  ├─ Task 8.2: 컴포넌트 문서
  └─ Task 8.3: 운영 문서
```

## 5. 병렬 실행 판단

### 병렬 가능한 태스크
- **Phase 1 내부**: Task 1.4 (UUID 유틸리티)는 독립적으로 병렬 개발 가능
- **Phase 2 내부**: Task 2.3 (BookmarkItem), Task 2.4 (FolderItem)는 병렬 개발 가능
- **Phase 3 내부**: Task 3.1 (BookmarkDialog), Task 3.2 (FolderDialog), Task 3.3 (ConfirmDialog)는 병렬 개발 가능

### Agent Team 사용 권장 여부
**권장하지 않음 (No)**

**이유**:
1. **Phase 간 의존성이 강함**: 각 Phase는 이전 Phase 완료 후 시작 가능 (순차 진행 필수)
2. **컴포넌트 간 긴밀한 통합**: BookmarkPanel은 모든 하위 컴포넌트를 통합하는 메인 컴포넌트로, 동시 작업 시 충돌 가능성 높음
3. **IndexedDB 서비스 의존성**: 모든 UI 컴포넌트가 BookmarkService에 의존하므로, 서비스 계층 완료 후 UI 작업 시작 필요
4. **설계 변경 시 전체 영향**: BookmarkService의 함수 시그니처 변경 시 모든 컴포넌트 수정 필요 (병렬 작업 시 재작업 증가)

### 병렬 실행 가능 시나리오 (선택적)
- **Phase 3 (다이얼로그)**: BookmarkDialog, FolderDialog, ConfirmDialog는 독립적이므로 3명의 팀원이 동시 개발 가능
- **전제 조건**: Phase 2 완료 후, 설계서(design.md)의 Props 인터페이스 명확히 정의됨
- **Git Worktree 구조**:
  - `.worktrees/bookmark-management-bookmark-dialog/` → `feature/bookmark-management-bookmark-dialog` 브랜치
  - `.worktrees/bookmark-management-folder-dialog/` → `feature/bookmark-management-folder-dialog` 브랜치
  - `.worktrees/bookmark-management-confirm-dialog/` → `feature/bookmark-management-confirm-dialog` 브랜치
- **주의**: 병렬 작업보다 순차 작업이 더 안전하며, 기능 크기가 크지 않아 병렬 작업의 이점이 크지 않음

## 6. 예상 소요 시간

| Phase | 예상 소요 시간 | 비고 |
|-------|----------------|------|
| Phase 1: 데이터 저장소 구현 | 4~6시간 | IndexedDB 학습 곡선 포함 |
| Phase 2: 북마크 UI 컴포넌트 | 6~8시간 | VirtualList 학습 포함 |
| Phase 3: 다이얼로그 구현 | 4~6시간 | 가상 키보드 통합 포함 |
| Phase 4: BrowserView 통합 | 3~4시간 | NavigationBar 수정 포함 |
| Phase 5: 리모컨 키 매핑 | 3~4시간 | Spotlight 학습 포함 |
| Phase 6: 선택적 기능 | 2~3시간 | 선택적 (기본 기능 완료 후) |
| Phase 7: 테스트 및 검증 | 4~6시간 | 프로젝터 환경 테스트 포함 |
| Phase 8: 코드 리뷰 및 문서화 | 2~3시간 | 컴포넌트 문서 작성 포함 |
| **총 예상 시간 (Phase 6 제외)** | **26~37시간** | 약 3~5일 (1일 8시간 기준) |
| **총 예상 시간 (Phase 6 포함)** | **28~40시간** | 약 4~5일 |

## 7. 리스크 및 대응 방안

| 리스크 | 영향도 | 발생 확률 | 대응 방안 |
|--------|--------|-----------|-----------|
| IndexedDB API 호환성 문제 | 높음 | 중간 | webOS 브라우저에서 IndexedDB 사전 테스트, 폴백 대안 준비 (LocalStorage) |
| VirtualList 성능 저하 (북마크 100개 이상) | 중간 | 낮음 | itemSize 최적화, 북마크 수 제한 (최대 500개), 페이지네이션 고려 |
| 리모컨 키 매핑 충돌 | 중간 | 중간 | Spotlight leaveFor 설정 검증, 백 버튼 우선순위 명확화 |
| 중복 북마크 추가 에러 처리 누락 | 낮음 | 중간 | addBookmark() 시 try-catch로 에러 처리, 사용자에게 명확한 메시지 표시 |
| 폴더 삭제 시 하위 북마크 미삭제 | 높음 | 낮음 | deleteFolder() 트랜잭션 검증, 단위 테스트로 하위 북마크 삭제 확인 |
| BrowserView 통합 시 기존 기능 회귀 | 중간 | 중간 | NavigationBar 수정 시 기존 버튼 동작 테스트, 회귀 테스트 실행 |
| 가상 키보드 통합 지연 (F-03 미완료) | 중간 | 낮음 | F-03 완료 전까지 기본 Input 사용, F-03 완료 후 통합 |
| 파비콘 표시 기능 추가 시 CORS 제약 | 낮음 | 높음 | 현재는 파비콘 미지원 (이모지 아이콘 사용), 향후 확장 시 Blob 저장 고려 |

## 8. 검증 계획

### 기능 검증 (완료 기준)
- [ ] **AC-1**: 북마크 추가 버튼 클릭 → 다이얼로그 표시 → 현재 페이지 제목, URL 자동 입력 → 저장 → IndexedDB 저장 → 토스트 메시지 표시
- [ ] **AC-2**: BookmarkPanel 열기 → 저장된 모든 북마크 리스트 표시 → 폴더 구조 트리 뷰 → 리모컨 방향키로 목록 스크롤 → 백 버튼으로 패널 닫기
- [ ] **AC-3**: 북마크 항목 선택 → 해당 URL로 페이지 로드 → 로딩 인디케이터 표시 → 로드 성공 시 BookmarkPanel 자동 닫기
- [ ] **AC-4**: 북마크 편집 버튼 → 편집 다이얼로그 표시 → 기존 정보 자동 입력 → 제목, 폴더 변경 → 저장 → IndexedDB 업데이트 → 토스트 메시지 표시
- [ ] **AC-5**: 북마크 삭제 버튼 → 삭제 확인 다이얼로그 → 확인 → IndexedDB에서 제거 → 목록에서 즉시 제거 → 토스트 메시지 표시
- [ ] **AC-6**: 새 폴더 버튼 → 폴더 생성 다이얼로그 → 폴더 이름 입력 → 생성 → IndexedDB 저장 → 폴더 항목 선택 → 해당 폴더 내 북마크 표시 → 백 버튼으로 상위 폴더 이동
- [ ] **AC-7**: 검색 입력 필드 → 검색어 입력 → 일치하는 북마크 필터링 → 검색 결과 실시간 업데이트 (선택적)
- [ ] **AC-8**: 북마크 추가/편집/삭제 → IndexedDB 즉시 반영 → 앱 재시작 → 저장된 북마크 데이터 정상 로드 (1초 이내)
- [ ] **AC-9**: 방향키로 북마크 목록 탐색 → 선택 버튼으로 북마크 실행 → 백 버튼으로 패널 닫기 → 컬러 버튼으로 북마크 추가/편집/삭제
- [ ] **AC-10**: 북마크 목록 로딩 시간 1초 이내 → 북마크 추가/편집/삭제 반응 속도 0.5초 이내 → 북마크 100개 이상일 때 스크롤 부드럽게 동작 → 폰트 크기 20px 이상 → 포커스 표시 명확

### 회귀 테스트 시나리오
1. **북마크 추가 → 목록에 표시 → 실행 → 페이지 로드 전체 흐름**
   - 현재 페이지에서 북마크 버튼 클릭
   - 북마크 추가 다이얼로그에서 제목, 폴더 선택
   - 저장 후 BookmarkPanel에서 북마크 확인
   - 북마크 선택 후 WebView에서 페이지 로드 확인

2. **폴더 생성 → 북마크를 폴더에 추가 → 폴더 내 북마크 조회**
   - 새 폴더 버튼으로 폴더 생성 (예: "엔터테인먼트")
   - 북마크 추가 시 폴더 선택 (엔터테인먼트)
   - 폴더 항목 클릭 후 해당 폴더 내 북마크 확인

3. **북마크 편집 후 변경 사항 목록 반영**
   - 북마크 편집 버튼으로 제목 변경 (예: "YouTube" → "유튜브")
   - 저장 후 목록에서 변경된 제목 확인

4. **북마크 삭제 후 목록에서 제거, 재시작 후에도 삭제 유지**
   - 북마크 삭제 버튼으로 삭제 확인 다이얼로그 표시
   - 확인 후 목록에서 즉시 제거
   - 앱 재시작 후 삭제된 북마크가 목록에 없음 확인

5. **앱 재시작 후 모든 북마크 및 폴더 데이터 정상 로드**
   - 북마크 3개, 폴더 2개 추가 후 앱 종료
   - 앱 재시작 후 BookmarkPanel 열기
   - 모든 북마크 및 폴더 데이터 정상 로드 확인 (1초 이내)

### 성능 검증
- **로딩 시간**: 북마크 100개 환경에서 BookmarkPanel 열기 시 로딩 시간 측정 (목표: 1초 이내)
- **반응 속도**: 북마크 추가/편집/삭제 후 UI 업데이트 시간 측정 (목표: 0.5초 이내)
- **스크롤 성능**: 북마크 100개 이상 환경에서 VirtualList 스크롤 프레임레이트 측정 (목표: 30fps 이상)
- **메모리 사용량**: 북마크 100개 환경에서 BookmarkPanel 메모리 사용량 측정 (목표: 100MB 이하)

### 리모컨 접근성 검증
- [ ] 방향키 상/하로 북마크/폴더 목록 스크롤 가능
- [ ] 방향키 좌로 상위 폴더로 이동 가능
- [ ] 방향키 우로 폴더 열기 가능
- [ ] 선택 버튼으로 북마크 실행, 폴더 열기 가능
- [ ] 백 버튼으로 BookmarkPanel 닫기, 다이얼로그 닫기 가능
- [ ] 컬러 버튼으로 북마크 추가/편집/삭제, 폴더 생성 가능 (선택적)
- [ ] 옵션 버튼으로 컨텍스트 메뉴 열기 가능
- [ ] 모든 기능을 마우스 없이 리모컨만으로 사용 가능

## 9. 참고 사항

### 관련 기능 연동
- **F-02 (웹뷰 통합)**: 북마크 실행 시 WebView로 페이지 로드
- **F-03 (URL 입력 UI)**: URLBar 자동완성에 북마크 데이터 제공 (F-03 완료 후)
- **F-04 (페이지 탐색 컨트롤)**: NavigationBar에 북마크 버튼 추가
- **F-05 (로딩 인디케이터)**: 북마크 실행 시 로딩 인디케이터 표시
- **F-06 (탭 관리)**: 북마크 클릭 시 "새 탭에서 열기" 옵션 추가 (F-06 완료 후)
- **F-08 (히스토리 관리)**: 북마크 실행 시 히스토리에 자동 기록 (F-08 완료 후)

### 기술 참고 자료
- Enact VirtualList: https://enactjs.com/docs/modules/moonstone/VirtualList/
- Enact Panels: https://enactjs.com/docs/modules/moonstone/Panels/
- Enact Dialog: https://enactjs.com/docs/modules/moonstone/Dialog/
- IndexedDB API: https://developer.mozilla.org/en-US/docs/Web/API/IndexedDB_API
- Enact Spotlight: https://enactjs.com/docs/modules/spotlight/Spotlight/

### 우선순위 조정 가능성
- **Phase 6 (선택적 기능)**: 검색 및 정렬 기능은 Could 우선순위로, 기본 북마크 CRUD 완료 후 추가 가능 (M3 이후 확장 가능)
- **폴더 관리**: Should 우선순위로, 기본 북마크 CRUD 완료 후 추가 가능 (현재는 Must로 포함)

### 병렬 작업 가능성 (재검토)
- F-07 (북마크 관리), F-08 (히스토리 관리), F-09 (검색 엔진 통합)은 PG-2 병렬 그룹으로 독립적으로 개발 가능
- BookmarkService와 HistoryService는 별도 IndexedDB 오브젝트 스토어 사용으로 충돌 없음
- 단, BrowserView 통합 시 NavigationBar 수정이 충돌 가능하므로, 순차 통합 권장

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-13 | 최초 작성 | F-07 요구사항 및 설계서 기반 구현 계획 수립 |
