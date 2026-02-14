# F-14: HTTPS 보안 표시 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/https-security-indicator/requirements.md
- 기술 설계서: docs/specs/https-security-indicator/design.md

---

## 2. 구현 Phase

### Phase 1: 유틸리티 구현 (기반 작업)
**담당**: frontend-dev
**예상 소요 시간**: 1시간

- [ ] Task 1-1: securityClassifier.js 유틸리티 구현
  - **설명**:
    - classifyUrl(url) 함수: URL 프로토콜 기반 보안 등급 분류 (`secure`, `insecure`, `local`, `unknown`)
    - isLocalHost(hostname) 함수: localhost 여부 판단 (localhost, 127.0.0.1, 사설 IP)
    - extractDomain(url) 함수: 도메인 추출 (경고 무시 목록 관리용)
    - null/undefined/빈 문자열 처리, URL 파싱 실패 예외 처리
  - **산출물**: `src/utils/securityClassifier.js` (신규, 약 80줄)
  - **완료 기준**:
    - 모든 보안 등급 분류 로직 구현 완료
    - 로컬 IP 패턴 매칭 정확성 검증 (192.168.*.*, 10.*.*.*, 172.16-31.*.*)

- [ ] Task 1-2: securityStorage.js 유틸리티 구현
  - **설명**:
    - getIgnoredDomains() 함수: SessionStorage에서 경고 무시 도메인 목록 조회 (Set 반환)
    - addIgnoredDomain(domain) 함수: 도메인 추가
    - removeIgnoredDomain(domain) 함수: 도메인 제거 (선택적)
    - clearIgnoredDomains() 함수: 목록 초기화 (선택적)
    - JSON 파싱 실패, 저장 용량 초과 예외 처리
  - **산출물**: `src/utils/securityStorage.js` (신규, 약 70줄)
  - **완료 기준**:
    - SessionStorage CRUD 로직 정상 동작
    - Set ↔ Array 변환 로직 검증

- [ ] Task 1-3: 단위 테스트 작성
  - **설명**:
    - securityClassifier.js 테스트: 각 보안 등급별 URL 분류 검증
    - securityStorage.js 테스트: 도메인 추가/제거/조회 검증
  - **산출물**:
    - `src/utils/__tests__/securityClassifier.test.js` (신규, 약 80줄)
    - `src/utils/__tests__/securityStorage.test.js` (신규, 약 60줄)
  - **완료 기준**: 모든 테스트 케이스 통과

---

### Phase 2: SecurityIndicator 컴포넌트 구현
**담당**: frontend-dev
**의존성**: Phase 1 완료 필요
**예상 소요 시간**: 1.5시간

- [ ] Task 2-1: SecurityIndicator 컴포넌트 구현
  - **설명**:
    - Props: securityLevel (secure/insecure/local/unknown), url, onClick, className
    - 보안 등급별 아이콘 렌더링: 🔒 (secure), ⚠ (insecure), ℹ️ (local), 없음 (unknown)
    - Spotlightable 컴포넌트로 구현 (리모컨 포커스 가능)
    - Moonstone Tooltip 통합 (포커스 시 보안 상태 설명 표시)
    - onClick 이벤트 핸들러 (FR-5 상세 정보 다이얼로그용, 선택적 구현)
  - **산출물**: `src/components/SecurityIndicator/SecurityIndicator.js` (신규, 약 100줄)
  - **완료 기준**:
    - 4가지 보안 등급별 아이콘 정확히 표시
    - Spotlight 포커스 동작 확인
    - Tooltip 표시 확인

- [ ] Task 2-2: SecurityIndicator 스타일 구현
  - **설명**:
    - 고정 너비 48px (레이아웃 시프트 방지)
    - 보안 등급별 아이콘 색상: Green (#4CAF50), Orange (#FF9800), Blue (#2196F3)
    - 폰트 크기 32px (프로젝터 대화면 가독성)
    - Spotlight 포커스 스타일 (outline)
  - **산출물**: `src/components/SecurityIndicator/SecurityIndicator.module.less` (신규, 약 40줄)
  - **완료 기준**: 대화면에서 아이콘 명확히 식별 가능

- [ ] Task 2-3: 컴포넌트 테스트 작성
  - **설명**:
    - HTTPS 아이콘 표시 테스트
    - HTTP 아이콘 표시 테스트
    - localhost 아이콘 표시 테스트
    - unknown 등급 시 아이콘 미표시 테스트
    - onClick 이벤트 테스트
  - **산출물**: `src/components/SecurityIndicator/__tests__/SecurityIndicator.test.js` (신규, 약 80줄)
  - **완료 기준**: 모든 테스트 케이스 통과

---

### Phase 3: URLBar 확장
**담당**: frontend-dev
**의존성**: Phase 2 완료 필요
**예상 소요 시간**: 1.5시간

- [ ] Task 3-1: URLBar 컴포넌트 로직 확장
  - **설명**:
    - 신규 상태 추가: securityLevel (useState)
    - useEffect 추가: value prop (현재 URL) 변경 시 classifyUrl(value) 호출하여 securityLevel 업데이트
    - SecurityIndicator 컴포넌트 통합 (좌측 아이콘 영역)
    - SecurityIndicator에 securityLevel, url(value) props 전달
    - handleSecurityIconClick 핸들러 구현 (선택적, FR-5용)
  - **파일**: `src/components/URLBar/URLBar.js` (수정, 약 230줄 → 260줄)
  - **변경 위치**: line 159-171 (urlBar div 구조)
  - **완료 기준**:
    - URL 변경 시 보안 아이콘 실시간 업데이트 (500ms 이내)
    - ESLint 에러 없음

- [ ] Task 3-2: URLBar 레이아웃 조정
  - **설명**:
    - urlBar div를 flexbox로 변경 (display: flex, align-items: center, gap: 8px)
    - SecurityIndicator 영역: flex-shrink: 0 (고정 너비 유지)
    - Input 영역: flex: 1 (남은 공간 차지)
    - 기존 자동완성 드롭다운, 가상 키보드 레이아웃 영향 확인
  - **파일**: `src/components/URLBar/URLBar.module.less` (수정)
  - **완료 기준**:
    - 보안 아이콘 추가 시 레이아웃 시프트 없음
    - Input 너비 유지

- [ ] Task 3-3: Spotlight 포커스 순서 조정
  - **설명**:
    - URLBar 포커스 순서: SecurityIndicator → Input → VirtualKeyboard
    - Spotlight.set()으로 leaveFor 설정 업데이트 (필요 시)
  - **완료 기준**: 리모컨 방향키로 SecurityIndicator 접근 가능

---

### Phase 4: SecurityWarningDialog 컴포넌트 구현
**담당**: frontend-dev
**의존성**: Phase 1 완료 필요 (Phase 3과 병렬 가능)
**예상 소요 시간**: 2시간

- [ ] Task 4-1: SecurityWarningDialog 컴포넌트 구현
  - **설명**:
    - Props: visible, url, onContinue, onGoBack, onDontShowAgain
    - Moonstone Dialog 컴포넌트 사용
    - UI 구조: 경고 아이콘(⚠), 메시지, URL 표시, 체크박스("오늘 이 사이트에 대해 다시 표시하지 않기")
    - 버튼: "계속 진행" (기본 포커스), "돌아가기"
    - Spotlight 포커스 순서: 체크박스 → 계속 진행 → 돌아가기
    - 체크박스 상태 관리 (local state)
  - **산출물**: `src/components/SecurityWarningDialog/SecurityWarningDialog.js` (신규, 약 120줄)
  - **완료 기준**:
    - 다이얼로그 표시 정상 동작
    - 버튼 클릭 이벤트 정상 동작
    - 체크박스 상태 onDontShowAgain 콜백 전달

- [ ] Task 4-2: SecurityWarningDialog 스타일 구현
  - **설명**:
    - 경고 아이콘 크기 48px, 색상 Orange (#FF9800)
    - URL 폰트: monospace, 색상: secondary
    - 메시지 폰트 크기 18px, 줄 높이 1.5
    - 체크박스 레이아웃 조정
  - **산출물**: `src/components/SecurityWarningDialog/SecurityWarningDialog.module.less` (신규, 약 50줄)
  - **완료 기준**: 프로젝터 대화면에서 가독성 확보

- [ ] Task 4-3: 컴포넌트 테스트 작성
  - **설명**:
    - 다이얼로그 표시 테스트 (visible prop)
    - 계속 진행 버튼 클릭 테스트
    - 돌아가기 버튼 클릭 테스트
    - 체크박스 클릭 시 onDontShowAgain 콜백 호출 테스트
  - **산출물**: `src/components/SecurityWarningDialog/__tests__/SecurityWarningDialog.test.js` (신규, 약 100줄)
  - **완료 기준**: 모든 테스트 케이스 통과

---

### Phase 5: BrowserView 통합
**담당**: frontend-dev
**의존성**: Phase 1, 3, 4 완료 필요
**예상 소요 시간**: 2시간

- [ ] Task 5-1: BrowserView 상태 관리 추가
  - **설명**:
    - 신규 상태 추가:
      - showSecurityWarning (Boolean): 경고 다이얼로그 표시 여부
      - pendingUrl (String): 경고 대기 중인 URL
    - useRef 추가: securityWarningTimerRef (디바운스 타이머)
  - **파일**: `src/views/BrowserView.js` (수정, 상단 상태 정의 영역)
  - **변경 위치**: line 60-80 부근 (상태 정의 영역)
  - **완료 기준**: 상태 정의 정상 동작

- [ ] Task 5-2: handleNavigationChange 확장
  - **설명**:
    - 기존 로직 유지 (히스토리 스택 업데이트, line 307-328)
    - 보안 경고 체크 로직 추가 (500ms 디바운스):
      - securityWarningTimerRef 타이머 취소/재시작
      - 500ms 후 checkSecurityWarning(url) 호출
    - checkSecurityWarning(url) 함수 구현:
      - classifyUrl(url) → 'insecure'인지 확인
      - extractDomain(url) → 도메인 추출
      - getIgnoredDomains() → 무시 목록 조회
      - 무시 목록에 없으면 showSecurityWarning=true, pendingUrl=url
  - **파일**: `src/views/BrowserView.js` (수정, line 307-328 확장)
  - **완료 기준**:
    - HTTP 사이트 접속 시 500ms 후 경고 다이얼로그 표시
    - HTTPS 사이트는 경고 표시하지 않음
    - 빠른 리다이렉트 시 타이머 취소 동작

- [ ] Task 5-3: SecurityWarningDialog 이벤트 핸들러 구현
  - **설명**:
    - handleSecurityContinue(): showSecurityWarning=false, pendingUrl=null
    - handleSecurityGoBack(): 다이얼로그 닫기 + webviewRef.goBack() 호출
    - handleSecurityDontShowAgain(checked):
      - checked === true 일 때 addIgnoredDomain(extractDomain(pendingUrl))
      - SessionStorage에 도메인 추가
  - **파일**: `src/views/BrowserView.js` (수정, 말단 핸들러 영역 추가)
  - **변경 위치**: line 400-500 부근 (핸들러 함수 영역)
  - **완료 기준**:
    - "계속 진행" 버튼으로 경고 닫기 동작
    - "돌아가기" 버튼으로 이전 페이지 이동 동작
    - 무시 옵션 체크 시 SessionStorage에 도메인 추가

- [ ] Task 5-4: SecurityWarningDialog JSX 렌더링 추가
  - **설명**:
    - BrowserView JSX의 말단(line 815 부근, DownloadConfirmDialog 뒤)에 SecurityWarningDialog 추가
    - Props 전달: visible={showSecurityWarning}, url={pendingUrl}, onContinue={handleSecurityContinue}, onGoBack={handleSecurityGoBack}, onDontShowAgain={handleSecurityDontShowAgain}
  - **파일**: `src/views/BrowserView.js` (수정, line 815 부근)
  - **완료 기준**: 다이얼로그 렌더링 정상 동작

---

### Phase 6: 통합 테스트 및 검증
**담당**: test-runner
**의존성**: Phase 5 완료 필요
**예상 소요 시간**: 1.5시간

- [ ] Task 6-1: BrowserView 통합 테스트 작성
  - **설명**:
    - HTTPS 사이트 접속 시 녹색 자물쇠 표시 시나리오
    - HTTP 사이트 접속 시 경고 다이얼로그 표시 시나리오 (500ms 대기)
    - localhost 예외 처리 시나리오 (경고 없이 정보 아이콘)
    - 경고 무시 옵션 사용 시나리오 (재접속 시 경고 없음)
    - 페이지 이동 시 보안 아이콘 실시간 업데이트 시나리오
  - **산출물**: `src/views/__tests__/BrowserView.security.test.js` (신규, 약 150줄)
  - **완료 기준**: 주요 시나리오 모두 검증

- [ ] Task 6-2: E2E 시나리오 테스트 (수동)
  - **설명**:
    - 실제 webOS 시뮬레이터/프로젝터에서 테스트
    - HTTPS 사이트 (https://www.google.com) 접속 → 녹색 자물쇠 확인
    - HTTP 사이트 (http://example.com) 접속 → 경고 다이얼로그 확인
    - HTTP → HTTPS 자동 리다이렉트 (http://github.com) → 경고 숨김 확인
    - 경고 무시 옵션 사용 → 재접속 시 경고 없음 확인
    - 앱 재시작 → 무시 옵션 초기화 확인
  - **완료 기준**: 모든 E2E 시나리오 통과

---

### Phase 7: 코드 및 문서 리뷰
**담당**: code-reviewer
**의존성**: Phase 6 완료 필요
**예상 소요 시간**: 1시간

- [ ] Task 7-1: 코드 리뷰
  - **검증 항목**:
    - securityClassifier.js URL 분류 로직 정확성
    - securityStorage.js SessionStorage 예외 처리
    - SecurityIndicator 컴포넌트 Spotlight 동작
    - URLBar 레이아웃 시프트 여부
    - BrowserView handleNavigationChange 디바운스 타이밍
    - SecurityWarningDialog UX (버튼 순서, 체크박스 동작)
  - **완료 기준**: 코드 품질 기준 충족

- [ ] Task 7-2: 기존 기능과의 호환성 검증
  - **검증 항목**:
    - F-02 (웹뷰 통합): onNavigationChange 기존 동작 유지
    - F-03 (URL 입력 UI): URLBar 레이아웃 정상 동작
    - F-04 (네비게이션 바): "돌아가기" 버튼 동작 일치
    - F-05 (로딩 인디케이터): 독립적 동작 확인
    - F-12 (다운로드): DownloadConfirmDialog와 충돌 없음
  - **완료 기준**: 기존 기능 회귀 없음

- [ ] Task 7-3: 수용 기준(AC) 체크리스트 검증
  - **검증 항목**: requirements.md의 AC-1~AC-7 모두 충족 확인
  - **완료 기준**: 모든 AC 통과

---

## 3. 태스크 의존성

```
Phase 1: 유틸리티 구현
├─ Task 1-1 (securityClassifier.js)
│     ├──▶ Task 1-3 (단위 테스트)
│     │
│     ├──▶ Phase 2: SecurityIndicator 구현
│     │     ├─ Task 2-1 (컴포넌트)
│     │     ├─ Task 2-2 (스타일)
│     │     └─ Task 2-3 (테스트)
│     │          │
│     │          ├──▶ Phase 3: URLBar 확장
│     │          │     ├─ Task 3-1 (로직 확장)
│     │          │     ├─ Task 3-2 (레이아웃)
│     │          │     └─ Task 3-3 (포커스 순서)
│     │          │          │
│     │          │          └──▶ Phase 5: BrowserView 통합
│     │          │
│     │          └──▶ Phase 4: SecurityWarningDialog 구현 (Phase 3과 병렬 가능)
│     │                ├─ Task 4-1 (컴포넌트)
│     │                ├─ Task 4-2 (스타일)
│     │                └─ Task 4-3 (테스트)
│     │                     │
│     │                     └──▶ Phase 5: BrowserView 통합
│     │
│     └──▶ Phase 5: BrowserView 통합
│           ├─ Task 5-1 (상태 관리)
│           ├─ Task 5-2 (handleNavigationChange)
│           ├─ Task 5-3 (이벤트 핸들러)
│           └─ Task 5-4 (JSX 렌더링)
│                │
│                └──▶ Phase 6: 통합 테스트
│                      ├─ Task 6-1 (통합 테스트)
│                      └─ Task 6-2 (E2E 테스트)
│                           │
│                           └──▶ Phase 7: 코드 리뷰
│                                 ├─ Task 7-1 (코드 리뷰)
│                                 ├─ Task 7-2 (호환성 검증)
│                                 └─ Task 7-3 (AC 체크)
│
└─ Task 1-2 (securityStorage.js)
      └──▶ Task 1-3 (단위 테스트)
           └──▶ Phase 4, 5 (SessionStorage 사용)
```

**병렬 가능한 태스크**:
- Phase 3 (URLBar 확장) 과 Phase 4 (SecurityWarningDialog 구현) 병렬 실행 가능
  - 두 Phase 모두 Phase 1(유틸리티)만 의존하고, 서로 독립적
  - Phase 5에서 합류

---

## 4. 충돌 영역 분석 (F-14 vs F-13)

### F-14 (HTTPS 보안 표시) 수정 파일
| 파일 | 변경 유형 | 변경 범위 |
|------|----------|----------|
| `src/utils/securityClassifier.js` | 신규 생성 | 전체 (80줄) |
| `src/utils/securityStorage.js` | 신규 생성 | 전체 (70줄) |
| `src/components/SecurityIndicator/SecurityIndicator.js` | 신규 생성 | 전체 (100줄) |
| `src/components/SecurityIndicator/SecurityIndicator.module.less` | 신규 생성 | 전체 (40줄) |
| `src/components/SecurityWarningDialog/SecurityWarningDialog.js` | 신규 생성 | 전체 (120줄) |
| `src/components/SecurityWarningDialog/SecurityWarningDialog.module.less` | 신규 생성 | 전체 (50줄) |
| `src/components/URLBar/URLBar.js` | Minor 변경 | 230줄 → 260줄 (SecurityIndicator 통합, line 159-171) |
| `src/components/URLBar/URLBar.module.less` | Minor 변경 | flexbox 레이아웃 추가 |
| `src/views/BrowserView.js` | Minor 변경 | 820줄 → 900줄 (80줄 추가) |

### F-13 (리모컨 단축키) 수정 파일 (재확인)
| 파일 | 변경 유형 | 변경 범위 |
|------|----------|----------|
| `src/utils/keyCodeConstants.js` | 신규 생성 | 전체 (80줄) |
| `src/hooks/useRemoteControl.js` | Major 변경 | 70줄 → 200줄 (130줄 추가) |
| `src/views/BrowserView.js` | Minor 변경 | 820줄 → 880줄 (60줄 추가, 핸들러 함수) |

### BrowserView.js 충돌 영역 상세 분석

| 항목 | F-14 변경 위치 | F-13 변경 위치 | 충돌 여부 |
|------|---------------|---------------|----------|
| **상태 정의 (line 60-80)** | 추가: showSecurityWarning, pendingUrl, securityWarningTimerRef | 없음 | ✅ 충돌 없음 |
| **handleNavigationChange (line 307-328)** | 확장: 보안 경고 체크 로직 추가 (500ms 디바운스) | 없음 | ✅ 충돌 없음 |
| **핸들러 함수 (line 400-500)** | 추가: handleSecurityContinue, handleSecurityGoBack, handleSecurityDontShowAgain | 추가: handleChannelUp, handleChannelDown, handleNumberKey | ⚠️ **잠재적 충돌 (낮음)** |
| **useRemoteControl 호출 (line 550-580)** | 없음 | 확장: 신규 옵션 추가 | ✅ 충돌 없음 |
| **JSX 렌더링 (line 815)** | 추가: SecurityWarningDialog | 없음 | ✅ 충돌 없음 |

### 충돌 가능성 평가

| 항목 | F-14 영역 | F-13 영역 | 충돌 가능성 |
|------|----------|----------|------------|
| **수정 파일 겹침** | URLBar.js, BrowserView.js | useRemoteControl.js, BrowserView.js | ⚠️ **BrowserView.js만 겹침** |
| **BrowserView.js 상태** | showSecurityWarning, pendingUrl | 없음 | ✅ 독립적 |
| **BrowserView.js 핸들러** | handleSecurity* (3개) | handleChannel*, handleNumberKey (3개) | ⚠️ **동일 영역에 추가** (낮음) |
| **handleNavigationChange** | 보안 체크 로직 추가 (말단) | 없음 | ✅ 독립적 |
| **Hook 의존성** | 없음 (securityClassifier.js 유틸리티만 사용) | useRemoteControl 확장 | ✅ 독립적 |
| **UI 컴포넌트** | URLBar (내부 수정), 신규 컴포넌트 | useRemoteControl (Hook), BrowserView (핸들러) | ✅ 독립적 |

### 결론: 병렬 실행 가능 🟢

**근거**:
1. **파일 충돌 최소화**: F-14는 주로 URLBar와 신규 컴포넌트 수정, F-13은 useRemoteControl Hook 수정
2. **BrowserView.js 충돌 범위 명확히 분리**:
   - F-14: 상태 정의(상단), handleNavigationChange 확장(중간), 핸들러 추가(말단), JSX(말단)
   - F-13: 핸들러 추가(말단), useRemoteControl 호출(중간)
   - 충돌 가능 영역: 핸들러 함수 영역 (line 400-500), 하지만 함수 이름이 다르므로 병합 가능
3. **상태 의존성 독립적**:
   - F-14: showSecurityWarning, pendingUrl (신규 상태)
   - F-13: 기존 상태(showKeyboard, activeTab) 읽기 전용
4. **Hook 독립성**: F-14는 Hook 수정 없음, F-13은 useRemoteControl 확장

**주의사항**:
- BrowserView.js 핸들러 함수 영역(line 400-500)에서 F-14와 F-13 모두 새로운 함수 추가
  - F-14: handleSecurityContinue, handleSecurityGoBack, handleSecurityDontShowAgain
  - F-13: handleChannelUp, handleChannelDown, handleNumberKey
  - 함수 이름이 다르므로 Git merge 시 자동 병합 가능
  - merge 시 함수 순서만 조정하면 충돌 해결
- F-13과 F-14 병렬 진행 시, code-reviewer가 BrowserView.js 변경사항 함께 검토 필요

---

## 5. 병렬 실행 판단

### F-14 내부 병렬 실행: YES ✅

**근거**:
- Phase 3 (URLBar 확장) 과 Phase 4 (SecurityWarningDialog 구현) 병렬 가능
- 두 Phase 모두 Phase 1(유틸리티)만 의존
- 서로 독립적인 컴포넌트 (URLBar vs SecurityWarningDialog)
- Phase 5에서 BrowserView 통합 시 합류

### F-14 vs F-13 병렬 실행: YES ✅ (권장)

**근거**:
1. **파일 충돌 최소화**:
   - F-14: URLBar.js, 신규 컴포넌트 6개
   - F-13: useRemoteControl.js, keyCodeConstants.js
   - 공통 파일: BrowserView.js만 (충돌 범위 명확히 분리됨)
2. **BrowserView.js 병합 가능**:
   - F-14: 상태 정의(상단), handleNavigationChange(중간), 핸들러(말단), JSX(말단)
   - F-13: 핸들러(말단), useRemoteControl 호출(중간)
   - 함수 이름 다르므로 Git merge 자동 가능
3. **소요 시간 단축**:
   - F-14: 10.5시간 (순차 실행)
   - F-13: 7시간 (plan.md 참조)
   - 병렬 실행 시 약 10.5시간 (F-14가 더 김) → 7시간 절약
4. **M3 마일스톤 우선순위**:
   - F-13: Priority 5 (Could)
   - F-14: Priority 4 (Must)
   - 동시 진행으로 M3 납기 준수 가능

### Agent Team 사용 권장 여부: YES ✅ (F-14 vs F-13 병렬)

**권장 실행 방식**:
- F-14와 F-13을 별도 worktree에서 병렬 개발
- F-14 worktree: `.worktrees/https-security-indicator/` → `feature/https-security-indicator` 브랜치
- F-13 worktree: `.worktrees/remote-control-shortcuts/` → `feature/remote-control-shortcuts` 브랜치
- 두 기능 완료 후 순차 merge (F-14 먼저, F-13 나중)
- code-reviewer가 merge 시 BrowserView.js 변경사항 함께 검토

---

## 6. 예상 소요 시간

| Phase | 작업 내용 | 예상 시간 |
|-------|----------|----------|
| Phase 1 | 유틸리티 구현 | 1시간 |
| Phase 2 | SecurityIndicator 컴포넌트 | 1.5시간 |
| Phase 3 | URLBar 확장 | 1.5시간 |
| Phase 4 | SecurityWarningDialog 컴포넌트 | 2시간 |
| Phase 5 | BrowserView 통합 | 2시간 |
| Phase 6 | 통합 테스트 및 검증 | 1.5시간 |
| Phase 7 | 코드 및 문서 리뷰 | 1시간 |
| **총합** | | **10.5시간** |

**병렬 실행 시 단축 시간**:
- Phase 3 + Phase 4 병렬: 2시간 → 2시간 (단축 없음, 단 리소스 2배 투입 시 병렬 효과)
- F-14 + F-13 병렬: 10.5시간 + 7시간 → 10.5시간 (7시간 단축)

---

## 7. 리스크 및 완화 전략

### 리스크 1: CORS 제약으로 인증서 정보 접근 불가
- **영향도**: Medium (기능 제약)
- **발생 확률**: 100% (확정)
- **완화 전략**:
  - URL 프로토콜 기반 보안 수준 분류로 제한
  - requirements.md의 "범위 외" 섹션에 명시
  - 사용자에게 프로토콜 기반 보안 표시임을 툴팁으로 안내

### 리스크 2: 빠른 리다이렉트 시 불필요한 경고 표시 (HTTP → HTTPS)
- **영향도**: Medium (UX 저하)
- **발생 확률**: 40%
- **완화 전략**:
  - 500ms 디바운스로 경고 표시 지연
  - 타이머 취소/재시작으로 자동 리다이렉트 감지
  - 최종 URL이 HTTPS이면 경고 표시하지 않음
  - Task 6-2 E2E 테스트에서 http://github.com 시나리오 검증

### 리스크 3: webOS 프로젝터에서 이모지 아이콘 렌더링 실패
- **영향도**: High (아이콘 미표시)
- **발생 확률**: 20%
- **완화 전략**:
  - 초기 구현은 유니코드 이모지 (🔒, ⚠, ℹ️) 사용
  - E2E 테스트에서 렌더링 실패 확인 시, Enact Moonstone Icon 컴포넌트로 대체
  - SVG 아이콘으로 fallback 구현 (선택적)

### 리스크 4: URLBar 레이아웃 시프트 (보안 아이콘 추가 시)
- **영향도**: Medium (UX 저하)
- **발생 확률**: 30%
- **완화 전략**:
  - SecurityIndicator 고정 너비 48px 확보
  - unknown 등급일 때도 빈 영역 유지 (display: inline-flex, width: 48px)
  - Task 7-1 코드 리뷰에서 레이아웃 시프트 검증

### 리스크 5: SessionStorage 용량 초과 (도메인 목록 100개 이상)
- **영향도**: Low (드문 케이스)
- **발생 확률**: 5%
- **완화 전략**:
  - addIgnoredDomain()에서 try-catch로 예외 처리
  - 도메인 100개 초과 시 가장 오래된 도메인 자동 제거 (LRU 방식, 선택적)
  - 일반 사용자는 100개 이하 사용 예상

### 리스크 6: F-13과 BrowserView.js merge 충돌
- **영향도**: Low (수동 해결 가능)
- **발생 확률**: 40% (병렬 진행 시)
- **완화 전략**:
  - F-14 완료 후 F-13 merge 순서 권장 (F-14가 먼저 완료 예상)
  - code-reviewer가 merge 시 핸들러 함수 순서 조정
  - 함수 이름 다르므로 충돌 최소화 (자동 병합 가능)

### 리스크 7: Spotlight 포커스 순서 혼란 (SecurityIndicator 추가 시)
- **영향도**: Medium (UX 저하)
- **발생 확률**: 30%
- **완화 전략**:
  - Task 3-3에서 Spotlight leaveFor 설정 명확히 정의
  - URLBar 포커스 순서: SecurityIndicator → Input → VirtualKeyboard
  - E2E 테스트에서 리모컨 방향키 탐색 검증

---

## 8. 완료 산출물

### 신규 파일 (총 6개 + 테스트 5개)
- `src/utils/securityClassifier.js` (80줄)
- `src/utils/securityStorage.js` (70줄)
- `src/components/SecurityIndicator/SecurityIndicator.js` (100줄)
- `src/components/SecurityIndicator/SecurityIndicator.module.less` (40줄)
- `src/components/SecurityWarningDialog/SecurityWarningDialog.js` (120줄)
- `src/components/SecurityWarningDialog/SecurityWarningDialog.module.less` (50줄)

### 신규 테스트 파일
- `src/utils/__tests__/securityClassifier.test.js` (80줄)
- `src/utils/__tests__/securityStorage.test.js` (60줄)
- `src/components/SecurityIndicator/__tests__/SecurityIndicator.test.js` (80줄)
- `src/components/SecurityWarningDialog/__tests__/SecurityWarningDialog.test.js` (100줄)
- `src/views/__tests__/BrowserView.security.test.js` (150줄)

### 수정 파일
- `src/components/URLBar/URLBar.js` (230줄 → 260줄, 30줄 추가)
- `src/components/URLBar/URLBar.module.less` (flexbox 레이아웃 추가)
- `src/views/BrowserView.js` (820줄 → 900줄, 80줄 추가)

### 변경 없는 파일
- `src/contexts/TabContext.js` (읽기 전용)
- `src/components/WebView/WebView.js` (onNavigationChange 콜백 재사용)
- `src/hooks/useRemoteControl.js` (영향 없음)

---

## 9. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초안 작성 | product-manager |

---

## 10. 다음 단계

### 10.1. F-14 단독 실행 시
- `/fullstack-feature F-14: HTTPS 보안 표시 구현` 명령어 실행
- Phase 1 → 2 → 3 → 4 → 5 → 6 → 7 순차 실행

### 10.2. F-14 + F-13 병렬 실행 시 (권장)
- `/fullstack-feature-team F-14(HTTPS 보안 표시) + F-13(리모컨 단축키) 병렬 구현` 명령어 실행
- F-14 worktree: `.worktrees/https-security-indicator/`
- F-13 worktree: `.worktrees/remote-control-shortcuts/`
- 두 기능 완료 후 순차 merge (F-14 먼저 → F-13 나중)

### 10.3. 선택적 구현 (M3 범위 외)
- **FR-5**: 보안 아이콘 클릭 시 상세 정보 다이얼로그
  - SecurityInfoDialog 컴포넌트 구현
  - 표시 내용: URL, 프로토콜, 보안 수준 (인증서 정보 불가)
  - SecurityIndicator onClick 이벤트 활성화
