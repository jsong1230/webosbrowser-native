# F-14: HTTPS 보안 표시 — 기술 설계서 (Native App)

## 1. 참조
- 요구사항 분석서: docs/specs/https-security/requirements.md

---

## 2. 아키텍처 개요

### 2.1. 컴포넌트 구조

```
BrowserWindow
│
├── URLBar (확장)
│   └── SecurityIndicator (신규 QLabel) ← 보안 아이콘 표시
│       - QPixmap 기반 아이콘
│       - QToolTip으로 상태 표시
│       - 포커스 가능 (리모컨 접근)
│
├── WebView (기존)
│   └── urlChanged(const QUrl &url) 시그널
│
└── SecurityWarningDialog (신규 QMessageBox/QDialog)
    - HTTP 경고 표시
    - "계속 진행" / "돌아가기" 버튼
    - 무시 옵션 체크박스 (선택적)
```

### 2.2. 데이터 흐름

```
WebView::urlChanged(const QUrl &url) 시그널
    ↓
BrowserWindow::onUrlChanged(const QUrl &url) 슬롯
    ↓ SecurityClassifier::classify(url) 호출
    ├─→ SecurityLevel 반환 (Secure/Insecure/Local/Unknown)
    │
    ├─→ URLBar::updateSecurityIndicator(SecurityLevel level)
    │       ↓ SecurityIndicator::setSecurityLevel(level)
    │       ↓ QLabel::setPixmap() (아이콘 변경)
    │       ↓ QLabel::setToolTip() (툴팁 갱신)
    │
    └─→ BrowserWindow::checkHttpWarning(const QUrl &url)
            ↓ SecurityClassifier::classify(url) == Insecure
            ↓ SecurityClassifier::isLocalAddress(host) == false
            ↓ ignoredDomains_ (QSet<QString>)에 없음
            ↓
        SecurityWarningDialog 표시 (QMessageBox::warning)
            ├─ "계속 진행" → 무시 도메인 추가 (선택 시)
            └─ "돌아가기" → WebView::goBack()
```

### 2.3. 상태 관리

| 상태 | 위치 | 타입 | 설명 |
|------|------|------|------|
| `currentSecurityLevel_` | URLBar (멤버 변수) | SecurityLevel | 현재 URL의 보안 등급 |
| `ignoredDomains_` | BrowserWindow (멤버 변수) | QSet\<QString\> | 경고 무시 도메인 목록 (세션 단위) |
| `warningTimer_` | BrowserWindow (멤버 변수) | QTimer* | 경고 디바운싱 타이머 (500ms) |
| `pendingUrl_` | BrowserWindow (멤버 변수) | QUrl | 경고 대기 중인 URL |

---

## 3. 아키텍처 결정

### 결정 1: 보안 등급 분류 방식
- **선택지**:
  - A) QUrl::scheme() 기반 프로토콜 분류 (https, http, file)
  - B) Qt WebEngineView의 인증서 정보 접근 (QWebEngineCertificateError)
  - C) 외부 SSL 검증 라이브러리 (OpenSSL)
- **결정**: A) QUrl::scheme() 기반 프로토콜 분류
- **근거**:
  - Qt 5.15의 QWebEngineView는 인증서 세부 정보 접근이 제한적 (에러 발생 시만 가능)
  - 정상 HTTPS 연결 시 인증서 정보 조회 API 부족
  - 외부 OpenSSL은 Qt WebEngine 내부 인증서와 동기화 어려움
  - 프로토콜 기반 분류는 빠르고 안정적 (대부분의 브라우저가 사용)
- **트레이드오프**:
  - 포기: 인증서 상세 정보 (발급 기관, 유효 기간, TLS 버전), Mixed Content 감지
  - 얻음: 즉시 분류 (0ms 응답), Qt WebEngine 독립적, 간단한 구현

### 결정 2: SecurityIndicator 구현 방식
- **선택지**:
  - A) QLabel 기반 (QPixmap 아이콘 + setToolTip)
  - B) QPushButton 기반 (클릭 가능, 상세 정보 다이얼로그 연결)
  - C) QWidget 기반 커스텀 위젯 (paintEvent 오버라이드)
- **결정**: A) QLabel 기반 (QPixmap 아이콘 + setToolTip)
- **근거**:
  - M3 범위에서는 아이콘 표시와 툴팁만 필요 (클릭 동작 불필요)
  - QLabel은 가볍고 포커스 지원 가능 (setFocusPolicy(Qt::StrongFocus))
  - 향후 QPushButton으로 전환 시 상속 구조 유지 가능
- **트레이드오프**:
  - 포기: 클릭 시 상세 정보 다이얼로그 (FR-5, M3 범위 외)
  - 얻음: 간단한 구현, 낮은 메모리 사용, 빠른 렌더링

### 결정 3: 경고 다이얼로그 표시 타이밍
- **선택지**:
  - A) urlChanged 시그널 즉시 표시
  - B) 500ms 디바운싱 후 표시 (QTimer::singleShot)
  - C) loadFinished 시그널 후 표시
- **결정**: B) 500ms 디바운싱 후 표시
- **근거**:
  - HTTP → HTTPS 자동 리다이렉트 시 중간 HTTP 상태에서 불필요한 경고 방지
  - urlChanged 시그널은 리다이렉트마다 여러 번 발생 가능
  - loadFinished까지 대기하면 사용자가 이미 페이지와 상호작용 시작
- **트레이드오프**:
  - 포기: 즉각적인 경고 (최대 500ms 지연)
  - 얻음: 자동 리다이렉트 시 경고 숨김, 사용자 경험 개선

### 결정 4: 경고 무시 도메인 저장 위치
- **선택지**:
  - A) 메모리 (QSet\<QString\>, 세션 단위)
  - B) QSettings (영구 저장)
  - C) LS2 API (webOS 로컬 DB, 영구 저장)
- **결정**: A) 메모리 (QSet\<QString\>, 세션 단위)
- **근거**:
  - 보안 경고는 앱 재시작 시 초기화되어야 사용자 안전 의식 유지
  - 영구 저장 시 사용자가 설정을 잊고 계속 HTTP 사이트 사용 위험
  - QSet\<QString\>은 빠른 조회 (O(1) 평균), 간단한 구현
- **트레이드오프**:
  - 포기: 앱 재시작 후 무시 설정 유지
  - 얻음: 보안성 향상, 매번 경고 재확인으로 안전 의식 강화

### 결정 5: URLBar 레이아웃 통합 방식
- **선택지**:
  - A) QHBoxLayout에 SecurityIndicator 추가 (inputLayout_)
  - B) URLBar 외부 독립 위젯으로 배치
  - C) QLineEdit의 setClearButtonEnabled 활용 (아이콘 영역 커스텀)
- **결정**: A) QHBoxLayout에 SecurityIndicator 추가
- **근거**:
  - 기존 URLBar는 QHBoxLayout (inputLayout_) 사용 중
  - SecurityIndicator를 첫 번째 위젯으로 추가하면 좌측 배치 보장
  - QLineEdit의 clear button 영역은 우측 전용 (좌측 커스텀 불가)
- **트레이드오프**:
  - 포기: QLineEdit 내장 아이콘 기능 활용
  - 얻음: 명확한 레이아웃 제어, 아이콘 크기/여백 자유 조정

---

## 4. 클래스 설계

### 4.1. SecurityLevel Enum (신규)

**파일**: `src/utils/SecurityClassifier.h`

```cpp
namespace webosbrowser {

/**
 * @enum SecurityLevel
 * @brief URL 보안 등급
 */
enum class SecurityLevel {
    Secure,    ///< HTTPS 보안 연결
    Insecure,  ///< HTTP 비보안 연결 (localhost 제외)
    Local,     ///< localhost, file://, 사설 IP
    Unknown    ///< about:blank, 빈 URL, 기타 프로토콜
};

} // namespace webosbrowser
```

### 4.2. SecurityClassifier 클래스 (신규)

**파일**: `src/utils/SecurityClassifier.h`, `src/utils/SecurityClassifier.cpp`

```cpp
namespace webosbrowser {

/**
 * @class SecurityClassifier
 * @brief URL 보안 등급 분류 유틸리티
 *
 * QUrl을 분석하여 SecurityLevel을 반환합니다.
 * 정적 메서드만 제공 (인스턴스 생성 불필요).
 */
class SecurityClassifier {
public:
    /**
     * @brief URL 보안 등급 분류
     * @param url 분류할 URL
     * @return SecurityLevel (Secure/Insecure/Local/Unknown)
     */
    static SecurityLevel classify(const QUrl &url);

    /**
     * @brief 로컬 주소 판별 (localhost, 사설 IP)
     * @param host 호스트명 또는 IP 주소
     * @return true면 로컬 주소
     */
    static bool isLocalAddress(const QString &host);

    /**
     * @brief SecurityLevel을 문자열로 변환 (디버깅용)
     * @param level SecurityLevel
     * @return "Secure", "Insecure", "Local", "Unknown"
     */
    static QString securityLevelToString(SecurityLevel level);

    /**
     * @brief SecurityLevel을 툴팁 텍스트로 변환
     * @param level SecurityLevel
     * @return "보안 연결", "안전하지 않음", "로컬 연결", ""
     */
    static QString securityLevelToTooltip(SecurityLevel level);

    /**
     * @brief SecurityLevel을 아이콘 파일 경로로 변환
     * @param level SecurityLevel
     * @return ":/icons/lock_secure.png", ":/icons/lock_insecure.png", etc.
     */
    static QString securityLevelToIconPath(SecurityLevel level);

private:
    // 사설 IP 패턴 (정규표현식)
    static const QRegularExpression privateIpPattern_;

    // 생성자 삭제 (정적 클래스)
    SecurityClassifier() = delete;
};

} // namespace webosbrowser
```

### 4.3. SecurityIndicator 클래스 (신규)

**파일**: `src/ui/SecurityIndicator.h`, `src/ui/SecurityIndicator.cpp`

```cpp
namespace webosbrowser {

/**
 * @class SecurityIndicator
 * @brief 보안 상태 아이콘 표시 위젯
 *
 * QLabel 기반으로 SecurityLevel에 따라 아이콘과 툴팁을 표시합니다.
 * 리모컨 포커스 가능 (Qt::StrongFocus).
 */
class SecurityIndicator : public QLabel {
    Q_OBJECT

public:
    explicit SecurityIndicator(QWidget *parent = nullptr);
    ~SecurityIndicator() override;

    SecurityIndicator(const SecurityIndicator&) = delete;
    SecurityIndicator& operator=(const SecurityIndicator&) = delete;

    /**
     * @brief 보안 등급 설정 (아이콘 및 툴팁 갱신)
     * @param level SecurityLevel
     */
    void setSecurityLevel(SecurityLevel level);

    /**
     * @brief 현재 보안 등급 반환
     * @return SecurityLevel
     */
    SecurityLevel securityLevel() const;

signals:
    /**
     * @brief 아이콘 클릭 시그널 (향후 FR-5 상세 정보 다이얼로그용)
     */
    void clicked();

protected:
    /**
     * @brief 마우스 클릭 이벤트 (향후 FR-5)
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 리모컨 Enter 키 이벤트 (향후 FR-5)
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 포커스 인 이벤트 (툴팁 표시)
     */
    void focusInEvent(QFocusEvent *event) override;

private:
    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 스타일 적용 (QSS)
     */
    void applyStyles();

    /**
     * @brief 아이콘 캐시 로드 (성능 최적화)
     */
    void loadIconCache();

private:
    SecurityLevel currentLevel_;     ///< 현재 보안 등급
    QPixmap secureIcon_;             ///< Secure 아이콘 (캐시)
    QPixmap insecureIcon_;           ///< Insecure 아이콘 (캐시)
    QPixmap localIcon_;              ///< Local 아이콘 (캐시)
};

} // namespace webosbrowser
```

### 4.4. URLBar 클래스 확장 (기존 코드 수정)

**파일**: `src/ui/URLBar.h`, `src/ui/URLBar.cpp`

**변경 내용**:
1. SecurityIndicator 멤버 변수 추가
2. QHBoxLayout (inputLayout_)에 SecurityIndicator 추가
3. updateSecurityIndicator(SecurityLevel level) 메서드 추가

```cpp
// URLBar.h에 추가
namespace webosbrowser {

class URLBar : public QWidget {
    Q_OBJECT

public:
    // ... (기존 메서드)

    /**
     * @brief 보안 아이콘 업데이트
     * @param level SecurityLevel
     */
    void updateSecurityIndicator(SecurityLevel level);

private:
    // ... (기존 멤버 변수)
    SecurityIndicator *securityIndicator_;  ///< 보안 상태 아이콘 (신규)
};

} // namespace webosbrowser
```

### 4.5. BrowserWindow 클래스 확장 (기존 코드 수정)

**파일**: `src/browser/BrowserWindow.h`, `src/browser/BrowserWindow.cpp`

**변경 내용**:
1. ignoredDomains_ (QSet\<QString\>) 멤버 변수 추가
2. warningTimer_ (QTimer*) 멤버 변수 추가
3. pendingUrl_ (QUrl) 멤버 변수 추가
4. checkHttpWarning(const QUrl &url) 메서드 추가
5. showSecurityWarningDialog(const QUrl &url) 메서드 추가
6. onUrlChanged 슬롯 확장 (보안 아이콘 및 경고 처리)

```cpp
// BrowserWindow.h에 추가
namespace webosbrowser {

class BrowserWindow : public QMainWindow {
    Q_OBJECT

private slots:
    /**
     * @brief URL 변경 핸들러 (확장)
     * @param url 새 URL
     */
    void onUrlChanged(const QUrl &url);  // 기존 메서드 확장

    /**
     * @brief HTTP 경고 다이얼로그 표시 (디바운싱 후 호출)
     */
    void onWarningTimerTimeout();

private:
    /**
     * @brief HTTP 경고 필요 여부 체크 및 타이머 시작
     * @param url 체크할 URL
     */
    void checkHttpWarning(const QUrl &url);

    /**
     * @brief 보안 경고 다이얼로그 표시
     * @param url 경고 대상 URL
     * @return true면 계속 진행, false면 돌아가기
     */
    bool showSecurityWarningDialog(const QUrl &url);

private:
    // ... (기존 멤버 변수)

    // 보안 관련 상태 (신규)
    QSet<QString> ignoredDomains_;   ///< 경고 무시 도메인 목록 (세션 단위)
    QTimer *warningTimer_;           ///< 경고 디바운싱 타이머 (500ms)
    QUrl pendingUrl_;                ///< 경고 대기 중인 URL
};

} // namespace webosbrowser
```

---

## 5. API 설계 (클래스 메서드)

### 5.1. SecurityClassifier::classify()

```cpp
SecurityLevel SecurityClassifier::classify(const QUrl &url) {
    // 빈 URL 또는 유효하지 않은 URL
    if (url.isEmpty() || !url.isValid()) {
        return SecurityLevel::Unknown;
    }

    QString scheme = url.scheme().toLower();
    QString host = url.host().toLower();

    // HTTPS → Secure
    if (scheme == "https") {
        return SecurityLevel::Secure;
    }

    // HTTP → Insecure (단, localhost/사설IP는 Local)
    if (scheme == "http") {
        if (isLocalAddress(host)) {
            return SecurityLevel::Local;
        }
        return SecurityLevel::Insecure;
    }

    // file:// → Local
    if (scheme == "file") {
        return SecurityLevel::Local;
    }

    // about, qrc, chrome 등 → Unknown
    return SecurityLevel::Unknown;
}
```

### 5.2. SecurityClassifier::isLocalAddress()

```cpp
bool SecurityClassifier::isLocalAddress(const QString &host) {
    // localhost, 127.0.0.1, ::1
    if (host == "localhost" || host == "127.0.0.1" || host == "::1") {
        return true;
    }

    // 사설 IP 정규표현식 체크
    // 10.0.0.0/8
    QRegularExpression ip10("^10\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
    if (ip10.match(host).hasMatch()) {
        return true;
    }

    // 172.16.0.0/12
    QRegularExpression ip172("^172\\.(1[6-9]|2\\d|3[01])\\.\\d{1,3}\\.\\d{1,3}$");
    if (ip172.match(host).hasMatch()) {
        return true;
    }

    // 192.168.0.0/16
    QRegularExpression ip192("^192\\.168\\.\\d{1,3}\\.\\d{1,3}$");
    if (ip192.match(host).hasMatch()) {
        return true;
    }

    return false;
}
```

### 5.3. SecurityIndicator::setSecurityLevel()

```cpp
void SecurityIndicator::setSecurityLevel(SecurityLevel level) {
    currentLevel_ = level;

    // 아이콘 변경
    switch (level) {
        case SecurityLevel::Secure:
            setPixmap(secureIcon_);
            setToolTip("보안 연결");
            break;
        case SecurityLevel::Insecure:
            setPixmap(insecureIcon_);
            setToolTip("안전하지 않음");
            break;
        case SecurityLevel::Local:
            setPixmap(localIcon_);
            setToolTip("로컬 연결");
            break;
        case SecurityLevel::Unknown:
            clear();  // 아이콘 숨김
            setToolTip("");
            break;
    }

    qDebug() << "SecurityIndicator: 보안 등급 변경 -"
             << SecurityClassifier::securityLevelToString(level);
}
```

### 5.4. URLBar::updateSecurityIndicator()

```cpp
void URLBar::updateSecurityIndicator(SecurityLevel level) {
    securityIndicator_->setSecurityLevel(level);
}
```

### 5.5. BrowserWindow::checkHttpWarning()

```cpp
void BrowserWindow::checkHttpWarning(const QUrl &url) {
    // Insecure가 아니면 무시
    SecurityLevel level = SecurityClassifier::classify(url);
    if (level != SecurityLevel::Insecure) {
        warningTimer_->stop();
        return;
    }

    // 이미 무시한 도메인이면 무시
    QString host = url.host();
    if (ignoredDomains_.contains(host)) {
        qDebug() << "BrowserWindow: 경고 무시 (도메인:" << host << ")";
        return;
    }

    // 500ms 디바운싱
    pendingUrl_ = url;
    warningTimer_->stop();
    warningTimer_->start(500);  // 500ms 후 onWarningTimerTimeout 호출
}
```

### 5.6. BrowserWindow::showSecurityWarningDialog()

```cpp
bool BrowserWindow::showSecurityWarningDialog(const QUrl &url) {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("안전하지 않은 사이트");
    msgBox.setText("이 사이트는 보안 연결을 사용하지 않습니다.");
    msgBox.setInformativeText("개인 정보(비밀번호, 신용카드 번호 등)를 입력하지 마세요.");
    msgBox.setIcon(QMessageBox::Warning);

    // 버튼 설정
    QPushButton *continueBtn = msgBox.addButton("계속 진행", QMessageBox::AcceptRole);
    QPushButton *backBtn = msgBox.addButton("돌아가기", QMessageBox::RejectRole);
    msgBox.setDefaultButton(continueBtn);

    // 체크박스 (선택적)
    QCheckBox *dontShowAgain = new QCheckBox("이 세션 동안 이 사이트에 대해 다시 표시하지 않기");
    msgBox.setCheckBox(dontShowAgain);

    // 다이얼로그 표시
    int result = msgBox.exec();

    if (msgBox.clickedButton() == continueBtn) {
        // 체크박스 선택 시 도메인 저장
        if (dontShowAgain->isChecked()) {
            QString host = url.host();
            ignoredDomains_.insert(host);
            qDebug() << "BrowserWindow: 경고 무시 도메인 추가 -" << host;

            // 최대 100개 제한 (메모리 관리)
            if (ignoredDomains_.size() > 100) {
                qWarning() << "BrowserWindow: 경고 무시 도메인 목록 초과 (100개 제한)";
                // 가장 오래된 항목 제거 (QSet은 순서 없음 → 임의 제거)
                ignoredDomains_.erase(ignoredDomains_.begin());
            }
        }
        return true;  // 계속 진행
    } else {
        // 돌아가기
        webView_->goBack();
        return false;
    }
}
```

---

## 6. 시퀀스 흐름

### 6.1. HTTPS 사이트 접속 시 보안 아이콘 표시

```
사용자: URLBar에 "https://www.google.com" 입력 → Enter
    ↓
URLBar::onReturnPressed()
    ↓ validateAndCompleteUrl()
    ↓ emit urlSubmitted(QUrl("https://www.google.com"))
    ↓
BrowserWindow::onUrlSubmitted(const QUrl &url)
    ↓ webView_->load(url)
    ↓
WebView (내부): URL 로딩 시작
    ↓ emit urlChanged(QUrl("https://www.google.com"))
    ↓
BrowserWindow::onUrlChanged(const QUrl &url) [확장]
    ├─ SecurityLevel level = SecurityClassifier::classify(url)
    │   └─ url.scheme() == "https" → SecurityLevel::Secure
    │
    ├─ urlBar_->updateSecurityIndicator(level)
    │   └─ securityIndicator_->setSecurityLevel(SecurityLevel::Secure)
    │       ├─ setPixmap(secureIcon_)  // 녹색 자물쇠
    │       └─ setToolTip("보안 연결")
    │
    └─ checkHttpWarning(url)
        └─ level != Insecure → 경고 없음
```

### 6.2. HTTP 사이트 접속 시 경고 다이얼로그

```
사용자: URLBar에 "http://example.com" 입력 → Enter
    ↓
URLBar::onReturnPressed()
    ↓ emit urlSubmitted(QUrl("http://example.com"))
    ↓
BrowserWindow::onUrlSubmitted(const QUrl &url)
    ↓ webView_->load(url)
    ↓
WebView: emit urlChanged(QUrl("http://example.com"))
    ↓
BrowserWindow::onUrlChanged(const QUrl &url)
    ├─ SecurityLevel level = SecurityClassifier::classify(url)
    │   └─ url.scheme() == "http" && !isLocalAddress(host)
    │       → SecurityLevel::Insecure
    │
    ├─ urlBar_->updateSecurityIndicator(level)
    │   └─ securityIndicator_->setSecurityLevel(SecurityLevel::Insecure)
    │       ├─ setPixmap(insecureIcon_)  // 경고 아이콘 ⚠️
    │       └─ setToolTip("안전하지 않음")
    │
    └─ checkHttpWarning(url)
        ├─ level == Insecure → 경고 필요
        ├─ ignoredDomains_.contains("example.com") → false
        ├─ pendingUrl_ = url
        └─ warningTimer_->start(500)  // 500ms 디바운싱
            ↓ (500ms 후)
        onWarningTimerTimeout()
            ↓ showSecurityWarningDialog(pendingUrl_)
                ↓ QMessageBox::warning() 표시
                ├─ "계속 진행" 버튼 클릭 (+ 체크박스 선택)
                │   └─ ignoredDomains_.insert("example.com")
                │
                └─ "돌아가기" 버튼 클릭
                    └─ webView_->goBack()
```

### 6.3. HTTP → HTTPS 자동 리다이렉트 시 경고 숨김

```
사용자: URLBar에 "http://github.com" 입력 → Enter
    ↓
WebView: emit urlChanged(QUrl("http://github.com"))
    ↓
BrowserWindow::onUrlChanged(url)
    ├─ level = Insecure
    ├─ checkHttpWarning(url)
    │   └─ warningTimer_->start(500)  // 경고 타이머 시작
    │
    ↓ (200ms 후 리다이렉트 발생)
WebView: emit urlChanged(QUrl("https://github.com"))
    ↓
BrowserWindow::onUrlChanged(url)
    ├─ level = Secure
    ├─ urlBar_->updateSecurityIndicator(Secure)
    └─ checkHttpWarning(url)
        └─ level != Insecure → warningTimer_->stop()  // 경고 타이머 취소
            → 경고 다이얼로그 표시되지 않음
```

### 6.4. localhost 접속 시 경고 없음

```
사용자: URLBar에 "http://localhost:3000" 입력 → Enter
    ↓
WebView: emit urlChanged(QUrl("http://localhost:3000"))
    ↓
BrowserWindow::onUrlChanged(url)
    ├─ SecurityLevel level = SecurityClassifier::classify(url)
    │   └─ url.scheme() == "http" && isLocalAddress("localhost")
    │       → SecurityLevel::Local
    │
    ├─ urlBar_->updateSecurityIndicator(level)
    │   └─ securityIndicator_->setSecurityLevel(SecurityLevel::Local)
    │       ├─ setPixmap(localIcon_)  // 정보 아이콘 ℹ️
    │       └─ setToolTip("로컬 연결")
    │
    └─ checkHttpWarning(url)
        └─ level != Insecure → 경고 없음
```

---

## 7. 영향 범위 분석

### 7.1. 수정 필요한 기존 파일

| 파일 | 변경 내용 | 영향도 |
|------|-----------|--------|
| `src/ui/URLBar.h` | SecurityIndicator* 멤버 변수 추가, updateSecurityIndicator() 메서드 추가 | Low |
| `src/ui/URLBar.cpp` | SecurityIndicator 생성 및 inputLayout_에 추가, updateSecurityIndicator() 구현 | Low |
| `src/browser/BrowserWindow.h` | ignoredDomains_, warningTimer_, pendingUrl_ 멤버 변수 추가, checkHttpWarning/showSecurityWarningDialog 메서드 추가 | Medium |
| `src/browser/BrowserWindow.cpp` | onUrlChanged 슬롯 확장 (보안 아이콘 및 경고 처리), 경고 다이얼로그 구현 | Medium |

### 7.2. 새로 생성할 파일

| 파일 | 역할 | 의존성 |
|------|------|--------|
| `src/utils/SecurityClassifier.h` | SecurityLevel enum, SecurityClassifier 클래스 선언 | Qt Core (QUrl, QString, QRegularExpression) |
| `src/utils/SecurityClassifier.cpp` | SecurityClassifier 구현 | SecurityClassifier.h |
| `src/ui/SecurityIndicator.h` | SecurityIndicator 클래스 선언 | Qt Widgets (QLabel), SecurityClassifier.h |
| `src/ui/SecurityIndicator.cpp` | SecurityIndicator 구현 | SecurityIndicator.h |
| `resources/icons/lock_secure.png` | HTTPS 보안 아이콘 (32x32px, 녹색 자물쇠) | — |
| `resources/icons/lock_insecure.png` | HTTP 경고 아이콘 (32x32px, 노란색 경고 ⚠️) | — |
| `resources/icons/info.png` | 로컬 연결 아이콘 (32x32px, 파란색 정보 ℹ️) | — |
| `tests/unit/SecurityClassifierTest.cpp` | SecurityClassifier 단위 테스트 | Google Test |

### 7.3. 영향 받는 기존 기능

| 기능 | 영향 내용 | 대응 방안 |
|------|-----------|-----------|
| F-03 (URL 입력 UI) | URLBar 레이아웃 변경 (SecurityIndicator 추가) | QHBoxLayout에 SecurityIndicator 추가, 고정 너비 설정으로 레이아웃 시프트 방지 |
| F-02 (WebView 통합) | urlChanged 시그널 구독 추가 | BrowserWindow::onUrlChanged 슬롯 확장 (기존 로직 유지) |
| F-04 (네비게이션 컨트롤) | goBack() 메서드 호출 추가 | 경고 다이얼로그 "돌아가기" 버튼에서 webView_->goBack() 호출 |

---

## 8. 기술적 주의사항

### 8.1. 보안 관련
- **중요**: 보안 등급 분류는 서버 인증서가 아닌 URL 프로토콜 기반이므로, 인증서 오류(만료, 자체 서명)를 감지할 수 없습니다. Qt WebEngine 내부에서 인증서 오류 시 자동 차단하므로 추가 처리 불필요.
- HTTP 경고 무시 옵션은 세션 단위만 제공 (영구 저장 금지). QSettings나 LS2 API 사용 금지.
- 사설 IP 정규표현식은 RFC 1918 표준 준수 (10.0.0.0/8, 172.16.0.0/12, 192.168.0.0/16).

### 8.2. 성능 최적화
- SecurityIndicator는 아이콘을 QPixmap으로 캐싱하여 매번 파일 I/O 방지.
- QTimer::singleShot (500ms)로 경고 디바운싱하여 불필요한 다이얼로그 표시 방지.
- QSet\<QString\>은 평균 O(1) 조회 성능 (ignoredDomains_ 체크).

### 8.3. 리모컨 UX
- SecurityIndicator는 setFocusPolicy(Qt::StrongFocus)로 리모컨 포커스 가능.
- 포커스 시 focusInEvent에서 QToolTip::showText() 호출하여 툴팁 표시.
- 경고 다이얼로그는 QMessageBox 기본 버튼 포커스 사용 (리모컨 방향키로 이동).

### 8.4. 메모리 관리
- SecurityIndicator는 URLBar의 자식 위젯 → URLBar 소멸 시 자동 삭제 (Qt 부모-자식 메모리 관리).
- ignoredDomains_는 QSet\<QString\>으로 최대 100개 제한 (메모리 약 10KB).
- warningTimer_는 BrowserWindow 멤버 변수 → BrowserWindow 소멸 시 자동 삭제.

### 8.5. Qt 버전 호환성
- QRegularExpression은 Qt 5.0+ 사용 (Qt 4.x의 QRegExp 대신).
- QMessageBox::addButton은 Qt 5.x에서 안정적.
- QWebEngineView의 인증서 정보 접근은 Qt 5.15에서 제한적 (Qt 6.x에서 개선).

---

## 9. 데이터 모델 (N/A)

이 기능은 데이터베이스나 영구 저장소를 사용하지 않습니다.
- ignoredDomains_는 메모리 (QSet\<QString\>) 전용 (세션 단위).
- 보안 등급은 실시간 URL 분석 결과 (저장하지 않음).

---

## 10. 에러 처리

### 10.1. QUrl 파싱 오류
- **상황**: 잘못된 형식의 URL 입력 (QUrl::isValid() == false)
- **처리**: SecurityClassifier::classify()에서 SecurityLevel::Unknown 반환
- **사용자 경험**: 아이콘 숨김 (경고 없음)

### 10.2. 아이콘 리소스 로드 실패
- **상황**: resources/icons/*.png 파일 없음 또는 손상
- **처리**: QPixmap::isNull() 체크 후 qWarning 로그 출력, 빈 QLabel 표시
- **사용자 경험**: 아이콘 없이 툴팁만 표시

### 10.3. ignoredDomains_ 메모리 초과
- **상황**: 무시 도메인 100개 초과
- **처리**: 가장 오래된 항목 제거 (QSet::erase(begin()))
- **로그**: qWarning으로 경고 로그 출력

### 10.4. 리다이렉트 루프
- **상황**: HTTP ↔ HTTPS 무한 리다이렉트
- **처리**: warningTimer_ 디바운싱으로 최종 URL만 처리
- **제약**: Qt WebEngine은 리다이렉트 루프를 자동 감지 및 차단 (10회 제한)

---

## 11. 테스트 계획

### 11.1. 단위 테스트 (Google Test)

**파일**: `tests/unit/SecurityClassifierTest.cpp`

```cpp
TEST(SecurityClassifierTest, ClassifyHttpsUrl) {
    QUrl url("https://example.com");
    EXPECT_EQ(SecurityClassifier::classify(url), SecurityLevel::Secure);
}

TEST(SecurityClassifierTest, ClassifyHttpUrl) {
    QUrl url("http://example.com");
    EXPECT_EQ(SecurityClassifier::classify(url), SecurityLevel::Insecure);
}

TEST(SecurityClassifierTest, ClassifyLocalhost) {
    QUrl url("http://localhost:3000");
    EXPECT_EQ(SecurityClassifier::classify(url), SecurityLevel::Local);
}

TEST(SecurityClassifierTest, ClassifyPrivateIp) {
    EXPECT_TRUE(SecurityClassifier::isLocalAddress("192.168.1.1"));
    EXPECT_TRUE(SecurityClassifier::isLocalAddress("10.0.0.1"));
    EXPECT_TRUE(SecurityClassifier::isLocalAddress("172.16.0.1"));
    EXPECT_FALSE(SecurityClassifier::isLocalAddress("8.8.8.8"));
}

TEST(SecurityClassifierTest, ClassifyFileUrl) {
    QUrl url("file:///path/to/file.html");
    EXPECT_EQ(SecurityClassifier::classify(url), SecurityLevel::Local);
}

TEST(SecurityClassifierTest, ClassifyUnknownUrl) {
    QUrl url("about:blank");
    EXPECT_EQ(SecurityClassifier::classify(url), SecurityLevel::Unknown);
}
```

### 11.2. 통합 테스트 (Qt Test)

**파일**: `tests/integration/SecurityIndicatorTest.cpp`

```cpp
TEST_F(SecurityIndicatorTest, SetSecurityLevelSecure) {
    SecurityIndicator indicator;
    indicator.setSecurityLevel(SecurityLevel::Secure);

    EXPECT_FALSE(indicator.pixmap().isNull());
    EXPECT_EQ(indicator.toolTip(), QString("보안 연결"));
}

TEST_F(SecurityIndicatorTest, SetSecurityLevelInsecure) {
    SecurityIndicator indicator;
    indicator.setSecurityLevel(SecurityLevel::Insecure);

    EXPECT_FALSE(indicator.pixmap().isNull());
    EXPECT_EQ(indicator.toolTip(), QString("안전하지 않음"));
}
```

### 11.3. UI 테스트 (수동)

**테스트 시나리오**:
1. URLBar에 `https://www.google.com` 입력 → 녹색 자물쇠 아이콘 표시
2. URLBar에 `http://example.com` 입력 → 경고 다이얼로그 표시 → "계속 진행"
3. 경고 다이얼로그에서 체크박스 선택 → 재접속 시 경고 없음
4. 앱 재시작 → `http://example.com` 접속 → 경고 다시 표시
5. URLBar에 `http://localhost:3000` 입력 → 정보 아이콘 표시 (경고 없음)
6. 리모컨으로 SecurityIndicator 포커스 → 툴팁 표시

---

## 12. 구현 순서

### Phase 1: 보안 등급 분류 (1일)
1. SecurityLevel enum 정의 (`src/utils/SecurityClassifier.h`)
2. SecurityClassifier 클래스 구현
   - classify(const QUrl &url) 메서드
   - isLocalAddress(const QString &host) 메서드
   - 정규표현식 패턴 (사설 IP)
3. 단위 테스트 작성 및 실행 (Google Test)

### Phase 2: SecurityIndicator 위젯 (1일)
1. SecurityIndicator 클래스 구현 (`src/ui/SecurityIndicator.h/cpp`)
   - setSecurityLevel(SecurityLevel level) 메서드
   - QPixmap 아이콘 캐싱
   - 포커스 이벤트 처리
2. 아이콘 리소스 추가 (`resources/icons/*.png`)
3. QSS 스타일 적용 (포커스 테두리)

### Phase 3: URLBar 통합 (0.5일)
1. URLBar 클래스 확장
   - SecurityIndicator* 멤버 변수 추가
   - QHBoxLayout (inputLayout_)에 SecurityIndicator 추가
   - updateSecurityIndicator(SecurityLevel level) 메서드 구현
2. UI 테스트 (수동)

### Phase 4: BrowserWindow 통합 (1일)
1. BrowserWindow 클래스 확장
   - ignoredDomains_, warningTimer_, pendingUrl_ 멤버 변수 추가
   - onUrlChanged 슬롯 확장 (보안 아이콘 업데이트)
   - checkHttpWarning(const QUrl &url) 메서드 구현
   - showSecurityWarningDialog(const QUrl &url) 메서드 구현
2. 경고 다이얼로그 UI 구현 (QMessageBox)

### Phase 5: 테스트 및 검증 (1일)
1. 통합 테스트 실행 (Qt Test)
2. UI 테스트 (수동) - TS-1 ~ TS-9
3. 회귀 테스트 (F-02, F-03, F-04와 통합)
4. 리모컨 UX 테스트 (webOS 프로젝터)

**총 예상 기간**: 4.5일

---

## 13. 리스크 및 대응 방안

| 리스크 | 영향도 | 확률 | 대응 방안 |
|--------|--------|------|-----------|
| Qt WebEngineView 인증서 정보 접근 제약 | High | 확정 | FR-5 (상세 정보 다이얼로그)는 M3 범위 외로 처리. 프로토콜 기반 분류에 집중 |
| HTTP → HTTPS 리다이렉트 시 경고 깜박임 | Medium | Medium | 500ms 디바운싱 (QTimer::singleShot)으로 불필요한 경고 방지 |
| URLBar 레이아웃 시프트 | Low | Low | SecurityIndicator 영역 고정 너비 (32px + 여백) 확보, setFixedSize 사용 |
| 사설 IP 정규표현식 오류 | Medium | Low | RFC 1918 표준 정확히 구현, 단위 테스트로 검증 (TS-9) |
| 리모컨 UX에서 툴팁 접근 어려움 | Low | Medium | focusInEvent에서 QToolTip::showText() 자동 표시 |
| ignoredDomains_ 메모리 누수 | Low | Low | 최대 100개 제한, 세션 종료 시 자동 해제 |

---

## 14. 추후 개선 사항 (M3 범위 외)

### 14.1. FR-5: 보안 아이콘 클릭 시 상세 정보 다이얼로그
- Qt 6.x 업그레이드 후 QWebEngineCertificateError 확장 기능 활용
- 인증서 발급 기관, 유효 기간, TLS 버전 표시
- SecurityIndicator를 QPushButton으로 변경, clicked 시그널 연결

### 14.2. Mixed Content 감지
- Qt WebEngine의 혼합 콘텐츠 정책 활용
- HTTPS 페이지 내 HTTP 리소스 로드 감지 및 경고

### 14.3. HSTS 자동 업그레이드
- HTTP → HTTPS 자동 전환 (Qt WebEngine 내장 기능 활용)
- URLBar 입력 시 https:// 우선 시도

### 14.4. 아이콘 테마 지원
- 라이트/다크 테마에 따라 아이콘 자동 변경
- resources/icons/light/, resources/icons/dark/ 디렉토리 구조

---

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | F-14 기술 설계 |
