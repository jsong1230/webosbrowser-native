# 로딩 인디케이터 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/loading-indicator/requirements.md
- 기술 설계서: docs/specs/loading-indicator/design.md
- WebView 헤더: src/browser/WebView.h
- BrowserWindow 헤더: src/browser/BrowserWindow.h

## 2. 구현 Phase

### Phase 1: 리소스 준비 및 헤더 정의
**목표**: LoadingIndicator 클래스 인터페이스 정의 및 필요한 리소스 파일 준비

- [ ] Task 1-1: GIF 스피너 리소스 생성 → cpp-dev
  - 파일: `resources/icons/spinner.gif`
  - 스펙: 80x80px, 12프레임, 투명 배경, 흰색 회전 원형
  - 또는 임시로 무료 아이콘 사용 (fontawesome 등)

- [ ] Task 1-2: 에러 아이콘 PNG 생성 → cpp-dev
  - 파일: `resources/icons/error.png`
  - 스펙: 80x80px, 투명 배경, 빨간색 삼각형 경고 표시

- [ ] Task 1-3: Qt 리소스 파일 생성 → cpp-dev
  - 파일: `resources/resources.qrc`
  - 내용: spinner.gif, error.png 등록
  - XML 형식으로 Qt 리소스 시스템 설정

- [ ] Task 1-4: LoadingIndicator 헤더 파일 작성 → cpp-dev
  - 파일: `src/ui/LoadingIndicator.h`
  - 클래스 인터페이스 정의 (public 메서드, 시그널/슬롯)
  - Doxygen 주석 작성
  - `#pragma once` 헤더 가드

**예상 산출물**:
- `resources/icons/spinner.gif`
- `resources/icons/error.png`
- `resources/resources.qrc`
- `src/ui/LoadingIndicator.h`

**완료 기준**:
- [ ] LoadingIndicator.h가 컴파일 에러 없이 빌드됨
- [ ] Qt Creator에서 resources.qrc가 정상 인식됨
- [ ] 헤더 파일에 모든 public 메서드가 Doxygen 주석 포함

**예상 소요 시간**: 1~2시간

---

### Phase 2: LoadingIndicator 클래스 구현
**목표**: LoadingIndicator 클래스의 핵심 기능 구현 (UI 구성, 애니메이션, 상태 관리)

- [ ] Task 2-1: LoadingIndicator.cpp 기본 구조 구현 → cpp-dev
  - 파일: `src/ui/LoadingIndicator.cpp`
  - 생성자/소멸자 구현
  - `setupUI()` 함수 구현 (QProgressBar, QLabel, QMovie 초기화)
  - `setupAnimations()` 함수 구현 (QPropertyAnimation 설정)

- [ ] Task 2-2: 프로그레스바 UI 구현 → cpp-dev
  - QProgressBar 생성 및 QSS 스타일 적용
  - 높이 8px, 녹색 (#00C851) 기본 색상
  - 상단 배치 (BrowserWindow 레이아웃 연동은 Phase 3)

- [ ] Task 2-3: 스피너 오버레이 UI 구현 → cpp-dev
  - QWidget 오버레이 컨테이너 생성
  - QLabel + QMovie로 GIF 스피너 표시
  - 상태 텍스트 QLabel ("페이지 로딩 중...")
  - URL 텍스트 QLabel (말줄임 처리)
  - 반투명 배경 (rgba(0,0,0,0.5)), 중앙 정렬

- [ ] Task 2-4: 진행률 업데이트 로직 구현 → cpp-dev
  - `setProgress(int)` 함수 구현
  - QTimer 기반 쓰로틀링 (100ms 간격, 10fps)
  - `onUpdateThrottled()` 슬롯 구현
  - cachedProgress_ 상태 관리

- [ ] Task 2-5: 페이드인/아웃 애니메이션 구현 → cpp-dev
  - `show()` 함수: QPropertyAnimation (opacity 0.0 → 1.0, 0.2초)
  - `hide(bool immediate)` 함수: QPropertyAnimation (opacity 1.0 → 0.0, 0.5초)
  - QGraphicsOpacityEffect 적용
  - `raise()` 호출로 z-order 최상위 배치

- [ ] Task 2-6: 에러 상태 처리 구현 → cpp-dev
  - `showError(QString)` 함수 구현
  - 프로그레스바 QSS 동적 변경 (빨간색 #ff4444)
  - 스피너 → 에러 아이콘(⚠️) 교체
  - 1초 후 hide() 타이머 설정
  - `resetToNormalState()` 함수 구현 (녹색 복원)

- [ ] Task 2-7: 타임아웃 경고 구현 → cpp-dev
  - `showTimeoutWarning()` 함수 구현
  - 상태 텍스트 변경: "로딩이 오래 걸리고 있습니다"
  - URL 텍스트 변경: "네트워크를 확인해주세요"

- [ ] Task 2-8: URL 표시 및 말줄임 처리 → cpp-dev
  - `setLoadingUrl(QString)` 함수 구현
  - QString::left(50) + "..." 처리
  - 또는 QFontMetrics::elidedText() 사용

**예상 산출물**:
- `src/ui/LoadingIndicator.cpp` (완성)

**완료 기준**:
- [ ] LoadingIndicator 단독 테스트 (QApplication 환경에서 show/hide 동작 확인)
- [ ] 프로그레스바가 0~100% 증가 애니메이션 정상 동작
- [ ] 스피너 GIF가 부드럽게 회전
- [ ] 페이드인/아웃 애니메이션이 0.2초/0.5초 동작
- [ ] 에러 상태 시 빨간색 프로그레스바 + 에러 아이콘 표시
- [ ] 컴파일 경고 없음 (Qt Creator 확인)

**예상 소요 시간**: 4~6시간

---

### Phase 3: BrowserWindow 통합 및 WebView 연동
**목표**: LoadingIndicator를 BrowserWindow에 통합하고 WebView 시그널 연결

- [ ] Task 3-1: BrowserWindow.h 수정 → cpp-dev
  - 파일: `src/browser/BrowserWindow.h`
  - `LoadingIndicator *loadingIndicator_` 멤버 변수 추가
  - `#include "ui/LoadingIndicator.h"` 추가

- [ ] Task 3-2: BrowserWindow.cpp setupUI() 수정 → cpp-dev
  - 파일: `src/browser/BrowserWindow.cpp`
  - LoadingIndicator 생성 (`new LoadingIndicator(centralWidget_)`)
  - 초기 상태: 숨김 (hide(true))

- [ ] Task 3-3: BrowserWindow.cpp setupConnections() 수정 → cpp-dev
  - WebView::loadStarted 시그널 연결 → LoadingIndicator::show()
  - WebView::loadProgress 시그널 연결 → LoadingIndicator::setProgress()
  - WebView::loadFinished 시그널 연결 → LoadingIndicator::hide(false)
  - WebView::urlChanged 시그널 연결 → LoadingIndicator::setLoadingUrl()
  - WebView::loadError 시그널 연결 → LoadingIndicator::showError()
  - WebView::loadTimeout 시그널 연결 → LoadingIndicator::showTimeoutWarning()

- [ ] Task 3-4: BrowserWindow.cpp resizeEvent() 오버라이드 → cpp-dev
  - 오버레이 위치 조정 로직 구현
  - WebView 중앙에 LoadingIndicator 배치
  - 크기: 400x200px (스피너 + 텍스트 표시)

- [ ] Task 3-5: WebView.cpp loadTimeout() 시그널 구현 → cpp-dev
  - 파일: `src/browser/WebView.cpp`
  - `loadStarted()` 시 QTimer::singleShot(30000) 시작
  - `loadFinished()` 시 타이머 취소
  - 30초 경과 시 `emit loadTimeout()` 시그널 발생
  - 헤더에 `void loadTimeout()` 시그널 추가 (아직 없다면)

**예상 산출물**:
- `src/browser/BrowserWindow.h` (수정)
- `src/browser/BrowserWindow.cpp` (수정)
- `src/browser/WebView.cpp` (수정, loadTimeout 구현)

**완료 기준**:
- [ ] BrowserWindow 실행 시 LoadingIndicator가 정상 생성됨
- [ ] 페이지 로딩 시작 시 LoadingIndicator 표시됨
- [ ] 진행률이 실시간으로 업데이트됨 (0% → 100%)
- [ ] 로딩 완료 시 페이드아웃 후 숨김
- [ ] WebView 중앙에 스피너 오버레이 정상 배치
- [ ] 창 크기 변경 시 오버레이 위치 재조정됨

**예상 소요 시간**: 3~4시간

---

### Phase 4: CMakeLists.txt 수정 및 빌드 설정
**목표**: Qt 리소스 파일 빌드 시스템 통합

- [ ] Task 4-1: CMakeLists.txt 수정 → cpp-dev
  - 파일: `CMakeLists.txt` (루트)
  - `qt5_add_resources()` 추가 (resources.qrc 등록)
  - LoadingIndicator.cpp를 SOURCES에 추가
  - Qt5Widgets 링크 확인 (이미 있음)

- [ ] Task 4-2: 빌드 테스트 → cpp-dev
  - `mkdir build && cd build`
  - `cmake ..`
  - `make`
  - 컴파일 에러 없이 빌드 성공 확인
  - 리소스 파일이 바이너리에 포함되었는지 확인 (qrc 내용)

**예상 산출물**:
- `CMakeLists.txt` (수정)

**완료 기준**:
- [ ] 프로젝트가 빌드 에러 없이 컴파일됨
- [ ] GIF 스피너가 정상 로드됨 (QMovie에서 리소스 경로 읽기)
- [ ] 에러 아이콘이 정상 로드됨 (QPixmap에서 리소스 경로 읽기)

**예상 소요 시간**: 1시간

---

### Phase 5: 통합 테스트 및 시나리오 검증
**목표**: 실제 webOS 환경에서 다양한 시나리오 테스트

- [ ] Task 5-1: 일반 페이지 로딩 테스트 → test-runner
  - 시나리오: www.google.com 로딩
  - 검증: 프로그레스바 0% → 100%, 스피너 회전, 페이드아웃

- [ ] Task 5-2: 느린 네트워크 환경 테스트 → test-runner
  - 시나리오: 네트워크 쓰로틀링 (webOS Developer Mode)
  - 검증: 진행률이 천천히 증가, 30초 초과 시 타임아웃 경고 표시

- [ ] Task 5-3: 로딩 에러 테스트 → test-runner
  - 시나리오: http://invalidurl.com 접속
  - 검증: 프로그레스바 빨간색, 에러 아이콘(⚠️), 1초 후 에러 페이지 전환

- [ ] Task 5-4: 빠른 페이지 로딩 테스트 (캐시) → test-runner
  - 시나리오: 이미 캐시된 페이지 재로딩
  - 검증: 프로그레스바 즉시 100% 이동 후 페이드아웃 (깜박임 없음)

- [ ] Task 5-5: 연속 로딩 테스트 → test-runner
  - 시나리오: 페이지 A 로딩 중 → 페이지 B 로딩 (탭 전환 시뮬레이션)
  - 검증: 이전 LoadingIndicator 정리, 새 로딩 시작

- [ ] Task 5-6: 리모컨 Back 버튼 테스트 → test-runner
  - 시나리오: 페이지 로딩 중 Back 버튼 누름
  - 검증: 로딩 취소, LoadingIndicator 즉시 숨김 (WebView::stop() 연동)

- [ ] Task 5-7: 창 크기 변경 테스트 → test-runner
  - 시나리오: 로딩 중 창 크기 변경 (리모컨 메뉴 버튼 등)
  - 검증: 오버레이 위치가 항상 중앙 유지

- [ ] Task 5-8: 대화면 프로젝터 시인성 테스트 → test-runner
  - 시나리오: LG 프로젝터 HU715QW (100인치) 3m 거리 시청
  - 검증: 프로그레스바 8px 높이가 보임, 스피너 80px 명확히 보임, 텍스트 24px 가독

**예상 산출물**:
- (테스트 결과 문서, 별도 파일)

**완료 기준**:
- [ ] 모든 시나리오가 예상대로 동작함
- [ ] 프로그레스바가 30fps 이상 부드럽게 애니메이션됨
- [ ] LoadingIndicator 메모리 사용량 3MB 이하 확인 (Qt Creator Profiler)
- [ ] CPU 사용률 5% 이하 확인 (로딩 애니메이션 중)
- [ ] 리모컨 포커스가 LoadingIndicator에 가지 않음 (NoFocus 정책)

**예상 소요 시간**: 4~5시간

---

### Phase 6: 단위 테스트 작성
**목표**: LoadingIndicator 클래스의 단위 테스트 작성 (Google Test + Qt Test)

- [ ] Task 6-1: 단위 테스트 파일 생성 → test-runner
  - 파일: `tests/unit/LoadingIndicatorTest.cpp`
  - Google Test 프레임워크 사용
  - QTest 유틸리티 (qWait 등) 활용

- [ ] Task 6-2: 초기 상태 테스트 → test-runner
  - 테스트: `InitialState` - LoadingIndicator가 생성 시 숨겨져 있는지 확인

- [ ] Task 6-3: show/hide 테스트 → test-runner
  - 테스트: `ShowHide` - show() 호출 시 표시, hide(true) 호출 시 즉시 숨김

- [ ] Task 6-4: 진행률 업데이트 테스트 → test-runner
  - 테스트: `SetProgress` - setProgress(50) 호출 후 100ms 대기, 진행률 반영 확인

- [ ] Task 6-5: 에러 상태 테스트 → test-runner
  - 테스트: `ErrorState` - showError() 호출 후 1초 대기, 자동 숨김 확인

- [ ] Task 6-6: 진행률 경계 테스트 → test-runner
  - 테스트: `ProgressBounds` - setProgress(-10), setProgress(150) 호출 시 qBound 동작 확인

- [ ] Task 6-7: 통합 테스트 작성 → test-runner
  - 파일: `tests/integration/WebViewLoadingTest.cpp`
  - WebView와 LoadingIndicator 통합 테스트
  - QSignalSpy로 시그널 연결 확인

**예상 산출물**:
- `tests/unit/LoadingIndicatorTest.cpp`
- `tests/integration/WebViewLoadingTest.cpp`

**완료 기준**:
- [ ] 모든 단위 테스트 통과 (Google Test 실행)
- [ ] 코드 커버리지 80% 이상 (주요 함수 모두 테스트)
- [ ] CI/CD 파이프라인에 테스트 자동 실행 추가 (향후)

**예상 소요 시간**: 3~4시간

---

### Phase 7: 코드 리뷰 및 문서 작성
**목표**: 코드 품질 검증 및 컴포넌트 문서 작성

- [ ] Task 7-1: 코드 리뷰 → code-reviewer
  - LoadingIndicator.h/cpp 코드 품질 검토
  - BrowserWindow 수정 사항 검토
  - Qt 코딩 컨벤션 준수 확인 (스마트 포인터, 네임스페이스 등)
  - 컴파일 경고 없음 확인

- [ ] Task 7-2: 성능 프로파일링 → code-reviewer
  - Qt Creator Profiler로 메모리 사용량 확인 (3MB 이하)
  - CPU 사용률 측정 (5% 이하)
  - 프레임레이트 측정 (30fps 이상)

- [ ] Task 7-3: 컴포넌트 문서 작성 → doc-writer
  - 파일: `docs/components/LoadingIndicator.md`
  - 내용: 개요, 주요 기능, API 문서, 사용 예제, 스타일 커스터마이징
  - Doxygen 문서 생성 (헤더 주석 기반)

- [ ] Task 7-4: 변경 이력 업데이트 → doc-writer
  - 파일: `docs/dev-log.md`
  - F-05 구현 완료 기록 (날짜, 주요 변경 사항, 파일 목록)
  - 파일: `CHANGELOG.md`
  - F-05 추가 (버전, 기능 설명)

**예상 산출물**:
- `docs/components/LoadingIndicator.md`
- `docs/dev-log.md` (업데이트)
- `CHANGELOG.md` (업데이트)

**완료 기준**:
- [ ] 코드 리뷰에서 지적된 사항 모두 수정됨
- [ ] 컴파일 경고 0건
- [ ] Doxygen 문서가 정상 생성됨
- [ ] 컴포넌트 문서에 사용 예제 포함됨

**예상 소요 시간**: 2~3시간

---

## 3. 태스크 의존성

```
Phase 1 (리소스 준비)
  ├── Task 1-1: spinner.gif 생성
  ├── Task 1-2: error.png 생성
  ├── Task 1-3: resources.qrc 생성
  └── Task 1-4: LoadingIndicator.h 작성
         │
         ▼
Phase 2 (LoadingIndicator 구현)
  ├── Task 2-1: LoadingIndicator.cpp 기본 구조
  ├── Task 2-2: 프로그레스바 UI
  ├── Task 2-3: 스피너 오버레이 UI
  ├── Task 2-4: 진행률 업데이트 로직
  ├── Task 2-5: 페이드인/아웃 애니메이션
  ├── Task 2-6: 에러 상태 처리
  ├── Task 2-7: 타임아웃 경고
  └── Task 2-8: URL 표시 및 말줄임
         │
         ▼
Phase 3 (BrowserWindow 통합)
  ├── Task 3-1: BrowserWindow.h 수정
  ├── Task 3-2: BrowserWindow setupUI()
  ├── Task 3-3: BrowserWindow setupConnections()
  ├── Task 3-4: BrowserWindow resizeEvent()
  └── Task 3-5: WebView loadTimeout() 구현
         │
         ▼
Phase 4 (빌드 설정)
  ├── Task 4-1: CMakeLists.txt 수정
  └── Task 4-2: 빌드 테스트
         │
         ▼
Phase 5 (통합 테스트)
  ├── Task 5-1 ~ 5-8: 다양한 시나리오 테스트
         │
         ▼
Phase 6 (단위 테스트)
  ├── Task 6-1 ~ 6-7: 단위 테스트 작성
         │
         ▼
Phase 7 (코드 리뷰 및 문서)
  ├── Task 7-1: 코드 리뷰
  ├── Task 7-2: 성능 프로파일링
  ├── Task 7-3: 컴포넌트 문서
  └── Task 7-4: 변경 이력 업데이트
```

**의존성 요약**:
- Phase 1 완료 후 Phase 2 시작 가능 (헤더 의존)
- Phase 2 완료 후 Phase 3 시작 가능 (LoadingIndicator 구현 완료)
- Phase 3 완료 후 Phase 4 시작 가능 (통합 코드 완료)
- Phase 4 완료 후 Phase 5 시작 가능 (빌드 성공 필요)
- Phase 5와 Phase 6은 병렬 가능 (통합 테스트 vs 단위 테스트)
- Phase 7은 모든 Phase 완료 후 시작 (최종 검증)

---

## 4. 병렬 실행 판단 (PG-1 그룹)

### PG-1 병렬 배치: F-03 (네비게이션), F-04 (히스토리), F-05 (로딩 인디케이터)

#### 병렬 가능 여부: **부분 병렬 가능**

#### 병렬 가능한 태스크:
- **Phase 1 (F-05) || Phase 1 (F-03, F-04)**:
  - F-05의 리소스 준비(spinner.gif, error.png)는 F-03, F-04와 독립적
  - LoadingIndicator.h 작성도 독립적 (WebView 시그널만 의존)

- **Phase 2 (F-05) || Phase 2 (F-03, F-04)**:
  - LoadingIndicator 구현은 NavigationBar, HistoryPanel과 독립적
  - 모두 BrowserWindow에 통합되지만 서로 다른 위젯

- **Phase 5 (F-05) || Phase 5 (F-03, F-04)**:
  - 통합 테스트는 병렬 실행 가능 (각 기능 독립 테스트)

#### 순차 필수 태스크:
- **Phase 3 (F-05) ← Phase 3 (F-03, F-04)**:
  - BrowserWindow.cpp setupConnections()에서 모두 시그널 연결
  - 충돌 가능 (동일 파일 수정)
  - 대응: F-03, F-04 완료 후 F-05 통합 권장

- **Phase 4 (F-05) ← Phase 4 (F-03, F-04)**:
  - CMakeLists.txt 수정 (SOURCES에 파일 추가)
  - 충돌 가능 (동일 파일 수정)
  - 대응: Git merge로 통합 가능하지만 순차 권장

#### Agent Team 사용 권장 여부: **No (순차 권장)**

#### 권장 이유:
1. **BrowserWindow 통합 충돌**: F-03, F-04, F-05 모두 BrowserWindow.cpp를 수정하므로 Git 충돌 가능성 높음
2. **시그널/슬롯 통합**: WebView 시그널 연결 코드가 같은 `setupConnections()` 함수에 집중
3. **CMakeLists.txt 충돌**: 세 기능 모두 SOURCES에 파일 추가하므로 순차 작업 효율적
4. **리소스 파일 통합**: resources.qrc에 각 기능의 아이콘 추가 시 충돌 가능
5. **테스트 단계 통합**: 통합 테스트는 병렬 가능하지만, 빌드 환경 공유로 인한 간섭 가능

#### 권장 작업 순서:
1. **F-03 (네비게이션)** 먼저 구현 (NavigationBar는 BrowserWindow 상단 툴바)
2. **F-04 (히스토리)** 다음 구현 (HistoryPanel은 사이드바)
3. **F-05 (로딩 인디케이터)** 마지막 구현 (WebView 오버레이)
4. 이유: UI 레이아웃 순서 (툴바 → 사이드바 → 오버레이)가 자연스러움

#### 대안: 부분 병렬 전략
만약 병렬 작업이 필요하다면:
- **Phase 1~2는 병렬** (각 기능의 헤더 및 구현 파일 독립 작성)
- **Phase 3 이후는 순차** (BrowserWindow 통합 시 충돌 방지)
- **Git 브랜치 전략**: `feature/F-03-navigation`, `feature/F-04-history`, `feature/F-05-loading` 분리 후, 메인에 순차 머지

---

## 5. 예상 소요 시간

### Phase별 소요 시간
| Phase | 작업 내용 | 예상 시간 |
|-------|---------|---------|
| Phase 1 | 리소스 준비 및 헤더 정의 | 1~2시간 |
| Phase 2 | LoadingIndicator 클래스 구현 | 4~6시간 |
| Phase 3 | BrowserWindow 통합 및 WebView 연동 | 3~4시간 |
| Phase 4 | CMakeLists.txt 수정 및 빌드 설정 | 1시간 |
| Phase 5 | 통합 테스트 및 시나리오 검증 | 4~5시간 |
| Phase 6 | 단위 테스트 작성 | 3~4시간 |
| Phase 7 | 코드 리뷰 및 문서 작성 | 2~3시간 |

### 총 예상 소요 시간
- **최소**: 18시간
- **최대**: 25시간
- **평균**: 21.5시간 (약 3일, 1일 7시간 기준)

### 시간 분포
- **구현**: 9~13시간 (Phase 1~4)
- **테스트**: 7~9시간 (Phase 5~6)
- **문서화**: 2~3시간 (Phase 7)

---

## 6. 리스크

### 리스크 1: GIF 스피너 리소스 품질
| 항목 | 내용 |
|-----|------|
| **리스크** | 직접 제작한 GIF 스피너의 품질이 낮거나 투명 배경 지원 부족 |
| **영향** | 스피너가 부자연스럽게 보이거나 배경색과 충돌 (사용자 경험 저해) |
| **발생 확률** | 중 (30%) |
| **대응 방안** | - 임시로 무료 아이콘 라이브러리 사용 (fontawesome, material-design-icons) <br> - 또는 QProgressBar BusyIndicator 스타일 대체 (GIF 없이 QPainter 회전) <br> - LG webOS 디자인 가이드 참고하여 스피너 스타일 통일 |

### 리스크 2: QMovie 성능 이슈
| 항목 | 내용 |
|-----|------|
| **리스크** | QMovie GIF 애니메이션이 webOS 프로젝터 환경에서 끊김 또는 CPU 사용률 높음 |
| **영향** | 부드럽지 않은 스피너 회전 (30fps 미만), CPU 5% 초과 사용 |
| **발생 확률** | 중 (25%) |
| **대응 방안** | - GIF 프레임 수 감소 (12프레임 → 8프레임) <br> - QMovie 캐싱 활성화 (`setCacheMode(QMovie::CacheAll)`) <br> - QPainter + QTimer 커스텀 회전 애니메이션으로 대체 (GPU 가속) <br> - 하드웨어 테스트 조기 실시 (Phase 5 전에 Phase 2 검증) |

### 리스크 3: BrowserWindow 통합 시 레이아웃 충돌
| 항목 | 내용 |
|-----|------|
| **리스크** | LoadingIndicator를 BrowserWindow 레이아웃에 추가 시 기존 WebView 레이아웃과 충돌 |
| **영향** | 오버레이가 제대로 표시되지 않거나 WebView가 가려짐 |
| **발생 확률** | 중 (20%) |
| **대응 방안** | - resizeEvent() 오버라이드로 수동 위치 조정 (mainLayout_ 사용 안 함) <br> - QStackedWidget 대안 고려 (오버레이 전용 레이어) <br> - raise() + setGeometry()로 z-order 명시적 관리 <br> - 디버깅: Qt Creator UI Debugger로 위젯 계층 구조 확인 |

### 리스크 4: 프로그레스바 쓰로틀링 부작용
| 항목 | 내용 |
|-----|------|
| **리스크** | 100ms 쓰로틀링으로 인해 진행률 업데이트가 끊기거나 최종 100%에 도달하지 못함 |
| **영향** | 사용자가 로딩 완료를 인지하지 못하거나, 프로그레스바가 95%에서 멈춤 |
| **발생 확률** | 낮음 (15%) |
| **대응 방안** | - loadFinished(true) 시그널 수신 시 즉시 setProgress(100) 호출 (쓰로틀링 우회) <br> - QTimer 간격을 50ms로 단축 (20fps, 부드러움 vs 성능 균형) <br> - cachedProgress_ 업데이트 로직 검증 (경계 조건 테스트) <br> - 통합 테스트에서 빠른 로딩 시나리오 집중 테스트 |

### 리스크 5: WebView loadTimeout() 시그널 누락
| 항목 | 내용 |
|-----|------|
| **리스크** | WebView.h에 loadTimeout() 시그널이 정의되어 있지 않아 추가 구현 필요 |
| **영향** | 30초 타임아웃 경고 기능이 동작하지 않음 |
| **발생 확률** | 중 (30%) |
| **대응 방안** | - WebView.cpp에서 QTimer::singleShot(30000) 구현 <br> - loadStarted() 시 타이머 시작, loadFinished() 시 타이머 정지 <br> - WebView.h에 `signals: void loadTimeout();` 추가 <br> - F-02 설계서 확인 (이미 정의되어 있을 가능성) |

### 리스크 6: 페이드아웃 애니메이션 중 숨김 타이밍
| 항목 | 내용 |
|-----|------|
| **리스크** | 페이드아웃 애니메이션(0.5초) 중 새 로딩 시작 시, 이전 인디케이터가 완전히 사라지지 않음 |
| **영향** | 화면에 두 개의 LoadingIndicator가 겹쳐 보임 (시각적 버그) |
| **발생 확률** | 중 (20%) |
| **대응 방안** | - loadStarted() 시그널 수신 시 즉시 hide(true) 호출 (이전 애니메이션 취소) <br> - QPropertyAnimation::stop() 호출로 진행 중인 애니메이션 중단 <br> - resetToNormalState()에서 애니메이션 상태 초기화 <br> - 연속 로딩 시나리오 집중 테스트 (Phase 5-5) |

### 리스크 7: Qt 리소스 시스템 빌드 실패
| 항목 | 내용 |
|-----|------|
| **리스크** | CMakeLists.txt에서 qt5_add_resources() 설정 오류로 리소스 파일 빌드 실패 |
| **영향** | spinner.gif, error.png 리소스 로드 실패 → 스피너 표시 안 됨 |
| **발생 확률** | 낮음 (10%) |
| **대응 방안** | - CMake 3.16+ 버전 확인 (qt5_add_resources 지원) <br> - resources.qrc 파일 경로 확인 (절대 경로 vs 상대 경로) <br> - 빌드 로그에서 qrc 컴파일 메시지 확인 <br> - 대체: Qt Creator에서 .qrc 파일 직접 추가 (UI로 설정) |

### 리스크 8: 대화면 프로젝터 시인성 부족
| 항목 | 내용 |
|-----|------|
| **리스크** | 100인치 대화면에서 프로그레스바 8px, 스피너 80px가 너무 작아 보이지 않음 |
| **영향** | 사용자가 로딩 중임을 인지하지 못함 (가독성 저해) |
| **발생 확률** | 낮음 (10%) |
| **대응 방안** | - 실제 디바이스 조기 테스트 (Phase 5-8 최우선) <br> - 프로그레스바 높이 12px로 증가 (QSS 수정) <br> - 스피너 크기 120px로 증가 <br> - 텍스트 폰트 크기 28px로 증가 <br> - LG webOS 디자인 가이드 참고 (권장 UI 크기) |

---

## 7. 완료 기준 (Definition of Done)

### 기능 완료
- [ ] LoadingIndicator 클래스가 모든 요구사항(FR-1~FR-7) 충족
- [ ] 프로그레스바가 실제 진행률(0~100%) 실시간 표시
- [ ] 스피너 애니메이션이 부드럽게 회전 (30fps 이상)
- [ ] 페이드인/아웃 애니메이션 정상 동작
- [ ] 에러 상태 시각적 피드백 (빨간색 프로그레스바 + 에러 아이콘)
- [ ] 타임아웃 경고 표시 (30초 초과)
- [ ] BrowserWindow에 정상 통합 (WebView 시그널 연결)

### 품질 완료
- [ ] 모든 단위 테스트 통과 (Google Test)
- [ ] 모든 통합 테스트 통과 (Phase 5 시나리오 8개)
- [ ] 코드 커버리지 80% 이상
- [ ] 컴파일 경고 0건
- [ ] 메모리 사용량 3MB 이하
- [ ] CPU 사용률 5% 이하
- [ ] 프레임레이트 30fps 이상

### 문서 완료
- [ ] LoadingIndicator.h Doxygen 주석 작성
- [ ] docs/components/LoadingIndicator.md 작성
- [ ] docs/dev-log.md 업데이트 (F-05 완료 기록)
- [ ] CHANGELOG.md 업데이트 (F-05 추가)

### 코드 리뷰 완료
- [ ] code-reviewer 검토 통과
- [ ] Qt 코딩 컨벤션 준수 (C++17, 스마트 포인터, 네임스페이스)
- [ ] 성능 프로파일링 통과 (메모리, CPU, FPS)

### 배포 준비
- [ ] LG 프로젝터 HU715QW에서 실제 동작 확인
- [ ] webOS Native CLI로 IPK 생성 성공
- [ ] 프로젝터에 설치 및 실행 성공

---

## 8. 의존 기능 확인

### F-02 (WebView) 의존성
- **필요한 시그널**:
  - `void loadStarted()` ✓ (이미 정의됨)
  - `void loadProgress(int progress)` ✓ (이미 정의됨)
  - `void loadFinished(bool success)` ✓ (이미 정의됨)
  - `void urlChanged(const QUrl &url)` ✓ (이미 정의됨)
  - `void loadError(const QString &errorString)` ✓ (이미 정의됨)
  - `void loadTimeout()` ? (확인 필요, 없으면 Phase 3-5에서 추가)

### F-04 (네비게이션) 연동
- **연동 내용**: 뒤로/앞으로 버튼 클릭 시에도 로딩 인디케이터 동작
- **구현**: WebView 시그널 기반이므로 자동 동작 (추가 코드 불필요)
- **확인**: Phase 5 통합 테스트에서 검증

### F-10 (에러 처리) 연동
- **연동 내용**: 로딩 실패 시 1초 후 에러 페이지 전환
- **구현**: LoadingIndicator::showError()에서 1초 타이머 처리
- **확인**: Phase 5-3 에러 테스트에서 검증

---

## 9. 파일 목록

### 새로 생성할 파일
- `src/ui/LoadingIndicator.h`
- `src/ui/LoadingIndicator.cpp`
- `resources/icons/spinner.gif`
- `resources/icons/error.png`
- `resources/resources.qrc`
- `docs/components/LoadingIndicator.md`
- `tests/unit/LoadingIndicatorTest.cpp`
- `tests/integration/WebViewLoadingTest.cpp`

### 수정할 기존 파일
- `src/browser/BrowserWindow.h` (LoadingIndicator 멤버 추가)
- `src/browser/BrowserWindow.cpp` (setupUI, setupConnections, resizeEvent 수정)
- `src/browser/WebView.cpp` (loadTimeout 시그널 구현)
- `CMakeLists.txt` (qt5_add_resources, LoadingIndicator.cpp 추가)
- `docs/dev-log.md` (F-05 완료 기록)
- `CHANGELOG.md` (F-05 추가)

---

## 10. 참고 사항

### Qt 관련 참고 자료
- QProgressBar: https://doc.qt.io/qt-5/qprogressbar.html
- QMovie: https://doc.qt.io/qt-5/qmovie.html
- QPropertyAnimation: https://doc.qt.io/qt-5/qpropertyanimation.html
- Qt Style Sheets: https://doc.qt.io/qt-5/stylesheet-reference.html

### webOS 관련 참고 자료
- webOS Native API: https://webostv.developer.lge.com/develop/native-apps/native-app-overview
- Qt for webOS: https://webostv.developer.lge.com/develop/native-apps/webos-qt-overview

### 프로젝트 문서
- PRD: docs/project/prd.md
- 요구사항: docs/specs/loading-indicator/requirements.md
- 설계: docs/specs/loading-indicator/design.md

---

## 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | Native App 관점 전면 재작성 (Web App → C++/Qt 전환) | product-manager |
| 2026-02-14 | Phase 1~7 계획 수립 (리소스 준비 → 구현 → 테스트 → 문서) | product-manager |
