# 북마크 관리 — 기술 설계서

## 1. 참조
- 요구사항 분석서: docs/specs/bookmark-management/requirements.md
- PRD: docs/project/prd.md
- CLAUDE.md: /Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md

## 2. 아키텍처 개요

### 전체 구조
사용자가 자주 방문하는 웹사이트를 북마크로 저장하고 관리할 수 있는 기능을 구현합니다. IndexedDB로 영속 데이터를 저장하며, 리모컨 최적화 UI로 북마크 CRUD 및 폴더 관리를 제공합니다.

```
┌─────────────────────────────────────────────────────────┐
│                      BrowserView                         │
│  ┌────────────────────────────────────────────────────┐ │
│  │             NavigationBar (F-04)                    │ │
│  │  [뒤로] [앞으로] [새로고침] [홈] [북마크]          │ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │                  WebView (F-02)                     │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘

BookmarkPanel (북마크 버튼 클릭 시 오버레이)
┌─────────────────────────────────────────────────────────┐
│  북마크                                            [닫기] │
│  ────────────────────────────────────────────────────   │
│  📁 루트 폴더                           [새 폴더] [추가] │
│  ┌──────────────────────────────────────────────────┐  │
│  │  📁 엔터테인먼트              ▶                   │  │
│  │  🔖 YouTube                                       │  │
│  │      https://www.youtube.com                      │  │
│  │  🔖 Netflix                                       │  │
│  │      https://www.netflix.com                      │  │
│  │  📁 뉴스                          ▶                │  │
│  │  🔖 Naver                                         │  │
│  │      https://www.naver.com                        │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### 핵심 설계 원칙
1. **영속성**: IndexedDB로 북마크 데이터 저장 (앱 재시작 후에도 유지)
2. **계층형 폴더 구조**: 1단계 서브폴더 지원 (루트 > 폴더 > 북마크)
3. **리모컨 최적화**: Spotlight 통합으로 방향키 탐색, 선택 버튼으로 실행
4. **CRUD 완전 지원**: 추가, 조회, 편집, 삭제, 폴더 관리
5. **BrowserView 연동**: 현재 페이지 북마크 추가, 북마크에서 페이지 열기

## 3. 아키텍처 결정

### 결정 1: 데이터 저장소 선택
- **선택지**:
  - A) LocalStorage (5MB 제한)
  - B) IndexedDB (최소 50MB 이상)
  - C) webOS LS2 API (LG 전용, 데이터베이스 서비스)
- **결정**: B) IndexedDB
- **근거**:
  - LocalStorage는 용량 제한(5MB)으로 북마크 수백 개 저장 시 부족
  - IndexedDB는 구조화된 데이터 저장에 적합 (인덱스, 트랜잭션 지원)
  - webOS LS2 API는 문서 부족, IndexedDB가 웹 표준으로 호환성 우수
  - 북마크 검색, 정렬 시 인덱스 활용으로 성능 향상
- **트레이드오프**: 비동기 API로 코드 복잡도 증가 (Promise 기반)

### 결정 2: 폴더 구조 깊이
- **선택지**:
  - A) 단일 레벨 (폴더 없음, 북마크만 평면 리스트)
  - B) 1단계 서브폴더 (루트 > 폴더 > 북마크)
  - C) 다단계 폴더 (무제한 깊이)
- **결정**: B) 1단계 서브폴더
- **근거**:
  - 리모컨 탐색 시 너무 깊은 계층은 사용자 경험 저하
  - 대부분의 사용자는 1~2단계 분류로 충분 (엔터테인먼트, 뉴스, 쇼핑 등)
  - UI 복잡도 감소 (브레드크럼, 백 네비게이션 단순화)
  - 향후 확장 가능 (데이터 모델은 parentId 지원)
- **트레이드오프**: 복잡한 분류 체계 지원 불가 (향후 확장 가능)

### 결정 3: UI 패턴 (패널 vs 전체 화면)
- **선택지**:
  - A) Enact Panel (오버레이 패널, 화면 일부 차지)
  - B) 전체 화면 뷰 (라우팅 기반, BrowserView와 분리)
  - C) 드롭다운 리스트 (URLBar 아래 작은 드롭다운)
- **결정**: A) Enact Panel (오버레이 패널)
- **근거**:
  - 빠른 접근성: 북마크 버튼 → 패널 오버레이 → 북마크 선택 → 페이지 로드 (최소 단계)
  - 컨텍스트 유지: WebView 화면 위에 오버레이되어 현재 페이지 컨텍스트 유지
  - Enact Panels는 리모컨 네비게이션 최적화 (Spotlight 자동 관리)
  - 드롭다운은 북마크 수 많을 때 스크롤 어려움
- **트레이드오프**: 화면 크기 제약 (패널 크기 조정 필요)

### 결정 4: 북마크 추가 UI 흐름
- **선택지**:
  - A) 북마크 버튼 클릭 → 즉시 추가 (현재 페이지, 자동 제목)
  - B) 북마크 버튼 클릭 → 다이얼로그 표시 (제목, 폴더 선택)
  - C) BookmarkPanel 내부에서만 추가 가능 (URLBar에 버튼 없음)
- **결정**: B) 북마크 버튼 클릭 → 다이얼로그 표시
- **근거**:
  - 사용자가 제목 편집, 폴더 선택 가능 (유연성 제공)
  - 중복 북마크 추가 방지 (다이얼로그에서 중복 체크)
  - 즉시 추가는 실수로 중복 추가 가능성
- **트레이드오프**: 한 번 더 클릭 필요 (UX 트레이드오프)

### 결정 5: 북마크 실행 방식
- **선택지**:
  - A) 현재 탭에서 열기 (WebView URL 변경)
  - B) 새 탭에서 열기 (F-06 탭 관리 필요)
  - C) 선택 버튼: 현재 탭, 컨텍스트 메뉴: 새 탭
- **결정**: A) 현재 탭에서 열기 (기본), C) 향후 F-06 연동
- **근거**:
  - F-06 탭 관리 미구현 시 현재 탭에서만 열기 가능
  - 대부분의 사용자는 현재 탭 전환으로 충분
  - F-06 완료 후 컨텍스트 메뉴로 "새 탭에서 열기" 옵션 추가
- **트레이드오프**: 현재는 단일 탭만 지원

### 결정 6: 북마크 자동완성 연동 (F-03 URLBar)
- **선택지**:
  - A) URLBar에 북마크 자동완성 표시 (제목, URL 일치)
  - B) 북마크 패널에서만 접근 (자동완성 미제공)
  - C) 북마크 검색 기능 제공 (BookmarkPanel 내부)
- **결정**: A) + C) 조합 (URLBar 자동완성 + 패널 내 검색)
- **근거**:
  - URLBar 자동완성은 빠른 접근성 제공 (F-03과 연동)
  - BookmarkPanel 내부 검색은 북마크 목록 탐색 시 유용
  - 북마크 수 많을 때 검색 필수 기능
- **구현**: F-03 완료 후 URLBar에 `suggestions` prop으로 북마크 제공

## 4. 데이터 모델 설계

### IndexedDB 스키마

#### 오브젝트 스토어: `bookmarks`
| 컬럼 | 타입 | 제약조건 | 설명 |
|------|------|----------|------|
| id | String | PK, UUID | 북마크 고유 식별자 |
| title | String | NOT NULL | 북마크 제목 (사용자 편집 가능) |
| url | String | NOT NULL, UNIQUE | 북마크 URL (중복 방지) |
| folderId | String | NULL | 폴더 ID (NULL이면 루트 폴더) |
| favicon | String | NULL | 파비콘 URL (선택적 기능) |
| description | String | NULL | 북마크 설명 메모 (선택적) |
| createdAt | Number | NOT NULL | 생성 시각 (Unix timestamp) |
| updatedAt | Number | NOT NULL | 수정 시각 (Unix timestamp) |
| visitCount | Number | DEFAULT 0 | 방문 횟수 (정렬용) |

**인덱스**:
- `folderId`: 폴더별 북마크 조회 (복합 인덱스: folderId + createdAt)
- `url`: 중복 체크 (UNIQUE 인덱스)
- `title`: 검색 기능 (부분 일치 검색)
- `createdAt`: 날짜순 정렬

#### 오브젝트 스토어: `folders`
| 컬럼 | 타입 | 제약조건 | 설명 |
|------|------|----------|------|
| id | String | PK, UUID | 폴더 고유 식별자 |
| name | String | NOT NULL | 폴더 이름 |
| parentId | String | NULL | 부모 폴더 ID (NULL이면 루트) |
| createdAt | Number | NOT NULL | 생성 시각 (Unix timestamp) |

**인덱스**:
- `parentId`: 서브폴더 조회
- `name`: 폴더 이름 검색

### IndexedDB 초기화 코드
```javascript
// src/services/indexedDBService.js
const DB_NAME = 'webOSBrowserDB'
const DB_VERSION = 1

/**
 * IndexedDB 초기화 및 스키마 생성
 */
export const initDB = () => {
	return new Promise((resolve, reject) => {
		const request = indexedDB.open(DB_NAME, DB_VERSION)

		request.onerror = () => reject(request.error)
		request.onsuccess = () => resolve(request.result)

		request.onupgradeneeded = (event) => {
			const db = event.target.result

			// bookmarks 오브젝트 스토어 생성
			if (!db.objectStoreNames.contains('bookmarks')) {
				const bookmarkStore = db.createObjectStore('bookmarks', { keyPath: 'id' })
				bookmarkStore.createIndex('folderId', 'folderId', { unique: false })
				bookmarkStore.createIndex('url', 'url', { unique: true })
				bookmarkStore.createIndex('title', 'title', { unique: false })
				bookmarkStore.createIndex('createdAt', 'createdAt', { unique: false })
			}

			// folders 오브젝트 스토어 생성
			if (!db.objectStoreNames.contains('folders')) {
				const folderStore = db.createObjectStore('folders', { keyPath: 'id' })
				folderStore.createIndex('parentId', 'parentId', { unique: false })
				folderStore.createIndex('name', 'name', { unique: false })
			}
		}
	})
}
```

## 5. 서비스 계층 설계

### BookmarkService 구조
```
src/services/bookmarkService.js
├── initDB()                  # IndexedDB 초기화
├── getAllBookmarks()         # 모든 북마크 조회
├── getBookmarksByFolder(folderId)  # 폴더별 북마크 조회
├── getBookmarkById(id)       # 단일 북마크 조회
├── getBookmarkByUrl(url)     # URL로 북마크 조회 (중복 체크)
├── addBookmark(bookmark)     # 북마크 추가
├── updateBookmark(id, updates) # 북마크 수정
├── deleteBookmark(id)        # 북마크 삭제
├── searchBookmarks(query)    # 북마크 검색 (제목, URL)
├── incrementVisitCount(id)   # 방문 횟수 증가
├── getAllFolders()           # 모든 폴더 조회
├── getFoldersByParent(parentId) # 서브폴더 조회
├── addFolder(folder)         # 폴더 추가
├── updateFolder(id, updates) # 폴더 이름 변경
└── deleteFolder(id)          # 폴더 삭제 (하위 북마크 포함)
```

### 주요 함수 구현 예시

#### addBookmark()
```javascript
/**
 * 북마크 추가
 * @param {Object} bookmark - { title, url, folderId, description }
 * @returns {Promise<Object>} - 추가된 북마크 객체
 */
export const addBookmark = async (bookmark) => {
	const db = await initDB()

	// 중복 체크
	const existing = await getBookmarkByUrl(bookmark.url)
	if (existing) {
		throw new Error('이미 북마크에 추가된 URL입니다.')
	}

	// 북마크 객체 생성
	const newBookmark = {
		id: generateUUID(),
		title: bookmark.title,
		url: bookmark.url,
		folderId: bookmark.folderId || null,
		favicon: bookmark.favicon || null,
		description: bookmark.description || null,
		createdAt: Date.now(),
		updatedAt: Date.now(),
		visitCount: 0
	}

	// IndexedDB에 저장
	const transaction = db.transaction(['bookmarks'], 'readwrite')
	const store = transaction.objectStore('bookmarks')
	await promisifyRequest(store.add(newBookmark))

	logger.info('[BookmarkService] 북마크 추가 성공:', newBookmark)
	return newBookmark
}
```

#### deleteFolder()
```javascript
/**
 * 폴더 삭제 (하위 북마크도 함께 삭제)
 * @param {String} folderId - 폴더 ID
 * @returns {Promise<void>}
 */
export const deleteFolder = async (folderId) => {
	const db = await initDB()

	// 폴더 내 북마크 조회
	const bookmarksInFolder = await getBookmarksByFolder(folderId)

	// 트랜잭션으로 폴더 + 북마크 일괄 삭제
	const transaction = db.transaction(['bookmarks', 'folders'], 'readwrite')
	const bookmarkStore = transaction.objectStore('bookmarks')
	const folderStore = transaction.objectStore('folders')

	// 북마크 삭제
	for (const bookmark of bookmarksInFolder) {
		await promisifyRequest(bookmarkStore.delete(bookmark.id))
	}

	// 폴더 삭제
	await promisifyRequest(folderStore.delete(folderId))

	logger.info('[BookmarkService] 폴더 삭제 성공:', { folderId, deletedBookmarks: bookmarksInFolder.length })
}
```

#### searchBookmarks()
```javascript
/**
 * 북마크 검색 (제목, URL에서 부분 일치)
 * @param {String} query - 검색어
 * @returns {Promise<Array>} - 검색 결과 북마크 배열
 */
export const searchBookmarks = async (query) => {
	const allBookmarks = await getAllBookmarks()
	const lowerQuery = query.toLowerCase()

	return allBookmarks.filter(bookmark =>
		bookmark.title.toLowerCase().includes(lowerQuery) ||
		bookmark.url.toLowerCase().includes(lowerQuery)
	)
}
```

## 6. 컴포넌트 설계

### BookmarkPanel 컴포넌트

#### 컴포넌트 구조
```
src/components/BookmarkPanel/
├── BookmarkPanel.js           # 메인 패널 컴포넌트
├── BookmarkList.js            # 북마크 리스트
├── BookmarkItem.js            # 북마크 항목
├── FolderItem.js              # 폴더 항목
├── BookmarkDialog.js          # 북마크 추가/편집 다이얼로그
├── FolderDialog.js            # 폴더 추가/편집 다이얼로그
├── ConfirmDialog.js           # 삭제 확인 다이얼로그
├── BookmarkPanel.module.less  # 스타일
└── index.js                   # Export 진입점
```

#### BookmarkPanel Props 인터페이스
```javascript
// src/components/BookmarkPanel/BookmarkPanel.js
import PropTypes from 'prop-types'

BookmarkPanel.propTypes = {
	// 패널 표시 상태
	visible: PropTypes.bool.isRequired,

	// 현재 페이지 정보 (북마크 추가 시 사용)
	currentUrl: PropTypes.string,
	currentTitle: PropTypes.string,

	// 콜백
	onClose: PropTypes.func.isRequired,              // 패널 닫기
	onBookmarkSelect: PropTypes.func.isRequired,     // 북마크 선택 시 페이지 열기
	onBookmarkAdded: PropTypes.func,                 // 북마크 추가 완료 (토스트 메시지용)

	// 스타일 커스터마이징
	className: PropTypes.string
}

BookmarkPanel.defaultProps = {
	currentUrl: '',
	currentTitle: '',
	onBookmarkAdded: () => {},
	className: ''
}
```

#### BookmarkPanel 상태 관리
```javascript
// src/components/BookmarkPanel/BookmarkPanel.js
import { useState, useEffect } from 'react'
import * as bookmarkService from '../../services/bookmarkService'

const BookmarkPanel = ({ visible, currentUrl, currentTitle, onClose, onBookmarkSelect }) => {
	// 북마크 및 폴더 데이터
	const [bookmarks, setBookmarks] = useState([])
	const [folders, setFolders] = useState([])
	const [currentFolderId, setCurrentFolderId] = useState(null)  // 현재 탐색 중인 폴더

	// UI 상태
	const [showAddDialog, setShowAddDialog] = useState(false)
	const [showFolderDialog, setShowFolderDialog] = useState(false)
	const [editingBookmark, setEditingBookmark] = useState(null)  // 편집 중인 북마크
	const [editingFolder, setEditingFolder] = useState(null)      // 편집 중인 폴더
	const [searchQuery, setSearchQuery] = useState('')            // 검색어

	// 검색 결과
	const [searchResults, setSearchResults] = useState([])

	// IndexedDB 초기화 및 데이터 로드
	useEffect(() => {
		if (visible) {
			loadBookmarks()
			loadFolders()
		}
	}, [visible, currentFolderId])

	const loadBookmarks = async () => {
		try {
			const data = currentFolderId
				? await bookmarkService.getBookmarksByFolder(currentFolderId)
				: await bookmarkService.getAllBookmarks()
			setBookmarks(data)
		} catch (error) {
			logger.error('[BookmarkPanel] 북마크 로드 실패:', error)
		}
	}

	const loadFolders = async () => {
		try {
			const data = await bookmarkService.getAllFolders()
			setFolders(data)
		} catch (error) {
			logger.error('[BookmarkPanel] 폴더 로드 실패:', error)
		}
	}

	// ... (핸들러 함수 생략)
}
```

### BookmarkList 컴포넌트 (Enact VirtualList)
```javascript
// src/components/BookmarkPanel/BookmarkList.js
import { VirtualList } from '@enact/moonstone/VirtualList'
import BookmarkItem from './BookmarkItem'
import FolderItem from './FolderItem'

const BookmarkList = ({ bookmarks, folders, onBookmarkClick, onFolderClick, onEdit, onDelete }) => {
	// 폴더 + 북마크 결합 (폴더 우선 표시)
	const items = [
		...folders.map(folder => ({ type: 'folder', data: folder })),
		...bookmarks.map(bookmark => ({ type: 'bookmark', data: bookmark }))
	]

	const renderItem = ({ index, ...rest }) => {
		const item = items[index]

		if (item.type === 'folder') {
			return (
				<FolderItem
					{...rest}
					folder={item.data}
					onClick={onFolderClick}
					onEdit={onEdit}
					onDelete={onDelete}
				/>
			)
		} else {
			return (
				<BookmarkItem
					{...rest}
					bookmark={item.data}
					onClick={onBookmarkClick}
					onEdit={onEdit}
					onDelete={onDelete}
				/>
			)
		}
	}

	return (
		<VirtualList
			dataSize={items.length}
			itemRenderer={renderItem}
			itemSize={80}  // 항목 높이 (리모컨 포커스 고려)
			spacing={8}
		/>
	)
}

export default BookmarkList
```

### BookmarkItem 컴포넌트
```javascript
// src/components/BookmarkPanel/BookmarkItem.js
import { useState } from 'react'
import Button from '@enact/moonstone/Button'
import Spotlight from '@enact/spotlight'
import css from './BookmarkPanel.module.less'

const BookmarkItem = ({ bookmark, onClick, onEdit, onDelete, ...rest }) => {
	const [showMenu, setShowMenu] = useState(false)

	const handleClick = () => {
		onClick(bookmark)
	}

	const handleContextMenu = () => {
		setShowMenu(!showMenu)
	}

	return (
		<div className={css.bookmarkItem} {...rest}>
			{/* 북마크 정보 영역 (선택 버튼으로 페이지 열기) */}
			<div
				className={css.bookmarkInfo}
				onClick={handleClick}
				spotlightId={`bookmark-${bookmark.id}`}
			>
				<div className={css.favicon}>🔖</div>
				<div className={css.details}>
					<div className={css.title}>{bookmark.title}</div>
					<div className={css.url}>{bookmark.url}</div>
				</div>
			</div>

			{/* 컨텍스트 메뉴 (옵션 버튼) */}
			{showMenu && (
				<div className={css.contextMenu}>
					<Button onClick={() => onEdit(bookmark)} small>편집</Button>
					<Button onClick={() => onDelete(bookmark)} small>삭제</Button>
				</div>
			)}
		</div>
	)
}

export default BookmarkItem
```

### BookmarkDialog 컴포넌트 (추가/편집)
```javascript
// src/components/BookmarkPanel/BookmarkDialog.js
import { useState, useEffect } from 'react'
import Dialog from '@enact/moonstone/Dialog'
import Input from '@enact/moonstone/Input'
import Dropdown from '@enact/moonstone/Dropdown'
import Button from '@enact/moonstone/Button'
import VirtualKeyboard from '../VirtualKeyboard'

const BookmarkDialog = ({ visible, bookmark, folders, onSave, onCancel }) => {
	const [title, setTitle] = useState('')
	const [url, setUrl] = useState('')
	const [folderId, setFolderId] = useState(null)
	const [showKeyboard, setShowKeyboard] = useState(false)

	useEffect(() => {
		if (bookmark) {
			// 편집 모드
			setTitle(bookmark.title)
			setUrl(bookmark.url)
			setFolderId(bookmark.folderId)
		} else {
			// 추가 모드 (초기화)
			setTitle('')
			setUrl('')
			setFolderId(null)
		}
	}, [bookmark, visible])

	const handleSave = () => {
		const data = { title, url, folderId }
		onSave(data)
	}

	return (
		<Dialog open={visible} onClose={onCancel} title={bookmark ? '북마크 편집' : '북마크 추가'}>
			<Input
				value={title}
				onChange={(e) => setTitle(e.value)}
				onFocus={() => setShowKeyboard(true)}
				placeholder="북마크 제목"
			/>
			<Input
				value={url}
				onChange={(e) => setUrl(e.value)}
				onFocus={() => setShowKeyboard(true)}
				placeholder="URL"
				disabled={!!bookmark}  // 편집 시 URL 변경 불가
			/>
			<Dropdown
				selected={folderId}
				onSelect={({ selected }) => setFolderId(selected)}
			>
				<option value={null}>루트 폴더</option>
				{folders.map(folder => (
					<option key={folder.id} value={folder.id}>{folder.name}</option>
				))}
			</Dropdown>

			{showKeyboard && (
				<VirtualKeyboard
					visible={showKeyboard}
					onCancel={() => setShowKeyboard(false)}
				/>
			)}

			<Button onClick={handleSave}>저장</Button>
			<Button onClick={onCancel}>취소</Button>
		</Dialog>
	)
}

export default BookmarkDialog
```

## 7. 시퀀스 흐름

### 주요 시나리오: 북마크 추가
```
사용자         BrowserView    NavigationBar   BookmarkPanel   BookmarkDialog   BookmarkService   IndexedDB
  │                │               │                │                │                 │              │
  │  북마크 버튼 클릭              │                │                │                 │              │
  │───────────────────────────────▶│                │                │                 │              │
  │                │               │  setShowBookmarkPanel(true)     │                 │              │
  │                │               │────────────────▶│                │                 │              │
  │                │               │                │  loadBookmarks()                 │              │
  │                │               │                │────────────────────────────────▶│              │
  │                │               │                │                │  getAllBookmarks()            │
  │                │               │                │                │                 │─────────────▶│
  │                │               │                │                │                 │◀─────────────│
  │                │               │                │◀────────────────────────────────│              │
  │                │               │                │  BookmarkPanel 렌더링           │              │
  │                │               │                │                │                 │              │
  │  "추가" 버튼 클릭              │                │                │                 │              │
  │───────────────────────────────────────────────▶│                │                 │              │
  │                │               │                │  setShowAddDialog(true)          │              │
  │                │               │                │────────────────▶│                 │              │
  │                │               │                │                │  currentTitle, currentUrl 자동 입력
  │                │               │                │                │                 │              │
  │  제목 편집, 폴더 선택          │                │                │                 │              │
  │───────────────────────────────────────────────────────────────▶│                 │              │
  │                │               │                │                │                 │              │
  │  "저장" 버튼 클릭              │                │                │                 │              │
  │───────────────────────────────────────────────────────────────▶│                 │              │
  │                │               │                │                │  addBookmark({ title, url, folderId })
  │                │               │                │                │─────────────────▶              │
  │                │               │                │                │                 │  getBookmarkByUrl(url)
  │                │               │                │                │                 │─────────────▶│
  │                │               │                │                │                 │◀─────────────│
  │                │               │                │                │                 │  (중복 체크)
  │                │               │                │                │                 │  store.add(newBookmark)
  │                │               │                │                │                 │─────────────▶│
  │                │               │                │                │                 │◀─────────────│
  │                │               │                │                │◀─────────────────              │
  │                │               │                │  loadBookmarks() (목록 갱신)     │              │
  │                │               │                │────────────────────────────────▶│              │
  │                │               │                │◀────────────────────────────────│              │
  │                │               │                │  토스트 메시지 표시              │              │
  │◀───────────────────────────────────────────────│  "북마크가 저장되었습니다"       │              │
```

### 에러 시나리오: 중복 북마크 추가 시도
```
사용자         BookmarkDialog   BookmarkService   IndexedDB
  │                 │                 │              │
  │  "저장" 클릭    │                 │              │
  │────────────────▶│                 │              │
  │                 │  addBookmark()  │              │
  │                 │─────────────────▶              │
  │                 │                 │  getBookmarkByUrl(url)
  │                 │                 │─────────────▶│
  │                 │                 │◀─────────────│
  │                 │                 │  (existing 발견)
  │                 │  throw Error('이미 북마크에 추가된 URL입니다.')
  │                 │◀─────────────────              │
  │  에러 다이얼로그 표시             │              │
  │◀────────────────│  "이미 북마크에 추가된 페이지입니다"
```

### 주요 시나리오: 북마크 실행
```
사용자         BookmarkPanel   BookmarkService   BrowserView   WebView
  │                │                 │              │            │
  │  북마크 선택   │                 │              │            │
  │───────────────▶│                 │              │            │
  │                │  incrementVisitCount(id)       │            │
  │                │─────────────────▶              │            │
  │                │                 │  store.put({ visitCount: visitCount + 1 })
  │                │◀─────────────────              │            │
  │                │  onBookmarkSelect({ url })     │            │
  │                │────────────────────────────────▶│            │
  │                │                 │              │  setCurrentUrl(url)
  │                │                 │              │───────────▶│
  │                │  onClose()      │              │            │
  │                │  (패널 닫기)    │              │            │
```

## 8. 리모컨 키 매핑

### BookmarkPanel 포커스 흐름
```
Spotlight Container: BookmarkPanel
├── Header (닫기 버튼)
├── ActionBar (추가, 새 폴더, 검색)
└── BookmarkList (VirtualList)
    ├── FolderItem (방향키 하)
    ├── FolderItem (방향키 하)
    └── BookmarkItem (방향키 하)
```

### 리모컨 키 매핑
| 키 | 동작 | 컨텍스트 |
|----|------|----------|
| **방향키 상/하** | 북마크/폴더 목록 스크롤 | BookmarkList |
| **방향키 좌** | 상위 폴더로 이동 | 서브폴더 탐색 중 |
| **방향키 우** | 폴더 열기 (서브폴더 표시) | 폴더 항목 포커스 시 |
| **선택 버튼 (Enter/OK)** | 북마크 실행 (페이지 열기), 폴더 열기 | BookmarkItem, FolderItem |
| **백 버튼 (Backspace)** | BookmarkPanel 닫기, 다이얼로그 닫기 | 패널 또는 다이얼로그 열림 시 |
| **컬러 버튼 (빨강)** | 북마크 추가 (현재 페이지) | BookmarkPanel 열림 시 |
| **컬러 버튼 (파랑)** | 북마크 편집 | BookmarkItem 포커스 시 |
| **컬러 버튼 (노랑)** | 북마크 삭제 | BookmarkItem 포커스 시 |
| **컬러 버튼 (초록)** | 새 폴더 생성 | BookmarkPanel 열림 시 |
| **옵션 버튼** | 컨텍스트 메뉴 열기 (편집, 삭제, 새 탭에서 열기) | BookmarkItem 포커스 시 |

### Spotlight 설정 (BookmarkPanel)
```javascript
// src/components/BookmarkPanel/BookmarkPanel.js
useEffect(() => {
	if (visible) {
		// BookmarkPanel 포커스 설정
		Spotlight.set('bookmark-panel', {
			defaultElement: '[data-spotlight-id="bookmark-list"]',
			enterTo: 'default-element',
			leaveFor: {
				left: '',  // 좌측 이동 방지 (상위 폴더 이동으로 처리)
				right: ''  // 우측 이동 방지 (폴더 열기로 처리)
			}
		})

		// 초기 포커스를 BookmarkList로 이동
		Spotlight.focus('bookmark-list')
	}
}, [visible])

// 방향키 좌: 상위 폴더로 이동
const handleKeyDown = (event) => {
	if (event.keyCode === 37 && currentFolderId) {  // Arrow Left
		event.preventDefault()
		setCurrentFolderId(null)  // 루트 폴더로 이동
		logger.info('[BookmarkPanel] 상위 폴더로 이동')
	}
}
```

## 9. BrowserView 통합

### NavigationBar에 북마크 버튼 추가
```javascript
// src/components/NavigationBar/NavigationBar.js (수정)
<Button
	className={css.navButton}
	onClick={onBookmarkClick}
	icon="star"
	spotlightId="nav-bookmark"
>
	북마크
</Button>
```

### BrowserView에서 BookmarkPanel 통합
```javascript
// src/views/BrowserView.js (수정)
import { useState } from 'react'
import BookmarkPanel from '../components/BookmarkPanel'

const BrowserView = () => {
	const [currentUrl, setCurrentUrl] = useState('https://www.google.com')
	const [currentTitle, setCurrentTitle] = useState('Google')
	const [showBookmarkPanel, setShowBookmarkPanel] = useState(false)

	// 북마크 버튼 클릭 핸들러 (NavigationBar에서 전달)
	const handleBookmarkClick = () => {
		setShowBookmarkPanel(true)
	}

	// 북마크 선택 핸들러
	const handleBookmarkSelect = (bookmark) => {
		setCurrentUrl(bookmark.url)
		setCurrentTitle(bookmark.title)
		setShowBookmarkPanel(false)
		logger.info('[BrowserView] 북마크에서 페이지 열기:', bookmark.url)
	}

	return (
		<Panel className={css.browserView}>
			<NavigationBar
				onBookmarkClick={handleBookmarkClick}
				{/* ... 기타 props */}
			/>

			<WebView
				url={currentUrl}
				onLoadEnd={({ title }) => setCurrentTitle(title)}
				{/* ... 기타 props */}
			/>

			{/* BookmarkPanel 오버레이 */}
			<BookmarkPanel
				visible={showBookmarkPanel}
				currentUrl={currentUrl}
				currentTitle={currentTitle}
				onClose={() => setShowBookmarkPanel(false)}
				onBookmarkSelect={handleBookmarkSelect}
			/>
		</Panel>
	)
}
```

## 10. 스타일 설계

### BookmarkPanel 스타일
```less
// src/components/BookmarkPanel/BookmarkPanel.module.less
.bookmarkPanel {
	position: fixed;
	top: 0;
	right: 0;
	width: 600px;  // 패널 너비 (리모컨 탐색 고려)
	height: 100vh;
	background-color: var(--bg-color);
	box-shadow: -4px 0 16px rgba(0, 0, 0, 0.5);
	z-index: 1000;
	display: flex;
	flex-direction: column;
	padding: var(--spacing-lg);
}

.header {
	display: flex;
	justify-content: space-between;
	align-items: center;
	margin-bottom: var(--spacing-md);
}

.title {
	font-size: 28px;  // 대화면 가독성
	font-weight: bold;
	color: var(--text-color);
}

.actionBar {
	display: flex;
	gap: 12px;
	margin-bottom: var(--spacing-md);
}

.bookmarkList {
	flex: 1;
	overflow-y: auto;
}

.bookmarkItem {
	display: flex;
	align-items: center;
	padding: 16px;
	background-color: rgba(255, 255, 255, 0.05);
	border-radius: 8px;
	margin-bottom: 8px;
	cursor: pointer;
	transition: background-color 0.2s;

	&:focus {
		outline: 3px solid var(--accent-color);  // 포커스 표시
		background-color: rgba(255, 255, 255, 0.1);
	}
}

.bookmarkInfo {
	display: flex;
	align-items: center;
	flex: 1;
}

.favicon {
	font-size: 32px;
	margin-right: 16px;
}

.details {
	flex: 1;
}

.title {
	font-size: 20px;
	color: var(--text-color);
	margin-bottom: 4px;
}

.url {
	font-size: 16px;
	color: var(--text-secondary-color);
	overflow: hidden;
	text-overflow: ellipsis;
	white-space: nowrap;
}

.contextMenu {
	display: flex;
	gap: 8px;
}

// 폴더 항목 스타일
.folderItem {
	composes: bookmarkItem;

	.folderIcon {
		font-size: 32px;
		margin-right: 16px;
	}
}
```

## 11. 영향 범위 분석

### 수정 필요한 기존 파일
1. **src/views/BrowserView.js**:
   - BookmarkPanel import 추가
   - showBookmarkPanel 상태 추가
   - handleBookmarkClick, handleBookmarkSelect 핸들러 추가
   - JSX에 `<BookmarkPanel>` 오버레이 추가
2. **src/components/NavigationBar/NavigationBar.js**:
   - 북마크 버튼 추가 (icon="star")
   - onBookmarkClick prop 추가
3. **src/components/URLBar/URLBar.js** (F-03 완료 후):
   - suggestions prop에 북마크 데이터 추가 (BookmarkService에서 제공)
   - 자동완성 드롭다운에 북마크 표시

### 새로 생성할 파일
1. **src/services/bookmarkService.js**: 북마크 CRUD 서비스
2. **src/services/indexedDBService.js**: IndexedDB 초기화 및 유틸리티
3. **src/components/BookmarkPanel/BookmarkPanel.js**: 메인 패널 컴포넌트
4. **src/components/BookmarkPanel/BookmarkList.js**: VirtualList 래퍼
5. **src/components/BookmarkPanel/BookmarkItem.js**: 북마크 항목
6. **src/components/BookmarkPanel/FolderItem.js**: 폴더 항목
7. **src/components/BookmarkPanel/BookmarkDialog.js**: 추가/편집 다이얼로그
8. **src/components/BookmarkPanel/FolderDialog.js**: 폴더 다이얼로그
9. **src/components/BookmarkPanel/ConfirmDialog.js**: 삭제 확인 다이얼로그
10. **src/components/BookmarkPanel/BookmarkPanel.module.less**: 스타일
11. **src/components/BookmarkPanel/index.js**: Export 진입점
12. **src/utils/uuid.js**: UUID 생성 유틸리티 (crypto.randomUUID 또는 폴백)

### 영향 받는 기존 기능
- **F-03 (URL 입력 UI)**: BookmarkService에서 북마크 데이터를 제공하여 자동완성 표시
- **F-04 (페이지 탐색 컨트롤)**: NavigationBar에 북마크 버튼 추가
- **F-06 (탭 관리, 향후)**: 북마크 클릭 시 "새 탭에서 열기" 옵션 추가
- **F-08 (히스토리 관리)**: 북마크 실행 시 히스토리에 자동 기록 (onBookmarkSelect에서 호출)

## 12. 기술적 주의사항

### IndexedDB 비동기 처리
- **문제**: IndexedDB는 비동기 API로 Promise 기반 처리 필요
- **대응**: 모든 IndexedDB 작업을 async/await로 래핑, 에러 처리 필수
- **유틸리티 함수**:
  ```javascript
  // src/services/indexedDBService.js
  export const promisifyRequest = (request) => {
  	return new Promise((resolve, reject) => {
  		request.onsuccess = () => resolve(request.result)
  		request.onerror = () => reject(request.error)
  	})
  }
  ```

### 중복 북마크 방지
- **문제**: 동일 URL을 여러 번 추가하면 중복 북마크 생성
- **대응**: `url` 인덱스를 UNIQUE로 설정, addBookmark() 시 중복 체크
- **에러 처리**: 중복 시 "이미 북마크에 추가된 페이지입니다" 메시지 표시

### 폴더 삭제 시 북마크 처리
- **문제**: 폴더 삭제 시 하위 북마크 처리 방법
- **대응**: 폴더 삭제 시 하위 북마크도 함께 삭제 (경고 메시지 표시)
- **대안**: 향후 확장 시 "루트 폴더로 이동" 옵션 제공 가능

### VirtualList 성능 최적화
- **문제**: 북마크 수 많을 때 (100개 이상) 렌더링 성능 저하 가능
- **대응**: Enact VirtualList 사용 (가상 스크롤)으로 DOM 노드 수 최소화
- **측정**: 북마크 1000개 테스트 시 스크롤 프레임레이트 30fps 이상 유지 확인

### 북마크 데이터 마이그레이션 (향후)
- **문제**: IndexedDB 스키마 변경 시 기존 데이터 마이그레이션 필요
- **대응**: `onupgradeneeded` 이벤트에서 버전별 마이그레이션 로직 추가
- **예시**: DB_VERSION을 2로 증가 시 기존 bookmarks에 visitCount 컬럼 추가

### 파비콘 처리 (선택적 기능)
- **현재**: 파비콘 표시 안 함 (이모지 아이콘 사용)
- **향후**: 북마크 추가 시 웹사이트 파비콘 다운로드 및 저장 (Blob)
- **제약**: CORS 제약으로 Same-Origin 사이트만 파비콘 다운로드 가능

## 13. 확장성 고려사항

### F-06 (탭 관리)와의 연동
- **현재**: 북마크 클릭 시 현재 탭에서만 열기
- **F-06 시**: 컨텍스트 메뉴에 "새 탭에서 열기" 옵션 추가
- **구현 방향**: onBookmarkSelect에 `openInNewTab` 파라미터 추가

### F-08 (히스토리 관리)와의 연동
- **현재**: 북마크 실행 시 히스토리 미기록
- **F-08 시**: onBookmarkSelect에서 historyService.addHistory() 호출
- **구현 방향**: BrowserView에서 북마크 실행 시 히스토리 서비스 호출

### F-11 (설정 화면)와의 연동
- **북마크 동기화**: 설정에서 LG 계정 연동 시 북마크 클라우드 동기화
- **북마크 가져오기/내보내기**: HTML 북마크 파일 가져오기/내보내기 기능

### F-15 (즐겨찾기 홈 화면)와의 연동
- **자주 가는 사이트**: visitCount 기반으로 상위 N개 북마크 표시
- **구현 방향**: BookmarkService에 `getTopBookmarks(limit)` 함수 추가

### 북마크 태그 기능 (향후)
- **현재**: 폴더 구조만 지원
- **향후**: 북마크에 태그 추가 기능 (예: #엔터테인먼트, #뉴스)
- **데이터 모델**: bookmarks 테이블에 `tags` 컬럼 추가 (배열)

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-13 | 최초 작성 | F-07 요구사항 기반 기술 설계 |
