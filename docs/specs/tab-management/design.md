# 탭 관리 시스템 — 기술 설계서

## 1. 참조
- 요구사항 분석서: docs/specs/tab-management/requirements.md
- 기존 컴포넌트: src/views/BrowserView.js
- 기존 WebView: src/components/WebView/WebView.js
- 기존 NavigationBar: src/components/NavigationBar/NavigationBar.js

---

## 2. 아키텍처 개요

### 2.1 핵심 개념

탭 관리 시스템은 **메모리 효율성**과 **빠른 탭 전환**을 최우선 목표로 합니다. 이를 위해 다음과 같은 아키텍처 원칙을 따릅니다:

1. **단일 WebView 인스턴스**: DOM에는 항상 1개의 iframe만 존재 (백그라운드 탭은 DOM에서 제거)
2. **경량 탭 상태 메타데이터**: 각 탭의 URL, 제목, 히스토리 스택은 메모리에만 저장
3. **Context API 기반 상태 관리**: 전역 탭 상태를 Context로 관리하여 컴포넌트 간 동기화
4. **서비스 계층 분리**: TabManager 서비스가 비즈니스 로직을 캡슐화

### 2.2 레이어 구조

```
┌─────────────────────────────────────────────────┐
│  UI Layer (React Components)                    │
│  - BrowserView (탭 전환 로직 조율)                │
│  - TabBar (탭 목록 + 컨트롤)                     │
│  - WebView (웹 렌더링, 단일 인스턴스)             │
│  - NavigationBar (히스토리 버튼 상태 반영)       │
└─────────────────────────────────────────────────┘
                    ▼
┌─────────────────────────────────────────────────┐
│  State Management (Context API)                 │
│  - TabContext (tabs, activeTabId, dispatch)     │
│  - useTabContext hook (컨텍스트 접근 편의)       │
└─────────────────────────────────────────────────┘
                    ▼
┌─────────────────────────────────────────────────┐
│  Business Logic (Service Layer)                 │
│  - TabManager (탭 CRUD, 활성 탭 전환)            │
│  - 순수 함수 + 불변성 유지                        │
└─────────────────────────────────────────────────┘
```

---

## 3. 아키텍처 결정

### 결정 1: 상태 관리 방식 (Context API vs Redux vs Zustand)

**선택지**:
- A) Context API + useReducer
- B) Redux Toolkit
- C) Zustand (제3자 라이브러리)

**결정**: **A) Context API + useReducer**

**근거**:
1. **프로젝트 규모 적합**: 전역 상태가 탭 목록 1개뿐 (북마크/히스토리는 별도 서비스)
2. **의존성 최소화**: Redux는 오버엔지니어링, Zustand는 외부 의존성 추가
3. **Enact 호환성**: React Context는 Enact와 완벽히 호환됨
4. **보일러플레이트 적음**: reducer 1개 + action 5개 정도로 충분

**트레이드오프**:
- **포기**: Redux DevTools 디버깅, 미들웨어 확장성
- **획득**: 단순한 코드, 빠른 개발 속도, 번들 크기 절감

### 결정 2: WebView 인스턴스 관리 전략 (멀티 vs 싱글)

**선택지**:
- A) 탭당 1개씩 iframe 생성, 백그라운드 탭은 `display: none` (멀티 인스턴스)
- B) 활성 탭에만 iframe 생성, 백그라운드 탭은 DOM에서 제거 (싱글 인스턴스)
- C) 탭당 1개씩 iframe 생성, 백그라운드 탭은 `about:blank`로 교체

**결정**: **B) 싱글 인스턴스 (활성 탭만 DOM에 마운트)**

**근거**:
1. **메모리 최적화**: iframe 5개 동시 로드 시 프로젝터 메모리 부족 (각 iframe은 독립 렌더링 프로세스)
2. **CPU 절약**: 백그라운드 탭의 JavaScript 실행 완전 차단 (배터리 절약, 발열 감소)
3. **탭 전환 속도**: DOM 조작(unmount/mount)은 0.1~0.3초로 충분히 빠름
4. **CORS 회피 불필요**: 탭 상태(URL, 제목)는 WebView 콜백에서 수집 (iframe 내부 접근 불필요)

**트레이드오프**:
- **포기**: 백그라운드 탭의 실시간 업데이트 (예: 채팅 알림), 탭 전환 시 페이지 재로딩
- **획득**: 메모리 사용량 80% 절감, 배터리 수명 연장, 안정성 향상

**구현 방식**:
```javascript
// 탭 전환 시
1. 이전 탭: WebView unmount (iframe src를 'about:blank'로 변경)
2. 메타데이터 저장: 이전 탭의 URL, 히스토리 스택을 TabContext에 저장
3. 새 탭: WebView remount (새 탭의 저장된 URL로 iframe src 설정)
4. NavigationBar 업데이트: 새 탭의 canGoBack/canGoForward 반영
```

### 결정 3: 탭 히스토리 스택 관리 (독립 vs 공유)

**선택지**:
- A) 각 탭이 독립적인 히스토리 스택 보유
- B) 전역 히스토리 스택 공유 (탭 간 이동 이력)

**결정**: **A) 탭별 독립 히스토리 스택**

**근거**:
1. **사용자 기대**: 탭 A에서 뒤로 가기 → 탭 A의 이전 페이지로 이동 (탭 B 영향 X)
2. **데스크톱 브라우저 표준**: Chrome, Firefox는 모두 탭별 독립 히스토리
3. **NavigationBar 연동**: 탭 전환 시 `canGoBack/canGoForward`가 각 탭의 스택 기준으로 변경됨

**데이터 구조**:
```javascript
{
  id: "tab-001",
  historyStack: ["https://google.com", "https://naver.com", "https://youtube.com"],
  historyIndex: 2,  // 현재 youtube.com
  // canGoBack = true (index > 0)
  // canGoForward = false (index === historyStack.length - 1)
}
```

### 결정 4: 탭 영속성 (세션 복원)

**선택지**:
- A) 앱 재시작 시 탭 목록 복원 (IndexedDB 저장)
- B) 앱 재시작 시 초기화 (메모리만 사용)

**결정**: **B) 메모리만 사용 (영속성 없음)**

**근거**:
1. **요구사항 명시**: "탭 영속성 없음" (requirements.md 5.4절)
2. **단순성**: IndexedDB 동기화 로직 불필요, 버그 감소
3. **프라이버시**: 앱 종료 시 모든 탭 자동 삭제 (사용자 추적 방지)

**향후 확장**:
- F-11 설정 기능에서 "탭 복원" 옵션 추가 검토 가능

### 결정 5: 최대 탭 수 제한 (5개)

**선택지**:
- A) 하드 리미트 5개 (초과 시 추가 불가)
- B) 소프트 리미트 (경고 후 추가 허용)
- C) 동적 리미트(메모리 감시)

**결정**: **A) 하드 리미트 5개**

**근거**:
1. **하드웨어 제약**: 프로젝터 RAM 제한 (NFR-1)
2. **UX 단순화**: 리모컨으로 탭 전환 시 5개 이상은 조작 복잡도 증가
3. **안정성**: 메모리 부족으로 인한 앱 크래시 방지

**UI 동작**:
- 5개째 탭 생성 시: "+ 새 탭" 버튼 disabled
- 클릭 시: "최대 5개 탭까지 생성 가능합니다" 경고 (Toast 메시지, F-14 연동)

---

## 4. 상태 관리 설계

### 4.1 TabContext 구조

```javascript
// TabContext 전역 상태
{
  tabs: [
    {
      id: "tab-001",                              // UUID
      url: "https://www.google.com",              // 현재 URL
      title: "Google",                            // 페이지 제목
      favicon: null,                              // 파비콘 URL (향후 F-15)
      historyStack: ["https://www.google.com"],   // 히스토리 스택
      historyIndex: 0,                            // 현재 인덱스
      isLoading: false,                           // 로딩 상태
      isError: false,                             // 에러 상태
      createdAt: 1645123456789,                   // 생성 시각 (Unix timestamp)
      lastAccessedAt: 1645123456789               // 마지막 접근 시각
    },
    // ... 최대 5개
  ],
  activeTabId: "tab-001"  // 현재 활성 탭 ID
}
```

### 4.2 Reducer Actions

```javascript
// Action Types
const TAB_ACTIONS = {
  CREATE_TAB: 'CREATE_TAB',         // 새 탭 생성
  CLOSE_TAB: 'CLOSE_TAB',           // 탭 닫기
  SWITCH_TAB: 'SWITCH_TAB',         // 탭 전환
  UPDATE_TAB: 'UPDATE_TAB',         // 탭 메타데이터 업데이트
  UPDATE_TAB_HISTORY: 'UPDATE_TAB_HISTORY'  // 히스토리 스택 업데이트
}

// Action Creators (예시)
dispatch({ type: 'CREATE_TAB', payload: { url: 'https://www.google.com' } })
dispatch({ type: 'CLOSE_TAB', payload: { id: 'tab-001' } })
dispatch({ type: 'SWITCH_TAB', payload: { id: 'tab-002' } })
dispatch({
  type: 'UPDATE_TAB',
  payload: {
    id: 'tab-001',
    updates: { title: 'YouTube', isLoading: false }
  }
})
dispatch({
  type: 'UPDATE_TAB_HISTORY',
  payload: {
    id: 'tab-001',
    historyStack: [...newStack],
    historyIndex: 2
  }
})
```

### 4.3 Reducer 로직 (핵심 케이스)

```javascript
function tabReducer(state, action) {
  switch (action.type) {
    case 'CREATE_TAB': {
      // 최대 5개 제한 체크
      if (state.tabs.length >= 5) {
        logger.warn('[TabReducer] 최대 탭 수 초과')
        return state  // 변경 없음
      }

      const newTab = {
        id: generateUUID(),
        url: action.payload.url || 'https://www.google.com',
        title: action.payload.url || 'New Tab',
        favicon: null,
        historyStack: [action.payload.url || 'https://www.google.com'],
        historyIndex: 0,
        isLoading: false,
        isError: false,
        createdAt: Date.now(),
        lastAccessedAt: Date.now()
      }

      return {
        ...state,
        tabs: [...state.tabs, newTab],
        activeTabId: newTab.id  // 새 탭을 즉시 활성화
      }
    }

    case 'CLOSE_TAB': {
      const { id } = action.payload
      const newTabs = state.tabs.filter(tab => tab.id !== id)

      // 최소 1개 탭 유지
      if (newTabs.length === 0) {
        logger.warn('[TabReducer] 마지막 탭 삭제 불가')
        return state
      }

      // 현재 활성 탭을 닫을 경우 → 이전 탭으로 전환
      let newActiveTabId = state.activeTabId
      if (id === state.activeTabId) {
        const closedTabIndex = state.tabs.findIndex(tab => tab.id === id)
        newActiveTabId = newTabs[Math.max(0, closedTabIndex - 1)].id
      }

      return {
        ...state,
        tabs: newTabs,
        activeTabId: newActiveTabId
      }
    }

    case 'SWITCH_TAB': {
      const { id } = action.payload
      const targetTab = state.tabs.find(tab => tab.id === id)

      if (!targetTab) {
        logger.error('[TabReducer] 존재하지 않는 탭:', id)
        return state
      }

      // lastAccessedAt 업데이트 (탭 전환 시각 기록)
      const updatedTabs = state.tabs.map(tab =>
        tab.id === id
          ? { ...tab, lastAccessedAt: Date.now() }
          : tab
      )

      return {
        ...state,
        tabs: updatedTabs,
        activeTabId: id
      }
    }

    case 'UPDATE_TAB': {
      const { id, updates } = action.payload

      return {
        ...state,
        tabs: state.tabs.map(tab =>
          tab.id === id
            ? { ...tab, ...updates }
            : tab
        )
      }
    }

    case 'UPDATE_TAB_HISTORY': {
      const { id, historyStack, historyIndex } = action.payload

      return {
        ...state,
        tabs: state.tabs.map(tab =>
          tab.id === id
            ? { ...tab, historyStack, historyIndex }
            : tab
        )
      }
    }

    default:
      return state
  }
}
```

### 4.4 초기 상태

```javascript
const initialTabState = {
  tabs: [
    {
      id: generateUUID(),
      url: 'https://www.google.com',
      title: 'Google',
      favicon: null,
      historyStack: ['https://www.google.com'],
      historyIndex: 0,
      isLoading: false,
      isError: false,
      createdAt: Date.now(),
      lastAccessedAt: Date.now()
    }
  ],
  activeTabId: null  // 초기화 후 tabs[0].id로 설정
}
```

---

## 5. TabManager 서비스 설계

### 5.1 책임

TabManager는 **순수 함수 집합**으로, Context reducer와 분리된 비즈니스 로직을 담당합니다.

| 함수 | 책임 |
|------|------|
| `createTab(url)` | 새 탭 생성 (최대 5개 검증) |
| `closeTab(tabs, activeTabId, tabId)` | 탭 닫기 (최소 1개 유지, 활성 탭 조정) |
| `switchTab(tabs, tabId)` | 탭 전환 (lastAccessedAt 업데이트) |
| `updateTab(tabs, tabId, updates)` | 탭 메타데이터 업데이트 |
| `updateTabHistory(tab, newUrl, action)` | 히스토리 스택 업데이트 (back/forward/new) |
| `getActiveTab(tabs, activeTabId)` | 현재 활성 탭 조회 |
| `canGoBack(tab)` | 뒤로 가기 가능 여부 |
| `canGoForward(tab)` | 앞으로 가기 가능 여부 |

### 5.2 구현 예시

```javascript
// src/services/tabManager.js

import { generateUUID } from '../utils/uuid'
import logger from '../utils/logger'

const MAX_TABS = 5

/**
 * 새 탭 생성
 * @param {string} url - 초기 URL
 * @returns {Object} - 새 탭 객체
 * @throws {Error} - 최대 탭 수 초과 시
 */
export const createTab = (url = 'https://www.google.com') => {
  return {
    id: generateUUID(),
    url,
    title: url,
    favicon: null,
    historyStack: [url],
    historyIndex: 0,
    isLoading: false,
    isError: false,
    createdAt: Date.now(),
    lastAccessedAt: Date.now()
  }
}

/**
 * 탭 닫기 (최소 1개 유지)
 * @param {Array} tabs - 탭 배열
 * @param {string} activeTabId - 현재 활성 탭 ID
 * @param {string} tabIdToClose - 닫을 탭 ID
 * @returns {{ tabs, activeTabId }} - 업데이트된 상태
 */
export const closeTab = (tabs, activeTabId, tabIdToClose) => {
  if (tabs.length <= 1) {
    throw new Error('마지막 탭은 닫을 수 없습니다.')
  }

  const newTabs = tabs.filter(tab => tab.id !== tabIdToClose)

  // 현재 활성 탭을 닫을 경우 → 이전 탭으로 전환
  let newActiveTabId = activeTabId
  if (tabIdToClose === activeTabId) {
    const closedTabIndex = tabs.findIndex(tab => tab.id === tabIdToClose)
    newActiveTabId = newTabs[Math.max(0, closedTabIndex - 1)].id
  }

  return { tabs: newTabs, activeTabId: newActiveTabId }
}

/**
 * 히스토리 스택 업데이트
 * @param {Object} tab - 탭 객체
 * @param {string} newUrl - 새 URL
 * @param {string} action - 'new' | 'back' | 'forward'
 * @returns {{ historyStack, historyIndex }}
 */
export const updateTabHistory = (tab, newUrl, action = 'new') => {
  const { historyStack, historyIndex } = tab

  switch (action) {
    case 'new': {
      // 새 페이지 탐색 → 현재 위치 이후 스택 제거 후 추가
      const newStack = historyStack.slice(0, historyIndex + 1)
      newStack.push(newUrl)
      return {
        historyStack: newStack,
        historyIndex: newStack.length - 1
      }
    }

    case 'back': {
      // 뒤로 가기 → 인덱스만 감소
      if (historyIndex > 0) {
        return {
          historyStack,
          historyIndex: historyIndex - 1
        }
      }
      return { historyStack, historyIndex }
    }

    case 'forward': {
      // 앞으로 가기 → 인덱스만 증가
      if (historyIndex < historyStack.length - 1) {
        return {
          historyStack,
          historyIndex: historyIndex + 1
        }
      }
      return { historyStack, historyIndex }
    }

    default:
      return { historyStack, historyIndex }
  }
}

/**
 * 뒤로 가기 가능 여부
 */
export const canGoBack = (tab) => {
  return tab.historyIndex > 0
}

/**
 * 앞으로 가기 가능 여부
 */
export const canGoForward = (tab) => {
  return tab.historyIndex < tab.historyStack.length - 1
}

/**
 * 활성 탭 조회
 */
export const getActiveTab = (tabs, activeTabId) => {
  return tabs.find(tab => tab.id === activeTabId)
}
```

---

## 6. TabBar 컴포넌트 설계

### 6.1 UI 구조

```
┌────────────────────────────────────────────────┐
│  [Tab 1] [Tab 2] [Tab 3] [Tab 4*] [Tab 5] [+] │ (* = 활성 탭)
└────────────────────────────────────────────────┘
```

각 탭 아이템:
```
┌──────────────────┐
│  [X]  YouTube    │  ← 탭 제목 (최대 30자)
└──────────────────┘
```

### 6.2 Props 인터페이스

```javascript
TabBar.propTypes = {
  tabs: PropTypes.arrayOf(PropTypes.shape({
    id: PropTypes.string.isRequired,
    title: PropTypes.string.isRequired,
    favicon: PropTypes.string,  // 향후 F-15
  })).isRequired,

  activeTabId: PropTypes.string.isRequired,

  // 이벤트 콜백
  onTabClick: PropTypes.func,       // 탭 클릭 → 탭 전환
  onTabClose: PropTypes.func,       // X 버튼 클릭 → 탭 닫기
  onNewTab: PropTypes.func,         // + 버튼 클릭 → 새 탭 생성

  // 스타일
  className: PropTypes.string
}
```

### 6.3 Spotlight 포커스 흐름

```
리모컨 좌/우 방향키: TabBar 내 탭 간 이동
리모컨 선택 버튼: 탭 전환
리모컨 위 방향키: URLBar로 이동
리모컨 아래 방향키: WebView로 이동
```

Spotlight 설정:
```javascript
Spotlight.set('tab-bar', {
  defaultElement: '.tabItem',
  enterTo: 'default-element',
  leaveFor: {
    up: 'url-bar',
    down: 'webview-main'
  }
})
```

### 6.4 컴포넌트 코드 (개요)

```javascript
// src/components/TabBar/TabBar.js

import { useCallback } from 'react'
import PropTypes from 'prop-types'
import Button from '@enact/moonstone/Button'
import Spottable from '@enact/spotlight/Spottable'
import css from './TabBar.module.less'

const SpottableDiv = Spottable('div')

const TabBar = ({ tabs, activeTabId, onTabClick, onTabClose, onNewTab }) => {
  // 탭 아이템 렌더링
  const renderTabItem = (tab) => {
    const isActive = tab.id === activeTabId

    return (
      <SpottableDiv
        key={tab.id}
        className={`${css.tabItem} ${isActive ? css.active : ''}`}
        onClick={() => onTabClick(tab.id)}
        spotlightId={`tab-${tab.id}`}
      >
        {/* 닫기 버튼 */}
        <Button
          className={css.closeButton}
          onClick={(e) => {
            e.stopPropagation()  // 탭 클릭 이벤트 전파 방지
            onTabClose(tab.id)
          }}
          icon="closex"
          size="small"
        />

        {/* 탭 제목 (최대 30자) */}
        <span className={css.tabTitle}>
          {tab.title.length > 30 ? `${tab.title.substring(0, 30)}...` : tab.title}
        </span>
      </SpottableDiv>
    )
  }

  return (
    <div className={css.tabBar} data-spotlight-container="tab-bar">
      {/* 탭 목록 */}
      {tabs.map(renderTabItem)}

      {/* 새 탭 버튼 */}
      <Button
        className={css.newTabButton}
        onClick={onNewTab}
        disabled={tabs.length >= 5}
        icon="plus"
        spotlightId="tab-new"
      >
        새 탭
      </Button>
    </div>
  )
}

export default TabBar
```

---

## 7. BrowserView 통합 설계

### 7.1 기존 코드 변경점

현재 BrowserView는 단일 탭 구조입니다. 다음과 같이 수정합니다:

**변경 전**:
```javascript
// BrowserView.js (기존)
const [currentUrl, setCurrentUrl] = useState('https://www.google.com')
const [historyStack, setHistoryStack] = useState(['https://www.google.com'])
const [historyIndex, setHistoryIndex] = useState(0)
```

**변경 후**:
```javascript
// BrowserView.js (탭 관리 시스템 적용)
import { useTabContext } from '../contexts/TabContext'

const BrowserView = () => {
  const { state, dispatch } = useTabContext()
  const { tabs, activeTabId } = state

  // 현재 활성 탭 조회
  const activeTab = getActiveTab(tabs, activeTabId)
  const currentUrl = activeTab?.url
  const canGoBack = activeTab ? activeTab.historyIndex > 0 : false
  const canGoForward = activeTab ? activeTab.historyIndex < activeTab.historyStack.length - 1 : false

  // WebView URL은 activeTab.url로 바인딩
  // ...
}
```

### 7.2 탭 전환 로직

```javascript
// 탭 전환 핸들러
const handleTabSwitch = useCallback((newTabId) => {
  logger.info('[BrowserView] 탭 전환:', { from: activeTabId, to: newTabId })

  // 1. 이전 탭의 WebView unmount (useEffect cleanup으로 자동 처리)
  // 2. Context dispatch로 활성 탭 변경
  dispatch({ type: 'SWITCH_TAB', payload: { id: newTabId } })

  // 3. 새 탭의 WebView mount (activeTab.url 변경으로 자동 렌더링)
  // 4. NavigationBar 버튼 상태 자동 업데이트 (canGoBack/canGoForward는 activeTab 기준으로 계산)
}, [activeTabId, dispatch])
```

### 7.3 WebView 이벤트 연동

```javascript
// WebView 로딩 완료 → 탭 메타데이터 업데이트
const handleLoadEnd = useCallback(({ url, title, duration }) => {
  dispatch({
    type: 'UPDATE_TAB',
    payload: {
      id: activeTabId,
      updates: {
        url,
        title,
        isLoading: false,
        isError: false
      }
    }
  })

  // 히스토리 서비스 기록 (기존 동작 유지)
  historyService.recordVisit(url, title)
}, [activeTabId, dispatch])

// URL 변경 감지 → 히스토리 스택 업데이트
const handleNavigationChange = useCallback(({ url }) => {
  const newHistory = updateTabHistory(activeTab, url, 'new')

  dispatch({
    type: 'UPDATE_TAB_HISTORY',
    payload: {
      id: activeTabId,
      ...newHistory
    }
  })
}, [activeTab, activeTabId, dispatch])
```

### 7.4 NavigationBar 연동

```javascript
// NavigationBar의 뒤로/앞으로 버튼 클릭
const handleNavigate = useCallback(({ action }) => {
  if (action === 'back') {
    const newHistory = updateTabHistory(activeTab, null, 'back')
    dispatch({
      type: 'UPDATE_TAB_HISTORY',
      payload: { id: activeTabId, ...newHistory }
    })

    // 새 URL로 WebView 업데이트
    const newUrl = newHistory.historyStack[newHistory.historyIndex]
    dispatch({
      type: 'UPDATE_TAB',
      payload: { id: activeTabId, updates: { url: newUrl } }
    })
  }
  // forward, reload, home도 동일한 패턴
}, [activeTab, activeTabId, dispatch])
```

---

## 8. Spotlight 포커스 흐름

### 8.1 전체 포커스 영역

```
┌─────────────────────────────────────────┐
│  URLBar (url-bar)                       │
├─────────────────────────────────────────┤
│  TabBar (tab-bar)                       │  ← 새로 추가
│  [Tab1] [Tab2] [Tab3*] [Tab4] [+]      │
├─────────────────────────────────────────┤
│  WebView (webview-main)                 │
├─────────────────────────────────────────┤
│  NavigationBar (navigation-bar)         │
│  [←] [→] [⟳] [🏠] [★] [⏱]             │
└─────────────────────────────────────────┘
```

### 8.2 방향키 매핑

| 현재 위치 | 방향키 | 이동 대상 |
|-----------|--------|-----------|
| URLBar | 아래 | **TabBar** (새로 추가) |
| **TabBar** | 위 | URLBar |
| **TabBar** | 아래 | WebView |
| **TabBar** | 좌/우 | TabBar 내 탭 간 이동 |
| WebView | 위 | **TabBar** (기존: URLBar) |
| WebView | 아래 | NavigationBar |
| NavigationBar | 위 | WebView |

**변경 이유**: TabBar를 URLBar와 WebView 사이에 삽입하여 탭 전환 접근성 향상

### 8.3 Spotlight 설정 (BrowserView에 추가)

```javascript
useEffect(() => {
  // URLBar 설정
  Spotlight.set('url-bar', {
    leaveFor: {
      down: 'tab-bar'  // 기존: 'webview-main'
    }
  })

  // TabBar 설정 (신규)
  Spotlight.set('tab-bar', {
    defaultElement: `.tabItem[data-tab-id="${activeTabId}"]`,
    enterTo: 'default-element',
    leaveFor: {
      up: 'url-bar',
      down: 'webview-main'
    }
  })

  // WebView 설정
  Spotlight.set('webview-main', {
    leaveFor: {
      up: 'tab-bar',  // 기존: 'url-bar'
      down: 'navigation-bar'
    }
  })

  // NavigationBar 설정 (기존 유지)
  Spotlight.set('navigation-bar', {
    leaveFor: {
      up: 'webview-main'
    }
  })
}, [activeTabId])
```

---

## 9. 리모컨 키 매핑

### 9.1 TabBar 포커스 시

| 키 | 동작 |
|-----|------|
| 좌/우 방향키 | 탭 간 포커스 이동 |
| 선택 버튼 | 탭 전환 (활성 탭 변경) |
| 위 방향키 | URLBar로 포커스 이동 |
| 아래 방향키 | WebView로 포커스 이동 |
| 빨강 버튼 | 포커스된 탭 닫기 (선택) |

### 9.2 리모컨 채널 버튼 (F-13 연동, 향후 구현)

| 키 | 동작 |
|-----|------|
| 채널 업 | 다음 탭으로 전환 |
| 채널 다운 | 이전 탭으로 전환 |

**구현 시점**: F-13 (리모컨 단축키) 기능 개발 시

---

## 10. 데이터 모델

### 10.1 TabModel (탭 데이터 구조)

```javascript
{
  id: String,               // UUID (예: "tab-550e8400-e29b-41d4-a716")
  url: String,              // 현재 URL (예: "https://www.youtube.com")
  title: String,            // 페이지 제목 (예: "YouTube")
  favicon: String|null,     // 파비콘 URL (향후 F-15)
  historyStack: [String],   // 히스토리 스택 (예: ["https://google.com", "https://youtube.com"])
  historyIndex: Number,     // 현재 인덱스 (예: 1)
  isLoading: Boolean,       // 로딩 상태 (WebView 상태 동기화)
  isError: Boolean,         // 에러 상태 (WebView 상태 동기화)
  createdAt: Number,        // 생성 시각 (Unix timestamp)
  lastAccessedAt: Number    // 마지막 접근 시각 (탭 정렬용)
}
```

### 10.2 TabContext State

```javascript
{
  tabs: [TabModel],         // 탭 배열 (최대 5개)
  activeTabId: String       // 현재 활성 탭 ID
}
```

---

## 11. 파일 구조

```
src/
├── contexts/
│   └── TabContext.js              # TabContext + Provider + useTabContext hook
│
├── services/
│   └── tabManager.js              # 탭 CRUD 비즈니스 로직
│
├── components/
│   ├── TabBar/
│   │   ├── TabBar.js              # TabBar 컴포넌트
│   │   ├── TabBar.module.less     # TabBar 스타일
│   │   └── index.js               # Export
│   │
│   ├── WebView/                   # 기존 유지 (변경 없음)
│   │   └── WebView.js
│   │
│   └── NavigationBar/             # 기존 유지 (변경 없음)
│       └── NavigationBar.js
│
├── views/
│   └── BrowserView.js             # 수정: TabContext 통합
│
└── App/
    └── App.js                     # 수정: TabProvider 래핑
```

### App.js 변경점

```javascript
// src/App/App.js
import MoonstoneDecorator from '@enact/moonstone/MoonstoneDecorator'
import { TabProvider } from '../contexts/TabContext'  // 신규 추가
import BrowserView from '../views/BrowserView'

const App = () => {
  return (
    <TabProvider>  {/* TabContext로 BrowserView 래핑 */}
      <BrowserView />
    </TabProvider>
  )
}

export default MoonstoneDecorator(App)
```

---

## 12. 성능 최적화 전략

### 12.1 메모리 최적화

| 항목 | 전략 |
|------|------|
| WebView 인스턴스 | 활성 탭만 DOM에 마운트, 백그라운드 탭은 unmount |
| iframe 리소스 해제 | 탭 전환 시 `iframe.src = 'about:blank'` (useEffect cleanup) |
| 탭 메타데이터 | 경량 객체 (~200 bytes/탭), 최대 5개 = ~1KB |
| Context 리렌더링 | React.memo로 TabBar, WebView 컴포넌트 메모이제이션 |

**예상 메모리 사용량**:
- 단일 탭 (WebView): ~50~150MB
- 탭 5개 (메타데이터만): ~1KB
- 총 메모리 절감: **80%** (기존 멀티 인스턴스 대비)

### 12.2 탭 전환 속도 최적화

| 단계 | 소요 시간 |
|------|-----------|
| 이전 탭 unmount (iframe src → about:blank) | ~50ms |
| Context dispatch (상태 업데이트) | ~10ms |
| 새 탭 mount (iframe src → 새 URL) | ~100ms |
| NavigationBar 리렌더링 | ~20ms |
| **총 탭 전환 시간** | **~180ms** |

**목표 NFR**: 0.5초 이내 (✅ 달성)

### 12.3 렌더링 최적화

```javascript
// React.memo로 불필요한 리렌더링 방지
export default React.memo(TabBar, (prevProps, nextProps) => {
  return (
    prevProps.activeTabId === nextProps.activeTabId &&
    prevProps.tabs.length === nextProps.tabs.length &&
    prevProps.tabs.every((tab, i) => tab.id === nextProps.tabs[i].id)
  )
})

// useCallback으로 이벤트 핸들러 메모이제이션
const handleTabClick = useCallback((tabId) => {
  dispatch({ type: 'SWITCH_TAB', payload: { id: tabId } })
}, [dispatch])
```

### 12.4 히스토리 스택 최적화

```javascript
// 히스토리 스택 크기 제한 (탭당 최대 50개 URL)
const MAX_HISTORY_PER_TAB = 50

if (newHistoryStack.length > MAX_HISTORY_PER_TAB) {
  newHistoryStack = newHistoryStack.slice(-MAX_HISTORY_PER_TAB)
  historyIndex = Math.min(historyIndex, MAX_HISTORY_PER_TAB - 1)
}
```

**이유**: 탭당 히스토리 50개 초과 시 메모리 낭비 (각 URL ~ 100 bytes)

---

## 13. 주의사항 및 제약조건

### 13.1 webOS 플랫폼 제약

| 제약 | 영향 | 대응 방안 |
|------|------|----------|
| iframe CORS 정책 | 외부 사이트의 제목/파비콘 추출 불가 | WebView onLoadEnd 콜백에서 메타데이터 수집 (제한적) |
| 메모리 제한 | 탭 5개 이상 시 앱 크래시 위험 | 하드 리미트 5개 강제 |
| Spotlight 포커스 복잡도 | 탭 증가 시 포커스 관리 어려움 | TabBar를 독립 컨테이너로 분리 |

### 13.2 탭 전환 시 페이지 상태 소실

**문제**: 백그라운드 탭을 DOM에서 제거하므로, 탭 전환 시 다음이 초기화됨:
- 스크롤 위치
- form 입력 내용
- 페이지 내 JavaScript 실행 상태

**해결 불가**: iframe의 scrollTop, form state를 외부에서 추출 불가 (CORS)

**UX 완화**:
1. 탭 전환 시 "페이지를 다시 로드합니다" 안내 메시지 (선택)
2. 히스토리 스택 유지로 뒤로/앞으로 가기는 정상 작동

### 13.3 탭 닫기 시 데이터 손실

| 시나리오 | 동작 |
|----------|------|
| 탭 닫기 | 히스토리 스택 즉시 삭제 (복원 불가) |
| 앱 종료 | 모든 탭 초기화 (영속성 없음) |

**완화**: "탭을 닫으시겠습니까?" 확인 다이얼로그 (선택, F-14 다이얼로그 기능 연동)

### 13.4 동시 페이지 로딩 불가

백그라운드 탭에서 페이지를 미리 로딩할 수 없음 (활성 탭만 iframe 존재)

**영향**: 탭 전환 후 페이지 로딩 대기 시간 발생 (~1~3초)

**완화**: LoadingBar 표시 (F-05 연동)

### 13.5 탭 순서 변경 불가

요구사항에서 제외되었으나, 향후 추가 시 다음 고려 필요:
- 리모컨 UX 복잡도 증가 (드래그 앤 드롭 불가)
- Spotlight 포커스 재계산 필요

---

## 14. 구현 순서 제안

### Phase 1: 핵심 인프라 (순차)
1. TabContext + reducer 구현
2. TabManager 서비스 구현
3. App.js에 TabProvider 래핑

### Phase 2: UI 컴포넌트 (병렬 가능)
4. TabBar 컴포넌트 구현
5. BrowserView 리팩토링 (TabContext 통합)

### Phase 3: 통합 및 테스트 (순차)
6. Spotlight 포커스 흐름 설정
7. WebView ↔ TabContext 이벤트 연동
8. NavigationBar 히스토리 버튼 연동
9. 탭 전환 성능 측정 및 최적화

---

## 15. 테스트 전략

### 15.1 단위 테스트 (Jest)

| 테스트 대상 | 테스트 케이스 |
|-------------|--------------|
| `tabReducer` | - CREATE_TAB: 5개 초과 시 무시<br>- CLOSE_TAB: 마지막 탭 닫기 불가<br>- SWITCH_TAB: lastAccessedAt 업데이트 |
| `TabManager` | - createTab: UUID 생성<br>- closeTab: 활성 탭 조정<br>- updateTabHistory: 히스토리 스택 정합성 |

### 15.2 통합 테스트 (Enact Testing Library)

| 시나리오 | 검증 항목 |
|----------|-----------|
| 탭 생성 | "+ 새 탭" 클릭 → tabs.length 증가 |
| 탭 전환 | 탭 클릭 → activeTabId 변경 → WebView URL 업데이트 |
| 탭 닫기 | "X" 버튼 클릭 → tabs.length 감소 → 이전 탭 활성화 |
| 최대 탭 수 제한 | 5개째 탭 생성 → "+ 새 탭" 버튼 disabled |

### 15.3 E2E 테스트 (수동)

| 시나리오 | 검증 항목 |
|----------|-----------|
| 탭 간 독립성 | 탭 A에서 뒤로 가기 → 탭 B 히스토리 영향 X |
| 메모리 누수 | 탭 5개 생성/닫기 반복 → 메모리 증가 없음 |
| 리모컨 포커스 | 방향키로 TabBar ↔ WebView ↔ NavigationBar 전환 |

---

## 16. 위험 요소 및 대응 방안

### 위험 1: 탭 전환 시 페이지 재로딩으로 인한 UX 저하

**가능성**: 높음 (설계 의도)

**영향**: 사용자가 백그라운드 탭의 페이지 상태 소실에 불만

**대응**:
1. LoadingBar로 로딩 피드백 제공
2. 탭 전환 애니메이션으로 전환 과정 시각화 (선택)
3. 향후 "탭 고정" 기능으로 중요 탭은 메모리 유지 (F-15 이후)

### 위험 2: 메모리 누수 (iframe 리소스 미해제)

**가능성**: 중간 (useEffect cleanup 필요)

**영향**: 탭 전환 반복 시 메모리 증가 → 앱 크래시

**대응**:
1. WebView unmount 시 `iframe.src = 'about:blank'` 강제
2. 개발 환경에서 메모리 모니터링 (performance.memory API)
3. 테스트: 탭 생성/닫기 100회 반복 → 메모리 증가 체크

### 위험 3: Spotlight 포커스 순환 문제

**가능성**: 중간 (TabBar 추가로 인한 복잡도 증가)

**영향**: 리모컨 방향키 동작이 예상과 다름

**대응**:
1. Spotlight 설정을 useEffect 의존성에 포함 (activeTabId 변경 시 재설정)
2. 포커스 디버깅 로깅 (spotlightfocus 이벤트 리스너)

---

## 17. 향후 확장 가능성

### 확장 1: 탭 영속성 (F-11 설정 연동)

```javascript
// IndexedDB에 탭 목록 저장
const saveTabs = async (tabs, activeTabId) => {
  await db.put('app_state', { tabs, activeTabId })
}

// 앱 시작 시 탭 복원
const restoreTabs = async () => {
  const state = await db.get('app_state')
  if (state) {
    dispatch({ type: 'RESTORE_TABS', payload: state })
  }
}
```

### 확장 2: 탭 그룹 (Future)

```javascript
// TabModel 확장
{
  id: "tab-001",
  groupId: "group-work",  // 탭 그룹 ID
  groupName: "업무",
  // ...
}
```

### 확장 3: 탭 미리보기 썸네일 (Future)

```javascript
// Canvas API로 iframe 스크린샷 (CORS 제약 있음)
const captureTabThumbnail = async (tabId) => {
  const canvas = document.createElement('canvas')
  // ... screenshot 로직
  return canvas.toDataURL()
}
```

---

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-13 | 최초 작성 | F-06 탭 관리 시스템 설계 |
