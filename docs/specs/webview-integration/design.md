# 웹뷰 통합 — 기술 설계서

## 1. 참조
- 요구사항 분석서: `docs/specs/webview-integration/requirements.md`
- PRD: `docs/project/prd.md`
- F-01 설계서: `docs/specs/project-setup/design.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md`

## 2. 아키텍처 개요

### 전체 구조
webOS 플랫폼에서 외부 웹 페이지를 렌더링하기 위해 HTML5 `<iframe>` 태그를 기반으로 한 WebView 컴포넌트를 구현합니다. Enact 공식 WebView 컴포넌트(@enact/webos/WebView)는 webOS TV 6.0 이상에서 지원되므로, webOS 4.x 호환성을 위해 iframe 기반 커스텀 WebView를 구축합니다.

```
┌─────────────────────────────────────────────────────────┐
│                      BrowserView                         │
│  ┌────────────────────────────────────────────────────┐ │
│  │                   URLBar (F-03)                     │ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │                  WebView Component                  │ │
│  │  ┌──────────────────────────────────────────────┐  │ │
│  │  │           <iframe> (웹 콘텐츠)                │  │ │
│  │  │  - src: {url}                                 │  │ │
│  │  │  - sandbox: 보안 정책                         │  │ │
│  │  │  - allowfullscreen, allow: 권한 정책          │  │ │
│  │  └──────────────────────────────────────────────┘  │ │
│  │  [ 로딩 오버레이 / 에러 오버레이 ]                 │ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │             NavigationBar (F-04)                    │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### 핵심 설계 원칙
1. **webOS 4.x 호환성 우선**: iframe 기반으로 구현하여 구형 webOS 지원
2. **이벤트 기반 아키텍처**: iframe의 load, error 이벤트를 활용한 상태 관리
3. **리모컨 포커스 통합**: Enact Spotlight와 iframe 내부 포커스 분리 관리
4. **메모리 효율성**: iframe 인스턴스 재사용, 명시적 리소스 해제
5. **보안 샌드박싱**: iframe의 sandbox 속성으로 악성 스크립트 격리

## 3. 아키텍처 결정

### 결정 1: iframe vs @enact/webos/WebView vs Custom Native WebView
- **선택지**:
  - A) HTML5 `<iframe>` 태그 사용
  - B) Enact 공식 `@enact/webos/WebView` 사용
  - C) webOS Native API (C++ WebAppManager) 사용
- **결정**: A) HTML5 `<iframe>` 태그
- **근거**:
  - **호환성**: iframe은 모든 webOS 버전(4.x~)에서 지원되며 추가 의존성 불필요
  - **Enact 제약**: @enact/webos/WebView는 webOS TV 6.0+ 전용, LG 프로젝터 HU175QW(webOS 4.x)는 미지원
  - **구현 간소화**: iframe은 HTML 표준으로 React에서 직접 제어 가능 (ref 사용)
  - **이벤트 지원**: load, error 이벤트로 기본적인 로딩 상태 추적 가능
  - **보안**: sandbox 속성으로 스크립트 실행 제어, 권한 관리 가능
- **트레이드오프**:
  - **포기**: Enact WebView의 네이티브 이벤트(loadprogress, domready, navigationchange 등)
  - **대응 방안**:
    - 진행률(loadprogress): MessageChannel 또는 postMessage로 iframe 내부와 통신 (선택, F-05에서 구현 시 검토)
    - 네비게이션 이벤트: iframe의 contentWindow.location 주기적 폴링 (제약 있지만 실현 가능)
    - 페이지 제목: document.title 접근 제한 시 기본 제목 사용 (보안 정책에 따라 다름)

### 결정 2: 상태 관리 패턴
- **선택지**:
  - A) useState로 단순 상태 관리 (idle, loading, loaded, error)
  - B) useReducer로 복잡한 상태 전이 관리
  - C) 외부 상태 라이브러리 (Redux, Zustand)
- **결정**: A) useState로 단순 상태 관리
- **근거**:
  - 웹뷰 상태는 4가지로 단순하며 전이 규칙이 명확함
  - useState + useEffect 조합으로 충분히 관리 가능
  - 외부 라이브러리 도입은 번들 크기 증가 및 오버헤드
- **트레이드오프**: 향후 상태가 복잡해지면 useReducer로 리팩토링 필요할 수 있음 (F-06 탭 관리 시점에 재평가)

### 결정 3: iframe 이벤트 리스너 관리
- **선택지**:
  - A) useEffect + addEventListener로 직접 관리
  - B) React 합성 이벤트 (onLoad, onError)
  - C) MutationObserver로 iframe 내부 DOM 감지
- **결정**: B) React 합성 이벤트 우선, A) 보조 사용
- **근거**:
  - React의 onLoad, onError는 선언적이고 자동 정리됨
  - 추가 이벤트(beforeunload 등)는 useEffect로 수동 등록
  - MutationObserver는 CORS 제약으로 사용 불가 (Same-Origin Policy)
- **트레이드오프**: React 이벤트는 제한적이지만, 추가 이벤트는 useEffect로 보완

### 결정 4: 에러 처리 전략
- **선택지**:
  - A) iframe의 onerror 이벤트만 사용
  - B) React Error Boundary 추가
  - C) 타임아웃 기반 에러 감지 (30초 이상 로딩 시 실패 판단)
- **결정**: A) + C) 조합 (B는 F-10에서 구현)
- **근거**:
  - iframe의 onerror는 HTTP 에러(404, 500)는 감지하지만 네트워크 타임아웃은 감지 못함
  - 타임아웃 로직으로 장시간 로딩 감지 → 에러로 간주
  - React Error Boundary는 전역 에러 처리로 F-10(에러 처리 기능)에서 구현
- **타임아웃 설정**: 30초 (PRD의 페이지 로딩 시간 5초 기준, 여유 두어 30초로 설정)

### 결정 5: Enact Spotlight 통합 방식
- **선택지**:
  - A) iframe을 Spotlightable로 설정하여 리모컨 포커스 받음
  - B) iframe 외부 컨테이너만 Spotlightable, 내부 포커스는 웹 페이지 자체 관리
  - C) 커스텀 Spotlight 포커스 이동 로직 구현
- **결정**: B) 외부 컨테이너만 Spotlightable
- **근거**:
  - iframe 내부 웹 페이지는 자체적으로 tabindex, focus 관리를 하므로 Spotlight와 충돌 가능
  - 컨테이너에 Spotlightable을 설정하여 WebView로 포커스 진입 시 iframe 내부는 웹 표준 포커스 관리
  - 리모컨 십자키는 webOS 시스템 레벨에서 키보드 이벤트로 변환되어 iframe 내부로 전달됨
- **트레이드오프**: iframe 내부↔외부 UI 간 포커스 전환이 명확하지 않을 수 있음 → F-04에서 뒤로 버튼 등으로 포커스 이탈 구현

### 결정 6: 메모리 관리 전략
- **선택지**:
  - A) iframe을 DOM에서 완전히 제거 (unmount 시)
  - B) iframe의 src를 빈 값으로 설정 후 재사용
  - C) iframe을 숨기고(display: none) 재사용
- **결정**: A) DOM에서 완전 제거
- **근거**:
  - iframe의 src 변경만으로는 이전 페이지 리소스가 즉시 해제되지 않을 수 있음
  - React의 컴포넌트 Unmount 시 DOM 제거로 명확한 메모리 해제 보장
  - F-06(탭 관리)에서 탭별 WebView 인스턴스 관리 시 재사용 고려 (현 단계는 단일 WebView)
- **트레이드오프**: 탭 전환 시 재로딩 발생 (F-06에서 최적화 검토)

## 4. WebView 컴포넌트 설계

### 컴포넌트 구조
```
src/components/WebView/
├── WebView.js           # 메인 컴포넌트
├── WebView.module.less  # 스타일
└── index.js             # Export 진입점
```

### Props 인터페이스
```javascript
// src/components/WebView/WebView.js
import PropTypes from 'prop-types'

WebView.propTypes = {
	// URL 로드
	url: PropTypes.string.isRequired,               // 로드할 URL

	// 로딩 이벤트 콜백
	onLoadStart: PropTypes.func,                    // 로딩 시작 시 호출
	onLoadProgress: PropTypes.func,                 // 진행률 변경 시 호출 (0~100)
	onLoadEnd: PropTypes.func,                      // 로딩 완료 시 호출

	// 에러 콜백
	onLoadError: PropTypes.func,                    // 로딩 실패 시 호출

	// 네비게이션 콜백
	onNavigationChange: PropTypes.func,             // URL 변경 시 호출

	// 상태 변경 콜백 (선택)
	onStateChange: PropTypes.func,                  // 상태 전이 시 호출 (idle→loading→loaded/error)

	// 스타일 커스터마이징 (선택)
	className: PropTypes.string,                    // CSS 클래스

	// 보안 설정 (선택)
	sandbox: PropTypes.string,                      // iframe sandbox 속성 (기본값 설정됨)
	allow: PropTypes.string                         // iframe allow 속성 (기본값 설정됨)
}

WebView.defaultProps = {
	onLoadStart: () => {},
	onLoadProgress: () => {},
	onLoadEnd: () => {},
	onLoadError: () => {},
	onNavigationChange: () => {},
	onStateChange: () => {},
	className: '',
	// 보안 샌드박스 기본값: 스크립트 허용, 폼 제출 허용, 팝업 차단
	sandbox: 'allow-scripts allow-same-origin allow-forms allow-popups-to-escape-sandbox',
	// 권한 기본값: 자동재생, 전체화면, 암호화 미디어 허용
	allow: 'autoplay; fullscreen; encrypted-media'
}
```

### 상태 정의
```javascript
const [state, setState] = useState('idle')  // 'idle' | 'loading' | 'loaded' | 'error'
const [error, setError] = useState(null)     // { errorCode, errorMessage, url }
const [loadStartTime, setLoadStartTime] = useState(0)  // 로딩 시작 시각 (성능 측정)
const iframeRef = useRef(null)               // iframe DOM 참조
const timeoutRef = useRef(null)              // 타임아웃 타이머 참조
const currentUrlRef = useRef('')             // 현재 URL (네비게이션 감지용)
```

### 상태 전이 다이어그램
```
      mount
        │
        ▼
   ┌────────┐  url 로드   ┌─────────┐  성공   ┌────────┐
   │  idle  │─────────────▶│ loading │────────▶│ loaded │
   └────────┘              └─────────┘         └────────┘
                                │                    │
                                │ 실패 or 타임아웃    │ 새 URL 로드
                                ▼                    │
                           ┌───────┐                 │
                           │ error │◀────────────────┘
                           └───────┘
                                │
                                │ 재시도
                                ▼
                           ┌─────────┐
                           │ loading │
                           └─────────┘
```

## 5. 시퀀스 흐름 설계

### 주요 시나리오: 정상 페이지 로딩
```
사용자       WebView Component      iframe         onLoad Callback    BrowserView
  │                │                   │                   │               │
  │  props.url     │                   │                   │               │
  │───────────────▶│                   │                   │               │
  │                │  useEffect 감지   │                   │               │
  │                │  (url 변경)       │                   │               │
  │                │                   │                   │               │
  │                │  setState('loading')                  │               │
  │                │  setLoadStartTime(now)                │               │
  │                │────────────────────────────────────onLoadStart()──────▶│
  │                │                   │                   │               │
  │                │  iframe.src = url │                   │               │
  │                │──────────────────▶│                   │               │
  │                │                   │  HTTP GET         │               │
  │                │                   │─────────────────▶ │               │
  │                │                   │  HTML 응답        │               │
  │                │                   │◀─────────────────│               │
  │                │                   │  렌더링 중...     │               │
  │                │                   │                   │               │
  │                │  onLoad 이벤트    │                   │               │
  │                │◀──────────────────│                   │               │
  │                │  setState('loaded')                   │               │
  │                │  duration = now - loadStartTime       │               │
  │                │  extractTitle()                       │               │
  │                │────────────────────────────────────onLoadEnd({        │
  │                │                   │                   │  url, title,  │
  │                │                   │                   │  duration })  │
  │                │                   │                   │──────────────▶│
  │                │  clearTimeout()   │                   │               │
```

### 에러 시나리오 1: 네트워크 에러 (404)
```
WebView Component      iframe         onError Callback    BrowserView
  │                       │                   │               │
  │  iframe.src = url     │                   │               │
  │──────────────────────▶│                   │               │
  │                       │  HTTP GET         │               │
  │                       │─────────────────▶ │               │
  │                       │  404 Not Found    │               │
  │                       │◀─────────────────│               │
  │  onError 이벤트       │                   │               │
  │◀──────────────────────│                   │               │
  │  setState('error')    │                   │               │
  │  setError({          │                   │               │
  │    errorCode: 404,   │                   │               │
  │    errorMessage: '페이지를 찾을 수 없습니다', │           │
  │    url               │                   │               │
  │  })                  │                   │               │
  │────────────────────────────────────────onLoadError({     │
  │                       │                   │  errorCode,   │
  │                       │                   │  errorMessage,│
  │                       │                   │  url })       │
  │                       │                   │──────────────▶│
  │  clearTimeout()       │                   │               │
```

### 에러 시나리오 2: 타임아웃 (30초 초과)
```
WebView Component      setTimeout          BrowserView
  │                       │                    │
  │  iframe.src = url     │                    │
  │  setState('loading')  │                    │
  │  setTimeout(30s)      │                    │
  │──────────────────────▶│                    │
  │                       │                    │
  │  ... 30초 경과 ...    │                    │
  │                       │                    │
  │  timeout 콜백 실행    │                    │
  │◀──────────────────────│                    │
  │  if (state === 'loading') {                │
  │    setState('error')  │                    │
  │    setError({         │                    │
  │      errorCode: -1,   │                    │
  │      errorMessage: '페이지 로딩 시간 초과', │
  │      url              │                    │
  │    })                 │                    │
  │────────────────────────────────────onLoadError(...)────▶│
  │  }                    │                    │
```

### 네비게이션 이벤트 감지 (폴링 방식)
```
WebView Component      setInterval        BrowserView
  │                       │                    │
  │  useEffect (mount)    │                    │
  │  setInterval(500ms)   │                    │
  │──────────────────────▶│                    │
  │                       │                    │
  │  ... 500ms 후 ...     │                    │
  │                       │                    │
  │  폴링 콜백 실행       │                    │
  │◀──────────────────────│                    │
  │  try {                │                    │
  │    newUrl = iframe.contentWindow.location.href │
  │  } catch (CORS) {     │                    │
  │    // Same-Origin Policy 위반 시 무시     │
  │    return             │                    │
  │  }                    │                    │
  │  if (newUrl !== currentUrlRef.current) {  │
  │    currentUrlRef.current = newUrl         │
  │────────────────────────────────────onNavigationChange({ url: newUrl })──▶│
  │  }                    │                    │
```

**주의**: iframe의 contentWindow.location은 Same-Origin Policy로 인해 다른 도메인 페이지에서는 접근 불가합니다. 이 경우 네비게이션 감지가 제한되며, F-04(페이지 탐색)에서 뒤로/앞으로 버튼을 iframe의 history API로 직접 제어하는 방식으로 보완합니다.

## 6. Enact Spotlight 통합 설계

### Spotlightable 컨테이너 설정
```javascript
import Spotlight from '@enact/spotlight'
import Spottable from '@enact/spotlight/Spottable'

// WebView 컨테이너를 Spottable로 감싸기
const SpottableDiv = Spottable('div')

const WebView = ({ url, onLoadStart, ... }) => {
	return (
		<SpottableDiv
			className={css.webviewContainer}
			spotlightId="webview-main"
			onSpotlightFocus={handleFocus}
			onSpotlightBlur={handleBlur}
		>
			<iframe
				ref={iframeRef}
				src={url}
				className={css.iframe}
				onLoad={handleLoad}
				onError={handleError}
				sandbox={sandbox}
				allow={allow}
				title="Web Content"
			/>
			{state === 'loading' && <LoadingOverlay />}
			{state === 'error' && <ErrorOverlay error={error} />}
		</SpottableDiv>
	)
}
```

### 리모컨 포커스 동작 흐름
1. **포커스 진입**: 사용자가 URLBar에서 아래 방향키 → WebView 컨테이너로 Spotlight 포커스 이동
2. **포커스 활성**: WebView 컨테이너가 포커스를 받으면 `onSpotlightFocus` 호출
3. **iframe 내부 포커스**: webOS 시스템이 리모컨 십자키를 키보드 이벤트(Arrow Up/Down/Left/Right)로 변환하여 iframe 내부로 전달 → 웹 페이지 자체 포커스 관리
4. **포커스 이탈**: 사용자가 뒤로 버튼 클릭 → Spotlight가 WebView 외부 UI(NavigationBar)로 이동

### 리모컨 키 매핑
| 리모컨 키 | webOS 이벤트 | iframe 내부 전달 | 동작 |
|-----------|-------------|------------------|------|
| 십자키 (↑↓←→) | KeyDown (Arrow Up/Down/Left/Right) | ✅ | 웹 페이지 내 포커스 이동, 스크롤 |
| 선택 (OK) | KeyDown (Enter) | ✅ | 링크 클릭, 버튼 활성화 |
| 뒤로 (Back) | KeyDown (Backspace) | ❌ (상위 캡처) | Spotlight 포커스 이탈 → NavigationBar |
| 홈 (Home) | 시스템 이벤트 | ❌ | 앱 홈 화면 이동 (F-15) |

**주의**: 뒤로 버튼은 WebView 컴포넌트의 상위(BrowserView)에서 `onKeyDown` 이벤트를 캡처하여 Spotlight 포커스를 NavigationBar로 이동시켜야 합니다 (F-04에서 구현).

## 7. 에러 처리 설계

### 에러 타입 분류
```javascript
// src/components/WebView/errorTypes.js
export const ErrorTypes = {
	NETWORK_ERROR: {
		code: -1,
		getMessage: () => '네트워크 연결을 확인해주세요'
	},
	TIMEOUT_ERROR: {
		code: -2,
		getMessage: () => '페이지 로딩 시간이 초과되었습니다 (30초)'
	},
	HTTP_404: {
		code: 404,
		getMessage: () => '페이지를 찾을 수 없습니다'
	},
	HTTP_500: {
		code: 500,
		getMessage: () => '서버 오류가 발생했습니다'
	},
	CORS_ERROR: {
		code: -3,
		getMessage: () => '보안 정책으로 페이지를 로드할 수 없습니다'
	},
	UNKNOWN_ERROR: {
		code: -999,
		getMessage: () => '알 수 없는 오류가 발생했습니다'
	}
}
```

### 에러 감지 로직
```javascript
// iframe onError 이벤트 핸들러
const handleError = (event) => {
	clearTimeout(timeoutRef.current)

	// iframe의 onError는 HTTP 상태 코드를 제공하지 않으므로 기본 에러로 처리
	const errorInfo = {
		errorCode: ErrorTypes.NETWORK_ERROR.code,
		errorMessage: ErrorTypes.NETWORK_ERROR.getMessage(),
		url: currentUrlRef.current
	}

	setState('error')
	setError(errorInfo)
	onLoadError(errorInfo)
}

// 타임아웃 에러 핸들러
const handleTimeout = () => {
	if (state !== 'loading') return  // 이미 로딩 완료/실패한 경우 무시

	const errorInfo = {
		errorCode: ErrorTypes.TIMEOUT_ERROR.code,
		errorMessage: ErrorTypes.TIMEOUT_ERROR.getMessage(),
		url: currentUrlRef.current
	}

	setState('error')
	setError(errorInfo)
	onLoadError(errorInfo)
}
```

### 에러 오버레이 UI (간단한 표시)
```javascript
// src/components/WebView/ErrorOverlay.js
const ErrorOverlay = ({ error, onRetry }) => (
	<div className={css.errorOverlay}>
		<div className={css.errorIcon}>⚠️</div>
		<h2 className={css.errorTitle}>페이지 로딩 실패</h2>
		<p className={css.errorMessage}>{error.errorMessage}</p>
		<p className={css.errorUrl}>URL: {error.url}</p>
		<button className={css.retryButton} onClick={onRetry}>
			다시 시도
		</button>
	</div>
)
```

**주의**: F-10(에러 처리 기능)에서 전역 에러 페이지를 구현하므로, 현 단계는 간단한 오버레이만 제공합니다.

## 8. 메모리 관리 전략

### 리소스 해제 로직
```javascript
useEffect(() => {
	// 컴포넌트 Mount 시 초기화
	setState('idle')
	setError(null)

	// 컴포넌트 Unmount 시 정리
	return () => {
		// 타임아웃 타이머 정리
		if (timeoutRef.current) {
			clearTimeout(timeoutRef.current)
		}

		// iframe src를 빈 값으로 설정 (리소스 해제 유도)
		if (iframeRef.current) {
			iframeRef.current.src = 'about:blank'
		}

		// 네비게이션 폴링 정리 (setInterval 사용 시)
		if (navigationIntervalRef.current) {
			clearInterval(navigationIntervalRef.current)
		}
	}
}, [])  // 빈 배열: Mount/Unmount 시에만 실행
```

### URL 변경 시 리소스 관리
```javascript
useEffect(() => {
	if (!url) return

	// 이전 타임아웃 정리
	if (timeoutRef.current) {
		clearTimeout(timeoutRef.current)
	}

	// 로딩 시작
	setState('loading')
	setLoadStartTime(Date.now())
	currentUrlRef.current = url
	onLoadStart()

	// 30초 타임아웃 설정
	timeoutRef.current = setTimeout(handleTimeout, 30000)

	// iframe src 변경 (자동으로 이전 페이지 리소스 해제 시도)
	if (iframeRef.current) {
		iframeRef.current.src = url
	}
}, [url])  // url이 변경될 때마다 실행
```

### 메모리 모니터링 (개발 모드)
```javascript
// src/utils/logger.js
export const logMemoryUsage = (context) => {
	if (process.env.NODE_ENV !== 'development') return

	if (performance && performance.memory) {
		const { usedJSHeapSize, totalJSHeapSize } = performance.memory
		console.log(`[Memory] ${context}:`, {
			used: `${(usedJSHeapSize / 1024 / 1024).toFixed(2)} MB`,
			total: `${(totalJSHeapSize / 1024 / 1024).toFixed(2)} MB`
		})
	}
}

// WebView.js에서 사용
useEffect(() => {
	logMemoryUsage('WebView Mount')
	return () => logMemoryUsage('WebView Unmount')
}, [])
```

## 9. BrowserView 통합 설계

### BrowserView 레이아웃
```javascript
// src/views/BrowserView.js
import React, { useState } from 'react'
import { Panel, Header } from '@enact/moonstone/Panel'
import WebView from '../components/WebView'
import css from './BrowserView.module.less'

const BrowserView = () => {
	const [currentUrl, setCurrentUrl] = useState('https://www.google.com')

	// WebView 이벤트 핸들러
	const handleLoadStart = () => {
		console.log('[BrowserView] 페이지 로딩 시작:', currentUrl)
	}

	const handleLoadEnd = ({ url, title, duration }) => {
		console.log('[BrowserView] 페이지 로딩 완료:', { url, title, duration })
	}

	const handleLoadError = ({ errorCode, errorMessage, url }) => {
		console.error('[BrowserView] 페이지 로딩 실패:', { errorCode, errorMessage, url })
	}

	const handleNavigationChange = ({ url }) => {
		console.log('[BrowserView] URL 변경:', url)
		// 향후 F-03(URLBar), F-08(히스토리)와 연동
	}

	return (
		<Panel className={css.browserView}>
			<Header title="webOS Browser" />

			{/* URLBar 영역 (F-03에서 구현 예정) */}
			<div className={css.urlBarPlaceholder}>
				URL: {currentUrl}
			</div>

			{/* WebView 메인 영역 */}
			<div className={css.webviewWrapper}>
				<WebView
					url={currentUrl}
					onLoadStart={handleLoadStart}
					onLoadEnd={handleLoadEnd}
					onLoadError={handleLoadError}
					onNavigationChange={handleNavigationChange}
				/>
			</div>

			{/* NavigationBar 영역 (F-04에서 구현 예정) */}
			<div className={css.navBarPlaceholder}>
				[ 뒤로 | 앞으로 | 새로고침 | 홈 ]
			</div>
		</Panel>
	)
}

export default BrowserView
```

### BrowserView 레이아웃 스타일
```less
// src/views/BrowserView.module.less
.browserView {
	display: flex;
	flex-direction: column;
	height: 100vh;
	width: 100vw;
	background-color: var(--bg-color);
}

.urlBarPlaceholder {
	height: 80px;
	padding: var(--spacing-md);
	background-color: #2a2a2a;
	color: var(--text-color);
	font-size: var(--font-size-min);
	display: flex;
	align-items: center;
}

.webviewWrapper {
	flex: 1;  // 남은 공간 전체 차지
	overflow: hidden;  // iframe 오버플로 방지
}

.navBarPlaceholder {
	height: 100px;
	padding: var(--spacing-md);
	background-color: #2a2a2a;
	color: var(--text-color);
	font-size: var(--font-size-min);
	display: flex;
	align-items: center;
	justify-content: center;
}
```

## 10. 구현 순서

### Phase 1: WebView 컴포넌트 기본 구조
1. `src/components/WebView/` 디렉토리 생성
2. `WebView.js` 생성 (Props 인터페이스, PropTypes 정의)
3. `WebView.module.less` 생성 (기본 스타일)
4. `index.js` 생성 (export default)

### Phase 2: iframe 기본 렌더링
1. iframe 요소 추가 (ref, src, sandbox, allow 속성 설정)
2. useState로 상태 관리 (idle, loading, loaded, error)
3. useEffect로 URL 변경 감지
4. 기본 스타일링 (전체 화면 채우기)

### Phase 3: 로딩 이벤트 처리
1. onLoad 이벤트 핸들러 구현
2. onLoadStart 콜백 호출
3. onLoadEnd 콜백 호출 (로딩 시간 계산)
4. 상태 전이 로직 구현 (idle → loading → loaded)

### Phase 4: 에러 처리
1. onError 이벤트 핸들러 구현
2. 타임아웃 로직 구현 (30초)
3. onLoadError 콜백 호출
4. ErrorOverlay 컴포넌트 추가 (간단한 에러 표시)

### Phase 5: Spotlight 통합
1. Spottable HOC로 컨테이너 감싸기
2. onSpotlightFocus, onSpotlightBlur 핸들러 추가
3. 리모컨 포커스 테스트 (탭 키로 로컬 테스트)

### Phase 6: 네비게이션 감지 (선택)
1. setInterval로 iframe.contentWindow.location 폴링
2. onNavigationChange 콜백 호출
3. CORS 에러 핸들링 (try-catch)

### Phase 7: 메모리 관리
1. useEffect cleanup 함수 구현
2. 타임아웃 타이머 정리
3. iframe src를 about:blank로 설정 (리소스 해제)

### Phase 8: BrowserView 통합
1. `src/views/BrowserView.js` 수정
2. WebView 컴포넌트 import 및 사용
3. 초기 URL 설정 (https://www.google.com)
4. 이벤트 핸들러 구현 (콘솔 로그)
5. 레이아웃 스타일링 (URLBar, WebView, NavigationBar 영역 분할)

### Phase 9: 로컬 테스트
1. `npm run serve` 실행
2. 브라우저에서 Google 홈페이지 렌더링 확인
3. 콘솔 로그에서 이벤트 출력 확인 (loadStart, loadEnd)
4. 네트워크 끊고 에러 처리 확인
5. 메모리 사용량 로깅 확인

### Phase 10: 실제 디바이스 테스트
1. `npm run pack-p` 빌드
2. `ares-package dist/` IPK 생성
3. 프로젝터 설치 및 실행
4. 리모컨으로 WebView 포커스 및 조작 확인
5. 주요 사이트 렌더링 테스트 (YouTube, Naver)

## 11. 기술적 주의사항

### iframe Same-Origin Policy (CORS)
- **문제**: iframe 내부 페이지가 다른 도메인일 경우 `contentWindow.location`, `contentDocument` 접근 불가
- **영향**: 네비게이션 감지, 페이지 제목 추출이 제한됨
- **대응**:
  - 네비게이션 감지는 제한적으로만 작동 (Same-Origin일 때만)
  - 페이지 제목은 기본값 사용 또는 사용자 입력 URL을 제목으로 대체
  - F-04(페이지 탐색)에서 뒤로/앞으로는 iframe의 history API 사용 (postMessage 활용 검토)

### webOS Chromium 엔진 버전
- **버전**: webOS 4.x는 Chromium 53 기반 (구형)
- **영향**: 최신 웹 표준(ES2020+, CSS Grid 등) 일부 미지원
- **대응**:
  - Babel 트랜스파일로 ES5 변환 (Enact CLI 기본 설정)
  - 주요 사이트 호환성 테스트 필수 (YouTube, Netflix는 폴백 지원)

### 리모컨 입력 처리
- **문제**: 웹 페이지가 마우스 전제로 설계된 경우 리모컨 조작 어려움
- **대응**:
  - 웹 페이지 자체 키보드 네비게이션에 의존 (tabindex, accesskey)
  - 조작 어려운 사이트는 사용자에게 안내 (F-10 에러 페이지에서 안내문 추가 고려)

### iframe의 sandbox 보안 정책
- **설정**: `allow-scripts allow-same-origin allow-forms allow-popups-to-escape-sandbox`
- **의미**:
  - `allow-scripts`: JavaScript 실행 허용 (기본 웹 기능 위해 필수)
  - `allow-same-origin`: 쿠키, localStorage 접근 허용
  - `allow-forms`: 폼 제출 허용 (검색 등)
  - `allow-popups-to-escape-sandbox`: 팝업이 샌드박스 밖에서 열림 (광고, 로그인 팝업 대응)
- **리스크**: 악성 스크립트 실행 가능 → 사용자에게 신뢰할 수 있는 사이트만 방문 권장 (F-14 보안 표시에서 경고)

### 메모리 누수 방지
- **중요**: iframe은 페이지 변경 시에도 이전 리소스를 일부 유지할 수 있음
- **대응**:
  - unmount 시 `iframe.src = 'about:blank'`로 명시적 해제
  - F-06(탭 관리)에서 탭당 메모리 사용량 모니터링 (200MB 제한)

### 성능 최적화
- **로딩 시간**: iframe은 HTML 파싱 후 리소스(이미지, 스크립트) 로드 완료 시 onLoad 발생
- **최적화 방안**:
  - iframe의 `loading="lazy"` 속성 사용 금지 (즉시 로드 필요)
  - F-05(로딩 인디케이터)에서 진행률 표시로 사용자 대기 경험 개선

## 12. 확장성 고려사항

### F-06(탭 관리)와의 연동 준비
- **현재**: 단일 WebView 인스턴스
- **F-06 시**: 탭별 WebView 인스턴스 배열 관리
- **준비**:
  - WebView 컴포넌트는 독립적으로 동작하도록 설계 (Props 기반)
  - 상태 관리를 상위 컴포넌트(BrowserView)로 리프팅 (탭별 상태 저장)
  - 메모리 사용량 제한 로직 추가 (탭당 200MB)

### F-05(로딩 인디케이터)와의 연동
- **현재**: 간단한 로딩 오버레이
- **F-05 시**: 프로그레스바 UI 개선
- **준비**:
  - onLoadProgress 콜백은 현재 구현 불가 (iframe 제약)
  - F-05에서 가상 진행률(0→100% 애니메이션) 또는 스피너로 대체

### F-04(페이지 탐색)와의 연동
- **현재**: 네비게이션 이벤트만 감지
- **F-04 시**: 뒤로/앞으로 버튼으로 iframe history 제어
- **준비**:
  - iframe ref를 상위 컴포넌트에 노출 (useImperativeHandle)
  - history.back(), history.forward() 호출 메서드 제공

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-12 | 최초 작성 | F-02 요구사항 기반 기술 설계 |
