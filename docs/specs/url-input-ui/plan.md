# URL 입력 UI — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: `docs/specs/url-input-ui/requirements.md`
- 기술 설계서: `docs/specs/url-input-ui/design.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md`
- 프로젝트 기획: `docs/project/features.md`

---

## 2. 구현 Phase

### Phase 1: URL 검증 유틸리티 구현
**담당**: frontend-dev
**예상 시간**: 30분
**의존성**: 없음

#### 작업 내용
- [ ] Task 1-1: `src/utils/urlValidator.js` 파일 생성
  - `validateAndFormatUrl(input)` 함수 구현
    - 프로토콜 포함 URL 검증 (http://, https://)
    - 도메인 형식 감지 및 자동 프로토콜 추가
    - localhost/IP 주소 지원
    - 검색어 감지 (현재는 null 반환, F-09 연동 시 확장)
  - `isValidUrl(url)` 함수 구현
  - `isSecureUrl(url)` 함수 구현 (HTTPS 체크)
  - JSDoc 주석 작성
- [ ] Task 1-2: 단위 테스트 작성
  - `src/__tests__/utils/urlValidator.test.js` 생성
  - 테스트 케이스:
    - 프로토콜 포함 URL (http://, https://)
    - 프로토콜 없는 도메인 (google.com → http://google.com)
    - localhost 및 IP 주소
    - 유효하지 않은 입력 (null, 빈 문자열, 특수문자만)
    - 경로 및 쿼리 파라미터 포함 URL
  - `npm test` 실행 및 통과 확인

#### 예상 산출물
- `src/utils/urlValidator.js`
- `src/__tests__/utils/urlValidator.test.js`

#### 완료 기준
- 모든 단위 테스트 통과 (100% 커버리지)
- URL 검증 로직이 requirements.md의 FR-4 요구사항 충족
- JSDoc 주석으로 함수 인터페이스 문서화

---

### Phase 2: VirtualKeyboard 컴포넌트 구현
**담당**: frontend-dev
**예상 시간**: 2시간
**의존성**: 없음 (Phase 1과 병렬 가능)

#### 작업 내용
- [ ] Task 2-1: 키보드 레이아웃 정의
  - `src/components/VirtualKeyboard/` 디렉토리 생성
  - `keyboardLayout.js` 생성
    - `KEYBOARD_LAYOUT` 배열 정의 (QWERTY 4행)
      - 1행: 숫자 및 특수문자 (1-0, -, =)
      - 2행: 영문 상단 (q-p, /, :)
      - 3행: 영문 중단 (a-l, ., _)
      - 4행: 영문 하단 (z-m, ?, &, =)
    - `CONTROL_KEYS` 배열 정의 (5행)
      - 스페이스 (4칸 폭), 백스페이스 (2칸), 전체삭제 (2칸), 확인 (2칸), 취소 (2칸)
  - export 문 추가
- [ ] Task 2-2: VirtualKeyboard 컴포넌트 구현
  - `VirtualKeyboard.js` 생성
  - Props 인터페이스 정의
    - `onInput`: 문자 입력 콜백
    - `onBackspace`: 백스페이스 콜백
    - `onSpace`: 스페이스 콜백
    - `onEnter`: 확인 버튼 콜백
    - `onCancel`: 취소 버튼 콜백
    - `visible`: 키보드 표시 여부
    - `className`: 스타일 커스터마이징
  - PropTypes 선언 및 defaultProps 설정
  - 상태 관리
    - `focusedKey`: 현재 포커스된 키 위치 (row, col)
    - `keyRefs`: 각 키의 DOM 참조 (useRef)
  - 렌더링 구조
    - KEYBOARD_LAYOUT 매핑하여 키 그리드 생성
    - 각 키는 Moonstone `Button` + `Spottable` HOC 사용
    - CONTROL_KEYS 매핑하여 제어 키 생성
    - SpotlightContainerDecorator로 키보드 영역 격리
      - `enterTo: 'default-element'`
      - `defaultElement: '[data-key="q"]'`
      - `restrict: 'self-only'`
      - `spotlightId: "virtual-keyboard"`
  - 이벤트 핸들러
    - `handleKeyPress(char)`: onInput 콜백 호출
    - `handleControlKey(keyId)`: 제어 키별 분기 처리
  - logger 통합 (디버그 로그)
- [ ] Task 2-3: VirtualKeyboard 스타일 구현
  - `VirtualKeyboard.module.less` 생성
  - 레이아웃 스타일
    - `.keyboard`: 화면 하단 고정 (position: absolute, bottom: 0)
    - `.row`: flexbox 가운데 정렬
    - `.key`: 최소 60x60px, 폰트 20px, 마진 4px
    - `.controlRow`: 제어 키 영역
    - `.controlKey`: 폭 가변 (span2: 120px, span4: 240px)
  - 포커스 스타일
    - Spotlight 포커스 시: 테두리 3px, 색상 #00aaff, box-shadow
    - active 상태: 배경색 어둡게
  - 색상 테마: 다크 모드 (배경 #1a1a1a, 키 #3a3a3a)
- [ ] Task 2-4: index.js 생성 및 export
  - `src/components/VirtualKeyboard/index.js` 생성
  - VirtualKeyboard 컴포넌트 export
- [ ] Task 2-5: 단위 테스트 작성
  - `src/__tests__/components/VirtualKeyboard.test.js` 생성
  - 테스트 케이스:
    - 키보드 렌더링 (visible=true)
    - 키보드 숨김 (visible=false)
    - 문자 키 클릭 시 onInput 콜백 호출
    - 제어 키 클릭 시 해당 콜백 호출 (onBackspace, onSpace, onEnter, onCancel)
    - QWERTY 레이아웃 모든 키 존재 확인
  - Enact Testing Library 사용

#### 예상 산출물
- `src/components/VirtualKeyboard/keyboardLayout.js`
- `src/components/VirtualKeyboard/VirtualKeyboard.js`
- `src/components/VirtualKeyboard/VirtualKeyboard.module.less`
- `src/components/VirtualKeyboard/index.js`
- `src/__tests__/components/VirtualKeyboard.test.js`

#### 완료 기준
- 키보드가 화면 하단에 정상 렌더링
- 모든 키(영문, 숫자, 특수문자, 제어 키)가 클릭 가능
- 포커스 스타일이 명확하게 표시
- 단위 테스트 통과
- requirements.md의 FR-2 (가상 키보드) 요구사항 충족

---

### Phase 3: URLBar 컴포넌트 구현 (기본 버전)
**담당**: frontend-dev
**예상 시간**: 1.5시간
**의존성**: Phase 1, Phase 2 완료

#### 작업 내용
- [ ] Task 3-1: URLBar 컴포넌트 구현
  - `src/components/URLBar/` 디렉토리 생성
  - `URLBar.js` 생성
  - Props 인터페이스 정의
    - `value`: 현재 URL 값
    - `onChange`: URL 변경 콜백
    - `onSubmit`: 확인 버튼 클릭 콜백
    - `suggestions`: 자동완성 제안 목록 (선택적, 기본값 [])
    - `onSuggestionSelect`: 자동완성 항목 선택 콜백 (선택적)
    - `onFocus`, `onBlur`: 포커스 이벤트 콜백
    - `className`: 스타일 커스터마이징
  - PropTypes 선언 및 defaultProps 설정
  - 상태 관리
    - `inputValue`: 내부 입력 값 (Props와 동기화)
    - `isFocused`: 포커스 상태
    - `showKeyboard`: 가상 키보드 표시 여부
    - `showSuggestions`: 자동완성 표시 여부 (기본: false, F-07/F-08 연동 시 활성화)
    - `selectedIndex`: 자동완성 선택 인덱스 (기본: -1)
    - `inputRef`: Input DOM 참조 (useRef)
  - Enact Input 컴포넌트 사용
    - Moonstone Input import
    - Spotlightable 적용 (기본 제공)
    - placeholder: "URL을 입력하세요"
  - VirtualKeyboard 통합
    - 조건부 렌더링 (showKeyboard=true일 때만 표시)
    - 키보드 이벤트 핸들러
      - `handleKeyboardInput(char)`: inputValue에 문자 추가, onChange 콜백 호출
      - `handleKeyboardBackspace()`: inputValue 마지막 문자 제거
      - `handleKeyboardSpace()`: inputValue에 스페이스 추가
      - `handleKeyboardEnter()`: handleSubmit 호출
      - `handleKeyboardCancel()`: showKeyboard=false, inputValue 이전 값 복원
  - URL 검증 및 제출
    - `handleSubmit()` 함수
      - `validateAndFormatUrl(inputValue)` 호출 (Phase 1 유틸리티)
      - 유효한 URL: onSubmit 콜백 호출, 키보드 닫기
      - 유효하지 않은 URL: logger.warn 로그, 에러 메시지 표시 (F-10 연동 시 확장)
  - 포커스 핸들러
    - `handleFocus()`: isFocused=true, showKeyboard=true, onFocus 콜백 호출
    - `handleBlur()`: isFocused=false, onBlur 콜백 호출 (키보드는 백 버튼으로만 닫기)
  - 리모컨 방향키 하 감지
    - `handleKeyDown(event)`: 방향키 아래 (keyCode 40) 감지 시 Spotlight.focus('virtual-keyboard')
- [ ] Task 3-2: URLBar 스타일 구현
  - `URLBar.module.less` 생성
  - 레이아웃 스타일
    - `.urlBar`: flexbox column, width 100%, padding 16px
    - `.input`: width 100%, height 60px, 폰트 18px, border-radius 8px
  - 포커스 스타일
    - Spotlight 포커스 시: 테두리 2px #00aaff, box-shadow
  - 색상 테마: 다크 모드 (배경 #2a2a2a, 입력 필드 #3a3a3a)
- [ ] Task 3-3: index.js 생성 및 export
  - `src/components/URLBar/index.js` 생성
  - URLBar 컴포넌트 export
- [ ] Task 3-4: 단위 테스트 작성
  - `src/__tests__/components/URLBar.test.js` 생성
  - 테스트 케이스:
    - URLBar 렌더링 (Input 필드 존재)
    - 포커스 시 VirtualKeyboard 표시
    - 키보드 문자 입력 시 inputValue 업데이트
    - 확인 버튼 클릭 시 onSubmit 콜백 호출 (유효한 URL)
    - 유효하지 않은 URL 입력 시 onSubmit 호출 안 됨
    - 취소 버튼 클릭 시 키보드 닫기 및 값 복원
  - Enact Testing Library 사용

#### 예상 산출물
- `src/components/URLBar/URLBar.js`
- `src/components/URLBar/URLBar.module.less`
- `src/components/URLBar/index.js`
- `src/__tests__/components/URLBar.test.js`

#### 완료 기준
- URLBar 컴포넌트가 정상 렌더링
- 포커스 시 VirtualKeyboard 자동 표시
- 키보드로 문자 입력 가능
- 확인 버튼으로 URL 제출 가능 (검증 통과 시)
- 단위 테스트 통과
- requirements.md의 FR-1, FR-4, FR-5 요구사항 충족

---

### Phase 4: BrowserView 통합
**담당**: frontend-dev
**예상 시간**: 45분
**의존성**: Phase 3 완료

#### 작업 내용
- [ ] Task 4-1: BrowserView에 URLBar 통합
  - `src/views/BrowserView.js` 수정
  - URLBar 컴포넌트 import
  - URL 상태 관리 추가
    - `const [currentUrl, setCurrentUrl] = useState('https://www.google.com')`
    - `const [inputValue, setInputValue] = useState(currentUrl)`
  - URLBar 이벤트 핸들러 구현
    - `handleUrlChange(newUrl)`: setInputValue(newUrl)
    - `handleUrlSubmit(validatedUrl)`: setCurrentUrl(validatedUrl), setInputValue(validatedUrl), logger.info
    - `handleSuggestionSelect(suggestion)`: (준비만, F-07/F-08 연동 시 활성화)
  - 자동완성 Hook 준비 (비활성화)
    - `const suggestions = []` (F-07, F-08 완료 후 useAutocompleteSuggestions로 교체)
  - URLBar 렌더링
    - Header 아래에 URLBar 컴포넌트 추가
    - Props 전달: value, onChange, onSubmit, suggestions, onSuggestionSelect
  - WebView에 currentUrl Props 전달
    - `<WebView url={currentUrl} />`
- [ ] Task 4-2: BrowserView 스타일 조정
  - `src/views/BrowserView.module.less` 수정
  - `.urlBarPlaceholder` 제거
  - URLBar 영역 스타일 조정 (필요 시)
- [ ] Task 4-3: 백 버튼 이벤트 캡처
  - BrowserView에 `handleKeyDown` 함수 추가
  - 백 버튼 (keyCode 8 또는 461) 감지
  - showKeyboard=true일 때만 event.preventDefault(), setShowKeyboard(false)
  - Panel 컴포넌트에 `onKeyDown={handleKeyDown}` 추가

#### 예상 산출물
- `src/views/BrowserView.js` (수정)
- `src/views/BrowserView.module.less` (수정)

#### 완료 기준
- BrowserView에서 URLBar 정상 렌더링
- URL 입력 후 WebView에 URL 전달 확인 (로그)
- 백 버튼으로 키보드 닫기 동작 확인
- requirements.md의 FR-5 (페이지 로드) 요구사항 충족

---

### Phase 5: Spotlight 통합 및 테스트
**담당**: frontend-dev
**예상 시간**: 1시간
**의존성**: Phase 4 완료

#### 작업 내용
- [ ] Task 5-1: URLBar ↔ VirtualKeyboard 포커스 전환 구현
  - URLBar에서 방향키 아래 감지 시 Spotlight.focus('virtual-keyboard') 호출
  - VirtualKeyboard의 SpotlightContainer 설정 확인
    - enterTo: 'default-element'
    - defaultElement: '[data-key="q"]'
    - restrict: 'self-only'
  - 키보드에서 백 버튼 동작 확인
- [ ] Task 5-2: 로컬 브라우저 테스트
  - `npm run serve` 실행
  - 테스트 시나리오:
    - 탭 키로 URL 입력 필드에 포커스 이동
    - 포커스 시 가상 키보드 자동 표시 확인
    - 탭 키로 키보드 내 키 간 포커스 이동 확인
    - 엔터 키로 문자 입력 확인 (탭 키는 포커스 이동, 엔터는 선택)
    - URL 입력 후 확인 버튼 클릭 → WebView URL 변경 확인 (개발자 도구 로그)
    - 백스페이스 키로 문자 삭제 확인
    - ESC 키로 키보드 닫기 확인
  - 브라우저 콘솔에서 에러 없음 확인
- [ ] Task 5-3: URL 검증 테스트
  - 다양한 URL 입력 테스트:
    - `google.com` → `http://google.com` 자동 보완 확인
    - `https://www.youtube.com` → 프로토콜 유지 확인
    - `localhost:8080` → `http://localhost:8080` 변환 확인
    - `invalid` → 에러 로그 확인 (제출 안 됨)
  - 로그에서 검증 결과 확인 (logger.info, logger.warn)

#### 예상 산출물
- 없음 (테스트 단계)

#### 완료 기준
- 로컬 브라우저에서 모든 테스트 시나리오 통과
- Spotlight 포커스 이동이 직관적이고 예측 가능
- URL 검증 및 자동 보완 정상 동작
- requirements.md의 FR-6 (리모컨 키 매핑) 요구사항 충족 (로컬에서 확인)

---

### Phase 6: 실제 디바이스 테스트
**담당**: frontend-dev
**예상 시간**: 1시간
**의존성**: Phase 5 완료

#### 작업 내용
- [ ] Task 6-1: 프로젝터 빌드 및 배포
  - `npm run pack-p` 실행
  - `ares-package dist/` 실행
  - `ares-install --device projector {ipk파일}` 실행
  - `ares-launch --device projector com.jsong.webosbrowser` 실행
- [ ] Task 6-2: 리모컨 조작 테스트
  - 리모컨 방향키로 URL 필드 포커스 이동
  - URL 필드 선택 시 가상 키보드 표시 확인
  - 리모컨 방향키로 키보드 내 키 간 포커스 이동 확인 (상하좌우)
  - 선택 버튼으로 문자 입력 확인
  - 백스페이스 키로 문자 삭제 확인
  - 스페이스 키로 공백 입력 확인
  - 확인 버튼으로 URL 제출 확인
  - 취소 버튼으로 키보드 닫기 및 값 복원 확인
  - 백 버튼으로 키보드 닫기 확인
- [ ] Task 6-3: 주요 사이트 URL 입력 테스트
  - `google.com` 입력 → `http://google.com` 자동 보완 → 페이지 로드 확인
  - `https://www.youtube.com` 입력 → 페이지 로드 확인
  - `naver.com` 입력 → 페이지 로드 확인
  - 잘못된 URL 입력 → 에러 로그 확인 (ares-log로 확인)
- [ ] Task 6-4: 성능 테스트
  - 리모컨 버튼 입력 후 UI 반응 속도 측정 (요구사항: 0.3초 이내)
  - 가상 키보드 렌더링 시간 측정 (요구사항: 1초 이내)
  - 메모리 사용량 확인 (요구사항: 키보드 포함 50MB 이하)
- [ ] Task 6-5: 가독성 테스트
  - 3m 거리에서 폰트 크기 가독성 확인 (URL 필드 18px, 키보드 키 20px)
  - 포커스된 요소가 명확하게 구분되는지 확인 (테두리 3px)
  - 대비 비율 확인 (배경과 전경 색상)
- [ ] Task 6-6: 로그 확인 및 디버깅
  - `ares-log --device projector -f com.jsong.webosbrowser` 실행
  - 에러 로그 없음 확인
  - URL 제출 로그 확인 ("[BrowserView] URL 제출: ...")
  - 키보드 입력 로그 확인 ("[VirtualKeyboard] 문자 입력: ...")

#### 예상 산출물
- 없음 (테스트 단계)

#### 완료 기준
- 리모컨으로 모든 기능 정상 동작
- 주요 웹사이트 URL 입력 및 로드 성공
- 성능 요구사항 충족 (입력 반응 0.3초, 키보드 렌더링 1초)
- 가독성 요구사항 충족 (3m 거리에서 명확)
- requirements.md의 모든 AC (AC-1~AC-7) 통과

---

### Phase 7: AutocompleteDropdown 컴포넌트 (선택적, F-07/F-08 완료 후)
**담당**: frontend-dev
**예상 시간**: 1시간
**의존성**: F-07 (북마크 관리), F-08 (히스토리 관리) 완료

#### 작업 내용
- [ ] Task 7-1: AutocompleteDropdown 컴포넌트 구현
  - `src/components/URLBar/AutocompleteDropdown.js` 생성
  - Props 인터페이스 정의
    - `suggestions`: 제안 목록 (url, title, favicon)
    - `selectedIndex`: 현재 선택된 인덱스
    - `onSelect`: 항목 선택 콜백
    - `visible`: 표시 여부
    - `className`: 스타일 커스터마이징
  - PropTypes 선언 및 defaultProps 설정
  - Enact VirtualList 사용
    - `dataSize={suggestions.length}`
    - `itemRenderer`: 제안 항목 렌더링 함수
    - `itemSize={60}`: 각 항목 높이 60px
  - 제안 항목 렌더링 구조
    - favicon (있을 경우), title, url 표시
    - 선택된 항목은 배경색 강조 (isSelected)
  - 조건부 렌더링
    - visible=false 또는 suggestions.length=0이면 null 반환
- [ ] Task 7-2: AutocompleteDropdown 스타일 구현
  - `AutocompleteDropdown.module.less` 생성 (또는 URLBar.module.less에 추가)
  - 드롭다운 스타일
    - 최대 높이 300px (5개 항목)
    - 배경색 #3a3a3a, border-radius 8px
  - 제안 항목 스타일
    - 높이 60px, flexbox 정렬
    - 선택 시 배경색 #4a4a4a
    - favicon 24x24px, 마진 8px
    - title 폰트 16px, url 폰트 14px (회색)
    - 말줄임(...) 처리
- [ ] Task 7-3: URLBar에 AutocompleteDropdown 통합
  - URLBar.js 수정
  - AutocompleteDropdown import
  - URLBar 렌더링 구조에 AutocompleteDropdown 추가
  - Props 전달: suggestions, selectedIndex, onSelect, visible=showSuggestions
  - `handleSuggestionSelect(suggestion)` 함수 구현
    - inputValue 업데이트
    - onSuggestionSelect 콜백 호출
    - showSuggestions=false
  - 방향키 아래로 제안 리스트 포커스 이동 구현
    - selectedIndex 증가/감소
  - 방향키 위로 URL 입력 필드 복귀 구현

#### 예상 산출물
- `src/components/URLBar/AutocompleteDropdown.js`
- `src/components/URLBar/URLBar.js` (수정)
- `src/components/URLBar/URLBar.module.less` (수정)

#### 완료 기준
- 자동완성 드롭다운 정상 렌더링
- 제안 목록이 올바르게 표시 (title, url, favicon)
- 방향키로 제안 리스트 탐색 가능
- 선택 버튼으로 제안 항목 선택 시 URL 필드에 채워짐
- requirements.md의 FR-3 (URL 자동완성) 요구사항 충족

---

### Phase 8: 자동완성 데이터 통합 (F-07/F-08 완료 후)
**담당**: frontend-dev
**예상 시간**: 45분
**의존성**: F-07 (북마크 관리), F-08 (히스토리 관리), Phase 7 완료

#### 작업 내용
- [ ] Task 8-1: useAutocompleteSuggestions Hook 구현
  - `src/hooks/useAutocompleteSuggestions.js` 생성
  - Hook 인터페이스 정의
    - `query`: 검색 쿼리 (string)
    - `maxResults`: 최대 결과 수 (기본값: 5)
    - 반환값: `suggestions` 배열
  - useEffect로 query 변경 감지
    - query 길이 2자 미만이면 빈 배열 반환
    - historyService.searchHistory(query, maxResults) 호출 (F-08)
    - bookmarkService.searchBookmarks(query, maxResults) 호출 (F-07)
    - 히스토리 + 북마크 통합 및 중복 제거
    - 상위 N개만 반환
  - JSDoc 주석 작성
- [ ] Task 8-2: BrowserView에 Hook 통합
  - BrowserView.js 수정
  - useAutocompleteSuggestions Hook import
  - `const suggestions = useAutocompleteSuggestions(inputValue, 5)` 호출
  - URLBar에 suggestions Props 전달
  - `handleSuggestionSelect` 함수 구현
    - setCurrentUrl(suggestion.url)
    - setInputValue(suggestion.url)
    - logger.info
- [ ] Task 8-3: 자동완성 성능 테스트
  - 히스토리 + 북마크 100개 데이터로 테스트
  - 문자 입력 후 0.5초 이내 제안 표시 확인
  - 메모리 사용량 확인

#### 예상 산출물
- `src/hooks/useAutocompleteSuggestions.js`
- `src/views/BrowserView.js` (수정)

#### 완료 기준
- 문자 입력 시 히스토리 + 북마크 제안 정상 표시
- 자동완성 성능 요구사항 충족 (0.5초 이내)
- requirements.md의 AC-5 (자동완성) 통과

---

### Phase 9: 검색 엔진 통합 (F-09 완료 후)
**담당**: frontend-dev
**예상 시간**: 30분
**의존성**: F-09 (검색 엔진 통합) 완료

#### 작업 내용
- [ ] Task 9-1: searchService.js 생성 (또는 F-09에서 생성된 파일 사용)
  - `src/services/searchService.js` 생성 (F-09에서 생성되었으면 스킵)
  - `buildSearchUrl(query, engine)` 함수 구현
    - 검색 엔진별 URL 생성 (google, naver, bing, duckduckgo)
    - encodeURIComponent로 쿼리 인코딩
  - JSDoc 주석 작성
- [ ] Task 9-2: urlValidator.js에 검색 엔진 통합
  - urlValidator.js 수정
  - `validateAndFormatUrl` 함수에서 검색어 감지 로직 추가
    - 도메인 형식이 아니고, 프로토콜도 없으면 검색어로 판단
    - `buildSearchUrl(input, 'google')` 호출하여 검색 URL 생성
    - 생성된 URL 반환
  - 단위 테스트 업데이트
    - 검색어 입력 시 검색 URL 생성 확인
- [ ] Task 9-3: BrowserView에 검색 엔진 설정 연동
  - 설정에서 기본 검색 엔진 선택 기능 연동 (F-11 설정 화면과 연계)
  - 현재는 하드코딩 ('google') 사용

#### 예상 산출물
- `src/services/searchService.js` (또는 수정)
- `src/utils/urlValidator.js` (수정)
- `src/__tests__/utils/urlValidator.test.js` (수정)

#### 완료 기준
- 검색어 입력 시 검색 엔진 URL로 자동 변환
- 예: "webos browser" → "https://www.google.com/search?q=webos%20browser"
- requirements.md의 FR-4 (URL 검증 및 자동 보완) 완전 충족

---

### Phase 10: 최종 테스트 및 문서화
**담당**: frontend-dev (구현), doc-writer (문서)
**예상 시간**: 1.5시간
**의존성**: Phase 9 완료 (또는 Phase 6 이후 진행 가능)

#### 작업 내용
- [ ] Task 10-1: 회귀 테스트 (frontend-dev)
  - requirements.md의 AC-1~AC-8 모두 검증
  - AC-1: URL 입력 필드 렌더링
  - AC-2: 가상 키보드 동작
  - AC-3: URL 유효성 검증 및 자동 보완
  - AC-4: 페이지 로드
  - AC-5: 자동완성 (F-07, F-08 연동 후)
  - AC-6: 리모컨 키 매핑
  - AC-7: 성능 및 가독성
  - AC-8: 회귀 테스트 (긴 URL, 특수문자)
  - 테스트 결과 문서화 (체크리스트)
- [ ] Task 10-2: 성능 테스트 (frontend-dev)
  - 입력 반응 속도: 리모컨 버튼 입력 후 0.3초 이내 확인
  - 키보드 렌더링 시간: 1초 이내 확인
  - 자동완성 검색 속도: 0.5초 이내 확인
  - 메모리 사용량: 50MB 이하 확인
  - 성능 측정 결과 로그에 기록
- [ ] Task 10-3: 컴포넌트 문서 작성 (doc-writer)
  - `docs/components/URLBar.md` 생성
    - Props 인터페이스
    - 사용 예시 코드
    - 리모컨 키 매핑
    - 상태 관리 방법
    - 주의사항 (포커스 전환, URL 검증 등)
  - `docs/components/VirtualKeyboard.md` 생성
    - Props 인터페이스
    - 키보드 레이아웃 설명
    - 사용 예시 코드
    - 리모컨 키 매핑
    - Spotlight 통합 방법
    - 주의사항 (포커스 경로, 키보드 경계 등)
- [ ] Task 10-4: CHANGELOG 업데이트 (doc-writer)
  - CHANGELOG.md에 F-03 URL 입력 UI 항목 추가
  - 주요 변경 사항:
    - URLBar 컴포넌트 추가
    - VirtualKeyboard 컴포넌트 추가
    - URL 검증 및 자동 보완 기능
    - 리모컨 최적화 가상 키보드
    - (F-07, F-08 연동 시) URL 자동완성 기능

#### 예상 산출물
- 회귀 테스트 체크리스트 (문서)
- 성능 테스트 결과 (로그 또는 문서)
- `docs/components/URLBar.md`
- `docs/components/VirtualKeyboard.md`
- `CHANGELOG.md` (수정)

#### 완료 기준
- requirements.md의 모든 AC 통과
- 성능 및 비기능 요구사항 충족
- 컴포넌트 문서 작성 완료
- CHANGELOG 업데이트 완료

---

## 3. 태스크 의존성 다이어그램

```
Phase 1: urlValidator.js
    │
    │ (병렬)
    ▼
Phase 2: VirtualKeyboard
    │
    ├───────────────┐
    │               │
    ▼               ▼
Phase 3: URLBar ──▶ Phase 4: BrowserView 통합
                        │
                        ▼
                    Phase 5: Spotlight 통합 및 테스트
                        │
                        ▼
                    Phase 6: 실제 디바이스 테스트
                        │
                        │ (선택적, F-07/F-08 완료 후)
                        ▼
                    Phase 7: AutocompleteDropdown
                        │
                        ▼
                    Phase 8: 자동완성 데이터 통합
                        │
                        │ (선택적, F-09 완료 후)
                        ▼
                    Phase 9: 검색 엔진 통합
                        │
                        ▼
                    Phase 10: 최종 테스트 및 문서화
```

### 병렬 실행 가능 태스크
- **Phase 1 (urlValidator.js)** 와 **Phase 2 (VirtualKeyboard)** 는 독립적이므로 병렬 실행 가능
- Phase 3 이후는 순차 실행 필수

---

## 4. 병렬 실행 판단

### Agent Team 사용 권장: **No (단독 실행 권장)**

#### 이유
1. **컴포넌트 강한 결합**:
   - URLBar와 VirtualKeyboard는 서로 긴밀하게 통합됨 (포커스 전환, 이벤트 전달)
   - 한 에이전트가 전체 흐름을 이해하고 구현하는 것이 효율적
2. **상태 관리 공유**:
   - URLBar의 inputValue가 VirtualKeyboard에서 변경됨
   - 두 컴포넌트 간 인터페이스 조율 필요 → 단독 작업이 더 빠름
3. **프로젝트 규모**:
   - 총 예상 시간: 약 8-10시간 (Phase 1-6 기준)
   - 병렬화 이득 < 조율 비용
4. **테스트 단계 의존성**:
   - Phase 5, 6은 모든 컴포넌트가 완성되어야 진행 가능
   - 병렬 작업 후 통합 시점에 디버깅 비용 증가 가능

#### 병렬 작업이 유리한 경우 (선택적)
- **Phase 1 (urlValidator.js)** 와 **Phase 2 (VirtualKeyboard)** 만 병렬 실행 가능
  - 두 작업은 완전 독립적 (공유 의존성 없음)
  - Agent Team으로 1시간 절약 가능 (2.5시간 → 1.5시간)
- 하지만 총 작업 시간에 비해 절감 효과가 크지 않으므로 **단독 실행 권장**

---

## 5. 리스크 및 대응 방안

| 리스크 | 영향도 | 발생 확률 | 대응 방안 |
|--------|--------|----------|-----------|
| **Spotlight 포커스 충돌** | 높음 | 중간 | - VirtualKeyboard를 SpotlightContainer로 격리 (`restrict: 'self-only'`)<br>- URLBar에서 명시적으로 Spotlight.focus() 호출<br>- 로컬 브라우저에서 탭 키로 사전 테스트 |
| **리모컨 백 버튼 이벤트 처리 실패** | 중간 | 낮음 | - BrowserView에서 onKeyDown으로 백 버튼 캡처<br>- event.preventDefault()로 기본 동작 방지<br>- 실제 디바이스에서 keyCode 확인 (8 또는 461) |
| **URL 검증 로직 예외 케이스** | 중간 | 중간 | - 단위 테스트에서 다양한 URL 형식 커버 (프로토콜, 경로, 쿼리, 특수문자)<br>- localhost, IP 주소 별도 처리<br>- 검색어 판단 기준 명확화 (점 유무) |
| **가상 키보드 성능 저하** | 중간 | 낮음 | - 키 렌더링에 React.memo 적용 (필요 시)<br>- Spotlight 포커스 이동 최적화<br>- 실제 디바이스에서 성능 테스트 |
| **Enact Input 기본 동작과 충돌** | 낮음 | 중간 | - VirtualKeyboard에서 Input의 value를 직접 제어<br>- onChange 이벤트를 VirtualKeyboard 입력 시에만 발생하도록 제어<br>- readOnly 속성 사용 안 함 (포커스 표시 필요) |
| **자동완성 데이터 통합 지연** | 낮음 | 중간 | - F-07, F-08 완료 전까지는 suggestions=[] 로 비활성화<br>- AutocompleteDropdown은 Phase 7에서 구현 (선택적)<br>- 기본 URL 입력 기능은 자동완성 없이도 동작 |
| **검색 엔진 통합 지연** | 낮음 | 중간 | - F-09 완료 전까지는 검색어 입력 시 null 반환 (에러 처리)<br>- Phase 9는 선택적으로 나중에 진행 가능<br>- 기본 URL 입력 기능은 검색 엔진 없이도 동작 |
| **실제 디바이스 테스트 실패** | 높음 | 낮음 | - 로컬 브라우저에서 사전 테스트 충분히 진행 (Phase 5)<br>- 리모컨 키 매핑 문서 확인<br>- ares-log로 실시간 로그 확인하며 디버깅 |

---

## 6. 검증 계획

### 단위 테스트 (Jest + Enact Testing Library)
- **urlValidator.js**: 모든 URL 형식 테스트 (프로토콜, 도메인, localhost, 특수문자)
- **VirtualKeyboard.js**: 렌더링, 키 클릭, 콜백 호출 테스트
- **URLBar.js**: 렌더링, 포커스, 키보드 입력, 제출, 취소 테스트
- **커버리지 목표**: 80% 이상

### 통합 테스트 (로컬 브라우저)
- URLBar ↔ VirtualKeyboard 포커스 전환
- URL 입력 → 검증 → WebView 전달 흐름
- 백 버튼으로 키보드 닫기

### 실제 디바이스 테스트 (webOS 프로젝터)
- 리모컨 조작 시나리오 (requirements.md AC-1~AC-8)
- 주요 웹사이트 URL 입력 및 로드
- 성능 측정 (입력 반응 속도, 키보드 렌더링 시간)
- 가독성 확인 (3m 거리)

### 회귀 테스트
- 긴 URL 입력 시 말줄임(...) 처리
- 특수문자 포함 URL 정상 입력 및 로드
- 유효하지 않은 URL 입력 시 에러 메시지 표시

---

## 7. 예상 산출물 요약

### 소스 코드
- `src/utils/urlValidator.js`
- `src/components/VirtualKeyboard/keyboardLayout.js`
- `src/components/VirtualKeyboard/VirtualKeyboard.js`
- `src/components/VirtualKeyboard/VirtualKeyboard.module.less`
- `src/components/VirtualKeyboard/index.js`
- `src/components/URLBar/URLBar.js`
- `src/components/URLBar/URLBar.module.less`
- `src/components/URLBar/index.js`
- `src/views/BrowserView.js` (수정)
- `src/views/BrowserView.module.less` (수정)

### 테스트 코드
- `src/__tests__/utils/urlValidator.test.js`
- `src/__tests__/components/VirtualKeyboard.test.js`
- `src/__tests__/components/URLBar.test.js`

### 선택적 (F-07, F-08, F-09 연동 시)
- `src/components/URLBar/AutocompleteDropdown.js`
- `src/hooks/useAutocompleteSuggestions.js`
- `src/services/searchService.js`

### 문서
- `docs/components/URLBar.md`
- `docs/components/VirtualKeyboard.md`
- `CHANGELOG.md` (수정)

---

## 8. 예상 소요 시간

| Phase | 예상 시간 |
|-------|----------|
| Phase 1: urlValidator.js | 30분 |
| Phase 2: VirtualKeyboard | 2시간 |
| Phase 3: URLBar (기본 버전) | 1.5시간 |
| Phase 4: BrowserView 통합 | 45분 |
| Phase 5: Spotlight 통합 및 테스트 | 1시간 |
| Phase 6: 실제 디바이스 테스트 | 1시간 |
| **Phase 1-6 합계 (기본 기능)** | **약 6.75시간** |
| Phase 7: AutocompleteDropdown (선택적) | 1시간 |
| Phase 8: 자동완성 데이터 통합 (선택적) | 45분 |
| Phase 9: 검색 엔진 통합 (선택적) | 30분 |
| Phase 10: 최종 테스트 및 문서화 | 1.5시간 |
| **전체 합계 (모든 기능)** | **약 10.5시간** |

### 우선순위
- **Must (Phase 1-6)**: 기본 URL 입력 및 페이지 로드 기능 → **6.75시간**
- **Should (Phase 7-8)**: 자동완성 기능 (F-07, F-08 완료 후) → **+1.75시간**
- **Could (Phase 9)**: 검색 엔진 통합 (F-09 완료 후) → **+0.5시간**
- **문서화 (Phase 10)**: 최종 테스트 및 문서 → **+1.5시간**

---

## 9. 최종 확인 사항

### 완료 기준 체크리스트
- [ ] URLBar 컴포넌트 구현 완료
- [ ] VirtualKeyboard 컴포넌트 구현 완료
- [ ] URL 검증 및 자동 보완 기능 구현 완료
- [ ] BrowserView에 통합 완료
- [ ] Spotlight 포커스 관리 정상 동작
- [ ] 리모컨 키 매핑 정상 동작 (실제 디바이스)
- [ ] 주요 웹사이트 URL 입력 및 로드 성공
- [ ] 성능 요구사항 충족 (입력 반응 0.3초, 키보드 렌더링 1초)
- [ ] 가독성 요구사항 충족 (3m 거리, 폰트 크기)
- [ ] 단위 테스트 통과 (80% 이상 커버리지)
- [ ] requirements.md의 모든 AC 통과
- [ ] 컴포넌트 문서 작성 완료
- [ ] CHANGELOG 업데이트 완료

### 다음 단계
- F-04 (페이지 탐색 UI): NavigationBar와 URLBar 간 포커스 이동 경로 조정
- F-07 (북마크 관리): AutocompleteDropdown 데이터 제공
- F-08 (히스토리 관리): AutocompleteDropdown 데이터 제공
- F-09 (검색 엔진 통합): URL이 아닌 검색어 입력 시 검색 엔진으로 쿼리

---

## 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-12 | 최초 작성 | product-manager |
