# 페이지 탐색 컨트롤 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/navigation-controls/requirements.md
- 기술 설계서: docs/specs/navigation-controls/design.md
- 웹뷰 설계서: docs/specs/webview-integration/design.md
- PRD: docs/project/prd.md
- CLAUDE.md: /Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md

## 2. 구현 Phase

### Phase 1: NavigationBar 컴포넌트 기본 구조 생성
**담당**: frontend-dev
**예상 산출물**:
- src/components/NavigationBar/NavigationBar.js
- src/components/NavigationBar/NavigationBar.module.less
- src/components/NavigationBar/index.js

**작업 내용**:
- [ ] 디렉토리 생성: src/components/NavigationBar/
- [ ] NavigationBar.js 생성 (함수형 컴포넌트, Props 인터페이스 정의)
- [ ] PropTypes 정의 (webviewRef, canGoBack, canGoForward, homeUrl, onNavigate, className)
- [ ] defaultProps 정의 (설계서 참조)
- [ ] NavigationBar.module.less 생성 (기본 스타일 구조)
- [ ] index.js 생성 (export default NavigationBar)

**완료 기준**:
- NavigationBar 컴포넌트가 BrowserView에서 import 가능
- PropTypes 경고 없이 렌더링
- 빈 컨테이너 div 렌더링 확인 (버튼 없음, 스타일만)

**예상 시간**: 30분

---

### Phase 2: 버튼 UI 구현
**담당**: frontend-dev
**의존성**: Phase 1 완료 필요
**예상 산출물**:
- NavigationBar.js (버튼 4개 추가)
- NavigationBar.module.less (레이아웃 및 버튼 스타일)

**작업 내용**:
- [ ] Enact Moonstone Button import
- [ ] 4개 버튼 배치 (뒤로, 앞으로, 새로고침, 홈)
- [ ] 버튼 아이콘 설정 (icon props: arrowlargeleft, arrowlargeright, refresh, home)
- [ ] 버튼 라벨 추가 (요구사항 FR-1~FR-4 참조)
- [ ] Flexbox 수평 배치 (justify-content: center, gap: 20px)
- [ ] 버튼 크기 설정 (min-width: 100px, min-height: 80px)
- [ ] 비활성화 상태 스타일링 (opacity: 0.5, disabled 상태)
- [ ] 포커스 스타일링 (border: 2px solid white)
- [ ] 클릭 애니메이션 (transform: scale(0.95))

**완료 기준**:
- 4개 버튼이 수평으로 중앙 정렬되어 렌더링
- 버튼 간격 20px 유지
- 버튼 크기 100px x 80px 이상
- disabled Props에 따라 버튼이 흐릿하게 표시
- 로컬 브라우저에서 CSS 스타일 확인

**예상 시간**: 1시간

---

### Phase 3: 버튼 이벤트 핸들러 구현
**담당**: frontend-dev
**의존성**: Phase 2 완료 필요
**예상 산출물**:
- NavigationBar.js (이벤트 핸들러 추가)

**작업 내용**:
- [ ] handleBack 함수 작성 (useCallback 사용)
  - canGoBack 확인 후 webviewRef.current.contentWindow.history.back() 호출
  - onNavigate({ action: 'back' }) 콜백 호출
  - try-catch 에러 처리 (조용히 실패)
  - logger.info로 로그 기록
- [ ] handleForward 함수 작성 (handleBack과 유사)
- [ ] handleReload 함수 작성
  - webviewRef.current.contentWindow.location.reload() 호출
  - onNavigate({ action: 'reload' }) 콜백 호출
- [ ] handleHome 함수 작성
  - onNavigate({ action: 'home', url: homeUrl }) 콜백 호출만 (iframe src 변경은 BrowserView가 담당)
- [ ] isNavigating 상태 추가 (중복 클릭 방지)
- [ ] 각 핸들러에서 0.5초 후 isNavigating 플래그 해제
- [ ] 버튼 onClick Props에 핸들러 연결
- [ ] 버튼 disabled Props 연결 (canGoBack, canGoForward, isNavigating)

**완료 기준**:
- 버튼 클릭 시 핸들러 함수 호출 확인 (console.log)
- isNavigating 플래그로 0.5초 동안 버튼 비활성화 확인
- 에러 발생 시에도 앱이 중단되지 않음 (조용히 실패)

**예상 시간**: 1.5시간

---

### Phase 4: WebView ref 노출 (WebView 수정)
**담당**: frontend-dev
**의존성**: Phase 3 완료 필요
**예상 산출물**:
- src/components/WebView/WebView.js (forwardRef 적용)

**작업 내용**:
- [ ] WebView 컴포넌트를 forwardRef로 변환
- [ ] useImperativeHandle로 iframe ref 노출
- [ ] iframe 편의 메서드 추가 (선택):
  - goBack(): iframe.contentWindow.history.back() 호출 후 boolean 반환
  - goForward(): iframe.contentWindow.history.forward() 호출 후 boolean 반환
  - reload(): iframe.contentWindow.location.reload() 호출 후 boolean 반환
- [ ] 각 메서드에 try-catch 에러 처리 추가
- [ ] logger.error로 에러 로그 기록

**완료 기준**:
- WebView ref를 통해 iframe 요소 접근 가능
- goBack, goForward, reload 메서드 호출 시 정상 동작
- CORS 에러 발생 시 false 반환 (앱 중단 없음)

**예상 시간**: 1시간

---

### Phase 5: BrowserView 통합 (히스토리 스택 관리)
**담당**: frontend-dev
**의존성**: Phase 4 완료 필요
**예상 산출물**:
- src/views/BrowserView.js (NavigationBar 통합)

**작업 내용**:
- [ ] webviewRef 생성 (useRef)
- [ ] WebView에 ref 전달
- [ ] canGoBack, canGoForward 상태 추가 (useState)
- [ ] historyStack 상태 추가 (useState, 초기값: [])
- [ ] historyIndex 상태 추가 (useState, 초기값: -1)
- [ ] handleNavigate 함수 구현:
  - action: 'back' → historyIndex--, currentUrl 업데이트
  - action: 'forward' → historyIndex++, currentUrl 업데이트
  - action: 'reload' → URL 변경 없음
  - action: 'home' → historyStack 초기화, currentUrl = homeUrl, historyIndex = 0
- [ ] handleNavigationChange 함수 구현:
  - URL 변경 감지 시 historyStack 업데이트
  - 현재 위치 이후 스택 제거 (앞으로 기록 삭제)
  - canGoBack, canGoForward 계산 (historyIndex 기반)
- [ ] NavigationBar import 및 배치 (WebView 아래)
- [ ] NavigationBar Props 전달 (webviewRef, canGoBack, canGoForward, homeUrl, onNavigate)
- [ ] homeUrl 하드코딩 ('https://www.google.com')

**완료 기준**:
- NavigationBar가 BrowserView에 렌더링
- 버튼 클릭 시 handleNavigate 호출 확인
- historyStack, historyIndex 상태가 정상 업데이트
- canGoBack, canGoForward가 정확히 계산됨 (historyIndex 기반)

**예상 시간**: 2시간

---

### Phase 6: Spotlight 통합 (리모컨 포커스 관리)
**담당**: frontend-dev
**의존성**: Phase 5 완료 필요
**예상 산출물**:
- NavigationBar.js (Spotlight 설정 추가)
- BrowserView.js (Spotlight 포커스 흐름 정의)

**작업 내용**:
- [ ] NavigationBar.js에 data-spotlight-container 속성 추가
- [ ] 각 버튼에 spotlightId 설정 (nav-back, nav-forward, nav-reload, nav-home)
- [ ] BrowserView.js에서 Spotlight.set 설정:
  - 'webview-main': leaveFor.down → 'navigation-bar'
  - 'navigation-bar': leaveFor.up → 'webview-main'
- [ ] 초기 포커스를 WebView로 설정 (Spotlight.focus('webview-main'))
- [ ] NavigationBar의 defaultElement 설정 (nav-back)

**완료 기준**:
- 로컬 브라우저에서 Tab 키로 버튼 포커스 이동 확인
- 버튼 포커스 시 흰색 테두리 표시 확인
- disabled 버튼은 포커스 건너뜀 확인

**예상 시간**: 1시간

---

### Phase 7: 리모컨 Back 키 처리
**담당**: frontend-dev
**의존성**: Phase 6 완료 필요
**예상 산출물**:
- src/hooks/useRemoteControl.js
- BrowserView.js (useRemoteControl 사용)

**작업 내용**:
- [ ] useRemoteControl Hook 생성 (src/hooks/useRemoteControl.js)
- [ ] Props 인터페이스 정의 (isWebViewFocused, onBackInWebView, onBackOutsideWebView)
- [ ] window keydown 이벤트 리스너 등록
- [ ] Back 키 감지 (Backspace: 8, webOS Back: 461)
- [ ] isWebViewFocused에 따라 분기:
  - true: onBackInWebView() 호출, event.preventDefault()
  - false: onBackOutsideWebView() 호출, 기본 동작 허용
- [ ] BrowserView에서 isWebViewFocused 상태 관리:
  - spotlightfocus 이벤트 리스너 등록
  - event.detail.spotlightId === 'webview-main' 확인
- [ ] useRemoteControl 사용:
  - onBackInWebView: canGoBack 확인 후 history.back() 또는 NavigationBar로 포커스 이동
  - onBackOutsideWebView: Spotlight 포커스 이탈 (기본 동작)

**완료 기준**:
- WebView 포커스 시 Backspace 키 → 브라우저 뒤로 동작
- NavigationBar 포커스 시 Backspace 키 → 포커스 이탈 (기본 동작)
- 첫 페이지에서 Backspace 키 → NavigationBar로 포커스 이동

**예상 시간**: 1.5시간

---

### Phase 8: 히스토리 상태 동기화 (WebView 수정)
**담당**: frontend-dev
**의존성**: Phase 7 완료 필요
**예상 산출물**:
- src/components/WebView/WebView.js (onNavigationChange 개선)

**작업 내용**:
- [ ] WebView에 URL 변경 감지 로직 추가 (setInterval 사용, 500ms 주기)
- [ ] iframe.contentWindow.location.href 읽기 (CORS 에러 시 무시)
- [ ] URL 변경 시 onNavigationChange 콜백 호출
- [ ] 콜백에 { url, canGoBack, canGoForward } 전달:
  - canGoBack: history.length > 1 (추정값, CORS 제약)
  - canGoForward: false (iframe에서 직접 감지 불가)
- [ ] CORS 에러 시 try-catch로 무시 (조용히 실패)
- [ ] navigationIntervalRef cleanup (컴포넌트 unmount 시)

**완료 기준**:
- WebView에서 URL 변경 감지 시 onNavigationChange 호출 확인
- BrowserView의 canGoBack 상태가 자동 업데이트
- CORS 환경(예: YouTube)에서도 에러 없이 동작

**주의사항**:
- canGoBack, canGoForward는 BrowserView의 historyStack 기반으로 계산하므로, WebView에서 전달받는 값은 참고용
- CORS 제약으로 Same-Origin이 아닌 페이지에서는 히스토리 상태 감지 불가 (버튼 항상 활성화)

**예상 시간**: 1.5시간

---

### Phase 9: 로컬 테스트
**담당**: frontend-dev
**의존성**: Phase 8 완료 필요
**예상 산출물**: 없음 (테스트 검증)

**작업 내용**:
- [ ] npm run serve 실행 (포트 8080)
- [ ] Google 홈페이지 로드 → 뒤로 버튼 비활성화 확인
- [ ] "YouTube" 검색 → 검색 결과 페이지 로드 → 뒤로 버튼 활성화 확인
- [ ] 뒤로 버튼 클릭 → Google 홈페이지 복귀 → 앞으로 버튼 활성화 확인
- [ ] 앞으로 버튼 클릭 → 검색 결과 페이지 복귀
- [ ] 새로고침 버튼 클릭 → 페이지 재로드 확인
- [ ] 홈 버튼 클릭 → Google 홈페이지 복귀 → 앞으로 버튼 비활성화 확인
- [ ] Tab 키로 버튼 포커스 이동 확인 (로컬 브라우저에서 Tab → Spotlight 방향키 대체)
- [ ] 버튼 클릭 시 눌림 효과 확인 (scale 애니메이션)
- [ ] 연속 클릭 시 0.5초 동안 버튼 비활성화 확인 (중복 클릭 방지)

**완료 기준**:
- 요구사항 분석서의 AC-1 (버튼 기본 동작) 모두 통과
- 요구사항 분석서의 AC-2 (버튼 상태 관리) 모두 통과
- 요구사항 분석서의 AC-6 (UI 가이드 준수) 모두 통과
- 콘솔 에러 없음

**예상 시간**: 1시간

---

### Phase 10: 실제 디바이스 테스트
**담당**: frontend-dev
**의존성**: Phase 9 완료 필요
**예상 산출물**: 없음 (디바이스 검증)

**작업 내용**:
- [ ] npm run pack-p (프로덕션 빌드)
- [ ] ares-package dist/ (IPK 생성)
- [ ] ares-install --device projector {ipk파일} (프로젝터 설치)
- [ ] ares-launch --device projector com.jsong.webosbrowser (앱 실행)
- [ ] 리모컨 방향키로 버튼 포커스 이동 확인 (좌/우 방향키)
- [ ] 리모컨 선택 버튼으로 버튼 클릭 확인
- [ ] 리모컨 위 방향키로 WebView로 포커스 이동 확인
- [ ] 리모컨 아래 방향키로 NavigationBar로 포커스 이동 확인
- [ ] 리모컨 Back 키 동작 확인:
  - WebView 포커스 시 → 브라우저 뒤로
  - NavigationBar 포커스 시 → 포커스 이탈
  - 첫 페이지에서 → NavigationBar로 포커스 이동
- [ ] 주요 사이트에서 테스트:
  - YouTube: 비디오 재생 → 뒤로 버튼 → 홈으로 복귀
  - Naver: 검색 → 결과 페이지 → 뒤로 버튼
  - Google: 검색 → 결과 페이지 → 새로고침 버튼
- [ ] 3m 거리에서 버튼 아이콘 명확히 인식 가능한지 확인
- [ ] 버튼 클릭 응답 시간 0.3초 이내 확인 (요구사항 NFR-1)

**완료 기준**:
- 요구사항 분석서의 AC-3 (리모컨 네비게이션) 모두 통과
- 요구사항 분석서의 AC-4 (리모컨 Back 키 매핑) 모두 통과
- 요구사항 분석서의 AC-5 (성능) 모두 통과
- 요구사항 분석서의 AC-8 (실제 디바이스 테스트) 모두 통과
- ares-log로 에러 없음 확인

**예상 시간**: 1.5시간

---

### Phase 11: 테스트 작성
**담당**: test-runner
**의존성**: Phase 10 완료 필요
**예상 산출물**:
- src/components/NavigationBar/NavigationBar.test.js

**작업 내용**:
- [ ] NavigationBar 컴포넌트 렌더링 테스트
- [ ] Props 전달 테스트 (canGoBack, canGoForward)
- [ ] 버튼 클릭 시 핸들러 호출 테스트 (onNavigate)
- [ ] 버튼 비활성화 상태 테스트 (disabled Props)
- [ ] isNavigating 플래그 테스트 (중복 클릭 방지)
- [ ] useRemoteControl Hook 테스트:
  - Back 키 감지 테스트
  - isWebViewFocused에 따른 분기 테스트
- [ ] CORS 에러 처리 테스트 (history API 호출 실패 시)

**완료 기준**:
- npm test 실행 시 모든 테스트 통과
- 테스트 커버리지 80% 이상

**예상 시간**: 2시간

---

### Phase 12: 코드 + 문서 리뷰
**담당**: code-reviewer
**의존성**: Phase 11 완료 필요
**예상 산출물**: 없음 (리뷰 검증)

**작업 내용**:
- [ ] 코드 리뷰:
  - PropTypes 정의 누락 확인
  - useCallback 의존성 배열 확인
  - try-catch 에러 처리 확인
  - logger 사용 확인 (console.log 금지)
  - CSS Modules 사용 확인 (인라인 스타일 금지)
  - Enact Spotlight 설정 확인
- [ ] 문서 리뷰:
  - requirements.md 완료 기준 충족 확인
  - design.md 설계 준수 확인
  - 컴포넌트 문서 작성 필요 여부 확인
- [ ] 성능 확인:
  - 버튼 클릭 응답 시간 0.3초 이내
  - 메모리 누수 없음 (cleanup 함수 확인)

**완료 기준**:
- 코드 컨벤션 준수 (CLAUDE.md 규칙)
- 요구사항 분석서의 모든 AC 통과
- 리뷰 승인 (Approve)

**예상 시간**: 1시간

---

## 3. 태스크 의존성

```
Phase 1 (컴포넌트 구조)
   │
   ▼
Phase 2 (버튼 UI)
   │
   ▼
Phase 3 (이벤트 핸들러)
   │
   ▼
Phase 4 (WebView ref 노출)
   │
   ▼
Phase 5 (BrowserView 통합) ──┐
   │                          │
   ▼                          │
Phase 6 (Spotlight)           │
   │                          │
   ▼                          │
Phase 7 (리모컨 Back 키)      │
   │                          │
   ▼                          │
Phase 8 (히스토리 동기화) ◀───┘
   │
   ▼
Phase 9 (로컬 테스트)
   │
   ▼
Phase 10 (디바이스 테스트)
   │
   ▼
Phase 11 (테스트 작성)
   │
   ▼
Phase 12 (코드 + 문서 리뷰)
```

**병렬 실행 가능한 태스크**: 없음 (모두 순차 의존성)

## 4. 병렬 실행 판단

**Agent Team 사용 권장 여부**: No

**이유**:
- NavigationBar는 단일 컴포넌트이며, 버튼 4개가 하나의 UI로 통합됨
- WebView, BrowserView와의 긴밀한 의존성으로 인해 순차 작업 필요
- 히스토리 스택 관리, Spotlight 통합, 리모컨 Back 키 처리는 모두 BrowserView 레벨에서 수정이 필요하여 병렬 작업 불가능
- 예상 총 작업 시간: 약 15시간 (1~2일) → 단일 개발자가 순차 작업하는 것이 효율적
- 병렬 작업 시 브랜치 간 머지 충돌 발생 가능성 높음 (BrowserView.js 동시 수정)

**병렬 작업 가능한 대안**:
- Phase 11 (테스트 작성)은 Phase 10 완료 후 별도 개발자가 병렬로 수행 가능하지만, 기능 구현과 테스트 작성을 동일 개발자가 수행하는 것이 테스트 품질 향상에 유리

## 5. 리스크

| 리스크 | 영향 | 확률 | 대응 방안 |
|--------|------|------|-----------|
| **iframe history API의 CORS 제약** | 다른 도메인 페이지에서 히스토리 상태 감지 불가, 버튼 활성/비활성 상태가 정확하지 않음 | 높음 | BrowserView에서 히스토리 스택을 별도 관리하여 정확도 향상. CORS 환경에서는 버튼을 항상 활성화하고, 클릭 시 조용히 실패 처리 (설계서 § 6 참조) |
| **리모컨 Back 키 코드가 webOS 버전마다 다름** | Back 키가 동작하지 않거나 의도하지 않은 동작 발생 | 중간 | useRemoteControl Hook에서 여러 키 코드를 모두 처리 (Backspace: 8, webOS Back: 461). 실제 디바이스에서 테스트 후 키 코드 추가 (설계서 § 12 참조) |
| **Spotlight 포커스 흐름이 복잡함** | 버튼 간 포커스 이동이 자연스럽지 않거나 포커스가 사라짐 | 중간 | Spotlight.set으로 포커스 흐름 명확히 정의. defaultElement, leaveFor 설정 추가. 실제 디바이스에서 테스트 후 조정 (설계서 § 7 참조) |
| **WebView ref 노출로 인한 결합도 증가** | NavigationBar가 WebView 내부 구현에 의존, 향후 WebView 변경 시 영향 | 낮음 | useImperativeHandle로 ref 인터페이스 명확히 정의. iframe 요소 외에는 노출하지 않음. WebView 변경 시 ref 인터페이스만 유지하면 NavigationBar 수정 불필요 (설계서 § 5 참조) |
| **중복 클릭으로 인한 히스토리 스택 오류** | 사용자가 연속으로 뒤로 버튼 클릭 시 의도하지 않은 페이지로 이동 | 낮음 | isNavigating 플래그로 0.5초 동안 중복 클릭 차단. 버튼 비활성화로 시각적 피드백 제공 (설계서 § 12 참조) |
| **F-02 WebView 구현이 완료되지 않음** | NavigationBar가 WebView ref를 받을 수 없어 동작 불가 | 낮음 | F-02는 이미 완료됨. Phase 4에서 WebView를 forwardRef로 변환하면 즉시 사용 가능 |
| **대화면 환경에서 버튼 크기가 작아 보일 수 있음** | 3m 거리에서 버튼 아이콘 인식 어려움 | 낮음 | 요구사항 NFR-2에 따라 버튼 크기 100px x 80px, 아이콘 32px x 32px로 설정. Phase 10에서 실제 디바이스 테스트 후 조정 |

## 6. 검증 계획

### 단위 테스트 (Phase 11)
- NavigationBar 컴포넌트 렌더링 테스트
- Props 전달 테스트
- 이벤트 핸들러 테스트
- useRemoteControl Hook 테스트
- CORS 에러 처리 테스트

### 통합 테스트 (Phase 9, 10)
- BrowserView와 NavigationBar 통합 테스트
- WebView와 NavigationBar history API 제어 테스트
- Spotlight 포커스 흐름 테스트
- 리모컨 Back 키 처리 테스트

### 사용자 시나리오 테스트 (Phase 10)
- 요구사항 분석서 § 8의 시나리오 1~5 모두 실행
- 주요 사이트(YouTube, Naver, Google)에서 정상 동작 확인

### 성능 테스트 (Phase 10)
- 버튼 클릭 후 0.3초 이내에 페이지 네비게이션 시작 (NFR-1)
- 히스토리 상태 업데이트 0.5초 이내 (NFR-1)
- 연속 클릭 시 응답 지연 없음

### UI/UX 테스트 (Phase 10)
- 버튼 크기 100px x 80px 이상 (NFR-2)
- 버튼 간격 20px 이상 (NFR-2)
- 포커스 테두리 2px, 흰색 (NFR-2)
- 비활성화 버튼 opacity: 0.5 (NFR-3)
- 3m 거리에서 버튼 아이콘 인식 가능 (AC-8)

### 에러 처리 테스트 (Phase 9, 10)
- iframe history API 호출 실패 시 조용히 실패 (AC-7)
- CORS 에러 시 버튼 활성화 상태 유지 (AC-7)
- 히스토리 상태 확인 실패 시 기본 상태 복구 (AC-7)

### 회귀 테스트
- F-02 (웹뷰 통합) 기능이 정상 동작하는지 확인
- BrowserView 수정으로 인한 기존 기능 영향 없음 확인

## 7. 확장성 고려사항

### F-06 (탭 관리)와의 연동 준비
- **현재 설계**: NavigationBar는 webviewRef Props로 받으므로, 활성 탭의 ref만 전달하면 즉시 연동 가능
- **F-06 구현 시**: BrowserView가 탭별 historyStack을 관리하고, 활성 탭의 webviewRef, canGoBack, canGoForward를 NavigationBar에 전달
- **변경 필요 사항**: BrowserView의 히스토리 스택을 탭 ID별로 분리 (Map<tabId, historyStack>)

### F-08 (히스토리 관리)와의 연동 준비
- **현재 설계**: BrowserView가 히스토리 스택을 메모리에서만 관리
- **F-08 구현 시**: handleNavigationChange에서 historyService.addHistory(url) 호출 추가
- **변경 필요 사항**: 앱 재시작 시 LS2 API에서 히스토리 불러와 historyStack 복원

### F-11 (설정 화면)과의 연동 준비
- **현재 설계**: 홈페이지 URL이 하드코딩 (https://www.google.com)
- **F-11 구현 시**: BrowserView에서 settingsService.getHomeUrl()로 설정 읽기
- **변경 필요 사항**: homeUrl을 state로 관리, useEffect로 앱 시작 시 설정 읽기

### F-12 (리모컨 단축키)와의 연동 준비
- **현재 설계**: useRemoteControl Hook이 Back 키만 처리
- **F-12 구현 시**: onChannelUp, onColorRed 등 콜백 추가
- **변경 필요 사항**: Hook Props 인터페이스 확장, 단축키 설정 읽어와 매핑

## 8. 예상 일정

| Phase | 예상 시간 | 누적 시간 |
|-------|----------|----------|
| Phase 1: 컴포넌트 구조 | 30분 | 0.5시간 |
| Phase 2: 버튼 UI | 1시간 | 1.5시간 |
| Phase 3: 이벤트 핸들러 | 1.5시간 | 3시간 |
| Phase 4: WebView ref 노출 | 1시간 | 4시간 |
| Phase 5: BrowserView 통합 | 2시간 | 6시간 |
| Phase 6: Spotlight 통합 | 1시간 | 7시간 |
| Phase 7: 리모컨 Back 키 | 1.5시간 | 8.5시간 |
| Phase 8: 히스토리 동기화 | 1.5시간 | 10시간 |
| Phase 9: 로컬 테스트 | 1시간 | 11시간 |
| Phase 10: 디바이스 테스트 | 1.5시간 | 12.5시간 |
| Phase 11: 테스트 작성 | 2시간 | 14.5시간 |
| Phase 12: 코드 + 문서 리뷰 | 1시간 | 15.5시간 |

**총 예상 시간**: 약 15.5시간 (2일)

## 9. 완료 기준

### 기능 완료 기준
- [ ] 요구사항 분석서의 모든 AC (AC-1~AC-8) 통과
- [ ] 로컬 브라우저에서 모든 버튼 정상 동작
- [ ] LG 프로젝터 HU175QW에서 리모컨으로 모든 기능 정상 동작
- [ ] 주요 사이트(YouTube, Naver, Google)에서 테스트 통과

### 코드 품질 기준
- [ ] npm test 실행 시 모든 테스트 통과
- [ ] 테스트 커버리지 80% 이상
- [ ] ESLint 경고 없음
- [ ] PropTypes 정의 완료
- [ ] logger 사용 (console.log 금지)
- [ ] CSS Modules 사용 (인라인 스타일 금지)

### 문서 완료 기준
- [ ] 컴포넌트 문서 작성 (docs/components/NavigationBar.md)
  - Props 인터페이스
  - 사용 예시
  - 리모컨 키 매핑
  - 주의사항
- [ ] dev-log.md 업데이트
- [ ] CHANGELOG.md 업데이트

### 성능 기준
- [ ] 버튼 클릭 응답 시간 0.3초 이내 (NFR-1)
- [ ] 히스토리 상태 업데이트 0.5초 이내 (NFR-1)
- [ ] 메모리 누수 없음

### 리뷰 승인
- [ ] code-reviewer 리뷰 승인 (Approve)
- [ ] 설계서 준수 확인
- [ ] 코드 컨벤션 준수 확인

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-12 | 최초 작성 | F-04 구현 계획서 작성 |
