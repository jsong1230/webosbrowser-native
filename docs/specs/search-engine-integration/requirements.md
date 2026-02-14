# 검색 엔진 통합 — 요구사항 분석서 (Native App)

## 1. 개요

### 기능명
검색 엔진 통합 (F-09)

### 목적
URLBar에서 URL이 아닌 검색어를 입력할 경우 자동으로 선택된 검색 엔진(Google, Naver 등)으로 쿼리를 전송하여, 사용자가 별도의 검색 사이트 접속 없이 즉시 검색 결과를 확인할 수 있도록 합니다.

### 대상 사용자
- 검색어를 직접 입력하여 빠르게 정보를 찾고자 하는 프로젝터 사용자
- URL 형식을 정확히 모르지만 키워드로 사이트나 정보를 찾고자 하는 일반 사용자
- 리모컨으로 최소한의 입력만으로 웹 검색을 원하는 엔터테인먼트 사용자

### 요청 배경
- 사용자는 정확한 URL을 모르거나 기억하지 못하는 경우가 많음
- "youtube", "날씨", "고양이 동영상" 등의 검색어로 빠르게 정보 접근 필요
- URL과 검색어를 자동으로 구분하여 사용자 경험 향상
- F-03 URL 입력 UI의 확장 기능으로, URLValidator가 URL로 인식하지 못하면 검색어로 처리
- **Native App 특성**: C++/Qt 기반으로 URL 인코딩 및 검색 엔진 서비스를 구현해야 함

---

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: URL vs 검색어 자동 판단
- **설명**: 사용자 입력이 URL인지 검색어인지 자동으로 판단합니다.
- **유저 스토리**: As a 사용자, I want URL 형식이 아닌 단어를 입력할 때 자동으로 검색 결과가 표시되기를 원합니다, so that URL 형식을 정확히 몰라도 원하는 정보를 찾을 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - **URL로 판단하는 경우**:
    - 프로토콜 포함: `http://`, `https://` (QUrl::scheme() 존재)
    - 도메인 형식: `example.com`, `www.example.com` (점 포함, TLD 필수)
    - localhost/IP: `localhost`, `192.168.1.1` (QHostAddress 유효성 검증)
  - **검색어로 판단하는 경우**:
    - 점(.) 없는 단일 단어: `youtube`, `weather`, `apple`
    - 복수 단어 (공백 포함): `cat videos`, `best restaurants`
    - 특수문자 없는 한글/영문 조합: `날씨 서울`, `오늘 뉴스`
  - **판단 로직**:
    1. `URLValidator::isValid(input)` 실행 (F-03 기존 로직)
    2. `URLValidator::isDomainFormat(input)` 체크 (정규표현식)
    3. 둘 다 false이면 검색어로 간주
    4. `SearchEngine::createSearchUrl(input, engine)` 호출
    5. 생성된 검색 URL을 WebView로 전달
  - **구현 위치**: URLBar::validateAndCompleteUrl() 메서드 수정 (F-03 설계서 참고)

### FR-2: 검색 엔진 선택 및 URL 생성
- **설명**: 사용자가 선택한 검색 엔진(Google, Naver 등)에 맞는 검색 URL을 생성합니다.
- **유저 스토리**: As a 사용자, I want 내가 선호하는 검색 엔진(Naver, Google 등)으로 검색하고 싶습니다, so that 익숙한 검색 결과 UI를 볼 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - **지원 검색 엔진**:
    - Google: `https://www.google.com/search?q={query}`
    - Naver: `https://search.naver.com/search.naver?query={query}`
    - Bing: `https://www.bing.com/search?q={query}`
    - DuckDuckGo: `https://duckduckgo.com/?q={query}`
  - **URL 인코딩**: 검색어는 `QUrl::toPercentEncoding()`로 URL 안전 문자열로 변환
    - Qt 함수로 한글, 공백, 특수문자 자동 인코딩
    - UTF-8 인코딩 사용
  - **기본 검색 엔진**: Google (설정에서 변경 가능, F-11 연동 준비)
  - **검색 엔진 설정 저장**: webOS LS2 API로 `defaultSearchEngine` 키로 저장
    - SettingsService::setSearchEngine(engine) 메서드 (F-11에서 구현)
    - 앱 시작 시 SettingsService::getSearchEngine() 로드
  - **SearchEngine 클래스**:
    - `src/services/SearchEngine.h` / `SearchEngine.cpp`
    - QMap으로 검색 엔진 정의 저장
    - `createSearchUrl(query, engine)` 정적 메서드

### FR-3: 검색어 입력 및 결과 표시
- **설명**: URLBar에서 검색어 입력 후 Enter 키 입력 시 검색 결과 페이지를 WebView에 로드합니다.
- **유저 스토리**: As a 사용자, I want 검색어를 입력하고 확인 버튼을 누르면 즉시 검색 결과가 표시되기를 원합니다, so that 별도의 검색 사이트 접속 없이 빠르게 정보를 얻을 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - 사용자가 URLBar에서 검색어 입력 (예: "cat videos")
  - Enter 키 입력 시 `URLBar::validateAndCompleteUrl(input)` 실행
  - URLValidator가 URL 아님 판단 → SearchEngine::createSearchUrl 호출
  - 생성된 URL 예시: `https://www.google.com/search?q=cat%20videos`
  - `emit urlSubmitted(searchUrl)` → WebView::load(searchUrl)
  - WebView에서 검색 결과 페이지 로드
  - URLBar에 검색 URL이 표시됨 (WebView::urlChanged 시그널로 업데이트)
  - 플로우:
    ```
    사용자 입력 ("cat videos")
      → URLBar::keyPressEvent(Qt::Key_Enter)
      → URLBar::validateAndCompleteUrl("cat videos")
      → URLValidator::isSearchQuery("cat videos") → true
      → SearchEngine::createSearchUrl("cat videos", "google")
      → QUrl("https://www.google.com/search?q=cat%20videos")
      → emit urlSubmitted(url)
      → BrowserWindow::onUrlSubmitted(url)
      → webView_->load(url)
    ```

### FR-4: SearchEngine 서비스 클래스
- **설명**: 검색 엔진 URL 생성 및 검색 엔진 목록 관리를 담당하는 서비스 클래스를 제공합니다.
- **유저 스토리**: As a 개발자, I want 검색 엔진 로직을 재사용 가능한 클래스로 분리하고 싶습니다, so that 코드 중복을 줄이고 유지보수를 쉽게 할 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - **클래스 설계**:
    - 헤더: `src/services/SearchEngine.h`
    - 소스: `src/services/SearchEngine.cpp`
    - 네임스페이스: `webosbrowser`
  - **정적 메서드**:
    - `createSearchUrl(query, engine)`: 검색 URL 생성
    - `getAvailableEngines()`: 지원하는 검색 엔진 목록 반환 (QStringList)
    - `getEngineName(engineId)`: 검색 엔진 ID → 표시 이름 변환
    - `getEngineUrl(engineId)`: 검색 엔진 ID → URL 템플릿 반환
  - **검색 엔진 정의**: QMap<QString, SearchEngineInfo> 구조체
    - `QString id`: 검색 엔진 ID (예: "google", "naver")
    - `QString name`: 표시 이름 (예: "Google", "Naver")
    - `QString urlTemplate`: URL 템플릿 (예: "https://www.google.com/search?q={query}")
  - **URL 인코딩 처리**:
    - QUrl::toPercentEncoding(query, QByteArray(), QByteArray()) 사용
    - UTF-8 인코딩으로 한글, 특수문자 처리
  - **에러 처리**:
    - 지원하지 않는 검색 엔진 ID이면 Google로 폴백
    - 빈 검색어이면 빈 QUrl 반환

### FR-5: 검색 엔진 설정 저장 및 로드 (F-11 연동 준비)
- **설명**: 사용자가 설정한 기본 검색 엔진을 LS2 API로 저장하고, 앱 시작 시 로드합니다.
- **유저 스토리**: As a 사용자, I want 설정한 검색 엔진이 앱 재시작 후에도 유지되기를 원합니다, so that 매번 검색 엔진을 다시 설정하지 않아도 됩니다.
- **우선순위**: Must (F-11 완료 후 통합)
- **상세 요구사항**:
  - **SettingsService 클래스** (F-11에서 구현 예정):
    - `setSearchEngine(engineId)`: 기본 검색 엔진 설정
    - `getSearchEngine()`: 기본 검색 엔진 조회 (기본값: "google")
  - **LS2 API 저장**:
    - StorageService::set("defaultSearchEngine", engineId) 호출
    - JSON 형식으로 저장
  - **앱 시작 시 로드**:
    - BrowserWindow 생성 시 SettingsService::getSearchEngine() 호출
    - URLBar에 기본 검색 엔진 전달 (setDefaultSearchEngine 메서드)
  - **설정 UI** (F-11에서 구현):
    - 설정 패널에 "기본 검색 엔진" 항목 추가
    - 라디오 버튼 또는 드롭다운으로 선택 (QRadioButton, QComboBox)
    - 변경 시 즉시 저장 및 반영

### FR-6: 검색 히스토리 저장 (F-08 연동)
- **설명**: 검색 결과 페이지도 히스토리에 저장하여 이후 자동완성 제안에 포함합니다.
- **유저 스토리**: As a 사용자, I want 이전에 검색했던 키워드를 히스토리에서 다시 찾을 수 있기를 원합니다, so that 같은 검색을 반복할 때 시간을 절약할 수 있습니다.
- **우선순위**: Should (F-08 완료 후 통합)
- **상세 요구사항**:
  - 검색 URL 로드 시 HistoryService::addHistory 호출
  - 히스토리 제목: 검색 엔진 이름 + 검색어 (예: "Google: cat videos")
    - WebView::titleChanged 시그널에서 제목 가져오기
    - 제목이 비어있으면 "Search: 검색어" 형식으로 저장
  - 자동완성에서 검색 히스토리 제안 (URLBar에서 검색어 입력 시)
  - 히스토리 타입 구분 (일반 페이지 vs 검색 결과)
    - HistoryItem에 `type` 필드 추가 (enum: Page, Search)

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **URL 생성 속도**: 검색 URL 생성은 10ms 이내 (단순 문자열 조합)
  - QString::arg(), QUrl::toPercentEncoding()은 빠른 함수
  - 프로파일링 불필요, 성능 영향 무시 가능
- **인코딩 처리**: QUrl::toPercentEncoding() 실행 시 성능 영향 무시 가능
  - Qt 내장 함수로 최적화되어 있음
- **설정 로드 시간**: LS2 API에서 검색 엔진 설정 로드는 50ms 이내
  - 비동기 호출이므로 UI 블로킹 없음

### 확장성
- **검색 엔진 추가 용이성**: 새로운 검색 엔진 추가 시 코드 수정 최소화
  - SearchEngine.cpp의 `engines_` QMap에 항목 추가만으로 가능
  - 설정 UI는 `SearchEngine::getAvailableEngines()` 결과 자동 반영
- **다국어 검색 지원**: 한글, 영문, 일본어 등 다국어 검색어 URL 인코딩 지원
  - QUrl::toPercentEncoding()이 UTF-8 인코딩 자동 처리
- **URL 템플릿 커스터마이징**: 추가 쿼리 파라미터 지원 가능
  - 예: Google 한국어 결과 `&hl=ko` 파라미터 추가
  - URL 템플릿에 `{query}` 외 추가 플레이스홀더 지원 가능

### 호환성
- **주요 검색 엔진 동작 보장**: Google, Naver는 webOS WebView에서 정상 렌더링 확인 필요
  - Bing, DuckDuckGo는 추가 테스트 권장
- **검색 결과 페이지 렌더링**: 검색 엔진의 JavaScript 기반 UI가 webOS에서 동작하는지 테스트
  - Google, Naver는 기본 렌더링 테스트 필수
  - Bing은 일부 기능 제한 가능성 있음

### 보안
- **XSS 방지**: 검색어는 QUrl::toPercentEncoding()로 인코딩하여 URL 인젝션 방지
  - 특수문자 (`<`, `>`, `"`, `'`, `&` 등)는 자동 이스케이프됨
- **안전한 검색 엔진**: 지원하는 검색 엔진은 HTTPS 전용
  - HTTP 검색 엔진은 보안상 미지원
  - SearchEngine 클래스에서 HTTPS URL만 허용

### 메모리 사용량
- **SearchEngine 클래스**: 정적 메서드만 사용, 인스턴스 생성 불필요
  - QMap<QString, SearchEngineInfo> 크기: 약 1KB 미만 (4개 검색 엔진)
- **URL 인코딩**: 검색어 길이에 비례하지만, 일반적으로 수백 바이트 수준

---

## 4. 데이터 구조

### SearchEngineInfo 구조체
```cpp
// src/services/SearchEngine.h

namespace webosbrowser {

struct SearchEngineInfo {
    QString id;          ///< 검색 엔진 ID (예: "google", "naver")
    QString name;        ///< 표시 이름 (예: "Google", "Naver")
    QString urlTemplate; ///< URL 템플릿 (예: "https://www.google.com/search?q={query}")
    QString iconPath;    ///< 아이콘 경로 (선택적, F-11 UI에서 사용)
};

} // namespace webosbrowser
```

### 검색 엔진 정의 (SearchEngine.cpp)
```cpp
// src/services/SearchEngine.cpp

namespace webosbrowser {

static const QMap<QString, SearchEngineInfo> engines_ = {
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

} // namespace webosbrowser
```

### 설정 저장 형식 (LS2 API, F-11 연동)
```json
// StorageService::set("defaultSearchEngine", "naver") 호출 시 저장 형식

{
  "key": "defaultSearchEngine",
  "value": "naver"
}
```

### 히스토리 항목 (F-08 연동 시)
```cpp
// src/services/HistoryService.h

namespace webosbrowser {

enum class HistoryType {
    Page,    ///< 일반 웹 페이지
    Search   ///< 검색 결과 페이지
};

struct HistoryItem {
    QString url;         ///< 방문한 URL
    QString title;       ///< 페이지 제목 (예: "Google: cat videos")
    qint64 visitedAt;    ///< 방문 시각 (Unix 타임스탬프)
    HistoryType type;    ///< 히스토리 타입 (Page or Search)
};

} // namespace webosbrowser
```

---

## 5. 제약조건

### 기술 제약사항
- **URL 최대 길이**: 브라우저 URL 길이 제한 (일반적으로 2048자) 고려
  - 검색어가 매우 긴 경우 URL 인코딩 후 길이 초과 가능
  - 초과 시 검색어 자르기 또는 에러 메시지 표시 (선택)
  - SearchEngine::createSearchUrl()에서 길이 체크 추가 가능
- **특수문자 처리**: QUrl::toPercentEncoding()가 모든 특수문자를 안전하게 인코딩하는지 테스트 필요
  - 한글, 이모지, 특수기호 (?, &, =, %, # 등)
  - Qt 문서 참조: https://doc.qt.io/qt-5/qurl.html#toPercentEncoding

### webOS 플랫폼 제약사항
- **검색 엔진 렌더링**: 일부 검색 엔진은 webOS WebView에서 JavaScript 실행 제한으로 정상 동작하지 않을 수 있음
  - Google, Naver는 기본 렌더링 테스트 필수
  - Bing은 일부 기능 제한 가능성 (예: 이미지 검색, 지도)
  - DuckDuckGo는 경량 UI로 webOS에서 잘 동작할 가능성 높음
- **네트워크 제약**: webOS 네트워크 스택이 일부 검색 엔진 API 호출 차단 가능성
  - CORS 정책으로 자동완성 API 호출 제한 (검색 자동완성 기능 미지원 이유)

### 의존성
- **F-01 (프로젝트 초기 설정)**: Qt 프로젝트 구조 및 CMake 빌드 설정 완료 필요
- **F-03 (URL 입력 UI)**: URLBar 컴포넌트와 URLValidator 유틸리티 필요
  - URLBar::validateAndCompleteUrl() 메서드 수정
  - URLValidator::isSearchQuery() 메서드 추가
- **F-02 (웹뷰 통합)**: 생성된 검색 URL을 WebView에 로드
  - WebView::load(const QUrl &url) 메서드
- **F-11 (설정 화면)**: (선택) 기본 검색 엔진 선택 UI
  - SettingsService::setSearchEngine(), getSearchEngine() 메서드
- **F-08 (히스토리 관리)**: (선택) 검색 히스토리 저장 및 자동완성 연동
  - HistoryService::addHistory() 메서드

### 범위 외 (Out of Scope)
- **고급 검색 옵션**: 이미지 검색, 뉴스 검색, 기간 필터 등 미지원
- **검색 자동완성**: 검색 엔진의 자동완성 제안 API 연동 미지원
  - Google Suggest API, Naver 자동완성 API는 CORS 제약으로 사용 불가
  - URLBar 자체 자동완성(히스토리, 북마크 기반)만 지원
- **검색 엔진별 필터링**: "site:naver.com" 같은 고급 쿼리 미지원 (사용자가 직접 입력 가능)
- **음성 검색**: 음성 입력 기능 미지원 (향후 확장 가능)
- **검색 엔진 커스터마이징**: 사용자 정의 검색 엔진 추가 기능 미지원

---

## 6. 용어 정의

| 용어 | 정의 |
|------|------|
| **검색 엔진** | 사용자 입력 검색어를 받아 웹 검색 결과를 제공하는 서비스 (Google, Naver 등) |
| **검색어** | URL 형식이 아닌 일반 텍스트 입력 (예: "cat videos", "날씨") |
| **검색 URL** | 검색 엔진의 쿼리 파라미터에 검색어를 포함한 전체 URL |
| **URL 인코딩** | 특수문자, 공백, 다국어를 URL 안전 문자열로 변환하는 과정 (QUrl::toPercentEncoding) |
| **기본 검색 엔진** | 사용자가 설정한 기본 검색 엔진 (설정하지 않으면 Google) |
| **검색 히스토리** | 검색어로 접근한 검색 결과 페이지의 방문 기록 |
| **URL 템플릿** | 검색 URL 생성을 위한 패턴 문자열 (예: "https://www.google.com/search?q={query}") |
| **LS2 API** | webOS 로컬 스토리지 API (비동기 메시지 버스 기반) |
| **QUrl::toPercentEncoding** | Qt의 URL 인코딩 함수 (UTF-8 기반) |

---

## 7. 완료 기준 (Acceptance Criteria)

### AC-1: URL vs 검색어 자동 판단
- [ ] `google.com` 입력 시 → `http://google.com`로 변환 (URL로 인식)
- [ ] `youtube` 입력 시 → 검색 URL 생성 (검색어로 인식)
- [ ] `cat videos` 입력 시 → 검색 URL 생성 (검색어로 인식)
- [ ] `https://example.com` 입력 시 → URL로 그대로 사용
- [ ] URLValidator::isSearchQuery() 메서드가 정상 동작

### AC-2: 검색 URL 생성
- [ ] Google 선택 시 `https://www.google.com/search?q=cat%20videos` 형식 생성
- [ ] Naver 선택 시 `https://search.naver.com/search.naver?query=cat%20videos` 형식 생성
- [ ] 검색어에 공백 포함 시 `%20`으로 인코딩됨
- [ ] 한글 검색어 정상 인코딩됨 (예: "고양이" → "%EA%B3%A0%EC%96%91%EC%9D%B4")
- [ ] 특수문자 검색어 정상 인코딩됨 (예: "c++", "a&b")

### AC-3: 검색 결과 페이지 로드
- [ ] URLBar에서 검색어 입력 → Enter 키 입력 시 WebView에서 검색 결과 페이지 로드
- [ ] Google에서 `youtube` 검색 시 검색 결과 페이지 정상 표시
- [ ] Naver에서 `고양이` 검색 시 검색 결과 페이지 정상 표시
- [ ] URLBar에 최종 검색 URL 표시 (WebView::urlChanged 연결)

### AC-4: SearchEngine 클래스 동작
- [ ] `SearchEngine::createSearchUrl("cat", "google")` 호출 시 올바른 URL 반환
- [ ] `SearchEngine::getAvailableEngines()` 호출 시 ["google", "naver", "bing", "duckduckgo"] 반환
- [ ] 지원하지 않는 검색 엔진 ID이면 Google로 폴백
- [ ] 빈 검색어 입력 시 빈 QUrl 반환

### AC-5: 검색 엔진 설정 저장 및 로드 (F-11 연동 시)
- [ ] SettingsService::setSearchEngine("naver") 호출 시 LS2 API로 저장
- [ ] 앱 재시작 후 SettingsService::getSearchEngine() 호출 시 "naver" 반환
- [ ] 설정 변경 시 즉시 다음 검색에 반영
- [ ] 설정 UI에서 검색 엔진 선택 가능 (F-11)

### AC-6: 검색 히스토리 저장 (F-08 연동 시)
- [ ] 검색 결과 페이지 로드 시 HistoryService::addHistory 호출
- [ ] 히스토리 제목: "Google: cat videos" 형식
- [ ] 히스토리 타입: HistoryType::Search로 저장
- [ ] 히스토리 패널에서 검색 기록 조회 가능

### AC-7: 에러 처리
- [ ] 검색어가 비어있으면 에러 메시지 표시 (URLBar::showError)
- [ ] 검색 URL이 2048자를 초과하면 경고 메시지 표시 (선택)
- [ ] 지원하지 않는 검색 엔진 설정값이면 Google로 폴백

### AC-8: 회귀 테스트
- [ ] 기존 URL 입력 기능 정상 동작 (검색 엔진 통합 후에도 URL 직접 입력 가능)
- [ ] 자동완성 기능과 충돌 없음 (F-03)
- [ ] 리모컨 조작으로 모든 기능 사용 가능

---

## 8. UI/UX 플로우

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
   │         │             │ isValid("cat") → false    │
   │         │             │ isDomainFormat("cat") → false
   │         │             │ isSearchQuery("cat") → true
   │         │◀────────────│              │            │
   │         │             │              │            │
   │         │ createSearchUrl("cat", "google")        │
   │         │──────────────────────────▶│            │
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
  → URLValidator::isValid("google.com") → true
  → URLValidator::autoComplete("google.com") → QUrl("http://google.com")
  → WebView 로드

입력: "google"
  → URLValidator::isValid("google") → false
  → URLValidator::isDomainFormat("google") → false (점 없음)
  → URLValidator::isSearchQuery("google") → true
  → SearchEngine::createSearchUrl("google", "google")
  → QUrl("https://www.google.com/search?q=google")
  → WebView 로드

입력: "cat videos"
  → URLValidator::isValid("cat videos") → false (공백 포함)
  → URLValidator::isDomainFormat("cat videos") → false
  → URLValidator::isSearchQuery("cat videos") → true
  → SearchEngine::createSearchUrl("cat videos", "google")
  → QUrl("https://www.google.com/search?q=cat%20videos")
  → WebView 로드
```

### 시나리오 3: 검색 엔진 변경 (F-11 연동)
```
사용자 → SettingsPanel → SettingsService → URLBar
   │            │                 │             │
   │ 설정 열기  │                 │             │
   │───────────▶│                 │             │
   │            │ "Naver" 선택    │             │
   │───────────▶│                 │             │
   │            │ setSearchEngine("naver")      │
   │            │────────────────▶│             │
   │            │                 │ StorageService::set("defaultSearchEngine", "naver")
   │            │                 │ (LS2 API 호출)
   │            │                 │             │
   │            │                 │ emit searchEngineChanged("naver")
   │            │                 │────────────▶│
   │            │                 │             │ defaultEngine_ = "naver"
```

---

## 9. 참고 사항

### 관련 문서
- PRD: `docs/project/prd.md`
- 기능 백로그: `docs/project/features.md`
- F-03 요구사항 분석서: `docs/specs/url-input-ui/requirements.md`
- F-03 기술 설계서: `docs/specs/url-input-ui/design.md`
- CLAUDE.md: 프로젝트 규칙 및 코딩 컨벤션

### 참고 구현
- Qt QUrl 클래스: https://doc.qt.io/qt-5/qurl.html
- Qt QUrl::toPercentEncoding: https://doc.qt.io/qt-5/qurl.html#toPercentEncoding
- Chrome 브라우저의 Omnibox (주소창) 동작 참고
  - URL과 검색어 자동 판단
  - 기본 검색 엔진 설정

### 검색 엔진별 URL 패턴
- **Google**: `https://www.google.com/search?q={query}`
  - 추가 파라미터: `&hl=ko` (한국어 결과)
- **Naver**: `https://search.naver.com/search.naver?query={query}`
  - 추가 파라미터: `&where=nexearch` (통합검색)
- **Bing**: `https://www.bing.com/search?q={query}`
  - 추가 파라미터: `&setlang=ko` (한국어 결과)
- **DuckDuckGo**: `https://duckduckgo.com/?q={query}`
  - 추가 파라미터: `&kl=kr-kr` (한국 지역)

### 구현 권장사항
- **SearchEngine 클래스**: 정적 메서드만 사용, 싱글톤 패턴 불필요
  - 인스턴스 생성 금지 (생성자 private)
- **URL 인코딩**: QUrl::toPercentEncoding() 사용 권장
  - QString::toUtf8() + 수동 인코딩보다 안전
- **검색 엔진 정의**: QMap 대신 std::unordered_map 사용 가능 (성능 향상)
  - 단, Qt 컨테이너 우선 사용 원칙 고려

### 우선순위 조정 가능성
- 검색 히스토리 저장 (FR-6)은 Should 우선순위로, F-08 완료 후 추가 가능
- 검색 엔진 아이콘 표시는 M3 이후 UI 개선으로 연기 가능
- Bing, DuckDuckGo 지원은 Google, Naver 테스트 후 추가 결정

---

## 10. 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|-----------|--------|
| 2026-02-14 | 1.0 | Native App (Qt/C++) 관점으로 요구사항 작성 | product-manager |
