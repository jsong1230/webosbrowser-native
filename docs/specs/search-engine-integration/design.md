# 검색 엔진 통합 — 기술 설계서 (Native App)

## 1. 참조
- 요구사항 분석서: docs/specs/search-engine-integration/requirements.md
- F-03 URL 입력 UI 설계서: docs/specs/url-input-ui/design.md
- URLBar 클래스: src/ui/URLBar.h / URLBar.cpp
- URLValidator 클래스: src/utils/URLValidator.h / URLValidator.cpp
- StorageService 클래스: src/services/StorageService.h / StorageService.cpp
- CLAUDE.md: 프로젝트 규칙 및 코딩 컨벤션

---

## 2. 아키텍처 개요

### 레이어 구조

```
┌─────────────────────────────────────────────────┐
│                   UI Layer                       │
│  ┌────────────┐       ┌──────────────┐         │
│  │  URLBar    │───────│ BrowserWindow │         │
│  │ (입력 처리) │       │ (시그널 연결) │         │
│  └────────────┘       └──────────────┘         │
└─────────────────────────────────────────────────┘
               ▼                      ▼
┌─────────────────────────────────────────────────┐
│              Business Logic Layer                │
│  ┌───────────────┐      ┌──────────────┐       │
│  │URLValidator   │      │SearchEngine  │       │
│  │(URL vs 검색어) │      │(URL 생성)     │       │
│  └───────────────┘      └──────────────┘       │
└─────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────┐
│                Service Layer                     │
│         ┌─────────────────────┐                 │
│         │  SettingsService    │ (F-11 연동)     │
│         │ (검색 엔진 설정 저장) │                 │
│         └─────────────────────┘                 │
└─────────────────────────────────────────────────┘
                           ▼
┌─────────────────────────────────────────────────┐
│              Data Storage Layer                  │
│         ┌─────────────────────┐                 │
│         │  StorageService     │                 │
│         │  (LS2 API 래퍼)     │                 │
│         └─────────────────────┘                 │
└─────────────────────────────────────────────────┘
```

### 데이터 플로우

```
사용자 입력 ("cat videos")
      │
      ▼
┌─────────────────────┐
│ URLBar::            │
│ validateAndComplete │
│ Url()               │
└─────────────────────┘
      │
      ▼
┌─────────────────────┐
│ URLValidator::      │
│ isUrl(input)        │
└─────────────────────┘
      │
      ├─ true ───▶ URL로 처리 (기존 로직)
      │
      └─ false ──▶ 검색어로 처리
                        │
                        ▼
                  ┌────────────────────┐
                  │ SearchEngine::     │
                  │ createSearchUrl()  │
                  └────────────────────┘
                        │
                        ▼
                  QUrl("https://www.google.com/search?q=cat%20videos")
                        │
                        ▼
                  emit urlSubmitted(url)
                        │
                        ▼
                  WebView::load(url)
```

---

## 3. 아키텍처 결정

### 결정 1: URL vs 검색어 판단 로직
- **선택지**:
  - A) URLBar 내부에 조건문으로 직접 판단
  - B) URLValidator에 isUrl() 메서드 추가, false이면 검색어로 처리
  - C) SearchEngine에 isSearchQuery() 메서드 추가
- **결정**: B) URLValidator에 isUrl() 메서드 추가
- **근거**:
  - URLValidator는 이미 URL 형식 검증 로직 보유 (isDomainFormat 정규식)
  - 책임 분리: URL 검증은 URLValidator, 검색 URL 생성은 SearchEngine
  - 정규표현식 기반으로 도메인 형식 (.com, .net 등), localhost, IP 주소 판단 가능
  - URLBar는 isUrl() 결과에 따라 분기만 하면 됨 (단순화)
- **트레이드오프**:
  - 포기: SearchEngine에서 모든 로직 처리 (응집도 낮음)
  - 획득: 명확한 책임 분리, 기존 URLValidator 재사용, 테스트 용이성

### 결정 2: SearchEngine 클래스 설계
- **선택지**:
  - A) 정적 메서드만 사용 (싱글톤 불필요)
  - B) 싱글톤 패턴 (인스턴스 하나만 생성)
  - C) BrowserWindow에서 인스턴스 생성 후 URLBar에 주입
- **결정**: A) 정적 메서드만 사용
- **근거**:
  - SearchEngine은 상태를 가지지 않음 (stateless)
  - 검색 엔진 정의는 QMap 정적 멤버로 충분
  - URL 생성 로직은 순수 함수 (입력 → 출력, 부작용 없음)
  - 싱글톤은 불필요한 복잡도 증가, 테스트 어려움
  - 정적 메서드는 호출이 간단하고 직관적
- **트레이드오프**:
  - 포기: 인스턴스 생성으로 의존성 주입 가능성 (DI 패턴)
  - 획득: 단순성, 빠른 호출, 메모리 절약, 테스트 용이

### 결정 3: 검색 엔진 추가 방법
- **선택지**:
  - A) SearchEngine.cpp의 QMap에 하드코딩 (컴파일 타임)
  - B) JSON 파일로 외부화 (런타임 로드)
  - C) LS2 API로 동적 추가 (관리자 설정)
- **결정**: A) QMap에 하드코딩
- **근거**:
  - M2 범위에서는 Google, Naver, Bing, DuckDuckGo 4개만 지원
  - 검색 엔진 추가는 개발자 작업 (사용자 커스터마이징 미지원)
  - JSON 파일 파싱은 불필요한 오버헤드 (4개 항목만)
  - 컴파일 타임에 모든 검색 엔진 정의 확정, 성능 최고
  - 향후 M3에서 JSON 외부화로 확장 가능 (리팩토링 쉬움)
- **트레이드오프**:
  - 포기: 런타임 검색 엔진 추가 유연성
  - 획득: 단순성, 성능, 컴파일 타임 검증

### 결정 4: 기본 검색 엔진 설정 저장 방식
- **선택지**:
  - A) LS2 API로 저장 (StorageService 사용)
  - B) QSettings (Qt 로컬 설정 파일)
  - C) 메모리에만 유지 (앱 재시작 시 기본값으로 복원)
- **결정**: A) LS2 API로 저장 (F-11 연동 준비)
- **근거**:
  - webOS 표준 방식은 LS2 API (비동기 메시지 버스)
  - 다른 앱 또는 webOS 설정과 통합 가능성 (장기적 이점)
  - QSettings는 webOS에서 비표준, 파일 위치 제한적
  - F-11 설정 화면에서 SettingsService::setSearchEngine() 호출 예정
  - 기존 StorageService 인프라 재사용 가능
- **트레이드오프**:
  - 포기: QSettings의 간편함, 동기 API
  - 획득: webOS 표준 준수, 확장성, F-11 연동 용이

---

## 4. 클래스 설계

### 4.1 SearchEngine 클래스

#### 헤더 파일 (src/services/SearchEngine.h)

```cpp
/**
 * @file SearchEngine.h
 * @brief 검색 엔진 URL 생성 및 관리 서비스
 */

#pragma once

#include <QString>
#include <QUrl>
#include <QMap>
#include <QStringList>

namespace webosbrowser {

/**
 * @struct SearchEngineInfo
 * @brief 검색 엔진 정의 구조체
 */
struct SearchEngineInfo {
    QString id;          ///< 검색 엔진 ID (예: "google", "naver")
    QString name;        ///< 표시 이름 (예: "Google", "Naver")
    QString urlTemplate; ///< URL 템플릿 (예: "https://www.google.com/search?q={query}")
    QString iconPath;    ///< 아이콘 경로 (선택적, F-11 UI에서 사용)
};

/**
 * @class SearchEngine
 * @brief 검색 엔진 URL 생성 및 검색 엔진 목록 관리
 *
 * 정적 메서드만 제공하는 유틸리티 클래스.
 * 검색어를 받아 선택된 검색 엔진의 검색 URL을 생성합니다.
 */
class SearchEngine {
public:
    // 생성자 금지 (정적 메서드만 사용)
    SearchEngine() = delete;
    ~SearchEngine() = delete;

    /**
     * @brief 검색 URL 생성
     * @param query 검색어 (예: "cat videos")
     * @param engineId 검색 엔진 ID (예: "google", "naver")
     * @return 검색 URL (예: "https://www.google.com/search?q=cat%20videos")
     *         지원하지 않는 검색 엔진이면 빈 QUrl 반환
     */
    static QUrl createSearchUrl(const QString &query, const QString &engineId);

    /**
     * @brief 지원하는 검색 엔진 ID 목록 반환
     * @return QStringList (예: ["google", "naver", "bing", "duckduckgo"])
     */
    static QStringList getAvailableEngines();

    /**
     * @brief 검색 엔진 표시 이름 반환
     * @param engineId 검색 엔진 ID
     * @return 표시 이름 (예: "Google"), 존재하지 않으면 빈 문자열
     */
    static QString getEngineName(const QString &engineId);

    /**
     * @brief 검색 엔진 URL 템플릿 반환
     * @param engineId 검색 엔진 ID
     * @return URL 템플릿 (예: "https://www.google.com/search?q={query}")
     */
    static QString getEngineUrlTemplate(const QString &engineId);

    /**
     * @brief 기본 검색 엔진 ID 반환
     * @return "google" (하드코딩된 기본값)
     */
    static QString getDefaultEngineId();

private:
    /**
     * @brief 검색 엔진 정의 맵
     *
     * 키: 검색 엔진 ID, 값: SearchEngineInfo 구조체
     */
    static const QMap<QString, SearchEngineInfo> engines_;
};

} // namespace webosbrowser
```

#### 구현 파일 (src/services/SearchEngine.cpp)

```cpp
/**
 * @file SearchEngine.cpp
 * @brief 검색 엔진 서비스 구현
 */

#include "SearchEngine.h"
#include <QDebug>

namespace webosbrowser {

// 검색 엔진 정의 (정적 멤버 초기화)
const QMap<QString, SearchEngineInfo> SearchEngine::engines_ = {
    {"google", {
        "google",
        "Google",
        "https://www.google.com/search?q={query}",
        ":/icons/google.png"
    }},
    {"naver", {
        "naver",
        "Naver",
        "https://search.naver.com/search.naver?query={query}",
        ":/icons/naver.png"
    }},
    {"bing", {
        "bing",
        "Bing",
        "https://www.bing.com/search?q={query}",
        ":/icons/bing.png"
    }},
    {"duckduckgo", {
        "duckduckgo",
        "DuckDuckGo",
        "https://duckduckgo.com/?q={query}",
        ":/icons/duckduckgo.png"
    }}
};

QUrl SearchEngine::createSearchUrl(const QString &query, const QString &engineId) {
    // 빈 검색어 체크
    if (query.trimmed().isEmpty()) {
        qDebug() << "SearchEngine: 빈 검색어";
        return QUrl();
    }

    // 검색 엔진 ID 정규화 (소문자 변환)
    QString normalizedEngineId = engineId.toLower();

    // 검색 엔진 존재 확인
    if (!engines_.contains(normalizedEngineId)) {
        qDebug() << "SearchEngine: 지원하지 않는 검색 엔진 -" << engineId
                 << "→ Google로 폴백";
        normalizedEngineId = "google";  // 기본값으로 폴백
    }

    // URL 템플릿 가져오기
    QString urlTemplate = engines_[normalizedEngineId].urlTemplate;

    // 검색어 URL 인코딩 (Qt 함수 사용)
    // QUrl::toPercentEncoding()은 UTF-8 기반, 공백 → %20, 한글 → %xx 변환
    QString encodedQuery = QUrl::toPercentEncoding(query);

    // {query} 플레이스홀더를 인코딩된 검색어로 치환
    QString finalUrl = urlTemplate;
    finalUrl.replace("{query}", encodedQuery);

    qDebug() << "SearchEngine: 검색 URL 생성 -" << finalUrl;

    return QUrl(finalUrl);
}

QStringList SearchEngine::getAvailableEngines() {
    return engines_.keys();
}

QString SearchEngine::getEngineName(const QString &engineId) {
    QString normalizedEngineId = engineId.toLower();

    if (engines_.contains(normalizedEngineId)) {
        return engines_[normalizedEngineId].name;
    }

    qDebug() << "SearchEngine: 존재하지 않는 검색 엔진 ID -" << engineId;
    return QString();
}

QString SearchEngine::getEngineUrlTemplate(const QString &engineId) {
    QString normalizedEngineId = engineId.toLower();

    if (engines_.contains(normalizedEngineId)) {
        return engines_[normalizedEngineId].urlTemplate;
    }

    qDebug() << "SearchEngine: 존재하지 않는 검색 엔진 ID -" << engineId;
    return QString();
}

QString SearchEngine::getDefaultEngineId() {
    return "google";
}

} // namespace webosbrowser
```

### 4.2 URLValidator 확장

#### 헤더 파일 수정 (src/utils/URLValidator.h)

기존 메서드 외 추가:

```cpp
// 기존 메서드 유지 (validate, normalize, isUrl, isValidUrlFormat)

/**
 * @brief 검색어 여부 판단 (URL이 아닌 경우)
 * @param input 사용자 입력 문자열
 * @return 검색어이면 true, URL이면 false
 */
static bool isSearchQuery(const QString &input);
```

#### 구현 파일 수정 (src/utils/URLValidator.cpp)

```cpp
// 기존 구현 유지

bool URLValidator::isSearchQuery(const QString &input) {
    // isUrl()이 false이면 검색어로 간주
    return !isUrl(input);
}
```

**참고**: 기존 `isUrl()` 메서드가 이미 존재하므로, `isSearchQuery()`는 그 반대값만 반환하면 됨.

### 4.3 URLBar 수정

#### 헤더 파일 수정 (src/ui/URLBar.h)

```cpp
// 기존 클래스 유지, 새 메서드 추가

private:
    /**
     * @brief 기본 검색 엔진 ID 설정
     * @param engineId 검색 엔진 ID (예: "google", "naver")
     */
    void setDefaultSearchEngine(const QString &engineId);

    /**
     * @brief 기본 검색 엔진 ID 반환
     * @return 현재 설정된 검색 엔진 ID
     */
    QString getDefaultSearchEngine() const;

private:
    QString defaultSearchEngine_;  ///< 기본 검색 엔진 ID (기본값: "google")
```

#### 구현 파일 수정 (src/ui/URLBar.cpp)

```cpp
// URLBar.cpp

#include "URLBar.h"
#include "../utils/URLValidator.h"
#include "../services/SearchEngine.h"  // 추가
#include <QDebug>

namespace webosbrowser {

URLBar::URLBar(QWidget *parent)
    : QWidget(parent)
    , inputField_(new QLineEdit(this))
    , errorLabel_(new QLabel(this))
    , mainLayout_(new QVBoxLayout(this))
    , inputLayout_(new QHBoxLayout())
    , previousUrl_()
    , defaultSearchEngine_("google")  // 추가: 기본값 초기화
{
    // 기존 생성자 코드 유지
    qDebug() << "URLBar: 생성 중...";
    setupUI();
    setupConnections();
    applyStyles();
    qDebug() << "URLBar: 생성 완료";
}

// 기본 검색 엔진 설정 메서드 추가
void URLBar::setDefaultSearchEngine(const QString &engineId) {
    defaultSearchEngine_ = engineId;
    qDebug() << "URLBar: 기본 검색 엔진 변경 -" << engineId;
}

QString URLBar::getDefaultSearchEngine() const {
    return defaultSearchEngine_;
}

// validateAndCompleteUrl() 메서드 수정
QUrl URLBar::validateAndCompleteUrl(const QString &input) {
    // URLValidator를 사용하여 URL 검증
    URLValidator::ValidationResult result = URLValidator::validate(input);

    QUrl url;

    // 1. 유효한 URL (프로토콜 포함)
    if (result == URLValidator::ValidationResult::Valid) {
        url = QUrl(input);
    }
    // 2. 프로토콜 누락 (자동 보완 가능)
    else if (result == URLValidator::ValidationResult::MissingScheme) {
        url = QUrl(URLValidator::normalize(input));
    }
    // 3. URL 형식 아님 → 검색어로 판단
    else if (URLValidator::isSearchQuery(input)) {
        qDebug() << "URLBar: 검색어로 판단 -" << input;

        // SearchEngine으로 검색 URL 생성
        url = SearchEngine::createSearchUrl(input, defaultSearchEngine_);

        if (!url.isValid()) {
            showError("검색 URL 생성 실패");
            return QUrl();
        }

        qDebug() << "URLBar: 검색 URL 생성 완료 -" << url.toString();
        return url;
    }
    // 4. 빈 입력 또는 유효하지 않은 형식
    else {
        return QUrl();  // 빈 QUrl 반환
    }

    // XSS 방어: 위험한 스키마 필터링 (기존 로직 유지)
    QStringList dangerousSchemes = {"javascript", "data", "vbscript", "file"};
    if (dangerousSchemes.contains(url.scheme().toLower())) {
        showError("허용되지 않는 URL 형식입니다");
        return QUrl();
    }

    return url;
}

} // namespace webosbrowser
```

---

## 5. 데이터 모델

### SearchEngineInfo 구조체

```cpp
struct SearchEngineInfo {
    QString id;          ///< 검색 엔진 ID (예: "google", "naver")
    QString name;        ///< 표시 이름 (예: "Google", "Naver")
    QString urlTemplate; ///< URL 템플릿 (예: "https://www.google.com/search?q={query}")
    QString iconPath;    ///< 아이콘 경로 (선택적, F-11 UI에서 사용)
};
```

### 검색 엔진 정의 (QMap)

| ID | Name | URL Template | Icon Path |
|---|---|---|---|
| google | Google | `https://www.google.com/search?q={query}` | `:/icons/google.png` |
| naver | Naver | `https://search.naver.com/search.naver?query={query}` | `:/icons/naver.png` |
| bing | Bing | `https://www.bing.com/search?q={query}` | `:/icons/bing.png` |
| duckduckgo | DuckDuckGo | `https://duckduckgo.com/?q={query}` | `:/icons/duckduckgo.png` |

### 설정 저장 형식 (LS2 API, F-11 연동)

```json
{
  "key": "defaultSearchEngine",
  "value": "naver"
}
```

StorageService::set("defaultSearchEngine", "naver") 호출 시 저장.

---

## 6. 시퀀스 다이어그램

### 시나리오 1: 검색어 입력 및 검색

```
사용자 → URLBar → URLValidator → SearchEngine → WebView
   │         │             │              │            │
   │ "cat"  입력           │              │            │
   │────────▶│             │              │            │
   │         │ Enter 키    │              │            │
   │────────▶│             │              │            │
   │         │ validateAndCompleteUrl("cat")           │
   │         │────────────▶│              │            │
   │         │             │ isUrl("cat") │            │
   │         │             │ return false │            │
   │         │◀────────────│              │            │
   │         │             │              │            │
   │         │ isSearchQuery("cat") → true│            │
   │         │────────────▶│              │            │
   │         │◀────────────│              │            │
   │         │             │              │            │
   │         │ createSearchUrl("cat", "google")        │
   │         │──────────────────────────▶│            │
   │         │             │              │ toPercentEncoding("cat")
   │         │             │              │ urlTemplate.replace("{query}", "cat")
   │         │             │              │ QUrl("https://www.google.com/search?q=cat")
   │         │◀──────────────────────────│            │
   │         │             │              │            │
   │         │ emit urlSubmitted(url)    │            │
   │         │───────────────────────────────────────▶│
   │         │             │              │ load(url)  │
   │         │             │              │────────────│
   │         │             │              │            │
   │         │ setText(url) (WebView::urlChanged)     │
   │         │◀───────────────────────────────────────│
```

### 시나리오 2: URL vs 검색어 판단

```
입력: "google.com"
  → URLBar::validateAndCompleteUrl("google.com")
  → URLValidator::validate("google.com") → MissingScheme
  → URLValidator::normalize("google.com") → "https://google.com"
  → QUrl("https://google.com")
  → emit urlSubmitted(url)
  → WebView::load(url)

입력: "google" (검색어)
  → URLBar::validateAndCompleteUrl("google")
  → URLValidator::validate("google") → InvalidFormat
  → URLValidator::isSearchQuery("google") → true
  → SearchEngine::createSearchUrl("google", "google")
  → QUrl("https://www.google.com/search?q=google")
  → emit urlSubmitted(url)
  → WebView::load(url)

입력: "cat videos" (검색어)
  → URLBar::validateAndCompleteUrl("cat videos")
  → URLValidator::validate("cat videos") → InvalidFormat
  → URLValidator::isSearchQuery("cat videos") → true
  → SearchEngine::createSearchUrl("cat videos", "google")
  → QUrl::toPercentEncoding("cat videos") → "cat%20videos"
  → QUrl("https://www.google.com/search?q=cat%20videos")
  → emit urlSubmitted(url)
  → WebView::load(url)
```

### 시나리오 3: 검색 엔진 변경 (F-11 연동 시)

```
사용자 → SettingsPanel → SettingsService → StorageService → URLBar
   │            │                 │             │                │
   │ 설정 열기  │                 │             │                │
   │───────────▶│                 │             │                │
   │            │ "Naver" 선택    │             │                │
   │───────────▶│                 │             │                │
   │            │ setSearchEngine("naver")      │                │
   │            │────────────────▶│             │                │
   │            │                 │ set("defaultSearchEngine", "naver")
   │            │                 │────────────▶│                │
   │            │                 │             │ LS2 API 호출   │
   │            │                 │             │────────────────│
   │            │                 │             │                │
   │            │                 │ emit searchEngineChanged("naver")
   │            │                 │──────────────────────────────▶│
   │            │                 │             │ setDefaultSearchEngine("naver")
   │            │                 │             │                │
```

---

## 7. URLBar 통합

### validateAndCompleteUrl() 메서드 수정

기존 로직에 검색어 처리 추가:

```cpp
QUrl URLBar::validateAndCompleteUrl(const QString &input) {
    URLValidator::ValidationResult result = URLValidator::validate(input);

    QUrl url;

    // 1. 유효한 URL (프로토콜 포함)
    if (result == URLValidator::ValidationResult::Valid) {
        url = QUrl(input);
    }
    // 2. 프로토콜 누락 (자동 보완 가능)
    else if (result == URLValidator::ValidationResult::MissingScheme) {
        url = QUrl(URLValidator::normalize(input));
    }
    // 3. URL 형식 아님 → 검색어로 판단
    else if (URLValidator::isSearchQuery(input)) {
        qDebug() << "URLBar: 검색어로 판단 -" << input;

        // SearchEngine으로 검색 URL 생성
        url = SearchEngine::createSearchUrl(input, defaultSearchEngine_);

        if (!url.isValid()) {
            showError("검색 URL 생성 실패");
            return QUrl();
        }

        qDebug() << "URLBar: 검색 URL 생성 완료 -" << url.toString();
        return url;
    }
    // 4. 빈 입력 또는 유효하지 않은 형식
    else {
        return QUrl();  // 빈 QUrl 반환
    }

    // XSS 방어: 위험한 스키마 필터링 (기존 로직 유지)
    QStringList dangerousSchemes = {"javascript", "data", "vbscript", "file"};
    if (dangerousSchemes.contains(url.scheme().toLower())) {
        showError("허용되지 않는 URL 형식입니다");
        return QUrl();
    }

    return url;
}
```

### BrowserWindow 통합 (F-11 연동 시)

```cpp
// BrowserWindow.cpp

void BrowserWindow::setupUI() {
    // 기존 UI 설정 유지

    // URLBar 생성 시 기본 검색 엔진 설정
    urlBar_ = new URLBar(this);

    // F-11: SettingsService에서 기본 검색 엔진 로드
    // QString engineId = settingsService_->getSearchEngine();
    // urlBar_->setDefaultSearchEngine(engineId);

    mainLayout_->addWidget(urlBar_);
    // ...
}

void BrowserWindow::setupConnections() {
    // 기존 연결 유지

    // F-11: 검색 엔진 설정 변경 시 URLBar 업데이트
    // connect(settingsService_, &SettingsService::searchEngineChanged,
    //         urlBar_, &URLBar::setDefaultSearchEngine);
}
```

---

## 8. 파일 구조

### 새로 생성할 파일

```
src/services/SearchEngine.h          # 검색 엔진 서비스 헤더
src/services/SearchEngine.cpp        # 검색 엔진 서비스 구현
```

### 수정 필요한 기존 파일

```
src/ui/URLBar.h                      # defaultSearchEngine_ 멤버 추가
src/ui/URLBar.cpp                    # validateAndCompleteUrl() 로직 수정
src/utils/URLValidator.h             # isSearchQuery() 메서드 선언 추가
src/utils/URLValidator.cpp           # isSearchQuery() 메서드 구현 추가
CMakeLists.txt                       # SearchEngine.cpp 추가
```

### CMakeLists.txt 수정

```cmake
set(SOURCES
    # ...
    src/ui/URLBar.cpp
    src/services/SearchEngine.cpp  # 추가
    # ...
)

set(HEADERS
    # ...
    src/ui/URLBar.h
    src/services/SearchEngine.h    # 추가
    # ...
)
```

---

## 9. 영향 범위 분석

### 수정 필요한 기존 파일

#### src/ui/URLBar.h / URLBar.cpp
- **변경 내용**:
  - `defaultSearchEngine_` 멤버 변수 추가 (QString)
  - `setDefaultSearchEngine()`, `getDefaultSearchEngine()` 메서드 추가
  - `validateAndCompleteUrl()` 메서드에 검색어 처리 로직 추가
  - `#include "../services/SearchEngine.h"` 추가
- **충돌 위험**:
  - PG-2 병렬 개발 시 URLBar.cpp 수정 충돌 가능성 있음
  - 해결 방법: validateAndCompleteUrl() 메서드 내부만 수정, 시그니처 변경 없음

#### src/utils/URLValidator.h / URLValidator.cpp
- **변경 내용**:
  - `isSearchQuery(const QString &input)` 정적 메서드 추가
  - 구현: `return !isUrl(input);` (기존 메서드 재사용)
- **충돌 위험**: 없음 (새 메서드 추가만)

### 새로 생성할 파일

#### src/services/SearchEngine.h / SearchEngine.cpp
- **역할**: 검색 URL 생성 및 검색 엔진 목록 관리
- **의존성**: Qt Core (QString, QUrl, QMap)
- **영향**: 없음 (완전히 새로운 클래스)

### 영향 받는 기존 기능

#### F-03 (URL 입력 UI)
- **영향**: URLBar::validateAndCompleteUrl() 메서드 수정
- **조치**: 기존 URL 입력 로직은 그대로 유지, 검색어 처리 로직만 추가
- **회귀 테스트**: 기존 URL 입력 시나리오 정상 동작 확인 필요

#### F-02 (웹뷰 통합)
- **영향**: 없음 (WebView::load(url) 인터페이스 변경 없음)
- **조치**: 검색 URL도 일반 URL과 동일하게 로드됨

#### F-11 (설정 화면) - 향후 기능
- **영향**: 기본 검색 엔진 설정 저장 및 로드
- **조치**: SettingsService에서 URLBar::setDefaultSearchEngine() 호출

#### F-08 (히스토리 관리) - 향후 기능
- **영향**: 검색 결과 페이지도 히스토리에 저장
- **조치**: HistoryService::addHistory() 호출 시 HistoryType::Search 전달

---

## 10. 기술적 주의사항

### URL 인코딩
- **함수**: `QUrl::toPercentEncoding(query)`
- **특징**:
  - UTF-8 인코딩 사용
  - 공백 → `%20`
  - 한글 → `%xx%xx%xx` (3바이트 UTF-8)
  - 특수문자 (`&`, `=`, `?`, `#`, `/` 등) 자동 이스케이프
- **예시**:
  ```cpp
  QUrl::toPercentEncoding("cat videos") → "cat%20videos"
  QUrl::toPercentEncoding("고양이")      → "%EA%B3%A0%EC%96%91%EC%9D%B4"
  QUrl::toPercentEncoding("c++")        → "c%2B%2B"
  ```

### URL 최대 길이
- **제약**: 브라우저 URL 길이 제한 (일반적으로 2048자)
- **대응**:
  - 검색어가 매우 긴 경우 (1500자 이상) 길이 체크 추가 가능
  - 초과 시 경고 메시지 표시 (선택적, M3 이후)
  - SearchEngine::createSearchUrl()에 길이 체크 로직 추가 가능

### XSS 방어
- **검증**: QUrl::toPercentEncoding()이 특수문자를 안전하게 인코딩
- **추가 방어**: URLBar에서 위험한 스키마 필터링 (기존 로직 유지)
- **예시**:
  ```cpp
  QStringList dangerousSchemes = {"javascript", "data", "vbscript", "file"};
  if (dangerousSchemes.contains(url.scheme().toLower())) {
      showError("허용되지 않는 URL 형식입니다");
      return QUrl();
  }
  ```

### 검색 엔진 렌더링 호환성
- **Google, Naver**: webOS WebView에서 기본 렌더링 테스트 필수
- **Bing, DuckDuckGo**: 일부 기능 제한 가능성 (JavaScript 실행 제한)
- **대응**: 테스트 후 렌더링 실패 시 경고 메시지 추가 (M3 이후)

### 메모리 사용량
- **SearchEngine 클래스**: 정적 메서드만 사용, 인스턴스 생성 불필요
- **QMap<QString, SearchEngineInfo>**: 약 1KB 미만 (4개 검색 엔진)
- **URL 인코딩**: 검색어 길이에 비례하지만, 일반적으로 수백 바이트 수준

### 성능 고려사항
- **URL 생성 속도**: 10ms 이내 (단순 문자열 조합)
- **프로파일링 불필요**: QString::replace(), QUrl::toPercentEncoding()은 최적화된 함수
- **비동기 처리 불필요**: 검색 URL 생성은 동기 처리로 충분

---

## 11. 위험 요소 및 완화 방안

### 위험 1: F-03 URLBar 수정 충돌
- **위험**: PG-2 병렬 개발 시 URLBar.cpp 동시 수정으로 Git 충돌 발생 가능
- **완화 방안**:
  - validateAndCompleteUrl() 메서드 내부만 수정, 시그니처 변경 없음
  - F-03 구현 완료 후 F-09 구현 시작 (순차 개발 권장)
  - Git 브랜치 전략: feature/f09-search-engine 브랜치 사용

### 위험 2: 검색 엔진 렌더링 실패
- **위험**: 일부 검색 엔진 (Bing, DuckDuckGo)이 webOS WebView에서 정상 동작하지 않을 가능성
- **완화 방안**:
  - Google, Naver 우선 테스트 (한국 사용자 대상)
  - Bing, DuckDuckGo는 추가 테스트 후 지원 여부 결정
  - 렌더링 실패 시 에러 메시지 표시 (WebView::loadError 시그널)

### 위험 3: URL 인코딩 버그
- **위험**: 특수문자, 이모지 등 일부 문자가 잘못 인코딩될 가능성
- **완화 방안**:
  - QUrl::toPercentEncoding() 단위 테스트 작성
  - 한글, 이모지, 특수기호 (`?`, `&`, `=`, `%`, `#`, `/`) 테스트
  - Qt 공식 문서 참조: https://doc.qt.io/qt-5/qurl.html#toPercentEncoding

### 위험 4: F-11 통합 지연
- **위험**: F-11 설정 화면 개발 지연 시 기본 검색 엔진 변경 기능 미지원
- **완화 방안**:
  - URLBar에 setDefaultSearchEngine() 메서드 미리 구현
  - 기본값 "google"로 하드코딩, F-11 완료 후 LS2 API 연동
  - F-09 단독 기능으로도 동작 가능하도록 설계

---

## 12. 테스트 계획

### 단위 테스트 (Google Test)

#### SearchEngine 테스트

```cpp
TEST(SearchEngineTest, CreateSearchUrl_Google) {
    QUrl url = SearchEngine::createSearchUrl("cat videos", "google");
    EXPECT_EQ(url.toString(), "https://www.google.com/search?q=cat%20videos");
}

TEST(SearchEngineTest, CreateSearchUrl_Naver) {
    QUrl url = SearchEngine::createSearchUrl("고양이", "naver");
    EXPECT_TRUE(url.toString().contains("https://search.naver.com/search.naver?query="));
    EXPECT_TRUE(url.toString().contains("%EA%B3%A0%EC%96%91%EC%9D%B4"));
}

TEST(SearchEngineTest, CreateSearchUrl_EmptyQuery) {
    QUrl url = SearchEngine::createSearchUrl("", "google");
    EXPECT_FALSE(url.isValid());
}

TEST(SearchEngineTest, CreateSearchUrl_UnsupportedEngine) {
    QUrl url = SearchEngine::createSearchUrl("test", "yahoo");
    // Google로 폴백
    EXPECT_TRUE(url.toString().contains("google.com"));
}

TEST(SearchEngineTest, GetAvailableEngines) {
    QStringList engines = SearchEngine::getAvailableEngines();
    EXPECT_EQ(engines.size(), 4);
    EXPECT_TRUE(engines.contains("google"));
    EXPECT_TRUE(engines.contains("naver"));
}
```

#### URLValidator 테스트

```cpp
TEST(URLValidatorTest, IsSearchQuery_True) {
    EXPECT_TRUE(URLValidator::isSearchQuery("cat videos"));
    EXPECT_TRUE(URLValidator::isSearchQuery("youtube"));
    EXPECT_TRUE(URLValidator::isSearchQuery("날씨 서울"));
}

TEST(URLValidatorTest, IsSearchQuery_False) {
    EXPECT_FALSE(URLValidator::isSearchQuery("google.com"));
    EXPECT_FALSE(URLValidator::isSearchQuery("http://example.com"));
    EXPECT_FALSE(URLValidator::isSearchQuery("localhost"));
}
```

### 통합 테스트 (Qt Test)

#### URLBar + SearchEngine 통합

```cpp
TEST(URLBarIntegrationTest, SubmitSearchQuery) {
    BrowserWindow window;
    URLBar *urlBar = window.findChild<URLBar*>();
    WebView *webView = window.findChild<WebView*>();

    QSignalSpy spy(webView, &WebView::loadStarted);

    // 검색어 입력 및 제출
    urlBar->setText("cat videos");
    QTest::keyClick(urlBar, Qt::Key_Enter);

    // WebView::load 호출 확인
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(webView->url().toString().contains("google.com/search?q=cat%20videos"));
}

TEST(URLBarIntegrationTest, UrlVsSearchQuery) {
    URLBar urlBar;

    // URL 입력
    urlBar.setText("google.com");
    QTest::keyClick(&urlBar, Qt::Key_Enter);
    // emit urlSubmitted(QUrl("https://google.com"))

    // 검색어 입력
    urlBar.setText("google");
    QTest::keyClick(&urlBar, Qt::Key_Enter);
    // emit urlSubmitted(QUrl("https://www.google.com/search?q=google"))
}
```

### 수동 테스트 시나리오

#### 시나리오 1: 검색어 입력
1. URLBar에 포커스
2. "cat videos" 입력
3. Enter 키 입력
4. **예상 결과**: Google 검색 결과 페이지 로드 (`https://www.google.com/search?q=cat%20videos`)

#### 시나리오 2: URL과 검색어 혼합
1. URLBar에 "google.com" 입력 → Google 메인 페이지 로드
2. URLBar에 "google" 입력 → Google 검색 결과 페이지 로드 (검색어: "google")
3. URLBar에 "youtube" 입력 → Google 검색 결과 페이지 로드 (검색어: "youtube")

#### 시나리오 3: 한글 검색
1. URLBar에 "고양이 동영상" 입력
2. Enter 키 입력
3. **예상 결과**: Google 검색 결과 페이지 로드, URL에 한글 인코딩 확인

#### 시나리오 4: 특수문자 검색
1. URLBar에 "c++" 입력
2. Enter 키 입력
3. **예상 결과**: Google 검색 결과 페이지 로드, URL에 `c%2B%2B` 확인

---

## 13. F-11 연동 설계 (향후 기능)

### SettingsService 클래스 (F-11에서 구현 예정)

```cpp
// src/services/SettingsService.h

#pragma once

#include <QObject>
#include <QString>

namespace webosbrowser {

class SettingsService : public QObject {
    Q_OBJECT

public:
    explicit SettingsService(QObject *parent = nullptr);
    ~SettingsService() override;

    /**
     * @brief 기본 검색 엔진 설정
     * @param engineId 검색 엔진 ID (예: "google", "naver")
     */
    void setSearchEngine(const QString &engineId);

    /**
     * @brief 기본 검색 엔진 조회
     * @return 검색 엔진 ID (기본값: "google")
     */
    QString getSearchEngine() const;

signals:
    /**
     * @brief 검색 엔진 변경 시그널
     * @param engineId 새 검색 엔진 ID
     */
    void searchEngineChanged(const QString &engineId);

private:
    QString currentSearchEngine_;  ///< 현재 설정된 검색 엔진 ID
    // StorageService 의존성 (LS2 API 호출)
};

} // namespace webosbrowser
```

### BrowserWindow 통합

```cpp
// BrowserWindow.cpp

void BrowserWindow::setupUI() {
    // ...

    urlBar_ = new URLBar(this);

    // SettingsService에서 기본 검색 엔진 로드
    QString engineId = settingsService_->getSearchEngine();
    urlBar_->setDefaultSearchEngine(engineId);

    // ...
}

void BrowserWindow::setupConnections() {
    // ...

    // 검색 엔진 설정 변경 시 URLBar 업데이트
    connect(settingsService_, &SettingsService::searchEngineChanged,
            urlBar_, &URLBar::setDefaultSearchEngine);
}
```

---

## 14. 확장 계획 (M3 이후)

### 검색 엔진 추가
- **방법**: SearchEngine.cpp의 QMap에 항목 추가
- **예시**:
  ```cpp
  {"yahoo", {
      "yahoo",
      "Yahoo",
      "https://search.yahoo.com/search?p={query}",
      ":/icons/yahoo.png"
  }}
  ```

### JSON 외부화
- **목적**: 컴파일 없이 검색 엔진 추가/수정 가능
- **구현**:
  - resources/search_engines.json 파일 생성
  - SearchEngine::init() 정적 메서드로 JSON 로드
  - QJsonDocument, QJsonArray 사용

### 검색 엔진별 추가 파라미터
- **예시**:
  - Google 한국어 결과: `&hl=ko`
  - Naver 통합검색: `&where=nexearch`
- **구현**:
  - SearchEngineInfo에 `QMap<QString, QString> params` 추가
  - createSearchUrl()에서 파라미터 추가 로직 구현

### 검색 히스토리 타입 구분 (F-08 연동)
- **구현**:
  - HistoryItem에 `HistoryType type` 필드 추가
  - enum class HistoryType { Page, Search }
  - 검색 URL 로드 시 HistoryType::Search로 저장

---

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 작성 | F-09 검색 엔진 통합 기술 설계 (Native App Qt/C++) |
