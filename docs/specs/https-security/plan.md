# F-14: HTTPS 보안 표시 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/https-security/requirements.md
- 기술 설계서: docs/specs/https-security/design.md

---

## 2. 구현 Phase

### Phase 1: 보안 등급 분류 유틸리티 구현
**목적**: URL 프로토콜 기반 보안 등급 분류 로직 구현

**태스크**:
- [ ] Task 1-1: SecurityLevel enum 정의 → cpp-dev
  - 파일: `src/utils/SecurityClassifier.h`
  - 내용: Secure, Insecure, Local, Unknown enum 정의
  - 완료 기준: enum 선언 완료, 네임스페이스 webosbrowser 적용

- [ ] Task 1-2: SecurityClassifier 클래스 헤더 작성 → cpp-dev
  - 파일: `src/utils/SecurityClassifier.h`
  - 내용: 정적 메서드 선언 (classify, isLocalAddress, securityLevelToString, securityLevelToTooltip, securityLevelToIconPath)
  - 완료 기준: 헤더 파일 작성, doxygen 주석 추가

- [ ] Task 1-3: SecurityClassifier::classify() 구현 → cpp-dev
  - 파일: `src/utils/SecurityClassifier.cpp`
  - 내용: QUrl::scheme() 기반 보안 등급 분류 로직
  - 로직:
    - https → Secure
    - http + localhost/사설IP → Local
    - http + 공인IP → Insecure
    - file → Local
    - 빈 URL, about:blank → Unknown
  - 완료 기준: 모든 프로토콜 분류 로직 구현, 단위 테스트 통과

- [ ] Task 1-4: SecurityClassifier::isLocalAddress() 구현 → cpp-dev
  - 파일: `src/utils/SecurityClassifier.cpp`
  - 내용: 로컬 주소 판별 로직 (localhost, 127.0.0.1, ::1, 사설 IP)
  - 정규표현식 패턴 (RFC 1918):
    - 10.0.0.0/8: `^10\.\d{1,3}\.\d{1,3}\.\d{1,3}$`
    - 172.16.0.0/12: `^172\.(1[6-9]|2\d|3[01])\.\d{1,3}\.\d{1,3}$`
    - 192.168.0.0/16: `^192\.168\.\d{1,3}\.\d{1,3}$`
  - 완료 기준: 모든 사설 IP 범위 정확히 판별, 단위 테스트 통과

- [ ] Task 1-5: 유틸리티 메서드 구현 → cpp-dev
  - 파일: `src/utils/SecurityClassifier.cpp`
  - 내용: securityLevelToString(), securityLevelToTooltip(), securityLevelToIconPath() 구현
  - 완료 기준: enum → 문자열 변환 로직 완료

**예상 산출물**:
- `src/utils/SecurityClassifier.h` (신규)
- `src/utils/SecurityClassifier.cpp` (신규)

**예상 소요 시간**: 1일 (8시간)
- Task 1-1, 1-2: 1시간
- Task 1-3: 3시간
- Task 1-4: 3시간
- Task 1-5: 1시간

**완료 기준**:
- SecurityClassifier::classify() 단위 테스트 통과 (AC-6)
- SecurityClassifier::isLocalAddress() 단위 테스트 통과 (TS-9)
- 컴파일 오류 없음

---

### Phase 2: SecurityIndicator 위젯 구현
**목적**: 보안 상태 아이콘 표시 UI 컴포넌트 구현

**태스크**:
- [ ] Task 2-1: SecurityIndicator 클래스 헤더 작성 → cpp-dev
  - 파일: `src/ui/SecurityIndicator.h`
  - 내용: QLabel 기반 위젯 클래스 선언
  - 시그널/슬롯: clicked() 시그널 선언 (향후 FR-5용)
  - 멤버 변수: currentLevel_, secureIcon_, insecureIcon_, localIcon_ (QPixmap 캐시)
  - 완료 기준: 헤더 작성, Q_OBJECT 매크로, doxygen 주석

- [ ] Task 2-2: SecurityIndicator::setSecurityLevel() 구현 → cpp-dev
  - 파일: `src/ui/SecurityIndicator.cpp`
  - 내용: SecurityLevel에 따라 아이콘(QPixmap)과 툴팁(QToolTip) 갱신
  - 로직:
    - Secure → secureIcon_, "보안 연결"
    - Insecure → insecureIcon_, "안전하지 않음"
    - Local → localIcon_, "로컬 연결"
    - Unknown → clear(), "" (아이콘 숨김)
  - 완료 기준: 모든 SecurityLevel 처리, 아이콘 변경 확인

- [ ] Task 2-3: 아이콘 리소스 추가 → cpp-dev
  - 파일: `resources/icons/lock_secure.png`, `lock_insecure.png`, `info.png`
  - 내용: 32x32px PNG 아이콘 (녹색 자물쇠, 노란색 경고, 파란색 정보)
  - Qt 리소스 파일 (resources.qrc) 등록
  - 완료 기준: 아이콘 파일 생성, QPixmap 로드 성공

- [ ] Task 2-4: 아이콘 캐시 로드 로직 구현 → cpp-dev
  - 파일: `src/ui/SecurityIndicator.cpp`
  - 내용: 생성자에서 QPixmap 사전 로드 (loadIconCache() 메서드)
  - 로직: QPixmap(":/icons/lock_secure.png") 등 리소스 로드
  - 완료 기준: 아이콘 캐시 로드 성공, 로드 실패 시 qWarning 로그 출력

- [ ] Task 2-5: 포커스 이벤트 처리 구현 → cpp-dev
  - 파일: `src/ui/SecurityIndicator.cpp`
  - 내용: focusInEvent 오버라이드, 포커스 시 툴팁 자동 표시 (QToolTip::showText)
  - 완료 기준: 리모컨 포커스 시 툴팁 표시 확인

- [ ] Task 2-6: QSS 스타일 적용 → cpp-dev
  - 파일: `src/ui/SecurityIndicator.cpp`
  - 내용: 포커스 테두리 (3px solid #00aaff), 아이콘 여백 설정
  - 완료 기준: 포커스 시 테두리 표시, 아이콘 크기 고정 (32x32px)

**예상 산출물**:
- `src/ui/SecurityIndicator.h` (신규)
- `src/ui/SecurityIndicator.cpp` (신규)
- `resources/icons/lock_secure.png` (신규)
- `resources/icons/lock_insecure.png` (신규)
- `resources/icons/info.png` (신규)
- `resources.qrc` (수정)

**예상 소요 시간**: 1일 (8시간)
- Task 2-1: 1시간
- Task 2-2: 2시간
- Task 2-3: 2시간
- Task 2-4: 1시간
- Task 2-5: 1시간
- Task 2-6: 1시간

**완료 기준**:
- SecurityIndicator 위젯 생성 성공
- setSecurityLevel() 호출 시 아이콘 정상 표시 (TS-1, TS-3)
- 리모컨 포커스 시 툴팁 표시 (AC-7)

**의존성**: Phase 1 완료 필요 (SecurityLevel enum)

---

### Phase 3: URLBar 통합
**목적**: URLBar에 SecurityIndicator 통합, 보안 아이콘 실시간 업데이트

**태스크**:
- [ ] Task 3-1: URLBar 클래스 헤더 확장 → cpp-dev
  - 파일: `src/ui/URLBar.h`
  - 내용: SecurityIndicator* 멤버 변수 추가, updateSecurityIndicator(SecurityLevel level) 메서드 선언
  - 완료 기준: 헤더 수정, 컴파일 오류 없음

- [ ] Task 3-2: URLBar 레이아웃에 SecurityIndicator 추가 → cpp-dev
  - 파일: `src/ui/URLBar.cpp`
  - 내용: inputLayout_ (QHBoxLayout)의 첫 번째 위젯으로 SecurityIndicator 추가
  - 로직:
    ```cpp
    securityIndicator_ = new SecurityIndicator(this);
    inputLayout_->insertWidget(0, securityIndicator_);  // 좌측 배치
    ```
  - 완료 기준: SecurityIndicator가 URLBar 좌측에 표시, 레이아웃 시프트 없음

- [ ] Task 3-3: URLBar::updateSecurityIndicator() 구현 → cpp-dev
  - 파일: `src/ui/URLBar.cpp`
  - 내용: securityIndicator_->setSecurityLevel(level) 호출
  - 완료 기준: 메서드 구현, BrowserWindow에서 호출 가능

**예상 산출물**:
- `src/ui/URLBar.h` (수정)
- `src/ui/URLBar.cpp` (수정)

**예상 소요 시간**: 0.5일 (4시간)
- Task 3-1: 1시간
- Task 3-2: 2시간
- Task 3-3: 1시간

**완료 기준**:
- URLBar에 SecurityIndicator 정상 표시
- updateSecurityIndicator() 호출 시 아이콘 갱신 확인
- 기존 URLBar 기능 (URL 입력, F-03) 정상 동작

**의존성**: Phase 2 완료 필요 (SecurityIndicator 위젯)

---

### Phase 4: BrowserWindow 통합 및 경고 다이얼로그
**목적**: URL 변경 시 보안 아이콘 자동 업데이트, HTTP 경고 다이얼로그 구현

**태스크**:
- [ ] Task 4-1: BrowserWindow 클래스 헤더 확장 → cpp-dev
  - 파일: `src/browser/BrowserWindow.h`
  - 내용: 멤버 변수 추가
    - `QSet<QString> ignoredDomains_;` (경고 무시 도메인 목록)
    - `QTimer *warningTimer_;` (500ms 디바운싱 타이머)
    - `QUrl pendingUrl_;` (경고 대기 중인 URL)
  - 메서드 선언:
    - `void checkHttpWarning(const QUrl &url);`
    - `bool showSecurityWarningDialog(const QUrl &url);`
    - `void onWarningTimerTimeout();` (슬롯)
  - 완료 기준: 헤더 수정, 컴파일 오류 없음

- [ ] Task 4-2: BrowserWindow::onUrlChanged() 확장 → cpp-dev
  - 파일: `src/browser/BrowserWindow.cpp`
  - 내용: 기존 슬롯에 보안 아이콘 업데이트 로직 추가
  - 로직:
    ```cpp
    void BrowserWindow::onUrlChanged(const QUrl &url) {
        // 기존 로직 유지 (URLBar URL 갱신 등)

        // [신규] 보안 등급 분류
        SecurityLevel level = SecurityClassifier::classify(url);

        // [신규] URLBar 보안 아이콘 업데이트
        urlBar_->updateSecurityIndicator(level);

        // [신규] HTTP 경고 체크
        checkHttpWarning(url);
    }
    ```
  - 완료 기준: onUrlChanged 슬롯 확장, 기존 로직 유지

- [ ] Task 4-3: BrowserWindow::checkHttpWarning() 구현 → cpp-dev
  - 파일: `src/browser/BrowserWindow.cpp`
  - 내용: HTTP 경고 필요 여부 체크 및 타이머 시작
  - 로직:
    - SecurityLevel != Insecure → 타이머 중지, 리턴
    - ignoredDomains_.contains(host) → 타이머 중지, 리턴
    - 500ms 디바운싱: pendingUrl_ = url, warningTimer_->start(500)
  - 완료 기준: 경고 필요 시 타이머 시작, 불필요 시 타이머 중지

- [ ] Task 4-4: BrowserWindow::showSecurityWarningDialog() 구현 → cpp-dev
  - 파일: `src/browser/BrowserWindow.cpp`
  - 내용: QMessageBox 경고 다이얼로그 표시
  - 다이얼로그 구성:
    - 타이틀: "안전하지 않은 사이트"
    - 메시지: "이 사이트는 보안 연결을 사용하지 않습니다."
    - 설명: "개인 정보(비밀번호, 신용카드 번호 등)를 입력하지 마세요."
    - 버튼: "계속 진행" (AcceptRole), "돌아가기" (RejectRole)
    - 체크박스: "이 세션 동안 이 사이트에 대해 다시 표시하지 않기"
  - 로직:
    - "계속 진행" + 체크박스 선택 → ignoredDomains_.insert(host)
    - "돌아가기" → webView_->goBack()
    - ignoredDomains_.size() > 100 → 가장 오래된 항목 제거
  - 완료 기준: 다이얼로그 표시, 버튼 동작 확인, 경고 무시 옵션 동작 확인

- [ ] Task 4-5: onWarningTimerTimeout() 슬롯 구현 → cpp-dev
  - 파일: `src/browser/BrowserWindow.cpp`
  - 내용: 타이머 타임아웃 시 경고 다이얼로그 표시
  - 로직:
    ```cpp
    void BrowserWindow::onWarningTimerTimeout() {
        if (!pendingUrl_.isEmpty()) {
            showSecurityWarningDialog(pendingUrl_);
            pendingUrl_.clear();
        }
    }
    ```
  - 완료 기준: 타이머 타임아웃 500ms 후 경고 다이얼로그 표시

- [ ] Task 4-6: 타이머 초기화 → cpp-dev
  - 파일: `src/browser/BrowserWindow.cpp`
  - 내용: 생성자에서 warningTimer_ 초기화
  - 로직:
    ```cpp
    warningTimer_ = new QTimer(this);
    warningTimer_->setSingleShot(true);
    connect(warningTimer_, &QTimer::timeout, this, &BrowserWindow::onWarningTimerTimeout);
    ```
  - 완료 기준: 타이머 생성, timeout 시그널 연결

**예상 산출물**:
- `src/browser/BrowserWindow.h` (수정)
- `src/browser/BrowserWindow.cpp` (수정)

**예상 소요 시간**: 1일 (8시간)
- Task 4-1: 1시간
- Task 4-2: 1시간
- Task 4-3: 2시간
- Task 4-4: 3시간
- Task 4-5: 0.5시간
- Task 4-6: 0.5시간

**완료 기준**:
- URL 변경 시 보안 아이콘 500ms 이내 업데이트 (AC-4, FR-4)
- HTTP 사이트 접속 시 경고 다이얼로그 표시 (AC-2, TS-2)
- 경고 무시 옵션 동작 (AC-5, TS-6)
- HTTP → HTTPS 리다이렉트 시 경고 숨김 (TS-7)
- localhost 접속 시 경고 없음 (AC-3, TS-3)

**의존성**: Phase 3 완료 필요 (URLBar::updateSecurityIndicator)

---

### Phase 5: 테스트 및 검증
**목적**: 단위 테스트, 통합 테스트, UI 테스트 실행

**태스크**:
- [ ] Task 5-1: SecurityClassifier 단위 테스트 작성 → test-runner
  - 파일: `tests/unit/SecurityClassifierTest.cpp`
  - 내용: Google Test 기반 단위 테스트
  - 테스트 케이스:
    - classify() 메서드 (TS-8, AC-6)
    - isLocalAddress() 메서드 (TS-9)
  - 완료 기준: 모든 테스트 통과

- [ ] Task 5-2: SecurityIndicator 통합 테스트 작성 → test-runner
  - 파일: `tests/integration/SecurityIndicatorTest.cpp`
  - 내용: Qt Test 기반 통합 테스트
  - 테스트 케이스:
    - setSecurityLevel() 호출 시 아이콘 변경 확인
    - 포커스 시 툴팁 표시 확인
  - 완료 기준: 모든 테스트 통과

- [ ] Task 5-3: UI 테스트 (수동) → test-runner
  - 내용: webOS 프로젝터에서 수동 테스트 실행
  - 테스트 시나리오: TS-1 ~ TS-7
    - HTTPS 사이트 보안 아이콘 표시 (TS-1)
    - HTTP 사이트 경고 다이얼로그 (TS-2)
    - localhost 예외 처리 (TS-3)
    - HTTPS → HTTP 이동 시 경고 (TS-4)
    - 페이지 내 링크 클릭 시 아이콘 업데이트 (TS-5)
    - 경고 무시 옵션 (TS-6)
    - HTTP → HTTPS 자동 리다이렉트 시 경고 숨김 (TS-7)
  - 완료 기준: 모든 테스트 시나리오 통과

- [ ] Task 5-4: 수용 기준 검증 → test-runner
  - 내용: 요구사항 분석서의 수용 기준 (AC-1 ~ AC-8) 검증
  - 완료 기준: 모든 수용 기준 충족

- [ ] Task 5-5: 회귀 테스트 → test-runner
  - 내용: 기존 기능 (F-02, F-03, F-04) 정상 동작 확인
  - 테스트:
    - F-02 (WebView 통합): URL 로드 정상 동작
    - F-03 (URL 입력 UI): URL 입력 및 자동완성 정상 동작
    - F-04 (네비게이션): 뒤로/앞으로 버튼 정상 동작
  - 완료 기준: 회귀 없음

**예상 산출물**:
- `tests/unit/SecurityClassifierTest.cpp` (신규)
- `tests/integration/SecurityIndicatorTest.cpp` (신규)
- 테스트 리포트 (통과/실패 결과)

**예상 소요 시간**: 1일 (8시간)
- Task 5-1: 3시간
- Task 5-2: 2시간
- Task 5-3: 2시간
- Task 5-4: 0.5시간
- Task 5-5: 0.5시간

**완료 기준**:
- 모든 단위 테스트 통과
- 모든 통합 테스트 통과
- 모든 UI 테스트 시나리오 통과
- 모든 수용 기준 충족
- 회귀 없음

**의존성**: Phase 4 완료 필요 (모든 구현 완료)

---

### Phase 6: 코드 리뷰 및 문서화
**목적**: 코드 품질 검증, 문서 업데이트

**태스크**:
- [ ] Task 6-1: 코드 리뷰 → code-reviewer
  - 내용: C++ 코딩 컨벤션 준수 확인
    - 주석: 한국어
    - 변수명: 영어 camelCase
    - 클래스명: PascalCase
    - 스마트 포인터 사용 확인 (raw 포인터 금지, 단 Qt 부모-자식은 예외)
    - Qt 시그널/슬롯 신규 문법 사용 확인
  - 완료 기준: 코드 리뷰 승인

- [ ] Task 6-2: 설계 문서 최종 검증 → code-reviewer
  - 내용: requirements.md, design.md와 실제 구현 일치 여부 확인
  - 완료 기준: 문서-코드 일치성 확인

- [ ] Task 6-3: CHANGELOG.md 업데이트 → doc-writer
  - 내용: F-14 완료 항목 추가
  - 완료 기준: CHANGELOG.md 업데이트

- [ ] Task 6-4: 개발 로그 작성 → doc-writer
  - 파일: `docs/dev-log.md`
  - 내용: F-14 구현 과정 요약, 리스크 대응, 의사결정 기록
  - 완료 기준: dev-log.md 업데이트

**예상 산출물**:
- CHANGELOG.md (수정)
- docs/dev-log.md (수정)

**예상 소요 시간**: 0.5일 (4시간)
- Task 6-1: 2시간
- Task 6-2: 1시간
- Task 6-3: 0.5시간
- Task 6-4: 0.5시간

**완료 기준**:
- 코드 리뷰 승인
- 문서-코드 일치성 확인
- CHANGELOG.md, dev-log.md 업데이트

**의존성**: Phase 5 완료 필요 (테스트 완료)

---

## 3. 태스크 의존성

```
Phase 1 (보안 등급 분류)
 │
 ├─ Task 1-1 (SecurityLevel enum) ──┐
 ├─ Task 1-2 (헤더 작성)           │
 ├─ Task 1-3 (classify 구현)        ├─▶ Phase 2 (SecurityIndicator)
 ├─ Task 1-4 (isLocalAddress 구현) │    │
 └─ Task 1-5 (유틸리티 메서드)      ┘    │
                                        │
Phase 2 (SecurityIndicator 위젯)         │
 │                                       │
 ├─ Task 2-1 (헤더 작성)                 │
 ├─ Task 2-2 (setSecurityLevel 구현)    │
 ├─ Task 2-3 (아이콘 리소스)             ├─▶ Phase 3 (URLBar 통합)
 ├─ Task 2-4 (아이콘 캐시 로드)          │    │
 ├─ Task 2-5 (포커스 이벤트)             │    │
 └─ Task 2-6 (QSS 스타일)               ┘    │
                                             │
Phase 3 (URLBar 통합)                         │
 │                                            │
 ├─ Task 3-1 (헤더 확장)                     │
 ├─ Task 3-2 (레이아웃 추가)                 ├─▶ Phase 4 (BrowserWindow 통합)
 └─ Task 3-3 (updateSecurityIndicator 구현)  │    │
                                             │    │
Phase 4 (BrowserWindow 통합 및 경고)          │    │
 │                                            │    │
 ├─ Task 4-1 (헤더 확장)                      │    │
 ├─ Task 4-2 (onUrlChanged 확장)             │    │
 ├─ Task 4-3 (checkHttpWarning 구현)         ├─▶ Phase 5 (테스트)
 ├─ Task 4-4 (showSecurityWarningDialog)     │    │
 ├─ Task 4-5 (onWarningTimerTimeout)         │    │
 └─ Task 4-6 (타이머 초기화)                  ┘    │
                                                  │
Phase 5 (테스트 및 검증)                          │
 │                                                │
 ├─ Task 5-1 (단위 테스트)                        │
 ├─ Task 5-2 (통합 테스트)                        ├─▶ Phase 6 (리뷰)
 ├─ Task 5-3 (UI 테스트)                         │
 ├─ Task 5-4 (수용 기준 검증)                    │
 └─ Task 5-5 (회귀 테스트)                       ┘
                                                  │
Phase 6 (코드 리뷰 및 문서화)                      │
 │                                                │
 ├─ Task 6-1 (코드 리뷰)                          │
 ├─ Task 6-2 (문서 검증)                          │
 ├─ Task 6-3 (CHANGELOG 업데이트)                 │
 └─ Task 6-4 (dev-log 작성)                      ┘
```

---

## 4. 병렬 실행 판단

### 4.1. PG-3 병렬 배치 종합 분석

PG-3 병렬 그룹:
- **F-12 (다운로드 관리)**: DownloadManager + DownloadPanel
- **F-13 (리모컨 단축키)**: KeyboardHandler
- **F-14 (HTTPS 보안 표시)**: SecurityIndicator + URLBar (확장)

#### 병렬 실행 가능성 분석

| 기능 | 주요 컴포넌트 | 공유 컴포넌트 | 충돌 영역 |
|------|-------------|-------------|----------|
| F-12 | DownloadManager, DownloadPanel | WebView (다운로드 시그널), BrowserWindow (패널 통합) | **중간** (BrowserWindow 레이아웃 수정) |
| F-13 | KeyboardHandler | BrowserWindow (키 이벤트 처리), TabManager (탭 전환) | **낮음** (독립적인 서비스) |
| F-14 | SecurityIndicator, URLBar (확장) | URLBar (레이아웃 수정), BrowserWindow (onUrlChanged 확장) | **중간** (URLBar, BrowserWindow 수정) |

#### 충돌 위험 평가

1. **F-12 vs F-13**:
   - **충돌 없음**: F-12는 DownloadPanel, F-13은 KeyboardHandler (독립적)
   - **병렬 가능**: 동시 작업 가능

2. **F-12 vs F-14**:
   - **충돌 낮음**: F-12는 DownloadPanel, F-14는 URLBar/SecurityIndicator
   - **공통 수정**: BrowserWindow (onUrlChanged vs 다운로드 이벤트 처리)
   - **병렬 가능**: BrowserWindow 수정 영역이 다름 (onUrlChanged vs 다운로드 슬롯)

3. **F-13 vs F-14**:
   - **충돌 없음**: F-13은 KeyboardHandler, F-14는 SecurityIndicator
   - **병렬 가능**: 동시 작업 가능

4. **PG-3 전체 병렬 실행**:
   - **가능**: F-12, F-13, F-14 모두 병렬 실행 가능
   - **단, BrowserWindow 충돌 주의**: F-12와 F-14가 BrowserWindow를 동시 수정하므로 병합 시 충돌 발생 가능
   - **권장**: F-12와 F-14는 순차 실행 또는 Agent Team으로 조율

#### Agent Team 사용 권장 여부

**권장: Yes (조건부)**

**권장 이유**:
1. **병렬 효율성 높음**:
   - Phase 1-2 (SecurityClassifier, SecurityIndicator)는 F-12, F-13과 100% 독립적
   - F-13 (KeyboardHandler)도 완전 독립적
   - 3개 기능의 Phase 1-2를 병렬 실행 시 약 2일 절약 가능

2. **BrowserWindow 충돌 관리 가능**:
   - F-12와 F-14의 BrowserWindow 수정은 Phase 4에서 발생
   - Agent Team 리더가 Phase 4 진입 시점을 조율하여 순차 병합 가능
   - 예: F-14 Phase 4 → F-12 Phase 4 (또는 반대)

3. **PG-3 마일스톤 일정 단축**:
   - 순차 실행: F-12 (5일) + F-13 (3일) + F-14 (4.5일) = 12.5일
   - 병렬 실행: Max(5일, 3일, 4.5일) + 병합 조율 (1일) = 6일
   - **약 6.5일 절약** (52% 단축)

**주의사항**:
- **Agent Team 리더**가 F-12, F-14의 BrowserWindow 수정 시점을 조율해야 함
- **Phase 3-4 진입 시**: F-14 Phase 3 완료 후 F-12 Phase 4 시작 (또는 반대)
- **병합 순서**: F-13 (독립적) → F-14 (URLBar) → F-12 (DownloadPanel)

#### 최종 권장 사항

**Agent Team 사용 권장**:
- **Phase 1-2**: F-12, F-13, F-14 병렬 실행 (완전 독립적)
- **Phase 3-4**: Agent Team 리더가 순차 조율
  - F-13 Phase 3-4 (독립적) 계속 병렬 실행
  - F-14 Phase 3 완료 → F-12 Phase 4 시작
  - F-14 Phase 4 완료 → F-12 Phase 4 완료 대기 → 병합
- **Phase 5-6**: 각 기능별 독립 테스트 후 통합 테스트

**예상 일정**:
- **병렬 Phase 1-2**: 2일 (F-12, F-13, F-14 동시 실행)
- **조율 Phase 3-4**: 3일 (F-14 Phase 3-4 → F-12 Phase 4, F-13 병렬)
- **병렬 Phase 5-6**: 1.5일 (각 기능별 테스트)
- **통합 병합 및 테스트**: 0.5일
- **총 예상 기간**: 7일 (순차 실행 대비 5.5일 절약)

---

## 5. 리스크

| 리스크 | 영향도 | 발생 확률 | 대응 방안 |
|--------|--------|----------|-----------|
| URLBar 레이아웃 시프트로 인한 UX 저하 | Medium | Low | SecurityIndicator 영역 고정 너비 확보 (setFixedSize), inputLayout_에 첫 번째 위젯 배치 |
| HTTP → HTTPS 리다이렉트 시 경고 깜박임 | Medium | Medium | 500ms 디바운싱 (QTimer::singleShot), 최종 URL이 HTTPS면 타이머 자동 취소 |
| 사설 IP 정규표현식 오류 (범위 외 IP를 사설로 판단) | Medium | Low | RFC 1918 표준 정확히 구현, 단위 테스트 (TS-9)로 모든 케이스 검증 |
| ignoredDomains_ 메모리 누수 (100개 초과) | Low | Low | QSet::size() 체크 후 100개 초과 시 가장 오래된 항목 제거 (QSet::erase(begin())) |
| 아이콘 리소스 로드 실패 (파일 없음) | Low | Low | QPixmap::isNull() 체크 후 qWarning 로그 출력, 빈 QLabel 표시 (툴팁만 표시) |
| Qt WebEngineView 인증서 정보 접근 제약 | High | 확정 | FR-5 (상세 정보 다이얼로그)는 M3 범위 외로 처리, 프로토콜 기반 분류에 집중 |
| BrowserWindow 충돌 (F-12와 F-14 동시 수정) | High | Medium | Agent Team 리더가 Phase 4 진입 시점 조율, 순차 병합 |
| URLBar 충돌 (F-09와 F-14 동시 수정) | Medium | Low | F-09 완료 후 F-14 시작 (의존성 체인), PG-2 완료 후 PG-3 시작 |

---

## 6. 완료 체크리스트

### 기능 요구사항 (FR)
- [ ] FR-1: HTTPS 보안 아이콘 표시 (AC-1, TS-1)
- [ ] FR-2: 비보안 사이트 접속 경고 다이얼로그 (AC-2, TS-2)
- [ ] FR-3: URL 프로토콜 자동 감지 및 분류 (AC-6, TS-8)
- [ ] FR-4: 보안 상태 실시간 업데이트 (AC-4, TS-5)

### 수용 기준 (AC)
- [ ] AC-1: HTTPS 사이트 아이콘 표시
- [ ] AC-2: HTTP 사이트 경고 다이얼로그
- [ ] AC-3: localhost 예외 처리
- [ ] AC-4: 페이지 이동 시 아이콘 업데이트 (500ms 이내)
- [ ] AC-5: 경고 무시 옵션 (세션 단위)
- [ ] AC-6: URL 프로토콜 자동 분류 (7개 케이스)
- [ ] AC-7: 아이콘 툴팁 표시
- [ ] AC-8: 리모컨 키 매핑 (향후 FR-5 구현 시)

### 테스트 시나리오 (TS)
- [ ] TS-1: HTTPS 사이트 보안 아이콘 표시
- [ ] TS-2: HTTP 사이트 경고 다이얼로그
- [ ] TS-3: localhost 예외 처리
- [ ] TS-4: HTTPS → HTTP 이동 시 경고
- [ ] TS-5: 페이지 내 링크 클릭 시 아이콘 업데이트
- [ ] TS-6: 경고 무시 옵션 (세션 단위)
- [ ] TS-7: HTTP → HTTPS 자동 리다이렉트 시 경고 숨김
- [ ] TS-8: URL 분류 유틸리티 단위 테스트
- [ ] TS-9: 사설 IP 패턴 정확도 테스트

### 비기능 요구사항 (NFR)
- [ ] NFR-1: 성능 (URL 변경 후 500ms 이내 아이콘 업데이트)
- [ ] NFR-2: 사용성 (아이콘 가독성 32x32px, 툴팁 16px)
- [ ] NFR-3: 보안 (경고 무시 옵션 세션 단위만, 영구 저장 금지)
- [ ] NFR-4: 확장성 (SecurityLevel enum 추가 가능, 아이콘 테마 지원 준비)

### 코드 품질
- [ ] C++ 코딩 컨벤션 준수 (주석 한국어, 변수명 영어 camelCase)
- [ ] 스마트 포인터 사용 (raw 포인터 금지, Qt 부모-자식 예외)
- [ ] Qt 시그널/슬롯 신규 문법 사용
- [ ] 모든 public 메서드 doxygen 주석 작성
- [ ] Google Test, Qt Test 통과

### 문서
- [ ] CHANGELOG.md 업데이트
- [ ] docs/dev-log.md 업데이트
- [ ] 설계서-코드 일치성 확인

---

## 7. 총 예상 소요 시간

| Phase | 예상 시간 | 누적 시간 |
|-------|----------|----------|
| Phase 1: 보안 등급 분류 | 1일 (8시간) | 1일 |
| Phase 2: SecurityIndicator 위젯 | 1일 (8시간) | 2일 |
| Phase 3: URLBar 통합 | 0.5일 (4시간) | 2.5일 |
| Phase 4: BrowserWindow 통합 및 경고 | 1일 (8시간) | 3.5일 |
| Phase 5: 테스트 및 검증 | 1일 (8시간) | 4.5일 |
| Phase 6: 코드 리뷰 및 문서화 | 0.5일 (4시간) | 5일 |

**총 예상 기간**: **5일** (순차 실행 기준)

**Agent Team 병렬 실행 시**: **2일** (Phase 1-2 병렬) + **1.5일** (Phase 3-4 조율) + **1.5일** (Phase 5-6) = **5일** (개인 작업 기준)

**PG-3 전체 병렬 실행 시**: **7일** (F-12, F-13, F-14 동시 실행 + 통합)

---

## 8. 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|-----------|--------|
| 2026-02-14 | 1.0 | 최초 작성 | product-manager |

---

## 9. 승인

| 역할 | 이름 | 승인 여부 | 날짜 |
|------|------|-----------|------|
| Product Manager | - | 승인 | 2026-02-14 |
| Architect | - | 대기 | - |
| C++ Developer | - | 대기 | - |
| Test Runner | - | 대기 | - |
