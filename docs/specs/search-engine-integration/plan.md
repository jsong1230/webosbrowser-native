# 검색 엔진 통합 — 구현 계획서 (F-09)

## 1. 참조 문서
- 요구사항 분석서: docs/specs/search-engine-integration/requirements.md
- 기술 설계서: docs/specs/search-engine-integration/design.md
- F-03 URL 입력 UI 설계서: docs/specs/url-input-ui/design.md
- CLAUDE.md: 프로젝트 규칙 및 코딩 컨벤션

---

## 2. 개요

### 2.1 기능 요약
URL이 아닌 검색어를 입력할 경우 자동으로 선택된 검색 엔진(Google, Naver 등)으로 쿼리를 전송하여, 사용자가 별도의 검색 사이트 접속 없이 즉시 검색 결과를 확인할 수 있는 기능을 구현합니다.

### 2.2 구현 목표
- SearchEngine 서비스 클래스 구현 (정적 메서드, 검색 URL 생성)
- URLValidator 확장 (isSearchQuery 메서드 추가)
- URLBar 수정 (URL vs 검색어 판단 로직 통합)
- 4개 주요 검색 엔진 지원 (Google, Naver, Bing, DuckDuckGo)
- URL 인코딩 처리 (한글, 특수문자, 공백)
- F-11 설정 화면 연동 준비 (기본 검색 엔진 변경)

### 2.3 의존성 체크
- **F-01 (프로젝트 초기 설정)**: ✅ 완료 (Qt/CMake 프로젝트 구조)
- **F-02 (웹뷰 통합)**: ✅ 완료 (WebView::load 메서드)
- **F-03 (URL 입력 UI)**: ✅ 완료 (URLBar, URLValidator)
- **F-11 (설정 화면)**: ⏳ 미완료 (기본 검색 엔진 설정 연동은 F-11 완료 후)

---

## 3. 구현 Phase

### Phase 1: SearchEngine 서비스 클래스 구현
**담당**: cpp-dev
**예상 소요 시간**: 2시간
**의존성**: 없음 (독립적 구현 가능)

#### 태스크 목록
- [ ] Task 1-1: SearchEngine.h 헤더 파일 작성
  - SearchEngineInfo 구조체 정의
  - SearchEngine 클래스 선언 (정적 메서드만)
  - 메서드: createSearchUrl, getAvailableEngines, getEngineName, getEngineUrlTemplate, getDefaultEngineId
  - 생성자 금지 (= delete)
- [ ] Task 1-2: SearchEngine.cpp 구현
  - QMap<QString, SearchEngineInfo> engines_ 정적 멤버 초기화
  - 4개 검색 엔진 정의 (Google, Naver, Bing, DuckDuckGo)
  - createSearchUrl 구현 (QUrl::toPercentEncoding 사용)
  - 빈 검색어 체크, 지원하지 않는 엔진 폴백 로직
  - 각 메서드 구현 (getAvailableEngines, getEngineName 등)
- [ ] Task 1-3: CMakeLists.txt 수정
  - SOURCES에 src/services/SearchEngine.cpp 추가
  - HEADERS에 src/services/SearchEngine.h 추가

#### 산출물
```
src/services/SearchEngine.h      # 검색 엔진 서비스 헤더
src/services/SearchEngine.cpp    # 검색 엔진 서비스 구현
CMakeLists.txt                    # 빌드 설정 업데이트
```

#### 완료 기준
- SearchEngine::createSearchUrl("cat", "google") 호출 시 올바른 URL 반환
- QUrl::toPercentEncoding()로 검색어 인코딩 처리
- 지원하지 않는 검색 엔진 ID이면 Google로 폴백
- getAvailableEngines() 호출 시 4개 검색 엔진 목록 반환
- 컴파일 에러 없음, 빌드 성공

---

### Phase 2: URLValidator 확장
**담당**: cpp-dev
**예상 소요 시간**: 30분
**의존성**: 없음 (F-03 완료 상태)

#### 태스크 목록
- [ ] Task 2-1: URLValidator.h 수정
  - isSearchQuery(const QString &input) 정적 메서드 선언 추가
  - 주석: "검색어 여부 판단 (URL이 아닌 경우)"
- [ ] Task 2-2: URLValidator.cpp 구현
  - isSearchQuery() 구현: return !isUrl(input);
  - 기존 isUrl() 메서드 재사용

#### 산출물
```
src/utils/URLValidator.h         # isSearchQuery 메서드 선언 추가
src/utils/URLValidator.cpp       # isSearchQuery 메서드 구현 추가
```

#### 완료 기준
- URLValidator::isSearchQuery("cat videos") → true
- URLValidator::isSearchQuery("google.com") → false
- URLValidator::isSearchQuery("http://example.com") → false
- 기존 URLValidator 메서드 정상 동작 (회귀 테스트)
- 컴파일 에러 없음

---

### Phase 3: URLBar 수정 (검색어 처리 통합)
**담당**: cpp-dev
**예상 소요 시간**: 1.5시간
**의존성**: Phase 1, Phase 2 완료 필요

#### 태스크 목록
- [ ] Task 3-1: URLBar.h 수정
  - defaultSearchEngine_ 멤버 변수 추가 (QString, 기본값: "google")
  - setDefaultSearchEngine(engineId) 메서드 선언
  - getDefaultSearchEngine() const 메서드 선언
- [ ] Task 3-2: URLBar.cpp 수정
  - 생성자에서 defaultSearchEngine_ 초기화 ("google")
  - #include "../services/SearchEngine.h" 추가
  - validateAndCompleteUrl() 메서드 수정:
    - ValidationResult::Valid → 기존 로직 (URL로 처리)
    - ValidationResult::MissingScheme → 기존 로직 (프로토콜 자동 보완)
    - URLValidator::isSearchQuery(input) → true인 경우 검색어 처리
      - SearchEngine::createSearchUrl(input, defaultSearchEngine_) 호출
      - 생성된 URL이 유효하지 않으면 에러 메시지 표시
      - 유효하면 QUrl 반환
    - 기존 XSS 방어 로직 유지
  - setDefaultSearchEngine(), getDefaultSearchEngine() 구현
- [ ] Task 3-3: 디버깅 로그 추가
  - 검색어 판단 시 qDebug() 출력
  - 검색 URL 생성 완료 시 qDebug() 출력

#### 산출물
```
src/ui/URLBar.h                   # 멤버 변수 및 메서드 추가
src/ui/URLBar.cpp                 # 검색어 처리 로직 통합
```

#### 완료 기준
- "cat videos" 입력 → SearchEngine::createSearchUrl 호출 → 검색 URL 생성
- "google.com" 입력 → 기존 URL 처리 로직 (http://google.com)
- "youtube" 입력 → 검색 URL 생성
- 기존 URL 입력 기능 정상 동작 (회귀 테스트)
- setDefaultSearchEngine("naver") 호출 후 검색 시 Naver 검색 URL 생성
- 컴파일 에러 없음, 빌드 성공

---

### Phase 4: 통합 테스트 및 디버깅
**담당**: test-runner
**예상 소요 시간**: 2시간
**의존성**: Phase 3 완료 필요

#### 태스크 목록
- [ ] Task 4-1: SearchEngine 단위 테스트 작성
  - tests/unit/SearchEngineTest.cpp 생성
  - createSearchUrl() 테스트 (Google, Naver, 한글, 특수문자)
  - getAvailableEngines() 테스트
  - 지원하지 않는 검색 엔진 폴백 테스트
  - 빈 검색어 테스트
- [ ] Task 4-2: URLValidator 단위 테스트 확장
  - tests/unit/URLValidatorTest.cpp 수정
  - isSearchQuery() 메서드 테스트 추가
- [ ] Task 4-3: URLBar 통합 테스트 작성
  - tests/integration/URLBarIntegrationTest.cpp 수정
  - 검색어 입력 시나리오 테스트
  - URL vs 검색어 판단 테스트
  - 검색 엔진 변경 테스트
- [ ] Task 4-4: 수동 테스트
  - 시나리오 1: "cat videos" 입력 → Google 검색 결과 페이지 로드 확인
  - 시나리오 2: "고양이" 입력 → 한글 인코딩 확인
  - 시나리오 3: "c++" 입력 → 특수문자 인코딩 확인
  - 시나리오 4: "google.com" vs "google" 판단 확인
- [ ] Task 4-5: 회귀 테스트
  - 기존 URL 입력 기능 정상 동작 확인 (F-03)
  - URLBar의 자동완성 기능과 충돌 없음 확인

#### 산출물
```
tests/unit/SearchEngineTest.cpp       # SearchEngine 단위 테스트
tests/unit/URLValidatorTest.cpp       # URLValidator 테스트 확장
tests/integration/URLBarIntegrationTest.cpp  # URLBar 통합 테스트
```

#### 완료 기준
- 모든 단위 테스트 통과
- 통합 테스트 통과
- 수동 테스트 시나리오 모두 성공
- 회귀 테스트 통과 (기존 기능 영향 없음)
- 로그에 검색어 판단 및 URL 생성 메시지 출력 확인

---

### Phase 5: 코드 리뷰 및 문서화
**담당**: code-reviewer
**예상 소요 시간**: 1시간
**의존성**: Phase 4 완료 필요

#### 태스크 목록
- [ ] Task 5-1: 코드 리뷰
  - SearchEngine 클래스 설계 검토 (정적 메서드만 사용)
  - URL 인코딩 로직 검증 (QUrl::toPercentEncoding)
  - URLBar 수정 사항 검토 (validateAndCompleteUrl)
  - 코딩 컨벤션 준수 확인 (CLAUDE.md)
  - 주석 및 문서화 수준 확인
- [ ] Task 5-2: 문서 업데이트
  - docs/dev-log.md 업데이트 (F-09 구현 완료)
  - CHANGELOG.md 업데이트 (F-09 추가)
  - README.md 업데이트 (검색 기능 설명 추가)
- [ ] Task 5-3: PR 생성
  - feature/f09-search-engine 브랜치 → main 병합 PR 생성
  - PR 설명: 구현 내용, 테스트 결과, 스크린샷 첨부
  - 리뷰어 지정

#### 산출물
```
docs/dev-log.md                   # 개발 로그 업데이트
CHANGELOG.md                      # 변경 이력 업데이트
README.md                         # 프로젝트 문서 업데이트
```

#### 완료 기준
- 코드 리뷰 통과 (code-reviewer 승인)
- 문서 업데이트 완료
- PR 생성 및 병합 완료
- main 브랜치에 F-09 코드 반영

---

## 4. 태스크 의존성 다이어그램

```
Phase 1 (SearchEngine 구현)
   │
   ├─ Task 1-1: SearchEngine.h 작성
   │      ▼
   ├─ Task 1-2: SearchEngine.cpp 구현
   │      ▼
   └─ Task 1-3: CMakeLists.txt 수정
         ▼
Phase 2 (URLValidator 확장)
   │
   ├─ Task 2-1: URLValidator.h 수정
   │      ▼
   └─ Task 2-2: URLValidator.cpp 구현
         ▼
Phase 1, 2 완료 ──▶ Phase 3 (URLBar 수정)
   │
   ├─ Task 3-1: URLBar.h 수정
   │      ▼
   ├─ Task 3-2: URLBar.cpp 수정
   │      ▼
   └─ Task 3-3: 디버깅 로그 추가
         ▼
Phase 3 완료 ──▶ Phase 4 (테스트)
   │
   ├─ Task 4-1: SearchEngine 단위 테스트
   ├─ Task 4-2: URLValidator 단위 테스트
   ├─ Task 4-3: URLBar 통합 테스트
   ├─ Task 4-4: 수동 테스트
   │      ▼
   └─ Task 4-5: 회귀 테스트
         ▼
Phase 4 완료 ──▶ Phase 5 (리뷰 및 문서화)
   │
   ├─ Task 5-1: 코드 리뷰
   ├─ Task 5-2: 문서 업데이트
   │      ▼
   └─ Task 5-3: PR 생성 및 병합
         ▼
      F-09 완료 ✅
```

---

## 5. 파일 작업 목록

### 5.1 새로 생성할 파일

| 파일 경로 | 설명 | 담당 Phase |
|-----------|------|------------|
| src/services/SearchEngine.h | 검색 엔진 서비스 헤더 | Phase 1 |
| src/services/SearchEngine.cpp | 검색 엔진 서비스 구현 | Phase 1 |
| tests/unit/SearchEngineTest.cpp | SearchEngine 단위 테스트 | Phase 4 |

### 5.2 수정할 기존 파일

| 파일 경로 | 수정 내용 | 담당 Phase |
|-----------|-----------|------------|
| src/utils/URLValidator.h | isSearchQuery() 메서드 선언 추가 | Phase 2 |
| src/utils/URLValidator.cpp | isSearchQuery() 메서드 구현 추가 | Phase 2 |
| src/ui/URLBar.h | defaultSearchEngine_ 멤버 변수 및 메서드 추가 | Phase 3 |
| src/ui/URLBar.cpp | validateAndCompleteUrl() 로직 수정, SearchEngine 통합 | Phase 3 |
| CMakeLists.txt | SearchEngine.cpp 추가 | Phase 1 |
| tests/unit/URLValidatorTest.cpp | isSearchQuery() 테스트 추가 | Phase 4 |
| tests/integration/URLBarIntegrationTest.cpp | 검색어 입력 시나리오 테스트 추가 | Phase 4 |
| docs/dev-log.md | F-09 구현 완료 로그 | Phase 5 |
| CHANGELOG.md | F-09 변경 이력 추가 | Phase 5 |
| README.md | 검색 기능 설명 추가 | Phase 5 |

---

## 6. 병렬 실행 판단

### 6.1 병렬 가능한 태스크
- **Phase 1과 Phase 2는 병렬 실행 가능**
  - SearchEngine 클래스와 URLValidator 확장은 서로 독립적
  - 충돌 위험 없음 (다른 파일 작업)
- **Phase 4 내 일부 태스크 병렬 가능**
  - Task 4-1 (SearchEngine 테스트)과 Task 4-2 (URLValidator 테스트)는 병렬 가능
  - Task 4-4 (수동 테스트)와 Task 4-3 (통합 테스트)는 병렬 가능

### 6.2 순차 실행 필요한 태스크
- **Phase 1, 2 → Phase 3 → Phase 4 → Phase 5는 순차 실행 필수**
  - Phase 3는 Phase 1, 2의 산출물 필요 (SearchEngine, URLValidator)
  - Phase 4는 Phase 3 완료 필요 (테스트 대상 코드 존재)
  - Phase 5는 Phase 4 완료 필요 (테스트 통과 후 리뷰)

### 6.3 Agent Team 사용 권장 여부
**권장: No (단독 개발 권장)**

#### 이유
1. **복잡도 낮음**: 총 5개 Phase, 소규모 변경 사항 (2개 클래스 추가, 2개 클래스 수정)
2. **의존성 단순**: 선형적 의존성 (1, 2 → 3 → 4 → 5)
3. **충돌 위험 최소**: URLBar.cpp 수정만 주의하면 됨 (F-03과 공유)
4. **개발 시간 짧음**: 총 7시간 예상 (단독 개발로 충분)
5. **코드 일관성**: 단일 개발자가 전체 흐름을 이해하고 구현하는 것이 유리

#### 단독 개발 시 장점
- 코드 일관성 유지
- 컨텍스트 스위칭 비용 감소
- Git 충돌 위험 최소화
- 디버깅 용이

#### Agent Team 사용 시 고려사항 (선택적)
- Phase 1과 Phase 2를 서로 다른 에이전트에 병렬 할당 가능
- 단, Phase 3는 단일 에이전트가 처리 권장 (URLBar 수정)
- 전체 구현 시간이 1일 이내이므로 병렬화의 실익 적음

---

## 7. 리스크 및 대응 방안

### 리스크 1: F-03 URLBar 수정 충돌
- **위험도**: 중간 (Medium)
- **영향**: Git 충돌 발생 가능성, F-03과 코드 동시 수정
- **대응 방안**:
  - F-03 구현 완료 후 F-09 시작 (순차 개발 권장)
  - feature/f09-search-engine 브랜치 사용
  - validateAndCompleteUrl() 메서드 내부만 수정, 시그니처 변경 없음
  - 변경 전후 diff 확인 및 코드 리뷰 강화
- **회귀 테스트**: URLBar 기존 기능 정상 동작 확인 필수

### 리스크 2: URL vs 검색어 판단 오류
- **위험도**: 중간 (Medium)
- **영향**: 사용자가 의도한 URL이 검색어로 잘못 판단되거나, 반대로 검색어가 URL로 판단됨
- **대응 방안**:
  - URLValidator::isUrl() 정규식 정확도 검증 (F-03 테스트 재확인)
  - 엣지 케이스 테스트 추가:
    - "localhost" (URL)
    - "192.168.1.1" (URL)
    - "example" (검색어)
    - "example.com" (URL)
    - "cat videos" (검색어)
  - 잘못된 판단 시 사용자 피드백 수집 (로그 분석)
  - 향후 판단 로직 개선 (M3 이후)

### 리스크 3: 검색 엔진 렌더링 실패
- **위험도**: 낮음 (Low)
- **영향**: 일부 검색 엔진 (Bing, DuckDuckGo)이 webOS WebView에서 정상 동작하지 않을 가능성
- **대응 방안**:
  - Google, Naver 우선 테스트 (한국 사용자 대상)
  - Bing, DuckDuckGo는 추가 테스트 후 지원 여부 결정
  - WebView::loadError 시그널 연결하여 렌더링 실패 감지
  - 렌더링 실패 시 에러 메시지 표시 (M3 이후)
  - 최악의 경우 해당 검색 엔진 제거 (QMap에서 삭제)

### 리스크 4: URL 인코딩 버그
- **위험도**: 낮음 (Low)
- **영향**: 특수문자, 이모지 등 일부 문자가 잘못 인코딩되어 검색 결과 오류
- **대응 방안**:
  - QUrl::toPercentEncoding() Qt 내장 함수 사용 (신뢰성 높음)
  - 단위 테스트로 다양한 입력 검증:
    - 한글: "고양이"
    - 공백: "cat videos"
    - 특수문자: "c++", "a&b", "100%"
    - 이모지: "😀"
  - Qt 공식 문서 참조: https://doc.qt.io/qt-5/qurl.html#toPercentEncoding
  - 인코딩 실패 시 에러 메시지 표시

### 리스크 5: F-11 통합 지연
- **위험도**: 낮음 (Low)
- **영향**: 기본 검색 엔진 설정 변경 기능 미지원
- **대응 방안**:
  - URLBar에 setDefaultSearchEngine() 메서드 미리 구현 (Phase 3)
  - 기본값 "google"로 하드코딩, F-11 완료 후 LS2 API 연동
  - F-09 단독 기능으로도 동작 가능하도록 설계
  - SettingsService 인터페이스 미리 정의 (design.md 참조)
  - F-11 완료 시 BrowserWindow::setupConnections()만 추가하면 됨

---

## 8. 테스트 전략

### 8.1 단위 테스트 (Google Test)

#### SearchEngine 테스트
- createSearchUrl() 기능 검증
  - Google, Naver, Bing, DuckDuckGo 각각 테스트
  - 한글 검색어 인코딩 검증
  - 공백 포함 검색어 인코딩 검증
  - 특수문자 검색어 인코딩 검증
- getAvailableEngines() 검증
- 지원하지 않는 검색 엔진 폴백 검증
- 빈 검색어 처리 검증

#### URLValidator 테스트
- isSearchQuery() 메서드 검증
  - URL 입력 시 false 반환
  - 검색어 입력 시 true 반환
  - 엣지 케이스 테스트 (localhost, IP, 도메인 형식 등)

### 8.2 통합 테스트 (Qt Test)

#### URLBar + SearchEngine 통합
- 검색어 입력 및 제출 시나리오
- URL vs 검색어 판단 시나리오
- 검색 엔진 변경 시나리오 (setDefaultSearchEngine 호출)
- WebView 로드 시그널 연결 확인

#### 회귀 테스트
- 기존 URL 입력 기능 정상 동작 확인 (F-03)
- URLBar 자동완성 기능과 충돌 없음 확인
- WebView 로드 기능 정상 동작 확인 (F-02)

### 8.3 수동 테스트

#### 테스트 시나리오
1. **시나리오 1: 검색어 입력**
   - URLBar에 "cat videos" 입력 → Enter
   - 예상 결과: Google 검색 결과 페이지 로드
   - 확인 항목: URL에 "google.com/search?q=cat%20videos" 포함

2. **시나리오 2: URL과 검색어 혼합**
   - "google.com" 입력 → Google 메인 페이지 로드 (URL로 인식)
   - "google" 입력 → Google 검색 결과 페이지 로드 (검색어로 인식)
   - "youtube" 입력 → Google 검색 결과 페이지 로드 (검색어로 인식)

3. **시나리오 3: 한글 검색**
   - "고양이 동영상" 입력 → Enter
   - 예상 결과: Google 검색 결과 페이지 로드
   - 확인 항목: URL에 한글 인코딩 확인 (%xx%xx%xx)

4. **시나리오 4: 특수문자 검색**
   - "c++" 입력 → Enter
   - 예상 결과: Google 검색 결과 페이지 로드
   - 확인 항목: URL에 "c%2B%2B" 확인

5. **시나리오 5: 검색 엔진 변경 (F-11 연동 시)**
   - 설정에서 기본 검색 엔진을 Naver로 변경
   - "고양이" 입력 → Enter
   - 예상 결과: Naver 검색 결과 페이지 로드
   - 확인 항목: URL에 "search.naver.com" 포함

### 8.4 성능 테스트
- **URL 생성 속도**: 검색 URL 생성 시간 측정 (10ms 이내 목표)
- **메모리 사용량**: SearchEngine 클래스 메모리 사용량 측정 (1KB 미만 목표)
- **프로파일링**: 필요 시 Qt Creator의 프로파일러 사용

### 8.5 테스트 커버리지 목표
- SearchEngine 클래스: 100% (모든 메서드 단위 테스트)
- URLValidator::isSearchQuery: 100%
- URLBar::validateAndCompleteUrl: 80% 이상 (검색어 처리 경로 포함)

---

## 9. 완료 기준

### 9.1 기능 완료 기준
- [ ] SearchEngine 클래스 구현 완료 (정적 메서드, QMap 정의)
- [ ] URLValidator::isSearchQuery() 메서드 추가 완료
- [ ] URLBar::validateAndCompleteUrl() 검색어 처리 로직 통합 완료
- [ ] 4개 검색 엔진 지원 (Google, Naver, Bing, DuckDuckGo)
- [ ] URL 인코딩 처리 정상 동작 (한글, 특수문자, 공백)
- [ ] 검색 URL 생성 및 WebView 로드 정상 동작

### 9.2 테스트 완료 기준
- [ ] SearchEngine 단위 테스트 모두 통과
- [ ] URLValidator 단위 테스트 모두 통과
- [ ] URLBar 통합 테스트 모두 통과
- [ ] 수동 테스트 시나리오 5개 모두 성공
- [ ] 회귀 테스트 통과 (F-03 기능 영향 없음)

### 9.3 코드 품질 기준
- [ ] 코딩 컨벤션 준수 (CLAUDE.md)
- [ ] 주석 및 문서화 완료 (Doxygen 스타일)
- [ ] 컴파일 에러 없음, 경고 없음
- [ ] 코드 리뷰 통과 (code-reviewer 승인)
- [ ] PR 생성 및 main 브랜치 병합 완료

### 9.4 문서화 완료 기준
- [ ] docs/dev-log.md 업데이트
- [ ] CHANGELOG.md 업데이트
- [ ] README.md 업데이트 (검색 기능 설명 추가)

---

## 10. 향후 확장 계획 (M3 이후)

### 10.1 F-11 설정 화면 연동
- SettingsService::setSearchEngine(), getSearchEngine() 구현
- 설정 UI에 "기본 검색 엔진" 항목 추가
- BrowserWindow에서 검색 엔진 변경 시그널 연결
- LS2 API로 설정 저장 및 로드

### 10.2 F-08 히스토리 관리 연동
- 검색 결과 페이지도 히스토리에 저장
- HistoryType::Search 타입 추가
- 히스토리 제목: "Google: cat videos" 형식
- 자동완성에서 검색 히스토리 제안

### 10.3 검색 엔진 추가
- Yahoo, Yandex, Baidu 등 추가 검색 엔진 지원
- JSON 외부화로 컴파일 없이 검색 엔진 추가 가능
- 검색 엔진별 추가 파라미터 지원 (언어, 지역 등)

### 10.4 고급 기능
- 검색 히스토리 타입 구분 (일반 페이지 vs 검색 결과)
- 검색 엔진별 아이콘 표시 (설정 UI)
- 검색 결과 페이지 렌더링 실패 감지 및 에러 메시지

---

## 11. 개발 일정 예상

| Phase | 예상 소요 시간 | 누적 시간 |
|-------|---------------|----------|
| Phase 1: SearchEngine 구현 | 2시간 | 2시간 |
| Phase 2: URLValidator 확장 | 30분 | 2.5시간 |
| Phase 3: URLBar 수정 | 1.5시간 | 4시간 |
| Phase 4: 테스트 | 2시간 | 6시간 |
| Phase 5: 리뷰 및 문서화 | 1시간 | 7시간 |
| **총 예상 소요 시간** | **7시간** | — |

**개발 완료 목표일**: 1일 (7시간 연속 작업 시)

---

## 12. 참고 사항

### 12.1 관련 링크
- Qt QUrl 클래스: https://doc.qt.io/qt-5/qurl.html
- Qt QUrl::toPercentEncoding: https://doc.qt.io/qt-5/qurl.html#toPercentEncoding
- Chrome Omnibox 동작 참고: https://developer.chrome.com/docs/extensions/reference/omnibox/

### 12.2 코딩 컨벤션 준수 사항
- 주석 언어: 한국어
- 변수명 언어: 영어 (camelCase)
- 클래스명: PascalCase (SearchEngine, URLValidator)
- 들여쓰기: 스페이스 4칸
- 헤더 가드: `#pragma once` 사용
- 스마트 포인터 우선 사용 (이 기능에서는 해당 없음)
- Qt 컨테이너 우선 사용 (QMap, QString, QUrl)

### 12.3 Git 브랜치 전략
- 작업 브랜치: `feature/f09-search-engine`
- 커밋 메시지 형식: Conventional Commits
  - `feat(F-09): SearchEngine 클래스 구현`
  - `feat(F-09): URLBar 검색어 처리 통합`
  - `test(F-09): SearchEngine 단위 테스트 추가`
  - `docs(F-09): 검색 기능 문서 업데이트`
- PR 제목: `feat(F-09): 검색 엔진 통합 기능 구현`

---

## 변경 이력

| 날짜 | 버전 | 변경 내용 | 작성자 |
|------|------|-----------|--------|
| 2026-02-14 | 1.0 | F-09 검색 엔진 통합 구현 계획서 최초 작성 | product-manager |
