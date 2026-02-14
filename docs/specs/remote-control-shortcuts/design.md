# F-13: 리모컨 단축키 — 기술 설계서

## 1. 참조
- 요구사항 분석서: docs/specs/remote-control-shortcuts/requirements.md

---

## 2. 아키텍처 결정

### 결정 1: 단축키 처리 방식 (Hook 확장 vs 별도 서비스)
- **선택지**:
  - A) useRemoteControl Hook 확장 (현재 구현 패턴 유지)
  - B) KeyboardHandler 서비스 분리 (중앙화된 키 매핑 관리)
  - C) Hybrid 방식 (useRemoteControl + keyCodeConstants.js)
- **결정**: C) Hybrid 방식
- **근거**:
  - useRemoteControl Hook은 이미 Back 키, Yellow 버튼을 처리하고 있어 기존 패턴 유지가 자연스러움
  - 키 코드 상수를 별도 파일(keyCodeConstants.js)로 분리하여 확장성 확보
  - 키 매핑 로직은 Hook 내부에 유지하되, BrowserView에서 콜백을 주입하여 유연성 확보
  - webOS 기기별 keyCode 차이(405 vs 406)를 상수 파일에서 일괄 관리
- **트레이드오프**:
  - 포기한 것: 별도 서비스 계층의 중앙화된 이벤트 버스 패턴
  - 얻은 것: 기존 코드 패턴 유지, 컴포넌트 간 의존성 최소화, Hook 재사용성

### 결정 2: 키보드 열릴 때 단축키 비활성화 메커니즘
- **선택지**:
  - A) useRemoteControl 내부에서 showKeyboard 상태를 props로 받아 분기 처리
  - B) BrowserView에서 keyDown 이벤트를 먼저 가로채서 stopPropagation()
  - C) useRemoteControl에서 event.target을 체크하여 Input 요소일 때 무시
- **결정**: A) useRemoteControl 내부 분기 처리
- **근거**:
  - URLBar의 키보드 상태는 BrowserView에서 이미 showKeyboard로 관리 중
  - Hook 옵션에 isKeyboardOpen을 추가하여 키보드 열린 상태를 전달
  - 키보드가 열린 상태에서는 숫자 키, 채널 버튼 등 일부 단축키만 비활성화
  - Back 키, 컬러 버튼은 키보드가 열려있어도 동작 (기존 동작 유지)
- **트레이드오프**:
  - 포기한 것: 완전히 독립적인 Hook (외부 상태 의존 없음)
  - 얻은 것: 명확한 키보드 우선순위, 기존 Back 키 동작 유지, 간단한 구현

### 결정 3: 탭 전환 시 포커스 관리
- **선택지**:
  - A) 탭 전환 후 항상 WebView에 포커스
  - B) 이전 포커스 위치를 기억하여 복원
  - C) 탭별로 마지막 포커스 위치를 저장
- **결정**: A) 항상 WebView에 포커스
- **근거**:
  - 요구사항 명시: "탭 전환 시 자동으로 WebView에 포커스"
  - 사용자 시나리오: 채널 버튼으로 탭 전환 → 즉시 웹 페이지 탐색
  - TabContext에 포커스 위치까지 저장하면 상태 복잡도 증가
  - BrowserView의 기존 handleTabClick 패턴과 일치
- **트레이드오프**:
  - 포기한 것: 탭별 독립적인 포커스 상태 복원
  - 얻은 것: 단순하고 예측 가능한 UX, 구현 복잡도 감소

### 결정 4: Yellow 버튼 충돌 해결 방안
- **선택지**:
  - A) Yellow → 북마크 추가 (features.md 원안)
  - B) Yellow → 다운로드 패널 열기 (현재 구현 유지)
  - C) Green → 히스토리 패널 (Yellow는 다운로드 유지)
- **결정**: C) Green → 히스토리 패널
- **근거**:
  - Yellow 버튼은 F-12(다운로드) 기능에서 이미 구현되어 운영 중
  - 기존 사용자 경험을 유지하는 것이 우선순위 높음
  - 컬러 버튼 매핑: Red(북마크) / Green(히스토리) / Yellow(다운로드) / Blue(새 탭)
  - 북마크 추가는 NavigationBar의 버튼으로 충분히 접근 가능
- **트레이드오프**:
  - 포기한 것: features.md의 원래 Yellow 키 매핑 (북마크 추가)
  - 얻은 것: 기존 기능 유지, 구현 일관성, 사용자 혼란 최소화

### 결정 5: 스크롤 제어 기능 (FR-4) 구현 여부
- **선택지**:
  - A) M2 마일스톤에 포함 (우선순위 Could)
  - B) M3 이후로 연기
  - C) 구현하지 않음
- **결정**: B) M3 이후로 연기
- **근거**:
  - 우선순위 Could (선택적 기능)
  - CORS 정책으로 일부 사이트에서 동작하지 않아 UX 혼란 가능성
  - 탭 전환, 컬러 버튼 단축키(Must/Should)가 더 핵심적
  - iframe contentWindow 접근이 제한된 경우 에러 핸들링 복잡도 증가
- **트레이드오프**:
  - 포기한 것: 자동 스크롤 편의 기능 (긴 문서 읽기)
  - 얻은 것: M2 개발 범위 축소, 안정성 향상, 핵심 기능 집중

---

## 3. 컴포넌트 설계

### useRemoteControl Hook 확장

#### 확장된 옵션 인터페이스
```javascript
useRemoteControl({
  // 기존 옵션 (F-04, F-12에서 사용 중)
  isWebViewFocused: boolean,
  onBackInWebView: Function,
  onBackOutsideWebView: Function,
  onYellowButton: Function,  // 다운로드 패널 열기

  // F-13 추가 옵션
  isKeyboardOpen: boolean,  // URLBar 키보드 열린 상태

  // 채널 버튼 (FR-1)
  onChannelUp: Function,    // 이전 탭으로 전환
  onChannelDown: Function,  // 다음 탭으로 전환

  // 컬러 버튼 (FR-2)
  onRedButton: Function,    // 북마크 패널 열기
  onGreenButton: Function,  // 히스토리 패널 열기
  onBlueButton: Function,   // 새 탭 생성

  // 숫자 키 (FR-3)
  onNumberKey: Function,    // (number) => void, 1~5 탭 선택

  // 설정 버튼 (FR-5)
  onMenuButton: Function    // 설정 패널 열기
})
```

#### 키 이벤트 처리 흐름
```
window.keydown 이벤트
  │
  ├─ isKeyboardOpen === true
  │   ├─ 숫자 키 (49~53) → 무시 (키보드 입력 우선)
  │   ├─ 채널 버튼 (427, 428) → 무시
  │   └─ 컬러 버튼, 설정 버튼 → 허용 (패널 열기 가능)
  │
  ├─ isKeyboardOpen === false
  │   ├─ Back 키 (8, 461)
  │   │   ├─ isWebViewFocused → onBackInWebView()
  │   │   └─ !isWebViewFocused → onBackOutsideWebView()
  │   │
  │   ├─ 채널 버튼 (427, 428)
  │   │   ├─ Channel Up → onChannelUp()
  │   │   └─ Channel Down → onChannelDown()
  │   │
  │   ├─ 컬러 버튼
  │   │   ├─ Red (403, 404) → onRedButton()
  │   │   ├─ Green (405, 406) → onGreenButton()
  │   │   ├─ Yellow (405, 406) → onYellowButton()  (기존)
  │   │   └─ Blue (407, 408) → onBlueButton()
  │   │
  │   ├─ 숫자 키 (49~53)
  │   │   └─ onNumberKey(number)  // number = 1~5
  │   │
  │   └─ 설정 버튼 (457, 18)
  │       └─ onMenuButton()
  │
  └─ event.preventDefault() 호출 여부
      ├─ 처리된 키: preventDefault()
      └─ 미처리 키: 기본 동작 허용
```

#### 디바운스 처리
```javascript
// 0.5초 이내 중복 입력 방지
const lastKeyTime = useRef({})

const isDebounced = (keyCode, debounceMs = 500) => {
  const now = Date.now()
  const lastTime = lastKeyTime.current[keyCode] || 0

  if (now - lastTime < debounceMs) {
    return true  // 무시
  }

  lastKeyTime.current[keyCode] = now
  return false
}
```

### BrowserView.js 연동 방안

#### useRemoteControl 호출 확장
```javascript
// BrowserView.js (기존)
const [showKeyboard, setShowKeyboard] = useState(false)

// useRemoteControl 확장 호출
useRemoteControl({
  // 기존 옵션
  isWebViewFocused,
  onBackInWebView: handleBackInWebView,
  onBackOutsideWebView: handleBackOutsideWebView,
  onYellowButton: handleDownloadPanelOpen,

  // F-13 추가 옵션
  isKeyboardOpen: showKeyboard,
  onChannelUp: handleChannelUp,
  onChannelDown: handleChannelDown,
  onRedButton: handleBookmarkClick,  // 기존 함수 재사용
  onGreenButton: handleHistoryClick,  // 기존 함수 재사용
  onBlueButton: handleNewTab,         // 기존 함수 재사용
  onNumberKey: handleNumberKey,
  onMenuButton: handleSettingsClick   // 기존 함수 재사용
})
```

#### 추가 핸들러 함수 (BrowserView.js)

##### 채널 버튼 핸들러 (탭 전환)
```javascript
/**
 * 채널 Up 버튼 핸들러 (이전 탭으로 전환)
 */
const handleChannelUp = useCallback(() => {
  if (tabs.length <= 1) return  // 탭 1개일 때 무시

  const currentIndex = tabs.findIndex(tab => tab.id === activeTabId)
  const prevIndex = (currentIndex - 1 + tabs.length) % tabs.length  // 순환
  const prevTabId = tabs[prevIndex].id

  logger.info('[BrowserView] Channel Up → 이전 탭 전환:', {
    from: activeTabId,
    to: prevTabId
  })

  // 탭 전환
  dispatch({ type: TAB_ACTIONS.SWITCH_TAB, payload: { id: prevTabId } })

  // URL 입력창 업데이트
  const prevTab = tabs[prevIndex]
  setInputValue(prevTab.url)

  // WebView 포커스
  Spotlight.focus('webview-main')
}, [tabs, activeTabId, dispatch])

/**
 * 채널 Down 버튼 핸들러 (다음 탭으로 전환)
 */
const handleChannelDown = useCallback(() => {
  if (tabs.length <= 1) return  // 탭 1개일 때 무시

  const currentIndex = tabs.findIndex(tab => tab.id === activeTabId)
  const nextIndex = (currentIndex + 1) % tabs.length  // 순환
  const nextTabId = tabs[nextIndex].id

  logger.info('[BrowserView] Channel Down → 다음 탭 전환:', {
    from: activeTabId,
    to: nextTabId
  })

  dispatch({ type: TAB_ACTIONS.SWITCH_TAB, payload: { id: nextTabId } })

  const nextTab = tabs[nextIndex]
  setInputValue(nextTab.url)

  Spotlight.focus('webview-main')
}, [tabs, activeTabId, dispatch])
```

##### 숫자 키 핸들러 (직접 탭 선택)
```javascript
/**
 * 숫자 키 핸들러 (1~5로 직접 탭 선택)
 * @param {number} number - 입력된 숫자 (1~5)
 */
const handleNumberKey = useCallback((number) => {
  const targetIndex = number - 1  // 1-based → 0-based

  // 탭이 존재하지 않으면 무시
  if (targetIndex < 0 || targetIndex >= tabs.length) {
    logger.debug('[BrowserView] 존재하지 않는 탭 번호:', number)
    return
  }

  const targetTab = tabs[targetIndex]

  // 이미 활성화된 탭이면 무시
  if (targetTab.id === activeTabId) {
    logger.debug('[BrowserView] 이미 활성화된 탭')
    return
  }

  logger.info('[BrowserView] 숫자 키로 탭 선택:', {
    number,
    tabId: targetTab.id
  })

  dispatch({ type: TAB_ACTIONS.SWITCH_TAB, payload: { id: targetTab.id } })
  setInputValue(targetTab.url)
  Spotlight.focus('webview-main')
}, [tabs, activeTabId, dispatch])
```

---

## 4. 상수 관리 설계

### keyCodeConstants.js 파일 구조
```javascript
/**
 * keyCodeConstants.js
 * webOS 리모컨 키 코드 상수 정의
 *
 * webOS 기기별로 keyCode가 다를 수 있으므로 배열로 관리
 * 예: Yellow 버튼은 405 또는 406
 */

// Back 키
export const KEY_BACK = [8, 461]  // Backspace, webOS Back

// 채널 버튼
export const KEY_CHANNEL_UP = [427]
export const KEY_CHANNEL_DOWN = [428]

// 컬러 버튼
export const KEY_RED = [403, 404]
export const KEY_GREEN = [405, 406]  // ⚠️ Yellow와 keyCode 중복 가능
export const KEY_YELLOW = [405, 406]
export const KEY_BLUE = [407, 408]

// 숫자 키 (1~5)
export const KEY_NUMBER_1 = [49]
export const KEY_NUMBER_2 = [50]
export const KEY_NUMBER_3 = [51]
export const KEY_NUMBER_4 = [52]
export const KEY_NUMBER_5 = [53]

// 설정 버튼
export const KEY_MENU = [457, 18]  // Menu/Settings

// 재생 컨트롤 (M3 이후 구현 예정)
export const KEY_PLAY_PAUSE = [415, 19]
export const KEY_FAST_FORWARD = [417]
export const KEY_REWIND = [412]

/**
 * 키 코드 매칭 헬퍼 함수
 * @param {number} keyCode - 이벤트 keyCode
 * @param {number[]} keyCodeArray - 비교할 키 코드 배열
 * @returns {boolean}
 */
export const isKeyMatch = (keyCode, keyCodeArray) => {
  return keyCodeArray.includes(keyCode)
}

/**
 * 숫자 키 추출 (49~53 → 1~5)
 * @param {number} keyCode
 * @returns {number|null} - 1~5 또는 null
 */
export const getNumberFromKeyCode = (keyCode) => {
  if (keyCode >= 49 && keyCode <= 53) {
    return keyCode - 48  // ASCII 49 → 숫자 1
  }
  return null
}
```

### 상수 사용 예시 (useRemoteControl.js)
```javascript
import {
  KEY_BACK,
  KEY_CHANNEL_UP,
  KEY_CHANNEL_DOWN,
  KEY_RED,
  KEY_GREEN,
  KEY_YELLOW,
  KEY_BLUE,
  KEY_MENU,
  isKeyMatch,
  getNumberFromKeyCode
} from '../utils/keyCodeConstants'

const handleKeyDown = (event) => {
  const { keyCode } = event

  // Back 키 감지
  if (isKeyMatch(keyCode, KEY_BACK)) {
    // 기존 로직
  }

  // 채널 버튼 감지
  if (!isKeyboardOpen) {
    if (isKeyMatch(keyCode, KEY_CHANNEL_UP)) {
      onChannelUp?.()
      event.preventDefault()
      return
    }

    if (isKeyMatch(keyCode, KEY_CHANNEL_DOWN)) {
      onChannelDown?.()
      event.preventDefault()
      return
    }
  }

  // 숫자 키 감지
  const number = getNumberFromKeyCode(keyCode)
  if (number && !isKeyboardOpen) {
    onNumberKey?.(number)
    event.preventDefault()
    return
  }

  // 컬러 버튼 감지
  if (isKeyMatch(keyCode, KEY_RED)) {
    onRedButton?.()
    event.preventDefault()
  } else if (isKeyMatch(keyCode, KEY_GREEN)) {
    onGreenButton?.()
    event.preventDefault()
  } else if (isKeyMatch(keyCode, KEY_BLUE)) {
    onBlueButton?.()
    event.preventDefault()
  }

  // 설정 버튼 감지
  if (isKeyMatch(keyCode, KEY_MENU)) {
    onMenuButton?.()
    event.preventDefault()
  }
}
```

---

## 5. 시퀀스 흐름

### 주요 시나리오 1: 채널 버튼으로 탭 전환
```
사용자 → 리모컨 → useRemoteControl → BrowserView → TabContext → WebView
  │                    │                   │              │           │
  │  Channel Down      │                   │              │           │
  │  버튼 누름          │                   │              │           │
  │────────────────────▶ keydown 이벤트    │              │           │
  │                    │  (keyCode: 428)   │              │           │
  │                    │                   │              │           │
  │                    │  isKeyboardOpen?  │              │           │
  │                    │  ─────────────▶ false            │           │
  │                    │                   │              │           │
  │                    │  onChannelDown()  │              │           │
  │                    │──────────────────▶│              │           │
  │                    │                   │  handleChannelDown()     │
  │                    │                   │  - 다음 탭 ID 계산       │
  │                    │                   │  - 순환 전환 (마지막→첫) │
  │                    │                   │              │           │
  │                    │                   │  dispatch(SWITCH_TAB)    │
  │                    │                   │─────────────▶│           │
  │                    │                   │              │ tabReducer()
  │                    │                   │              │ activeTabId 업데이트
  │                    │                   │              │           │
  │                    │                   │  setInputValue(newUrl)   │
  │                    │                   │              │           │
  │                    │                   │  Spotlight.focus('webview-main')
  │                    │                   │─────────────────────────▶│
  │                    │                   │              │           │ 포커스 이동
  │                    │                   │              │           │
  │  탭 전환 완료 (0.3초 이내)                                          │
  │◀──────────────────────────────────────────────────────────────────│
```

### 주요 시나리오 2: 컬러 버튼으로 북마크 패널 열기
```
사용자 → 리모컨 → useRemoteControl → BrowserView → BookmarkPanel
  │                    │                   │              │
  │  Red 버튼 누름     │                   │              │
  │────────────────────▶ keydown 이벤트    │              │
  │                    │  (keyCode: 403)   │              │
  │                    │                   │              │
  │                    │  isKeyMatch(403, KEY_RED)        │
  │                    │  → true           │              │
  │                    │                   │              │
  │                    │  onRedButton()    │              │
  │                    │──────────────────▶│              │
  │                    │                   │  handleBookmarkClick()
  │                    │                   │  setShowBookmarkPanel(true)
  │                    │                   │              │
  │                    │                   │  <BookmarkPanel visible={true} />
  │                    │                   │─────────────▶│
  │                    │                   │              │ 패널 렌더링
  │                    │                   │              │ Spotlight 포커스 자동 이동
  │                    │                   │              │
  │  북마크 패널 표시                                       │
  │◀───────────────────────────────────────────────────────│
```

### 주요 시나리오 3: 숫자 키로 직접 탭 선택
```
사용자 → 리모컨 → useRemoteControl → BrowserView → TabContext
  │                    │                   │              │
  │  숫자 3 버튼 누름  │                   │              │
  │────────────────────▶ keydown 이벤트    │              │
  │                    │  (keyCode: 51)    │              │
  │                    │                   │              │
  │                    │  isKeyboardOpen?  │              │
  │                    │  ─────────────▶ false            │
  │                    │                   │              │
  │                    │  getNumberFromKeyCode(51)        │
  │                    │  → 3              │              │
  │                    │                   │              │
  │                    │  onNumberKey(3)   │              │
  │                    │──────────────────▶│              │
  │                    │                   │  handleNumberKey(3)
  │                    │                   │  - targetIndex = 2
  │                    │                   │  - tabs[2] 존재?
  │                    │                   │  - 이미 활성화?
  │                    │                   │  → 조건 통과
  │                    │                   │              │
  │                    │                   │  dispatch(SWITCH_TAB)
  │                    │                   │─────────────▶│
  │                    │                   │              │ 탭 전환
  │                    │                   │              │
  │  세 번째 탭 활성화                                       │
  │◀────────────────────────────────────────────────────────│
```

### 에러 시나리오 1: URLBar 키보드 열린 상태에서 숫자 키 입력
```
사용자 → 리모컨 → useRemoteControl → URLBar
  │                    │                   │
  │  URLBar 클릭       │                   │
  │────────────────────────────────────────▶ 키보드 열림
  │                    │                   │ setShowKeyboard(true)
  │                    │                   │
  │  숫자 3 버튼 누름  │                   │
  │────────────────────▶ keydown 이벤트    │
  │                    │  (keyCode: 51)    │
  │                    │                   │
  │                    │  isKeyboardOpen?  │
  │                    │  ─────────────▶ true ❌
  │                    │                   │
  │                    │  getNumberFromKeyCode(51)
  │                    │  → 3              │
  │                    │  BUT isKeyboardOpen === true
  │                    │  → return (무시)  │
  │                    │                   │
  │                    │  event.preventDefault() 호출 안 됨
  │                    │                   │
  │                    │                   │ 기본 동작 허용
  │                    │                   │ URLBar에 "3" 입력 ✅
  │                    │                   │
  │  URLBar에 "3" 표시                      │
  │◀────────────────────────────────────────│
```

### 에러 시나리오 2: 최대 탭 수 제한 (Blue 버튼)
```
사용자 → 리모컨 → useRemoteControl → BrowserView → TabContext
  │                    │                   │              │
  │  현재 탭 5개       │                   │              │
  │                    │                   │              │
  │  Blue 버튼 누름    │                   │              │
  │────────────────────▶ keydown 이벤트    │              │
  │                    │  (keyCode: 407)   │              │
  │                    │                   │              │
  │                    │  onBlueButton()   │              │
  │                    │──────────────────▶│              │
  │                    │                   │  handleNewTab()
  │                    │                   │  isMaxTabsReached(tabs)
  │                    │                   │  → true ❌   │
  │                    │                   │              │
  │                    │                   │  logger.warn('최대 탭 수 초과')
  │                    │                   │              │
  │                    │                   │  return (중단)
  │                    │                   │              │
  │  탭 생성 안 됨 (경고 로그만 출력)                       │
  │◀────────────────────────────────────────────────────────│
```

---

## 6. 영향 범위 분석

### 수정 필요한 기존 파일

#### 1. src/hooks/useRemoteControl.js (Major 변경)
- **변경 내용**:
  - 옵션 인터페이스 확장 (isKeyboardOpen, 채널/컬러/숫자/설정 버튼 콜백)
  - keyCodeConstants.js import 추가
  - 디바운스 로직 추가 (useRef로 마지막 키 입력 시각 기록)
  - 키보드 열린 상태일 때 일부 단축키 무시 로직
  - logger.debug로 모든 키 입력 기록
- **추정 라인 수**: 70줄 → 200줄 (약 130줄 추가)

#### 2. src/views/BrowserView.js (Minor 변경)
- **변경 내용**:
  - useRemoteControl 호출 확장 (기존 4개 옵션 → 12개 옵션)
  - handleChannelUp, handleChannelDown, handleNumberKey 추가 (각 20줄)
  - 기존 함수 재사용: handleBookmarkClick, handleHistoryClick, handleNewTab, handleSettingsClick
- **추정 라인 수**: 820줄 → 880줄 (약 60줄 추가)

#### 3. src/contexts/TabContext.js (변경 없음)
- **영향**: SWITCH_TAB 액션은 이미 구현되어 있음 (F-06에서 구현)
- **추가 작업 없음**

#### 4. src/components/NavigationBar/NavigationBar.js (변경 없음)
- **영향**: 버튼 클릭 핸들러는 기존 그대로 유지
- **단축키 로직 없음** (useRemoteControl Hook이 담당)

### 새로 생성할 파일

#### 1. src/utils/keyCodeConstants.js (신규)
- **역할**: webOS 리모컨 키 코드 상수 정의 및 헬퍼 함수
- **주요 내용**:
  - KEY_BACK, KEY_CHANNEL_UP, KEY_CHANNEL_DOWN
  - KEY_RED, KEY_GREEN, KEY_YELLOW, KEY_BLUE
  - KEY_NUMBER_1~5, KEY_MENU
  - isKeyMatch(), getNumberFromKeyCode() 헬퍼 함수
- **추정 라인 수**: 80줄

### 영향 받는 기존 기능

#### F-04 (페이지 탐색 컨트롤)
- **영향**: useRemoteControl의 Back 키 로직 변경 없음
- **호환성**: 기존 동작 유지 (isWebViewFocused 분기)

#### F-06 (탭 관리 시스템)
- **영향**: 채널 버튼, 숫자 키로 탭 전환 가능 → 사용성 개선
- **변경**: TabContext의 SWITCH_TAB 액션 호출 증가

#### F-07 (북마크 시스템)
- **영향**: Red 버튼으로 북마크 패널 빠른 접근 가능
- **변경**: BookmarkPanel 열림 경로 추가 (NavigationBar 버튼 + Red 버튼)

#### F-08 (히스토리 패널)
- **영향**: Green 버튼으로 히스토리 패널 빠른 접근 가능
- **변경**: HistoryPanel 열림 경로 추가

#### F-11 (설정 화면)
- **영향**: Menu 버튼으로 설정 패널 빠른 접근 가능
- **변경**: SettingsPanel 열림 경로 추가

#### F-12 (다운로드 관리)
- **영향**: Yellow 버튼 동작 유지 (기존 구현)
- **변경 없음**

---

## 7. 기술적 주의사항

### 1. Yellow 버튼 keyCode 중복 문제
- **문제**: Yellow(405, 406)와 Green(405, 406)이 keyCode가 동일할 수 있음
- **해결 방안**:
  - useRemoteControl에서 Yellow 버튼 처리를 Green보다 먼저 배치 (기존 동작 우선)
  - 실제 webOS 기기에서는 하드웨어 이벤트로 구분 가능 (event.key 추가 확인)
  - 테스트 시 keyCode 로그를 남겨 실제 기기별 차이 확인

### 2. 디바운스 타이밍
- 0.5초 디바운스는 사용자 경험에 영향을 줄 수 있음
- 채널 버튼 연속 클릭 시나리오 고려: 0.3초로 축소 고려
- 키별로 다른 디바운스 시간 적용 가능:
  - 채널 버튼: 300ms (빠른 탭 전환)
  - 컬러 버튼: 500ms (중복 패널 열기 방지)
  - 숫자 키: 300ms

### 3. 키보드 우선순위 관리
- URLBar 키보드가 열린 상태에서 Back 키 동작:
  - BrowserView의 handleKeyDown에서 먼저 처리 (키보드 닫기)
  - useRemoteControl의 Back 키 로직은 실행되지 않음 (event.stopPropagation)
- 현재 구조에서는 BrowserView가 먼저 키 이벤트를 받지 못함
- **해결 방안**: useRemoteControl에서 isKeyboardOpen일 때 Back 키는 그대로 허용 (기본 동작)

### 4. Spotlight 포커스 충돌 방지
- 패널이 열려있을 때 채널 버튼 동작:
  - 패널 내부에 Spotlight 컨테이너가 있어 키 이벤트가 패널로 먼저 전달됨
  - useRemoteControl은 window 레벨 이벤트이므로 나중에 실행
  - **해결 방안**: 패널이 visible일 때는 단축키 무시 (BrowserView에서 패널 상태 전달)

### 5. iframe contentWindow 접근 제한
- WebView 포커스 상태 판단 시 CORS 에러 가능
- 현재 isWebViewFocused는 Spotlight 이벤트로 추적 (안전)
- 탭 전환 후 WebView로 포커스 이동 시 Spotlight.focus('webview-main') 사용 (안전)

### 6. 동시 입력 시나리오
- 채널 버튼 + 컬러 버튼 동시 입력 → 디바운스로 방지
- 숫자 키 빠른 연속 입력 (예: 1→2→3) → 각 입력은 독립 처리 (디바운스 키별 분리)

### 7. 성능 최적화
- useCallback으로 핸들러 함수 메모이제이션 (BrowserView)
- useRef로 디바운스 상태 관리 (리렌더링 방지)
- logger.debug는 프로덕션 빌드에서 자동 제거 (logger.js 설정 확인)

---

## 8. 테스트 전략

### 단위 테스트 (Jest)

#### useRemoteControl.test.js 확장
```javascript
describe('F-13 리모컨 단축키', () => {
  describe('채널 버튼 (FR-1)', () => {
    it('Channel Up 버튼(keyCode 427)을 감지한다', () => {
      const onChannelUp = jest.fn()

      renderHook(() => useRemoteControl({ onChannelUp }))

      fireEvent.keyDown(window, { keyCode: 427 })

      expect(onChannelUp).toHaveBeenCalledTimes(1)
    })

    it('키보드 열린 상태에서 채널 버튼을 무시한다', () => {
      const onChannelUp = jest.fn()

      renderHook(() => useRemoteControl({
        isKeyboardOpen: true,
        onChannelUp
      }))

      fireEvent.keyDown(window, { keyCode: 427 })

      expect(onChannelUp).not.toHaveBeenCalled()
    })

    it('0.5초 이내 중복 입력을 무시한다', () => {
      const onChannelUp = jest.fn()

      renderHook(() => useRemoteControl({ onChannelUp }))

      fireEvent.keyDown(window, { keyCode: 427 })
      fireEvent.keyDown(window, { keyCode: 427 })  // 즉시 재입력

      expect(onChannelUp).toHaveBeenCalledTimes(1)  // 1번만 호출
    })
  })

  describe('컬러 버튼 (FR-2)', () => {
    it('Red 버튼(keyCode 403, 404)을 감지한다', () => {
      const onRedButton = jest.fn()

      renderHook(() => useRemoteControl({ onRedButton }))

      fireEvent.keyDown(window, { keyCode: 403 })
      expect(onRedButton).toHaveBeenCalledTimes(1)

      fireEvent.keyDown(window, { keyCode: 404 })
      expect(onRedButton).toHaveBeenCalledTimes(2)
    })

    it('Green 버튼으로 히스토리 패널을 연다', () => {
      const onGreenButton = jest.fn()

      renderHook(() => useRemoteControl({ onGreenButton }))

      fireEvent.keyDown(window, { keyCode: 405 })

      expect(onGreenButton).toHaveBeenCalled()
    })

    it('Blue 버튼으로 새 탭을 생성한다', () => {
      const onBlueButton = jest.fn()

      renderHook(() => useRemoteControl({ onBlueButton }))

      fireEvent.keyDown(window, { keyCode: 407 })

      expect(onBlueButton).toHaveBeenCalled()
    })

    it('Yellow 버튼 기존 동작을 유지한다 (다운로드 패널)', () => {
      const onYellowButton = jest.fn()

      renderHook(() => useRemoteControl({ onYellowButton }))

      fireEvent.keyDown(window, { keyCode: 405 })

      expect(onYellowButton).toHaveBeenCalled()
    })
  })

  describe('숫자 키 (FR-3)', () => {
    it('숫자 1~5 키를 감지하여 onNumberKey 호출', () => {
      const onNumberKey = jest.fn()

      renderHook(() => useRemoteControl({ onNumberKey }))

      fireEvent.keyDown(window, { keyCode: 49 })  // "1"
      expect(onNumberKey).toHaveBeenCalledWith(1)

      fireEvent.keyDown(window, { keyCode: 53 })  // "5"
      expect(onNumberKey).toHaveBeenCalledWith(5)
    })

    it('키보드 열린 상태에서 숫자 키를 무시한다', () => {
      const onNumberKey = jest.fn()

      renderHook(() => useRemoteControl({
        isKeyboardOpen: true,
        onNumberKey
      }))

      fireEvent.keyDown(window, { keyCode: 49 })

      expect(onNumberKey).not.toHaveBeenCalled()
    })
  })

  describe('설정 버튼 (FR-5)', () => {
    it('Menu 버튼(keyCode 457, 18)을 감지한다', () => {
      const onMenuButton = jest.fn()

      renderHook(() => useRemoteControl({ onMenuButton }))

      fireEvent.keyDown(window, { keyCode: 457 })
      expect(onMenuButton).toHaveBeenCalledTimes(1)

      fireEvent.keyDown(window, { keyCode: 18 })
      expect(onMenuButton).toHaveBeenCalledTimes(2)
    })
  })
})
```

#### BrowserView.shortcuts.test.js (신규)
```javascript
describe('BrowserView 단축키 통합 테스트', () => {
  it('Channel Down 버튼으로 다음 탭으로 전환', () => {
    const { tabs, activeTabId } = renderBrowserViewWithTabs(3)

    // 현재 탭 = Tab1
    expect(activeTabId).toBe(tabs[0].id)

    // Channel Down 버튼
    fireEvent.keyDown(window, { keyCode: 428 })

    // 다음 탭 = Tab2
    expect(getActiveTabId()).toBe(tabs[1].id)

    // WebView 포커스 확인
    expect(Spotlight.getCurrent()).toBe('webview-main')
  })

  it('숫자 3 버튼으로 세 번째 탭 선택', () => {
    const { tabs } = renderBrowserViewWithTabs(5)

    // 숫자 3 버튼
    fireEvent.keyDown(window, { keyCode: 51 })

    // 세 번째 탭 활성화
    expect(getActiveTabId()).toBe(tabs[2].id)
  })

  it('Red 버튼으로 북마크 패널 열기', () => {
    render(<BrowserView />)

    // Red 버튼
    fireEvent.keyDown(window, { keyCode: 403 })

    // BookmarkPanel visible
    expect(screen.getByText('북마크')).toBeInTheDocument()
  })

  it('최대 탭 수 제한 시 Blue 버튼 무시', () => {
    const { tabs } = renderBrowserViewWithTabs(5)  // 최대 5개

    // Blue 버튼 (새 탭 생성)
    fireEvent.keyDown(window, { keyCode: 407 })

    // 탭 수 변화 없음
    expect(getTabs().length).toBe(5)

    // 경고 로그 확인
    expect(logger.warn).toHaveBeenCalledWith(
      expect.stringContaining('최대 탭 수 초과')
    )
  })
})
```

### 통합 테스트 (실제 webOS 기기)

#### 테스트 케이스
| ID | 테스트 시나리오 | 예상 결과 | 검증 항목 |
|----|---------------|----------|----------|
| IT-1 | 탭 3개 생성 → Channel Down 3회 클릭 | Tab1 → Tab2 → Tab3 → Tab1 (순환) | 탭 전환, WebView 포커스 |
| IT-2 | 탭 4개 생성 → 숫자 3 버튼 | 세 번째 탭 활성화 | 직접 탭 선택 |
| IT-3 | URLBar 클릭 → 숫자 3 버튼 | URLBar에 "3" 입력, 탭 전환 안 됨 | 키보드 우선순위 |
| IT-4 | Red 버튼 클릭 | 북마크 패널 열림 | 패널 표시, 포커스 이동 |
| IT-5 | Green 버튼 클릭 | 히스토리 패널 열림 | 패널 표시 |
| IT-6 | Blue 버튼 클릭 (탭 5개) | 새 탭 생성 안 됨 | 최대 탭 수 제한 |
| IT-7 | Menu 버튼 클릭 | 설정 패널 열림 | 패널 표시 |
| IT-8 | Channel Up 연속 빠른 클릭 | 디바운스로 0.5초당 1회만 처리 | 중복 입력 방지 |
| IT-9 | Yellow 버튼 클릭 | 다운로드 패널 열림 (기존 동작 유지) | 기존 기능 호환성 |

### 성능 테스트

#### 반응 속도 측정
```javascript
describe('단축키 반응 속도 (NFR-1)', () => {
  it('리모컨 키 입력 후 0.3초 이내 반응', () => {
    const startTime = Date.now()

    fireEvent.keyDown(window, { keyCode: 427 })  // Channel Up

    const endTime = Date.now()
    const responseTime = endTime - startTime

    expect(responseTime).toBeLessThan(300)  // 300ms 이내
  })
})
```

---

## 9. 에러 처리

### 에러 케이스 정의

| 에러 케이스 | 발생 조건 | 처리 방법 | 사용자 피드백 |
|-----------|----------|----------|-------------|
| 존재하지 않는 탭 선택 | 숫자 키로 없는 탭 선택 (예: 탭 3개, 숫자 5 입력) | 무시, logger.debug 기록 | 없음 (조용히 실패) |
| 최대 탭 수 초과 | Blue 버튼으로 탭 생성 (이미 5개) | 무시, logger.warn 기록 | 없음 (F-14 토스트 예정) |
| 이미 활성화된 탭 선택 | 현재 활성 탭 번호 입력 | 무시, logger.debug 기록 | 없음 |
| 탭 1개일 때 채널 버튼 | Channel Up/Down 클릭 | 무시, 함수 초기 return | 없음 |
| 패널 열린 상태에서 컬러 버튼 | Red 버튼으로 북마크 패널 중복 열기 | 무시 (BrowserView에서 상태 체크) | 없음 |
| Spotlight 포커스 충돌 | 패널 내부에서 채널 버튼 | 패널 내부 네비게이션 우선 | 없음 |

### 로깅 전략

```javascript
// useRemoteControl.js
const handleKeyDown = (event) => {
  const { keyCode } = event

  // 모든 키 입력 디버그 로그
  logger.debug('[useRemoteControl] 키 입력:', {
    keyCode,
    key: event.key,
    isKeyboardOpen
  })

  // 처리된 키
  if (isKeyMatch(keyCode, KEY_CHANNEL_UP)) {
    logger.info('[useRemoteControl] Channel Up 감지')
    onChannelUp?.()
  }

  // 무시된 키 (키보드 열림)
  if (isKeyboardOpen && isKeyMatch(keyCode, KEY_CHANNEL_UP)) {
    logger.debug('[useRemoteControl] Channel Up 무시 (키보드 열림)')
    return
  }

  // 디바운스로 무시된 키
  if (isDebounced(keyCode)) {
    logger.debug('[useRemoteControl] 중복 입력 무시 (디바운스):', keyCode)
    return
  }
}
```

---

## 10. 확장 가능성

### M3 이후 추가 가능한 기능

#### 1. 스크롤 제어 (FR-4)
- Play/Pause 버튼으로 자동 스크롤
- FastForward/Rewind 버튼으로 페이지 끝/위로 이동
- CORS 에러 처리 추가 필요

#### 2. 사용자 정의 키 매핑 (F-16 후보)
- 설정 패널에서 컬러 버튼 기능 변경
- 로컬 저장소에 키 매핑 저장
- keyCodeConstants.js를 동적으로 오버라이드

#### 3. 단축키 튜토리얼 (F-17 후보)
- 앱 첫 실행 시 단축키 안내 오버레이
- "다시 보지 않기" 옵션 제공

#### 4. 북마크 추가 단축키
- features.md 원안: Yellow 버튼으로 북마크 추가
- 현재는 Yellow가 다운로드로 사용 중이므로 보류
- 대안: 숫자 0 키, 또는 장기 누름(Long Press) 패턴

---

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | F-13 기능 설계 |
