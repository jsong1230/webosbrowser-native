# 에러 처리 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: `docs/specs/error-handling/requirements.md`
- 기술 설계서: `docs/specs/error-handling/design.md`

---

## 2. 구현 Phase

### Phase 1: 에러 분류 유틸리티 구현
- [ ] Task 1-1: `src/utils/errorClassifier.js` 생성 → frontend-dev
  - `classifyErrorByTitle()` 함수 구현 (페이지 타이틀 기반 404, 500 감지)
  - `getErrorMessage()` 함수 구현 (에러 코드 → 메시지 변환)
  - `getErrorTitle()` 함수 구현 (에러 코드 → 제목 변환)
  - 단위 테스트 작성 (Jest)
- **예상 산출물**: `src/utils/errorClassifier.js`
- **완료 기준**:
  - 404, 500, 네트워크(-1), 타임아웃(-2), CORS(-3), 알 수 없음(-99) 에러 타입 분류 가능
  - 각 에러 코드에 대한 한국어 메시지 반환
  - 테스트 커버리지 90% 이상
- **예상 소요 시간**: 1시간

---

### Phase 2: ErrorPage 컴포넌트 구현 (UI)
- [ ] Task 2-1: `src/components/ErrorPage/ErrorPage.js` 생성 → frontend-dev
  - PropTypes 정의 (errorCode, errorMessage, url, onRetry, onGoHome)
  - 에러 아이콘, 제목, 메시지, URL 표시 레이아웃 구현
  - Moonstone Button 컴포넌트 사용 (재시도, 홈 버튼)
  - Spotlight 포커스 관리 (초기 포커스: 재시도 버튼)
  - 버튼 클릭 핸들러 (onRetry, onGoHome 콜백 호출)
- [ ] Task 2-2: `src/components/ErrorPage/ErrorPage.module.less` 생성 → frontend-dev
  - 반투명 검은색 오버레이 (rgba(0, 0, 0, 0.9))
  - 대화면 최적화 폰트 크기 (제목 48px, 메시지 28px, URL 22px)
  - 페이드인/아웃 애니메이션 (300ms/200ms)
  - 버튼 스타일 (최소 200x60px, 폰트 24px)
  - Spotlight 포커스 스타일 (노란색 테두리 4px)
- [ ] Task 2-3: `src/components/ErrorPage/index.js` 생성 → frontend-dev
  - ErrorPage 컴포넌트 export default
- **예상 산출물**:
  - `src/components/ErrorPage/ErrorPage.js`
  - `src/components/ErrorPage/ErrorPage.module.less`
  - `src/components/ErrorPage/index.js`
- **완료 기준**:
  - ErrorPage 컴포넌트가 독립적으로 렌더링 가능 (Storybook 테스트)
  - 리모컨 방향키로 재시도 ↔ 홈 버튼 전환 가능
  - 페이드인/아웃 애니메이션 정상 동작
  - Moonstone 스타일 가이드 준수
- **예상 소요 시간**: 2시간

---

### Phase 3: WebView 컴포넌트 수정 (에러 감지 및 ErrorPage 통합)
- [ ] Task 3-1: WebView.js 에러 상태 관리 추가 → frontend-dev
  - `const [error, setError] = useState(null)` 추가
  - 에러 정보 구조: `{ errorCode, errorMessage, url }`
- [ ] Task 3-2: WebView.js 인라인 에러 UI 제거 → frontend-dev
  - 377~397줄 기존 에러 메시지 제거
  - ErrorPage 컴포넌트 import 및 렌더링 로직 추가
- [ ] Task 3-3: WebView.js handleGoHome 핸들러 추가 → frontend-dev
  - 홈 버튼 클릭 시 homeUrl로 iframe.src 변경
  - changeState('loading') 호출
  - 타임아웃 타이머 재시작
- [ ] Task 3-4: WebView.js handleLoad 수정 (휴리스틱 에러 감지) → frontend-dev
  - extractTitle()로 페이지 제목 추출
  - `classifyErrorByTitle(title)` 호출
  - 404, 500 감지 시 changeState('error', errorInfo) 호출
  - onLoadError 콜백 호출
- [ ] Task 3-5: WebView.js PropTypes 수정 → frontend-dev
  - `homeUrl: PropTypes.string` 추가 (defaultProps: 'https://www.google.com')
- [ ] Task 3-6: WebView.module.less 에러 스타일 제거 → frontend-dev
  - 40~100줄 기존 에러 스타일 삭제 (ErrorPage로 이동)
- **의존성**: Phase 1 (errorClassifier), Phase 2 (ErrorPage) 완료 필요
- **예상 산출물**:
  - `src/components/WebView/WebView.js` (수정)
  - `src/components/WebView/WebView.module.less` (수정)
- **완료 기준**:
  - iframe onError 시 ErrorPage 렌더링
  - 30초 타임아웃 시 ErrorPage 렌더링
  - 404, 500 페이지 로딩 시 ErrorPage 렌더링
  - 재시도 버튼 클릭 시 iframe 재로딩
  - 홈 버튼 클릭 시 homeUrl로 이동
  - 에러 로그 기록 (logger.error)
- **예상 소요 시간**: 2시간

---

### Phase 4: BrowserView 수정 (homeUrl 전달)
- [ ] Task 4-1: BrowserView.js WebView에 homeUrl prop 전달 → frontend-dev
  - 591줄 WebView 컴포넌트에 `homeUrl={homeUrl}` prop 추가
  - homeUrl 상태 관리 확인 (현재 BrowserView에 존재)
- **의존성**: Phase 3 완료 필요
- **예상 산출물**: `src/views/BrowserView.js` (수정)
- **완료 기준**:
  - BrowserView → WebView로 homeUrl prop 전달
  - ErrorPage 홈 버튼 클릭 시 homeUrl로 이동 가능
- **예상 소요 시간**: 30분

---

### Phase 5: TabContext 연동 (isError 플래그)
- [ ] Task 5-1: BrowserView.handleLoadError 확인 → frontend-dev
  - 현재 구현에서 isError 플래그를 TabContext에 업데이트하는지 확인
  - 없으면 `dispatch({ type: TAB_ACTIONS.UPDATE_TAB, payload: { isError: true } })` 추가
- [ ] Task 5-2: LoadingBar isError 반영 확인 → frontend-dev
  - LoadingBar 컴포넌트가 isError=true 시 빨간색으로 표시되는지 확인 (F-05에서 구현됨)
  - 정상 동작하면 수정 불필요
- **의존성**: Phase 3, 4 완료 필요
- **예상 산출물**: `src/views/BrowserView.js` (수정, 필요 시)
- **완료 기준**:
  - WebView 에러 발생 시 TabContext의 isError=true로 업데이트
  - LoadingBar가 에러 상태를 빨간색으로 표시
  - 탭 전환 후 에러 탭 복귀 시 에러 상태 유지
- **예상 소요 시간**: 30분

---

### Phase 6: 스타일링 및 애니메이션 최적화
- [ ] Task 6-1: ErrorPage.module.less 최적화 → frontend-dev
  - 페이드인/아웃 애니메이션 GPU 가속 확인 (opacity, transform 사용)
  - 대화면 가독성 테스트 (3m 거리에서 프로젝터 화면)
  - Moonstone 테마 스타일 가이드 준수 확인
- [ ] Task 6-2: Spotlight 포커스 테스트 → frontend-dev
  - 재시도 버튼 초기 포커스 자동 설정 확인
  - 좌/우 방향키로 버튼 전환 테스트
  - Back 키 누를 시 에러 화면 유지 (포커스 이탈 불가) 확인
- **의존성**: Phase 2, 3 완료 필요
- **예상 산출물**: `src/components/ErrorPage/ErrorPage.module.less` (수정)
- **완료 기준**:
  - 애니메이션이 부드럽게 동작 (300ms 페이드인, 200ms 페이드아웃)
  - 3m 거리에서 에러 메시지 명확하게 읽힘
  - Spotlight 포커스가 정상 동작 (노란색 테두리 4px)
- **예상 소요 시간**: 1시간

---

### Phase 7: 테스트 작성 (단위 + 통합)
- [ ] Task 7-1: errorClassifier.js 단위 테스트 → test-runner
  - 404, 500 페이지 타이틀 감지 테스트
  - 에러 코드 → 메시지 변환 테스트
  - edge case 테스트 (빈 문자열, null, undefined)
- [ ] Task 7-2: ErrorPage 컴포넌트 단위 테스트 → test-runner
  - 재시도 버튼 클릭 시 onRetry 콜백 호출 확인
  - 홈 버튼 클릭 시 onGoHome 콜백 호출 확인
  - 에러 메시지, URL 렌더링 확인
  - 초기 포커스 테스트 (재시도 버튼)
- [ ] Task 7-3: WebView 통합 테스트 (에러 감지) → test-runner
  - iframe onError 트리거 시 ErrorPage 렌더링 확인
  - 30초 타임아웃 시 ErrorPage 렌더링 확인
  - 404 페이지 로딩 시 ErrorPage 렌더링 확인 (휴리스틱)
- [ ] Task 7-4: WebView 통합 테스트 (재시도/홈) → test-runner
  - 재시도 버튼 클릭 시 iframe 재로딩 확인
  - 홈 버튼 클릭 시 homeUrl로 이동 확인
- **의존성**: Phase 1~6 완료 필요
- **예상 산출물**:
  - `src/utils/__tests__/errorClassifier.test.js`
  - `src/components/ErrorPage/__tests__/ErrorPage.test.js`
  - `src/components/WebView/__tests__/WebView.error.test.js`
- **완료 기준**:
  - 모든 테스트 통과 (npm test)
  - 코드 커버리지 85% 이상
  - 에러 시나리오 10개 이상 커버
- **예상 소요 시간**: 2시간

---

### Phase 8: 코드 + 문서 리뷰
- [ ] Task 8-1: 코드 리뷰 → code-reviewer
  - ErrorPage 컴포넌트 PropTypes 검증
  - WebView 수정사항 검증 (에러 감지, 재시도, 홈 버튼)
  - 에러 로깅 확인 (logger.error)
  - Moonstone 스타일 가이드 준수 확인
- [ ] Task 8-2: 문서 리뷰 → code-reviewer
  - requirements.md, design.md, plan.md 일관성 확인
  - 기술 설계서 아키텍처 결정 구현 확인
  - 수용 기준(AC-1~AC-10) 충족 확인
- [ ] Task 8-3: 리팩토링 (필요 시) → frontend-dev
  - 리뷰 피드백 반영
  - 코드 중복 제거
  - 주석 추가 (에러 처리 로직)
- **의존성**: Phase 1~7 완료 필요
- **예상 산출물**: 리뷰 코멘트, 리팩토링 커밋
- **완료 기준**:
  - code-reviewer의 승인 (Approve)
  - 요구사항 분석서의 수용 기준(AC) 10개 모두 충족
  - ESLint 검사 통과
- **예상 소요 시간**: 1시간

---

## 3. 태스크 의존성

```
Phase 1 (errorClassifier)
    │
    ├───────────────┐
    │               │
    ↓               ↓
Phase 2 (ErrorPage)  Phase 3 (WebView 수정)
    │               │
    └───────┬───────┘
            ↓
    Phase 4 (BrowserView)
            ↓
    Phase 5 (TabContext)
            ↓
    Phase 6 (스타일링)
            ↓
    Phase 7 (테스트)
            ↓
    Phase 8 (리뷰)
```

**병렬 실행 가능 구간**:
- Phase 2 (ErrorPage) 와 Phase 3-1~3-3 (WebView 상태 관리, 핸들러)은 독립적이므로 병렬 실행 가능
- 단, Phase 3-4 (휴리스틱 에러 감지)는 Phase 1 (errorClassifier) 완료 필요
- Phase 3-2 (ErrorPage 통합)는 Phase 2 (ErrorPage) 완료 필요

---

## 4. 병렬 실행 판단

### 병렬 실행 가능 여부
**NO - 순차 개발 권장**

### 판단 근거
1. **컴포넌트 수**: 주요 컴포넌트가 2개 (ErrorPage, WebView 수정)로 적음
2. **의존성**: Phase 3 (WebView)이 Phase 1, 2의 결과물을 모두 참조해야 함
3. **통합 테스트 중요도**: ErrorPage와 WebView의 긴밀한 통합이 핵심
4. **구현 복잡도**: Medium (유틸리티 + 컴포넌트 + 통합)
5. **예상 총 소요 시간**: 10시간 (1인 개발자가 순차 작업 시 1~2일)

### Agent Team 사용하지 않는 이유
- **통합 테스트 부담**: 병렬 작업 후 merge 시 통합 테스트 복잡도 증가
- **코드 충돌 위험**: WebView.js 파일을 여러 에이전트가 동시 수정 시 충돌 가능성
- **단순성 유지**: 1명의 frontend-dev가 전체 흐름을 이해하고 구현하는 것이 효율적

### 순차 개발 권장 이유
- **일관된 에러 처리 로직**: 에러 감지 → 분류 → UI 표시가 하나의 플로우로 연결됨
- **빠른 피드백**: Phase 1~3을 연속으로 구현하며 즉시 통합 테스트 가능
- **리팩토링 용이성**: 중간에 설계 변경 시 빠르게 대응 가능

---

## 5. 리스크

| 리스크 | 영향도 | 발생 가능성 | 대응 방안 |
|--------|--------|------------|----------|
| **iframe 제약으로 HTTP 상태 코드 접근 불가** | High | 확정됨 | 휴리스틱 방식(페이지 타이틀 기반) 사용. 정확도 제한 수용. |
| **휴리스틱 404/500 감지 정확도 낮음** | Medium | High | 대표 사이트(YouTube, Netflix, Google) 테스트 후 키워드 튜닝. 완벽한 정확도는 포기. |
| **에러 화면 애니메이션 성능 저하** | Medium | Low | GPU 가속 속성(opacity, transform) 사용. will-change 힌트 추가. |
| **Spotlight 포커스 이탈 문제** | Medium | Medium | Back 키 이벤트 핸들러로 포커스 트랩 구현. Moonstone 포커스 API 활용. |
| **에러 상태 탭 전환 시 유지 실패** | Low | Low | TabContext에 isError 플래그 저장. Phase 5에서 검증. |
| **homeUrl 미전달 버그** | Low | Low | BrowserView에서 homeUrl prop 전달 확인. PropTypes로 검증. |
| **테스트 커버리지 부족** | Medium | Medium | Phase 7에서 에러 시나리오 10개 이상 테스트 작성. 통합 테스트 우선. |

---

## 6. 예상 복잡도

### 전체 복잡도: **Medium**

| Phase | 복잡도 | 이유 |
|-------|--------|------|
| Phase 1 (errorClassifier) | Low | 단순 문자열 매칭 로직 |
| Phase 2 (ErrorPage) | Medium | Moonstone 컴포넌트 + Spotlight 포커스 관리 |
| Phase 3 (WebView 수정) | High | 에러 감지, 휴리스틱 로직, 상태 관리 통합 |
| Phase 4 (BrowserView) | Low | 단순 prop 전달 |
| Phase 5 (TabContext) | Low | 기존 구현 확인 (수정 최소) |
| Phase 6 (스타일링) | Low | CSS 애니메이션 |
| Phase 7 (테스트) | Medium | 통합 테스트 시나리오 다양 |
| Phase 8 (리뷰) | Low | 문서 일관성 검증 |

---

## 7. 테스트 전략

### 7.1 단위 테스트 (Jest)
**대상**: errorClassifier.js, ErrorPage 컴포넌트

| 테스트 항목 | 범위 |
|-----------|------|
| errorClassifier.classifyErrorByTitle() | 404, 500, null 케이스 |
| errorClassifier.getErrorMessage() | 모든 에러 코드 (-1, -2, -3, 404, 500, -99) |
| ErrorPage 버튼 클릭 | onRetry, onGoHome 콜백 호출 확인 |
| ErrorPage 렌더링 | errorMessage, url, 버튼 텍스트 표시 확인 |
| ErrorPage 초기 포커스 | 재시도 버튼에 포커스 확인 |

**목표 커버리지**: 90%

### 7.2 통합 테스트 (Enact Testing Library)
**대상**: WebView + ErrorPage 통합

| 테스트 시나리오 | 검증 항목 |
|---------------|----------|
| 네트워크 에러 | iframe onError → ErrorPage 렌더링 → errorCode=-1 |
| 타임아웃 에러 | 30초 경과 → ErrorPage 렌더링 → errorCode=-2 |
| 404 휴리스틱 | 페이지 타이틀 "404 Not Found" → errorCode=404 |
| 재시도 성공 | 재시도 버튼 클릭 → iframe 재로딩 → ErrorPage 제거 |
| 홈 이동 | 홈 버튼 클릭 → homeUrl로 이동 → ErrorPage 제거 |
| 에러 로깅 | 에러 발생 시 logger.error() 호출 확인 |

**목표 커버리지**: 85%

### 7.3 E2E 테스트 (수동, 프로젝터 실제 환경)
**대상**: 리모컨 입력 시나리오

| 시나리오 | 리모컨 입력 | 예상 결과 |
|----------|-------------|----------|
| 에러 발생 | 잘못된 URL 입력 | ErrorPage 표시, 재시도 버튼 포커스 |
| 재시도 | 선택 버튼 (재시도) | iframe 재로딩 시작 |
| 홈 이동 | 우 방향키 → 선택 버튼 (홈) | 홈페이지로 이동 |
| 에러 탭 전환 | 다른 탭 이동 → 에러 탭 복귀 | 에러 화면 유지 |
| 포커스 트랩 | Back 키 | 에러 화면 유지 (포커스 이탈 안 됨) |

**테스트 사이트**:
- 네트워크 에러: `http://invalid-domain-12345.com`
- 타임아웃: `https://httpstat.us/200?sleep=35000`
- 404: `https://www.google.com/404`
- 500: `https://httpstat.us/500`

---

## 8. 완료 기준 (Definition of Done)

### Phase별 완료 기준
- [x] Phase 1: errorClassifier.js 단위 테스트 통과, 커버리지 90%
- [x] Phase 2: ErrorPage 컴포넌트 독립 렌더링 가능, Storybook 테스트
- [x] Phase 3: WebView 에러 감지 → ErrorPage 렌더링 통합 완료
- [x] Phase 4: BrowserView → WebView homeUrl prop 전달 확인
- [x] Phase 5: TabContext isError 플래그 업데이트 확인
- [x] Phase 6: 3m 거리에서 대화면 가독성 테스트 통과
- [x] Phase 7: 단위 + 통합 테스트 85% 이상 커버리지
- [x] Phase 8: code-reviewer 승인, ESLint 통과

### 전체 기능 완료 기준
1. **요구사항 충족**: requirements.md의 수용 기준(AC-1~AC-10) 모두 충족
2. **테스트 통과**: `npm test` 모두 통과, 커버리지 85% 이상
3. **코드 품질**: ESLint 검사 통과, PropTypes 검증 완료
4. **문서화**: ErrorPage 컴포넌트 문서 작성 (`docs/components/ErrorPage.md`)
5. **실제 환경 테스트**: 프로젝터 실제 환경에서 리모컨 E2E 테스트 5개 시나리오 통과
6. **리뷰 승인**: code-reviewer의 Approve

---

## 9. 롤백 계획

### 롤백 트리거
- 테스트 실패율 30% 이상
- 프로젝터 실제 환경에서 Critical 버그 발견 (예: 앱 크래시)
- iframe 제약으로 인한 구현 불가 판단

### 롤백 절차
1. Phase 3~8 구현 코드 롤백
2. WebView.js 기존 인라인 에러 UI 복구 (377~397줄)
3. 요구사항 재검토 (iframe 제약 수용 범위 재조정)
4. 대안 설계 (예: 에러 타입 단순화, ErrorPage 제거)

---

## 10. 변경 이력

| 날짜 | 변경 내용 | 사유 |
|------|----------|------|
| 2026-02-13 | 초안 작성 | F-10 구현 계획 수립 |

---

## 다음 단계

이 구현 계획서를 기반으로 frontend-dev 에이전트가 Phase 1부터 순차적으로 구현합니다.

### 구현 시작 명령어
```
/fullstack-feature F-10 에러 처리
```

또는 수동으로 Phase별 구현:
```
Phase 1: errorClassifier.js 유틸리티 구현
Phase 2: ErrorPage 컴포넌트 구현
Phase 3: WebView 수정 (에러 감지, ErrorPage 통합)
...
```
