# F-13: 리모컨 단축키 — 기술 설계서

## 1. 참조
- **요구사항 분석서**: `docs/specs/remote-shortcuts/requirements.md`
- **관련 기능**:
  - F-06 (탭 관리 시스템)
  - F-07 (북마크 관리)
  - F-08 (히스토리 관리)
  - F-11 (설정 화면)
  - F-12 (다운로드 관리)

---

## 2. 아키텍처 개요

### 2.1 설계 철학

**단순성 우선 (KISS 원칙)**:
- 초기 버전(M2)은 BrowserWindow::keyPressEvent()에서 직접 키 매핑 처리
- 복잡한 KeyboardHandler 서비스 계층은 사용자 정의 키 매핑 기능이 추가되는 M3 이후로 연기
- Qt의 이벤트 시스템을 최대한 활용하여 별도 추상화 레이어 없이 구현

**확장성 고려**:
- webOS 키 코드를 상수로 분리하여 향후 기기별 키코드 변경에 대응
- 디바운스 메커니즘을 범용적으로 설계하여 모든 키에 재사용 가능
- 키 매핑 로직을 명확히 분리하여 향후 설정 화면 연동 용이

### 2.2 이벤트 전파 흐름

```
webOS 리모컨 입력
    ↓
Qt Event System (QApplication::notify)
    ↓
BrowserWindow::keyPressEvent()  ←─ 여기서 모든 단축키 처리
    │
    ├─ URLBar 포커스 체크 (hasFocus())
    │   └─ Yes → event->ignore() → URLBar::keyPressEvent()로 전달
    │
    ├─ 패널 열림 체크 (bookmarkPanel_->isVisible())
    │   └─ Yes → 패널 내부에서 처리
    │
    ├─ 디바운스 체크 (lastKeyPressTime_)
    │   └─ 0.5초 이내 재입력 → event->ignore()
    │
    ├─ 채널 버튼 처리
    │   ├─ Channel Up → tabManager_->previousTab()
    │   └─ Channel Down → tabManager_->nextTab()
    │
    ├─ 컬러 버튼 처리
    │   ├─ Red → bookmarkPanel_->show()
    │   ├─ Green → historyPanel_->show()
    │   ├─ Yellow → downloadPanel_->show()
    │   └─ Blue → tabManager_->createTab()
    │
    ├─ 숫자 버튼 처리
    │   └─ 1~5 → tabManager_->switchToTab(index)
    │
    ├─ 설정 버튼 처리
    │   └─ Menu → settingsPanel_->show()
    │
    └─ 재생 버튼 처리 (M3 이후)
        ├─ Play/Pause → autoScroll start/stop
        ├─ FastForward → scrollToBottom()
        └─ Rewind → scrollToTop()
```

---

## 3. 아키텍처 결정

### 결정 1: 키 이벤트 처리 위치

- **선택지**:
  - A) BrowserWindow::keyPressEvent()에서 직접 처리
  - B) KeyboardHandler 서비스 클래스로 분리
  - C) Qt의 QShortcut 클래스 사용
- **결정**: **A) BrowserWindow::keyPressEvent()에서 직접 처리**
- **근거**:
  - M2 목표는 빠른 PoC 구현 (3일 이내 완료)
  - 키 매핑이 단순하고 고정적 (사용자 정의 불필요)
  - Qt 이벤트 전파 흐름을 직접 제어할 수 있어 포커스 관리 용이
  - QShortcut은 리모컨 특수키(Channel Up/Down, Color Button)와 호환성 불확실
- **트레이드오프**:
  - 포기: 깔끔한 관심사 분리 (SoC), 테스트 용이성
  - 획득: 빠른 구현, 단순한 디버깅, Qt 이벤트 시스템과의 완벽한 통합

### 결정 2: 키 코드 상수 관리

- **선택지**:
  - A) 헤더 파일에 constexpr 상수로 정의 (`KeyCodeConstants.h`)
  - B) enum class로 정의 (`enum class RemoteKey { ChannelUp = 427, ... }`)
  - C) 설정 파일 (JSON)에서 런타임 로드
- **결정**: **A) 헤더 파일에 constexpr 상수로 정의**
- **근거**:
  - 컴파일 타임 상수이므로 런타임 오버헤드 없음
  - 코드에서 `KEY_CHANNEL_UP` 같은 직관적인 이름 사용 가능
  - enum class는 Qt::Key와 비교 시 명시적 캐스팅 필요 (코드 복잡도 증가)
  - 설정 파일 방식은 런타임 파싱 오버헤드와 에러 핸들링 복잡도 증가
- **트레이드오프**:
  - 포기: 런타임 키 매핑 변경 (리컴파일 필요)
  - 획득: 타입 안정성, 제로 런타임 오버헤드, 단순한 코드

### 결정 3: 디바운스 메커니즘

- **선택지**:
  - A) QMap<int, qint64> + QDateTime::currentMSecsSinceEpoch()
  - B) QTimer + QMap<int, QTimer*>
  - C) 디바운스 없이 빠른 연속 입력 허용
- **결정**: **A) QMap<int, qint64> + QDateTime::currentMSecsSinceEpoch()**
- **근거**:
  - 가장 단순한 구현 (QTimer 객체 관리 불필요)
  - 메모리 효율적 (키당 8바이트만 사용)
  - 리모컨 버튼은 물리적 반복 입력이 드물어 QTimer의 비동기 처리 불필요
- **트레이드오프**:
  - 포기: QTimer의 우아한 비동기 디자인
  - 획득: 단순성, 낮은 메모리 사용량, 동기적 처리

### 결정 4: 탭 전환 구현 방식

- **선택지**:
  - A) TabManager에 previousTab()/nextTab() 메서드 추가 (순환 전환)
  - B) TabManager::switchToTab(int index) 사용하여 BrowserWindow에서 인덱스 계산
  - C) 현재 TabManager는 단일 탭만 지원 → 리팩토링하여 다중 탭 구조로 변경
- **결정**: **C) TabManager 리팩토링 → 다중 탭 구조로 변경**
- **근거**:
  - 현재 TabManager는 단일 WebView만 관리 (`currentTab_` 포인터)
  - 요구사항에서 "탭 전환" 기능이 필수이므로 다중 탭 지원 필수
  - F-06 (탭 관리 시스템)이 이미 구현되었다고 가정하면 리팩토링 필요
  - 순환 전환 로직(`previousTab()`, `nextTab()`)은 TabManager에서 관리하는 것이 응집도 향상
- **트레이드오프**:
  - 포기: 단일 탭의 단순성
  - 획득: 확장 가능한 탭 관리 아키텍처, 명확한 책임 분리

### 결정 5: 포커스 우선순위 처리

- **선택지**:
  - A) BrowserWindow::keyPressEvent() 최상단에서 포커스 체크 후 조기 반환
  - B) installEventFilter()로 모든 자식 위젯의 키 이벤트 가로채기
  - C) 각 위젯(URLBar, BookmarkPanel)에서 처리 후 event->accept() 호출
- **결정**: **A) BrowserWindow::keyPressEvent() 최상단에서 포커스 체크**
- **근거**:
  - Qt 이벤트 전파는 자식 → 부모 순이지만, BrowserWindow에서 명시적으로 체크하면 의도 명확
  - installEventFilter()는 모든 이벤트를 가로채므로 오버헤드 증가
  - URLBar, BookmarkPanel의 기존 keyPressEvent() 구현을 유지하면서 충돌 방지
- **트레이드오프**:
  - 포기: Qt의 기본 이벤트 전파 흐름 활용
  - 획득: 명확한 우선순위 제어, 디버깅 용이성

---

## 4. 클래스 설계

### 4.1 KeyCodeConstants.h (새 파일)

**위치**: `src/utils/KeyCodeConstants.h`

**목적**: webOS 리모컨 키 코드를 상수로 정의하여 코드 가독성 향상 및 유지보수 용이

```cpp
/**
 * @file KeyCodeConstants.h
 * @brief webOS 리모컨 키 코드 상수 정의
 *
 * LG webOS 프로젝터 HU715QW 리모컨의 keyCode 매핑.
 * 기기 및 펌웨어 버전에 따라 달라질 수 있으므로
 * 실제 기기에서 QKeyEvent::key() 값을 로그로 확인 후 수정.
 */

#pragma once

namespace webosbrowser {
namespace KeyCode {

// 채널 버튼
constexpr int CHANNEL_UP = 427;      // Qt::Key_ChannelUp
constexpr int CHANNEL_DOWN = 428;    // Qt::Key_ChannelDown

// 컬러 버튼
constexpr int RED = 403;             // Qt::Key_Red
constexpr int GREEN = 405;           // Qt::Key_Green
constexpr int YELLOW = 406;          // Qt::Key_Yellow (주의: Green과 중복 가능)
constexpr int BLUE = 407;            // Qt::Key_Blue

// 숫자 버튼
constexpr int NUM_1 = 49;            // Qt::Key_1
constexpr int NUM_2 = 50;            // Qt::Key_2
constexpr int NUM_3 = 51;            // Qt::Key_3
constexpr int NUM_4 = 52;            // Qt::Key_4
constexpr int NUM_5 = 53;            // Qt::Key_5

// 설정 버튼
constexpr int MENU = 457;            // Qt::Key_Menu
constexpr int SETTINGS = 18;         // 대체 키코드 (기기에 따라 다름)

// 재생 컨트롤 버튼 (M3 이후)
constexpr int PLAY = 415;            // Qt::Key_MediaPlay
constexpr int PAUSE = 19;            // Qt::Key_MediaPause
constexpr int FAST_FORWARD = 417;    // Qt::Key_MediaNext
constexpr int REWIND = 412;          // Qt::Key_MediaPrevious

// 기타
constexpr int BACK = 27;             // Qt::Key_Escape (Back 버튼)

} // namespace KeyCode
} // namespace webosbrowser
```

### 4.2 BrowserWindow 확장

**파일**: `src/browser/BrowserWindow.h`

**변경 내용**: 키 이벤트 처리 메서드 및 디바운스 관련 멤버 추가

```cpp
// BrowserWindow.h에 추가할 내용

protected:
    /**
     * @brief 리모컨 키 이벤트 처리 (오버라이드)
     * @param event 키 이벤트
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    /**
     * @brief 디바운스 체크 (0.5초 이내 중복 입력 방지)
     * @param keyCode 키 코드
     * @return true이면 중복 입력 (무시), false이면 처리 허용
     */
    bool shouldDebounce(int keyCode);

    /**
     * @brief 채널 버튼 처리 (탭 전환)
     * @param keyCode Qt::Key_ChannelUp 또는 Qt::Key_ChannelDown
     */
    void handleChannelButton(int keyCode);

    /**
     * @brief 컬러 버튼 처리 (패널 열기, 새 탭)
     * @param keyCode Qt::Key_Red, Green, Yellow, Blue
     */
    void handleColorButton(int keyCode);

    /**
     * @brief 숫자 버튼 처리 (직접 탭 선택)
     * @param keyCode Qt::Key_1 ~ Qt::Key_5
     */
    void handleNumberButton(int keyCode);

    /**
     * @brief 설정 버튼 처리
     * @param keyCode Qt::Key_Menu
     */
    void handleMenuButton(int keyCode);

    /**
     * @brief 재생 버튼 처리 (M3 이후)
     * @param keyCode Play, Pause, FastForward, Rewind
     */
    void handlePlaybackButton(int keyCode);

private:
    // 디바운스 관리
    QMap<int, qint64> lastKeyPressTime_;  ///< 키별 마지막 입력 시각 (ms)
    static constexpr int DEBOUNCE_MS = 500;  ///< 디바운스 지속 시간 (0.5초)

    // 자동 스크롤 (M3 이후)
    QTimer *autoScrollTimer_;  ///< 자동 스크롤 타이머
    bool isAutoScrolling_;     ///< 자동 스크롤 중 여부
    static constexpr int AUTO_SCROLL_INTERVAL_MS = 10;  ///< 스크롤 간격 (10ms)
    static constexpr int AUTO_SCROLL_STEP_PX = 5;       ///< 스크롤 단계 (5px)
```

### 4.3 TabManager 확장

**파일**: `src/browser/TabManager.h`

**변경 내용**: 다중 탭 지원 및 탭 전환 메서드 추가

```cpp
// TabManager.h 변경 내용

public:
    /**
     * @brief 새 탭 생성 (최대 5개 제한)
     * @param url 초기 URL (기본값: Google 홈페이지)
     * @return 생성 성공 여부
     */
    bool createTab(const QString& url = "https://www.google.com");

    /**
     * @brief 탭 전환 (인덱스 지정)
     * @param index 탭 인덱스 (0~4)
     * @return 전환 성공 여부
     */
    bool switchToTab(int index);

    /**
     * @brief 이전 탭으로 전환 (순환)
     * @return 전환 성공 여부
     */
    bool previousTab();

    /**
     * @brief 다음 탭으로 전환 (순환)
     * @return 전환 성공 여부
     */
    bool nextTab();

    /**
     * @brief 탭 닫기
     * @param index 탭 인덱스
     * @return 닫기 성공 여부
     */
    bool closeTab(int index);

    /**
     * @brief 현재 활성 탭의 인덱스 반환
     * @return 탭 인덱스 (0~4)
     */
    int getCurrentTabIndex() const;

signals:
    /**
     * @brief 탭 전환 시그널
     * @param index 새 탭 인덱스
     * @param url 새 탭 URL
     * @param title 새 탭 제목
     */
    void tabSwitched(int index, const QString& url, const QString& title);

    /**
     * @brief 새 탭 생성 시그널
     * @param index 새 탭 인덱스
     */
    void tabCreated(int index);

    /**
     * @brief 탭 닫힘 시그널
     * @param index 닫힌 탭 인덱스
     */
    void tabClosed(int index);

private:
    struct TabData {
        QString url;
        QString title;
        QByteArray historyState;  ///< 웹 히스토리 상태 (직렬화)
    };

    QVector<TabData> tabs_;       ///< 탭 목록 (최대 5개)
    int currentTabIndex_;         ///< 현재 활성 탭 인덱스
    WebView *webView_;            ///< 공유 WebView 인스턴스

    static constexpr int MAX_TABS = 5;  ///< 최대 탭 개수

    /**
     * @brief 탭 데이터 저장 (전환 전)
     */
    void saveCurrentTabState();

    /**
     * @brief 탭 데이터 복원 (전환 후)
     * @param index 탭 인덱스
     */
    void restoreTabState(int index);
```

**주의**: 현재 TabManager는 단일 탭만 지원하는 간소화 버전이므로, 실제 구현 시 위 설계대로 리팩토링 필요.

---

## 5. 키 이벤트 처리 플로우

### 5.1 BrowserWindow::keyPressEvent() 구조

```cpp
void BrowserWindow::keyPressEvent(QKeyEvent *event) {
    int keyCode = event->key();

    // 로깅 (디버깅용)
    Logger::debug(QString("[BrowserWindow] 키 입력: keyCode=%1").arg(keyCode));

    // 1. URLBar 포커스 체크 (숫자 키, 채널 버튼 무시)
    if (urlBar_->hasFocus()) {
        if (keyCode >= KeyCode::NUM_1 && keyCode <= KeyCode::NUM_5) {
            event->ignore();  // URLBar로 전달
            return;
        }
        if (keyCode == KeyCode::CHANNEL_UP || keyCode == KeyCode::CHANNEL_DOWN) {
            event->ignore();
            return;
        }
        // 컬러 버튼, 설정 버튼은 처리 계속 (패널 열기 허용)
    }

    // 2. 패널 열림 체크 (패널 내부 키 이벤트 우선)
    if (bookmarkPanel_->isVisible() && bookmarkPanel_->hasFocus()) {
        // Back 키만 처리 (패널 닫기)
        if (keyCode == KeyCode::BACK) {
            bookmarkPanel_->hide();
            webView_->setFocus();
            event->accept();
            return;
        }
        event->ignore();  // 패널에서 처리
        return;
    }
    // historyPanel_, downloadPanel_, settingsPanel_도 동일 처리

    // 3. 디바운스 체크
    if (shouldDebounce(keyCode)) {
        Logger::debug(QString("[BrowserWindow] 디바운스: keyCode=%1 무시").arg(keyCode));
        event->accept();
        return;
    }

    // 4. 키 매핑 처리
    if (keyCode == KeyCode::CHANNEL_UP || keyCode == KeyCode::CHANNEL_DOWN) {
        handleChannelButton(keyCode);
        event->accept();
    } else if (keyCode == KeyCode::RED || keyCode == KeyCode::GREEN ||
               keyCode == KeyCode::YELLOW || keyCode == KeyCode::BLUE) {
        handleColorButton(keyCode);
        event->accept();
    } else if (keyCode >= KeyCode::NUM_1 && keyCode <= KeyCode::NUM_5) {
        handleNumberButton(keyCode);
        event->accept();
    } else if (keyCode == KeyCode::MENU || keyCode == KeyCode::SETTINGS) {
        handleMenuButton(keyCode);
        event->accept();
    } else if (keyCode == KeyCode::PLAY || keyCode == KeyCode::PAUSE ||
               keyCode == KeyCode::FAST_FORWARD || keyCode == KeyCode::REWIND) {
        // M3 이후 구현
        handlePlaybackButton(keyCode);
        event->accept();
    } else {
        // 기본 처리 (Qt 기본 동작, 예: Back 키 = 브라우저 뒤로가기)
        QMainWindow::keyPressEvent(event);
    }
}
```

### 5.2 handleChannelButton() 구현

```cpp
void BrowserWindow::handleChannelButton(int keyCode) {
    if (tabManager_->getTabCount() <= 1) {
        Logger::debug("[BrowserWindow] 탭이 1개뿐이므로 전환 무시");
        return;
    }

    bool success = false;
    if (keyCode == KeyCode::CHANNEL_UP) {
        success = tabManager_->previousTab();
    } else if (keyCode == KeyCode::CHANNEL_DOWN) {
        success = tabManager_->nextTab();
    }

    if (success) {
        // 탭 전환 성공 시 UI 업데이트 (시그널로 자동 처리됨)
        Logger::info("[BrowserWindow] 탭 전환 완료");
    } else {
        Logger::warning("[BrowserWindow] 탭 전환 실패");
    }
}
```

### 5.3 handleColorButton() 구현

```cpp
void BrowserWindow::handleColorButton(int keyCode) {
    switch (keyCode) {
        case KeyCode::RED:
            if (!bookmarkPanel_->isVisible()) {
                bookmarkPanel_->show();
                bookmarkPanel_->setFocus();
                Logger::info("[BrowserWindow] 북마크 패널 열림");
            }
            break;
        case KeyCode::GREEN:
            if (!historyPanel_->isVisible()) {
                historyPanel_->show();
                historyPanel_->setFocus();
                Logger::info("[BrowserWindow] 히스토리 패널 열림");
            }
            break;
        case KeyCode::YELLOW:
            if (!downloadPanel_->isVisible()) {
                downloadPanel_->show();
                downloadPanel_->setFocus();
                Logger::info("[BrowserWindow] 다운로드 패널 열림");
            }
            break;
        case KeyCode::BLUE:
            if (tabManager_->getTabCount() >= 5) {
                Logger::warning("[BrowserWindow] 새 탭 생성 실패: 최대 5개");
                // 선택적: QMessageBox 경고 표시
            } else {
                bool success = tabManager_->createTab();
                if (success) {
                    Logger::info("[BrowserWindow] 새 탭 생성 완료");
                }
            }
            break;
        default:
            Logger::warning(QString("[BrowserWindow] 알 수 없는 컬러 버튼: %1").arg(keyCode));
    }
}
```

### 5.4 handleNumberButton() 구현

```cpp
void BrowserWindow::handleNumberButton(int keyCode) {
    int tabIndex = keyCode - KeyCode::NUM_1;  // 0~4

    if (tabIndex >= tabManager_->getTabCount()) {
        Logger::debug(QString("[BrowserWindow] 존재하지 않는 탭 번호: %1").arg(tabIndex + 1));
        return;
    }

    if (tabIndex == tabManager_->getCurrentTabIndex()) {
        Logger::debug(QString("[BrowserWindow] 이미 활성화된 탭: %1").arg(tabIndex + 1));
        return;
    }

    bool success = tabManager_->switchToTab(tabIndex);
    if (success) {
        Logger::info(QString("[BrowserWindow] 탭 %1로 전환 완료").arg(tabIndex + 1));
    } else {
        Logger::warning(QString("[BrowserWindow] 탭 %1 전환 실패").arg(tabIndex + 1));
    }
}
```

### 5.5 shouldDebounce() 구현

```cpp
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

---

## 6. TabManager 시그널/슬롯 연결

### 6.1 BrowserWindow::setupConnections()에 추가

```cpp
// TabManager 시그널 연결
connect(tabManager_, &TabManager::tabSwitched, this, [this](int index, const QString& url, const QString& title) {
    // URLBar 업데이트
    urlBar_->setText(url);

    // NavigationBar 상태 업데이트
    navigationBar_->updateTitle(title);

    // WebView 포커스
    webView_->setFocus();

    Logger::info(QString("[BrowserWindow] 탭 %1 활성화: %2").arg(index + 1).arg(title));
});

connect(tabManager_, &TabManager::tabCreated, this, [this](int index) {
    Logger::info(QString("[BrowserWindow] 탭 %1 생성됨").arg(index + 1));
    // 선택적: 탭 바 UI 업데이트 (M3 이후)
});

connect(tabManager_, &TabManager::tabClosed, this, [this](int index) {
    Logger::info(QString("[BrowserWindow] 탭 %1 닫힘").arg(index + 1));
    // 선택적: 탭 바 UI 업데이트 (M3 이후)
});
```

---

## 7. 포커스 관리 전략

### 7.1 포커스 우선순위

1. **URLBar 포커스 (최우선)**:
   - 가상 키보드가 열려있을 때
   - 숫자 키, 채널 버튼은 URLBar로 입력
   - 컬러 버튼, 설정 버튼은 BrowserWindow에서 처리 (패널 열기 허용)

2. **패널 포커스 (2순위)**:
   - 북마크, 히스토리, 다운로드, 설정 패널이 열려있을 때
   - Back 키만 BrowserWindow에서 처리 (패널 닫기)
   - 나머지 키는 패널 내부에서 처리 (방향키, 선택 버튼 등)

3. **WebView 포커스 (기본)**:
   - 일반적인 브라우징 상태
   - 모든 리모컨 단축키가 BrowserWindow에서 처리됨

### 7.2 포커스 전환 흐름

```
탭 전환 → webView_->setFocus()
패널 열기 → bookmarkPanel_->setFocus()
패널 닫기 → webView_->setFocus()
URLBar 클릭 → urlBar_->setFocusToInput()
```

### 7.3 Qt 포커스 정책 설정

```cpp
// BrowserWindow 생성자에서 포커스 정책 설정
webView_->setFocusPolicy(Qt::StrongFocus);
urlBar_->setFocusPolicy(Qt::StrongFocus);
bookmarkPanel_->setFocusPolicy(Qt::StrongFocus);
historyPanel_->setFocusPolicy(Qt::StrongFocus);
```

---

## 8. 디바운스 메커니즘

### 8.1 목적
- 리모컨 버튼의 물리적 반복 입력 방지 (채터링)
- 사용자가 실수로 빠르게 두 번 누르는 경우 방지

### 8.2 구현 방식
- `QMap<int, qint64>` 사용하여 키별 마지막 입력 시각 저장
- 0.5초(500ms) 이내 재입력 시 무시

### 8.3 디바운스 대상 키
- 채널 버튼 (Channel Up/Down)
- 컬러 버튼 (Red/Green/Yellow/Blue)
- 숫자 버튼 (1~5)
- 설정 버튼 (Menu)

### 8.4 디바운스 제외 키
- Back 키 (빠른 연속 닫기 허용)
- 방향키 (Qt 기본 동작, 디바운스 불필요)

---

## 9. 재생 버튼 자동 스크롤 (M3 이후)

### 9.1 구현 계획

**자동 스크롤 시작/멈춤**:
```cpp
void BrowserWindow::handlePlaybackButton(int keyCode) {
    if (keyCode == KeyCode::PLAY) {
        if (!isAutoScrolling_) {
            isAutoScrolling_ = true;
            autoScrollTimer_->start(AUTO_SCROLL_INTERVAL_MS);
            Logger::info("[BrowserWindow] 자동 스크롤 시작");
        }
    } else if (keyCode == KeyCode::PAUSE) {
        if (isAutoScrolling_) {
            isAutoScrolling_ = false;
            autoScrollTimer_->stop();
            Logger::info("[BrowserWindow] 자동 스크롤 멈춤");
        }
    } else if (keyCode == KeyCode::FAST_FORWARD) {
        scrollToBottom();
    } else if (keyCode == KeyCode::REWIND) {
        scrollToTop();
    }
}
```

**자동 스크롤 타이머 슬롯**:
```cpp
connect(autoScrollTimer_, &QTimer::timeout, this, [this]() {
    QString script = QString("window.scrollBy(0, %1);").arg(AUTO_SCROLL_STEP_PX);
    webView_->page()->runJavaScript(script);
});
```

**페이지 끝/위로 이동**:
```cpp
void BrowserWindow::scrollToBottom() {
    QString script = "window.scrollTo(0, document.body.scrollHeight);";
    webView_->page()->runJavaScript(script, [](const QVariant& result) {
        Logger::info("[BrowserWindow] 페이지 끝으로 스크롤 완료");
    });
}

void BrowserWindow::scrollToTop() {
    QString script = "window.scrollTo(0, 0);";
    webView_->page()->runJavaScript(script, [](const QVariant& result) {
        Logger::info("[BrowserWindow] 페이지 맨 위로 스크롤 완료");
    });
}
```

### 9.2 CORS 에러 핸들링

일부 사이트는 JavaScript 실행을 차단할 수 있으므로 에러 처리:
```cpp
webView_->page()->runJavaScript(script, [](const QVariant& result) {
    if (result.isNull()) {
        Logger::warning("[BrowserWindow] JavaScript 실행 실패 (CORS 정책)");
    }
});
```

---

## 10. 영향 범위 분석

### 10.1 수정 필요한 기존 파일

| 파일 경로 | 변경 내용 | 이유 |
|-----------|----------|------|
| `src/browser/BrowserWindow.h` | keyPressEvent() 오버라이드 추가, 디바운스 멤버 변수 추가 | 키 이벤트 처리 |
| `src/browser/BrowserWindow.cpp` | keyPressEvent() 구현, 핸들러 메서드 구현 | 키 매핑 로직 |
| `src/browser/TabManager.h` | 다중 탭 지원 메서드 추가 (createTab, switchToTab 등) | 탭 전환 기능 |
| `src/browser/TabManager.cpp` | 다중 탭 관리 로직 구현 | 탭 데이터 저장/복원 |
| `src/ui/BookmarkPanel.cpp` | 기존 keyPressEvent() 유지 (Back 키 제외) | 패널 내부 키 처리 |
| `src/ui/HistoryPanel.cpp` | 기존 keyPressEvent() 유지 (Back 키 제외) | 패널 내부 키 처리 |
| `src/ui/URLBar.cpp` | 기존 keyPressEvent() 유지 | URL 입력 처리 |

### 10.2 새로 생성할 파일

| 파일 경로 | 역할 | 크기 |
|-----------|------|------|
| `src/utils/KeyCodeConstants.h` | webOS 리모컨 키 코드 상수 정의 | ~100줄 |

### 10.3 영향 받는 기존 기능

| 기능 ID | 기능명 | 영향 내용 |
|---------|--------|----------|
| F-06 | 탭 관리 시스템 | TabManager 리팩토링 필요 (단일 탭 → 다중 탭) |
| F-07 | 북마크 관리 | Red 버튼으로 패널 열기 통합 |
| F-08 | 히스토리 관리 | Green 버튼으로 패널 열기 통합 |
| F-11 | 설정 화면 | Menu 버튼으로 패널 열기 통합 |
| F-12 | 다운로드 관리 | Yellow 버튼으로 패널 열기 통합 |
| F-04 | URL 입력 및 검색 | URLBar 포커스 시 숫자/채널 버튼 무시 |

---

## 11. 기술적 주의사항

### 11.1 webOS 리모컨 키코드 검증

**문제**: webOS 기기 및 펌웨어 버전에 따라 키코드가 다를 수 있음.

**해결**:
1. 개발 초기에 모든 리모컨 버튼의 `QKeyEvent::key()` 값을 로그로 출력
2. 실제 LG 프로젝터 HU715QW에서 테스트 후 `KeyCodeConstants.h` 업데이트
3. Yellow와 Green 버튼이 keyCode 405로 중복될 경우, 대체 키 매핑 고려

### 11.2 Qt 이벤트 전파 순서 이해

**주의**: Qt의 키 이벤트는 자식 위젯 → 부모 위젯 순으로 전파됨.

**영향**:
- URLBar에 포커스가 있을 때 숫자 키를 입력하면 URLBar::keyPressEvent()가 먼저 호출됨
- BrowserWindow::keyPressEvent()에서 명시적으로 `urlBar_->hasFocus()` 체크 필요

### 11.3 TabManager 리팩토링 범위

**현재 상태**: TabManager는 단일 WebView만 관리 (`currentTab_` 포인터).

**필요 작업**:
1. `QVector<TabData> tabs_` 추가
2. 탭 전환 시 현재 WebView 상태 저장 (`saveCurrentTabState()`)
3. 새 탭 활성화 시 상태 복원 (`restoreTabState()`)
4. 웹 히스토리 직렬화/역직렬화 (QWebEngineHistory API 사용)

**리스크**: F-06이 아직 다중 탭을 완전히 지원하지 않을 경우, F-13 구현 전 F-06 리팩토링 필요.

### 11.4 자동 스크롤 성능

**문제**: 10ms 간격으로 JavaScript 실행 시 CPU 부하 발생 가능.

**대안**:
1. 스크롤 간격을 50ms로 늘리고 step을 20px로 증가
2. Qt의 QPropertyAnimation + QGraphicsOpacityEffect 사용 (JavaScript 없이)
3. 사용자 설정에서 스크롤 속도 조절 가능하게 (M4 이후)

### 11.5 패널 오버레이 충돌 방지

**문제**: 여러 패널이 동시에 열리는 경우 포커스 충돌.

**해결**:
1. 컬러 버튼 핸들러에서 `isVisible()` 체크 후 패널 열기
2. 패널 열기 시 다른 패널 자동 닫기 (선택적)
3. 패널 열림/닫힘 애니메이션 중복 방지 (QPropertyAnimation::State 체크)

---

## 12. 테스트 전략

### 12.1 단위 테스트

**파일**: `tests/unit/BrowserWindowKeyEventTest.cpp`

**테스트 케이스**:
- `test_shouldDebounce_ReturnsTrueWithinDebouncePeriod()`
- `test_handleChannelButton_SwitchesTab()`
- `test_handleColorButton_OpensPanel()`
- `test_handleNumberButton_SwitchesToTabByIndex()`
- `test_handleNumberButton_IgnoresIfURLBarHasFocus()`

### 12.2 통합 테스트

**파일**: `tests/integration/RemoteShortcutsIntegrationTest.cpp`

**시나리오**:
1. 탭 3개 생성 → Channel Down 누름 → 탭 전환 확인
2. Red 버튼 누름 → 북마크 패널 열림 확인
3. URLBar 포커스 상태에서 숫자 3 입력 → URLBar에 "3" 입력 확인

### 12.3 실제 기기 테스트

**필수 확인 사항**:
1. LG 프로젝터 HU715QW에서 모든 리모컨 버튼의 keyCode 로그 확인
2. 탭 전환 시 WebView 렌더링 지연 시간 측정 (0.5초 이내)
3. 패널 슬라이드 인 애니메이션 부드러움 확인 (60fps)
4. 디바운스 0.5초 동작 확인 (빠른 연속 입력 무시)

---

## 13. 시퀀스 다이어그램

### 13.1 채널 버튼 탭 전환 (정상 흐름)

```
사용자 → BrowserWindow → TabManager → WebView
  │                 │              │         │
  │  Channel Down   │              │         │
  │────────────────▶│              │         │
  │                 │  shouldDebounce()     │
  │                 │──────────┐   │         │
  │                 │◀─────────┘   │         │
  │                 │  nextTab()    │         │
  │                 │─────────────▶│         │
  │                 │              │  saveCurrentTabState()
  │                 │              │──────┐  │
  │                 │              │◀─────┘  │
  │                 │              │  restoreTabState(index)
  │                 │              │──────┐  │
  │                 │              │◀─────┘  │
  │                 │              │  load(url)
  │                 │              │─────────▶
  │                 │◀─────────────│         │
  │                 │  tabSwitched(index, url, title)
  │                 │──────────┐   │         │
  │                 │◀─────────┘   │         │
  │                 │  urlBar_->setText(url)
  │                 │  webView_->setFocus()
  │◀────────────────│              │         │
  │  탭 전환 완료   │              │         │
```

### 13.2 컬러 버튼 패널 열기 (Red 버튼)

```
사용자 → BrowserWindow → BookmarkPanel
  │                 │              │
  │  Red Button     │              │
  │────────────────▶│              │
  │                 │  shouldDebounce()
  │                 │──────────┐   │
  │                 │◀─────────┘   │
  │                 │  handleColorButton(RED)
  │                 │──────────┐   │
  │                 │◀─────────┘   │
  │                 │  isVisible() │
  │                 │─────────────▶│
  │                 │◀─────────────│
  │                 │  show()      │
  │                 │─────────────▶│
  │                 │  (슬라이드 인 애니메이션)
  │                 │              │
  │                 │  setFocus()  │
  │                 │─────────────▶│
  │◀────────────────│              │
  │  패널 열림      │              │
```

### 13.3 숫자 버튼 직접 탭 선택 (포커스 체크)

```
사용자 → BrowserWindow → URLBar → TabManager
  │                 │         │         │
  │  숫자 3 입력    │         │         │
  │────────────────▶│         │         │
  │                 │  urlBar_->hasFocus()
  │                 │────────▶│         │
  │                 │◀────────│         │
  │                 │  (true)  │         │
  │                 │  event->ignore()
  │                 │────────▶│         │
  │                 │  keyPressEvent('3')
  │                 │         │───┐     │
  │                 │         │◀──┘     │
  │                 │         │  inputField_->insert("3")
  │◀────────────────│         │         │
  │  URLBar에 "3" 입력        │         │
```

---

## 14. 확장성 설계

### 14.1 사용자 정의 키 매핑 (M4 이후)

**계획**:
1. 설정 화면에 "키보드 단축키 설정" 메뉴 추가
2. `KeyboardHandler` 서비스 클래스 도입
3. 키 매핑을 JSON 파일로 저장 (webOS LS2 API)
4. `KeyboardHandler::getAction(int keyCode)` 메서드로 동적 매핑

**구조**:
```cpp
class KeyboardHandler {
public:
    enum Action {
        TabPrevious,
        TabNext,
        BookmarkPanel,
        HistoryPanel,
        // ...
    };

    Action getAction(int keyCode);
    void setMapping(int keyCode, Action action);
    void loadMappingsFromStorage();
    void saveMappingsToStorage();

private:
    QMap<int, Action> customMappings_;
    QMap<int, Action> defaultMappings_;
};
```

### 14.2 키보드 단축키 튜토리얼 (M5 이후)

**계획**:
1. 첫 실행 시 오버레이 화면 표시 (반투명)
2. 주요 리모컨 버튼 그림 + 기능 설명
3. "다시 보지 않기" 체크박스

### 14.3 음성 인식 단축키 (M6 이후)

**계획**:
1. webOS Luna Service API를 사용하여 음성 인식 통합
2. "북마크 열기", "새 탭", "이전 페이지" 같은 명령어 지원
3. 음성 인식 실패 시 시각적 피드백 (토스트 메시지)

---

## 15. 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | F-13 기능 설계 시작 |
| 2026-02-14 | TabManager 다중 탭 리팩토링 결정 추가 | 요구사항에서 탭 전환 기능 필수 |
| 2026-02-14 | 디바운스 메커니즘 설계 완료 | 리모컨 물리적 반복 입력 방지 |
| 2026-02-14 | 재생 버튼 자동 스크롤 설계 (M3 이후로 연기) | 우선순위 Could 항목 |
