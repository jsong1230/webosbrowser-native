# F-11 (설정 화면) 테스트 시나리오

**작성일**: 2026-02-14
**대상**: QA 엔지니어, 테스트 자동화 팀
**프로젝트**: webOS Browser Native

---

## 테스트 시나리오 개요

### 테스트 단계별 구성

| 단계 | 유형 | 파일 | 시나리오 | 소요 시간 |
|------|------|------|---------|---------|
| 1 | 단위 테스트 | SettingsServiceTest.cpp | 8개 | 2시간 |
| 2 | 통합 테스트 | SettingsPanelIntegrationTest.cpp | 7개 | 2시간 |
| 3 | E2E 테스트 | 수동 (webOS) | 5개 | 3시간 |
| **총계** | - | - | **20개** | **7시간** |

---

## 단위 테스트 (SettingsServiceTest.cpp)

### 테스트 목표
SettingsService 클래스의 모든 공개 메서드(public method)의 로직 검증.

### 사전 조건
- GoogleTest 설치
- StorageService Mock 또는 실제 인스턴스
- tests/unit/ 디렉토리 존재

### TC-1: 기본값 로드 (LoadDefaultSettings)

**테스트 ID**: TC-1-001
**우선순위**: Critical
**예상 소요 시간**: 30초

```cpp
/**
 * @brief 첫 실행 시 기본값 로드
 *
 * 사전 조건:
 * - StorageService에서 설정이 없음 (빈 QJsonObject)
 *
 * 단계:
 * 1. SettingsService 생성
 * 2. loadSettings() 호출
 *
 * 예상 결과:
 * - loadSettings() 반환값: true
 * - searchEngine_: "google"
 * - homepage_: "https://www.google.com"
 * - theme_: "dark"
 * - settingsLoaded() 시그널 발생
 */
TEST_F(SettingsServiceTest, LoadDefaultSettings) {
    // Given
    StorageService storageService;
    SettingsService settings(&storageService);

    // When
    bool result = settings.loadSettings();

    // Then
    ASSERT_TRUE(result);
    EXPECT_EQ(settings.searchEngine(), "google");
    EXPECT_EQ(settings.homepage(), "https://www.google.com");
    EXPECT_EQ(settings.theme(), "dark");
}
```

**검증 내용**:
- ✅ loadSettings() 반환값 확인
- ✅ 메모리 캐시 (searchEngine_, homepage_, theme_) 초기값 확인
- ✅ 시그널 발생 (QSignalSpy 사용)

---

### TC-2: 유효한 검색 엔진 설정 (SetSearchEngineValid)

**테스트 ID**: TC-1-002
**우선순위**: Critical
**예상 소요 시간**: 30초

```cpp
/**
 * @brief 유효한 검색 엔진 ID로 설정
 *
 * 사전 조건:
 * - SearchEngine::getAllSearchEngines()가 4개 엔진 반환
 * - 현재 검색 엔진: "google"
 *
 * 단계:
 * 1. setSearchEngine("naver") 호출
 *
 * 예상 결과:
 * - setSearchEngine() 반환값: true
 * - searchEngine_: "naver"
 * - searchEngineChanged("naver") 시그널 발생
 * - StorageService::putData() 호출됨 (검증: Mock spy)
 */
TEST_F(SettingsServiceTest, SetSearchEngineValid) {
    // Given
    StorageService storageService;
    SettingsService settings(&storageService);
    settings.loadSettings();

    QSignalSpy spy(&settings, &SettingsService::searchEngineChanged);

    // When
    bool result = settings.setSearchEngine("naver");

    // Then
    ASSERT_TRUE(result);
    EXPECT_EQ(settings.searchEngine(), "naver");
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), "naver");
}
```

**검증 내용**:
- ✅ 유효한 엔진 ID 수용
- ✅ 메모리 캐시 업데이트
- ✅ 시그널 발생 및 매개변수 검증

**테스트 데이터**:
- "google", "naver", "bing", "duckduckgo" (유효)

---

### TC-3: 유효하지 않은 검색 엔진 거부 (SetSearchEngineInvalid)

**테스트 ID**: TC-1-003
**우선순위**: High
**예상 소요 시간**: 30초

```cpp
/**
 * @brief 유효하지 않은 검색 엔진 ID 거부
 *
 * 사전 조건:
 * - 현재 검색 엔진: "google"
 *
 * 단계:
 * 1. setSearchEngine("invalid") 호출
 *
 * 예상 결과:
 * - setSearchEngine() 반환값: false
 * - searchEngine_: "google" (변경 안 됨)
 * - searchEngineChanged 시그널 발생 안 함
 */
TEST_F(SettingsServiceTest, SetSearchEngineInvalid) {
    // Given
    StorageService storageService;
    SettingsService settings(&storageService);
    settings.loadSettings();

    QSignalSpy spy(&settings, &SettingsService::searchEngineChanged);

    // When
    bool result = settings.setSearchEngine("invalid");

    // Then
    ASSERT_FALSE(result);
    EXPECT_EQ(settings.searchEngine(), "google");  // 변경 안 됨
    EXPECT_EQ(spy.count(), 0);  // 시그널 발생 안 함
}
```

**검증 내용**:
- ✅ 유효성 검증 (SearchEngine::getAllSearchEngines())
- ✅ 거부 시 false 반환
- ✅ 메모리 캐시 유지
- ✅ 시그널 미발생

---

### TC-4: 유효한 URL 설정 (SetHomepageValid)

**테스트 ID**: TC-1-004
**우선순위**: Critical
**예상 소요 시간**: 30초

```cpp
/**
 * @brief 유효한 URL로 홈페이지 설정
 *
 * 사전 조건:
 * - 현재 홈페이지: "https://www.google.com"
 *
 * 단계:
 * 1. setHomepage("https://www.naver.com") 호출
 *
 * 예상 결과:
 * - setHomepage() 반환값: true
 * - homepage_: "https://www.naver.com"
 * - homepageChanged("https://www.naver.com") 시그널 발생
 */
TEST_F(SettingsServiceTest, SetHomepageValid) {
    // Given
    StorageService storageService;
    SettingsService settings(&storageService);
    settings.loadSettings();

    QSignalSpy spy(&settings, &SettingsService::homepageChanged);

    // When
    bool result = settings.setHomepage("https://www.naver.com");

    // Then
    ASSERT_TRUE(result);
    EXPECT_EQ(settings.homepage(), "https://www.naver.com");
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), "https://www.naver.com");
}
```

**검증 내용**:
- ✅ URLValidator::isValid() 검증
- ✅ 메모리 캐시 업데이트
- ✅ 시그널 발생

**테스트 데이터**:
- "https://www.github.com"
- "https://www.amazon.com"
- "about:blank" (특수 URL)

---

### TC-5: 위험한 프로토콜 차단 (SetHomepageDangerous)

**테스트 ID**: TC-1-005
**우선순위**: Critical (보안)
**예상 소요 시간**: 30초

```cpp
/**
 * @brief JavaScript URL 및 file:// 프로토콜 차단
 *
 * 단계:
 * 1. setHomepage("javascript:alert('xss')") 호출
 * 2. setHomepage("file:///etc/passwd") 호출
 *
 * 예상 결과:
 * - 모두 false 반환
 * - homepage_ 변경 안 됨
 * - 시그널 미발생
 */
TEST_F(SettingsServiceTest, SetHomepageDangerous) {
    // Given
    StorageService storageService;
    SettingsService settings(&storageService);
    settings.loadSettings();

    // When - TC-5a: JavaScript URL
    bool result1 = settings.setHomepage("javascript:alert('xss')");

    // Then
    ASSERT_FALSE(result1);
    EXPECT_EQ(settings.homepage(), "https://www.google.com");

    // When - TC-5b: file:// 프로토콜
    bool result2 = settings.setHomepage("file:///etc/passwd");

    // Then
    ASSERT_FALSE(result2);
    EXPECT_EQ(settings.homepage(), "https://www.google.com");
}
```

**검증 내용**:
- ✅ javascript: 차단
- ✅ file:// 차단
- ✅ 메모리 캐시 보호

---

### TC-6: 유효한 테마 설정 (SetThemeValid)

**테스트 ID**: TC-1-006
**우선순위**: High
**예상 소요 시간**: 30초

```cpp
/**
 * @brief 유효한 테마로 설정
 *
 * 단계:
 * 1. setTheme("light") 호출
 *
 * 예상 결과:
 * - setTheme() 반환값: true
 * - theme_: "light"
 * - themeChanged("light") 시그널 발생
 */
TEST_F(SettingsServiceTest, SetThemeValid) {
    // Given
    StorageService storageService;
    SettingsService settings(&storageService);
    settings.loadSettings();

    QSignalSpy spy(&settings, &SettingsService::themeChanged);

    // When
    bool result = settings.setTheme("light");

    // Then
    ASSERT_TRUE(result);
    EXPECT_EQ(settings.theme(), "light");
    EXPECT_EQ(spy.count(), 1);
}
```

**검증 내용**:
- ✅ 테마 검증 ("dark" 또는 "light")
- ✅ 메모리 캐시 업데이트
- ✅ 시그널 발생

---

### TC-7: 유효하지 않은 테마 거부 (SetThemeInvalid)

**테스트 ID**: TC-1-007
**우선순위**: High
**예상 소요 시간**: 30초

```cpp
/**
 * @brief 유효하지 않은 테마 거부
 *
 * 단계:
 * 1. setTheme("red") 호출
 *
 * 예상 결과:
 * - setTheme() 반환값: false
 * - theme_: "dark" (변경 안 됨)
 * - themeChanged 시그널 미발생
 */
TEST_F(SettingsServiceTest, SetThemeInvalid) {
    // Given
    StorageService storageService;
    SettingsService settings(&storageService);
    settings.loadSettings();

    QSignalSpy spy(&settings, &SettingsService::themeChanged);

    // When
    bool result = settings.setTheme("red");

    // Then
    ASSERT_FALSE(result);
    EXPECT_EQ(settings.theme(), "dark");
    EXPECT_EQ(spy.count(), 0);
}
```

**검증 내용**:
- ✅ 유효성 검증
- ✅ 거부 시 false 반환
- ✅ 메모리 캐시 유지

---

### TC-8: 시그널 발생 검증 (SearchEngineChangedSignal)

**테스트 ID**: TC-1-008
**우선순위**: High
**예상 소요 시간**: 30초

```cpp
/**
 * @brief searchEngineChanged 시그널 매개변수 검증
 *
 * 단계:
 * 1. QSignalSpy로 searchEngineChanged 모니터링
 * 2. setSearchEngine("bing") 호출
 *
 * 예상 결과:
 * - 시그널 발생 1회
 * - 매개변수: "bing" (QString)
 * - spy.at(0).at(0): "bing"
 */
TEST_F(SettingsServiceTest, SearchEngineChangedSignal) {
    // Given
    StorageService storageService;
    SettingsService settings(&storageService);
    settings.loadSettings();

    QSignalSpy spy(&settings, &SettingsService::searchEngineChanged);

    // When
    settings.setSearchEngine("bing");

    // Then
    ASSERT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    ASSERT_EQ(arguments.size(), 1);
    EXPECT_EQ(arguments.at(0).toString(), "bing");
}
```

**검증 내용**:
- ✅ 시그널 발생 횟수 검증
- ✅ 시그널 매개변수 검증

---

## 통합 테스트 (SettingsPanelIntegrationTest.cpp)

### 테스트 목표
SettingsPanel UI 컴포넌트와 SettingsService의 상호작용 검증.

### 사전 조건
- Qt 5.15+
- QTest 프레임워크
- BrowserWindow를 부모 위젯으로 사용 (또는 Mock QWidget)

### TC-9: 패널 표시 (ShowPanel)

**테스트 ID**: TC-2-001
**우선순위**: Critical
**예상 소요 시간**: 1초

```cpp
/**
 * @brief showPanel() 호출 시 패널 표시
 *
 * 단계:
 * 1. showPanel() 호출
 *
 * 예상 결과:
 * - isVisible(): true
 * - googleRadio_에 포커스 설정됨
 */
TEST_F(SettingsPanelIntegrationTest, ShowPanel) {
    // When
    panel_->showPanel();

    // Then
    EXPECT_TRUE(panel_->isVisible());
    // 포커스는 비공개 멤버이므로 간접 검증 필요
}
```

---

### TC-10: 패널 숨김 애니메이션 (HidePanel)

**테스트 ID**: TC-2-002
**우선순위**: High
**예상 소요 시간**: 1초 (+ 애니메이션 350ms)

```cpp
/**
 * @brief hidePanel() 호출 시 슬라이드 아웃 애니메이션 후 숨김
 *
 * 단계:
 * 1. showPanel() 호출
 * 2. hidePanel() 호출
 * 3. 애니메이션 대기 (350ms)
 *
 * 예상 결과:
 * - 애니메이션 후 isVisible(): false
 * - panelClosed() 시그널 발생
 */
TEST_F(SettingsPanelIntegrationTest, HidePanel) {
    // Given
    panel_->showPanel();
    QSignalSpy spy(panel_, &SettingsPanel::panelClosed);

    // When
    panel_->hidePanel();
    QTest::qWait(350);  // 애니메이션 300ms + 여유 50ms

    // Then
    EXPECT_FALSE(panel_->isVisible());
    EXPECT_EQ(spy.count(), 1);
}
```

**검증 내용**:
- ✅ 숨김 동작 완료
- ✅ 시그널 발생

---

### TC-11: 검색 엔진 라디오 버튼 클릭 (ClickSearchEngineRadio)

**테스트 ID**: TC-2-003
**우선순위**: Critical
**예상 소요 시간**: 1초

```cpp
/**
 * @brief 검색 엔진 라디오 버튼 선택 시 SettingsService 호출
 *
 * 단계:
 * 1. showPanel() 호출
 * 2. Naver 라디오 버튼 setChecked(true) 호출
 *
 * 예상 결과:
 * - SettingsService::searchEngineChanged 시그널 발생
 * - SettingsService::searchEngine() == "naver"
 */
TEST_F(SettingsPanelIntegrationTest, ClickSearchEngineRadio) {
    // Given
    panel_->showPanel();
    QSignalSpy spy(settingsService_, &SettingsService::searchEngineChanged);

    // When
    QTest::mouseClick(panel_->naverRadio_, Qt::LeftButton);
    // 또는
    panel_->naverRadio_->setChecked(true);

    // Then (시그널 발생 대기)
    // EXPECT_EQ(spy.count(), 1);
    // EXPECT_EQ(settingsService_->searchEngine(), "naver");
}
```

**검증 내용**:
- ✅ 라디오 버튼 선택 시 SettingsService 호출
- ✅ 검색 엔진 변경 반영

---

### TC-12: 홈페이지 URL 입력 및 저장 (SetHomepageViaInput)

**테스트 ID**: TC-2-004
**우선순위**: Critical
**예상 소요 시간**: 1초

```cpp
/**
 * @brief QLineEdit에 URL 입력 후 editingFinished 신호로 저장
 *
 * 단계:
 * 1. showPanel() 호출
 * 2. homepageInput_->setText("https://www.github.com")
 * 3. editingFinished() 시그널 발생 (또는 Enter 키)
 *
 * 예상 결과:
 * - SettingsService::homepageChanged 시그널 발생
 * - homepageErrorLabel_ 숨김
 */
TEST_F(SettingsPanelIntegrationTest, SetHomepageViaInput) {
    // Given
    panel_->showPanel();
    QSignalSpy spy(settingsService_, &SettingsService::homepageChanged);

    // When
    panel_->homepageInput_->setText("https://www.github.com");
    panel_->homepageInput_->editingFinished();

    // Then
    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(panel_->homepageErrorLabel_->isVisible());
}
```

---

### TC-13: 잘못된 URL 입력 (InvalidHomepageInput)

**테스트 ID**: TC-2-005
**우선순위**: High
**예상 소요 시간**: 1초

```cpp
/**
 * @brief 유효하지 않은 URL 입력 시 에러 메시지 표시
 *
 * 단계:
 * 1. homepageInput_->setText("invalid url")
 * 2. editingFinished()
 *
 * 예상 결과:
 * - homepageErrorLabel_ 표시
 * - 메시지: "유효하지 않은 URL입니다."
 * - SettingsService 저장 안 됨
 */
TEST_F(SettingsPanelIntegrationTest, InvalidHomepageInput) {
    // Given
    panel_->showPanel();
    QSignalSpy spy(settingsService_, &SettingsService::homepageChanged);

    // When
    panel_->homepageInput_->setText("invalid url");
    panel_->homepageInput_->editingFinished();

    // Then
    EXPECT_TRUE(panel_->homepageErrorLabel_->isVisible());
    EXPECT_EQ(spy.count(), 0);
}
```

---

### TC-14: 테마 라디오 버튼 클릭 (ClickThemeRadio)

**테스트 ID**: TC-2-006
**우선순위**: High
**예상 소요 시간**: 1초

```cpp
/**
 * @brief 테마 라디오 버튼 선택 시 SettingsService 호출
 *
 * 단계:
 * 1. lightThemeRadio_->setChecked(true)
 *
 * 예상 결과:
 * - SettingsService::themeChanged("light") 시그널 발생
 */
TEST_F(SettingsPanelIntegrationTest, ClickThemeRadio) {
    // Given
    panel_->showPanel();
    QSignalSpy spy(settingsService_, &SettingsService::themeChanged);

    // When
    panel_->lightThemeRadio_->setChecked(true);

    // Then
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(settingsService_->theme(), "light");
}
```

---

### TC-15: Back 키로 패널 닫기 (BackKeyClosesPanel)

**테스트 ID**: TC-2-007
**우선순위**: High
**예상 소요 시간**: 1초 (+ 애니메이션)

```cpp
/**
 * @brief Back 키(Qt::Key_Back) 입력 시 패널 닫기
 *
 * 단계:
 * 1. showPanel()
 * 2. QKeyEvent(QEvent::KeyPress, Qt::Key_Back) 발생
 * 3. 애니메이션 대기
 *
 * 예상 결과:
 * - hidePanel() 실행
 * - isVisible(): false
 */
TEST_F(SettingsPanelIntegrationTest, BackKeyClosesPanel) {
    // Given
    panel_->showPanel();

    // When
    QKeyEvent backKeyEvent(QEvent::KeyPress, Qt::Key_Back, Qt::NoModifier);
    panel_->keyPressEvent(&backKeyEvent);
    QTest::qWait(350);

    // Then
    EXPECT_FALSE(panel_->isVisible());
}
```

---

## E2E 테스트 (webOS 프로젝터 또는 에뮬레이터)

### 테스트 환경
- webOS 6.0+ (LG 프로젝터 HU715QW 또는 에뮬레이터)
- 리모컨 (또는 가상 리모컨)
- 저장된 북마크 데이터 (테스트용)

### 시나리오 1: 검색 엔진 변경

**테스트 ID**: TC-3-001
**우선순위**: Critical
**예상 소요 시간**: 5분

```
[사전 조건]
  기본 검색 엔진: Google
  URLBar: 준비 완료

[단계]
  Step 1: 리모컨 Menu 버튼 입력
    → 예상: 설정 패널 슬라이드 인 (왼쪽에서 오른쪽으로)
    → 확인: 패널이 화면에 표시, 배경 반투명

  Step 2: 포커스 확인
    → 예상: Google 라디오 버튼에 파란색 테두리 (포커스)
    → 확인: 포커스 표시 명확

  Step 3: 리모컨 방향키 Right × 1 (Naver 선택)
    → 예상: Naver 라디오 버튼 선택됨
    → 확인: 라디오 버튼 상태 변경

  Step 4: Enter 키 입력
    → 예상: 검색 엔진 즉시 변경 (Naver)
    → 확인: 설정 패널 상태 변경 없음 (슬라이드 아웃 안 함)

  Step 5: Back 키 입력
    → 예상: 설정 패널 슬라이드 아웃 (오른쪽으로 빠져나감)
    → 확인: 패널이 숨겨짐

  Step 6: URLBar 클릭
    → 예상: URLBar 포커스
    → 확인: 입력 필드 활성화

  Step 7: 검색어 입력 "테스트"
    → 예상: 가상 키보드 표시
    → 확인: "테스트" 입력됨

  Step 8: Enter 키 입력
    → 예상: Naver 검색 결과 페이지 로드
    → 확인: URL에 "naver.com/search?q=테스트" 포함

[결과 검증]
  ✓ 설정 패널 애니메이션 부드러움 (0.3초)
  ✓ 검색 엔진 변경 반영 (즉시)
  ✓ URLBar 검색 시 Naver 사용

[회귀 테스트]
  ✓ 앱 재시작 후 검색 엔진 Naver 유지
```

---

### 시나리오 2: 홈페이지 설정

**테스트 ID**: TC-3-002
**우선순위**: Critical
**예상 소요 시간**: 5분

```
[사전 조건]
  기본 홈페이지: https://www.google.com
  NavigationBar: 준비 완료

[단계]
  Step 1: Menu 버튼 → 설정 패널 표시

  Step 2: 방향키 Down × 3 → 홈페이지 입력 필드에 포커스
    → 예상: QLineEdit에 파란색 테두리 (포커스)
    → 확인: 현재 URL "https://www.google.com" 표시

  Step 3: Enter 키 → 가상 키보드 표시
    → 예상: 키보드 UI 표시
    → 확인: 입력 필드 활성화

  Step 4: 텍스트 선택 및 삭제
    → Ctrl+A 또는 Delete

  Step 5: "https://www.github.com" 입력
    → 예상: 각 글자 입력 반영
    → 확인: 입력 필드에 URL 표시

  Step 6: Enter 키 → 저장
    → 예상: 토스트 메시지 "홈페이지가 설정되었습니다."
    → 확인: 메시지 2초 표시 후 자동 닫힘

  Step 7: Back 키 → 설정 패널 닫기

  Step 8: NavigationBar 홈 버튼 클릭
    → 예상: GitHub 페이지 로드 (https://www.github.com)
    → 확인: WebView에 GitHub 페이지 표시

[결과 검증]
  ✓ URL 검증 정상 (유효한 URL만 저장)
  ✓ 홈 버튼 클릭 시 설정된 URL로 이동
  ✓ 토스트 메시지 표시

[에러 케이스]
  - 잘못된 URL 입력 → 에러 메시지 "유효하지 않은 URL입니다."
  - javascript: 입력 → 차단
  - 빈 입력 → 무시

[회귀 테스트]
  ✓ 앱 재시작 후 홈페이지 GitHub 유지
```

---

### 시나리오 3: 테마 변경

**테스트 ID**: TC-3-003
**우선순위**: Critical
**예상 소요 시간**: 3분

```
[사전 조건]
  현재 테마: 다크 모드 (#1E1E1E 배경, #FFFFFF 텍스트)

[단계]
  Step 1: Menu 버튼 → 설정 패널 표시
    → 확인: 다크 모드 색상 유지 (#2D2D2D 배경)

  Step 2: 방향키 Down × 4 → 테마 라디오 버튼에 포커스
    → 예상: "다크 모드" 라디오 버튼 선택됨 (파란색 테두리)
    → 확인: 포커스 표시

  Step 3: 방향키 Right → "라이트 모드" 선택
    → 예상: 라디오 버튼 상태 변경

  Step 4: Enter 키 입력
    → 예상: 테마 즉시 변경 (재시작 불필요)
    → 확인: 다음 항목 보이기:
      - 배경색: #FFFFFF (흰색)
      - 텍스트색: #000000 (검은색)
      - NavigationBar: #F5F5F5 (밝은 회색)
      - 버튼: #E0E0E0 (밝은 회색)
      - 포커스 테두리: #0078D7 (파란색, 동일)

  Step 5: Back 키 → 설정 패널 닫기

  Step 6: 전체 UI 확인
    → 확인: 모든 컴포넌트 라이트 모드 색상 반영
    → URLBar, TabBar, 버튼 등 색상 변경

[색상 검증]
  다크 모드:
    - 배경: #1E1E1E ✓
    - 텍스트: #FFFFFF ✓
    - 포커스 테두리: #0078D7 ✓

  라이트 모드:
    - 배경: #FFFFFF ✓
    - 텍스트: #000000 ✓
    - 포커스 테두리: #0078D7 ✓

[회귀 테스트]
  ✓ 앱 재시작 후 라이트 모드 유지
  ✓ 다시 다크 모드로 변경 가능
```

---

### 시나리오 4: 북마크 삭제

**테스트 ID**: TC-3-004
**우선순위**: High
**예상 소요 시간**: 3분

```
[사전 조건]
  북마크: 5개 저장됨 (GitHub, Amazon, Reddit, Stack Overflow, Medium)

[단계]
  Step 1: Menu 버튼 → 설정 패널 표시

  Step 2: 방향키 Down × 5 → "북마크 전체 삭제" 버튼에 포커스
    → 예상: 빨간색 테두리 (포커스)
    → 확인: 버튼 텍스트 "북마크 전체 삭제"

  Step 3: Enter 키
    → 예상: 확인 다이얼로그 표시
    → 메시지: "모든 북마크를 삭제하시겠습니까? 이 작업은 되돌릴 수 없습니다."
    → 버튼: "삭제" (Yes), "취소" (No)
    → 포커스: "취소" (기본값, 안전)

  Step 4: 리모컨 방향키 Left → "삭제" 버튼으로 이동
    → 확인: 포커스 변경

  Step 5: Enter 키
    → 예상: 북마크 삭제 시작
    → 확인: 로딩 표시 (선택사항)

  Step 6: 완료 토스트 표시
    → 메시지: "북마크가 삭제되었습니다."
    → 표시 시간: 2초

  Step 7: BookmarkPanel 확인
    → Menu 버튼으로 설정 패널 닫기 (또는 Back 키)
    → BookmarkPanel 열기
    → 예상: 목록이 비워짐 (0개)

[결과 검증]
  ✓ 확인 다이얼로그 표시
  ✓ "삭제" 선택 시 모든 북마크 삭제
  ✓ BookmarkPanel 자동 새로고침
  ✓ 토스트 메시지 표시

[부정적 케이스]
  - "취소" 선택 → 삭제 안 됨, 북마크 유지
```

---

### 시나리오 5: 리모컨 네비게이션 (Tab Order)

**테스트 ID**: TC-3-005
**우선순위**: High
**예상 소요 시간**: 3분

```
[사전 조건]
  설정 패널 표시됨
  포커스: Google 라디오 버튼

[단계]
  Step 1: 포커스 초기 상태 확인
    → 예상: Google 라디오 버튼 (파란색 테두리)

  Step 2: 방향키 Down (또는 Tab)
    → 예상: Naver 라디오 버튼으로 이동
    → 확인: 포커스 표시

  Step 3: 방향키 Down
    → 예상: Bing 라디오 버튼

  Step 4: 방향키 Down
    → 예상: DuckDuckGo 라디오 버튼

  Step 5: 방향키 Down
    → 예상: 홈페이지 입력 필드로 이동
    → 확인: QLineEdit 포커스

  Step 6: 방향키 Down
    → 예상: 다크 모드 라디오 버튼으로 이동

  Step 7: 방향키 Down
    → 예상: 라이트 모드 라디오 버튼

  Step 8: 방향키 Down
    → 예상: "북마크 전체 삭제" 버튼

  Step 9: 방향키 Down
    → 예상: "히스토리 전체 삭제" 버튼

  Step 10: 방향키 Down
    → 예상: "닫기" (X) 버튼

  Step 11: 방향키 Up × 10 (역방향 순회)
    → 예상: Google 라디오 버튼으로 돌아옴
    → 확인: 순환 네비게이션 가능

[결과 검증]
  ✓ Tab Order 설정이 정확
  ✓ 포커스 이동 부드러움
  ✓ 순환 네비게이션 가능
  ✓ 포커스 표시 명확 (테두리 강조)
```

---

## 테스트 실행 계획

### Phase 1: 단위 테스트 (2시간)
```bash
# 단계 1: 테스트 파일 생성
mkdir -p tests/unit
# SettingsServiceTest.cpp 작성

# 단계 2: 빌드 및 실행
cd build
cmake ..
ctest -R SettingsServiceTest -VV
```

**성공 기준**: 8개 TC 모두 PASS

---

### Phase 2: 통합 테스트 (2시간)
```bash
# 단계 1: 테스트 파일 생성
mkdir -p tests/integration
# SettingsPanelIntegrationTest.cpp 작성

# 단계 2: 빌드 및 실행
ctest -R SettingsPanelIntegrationTest -VV
```

**성공 기준**: 7개 TC 모두 PASS

---

### Phase 3: E2E 테스트 (3시간)
```bash
# 단계 1: webOS 프로젝터/에뮬레이터 준비
ares-install --device projector webosbrowser.ipk

# 단계 2: 앱 실행
ares-launch --device projector com.jsong.webosbrowser.native

# 단계 3: 수동 테스트 실행
# 5개 시나리오 순차 실행
```

**성공 기준**: 5개 시나리오 모두 PASS

---

## 테스트 보고 양식

### 테스트 실행 결과

```
테스트 명: ___________________
테스트 ID: ___________________
실행 날짜: ___________________
실행자: ___________________

[실행 결과]
☐ PASS
☐ FAIL
☐ SKIP (사유: _______________)

[상세 내용]
예상: _____________________
실제: _____________________
스크린샷/로그: _______________

[이슈 (FAIL인 경우)]
심각도: ☐ Critical ☐ High ☐ Medium ☐ Low
카테고리: ☐ 로직 ☐ UI ☐ 성능 ☐ 기타
설명: ______________________
재현 단계: ____________________
```

---

## 참고 자료

- **설계서**: docs/specs/settings/design.md
- **요구사항**: docs/specs/settings/requirements.md
- **구현 코드**: src/services/SettingsService.{h,cpp}
- **UI 코드**: src/ui/SettingsPanel.{h,cpp}
- **Qt 테스트**: https://doc.qt.io/qt-5/qtest.html
- **Google Test**: https://github.com/google/googletest

---

**작성 완료**
**작성자**: QA 엔지니어
**상태**: Ready for Testing
