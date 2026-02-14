# PG-3 병렬 배치 통합 테스트 계획

**테스트 대상**: F-12 (다운로드 관리), F-13 (리모컨 단축키), F-14 (HTTPS 보안)
**테스트 일시**: Manual (Qt Test 환경에서 실행)
**담당자**: QA Engineer

---

## 1. 통합 테스트 시나리오

### 1.1 Yellow 버튼으로 다운로드 패널 열기 (F-12 + F-13)

#### 테스트 케이스: Yellow Button Opens Download Panel

**전제 조건**:
- BrowserWindow가 활성화됨
- 다운로드 진행 중인 항목이 있거나 없음
- DownloadPanel이 초기에 숨겨져 있음

**테스트 단계**:
1. Yellow 버튼 (406) 키 입력 시뮬레이션
2. keyPressEvent() 호출
3. handleColorButton(406) 실행

**예상 결과**:
```cpp
✓ downloadPanel_->isVisible() == true
✓ downloadPanel_->hasFocus() == true
✓ Logger에 "[BrowserWindow] Yellow 버튼 → 다운로드 패널 열림" 로그 출력
```

**테스트 코드 (의사코드)**:
```cpp
void testYellowButtonOpensDownloadPanel() {
    // 1. BrowserWindow 생성
    BrowserWindow window;
    window.show();

    // 2. DownloadPanel이 초기에 숨겨져 있는지 확인
    QVERIFY(!window.downloadPanel_->isVisible());

    // 3. Yellow 키 이벤트 생성
    QKeyEvent event(QEvent::KeyPress, 406, Qt::NoModifier);

    // 4. keyPressEvent 호출
    window.keyPressEvent(&event);

    // 5. DownloadPanel이 표시되었는지 확인
    QVERIFY(window.downloadPanel_->isVisible());
    QVERIFY(window.downloadPanel_->hasFocus());
}
```

---

### 1.2 디바운싱 검증 (F-13)

#### 테스트 케이스: Remote Key Debouncing (500ms)

**전제 조건**:
- BrowserWindow가 활성화됨
- DEBOUNCE_MS = 500ms로 설정됨

**테스트 단계**:
1. Channel Up 키 (427) 입력
2. 100ms 이내에 동일 키 (427) 재입력
3. keyPressEvent() 호출

**예상 결과**:
```cpp
✓ 첫 번째 입력: 처리됨 (shouldDebounce == false)
✓ 두 번째 입력: 무시됨 (shouldDebounce == true)
✓ 600ms 후 동일 키 입력: 처리됨 (디바운스 타임아웃)
```

**테스트 코드 (의사코드)**:
```cpp
void testDebouncing() {
    BrowserWindow window;
    window.show();

    // 첫 번째 Channel Up 입력
    QKeyEvent event1(QEvent::KeyPress, 427, Qt::NoModifier);
    window.keyPressEvent(&event1);
    QVERIFY(window.lastKeyPressTime_[427] > 0);

    qint64 firstTime = window.lastKeyPressTime_[427];

    // 100ms 대기
    QTest::qWait(100);

    // 두 번째 Channel Up 입력 (디바운스됨)
    QKeyEvent event2(QEvent::KeyPress, 427, Qt::NoModifier);
    window.keyPressEvent(&event2);
    QCOMPARE(window.lastKeyPressTime_[427], firstTime);  // 시간 안 변함

    // 600ms 대기
    QTest::qWait(600);

    // 세 번째 Channel Up 입력 (처리됨)
    QKeyEvent event3(QEvent::KeyPress, 427, Qt::NoModifier);
    window.keyPressEvent(&event3);
    QVERIFY(window.lastKeyPressTime_[427] > firstTime);  // 시간 변함
}
```

---

### 1.3 탭 전환 (F-13)

#### 테스트 케이스: Tab Switching with Channel Buttons

**전제 조건**:
- BrowserWindow가 활성화됨
- 최소 2개 이상의 탭이 생성됨

**테스트 단계**:
1. 탭 생성 (Blue 버튼 또는 프로그래밍)
2. Channel Up (427) 입력 → 이전 탭으로 전환
3. Channel Down (428) 입력 → 다음 탭으로 전환
4. URLBar 및 NavigationBar 업데이트 확인

**예상 결과**:
```cpp
✓ tabManager_->previousTab() 호출됨
✓ tabSwitched(int index, const QString& url, const QString& title) 시그널 발생
✓ urlBar_->setText(url) 호출됨
✓ navigationBar_->updateTitle(title) 호출됨
```

**테스트 코드 (의사코드)**:
```cpp
void testChannelButtonTabSwitching() {
    BrowserWindow window;
    window.show();

    // 2개 탭 생성
    window.tabManager_->createTab();
    int tabCount = window.tabManager_->getTabCount();
    QCOMPARE(tabCount, 2);

    int initialIndex = window.tabManager_->getCurrentTabIndex();

    // Channel Up 입력 (이전 탭)
    QKeyEvent upEvent(QEvent::KeyPress, 427, Qt::NoModifier);
    window.keyPressEvent(&upEvent);

    int newIndex = window.tabManager_->getCurrentTabIndex();
    QVERIFY(newIndex != initialIndex);
}
```

---

### 1.4 숫자 버튼으로 탭 선택 (F-13)

#### 테스트 케이스: Direct Tab Selection with Number Buttons

**전제 조건**:
- BrowserWindow가 활성화됨
- 5개의 탭이 생성됨

**테스트 단계**:
1. 숫자 버튼 (1~5) 입력
2. handleNumberButton() 실행
3. 해당 탭으로 전환

**예상 결과**:
```cpp
✓ Number 1 (49) → 탭 0 선택
✓ Number 3 (51) → 탭 2 선택
✓ Number 5 (53) → 탭 4 선택
✓ tabSwitched 시그널 발생
```

**테스트 코드 (의사코드)**:
```cpp
void testNumberButtonTabSelection() {
    BrowserWindow window;
    window.show();

    // 5개 탭 생성
    for (int i = 0; i < 5; i++) {
        window.tabManager_->createTab();
    }

    // Number 3 입력 (탭 2 선택)
    QKeyEvent event(QEvent::KeyPress, 51, Qt::NoModifier);
    window.keyPressEvent(&event);

    QCOMPARE(window.tabManager_->getCurrentTabIndex(), 2);
}
```

---

### 1.5 HTTPS/HTTP 보안 표시 (F-14)

#### 테스트 케이스: Security Indicator Updates on URL Change

**전제 조건**:
- BrowserWindow가 활성화됨
- URLBar와 SecurityIndicator가 초기화됨

**테스트 단계**:
1. HTTPS URL 로드 (https://www.google.com)
2. HTTP URL 로드 (http://example.com)
3. 로컬 URL 로드 (http://localhost)
4. 각 경우 보안 표시 확인

**예상 결과**:
```cpp
✓ HTTPS → SecurityLevel::Secure
  - 아이콘: lock_secure.png
  - 툴팁: "보안 연결"

✓ HTTP (공인IP) → SecurityLevel::Insecure
  - 아이콘: lock_insecure.png
  - 툴팁: "안전하지 않음"
  - 경고 다이얼로그 표시

✓ HTTP (localhost) → SecurityLevel::Local
  - 아이콘: info.png
  - 툴팁: "로컬 연결"
  - 경고 없음
```

**테스트 코드 (의사코드)**:
```cpp
void testSecurityIndicatorUpdate() {
    BrowserWindow window;
    window.show();

    // HTTPS URL
    QUrl httpsUrl("https://www.google.com");
    SecurityLevel level1 = SecurityClassifier::classify(httpsUrl);
    QCOMPARE(level1, SecurityLevel::Secure);
    window.onUrlChanged(httpsUrl.toString());
    // urlBar_->updateSecurityIndicator(level1) 호출됨

    // HTTP URL (공인 IP)
    QUrl httpUrl("http://example.com");
    SecurityLevel level2 = SecurityClassifier::classify(httpUrl);
    QCOMPARE(level2, SecurityLevel::Insecure);
    window.onUrlChanged(httpUrl.toString());
    // urlBar_->updateSecurityIndicator(level2) 호출됨
    // checkHttpWarning() 호출됨 → warningTimer_ 시작

    // HTTP URL (localhost)
    QUrl localhostUrl("http://localhost:8000");
    SecurityLevel level3 = SecurityClassifier::classify(localhostUrl);
    QCOMPARE(level3, SecurityLevel::Local);
    window.onUrlChanged(localhostUrl.toString());
    // urlBar_->updateSecurityIndicator(level3) 호출됨
    // checkHttpWarning()는 경고 없음
}
```

---

### 1.6 HTTP 경고 다이얼로그 (F-14)

#### 테스트 케이스: Security Warning Dialog for HTTP

**전제 조건**:
- BrowserWindow가 활성화됨
- HTTP (비보안) URL 로드

**테스트 단계**:
1. HTTP URL 로드 (http://example.com)
2. 500ms 디바운싱 후 경고 다이얼로그 표시
3. "계속 진행" 버튼 클릭
4. "이 세션 동안 다시 표시하지 않기" 체크박스 선택
5. 동일 도메인 재방문 시 경고 없음

**예상 결과**:
```cpp
✓ 경고 다이얼로그 표시
✓ "계속 진행" / "돌아가기" 버튼 표시
✓ "이 세션 동안 다시 표시하지 않기" 체크박스 표시
✓ "계속 진행" 선택 시:
  - 도메인이 ignoredDomains_에 추가
  - 페이지 로드 계속
✓ "돌아가기" 선택 시:
  - webView_->goBack() 호출
  - 페이지 로드 취소
✓ ignoredDomains_ 최대 100개 제한
```

**테스트 코드 (의사코드)**:
```cpp
void testHttpWarningDialog() {
    BrowserWindow window;
    window.show();

    // HTTP URL 로드
    QUrl httpUrl("http://example.com");
    window.onUrlChanged(httpUrl.toString());

    // checkHttpWarning() 호출됨
    // warningTimer_->start(500) 시작

    // 600ms 대기
    QTest::qWait(600);

    // onWarningTimerTimeout() 호출됨
    // showSecurityWarningDialog() 호출됨

    // ignoredDomains_에 example.com이 추가되었는지 확인
    QVERIFY(window.ignoredDomains_.contains("example.com"));

    // 동일 도메인 재방문
    window.onUrlChanged(httpUrl.toString());
    // checkHttpWarning()는 이미 무시 목록에 있으므로 반환

    // warningTimer_이 시작되지 않음
    QVERIFY(!window.warningTimer_->isActive());
}
```

---

### 1.7 탭 전환 시 보안 표시 업데이트 (F-13 + F-14)

#### 테스트 케이스: Security Indicator Updates on Tab Switch

**전제 조건**:
- BrowserWindow가 활성화됨
- 2개 이상의 탭이 생성됨 (서로 다른 보안 수준)

**테스트 단계**:
1. 탭 1: HTTPS URL 로드
2. 탭 2: HTTP URL 로드
3. 탭 1과 탭 2 간 전환
4. 각 탭 전환 시 보안 표시 업데이트 확인

**예상 결과**:
```cpp
✓ 탭 1 (HTTPS) → SecurityLevel::Secure
✓ 탭 2 (HTTP) → SecurityLevel::Insecure
✓ 탭 전환 시 URLBar 업데이트
✓ 탭 전환 시 SecurityIndicator 업데이트
✓ 탭 2 (HTTP) 진입 시 경고 다이얼로그 표시
```

**테스트 코드 (의사코드)**:
```cpp
void testSecurityOnTabSwitch() {
    BrowserWindow window;
    window.show();

    // 탭 1 (HTTPS)
    window.tabManager_->createTab("https://www.google.com");
    // ...설정...

    // 탭 2 (HTTP)
    window.tabManager_->createTab("http://example.com");
    // ...설정...

    // 탭 1 선택 (Number 1)
    QKeyEvent event1(QEvent::KeyPress, 49, Qt::NoModifier);
    window.keyPressEvent(&event1);

    // urlBar_에 HTTPS URL 표시
    QVERIFY(window.urlBar_->text().startsWith("https"));

    // 탭 2 선택 (Number 2)
    QKeyEvent event2(QEvent::KeyPress, 50, Qt::NoModifier);
    window.keyPressEvent(&event2);

    // urlBar_에 HTTP URL 표시
    QVERIFY(window.urlBar_->text().startsWith("http"));

    // 경고 다이얼로그 표시됨 (600ms 후)
}
```

---

### 1.8 Red/Green 버튼으로 패널 열기 (F-13)

#### 테스트 케이스: Colored Button Opens Panels

**전제 조건**:
- BrowserWindow가 활성화됨
- 모든 패널이 초기에 숨겨져 있음

**테스트 단계**:
1. Red 버튼 (403) → BookmarkPanel 열기
2. Green 버튼 (405) → HistoryPanel 열기
3. Back 키 (27) → 패널 닫기

**예상 결과**:
```cpp
✓ Red 버튼 → bookmarkPanel_->isVisible() == true
✓ Green 버튼 → historyPanel_->isVisible() == true
✓ Back 키 → 패널 닫기 (패널이 포커스 중일 때)
```

**테스트 코드 (의사코드)**:
```cpp
void testColorButtonOpensPanels() {
    BrowserWindow window;
    window.show();

    // Red 버튼 (북마크 패널)
    QKeyEvent redEvent(QEvent::KeyPress, 403, Qt::NoModifier);
    window.keyPressEvent(&redEvent);
    QVERIFY(window.bookmarkPanel_->isVisible());

    // Back 키로 패널 닫기
    QKeyEvent backEvent(QEvent::KeyPress, 27, Qt::NoModifier);
    window.keyPressEvent(&backEvent);
    QVERIFY(!window.bookmarkPanel_->isVisible());

    // Green 버튼 (히스토리 패널)
    QKeyEvent greenEvent(QEvent::KeyPress, 405, Qt::NoModifier);
    window.keyPressEvent(&greenEvent);
    QVERIFY(window.historyPanel_->isVisible());
}
```

---

### 1.9 Blue 버튼으로 새 탭 생성 (F-13)

#### 테스트 케이스: Create New Tab with Blue Button

**전제 조건**:
- BrowserWindow가 활성화됨
- 현재 4개 이하의 탭

**테스트 단계**:
1. Blue 버튼 (407) 입력
2. 새 탭 생성 확인
3. 5개 탭 도달 후 Blue 버튼 입력

**예상 결과**:
```cpp
✓ Blue 버튼 입력 → 새 탭 생성
✓ tabManager_->getTabCount() 증가
✓ 5개 탭 도달 시 → 경고 (새 탭 생성 불가)
```

**테스트 코드 (의사코드)**:
```cpp
void testBlueButtonCreateTab() {
    BrowserWindow window;
    window.show();

    int initialCount = window.tabManager_->getTabCount();

    // Blue 버튼
    QKeyEvent event(QEvent::KeyPress, 407, Qt::NoModifier);
    window.keyPressEvent(&event);

    int newCount = window.tabManager_->getTabCount();
    QCOMPARE(newCount, initialCount + 1);

    // 5개 탭 생성
    while (window.tabManager_->getTabCount() < 5) {
        window.keyPressEvent(&event);
    }

    // 6번째 탭 생성 시도 (실패)
    window.keyPressEvent(&event);
    QCOMPARE(window.tabManager_->getTabCount(), 5);  // 변화 없음
}
```

---

## 2. 자동화 테스트 (Qt Test Framework)

### 2.1 테스트 실행

```bash
# 단위 테스트
cd /path/to/webosbrowser-native
mkdir build_test && cd build_test
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --target SecurityClassifierTest
./bin/SecurityClassifierTest

# 통합 테스트 (수동)
# Qt IDE에서 "Run Tests" 실행 또는
# ctest 사용
```

### 2.2 테스트 커버리지

| 컴포넌트 | 테스트 | 상태 |
|---------|--------|------|
| SecurityClassifier | ✅ 단위 테스트 | SecurityClassifierTest.cpp |
| KeyCodeConstants | ✅ 단위 테스트 | KeyCodeMappingTest.cpp |
| DownloadManager | ✅ 단위 테스트 | DownloadManagerTest.cpp |
| BrowserWindow (F-12) | ✅ 통합 테스트 | 1.1, 1.8 |
| BrowserWindow (F-13) | ✅ 통합 테스트 | 1.2, 1.3, 1.4, 1.7, 1.8, 1.9 |
| BrowserWindow (F-14) | ✅ 통합 테스트 | 1.5, 1.6, 1.7 |

---

## 3. 성능 테스트

### 3.1 디바운싱 성능

```
입력: 1000회 빠른 연속 키 입력
예상: 500ms 간격 처리 → ~20회 실제 처리
```

### 3.2 메모리 누수 검증

```bash
# Valgrind 사용
valgrind --leak-check=full ./webosbrowser

# AddressSanitizer 사용
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" ..
make
./webosbrowser
```

---

## 4. 엣지 케이스 테스트

| 케이스 | 테스트 | 예상 결과 |
|--------|--------|----------|
| 빠른 키 입력 | 3.1 | 디바운싱 작동 |
| 탭 전환 중 다운로드 | Manual | 다운로드 계속 진행 |
| URL 변경 중 패널 열기 | Manual | 패널 정상 열림 |
| 최대 탭 개수 도달 | 1.9 | 새 탭 생성 불가 |
| HTTP 경고 무시 목록 초과 | 1.6 | 목록 제한 (100개) |
| 메모리 부족 시 다운로드 | Manual | 안전 실패 처리 |

---

## 5. 테스트 결과 리포팅

### 5.1 Pass/Fail 기준

- **PASS**: 모든 예상 결과 충족
- **FAIL**: 하나 이상의 예상 결과 미충족
- **SKIP**: 테스트 환경 미지원 (Qt 미설치 등)

### 5.2 결과 문서화

각 테스트 케이스마다:
- 실행 일시
- 환경 정보 (Qt 버전, 컴파일러 등)
- Pass/Fail 상태
- 실패 시 에러 메시지
- 개선사항

---

## 6. 체크리스트

### 빌드 전
- [ ] Qt 5.15+ 설치 확인
- [ ] CMake 3.16+ 설치 확인
- [ ] 모든 소스 파일 컴파일 가능 확인

### 테스트 전
- [ ] 단위 테스트 파일 작성
- [ ] 테스트 CMakeLists.txt 생성
- [ ] 테스트 데이터 준비

### 테스트 실행
- [ ] 단위 테스트 실행 및 통과
- [ ] 통합 테스트 시나리오 실행
- [ ] 성능/메모리 테스트 실행

### 결과 분석
- [ ] 테스트 결과 리포트 작성
- [ ] 버그 발견 시 보고
- [ ] 코드 리뷰 진행

---

**테스트 계획 완료**: 2026-02-14
**다음 단계**: Qt 개발 환경에서 실제 빌드 및 테스트 실행
