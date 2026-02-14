# 히스토리 관리 — 기술 설계서

## 1. 참조
- 요구사항 분석서: docs/specs/history-management/requirements.md
- 북마크 관리 설계서: docs/specs/bookmark-management/design.md
- PRD: docs/project/prd.md
- CLAUDE.md: /Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md

## 2. 아키텍처 개요

### 전체 구조
사용자가 방문한 웹 페이지를 자동으로 기록하고 관리할 수 있는 기능을 구현합니다. IndexedDB로 영속 데이터를 저장하며, 리모컨 최적화 UI로 날짜별 히스토리 조회, 검색, 삭제를 제공합니다.

```
┌─────────────────────────────────────────────────────────┐
│                      BrowserView                         │
│  ┌────────────────────────────────────────────────────┐ │
│  │             NavigationBar (F-04)                    │ │
│  │  [뒤로] [앞으로] [새로고침] [홈] [북마크] [히스토리]│ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │                  WebView (F-02)                     │ │
│  │  onLoadEnd → historyService.recordVisit()          │ │
│  │  (자동 히스토리 기록)                              │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘

HistoryPanel (히스토리 버튼 클릭 시 오버레이)
┌─────────────────────────────────────────────────────────┐
│  히스토리                         [검색] [모두 삭제] [닫기]│
│  ────────────────────────────────────────────────────   │
│  📅 오늘                                                │
│  ┌──────────────────────────────────────────────────┐  │
│  │  🔖 YouTube - 동영상 제목               14:35    │  │
│  │      https://www.youtube.com/watch?v=...         │  │
│  │  🔖 Naver                                12:20    │  │
│  │      https://www.naver.com                       │  │
│  └──────────────────────────────────────────────────┘  │
│  📅 어제                                                │
│  ┌──────────────────────────────────────────────────┐  │
│  │  🔖 Netflix                              22:45    │  │
│  │      https://www.netflix.com                     │  │
│  └──────────────────────────────────────────────────┘  │
│  📅 지난 7일                                            │
│  ...                                                    │
└─────────────────────────────────────────────────────────┘
```

### 핵심 설계 원칙
1. **자동 기록**: WebView의 `onLoadEnd` 이벤트 발생 시 자동으로 히스토리 저장
2. **영속성**: IndexedDB로 히스토리 데이터 저장 (앱 재시작 후에도 유지)
3. **날짜별 그룹화**: "오늘", "어제", "지난 7일", "이번 달", "이전" 그룹으로 시각적 분류
4. **리모컨 최적화**: Spotlight 통합으로 방향키 탐색, 선택 버튼으로 페이지 열기
5. **검색 및 삭제**: 제목/URL 검색, 개별/기간별/전체 삭제 지원
6. **중복 방지**: 동일 URL을 1분 내 재방문 시 중복 기록 안 함 (방문 시각만 업데이트)

### 북마크 관리와의 차이점
| 측면 | 히스토리 관리 (F-08) | 북마크 관리 (F-07) |
|------|---------------------|-------------------|
| **기록 방식** | 자동 (페이지 방문 시) | 수동 (사용자가 추가 버튼 클릭) |
| **데이터 구조** | 단순 리스트 (폴더 없음) | 폴더 계층 구조 (1단계 서브폴더) |
| **정렬 기준** | 날짜별 그룹화 (최신순) | 폴더별 그룹화 (생성일순) |
| **중복 처리** | 중복 허용 (visitCount 증가) | 중복 방지 (동일 URL 추가 불가) |
| **UI 초점** | 빠른 조회 및 검색 | 편집 및 폴더 관리 |
| **용량 제한** | 최대 5,000개 (오래된 항목 자동 삭제) | 제한 없음 (사용자가 직접 관리) |
| **자동완성 연동** | URLBar 자동완성 제공 (방문 빈도 우선) | URLBar 자동완성 제공 (제목 일치) |

## 3. 아키텍처 결정

### 결정 1: 데이터 저장소 선택
- **선택지**:
  - A) LocalStorage (5MB 제한)
  - B) IndexedDB (최소 50MB 이상)
  - C) webOS LS2 API (LG 전용)
- **결정**: B) IndexedDB
- **근거**:
  - 북마크 관리(F-07)와 동일한 저장소 사용 (일관성, 재사용성)
  - 히스토리는 북마크보다 데이터 양이 많음 (5,000개 제한)
  - IndexedDB는 인덱스 지원으로 날짜별/제목별 빠른 조회 가능
  - LocalStorage는 용량 제한으로 부적합
  - webOS LS2 API는 문서 부족, 웹 표준 우선
- **트레이드오프**: 비동기 API로 코드 복잡도 증가 (Promise 기반)

### 결정 2: 히스토리 자동 기록 시점
- **선택지**:
  - A) WebView의 `onLoadStart` 시점 (페이지 로딩 시작)
  - B) WebView의 `onLoadEnd` 시점 (페이지 로딩 완료)
  - C) `onNavigationChange` 시점 (URL 변경 감지)
- **결정**: B) `onLoadEnd` 시점
- **근거**:
  - 로딩 성공한 페이지만 기록 (에러 페이지 제외)
  - 페이지 제목을 `onLoadEnd`에서 추출 가능 (document.title)
  - `onLoadStart`는 로딩 실패 가능성 있음 (중간에 취소/에러)
  - `onNavigationChange`는 폴링 기반으로 정확도 낮음 (500ms 간격)
- **트레이드오프**: 로딩 시간이 긴 페이지는 기록 지연 (사용자 경험 저하 가능성 낮음)

### 결정 3: 중복 방지 전략
- **선택지**:
  - A) 동일 URL 완전 중복 방지 (URL을 UNIQUE 키로)
  - B) 1분 내 재방문 시 중복 기록 안 함 (visitedAt 업데이트)
  - C) 중복 허용 (모든 방문 기록)
- **결정**: B) 1분 내 재방문 시 중복 기록 안 함
- **근거**:
  - 동일 페이지를 짧은 시간 내 재방문 시 히스토리 목록이 중복으로 가득 참
  - 1분 이상 시간 차이 나면 별도 방문으로 기록 (유의미한 재방문)
  - visitCount 증가로 방문 빈도 추적 (자동완성 우선순위 활용)
  - URL을 UNIQUE로 하면 히스토리가 1개만 남아 "방문 시각" 정보 손실
- **트레이드오프**: 1분 임계값 조정 필요 (설정 화면에서 변경 가능하도록 향후 확장)

### 결정 4: 날짜별 그룹화 UI 구조
- **선택지**:
  - A) 단일 VirtualList (날짜 헤더 + 히스토리 항목 혼재)
  - B) Scroller 내부에 날짜별 Section 컴포넌트 (div 그룹)
  - C) Panel 전환 (날짜 선택 → 해당 날짜 히스토리 표시)
- **결정**: A) 단일 VirtualList (날짜 헤더 + 히스토리 항목 혼재)
- **근거**:
  - VirtualList로 성능 최적화 (히스토리 5,000개도 부드러운 스크롤)
  - 날짜 헤더를 VirtualList의 항목으로 포함 (type: 'header')
  - 리모컨 방향키로 연속 스크롤 가능 (Panel 전환보다 UX 우수)
  - 날짜 그룹 간 구분선 표시로 시각적 분리
- **트레이드오통**: VirtualList itemRenderer가 복잡해짐 (날짜 헤더와 히스토리 항목 조건부 렌더링)

### 결정 5: 히스토리 용량 제한
- **선택지**:
  - A) 제한 없음 (무한 저장)
  - B) 시간 기반 삭제 (30일 이상 자동 삭제)
  - C) 개수 기반 제한 (최대 5,000개, 오래된 항목부터 삭제)
- **결정**: C) 개수 기반 제한 (최대 5,000개)
- **근거**:
  - 시간 기반 삭제는 사용자 패턴에 따라 부적합 (매일 100개 방문하면 30일=3,000개)
  - 개수 기반은 일정한 성능 보장 (IndexedDB 조회 속도 일정)
  - 5,000개는 일반 사용자의 수개월치 히스토리 (보수적 추정)
  - 오래된 항목부터 삭제로 최신 데이터 보호
- **트레이드오프**: 설정 화면에서 제한 개수 변경 가능하도록 향후 확장 필요

### 결정 6: 검색 구현 방법
- **선택지**:
  - A) IndexedDB 인덱스 활용 (복합 쿼리, 제한적)
  - B) 전체 데이터 로드 후 클라이언트 필터링 (filter() 사용)
  - C) Web Worker로 백그라운드 검색
- **결정**: B) 전체 데이터 로드 후 클라이언트 필터링
- **근거**:
  - IndexedDB는 LIKE 쿼리 미지원 (부분 일치 검색 어려움)
  - 히스토리 최대 5,000개로 메모리 부담 적음 (~5MB 추정)
  - 클라이언트 필터링이 단순하고 빠름 (array.filter() 성능 우수)
  - Web Worker는 복잡도 증가 대비 성능 개선 미미
- **트레이드오프**: 히스토리가 5,000개일 때 검색 속도 0.3초 이내 유지 확인 필요

## 4. 데이터 모델 설계

### IndexedDB 스키마

#### 오브젝트 스토어: `history`
| 컬럼 | 타입 | 제약조건 | 설명 |
|------|------|----------|------|
| id | String | PK, UUID | 히스토리 고유 식별자 |
| url | String | NOT NULL | 방문한 URL |
| title | String | NOT NULL | 웹 페이지 제목 |
| favicon | String | NULL | 파비콘 URL (선택적 기능) |
| visitedAt | Number | NOT NULL | 방문 시각 (Unix timestamp 밀리초) |
| visitCount | Number | DEFAULT 1 | 동일 URL 방문 횟수 |

**인덱스**:
- `url`: 중복 체크 및 빠른 조회 (복합 인덱스: url + visitedAt)
- `visitedAt`: 날짜별 정렬 및 기간별 삭제
- `title`: 검색 기능 (부분 일치 검색은 클라이언트에서 처리)

**북마크 스키마와의 차이점**:
- `folderId` 컬럼 없음 (폴더 구조 미지원)
- `description` 컬럼 없음 (사용자 메모 미지원)
- `updatedAt` 컬럼 없음 (히스토리는 수정 안 함)
- `visitCount` 추가 (방문 빈도 추적)

### IndexedDB 초기화 코드
```javascript
// src/services/indexedDBService.js (F-07에서 생성된 파일 확장)
const DB_NAME = 'webOSBrowserDB'
const DB_VERSION = 1  // F-07과 동일 버전 (한 번에 생성)

/**
 * IndexedDB 초기화 및 스키마 생성
 * (F-07 bookmarkService에서 이미 생성된 경우 재사용)
 */
export const initDB = () => {
	return new Promise((resolve, reject) => {
		const request = indexedDB.open(DB_NAME, DB_VERSION)

		request.onerror = () => reject(request.error)
		request.onsuccess = () => resolve(request.result)

		request.onupgradeneeded = (event) => {
			const db = event.target.result

			// bookmarks 오브젝트 스토어 (F-07에서 생성)
			if (!db.objectStoreNames.contains('bookmarks')) {
				const bookmarkStore = db.createObjectStore('bookmarks', { keyPath: 'id' })
				bookmarkStore.createIndex('folderId', 'folderId', { unique: false })
				bookmarkStore.createIndex('url', 'url', { unique: true })
				bookmarkStore.createIndex('title', 'title', { unique: false })
				bookmarkStore.createIndex('createdAt', 'createdAt', { unique: false })
			}

			// folders 오브젝트 스토어 (F-07에서 생성)
			if (!db.objectStoreNames.contains('folders')) {
				const folderStore = db.createObjectStore('folders', { keyPath: 'id' })
				folderStore.createIndex('parentId', 'parentId', { unique: false })
				folderStore.createIndex('name', 'name', { unique: false })
			}

			// history 오브젝트 스토어 (F-08에서 생성)
			if (!db.objectStoreNames.contains('history')) {
				const historyStore = db.createObjectStore('history', { keyPath: 'id' })
				historyStore.createIndex('url', 'url', { unique: false })
				historyStore.createIndex('visitedAt', 'visitedAt', { unique: false })
				historyStore.createIndex('title', 'title', { unique: false })
				historyStore.createIndex('urlVisitedAt', ['url', 'visitedAt'], { unique: false })  // 복합 인덱스
			}
		}
	})
}
```

## 5. 서비스 계층 설계

### historyService 구조
```
src/services/historyService.js
├── recordVisit(url, title, favicon?)       # 히스토리 자동 기록 (중복 체크 포함)
├── getAllHistory()                         # 모든 히스토리 조회 (visitedAt 역순)
├── getHistoryByDateRange(startDate, endDate) # 기간별 히스토리 조회
├── getHistoryById(id)                      # 단일 히스토리 조회
├── deleteHistory(id)                       # 개별 히스토리 삭제
├── deleteHistoryByDateRange(startDate, endDate) # 기간별 히스토리 삭제
├── deleteAllHistory()                      # 전체 히스토리 삭제
├── searchHistory(query)                    # 히스토리 검색 (제목, URL)
├── pruneOldHistory()                       # 5,000개 초과 시 오래된 항목 삭제
└── groupHistoryByDate(historyItems)        # 날짜별 그룹화 (UI용)
```

### 주요 함수 구현 예시

#### recordVisit() — 자동 히스토리 기록
```javascript
/**
 * 히스토리 자동 기록 (WebView의 onLoadEnd에서 호출)
 * @param {String} url - 방문한 URL
 * @param {String} title - 웹 페이지 제목
 * @param {String} favicon - 파비콘 URL (선택)
 * @returns {Promise<Object>} - 추가/업데이트된 히스토리 객체
 */
export const recordVisit = async (url, title, favicon = null) => {
	const db = await initDB()
	const now = Date.now()

	// 1분 내 동일 URL 방문 체크 (중복 방지)
	const recentVisit = await getRecentVisitByUrl(url, 60000)  // 60000ms = 1분

	if (recentVisit) {
		// 중복: visitedAt, visitCount 업데이트
		const updatedHistory = {
			...recentVisit,
			visitedAt: now,
			visitCount: recentVisit.visitCount + 1
		}

		const transaction = db.transaction(['history'], 'readwrite')
		const store = transaction.objectStore('history')
		await promisifyRequest(store.put(updatedHistory))

		logger.info('[HistoryService] 히스토리 업데이트 (중복 방지):', updatedHistory)
		return updatedHistory
	}

	// 신규: 히스토리 추가
	const newHistory = {
		id: generateUUID(),
		url,
		title: title || url,
		favicon,
		visitedAt: now,
		visitCount: 1
	}

	const transaction = db.transaction(['history'], 'readwrite')
	const store = transaction.objectStore('history')
	await promisifyRequest(store.add(newHistory))

	logger.info('[HistoryService] 히스토리 추가:', newHistory)

	// 용량 제한 체크 (5,000개 초과 시 오래된 항목 삭제)
	await pruneOldHistory()

	return newHistory
}

/**
 * 최근 1분 내 동일 URL 방문 조회 (중복 체크용)
 * @param {String} url - 조회할 URL
 * @param {Number} timeWindow - 시간 윈도우 (밀리초)
 * @returns {Promise<Object|null>} - 히스토리 객체 또는 null
 */
const getRecentVisitByUrl = async (url, timeWindow) => {
	const db = await initDB()
	const now = Date.now()
	const threshold = now - timeWindow

	const transaction = db.transaction(['history'], 'readonly')
	const store = transaction.objectStore('history')
	const index = store.index('urlVisitedAt')

	// 복합 인덱스로 URL + 최근 방문 조회
	const range = IDBKeyRange.bound([url, threshold], [url, now])
	const request = index.openCursor(range, 'prev')  // 최신순

	return new Promise((resolve, reject) => {
		request.onsuccess = () => {
			const cursor = request.result
			if (cursor) {
				resolve(cursor.value)  // 가장 최근 방문
			} else {
				resolve(null)
			}
		}
		request.onerror = () => reject(request.error)
	})
}
```

#### groupHistoryByDate() — 날짜별 그룹화
```javascript
/**
 * 히스토리를 날짜별로 그룹화 (UI 렌더링용)
 * @param {Array} historyItems - 히스토리 배열 (visitedAt 역순 정렬)
 * @returns {Array} - 그룹화된 배열 [{ type: 'header', label: '오늘' }, { type: 'item', data: {...} }, ...]
 */
export const groupHistoryByDate = (historyItems) => {
	const now = Date.now()
	const oneDay = 24 * 60 * 60 * 1000

	const groups = {
		today: { label: '오늘', items: [] },
		yesterday: { label: '어제', items: [] },
		last7Days: { label: '지난 7일', items: [] },
		thisMonth: { label: '이번 달', items: [] },
		older: { label: '이전', items: [] }
	}

	// 히스토리를 날짜 그룹에 분류
	historyItems.forEach(item => {
		const diff = now - item.visitedAt

		if (diff < oneDay) {
			groups.today.items.push(item)
		} else if (diff < oneDay * 2) {
			groups.yesterday.items.push(item)
		} else if (diff < oneDay * 7) {
			groups.last7Days.items.push(item)
		} else if (diff < oneDay * 30) {
			groups.thisMonth.items.push(item)
		} else {
			groups.older.items.push(item)
		}
	})

	// VirtualList용 평탄화 배열 생성 (날짜 헤더 + 항목)
	const flatArray = []
	for (const key in groups) {
		const group = groups[key]
		if (group.items.length > 0) {
			// 날짜 헤더 추가
			flatArray.push({ type: 'header', label: group.label })

			// 히스토리 항목 추가
			group.items.forEach(item => {
				flatArray.push({ type: 'item', data: item })
			})
		}
	}

	return flatArray
}
```

#### deleteHistoryByDateRange() — 기간별 삭제
```javascript
/**
 * 기간별 히스토리 삭제
 * @param {Number} startDate - 시작 시각 (Unix timestamp)
 * @param {Number} endDate - 종료 시각 (Unix timestamp)
 * @returns {Promise<Number>} - 삭제된 항목 개수
 */
export const deleteHistoryByDateRange = async (startDate, endDate) => {
	const db = await initDB()

	const transaction = db.transaction(['history'], 'readwrite')
	const store = transaction.objectStore('history')
	const index = store.index('visitedAt')

	const range = IDBKeyRange.bound(startDate, endDate)
	const request = index.openCursor(range)

	let deletedCount = 0

	return new Promise((resolve, reject) => {
		request.onsuccess = () => {
			const cursor = request.result
			if (cursor) {
				cursor.delete()
				deletedCount++
				cursor.continue()
			} else {
				logger.info(`[HistoryService] 기간별 히스토리 삭제 완료: ${deletedCount}개`)
				resolve(deletedCount)
			}
		}
		request.onerror = () => reject(request.error)
	})
}
```

#### pruneOldHistory() — 용량 제한 (5,000개)
```javascript
/**
 * 히스토리 용량 제한 (최대 5,000개)
 * 5,000개 초과 시 오래된 항목부터 삭제
 * @returns {Promise<Number>} - 삭제된 항목 개수
 */
export const pruneOldHistory = async () => {
	const MAX_HISTORY_COUNT = 5000

	const allHistory = await getAllHistory()

	if (allHistory.length <= MAX_HISTORY_COUNT) {
		return 0
	}

	const db = await initDB()
	const transaction = db.transaction(['history'], 'readwrite')
	const store = transaction.objectStore('history')

	// 오래된 항목 (visitedAt 오름차순 정렬)
	const sortedHistory = allHistory.sort((a, b) => a.visitedAt - b.visitedAt)
	const deleteCount = allHistory.length - MAX_HISTORY_COUNT

	// 오래된 항목부터 삭제
	for (let i = 0; i < deleteCount; i++) {
		await promisifyRequest(store.delete(sortedHistory[i].id))
	}

	logger.info(`[HistoryService] 용량 제한으로 오래된 히스토리 삭제: ${deleteCount}개`)
	return deleteCount
}
```

#### searchHistory() — 히스토리 검색
```javascript
/**
 * 히스토리 검색 (제목, URL에서 부분 일치)
 * @param {String} query - 검색어
 * @returns {Promise<Array>} - 검색 결과 히스토리 배열
 */
export const searchHistory = async (query) => {
	const allHistory = await getAllHistory()
	const lowerQuery = query.toLowerCase()

	return allHistory.filter(history =>
		history.title.toLowerCase().includes(lowerQuery) ||
		history.url.toLowerCase().includes(lowerQuery)
	)
}
```

## 6. 컴포넌트 설계

### HistoryPanel 컴포넌트

#### 컴포넌트 구조
```
src/components/HistoryPanel/
├── HistoryPanel.js                 # 메인 패널 컴포넌트
├── HistoryList.js                  # 히스토리 리스트 (VirtualList)
├── HistoryItem.js                  # 히스토리 항목
├── DateHeader.js                   # 날짜 그룹 헤더
├── HistorySearchBar.js             # 검색 바
├── DeleteRangeDialog.js            # 기간별 삭제 다이얼로그
├── ConfirmDialog.js                # 삭제 확인 다이얼로그
├── HistoryPanel.module.less        # 스타일
└── index.js                        # Export 진입점
```

#### HistoryPanel Props 인터페이스
```javascript
// src/components/HistoryPanel/HistoryPanel.js
import PropTypes from 'prop-types'

HistoryPanel.propTypes = {
	// 패널 표시 상태
	visible: PropTypes.bool.isRequired,

	// 콜백
	onClose: PropTypes.func.isRequired,               // 패널 닫기
	onHistorySelect: PropTypes.func.isRequired,       // 히스토리 선택 시 페이지 열기
	onHistoryDeleted: PropTypes.func,                 // 히스토리 삭제 완료 (토스트 메시지용)

	// 스타일 커스터마이징
	className: PropTypes.string
}

HistoryPanel.defaultProps = {
	onHistoryDeleted: () => {},
	className: ''
}
```

#### HistoryPanel 상태 관리
```javascript
// src/components/HistoryPanel/HistoryPanel.js
import { useState, useEffect } from 'react'
import * as historyService from '../../services/historyService'

const HistoryPanel = ({ visible, onClose, onHistorySelect }) => {
	// 히스토리 데이터
	const [historyItems, setHistoryItems] = useState([])
	const [groupedHistory, setGroupedHistory] = useState([])  // VirtualList용 평탄화 배열

	// UI 상태
	const [searchQuery, setSearchQuery] = useState('')
	const [showDeleteDialog, setShowDeleteDialog] = useState(false)
	const [showRangeDialog, setShowRangeDialog] = useState(false)
	const [deletingItem, setDeletingItem] = useState(null)  // 삭제 중인 히스토리

	// IndexedDB 초기화 및 데이터 로드
	useEffect(() => {
		if (visible) {
			loadHistory()
		}
	}, [visible])

	// 검색어 변경 시 필터링
	useEffect(() => {
		if (searchQuery) {
			// 검색 결과 필터링
			historyService.searchHistory(searchQuery).then(results => {
				const grouped = historyService.groupHistoryByDate(results)
				setGroupedHistory(grouped)
			})
		} else {
			// 전체 히스토리 표시
			const grouped = historyService.groupHistoryByDate(historyItems)
			setGroupedHistory(grouped)
		}
	}, [searchQuery, historyItems])

	const loadHistory = async () => {
		try {
			const data = await historyService.getAllHistory()
			setHistoryItems(data)

			const grouped = historyService.groupHistoryByDate(data)
			setGroupedHistory(grouped)
		} catch (error) {
			logger.error('[HistoryPanel] 히스토리 로드 실패:', error)
		}
	}

	// 히스토리 선택 핸들러
	const handleHistoryClick = (history) => {
		onHistorySelect({ url: history.url, title: history.title })
		onClose()
	}

	// 개별 삭제 핸들러
	const handleDelete = async (history) => {
		setDeletingItem(history)
		setShowDeleteDialog(true)
	}

	const confirmDelete = async () => {
		if (!deletingItem) return

		try {
			await historyService.deleteHistory(deletingItem.id)
			await loadHistory()
			setShowDeleteDialog(false)
			setDeletingItem(null)
			logger.info('[HistoryPanel] 히스토리 삭제 완료:', deletingItem.url)
		} catch (error) {
			logger.error('[HistoryPanel] 히스토리 삭제 실패:', error)
		}
	}

	// 전체 삭제 핸들러
	const handleDeleteAll = async () => {
		try {
			await historyService.deleteAllHistory()
			await loadHistory()
			logger.info('[HistoryPanel] 전체 히스토리 삭제 완료')
		} catch (error) {
			logger.error('[HistoryPanel] 전체 히스토리 삭제 실패:', error)
		}
	}

	// 기간별 삭제 핸들러
	const handleDeleteRange = async (range) => {
		// range: 'lastHour', 'today', 'last7Days', 'last30Days', 'all'
		const now = Date.now()
		const oneHour = 60 * 60 * 1000
		const oneDay = 24 * 60 * 60 * 1000

		let startDate = 0
		let endDate = now

		switch (range) {
			case 'lastHour':
				startDate = now - oneHour
				break
			case 'today':
				startDate = now - oneDay
				break
			case 'last7Days':
				startDate = now - oneDay * 7
				break
			case 'last30Days':
				startDate = now - oneDay * 30
				break
			case 'all':
				// deleteAllHistory() 호출
				await handleDeleteAll()
				return
			default:
				return
		}

		try {
			await historyService.deleteHistoryByDateRange(startDate, endDate)
			await loadHistory()
			setShowRangeDialog(false)
			logger.info(`[HistoryPanel] 기간별 히스토리 삭제 완료: ${range}`)
		} catch (error) {
			logger.error('[HistoryPanel] 기간별 히스토리 삭제 실패:', error)
		}
	}

	// ... (JSX 렌더링 생략)
}
```

### HistoryList 컴포넌트 (Enact VirtualList)
```javascript
// src/components/HistoryPanel/HistoryList.js
import { VirtualList } from '@enact/moonstone/VirtualList'
import HistoryItem from './HistoryItem'
import DateHeader from './DateHeader'

const HistoryList = ({ groupedHistory, onHistoryClick, onDelete }) => {
	/**
	 * VirtualList itemRenderer
	 * groupedHistory: [{ type: 'header', label: '오늘' }, { type: 'item', data: {...} }, ...]
	 */
	const renderItem = ({ index, ...rest }) => {
		const item = groupedHistory[index]

		if (item.type === 'header') {
			// 날짜 헤더
			return <DateHeader {...rest} label={item.label} />
		} else {
			// 히스토리 항목
			return (
				<HistoryItem
					{...rest}
					history={item.data}
					onClick={onHistoryClick}
					onDelete={onDelete}
				/>
			)
		}
	}

	return (
		<VirtualList
			dataSize={groupedHistory.length}
			itemRenderer={renderItem}
			itemSize={({ index }) => {
				// 날짜 헤더는 높이 60px, 히스토리 항목은 80px
				const item = groupedHistory[index]
				return item.type === 'header' ? 60 : 80
			}}
			spacing={0}
		/>
	)
}

export default HistoryList
```

### HistoryItem 컴포넌트
```javascript
// src/components/HistoryPanel/HistoryItem.js
import { useState } from 'react'
import Button from '@enact/moonstone/Button'
import Spotlight from '@enact/spotlight'
import css from './HistoryPanel.module.less'

const HistoryItem = ({ history, onClick, onDelete, ...rest }) => {
	const [showMenu, setShowMenu] = useState(false)

	const handleClick = () => {
		onClick(history)
	}

	const handleContextMenu = () => {
		setShowMenu(!showMenu)
	}

	// 방문 시각 포맷 (HH:mm)
	const formatTime = (timestamp) => {
		const date = new Date(timestamp)
		const hours = String(date.getHours()).padStart(2, '0')
		const minutes = String(date.getMinutes()).padStart(2, '0')
		return `${hours}:${minutes}`
	}

	return (
		<div className={css.historyItem} {...rest}>
			{/* 히스토리 정보 영역 (선택 버튼으로 페이지 열기) */}
			<div
				className={css.historyInfo}
				onClick={handleClick}
				spotlightId={`history-${history.id}`}
			>
				<div className={css.favicon}>🔖</div>
				<div className={css.details}>
					<div className={css.title}>{history.title}</div>
					<div className={css.url}>{history.url}</div>
				</div>
				<div className={css.time}>{formatTime(history.visitedAt)}</div>
			</div>

			{/* 컨텍스트 메뉴 (옵션 버튼) */}
			{showMenu && (
				<div className={css.contextMenu}>
					<Button onClick={() => onDelete(history)} small>삭제</Button>
					{/* 향후: 북마크 추가, 새 탭에서 열기 버튼 */}
				</div>
			)}
		</div>
	)
}

export default HistoryItem
```

### DateHeader 컴포넌트
```javascript
// src/components/HistoryPanel/DateHeader.js
import css from './HistoryPanel.module.less'

const DateHeader = ({ label, ...rest }) => {
	return (
		<div className={css.dateHeader} {...rest}>
			<span className={css.dateLabel}>📅 {label}</span>
		</div>
	)
}

export default DateHeader
```

## 7. WebView 연동 (자동 히스토리 기록)

### BrowserView에서 historyService.recordVisit() 호출
```javascript
// src/views/BrowserView.js (수정)
import * as historyService from '../services/historyService'

const BrowserView = () => {
	const [currentUrl, setCurrentUrl] = useState('https://www.google.com')
	const [currentTitle, setCurrentTitle] = useState('Google')
	const [showHistoryPanel, setShowHistoryPanel] = useState(false)

	// WebView의 onLoadEnd 콜백에서 히스토리 자동 기록
	const handleLoadEnd = async ({ url, title }) => {
		setCurrentTitle(title)

		// 히스토리 자동 기록
		try {
			await historyService.recordVisit(url, title)
			logger.info('[BrowserView] 히스토리 자동 기록 완료:', url)
		} catch (error) {
			logger.error('[BrowserView] 히스토리 기록 실패:', error)
		}
	}

	// 히스토리 버튼 클릭 핸들러
	const handleHistoryClick = () => {
		setShowHistoryPanel(true)
	}

	// 히스토리 선택 핸들러
	const handleHistorySelect = (history) => {
		setCurrentUrl(history.url)
		setCurrentTitle(history.title)
		setShowHistoryPanel(false)
		logger.info('[BrowserView] 히스토리에서 페이지 열기:', history.url)
	}

	return (
		<Panel className={css.browserView}>
			<NavigationBar
				onHistoryClick={handleHistoryClick}
				{/* ... 기타 props */}
			/>

			<WebView
				url={currentUrl}
				onLoadEnd={handleLoadEnd}  // 히스토리 자동 기록
				{/* ... 기타 props */}
			/>

			{/* HistoryPanel 오버레이 */}
			<HistoryPanel
				visible={showHistoryPanel}
				onClose={() => setShowHistoryPanel(false)}
				onHistorySelect={handleHistorySelect}
			/>
		</Panel>
	)
}
```

### NavigationBar에 히스토리 버튼 추가
```javascript
// src/components/NavigationBar/NavigationBar.js (수정)
<Button
	className={css.navButton}
	onClick={onHistoryClick}
	icon="list"
	spotlightId="nav-history"
>
	히스토리
</Button>
```

## 8. 시퀀스 흐름

### 주요 시나리오: 자동 히스토리 기록
```
사용자         URLBar      WebView           historyService      IndexedDB
  │              │            │                    │                │
  │  URL 입력 후 Enter         │                    │                │
  │──────────────▶│            │                    │                │
  │              │  setCurrentUrl(url)              │                │
  │              │────────────▶│                    │                │
  │              │            │  onLoadStart()      │                │
  │              │            │  (로딩 시작)        │                │
  │              │            │                    │                │
  │              │            │  (페이지 로딩 중...)│                │
  │              │            │                    │                │
  │              │            │  onLoadEnd({ url, title })           │
  │              │            │─────────────────────▶               │
  │              │            │                    │  recordVisit(url, title)
  │              │            │                    │─────────────────▶
  │              │            │                    │  getRecentVisitByUrl(url, 60000)
  │              │            │                    │─────────────────▶
  │              │            │                    │◀─────────────────
  │              │            │                    │  (중복 체크: null)
  │              │            │                    │  store.add(newHistory)
  │              │            │                    │─────────────────▶
  │              │            │                    │◀─────────────────
  │              │            │                    │  pruneOldHistory()
  │              │            │                    │─────────────────▶
  │              │            │                    │◀─────────────────
  │              │            │◀─────────────────────               │
  │              │  로딩 완료, 히스토리 자동 기록 완료                │
```

### 에러 시나리오: 1분 내 동일 URL 재방문 (중복 방지)
```
사용자         WebView           historyService      IndexedDB
  │               │                    │                │
  │  동일 URL 재방문 (1분 내)           │                │
  │──────────────▶│                    │                │
  │               │  onLoadEnd({ url, title })           │
  │               │─────────────────────▶               │
  │               │                    │  recordVisit(url, title)
  │               │                    │─────────────────▶
  │               │                    │  getRecentVisitByUrl(url, 60000)
  │               │                    │─────────────────▶
  │               │                    │◀─────────────────
  │               │                    │  (recentVisit 발견)
  │               │                    │  store.put({ ...recentVisit, visitedAt: now, visitCount: +1 })
  │               │                    │─────────────────▶
  │               │                    │◀─────────────────
  │               │◀─────────────────────               │
  │  히스토리 업데이트 (중복 기록 안 함)                │
```

### 주요 시나리오: 히스토리 목록 조회 및 실행
```
사용자         HistoryPanel   historyService   BrowserView   WebView
  │                │                 │              │            │
  │  히스토리 버튼 클릭                │              │            │
  │───────────────▶│                 │              │            │
  │                │  loadHistory()  │              │            │
  │                │─────────────────▶              │            │
  │                │                 │  getAllHistory()          │
  │                │                 │─────────────▶│            │
  │                │                 │◀─────────────│            │
  │                │                 │  groupHistoryByDate(data) │
  │                │                 │─────────────▶│            │
  │                │                 │◀─────────────│            │
  │                │◀─────────────────              │            │
  │                │  HistoryPanel 렌더링 (날짜별 그룹)          │
  │                │                 │              │            │
  │  히스토리 항목 선택                │              │            │
  │───────────────▶│                 │              │            │
  │                │  onHistorySelect({ url, title })            │
  │                │──────────────────────────────▶│            │
  │                │                 │              │  setCurrentUrl(url)
  │                │                 │              │───────────▶│
  │                │  onClose()      │              │            │
  │                │  (패널 닫기)    │              │            │
```

### 주요 시나리오: 히스토리 검색
```
사용자         HistoryPanel   historyService
  │                │                 │
  │  검색 버튼 클릭│                 │
  │───────────────▶│                 │
  │                │  setSearchQuery('youtube')
  │                │                 │
  │  검색어 입력   │                 │
  │───────────────▶│                 │
  │                │  searchHistory(query)
  │                │─────────────────▶
  │                │                 │  getAllHistory() → filter()
  │                │◀─────────────────
  │                │  groupHistoryByDate(results)
  │                │─────────────────▶
  │                │◀─────────────────
  │                │  VirtualList 갱신 (검색 결과)
  │◀───────────────│                 │
```

### 주요 시나리오: 기간별 히스토리 삭제
```
사용자         HistoryPanel   DeleteRangeDialog   historyService   IndexedDB
  │                │                 │                    │              │
  │  "모두 삭제" 버튼 클릭            │                    │              │
  │───────────────▶│                 │                    │              │
  │                │  setShowRangeDialog(true)            │              │
  │                │────────────────▶│                    │              │
  │                │                 │  DeleteRangeDialog 렌더링         │
  │                │                 │  (지난 1시간, 오늘, 지난 7일, ...)│
  │                │                 │                    │              │
  │  "지난 7일" 선택│                 │                    │              │
  │───────────────────────────────▶│                    │              │
  │                │                 │  handleDeleteRange('last7Days')   │
  │                │                 │─────────────────────▶            │
  │                │                 │                    │  deleteHistoryByDateRange(startDate, endDate)
  │                │                 │                    │─────────────▶
  │                │                 │                    │  index.openCursor(range)
  │                │                 │                    │  cursor.delete() (반복)
  │                │                 │                    │◀─────────────
  │                │                 │◀─────────────────────            │
  │                │                 │  loadHistory() (목록 갱신)       │
  │                │                 │─────────────────────▶            │
  │                │                 │◀─────────────────────            │
  │                │  토스트 메시지 표시                    │              │
  │◀───────────────────────────────│  "지난 7일 히스토리가 삭제되었습니다"│
```

## 9. 리모컨 키 매핑

### HistoryPanel 포커스 흐름
```
Spotlight Container: HistoryPanel
├── Header (검색, 모두 삭제, 닫기 버튼)
└── HistoryList (VirtualList)
    ├── DateHeader (날짜 헤더, 포커스 불가)
    ├── HistoryItem (방향키 하)
    ├── HistoryItem (방향키 하)
    ├── DateHeader (날짜 헤더, 포커스 불가)
    └── HistoryItem (방향키 하)
```

### 리모컨 키 매핑
| 키 | 동작 | 컨텍스트 |
|----|------|----------|
| **방향키 상/하** | 히스토리 목록 스크롤 | HistoryList |
| **선택 버튼 (Enter/OK)** | 히스토리 실행 (페이지 열기) | HistoryItem |
| **백 버튼 (Backspace)** | HistoryPanel 닫기, 다이얼로그 닫기 | 패널 또는 다이얼로그 열림 시 |
| **컬러 버튼 (빨강)** | 히스토리 검색 | HistoryPanel 열림 시 |
| **컬러 버튼 (노랑)** | 히스토리 삭제 (개별) | HistoryItem 포커스 시 |
| **컬러 버튼 (파랑)** | 모두 삭제 | HistoryPanel 열림 시 |
| **옵션 버튼** | 컨텍스트 메뉴 열기 (삭제, 북마크 추가) | HistoryItem 포커스 시 |

### Spotlight 설정 (HistoryPanel)
```javascript
// src/components/HistoryPanel/HistoryPanel.js
useEffect(() => {
	if (visible) {
		// HistoryPanel 포커스 설정
		Spotlight.set('history-panel', {
			defaultElement: '[data-spotlight-id="history-list"]',
			enterTo: 'default-element'
		})

		// 초기 포커스를 HistoryList로 이동
		Spotlight.focus('history-list')
	}
}, [visible])
```

### DateHeader 포커스 스킵 처리
```javascript
// src/components/HistoryPanel/DateHeader.js
// DateHeader는 포커스 불가 (Spotlight에서 스킵)
const DateHeader = ({ label, ...rest }) => {
	return (
		<div className={css.dateHeader} data-spotlight-disabled {...rest}>
			<span className={css.dateLabel}>📅 {label}</span>
		</div>
	)
}
```

## 10. URL 자동완성 연동 (F-03)

### URLBar에 히스토리 기반 자동완성 제공
```javascript
// src/components/URLBar/URLBar.js (수정)
import * as historyService from '../../services/historyService'

const URLBar = ({ onSubmit, ...rest }) => {
	const [inputValue, setInputValue] = useState('')
	const [suggestions, setSuggestions] = useState([])

	// 입력값 변경 시 히스토리 기반 자동완성
	useEffect(() => {
		if (inputValue.length >= 2) {
			// 히스토리 검색
			historyService.searchHistory(inputValue).then(results => {
				// 방문 빈도 및 최근 방문 순으로 정렬
				const sorted = results.sort((a, b) => {
					// 1순위: visitCount 높은 순
					if (b.visitCount !== a.visitCount) {
						return b.visitCount - a.visitCount
					}
					// 2순위: visitedAt 최신순
					return b.visitedAt - a.visitedAt
				})

				// 최대 10개 제안
				setSuggestions(sorted.slice(0, 10))
			})
		} else {
			setSuggestions([])
		}
	}, [inputValue])

	// ... (자동완성 렌더링)
}
```

## 11. 스타일 설계

### HistoryPanel 스타일
```less
// src/components/HistoryPanel/HistoryPanel.module.less
.historyPanel {
	position: fixed;
	top: 0;
	right: 0;
	width: 600px;  // 패널 너비
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

.historyList {
	flex: 1;
	overflow-y: auto;
}

// 날짜 헤더 스타일
.dateHeader {
	padding: 12px 16px;
	background-color: rgba(255, 255, 255, 0.03);
	border-bottom: 1px solid rgba(255, 255, 255, 0.1);
	margin-bottom: 4px;
}

.dateLabel {
	font-size: 22px;
	font-weight: bold;
	color: var(--text-secondary-color);
}

// 히스토리 항목 스타일
.historyItem {
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

.historyInfo {
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

.time {
	font-size: 18px;
	color: var(--text-secondary-color);
	margin-left: 16px;
	min-width: 60px;
	text-align: right;
}

.contextMenu {
	display: flex;
	gap: 8px;
}

// 빈 히스토리 메시지
.emptyMessage {
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	height: 100%;
	color: var(--text-secondary-color);
	font-size: 20px;
}
```

## 12. 영향 범위 분석

### 수정 필요한 기존 파일
1. **src/views/BrowserView.js**:
   - HistoryPanel import 추가
   - showHistoryPanel 상태 추가
   - handleLoadEnd에서 historyService.recordVisit() 호출 (자동 기록)
   - handleHistoryClick, handleHistorySelect 핸들러 추가
   - JSX에 `<HistoryPanel>` 오버레이 추가

2. **src/components/NavigationBar/NavigationBar.js**:
   - 히스토리 버튼 추가 (icon="list")
   - onHistoryClick prop 추가

3. **src/components/URLBar/URLBar.js** (F-03 완료 후):
   - suggestions prop에 히스토리 데이터 추가 (historyService에서 제공)
   - 자동완성 드롭다운에 히스토리 표시 (방문 빈도 우선순위)

4. **src/services/indexedDBService.js** (F-07에서 생성):
   - `history` 오브젝트 스토어 추가 (onupgradeneeded)
   - DB_VERSION은 F-07과 동일 (1) 유지 (동시 생성)

### 새로 생성할 파일
1. **src/services/historyService.js**: 히스토리 CRUD 서비스
2. **src/components/HistoryPanel/HistoryPanel.js**: 메인 패널 컴포넌트
3. **src/components/HistoryPanel/HistoryList.js**: VirtualList 래퍼
4. **src/components/HistoryPanel/HistoryItem.js**: 히스토리 항목
5. **src/components/HistoryPanel/DateHeader.js**: 날짜 그룹 헤더
6. **src/components/HistoryPanel/HistorySearchBar.js**: 검색 바
7. **src/components/HistoryPanel/DeleteRangeDialog.js**: 기간별 삭제 다이얼로그
8. **src/components/HistoryPanel/ConfirmDialog.js**: 삭제 확인 다이얼로그
9. **src/components/HistoryPanel/HistoryPanel.module.less**: 스타일
10. **src/components/HistoryPanel/index.js**: Export 진입점

### 영향 받는 기존 기능
- **F-02 (웹뷰 통합)**: onLoadEnd 콜백에서 historyService.recordVisit() 호출 (자동 기록)
- **F-03 (URL 입력 UI)**: historyService에서 히스토리 데이터를 제공하여 자동완성 표시
- **F-04 (페이지 탐색 컨트롤)**: NavigationBar에 히스토리 버튼 추가
- **F-07 (북마크 관리)**: 히스토리 실행 시 북마크 추가 옵션 제공 (컨텍스트 메뉴)

## 13. 기술적 주의사항

### IndexedDB 비동기 처리
- **문제**: IndexedDB는 비동기 API로 Promise 기반 처리 필요
- **대응**: 모든 IndexedDB 작업을 async/await로 래핑, 에러 처리 필수
- **유틸리티 함수**: F-07 bookmarkService에서 이미 구현된 promisifyRequest() 재사용

### 중복 방지 로직 (1분 임계값)
- **문제**: 동일 URL을 짧은 시간 내 재방문 시 히스토리 중복 기록
- **대응**: getRecentVisitByUrl()로 1분 내 동일 URL 방문 체크 → visitedAt, visitCount 업데이트
- **복합 인덱스**: `urlVisitedAt` 인덱스 (url + visitedAt)로 빠른 조회

### 히스토리 용량 제한 (5,000개)
- **문제**: 히스토리가 무한정 쌓이면 IndexedDB 조회 속도 저하
- **대응**: pruneOldHistory()를 recordVisit() 시 호출, 5,000개 초과 시 오래된 항목 삭제
- **성능 최적화**: 삭제는 백그라운드로 처리, UI 차단 없음

### VirtualList 성능 최적화
- **문제**: 히스토리 5,000개 렌더링 시 DOM 노드 과다 생성
- **대응**: Enact VirtualList 사용으로 가상 스크롤 (뷰포트에 보이는 항목만 렌더링)
- **측정**: 히스토리 5,000개 테스트 시 스크롤 프레임레이트 30fps 이상 유지 확인

### 날짜별 그룹화 계산 성능
- **문제**: groupHistoryByDate() 함수가 5,000개 배열을 반복 순회
- **대응**: 한 번만 계산 후 캐싱, 검색어 변경 시에만 재계산
- **성능**: 5,000개 그룹화 계산 시간 100ms 이내 유지

### WebView CORS 제약 (페이지 제목 추출)
- **문제**: Same-Origin Policy로 Cross-Origin 페이지의 document.title 접근 불가
- **대응**: try-catch로 CORS 에러 무시, 제목 추출 실패 시 URL을 제목으로 사용
- **WebView.js**: extractTitle() 함수에서 이미 처리됨 (F-02)

### 파비콘 처리 (선택적 기능)
- **현재**: 파비콘 표시 안 함 (이모지 아이콘 사용)
- **향후**: 히스토리 기록 시 웹사이트 파비콘 다운로드 및 저장 (Blob)
- **제약**: CORS 제약으로 Same-Origin 사이트만 파비콘 다운로드 가능

## 14. 확장성 고려사항

### F-07 (북마크 관리)와의 연동
- **현재**: 히스토리와 북마크는 독립적
- **향후**: 히스토리 항목에서 "북마크 추가" 버튼 제공 (컨텍스트 메뉴)
- **구현 방향**: HistoryItem에서 bookmarkService.addBookmark() 호출

### F-11 (설정 화면)와의 연동
- **히스토리 자동 삭제**: 설정에서 자동 삭제 기간 설정 (30일 이후 자동 삭제)
- **히스토리 기록 끄기**: 설정에서 히스토리 기록 비활성화 (시크릿 모드)
- **히스토리 용량 제한**: 설정에서 최대 히스토리 개수 변경 (5,000 → 10,000)

### F-15 (즐겨찾기 홈 화면)와의 연동
- **자주 가는 사이트**: visitCount 기반으로 상위 N개 히스토리 표시
- **구현 방향**: historyService에 `getTopHistory(limit)` 함수 추가

### 히스토리 동기화 (향후)
- **현재**: 로컬 IndexedDB만 지원
- **향후**: LG 계정 연동으로 히스토리 클라우드 동기화
- **데이터 모델**: 히스토리 스키마에 `syncedAt` 컬럼 추가

### 히스토리 가져오기/내보내기 (향후)
- **현재**: 미지원
- **향후**: JSON 파일 가져오기/내보내기 기능 추가
- **구현 방향**: historyService에 `exportHistory()`, `importHistory()` 함수 추가

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-13 | 최초 작성 | F-08 요구사항 기반 기술 설계 |
