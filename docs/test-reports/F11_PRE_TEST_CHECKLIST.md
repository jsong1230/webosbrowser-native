# F-11 (설정 화면) - 테스트 전 체크리스트

**작성일**: 2026-02-14
**대상**: 개발자, QA 엔지니어
**목적**: 테스트 시작 전 모든 구현이 완료되었는지 확인

---

## 빌드 환경 확인

- [ ] Qt 5.15+ 설치됨
- [ ] CMake 3.16+ 설치됨
- [ ] webOS Native SDK 설치됨
- [ ] Google Test 설치됨

**확인 명령어**:
```bash
qmake --version          # Qt 버전 확인
cmake --version         # CMake 버전 확인
```

---

## 코드 완성도 확인

### SettingsService

- [ ] src/services/SettingsService.h 존재
- [ ] src/services/SettingsService.cpp 존재
- [ ] 생성자/소멸자 구현됨
- [ ] loadSettings() 구현됨
- [ ] searchEngine() getter 구현됨
- [ ] homepage() getter 구현됨
- [ ] theme() getter 구현됨
- [ ] setSearchEngine() 구현됨
- [ ] setHomepage() 구현됨
- [ ] setTheme() 구현됨
- [ ] saveToStorage() 구현됨
- [ ] 5개 시그널 선언됨

**확인 명령어**:
```bash
grep -n "class SettingsService" src/services/SettingsService.h
grep -n "signals:" src/services/SettingsService.h
```

### SettingsPanel

- [ ] src/ui/SettingsPanel.h 존재
- [ ] src/ui/SettingsPanel.cpp 존재
- [ ] setupUI() 구현됨
- [ ] setupConnections() 구현됨
- [ ] setupFocusOrder() 구현됨
- [ ] showPanel() 구현됨
- [ ] hidePanel() 구현됨
- [ ] keyPressEvent() 구현됨
- [ ] 8개 private slots 모두 구현됨
- [ ] QPropertyAnimation 포함됨

**확인 명령어**:
```bash
grep -n "void showPanel" src/ui/SettingsPanel.{h,cpp}
grep -n "void hidePanel" src/ui/SettingsPanel.{h,cpp}
```

### BrowserWindow 통합

- [ ] BrowserWindow.h에 Forward declaration (SettingsService, SettingsPanel)
- [ ] BrowserWindow.h에 멤버 변수 추가 (settingsService_, settingsPanel_)
- [ ] BrowserWindow.cpp에서 SettingsService 생성
- [ ] BrowserWindow.cpp에서 SettingsPanel 생성
- [ ] handleMenuButton() 구현됨
- [ ] applyTheme() 구현됨
- [ ] onThemeChanged() 슬롯 구현됨
- [ ] onHomepageChanged() 슬롯 구현됨

**확인 명령어**:
```bash
grep -n "SettingsService\|SettingsPanel" src/browser/BrowserWindow.h
grep -n "void handleMenuButton\|void applyTheme" src/browser/BrowserWindow.cpp
```

### NavigationBar 수정

- [ ] NavigationBar.h에 setHomepage() 메서드 추가
- [ ] NavigationBar.cpp에서 setHomepage() 구현
- [ ] onHomeClicked()에서 동적 홈페이지 사용

**확인 명령어**:
```bash
grep -n "setHomepage\|homepage_" src/ui/NavigationBar.h
```

---

## 테마 파일 확인

- [ ] resources/styles/dark.qss 존재
- [ ] resources/styles/light.qss 존재
- [ ] dark.qss에 모든 위젯 스타일 포함
  - [ ] QMainWindow
  - [ ] QPushButton (normal, hover, focus)
  - [ ] QLineEdit (normal, focus)
  - [ ] QRadioButton
  - [ ] QLabel
  - [ ] QListWidget
- [ ] light.qss에 모든 위젯 스타일 포함
- [ ] resources/resources.qrc 존재
- [ ] resources.qrc에 두 QSS 파일 등록됨

**확인 명령어**:
```bash
ls -la resources/styles/
cat resources/resources.qrc
```

---

## CMakeLists.txt 확인

- [ ] SettingsService.cpp SOURCES에 포함
- [ ] SettingsPanel.cpp SOURCES에 포함
- [ ] SettingsService.h HEADERS에 포함
- [ ] SettingsPanel.h HEADERS에 포함
- [ ] resources.qrc RESOURCES에 포함
- [ ] CMAKE_AUTOMOC ON
- [ ] CMAKE_AUTORCC ON
- [ ] Qt5 Widgets 링크됨

**확인 명령어**:
```bash
grep -n "SettingsService\|SettingsPanel\|resources.qrc" CMakeLists.txt
```

---

## 빌드 확인

- [ ] 프로젝트 빌드 성공
- [ ] 빌드 경고 없음
- [ ] 링크 에러 없음

**빌드 단계**:
```bash
mkdir -p build
cd build
cmake ..
make
```

**확인 내용**:
```
webosbrowser 실행 파일 생성되었는가?
빌드 시간이 5분 이상 걸렸는가? (정상)
에러 메시지가 없는가?
```

---

## 테스트 파일 준비

### 단위 테스트

- [ ] tests/unit/ 디렉토리 존재
- [ ] tests/unit/SettingsServiceTest.cpp 작성 준비
- [ ] GoogleTest CMakeLists.txt 설정 완료

### 통합 테스트

- [ ] tests/integration/ 디렉토리 존재
- [ ] tests/integration/SettingsPanelIntegrationTest.cpp 작성 준비

### E2E 테스트

- [ ] webOS 프로젝터/에뮬레이터 준비됨
- [ ] 테스트용 북마크 데이터 생성됨
- [ ] 리모컨 (또는 가상 리모컨) 사용 가능

---

## 코드 품질 확인

### Include 경로

- [ ] SettingsService.cpp: `#include "SettingsService.h"` ✓
- [ ] SettingsService.cpp: `#include "StorageService.h"` ✓
- [ ] SettingsService.cpp: `#include "../utils/URLValidator.h"` ✓
- [ ] SettingsPanel.cpp: `#include "SettingsPanel.h"` ✓
- [ ] SettingsPanel.cpp: `#include "../services/SettingsService.h"` ✓
- [ ] BrowserWindow.cpp: `#include "SettingsService.h"` ✓
- [ ] BrowserWindow.cpp: `#include "SettingsPanel.h"` ✓

**확인 명령어**:
```bash
head -15 src/services/SettingsService.cpp | grep "#include"
head -15 src/ui/SettingsPanel.cpp | grep "#include"
```

### Qt 의존성

- [ ] SettingsService: `Q_OBJECT` 포함
- [ ] SettingsPanel: `Q_OBJECT` 포함
- [ ] 모든 시그널/슬롯 연결 정상
- [ ] Parent-child 메모리 관리 정상

**확인 명령어**:
```bash
grep -n "Q_OBJECT" src/services/SettingsService.h
grep -n "Q_OBJECT" src/ui/SettingsPanel.h
```

### 코딩 컨벤션

- [ ] 클래스명 PascalCase (SettingsService, SettingsPanel)
- [ ] 멤버 변수 camelCase with suffix_ (searchEngine_, homepage_)
- [ ] 함수명 camelCase (loadSettings, setSearchEngine)
- [ ] 메서드 순서: public → protected → private
- [ ] pragma once 사용 (include guard)
- [ ] 네임스페이스 webosbrowser 사용

---

## 기능 동작 확인 (수동)

### 검색 엔진 선택

- [ ] 설정 패널에서 4개 검색 엔진 표시
- [ ] 라디오 버튼 선택 가능
- [ ] 선택 후 즉시 저장 (LS2 API)

**확인 단계**:
1. App 실행
2. Menu 버튼 → 설정 패널 표시
3. Naver 선택 → 저장 확인
4. URLBar에서 검색 → Naver 사용

### 홈페이지 설정

- [ ] QLineEdit에 URL 입력 가능
- [ ] URL 검증 동작
- [ ] 위험 프로토콜 (javascript:, file://) 차단
- [ ] about: URL 허용

**확인 단계**:
1. 설정 패널에서 홈페이지 입력
2. "https://www.github.com" 입력
3. Enter → 저장
4. Home 버튼 클릭 → GitHub 이동

### 테마 변경

- [ ] 다크/라이트 모드 라디오 표시
- [ ] 테마 선택 시 즉시 적용
- [ ] 색상 변경 확인 (배경, 텍스트, 테두리)

**확인 단계**:
1. 라이트 모드 선택
2. 전체 UI 색상 변경 확인
3. 다시 다크 모드 선택
4. 색상 원래대로 복원

### 브라우징 데이터 삭제

- [ ] 확인 다이얼로그 표시
- [ ] "삭제" 버튼 클릭 시 실제 삭제
- [ ] "취소" 버튼 클릭 시 삭제 안 됨

**확인 단계**:
1. 북마크 5개 생성
2. 설정 패널 → "북마크 전체 삭제"
3. 확인 다이얼로그 → "삭제"
4. 완료 토스트 메시지 표시 확인

### 리모컨 네비게이션

- [ ] 방향키로 항목 간 이동 가능
- [ ] Tab Order 정상
- [ ] Back 키로 패널 닫기

**확인 단계**:
1. Menu 버튼 → 설정 패널
2. 방향키 Down × 10 → 순환 네비게이션 확인
3. Back 키 → 패널 닫기

---

## 성능 확인

- [ ] 설정 로드 시간: 500ms 이내
- [ ] 설정 저장 시간: 200ms 이내 (또는 비동기 완료)
- [ ] 테마 변경 시간: 0.5초 이내
- [ ] 슬라이드 애니메이션: 0.3초 (부드러움)

**확인 방법**:
```cpp
// qDebug() 타이밍 로그 확인
[SettingsService] 설정 로드 완료: ...
[BrowserWindow] 테마 적용 완료: ...
```

---

## 에러 처리 확인

- [ ] StorageService 없을 때 처리
- [ ] 잘못된 URL 입력 시 에러 메시지
- [ ] LS2 저장 실패 시 에러 메시지
- [ ] 데이터 삭제 실패 시 에러 메시지

---

## 문서 확인

- [ ] F11_VALIDATION_REPORT.md 작성됨
- [ ] F11_VALIDATION_SUMMARY.txt 작성됨
- [ ] F11_TEST_SCENARIOS.md 작성됨
- [ ] 테스트 케이스 20개 정의됨

---

## 최종 서명

| 항목 | 담당자 | 완료 | 날짜 |
|------|--------|------|------|
| 코드 구현 | cpp-dev | ☐ | ___ |
| 빌드 확인 | cpp-dev | ☐ | ___ |
| 정적 검증 | test-runner | ☐ | ___ |
| 테스트 설계 | test-runner | ☐ | ___ |
| 최종 승인 | code-reviewer | ☐ | ___ |

---

## 다음 단계

모든 항목이 확인되면 다음을 진행합니다:

1. **단위 테스트 작성** (2시간)
   - SettingsServiceTest.cpp

2. **통합 테스트 작성** (2시간)
   - SettingsPanelIntegrationTest.cpp

3. **E2E 테스트 실행** (3시간)
   - 5개 사용자 시나리오

4. **코드 리뷰** (1시간)
   - 아키텍처, 컨벤션, 성능

5. **최종 배포 승인**
   - CHANGELOG.md 업데이트
   - features.md 상태 변경

---

**점검 완료 시 테스트 시작**
**예상 완료: 2026-02-17**
