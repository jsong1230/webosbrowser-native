# URL 입력 UI — 구현 계획서 (Native App)

## 1. 참조 문서
- 요구사항 분석서: docs/specs/url-input-ui/requirements.md
- 기술 설계서: docs/specs/url-input-ui/design.md
- PRD: docs/project/prd.md
- 기능 백로그: docs/project/features.md
- CLAUDE.md: 프로젝트 규칙 및 코딩 컨벤션

---

## 2. 구현 Phase

### Phase 1: 기본 인프라 구축
**담당자**: cpp-dev
**예상 소요 시간**: 2시간
**의존성**: F-02 (WebView 통합) 완료 필요

#### Task 1.1: URLValidator 유틸리티 구현
- [ ] src/utils/URLValidator.h 헤더 작성
  - 정적 메서드 선언: `isValid`, `autoComplete`, `isSearchQuery`, `isDomainFormat`
  - QRegularExpression 기반 도메인 정규표현식 선언
- [ ] src/utils/URLValidator.cpp 구현
  - `isValid()`: QUrl::isValid() 기반 URL 유효성 검증
  - `autoComplete()`: QUrl::fromUserInput() 활용, 프로토콜 자동 추가 (http://)
  - `isSearchQuery()`: 공백 포함 또는 도메인 패턴 미일치 시 true 반환
  - `isDomainFormat()`: 정규표현식으로 도메인 패턴 검증
    - 패턴: `^([a-zA-Z0-9-]+\.)+[a-zA-Z]{2,}(\/.*)?$`
    - 예: `google.com`, `www.example.com/path`
- [ ] CMakeLists.txt 수정
  - SOURCES에 `src/utils/URLValidator.cpp` 추가 (이미 존재하는지 확인)

**산출물**:
- src/utils/URLValidator.h
- src/utils/URLValidator.cpp
- 수정된 CMakeLists.txt (필요 시)

**완료 기준**:
- URLValidator::isValid("http://google.com") → true
- URLValidator::autoComplete("google.com") → QUrl("http://google.com")
- URLValidator::isSearchQuery("hello world") → true
- URLValidator::isDomainFormat("google.com") → true

---

### Phase 2: VirtualKeyboard 구현
**담당자**: cpp-dev
**예상 소요 시간**: 4시간
**의존성**: Phase 1 완료 불필요 (독립적)

#### Task 2.1: VirtualKeyboard 클래스 헤더 작성
- [ ] src/ui/VirtualKeyboard.h 생성
  - QWidget 상속 클래스 선언
  - 시그널 선언: `characterEntered`, `backspacePressed`, `enterPressed`, `spacePressed`, `closeRequested`
  - keyPressEvent 오버라이드 선언
  - moveFocusInGrid, getCurrentFocusPosition 프라이빗 메서드 선언
  - 멤버 변수: `QGridLayout *keyboardLayout_`, `QVector<QVector<QPushButton*>> keys_`
  - 상수 정의: `ROWS = 4`, `COLS = 11`

#### Task 2.2: VirtualKeyboard 클래스 구현
- [ ] src/ui/VirtualKeyboard.cpp 생성
  - `setupUI()`: QWERTY 레이아웃 그리드 생성
    - 행 0: 숫자 1-0 + `-`
    - 행 1: qwertyuiop + `/`
    - 행 2: asdfghjkl + `:` + `.`
    - 행 3: zxcvbnm + `?` + `&` + `=` + `_`
    - 제어 키 (행 4): Space (4칸), Backspace (2칸), Enter (3칸), Cancel (2칸)
  - `createKeyButton()`: QPushButton 생성 및 스타일 설정
    - 최소 크기: 60x60px
    - 폰트 크기: 20px
  - `keyPressEvent()`: 리모컨 방향키 처리
    - Qt::Key_Up/Down/Left/Right → moveFocusInGrid 호출
    - Qt::Key_Select/Enter → 현재 포커스된 버튼 클릭 시뮬레이션
    - Qt::Key_Escape/Back → emit closeRequested(), hide()
  - `moveFocusInGrid()`: 그리드 내 포커스 이동 로직
    - 2D 배열 좌표 계산, 순환 이동 (모듈러 연산)
  - `getCurrentFocusPosition()`: focusWidget()을 QPoint로 변환
  - `onKeyButtonClicked()`: 버튼 클릭 시 해당 문자 emit characterEntered
  - `applyStyles()`: QSS 스타일 적용
    - 포커스 시 3px 파란 테두리 (border: 3px solid #00aaff)
    - 배경색: #3a3a3a, 폰트색: #ffffff

#### Task 2.3: CMakeLists.txt 수정
- [ ] SOURCES에 `src/ui/VirtualKeyboard.cpp` 추가
- [ ] HEADERS에 `src/ui/VirtualKeyboard.h` 추가

**산출물**:
- src/ui/VirtualKeyboard.h
- src/ui/VirtualKeyboard.cpp
- 수정된 CMakeLists.txt

**완료 기준**:
- VirtualKeyboard 위젯이 화면에 표시됨 (QDialog 또는 QWidget)
- 리모컨 방향키로 모든 키에 포커스 이동 가능
- Select 키로 문자 입력 시 characterEntered 시그널 발생
- ESC/Back 키로 키보드 닫기 가능
- 포커스된 키가 3px 파란 테두리로 명확히 표시됨

---

### Phase 3: URLBar 컴포넌트 구현
**담당자**: cpp-dev
**예상 소요 시간**: 4시간
**의존성**: Phase 1, Phase 2 완료 필요

#### Task 3.1: URLBar 클래스 구현 (UI 부분)
- [ ] src/ui/URLBar.cpp 기본 구조 작성
  - 생성자: UI 초기화, 시그널/슬롯 연결, 스타일 적용
  - `setupUI()`: QVBoxLayout 기반 레이아웃 구성
    - inputLayout_ (QHBoxLayout): inputField_ 배치
    - errorLabel_ (QLabel): 에러 메시지 표시, 초기에는 숨김
    - autocompleteFrame_ (QFrame): autocompleteList_ 포함, 초기에는 숨김
  - `applyStyles()`: QSS 스타일 적용
    - inputField_: 폰트 18px, 포커스 시 3px 파란 테두리
    - errorLabel_: 빨간색 폰트 (#ff4444), 14px
    - autocompleteList_: 배경 #2a2a2a, 폰트 16px

#### Task 3.2: URLBar 입력 및 검증 로직
- [ ] `keyPressEvent()` 오버라이드
  - Qt::Key_Enter/Return → validateAndCompleteUrl() 호출 → emit urlSubmitted
  - Qt::Key_Escape/Back → 이전 URL 복원 (previousUrl_), emit editingCancelled
  - Qt::Key_Down → 자동완성 리스트로 포커스 이동
  - Qt::Key_Select → showVirtualKeyboard() 호출
- [ ] `validateAndCompleteUrl()` 구현
  - URLValidator::isValid() 체크
  - false → showError("유효한 URL을 입력하세요"), return QUrl()
  - true → URLValidator::autoComplete() 호출, return QUrl
- [ ] `showError()`, `hideError()` 구현
  - errorLabel_->setText(message), errorLabel_->show()
  - inputField_->setStyleSheet("border: 2px solid red;")
  - 3초 후 자동으로 hideError() (QTimer::singleShot)
- [ ] focusInEvent, focusOutEvent 오버라이드
  - 포커스 인: 테두리 하이라이트 (QSS 자동 적용)
  - 포커스 아웃: 가상 키보드 숨김 (선택사항)

#### Task 3.3: URLBar + VirtualKeyboard 통합
- [ ] VirtualKeyboard 인스턴스 생성 및 멤버 변수 초기화
  - virtualKeyboard_ = std::make_unique<VirtualKeyboard>(this)
  - virtualKeyboard_->hide() (초기에는 숨김)
- [ ] `showVirtualKeyboard()` 구현
  - previousUrl_ = inputField_->text() (취소 시 복원용)
  - virtualKeyboard_->show()
  - virtualKeyboard_->setFocus()
- [ ] 시그널/슬롯 연결 (setupConnections)
  - VirtualKeyboard::characterEntered → URLBar::onKeyboardCharacterEntered
  - VirtualKeyboard::backspacePressed → inputField_->backspace()
  - VirtualKeyboard::spacePressed → inputField_->insert(" ")
  - VirtualKeyboard::enterPressed → validateAndCompleteUrl() 호출
  - VirtualKeyboard::closeRequested → URLBar::onKeyboardClosed
- [ ] `onKeyboardCharacterEntered()` 구현
  - inputField_->insert(character)
- [ ] `onKeyboardClosed()` 구현
  - virtualKeyboard_->hide()
  - inputField_->setFocus()

**산출물**:
- src/ui/URLBar.cpp (확장 구현)

**완료 기준**:
- URLBar 위젯이 BrowserWindow 상단에 표시됨
- 리모컨 포커스 시 테두리 하이라이트 표시
- Select 키로 가상 키보드 표시
- 가상 키보드로 문자 입력 시 inputField_에 반영
- Enter 키로 URL 제출 시 urlSubmitted 시그널 발생
- ESC 키로 입력 취소 시 이전 URL 복원
- 유효하지 않은 URL 입력 시 에러 메시지 표시

---

### Phase 4: 자동완성 기능 구현 (선택적)
**담당자**: cpp-dev
**예상 소요 시간**: 3시간
**의존성**: Phase 3 완료, F-07 (북마크), F-08 (히스토리) 완료 시 활성화

#### Task 4.1: 자동완성 UI 구현
- [ ] `showAutocompletePopup()` 구현
  - autocompleteList_->clear()
  - QStringList를 QListWidgetItem으로 추가
  - 각 항목: 타이틀 + URL (두 줄 표시)
  - autocompleteFrame_->show()
  - isAutocompleteVisible_ = true
- [ ] `hideAutocompletePopup()` 구현
  - autocompleteFrame_->hide()
  - isAutocompleteVisible_ = false
- [ ] autocompleteList_ 키 이벤트 처리
  - Qt::Key_Up/Down → 항목 간 이동
  - Qt::Key_Select/Enter → 선택된 항목의 URL을 inputField_에 채움
  - Qt::Key_Escape → hideAutocompletePopup()

#### Task 4.2: 자동완성 검색 로직
- [ ] `onTextChanged()` 슬롯 구현
  - hideError() 호출
  - debounceTimer_->stop()
  - debounceTimer_->start(500) (500ms 디바운싱)
- [ ] `onDebounceTimeout()` 슬롯 구현
  - QString query = inputField_->text()
  - searchAutocomplete(query) 호출
- [ ] `searchAutocomplete()` 구현
  - query.length() < 2 → hideAutocompletePopup(), return
  - HistoryService::search(query) 호출 (F-08 완료 후)
  - BookmarkService::search(query) 호출 (F-07 완료 후)
  - 결과 병합, 중복 제거, 정렬
  - 최대 5개로 제한
  - showAutocompletePopup(suggestions) 호출
- [ ] 시그널/슬롯 연결
  - inputField_->textChanged → URLBar::onTextChanged
  - debounceTimer_->timeout → URLBar::onDebounceTimeout
  - autocompleteList_->itemClicked → URLBar::onAutocompleteItemSelected

#### Task 4.3: 의존성 주입 메서드 구현
- [ ] `setHistoryService()` 구현
  - historyService_ = service
- [ ] `setBookmarkService()` 구현
  - bookmarkService_ = service

**산출물**:
- src/ui/URLBar.cpp (자동완성 기능 추가)

**완료 기준**:
- 문자 입력 시 500ms 후 자동완성 팝업 표시
- 히스토리/북마크에서 일치하는 항목 검색 (최대 5개)
- 방향키 하(Key_Down)로 자동완성 리스트 포커스 이동
- Select 키로 항목 선택 시 URL 필드에 채워짐
- 제안 없을 시 팝업 숨김

**주의**: F-07, F-08 미완료 시 자동완성 검색 로직은 스켈레톤으로 남겨두고, 나중에 통합

---

### Phase 5: BrowserWindow 통합
**담당자**: cpp-dev
**예상 소요 시간**: 1시간
**의존성**: Phase 3 완료 필요

#### Task 5.1: BrowserWindow에 URLBar 추가
- [ ] src/browser/BrowserWindow.h 수정
  - `URLBar *urlBar_` 멤버 변수 선언
  - 헤더 인클루드: `#include "ui/URLBar.h"`
- [ ] src/browser/BrowserWindow.cpp 수정
  - `setupUI()` 메서드:
    - urlBar_ = new URLBar(this)
    - mainLayout_->addWidget(urlBar_) (WebView 위에 배치)
  - `setupConnections()` 메서드:
    - URLBar::urlSubmitted → WebView::load 시그널/슬롯 연결
    - WebView::urlChanged → URLBar::setText 연결 (현재 URL 표시)
    - WebView::loadError → URLBar::showError 연결 (에러 메시지 표시)

**산출물**:
- 수정된 src/browser/BrowserWindow.h
- 수정된 src/browser/BrowserWindow.cpp

**완료 기준**:
- BrowserWindow 실행 시 URLBar가 상단에 표시됨
- URLBar에서 URL 입력 후 Enter 시 WebView에서 페이지 로드
- 페이지 로드 성공 시 URLBar에 최종 URL 표시 (리다이렉트 반영)
- 페이지 로드 실패 시 URLBar에 에러 메시지 표시

---

### Phase 6: 스타일링 및 리소스 추가
**담당자**: cpp-dev
**예상 소요 시간**: 1시간
**의존성**: Phase 5 완료 후

#### Task 6.1: QSS 스타일 파일 작성 (선택적)
- [ ] resources/styles/urlbar.qss 생성
  - URLBar QLineEdit 스타일 (포커스, 일반)
  - 에러 라벨 스타일 (빨간색)
  - VirtualKeyboard QPushButton 스타일 (포커스, 일반)
  - 자동완성 QListWidget 스타일 (선택, 일반)
- [ ] URLBar::applyStyles() 수정
  - QFile::open(":/styles/urlbar.qss")
  - setStyleSheet(qssContent)
- [ ] resources.qrc 파일 수정
  - urlbar.qss를 리소스에 추가

**산출물**:
- resources/styles/urlbar.qss
- 수정된 resources.qrc (또는 CMakeLists.txt)

**완료 기준**:
- QSS 파일이 런타임에 로드되어 스타일 적용됨
- 포커스된 요소가 3px 파란 테두리로 명확히 표시됨
- 폰트 크기가 요구사항 충족 (18px 입력 필드, 20px 키보드)

---

### Phase 7: 단위 테스트 작성
**담당자**: test-runner
**예상 소요 시간**: 3시간
**의존성**: Phase 1, Phase 2, Phase 3 완료 필요

#### Task 7.1: URLValidator 단위 테스트
- [ ] tests/unit/URLValidatorTest.cpp 생성
  - TEST: ValidURL (유효한 URL 검증)
  - TEST: InvalidURL (잘못된 URL 검증)
  - TEST: AutoComplete (프로토콜 자동 추가)
  - TEST: SearchQuery (검색어 판단)
  - TEST: DomainFormat (도메인 형식 검증)
- [ ] CMakeLists.txt (tests 디렉토리) 수정
  - URLValidatorTest.cpp 추가

#### Task 7.2: VirtualKeyboard 단위 테스트
- [ ] tests/unit/VirtualKeyboardTest.cpp 생성
  - TEST: CharacterEntered (문자 입력 시그널)
  - TEST: FocusNavigation (그리드 포커스 이동)
  - TEST: BackspacePressed (백스페이스 시그널)
  - TEST: CloseRequested (키보드 닫기 시그널)
- [ ] CMakeLists.txt 수정
  - VirtualKeyboardTest.cpp 추가

#### Task 7.3: URLBar 통합 테스트
- [ ] tests/integration/URLBarIntegrationTest.cpp 생성
  - TEST: SubmitURL (URL 제출 후 WebView 로드)
  - TEST: CancelEditing (ESC 키로 입력 취소)
  - TEST: InvalidURLError (유효하지 않은 URL 에러 표시)
  - TEST: RemoteControlNavigation (리모컨 키 네비게이션)

**산출물**:
- tests/unit/URLValidatorTest.cpp
- tests/unit/VirtualKeyboardTest.cpp
- tests/integration/URLBarIntegrationTest.cpp
- 수정된 tests/CMakeLists.txt

**완료 기준**:
- 모든 단위 테스트 통과 (Google Test)
- 통합 테스트 통과 (Qt Test)
- 테스트 커버리지 80% 이상 (핵심 로직)

---

### Phase 8: 코드 리뷰 및 문서화
**담당자**: code-reviewer
**예상 소요 시간**: 2시간
**의존성**: Phase 1~7 완료 필요

#### Task 8.1: 코드 리뷰
- [ ] URLValidator 코드 리뷰
  - 정규표현식 검증 로직 확인
  - QUrl::fromUserInput 사용 적절성 검증
- [ ] VirtualKeyboard 코드 리뷰
  - 그리드 포커스 이동 로직 검증
  - 메모리 누수 확인 (QPushButton 배열)
- [ ] URLBar 코드 리뷰
  - 시그널/슬롯 연결 정합성 확인
  - keyPressEvent 로직 검증
- [ ] BrowserWindow 통합 검증
  - 레이아웃 구조 확인
  - 포커스 경로 검증

#### Task 8.2: 문서 검증
- [ ] requirements.md 충족 여부 확인
  - FR-1~FR-6 완료 기준 검증
  - 비기능 요구사항 (성능, 가독성, UX) 검증
- [ ] design.md 설계 준수 확인
  - 아키텍처 결정 준수 여부
  - API 명세 일치 여부
- [ ] plan.md 태스크 완료 확인
  - 모든 Phase 완료 여부 체크리스트

**산출물**:
- 코드 리뷰 결과 (CHANGELOG.md 또는 dev-log.md에 기록)
- 개선 제안 목록 (필요 시)

**완료 기준**:
- 모든 코드가 CLAUDE.md 코딩 컨벤션 준수
- 주요 기능 요구사항 (FR-1~FR-6) 모두 충족
- 비기능 요구사항 (성능, 가독성, UX) 검증 완료
- 테스트 통과 및 문서 업데이트 완료

---

## 3. 태스크 의존성 다이어그램

```
Phase 1 (URLValidator)
    │
    ├──────────────┐
    │              │
    ▼              ▼
Phase 2       Phase 3
(VirtualKeyboard)  (URLBar 기본)
    │              │
    └──────┬───────┘
           │
           ▼
       Phase 3.3
     (URLBar 통합)
           │
           ▼
       Phase 4
   (자동완성, 선택적)
           │
           ▼
       Phase 5
   (BrowserWindow 통합)
           │
           ▼
       Phase 6
     (스타일링)
           │
           ▼
       Phase 7
     (테스트 작성)
           │
           ▼
       Phase 8
     (코드 리뷰)
```

---

## 4. 병렬 실행 판단

### 병렬 가능한 태스크
- **Phase 1 (URLValidator)** 와 **Phase 2 (VirtualKeyboard)** 는 독립적이므로 병렬 실행 가능
- **Phase 7.1 (URLValidator 테스트)** 와 **Phase 7.2 (VirtualKeyboard 테스트)** 는 독립적이므로 병렬 실행 가능

### Agent Team 사용 권장 여부
**권장하지 않음** (순차 실행 권장)

#### 이유:
1. **의존성 체인**: Phase 3은 Phase 1, 2 완료 후 시작해야 하며, Phase 5는 Phase 3 완료 후 시작해야 함. 의존성이 복잡하지 않고 선형적이므로 순차 실행이 더 안전함.
2. **단일 개발자 컨텍스트**: URLBar, VirtualKeyboard, URLValidator는 모두 URL 입력 UI라는 하나의 기능 도메인에 속하므로, 단일 개발자(cpp-dev)가 전체 컨텍스트를 이해하며 순차 작업하는 것이 효율적임.
3. **통합 테스트 필요**: Phase 3에서 URLBar와 VirtualKeyboard를 통합하므로, 두 컴포넌트를 병렬로 개발하면 통합 시 충돌 가능성이 있음.
4. **PG-1 그룹 특성**: F-03은 F-04 (네비게이션 바), F-05 (로딩 인디케이터)와 병렬 구현 가능하지만, F-03 자체 내부에서는 Phase가 순차적으로 진행되는 것이 안전함.

#### 대안:
- F-03, F-04, F-05를 **별도 브랜치**에서 병렬 개발 (feature/url-input-ui, feature/navigation-bar, feature/loading-indicator)
- F-03 내부는 순차 실행 (Phase 1 → Phase 2 → Phase 3 → ... → Phase 8)

---

## 5. 예상 소요 시간

| Phase | 예상 시간 | 비고 |
|-------|----------|------|
| Phase 1: URLValidator 구현 | 2시간 | 정규표현식 테스트 포함 |
| Phase 2: VirtualKeyboard 구현 | 4시간 | 그리드 레이아웃 + 키 이벤트 처리 |
| Phase 3: URLBar 구현 | 4시간 | UI + 검증 로직 + VirtualKeyboard 통합 |
| Phase 4: 자동완성 구현 (선택) | 3시간 | F-07, F-08 완료 후 활성화 |
| Phase 5: BrowserWindow 통합 | 1시간 | 시그널/슬롯 연결 |
| Phase 6: 스타일링 | 1시간 | QSS 파일 작성 |
| Phase 7: 테스트 작성 | 3시간 | 단위 + 통합 테스트 |
| Phase 8: 코드 리뷰 | 2시간 | 문서 검증 포함 |
| **총 예상 시간** | **20시간** | **자동완성 포함 시 20시간, 미포함 시 17시간** |

### 현실적 일정 (1일 = 6시간 개발 가능 시)
- **자동완성 미포함**: 3일 (17시간 / 6시간)
- **자동완성 포함**: 4일 (20시간 / 6시간)

---

## 6. 리스크 및 대응

| 리스크 | 영향도 | 대응 방안 |
|--------|--------|-----------|
| **webOS 리모컨 키 코드가 Qt::Key enum과 다를 수 있음** | 높음 | - webOS 실기기에서 키 코드 로깅 테스트<br>- keyPressEvent에서 event->key() 값 출력하여 매핑 확인<br>- 필요 시 커스텀 키 매핑 테이블 작성 |
| **VirtualKeyboard 그리드 포커스 이동 로직 복잡도** | 중간 | - 단순한 모듈러 연산 방식 사용 (행, 열 순환 이동)<br>- 단위 테스트로 모든 포커스 이동 경로 검증<br>- 디버깅 시 현재 포커스 위치 로깅 |
| **QSS 스타일이 webOS에서 다르게 렌더링될 수 있음** | 중간 | - 초기에 최소한의 QSS만 사용 (테두리, 배경색, 폰트 크기)<br>- 실기기에서 테스트 후 스타일 조정<br>- 복잡한 그라데이션, 그림자 효과 지양 |
| **F-07, F-08 미완료 시 자동완성 기능 지연** | 낮음 | - Phase 4를 선택적 Phase로 설정<br>- F-07, F-08 완료 후 자동완성 통합<br>- 현재는 스켈레톤 코드만 남겨두고 나중에 활성화 |
| **URLBar와 WebView 간 시그널/슬롯 연결 실패** | 중간 | - BrowserWindow::setupConnections()에서 connect 반환값 검증<br>- qDebug로 시그널 발생 로깅<br>- Qt 문서 참조하여 시그널 시그니처 정확히 일치시킴 |
| **가상 키보드 메모리 누수 (QPushButton 배열)** | 낮음 | - QObject 부모-자식 관계로 자동 삭제 보장<br>- QGridLayout이 소유권 관리<br>- 단위 테스트에서 Valgrind로 메모리 검증 |
| **URL 자동 보완 로직이 엣지 케이스에서 실패** | 중간 | - URLValidator 단위 테스트에서 엣지 케이스 포함<br>  (예: localhost, 192.168.1.1, file://, 특수문자 포함 URL)<br>- QUrl::fromUserInput() 활용으로 표준 처리 |
| **리모컨 포커스 경로가 직관적이지 않음** | 중간 | - 사용자 테스트 (실제 프로젝터에서 테스트)<br>- 포커스 경로 문서화 (design.md 참조)<br>- 필요 시 setTabOrder 또는 커스텀 포커스 네비게이션 수정 |

---

## 7. 생성/수정 파일 목록

### 새로 생성할 파일
```
src/ui/VirtualKeyboard.h            # 가상 키보드 헤더
src/ui/VirtualKeyboard.cpp          # 가상 키보드 구현
src/ui/URLBar.cpp                   # URLBar 구현 (헤더는 이미 존재 가정)
src/utils/URLValidator.cpp          # URL 검증 유틸리티 구현 (헤더는 이미 존재 가정)
resources/styles/urlbar.qss         # QSS 스타일 (선택적)
tests/unit/URLValidatorTest.cpp    # URLValidator 단위 테스트
tests/unit/VirtualKeyboardTest.cpp # VirtualKeyboard 단위 테스트
tests/integration/URLBarIntegrationTest.cpp  # URLBar 통합 테스트
```

### 수정할 파일
```
src/browser/BrowserWindow.h         # URLBar 멤버 변수 추가, 헤더 인클루드
src/browser/BrowserWindow.cpp       # setupUI, setupConnections 수정
CMakeLists.txt                      # 새 파일들 추가 (VirtualKeyboard, URLValidator)
tests/CMakeLists.txt                # 테스트 파일 추가
resources.qrc                       # QSS 리소스 추가 (선택적)
```

---

## 8. 완료 기준 체크리스트

### 기능 요구사항 (FR)
- [ ] FR-1: URL 입력 필드가 화면 상단에 표시되고, 리모컨 포커스 가능
- [ ] FR-2: 가상 키보드가 리모컨 방향키로 조작 가능, 모든 키 입력 지원
- [ ] FR-3: 자동완성 팝업이 히스토리/북마크에서 제안 표시 (F-07, F-08 완료 후)
- [ ] FR-4: URL 유효성 검증 및 자동 보완 (프로토콜 자동 추가)
- [ ] FR-5: Enter 키로 URL 제출 시 WebView에서 페이지 로드
- [ ] FR-6: 리모컨 키 매핑 (방향키, Select, Back, Enter) 정상 동작

### 비기능 요구사항 (NFR)
- [ ] 성능: 리모컨 입력 후 0.3초 이내 UI 반응
- [ ] 가독성: 폰트 크기 18px (입력 필드), 20px (가상 키보드)
- [ ] UX: 포커스된 요소가 3px 테두리로 명확히 표시
- [ ] 확장성: 향후 한글 키보드 추가 가능하도록 VirtualKeyboard 모듈화

### 테스트
- [ ] URLValidator 단위 테스트 통과
- [ ] VirtualKeyboard 단위 테스트 통과
- [ ] URLBar 통합 테스트 통과
- [ ] 실기기에서 리모컨 테스트 완료

### 문서화
- [ ] 코드 주석 작성 (한국어)
- [ ] requirements.md 완료 기준 (AC-1~AC-8) 검증
- [ ] design.md 설계 준수 확인
- [ ] CHANGELOG.md 업데이트 (F-03 완료 내역)
- [ ] dev-log.md 진행 로그 기록

---

## 9. 다음 단계 (F-03 완료 후)

### 즉시 진행 가능
- **F-04 (네비게이션 바)**: URLBar 완료 후 병렬 진행 가능 (독립적)
- **F-05 (로딩 인디케이터)**: WebView::loadStarted/loadFinished 시그널 연결만 필요 (독립적)

### F-03 의존 기능
- **F-09 (검색 엔진 통합)**: URLValidator::isSearchQuery()가 true일 때 검색 엔진 URL 생성
  - URLBar::validateAndCompleteUrl() 수정 필요
- **F-14 (HTTPS 보안 표시)**: URLBar에 보안 아이콘 추가
  - URLBar::inputLayout_에 securityIcon_ 위젯 추가

### 자동완성 기능 활성화
- **F-07 (북마크 관리) 완료 후**: BookmarkService::search() 연결
- **F-08 (히스토리 관리) 완료 후**: HistoryService::search() 연결
- URLBar::searchAutocomplete() 스켈레톤 코드 활성화

---

## 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | Native App (C++/Qt) 버전으로 재작성 | product-manager |
