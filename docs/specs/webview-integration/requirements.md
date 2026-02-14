# 웹뷰 통합 — 요구사항 분석서

## 1. 개요

### 기능명
F-02: 웹뷰 통합

### 목적
webOS 플랫폼에서 제공하는 WebView API를 활용하여 외부 웹 페이지를 앱 내에서 안전하고 효율적으로 렌더링합니다. 이는 브라우저 앱의 핵심 기능으로, 사용자가 입력한 URL의 웹 콘텐츠를 실제로 표시하는 기반을 마련합니다.

### 대상 사용자
- 프로젝터로 웹 페이지를 탐색하려는 일반 사용자
- 스트리밍 서비스(YouTube, Netflix)를 이용하려는 사용자
- 뉴스, 블로그, 포털 사이트를 리모컨으로 탐색하려는 사용자

### 요청 배경
- F-01(프로젝트 초기 설정)에서 기본 프로젝트 구조가 완성됨
- 브라우저 앱의 핵심 기능인 웹 페이지 렌더링 구현 필요
- webOS의 WebView API 제약(보안 정책, 리소스 제한)을 고려한 안정적인 통합 필요
- 이후 F-04(페이지 탐색), F-05(로딩 인디케이터), F-06(탭 관리) 등이 이 기능에 의존
- PRD에서 정의한 "주요 사이트 렌더링 성공률 95% 이상" 달성의 기초

## 2. 유저 스토리

### US-1: 기본 웹 페이지 렌더링
- **As a** 프로젝터 사용자
- **I want** URL을 입력하면 해당 웹 페이지가 화면에 표시되기를
- **So that** 리모컨만으로 웹 콘텐츠를 즐길 수 있다

### US-2: 로딩 상태 확인
- **As a** 사용자
- **I want** 페이지가 로딩 중인지, 완료되었는지 알 수 있기를
- **So that** 페이지가 준비되기를 기다리며 불필요한 조작을 하지 않을 수 있다

### US-3: 로딩 실패 감지
- **As a** 사용자
- **I want** 페이지 로딩이 실패했을 때 에러 메시지를 보고 싶다
- **So that** 문제를 인지하고 재시도하거나 다른 URL을 시도할 수 있다

### US-4: 페이지 네비게이션 추적
- **As a** 사용자
- **I want** 웹 페이지 내에서 링크를 클릭했을 때 브라우저가 이를 인지하기를
- **So that** 뒤로 가기 버튼이나 URL 바가 자동으로 업데이트될 수 있다

### US-5: 안전한 웹뷰 종료
- **As a** 사용자
- **I want** 웹 페이지를 닫을 때 메모리가 제대로 해제되기를
- **So that** 여러 페이지를 탐색해도 앱이 느려지거나 멈추지 않는다

## 3. 기능 요구사항 (Functional Requirements)

### FR-1: webOS WebView API 조사 및 선택
- **설명**: webOS에서 제공하는 웹 페이지 렌더링 API를 조사하고 적합한 방식 선택
- **우선순위**: Must
- **세부 요구사항**:
  - webOS WebView 컴포넌트 조사 (Enact webos/WebView 패키지)
  - HTML iframe 태그와의 비교 분석
  - 각 방식의 제약사항 파악:
    - 보안 정책 (CORS, CSP)
    - 리소스 접근 제한 (로컬 파일, webOS API)
    - 이벤트 리스닝 가능 여부
    - 메모리/성능 차이
  - 선택 기준:
    - webOS 공식 지원 여부
    - 페이지 로딩 이벤트 수신 가능 여부
    - 네비게이션 제어 가능 여부 (뒤로/앞으로)
    - 에러 핸들링 지원 여부
  - 최종 선택 결과 문서화 (설계서에 반영)

### FR-2: WebView 컴포넌트 구현
- **설명**: 선택한 API로 재사용 가능한 WebView React 컴포넌트 구축
- **우선순위**: Must
- **세부 요구사항**:
  - 컴포넌트 위치: `src/components/WebView/WebView.js`
  - Props 인터페이스:
    - `url` (string, required): 로드할 URL
    - `onLoadStart` (function): 로딩 시작 콜백
    - `onLoadProgress` (function): 로딩 진행률 콜백 (0~100)
    - `onLoadEnd` (function): 로딩 완료 콜백
    - `onLoadError` (function): 로딩 실패 콜백 (에러 정보 포함)
    - `onNavigationChange` (function): URL 변경 콜백 (새 URL 포함)
  - PropTypes 검증 필수
  - 컴포넌트 생명주기 관리:
    - Mount: WebView 초기화
    - URL 변경 시: 새 페이지 로드
    - Unmount: WebView 리소스 해제
  - 스타일링:
    - 부모 컨테이너에 맞춰 전체 화면 채우기
    - 리모컨 포커스 가능 (Spotlight 통합)

### FR-3: 페이지 로딩 이벤트 처리
- **설명**: 웹 페이지 로딩의 각 단계를 감지하고 상위 컴포넌트에 전달
- **우선순위**: Must
- **세부 요구사항**:
  - 로딩 시작 이벤트 (loadstart):
    - URL 로드 시작 시점 감지
    - 로딩 시작 시각 기록
  - 로딩 진행률 이벤트 (loadprogress):
    - 진행률(0~100) 계산
    - 프로그레스바 표시를 위한 데이터 제공 (F-05 연계)
  - 로딩 완료 이벤트 (loadend):
    - 페이지 렌더링 완료 감지
    - 로딩 소요 시간 계산 (성능 모니터링용)
    - 페이지 제목(title) 추출 (탭/북마크 표시용)
  - DOM 준비 이벤트 (domready, 선택):
    - 스크립트 실행 전 DOM 트리 완성 시점 감지

### FR-4: 네비게이션 이벤트 처리
- **설명**: 웹 페이지 내 링크 클릭 등으로 URL이 변경될 때 이를 추적
- **우선순위**: Must
- **세부 요구사항**:
  - URL 변경 감지 (navigation):
    - 링크 클릭 시 새 URL 감지
    - 리다이렉트 감지 (예: http → https)
    - Form 제출로 인한 URL 변경 감지
  - 변경된 URL 상위 컴포넌트에 전달:
    - URLBar 업데이트용 (F-03 연계)
    - 히스토리 기록용 (F-08 연계)
  - 페이지 제목 변경 감지:
    - 동적으로 변경되는 제목 추적 (SPA 대응)

### FR-5: 에러 처리
- **설명**: 페이지 로딩 실패 시 에러 정보를 수집하고 상위 컴포넌트에 전달
- **우선순위**: Must
- **세부 요구사항**:
  - 에러 타입 분류:
    - 네트워크 에러 (연결 실패, 타임아웃)
    - HTTP 에러 (404, 500 등)
    - 렌더링 에러 (잘못된 HTML, 스크립트 오류)
    - CORS 에러 (외부 리소스 차단)
  - 에러 정보 구조:
    - `errorCode` (number): HTTP 상태 코드 또는 시스템 에러 코드
    - `errorMessage` (string): 사용자에게 표시할 메시지
    - `url` (string): 실패한 URL
  - 에러 콜백 호출:
    - `onLoadError({errorCode, errorMessage, url})`
  - ErrorPage 표시 연계 (F-10)

### FR-6: 웹뷰 상태 관리
- **설명**: WebView의 현재 상태를 추적하고 관리
- **우선순위**: Must
- **세부 요구사항**:
  - 상태 정의:
    - `idle`: 초기 상태 (URL 로드 전)
    - `loading`: 로딩 중
    - `loaded`: 로딩 완료
    - `error`: 로딩 실패
  - 상태 전환 규칙:
    - `idle` → `loading`: URL 로드 시작
    - `loading` → `loaded`: 로딩 성공
    - `loading` → `error`: 로딩 실패
    - `loaded` → `loading`: 새 URL 로드 시작
  - 상태 기반 UI 제어:
    - `loading` 시 로딩 인디케이터 표시 (F-05)
    - `error` 시 에러 페이지 표시 (F-10)
    - `loaded` 시 웹 콘텐츠 표시
  - 상태 변경 콜백:
    - `onStateChange(newState, oldState)`

### FR-7: 메모리 관리 및 리소스 해제
- **설명**: WebView 인스턴스의 생명주기를 관리하여 메모리 누수 방지
- **우선순위**: Must
- **세부 요구사항**:
  - 컴포넌트 Unmount 시:
    - 진행 중인 네트워크 요청 취소
    - 이벤트 리스너 정리
    - WebView 인스턴스 제거
  - URL 변경 시:
    - 이전 페이지 리소스 해제 (선택, API 지원 시)
  - 메모리 모니터링 (개발 모드):
    - 로딩 전후 메모리 사용량 로깅
    - 탭당 메모리 사용량 추적 (F-06 연계)

### FR-8: BrowserView에 WebView 통합
- **설명**: 구현한 WebView 컴포넌트를 메인 브라우저 화면에 통합
- **우선순위**: Must
- **세부 요구사항**:
  - `src/views/BrowserView/BrowserView.js` 수정:
    - WebView 컴포넌트 import
    - 초기 URL 설정 (테스트용, 예: https://www.google.com)
    - 로딩 이벤트 핸들러 구현 (콘솔 로그)
  - 레이아웃:
    - 상단: URLBar 영역 (F-03에서 구현 예정, 현재는 빈 공간 또는 임시 텍스트)
    - 중앙: WebView (화면 대부분 차지)
    - 하단: NavigationBar 영역 (F-04에서 구현 예정)
  - 테스트 시나리오:
    - 앱 실행 → 초기 URL 자동 로드
    - 로딩 상태 콘솔 로그 출력
    - 페이지 완료 시 웹 콘텐츠 표시

## 4. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **페이지 로딩 시간**:
  - 일반 웹 페이지: 5초 이내 (PRD 기준)
  - 경량 페이지 (Google 홈): 2초 이내
  - 측정 기준: `onLoadStart` ~ `onLoadEnd` 시간 차이
- **메모리 사용량**:
  - 단일 웹뷰 최대 200MB (PRD 기준)
  - 메모리 누수 없음 (1시간 연속 사용 시 메모리 증가 10% 이내)
- **이벤트 응답 속도**:
  - 로딩 이벤트 콜백 호출 지연: 100ms 이내
  - URL 변경 감지 지연: 200ms 이내

### UX (리모컨 최적화)
- **포커스 관리**:
  - WebView 내 링크/버튼에 리모컨 십자키로 포커스 이동 가능
  - Spotlight 통합: 웹 콘텐츠 외부 UI(URLBar, NavigationBar)와 포커스 전환 매끄럽게
- **가독성**:
  - 웹 페이지 확대/축소 금지 (대화면 전제, viewport meta 태그 제어)
  - 기본 폰트 크기 유지 (웹 페이지 자체 스타일 존중)
- **에러 복구**:
  - 에러 발생 시 명확한 메시지 표시 (F-10 연계)
  - 재시도 옵션 제공 (F-10 연계)

### 보안
- **HTTPS 우선**:
  - HTTP URL 입력 시 HTTPS로 자동 업그레이드 시도 (선택, F-14 연계)
  - 비보안 사이트 접속 시 경고 표시 준비 (F-14에서 구현)
- **쿠키 관리**:
  - WebView의 쿠키 저장소 격리 (다른 앱과 공유 방지)
  - 설정에서 쿠키 삭제 기능 준비 (F-11 연계)
- **악성 사이트 방지**:
  - 현 단계에서는 미구현 (추후 확장)
  - JavaScript 실행 허용 (기본 웹 기능 제공 위해)

### 호환성
- **주요 사이트 렌더링 성공**:
  - 목표: 95% 이상 (PRD 기준)
  - 테스트 대상:
    - YouTube (https://www.youtube.com)
    - Netflix (https://www.netflix.com)
    - Google (https://www.google.com)
    - Naver (https://www.naver.com)
    - Wikipedia (https://www.wikipedia.org)
- **HTML5/CSS3 지원**:
  - webOS Chromium 엔진 기반 (버전에 따라 ES6+ 지원)
  - Canvas, Video, Audio 태그 동작 확인
- **webOS 버전**:
  - webOS 4.x 이상 지원 (LG 프로젝터 HU175QW 기준)

### 안정성
- **크래시 방지**:
  - 잘못된 URL (예: `invalid://url`) 입력 시에도 앱 크래시 없음
  - 메모리 부족 시 Graceful degradation (에러 메시지 표시)
- **장시간 사용**:
  - 1시간 연속 사용 시 웹뷰 정상 동작
  - 여러 페이지 탐색 후에도 성능 저하 없음
- **에러 격리**:
  - 웹 페이지 스크립트 오류가 브라우저 앱에 영향 주지 않음

### 확장성
- **다중 웹뷰 지원 준비**:
  - 현 단계: 단일 웹뷰
  - 향후(F-06): 탭별 웹뷰 인스턴스 관리
  - 설계 시 다중 인스턴스를 고려한 구조
- **WebView 설정 확장 가능**:
  - JavaScript on/off (향후 설정 기능)
  - User-Agent 커스터마이징 (향후 필요 시)

## 5. 제약조건

### 플랫폼 제약
- **webOS WebView API 제한**:
  - 일부 웹 API가 제한될 수 있음 (예: Geolocation, WebRTC 제약)
  - CORS 정책이 일반 브라우저와 다를 수 있음 (webOS 보안 정책)
  - 파일 업로드/다운로드 기능 제약 (F-12에서 다루지만 API 지원 여부 확인 필요)
- **리모컨 입력**:
  - 웹 페이지의 마우스 이벤트를 리모컨 십자키로 변환해야 함
  - 일부 웹 페이지는 터치/마우스 전제로 설계되어 조작 어려움 (사용자 교육 필요)

### 기술 제약
- **메모리 제한**:
  - webOS 디바이스(프로젝터)의 RAM 제약 (일반적으로 1~2GB)
  - 탭당 200MB 이하로 메모리 사용 제한 (PRD)
- **네트워크**:
  - Wi-Fi 연결 필수 (유선랜 대안)
  - 네트워크 불안정 시 타임아웃 처리 필요 (기본 30초)
- **Enact 프레임워크**:
  - Enact의 Spotlight 시스템과 웹 페이지 내 포커스 관리 충돌 가능성
  - WebView 컴포넌트가 Enact 생명주기와 호환되도록 구현 필요

### 개발 환경 제약
- **webOS 시뮬레이터 제약**:
  - WebView가 시뮬레이터에서 제한적으로 동작할 수 있음
  - 실제 프로젝터 디바이스에서 테스트 필수
- **디버깅 제약**:
  - 웹 페이지 내부 디버깅은 webOS Developer Tools 필요
  - Chrome DevTools 원격 디버깅 설정 복잡도 높음

### 의존성
- **선행 기능**:
  - F-01 (프로젝트 초기 설정) 완료 필요
- **후속 기능**:
  - F-03 (URL 입력 UI) - WebView에 URL 전달
  - F-04 (페이지 탐색 컨트롤) - WebView 네비게이션 제어
  - F-05 (로딩 인디케이터) - WebView 로딩 이벤트 사용
  - F-06 (탭 관리) - WebView 인스턴스 관리
  - F-08 (히스토리 관리) - WebView URL 변경 이벤트 사용
  - F-10 (에러 처리) - WebView 에러 이벤트 사용

## 6. 용어 정의

| 용어 | 정의 |
|------|------|
| WebView | 앱 내에서 웹 콘텐츠를 렌더링하는 UI 컴포넌트 (네이티브 브라우저 엔진 사용) |
| iframe | HTML 표준 인라인 프레임 태그, 웹 페이지 내 다른 웹 페이지 삽입 |
| CORS | Cross-Origin Resource Sharing, 다른 도메인의 리소스 접근 보안 정책 |
| CSP | Content Security Policy, 웹 페이지 콘텐츠 보안 정책 (스크립트 실행 제한 등) |
| SPA | Single Page Application, 페이지 전환 없이 동적으로 콘텐츠를 변경하는 웹 앱 |
| Spotlight | Enact의 리모컨 포커스 관리 시스템 (십자키로 UI 요소 간 이동) |
| Graceful Degradation | 오류 발생 시 기능 축소하여 앱 전체 중단 방지하는 설계 원칙 |
| User-Agent | HTTP 요청 헤더, 브라우저/앱 식별 정보 (서버가 모바일/데스크톱 구분) |

## 7. 범위 외 (Out of Scope)

### 이번 단계에서 하지 않는 것
- **URL 입력 UI**: F-03에서 구현 (현 단계는 하드코딩된 초기 URL 사용)
- **뒤로/앞으로 버튼**: F-04에서 구현 (현 단계는 WebView 이벤트 수신만)
- **프로그레스바 UI**: F-05에서 구현 (현 단계는 콘솔 로그만)
- **다중 탭**: F-06에서 구현 (현 단계는 단일 WebView)
- **북마크/히스토리 저장**: F-07, F-08에서 구현 (현 단계는 URL 변경 감지만)
- **다운로드 처리**: F-12에서 구현
- **HTTPS 보안 표시**: F-14에서 구현
- **에러 페이지 UI**: F-10에서 구현 (현 단계는 onLoadError 콜백만)
- **설정 기능**: F-11에서 구현 (JavaScript on/off, User-Agent 등)

### 추후 확장 가능 기능
- **광고 차단**: 현재 범위 외, 향후 플러그인으로 추가 가능
- **스크린샷**: webOS API 지원 시 구현 가능
- **페이지 내 검색** (Ctrl+F 같은): 리모컨 조작 복잡도 높아 보류
- **시크릿 모드**: 쿠키/히스토리 저장 안 함 (추후 확장)

## 8. 완료 기준 (Acceptance Criteria)

### AC-1: webOS WebView API 조사 완료
- [ ] webOS WebView 컴포넌트 문서 확인
- [ ] iframe vs WebView 비교표 작성
- [ ] 최종 선택 결정 및 근거 문서화 (설계서에 포함)

### AC-2: WebView 컴포넌트 구현 완료
- [ ] `src/components/WebView/WebView.js` 생성
- [ ] Props 인터페이스 정의 (url, onLoadStart, onLoadProgress, onLoadEnd, onLoadError, onNavigationChange)
- [ ] PropTypes 검증 추가
- [ ] 컴포넌트 생명주기 구현 (mount, unmount, url 변경 시)

### AC-3: 로딩 이벤트 처리 완료
- [ ] `onLoadStart` 콜백 호출 확인 (콘솔 로그)
- [ ] `onLoadProgress` 진행률(0~100) 콜백 호출 확인
- [ ] `onLoadEnd` 콜백 호출 및 페이지 제목 추출 확인
- [ ] 로딩 소요 시간 계산 및 로깅

### AC-4: 네비게이션 이벤트 처리 완료
- [ ] 페이지 내 링크 클릭 시 `onNavigationChange` 콜백 호출 확인
- [ ] 새 URL 정상 전달 확인
- [ ] 리다이렉트 감지 확인 (예: http → https)

### AC-5: 에러 처리 완료
- [ ] 네트워크 에러 감지 (Wi-Fi 끄고 테스트)
- [ ] 404 에러 감지 (존재하지 않는 URL)
- [ ] `onLoadError` 콜백 호출 및 에러 정보(errorCode, errorMessage, url) 확인

### AC-6: 상태 관리 완료
- [ ] 상태 전이 구현 (idle → loading → loaded/error)
- [ ] 콘솔에 상태 변경 로그 출력
- [ ] 상태에 따른 UI 제어 (loading 시 텍스트 표시 등)

### AC-7: 메모리 관리 확인
- [ ] 컴포넌트 Unmount 시 리소스 해제 확인 (콘솔 로그)
- [ ] 10개 페이지 연속 로드 후 메모리 사용량 확인 (200MB 이하)
- [ ] 1시간 연속 사용 후 메모리 누수 확인 (10% 이내 증가)

### AC-8: BrowserView 통합 완료
- [ ] `src/views/BrowserView/BrowserView.js`에 WebView 통합
- [ ] 초기 URL (https://www.google.com) 자동 로드
- [ ] 로딩 이벤트 핸들러 구현 (콘솔 로그)
- [ ] 페이지 렌더링 확인 (Google 홈페이지 표시)

### AC-9: 주요 사이트 렌더링 테스트
- [ ] YouTube (https://www.youtube.com) 정상 렌더링
- [ ] Netflix (https://www.netflix.com) 정상 렌더링
- [ ] Google (https://www.google.com) 정상 렌더링
- [ ] Naver (https://www.naver.com) 정상 렌더링
- [ ] Wikipedia (https://www.wikipedia.org) 정상 렌더링
- [ ] 렌더링 성공률: 5개 중 5개 (100%)

### AC-10: 리모컨 조작 확인
- [ ] WebView 포커스 시 십자키로 링크 간 이동 가능
- [ ] Enter 키로 링크 클릭 가능
- [ ] Spotlight 통합 확인 (외부 UI와 포커스 전환)

### AC-11: 성능 기준 충족
- [ ] Google 홈페이지 로딩 시간: 2초 이내
- [ ] 일반 페이지 로딩 시간: 5초 이내 (예: Naver 홈)
- [ ] 이벤트 콜백 지연: 100ms 이내

### AC-12: 실제 디바이스 테스트
- [ ] LG 프로젝터 HU175QW에서 앱 실행 성공
- [ ] WebView 정상 렌더링 확인
- [ ] 리모컨으로 페이지 조작 확인

### AC-13: 문서화
- [ ] WebView 컴포넌트 주석 (한국어) 추가
- [ ] Props 인터페이스 문서화 (JSDoc 또는 주석)
- [ ] 사용 예시 추가 (BrowserView 코드)
- [ ] API 선택 근거 문서 (설계서에 포함)

## 9. 우선순위 및 복잡도

- **우선순위**: Must (브라우저 앱의 핵심 기능)
- **예상 복잡도**: High
  - webOS API 학습 곡선
  - 이벤트 리스닝 구현 복잡도
  - Enact/Spotlight 통합 난이도
  - 실제 디바이스 테스트 필요성
- **예상 소요 시간**: 1~2일
  - API 조사: 3~4시간
  - 컴포넌트 구현: 4~6시간
  - 통합 및 테스트: 4~6시간
  - 디바이스 테스트: 2~3시간

## 10. 리스크

| 리스크 | 확률 | 영향도 | 대응 방안 |
|--------|------|--------|-----------|
| webOS WebView API 문서 부족 | 중 | 고 | Enact 커뮤니티, LG 개발자 포럼 활용. 최악의 경우 iframe fallback |
| 주요 사이트 렌더링 실패 (예: Netflix DRM) | 중 | 중 | 사이트별 호환성 테스트, 불가능한 사이트는 사용자에게 안내 |
| 리모컨 포커스와 웹 페이지 내 포커스 충돌 | 고 | 중 | Spotlight 설정 조정, 웹 페이지 내 포커스는 웹 표준 탐색 사용 |
| 메모리 누수 발생 | 중 | 고 | 철저한 리소스 해제 구현, React 생명주기 활용, 프로파일링 |
| 네트워크 불안정 시 타임아웃 처리 미흡 | 중 | 중 | 타임아웃 설정 (30초), 재시도 옵션 제공 (F-10 연계) |
| 실제 디바이스에서만 발생하는 버그 | 고 | 중 | 조기 실제 디바이스 테스트, 로그 수집 메커니즘 구축 |

## 11. 의존성

### 선행 기능
- **F-01 (프로젝트 초기 설정)**: 완료 ✅

### 후속 기능
- **F-03 (URL 입력 UI)**: WebView에 동적 URL 전달
- **F-04 (페이지 탐색 컨트롤)**: WebView 네비게이션 API 호출 (뒤로/앞으로)
- **F-05 (로딩 인디케이터)**: WebView 로딩 이벤트 구독
- **F-06 (탭 관리)**: 다중 WebView 인스턴스 관리
- **F-08 (히스토리 관리)**: WebView URL 변경 이벤트 저장
- **F-10 (에러 처리)**: WebView 에러 이벤트로 에러 페이지 표시

## 12. 참고 자료

### webOS/Enact 문서
- Enact 공식 문서: https://enactjs.com/docs/
- webOS TV 개발자 가이드: https://webostv.developer.lge.com
- Enact WebView (가능 시): https://github.com/enactjs/enact
- Moonstone 컴포넌트: https://enactjs.com/docs/modules/moonstone/

### 기존 프로젝트 문서
- PRD: `/Users/jsong/dev/jsong1230-github/webosbrowser/docs/project/prd.md`
- 기능 백로그: `/Users/jsong/dev/jsong1230-github/webosbrowser/docs/project/features.md`
- F-01 요구사항: `/Users/jsong/dev/jsong1230-github/webosbrowser/docs/specs/project-setup/requirements.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md`

### 기술 참고
- HTML iframe MDN: https://developer.mozilla.org/en-US/docs/Web/HTML/Element/iframe
- React 생명주기: https://react.dev/learn/lifecycle-of-reactive-effects
- webOS LS2 API (필요 시): https://webostv.developer.lge.com/develop/references/luna-service2-api/

## 13. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-12 | 초기 요구사항 분석서 작성 | product-manager |
