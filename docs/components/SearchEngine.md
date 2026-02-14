# SearchEngine 클래스 문서

## 개요

SearchEngine 클래스는 검색 엔진 통합 서비스를 제공하는 정적 클래스입니다. URL이 아닌 검색어 입력 시 선택된 검색 엔진(Google, Naver, Bing, DuckDuckGo)의 검색 URL을 생성합니다.

**파일 위치**: `src/services/SearchEngine.h`, `src/services/SearchEngine.cpp`

**네임스페이스**: `webosbrowser`

## 지원 검색 엔진

| 검색 엔진 | ID | URL 템플릿 |
|---------|------|-----------|
| Google | `google` | `https://www.google.com/search?q={query}` |
| Naver | `naver` | `https://search.naver.com/search.naver?query={query}` |
| Bing | `bing` | `https://www.bing.com/search?q={query}` |
| DuckDuckGo | `duckduckgo` | `https://duckduckgo.com/?q={query}` |

## 주요 기능

### 1. 검색 URL 생성

```cpp
QString buildSearchUrl(const QString &query, const QString &engineId = QString())
```

검색어를 입력받아 선택된 검색 엔진의 검색 URL을 생성합니다.

**매개변수**:
- `query`: 검색어 (예: "youtube", "고양이 동영상")
- `engineId`: 검색 엔진 ID (생략 시 기본 검색 엔진 사용)

**반환값**: 검색 URL (실패 시 빈 문자열)

**예시**:
```cpp
// Google 검색
QString url = SearchEngine::buildSearchUrl("youtube", "google");
// → "https://www.google.com/search?q=youtube"

// Naver 검색 (한글)
QString url = SearchEngine::buildSearchUrl("고양이 동영상", "naver");
// → "https://search.naver.com/search.naver?query=%EA%B3%A0%EC%96%91%EC%9D%B4%20%EB%8F%99%EC%98%81%EC%83%81"

// 기본 검색 엔진 사용
QString url = SearchEngine::buildSearchUrl("test");
// → Google 검색 (기본값)
```

**주요 처리**:
1. 검색어 공백 제거 및 유효성 검증
2. 검색 엔진 결정 (파라미터 → 설정 → 기본값)
3. URL 인코딩 (`QUrl::toPercentEncoding()`)
4. URL 템플릿에 쿼리 삽입

### 2. 기본 검색 엔진 조회

```cpp
QString getDefaultSearchEngine()
```

현재 설정된 기본 검색 엔진 ID를 조회합니다.

**반환값**: 검색 엔진 ID (기본: `"google"`)

**예시**:
```cpp
QString engine = SearchEngine::getDefaultSearchEngine();
// → "google" (기본값) 또는 저장된 값
```

**저장소**: Qt Settings (`QSettings("LG", "webOSBrowser")`)

### 3. 기본 검색 엔진 설정

```cpp
bool setDefaultSearchEngine(const QString &engineId)
```

기본 검색 엔진을 설정합니다.

**매개변수**:
- `engineId`: 검색 엔진 ID

**반환값**: 설정 성공 여부

**예시**:
```cpp
bool success = SearchEngine::setDefaultSearchEngine("naver");
// → true (성공)

bool success = SearchEngine::setDefaultSearchEngine("invalid");
// → false (유효하지 않은 ID)
```

### 4. 검색 엔진 목록 조회

```cpp
QList<SearchEngineInfo> getAllSearchEngines()
```

지원하는 모든 검색 엔진 목록을 조회합니다. (F-11 설정 UI에서 사용)

**반환값**: 검색 엔진 정보 리스트

**예시**:
```cpp
QList<SearchEngineInfo> engines = SearchEngine::getAllSearchEngines();
// → [{ id: "google", name: "Google", ... }, ...]

for (const auto& engine : engines) {
    qDebug() << engine.name;
}
// 출력: Google, Naver, Bing, DuckDuckGo
```

### 5. 검색 엔진 이름 조회

```cpp
QString getSearchEngineName(const QString &engineId)
```

검색 엔진 ID로 표시 이름을 조회합니다.

**매개변수**:
- `engineId`: 검색 엔진 ID

**반환값**: 검색 엔진 이름 (없으면 `"Unknown"`)

**예시**:
```cpp
QString name = SearchEngine::getSearchEngineName("google");
// → "Google"
```

### 6. 검색어 판단

```cpp
bool isSearchQuery(const QString &input)
```

사용자 입력이 검색어인지 URL인지 판단합니다.

**매개변수**:
- `input`: 사용자 입력 문자열

**반환값**: true면 검색어, false면 URL로 간주

**예시**:
```cpp
SearchEngine::isSearchQuery("youtube");           // → true (검색어)
SearchEngine::isSearchQuery("google.com");         // → false (URL)
SearchEngine::isSearchQuery("https://naver.com"); // → false (URL)
```

**판단 로직**:
1. 프로토콜 포함 (`http://`, `https://`, `ftp://`) → URL
2. 도메인 형식 (`.` 포함, TLD 필수) → URL
3. localhost 또는 IP 주소 → URL
4. 위에 해당하지 않으면 → 검색어

## SearchEngineInfo 구조체

```cpp
struct SearchEngineInfo {
    QString id;             // 검색 엔진 ID
    QString name;           // 표시 이름
    QString urlTemplate;    // URL 템플릿
};
```

## 사용 예시

### URLBar에서 검색어 처리

```cpp
// src/ui/URLBar.cpp

#include "../services/SearchEngine.h"

QUrl URLBar::validateAndCompleteUrl(const QString &input) {
    // 1. 검색어인지 확인
    if (URLValidator::isSearchQuery(input)) {
        QString searchUrl = SearchEngine::buildSearchUrl(input);
        if (!searchUrl.isEmpty()) {
            return QUrl(searchUrl);
        }
    }

    // 2. URL로 처리 (기존 로직)
    // ...
}
```

### F-11 설정 화면에서 검색 엔진 선택

```cpp
// src/views/SettingsPanel.cpp (F-11에서 구현)

#include "../services/SearchEngine.h"

void SettingsPanel::setupSearchEngineSection() {
    // 모든 검색 엔진 조회
    QList<SearchEngineInfo> engines = SearchEngine::getAllSearchEngines();

    // 현재 설정된 엔진
    QString currentEngine = SearchEngine::getDefaultSearchEngine();

    // UI 생성 (라디오 버튼)
    for (const auto& engine : engines) {
        QRadioButton* radio = new QRadioButton(engine.name);
        radio->setChecked(engine.id == currentEngine);

        connect(radio, &QRadioButton::clicked, [engine]() {
            SearchEngine::setDefaultSearchEngine(engine.id);
        });

        layout->addWidget(radio);
    }
}
```

## URL 인코딩

검색어는 `QUrl::toPercentEncoding()`을 사용하여 URL 안전 문자열로 변환됩니다.

**인코딩 예시**:
- `"youtube"` → `"youtube"` (영문)
- `"고양이"` → `"%EA%B3%A0%EC%96%91%EC%9D%B4"` (한글)
- `"고양이 동영상"` → `"%EA%B3%A0%EC%96%91%EC%9D%B4%20%EB%8F%99%EC%98%81%EC%83%81"` (공백 → `%20`)
- `"a+b=c"` → `"a%2Bb%3Dc"` (특수문자)

## 보안 고려사항

### XSS 방지

검색어는 `QUrl::toPercentEncoding()`로 인코딩하여 URL 인젝션을 방지합니다.

```cpp
// 위험: 직접 문자열 삽입 (XSS 가능)
QString badUrl = QString("https://google.com/search?q=%1").arg(query);  // ❌

// 안전: 인코딩 후 삽입
QString encodedQuery = QUrl::toPercentEncoding(query);
QString safeUrl = urlTemplate.replace("{query}", encodedQuery);  // ✅
```

### HTTPS 전용

모든 검색 엔진은 HTTPS만 지원합니다. HTTP 검색 엔진은 미지원.

## 검색 엔진 추가 가이드

새로운 검색 엔진을 추가하려면 `SearchEngine.cpp`의 `getSearchEngines()` 함수만 수정하면 됩니다.

```cpp
// src/services/SearchEngine.cpp

const QMap<QString, SearchEngineInfo>& SearchEngine::getSearchEngines() {
    static const QMap<QString, SearchEngineInfo> engines = {
        // 기존 검색 엔진
        { "google", { ... } },
        { "naver", { ... } },

        // 신규 검색 엔진 추가
        {
            "yahoo",
            {
                "yahoo",
                "Yahoo",
                "https://search.yahoo.com/search?p={query}"
            }
        }
    };
    return engines;
}
```

**주의사항**:
1. ID는 소문자 영문자만 사용
2. URL 템플릿에 `{query}` 플레이스홀더 필수
3. HTTPS URL만 사용
4. 재빌드 필요

## 제약사항

### URL 최대 길이

브라우저 URL 길이 제한 (일반적으로 2048자)이 있으므로, 매우 긴 검색어(1000자 이상)는 인코딩 후 길이 초과 가능.

**현재 처리**: 제한 없이 처리 (실무에서는 드문 케이스)

**향후 확장**: 필요 시 `buildSearchUrl()`에서 검색어 길이 제한 추가 (예: 500자)

### Qt Settings 초기화

앱 데이터 삭제 시 Qt Settings가 초기화되어 기본 검색 엔진 설정이 손실됩니다.

**대응**: `getDefaultSearchEngine()`에서 저장된 값이 없으면 Google을 기본값으로 반환

### 검색 엔진 렌더링 호환성

일부 검색 엔진은 webOS WebView에서 JavaScript 실행 제한으로 정상 렌더링하지 않을 수 있습니다.

**권장**: 실제 디바이스에서 각 검색 엔진 테스트 후 사용

## 테스트

단위 테스트: `tests/unit/SearchEngineTest.cpp` (23개 테스트 케이스)

```bash
# 테스트 실행
cd build
cmake ..
make
./bin/tests/webosbrowser_tests --gtest_filter=SearchEngineTest.*
```

## 변경 이력

| 날짜 | 버전 | 변경 내용 |
|------|------|-----------|
| 2026-02-14 | 0.1.0 | F-09 초기 구현 (Google, Naver, Bing, DuckDuckGo 지원) |

## 관련 문서

- 요구사항 분석서: `docs/specs/search-engine-integration/requirements.md`
- 기술 설계서: `docs/specs/search-engine-integration/design.md`
- 구현 계획서: `docs/specs/search-engine-integration/plan.md`
- URLValidator 문서: `docs/components/URLValidator.md`
