# F-11: 설정 화면 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/settings/requirements.md
- 기술 설계서: docs/specs/settings/design.md
- F-09 검색 엔진 통합: docs/specs/search-engine-integration/
- F-07 북마크 관리: docs/specs/bookmark-management/
- F-08 히스토리 관리: docs/specs/history-management/
- F-13 리모컨 단축키: docs/specs/remote-shortcuts/

---

## 2. 구현 Phase

### Phase 1: SettingsService 서비스 계층 구현
**담당**: cpp-dev
**예상 소요 시간**: 1.5시간
**복잡도**: Medium

#### 태스크
- [ ] Task 1-1: SettingsService.h 헤더 파일 작성
  - 클래스 선언 (QObject 상속)
  - public 메서드: loadSettings(), getters (searchEngine, homepage, theme)
  - public 메서드: setters (setSearchEngine, setHomepage, setTheme)
  - signals: searchEngineChanged, homepageChanged, themeChanged, settingsLoaded
  - private: StorageService 포인터, 메모리 캐시 변수 (searchEngine_, homepage_, theme_)
  - private: 상수 (DEFAULT_SEARCH_ENGINE, DEFAULT_HOMEPAGE, DEFAULT_THEME, STORAGE_ID)

- [ ] Task 1-2: SettingsService.cpp 기본 구조 구현
  - 생성자/소멸자 구현
  - StorageService 의존성 주입
  - getter 메서드 구현 (메모리 캐시 반환)

- [ ] Task 1-3: loadSettings() 구현
  - StorageService::getData() 호출 (DataKind::Settings, "browser_settings")
  - 성공 시: JSON 파싱 → 메모리 캐시 초기화 → emit settingsLoaded()
  - 실패 시: 기본값 사용 → 메모리 캐시 초기화 → saveToStorage() 호출

- [ ] Task 1-4: setSearchEngine() 구현
  - SearchEngine::getAllSearchEngines()로 유효성 검증
  - 메모리 캐시 업데이트 (searchEngine_ = engineId)
  - SearchEngine::setDefaultSearchEngine(engineId) 호출
  - saveToStorage() 호출
  - 성공 시 emit searchEngineChanged(engineId)

- [ ] Task 1-5: setHomepage() 구현
  - URLValidator::isValid(url) 검증
  - 위험 프로토콜 차단 (javascript:, file://)
  - about: URL 허용
  - 메모리 캐시 업데이트 (homepage_ = url)
  - saveToStorage() 호출
  - 성공 시 emit homepageChanged(url)

- [ ] Task 1-6: setTheme() 구현
  - 유효성 검증 (theme == "dark" || theme == "light")
  - 메모리 캐시 업데이트 (theme_ = themeId)
  - saveToStorage() 호출
  - 성공 시 emit themeChanged(themeId)

- [ ] Task 1-7: saveToStorage() 구현
  - QJsonObject 생성 (searchEngine, homepage, theme)
  - StorageService::putData(DataKind::Settings, "browser_settings", json) 호출
  - 반환값 체크 (성공/실패)

**예상 산출물**:
- `src/services/SettingsService.h`
- `src/services/SettingsService.cpp`

**완료 기준**:
- SettingsService 클래스가 컴파일 가능
- loadSettings() 호출 시 기본값 또는 LS2 데이터 로드
- setSearchEngine/Homepage/Theme() 호출 시 LS2 저장 및 시그널 발생
- 시그널 발생 확인 (QSignalSpy 또는 수동 테스트)

---

### Phase 2: SettingsPanel UI 계층 구현 (기본 구조)
**담당**: cpp-dev
**예상 소요 시간**: 2시간
**복잡도**: High

#### 태스크
- [ ] Task 2-1: SettingsPanel.h 헤더 파일 작성
  - QWidget 상속
  - public: showPanel(), hidePanel() 메서드
  - signals: panelClosed()
  - protected: keyPressEvent() 오버라이드
  - private slots: onSearchEngineRadioToggled, onHomepageEditingFinished, onThemeRadioToggled 등
  - private: setupUI(), setupConnections(), setupFocusOrder() 메서드
  - private: UI 위젯 포인터 (라디오 버튼, 입력 필드, 버튼 등)
  - private: SettingsService, BookmarkService, HistoryService 포인터
  - private: QPropertyAnimation 포인터 (슬라이드 애니메이션)

- [ ] Task 2-2: SettingsPanel.cpp 생성자 및 setupUI() 구현
  - 생성자: 의존성 주입 (SettingsService, BookmarkService, HistoryService)
  - setupUI() 호출: 메인 레이아웃(QVBoxLayout) 생성
  - contentWidget_ (QWidget) 생성 및 레이아웃 설정
  - QFormLayout 생성 (라벨 + 입력 필드 정렬)

- [ ] Task 2-3: 검색 엔진 섹션 UI 구현
  - QLabel ("검색 엔진:") 추가
  - QRadioButton 4개 생성 (Google, Naver, Bing, DuckDuckGo)
  - QButtonGroup으로 라디오 버튼 그룹화
  - QHBoxLayout으로 라디오 버튼 가로 정렬
  - formLayout에 추가

- [ ] Task 2-4: 홈페이지 섹션 UI 구현
  - QLabel ("홈페이지 URL:") 추가
  - QLineEdit (homepageInput_) 생성 및 placeholder 설정
  - QLabel (homepageErrorLabel_) 생성 (에러 메시지용, 초기 숨김)
  - formLayout에 추가

- [ ] Task 2-5: 테마 섹션 UI 구현
  - QLabel ("테마:") 추가
  - QRadioButton 2개 생성 (다크 모드, 라이트 모드)
  - QButtonGroup으로 라디오 버튼 그룹화
  - QHBoxLayout으로 라디오 버튼 가로 정렬
  - formLayout에 추가

- [ ] Task 2-6: 브라우징 데이터 삭제 섹션 UI 구현
  - QLabel ("브라우징 데이터 삭제:") 추가
  - QPushButton 2개 생성 ("북마크 전체 삭제", "히스토리 전체 삭제")
  - 버튼 스타일: 빨간색 계열 (경고 표시)
  - QHBoxLayout으로 버튼 가로 정렬
  - formLayout에 추가

- [ ] Task 2-7: 닫기 버튼 및 최종 레이아웃
  - QPushButton (closeButton_) 생성 ("닫기")
  - contentLayout 하단에 추가
  - contentWidget_를 mainLayout_에 추가
  - 패널 초기 상태: hide()

**예상 산출물**:
- `src/ui/SettingsPanel.h`
- `src/ui/SettingsPanel.cpp` (UI 구조만, 동작 미구현)

**완료 기준**:
- SettingsPanel UI가 화면에 표시됨 (showPanel() 호출 시)
- 모든 UI 컴포넌트 (라디오 버튼, 입력 필드, 버튼)가 정상 렌더링
- 레이아웃이 디자인 시안과 일치

---

### Phase 3: SettingsPanel 동작 구현 (시그널/슬롯 연결)
**담당**: cpp-dev
**예상 소요 시간**: 1.5시간
**복잡도**: Medium

#### 태스크
- [ ] Task 3-1: setupConnections() 구현
  - 검색 엔진 라디오 버튼 toggled 시그널 → onSearchEngineRadioToggled 슬롯 연결
  - 홈페이지 입력 필드 editingFinished 시그널 → onHomepageEditingFinished 슬롯 연결
  - 테마 라디오 버튼 toggled 시그널 → onThemeRadioToggled 슬롯 연결
  - 북마크 삭제 버튼 clicked 시그널 → onClearBookmarksClicked 슬롯 연결
  - 히스토리 삭제 버튼 clicked 시그널 → onClearHistoryClicked 슬롯 연결
  - 닫기 버튼 clicked 시그널 → onCloseButtonClicked 슬롯 연결
  - SettingsService::settingsLoaded 시그널 → onSettingsLoaded 슬롯 연결

- [ ] Task 3-2: onSearchEngineRadioToggled() 구현
  - sender()로 클릭된 라디오 버튼 식별
  - engineId 추출 (google, naver, bing, duckduckgo)
  - SettingsService::setSearchEngine(engineId) 호출

- [ ] Task 3-3: onHomepageEditingFinished() 구현
  - homepageInput_->text() 가져오기
  - URLValidator::isValid(url) 검증
  - 실패 시: homepageErrorLabel_ 표시 ("유효하지 않은 URL입니다.")
  - 성공 시: SettingsService::setHomepage(url) 호출
  - 저장 성공 시: homepageErrorLabel_ 숨김 + showToast("홈페이지가 설정되었습니다.")

- [ ] Task 3-4: onThemeRadioToggled() 구현
  - sender()로 클릭된 라디오 버튼 식별
  - themeId 추출 (dark, light)
  - SettingsService::setTheme(themeId) 호출

- [ ] Task 3-5: onClearBookmarksClicked() 구현
  - showConfirmDialog("모든 북마크를 삭제하시겠습니까?...") 호출
  - 사용자가 "삭제" 선택 시:
    - BookmarkService::getAllBookmarks() 호출 (비동기 콜백)
    - 콜백에서 모든 북마크 ID에 대해 deleteBookmark() 호출
    - showToast("북마크가 삭제되었습니다.")

- [ ] Task 3-6: onClearHistoryClicked() 구현
  - showConfirmDialog("모든 히스토리를 삭제하시겠습니까?...") 호출
  - 사용자가 "삭제" 선택 시:
    - HistoryService::getAllHistory() 호출 (비동기 콜백)
    - 콜백에서 모든 히스토리 ID에 대해 deleteHistory() 호출
    - showToast("히스토리가 삭제되었습니다.")

- [ ] Task 3-7: onCloseButtonClicked() 구현
  - hidePanel() 호출

- [ ] Task 3-8: onSettingsLoaded() 구현
  - loadCurrentSettings() 호출
  - SettingsService에서 현재 설정 가져오기
  - 각 UI 위젯에 값 반영 (라디오 버튼 체크, 입력 필드 값 설정)

- [ ] Task 3-9: 헬퍼 메서드 구현
  - showConfirmDialog(message): QMessageBox::warning() 사용, Yes/No 버튼, 반환값 bool
  - showToast(message): QLabel 또는 QMessageBox::information() 사용
  - loadCurrentSettings(): SettingsService getter 호출 → UI 반영

**완료 기준**:
- 검색 엔진 라디오 버튼 클릭 시 SettingsService::searchEngineChanged 시그널 발생
- 홈페이지 입력 후 Enter 키 → 저장 또는 에러 메시지 표시
- 테마 라디오 버튼 클릭 시 SettingsService::themeChanged 시그널 발생
- 브라우징 데이터 삭제 버튼 클릭 시 확인 다이얼로그 → 삭제 실행
- 닫기 버튼 클릭 시 패널 숨김

---

### Phase 4: SettingsPanel 애니메이션 및 리모컨 네비게이션
**담당**: cpp-dev
**예상 소요 시간**: 1시간
**복잡도**: Medium

#### 태스크
- [ ] Task 4-1: showPanel() 애니메이션 구현
  - QPropertyAnimation (slideAnimation_) 생성 (targetObject: contentWidget_, property: "geometry")
  - startValue: 화면 오른쪽 밖 (x = parentWidget()->width())
  - endValue: 화면 오른쪽에서 슬라이드 인 (x = parentWidget()->width() - 400 - 20)
  - duration: 300ms, easing: QEasingCurve::OutCubic
  - show() → raise() → slideAnimation_->start()
  - 첫 번째 항목에 포커스 (googleRadio_->setFocus())

- [ ] Task 4-2: hidePanel() 애니메이션 구현
  - slideAnimation_ 설정
  - startValue: 현재 위치
  - endValue: 화면 오른쪽 밖
  - 애니메이션 finished 시그널 → hide() + emit panelClosed()

- [ ] Task 4-3: setupFocusOrder() 구현
  - setTabOrder()로 포커스 순서 설정
  - 순서: googleRadio → naverRadio → bingRadio → duckduckgoRadio → homepageInput → darkThemeRadio → lightThemeRadio → clearBookmarksButton → clearHistoryButton → closeButton
  - 모든 위젯에 setFocusPolicy(Qt::StrongFocus) 설정

- [ ] Task 4-4: keyPressEvent() 구현
  - Qt::Key_Escape 또는 Qt::Key_Back 감지 → hidePanel() 호출
  - 기타 키는 QWidget::keyPressEvent() 호출 (Qt 기본 Tab Order 처리)

**완료 기준**:
- showPanel() 호출 시 슬라이드 인 애니메이션 실행 (부드러운 전환)
- hidePanel() 호출 시 슬라이드 아웃 애니메이션 실행
- 리모컨 방향키 (Up/Down)로 항목 간 포커스 이동
- 리모컨 Back 키로 패널 닫기
- 포커스 표시 명확 (테두리 강조)

---

### Phase 5: 테마 시스템 구현 (QSS 파일)
**담당**: cpp-dev
**예상 소요 시간**: 0.5시간
**복잡도**: Low

#### 태스크
- [ ] Task 5-1: dark.qss 파일 작성
  - `resources/styles/dark.qss` 생성
  - QMainWindow, QWidget, QPushButton, QLineEdit, QRadioButton, QLabel 스타일 정의
  - 색상: 배경 #1E1E1E, 텍스트 #FFFFFF, 포커스 테두리 #0078D7

- [ ] Task 5-2: light.qss 파일 작성
  - `resources/styles/light.qss` 생성
  - QMainWindow, QWidget, QPushButton, QLineEdit, QRadioButton, QLabel 스타일 정의
  - 색상: 배경 #FFFFFF, 텍스트 #000000, 포커스 테두리 #0078D7

- [ ] Task 5-3: resources.qrc 파일 작성
  - `resources/resources.qrc` 생성
  - `<qresource prefix="/styles">` 설정
  - dark.qss, light.qss 파일 등록

**예상 산출물**:
- `resources/styles/dark.qss`
- `resources/styles/light.qss`
- `resources/resources.qrc`

**완료 기준**:
- QRC 파일이 CMake에 의해 빌드됨
- `:/styles/dark.qss`, `:/styles/light.qss` 경로로 접근 가능
- 각 QSS 파일에 모든 위젯 스타일 정의됨

---

### Phase 6: BrowserWindow 통합
**담당**: cpp-dev
**예상 소요 시간**: 1시간
**복잡도**: Medium

#### 태스크
- [ ] Task 6-1: BrowserWindow.h 수정
  - Forward declarations에 SettingsService, SettingsPanel 추가
  - private 멤버 변수: SettingsService *settingsService_, SettingsPanel *settingsPanel_ 추가
  - private slots: onSearchEngineChanged, onHomepageChanged, onThemeChanged 추가
  - private 메서드: applyTheme(const QString &themeId) 추가

- [ ] Task 6-2: BrowserWindow.cpp 생성자 수정
  - SettingsService 생성 (new SettingsService(storageService_, this))
  - settingsService_->loadSettings() 호출
  - SettingsPanel 생성 (new SettingsPanel(settingsService_, bookmarkService_, historyService_, this))
  - 초기 테마 적용: applyTheme(settingsService_->theme())

- [ ] Task 6-3: setupConnections() 수정
  - SettingsService::searchEngineChanged → BrowserWindow::onSearchEngineChanged 연결
  - SettingsService::homepageChanged → BrowserWindow::onHomepageChanged 연결
  - SettingsService::themeChanged → BrowserWindow::onThemeChanged 연결

- [ ] Task 6-4: handleMenuButton() 수정
  - 기존 TODO 주석 제거
  - settingsPanel_->isVisible() 체크
  - true이면 settingsPanel_->hidePanel(), false이면 settingsPanel_->showPanel()

- [ ] Task 6-5: onHomepageChanged() 구현
  - navigationBar_->setHomepage(url) 호출

- [ ] Task 6-6: onThemeChanged() 구현
  - applyTheme(themeId) 호출

- [ ] Task 6-7: applyTheme() 구현
  - QString qssFilePath = QString(":/styles/%1.qss").arg(themeId)
  - QFile qssFile(qssFilePath) 열기
  - readAll()로 QSS 내용 읽기
  - qApp->setStyleSheet(styleSheet) 호출
  - 실패 시 qWarning() 출력

**완료 기준**:
- BrowserWindow 생성 시 SettingsService, SettingsPanel 정상 초기화
- Menu 버튼 클릭 시 SettingsPanel 열기/닫기
- 설정 변경 시 시그널/슬롯 연결 정상 동작
- 테마 변경 시 applyTheme() 호출 → 전역 스타일시트 적용

---

### Phase 7: NavigationBar 수정 (홈페이지 동적 설정)
**담당**: cpp-dev
**예상 소요 시간**: 0.5시간
**복잡도**: Low

#### 태스크
- [ ] Task 7-1: NavigationBar.h 수정
  - public 메서드 추가: void setHomepage(const QString &url)
  - private 멤버 변수: QString homepage_ 추가
  - static const QString DEFAULT_HOME_URL 상수 삭제

- [ ] Task 7-2: NavigationBar.cpp 수정
  - 생성자에서 homepage_ 초기화 (기본값 "https://www.google.com")
  - setHomepage() 구현: homepage_ = url
  - onHomeClicked() 수정: webView_->load(QUrl(homepage_)) 호출 (하드코딩된 URL 제거)

**완료 기준**:
- NavigationBar::setHomepage() 호출 시 멤버 변수 업데이트
- 홈 버튼 클릭 시 설정된 홈페이지로 이동
- DEFAULT_HOME_URL 상수 제거됨

---

### Phase 8: CMakeLists.txt 및 빌드 설정
**담당**: cpp-dev
**예상 소요 시간**: 0.5시간
**복잡도**: Low

#### 태스크
- [ ] Task 8-1: CMakeLists.txt 소스 파일 추가
  - set(SOURCES ...) 블록에 src/services/SettingsService.cpp 추가
  - set(SOURCES ...) 블록에 src/ui/SettingsPanel.cpp 추가

- [ ] Task 8-2: CMakeLists.txt 헤더 파일 추가
  - set(HEADERS ...) 블록에 src/services/SettingsService.h 추가
  - set(HEADERS ...) 블록에 src/ui/SettingsPanel.h 추가

- [ ] Task 8-3: CMakeLists.txt QRC 리소스 추가
  - set(RESOURCES resources/resources.qrc) 블록 추가
  - add_executable(...) 호출에 ${RESOURCES} 인자 추가

- [ ] Task 8-4: 빌드 테스트
  - mkdir build && cd build
  - cmake ..
  - make
  - 빌드 에러 없는지 확인

**완료 기준**:
- CMakeLists.txt에 모든 소스/헤더 파일 등록됨
- QRC 리소스 파일이 빌드 시스템에 포함됨
- 빌드 성공 (webosbrowser 실행 파일 생성)
- 빌드 경고 없음

---

### Phase 9: 단위 테스트 및 통합 테스트
**담당**: test-runner
**예상 소요 시간**: 1시간
**복잡도**: Medium

#### 태스크
- [ ] Task 9-1: SettingsService 단위 테스트 작성
  - tests/unit/SettingsServiceTest.cpp 생성
  - 테스트: loadSettings() 기본값 로드
  - 테스트: setSearchEngine() 유효한 엔진 설정
  - 테스트: setSearchEngine() 유효하지 않은 엔진 거부
  - 테스트: setHomepage() 유효한 URL 설정
  - 테스트: setHomepage() 유효하지 않은 URL 거부
  - 테스트: setTheme() 유효한 테마 설정
  - 테스트: setTheme() 유효하지 않은 테마 거부
  - 테스트: searchEngineChanged 시그널 발생

- [ ] Task 9-2: SettingsPanel UI 통합 테스트
  - tests/integration/SettingsPanelTest.cpp 생성
  - 테스트: showPanel() 호출 시 패널 표시
  - 테스트: hidePanel() 호출 시 패널 숨김
  - 테스트: 검색 엔진 라디오 버튼 클릭 → SettingsService 호출
  - 테스트: 홈페이지 입력 후 Enter → 저장
  - 테스트: 테마 라디오 버튼 클릭 → 테마 변경
  - 테스트: 리모컨 Back 키 → 패널 닫기

- [ ] Task 9-3: BrowserWindow 통합 테스트
  - tests/integration/BrowserWindowTest.cpp 수정
  - 테스트: Menu 버튼 클릭 → SettingsPanel 토글
  - 테스트: 설정 변경 → BrowserWindow 슬롯 호출
  - 테스트: applyTheme() → 스타일시트 적용

- [ ] Task 9-4: E2E 테스트 시나리오
  - 앱 시작 → 기본 설정 로드
  - Menu 버튼 → 설정 패널 열림
  - 검색 엔진 변경 (Naver) → URLBar 검색 시 반영
  - 홈페이지 설정 ("https://www.naver.com") → 홈 버튼 클릭 시 이동
  - 테마 변경 (라이트 모드) → UI 즉시 업데이트
  - 브라우징 데이터 삭제 → 확인 다이얼로그 → 실제 삭제
  - 앱 재시작 → 설정 유지 확인

**완료 기준**:
- 모든 단위 테스트 통과
- 모든 통합 테스트 통과
- E2E 테스트 시나리오 수동 검증 완료
- 테스트 커버리지 80% 이상 (SettingsService, SettingsPanel)

---

## 3. 태스크 의존성 다이어그램

```
Phase 1 (SettingsService) ──▶ Phase 2 (SettingsPanel UI) ──▶ Phase 3 (SettingsPanel 동작)
                                                               │
                                                               ▼
                                                           Phase 4 (애니메이션)
                                                               │
                                                               ▼
Phase 5 (QSS 테마) ──────────────────────────────────────▶ Phase 6 (BrowserWindow 통합)
                                                               │
                                                               ▼
                                                           Phase 7 (NavigationBar 수정)
                                                               │
                                                               ▼
                                                           Phase 8 (빌드 설정)
                                                               │
                                                               ▼
                                                           Phase 9 (테스트)
```

### 병렬 실행 가능 태스크
- Phase 1 (SettingsService) 과 Phase 5 (QSS 테마) 는 독립적이므로 병렬 가능
- Phase 2, 3, 4 (SettingsPanel) 는 순차 실행 필요 (UI 구조 → 동작 → 애니메이션)
- Phase 6, 7 (통합) 은 Phase 1-5 완료 후 순차 실행 필요

---

## 4. 실행 모드 판단

### 병렬 실행 가능성 분석
- **Phase 독립성**: Phase 1 (SettingsService)과 Phase 5 (QSS 테마)는 독립적이지만, 나머지 Phase는 순차 의존성이 강함
- **파일 충돌 위험**:
  - BrowserWindow.h/.cpp는 Phase 6에서만 수정 (충돌 없음)
  - NavigationBar.h/.cpp는 Phase 7에서만 수정 (충돌 없음)
  - SettingsPanel은 Phase 2-4에서 순차 수정 (충돌 위험)
- **태스크 수**: 총 40개 태스크 (병렬화 이점 있음)
- **복잡도**: Phase 2-4는 UI 컴포넌트 구현으로 순차 디버깅 필요

### 결론: 순차 실행 권장
**이유**:
1. SettingsPanel UI 구현 (Phase 2-4)이 전체 작업의 50%를 차지하며, 순차 디버깅이 필수
2. Phase 6-8 (통합 및 빌드)은 Phase 1-5 완료 후 실행해야 함
3. Phase 1과 Phase 5만 병렬 가능하지만, 병렬화 오버헤드 대비 이점 미미
4. 단일 개발자(cpp-dev)가 순차 작업 시 컨텍스트 스위칭 비용 없음
5. 설정 화면은 단일 서비스 + 단일 UI 컴포넌트로 복잡도 낮음

**권장 실행 방식**:
```bash
# 순차 파이프라인 실행
/fullstack-feature F-11 설정 화면
```

**병렬 실행 시 주의사항** (필요 시):
- Phase 1과 Phase 5를 병렬로 시작
- Phase 2-4는 Phase 1 완료 후 순차 실행
- Phase 6-8은 Phase 1-5 완료 후 순차 실행

---

## 5. 예상 복잡도 및 소요 시간

| Phase | 복잡도 | 예상 시간 | 주요 작업 |
|-------|--------|-----------|-----------|
| Phase 1 | Medium | 1.5시간 | SettingsService 서비스 계층 (7개 태스크) |
| Phase 2 | High | 2시간 | SettingsPanel UI 기본 구조 (7개 태스크) |
| Phase 3 | Medium | 1.5시간 | SettingsPanel 시그널/슬롯 연결 (9개 태스크) |
| Phase 4 | Medium | 1시간 | 애니메이션 및 리모컨 네비게이션 (4개 태스크) |
| Phase 5 | Low | 0.5시간 | QSS 테마 파일 작성 (3개 태스크) |
| Phase 6 | Medium | 1시간 | BrowserWindow 통합 (7개 태스크) |
| Phase 7 | Low | 0.5시간 | NavigationBar 수정 (2개 태스크) |
| Phase 8 | Low | 0.5시간 | CMakeLists.txt 빌드 설정 (4개 태스크) |
| Phase 9 | Medium | 1시간 | 단위 테스트 및 통합 테스트 (4개 태스크) |
| **총합** | **-** | **10시간** | **47개 태스크** |

### 복잡도 기준
- **Low**: 단순 파일 작성, 설정 수정 (에러 가능성 낮음)
- **Medium**: 비즈니스 로직 구현, 시그널/슬롯 연결 (디버깅 필요)
- **High**: 복잡한 UI 구현, 여러 위젯 조합 (반복 테스트 필요)

---

## 6. 위험 요소 및 대응 방안

| 위험 요소 | 영향도 | 대응 방안 |
|-----------|--------|-----------|
| **LS2 API 비동기 처리 복잡성** | Medium | StorageService 메서드 재사용, QTimer 사용, 메모리 캐시로 즉시 반영 |
| **QSS 스타일시트 제약** | Low | QSS로 스타일링 불가능한 위젯은 QPalette 폴백 사용 |
| **리모컨 Tab Order 설정 복잡** | Medium | BookmarkPanel, HistoryPanel 구현 참고, setupFocusOrder() 메서드로 명시적 설정 |
| **URLValidator 특수 URL 처리** | Low | about:blank, about:favorites 등 화이트리스트 방식 허용, 단위 테스트로 검증 |
| **애니메이션 성능 문제** | Low | QPropertyAnimation은 GPU 가속 지원, duration 300ms로 부드러움 보장 |
| **BrowserWindow 통합 시 기존 코드 충돌** | Medium | handleMenuButton()만 수정, 기존 코드 최소 변경, 회귀 테스트 필수 |
| **테마 변경 시 일부 위젯 미반영** | Low | qApp->setStyleSheet() 전역 적용, 필요 시 개별 위젯 update() 호출 |

---

## 7. 의존성 체크리스트

### 기능 의존성
- [x] **F-09 (검색 엔진 통합)**: SearchEngine::getAllSearchEngines(), setDefaultSearchEngine() 사용 가능
- [x] **F-07 (북마크 관리)**: BookmarkService::getAllBookmarks(), deleteBookmark() 사용 가능
- [x] **F-08 (히스토리 관리)**: HistoryService::getAllHistory(), deleteHistory() 사용 가능
- [x] **F-13 (리모컨 단축키)**: BrowserWindow::handleMenuButton() 이미 구현됨, Menu 버튼 키 코드 처리 완료
- [x] **F-01 (프로젝트 초기 설정)**: StorageService (LS2 API 래퍼) 사용 가능, DataKind::Settings 지원
- [x] **F-03 (URL 검증)**: URLValidator::isValid() 사용 가능 (홈페이지 URL 검증)

### 코드 의존성
- [x] **StorageService 클래스**: putData(), getData() 메서드 구현 완료
- [x] **SearchEngine 클래스**: 정적 메서드 구현 완료 (F-09)
- [x] **URLValidator 클래스**: isValid() 메서드 구현 완료 (F-03)
- [x] **BrowserWindow 클래스**: keyPressEvent() 리모컨 키 처리 구현 완료 (F-13)
- [x] **NavigationBar 클래스**: 기본 구조 구현 완료, setHomepage() 메서드만 추가 필요

### 외부 라이브러리 의존성
- [x] **Qt 5.15+**: QWidget, QObject, QPropertyAnimation, QFile, QMessageBox 사용
- [x] **Qt 리소스 시스템 (qrc)**: QSS 파일 임베드
- [x] **webOS LS2 API**: StorageService를 통한 간접 사용

---

## 8. 인수 기준 (Acceptance Criteria)

### AC-1: 설정 패널 접근 및 네비게이션
- [ ] NavigationBar의 "설정" 버튼 클릭 시 SettingsPanel 표시
- [ ] 리모컨 Menu 버튼 (Qt::Key_Menu) 클릭 시 SettingsPanel 토글 (열기/닫기)
- [ ] SettingsPanel 표시 시 슬라이드 인 애니메이션 (0.3초, 부드러운 전환)
- [ ] SettingsPanel 숨김 시 슬라이드 아웃 애니메이션
- [ ] 리모컨 방향키 (Up/Down)로 항목 간 포커스 이동 (Tab Order 준수)
- [ ] 리모컨 Back 키로 SettingsPanel 닫기
- [ ] 포커스 표시 명확 (테두리 강조, 색상 #0078D7)

### AC-2: 검색 엔진 선택
- [ ] 설정 패널에서 검색 엔진 라디오 버튼 4개 표시 (Google, Naver, Bing, DuckDuckGo)
- [ ] 라디오 버튼 선택 시 즉시 SettingsService::setSearchEngine() 호출
- [ ] 검색 엔진 변경 시 LS2 API에 저장 (비동기)
- [ ] 검색 엔진 변경 후 URLBar에서 검색어 입력 시 새 검색 엔진 사용
- [ ] 앱 재시작 후에도 선택한 검색 엔진 유지
- [ ] SearchEngine::getDefaultSearchEngine() 반영 확인

### AC-3: 홈페이지 설정
- [ ] 설정 패널에서 홈페이지 URL 입력 필드 (QLineEdit) 표시
- [ ] Placeholder: "예: https://www.google.com"
- [ ] 잘못된 URL 입력 시 에러 메시지 표시 (homepageErrorLabel_: "유효하지 않은 URL입니다.")
- [ ] 유효한 URL 입력 시 LS2 API에 저장
- [ ] 위험 프로토콜 (javascript:, file://) 입력 시 저장 거부 및 에러 메시지
- [ ] about: URL 허용 (예: about:blank)
- [ ] NavigationBar 홈 버튼 클릭 시 설정된 URL로 이동
- [ ] 앱 재시작 후에도 설정 유지

### AC-4: 테마 설정 (다크/라이트 모드)
- [ ] 설정 패널에서 테마 라디오 버튼 2개 표시 (다크 모드, 라이트 모드)
- [ ] 라디오 버튼 선택 시 즉시 SettingsService::setTheme() 호출
- [ ] 테마 변경 시 LS2 API에 저장
- [ ] 테마 변경 즉시 모든 컴포넌트에 새 스타일시트 적용 (재시작 불필요)
- [ ] 다크 모드: 배경 #1E1E1E, 텍스트 #FFFFFF, 포커스 테두리 #0078D7
- [ ] 라이트 모드: 배경 #FFFFFF, 텍스트 #000000, 포커스 테두리 #0078D7
- [ ] 앱 시작 시 저장된 테마로 초기화
- [ ] 테마 변경 0.5초 이내 완료 (체감 지연 없음)

### AC-5: 브라우징 데이터 삭제
- [ ] "북마크 전체 삭제" 버튼 클릭 시 확인 다이얼로그 표시
- [ ] 확인 다이얼로그 메시지: "모든 북마크를 삭제하시겠습니까? 이 작업은 되돌릴 수 없습니다."
- [ ] 다이얼로그 버튼: "삭제" (Yes), "취소" (No)
- [ ] "삭제" 선택 시 BookmarkService::getAllBookmarks() 호출 → 모든 북마크 삭제
- [ ] 삭제 완료 후 토스트 메시지: "북마크가 삭제되었습니다."
- [ ] BookmarkPanel 목록 자동 새로고침 (비워짐)
- [ ] "취소" 선택 시 삭제 안 됨
- [ ] "히스토리 전체 삭제"도 동일한 플로우

### AC-6: 설정 변경 즉시 저장
- [ ] 검색 엔진 라디오 버튼 선택 시 즉시 저장 (저장 버튼 불필요)
- [ ] 테마 라디오 버튼 선택 시 즉시 저장 및 적용
- [ ] 홈페이지 URL은 Enter 키 또는 포커스 이동 시 저장
- [ ] 저장 실패 시 에러 메시지 표시 (QMessageBox::critical)

### AC-7: 에러 처리
- [ ] 홈페이지 URL 검증 실패 시 homepageErrorLabel_ 표시
- [ ] LS2 API 저장 실패 시 QMessageBox::critical 표시 ("설정 저장에 실패했습니다.")
- [ ] 브라우징 데이터 삭제 실패 시 에러 메시지 표시
- [ ] 설정 로드 실패 시 기본값 사용 (에러 무시, 사용자 영향 없음)

### AC-8: 회귀 테스트
- [ ] 기존 NavigationBar, URLBar, TabBar 동작 정상
- [ ] F-09 검색 엔진 통합 기능 정상 (URLBar 검색 시 설정된 검색 엔진 사용)
- [ ] F-13 리모컨 단축키와 충돌 없음 (Menu 버튼 정상 동작)
- [ ] BookmarkPanel, HistoryPanel 기능 정상 (설정 패널과 독립적)
- [ ] WebView 로딩 정상 (설정 변경 후에도 웹 페이지 정상 표시)

---

## 9. 다음 단계

이 계획서가 승인되면 다음과 같이 진행합니다:

### 순차 실행 방식 (권장)
1. **Phase 1**: cpp-dev가 SettingsService 서비스 계층 구현
2. **Phase 2-4**: cpp-dev가 SettingsPanel UI 계층 구현 (UI 구조 → 동작 → 애니메이션)
3. **Phase 5**: cpp-dev가 QSS 테마 파일 작성
4. **Phase 6**: cpp-dev가 BrowserWindow 통합
5. **Phase 7**: cpp-dev가 NavigationBar 수정
6. **Phase 8**: cpp-dev가 CMakeLists.txt 빌드 설정
7. **Phase 9**: test-runner가 단위 테스트 및 통합 테스트 실행
8. **최종**: code-reviewer가 코드 + 문서 리뷰

### 실행 명령어
```bash
# 자동 순차 파이프라인
/fullstack-feature F-11 설정 화면

# 또는 수동 개발 시작
/agent cpp-dev "F-11 Phase 1 SettingsService 구현 시작"
```

### 완료 후 작업
- doc-writer가 CHANGELOG.md 업데이트
- doc-writer가 docs/dev-log.md 업데이트
- features.md의 F-11 상태를 "완료"로 변경

---

## 10. 참고 문서

### 프로젝트 문서
- PRD: docs/project/prd.md
- 기능 백로그: docs/project/features.md
- 프로젝트 규칙: CLAUDE.md
- 개발 환경 설정: GETTING_STARTED.md

### 관련 기능 설계서
- F-09 검색 엔진 통합: docs/specs/search-engine-integration/
- F-07 북마크 관리: docs/specs/bookmark-management/
- F-08 히스토리 관리: docs/specs/history-management/
- F-13 리모컨 단축키: docs/specs/remote-shortcuts/
- F-03 URL 검증: docs/specs/url-validation/

### Qt 공식 문서
- QWidget: https://doc.qt.io/qt-5/qwidget.html
- QPropertyAnimation: https://doc.qt.io/qt-5/qpropertyanimation.html
- Qt Style Sheets: https://doc.qt.io/qt-5/stylesheet.html
- Qt Resource System: https://doc.qt.io/qt-5/resources.html
- QSettings: https://doc.qt.io/qt-5/qsettings.html

### 참고 구현
- Chrome 설정 화면: chrome://settings/
- Firefox 설정: about:preferences
- webOS Native App 샘플: https://webostv.developer.lge.com/develop/native-apps/samples

---

## 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|-----------|--------|
| 2026-02-14 | 1.0 | F-11 설정 화면 구현 계획서 초안 작성 (Native App) | product-manager |
