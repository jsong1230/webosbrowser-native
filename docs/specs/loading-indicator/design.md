# 로딩 인디케이터 — 기술 설계서

## 1. 참조
- 요구사항 분석서: `docs/specs/loading-indicator/requirements.md`
- WebView 설계서: `docs/specs/webview-integration/design.md`
- PRD: `docs/project/prd.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md`

## 2. 아키텍처 개요

### 전체 구조
WebView 컴포넌트의 로딩 상태(onLoadStart, onLoadEnd, onLoadError)를 기반으로 시각적 피드백을 제공하는 LoadingBar 컴포넌트를 구현합니다. iframe의 실제 로딩 진행률을 알 수 없으므로 시간 기반 가상 진행률 알고리즘을 적용합니다.

```
┌─────────────────────────────────────────────────────────┐
│                      BrowserView                         │
│  ┌────────────────────────────────────────────────────┐ │
│  │                   URLBar (F-03)                     │ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │              LoadingBar (F-05)                      │ │
│  │  ┌──────────────────────────────────────────────┐  │ │
│  │  │   [========>        ] 65%                    │  │ │
│  │  └──────────────────────────────────────────────┘  │ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │                  WebView Component                  │ │
│  │  ┌──────────────────────────────────────────────┐  │ │
│  │  │           <iframe> (웹 콘텐츠)                │  │ │
│  │  └──────────────────────────────────────────────┘  │ │
│  │  [ 로딩 스피너 오버레이 (중앙) ]                   │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### 핵심 설계 원칙
1. **가상 진행률 기반**: iframe은 실제 진행률을 제공하지 않으므로 시간 기반 가상 진행률 계산
2. **이벤트 기반 동기화**: WebView의 onLoadStart, onLoadEnd, onLoadError 이벤트로 상태 동기화
3. **성능 최적화**: CSS transform, opacity만 사용하여 GPU 가속 애니메이션 구현
4. **Enact UI 통합**: Moonstone Spinner 활용, 프로그레스바는 커스텀 구현 (Moonstone ProgressBar는 정적 진행률용)
5. **원거리 가독성**: 최소 8px 높이의 프로그레스바, 80px 크기의 스피너

## 3. 아키텍처 결정

### 결정 1: 프로그레스바 구현 방식
- **선택지**:
  - A) Enact `@enact/moonstone/ProgressBar` 사용
  - B) 커스텀 HTML + CSS 애니메이션 구현
  - C) Canvas API로 그래픽 렌더링
- **결정**: B) 커스텀 HTML + CSS 애니메이션
- **근거**:
  - Enact ProgressBar는 정적 진행률 표시용으로, 부드러운 가상 진행률 애니메이션에 부적합
  - CSS transform (scaleX)로 GPU 가속 애니메이션 구현 가능 (성능 우수)
  - Canvas는 오버헤드가 크고 webOS 프로젝터 환경에서 성능 저하 우려
  - 커스텀 구현으로 Ease-out 곡선, 페이드아웃 등 세밀한 제어 가능
- **트레이드오프**: Enact 테마 자동 적용 불가 → CSS Variables로 테마 색상 직접 사용

### 결정 2: 스피너 구현 방식
- **선택지**:
  - A) Enact `@enact/moonstone/Spinner` 사용
  - B) 커스텀 CSS 회전 애니메이션
  - C) GIF/SVG 애니메이션 파일
- **결정**: A) Enact `@enact/moonstone/Spinner`
- **근거**:
  - Enact Spinner는 부드러운 회전 애니메이션을 제공하며 성능 검증됨
  - Moonstone 테마와 일관성 유지 (다크/라이트 모드 자동 적용)
  - 크기 조정 가능 (`size` prop 제공)
  - 커스텀 구현은 브라우저 호환성, 성능 테스트 부담
- **트레이드오프**: Enact 의존성 증가 (이미 프로젝트에 포함되어 있으므로 추가 부담 없음)

### 결정 3: 가상 진행률 알고리즘
- **선택지**:
  - A) 선형 증가 (0% → 100%, 일정 속도)
  - B) 단계별 증가 (0→60% 빠름, 60→90% 느림, 90→95% 매우 느림)
  - C) 지수 함수 기반 Ease-out 곡선
- **결정**: B) 단계별 증가 + C) Ease-out 애니메이션 조합
- **근거**:
  - **체감 속도 개선**: 초반 빠른 증가로 사용자에게 "로딩이 진행 중"임을 즉시 인지
  - **실제 로딩 대기**: 중반부터 느려져 실제 네트워크 로딩 완료를 대기
  - **무한 대기 방지**: 90% 이상에서 매우 느리게 증가하여 100%에 도달하지 않음 (실제 완료 이벤트 대기)
  - **자연스러운 전환**: CSS Ease-out 곡선으로 부드러운 애니메이션
- **알고리즘**:
  - 0~3초: 0% → 60% (20%/초 속도)
  - 3~10초: 60% → 90% (4.3%/초 속도)
  - 10초~완료: 90% → 95% (0.5%/초 속도)
  - 완료 이벤트 수신 시: 현재 진행률 → 100% (즉시 이동 후 페이드아웃)

### 결정 4: 애니메이션 프레임워크
- **선택지**:
  - A) setInterval로 진행률 업데이트
  - B) requestAnimationFrame으로 진행률 업데이트
  - C) CSS keyframes 애니메이션만 사용 (JavaScript 없음)
- **결정**: B) requestAnimationFrame
- **근거**:
  - requestAnimationFrame은 브라우저 리페인트에 동기화되어 부드러운 60fps 애니메이션 보장
  - setInterval은 백그라운드 탭에서 throttle되어 애니메이션 끊김 가능
  - CSS keyframes만으로는 동적 완료 시점 처리 불가 (로딩 완료 이벤트 대응 필요)
  - webOS Chromium 53+에서 requestAnimationFrame 지원 확인됨
- **트레이드오프**: 약간의 복잡도 증가 (타이머 관리, cleanup 필요)

### 결정 5: 로딩 오버레이 구조
- **선택지**:
  - A) WebView 내부에 오버레이 통합 (현재 구조 유지)
  - B) LoadingBar를 독립 컴포넌트로 분리 → BrowserView에서 별도 렌더링
  - C) 전역 Portal로 오버레이 렌더링
- **결정**: A) WebView 내부 통합 + B) LoadingBar 독립 컴포넌트 추가
- **근거**:
  - **WebView 내부 스피너**: 웹 콘텐츠 중앙에 오버레이되어야 하므로 WebView 내부가 적합
  - **독립 LoadingBar**: 화면 상단에 고정되는 프로그레스바는 BrowserView 레벨에서 관리 (URLBar 아래 배치)
  - **관심사 분리**: LoadingBar는 재사용 가능한 독립 컴포넌트로 설계 (향후 F-06 탭 관리에서 탭별 로딩 표시 재사용)
- **구조**:
  ```
  BrowserView
  ├── URLBar (F-03)
  ├── LoadingBar (F-05) ← 새로 추가
  └── WebView
      └── LoadingSpinner (오버레이, WebView 내부 유지)
  ```

### 결정 6: 에러 상태 시각적 피드백
- **선택지**:
  - A) 프로그레스바 색상만 변경 (녹색 → 빨간색)
  - B) 프로그레스바 + 스피너를 에러 아이콘으로 교체
  - C) 에러 오버레이만 표시 (프로그레스바 숨김)
- **결정**: A) + B) 조합 (프로그레스바 빨간색 + 스피너 → 에러 아이콘)
- **근거**:
  - 색상 변경만으로는 색맹 사용자에게 인지 어려움 (접근성)
  - 에러 아이콘(⚠️)으로 명확한 시각적 피드백 제공
  - 1초간 표시 후 F-02 WebView의 ErrorOverlay로 전환 (현재 구조 유지)
- **트레이드오프**: 약간의 복잡도 증가 (에러 상태 분기 처리)

## 4. LoadingBar 컴포넌트 설계

### 컴포넌트 구조
```
src/components/LoadingBar/
├── LoadingBar.js           # 메인 컴포넌트
├── LoadingBar.module.less  # 스타일
└── index.js                # Export 진입점
```

### Props 인터페이스
```javascript
// src/components/LoadingBar/LoadingBar.js
import PropTypes from 'prop-types'

LoadingBar.propTypes = {
	// 로딩 상태
	isLoading: PropTypes.bool.isRequired,        // 로딩 중 여부
	isError: PropTypes.bool,                     // 에러 상태 여부

	// 진행률 제어 (선택, 외부에서 제어 시 사용)
	progress: PropTypes.number,                  // 0~100, 지정 시 가상 진행률 무시

	// 콜백
	onLoadComplete: PropTypes.func,              // 100% 도달 후 페이드아웃 완료 시 호출

	// 스타일 커스터마이징
	className: PropTypes.string,
	barHeight: PropTypes.number,                 // 프로그레스바 높이 (기본 8px)
	barColor: PropTypes.string,                  // 프로그레스바 색상 (기본 Moonstone 액센트 색)
	errorColor: PropTypes.string                 // 에러 시 색상 (기본 빨간색)
}

LoadingBar.defaultProps = {
	isError: false,
	progress: null,  // null이면 가상 진행률 사용
	onLoadComplete: () => {},
	className: '',
	barHeight: 8,
	barColor: 'var(--accent-color)',             // Moonstone 테마 변수
	errorColor: '#d32f2f'                        // Material Design Red 700
}
```

### 상태 정의
```javascript
// 가상 진행률 상태
const [virtualProgress, setVirtualProgress] = useState(0)  // 0~100

// 애니메이션 상태
const [isAnimating, setIsAnimating] = useState(false)

// 페이드아웃 상태
const [isFadingOut, setIsFadingOut] = useState(false)

// requestAnimationFrame 참조
const rafRef = useRef(null)
const startTimeRef = useRef(0)
```

### 가상 진행률 알고리즘 구현
```javascript
/**
 * 가상 진행률 계산 함수
 * @param {number} elapsedMs - 로딩 시작부터 경과한 밀리초
 * @returns {number} - 0~95 범위의 진행률
 */
const calculateVirtualProgress = (elapsedMs) => {
	const seconds = elapsedMs / 1000

	if (seconds < 3) {
		// 0~3초: 0% → 60% (빠른 증가)
		return (seconds / 3) * 60
	} else if (seconds < 10) {
		// 3~10초: 60% → 90% (중간 속도)
		const t = (seconds - 3) / 7  // 0~1 범위로 정규화
		return 60 + t * 30
	} else {
		// 10초 이후: 90% → 95% (매우 느림)
		const t = Math.min((seconds - 10) / 20, 1)  // 20초 동안 5% 증가 (0.25%/초)
		return 90 + t * 5
	}
}

/**
 * requestAnimationFrame 기반 진행률 업데이트
 */
useEffect(() => {
	if (!isLoading || isError) {
		// 로딩 중이 아니거나 에러 상태면 애니메이션 중단
		if (rafRef.current) {
			cancelAnimationFrame(rafRef.current)
			rafRef.current = null
		}
		return
	}

	// 로딩 시작 시각 기록
	startTimeRef.current = Date.now()
	setIsAnimating(true)

	// 애니메이션 루프
	const animate = () => {
		const elapsed = Date.now() - startTimeRef.current
		const newProgress = calculateVirtualProgress(elapsed)

		setVirtualProgress(newProgress)

		// 95%에 도달하면 애니메이션 중단 (실제 완료 이벤트 대기)
		if (newProgress < 95) {
			rafRef.current = requestAnimationFrame(animate)
		} else {
			setIsAnimating(false)
		}
	}

	rafRef.current = requestAnimationFrame(animate)

	// cleanup
	return () => {
		if (rafRef.current) {
			cancelAnimationFrame(rafRef.current)
			rafRef.current = null
		}
	}
}, [isLoading, isError])
```

### 로딩 완료 처리 (100% 이동 + 페이드아웃)
```javascript
useEffect(() => {
	// isLoading이 false가 되면 (로딩 완료) 100%로 즉시 이동
	if (!isLoading && virtualProgress > 0 && !isError) {
		setVirtualProgress(100)

		// 0.5초 후 페이드아웃 시작
		const fadeOutTimer = setTimeout(() => {
			setIsFadingOut(true)

			// 페이드아웃 애니메이션 완료 후 (0.5초) 콜백 호출
			setTimeout(() => {
				setVirtualProgress(0)
				setIsFadingOut(false)
				onLoadComplete()
			}, 500)
		}, 100)  // 100% 도달 후 0.1초 유지

		return () => clearTimeout(fadeOutTimer)
	}
}, [isLoading, virtualProgress, isError, onLoadComplete])
```

### 에러 처리
```javascript
useEffect(() => {
	if (isError) {
		// 에러 발생 시 애니메이션 중단
		if (rafRef.current) {
			cancelAnimationFrame(rafRef.current)
			rafRef.current = null
		}

		// 1초 후 페이드아웃
		const errorTimer = setTimeout(() => {
			setVirtualProgress(0)
		}, 1000)

		return () => clearTimeout(errorTimer)
	}
}, [isError])
```

## 5. WebView 내 LoadingSpinner 개선

### LoadingSpinner 컴포넌트 (WebView 내부)
```javascript
// src/components/WebView/LoadingSpinner.js
import Spinner from '@enact/moonstone/Spinner'
import css from './LoadingSpinner.module.less'

const LoadingSpinner = ({ url, isError }) => {
	if (isError) {
		// 에러 아이콘 표시
		return (
			<div className={css.spinnerOverlay}>
				<div className={css.errorIcon}>⚠️</div>
				<p className={css.loadingText}>페이지 로딩 실패</p>
			</div>
		)
	}

	// 로딩 스피너 표시
	return (
		<div className={css.spinnerOverlay}>
			<Spinner size="large" className={css.spinner} />
			<p className={css.loadingText}>페이지 로딩 중...</p>
			{url && (
				<p className={css.loadingUrl}>
					{url.length > 50 ? `${url.slice(0, 50)}...` : url}
				</p>
			)}
		</div>
	)
}

export default LoadingSpinner
```

### LoadingSpinner 스타일
```less
// src/components/WebView/LoadingSpinner.module.less
.spinnerOverlay {
	position: absolute;
	top: 0;
	left: 0;
	right: 0;
	bottom: 0;
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	background-color: rgba(0, 0, 0, 0.7);  // 반투명 오버레이
	z-index: 1000;
}

.spinner {
	// Enact Spinner 크기 커스터마이징
	width: 80px !important;
	height: 80px !important;
}

.errorIcon {
	font-size: 80px;
	line-height: 1;
	margin-bottom: 16px;
}

.loadingText {
	color: var(--text-color);
	font-size: 24px;  // PRD 최소 가독성 기준
	margin-top: 16px;
	margin-bottom: 8px;
}

.loadingUrl {
	color: var(--text-secondary-color);
	font-size: 18px;
	max-width: 80%;
	text-align: center;
	word-break: break-all;
}
```

## 6. LoadingBar 스타일 설계

### CSS 구조
```less
// src/components/LoadingBar/LoadingBar.module.less
.loadingBarContainer {
	position: relative;
	width: 100%;
	height: 8px;  // 기본 높이 (prop으로 조정 가능)
	background-color: rgba(255, 255, 255, 0.1);  // 배경 트랙
	overflow: hidden;
}

.loadingBar {
	position: absolute;
	top: 0;
	left: 0;
	height: 100%;
	background-color: var(--accent-color);  // Moonstone 액센트 색 (녹색)
	transform-origin: left center;
	transform: scaleX(0);  // 초기 0%
	transition: transform 0.1s ease-out;  // 부드러운 애니메이션
	will-change: transform;  // GPU 가속 힌트
}

.loadingBar.error {
	background-color: #d32f2f;  // 에러 시 빨간색
}

.loadingBar.fadeOut {
	opacity: 0;
	transition: opacity 0.5s ease-out;
}

// 대화면 가독성을 위한 높이 증가 옵션
.loadingBarContainer.thick {
	height: 12px;
}
```

### JSX 렌더링 로직
```javascript
const LoadingBar = ({
	isLoading,
	isError,
	progress,
	onLoadComplete,
	className,
	barHeight,
	barColor,
	errorColor
}) => {
	// ... (상태 및 로직 생략)

	// 표시할 진행률 결정 (외부 progress 우선, 없으면 가상 진행률 사용)
	const displayProgress = progress !== null ? progress : virtualProgress

	// 렌더링하지 않을 조건
	if (!isLoading && displayProgress === 0) {
		return null
	}

	return (
		<div
			className={`${css.loadingBarContainer} ${className}`}
			style={{ height: `${barHeight}px` }}
		>
			<div
				className={`${css.loadingBar} ${isError ? css.error : ''} ${isFadingOut ? css.fadeOut : ''}`}
				style={{
					transform: `scaleX(${displayProgress / 100})`,
					backgroundColor: isError ? errorColor : barColor
				}}
			/>
		</div>
	)
}
```

## 7. 시퀀스 흐름

### 주요 시나리오: 정상 페이지 로딩
```
사용자    BrowserView    LoadingBar    WebView    requestAnimationFrame
  │            │              │            │                │
  │  URL 입력  │              │            │                │
  │───────────▶│              │            │                │
  │            │  setUrl()    │            │                │
  │            │─────────────────────────▶│                │
  │            │              │            │  onLoadStart() │
  │            │              │◀───────────│                │
  │            │  isLoading=true          │                │
  │            │─────────────▶│            │                │
  │            │              │  startTimeRef.current = now │
  │            │              │  animate() │                │
  │            │              │────────────────────────────▶│
  │            │              │            │  calculateVirtualProgress(0ms) = 0%
  │            │              │◀────────────────────────────│
  │            │              │  setVirtualProgress(0)      │
  │            │              │            │                │
  │            │              │  ... 16ms 후 ...            │
  │            │              │  animate() │                │
  │            │              │────────────────────────────▶│
  │            │              │            │  calculateVirtualProgress(16ms) = 0.3%
  │            │              │◀────────────────────────────│
  │            │              │  setVirtualProgress(0.3)    │
  │            │              │  CSS transform: scaleX(0.003)
  │            │              │            │                │
  │            │              │  ... 3초 후 ...             │
  │            │              │  animate() │                │
  │            │              │────────────────────────────▶│
  │            │              │            │  calculateVirtualProgress(3000ms) = 60%
  │            │              │◀────────────────────────────│
  │            │              │  setVirtualProgress(60)     │
  │            │              │  CSS transform: scaleX(0.6) │
  │            │              │            │                │
  │            │              │            │  ... 5초 후 (실제 로딩 완료) ...
  │            │              │            │  onLoadEnd()   │
  │            │              │◀───────────│                │
  │            │  isLoading=false         │                │
  │            │─────────────▶│            │                │
  │            │              │  cancelAnimationFrame()     │
  │            │              │  setVirtualProgress(100)    │
  │            │              │  CSS transform: scaleX(1.0) │
  │            │              │            │                │
  │            │              │  ... 0.1초 후 ...           │
  │            │              │  setIsFadingOut(true)       │
  │            │              │  CSS opacity: 0 (0.5초 전환)│
  │            │              │            │                │
  │            │              │  ... 0.5초 후 ...           │
  │            │              │  setVirtualProgress(0)      │
  │            │              │  onLoadComplete()           │
  │            │              │──────────▶                  │
```

### 에러 시나리오: 네트워크 에러
```
BrowserView    LoadingBar    WebView    requestAnimationFrame
  │                │            │                │
  │  setUrl()      │            │                │
  │───────────────────────────▶│                │
  │                │            │  onLoadStart() │
  │                │◀───────────│                │
  │  isLoading=true             │                │
  │───────────────▶│            │                │
  │                │  animate() 시작             │
  │                │────────────────────────────▶│
  │                │  60% 도달  │                │
  │                │            │                │
  │                │            │  onLoadError() │
  │                │◀───────────│                │
  │  isError=true  │            │                │
  │───────────────▶│            │                │
  │                │  cancelAnimationFrame()     │
  │                │  CSS: backgroundColor = red │
  │                │            │                │
  │                │  ... 1초 후 ...             │
  │                │  setVirtualProgress(0)      │
  │                │  페이드아웃                 │
```

## 8. BrowserView 통합

### BrowserView 수정 사항
```javascript
// src/views/BrowserView.js
import { useState } from 'react'
import { Panel, Header } from '@enact/moonstone/Panels'
import WebView from '../components/WebView'
import LoadingBar from '../components/LoadingBar'  // ← 새로 추가
import logger from '../utils/logger'

import css from './BrowserView.module.less'

const BrowserView = () => {
	const [currentUrl, setCurrentUrl] = useState('https://www.google.com')
	const [isLoading, setIsLoading] = useState(false)  // ← 새로 추가
	const [isError, setIsError] = useState(false)      // ← 새로 추가

	const handleLoadStart = () => {
		setIsLoading(true)
		setIsError(false)
		logger.info('[BrowserView] 페이지 로딩 시작:', currentUrl)
	}

	const handleLoadEnd = ({ url, title, duration }) => {
		setIsLoading(false)
		setIsError(false)
		logger.info('[BrowserView] 페이지 로딩 완료:', { url, title, duration })
	}

	const handleLoadError = ({ errorCode, errorMessage, url }) => {
		setIsLoading(false)
		setIsError(true)
		logger.error('[BrowserView] 페이지 로딩 실패:', { errorCode, errorMessage, url })
	}

	const handleLoadComplete = () => {
		logger.debug('[BrowserView] LoadingBar 페이드아웃 완료')
	}

	return (
		<Panel className={css.browserView}>
			<Header title="webOS Browser" />

			{/* URLBar 영역 (F-03에서 구현 예정) */}
			<div className={css.urlBarPlaceholder}>
				<span className={css.urlLabel}>URL:</span>
				<span className={css.urlText}>{currentUrl}</span>
			</div>

			{/* LoadingBar 영역 (새로 추가) */}
			<LoadingBar
				isLoading={isLoading}
				isError={isError}
				onLoadComplete={handleLoadComplete}
			/>

			{/* WebView 메인 영역 */}
			<div className={css.webviewWrapper}>
				<WebView
					url={currentUrl}
					onLoadStart={handleLoadStart}
					onLoadEnd={handleLoadEnd}
					onLoadError={handleLoadError}
				/>
			</div>

			{/* NavigationBar 영역 (F-04에서 구현 예정) */}
			<div className={css.navBarPlaceholder}>
				<span className={css.navButton}>[ 뒤로 ]</span>
				<span className={css.navButton}>[ 앞으로 ]</span>
				<span className={css.navButton}>[ 새로고침 ]</span>
				<span className={css.navButton}>[ 홈 ]</span>
			</div>
		</Panel>
	)
}

export default BrowserView
```

### BrowserView 스타일 업데이트
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
	flex-shrink: 0;  // URLBar 높이 고정
}

.urlLabel {
	margin-right: 12px;
	font-weight: bold;
}

.urlText {
	flex: 1;
	overflow: hidden;
	text-overflow: ellipsis;
	white-space: nowrap;
}

// LoadingBar는 별도 공간 차지하지 않음 (0px 높이, 절대 위치)
// → BrowserView.js에서 LoadingBar를 렌더링하되, CSS로 URLBar 아래에 오버레이로 배치

.webviewWrapper {
	flex: 1;  // 남은 공간 전체 차지
	overflow: hidden;
	position: relative;  // LoadingBar의 절대 위치 기준
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
	gap: 24px;
	flex-shrink: 0;  // NavigationBar 높이 고정
}

.navButton {
	cursor: pointer;
	padding: 8px 16px;
	background-color: rgba(255, 255, 255, 0.1);
	border-radius: 4px;
	transition: background-color 0.2s;

	&:hover {
		background-color: rgba(255, 255, 255, 0.2);
	}
}
```

## 9. 애니메이션 최적화

### GPU 가속 활성화
- **transform 속성 사용**: `scaleX()`는 GPU 레이어에서 처리되어 리페인트 발생 안 함
- **will-change 힌트**: CSS `will-change: transform`으로 브라우저에 최적화 힌트 제공
- **opacity 전환**: 페이드아웃 시 `opacity` 속성 사용 (GPU 가속)

### 피해야 할 속성
- ❌ `width`, `left`, `margin-left`: 리플로우 유발 (성능 저하)
- ❌ `background-position`: GPU 가속 안 됨
- ✅ `transform: scaleX()`: GPU 가속 (권장)
- ✅ `opacity`: GPU 가속 (권장)

### requestAnimationFrame 최적화
```javascript
// 불필요한 렌더링 방지: 진행률이 실제로 변경된 경우에만 setState 호출
const animate = () => {
	const elapsed = Date.now() - startTimeRef.current
	const newProgress = calculateVirtualProgress(elapsed)

	// 진행률이 0.5% 이상 변경된 경우에만 업데이트 (불필요한 렌더링 방지)
	if (Math.abs(newProgress - virtualProgress) > 0.5) {
		setVirtualProgress(newProgress)
	}

	if (newProgress < 95) {
		rafRef.current = requestAnimationFrame(animate)
	}
}
```

### 메모리 효율성
- **타이머 정리**: 컴포넌트 Unmount 시 requestAnimationFrame 취소
- **이벤트 리스너 없음**: 모든 상태 전이는 props 기반 (메모리 누수 없음)

## 10. 타임아웃 메시지 표시 (30초)

### LoadingSpinner에 타임아웃 경고 추가
```javascript
// src/components/WebView/LoadingSpinner.js
import { useState, useEffect } from 'react'
import Spinner from '@enact/moonstone/Spinner'
import css from './LoadingSpinner.module.less'

const LoadingSpinner = ({ url, isError, loadingStartTime }) => {
	const [isTimeout, setIsTimeout] = useState(false)

	// 30초 경과 감지
	useEffect(() => {
		if (!loadingStartTime) return

		const timer = setTimeout(() => {
			setIsTimeout(true)
		}, 30000)  // 30초

		return () => clearTimeout(timer)
	}, [loadingStartTime])

	if (isError) {
		return (
			<div className={css.spinnerOverlay}>
				<div className={css.errorIcon}>⚠️</div>
				<p className={css.loadingText}>페이지 로딩 실패</p>
			</div>
		)
	}

	return (
		<div className={css.spinnerOverlay}>
			<Spinner size="large" className={css.spinner} />
			<p className={css.loadingText}>
				{isTimeout ? '로딩이 오래 걸리고 있습니다. 네트워크를 확인해주세요.' : '페이지 로딩 중...'}
			</p>
			{url && (
				<p className={css.loadingUrl}>
					{url.length > 50 ? `${url.slice(0, 50)}...` : url}
				</p>
			)}
		</div>
	)
}

export default LoadingSpinner
```

### WebView에서 loadingStartTime 전달
```javascript
// src/components/WebView/WebView.js (수정)
// LoadingSpinner에 loadingStartTime prop 추가
{state === 'loading' && (
	<LoadingSpinner
		url={currentUrlRef.current}
		isError={false}
		loadingStartTime={loadStartTimeRef.current}
	/>
)}

{state === 'error' && error && (
	<LoadingSpinner
		url={currentUrlRef.current}
		isError={true}
		loadingStartTime={null}
	/>
)}
```

## 11. 영향 범위 분석

### 수정 필요한 기존 파일
1. **src/views/BrowserView.js**:
   - LoadingBar 컴포넌트 import 추가
   - isLoading, isError 상태 추가
   - handleLoadStart, handleLoadEnd, handleLoadError에서 상태 업데이트
   - JSX에 `<LoadingBar>` 추가 (URLBar 아래)
2. **src/views/BrowserView.module.less**:
   - LoadingBar 배치를 위한 스타일 조정 (URLBar 아래 오버레이)
3. **src/components/WebView/WebView.js**:
   - LoadingSpinner를 별도 컴포넌트로 분리 (현재 JSX에서 인라인 렌더링)
   - LoadingSpinner에 loadingStartTime prop 전달

### 새로 생성할 파일
1. **src/components/LoadingBar/LoadingBar.js**: 프로그레스바 컴포넌트
2. **src/components/LoadingBar/LoadingBar.module.less**: 프로그레스바 스타일
3. **src/components/LoadingBar/index.js**: Export 진입점
4. **src/components/WebView/LoadingSpinner.js**: 스피너 오버레이 컴포넌트 (WebView에서 분리)
5. **src/components/WebView/LoadingSpinner.module.less**: 스피너 스타일

### 영향 받는 기존 기능
- **F-02 (WebView)**: LoadingSpinner를 별도 컴포넌트로 분리하여 재사용성 향상 (기능 변경 없음)
- **F-03 (URLBar)**: LoadingBar가 URLBar 아래에 배치되므로 레이아웃 조정 필요 (F-03 구현 시 고려)
- **F-04 (페이지 탐색)**: 뒤로/앞으로 버튼 클릭 시에도 LoadingBar 동작 (이벤트 기반이므로 자동 동작)

## 12. 기술적 주의사항

### iframe 진행률 제약
- **문제**: HTML iframe은 실제 로딩 진행률(loadprogress 이벤트) 제공 안 함
- **대응**: 가상 진행률 알고리즘으로 사용자에게 진행 상태 체감 제공
- **한계**: 실제 로딩 완료 시점과 가상 진행률이 불일치할 수 있음 (95%에서 대기 후 완료 이벤트 시 100% 이동)

### requestAnimationFrame 호환성
- **webOS 4.x Chromium 53**: requestAnimationFrame 지원 확인됨
- **폴백**: 만약 지원되지 않을 경우 setInterval로 폴백 (코드에 조건 추가)
  ```javascript
  if (typeof requestAnimationFrame === 'undefined') {
  	// setInterval 기반 애니메이션 로직
  }
  ```

### CSS transform 성능
- **webOS 프로젝터 제약**: 하드웨어 성능이 제한적이므로 복잡한 애니메이션 회피
- **최적화**: `transform: scaleX()`만 사용하여 GPU 가속 최대 활용
- **측정**: 개발 모드에서 `performance.now()`로 프레임 시간 측정, 33ms(30fps) 이하 유지 확인

### 색맹 사용자 접근성
- **문제**: 프로그레스바 색상만으로 에러 구분 시 색맹 사용자 인지 어려움
- **대응**: 색상 + 에러 아이콘(⚠️) 조합으로 다중 시각적 단서 제공
- **추가 고려**: 향후 F-11(설정)에서 고대비 테마 제공 시 에러 색상 대비 강화

### 모션 감도
- **PRD 요구사항**: 모션 저감 모드 옵션 제공 고려 (F-11에서 구현 예정)
- **현재 단계**: 기본 애니메이션 구현, 모션 감도 옵션은 F-11에서 추가
- **구현 방향**: F-11에서 `prefersReducedMotion` 설정 추가 → LoadingBar에서 애니메이션 비활성화 처리

### 탭 관리 연동 (F-06)
- **현재**: BrowserView 레벨에서 단일 LoadingBar 관리
- **F-06 시**: 탭별 로딩 상태 관리 필요
- **준비**:
  - LoadingBar는 독립 컴포넌트로 설계되어 재사용 가능
  - 활성 탭의 isLoading, isError 상태를 LoadingBar에 전달
  - 백그라운드 탭 로딩 시 LoadingBar 숨김 처리 (활성 탭만 표시)

## 13. 테스트 시나리오

### 단위 테스트 (Jest)
1. **LoadingBar 컴포넌트**:
   - 가상 진행률 계산 함수 테스트 (0초, 3초, 10초 입력 시 예상 진행률)
   - isLoading=true 시 프로그레스바 렌더링 확인
   - isLoading=false 시 100% 이동 후 페이드아웃 확인
   - isError=true 시 빨간색 변경 확인
2. **LoadingSpinner 컴포넌트**:
   - 30초 경과 시 타임아웃 메시지 표시 확인
   - isError=true 시 에러 아이콘 표시 확인

### 통합 테스트 (BrowserView)
1. WebView 로딩 시작 → LoadingBar 표시 확인
2. WebView 로딩 완료 → LoadingBar 100% 도달 후 사라짐 확인
3. WebView 로딩 실패 → LoadingBar 빨간색 변경 확인

### E2E 테스트 (실제 디바이스)
1. 빠른 페이지 로딩 (1초 이내): 프로그레스바가 빠르게 100% 도달
2. 느린 페이지 로딩 (10초): 프로그레스바가 90%까지 증가 후 대기
3. 매우 느린 페이지 로딩 (30초 초과): 타임아웃 메시지 표시
4. 페이지 로딩 실패 (404): 프로그레스바 빨간색 + 에러 아이콘
5. 연속 로딩 (URL 빠르게 여러 번 변경): 이전 애니메이션 취소 및 새 로딩 시작

### 성능 테스트
1. requestAnimationFrame 프레임레이트 측정 (30fps 이상 유지 확인)
2. 메모리 사용량 측정 (LoadingBar 추가로 5MB 이하 증가 확인)
3. CPU 사용률 측정 (애니메이션 중 10% 이하 확인)

## 14. 구현 순서

### Phase 1: LoadingBar 컴포넌트 기본 구조
1. `src/components/LoadingBar/` 디렉토리 생성
2. `LoadingBar.js` 생성 (Props 인터페이스, PropTypes 정의)
3. `LoadingBar.module.less` 생성 (기본 스타일)
4. `index.js` 생성 (export default)

### Phase 2: 가상 진행률 알고리즘 구현
1. `calculateVirtualProgress()` 함수 구현
2. requestAnimationFrame 기반 애니메이션 루프 구현
3. useEffect로 isLoading 감지 및 애니메이션 시작/중단
4. 95% 도달 시 애니메이션 중단 로직 추가

### Phase 3: 로딩 완료 및 페이드아웃 처리
1. isLoading=false 감지 → 100% 즉시 이동
2. 0.1초 후 페이드아웃 시작
3. 0.5초 후 onLoadComplete 콜백 호출
4. CSS opacity 전환 애니메이션 추가

### Phase 4: 에러 처리
1. isError=true 시 애니메이션 중단
2. 프로그레스바 색상 빨간색 변경 (CSS 클래스 추가)
3. 1초 후 페이드아웃

### Phase 5: LoadingSpinner 분리 (WebView 내부)
1. `src/components/WebView/LoadingSpinner.js` 생성
2. WebView.js의 인라인 스피너 JSX를 LoadingSpinner 컴포넌트로 이동
3. Enact Spinner import 및 사용
4. 30초 타임아웃 메시지 로직 추가
5. 에러 아이콘 표시 로직 추가

### Phase 6: BrowserView 통합
1. `src/views/BrowserView.js`에 LoadingBar import
2. isLoading, isError 상태 추가
3. handleLoadStart, handleLoadEnd, handleLoadError에서 상태 업데이트
4. JSX에 `<LoadingBar>` 추가 (URLBar 아래)
5. 레이아웃 스타일 조정 (BrowserView.module.less)

### Phase 7: 애니메이션 최적화
1. CSS `will-change: transform` 추가
2. 불필요한 렌더링 방지 로직 추가 (0.5% 이상 변경 시에만 업데이트)
3. GPU 가속 확인 (Chrome DevTools Performance 탭)

### Phase 8: 로컬 테스트
1. `npm run serve` 실행
2. 브라우저에서 프로그레스바 표시 확인
3. 가상 진행률 증가 확인 (개발자 도구에서 진행률 로깅)
4. 로딩 완료 시 100% 이동 및 페이드아웃 확인
5. 에러 시 빨간색 변경 확인

### Phase 9: 실제 디바이스 테스트
1. `npm run pack-p` 빌드
2. `ares-package dist/` IPK 생성
3. 프로젝터 설치 및 실행
4. 대화면(100인치)에서 프로그레스바 시인성 확인 (3m 거리)
5. 주요 사이트 로딩 시 정상 동작 확인 (Google, YouTube)
6. 애니메이션 프레임레이트 확인 (부드러운 움직임)

### Phase 10: 성능 측정 및 최적화
1. requestAnimationFrame 프레임 시간 로깅 (33ms 이하 확인)
2. 메모리 사용량 측정 (5MB 이하 확인)
3. 느린 페이지 로딩 시 90% 대기 동작 확인
4. 성능 이슈 발견 시 애니메이션 간격 조정

## 15. 확장성 고려사항

### F-06 (탭 관리)와의 연동
- **현재**: 단일 WebView → 단일 LoadingBar
- **F-06 시**: 다중 탭 → 활성 탭의 로딩 상태만 LoadingBar에 표시
- **준비**:
  - LoadingBar는 독립 컴포넌트로 설계되어 재사용 가능
  - BrowserView에서 활성 탭의 isLoading, isError를 LoadingBar에 전달
  - 백그라운드 탭 로딩 시 탭 UI에 작은 로딩 아이콘만 표시 (LoadingBar 숨김)

### F-11 (설정)과의 연동
- **모션 저감 모드**: 설정에서 애니메이션 비활성화 옵션 제공
- **구현 방향**:
  - F-11에서 `prefersReducedMotion` boolean 설정 추가
  - LoadingBar에 `disableAnimation` prop 추가
  - disableAnimation=true 시 requestAnimationFrame 사용 안 함, 즉시 100% 표시
- **고대비 테마**: 에러 색상을 더 강한 대비로 변경 (색맹 사용자 대응)

### 실제 진행률 지원 (향후)
- **현재**: 가상 진행률만 지원 (iframe 제약)
- **향후**: postMessage API로 iframe 내부와 통신하여 실제 리소스 로딩 진행률 수집 가능
- **구현 방향**:
  - LoadingBar에 `progress` prop 추가 (이미 설계에 포함)
  - progress가 null이면 가상 진행률 사용, 숫자이면 실제 진행률 사용
  - iframe 내부에 진행률 추적 스크립트 주입 (Same-Origin일 때만 가능)

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-12 | 최초 작성 | F-05 요구사항 기반 기술 설계 |
