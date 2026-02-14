# 웹뷰 통합 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: `docs/specs/webview-integration/requirements.md`
- 기술 설계서: `docs/specs/webview-integration/design.md`
- PRD: `docs/project/prd.md`
- 기능 백로그: `docs/project/features.md`

## 2. 구현 개요

### 목표
webOS 플랫폼에서 외부 웹 페이지를 렌더링하기 위한 WebView 컴포넌트를 구현하고 BrowserView에 통합합니다. iframe 기반으로 webOS 4.x 호환성을 보장하며, 로딩 이벤트 처리, 에러 핸들링, 리모컨 포커스 통합을 포함합니다.

### 작업 범위
- WebView 컴포넌트 구현 (iframe 기반)
- 로딩 상태 관리 (idle → loading → loaded/error)
- 에러 처리 (네트워크 에러, 타임아웃)
- Enact Spotlight 통합 (리모컨 포커스)
- BrowserView에 통합
- 실제 디바이스 테스트

### 작업 방식
**순차 실행** (Agent Team 사용 불필요)
- 이유: WebView 컴포넌트 1개 구현으로 작업 분리 불가능
- Frontend 단일 작업이므로 `frontend-dev` 서브에이전트가 순차 진행

## 3. 구현 Phase

### Phase 1: WebView 컴포넌트 기본 구조 생성
**담당**: frontend-dev
**예상 소요**: 30분

**태스크**:
- [ ] T-1.1: `src/components/WebView/` 디렉토리 생성
- [ ] T-1.2: `WebView.js` 생성 (Props 인터페이스, PropTypes 정의)
- [ ] T-1.3: `WebView.module.less` 생성 (기본 스타일)
- [ ] T-1.4: `index.js` 생성 (export default)

**산출물**:
```
src/components/WebView/
├── WebView.js
├── WebView.module.less
└── index.js
```

**완료 기준**:
- Props 인터페이스 정의 (url, onLoadStart, onLoadEnd, onLoadError, onNavigationChange, onStateChange, className, sandbox, allow)
- PropTypes 검증 추가
- defaultProps 설정 (빈 콜백 함수, 기본 sandbox/allow 값)
- index.js에서 WebView export

**참조**:
- 설계서 § 4. WebView 컴포넌트 설계 (Props 인터페이스)

---

### Phase 2: iframe 기본 렌더링 구현
**담당**: frontend-dev
**예상 소요**: 1시간

**태스크**:
- [ ] T-2.1: iframe 요소 추가 (ref, src, sandbox, allow 속성 설정)
- [ ] T-2.2: useState로 상태 관리 (state: 'idle'|'loading'|'loaded'|'error')
- [ ] T-2.3: useRef로 참조 관리 (iframeRef, timeoutRef, currentUrlRef, loadStartTimeRef)
- [ ] T-2.4: useEffect로 URL 변경 감지 및 로딩 시작
- [ ] T-2.5: 기본 스타일링 (전체 화면 채우기, iframe border 제거)

**산출물**:
- iframe 렌더링 기능
- 상태 관리 로직

**완료 기준**:
- iframe이 props.url을 src로 받아 렌더링됨
- url 변경 시 useEffect가 감지하여 state를 'loading'으로 전환
- currentUrlRef.current에 url 저장
- sandbox 속성: `allow-scripts allow-same-origin allow-forms allow-popups-to-escape-sandbox`
- allow 속성: `autoplay; fullscreen; encrypted-media`
- iframe이 부모 컨테이너 전체를 채우도록 스타일링 (width: 100%, height: 100%, border: none)

**참조**:
- 설계서 § 4. WebView 컴포넌트 설계 (Props 인터페이스, 상태 정의)
- 설계서 § 10. 구현 순서 Phase 2

---

### Phase 3: 로딩 이벤트 처리 구현
**담당**: frontend-dev
**예상 소요**: 1시간

**태스크**:
- [ ] T-3.1: onLoad 이벤트 핸들러 구현 (handleLoad)
- [ ] T-3.2: onLoadStart 콜백 호출 (useEffect에서 url 변경 시)
- [ ] T-3.3: onLoadEnd 콜백 호출 (로딩 시간 계산, 페이지 제목 추출 시도)
- [ ] T-3.4: 상태 전이 로직 구현 (idle → loading → loaded)
- [ ] T-3.5: 로딩 시간 계산 (Date.now() - loadStartTime)
- [ ] T-3.6: 페이지 제목 추출 (iframe.contentDocument.title, CORS 제약 시 기본값 사용)

**산출물**:
- 로딩 이벤트 핸들러
- 콜백 호출 로직

**완료 기준**:
- url 변경 시 onLoadStart() 호출
- iframe onLoad 이벤트 발생 시:
  - state를 'loaded'로 전환
  - 로딩 소요 시간 계산 (duration = Date.now() - loadStartTime)
  - 페이지 제목 추출 시도 (CORS 에러 시 url을 제목으로 사용)
  - onLoadEnd({ url, title, duration }) 호출
  - 타임아웃 타이머 정리 (clearTimeout)
- 콘솔에 로딩 이벤트 로그 출력 (개발 모드)

**참조**:
- 설계서 § 5. 시퀀스 흐름 설계 (정상 페이지 로딩)
- 설계서 § 10. 구현 순서 Phase 3

---

### Phase 4: 에러 처리 구현
**담당**: frontend-dev
**예상 소요**: 1.5시간

**태스크**:
- [ ] T-4.1: onError 이벤트 핸들러 구현 (handleError)
- [ ] T-4.2: 타임아웃 로직 구현 (30초 초과 시 에러 처리)
- [ ] T-4.3: 에러 타입 분류 유틸 생성 (errorTypes.js)
- [ ] T-4.4: onLoadError 콜백 호출
- [ ] T-4.5: ErrorOverlay 컴포넌트 생성 (간단한 에러 표시)
- [ ] T-4.6: 재시도 기능 구현 (ErrorOverlay의 "다시 시도" 버튼)

**산출물**:
```
src/components/WebView/
├── errorTypes.js
├── ErrorOverlay.js
```

**완료 기준**:
- iframe onError 이벤트 발생 시:
  - state를 'error'로 전환
  - 에러 정보 생성 (errorCode, errorMessage, url)
  - onLoadError({ errorCode, errorMessage, url }) 호출
  - 타임아웃 타이머 정리
- 로딩 시작 후 30초 경과 시:
  - state가 여전히 'loading'이면 타임아웃 에러로 처리
  - errorCode: -2, errorMessage: '페이지 로딩 시간이 초과되었습니다 (30초)'
- ErrorOverlay 표시 (state === 'error'일 때)
- ErrorOverlay에 "다시 시도" 버튼 (클릭 시 url을 다시 로드)
- 에러 타입 분류: NETWORK_ERROR(-1), TIMEOUT_ERROR(-2), HTTP_404(404), HTTP_500(500), CORS_ERROR(-3), UNKNOWN_ERROR(-999)

**참조**:
- 설계서 § 5. 시퀀스 흐름 설계 (에러 시나리오 1, 2)
- 설계서 § 7. 에러 처리 설계
- 설계서 § 10. 구현 순서 Phase 4

---

### Phase 5: Enact Spotlight 통합
**담당**: frontend-dev
**예상 소요**: 1시간

**태스크**:
- [ ] T-5.1: Spottable HOC로 WebView 컨테이너 감싸기
- [ ] T-5.2: onSpotlightFocus 핸들러 추가 (콘솔 로그)
- [ ] T-5.3: onSpotlightBlur 핸들러 추가 (콘솔 로그)
- [ ] T-5.4: spotlightId 설정 ('webview-main')
- [ ] T-5.5: 로컬 테스트 (Tab 키로 포커스 이동 확인)

**산출물**:
- Spotlight 통합 WebView 컴포넌트

**완료 기준**:
- WebView 컨테이너가 Spottable로 감싸짐
- Tab 키로 WebView로 포커스 이동 가능 (로컬 테스트)
- 포커스 진입 시 onSpotlightFocus 호출 (콘솔 로그 확인)
- 포커스 이탈 시 onSpotlightBlur 호출 (콘솔 로그 확인)
- iframe 내부는 웹 페이지 자체 포커스 관리 사용 (Spotlight 간섭 없음)

**참조**:
- 설계서 § 6. Enact Spotlight 통합 설계
- 설계서 § 10. 구현 순서 Phase 5

---

### Phase 6: 네비게이션 감지 구현 (선택)
**담당**: frontend-dev
**예상 소요**: 1시간

**태스크**:
- [ ] T-6.1: setInterval로 iframe.contentWindow.location 폴링 (500ms 간격)
- [ ] T-6.2: URL 변경 감지 (currentUrlRef와 비교)
- [ ] T-6.3: onNavigationChange 콜백 호출
- [ ] T-6.4: CORS 에러 핸들링 (try-catch, Same-Origin Policy 위반 시 무시)

**산출물**:
- 네비게이션 감지 로직

**완료 기준**:
- iframe 내부 페이지에서 링크 클릭 시 URL 변경 감지 (Same-Origin인 경우만)
- onNavigationChange({ url: newUrl }) 호출
- CORS 에러 발생 시 무시 (콘솔 경고 없음)
- 컴포넌트 unmount 시 setInterval 정리

**참조**:
- 설계서 § 5. 시퀀스 흐름 설계 (네비게이션 이벤트 감지)
- 설계서 § 10. 구현 순서 Phase 6
- 설계서 § 11. 기술적 주의사항 (iframe Same-Origin Policy)

**주의**:
이 기능은 CORS 제약으로 제한적으로 작동합니다. 대부분의 외부 사이트는 Same-Origin Policy로 접근 차단됩니다. F-04(페이지 탐색)에서 뒤로/앞으로 버튼으로 보완 예정이므로, 이 Phase는 선택적으로 구현합니다.

---

### Phase 7: 메모리 관리 구현
**담당**: frontend-dev
**예상 소요**: 30분

**태스크**:
- [ ] T-7.1: useEffect cleanup 함수 구현 (컴포넌트 unmount 시)
- [ ] T-7.2: 타임아웃 타이머 정리 (clearTimeout)
- [ ] T-7.3: 네비게이션 폴링 정리 (clearInterval, Phase 6 구현 시)
- [ ] T-7.4: iframe src를 about:blank로 설정 (리소스 해제)
- [ ] T-7.5: 메모리 모니터링 로거 추가 (src/utils/logger.js)

**산출물**:
```
src/utils/logger.js
```

**완료 기준**:
- 컴포넌트 unmount 시:
  - timeoutRef.current 존재 시 clearTimeout 호출
  - navigationIntervalRef.current 존재 시 clearInterval 호출
  - iframeRef.current.src = 'about:blank' 설정
- 개발 모드에서 메모리 사용량 로깅:
  - Mount 시: `logMemoryUsage('WebView Mount')`
  - Unmount 시: `logMemoryUsage('WebView Unmount')`
  - performance.memory API 사용 (지원 시)

**참조**:
- 설계서 § 8. 메모리 관리 전략
- 설계서 § 10. 구현 순서 Phase 7

---

### Phase 8: BrowserView 통합
**담당**: frontend-dev
**예상 소요**: 1시간

**태스크**:
- [ ] T-8.1: `src/views/BrowserView/BrowserView.js` 수정
- [ ] T-8.2: WebView 컴포넌트 import
- [ ] T-8.3: useState로 currentUrl 상태 관리 (초기값: 'https://www.google.com')
- [ ] T-8.4: WebView 이벤트 핸들러 구현 (콘솔 로그)
- [ ] T-8.5: 레이아웃 구조 생성 (URLBar placeholder, WebView, NavigationBar placeholder)
- [ ] T-8.6: `BrowserView.module.less` 스타일링

**산출물**:
- BrowserView에 WebView 통합

**완료 기준**:
- BrowserView에서 WebView 컴포넌트 렌더링
- 초기 URL: 'https://www.google.com'
- 이벤트 핸들러 구현:
  - handleLoadStart: 콘솔에 '페이지 로딩 시작: {url}' 출력
  - handleLoadEnd: 콘솔에 '페이지 로딩 완료: {url, title, duration}' 출력
  - handleLoadError: 콘솔에 '페이지 로딩 실패: {errorCode, errorMessage, url}' 출력
  - handleNavigationChange: 콘솔에 'URL 변경: {url}' 출력
- 레이아웃:
  - 상단 URLBar placeholder (80px 높이, 현재 URL 텍스트 표시)
  - 중앙 WebView (flex: 1, 남은 공간 전체 차지)
  - 하단 NavigationBar placeholder (100px 높이, '[ 뒤로 | 앞으로 | 새로고침 | 홈 ]' 텍스트)

**참조**:
- 설계서 § 9. BrowserView 통합 설계
- 설계서 § 10. 구현 순서 Phase 8

---

### Phase 9: 로컬 테스트
**담당**: frontend-dev
**예상 소요**: 1시간

**태스크**:
- [ ] T-9.1: `npm run serve` 실행
- [ ] T-9.2: 브라우저에서 http://localhost:8080 접속
- [ ] T-9.3: Google 홈페이지 렌더링 확인
- [ ] T-9.4: 콘솔에서 로딩 이벤트 로그 확인
- [ ] T-9.5: 네트워크 끊고 에러 처리 확인 (Chrome DevTools Network 탭 → Offline)
- [ ] T-9.6: 존재하지 않는 URL로 에러 확인 (url을 'https://invalid.test' 등으로 변경)
- [ ] T-9.7: 메모리 사용량 로그 확인
- [ ] T-9.8: Tab 키로 WebView 포커스 이동 확인

**완료 기준**:
- Google 홈페이지가 WebView에 정상 렌더링됨
- 콘솔 로그:
  - '[BrowserView] 페이지 로딩 시작: https://www.google.com'
  - '[BrowserView] 페이지 로딩 완료: { url, title, duration }'
  - '[Memory] WebView Mount: { used, total }'
- 네트워크 끊은 상태에서 새 URL 로드 시:
  - ErrorOverlay 표시
  - 콘솔에 '[BrowserView] 페이지 로딩 실패: { errorCode, errorMessage, url }' 출력
- "다시 시도" 버튼 클릭 시 재로딩 시도
- 메모리 로그 정상 출력

**참조**:
- 설계서 § 10. 구현 순서 Phase 9
- 요구사항 분석서 § 8. 완료 기준 AC-2~AC-7

---

### Phase 10: 주요 사이트 렌더링 테스트 (로컬)
**담당**: frontend-dev
**예상 소요**: 1시간

**태스크**:
- [ ] T-10.1: YouTube (https://www.youtube.com) 렌더링 테스트
- [ ] T-10.2: Netflix (https://www.netflix.com) 렌더링 테스트
- [ ] T-10.3: Naver (https://www.naver.com) 렌더링 테스트
- [ ] T-10.4: Wikipedia (https://www.wikipedia.org) 렌더링 테스트
- [ ] T-10.5: 렌더링 성공률 계산 (5개 중 몇 개 성공)
- [ ] T-10.6: 각 사이트 로딩 시간 측정
- [ ] T-10.7: 실패한 사이트는 에러 정보 기록 (이슈 생성)

**완료 기준**:
- 5개 사이트 중 최소 4개 정상 렌더링 (80% 이상, PRD 목표 95%)
- 로딩 시간:
  - Google: 2초 이내
  - 기타 사이트: 5초 이내
- 실패한 사이트가 있을 경우:
  - 에러 정보 기록 (errorCode, errorMessage)
  - 브라우저 콘솔 오류 로그 확인
  - 해결 방법 검토 (sandbox 속성 조정, allow 속성 추가 등)

**참조**:
- 요구사항 분석서 § 3. 비기능 요구사항 (호환성)
- 요구사항 분석서 § 8. 완료 기준 AC-9

---

### Phase 11: 실제 디바이스 테스트
**담당**: frontend-dev
**예상 소요**: 2시간

**태스크**:
- [ ] T-11.1: `npm run pack-p` 빌드
- [ ] T-11.2: `ares-package dist/` IPK 생성
- [ ] T-11.3: 프로젝터(LG HU175QW)에 설치 (`ares-install`)
- [ ] T-11.4: 앱 실행 (`ares-launch`)
- [ ] T-11.5: 리모컨으로 WebView 포커스 이동 확인
- [ ] T-11.6: 리모컨 십자키로 웹 페이지 내 링크 이동 확인
- [ ] T-11.7: 리모컨 선택 버튼(OK)으로 링크 클릭 확인
- [ ] T-11.8: YouTube, Naver 렌더링 확인
- [ ] T-11.9: 에러 처리 테스트 (Wi-Fi 끄고 재실행)
- [ ] T-11.10: 메모리 사용량 확인 (webOS DevTools 또는 로그)

**완료 기준**:
- 프로젝터에서 앱 정상 실행
- Google 홈페이지 정상 렌더링
- 리모컨 조작:
  - 십자키로 WebView 포커스 진입
  - 십자키로 웹 페이지 내 링크 간 이동
  - 선택 버튼으로 링크 클릭 성공
- YouTube, Naver 등 주요 사이트 렌더링 확인 (최소 3개 이상)
- Wi-Fi 끊고 재실행 시:
  - ErrorOverlay 표시
  - 앱 크래시 없음
- 1시간 연속 사용 후 메모리 누수 없음 (200MB 이하 유지)

**참조**:
- 요구사항 분석서 § 8. 완료 기준 AC-12
- 설계서 § 10. 구현 순서 Phase 10

---

### Phase 12: 테스트 및 리뷰
**담당**: test-runner, code-reviewer
**예상 소요**: 2시간

**태스크**:
- [ ] T-12.1: 단위 테스트 작성 (WebView 컴포넌트)
  - Props 검증
  - 로딩 이벤트 콜백 호출 확인
  - 에러 처리 로직 확인
  - 상태 전이 테스트
- [ ] T-12.2: 통합 테스트 작성 (BrowserView)
  - WebView 렌더링 확인
  - 이벤트 핸들러 호출 확인
- [ ] T-12.3: 코드 리뷰
  - Props 인터페이스 설계 적절성
  - 에러 핸들링 완전성
  - 메모리 관리 로직 검증
  - Spotlight 통합 확인
- [ ] T-12.4: 문서 리뷰
  - 코드 주석 완전성 (한국어)
  - PropTypes 문서화
  - 사용 예시 (BrowserView 코드)

**완료 기준**:
- 단위 테스트 통과율: 90% 이상
- 통합 테스트 통과
- 코드 리뷰 승인 (Critical 이슈 0건)
- 문서 리뷰 승인 (주석, PropTypes 문서화 완료)

**참조**:
- 요구사항 분석서 § 8. 완료 기준 AC-13

---

### Phase 13: 기술 문서 작성
**담당**: frontend-dev
**예상 소요**: 1시간

**태스크**:
- [ ] T-13.1: `docs/components/WebView.md` 작성
  - 컴포넌트 개요
  - Props 인터페이스 (표 형식)
  - 사용 예시 (BrowserView 코드 발췌)
  - 이벤트 흐름 (로딩, 에러)
  - 주의사항 (CORS, Spotlight 통합)
- [ ] T-13.2: API 선택 근거 문서화 (설계서에 포함됨, 확인만)
- [ ] T-13.3: 컴포넌트 코드 주석 추가 (한국어)
  - 각 함수 설명
  - 복잡한 로직 설명 (타임아웃, 네비게이션 폴링)

**산출물**:
```
docs/components/WebView.md
```

**완료 기준**:
- WebView.md 작성 완료
  - Props 인터페이스 표
  - 사용 예시 코드
  - 이벤트 흐름 설명
  - CORS, Spotlight 통합 주의사항
- 코드 주석 추가 (주요 함수, 복잡한 로직)
- 설계서의 API 선택 근거 확인 (§ 3. 아키텍처 결정)

**참조**:
- CLAUDE.md § 문서화 규칙 (사후 문서)

---

### Phase 14: 운영 문서 작성 및 커밋
**담당**: doc-writer
**예상 소요**: 30분

**태스크**:
- [ ] T-14.1: `docs/dev-log.md` 업데이트
  - F-02 구현 내용 요약
  - 주요 결정 사항 (iframe 선택 이유)
  - 테스트 결과 (주요 사이트 렌더링 성공률)
- [ ] T-14.2: `CHANGELOG.md` 업데이트
  - [Added] WebView 컴포넌트 (iframe 기반)
  - [Added] 로딩/에러 처리
  - [Added] Enact Spotlight 통합
- [ ] T-14.3: Git 커밋
  - 커밋 메시지: `feat(F-02): WebView 컴포넌트 구현 및 BrowserView 통합`
  - 커밋 분리:
    1. WebView 컴포넌트 구현
    2. BrowserView 통합
    3. 기술 문서 (docs/components/WebView.md)
    4. 운영 문서 (dev-log.md, CHANGELOG.md)

**완료 기준**:
- dev-log.md에 F-02 항목 추가
- CHANGELOG.md에 변경사항 기록
- Git 커밋 4개 생성 (설계/구현/기술문서/운영문서 분리)
- 커밋 메시지 Conventional Commits 형식 준수

**참조**:
- CLAUDE.md § 문서화 규칙 (운영 문서)
- CLAUDE.md § Git 규칙

---

## 4. 태스크 의존성

```
Phase 1 (기본 구조)
    │
    ▼
Phase 2 (iframe 렌더링)
    │
    ├──▶ Phase 3 (로딩 이벤트)
    │       │
    │       ▼
    │   Phase 4 (에러 처리)
    │       │
    │       ▼
    │   Phase 5 (Spotlight)
    │       │
    │       ├──▶ Phase 6 (네비게이션, 선택) ──┐
    │       │                                  │
    │       └──▶ Phase 7 (메모리 관리) ───────┤
    │                                           │
    └──────────────────────────────────────────┤
                                                │
                                                ▼
                                        Phase 8 (BrowserView 통합)
                                                │
                                                ▼
                                        Phase 9 (로컬 테스트)
                                                │
                                                ▼
                                        Phase 10 (사이트 테스트)
                                                │
                                                ▼
                                        Phase 11 (디바이스 테스트)
                                                │
                                                ▼
                                        Phase 12 (테스트 & 리뷰)
                                                │
                                                ├──▶ Phase 13 (기술 문서)
                                                │
                                                ▼
                                        Phase 14 (운영 문서 & 커밋)
```

### 의존성 설명
- **Phase 1 → Phase 2**: 기본 구조 없이는 구현 불가
- **Phase 2 → Phase 3**: iframe이 있어야 로딩 이벤트 처리 가능
- **Phase 3 → Phase 4**: 로딩 로직 이해 필요
- **Phase 4 → Phase 5**: 에러 처리 완료 후 Spotlight 통합
- **Phase 5 → Phase 6/7**: 병렬 가능 (선택)
- **Phase 1~7 → Phase 8**: WebView 컴포넌트 완성 후 통합
- **Phase 8 → Phase 9**: BrowserView 통합 후 로컬 테스트
- **Phase 9 → Phase 10**: 로컬 테스트 통과 후 사이트 테스트
- **Phase 10 → Phase 11**: 로컬 검증 후 디바이스 테스트
- **Phase 11 → Phase 12**: 디바이스 검증 후 자동화 테스트 & 리뷰
- **Phase 12 → Phase 13/14**: 병렬 가능 (기술문서/운영문서)

## 5. 병렬 실행 판단

### Agent Team 사용 여부
**권장: No (순차 실행)**

### 이유
1. **단일 컴포넌트 구현**: WebView 컴포넌트 1개만 구현하므로 작업 분리 불가능
2. **강한 의존성**: 대부분의 Phase가 순차적 의존성을 가짐 (Phase 1~8)
3. **Frontend 단일 작업**: Backend 작업이 없으므로 frontend-dev 서브에이전트가 순차 진행
4. **테스트 단계 병렬 불필요**: Phase 9~11은 순차 테스트가 효율적 (로컬 → 사이트 → 디바이스)
5. **병렬 가능 단계 적음**: Phase 6/7, Phase 13/14만 병렬 가능하나, 소요 시간이 짧아 병렬 효과 미미

### 예외: Phase 6과 Phase 7 병렬 가능
Phase 6(네비게이션 감지)는 선택적이며, Phase 7(메모리 관리)와 독립적이므로 병렬 실행 가능합니다. 그러나 두 작업 모두 1시간 이내로 짧아 순차 실행 권장합니다.

### 결론
**순차 실행**으로 진행하며, `/fullstack-feature` 명령어로 product-manager → architect → frontend-dev → test-runner → code-reviewer → doc-writer 순차 진행합니다.

## 6. 구현 순서 (권장)

1. **Phase 1~7**: WebView 컴포넌트 구현 (6시간)
   - 기본 구조 → iframe 렌더링 → 로딩 이벤트 → 에러 처리 → Spotlight → 네비게이션(선택) → 메모리 관리
2. **Phase 8**: BrowserView 통합 (1시간)
3. **Phase 9~10**: 로컬 테스트 (2시간)
   - 기본 동작 확인 → 주요 사이트 렌더링 테스트
4. **Phase 11**: 실제 디바이스 테스트 (2시간)
   - 프로젝터 배포 → 리모컨 조작 확인 → 메모리 확인
5. **Phase 12**: 테스트 및 리뷰 (2시간)
   - 단위/통합 테스트 → 코드 리뷰 → 문서 리뷰
6. **Phase 13~14**: 문서화 및 커밋 (1.5시간)
   - 기술 문서 작성 → 운영 문서 업데이트 → Git 커밋

**총 예상 소요 시간**: 14.5시간 (약 2일)

## 7. 예상 작업 시간

| Phase | 작업 내용 | 소요 시간 | 누적 시간 |
|-------|----------|----------|----------|
| Phase 1 | WebView 기본 구조 | 0.5h | 0.5h |
| Phase 2 | iframe 렌더링 | 1h | 1.5h |
| Phase 3 | 로딩 이벤트 | 1h | 2.5h |
| Phase 4 | 에러 처리 | 1.5h | 4h |
| Phase 5 | Spotlight 통합 | 1h | 5h |
| Phase 6 | 네비게이션 (선택) | 1h | 6h |
| Phase 7 | 메모리 관리 | 0.5h | 6.5h |
| Phase 8 | BrowserView 통합 | 1h | 7.5h |
| Phase 9 | 로컬 테스트 | 1h | 8.5h |
| Phase 10 | 사이트 테스트 | 1h | 9.5h |
| Phase 11 | 디바이스 테스트 | 2h | 11.5h |
| Phase 12 | 테스트 & 리뷰 | 2h | 13.5h |
| Phase 13 | 기술 문서 | 1h | 14.5h |
| Phase 14 | 운영 문서 & 커밋 | 0.5h | 15h |
| **합계** | | **15h** | **(약 2일)** |

**주의**: Phase 6(네비게이션 감지)는 선택적이므로, 생략 시 14시간으로 단축됩니다.

## 8. 리스크 및 대응책

| 리스크 | 확률 | 영향도 | 대응 방안 |
|--------|------|--------|-----------|
| **iframe의 CORS 제약으로 페이지 제목, 네비게이션 감지 불가** | 고 | 중 | - 페이지 제목은 URL을 제목으로 사용<br>- 네비게이션 감지는 F-04에서 history API로 보완<br>- Same-Origin 사이트는 정상 작동 |
| **주요 사이트 렌더링 실패 (YouTube, Netflix)** | 중 | 고 | - YouTube: 기본 페이지는 렌더링 가능, 로그인 기능은 제약 예상<br>- Netflix: DRM 제약으로 재생 불가 가능성 있음 (사용자 안내)<br>- sandbox/allow 속성 조정으로 해결 시도 |
| **리모컨 포커스와 웹 페이지 포커스 충돌** | 중 | 중 | - WebView 컨테이너만 Spotlightable로 설정<br>- iframe 내부는 웹 표준 포커스 사용<br>- 뒤로 버튼으로 포커스 이탈 (F-04) |
| **메모리 누수 (iframe 리소스 미해제)** | 중 | 고 | - unmount 시 iframe.src = 'about:blank'<br>- useEffect cleanup으로 타이머/리스너 정리<br>- 개발 모드 메모리 로깅으로 모니터링 |
| **webOS 디바이스에서 iframe 동작 제약** | 중 | 고 | - 조기 실제 디바이스 테스트 (Phase 11)<br>- iframe이 webOS에서 지원되지 않으면 @enact/webos/WebView로 전환 (설계 변경) |
| **네트워크 타임아웃 처리 미흡** | 낮 | 중 | - 30초 타임아웃 설정 (PRD 기준 5초에 여유 두어 설정)<br>- 타임아웃 시 에러 메시지 명확히 표시<br>- "다시 시도" 옵션 제공 |
| **실제 디바이스 접근 불가** | 낮 | 고 | - webOS 시뮬레이터로 기본 동작 확인<br>- Phase 11은 디바이스 접근 가능 시 수행<br>- 로컬 테스트(Phase 9~10)로 대부분 검증 가능 |
| **Phase 6(네비게이션 감지) 구현 복잡도 높음** | 중 | 낮 | - Phase 6은 선택적이므로 생략 가능<br>- F-04(페이지 탐색)에서 보완 예정<br>- 구현 시 CORS 에러 무시 처리로 간소화 |

### 주요 리스크 완화 전략
1. **iframe CORS 제약**: 설계 단계에서 이미 인지하고 대응 방안 마련 (설계서 § 11. 기술적 주의사항)
2. **디바이스 호환성**: 조기 디바이스 테스트 (Phase 11)로 문제 조기 발견
3. **메모리 관리**: 개발 모드 메모리 로깅으로 지속 모니터링

## 9. 완료 체크리스트

### 코드 구현
- [ ] WebView 컴포넌트 구현 완료 (Props, PropTypes, 상태 관리)
- [ ] iframe 렌더링 (src, sandbox, allow 속성)
- [ ] 로딩 이벤트 처리 (onLoadStart, onLoadEnd, duration 계산)
- [ ] 에러 처리 (onError, 타임아웃, ErrorOverlay)
- [ ] Enact Spotlight 통합 (Spottable, onSpotlightFocus/Blur)
- [ ] 네비게이션 감지 (선택, setInterval 폴링)
- [ ] 메모리 관리 (useEffect cleanup, 타이머/리스너 정리)
- [ ] BrowserView 통합 (WebView import, 이벤트 핸들러)

### 테스트
- [ ] 로컬 테스트 통과 (Google 렌더링, 로딩 이벤트 로그)
- [ ] 에러 처리 테스트 (네트워크 끊기, 잘못된 URL)
- [ ] 주요 사이트 렌더링 (YouTube, Netflix, Naver, Wikipedia) - 최소 4개 성공
- [ ] 로딩 시간 기준 충족 (Google 2초 이내, 기타 5초 이내)
- [ ] 실제 디바이스 테스트 (프로젝터, 리모컨 조작)
- [ ] 단위 테스트 작성 및 통과 (90% 이상)

### 문서
- [ ] WebView 컴포넌트 주석 (한국어, 주요 함수 설명)
- [ ] PropTypes 문서화 (JSDoc 또는 주석)
- [ ] `docs/components/WebView.md` 작성 (Props, 사용 예시, 주의사항)
- [ ] 설계서 API 선택 근거 확인 (§ 3. 아키텍처 결정)
- [ ] `docs/dev-log.md` 업데이트
- [ ] `CHANGELOG.md` 업데이트

### 리뷰
- [ ] 코드 리뷰 통과 (code-reviewer, Critical 이슈 0건)
- [ ] 문서 리뷰 통과 (code-reviewer, 주석/기술문서 확인)
- [ ] 테스트 리뷰 통과 (test-runner, 단위/통합 테스트)

### Git
- [ ] Git 커밋 4개 생성 (설계/구현/기술문서/운영문서 분리)
- [ ] 커밋 메시지 Conventional Commits 형식 준수
- [ ] 브랜치: `feature/webview-integration` (main에서 분기)

### 기능 확인
- [ ] Google 홈페이지 자동 로드 (https://www.google.com)
- [ ] 리모컨 십자키로 WebView 포커스 진입
- [ ] 리모컨 십자키로 웹 페이지 내 링크 이동
- [ ] 리모컨 선택 버튼(OK)으로 링크 클릭
- [ ] 에러 발생 시 ErrorOverlay 표시 및 "다시 시도" 버튼 동작
- [ ] 메모리 누수 없음 (1시간 연속 사용 시 200MB 이하 유지)

## 10. 이후 기능 연동 준비

### F-03 (URL 입력 UI)
- **연동 방법**: URLBar에서 입력된 URL을 BrowserView의 `setCurrentUrl()`로 전달 → WebView의 `props.url` 변경
- **준비 사항**: BrowserView에서 currentUrl 상태를 useState로 관리 (이미 Phase 8에서 구현됨)

### F-04 (페이지 탐색 컨트롤)
- **연동 방법**: NavigationBar의 뒤로/앞으로 버튼이 iframe의 history.back(), history.forward() 호출
- **준비 사항**: WebView 컴포넌트에서 useImperativeHandle로 goBack(), goForward() 메서드 노출

### F-05 (로딩 인디케이터)
- **연동 방법**: WebView의 onLoadStart/onLoadEnd 이벤트를 받아 프로그레스바 UI 표시
- **준비 사항**: 이미 Phase 3에서 이벤트 콜백 구현됨

### F-06 (탭 관리)
- **연동 방법**: 탭별 WebView 인스턴스 배열 관리, 활성 탭만 렌더링
- **준비 사항**: WebView 컴포넌트는 독립적으로 동작하므로 수정 불필요, BrowserView에서 탭 상태 관리

### F-08 (히스토리 관리)
- **연동 방법**: WebView의 onNavigationChange 이벤트를 받아 히스토리 DB에 저장
- **준비 사항**: Phase 6에서 네비게이션 감지 구현 (선택, CORS 제약 있음)

### F-10 (에러 처리)
- **연동 방법**: WebView의 onLoadError 이벤트를 받아 전역 에러 페이지 표시
- **준비 사항**: Phase 4에서 onLoadError 콜백 구현됨, ErrorOverlay는 임시 UI

## 11. 참고 자료

### 프로젝트 문서
- PRD: `/Users/jsong/dev/jsong1230-github/webosbrowser/docs/project/prd.md`
- 기능 백로그: `/Users/jsong/dev/jsong1230-github/webosbrowser/docs/project/features.md`
- F-01 설계서: `/Users/jsong/dev/jsong1230-github/webosbrowser/docs/specs/project-setup/design.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md`

### 기술 참고
- Enact 공식 문서: https://enactjs.com/docs/
- Enact Spotlight: https://enactjs.com/docs/modules/spotlight/
- HTML iframe MDN: https://developer.mozilla.org/en-US/docs/Web/HTML/Element/iframe
- React useEffect: https://react.dev/reference/react/useEffect
- React useImperativeHandle: https://react.dev/reference/react/useImperativeHandle (F-04 준비용)

### webOS 참고
- webOS TV 개발자 가이드: https://webostv.developer.lge.com
- webOS CLI: https://webostv.developer.lge.com/develop/tools/cli-introduction/
- webOS 디버깅: https://webostv.developer.lge.com/develop/tools/debugging/

## 12. 추가 고려사항

### Phase 6 (네비게이션 감지) 구현 여부 판단 기준
- **구현 권장**: F-08(히스토리 관리)이 높은 우선순위이고, URL 변경 감지가 필수인 경우
- **구현 생략**: CORS 제약으로 대부분의 외부 사이트에서 동작하지 않으므로, F-04(페이지 탐색)의 history API로 대체 가능한 경우

현재 PRD와 기능 백로그를 확인한 결과, F-04와 F-08의 우선순위가 높지 않으므로 **Phase 6은 선택적으로 구현**하는 것이 합리적입니다.

### 실제 디바이스 테스트 (Phase 11) 중요성
webOS 프로젝터(LG HU175QW)는 Chromium 53 기반으로 최신 브라우저와 다른 동작을 보일 수 있습니다. 특히:
- iframe 샌드박스 정책 차이
- 리모컨 키 이벤트 변환 방식
- 메모리 제약 (1~2GB RAM)

**조기 디바이스 테스트**로 문제를 빠르게 발견하고 대응하는 것이 중요합니다.

### 성능 최적화 여지
현재 설계는 기본 동작 구현에 초점을 두고 있으며, 향후 최적화 가능한 부분:
1. **iframe 로딩 최적화**: 초기 로딩 시 placeholder 이미지 표시
2. **진행률 개선**: F-05에서 가상 진행률 애니메이션 추가
3. **메모리 최적화**: F-06에서 탭 전환 시 비활성 탭의 iframe 일시 정지 (suspend)

---

## 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-12 | 초기 구현 계획서 작성 | product-manager |
