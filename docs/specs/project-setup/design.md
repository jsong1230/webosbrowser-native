# 프로젝트 초기 설정 — 기술 설계서

## 1. 참조
- 요구사항 분석서: `docs/specs/project-setup/requirements.md`
- PRD: `docs/project/prd.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser/CLAUDE.md`

## 2. 아키텍처 결정

### 결정 1: Enact CLI 버전 및 템플릿
- **선택지**:
  - A) Enact CLI 4.x + Moonstone 템플릿 (webOS TV 최적화)
  - B) Enact CLI 5.x + Sandstone 템플릿 (신규 버전)
  - C) 수동 React 프로젝트 + Enact 라이브러리 설치
- **결정**: A) Enact CLI 4.x + Moonstone 템플릿
- **근거**:
  - LG 프로젝터 HU175QW는 webOS 4.x 기반으로 Moonstone이 공식 지원
  - Moonstone UI 컴포넌트가 리모컨 조작에 최적화됨 (Spotlight 시스템)
  - Enact CLI가 webOS 빌드 설정(webpack, babel)을 자동 구성
- **트레이드오프**: Sandstone(신규 디자인 시스템)을 포기하지만, 안정성과 호환성 확보

### 결정 2: 라우팅 메커니즘
- **선택지**:
  - A) React Router (SPA 라우팅)
  - B) Enact Router (@enact/router)
  - C) 상태 기반 화면 전환 (useState + 조건부 렌더링)
- **결정**: C) 상태 기반 화면 전환 (초기), B) Enact Router (F-06 탭 관리 시 마이그레이션)
- **근거**:
  - F-01 단계에서는 단일 화면만 렌더링하므로 라우터 오버헤드 불필요
  - 상태 기반 전환이 단순하고 디버깅 용이
  - F-06(탭 관리) 시점에 Enact Router 도입으로 복잡한 화면 전환 대응
- **트레이드오프**: 초기 단순성 확보, 추후 마이그레이션 비용 발생하지만 F-01의 범위를 최소화

### 결정 3: 전역 스타일 관리
- **선택지**:
  - A) CSS Modules만 사용 (컴포넌트별 스타일)
  - B) CSS Modules + CSS 변수 (테마 관리)
  - C) Styled Components (CSS-in-JS)
- **결정**: B) CSS Modules + CSS 변수
- **근거**:
  - Enact 공식 패턴은 CSS Modules(`.module.less`)
  - CSS 변수로 전역 테마(색상, 폰트 크기)를 중앙 관리
  - 대화면 가독성을 위한 폰트 크기 조정이 CSS 변수로 유연하게 가능
- **트레이드오프**: CSS-in-JS의 동적 스타일링은 포기하지만, 리모컨 UX에서는 정적 스타일이 더 적합

### 결정 4: 디렉토리 구조
- **선택지**:
  - A) 플랫 구조 (모든 컴포넌트를 src/components/에 배치)
  - B) 레이어별 구조 (components, views, services, hooks, utils 분리)
  - C) 기능별 구조 (feature/bookmark/, feature/history/)
- **결정**: B) 레이어별 구조
- **근거**:
  - 컴포넌트(재사용), 뷰(화면), 서비스(비즈니스 로직)를 명확히 분리하여 유지보수성 향상
  - 프론트엔드 전용 프로젝트이므로 레이어별 구조가 직관적
  - 15개 기능을 수용할 수 있는 확장성 확보
- **트레이드오프**: 기능별 구조의 응집도는 포기하지만, 초기 설정에서는 레이어 분리가 더 명확

### 결정 5: 코드 품질 도구
- **선택지**:
  - A) ESLint만 사용
  - B) ESLint + Prettier
  - C) ESLint + Prettier + Husky (pre-commit hook)
- **결정**: B) ESLint + Prettier
- **근거**:
  - ESLint는 React/Enact 특화 규칙으로 코드 품질 유지
  - Prettier는 팀 코드 스타일 통일 (탭, 세미콜론 없음, 싱글 쿼터)
  - Husky는 F-01 범위 외, 추후 CI/CD 설정 시 추가
- **트레이드오프**: pre-commit hook 없이 수동 린트 실행, 하지만 초기 설정 복잡도 감소

## 3. 기술 스택 및 의존성

### 핵심 프레임워크
- **Enact 4.x**: React 기반 webOS UI 프레임워크
- **Moonstone**: Enact의 리모컨 최적화 UI 컴포넌트 라이브러리
- **React 17.x**: Enact 4.x가 의존하는 React 버전

### 빌드 도구
- **Enact CLI**: webpack, babel, less-loader 자동 설정
- **webpack 5**: 번들러 (Enact CLI 내장)
- **Babel**: ES6+ → ES5 트랜스파일 (Enact CLI 내장)
- **LESS**: CSS 프리프로세서 (Enact 기본)

### 코드 품질 도구
- **ESLint**: JavaScript 린터 (eslint-plugin-enact, eslint-plugin-react-hooks)
- **Prettier**: 코드 포맷터

### webOS 도구
- **@webos-tools/cli**: webOS 앱 패키징, 배포 도구
- **ares-***: 프로젝터 설치, 실행, 디버깅 명령어

## 4. 디렉토리 구조 설계

### 최종 구조
```
webosbrowser/
├── src/
│   ├── App/
│   │   ├── App.js                  # 메인 앱 컴포넌트 (라우팅 로직)
│   │   ├── App.module.less         # 전역 스타일 + CSS 변수
│   │   └── package.json            # Enact 메타데이터
│   ├── components/                 # 재사용 가능한 UI 컴포넌트
│   │   └── .gitkeep                # 빈 디렉토리 커밋용
│   ├── views/                      # 화면 단위 컴포넌트
│   │   ├── BrowserView.js          # 메인 브라우저 화면 (F-02에서 구현)
│   │   └── .gitkeep
│   ├── services/                   # 비즈니스 로직 (북마크, 히스토리 등)
│   │   └── .gitkeep
│   ├── hooks/                      # 커스텀 React Hooks
│   │   └── .gitkeep
│   └── utils/                      # 유틸리티 함수 (logger, validator)
│       └── logger.js               # 로깅 유틸 (console.log 대체)
├── webos-meta/
│   ├── appinfo.json                # webOS 앱 메타데이터
│   ├── icon.png                    # 80x80 아이콘
│   └── largeIcon.png               # 130x130 아이콘
├── resources/                      # 정적 리소스 (이미지, 폰트)
├── dist/                           # 빌드 결과물 (gitignore)
├── docs/                           # 프로젝트 문서 (기존 유지)
├── .eslintrc.js                    # ESLint 설정
├── .prettierrc                     # Prettier 설정
├── .gitignore                      # Git 제외 파일
├── package.json                    # npm 설정
└── README.md                       # 프로젝트 소개
```

### 디렉토리 역할 설명

#### `src/App/`
- 앱 진입점 (App.js)
- 전역 스타일 (App.module.less)
- Enact 앱 메타데이터 (package.json)

#### `src/components/`
- 재사용 가능한 UI 컴포넌트
- 예: URLBar, NavigationBar, LoadingBar, TabBar (F-03~F-06에서 구현)
- 각 컴포넌트는 디렉토리로 구성 (예: `components/URLBar/URLBar.js`, `URLBar.module.less`)

#### `src/views/`
- 화면 단위 컴포넌트 (페이지 레벨)
- 예: BrowserView (메인), ErrorPage (에러), HomePage (홈)
- components를 조합하여 화면 구성

#### `src/services/`
- 비즈니스 로직 (북마크, 히스토리, 로컬 저장소)
- 예: bookmarkService.js, historyService.js, storageService.js (F-07, F-08에서 구현)
- LS2 API 래퍼

#### `src/hooks/`
- 커스텀 React Hooks
- 예: useRemoteControl.js, useWebView.js (F-02, F-04에서 구현)

#### `src/utils/`
- 범용 유틸리티 함수
- logger.js: console.log 대체 로깅 (웹뷰 디버깅용)
- urlValidator.js: URL 유효성 검사 (F-03에서 구현)

#### `webos-meta/`
- webOS 앱 메타데이터
- appinfo.json: 앱 ID, 버전, 제목, 진입점 정의
- 아이콘 파일 (80x80, 130x130)

## 5. App.js 설계 (라우팅 로직)

### 초기 구조 (F-01)
```javascript
// src/App/App.js
import { useState } from 'react'
import { Panel, Header } from '@enact/moonstone/Panel'
import Spotlight from '@enact/spotlight'
import css from './App.module.less'

const App = () => {
	// 상태 기반 화면 전환 (F-01 단계에서는 Hello World만)
	const [currentView, setCurrentView] = useState('hello')

	return (
		<Panel className={css.app}>
			<Header title="webOS Browser" />
			{currentView === 'hello' && (
				<div className={css.helloContainer}>
					<h1 className={css.title}>Hello webOS Browser</h1>
					<button
						onClick={() => console.log('시작 버튼 클릭')}
						className={css.startButton}
					>
						시작
					</button>
				</div>
			)}
		</Panel>
	)
}

export default App
```

### Hello World 컴포넌트 설계
- **목적**: 프로젝트 실행 확인용 최소 화면
- **구성**:
  - Moonstone Panel로 전체 화면 감싸기
  - Header에 앱 제목 표시
  - 중앙에 "Hello webOS Browser" 텍스트 (폰트 크기 36px)
  - Spotlightable한 "시작" 버튼 1개 (리모컨 포커스 테스트용)
- **포커스 동작**:
  - 앱 실행 시 "시작" 버튼에 자동 포커스
  - 리모컨 선택 버튼 클릭 시 콘솔 로그 출력

## 6. 전역 스타일 설계

### CSS 변수 정의 (App.module.less)
```less
// src/App/App.module.less
:root {
	// 폰트 크기 (대화면 가독성)
	--font-size-min: 24px;       // 최소 폰트 크기 (본문)
	--font-size-title: 36px;     // 제목 폰트 크기
	--font-size-button: 28px;    // 버튼 폰트 크기

	// 색상 (다크 테마)
	--bg-color: #1e1e1e;         // 배경색
	--text-color: #ffffff;       // 텍스트 색상
	--text-secondary: #b0b0b0;   // 보조 텍스트

	// 포커스 스타일 (Spotlight)
	--focus-border-color: #00aaff; // 포커스 테두리 색상
	--focus-border-width: 4px;     // 포커스 테두리 두께
	--focus-bg-color: rgba(0, 170, 255, 0.1); // 포커스 배경

	// 간격
	--spacing-xs: 8px;
	--spacing-sm: 16px;
	--spacing-md: 24px;
	--spacing-lg: 32px;
	--spacing-xl: 48px;
}

.app {
	background-color: var(--bg-color);
	color: var(--text-color);
	font-size: var(--font-size-min);
	height: 100vh;
	width: 100vw;
}

.helloContainer {
	display: flex;
	flex-direction: column;
	align-items: center;
	justify-content: center;
	height: 100%;
}

.title {
	font-size: var(--font-size-title);
	margin-bottom: var(--spacing-lg);
}

.startButton {
	font-size: var(--font-size-button);
	padding: var(--spacing-sm) var(--spacing-lg);
	border: var(--focus-border-width) solid transparent;
	background-color: #333;
	color: var(--text-color);
	cursor: pointer;
	transition: all 0.2s ease;

	&:focus {
		border-color: var(--focus-border-color);
		background-color: var(--focus-bg-color);
		outline: none;
	}
}
```

### Moonstone 테마 커스터마이징
- Moonstone 기본 스타일 유지 (Panel, Header, Button 등)
- 전역 CSS 변수로 색상, 폰트 크기만 재정의
- Spotlight 포커스 스타일은 Moonstone 기본 사용 (--focus-* 변수는 커스텀 컴포넌트용)

## 7. ESLint 및 Prettier 설정

### .eslintrc.js
```javascript
module.exports = {
	extends: [
		'eslint:recommended',
		'plugin:react/recommended',
		'plugin:react-hooks/recommended',
		'plugin:enact/recommended'
	],
	plugins: ['react', 'react-hooks', 'enact'],
	parserOptions: {
		ecmaVersion: 2020,
		sourceType: 'module',
		ecmaFeatures: {
			jsx: true
		}
	},
	env: {
		browser: true,
		es6: true,
		node: true
	},
	rules: {
		// PropTypes 검증 필수
		'react/prop-types': 'error',

		// console.log 경고 (logger.js 사용 권장)
		'no-console': 'warn',

		// React Hooks 규칙
		'react-hooks/rules-of-hooks': 'error',
		'react-hooks/exhaustive-deps': 'warn',

		// Enact 규칙
		'enact/display-name': 'warn',
		'enact/kind-name': 'warn',

		// 세미콜론 없음 (Enact 스타일)
		'semi': ['error', 'never']
	},
	settings: {
		react: {
			version: 'detect'
		}
	}
}
```

### .prettierrc
```json
{
	"useTabs": true,
	"tabWidth": 4,
	"semi": false,
	"singleQuote": true,
	"trailingComma": "none",
	"printWidth": 100,
	"arrowParens": "avoid"
}
```

### package.json 스크립트 추가
```json
{
	"scripts": {
		"lint": "eslint src/ --ext .js,.jsx",
		"lint:fix": "eslint src/ --ext .js,.jsx --fix",
		"format": "prettier --write \"src/**/*.{js,jsx,less}\"",
		"format:check": "prettier --check \"src/**/*.{js,jsx,less}\""
	}
}
```

## 8. webOS 메타데이터 설계

### webos-meta/appinfo.json
```json
{
	"id": "com.jsong.webosbrowser",
	"version": "0.1.0",
	"vendor": "jsong",
	"type": "web",
	"main": "index.html",
	"title": "webOS Browser",
	"icon": "icon.png",
	"largeIcon": "largeIcon.png",
	"resolution": "1920x1080",
	"bgImage": "",
	"transparent": false
}
```

### 아이콘 준비
- icon.png (80x80): 앱 런처 아이콘
- largeIcon.png (130x130): 설정 화면 아이콘
- 초기 단계에서는 Enact 기본 아이콘 사용 또는 단색 플레이스홀더

## 9. 구현 순서

### Phase 1: Enact 프로젝트 생성
1. Enact CLI 설치: `npm install -g @enact/cli`
2. 프로젝트 생성: `enact create webosbrowser`
3. 생성 확인: `cd webosbrowser && npm install`

### Phase 2: 디렉토리 구조 구성
1. `src/` 하위에 components, views, services, hooks, utils 디렉토리 생성
2. 각 디렉토리에 .gitkeep 파일 추가
3. `src/utils/logger.js` 생성 (console.log 래퍼)

### Phase 3: App.js Hello World 구현
1. `src/App/App.js` 수정 (Hello World 화면)
2. `src/App/App.module.less`에 전역 CSS 변수 정의
3. Moonstone Panel, Header 사용
4. 리모컨 포커스 가능한 "시작" 버튼 추가

### Phase 4: 코드 품질 도구 설정
1. `.eslintrc.js` 생성 (React, Enact 규칙)
2. `.prettierrc` 생성 (탭, 세미콜론 없음)
3. package.json에 lint, format 스크립트 추가
4. `npm run lint` 실행하여 에러 없음 확인

### Phase 5: webOS 메타데이터 설정
1. `webos-meta/appinfo.json` 수정 (앱 ID, 제목, 버전)
2. 아이콘 파일 준비 (placeholder)

### Phase 6: 로컬 테스트
1. `npm run serve` 실행 (http://localhost:8080)
2. 브라우저에서 "Hello webOS Browser" 화면 확인
3. 탭 키로 "시작" 버튼 포커스 확인
4. 버튼 클릭 시 콘솔 로그 출력 확인

### Phase 7: 빌드 및 배포 테스트 (선택)
1. `npm run pack-p` 실행 (프로덕션 빌드)
2. `ares-package dist/` 실행 (IPK 생성)
3. `ares-install --device projector {ipk파일}` 실행 (프로젝터 설치)
4. 프로젝터에서 앱 실행 확인

### Phase 8: 문서 업데이트
1. .gitignore 업데이트 (dist/, node_modules/, .worktrees/ 추가)
2. CLAUDE.md에 디렉토리 구조 반영 (이미 완료)
3. README.md 생성 (프로젝트 소개, 실행 방법)

## 10. 기술적 주의사항

### webOS 플랫폼 제약
- **Spotlight 시스템**: 모든 인터랙티브 요소는 Spotlightable 속성이 필요
- **리모컨 전용**: 마우스 이벤트 대신 키보드 이벤트로 조작
- **메모리 제약**: 프로젝터 하드웨어 리소스가 제한적이므로 번들 크기 최소화
- **LS2 API**: 로컬 저장소는 webOS LS2 API를 통해 접근 (LocalStorage보다 안정적)

### Enact 프레임워크 모범 사례
- **PropTypes 필수**: 모든 컴포넌트에 PropTypes 정의
- **kind 패턴**: Enact의 Higher-Order Component 패턴 사용 (재사용성 향상)
- **Spotlight 설정**: Spotlight.setDefaultDirections()로 포커스 이동 경로 커스터마이징
- **CSS Modules**: 스타일은 `.module.less`로 캡슐화

### 보안 및 에러 처리
- **URL 검증**: F-03에서 URL 입력 시 반드시 검증 (XSS 방지)
- **에러 바운더리**: React Error Boundary로 전역 에러 처리 (F-10에서 구현)
- **로깅**: console.log 대신 logger.js 사용 (webOS 로그와 통합)

### 성능 최적화
- **코드 스플리팅**: React.lazy()로 컴포넌트 지연 로딩 (F-06 이후 적용)
- **메모이제이션**: React.memo()로 불필요한 렌더링 방지
- **웹뷰 최적화**: F-02에서 웹뷰 인스턴스 재사용 (메모리 절약)

## 11. 테스트 전략

### 단위 테스트 (F-01 범위 외, F-02 이후)
- Jest + Enact Testing Library
- 컴포넌트별 렌더링 테스트
- 서비스 로직 단위 테스트

### 수동 테스트 (F-01)
- **로컬 브라우저 테스트**:
  - `npm run serve` 실행
  - 크롬 DevTools로 콘솔 로그 확인
  - 탭 키로 포커스 이동 확인

- **프로젝터 배포 테스트** (선택):
  - IPK 생성 및 설치
  - 리모컨으로 앱 실행
  - "시작" 버튼 포커스 및 클릭 동작 확인

### 린트 테스트
- `npm run lint` 실행 시 에러 0개
- `npm run format:check` 실행 시 포맷 오류 0개

## 12. 영향 범위 분석

### 수정 필요한 기존 파일
없음 (새 프로젝트 생성)

### 새로 생성할 파일
- **프로젝트 루트**:
  - `.gitignore`: dist/, node_modules/, .worktrees/
  - `.eslintrc.js`: ESLint 규칙
  - `.prettierrc`: Prettier 규칙
  - `package.json`: Enact CLI가 자동 생성 (스크립트 추가)
  - `README.md`: 프로젝트 소개

- **src/**:
  - `src/App/App.js`: 메인 앱 컴포넌트
  - `src/App/App.module.less`: 전역 스타일
  - `src/components/.gitkeep`
  - `src/views/.gitkeep`
  - `src/services/.gitkeep`
  - `src/hooks/.gitkeep`
  - `src/utils/logger.js`: 로깅 유틸

- **webos-meta/**:
  - `webos-meta/appinfo.json`: webOS 메타데이터
  - `webos-meta/icon.png`: 80x80 아이콘
  - `webos-meta/largeIcon.png`: 130x130 아이콘

### 영향 받는 기존 기능
없음 (F-01이 모든 후속 기능의 의존성 시작점)

## 13. 리스크 및 완화 전략

### 리스크 1: Enact CLI 버전 호환성
- **설명**: Enact CLI 버전이 맞지 않으면 빌드 실패 가능
- **완화**: package.json에 정확한 버전 명시, 공식 문서 참조

### 리스크 2: webOS 개발자 모드 설정
- **설명**: 프로젝터 개발자 모드 활성화 및 ares-setup-device 실패 가능
- **완화**: docs/development-guide.md에 상세한 설정 가이드 작성

### 리스크 3: Spotlight 포커스 버그
- **설명**: 리모컨 포커스 이동이 예상과 다르게 동작 가능
- **완화**: Spotlight.setDefaultDirections()로 명시적 포커스 경로 설정

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-12 | 최초 작성 | F-01 요구사항 기반 기술 설계 |
