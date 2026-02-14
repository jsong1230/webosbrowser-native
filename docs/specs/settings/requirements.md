# F-11: 설정 화면 — 요구사항 분석서

## 1. 개요

### 기능명
설정 화면 (Settings Panel)

### 기능 ID
F-11

### 목적
사용자가 브라우저 동작을 개인화할 수 있는 설정 화면을 제공합니다. 검색 엔진 선택, 홈페이지 URL, 테마(다크/라이트), 브라우징 데이터 삭제 등의 기능을 리모컨 최적화 UI로 제공하여 사용성을 향상시킵니다.

### 대상 사용자
- 기본 검색 엔진을 변경하고 싶은 사용자 (Google ↔ Naver)
- 홈 버튼 클릭 시 이동할 페이지를 설정하고 싶은 사용자
- 다크/라이트 테마로 시청 환경에 맞게 UI를 변경하고 싶은 사용자
- 북마크, 히스토리 등 브라우징 데이터를 삭제하고 싶은 사용자

### 요청 배경
- F-09 (검색 엔진 통합)에서 검색 엔진 선택 기능이 구현되었으나, 설정 UI가 없어 변경 불가
- NavigationBar의 홈 버튼이 하드코딩된 URL("https://www.google.com")을 사용하여 사용자 커스터마이징 불가
- 프로젝터 사용 환경(어두운 방 vs 밝은 방)에 따라 테마 변경 필요
- 개인정보 보호를 위해 북마크, 히스토리 삭제 기능 제공 필요
- F-13 (리모컨 단축키)에서 Menu 버튼이 설정 패널을 여는 것으로 설계됨

---

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: 검색 엔진 선택

- **설명**: 사용자가 기본 검색 엔진을 Google, Naver, Bing, DuckDuckGo 중에서 선택할 수 있습니다.
- **유저 스토리**: As a 사용자, I want 내가 선호하는 검색 엔진을 기본으로 설정하고 싶다, so that URLBar에서 검색어 입력 시 선호하는 검색 결과를 볼 수 있다.
- **우선순위**: Must
- **상세 요구사항**:
  - **지원 검색 엔진**: Google, Naver, Bing, DuckDuckGo (F-09 SearchEngine 클래스에서 제공)
  - **UI 컴포넌트**: QRadioButton 또는 QComboBox
    - 리모컨 최적화를 위해 QRadioButton 권장 (방향키로 선택)
    - 각 검색 엔진 이름 표시 (예: "Google", "Naver")
  - **기본값**: "google" (F-09 SearchEngine::DEFAULT_ENGINE_ID)
  - **설정 변경 흐름**:
    1. 사용자가 라디오 버튼에서 검색 엔진 선택
    2. `SearchEngine::setDefaultSearchEngine(engineId)` 호출 → LS2 API 저장
    3. 즉시 다음 검색에 반영 (URLBar에서 검색어 입력 시 변경된 검색 엔진 사용)
    4. 앱 재시작 후에도 설정 유지 (`SearchEngine::getDefaultSearchEngine()` 로드)
  - **LS2 저장 키**: `defaultSearchEngine` (SearchEngine::STORAGE_KEY)

### FR-2: 홈페이지 설정

- **설명**: 사용자가 NavigationBar의 홈 버튼 클릭 시 이동할 URL을 설정할 수 있습니다.
- **유저 스토리**: As a 사용자, I want 홈 버튼을 눌렀을 때 내가 자주 가는 사이트로 이동하고 싶다, so that 매번 URL을 입력하지 않아도 된다.
- **우선순위**: Must
- **상세 요구사항**:
  - **UI 컴포넌트**: QLineEdit (URL 입력 필드)
    - Placeholder: "예: https://www.google.com"
    - 리모컨 가상 키보드로 입력 (URLBar와 동일한 방식)
  - **기본값**:
    - 초기값: "about:blank" (빈 페이지) 또는 "https://www.google.com"
    - F-15 (즐겨찾기 홈 화면) 구현 시: "about:favorites" (특수 URL로 홈 화면 표시)
  - **URL 검증**: URLValidator::isValid() 사용
    - 유효하지 않은 URL 입력 시 에러 메시지 표시 (QLabel 또는 QMessageBox)
    - 프로토콜 없는 입력(예: "google.com") → 자동으로 "http://" 추가
  - **설정 변경 흐름**:
    1. 사용자가 QLineEdit에 URL 입력
    2. "저장" 버튼 클릭 또는 Enter 키 입력
    3. URLValidator::isValid() 검증
    4. 유효하면 SettingsService::setHomepage(url) 호출 → LS2 API 저장
    5. NavigationBar::onHomeClicked()에서 설정된 URL 로드
  - **LS2 저장 키**: `homepage` (SettingsService에서 관리)

### FR-3: 테마 설정 (다크/라이트 모드)

- **설명**: 사용자가 브라우저 UI의 테마를 다크 모드 또는 라이트 모드로 전환할 수 있습니다.
- **유저 스토리**: As a 사용자, I want 어두운 방에서는 다크 모드를, 밝은 방에서는 라이트 모드를 사용하고 싶다, so that 눈의 피로를 줄일 수 있다.
- **우선순위**: Must
- **상세 요구사항**:
  - **지원 테마**: "dark", "light"
  - **UI 컴포넌트**: QRadioButton 또는 토글 스위치 (QCheckBox + 커스텀 스타일)
    - 옵션: "다크 모드", "라이트 모드"
  - **기본값**: "dark" (프로젝터는 주로 어두운 환경에서 사용)
  - **테마 적용 범위**:
    - BrowserWindow 배경색
    - NavigationBar, URLBar, TabBar 배경색 및 텍스트 색상
    - SettingsPanel, BookmarkPanel, HistoryPanel 배경색
    - 버튼, 입력 필드 스타일
  - **색상 정의** (QSS 스타일시트):
    ```cpp
    // Dark Mode
    background-color: #1E1E1E;  // 브라우저 배경
    color: #FFFFFF;             // 텍스트
    border-color: #444444;      // 테두리

    // Light Mode
    background-color: #FFFFFF;  // 브라우저 배경
    color: #000000;             // 텍스트
    border-color: #CCCCCC;      // 테두리
    ```
  - **설정 변경 흐름**:
    1. 사용자가 테마 라디오 버튼 선택
    2. `SettingsService::setTheme(theme)` 호출 → LS2 API 저장
    3. `emit themeChanged(theme)` 시그널 발생
    4. BrowserWindow::onThemeChanged(theme) 슬롯에서 모든 컴포넌트에 새 스타일시트 적용
    5. 재시작 없이 즉시 UI 업데이트
  - **LS2 저장 키**: `theme`
  - **앱 시작 시 로드**:
    - BrowserWindow 생성 시 `SettingsService::getTheme()` 호출
    - 저장된 테마로 초기 스타일시트 적용

### FR-4: 브라우징 데이터 삭제

- **설명**: 사용자가 북마크 전체 또는 히스토리 전체를 삭제할 수 있습니다.
- **유저 스토리**: As a 사용자, I want 개인정보 보호를 위해 브라우징 데이터를 삭제하고 싶다, so that 다른 사람이 내 방문 기록을 볼 수 없다.
- **우선순위**: Must
- **상세 요구사항**:
  - **삭제 대상**:
    - 북마크 전체 삭제 (BookmarkService::deleteAllBookmarks())
    - 히스토리 전체 삭제 (HistoryService::deleteAllHistory())
  - **UI 컴포넌트**: QPushButton
    - 버튼 텍스트: "북마크 전체 삭제", "히스토리 전체 삭제"
    - 버튼 색상: 빨간색 계열 (경고 표시)
  - **확인 다이얼로그**: QMessageBox
    - 제목: "경고"
    - 메시지: "모든 북마크를 삭제하시겠습니까? 이 작업은 되돌릴 수 없습니다."
    - 버튼: "삭제" (QMessageBox::Yes), "취소" (QMessageBox::No)
    - 리모컨 최적화: Yes/No 버튼 간 방향키로 이동, Enter로 선택
  - **설정 변경 흐름**:
    1. 사용자가 "북마크 전체 삭제" 버튼 클릭
    2. QMessageBox 확인 다이얼로그 표시
    3. "삭제" 선택 시:
       - `BookmarkService::deleteAllBookmarks()` 호출
       - LS2 API에서 북마크 데이터 삭제
       - BookmarkPanel 자동 새로고침 (북마크 목록 비워짐)
       - 성공 메시지 표시 (QMessageBox::information 또는 QLabel)
    4. "취소" 선택 시 아무 동작 없음
  - **히스토리 삭제도 동일한 플로우**
  - **주의사항**: 실행 취소(Undo) 기능 없음 → 확인 다이얼로그에서 명확히 경고

### FR-5: 설정 패널 접근 및 네비게이션

- **설명**: 설정 패널을 열고, 리모컨 방향키로 항목 간 이동하고, 변경사항을 저장하는 UI 흐름을 제공합니다.
- **유저 스토리**: As a 사용자, I want 리모컨만으로 모든 설정 항목에 접근하고 변경할 수 있기를 원한다, so that 마우스나 터치 없이 편리하게 설정할 수 있다.
- **우선순위**: Must
- **상세 요구사항**:
  - **패널 오픈 방법**:
    1. NavigationBar의 "설정" 버튼 클릭 (기존 TODO 구현)
    2. 리모컨 Menu 버튼 (F-13에서 이미 BrowserWindow::handleMenuButton() 구현됨)
  - **패널 타입**: QWidget 기반 오버레이 패널 또는 독립 QDialog
    - 오버레이 권장: BrowserWindow 위에 슬라이드 인 애니메이션 (QPropertyAnimation)
    - 배경: 반투명 검은색 배경 (rgba(0, 0, 0, 0.7)) + 설정 패널 (중앙 정렬)
  - **패널 레이아웃**:
    ```
    ┌─────────────────────────────────────────┐
    │          설정 (Settings)                 │
    ├─────────────────────────────────────────┤
    │  검색 엔진:                              │
    │    ◉ Google  ○ Naver  ○ Bing           │
    ├─────────────────────────────────────────┤
    │  홈페이지 URL:                           │
    │    [https://www.google.com        ]     │
    ├─────────────────────────────────────────┤
    │  테마:                                   │
    │    ◉ 다크 모드  ○ 라이트 모드           │
    ├─────────────────────────────────────────┤
    │  브라우징 데이터 삭제:                   │
    │    [북마크 전체 삭제]  [히스토리 전체 삭제] │
    ├─────────────────────────────────────────┤
    │              [저장]   [닫기]             │
    └─────────────────────────────────────────┘
    ```
  - **포커스 순서 (Tab Order)**:
    - 검색 엔진 라디오 버튼 → 홈페이지 입력 필드 → 테마 라디오 버튼 → 브라우징 데이터 삭제 버튼 → 저장 버튼 → 닫기 버튼
    - `QWidget::setFocusPolicy(Qt::StrongFocus)` 설정
    - `QWidget::setTabOrder(widget1, widget2)` 호출
  - **리모컨 키 매핑**:
    - **방향키 (Up/Down)**: 항목 간 포커스 이동 (Qt 기본 동작)
    - **방향키 (Left/Right)**: 라디오 버튼 간 선택 전환
    - **Enter**: 버튼 클릭 (저장, 닫기, 삭제)
    - **Back**: 설정 패널 닫기 (변경사항 자동 저장 또는 확인 다이얼로그)
  - **변경사항 저장 시점**:
    - 옵션 1 (권장): 항목 변경 즉시 저장 (검색 엔진 선택 시 즉시 LS2 API 호출)
    - 옵션 2: "저장" 버튼 클릭 시 일괄 저장 (사용자 혼란 가능성)
    - **최종 선택**: 옵션 1 (즉시 저장) — 모바일 UX 패턴과 일치
  - **패널 닫기 동작**:
    - "닫기" 버튼 클릭 → `SettingsPanel::hide()` + 슬라이드 아웃 애니메이션
    - Back 키 입력 → 동일 동작
    - 패널 외부 클릭 → 닫기 (터치 환경 대비, 리모컨에서는 불필요)

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **설정 로드 시간**: LS2 API에서 설정 로드 500ms 이내
  - 비동기 호출이므로 UI 블로킹 없음
  - 로드 실패 시 기본값 사용 (fallback)
- **설정 저장 시간**: LS2 API 저장 호출 200ms 이내
  - 사용자는 즉시 저장된 것으로 인식 (비동기 콜백)
  - 저장 실패 시 에러 메시지 표시 (QMessageBox::critical)
- **테마 변경 반응 속도**: 테마 변경 후 모든 컴포넌트 스타일 업데이트 0.5초 이내
  - QSS 스타일시트 재적용은 빠름 (Qt 내부 최적화)

### 접근성
- **리모컨 최적화**: 모든 설정 항목에 리모컨 방향키로 접근 가능
  - 포커스 표시 명확 (테두리 강조, Qt::FocusPolicy::StrongFocus)
  - Tab Order 설정으로 순차적 이동 보장
- **고대비 테마**: 다크 모드, 라이트 모드 모두 WCAG 2.1 AA 수준 대비 비율 준수
  - 다크 모드: 배경 #1E1E1E, 텍스트 #FFFFFF (21:1 대비)
  - 라이트 모드: 배경 #FFFFFF, 텍스트 #000000 (21:1 대비)
- **에러 메시지**: 시각적 + 텍스트 피드백 모두 제공
  - 잘못된 URL 입력 시: QLineEdit 테두리 빨간색 + QLabel 에러 메시지

### 보안
- **홈페이지 URL 검증**: URLValidator::isValid() 사용
  - JavaScript URL (javascript:) 입력 방지
  - file:// 프로토콜 입력 방지 (보안상 제한)
  - 허용 프로토콜: http://, https://, about:
- **브라우징 데이터 삭제**:
  - 확인 다이얼로그 필수 (실수 방지)
  - 삭제 후 즉시 LS2 API에서 데이터 제거 (메모리 캐시도 비움)
- **설정 데이터 저장**:
  - LS2 API는 앱별로 격리된 스토리지 사용 (다른 앱 접근 불가)
  - JSON 형식으로 저장 (평문이지만 webOS 샌드박스 내부)

### 확장성
- **설정 항목 추가 용이성**:
  - SettingsService 클래스에 getter/setter 메서드 추가만으로 새 설정 지원
  - SettingsPanel UI에 위젯 추가 (QFormLayout 사용 권장)
- **테마 확장**:
  - 추가 테마 (예: "high-contrast", "blue") 지원 가능
  - 테마별 QSS 파일 분리 (resources/themes/dark.qss, light.qss)

### 메모리 사용량
- **SettingsPanel**: 약 10KB (QWidget + 자식 위젯)
- **LS2 저장 데이터**: 약 1KB 미만 (JSON 형식)
  ```json
  {
    "defaultSearchEngine": "google",
    "homepage": "https://www.google.com",
    "theme": "dark"
  }
  ```

---

## 4. 데이터 구조

### SettingsService 클래스

```cpp
// src/services/SettingsService.h

#pragma once

#include <QObject>
#include <QString>

namespace webosbrowser {

class StorageService;

/**
 * @class SettingsService
 * @brief 브라우저 설정 관리 서비스
 *
 * 검색 엔진, 홈페이지, 테마 등의 설정을 LS2 API로 저장/로드합니다.
 */
class SettingsService : public QObject {
    Q_OBJECT

public:
    explicit SettingsService(StorageService *storageService, QObject *parent = nullptr);
    ~SettingsService() override;

    SettingsService(const SettingsService&) = delete;
    SettingsService& operator=(const SettingsService&) = delete;

    /**
     * @brief 설정 로드 (앱 시작 시 호출)
     * @return 로드 성공 여부
     */
    bool loadSettings();

    // 검색 엔진
    QString getSearchEngine() const;
    bool setSearchEngine(const QString &engineId);

    // 홈페이지
    QString getHomepage() const;
    bool setHomepage(const QString &url);

    // 테마
    QString getTheme() const;
    bool setTheme(const QString &theme);

signals:
    /**
     * @brief 검색 엔진 변경 시그널
     * @param engineId 새 검색 엔진 ID
     */
    void searchEngineChanged(const QString &engineId);

    /**
     * @brief 홈페이지 변경 시그널
     * @param url 새 홈페이지 URL
     */
    void homepageChanged(const QString &url);

    /**
     * @brief 테마 변경 시그널
     * @param theme 새 테마 ID ("dark" 또는 "light")
     */
    void themeChanged(const QString &theme);

    /**
     * @brief 설정 로드 완료 시그널
     */
    void settingsLoaded();

private:
    StorageService *storageService_;

    // 설정 캐시 (메모리)
    QString searchEngine_;
    QString homepage_;
    QString theme_;

    // 기본값
    static constexpr const char* DEFAULT_SEARCH_ENGINE = "google";
    static constexpr const char* DEFAULT_HOMEPAGE = "https://www.google.com";
    static constexpr const char* DEFAULT_THEME = "dark";

    // LS2 저장 키
    static constexpr const char* KEY_SEARCH_ENGINE = "defaultSearchEngine";
    static constexpr const char* KEY_HOMEPAGE = "homepage";
    static constexpr const char* KEY_THEME = "theme";
};

} // namespace webosbrowser
```

### LS2 저장 데이터 형식

```json
// StorageService::putData(DataKind::Settings, "browser", data) 호출 시

{
  "_id": "browser",
  "defaultSearchEngine": "google",
  "homepage": "https://www.google.com",
  "theme": "dark"
}
```

### 테마 스타일시트 (QSS)

```cpp
// resources/themes/dark.qss

QWidget#BrowserWindow {
    background-color: #1E1E1E;
}

QWidget#NavigationBar, QWidget#URLBar, QWidget#TabBar {
    background-color: #2D2D2D;
    border-bottom: 1px solid #444444;
}

QPushButton {
    background-color: #3C3C3C;
    color: #FFFFFF;
    border: 1px solid #555555;
    border-radius: 4px;
    padding: 8px 16px;
}

QPushButton:hover {
    background-color: #4A4A4A;
}

QPushButton:focus {
    border: 2px solid #0078D7;  /* 포커스 강조 */
}

QLineEdit {
    background-color: #2D2D2D;
    color: #FFFFFF;
    border: 1px solid #555555;
    border-radius: 4px;
    padding: 4px 8px;
}

QLineEdit:focus {
    border: 2px solid #0078D7;
}

QRadioButton {
    color: #FFFFFF;
}

QRadioButton::indicator {
    width: 16px;
    height: 16px;
}

QRadioButton::indicator:checked {
    background-color: #0078D7;
    border: 2px solid #0078D7;
    border-radius: 8px;
}
```

---

## 5. 제약조건

### 기술 제약사항
- **LS2 API 비동기 처리**:
  - StorageService::putData()는 비동기 호출이므로 성공/실패 콜백 처리 필요
  - 저장 실패 시 에러 메시지 표시 (QMessageBox::critical)
- **QSS 스타일시트 제약**:
  - 모든 Qt 위젯이 QSS로 스타일링 가능한 것은 아님 (예: QMessageBox 일부 속성)
  - 복잡한 애니메이션은 QPropertyAnimation 사용 (QSS 제한)
- **URL 검증 제약**:
  - URLValidator가 일부 특수 URL (예: about:blank, chrome://...) 처리 못할 수 있음
  - 화이트리스트 방식으로 허용 프로토콜 명시 (http, https, about)

### webOS 플랫폼 제약사항
- **리모컨 키 코드 차이**:
  - Menu 버튼의 keyCode가 기기별로 다를 수 있음 (457 또는 18)
  - BrowserWindow::keyPressEvent()에서 둘 다 처리
- **LS2 API 용량 제한**:
  - 설정 데이터는 작으므로 (1KB 미만) 제약 없음
  - 향후 설정 항목 추가 시에도 수십 KB 수준 예상

### 의존성
- **F-09 (검색 엔진 통합)**: SearchEngine 클래스 (검색 엔진 목록, getter/setter)
- **F-07 (북마크 관리)**: BookmarkService::deleteAllBookmarks()
- **F-08 (히스토리 관리)**: HistoryService::deleteAllHistory()
- **F-13 (리모컨 단축키)**: Menu 버튼으로 설정 패널 열기 (handleMenuButton 이미 구현됨)
- **F-01 (프로젝트 초기 설정)**: StorageService (LS2 API 래퍼)

### 범위 외 (Out of Scope)
- **고급 설정**: 쿠키 관리, 캐시 설정, JavaScript 활성화/비활성화 등은 M4 이후 검토
- **사용자별 프로필**: 다중 사용자 프로필 지원 미포함 (단일 프로필만)
- **설정 내보내기/가져오기**: 설정을 JSON 파일로 내보내기 기능 미포함
- **언어 설정**: UI 다국어 지원 (한국어/영어) 미포함 (M4 이후)
- **사용자 정의 CSS**: 웹 페이지에 커스텀 CSS 적용 기능 미포함

---

## 6. 용어 정의

| 용어 | 정의 |
|------|------|
| **설정 패널** | 브라우저 설정을 변경할 수 있는 오버레이 UI 패널 (SettingsPanel) |
| **검색 엔진** | 검색어 입력 시 사용할 기본 검색 서비스 (Google, Naver 등) |
| **홈페이지** | NavigationBar의 홈 버튼 클릭 시 이동할 URL |
| **테마** | 브라우저 UI의 색상 스킴 (다크 모드, 라이트 모드) |
| **브라우징 데이터** | 북마크, 히스토리, 쿠키, 캐시 등 사용자 데이터 |
| **LS2 API** | webOS 로컬 스토리지 API (Luna Service 2, DB8 NoSQL) |
| **QSS** | Qt Style Sheet (CSS와 유사한 Qt 위젯 스타일링 언어) |
| **Menu 버튼** | webOS 리모컨의 설정/메뉴 버튼 (Qt::Key_Menu) |
| **오버레이 패널** | 메인 윈도우 위에 표시되는 슬라이드 인 패널 (모달 또는 비모달) |

---

## 7. 완료 기준 (Acceptance Criteria)

### AC-1: 검색 엔진 선택
- [ ] 설정 패널에서 검색 엔진 라디오 버튼 선택 가능 (Google, Naver, Bing, DuckDuckGo)
- [ ] 검색 엔진 변경 시 즉시 LS2 API에 저장
- [ ] 변경 후 URLBar에서 검색어 입력 시 새 검색 엔진 사용
- [ ] 앱 재시작 후에도 선택한 검색 엔진 유지
- [ ] SearchEngine::setDefaultSearchEngine() 호출 → SearchEngine::getDefaultSearchEngine() 반영

### AC-2: 홈페이지 설정
- [ ] 설정 패널에서 홈페이지 URL 입력 가능 (QLineEdit)
- [ ] 잘못된 URL 입력 시 에러 메시지 표시 (URLValidator::isValid() 사용)
- [ ] 유효한 URL 입력 시 LS2 API에 저장
- [ ] NavigationBar의 홈 버튼 클릭 시 설정된 URL로 이동
- [ ] 앱 재시작 후에도 설정 유지

### AC-3: 테마 설정
- [ ] 설정 패널에서 테마 라디오 버튼 선택 가능 (다크 모드, 라이트 모드)
- [ ] 테마 변경 시 즉시 모든 컴포넌트에 새 스타일시트 적용 (재시작 불필요)
- [ ] 다크 모드: 배경 #1E1E1E, 텍스트 #FFFFFF
- [ ] 라이트 모드: 배경 #FFFFFF, 텍스트 #000000
- [ ] 앱 재시작 시 저장된 테마로 초기화
- [ ] SettingsService::themeChanged 시그널 → BrowserWindow::onThemeChanged 슬롯 연결

### AC-4: 브라우징 데이터 삭제
- [ ] "북마크 전체 삭제" 버튼 클릭 시 확인 다이얼로그 표시
- [ ] 확인 다이얼로그에서 "삭제" 선택 시 BookmarkService::deleteAllBookmarks() 호출
- [ ] 삭제 완료 후 BookmarkPanel 목록 비워짐 (자동 새로고침)
- [ ] "히스토리 전체 삭제"도 동일한 플로우
- [ ] 확인 다이얼로그에서 "취소" 선택 시 삭제 안 됨

### AC-5: 설정 패널 접근 및 네비게이션
- [ ] NavigationBar의 "설정" 버튼 클릭으로 설정 패널 열림
- [ ] 리모컨 Menu 버튼으로 설정 패널 열림 (F-13 연동)
- [ ] 설정 패널 슬라이드 인 애니메이션 (0.3초)
- [ ] 리모컨 방향키 (Up/Down)로 항목 간 포커스 이동
- [ ] 리모컨 Enter 키로 버튼 클릭, 라디오 버튼 선택
- [ ] 리모컨 Back 키로 설정 패널 닫기 (슬라이드 아웃 애니메이션)
- [ ] 포커스 표시 명확 (테두리 강조)

### AC-6: 설정 변경 즉시 저장
- [ ] 검색 엔진 라디오 버튼 선택 시 즉시 저장 (저장 버튼 불필요)
- [ ] 테마 라디오 버튼 선택 시 즉시 저장 및 적용
- [ ] 홈페이지 URL은 "저장" 버튼 또는 Enter 키로 저장

### AC-7: 에러 처리
- [ ] 홈페이지 URL 검증 실패 시 QLabel에 에러 메시지 표시
- [ ] LS2 API 저장 실패 시 QMessageBox::critical 표시
- [ ] 브라우징 데이터 삭제 실패 시 에러 메시지 표시

### AC-8: 회귀 테스트
- [ ] 기존 NavigationBar, URLBar, TabBar 동작 정상
- [ ] F-09 검색 엔진 통합 기능 정상 (URLBar 검색)
- [ ] F-13 리모컨 단축키와 충돌 없음

---

## 8. UI/UX 플로우

### 시나리오 1: 검색 엔진 변경

```
사용자 → Menu 버튼 → SettingsPanel → SearchEngine 라디오 버튼 선택 → LS2 저장
   │           │               │                    │                         │
   │  Menu 키 입력              │                    │                         │
   │─────────────▶│             │                    │                         │
   │              │ show() + 슬라이드 인             │                         │
   │              │────────────▶│                    │                         │
   │              │             │ 포커스: Google → Naver                       │
   │              │             │ Enter 키 입력      │                         │
   │              │             │───────────────────▶│                         │
   │              │             │                    │ SettingsService::setSearchEngine("naver")
   │              │             │                    │────────────────────────▶│
   │              │             │                    │ StorageService::putData(DataKind::Settings, ...)
   │              │             │                    │                         │
   │              │             │                    │◀────────────────────────│
   │              │             │                    │ emit searchEngineChanged("naver")
   │              │             │                    │────────────────────────▶URLBar
```

### 시나리오 2: 홈페이지 설정

```
사용자 → SettingsPanel → 홈페이지 입력 필드 → URL 입력 → Enter 키 → LS2 저장
   │           │                  │                  │           │             │
   │  방향키로 이동                │                  │           │             │
   │──────────▶│                  │                  │           │             │
   │           │ 포커스: 홈페이지 필드               │           │             │
   │           │─────────────────▶│                  │           │             │
   │           │                  │ 가상 키보드 표시 │           │             │
   │           │                  │ "https://www.naver.com" 입력│             │
   │           │                  │──────────────────▶│           │             │
   │           │                  │                  │ Enter 키  │             │
   │           │                  │                  │──────────▶│             │
   │           │                  │                  │ URLValidator::isValid() → true
   │           │                  │                  │           │ SettingsService::setHomepage()
   │           │                  │                  │           │────────────▶│
   │           │                  │                  │           │ emit homepageChanged()
   │           │                  │                  │           │────────────▶NavigationBar
```

### 시나리오 3: 테마 변경

```
사용자 → SettingsPanel → 테마 라디오 버튼 → LS2 저장 → 스타일시트 재적용
   │           │                  │                  │                  │
   │  라디오 버튼 선택 (라이트 모드)                 │                  │
   │──────────▶│                  │                  │                  │
   │           │ Light 라디오 버튼 클릭              │                  │
   │           │─────────────────▶│                  │                  │
   │           │                  │ SettingsService::setTheme("light")  │
   │           │                  │──────────────────▶│                  │
   │           │                  │                  │ emit themeChanged("light")
   │           │                  │                  │─────────────────▶BrowserWindow
   │           │                  │                  │                  │
   │           │                  │                  │ onThemeChanged("light")
   │           │                  │                  │ loadThemeStylesheet("light")
   │           │                  │                  │ setStyleSheet(lightQss)
   │           │                  │                  │ 모든 자식 위젯 업데이트
   │           │                  │                  │◀─────────────────│
```

### 시나리오 4: 브라우징 데이터 삭제

```
사용자 → SettingsPanel → "북마크 전체 삭제" 버튼 → 확인 다이얼로그 → 삭제
   │           │                  │                        │                │
   │  버튼 클릭 │                  │                        │                │
   │──────────▶│                  │                        │                │
   │           │ onDeleteBookmarksClicked()                │                │
   │           │─────────────────▶│                        │                │
   │           │                  │ QMessageBox::warning() │                │
   │           │                  │───────────────────────▶│                │
   │           │                  │                        │ "삭제" 선택     │
   │           │                  │                        │───────────────▶│
   │           │                  │                        │ BookmarkService::deleteAllBookmarks()
   │           │                  │                        │ LS2 API 호출   │
   │           │                  │                        │                │
   │           │                  │                        │ emit allBookmarksDeleted()
   │           │                  │                        │───────────────▶BookmarkPanel
   │           │                  │                        │ 목록 새로고침   │
   │           │                  │ QMessageBox::information("삭제 완료")   │
```

---

## 9. 참고 사항

### 관련 문서
- PRD: `docs/project/prd.md`
- 기능 백로그: `docs/project/features.md`
- F-09 요구사항 분석서: `docs/specs/search-engine-integration/requirements.md`
- F-07 요구사항 분석서: `docs/specs/bookmark-management/requirements.md`
- F-08 요구사항 분석서: `docs/specs/history-management/requirements.md`
- F-13 요구사항 분석서: `docs/specs/remote-shortcuts/requirements.md`
- CLAUDE.md: 프로젝트 규칙 및 코딩 컨벤션

### 참고 구현
- Chrome 브라우저 설정 화면: `chrome://settings/`
  - 검색 엔진 관리
  - 시작 페이지 설정
  - 테마 선택 (다크/라이트)
  - 인터넷 사용 기록 삭제
- Firefox 브라우저 설정: `about:preferences`
  - 홈 페이지 설정
  - 프라이버시 및 보안 (방문 기록 삭제)
  - 테마 (시스템 테마 따르기)
- Qt QSettings 클래스: 설정 저장/로드 패턴
  - https://doc.qt.io/qt-5/qsettings.html

### Qt 위젯 선택 가이드
- **검색 엔진 선택**: QRadioButton (리모컨 방향키로 선택 쉬움)
- **홈페이지 입력**: QLineEdit (가상 키보드 연동)
- **테마 선택**: QRadioButton 또는 QCheckBox (토글 스위치)
- **삭제 버튼**: QPushButton (빨간색 스타일)
- **확인 다이얼로그**: QMessageBox::warning() (Yes/No 버튼)
- **패널 레이아웃**: QFormLayout (라벨 + 입력 필드 정렬)

### 구현 권장사항
- **SettingsService 싱글톤 패턴**:
  - 앱 전체에서 하나의 인스턴스만 사용 (BrowserWindow에서 소유)
  - 다른 컴포넌트는 시그널/슬롯으로 설정 변경 통지 받음
- **테마 스타일시트 분리**:
  - `resources/themes/dark.qss`, `light.qss` 파일로 분리
  - Qt 리소스 시스템 (qrc) 사용 (`:/themes/dark.qss`)
- **설정 변경 디바운스**:
  - 라디오 버튼 빠르게 여러 번 클릭 시 LS2 API 호출 중복 방지
  - QTimer::singleShot(300ms) 사용

### 우선순위 조정 가능성
- FR-4 (브라우징 데이터 삭제)의 쿠키/캐시 삭제는 M4 이후로 연기 가능 (북마크/히스토리 삭제 우선)
- FR-3 (테마 설정)의 라이트 모드는 Should 우선순위로 조정 가능 (다크 모드만 M3에서 구현)

---

## 10. 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|-----------|--------|
| 2026-02-14 | 1.0 | F-11 설정 화면 요구사항 분석서 초안 작성 (Web App → Native App 전환) | product-manager |
