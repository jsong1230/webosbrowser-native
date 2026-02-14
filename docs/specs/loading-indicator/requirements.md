# 로딩 인디케이터 — 요구사항 분석서 (Native App)

## 1. 개요

### 기능명
F-05: 로딩 인디케이터 (Loading Indicator)

### 목적
웹 페이지 로딩 중 시각적 피드백(프로그레스바, 스피너)을 제공하여 사용자가 현재 상태를 명확히 인지하고, 로딩 완료까지 기다릴 수 있도록 합니다.

### 대상 사용자
- webOS Browser Native App의 모든 사용자 (웹 페이지 탐색 시)
- 특히 네트워크 속도가 느린 환경의 사용자
- 대화면 프로젝터에서 원거리 시청하는 사용자

### 요청 배경
- **사용자 경험**: 웹 페이지 로딩 시간이 길어질 경우 사용자는 앱이 멈춘 것으로 오해할 수 있음
- **PRD 요구사항**: 페이지 로딩 5초 이내 목표 (일반 웹사이트 기준)이지만, 네트워크 지연 시 더 길어질 수 있음
- **플랫폼 제약**: webOS 프로젝터 환경에서는 무선 네트워크 사용으로 로딩 시간 변동성이 큼
- **시각적 피드백 필요성**: 리모컨 조작 환경에서는 진행 상태 표시가 더욱 중요 (마우스 커서가 없어 시각적 단서 부족)

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: 프로그레스바 표시
- **설명**: 웹 페이지 로딩 중 화면 상단에 수평 프로그레스바를 표시하여 로딩 진행 상태를 시각적으로 전달합니다.
- **유저 스토리**: As a 사용자, I want 페이지 로딩 중 진행 상태를 보고 싶다, so that 앱이 정상 동작 중임을 확인하고 기다릴 수 있다.
- **우선순위**: Must
- **상세 기능**:
  - 로딩 시작 시 프로그레스바가 0%에서 시작
  - 로딩 진행에 따라 프로그레스바가 증가 (0% → 100%)
  - 로딩 완료 시 100%에 도달 후 페이드아웃
  - 로딩 실패 시 프로그레스바 즉시 사라짐
- **기술적 구현** (Native App):
  - **Qt QProgressBar** 위젯 사용
  - WebView의 `loadProgress(int)` 시그널 연결하여 실시간 진행률 표시
  - QWebEngineView는 실제 로딩 진행률(0~100%) 제공 가능
  - 프로그레스바 스타일은 QSS(Qt Style Sheet)로 커스터마이징

### FR-2: 로딩 스피너 표시
- **설명**: 프로그레스바와 함께 화면 중앙에 로딩 스피너(회전 애니메이션)를 표시하여 로딩 중임을 강조합니다.
- **유저 스토리**: As a 사용자, I want 페이지가 로딩 중일 때 명확한 시각적 표시를 보고 싶다, so that 앱이 응답하지 않는 것으로 착각하지 않는다.
- **우선순위**: Must
- **상세 기능**:
  - 로딩 시작 시 화면 중앙에 회전하는 스피너 표시
  - 스피너 아래에 "페이지 로딩 중..." 텍스트 표시
  - 로딩 완료 또는 실패 시 스피너 즉시 사라짐
  - 스피너는 WebView 위에 오버레이로 표시 (z-order 높음)
- **기술적 구현** (Native App):
  - **Qt QLabel** + **QMovie** (GIF 애니메이션) 또는 **QProgressBar (busy indicator)**
  - 대안: **QPainter** + **QTimer**로 커스텀 회전 애니메이션 구현
  - 오버레이: `QWidget::raise()` 또는 `QStackedWidget` 사용
  - 텍스트는 **QLabel** (스타일시트로 폰트 크기, 정렬 설정)

### FR-3: 로딩 상태 텍스트 표시
- **설명**: 현재 로딩 중인 URL 또는 상태 메시지를 텍스트로 표시하여 사용자에게 추가 정보를 제공합니다.
- **유저 스토리**: As a 사용자, I want 어떤 페이지를 로딩 중인지 알고 싶다, so that 올바른 URL을 입력했는지 확인할 수 있다.
- **우선순위**: Should
- **상세 기능**:
  - 로딩 중인 URL을 짧게 표시 (예: "www.google.com 로딩 중...")
  - 긴 URL은 말줄임(...) 처리 (최대 50자)
  - 스피너 아래 또는 프로그레스바 옆에 배치
  - 로딩 완료/실패 시 사라짐
- **기술적 구현** (Native App):
  - **QLabel**로 URL 표시
  - `QString::left(50) + "..."` 또는 **QFontMetrics::elidedText()** 사용
  - WebView의 `urlChanged(QUrl)` 시그널 연결

### FR-4: 로딩 타임아웃 표시
- **설명**: 로딩 시간이 30초를 초과할 경우 "로딩이 오래 걸리고 있습니다" 메시지를 표시합니다.
- **유저 스토리**: As a 사용자, I want 로딩이 비정상적으로 오래 걸릴 때 알림을 받고 싶다, so that 취소 또는 재시도 여부를 결정할 수 있다.
- **우선순위**: Should
- **상세 기능**:
  - 로딩 시작 후 30초 경과 시 경고 메시지 표시
  - "로딩이 오래 걸리고 있습니다. 네트워크를 확인해주세요." 텍스트
  - 취소 버튼 제공 (리모컨 Back 버튼 또는 화면 버튼)
  - 타임아웃 발생 시 WebView의 `loadTimeout()` 시그널 발생
- **기술적 구현** (Native App):
  - **QTimer** (30초 단발성 타이머)
  - `loadStarted()` 시 타이머 시작, `loadFinished()` 시 타이머 정지
  - 타임아웃 시 **QLabel** 또는 **QMessageBox**로 경고 표시
  - WebView::stop() 슬롯 호출로 로딩 취소

### FR-5: 실제 진행률 표시 (Qt WebEngine 기반)
- **설명**: QWebEngineView는 실제 로딩 진행률을 제공하므로, 이를 활용하여 정확한 프로그레스바를 표시합니다.
- **유저 스토리**: As a 사용자, I want 프로그레스바가 실제 로딩 진행률을 반영하길 원한다, so that 정확한 진행 상태를 파악할 수 있다.
- **우선순위**: Must
- **상세 기능**:
  - QWebEngineView의 `loadProgress(int progress)` 시그널 수신
  - 프로그레스바 값을 실시간으로 0~100% 업데이트
  - 진행률 0%: 로딩 시작
  - 진행률 100%: 로딩 완료 (페이드아웃 시작)
- **기술적 구현** (Native App):
  - WebView 클래스에서 `loadProgress(int)` 시그널 emit
  - LoadingIndicator에서 `QProgressBar::setValue(progress)` 호출
  - **Web App과의 차이**: Qt는 실제 진행률 제공 (가상 진행률 불필요)

### FR-6: 에러 상태 시각적 피드백
- **설명**: 페이지 로딩 실패 시 프로그레스바를 빨간색으로 변경하고 에러 아이콘을 표시합니다.
- **유저 스토리**: As a 사용자, I want 로딩 실패 시 명확한 시각적 피드백을 받고 싶다, so that 문제가 발생했음을 즉시 인지할 수 있다.
- **우선순위**: Should
- **상세 기능**:
  - 로딩 실패 시 프로그레스바 색상을 녹색 → 빨간색으로 변경
  - 스피너를 에러 아이콘(⚠️)으로 교체
  - 1초간 표시 후 페이드아웃
  - F-10(에러 처리)의 ErrorPage로 전환
- **기술적 구현** (Native App):
  - WebView의 `loadError(QString)` 시그널 연결
  - QProgressBar 스타일시트 동적 변경: `setStyleSheet("QProgressBar::chunk { background: red; }")`
  - QLabel 아이콘 변경: `QPixmap` 또는 유니코드 문자(⚠️)
  - QTimer (1초) 후 hide() 호출

### FR-7: Qt Widgets UI 통합
- **설명**: Qt Widgets 기반 UI 컴포넌트를 사용하여 플랫폼 일관성을 유지합니다.
- **우선순위**: Must
- **상세 기능**:
  - Qt의 표준 위젯 활용 (QProgressBar, QLabel, QWidget)
  - Qt Style Sheet (QSS)로 webOS 스타일 적용
  - 다크/라이트 테마 지원 (QPalette 또는 QSS 조건부 적용)
  - 커스텀 스타일 최소화 (Qt 기본 스타일 우선)
- **기술적 구현** (Native App):
  - QProgressBar의 기본 스타일 사용 (수정 최소화)
  - QSS 파일 분리 (`resources/styles/loading-indicator.qss`)
  - 테마 전환 시 `QApplication::setStyleSheet()` 호출

## 3. 비기능 요구사항 (Non-Functional Requirements)

### NFR-1: 성능
- **애니메이션 프레임레이트**: 프로그레스바 애니메이션은 최소 30fps 유지 (부드러운 시각 효과)
- **메모리 사용량**: LoadingIndicator 위젯은 3MB 이하 메모리 사용 (Qt 위젯 경량)
- **CPU 사용률**: 로딩 애니메이션이 WebView 렌더링을 방해하지 않도록 CPU 사용률 5% 이하 유지
- **초기화 시간**: LoadingIndicator 위젯 생성 시간 30ms 이하
- **UI 갱신 빈도**: QProgressBar 업데이트는 최대 10fps (과도한 repaint 방지)

### NFR-2: 사용성 (UX)
- **가독성**: 프로그레스바는 화면 상단 전체 너비를 차지하여 원거리에서도 보임
- **스피너 크기**: 스피너는 최소 80px 이상 크기로 대화면에서도 명확히 보임
- **텍스트 크기**: 로딩 상태 텍스트는 최소 24px 폰트 (PRD 가독성 기준)
- **대비**: 스피너와 배경 간 명도 대비 4.5:1 이상 (WCAG AA 기준)
- **페이드아웃 시간**: 로딩 완료 후 페이드아웃은 0.5초 이내 (빠른 전환)

### NFR-3: 접근성
- **색맹 대응**: 프로그레스바는 색상뿐만 아니라 애니메이션으로도 진행 상태 전달
- **모션 저감**: 모션 감도가 높은 사용자를 위해 애니메이션 비활성화 옵션 제공 (향후 F-11 설정에서 구현 고려)

### NFR-4: 반응성
- **로딩 시작 지연**: WebView 로딩 시작 후 50ms 이내 프로그레스바 표시 시작 (Qt 시그널/슬롯 즉시 실행)
- **로딩 완료 지연**: WebView 로딩 완료 후 100ms 이내 프로그레스바 100% 도달 및 페이드아웃

### NFR-5: 호환성
- **webOS 버전**: webOS 4.x 이상에서 동일한 Qt UI 동작 보장
- **Qt 버전**: Qt 5.12 이상 (LTS 버전), QWebEngineView 기능 활용
- **디바이스**: LG 프로젝터 HU715QW에서 정상 동작 (OpenGL 가속 활용)

## 4. 제약조건

### 기술적 제약
1. **Qt WebEngine 의존**: QWebEngineView의 loadProgress 시그널 제공 필수
   - 대응: WebView.h에 이미 `loadProgress(int)` 시그널 정의됨
2. **webOS 성능 제약**: 프로젝터 하드웨어 성능이 제한적이므로 UI 복잡도 제한
   - 대응: Qt 표준 위젯 사용 (하드웨어 가속 지원), 커스텀 그리기 최소화
3. **리모컨 입력**: 프로그레스바는 포커스를 받지 않아야 함 (시각적 피드백만)
   - 대응: `QWidget::setFocusPolicy(Qt::NoFocus)` 설정

### 디자인 제약
1. **Qt Widgets 스타일**: Qt 표준 위젯 스타일을 따라야 하므로 커스텀 스타일 최소화
2. **대화면 가독성**: 최소 폰트 크기 24px, 프로그레스바 높이 최소 8px

### 기능 의존성
1. **F-02 의존**: WebView의 `loadStarted()`, `loadProgress(int)`, `loadFinished(bool)`, `loadError(QString)` 시그널 필요
2. **F-04 연동**: 페이지 탐색(뒤로/앞으로) 시에도 로딩 인디케이터 동작
3. **F-10 연동**: 로딩 실패 시 에러 페이지로 전환

## 5. 용어 정의

| 용어 | 정의 |
|------|------|
| 프로그레스바 (ProgressBar) | 로딩 진행률을 수평 막대로 표시하는 UI 요소 (0~100%) |
| 스피너 (Spinner) | 회전 애니메이션으로 로딩 중임을 표시하는 UI 요소 |
| QProgressBar | Qt Widgets의 표준 진행률 표시 위젯 |
| QSS (Qt Style Sheet) | Qt 위젯 스타일링을 위한 CSS 유사 언어 |
| 오버레이 (Overlay) | WebView 위에 겹쳐서 표시되는 UI 레이어 (raise() 사용) |
| 페이드아웃 (Fade-out) | 불투명도를 점차 감소시켜 사라지는 애니메이션 효과 (QPropertyAnimation) |
| 시그널/슬롯 (Signal/Slot) | Qt의 이벤트 통신 메커니즘 |

## 6. 범위 외 (Out of Scope)

### 이번 기능에서 하지 않는 것
- **가상 진행률 알고리즘**: Qt WebEngine은 실제 진행률 제공하므로 불필요
- **로딩 취소 기능**: F-04에서 구현 예정 (뒤로 버튼으로 네비게이션 취소)
- **로딩 통계 수집**: 페이지별 평균 로딩 시간 통계는 F-08(히스토리)에서 고려
- **네트워크 속도 표시**: 현재 네트워크 속도 측정 및 표시는 범위 외
- **다운로드 진행률**: 파일 다운로드는 F-12(다운로드 관리)에서 별도 구현

## 7. 완료 기준 (Acceptance Criteria)

### AC-1: 프로그레스바 정상 동작
- [ ] 페이지 로딩 시작 시 프로그레스바가 화면 상단에 표시됨
- [ ] 프로그레스바가 0%에서 시작하여 실제 진행률에 따라 증가함
- [ ] 로딩 완료 시 프로그레스바가 100%에 도달 후 0.5초 이내 사라짐
- [ ] 프로그레스바 높이는 8px 이상이며 원거리에서도 명확히 보임

### AC-2: 스피너 정상 동작
- [ ] 페이지 로딩 시작 시 화면 중앙에 스피너가 표시됨
- [ ] 스피너는 부드럽게 회전하며 멈추지 않음 (30fps 이상)
- [ ] 스피너 아래에 "페이지 로딩 중..." 텍스트 표시됨
- [ ] 로딩 완료 시 스피너가 즉시 사라짐

### AC-3: 실제 진행률 표시
- [ ] QWebEngineView의 loadProgress 시그널이 0~100% 값 emit
- [ ] 프로그레스바가 실시간으로 진행률 반영
- [ ] 진행률 100% 도달 시 즉시 페이드아웃 시작

### AC-4: 에러 상태 시각적 피드백
- [ ] 로딩 실패 시 프로그레스바가 빨간색으로 변경됨
- [ ] 스피너가 에러 아이콘(⚠️)으로 교체됨
- [ ] 1초간 표시 후 에러 오버레이로 전환됨

### AC-5: 성능 요구사항 충족
- [ ] 프로그레스바 애니메이션이 30fps 이상 유지됨
- [ ] LoadingIndicator 위젯 메모리 사용량 3MB 이하
- [ ] 로딩 시작 후 50ms 이내 프로그레스바 표시 시작

### AC-6: 다양한 시나리오 테스트
- [ ] 빠른 페이지 로딩 (1초 이내): 프로그레스바가 즉시 100%로 이동 후 사라짐
- [ ] 느린 페이지 로딩 (10초 이상): 프로그레스바가 진행률 반영하며 증가
- [ ] 매우 느린 페이지 로딩 (30초 초과): 타임아웃 메시지 표시됨
- [ ] 페이지 로딩 실패 (404): 프로그레스바가 빨간색으로 변경 후 에러 화면 전환
- [ ] 연속 로딩 (탭 전환): 이전 로딩 인디케이터가 정리되고 새 로딩 시작

### AC-7: 리모컨 조작 통합
- [ ] 로딩 중에도 리모컨 Back 버튼으로 로딩 취소 가능 (F-04 연동)
- [ ] 로딩 중 포커스는 WebView 컨테이너에 유지됨
- [ ] 프로그레스바는 포커스를 받지 않음 (NoFocus 정책)

### AC-8: 실제 디바이스 테스트
- [ ] LG 프로젝터 HU715QW에서 프로그레스바가 정상 표시됨
- [ ] 대화면(100인치 이상)에서 3m 거리에서 프로그레스바 시인성 확인
- [ ] 주요 웹사이트(Google, YouTube, Naver) 로딩 시 정상 동작

### AC-9: 코드 품질
- [ ] LoadingIndicator 클래스 Doxygen 주석 작성 완료
- [ ] 헤더 가드 (`#pragma once`) 사용
- [ ] 컴파일 경고 없음 (Qt Creator에서 확인)
- [ ] 컴포넌트 문서(docs/components/LoadingIndicator.md) 작성 완료

## 8. 사용 시나리오

### 시나리오 1: 일반 페이지 로딩
1. 사용자가 URLBar에서 "www.google.com" 입력 후 Enter
2. WebView가 `loadStarted()` 시그널 emit
3. **프로그레스바가 화면 상단에 표시**, 0%에서 시작
4. **스피너가 화면 중앙에 표시**, "페이지 로딩 중..." 텍스트
5. WebView가 `loadProgress(25)` emit → 프로그레스바 25% 표시
6. WebView가 `loadProgress(50)` emit → 프로그레스바 50% 표시
7. WebView가 `loadProgress(100)` emit → 프로그레스바 100% 도달
8. WebView가 `loadFinished(true)` emit
9. **프로그레스바가 0.5초 페이드아웃**, **스피너 사라짐**, Google 홈페이지 표시

### 시나리오 2: 느린 네트워크 환경
1. 사용자가 "www.youtube.com" 로딩 시작
2. 프로그레스바 및 스피너 표시
3. 진행률이 천천히 증가: 10% → 20% → 30% ...
4. 30초 경과: **"로딩이 오래 걸리고 있습니다" QLabel 표시**
5. 35초 경과: WebView가 `loadFinished(true)` emit
6. 프로그레스바 100% 이동 후 사라짐

### 시나리오 3: 로딩 실패
1. 사용자가 "http://invalidurl.com" 로딩 시도
2. 프로그레스바 및 스피너 표시
3. 진행률 30% 도달
4. WebView가 `loadError("ERR_NAME_NOT_RESOLVED")` 시그널 emit
5. **프로그레스바가 빨간색으로 변경** (QSS 동적 적용)
6. **스피너가 에러 아이콘(⚠️)으로 교체**
7. 1초 후 에러 오버레이 표시 (F-02의 ErrorOverlay)

### 시나리오 4: 빠른 페이지 로딩 (캐시)
1. 사용자가 이미 캐시된 "www.google.com" 재로딩
2. 프로그레스바 및 스피너 표시
3. 0.5초 이내: WebView가 `loadProgress(100)` → `loadFinished(true)` emit
4. 프로그레스바 즉시 100% 이동 후 페이드아웃
5. 스피너 즉시 사라짐

## 9. 향후 확장 가능성

### F-06 (탭 관리)와의 연동
- 각 탭마다 독립적인 로딩 인디케이터 상태 관리
- 백그라운드 탭 로딩 시 작은 로딩 아이콘만 표시 (프로그레스바 숨김)
- TabManager에서 활성 탭 변경 시 LoadingIndicator 위젯 전환

### F-11 (설정 화면)과의 연동
- 애니메이션 비활성화 옵션 제공 (모션 감도가 높은 사용자 대응)
- 프로그레스바 스타일 선택 (슬림/굵게)
- QSettings를 통한 설정 영속화

### 성능 모니터링
- 페이지별 평균 로딩 시간 수집 → HistoryService와 통합
- QElapsedTimer로 로딩 시간 측정

## 10. Native App 특화 사항

### Web App PoC와의 차이점
| 항목 | Web App (React/Enact) | Native App (Qt/C++) |
|------|----------------------|---------------------|
| 진행률 | iframe 제약으로 가상 진행률 | QWebEngineView가 실제 진행률 제공 |
| UI 프레임워크 | Enact Moonstone ProgressBar/Spinner | Qt QProgressBar/QLabel |
| 애니메이션 | CSS transitions/keyframes | QPropertyAnimation 또는 QTimer |
| 스타일링 | CSS | QSS (Qt Style Sheet) |
| 이벤트 통신 | React state/props | Qt Signal/Slot |
| 메모리 | JavaScript 엔진 (V8) | Qt C++ (경량) |

### Qt 기술 스택
- **UI 위젯**: QProgressBar, QLabel, QWidget
- **애니메이션**: QPropertyAnimation (페이드아웃), QTimer (스피너 회전)
- **스타일링**: QSS 파일 (`resources/styles/loading-indicator.qss`)
- **레이아웃**: QVBoxLayout, QHBoxLayout
- **이벤트**: Signal/Slot 메커니즘

### 성능 최적화 전략
1. **프로그레스바 업데이트 쓰로틀링**: loadProgress 시그널이 빈번하게 emit될 경우, 10fps로 제한 (QTimer 사용)
2. **하드웨어 가속**: Qt OpenGL 렌더링 활용 (webOS 기본 지원)
3. **메모리 관리**: PIMPL 패턴으로 내부 구현 캡슐화, QScopedPointer 사용

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | Native App 관점 전면 재작성 | Qt/C++ 기술 스택으로 마이그레이션 |
| 2026-02-14 | FR-5 추가 (실제 진행률 표시) | QWebEngineView 기능 활용 |
| 2026-02-14 | 가상 진행률 알고리즘 삭제 | Qt는 실제 진행률 제공하므로 불필요 |
