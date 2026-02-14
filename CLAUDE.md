# CLAUDE.md — webOS Browser Native 프로젝트

# 프로젝트: webOS Browser Native

## 프로젝트 개요

LG 프로젝터 HU715QW용 웹 브라우저 Native App. C++/Qt 기반으로 webOS WebView API를 사용하여 실제 외부 웹사이트 렌더링을 구현합니다. Web App PoC(webosbrowser)의 설계 문서를 기반으로 하며, 리모컨 최적화 UX를 제공합니다.

**전신 프로젝트**: [webosbrowser](https://github.com/jsong1230/webosbrowser) (React/Enact Web App PoC)

---

## 기술 스택

- **플랫폼**: webOS (LG 프로젝터 HU715QW)
- **언어**: C++ (C++17)
- **GUI 프레임워크**: Qt 5.15+
- **웹 렌더링**: webOS WebView API (Native)
- **로컬 저장소**: webOS LS2 API (북마크, 히스토리, 설정)
- **빌드 시스템**: CMake 3.16+
- **패키징**: webOS Native CLI
- **테스트**: Google Test (단위), Qt Test (통합)
- **배포**: webOS Developer Mode (개발), LG Content Store (프로덕션)

---

## 디렉토리 구조

```
webosbrowser-native/
├── src/                        # C++ 소스 코드
│   ├── main.cpp                # 진입점
│   ├── browser/                # 브라우저 코어
│   │   ├── BrowserWindow.cpp   # 메인 윈도우
│   │   ├── BrowserWindow.h
│   │   ├── WebView.cpp         # WebView 래퍼
│   │   ├── WebView.h
│   │   ├── TabManager.cpp      # 탭 관리
│   │   └── TabManager.h
│   ├── ui/                     # UI 컴포넌트
│   │   ├── URLBar.cpp          # URL 입력
│   │   ├── URLBar.h
│   │   ├── NavigationBar.cpp   # 네비게이션
│   │   ├── NavigationBar.h
│   │   ├── BookmarkPanel.cpp   # 북마크 패널
│   │   └── BookmarkPanel.h
│   ├── services/               # 비즈니스 로직
│   │   ├── BookmarkService.cpp
│   │   ├── BookmarkService.h
│   │   ├── HistoryService.cpp
│   │   ├── HistoryService.h
│   │   ├── StorageService.cpp  # LS2 API 래퍼
│   │   └── StorageService.h
│   └── utils/                  # 유틸리티
│       ├── Logger.cpp
│       ├── Logger.h
│       ├── URLValidator.cpp
│       └── URLValidator.h
├── include/                    # 공개 헤더
├── resources/                  # 리소스
│   ├── icons/
│   └── qml/                    # QML UI (선택사항)
├── webos-meta/                 # webOS 메타데이터
│   ├── appinfo.json
│   ├── icon.png
│   └── largeIcon.png
├── tests/                      # 테스트
│   ├── unit/
│   └── integration/
├── docs/                       # 설계 문서 (Web App에서 복사)
│   ├── project/
│   │   ├── prd.md
│   │   ├── features.md
│   │   └── roadmap.md
│   └── specs/                  # 15개 기능 설계서
├── CMakeLists.txt              # CMake 빌드 설정
├── README.md
├── GETTING_STARTED.md
└── .claude/                    # Claude Code 설정
    ├── agents/
    ├── commands/
    └── scripts/
```

---

## 코딩 컨벤션

### 일반
- 주석 언어: 한국어
- 변수명 언어: 영어 (camelCase)
- 클래스명: PascalCase
- 들여쓰기: 스페이스 4칸
- 세미콜론: 필수

### C++ 스타일
- 표준: C++17
- 헤더 가드: `#pragma once` 사용
- 포인터: 스마트 포인터 우선 (`std::unique_ptr`, `std::shared_ptr`)
- 문자열: `QString` (Qt 프레임워크)
- 컨테이너: Qt 컨테이너 우선 (`QVector`, `QMap`, `QList`)
- 네임스페이스: `namespace webosbrowser { }`

### Qt 스타일
- 시그널/슬롯 사용 (Qt5 신규 문법)
- `Q_OBJECT` 매크로 필수 (시그널/슬롯 사용 클래스)
- `QObject` 상속 시 복사 생성자 삭제
- UI는 Qt Widgets 또는 QML 사용

### 파일 명명
- 소스: `ClassName.cpp`
- 헤더: `ClassName.h`
- 테스트: `ClassNameTest.cpp`

### 공통 금지 사항
- `new`/`delete` 직접 사용 금지 (스마트 포인터 사용)
- 전역 변수 사용 금지
- `goto` 사용 금지
- Raw 포인터 반환 금지 (참조 또는 스마트 포인터)

---

## Git 규칙

### 커밋 메시지
- 형식: Conventional Commits
  - `feat:` 새 기능 / `fix:` 버그 수정 / `refactor:` 리팩토링
  - `test:` 테스트 / `docs:` 문서 / `chore:` 기타
  - `build:` 빌드 시스템 / `perf:` 성능 개선
- 본문 언어: 한국어
- 예시: `feat: WebView 컴포넌트 구현`
- 하나의 논리적 단위 = 하나의 커밋

### 브랜치 전략
- 메인 브랜치: `main`
- 작업 브랜치: `feature/{기능명}`, `fix/{이슈명}`
- 병렬 작업 시: `feature/{기능명}-{컴포넌트}`

---

## 빌드 및 실행

### 개발 환경 설정
```bash
# Qt 설치
brew install qt@5

# CMake 설치
brew install cmake

# webOS Native SDK 설치
# LG 공식 사이트에서 다운로드
```

### 빌드
```bash
mkdir build && cd build
cmake ..
make
```

### webOS 프로젝터 배포
```bash
# IPK 생성
ares-package build/

# 설치
ares-install --device projector {생성된ipk파일}

# 실행
ares-launch --device projector com.jsong.webosbrowser.native
```

---

## 에이전트 라우팅 규칙

### 프로젝트 기획
| 단계 | 에이전트 | 작성하는 문서 |
|------|----------|-------------|
| 프로젝트 기획 | `project-planner` | `docs/project/prd.md`, `features.md`, `roadmap.md` |

### 서브에이전트 (순차 작업)
| 단계 | 에이전트 | 작성하는 문서 |
|------|----------|-------------|
| 요구사항 분석 | `product-manager` | `docs/specs/{기능}/requirements.md` |
| 기술 설계 | `architect` | `docs/specs/{기능}/design.md` |
| 구현 계획 | `product-manager` | `docs/specs/{기능}/plan.md` |
| C++ 구현 | **cpp-dev** (필요시 생성) | 소스 코드 + 헤더 |
| 테스트 | `test-runner` | — |
| 코드 + 문서 리뷰 | `code-reviewer` | — (검증만) |
| 진행 로그 / CHANGELOG | `doc-writer` | `docs/dev-log.md`, `CHANGELOG.md` |

**주의**: 이 프로젝트는 C++/Qt 기반이므로 `frontend-dev`, `backend-dev` 대신 C++ 개발 에이전트를 사용합니다.

---

## 파이프라인 실행 가이드

### 자동 연속 개발 (권장)
```
/auto-dev
```
features.md를 읽어 자동 연속 개발.

### 수동 기능 선택
```
/next-feature
```

### 순차 파이프라인
```
/fullstack-feature {기능 설명}
```

---

## 프로젝트 특이사항

### webOS Native App 제약사항
- **리모컨 입력 최적화**: Qt 키 이벤트 처리
- **WebView API**: webOS 전용 API 사용 (표준 Qt WebEngine 아님)
- **LS2 API**: 비동기 메시지 버스, JSON 기반
- **메모리 제약**: 프로젝터 하드웨어 고려

### 보안 요구사항
- HTTPS 사이트 우선
- 안전하지 않은 사이트 경고
- 쿠키/캐시 삭제 기능

---

## 참고 자료

### 공식 문서
- webOS Native API: https://webostv.developer.lge.com/develop/native-apps/native-app-overview
- Qt for webOS: https://webostv.developer.lge.com/develop/native-apps/webos-qt-overview
- Qt 5 Documentation: https://doc.qt.io/qt-5/

### 프로젝트 문서
- 개발 환경 설정: `GETTING_STARTED.md`
- Web App PoC: https://github.com/jsong1230/webosbrowser
- 설계 문서: `docs/specs/`
