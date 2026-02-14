# 검색 엔진 통합 — 요구사항 분석서

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
- "유튜브 고양이 동영상", "오늘 날씨" 등의 검색어로 빠르게 정보 접근 필요
- URL과 검색어를 자동으로 구분하여 사용자 경험 향상
- F-03 URL 입력 UI의 확장 기능으로, URL 검증 실패 시 검색어로 처리

---

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: URL vs 검색어 자동 판단
- **설명**: 사용자 입력이 URL인지 검색어인지 자동으로 판단합니다.
- **유저 스토리**: As a 사용자, I want URL 형식이 아닌 단어를 입력할 때 자동으로 검색 결과가 표시되기를 원합니다, so that URL 형식을 정확히 몰라도 원하는 정보를 찾을 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - **URL로 판단하는 경우**:
    - 프로토콜 포함: `http://`, `https://`
    - 도메인 형식: `example.com`, `www.example.com` (점 포함, TLD 필수)
    - localhost/IP: `localhost`, `192.168.1.1`
  - **검색어로 판단하는 경우**:
    - 점(.) 없는 단일 단어: `youtube`, `weather`, `apple`
    - 복수 단어 (공백 포함): `고양이 동영상`, `best restaurants`
    - 특수문자 없는 한글/영문 조합: `오늘 날씨`, `날씨 서울`
  - **판단 로직**:
    1. `validateAndFormatUrl(input)` 실행 (기존 F-03 로직)
    2. 결과가 null이면 검색어로 간주
    3. `buildSearchUrl(input, defaultSearchEngine)` 호출
    4. 생성된 검색 URL을 WebView로 전달

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
  - **URL 인코딩**: 검색어는 `encodeURIComponent()`로 URL 안전 문자열로 변환
  - **기본 검색 엔진**: Google (설정에서 변경 가능, F-11 연동)
  - **검색 엔진 설정 저장**: localStorage에 `defaultSearchEngine` 키로 저장

### FR-3: 검색어 입력 및 결과 표시
- **설명**: URLBar에서 검색어 입력 후 확인 버튼 클릭 시 검색 결과 페이지를 WebView에 로드합니다.
- **유저 스토리**: As a 사용자, I want 검색어를 입력하고 확인 버튼을 누르면 즉시 검색 결과가 표시되기를 원합니다, so that 별도의 검색 사이트 접속 없이 빠르게 정보를 얻을 수 있습니다.
- **우선순위**: Must
- **상세 요구사항**:
  - 사용자가 URLBar에서 검색어 입력 (예: "고양이 동영상")
  - 확인 버튼 클릭 시 `validateAndFormatUrl()` 실행 → null 반환 (URL 아님)
  - `buildSearchUrl("고양이 동영상", "naver")` 실행
  - 생성된 URL: `https://search.naver.com/search.naver?query=%EA%B3%A0%EC%96%91%EC%9D%B4%20%EB%8F%99%EC%98%81%EC%83%81`
  - WebView에서 검색 결과 페이지 로드
  - URLBar에 검색 URL이 표시됨 (검색어가 아닌 전체 URL)

### FR-4: 검색 엔진 설정 UI (F-11 연동 준비)
- **설명**: 설정 화면에서 기본 검색 엔진을 선택할 수 있도록 인터페이스를 제공합니다.
- **유저 스토리**: As a 사용자, I want 설정에서 기본 검색 엔진을 변경할 수 있기를 원합니다, so that 내가 선호하는 검색 엔진을 기본으로 사용할 수 있습니다.
- **우선순위**: Must (F-11 완료 후 통합)
- **상세 요구사항**:
  - F-11 설정 화면에서 "기본 검색 엔진" 항목 추가
  - 선택지: Google, Naver, Bing, DuckDuckGo (라디오 버튼 또는 드롭다운)
  - 설정 변경 시 localStorage에 저장
  - 앱 시작 시 localStorage에서 기본 검색 엔진 로드
  - 설정 변경 시 즉시 반영 (재시작 불필요)

### FR-5: 검색 히스토리 저장 (F-08 연동)
- **설명**: 검색 결과 페이지도 히스토리에 저장하여 이후 자동완성 제안에 포함합니다.
- **유저 스토리**: As a 사용자, I want 이전에 검색했던 키워드를 히스토리에서 다시 찾을 수 있기를 원합니다, so that 같은 검색을 반복할 때 시간을 절약할 수 있습니다.
- **우선순위**: Should (F-08 완료 후 통합)
- **상세 요구사항**:
  - 검색 URL 로드 시 히스토리에 저장 (F-08 historyService 사용)
  - 히스토리 제목: 검색 엔진 이름 + 검색어 (예: "Naver: 고양이 동영상")
  - 자동완성에서 검색 히스토리 제안 (URLBar에서 검색어 입력 시)

---

## 3. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **URL 생성 속도**: 검색 URL 생성은 10ms 이내 (단순 문자열 조합)
- **인코딩 처리**: `encodeURIComponent()` 실행 시 성능 영향 무시 가능 (빠른 내장 함수)
- **설정 로드 시간**: localStorage에서 검색 엔진 설정 로드는 5ms 이내

### 확장성
- **검색 엔진 추가 용이성**: 새로운 검색 엔진 추가 시 설정 객체만 수정
  - `SEARCH_ENGINES` 객체에 엔진 이름과 URL 템플릿 추가
  - 설정 UI에서 선택지 자동 생성 가능하도록 설계
- **다국어 검색 지원**: 한글, 영문, 일본어 등 다국어 검색어 URL 인코딩 지원

### 호환성
- **주요 검색 엔진 동작 보장**: Google, Naver, Bing은 webOS WebView에서 정상 렌더링 확인
- **검색 결과 페이지 렌더링**: 검색 엔진의 JavaScript 기반 UI가 webOS에서 동작하는지 테스트 필요

### 보안
- **XSS 방지**: 검색어는 `encodeURIComponent()`로 인코딩하여 URL 인젝션 방지
- **안전한 검색 엔진**: 지원하는 검색 엔진은 HTTPS 전용 (HTTP 검색 엔진 미지원)

---

## 4. 데이터 구조

### 검색 엔진 설정 (localStorage)
```javascript
// Key: 'defaultSearchEngine'
// Value: 'google' | 'naver' | 'bing' | 'duckduckgo'
// 기본값: 'google'

// 예시
localStorage.setItem('defaultSearchEngine', 'naver')
```

### 검색 엔진 정의 (코드 내 상수)
```javascript
// src/services/searchService.js

export const SEARCH_ENGINES = {
	google: {
		name: 'Google',
		url: 'https://www.google.com/search?q={query}',
		icon: '/assets/icons/google.png'  // 선택적
	},
	naver: {
		name: 'Naver',
		url: 'https://search.naver.com/search.naver?query={query}',
		icon: '/assets/icons/naver.png'
	},
	bing: {
		name: 'Bing',
		url: 'https://www.bing.com/search?q={query}',
		icon: '/assets/icons/bing.png'
	},
	duckduckgo: {
		name: 'DuckDuckGo',
		url: 'https://duckduckgo.com/?q={query}',
		icon: '/assets/icons/duckduckgo.png'
	}
}
```

### 검색 히스토리 항목 (F-08 연동 시)
```javascript
// historyService에 저장되는 검색 히스토리 형식
{
	url: 'https://www.google.com/search?q=%EA%B3%A0%EC%96%91%EC%9D%B4',
	title: 'Google: 고양이',  // 검색 엔진 이름 + 검색어
	visitedAt: 1707825600000,  // 타임스탬프
	type: 'search'  // 히스토리 타입 (일반 페이지와 구분)
}
```

---

## 5. 제약조건

### 기술 제약사항
- **URL 최대 길이**: 브라우저 URL 길이 제한 (일반적으로 2048자) 고려
  - 검색어가 매우 긴 경우 URL 인코딩 후 길이 초과 가능
  - 초과 시 검색어 자르기 또는 에러 메시지 표시 (선택)
- **특수문자 처리**: `encodeURIComponent()`가 모든 특수문자를 안전하게 인코딩하는지 테스트 필요
  - 한글, 이모지, 특수기호 등

### webOS 플랫폼 제약사항
- **검색 엔진 렌더링**: 일부 검색 엔진은 webOS WebView에서 JavaScript 실행 제한으로 정상 동작하지 않을 수 있음
  - Google, Naver는 기본 렌더링 테스트 필수
  - Bing, DuckDuckGo는 추가 테스트 권장

### 의존성
- **F-03 (URL 입력 UI)**: URLBar 컴포넌트와 `validateAndFormatUrl` 유틸리티 필요
- **F-02 (웹뷰 통합)**: 생성된 검색 URL을 WebView에 로드
- **F-11 (설정 화면)**: 기본 검색 엔진 선택 UI (F-09 완료 후 통합)
- **F-08 (히스토리 관리)**: (선택) 검색 히스토리 저장 및 자동완성 연동

### 범위 외 (Out of Scope)
- **고급 검색 옵션**: 이미지 검색, 뉴스 검색, 기간 필터 등 미지원
- **검색 자동완성**: 검색 엔진의 자동완성 제안 API 연동 미지원 (URLBar 자체 자동완성만 사용)
- **검색 엔진별 필터링**: "site:naver.com" 같은 고급 쿼리 미지원 (사용자가 직접 입력 가능)
- **음성 검색**: 음성 입력 기능 미지원 (향후 확장 가능)

---

## 6. 용어 정의

| 용어 | 정의 |
|------|------|
| **검색 엔진** | 사용자 입력 검색어를 받아 웹 검색 결과를 제공하는 서비스 (Google, Naver 등) |
| **검색어** | URL 형식이 아닌 일반 텍스트 입력 (예: "고양이 동영상") |
| **검색 URL** | 검색 엔진의 쿼리 파라미터에 검색어를 포함한 전체 URL |
| **URL 인코딩** | 특수문자, 공백, 다국어를 URL 안전 문자열로 변환하는 과정 (`encodeURIComponent`) |
| **기본 검색 엔진** | 사용자가 설정한 기본 검색 엔진 (설정하지 않으면 Google) |
| **검색 히스토리** | 검색어로 접근한 검색 결과 페이지의 방문 기록 |

---

## 7. 완료 기준 (Acceptance Criteria)

### AC-1: URL vs 검색어 자동 판단
- [ ] `google.com` 입력 시 → `http://google.com`로 변환 (URL로 인식)
- [ ] `youtube` 입력 시 → 검색 URL 생성 (검색어로 인식)
- [ ] `고양이 동영상` 입력 시 → 검색 URL 생성 (검색어로 인식)
- [ ] `https://example.com` 입력 시 → URL로 그대로 사용

### AC-2: 검색 URL 생성
- [ ] Google 선택 시 `https://www.google.com/search?q=검색어` 형식 생성
- [ ] Naver 선택 시 `https://search.naver.com/search.naver?query=검색어` 형식 생성
- [ ] 검색어에 공백 포함 시 `%20`으로 인코딩됨 (예: `고양이 동영상` → `%EA%B3%A0%EC%96%91%EC%9D%B4%20%EB%8F%99%EC%98%81%EC%83%81`)
- [ ] 한글, 영문, 숫자, 특수문자 모두 정상 인코딩됨

### AC-3: 검색 결과 페이지 로드
- [ ] URLBar에서 검색어 입력 → 확인 버튼 클릭 시 WebView에서 검색 결과 페이지 로드
- [ ] Google에서 `youtube` 검색 시 검색 결과 페이지 정상 표시
- [ ] Naver에서 `고양이` 검색 시 검색 결과 페이지 정상 표시
- [ ] URLBar에 최종 검색 URL 표시 (검색어가 아닌 전체 URL)

### AC-4: 검색 엔진 설정 저장
- [ ] localStorage에 `defaultSearchEngine` 키로 설정 저장
- [ ] 앱 재시작 후 이전 설정 유지
- [ ] 설정 변경 시 즉시 다음 검색에 반영

### AC-5: 검색 엔진 선택 UI (F-11 연동 시)
- [ ] 설정 화면에서 "기본 검색 엔진" 항목 표시
- [ ] Google, Naver, Bing, DuckDuckGo 선택 가능
- [ ] 현재 선택된 검색 엔진 하이라이트 표시
- [ ] 검색 엔진 변경 시 즉시 저장 및 반영

### AC-6: 검색 히스토리 저장 (F-08 연동 시)
- [ ] 검색 결과 페이지 로드 시 히스토리에 자동 저장
- [ ] 히스토리 제목: "검색엔진명: 검색어" 형식
- [ ] 히스토리 패널에서 검색 기록 조회 가능
- [ ] 검색 히스토리 클릭 시 검색 결과 페이지 재로드

### AC-7: 에러 처리
- [ ] 검색어가 비어있으면 에러 메시지 표시 (F-10 연동)
- [ ] 검색 URL이 2048자를 초과하면 경고 메시지 표시 (선택)
- [ ] 지원하지 않는 검색 엔진 설정값이면 Google로 폴백

### AC-8: 회귀 테스트
- [ ] 기존 URL 입력 기능 정상 동작 (검색 엔진 통합 후에도 URL 입력 가능)
- [ ] 자동완성 기능과 충돌 없음 (F-07, F-08 연동 시)
- [ ] 리모컨 조작으로 모든 기능 사용 가능

---

## 8. 참고 사항

### 관련 문서
- PRD: `docs/project/prd.md`
- 기능 백로그: `docs/project/features.md`
- F-03 요구사항 분석서: `docs/specs/url-input-ui/requirements.md`
- F-03 기술 설계서: `docs/specs/url-input-ui/design.md`
- CLAUDE.md: 프로젝트 규칙 및 코딩 컨벤션

### 참고 구현
- 기존 `src/utils/urlValidator.js` (F-03에서 구현됨)
  - `validateAndFormatUrl()` 함수에서 null 반환 시 검색어로 처리
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

### 우선순위 조정 가능성
- 검색 히스토리 저장 (FR-5)은 Should 우선순위로, F-08 완료 후 추가 가능
- 검색 엔진 아이콘 표시는 M3 이후 UI 개선으로 연기 가능

---

## 9. 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-13 | 최초 작성 | F-09 요구사항 분석 |
