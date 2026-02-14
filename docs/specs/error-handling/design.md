# 에러 처리 — 기술 설계서

## 1. 참조
- 요구사항 분석서: `docs/specs/error-handling/requirements.md`

---

## 2. 아키텍처 결정

### 결정 1: ErrorPage 컴포넌트 위치
- **선택지**:
  - A) WebView 내부에 인라인으로 유지 (현재 구현)
  - B) 독립 컴포넌트로 분리 (`src/components/ErrorPage/ErrorPage.js`)
  - C) BrowserView에서 직접 렌더링
- **결정**: **B) 독립 컴포넌트로 분리**
- **근거**:
  - 재사용성: 향후 다른 화면(예: SettingsPanel)에서도 에러 화면 표시 가능
  - 관심사 분리: WebView는 iframe 관리에 집중, ErrorPage는 에러 UI에 집중
  - 테스트 용이성: 독립 컴포넌트로 단위 테스트 작성 가능
  - 코드 가독성: WebView.js 파일 크기 축소 (현재 445줄 → 약 350줄)
- **트레이드오프**:
  - 포기: WebView 내부에서 완결된 에러 처리 (단순성)
  - 획득: 재사용성, 테스트 용이성, 유지보수성

### 결정 2: 에러 상태 관리 위치
- **선택지**:
  - A) WebView 로컬 상태 (useState)
  - B) TabContext에 에러 정보 저장
  - C) WebView 로컬 상태 + TabContext에 isError 플래그만 저장
- **결정**: **C) WebView 로컬 상태 + TabContext에 isError 플래그만 저장**
- **근거**:
  - 에러 정보(errorCode, errorMessage)는 일시적 상태 → WebView 로컬 상태로 충분
  - TabContext에는 isError 플래그만 저장하여 탭 전환 시 에러 상태 유지
  - LoadingBar 컴포넌트가 isError를 참조하므로 TabContext 연동 필요
  - 메모리 최적화: 에러 정보는 탭 전환 시 폐기 (재진입 시 재생성)
- **트레이드오프**:
  - 포기: 에러 정보의 영속성 (탭 전환 시 재시도 불가)
  - 획득: 메모리 최적화, 구현 단순성, LoadingBar 연동

### 결정 3: 에러 타입 감지 메커니즘
- **선택지**:
  - A) iframe onError만 사용 (HTTP 상태 코드 접근 불가)
  - B) 타임아웃 + onError + 휴리스틱 (페이지 타이틀로 404 판단)
  - C) webOS LS2 API로 네트워크 상태 모니터링
- **결정**: **B) 타임아웃 + onError + 휴리스틱**
- **근거**:
  - iframe 제약: Same-Origin Policy로 인해 HTTP 상태 코드 접근 불가
  - 휴리스틱 방식: 페이지 타이틀에 "404", "500", "Not Found" 등의 키워드 감지
  - 타임아웃: 30초 초과 시 타임아웃 에러로 처리 (현재 구현 유지)
  - LS2 API는 과도한 복잡성 (네트워크 상태는 onError로 감지 가능)
- **트레이드오프**:
  - 포기: 정확한 HTTP 상태 코드 (404, 500 등)
  - 획득: iframe 제약 하에서 최선의 에러 타입 분류

### 결정 4: 홈 버튼 표시 여부
- **선택지**:
  - A) 재시도 버튼만 표시 (단순)
  - B) 재시도 + 홈 버튼 (2개 버튼)
  - C) 재시도 + 홈 + 뒤로 버튼 (3개 버튼)
- **결정**: **B) 재시도 + 홈 버튼 (2개 버튼)**
- **근거**:
  - 사용자 경험: 재시도 실패 시 홈으로 이동할 수 있는 대안 제공
  - 리모컨 조작: 좌/우 방향키로 2개 버튼 전환 (단순)
  - 뒤로 버튼은 불필요: NavigationBar의 뒤로 버튼과 중복
- **트레이드오프**:
  - 포기: 단순한 UI (버튼 1개)
  - 획득: 사용자 편의성 (대안 동선 제공)

### 결정 5: 에러 화면 애니메이션
- **선택지**:
  - A) 애니메이션 없음 (즉시 표시/제거)
  - B) 페이드인/아웃만 (CSS transition)
  - C) 슬라이드 + 페이드인/아웃 (복잡)
- **결정**: **B) 페이드인/아웃만 (CSS transition)**
- **근거**:
  - UX: 부드러운 전환으로 사용자 경험 개선
  - 성능: CSS transition은 GPU 가속 사용 (opacity, transform)
  - 리모컨 최적화: 슬라이드 애니메이션은 대화면에서 시각적 혼란 야기 가능
- **트레이드오ffe**:
  - 포기: 화려한 애니메이션
  - 획득: 부드러운 UX, 성능 최적화

---

## 3. 컴포넌트 아키텍처

### 3.1 컴포넌트 트리

```
BrowserView
├── URLBar
├── TabBar
├── LoadingBar (isError 반영)
├── WebView
│   ├── iframe
│   ├── LoadingSpinner (isError=true 시 빨간색)
│   └── ErrorPage (새로 분리) ← F-10
│       ├── 에러 아이콘
│       ├── 에러 제목
│       ├── 에러 메시지
│       ├── URL 표시
│       ├── 재시도 버튼 (Spottable)
│       └── 홈 버튼 (Spottable)
└── NavigationBar
```

### 3.2 ErrorPage 컴포넌트 Props 인터페이스

```javascript
ErrorPage.propTypes = {
	// 에러 정보
	errorCode: PropTypes.number.isRequired,     // -1: 네트워크, -2: 타임아웃, -99: 알 수 없음
	errorMessage: PropTypes.string.isRequired,  // 사용자 친화적 메시지
	url: PropTypes.string.isRequired,           // 실패한 URL

	// 버튼 핸들러
	onRetry: PropTypes.func.isRequired,         // 재시도 버튼 클릭 시
	onGoHome: PropTypes.func.isRequired,        // 홈 버튼 클릭 시

	// 스타일 커스터마이징
	className: PropTypes.string
}
```

### 3.3 WebView 수정사항

현재 WebView.js의 에러 처리 로직을 다음과 같이 개선:

1. **에러 상태 변수 추가**:
   ```javascript
   const [error, setError] = useState(null)
   // { errorCode, errorMessage, url }
   ```

2. **기존 인라인 에러 UI 제거** (377~397줄):
   ```javascript
   {/* 에러 상태 표시 */}
   {state === 'error' && error && (
       <ErrorPage
           errorCode={error.errorCode}
           errorMessage={error.errorMessage}
           url={error.url}
           onRetry={handleRetry}
           onGoHome={handleGoHome}
       />
   )}
   ```

3. **handleGoHome 핸들러 추가**:
   ```javascript
   const handleGoHome = useCallback((homeUrl) => {
       logger.info('[WebView] 홈 버튼 클릭')
       if (iframeRef.current) {
           iframeRef.current.src = homeUrl
           changeState('loading')
           loadStartTimeRef.current = Date.now()
           if (onLoadStart) {
               onLoadStart()
           }
           timeoutRef.current = setTimeout(handleTimeout, 30000)
       }
   }, [changeState, onLoadStart, handleTimeout])
   ```

4. **PropTypes에 homeUrl 추가**:
   ```javascript
   WebView.propTypes = {
       // ... 기존 props
       homeUrl: PropTypes.string  // 홈 버튼 클릭 시 이동할 URL
   }

   WebView.defaultProps = {
       // ... 기존 defaults
       homeUrl: 'https://www.google.com'
   }
   ```

### 3.4 BrowserView 수정사항

WebView에 homeUrl prop 전달:

```javascript
<WebView
    ref={webviewRef}
    url={currentUrl}
    homeUrl={homeUrl}  // ← 추가
    onLoadStart={handleLoadStart}
    onLoadEnd={handleLoadEnd}
    onLoadError={handleLoadError}
    onNavigationChange={handleNavigationChange}
    onStateChange={handleStateChange}
/>
```

---

## 4. 에러 타입 분류 로직

### 4.1 에러 감지 시나리오

| 시나리오 | 감지 방법 | errorCode | errorMessage |
|----------|----------|-----------|--------------|
| 네트워크 에러 | iframe onError | -1 | "네트워크 연결을 확인해주세요" |
| 타임아웃 | 30초 타이머 | -2 | "페이지 로딩 시간이 초과되었습니다 (30초)" |
| CORS 에러 | 페이지 로딩 후 iframe.contentDocument 접근 실패 | -3 | "이 페이지는 보안 정책으로 인해 표시할 수 없습니다" |
| 404 에러 (휴리스틱) | 페이지 타이틀에 "404", "Not Found" 포함 | 404 | "페이지를 찾을 수 없습니다" |
| 5xx 에러 (휴리스틱) | 페이지 타이틀에 "500", "503", "Server Error" 포함 | 500 | "서버에 일시적인 문제가 발생했습니다" |
| 알 수 없음 | 위에 해당 없음 | -99 | "페이지 로딩 중 오류가 발생했습니다" |

### 4.2 에러 타입 유틸리티 함수

새 파일 생성: `src/utils/errorClassifier.js`

```javascript
/**
 * 에러 타입 분류 유틸리티
 * iframe 제약 하에서 휴리스틱 방식으로 에러 타입 추론
 */

/**
 * 페이지 타이틀 기반 에러 타입 추론
 * @param {string} title - iframe 페이지 타이틀
 * @returns {{ errorCode: number, errorMessage: string } | null}
 */
export const classifyErrorByTitle = (title) => {
	if (!title) return null

	const lowerTitle = title.toLowerCase()

	// 404 에러 감지
	if (lowerTitle.includes('404') || lowerTitle.includes('not found')) {
		return {
			errorCode: 404,
			errorMessage: '페이지를 찾을 수 없습니다'
		}
	}

	// 5xx 에러 감지
	if (lowerTitle.includes('500') || lowerTitle.includes('503') || lowerTitle.includes('server error')) {
		return {
			errorCode: 500,
			errorMessage: '서버에 일시적인 문제가 발생했습니다'
		}
	}

	return null
}

/**
 * 에러 코드에 따른 메시지 생성
 * @param {number} errorCode
 * @returns {string}
 */
export const getErrorMessage = (errorCode) => {
	switch (errorCode) {
		case -1:
			return '네트워크 연결을 확인해주세요'
		case -2:
			return '페이지 로딩 시간이 초과되었습니다 (30초)'
		case -3:
			return '이 페이지는 보안 정책으로 인해 표시할 수 없습니다'
		case 404:
			return '페이지를 찾을 수 없습니다'
		case 500:
		case 503:
			return '서버에 일시적인 문제가 발생했습니다'
		default:
			return '페이지 로딩 중 오류가 발생했습니다'
	}
}

/**
 * 에러 코드에 따른 제목 생성
 * @param {number} errorCode
 * @returns {string}
 */
export const getErrorTitle = (errorCode) => {
	switch (errorCode) {
		case -1:
			return '연결 실패'
		case -2:
			return '로딩 시간 초과'
		case -3:
			return '접근 제한'
		case 404:
			return '페이지 없음'
		case 500:
		case 503:
			return '서버 오류'
		default:
			return '페이지 로딩 실패'
	}
}
```

### 4.3 WebView handleLoad 수정 (휴리스틱 에러 감지)

```javascript
const handleLoad = useCallback(() => {
	// 타임아웃 타이머 정리
	if (timeoutRef.current) {
		clearTimeout(timeoutRef.current)
		timeoutRef.current = null
	}

	// 페이지 제목 추출
	const title = extractTitle()

	// 휴리스틱 에러 감지 (404, 500 등)
	const heuristicError = classifyErrorByTitle(title)
	if (heuristicError) {
		// 에러 상태로 전환
		const errorInfo = {
			...heuristicError,
			url: currentUrlRef.current
		}
		changeState('error', errorInfo)

		if (onLoadError) {
			onLoadError(errorInfo)
		}

		logger.warn('[WebView] 휴리스틱 에러 감지:', errorInfo)
		return  // 정상 로딩 처리 중단
	}

	// 정상 로딩 완료
	changeState('loaded')

	const duration = Date.now() - loadStartTimeRef.current

	if (onLoadEnd) {
		onLoadEnd({ url: currentUrlRef.current, title, duration })
	}

	logger.info(`[WebView] 페이지 로딩 완료: ${currentUrlRef.current} (${duration}ms)`)
}, [changeState, extractTitle, onLoadEnd, onLoadError])
```

---

## 5. UI/UX 설계

### 5.1 ErrorPage 레이아웃

```
┌─────────────────────────────────────────────────┐
│                                                 │
│                     ⚠️                          │  ← 에러 아이콘 (80x80px)
│                                                 │
│            페이지 로딩 실패                      │  ← 에러 제목 (48px, 굵게)
│                                                 │
│        네트워크 연결을 확인해주세요              │  ← 에러 메시지 (28px)
│                                                 │
│       URL: https://example.com/page             │  ← URL (22px, 회색, 최대 50자)
│                                                 │
│                                                 │
│       ┌──────────┐     ┌──────────┐            │
│       │ 다시 시도 │     │ 홈으로    │            │  ← 버튼 (200x60px)
│       └──────────┘     └──────────┘            │
│           ↑                                     │  ← 초기 포커스
│                                                 │
└─────────────────────────────────────────────────┘
```

### 5.2 Moonstone 컴포넌트 사용

- **버튼**: `@enact/moonstone/Button` (Spottable 기본 지원)
- **아이콘**: CSS 기반 (⚠️ 이모지 또는 SVG)
- **레이아웃**: Flexbox (중앙 정렬)

### 5.3 Spotlight 포커스 흐름

```
초기 진입
    ↓
[다시 시도] ←→ [홈으로]  (좌/우 방향키)
    ↓
선택 버튼: 해당 동작 실행
백 키: 에러 화면 유지 (포커스 이탈 불가)
```

### 5.4 스타일 설계

새 파일 생성: `src/components/ErrorPage/ErrorPage.module.less`

```less
/**
 * ErrorPage 스타일
 */

.errorPageContainer {
	position: absolute;
	top: 0;
	left: 0;
	width: 100%;
	height: 100%;
	background-color: rgba(0, 0, 0, 0.9);  // 반투명 검은색
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	z-index: 20;  // WebView 위에 표시 (LoadingSpinner보다 위)
	padding: 60px;
	text-align: center;

	// 페이드인 애니메이션
	animation: fadeIn 0.3s ease-out;
}

@keyframes fadeIn {
	from {
		opacity: 0;
	}
	to {
		opacity: 1;
	}
}

.errorPageContainer.fadeOut {
	opacity: 0;
	transition: opacity 0.2s ease-out;
}

.errorIcon {
	font-size: 80px;
	margin-bottom: 30px;
	// 이모지 대신 SVG 사용 시:
	// width: 80px;
	// height: 80px;
	// fill: #FFD700;  // 노란색 경고 아이콘
}

.errorTitle {
	color: #ffffff;
	font-size: 48px;
	font-weight: bold;
	margin-bottom: 20px;
	line-height: 1.2;
}

.errorMessage {
	color: #cccccc;
	font-size: 28px;
	margin-bottom: 16px;
	line-height: 1.4;
	max-width: 800px;  // 메시지 최대 너비
}

.errorUrl {
	color: #999999;
	font-size: 22px;
	margin-bottom: 40px;
	word-break: break-all;
	max-width: 900px;
	overflow: hidden;
	text-overflow: ellipsis;
	white-space: nowrap;
}

.buttonGroup {
	display: flex;
	gap: 20px;  // 버튼 간격
	align-items: center;
	justify-content: center;
}

.retryButton,
.homeButton {
	min-width: 200px;
	min-height: 60px;
	font-size: 24px;
}

// Spotlight 포커스 시 노란색 테두리 (Moonstone 기본 스타일 오버라이드)
.retryButton:focus,
.homeButton:focus {
	outline: 4px solid #FFD700;
	outline-offset: 4px;
}
```

---

## 6. 시퀀스 흐름

### 6.1 네트워크 에러 시나리오

```
사용자 → URLBar → BrowserView → WebView → iframe
                                     │
                                     │  onError 이벤트
                                     ↓
                              handleError()
                                     │
                                     │  setError({ errorCode: -1, ... })
                                     │  changeState('error')
                                     ↓
                              ErrorPage 렌더링
                                     │
                                     │  재시도 버튼 클릭
                                     ↓
                              handleRetry()
                                     │
                                     │  iframe.src = url
                                     │  changeState('loading')
                                     ↓
                              (다시 로딩 시작)
```

### 6.2 타임아웃 시나리오

```
WebView → iframe 로딩 시작
    │
    │  30초 경과
    ↓
setTimeout(handleTimeout, 30000)
    │
    │  타이머 트리거
    ↓
handleTimeout()
    │
    │  setError({ errorCode: -2, ... })
    │  changeState('error')
    ↓
ErrorPage 렌더링
    │
    │  홈 버튼 클릭
    ↓
handleGoHome(homeUrl)
    │
    │  iframe.src = homeUrl
    │  changeState('loading')
    ↓
(홈페이지 로딩 시작)
```

### 6.3 휴리스틱 404 감지 시나리오

```
WebView → iframe 로딩 완료
    │
    │  onLoad 이벤트
    ↓
handleLoad()
    │
    │  extractTitle() → "404 - Page Not Found"
    ↓
classifyErrorByTitle(title)
    │
    │  { errorCode: 404, errorMessage: "페이지를 찾을 수 없습니다" }
    ↓
changeState('error', errorInfo)
    │
    │  onLoadError(errorInfo)  // BrowserView로 전파
    ↓
ErrorPage 렌더링 (404 메시지)
```

### 6.4 에러 상태 → TabContext 전파

```
WebView.handleError()
    │
    │  onLoadError({ errorCode, errorMessage, url })
    ↓
BrowserView.handleLoadError()
    │
    │  dispatch({
    │    type: TAB_ACTIONS.UPDATE_TAB,
    │    payload: {
    │      id: activeTabId,
    │      updates: { isError: true, isLoading: false }
    │    }
    │  })
    ↓
TabContext.tabReducer()
    │
    │  state.tabs[activeTabId].isError = true
    ↓
LoadingBar 렌더링 (빨간색)
```

---

## 7. 영향 범위 분석

### 7.1 수정 필요한 기존 파일

| 파일 경로 | 변경 내용 | 이유 |
|----------|----------|------|
| `src/components/WebView/WebView.js` | 1. 인라인 에러 UI 제거 (377~397줄)<br>2. ErrorPage 컴포넌트 import 및 렌더링<br>3. handleGoHome 핸들러 추가<br>4. homeUrl prop 추가<br>5. handleLoad에 휴리스틱 에러 감지 추가 | ErrorPage 컴포넌트 분리, 홈 버튼 기능 추가, 404/500 감지 |
| `src/components/WebView/WebView.module.less` | 에러 관련 스타일 제거 (40~100줄) | ErrorPage로 스타일 이동 |
| `src/views/BrowserView.js` | WebView에 homeUrl prop 전달 (591줄) | 홈 버튼 클릭 시 이동할 URL 제공 |

### 7.2 새로 생성할 파일

| 파일 경로 | 역할 |
|----------|------|
| `src/components/ErrorPage/ErrorPage.js` | 에러 화면 UI 컴포넌트 (에러 정보 표시, 재시도/홈 버튼) |
| `src/components/ErrorPage/ErrorPage.module.less` | ErrorPage 스타일 (대화면 최적화, 애니메이션) |
| `src/components/ErrorPage/index.js` | export default (컴포넌트 진입점) |
| `src/utils/errorClassifier.js` | 에러 타입 분류 유틸리티 (휴리스틱 로직) |

### 7.3 영향 받는 기존 기능

| 기능명 | 영향 내용 | 대응 방안 |
|--------|----------|----------|
| F-05 LoadingBar | isError 플래그를 TabContext에서 읽어 빨간색 표시 | 변경 없음 (기존 동작 유지) |
| F-06 TabBar | 에러 탭 전환 시 에러 상태 유지 | TabContext에 isError 플래그 저장 (기존 구현 유지) |
| F-04 NavigationBar | 에러 발생 시 뒤로/앞으로 버튼 동작 | 변경 없음 (히스토리 스택 독립) |

---

## 8. 기술적 주의사항

### 8.1 iframe 제약 대응
- **HTTP 상태 코드 접근 불가**: iframe의 Same-Origin Policy로 인해 cross-origin 페이지의 HTTP 응답 헤더 접근 불가
- **대응 방안**: 페이지 타이틀 기반 휴리스틱 감지 (404, 500 등)
- **제약 수용**: 정확한 HTTP 상태 코드 대신 "추정" 에러 타입으로 처리

### 8.2 에러 로깅
- **로그 레벨**: `logger.error()` 사용 (네트워크 에러, 타임아웃)
- **로그 내용**:
  - 타임스탬프 (자동)
  - 에러 코드
  - 에러 메시지
  - 실패한 URL
  - 로딩 소요 시간
- **로그 예시**:
  ```javascript
  logger.error('[WebView] 페이지 로딩 실패:', {
      errorCode: -1,
      errorMessage: '네트워크 연결을 확인해주세요',
      url: 'https://example.com',
      duration: 5000  // ms
  })
  ```

### 8.3 메모리 최적화
- **에러 정보 폐기**: 탭 전환 시 WebView 로컬 상태(error)는 폐기
- **TabContext 최소화**: isError 플래그만 저장 (errorCode, errorMessage는 저장 안 함)
- **재시도 시 재생성**: 에러 탭 재진입 시 iframe src 변경으로 로딩 재시작

### 8.4 애니메이션 성능
- **GPU 가속 사용**: `opacity`, `transform` 속성만 사용
- **will-change 힌트**: 애니메이션 시작 전 브라우저에 힌트 제공
- **페이드인**: 300ms (부드러운 진입)
- **페이드아웃**: 200ms (빠른 제거)

### 8.5 보안 고려사항
- **URL 길이 제한**: 에러 화면에 표시할 URL은 최대 50자로 제한 (XSS 방지)
- **에러 메시지 고정**: 서버 에러 메시지를 그대로 표시하지 않고 고정 메시지 사용 (정보 노출 방지)
- **스택 트레이스 숨김**: 개발 환경에서도 스택 트레이스를 사용자에게 노출하지 않음 (로그만 기록)

### 8.6 리모컨 포커스 관리
- **초기 포커스**: ErrorPage 렌더링 시 재시도 버튼에 자동 포커스
- **포커스 트랩**: 에러 화면 내부에서만 포커스 이동 (외부로 이탈 불가)
- **Back 키 처리**: 에러 화면에서 Back 키 누를 시 에러 화면 유지 (포커스를 NavigationBar로 이동하지 않음)

---

## 9. 아키텍처 다이어그램

### 9.1 컴포넌트 계층 구조

```
┌─────────────────────────────────────────────────────┐
│                   BrowserView                       │
│  ┌───────────────────────────────────────────────┐  │
│  │ URLBar                                        │  │
│  ├───────────────────────────────────────────────┤  │
│  │ TabBar                                        │  │
│  ├───────────────────────────────────────────────┤  │
│  │ LoadingBar (isError → 빨간색)                 │  │
│  ├───────────────────────────────────────────────┤  │
│  │ WebView                                       │  │
│  │  ┌─────────────────────────────────────────┐  │  │
│  │  │ iframe                                  │  │  │
│  │  └─────────────────────────────────────────┘  │  │
│  │  ┌─────────────────────────────────────────┐  │  │
│  │  │ LoadingSpinner (state='loading')       │  │  │
│  │  └─────────────────────────────────────────┘  │  │
│  │  ┌─────────────────────────────────────────┐  │  │
│  │  │ ErrorPage (state='error') ← F-10       │  │  │
│  │  │  - 에러 아이콘                          │  │  │
│  │  │  - 에러 제목                            │  │  │
│  │  │  - 에러 메시지                          │  │  │
│  │  │  - URL                                  │  │  │
│  │  │  - [다시 시도] [홈으로]                │  │  │
│  │  └─────────────────────────────────────────┘  │  │
│  ├───────────────────────────────────────────────┤  │
│  │ NavigationBar                                 │  │
│  └───────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘

TabContext (전역 상태)
├── tabs[].isError ← WebView에서 업데이트
└── tabs[].isLoading
```

### 9.2 데이터 플로우

```
1. 에러 감지 (iframe onError / 타임아웃)
   WebView.handleError()
        ↓
   setError({ errorCode: -1, errorMessage: "네트워크 연결을 확인해주세요", url })
        ↓
   changeState('error')
        ↓
   onLoadError(errorInfo)  // BrowserView로 전파
        ↓
   BrowserView.handleLoadError()
        ↓
   dispatch({ type: UPDATE_TAB, payload: { isError: true } })
        ↓
   TabContext 업데이트
        ↓
   LoadingBar 렌더링 (빨간색)

2. 재시도 버튼 클릭
   ErrorPage → onRetry()
        ↓
   WebView.handleRetry()
        ↓
   iframe.src = currentUrlRef.current
        ↓
   changeState('loading')
        ↓
   LoadingBar 렌더링 (녹색)

3. 홈 버튼 클릭
   ErrorPage → onGoHome()
        ↓
   WebView.handleGoHome(homeUrl)
        ↓
   iframe.src = homeUrl
        ↓
   changeState('loading')
        ↓
   LoadingBar 렌더링 (녹색)
```

### 9.3 상태 전이 다이어그램

```
┌──────┐
│ idle │
└──┬───┘
   │  URL 변경
   ↓
┌─────────┐
│ loading │ ←───────────────┐
└──┬───┬──┘                 │
   │   │                    │
   │   │  타임아웃 or       │  재시도 버튼 클릭
   │   │  onError           │  or 홈 버튼 클릭
   │   ↓                    │
   │  ┌───────┐             │
   │  │ error │─────────────┘
   │  └───────┘
   │   (ErrorPage 렌더링)
   │
   │  onLoad (정상)
   ↓
┌────────┐
│ loaded │
└────────┘
```

---

## 10. 테스트 시나리오

### 10.1 단위 테스트 (ErrorPage 컴포넌트)

| 테스트 케이스 | 입력 | 예상 출력 |
|--------------|------|----------|
| 재시도 버튼 클릭 | onRetry mock 함수 | onRetry 호출됨 |
| 홈 버튼 클릭 | onGoHome mock 함수 | onGoHome 호출됨 |
| 에러 메시지 렌더링 | errorMessage prop | 메시지가 DOM에 표시됨 |
| URL 50자 제한 | url="https://example.com/very-long-url..." | 말줄임표로 축약 표시 |
| 초기 포커스 | ErrorPage 렌더링 | 재시도 버튼에 포커스 |

### 10.2 통합 테스트 (WebView + ErrorPage)

| 테스트 케이스 | 시나리오 | 예상 동작 |
|--------------|----------|----------|
| 네트워크 에러 | iframe onError 트리거 | ErrorPage 렌더링, errorCode=-1 |
| 타임아웃 | 30초 경과 | ErrorPage 렌더링, errorCode=-2 |
| 404 감지 | 페이지 타이틀 "404 Not Found" | ErrorPage 렌더링, errorCode=404 |
| 재시도 성공 | 재시도 버튼 클릭 → 정상 로딩 | ErrorPage 제거, LoadingBar 녹색 |
| 홈 이동 | 홈 버튼 클릭 | homeUrl로 이동, ErrorPage 제거 |

### 10.3 E2E 테스트 (리모컨 시나리오)

| 시나리오 | 리모컨 입력 | 예상 동작 |
|----------|-------------|----------|
| 에러 발생 후 재시도 | 1. 잘못된 URL 입력<br>2. 에러 화면 표시<br>3. 선택 버튼 (재시도) | 페이지 다시 로딩 |
| 에러 발생 후 홈 이동 | 1. 타임아웃 에러<br>2. 우 방향키 (홈 버튼)<br>3. 선택 버튼 | 홈페이지로 이동 |
| 에러 탭 전환 | 1. 에러 탭<br>2. 다른 탭으로 전환<br>3. 다시 에러 탭 복귀 | 에러 화면 유지 (isError=true) |

---

## 11. 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|----------|------|
| 2026-02-13 | 최초 작성 | F-10 기술 설계 |

---

## 다음 단계

이 기술 설계서를 기반으로 product-manager 에이전트가 다음 문서를 작성합니다:
- `docs/specs/error-handling/plan.md` (구현 계획서)
