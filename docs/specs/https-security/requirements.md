# F-14: HTTPS 보안 표시 — 요구사항 분석서 (Native App)

## 1. 개요

- **기능명**: HTTPS 보안 표시
- **기능 ID**: F-14
- **목적**: 사용자가 접속 중인 웹사이트의 보안 상태를 시각적으로 확인하고, 안전하지 않은 사이트 접속 시 경고를 받을 수 있도록 한다.
- **대상 사용자**: webOS 프로젝터로 웹 브라우징을 하는 모든 사용자 (특히 개인정보 입력이 필요한 사이트 이용 시)
- **요청 배경**:
  - webOS 프로젝터는 개인 정보 입력이 가능한 브라우징 환경이므로, 보안 연결 여부를 명확히 표시하여 사용자의 보안 인식을 강화해야 한다.
  - 특히 리모컨 기반 UX에서는 URL을 자세히 읽지 않고 조작하는 경우가 많아, 보안 상태를 즉시 식별할 수 있는 시각적 UI가 필요하다.
  - 피싱 사이트나 안전하지 않은 HTTP 연결에서 개인 정보를 입력하는 것을 방지한다.
  - 대화면 프로젝터 환경 (3m 거리)에서 보안 상태가 명확히 보여야 한다.
  - **Native App 특성**: Qt/C++ 기반으로 Qt WebEngineView의 인증서 정보와 URL 프로토콜을 직접 접근 가능

---

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: HTTPS 보안 아이콘 표시
- **설명**: URLBar에 현재 페이지의 보안 상태를 나타내는 아이콘을 표시한다.
- **유저 스토리**: As a 사용자, I want URL 입력창 옆에 자물쇠 아이콘을 보고, so that 내가 접속 중인 사이트가 보안 연결인지 즉시 알 수 있다.
- **우선순위**: Must
- **세부 요구사항**:
  - **보안 등급 분류**:
    - **HTTPS (Secure)**: 녹색 자물쇠 아이콘 + "보안 연결" 툴팁
    - **HTTP (Insecure)**: 경고 아이콘 (⚠️) + "안전하지 않음" 툴팁
    - **localhost / file://**: 정보 아이콘 (ℹ️) + "로컬 연결" 툴팁
    - **Unknown**: 아이콘 없음 (초기 상태, about:blank 등)
  - **구현 방식**:
    - SecurityIndicator 컴포넌트: Qt QWidget 또는 QLabel 기반
    - URLBar 좌측에 아이콘 배치 (QHBoxLayout으로 구성)
    - WebView의 `urlChanged(const QUrl &url)` 시그널 연결
    - QUrl::scheme()으로 프로토콜 체크 (https, http, file, 기타)
    - 아이콘 리소스: resources/icons/lock_secure.png, lock_insecure.png, info.png
  - **동작 방식**:
    - WebView의 `urlChanged` 시그널 발생 시 URLBar가 SecurityIndicator를 업데이트
    - QUrl::scheme() 체크:
      - `"https"` → Secure 아이콘
      - `"http"` → Insecure 아이콘 (단, localhost 예외)
      - `"file"` → Local 아이콘
      - 기타 → Unknown (아이콘 숨김)
    - 리모컨 포커스 시 Qt Tooltip으로 보안 상태 텍스트 표시 (setToolTip)
  - **스타일링**:
    - 아이콘 크기: 32x32px (3m 거리 가독성)
    - QSS로 색상 및 여백 설정
    - 포커스 시 테두리 강조 (3px solid #00aaff)

### FR-2: 비보안 사이트 접속 경고 다이얼로그
- **설명**: 사용자가 HTTP 사이트로 처음 이동할 때 경고 다이얼로그를 표시한다.
- **유저 스토리**: As a 사용자, I want 안전하지 않은 사이트에 접속할 때 경고를 받아서, so that 개인 정보를 입력하기 전에 보안 위험을 인지할 수 있다.
- **우선순위**: Must
- **세부 요구사항**:
  - **경고 조건**:
    - URL 프로토콜이 `http://`로 시작하는 사이트로 이동할 때
    - 예외: `localhost`, `127.0.0.1`, `192.168.*.*`, `10.*.*.*` 등 로컬/사설 IP는 경고하지 않음
  - **다이얼로그 구현**:
    - Qt QMessageBox 또는 커스텀 QDialog 사용
    - 타이틀: "안전하지 않은 사이트"
    - 메시지: "이 사이트는 보안 연결을 사용하지 않습니다.\n개인 정보(비밀번호, 신용카드 번호 등)를 입력하지 마세요."
    - 버튼:
      - "계속 진행" (기본 포커스, Qt::AcceptRole): 경고를 무시하고 사이트 로드
      - "돌아가기" (Qt::RejectRole): WebView::goBack() 호출
    - 체크박스 (선택사항): "이 세션 동안 이 사이트에 대해 다시 표시하지 않기"
  - **무시 도메인 목록 관리**:
    - 체크박스 선택 시 해당 도메인을 메모리에 저장 (QSet<QString> 또는 std::unordered_set)
    - 세션 종료 시 초기화 (영구 저장하지 않음)
    - 최대 100개 도메인 저장 (메모리 제한)
  - **동작 방식**:
    - WebView의 `urlChanged` 시그널 발생 시:
      1. QUrl::scheme() 체크 → `"http"`인지 확인
      2. QUrl::host()로 도메인 추출 → 로컬 IP 패턴 체크 (정규표현식 또는 문자열 비교)
      3. 무시 도메인 목록에 없으면 다이얼로그 표시
    - 다이얼로그 표시 중 WebView 로딩 일시 중지 (선택적, 기술적 제약 확인 필요)
    - "계속 진행" 선택 시 로딩 재개

### FR-3: URL 프로토콜 자동 감지 및 분류
- **설명**: 웹 페이지 URL을 분석하여 보안 등급을 자동으로 분류한다.
- **유저 스토리**: As a 개발자, I want URL 프로토콜을 자동으로 감지하여, so that 보안 아이콘과 경고를 일관되게 표시할 수 있다.
- **우선순위**: Must
- **세부 요구사항**:
  - **보안 등급 enum 정의**:
    ```cpp
    namespace webosbrowser {

    enum class SecurityLevel {
        Secure,    // https://
        Insecure,  // http:// (localhost 제외)
        Local,     // http://localhost, file://, 사설 IP
        Unknown    // about:blank, 빈 URL 등
    };

    } // namespace webosbrowser
    ```
  - **분류 유틸리티 함수**:
    - 위치: `src/utils/SecurityClassifier.h`, `src/utils/SecurityClassifier.cpp`
    - 함수 시그니처:
      ```cpp
      namespace webosbrowser {

      class SecurityClassifier {
      public:
          static SecurityLevel classify(const QUrl &url);
          static bool isLocalAddress(const QString &host);
          static QString securityLevelToString(SecurityLevel level);
          static QString securityLevelToIconPath(SecurityLevel level);
      };

      } // namespace webosbrowser
      ```
  - **분류 로직**:
    - `QUrl::scheme()` 체크:
      - `"https"` → Secure
      - `"http"` → Insecure (단, host가 localhost/사설IP면 Local)
      - `"file"` → Local
      - 빈 문자열 또는 null → Unknown
      - 기타 (`about`, `chrome`, `qrc`) → Unknown
    - 로컬 주소 판별 로직 (`isLocalAddress`):
      - `"localhost"`, `"127.0.0.1"` → true
      - 정규표현식: `192.168.\d+.\d+`, `10.\d+.\d+.\d+`, `172.(1[6-9]|2\d|3[01]).\d+.\d+` (사설 IP 범위)
      - QRegularExpression 사용
  - **예외 처리**:
    - 빈 URL, null URL → Unknown
    - QUrl 파싱 실패 (QUrl::isValid() == false) → Unknown

### FR-4: 보안 상태 실시간 업데이트
- **설명**: 페이지 이동 시 보안 아이콘이 실시간으로 업데이트된다.
- **유저 스토리**: As a 사용자, I want 페이지를 이동할 때마다 보안 아이콘이 즉시 바뀌어서, so that 현재 페이지의 보안 상태를 항상 정확히 알 수 있다.
- **우선순위**: Must
- **세부 요구사항**:
  - **이벤트 연결**:
    - WebView의 `urlChanged(const QUrl &url)` 시그널을 URLBar의 슬롯에 연결
    - URLBar는 SecurityIndicator 컴포넌트를 갱신
  - **응답 속도**:
    - URL 변경 감지 후 500ms 이내에 보안 아이콘 업데이트 (Qt 이벤트 루프는 즉시 처리하므로 일반적으로 50ms 이내)
    - 페이지 로딩 완료를 기다리지 않음 (loadFinished 시그널 불필요)
  - **UI 갱신**:
    - SecurityIndicator::updateSecurityLevel(const QUrl &url) 메서드 호출
    - QLabel::setPixmap()으로 아이콘 이미지 변경
    - QLabel::setToolTip()으로 툴팁 갱신
  - **리다이렉트 처리**:
    - HTTP → HTTPS 자동 리다이렉트 시 중간 HTTP 상태를 일시적으로 표시할 수 있음
    - 최종 URL에서 아이콘이 정확히 반영되면 문제없음

### FR-5: 보안 상태 정보 다이얼로그 (선택적)
- **설명**: 보안 아이콘 클릭 시 상세 보안 정보를 표시하는 다이얼로그를 제공한다.
- **유저 스토리**: As a 사용자, I want 자물쇠 아이콘을 클릭하여, so that 현재 사이트의 인증서 정보나 보안 수준을 자세히 확인할 수 있다.
- **우선순위**: Could (M3 범위 외, 향후 구현 가능)
- **세부 요구사항**:
  - **다이얼로그 내용**:
    - 보안 수준: "보안 연결" / "안전하지 않음" / "로컬 연결"
    - URL: 전체 URL 표시 (QUrl::toString())
    - 프로토콜: HTTPS / HTTP / file://
    - 인증서 정보 (Qt WebEngineView 지원 시):
      - 발급 기관 (Issuer)
      - 유효 기간 (Valid From ~ Valid To)
      - 암호화 수준 (TLS 버전)
  - **구현 방식**:
    - SecurityIndicator를 클릭 가능한 QPushButton으로 구현 (또는 QLabel + mousePressEvent)
    - 클릭 시 SecurityDetailDialog (QDialog) 표시
    - Qt WebEngineView의 인증서 정보 접근 (QWebEngineCertificateError, 제한적)
  - **제약사항**:
    - Qt WebEngineView는 인증서 세부 정보 접근이 제한적일 수 있음
    - webOS Native WebView API의 인증서 접근 기능 확인 필요
    - 접근 불가 시 "정보 없음" 표시

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### NFR-1: 성능
- **반응 속도**: URL 변경 감지 후 500ms 이내에 보안 아이콘 업데이트
  - Qt 시그널/슬롯은 일반적으로 50ms 이내 처리 (동일 스레드)
  - 아이콘 파일 사전 로드로 I/O 지연 제거 (QPixmap 캐싱)
- **UI 렌더링**: 아이콘 표시로 인한 URLBar 레이아웃 시프트 없음
  - 아이콘 영역 고정 너비 확보 (32px + 여백)
  - QHBoxLayout에서 아이콘 QLabel의 고정 크기 설정 (setFixedSize)
- **메모리**: 경고 무시 도메인 목록을 메모리에 저장 (세션당 최대 100개 제한)
  - QSet<QString> 또는 std::unordered_set<std::string> 사용
  - 메모리 사용량: 약 10KB 이하

### NFR-2: 사용성
- **일관성**: 보안 아이콘은 항상 URLBar 좌측 고정 위치에 표시
  - QHBoxLayout에서 아이콘을 첫 번째 위젯으로 배치
- **가독성**: 프로젝터 대화면 (3m 거리)에서 아이콘이 명확히 식별 가능
  - 아이콘 크기: 최소 32x32px (권장 48x48px)
  - 폰트 크기 (툴팁): 최소 16px
  - 색상 대비: 배경과 4.5:1 이상 (WCAG AA 기준)
- **접근성**: 리모컨 포커스로 아이콘에 접근 가능
  - SecurityIndicator를 포커스 가능한 위젯으로 구현 (setFocusPolicy(Qt::StrongFocus))
  - 포커스 시 툴팁 자동 표시 (QToolTip::showText)
  - 리모컨 Enter 키로 상세 정보 다이얼로그 열기 (FR-5 구현 시)

### NFR-3: 보안
- **보안 정책**: HTTP 사이트 경고는 사용자가 명시적으로 무시하지 않는 한 항상 표시
  - 무시 옵션은 세션 단위만 (영구 저장 금지)
  - 앱 재시작 시 무시 목록 초기화
- **개인 정보 보호**: 경고 무시 도메인 목록은 세션 메모리에만 저장
  - 로컬 파일 또는 LS2 API 저장소에 저장하지 않음
  - QSettings 사용 금지 (영구 저장 방지)

### NFR-4: 확장성
- **보안 등급 추가**: SecurityLevel enum에 새로운 등급 추가 가능
  - 예: `MixedContent` (HTTPS 페이지 내 HTTP 리소스)
  - 예: `CertificateError` (인증서 오류)
- **아이콘 테마**: 라이트/다크 테마에 따라 아이콘 자동 변경
  - 아이콘 리소스 디렉토리 구조: resources/icons/light/, resources/icons/dark/
  - QSS 또는 런타임 QIcon::fromTheme() 사용

---

## 4. 제약조건

### TC-1: Qt WebEngineView 인증서 정보 제약
- **설명**: Qt WebEngineView는 인증서 세부 정보 접근이 제한적이다.
- **영향**:
  - SSL 인증서 발급 기관, 유효 기간 등 세부 정보 접근 어려움
  - QWebEngineCertificateError는 에러 발생 시에만 사용 가능
  - 정상 HTTPS 연결 시 인증서 정보 조회 API 부족 (Qt 6.x에서 일부 개선)
- **대응 방안**:
  - FR-5 (상세 정보 다이얼로그)는 선택적 기능으로 구현
  - 프로토콜 기반 보안 수준 분류에 집중 (FR-1, FR-3)
  - webOS Native WebView API의 인증서 접근 기능 추가 조사

### TC-2: webOS 리모컨 UX 제약
- **설명**: 리모컨 기반 UX에서는 마우스 호버, 우클릭 메뉴 등을 사용할 수 없다.
- **영향**: 보안 아이콘 호버 시 상세 정보 표시 불가
- **대응 방안**:
  - 리모컨 포커스 시 툴팁으로 간략한 정보 표시 (QToolTip)
  - 리모컨 Enter 키로 상세 정보 다이얼로그 열기 (FR-5 구현 시)
  - keyPressEvent 오버라이드로 Qt::Key_Enter 처리

### TC-3: HTTP → HTTPS 자동 리다이렉트 감지
- **설명**: 일부 사이트는 HTTP 요청을 HTTPS로 자동 리다이렉트한다.
- **영향**: HTTP 경고 다이얼로그가 잠깐 표시된 후 HTTPS로 전환될 수 있음
- **대응 방안**:
  - urlChanged 시그널이 여러 번 발생할 수 있으므로, 최종 URL만 처리
  - 경고 다이얼로그 표시 전 500ms 디바운싱 (QTimer::singleShot)
  - HTTPS로 리다이렉트되면 경고 자동 닫기

### TC-4: 로컬 IP 패턴 정확도
- **설명**: 사설 IP 범위 패턴을 정확히 판별해야 한다.
- **영향**: 사설 IP를 공인 IP로 잘못 판단 시 불필요한 경고 표시
- **대응 방안**:
  - 정확한 정규표현식 패턴 사용 (RFC 1918 사설 IP 범위)
    - 10.0.0.0/8: `^10\.\d{1,3}\.\d{1,3}\.\d{1,3}$`
    - 172.16.0.0/12: `^172\.(1[6-9]|2\d|3[01])\.\d{1,3}\.\d{1,3}$`
    - 192.168.0.0/16: `^192\.168\.\d{1,3}\.\d{1,3}$`
  - localhost 별칭: `localhost`, `127.0.0.1`, `::1`

---

## 5. 용어 정의

| 용어 | 정의 |
|------|------|
| **HTTPS** | HTTP over SSL/TLS. 암호화된 보안 연결을 사용하는 프로토콜 |
| **SSL/TLS** | Secure Sockets Layer / Transport Layer Security. 웹 통신을 암호화하는 프로토콜 |
| **SecurityIndicator** | URLBar에 보안 상태 아이콘을 표시하는 Qt 위젯 (QLabel 또는 QPushButton 기반) |
| **SecurityLevel** | URL 프로토콜 기반 보안 등급 enum (Secure, Insecure, Local, Unknown) |
| **SecurityClassifier** | URL을 분석하여 SecurityLevel을 반환하는 유틸리티 클래스 |
| **QUrl** | Qt의 URL 파싱 및 조작 클래스 (프로토콜, 호스트, 경로 추출) |
| **QMessageBox** | Qt의 모달 다이얼로그 위젯 (경고, 확인, 정보 표시) |
| **QToolTip** | Qt의 툴팁 표시 클래스 (위젯 위에 마우스 호버 또는 포커스 시 표시) |
| **사설 IP** | 인터넷에 직접 노출되지 않는 로컬 네트워크 IP (10.x, 172.16~31.x, 192.168.x) |
| **디바운싱** | 연속된 이벤트를 지연시켜 마지막 이벤트만 처리하는 기법 (QTimer::singleShot) |

---

## 6. 범위 외 (Out of Scope)

### 6.1. M3 범위 외 (향후 구현 고려)
- **FR-5**: 보안 아이콘 클릭 시 상세 정보 다이얼로그
  - 이유: M3 핵심 기능이 아니며, Qt WebEngineView 인증서 접근 제약
  - 향후 Qt 6.x 업그레이드 또는 webOS Native API로 구현 가능

### 6.2. 기술적 제약으로 구현 불가 (Qt 5.15 기준)
- **SSL 인증서 세부 정보 표시**: 인증 기관, 유효 기간, 암호화 수준 등
  - 이유: Qt 5.15의 QWebEngineView는 인증서 세부 정보 접근 API 제한적
  - 대안: Qt 6.x의 QWebEngineCertificateError 확장 기능 활용 (추후)
- **Mixed Content 감지 및 경고**:
  - 이유: Qt WebEngineView는 페이지 내부 리소스 로드 상태를 외부에 노출하지 않음
  - 범위 외로 명시
- **HSTS 자동 업그레이드**:
  - 이유: Qt WebEngineView는 HSTS 헤더를 자동 처리 (브라우저 엔진 내부)
  - URL 입력 시 자동으로 https:// 우선 시도 (F-03 URLBar에서 처리)
- **인증서 오류 세부 분류** (만료, 자체 서명, 도메인 불일치 등):
  - 이유: QWebEngineCertificateError는 오류 타입을 제공하지만 정상 연결 시 접근 불가
  - 범위 외로 명시

### 6.3. 사용자 설정 제외
- **보안 경고 완전 비활성화 옵션**:
  - 이유: 사용자 보안을 위해 경고는 항상 표시되어야 함
  - 무시 옵션은 세션 단위로만 제공 (영구 저장 금지)

---

## 7. 수용 기준 (Acceptance Criteria)

### AC-1: HTTPS 사이트 아이콘 표시
- **Given**: 사용자가 `https://www.google.com`에 접속
- **When**: 페이지가 로드됨 (WebView::urlChanged 시그널 발생)
- **Then**:
  - URLBar 좌측에 녹색 자물쇠 아이콘 표시
  - 아이콘에 포커스 시 "보안 연결" 툴팁 표시

### AC-2: HTTP 사이트 경고 다이얼로그
- **Given**: 사용자가 `http://example.com`에 접속
- **When**: URL이 입력되거나 링크를 클릭 (WebView::urlChanged 시그널 발생)
- **Then**:
  - 경고 QMessageBox 표시: "안전하지 않은 사이트"
  - "계속 진행" 버튼 선택 시 페이지 로드 계속, URLBar에 경고 아이콘 (⚠️) 표시
  - "돌아가기" 버튼 선택 시 WebView::goBack() 호출, 이전 페이지로 돌아감

### AC-3: localhost 예외 처리
- **Given**: 사용자가 `http://localhost:3000`에 접속
- **When**: 페이지가 로드됨
- **Then**:
  - 경고 다이얼로그 표시되지 않음
  - URLBar 좌측에 정보 아이콘 (ℹ️) 표시
  - 아이콘에 포커스 시 "로컬 연결" 툴팁 표시

### AC-4: 페이지 이동 시 아이콘 업데이트
- **Given**: 사용자가 `https://www.google.com`에서 `http://example.com`으로 이동
- **When**: 페이지 이동이 감지됨 (WebView::urlChanged 시그널)
- **Then**:
  - 500ms 이내에 보안 아이콘이 자물쇠 (🔒) → 경고 (⚠️)로 변경
  - 경고 다이얼로그 표시 (처음 접속 시)

### AC-5: 경고 무시 옵션 (세션 단위)
- **Given**: 사용자가 `http://example.com`에 처음 접속하여 경고 다이얼로그 표시
- **When**: "이 세션 동안 이 사이트에 대해 다시 표시하지 않기" 체크 후 "계속 진행" 선택
- **Then**:
  - 세션 동안 `http://example.com`에 재접속 시 경고 다이얼로그 표시되지 않음
  - 앱 재시작 후에는 경고 다시 표시됨 (메모리 초기화)

### AC-6: URL 프로토콜 자동 분류
- **Given**: SecurityClassifier::classify() 유틸리티 함수 호출
- **When**: 다양한 URL 입력
- **Then**:
  - `https://example.com` → SecurityLevel::Secure
  - `http://example.com` → SecurityLevel::Insecure
  - `http://localhost:3000` → SecurityLevel::Local
  - `http://192.168.1.1` → SecurityLevel::Local
  - `file:///path/to/file.html` → SecurityLevel::Local
  - `about:blank` → SecurityLevel::Unknown
  - 빈 QUrl → SecurityLevel::Unknown

### AC-7: 아이콘 툴팁 표시
- **Given**: 사용자가 URLBar의 보안 아이콘에 포커스
- **When**: 리모컨으로 포커스 이동
- **Then**:
  - HTTPS 사이트: "보안 연결" 툴팁 표시
  - HTTP 사이트: "안전하지 않음" 툴팁 표시
  - 로컬: "로컬 연결" 툴팁 표시

### AC-8: 리모컨 키 매핑
- **Given**: SecurityIndicator에 포커스
- **When**: 리모컨 Enter 키 입력 (Qt::Key_Enter)
- **Then**: (FR-5 구현 시) 상세 정보 다이얼로그 표시

---

## 8. 테스트 시나리오

### TS-1: HTTPS 사이트 보안 아이콘 표시
1. 앱 실행
2. URLBar에 `https://www.google.com` 입력 후 확인
3. 페이지 로드 완료
4. **검증**: URLBar 좌측에 녹색 자물쇠 아이콘 표시
5. 아이콘에 포커스 이동 (리모컨 방향키)
6. **검증**: "보안 연결" 툴팁 표시

### TS-2: HTTP 사이트 경고 다이얼로그
1. 앱 실행
2. URLBar에 `http://example.com` 입력 후 확인
3. **검증**: 경고 QMessageBox 표시 ("안전하지 않은 사이트")
4. "계속 진행" 버튼 선택 (리모컨 Enter)
5. 페이지 로드 완료
6. **검증**: URLBar 좌측에 경고 아이콘 (⚠️) 표시

### TS-3: localhost 예외 처리
1. 앱 실행
2. URLBar에 `http://localhost:3000` 입력 후 확인
3. **검증**: 경고 다이얼로그 표시되지 않음
4. 페이지 로드 완료
5. **검증**: URLBar 좌측에 정보 아이콘 (ℹ️) 표시

### TS-4: HTTPS → HTTP 이동 시 경고
1. 앱 실행
2. URLBar에 `https://www.google.com` 입력 후 확인
3. 페이지 로드 완료
4. URLBar에 `http://example.com` 입력 후 확인
5. **검증**: 경고 다이얼로그 표시
6. "계속 진행" 선택
7. **검증**: 보안 아이콘이 자물쇠 → 경고로 변경

### TS-5: 페이지 내 링크 클릭 시 보안 아이콘 업데이트
1. 앱 실행
2. `https://www.google.com` 접속
3. 페이지 내에서 `http://example.com` 링크 클릭 (리모컨 Enter)
4. **검증**: 500ms 이내에 보안 아이콘이 자물쇠 → 경고로 변경
5. **검증**: 경고 다이얼로그 표시

### TS-6: 경고 무시 옵션 (세션 단위)
1. 앱 실행
2. `http://example.com` 접속
3. 경고 다이얼로그에서 "이 세션 동안 이 사이트에 대해 다시 표시하지 않기" 체크
4. "계속 진행" 선택
5. 다른 페이지로 이동 후 다시 `http://example.com` 접속
6. **검증**: 경고 다이얼로그 표시되지 않음
7. 앱 재시작
8. `http://example.com` 접속
9. **검증**: 경고 다이얼로그 다시 표시됨

### TS-7: HTTP → HTTPS 자동 리다이렉트 시 경고 숨김
1. 앱 실행
2. `http://github.com` 접속 (실제로는 `https://github.com`으로 자동 리다이렉트됨)
3. **검증**: 경고 다이얼로그가 표시되더라도 500ms 내에 자동으로 닫힘 (또는 표시되지 않음)
4. 최종 페이지 로드 완료
5. **검증**: URLBar에 `https://github.com` 표시, 녹색 자물쇠 아이콘 표시

### TS-8: URL 분류 유틸리티 단위 테스트
1. SecurityClassifier 단위 테스트 실행 (Google Test)
2. **검증**: 다음 URL 분류 결과 확인
   - `https://example.com` → Secure
   - `http://example.com` → Insecure
   - `http://localhost` → Local
   - `http://127.0.0.1:8080` → Local
   - `http://192.168.1.1` → Local
   - `http://10.0.0.1` → Local
   - `file:///path/to/file.html` → Local
   - `about:blank` → Unknown
   - 빈 QUrl → Unknown

### TS-9: 사설 IP 패턴 정확도 테스트
1. SecurityClassifier::isLocalAddress() 테스트
2. **검증**: 다음 IP 판별 결과 확인
   - `192.168.0.1` → true
   - `10.1.1.1` → true
   - `172.16.0.1` → true
   - `172.32.0.1` → false (사설 IP 범위 밖)
   - `8.8.8.8` → false (공인 IP)
   - `localhost` → true
   - `127.0.0.1` → true

---

## 9. 우선순위 및 구현 순서

### Phase 1: 보안 등급 분류 (High Priority)
- **FR-3**: URL 프로토콜 자동 감지 및 분류
- **산출물**:
  - `src/utils/SecurityClassifier.h`
  - `src/utils/SecurityClassifier.cpp`
  - SecurityLevel enum 정의
  - classify(), isLocalAddress() 메서드 구현
- **테스트**: TS-8, TS-9 (단위 테스트)

### Phase 2: 보안 아이콘 표시 (High Priority)
- **FR-1**: HTTPS 보안 아이콘 표시
- **FR-4**: 보안 상태 실시간 업데이트
- **산출물**:
  - `src/ui/SecurityIndicator.h`
  - `src/ui/SecurityIndicator.cpp`
  - SecurityIndicator 컴포넌트 (QLabel 또는 QWidget 기반)
  - URLBar 컴포넌트 확장 (SecurityIndicator 통합)
  - 아이콘 리소스 (resources/icons/lock_secure.png, lock_insecure.png, info.png)
- **연동**: WebView::urlChanged 시그널 연결
- **테스트**: TS-1, TS-3, TS-4, TS-5

### Phase 3: 경고 다이얼로그 (High Priority)
- **FR-2**: 비보안 사이트 접속 경고 다이얼로그
- **산출물**:
  - `src/ui/SecurityWarningDialog.h` (선택적, QMessageBox로 구현 가능)
  - `src/ui/SecurityWarningDialog.cpp`
  - 경고 다이얼로그 로직 (BrowserWindow 또는 URLBar에 구현)
  - 무시 도메인 목록 관리 (QSet<QString> 멤버 변수)
- **테스트**: TS-2, TS-4, TS-6, TS-7

### Phase 4: 테스트 및 검증 (Must)
- **AC-1 ~ AC-8**: 수용 기준 테스트
- **TS-1 ~ TS-9**: 테스트 시나리오 실행
- **회귀 테스트**: F-02, F-03, F-04와 통합 테스트

### Phase 5: 상세 정보 다이얼로그 (Low Priority, 선택적)
- **FR-5**: 보안 상태 정보 다이얼로그 (M3 범위 외)
- **산출물**:
  - `src/ui/SecurityDetailDialog.h`
  - `src/ui/SecurityDetailDialog.cpp`
  - 인증서 정보 표시 (Qt WebEngineView API 제약 확인 후 구현)
- **테스트**: AC-8 (리모컨 Enter 키)

---

## 10. 의존성

### 선행 기능
- **F-02 (웹뷰 통합)**: WebView 컴포넌트의 `urlChanged(const QUrl &url)` 시그널 필수
- **F-03 (URL 입력 UI)**: URLBar 컴포넌트 확장 필요

### 연동 컴포넌트
- **WebView** (`src/browser/WebView.h`):
  - `urlChanged(const QUrl &url)` 시그널
  - `goBack()` 메서드 (경고 다이얼로그 "돌아가기" 버튼)
- **URLBar** (`src/ui/URLBar.h`):
  - SecurityIndicator 컴포넌트 통합 (QHBoxLayout)
  - urlChanged 시그널 구독하여 SecurityIndicator 업데이트
- **BrowserWindow** (`src/browser/BrowserWindow.h`):
  - 경고 다이얼로그 상태 관리 (또는 URLBar에서 관리)
  - 무시 도메인 목록 저장 (QSet<QString> 멤버 변수)

### 외부 라이브러리
- **Qt Core**: QUrl, QString, QSet, QRegularExpression
- **Qt Widgets**: QLabel, QPushButton, QMessageBox, QToolTip, QDialog
- **Qt Gui**: QPixmap, QIcon
- **없음**: 외부 SSL 검증 라이브러리 불필요 (Qt WebEngine 내장 기능 사용)

---

## 11. 리스크 및 대응 방안

| 리스크 | 영향도 | 확률 | 대응 방안 |
|--------|--------|------|-----------|
| Qt WebEngineView 인증서 정보 접근 제약 | High | 확정 | FR-5 (상세 정보 다이얼로그)를 선택적 기능으로 처리. 프로토콜 기반 보안 수준 분류에 집중 (FR-1, FR-3) |
| HTTP → HTTPS 리다이렉트 시 경고 깜박임 | Medium | Medium | 경고 다이얼로그 500ms 디바운싱 (QTimer::singleShot). 최종 URL이 HTTPS면 경고 자동 닫기 |
| 사용자가 경고를 무시하고 HTTP 사이트 사용 | Low | High | 세션 단위 무시 옵션 제공. 앱 재시작 시 초기화하여 보안 의식 유지 |
| 사설 IP 패턴 정규표현식 오류 | Medium | Low | RFC 1918 사설 IP 범위를 정확히 구현. 단위 테스트로 검증 (TS-9) |
| 리모컨 UX에서 보안 정보 접근 어려움 | Low | Medium | 툴팁으로 간략한 정보 제공. 상세 정보는 리모컨 Enter 키로 다이얼로그 열기 (FR-5 구현 시) |
| URLBar 레이아웃 시프트로 인한 UX 저하 | Low | Low | SecurityIndicator 영역 고정 너비 확보 (setFixedSize). QHBoxLayout에서 아이콘 첫 번째 배치 |

---

## 12. 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|-----------|--------|
| 2026-02-14 | 2.0 | Native App (Qt/C++) 관점으로 요구사항 재작성 | product-manager |

---

## 13. 검토 및 승인

| 역할 | 이름 | 승인 여부 | 날짜 |
|------|------|-----------|------|
| Product Manager | - | 승인 | 2026-02-14 |
| Architect | - | 대기 | - |
| C++ Developer | - | 대기 | - |
