# 프로젝트 초기 설정 — 요구사항 분석서

## 1. 개요

### 기능명
F-01: 프로젝트 초기 설정

### 목적
webOS Enact 프레임워크 기반의 브라우저 앱 프로젝트를 생성하고, 개발에 필요한 기본 구조(디렉토리, 라우팅, 스타일, 린트)를 세팅하여 이후 기능 개발을 위한 기반을 마련합니다.

### 대상 사용자
개발자 (프로젝트 팀 내부)

### 요청 배경
- webOS 브라우저 앱 개발을 위한 프로젝트 뼈대 구축 필요
- Enact 프레임워크의 모범 사례(디렉토리 구조, 라우팅, 스타일링)를 따르는 초기 설정 필요
- 코드 품질 유지를 위한 ESLint, Prettier 설정 필요
- 이 단계는 F-02(웹뷰), F-03(URL 입력), F-04(탐색 컨트롤) 등 모든 후속 기능의 의존성 시작점

## 2. 기능 요구사항 (Functional Requirements)

### FR-1: webOS Enact 프로젝트 생성
- **설명**: Enact CLI를 사용하여 webOS TV/프로젝터용 React 앱 프로젝트 생성
- **유저 스토리**: As a 개발자, I want Enact CLI로 프로젝트를 생성하여, webOS 플랫폼에 최적화된 초기 구조를 확보하고 싶다
- **우선순위**: Must
- **세부 요구사항**:
  - Enact CLI 설치 (npm install -g @enact/cli)
  - 프로젝트 생성 명령어: `enact create webosbrowser`
  - Moonstone 테마 사용 (리모컨 최적화 UI 컴포넌트)
  - 생성된 기본 파일 확인: package.json, webpack.config.js, src/App/, webos-meta/

### FR-2: 디렉토리 구조 구성
- **설명**: 확장 가능하고 유지보수가 쉬운 디렉토리 구조 구축
- **유저 스토리**: As a 개발자, I want 명확한 디렉토리 구조를 만들어, 각 레이어(컴포넌트/서비스/유틸)의 역할을 분리하고 싶다
- **우선순위**: Must
- **세부 요구사항**:
  - `src/components/` — UI 컴포넌트 (재사용 가능한 컴포넌트)
  - `src/views/` — 화면 단위 컴포넌트 (BrowserView, HomePage, ErrorPage)
  - `src/services/` — 비즈니스 로직 (북마크, 히스토리, 로컬 저장소 래퍼)
  - `src/hooks/` — 커스텀 React Hooks (리모컨, 웹뷰 상태 관리)
  - `src/utils/` — 유틸리티 함수 (URL 검증, 로거)
  - `webos-meta/` — webOS 메타데이터 (appinfo.json, 아이콘)
  - `docs/` — 프로젝트 문서
  - 각 디렉토리에 README.md 또는 .gitkeep 파일 추가 (빈 디렉토리 커밋용)

### FR-3: 라우팅 설정
- **설명**: 화면 간 전환을 위한 기본 라우팅 구조 구축 (Enact Router 또는 React 상태 기반)
- **유저 스토리**: As a 개발자, I want 라우팅 메커니즘을 설정하여, 브라우저 메인 화면, 설정 화면, 에러 페이지 간 전환을 구현하고 싶다
- **우선순위**: Must
- **세부 요구사항**:
  - `src/App/App.js`에 라우팅 로직 구현
  - 초기 화면: BrowserView (메인 브라우저 UI)
  - 라우트 예시:
    - `/` — BrowserView (메인)
    - `/settings` — SettingsPanel (설정, F-11에서 구현)
    - `/error` — ErrorPage (에러 페이지, F-10에서 구현)
  - 리모컨 Back 버튼으로 이전 화면 복귀 지원 준비 (실제 구현은 F-04)

### FR-4: 전역 스타일 설정
- **설명**: Moonstone 테마 적용 및 프로젝트 공통 스타일 정의
- **유저 스토리**: As a 개발자, I want 전역 스타일을 설정하여, 대화면 가독성(폰트 크기, 색상)과 일관된 UI를 구축하고 싶다
- **우선순위**: Must
- **세부 요구사항**:
  - Moonstone 테마 적용 (Enact 기본)
  - 전역 CSS 변수 정의 (`src/App/App.module.less`):
    - `--font-size-min: 24px` (최소 폰트 크기, 3m 거리 가독성)
    - `--focus-border: 4px solid #00aaff` (포커스 표시)
    - `--bg-color: #1e1e1e` (다크 테마)
    - `--text-color: #ffffff`
  - Spotlight 설정 (리모컨 포커스 스타일)
  - CSS Modules 사용 (`.module.less`)

### FR-5: ESLint, Prettier 설정
- **설명**: 코드 품질 유지를 위한 린트 및 포맷터 설정
- **유저 스토리**: As a 개발자, I want 린트와 포맷터를 설정하여, 팀 내 코드 스타일 일관성을 유지하고 싶다
- **우선순위**: Must
- **세부 요구사항**:
  - `.eslintrc.js` 생성:
    - 규칙: Enact 권장 + React Hooks 규칙
    - PropTypes 검증 필수
    - no-console 경고 (logger.js 사용 권장)
  - `.prettierrc` 생성:
    - 탭 사용 (Enact 스타일)
    - 세미콜론 제거
    - 싱글 쿼터 사용
  - `package.json`에 스크립트 추가:
    - `"lint": "eslint src/"`
    - `"format": "prettier --write src/"`

### FR-6: Hello World 화면 구현
- **설명**: 프로젝트 실행 시 "Hello webOS Browser" 메시지를 보여주는 최소 화면 구현
- **유저 스토리**: As a 개발자, I want 앱을 실행하여 기본 화면이 정상적으로 표시되는지 확인하고 싶다
- **우선순위**: Must
- **세부 요구사항**:
  - `src/App/App.js`에서 "Hello webOS Browser" 텍스트 렌더링
  - Moonstone의 `<Panel>` 컴포넌트 사용
  - 중앙 정렬, 큰 폰트 (36px)
  - 리모컨으로 포커스 가능한 버튼 1개 추가 (예: "시작" 버튼, 클릭 시 콘솔 로그)

## 3. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **앱 실행 시간**: 앱 실행 후 Hello World 화면까지 3초 이내
- **빌드 시간**: `npm run pack` 명령어로 프로덕션 빌드 30초 이내

### 코드 품질
- ESLint 에러 0개 (경고는 허용)
- PropTypes 검증 누락 없음
- 모든 컴포넌트에 주석 (한국어) 추가

### 확장성
- 디렉토리 구조는 15개 이상의 컴포넌트를 수용 가능해야 함
- 새로운 화면 추가 시 라우팅 로직 수정만으로 가능해야 함

### 유지보수성
- 각 디렉토리에 역할 설명 주석 또는 README 추가
- 코드 중복 최소화 (공통 스타일은 전역 CSS로)

## 4. 제약조건

### 플랫폼 제약
- webOS 4.0 이상 지원
- Enact CLI 버전 4.x 이상 사용
- Node.js 16.x 이상, npm 8.x 이상 필요

### 기술 제약
- Enact Moonstone 테마 필수 사용 (리모컨 최적화)
- CSS Modules만 사용 (인라인 스타일 금지)
- React Class 컴포넌트 금지 (함수형 컴포넌트만)

### 개발 환경 제약
- macOS 환경 (webOS CLI 설치 필요)
- LG 프로젝터 개발자 모드 설정 완료 필요 (ares-setup-device)

## 5. 용어 정의

| 용어 | 정의 |
|------|------|
| Enact | LG에서 개발한 React 기반 webOS UI 프레임워크 |
| Moonstone | Enact의 리모컨 최적화 UI 컴포넌트 라이브러리 |
| Spotlight | Enact의 포커스 관리 시스템 (리모컨 방향키로 UI 요소 간 이동) |
| LS2 API | webOS 로컬 서비스 API (로컬 저장소, 시스템 기능 접근) |
| Spotlightable | Spotlight 시스템에서 포커스 가능한 컴포넌트 |
| appinfo.json | webOS 앱 메타데이터 (ID, 버전, 제목 등) |
| IPK | webOS 앱 패키지 포맷 (Android APK와 유사) |

## 6. 범위 외 (Out of Scope)

### 이번 단계에서 하지 않는 것
- **웹뷰 통합**: F-02에서 구현
- **URL 입력 UI**: F-03에서 구현
- **페이지 탐색 컨트롤**: F-04에서 구현
- **로딩 인디케이터**: F-05에서 구현
- **실제 웹 페이지 렌더링**: 이 단계는 "Hello World" 정적 화면만
- **리모컨 단축키 매핑**: F-12에서 구현 (이 단계는 기본 포커스만)
- **북마크/히스토리 관리**: F-07, F-08에서 구현
- **설정 화면**: F-11에서 구현

## 7. 완료 기준 (Acceptance Criteria)

### AC-1: 프로젝트 생성 완료
- [ ] `enact create webosbrowser` 명령어로 프로젝트 생성
- [ ] `package.json`에 Enact 의존성 포함 확인
- [ ] `npm install` 실행 시 에러 없음

### AC-2: 디렉토리 구조 구축 완료
- [ ] `src/components/`, `src/views/`, `src/services/`, `src/hooks/`, `src/utils/` 생성
- [ ] 각 디렉토리에 `.gitkeep` 또는 README 추가
- [ ] `webos-meta/appinfo.json` 존재 및 앱 ID 설정 (`com.jsong.webosbrowser`)

### AC-3: 라우팅 동작 확인
- [ ] `src/App/App.js`에서 BrowserView 라우트 렌더링
- [ ] (선택) React Router 또는 상태 기반 라우팅 구조 확인

### AC-4: 전역 스타일 적용 확인
- [ ] Moonstone 테마 적용됨 (버튼, 패널 등 Moonstone 스타일)
- [ ] `src/App/App.module.less`에 전역 CSS 변수 정의
- [ ] 최소 폰트 크기 24px 적용 확인

### AC-5: 린트/포맷 설정 완료
- [ ] `.eslintrc.js` 생성 및 규칙 설정
- [ ] `.prettierrc` 생성
- [ ] `npm run lint` 실행 시 에러 0개

### AC-6: Hello World 화면 표시
- [ ] `npm run serve` 실행 시 로컬 서버 구동 (http://localhost:8080)
- [ ] 브라우저에서 "Hello webOS Browser" 텍스트 표시
- [ ] "시작" 버튼 클릭 시 콘솔 로그 출력 확인

### AC-7: webOS 프로젝터 배포 테스트 (선택)
- [ ] `npm run pack-p` 실행 시 빌드 성공
- [ ] `ares-package dist/` 실행 시 IPK 생성
- [ ] `ares-install --device projector {ipk}` 실행 시 설치 성공
- [ ] 프로젝터에서 앱 실행 시 "Hello webOS Browser" 화면 표시

### AC-8: 문서화
- [ ] `CLAUDE.md`에 디렉토리 구조 반영
- [ ] `docs/development-guide.md`에 프로젝트 세팅 가이드 작성 (선택, F-01 완료 후)
- [ ] `.gitignore`에 `dist/`, `node_modules/`, `.worktrees/` 추가

## 8. 우선순위 및 복잡도

- **우선순위**: Must (모든 후속 기능의 의존성 시작점)
- **예상 복잡도**: Low
- **예상 소요 시간**: 2~3시간 (Enact CLI 설치 + 프로젝트 생성 + 구조 세팅)

## 9. 의존성

- **선행 기능**: 없음 (프로젝트 시작점)
- **후속 기능**: F-02(웹뷰), F-03(URL 입력), F-04(탐색 컨트롤), F-05(로딩), F-06(탭), F-07(북마크) 등 모든 기능

## 10. 참고 자료

- Enact 공식 문서: https://enactjs.com/docs/
- Enact CLI: https://github.com/enactjs/cli
- webOS TV 개발자 사이트: https://webostv.developer.lge.com
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md`
- PRD: `/Users/jsong/dev/jsong1230-github/webosbrowser/docs/project/prd.md`
- 기능 백로그: `/Users/jsong/dev/jsong1230-github/webosbrowser/docs/project/features.md`
