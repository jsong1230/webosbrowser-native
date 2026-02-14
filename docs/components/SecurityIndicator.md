# SecurityIndicator 컴포넌트

## 1. 개요

- **컴포넌트명**: SecurityIndicator
- **타입**: Qt QLabel 기반 위젯
- **목적**: URL 보안 상태를 시각적 아이콘으로 표시
- **위치**: URLBar 좌측
- **기능**: HTTPS/HTTP/localhost 보안 등급 표시, 툴팁 제공

---

## 2. 주요 기능

### 2.1. 보안 등급 표시
- **Secure (HTTPS)**: 녹색 자물쇠 아이콘 (lock_secure.png)
  - 툴팁: "보안 연결"
- **Insecure (HTTP)**: 노란색 경고 아이콘 (lock_insecure.png)
  - 툴팁: "안전하지 않음"
- **Local (localhost/file)**: 파란색 정보 아이콘 (info.png)
  - 툴팁: "로컬 연결"
- **Unknown**: 아이콘 숨김
  - 툴팁: ""

### 2.2. 리모컨 접근성
- 포커스 정책: Qt::StrongFocus
- 포커스 시 자동 툴팁 표시 (QToolTip::showText)
- 포커스 테두리: 2px solid #00aaff

### 2.3. 아이콘 캐싱
- QPixmap으로 아이콘 사전 로드 (성능 최적화)
- 아이콘 크기: 32x32px (고정)
- 위젯 크기: 40x40px (패딩 포함)

---

## 3. API

### 3.1. Public 메서드

#### `void setSecurityLevel(SecurityLevel level)`
- **설명**: 보안 등급 설정 및 아이콘/툴팁 갱신
- **파라미터**:
  - `level`: SecurityLevel enum (Secure/Insecure/Local/Unknown)
- **반환값**: void
- **예시**:
  ```cpp
  securityIndicator->setSecurityLevel(SecurityLevel::Secure);
  ```

#### `SecurityLevel securityLevel() const`
- **설명**: 현재 보안 등급 반환
- **반환값**: SecurityLevel

### 3.2. Signals

#### `void clicked()`
- **설명**: 아이콘 클릭 시그널 (향후 FR-5 상세 정보 다이얼로그용)
- **발생 조건**: 마우스 클릭 또는 리모컨 Enter 키

---

## 4. 사용 예시

### 4.1. 기본 사용법
```cpp
// URLBar.cpp에서
securityIndicator_ = new SecurityIndicator(this);
inputLayout_->addWidget(securityIndicator_);

// BrowserWindow.cpp에서
void BrowserWindow::onUrlChanged(const QString& url) {
    QUrl qurl(url);
    SecurityLevel level = SecurityClassifier::classify(qurl);
    urlBar_->updateSecurityIndicator(level);
}
```

### 4.2. URLBar 통합
```cpp
// URLBar.h
private:
    SecurityIndicator *securityIndicator_;

// URLBar.cpp
void URLBar::setupUI() {
    inputLayout_->addWidget(securityIndicator_);  // 좌측 배치
    inputLayout_->addWidget(inputField_);
}

void URLBar::updateSecurityIndicator(SecurityLevel level) {
    securityIndicator_->setSecurityLevel(level);
}
```

---

## 5. 의존성

### 5.1. 헤더 포함
```cpp
#include "../utils/SecurityClassifier.h"
```

### 5.2. Qt 모듈
- Qt5::Widgets (QLabel, QPixmap, QToolTip)
- Qt5::Gui (QMouseEvent, QKeyEvent, QFocusEvent)

### 5.3. 리소스 파일
- `resources/icons/lock_secure.png` (32x32px)
- `resources/icons/lock_insecure.png` (32x32px)
- `resources/icons/info.png` (32x32px)

---

## 6. 스타일 (QSS)

```cpp
setStyleSheet(
    "QLabel {"
    "    border: 2px solid transparent;"
    "    border-radius: 4px;"
    "    padding: 4px;"
    "}"
    "QLabel:focus {"
    "    border: 2px solid #00aaff;"  // 포커스 테두리
    "}"
);
```

---

## 7. 제약사항

### 7.1. 아이콘 리소스 필수
- 아이콘 파일이 없으면 qWarning 로그 출력
- 아이콘 로드 실패 시 빈 QLabel 표시 (툴팁만 제공)

### 7.2. 리모컨 UX
- 마우스 호버 불가 (리모컨 환경)
- 포커스 시에만 툴팁 표시

### 7.3. 향후 개선 (FR-5)
- 클릭 시 상세 정보 다이얼로그 (인증서 정보)
- QPushButton으로 변경 고려

---

## 8. 테스트

### 8.1. 단위 테스트
- SecurityIndicatorTest.cpp (Qt Test)
- setSecurityLevel() 메서드 테스트
- 아이콘 로드 테스트

### 8.2. 통합 테스트
- URLBar와의 통합 테스트
- BrowserWindow와의 통합 테스트
- 리모컨 포커스 테스트 (webOS 프로젝터)

---

## 9. 변경 이력

| 날짜 | 버전 | 변경 내용 |
|------|------|-----------|
| 2026-02-14 | 1.0 | SecurityIndicator 컴포넌트 초기 구현 (F-14) |

---

## 10. 관련 문서

- `docs/specs/https-security/requirements.md` (요구사항)
- `docs/specs/https-security/design.md` (기술 설계)
- `src/utils/SecurityClassifier.h` (보안 등급 분류)
- `src/ui/URLBar.h` (URLBar 통합)
