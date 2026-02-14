# 로딩 인디케이터 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: `docs/specs/loading-indicator/requirements.md`
- 기술 설계서: `docs/specs/loading-indicator/design.md`
- PRD: `docs/project/prd.md`
- 기능 백로그: `docs/project/features.md`

## 2. 구현 Phase

### Phase 1: LoadingBar 컴포넌트 기본 구조 생성
**담당**: frontend-dev
**예상 시간**: 30분
**의존성**: 없음 (F-02 완료됨)

#### 작업 내용
- [ ] `src/components/LoadingBar/` 디렉토리 생성
- [ ] `LoadingBar.js` 기본 구조 작성
  - Props 인터페이스 정의 (isLoading, isError, progress, onLoadComplete, 스타일 props)
  - PropTypes 정의
  - defaultProps 설정
- [ ] `LoadingBar.module.less` 생성
  - `.loadingBarContainer` 스타일 (width: 100%, height: 8px)
  - `.loadingBar` 스타일 (transform 기반 애니메이션)
  - `.error`, `.fadeOut` 클래스 정의
- [ ] `index.js` export 파일 생성

#### 완료 기준
- `import LoadingBar from '../components/LoadingBar'` 정상 동작
- PropTypes 검증 통과
- ESLint 경고 없음

#### 산출물
- `src/components/LoadingBar/LoadingBar.js`
- `src/components/LoadingBar/LoadingBar.module.less`
- `src/components/LoadingBar/index.js`

---

### Phase 2: 가상 진행률 알고리즘 구현
**담당**: frontend-dev
**예상 시간**: 1시간
**의존성**: Phase 1 완료

#### 작업 내용
- [ ] `calculateVirtualProgress(elapsedMs)` 함수 구현
  - 0~3초: 0% → 60% (빠른 증가)
  - 3~10초: 60% → 90% (중간 속도)
  - 10초 이후: 90% → 95% (매우 느림)
- [ ] requestAnimationFrame 기반 애니메이션 루프 구현
  - `rafRef` useRef로 애니메이션 ID 저장
  - `startTimeRef` useRef로 시작 시각 기록
  - `animate()` 재귀 함수 구현
- [ ] useEffect로 isLoading 감지 및 애니메이션 시작
  - isLoading=true 시 애니메이션 시작
  - isLoading=false 또는 isError=true 시 애니메이션 중단
- [ ] 95% 도달 시 애니메이션 중단 로직
- [ ] cleanup 함수로 cancelAnimationFrame 호출

#### 완료 기준
- 로딩 시작 시 프로그레스바가 0%에서 시작
- 3초 후 60%, 10초 후 90% 도달 확인 (브라우저 개발자 도구 로깅)
- 95% 도달 후 애니메이션 중단 확인
- 컴포넌트 unmount 시 타이머 정리 확인

#### 산출물
- `LoadingBar.js`에 가상 진행률 로직 추가 (상태: useState, useEffect, useRef)

---

### Phase 3: 로딩 완료 및 페이드아웃 처리
**담당**: frontend-dev
**예상 시간**: 45분
**의존성**: Phase 2 완료

#### 작업 내용
- [ ] useEffect로 isLoading=false 감지
  - virtualProgress > 0일 때만 처리
  - setVirtualProgress(100) 호출 (즉시 100% 이동)
- [ ] 100ms 후 페이드아웃 시작
  - setIsFadingOut(true) 호출
  - CSS opacity 전환 애니메이션 (0.5초)
- [ ] 페이드아웃 완료 후 (0.5초) 상태 초기화
  - setVirtualProgress(0)
  - setIsFadingOut(false)
  - onLoadComplete() 콜백 호출
- [ ] CSS 페이드아웃 애니메이션 추가
  - `.fadeOut` 클래스: `opacity: 0; transition: opacity 0.5s ease-out;`

#### 완료 기준
- 로딩 완료 시 프로그레스바가 100% 도달
- 0.1초 유지 후 페이드아웃 시작
- 0.5초 후 프로그레스바 사라짐
- onLoadComplete 콜백 호출 확인

#### 산출물
- `LoadingBar.js`에 완료 처리 로직 추가
- `LoadingBar.module.less`에 `.fadeOut` 스타일 추가

---

### Phase 4: 에러 상태 처리
**담당**: frontend-dev
**예상 시간**: 30분
**의존성**: Phase 2 완료

#### 작업 내용
- [ ] useEffect로 isError=true 감지
  - 애니메이션 중단 (cancelAnimationFrame)
  - 프로그레스바 색상 빨간색 변경
- [ ] 1초 후 페이드아웃
  - setTimeout으로 1초 대기
  - setVirtualProgress(0) 호출
- [ ] CSS 에러 스타일 추가
  - `.error` 클래스: `background-color: #d32f2f;`
- [ ] cleanup 함수로 타이머 정리

#### 완료 기준
- 에러 발생 시 프로그레스바가 빨간색으로 변경
- 1초 후 프로그레스바 사라짐
- 애니메이션 중단 확인

#### 산출물
- `LoadingBar.js`에 에러 처리 로직 추가
- `LoadingBar.module.less`에 `.error` 스타일 추가

---

### Phase 5: LoadingSpinner 컴포넌트 분리 (WebView 내부)
**담당**: frontend-dev
**예상 시간**: 1시간
**의존성**: 없음 (F-02 WebView 구조 활용)

#### 작업 내용
- [ ] `src/components/WebView/LoadingSpinner.js` 생성
  - Enact `@enact/moonstone/Spinner` import
  - Props: `url`, `isError`, `loadingStartTime`
- [ ] 로딩 스피너 렌더링
  - Enact Spinner 사용 (size="large")
  - "페이지 로딩 중..." 텍스트
  - URL 표시 (최대 50자, 말줄임)
- [ ] 에러 아이콘 표시
  - isError=true 시 스피너 대신 ⚠️ 아이콘
  - "페이지 로딩 실패" 텍스트
- [ ] 30초 타임아웃 메시지 로직
  - useEffect로 loadingStartTime 감지
  - 30초 타이머 설정
  - 타임아웃 시 "로딩이 오래 걸리고 있습니다" 메시지
- [ ] `LoadingSpinner.module.less` 스타일 작성
  - `.spinnerOverlay`: 전체 화면 반투명 오버레이
  - `.spinner`: 80px 크기
  - `.errorIcon`: 80px 폰트 크기
  - `.loadingText`: 24px 폰트 (가독성 기준)
  - `.loadingUrl`: 18px 폰트, 80% 너비

#### 완료 기준
- 로딩 중 화면 중앙에 스피너 표시
- 스피너가 부드럽게 회전
- 30초 초과 시 타임아웃 메시지 표시
- 에러 시 에러 아이콘 표시

#### 산출물
- `src/components/WebView/LoadingSpinner.js`
- `src/components/WebView/LoadingSpinner.module.less`

---

### Phase 6: WebView에 LoadingSpinner 통합
**담당**: frontend-dev
**예상 시간**: 30분
**의존성**: Phase 5 완료

#### 작업 내용
- [ ] `src/components/WebView/WebView.js` 수정
  - LoadingSpinner import 추가
  - 기존 인라인 스피너 JSX를 LoadingSpinner 컴포넌트로 교체
  - loadingStartTime prop 전달 (loadStartTimeRef.current)
- [ ] state='loading' 시 LoadingSpinner 렌더링
  - `<LoadingSpinner url={currentUrlRef.current} isError={false} loadingStartTime={loadStartTimeRef.current} />`
- [ ] state='error' 시 LoadingSpinner 에러 모드
  - `<LoadingSpinner url={currentUrlRef.current} isError={true} loadingStartTime={null} />`

#### 완료 기준
- WebView 로딩 시 LoadingSpinner 표시
- 로딩 완료 시 스피너 사라짐
- 에러 시 에러 아이콘 표시

#### 산출물
- `src/components/WebView/WebView.js` 수정

---

### Phase 7: BrowserView에 LoadingBar 통합
**담당**: frontend-dev
**예상 시간**: 1시간
**의존성**: Phase 3, Phase 4 완료

#### 작업 내용
- [ ] `src/views/BrowserView.js` 수정
  - LoadingBar import 추가
  - isLoading, isError 상태 추가 (useState)
  - handleLoadStart, handleLoadEnd, handleLoadError 함수 수정
    - handleLoadStart: `setIsLoading(true)`, `setIsError(false)`
    - handleLoadEnd: `setIsLoading(false)`, `setIsError(false)`
    - handleLoadError: `setIsLoading(false)`, `setIsError(true)`
  - handleLoadComplete 함수 추가 (로깅용)
- [ ] JSX에 LoadingBar 추가
  - URLBar 아래에 `<LoadingBar isLoading={isLoading} isError={isError} onLoadComplete={handleLoadComplete} />` 배치
- [ ] `src/views/BrowserView.module.less` 레이아웃 조정
  - URLBar 아래 LoadingBar 배치 (별도 공간 차지하지 않도록 절대 위치 고려)

#### 완료 기준
- BrowserView에서 페이지 로딩 시 LoadingBar 표시
- 로딩 완료 시 LoadingBar 100% 도달 후 사라짐
- 에러 시 LoadingBar 빨간색 변경

#### 산출물
- `src/views/BrowserView.js` 수정
- `src/views/BrowserView.module.less` 레이아웃 조정

---

### Phase 8: 애니메이션 최적화
**담당**: frontend-dev
**예상 시간**: 45분
**의존성**: Phase 2 완료

#### 작업 내용
- [ ] CSS `will-change: transform` 추가
  - `LoadingBar.module.less`의 `.loadingBar` 클래스에 추가
- [ ] 불필요한 렌더링 방지
  - animate() 함수에서 진행률이 0.5% 이상 변경된 경우에만 setState 호출
  ```javascript
  if (Math.abs(newProgress - virtualProgress) > 0.5) {
    setVirtualProgress(newProgress)
  }
  ```
- [ ] requestAnimationFrame 폴백 추가 (webOS 호환성)
  - `typeof requestAnimationFrame === 'undefined'` 체크
  - 폴백: setInterval 사용
- [ ] 성능 로깅 추가 (개발 모드)
  - `performance.now()`로 프레임 시간 측정
  - 33ms(30fps) 초과 시 경고 로그

#### 완료 기준
- 프로그레스바 애니메이션이 부드럽게 동작 (30fps 이상)
- 불필요한 리렌더링 최소화 확인 (React DevTools Profiler)
- webOS 프로젝터에서 GPU 가속 확인 (Chrome DevTools Performance 탭)

#### 산출물
- `LoadingBar.js` 최적화 로직 추가
- `LoadingBar.module.less`에 `will-change` 추가

---

### Phase 9: 로컬 환경 테스트
**담당**: frontend-dev
**예상 시간**: 1시간
**의존성**: Phase 7 완료

#### 작업 내용
- [ ] `npm run serve` 실행
- [ ] 브라우저(http://localhost:8080)에서 테스트
  - 프로그레스바 표시 확인
  - 가상 진행률 증가 확인 (개발자 도구 Console에 로깅)
  - 로딩 완료 시 100% 이동 및 페이드아웃 확인
  - 에러 시 빨간색 변경 확인
- [ ] 다양한 시나리오 테스트
  - 빠른 로딩 (캐시 히트): 즉시 100% 이동
  - 느린 로딩 (대용량 페이지): 90%까지만 증가
  - 에러 (잘못된 URL): 빨간색 프로그레스바 + 에러 아이콘
- [ ] Chrome DevTools Performance 탭에서 프레임레이트 측정
  - 30fps 이상 유지 확인
- [ ] ESLint 검사 (`npm run lint`)
  - 경고 없음 확인

#### 완료 기준
- 모든 시나리오에서 LoadingBar 정상 동작
- 애니메이션이 부드럽고 끊기지 않음
- ESLint 경고 없음

---

### Phase 10: 실제 디바이스 테스트
**담당**: frontend-dev
**예상 시간**: 1.5시간
**의존성**: Phase 9 완료

#### 작업 내용
- [ ] 프로덕션 빌드
  - `npm run pack-p` 실행
  - `ares-package dist/` IPK 생성
- [ ] LG 프로젝터 HU175QW 배포
  - `ares-install --device projector {ipk파일}`
  - `ares-launch --device projector com.jsong.webosbrowser`
- [ ] 대화면 시인성 테스트
  - 100인치 이상 화면에서 3m 거리에서 프로그레스바 시인성 확인
  - 프로그레스바 높이 8px가 적절한지 확인 (필요 시 12px로 조정)
- [ ] 주요 웹사이트 로딩 테스트
  - Google: 빠른 로딩 (1~2초)
  - YouTube: 중간 속도 (3~5초)
  - Naver: 중간 속도 (3~5초)
  - Netflix: 느린 로딩 (5~10초)
- [ ] 애니메이션 프레임레이트 확인
  - 부드러운 움직임 확인 (육안 검사)
  - 프로젝터 성능 제약으로 끊김 발생 시 최적화 필요
- [ ] 30초 타임아웃 테스트
  - 네트워크 속도를 인위적으로 제한하여 30초 초과 상황 재현
  - 타임아웃 메시지 표시 확인
- [ ] 리모컨 조작 테스트
  - 로딩 중 Back 버튼으로 로딩 취소 가능 확인 (F-04 연동)

#### 완료 기준
- 프로젝터 대화면에서 프로그레스바가 명확히 보임
- 주요 웹사이트 로딩 시 정상 동작
- 애니메이션이 부드럽게 동작 (30fps 이상)
- 30초 타임아웃 메시지 표시 정상

#### 산출물
- IPK 파일 (`com.jsong.webosbrowser_0.1.0_all.ipk`)
- 디바이스 테스트 로그 (ares-log)

---

### Phase 11: 성능 측정 및 최적화
**담당**: frontend-dev
**예상 시간**: 1시간
**의존성**: Phase 10 완료

#### 작업 내용
- [ ] requestAnimationFrame 프레임 시간 측정
  - `performance.now()`로 각 프레임 시간 로깅
  - 평균 프레임 시간 33ms(30fps) 이하 확인
- [ ] 메모리 사용량 측정
  - Chrome DevTools Memory 탭에서 Heap Snapshot 비교
  - LoadingBar 추가로 5MB 이하 증가 확인
- [ ] CPU 사용률 측정
  - Chrome DevTools Performance 탭에서 CPU 사용률 확인
  - 애니메이션 중 10% 이하 유지 확인
- [ ] 성능 이슈 발견 시 최적화
  - requestAnimationFrame 간격 조정 (예: 16ms → 33ms)
  - 불필요한 렌더링 추가 방지
  - CSS 애니메이션 복잡도 감소
- [ ] 느린 페이지 로딩 시 90% 대기 동작 확인
  - 10초 이상 로딩 시 프로그레스바가 90%에서 대기하는지 확인

#### 완료 기준
- 프레임레이트 30fps 이상 유지
- 메모리 사용량 5MB 이하 증가
- CPU 사용률 10% 이하
- 성능 요구사항 충족 확인

---

### Phase 12: 문서 작성 및 검증
**담당**: frontend-dev
**예상 시간**: 1시간
**의존성**: Phase 11 완료

#### 작업 내용
- [ ] LoadingBar 컴포넌트 문서 작성
  - `docs/components/LoadingBar.md` 생성
  - Props 인터페이스 문서화
  - 사용 예시 코드
  - 가상 진행률 알고리즘 설명
  - 주의사항 (iframe 제약, 성능 최적화 팁)
- [ ] LoadingSpinner 컴포넌트 문서 작성
  - `docs/components/LoadingSpinner.md` 생성
  - Props 인터페이스
  - 30초 타임아웃 로직 설명
- [ ] PropTypes 검증 확인
  - 모든 컴포넌트에 PropTypes 정의 완료
  - 타입 불일치 시 Console 경고 확인
- [ ] ESLint 최종 검사
  - `npm run lint` 실행
  - 경고 없음 확인

#### 완료 기준
- 컴포넌트 문서 작성 완료
- PropTypes 정의 완료
- ESLint 경고 없음

#### 산출물
- `docs/components/LoadingBar.md`
- `docs/components/LoadingSpinner.md`

---

### Phase 13: 테스트 작성 및 리뷰 (test-runner, code-reviewer)
**담당**: test-runner → code-reviewer
**예상 시간**: 1.5시간
**의존성**: Phase 12 완료

#### 작업 내용 (test-runner)
- [ ] LoadingBar 단위 테스트 작성 (`src/components/LoadingBar/LoadingBar.test.js`)
  - calculateVirtualProgress 함수 테스트 (0초, 3초, 10초 입력 시 예상 진행률)
  - isLoading=true 시 프로그레스바 렌더링 확인
  - isLoading=false 시 100% 이동 확인
  - isError=true 시 빨간색 변경 확인
- [ ] LoadingSpinner 단위 테스트 작성
  - 30초 타임아웃 메시지 표시 확인
  - isError=true 시 에러 아이콘 표시 확인
- [ ] 테스트 실행 (`npm test`)
  - 모든 테스트 통과 확인

#### 작업 내용 (code-reviewer)
- [ ] 코드 리뷰 체크리스트 검증
  - PropTypes 정의 완료
  - 주석 언어 한국어 확인
  - 변수명 camelCase 확인
  - 탭 들여쓰기 확인
  - 세미콜론 없음 확인
  - console.log 대신 logger.js 사용 확인
- [ ] 컴포넌트 문서 리뷰
  - Props 인터페이스 정확성 확인
  - 사용 예시 코드 실행 가능성 확인
- [ ] 설계서 구현 일치 확인
  - design.md의 아키텍처 결정 사항 준수 확인
  - 가상 진행률 알고리즘 일치 확인
  - 성능 요구사항 충족 확인

#### 완료 기준
- 모든 단위 테스트 통과
- 코드 리뷰 통과 (승인)
- 컴포넌트 문서 검증 완료

---

## 3. 태스크 의존성

```
Phase 1 (LoadingBar 기본 구조)
  │
  ├─▶ Phase 2 (가상 진행률 알고리즘)
  │     │
  │     ├─▶ Phase 3 (로딩 완료 & 페이드아웃)
  │     │     │
  │     │     └─▶ Phase 7 (BrowserView 통합)
  │     │           │
  │     │           └─▶ Phase 9 (로컬 테스트)
  │     │                 │
  │     │                 └─▶ Phase 10 (디바이스 테스트)
  │     │                       │
  │     │                       └─▶ Phase 11 (성능 측정)
  │     │                             │
  │     │                             └─▶ Phase 12 (문서 작성)
  │     │                                   │
  │     │                                   └─▶ Phase 13 (테스트 & 리뷰)
  │     │
  │     ├─▶ Phase 4 (에러 처리) ──┐
  │     │                          │
  │     └─▶ Phase 8 (애니메이션 최적화) ──┘
  │
  └─▶ Phase 5 (LoadingSpinner 분리)
        │
        └─▶ Phase 6 (WebView 통합) ──┘
```

**병렬 실행 가능 구간**:
- **Phase 5, 6 (LoadingSpinner)**: Phase 1 완료 후 즉시 시작 가능 (Phase 2~4와 독립)
- **Phase 8 (애니메이션 최적화)**: Phase 2 완료 후 Phase 3, 4와 병렬 실행 가능
- **Phase 4 (에러 처리)**: Phase 2 완료 후 Phase 3과 병렬 실행 가능

## 4. 병렬 실행 판단

### Agent Team 사용 권장 여부
**No** (순차 실행 권장)

### 이유
1. **컴포넌트 간 의존성이 높음**:
   - LoadingBar와 LoadingSpinner가 모두 BrowserView에서 통합되므로 최종 통합 시점에서 조율 필요
   - Phase 7에서 BrowserView.js를 수정하는데, LoadingBar와 LoadingSpinner 모두 영향 받음
2. **병렬 작업 효율성 낮음**:
   - LoadingSpinner는 Phase 5~6으로 1.5시간 소요 (Agent Team 설정 오버헤드보다 작음)
   - 병렬 작업 시 Worktree 관리, Merge 복잡도 증가로 시간 절약 효과 미미
3. **테스트 의존성**:
   - Phase 9 (로컬 테스트)에서 LoadingBar와 LoadingSpinner를 동시에 테스트해야 함
   - 하나의 팀원이 순차적으로 테스트하는 것이 효율적
4. **단일 개발자 작업량 적정**:
   - 전체 작업 시간 약 10~12시간으로 단일 개발자가 1~2일 내 완료 가능

### 예외 상황
만약 F-03 (URL 입력 UI), F-04 (페이지 탐색 컨트롤)과 함께 병렬 그룹(PG-1)으로 개발한다면:
- F-03: URLBar 컴포넌트 (Agent 1)
- F-04: NavigationBar 컴포넌트 (Agent 2)
- F-05: LoadingBar + LoadingSpinner (Agent 3)
- 각 기능이 독립된 컴포넌트로 충돌 없음 → Agent Team 사용 가능

## 5. 리스크 및 대응 방안

| 리스크 | 영향도 | 발생 가능성 | 대응 방안 |
|--------|--------|-------------|-----------|
| **iframe 실제 진행률 제약** | 중 | 확정 | 가상 진행률 알고리즘으로 체감 속도 최적화 (설계서 반영됨) |
| **webOS 프로젝터 성능 제약** | 중 | 높음 | GPU 가속(transform, opacity), will-change 힌트 사용, 필요 시 requestAnimationFrame 간격 조정 |
| **requestAnimationFrame 미지원** | 낮 | 낮음 | setInterval 폴백 추가 (Phase 8에서 구현) |
| **30초 타임아웃 미동작** | 낮 | 중 | LoadingSpinner에 useEffect cleanup 확인, 타이머 정리 누락 방지 |
| **프로그레스바 시인성 부족** | 중 | 중 | 디바이스 테스트(Phase 10)에서 확인 후 높이 8px → 12px 조정 |
| **애니메이션 끊김 (30fps 미달)** | 중 | 중 | 불필요한 렌더링 방지 (0.5% 이상 변경 시에만 업데이트), CPU 사용률 모니터링 |
| **색맹 사용자 접근성** | 낮 | 낮음 | 에러 시 색상 + 아이콘 조합 (설계서 반영됨) |
| **탭 관리(F-06) 연동 이슈** | 낮 | 낮음 | LoadingBar를 독립 컴포넌트로 설계하여 재사용 가능하도록 준비됨 |

### Critical 리스크
**없음**. 모든 리스크는 대응 방안이 마련되어 있으며, 기술 설계서에서 충분히 검토됨.

## 6. 검증 계획

### 6.1 단위 테스트 (Jest)
**담당**: test-runner
**파일**: `src/components/LoadingBar/LoadingBar.test.js`, `src/components/WebView/LoadingSpinner.test.js`

#### LoadingBar 테스트
- [ ] calculateVirtualProgress(0) === 0
- [ ] calculateVirtualProgress(3000) === 60
- [ ] calculateVirtualProgress(10000) === 90
- [ ] calculateVirtualProgress(30000) === 95
- [ ] isLoading=true 시 프로그레스바 렌더링 확인
- [ ] isLoading=false 시 virtualProgress=100 확인
- [ ] isError=true 시 `.error` 클래스 적용 확인
- [ ] onLoadComplete 콜백 호출 확인

#### LoadingSpinner 테스트
- [ ] isError=false 시 Spinner 렌더링 확인
- [ ] isError=true 시 에러 아이콘(⚠️) 렌더링 확인
- [ ] loadingStartTime + 30초 후 타임아웃 메시지 확인
- [ ] url prop이 50자 초과 시 말줄임 확인

### 6.2 통합 테스트 (BrowserView)
**담당**: frontend-dev
**환경**: 로컬 개발 서버 (npm run serve)

- [ ] WebView 로딩 시작 → LoadingBar 표시 확인
- [ ] WebView 로딩 완료 → LoadingBar 100% 도달 후 사라짐 확인
- [ ] WebView 로딩 실패 → LoadingBar 빨간색 변경 확인
- [ ] LoadingSpinner가 WebView 중앙에 오버레이로 표시 확인

### 6.3 E2E 테스트 (실제 디바이스)
**담당**: frontend-dev
**환경**: LG 프로젝터 HU175QW

#### 시나리오 1: 빠른 페이지 로딩 (캐시 히트)
1. Google 홈페이지 로드 (1~2초)
2. LoadingBar가 60% → 100% 빠르게 이동
3. 0.5초 후 페이드아웃

#### 시나리오 2: 느린 페이지 로딩 (대용량 페이지)
1. YouTube 홈페이지 로드 (5~10초)
2. LoadingBar가 3초 후 60%, 10초 후 90% 도달
3. 로딩 완료 시 100% 이동 후 페이드아웃

#### 시나리오 3: 매우 느린 페이지 로딩 (30초 초과)
1. 네트워크 속도 제한 (테스트 환경 설정)
2. LoadingBar가 90%까지 증가 후 대기
3. 30초 경과 시 LoadingSpinner에 타임아웃 메시지 표시
4. 로딩 완료 또는 취소 시 정상 처리

#### 시나리오 4: 페이지 로딩 실패 (404)
1. 잘못된 URL 입력 (예: http://invalidurl.com)
2. LoadingBar가 빨간색으로 변경
3. LoadingSpinner가 에러 아이콘(⚠️)으로 변경
4. 1초 후 ErrorOverlay 표시

#### 시나리오 5: 연속 로딩 (URL 빠르게 변경)
1. Google → YouTube → Naver 연속 입력
2. 이전 LoadingBar 애니메이션이 취소되고 새 로딩 시작
3. 메모리 누수 없이 정상 동작

### 6.4 성능 테스트
**담당**: frontend-dev
**환경**: LG 프로젝터 HU175QW + Chrome DevTools (ares-inspect)

- [ ] requestAnimationFrame 평균 프레임 시간 33ms(30fps) 이하 확인
- [ ] LoadingBar 메모리 사용량 5MB 이하 확인 (Heap Snapshot)
- [ ] 애니메이션 중 CPU 사용률 10% 이하 확인 (Performance 탭)
- [ ] GPU 가속 활성화 확인 (Layers 탭에서 transform 레이어 확인)

### 6.5 접근성 테스트
**담당**: code-reviewer

- [ ] 프로그레스바 높이 8px 이상 (시인성)
- [ ] 스피너 크기 80px 이상 (대화면 가독성)
- [ ] 텍스트 폰트 크기 24px 이상 (PRD 기준)
- [ ] 에러 시 색상 + 아이콘 조합 (색맹 대응)
- [ ] 명도 대비 4.5:1 이상 (WCAG AA 기준)

### 6.6 완료 기준 (Acceptance Criteria)
**모든 항목 체크 완료 시 F-05 완료 판정**

- [ ] AC-1: 프로그레스바 정상 동작 (표시, 증가, 페이드아웃)
- [ ] AC-2: 스피너 정상 동작 (회전, 텍스트, 사라짐)
- [ ] AC-3: 가상 진행률 알고리즘 작동 (3초→60%, 10초→90%)
- [ ] AC-4: 에러 상태 시각적 피드백 (빨간색, 에러 아이콘)
- [ ] AC-5: 성능 요구사항 충족 (30fps, 5MB, 100ms 이내 표시)
- [ ] AC-6: 다양한 시나리오 테스트 통과
- [ ] AC-7: 리모컨 조작 통합 (Back 버튼 취소)
- [ ] AC-8: 실제 디바이스 테스트 통과 (시인성, 주요 사이트)
- [ ] AC-9: 코드 품질 (PropTypes, ESLint, 문서)

## 7. 산출물 체크리스트

### 구현 파일
- [ ] `src/components/LoadingBar/LoadingBar.js` (250줄 예상)
- [ ] `src/components/LoadingBar/LoadingBar.module.less` (80줄 예상)
- [ ] `src/components/LoadingBar/index.js` (1줄)
- [ ] `src/components/WebView/LoadingSpinner.js` (100줄 예상)
- [ ] `src/components/WebView/LoadingSpinner.module.less` (60줄 예상)
- [ ] `src/views/BrowserView.js` (수정, +30줄)
- [ ] `src/views/BrowserView.module.less` (수정, +10줄)
- [ ] `src/components/WebView/WebView.js` (수정, +10줄)

### 테스트 파일
- [ ] `src/components/LoadingBar/LoadingBar.test.js` (150줄 예상)
- [ ] `src/components/WebView/LoadingSpinner.test.js` (100줄 예상)

### 문서 파일
- [ ] `docs/components/LoadingBar.md` (컴포넌트 문서)
- [ ] `docs/components/LoadingSpinner.md` (컴포넌트 문서)

### 검증 완료
- [ ] ESLint 경고 없음
- [ ] PropTypes 정의 완료
- [ ] 단위 테스트 통과
- [ ] 통합 테스트 통과
- [ ] E2E 테스트 통과
- [ ] 성능 테스트 통과
- [ ] 코드 리뷰 승인

## 8. 예상 총 소요 시간

| Phase | 시간 | 누적 |
|-------|------|------|
| Phase 1: LoadingBar 기본 구조 | 30분 | 0.5h |
| Phase 2: 가상 진행률 알고리즘 | 1시간 | 1.5h |
| Phase 3: 로딩 완료 & 페이드아웃 | 45분 | 2.25h |
| Phase 4: 에러 처리 | 30분 | 2.75h |
| Phase 5: LoadingSpinner 분리 | 1시간 | 3.75h |
| Phase 6: WebView 통합 | 30분 | 4.25h |
| Phase 7: BrowserView 통합 | 1시간 | 5.25h |
| Phase 8: 애니메이션 최적화 | 45분 | 6h |
| Phase 9: 로컬 테스트 | 1시간 | 7h |
| Phase 10: 디바이스 테스트 | 1.5시간 | 8.5h |
| Phase 11: 성능 측정 | 1시간 | 9.5h |
| Phase 12: 문서 작성 | 1시간 | 10.5h |
| Phase 13: 테스트 & 리뷰 | 1.5시간 | 12h |

**총 예상 시간**: 12시간 (1.5일)

## 9. 다음 단계 (F-05 완료 후)

### F-03 (URL 입력 UI) 또는 F-04 (페이지 탐색 컨트롤)
- F-05 완료 후 PG-1 그룹의 다른 기능 개발 가능
- F-03, F-04는 F-05와 독립적이므로 순서 무관
- features.md에서 다음 기능 선택 (/next-feature 명령어)

### 병렬 그룹 PG-1 완료 후
- M1 마일스톤의 모든 기능 (F-01~F-05) 완료
- M2 마일스톤 시작: F-06 (탭 관리) 개발

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-12 | 최초 작성 | F-05 요구사항 및 설계서 기반 구현 계획 수립 |
