# 웹뷰 통합 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/webview-integration/requirements.md
- 기술 설계서: docs/specs/webview-integration/design.md
- PRD: docs/project/prd.md
- CLAUDE.md: /Users/jsong/dev/jsong1230-github/webosbrowser-native/CLAUDE.md

## 2. 개요

F-02 웹뷰 통합 기능은 webOS Browser Native App의 핵심 기능으로, Qt WebEngineView를 활용하여 실제 외부 웹 페이지를 렌더링합니다. 이 기능은 Web App PoC의 iframe 제약을 극복하고, YouTube, Netflix 등 주요 사이트를 프로젝터에서 탐색할 수 있는 기반을 제공합니다.

이 구현은 **단일 개발자가 순차적으로 진행해야 하는 Core 기능**입니다. WebView 클래스의 PIMPL 패턴 구현과 BrowserWindow 통합이 밀접하게 연관되어 있어 병렬 실행이 불가능합니다.

### 주요 특징
- PIMPL 패턴으로 구현 세부사항 캡슐화
- Qt 시그널/슬롯으로 이벤트 주도 아키텍처 구현
- 30초 타임아웃 메커니즘으로 로딩 실패 감지
- 리모컨 입력을 위한 키 이벤트 처리 준비

## 3. 구현 Phase

### Phase 1: 개발 환경 확인 및 CMake 설정
**목표**: Qt WebEngineWidgets 모듈 의존성 추가 및 빌드 환경 확인

#### 태스크
- [ ] **Task 1-1**: CMakeLists.txt에 Qt5::WebEngineWidgets 모듈 추가
  - 파일: `CMakeLists.txt`
  - 작업: `find_package(Qt5 REQUIRED COMPONENTS ... WebEngineWidgets)` 추가
  - 작업: `target_link_libraries(webosbrowser ... Qt5::WebEngineWidgets)` 추가
  - 예상 시간: 15분

- [ ] **Task 1-2**: 빌드 테스트
  - 명령: `mkdir -p build && cd build && cmake .. && make`
  - 확인: Qt WebEngineWidgets 모듈 정상 링크
  - 예상 시간: 15분

#### 완료 기준
- CMake 빌드 성공
- Qt WebEngineWidgets 라이브러리 링크 확인
- 기존 코드 (BrowserWindow) 정상 실행

#### 예상 소요 시간
30분

---

### Phase 2: WebView 클래스 스켈레톤 구현
**목표**: WebView.h/cpp 파일 생성 및 PIMPL 패턴 기본 구조 구현

#### 태스크
- [ ] **Task 2-1**: WebView.h 생성 (공개 인터페이스)
  - 파일: `src/browser/WebView.h`
  - 작업:
    - `#pragma once` 헤더 가드
    - `namespace webosbrowser` 선언
    - `LoadingState` enum class 정의 (Idle, Loading, Loaded, Error)
    - `WebView` 클래스 선언 (QWidget 상속)
    - 공개 API 선언 (load, reload, stop, goBack, goForward 등)
    - 시그널 선언 (loadStarted, loadProgress, loadFinished, titleChanged, urlChanged, loadError)
    - PIMPL 패턴 선언 (WebViewPrivate, QScopedPointer, Q_DECLARE_PRIVATE)
  - 참조: design.md 섹션 4 "WebView.h (공개 인터페이스)"
  - 예상 시간: 1시간

- [ ] **Task 2-2**: WebView.cpp 생성 (구현 스켈레톤)
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - `#include "WebView.h"` 추가
    - Qt 헤더 포함 (QWebEngineView, QVBoxLayout, QTimer, QDebug)
    - `WebViewPrivate` 클래스 정의 (멤버 변수만, 구현은 Phase 3)
    - `WebView` 생성자/소멸자 구현 (qDebug 로그만)
    - 빈 메서드 스텁 생성 (컴파일 에러 방지)
  - 예상 시간: 1시간

- [ ] **Task 2-3**: CMakeLists.txt에 소스 파일 추가
  - 파일: `CMakeLists.txt`
  - 작업: `add_executable()` 또는 `target_sources()`에 WebView.cpp 추가
  - 예상 시간: 10분

- [ ] **Task 2-4**: 빌드 테스트
  - 명령: `cd build && make`
  - 확인: 컴파일 에러 없이 빌드 성공
  - 예상 시간: 10분

#### 완료 기준
- WebView.h 공개 인터페이스 선언 완료
- WebView.cpp 스켈레톤 구현 완료 (빈 메서드 스텁)
- 빌드 성공 (링크 에러 없음)

#### 예상 소요 시간
2시간 20분

---

### Phase 3: QWebEngineView 통합 및 기본 로딩 기능
**목표**: QWebEngineView를 WebView에 통합하고 기본 페이지 로딩 구현

#### 태스크
- [ ] **Task 3-1**: WebViewPrivate 클래스 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - `QWebEngineView *webEngineView` 멤버 변수 추가
    - `LoadingState currentState` 멤버 변수 추가
    - `QTimer *timeoutTimer` 멤버 변수 추가
    - `qint64 loadStartTime`, `int loadTimeout` 멤버 변수 추가
    - 생성자에서 QWebEngineView 인스턴스 생성
    - QTimer 초기화 (30초 타임아웃, singleShot)
  - 참조: design.md 섹션 4 "WebViewPrivate 클래스"
  - 예상 시간: 1시간

- [ ] **Task 3-2**: WebView 생성자 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - QVBoxLayout 생성 및 QWebEngineView 배치
    - 레이아웃 마진 제거 (setContentsMargins(0,0,0,0))
    - qDebug() 로그 추가 ("[WebView] 생성 중...")
  - 예상 시간: 30분

- [ ] **Task 3-3**: load() 메서드 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - `d->webEngineView->load(url)` 호출
    - qDebug() 로그 추가
  - 예상 시간: 15분

- [ ] **Task 3-4**: reload(), stop() 메서드 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - `d->webEngineView->reload()` 호출
    - `d->webEngineView->stop()` 호출
    - stop() 시 타임아웃 타이머 정지
    - qDebug() 로그 추가
  - 예상 시간: 15분

- [ ] **Task 3-5**: url(), title(), state() 메서드 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - `d->webEngineView->url()` 반환
    - `d->webEngineView->title()` 반환
    - `d->currentState` 반환
  - 예상 시간: 15분

#### 완료 기준
- QWebEngineView가 WebView에 통합됨
- load() 호출 시 페이지 로딩 시작 (아직 이벤트 처리는 안 함)
- 빌드 및 실행 성공

#### 예상 소요 시간
2시간 15분

---

### Phase 4: 로딩 이벤트 처리 구현
**목표**: loadStarted, loadProgress, loadFinished 시그널 연결 및 상태 관리

#### 태스크
- [ ] **Task 4-1**: private 슬롯 선언
  - 파일: `src/browser/WebView.h`
  - 작업:
    - `private slots:` 섹션 추가
    - `void onLoadStarted();` 선언
    - `void onLoadProgress(int progress);` 선언
    - `void onLoadFinished(bool ok);` 선언
    - `void onLoadTimeout();` 선언
  - 예상 시간: 10분

- [ ] **Task 4-2**: 시그널/슬롯 연결 (생성자)
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - QWebEngineView::loadStarted → WebView::onLoadStarted 연결
    - QWebEngineView::loadProgress → WebView::onLoadProgress 연결
    - QWebEngineView::loadFinished → WebView::onLoadFinished 연결
    - QWebEngineView::titleChanged → WebView::titleChanged 직접 전달
    - QWebEngineView::urlChanged → WebView::urlChanged 직접 전달
    - QTimer::timeout → WebView::onLoadTimeout 연결
  - 예상 시간: 30분

- [ ] **Task 4-3**: onLoadStarted() 슬롯 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - `d->currentState = LoadingState::Loading` 설정
    - `d->loadStartTime = QDateTime::currentMSecsSinceEpoch()` 기록
    - `d->timeoutTimer->start(d->loadTimeout)` 타이머 시작
    - `emit loadStarted()` 시그널 발생
    - qDebug() 로그 추가
  - 예상 시간: 20분

- [ ] **Task 4-4**: onLoadProgress() 슬롯 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - `emit loadProgress(progress)` 시그널 전달
    - qDebug() 로그 추가 (진행률 출력)
  - 예상 시간: 10분

- [ ] **Task 4-5**: onLoadFinished() 슬롯 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - 타임아웃 타이머 정지 (`d->timeoutTimer->stop()`)
    - 로딩 소요 시간 계산 및 로그
    - 성공 시: `d->currentState = LoadingState::Loaded`, `emit loadFinished(true)`
    - 실패 시: `d->currentState = LoadingState::Error`, `emit loadError(...)`, `emit loadFinished(false)`
    - qDebug() 로그 추가
  - 참조: design.md 섹션 4 "onLoadFinished 구현"
  - 예상 시간: 30분

- [ ] **Task 4-6**: onLoadTimeout() 슬롯 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - 현재 상태가 Loading인 경우만 처리
    - `d->webEngineView->stop()` 호출
    - `d->currentState = LoadingState::Error` 설정
    - `emit loadError(-2, "페이지 로딩 시간이 초과되었습니다 (30초)", url)` 시그널 발생
    - `emit loadFinished(false)` 시그널 발생
    - qDebug() 로그 추가
  - 예상 시간: 20분

#### 완료 기준
- loadStarted, loadProgress, loadFinished 시그널이 정상 발생
- 로딩 소요 시간 계산 및 로그 출력
- 타임아웃 30초 초과 시 에러 처리
- 상태 전이 정상 동작 (Idle → Loading → Loaded/Error)

#### 예상 소요 시간
2시간

---

### Phase 5: 네비게이션 API 구현
**목표**: 뒤로/앞으로 가기 기능 구현 (F-04 연계 준비)

#### 태스크
- [ ] **Task 5-1**: canGoBack(), canGoForward() 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - `d->webEngineView->history()->canGoBack()` 반환
    - `d->webEngineView->history()->canGoForward()` 반환
  - 예상 시간: 15분

- [ ] **Task 5-2**: goBack(), goForward() 구현
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - `d->webEngineView->back()` 호출
    - `d->webEngineView->forward()` 호출
    - qDebug() 로그 추가
  - 예상 시간: 15분

#### 완료 기준
- canGoBack()/canGoForward() API가 히스토리 상태 정확히 반환
- goBack()/goForward() 호출 시 페이지 네비게이션 동작

#### 예상 소요 시간
30분

---

### Phase 6: BrowserWindow 통합
**목표**: BrowserWindow에 WebView 추가 및 시그널/슬롯 연결

#### 태스크
- [ ] **Task 6-1**: BrowserWindow.h 수정
  - 파일: `src/browser/BrowserWindow.h`
  - 작업:
    - `#include "WebView.h"` 추가
    - `WebView *webView_` 멤버 변수 추가
    - private 슬롯 선언 (6개 이벤트 핸들러):
      - `void onWebViewLoadStarted();`
      - `void onWebViewLoadProgress(int progress);`
      - `void onWebViewLoadFinished(bool success);`
      - `void onWebViewTitleChanged(const QString &title);`
      - `void onWebViewUrlChanged(const QUrl &url);`
      - `void onWebViewLoadError(int errorCode, const QString &errorMessage, const QUrl &url);`
  - 참조: design.md 섹션 5 "BrowserWindow.h 수정"
  - 예상 시간: 30분

- [ ] **Task 6-2**: BrowserWindow.cpp 생성자 수정
  - 파일: `src/browser/BrowserWindow.cpp`
  - 작업:
    - WebView 인스턴스 생성 (`webView_ = new WebView(centralWidget_)`)
    - setupUI()에서 레이아웃에 WebView 추가
    - stretch factor 설정 (`mainLayout_->addWidget(webView_, 1)`)
  - 예상 시간: 20분

- [ ] **Task 6-3**: setupConnections() 메서드 구현
  - 파일: `src/browser/BrowserWindow.cpp`
  - 작업:
    - 6개 WebView 시그널을 BrowserWindow 슬롯에 연결
    - qDebug() 로그 추가 ("[BrowserWindow] 시그널/슬롯 연결 완료")
  - 참조: design.md 섹션 5 "setupConnections()"
  - 예상 시간: 20분

- [ ] **Task 6-4**: 이벤트 핸들러 슬롯 구현
  - 파일: `src/browser/BrowserWindow.cpp`
  - 작업:
    - `onWebViewLoadStarted()`: qDebug() 로그만 (F-05에서 로딩 인디케이터 추가 예정)
    - `onWebViewLoadProgress(int)`: 진행률 로그 출력
    - `onWebViewLoadFinished(bool)`: 성공/실패 로그, 페이지 제목/URL 출력
    - `onWebViewTitleChanged(QString)`: 제목 변경 로그 (F-06 탭 제목 업데이트 예정)
    - `onWebViewUrlChanged(QUrl)`: URL 변경 로그 (F-03 URLBar 업데이트, F-08 히스토리 저장 예정)
    - `onWebViewLoadError(int, QString, QUrl)`: 에러 정보 로그 (F-13 에러 페이지 표시 예정)
  - 참조: design.md 섹션 5 "이벤트 핸들러 구현"
  - 예상 시간: 40분

- [ ] **Task 6-5**: 초기 URL 로드
  - 파일: `src/browser/BrowserWindow.cpp`
  - 작업:
    - 생성자 마지막에 `webView_->load(QUrl("https://www.google.com"))` 추가
    - qDebug() 로그 추가
  - 예상 시간: 10분

#### 완료 기준
- BrowserWindow에 WebView가 중앙 영역에 배치됨
- 6개 이벤트 핸들러가 정상 호출되며 qDebug() 로그 출력
- 초기 URL (Google 홈) 자동 로드

#### 예상 소요 시간
2시간

---

### Phase 7: 로컬 테스트 및 디버깅
**목표**: 개발 환경(macOS)에서 기능 검증

#### 태스크
- [ ] **Task 7-1**: 빌드 및 실행
  - 명령: `cd build && make && ./webosbrowser`
  - 확인: 앱 정상 실행, WebView 표시
  - 예상 시간: 15분

- [ ] **Task 7-2**: 기본 로딩 테스트
  - 테스트:
    - Google 홈페이지 자동 로드 확인
    - 콘솔 로그 확인 (loadStarted, loadProgress, loadFinished)
    - 웹 콘텐츠 정상 표시 확인
  - 예상 시간: 15분

- [ ] **Task 7-3**: 로딩 이벤트 시그널 검증
  - 테스트:
    - loadStarted 시그널 발생 확인
    - loadProgress 0~100% 증가 확인
    - loadFinished(true) 시그널 확인
    - 로딩 소요 시간 로그 확인 (2~5초 이내)
  - 예상 시간: 15분

- [ ] **Task 7-4**: 여러 사이트 테스트
  - 테스트:
    - 코드에서 초기 URL 변경하여 다시 빌드:
      - https://www.naver.com
      - https://www.youtube.com
      - https://www.wikipedia.org
    - 각 사이트 정상 렌더링 확인
  - 예상 시간: 30분

- [ ] **Task 7-5**: 에러 처리 테스트
  - 테스트:
    - 존재하지 않는 URL 테스트 (http://invalid.invalid)
    - loadError 시그널 발생 확인
    - 에러 코드(-1), 에러 메시지, URL 로그 확인
  - 예상 시간: 15분

- [ ] **Task 7-6**: 타임아웃 테스트
  - 테스트:
    - 매우 느린 사이트 또는 네트워크 시뮬레이션
    - 30초 후 onLoadTimeout() 호출 확인
    - loadError(-2, "타임아웃 메시지", url) 확인
  - 예상 시간: 15분

- [ ] **Task 7-7**: 메모리 사용량 확인
  - 테스트:
    - 앱 실행 후 Activity Monitor (macOS)로 메모리 사용량 확인
    - WebView 로딩 전/후 메모리 증가 확인 (200MB 이하 목표)
  - 예상 시간: 15분

#### 완료 기준
- Google, Naver, YouTube, Wikipedia 모두 정상 렌더링 (4/4 성공)
- 로딩 이벤트 시그널 모두 정상 발생
- 에러 처리 및 타임아웃 정상 동작
- 메모리 사용량 200MB 이하

#### 예상 소요 시간
2시간

---

### Phase 8: 실제 디바이스 테스트
**목표**: LG 프로젝터 HU715QW에서 기능 검증 및 리모컨 조작 확인

#### 태스크
- [ ] **Task 8-1**: IPK 패키지 생성
  - 명령: `ares-package build/`
  - 확인: IPK 파일 생성 성공
  - 예상 시간: 15분

- [ ] **Task 8-2**: 프로젝터 설치 및 실행
  - 명령:
    - `ares-install --device projector {생성된ipk파일}`
    - `ares-launch --device projector com.jsong.webosbrowser.native`
  - 확인: 앱 정상 설치 및 실행
  - 예상 시간: 15분

- [ ] **Task 8-3**: 기본 렌더링 확인
  - 테스트:
    - Google 홈페이지 정상 표시
    - 프로젝터 화면에 웹 콘텐츠 렌더링 확인
  - 예상 시간: 15분

- [ ] **Task 8-4**: 리모컨 조작 테스트 (기본)
  - 테스트:
    - 십자키로 WebView 내 링크 간 포커스 이동 확인
    - Enter 키로 링크 클릭 확인 (새 페이지 로드)
    - 페이지 스크롤 확인 (십자키 상/하)
  - 주의: 상세한 키 이벤트 처리는 F-04, F-12에서 구현 예정
  - 예상 시간: 30분

- [ ] **Task 8-5**: 주요 사이트 렌더링 성공률 테스트
  - 테스트:
    - 코드에서 초기 URL 변경 후 재배포:
      - https://www.youtube.com
      - https://www.netflix.com (DRM 제약 확인)
      - https://www.naver.com
      - https://www.wikipedia.org
    - 각 사이트 렌더링 성공 여부 기록
  - 목표: 5개 중 최소 4개 성공 (80%, 목표는 95%)
  - 예상 시간: 1시간

- [ ] **Task 8-6**: 성능 측정
  - 테스트:
    - Google 홈페이지 로딩 시간: 콘솔 로그 확인 (2초 이내 목표)
    - Naver 홈페이지 로딩 시간: 5초 이내 목표
    - 로딩 이벤트 시그널 지연: 100ms 이내 (체감 확인)
  - 예상 시간: 30분

- [ ] **Task 8-7**: 장시간 사용 테스트
  - 테스트:
    - 여러 페이지 탐색 (10개 이상)
    - 1시간 연속 사용
    - 메모리 사용량 증가 확인 (10% 이내 목표)
    - 앱 크래시 여부 확인
  - 예상 시간: 1시간 30분

- [ ] **Task 8-8**: 로그 수집 및 분석
  - 명령: `ares-log --device projector -f com.jsong.webosbrowser.native`
  - 확인:
    - qDebug() 로그가 webOS 로그에 정상 출력
    - 에러 메시지 없음
  - 예상 시간: 15분

#### 완료 기준
- 프로젝터에서 앱 정상 실행
- 주요 사이트 렌더링 성공률 80% 이상 (5개 중 4개)
- 리모컨 기본 조작 (십자키, Enter) 동작
- 성능 기준 충족 (로딩 시간 5초 이내)
- 장시간 사용 시 안정성 확인 (크래시 없음)

#### 예상 소요 시간
4시간 30분

---

### Phase 9: 문서화 및 코드 리뷰
**목표**: 코드 주석 추가 및 문서 업데이트

#### 태스크
- [ ] **Task 9-1**: WebView.h Doxygen 주석 보완
  - 파일: `src/browser/WebView.h`
  - 작업:
    - 모든 공개 메서드에 @brief, @param, @return 추가
    - 시그널에 @brief 설명 추가
    - enum LoadingState 값에 설명 추가
  - 예상 시간: 30분

- [ ] **Task 9-2**: WebView.cpp 주석 추가
  - 파일: `src/browser/WebView.cpp`
  - 작업:
    - 복잡한 로직에 한국어 주석 추가
    - WebViewPrivate 클래스 멤버 변수 주석 추가
    - 상태 전이 로직 주석 보완
  - 예상 시간: 30분

- [ ] **Task 9-3**: BrowserWindow 주석 업데이트
  - 파일: `src/browser/BrowserWindow.h`, `BrowserWindow.cpp`
  - 작업:
    - WebView 관련 멤버 변수 및 메서드 주석 추가
    - TODO 주석 추가 (F-03, F-04, F-05 연계 부분)
  - 예상 시간: 20분

- [ ] **Task 9-4**: README.md 업데이트
  - 파일: `README.md`
  - 작업:
    - F-02 완료 상태 업데이트
    - WebView 클래스 소개 추가
    - 빌드 의존성 업데이트 (Qt WebEngineWidgets)
  - 예상 시간: 20분

- [ ] **Task 9-5**: CHANGELOG.md 작성
  - 파일: `CHANGELOG.md`
  - 작업:
    - F-02 웹뷰 통합 완료 항목 추가
    - 주요 변경 사항 요약 (WebView 클래스, BrowserWindow 통합)
  - 예상 시간: 15분

- [ ] **Task 9-6**: 코드 리뷰 (Self-review)
  - 작업:
    - 코딩 컨벤션 준수 확인 (CLAUDE.md 기준)
    - 스마트 포인터 사용 확인 (Raw 포인터 없음)
    - qDebug() 로그 일관성 확인
    - 네임스페이스 `webosbrowser` 사용 확인
  - 예상 시간: 30분

#### 완료 기준
- 모든 공개 API에 Doxygen 주석 추가
- 코드 내 한국어 주석으로 복잡한 로직 설명
- README.md 및 CHANGELOG.md 업데이트
- 코딩 컨벤션 100% 준수

#### 예상 소요 시간
2시간 25분

---

## 4. 태스크 의존성 다이어그램

```
Phase 1 (CMake 설정)
  │
  ├──▶ Phase 2 (WebView 스켈레톤)
  │         │
  │         ├──▶ Phase 3 (QWebEngineView 통합)
  │         │         │
  │         │         └──▶ Phase 4 (로딩 이벤트)
  │         │                   │
  │         │                   └──▶ Phase 5 (네비게이션 API)
  │         │                             │
  │         └─────────────────────────────┴──▶ Phase 6 (BrowserWindow 통합)
  │                                                   │
  │                                                   ├──▶ Phase 7 (로컬 테스트)
  │                                                   │         │
  │                                                   │         └──▶ Phase 8 (디바이스 테스트)
  │                                                   │                   │
  └───────────────────────────────────────────────────┴───────────────────┴──▶ Phase 9 (문서화)
```

### 의존성 설명
- **Phase 1 → Phase 2**: CMake 설정 완료 후 WebView 파일 생성 가능
- **Phase 2 → Phase 3**: 스켈레톤 완성 후 QWebEngineView 통합 가능
- **Phase 3 → Phase 4**: 기본 구조 완성 후 이벤트 처리 구현 가능
- **Phase 4 → Phase 5**: 로딩 이벤트 완성 후 네비게이션 API 독립적 구현 가능
- **Phase 5, Phase 3 → Phase 6**: WebView 클래스 완성 후 BrowserWindow 통합 가능
- **Phase 6 → Phase 7**: BrowserWindow 통합 후 로컬 테스트 가능
- **Phase 7 → Phase 8**: 로컬 검증 완료 후 디바이스 테스트
- **Phase 8 → Phase 9**: 모든 기능 검증 후 문서화

## 5. 병렬 실행 판단

### 판단: 병렬 실행 불가 (Sequential Only)

### 근거
1. **PIMPL 패턴 구현의 밀접한 결합**:
   - WebView.h 공개 인터페이스와 WebView.cpp의 WebViewPrivate 구현이 긴밀하게 연관됨
   - 인터페이스 변경 시 구현도 즉시 변경 필요 (순차적 반복 작업)

2. **QWebEngineView 통합의 복잡성**:
   - 시그널/슬롯 연결, 상태 관리, 타임아웃 처리가 하나의 로직으로 엮여 있음
   - 병렬 작업 시 시그널 연결 누락, 상태 충돌 등 통합 이슈 발생 가능

3. **BrowserWindow 통합 의존성**:
   - WebView 클래스 완성 전까지 BrowserWindow 수정 불가
   - WebView의 시그널 정의가 확정되어야 BrowserWindow 슬롯 구현 가능

4. **단일 Core 기능**:
   - WebView는 브라우저 앱의 핵심 컴포넌트로, 기능 분리가 불가능
   - Frontend/Backend 구분이 아닌, 단일 C++ 클래스의 순차적 구현

5. **개발 환경의 제약**:
   - 로컬 테스트 → 디바이스 테스트의 순차적 검증 필수
   - 실제 디바이스 테스트는 로컬 검증 완료 후에만 가능 (IPK 생성 비용)

### Agent Team 사용 권장 여부: No

### 결론
F-02 웹뷰 통합은 **단일 C++ 개발자(cpp-dev)가 순차적으로 구현해야 하는 Core 기능**입니다. 병렬 실행 시 오히려 통합 복잡도가 증가하고 디버깅 비용이 커지므로, 순차 파이프라인 방식으로 진행하는 것이 효율적입니다.

## 6. 예상 소요 시간 요약

| Phase | 설명 | 예상 시간 |
|-------|------|----------|
| Phase 1 | CMake 설정 | 30분 |
| Phase 2 | WebView 스켈레톤 | 2시간 20분 |
| Phase 3 | QWebEngineView 통합 | 2시간 15분 |
| Phase 4 | 로딩 이벤트 처리 | 2시간 |
| Phase 5 | 네비게이션 API | 30분 |
| Phase 6 | BrowserWindow 통합 | 2시간 |
| Phase 7 | 로컬 테스트 | 2시간 |
| Phase 8 | 디바이스 테스트 | 4시간 30분 |
| Phase 9 | 문서화 | 2시간 25분 |
| **총합** | | **18시간 30분** |

### 일정 환산
- **순수 작업 시간**: 18.5시간
- **버퍼 (20%)**: 3.7시간
- **총 예상 시간**: 약 22시간 (2.5~3일)

### 일정 분배 예시
- **Day 1**: Phase 1~5 완료 (7시간 35분 → 8시간)
- **Day 2**: Phase 6~7 완료 (4시간 → 디버깅 포함 6시간)
- **Day 3**: Phase 8~9 완료 (6시간 55분 → 8시간)

## 7. 파일 목록

### 새로 생성할 파일
- `src/browser/WebView.h` (공개 인터페이스)
- `src/browser/WebView.cpp` (구현 + WebViewPrivate)

### 수정할 파일
- `CMakeLists.txt` (Qt WebEngineWidgets 의존성 추가)
- `src/browser/BrowserWindow.h` (WebView 멤버 변수 및 슬롯 추가)
- `src/browser/BrowserWindow.cpp` (WebView 통합 및 이벤트 핸들러)
- `README.md` (F-02 완료 상태 업데이트)
- `CHANGELOG.md` (변경 사항 기록)

### 참조할 파일
- `docs/specs/webview-integration/requirements.md`
- `docs/specs/webview-integration/design.md`
- `CLAUDE.md` (코딩 컨벤션 확인)

## 8. 리스크 및 대응 방안

| 리스크 | 확률 | 영향도 | 대응 방안 |
|--------|------|--------|-----------|
| Qt WebEngineWidgets가 webOS에서 미지원 | 낮음 | 치명적 | webOS Native SDK 문서로 사전 확인. Qt for webOS 공식 지원 확인. 최악의 경우 Qt WebView로 fallback 또는 webOS Native WebView API 사용 |
| Chromium 메모리 과다 사용 (200MB 초과) | 중간 | 높음 | QWebEngineProfile로 캐시 크기 제한. 탭 개수 제한 (F-06에서 구현). 프로파일링 후 최적화 |
| 리모컨 포커스와 웹 페이지 포커스 충돌 | 높음 | 중간 | BrowserWindow에서 키 이벤트 중앙 관리 (F-04, F-12). 웹 페이지는 자체 tabindex 사용. 실제 디바이스 테스트로 조기 발견 |
| Netflix DRM 렌더링 실패 | 중간 | 중간 | Widevine DRM 지원 확인. 불가능한 경우 사용자에게 안내 메시지 표시 (F-13). 다른 주요 사이트로 대체 테스트 |
| 타임아웃 30초가 부적절 | 낮음 | 낮음 | 실제 네트워크 환경에서 테스트 후 조정 (20~40초 범위). 설정에서 변경 가능하도록 확장 (F-10) |
| QWebEngineView 프로세스 크래시 | 중간 | 높음 | F-13에서 renderProcessTerminated 시그널 처리. 크래시 시 재시작 메커니즘 구현 |
| 실제 디바이스에서만 발생하는 버그 | 높음 | 중간 | 조기 실제 디바이스 테스트 (Phase 8). qDebug() 로그를 webOS 로그에 통합하여 원격 디버깅 가능. ares-log로 실시간 로그 수집 |
| 로컬 테스트(macOS)와 디바이스(webOS) 동작 차이 | 중간 | 중간 | 가능한 조기에 디바이스 테스트. 플랫폼별 조건부 컴파일 (`#ifdef Q_OS_LINUX`) 준비. Qt 버전 일치 확인 (5.15) |

### 리스크 완화 전략
1. **사전 조사**: Phase 1에서 Qt WebEngineWidgets의 webOS 지원 문서 확인
2. **조기 디바이스 테스트**: Phase 7 로컬 테스트 완료 즉시 Phase 8 진행
3. **로그 메커니즘**: 모든 단계에서 qDebug() 로그 추가 (디버깅 용이성)
4. **버퍼 시간**: 예상 소요 시간에 20% 버퍼 포함
5. **Fallback 옵션**: Qt WebEngineView 실패 시 Qt WebView 또는 webOS Native API로 전환 가능

## 9. 완료 기준 (Acceptance Criteria)

### 기능 완료 기준
- [ ] WebView 클래스 구현 완료 (WebView.h, WebView.cpp)
- [ ] PIMPL 패턴으로 QWebEngineView 캡슐화
- [ ] load(), reload(), stop() 기본 API 동작
- [ ] loadStarted, loadProgress, loadFinished, titleChanged, urlChanged, loadError 시그널 모두 정상 발생
- [ ] 30초 타임아웃 메커니즘 동작
- [ ] goBack(), goForward() 네비게이션 API 동작
- [ ] BrowserWindow에 WebView 통합 완료
- [ ] 초기 URL (https://www.google.com) 자동 로드

### 테스트 완료 기준
- [ ] 로컬 테스트 (macOS): Google, Naver, YouTube, Wikipedia 모두 렌더링 성공
- [ ] 디바이스 테스트 (webOS): 주요 사이트 렌더링 성공률 80% 이상 (5개 중 4개)
- [ ] 로딩 시간: Google 2초 이내, 일반 페이지 5초 이내
- [ ] 메모리 사용량: 단일 WebView 200MB 이하
- [ ] 에러 처리: 네트워크 에러, 타임아웃 에러 정상 감지
- [ ] 리모컨 조작: 십자키 포커스 이동, Enter 키 링크 클릭 동작

### 문서 완료 기준
- [ ] WebView.h 모든 공개 API에 Doxygen 주석 추가
- [ ] WebView.cpp 복잡한 로직에 한국어 주석 추가
- [ ] README.md 및 CHANGELOG.md 업데이트
- [ ] 코딩 컨벤션 100% 준수 (CLAUDE.md 기준)

### 통합 완료 기준
- [ ] CMake 빌드 성공 (Qt WebEngineWidgets 링크)
- [ ] IPK 패키지 생성 성공
- [ ] 프로젝터 설치 및 실행 성공
- [ ] 장시간 사용 (1시간) 안정성 확인 (크래시 없음)

## 10. 다음 단계 (Next Steps)

F-02 완료 후 다음 기능으로 진행할 수 있는 우선순위:

### 우선순위 1: F-03 (URL 입력 UI)
- **이유**: WebView::load(QUrl) API가 준비되어 즉시 연동 가능
- **의존성**: F-02 완료 필요
- **예상 기간**: 1~2일

### 우선순위 2: F-04 (페이지 탐색 컨트롤)
- **이유**: WebView::goBack(), goForward() API가 준비되어 UI만 추가하면 됨
- **의존성**: F-02 완료 필요
- **예상 기간**: 1~1.5일

### 우선순위 3: F-05 (로딩 인디케이터)
- **이유**: WebView::loadProgress(int) 시그널이 준비되어 프로그레스바만 구현하면 됨
- **의존성**: F-02 완료 필요
- **예상 기간**: 0.5~1일

### 병렬 작업 가능 (F-02 완료 후)
F-02 완료 후 F-03, F-04, F-05는 서로 독립적이므로 병렬 작업 가능:
- **Team 구성**: cpp-dev 2~3명
- **작업 분배**:
  - Developer 1: F-03 (URL 입력 UI)
  - Developer 2: F-04 (페이지 탐색 컨트롤)
  - Developer 3: F-05 (로딩 인디케이터)

## 11. 참고 자료

### Qt 공식 문서
- Qt WebEngineView: https://doc.qt.io/qt-5/qwebengineview.html
- Qt Signals and Slots: https://doc.qt.io/qt-5/signalsandslots.html
- Qt Smart Pointers: https://doc.qt.io/qt-5/smart-pointers.html
- PIMPL 패턴: https://en.cppreference.com/w/cpp/language/pimpl

### webOS 공식 문서
- webOS Native API: https://webostv.developer.lge.com/develop/native-apps/native-app-overview
- Qt for webOS: https://webostv.developer.lge.com/develop/native-apps/webos-qt-overview
- webOS CLI 도구: https://webostv.developer.lge.com/develop/tools/cli/introduction

### 프로젝트 문서
- 요구사항: docs/specs/webview-integration/requirements.md
- 기술 설계: docs/specs/webview-integration/design.md
- CLAUDE.md: /Users/jsong/dev/jsong1230-github/webosbrowser-native/CLAUDE.md
- PRD: docs/project/prd.md

## 12. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초기 구현 계획서 작성 (Native App) | product-manager |
| 2026-02-14 | design.md의 9단계 Phase를 상세 태스크로 분해 | product-manager |
| 2026-02-14 | 병렬 실행 불가 판단 및 순차 실행 순서 확정 | product-manager |
