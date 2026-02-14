# F-13: 리모컨 단축키 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/remote-control-shortcuts/requirements.md
- 기술 설계서: docs/specs/remote-control-shortcuts/design.md

---

## 2. 구현 Phase

### Phase 1: 기반 작업 (키 상수 정의)
**담당**: frontend-dev
**예상 소요 시간**: 30분

- [ ] Task 1-1: keyCodeConstants.js 파일 생성 및 키 코드 상수 정의
  - **설명**:
    - webOS 리모컨 키 코드를 상수로 정의 (채널, 컬러, 숫자, 설정 버튼)
    - isKeyMatch(), getNumberFromKeyCode() 헬퍼 함수 구현
  - **산출물**: `src/utils/keyCodeConstants.js` (신규, 약 80줄)
  - **완료 기준**:
    - 모든 필요한 키 코드 상수 정의 완료
    - 헬퍼 함수 동작 검증 (Jest 단위 테스트)

---

### Phase 2: useRemoteControl Hook 확장
**담당**: frontend-dev
**의존성**: Phase 1 완료 필요
**예상 소요 시간**: 2시간

- [ ] Task 2-1: useRemoteControl.js 옵션 인터페이스 확장
  - **설명**:
    - 기존 옵션 유지 (isWebViewFocused, onBackInWebView, onBackOutsideWebView, onYellowButton)
    - 신규 옵션 추가:
      - isKeyboardOpen (URLBar 키보드 상태)
      - onChannelUp, onChannelDown (채널 버튼 콜백)
      - onRedButton, onGreenButton, onBlueButton (컬러 버튼 콜백)
      - onNumberKey (숫자 키 콜백, 1~5)
      - onMenuButton (설정 버튼 콜백)
  - **산출물**: `src/hooks/useRemoteControl.js` (70줄 → 200줄, 약 130줄 추가)
  - **완료 기준**:
    - 모든 옵션 콜백이 적절한 keyCode에 매핑됨
    - keyCodeConstants.js import 정상 동작

- [ ] Task 2-2: 디바운스 로직 추가
  - **설명**:
    - useRef로 마지막 키 입력 시각 기록 (lastKeyTime)
    - 0.5초 이내 동일 키 재입력 무시
    - 키별 독립적인 디바운스 타이머 관리
  - **완료 기준**:
    - 연속 입력 시 0.5초 이내 중복 호출 방지 확인

- [ ] Task 2-3: 키보드 열린 상태 분기 처리
  - **설명**:
    - isKeyboardOpen === true 일 때:
      - 숫자 키(49~53) → 무시 (URLBar 입력 우선)
      - 채널 버튼(427, 428) → 무시
      - 컬러 버튼, 설정 버튼 → 허용 (패널 열기 가능)
    - Back 키는 기존 동작 유지
  - **완료 기준**:
    - URLBar 키보드 열린 상태에서 숫자 키 입력 시 탭 전환 안 됨

- [ ] Task 2-4: logger.debug로 모든 키 입력 기록
  - **설명**:
    - handleKeyDown에서 keyCode, event.key, isKeyboardOpen 로그
    - 처리된 키: logger.info
    - 무시된 키: logger.debug
  - **완료 기준**:
    - 리모컨 키 입력 시 콘솔에 로그 출력 확인

---

### Phase 3: BrowserView.js 핸들러 구현
**담당**: frontend-dev
**의존성**: Phase 2 완료 필요
**예상 소요 시간**: 2시간

- [ ] Task 3-1: 채널 버튼 핸들러 구현 (handleChannelUp, handleChannelDown)
  - **설명**:
    - 이전/다음 탭 전환 로직 (순환 전환)
    - 탭 1개일 때 무시
    - TAB_ACTIONS.SWITCH_TAB dispatch
    - setInputValue(newUrl) 업데이트
    - Spotlight.focus('webview-main') 호출
  - **산출물**: `src/views/BrowserView.js` (820줄 → 880줄, 약 60줄 추가)
  - **완료 기준**:
    - Channel Up/Down 버튼으로 탭 전환 동작
    - 순환 전환 (첫 탭 ↔ 마지막 탭) 확인

- [ ] Task 3-2: 숫자 키 핸들러 구현 (handleNumberKey)
  - **설명**:
    - 입력된 숫자(1~5)로 직접 탭 선택
    - 존재하지 않는 탭 번호 무시
    - 이미 활성화된 탭 무시
    - 탭 전환 후 WebView 포커스
  - **완료 기준**:
    - 숫자 3 버튼으로 세 번째 탭 선택 가능
    - 탭 4개일 때 숫자 5 버튼 무시

- [ ] Task 3-3: useRemoteControl Hook 호출 확장
  - **설명**:
    - 기존 옵션 유지 (isWebViewFocused, onBackInWebView, onBackOutsideWebView, onYellowButton)
    - 신규 옵션 추가:
      - isKeyboardOpen: showKeyboard
      - onChannelUp: handleChannelUp
      - onChannelDown: handleChannelDown
      - onRedButton: handleBookmarkClick (기존 함수 재사용)
      - onGreenButton: handleHistoryClick (기존 함수 재사용)
      - onBlueButton: handleNewTab (기존 함수 재사용)
      - onNumberKey: handleNumberKey
      - onMenuButton: handleSettingsClick (기존 함수 재사용)
  - **완료 기준**:
    - 모든 단축키가 적절한 핸들러에 매핑됨
    - ESLint 에러 없음

---

### Phase 4: 테스트 작성
**담당**: test-runner
**의존성**: Phase 3 완료 필요
**예상 소요 시간**: 1.5시간

- [ ] Task 4-1: useRemoteControl Hook 단위 테스트 확장
  - **설명**:
    - 채널 버튼 감지 테스트 (keyCode 427, 428)
    - 컬러 버튼 감지 테스트 (Red/Green/Blue)
    - 숫자 키 감지 테스트 (1~5)
    - 설정 버튼 감지 테스트 (keyCode 457, 18)
    - 디바운스 테스트 (0.5초 이내 중복 입력 무시)
    - 키보드 열린 상태 분기 테스트
  - **산출물**: `src/hooks/__tests__/useRemoteControl.test.js` (확장)
  - **완료 기준**: 모든 테스트 케이스 통과

- [ ] Task 4-2: BrowserView 통합 테스트 작성
  - **설명**:
    - Channel Down 버튼으로 탭 전환 시나리오
    - 숫자 3 버튼으로 직접 탭 선택 시나리오
    - Red 버튼으로 북마크 패널 열기
    - Blue 버튼으로 최대 탭 수 제한 체크
    - URLBar 키보드 열린 상태에서 숫자 키 입력 시 충돌 방지
  - **산출물**: `src/views/__tests__/BrowserView.shortcuts.test.js` (신규)
  - **완료 기준**: 주요 시나리오 모두 검증

---

### Phase 5: 검증 및 리뷰
**담당**: code-reviewer
**의존성**: Phase 4 완료 필요
**예상 소요 시간**: 1시간

- [ ] Task 5-1: 코드 리뷰
  - **검증 항목**:
    - useRemoteControl Hook 확장 로직 검토
    - 디바운스 타이밍 적절성 (0.5초 vs 0.3초)
    - 키보드 우선순위 분기 처리 검토
    - BrowserView 핸들러 함수 로직 검증
    - keyCodeConstants.js 상수 정의 완전성

- [ ] Task 5-2: 기존 기능과의 호환성 검증
  - **검증 항목**:
    - F-04 (페이지 탐색): Back 키 기존 동작 유지 확인
    - F-06 (탭 관리): 채널 버튼 탭 전환과 기존 TabBar 클릭 동작 일치
    - F-07 (북마크): Red 버튼 동작과 NavigationBar 버튼 동작 일치
    - F-08 (히스토리): Green 버튼 동작 확인
    - F-12 (다운로드): Yellow 버튼 기존 동작 유지
    - F-11 (설정): Menu 버튼 동작 확인

- [ ] Task 5-3: 수용 기준(AC) 체크리스트 검증
  - **검증 항목**: requirements.md의 AC-1~AC-8 모두 충족 확인

---

## 3. 태스크 의존성

```
Task 1-1 (keyCodeConstants.js 생성)
   │
   ├──▶ Task 2-1 (useRemoteControl 옵션 확장)
   │       │
   │       ├──▶ Task 2-2 (디바운스 로직)
   │       │       │
   │       │       ├──▶ Task 2-3 (키보드 분기 처리)
   │       │       │       │
   │       │       │       └──▶ Task 2-4 (로깅 추가)
   │       │       │               │
   │       │       │               ├──▶ Task 3-1 (채널 버튼 핸들러)
   │       │       │               │       │
   │       │       │               │       ├──▶ Task 3-2 (숫자 키 핸들러)
   │       │       │               │       │       │
   │       │       │               │       │       └──▶ Task 3-3 (Hook 호출 확장)
   │       │       │               │       │               │
   │       │       │               │       │               └──▶ Task 4-1 (Hook 테스트)
   │       │       │               │       │                       │
   │       │       │               │       │                       ├──▶ Task 4-2 (BrowserView 테스트)
   │       │       │               │       │                       │       │
   │       │       │               │       │                       │       └──▶ Task 5-1 (코드 리뷰)
   │       │       │               │       │                       │               │
   │       │       │               │       │                       │               ├──▶ Task 5-2 (호환성 검증)
   │       │       │               │       │                       │               │       │
   │       │       │               │       │                       │               │       └──▶ Task 5-3 (AC 체크)
   │       │       │               │       │                       │               │
   │       │       │               │       │                       │               └──▶ 완료
```

**병렬 가능한 태스크**:
- 없음 (모든 태스크가 순차 의존성을 가짐)

---

## 4. 충돌 영역 분석 (F-13 vs F-14)

### F-13 (리모컨 단축키) 수정 파일
| 파일 | 변경 유형 | 변경 범위 |
|------|----------|----------|
| `src/utils/keyCodeConstants.js` | 신규 생성 | 전체 (80줄) |
| `src/hooks/useRemoteControl.js` | Major 변경 | 70줄 → 200줄 (130줄 추가) |
| `src/views/BrowserView.js` | Minor 변경 | 820줄 → 880줄 (60줄 추가, 핸들러 함수) |

### F-14 (HTTPS 보안 표시) 예상 수정 파일
| 파일 | 변경 유형 | 변경 범위 (예상) |
|------|----------|----------------|
| `src/components/URLBar/URLBar.js` | Minor 변경 | 자물쇠 아이콘 추가 (UI 레이어) |
| `src/components/WebView/WebView.js` | Minor 변경 | URL 변경 이벤트 리스닝 (보안 상태 전달) |
| `src/utils/securityHelper.js` | 신규 생성 | isSecureURL() 헬퍼 함수 |

### 충돌 영역 판단
| 항목 | F-13 영역 | F-14 영역 | 충돌 여부 |
|------|----------|----------|----------|
| **수정 파일** | useRemoteControl.js, BrowserView.js | URLBar.js, WebView.js | ❌ 겹치지 않음 |
| **BrowserView.js 수정 범위** | 핸들러 함수 추가 (말단) | URL 상태 관리 (상단) | ⚠️ 잠재적 충돌 (낮음) |
| **Hook 의존성** | useRemoteControl (확장) | useSecurityIndicator (신규) | ❌ 독립적 |
| **Context/상태** | TabContext (읽기 전용) | activeTab.url (읽기 전용) | ❌ 충돌 없음 |
| **UI 컴포넌트** | NavigationBar, TabBar (호출만) | URLBar (내부 수정) | ❌ 독립적 |

### 결론: 병렬 실행 가능 🟢

**근거**:
1. **파일 충돌 없음**: F-13은 useRemoteControl.js, F-14는 URLBar.js/WebView.js를 수정
2. **BrowserView.js 충돌 낮음**:
   - F-13: 핸들러 함수 추가 (말단 영역, 약 60줄)
   - F-14: URL 상태 관리 또는 보안 아이콘 표시 로직 추가 (상단 영역)
   - 수정 범위가 명확히 분리됨
3. **상태 의존성 독립적**:
   - F-13: TabContext의 SWITCH_TAB 액션 호출 (기존 API)
   - F-14: activeTab.url 읽기 전용 접근
4. **Hook 독립성**: useRemoteControl 확장 vs useSecurityIndicator 신규 생성

**주의사항**:
- BrowserView.js에서 merge 시 충돌 가능성 낮지만, 코드 리뷰 시 함께 확인 필요
- F-14에서 URLBar에 새로운 props 추가 시, F-13에서 URLBar 호출 부분에 영향 없음 (읽기 전용)

---

## 5. 병렬 실행 판단

### Agent Team 사용 권장 여부: **NO** ❌

**근거**:
1. **순차 의존성**:
   - Phase 1 (keyCodeConstants.js) → Phase 2 (useRemoteControl) → Phase 3 (BrowserView) → Phase 4 (테스트) → Phase 5 (리뷰)
   - 모든 태스크가 선형 의존성을 가짐
2. **컴포넌트 개수**: 1개 (useRemoteControl Hook 확장)
   - Agent Team 기준: 독립 컴포넌트 2개 이상 필요
3. **작업 복잡도**:
   - useRemoteControl Hook 확장이 핵심 (200줄)
   - BrowserView 핸들러는 간단 (각 20줄)
   - 분리 실익 없음
4. **소요 시간**:
   - 전체 약 7시간 (상수 정의 0.5h + Hook 확장 2h + 핸들러 2h + 테스트 1.5h + 리뷰 1h)
   - 순차 작업이 더 효율적

### 권장 실행 방식: **순차 파이프라인** (/fullstack-feature)

---

## 6. 예상 소요 시간

| Phase | 작업 내용 | 예상 시간 |
|-------|----------|----------|
| Phase 1 | 키 상수 정의 | 0.5시간 |
| Phase 2 | useRemoteControl Hook 확장 | 2시간 |
| Phase 3 | BrowserView 핸들러 구현 | 2시간 |
| Phase 4 | 테스트 작성 | 1.5시간 |
| Phase 5 | 검증 및 리뷰 | 1시간 |
| **총합** | | **7시간** |

---

## 7. 리스크 및 완화 전략

### 리스크 1: Yellow 버튼과 Green 버튼 keyCode 충돌 (405, 406)
- **영향도**: Medium
- **발생 확률**: 30% (webOS 기기별 차이)
- **완화 전략**:
  - useRemoteControl에서 Yellow 버튼 처리를 Green보다 먼저 배치 (기존 동작 우선)
  - 실제 webOS 기기에서 keyCode 로그를 남겨 차이 확인
  - event.key 추가 확인으로 보완 (가능 시)

### 리스크 2: 디바운스 타이밍이 너무 길어 UX 저하
- **영향도**: Low
- **발생 확률**: 20%
- **완화 전략**:
  - 초기 구현은 0.5초로 설정
  - 실제 사용 테스트 후 0.3초로 축소 고려
  - 키별로 다른 디바운스 시간 적용 가능 (채널: 300ms, 컬러: 500ms)

### 리스크 3: URLBar 키보드 열린 상태에서 단축키 충돌
- **영향도**: High (Must 요구사항)
- **발생 확률**: 10%
- **완화 전략**:
  - Phase 2에서 isKeyboardOpen 분기 처리 철저히 구현
  - Task 4-2 통합 테스트에서 시나리오 검증
  - BrowserView에서 showKeyboard 상태를 정확히 전달

### 리스크 4: Spotlight 포커스 충돌 (패널 열린 상태)
- **영향도**: Medium
- **발생 확률**: 15%
- **완화 전략**:
  - 패널이 visible일 때 단축키 무시 로직 추가 (선택적)
  - BrowserView에서 패널 상태(showBookmarkPanel 등)를 useRemoteControl에 전달
  - Phase 5 호환성 검증에서 확인

### 리스크 5: 기존 Back 키 동작 변경 (회귀)
- **영향도**: High (기존 기능 손상)
- **발생 확률**: 5%
- **완화 전략**:
  - Phase 2에서 Back 키 로직 변경하지 않음 (기존 코드 유지)
  - Task 5-2 호환성 검증에서 F-04 동작 철저히 테스트
  - 단위 테스트에 Back 키 회귀 테스트 추가

### 리스크 6: F-14와 BrowserView.js merge 충돌
- **영향도**: Low
- **발생 확률**: 25% (병렬 진행 시)
- **완화 전략**:
  - F-13, F-14 병렬 진행 가능하나, BrowserView.js merge 시 주의
  - F-13은 핸들러 함수 추가 (말단), F-14는 상태 관리 (상단)으로 범위 분리
  - code-reviewer가 두 기능의 BrowserView.js 변경사항 함께 검토

---

## 8. 완료 산출물

### 신규 파일
- `src/utils/keyCodeConstants.js` (80줄)
- `src/views/__tests__/BrowserView.shortcuts.test.js` (신규 테스트)

### 수정 파일
- `src/hooks/useRemoteControl.js` (70줄 → 200줄)
- `src/views/BrowserView.js` (820줄 → 880줄)
- `src/hooks/__tests__/useRemoteControl.test.js` (테스트 확장)

### 변경 없는 파일
- `src/contexts/TabContext.js` (SWITCH_TAB 액션 재사용)
- `src/components/NavigationBar/NavigationBar.js` (단축키 로직 없음)

---

## 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초안 작성 | product-manager |
