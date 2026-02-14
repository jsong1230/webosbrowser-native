# 설정 화면 — 기술 설계서

## 1. 참조
- 요구사항 분석서: `docs/specs/settings/requirements.md`
- 관련 서비스:
  - `src/services/searchService.js` (검색 엔진 관리)
  - `src/services/bookmarkService.js` (북마크 삭제)
  - `src/services/historyService.js` (히스토리 삭제)
  - `src/services/downloadService.js` (다운로드 삭제)
  - `src/services/indexedDBService.js` (IndexedDB 초기화)

---

## 2. 아키텍처 결정

### 결정 1: 설정 저장소 선택 (LocalStorage vs LS2 API)
- **선택지**:
  - A) LocalStorage — 브라우저 기본 API, 간단한 key-value 저장
  - B) webOS LS2 API — webOS 플랫폼 네이티브 API, 더 안정적인 영속화
  - C) IndexedDB — 구조화된 데이터, 대용량 저장 가능
- **결정**: **A) LocalStorage**
- **근거**:
  - 설정 데이터는 단순 JSON 객체 (100 bytes 이하)로 LocalStorage 용량 제한 내
  - searchService.js에서 이미 LocalStorage 사용 중 (일관성)
  - LS2 API는 webOS 문서가 제한적이고 구현 복잡도가 높음
  - IndexedDB는 설정 데이터에 과도한 복잡성 추가
- **트레이드오프**:
  - 포기: LS2 API의 플랫폼 네이티브 영속성 (앱 재설치 시 데이터 보존)
  - 얻음: 구현 간결성, 기존 코드베이스 일관성, 브라우저 호환성

### 결정 2: 설정 관리 레이어 (Context vs 로컬 상태)
- **선택지**:
  - A) React Context — 전역 상태 관리, 앱 전체에서 접근
  - B) SettingsPanel 로컬 상태 — 설정 패널 내부에서만 관리
  - C) settingsService.js 모듈 스코프 — 서비스 레이어에서 싱글톤으로 관리
- **결정**: **C) settingsService.js 모듈 스코프 (+ 앱 초기화 시 로드)**
- **근거**:
  - 설정은 앱 시작 시 로드 후 거의 변경되지 않음 (컴포넌트 리렌더링 불필요)
  - 검색 엔진은 이미 searchService.js에서 모듈 스코프로 관리 중
  - 테마/홈페이지는 BrowserView, NavigationBar에서만 참조
  - Context 추가 시 오버헤드 증가 (TabContext, DownloadContext 이미 존재)
- **트레이드오프**:
  - 포기: 설정 변경 시 리액티브 업데이트 (수동 이벤트 발생 필요)
  - 얻음: 단순한 구조, 불필요한 리렌더링 방지, 메모리 효율

### 결정 3: 테마 적용 방식 (CSS 클래스 vs Enact skinVariants)
- **선택지**:
  - A) CSS 클래스 전환 (`body.dark-theme`) — 수동 CSS 관리
  - B) Enact MoonstoneDecorator의 skinVariants prop — Enact 기본 테마 시스템
  - C) CSS 변수 (`--theme-color`) — CSS Custom Properties
- **결정**: **B) Enact skinVariants (MoonstoneDecorator)**
- **근거**:
  - Enact Moonstone은 `light`/`dark` skin을 기본 지원 (컴포넌트 자동 스타일 변경)
  - App.js의 MoonstoneDecorator에 `skin` prop 전달로 전역 적용 가능
  - 커스텀 CSS 클래스 관리 불필요 (Moonstone 컴포넌트가 자동 적응)
  - webOS 플랫폼 권장 방식
- **트레이드오프**:
  - 포기: 세밀한 커스텀 테마 제어
  - 얻음: Enact 에코시스템 일관성, 구현 간결성, 유지보수성

### 결정 4: 브라우징 데이터 삭제 방법 (WebView API vs IndexedDB)
- **선택지**:
  - A) WebView API (`clearCache()`, `clearCookies()`) — 브라우저 캐시/쿠키 삭제
  - B) IndexedDB `clear()` — 앱의 로컬 DB (북마크, 히스토리, 다운로드) 삭제
  - C) 혼합 (A + B) — WebView 캐시와 IndexedDB 모두 삭제
- **결정**: **C) 혼합 (WebView API + IndexedDB)**
- **근거**:
  - "쿠키/캐시 삭제"는 WebView의 웹 저장소 대상 (WebView API 필요)
  - "히스토리/북마크/다운로드 삭제"는 앱의 IndexedDB 대상 (기존 서비스 재사용)
  - 사용자는 두 유형을 구분하지 않음 (체크박스로 선택적 삭제 제공)
- **트레이드오법**:
  - 포기: 단일 API로 일관성 있는 삭제
  - 얻음: 완전한 데이터 삭제, 사용자 기대 충족

### 결정 5: SettingsPanel UI 구조 (Panel vs Popup)
- **선택지**:
  - A) Enact Panel — 전체 화면 설정 패널 (BookmarkPanel과 동일 패턴)
  - B) Enact Popup — 모달 다이얼로그 (중앙 오버레이)
  - C) 인라인 설정 영역 — BrowserView 내부에 토글
- **결정**: **A) Enact Panel**
- **근거**:
  - BookmarkPanel, HistoryPanel, DownloadPanel과 동일한 UX 패턴
  - 설정 항목이 5개 이상으로 Popup보다 Panel이 적합 (스크롤 지원)
  - Spotlight 포커스 관리 일관성 (패널 열림 → 자동 포커스)
  - 대화면 환경에서 가독성 확보 (전체 화면 활용)
- **트레이드오프**:
  - 포기: 가벼운 모달 UI (빠른 설정 변경)
  - 얻음: 일관된 UX, 확장성 (설정 항목 추가 용이)

---

## 3. 컴포넌트 아키텍처

### 컴포넌트 트리
```
SettingsPanel (src/components/SettingsPanel/index.js)
├── Panel (Enact Moonstone)
│   ├── Header (제목: "설정")
│   ├── Scroller
│   │   ├── SettingItem (검색 엔진)
│   │   │   └── Dropdown (Google, Naver, Bing, DuckDuckGo)
│   │   ├── SettingItem (홈페이지 URL)
│   │   │   ├── Input (URL 입력)
│   │   │   └── Button ("저장")
│   │   ├── SettingItem (테마)
│   │   │   └── Dropdown (Light, Dark)
│   │   ├── SettingItem (브라우징 데이터 삭제)
│   │   │   ├── CheckboxItem (히스토리)
│   │   │   ├── CheckboxItem (북마크)
│   │   │   ├── CheckboxItem (쿠키/캐시)
│   │   │   ├── CheckboxItem (다운로드)
│   │   │   └── Button ("삭제")
│   │   └── Button ("닫기")
│   └── ConfirmDialog (삭제 확인)
│       ├── 제목: "브라우징 데이터 삭제"
│       ├── 내용: "선택한 데이터를 삭제하시겠습니까?"
│       └── 버튼: "삭제", "취소"
```

### 컴포넌트 상세

#### SettingsPanel (메인 컴포넌트)
**파일**: `src/components/SettingsPanel/SettingsPanel.js`

**Props**:
```javascript
{
  visible: PropTypes.bool.isRequired,  // 패널 표시 여부
  onClose: PropTypes.func.isRequired   // 닫기 콜백
}
```

**상태**:
```javascript
{
  // 설정 값
  searchEngine: 'google',                // 검색 엔진 ID
  homepageUrl: 'https://www.google.com', // 홈페이지 URL
  theme: 'light',                        // 테마 (light/dark)

  // 삭제 대상 선택
  deleteTargets: {
    history: false,
    bookmarks: false,
    cookies: false,
    downloads: false
  },

  // UI 상태
  showConfirmDialog: false,              // 삭제 확인 다이얼로그
  homepageInputError: null               // 홈페이지 URL 유효성 에러
}
```

**주요 메서드**:
- `loadSettings()` — settingsService에서 설정 로드
- `handleSearchEngineChange(engineId)` — 검색 엔진 변경 → 즉시 저장
- `handleHomepageChange(url)` — 홈페이지 URL 입력
- `handleHomepageSave()` — 홈페이지 URL 저장 (유효성 검증)
- `handleThemeChange(theme)` — 테마 변경 → 즉시 적용 + 저장
- `handleDeleteTargetToggle(target)` — 삭제 대상 체크박스 토글
- `handleDeleteClick()` — 삭제 버튼 클릭 → 확인 다이얼로그 표시
- `handleDeleteConfirm()` — 삭제 확인 → 실제 삭제 수행
- `applyTheme(theme)` — 테마 적용 (App 컴포넌트에 이벤트 전달)

#### SettingItem (재사용 가능한 설정 항목)
**파일**: `src/components/SettingsPanel/SettingItem.js`

**Props**:
```javascript
{
  label: PropTypes.string.isRequired,      // 설정 항목 레이블 (예: "검색 엔진")
  description: PropTypes.string,           // 설명 텍스트 (선택)
  children: PropTypes.node.isRequired      // 설정 컨트롤 (Dropdown, Input 등)
}
```

**렌더링**:
```jsx
<div className={css.settingItem}>
  <div className={css.label}>{label}</div>
  {description && <div className={css.description}>{description}</div>}
  <div className={css.control}>
    {children}
  </div>
</div>
```

---

## 4. 서비스 레이어 설계

### settingsService.js
**파일**: `src/services/settingsService.js`

**책임**:
- 설정 데이터의 CRUD (LocalStorage 읽기/쓰기)
- 설정 값 유효성 검증
- 기본값 관리
- 브라우징 데이터 삭제 오케스트레이션

**데이터 구조** (LocalStorage):
```javascript
// 키: "browserSettings"
{
  searchEngine: "google",                    // 검색 엔진 ID
  homepageUrl: "https://www.google.com",     // 홈페이지 URL
  theme: "light"                             // 테마 (light/dark)
}
```

**주요 함수**:

```javascript
/**
 * 설정 로드 (LocalStorage → 메모리)
 * @returns {Object} - 설정 객체
 */
export const loadSettings = () => {
  try {
    const json = localStorage.getItem(SETTINGS_KEY)
    if (json) {
      const settings = JSON.parse(json)
      return { ...DEFAULT_SETTINGS, ...settings }
    }
  } catch (error) {
    logger.error('[settingsService] 설정 로드 실패:', error)
  }
  return DEFAULT_SETTINGS
}

/**
 * 설정 저장 (메모리 → LocalStorage)
 * @param {Object} settings - 설정 객체
 * @returns {boolean} - 성공 여부
 */
export const saveSettings = (settings) => {
  try {
    const json = JSON.stringify(settings)
    localStorage.setItem(SETTINGS_KEY, json)
    logger.info('[settingsService] 설정 저장 완료:', settings)
    return true
  } catch (error) {
    logger.error('[settingsService] 설정 저장 실패:', error)
    return false
  }
}

/**
 * 검색 엔진 변경
 * @param {string} engineId - 검색 엔진 ID
 * @returns {boolean} - 성공 여부
 */
export const setSearchEngine = (engineId) => {
  // searchService.setDefaultSearchEngine() 래퍼
  return searchService.setDefaultSearchEngine(engineId)
}

/**
 * 홈페이지 URL 변경
 * @param {string} url - 홈페이지 URL
 * @returns {boolean} - 성공 여부
 */
export const setHomepageUrl = (url) => {
  // URL 유효성 검증
  if (!isValidUrl(url)) {
    logger.warn('[settingsService] 유효하지 않은 홈페이지 URL:', url)
    return false
  }

  const settings = loadSettings()
  settings.homepageUrl = url
  return saveSettings(settings)
}

/**
 * 테마 변경
 * @param {string} theme - 테마 ('light' 또는 'dark')
 * @returns {boolean} - 성공 여부
 */
export const setTheme = (theme) => {
  if (!['light', 'dark'].includes(theme)) {
    logger.warn('[settingsService] 유효하지 않은 테마:', theme)
    return false
  }

  const settings = loadSettings()
  settings.theme = theme
  return saveSettings(settings)
}

/**
 * 홈페이지 URL 조회
 * @returns {string} - 홈페이지 URL
 */
export const getHomepageUrl = () => {
  const settings = loadSettings()
  return settings.homepageUrl
}

/**
 * 테마 조회
 * @returns {string} - 테마 ('light' 또는 'dark')
 */
export const getTheme = () => {
  const settings = loadSettings()
  return settings.theme
}

/**
 * 브라우징 데이터 삭제
 * @param {Object} targets - 삭제 대상 { history, bookmarks, cookies, downloads }
 * @returns {Promise<Object>} - 삭제 결과 { deletedCount, errors }
 */
export const deleteBrowsingData = async (targets) => {
  const results = {
    deletedCount: {
      history: 0,
      bookmarks: 0,
      cookies: 0,
      downloads: 0
    },
    errors: []
  }

  try {
    // 히스토리 삭제
    if (targets.history) {
      await historyService.deleteAllHistory()
      const count = await historyService.getAllHistory().then(list => list.length)
      results.deletedCount.history = count
      logger.info('[settingsService] 히스토리 삭제 완료')
    }

    // 북마크 삭제
    if (targets.bookmarks) {
      const allBookmarks = await bookmarkService.getAllBookmarks()
      for (const bookmark of allBookmarks) {
        await bookmarkService.deleteBookmark(bookmark.id)
      }
      results.deletedCount.bookmarks = allBookmarks.length
      logger.info('[settingsService] 북마크 삭제 완료:', allBookmarks.length, '개')
    }

    // 쿠키/캐시 삭제 (WebView API)
    if (targets.cookies) {
      // TODO: WebView API 연동 (clearCache, clearCookies)
      // webview.clearCache()
      // webview.clearCookies()
      logger.info('[settingsService] 쿠키/캐시 삭제 완료')
    }

    // 다운로드 목록 삭제
    if (targets.downloads) {
      const allDownloads = await downloadService.getDownloadHistory()
      for (const download of allDownloads) {
        await downloadService.deleteDownload(download.id)
      }
      results.deletedCount.downloads = allDownloads.length
      logger.info('[settingsService] 다운로드 삭제 완료:', allDownloads.length, '개')
    }

    return results
  } catch (error) {
    logger.error('[settingsService] 브라우징 데이터 삭제 실패:', error)
    results.errors.push(error.message)
    throw error
  }
}

/**
 * URL 유효성 검증
 * @param {string} url - URL
 * @returns {boolean} - 유효 여부
 */
const isValidUrl = (url) => {
  try {
    const parsed = new URL(url)
    return ['http:', 'https:'].includes(parsed.protocol)
  } catch {
    return false
  }
}
```

---

## 5. BrowserView 통합

### 설정 버튼 추가 (NavigationBar)
**파일**: `src/components/NavigationBar/NavigationBar.js`

**변경사항**:
- 설정 버튼 추가 (톱니바퀴 아이콘)
- `onSettingsClick` 콜백 prop 추가

**Props 추가**:
```javascript
{
  onSettingsClick: PropTypes.func  // 설정 버튼 클릭 콜백
}
```

**렌더링**:
```jsx
<IconButton
  data-spotlight-id="nav-settings"
  icon="gear"
  onClick={onSettingsClick}
  aria-label="설정"
/>
```

### BrowserView 이벤트 핸들러
**파일**: `src/views/BrowserView.js`

**상태 추가**:
```javascript
const [showSettingsPanel, setShowSettingsPanel] = useState(false)
const [currentTheme, setCurrentTheme] = useState('light')
```

**이벤트 핸들러**:
```javascript
/**
 * 설정 패널 열기
 */
const handleSettingsClick = () => {
  setShowSettingsPanel(true)
  logger.info('[BrowserView] SettingsPanel 열기')
}

/**
 * 설정 패널 닫기
 */
const handleSettingsPanelClose = () => {
  setShowSettingsPanel(false)
  logger.info('[BrowserView] SettingsPanel 닫기')
}

/**
 * 테마 변경 적용
 */
const handleThemeChange = (newTheme) => {
  setCurrentTheme(newTheme)
  logger.info('[BrowserView] 테마 변경:', newTheme)

  // App 컴포넌트에 이벤트 전달 (MoonstoneDecorator skin 변경)
  window.dispatchEvent(new CustomEvent('themechange', { detail: { theme: newTheme } }))
}

/**
 * 홈페이지 URL 변경 적용
 */
const handleHomepageChange = (newUrl) => {
  // homeUrl 상태 업데이트 (NavigationBar의 홈 버튼 동작 변경)
  setHomeUrl(newUrl)
  logger.info('[BrowserView] 홈페이지 URL 변경:', newUrl)
}
```

**렌더링**:
```jsx
<NavigationBar
  // 기존 props...
  onSettingsClick={handleSettingsClick}
/>

<SettingsPanel
  visible={showSettingsPanel}
  onClose={handleSettingsPanelClose}
  onThemeChange={handleThemeChange}
  onHomepageChange={handleHomepageChange}
/>
```

### App.js 테마 적용
**파일**: `src/App/App.js`

**상태 추가**:
```javascript
const [skin, setSkin] = useState('light')
```

**테마 변경 이벤트 리스너**:
```javascript
useEffect(() => {
  // settingsService에서 초기 테마 로드
  const initialTheme = settingsService.getTheme()
  setSkin(initialTheme)

  // BrowserView에서 테마 변경 이벤트 수신
  const handleThemeChange = (event) => {
    setSkin(event.detail.theme)
    logger.info('[App] 테마 적용:', event.detail.theme)
  }

  window.addEventListener('themechange', handleThemeChange)

  return () => {
    window.removeEventListener('themechange', handleThemeChange)
  }
}, [])
```

**MoonstoneDecorator에 skin prop 전달**:
```javascript
export default MoonstoneDecorator({ skin })(App)
```

---

## 6. 시퀀스 다이어그램

### 주요 시나리오 1: 검색 엔진 변경
```
사용자 → SettingsPanel → settingsService → searchService → LocalStorage
  │                        │                   │
  │  Dropdown 선택         │                   │
  │  (Naver)              │                   │
  │───────────────────────▶│                   │
  │                        │  setSearchEngine() │
  │                        │───────────────────▶│
  │                        │                   │  setDefaultSearchEngine()
  │                        │                   │──────────────────────────▶
  │                        │                   │  (localStorage.setItem)
  │                        │                   │◀──────────────────────────
  │                        │◀───────────────────│
  │  "저장 완료"           │                   │
  │  (즉시 적용)          │                   │
  │◀───────────────────────│                   │
```

### 주요 시나리오 2: 테마 변경
```
사용자 → SettingsPanel → settingsService → BrowserView → App
  │                        │                   │           │
  │  Dropdown 선택         │                   │           │
  │  (Dark)               │                   │           │
  │───────────────────────▶│                   │           │
  │                        │  setTheme('dark') │           │
  │                        │───────────────────▶           │
  │                        │  (localStorage 저장)          │
  │                        │                   │           │
  │                        │  onThemeChange()  │           │
  │                        │───────────────────▶│           │
  │                        │                   │  CustomEvent('themechange')
  │                        │                   │───────────▶│
  │                        │                   │           │  setSkin('dark')
  │                        │                   │           │  MoonstoneDecorator 재렌더링
  │                        │                   │           │  (UI 색상 변경)
  │  UI 즉시 변경          │                   │           │
  │  (다크 모드 적용)      │                   │           │
  │◀───────────────────────┴───────────────────┴───────────┘
```

### 주요 시나리오 3: 브라우징 데이터 삭제
```
사용자 → SettingsPanel → ConfirmDialog → settingsService → historyService / bookmarkService
  │                        │                  │                 │
  │  체크박스 선택         │                  │                 │
  │  (히스토리, 북마크)    │                  │                 │
  │───────────────────────▶│                  │                 │
  │  "삭제" 버튼 클릭      │                  │                 │
  │───────────────────────▶│                  │                 │
  │                        │  확인 다이얼로그  │                 │
  │                        │  표시            │                 │
  │                        │─────────────────▶│                 │
  │  "삭제" 확인           │                  │                 │
  │───────────────────────────────────────────▶│                 │
  │                        │                  │  deleteBrowsingData()
  │                        │                  │─────────────────▶│
  │                        │                  │                 │  deleteAllHistory()
  │                        │                  │                 │──────────────────▶
  │                        │                  │                 │◀──────────────────
  │                        │                  │                 │
  │                        │                  │  (북마크 삭제)   │
  │                        │                  │─────────────────▶│
  │                        │                  │◀─────────────────│
  │                        │                  │                 │
  │                        │  토스트 메시지    │                 │
  │                        │  "브라우징 데이터가 삭제되었습니다"
  │◀───────────────────────┴──────────────────┘                 │
```

### 에러 시나리오: 홈페이지 URL 유효성 검증 실패
```
사용자 → SettingsPanel → settingsService
  │                        │
  │  Input 입력            │
  │  "invalid-url"        │
  │───────────────────────▶│
  │  "저장" 버튼 클릭      │
  │───────────────────────▶│
  │                        │  setHomepageUrl()
  │                        │  (유효성 검증 실패)
  │                        │──────────────────▶
  │  에러 메시지 표시       │
  │  "유효한 URL을 입력하세요"
  │◀───────────────────────│
```

---

## 7. 영향 범위 분석

### 새로 생성할 파일
| 파일 경로 | 역할 |
|----------|------|
| `src/components/SettingsPanel/SettingsPanel.js` | 설정 패널 메인 컴포넌트 |
| `src/components/SettingsPanel/SettingItem.js` | 재사용 가능한 설정 항목 컴포넌트 |
| `src/components/SettingsPanel/SettingsPanel.module.less` | 설정 패널 스타일 |
| `src/components/SettingsPanel/index.js` | Export 파일 |
| `src/services/settingsService.js` | 설정 관리 서비스 |

### 수정 필요한 기존 파일
| 파일 경로 | 변경 내용 |
|----------|----------|
| `src/views/BrowserView.js` | - SettingsPanel 통합<br>- `showSettingsPanel` 상태 추가<br>- `handleSettingsClick`, `handleThemeChange`, `handleHomepageChange` 핸들러 추가<br>- `homeUrl` 상태를 settingsService에서 로드 |
| `src/components/NavigationBar/NavigationBar.js` | - 설정 버튼 추가 (톱니바퀴 아이콘)<br>- `onSettingsClick` prop 추가 |
| `src/App/App.js` | - `skin` 상태 추가<br>- `themechange` 이벤트 리스너 추가<br>- MoonstoneDecorator에 `skin` prop 전달 |
| `src/services/indexedDBService.js` | 변경 없음 (기존 함수 재사용) |

### 영향 받는 기존 기능
| 기능 | 영향 내용 |
|------|----------|
| F-09 검색 엔진 통합 | searchService의 `setDefaultSearchEngine()` 재사용 (변경 없음) |
| F-07 북마크 관리 | bookmarkService의 `deleteBookmark()` 재사용 (북마크 전체 삭제 시) |
| F-08 히스토리 관리 | historyService의 `deleteAllHistory()` 재사용 |
| F-12 다운로드 관리 | downloadService의 `deleteDownload()` 재사용 |
| F-04 네비게이션바 | 홈 버튼 동작이 settingsService의 홈페이지 URL로 변경 |
| 전체 UI | 테마 변경 시 Moonstone 컴포넌트 자동 스타일 변경 |

---

## 8. 기술적 주의사항

### LocalStorage 용량 제한
- **문제**: webOS는 앱당 LocalStorage 용량 제한이 있을 수 있음 (보통 5MB)
- **해결**: 설정 JSON은 100 bytes 이하로 매우 작음 (문제 없음)
- **모니터링**: 저장 실패 시 에러 로그 기록 + 기본값 사용

### 테마 변경 시 화면 깜빡임 방지
- **문제**: MoonstoneDecorator 재렌더링 시 잠깐 화면 깜빡임 가능
- **해결**:
  1. CSS 트랜지션 추가 (`transition: background-color 0.3s ease`)
  2. `setSkin()` 호출 전에 임시 로딩 상태 표시 (선택)
- **테스트**: 실제 webOS 프로젝터에서 전환 부드러움 확인

### WebView 쿠키/캐시 삭제 API
- **문제**: webOS WebView API 문서가 제한적 (clearCache/clearCookies 메서드 존재 여부 불확실)
- **해결**:
  1. 구현 시 webOS WebView API 문서 확인 (https://webostv.developer.lge.com)
  2. API가 없을 경우: WebView를 새 인스턴스로 교체 (임시 해결)
  3. 최악의 경우: "쿠키/캐시 삭제는 브라우저 재시작 후 적용됩니다" 안내
- **우선순위**: 구현 단계에서 검증

### 브라우징 데이터 삭제 시간
- **문제**: 대용량 히스토리(5,000개) 삭제 시 5초 이상 소요 가능
- **해결**:
  1. 삭제 중 로딩 인디케이터 표시 (Spinner)
  2. 비동기 삭제 (UI 블로킹 방지)
  3. 삭제 완료 후 토스트 알림
- **성능 목표**: 5초 이내 완료 (요구사항 충족)

### Spotlight 포커스 관리
- **문제**: SettingsPanel 열릴 때 포커스가 첫 번째 Dropdown으로 자동 이동해야 함
- **해결**:
  ```javascript
  useEffect(() => {
    if (visible) {
      Spotlight.set('settings-panel', {
        defaultElement: '[data-spotlight-id="search-engine-dropdown"]',
        enterTo: 'default-element'
      })
      setTimeout(() => {
        Spotlight.focus('settings-panel')
      }, 100)
    }
  }, [visible])
  ```

### 설정 변경 즉시 반영
- **검색 엔진**: searchService가 모듈 스코프로 관리 → 즉시 반영됨
- **홈페이지 URL**: BrowserView의 `homeUrl` 상태 업데이트 → NavigationBar 리렌더링
- **테마**: App 컴포넌트의 `skin` prop 변경 → MoonstoneDecorator 재렌더링

### 동시성 제어
- **문제**: 사용자가 설정을 빠르게 변경할 때 LocalStorage 쓰기 충돌 가능
- **해결**: LocalStorage는 싱글 스레드 동기 API로 충돌 없음 (순차 처리)
- **추가 보호**: 설정 저장 시 debounce 적용 (선택 사항)

---

## 9. Spotlight 포커스 플로우

### SettingsPanel 내부 포커스 순서
```
1. 검색 엔진 Dropdown
   ↓ (Down)
2. 홈페이지 URL Input
   ↓ (Down)
3. 홈페이지 저장 Button
   ↓ (Down)
4. 테마 Dropdown
   ↓ (Down)
5. 히스토리 CheckboxItem
   ↓ (Down)
6. 북마크 CheckboxItem
   ↓ (Down)
7. 쿠키/캐시 CheckboxItem
   ↓ (Down)
8. 다운로드 CheckboxItem
   ↓ (Down)
9. 삭제 Button
   ↓ (Down)
10. 닫기 Button
   ↑ (Up: 첫 번째 항목으로 순환)
```

### Spotlight 설정
```javascript
Spotlight.set('settings-panel', {
  defaultElement: '[data-spotlight-id="search-engine-dropdown"]',
  enterTo: 'default-element',
  leaveFor: {
    left: '',   // 좌측 이동 방지
    right: ''   // 우측 이동 방지
  }
})
```

---

## 10. UI/UX 설계

### 레이아웃
- **전체 화면**: Panel (BookmarkPanel과 동일)
- **헤더**: "설정" 제목
- **본문**: Scroller (설정 항목 스크롤)
- **푸터**: 닫기 버튼 (고정)

### 색상 (Moonstone 기본 스타일 사용)
- **Light 테마**: 흰색 배경, 검은색 텍스트
- **Dark 테마**: 검은색 배경, 흰색 텍스트
- **포커스**: 파란색 테두리 (Spotlight 기본)
- **에러**: 빨간색 텍스트 (홈페이지 URL 유효성 에러)

### 폰트 크기 (대화면 최적화)
- **항목 레이블**: 20px (Medium)
- **설명 텍스트**: 16px (Small)
- **Dropdown/Input**: 18px (Medium)
- **버튼 텍스트**: 20px (Medium)

### 간격
- **항목 간 간격**: 24px
- **레이블-컨트롤 간격**: 12px
- **패널 여백**: 48px (상하좌우)

---

## 11. 데이터 플로우

### 앱 시작 시
```
1. App.js 초기화
   ↓
2. settingsService.loadSettings() 호출
   ↓ (LocalStorage 읽기)
3. 설정 값 로드 (검색 엔진, 홈페이지, 테마)
   ↓
4. App.js의 skin 상태 설정 (테마 적용)
   ↓
5. BrowserView의 homeUrl 상태 설정
```

### 설정 변경 시
```
1. 사용자가 SettingsPanel에서 설정 변경
   ↓
2. settingsService.setSetting() 호출
   ↓ (LocalStorage 쓰기)
3. 설정 즉시 저장
   ↓
4. 콜백으로 BrowserView에 변경 사항 전달
   ↓
5. BrowserView가 상태 업데이트 (homeUrl, theme)
   ↓
6. 관련 컴포넌트 리렌더링 (NavigationBar, App 등)
```

### 브라우징 데이터 삭제 시
```
1. 사용자가 삭제 대상 선택 + 삭제 버튼 클릭
   ↓
2. ConfirmDialog 표시
   ↓
3. 사용자 확인
   ↓
4. settingsService.deleteBrowsingData() 호출
   ↓ (비동기)
5. 각 서비스의 삭제 함수 호출
   - historyService.deleteAllHistory()
   - bookmarkService.deleteBookmark() (반복)
   - downloadService.deleteDownload() (반복)
   - WebView API (clearCache, clearCookies)
   ↓
6. 삭제 완료 후 토스트 메시지 표시
```

---

## 12. 확장성 고려

### 새 설정 항목 추가 시
1. `DEFAULT_SETTINGS`에 기본값 추가
2. SettingsPanel에 새 SettingItem 추가
3. settingsService에 getter/setter 함수 추가
4. BrowserView에 콜백 핸들러 추가 (필요 시)

### 예시: "글꼴 크기" 설정 추가
```javascript
// settingsService.js
const DEFAULT_SETTINGS = {
  searchEngine: 'google',
  homepageUrl: 'https://www.google.com',
  theme: 'light',
  fontSize: 'medium'  // 신규 추가
}

export const setFontSize = (size) => {
  const settings = loadSettings()
  settings.fontSize = size
  return saveSettings(settings)
}

// SettingsPanel.js
<SettingItem label="글꼴 크기" description="텍스트 크기를 조정합니다.">
  <Dropdown
    selected={fontSize}
    onSelect={handleFontSizeChange}
  >
    {['small', 'medium', 'large']}
  </Dropdown>
</SettingItem>
```

### 향후 확장 가능 설정
- 자동완성 활성화/비활성화 (F-03 연동)
- 쿠키 차단 정책 (전체 차단 / 타사 쿠키 차단)
- 광고 차단 활성화 (webOS 확장 필요)
- 언어 설정 (다국어 지원 시)
- 프록시 설정 (네트워크 설정)
- 개인정보 보호 모드 (시크릿 탭)

---

## 13. 테스트 시나리오

### 단위 테스트
- `settingsService.loadSettings()` — LocalStorage 없을 때 기본값 반환
- `settingsService.saveSettings()` — LocalStorage에 JSON 저장 확인
- `settingsService.isValidUrl()` — URL 유효성 검증 (http/https만 허용)
- `settingsService.deleteBrowsingData()` — 각 서비스 호출 확인

### 통합 테스트
- 검색 엔진 변경 → URLBar에서 검색 시 변경된 엔진 사용 확인
- 홈페이지 URL 변경 → NavigationBar 홈 버튼 클릭 시 이동 확인
- 테마 변경 → 전체 UI 색상 즉시 변경 확인
- 브라우징 데이터 삭제 → 히스토리/북마크 패널에서 데이터 사라짐 확인

### 리모컨 조작 테스트
- 설정 패널 열림 → 첫 번째 Dropdown에 자동 포커스
- 십자키 Up/Down → 설정 항목 간 포커스 이동
- 십자키 Left/Right → Dropdown 값 변경
- OK 버튼 → Input 활성화, 버튼 클릭
- Back 버튼 → 설정 패널 닫기

### 성능 테스트
- 설정 저장 시간 → 0.3초 이내
- 테마 변경 시간 → 0.5초 이내 UI 반영
- 브라우징 데이터 삭제 시간 → 5초 이내 (5,000개 히스토리)

### 에러 처리 테스트
- 유효하지 않은 홈페이지 URL 입력 → 에러 메시지 표시
- LocalStorage 저장 실패 → 기본값 사용 + 로그 기록
- 브라우징 데이터 삭제 실패 → 에러 토스트 표시

---

## 14. 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | F-11 설정 화면 기술 설계 |
