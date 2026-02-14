# 검색 엔진 통합 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: `docs/specs/search-engine-integration/requirements.md`
- 기술 설계서: `docs/specs/search-engine-integration/design.md`
- PRD: `docs/project/prd.md`
- 기능 백로그: `docs/project/features.md`

## 2. 구현 개요

### 목표
URL 입력 UI (F-03)를 확장하여 URL이 아닌 검색어 입력 시 자동으로 선택된 검색 엔진의 검색 결과 페이지를 로드하는 기능 구현.

### 핵심 전략
- **최소 침습 원칙**: 기존 F-03 URL 입력 흐름을 최대한 유지하고 검증 실패 시점에만 검색 로직 추가
- **Fallback 패턴**: URL 검증 실패 → 검색어로 간주 → 검색 URL 생성
- **설정 영속성**: localStorage로 기본 검색 엔진 저장 (F-11 설정 화면 연동 준비)
- **단계적 확장**: F-09는 검색 로직만, F-11에서 설정 UI 추가, F-08 연동 시 검색 히스토리 저장

### 구현 범위
- ✅ **포함**: 검색 엔진 정의, 검색 URL 생성, URL vs 검색어 자동 판단, 설정 저장/로드
- ⚠️ **F-11 연동 준비**: 설정 UI용 인터페이스 제공 (getAllSearchEngines, setDefaultSearchEngine)
- ⚠️ **F-08 연동 준비**: 검색 히스토리 저장 인터페이스 설계 (구현은 F-08 완료 후)
- ❌ **범위 외**: 검색 제안 API 연동, 음성 검색, 검색 필터링

## 3. 구현 Phase

### Phase 1: 검색 엔진 서비스 구현 (핵심 로직)
**담당**: frontend-dev
**예상 시간**: 2시간
**우선순위**: Critical (다른 Phase의 기반)

- [ ] Task 1.1: searchService.js 생성
  - `SEARCH_ENGINES` 상수 정의 (Google, Naver, Bing, DuckDuckGo)
  - `DEFAULT_SEARCH_ENGINE` 상수 정의
  - logger import
- [ ] Task 1.2: buildSearchUrl() 함수 구현
  - 검색어 유효성 검증 (빈 문자열 체크)
  - 검색 엔진 결정 (파라미터 → 설정 → 기본값)
  - 검색 엔진 정의 조회 및 폴백 처리
  - encodeURIComponent()로 검색어 인코딩
  - URL 템플릿에 쿼리 삽입
  - 로깅
- [ ] Task 1.3: getDefaultSearchEngine() 함수 구현
  - localStorage에서 'defaultSearchEngine' 조회
  - 유효성 검증 (SEARCH_ENGINES에 존재하는 ID인지)
  - 기본값 반환 (google)
  - 에러 처리 (localStorage 접근 실패)
- [ ] Task 1.4: setDefaultSearchEngine() 함수 구현
  - 검색 엔진 ID 유효성 검증
  - localStorage에 저장
  - 성공/실패 반환
  - 에러 처리
- [ ] Task 1.5: 유틸리티 함수 구현 (F-11 연동용)
  - getAllSearchEngines() 구현
  - getSearchEngineName() 구현

**산출물**:
- `src/services/searchService.js` (약 300줄)

**완료 기준**:
- [x] 4개 검색 엔진 정의 완료 (Google, Naver, Bing, DuckDuckGo)
- [x] buildSearchUrl() 단위 테스트 통과 (6개 케이스)
- [x] 설정 저장/로드 단위 테스트 통과 (4개 케이스)
- [x] JSDoc 주석 작성 완료

---

### Phase 2: URL 검증 로직 확장 (검색어 감지)
**담당**: frontend-dev
**예상 시간**: 1.5시간
**의존성**: Phase 1 완료 필요
**우선순위**: Critical

- [ ] Task 2.1: urlValidator.js 수정
  - searchService import 추가
  - validateAndFormatUrl() 함수 확장
  - 기존 3단계 URL 검증 로직 유지 (프로토콜, 도메인, localhost)
  - 4단계 추가: 검색어로 간주 → buildSearchUrl() 호출
  - 로깅 추가
- [ ] Task 2.2: 기존 F-03 회귀 테스트
  - 기존 URL 입력 테스트 케이스 재실행
  - 도메인 형식 우선 처리 확인
  - 프로토콜 포함 URL 정상 처리 확인

**산출물**:
- `src/utils/urlValidator.js` (기존 코드 + 약 30줄)

**완료 기준**:
- [x] 검색어 감지 로직 추가 완료
- [x] URL vs 검색어 자동 판단 단위 테스트 통과 (4개 케이스)
- [x] 기존 F-03 회귀 테스트 통과 (URL 입력 정상 동작)

---

### Phase 3: 통합 및 검증 (URLBar → WebView 플로우)
**담당**: frontend-dev
**예상 시간**: 1시간
**의존성**: Phase 1, 2 완료 필요
**우선순위**: High

- [ ] Task 3.1: BrowserView 통합 테스트
  - 검색어 입력 → 검색 URL 생성 → WebView 로드 플로우 확인
  - URLBar에서 검색어 입력 후 확인 버튼 클릭
  - WebView에 검색 URL 전달 확인
- [ ] Task 3.2: 에러 처리 확인 (F-10 연동 준비)
  - 빈 검색어 입력 시 null 반환 확인
  - URLBar에서 에러 처리 준비 (F-10에서 UI 구현)
- [ ] Task 3.3: 리모컨 조작 테스트
  - 가상 키보드로 검색어 입력 후 확인 버튼 선택
  - 포커스 이동 정상 동작 확인

**산출물**:
- 통합 테스트 결과 (테스트 케이스 3개)

**완료 기준**:
- [x] 검색어 입력 → 검색 결과 로드 플로우 정상 동작
- [x] URL 입력 → 일반 페이지 로드 플로우 정상 동작 (회귀 테스트)
- [x] 빈 입력 시 에러 처리 정상 동작

---

### Phase 4: 실제 디바이스 테스트 (webOS 프로젝터)
**담당**: frontend-dev
**예상 시간**: 2시간
**의존성**: Phase 3 완료 필요
**우선순위**: High

- [ ] Task 4.1: 검색 엔진별 렌더링 테스트
  - Google 검색 결과 페이지 렌더링 확인
  - Naver 검색 결과 페이지 렌더링 확인
  - Bing 검색 결과 페이지 렌더링 확인
  - DuckDuckGo 검색 결과 페이지 렌더링 확인
- [ ] Task 4.2: 다양한 검색어 테스트
  - 영문 검색어: "youtube", "weather"
  - 한글 검색어: "네이버", "날씨 서울"
  - 공백 포함: "고양이 동영상", "best restaurants"
  - 특수문자 포함: "a+b=c", "C++ tutorial"
- [ ] Task 4.3: URL vs 검색어 혼합 시나리오
  - "google.com" → URL 로드
  - "youtube" → 검색 결과
  - "https://naver.com" → URL 로드
  - "192.168.1.1" → URL 로드
- [ ] Task 4.4: 설정 영속성 테스트
  - localStorage에 검색 엔진 저장 확인
  - 앱 재시작 후 설정 유지 확인
  - 앱 데이터 삭제 후 기본값(Google) 사용 확인

**산출물**:
- 테스트 결과 보고서 (8개 시나리오)

**완료 기준**:
- [x] 4개 검색 엔진 모두 정상 렌더링 확인
- [x] 다국어 검색어 (한글, 영문) 정상 동작
- [x] URL 우선 처리 로직 정상 동작
- [x] 설정 영속성 확인

---

### Phase 5: 테스트 작성 및 코드 리뷰
**담당**: test-runner, code-reviewer
**예상 시간**: 2시간
**의존성**: Phase 4 완료 필요
**우선순위**: High

- [ ] Task 5.1: 단위 테스트 작성 (test-runner)
  - `searchService.test.js` 작성 (14개 테스트 케이스)
    - buildSearchUrl() 테스트 (6개)
    - getDefaultSearchEngine() 테스트 (2개)
    - setDefaultSearchEngine() 테스트 (2개)
    - getAllSearchEngines() 테스트 (1개)
    - getSearchEngineName() 테스트 (1개)
  - `urlValidator.test.js` 확장 (4개 테스트 케이스)
    - 검색어 인식 테스트 (2개)
    - URL 우선 처리 테스트 (2개)
  - Jest 실행 및 커버리지 확인 (목표: 95% 이상)
- [ ] Task 5.2: 통합 테스트 작성 (test-runner)
  - `BrowserView.test.js` 확장 (1개 테스트 케이스)
    - 검색어 입력 → 검색 URL 생성 → WebView 로드 플로우
- [ ] Task 5.3: 코드 리뷰 (code-reviewer)
  - searchService.js 코드 리뷰
    - 에러 처리 적절성 확인
    - 보안 (XSS 방지: encodeURIComponent 사용 확인)
    - JSDoc 주석 완성도 확인
  - urlValidator.js 코드 리뷰
    - 기존 로직 영향 최소화 확인
    - 검색어 감지 로직 정확성 확인
  - 코딩 컨벤션 준수 확인 (CLAUDE.md)

**산출물**:
- `src/__tests__/services/searchService.test.js` (약 200줄)
- `src/__tests__/utils/urlValidator.test.js` (기존 + 약 50줄)
- 코드 리뷰 보고서

**완료 기준**:
- [x] 단위 테스트 18개 모두 통과
- [x] 통합 테스트 1개 통과
- [x] 테스트 커버리지 95% 이상
- [x] 코드 리뷰 승인 (Critical/High 이슈 0개)

---

### Phase 6: 문서화 및 커밋
**담당**: frontend-dev, doc-writer
**예상 시간**: 1시간
**의존성**: Phase 5 완료 필요
**우선순위**: Medium

- [ ] Task 6.1: 컴포넌트 문서 작성 (frontend-dev)
  - searchService.md 작성
    - API 레퍼런스 (5개 함수)
    - 사용 예시
    - 검색 엔진 추가 가이드
- [ ] Task 6.2: 진행 로그 업데이트 (doc-writer)
  - docs/dev-log.md에 F-09 완료 기록
  - 주요 구현 내용 요약
  - 테스트 결과 요약
- [ ] Task 6.3: CHANGELOG 업데이트 (doc-writer)
  - CHANGELOG.md에 F-09 추가
  - 새로운 기능 설명
  - API 변경 사항 (urlValidator 확장)
- [ ] Task 6.4: features.md 상태 업데이트 (doc-writer)
  - F-09 상태를 '완료'로 변경
  - 완료 일자 기록
- [ ] Task 6.5: Git 커밋 (frontend-dev)
  - 구현 코드 커밋: `feat(F-09): 검색 엔진 통합 기능 구현`
  - 테스트 커밋: `test(F-09): searchService, urlValidator 단위 테스트 추가`
  - 문서 커밋: `docs(F-09): searchService 컴포넌트 문서 작성`
  - 운영 문서 커밋: `chore: F-09 상태를 '완료'로 업데이트`

**산출물**:
- `docs/components/searchService.md`
- `docs/dev-log.md` (업데이트)
- `CHANGELOG.md` (업데이트)
- `docs/project/features.md` (업데이트)
- Git 커밋 4개

**완료 기준**:
- [x] searchService.md 작성 완료 (API 레퍼런스 포함)
- [x] 진행 로그 및 CHANGELOG 업데이트 완료
- [x] Git 커밋 완료 (4개 커밋)
- [x] ESLint 검증 통과

## 4. 태스크 의존성 그래프

```
Phase 1: 검색 엔진 서비스 구현
  │
  ├─▶ Task 1.1 → Task 1.2 → Task 1.3 → Task 1.4 → Task 1.5
  │
  └─▶ Phase 2: URL 검증 로직 확장
        │
        ├─▶ Task 2.1 → Task 2.2
        │
        └─▶ Phase 3: 통합 및 검증
              │
              ├─▶ Task 3.1 → Task 3.2 → Task 3.3
              │
              └─▶ Phase 4: 실제 디바이스 테스트
                    │
                    ├─▶ Task 4.1 → Task 4.2 → Task 4.3 → Task 4.4
                    │
                    └─▶ Phase 5: 테스트 작성 및 코드 리뷰
                          │
                          ├─▶ Task 5.1 (test-runner)
                          ├─▶ Task 5.2 (test-runner)
                          └─▶ Task 5.3 (code-reviewer)
                                │
                                └─▶ Phase 6: 문서화 및 커밋
                                      │
                                      └─▶ Task 6.1 → Task 6.2 → Task 6.3 → Task 6.4 → Task 6.5
```

### 병렬 실행 가능 태스크
- **Phase 1 내**: Task 1.1 → 1.2 → 1.3은 순차 (의존성 있음), 1.4/1.5는 1.3 완료 후 병렬 가능
- **Phase 5 내**: Task 5.1, 5.2는 병렬 가능 (서로 독립), Task 5.3은 5.1/5.2 완료 후 실행
- **Phase 6 내**: Task 6.1/6.2/6.3/6.4는 병렬 가능, Task 6.5는 모두 완료 후 실행

## 5. 병렬 실행 판단

### Agent Team 사용 여부: **No (순차 실행 권장)**

### 판단 근거
1. **Phase 간 강한 의존성**:
   - Phase 1 (검색 엔진 서비스) → Phase 2 (URL 검증 확장) → Phase 3 (통합)
   - 각 Phase가 이전 Phase의 산출물을 의존하므로 순차 실행 필수
2. **구현 복잡도 낮음**:
   - 주요 코드는 2개 파일 (searchService.js 신규, urlValidator.js 확장)
   - 총 예상 소요 시간 9.5시간 (1~2일 내 완료 가능)
3. **테스트 단계 병렬화 불필요**:
   - Phase 5의 테스트 작성은 병렬 가능하지만, 테스트 작성 자체가 빠르고 단순함 (2시간)
   - worktree 생성 및 병합 오버헤드가 병렬화 이점보다 클 수 있음
4. **문서화 단계 병렬화 효과 미미**:
   - Phase 6의 문서 작성은 병렬 가능하지만, 전체 1시간 소요로 병렬화 불필요

### 권장 실행 방식
- **순차 실행**: `/fullstack-feature F-09 검색 엔진 통합`
- **에이전트 플로우**:
  1. `product-manager`: 이 plan.md 작성 완료 ✅
  2. `frontend-dev`: Phase 1~4 순차 실행 (코드 구현 + 디바이스 테스트)
  3. `test-runner`: Phase 5.1~5.2 실행 (단위/통합 테스트)
  4. `code-reviewer`: Phase 5.3 실행 (코드 리뷰)
  5. `frontend-dev`: Phase 6.1 실행 (컴포넌트 문서)
  6. `doc-writer`: Phase 6.2~6.5 실행 (진행 로그 + 커밋)

## 6. 예상 소요 시간

| Phase | 담당 | 예상 시간 | 비고 |
|-------|------|----------|------|
| Phase 1: 검색 엔진 서비스 구현 | frontend-dev | 2시간 | 5개 태스크 (핵심 로직) |
| Phase 2: URL 검증 로직 확장 | frontend-dev | 1.5시간 | 기존 코드 확장 + 회귀 테스트 |
| Phase 3: 통합 및 검증 | frontend-dev | 1시간 | 통합 테스트 3개 |
| Phase 4: 실제 디바이스 테스트 | frontend-dev | 2시간 | 검색 엔진 4개 × 시나리오 8개 |
| Phase 5: 테스트 작성 및 코드 리뷰 | test-runner, code-reviewer | 2시간 | 단위 테스트 18개 + 코드 리뷰 |
| Phase 6: 문서화 및 커밋 | frontend-dev, doc-writer | 1시간 | 문서 5개 + 커밋 4개 |
| **총합** | | **9.5시간** | 약 1~2일 |

### 시간 분배
- **코드 구현**: 6.5시간 (68%)
- **테스트 및 검증**: 2시간 (21%)
- **문서화**: 1시간 (11%)

### 마일스톤 목표 (M2)
- F-09는 M2 (기능 확장 및 고도화)에 포함됨
- **목표 완료 일자**: 2026-02-14 (금일 시작 시 내일 완료 가능)

## 7. 리스크 및 대응 방안

### 리스크 1: 검색 엔진 렌더링 실패
- **설명**: 일부 검색 엔진은 webOS WebView에서 JavaScript 실행 제한으로 정상 렌더링 실패 가능
- **영향도**: Medium (특정 검색 엔진만 영향)
- **발생 확률**: 30%
- **대응 방안**:
  - Phase 4에서 4개 검색 엔진 모두 실제 디바이스에서 렌더링 테스트
  - 렌더링 실패 검색 엔진은 SEARCH_ENGINES에서 제거 또는 경고 메시지 표시
  - 최소 2개 검색 엔진(Google, Naver)은 정상 동작 보장
- **대체 계획**: 렌더링 실패 시 해당 검색 엔진 비활성화 + F-11에서 선택지 제외

### 리스크 2: URL vs 검색어 구분 모호성
- **설명**: "apple"은 검색어인가 apple.com 도메인인가? 사용자 의도와 다른 결과 가능
- **영향도**: Low (사용자가 검색 결과에서 원하는 사이트 클릭 가능)
- **발생 확률**: 50%
- **대응 방안**:
  - 현재 로직: 점(.) 포함 시 도메인 우선, 점 없으면 검색어 처리
  - 사용자 피드백 수집 후 필요 시 로직 조정 (예: "apple.com" 추천)
  - F-07 자동완성 연동 시 도메인 제안 우선 표시로 보완 가능
- **대체 계획**: F-11 설정에서 "URL 우선 모드" 옵션 추가 (향후 확장)

### 리스크 3: localStorage 초기화
- **설명**: 앱 데이터 삭제 시 검색 엔진 설정 초기화됨
- **영향도**: Low (Google 기본값으로 폴백)
- **발생 확률**: 10%
- **대응 방안**:
  - getDefaultSearchEngine()에서 localStorage 접근 실패 시 Google 반환
  - 사용자에게 설정 초기화 안내 (F-10 에러 처리 연동 시)
- **대체 계획**: 향후 webOS LS2 API로 전환하여 영속성 강화 (M3 이후)

### 리스크 4: 검색어 길이 제한
- **설명**: 매우 긴 검색어 (1000자 이상) 입력 시 URL 길이 제한 (2048자) 초과 가능
- **영향도**: Low (실무에서는 드문 케이스)
- **발생 확률**: 5%
- **대응 방안**:
  - 현재는 제한 없이 처리 (encodeURIComponent 후 URL 생성)
  - Phase 4 테스트에서 긴 검색어 시나리오 테스트
  - 필요 시 buildSearchUrl()에서 검색어 길이 제한 (예: 500자) 추가
- **대체 계획**: 검색어 자르기 + 경고 메시지 표시 (F-10 연동)

### 리스크 5: F-03 회귀 버그
- **설명**: urlValidator.js 확장 시 기존 URL 입력 로직에 버그 발생 가능
- **영향도**: High (F-03 기능 중단)
- **발생 확률**: 10%
- **대응 방안**:
  - Phase 2에서 기존 F-03 회귀 테스트 필수
  - urlValidator.js 수정 시 기존 로직 최대한 유지 (검색어 처리는 맨 마지막 단계에 추가)
  - 단위 테스트로 기존 URL 입력 케이스 모두 검증
- **대체 계획**: 회귀 버그 발생 시 urlValidator.js 롤백 + 검색 로직 재설계

## 8. 검증 계획

### 8.1 단위 테스트 (Phase 5.1)

#### searchService.test.js (14개 테스트 케이스)
```javascript
describe('searchService', () => {
  // buildSearchUrl() 테스트 (6개)
  test('buildSearchUrl - Google 검색')
  test('buildSearchUrl - Naver 검색')
  test('buildSearchUrl - 공백 포함 검색어')
  test('buildSearchUrl - 빈 검색어')
  test('buildSearchUrl - 알 수 없는 검색 엔진 (폴백)')
  test('buildSearchUrl - 특수문자 인코딩')

  // getDefaultSearchEngine() 테스트 (2개)
  test('getDefaultSearchEngine - 저장된 값')
  test('getDefaultSearchEngine - 기본값')

  // setDefaultSearchEngine() 테스트 (2개)
  test('setDefaultSearchEngine - 성공')
  test('setDefaultSearchEngine - 실패 (유효하지 않은 엔진)')

  // 유틸리티 함수 테스트 (2개)
  test('getAllSearchEngines - 4개 엔진 반환')
  test('getSearchEngineName - 이름 조회')
})
```

#### urlValidator.test.js (4개 테스트 케이스 추가)
```javascript
describe('urlValidator - 검색어 처리', () => {
  test('검색어 인식 - 단일 단어 ("youtube")')
  test('검색어 인식 - 복수 단어 ("고양이 동영상")')
  test('URL 우선 - 도메인 형식 ("apple.com")')
  test('URL 우선 - 프로토콜 포함 ("https://naver.com")')
})
```

### 8.2 통합 테스트 (Phase 5.2)
```javascript
describe('BrowserView - 검색 엔진 통합', () => {
  test('검색어 입력 → 검색 URL 생성 → WebView 로드')
})
```

### 8.3 실제 디바이스 테스트 (Phase 4)

#### 시나리오 1: 검색 엔진별 렌더링 (Task 4.1)
| 검색 엔진 | 테스트 URL | 예상 결과 |
|---------|-----------|----------|
| Google | `https://www.google.com/search?q=test` | 검색 결과 정상 표시 |
| Naver | `https://search.naver.com/search.naver?query=test` | 검색 결과 정상 표시 |
| Bing | `https://www.bing.com/search?q=test` | 검색 결과 정상 표시 |
| DuckDuckGo | `https://duckduckgo.com/?q=test` | 검색 결과 정상 표시 |

#### 시나리오 2: 다양한 검색어 (Task 4.2)
| 검색어 | 인코딩 후 | 예상 결과 |
|-------|----------|----------|
| "youtube" | `youtube` | Google 검색 결과 로드 |
| "네이버" | `%EB%84%A4%EC%9D%B4%EB%B2%84` | Google 검색 결과 로드 |
| "고양이 동영상" | `%EA%B3%A0%EC%96%91%EC%9D%B4%20%EB%8F%99%EC%98%81%EC%83%81` | Google 검색 결과 로드 |
| "a+b=c" | `a%2Bb%3Dc` | Google 검색 결과 로드 |

#### 시나리오 3: URL vs 검색어 혼합 (Task 4.3)
| 입력값 | 판단 | 최종 동작 |
|-------|------|----------|
| "google.com" | URL (도메인) | http://google.com 로드 |
| "youtube" | 검색어 | Google 검색: youtube |
| "https://naver.com" | URL (프로토콜 포함) | https://naver.com 로드 |
| "192.168.1.1" | URL (IP 주소) | http://192.168.1.1 로드 |
| "날씨 서울" | 검색어 | Google 검색: 날씨 서울 |
| "" | null | 에러 메시지 표시 |

#### 시나리오 4: 설정 영속성 (Task 4.4)
1. Naver를 기본 검색 엔진으로 설정 (localStorage 저장)
2. "youtube" 검색 → Naver 검색 결과 로드 확인
3. 앱 재시작 → "apple" 검색 → Naver 검색 결과 로드 확인
4. 앱 데이터 삭제 → "test" 검색 → Google 검색 결과 로드 확인 (기본값)

### 8.4 완료 기준 (Acceptance Criteria)
**Phase별 완료 기준은 각 Phase의 "완료 기준" 섹션 참조**

#### 전체 기능 완료 기준 (요구사항 분석서 AC 기반)
- [x] **AC-1**: URL vs 검색어 자동 판단 (4개 케이스 모두 통과)
- [x] **AC-2**: 검색 URL 생성 (4개 케이스 모두 통과)
- [x] **AC-3**: 검색 결과 페이지 로드 (3개 케이스 모두 통과)
- [x] **AC-4**: 검색 엔진 설정 저장 (3개 케이스 모두 통과)
- [x] **AC-7**: 에러 처리 (빈 검색어 → null 반환 확인)
- [x] **AC-8**: 회귀 테스트 (기존 F-03 정상 동작 확인)

## 9. F-11 설정 화면 연동 준비

### 제공 인터페이스
F-09에서 구현하는 searchService는 F-11 설정 화면에서 사용할 수 있도록 다음 인터페이스를 제공합니다:

```javascript
// F-11에서 사용할 API
import {
  getAllSearchEngines,      // 검색 엔진 목록 조회 (라디오 버튼 생성)
  getDefaultSearchEngine,   // 현재 선택된 검색 엔진 조회 (라디오 버튼 체크)
  setDefaultSearchEngine    // 검색 엔진 변경 (라디오 버튼 선택 시)
} from '../services/searchService'
```

### F-11 구현 시 예상 코드
```javascript
// src/views/SettingsPanel.js (F-11에서 구현)

const SettingsPanel = () => {
  const [engines] = useState(getAllSearchEngines())  // ['google', 'naver', ...]
  const [currentEngine, setCurrentEngine] = useState(getDefaultSearchEngine())

  const handleEngineChange = (engineId) => {
    const success = setDefaultSearchEngine(engineId)
    if (success) {
      setCurrentEngine(engineId)
      // 성공 메시지 표시 (F-10 연동)
    }
  }

  return (
    <Panel>
      <h3>기본 검색 엔진</h3>
      {engines.map(engine => (
        <RadioItem
          key={engine.id}
          selected={currentEngine === engine.id}
          onToggle={() => handleEngineChange(engine.id)}
        >
          {engine.name}
        </RadioItem>
      ))}
    </Panel>
  )
}
```

### F-11 연동 시점
- F-09 완료 후 F-11 구현 시 설정 UI 추가
- searchService API는 수정 불필요 (인터페이스 이미 제공됨)
- SettingsPanel에서 import만 추가하면 즉시 사용 가능

## 10. F-08 히스토리 연동 준비

### 검색 히스토리 저장 인터페이스
F-08 완료 후 검색 히스토리를 저장하려면 다음과 같이 연동합니다:

#### historyService.js 확장 (F-08 구현 시 추가)
```javascript
// src/services/historyService.js (F-08 구현 시 확장)

import { getSearchEngineName } from './searchService'

/**
 * 검색 URL인지 감지 및 제목 생성
 */
const generateHistoryTitle = (url, pageTitle) => {
  // 검색 URL 패턴 감지
  const searchPatterns = {
    google: /google\.com\/search\?.*q=([^&]+)/,
    naver: /search\.naver\.com\/search\.naver\?.*query=([^&]+)/,
    bing: /bing\.com\/search\?.*q=([^&]+)/,
    duckduckgo: /duckduckgo\.com\/\?.*q=([^&]+)/
  }

  for (const [engineId, pattern] of Object.entries(searchPatterns)) {
    const match = url.match(pattern)
    if (match) {
      const query = decodeURIComponent(match[1])
      const engineName = getSearchEngineName(engineId)
      return `${engineName}: ${query}`  // 예: "Google: 고양이"
    }
  }

  // 일반 페이지면 페이지 제목 사용
  return pageTitle || url
}
```

#### 검색 히스토리 항목 형식
```javascript
{
  url: 'https://www.google.com/search?q=%EB%84%A4%EC%9D%B4%EB%B2%84',
  title: 'Google: 네이버',  // 검색 엔진 이름 + 검색어
  visitedAt: 1707825600000,
  type: 'search'  // 히스토리 타입 (일반 페이지와 구분)
}
```

### F-08 연동 시점
- F-08 완료 후 historyService에서 generateHistoryTitle() 추가
- WebView에서 검색 URL 로드 시 자동으로 히스토리에 저장
- URLBar 자동완성에서 검색 히스토리 제안 가능

## 11. 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-13 | 최초 작성 | F-09 요구사항 및 설계서 기반 구현 계획 수립 |
