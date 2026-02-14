# KeyboardHandler (리모컨 단축키) 컴포넌트 문서

## 개요

**컴포넌트명**: KeyboardHandler (리모컨 단축키 처리)
**관련 기능**: F-13 (리모컨 단축키)
**구현 위치**: `BrowserWindow::keyPressEvent()` + `src/utils/KeyCodeConstants.h`
**목적**: webOS 리모컨의 채널 버튼, 컬러 버튼, 숫자 버튼을 활용하여 브라우저 주요 기능에 빠르게 접근

## 아키텍처

### 설계 철학

- **단순성 우선 (KISS 원칙)**: 별도 서비스 계층 없이 `BrowserWindow::keyPressEvent()`에서 직접 처리
- **확장성 고려**: 키 코드를 상수로 분리하여 향후 사용자 정의 키 매핑 기능 추가 용이

### 이벤트 전파 흐름

```
webOS 리모컨 입력
    ↓
Qt Event System (QApplication::notify)
    ↓
BrowserWindow::keyPressEvent()
    │
    ├─ URLBar 포커스 체크
    │   └─ Yes → event->ignore() → URLBar로 전달
    │
    ├─ 패널 열림 체크
    │   └─ Yes → 패널 내부에서 처리 (Back 키는 BrowserWindow에서 처리)
    │
    ├─ 디바운스 체크 (0.5초 이내 중복 입력 방지)
    │   └─ Yes → event->ignore()
    │
    ├─ 채널 버튼 → TabManager::previousTab() / nextTab()
    ├─ 컬러 버튼 → 패널 열기 / 새 탭 생성
    ├─ 숫자 버튼 → TabManager::switchToTab(index)
    └─ 설정 버튼 → SettingsPanel::show()
```

## 구현 파일

### 1. KeyCodeConstants.h

**위치**: `src/utils/KeyCodeConstants.h`
**역할**: webOS 리모컨 키 코드 상수 정의

**주요 상수**:
```cpp
namespace webosbrowser {
namespace KeyCode {
    // 채널 버튼
    constexpr int CHANNEL_UP = 427;
    constexpr int CHANNEL_DOWN = 428;

    // 컬러 버튼
    constexpr int RED = 403;      // 북마크
    constexpr int GREEN = 405;    // 히스토리
    constexpr int YELLOW = 406;   // 다운로드
    constexpr int BLUE = 407;     // 새 탭

    // 숫자 버튼
    constexpr int NUM_1 = 49;     // 첫 번째 탭
    constexpr int NUM_2 = 50;     // 두 번째 탭
    constexpr int NUM_3 = 51;
    constexpr int NUM_4 = 52;
    constexpr int NUM_5 = 53;

    // 설정 버튼
    constexpr int MENU = 457;
    constexpr int SETTINGS = 18;  // 대체 키코드

    // 기타
    constexpr int BACK = 27;
}
}
```

**주의사항**:
- webOS 기기 및 펌웨어 버전에 따라 keyCode가 다를 수 있음
- Yellow(406)와 Green(405) 키 코드가 중복될 가능성 있음 (실제 기기 테스트 필요)

### 2. BrowserWindow::keyPressEvent()

**위치**: `src/browser/BrowserWindow.cpp`
**역할**: 리모컨 키 입력 처리의 진입점

**구현 흐름**:
1. 키 코드 추출 (`event->key()`)
2. Logger::debug로 키 입력 로그 기록
3. URLBar 포커스 체크 (숫자/채널 버튼 무시)
4. 패널 열림 체크 (Back 키만 처리)
5. 디바운스 체크 (0.5초 이내 중복 입력 방지)
6. 키 매핑별 핸들러 호출
7. 기본 처리 (`QMainWindow::keyPressEvent()`)

### 3. 핸들러 메서드

#### handleChannelButton(int keyCode)
- **기능**: 채널 Up/Down 버튼으로 탭 전환
- **조건**: 탭이 2개 이상일 때만 동작
- **동작**:
  - `CHANNEL_UP` → `TabManager::previousTab()` (순환)
  - `CHANNEL_DOWN` → `TabManager::nextTab()` (순환)

#### handleColorButton(int keyCode)
- **기능**: 컬러 버튼으로 패널 열기 및 새 탭 생성
- **동작**:
  - `RED` → `BookmarkPanel::show()` (북마크 패널)
  - `GREEN` → `HistoryPanel::show()` (히스토리 패널)
  - `YELLOW` → `DownloadPanel::show()` (다운로드 패널, F-12 구현 시)
  - `BLUE` → `TabManager::createTab()` (새 탭, 최대 5개 제한)
- **중복 방지**: `isVisible()` 체크하여 이미 열린 패널은 무시

#### handleNumberButton(int keyCode)
- **기능**: 숫자 버튼으로 특정 탭에 즉시 이동
- **동작**: `1~5` → `TabManager::switchToTab(index)`
- **조건**:
  - URLBar 포커스 시 무시 (URLBar에 숫자 입력)
  - 존재하지 않는 탭 번호 무시
  - 이미 활성화된 탭 무시

#### handleMenuButton(int keyCode)
- **기능**: 설정 버튼으로 설정 패널 열기
- **동작**: `SettingsPanel::show()` (F-11 구현 시)

## 디바운스 메커니즘

**목적**: 리모컨 버튼의 물리적 반복 입력 방지 (채터링)

**구현 방식**:
```cpp
QMap<int, qint64> lastKeyPressTime_;  // 키별 마지막 입력 시각
static constexpr int DEBOUNCE_MS = 500;  // 0.5초

bool BrowserWindow::shouldDebounce(int keyCode) {
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (lastKeyPressTime_.contains(keyCode)) {
        qint64 lastTime = lastKeyPressTime_[keyCode];
        if (currentTime - lastTime < DEBOUNCE_MS) {
            return true;  // 중복 입력
        }
    }
    lastKeyPressTime_[keyCode] = currentTime;
    return false;  // 처리 허용
}
```

**디바운스 대상 키**:
- 채널 버튼 (Channel Up/Down)
- 컬러 버튼 (Red/Green/Yellow/Blue)
- 숫자 버튼 (1~5)
- 설정 버튼 (Menu)

## TabManager 시그널 연결

`BrowserWindow::setupConnections()`에서 TabManager 시그널 연결:

```cpp
connect(tabManager_, &TabManager::tabSwitched, this,
    [this](int index, const QString& url, const QString& title) {
        urlBar_->setText(url);
        navigationBar_->updateTitle(title);
        webView_->setFocus();
        Logger::info(QString("탭 %1 활성화: %2").arg(index + 1).arg(title));
    });

connect(tabManager_, &TabManager::tabCreated, this,
    [this](int index) {
        Logger::info(QString("탭 %1 생성됨").arg(index + 1));
    });

connect(tabManager_, &TabManager::tabClosed, this,
    [this](int index) {
        Logger::info(QString("탭 %1 닫힘").arg(index + 1));
    });
```

## 포커스 관리

### 포커스 우선순위

1. **URLBar 포커스** (최우선):
   - 가상 키보드 열려있을 때
   - 숫자 키, 채널 버튼은 URLBar로 입력
   - 컬러 버튼, 설정 버튼은 BrowserWindow에서 처리

2. **패널 포커스** (2순위):
   - 북마크, 히스토리 패널이 열려있을 때
   - Back 키만 BrowserWindow에서 처리 (패널 닫기)
   - 나머지 키는 패널 내부에서 처리

3. **WebView 포커스** (기본):
   - 일반적인 브라우징 상태
   - 모든 리모컨 단축키가 BrowserWindow에서 처리됨

### 포커스 전환 흐름

```
탭 전환 → webView_->setFocus()
패널 열기 → bookmarkPanel_->setFocus()
패널 닫기 → webView_->setFocus()
URLBar 클릭 → urlBar_->setFocusToInput()
```

## 키 매핑 테이블

| 키 | Qt::Key | webOS keyCode | 동작 | 관련 기능 |
|----|---------|---------------|------|-----------|
| Channel Up | Qt::Key_ChannelUp | 427 | 이전 탭 전환 | F-06 |
| Channel Down | Qt::Key_ChannelDown | 428 | 다음 탭 전환 | F-06 |
| Red | Qt::Key_Red | 403 | 북마크 패널 열기 | F-07 |
| Green | Qt::Key_Green | 405 | 히스토리 패널 열기 | F-08 |
| Yellow | Qt::Key_Yellow | 406 | 다운로드 패널 열기 | F-12 |
| Blue | Qt::Key_Blue | 407 | 새 탭 생성 | F-06 |
| 1~5 | Qt::Key_1~5 | 49~53 | 직접 탭 선택 | F-06 |
| Menu | Qt::Key_Menu | 457 | 설정 패널 열기 | F-11 |
| Back | Qt::Key_Escape | 27 | 패널 닫기 / 브라우저 뒤로 | - |

## 로깅

### 디버그 로그
- 모든 키 입력: `[BrowserWindow] 키 입력: keyCode=427`
- 디바운스 무시: `[BrowserWindow] 디바운스: keyCode=427 무시`
- 탭 전환 무시: `[BrowserWindow] 탭이 1개뿐이므로 전환 무시`
- 존재하지 않는 탭: `[BrowserWindow] 존재하지 않는 탭 번호: 4`

### 정보 로그
- 탭 전환 성공: `[BrowserWindow] 탭 전환 완료`
- 패널 열기: `[BrowserWindow] Red 버튼 → 북마크 패널 열림`
- 새 탭 생성: `[BrowserWindow] Blue 버튼 → 새 탭 생성 완료`

### 경고 로그
- 탭 전환 실패: `[BrowserWindow] 탭 전환 실패`
- 최대 탭 수 초과: `[BrowserWindow] Blue 버튼 → 새 탭 생성 실패: 최대 5개`
- 알 수 없는 키: `[BrowserWindow] 알 수 없는 컬러 버튼: 999`

## 제약사항

### webOS 리모컨 키코드 제약
- 기기별 keyCode 차이 가능 (실제 기기 테스트 필수)
- Yellow(406)와 Green(405) 키 코드 중복 가능
- Home 버튼, Power 버튼은 webOS 시스템 예약 (앱에서 가로챌 수 없음)

### Qt 이벤트 시스템 제약
- 이벤트 전파 순서: 자식 위젯 → 부모 위젯
- `event->accept()` 호출 시 전파 중단
- `event->ignore()` 호출 시 부모로 전달

### 성능 제약
- 탭 전환 시 WebView::load() 호출 → 0.5~1초 지연 가능
- 디바운스로 0.5초 이내 중복 입력 무시

## 의존성

### 필수 의존
- **TabManager**: 탭 전환, 탭 생성, 탭 닫기 기능
- **BookmarkPanel**: Red 버튼으로 패널 열기
- **HistoryPanel**: Green 버튼으로 패널 열기
- **URLBar**: 포커스 체크 및 URL 업데이트
- **NavigationBar**: 탭 전환 시 제목 업데이트
- **WebView**: 탭 전환 시 포커스 이동

### 선택적 의존
- **DownloadPanel** (F-12): Yellow 버튼으로 패널 열기
- **SettingsPanel** (F-11): Menu 버튼으로 패널 열기

## 테스트

### 수동 테스트 시나리오

1. **채널 버튼 탭 전환**:
   - 탭 3개 생성
   - Channel Down 연속 입력 → 순환 전환 확인

2. **컬러 버튼 패널 열기**:
   - Red 버튼 → 북마크 패널 열림
   - Back 키 → 패널 닫힘

3. **숫자 버튼 탭 선택**:
   - 탭 4개 생성
   - 숫자 3 버튼 → 세 번째 탭 활성화

4. **URLBar 포커스 충돌 방지**:
   - URLBar 클릭
   - 숫자 3 입력 → URLBar에 "3" 입력됨 (탭 전환 안 됨)

5. **최대 탭 수 제한**:
   - Blue 버튼 5회 연속 입력
   - 6번째는 무시 + 경고 로그

### 실제 기기 테스트 체크리스트

- [ ] 모든 리모컨 버튼의 keyCode 로그 확인
- [ ] 채널 버튼 탭 전환 동작 확인
- [ ] 컬러 버튼 패널 열기 동작 확인
- [ ] 숫자 버튼 탭 선택 동작 확인
- [ ] 디바운스 0.5초 동작 확인
- [ ] Yellow/Green 키 코드 중복 여부 확인
- [ ] 탭 전환 시 렌더링 지연 시간 측정 (0.5초 이내 목표)

## 향후 개선 사항 (M3 이후)

### 1. 재생 버튼 자동 스크롤 (M3)
- Play/Pause 버튼으로 자동 스크롤 시작/멈춤
- FastForward/Rewind 버튼으로 페이지 끝/위로 이동

### 2. 사용자 정의 키 매핑 (M4)
- 설정 화면에서 키 매핑 변경 기능
- KeyboardHandler 서비스 클래스 분리

### 3. 키보드 단축키 튜토리얼 (M5)
- 첫 실행 시 오버레이 화면 표시
- 주요 리모컨 버튼 그림 + 기능 설명

## 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 최초 작성 | cpp-dev |
| 2026-02-14 | Phase 1-5 구현 완료 (채널/컬러/숫자/설정 버튼) | cpp-dev |
| 2026-02-14 | TabManager 다중 탭 지원 리팩토링 완료 | cpp-dev |
