# ErrorPage 컴포넌트 문서

## 개요

ErrorPage는 웹 페이지 로딩 실패 시 표시되는 에러 화면 Qt 위젯입니다. WebView와 QStackedLayout으로 전환 표시되며, 리모컨 포커스 관리 및 재시도/홈 이동 기능을 제공합니다.

## 파일 위치

- **헤더**: `/Users/jsong/dev/jsong1230-github/webosbrowser-native/src/ui/ErrorPage.h`
- **구현**: `/Users/jsong/dev/jsong1230-github/webosbrowser-native/src/ui/ErrorPage.cpp`

## 기술 스택

- **언어**: C++17
- **GUI 프레임워크**: Qt 5.15+
- **부모 클래스**: QWidget
- **의존성**: WebView (에러 시그널 구독)

## 클래스 구조

### ErrorType enum class

```cpp
enum class ErrorType {
    NetworkError = -1,   // 네트워크 연결 실패 (DNS, 연결 거부 등)
    Timeout = -2,        // 타임아웃 (30초 초과)
    CorsError = -3,      // CORS 정책 위반
    UnknownError = -99   // 알 수 없는 에러
};
```

### ErrorInfo struct

```cpp
struct ErrorInfo {
    ErrorType type;          // 에러 타입
    QString errorMessage;    // 사용자 표시용 메시지
    QUrl url;                // 실패한 URL
    QDateTime timestamp;     // 에러 발생 시각
};
```

### ErrorPage 클래스

**주요 메서드**:
- `showError(ErrorType, QString, QUrl)`: 에러 화면 표시
- `showError(const ErrorInfo&)`: ErrorInfo로 에러 화면 표시 (오버로드)
- `keyPressEvent(QKeyEvent*)`: 리모컨 키 입력 처리
- `showEvent(QShowEvent*)`: 페이드인 애니메이션 실행

**시그널**:
- `retryRequested()`: 재시도 버튼 클릭
- `homeRequested()`: 홈 이동 버튼 클릭

## UI 구조

```
ErrorPage (QWidget)
├─ mainLayout (QVBoxLayout)
│  ├─ iconLabel (QLabel)           # 에러 아이콘 (⚠)
│  ├─ titleLabel (QLabel)          # 에러 제목 (48px, Bold)
│  ├─ messageLabel (QLabel)        # 에러 메시지 (28px)
│  ├─ urlLabel (QLabel)            # 실패한 URL (22px, 회색)
│  └─ buttonLayout (QHBoxLayout)
│     ├─ retryButton (QPushButton) # 재시도 버튼 (200x60px)
│     └─ homeButton (QPushButton)  # 홈 이동 버튼 (200x60px)
```

## 스타일

### 배경
- **색상**: `rgba(0, 0, 0, 230)` (반투명 검은색)

### 텍스트
- **제목**: 48px, Bold, 흰색
- **메시지**: 28px, 흰색
- **URL**: 22px, 회색 (#999999)
- **아이콘**: 80px, 주황색 (#FFA500)

### 버튼
- **배경색**: 파란색 (#1E90FF)
- **텍스트**: 흰색, 24px
- **크기**: 최소 200x60px
- **테두리**: 2px solid #1E90FF, border-radius 8px
- **포커스**: 4px solid #FFD700 (노란색 테두리)

## BrowserWindow 통합

### QStackedLayout 구조

```
BrowserWindow
└─ centralWidget
   └─ mainLayout (QVBoxLayout)
      ├─ URLBar
      ├─ NavigationBar
      ├─ LoadingIndicator
      └─ contentWidget (QWidget)
         └─ stackedLayout (QStackedLayout)
            ├─ webView (기본 표시)
            └─ errorPage (에러 시 전환)
```

### 시그널/슬롯 연결

```cpp
// WebView 에러 시그널 → BrowserWindow
connect(webView_, &WebView::loadError, this, &BrowserWindow::onLoadError);
connect(webView_, &WebView::loadTimeout, this, &BrowserWindow::onLoadTimeout);

// ErrorPage 시그널 → BrowserWindow
connect(errorPage_, &ErrorPage::retryRequested, this, &BrowserWindow::onRetryRequested);
connect(errorPage_, &ErrorPage::homeRequested, this, &BrowserWindow::onHomeRequested);

// WebView 로딩 성공 → WebView로 전환
connect(webView_, &WebView::loadFinished, [this](bool success) {
    if (success) {
        stackedLayout_->setCurrentWidget(webView_);
    }
});
```

## 동작 흐름

### 에러 발생 시

1. WebView에서 `loadError(const QString&)` 또는 `loadTimeout()` 시그널 emit
2. BrowserWindow의 `onLoadError()` 또는 `onLoadTimeout()` 슬롯 호출
3. ErrorInfo 생성 (타입, 메시지, URL, 타임스탬프)
4. `errorPage_->showError(errorInfo)` 호출
5. ErrorPage UI 업데이트 (제목, 메시지, URL)
6. `stackedLayout_->setCurrentWidget(errorPage_)` 호출
7. ErrorPage 표시 (페이드인 애니메이션 300ms)
8. 재시도 버튼에 자동 포커스

### 재시도 버튼 클릭 시

1. `retryButton_->clicked()` 시그널 emit
2. `ErrorPage::retryRequested()` 시그널 emit
3. BrowserWindow의 `onRetryRequested()` 슬롯 호출
4. QPropertyAnimation으로 페이드아웃 (200ms)
5. 애니메이션 완료 후:
   - `stackedLayout_->setCurrentWidget(webView_)`
   - `webView_->reload()`
   - `errorPage_->hide()`

### 홈 이동 버튼 클릭 시

1. `homeButton_->clicked()` 시그널 emit
2. `ErrorPage::homeRequested()` 시그널 emit
3. BrowserWindow의 `onHomeRequested()` 슬롯 호출
4. QPropertyAnimation으로 페이드아웃 (200ms)
5. 애니메이션 완료 후:
   - `stackedLayout_->setCurrentWidget(webView_)`
   - `webView_->load(QUrl("https://www.google.com"))`
   - `errorPage_->hide()`

## 리모컨 포커스 관리

### 포커스 흐름

```
errorPage_ 표시
  │
  └─ retryButton_->setFocus(Qt::OtherFocusReason)
       │
       ├─ [좌/우 방향키] → homeButton_
       │      │
       │      └─ [좌/우 방향키] → retryButton_ (순환)
       │
       └─ [Enter] → retryRequested() signal
```

### 키 이벤트 처리

```cpp
void ErrorPage::keyPressEvent(QKeyEvent *event) {
    // Back 키 무시 (에러 화면에서 벗어나지 못하도록)
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Back) {
        event->ignore();
        return;
    }

    // 나머지 키는 Qt 탭 오더 시스템으로 처리
    QWidget::keyPressEvent(event);
}
```

### 탭 오더 설정

```cpp
setTabOrder(retryButton_, homeButton_);
setTabOrder(homeButton_, retryButton_);  // 순환
```

## 애니메이션

### 페이드인 (300ms)

```cpp
void ErrorPage::startFadeInAnimation() {
    QPropertyAnimation *fadeIn = new QPropertyAnimation(this, "windowOpacity");
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}
```

**주의**: `windowOpacity`는 최상위 윈도우에서만 동작할 수 있습니다. QStackedLayout 내부에서는 동작하지 않을 수 있습니다.

### 페이드아웃 (200ms)

```cpp
void BrowserWindow::onRetryRequested() {
    QPropertyAnimation *fadeOut = new QPropertyAnimation(errorPage_, "windowOpacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    connect(fadeOut, &QPropertyAnimation::finished, this, [this]() {
        stackedLayout_->setCurrentWidget(webView_);
        webView_->reload();
        errorPage_->hide();
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}
```

## 에러 타입별 처리

### BrowserWindow에서 에러 타입 추론

```cpp
void BrowserWindow::onLoadError(const QString &errorString) {
    // 에러 메시지 파싱으로 ErrorType 추론
    ErrorType type = ErrorType::NetworkError;  // 기본값

    if (errorString.contains("timeout", Qt::CaseInsensitive)) {
        type = ErrorType::Timeout;
    } else if (errorString.contains("cors", Qt::CaseInsensitive) ||
               errorString.contains("cross-origin", Qt::CaseInsensitive)) {
        type = ErrorType::CorsError;
    }

    // ErrorInfo 생성 및 ErrorPage 표시
    ErrorInfo errorInfo;
    errorInfo.type = type;
    errorInfo.errorMessage = errorString;
    errorInfo.url = webView_->url();
    errorInfo.timestamp = QDateTime::currentDateTime();

    errorPage_->showError(errorInfo);
    stackedLayout_->setCurrentWidget(errorPage_);

    // 로그 기록
    qCritical() << "Page load error:"
                << "type=" << static_cast<int>(type)
                << "message=" << errorString
                << "url=" << errorInfo.url.toString();
}
```

### ErrorPage에서 제목 설정

```cpp
void ErrorPage::showError(ErrorType type, const QString &errorMessage, const QUrl &url) {
    QString title;
    switch (type) {
        case ErrorType::NetworkError:
            title = "네트워크 연결 실패";
            break;
        case ErrorType::Timeout:
            title = "로딩 시간 초과";
            break;
        case ErrorType::CorsError:
            title = "보안 정책 오류";
            break;
        case ErrorType::UnknownError:
        default:
            title = "페이지 로딩 오류";
            break;
    }

    titleLabel_->setText(title);
    messageLabel_->setText(errorMessage);
    urlLabel_->setText(truncateUrl(url));
}
```

## 메모리 관리

### Qt Parent-Child 시스템

- ErrorPage는 `contentWidget_`를 부모로 생성
- 모든 자식 UI 컴포넌트 (QLabel, QPushButton)는 ErrorPage를 부모로 생성
- BrowserWindow 소멸 시 자동으로 모든 자식 위젯 해제
- 스마트 포인터 불필요 (Qt parent-child 자동 관리)

### 애니메이션 객체 메모리 관리

```cpp
QPropertyAnimation *fadeOut = new QPropertyAnimation(errorPage_, "windowOpacity");
fadeOut->start(QAbstractAnimation::DeleteWhenStopped);  // 자동 삭제
```

- `DeleteWhenStopped` 플래그로 애니메이션 완료 후 자동 delete
- parent 설정 불필요 (임시 객체)

## 테스트 시나리오

### 기능 테스트

1. **네트워크 에러 감지**
   - 잘못된 URL 입력 (`https://invalid-domain-12345.com`)
   - ErrorPage 표시 확인
   - 제목: "네트워크 연결 실패"

2. **타임아웃 에러**
   - 30초 초과 로딩
   - ErrorPage 표시 확인
   - 제목: "로딩 시간 초과"

3. **재시도 버튼**
   - 재시도 버튼 클릭
   - 페이지 재로드 확인
   - ErrorPage 숨김 확인

4. **홈 이동 버튼**
   - 홈 버튼 클릭
   - 홈페이지(https://www.google.com) 이동 확인
   - ErrorPage 숨김 확인

### 리모컨 포커스 테스트

1. **자동 포커스**
   - ErrorPage 표시 시 재시도 버튼에 자동 포커스
   - 노란색 테두리 (4px solid #FFD700) 표시 확인

2. **좌/우 방향키**
   - 좌/우 방향키로 재시도 ↔ 홈 버튼 전환 확인
   - 순환 동작 확인

3. **Back 키 무시**
   - Back 키 입력 시 ErrorPage 유지 확인
   - 벗어나지 못함 확인

4. **Enter 키**
   - Enter 키로 버튼 클릭 확인
   - 재시도 또는 홈 이동 동작 확인

### 대화면 가독성 테스트

- 3m 거리에서 프로젝터 화면 확인
- 제목 (48px) 가독 가능 확인
- 메시지 (28px) 가독 가능 확인
- 버튼 텍스트 (24px) 가독 가능 확인
- URL (22px) 가독 가능 확인

### 성능 테스트

- 에러 발생 → 화면 표시: 500ms 이내 목표
- 재시도 클릭 → 로딩 시작: 300ms 이내 목표
- 페이드인/아웃 애니메이션: 60fps 또는 최소 30fps

## 로그 예시

### 네트워크 에러

```
WebView: 로딩 시작
WebView: 로딩 실패 - https://invalid-domain.com
WebView: 상태 변경 - 3 (Error)
ErrorPage: 생성 중...
ErrorPage: UI 초기화 완료
ErrorPage: 스타일 적용 완료
ErrorPage: 시그널/슬롯 연결 완료
ErrorPage: 생성 완료
ErrorPage: 에러 표시 - 네트워크 연결 실패: 페이지 로딩 실패
ErrorPage: 페이드인 애니메이션 시작
[Critical] Page load error: type=-1 message=페이지 로딩 실패 url=https://invalid-domain.com
```

### 재시도

```
BrowserWindow: 재시도 요청
ErrorPage: 페이드인 애니메이션 시작
WebView: 새로고침
WebView: 로딩 시작
```

## 제약사항

### Qt WebEngineView 에러 감지 제약

- Qt WebEngineView는 `loadFinished(false)`와 에러 문자열만 제공
- HTTP 상태 코드(404, 500)는 페이지가 정상 렌더링되므로 에러로 감지 안 됨
- 에러 타입은 문자열 파싱으로 추론 (완벽하지 않음)

### 애니메이션 제약

- `windowOpacity` 애니메이션은 최상위 윈도우에서만 동작할 수 있음
- QStackedLayout 내부에서는 동작하지 않을 수 있음
- 프로젝터 GPU가 약한 경우 애니메이션이 끊길 수 있음

### 리모컨 포커스 스타일 제약

- webOS Qt 빌드에서 QSS focus 스타일이 플랫폼 의존적일 수 있음
- 실제 디바이스에서 테스트 필요

## 향후 개선 사항

### 에러 타입별 복구 제안

```cpp
// ErrorPage에 suggestionLabel 추가
if (type == ErrorType::NetworkError) {
    suggestionLabel_->setText("Wi-Fi 연결을 확인하세요");
} else if (type == ErrorType::Timeout) {
    suggestionLabel_->setText("잠시 후 다시 시도하세요");
}
```

### 에러 아이콘 이미지

```cpp
// 텍스트 아이콘 대신 QPixmap 사용
QString ErrorPage::getErrorIcon(ErrorType type) const {
    switch (type) {
        case ErrorType::NetworkError:
            return ":/icons/network_error.png";
        case ErrorType::Timeout:
            return ":/icons/timeout_error.png";
        default:
            return ":/icons/error.png";
    }
}
```

### 에러 통계 수집

```cpp
// SettingsService에 에러 통계 저장
void BrowserWindow::onLoadError(const QString &errorString) {
    // ... 기존 코드 ...

    // 에러 통계 기록
    settingsService_->recordErrorStatistics(errorInfo);
}
```

### 홈 URL 설정 가능

```cpp
// SettingsService에서 홈 URL 가져오기
void BrowserWindow::onHomeRequested() {
    // ... 페이드아웃 애니메이션 ...

    QString homeUrl = settingsService_->homeUrl();
    webView_->load(QUrl(homeUrl));
}
```

## 관련 문서

- **요구사항 분석서**: `docs/specs/error-handling/requirements.md`
- **기술 설계서**: `docs/specs/error-handling/design.md`
- **구현 계획서**: `docs/specs/error-handling/plan.md`
- **WebView 컴포넌트**: `src/browser/WebView.h`, `src/browser/WebView.cpp`
- **BrowserWindow 컴포넌트**: `src/browser/BrowserWindow.h`, `src/browser/BrowserWindow.cpp`

## 작성 정보

- **작성일**: 2026-02-14
- **작성자**: Claude (AI Assistant)
- **버전**: 1.0.0
- **기능 ID**: F-10 (에러 처리)
