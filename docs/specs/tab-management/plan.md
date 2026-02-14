# 탭 관리 시스템 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/tab-management/requirements.md
- 기술 설계서: docs/specs/tab-management/design.md

---

## 2. 구현 개요

### 2.1 구현 목표
여러 웹 페이지를 동시에 열어두고 효율적으로 전환할 수 있는 다중 탭 브라우징 환경을 제공합니다. **메모리 최적화**를 위해 싱글 WebView 인스턴스 전략을 채택하여, 활성 탭만 DOM에 마운트하고 백그라운드 탭은 경량 메타데이터로 관리합니다.

### 2.2 핵심 기술 결정
| 항목 | 결정 | 근거 |
|------|------|------|
| 상태 관리 | Context API + useReducer | 프로젝트 규모에 적합, Redux는 오버엔지니어링 |
| WebView 전략 | 싱글 인스턴스 (활성 탭만 DOM 마운트) | 메모리 80% 절감, 탭 전환 ~180ms |
| 히스토리 관리 | 탭별 독립 히스토리 스택 | 데스크톱 브라우저 표준, 사용자 기대 |
| 최대 탭 수 | 하드 리미트 5개 | 프로젝터 메모리 제약 |
| 탭 영속성 | 없음 (메모리만 사용) | 요구사항 명시, 단순성 확보 |

### 2.3 주요 리스크
1. **BrowserView 리팩토링 복잡도**: 기존 단일 탭 구조를 다중 탭 구조로 전환하는 작업이 핵심. 기존 state(currentUrl, historyStack) 제거 → TabContext 통합
2. **탭 전환 시 페이지 재로딩**: 싱글 WebView 전략의 트레이드오프. LoadingBar로 UX 완화 필요
3. **메모리 누수**: iframe 리소스 해제를 useEffect cleanup에서 정확히 처리하지 않으면 반복 전환 시 메모리 증가
4. **Spotlight 포커스 복잡도**: URLBar-TabBar-WebView-NavigationBar 간 포커스 흐름 재설계 필요

---

## 3. 구현 Phase

### Phase 1: 핵심 인프라 구축 (순차 실행)
**목표**: 탭 상태 관리와 비즈니스 로직 구현

#### Task 1-1: TabContext 구현
- **담당**: frontend-dev
- **산출물**: `src/contexts/TabContext.js`
- **설명**:
  - TabContext 생성 (초기 상태: 탭 1개, activeTabId 설정)
  - tabReducer 구현 (5가지 액션: CREATE_TAB, CLOSE_TAB, SWITCH_TAB, UPDATE_TAB, UPDATE_TAB_HISTORY)
  - TabProvider 컴포넌트 구현 (useReducer 래핑)
  - useTabContext hook 구현 (Context 접근 편의)
- **완료 기준**:
  - [ ] reducer가 5개 액션 모두 처리
  - [ ] 최대 5개 탭 제한 로직 포함
  - [ ] 최소 1개 탭 유지 로직 포함
  - [ ] activeTabId 변경 시 lastAccessedAt 업데이트
- **예상 소요 시간**: 2시간

#### Task 1-2: TabManager 서비스 구현
- **담당**: frontend-dev
- **산출물**: `src/services/tabManager.js`
- **설명**:
  - 순수 함수 집합 구현
  - createTab(url): UUID 생성, 초기 TabModel 반환
  - closeTab(tabs, activeTabId, tabIdToClose): 탭 삭제 + 활성 탭 조정
  - updateTabHistory(tab, newUrl, action): 히스토리 스택 업데이트 (new/back/forward)
  - canGoBack(tab), canGoForward(tab): 히스토리 버튼 상태 계산
  - getActiveTab(tabs, activeTabId): 활성 탭 조회
- **완료 기준**:
  - [ ] 단위 테스트 통과 (Jest)
  - [ ] ESLint 통과 (세미콜론 없음, 탭 들여쓰기)
  - [ ] PropTypes 타입 검증 (JSDoc 주석)
- **예상 소요 시간**: 2시간
- **의존성**: Task 1-1 완료 후 시작 (TabModel 구조 확정)

#### Task 1-3: UUID 유틸리티 구현
- **담당**: frontend-dev
- **산출물**: `src/utils/uuid.js`
- **설명**:
  - generateUUID() 함수 구현 (crypto.randomUUID 또는 Math.random 폴백)
  - 탭 ID 생성 시 사용 (예: "tab-550e8400-e29b-41d4-a716")
- **완료 기준**:
  - [ ] 고유 ID 생성 보장 (테스트: 1000개 생성 → 중복 없음)
- **예상 소요 시간**: 30분
- **의존성**: Task 1-2와 병렬 실행 가능

#### Task 1-4: App.js에 TabProvider 래핑
- **담당**: frontend-dev
- **산출물**: `src/App/App.js` (수정)
- **설명**:
  - TabProvider로 BrowserView 래핑
  - 전역 탭 상태를 App 레벨에서 제공
- **완료 기준**:
  - [ ] TabContext가 BrowserView에서 접근 가능
  - [ ] 앱 시작 시 초기 탭 1개 자동 생성
- **예상 소요 시간**: 30분
- **의존성**: Task 1-1, 1-2 완료 후 시작

**Phase 1 완료 기준**:
- [ ] TabContext와 TabManager가 독립적으로 테스트 가능
- [ ] App.js에서 TabProvider로 전역 상태 제공
- [ ] 단위 테스트 통과 (tabReducer, TabManager)

**예상 총 소요 시간**: 5시간

---

### Phase 2: TabBar UI 컴포넌트 구현 (순차 실행)
**목표**: 탭 목록과 컨트롤을 표시하는 UI 구현

#### Task 2-1: TabBar 컴포넌트 기본 구조
- **담당**: frontend-dev
- **산출물**: `src/components/TabBar/TabBar.js`, `src/components/TabBar/index.js`
- **설명**:
  - TabBar 컴포넌트 생성 (함수형 컴포넌트)
  - Props 인터페이스 정의 (tabs, activeTabId, onTabClick, onTabClose, onNewTab)
  - PropTypes 타입 검증 추가
  - 탭 목록 렌더링 (tabs.map으로 반복)
  - 각 탭 아이템: 제목 (최대 30자, 초과 시 "..."), 닫기 버튼 (X)
  - "+ 새 탭" 버튼 (tabs.length >= 5일 때 disabled)
- **완료 기준**:
  - [ ] 탭 목록이 화면에 표시됨
  - [ ] 현재 활성 탭이 시각적으로 강조됨 (CSS 클래스)
  - [ ] 닫기 버튼 클릭 시 onTabClose 콜백 호출
  - [ ] PropTypes 타입 검증 통과
- **예상 소요 시간**: 2시간
- **의존성**: Phase 1 완료 후 시작 (TabContext 필요)

#### Task 2-2: TabBar 스타일링
- **담당**: frontend-dev
- **산출물**: `src/components/TabBar/TabBar.module.less`
- **설명**:
  - CSS Modules로 스타일 작성
  - 탭 아이템: 최소 폰트 크기 24px, 고대비 색상
  - 활성 탭: 배경색 변경 (Moonstone 테마 색상 사용)
  - 탭 간격: 16px (대화면 가독성)
  - 닫기 버튼: 우측 상단 배치, 아이콘 크기 20px
  - "+ 새 탭" 버튼: 우측 끝 고정 배치
- **완료 기준**:
  - [ ] 대화면(2~3m 거리)에서 가독성 확보
  - [ ] 활성 탭과 비활성 탭 구분 명확
  - [ ] 리모컨 포커스 상태 표시 (테두리)
- **예상 소요 시간**: 1.5시간
- **의존성**: Task 2-1과 병렬 실행 가능

#### Task 2-3: Spotlight 포커스 통합
- **담당**: frontend-dev
- **산출물**: `src/components/TabBar/TabBar.js` (수정)
- **설명**:
  - Spottable HOC 적용 (탭 아이템, 새 탭 버튼)
  - spotlight-container 설정 (TabBar 영역)
  - 좌/우 방향키로 탭 간 포커스 이동
  - 선택 버튼으로 탭 전환
  - 포커스 상태 CSS 클래스 추가 (.focused)
- **완료 기준**:
  - [ ] 리모컨 좌/우 방향키로 탭 포커스 이동
  - [ ] 선택 버튼으로 onTabClick 호출
  - [ ] 포커스 상태가 시각적으로 명확 (테두리 표시)
- **예상 소요 시간**: 1.5시간
- **의존성**: Task 2-1 완료 후 시작

**Phase 2 완료 기준**:
- [ ] TabBar가 화면에 표시되고 탭 목록 렌더링
- [ ] 리모컨으로 탭 간 포커스 이동 가능
- [ ] 탭 클릭/닫기/생성 이벤트 콜백 정상 호출
- [ ] Moonstone UI 스타일 가이드 준수

**예상 총 소요 시간**: 5시간

---

### Phase 3: BrowserView 리팩토링 (순차 실행, 핵심)
**목표**: 기존 단일 탭 구조를 다중 탭 구조로 전환

#### Task 3-1: BrowserView 상태 제거 및 Context 통합
- **담당**: frontend-dev
- **산출물**: `src/views/BrowserView.js` (대규모 수정)
- **설명**:
  - **제거**: 기존 useState 제거 (currentUrl, historyStack, historyIndex, isLoading 등)
  - **추가**: useTabContext() hook 사용
  - **변경**: activeTab 기준으로 WebView props 바인딩
    - currentUrl → activeTab.url
    - canGoBack → activeTab.historyIndex > 0
    - canGoForward → activeTab.historyIndex < activeTab.historyStack.length - 1
  - TabBar 컴포넌트 추가 (URLBar 아래, WebView 위)
  - TabBar 이벤트 핸들러 구현 (onTabClick, onTabClose, onNewTab)
- **완료 기준**:
  - [ ] 기존 state 완전 제거 (useState 0개)
  - [ ] TabContext에서 activeTab 조회
  - [ ] WebView가 activeTab.url 기준으로 렌더링
  - [ ] ESLint 통과
- **예상 소요 시간**: 3시간
- **의존성**: Phase 1, 2 완료 후 시작

#### Task 3-2: 탭 전환 로직 구현
- **담당**: frontend-dev
- **산출물**: `src/views/BrowserView.js` (수정)
- **설명**:
  - handleTabSwitch 함수 구현
  - dispatch({ type: 'SWITCH_TAB', payload: { id: newTabId } })
  - WebView unmount → remount 처리 (activeTabId 변경 시 useEffect 트리거)
  - NavigationBar 버튼 상태 자동 업데이트 (canGoBack/canGoForward 재계산)
  - 로깅 추가 (logger.info로 탭 전환 이벤트 기록)
- **완료 기준**:
  - [ ] 탭 클릭 시 WebView가 새 탭의 URL로 전환됨
  - [ ] NavigationBar 버튼 상태가 올바르게 업데이트됨
  - [ ] 탭 전환 시간 0.5초 이내 (성능 측정)
- **예상 소요 시간**: 2시간
- **의존성**: Task 3-1 완료 후 시작

#### Task 3-3: WebView 이벤트 연동
- **담당**: frontend-dev
- **산출물**: `src/views/BrowserView.js` (수정)
- **설명**:
  - handleLoadEnd 수정: 탭 메타데이터 업데이트
    - dispatch({ type: 'UPDATE_TAB', payload: { id: activeTabId, updates: { url, title, isLoading: false } } })
  - handleNavigationChange 수정: 히스토리 스택 업데이트
    - updateTabHistory(activeTab, newUrl, 'new') 호출
    - dispatch({ type: 'UPDATE_TAB_HISTORY', payload: { ... } })
  - handleNavigationAction 수정: 뒤로/앞으로 버튼 클릭 시 히스토리 스택 조작
- **완료 기준**:
  - [ ] 페이지 로딩 완료 시 탭 제목 업데이트
  - [ ] URL 변경 시 히스토리 스택에 추가
  - [ ] 뒤로/앞으로 버튼 클릭 시 히스토리 인덱스 조정
  - [ ] 히스토리 서비스(F-08)와 정상 연동
- **예상 소요 시간**: 2.5시간
- **의존성**: Task 3-2 완료 후 시작

#### Task 3-4: WebView 메모리 해제 처리
- **담당**: frontend-dev
- **산출물**: `src/components/WebView/WebView.js` (수정)
- **설명**:
  - useEffect cleanup 함수에서 iframe.src = 'about:blank' 실행
  - unmount 시 WebView 인스턴스 리소스 해제
  - 로깅 추가 (logger.debug로 unmount 이벤트 기록)
- **완료 기준**:
  - [ ] 탭 전환 시 이전 WebView가 완전히 해제됨
  - [ ] 메모리 누수 테스트 통과 (탭 생성/닫기 100회 반복 → 메모리 증가 없음)
- **예상 소요 시간**: 1시간
- **의존성**: Task 3-2와 병렬 실행 가능

**Phase 3 완료 기준**:
- [ ] BrowserView가 TabContext 기반으로 동작
- [ ] 탭 전환 시 WebView가 올바른 URL로 전환됨
- [ ] NavigationBar 히스토리 버튼이 탭별로 정확히 반영됨
- [ ] 메모리 누수 없음

**예상 총 소요 시간**: 8.5시간

---

### Phase 4: Spotlight 포커스 흐름 재설계 (순차 실행)
**목표**: URLBar-TabBar-WebView-NavigationBar 간 포커스 전환 구현

#### Task 4-1: Spotlight 컨테이너 설정
- **담당**: frontend-dev
- **산출물**: `src/views/BrowserView.js` (useEffect 추가)
- **설명**:
  - Spotlight.set으로 4개 영역 설정 (url-bar, tab-bar, webview-main, navigation-bar)
  - leaveFor 설정:
    - url-bar: 아래 → tab-bar
    - tab-bar: 위 → url-bar, 아래 → webview-main
    - webview-main: 위 → tab-bar, 아래 → navigation-bar
    - navigation-bar: 위 → webview-main
  - activeTabId 변경 시 tab-bar defaultElement 재설정 (현재 활성 탭으로 포커스)
- **완료 기준**:
  - [ ] 리모컨 위/아래 방향키로 4개 영역 간 이동
  - [ ] TabBar 포커스 시 현재 활성 탭이 기본 포커스 대상
  - [ ] 포커스 순환 테스트 통과 (URLBar → TabBar → WebView → NavigationBar → WebView → ...)
- **예상 소요 시간**: 2시간
- **의존성**: Phase 2, 3 완료 후 시작

#### Task 4-2: 포커스 디버깅 로깅
- **담당**: frontend-dev
- **산출물**: `src/views/BrowserView.js` (수정)
- **설명**:
  - spotlightfocus 이벤트 리스너 추가 (개발 환경 전용)
  - 포커스 이동 시 logger.debug로 출력 (from/to 영역)
  - 포커스 문제 발생 시 디버깅 용이
- **완료 기준**:
  - [ ] 포커스 이동 로그가 콘솔에 출력됨
  - [ ] 프로덕션 빌드에서 로그 비활성화
- **예상 소요 시간**: 30분
- **의존성**: Task 4-1과 병렬 실행 가능

**Phase 4 완료 기준**:
- [ ] 리모컨 방향키로 모든 UI 영역 접근 가능
- [ ] 포커스 흐름이 사용자 기대와 일치
- [ ] 포커스 막힘 현상 없음

**예상 총 소요 시간**: 2.5시간

---

### Phase 5: 성능 최적화 및 검증 (순차 실행)
**목표**: 탭 전환 성능 측정 및 최적화

#### Task 5-1: 탭 전환 성능 측정
- **담당**: frontend-dev
- **산출물**: `src/views/BrowserView.js` (수정)
- **설명**:
  - handleTabSwitch에 performance.now() 타이머 추가
  - 탭 전환 시작 → WebView 로딩 완료까지 시간 측정
  - 로그 출력: "탭 전환 시간: XXXms"
  - 목표: 180ms 이내 (설계서 예상치)
- **완료 기준**:
  - [ ] 탭 전환 시간이 0.5초 이내 (NFR-1)
  - [ ] 탭 5개 생성 시에도 성능 저하 없음
- **예상 소요 시간**: 1시간
- **의존성**: Phase 3 완료 후 시작

#### Task 5-2: React.memo 최적화
- **담당**: frontend-dev
- **산출물**: `src/components/TabBar/TabBar.js`, `src/components/WebView/WebView.js` (수정)
- **설명**:
  - TabBar를 React.memo로 래핑
  - 비교 함수 구현: activeTabId, tabs.length, tabs[].id 비교
  - WebView도 React.memo 적용 (url props 비교)
  - useCallback으로 이벤트 핸들러 메모이제이션
- **완료 기준**:
  - [ ] 불필요한 리렌더링 방지 (React DevTools로 확인)
  - [ ] 탭 전환 시 TabBar 리렌더링 최소화
- **예상 소요 시간**: 1.5시간
- **의존성**: Task 5-1과 병렬 실행 가능

#### Task 5-3: 히스토리 스택 크기 제한
- **담당**: frontend-dev
- **산출물**: `src/services/tabManager.js` (수정)
- **설명**:
  - updateTabHistory에서 히스토리 스택 최대 50개 제한
  - 50개 초과 시 오래된 URL 삭제 (FIFO)
  - 메모리 절약 (탭당 히스토리 50개 × 100 bytes = 5KB)
- **완료 기준**:
  - [ ] 히스토리 스택이 50개를 넘지 않음
  - [ ] 히스토리 인덱스 조정 로직 정상 작동
- **예상 소요 시간**: 30분
- **의존성**: Task 5-1과 병렬 실행 가능

**Phase 5 완료 기준**:
- [ ] 탭 전환 시간 0.5초 이내
- [ ] 메모리 사용량 최적화 (탭 5개 시 메타데이터 ~1KB)
- [ ] React.memo로 불필요한 리렌더링 방지

**예상 총 소요 시간**: 3시간

---

### Phase 6: 테스트 작성 (순차 실행)
**목표**: 단위 테스트 및 통합 테스트 작성

#### Task 6-1: TabManager 단위 테스트
- **담당**: test-runner
- **산출물**: `src/services/tabManager.test.js`
- **설명**:
  - Jest 테스트 작성
  - createTab: UUID 생성, 초기 TabModel 구조 검증
  - closeTab: 최소 1개 유지, 활성 탭 조정 로직
  - updateTabHistory: new/back/forward 동작 검증
  - canGoBack/canGoForward: 히스토리 인덱스 경계 케이스
- **완료 기준**:
  - [ ] 모든 테스트 케이스 통과 (커버리지 90% 이상)
  - [ ] 경계 조건 테스트 포함 (최대 탭 수, 최소 탭 수)
- **예상 소요 시간**: 2시간
- **의존성**: Phase 1 완료 후 시작 가능 (Phase 3-5와 병렬 가능)

#### Task 6-2: tabReducer 단위 테스트
- **담당**: test-runner
- **산출물**: `src/contexts/TabContext.test.js`
- **설명**:
  - tabReducer의 5개 액션 테스트
  - CREATE_TAB: 최대 5개 제한 검증
  - CLOSE_TAB: 마지막 탭 닫기 불가, 활성 탭 조정
  - SWITCH_TAB: lastAccessedAt 업데이트
  - UPDATE_TAB: 탭 메타데이터 업데이트
  - UPDATE_TAB_HISTORY: 히스토리 스택 업데이트
- **완료 기준**:
  - [ ] 모든 테스트 케이스 통과
  - [ ] 불변성 유지 검증 (원본 state 변경 X)
- **예상 소요 시간**: 2시간
- **의존성**: Task 6-1과 병렬 실행 가능

#### Task 6-3: TabBar 통합 테스트
- **담당**: test-runner
- **산출물**: `src/components/TabBar/TabBar.test.js`
- **설명**:
  - Enact Testing Library 사용
  - 탭 목록 렌더링 검증
  - 탭 클릭 → onTabClick 콜백 호출
  - 닫기 버튼 클릭 → onTabClose 콜백 호출
  - "+ 새 탭" 버튼 disabled 상태 검증 (5개째)
- **완료 기준**:
  - [ ] 모든 테스트 케이스 통과
  - [ ] 리모컨 포커스 시뮬레이션 테스트 포함
- **예상 소요 시간**: 2시간
- **의존성**: Phase 2 완료 후 시작

#### Task 6-4: BrowserView 통합 테스트
- **담당**: test-runner
- **산출물**: `src/views/BrowserView.test.js`
- **설명**:
  - TabContext mock 설정
  - 탭 전환 시 WebView URL 업데이트 검증
  - NavigationBar 버튼 상태 변경 검증
  - 탭 생성/닫기 시나리오 테스트
- **완료 기준**:
  - [ ] 모든 테스트 케이스 통과
  - [ ] Context dispatch 호출 검증
- **예상 소요 시간**: 2.5시간
- **의존성**: Phase 3 완료 후 시작

**Phase 6 완료 기준**:
- [ ] 단위 테스트 커버리지 90% 이상
- [ ] 통합 테스트 통과
- [ ] ESLint 통과

**예상 총 소요 시간**: 8.5시간

---

### Phase 7: 코드 리뷰 및 문서 작성 (순차 실행)
**목표**: 코드 품질 검증 및 문서 작성

#### Task 7-1: 코드 리뷰
- **담당**: code-reviewer
- **산출물**: 리뷰 코멘트
- **설명**:
  - 코딩 컨벤션 준수 검증 (세미콜론 없음, 탭 들여쓰기, 한국어 주석)
  - PropTypes 타입 검증 누락 확인
  - 메모리 누수 위험 코드 검토 (useEffect cleanup)
  - Spotlight 설정 오류 검토
  - 불필요한 리렌더링 가능성 검토
- **완료 기준**:
  - [ ] Critical 이슈 0건
  - [ ] Major 이슈 해결
  - [ ] 코드 리뷰 승인
- **예상 소요 시간**: 2시간
- **의존성**: Phase 1-6 완료 후 시작

#### Task 7-2: 컴포넌트 문서 작성
- **담당**: frontend-dev
- **산출물**: `docs/components/TabBar.md`, `docs/components/TabManager.md`
- **설명**:
  - Props 인터페이스 문서화
  - 사용 예시 코드
  - 리모컨 키 매핑
  - 주의사항 (최대 5개 제한, 메모리 누수 방지)
- **완료 기준**:
  - [ ] 문서가 Markdown 형식으로 작성됨
  - [ ] 코드 예시가 실행 가능
- **예상 소요 시간**: 1.5시간
- **의존성**: Task 7-1과 병렬 실행 가능

#### Task 7-3: 운영 문서 업데이트
- **담당**: doc-writer
- **산출물**: `docs/dev-log.md`, `CHANGELOG.md`
- **설명**:
  - dev-log.md에 F-06 진행 기록 추가
  - CHANGELOG.md에 탭 관리 시스템 추가 내역 기록
  - 주요 변경 사항: Context API 도입, BrowserView 리팩토링
- **완료 기준**:
  - [ ] dev-log.md 업데이트
  - [ ] CHANGELOG.md 업데이트 (버전 0.2.0)
- **예상 소요 시간**: 1시간
- **의존성**: Task 7-1 완료 후 시작

**Phase 7 완료 기준**:
- [ ] 코드 리뷰 승인
- [ ] 컴포넌트 문서 작성 완료
- [ ] 운영 문서 업데이트 완료

**예상 총 소요 시간**: 4.5시간

---

## 4. 태스크 의존성

```
Phase 1: 핵심 인프라
┌──────────────────────────────────────────────────┐
│ Task 1-1 (TabContext)                            │
│         ▼                                        │
│ Task 1-2 (TabManager) ║ Task 1-3 (UUID)         │
│         ▼                        ▼               │
│ Task 1-4 (App.js 래핑)                           │
└──────────────────────────────────────────────────┘
                     ▼
Phase 2: TabBar UI
┌──────────────────────────────────────────────────┐
│ Task 2-1 (TabBar 기본) ║ Task 2-2 (스타일)        │
│         ▼                                        │
│ Task 2-3 (Spotlight 통합)                        │
└──────────────────────────────────────────────────┘
                     ▼
Phase 3: BrowserView 리팩토링 (핵심)
┌──────────────────────────────────────────────────┐
│ Task 3-1 (상태 제거 + Context 통합)               │
│         ▼                                        │
│ Task 3-2 (탭 전환 로직) ║ Task 3-4 (메모리 해제)  │
│         ▼                                        │
│ Task 3-3 (WebView 이벤트 연동)                    │
└──────────────────────────────────────────────────┘
                     ▼
Phase 4: Spotlight 포커스
┌──────────────────────────────────────────────────┐
│ Task 4-1 (Spotlight 설정) ║ Task 4-2 (디버깅)    │
└──────────────────────────────────────────────────┘
                     ▼
Phase 5: 성능 최적화
┌──────────────────────────────────────────────────┐
│ Task 5-1 (성능 측정) ║ Task 5-2 (React.memo)    │
│                      ║ Task 5-3 (히스토리 제한)  │
└──────────────────────────────────────────────────┘
                     ▼
Phase 6: 테스트 작성
┌──────────────────────────────────────────────────┐
│ Task 6-1 (TabManager 테스트) ║ Task 6-2 (Reducer 테스트) │
│                               ║                           │
│ Task 6-3 (TabBar 테스트)      ║ Task 6-4 (BrowserView 테스트) │
└──────────────────────────────────────────────────┘
                     ▼
Phase 7: 리뷰 및 문서
┌──────────────────────────────────────────────────┐
│ Task 7-1 (코드 리뷰)                              │
│         ▼                                        │
│ Task 7-2 (컴포넌트 문서) ║ Task 7-3 (운영 문서)  │
└──────────────────────────────────────────────────┘
```

---

## 5. 병렬 실행 판단

### 5.1 Agent Team 사용 권장 여부
**판단: No (순차 실행 권장)**

### 5.2 근거
1. **Context 의존성**: BrowserView 리팩토링(Phase 3)은 TabContext(Phase 1)와 TabBar(Phase 2)에 의존. 병렬 실행 시 인터페이스 불일치 위험
2. **단일 파일 충돌**: BrowserView.js 수정이 Phase 3의 4개 태스크에 집중됨. worktree 분리해도 merge 시 충돌 발생
3. **테스트 의존성**: Phase 6 테스트는 Phase 1-5 완료 필수. 병렬 실행 불가
4. **중간 규모**: 총 소요 시간 37.5시간 (약 5일). Agent Team 오버헤드가 이점을 상쇄

### 5.3 부분적 병렬 가능 태스크
| Task 쌍 | 병렬 가능 여부 | 조건 |
|---------|---------------|------|
| Task 1-2, 1-3 | ✅ | TabModel 구조만 합의하면 독립 실행 가능 |
| Task 2-1, 2-2 | ✅ | CSS Modules는 컴포넌트 구조와 독립 |
| Task 3-2, 3-4 | ✅ | WebView.js와 BrowserView.js 파일 분리 |
| Task 4-1, 4-2 | ✅ | 로깅은 Spotlight 설정에 의존하지 않음 |
| Task 5-1, 5-2, 5-3 | ✅ | 성능 측정, React.memo, 히스토리 제한 모두 독립 |
| Task 6-1, 6-2, 6-3 | ✅ | 각 파일별 테스트로 충돌 없음 |
| Task 7-2, 7-3 | ✅ | 문서 작성은 병렬 가능 |

### 5.4 최종 권장 방안
**순차 실행 + 부분 병렬**
- Phase 내에서 병렬 가능한 태스크만 선택적으로 병렬 실행
- 예: Task 6-1, 6-2, 6-3, 6-4를 Agent Team으로 병렬 실행 (테스트 작성 시간 8.5h → 4h로 단축 가능)
- 대부분의 Phase는 순차 실행 (Context 의존성, 단일 파일 충돌)

---

## 6. 예상 소요 시간

| Phase | 소요 시간 | 비고 |
|-------|----------|------|
| Phase 1: 핵심 인프라 | 5시간 | 순차 실행 필수 |
| Phase 2: TabBar UI | 5시간 | 순차 실행 필수 |
| Phase 3: BrowserView 리팩토링 | 8.5시간 | **핵심 작업**, 신중한 리팩토링 필요 |
| Phase 4: Spotlight 포커스 | 2.5시간 | 순차 실행 권장 |
| Phase 5: 성능 최적화 | 3시간 | 부분 병렬 가능 (1.5h로 단축 가능) |
| Phase 6: 테스트 작성 | 8.5시간 | Agent Team 병렬 가능 (4h로 단축 가능) |
| Phase 7: 리뷰 및 문서 | 4.5시간 | 순차 실행 권장 |
| **총 소요 시간** | **37시간** | 약 **5일** (1일 8시간 기준) |

### 병렬 실행 시 단축 가능 시간
- Phase 6 테스트 작성: 8.5h → 4h (Agent Team 4명)
- 총 단축 시간: **4.5시간**
- **최종 소요 시간**: **32.5시간** (약 4일)

---

## 7. 리스크 및 대응 방안

### 리스크 1: BrowserView 리팩토링 복잡도
**가능성**: 높음
**영향**: Phase 3에서 예상 시간 초과, 버그 증가
**대응 방안**:
1. Task 3-1에서 기존 코드를 충분히 분석 후 시작 (사전 조사 1시간 추가)
2. 작은 단위로 커밋 (예: state 제거 → Context 통합 → 이벤트 연동)
3. 각 단계마다 수동 테스트 (탭 전환 동작 확인)
4. 문제 발생 시 code-reviewer에게 즉시 리뷰 요청

### 리스크 2: 탭 전환 시 페이지 재로딩으로 인한 UX 저하
**가능성**: 확정 (설계 의도)
**영향**: 사용자 불만, 요구사항 변경 가능성
**대응 방안**:
1. LoadingBar(F-05)로 로딩 피드백 제공
2. 탭 전환 시 "페이지를 다시 로드합니다" 안내 메시지 표시 (선택)
3. 향후 "탭 고정" 기능으로 중요 탭은 메모리 유지 (F-15 이후)
4. 프로토타입 테스트로 사용자 피드백 수집

### 리스크 3: 메모리 누수
**가능성**: 중간 (useEffect cleanup 미처리 시)
**영향**: 탭 전환 반복 시 메모리 증가 → 앱 크래시
**대응 방안**:
1. Task 3-4에서 iframe.src = 'about:blank' 명시적 실행
2. 개발 환경에서 performance.memory API로 메모리 모니터링
3. 테스트: 탭 생성/닫기 100회 반복 → 메모리 증가 체크
4. 문제 발생 시 React DevTools로 unmount 로그 확인

### 리스크 4: Spotlight 포커스 순환 문제
**가능성**: 중간 (TabBar 추가로 인한 복잡도 증가)
**영향**: 리모컨 방향키 동작이 예상과 다름, 사용자 혼란
**대응 방안**:
1. Phase 4에서 충분한 테스트 시간 확보 (수동 테스트)
2. spotlightfocus 이벤트 리스너로 디버깅 로그 출력
3. Spotlight 설정을 useEffect 의존성에 포함 (activeTabId 변경 시 재설정)
4. 문제 발생 시 Enact 공식 문서 참조

### 리스크 5: 탭 전환 성능 목표 미달 (0.5초 초과)
**가능성**: 낮음 (설계서 예상 180ms)
**영향**: NFR-1 미충족, 사용자 체감 지연
**대응 방안**:
1. Phase 5에서 performance.now()로 정확한 시간 측정
2. 병목 구간 분석 (unmount/mount/dispatch/rerender)
3. React.memo로 불필요한 리렌더링 제거
4. 최후 수단: 탭 전환 애니메이션으로 체감 속도 향상

### 리스크 6: 기존 WebView/NavigationBar와의 호환성 문제
**가능성**: 중간 (F-02, F-04 기능 의존)
**영향**: 히스토리 버튼 오동작, 로딩 인디케이터 미표시
**대응 방안**:
1. Task 3-3에서 WebView 이벤트 콜백 시그니처 확인
2. NavigationBar props 인터페이스 변경 없이 canGoBack/canGoForward만 조정
3. 통합 테스트로 기존 기능 정상 작동 검증
4. 문제 발생 시 F-02, F-04 설계서 재검토

---

## 8. 검증 계획

### 8.1 기능 완료 검증 (DoD: Definition of Done)
- [ ] 새 탭 생성 버튼 클릭 시 새 탭이 추가되고 활성화됨
- [ ] 최대 5개 탭까지 생성 가능, 초과 시 "+ 새 탭" 버튼 disabled
- [ ] 탭 클릭 시 WebView가 해당 탭의 URL로 전환됨
- [ ] 탭 닫기 버튼 클릭 시 해당 탭이 삭제되고 이전 탭으로 전환됨
- [ ] 마지막 탭은 닫을 수 없음 (최소 1개 유지)
- [ ] 각 탭의 히스토리 스택이 독립적으로 관리됨
- [ ] TabBar에 탭 목록이 표시되고, 현재 활성 탭이 시각적으로 강조됨

### 8.2 성능 완료 검증
- [ ] 탭 전환 시간 0.5초 이내 (performance.now() 측정)
- [ ] 탭 5개 생성 시에도 브라우저 성능 저하 없음
- [ ] 백그라운드 탭의 WebView는 DOM에서 제거되어 메모리 절약
- [ ] 메모리 누수 테스트 통과 (탭 생성/닫기 100회 반복)

### 8.3 UX 완료 검증
- [ ] 리모컨 좌/우 방향키로 TabBar의 탭 간 포커스 이동
- [ ] 리모컨 선택 버튼으로 탭 전환 가능
- [ ] 리모컨 포커스 상태가 명확히 표시됨 (테두리, 배경색)
- [ ] 대화면에서 TabBar 텍스트가 가독성 있게 표시됨 (최소 24px)
- [ ] URLBar → TabBar → WebView → NavigationBar 포커스 흐름 정상 작동

### 8.4 통합 완료 검증
- [ ] NavigationBar의 뒤로/앞으로 버튼이 탭 전환 시 올바른 상태로 업데이트됨
- [ ] LoadingBar가 탭 전환 시에도 올바르게 표시됨
- [ ] 히스토리 서비스(F-08)가 각 탭의 방문 기록을 정확히 저장함

### 8.5 코드 품질 완료 검증
- [ ] TabManager 서비스 단위 테스트 통과
- [ ] tabReducer 단위 테스트 통과
- [ ] TabBar 컴포넌트 렌더링 테스트 통과
- [ ] BrowserView 통합 테스트 통과
- [ ] ESLint 규칙 통과 (세미콜론 없음, 탭 들여쓰기)
- [ ] PropTypes 타입 검증 추가
- [ ] 코드 리뷰 승인 (code-reviewer)

### 8.6 문서 완료 검증
- [ ] 컴포넌트 문서 작성 (`docs/components/TabBar.md`, `docs/components/TabManager.md`)
- [ ] CHANGELOG 업데이트 (버전 0.2.0)
- [ ] dev-log 업데이트

---

## 9. 주요 파일 목록

### 새로 생성되는 파일
```
src/contexts/TabContext.js                # TabContext + Provider + hook
src/services/tabManager.js                # 탭 CRUD 비즈니스 로직
src/utils/uuid.js                         # UUID 생성 유틸리티
src/components/TabBar/TabBar.js           # TabBar 컴포넌트
src/components/TabBar/TabBar.module.less  # TabBar 스타일
src/components/TabBar/index.js            # Export
docs/components/TabBar.md                 # TabBar 문서
docs/components/TabManager.md             # TabManager 문서
src/services/tabManager.test.js          # TabManager 테스트
src/contexts/TabContext.test.js          # TabContext 테스트
src/components/TabBar/TabBar.test.js     # TabBar 테스트
src/views/BrowserView.test.js            # BrowserView 테스트
```

### 수정되는 파일
```
src/App/App.js                            # TabProvider 래핑
src/views/BrowserView.js                  # TabContext 통합 (대규모 수정)
src/components/WebView/WebView.js        # useEffect cleanup 추가
```

---

## 10. 커밋 전략

### 커밋 분리 기준
1. **Phase 1 완료 후**: `feat(F-06): TabContext와 TabManager 서비스 구현`
2. **Phase 2 완료 후**: `feat(F-06): TabBar UI 컴포넌트 구현`
3. **Phase 3 완료 후**: `refactor(F-06): BrowserView를 다중 탭 구조로 리팩토링`
4. **Phase 4 완료 후**: `feat(F-06): Spotlight 포커스 흐름 재설계`
5. **Phase 5 완료 후**: `perf(F-06): 탭 전환 성능 최적화`
6. **Phase 6 완료 후**: `test(F-06): 탭 관리 시스템 테스트 추가`
7. **Phase 7 완료 후**: `docs(F-06): TabBar 및 TabManager 컴포넌트 문서`

### 커밋 메시지 예시
```
feat(F-06): TabContext와 TabManager 서비스 구현

- TabContext, TabProvider, useTabContext hook 추가
- tabReducer로 5가지 액션 처리 (CREATE_TAB, CLOSE_TAB, SWITCH_TAB, UPDATE_TAB, UPDATE_TAB_HISTORY)
- TabManager 서비스 순수 함수 구현 (createTab, closeTab, updateTabHistory 등)
- UUID 생성 유틸리티 추가
- App.js에 TabProvider 래핑
```

---

## 11. 성공 기준

### 기능적 성공 기준
1. 사용자가 리모컨으로 탭을 생성/전환/닫기 가능
2. 탭별로 독립적인 히스토리 스택 유지
3. 최대 5개 탭 제한 동작
4. 마지막 탭 닫기 방지

### 비기능적 성공 기준
1. 탭 전환 시간 0.5초 이내
2. 메모리 사용량 80% 절감 (기존 멀티 인스턴스 대비)
3. 리모컨 포커스 흐름 직관적
4. 코드 커버리지 90% 이상

### 프로젝트 성공 기준
1. 모든 테스트 통과
2. 코드 리뷰 승인
3. 컴포넌트 문서 작성 완료
4. 프로젝터 실기기에서 정상 동작 확인

---

## 변경 이력

| 날짜 | 변경 내용 | 담당 |
|------|-----------|------|
| 2026-02-13 | 최초 작성 | product-manager |
