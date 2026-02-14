# URL 입력 UI — 기술 설계서 (Native App)

## 1. 참조
- 요구사항 분석서: docs/specs/url-input-ui/requirements.md
- WebView API: src/browser/WebView.h
- PRD: docs/project/prd.md
- 기능 백로그: docs/project/features.md
- CLAUDE.md: 프로젝트 규칙 및 코딩 컨벤션

---

## 2. 아키텍처 결정

### 결정 1: URLBar 클래스 설계 방식
- **선택지**:
  - A) QLineEdit 직접 상속 (단일 클래스)
  - B) QWidget 상속 + QLineEdit 컴포지션 (복합 위젯)
  - C) QFrame 상속 + 커스텀 페인팅
- **결정**: B) QWidget 상속 + QLineEdit 컴포지션
- **근거**:
  - URLBar는 단순 입력 필드가 아닌 복합 위젯 (입력 필드 + 보안 아이콘 + 에러 라벨 + 자동완성 팝업 등)
  - QLineEdit을 멤버로 포함하면 기능 확장이 용이 (F-09 검색 엔진, F-14 HTTPS 표시 등)
  - 레이아웃 관리가 명확해짐 (QHBoxLayout으로 내부 요소 배치)
  - QLineEdit의 기본 기능(텍스트 편집, 시그널)을 그대로 활용하면서 추가 UI 요소 통합 가능
- **트레이드오프**:
  - 포기: QLineEdit 직접 상속의 단순함, 일부 API 노출 필요 (setText, text 등을 래핑)
  - 획득: 높은 확장성, 명확한 책임 분리, 향후 기능 추가 용이

### 결정 2: 가상 키보드 구현 방식
- **선택지**:
  - A) Qt Virtual Keyboard 플러그인 사용
  - B) 커스텀 QWidget + QPushButton 그리드 (QWERTY 레이아웃)
  - C) QML 기반 커스텀 키보드
- **결정**: B) 커스텀 QWidget + QPushButton 그리드
- **근거**:
  - Qt Virtual Keyboard는 터치/마우스 입력에 최적화되어 있으며, 리모컨 5-way 방향키 커스터마이징이 어려움
  - QPushButton 그리드는 keyPressEvent를 완전히 제어 가능 (상/하/좌/우 포커스 이동 로직 구현)
  - QWERTY 레이아웃을 QGridLayout으로 구현 시 유지보수가 쉬움
  - 향후 한글 키보드 등 다국어 레이아웃 추가 시 클래스 상속으로 확장 가능
  - 리모컨 포커스 하이라이트를 QSS로 세밀하게 조정 가능
- **트레이드오프**:
  - 포기: Qt Virtual Keyboard의 완성도 높은 UI, IME 통합
  - 획득: 리모컨 최적화, 커스텀 레이아웃 자유도, 키 입력 로직 완전 제어

### 결정 3: 자동완성 UI 구현 방식
- **선택지**:
  - A) QCompleter (Qt 기본 자동완성)
  - B) QListWidget 커스텀 팝업
  - C) QTreeWidget (북마크 폴더 구조 표시)
- **결정**: B) QListWidget 커스텀 팝업
- **근거**:
  - QCompleter는 마우스 환경에 최적화되어 있으며, 리모컨 방향키 포커스 이동 커스터마이징이 제한적
  - QListWidget은 keyPressEvent를 오버라이드하여 Qt::Key_Down/Up으로 항목 이동 구현 가능
  - 각 항목에 타이틀 + URL을 표시하려면 QListWidgetItem의 setText에 두 줄 텍스트 설정 또는 커스텀 delegate 사용
  - URLBar 아래에 QFrame으로 팝업을 띄우고, 리모컨 Key_Down으로 포커스 전환 구현
  - HistoryService, BookmarkService와의 통합이 명확 (search 메서드 결과를 QListWidget에 추가)
- **트레이드오프**:
  - 포기: QCompleter의 자동 필터링 및 매칭 기능
  - 획득: 리모컨 포커스 제어, 두 줄 표시 (타이틀 + URL), 확장 가능한 UI

### 결정 4: URL 검증 및 자동 보완 로직 위치
- **선택지**:
  - A) URLBar 클래스 내부에 구현
  - B) URLValidator 유틸리티 클래스로 분리
  - C) WebView::load 메서드 내부에서 처리
- **결정**: B) URLValidator 유틸리티 클래스로 분리
- **근거**:
  - URL 검증 로직은 재사용 가능 (F-07 북마크 추가 시에도 URL 검증 필요)
  - URLBar는 UI 로직에 집중, 비즈니스 로직은 유틸리티로 분리 (SRP 원칙)
  - 단위 테스트 작성이 용이 (URLValidator 독립적으로 테스트)
  - 정규표현식 기반 도메인 검증, QUrl::fromUserInput 활용 등 복잡한 로직 캡슐화
- **트레이드오프**:
  - 포기: URLBar 내부에서 직접 처리하는 단순함
  - 획득: 재사용성, 테스트 용이성, 책임 분리

### 결정 5: BrowserWindow 레이아웃 통합 방식
- **선택지**:
  - A) QVBoxLayout에 상단부터 URLBar → NavigationBar → WebView 순서로 배치
  - B) QDockWidget으로 URLBar와 NavigationBar를 도킹 가능하게 구성
  - C) QStackedWidget으로 URLBar와 가상 키보드를 전환
- **결정**: A) QVBoxLayout에 순차 배치 (URLBar → WebView)
- **근거**:
  - 단순하고 직관적인 레이아웃 (리모컨 사용자가 이해하기 쉬운 구조)
  - NavigationBar는 별도 기능(F-04)이므로 F-03에서는 URLBar만 추가
  - QVBoxLayout::addWidget으로 URLBar를 BrowserWindow의 mainLayout_에 삽입
  - 가상 키보드는 QDialog 또는 별도 QWidget으로 모달/비모달 표시
- **트레이드오프**:
  - 포기: 복잡한 도킹 레이아웃, 동적 UI 재배치
  - 획득: 단순성, 유지보수 용이, 리모컨 포커스 경로 명확

---

## 3. 클래스 설계

### 3.1 URLBar 클래스

#### 헤더 파일 (src/ui/URLBar.h)
```cpp
#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <QUrl>
#include <QTimer>
#include <QKeyEvent>
#include <memory>

namespace webosbrowser {

// Forward declarations
class URLValidator;
class VirtualKeyboard;
class HistoryService;
class BookmarkService;

/**
 * @class URLBar
 * @brief URL 입력 바 위젯 (리모컨 최적화)
 *
 * QLineEdit 기반 URL 입력 필드와 자동완성 팝업, 에러 표시 기능 포함.
 * 리모컨 방향키로 가상 키보드 및 자동완성 리스트 탐색 가능.
 */
class URLBar : public QWidget {
    Q_OBJECT

public:
    explicit URLBar(QWidget *parent = nullptr);
    ~URLBar() override;

    URLBar(const URLBar&) = delete;
    URLBar& operator=(const URLBar&) = delete;

    /**
     * @brief 현재 입력된 URL 반환
     */
    QString text() const;

    /**
     * @brief URL 텍스트 설정 (프로그래밍 방식)
     */
    void setText(const QString &url);

    /**
     * @brief 입력 필드에 포커스 설정
     */
    void setFocusToInput();

    /**
     * @brief 에러 메시지 표시
     */
    void showError(const QString &message);

    /**
     * @brief 에러 메시지 숨김
     */
    void hideError();

    /**
     * @brief 히스토리 서비스 주입 (의존성 주입)
     */
    void setHistoryService(HistoryService *service);

    /**
     * @brief 북마크 서비스 주입 (의존성 주입)
     */
    void setBookmarkService(BookmarkService *service);

signals:
    /**
     * @brief URL 입력 완료 (Enter 키 입력 시)
     * @param url 입력된 URL (자동 보완 후)
     */
    void urlSubmitted(const QUrl &url);

    /**
     * @brief 입력 취소 (ESC 키 입력 시)
     */
    void editingCancelled();

protected:
    /**
     * @brief 키 이벤트 처리 (리모컨 방향키, Enter, ESC 등)
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 포커스 인 이벤트 (테두리 하이라이트)
     */
    void focusInEvent(QFocusEvent *event) override;

    /**
     * @brief 포커스 아웃 이벤트
     */
    void focusOutEvent(QFocusEvent *event) override;

private slots:
    /**
     * @brief 텍스트 변경 시 자동완성 검색 (디바운싱)
     */
    void onTextChanged(const QString &text);

    /**
     * @brief 자동완성 항목 선택
     */
    void onAutocompleteItemSelected(QListWidgetItem *item);

    /**
     * @brief 가상 키보드에서 문자 입력
     */
    void onKeyboardCharacterEntered(const QString &character);

    /**
     * @brief 가상 키보드 닫기
     */
    void onKeyboardClosed();

    /**
     * @brief 디바운스 타이머 타임아웃 (자동완성 검색 실행)
     */
    void onDebounceTimeout();

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

    /**
     * @brief 스타일 적용 (QSS)
     */
    void applyStyles();

    /**
     * @brief 가상 키보드 표시
     */
    void showVirtualKeyboard();

    /**
     * @brief 자동완성 팝업 표시
     */
    void showAutocompletePopup(const QStringList &suggestions);

    /**
     * @brief 자동완성 팝업 숨김
     */
    void hideAutocompletePopup();

    /**
     * @brief URL 검증 및 자동 보완
     */
    QUrl validateAndCompleteUrl(const QString &input);

    /**
     * @brief 자동완성 검색 (비동기)
     */
    void searchAutocomplete(const QString &query);

private:
    // UI 컴포넌트
    QLineEdit *inputField_;              ///< URL 입력 필드
    QLabel *errorLabel_;                 ///< 에러 메시지 라벨
    QListWidget *autocompleteList_;      ///< 자동완성 팝업 리스트
    QFrame *autocompleteFrame_;          ///< 자동완성 팝업 프레임

    // 레이아웃
    QVBoxLayout *mainLayout_;            ///< 메인 레이아웃
    QHBoxLayout *inputLayout_;           ///< 입력 필드 레이아웃

    // 서비스 및 유틸리티
    std::unique_ptr<URLValidator> validator_;          ///< URL 검증 유틸리티
    std::unique_ptr<VirtualKeyboard> virtualKeyboard_; ///< 가상 키보드
    HistoryService *historyService_;                   ///< 히스토리 서비스 (외부 주입)
    BookmarkService *bookmarkService_;                 ///< 북마크 서비스 (외부 주입)

    // 상태 관리
    QString previousUrl_;                ///< 입력 전 URL (취소 시 복원)
    QTimer *debounceTimer_;              ///< 자동완성 디바운싱 타이머
    bool isAutocompleteVisible_;         ///< 자동완성 팝업 표시 여부
};

} // namespace webosbrowser
```

### 3.2 VirtualKeyboard 클래스

#### 헤더 파일 (src/ui/VirtualKeyboard.h)
```cpp
#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QString>
#include <QVector>
#include <QKeyEvent>
#include <QPoint>

namespace webosbrowser {

/**
 * @class VirtualKeyboard
 * @brief 리모컨 최적화 가상 키보드 (QWERTY 레이아웃)
 *
 * QPushButton 그리드로 구성된 가상 키보드.
 * 리모컨 방향키로 키 간 포커스 이동, 선택 버튼으로 문자 입력.
 */
class VirtualKeyboard : public QWidget {
    Q_OBJECT

public:
    explicit VirtualKeyboard(QWidget *parent = nullptr);
    ~VirtualKeyboard() override;

    VirtualKeyboard(const VirtualKeyboard&) = delete;
    VirtualKeyboard& operator=(const VirtualKeyboard&) = delete;

signals:
    /**
     * @brief 문자 입력 시그널
     * @param character 입력된 문자 (알파벳, 숫자, 특수문자)
     */
    void characterEntered(const QString &character);

    /**
     * @brief 백스페이스 입력 시그널
     */
    void backspacePressed();

    /**
     * @brief Enter 입력 시그널
     */
    void enterPressed();

    /**
     * @brief 스페이스 입력 시그널
     */
    void spacePressed();

    /**
     * @brief 키보드 닫기 시그널 (ESC/Back 키)
     */
    void closeRequested();

protected:
    /**
     * @brief 키 이벤트 처리 (리모컨 방향키, Select 등)
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief 키 버튼 클릭 (마우스 또는 Select 키)
     */
    void onKeyButtonClicked();

private:
    /**
     * @brief UI 초기화 (그리드 레이아웃 구성)
     */
    void setupUI();

    /**
     * @brief 키 버튼 생성
     */
    QPushButton* createKeyButton(const QString &label);

    /**
     * @brief 스타일 적용 (QSS)
     */
    void applyStyles();

    /**
     * @brief 그리드 상에서 포커스 이동
     * @param direction 방향 (Qt::Key_Up, Down, Left, Right)
     */
    void moveFocusInGrid(int direction);

    /**
     * @brief 현재 포커스된 버튼의 그리드 좌표 반환
     */
    QPoint getCurrentFocusPosition() const;

private:
    QGridLayout *keyboardLayout_;        ///< 키보드 그리드 레이아웃
    QVector<QVector<QPushButton*>> keys_;///< 2D 버튼 배열 (행, 열)

    // QWERTY 레이아웃 (4행 + 제어 키)
    static constexpr int ROWS = 4;
    static constexpr int COLS = 11;
};

} // namespace webosbrowser
```

### 3.3 URLValidator 클래스 (확장)

#### 헤더 파일 (src/utils/URLValidator.h)
```cpp
#pragma once

#include <QString>
#include <QUrl>
#include <QRegularExpression>

namespace webosbrowser {

/**
 * @class URLValidator
 * @brief URL 검증 및 자동 보완 유틸리티
 */
class URLValidator {
public:
    URLValidator() = default;
    ~URLValidator() = default;

    /**
     * @brief URL 유효성 검증
     * @param url 검증할 URL 문자열
     * @return 유효하면 true
     */
    static bool isValid(const QString &url);

    /**
     * @brief URL 자동 보완 (프로토콜 추가)
     * @param input 사용자 입력 문자열
     * @return 보완된 QUrl
     */
    static QUrl autoComplete(const QString &input);

    /**
     * @brief 검색어 여부 판단 (URL이 아닌 경우)
     * @param input 사용자 입력 문자열
     * @return 검색어이면 true
     */
    static bool isSearchQuery(const QString &input);

    /**
     * @brief 도메인 형식 검증 (정규표현식)
     * @param input 도메인 문자열
     * @return 도메인 형식이면 true
     */
    static bool isDomainFormat(const QString &input);

private:
    static QRegularExpression domainRegex_;  ///< 도메인 정규표현식
};

} // namespace webosbrowser
```

---

## 4. 시퀀스 다이어그램

### 시나리오 1: URL 입력 및 페이지 로드

```
사용자 (리모컨) → BrowserWindow → URLBar → VirtualKeyboard → URLValidator → WebView
       │                │              │             │                │            │
       │ 포커스 이동     │              │             │                │            │
       │ (방향키)        │              │             │                │            │
       │───────────────▶│              │             │                │            │
       │                │ setFocusToInput()          │                │            │
       │                │─────────────▶│             │                │            │
       │                │              │ showVirtualKeyboard()         │            │
       │                │              │────────────▶│                │            │
       │                │              │             │ show()         │            │
       │                │              │             │◀───────────────│            │
       │ 방향키로 키 선택│              │             │                │            │
       │───────────────▶│              │             │                │            │
       │                │              │             │ moveFocusInGrid()          │
       │                │              │             │────────────────│            │
       │ Select 키 입력  │              │             │                │            │
       │───────────────▶│              │             │                │            │
       │                │              │             │ characterEntered("g")      │
       │                │              │◀────────────│                │            │
       │                │ onKeyboardCharacterEntered()│                │            │
       │                │              │             │                │            │
       │                │ inputField_->insert("g")   │                │            │
       │                │              │             │                │            │
       │ (반복 입력)     │              │             │                │            │
       │ "google.com"   │              │             │                │            │
       │                │ onTextChanged("google.com")│                │            │
       │                │              │             │                │            │
       │                │ searchAutocomplete(query)  │                │            │
       │                │              │             │                │            │
       │                │ (HistoryService, BookmarkService 검색)       │            │
       │                │              │             │                │            │
       │                │ showAutocompletePopup(list)│                │            │
       │                │              │             │                │            │
       │ Enter 키 입력   │              │             │                │            │
       │───────────────▶│              │             │                │            │
       │                │ keyPressEvent(Qt::Key_Enter)               │            │
       │                │              │             │                │            │
       │                │ validateAndCompleteUrl("google.com")        │            │
       │                │              │             │ autoComplete() │            │
       │                │              │             │───────────────▶│            │
       │                │              │             │ QUrl("http://google.com")  │
       │                │              │             │◀───────────────│            │
       │                │              │             │                │            │
       │                │ emit urlSubmitted(url)     │                │            │
       │                │──────────────────────────────────────────────────────────▶│
       │                │              │             │                │ load(url)  │
       │                │              │             │                │────────────│
```

### 시나리오 2: 자동완성 선택

```
사용자 → URLBar → AutocompleteList → HistoryService → WebView
   │         │             │                 │             │
   │ 텍스트 입력("goo")    │                 │             │
   │────────▶│             │                 │             │
   │         │ onTextChanged("goo")          │             │
   │         │             │                 │             │
   │         │ searchAutocomplete("goo")     │             │
   │         │             │ search("goo")   │             │
   │         │             │────────────────▶│             │
   │         │             │ [{title, url}]  │             │
   │         │             │◀────────────────│             │
   │         │ showAutocompletePopup(list)   │             │
   │         │────────────▶│                 │             │
   │         │             │ show()          │             │
   │         │             │◀────────────────│             │
   │ Key_Down│             │                 │             │
   │────────▶│ 포커스 이동 │                 │             │
   │         │────────────▶│ setFocus()      │             │
   │         │             │◀────────────────│             │
   │ Select 키│             │                 │             │
   │────────▶│ onAutocompleteItemSelected()  │             │
   │         │             │                 │             │
   │         │ setText(item->url)            │             │
   │         │             │                 │             │
   │         │ emit urlSubmitted(url)        │             │
   │         │──────────────────────────────────────────────▶│
```

### 시나리오 3: 입력 취소 (ESC)

```
사용자 → URLBar → VirtualKeyboard
   │         │             │
   │ 텍스트 입력 시작      │
   │────────▶│             │
   │         │ previousUrl_ = inputField_->text()
   │         │             │
   │ 가상 키보드 표시       │
   │         │────────────▶│
   │         │             │ show()
   │         │             │◀───────────────
   │ ESC/Back 키           │
   │────────▶│             │
   │         │ keyPressEvent(Qt::Key_Escape)
   │         │             │
   │         │ setText(previousUrl_)  // 이전 URL 복원
   │         │             │
   │         │ virtualKeyboard_->hide()
   │         │────────────▶│
   │         │             │ hide()
   │         │             │◀───────────────
   │         │ emit editingCancelled()
```

### 시나리오 4: URL 유효성 검증 실패

```
사용자 → URLBar → URLValidator → ErrorLabel
   │         │             │             │
   │ "invalid url !!!" 입력│             │
   │────────▶│             │             │
   │         │ onTextChanged(text)       │
   │         │             │             │
   │ Enter 키│             │             │
   │────────▶│ validateAndCompleteUrl(text)
   │         │────────────▶│             │
   │         │             │ isValid()   │
   │         │             │ return false│
   │         │◀────────────│             │
   │         │ showError("유효한 URL을 입력하세요")
   │         │──────────────────────────▶│
   │         │             │             │ setText(msg)
   │         │             │             │ setStyleSheet("red")
   │         │             │             │ show()
   │         │             │             │◀────────────────
```

---

## 5. 데이터 플로우 설계

### URL 입력 데이터 플로우

```
[리모컨 입력]
    │
    ▼
[keyPressEvent] → [방향키] → [VirtualKeyboard::moveFocusInGrid]
    │                              │
    │                              ▼
    │                       [QPushButton 포커스 이동]
    │
    ▼
[Qt::Key_Select] → [VirtualKeyboard::onKeyButtonClicked]
    │
    ▼
[emit characterEntered("x")] → [URLBar::onKeyboardCharacterEntered]
    │
    ▼
[inputField_->insert("x")] → [emit textChanged("...x")]
    │
    ▼
[URLBar::onTextChanged] → [debounceTimer_->start(500ms)]
    │
    ▼ (500ms 후)
[URLBar::searchAutocomplete]
    │
    ├─▶ [HistoryService::search(query)] → [LS2 API 호출]
    │       │
    │       ▼
    │   [QList<HistoryItem> results]
    │
    ├─▶ [BookmarkService::search(query)] → [LS2 API 호출]
    │       │
    │       ▼
    │   [QList<BookmarkItem> results]
    │
    ▼
[병합 및 정렬] → [최대 5개 선택]
    │
    ▼
[URLBar::showAutocompletePopup(list)]
    │
    ▼
[autocompleteList_->addItems(list)]
    │
    ▼
[autocompleteFrame_->show()]
```

### URL 제출 데이터 플로우

```
[Qt::Key_Enter] → [URLBar::keyPressEvent]
    │
    ▼
[URLBar::validateAndCompleteUrl(text)]
    │
    ├─▶ [URLValidator::isValid(text)]
    │       │
    │       ├─▶ false → [URLBar::showError("유효한 URL을 입력하세요")]
    │       │               │
    │       │               ▼
    │       │           [종료]
    │       │
    │       ▼
    │   true → [URLValidator::isDomainFormat(text)]
    │               │
    │               ├─▶ true → [URLValidator::autoComplete(text)]
    │               │               │
    │               │               ├─▶ [QUrl::scheme().isEmpty()?]
    │               │               │       │
    │               │               │       ├─▶ true → [url.setScheme("http")]
    │               │               │       │
    │               │               │       ▼
    │               │               │   false → [url 그대로]
    │               │               │
    │               │               ▼
    │               │           [return QUrl]
    │               │
    │               ▼
    │           false → [URLValidator::isSearchQuery(text)]
    │                       │
    │                       ▼
    │                   true → [F-09 SearchEngine으로 전달]
    │                               │
    │                               ▼
    │                           [return searchEngineUrl]
    │
    ▼
[emit urlSubmitted(url)] → [BrowserWindow::onUrlSubmitted]
    │
    ▼
[webView_->load(url)] → [WebView::load(url)]
    │
    ├─▶ [emit loadStarted()] → [LoadingBar 표시 (F-05)]
    │
    ├─▶ [QWebEngineView::setUrl(url)]
    │
    ▼
[페이지 로딩 시작]
```

---

## 6. 영향 범위 분석

### 6.1 수정 필요한 기존 파일

#### src/browser/BrowserWindow.h / BrowserWindow.cpp
- **변경 내용**:
  - `URLBar *urlBar_` 멤버 추가
  - `setupUI()` 메서드에서 URLBar 인스턴스 생성 및 레이아웃 추가
  - `setupConnections()` 메서드에서 URLBar::urlSubmitted 시그널과 WebView::load 슬롯 연결
  - WebView::urlChanged 시그널과 URLBar::setText 연결 (현재 URL 표시)
- **예시 코드**:
  ```cpp
  // BrowserWindow.cpp
  void BrowserWindow::setupUI() {
      centralWidget_ = new QWidget(this);
      mainLayout_ = new QVBoxLayout(centralWidget_);

      // URLBar 추가
      urlBar_ = new URLBar(this);
      mainLayout_->addWidget(urlBar_);

      // WebView
      webView_ = new WebView(this);
      mainLayout_->addWidget(webView_);

      // StatusLabel
      statusLabel_ = new QLabel(this);
      mainLayout_->addWidget(statusLabel_);

      setCentralWidget(centralWidget_);
  }

  void BrowserWindow::setupConnections() {
      // URLBar → WebView
      connect(urlBar_, &URLBar::urlSubmitted,
              webView_, static_cast<void(WebView::*)(const QUrl&)>(&WebView::load));

      // WebView → URLBar (현재 URL 표시)
      connect(webView_, &WebView::urlChanged,
              urlBar_, &URLBar::setText);

      // WebView → StatusLabel (로딩 상태 표시)
      connect(webView_, &WebView::loadStarted,
              statusLabel_, []{ statusLabel_->setText("Loading..."); });
      connect(webView_, &WebView::loadFinished,
              statusLabel_, [](bool success){
                  statusLabel_->setText(success ? "Loaded" : "Failed");
              });
  }
  ```

#### src/utils/URLValidator.h / URLValidator.cpp
- **변경 내용**:
  - 스켈레톤 코드를 완전한 구현체로 확장
  - `isValid`, `autoComplete`, `isSearchQuery`, `isDomainFormat` 정적 메서드 추가
  - 정규표현식 기반 도메인 검증 구현
- **영향**: 기존 파일이 거의 비어있으므로 충돌 없음

#### CMakeLists.txt
- **변경 내용**:
  - 새 파일 추가 (VirtualKeyboard.cpp, VirtualKeyboard.h)
  - `SOURCES`에 `src/ui/VirtualKeyboard.cpp` 추가
  - `HEADERS`에 `src/ui/VirtualKeyboard.h` 추가
- **예시**:
  ```cmake
  set(SOURCES
      # ...
      src/ui/URLBar.cpp
      src/ui/VirtualKeyboard.cpp  # 추가
  )

  set(HEADERS
      # ...
      src/ui/URLBar.h
      src/ui/VirtualKeyboard.h    # 추가
  )
  ```

### 6.2 새로 생성할 파일

#### src/ui/VirtualKeyboard.h / VirtualKeyboard.cpp
- **역할**: 리모컨 최적화 가상 키보드 구현
- **의존성**: Qt Widgets (QPushButton, QGridLayout)

#### src/ui/URLBar.cpp
- **역할**: URLBar 클래스 구현
- **의존성**: VirtualKeyboard, URLValidator, HistoryService, BookmarkService

#### src/utils/URLValidator.cpp
- **역할**: URL 검증 및 자동 보완 로직 구현
- **의존성**: Qt Core (QUrl, QRegularExpression)

#### resources/styles/urlbar.qss (선택사항)
- **역할**: URLBar 및 VirtualKeyboard QSS 스타일 정의
- **내용**:
  ```css
  /* URLBar 포커스 스타일 */
  URLBar QLineEdit:focus {
      border: 3px solid #00aaff;
      background: #2a2a2a;
      color: #ffffff;
  }

  /* 에러 라벨 스타일 */
  URLBar QLabel#errorLabel {
      color: #ff4444;
      font-size: 14px;
  }

  /* 가상 키보드 버튼 스타일 */
  VirtualKeyboard QPushButton {
      min-width: 60px;
      min-height: 60px;
      font-size: 20px;
      background: #3a3a3a;
      color: #ffffff;
      border: 2px solid #555555;
      border-radius: 5px;
  }

  VirtualKeyboard QPushButton:focus {
      border: 3px solid #00aaff;
      background: #4a4a4a;
  }

  VirtualKeyboard QPushButton:pressed {
      background: #1a1a1a;
  }

  /* 자동완성 리스트 스타일 */
  URLBar QListWidget {
      background: #2a2a2a;
      color: #ffffff;
      font-size: 16px;
      border: 2px solid #555555;
  }

  URLBar QListWidget::item:selected {
      background: #00aaff;
  }
  ```

### 6.3 영향 받는 기존 기능

#### F-02 (웹뷰 통합)
- **영향**: URLBar에서 emit한 urlSubmitted 시그널을 WebView::load 슬롯에 연결
- **조치**: BrowserWindow::setupConnections()에서 시그널/슬롯 연결 추가

#### F-09 (검색 엔진 통합) - 향후 기능
- **영향**: URLValidator::isSearchQuery()가 true를 반환하면 SearchEngine 서비스로 전달
- **조치**: F-09 구현 시 URLBar::validateAndCompleteUrl() 메서드 수정 필요

#### F-14 (HTTPS 보안 표시) - 향후 기능
- **영향**: URLBar에 보안 아이콘 추가 (QLabel 또는 QPushButton)
- **조치**: F-14 구현 시 URLBar::inputLayout_에 securityIcon_ 위젯 추가

---

## 7. API 명세

### URLBar Public API

#### 메서드

| 메서드 | 파라미터 | 반환값 | 설명 |
|--------|----------|--------|------|
| `text()` | - | `QString` | 현재 입력된 URL 반환 |
| `setText(url)` | `const QString &url` | `void` | URL 텍스트 설정 (프로그래밍 방식) |
| `setFocusToInput()` | - | `void` | 입력 필드에 포커스 설정 |
| `showError(message)` | `const QString &message` | `void` | 에러 메시지 표시 |
| `hideError()` | - | `void` | 에러 메시지 숨김 |
| `setHistoryService(service)` | `HistoryService *service` | `void` | 히스토리 서비스 주입 |
| `setBookmarkService(service)` | `BookmarkService *service` | `void` | 북마크 서비스 주입 |

#### 시그널

| 시그널 | 파라미터 | 설명 |
|--------|----------|------|
| `urlSubmitted` | `const QUrl &url` | URL 입력 완료 (Enter 키 입력 시) |
| `editingCancelled` | - | 입력 취소 (ESC 키 입력 시) |

### VirtualKeyboard Public API

#### 시그널

| 시그널 | 파라미터 | 설명 |
|--------|----------|------|
| `characterEntered` | `const QString &character` | 문자 입력 시그널 |
| `backspacePressed` | - | 백스페이스 입력 시그널 |
| `enterPressed` | - | Enter 입력 시그널 |
| `spacePressed` | - | 스페이스 입력 시그널 |
| `closeRequested` | - | 키보드 닫기 시그널 |

### URLValidator Public API

#### 정적 메서드

| 메서드 | 파라미터 | 반환값 | 설명 |
|--------|----------|--------|------|
| `isValid(url)` | `const QString &url` | `bool` | URL 유효성 검증 |
| `autoComplete(input)` | `const QString &input` | `QUrl` | URL 자동 보완 (프로토콜 추가) |
| `isSearchQuery(input)` | `const QString &input` | `bool` | 검색어 여부 판단 |
| `isDomainFormat(input)` | `const QString &input` | `bool` | 도메인 형식 검증 |

---

## 8. 포커스 네비게이션 설계

### 포커스 경로 (리모컨 방향키)

```
[BrowserWindow]
    │
    ├─▶ [URLBar::inputField_] ◀───┐
    │       │                      │
    │       │ (편집 모드 진입)     │
    │       │                      │
    │       ▼                      │
    │   [VirtualKeyboard]          │
    │       │                      │
    │       │ (그리드 내 포커스 이동)
    │       │  상/하/좌/우         │
    │       │                      │
    │       │ (Enter 또는 ESC)     │
    │       │                      │
    │       ▼                      │
    │   [URLBar::inputField_] ─────┘
    │       │
    │       │ (Key_Down)
    │       │
    │       ▼
    │   [autocompleteList_] ◀──┐
    │       │                  │
    │       │ (상/하 항목 선택)│
    │       │                  │
    │       │ (Select 키)      │
    │       │                  │
    │       └──────────────────┘
    │       │
    │       │ (Enter 키)
    │       │
    │       ▼
    │   [WebView] (URL 로드)
    │
    └─▶ [NavigationBar] (F-04)
    │
    └─▶ [WebView]
```

### keyPressEvent 구현 로직

#### URLBar::keyPressEvent
```cpp
void URLBar::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            // URL 검증 및 제출
            {
                QUrl url = validateAndCompleteUrl(inputField_->text());
                if (!url.isEmpty()) {
                    hideAutocompletePopup();
                    virtualKeyboard_->hide();
                    emit urlSubmitted(url);
                }
            }
            break;

        case Qt::Key_Escape:
        case Qt::Key_Back:
            // 입력 취소, 이전 URL 복원
            inputField_->setText(previousUrl_);
            hideAutocompletePopup();
            virtualKeyboard_->hide();
            emit editingCancelled();
            break;

        case Qt::Key_Down:
            // 자동완성 리스트로 포커스 이동
            if (isAutocompleteVisible_ && autocompleteList_->count() > 0) {
                autocompleteList_->setFocus();
                autocompleteList_->setCurrentRow(0);
            }
            break;

        case Qt::Key_Select:
            // 가상 키보드 표시
            if (!virtualKeyboard_->isVisible()) {
                previousUrl_ = inputField_->text();  // 현재 URL 저장
                showVirtualKeyboard();
            }
            break;

        default:
            QWidget::keyPressEvent(event);
            break;
    }
}
```

#### VirtualKeyboard::keyPressEvent
```cpp
void VirtualKeyboard::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
            // 그리드 내 포커스 이동
            moveFocusInGrid(event->key());
            break;

        case Qt::Key_Select:
        case Qt::Key_Enter:
        case Qt::Key_Return:
            // 현재 포커스된 버튼 클릭 시뮬레이션
            {
                QPushButton *focusedButton = qobject_cast<QPushButton*>(focusWidget());
                if (focusedButton) {
                    focusedButton->click();
                }
            }
            break;

        case Qt::Key_Escape:
        case Qt::Key_Back:
            // 키보드 닫기
            emit closeRequested();
            hide();
            break;

        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void VirtualKeyboard::moveFocusInGrid(int direction) {
    QPoint currentPos = getCurrentFocusPosition();
    int row = currentPos.y();
    int col = currentPos.x();

    switch (direction) {
        case Qt::Key_Up:
            row = (row - 1 + ROWS) % ROWS;  // 순환 이동
            break;
        case Qt::Key_Down:
            row = (row + 1) % ROWS;
            break;
        case Qt::Key_Left:
            col = (col - 1 + COLS) % COLS;
            break;
        case Qt::Key_Right:
            col = (col + 1) % COLS;
            break;
    }

    // 새 위치로 포커스 이동
    if (keys_[row][col]) {
        keys_[row][col]->setFocus();
    }
}
```

---

## 9. 가상 키보드 레이아웃 설계

### QWERTY 레이아웃 (4행 + 제어 키)

```
[행 0]  1  2  3  4  5  6  7  8  9  0  -
[행 1]  q  w  e  r  t  y  u  i  o  p  /
[행 2]  a  s  d  f  g  h  j  k  l  :  .
[행 3]  z  x  c  v  b  n  m  ?  &  =  _
[제어]  [Space]  [Backspace]  [Enter]  [Cancel]
```

### 구현 코드 (VirtualKeyboard::setupUI)

```cpp
void VirtualKeyboard::setupUI() {
    keyboardLayout_ = new QGridLayout(this);
    keyboardLayout_->setSpacing(5);

    // 행 0: 숫자 + 특수문자
    QStringList row0 = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-"};
    keys_.append(QVector<QPushButton*>());
    for (int col = 0; col < row0.size(); ++col) {
        QPushButton *btn = createKeyButton(row0[col]);
        keys_[0].append(btn);
        keyboardLayout_->addWidget(btn, 0, col);
    }

    // 행 1: QWERTY (q-p) + /
    QStringList row1 = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "/"};
    keys_.append(QVector<QPushButton*>());
    for (int col = 0; col < row1.size(); ++col) {
        QPushButton *btn = createKeyButton(row1[col]);
        keys_[1].append(btn);
        keyboardLayout_->addWidget(btn, 1, col);
    }

    // 행 2: ASDFG (a-l) + : .
    QStringList row2 = {"a", "s", "d", "f", "g", "h", "j", "k", "l", ":", "."};
    keys_.append(QVector<QPushButton*>());
    for (int col = 0; col < row2.size(); ++col) {
        QPushButton *btn = createKeyButton(row2[col]);
        keys_[2].append(btn);
        keyboardLayout_->addWidget(btn, 2, col);
    }

    // 행 3: ZXCV (z-m) + 특수문자
    QStringList row3 = {"z", "x", "c", "v", "b", "n", "m", "?", "&", "=", "_"};
    keys_.append(QVector<QPushButton*>());
    for (int col = 0; col < row3.size(); ++col) {
        QPushButton *btn = createKeyButton(row3[col]);
        keys_[3].append(btn);
        keyboardLayout_->addWidget(btn, 3, col);
    }

    // 제어 키 (행 4)
    QPushButton *spaceBtn = createKeyButton("Space");
    spaceBtn->setMinimumWidth(200);  // 스페이스는 넓게
    keyboardLayout_->addWidget(spaceBtn, 4, 0, 1, 4);  // 4칸 차지
    connect(spaceBtn, &QPushButton::clicked, this, &VirtualKeyboard::spacePressed);

    QPushButton *backspaceBtn = createKeyButton("⌫");
    keyboardLayout_->addWidget(backspaceBtn, 4, 4, 1, 2);
    connect(backspaceBtn, &QPushButton::clicked, this, &VirtualKeyboard::backspacePressed);

    QPushButton *enterBtn = createKeyButton("Enter");
    keyboardLayout_->addWidget(enterBtn, 4, 6, 1, 3);
    connect(enterBtn, &QPushButton::clicked, this, &VirtualKeyboard::enterPressed);

    QPushButton *cancelBtn = createKeyButton("Cancel");
    keyboardLayout_->addWidget(cancelBtn, 4, 9, 1, 2);
    connect(cancelBtn, &QPushButton::clicked, this, &VirtualKeyboard::closeRequested);

    // 첫 번째 버튼에 초기 포커스
    if (!keys_.isEmpty() && !keys_[0].isEmpty()) {
        keys_[0][0]->setFocus();
    }
}
```

---

## 10. 자동완성 검색 로직

### 검색 흐름

```cpp
void URLBar::searchAutocomplete(const QString &query) {
    if (query.length() < 2) {  // 최소 2글자 이상
        hideAutocompletePopup();
        return;
    }

    QStringList suggestions;

    // 1. HistoryService에서 검색 (F-08 완료 후)
    if (historyService_) {
        // QList<HistoryItem> historyResults = historyService_->search(query);
        // for (const auto &item : historyResults) {
        //     suggestions.append(QString("%1\n%2").arg(item.title, item.url));
        // }
    }

    // 2. BookmarkService에서 검색 (F-07 완료 후)
    if (bookmarkService_) {
        // QList<BookmarkItem> bookmarkResults = bookmarkService_->search(query);
        // for (const auto &item : bookmarkResults) {
        //     suggestions.append(QString("%1\n%2").arg(item.title, item.url));
        // }
    }

    // 3. 중복 제거 및 정렬
    suggestions.removeDuplicates();
    suggestions.sort();

    // 4. 최대 5개로 제한
    if (suggestions.size() > 5) {
        suggestions = suggestions.mid(0, 5);
    }

    // 5. 자동완성 팝업 표시
    if (!suggestions.isEmpty()) {
        showAutocompletePopup(suggestions);
    } else {
        hideAutocompletePopup();
    }
}
```

### 디바운싱 구현

```cpp
void URLBar::onTextChanged(const QString &text) {
    // 에러 라벨 숨김
    hideError();

    // 디바운싱: 500ms 동안 추가 입력이 없을 때만 검색
    debounceTimer_->stop();
    debounceTimer_->start(500);  // 500ms 후 timeout 시그널 발생
}

// setupConnections()에서
connect(debounceTimer_, &QTimer::timeout, this, &URLBar::onDebounceTimeout);

void URLBar::onDebounceTimeout() {
    QString query = inputField_->text();
    searchAutocomplete(query);
}
```

---

## 11. 기술적 주의사항

### 리모컨 키 이벤트 처리
- webOS 리모컨 키 코드는 Qt::Key enum과 매핑되어 있음
- Qt::Key_Select는 webOS 리모컨의 중앙 선택 버튼
- Qt::Key_Back은 webOS 리모컨의 뒤로 버튼
- 포커스 체인을 명확히 하여 사용자가 혼란스럽지 않도록 주의

### QSS 스타일 주의사항
- QSS는 런타임에 동적으로 변경 가능하지만, 성능 고려하여 초기화 시 한 번만 로드
- `:focus` pseudo-state는 Qt 이벤트 루프에서 자동 처리
- 복잡한 스타일은 QSS 파일로 분리 (resources/styles/urlbar.qss)
- `setObjectName()`을 사용하여 QSS 선택자로 위젯 식별 (#urlBar, #errorLabel 등)

### 메모리 관리
- VirtualKeyboard는 URLBar의 멤버로 `std::unique_ptr`로 관리 (싱글톤 아님, URLBar 소멸 시 자동 삭제)
- HistoryService, BookmarkService는 BrowserWindow에서 생성 후 URLBar에 주입 (의존성 주입)
- QPushButton 배열은 QGridLayout이 소유권 관리 (QObject 부모-자식 관계)

### LS2 API 비동기 호출
- HistoryService::search(), BookmarkService::search()는 LS2 API 호출 시 비동기 처리 필요
- Qt Concurrent 또는 QThread를 사용하여 메인 스레드 블로킹 방지
- 검색 결과가 준비되면 시그널/슬롯으로 URLBar에 전달

### URL 자동 보완 규칙
- 프로토콜 없는 입력 → `http://` 자동 추가 (예: `google.com` → `http://google.com`)
- 공백 포함 또는 도메인 패턴 미일치 → 검색어로 판단 (F-09 SearchEngine으로 전달)
- `https://`가 입력되면 그대로 유지
- 로컬 IP (예: `192.168.1.1`) → 도메인으로 인정, `http://192.168.1.1`로 변환

### 에러 처리
- URL 검증 실패 시 errorLabel_에 메시지 표시, inputField_ 테두리를 빨간색으로 변경
- WebView::loadError 시그널 수신 시 URLBar::showError() 호출 (BrowserWindow에서 연결)
- 에러 메시지는 3초 후 자동으로 숨김 (QTimer::singleShot)

### 접근성
- 포커스된 요소는 3px 두께 테두리로 명확히 표시 (WCAG 기준)
- 폰트 크기는 3m 거리에서 가독 가능하도록 최소 18px (URL 입력), 20px (가상 키보드)
- 색상 대비 비율 4.5:1 이상 유지 (다크 모드 기본)

---

## 12. 테스트 계획

### 단위 테스트 (Google Test)

#### URLValidator 테스트
```cpp
TEST(URLValidatorTest, ValidURL) {
    EXPECT_TRUE(URLValidator::isValid("http://google.com"));
    EXPECT_TRUE(URLValidator::isValid("https://example.com/path"));
    EXPECT_FALSE(URLValidator::isValid("invalid url !!!"));
}

TEST(URLValidatorTest, AutoComplete) {
    QUrl url = URLValidator::autoComplete("google.com");
    EXPECT_EQ(url.toString(), "http://google.com");

    url = URLValidator::autoComplete("https://example.com");
    EXPECT_EQ(url.toString(), "https://example.com");
}

TEST(URLValidatorTest, SearchQuery) {
    EXPECT_TRUE(URLValidator::isSearchQuery("hello world"));
    EXPECT_FALSE(URLValidator::isSearchQuery("google.com"));
}
```

#### VirtualKeyboard 테스트
```cpp
TEST(VirtualKeyboardTest, CharacterEntered) {
    VirtualKeyboard keyboard;
    QSignalSpy spy(&keyboard, &VirtualKeyboard::characterEntered);

    // 첫 번째 버튼 (1) 클릭 시뮬레이션
    QTest::keyClick(&keyboard, Qt::Key_Select);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), "1");
}

TEST(VirtualKeyboardTest, FocusNavigation) {
    VirtualKeyboard keyboard;
    // 초기 포커스: keys_[0][0]

    QTest::keyClick(&keyboard, Qt::Key_Right);
    // 포커스: keys_[0][1]로 이동

    // 현재 포커스된 버튼 확인
    QPushButton *focusedBtn = qobject_cast<QPushButton*>(keyboard.focusWidget());
    ASSERT_NE(focusedBtn, nullptr);
    EXPECT_EQ(focusedBtn->text(), "2");
}
```

### 통합 테스트 (Qt Test)

#### URLBar + WebView 통합
```cpp
TEST(URLBarIntegrationTest, SubmitURL) {
    BrowserWindow window;
    URLBar *urlBar = window.findChild<URLBar*>();
    WebView *webView = window.findChild<WebView*>();

    QSignalSpy spy(webView, &WebView::loadStarted);

    // URL 입력 및 제출
    urlBar->setText("google.com");
    QTest::keyClick(urlBar, Qt::Key_Enter);

    // WebView::load 호출 확인
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(webView->url().toString(), "http://google.com");
}
```

### 리모컨 키 이벤트 테스트
```cpp
TEST(RemoteControlTest, KeyNavigation) {
    URLBar urlBar;
    urlBar.setFocusToInput();

    // Select 키로 가상 키보드 표시
    QTest::keyClick(&urlBar, Qt::Key_Select);

    VirtualKeyboard *keyboard = urlBar.findChild<VirtualKeyboard*>();
    ASSERT_NE(keyboard, nullptr);
    EXPECT_TRUE(keyboard->isVisible());

    // ESC 키로 키보드 닫기
    QTest::keyClick(&urlBar, Qt::Key_Escape);
    EXPECT_FALSE(keyboard->isVisible());
}
```

---

## 13. 성능 최적화

### 자동완성 검색 최적화
- **디바운싱**: 500ms 지연으로 연속 입력 시 검색 횟수 감소
- **비동기 검색**: LS2 API 호출을 QThread 또는 QtConcurrent로 비동기 처리
- **최대 5개 제한**: 자동완성 결과를 5개로 제한하여 렌더링 부하 감소
- **캐싱**: 최근 검색 결과를 메모리에 캐싱 (QMap<QString, QStringList>)

### 가상 키보드 렌더링 최적화
- **사전 생성**: VirtualKeyboard를 URLBar 생성 시 미리 생성 (지연 로딩 아님)
- **hide()로 숨김**: 매번 delete/new 하지 않고 show/hide로 재사용
- **QSS 최소화**: 복잡한 그라데이션, 그림자 효과 지양 (단색 배경)

### 메모리 사용량
- URLBar + VirtualKeyboard + 자동완성 리스트 총 메모리: 약 10~20MB 예상
- QPushButton 44개 (키보드) + QListWidget (5개 항목) = 경량

---

## 14. 확장 계획

### F-09 (검색 엔진 통합) 연계
- URLValidator::isSearchQuery()가 true를 반환하면 SearchEngine::createSearchUrl(query, engine) 호출
- URLBar::validateAndCompleteUrl() 메서드 수정:
  ```cpp
  QUrl URLBar::validateAndCompleteUrl(const QString &input) {
      if (URLValidator::isDomainFormat(input)) {
          return URLValidator::autoComplete(input);
      } else if (URLValidator::isSearchQuery(input)) {
          // F-09: SearchEngine 연동
          QString searchUrl = SearchEngine::createSearchUrl(input, "google");
          return QUrl(searchUrl);
      } else {
          showError("유효한 URL 또는 검색어를 입력하세요");
          return QUrl();
      }
  }
  ```

### F-14 (HTTPS 보안 표시) 연계
- URLBar::inputLayout_에 securityIcon_ (QLabel) 추가:
  ```cpp
  // URLBar.h
  QLabel *securityIcon_;  ///< 보안 아이콘 (자물쇠)

  // URLBar.cpp
  void URLBar::setupUI() {
      inputLayout_ = new QHBoxLayout();

      securityIcon_ = new QLabel(this);
      securityIcon_->setPixmap(QPixmap(":/icons/lock.png"));
      securityIcon_->hide();  // 초기에는 숨김
      inputLayout_->addWidget(securityIcon_);

      inputField_ = new QLineEdit(this);
      inputLayout_->addWidget(inputField_);

      mainLayout_->addLayout(inputLayout_);
  }

  // BrowserWindow.cpp (시그널 연결)
  connect(webView_, &WebView::urlChanged, this, [this](const QUrl &url) {
      if (url.scheme() == "https") {
          urlBar_->securityIcon_->show();
      } else {
          urlBar_->securityIcon_->hide();
      }
  });
  ```

### 다국어 키보드 (M3 이후)
- VirtualKeyboard를 추상 클래스로 변경
- EnglishKeyboard, KoreanKeyboard, JapaneseKeyboard 파생 클래스 구현
- 설정에서 키보드 언어 선택 (F-11)
- URLBar에서 현재 설정된 키보드 인스턴스 생성

---

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | F-03 URL 입력 UI 기술 설계 (Native App Qt/C++) |
