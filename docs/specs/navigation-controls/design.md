# 페이지 탐색 컨트롤 — 기술 설계서

## 1. 참조
- 요구사항 분석서: `docs/specs/navigation-controls/requirements.md`
- 웹뷰 설계서: `docs/specs/webview-integration/design.md`
- PRD: `docs/project/prd.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md`

## 2. 아키텍처 개요

### 전체 구조
NavigationBar는 웹 브라우저의 핵심 탐색 기능(뒤로 가기, 앞으로 가기, 새로고침, 홈)을 리모컨 최적화 버튼 UI로 제공합니다. WebView의 iframe history API를 제어하여 페이지 네비게이션을 담당하며, Enact Spotlight와 통합되어 리모컨 포커스 관리를 지원합니다.

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
│  │  │  - history.back() ◀── NavigationBar 제어     │  │ │
│  │  │  - history.forward() ◀── NavigationBar 제어  │  │ │
│  │  │  - location.reload() ◀── NavigationBar 제어  │  │ │
│  │  └──────────────────────────────────────────────┘  │ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │             NavigationBar (F-04)                    │ │
│  │  [ ← 뒤로 ] [ 앞으로 → ] [ 🔄 새로고침 ] [ 🏠 홈 ] │ │
│  │      ▲           ▲            ▲            ▲       │ │
│  │      └───────────┴────────────┴────────────┘       │ │
│  │         Enact Spotlight 포커스 관리               │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### 핵심 설계 원칙
1. **WebView와의 명확한 인터페이스**: NavigationBar는 iframe ref를 통해 직접 history API 제어
2. **상태 동기화**: WebView의 onNavigationChange 이벤트를 구독하여 버튼 활성/비활성 상태 실시간 업데이트
3. **리모컨 포커스 최적화**: Enact Spotlight로 버튼 간 수평 이동, WebView와의 수직 이동 관리
4. **에러 허용 설계**: iframe history API 호출 실패 시 조용히 실패 (UI 변화 없음)
5. **CORS 대응**: Same-Origin Policy로 인한 제약을 Props 기반 상태 관리로 우회

## 3. 아키텍처 결정

### 결정 1: iframe history API 제어 방식
- **선택지**:
  - A) NavigationBar가 WebView의 iframe ref를 직접 참조하여 history API 호출
  - B) NavigationBar가 BrowserView에 이벤트를 발생시키고, BrowserView가 WebView에 전달
  - C) postMessage로 iframe 내부에 메시지 전달하여 history 제어
- **결정**: A) NavigationBar가 iframe ref 직접 참조
- **근거**:
  - iframe의 history API(back, forward, go)는 외부에서 직접 호출 가능 (Same-Origin Policy 영향 없음)
  - 이벤트 중개자(BrowserView)를 거치는 B안은 불필요한 레이어 추가
  - postMessage(C안)는 iframe 내부 스크립트 필요 → 외부 사이트 제어 불가
  - React의 useImperativeHandle로 WebView에서 iframe ref를 안전하게 노출 가능
- **트레이드오프**:
  - **장점**: 직접 제어로 응답 속도 빠름 (0.1초 이내), 코드 간결
  - **단점**: NavigationBar가 WebView 내부 구현에 의존 (결합도 증가)
  - **대응**: ref 인터페이스를 명확히 정의하여 의존성 최소화

### 결정 2: 버튼 활성/비활성 상태 관리
- **선택지**:
  - A) NavigationBar가 iframe.contentWindow.history.length를 직접 읽어서 판단
  - B) WebView가 onNavigationChange 이벤트에서 히스토리 상태를 함께 전달
  - C) BrowserView에서 히스토리 스택을 별도 상태로 관리
- **결정**: B) WebView가 히스토리 상태 전달 (CORS 제약으로 불가능한 경우 기본 활성화)
- **근거**:
  - iframe의 history 객체는 Same-Origin Policy로 다른 도메인 페이지에서 접근 불가
  - A안은 CORS 에러 발생 시 버튼 상태 업데이트 실패
  - C안은 브라우저 자체 히스토리 스택과 불일치 가능성 (사용자가 iframe 내부에서 링크 클릭 시)
  - B안은 WebView가 주기적으로 감지한 히스토리 상태를 전달하여 최신 상태 유지
- **트레이드오프**:
  - **장점**: CORS 제약 우회, 히스토리 상태 정확도 향상
  - **단점**: CORS 환경에서는 히스토리 상태 감지 불가 → 버튼 항상 활성화
  - **대응**: CORS 환경에서는 버튼 클릭 시 동작 실패 시에도 UI 에러 표시 없이 무시

### 결정 3: 리모컨 Back 키 처리 레벨
- **선택지**:
  - A) NavigationBar 컴포넌트에서 Back 키 캡처
  - B) BrowserView에서 Back 키 캡처 (전역 처리)
  - C) WebView에서 Back 키 캡처
- **결정**: B) BrowserView에서 전역 처리
- **근거**:
  - Back 키 동작은 현재 포커스 위치에 따라 다름 (WebView 포커스 시: 브라우저 뒤로, NavigationBar 포커스 시: 포커스 이탈)
  - BrowserView가 전체 레이아웃을 관리하므로 포커스 상태 판단 용이
  - NavigationBar 단독 처리(A안)는 WebView 포커스 상태 확인 어려움
  - WebView 처리(C안)는 NavigationBar 포커스 상태 확인 어려움
- **트레이드오프**:
  - **장점**: 포커스 흐름 중앙 관리로 일관성 유지
  - **단점**: BrowserView가 리모컨 입력 로직 포함 → 복잡도 증가
  - **대응**: 별도 커스텀 Hook(useRemoteControl)으로 분리하여 재사용성 확보

### 결정 4: NavigationBar UI 프레임워크
- **선택지**:
  - A) Enact Moonstone Button 컴포넌트 사용
  - B) 커스텀 버튼 컴포넌트 (Spottable HOC 사용)
  - C) HTML button + CSS 스타일링
- **결정**: A) Enact Moonstone Button 사용
- **근거**:
  - Moonstone Button은 리모컨 포커스, 테마, 애니메이션이 기본 제공됨
  - Spotlight 통합이 자동으로 되어 있어 포커스 관리 간편
  - webOS 플랫폼 UI 가이드라인 준수 (일관된 UX)
  - 커스텀 구현(B, C)은 접근성, 테마, 포커스 로직 직접 구현 필요 (개발 시간 증가)
- **트레이드오프**:
  - **장점**: 개발 속도 빠름, 안정성 높음, 접근성 자동 지원
  - **단점**: Moonstone 스타일에 제약 (커스터마이징 제한적)
  - **대응**: CSS Modules로 추가 스타일링, 필요 시 minWidth, icon props 활용

### 결정 5: 홈페이지 URL 관리
- **선택지**:
  - A) 하드코딩 (https://www.google.com)
  - B) BrowserView 상태로 관리
  - C) LS2 API로 영구 저장 (F-11 설정 화면과 연동)
- **결정**: A) 현재는 하드코딩, F-11 구현 시 C로 확장
- **근거**:
  - F-11(설정 화면)이 구현되기 전까지는 홈페이지 변경 UI 없음
  - 하드코딩으로 최소 기능 구현 후 F-11에서 LS2 API로 확장
  - B안(상태 관리)은 앱 재시작 시 초기화되어 의미 없음
- **트레이드오프**:
  - **장점**: 구현 간단, 초기 기능 제공 가능
  - **단점**: 사용자 커스터마이징 불가
  - **대응**: F-11 구현 시 설정 서비스로 확장 (설계 시점에 확장성 고려)

### 결정 6: 버튼 간격 및 레이아웃 방식
- **선택지**:
  - A) Flexbox 수평 배치 (justify-content: space-between)
  - B) Flexbox 수평 배치 (justify-content: center + gap)
  - C) CSS Grid (4열 고정)
- **결정**: B) Flexbox 중앙 정렬 + gap
- **근거**:
  - 버튼 수가 4개로 고정되어 있으며, 중앙 정렬이 시각적으로 균형감 제공
  - gap 속성으로 버튼 간격 20px 일관되게 유지 (요구사항 NFR-2 준수)
  - space-between(A안)은 화면 너비에 따라 간격 변동 (일관성 저하)
  - Grid(C안)는 4개 버튼에는 과도한 레이아웃 (Flexbox로 충분)
- **트레이드오프**:
  - **장점**: 간결한 CSS, 일관된 간격, 중앙 정렬
  - **단점**: 향후 버튼 추가 시 레이아웃 재조정 필요
  - **대응**: 버튼 추가는 F-12(단축키) 이후 고려, 현재는 4개 고정

## 4. NavigationBar 컴포넌트 설계

### 컴포넌트 구조
```
src/components/NavigationBar/
├── NavigationBar.js           # 메인 컴포넌트
├── NavigationBar.module.less  # 스타일
└── index.js                   # Export 진입점
```

### Props 인터페이스
```javascript
// src/components/NavigationBar/NavigationBar.js
import PropTypes from 'prop-types'

NavigationBar.propTypes = {
	// WebView의 iframe ref (history API 제어용)
	webviewRef: PropTypes.shape({
		current: PropTypes.instanceOf(Element)
	}).isRequired,

	// 버튼 활성/비활성 상태 (WebView에서 전달)
	canGoBack: PropTypes.bool,       // 뒤로 가기 가능 여부
	canGoForward: PropTypes.bool,    // 앞으로 가기 가능 여부

	// 홈페이지 URL (홈 버튼 클릭 시 이동할 URL)
	homeUrl: PropTypes.string,

	// 네비게이션 이벤트 콜백
	onNavigate: PropTypes.func,      // 네비게이션 동작 시 호출 ({ action: 'back'|'forward'|'reload'|'home', url })

	// 스타일 커스터마이징 (선택)
	className: PropTypes.string
}

NavigationBar.defaultProps = {
	canGoBack: false,
	canGoForward: false,
	homeUrl: 'https://www.google.com',
	onNavigate: () => {},
	className: ''
}
```

### 상태 정의
```javascript
// NavigationBar는 버튼 상태를 Props로 받으므로 내부 상태 최소화
const [isNavigating, setIsNavigating] = useState(false)  // 네비게이션 진행 중 플래그 (중복 클릭 방지)
```

## 5. WebView history API 제어 설계

### iframe ref 노출 (WebView 컴포넌트 수정)
```javascript
// src/components/WebView/WebView.js
import { forwardRef, useImperativeHandle } from 'react'

const WebView = forwardRef(({ url, onLoadStart, ... }, ref) => {
	const iframeRef = useRef(null)

	// ref를 통해 iframe 요소 노출
	useImperativeHandle(ref, () => ({
		// iframe 요소 직접 접근
		get iframe() {
			return iframeRef.current
		},

		// 편의 메서드 제공 (선택)
		goBack: () => {
			if (iframeRef.current && iframeRef.current.contentWindow) {
				try {
					iframeRef.current.contentWindow.history.back()
					return true
				} catch (error) {
					logger.error('[WebView] history.back() 실패:', error)
					return false
				}
			}
			return false
		},

		goForward: () => {
			if (iframeRef.current && iframeRef.current.contentWindow) {
				try {
					iframeRef.current.contentWindow.history.forward()
					return true
				} catch (error) {
					logger.error('[WebView] history.forward() 실패:', error)
					return false
				}
			}
			return false
		},

		reload: () => {
			if (iframeRef.current && iframeRef.current.contentWindow) {
				try {
					iframeRef.current.contentWindow.location.reload()
					return true
				} catch (error) {
					logger.error('[WebView] location.reload() 실패:', error)
					return false
				}
			}
			return false
		}
	}), [])

	// ... 기존 코드 ...
})
```

### NavigationBar에서 history API 호출
```javascript
// src/components/NavigationBar/NavigationBar.js

// 뒤로 가기 버튼 핸들러
const handleBack = useCallback(() => {
	if (!canGoBack || isNavigating) return

	setIsNavigating(true)

	try {
		// WebView ref를 통해 iframe history.back() 호출
		if (webviewRef.current && webviewRef.current.contentWindow) {
			webviewRef.current.contentWindow.history.back()

			// 네비게이션 콜백 호출
			if (onNavigate) {
				onNavigate({ action: 'back' })
			}

			logger.info('[NavigationBar] 뒤로 가기 실행')
		}
	} catch (error) {
		logger.error('[NavigationBar] 뒤로 가기 실패:', error)
	} finally {
		// 0.5초 후 플래그 해제 (중복 클릭 방지)
		setTimeout(() => setIsNavigating(false), 500)
	}
}, [canGoBack, isNavigating, webviewRef, onNavigate])

// 앞으로 가기 버튼 핸들러 (유사 로직)
const handleForward = useCallback(() => { /* ... */ }, [canGoForward, isNavigating, webviewRef, onNavigate])

// 새로고침 버튼 핸들러
const handleReload = useCallback(() => {
	if (isNavigating) return

	setIsNavigating(true)

	try {
		if (webviewRef.current && webviewRef.current.contentWindow) {
			webviewRef.current.contentWindow.location.reload()

			if (onNavigate) {
				onNavigate({ action: 'reload' })
			}

			logger.info('[NavigationBar] 새로고침 실행')
		}
	} catch (error) {
		logger.error('[NavigationBar] 새로고침 실패:', error)
	} finally {
		setTimeout(() => setIsNavigating(false), 500)
	}
}, [isNavigating, webviewRef, onNavigate])

// 홈 버튼 핸들러
const handleHome = useCallback(() => {
	if (isNavigating) return

	setIsNavigating(true)

	// BrowserView에 URL 변경 요청 (직접 iframe src 변경하지 않음)
	if (onNavigate) {
		onNavigate({ action: 'home', url: homeUrl })
	}

	logger.info('[NavigationBar] 홈으로 이동:', homeUrl)

	setTimeout(() => setIsNavigating(false), 500)
}, [isNavigating, homeUrl, onNavigate])
```

## 6. 히스토리 상태 동기화 설계

### WebView에서 히스토리 상태 감지
```javascript
// src/components/WebView/WebView.js

/**
 * 네비게이션 감지 및 히스토리 상태 추적
 * CORS 제약으로 Same-Origin이 아닌 경우 감지 불가
 */
useEffect(() => {
	navigationIntervalRef.current = setInterval(() => {
		try {
			if (iframeRef.current && iframeRef.current.contentWindow) {
				const history = iframeRef.current.contentWindow.history
				const newUrl = iframeRef.current.contentWindow.location.href

				// URL 변경 감지
				if (newUrl !== currentUrlRef.current) {
					currentUrlRef.current = newUrl

					// 히스토리 상태 전달 (canGoBack, canGoForward는 추정)
					// history.length는 Same-Origin에서만 접근 가능
					const historyState = {
						url: newUrl,
						canGoBack: history.length > 1,  // 추정 (정확하지 않음)
						canGoForward: false             // iframe에서는 직접 감지 불가
					}

					// onNavigationChange 콜백 호출
					if (onNavigationChange) {
						onNavigationChange(historyState)
					}

					logger.info('[WebView] URL 변경 감지:', historyState)
				}
			}
		} catch (error) {
			// CORS 에러 시 무시 (Same-Origin Policy)
			// 이 경우 NavigationBar 버튼은 항상 활성화 상태 유지
		}
	}, 500)

	return () => {
		if (navigationIntervalRef.current) {
			clearInterval(navigationIntervalRef.current)
		}
	}
}, [onNavigationChange])
```

### BrowserView에서 히스토리 상태 관리
```javascript
// src/views/BrowserView.js

const [canGoBack, setCanGoBack] = useState(false)
const [canGoForward, setCanGoForward] = useState(false)

const handleNavigationChange = ({ url, canGoBack, canGoForward }) => {
	logger.info('[BrowserView] URL 변경:', url)

	// 히스토리 상태 업데이트
	if (canGoBack !== undefined) {
		setCanGoBack(canGoBack)
	}
	if (canGoForward !== undefined) {
		setCanGoForward(canGoForward)
	}

	// URLBar 업데이트 (F-03 연동)
	// setCurrentUrl(url)
}

// NavigationBar에 Props 전달
<NavigationBar
	webviewRef={webviewRef}
	canGoBack={canGoBack}
	canGoForward={canGoForward}
	homeUrl={homeUrl}
	onNavigate={handleNavigate}
/>
```

**주의**: iframe의 history API는 length 속성만 제공하며, 현재 인덱스는 알 수 없습니다. 따라서 canGoForward는 정확히 감지할 수 없으며, 사용자가 뒤로 간 후에만 활성화하는 로직을 BrowserView에서 별도 관리해야 합니다.

### canGoForward 정확히 추적하기 (BrowserView 로직)
```javascript
// src/views/BrowserView.js

const [historyStack, setHistoryStack] = useState([])    // 방문 URL 스택
const [historyIndex, setHistoryIndex] = useState(-1)    // 현재 위치 인덱스

const handleNavigate = ({ action, url }) => {
	logger.info('[BrowserView] 네비게이션 동작:', { action, url })

	switch (action) {
		case 'back':
			if (historyIndex > 0) {
				setHistoryIndex(historyIndex - 1)
				setCurrentUrl(historyStack[historyIndex - 1])
			}
			break

		case 'forward':
			if (historyIndex < historyStack.length - 1) {
				setHistoryIndex(historyIndex + 1)
				setCurrentUrl(historyStack[historyIndex + 1])
			}
			break

		case 'reload':
			// URL 변경 없음, WebView에서 reload() 호출됨
			break

		case 'home':
			// 홈으로 이동 → 스택 정리
			setCurrentUrl(url)
			setHistoryStack([url])
			setHistoryIndex(0)
			break

		default:
			logger.warn('[BrowserView] 알 수 없는 네비게이션 동작:', action)
	}
}

const handleNavigationChange = ({ url }) => {
	logger.info('[BrowserView] URL 변경:', url)

	// 새 URL로 탐색 시 스택 업데이트
	if (url !== historyStack[historyIndex]) {
		// 현재 위치 이후 스택 제거 (앞으로 기록 삭제)
		const newStack = historyStack.slice(0, historyIndex + 1)
		newStack.push(url)
		setHistoryStack(newStack)
		setHistoryIndex(newStack.length - 1)
		setCurrentUrl(url)
	}
}

// canGoBack, canGoForward 계산
const canGoBack = historyIndex > 0
const canGoForward = historyIndex < historyStack.length - 1
```

## 7. Enact Spotlight 통합 설계

### NavigationBar Spotlight 설정
```javascript
// src/components/NavigationBar/NavigationBar.js
import Button from '@enact/moonstone/Button'
import Spotlight from '@enact/spotlight'

const NavigationBar = ({ webviewRef, canGoBack, canGoForward, homeUrl, onNavigate, className }) => {
	return (
		<div
			className={`${css.navigationBar} ${className}`}
			data-spotlight-container="navigation-bar"
			data-spotlight-id="navigation-bar"
		>
			{/* 뒤로 가기 버튼 */}
			<Button
				className={css.navButton}
				disabled={!canGoBack}
				onClick={handleBack}
				icon="arrowlargeleft"
				spotlightId="nav-back"
			>
				뒤로
			</Button>

			{/* 앞으로 가기 버튼 */}
			<Button
				className={css.navButton}
				disabled={!canGoForward}
				onClick={handleForward}
				icon="arrowlargeright"
				spotlightId="nav-forward"
			>
				앞으로
			</Button>

			{/* 새로고침 버튼 */}
			<Button
				className={css.navButton}
				onClick={handleReload}
				icon="refresh"
				spotlightId="nav-reload"
			>
				새로고침
			</Button>

			{/* 홈 버튼 */}
			<Button
				className={css.navButton}
				onClick={handleHome}
				icon="home"
				spotlightId="nav-home"
			>
				홈
			</Button>
		</div>
	)
}
```

### Spotlight 포커스 흐름
```
URLBar (F-03)
    │
    ▼ (아래 방향키)
WebView (spotlightId: "webview-main")
    │
    ▼ (아래 방향키 or Back 키)
NavigationBar (data-spotlight-container)
    │
    ├── 뒤로 버튼 (spotlightId: "nav-back")
    ├── 앞으로 버튼 (spotlightId: "nav-forward")
    ├── 새로고침 버튼 (spotlightId: "nav-reload")
    └── 홈 버튼 (spotlightId: "nav-home")
         │
         ▼ (좌/우 방향키로 버튼 간 이동)
```

### Spotlight 설정 (BrowserView)
```javascript
// src/views/BrowserView.js
import Spotlight from '@enact/spotlight'

useEffect(() => {
	// 초기 포커스를 WebView로 설정
	Spotlight.focus('webview-main')

	// Spotlight 방향 설정 (위/아래 키로 WebView ↔ NavigationBar 전환)
	Spotlight.set('webview-main', {
		defaultElement: '.webviewContainer',
		enterTo: 'default-element',
		leaveFor: {
			down: 'navigation-bar'  // 아래 방향키: NavigationBar로
		}
	})

	Spotlight.set('navigation-bar', {
		defaultElement: '[spotlightId="nav-back"]',
		enterTo: 'default-element',
		leaveFor: {
			up: 'webview-main'  // 위 방향키: WebView로
		}
	})
}, [])
```

## 8. 리모컨 Back 키 처리 설계

### 커스텀 Hook: useRemoteControl
```javascript
// src/hooks/useRemoteControl.js
import { useEffect } from 'react'
import logger from '../utils/logger'

/**
 * 리모컨 Back 키 처리 Hook
 * @param {Object} options
 * @param {boolean} options.isWebViewFocused - WebView에 포커스 있는지 여부
 * @param {Function} options.onBackInWebView - WebView 포커스 시 Back 키 동작
 * @param {Function} options.onBackOutsideWebView - WebView 외부 포커스 시 Back 키 동작
 */
const useRemoteControl = ({ isWebViewFocused, onBackInWebView, onBackOutsideWebView }) => {
	useEffect(() => {
		const handleKeyDown = (event) => {
			// Back 키 감지 (Backspace 또는 webOS Back 키)
			if (event.key === 'Backspace' || event.keyCode === 8 || event.keyCode === 461) {
				logger.debug('[useRemoteControl] Back 키 감지')

				// WebView 포커스 여부에 따라 분기
				if (isWebViewFocused) {
					// WebView 포커스 시: 브라우저 뒤로 가기
					if (onBackInWebView) {
						onBackInWebView()
						event.preventDefault()  // 기본 동작 방지 (페이지 뒤로 가기)
					}
				} else {
					// NavigationBar 등 외부 포커스 시: Spotlight 포커스 이탈
					if (onBackOutsideWebView) {
						onBackOutsideWebView()
						// 기본 동작 허용 (Spotlight 포커스 이동)
					}
				}
			}
		}

		// 전역 keydown 이벤트 리스너 등록
		window.addEventListener('keydown', handleKeyDown)

		// cleanup
		return () => {
			window.removeEventListener('keydown', handleKeyDown)
		}
	}, [isWebViewFocused, onBackInWebView, onBackOutsideWebView])
}

export default useRemoteControl
```

### BrowserView에서 useRemoteControl 사용
```javascript
// src/views/BrowserView.js
import useRemoteControl from '../hooks/useRemoteControl'
import Spotlight from '@enact/spotlight'

const BrowserView = () => {
	const webviewRef = useRef(null)
	const [isWebViewFocused, setIsWebViewFocused] = useState(true)

	// Spotlight 포커스 변경 감지
	useEffect(() => {
		const handleSpotlightFocus = (event) => {
			const focusedId = event.detail?.spotlightId
			setIsWebViewFocused(focusedId === 'webview-main')
		}

		window.addEventListener('spotlightfocus', handleSpotlightFocus)

		return () => {
			window.removeEventListener('spotlightfocus', handleSpotlightFocus)
		}
	}, [])

	// 리모컨 Back 키 처리
	useRemoteControl({
		isWebViewFocused,
		onBackInWebView: () => {
			// WebView 포커스 시: 브라우저 뒤로 가기
			if (canGoBack && webviewRef.current) {
				try {
					webviewRef.current.contentWindow.history.back()
					logger.info('[BrowserView] 리모컨 Back 키 → 브라우저 뒤로')
				} catch (error) {
					logger.error('[BrowserView] 브라우저 뒤로 실패:', error)
				}
			} else {
				// 뒤로 갈 수 없을 때: 포커스를 NavigationBar로 이동
				Spotlight.focus('navigation-bar')
				logger.info('[BrowserView] 리모컨 Back 키 → NavigationBar 포커스')
			}
		},
		onBackOutsideWebView: () => {
			// NavigationBar 포커스 시: Spotlight 기본 동작 (포커스 이탈)
			logger.info('[BrowserView] 리모컨 Back 키 → Spotlight 포커스 이탈')
		}
	})

	// ... 기존 코드 ...
}
```

## 9. 시퀀스 다이어그램

### 시나리오 1: 뒤로 가기 버튼 클릭 (정상 동작)
```
사용자    NavigationBar    BrowserView    WebView (iframe)
  │              │               │                │
  │  리모컨 선택 │               │                │
  │─────────────▶│               │                │
  │              │  handleBack() │                │
  │              │  canGoBack 확인│               │
  │              │  (true)        │               │
  │              │               │                │
  │              │  webviewRef.contentWindow      │
  │              │    .history.back()             │
  │              │────────────────────────────────▶│
  │              │               │                │  history.back()
  │              │               │                │  이전 페이지 로드
  │              │               │                │
  │              │  onNavigate({ action: 'back' })│
  │              │──────────────▶│                │
  │              │               │  historyIndex--│
  │              │               │  setCurrentUrl │
  │              │               │                │
  │              │               │  onNavigationChange({ url })
  │              │               │◀───────────────│
  │              │               │  setCanGoBack  │
  │              │               │  setCanGoForward│
  │              │               │                │
  │              │◀ canGoBack    │                │
  │              │   canGoForward│                │
  │              │   Props 업데이트│              │
  │              │  (버튼 상태 변경)              │
```

### 시나리오 2: 리모컨 Back 키 (WebView 포커스)
```
사용자    BrowserView    useRemoteControl    WebView (iframe)
  │              │               │                │
  │  리모컨      │               │                │
  │  Back 키     │               │                │
  │─────────────▶│               │                │
  │              │  onKeyDown    │                │
  │              │──────────────▶│                │
  │              │               │  isWebViewFocused?
  │              │               │  (true)         │
  │              │               │                │
  │              │               │  onBackInWebView()
  │              │◀──────────────│                │
  │              │  canGoBack 확인│               │
  │              │  (true)        │               │
  │              │  webviewRef.contentWindow      │
  │              │    .history.back()             │
  │              │────────────────────────────────▶│
  │              │               │                │  history.back()
  │              │               │                │  이전 페이지 로드
```

### 시나리오 3: 뒤로 가기 버튼 비활성화 (첫 페이지)
```
사용자    NavigationBar    BrowserView    WebView (iframe)
  │              │               │                │
  │  첫 페이지   │               │                │
  │  (Google)    │               │                │
  │              │               │  onNavigationChange
  │              │               │◀───────────────│
  │              │               │  { url, canGoBack: false }
  │              │               │  setCanGoBack(false)
  │              │               │                │
  │              │◀ canGoBack    │                │
  │              │   = false     │                │
  │              │  Props 업데이트│              │
  │              │  (뒤로 버튼 비활성화)          │
  │              │  opacity: 0.5 │                │
  │              │  disabled      │                │
  │              │               │                │
  │  리모컨 선택 │               │                │
  │─────────────▶│               │                │
  │              │  handleBack() │                │
  │              │  canGoBack 확인│               │
  │              │  (false) → return              │
  │              │  (동작 안 함) │                │
```

### 시나리오 4: 홈 버튼 클릭
```
사용자    NavigationBar    BrowserView    WebView
  │              │               │                │
  │  리모컨 선택 │               │                │
  │─────────────▶│               │                │
  │              │  handleHome() │                │
  │              │  onNavigate({ action: 'home',  │
  │              │               url: homeUrl })  │
  │              │──────────────▶│                │
  │              │               │  setCurrentUrl(homeUrl)
  │              │               │  setHistoryStack([homeUrl])
  │              │               │  setHistoryIndex(0)
  │              │               │                │
  │              │               │  url Props 변경
  │              │               │────────────────▶│
  │              │               │                │  iframe.src 변경
  │              │               │                │  Google 홈페이지 로드
  │              │               │                │
  │              │               │  onLoadEnd()   │
  │              │               │◀───────────────│
  │              │               │  onNavigationChange
  │              │               │◀───────────────│
  │              │               │  setCanGoBack(false)
  │              │               │  setCanGoForward(false)
  │              │               │                │
  │              │◀ canGoBack    │                │
  │              │   = false     │                │
  │              │  (뒤로/앞으로 비활성화)        │
```

## 10. 에러 처리 설계

### iframe history API 호출 실패
```javascript
// NavigationBar.js

const handleBack = useCallback(() => {
	if (!canGoBack || isNavigating) return

	setIsNavigating(true)

	try {
		if (webviewRef.current && webviewRef.current.contentWindow) {
			webviewRef.current.contentWindow.history.back()

			if (onNavigate) {
				onNavigate({ action: 'back' })
			}

			logger.info('[NavigationBar] 뒤로 가기 실행')
		}
	} catch (error) {
		// CORS 에러 또는 보안 정책으로 history API 호출 실패
		// 조용히 실패 (사용자에게 에러 표시 없음, 콘솔 로그만)
		logger.error('[NavigationBar] 뒤로 가기 실패 (CORS 또는 보안 정책):', error)

		// 에러 시에도 UI는 정상 유지 (버튼 상태 변경 없음)
		// BrowserView의 히스토리 스택은 이미 업데이트되었으므로 되돌리지 않음
	} finally {
		setTimeout(() => setIsNavigating(false), 500)
	}
}, [canGoBack, isNavigating, webviewRef, onNavigate])
```

### CORS 환경에서 히스토리 상태 감지 실패
```javascript
// WebView.js

useEffect(() => {
	navigationIntervalRef.current = setInterval(() => {
		try {
			if (iframeRef.current && iframeRef.current.contentWindow) {
				const newUrl = iframeRef.current.contentWindow.location.href

				if (newUrl !== currentUrlRef.current) {
					currentUrlRef.current = newUrl

					// 히스토리 상태 전달
					const historyState = {
						url: newUrl,
						canGoBack: true,   // CORS 환경에서는 항상 true (정확도 낮음)
						canGoForward: false
					}

					if (onNavigationChange) {
						onNavigationChange(historyState)
					}

					logger.info('[WebView] URL 변경 감지:', historyState)
				}
			}
		} catch (error) {
			// CORS 에러 시 무시 (Same-Origin Policy)
			// 이 경우 NavigationBar 버튼은 항상 활성화 상태 유지
			// 버튼 클릭 시 동작 실패해도 조용히 실패
		}
	}, 500)

	return () => {
		if (navigationIntervalRef.current) {
			clearInterval(navigationIntervalRef.current)
		}
	}
}, [onNavigationChange])
```

**대응 전략**: CORS 제약이 있는 외부 사이트에서는 히스토리 상태를 정확히 감지할 수 없으므로, NavigationBar 버튼을 항상 활성화하고, 버튼 클릭 시 동작 실패 시에도 에러 UI 표시 없이 조용히 실패합니다. 사용자는 버튼 클릭 후 페이지가 변경되지 않으면 현재 상태에서 해당 동작이 불가능함을 직관적으로 인지합니다.

## 11. 스타일링 설계

### NavigationBar 스타일
```less
// src/components/NavigationBar/NavigationBar.module.less

.navigationBar {
	display: flex;
	justify-content: center;
	align-items: center;
	gap: 20px;  // 버튼 간격 20px (요구사항 NFR-2)
	height: 100px;
	padding: 0 var(--spacing-lg);
	background-color: #2a2a2a;
	border-top: 1px solid #444;
}

.navButton {
	min-width: 100px;   // 최소 폭 100px (요구사항 NFR-2)
	min-height: 80px;   // 최소 높이 80px (요구사항 NFR-2)
	font-size: 16px;    // 폰트 크기 16px (요구사항 NFR-2)

	// Moonstone Button은 기본적으로 포커스 스타일 제공
	// 추가 커스터마이징 필요 시 아래 선택자 사용
	&:global(.spottable):focus {
		border: 2px solid white;  // 포커스 테두리 2px (요구사항 NFR-2)
		outline: none;
	}

	// 비활성화 상태 (요구사항 NFR-3)
	&:disabled {
		opacity: 0.5;        // 비활성 버튼 흐릿하게
		cursor: not-allowed;
	}

	// 클릭 애니메이션 (요구사항 NFR-3)
	&:active {
		transform: scale(0.95);
		transition: transform 0.1s ease;
	}
}
```

### BrowserView 레이아웃 조정
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
	gap: 10px;
}

.webviewWrapper {
	flex: 1;  // 남은 공간 전체 차지
	overflow: hidden;
}

// NavigationBar placeholder 삭제 (실제 컴포넌트로 대체)
// .navBarPlaceholder는 삭제
```

## 12. 기술적 주의사항

### iframe history API의 제약사항
- **Same-Origin Policy**: iframe의 contentWindow.history는 다른 도메인 페이지에서 접근 제한
- **history.length**: 히스토리 항목 개수를 알 수 있지만, 현재 인덱스는 알 수 없음
- **canGoForward 감지 불가**: 브라우저는 앞으로 갈 수 있는지 여부를 외부에서 직접 확인할 방법 없음
- **대응**: BrowserView에서 히스토리 스택을 별도로 관리하여 canGoBack, canGoForward를 정확히 추적

### 리모컨 Back 키 코드
- webOS 버전에 따라 Back 키 코드가 다를 수 있음 (Backspace: 8, webOS Back: 461)
- **테스트 필수**: 실제 LG 프로젝터 HU175QW에서 Back 키 코드 확인
- **대응**: useRemoteControl Hook에서 여러 키 코드를 모두 처리

### Enact Spotlight의 포커스 우선순위
- Spotlight는 disabled 버튼에 포커스를 주지 않음 (자동 건너뜀)
- 4개 버튼 중 일부가 비활성화되면 포커스 순서가 자동으로 조정됨
- **주의**: 첫 페이지에서 뒤로 버튼이 비활성화되면, 앞으로 버튼으로 첫 포커스 이동

### 중복 클릭 방지
- 네비게이션 동작(뒤로/앞으로/새로고침)은 페이지 로딩 시간이 소요됨
- 연속 클릭 시 iframe이 여러 번 동작하여 의도하지 않은 결과 발생 가능
- **대응**: isNavigating 플래그로 0.5초 동안 중복 클릭 차단

### 메모리 누수 방지
- useCallback의 의존성 배열에 state 포함 시 무한 재생성 가능
- **대응**: 필요한 의존성만 포함, 불필요한 state는 ref로 대체

### CORS 환경에서의 사용자 경험
- CORS로 히스토리 상태 감지 실패 시 버튼이 항상 활성화됨
- 사용자가 클릭해도 동작하지 않을 수 있음 (첫 페이지에서 뒤로 버튼 등)
- **대응**: 버튼 클릭 시 조용히 실패하여 앱이 중단되지 않도록 함. 사용자는 페이지가 변경되지 않으면 해당 동작이 불가능함을 직관적으로 인지

## 13. 구현 순서

### Phase 1: NavigationBar 컴포넌트 기본 구조
1. `src/components/NavigationBar/` 디렉토리 생성
2. `NavigationBar.js` 생성 (Props 인터페이스, PropTypes 정의)
3. `NavigationBar.module.less` 생성 (기본 스타일)
4. `index.js` 생성 (export default)

### Phase 2: 버튼 UI 구현
1. Enact Moonstone Button import
2. 4개 버튼 배치 (뒤로, 앞으로, 새로고침, 홈)
3. 버튼 아이콘 설정 (icon props)
4. 버튼 레이아웃 스타일링 (Flexbox, gap)
5. 비활성화 상태 스타일링 (disabled, opacity)

### Phase 3: 버튼 이벤트 핸들러 구현
1. handleBack, handleForward, handleReload, handleHome 함수 작성
2. webviewRef를 통해 iframe history API 호출
3. onNavigate 콜백 호출
4. isNavigating 플래그로 중복 클릭 방지
5. try-catch로 에러 처리 (조용히 실패)

### Phase 4: WebView ref 노출 (WebView 수정)
1. WebView 컴포넌트를 forwardRef로 변환
2. useImperativeHandle로 iframe ref 노출
3. goBack, goForward, reload 편의 메서드 추가 (선택)

### Phase 5: BrowserView 통합
1. webviewRef 생성 (useRef)
2. WebView에 ref 전달
3. canGoBack, canGoForward 상태 관리
4. historyStack, historyIndex 상태 추가 (정확한 히스토리 추적)
5. handleNavigate 함수 구현 (히스토리 스택 업데이트)
6. NavigationBar 컴포넌트 import 및 사용
7. Props 전달 (webviewRef, canGoBack, canGoForward, homeUrl, onNavigate)

### Phase 6: Spotlight 통합
1. NavigationBar에 spotlightId 설정
2. BrowserView에서 Spotlight.set으로 포커스 흐름 정의
3. WebView ↔ NavigationBar 간 포커스 전환 테스트 (위/아래 방향키)
4. 버튼 간 포커스 이동 테스트 (좌/우 방향키)

### Phase 7: 리모컨 Back 키 처리
1. useRemoteControl Hook 생성 (src/hooks/)
2. BrowserView에서 isWebViewFocused 상태 관리
3. useRemoteControl 사용 (onBackInWebView, onBackOutsideWebView)
4. Back 키 이벤트 핸들러 구현 (webviewRef.contentWindow.history.back())

### Phase 8: 히스토리 상태 동기화
1. WebView의 onNavigationChange 콜백에 canGoBack, canGoForward 추가 (CORS 제약 고려)
2. BrowserView에서 히스토리 스택 관리 로직 구현
3. NavigationBar에 정확한 canGoBack, canGoForward Props 전달
4. 버튼 활성/비활성 상태 자동 업데이트 확인

### Phase 9: 로컬 테스트
1. `npm run serve` 실행
2. Google 홈페이지 로드 → 뒤로 버튼 비활성화 확인
3. 검색 후 페이지 이동 → 뒤로 버튼 활성화 확인
4. 뒤로 버튼 클릭 → 이전 페이지 복귀 확인
5. 앞으로 버튼 활성화 확인
6. 새로고침 버튼 클릭 → 페이지 재로드 확인
7. 홈 버튼 클릭 → Google 홈페이지 복귀 확인
8. 키보드 방향키로 버튼 포커스 이동 확인 (탭 키로 로컬 테스트)

### Phase 10: 실제 디바이스 테스트
1. `npm run pack-p` 빌드
2. `ares-package dist/` IPK 생성
3. 프로젝터 설치 및 실행
4. 리모컨 방향키로 버튼 포커스 이동 확인
5. 리모컨 선택 버튼으로 버튼 클릭 확인
6. 리모컨 Back 키 동작 확인 (WebView 포커스 시 브라우저 뒤로)
7. 주요 사이트(YouTube, Naver, Google)에서 뒤로/앞으로 동작 확인
8. 3m 거리에서 버튼 아이콘 명확히 인식 가능한지 확인

## 14. 확장성 고려사항

### F-06(탭 관리)와의 연동
- **현재**: 단일 WebView 인스턴스 제어
- **F-06 시**: 활성 탭의 WebView만 제어하도록 변경
- **준비**:
  - NavigationBar는 webviewRef Props로 받으므로, BrowserView가 활성 탭의 ref를 전달하도록 수정하면 됨
  - 탭별 히스토리 스택 관리 (historyStack을 탭 ID별로 분리)

### F-08(히스토리 관리)와의 연동
- **현재**: BrowserView가 히스토리 스택을 메모리에서만 관리
- **F-08 시**: 히스토리 스택을 LS2 API로 영구 저장
- **준비**:
  - handleNavigationChange에서 historyService.addHistory(url) 호출 추가
  - 앱 재시작 시 LS2에서 히스토리 불러와 스택 복원

### F-11(설정 화면)과의 연동
- **현재**: 홈페이지 URL이 하드코딩 (https://www.google.com)
- **F-11 시**: settingsService.getHomeUrl()로 설정 읽기
- **준비**:
  - BrowserView에서 homeUrl을 state로 관리
  - useEffect로 앱 시작 시 settingsService에서 homeUrl 불러오기
  - NavigationBar에 Props 전달

### F-12(리모컨 단축키)와의 연동
- **현재**: Back 키만 처리
- **F-12 시**: 채널 버튼, 컬러 버튼으로 뒤로/앞으로/새로고침 매핑
- **준비**:
  - useRemoteControl Hook에 onChannelUp, onColorRed 등 콜백 추가
  - BrowserView에서 단축키 설정 읽어와 매핑

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-12 | 최초 작성 | F-04 요구사항 기반 기술 설계 |
