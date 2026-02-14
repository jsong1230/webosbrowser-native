# F-14: HTTPS 보안 표시 — 기술 설계서

## 1. 참조
- 요구사항 분석서: docs/specs/https-security-indicator/requirements.md

---

## 2. 아키텍처 개요

### 2.1. 컴포넌트 구조

```
BrowserView
│
├── URLBar (확장)
│   └── SecurityIndicator (신규) ← 보안 아이콘 표시
│
└── SecurityWarningDialog (신규) ← HTTP 경고 다이얼로그
```

### 2.2. 데이터 흐름

```
WebView (onNavigationChange)
    ↓ URL 변경 감지 (500ms 폴링)
BrowserView (handleNavigationChange)
    ↓ URL 전달
    ├─→ URLBar (value prop)
    │       ↓ securityClassifier.classifyUrl(url)
    │   SecurityIndicator
    │       ↓ 보안 등급별 아이콘 표시
    │
    └─→ SecurityWarningDialog
            ↓ HTTP URL 감지 (디바운스 500ms)
            ↓ 경고 무시 도메인 체크 (SessionStorage)
        HTTP 경고 다이얼로그 표시
```

### 2.3. 상태 관리

| 상태 | 위치 | 타입 | 설명 |
|------|------|------|------|
| `securityLevel` | URLBar (local state) | String | 현재 URL의 보안 등급 (`secure`, `insecure`, `local`, `unknown`) |
| `showSecurityWarning` | BrowserView (local state) | Boolean | 경고 다이얼로그 표시 여부 |
| `pendingUrl` | BrowserView (local state) | String | 경고 대기 중인 URL |
| `ignoredDomains` | SessionStorage | Set | 경고 무시 도메인 목록 (세션 단위) |

---

## 3. 아키텍처 결정

### 결정 1: 보안 등급 분류 방식
- **선택지**:
  - A) URL 프로토콜 기반 분류 (https://, http://)
  - B) webOS iframe postMessage로 인증서 정보 요청
  - C) 외부 SSL 검증 서비스 API 호출
- **결정**: A) URL 프로토콜 기반 분류
- **근거**:
  - webOS iframe은 CORS 제약으로 인증서 정보에 접근 불가 (Same-Origin Policy)
  - 외부 API 호출은 네트워크 의존성과 성능 문제 발생
  - URL 프로토콜 기반 분류는 빠르고 안정적이며, 대부분의 브라우저가 사용하는 방식
- **트레이드오프**:
  - 포기: 인증서 상세 정보 표시 (유효 기간, 인증 기관 등), Mixed Content 감지
  - 얻음: 빠른 반응 속도 (0ms), CORS 제약 회피, 네트워크 독립적

### 결정 2: 경고 다이얼로그 표시 타이밍
- **선택지**:
  - A) URL 변경 즉시 표시
  - B) 500ms 디바운스 후 표시
  - C) 페이지 로딩 완료 후 표시
- **결정**: B) 500ms 디바운스 후 표시
- **근거**:
  - WebView는 500ms 폴링으로 URL 변경 감지 → 빠른 리다이렉트 시 중간 URL 감지 가능
  - HTTP → HTTPS 자동 리다이렉트 시 불필요한 경고 방지
  - 페이지 로딩 완료까지 대기하면 사용자가 이미 폼 입력 시작할 수 있음
- **트레이드오프**:
  - 포기: 즉각적인 경고 표시 (최대 500ms 지연)
  - 얻음: 자동 리다이렉트 시 경고 숨김, 불필요한 경고 감소

### 결정 3: 경고 무시 도메인 저장 위치
- **선택지**:
  - A) SessionStorage (메모리 기반, 세션 단위)
  - B) LocalStorage (영구 저장)
  - C) LS2 API (webOS 로컬 DB, 영구 저장)
- **결정**: A) SessionStorage (메모리 기반)
- **근거**:
  - 보안 경고는 사용자 안전을 위해 앱 재시작 시 초기화되는 것이 바람직
  - 영구 저장 시 사용자가 설정을 잊고 계속 HTTP 사이트를 사용할 위험
  - SessionStorage는 빠르고 간단하며, webOS에서 기본 지원
- **트레이드오프**:
  - 포기: 앱 재시작 후 무시 설정 유지
  - 얻음: 보안성 향상, 사용자가 매번 경고를 재확인하여 안전 의식 강화

### 결정 4: SecurityIndicator 위치
- **선택지**:
  - A) URLBar 내부 좌측 아이콘 영역 (독립 컴포넌트)
  - B) URLBar Input 컴포넌트의 iconBefore prop 활용
  - C) URLBar 외부 독립 영역
- **결정**: A) URLBar 내부 좌측 아이콘 영역 (독립 컴포넌트)
- **근거**:
  - Enact Input의 iconBefore prop은 Spotlight 포커스 제어가 제한적
  - 독립 컴포넌트로 구현하면 툴팁, 클릭 이벤트 등 확장성 확보
  - 일반 브라우저(Chrome, Firefox)의 UX 패턴과 일치
- **트레이드오프**:
  - 포기: Enact Input의 내장 아이콘 기능
  - 얻음: Spotlight 포커스 제어, 툴팁 표시, 클릭 이벤트 처리, 향후 상세 정보 다이얼로그 확장 용이

---

## 4. 컴포넌트 설계

### 4.1. SecurityIndicator 컴포넌트

**위치**: `src/components/SecurityIndicator/SecurityIndicator.js`

**Props 인터페이스**:
```javascript
SecurityIndicator.propTypes = {
	// 보안 등급
	securityLevel: PropTypes.oneOf(['secure', 'insecure', 'local', 'unknown']).isRequired,

	// 현재 URL (툴팁 표시용)
	url: PropTypes.string,

	// 클릭 이벤트 핸들러 (FR-5 상세 정보 다이얼로그용, 선택적)
	onClick: PropTypes.func,

	// 스타일 커스터마이징
	className: PropTypes.string
}
```

**보안 등급별 UI 매핑**:
| 등급 | 아이콘 | 색상 | 툴팁 |
|------|--------|------|------|
| `secure` | 🔒 (lock) | Green (#4CAF50) | "보안 연결 (HTTPS)" |
| `insecure` | ⚠ (warning) | Orange (#FF9800) | "안전하지 않음 (HTTP)" |
| `local` | ℹ️ (info) | Blue (#2196F3) | "로컬 연결" |
| `unknown` | (아이콘 없음) | - | - |

**Spotlight 동작**:
- Spotlightable 컴포넌트로 구현
- 리모컨 방향키로 포커스 가능
- 포커스 시 툴팁 자동 표시 (Moonstone Tooltip)
- 선택 키(Enter) 클릭 시 상세 정보 다이얼로그 표시 (FR-5, 선택적 구현)

**렌더링 조건**:
- `securityLevel === 'unknown'` → 아이콘 표시하지 않음
- 나머지 등급 → 아이콘 + 툴팁 표시

**CSS 구조** (`SecurityIndicator.module.less`):
```less
.securityIndicator {
	display: inline-flex;
	align-items: center;
	justify-content: center;
	width: 48px;  // 고정 너비 (레이아웃 시프트 방지)
	height: 48px;
	cursor: pointer;

	.icon {
		font-size: 32px;  // 대화면 가독성

		&.secure {
			color: #4CAF50;  // Green
		}

		&.insecure {
			color: #FF9800;  // Orange
		}

		&.local {
			color: #2196F3;  // Blue
		}
	}

	// Spotlight 포커스 스타일
	&:focus {
		outline: 2px solid var(--moonstone-spotlight-color);
		outline-offset: 2px;
	}
}
```

---

### 4.2. SecurityWarningDialog 컴포넌트

**위치**: `src/components/SecurityWarningDialog/SecurityWarningDialog.js`

**Props 인터페이스**:
```javascript
SecurityWarningDialog.propTypes = {
	// 다이얼로그 표시 여부
	visible: PropTypes.bool.isRequired,

	// 경고 대상 URL
	url: PropTypes.string.isRequired,

	// 확인 버튼 콜백 (로딩 계속)
	onContinue: PropTypes.func.isRequired,

	// 돌아가기 버튼 콜백 (history.back)
	onGoBack: PropTypes.func.isRequired,

	// 무시 옵션 체크 콜백
	onDontShowAgain: PropTypes.func.isRequired
}
```

**UI 구조**:
```
[다이얼로그]
┌─────────────────────────────────────────┐
│ 안전하지 않은 사이트                      │
├─────────────────────────────────────────┤
│ ⚠ 이 사이트는 보안 연결을 사용하지 않습니다│
│                                         │
│ URL: http://example.com                 │
│                                         │
│ 개인 정보(비밀번호, 신용카드 번호 등)를   │
│ 입력하지 마세요.                         │
│                                         │
│ □ 오늘 이 사이트에 대해 다시 표시하지    │
│   않기                                  │
├─────────────────────────────────────────┤
│ [계속 진행] [돌아가기]                   │
└─────────────────────────────────────────┘
```

**Spotlight 포커스 순서**:
1. "오늘 이 사이트에 대해 다시 표시하지 않기" 체크박스
2. "계속 진행" 버튼 (기본 포커스)
3. "돌아가기" 버튼

**CSS 구조** (`SecurityWarningDialog.module.less`):
```less
.securityWarningDialog {
	.content {
		padding: 24px;

		.warningIcon {
			font-size: 48px;
			color: #FF9800;  // Orange
			margin-bottom: 16px;
		}

		.url {
			font-family: monospace;
			color: var(--moonstone-text-secondary);
			margin-bottom: 16px;
		}

		.message {
			font-size: 18px;
			line-height: 1.5;
			margin-bottom: 24px;
		}

		.checkbox {
			display: flex;
			align-items: center;
			margin-top: 16px;
		}
	}
}
```

---

### 4.3. securityClassifier 유틸리티

**위치**: `src/utils/securityClassifier.js`

**함수 인터페이스**:
```javascript
/**
 * URL 프로토콜 기반 보안 등급 분류
 *
 * @param {string} url - 분류할 URL
 * @returns {'secure'|'insecure'|'local'|'unknown'}
 */
export const classifyUrl = (url) => {
	// null, undefined, 빈 문자열 체크
	if (!url || typeof url !== 'string' || url.trim().length === 0) {
		return 'unknown'
	}

	try {
		const urlObj = new URL(url)
		const protocol = urlObj.protocol
		const hostname = urlObj.hostname

		// HTTPS → secure
		if (protocol === 'https:') {
			return 'secure'
		}

		// HTTP + localhost → local
		if (protocol === 'http:' && isLocalHost(hostname)) {
			return 'local'
		}

		// HTTP + 외부 도메인 → insecure
		if (protocol === 'http:') {
			return 'insecure'
		}

		// file:// → local
		if (protocol === 'file:') {
			return 'local'
		}

		// about:, data: 등 → unknown
		return 'unknown'
	} catch (error) {
		// URL 파싱 실패 → unknown
		logger.warn('[securityClassifier] URL 파싱 실패:', error.message)
		return 'unknown'
	}
}

/**
 * localhost 여부 판단
 *
 * @param {string} hostname
 * @returns {boolean}
 */
const isLocalHost = (hostname) => {
	// localhost, 127.0.0.1, ::1
	if (hostname === 'localhost' || hostname === '127.0.0.1' || hostname === '[::1]') {
		return true
	}

	// 192.168.*.*, 10.*.*.*, 172.16-31.*.*
	if (/^192\.168\.\d{1,3}\.\d{1,3}$/.test(hostname)) {
		return true
	}
	if (/^10\.\d{1,3}\.\d{1,3}\.\d{1,3}$/.test(hostname)) {
		return true
	}
	if (/^172\.(1[6-9]|2[0-9]|3[0-1])\.\d{1,3}\.\d{1,3}$/.test(hostname)) {
		return true
	}

	return false
}

/**
 * 도메인 추출 (경고 무시 목록 관리용)
 *
 * @param {string} url
 * @returns {string|null} - 도메인 (예: example.com) 또는 null
 */
export const extractDomain = (url) => {
	try {
		const urlObj = new URL(url)
		return urlObj.hostname
	} catch (error) {
		logger.warn('[securityClassifier] 도메인 추출 실패:', error.message)
		return null
	}
}
```

---

## 5. URLBar 확장 방안

### 5.1. URLBar 컴포넌트 수정

**현재 구조** (line 159-171):
```javascript
<div className={`${css.urlBar} ${className}`}>
	<Input
		ref={inputRef}
		value={inputValue}
		onChange={(event) => handleChange(event.value)}
		// ...
	/>
	{/* ... */}
</div>
```

**확장 구조**:
```javascript
<div className={`${css.urlBar} ${className}`}>
	{/* 보안 아이콘 (좌측) */}
	<SecurityIndicator
		securityLevel={securityLevel}
		url={value}
		onClick={handleSecurityIconClick}
		className={css.securityIndicator}
	/>

	{/* URL 입력 필드 */}
	<Input
		ref={inputRef}
		value={inputValue}
		onChange={(event) => handleChange(event.value)}
		// ...
		className={css.input}
	/>

	{/* 기존 자동완성 드롭다운, 가상 키보드 */}
	{/* ... */}
</div>
```

### 5.2. URLBar 상태 추가

**신규 상태**:
```javascript
const [securityLevel, setSecurityLevel] = useState('unknown')
```

**useEffect로 URL 변경 감지**:
```javascript
useEffect(() => {
	// value prop (현재 URL) 변경 시 보안 등급 재계산
	const level = classifyUrl(value)
	setSecurityLevel(level)
}, [value])
```

### 5.3. CSS 레이아웃 조정

**URLBar.module.less**:
```less
.urlBar {
	display: flex;
	align-items: center;
	gap: 8px;  // 아이콘과 Input 사이 간격

	.securityIndicator {
		flex-shrink: 0;  // 고정 너비 유지
	}

	.input {
		flex: 1;  // 남은 공간 차지
	}
}
```

---

## 6. BrowserView 상태 관리

### 6.1. 신규 상태

```javascript
// 경고 다이얼로그 상태
const [showSecurityWarning, setShowSecurityWarning] = useState(false)
const [pendingUrl, setPendingUrl] = useState(null)

// 경고 디바운스 타이머
const securityWarningTimerRef = useRef(null)
```

### 6.2. handleNavigationChange 확장

**현재 코드** (BrowserView.js line 307-328):
```javascript
const handleNavigationChange = useCallback(({ url }) => {
	logger.info('[BrowserView] URL 변경:', url)
	// 히스토리 스택 업데이트...
}, [activeTab, activeTabId, dispatch])
```

**확장 코드**:
```javascript
const handleNavigationChange = useCallback(({ url }) => {
	logger.info('[BrowserView] URL 변경:', url)

	// 기존 히스토리 스택 업데이트 로직...

	// 보안 경고 체크 (500ms 디바운스)
	if (securityWarningTimerRef.current) {
		clearTimeout(securityWarningTimerRef.current)
	}

	securityWarningTimerRef.current = setTimeout(() => {
		checkSecurityWarning(url)
	}, 500)
}, [activeTab, activeTabId, dispatch])

/**
 * 보안 경고 체크
 */
const checkSecurityWarning = useCallback((url) => {
	const level = classifyUrl(url)

	// HTTP 사이트이고, 경고 무시 목록에 없으면 경고 표시
	if (level === 'insecure') {
		const domain = extractDomain(url)
		const ignoredDomains = getIgnoredDomains()

		if (!ignoredDomains.has(domain)) {
			setPendingUrl(url)
			setShowSecurityWarning(true)
			logger.info('[BrowserView] HTTP 경고 다이얼로그 표시:', url)
		}
	}
}, [])
```

### 6.3. SecurityWarningDialog 이벤트 핸들러

```javascript
/**
 * 계속 진행 버튼
 */
const handleSecurityContinue = useCallback(() => {
	setShowSecurityWarning(false)
	setPendingUrl(null)
	logger.info('[BrowserView] HTTP 경고 무시 - 계속 진행')
}, [])

/**
 * 돌아가기 버튼
 */
const handleSecurityGoBack = useCallback(() => {
	setShowSecurityWarning(false)
	setPendingUrl(null)

	// WebView history.back() 호출
	if (webviewRef.current) {
		try {
			webviewRef.current.goBack()
			logger.info('[BrowserView] HTTP 경고 - 뒤로 가기')
		} catch (error) {
			logger.error('[BrowserView] 뒤로 가기 실패:', error)
		}
	}
}, [])

/**
 * 무시 옵션 체크
 */
const handleSecurityDontShowAgain = useCallback((checked) => {
	if (checked && pendingUrl) {
		const domain = extractDomain(pendingUrl)
		addIgnoredDomain(domain)
		logger.info('[BrowserView] 경고 무시 도메인 추가:', domain)
	}
}, [pendingUrl])
```

### 6.4. JSX 렌더링 추가

**BrowserView.js** (line 815 부근에 추가):
```javascript
{/* SecurityWarningDialog (F-14 구현) */}
{showSecurityWarning && (
	<SecurityWarningDialog
		visible={showSecurityWarning}
		url={pendingUrl}
		onContinue={handleSecurityContinue}
		onGoBack={handleSecurityGoBack}
		onDontShowAgain={handleSecurityDontShowAgain}
	/>
)}
```

---

## 7. SessionStorage 관리

### 7.1. ignoredDomains 관리 함수

**위치**: `src/utils/securityStorage.js` (신규 파일)

```javascript
/**
 * 경고 무시 도메인 목록 관리 (SessionStorage)
 */

const STORAGE_KEY = 'ignoredSecurityDomains'

/**
 * 경고 무시 도메인 목록 조회
 *
 * @returns {Set<string>}
 */
export const getIgnoredDomains = () => {
	try {
		const json = sessionStorage.getItem(STORAGE_KEY)
		if (!json) {
			return new Set()
		}

		const array = JSON.parse(json)
		return new Set(array)
	} catch (error) {
		logger.error('[securityStorage] 도메인 목록 로드 실패:', error)
		return new Set()
	}
}

/**
 * 경고 무시 도메인 추가
 *
 * @param {string} domain
 */
export const addIgnoredDomain = (domain) => {
	try {
		const domains = getIgnoredDomains()
		domains.add(domain)

		// Set → Array → JSON
		const array = Array.from(domains)
		sessionStorage.setItem(STORAGE_KEY, JSON.stringify(array))

		logger.info('[securityStorage] 도메인 추가:', domain)
	} catch (error) {
		logger.error('[securityStorage] 도메인 추가 실패:', error)
	}
}

/**
 * 경고 무시 도메인 제거
 *
 * @param {string} domain
 */
export const removeIgnoredDomain = (domain) => {
	try {
		const domains = getIgnoredDomains()
		domains.delete(domain)

		const array = Array.from(domains)
		sessionStorage.setItem(STORAGE_KEY, JSON.stringify(array))

		logger.info('[securityStorage] 도메인 제거:', domain)
	} catch (error) {
		logger.error('[securityStorage] 도메인 제거 실패:', error)
	}
}

/**
 * 경고 무시 도메인 목록 초기화
 */
export const clearIgnoredDomains = () => {
	try {
		sessionStorage.removeItem(STORAGE_KEY)
		logger.info('[securityStorage] 도메인 목록 초기화')
	} catch (error) {
		logger.error('[securityStorage] 도메인 목록 초기화 실패:', error)
	}
}
```

---

## 8. 시퀀스 흐름

### 8.1. 주요 시나리오: HTTPS 사이트 접속

```
사용자 → URLBar → BrowserView → WebView → SecurityIndicator
  │                    │              │              │
  │  URLBar 입력       │              │              │
  │  "https://..."     │              │              │
  │──────────────────▶│              │              │
  │                    │  URL 업데이트│              │
  │                    │──────────────▶              │
  │                    │              │  onNavigationChange(url)
  │                    │              │──────────────▶
  │                    │◀─────────────│              │
  │                    │  classifyUrl(url) → 'secure'│
  │                    │────────────────────────────▶│
  │                    │              │              │  녹색 자물쇠 표시
  │◀───────────────────────────────────────────────│
```

### 8.2. 에러 시나리오: HTTP 사이트 접속 (첫 방문)

```
사용자 → URLBar → BrowserView → WebView → SecurityWarningDialog
  │                    │              │              │
  │  URLBar 입력       │              │              │
  │  "http://..."      │              │              │
  │──────────────────▶│              │              │
  │                    │  URL 업데이트│              │
  │                    │──────────────▶              │
  │                    │              │  onNavigationChange(url)
  │                    │              │──────────────▶
  │                    │◀─────────────│              │
  │                    │  500ms 디바운스│             │
  │                    │  checkSecurityWarning(url)  │
  │                    │  classifyUrl(url) → 'insecure'
  │                    │  getIgnoredDomains() → {}   │
  │                    │  setShowSecurityWarning(true)
  │                    │───────────────────────────▶│
  │                    │              │              │  경고 다이얼로그 표시
  │◀──────────────────────────────────────────────│
  │                    │              │              │
  │  "계속 진행" 클릭  │              │              │
  │──────────────────────────────────────────────▶│
  │                    │  onContinue()│              │
  │                    │◀──────────────────────────│
  │                    │  setShowSecurityWarning(false)
  │                    │              │              │
  │                    │  URLBar: 경고 아이콘 표시   │
```

### 8.3. 에러 시나리오: HTTP → HTTPS 자동 리다이렉트

```
사용자 → URLBar → BrowserView → WebView → SecurityWarningDialog
  │                    │              │              │
  │  URLBar 입력       │              │              │
  │  "http://github.com"│             │              │
  │──────────────────▶│              │              │
  │                    │  URL 업데이트│              │
  │                    │──────────────▶              │
  │                    │              │  onNavigationChange("http://github.com")
  │                    │              │──────────────▶
  │                    │◀─────────────│              │
  │                    │  500ms 디바운스│             │
  │                    │  (타이머 시작) │             │
  │                    │              │  onNavigationChange("https://github.com")
  │                    │              │──────────────▶ (리다이렉트)
  │                    │◀─────────────│              │
  │                    │  500ms 디바운스│             │
  │                    │  (타이머 취소 + 재시작)     │
  │                    │  checkSecurityWarning("https://github.com")
  │                    │  classifyUrl(url) → 'secure' │
  │                    │  (경고 표시하지 않음)        │
  │                    │              │              │
  │                    │  URLBar: 녹색 자물쇠 표시    │
```

---

## 9. 영향 범위 분석

### 9.1. 신규 생성 파일

| 파일 | 역할 |
|------|------|
| `src/components/SecurityIndicator/SecurityIndicator.js` | 보안 아이콘 컴포넌트 |
| `src/components/SecurityIndicator/SecurityIndicator.module.less` | 보안 아이콘 스타일 |
| `src/components/SecurityWarningDialog/SecurityWarningDialog.js` | HTTP 경고 다이얼로그 |
| `src/components/SecurityWarningDialog/SecurityWarningDialog.module.less` | 경고 다이얼로그 스타일 |
| `src/utils/securityClassifier.js` | URL 보안 등급 분류 유틸리티 |
| `src/utils/securityStorage.js` | 경고 무시 도메인 관리 (SessionStorage) |

### 9.2. 수정 필요 파일

| 파일 | 변경 내용 | 라인 |
|------|-----------|------|
| `src/components/URLBar/URLBar.js` | SecurityIndicator 통합, securityLevel 상태 추가 | 11, 159-171 |
| `src/components/URLBar/URLBar.module.less` | 좌측 아이콘 영역 레이아웃 조정 | - |
| `src/views/BrowserView.js` | SecurityWarningDialog 상태 관리, handleNavigationChange 확장 | 307-328, 815 |

### 9.3. 영향 받는 기존 기능

| 기능 | 영향 내용 | 대응 방안 |
|------|-----------|-----------|
| **F-03 (URL 입력 UI)** | URLBar 레이아웃 변경 (좌측 아이콘 추가) | CSS flexbox로 레이아웃 조정, 기존 Input 너비는 유지 |
| **F-02 (웹뷰 통합)** | onNavigationChange 콜백 확장 | 기존 로직 유지, 보안 체크 로직 추가 (디바운스) |
| **F-04 (네비게이션 바)** | "돌아가기" 버튼 동작 확장 | SecurityWarningDialog에서 webviewRef.goBack() 호출 |
| **F-05 (로딩 인디케이터)** | 없음 | 영향 없음 (독립적 동작) |

---

## 10. 에러 처리 및 엣지 케이스

### 10.1. URL 파싱 실패

| 상황 | 처리 방법 |
|------|-----------|
| URL이 null/undefined | `classifyUrl()` → `'unknown'` 반환, 아이콘 표시하지 않음 |
| URL 형식이 잘못됨 | `new URL()` 예외 catch → `'unknown'` 반환 |
| about:blank | `classifyUrl()` → `'unknown'` 반환 |

### 10.2. SessionStorage 실패

| 상황 | 처리 방법 |
|------|-----------|
| SessionStorage 접근 불가 | try-catch로 예외 처리, 빈 Set 반환 |
| JSON 파싱 실패 | catch 블록에서 빈 Set 반환 |
| 저장 용량 초과 (5MB) | catch 블록에서 에러 로깅, 기능은 계속 동작 |

### 10.3. 빠른 리다이렉트

| 상황 | 처리 방법 |
|------|-----------|
| HTTP → HTTPS 리다이렉트 | 500ms 디바운스로 타이머 취소/재시작, 최종 HTTPS URL만 체크 |
| 연속 리다이렉트 (3번 이상) | 디바운스가 자동으로 처리, 최종 URL만 체크 |

### 10.4. 동시 다이얼로그 표시

| 상황 | 처리 방법 |
|------|-----------|
| SecurityWarningDialog + DownloadConfirmDialog | SecurityWarningDialog가 우선 표시 (보안 경고가 더 중요) |
| SecurityWarningDialog + ErrorPage | ErrorPage가 WebView 영역에 표시되므로 충돌 없음 |

### 10.5. localhost 예외 처리

| 상황 | 처리 방법 |
|------|-----------|
| http://localhost | `isLocalHost()` → `true` → `'local'` 등급, 경고 표시하지 않음 |
| http://127.0.0.1 | `isLocalHost()` → `true` → `'local'` 등급 |
| http://192.168.1.1 | `isLocalHost()` → `true` → `'local'` 등급 (사설 IP) |
| http://10.0.0.1 | `isLocalHost()` → `true` → `'local'` 등급 |

---

## 11. 테스트 전략

### 11.1. 단위 테스트

**securityClassifier.js**:
```javascript
describe('securityClassifier', () => {
	test('HTTPS URL → secure', () => {
		expect(classifyUrl('https://example.com')).toBe('secure')
	})

	test('HTTP URL → insecure', () => {
		expect(classifyUrl('http://example.com')).toBe('insecure')
	})

	test('HTTP localhost → local', () => {
		expect(classifyUrl('http://localhost')).toBe('local')
		expect(classifyUrl('http://127.0.0.1')).toBe('local')
		expect(classifyUrl('http://192.168.1.1')).toBe('local')
	})

	test('file:// → local', () => {
		expect(classifyUrl('file:///path/to/file.html')).toBe('local')
	})

	test('about:blank → unknown', () => {
		expect(classifyUrl('about:blank')).toBe('unknown')
	})

	test('null/undefined → unknown', () => {
		expect(classifyUrl(null)).toBe('unknown')
		expect(classifyUrl(undefined)).toBe('unknown')
		expect(classifyUrl('')).toBe('unknown')
	})

	test('도메인 추출', () => {
		expect(extractDomain('http://example.com/path')).toBe('example.com')
		expect(extractDomain('https://www.google.com')).toBe('www.google.com')
		expect(extractDomain('invalid')).toBe(null)
	})
})
```

**securityStorage.js**:
```javascript
describe('securityStorage', () => {
	beforeEach(() => {
		sessionStorage.clear()
	})

	test('도메인 추가 및 조회', () => {
		addIgnoredDomain('example.com')
		const domains = getIgnoredDomains()
		expect(domains.has('example.com')).toBe(true)
	})

	test('중복 도메인 추가', () => {
		addIgnoredDomain('example.com')
		addIgnoredDomain('example.com')
		const domains = getIgnoredDomains()
		expect(domains.size).toBe(1)
	})

	test('도메인 제거', () => {
		addIgnoredDomain('example.com')
		removeIgnoredDomain('example.com')
		const domains = getIgnoredDomains()
		expect(domains.has('example.com')).toBe(false)
	})

	test('목록 초기화', () => {
		addIgnoredDomain('example.com')
		clearIgnoredDomains()
		const domains = getIgnoredDomains()
		expect(domains.size).toBe(0)
	})
})
```

### 11.2. 컴포넌트 테스트

**SecurityIndicator.js**:
```javascript
describe('SecurityIndicator', () => {
	test('HTTPS 아이콘 표시', () => {
		const { getByText } = render(
			<SecurityIndicator securityLevel="secure" url="https://example.com" />
		)
		expect(getByText('🔒')).toBeInTheDocument()
	})

	test('HTTP 아이콘 표시', () => {
		const { getByText } = render(
			<SecurityIndicator securityLevel="insecure" url="http://example.com" />
		)
		expect(getByText('⚠')).toBeInTheDocument()
	})

	test('unknown 등급은 아이콘 표시하지 않음', () => {
		const { container } = render(
			<SecurityIndicator securityLevel="unknown" url="" />
		)
		expect(container.firstChild).toBeNull()
	})

	test('클릭 이벤트', () => {
		const onClick = jest.fn()
		const { getByText } = render(
			<SecurityIndicator securityLevel="secure" url="https://example.com" onClick={onClick} />
		)
		fireEvent.click(getByText('🔒'))
		expect(onClick).toHaveBeenCalledTimes(1)
	})
})
```

**SecurityWarningDialog.js**:
```javascript
describe('SecurityWarningDialog', () => {
	test('다이얼로그 표시', () => {
		const { getByText } = render(
			<SecurityWarningDialog
				visible={true}
				url="http://example.com"
				onContinue={jest.fn()}
				onGoBack={jest.fn()}
				onDontShowAgain={jest.fn()}
			/>
		)
		expect(getByText('안전하지 않은 사이트')).toBeInTheDocument()
	})

	test('계속 진행 버튼', () => {
		const onContinue = jest.fn()
		const { getByText } = render(
			<SecurityWarningDialog
				visible={true}
				url="http://example.com"
				onContinue={onContinue}
				onGoBack={jest.fn()}
				onDontShowAgain={jest.fn()}
			/>
		)
		fireEvent.click(getByText('계속 진행'))
		expect(onContinue).toHaveBeenCalledTimes(1)
	})

	test('돌아가기 버튼', () => {
		const onGoBack = jest.fn()
		const { getByText } = render(
			<SecurityWarningDialog
				visible={true}
				url="http://example.com"
				onContinue={jest.fn()}
				onGoBack={onGoBack}
				onDontShowAgain={jest.fn()}
			/>
		)
		fireEvent.click(getByText('돌아가기'))
		expect(onGoBack).toHaveBeenCalledTimes(1)
	})

	test('무시 옵션 체크', () => {
		const onDontShowAgain = jest.fn()
		const { getByLabelText } = render(
			<SecurityWarningDialog
				visible={true}
				url="http://example.com"
				onContinue={jest.fn()}
				onGoBack={jest.fn()}
				onDontShowAgain={onDontShowAgain}
			/>
		)
		fireEvent.click(getByLabelText('오늘 이 사이트에 대해 다시 표시하지 않기'))
		expect(onDontShowAgain).toHaveBeenCalledWith(true)
	})
})
```

### 11.3. 통합 테스트

**BrowserView + WebView + URLBar + SecurityIndicator**:
```javascript
describe('HTTPS 보안 표시 통합', () => {
	test('HTTPS 사이트 접속 시 녹색 자물쇠 표시', async () => {
		const { getByText } = render(<BrowserView />)

		// URLBar에 HTTPS URL 입력
		const input = screen.getByPlaceholderText('URL을 입력하세요')
		fireEvent.change(input, { target: { value: 'https://example.com' } })
		fireEvent.submit(input)

		// WebView 로딩 대기
		await waitFor(() => {
			expect(getByText('🔒')).toBeInTheDocument()
		})
	})

	test('HTTP 사이트 접속 시 경고 다이얼로그 표시', async () => {
		const { getByText } = render(<BrowserView />)

		// URLBar에 HTTP URL 입력
		const input = screen.getByPlaceholderText('URL을 입력하세요')
		fireEvent.change(input, { target: { value: 'http://example.com' } })
		fireEvent.submit(input)

		// 500ms 디바운스 대기
		await waitFor(() => {
			expect(getByText('안전하지 않은 사이트')).toBeInTheDocument()
		}, { timeout: 1000 })
	})

	test('경고 무시 옵션 사용 시 재접속 시 경고 표시하지 않음', async () => {
		const { getByText, queryByText } = render(<BrowserView />)

		// HTTP 사이트 접속
		const input = screen.getByPlaceholderText('URL을 입력하세요')
		fireEvent.change(input, { target: { value: 'http://example.com' } })
		fireEvent.submit(input)

		// 경고 다이얼로그 대기
		await waitFor(() => {
			expect(getByText('안전하지 않은 사이트')).toBeInTheDocument()
		}, { timeout: 1000 })

		// "오늘 이 사이트에 대해 다시 표시하지 않기" 체크
		fireEvent.click(getByLabelText('오늘 이 사이트에 대해 다시 표시하지 않기'))
		fireEvent.click(getByText('계속 진행'))

		// 다른 페이지로 이동
		fireEvent.change(input, { target: { value: 'https://google.com' } })
		fireEvent.submit(input)

		// 다시 HTTP 사이트로 이동
		fireEvent.change(input, { target: { value: 'http://example.com' } })
		fireEvent.submit(input)

		// 경고 다이얼로그 표시되지 않음
		await waitFor(() => {
			expect(queryByText('안전하지 않은 사이트')).not.toBeInTheDocument()
		}, { timeout: 1000 })
	})
})
```

### 11.4. E2E 테스트

**실제 webOS 디바이스 테스트**:
1. HTTPS 사이트 (https://www.google.com) 접속 → 녹색 자물쇠 표시 확인
2. HTTP 사이트 (http://example.com) 접속 → 경고 다이얼로그 표시 확인
3. localhost (http://localhost:3000) 접속 → 경고 없이 정보 아이콘 표시 확인
4. HTTP → HTTPS 자동 리다이렉트 (http://github.com) → 경고 숨김, 최종 HTTPS 아이콘 표시 확인
5. 경고 무시 옵션 사용 → 세션 내 재접속 시 경고 없음 확인
6. 앱 재시작 → 무시 옵션 초기화, 경고 다시 표시 확인

---

## 12. 기술적 주의사항

### 주의 1: CORS 제약
- webOS iframe은 Same-Origin Policy로 인해 외부 도메인의 Document 객체, 인증서 정보에 접근할 수 없음
- 따라서 SSL 인증서 상세 정보 표시 불가 (유효 기간, 인증 기관, 암호화 수준 등)
- Mixed Content 감지 불가 (HTTPS 페이지 내 HTTP 리소스 로드 상태)

### 주의 2: 500ms 폴링 제약
- WebView는 500ms 간격으로 URL 변경을 감지하므로, 빠른 리다이렉트 시 중간 URL을 놓칠 수 있음
- 따라서 경고 다이얼로그는 500ms 디바운스로 지연 표시 (자동 리다이렉트 시 경고 숨김)

### 주의 3: SessionStorage 제한
- SessionStorage는 브라우저 탭 단위로 격리되지만, webOS 앱은 단일 인스턴스이므로 전역 공유
- 최대 저장 용량 5MB (초과 시 예외 발생) → 도메인 목록은 최대 100개로 제한 (비즈니스 로직에서 관리)

### 주의 4: 아이콘 폰트 vs 이미지
- 현재 설계는 유니코드 이모지 아이콘 사용 (🔒, ⚠, ℹ️)
- webOS 프로젝터에서 이모지 렌더링 실패 시, SVG 아이콘으로 대체 필요
- Enact Moonstone의 `Icon` 컴포넌트 활용 검토 (내장 아이콘 제공)

### 주의 5: Spotlight 포커스 순서
- URLBar에 SecurityIndicator 추가 시, Spotlight 포커스 순서 재조정 필요
- SecurityIndicator → URLBar Input → VirtualKeyboard 순서로 포커스 이동
- Spotlight.set()으로 leaveFor 설정 업데이트

---

## 13. 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | - |

---

## 14. 다음 단계

### 14.1. 구현 순서 (Phase 기준)

**Phase 1: 유틸리티 구현**
1. `src/utils/securityClassifier.js` 구현
2. `src/utils/securityStorage.js` 구현
3. 단위 테스트 작성 및 실행

**Phase 2: SecurityIndicator 구현**
1. `src/components/SecurityIndicator/SecurityIndicator.js` 구현
2. `src/components/SecurityIndicator/SecurityIndicator.module.less` 구현
3. 컴포넌트 테스트 작성

**Phase 3: URLBar 확장**
1. `src/components/URLBar/URLBar.js` 수정 (SecurityIndicator 통합)
2. `src/components/URLBar/URLBar.module.less` 수정 (레이아웃 조정)
3. Spotlight 포커스 순서 조정

**Phase 4: SecurityWarningDialog 구현**
1. `src/components/SecurityWarningDialog/SecurityWarningDialog.js` 구현
2. `src/components/SecurityWarningDialog/SecurityWarningDialog.module.less` 구현
3. 컴포넌트 테스트 작성

**Phase 5: BrowserView 통합**
1. `src/views/BrowserView.js` 수정 (handleNavigationChange 확장, 상태 관리 추가)
2. SecurityWarningDialog 이벤트 핸들러 구현
3. 통합 테스트 작성

**Phase 6: E2E 테스트**
1. webOS 시뮬레이터 테스트
2. 실제 프로젝터 디바이스 테스트
3. 버그 수정 및 최적화

### 14.2. 선택적 구현 (M3 범위 외)

**FR-5: 보안 아이콘 클릭 시 상세 정보 다이얼로그**
- SecurityInfoDialog 컴포넌트 구현
- SecurityIndicator onClick 이벤트 핸들러 추가
- 표시 가능한 정보: URL, 프로토콜, 보안 수준 (인증서 정보는 불가)

---

## 15. 참고 자료

### 15.1. 유사 구현 사례
- Chrome 브라우저 보안 아이콘 (주소창 좌측)
- Firefox 브라우저 보안 아이콘
- Safari 브라우저 보안 아이콘

### 15.2. 기술 문서
- MDN: `<iframe>` Sandbox 속성
- MDN: Same-Origin Policy
- Enact Moonstone: Dialog 컴포넌트
- Enact Spotlight: 포커스 관리

### 15.3. 보안 표준
- OWASP: Transport Layer Protection
- W3C: Mixed Content
- IETF: HTTP Strict Transport Security (HSTS)
