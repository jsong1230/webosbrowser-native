# 페이지 탐색 컨트롤 — 요구사항 분석서 (Native App)

## 1. 개요

### 기능명
F-04: 페이지 탐색 컨트롤 (NavigationBar)

### 목적
Qt Widgets 기반의 브라우저 네비게이션 UI를 제공하여, webOS 프로젝터 환경에서 리모컨으로 웹 페이지를 직관적으로 탐색할 수 있도록 합니다. WebView의 히스토리 API와 연동하여 뒤로/앞으로/새로고침/홈 기능을 구현합니다.

### 대상 사용자
- webOS Browser Native App의 모든 사용자
- 리모컨으로 웹 페이지를 탐색하는 일반 사용자
- 여러 웹 페이지를 오가며 정보를 찾는 사용자
- 영상 스트리밍 플랫폼 내에서 콘텐츠 간 이동하는 사용자

### 요청 배경
- 웹 브라우저의 핵심 네비게이션 기능(뒤로/앞으로)은 브라우저 사용성의 기본 요소입니다.
- F-02(웹뷰 통합)에서 Qt WebEngineView 기반 WebView를 구현했으므로, 이를 제어할 Native 탐색 UI가 필요합니다.
- 리모컨 환경에서는 마우스/키보드 UI가 없으므로 명시적인 버튼 위젯이 필수입니다.
- PRD에서 "리모컨 최적화 UX"가 핵심 가치로 정의되었으며, 탐색 버튼은 Qt의 키 이벤트 시스템으로 리모컨 입력을 처리해야 합니다.

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: 뒤로 가기 버튼
- **설명**: WebView의 히스토리 스택에서 이전 페이지로 이동하는 버튼
- **유저 스토리**:
  - As a 웹 브라우저 사용자, I want 뒤로 가기 버튼을 클릭하여 이전에 본 페이지로 돌아가고 싶다, so that 잘못 클릭한 링크나 이전 내용을 다시 확인할 수 있다.
- **상세 요구사항**:
  - 버튼 클릭 시 `WebView::goBack()` 메서드 호출
  - `WebView::canGoBack() == false`일 때 버튼 비활성화 (`setEnabled(false)`)
  - 비활성화된 버튼은 시각적으로 구분 (Qt 기본 disabled 스타일 또는 커스텀 스타일시트)
  - 리모컨 포커스를 받을 수 있으나 클릭 시 동작하지 않음
  - 아이콘: `QIcon` 또는 유니코드 화살표 (← 또는 U+2190)
  - 위젯 타입: `QPushButton` 또는 `QToolButton`
- **우선순위**: Must

### FR-2: 앞으로 가기 버튼
- **설명**: 뒤로 가기 이후 다시 앞 페이지로 이동하는 버튼
- **유저 스토리**:
  - As a 웹 브라우저 사용자, I want 뒤로 간 후 다시 앞으로 가기 버튼을 클릭하여 최근 페이지로 돌아가고 싶다, so that 탐색 중 실수로 뒤로 간 경우 빠르게 복구할 수 있다.
- **상세 요구사항**:
  - 버튼 클릭 시 `WebView::goForward()` 메서드 호출
  - `WebView::canGoForward() == false`일 때 버튼 비활성화
  - 비활성화된 버튼은 시각적으로 구분 (FR-1과 동일한 스타일)
  - 아이콘: 오른쪽 화살표 (→ 또는 U+2192)
  - 위젯 타입: `QPushButton` 또는 `QToolButton`
- **우선순위**: Must

### FR-3: 새로고침 버튼
- **설명**: 현재 페이지를 다시 로드하는 버튼
- **유저 스토리**:
  - As a 웹 브라우저 사용자, I want 새로고침 버튼을 클릭하여 페이지를 다시 불러오고 싶다, so that 콘텐츠가 업데이트되거나 로딩 실패 시 재시도할 수 있다.
- **상세 요구사항**:
  - 버튼 클릭 시 `WebView::reload()` 메서드 호출
  - 버튼은 항상 활성화 상태 (비활성화되지 않음)
  - 로딩 중에도 클릭 가능 (로딩 중단 후 재로드)
  - 아이콘: 새로고침 아이콘 (🔄 또는 U+21BB 원형 화살표)
  - 위젯 타입: `QPushButton` 또는 `QToolButton`
- **우선순위**: Must

### FR-4: 홈 버튼
- **설명**: 사용자가 설정한 홈페이지 URL로 이동하는 버튼
- **유저 스토리**:
  - As a 웹 브라우저 사용자, I want 홈 버튼을 클릭하여 자주 사용하는 시작 페이지로 바로 이동하고 싶다, so that 깊은 탐색 후 빠르게 시작점으로 돌아갈 수 있다.
- **상세 요구사항**:
  - 버튼 클릭 시 설정된 홈페이지 URL로 `WebView::load(homePageUrl)` 호출
  - 기본 홈페이지 URL: `https://www.google.com` (설정 없을 경우)
  - 홈페이지 설정은 F-11(설정 화면)에서 변경 가능 (현재는 하드코딩 또는 SettingsService에서 조회)
  - 버튼은 항상 활성화 상태
  - 아이콘: 집 아이콘 (🏠 또는 U+2302 ⌂)
  - 위젯 타입: `QPushButton` 또는 `QToolButton`
- **우선순위**: Must

### FR-5: 리모컨 포커스 네비게이션
- **설명**: Qt의 포커스 시스템으로 리모컨 방향키로 버튼 간 포커스 이동이 자연스럽게 동작
- **유저 스토리**:
  - As a 웹 브라우저 사용자, I want 리모컨 좌/우 방향키로 탐색 버튼 간 이동하고 싶다, so that 마우스 없이도 원하는 버튼을 선택할 수 있다.
- **상세 요구사항**:
  - NavigationBar의 버튼들은 Qt Focus Policy 설정 (`setFocusPolicy(Qt::StrongFocus)`)
  - 버튼 순서: 뒤로 → 앞으로 → 새로고침 → 홈 (좌→우)
  - `QWidget::setTabOrder()` 메서드로 포커스 순서 명시적 설정
  - 리모컨 좌/우 방향키 (Qt::Key_Left, Qt::Key_Right)로 버튼 간 포커스 이동
  - 리모컨 위 방향키 (Qt::Key_Up)로 WebView 또는 URLBar로 포커스 이동
  - 포커스된 버튼은 Qt 기본 포커스 스타일 또는 커스텀 스타일시트로 강조 (테두리, 배경색 변경)
  - 리모컨 선택 버튼 (Qt::Key_Enter 또는 Qt::Key_Select)으로 버튼 활성화
- **우선순위**: Must

### FR-6: 버튼 상태 동기화
- **설명**: WebView의 히스토리 상태 변경 시 버튼 활성/비활성 상태가 자동으로 업데이트
- **유저 스토리**:
  - As a 웹 브라우저 사용자, I want 뒤로 갈 수 없을 때 뒤로 버튼이 비활성화되는 것을 보고 싶다, so that 클릭해도 동작하지 않을 버튼을 미리 알 수 있다.
- **상세 요구사항**:
  - WebView의 `urlChanged(const QUrl&)` 시그널에 연결된 슬롯에서 버튼 상태 업데이트
  - `WebView::canGoBack()`, `WebView::canGoForward()` 메서드 호출 결과로 버튼 활성 여부 결정
  - `QPushButton::setEnabled(bool)` 메서드로 버튼 상태 변경
  - 상태 변경 시 UI 즉시 업데이트 (Qt 이벤트 루프에서 자동 처리)
- **우선순위**: Must
- **기술적 참고**: Qt WebEngineView는 히스토리 상태를 자동으로 관리하며, `canGoBack()/canGoForward()` API를 제공합니다.

### FR-7: 리모컨 Back 키 매핑
- **설명**: 리모컨의 물리적 Back 버튼을 누를 때 브라우저 뒤로 가기 동작
- **유저 스토리**:
  - As a 웹 브라우저 사용자, I want 리모컨의 뒤로 버튼을 눌러 페이지를 뒤로 가고 싶다, so that UI 버튼을 찾지 않고도 빠르게 뒤로 갈 수 있다.
- **상세 요구사항**:
  - BrowserWindow 레벨에서 `keyPressEvent(QKeyEvent*)` 오버라이드하여 키 이벤트 캡처
  - KeyCode 확인 (`Qt::Key_Backspace` 또는 webOS 전용 Back 키 코드)
  - WebView에 포커스가 있을 때: `WebView::goBack()` 호출 (FR-1과 동일)
  - WebView에 포커스가 없을 때 (URLBar, NavigationBar): Qt 기본 포커스 이탈 동작 (event->ignore())
  - 히스토리가 비어있을 때 (`canGoBack() == false`): 이벤트 무시 또는 앱 종료 준비 (F-15 구현 시 연동)
- **우선순위**: Must

## 3. 비기능 요구사항 (Non-Functional Requirements)

### NFR-1: 성능
- **버튼 응답 시간**: 버튼 클릭 후 0.3초 이내에 페이지 네비게이션 시작 (PRD 기준)
- **히스토리 상태 업데이트**: WebView URL 변경 시그널 수신 후 0.1초 이내에 버튼 상태 업데이트 (Qt 시그널/슬롯은 동기 호출이므로 즉시 처리)
- **메모리 사용**: NavigationBar 위젯은 50KB 이하의 메모리만 사용 (4개 버튼 + 레이아웃)

### NFR-2: UX (리모컨 최적화)
- **버튼 크기**: 최소 100px x 80px (3m 거리에서 인식 가능, PRD 기준)
- **버튼 간격**: 최소 20px (리모컨 포커스 이동 시 명확한 시각적 구분)
- **포커스 표시**: 포커스된 버튼은 2px 두께의 흰색 테두리 또는 배경색 변경 (Qt 스타일시트 또는 Focus Rectangle)
- **아이콘 크기**: 최소 32px x 32px (명확한 인식, `QIcon::pixmap(QSize(32, 32))`)
- **폰트 크기**: 버튼 라벨은 최소 16pt (PRD 기준, 선택 사항 - 아이콘만 사용 시 불필요)
- **색상 대비**: WCAG 2.1 AA 기준 (4.5:1 이상) 준수 (Qt 스타일시트에서 설정)

### NFR-3: 접근성
- **비활성화된 버튼 표시**: 비활성 버튼은 Qt 기본 disabled 스타일 또는 opacity: 0.5 스타일시트
- **시각적 피드백**: 버튼 클릭 시 눌림 효과 (QPushButton 기본 pressed 상태 또는 커스텀 애니메이션)
- **키보드 네비게이션**: 리모컨 방향키로 모든 버튼 접근 가능 (Qt Focus Chain)
- **스크린 리더 지원**: `QWidget::setAccessibleName()`, `setAccessibleDescription()` 설정 (선택 사항, webOS 지원 제한)

### NFR-4: 신뢰성
- **에러 처리**: WebView API 호출 실패 시 조용히 실패 (에러 메시지 없음, qDebug() 로그만)
- **상태 복구**: WebView 객체 소멸 또는 무효 시 버튼은 비활성화 상태로 설정
- **Null 포인터 체크**: `WebView*` 포인터 접근 전 항상 `nullptr` 체크

### NFR-5: 호환성
- **webOS 버전**: webOS 4.x 이상 (LG 프로젝터 HU715QW)
- **Qt 버전**: Qt 5.15+ (webOS Native SDK 권장 버전)
- **Qt WebEngine**: Qt WebEngineView의 history API는 모든 Qt 5.x 버전에서 지원

## 4. 제약사항

### 기술적 제약
1. **Qt WebEngineView 히스토리 API**:
   - Qt WebEngineView는 `QWebEngineHistory` 객체를 통해 히스토리 관리
   - `canGoBack()`, `canGoForward()` 메서드는 동기 호출로 즉시 결과 반환
   - Web App의 Same-Origin Policy 제약은 Native App에서는 없음
   - 대응: `WebView::canGoBack()`, `WebView::canGoForward()` 메서드를 PIMPL 패턴으로 래핑

2. **webOS 리모컨 키 이벤트**:
   - webOS는 리모컨 Back 버튼을 `Qt::Key_Backspace`로 변환 (추가 조사 필요)
   - 일부 webOS 버전은 커스텀 키 코드 사용 가능
   - 대응: `BrowserWindow::keyPressEvent()`에서 복수 키 코드 처리 (Backspace, Escape 등)

3. **Qt Focus System**:
   - NavigationBar와 WebView 간 포커스 전환이 명확해야 함
   - `QWidget::setFocusPolicy()`, `setTabOrder()` 메서드로 포커스 순서 관리
   - 대응: BrowserWindow 레벨에서 전체 포커스 흐름 설계 (F-02 설계서 참조)

### 의존성
- **F-01 (프로젝트 초기 설정)**: Qt 프로젝트 구조, CMakeLists.txt 필요 (완료)
- **F-02 (웹뷰 통합)**: WebView 클래스가 구현되어 있어야 함 (완료)
- **F-02의 WebView API**: `goBack()`, `goForward()`, `reload()`, `canGoBack()`, `canGoForward()`, `urlChanged()` 시그널 필요 (구현 완료 확인됨)

### 플랫폼 제약
- **리모컨 입력**: 마우스/터치 입력 불가, 리모컨 전용
- **화면 크기**: 프로젝터 대화면 (1920x1080 이상) 기준 UI 설계
- **성능**: webOS 하드웨어 제약으로 복잡한 애니메이션 자제 (Qt Widgets 기본 애니메이션만 사용)

## 5. 용어 정의

| 용어 | 정의 |
|------|------|
| **NavigationBar** | Qt Widgets 기반 탐색 컨트롤 UI 컴포넌트 (뒤로/앞으로/새로고침/홈 버튼 포함) |
| **히스토리 스택** | Qt WebEngineView가 관리하는 방문 페이지 기록 (QWebEngineHistory) |
| **뒤로 가기** | `WebView::goBack()` 호출, 이전 페이지로 이동 |
| **앞으로 가기** | `WebView::goForward()` 호출, 뒤로 간 후 다시 앞 페이지로 이동 |
| **새로고침** | `WebView::reload()` 호출, 현재 페이지 재로드 |
| **홈페이지** | 사용자가 설정한 기본 시작 페이지 (기본값: Google) |
| **리모컨 Back 키** | webOS 리모컨의 물리적 뒤로 버튼 (Qt::Key_Backspace로 매핑) |
| **포커스 정책 (Focus Policy)** | Qt 위젯이 키보드 포커스를 받을 수 있는지 결정하는 설정 (Qt::StrongFocus 등) |
| **비활성화 (disabled)** | 버튼이 클릭 불가능하고 시각적으로 흐릿하게 표시되는 상태 (`setEnabled(false)`) |
| **PIMPL 패턴** | Pointer to IMPLementation, 구현 세부사항을 헤더에서 숨기는 C++ 디자인 패턴 |
| **Qt 시그널/슬롯** | Qt의 이벤트 기반 통신 메커니즘 (WebView → NavigationBar 통신에 사용) |

## 6. 범위 외 (Out of Scope)

### 이번 기능에서 하지 않는 것
1. **URL 표시 기능**: NavigationBar는 현재 URL을 표시하지 않음 (F-03 URLBar에서 구현)
2. **로딩 진행률 표시**: 새로고침 시 로딩 상태는 F-05(로딩 인디케이터)에서 처리
3. **히스토리 패널 UI**: 방문 기록 목록은 F-08(히스토리 관리)에서 구현
4. **북마크 추가 버튼**: 북마크 기능은 F-07(북마크 관리)에서 구현
5. **설정 버튼**: 설정 화면 진입은 F-11(설정 화면)에서 구현
6. **탭 전환 버튼**: 탭 관리는 F-06(탭 관리)에서 구현
7. **진행률 기반 새로고침 중단**: Qt WebEngineView는 진행률 API를 제공하지만 이번에는 단순 reload만 구현
8. **홈페이지 설정 UI**: F-11(설정 화면)에서 구현 예정, 현재는 하드코딩 또는 SettingsService 기본값

### 향후 확장 가능 기능
1. **버튼 커스터마이징**: 사용자가 버튼 순서, 표시 여부를 설정 (F-11 연동)
2. **제스처 지원**: 리모컨 스와이프로 뒤로/앞으로 (webOS 6.x 이상, Qt Gesture 프레임워크)
3. **음성 명령**: "뒤로 가기"라고 말하면 동작 (추후 AI 통합)
4. **버튼 애니메이션**: 클릭 시 버튼 회전/페이드 효과 (Qt Property Animation)

## 7. 완료 기준 (Acceptance Criteria)

### AC-1: 버튼 기본 동작
- [ ] 뒤로 가기 버튼 클릭 시 이전 페이지로 이동 (`WebView::goBack()` 호출)
- [ ] 앞으로 가기 버튼 클릭 시 다음 페이지로 이동 (`WebView::goForward()` 호출)
- [ ] 새로고침 버튼 클릭 시 현재 페이지 재로드 (`WebView::reload()` 호출)
- [ ] 홈 버튼 클릭 시 Google 홈페이지로 이동 (`WebView::load("https://www.google.com")`)

### AC-2: 버튼 상태 관리
- [ ] 첫 페이지에서 뒤로 가기 버튼 비활성화 (`canGoBack() == false` → `setEnabled(false)`)
- [ ] 뒤로 간 후에만 앞으로 가기 버튼 활성화 (`canGoForward() == true` → `setEnabled(true)`)
- [ ] 비활성화된 버튼은 흐릿하게 표시 (Qt 기본 disabled 스타일)
- [ ] 비활성화된 버튼 클릭 시 동작하지 않음 (Qt 자동 처리)

### AC-3: 리모컨 네비게이션
- [ ] 리모컨 좌/우 방향키로 버튼 간 포커스 이동 (Qt Focus Chain)
- [ ] 포커스된 버튼에 흰색 테두리 표시 (Qt 포커스 스타일 또는 커스텀 스타일시트)
- [ ] 리모컨 선택 버튼(Enter/Select)으로 버튼 활성화
- [ ] 리모컨 위 방향키로 WebView 또는 URLBar로 포커스 이동

### AC-4: 리모컨 Back 키 매핑
- [ ] WebView에 포커스 있을 때 리모컨 Back 키 → 브라우저 뒤로 (`goBack()` 호출)
- [ ] NavigationBar에 포커스 있을 때 리모컨 Back 키 → 포커스 이탈 (event->ignore())
- [ ] 첫 페이지에서 리모컨 Back 키 → 앱 동작 유지 (종료 안 함, `canGoBack() == false` 확인)

### AC-5: 성능
- [ ] 버튼 클릭 후 0.3초 이내에 페이지 네비게이션 시작 (Qt WebEngineView는 즉시 로드 시작)
- [ ] WebView URL 변경 시그널 수신 후 0.1초 이내에 버튼 상태 업데이트 (동기 호출)
- [ ] 연속 클릭 시에도 응답 지연 없음 (Qt 이벤트 루프 성능 의존)

### AC-6: UI 가이드 준수
- [ ] 버튼 크기 100px x 80px 이상 (`setMinimumSize(100, 80)`)
- [ ] 버튼 간격 20px 이상 (QHBoxLayout spacing 설정)
- [ ] 아이콘 크기 32px x 32px (`QIcon::pixmap(QSize(32, 32))`)
- [ ] 포커스 테두리 2px, 흰색 (Qt 스타일시트 `:focus` 선택자)
- [ ] 색상 대비 4.5:1 이상 (스타일시트에서 확인)

### AC-7: 에러 처리
- [ ] WebView 포인터 `nullptr` 시 버튼 비활성화 및 조용히 실패
- [ ] WebView API 호출 실패 시 qDebug() 로그만 출력
- [ ] 히스토리 상태 확인 실패 시 기본 비활성화 상태로 복구

### AC-8: 실제 디바이스 테스트
- [ ] LG 프로젝터 HU715QW에서 리모컨으로 모든 버튼 조작 가능
- [ ] 주요 사이트 (YouTube, Naver, Google)에서 뒤로/앞으로 정상 동작
- [ ] 3m 거리에서 버튼 아이콘 명확히 인식 가능
- [ ] 버튼 포커스 이동이 자연스럽고 직관적

## 8. 테스트 시나리오

### 시나리오 1: 정상 네비게이션 플로우
1. Google 홈페이지 로드 → 뒤로 버튼 비활성화 확인 (`canGoBack() == false`)
2. "YouTube" 검색 → 검색 결과 페이지 로드 → 뒤로 버튼 활성화 확인 (`canGoBack() == true`)
3. 뒤로 버튼 클릭 → Google 홈페이지로 복귀 → 앞으로 버튼 활성화 확인 (`canGoForward() == true`)
4. 앞으로 버튼 클릭 → 검색 결과 페이지로 복귀
5. 홈 버튼 클릭 → Google 홈페이지로 복귀 → 앞으로 버튼 비활성화 확인 (히스토리 스택 리셋)

### 시나리오 2: 리모컨 Back 키 사용
1. Google 홈페이지 로드
2. "Naver" 검색 → 검색 결과 페이지 로드
3. WebView에 포커스 → 리모컨 Back 키 → Google 홈페이지로 복귀
4. 리모컨 아래 방향키 → NavigationBar로 포커스 이동
5. 리모컨 Back 키 → 포커스 이탈 (페이지 이동 안 함, event->ignore())

### 시나리오 3: 새로고침 동작
1. YouTube 홈페이지 로드
2. 비디오 하나 클릭 → 비디오 페이지 로드
3. 새로고침 버튼 클릭 → 비디오 페이지 재로드 → 동일 비디오 표시 확인

### 시나리오 4: 에러 대응
1. 네트워크 끊기 → "example.com" 로드 실패 (WebView 에러 시그널)
2. 뒤로 버튼 클릭 → 정상 동작 (이전 페이지로 복귀)
3. 네트워크 복구 → 새로고침 버튼 클릭 → 페이지 정상 로드

### 시나리오 5: 버튼 상태 동기화
1. 여러 페이지 탐색 (Google → Naver → YouTube → 뒤로 → 뒤로 → 앞으로)
2. 각 페이지 이동 후 버튼 상태 확인 (활성/비활성)
3. `canGoBack()`, `canGoForward()` 결과와 버튼 `isEnabled()` 상태 일치 확인
4. WebView의 `urlChanged()` 시그널 연결 정상 확인

## 9. 참조 문서

- **PRD**: `docs/project/prd.md`
- **기능 백로그**: `docs/project/features.md` (F-04 섹션)
- **F-02 설계서**: `docs/specs/webview-integration/design.md` (WebView API 인터페이스)
- **WebView 헤더**: `src/browser/WebView.h` (구현된 API 참조)
- **CLAUDE.md**: `/Users/jsong/dev/jsong1230-github/webosbrowser-native/CLAUDE.md` (프로젝트 규칙)
- **Qt QPushButton**: https://doc.qt.io/qt-5/qpushbutton.html
- **Qt QToolButton**: https://doc.qt.io/qt-5/qtoolbutton.html
- **Qt WebEngineView**: https://doc.qt.io/qt-5/qwebengineview.html
- **Qt Focus Management**: https://doc.qt.io/qt-5/focus.html

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | Native App(Qt/C++) 관점으로 재작성 | Web App PoC에서 Native App 전환 |
| 2026-02-14 | React/Enact 내용을 Qt Widgets로 교체 | Qt 5.15+ 기반 구현 |
| 2026-02-14 | WebView API를 Qt WebEngineView 기반으로 변경 | F-02에서 구현된 WebView 클래스 사용 |
