# 프로젝트 초기 설정 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: `docs/specs/project-setup/requirements.md`
- 기술 설계서: `docs/specs/project-setup/design.md`
- 기능 백로그: `docs/project/features.md`

## 2. 구현 개요

### 목표
webOS Enact 프레임워크 기반의 브라우저 앱 프로젝트를 생성하고, 개발에 필요한 기본 구조(디렉토리, 라우팅, 스타일, 린트)를 세팅하여 F-02 이후 모든 기능 개발의 기반을 마련합니다.

### 작업 범위
- Enact CLI로 프로젝트 생성 (Moonstone 테마)
- 디렉토리 구조 구성 (components, views, services, hooks, utils)
- Hello World 화면 구현 (리모컨 포커스 테스트용)
- 전역 스타일 설정 (CSS 변수, Moonstone 커스터마이징)
- ESLint + Prettier 설정
- webOS 메타데이터 설정 (appinfo.json, 아이콘)
- 로컬 테스트 (브라우저) 및 선택적 프로젝터 배포 테스트

### 예상 소요 시간
- **총 소요 시간**: 2~3시간
- **복잡도**: Low (초기 설정이므로 비즈니스 로직 없음)

## 3. 태스크 분해

### T-1: Enact CLI 설치 및 프로젝트 생성
- **담당**: backend-dev (프론트엔드 전용 프로젝트이지만 초기 설정은 backend-dev가 처리)
- **내용**:
  - Enact CLI 설치: `npm install -g @enact/cli`
  - 프로젝트 생성: `enact create webosbrowser`
  - 생성된 프로젝트로 이동 및 의존성 설치: `cd webosbrowser && npm install`
- **산출물**:
  - `webosbrowser/` 디렉토리 생성 (package.json, webpack.config.js, src/App/, webos-meta/ 포함)
- **완료 기준**:
  - `package.json`에 `@enact/core`, `@enact/moonstone` 의존성 포함
  - `npm install` 실행 시 에러 없음
- **예상 소요 시간**: 15분

### T-2: 디렉토리 구조 구성
- **담당**: backend-dev
- **내용**:
  - `src/` 하위에 디렉토리 생성:
    - `components/`, `views/`, `services/`, `hooks/`, `utils/`
  - 각 디렉토리에 `.gitkeep` 파일 추가 (빈 디렉토리 커밋용)
  - `src/utils/logger.js` 생성 (console.log 래퍼)
- **산출물**:
  - `src/components/.gitkeep`
  - `src/views/.gitkeep`
  - `src/services/.gitkeep`
  - `src/hooks/.gitkeep`
  - `src/utils/logger.js`
- **완료 기준**:
  - 5개 디렉토리 생성 확인 (ls 명령어)
  - logger.js에 기본 로깅 함수 구현 (info, warn, error)
- **예상 소요 시간**: 10분

### T-3: App.js Hello World 구현
- **담당**: frontend-dev
- **내용**:
  - `src/App/App.js` 수정:
    - Moonstone Panel, Header 컴포넌트 사용
    - "Hello webOS Browser" 텍스트 렌더링 (중앙 정렬, 36px)
    - 리모컨 포커스 가능한 "시작" 버튼 추가
    - 버튼 클릭 시 콘솔 로그 출력 (`console.log('시작 버튼 클릭')`)
  - 상태 기반 화면 전환 준비 (useState로 currentView 관리)
- **산출물**:
  - `src/App/App.js` (수정)
- **완료 기준**:
  - Moonstone Panel, Header 컴포넌트 import 확인
  - "Hello webOS Browser" 텍스트 렌더링
  - "시작" 버튼 클릭 시 콘솔 로그 출력
- **예상 소요 시간**: 20분

### T-4: 전역 스타일 설정
- **담당**: frontend-dev
- **내용**:
  - `src/App/App.module.less` 수정:
    - CSS 변수 정의 (폰트 크기, 색상, 포커스 스타일, 간격)
    - `.app`, `.helloContainer`, `.title`, `.startButton` 스타일 구현
  - Moonstone 테마 유지하면서 CSS 변수로 커스터마이징
- **산출물**:
  - `src/App/App.module.less` (수정)
- **완료 기준**:
  - CSS 변수 10개 이상 정의 (--font-size-*, --bg-color, --focus-* 등)
  - Hello World 화면에 전역 스타일 적용 확인
  - 최소 폰트 크기 24px 적용
- **예상 소요 시간**: 20분

### T-5: ESLint 설정
- **담당**: backend-dev
- **내용**:
  - `.eslintrc.js` 생성:
    - eslint:recommended, plugin:react/recommended, plugin:react-hooks/recommended, plugin:enact/recommended 확장
    - PropTypes 검증 필수 (react/prop-types: error)
    - console.log 경고 (no-console: warn)
    - 세미콜론 없음 (semi: ['error', 'never'])
  - package.json에 lint 스크립트 추가:
    - `"lint": "eslint src/ --ext .js,.jsx"`
    - `"lint:fix": "eslint src/ --ext .js,.jsx --fix"`
- **산출물**:
  - `.eslintrc.js`
  - `package.json` (scripts 수정)
- **완료 기준**:
  - `npm run lint` 실행 시 에러 0개 (경고는 허용)
  - ESLint 규칙 10개 이상 정의
- **예상 소요 시간**: 15분

### T-6: Prettier 설정
- **담당**: backend-dev
- **내용**:
  - `.prettierrc` 생성:
    - useTabs: true, tabWidth: 4
    - semi: false, singleQuote: true
    - trailingComma: none, printWidth: 100
  - package.json에 format 스크립트 추가:
    - `"format": "prettier --write \"src/**/*.{js,jsx,less}\""`
    - `"format:check": "prettier --check \"src/**/*.{js,jsx,less}\""`
- **산출물**:
  - `.prettierrc`
  - `package.json` (scripts 수정)
- **완료 기준**:
  - `npm run format:check` 실행 시 포맷 오류 0개
  - Prettier 규칙 7개 정의
- **예상 소요 시간**: 10분

### T-7: webOS 메타데이터 설정
- **담당**: backend-dev
- **내용**:
  - `webos-meta/appinfo.json` 수정:
    - 앱 ID: `com.jsong.webosbrowser`
    - 버전: `0.1.0`
    - 제목: `webOS Browser`
    - vendor: `jsong`
  - 아이콘 파일 준비 (placeholder):
    - `webos-meta/icon.png` (80x80, 단색 또는 Enact 기본)
    - `webos-meta/largeIcon.png` (130x130, 단색 또는 Enact 기본)
- **산출물**:
  - `webos-meta/appinfo.json` (수정)
  - `webos-meta/icon.png`, `webos-meta/largeIcon.png`
- **완료 기준**:
  - appinfo.json의 id, version, title 확인
  - 아이콘 파일 존재 확인 (크기 무관, placeholder 허용)
- **예상 소요 시간**: 10분

### T-8: .gitignore 및 문서 업데이트
- **담당**: doc-writer
- **내용**:
  - `.gitignore` 생성 또는 수정:
    - `dist/`, `node_modules/`, `.worktrees/`, `.DS_Store` 추가
  - `README.md` 생성:
    - 프로젝트 소개 (webOS 브라우저 앱)
    - 실행 방법 (npm run serve, npm run pack-p)
    - 린트/포맷 명령어
- **산출물**:
  - `.gitignore`
  - `README.md`
- **완료 기준**:
  - .gitignore에 4개 이상 항목 포함
  - README.md에 프로젝트 소개, 실행 방법 포함
- **예상 소요 시간**: 15분

### T-9: 로컬 테스트 (브라우저)
- **담당**: test-runner
- **내용**:
  - 개발 서버 실행: `npm run serve` (http://localhost:8080)
  - 브라우저에서 테스트:
    - "Hello webOS Browser" 텍스트 표시 확인
    - "시작" 버튼 표시 확인
    - 탭 키로 버튼 포커스 이동 확인
    - 버튼 클릭 시 콘솔 로그 출력 확인
  - ESLint 실행: `npm run lint` (에러 0개 확인)
  - Prettier 실행: `npm run format:check` (포맷 오류 0개 확인)
- **산출물**: 없음 (테스트 검증만)
- **완료 기준**:
  - npm run serve 실행 시 에러 없이 서버 구동
  - 브라우저에서 Hello World 화면 표시
  - 린트/포맷 에러 0개
- **예상 소요 시간**: 15분

### T-10: 프로젝터 배포 테스트 (선택)
- **담당**: test-runner
- **내용**:
  - 프로덕션 빌드: `npm run pack-p` (또는 `npm run pack`)
  - IPK 생성: `ares-package dist/`
  - 프로젝터 설치: `ares-install --device projector {ipk파일}`
  - 프로젝터에서 앱 실행 및 동작 확인
- **산출물**: 없음 (테스트 검증만)
- **완료 기준**:
  - IPK 생성 성공
  - 프로젝터 설치 성공
  - 프로젝터에서 "Hello webOS Browser" 화면 표시
- **예상 소요 시간**: 20분 (선택 사항)
- **참고**: 로컬 테스트만으로도 충분하므로, 프로젝터 배포는 F-02 이후 통합 테스트 시점에 진행 가능

### T-11: 코드 및 문서 리뷰
- **담당**: code-reviewer
- **내용**:
  - 코드 리뷰:
    - ESLint 규칙 준수 확인
    - PropTypes 검증 누락 없음 확인
    - CSS Modules 사용 확인 (인라인 스타일 금지)
  - 문서 리뷰:
    - requirements.md ↔ 구현 일치 확인
    - design.md ↔ 구현 일치 확인
    - README.md 내용 정확성 확인
- **산출물**: 없음 (검증 및 피드백만)
- **완료 기준**:
  - 코드 품질 이슈 0건
  - 설계↔구현 불일치 0건
- **예상 소요 시간**: 15분

### T-12: 운영 문서 작성 및 커밋
- **담당**: doc-writer
- **내용**:
  - `docs/dev-log.md` 업데이트 (F-01 진행 기록)
  - `CHANGELOG.md` 업데이트 (0.1.0 — 프로젝트 초기 설정)
  - Git 커밋:
    - 커밋 1: 프로젝트 생성 및 디렉토리 구조
    - 커밋 2: Hello World 구현 및 전역 스타일
    - 커밋 3: ESLint, Prettier 설정
    - 커밋 4: webOS 메타데이터 및 문서
- **산출물**:
  - `docs/dev-log.md` (업데이트)
  - `CHANGELOG.md` (업데이트)
  - Git 커밋 4건
- **완료 기준**:
  - dev-log.md에 F-01 진행 기록 추가
  - CHANGELOG.md에 0.1.0 버전 기록
  - 커밋 메시지 Conventional Commits 형식 준수
- **예상 소요 시간**: 20분

## 4. 태스크 의존성

### 의존성 다이어그램
```
T-1 (Enact 프로젝트 생성)
  │
  ├──▶ T-2 (디렉토리 구조) ──┐
  │                         │
  └──▶ T-5 (ESLint) ────────┤
       T-6 (Prettier) ──────┤
       T-7 (webOS 메타) ────┼──▶ T-9 (로컬 테스트) ──▶ T-11 (리뷰) ──▶ T-12 (문서+커밋)
                            │         │
  T-3 (Hello World) ────────┤         │
  T-4 (전역 스타일) ────────┘         │
                                      │
  T-8 (문서 업데이트) ────────────────┘
                                      │
  T-10 (프로젝터 배포, 선택) ────────┘
```

### 의존성 설명
- **T-1 → 모든 태스크**: 프로젝트 생성 완료 후 모든 작업 시작 가능
- **T-2, T-5, T-6, T-7 → T-9**: 디렉토리, 린트, 메타데이터 완료 후 테스트 가능
- **T-3, T-4 → T-9**: Hello World 구현 및 스타일 완료 후 테스트 가능
- **T-8 → T-9**: 문서 완료 후 최종 테스트 가능
- **T-9 → T-11**: 테스트 통과 후 리뷰 시작
- **T-11 → T-12**: 리뷰 통과 후 문서 작성 및 커밋

## 5. 병렬 실행 판단

### 병렬 가능한 태스크
- **Phase 1**: T-1 (순차, 선행 작업 필요)
- **Phase 2**: T-2, T-5, T-6, T-7 (병렬 가능, 모두 T-1에만 의존)
- **Phase 3**: T-3, T-4, T-8 (병렬 가능, Phase 2 완료 후)
- **Phase 4**: T-9, T-10 (순차, T-10은 선택)
- **Phase 5**: T-11, T-12 (순차)

### Agent Team 사용 권장 여부
**권장하지 않음 (순차 실행)**

**근거**:
- F-01은 **프로젝트 기반 구축**으로, 태스크 간 의존성이 높음
- Enact CLI로 생성된 프로젝트 구조를 여러 에이전트가 동시에 수정하면 충돌 위험
- 독립 작업(T-2, T-5, T-6, T-7)이 4개 있지만, 각 태스크가 단순하여(10~15분) 병렬화 이점이 적음
- 순차 실행 시 총 2~3시간, 병렬 실행 시 30분 절약에 불과
- 초기 설정은 **안정성과 디버깅 용이성**이 우선이므로 순차 실행 권장

### 권장 실행 방식
`/fullstack-feature F-01 프로젝트 초기 설정` (서브에이전트 순차 실행)

## 6. 구현 순서 (권장)

### Phase 1: 프로젝트 생성 (T-1)
1. Enact CLI 설치 및 프로젝트 생성
2. 생성 확인 및 의존성 설치

### Phase 2: 기본 구조 및 설정 (T-2, T-5, T-6, T-7)
1. 디렉토리 구조 구성 (T-2)
2. ESLint 설정 (T-5)
3. Prettier 설정 (T-6)
4. webOS 메타데이터 설정 (T-7)

### Phase 3: Hello World 구현 (T-3, T-4)
1. App.js Hello World 구현 (T-3)
2. 전역 스타일 설정 (T-4)

### Phase 4: 문서 업데이트 (T-8)
1. .gitignore 및 README.md 작성

### Phase 5: 테스트 (T-9, T-10)
1. 로컬 테스트 (브라우저) (T-9)
2. 프로젝터 배포 테스트 (T-10, 선택)

### Phase 6: 리뷰 및 마무리 (T-11, T-12)
1. 코드 및 문서 리뷰 (T-11)
2. 운영 문서 작성 및 커밋 (T-12)

## 7. 예상 작업 시간

| 태스크 | 내용 | 담당 에이전트 | 소요 시간 | 의존성 |
|--------|------|--------------|-----------|--------|
| T-1 | Enact 프로젝트 생성 | backend-dev | 15분 | 없음 |
| T-2 | 디렉토리 구조 구성 | backend-dev | 10분 | T-1 |
| T-3 | Hello World 구현 | frontend-dev | 20분 | T-1 |
| T-4 | 전역 스타일 설정 | frontend-dev | 20분 | T-3 |
| T-5 | ESLint 설정 | backend-dev | 15분 | T-1 |
| T-6 | Prettier 설정 | backend-dev | 10분 | T-1 |
| T-7 | webOS 메타데이터 설정 | backend-dev | 10분 | T-1 |
| T-8 | .gitignore, README | doc-writer | 15분 | T-1 |
| T-9 | 로컬 테스트 | test-runner | 15분 | T-2~T-8 |
| T-10 | 프로젝터 배포 테스트 | test-runner | 20분 (선택) | T-9 |
| T-11 | 코드+문서 리뷰 | code-reviewer | 15분 | T-9 |
| T-12 | 운영 문서+커밋 | doc-writer | 20분 | T-11 |

**총 소요 시간**: 165분 (2시간 45분) + 선택(20분) = **2~3시간**

## 8. 리스크 및 대응책

### 리스크 1: Enact CLI 버전 호환성
- **설명**: Enact CLI 버전이 맞지 않으면 빌드 실패 가능
- **영향도**: High (프로젝트 생성 불가)
- **대응 방안**:
  - Enact CLI 4.x 이상 설치 (`npm install -g @enact/cli@^4.0.0`)
  - 공식 문서 참조: https://enactjs.com/docs/
  - 설치 후 버전 확인: `enact --version`

### 리스크 2: webOS 개발자 모드 설정 실패 (T-10)
- **설명**: 프로젝터 개발자 모드 활성화 및 ares-setup-device 실패 가능
- **영향도**: Medium (로컬 테스트로 대체 가능)
- **대응 방안**:
  - T-10은 선택 사항으로 설정 (로컬 테스트만으로 F-01 완료 가능)
  - 프로젝터 배포 테스트는 F-02 이후 통합 테스트 시점에 진행
  - webOS CLI 공식 문서 참조: https://webostv.developer.lge.com/develop/tools/cli-installation

### 리스크 3: Spotlight 포커스 버그
- **설명**: 리모컨 포커스 이동이 예상과 다르게 동작 가능
- **영향도**: Low (F-01은 단일 버튼만 있어 포커스 경로 단순)
- **대응 방안**:
  - Moonstone Button 컴포넌트 사용 (Spotlightable 기본 적용)
  - Spotlight.setDefaultDirections() 사용은 F-04 이후 적용

### 리스크 4: CSS Modules 미적용
- **설명**: CSS Modules 설정이 누락되면 스타일 캡슐화 실패
- **영향도**: Low (Enact CLI가 자동 설정)
- **대응 방안**:
  - Enact CLI가 webpack.config.js에 less-loader + CSS Modules 자동 설정
  - App.module.less 파일명 확인 (`.module.less` 확장자 필수)

### 리스크 5: PropTypes 누락
- **설명**: PropTypes 검증 누락 시 런타임 에러 발생 가능
- **영향도**: Low (F-01은 Props가 없음)
- **대응 방안**:
  - ESLint 규칙 `react/prop-types: error`로 자동 검사
  - T-11 리뷰 단계에서 PropTypes 누락 확인

## 9. 완료 체크리스트

### 프로젝트 생성 (T-1)
- [ ] `enact create webosbrowser` 실행 완료
- [ ] `package.json`에 `@enact/core`, `@enact/moonstone` 포함
- [ ] `npm install` 실행 시 에러 없음

### 디렉토리 구조 (T-2)
- [ ] `src/components/`, `src/views/`, `src/services/`, `src/hooks/`, `src/utils/` 생성
- [ ] 각 디렉토리에 `.gitkeep` 파일 추가
- [ ] `src/utils/logger.js` 생성 (info, warn, error 함수 포함)

### Hello World 구현 (T-3)
- [ ] `src/App/App.js`에 "Hello webOS Browser" 텍스트 렌더링
- [ ] Moonstone Panel, Header 사용 확인
- [ ] "시작" 버튼 추가 (클릭 시 콘솔 로그 출력)

### 전역 스타일 (T-4)
- [ ] `src/App/App.module.less`에 CSS 변수 10개 이상 정의
- [ ] 최소 폰트 크기 24px 적용
- [ ] 포커스 스타일 정의 (--focus-border-color, --focus-border-width)

### ESLint 설정 (T-5)
- [ ] `.eslintrc.js` 생성 (React, Enact 규칙 포함)
- [ ] package.json에 lint, lint:fix 스크립트 추가
- [ ] `npm run lint` 실행 시 에러 0개

### Prettier 설정 (T-6)
- [ ] `.prettierrc` 생성 (useTabs, semi: false 등)
- [ ] package.json에 format, format:check 스크립트 추가
- [ ] `npm run format:check` 실행 시 포맷 오류 0개

### webOS 메타데이터 (T-7)
- [ ] `webos-meta/appinfo.json` 수정 (id, version, title 확인)
- [ ] `webos-meta/icon.png`, `largeIcon.png` 존재 (placeholder 허용)

### 문서 업데이트 (T-8)
- [ ] `.gitignore`에 `dist/`, `node_modules/`, `.worktrees/` 추가
- [ ] `README.md` 생성 (프로젝트 소개, 실행 방법 포함)

### 로컬 테스트 (T-9)
- [ ] `npm run serve` 실행 시 http://localhost:8080 서버 구동
- [ ] 브라우저에서 "Hello webOS Browser" 화면 표시
- [ ] "시작" 버튼 클릭 시 콘솔 로그 출력
- [ ] 탭 키로 버튼 포커스 이동 확인

### 프로젝터 배포 테스트 (T-10, 선택)
- [ ] `npm run pack-p` 실행 시 빌드 성공
- [ ] `ares-package dist/` 실행 시 IPK 생성
- [ ] `ares-install --device projector {ipk}` 실행 시 설치 성공
- [ ] 프로젝터에서 "Hello webOS Browser" 화면 표시

### 코드 및 문서 리뷰 (T-11)
- [ ] ESLint 규칙 준수 확인 (에러 0개)
- [ ] PropTypes 검증 누락 없음
- [ ] CSS Modules 사용 확인 (인라인 스타일 금지)
- [ ] requirements.md ↔ 구현 일치 확인
- [ ] design.md ↔ 구현 일치 확인

### 운영 문서 및 커밋 (T-12)
- [ ] `docs/dev-log.md` 업데이트 (F-01 진행 기록)
- [ ] `CHANGELOG.md` 업데이트 (0.1.0 버전)
- [ ] Git 커밋 4건 (Conventional Commits 형식)
- [ ] features.md에서 F-01 상태 "In Progress" → "Done"

## 10. 다음 단계 (F-02 이후)

### F-02: 웹뷰 통합
- T-3에서 구현한 Hello World를 BrowserView로 교체
- webOS webview API 통합 (`webOSSystem.loadURI()`)
- `src/views/BrowserView.js` 생성

### F-03: URL 입력 UI
- `src/components/URLBar/URLBar.js` 구현
- T-2에서 생성한 `src/utils/` 활용 (URL 검증)

### F-04: 페이지 탐색 컨트롤
- `src/components/NavigationBar/NavigationBar.js` 구현
- T-2에서 생성한 `src/hooks/` 활용 (리모컨 Hook)

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-12 | 최초 작성 | F-01 요구사항 분석서 및 기술 설계서 기반 작성 |
