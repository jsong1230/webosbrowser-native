# F-13: 리모컨 단축키 — 요구사항 분석서

## 1. 개요

- **기능명**: 리모컨 단축키 (Remote Control Shortcuts)
- **기능 ID**: F-13
- **목적**: webOS 리모컨의 채널 버튼, 컬러 버튼, 숫자 버튼 등을 활용하여 브라우저 주요 기능(탭 전환, 북마크, 히스토리 접근 등)에 빠르게 접근할 수 있도록 함
- **대상 사용자**: LG 프로젝터 HU715QW 리모컨을 사용하는 모든 사용자
- **요청 배경**:
  - 현재 주요 기능(탭 전환, 북마크, 히스토리 등)은 NavigationBar의 버튼을 통해서만 접근 가능
  - 리모컨의 채널 버튼, 컬러 버튼 등 유휴 버튼을 활용하면 대화면 환경에서 UX 개선 가능
  - webOS TV 플랫폼 사용자들이 익숙한 단축키 패턴 제공 (채널 버튼 = 탭 전환, 컬러 버튼 = 주요 기능)
  - 3m 거리에서 리모컨만으로 모든 기능에 접근할 수 있는 사용성 향상

---

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: 채널 버튼을 통한 탭 전환

- **설명**: 리모컨의 채널 Up/Down 버튼으로 탭 간 빠른 전환 제공
- **유저 스토리**: As a 프로젝터 사용자, I want 채널 버튼으로 탭을 전환하고 싶다, so that 복잡한 Qt 위젯 포커스 네비게이션 없이 빠르게 탭을 이동할 수 있다
- **우선순위**: Must

#### 키 매핑
| 키 | Qt::Key | webOS keyCode | 동작 |
|----|---------|---------------|------|
| Channel Up | Qt::Key_ChannelUp | 427 | 이전 탭으로 전환 (순환) |
| Channel Down | Qt::Key_ChannelDown | 428 | 다음 탭으로 전환 (순환) |

#### 동작 상세
1. **탭 순환 전환**:
   - 마지막 탭에서 Channel Down → 첫 탭으로 이동
   - 첫 탭에서 Channel Up → 마지막 탭으로 이동
2. **포커스 관리**:
   - 탭 전환 후 자동으로 WebView 위젯에 포커스 (`webView_->setFocus()`)
   - URL 입력 필드(URLBar)의 값을 새 탭의 URL로 업데이트
3. **탭 1개일 때**:
   - 키 입력 무시 (로그만 기록)
4. **디바운스**:
   - 0.5초 이내 중복 입력 방지 (QTimer 기반)

#### 제약조건
- F-06 (탭 관리 시스템) 의존: TabManager::switchTab() 호출
- BrowserWindow::keyPressEvent() 오버라이드에서 처리
- NavigationBar 상태 업데이트 (현재 탭 제목, URL 표시)

---

### FR-2: 컬러 버튼을 통한 주요 기능 접근

- **설명**: 리모컨의 Red/Green/Blue/Yellow 컬러 버튼에 주요 기능 매핑
- **유저 스토리**: As a 프로젝터 사용자, I want 컬러 버튼으로 자주 쓰는 기능에 빠르게 접근하고 싶다, so that NavigationBar 버튼을 포커스하고 클릭하는 시간을 절약할 수 있다
- **우선순위**: Must

#### 키 매핑
| 키 | Qt::Key | webOS keyCode | 동작 | 관련 기능 |
|----|---------|---------------|------|-----------|
| Red | Qt::Key_Red | 403, 404 | 북마크 패널 열기 | F-07 |
| Green | Qt::Key_Green | 405, 406 | 히스토리 패널 열기 | F-08 |
| Yellow | Qt::Key_Yellow | 405, 406 | 다운로드 패널 열기 | F-12 |
| Blue | Qt::Key_Blue | 407, 408 | 새 탭 생성 | F-06 |

**주의**: Green과 Yellow의 keyCode가 중복될 수 있으므로, 실제 기기 테스트 필요.

#### 동작 상세
1. **북마크 패널 열기 (Red)**:
   - `BookmarkPanel::show()` 호출
   - 패널 슬라이드 인 애니메이션 (QPropertyAnimation)
   - 패널에 포커스 자동 이동 (`bookmarkPanel_->setFocus()`)
   - 이미 열려있을 때는 무시 (중복 열기 방지)
2. **히스토리 패널 열기 (Green)**:
   - `HistoryPanel::show()` 호출 (F-08 구현 시)
   - 패널 슬라이드 인 애니메이션
   - 패널에 포커스 자동 이동
3. **다운로드 패널 열기 (Yellow)**:
   - `DownloadPanel::show()` 호출 (F-12 구현 시)
4. **새 탭 생성 (Blue)**:
   - `TabManager::createTab()` 호출
   - 최대 탭 수(5개) 체크 (`TabManager::getTabCount() < 5`)
   - 최대 초과 시 QMessageBox 경고 표시 (선택적)

#### 제약조건
- F-06 (탭 관리), F-07 (북마크), F-08 (히스토리), F-12 (다운로드) 의존
- BrowserWindow에서 각 패널 포인터 관리 (`bookmarkPanel_`, `historyPanel_`, `downloadPanel_`)
- 패널이 이미 `isVisible() == true`이면 중복 실행 방지

---

### FR-3: 숫자 버튼을 통한 직접 탭 선택

- **설명**: 리모컨의 숫자 버튼(1~5)으로 특정 탭에 즉시 접근
- **유저 스토리**: As a 프로젝터 사용자, I want 숫자 버튼으로 원하는 탭에 바로 이동하고 싶다, so that 여러 번 채널 버튼을 누르지 않아도 된다
- **우선순위**: Should

#### 키 매핑
| 키 | Qt::Key | webOS keyCode | 동작 |
|----|---------|---------------|------|
| 1 | Qt::Key_1 | 49 | 첫 번째 탭으로 이동 |
| 2 | Qt::Key_2 | 50 | 두 번째 탭으로 이동 |
| 3 | Qt::Key_3 | 51 | 세 번째 탭으로 이동 |
| 4 | Qt::Key_4 | 52 | 네 번째 탭으로 이동 |
| 5 | Qt::Key_5 | 53 | 다섯 번째 탭으로 이동 |

#### 동작 상세
1. **탭 인덱스 매핑**:
   - 숫자 1 → tabs_[0], 숫자 2 → tabs_[1], ... , 숫자 5 → tabs_[4]
2. **탭 존재 여부 체크**:
   - 입력 숫자가 현재 탭 개수를 초과하면 무시 (예: 탭 3개일 때 4 입력 시 무시)
   - Logger::debug로 "존재하지 않는 탭 번호" 로그 기록
3. **이미 활성화된 탭**:
   - 입력 숫자가 현재 활성 탭과 동일하면 무시 (Logger::debug 기록)
4. **포커스 관리**:
   - 탭 전환 후 WebView에 포커스 (`webView_->setFocus()`)
5. **URLBar 키보드 열려있을 때**:
   - **숫자 키 입력을 URLBar가 우선 처리**
   - BrowserWindow::keyPressEvent()에서 `urlBar_->hasFocus()`를 체크하여 분기
   - URLBar 포커스 시 탭 전환 동작 무시

#### 제약조건
- 최대 탭 수가 5개이므로 1~5 버튼만 사용
- URLBar의 QLineEdit이 포커스를 가지고 있을 때는 동작하지 않음 (`event->ignore()` 호출)

---

### FR-4: 설정 버튼 매핑

- **설명**: 리모컨의 설정/메뉴 버튼으로 브라우저 설정 화면 접근
- **유저 스토리**: As a 프로젝터 사용자, I want 설정 버튼으로 브라우저 설정에 빠르게 접근하고 싶다, so that NavigationBar를 거치지 않아도 된다
- **우선순위**: Should

#### 키 매핑
| 키 | Qt::Key | webOS keyCode | 동작 | 관련 기능 |
|----|---------|---------------|------|-----------|
| Menu/Settings | Qt::Key_Menu | 457, 18 | 설정 패널 열기 | F-11 |

#### 동작 상세
1. **설정 패널 열기**:
   - `SettingsPanel::show()` 호출 (F-11 구현 시)
   - 패널에 포커스 자동 이동 (`settingsPanel_->setFocus()`)
   - 이미 열려있을 때는 무시

#### 제약조건
- F-11 (설정 화면) 의존
- BrowserWindow에서 `settingsPanel_` 포인터 관리

---

### FR-5: 재생 컨트롤 버튼 (Play/Pause) 매핑 (선택적)

- **설명**: 리모컨의 재생 버튼으로 페이지 스크롤 제어
- **유저 스토리**: As a 프로젝터 사용자, I want 재생 버튼으로 페이지를 스크롤하고 싶다, so that 긴 문서를 편하게 읽을 수 있다
- **우선순위**: Could (M3 이후 구현 검토)

#### 키 매핑
| 키 | Qt::Key | webOS keyCode | 동작 |
|----|---------|---------------|------|
| Play/Pause | Qt::Key_MediaPlay / Qt::Key_MediaPause | 415, 19 | 자동 스크롤 시작/멈춤 |
| FastForward | Qt::Key_MediaNext | 417 | 페이지 끝으로 스크롤 |
| Rewind | Qt::Key_MediaPrevious | 412 | 페이지 맨 위로 스크롤 |

#### 동작 상세
1. **자동 스크롤**:
   - QTimer 기반 자동 스크롤 (초당 100px, 10ms 간격)
   - Play 상태에서 Pause 누르면 QTimer 중지
2. **페이지 끝/위로 이동**:
   - WebView의 QWebEngineView::page()->runJavaScript() 호출
   - `window.scrollTo(0, document.body.scrollHeight)` (끝으로)
   - `window.scrollTo(0, 0)` (위로)
3. **WebView 포커스 시에만 동작**:
   - `webView_->hasFocus()` 체크

#### 제약조건
- **CORS 정책**으로 일부 사이트에서는 JavaScript 실행 불가
- QWebEngineView::page()->runJavaScript() 실행 결과를 확인하여 에러 처리
- 프레임 드롭 방지를 위해 스크롤 속도 최적화 필요

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### NFR-1: 반응 속도

- **목표**: 리모컨 키 입력 후 0.3초 이내 시각적 피드백 제공
- **구현 방안**:
  - BrowserWindow::keyPressEvent()에서 동기 처리 (이벤트 루프 블로킹 최소화)
  - 탭 전환 애니메이션은 0.5초 이내 완료 (QPropertyAnimation 지속 시간)
  - 패널 슬라이드 인 애니메이션도 0.3초 이내

### NFR-2: 디바운스 (중복 입력 방지)

- **목표**: 0.5초 이내 동일 키 재입력 무시
- **구현 방안**:
  - QMap<int, qint64> lastKeyPressTime_ 멤버 변수 관리
  - keyPressEvent()에서 QDateTime::currentMSecsSinceEpoch()로 마지막 입력 시각 체크
  - 0.5초(500ms) 이내 재입력이면 `event->ignore()` 호출

### NFR-3: 키 충돌 방지

- **URLBar 가상 키보드 우선순위**:
  - URLBar가 포커스를 가지고 있을 때 (`urlBar_->hasFocus()`) 숫자 키, 채널 버튼 무시
  - Back 키, 컬러 버튼은 URLBar 포커스와 무관하게 동작 (패널 열기 허용)
- **패널 내부 포커스 우선순위**:
  - 패널이 열려있을 때 (`bookmarkPanel_->isVisible()`) 패널 내부 키 이벤트 우선 처리
  - 패널 내부에서 처리되지 않은 키만 BrowserWindow로 전달
- **Back 키 기존 동작 유지**:
  - WebView 포커스 시: 브라우저 뒤로가기 (`webView_->back()`)
  - 패널 열려있을 때: 패널 닫기 (`bookmarkPanel_->hide()`)
  - NavigationBar 포커스 시: Qt 기본 동작 (포커스 이탈)

### NFR-4: 확장성

- **키 코드 상수 관리**:
  - `src/utils/KeyCodeConstants.h` 파일에 webOS 키 코드 상수 정의
  - `constexpr int KEY_CHANNEL_UP = 427;`
  - 새로운 키 매핑 추가 시 상수 파일만 수정
- **KeyboardHandler 서비스 (선택적)**:
  - 향후 사용자 정의 키 매핑 기능 추가 시 KeyboardHandler 클래스로 분리 가능
  - 현재는 BrowserWindow::keyPressEvent()에서 직접 처리 (단순성 우선)

### NFR-5: 디버깅 지원

- **키 입력 로깅**:
  - Logger::debug로 모든 키 입력 기록
  - `Logger::debug("[BrowserWindow] 키 입력: keyCode=" + QString::number(event->key()))`
- **키 매핑 실패 시 경고**:
  - 탭 전환 실패 시 `Logger::warn("[BrowserWindow] 탭 전환 실패: 존재하지 않는 탭")`
  - 최대 탭 수 초과 시 `Logger::warn("[BrowserWindow] 새 탭 생성 실패: 최대 5개")`

---

## 4. 제약조건

### 4.1 webOS 리모컨 키코드 제약사항

- **기기별 keyCode 차이**: webOS 기기 및 펌웨어 버전에 따라 keyCode가 다를 수 있음
  - Yellow/Green 버튼은 405 또는 406으로 중복 가능
  - 실제 기기에서 QKeyEvent::key() 값 로그로 확인 필수
- **시스템 예약 키**: 일부 키는 webOS 시스템 레벨에서 예약되어 앱에서 오버라이드 불가
  - Home 버튼 (Qt::Key_Home): webOS 런처로 이동 (앱에서 가로챌 수 없음)
  - Power 버튼: 프로젝터 전원 제어 (앱에서 가로챌 수 없음)

### 4.2 Qt 이벤트 시스템 제약

- **이벤트 전파 순서**:
  1. QWidget::keyPressEvent() (자식 위젯 → 부모 위젯 순)
  2. 자식 위젯이 `event->ignore()` 호출 시 부모로 전달
  3. `event->accept()` 호출 시 전파 중단
- **포커스 정책**:
  - Qt::StrongFocus 위젯만 키보드 이벤트 수신 가능
  - BrowserWindow는 Qt::StrongFocus 설정 필수
  - 자식 위젯(WebView, URLBar)에 포커스가 있을 때는 해당 위젯이 먼저 처리

### 4.3 WebView (QWebEngineView) 제약

- **iframe CORS 정책**:
  - 외부 사이트의 iframe contentWindow 접근 불가
  - JavaScript 스크롤 제어 시 일부 사이트에서 에러 발생 가능
  - `QWebEngineView::page()->runJavaScript()` 실행 결과를 람다로 확인하여 에러 핸들링
- **키 이벤트 소비**:
  - WebView에 포커스가 있을 때 일부 키는 웹 페이지로 전달됨
  - BrowserWindow::keyPressEvent()에서 먼저 가로채려면 `installEventFilter()` 사용 검토

### 4.4 성능 제약

- **탭 전환 렌더링 지연**:
  - 싱글 WebView 인스턴스 구조에서 탭 전환 시 WebView::load() 호출 → 0.5~1초 지연 가능
  - 사용자 피드백: LoadingIndicator 표시 (F-05 연동)
- **자동 스크롤 프레임 드롭**:
  - QTimer 기반 스크롤 시 10ms 간격으로 100px 이동 시 CPU 부하 발생 가능
  - 대안: QPropertyAnimation + QGraphicsOpacityEffect로 부드러운 스크롤 구현

---

## 5. 용어 정의

| 용어 | 정의 |
|------|------|
| **Channel Button** | 리모컨의 채널 Up/Down 버튼 (Qt::Key_ChannelUp, Qt::Key_ChannelDown) |
| **Color Button** | 리모컨의 Red/Green/Yellow/Blue 버튼 (Qt::Key_Red, Qt::Key_Green 등) |
| **Playback Control** | 재생 버튼 (Play/Pause, FastForward, Rewind) |
| **Back Button** | 리모컨의 뒤로가기 버튼 (Qt::Key_Escape 또는 webOS Back 키) |
| **Menu/Settings Button** | 리모컨의 설정/메뉴 버튼 (Qt::Key_Menu) |
| **Debounce** | 중복 입력 방지를 위한 짧은 대기 시간 (0.5초) |
| **KeyCodeConstants** | webOS 리모컨 키 코드 상수를 정의한 헤더 파일 (`src/utils/KeyCodeConstants.h`) |
| **QKeyEvent** | Qt 키 입력 이벤트 객체 (QWidget::keyPressEvent()로 수신) |
| **QWebEngineView** | Qt WebEngine 기반 웹 렌더링 위젯 (WebView 래퍼) |

---

## 6. 범위 외 (Out of Scope)

### 이번 버전에서 구현하지 않는 것

- **음성 인식 단축키**: webOS 음성 인식 API는 별도 기능으로 분리 (F-16 후보)
- **사용자 정의 키 매핑**: 설정 화면에서 키 매핑을 변경하는 기능 (추후 고려, F-17 후보)
- **키보드 단축키 튜토리얼**: 처음 실행 시 단축키를 안내하는 오버레이 (F-18 후보)
- **제스처 인식**: 리모컨 모션 센서 활용은 제외 (webOS 플랫폼 미지원)
- **북마크 추가 단축키 (Yellow 버튼)**: features.md에 명시되었으나 Yellow는 다운로드 패널로 사용하기로 결정 (design.md 참조)
- **마우스/터치 입력**: 리모컨 전용 기능 (마우스 클릭으로는 단축키 동작하지 않음)

---

## 7. 수용 기준 (Acceptance Criteria)

### AC-1: 채널 버튼 탭 전환

- [ ] Channel Up 버튼으로 이전 탭으로 전환 가능
- [ ] Channel Down 버튼으로 다음 탭으로 전환 가능
- [ ] 탭 전환 시 WebView에 자동 포커스 (`webView_->setFocus()` 호출)
- [ ] URLBar에 새 탭의 URL 표시 (`urlBar_->setText(newTab->url())`)
- [ ] 탭이 1개만 있을 때 키 입력 무시 (Logger::debug 로그)
- [ ] 순환 전환 동작 (첫 탭 ↔ 마지막 탭)
- [ ] 0.5초 이내 중복 입력 무시 (디바운스)

### AC-2: 컬러 버튼 기능 접근

- [ ] Red 버튼으로 북마크 패널 열기 가능 (`bookmarkPanel_->show()`)
- [ ] Green 버튼으로 히스토리 패널 열기 가능 (`historyPanel_->show()`)
- [ ] Blue 버튼으로 새 탭 생성 가능 (`TabManager::createTab()`)
- [ ] Yellow 버튼으로 다운로드 패널 열기 가능 (`downloadPanel_->show()`)
- [ ] 패널이 이미 `isVisible() == true`이면 중복 열기 무시
- [ ] 새 탭 생성 시 최대 탭 수(5개) 제한 체크 (`TabManager::getTabCount() < 5`)
- [ ] 최대 초과 시 Logger::warn 로그 출력

### AC-3: 숫자 버튼 직접 탭 선택

- [ ] 1~5 버튼으로 해당 탭에 즉시 이동 가능
- [ ] 존재하지 않는 탭 번호 입력 시 무시 (Logger::debug 로그)
- [ ] 이미 활성화된 탭 번호 입력 시 무시
- [ ] URLBar가 포커스를 가지고 있을 때 (`urlBar_->hasFocus()`) 숫자 키 입력 무시 (URLBar에 입력)
- [ ] 탭 전환 후 WebView에 포커스 (`webView_->setFocus()`)

### AC-4: 설정 버튼 접근

- [ ] Menu/Settings 버튼으로 설정 패널 열기 가능 (`settingsPanel_->show()`)
- [ ] 패널 열기 시 자동 포커스 (`settingsPanel_->setFocus()`)
- [ ] 패널이 이미 열려있을 때 무시

### AC-5: 스크롤 제어 (선택적, M3 이후)

- [ ] Play/Pause 버튼으로 자동 스크롤 시작/멈춤 (QTimer 기반)
- [ ] FastForward 버튼으로 페이지 끝으로 이동 (`window.scrollTo(0, document.body.scrollHeight)`)
- [ ] Rewind 버튼으로 페이지 맨 위로 이동 (`window.scrollTo(0, 0)`)
- [ ] WebView 포커스 시에만 동작 (`webView_->hasFocus()` 체크)
- [ ] CORS 에러 발생 시 Logger::warn 로그 출력

### AC-6: 반응 속도 및 키 충돌 방지

- [ ] 리모컨 키 입력 후 0.3초 이내 시각적 피드백 (패널 슬라이드 인 시작)
- [ ] 0.5초 이내 동일 키 재입력 무시 (디바운스)
- [ ] URLBar 포커스 시 채널/숫자 버튼 무시, 컬러 버튼은 허용
- [ ] 패널 열려있을 때 패널 내부 키 이벤트 우선 처리

### AC-7: 로깅 및 디버깅

- [ ] 모든 키 입력 시 Logger::debug로 keyCode 기록
- [ ] 키 매핑 실패 시 Logger::warn 로그 출력 (예: 존재하지 않는 탭, 최대 탭 수 초과)

### AC-8: 기존 기능과의 호환성

- [ ] Back 키 기존 동작 유지 (브라우저 뒤로가기/패널 닫기)
- [ ] Qt 포커스 시스템과 충돌 없음
- [ ] NavigationBar 버튼 클릭 동작 정상 작동 (리모컨 단축키와 독립적)
- [ ] F-06 (탭 관리) SWITCH_TAB 액션과 통합 (TabManager 호출)

---

## 8. 테스트 시나리오

### 시나리오 1: 채널 버튼으로 탭 전환

1. 탭 3개 생성 (Tab1, Tab2, Tab3)
2. Tab1 활성화 상태에서 Channel Down 버튼 누름
3. **예상**: Tab2로 전환, WebView 포커스, URLBar에 Tab2 URL 표시
4. Channel Down 한 번 더 누름
5. **예상**: Tab3로 전환
6. Channel Down 한 번 더 누름
7. **예상**: Tab1로 순환 전환

### 시나리오 2: 컬러 버튼으로 패널 열기

1. Red 버튼 누름
2. **예상**: 북마크 패널 슬라이드 인, 패널에 포커스
3. Back 키로 패널 닫기
4. Green 버튼 누름
5. **예상**: 히스토리 패널 슬라이드 인
6. Back 키로 패널 닫기
7. Blue 버튼 누름
8. **예상**: 새 탭 생성, 활성화 (Google 홈페이지 로드)

### 시나리오 3: 숫자 버튼으로 탭 선택

1. 탭 4개 생성
2. 숫자 3 버튼 누름
3. **예상**: 세 번째 탭 활성화
4. 숫자 5 버튼 누름
5. **예상**: 무시 (탭 5는 없으므로), Logger::debug 로그 기록
6. 숫자 1 버튼 누름
7. **예상**: 첫 번째 탭 활성화

### 시나리오 4: URLBar 포커스 시 충돌 방지

1. URLBar 클릭, 포커스 획득
2. 숫자 3 버튼 누름
3. **예상**: URLBar에 "3" 입력됨, 탭 전환 안 됨
4. Back 키로 포커스 해제
5. 숫자 3 버튼 누름
6. **예상**: 세 번째 탭으로 전환

### 시나리오 5: 최대 탭 수 제한

1. 탭 5개 생성 (최대 개수)
2. Blue 버튼 누름
3. **예상**: 새 탭 생성 안 됨, Logger::warn 로그 출력

### 시나리오 6: 스크롤 제어 (선택적)

1. 긴 웹 페이지 로드 (예: Wikipedia 문서)
2. Play 버튼 누름
3. **예상**: 자동 스크롤 시작 (QTimer, 10ms 간격)
4. Pause 버튼 누름
5. **예상**: 스크롤 멈춤 (QTimer 중지)
6. FastForward 버튼 누름
7. **예상**: 페이지 끝으로 즉시 이동
8. Rewind 버튼 누름
9. **예상**: 페이지 맨 위로 즉시 이동

---

## 9. 의존성

### 필수 의존 기능

| 기능 ID | 기능명 | 의존 이유 |
|---------|--------|----------|
| F-06 | 탭 관리 시스템 | 채널/숫자 버튼으로 탭 전환, Blue 버튼으로 새 탭 생성 |
| F-07 | 북마크 관리 | Red 버튼으로 북마크 패널 열기 |
| F-08 | 히스토리 관리 | Green 버튼으로 히스토리 패널 열기 |
| F-12 | 다운로드 관리 | Yellow 버튼으로 다운로드 패널 열기 |

### 선택적 의존 기능

| 기능 ID | 기능명 | 의존 이유 |
|---------|--------|----------|
| F-11 | 설정 화면 | Menu/Settings 버튼으로 설정 패널 열기 |
| F-05 | 로딩 인디케이터 | 탭 전환 시 로딩 피드백 표시 |

---

## 10. 우선순위 및 일정

### 우선순위

- **Must (필수)**: FR-1 (채널 버튼 탭 전환), FR-2 (컬러 버튼 기능 접근)
- **Should (권장)**: FR-3 (숫자 버튼 직접 탭 선택), FR-4 (설정 버튼)
- **Could (선택)**: FR-5 (재생 컨트롤 버튼 스크롤 제어)

### 개발 일정

- **Phase 1 (Must 구현)**: 3일
  - Day 1: KeyCodeConstants.h 작성, BrowserWindow::keyPressEvent() 기본 구조
  - Day 2: FR-1, FR-2 구현 (채널 버튼, 컬러 버튼)
  - Day 3: 단위 테스트 작성, 실제 기기 테스트
- **Phase 2 (Should 구현)**: 2일
  - Day 4: FR-3, FR-4 구현 (숫자 버튼, 설정 버튼)
  - Day 5: 통합 테스트, 디버깅
- **Phase 3 (Could 구현, M3 이후)**: 2일
  - Day 6: FR-5 구현 (스크롤 제어)
  - Day 7: CORS 에러 핸들링, 성능 최적화

---

## 11. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초안 작성 (webOS Native App C++/Qt 기반으로 재작성) | product-manager |
| 2026-02-14 | Qt 키 이벤트, QWebEngineView 제약사항 추가 | product-manager |
| 2026-02-14 | KeyCodeConstants.h 파일 추가, 디바운스 구현 방안 명시 | product-manager |
