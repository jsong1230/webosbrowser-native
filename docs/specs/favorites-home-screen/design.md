# 즐겨찾기 홈 화면 — 기술 설계서

## 1. 참조
- **요구사항 분석서**: `docs/specs/favorites-home-screen/requirements.md`
- **기존 서비스**: `src/services/bookmarkService.js`, `src/services/historyService.js`
- **관련 컴포넌트**: `src/components/NavigationBar/NavigationBar.js`, `src/views/BrowserView.js`

---

## 2. 아키텍처 개요

### 컴포넌트 다이어그램
```
┌─────────────────┐
│   BrowserView   │ (라우팅 로직)
└────────┬────────┘
         │ homeUrl === 'about:home'?
         ├───────────────┐
         │               │
┌────────▼────────┐  ┌──▼──────────────────┐
│   HomePage      │  │   WebView (iframe)  │
│   (신규)        │  │   (기존)            │
└────┬────────────┘  └─────────────────────┘
     │ useEffect(mount)
     │
┌────▼──────────────────────────────┐
│   HomeScreenService (신규)        │
│   - getTiles()                    │
│   - saveTileConfig()              │
│   - addTile(), removeTile()       │
│   - getTopSitesByVisitCount()     │
└────┬──────────────────────────────┘
     │ bookmarkService, historyService
     ├─────────────────┬──────────────
     │                 │
┌────▼──────────┐  ┌──▼────────────┐
│ bookmarkService│  │historyService │
│ (기존)        │  │ (기존)        │
└───────────────┘  └───────────────┘
```

### 데이터 흐름도
```
앱 시작/홈 버튼 클릭
  │
  ▼
BrowserView가 currentUrl 확인
  │ (currentUrl === 'about:home')
  │
  ▼
HomePage 컴포넌트 렌더링
  │
  ├─ useEffect → HomeScreenService.getTiles()
  │                │
  │                ├─ LocalStorage에서 타일 설정 로드
  │                │  (tileIds, tileType: auto/manual)
  │                │
  │                ├─ 자동 모드?
  │                │  └─ HomeScreenService.getTopSitesByVisitCount(6)
  │                │       │
  │                │       ├─ bookmarkService.getAllBookmarks()
  │                │       │  (visitCount 기준 정렬)
  │                │       │
  │                │       ├─ historyService.getAllHistory()
  │                │       │  (visitCount 기준 정렬)
  │                │       │
  │                │       └─ 병합 + 중복 제거 (도메인 기준)
  │                │
  │                └─ 수동 모드?
  │                   └─ tileIds 기반으로 북마크 조회
  │
  └─ tiles 렌더링 (Enact Scroller + 그리드 레이아웃)
     │
     ├─ 각 타일: ImageItem (파비콘 + 사이트 이름)
     │  │ onClick → BrowserView.handleUrlSubmit(tile.url)
     │  │            → TabContext.UPDATE_TAB (activeTabId, url)
     │  │            → WebView 로드
     │
     └─ 편집 모드?
        └─ 타일마다 삭제 버튼 + "사이트 추가" 타일
```

---

## 3. 아키텍처 결정

### 결정 1: 홈 화면 URL 형식
- **선택지**:
  - A) `about:home` (Chrome 스타일 특수 URL)
  - B) 컴포넌트 라우팅 (React Router 또는 state 기반)
  - C) 홈페이지 URL 우선 (F-11 설정과 통합)
- **결정**: **A) `about:home`**
- **근거**:
  - 브라우저 표준 관행 (Chrome, Firefox 모두 `about:` 프로토콜 사용)
  - WebView와 명확히 구분 가능 (URL 충돌 없음)
  - BrowserView에서 `currentUrl === 'about:home'` 조건으로 HomePage 렌더링
  - 홈페이지 URL(F-11)과 홈 화면(F-15)은 별개 개념
    - 홈페이지 URL: 설정에서 지정한 시작 페이지 (예: 구글, 네이버)
    - 홈 화면: 자주 가는 사이트 타일 그리드
- **트레이드오프**:
  - 포기: React Router 도입 (프로젝트 복잡도 증가 방지)
  - 얻음: 단순한 조건부 렌더링으로 구현 가능, 브라우저 히스토리 오염 방지

### 결정 2: 타일 데이터 저장소
- **선택지**:
  - A) LocalStorage (JSON)
  - B) IndexedDB (bookmarks/history와 동일)
  - C) 메모리 전용 (휘발성)
- **결정**: **A) LocalStorage**
- **근거**:
  - 타일 설정은 메타데이터만 저장 (tileIds 배열, tileType)
  - 실제 북마크/히스토리 데이터는 IndexedDB에서 조회
  - LocalStorage는 설정 데이터(F-11)와 동일한 저장소 사용
  - 앱 재시작 시에도 유지되어야 함 (영속성 요구)
- **트레이드오프**:
  - 포기: IndexedDB의 트랜잭션 일관성 (타일 설정은 단순 키-밸류)
  - 얻음: 빠른 로드 속도, 설정 데이터와 일관된 저장소

### 결정 3: 타일 자동 선정 알고리즘
- **선택지**:
  - A) 북마크 우선 + 히스토리 보조
  - B) 북마크/히스토리 통합 visitCount 정렬
  - C) 최근 방문 시각(visitedAt) 우선
- **결정**: **A) 북마크 우선 + 히스토리 보조**
- **근거**:
  - 북마크는 사용자가 명시적으로 저장한 사이트 → 의도가 명확
  - 히스토리는 실수로 방문한 사이트도 포함 → 신뢰도 낮음
  - 북마크 visitCount로 정렬 → 상위 N개 선정
  - 부족하면 히스토리 visitCount로 보충
  - 도메인 중복 제거 (예: `youtube.com/watch?v=A`, `youtube.com/watch?v=B` → `youtube.com`)
- **트레이드오프**:
  - 포기: 북마크 없는 신규 사용자는 히스토리만 표시
  - 얻음: 사용자 의도 반영, 노이즈 감소

### 결정 4: 그리드 레이아웃 구현
- **선택지**:
  - A) Enact GridListImageItem (VirtualList 기반)
  - B) CSS Grid + 직접 구현
  - C) Flexbox + 커스텀 컴포넌트
- **결정**: **B) CSS Grid + Enact ImageItem**
- **근거**:
  - GridListImageItem은 VirtualList 기반 → 대용량 데이터용 (타일 6~9개는 소량)
  - CSS Grid는 고정 레이아웃(3x2)에 최적화
  - Enact ImageItem은 파비콘 + 제목 표시에 적합
  - Spotlight 통합 용이 (각 ImageItem에 spotlightId 지정)
- **트레이드오프**:
  - 포기: VirtualList의 메모리 최적화 (불필요, 타일 개수 적음)
  - 얻음: 단순한 레이아웃 로직, 빠른 렌더링

### 결정 5: 파비콘 소스
- **선택지**:
  - A) 북마크/히스토리에 저장된 favicon 필드 사용
  - B) Google Favicon API (`https://www.google.com/s2/favicons?domain=example.com`)
  - C) 직접 스크린샷 캐싱 (F-02 WebView 통합)
- **결정**: **A) 북마크/히스토리 favicon 필드 우선 + B) Google Favicon API 폴백**
- **근거**:
  - 북마크/히스토리 서비스에서 이미 favicon 필드 제공
  - 없으면 Google Favicon API로 실시간 조회 (네트워크 요청 필요)
  - 스크린샷 캐싱은 복잡도 높음 (Phase 4 확장 기능)
- **트레이드오프**:
  - 포기: 고화질 썸네일 (Phase 4 확장)
  - 얻음: 빠른 구현, 안정적 파비콘 표시

### 결정 6: 편집 모드 구현 범위
- **선택지**:
  - A) 편집 모드 전체 구현 (삭제 + 추가 + 순서 변경)
  - B) 편집 모드 최소 구현 (삭제 + 추가만)
  - C) 편집 모드 Phase 2 구현 (자동 모드만)
- **결정**: **C) Phase 1에서는 자동 모드만 구현, Phase 2에서 편집 모드 추가**
- **근거**:
  - 요구사항에서 편집 모드는 "Should" 우선순위 (선택적)
  - 자동 모드가 핵심 기능 (Must)
  - 편집 모드는 UI 복잡도 높음 (리모컨 순서 변경 로직)
- **트레이드오프**:
  - 포기: 첫 릴리스에서 타일 수동 편집 불가
  - 얻음: 빠른 MVP 출시, 안정적 자동 모드

---

## 4. HomePage 컴포넌트 설계

### Props 인터페이스
```javascript
HomePage.propTypes = {
	// 타일 클릭 시 URL 로드 콜백
	onNavigate: PropTypes.func.isRequired,  // (url: string) => void

	// 홈 화면 종료 콜백 (선택적, 편집 모드 취소용)
	onClose: PropTypes.func,                // () => void

	// 스타일 커스터마이징
	className: PropTypes.string
}
```

### State 관리
```javascript
const [tiles, setTiles] = useState([])           // 타일 배열 (최대 9개)
const [isLoading, setIsLoading] = useState(true) // 타일 로딩 상태
const [isError, setIsError] = useState(false)    // 로드 실패 상태
const [isEditing, setIsEditing] = useState(false)// 편집 모드 (Phase 2)
```

### 라이프사이클
```javascript
useEffect(() => {
	// 마운트 시 타일 로드
	const loadTiles = async () => {
		try {
			setIsLoading(true)
			const tiles = await HomeScreenService.getTiles()
			setTiles(tiles)
			setIsError(false)
		} catch (error) {
			logger.error('[HomePage] 타일 로드 실패:', error)
			setIsError(true)
		} finally {
			setIsLoading(false)
		}
	}

	loadTiles()
}, [])

// 북마크/히스토리 변경 시 타일 갱신 (이벤트 리스너)
useEffect(() => {
	const handleDataChange = () => {
		loadTiles()  // 타일 재로드
	}

	window.addEventListener('bookmarkschanged', handleDataChange)
	window.addEventListener('historychanged', handleDataChange)

	return () => {
		window.removeEventListener('bookmarkschanged', handleDataChange)
		window.removeEventListener('historychanged', handleDataChange)
	}
}, [])
```

### Spotlight 설정 (그리드 네비게이션)
```javascript
useEffect(() => {
	// HomePage 컨테이너 Spotlight 설정
	Spotlight.set('home-page', {
		defaultElement: '[data-tile-index="0"]',  // 첫 번째 타일 포커스
		enterTo: 'default-element',
		restrict: 'self-only'  // 홈 화면 내부에서만 포커스 이동
	})

	// 초기 포커스
	Spotlight.focus('home-page')
}, [])
```

### 타일 클릭 핸들러
```javascript
const handleTileClick = useCallback((tile) => {
	logger.info('[HomePage] 타일 클릭:', tile.url)

	// 북마크인 경우 visitCount 증가
	if (tile.source === 'bookmark') {
		bookmarkService.incrementVisitCount(tile.id)
	}

	// BrowserView에 URL 전달
	onNavigate(tile.url)
}, [onNavigate])
```

### 타일 데이터 구조
```javascript
// HomeScreenService.getTiles() 반환 타입
{
	id: "uuid",                  // 북마크/히스토리 ID
	url: "https://youtube.com",  // 사이트 URL
	title: "YouTube",            // 사이트 이름
	favicon: "https://...",      // 파비콘 URL (null이면 기본 아이콘)
	visitCount: 42,              // 방문 횟수 (정렬 기준)
	source: "bookmark" | "history", // 출처
	domain: "youtube.com"        // 도메인 (중복 제거용)
}
```

---

## 5. HomeScreenService 설계

### API 목록

#### 1. `getTiles(count = 6)`
- **목적**: 현재 타일 설정을 로드하여 표시할 타일 배열 반환
- **파라미터**: `count` (number) — 표시할 타일 개수 (기본값: 6)
- **반환**: `Promise<Tile[]>` — 타일 배열
- **로직**:
  ```javascript
  export const getTiles = async (count = 6) => {
    // 1. LocalStorage에서 타일 설정 로드
    const config = loadTileConfig()

    // 2. 자동 모드 or 수동 모드 확인
    if (config.tileType === 'auto') {
      // 북마크/히스토리 기반 자동 선정
      return await getTopSitesByVisitCount(count)
    } else {
      // tileIds 기반으로 북마크 조회
      return await getTilesByIds(config.tileIds)
    }
  }
  ```

#### 2. `getTopSitesByVisitCount(count)`
- **목적**: 북마크/히스토리에서 visitCount 높은 상위 N개 사이트 선정
- **파라미터**: `count` (number) — 선정할 사이트 개수
- **반환**: `Promise<Tile[]>` — 타일 배열
- **로직**:
  ```javascript
  export const getTopSitesByVisitCount = async (count) => {
    // 1. 북마크 조회 (visitCount 역순 정렬)
    const bookmarks = await bookmarkService.getAllBookmarks()
    const sortedBookmarks = bookmarks
      .sort((a, b) => (b.visitCount || 0) - (a.visitCount || 0))
      .map(bm => ({
        id: bm.id,
        url: bm.url,
        title: bm.title,
        favicon: bm.favicon,
        visitCount: bm.visitCount || 0,
        source: 'bookmark',
        domain: extractDomain(bm.url)
      }))

    // 2. 북마크로 count 충족?
    if (sortedBookmarks.length >= count) {
      return sortedBookmarks.slice(0, count)
    }

    // 3. 히스토리에서 추가 선정
    const history = await historyService.getAllHistory()
    const sortedHistory = history
      .sort((a, b) => (b.visitCount || 0) - (a.visitCount || 0))
      .map(h => ({
        id: h.id,
        url: h.url,
        title: h.title,
        favicon: h.favicon,
        visitCount: h.visitCount || 0,
        source: 'history',
        domain: extractDomain(h.url)
      }))

    // 4. 병합 + 도메인 중복 제거
    const combined = [...sortedBookmarks, ...sortedHistory]
    const deduplicated = deduplicateByDomain(combined)

    // 5. 상위 count개 반환
    return deduplicated.slice(0, count)
  }
  ```

#### 3. `deduplicateByDomain(tiles)`
- **목적**: 동일 도메인의 타일 중복 제거 (visitCount 높은 것 우선)
- **파라미터**: `tiles` (Tile[]) — 타일 배열
- **반환**: `Tile[]` — 중복 제거된 타일 배열
- **로직**:
  ```javascript
  const deduplicateByDomain = (tiles) => {
    const domainMap = new Map()

    for (const tile of tiles) {
      if (!domainMap.has(tile.domain)) {
        domainMap.set(tile.domain, tile)
      } else {
        // 이미 존재하면 visitCount 비교
        const existing = domainMap.get(tile.domain)
        if (tile.visitCount > existing.visitCount) {
          domainMap.set(tile.domain, tile)
        }
      }
    }

    return Array.from(domainMap.values())
  }
  ```

#### 4. `loadTileConfig()`
- **목적**: LocalStorage에서 타일 설정 로드
- **반환**: `TileConfig`
- **로직**:
  ```javascript
  const TILE_CONFIG_KEY = 'homeScreenTileConfig'

  export const loadTileConfig = () => {
    try {
      const json = localStorage.getItem(TILE_CONFIG_KEY)
      if (json) {
        return JSON.parse(json)
      }
    } catch (error) {
      logger.error('[HomeScreenService] 타일 설정 로드 실패:', error)
    }

    // 기본 설정 반환
    return {
      tileIds: [],          // 수동 모드 타일 ID 배열
      tileType: 'auto',     // 'auto' | 'manual'
      tileCount: 6          // 표시할 타일 개수
    }
  }
  ```

#### 5. `saveTileConfig(config)`
- **목적**: 타일 설정을 LocalStorage에 저장
- **파라미터**: `config` (TileConfig)
- **반환**: `boolean` — 성공 여부
- **로직**:
  ```javascript
  export const saveTileConfig = (config) => {
    try {
      const json = JSON.stringify(config)
      localStorage.setItem(TILE_CONFIG_KEY, json)
      logger.info('[HomeScreenService] 타일 설정 저장 완료:', config)
      return true
    } catch (error) {
      logger.error('[HomeScreenService] 타일 설정 저장 실패:', error)
      return false
    }
  }
  ```

#### 6. `addTile(bookmarkId)` (Phase 2)
- **목적**: 수동 모드에서 타일 추가
- **파라미터**: `bookmarkId` (string) — 북마크 ID
- **반환**: `Promise<boolean>` — 성공 여부
- **로직**:
  ```javascript
  export const addTile = async (bookmarkId) => {
    const config = loadTileConfig()

    // 이미 추가된 타일인지 확인
    if (config.tileIds.includes(bookmarkId)) {
      logger.warn('[HomeScreenService] 이미 추가된 타일:', bookmarkId)
      return false
    }

    // 최대 개수 체크
    if (config.tileIds.length >= 9) {
      logger.warn('[HomeScreenService] 최대 타일 개수 초과 (9개)')
      return false
    }

    // 타일 추가
    config.tileIds.push(bookmarkId)
    config.tileType = 'manual'  // 수동 모드로 전환

    return saveTileConfig(config)
  }
  ```

#### 7. `removeTile(tileId)` (Phase 2)
- **목적**: 수동 모드에서 타일 삭제
- **파라미터**: `tileId` (string) — 타일 ID
- **반환**: `Promise<boolean>` — 성공 여부
- **로직**:
  ```javascript
  export const removeTile = async (tileId) => {
    const config = loadTileConfig()

    // tileIds에서 제거
    config.tileIds = config.tileIds.filter(id => id !== tileId)

    return saveTileConfig(config)
  }
  ```

#### 8. `extractDomain(url)`
- **목적**: URL에서 도메인 추출 (중복 제거용)
- **파라미터**: `url` (string) — URL
- **반환**: `string` — 도메인 (예: `youtube.com`)
- **로직**:
  ```javascript
  const extractDomain = (url) => {
    try {
      const parsed = new URL(url)
      return parsed.hostname.replace(/^www\./, '')  // www 제거
    } catch {
      return url  // 파싱 실패 시 원본 반환
    }
  }
  ```

---

## 6. UI/UX 설계

### 그리드 레이아웃
```less
// HomePage.module.less
.homePage {
	width: 100%;
	height: 100%;
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	background: var(--bg-color);  // 다크/라이트 테마 대응
	padding: 60px;
}

.tileGrid {
	display: grid;
	grid-template-columns: repeat(3, 300px);  // 3열 (고정 너비)
	grid-template-rows: repeat(2, 300px);     // 2행 (고정 높이)
	gap: 40px;                                // 타일 간격
	justify-content: center;
	align-content: center;
}

.tile {
	width: 300px;
	height: 300px;
	border-radius: 12px;
	background: var(--tile-bg);
	border: 3px solid transparent;
	cursor: pointer;
	transition: all 0.3s ease;

	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;

	&:hover,
	&:global(.spottable:focus) {
		border-color: var(--focus-color);  // 포커스 테두리
		transform: scale(1.05);             // 확대 효과
		box-shadow: 0 8px 24px rgba(0, 0, 0, 0.3);
	}
}

.tileFavicon {
	width: 96px;
	height: 96px;
	margin-bottom: 24px;
	border-radius: 50%;
	background: var(--favicon-bg);

	img {
		width: 100%;
		height: 100%;
		object-fit: contain;
	}
}

.tileTitle {
	font-size: 28px;
	font-weight: 600;
	color: var(--text-color);
	text-align: center;
	max-width: 280px;
	overflow: hidden;
	text-overflow: ellipsis;
	white-space: nowrap;
}

.tileUrl {
	font-size: 18px;
	color: var(--text-secondary);
	margin-top: 8px;
	text-align: center;
	max-width: 280px;
	overflow: hidden;
	text-overflow: ellipsis;
	white-space: nowrap;
}
```

### 타일 디자인 (Enact 통합)
```javascript
// HomePage.js 타일 렌더링
const renderTile = (tile, index) => {
	return (
		<div
			key={tile.id}
			className={css.tile}
			onClick={() => handleTileClick(tile)}
			data-tile-index={index}
			data-spotlight-id={`tile-${index}`}
			tabIndex={-1}
		>
			{/* 파비콘 */}
			<div className={css.tileFavicon}>
				{tile.favicon ? (
					<img
						src={tile.favicon}
						alt={tile.title}
						onError={(e) => {
							// 파비콘 로드 실패 시 기본 아이콘 사용
							e.target.src = getDefaultFavicon(tile.domain)
						}}
					/>
				) : (
					<img
						src={getDefaultFavicon(tile.domain)}
						alt="Default favicon"
					/>
				)}
			</div>

			{/* 사이트 이름 */}
			<div className={css.tileTitle}>{tile.title}</div>

			{/* URL (선택적) */}
			<div className={css.tileUrl}>{tile.domain}</div>
		</div>
	)
}

// 기본 파비콘 생성 (Google Favicon API 폴백)
const getDefaultFavicon = (domain) => {
	return `https://www.google.com/s2/favicons?domain=${domain}&sz=128`
}
```

### 빈 홈 화면 UI
```javascript
// HomePage.js 빈 상태 렌더링
{tiles.length === 0 && !isLoading && (
	<div className={css.emptyState}>
		<div className={css.emptyIcon}>🏠</div>
		<h2 className={css.emptyTitle}>자주 가는 사이트가 없습니다</h2>
		<p className={css.emptyMessage}>
			북마크를 추가하거나 웹 사이트를 방문해보세요
		</p>
		<div className={css.emptyActions}>
			<Button
				className={css.emptyButton}
				onClick={handleGoToBookmarks}
				icon="star"
			>
				북마크 관리
			</Button>
			<Button
				className={css.emptyButton}
				onClick={handleGoToUrl}
				icon="arrowlargeright"
			>
				URL 입력
			</Button>
		</div>
	</div>
)}
```

### 편집 모드 UI (Phase 2)
```javascript
// 편집 모드 활성화 시
{isEditing && (
	<>
		{/* 편집 헤더 */}
		<div className={css.editHeader}>
			<h2>타일 편집</h2>
			<Button onClick={handleExitEditMode}>완료</Button>
		</div>

		{/* 타일마다 삭제 버튼 */}
		<div className={css.tileDeleteButton} onClick={() => handleRemoveTile(tile.id)}>
			✕
		</div>

		{/* 사이트 추가 타일 */}
		<div className={css.addTile} onClick={handleAddTile}>
			<div className={css.addIcon}>+</div>
			<div className={css.addTitle}>사이트 추가</div>
		</div>
	</>
)}
```

---

## 7. BrowserView 통합 설계

### HomePage 라우팅 로직
```javascript
// BrowserView.js 수정
const BrowserView = () => {
	const { state, dispatch } = useTabContext()
	const activeTab = tabManager.getActiveTab(state.tabs, state.activeTabId)
	const currentUrl = activeTab?.url || 'https://www.google.com'

	// 홈 화면 표시 조건
	const isHomePage = currentUrl === 'about:home'

	return (
		<Panel className={css.browserView}>
			<Header title="webOS Browser" />

			{/* URLBar */}
			<URLBar value={inputValue} onChange={handleUrlChange} onSubmit={handleUrlSubmit} />

			{/* TabBar */}
			<TabBar tabs={state.tabs} activeTabId={state.activeTabId} ... />

			{/* LoadingBar */}
			<LoadingBar isLoading={isLoading} isError={isError} />

			{/* 조건부 렌더링: HomePage or WebView */}
			<div className={css.webviewWrapper}>
				{isHomePage ? (
					<HomePage onNavigate={handleHomePageNavigate} />
				) : (
					<WebView url={currentUrl} ... />
				)}
			</div>

			{/* NavigationBar */}
			<NavigationBar ... />

			{/* ... 기타 오버레이 패널들 ... */}
		</Panel>
	)
}

// 홈 화면 내비게이션 핸들러
const handleHomePageNavigate = useCallback((url) => {
	logger.info('[BrowserView] 홈 화면에서 사이트 열기:', url)

	// 활성 탭의 URL 업데이트
	dispatch({
		type: TAB_ACTIONS.UPDATE_TAB,
		payload: {
			id: state.activeTabId,
			updates: { url }
		}
	})

	// URL 입력창도 업데이트
	setInputValue(url)

	// WebView로 포커스 이동
	Spotlight.focus('webview-main')
}, [state.activeTabId, dispatch])
```

### NavigationBar 홈 버튼 수정
```javascript
// NavigationBar.js 홈 버튼 핸들러
const handleHome = useCallback(() => {
	if (isNavigating) return

	setIsNavigating(true)

	// BrowserView에 홈 화면 URL 전달
	if (onNavigate) {
		onNavigate({ action: 'home', url: 'about:home' })
	}

	logger.info('[NavigationBar] 홈 화면으로 이동')

	setTimeout(() => setIsNavigating(false), 500)
}, [isNavigating, onNavigate])
```

### 앱 초기 URL 설정
```javascript
// App.js 또는 TabContext 초기 상태 수정
const createInitialTab = () => {
	// F-11 설정에서 홈페이지 URL 로드
	const settings = settingsService.loadSettings()
	const initialUrl = settings.homepageUrl || 'about:home'  // 기본값: 홈 화면

	return {
		id: generateUUID(),
		url: initialUrl,
		title: initialUrl === 'about:home' ? '홈' : 'New Tab',
		favicon: null,
		historyStack: [initialUrl],
		historyIndex: 0,
		isLoading: false,
		isError: false,
		createdAt: Date.now(),
		lastAccessedAt: Date.now()
	}
}
```

---

## 8. 상태 관리 전략

### 타일 데이터 로딩
```javascript
// HomePage.js
const [tiles, setTiles] = useState([])
const [isLoading, setIsLoading] = useState(true)
const [isError, setIsError] = useState(false)

useEffect(() => {
	const loadTiles = async () => {
		try {
			setIsLoading(true)

			// HomeScreenService에서 타일 로드
			const loadedTiles = await HomeScreenService.getTiles(6)

			setTiles(loadedTiles)
			setIsError(false)
		} catch (error) {
			logger.error('[HomePage] 타일 로드 실패:', error)
			setIsError(true)
			setTiles([])  // 빈 배열로 초기화
		} finally {
			setIsLoading(false)
		}
	}

	loadTiles()
}, [])
```

### 북마크/히스토리 변경 감지
```javascript
// 북마크 추가/삭제 시 이벤트 발생
// bookmarkService.js
export const addBookmark = async (bookmark) => {
	// ... 북마크 추가 로직 ...

	// 이벤트 발생
	window.dispatchEvent(new CustomEvent('bookmarkschanged'))

	return newBookmark
}

export const deleteBookmark = async (id) => {
	// ... 북마크 삭제 로직 ...

	// 이벤트 발생
	window.dispatchEvent(new CustomEvent('bookmarkschanged'))
}

// HomePage.js에서 이벤트 수신
useEffect(() => {
	const handleBookmarksChanged = () => {
		logger.debug('[HomePage] 북마크 변경 감지 → 타일 재로드')
		loadTiles()
	}

	const handleHistoryChanged = () => {
		logger.debug('[HomePage] 히스토리 변경 감지 → 타일 재로드')
		loadTiles()
	}

	window.addEventListener('bookmarkschanged', handleBookmarksChanged)
	window.addEventListener('historychanged', handleHistoryChanged)

	return () => {
		window.removeEventListener('bookmarkschanged', handleBookmarksChanged)
		window.removeEventListener('historychanged', handleHistoryChanged)
	}
}, [])
```

### 편집 모드 상태 토글 (Phase 2)
```javascript
const [isEditing, setIsEditing] = useState(false)

const handleEnterEditMode = () => {
	setIsEditing(true)
	logger.info('[HomePage] 편집 모드 진입')
}

const handleExitEditMode = () => {
	setIsEditing(false)
	logger.info('[HomePage] 편집 모드 종료')
}
```

---

## 9. 에러 처리

### LocalStorage 실패 처리
```javascript
// HomeScreenService.js
export const loadTileConfig = () => {
	try {
		const json = localStorage.getItem(TILE_CONFIG_KEY)
		if (json) {
			return JSON.parse(json)
		}
	} catch (error) {
		logger.error('[HomeScreenService] 타일 설정 로드 실패:', error)
		// LocalStorage 접근 불가 시 기본 설정 반환
	}

	return {
		tileIds: [],
		tileType: 'auto',
		tileCount: 6
	}
}
```

### 파비콘 로드 실패 처리
```javascript
// HomePage.js
const handleFaviconError = (event, tile) => {
	logger.warn('[HomePage] 파비콘 로드 실패:', tile.url)

	// Google Favicon API 폴백
	event.target.src = `https://www.google.com/s2/favicons?domain=${tile.domain}&sz=128`

	// 폴백도 실패 시 기본 아이콘
	event.target.onerror = () => {
		event.target.src = '/resources/icons/default-favicon.png'
	}
}
```

### 북마크/히스토리 없을 때 처리
```javascript
// HomePage.js
useEffect(() => {
	const loadTiles = async () => {
		try {
			setIsLoading(true)
			const loadedTiles = await HomeScreenService.getTiles(6)

			if (loadedTiles.length === 0) {
				logger.info('[HomePage] 타일 없음 → 빈 상태 표시')
			}

			setTiles(loadedTiles)
			setIsError(false)
		} catch (error) {
			logger.error('[HomePage] 타일 로드 실패:', error)
			setIsError(true)
		} finally {
			setIsLoading(false)
		}
	}

	loadTiles()
}, [])

// JSX 빈 상태 렌더링
{tiles.length === 0 && !isLoading && (
	<EmptyState onGoToBookmarks={handleGoToBookmarks} />
)}
```

### 네트워크 요청 실패 (Google Favicon API)
```javascript
// 파비콘 API 실패 시 로컬 기본 아이콘 사용
const getDefaultFaviconFallback = () => {
	return '/resources/icons/default-favicon.png'
}
```

---

## 10. 성능 최적화

### 타일 데이터 캐싱
```javascript
// HomePage.js
const tilesRef = useRef(null)  // 타일 캐싱

useEffect(() => {
	const loadTiles = async () => {
		// 이미 로드된 타일이 있으면 캐시 사용
		if (tilesRef.current) {
			setTiles(tilesRef.current)
			return
		}

		try {
			setIsLoading(true)
			const loadedTiles = await HomeScreenService.getTiles(6)

			// 캐시 저장
			tilesRef.current = loadedTiles
			setTiles(loadedTiles)
		} catch (error) {
			logger.error('[HomePage] 타일 로드 실패:', error)
			setIsError(true)
		} finally {
			setIsLoading(false)
		}
	}

	loadTiles()
}, [])

// 북마크/히스토리 변경 시 캐시 무효화
const handleBookmarksChanged = () => {
	tilesRef.current = null  // 캐시 클리어
	loadTiles()
}
```

### 파비콘 Lazy Loading
```javascript
// HomePage.js
const [loadedFavicons, setLoadedFavicons] = useState(new Set())

const handleFaviconLoad = (tileId) => {
	setLoadedFavicons(prev => new Set(prev).add(tileId))
}

// 파비콘 로딩 중 플레이스홀더 표시
{!loadedFavicons.has(tile.id) && (
	<div className={css.faviconPlaceholder}>
		{/* 로딩 스피너 또는 기본 아이콘 */}
	</div>
)}
```

### 대용량 북마크/히스토리 처리
```javascript
// HomeScreenService.js
export const getTopSitesByVisitCount = async (count) => {
	// 1. 북마크 조회 (상위 50개만)
	const allBookmarks = await bookmarkService.getAllBookmarks()
	const topBookmarks = allBookmarks
		.sort((a, b) => (b.visitCount || 0) - (a.visitCount || 0))
		.slice(0, 50)  // 최대 50개까지만 처리 (성능 최적화)

	// 2. 히스토리 조회 (상위 50개만)
	const allHistory = await historyService.getAllHistory()
	const topHistory = allHistory
		.sort((a, b) => (b.visitCount || 0) - (a.visitCount || 0))
		.slice(0, 50)

	// 3. 병합 + 중복 제거
	const combined = [
		...topBookmarks.map(bm => ({
			id: bm.id,
			url: bm.url,
			title: bm.title,
			favicon: bm.favicon,
			visitCount: bm.visitCount || 0,
			source: 'bookmark',
			domain: extractDomain(bm.url)
		})),
		...topHistory.map(h => ({
			id: h.id,
			url: h.url,
			title: h.title,
			favicon: h.favicon,
			visitCount: h.visitCount || 0,
			source: 'history',
			domain: extractDomain(h.url)
		}))
	]

	const deduplicated = deduplicateByDomain(combined)

	return deduplicated.slice(0, count)
}
```

### React.memo로 불필요한 리렌더링 방지
```javascript
// HomePage.js
const TileItem = React.memo(({ tile, onClick }) => {
	return (
		<div className={css.tile} onClick={onClick}>
			<div className={css.tileFavicon}>
				<img src={tile.favicon || getDefaultFavicon(tile.domain)} alt={tile.title} />
			</div>
			<div className={css.tileTitle}>{tile.title}</div>
			<div className={css.tileUrl}>{tile.domain}</div>
		</div>
	)
})

// 타일 렌더링
{tiles.map((tile, index) => (
	<TileItem
		key={tile.id}
		tile={tile}
		onClick={() => handleTileClick(tile)}
	/>
))}
```

---

## 11. 테스트 전략

### 단위 테스트 (HomeScreenService)
```javascript
// __tests__/homeScreenService.test.js
describe('HomeScreenService', () => {
	test('getTiles는 기본 6개 타일 반환', async () => {
		const tiles = await HomeScreenService.getTiles()
		expect(tiles).toHaveLength(6)
	})

	test('getTopSitesByVisitCount는 visitCount 역순 정렬', async () => {
		const tiles = await HomeScreenService.getTopSitesByVisitCount(3)
		expect(tiles[0].visitCount).toBeGreaterThanOrEqual(tiles[1].visitCount)
	})

	test('deduplicateByDomain는 동일 도메인 제거', () => {
		const tiles = [
			{ domain: 'youtube.com', visitCount: 10 },
			{ domain: 'youtube.com', visitCount: 5 },
			{ domain: 'google.com', visitCount: 8 }
		]

		const result = HomeScreenService.deduplicateByDomain(tiles)

		expect(result).toHaveLength(2)
		expect(result[0].domain).toBe('youtube.com')
		expect(result[0].visitCount).toBe(10)  // 높은 visitCount 우선
	})

	test('loadTileConfig는 LocalStorage 실패 시 기본값 반환', () => {
		// LocalStorage 모킹 (에러 발생)
		jest.spyOn(localStorage, 'getItem').mockImplementation(() => {
			throw new Error('Storage error')
		})

		const config = HomeScreenService.loadTileConfig()

		expect(config.tileType).toBe('auto')
		expect(config.tileCount).toBe(6)
	})
})
```

### 컴포넌트 테스트 (HomePage)
```javascript
// __tests__/HomePage.test.js
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import HomePage from '../HomePage'
import * as HomeScreenService from '../../../services/homeScreenService'

describe('HomePage', () => {
	test('타일 로딩 중 로딩 인디케이터 표시', () => {
		jest.spyOn(HomeScreenService, 'getTiles').mockImplementation(() => new Promise(() => {}))

		render(<HomePage onNavigate={jest.fn()} />)

		expect(screen.getByText(/로딩 중/i)).toBeInTheDocument()
	})

	test('타일 로드 성공 시 타일 그리드 렌더링', async () => {
		const mockTiles = [
			{ id: '1', url: 'https://youtube.com', title: 'YouTube', domain: 'youtube.com', visitCount: 10 },
			{ id: '2', url: 'https://google.com', title: 'Google', domain: 'google.com', visitCount: 8 }
		]

		jest.spyOn(HomeScreenService, 'getTiles').mockResolvedValue(mockTiles)

		render(<HomePage onNavigate={jest.fn()} />)

		await waitFor(() => {
			expect(screen.getByText('YouTube')).toBeInTheDocument()
			expect(screen.getByText('Google')).toBeInTheDocument()
		})
	})

	test('타일 클릭 시 onNavigate 콜백 호출', async () => {
		const mockTiles = [
			{ id: '1', url: 'https://youtube.com', title: 'YouTube', domain: 'youtube.com', visitCount: 10 }
		]

		jest.spyOn(HomeScreenService, 'getTiles').mockResolvedValue(mockTiles)

		const onNavigate = jest.fn()
		render(<HomePage onNavigate={onNavigate} />)

		await waitFor(() => {
			const tile = screen.getByText('YouTube')
			fireEvent.click(tile.closest('.tile'))
		})

		expect(onNavigate).toHaveBeenCalledWith('https://youtube.com')
	})

	test('타일 없을 때 빈 상태 메시지 표시', async () => {
		jest.spyOn(HomeScreenService, 'getTiles').mockResolvedValue([])

		render(<HomePage onNavigate={jest.fn()} />)

		await waitFor(() => {
			expect(screen.getByText(/자주 가는 사이트가 없습니다/i)).toBeInTheDocument()
		})
	})
})
```

### 통합 테스트 (BrowserView + HomePage)
```javascript
// __tests__/BrowserView-HomePage.integration.test.js
describe('BrowserView + HomePage 통합', () => {
	test('홈 버튼 클릭 시 HomePage 렌더링', async () => {
		const { getByText } = render(<BrowserView />)

		// NavigationBar 홈 버튼 클릭
		const homeButton = getByText('홈')
		fireEvent.click(homeButton)

		// HomePage 렌더링 확인
		await waitFor(() => {
			expect(screen.getByTestId('home-page')).toBeInTheDocument()
		})
	})

	test('HomePage 타일 클릭 시 WebView로 전환', async () => {
		const mockTiles = [
			{ id: '1', url: 'https://youtube.com', title: 'YouTube', domain: 'youtube.com', visitCount: 10 }
		]

		jest.spyOn(HomeScreenService, 'getTiles').mockResolvedValue(mockTiles)

		const { getByText } = render(<BrowserView />)

		// HomePage로 이동
		const homeButton = getByText('홈')
		fireEvent.click(homeButton)

		// 타일 클릭
		await waitFor(() => {
			const tile = screen.getByText('YouTube')
			fireEvent.click(tile.closest('.tile'))
		})

		// WebView로 전환 확인
		await waitFor(() => {
			expect(screen.getByTestId('webview-main')).toBeInTheDocument()
		})
	})
})
```

---

## 12. 마이그레이션 및 호환성

### 기존 북마크/히스토리 데이터 활용
- **북마크/히스토리 스키마 변경 불필요**
  - 기존 `visitCount` 필드 활용 (F-07, F-08에서 이미 구현됨)
  - HomePage는 읽기 전용으로 북마크/히스토리 데이터 조회
  - 타일 클릭 시 `bookmarkService.incrementVisitCount()` 호출로 방문 횟수 증가

### LocalStorage 스키마 버전 관리
```javascript
// HomeScreenService.js
const TILE_CONFIG_VERSION = 1

export const loadTileConfig = () => {
	try {
		const json = localStorage.getItem(TILE_CONFIG_KEY)
		if (json) {
			const config = JSON.parse(json)

			// 버전 체크
			if (!config.version || config.version < TILE_CONFIG_VERSION) {
				logger.warn('[HomeScreenService] 타일 설정 버전 불일치 → 마이그레이션')
				return migrateConfig(config)
			}

			return config
		}
	} catch (error) {
		logger.error('[HomeScreenService] 타일 설정 로드 실패:', error)
	}

	return getDefaultConfig()
}

const migrateConfig = (oldConfig) => {
	// 버전 1: tileIds, tileType, tileCount 추가
	const newConfig = {
		version: TILE_CONFIG_VERSION,
		tileIds: oldConfig.tileIds || [],
		tileType: oldConfig.tileType || 'auto',
		tileCount: oldConfig.tileCount || 6
	}

	// 새 버전으로 저장
	saveTileConfig(newConfig)

	return newConfig
}

const getDefaultConfig = () => {
	return {
		version: TILE_CONFIG_VERSION,
		tileIds: [],
		tileType: 'auto',
		tileCount: 6
	}
}
```

### 기존 사용자 초기 경험
- **앱 업데이트 후 첫 실행**:
  1. 기존 북마크가 있으면 → 자동으로 상위 6개 타일 표시
  2. 북마크가 없으면 → 히스토리 기반 타일 표시
  3. 둘 다 없으면 → 빈 홈 화면 안내 메시지 표시

---

## 13. 파일 구조

### 신규 파일
```
src/
├── components/
│   └── HomePage/
│       ├── HomePage.js           # 홈 화면 컴포넌트
│       ├── HomePage.module.less  # 홈 화면 스타일
│       ├── TileItem.js           # 개별 타일 컴포넌트 (선택적)
│       └── index.js              # export
│
├── services/
│   └── homeScreenService.js      # 홈 화면 타일 데이터 관리
│
└── utils/
    └── domainExtractor.js        # URL 도메인 추출 유틸 (선택적)
```

### 수정 파일
```
src/
├── views/
│   └── BrowserView.js            # HomePage 라우팅 로직 추가
│
├── components/
│   └── NavigationBar/
│       └── NavigationBar.js      # 홈 버튼 동작 수정 (about:home)
│
└── contexts/
    └── TabContext.js             # 초기 탭 URL 설정 (about:home)
```

---

## 14. 구현 단계 계획

### Phase 1: 자동 모드 구현 (MVP)
1. **HomeScreenService 구현**
   - `getTiles()`, `getTopSitesByVisitCount()`, `deduplicateByDomain()`
   - LocalStorage 타일 설정 로드/저장
2. **HomePage 컴포넌트 구현**
   - 타일 그리드 레이아웃 (CSS Grid)
   - 타일 클릭 핸들러
   - 빈 상태 렌더링
3. **BrowserView 통합**
   - HomePage 라우팅 (`about:home`)
   - NavigationBar 홈 버튼 수정
4. **테스트**
   - 단위 테스트 (HomeScreenService)
   - 컴포넌트 테스트 (HomePage)
   - 통합 테스트 (BrowserView + HomePage)

### Phase 2: 편집 모드 구현 (확장)
1. **HomeScreenService 확장**
   - `addTile()`, `removeTile()`, `reorderTiles()`
2. **HomePage 편집 모드 UI**
   - 편집 버튼 + 편집 헤더
   - 타일 삭제 버튼
   - 사이트 추가 다이얼로그
3. **북마크 선택 다이얼로그**
   - 북마크 목록 표시
   - 선택한 북마크 타일 추가
4. **테스트**
   - 편집 모드 단위/컴포넌트 테스트

### Phase 3: 폴리싱
1. **파비콘 최적화**
   - Google Favicon API 통합
   - 파비콘 캐싱 (IndexedDB)
2. **애니메이션 추가**
   - 타일 호버/포커스 애니메이션
   - 페이지 전환 애니메이션
3. **접근성 개선**
   - aria-label 추가
   - 키보드 네비게이션 최적화

---

## 15. 중대한 아키텍처 결정 요약

### 1. 홈 화면과 홈페이지 URL의 분리
- **결정**: 홈 화면(`about:home`)과 홈페이지 URL(F-11 설정)은 별개 개념
- **이유**:
  - 홈 화면: 자주 가는 사이트 타일 그리드 (브라우저 자체 기능)
  - 홈페이지 URL: 사용자가 설정한 시작 페이지 (외부 웹사이트)
- **영향**:
  - NavigationBar 홈 버튼: `about:home`으로 이동
  - 앱 초기 URL: F-11 설정의 홈페이지 URL 또는 `about:home`

### 2. 타일 자동 선정 vs 수동 편집
- **결정**: Phase 1에서는 자동 모드만 구현
- **이유**: 편집 모드는 선택적 기능(Should), 자동 모드가 핵심(Must)
- **영향**: MVP 출시 후 사용자 피드백 기반으로 편집 모드 추가 여부 결정

### 3. LocalStorage vs IndexedDB
- **결정**: 타일 설정은 LocalStorage, 북마크/히스토리는 IndexedDB
- **이유**: 타일 설정은 메타데이터(JSON), 북마크/히스토리는 대용량 데이터
- **영향**: 설정 데이터(F-11)와 일관된 저장소 사용, 빠른 로드

---

## 변경 이력
| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | F-15 기술 설계 |
