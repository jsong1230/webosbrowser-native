# URL 입력 UI — 기술 설계서

## 1. 참조
- 요구사항 분석서: `docs/specs/url-input-ui/requirements.md`
- 웹뷰 설계서: `docs/specs/webview-integration/design.md`
- PRD: `docs/project/prd.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md`

## 2. 아키텍처 개요

### 전체 구조
webOS 리모컨 전용 URL 입력 시스템을 구현합니다. Enact Moonstone Input 컴포넌트를 베이스로 하되, 리모컨 조작에 최적화된 커스텀 가상 키보드를 추가하여 물리 키보드 없이 URL 입력이 가능하도록 합니다.

```
┌─────────────────────────────────────────────────────────┐
│                      BrowserView                         │
│  ┌────────────────────────────────────────────────────┐ │
│  │                   URLBar Component                  │ │
│  │  ┌──────────────────────────────────────────────┐  │ │
│  │  │  Input Field (현재 URL 표시/편집)            │  │ │
│  │  │  - Enact Input 기반                          │  │ │
│  │  │  - Spotlightable                             │  │ │
│  │  │  - onFocus → VirtualKeyboard 표시            │  │ │
│  │  └──────────────────────────────────────────────┘  │ │
│  │  ┌──────────────────────────────────────────────┐  │ │
│  │  │  Autocomplete Dropdown (선택적)              │  │ │
│  │  │  - History + Bookmark 제안                   │  │ │
│  │  │  - VirtualList로 렌더링                       │  │ │
│  │  └──────────────────────────────────────────────┘  │ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │              VirtualKeyboard Component              │ │
│  │  ┌──────────────────────────────────────────────┐  │ │
│  │  │  키보드 그리드 (영문, 숫자, 특수문자)        │  │ │
│  │  │  - Spottable Buttons                         │  │ │
│  │  │  - 리모컨 십자키로 포커스 이동               │  │ │
│  │  │  - 선택 버튼으로 문자 입력                   │  │ │
│  │  └──────────────────────────────────────────────┘  │ │
│  │  ┌──────────────────────────────────────────────┐  │ │
│  │  │  제어 버튼 (백스페이스, 스페이스, 확인 등)   │  │ │
│  │  └──────────────────────────────────────────────┘  │ │
│  └────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────┐ │
│  │                  WebView Component                  │ │
│  └────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### 핵심 설계 원칙
1. **리모컨 우선 설계**: 모든 입력은 리모컨 5-way 방향키로만 조작 가능
2. **Spotlight 통합**: Enact Spotlight 시스템으로 일관된 포커스 관리
3. **컴포넌트 독립성**: URLBar와 VirtualKeyboard는 독립 컴포넌트로 재사용 가능
4. **상태 리프팅**: URL 상태는 BrowserView에서 관리, Props로 전달
5. **단계적 기능 추가**: 기본 입력 → 검증 → 자동완성 순으로 구현

## 3. 아키텍처 결정

### 결정 1: Input 컴포넌트 선택
- **선택지**:
  - A) Enact Input 컴포넌트 사용 (기본 제공)
  - B) HTML input 태그 + Spottable HOC
  - C) 완전 커스텀 Input 컴포넌트
- **결정**: A) Enact Input 컴포넌트 사용
- **근거**:
  - **Spotlight 통합**: Enact Input은 기본적으로 Spotlightable하여 리모컨 포커스 관리 자동
  - **접근성**: 키보드 이벤트, 상태 관리 내장
  - **일관성**: Moonstone UI 디자인 시스템과 자동 통합
  - **유지보수**: Enact 프레임워크 업데이트 시 호환성 보장
- **트레이드오프**: Enact Input의 기본 동작(키보드 입력 대기)을 가상 키보드로 우회해야 함

### 결정 2: 가상 키보드 레이아웃
- **선택지**:
  - A) QWERTY 레이아웃 (표준 키보드 배열)
  - B) ABC 순서 레이아웃 (알파벳 순)
  - C) 빈도 기반 최적화 레이아웃 (자주 쓰는 문자 중앙)
- **결정**: A) QWERTY 레이아웃
- **근거**:
  - **익숙함**: 대부분 사용자가 QWERTY 배열에 익숙함
  - **예측 가능성**: 물리 키보드와 동일한 배열로 학습 곡선 낮음
  - **호환성**: 향후 한글 키보드 추가 시 표준 한글 QWERTY와 통일
- **트레이드오프**: 리모컨 조작 시 문자 간 이동 거리가 길 수 있음 → 자동완성으로 보완

### 결정 3: 자동완성 데이터 소스
- **선택지**:
  - A) 히스토리 + 북마크 통합 검색
  - B) 히스토리만 사용
  - C) 북마크만 사용
- **결정**: A) 히스토리 + 북마크 통합 검색 (F-07, F-08 완료 후)
- **근거**:
  - **사용자 편의**: 히스토리는 최근 방문 사이트, 북마크는 자주 가는 사이트 → 둘 다 유용
  - **빠른 접근**: 입력 문자가 적어도 관련 사이트 제안 가능
  - **통합 UX**: 단일 드롭다운에서 모든 제안 표시
- **트레이드오프**: F-07, F-08 완료 전까지는 자동완성 비활성화

### 결정 4: URL 검증 시점
- **선택지**:
  - A) 입력 중 실시간 검증 (onChange)
  - B) 확인 버튼 클릭 시 검증 (onSubmit)
  - C) 포커스 이탈 시 검증 (onBlur)
- **결정**: B) 확인 버튼 클릭 시 검증
- **근거**:
  - **UX**: 입력 중 에러 메시지 표시는 방해가 될 수 있음
  - **성능**: 매 입력마다 검증 로직 실행 불필요
  - **명확성**: 사용자가 입력 완료 후 확인 버튼 클릭 시점에 검증 → 예측 가능
- **트레이드오프**: 입력 중 잘못된 형식을 즉시 알 수 없음 → 자동완성 제안으로 보완

### 결정 5: 가상 키보드 표시 방식
- **선택지**:
  - A) URL 입력 필드 포커스 시 자동 표시
  - B) 사용자가 선택 버튼 클릭 시 표시
  - C) 토글 버튼으로 수동 제어
- **결정**: A) URL 입력 필드 포커스 시 자동 표시
- **근거**:
  - **직관성**: 입력 필드 선택 = 입력 의도 → 키보드 자동 표시가 자연스러움
  - **단계 절약**: 추가 버튼 클릭 불필요
  - **일관성**: 스마트폰 가상 키보드와 동일한 동작 패턴
- **트레이드오프**: 입력 필드에 실수로 포커스 시 키보드가 의도치 않게 표시될 수 있음 → 백 버튼으로 닫기 가능

### 결정 6: 상태 관리 패턴
- **선택지**:
  - A) URLBar 컴포넌트 내부에서 useState로 URL 관리
  - B) BrowserView에서 URL 상태 관리, Props로 전달
  - C) Context API로 전역 URL 상태 관리
- **결정**: B) BrowserView에서 URL 상태 관리
- **근거**:
  - **상태 리프팅**: BrowserView가 URL을 WebView에도 전달해야 하므로 상위 컴포넌트에서 관리
  - **단순성**: 전역 상태 불필요 (URL은 BrowserView 하위에서만 사용)
  - **재사용성**: URLBar는 Props로 제어되므로 다른 화면에서도 재사용 가능
- **트레이드오프**: Props drilling 발생하지만, 깊이가 1단계로 관리 가능

## 4. 컴포넌트 설계

### 4.1 URLBar 컴포넌트

#### Props 인터페이스
```javascript
// src/components/URLBar/URLBar.js
import PropTypes from 'prop-types'

URLBar.propTypes = {
	// URL 값 및 제어
	value: PropTypes.string.isRequired,        // 현재 URL 값
	onChange: PropTypes.func.isRequired,       // URL 변경 시 호출
	onSubmit: PropTypes.func.isRequired,       // 확인 버튼 클릭 시 호출

	// 자동완성 (선택적)
	suggestions: PropTypes.arrayOf(            // 자동완성 제안 목록
		PropTypes.shape({
			url: PropTypes.string.isRequired,
			title: PropTypes.string,
			favicon: PropTypes.string           // 선택 (북마크/히스토리에서 제공)
		})
	),
	onSuggestionSelect: PropTypes.func,        // 자동완성 항목 선택 시 호출

	// 포커스 제어
	onFocus: PropTypes.func,                   // 포커스 진입 시 호출
	onBlur: PropTypes.func,                    // 포커스 이탈 시 호출

	// 스타일 커스터마이징
	className: PropTypes.string
}

URLBar.defaultProps = {
	suggestions: [],
	onSuggestionSelect: () => {},
	onFocus: () => {},
	onBlur: () => {},
	className: ''
}
```

#### 상태 정의
```javascript
const [inputValue, setInputValue] = useState(value)         // 내부 입력 값 (Props와 동기화)
const [isFocused, setIsFocused] = useState(false)           // 포커스 상태
const [showKeyboard, setShowKeyboard] = useState(false)     // 가상 키보드 표시 여부
const [showSuggestions, setShowSuggestions] = useState(false) // 자동완성 표시 여부
const [selectedIndex, setSelectedIndex] = useState(-1)      // 자동완성 선택 인덱스 (-1: 선택 없음)
const inputRef = useRef(null)                               // Input DOM 참조
```

#### 주요 메서드
```javascript
// 포커스 핸들러
const handleFocus = () => {
	setIsFocused(true)
	setShowKeyboard(true)
	onFocus()
}

const handleBlur = () => {
	setIsFocused(false)
	// 키보드는 백 버튼으로만 닫기 (자동 닫기 안 함)
	onBlur()
}

// 입력 변경 핸들러
const handleChange = (value) => {
	setInputValue(value)
	onChange(value)

	// 자동완성 제안 표시 (입력 길이 2자 이상)
	if (value.length >= 2 && suggestions.length > 0) {
		setShowSuggestions(true)
		setSelectedIndex(-1)  // 선택 초기화
	} else {
		setShowSuggestions(false)
	}
}

// 확인 버튼 핸들러 (가상 키보드의 Enter 키)
const handleSubmit = () => {
	// 자동완성 항목 선택 중이면 해당 항목 사용
	if (selectedIndex >= 0 && suggestions[selectedIndex]) {
		const selected = suggestions[selectedIndex]
		setInputValue(selected.url)
		onSuggestionSelect(selected)
	}

	// URL 검증 및 제출
	const validatedUrl = validateAndFormatUrl(inputValue)
	if (validatedUrl) {
		onSubmit(validatedUrl)
		setShowKeyboard(false)
		setShowSuggestions(false)
	} else {
		// 에러 메시지 표시 (F-10 에러 처리에서 구현)
		logger.warn('[URLBar] 유효하지 않은 URL:', inputValue)
	}
}

// 취소 버튼 핸들러 (백 버튼)
const handleCancel = () => {
	setShowKeyboard(false)
	setShowSuggestions(false)
	setInputValue(value)  // 이전 값 복원
}

// 자동완성 항목 선택 핸들러
const handleSuggestionSelect = (suggestion) => {
	setInputValue(suggestion.url)
	onSuggestionSelect(suggestion)
	setShowSuggestions(false)
}
```

### 4.2 VirtualKeyboard 컴포넌트

#### Props 인터페이스
```javascript
// src/components/VirtualKeyboard/VirtualKeyboard.js
import PropTypes from 'prop-types'

VirtualKeyboard.propTypes = {
	// 입력 제어
	onInput: PropTypes.func.isRequired,        // 문자 입력 시 호출 (char)
	onBackspace: PropTypes.func.isRequired,    // 백스페이스 클릭 시 호출
	onSpace: PropTypes.func.isRequired,        // 스페이스 클릭 시 호출
	onEnter: PropTypes.func.isRequired,        // 확인 버튼 클릭 시 호출
	onCancel: PropTypes.func.isRequired,       // 취소 버튼 클릭 시 호출

	// 표시 제어
	visible: PropTypes.bool,                   // 키보드 표시 여부

	// 스타일 커스터마이징
	className: PropTypes.string
}

VirtualKeyboard.defaultProps = {
	visible: true,
	className: ''
}
```

#### 키보드 레이아웃 정의
```javascript
// src/components/VirtualKeyboard/keyboardLayout.js

// QWERTY 레이아웃 (4행)
export const KEYBOARD_LAYOUT = [
	// 1행: 숫자 및 특수문자
	['1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '='],

	// 2행: 영문 상단
	['q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '/', ':'],

	// 3행: 영문 중단
	['a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '.', '_'],

	// 4행: 영문 하단 + 특수키
	['z', 'x', 'c', 'v', 'b', 'n', 'm', '?', '&', '=']
]

// 제어 키 (5행)
export const CONTROL_KEYS = [
	{ id: 'space', label: '스페이스', span: 4 },       // 4칸 폭
	{ id: 'backspace', label: '←', span: 2 },         // 2칸 폭
	{ id: 'clear', label: '전체삭제', span: 2 },      // 2칸 폭
	{ id: 'enter', label: '확인', span: 2 },          // 2칸 폭
	{ id: 'cancel', label: '취소', span: 2 }          // 2칸 폭
]
```

#### 상태 정의
```javascript
const [focusedKey, setFocusedKey] = useState({ row: 0, col: 0 })  // 현재 포커스된 키 위치
const keyRefs = useRef({})  // 각 키의 DOM ref (Spotlight 포커스 이동용)
```

#### 주요 메서드
```javascript
// 키 클릭 핸들러
const handleKeyPress = (char) => {
	onInput(char)
	logger.debug('[VirtualKeyboard] 문자 입력:', char)
}

// 제어 키 핸들러
const handleControlKey = (keyId) => {
	switch (keyId) {
		case 'space':
			onSpace()
			break
		case 'backspace':
			onBackspace()
			break
		case 'clear':
			onCancel()  // 전체 삭제는 취소와 동일 (이전 값 복원)
			break
		case 'enter':
			onEnter()
			break
		case 'cancel':
			onCancel()
			break
		default:
			break
	}
	logger.debug('[VirtualKeyboard] 제어 키 입력:', keyId)
}

// 리모컨 십자키 이동 핸들러 (Spotlight가 자동 처리하지만 수동 제어 필요 시)
const handleArrowKey = (direction) => {
	// Spotlight의 기본 동작을 사용하므로 추가 구현 불필요
	// 단, 키보드 경계에서 포커스 이탈 방지 필요 (Spotlight 설정으로 처리)
}
```

### 4.3 AutocompleteDropdown 컴포넌트 (선택적)

#### Props 인터페이스
```javascript
// src/components/URLBar/AutocompleteDropdown.js
import PropTypes from 'prop-types'

AutocompleteDropdown.propTypes = {
	// 제안 목록
	suggestions: PropTypes.arrayOf(
		PropTypes.shape({
			url: PropTypes.string.isRequired,
			title: PropTypes.string,
			favicon: PropTypes.string
		})
	).isRequired,

	// 선택 제어
	selectedIndex: PropTypes.number,           // 현재 선택된 인덱스 (-1: 선택 없음)
	onSelect: PropTypes.func.isRequired,       // 항목 선택 시 호출

	// 표시 제어
	visible: PropTypes.bool,

	// 스타일
	className: PropTypes.string
}

AutocompleteDropdown.defaultProps = {
	selectedIndex: -1,
	visible: false,
	className: ''
}
```

#### 렌더링 구조
```javascript
// Enact VirtualList 사용 (성능 최적화)
import { VirtualList } from '@enact/moonstone/VirtualList'

const AutocompleteDropdown = ({ suggestions, selectedIndex, onSelect, visible }) => {
	if (!visible || suggestions.length === 0) return null

	const renderItem = ({ index, ...rest }) => {
		const item = suggestions[index]
		const isSelected = index === selectedIndex

		return (
			<div
				{...rest}
				className={`${css.suggestionItem} ${isSelected ? css.selected : ''}`}
				onClick={() => onSelect(item)}
			>
				{item.favicon && <img src={item.favicon} className={css.favicon} alt="" />}
				<div className={css.info}>
					<div className={css.title}>{item.title || item.url}</div>
					<div className={css.url}>{item.url}</div>
				</div>
			</div>
		)
	}

	return (
		<div className={css.dropdown}>
			<VirtualList
				dataSize={suggestions.length}
				itemRenderer={renderItem}
				itemSize={60}  // 각 항목 높이 60px
			/>
		</div>
	)
}
```

## 5. URL 검증 및 자동 보완 설계

### URL 검증 로직
```javascript
// src/utils/urlValidator.js

/**
 * URL 유효성 검증 및 자동 보완
 *
 * @param {string} input - 사용자 입력 문자열
 * @returns {string|null} - 유효한 URL 또는 null
 */
export const validateAndFormatUrl = (input) => {
	if (!input || typeof input !== 'string') return null

	const trimmed = input.trim()
	if (trimmed.length === 0) return null

	// 1. 프로토콜 포함 URL (http://, https://)
	if (/^https?:\/\/.+/.test(trimmed)) {
		try {
			const url = new URL(trimmed)
			return url.href
		} catch (error) {
			return null
		}
	}

	// 2. 도메인 형식 감지 (example.com, www.example.com)
	if (/^[a-zA-Z0-9]+([\-\.]{1}[a-zA-Z0-9]+)*\.[a-zA-Z]{2,}(\/.*)?$/.test(trimmed)) {
		// http:// 자동 추가
		try {
			const url = new URL(`http://${trimmed}`)
			return url.href
		} catch (error) {
			return null
		}
	}

	// 3. localhost 또는 IP 주소
	if (/^(localhost|(\d{1,3}\.){3}\d{1,3})(:\d+)?(\/.*)?$/.test(trimmed)) {
		try {
			const url = new URL(`http://${trimmed}`)
			return url.href
		} catch (error) {
			return null
		}
	}

	// 4. 검색어로 판단 (F-09 검색 엔진 통합 시 구현)
	// 현재는 null 반환 (검증 실패)
	// 향후: return buildSearchUrl(trimmed, searchEngine)
	return null
}

/**
 * URL이 유효한 형식인지만 체크 (자동 보완 없이)
 *
 * @param {string} url - 검증할 URL
 * @returns {boolean}
 */
export const isValidUrl = (url) => {
	try {
		new URL(url)
		return true
	} catch (error) {
		return false
	}
}

/**
 * HTTPS 프로토콜 사용 여부 체크
 *
 * @param {string} url - 검증할 URL
 * @returns {boolean}
 */
export const isSecureUrl = (url) => {
	try {
		const urlObj = new URL(url)
		return urlObj.protocol === 'https:'
	} catch (error) {
		return false
	}
}
```

### 검색 엔진 통합 준비 (F-09에서 구현)
```javascript
// src/services/searchService.js

/**
 * 검색 엔진 URL 생성
 *
 * @param {string} query - 검색어
 * @param {string} engine - 검색 엔진 ('google', 'naver' 등)
 * @returns {string} - 검색 URL
 */
export const buildSearchUrl = (query, engine = 'google') => {
	const encodedQuery = encodeURIComponent(query)

	const searchEngines = {
		google: `https://www.google.com/search?q=${encodedQuery}`,
		naver: `https://search.naver.com/search.naver?query=${encodedQuery}`,
		bing: `https://www.bing.com/search?q=${encodedQuery}`,
		duckduckgo: `https://duckduckgo.com/?q=${encodedQuery}`
	}

	return searchEngines[engine] || searchEngines.google
}
```

## 6. Enact Spotlight 통합 설계

### Spotlight 포커스 경로
```
URLBar Input Field (포커스 진입)
    ↓ (선택 버튼 or 자동)
VirtualKeyboard 표시
    ↓ (방향키 아래)
VirtualKeyboard 첫 번째 키 (1행 1열)
    ↓ (방향키 좌/우/상/하)
VirtualKeyboard 내 키 간 이동
    ↓ (방향키 아래 - 마지막 행)
제어 키 (스페이스, 백스페이스, 확인, 취소)
    ↓ (백 버튼)
URLBar Input Field로 복귀 + 키보드 닫기
```

### Spotlight 설정 (키보드 경계 제어)
```javascript
// src/components/VirtualKeyboard/VirtualKeyboard.js
import Spotlight from '@enact/spotlight'
import SpotlightContainerDecorator from '@enact/spotlight/SpotlightContainerDecorator'

// Spotlight Container로 키보드 영역 격리
const Container = SpotlightContainerDecorator(
	{
		enterTo: 'default-element',           // 진입 시 첫 번째 요소로 포커스
		defaultElement: '[data-key="q"]',     // 기본 포커스: Q 키
		preserveId: true,                     // 컨테이너 ID 유지
		restrict: 'self-only'                 // 키보드 내부에서만 포커스 이동
	},
	'div'
)

const VirtualKeyboard = ({ visible, onInput, onEnter, onCancel, ... }) => {
	if (!visible) return null

	return (
		<Container className={css.keyboard} spotlightId="virtual-keyboard">
			{/* 키보드 렌더링 */}
			{KEYBOARD_LAYOUT.map((row, rowIndex) => (
				<div key={rowIndex} className={css.row}>
					{row.map((char, colIndex) => (
						<SpottableButton
							key={char}
							data-key={char}
							className={css.key}
							onClick={() => onInput(char)}
						>
							{char}
						</SpottableButton>
					))}
				</div>
			))}

			{/* 제어 키 */}
			<div className={css.controlRow}>
				{CONTROL_KEYS.map(key => (
					<SpottableButton
						key={key.id}
						data-key={key.id}
						className={`${css.controlKey} ${css[`span${key.span}`]}`}
						onClick={() => handleControlKey(key.id)}
					>
						{key.label}
					</SpottableButton>
				))}
			</div>
		</Container>
	)
}
```

### 리모컨 키 매핑
| 리모컨 키 | 동작 | 처리 위치 |
|-----------|------|----------|
| 십자키 (↑↓←→) | 키보드 내 포커스 이동 | Spotlight 자동 |
| 선택 (OK) | 문자 입력 / 제어 키 실행 | VirtualKeyboard onClick |
| 백 (Back) | 키보드 닫기, 입력 취소 | BrowserView onKeyDown 캡처 |
| 홈 (Home) | (미사용, 시스템 이벤트) | - |

### URLBar ↔ VirtualKeyboard 포커스 전환
```javascript
// src/components/URLBar/URLBar.js

const handleKeyDown = (event) => {
	// 방향키 아래: 키보드로 포커스 이동
	if (event.keyCode === 40) {  // Arrow Down
		if (showKeyboard) {
			Spotlight.focus('virtual-keyboard')  // 키보드 컨테이너로 포커스 이동
		}
	}
}

return (
	<div className={css.urlBar}>
		<Input
			ref={inputRef}
			value={inputValue}
			onChange={handleChange}
			onFocus={handleFocus}
			onBlur={handleBlur}
			onKeyDown={handleKeyDown}
			placeholder="URL을 입력하세요"
			className={css.input}
		/>
		{showKeyboard && (
			<VirtualKeyboard
				visible={showKeyboard}
				onInput={handleKeyboardInput}
				onEnter={handleSubmit}
				onCancel={handleCancel}
				{...}
			/>
		)}
	</div>
)
```

## 7. 자동완성 설계 (선택적, F-07/F-08 연동)

### 데이터 소스 통합
```javascript
// src/hooks/useAutocompleteSuggestions.js

import { useState, useEffect } from 'react'
import { searchHistory } from '../services/historyService'
import { searchBookmarks } from '../services/bookmarkService'

/**
 * 자동완성 제안 Hook
 *
 * @param {string} query - 검색 쿼리
 * @param {number} maxResults - 최대 결과 수 (기본: 5)
 * @returns {Array} - 제안 목록
 */
export const useAutocompleteSuggestions = (query, maxResults = 5) => {
	const [suggestions, setSuggestions] = useState([])

	useEffect(() => {
		if (!query || query.length < 2) {
			setSuggestions([])
			return
		}

		// 히스토리 검색 (F-08)
		const historyResults = searchHistory(query, maxResults)

		// 북마크 검색 (F-07)
		const bookmarkResults = searchBookmarks(query, maxResults)

		// 통합 및 정렬 (히스토리 우선, 중복 제거)
		const combined = [...historyResults, ...bookmarkResults]
		const unique = Array.from(new Map(combined.map(item => [item.url, item])).values())

		// 상위 N개만 반환
		const topResults = unique.slice(0, maxResults)

		setSuggestions(topResults)
	}, [query, maxResults])

	return suggestions
}
```

### BrowserView 통합
```javascript
// src/views/BrowserView.js

import { useAutocompleteSuggestions } from '../hooks/useAutocompleteSuggestions'

const BrowserView = () => {
	const [currentUrl, setCurrentUrl] = useState('https://www.google.com')
	const [inputValue, setInputValue] = useState(currentUrl)

	// 자동완성 제안 (F-07, F-08 완료 후 활성화)
	const suggestions = useAutocompleteSuggestions(inputValue, 5)

	const handleUrlChange = (newUrl) => {
		setInputValue(newUrl)
	}

	const handleUrlSubmit = (validatedUrl) => {
		setCurrentUrl(validatedUrl)
		setInputValue(validatedUrl)
		logger.info('[BrowserView] URL 제출:', validatedUrl)
	}

	const handleSuggestionSelect = (suggestion) => {
		setCurrentUrl(suggestion.url)
		setInputValue(suggestion.url)
		logger.info('[BrowserView] 자동완성 선택:', suggestion)
	}

	return (
		<Panel className={css.browserView}>
			<Header title="webOS Browser" />

			<URLBar
				value={inputValue}
				onChange={handleUrlChange}
				onSubmit={handleUrlSubmit}
				suggestions={suggestions}
				onSuggestionSelect={handleSuggestionSelect}
			/>

			{/* WebView ... */}
		</Panel>
	)
}
```

## 8. 시퀀스 흐름 설계

### 주요 시나리오: URL 입력 및 페이지 로드
```
사용자      URLBar      VirtualKeyboard    urlValidator    WebView
  │            │               │                  │             │
  │ 방향키 이동 │               │                  │             │
  │───────────▶│               │                  │             │
  │            │ onFocus       │                  │             │
  │            │ 키보드 표시    │                  │             │
  │            │──────────────▶│                  │             │
  │            │               │                  │             │
  │ 선택 (문자) │               │                  │             │
  │────────────┼──────────────▶│                  │             │
  │            │               │ onInput(char)    │             │
  │            │◀──────────────│                  │             │
  │            │ onChange(value)                  │             │
  │            │ 입력 필드 업데이트                │             │
  │            │               │                  │             │
  │ 선택 (확인) │               │                  │             │
  │────────────┼──────────────▶│                  │             │
  │            │               │ onEnter()        │             │
  │            │◀──────────────│                  │             │
  │            │ validateAndFormatUrl(input)       │             │
  │            │─────────────────────────────────▶│             │
  │            │               │    validUrl      │             │
  │            │◀─────────────────────────────────│             │
  │            │ onSubmit(validUrl)               │             │
  │            │──────────────────────────────────┼────────────▶│
  │            │ 키보드 닫기    │                  │             │
  │            │               │                  │  페이지 로드 │
```

### 에러 시나리오: 유효하지 않은 URL
```
사용자      URLBar      VirtualKeyboard    urlValidator
  │            │               │                  │
  │ "invalid" 입력              │                  │
  │            │ onChange("invalid")              │
  │            │               │                  │
  │ 선택 (확인) │               │                  │
  │────────────┼──────────────▶│                  │
  │            │               │ onEnter()        │
  │            │◀──────────────│                  │
  │            │ validateAndFormatUrl("invalid")  │
  │            │─────────────────────────────────▶│
  │            │               │      null        │
  │            │◀─────────────────────────────────│
  │            │ 에러 메시지 표시                 │
  │            │ "유효한 URL을 입력하세요"        │
  │◀───────────│               │                  │
  │            │ 키보드 유지    │                  │
```

### 자동완성 시나리오 (F-07, F-08 연동 시)
```
사용자      URLBar      useAutocompleteSuggestions    historyService / bookmarkService
  │            │                     │                           │
  │ "goo" 입력 │                     │                           │
  │───────────▶│                     │                           │
  │            │ onChange("goo")     │                           │
  │            │ query.length >= 2   │                           │
  │            │────────────────────▶│                           │
  │            │                     │ searchHistory("goo")      │
  │            │                     │──────────────────────────▶│
  │            │                     │ [{ url: "google.com", ... }]
  │            │                     │◀──────────────────────────│
  │            │                     │ searchBookmarks("goo")    │
  │            │                     │──────────────────────────▶│
  │            │                     │ [{ url: "google.com", ... }]
  │            │                     │◀──────────────────────────│
  │            │ suggestions: [...]  │                           │
  │            │◀────────────────────│                           │
  │            │ showSuggestions(true)                           │
  │            │ 드롭다운 표시       │                           │
  │◀───────────│                     │                           │
  │            │                     │                           │
  │ 방향키 아래 │                     │                           │
  │───────────▶│ 제안 항목 포커스    │                           │
  │            │                     │                           │
  │ 선택 (항목) │                     │                           │
  │───────────▶│ onSuggestionSelect(item)                       │
  │            │ onChange(item.url)  │                           │
  │            │ 드롭다운 닫기       │                           │
```

## 9. 스타일 설계

### URLBar 스타일
```less
// src/components/URLBar/URLBar.module.less

.urlBar {
	display: flex;
	flex-direction: column;
	width: 100%;
	padding: var(--spacing-md);
	background-color: #2a2a2a;
}

.input {
	width: 100%;
	height: 60px;
	font-size: 18px;  // 최소 폰트 크기 (요구사항)
	padding: var(--spacing-sm) var(--spacing-md);
	border: 2px solid transparent;
	border-radius: 8px;
	background-color: #3a3a3a;
	color: #ffffff;

	// Spotlight 포커스 시
	&:global(.spottable):focus {
		border-color: var(--accent-color, #00aaff);
		box-shadow: 0 0 8px rgba(0, 170, 255, 0.5);
	}
}

.dropdown {
	width: 100%;
	max-height: 300px;  // 5개 항목 (60px * 5)
	overflow: hidden;
	background-color: #3a3a3a;
	border-radius: 8px;
	margin-top: 8px;
}

.suggestionItem {
	display: flex;
	align-items: center;
	height: 60px;
	padding: var(--spacing-sm) var(--spacing-md);
	border-bottom: 1px solid #4a4a4a;
	cursor: pointer;

	&:hover,
	&.selected {
		background-color: #4a4a4a;
	}
}

.favicon {
	width: 24px;
	height: 24px;
	margin-right: var(--spacing-sm);
	border-radius: 4px;
}

.info {
	flex: 1;
	overflow: hidden;
}

.title {
	font-size: 16px;
	color: #ffffff;
	white-space: nowrap;
	overflow: hidden;
	text-overflow: ellipsis;
}

.url {
	font-size: 14px;
	color: #999999;
	white-space: nowrap;
	overflow: hidden;
	text-overflow: ellipsis;
}
```

### VirtualKeyboard 스타일
```less
// src/components/VirtualKeyboard/VirtualKeyboard.module.less

.keyboard {
	position: absolute;
	bottom: 0;
	left: 0;
	right: 0;
	padding: var(--spacing-md);
	background-color: #1a1a1a;
	border-top: 2px solid #3a3a3a;
	z-index: 1000;
}

.row {
	display: flex;
	justify-content: center;
	margin-bottom: 8px;
}

.key {
	min-width: 60px;
	min-height: 60px;  // 최소 키 크기 (요구사항)
	margin: 0 4px;
	font-size: 20px;  // 최소 폰트 크기 (요구사항)
	font-weight: bold;
	color: #ffffff;
	background-color: #3a3a3a;
	border: 2px solid transparent;
	border-radius: 8px;
	cursor: pointer;

	// Spotlight 포커스 시
	&:global(.spottable):focus {
		border-color: var(--accent-color, #00aaff);
		background-color: #4a4a4a;
		box-shadow: 0 0 8px rgba(0, 170, 255, 0.5);
	}

	// 클릭 시
	&:active {
		background-color: #2a2a2a;
	}
}

.controlRow {
	display: flex;
	justify-content: center;
	margin-top: 12px;
}

.controlKey {
	min-height: 60px;
	margin: 0 4px;
	font-size: 18px;
	color: #ffffff;
	background-color: #4a4a4a;
	border: 2px solid transparent;
	border-radius: 8px;
	cursor: pointer;

	&:global(.spottable):focus {
		border-color: var(--accent-color, #00aaff);
		background-color: #5a5a5a;
		box-shadow: 0 0 8px rgba(0, 170, 255, 0.5);
	}

	// 폭 조정 (CONTROL_KEYS의 span 값에 따라)
	&.span2 { width: 120px; }
	&.span4 { width: 240px; }
}
```

## 10. 영향 범위 분석

### 수정 필요한 기존 파일
- **`src/views/BrowserView.js`**:
  - URLBar 컴포넌트 import 및 사용
  - URL 상태 관리 추가 (useState)
  - WebView에 URL Props 전달
  - 자동완성 Hook 통합 (F-07, F-08 완료 후)
- **`src/views/BrowserView.module.less`**:
  - `.urlBarPlaceholder` 제거
  - URLBar 영역 스타일 조정

### 새로 생성할 파일
- **컴포넌트**:
  - `src/components/URLBar/URLBar.js`
  - `src/components/URLBar/URLBar.module.less`
  - `src/components/URLBar/index.js`
  - `src/components/URLBar/AutocompleteDropdown.js` (선택적)
  - `src/components/VirtualKeyboard/VirtualKeyboard.js`
  - `src/components/VirtualKeyboard/VirtualKeyboard.module.less`
  - `src/components/VirtualKeyboard/keyboardLayout.js`
  - `src/components/VirtualKeyboard/index.js`
- **유틸리티**:
  - `src/utils/urlValidator.js`
- **서비스** (F-09 연동 시):
  - `src/services/searchService.js`
- **Hooks** (F-07, F-08 연동 시):
  - `src/hooks/useAutocompleteSuggestions.js`
- **테스트**:
  - `src/__tests__/components/URLBar.test.js`
  - `src/__tests__/components/VirtualKeyboard.test.js`
  - `src/__tests__/utils/urlValidator.test.js`

### 영향 받는 기존 기능
- **F-02 (웹뷰 통합)**: URL 변경 시 WebView 컴포넌트로 전달 → 페이지 로드
- **F-07 (북마크 관리)**: (연동 시) 북마크 데이터를 자동완성에 제공
- **F-08 (히스토리 관리)**: (연동 시) 히스토리 데이터를 자동완성에 제공
- **F-09 (검색 엔진 통합)**: (연동 시) URL이 아닌 검색어 입력 시 검색 엔진으로 쿼리
- **F-04 (페이지 탐색)**: NavigationBar와 URLBar 간 포커스 이동 경로 조정 필요

## 11. 구현 순서

### Phase 1: URL 검증 유틸리티
1. `src/utils/urlValidator.js` 생성
2. `validateAndFormatUrl`, `isValidUrl`, `isSecureUrl` 함수 구현
3. 단위 테스트 작성 (`src/__tests__/utils/urlValidator.test.js`)

### Phase 2: VirtualKeyboard 컴포넌트
1. `src/components/VirtualKeyboard/` 디렉토리 생성
2. `keyboardLayout.js` 생성 (KEYBOARD_LAYOUT, CONTROL_KEYS 정의)
3. `VirtualKeyboard.js` 생성
   - Props 인터페이스 정의
   - 키보드 렌더링 (QWERTY 레이아웃)
   - 제어 키 렌더링
   - SpotlightContainer 설정
4. `VirtualKeyboard.module.less` 생성 (키 스타일, 포커스 스타일)
5. `index.js` 생성 (export)
6. 단위 테스트 작성

### Phase 3: URLBar 컴포넌트 (기본 버전)
1. `src/components/URLBar/` 디렉토리 생성
2. `URLBar.js` 생성
   - Props 인터페이스 정의
   - Enact Input 사용
   - VirtualKeyboard 통합 (포커스 시 표시)
   - handleSubmit에서 urlValidator 사용
3. `URLBar.module.less` 생성
4. `index.js` 생성 (export)
5. 단위 테스트 작성

### Phase 4: BrowserView 통합
1. `src/views/BrowserView.js` 수정
   - URLBar import 및 사용
   - URL 상태 관리 추가
   - WebView에 URL Props 전달
   - URLBar 이벤트 핸들러 구현
2. `src/views/BrowserView.module.less` 수정
   - `.urlBarPlaceholder` 제거
   - URLBar 영역 스타일 조정

### Phase 5: Spotlight 통합 및 테스트
1. URLBar ↔ VirtualKeyboard 포커스 전환 구현
2. 리모컨 백 버튼으로 키보드 닫기 구현
3. 로컬 브라우저 테스트 (`npm run serve`)
   - 탭 키로 포커스 이동 테스트
   - 키보드 입력 테스트
   - URL 검증 테스트

### Phase 6: 실제 디바이스 테스트
1. `npm run pack-p` 빌드
2. 프로젝터 설치 및 실행
3. 리모컨 조작 테스트
   - URL 필드 포커스
   - 키보드 문자 입력
   - 백스페이스, 스페이스, 확인 버튼
   - 백 버튼으로 키보드 닫기
4. 주요 사이트 URL 입력 및 로드 테스트
   - google.com → http://google.com 자동 보완 확인
   - https://www.youtube.com 입력 및 로드

### Phase 7: AutocompleteDropdown 컴포넌트 (선택적, F-07/F-08 완료 후)
1. `src/components/URLBar/AutocompleteDropdown.js` 생성
2. VirtualList로 제안 목록 렌더링
3. 자동완성 선택 로직 구현
4. URLBar에 통합

### Phase 8: 자동완성 데이터 통합 (F-07/F-08 완료 후)
1. `src/hooks/useAutocompleteSuggestions.js` 생성
2. historyService, bookmarkService 통합
3. BrowserView에서 Hook 사용
4. URLBar에 suggestions Props 전달

### Phase 9: 검색 엔진 통합 (F-09 완료 후)
1. `src/services/searchService.js` 생성
2. urlValidator에서 검색어 감지 시 buildSearchUrl 호출
3. 설정에서 기본 검색 엔진 선택 기능 연동

### Phase 10: 최종 테스트 및 문서화
1. 회귀 테스트 (requirements.md의 AC-1~AC-8)
2. 성능 테스트 (입력 반응 속도 0.3초 이내)
3. 컴포넌트 문서 작성 (`docs/components/URLBar.md`, `VirtualKeyboard.md`)
4. CHANGELOG 업데이트

## 12. 기술적 주의사항

### Enact Input의 기본 동작 제한
- **문제**: Enact Input은 물리 키보드 입력을 기대하지만, webOS에서는 리모컨만 사용
- **대응**:
  - Input의 `readOnly` 속성을 고려했으나, 포커스 표시가 필요하므로 사용 안 함
  - VirtualKeyboard에서 `onInput` 콜백으로 Input의 value를 직접 업데이트
  - Input의 onChange 이벤트는 VirtualKeyboard 입력 시에만 발생하도록 제어

### Spotlight 포커스 충돌
- **문제**: URLBar Input과 VirtualKeyboard 간 포커스 전환 시 Spotlight가 예상치 못한 동작
- **대응**:
  - VirtualKeyboard를 SpotlightContainer로 격리 (`restrict: 'self-only'`)
  - URLBar에서 방향키 아래 감지 시 명시적으로 `Spotlight.focus('virtual-keyboard')` 호출
  - 백 버튼 이벤트는 BrowserView에서 캡처하여 키보드 닫기 처리

### URL 검증 시 검색어와 도메인 구분
- **문제**: "apple"은 검색어인가 도메인(apple.com)인가?
- **대응**:
  - 도메인 형식(점 포함) 우선: "example.com"은 도메인으로 처리
  - 점 없는 단일 단어는 검색어로 처리 (F-09 완료 후)
  - 현재 버전에서는 점 없는 입력은 유효하지 않은 URL로 에러 처리

### 자동완성 성능
- **문제**: 히스토리/북마크가 수천 개일 경우 검색 성능 저하
- **대응**:
  - `useAutocompleteSuggestions`에서 최대 5개만 반환
  - historyService, bookmarkService에서 인덱스 기반 검색 (F-07, F-08에서 구현)
  - 입력 길이 2자 이상일 때만 검색 시작

### VirtualKeyboard 메모리 누수
- **문제**: 키보드 표시/숨김 반복 시 이벤트 리스너 누적 가능
- **대응**:
  - useEffect cleanup 함수에서 이벤트 리스너 정리
  - Spotlight 이벤트는 Enact가 자동 정리

### 리모컨 백 버튼 이벤트 캡처
- **문제**: 백 버튼은 기본적으로 앱 종료 또는 이전 화면 이동
- **대응**:
  - BrowserView에서 `onKeyDown` 이벤트를 캡처
  - 키보드가 표시 중이면 `event.preventDefault()`로 기본 동작 방지
  - 키보드 닫기 로직 실행

```javascript
// src/views/BrowserView.js

const handleKeyDown = (event) => {
	// 백 버튼 (keyCode 8: Backspace, keyCode 461: webOS Back)
	if (event.keyCode === 8 || event.keyCode === 461) {
		if (showKeyboard) {
			event.preventDefault()
			event.stopPropagation()
			setShowKeyboard(false)
			logger.info('[BrowserView] 백 버튼으로 키보드 닫기')
		}
	}
}

return (
	<Panel className={css.browserView} onKeyDown={handleKeyDown}>
		{/* ... */}
	</Panel>
)
```

## 13. 확장성 고려사항

### 한글 키보드 추가 (M3 이후)
- **준비**:
  - `keyboardLayout.js`에서 다국어 레이아웃을 배열로 관리
  - VirtualKeyboard에 `layout` Props 추가 ('en', 'ko' 선택 가능)
  - 키보드 전환 버튼 추가 (제어 키 영역)

```javascript
// src/components/VirtualKeyboard/keyboardLayout.js

export const LAYOUTS = {
	en: {
		name: 'English',
		rows: [ /* QWERTY 레이아웃 */ ]
	},
	ko: {
		name: '한글',
		rows: [ /* 한글 자음/모음 레이아웃 */ ]
	}
}
```

### 음성 입력 (추후 확장)
- **준비**:
  - URLBar에 `showVoiceButton` Props 추가
  - 음성 인식 버튼 추가 (제어 키 영역)
  - webOS VoiceService API 연동 (별도 서비스)

### URL 히스토리 동기화 (추후 확장)
- **준비**:
  - 자동완성 데이터를 외부 API에서 가져오도록 확장
  - LG 계정 연동 (webOS Service API)

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-12 | 최초 작성 | F-03 요구사항 기반 기술 설계 |
