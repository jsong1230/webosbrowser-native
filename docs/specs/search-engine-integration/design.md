# 검색 엔진 통합 — 기술 설계서

## 1. 참조
- 요구사항 분석서: `docs/specs/search-engine-integration/requirements.md`
- F-03 기술 설계서: `docs/specs/url-input-ui/design.md`
- PRD: `docs/project/prd.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md`

## 2. 아키텍처 개요

### 핵심 개념
F-03 URL 입력 UI를 확장하여 URL이 아닌 검색어 입력 시 자동으로 검색 엔진(Google, Naver 등)의 검색 결과 페이지를 로드하는 기능을 구현합니다. URL 검증 로직에 검색어 감지 및 검색 URL 생성 단계를 추가하여 사용자가 URL 형식을 몰라도 자연스럽게 웹 검색을 사용할 수 있도록 합니다.

### 전체 아키텍처
```
┌─────────────────────────────────────────────────────────┐
│                      BrowserView                         │
│  ┌────────────────────────────────────────────────────┐ │
│  │                   URLBar Component                  │ │
│  │  - 사용자 입력 ("youtube", "고양이 동영상" 등)    │ │
│  │  - handleSubmit() → validateAndFormatUrl() 호출   │ │
│  └────────────────────────────────────────────────────┘ │
│                           ↓                               │
│  ┌────────────────────────────────────────────────────┐ │
│  │          urlValidator.js (확장)                     │ │
│  │  1. URL 형식 검증 (기존 F-03 로직)                 │ │
│  │     → 성공: 유효한 URL 반환                        │ │
│  │     → 실패: null → 2단계 진행                      │ │
│  │                                                      │ │
│  │  2. 검색어로 간주 (신규 F-09 로직)                 │ │
│  │     → buildSearchUrl(input, engine) 호출           │ │
│  │     → 검색 URL 생성                                │ │
│  └────────────────────────────────────────────────────┘ │
│                           ↓                               │
│  ┌────────────────────────────────────────────────────┐ │
│  │         searchService.js (신규)                     │ │
│  │  - SEARCH_ENGINES 상수 (Google, Naver, ...)       │ │
│  │  - buildSearchUrl(query, engine)                   │ │
│  │  - getDefaultSearchEngine() (localStorage 조회)   │ │
│  │  - setDefaultSearchEngine(engine)                  │ │
│  └────────────────────────────────────────────────────┘ │
│                           ↓                               │
│  ┌────────────────────────────────────────────────────┐ │
│  │                  WebView Component                  │ │
│  │  - 생성된 검색 URL로 페이지 로드                   │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### 설계 원칙
1. **F-03 확장 최소화**: 기존 URL 입력 흐름을 최대한 유지하고 검증 실패 시점에만 검색 엔진 로직 추가
2. **Fallback 패턴**: URL 검증 실패 → 검색어로 간주 → 검색 URL 생성
3. **검색 엔진 독립성**: 새로운 검색 엔진 추가 시 `SEARCH_ENGINES` 상수만 수정
4. **설정 영속성**: localStorage로 기본 검색 엔진 저장 (F-11 설정 화면 연동 준비)
5. **에러 처리**: 검색어가 빈 문자열이면 에러 처리 (F-10 연동)

## 3. 아키텍처 결정

### 결정 1: URL 검증 실패 시 검색어 처리 시점
- **선택지**:
  - A) urlValidator에서 검색 URL 생성까지 처리
  - B) urlValidator는 null 반환, URLBar에서 검색 URL 생성
  - C) 별도의 inputProcessor 서비스 생성
- **결정**: A) urlValidator에서 검색 URL 생성까지 처리
- **근거**:
  - **단일 책임**: urlValidator가 "입력 문자열 → 유효한 URL 변환"을 전담
  - **간결성**: URLBar는 검증 결과만 받아 WebView에 전달 (로직 단순화)
  - **일관성**: 기존 F-03 설계와 동일한 흐름 유지
- **트레이드오프**: urlValidator가 searchService에 의존성 추가 (하지만 단방향 의존이므로 관리 가능)

### 결정 2: 검색 엔진 정의 방식
- **선택지**:
  - A) 상수 객체로 코드 내 정의 (`SEARCH_ENGINES`)
  - B) JSON 파일로 분리 (`searchEngines.json`)
  - C) localStorage에서 사용자 정의 검색 엔진 허용
- **결정**: A) 상수 객체로 코드 내 정의
- **근거**:
  - **단순성**: 외부 파일 로딩 불필요, 빌드 시점에 포함
  - **타입 안전성**: JavaScript 객체로 IDE 자동완성 지원
  - **확장성**: 새 검색 엔진 추가 시 코드 한 곳만 수정
  - **보안**: 사용자 정의 검색 엔진은 향후 확장 고려 (현재는 불필요)
- **트레이드오프**: 검색 엔진 추가 시 재빌드 필요 (하지만 빈도가 낮아 문제 없음)

### 결정 3: 기본 검색 엔진 저장 위치
- **선택지**:
  - A) localStorage (브라우저 로컬 저장소)
  - B) webOS LS2 API (시스템 저장소)
  - C) 앱 메모리만 사용 (재시작 시 초기화)
- **결정**: A) localStorage
- **근거**:
  - **호환성**: webOS WebView는 localStorage 지원
  - **간편성**: LS2 API보다 간단한 API (동기 방식)
  - **영속성**: 앱 재시작 후에도 설정 유지
  - **표준**: 웹 표준 API로 디버깅 용이
- **트레이드오프**: 앱 데이터 삭제 시 설정 초기화 (하지만 일반적인 사용 시나리오)

### 결정 4: 검색어 인코딩 방식
- **선택지**:
  - A) encodeURIComponent() (표준 URL 인코딩)
  - B) encodeURI() (전체 URL 인코딩)
  - C) 커스텀 인코딩 함수 (특수문자만 변환)
- **결정**: A) encodeURIComponent()
- **근거**:
  - **안전성**: 모든 특수문자를 안전하게 인코딩 (XSS 방지)
  - **표준**: 브라우저 내장 함수로 호환성 보장
  - **다국어 지원**: 한글, 일본어 등 다국어 검색어 처리
  - **검색 엔진 호환성**: 모든 주요 검색 엔진에서 지원
- **트레이드오프**: URL의 다른 부분(프로토콜, 도메인)까지 인코딩하므로 검색어 부분에만 사용 필요

### 결정 5: 검색 히스토리 저장 방식 (F-08 연동)
- **선택지**:
  - A) 검색 URL을 일반 히스토리와 동일하게 저장
  - B) 별도의 `type: 'search'` 필드 추가
  - C) 검색 히스토리는 별도 저장소 사용
- **결정**: B) `type: 'search'` 필드 추가
- **근거**:
  - **통합 관리**: historyService에서 일반 히스토리와 통합 관리
  - **구분 가능**: 검색 히스토리만 필터링 가능 (타입 구분)
  - **자동완성 활용**: URLBar 자동완성에서 검색 히스토리 제안 가능
  - **확장성**: 향후 검색 히스토리 UI 별도 표시 가능
- **트레이드오프**: historyService의 데이터 구조 수정 필요 (F-08 구현 시 반영)

### 결정 6: 설정 UI 연동 시점 (F-11)
- **선택지**:
  - A) F-09 구현과 동시에 설정 UI 구현
  - B) F-09는 검색 엔진 로직만, F-11에서 설정 UI 추가
  - C) F-09에서 임시 설정 UI 구현 후 F-11에서 통합
- **결정**: B) F-09는 검색 엔진 로직만, F-11에서 설정 UI 추가
- **근거**:
  - **단계적 구현**: F-09는 검색 기능 자체에 집중
  - **의존성 분리**: F-11 설정 화면은 독립적인 기능
  - **테스트 용이성**: 검색 로직을 먼저 완성 후 UI 추가
  - **기본값 제공**: Google을 기본 검색 엔진으로 사용 (설정 UI 없어도 동작)
- **트레이드오프**: F-09 완료 시점에는 사용자가 검색 엔진 변경 불가 (하지만 Google 기본값으로 충분)

## 4. 서비스 설계

### 4.1 searchService.js (신규)

#### 검색 엔진 정의
```javascript
// src/services/searchService.js

/**
 * 지원하는 검색 엔진 목록
 * 새로운 검색 엔진 추가 시 이 객체만 수정
 */
export const SEARCH_ENGINES = {
	google: {
		id: 'google',
		name: 'Google',
		urlTemplate: 'https://www.google.com/search?q={query}',
		icon: '/assets/icons/google.png'  // 선택적 (F-11에서 사용)
	},
	naver: {
		id: 'naver',
		name: 'Naver',
		urlTemplate: 'https://search.naver.com/search.naver?query={query}',
		icon: '/assets/icons/naver.png'
	},
	bing: {
		id: 'bing',
		name: 'Bing',
		urlTemplate: 'https://www.bing.com/search?q={query}',
		icon: '/assets/icons/bing.png'
	},
	duckduckgo: {
		id: 'duckduckgo',
		name: 'DuckDuckGo',
		urlTemplate: 'https://duckduckgo.com/?q={query}',
		icon: '/assets/icons/duckduckgo.png'
	}
}

// 기본 검색 엔진 (설정 없을 때 사용)
export const DEFAULT_SEARCH_ENGINE = 'google'

// localStorage 키
const STORAGE_KEY = 'defaultSearchEngine'
```

#### 주요 함수
```javascript
/**
 * 검색 URL 생성
 *
 * @param {string} query - 검색어
 * @param {string} engineId - 검색 엔진 ID (기본: 현재 설정된 엔진)
 * @returns {string} - 검색 URL
 *
 * @example
 * buildSearchUrl('youtube')
 * // → 'https://www.google.com/search?q=youtube'
 *
 * buildSearchUrl('고양이 동영상', 'naver')
 * // → 'https://search.naver.com/search.naver?query=%EA%B3%A0%EC%96%91%EC%9D%B4%20%EB%8F%99%EC%98%81%EC%83%81'
 */
export const buildSearchUrl = (query, engineId = null) => {
	// 검색어가 비어있으면 null 반환
	if (!query || typeof query !== 'string' || query.trim().length === 0) {
		logger.warn('[searchService] 검색어가 비어있음')
		return null
	}

	// 검색 엔진 결정 (파라미터 → 설정 → 기본값)
	const targetEngine = engineId || getDefaultSearchEngine()

	// 검색 엔진 정의 조회
	const engine = SEARCH_ENGINES[targetEngine]
	if (!engine) {
		logger.warn('[searchService] 알 수 없는 검색 엔진:', targetEngine)
		// 폴백: Google 사용
		return buildSearchUrl(query, DEFAULT_SEARCH_ENGINE)
	}

	// URL 인코딩 (특수문자, 공백, 다국어 처리)
	const encodedQuery = encodeURIComponent(query.trim())

	// URL 템플릿에 쿼리 삽입
	const searchUrl = engine.urlTemplate.replace('{query}', encodedQuery)

	logger.info('[searchService] 검색 URL 생성:', { query, engine: targetEngine, url: searchUrl })
	return searchUrl
}

/**
 * 현재 설정된 기본 검색 엔진 조회
 *
 * @returns {string} - 검색 엔진 ID
 *
 * @example
 * getDefaultSearchEngine()  // → 'google'
 */
export const getDefaultSearchEngine = () => {
	try {
		const saved = localStorage.getItem(STORAGE_KEY)
		// 유효한 검색 엔진 ID인지 확인
		if (saved && SEARCH_ENGINES[saved]) {
			logger.debug('[searchService] 저장된 검색 엔진 사용:', saved)
			return saved
		}
	} catch (error) {
		logger.error('[searchService] localStorage 조회 실패:', error)
	}

	// 기본값 반환
	logger.debug('[searchService] 기본 검색 엔진 사용:', DEFAULT_SEARCH_ENGINE)
	return DEFAULT_SEARCH_ENGINE
}

/**
 * 기본 검색 엔진 설정
 *
 * @param {string} engineId - 검색 엔진 ID
 * @returns {boolean} - 설정 성공 여부
 *
 * @example
 * setDefaultSearchEngine('naver')  // → true
 */
export const setDefaultSearchEngine = (engineId) => {
	// 유효한 검색 엔진인지 확인
	if (!SEARCH_ENGINES[engineId]) {
		logger.warn('[searchService] 유효하지 않은 검색 엔진:', engineId)
		return false
	}

	try {
		localStorage.setItem(STORAGE_KEY, engineId)
		logger.info('[searchService] 기본 검색 엔진 설정:', engineId)
		return true
	} catch (error) {
		logger.error('[searchService] localStorage 저장 실패:', error)
		return false
	}
}

/**
 * 모든 검색 엔진 목록 조회 (F-11 설정 UI에서 사용)
 *
 * @returns {Array} - 검색 엔진 배열
 *
 * @example
 * getAllSearchEngines()
 * // → [
 * //   { id: 'google', name: 'Google', ... },
 * //   { id: 'naver', name: 'Naver', ... },
 * //   ...
 * // ]
 */
export const getAllSearchEngines = () => {
	return Object.values(SEARCH_ENGINES)
}

/**
 * 검색 엔진 이름 조회
 *
 * @param {string} engineId - 검색 엔진 ID
 * @returns {string} - 검색 엔진 이름
 *
 * @example
 * getSearchEngineName('google')  // → 'Google'
 */
export const getSearchEngineName = (engineId) => {
	const engine = SEARCH_ENGINES[engineId]
	return engine ? engine.name : 'Unknown'
}
```

### 4.2 urlValidator.js 확장

#### 검색어 감지 및 검색 URL 생성
```javascript
// src/utils/urlValidator.js (기존 파일 확장)

import logger from './logger'
import { buildSearchUrl } from '../services/searchService'

/**
 * URL 유효성 검증 및 자동 보완 (F-09 확장)
 *
 * @param {string} input - 사용자 입력 문자열
 * @returns {string|null} - 유효한 URL 또는 검색 URL 또는 null
 *
 * @example
 * validateAndFormatUrl('google.com')
 * // → 'http://google.com' (URL로 인식)
 *
 * validateAndFormatUrl('youtube')
 * // → 'https://www.google.com/search?q=youtube' (검색어로 인식)
 *
 * validateAndFormatUrl('고양이 동영상')
 * // → 'https://www.google.com/search?q=%EA%B3%A0%EC%96%91%EC%9D%B4%20%EB%8F%99%EC%98%81%EC%83%81'
 */
export const validateAndFormatUrl = (input) => {
	if (!input || typeof input !== 'string') {
		logger.debug('[urlValidator] 입력값이 비어있거나 문자열이 아님')
		return null
	}

	const trimmed = input.trim()
	if (trimmed.length === 0) {
		logger.debug('[urlValidator] 공백 문자열 입력')
		return null
	}

	// 1. 프로토콜 포함 URL (http://, https://)
	if (/^https?:\/\/.+/.test(trimmed)) {
		try {
			const url = new URL(trimmed)
			logger.debug('[urlValidator] 프로토콜 포함 URL 검증 성공:', url.href)
			return url.href
		} catch (error) {
			logger.warn('[urlValidator] 프로토콜 포함 URL 검증 실패:', error.message)
			return null
		}
	}

	// 2. 도메인 형식 감지 (example.com, www.example.com)
	// - 영문자/숫자로 시작
	// - 하이픈(-), 점(.)으로 구분 가능
	// - 최상위 도메인(.com, .org 등) 필수
	// - 경로(/path), 쿼리(?query) 선택적
	if (/^[a-zA-Z0-9]+([-.]{1}[a-zA-Z0-9]+)*\.[a-zA-Z]{2,}(\/.*)?$/.test(trimmed)) {
		// http:// 자동 추가
		try {
			const url = new URL(`http://${trimmed}`)
			logger.debug('[urlValidator] 도메인 형식 자동 보완:', url.href)
			return url.href
		} catch (error) {
			logger.warn('[urlValidator] 도메인 URL 생성 실패:', error.message)
			return null
		}
	}

	// 3. localhost 또는 IP 주소
	// - localhost[:포트][/경로]
	// - xxx.xxx.xxx.xxx[:포트][/경로]
	if (/^(localhost|(\d{1,3}\.){3}\d{1,3})(:\d+)?(\/.*)?$/.test(trimmed)) {
		try {
			const url = new URL(`http://${trimmed}`)
			logger.debug('[urlValidator] localhost/IP 주소 자동 보완:', url.href)
			return url.href
		} catch (error) {
			logger.warn('[urlValidator] localhost/IP URL 생성 실패:', error.message)
			return null
		}
	}

	// 4. 검색어로 판단 (F-09 신규 로직)
	// - 위의 모든 URL 형식에 해당하지 않으면 검색어로 간주
	// - buildSearchUrl()로 검색 URL 생성
	logger.debug('[urlValidator] URL 형식 아님 → 검색어로 처리:', trimmed)
	const searchUrl = buildSearchUrl(trimmed)

	if (searchUrl) {
		logger.info('[urlValidator] 검색 URL 생성 성공:', searchUrl)
		return searchUrl
	} else {
		// 검색 URL 생성 실패 (검색어가 비어있는 경우)
		logger.warn('[urlValidator] 검색 URL 생성 실패 (빈 검색어)')
		return null
	}
}

// isValidUrl, isSecureUrl 함수는 기존 그대로 유지
```

### 4.3 URLBar 컴포넌트 수정 불필요
F-03에서 구현된 URLBar는 수정 없이 그대로 사용 가능합니다. `validateAndFormatUrl` 함수가 검색 URL을 반환하므로 URLBar는 이를 일반 URL로 간주하여 WebView에 전달합니다.

```javascript
// src/components/URLBar/URLBar.js (기존 코드, 수정 불필요)

const handleSubmit = useCallback(() => {
	// 자동완성 항목 선택 중이면 해당 항목 사용
	if (selectedIndex >= 0 && suggestions[selectedIndex]) {
		const selected = suggestions[selectedIndex]
		setInputValue(selected.url)
		onSuggestionSelect(selected)
		logger.info('[URLBar] 자동완성 항목 선택:', selected.url)
	}

	// URL 검증 및 제출 (검색 URL 포함)
	const validatedUrl = validateAndFormatUrl(inputValue)
	if (validatedUrl) {
		onSubmit(validatedUrl)
		setShowKeyboard(false)
		setShowSuggestions(false)
		logger.info('[URLBar] URL 제출:', validatedUrl)
	} else {
		// 에러 메시지 표시 (F-10 에러 처리에서 구현)
		logger.warn('[URLBar] 유효하지 않은 URL:', inputValue)
		// TODO: 에러 UI 표시
	}
}, [inputValue, selectedIndex, suggestions, onSubmit, onSuggestionSelect])
```

## 5. 데이터 모델

### localStorage 데이터 구조
```javascript
// Key: 'defaultSearchEngine'
// Value: 'google' | 'naver' | 'bing' | 'duckduckgo'
// 기본값: 'google' (코드 내 상수)

// 예시
localStorage.setItem('defaultSearchEngine', 'naver')
```

### 검색 히스토리 항목 (F-08 연동 시)
```javascript
// historyService에 저장되는 검색 히스토리 형식
{
	url: 'https://www.google.com/search?q=%EB%84%A4%EC%9D%B4%EB%B2%84',
	title: 'Google: 네이버',  // 검색 엔진 이름 + 검색어
	visitedAt: 1707825600000,  // 타임스탬프
	type: 'search'  // 히스토리 타입 (일반 페이지와 구분)
}
```

히스토리 제목 생성 로직 (F-08 구현 시):
```javascript
// src/services/historyService.js (F-08 구현 시 추가)

import { getSearchEngineName } from './searchService'

/**
 * 검색 URL인지 감지 및 제목 생성
 */
const generateHistoryTitle = (url, pageTitle) => {
	// 검색 URL 패턴 감지
	const searchPatterns = {
		google: /google\.com\/search\?.*q=([^&]+)/,
		naver: /search\.naver\.com\/search\.naver\?.*query=([^&]+)/,
		bing: /bing\.com\/search\?.*q=([^&]+)/,
		duckduckgo: /duckduckgo\.com\/\?.*q=([^&]+)/
	}

	for (const [engineId, pattern] of Object.entries(searchPatterns)) {
		const match = url.match(pattern)
		if (match) {
			const query = decodeURIComponent(match[1])
			const engineName = getSearchEngineName(engineId)
			return `${engineName}: ${query}`
		}
	}

	// 일반 페이지면 페이지 제목 사용
	return pageTitle || url
}
```

## 6. 시퀀스 다이어그램

### 주요 시나리오: 검색어 입력 → 검색 결과 로드
```
사용자      URLBar      urlValidator    searchService    WebView
  │            │               │                │             │
  │ "youtube" 입력              │                │             │
  │───────────▶│               │                │             │
  │            │ onChange("youtube")            │             │
  │            │               │                │             │
  │ 선택 (확인) │               │                │             │
  │───────────▶│               │                │             │
  │            │ validateAndFormatUrl("youtube")│             │
  │            │──────────────▶│                │             │
  │            │               │ 1. 프로토콜 체크 (실패)       │
  │            │               │ 2. 도메인 체크 (실패)         │
  │            │               │ 3. localhost 체크 (실패)      │
  │            │               │ 4. 검색어로 간주              │
  │            │               │                │             │
  │            │               │ buildSearchUrl("youtube")    │
  │            │               │───────────────▶│             │
  │            │               │                │ getDefaultSearchEngine()
  │            │               │                │ → 'google'
  │            │               │                │ encodeURIComponent("youtube")
  │            │               │                │ → 'youtube'
  │            │               │ searchUrl      │
  │            │               │◀───────────────│             │
  │            │               │ 'https://www.google.com/search?q=youtube'
  │            │  searchUrl    │                │             │
  │            │◀──────────────│                │             │
  │            │ onSubmit(searchUrl)            │             │
  │            │────────────────────────────────┼────────────▶│
  │            │ 키보드 닫기    │                │             │
  │            │               │                │  페이지 로드 │
  │            │               │                │  (Google 검색 결과)
```

### URL과 검색어 혼합 시나리오
```
입력값                   URL 검증 결과                      최종 동작
---------------------------------------------------------------------
"google.com"        → URL 형식 (도메인)                 → http://google.com 로드
"youtube"           → 검색어                           → Google 검색: youtube
"https://naver.com" → URL 형식 (프로토콜 포함)          → https://naver.com 로드
"고양이 동영상"      → 검색어 (공백 포함)                → Google 검색: 고양이 동영상
"192.168.1.1"       → URL 형식 (IP 주소)               → http://192.168.1.1 로드
"날씨 서울"         → 검색어 (한글 공백)                → Google 검색: 날씨 서울
""                  → null (빈 문자열)                 → 에러 메시지
```

### 검색 엔진 설정 변경 (F-11 연동 시)
```
사용자      SettingsPanel    searchService    localStorage
  │              │                  │                │
  │ 설정 화면 진입 │                  │                │
  │─────────────▶│                  │                │
  │              │ getAllSearchEngines()             │
  │              │─────────────────▶│                │
  │              │ [google, naver, ...]              │
  │              │◀─────────────────│                │
  │              │ getDefaultSearchEngine()          │
  │              │─────────────────▶│                │
  │              │                  │ getItem('defaultSearchEngine')
  │              │                  │───────────────▶│
  │              │                  │   'google'     │
  │              │                  │◀───────────────│
  │              │   'google'       │                │
  │              │◀─────────────────│                │
  │              │ 현재 선택: Google (라디오 버튼 체크)│
  │              │                  │                │
  │ 'naver' 선택  │                  │                │
  │─────────────▶│                  │                │
  │              │ setDefaultSearchEngine('naver')  │
  │              │─────────────────▶│                │
  │              │                  │ setItem('defaultSearchEngine', 'naver')
  │              │                  │───────────────▶│
  │              │   true (성공)     │                │
  │              │◀─────────────────│                │
  │              │ 설정 저장 완료 메시지              │
  │◀─────────────│                  │                │
```

## 7. F-11 설정 화면 인터페이스 (연동 준비)

F-09는 검색 엔진 로직만 구현하고, F-11에서 설정 UI를 추가합니다. 설정 UI에서 사용할 인터페이스는 searchService에서 제공합니다.

### 설정 화면에서 사용할 API
```javascript
// src/views/SettingsPanel.js (F-11에서 구현)

import {
	getAllSearchEngines,
	getDefaultSearchEngine,
	setDefaultSearchEngine
} from '../services/searchService'

const SettingsPanel = () => {
	const [engines] = useState(getAllSearchEngines())  // 검색 엔진 목록
	const [currentEngine, setCurrentEngine] = useState(getDefaultSearchEngine())

	const handleEngineChange = (engineId) => {
		const success = setDefaultSearchEngine(engineId)
		if (success) {
			setCurrentEngine(engineId)
			// 성공 메시지 표시 (F-10 연동)
		} else {
			// 에러 메시지 표시
		}
	}

	return (
		<Panel>
			<Header title="설정" />
			<div>
				<h3>기본 검색 엔진</h3>
				{engines.map(engine => (
					<RadioItem
						key={engine.id}
						selected={currentEngine === engine.id}
						onToggle={() => handleEngineChange(engine.id)}
					>
						{engine.name}
					</RadioItem>
				))}
			</div>
		</Panel>
	)
}
```

### 설정 UI 예시 (F-11 구현 시)
```
┌────────────────────────────────────┐
│             설정                    │
├────────────────────────────────────┤
│                                     │
│  기본 검색 엔진                      │
│  ◉ Google                           │
│  ○ Naver                            │
│  ○ Bing                             │
│  ○ DuckDuckGo                       │
│                                     │
│  [ 홈페이지 설정 ]                   │
│  [ 쿠키 및 캐시 삭제 ]                │
│  ...                                │
└────────────────────────────────────┘
```

## 8. 파일 구조

### 신규 파일
```
src/
├── services/
│   └── searchService.js         # 검색 엔진 관리 서비스 (신규)
└── utils/
    └── urlValidator.js          # URL 검증 유틸리티 (확장)
```

### 수정 파일
```
src/
└── utils/
    └── urlValidator.js          # 검색어 감지 로직 추가
```

### F-11 연동 시 추가 파일 (예정)
```
src/
├── views/
│   └── SettingsPanel.js         # 설정 화면 (F-11에서 구현)
└── services/
    └── historyService.js        # 검색 히스토리 저장 (F-08 확장)
```

## 9. 영향 범위 분석

### 수정 필요한 기존 파일
- **`src/utils/urlValidator.js`**:
  - 검색어 감지 로직 추가 (4단계: 검색 URL 생성)
  - searchService import 추가

### 새로 생성할 파일
- **`src/services/searchService.js`**: 검색 엔진 관리 서비스
  - 검색 엔진 정의 (`SEARCH_ENGINES`)
  - 검색 URL 생성 (`buildSearchUrl`)
  - 설정 관리 (`getDefaultSearchEngine`, `setDefaultSearchEngine`)
  - 검색 엔진 목록 조회 (`getAllSearchEngines`)

### 영향 받는 기존 기능
- **F-03 (URL 입력 UI)**: URLBar 컴포넌트는 수정 불필요, urlValidator만 확장
- **F-02 (웹뷰 통합)**: 검색 URL도 일반 URL로 간주하여 WebView에 로드
- **F-08 (히스토리 관리)**: (연동 시) 검색 히스토리 저장 (`type: 'search'` 필드 추가)
- **F-07 (북마크 관리)**: (연동 시) 자동완성에서 검색 히스토리 제안
- **F-11 (설정 화면)**: (연동 시) 기본 검색 엔진 선택 UI 추가

### 영향 없는 기능
- **F-04 (페이지 탐색)**: NavigationBar는 검색 URL도 일반 URL로 처리
- **F-05 (로딩 인디케이터)**: 검색 결과 페이지도 일반 페이지로 로딩 표시
- **F-06 (탭 관리)**: (미구현) 검색 URL도 일반 URL로 탭에 저장

## 10. 기술 스택

### 핵심 기술
- **JavaScript ES6+**: 모듈, 화살표 함수, 템플릿 리터럴
- **localStorage**: 검색 엔진 설정 저장
- **encodeURIComponent()**: 검색어 URL 인코딩

### 의존성
- **기존 F-03 구현**: urlValidator, URLBar 컴포넌트
- **기존 F-02 구현**: WebView 컴포넌트
- **logger 유틸리티**: 로깅

### 외부 API
- **검색 엔진 API**: 없음 (검색 엔진의 웹 UI를 WebView에서 렌더링)
- **webOS API**: 없음 (localStorage만 사용)

## 11. 주의사항 및 제약조건

### 보안
- **XSS 방지**: 검색어는 `encodeURIComponent()`로 인코딩하여 URL 인젝션 방지
  ```javascript
  // 위험: 직접 문자열 삽입
  const badUrl = `https://google.com/search?q=${query}`  // XSS 가능

  // 안전: 인코딩 후 삽입
  const safeUrl = `https://google.com/search?q=${encodeURIComponent(query)}`
  ```
- **HTTPS 전용**: 모든 검색 엔진은 HTTPS만 지원 (HTTP 검색 엔진 미지원)

### URL 최대 길이 제한
- **제약**: 브라우저 URL 길이 제한 (일반적으로 2048자)
- **대응**:
  - 검색어가 매우 긴 경우 (1000자 이상) URL 인코딩 후 길이 초과 가능
  - 현재는 제한 없이 처리 (실무에서는 드문 케이스)
  - 향후 필요 시 검색어 자르기 로직 추가

### 검색 엔진 렌더링 호환성
- **제약**: 일부 검색 엔진은 webOS WebView에서 JavaScript 실행 제한으로 정상 동작하지 않을 수 있음
- **테스트 필요**:
  - Google: 기본 렌더링 테스트 필수
  - Naver: 기본 렌더링 테스트 필수
  - Bing, DuckDuckGo: 추가 테스트 권장
- **대응**: 렌더링 실패 시 해당 검색 엔진 제거 또는 경고 메시지 표시

### 검색어와 도메인 구분 모호성
- **문제**: "apple"은 검색어인가 apple.com 도메인인가?
- **현재 로직**:
  - **URL 우선**: 점(.) 포함 시 도메인으로 처리 (예: "apple.com" → http://apple.com)
  - **검색어 후순위**: 점 없는 단일 단어는 검색어로 처리 (예: "apple" → Google 검색: apple)
- **트레이드오프**: 사용자가 apple.com을 의도했지만 "apple"만 입력하면 검색 결과로 이동
  - 하지만 검색 결과에서 apple.com 링크를 클릭할 수 있으므로 큰 문제 없음

### 특수문자 인코딩
- **제약**: `encodeURIComponent()`가 일부 특수문자를 예상과 다르게 인코딩할 수 있음
  ```javascript
  encodeURIComponent('hello world')  // → 'hello%20world'
  encodeURIComponent('고양이')        // → '%EA%B3%A0%EC%96%91%EC%9D%B4'
  encodeURIComponent('a+b')          // → 'a%2Bb'
  ```
- **대응**: 모든 검색 엔진은 표준 URL 인코딩을 지원하므로 문제 없음

### localStorage 제약사항
- **제약**: webOS WebView는 localStorage를 지원하지만, 앱 데이터 삭제 시 초기화됨
- **대응**:
  - 설정 초기화 시 Google을 기본 검색 엔진으로 사용
  - 향후 LS2 API로 전환 가능 (확장성 고려)

### 검색 히스토리 중복
- **문제**: 같은 검색어를 반복 입력 시 히스토리에 중복 저장될 수 있음
- **대응**: F-08 historyService에서 중복 제거 로직 구현 (URL 기준 최신 항목만 유지)

## 12. 테스트 시나리오

### 단위 테스트 (searchService.js)
```javascript
// src/__tests__/services/searchService.test.js

describe('searchService', () => {
	test('buildSearchUrl - Google 검색', () => {
		const url = buildSearchUrl('youtube', 'google')
		expect(url).toBe('https://www.google.com/search?q=youtube')
	})

	test('buildSearchUrl - Naver 검색', () => {
		const url = buildSearchUrl('네이버', 'naver')
		expect(url).toBe('https://search.naver.com/search.naver?query=%EB%84%A4%EC%9D%B4%EB%B2%84')
	})

	test('buildSearchUrl - 공백 포함 검색어', () => {
		const url = buildSearchUrl('고양이 동영상', 'google')
		expect(url).toBe('https://www.google.com/search?q=%EA%B3%A0%EC%96%91%EC%9D%B4%20%EB%8F%99%EC%98%81%EC%83%81')
	})

	test('buildSearchUrl - 빈 검색어', () => {
		const url = buildSearchUrl('', 'google')
		expect(url).toBeNull()
	})

	test('buildSearchUrl - 알 수 없는 검색 엔진', () => {
		const url = buildSearchUrl('test', 'unknown')
		expect(url).toBe('https://www.google.com/search?q=test')  // 폴백: Google
	})

	test('getDefaultSearchEngine - 저장된 값', () => {
		localStorage.setItem('defaultSearchEngine', 'naver')
		const engine = getDefaultSearchEngine()
		expect(engine).toBe('naver')
	})

	test('getDefaultSearchEngine - 기본값', () => {
		localStorage.removeItem('defaultSearchEngine')
		const engine = getDefaultSearchEngine()
		expect(engine).toBe('google')
	})

	test('setDefaultSearchEngine - 성공', () => {
		const result = setDefaultSearchEngine('bing')
		expect(result).toBe(true)
		expect(localStorage.getItem('defaultSearchEngine')).toBe('bing')
	})

	test('setDefaultSearchEngine - 실패 (유효하지 않은 엔진)', () => {
		const result = setDefaultSearchEngine('invalid')
		expect(result).toBe(false)
	})
})
```

### 단위 테스트 (urlValidator.js 확장)
```javascript
// src/__tests__/utils/urlValidator.test.js (기존 테스트에 추가)

describe('urlValidator - 검색어 처리', () => {
	test('검색어 인식 - 단일 단어', () => {
		const url = validateAndFormatUrl('youtube')
		expect(url).toBe('https://www.google.com/search?q=youtube')
	})

	test('검색어 인식 - 복수 단어', () => {
		const url = validateAndFormatUrl('고양이 동영상')
		expect(url).toContain('google.com/search?q=')
		expect(url).toContain('%EA%B3%A0%EC%96%91%EC%9D%B4')
	})

	test('URL 우선 - 도메인 형식', () => {
		const url = validateAndFormatUrl('apple.com')
		expect(url).toBe('http://apple.com')
		expect(url).not.toContain('search')
	})

	test('URL 우선 - 프로토콜 포함', () => {
		const url = validateAndFormatUrl('https://naver.com')
		expect(url).toBe('https://naver.com')
		expect(url).not.toContain('search')
	})
})
```

### 통합 테스트 (BrowserView)
```javascript
// src/__tests__/views/BrowserView.test.js

describe('BrowserView - 검색 엔진 통합', () => {
	test('검색어 입력 → 검색 URL 생성 → WebView 로드', async () => {
		const { getByPlaceholderText, getByText } = render(<BrowserView />)

		// URLBar에 검색어 입력
		const input = getByPlaceholderText('URL을 입력하세요')
		fireEvent.change(input, { target: { value: 'youtube' } })

		// 가상 키보드에서 확인 버튼 클릭
		const confirmButton = getByText('확인')
		fireEvent.click(confirmButton)

		// WebView에 검색 URL 전달 확인
		await waitFor(() => {
			expect(input.value).toContain('google.com/search?q=youtube')
		})
	})
})
```

### 실제 디바이스 테스트
| 테스트 케이스 | 입력값 | 예상 결과 |
|-------------|-------|----------|
| URL 입력 | `google.com` | http://google.com 로드 |
| 검색어 (영문) | `youtube` | Google 검색: youtube |
| 검색어 (한글) | `네이버` | Google 검색: 네이버 |
| 검색어 (공백) | `고양이 동영상` | Google 검색: 고양이 동영상 |
| 검색어 (특수문자) | `a+b=c` | Google 검색: a+b=c |
| 빈 입력 | `` | 에러 메시지 표시 |
| 프로토콜 포함 | `https://naver.com` | https://naver.com 로드 |
| IP 주소 | `192.168.1.1` | http://192.168.1.1 로드 |

### 검색 엔진별 렌더링 테스트
| 검색 엔진 | 테스트 URL | 예상 동작 |
|---------|-----------|----------|
| Google | `https://www.google.com/search?q=test` | 검색 결과 정상 표시 |
| Naver | `https://search.naver.com/search.naver?query=test` | 검색 결과 정상 표시 |
| Bing | `https://www.bing.com/search?q=test` | 검색 결과 정상 표시 |
| DuckDuckGo | `https://duckduckgo.com/?q=test` | 검색 결과 정상 표시 |

## 13. 확장성 고려사항

### 검색 엔진 추가
새로운 검색 엔진 추가 시 `SEARCH_ENGINES` 상수만 수정:
```javascript
// src/services/searchService.js

export const SEARCH_ENGINES = {
	// 기존 검색 엔진
	google: { ... },
	naver: { ... },

	// 신규 검색 엔진 추가
	yahoo: {
		id: 'yahoo',
		name: 'Yahoo',
		urlTemplate: 'https://search.yahoo.com/search?p={query}',
		icon: '/assets/icons/yahoo.png'
	}
}
```

### 검색 엔진별 추가 파라미터
검색 엔진마다 다른 쿼리 파라미터를 지원하려면:
```javascript
export const SEARCH_ENGINES = {
	google: {
		id: 'google',
		name: 'Google',
		urlTemplate: 'https://www.google.com/search?q={query}&hl=ko',  // 언어 파라미터 추가
		icon: '/assets/icons/google.png'
	},
	naver: {
		id: 'naver',
		name: 'Naver',
		urlTemplate: 'https://search.naver.com/search.naver?query={query}&where=nexearch',  // 통합검색 파라미터
		icon: '/assets/icons/naver.png'
	}
}
```

### 검색 제안 API 연동 (추후 확장)
검색 엔진의 자동완성 제안 API를 연동하려면:
```javascript
// src/services/searchService.js (향후 확장)

/**
 * 검색 제안 조회
 * @param {string} query - 검색어
 * @param {string} engineId - 검색 엔진 ID
 * @returns {Promise<Array>} - 제안 목록
 */
export const fetchSearchSuggestions = async (query, engineId = 'google') => {
	const suggestionApis = {
		google: `https://suggestqueries.google.com/complete/search?client=firefox&q=${encodeURIComponent(query)}`,
		naver: `https://ac.search.naver.com/nx/ac?q=${encodeURIComponent(query)}`
	}

	const apiUrl = suggestionApis[engineId]
	if (!apiUrl) return []

	try {
		const response = await fetch(apiUrl)
		const data = await response.json()
		return data[1] || []  // Google은 [query, [suggestions]] 형식
	} catch (error) {
		logger.error('[searchService] 검색 제안 조회 실패:', error)
		return []
	}
}
```

### 음성 검색 (추후 확장)
webOS VoiceService API를 연동하여 음성 검색 지원:
```javascript
// src/services/voiceService.js (향후 확장)

import { buildSearchUrl } from './searchService'

export const startVoiceSearch = (callback) => {
	// webOS VoiceService API 호출
	window.webOSVoice.startRecognition({
		onSuccess: (text) => {
			const searchUrl = buildSearchUrl(text)
			callback(searchUrl)
		},
		onError: (error) => {
			logger.error('[voiceService] 음성 인식 실패:', error)
		}
	})
}
```

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-13 | 최초 작성 | F-09 요구사항 기반 기술 설계 |
