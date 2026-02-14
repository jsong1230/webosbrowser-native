# URL 입력 UI — 요구사항 분석서 (Native App)

## 1. 개요

### 기능명
URL 입력 UI (F-03)

### 목적
리모컨만으로 웹 URL을 입력할 수 있는 사용자 인터페이스를 제공하여, 프로젝터 대화면 환경에서 편리하게 웹사이트에 접근할 수 있도록 합니다.

### 대상 사용자
- 리모컨으로 URL을 입력하여 웹 브라우징을 하고자 하는 프로젝터 사용자
- 마우스/키보드 없이 TV 리모컨만으로 웹 탐색을 원하는 일반 사용자
- 영상 스트리밍 사이트, 포털, 뉴스 사이트에 빠르게 접근하고자 하는 가정용 엔터테인먼트 사용자

### 요청 배경
- webOS 프로젝터는 물리적 키보드가 없어 URL 입력이 어려움
- 기존 브라우저는 마우스/터치 전제로 설계되어 리모컨 조작에 최적화되지 않음
- 3m 거리에서도 명확하게 보이는 대화면 최적화 입력 UI 필요
- 리모컨 5-way 방향키(상/하/좌/우/선택)로 모든 입력 가능해야 함
- **Native App 특성**: C++/Qt 기반으로 리모컨 키 이벤트를 직접 처리해야 함

---

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: URL 입력 필드
- **설명**: 사용자가 URL을 입력할 수 있는 텍스트 입력 필드를 제공합니다.
- **유저 스토리**: As a 프로젝터 사용자, I want URL 입력 필드를 리모컨으로 포커스하고 편집할 수 있기를 원합니다, so that 키보드 없이 웹사이트에 접근할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - URLBar 컴포넌트: Qt QLineEdit 또는 커스텀 QWidget 상속 클래스
  - Qt 포커스 정책 (setFocusPolicy): Qt::StrongFocus 설정
  - 리모컨 포커스 시 QSS(Qt Style Sheets)로 테두리 강조 표시
  - 현재 입력된 URL 또는 현재 페이지 URL 표시 (WebView::urlChanged 시그널 연결)
  - 입력 중일 때(편집 모드)와 비활성 상태 구분 (스타일 변경)
  - 최소 폰트 크기 18px (3m 거리 가독성) - QFont::setPointSize(18)
  - 입력 필드 너비: 화면 너비의 60% 이상 (QLayout 비율 설정)
  - 읽기 전용 모드 지원 (편집 모드 진입 전)

### FR-2: 가상 키보드 (리모컨 최적화)
- **설명**: 리모컨 방향키로 조작 가능한 가상 키보드를 제공합니다.
- **유저 스토리**: As a 리모컨 사용자, I want 방향키로 문자를 선택하고 입력할 수 있기를 원합니다, so that 물리 키보드 없이 URL을 입력할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - 키보드 레이아웃:
    - 영문 알파벳 (a-z, A-Z)
    - 숫자 (0-9)
    - 특수문자 (`.`, `/`, `-`, `:`, `?`, `=`, `&`, `_`, `@`)
    - 제어 키 (스페이스, 백스페이스, 확인/Enter, 취소)
  - 구현 방식:
    - **Option 1**: Qt Virtual Keyboard 플러그인 사용 (Qt 5.7+)
    - **Option 2**: 커스텀 QWidget 그리드 레이아웃 (QPushButton 배열)
    - **권장**: Option 2 (webOS 리모컨 키 이벤트 세밀한 제어 가능)
  - 그리드 레이아웃 (QWERTY 배열 또는 최적화된 배열) - QGridLayout 사용
  - 리모컨 상/하/좌/우로 키 간 포커스 이동 (keyPressEvent 오버라이드)
  - 선택 버튼(Qt::Key_Select 또는 Qt::Key_Enter)으로 문자 입력
  - 현재 포커스된 키 하이라이트 (두꺼운 테두리, 배경색 변화) - QSS 또는 paintEvent
  - 키 크기: 최소 60x60px (리모컨 조작 편의성) - setMinimumSize(60, 60)
  - 백 버튼(Qt::Key_Back)으로 가상 키보드 닫기
  - 가상 키보드는 QDialog 또는 QWidget으로 구현, 모달/비모달 선택 가능

### FR-3: URL 자동완성
- **설명**: 입력된 문자열과 일치하는 히스토리, 북마크 목록을 자동완성 제안으로 표시합니다.
- **유저 스토리**: As a 사용자, I want 이전에 방문했던 사이트를 빠르게 다시 접근할 수 있기를 원합니다, so that 전체 URL을 다시 입력하지 않아도 됩니다.
- **우선순위**: Should
- **상세 요구사항**:
  - 입력 중 히스토리, 북마크에서 일치하는 항목 검색
    - QLineEdit::textChanged 시그널에 연결
    - HistoryService, BookmarkService에서 검색 (F-07, F-08 의존)
  - 드롭다운 리스트로 자동완성 제안 표시 (최대 5개)
    - QCompleter 사용 또는 커스텀 QListWidget
    - URLBar 아래에 팝업 형태로 표시
  - 방향키 하(Qt::Key_Down)로 자동완성 리스트 포커스 이동
  - 선택 버튼(Qt::Key_Select)으로 제안 선택 → URL 필드에 채우기
  - 제안 항목: 타이틀 + URL 표시 (QListWidgetItem에 두 줄 표시)
  - 제안 없을 시 드롭다운 숨김 (QWidget::hide())

### FR-4: URL 유효성 검증 및 자동 보완
- **설명**: 입력된 문자열이 유효한 URL인지 검증하고, 필요 시 자동으로 프로토콜을 추가합니다.
- **유저 스토리**: As a 사용자, I want `http://`를 생략해도 웹사이트에 접근할 수 있기를 원합니다, so that 입력 시간을 절약할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - URL 형식 검증:
    - 도메인 형식: `example.com`, `www.example.com`
    - 프로토콜 포함: `http://`, `https://`
    - 경로 포함: `example.com/path?query=1`
    - QUrl::isValid()로 기본 검증
    - 정규표현식(QRegularExpression)으로 도메인 패턴 검증
  - 자동 보완 규칙:
    - 프로토콜 없으면 `http://` 자동 추가 (예: `google.com` → `http://google.com`)
      - QUrl::scheme().isEmpty() 체크 후 추가
    - 검색어로 판단 시 검색 엔진으로 쿼리 (F-09 검색 엔진 통합과 연계)
      - 공백 포함 또는 도메인 패턴 미일치 시 검색어로 판단
  - 유효하지 않은 입력 시 에러 메시지 표시 (예: "유효한 URL을 입력하세요")
    - QMessageBox 또는 URLBar 아래 QLabel로 에러 표시
    - 빨간색 폰트 + 아이콘

### FR-5: URL 입력 확인 및 페이지 로드
- **설명**: 사용자가 URL 입력을 완료하면 WebView 컴포넌트로 페이지를 로드합니다.
- **유저 스토리**: As a 사용자, I want URL 입력 후 확인 버튼을 누르면 해당 페이지가 로드되기를 원합니다, so that 입력한 사이트를 즉시 볼 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - 확인(Enter) 버튼(Qt::Key_Enter, Qt::Key_Return) 클릭 시 WebView::load(url) 호출
  - URL 로드 시작 시 로딩 인디케이터 표시 (F-05와 연계)
    - WebView::loadStarted 시그널 연결
  - 로드 성공 시 URL 입력 필드에 현재 페이지 URL 표시
    - WebView::urlChanged 시그널 연결하여 최종 URL 표시 (리다이렉트 고려)
  - 로드 실패 시 에러 메시지 표시 (F-10 에러 처리와 연계)
    - WebView::loadError 시그널 연결
  - ESC/취소 버튼(Qt::Key_Escape)으로 입력 취소 (이전 URL 복원)
    - 입력 전 URL을 멤버 변수에 저장 후 복원

### FR-6: 리모컨 키 매핑
- **설명**: webOS 리모컨 물리 버튼을 URL 입력 UI에 매핑합니다.
- **유저 스토리**: As a 리모컨 사용자, I want 리모컨 버튼만으로 URL 입력의 모든 기능을 사용할 수 있기를 원합니다, so that 추가 입력 장치 없이 브라우저를 사용할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - Qt 키 이벤트 처리 (QKeyEvent):
    - Qt::Key_Up / Qt::Key_Down / Qt::Key_Left / Qt::Key_Right: 가상 키보드 내 포커스 이동, 자동완성 리스트 이동
    - Qt::Key_Select (또는 Qt::Key_Enter): 키 입력, 자동완성 항목 선택, 확인 버튼 실행
    - Qt::Key_Back (또는 Qt::Key_Escape): 가상 키보드 닫기, URL 입력 취소
    - Qt::Key_Home: (선택) 홈 페이지로 이동 (설정에서 지정한 홈 URL)
  - keyPressEvent 오버라이드로 구현:
    ```cpp
    void URLBar::keyPressEvent(QKeyEvent *event) override {
        switch (event->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                // URL 로드
                break;
            case Qt::Key_Escape:
                // 입력 취소
                break;
            // ...
        }
    }
    ```
  - 포커스 체인 설정 (setTabOrder) 또는 커스텀 포커스 네비게이션 구현

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **입력 반응 속도**: 리모컨 버튼 입력 후 0.3초 이내에 UI 반응 (포커스 이동, 문자 입력)
  - Qt 이벤트 루프는 기본적으로 반응 속도가 빠르므로 특별한 최적화 불필요
  - 단, 자동완성 검색은 비동기 처리 (QThread 또는 QtConcurrent)
- **가상 키보드 렌더링 시간**: 키보드 호출 후 1초 이내 화면 표시
  - QWidget::show() 호출 즉시 표시 (레이아웃 사전 생성)
- **자동완성 검색 속도**: 문자 입력 후 0.5초 이내 제안 목록 표시
  - QTimer::singleShot(500ms)로 디바운싱 구현 (연속 입력 시 검색 지연)
  - LS2 API 호출 비동기 처리
- **메모리 사용량**: 가상 키보드 포함 URL 입력 UI는 최대 50MB 이하
  - Qt Widgets는 경량이므로 메모리 사용량 낮음
  - 가상 키보드는 싱글톤 패턴으로 재사용

### 가독성 (대화면 최적화)
- **폰트 크기**:
  - URL 입력 필드: 최소 18px (QFont::setPointSize(18))
  - 가상 키보드 키: 최소 20px (QFont::setPointSize(20))
  - 자동완성 제안: 최소 16px (QFont::setPointSize(16))
- **대비**: 배경과 전경 색상 대비 비율 4.5:1 이상 (WCAG AA 기준)
  - QSS로 색상 설정 (예: background: #1e1e1e; color: #ffffff;)
- **포커스 표시**: 포커스된 요소는 3px 두께 테두리 또는 명확한 배경색 변화
  - QSS: `border: 3px solid #00aaff;` (포커스 시)

### UX (리모컨 최적화)
- **포커스 경로**: 리모컨 방향키로 모든 UI 요소 접근 가능, 포커스 경로가 직관적
  - setTabOrder로 포커스 순서 지정
  - 또는 keyPressEvent에서 커스텀 포커스 네비게이션
- **키보드 레이아웃**: 사용 빈도 높은 문자(알파벳, 숫자)를 중앙에 배치
  - 3행 QWERTY 레이아웃 (키보드 표준 배열)
- **백스페이스 위치**: 키보드 우측 상단 또는 하단에 쉽게 접근 가능한 위치
  - 그리드 레이아웃 마지막 행 또는 우측 끝에 배치
- **에러 피드백**: 잘못된 입력 시 시각적 피드백 (빨간색 테두리, 에러 메시지)
  - QLineEdit::setStyleSheet("border: 2px solid red;")
  - URLBar 아래 에러 QLabel 표시

### 확장성
- **다국어 지원**: 향후 한글 키보드, 일본어 키보드 추가 가능하도록 키보드 레이아웃 모듈화
  - VirtualKeyboard 클래스를 추상 클래스로 설계
  - EnglishKeyboard, KoreanKeyboard 등 파생 클래스로 구현
- **키보드 테마**: 다크/라이트 모드 지원 가능하도록 스타일 분리
  - QSS 파일로 테마 분리 (dark.qss, light.qss)
  - 런타임에 setStyleSheet로 테마 전환

### 보안
- **입력 검증**: XSS 공격 방지를 위해 입력값 이스케이프 처리
  - QUrl::fromUserInput() 사용 (자동 이스케이프)
  - 또는 QString::toHtmlEscaped() 적용
- **HTTPS 우선**: URL 자동 보완 시 HTTPS 우선 제안 (F-14 보안 표시와 연계)
  - 자동 보완 시 https:// 우선 추가 (http://로 실패 시 https:// 재시도)

---

## 4. 제약조건

### webOS 플랫폼 제약사항
- **리모컨 전용 입력**: 마우스, 터치, 물리 키보드 사용 불가
  - Qt 키 이벤트만 사용 (QKeyEvent)
  - 마우스 이벤트(QMouseEvent)는 테스트 목적으로만 사용
- **5-way 방향키**: 상/하/좌/우/선택 버튼만으로 모든 입력 처리
  - Qt::Key_Up, Qt::Key_Down, Qt::Key_Left, Qt::Key_Right, Qt::Key_Select
- **화면 해상도**: 4K UHD (3840x2160) 기본, Full HD (1920x1080) 호환
  - Qt 스케일링 정책 사용 (setGeometry에 비율 기반 좌표)
  - 또는 QLayout으로 자동 스케일링

### 기술 제약사항
- **프레임워크**: Qt 5.15+ (webOS 기본 제공 버전)
  - Qt Widgets 사용 (QWidget, QLineEdit, QPushButton, QLayout)
  - Qt WebEngine (QWebEngineView) 또는 webOS WebView API
- **UI 컴포넌트**: Qt Widgets 표준 컴포넌트 우선 사용
  - QLineEdit (URL 입력)
  - QPushButton (가상 키보드 키)
  - QGridLayout (가상 키보드 레이아웃)
  - QCompleter 또는 QListWidget (자동완성)
- **스타일링**: Qt Style Sheets (QSS) 사용
  - CSS와 유사한 문법으로 스타일 정의
  - setStyleSheet() 또는 .qss 파일 로드
- **상태 관리**: C++ 멤버 변수 및 시그널/슬롯
  - Qt 시그널/슬롯 메커니즘으로 이벤트 전달
  - Q_PROPERTY로 데이터 바인딩 (선택사항)

### 의존성
- **F-01 (프로젝트 초기 설정)**: Qt 프로젝트 구조 및 CMake 빌드 설정 완료 필요
- **F-02 (웹뷰 통합)**: URL 로드를 위한 WebView 컴포넌트 API 필요
  - WebView::load(const QUrl &url) 메서드
  - WebView::urlChanged(const QUrl &url) 시그널
  - WebView::loadStarted(), loadFinished(), loadError() 시그널
- **F-08 (히스토리 관리)**: (선택) 자동완성 기능에서 히스토리 데이터 사용
  - HistoryService::search(const QString &query) 메서드
- **F-07 (북마크 관리)**: (선택) 자동완성 기능에서 북마크 데이터 사용
  - BookmarkService::search(const QString &query) 메서드

### 범위 외 (Out of Scope)
- **음성 입력**: 음성 인식을 통한 URL 입력 미지원
- **한글 키보드**: 이번 버전에서는 영문 키보드만 제공 (향후 확장 가능)
- **URL 히스토리 동기화**: 외부 브라우저와 히스토리 동기화 미지원
- **QR 코드 스캔**: 리모컨 카메라를 통한 QR 코드 URL 입력 미지원
- **IME (Input Method Editor)**: Qt 기본 IME는 리모컨 환경에서 부적합하므로 사용하지 않음

---

## 5. 용어 정의

| 용어 | 정의 |
|------|------|
| **URLBar** | URL을 입력하고 표시하는 Qt 위젯 (QLineEdit 기반 또는 커스텀 QWidget) |
| **가상 키보드** | 리모컨으로 조작 가능한 화면상 키보드 UI (QPushButton 그리드) |
| **Qt 포커스** | Qt의 포커스 시스템 (QWidget::setFocus, focusInEvent, focusOutEvent) |
| **QSS** | Qt Style Sheets - CSS와 유사한 Qt 스타일링 언어 |
| **5-way 방향키** | webOS 리모컨의 상/하/좌/우/선택 버튼 (Qt::Key_Up, Down, Left, Right, Select) |
| **자동완성** | 입력된 문자열과 일치하는 히스토리, 북마크를 제안하는 기능 (QCompleter 또는 커스텀) |
| **URL 검증** | 입력된 문자열이 유효한 URL 형식인지 확인하는 프로세스 (QUrl::isValid) |
| **프로토콜 자동 추가** | `example.com` 입력 시 `http://example.com`으로 변환하는 기능 (QUrl::scheme) |
| **PIMPL** | Pointer to Implementation 패턴 (헤더 의존성 감소) |
| **시그널/슬롯** | Qt의 이벤트 전달 메커니즘 (connect 함수로 연결) |

---

## 6. 완료 기준 (Acceptance Criteria)

### AC-1: URL 입력 필드 렌더링
- [ ] URLBar 컴포넌트가 화면 상단에 표시됨 (QLineEdit 또는 커스텀 QWidget)
- [ ] 리모컨 방향키로 URL 입력 필드에 포커스 이동 가능 (setFocusPolicy)
- [ ] 포커스 시 명확한 테두리 또는 배경색 하이라이트 표시 (QSS 또는 focusInEvent)
- [ ] 현재 페이지 URL이 입력 필드에 표시됨 (WebView::urlChanged 연결)

### AC-2: 가상 키보드 동작
- [ ] URL 입력 필드 선택 시 가상 키보드가 화면 하단에 표시됨 (QDialog 또는 QWidget)
- [ ] 리모컨 방향키로 키보드 내 모든 키에 포커스 이동 가능 (keyPressEvent 구현)
- [ ] 선택 버튼(Qt::Key_Select)으로 문자 입력 시 URL 필드에 반영됨 (시그널/슬롯)
- [ ] 백스페이스 키로 마지막 문자 삭제 가능 (QLineEdit::backspace 또는 텍스트 조작)
- [ ] 백 버튼(Qt::Key_Back)으로 가상 키보드 닫기 가능 (QWidget::hide 또는 QDialog::reject)

### AC-3: URL 유효성 검증 및 자동 보완
- [ ] `google.com` 입력 시 `http://google.com`으로 자동 변환 (QUrl::scheme 체크)
- [ ] `https://example.com` 입력 시 프로토콜 유지
- [ ] 유효하지 않은 입력 시 에러 메시지 표시 (QMessageBox 또는 에러 QLabel)
- [ ] 검색어 입력 시 검색 엔진으로 쿼리 (F-09와 연계)

### AC-4: 페이지 로드
- [ ] 확인(Enter) 버튼 클릭 시 WebView::load(url) 호출
- [ ] 로드 성공 시 입력 필드에 최종 URL 표시 (WebView::urlChanged 연결)
- [ ] 로드 실패 시 에러 메시지 표시 (WebView::loadError 연결)
- [ ] ESC/취소 버튼으로 입력 취소 가능 (이전 URL 복원)

### AC-5: 자동완성 (선택적)
- [ ] 문자 입력 시 일치하는 히스토리, 북마크 제안 표시 (QCompleter 또는 QListWidget)
- [ ] 방향키 하(Qt::Key_Down)로 제안 리스트 포커스 이동
- [ ] 선택 버튼으로 제안 선택 시 URL 필드에 채워짐 (setEditText)
- [ ] 제안 없을 시 드롭다운 숨김 (QWidget::hide)

### AC-6: 리모컨 키 매핑
- [ ] 방향키로 가상 키보드 및 자동완성 리스트 탐색 가능 (keyPressEvent 구현)
- [ ] 선택 버튼(Qt::Key_Select)으로 문자 입력 및 확인 가능
- [ ] 백 버튼(Qt::Key_Back)으로 키보드 닫기 가능
- [ ] 모든 기능을 마우스 없이 리모컨만으로 사용 가능 (키보드 이벤트 테스트)

### AC-7: 성능 및 가독성
- [ ] 리모컨 입력 후 0.3초 이내 UI 반응 (Qt 이벤트 루프 지연 측정)
- [ ] 폰트 크기가 3m 거리에서 가독 가능 (최소 18px) (setPointSize 확인)
- [ ] 포커스된 요소가 명확하게 구분됨 (QSS 또는 paintEvent 확인)

### AC-8: 회귀 테스트
- [ ] 주요 웹사이트 URL 입력 후 정상 로드 (google.com, naver.com, youtube.com)
- [ ] 긴 URL 입력 시 필드 스크롤 또는 말줄임(...) 표시 (QLineEdit::ElidedText)
- [ ] 특수문자 포함 URL 정상 입력 및 로드 (예: `example.com/path?q=test&lang=ko`)

---

## 7. 참고 사항

### 관련 문서
- PRD: `docs/project/prd.md`
- 기능 백로그: `docs/project/features.md`
- CLAUDE.md: 프로젝트 규칙 및 코딩 컨벤션
- F-02 WebView 헤더: `src/browser/WebView.h`

### 참고 구현
- Qt QLineEdit: https://doc.qt.io/qt-5/qlineedit.html
- Qt QCompleter: https://doc.qt.io/qt-5/qcompleter.html
- Qt Style Sheets: https://doc.qt.io/qt-5/stylesheet.html
- Qt Virtual Keyboard: https://doc.qt.io/qt-5/qtvirtualkeyboard-index.html
- Qt Key Events: https://doc.qt.io/qt-5/qkeyevent.html

### 우선순위 조정 가능성
- 자동완성 기능 (FR-3)은 Should 우선순위로, F-07, F-08 완료 후 추가 가능
- 한글 키보드는 M3 이후 확장 기능으로 연기 가능
- Qt Virtual Keyboard 플러그인 대신 커스텀 키보드 구현 권장 (리모컨 최적화)

### 구현 권장사항
- **가상 키보드**: Qt Virtual Keyboard보다 커스텀 QPushButton 그리드 권장
  - 이유: webOS 리모컨 키 이벤트를 세밀하게 제어 가능
  - Qt Virtual Keyboard는 터치/마우스 최적화되어 있음
- **자동완성**: QCompleter는 마우스 환경에 최적화되어 있으므로, 리모컨 환경에서는 커스텀 QListWidget 구현 권장
- **포커스 관리**: setTabOrder보다 keyPressEvent에서 커스텀 포커스 네비게이션 구현 권장
  - 이유: 그리드 레이아웃에서 상/하/좌/우 이동이 직관적
- **스타일링**: QSS 파일로 분리하여 런타임에 테마 전환 가능하도록 구현

---

## 8. 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|-----------|--------|
| 2026-02-14 | 1.0 | Native App (Qt/C++) 관점으로 요구사항 재작성 | product-manager |
