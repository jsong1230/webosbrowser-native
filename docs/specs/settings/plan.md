# 설정 화면 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: `docs/specs/settings/requirements.md`
- 기술 설계서: `docs/specs/settings/design.md`
- 관련 기능: F-09 (검색 엔진 통합), F-07 (북마크 관리), F-08 (히스토리 관리), F-12 (다운로드 관리)

---

## 2. 구현 Phase

### Phase 1: 서비스 레이어 구현
**목표**: 설정 데이터 관리 및 브라우징 데이터 삭제 로직 구현

**태스크**:
- [ ] **Task 1-1**: settingsService.js 기본 구조 생성 (1시간)
  - 담당: frontend-dev
  - 설명: LocalStorage 기반 설정 CRUD 함수 구현
  - 산출물:
    - `src/services/settingsService.js`
  - 세부 작업:
    - `DEFAULT_SETTINGS` 상수 정의
    - `loadSettings()` 함수 (LocalStorage 읽기 + 기본값 병합)
    - `saveSettings()` 함수 (LocalStorage 쓰기)
    - URL 유효성 검증 함수 `isValidUrl()`
  - 완료 기준:
    - LocalStorage에 설정 JSON 저장/읽기 정상 동작
    - 저장 실패 시 기본값 반환
    - logger로 에러 기록

- [ ] **Task 1-2**: settingsService.js 설정 변경 함수 구현 (1시간)
  - 담당: frontend-dev
  - 의존성: Task 1-1 완료
  - 설명: 검색 엔진, 홈페이지 URL, 테마 설정 함수
  - 산출물:
    - `settingsService.js` 업데이트
  - 세부 작업:
    - `setSearchEngine(engineId)` — searchService 래퍼
    - `setHomepageUrl(url)` — URL 검증 + 저장
    - `setTheme(theme)` — 테마 저장 (light/dark)
    - `getHomepageUrl()` — 홈페이지 URL 조회
    - `getTheme()` — 테마 조회
  - 완료 기준:
    - 각 함수 호출 시 LocalStorage에 즉시 반영
    - 유효하지 않은 입력 시 false 반환 + 로그

- [ ] **Task 1-3**: settingsService.js 브라우징 데이터 삭제 함수 구현 (1.5시간)
  - 담당: frontend-dev
  - 의존성: Task 1-2 완료, F-07/F-08/F-12 구현 완료
  - 설명: 히스토리, 북마크, 쿠키/캐시, 다운로드 삭제 오케스트레이션
  - 산출물:
    - `settingsService.js` 업데이트
  - 세부 작업:
    - `deleteBrowsingData(targets)` 비동기 함수 구현
    - 히스토리 삭제: `historyService.deleteAllHistory()` 호출
    - 북마크 삭제: 모든 북마크 순회하며 `bookmarkService.deleteBookmark()` 호출
    - 다운로드 삭제: 모든 다운로드 순회하며 `downloadService.deleteDownload()` 호출
    - 쿠키/캐시 삭제: WebView API 호출 (TODO로 남김, Phase 4에서 구현)
    - 삭제 결과 객체 반환 (`{ deletedCount, errors }`)
  - 완료 기준:
    - 선택된 데이터만 삭제 (체크박스 반영)
    - 삭제 실패 시 에러 메시지 포함하여 반환
    - 비동기 처리로 UI 블로킹 방지

---

### Phase 2: UI 컴포넌트 구현
**목표**: SettingsPanel 및 하위 컴포넌트 구현

**태스크**:
- [ ] **Task 2-1**: SettingItem 컴포넌트 구현 (1시간)
  - 담당: frontend-dev
  - 의존성: Task 1-1 완료
  - 설명: 재사용 가능한 설정 항목 레이아웃 컴포넌트
  - 산출물:
    - `src/components/SettingsPanel/SettingItem.js`
    - `src/components/SettingsPanel/SettingItem.module.less`
  - 세부 작업:
    - Props: `label`, `description`, `children`
    - 레이아웃: 레이블 + 설명 + 컨트롤 (children)
    - 스타일: Moonstone 폰트 크기 (레이블 20px, 설명 16px)
    - Spotlightable 포커스 지원
  - 완료 기준:
    - Dropdown, Input, Button 등 자식 컴포넌트 렌더링 가능
    - 대화면에서 가독성 확보 (3m 거리 테스트)

- [ ] **Task 2-2**: SettingsPanel 기본 UI 구조 구현 (1.5시간)
  - 담당: frontend-dev
  - 의존성: Task 2-1 완료
  - 설명: Panel 컴포넌트 구조 및 기본 레이아웃
  - 산출물:
    - `src/components/SettingsPanel/SettingsPanel.js`
    - `src/components/SettingsPanel/SettingsPanel.module.less`
    - `src/components/SettingsPanel/index.js` (export)
  - 세부 작업:
    - Enact Panel 컴포넌트 사용 (BookmarkPanel 패턴 참조)
    - Header: "설정" 제목
    - Scroller: 설정 항목 스크롤 영역
    - Props: `visible`, `onClose`
    - 상태: `searchEngine`, `homepageUrl`, `theme`, `deleteTargets`, `showConfirmDialog`, `homepageInputError`
    - 초기화: `useEffect`로 settingsService에서 설정 로드
  - 완료 기준:
    - Panel 열림/닫힘 정상 동작
    - Spotlight 포커스 관리 (Panel 열릴 때 첫 번째 항목 자동 포커스)
    - Back 버튼으로 패널 닫기

- [ ] **Task 2-3**: 검색 엔진 선택 항목 구현 (1시간)
  - 담당: frontend-dev
  - 의존성: Task 2-2 완료
  - 설명: 검색 엔진 드롭다운 및 변경 로직
  - 산출물:
    - `SettingsPanel.js` 업데이트
  - 세부 작업:
    - SettingItem + Enact Dropdown (Picker) 사용
    - 옵션: Google, Naver, DuckDuckGo, Bing
    - `handleSearchEngineChange()` 핸들러
    - 변경 시 즉시 `settingsService.setSearchEngine()` 호출
  - 완료 기준:
    - 드롭다운 선택 시 즉시 저장
    - URLBar에서 검색 시 변경된 엔진 사용 확인 (통합 테스트)

- [ ] **Task 2-4**: 홈페이지 URL 설정 항목 구현 (1.5시간)
  - 담당: frontend-dev
  - 의존성: Task 2-3 완료
  - 설명: 홈페이지 URL 입력 필드 및 저장 로직
  - 산출물:
    - `SettingsPanel.js` 업데이트
  - 세부 작업:
    - SettingItem + Enact Input (가상 키보드 지원)
    - 저장 버튼 (Enact Button)
    - `handleHomepageChange()` 핸들러 (입력 값 업데이트)
    - `handleHomepageSave()` 핸들러 (유효성 검증 + 저장)
    - 유효하지 않은 URL 입력 시 에러 메시지 표시 (`homepageInputError` 상태)
  - 완료 기준:
    - 유효한 URL 저장 시 NavigationBar 홈 버튼 동작 변경 확인
    - 유효하지 않은 URL 입력 시 "유효한 URL을 입력하세요" 메시지 표시
    - 에러 메시지 빨간색 표시

- [ ] **Task 2-5**: 테마 선택 항목 구현 (1시간)
  - 담당: frontend-dev
  - 의존성: Task 2-4 완료
  - 설명: 테마 드롭다운 및 변경 로직
  - 산출물:
    - `SettingsPanel.js` 업데이트
  - 세부 작업:
    - SettingItem + Enact Dropdown (Picker)
    - 옵션: Light, Dark
    - `handleThemeChange()` 핸들러
    - 변경 시 `settingsService.setTheme()` 호출 + `onThemeChange` 콜백 호출
  - 완료 기준:
    - 테마 선택 시 즉시 UI 색상 변경 확인
    - App 컴포넌트 MoonstoneDecorator skin prop 업데이트 확인

- [ ] **Task 2-6**: 브라우징 데이터 삭제 항목 구현 (2시간)
  - 담당: frontend-dev
  - 의존성: Task 2-5 완료
  - 설명: 삭제 대상 체크박스 및 삭제 로직
  - 산출물:
    - `SettingsPanel.js` 업데이트
  - 세부 작업:
    - SettingItem + Enact CheckboxItem (4개: 히스토리, 북마크, 쿠키/캐시, 다운로드)
    - 삭제 버튼 (Enact Button)
    - `handleDeleteTargetToggle()` 핸들러 (체크박스 상태 변경)
    - `handleDeleteClick()` 핸들러 (확인 다이얼로그 표시)
    - `handleDeleteConfirm()` 핸들러 (실제 삭제 수행)
    - 삭제 중 로딩 스피너 표시 (LoadingSpinner 컴포넌트 재사용)
    - 삭제 완료 후 토스트 메시지 표시 (Enact Alert)
  - 완료 기준:
    - 체크박스 선택 시 삭제 대상 상태 변경
    - 삭제 버튼 클릭 시 확인 다이얼로그 표시
    - 확인 후 선택된 데이터만 삭제
    - 삭제 완료 시 "브라우징 데이터가 삭제되었습니다" 토스트 표시
    - 히스토리/북마크 패널에서 데이터 사라짐 확인 (통합 테스트)

- [ ] **Task 2-7**: 확인 다이얼로그 구현 (0.5시간)
  - 담당: frontend-dev
  - 의존성: Task 2-6과 병렬 가능
  - 설명: 브라우징 데이터 삭제 확인 다이얼로그
  - 산출물:
    - `SettingsPanel.js` 업데이트 (Enact Popup 사용)
  - 세부 작업:
    - Enact Popup 컴포넌트 사용
    - 제목: "브라우징 데이터 삭제"
    - 내용: "선택한 데이터를 삭제하시겠습니까?"
    - 버튼: "삭제", "취소"
    - `showConfirmDialog` 상태로 표시 여부 제어
  - 완료 기준:
    - 삭제 버튼 클릭 시 다이얼로그 표시
    - 취소 버튼 클릭 시 다이얼로그 닫기
    - 삭제 버튼 클릭 시 `handleDeleteConfirm()` 호출

---

### Phase 3: BrowserView 및 App 통합
**목표**: 설정 패널을 BrowserView에 통합하고 테마를 App에 적용

**태스크**:
- [ ] **Task 3-1**: NavigationBar에 설정 버튼 추가 (0.5시간)
  - 담당: frontend-dev
  - 의존성: Phase 2 완료
  - 설명: 설정 버튼 (톱니바퀴 아이콘) 추가
  - 산출물:
    - `src/components/NavigationBar/NavigationBar.js` 수정
  - 세부 작업:
    - IconButton 추가 (icon="gear")
    - `onSettingsClick` prop 추가
    - Spotlight ID: "nav-settings"
  - 완료 기준:
    - 설정 버튼 클릭 시 `onSettingsClick` 콜백 호출
    - 리모컨 포커스 정상 동작

- [ ] **Task 3-2**: BrowserView에 SettingsPanel 통합 (1시간)
  - 담당: frontend-dev
  - 의존성: Task 3-1 완료
  - 설명: BrowserView에서 SettingsPanel 열기/닫기 및 이벤트 처리
  - 산출물:
    - `src/views/BrowserView.js` 수정
  - 세부 작업:
    - `showSettingsPanel` 상태 추가
    - `handleSettingsClick()` 핸들러 (패널 열기)
    - `handleSettingsPanelClose()` 핸들러 (패널 닫기)
    - `handleThemeChange(newTheme)` 핸들러 (테마 변경 → CustomEvent 발생)
    - `handleHomepageChange(newUrl)` 핸들러 (homeUrl 상태 업데이트)
    - SettingsPanel 컴포넌트 렌더링 (props: visible, onClose, onThemeChange, onHomepageChange)
  - 완료 기준:
    - NavigationBar 설정 버튼 클릭 시 SettingsPanel 열림
    - SettingsPanel 닫기 버튼/Back 버튼으로 패널 닫힘
    - 테마 변경 시 CustomEvent 발생 확인 (브라우저 콘솔)

- [ ] **Task 3-3**: App.js에 테마 적용 로직 추가 (1시간)
  - 담당: frontend-dev
  - 의존성: Task 3-2 완료
  - 설명: MoonstoneDecorator에 동적 skin prop 전달
  - 산출물:
    - `src/App/App.js` 수정
  - 세부 작업:
    - `skin` 상태 추가
    - `useEffect`로 초기 테마 로드 (`settingsService.getTheme()`)
    - `themechange` CustomEvent 리스너 등록
    - 리스너에서 `setSkin(event.detail.theme)` 호출
    - MoonstoneDecorator에 `skin` prop 전달 (`export default MoonstoneDecorator({ skin })(App)`)
  - 완료 기준:
    - 앱 시작 시 저장된 테마 적용
    - SettingsPanel에서 테마 변경 시 전체 UI 색상 즉시 변경
    - 화면 깜빡임 없이 부드럽게 전환 (CSS transition)

- [ ] **Task 3-4**: homeUrl 초기화 및 동적 변경 로직 추가 (0.5시간)
  - 담당: frontend-dev
  - 의존성: Task 3-3 완료
  - 설명: BrowserView의 homeUrl 상태를 settingsService에서 로드 및 동적 변경
  - 산출물:
    - `src/views/BrowserView.js` 수정
  - 세부 작업:
    - `useEffect`로 `settingsService.getHomepageUrl()` 호출하여 초기 homeUrl 설정
    - `handleHomepageChange(newUrl)` 핸들러에서 `setHomeUrl(newUrl)` 호출
  - 완료 기준:
    - 앱 시작 시 저장된 홈페이지 URL로 초기화
    - SettingsPanel에서 홈페이지 URL 변경 시 NavigationBar 홈 버튼 동작 변경

---

### Phase 4: WebView 쿠키/캐시 삭제 연동 (선택적)
**목표**: webOS WebView API를 사용한 쿠키/캐시 삭제 구현

**태스크**:
- [ ] **Task 4-1**: WebView API 조사 및 문서 확인 (0.5시간)
  - 담당: frontend-dev
  - 의존성: Phase 3 완료
  - 설명: webOS WebView API 문서에서 clearCache/clearCookies 메서드 존재 여부 확인
  - 산출물:
    - 조사 결과 문서 (코멘트 또는 로그)
  - 완료 기준:
    - API 문서 링크 및 메서드 시그니처 확인
    - 사용 가능 여부 판단

- [ ] **Task 4-2**: WebView 쿠키/캐시 삭제 구현 (1시간)
  - 담당: frontend-dev
  - 의존성: Task 4-1 완료 (API 사용 가능한 경우)
  - 설명: webView.clearCache(), webView.clearCookies() 호출 로직 구현
  - 산출물:
    - `settingsService.js` 수정 (deleteBrowsingData 함수 내부)
    - `src/components/WebView/WebView.js` 수정 (clearCache/clearCookies 메서드 노출)
  - 세부 작업:
    - WebView 컴포넌트에서 webView 인스턴스 접근 방법 확인 (ref 사용)
    - settingsService에서 webView 인스턴스를 인자로 받거나 Context로 접근
    - clearCache(), clearCookies() 메서드 호출
    - 실패 시 에러 로그 기록
  - 완료 기준:
    - 쿠키/캐시 삭제 후 웹사이트 재로그인 필요 확인 (세션 초기화)
    - 에러 발생 시 사용자에게 토스트 메시지 표시

- [ ] **Task 4-3**: WebView API 미지원 시 대체 로직 구현 (0.5시간)
  - 담당: frontend-dev
  - 의존성: Task 4-1 완료 (API 미지원인 경우)
  - 설명: WebView API가 없을 경우 사용자 안내 메시지 표시
  - 산출물:
    - `settingsService.js` 수정
  - 세부 작업:
    - deleteBrowsingData에서 쿠키/캐시 삭제 시도 시 사용자에게 안내:
      - "쿠키/캐시 삭제는 브라우저 재시작 후 적용됩니다" 토스트 메시지
      - 또는 "현재 플랫폼에서 지원하지 않는 기능입니다" 메시지
  - 완료 기준:
    - 쿠키/캐시 삭제 체크박스 선택 시 안내 메시지 표시

---

### Phase 5: 스타일링 및 UX 개선
**목표**: 대화면 최적화 및 리모컨 포커스 관리 개선

**태스크**:
- [ ] **Task 5-1**: SettingsPanel 스타일링 (1시간)
  - 담당: frontend-dev
  - 의존성: Phase 2 완료
  - 설명: 대화면 가독성 및 Moonstone 테마 일관성 유지
  - 산출물:
    - `SettingsPanel.module.less` 수정
  - 세부 작업:
    - 폰트 크기: 레이블 20px, 설명 16px, Dropdown/Input 18px
    - 간격: 항목 간 24px, 레이블-컨트롤 12px
    - 패널 여백: 48px (상하좌우)
    - 포커스 스타일: Moonstone 기본 (파란색 테두리) 유지
    - 에러 메시지 스타일: 빨간색 텍스트, 16px
  - 완료 기준:
    - 3m 거리에서 텍스트 읽기 가능 (실제 프로젝터 테스트)
    - Light/Dark 테마 모두에서 가독성 확보

- [ ] **Task 5-2**: Spotlight 포커스 순서 최적화 (0.5시간)
  - 담당: frontend-dev
  - 의존성: Task 5-1 완료
  - 설명: 설정 패널 내부 포커스 순서 및 진입/탈출 로직 최적화
  - 산출물:
    - `SettingsPanel.js` 수정
  - 세부 작업:
    - Spotlight.set() 설정:
      - defaultElement: 검색 엔진 Dropdown
      - enterTo: 'default-element'
      - leaveFor: left/right 이동 방지
    - 패널 열릴 때 자동 포커스 (useEffect + Spotlight.focus())
    - 포커스 순서: 검색 엔진 → 홈페이지 URL → 저장 버튼 → 테마 → 체크박스들 → 삭제 버튼 → 닫기 버튼
  - 완료 기준:
    - 리모컨 Up/Down으로 논리적인 순서로 항목 간 이동
    - 패널 열릴 때 첫 번째 항목에 자동 포커스
    - Back 버튼으로 패널 닫기 시 이전 포커스 위치 복원

- [ ] **Task 5-3**: 테마 전환 애니메이션 추가 (0.5시간)
  - 담당: frontend-dev
  - 의존성: Task 5-2 완료
  - 설명: 테마 변경 시 부드러운 전환 효과
  - 산출물:
    - `src/App/App.module.less` 수정
  - 세부 작업:
    - CSS transition 추가: `transition: background-color 0.3s ease, color 0.3s ease;`
    - Moonstone 컴포넌트에도 transition 적용되도록 전역 CSS 클래스 추가
  - 완료 기준:
    - 테마 변경 시 화면 깜빡임 없이 부드럽게 전환
    - 0.3초 이내 전환 완료

- [ ] **Task 5-4**: 로딩 인디케이터 추가 (브라우징 데이터 삭제 중) (0.5시간)
  - 담당: frontend-dev
  - 의존성: Task 5-3 완료
  - 설명: 브라우징 데이터 삭제 중 LoadingSpinner 표시
  - 산출물:
    - `SettingsPanel.js` 수정
  - 세부 작업:
    - `isDeleting` 상태 추가
    - `handleDeleteConfirm()` 호출 시 `setIsDeleting(true)`
    - LoadingSpinner 컴포넌트 렌더링 (F-05 컴포넌트 재사용)
    - 삭제 완료 후 `setIsDeleting(false)`
  - 완료 기준:
    - 삭제 중 Spinner 표시
    - 삭제 완료 후 Spinner 사라지고 토스트 메시지 표시

---

### Phase 6: 테스트 및 검증
**목표**: 단위 테스트, 통합 테스트, 리모컨 조작 테스트

**태스크**:
- [ ] **Task 6-1**: settingsService 단위 테스트 작성 (1.5시간)
  - 담당: test-runner
  - 의존성: Phase 1 완료
  - 설명: 설정 CRUD 및 브라우징 데이터 삭제 로직 테스트
  - 산출물:
    - `src/services/__tests__/settingsService.test.js`
  - 테스트 케이스:
    - `loadSettings()` — LocalStorage 없을 때 기본값 반환
    - `saveSettings()` — LocalStorage에 JSON 저장 확인
    - `setSearchEngine()` — searchService 호출 확인
    - `setHomepageUrl()` — 유효한 URL 저장, 유효하지 않은 URL 거부
    - `setTheme()` — 테마 저장 (light/dark만 허용)
    - `isValidUrl()` — URL 유효성 검증 (http/https만 허용)
    - `deleteBrowsingData()` — 각 서비스 호출 확인 (mock)
  - 완료 기준:
    - 모든 테스트 케이스 통과 (커버리지 80% 이상)

- [ ] **Task 6-2**: SettingsPanel 통합 테스트 작성 (1.5시간)
  - 담당: test-runner
  - 의존성: Phase 2 완료
  - 설명: 설정 변경 시 UI 및 서비스 연동 테스트
  - 산출물:
    - `src/components/SettingsPanel/__tests__/SettingsPanel.test.js`
  - 테스트 케이스:
    - 검색 엔진 변경 → settingsService.setSearchEngine() 호출 확인
    - 홈페이지 URL 저장 → 유효한 URL일 때만 저장, 에러 메시지 표시
    - 테마 변경 → onThemeChange 콜백 호출 확인
    - 브라우징 데이터 삭제 → 확인 다이얼로그 표시 → 삭제 완료 후 토스트 표시
    - Panel 닫기 → onClose 콜백 호출 확인
  - 완료 기준:
    - 모든 테스트 케이스 통과
    - Enact Testing Library 사용 (Spotlight 포커스 테스트)

- [ ] **Task 6-3**: 리모컨 조작 E2E 테스트 (1시간)
  - 담당: test-runner
  - 의존성: Phase 3 완료
  - 설명: 실제 리모컨 입력으로 설정 패널 조작 시나리오 테스트
  - 산출물:
    - 테스트 스크립트 (수동 테스트 체크리스트)
  - 테스트 시나리오:
    1. NavigationBar 설정 버튼 클릭 → SettingsPanel 열림
    2. 십자키 Down으로 설정 항목 간 이동 (포커스 순서 확인)
    3. 검색 엔진 Dropdown에서 좌/우 키로 값 변경
    4. 홈페이지 URL Input에서 가상 키보드로 입력
    5. 저장 버튼 클릭 → 홈페이지 URL 저장 확인
    6. 테마 Dropdown에서 Dark 선택 → UI 색상 즉시 변경
    7. 브라우징 데이터 체크박스 선택 (히스토리, 북마크)
    8. 삭제 버튼 클릭 → 확인 다이얼로그 → 삭제 확인
    9. Back 버튼으로 패널 닫기
  - 완료 기준:
    - 모든 시나리오 정상 동작
    - 포커스 이동 자연스러움
    - 에러 없이 설정 저장 및 삭제 완료

- [ ] **Task 6-4**: 성능 테스트 (0.5시간)
  - 담당: test-runner
  - 의존성: Task 6-3 완료
  - 설명: 설정 저장, 테마 변경, 브라우징 데이터 삭제 성능 측정
  - 산출물:
    - 성능 측정 결과 문서
  - 테스트 항목:
    - 설정 저장 시간 → 0.3초 이내
    - 테마 변경 반응 시간 → 0.5초 이내 UI 반영
    - 브라우징 데이터 삭제 시간 → 5초 이내 (5,000개 히스토리 기준)
  - 완료 기준:
    - 모든 성능 목표 충족
    - 성능 미달 시 개선 방안 제시

---

### Phase 7: 코드 리뷰 및 문서화
**목표**: 코드 품질 검증 및 컴포넌트 문서 작성

**태스크**:
- [ ] **Task 7-1**: 코드 리뷰 (1시간)
  - 담당: code-reviewer
  - 의존성: Phase 6 완료
  - 설명: 전체 코드 및 테스트 리뷰
  - 검토 항목:
    - settingsService 로직 정합성 (URL 검증, 에러 처리)
    - SettingsPanel 컴포넌트 구조 및 가독성
    - Spotlight 포커스 관리 일관성
    - 테스트 커버리지 및 시나리오 충분성
    - 코딩 컨벤션 준수 (주석 한국어, 변수명 영어 camelCase)
  - 완료 기준:
    - 모든 Critical/High 이슈 해결
    - Medium 이슈 70% 이상 해결
    - 코드 병합 승인

- [ ] **Task 7-2**: 컴포넌트 문서 작성 (1시간)
  - 담당: frontend-dev
  - 의존성: Task 7-1 완료
  - 설명: SettingsPanel 및 SettingItem 컴포넌트 문서
  - 산출물:
    - `docs/components/SettingsPanel.md`
    - `docs/components/SettingItem.md`
  - 포함 내용:
    - Props 인터페이스 (JSDoc 형식)
    - 사용 예시 (코드 스니펫)
    - 리모컨 키 매핑 (설정 패널 내부 포커스 순서)
    - 상태 관리 방법 (settingsService 사용법)
    - 주의사항 (WebView API 미지원 시 대체 로직)
  - 완료 기준:
    - 다른 개발자가 문서만 보고 컴포넌트 사용 가능
    - 코드 예시 실행 가능 (복사-붙여넣기)

- [ ] **Task 7-3**: 운영 문서 업데이트 (0.5시간)
  - 담당: doc-writer
  - 의존성: Task 7-2 완료
  - 설명: dev-log.md 및 CHANGELOG.md 업데이트
  - 산출물:
    - `docs/dev-log.md` 업데이트 (F-11 완료 로그)
    - `CHANGELOG.md` 업데이트 (0.1.0 버전에 F-11 추가)
  - 완료 기준:
    - 기능 설명, 주요 변경 사항, 테스트 결과 기록
    - CHANGELOG에 사용자 관점의 변경 내용 기록

---

## 3. 태스크 의존성 다이어그램

```
Phase 1: 서비스 레이어
Task 1-1 ──▶ Task 1-2 ──▶ Task 1-3
(기본 CRUD)   (설정 변경)   (데이터 삭제)

Phase 2: UI 컴포넌트
Task 2-1 ──▶ Task 2-2 ──┬──▶ Task 2-3 ──▶ Task 2-4 ──▶ Task 2-5 ──▶ Task 2-6
(SettingItem) (Panel 구조) │   (검색 엔진)  (홈페이지)   (테마)      (데이터 삭제)
                           │
                           └──▶ Task 2-7 (확인 다이얼로그, 병렬 가능)

Phase 3: 통합
Task 3-1 ──▶ Task 3-2 ──▶ Task 3-3 ──▶ Task 3-4
(설정 버튼)  (BrowserView)  (App 테마)   (homeUrl)

Phase 4: WebView 연동 (선택적)
Task 4-1 ──┬──▶ Task 4-2 (API 지원 시)
(API 조사)  │
           └──▶ Task 4-3 (API 미지원 시)

Phase 5: 스타일링
Task 5-1 ──▶ Task 5-2 ──▶ Task 5-3 ──▶ Task 5-4
(스타일)   (Spotlight)  (애니메이션) (로딩)

Phase 6: 테스트
Task 6-1 ──┬──▶ Task 6-3 ──▶ Task 6-4
(단위)     │     (E2E)       (성능)
           │
Task 6-2 ──┘
(통합)

Phase 7: 마무리
Task 7-1 ──▶ Task 7-2 ──▶ Task 7-3
(코드 리뷰)  (컴포넌트 문서) (운영 문서)
```

---

## 4. 병렬 실행 판단

### 병렬 실행 불가능 — 순차 개발 권장

**이유**:
1. **단일 컴포넌트 의존성**: SettingsPanel 하나가 모든 설정 항목을 포함하므로 분리 어려움
2. **서비스 공유**: settingsService가 모든 UI 컴포넌트에서 공유되므로 동시 수정 시 충돌 위험
3. **통합 복잡도**: BrowserView, App.js 통합이 필수이므로 병렬 작업 시 머지 충돌 가능성 높음
4. **Spotlight 포커스**: 설정 패널 내부 포커스 순서가 복잡하여 일관된 구현 필요
5. **WebView 연동**: Phase 4는 선택적이며 Phase 3 완료 후 판단 필요

### Agent Team 사용 권장 여부: **No**

**권장 사항**: 단일 frontend-dev 에이전트가 Phase 1~7을 순차 진행

**예외**: Phase 2와 Task 2-7(확인 다이얼로그)만 병렬 가능하지만, 태스크 크기가 작아 병렬 효율이 낮음

---

## 5. 예상 복잡도

| Phase | 복잡도 | 이유 |
|-------|--------|------|
| Phase 1 | **Medium** | LocalStorage CRUD는 단순하지만, 브라우징 데이터 삭제 오케스트레이션이 복잡 (여러 서비스 호출) |
| Phase 2 | **High** | 설정 항목이 다양하고 Spotlight 포커스 관리, 유효성 검증, 확인 다이얼로그 등 UX 로직 복잡 |
| Phase 3 | **Medium** | BrowserView/App 통합은 비교적 명확하지만, CustomEvent 기반 테마 변경 로직 테스트 필요 |
| Phase 4 | **Low (선택적)** | WebView API 지원 여부에 따라 간단한 호출 또는 안내 메시지만 추가 |
| Phase 5 | **Low** | 스타일링 및 애니메이션은 단순 CSS 작업 |
| Phase 6 | **Medium** | 단위/통합 테스트는 표준적이지만, E2E 시나리오가 다양하고 성능 측정 필요 |
| Phase 7 | **Low** | 코드 리뷰 및 문서화는 표준 프로세스 |

**전체 복잡도**: **Medium-High**
- **예상 구현 시간**: 15~20시간 (Phase 1~7 합산)
- **주요 리스크**: Spotlight 포커스 관리, WebView API 미지원 대응

---

## 6. 리스크 및 대응 방안

| 리스크 | 영향도 | 발생 가능성 | 대응 방안 |
|--------|--------|-------------|-----------|
| **WebView API 미지원** (쿠키/캐시 삭제) | High | Medium | Phase 4-1에서 API 조사 → 미지원 시 사용자 안내 메시지 표시 (Task 4-3) |
| **테마 전환 시 화면 깜빡임** | Medium | Low | CSS transition 추가 (Task 5-3), 실제 webOS에서 테스트 필수 |
| **Spotlight 포커스 순서 혼란** | Medium | Medium | 설계서의 포커스 플로우 엄격히 준수, E2E 테스트로 검증 (Task 6-3) |
| **대용량 히스토리 삭제 시간 초과** (5,000개) | Low | Low | 비동기 처리 + 로딩 인디케이터 (Task 5-4), 성능 테스트로 검증 (Task 6-4) |
| **홈페이지 URL 유효성 검증 취약점** | High | Low | isValidUrl() 함수로 XSS 방지 (http/https만 허용), 테스트로 검증 (Task 6-1) |
| **LocalStorage 용량 초과** | Low | Very Low | 설정 JSON이 100 bytes 이하로 문제 없음, 저장 실패 시 에러 로그 |
| **설정 변경 시 앱 전체 리렌더링** | Low | Low | settingsService 모듈 스코프로 관리, 필요한 컴포넌트만 리렌더링 |

**우선순위 리스크**: WebView API 미지원 (Phase 4-1에서 조기 검증 필수)

---

## 7. 테스트 전략

### 단위 테스트 (Jest)
- **범위**: settingsService.js 모든 함수
- **목표**: 코드 커버리지 80% 이상
- **주요 테스트**:
  - LocalStorage 읽기/쓰기 정상 동작
  - URL 유효성 검증 (http/https 외 거부)
  - 브라우징 데이터 삭제 시 각 서비스 호출 확인 (mock)

### 통합 테스트 (Enact Testing Library)
- **범위**: SettingsPanel 컴포넌트 및 BrowserView 통합
- **목표**: 설정 변경 시 UI 및 서비스 연동 검증
- **주요 테스트**:
  - 검색 엔진 변경 → settingsService 호출 확인
  - 홈페이지 URL 저장 → NavigationBar 홈 버튼 동작 변경
  - 테마 변경 → App 컴포넌트 skin prop 변경 확인
  - 브라우징 데이터 삭제 → 확인 다이얼로그 → 삭제 완료 토스트

### E2E 테스트 (수동 테스트)
- **범위**: 실제 webOS 프로젝터에서 리모컨 조작
- **목표**: 리모컨 포커스 및 설정 변경 전체 플로우 검증
- **주요 시나리오**:
  - 설정 패널 열기 → 설정 변경 → 저장 → 패널 닫기 → 변경 사항 반영 확인
  - 대화면 가독성 테스트 (3m 거리)
  - 테마 전환 부드러움 테스트

### 성능 테스트
- **목표**: 요구사항의 성능 목표 충족 확인
- **측정 항목**:
  - 설정 저장 시간 < 0.3초
  - 테마 변경 반응 시간 < 0.5초
  - 브라우징 데이터 삭제 시간 < 5초 (5,000개 히스토리)

---

## 8. 예상 산출물

### 소스 코드
| 파일 경로 | 설명 |
|----------|------|
| `src/services/settingsService.js` | 설정 CRUD 및 브라우징 데이터 삭제 서비스 |
| `src/components/SettingsPanel/SettingsPanel.js` | 설정 패널 메인 컴포넌트 |
| `src/components/SettingsPanel/SettingItem.js` | 재사용 가능한 설정 항목 컴포넌트 |
| `src/components/SettingsPanel/SettingsPanel.module.less` | 설정 패널 스타일 |
| `src/components/SettingsPanel/SettingItem.module.less` | 설정 항목 스타일 |
| `src/components/SettingsPanel/index.js` | Export 파일 |
| `src/views/BrowserView.js` (수정) | SettingsPanel 통합 및 이벤트 핸들러 |
| `src/components/NavigationBar/NavigationBar.js` (수정) | 설정 버튼 추가 |
| `src/App/App.js` (수정) | 테마 적용 로직 |
| `src/App/App.module.less` (수정) | 테마 전환 애니메이션 |

### 테스트
| 파일 경로 | 설명 |
|----------|------|
| `src/services/__tests__/settingsService.test.js` | settingsService 단위 테스트 |
| `src/components/SettingsPanel/__tests__/SettingsPanel.test.js` | SettingsPanel 통합 테스트 |

### 문서
| 파일 경로 | 설명 |
|----------|------|
| `docs/components/SettingsPanel.md` | SettingsPanel 컴포넌트 문서 |
| `docs/components/SettingItem.md` | SettingItem 컴포넌트 문서 |
| `docs/dev-log.md` (업데이트) | F-11 기능 완료 로그 |
| `CHANGELOG.md` (업데이트) | 0.1.0 버전에 F-11 추가 |

---

## 9. 완료 기준

### 기능적 완료 기준
- [ ] 설정 패널이 NavigationBar 설정 버튼으로 열리고 Back 버튼으로 닫힘
- [ ] 검색 엔진 변경 시 URLBar에서 검색 시 변경된 엔진 사용
- [ ] 홈페이지 URL 저장 시 NavigationBar 홈 버튼 동작 변경
- [ ] 테마 변경 시 전체 UI 색상 즉시 변경 (Light/Dark)
- [ ] 브라우징 데이터 삭제 시 확인 다이얼로그 표시 후 선택된 데이터만 삭제
- [ ] 앱 재시작 시 모든 설정 값 복원 (LocalStorage 영속화)

### 비기능적 완료 기준
- [ ] 설정 저장 시간 < 0.3초
- [ ] 테마 변경 반응 시간 < 0.5초
- [ ] 브라우징 데이터 삭제 시간 < 5초 (5,000개 히스토리)
- [ ] 대화면에서 가독성 확보 (3m 거리에서 텍스트 읽기 가능)
- [ ] Spotlight 포커스 순서 논리적이고 자연스러움
- [ ] 테마 전환 시 화면 깜빡임 없이 부드럽게 전환

### 품질 완료 기준
- [ ] 단위 테스트 커버리지 80% 이상 (settingsService)
- [ ] 통합 테스트 모든 시나리오 통과 (SettingsPanel)
- [ ] E2E 테스트 리모컨 조작 정상 동작
- [ ] 코드 리뷰 승인 (Critical/High 이슈 0건)
- [ ] 컴포넌트 문서 작성 완료

### 문서 완료 기준
- [ ] SettingsPanel, SettingItem 컴포넌트 문서 작성
- [ ] dev-log.md 및 CHANGELOG.md 업데이트

---

## 10. 변경 이력

| 날짜 | 변경 내용 | 담당 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | product-manager |
