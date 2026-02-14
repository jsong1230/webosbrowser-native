# F-13: 리모컨 단축키 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/remote-shortcuts/requirements.md
- 기술 설계서: docs/specs/remote-shortcuts/design.md

---

## 2. 구현 Phase

### Phase 1: 기반 구조 구축 (키 코드 상수 + 디바운스)
**목표**: webOS 리모컨 키 코드 상수 정의 및 디바운스 메커니즘 기반 마련

**담당**: cpp-dev

#### Task 1-1: KeyCodeConstants.h 파일 생성
- **파일**: `src/utils/KeyCodeConstants.h` (신규)
- **작업 내용**:
  - webOS 리모컨 키 코드 상수 정의
  - namespace webosbrowser::KeyCode 사용
  - constexpr로 컴파일 타임 상수 선언
  - 채널 버튼 (CHANNEL_UP, CHANNEL_DOWN)
  - 컬러 버튼 (RED, GREEN, YELLOW, BLUE)
  - 숫자 버튼 (NUM_1~5)
  - 설정 버튼 (MENU, SETTINGS)
  - 재생 버튼 (PLAY, PAUSE, FAST_FORWARD, REWIND) — M3 이후
  - 기타 (BACK)
  - 주석으로 webOS keyCode 및 Qt::Key 매핑 명시
- **완료 기준**:
  - 헤더 파일이 정상적으로 포함(include)되고 컴파일 에러 없음
  - 상수 값이 설계서와 일치
  - 주석으로 각 키 코드 설명 추가
- **예상 소요 시간**: 0.5시간

#### Task 1-2: BrowserWindow 헤더 확장 (디바운스 멤버 추가)
- **파일**: `src/browser/BrowserWindow.h` (수정)
- **작업 내용**:
  - keyPressEvent() 메서드 오버라이드 선언
  - 디바운스 관련 private 멤버 변수 추가:
    - `QMap<int, qint64> lastKeyPressTime_` — 키별 마지막 입력 시각
    - `static constexpr int DEBOUNCE_MS = 500` — 디바운스 지속 시간
  - 핸들러 메서드 선언:
    - `bool shouldDebounce(int keyCode)`
    - `void handleChannelButton(int keyCode)`
    - `void handleColorButton(int keyCode)`
    - `void handleNumberButton(int keyCode)`
    - `void handleMenuButton(int keyCode)`
    - `void handlePlaybackButton(int keyCode)` — M3 이후
  - 자동 스크롤 관련 멤버 (M3 이후):
    - `QTimer *autoScrollTimer_`
    - `bool isAutoScrolling_`
    - `static constexpr int AUTO_SCROLL_INTERVAL_MS = 10`
    - `static constexpr int AUTO_SCROLL_STEP_PX = 5`
- **완료 기준**:
  - 컴파일 에러 없음
  - 설계서 4.2장과 일치
  - Doxygen 주석 추가
- **예상 소요 시간**: 0.5시간

#### Task 1-3: shouldDebounce() 메서드 구현
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - `shouldDebounce(int keyCode)` 구현
  - QDateTime::currentMSecsSinceEpoch()로 현재 시각 조회
  - lastKeyPressTime_에서 해당 키의 마지막 입력 시각 조회
  - 0.5초(500ms) 이내 재입력이면 true 반환 (중복 입력)
  - 그렇지 않으면 lastKeyPressTime_ 업데이트 후 false 반환 (처리 허용)
- **완료 기준**:
  - 0.5초 이내 중복 입력이 무시되는지 단위 테스트 통과
  - Logger::debug로 디바운스 동작 로그 기록
- **예상 소요 시간**: 0.5시간

---

### Phase 2: 채널 버튼 탭 전환 (Must, FR-1)
**목표**: 리모컨 채널 Up/Down 버튼으로 탭 전환 기능 구현

**담당**: cpp-dev

**의존성**: Phase 1 완료, F-06 (탭 관리 시스템) 완료

#### Task 2-1: TabManager 다중 탭 지원 확인 및 리팩토링
- **파일**: `src/browser/TabManager.h`, `src/browser/TabManager.cpp` (수정)
- **작업 내용**:
  - 현재 TabManager가 다중 탭을 지원하는지 확인
  - 만약 단일 탭만 지원한다면 다음 작업 수행:
    - `QVector<TabData> tabs_` 멤버 추가 (TabData 구조체: url, title, historyState)
    - `int currentTabIndex_` 멤버 추가
    - `static constexpr int MAX_TABS = 5` 추가
    - `previousTab()` 메서드 구현 (순환 전환, 현재 인덱스 - 1)
    - `nextTab()` 메서드 구현 (순환 전환, 현재 인덱스 + 1)
    - `switchToTab(int index)` 메서드 구현
    - 탭 전환 시 시그널 발생: `tabSwitched(int index, QString url, QString title)`
  - 이미 다중 탭을 지원한다면 이 Task 생략
- **완료 기준**:
  - TabManager::previousTab(), nextTab() 호출 시 정상적으로 탭 전환
  - 첫 탭에서 previousTab() 호출 시 마지막 탭으로 순환 전환 확인
  - 마지막 탭에서 nextTab() 호출 시 첫 탭으로 순환 전환 확인
- **예상 소요 시간**: 2시간 (리팩토링 필요 시), 0.5시간 (이미 지원 시)

#### Task 2-2: handleChannelButton() 구현
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - `handleChannelButton(int keyCode)` 구현
  - 탭 개수가 1개 이하이면 Logger::debug 로그 후 리턴
  - keyCode가 KeyCode::CHANNEL_UP이면 `tabManager_->previousTab()` 호출
  - keyCode가 KeyCode::CHANNEL_DOWN이면 `tabManager_->nextTab()` 호출
  - 탭 전환 성공 시 Logger::info 로그
  - 탭 전환 실패 시 Logger::warning 로그
- **완료 기준**:
  - 채널 버튼 입력 시 탭이 정상적으로 전환됨
  - 단위 테스트 통과: 탭 3개 생성 후 채널 버튼으로 순환 전환
- **예상 소요 시간**: 1시간

#### Task 2-3: keyPressEvent() 기본 구조 구현 (채널 버튼 처리)
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - `keyPressEvent(QKeyEvent *event)` 오버라이드 구현
  - 설계서 5.1장 구조 참조
  - 단계:
    1. keyCode 추출 (`event->key()`)
    2. Logger::debug로 키 입력 로그 기록
    3. URLBar 포커스 체크 (`urlBar_->hasFocus()`) — 채널 버튼은 무시
    4. 패널 열림 체크 (추후 Phase 3에서 구현)
    5. 디바운스 체크 (`shouldDebounce()`)
    6. 채널 버튼 처리 (`KeyCode::CHANNEL_UP`, `CHANNEL_DOWN`)
    7. 기타 키는 `QMainWindow::keyPressEvent(event)` 호출
- **완료 기준**:
  - 채널 버튼 입력 시 handleChannelButton() 호출 확인
  - URLBar 포커스 시 채널 버튼 무시 확인
  - 디바운스 동작 확인 (0.5초 이내 중복 입력 무시)
- **예상 소요 시간**: 1.5시간

#### Task 2-4: TabManager 시그널 → BrowserWindow 슬롯 연결
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - BrowserWindow 생성자 또는 setupConnections() 메서드에서 시그널 연결
  - TabManager::tabSwitched 시그널 → 람다 슬롯 연결
  - 슬롯 내용:
    - URLBar에 새 탭의 URL 표시 (`urlBar_->setText(url)`)
    - NavigationBar에 새 탭의 제목 표시 (`navigationBar_->updateTitle(title)`)
    - WebView에 포커스 (`webView_->setFocus()`)
    - Logger::info 로그 기록
- **완료 기준**:
  - 탭 전환 시 URLBar, NavigationBar가 자동 업데이트됨
  - WebView에 포커스가 자동으로 이동함
- **예상 소요 시간**: 0.5시간

---

### Phase 3: 컬러 버튼 주요 기능 접근 (Must, FR-2)
**목표**: 리모컨 컬러 버튼(Red/Green/Yellow/Blue)으로 패널 열기 및 새 탭 생성

**담당**: cpp-dev

**의존성**: Phase 2 완료, F-07 (북마크), F-08 (히스토리), F-12 (다운로드) 완료

#### Task 3-1: handleColorButton() 구현
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - `handleColorButton(int keyCode)` 구현
  - switch 문으로 keyCode 분기:
    - `KeyCode::RED`: 북마크 패널 열기
      - `bookmarkPanel_->isVisible()` 체크, 이미 열려있으면 무시
      - `bookmarkPanel_->show()` 호출
      - `bookmarkPanel_->setFocus()` 호출
      - Logger::info 로그
    - `KeyCode::GREEN`: 히스토리 패널 열기
      - 동일한 로직 (`historyPanel_`)
    - `KeyCode::YELLOW`: 다운로드 패널 열기
      - 동일한 로직 (`downloadPanel_`)
    - `KeyCode::BLUE`: 새 탭 생성
      - `tabManager_->getTabCount() >= 5` 체크 (최대 탭 수)
      - 최대 초과 시 Logger::warning 로그 후 리턴
      - `tabManager_->createTab()` 호출
      - 성공 시 Logger::info 로그
- **완료 기준**:
  - Red 버튼으로 북마크 패널 열림
  - Green 버튼으로 히스토리 패널 열림
  - Yellow 버튼으로 다운로드 패널 열림
  - Blue 버튼으로 새 탭 생성 (최대 5개 제한)
  - 이미 열려있는 패널은 중복 열기 방지
- **예상 소요 시간**: 1.5시간

#### Task 3-2: keyPressEvent()에 컬러 버튼 처리 추가
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - keyPressEvent()에 컬러 버튼 처리 로직 추가
  - 채널 버튼 처리 후 컬러 버튼 체크 추가:
    ```cpp
    else if (keyCode == KeyCode::RED || keyCode == KeyCode::GREEN ||
             keyCode == KeyCode::YELLOW || keyCode == KeyCode::BLUE) {
        handleColorButton(keyCode);
        event->accept();
    }
    ```
  - URLBar 포커스 시에도 컬러 버튼은 처리 (패널 열기 허용)
- **완료 기준**:
  - 컬러 버튼 입력 시 handleColorButton() 호출 확인
  - URLBar 포커스 시에도 컬러 버튼 동작 확인
- **예상 소요 시간**: 0.5시간

#### Task 3-3: 패널 포커스 우선순위 처리 (Back 키 처리)
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - keyPressEvent() 상단에 패널 열림 체크 로직 추가
  - 패널이 열려있고 포커스를 가지고 있을 때:
    - Back 키만 BrowserWindow에서 처리 (패널 닫기)
    - 나머지 키는 `event->ignore()` 호출하여 패널로 전달
  - Back 키 처리:
    - `bookmarkPanel_->isVisible()` 이면 `bookmarkPanel_->hide()` 호출
    - 동일하게 historyPanel_, downloadPanel_, settingsPanel_ 처리
    - 패널 닫은 후 `webView_->setFocus()` 호출
- **완료 기준**:
  - 패널 열려있을 때 Back 키로 패널 닫힘
  - 패널 닫은 후 WebView에 포커스 이동 확인
  - 패널 내부에서 방향키는 패널에서 처리됨 (BrowserWindow에서 가로채지 않음)
- **예상 소요 시간**: 1시간

---

### Phase 4: 숫자 버튼 직접 탭 선택 (Should, FR-3)
**목표**: 리모컨 숫자 버튼(1~5)으로 특정 탭에 즉시 이동

**담당**: cpp-dev

**의존성**: Phase 3 완료

#### Task 4-1: handleNumberButton() 구현
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - `handleNumberButton(int keyCode)` 구현
  - 탭 인덱스 계산: `int tabIndex = keyCode - KeyCode::NUM_1` (0~4)
  - 탭 존재 여부 체크: `tabIndex >= tabManager_->getTabCount()`이면 무시
  - 이미 활성화된 탭 체크: `tabIndex == tabManager_->getCurrentTabIndex()`이면 무시
  - `tabManager_->switchToTab(tabIndex)` 호출
  - 성공 시 Logger::info 로그
  - 실패 시 Logger::warning 로그
- **완료 기준**:
  - 숫자 1~5 버튼으로 해당 탭에 즉시 이동
  - 존재하지 않는 탭 번호 입력 시 무시 + 로그
  - 이미 활성화된 탭 번호 입력 시 무시 + 로그
- **예상 소요 시간**: 1시간

#### Task 4-2: keyPressEvent()에 숫자 버튼 처리 추가
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - keyPressEvent()에 숫자 버튼 처리 로직 추가
  - URLBar 포커스 체크에서 숫자 버튼 무시 로직 추가:
    ```cpp
    if (urlBar_->hasFocus()) {
        if (keyCode >= KeyCode::NUM_1 && keyCode <= KeyCode::NUM_5) {
            event->ignore();  // URLBar로 전달
            return;
        }
    }
    ```
  - 컬러 버튼 처리 후 숫자 버튼 체크 추가:
    ```cpp
    else if (keyCode >= KeyCode::NUM_1 && keyCode <= KeyCode::NUM_5) {
        handleNumberButton(keyCode);
        event->accept();
    }
    ```
- **완료 기준**:
  - 숫자 버튼 입력 시 handleNumberButton() 호출 확인
  - URLBar 포커스 시 숫자 버튼이 URLBar로 전달됨 (탭 전환 안 됨)
- **예상 소요 시간**: 0.5시간

---

### Phase 5: 설정 버튼 매핑 (Should, FR-4)
**목표**: 리모컨 Menu/Settings 버튼으로 설정 패널 열기

**담당**: cpp-dev

**의존성**: Phase 4 완료, F-11 (설정 화면) 완료

#### Task 5-1: handleMenuButton() 구현
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - `handleMenuButton(int keyCode)` 구현
  - `settingsPanel_->isVisible()` 체크, 이미 열려있으면 무시
  - `settingsPanel_->show()` 호출
  - `settingsPanel_->setFocus()` 호출
  - Logger::info 로그
- **완료 기준**:
  - Menu 버튼으로 설정 패널 열림
  - 이미 열려있으면 중복 열기 방지
  - 패널 열 때 자동 포커스
- **예상 소요 시간**: 0.5시간

#### Task 5-2: keyPressEvent()에 설정 버튼 처리 추가
- **파일**: `src/browser/BrowserWindow.cpp` (수정)
- **작업 내용**:
  - 숫자 버튼 처리 후 설정 버튼 체크 추가:
    ```cpp
    else if (keyCode == KeyCode::MENU || keyCode == KeyCode::SETTINGS) {
        handleMenuButton(keyCode);
        event->accept();
    }
    ```
- **완료 기준**:
  - Menu 버튼 입력 시 handleMenuButton() 호출 확인
- **예상 소요 시간**: 0.25시간

---

### Phase 6: 테스트 및 검증
**목표**: 단위 테스트 작성 및 실제 기기 테스트

**담당**: test-runner

**의존성**: Phase 5 완료

#### Task 6-1: 단위 테스트 작성
- **파일**: `tests/unit/BrowserWindowKeyEventTest.cpp` (신규)
- **작업 내용**:
  - Google Test 기반 단위 테스트
  - 테스트 케이스:
    - `TEST(BrowserWindow, ShouldDebounceReturnsTrueWithinDebouncePeriod)`: 디바운스 동작 확인
    - `TEST(BrowserWindow, ChannelButtonSwitchesTab)`: 채널 버튼 탭 전환 확인
    - `TEST(BrowserWindow, ColorButtonOpensPanel)`: 컬러 버튼 패널 열기 확인
    - `TEST(BrowserWindow, NumberButtonSwitchesToTabByIndex)`: 숫자 버튼 탭 선택 확인
    - `TEST(BrowserWindow, NumberButtonIgnoredIfURLBarHasFocus)`: URLBar 포커스 시 숫자 버튼 무시 확인
    - `TEST(BrowserWindow, BlueButtonCreatesNewTab)`: Blue 버튼 새 탭 생성 확인
    - `TEST(BrowserWindow, BlueButtonIgnoresMaxTabLimit)`: 최대 탭 수 제한 확인
  - Mock 객체 사용: QKeyEvent 모킹
- **완료 기준**:
  - 모든 테스트 케이스 통과 (PASS)
  - 테스트 커버리지 80% 이상
- **예상 소요 시간**: 3시간

#### Task 6-2: 통합 테스트 작성
- **파일**: `tests/integration/RemoteShortcutsIntegrationTest.cpp` (신규)
- **작업 내용**:
  - Qt Test 기반 통합 테스트
  - 시나리오 테스트:
    - 탭 3개 생성 → Channel Down 연속 입력 → 순환 전환 확인
    - Red 버튼 → 북마크 패널 열림 → Back 키 → 패널 닫힘 확인
    - URLBar 포커스 → 숫자 3 입력 → URLBar에 "3" 입력됨 확인
    - Blue 버튼 5회 연속 입력 → 최대 5개 탭 생성, 6번째는 무시 확인
  - 실제 위젯 인스턴스 생성 (Mock 없이)
- **완료 기준**:
  - 모든 시나리오 통과
- **예상 소요 시간**: 2시간

#### Task 6-3: 실제 기기 테스트 (LG 프로젝터 HU715QW)
- **담당**: test-runner
- **작업 내용**:
  - webOS 프로젝터에 IPK 설치 후 테스트
  - 모든 리모컨 버튼의 keyCode 로그 확인 및 KeyCodeConstants.h 업데이트
  - 채널 버튼, 컬러 버튼, 숫자 버튼, 설정 버튼 동작 확인
  - 탭 전환 시 WebView 렌더링 지연 시간 측정 (0.5초 이내 목표)
  - 패널 슬라이드 인 애니메이션 부드러움 확인 (60fps 목표)
  - 디바운스 0.5초 동작 확인 (빠른 연속 입력 무시)
  - Yellow/Green 버튼 keyCode 중복 확인 (405로 중복될 수 있음)
- **완료 기준**:
  - 모든 리모컨 단축키가 정상 동작
  - keyCode 불일치 발견 시 KeyCodeConstants.h 수정
  - 성능 목표 달성 (탭 전환 0.5초 이내, 애니메이션 60fps)
- **예상 소요 시간**: 2시간

---

### Phase 7: 코드 및 문서 리뷰
**목표**: 코드 품질 및 문서 완성도 검증

**담당**: code-reviewer

**의존성**: Phase 6 완료

#### Task 7-1: 코드 리뷰
- **작업 내용**:
  - BrowserWindow.cpp 코드 리뷰
    - keyPressEvent() 로직 명확성 검증
    - 디바운스 메커니즘 정확성 검증
    - 메모리 누수 확인 (QMap 사용, QTimer 정리)
    - 포인터 안정성 확인 (널 체크)
  - TabManager.cpp 리팩토링 검증 (다중 탭 지원)
  - KeyCodeConstants.h 상수 값 정확성 검증
  - 코딩 컨벤션 준수 확인 (CLAUDE.md 규칙)
  - Doxygen 주석 완성도 확인
- **완료 기준**:
  - 코드 리뷰 피드백 반영 완료
  - 모든 컴파일 경고 제거
- **예상 소요 시간**: 2시간

#### Task 7-2: 문서 리뷰
- **작업 내용**:
  - requirements.md, design.md, plan.md 일관성 검증
  - 코드와 설계서 일치 여부 확인
  - 테스트 커버리지가 AC(Acceptance Criteria)를 만족하는지 확인
- **완료 기준**:
  - 문서 불일치 발견 시 수정
- **예상 소요 시간**: 1시간

---

## 3. 태스크 의존성

```
Phase 1 (기반 구조)
  ├─ Task 1-1 (KeyCodeConstants.h)
  ├─ Task 1-2 (BrowserWindow.h 확장)
  └─ Task 1-3 (shouldDebounce 구현)
          ↓
Phase 2 (채널 버튼 탭 전환)
  ├─ Task 2-1 (TabManager 리팩토링) ← F-06 의존
  ├─ Task 2-2 (handleChannelButton)
  ├─ Task 2-3 (keyPressEvent 기본 구조)
  └─ Task 2-4 (TabManager 시그널 연결)
          ↓
Phase 3 (컬러 버튼 주요 기능)
  ├─ Task 3-1 (handleColorButton) ← F-07, F-08, F-12 의존
  ├─ Task 3-2 (keyPressEvent 컬러 버튼)
  └─ Task 3-3 (패널 포커스 우선순위)
          ↓
Phase 4 (숫자 버튼 탭 선택)
  ├─ Task 4-1 (handleNumberButton)
  └─ Task 4-2 (keyPressEvent 숫자 버튼)
          ↓
Phase 5 (설정 버튼)
  ├─ Task 5-1 (handleMenuButton) ← F-11 의존
  └─ Task 5-2 (keyPressEvent 설정 버튼)
          ↓
Phase 6 (테스트)
  ├─ Task 6-1 (단위 테스트)
  ├─ Task 6-2 (통합 테스트)
  └─ Task 6-3 (실제 기기 테스트)
          ↓
Phase 7 (리뷰)
  ├─ Task 7-1 (코드 리뷰)
  └─ Task 7-2 (문서 리뷰)
```

---

## 4. 병렬 실행 판단

### 4.1 PG-3 병렬 배치 컨텍스트

features.md에 따르면 F-13은 PG-3 병렬 그룹에 속합니다:
- **F-12 (다운로드 관리)**: DownloadManager 서비스 + DownloadPanel 컴포넌트
- **F-13 (리모컨 단축키)**: KeyboardHandler 서비스
- **F-14 (HTTPS 보안 표시)**: SecurityIndicator 컴포넌트 + URLBar 확장

### 4.2 충돌 영역 분석

#### F-13 vs F-12 충돌 분석

| 영역 | F-13 접근 | F-12 접근 | 충돌 여부 |
|------|-----------|-----------|-----------|
| **BrowserWindow.h/cpp** | keyPressEvent() 오버라이드, 멤버 변수 추가 | downloadPanel_ 포인터 추가 | ⚠️ **충돌 위험 (낮음)** |
| **BrowserWindow 멤버** | lastKeyPressTime_, autoScrollTimer_ 추가 | downloadPanel_ 추가 | ✅ **충돌 없음** |
| **DownloadPanel** | Yellow 버튼으로 패널 열기 (show() 호출) | 패널 내부 구현 (UI, 다운로드 로직) | ✅ **충돌 없음** |
| **TabManager** | 다중 탭 지원 리팩토링 (previousTab, nextTab 추가) | 사용하지 않음 | ✅ **충돌 없음** |
| **URLBar** | 포커스 체크 (hasFocus()) | 사용하지 않음 | ✅ **충돌 없음** |

**결론**: BrowserWindow.h/cpp 파일에서 충돌 가능성이 있으나, 추가하는 멤버 변수와 메서드가 완전히 독립적이므로 **병렬 실행 가능**. 단, 최종 병합 시 BrowserWindow.h 충돌 해결 필요.

#### F-13 vs F-14 충돌 분석

| 영역 | F-13 접근 | F-14 접근 | 충돌 여부 |
|------|-----------|-----------|-----------|
| **BrowserWindow.h/cpp** | keyPressEvent() 오버라이드 추가 | securityIndicator_ 포인터 추가 | ⚠️ **충돌 위험 (낮음)** |
| **URLBar** | 포커스 체크 (hasFocus()), setText() 호출 | 보안 아이콘 표시 (UI 확장) | ⚠️ **충돌 위험 (중간)** |
| **WebView** | 탭 전환 시 load() 호출, 포커스 관리 | URL 변경 시그널 리스닝 (HTTPS 체크) | ✅ **충돌 없음** |

**결론**: URLBar 컴포넌트를 F-14가 UI 확장하고 F-13은 포커스 체크만 수행하므로 **충돌 가능성 중간**. URLBar.h/cpp 파일 동시 수정 시 병합 충돌 발생 가능. **병렬 실행 시 주의 필요**, 순차 실행 권장.

### 4.3 병렬 실행 최종 판단

#### 가능한 병렬 조합

**Option 1: F-13 단독 실행 (권장)**
- 이유: F-13이 BrowserWindow와 TabManager 리팩토링을 수반하므로, 다른 기능과 독립적으로 진행하는 것이 안전
- 장점: 충돌 리스크 최소화, TabManager 리팩토링 안정성 확보
- 단점: 병렬화 이점 없음

**Option 2: F-12 + F-13 병렬 실행 (조건부 가능)**
- 조건: F-13의 Task 2-1 (TabManager 리팩토링) 완료 후 병렬 시작
- 병렬 가능 Phase:
  - F-13 Phase 3~5 (컬러/숫자/설정 버튼) ∥ F-12 Phase 1~3 (서비스+UI)
- 충돌 영역: BrowserWindow.h (멤버 변수 추가만, 충돌 해결 간단)
- 장점: 개발 기간 단축 (약 2일)
- 단점: BrowserWindow.h 병합 시 충돌 해결 필요

**Option 3: F-13 + F-14 병렬 실행 (비권장)**
- 이유: URLBar 충돌 위험 중간, 병합 복잡도 증가
- 권장: F-13 → F-14 순차 실행

#### Agent Team 사용 권장 여부

**권장**: **No** (순차 실행 권장)

**근거**:
1. **TabManager 리팩토링 필요**: F-13은 TabManager를 단일 탭 → 다중 탭 구조로 리팩토링해야 하므로, 이 작업이 F-06의 완성도에 영향을 미칠 수 있음. 독립적으로 안정화 필요.
2. **BrowserWindow 핵심 변경**: keyPressEvent() 오버라이드는 BrowserWindow의 이벤트 처리 흐름을 변경하는 핵심 작업이므로, 다른 기능과 동시에 수정 시 디버깅 복잡도 증가.
3. **실제 기기 테스트 의존성**: F-13은 실제 webOS 프로젝터에서 리모컨 keyCode 검증이 필수이므로, 단독 테스트 후 안정화하는 것이 효율적.

**대안**:
- F-13을 먼저 완료한 후, F-12와 F-14를 병렬 실행하는 것이 더 안전 (F-12 ∥ F-14, URLBar 충돌만 해결)

---

## 5. 리스크 및 대응 방안

### 리스크 1: webOS 리모컨 keyCode 불일치

**영향도**: 높음

**시나리오**:
- KeyCodeConstants.h에 정의된 키 코드가 실제 LG 프로젝터 HU715QW의 keyCode와 다를 수 있음
- Yellow와 Green 버튼이 keyCode 405로 중복될 가능성

**대응 방안**:
1. Phase 1 완료 후 즉시 실제 기기에서 keyCode 로그 수집
2. 모든 리모컨 버튼의 QKeyEvent::key() 값을 Logger::debug로 출력
3. 불일치 발견 시 KeyCodeConstants.h 업데이트 후 재빌드
4. Yellow/Green 중복 시 대체 키 매핑 고려 (예: Yellow → 숫자 0 버튼)

### 리스크 2: TabManager 리팩토링 범위 초과

**영향도**: 높음

**시나리오**:
- 현재 TabManager가 단일 탭만 지원할 경우, 다중 탭 리팩토링 범위가 예상보다 클 수 있음
- F-06 (탭 관리 시스템)이 이미 완료되었다고 가정했으나, 실제로는 미완성일 수 있음

**대응 방안**:
1. Task 2-1 시작 전 TabManager 코드 리뷰 및 현황 파악
2. 만약 리팩토링 범위가 크다면 F-13을 F-06 리팩토링으로 분리 후 재계획
3. 예상 소요 시간을 2시간 → 4시간으로 증가 가능성 고려

### 리스크 3: Qt 이벤트 전파 순서 문제

**영향도**: 중간

**시나리오**:
- URLBar나 패널이 포커스를 가지고 있을 때, BrowserWindow::keyPressEvent()가 호출되지 않을 수 있음
- Qt의 이벤트 전파는 자식 → 부모 순이므로, 자식이 event->accept()를 호출하면 부모로 전달되지 않음

**대응 방안**:
1. BrowserWindow에서 installEventFilter()를 사용하여 모든 자식 위젯의 키 이벤트 가로채기 검토
2. 대안: URLBar, BookmarkPanel 등 자식 위젯에서 처리하지 않은 키는 명시적으로 event->ignore() 호출하도록 수정
3. 통합 테스트로 포커스 우선순위 동작 검증

### 리스크 4: 자동 스크롤 성능 문제 (M3 이후)

**영향도**: 낮음 (M3 이후 구현)

**시나리오**:
- 10ms 간격으로 JavaScript 실행 시 CPU 부하 발생 가능
- 일부 웹 페이지에서 스크롤이 매끄럽지 않을 수 있음

**대응 방안**:
1. 초기 구현은 50ms 간격 + 20px step으로 시작 (부하 감소)
2. 사용자 설정에서 스크롤 속도 조절 기능 추가 (M4 이후)
3. QPropertyAnimation 기반 스크롤 대안 검토

### 리스크 5: 최대 탭 수 제한 UX

**영향도**: 낮음

**시나리오**:
- 사용자가 Blue 버튼으로 새 탭을 생성하려 할 때 이미 5개 탭이 있으면 무시됨
- 시각적 피드백 없이 무시되면 사용자가 혼란스러울 수 있음

**대응 방안**:
1. Logger::warning으로 로그 기록 (디버깅용)
2. 선택적: QMessageBox 또는 토스트 메시지로 "최대 5개 탭까지 생성 가능합니다" 표시
3. M3 이후 구현 검토 (Must 항목은 아님)

---

## 6. 예상 산출물

### 신규 파일
- `src/utils/KeyCodeConstants.h` (100줄)

### 수정 파일
- `src/browser/BrowserWindow.h` (+50줄)
- `src/browser/BrowserWindow.cpp` (+300줄)
- `src/browser/TabManager.h` (+40줄, 리팩토링 필요 시)
- `src/browser/TabManager.cpp` (+150줄, 리팩토링 필요 시)

### 테스트 파일
- `tests/unit/BrowserWindowKeyEventTest.cpp` (신규, 300줄)
- `tests/integration/RemoteShortcutsIntegrationTest.cpp` (신규, 200줄)

---

## 7. 예상 소요 시간

| Phase | 예상 소요 시간 |
|-------|---------------|
| Phase 1: 기반 구조 | 1.5시간 |
| Phase 2: 채널 버튼 탭 전환 | 5시간 (리팩토링 포함 시 7시간) |
| Phase 3: 컬러 버튼 주요 기능 | 3시간 |
| Phase 4: 숫자 버튼 탭 선택 | 1.5시간 |
| Phase 5: 설정 버튼 | 0.75시간 |
| Phase 6: 테스트 | 7시간 |
| Phase 7: 리뷰 | 3시간 |
| **총 예상 시간** | **21.75시간 (약 3일)** |

**버퍼 포함 (리팩토링 범위 증가, 실제 기기 테스트 지연)**: **4일**

---

## 8. 완료 기준

### 기능 완료 기준 (Acceptance Criteria 기반)

- [ ] **AC-1: 채널 버튼 탭 전환**
  - [ ] Channel Up 버튼으로 이전 탭 전환
  - [ ] Channel Down 버튼으로 다음 탭 전환
  - [ ] 탭 전환 시 WebView 포커스, URLBar URL 업데이트
  - [ ] 순환 전환 동작 (첫 ↔ 마지막 탭)
  - [ ] 탭 1개일 때 무시
  - [ ] 디바운스 0.5초 동작

- [ ] **AC-2: 컬러 버튼 기능 접근**
  - [ ] Red 버튼으로 북마크 패널 열기
  - [ ] Green 버튼으로 히스토리 패널 열기
  - [ ] Yellow 버튼으로 다운로드 패널 열기
  - [ ] Blue 버튼으로 새 탭 생성 (최대 5개 제한)
  - [ ] 중복 열기 방지

- [ ] **AC-3: 숫자 버튼 직접 탭 선택**
  - [ ] 1~5 버튼으로 해당 탭 이동
  - [ ] 존재하지 않는 탭 무시
  - [ ] URLBar 포커스 시 숫자 키 무시

- [ ] **AC-4: 설정 버튼 접근**
  - [ ] Menu 버튼으로 설정 패널 열기

- [ ] **AC-5: 반응 속도 및 키 충돌 방지**
  - [ ] 0.3초 이내 시각적 피드백
  - [ ] URLBar 포커스 시 충돌 방지
  - [ ] 패널 내부 키 이벤트 우선 처리

- [ ] **AC-6: 로깅 및 디버깅**
  - [ ] 모든 키 입력 로그 기록
  - [ ] 키 매핑 실패 시 경고 로그

### 품질 완료 기준

- [ ] 단위 테스트 커버리지 80% 이상
- [ ] 통합 테스트 모든 시나리오 통과
- [ ] 실제 기기 테스트 통과 (LG 프로젝터 HU715QW)
- [ ] 코드 리뷰 승인
- [ ] 문서 일관성 검증 완료
- [ ] 컴파일 경고 0개
- [ ] 메모리 누수 없음 (Valgrind 검증)

---

## 9. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초안 작성 | product-manager |
| 2026-02-14 | PG-3 병렬 배치 충돌 분석 추가 | product-manager |
| 2026-02-14 | Agent Team 비권장 결정 (순차 실행 권장) | product-manager |
| 2026-02-14 | 리스크 5가지 추가 및 대응 방안 상세화 | product-manager |
