# webOS Browser Native

> webOS Native App (C++/Qt) 기반 웹 브라우저 프로젝트

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/jsong1230/webosbrowser-native)
[![Platform](https://img.shields.io/badge/platform-webOS-green.svg)](https://www.webosose.org/)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-5.15+-green.svg)](https://www.qt.io/)
[![Status](https://img.shields.io/badge/status-completed-success.svg)](README.md)

**🎯 프로젝트 상태**: v1.0.0 완료 (UI 프레임워크 + 참고 자료)

---

## ⚠️ 중요 공지

이 프로젝트는 webOS TV 플랫폼의 기술적 제약으로 인해 **완전한 웹 브라우저 기능을 구현할 수 없음**이 확인되었습니다.

**현재 상태:**
- ✅ 완전한 UI 프레임워크 (15개 기능)
- ✅ 리모컨 최적화 인터페이스
- ✅ 상세한 설계 문서
- ⚠️ 실제 웹 렌더링 제약 (WebView API 없음)

자세한 내용은 [CURRENT_STATUS.md](CURRENT_STATUS.md)를 참조하세요.

---

## 프로젝트 정보

- **버전**: 1.0.0 (UI 프레임워크 완성)
- **기술 스택**: C++17, Qt 5.15+, CMake 3.16+
- **타겟 플랫폼**: webOS 6.x (LG HU715QW 프로젝터)
- **전신 프로젝트**: [webosbrowser](https://github.com/jsong1230/webosbrowser) (Web App PoC)
- **저장소**: https://github.com/jsong1230/webosbrowser-native

## 프로젝트 배경

### 목표
LG 프로젝터(HU715QW)에서 작동하는 완전한 웹 브라우저 Native App 개발

### 여정
1. **Web App PoC** → iframe 제약으로 한계
2. **Native App 전환** → Qt WebEngine 없음 발견
3. **다양한 시도** → 기술적 제약 확인
4. **UI 프레임워크 완성** → 참고 자료로 완성

자세한 개발 과정은 [JOURNEY.md](JOURNEY.md)를 참조하세요.

---

## 주요 기능 (v1.0.0)

### ✅ 구현 완료된 기능

**MS-1: 핵심 기능 (5개)**
- F-01: 프로젝트 초기 설정
- F-02: WebView 통합 (3가지 구현체)
- F-03: URL 입력 UI
- F-04: 페이지 탐색 컨트롤
- F-05: 로딩 인디케이터

**MS-2: 편의 기능 (5개)**
- F-06: 탭 관리 시스템
- F-07: 북마크 관리
- F-08: 히스토리 관리
- F-09: 검색 엔진 통합
- F-10: 에러 처리

**MS-3: 고급 기능 (5개)**
- F-11: 설정 화면
- F-12: 다운로드 관리 (설계)
- F-13: 리모컨 단축키
- F-14: HTTPS 보안 표시
- F-15: 즐겨찾기 홈 화면

**WebView 구현체:**
- `WebView_stub.cpp`: 시각화 스텁 (Mac 개발용)
- `WebView_lunasvc.cpp`: Luna Service 기반 (시스템 브라우저 호출)
- `WebView.cpp`: Qt WebEngine 버전 (프로젝터 미지원)

---

## 기술적 제약사항

### webOS TV 플랫폼의 한계

| 항목 | 상태 | 설명 |
|------|------|------|
| 공식 Native SDK | ❌ | LG는 Web App만 공식 지원 |
| Qt WebEngine | ❌ | 프로젝터에 미설치 |
| Native App Shell | ❌ | 크래시 발생 |
| 크로스 컴파일 | ⚠️ | ARMv7 타겟, 복잡한 환경 |

**프로젝터 환경:**
- 모델: LG HU715QW-GL
- OS: webOS 6.3.1
- CPU: ARMv7 (32-bit ARM)
- Qt: 5.12.3 (WebEngine 없음)

자세한 내용: [CURRENT_STATUS.md](CURRENT_STATUS.md)

---

## 프로젝트 구조

```
webosbrowser-native/
├── src/                    # C++ 소스 코드
│   ├── main.cpp
│   ├── browser/           # 브라우저 코어
│   │   ├── BrowserWindow.cpp
│   │   ├── TabManager.cpp
│   │   ├── WebView_stub.cpp      # 시각화 스텁
│   │   ├── WebView_lunasvc.cpp   # Luna Service
│   │   └── WebView.cpp           # Qt WebEngine (미사용)
│   ├── ui/                # UI 컴포넌트 (14개)
│   ├── services/          # 비즈니스 로직 (5개)
│   └── utils/             # 유틸리티 (5개)
├── include/               # 헤더 파일
├── resources/             # 리소스
├── webos-meta/           # webOS 메타데이터
├── docs/                 # 설계 문서
│   ├── project/          # PRD, 기능 백로그, 로드맵
│   ├── specs/            # 15개 기능 설계서
│   ├── dev-log.md        # 개발 로그
│   └── test-reports/     # 테스트 리포트
├── CMakeLists.txt        # CMake 빌드 설정
├── CURRENT_STATUS.md     # 현재 상태 및 제약사항
├── JOURNEY.md            # 개발 여정 문서
└── README.md
```

---

## 개발 환경

### 필수 도구
- **Qt 5.x**: GUI 프레임워크
- **CMake 3.16+**: 빌드 시스템
- **C++17 컴파일러**: Clang 또는 GCC
- **webOS CLI**: 배포 도구

### 설치

```bash
# Qt 설치
brew install qt@5

# CMake 설치
brew install cmake

# webOS CLI (npm)
npm install -g @webos/cli
```

---

## 빌드 및 실행

### Mac에서 개발 빌드

```bash
# 빌드 디렉토리 생성
mkdir -p build && cd build

# CMake 설정
cmake -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@5 ..

# 컴파일
make

# 실행 (WebView 스텁 모드)
./bin/webosbrowser
```

**출력 예시:**
```
webOS Browser Native App 시작...
[WebView] 스텁 구현 - 실제 웹 렌더링 기능 없음 (시각화 표시)
BrowserWindow: 생성 완료
```

### webOS 프로젝터 배포

**⚠️ 주의**: 실제 웹 렌더링은 프로젝터에서도 제약이 있습니다.

```bash
# IPK 패키지 생성
mkdir -p webos-deploy/bin
cp build/bin/webosbrowser webos-deploy/bin/
cp -r webos-meta/* webos-deploy/
ares-package webos-deploy/ --outdir ./dist

# 프로젝터 설치
ares-install --device projector dist/*.ipk

# 실행
ares-launch --device projector com.jsong.webosbrowser.native
```

---

## 문서

- **현재 상태**: [CURRENT_STATUS.md](CURRENT_STATUS.md) - 기술적 제약 및 결론
- **개발 여정**: [JOURNEY.md](JOURNEY.md) - 전체 개발 과정
- **개발 로그**: [docs/dev-log.md](docs/dev-log.md) - 상세 로그
- **변경 이력**: [CHANGELOG.md](CHANGELOG.md) - 버전별 변경사항
- **배포 가이드**: [DEPLOY_GUIDE.md](DEPLOY_GUIDE.md) - 배포 방법

**설계 문서:**
- [docs/project/prd.md](docs/project/prd.md) - 제품 요구사항
- [docs/project/features.md](docs/project/features.md) - 기능 백로그
- [docs/specs/](docs/specs/) - 15개 기능별 상세 설계

---

## 학습 가치

이 프로젝트는 실제 제품으로는 완성되지 못했지만, 다음과 같은 학습 가치가 있습니다:

### 1. webOS Native App 개발
- C++/Qt 기반 GUI 개발
- webOS 플랫폼 이해
- 리모컨 최적화 UX

### 2. 소프트웨어 아키텍처
- PIMPL 패턴
- MVC 아키텍처
- 서비스 레이어 설계

### 3. 프로젝트 관리
- 기능 분해 및 우선순위
- 마일스톤 계획
- 기술적 제약 대응

### 4. 문서화
- PRD, 설계서 작성
- API 문서
- 의사결정 기록

---

## 향후 방향

### 옵션 A: 참고 프로젝트로 활용 (현재)
- webOS Native App 학습 자료
- Qt/C++ 코드베이스 참고
- UI 프레임워크 재사용

### 옵션 B: Luna Service 버전 개발
- 시스템 브라우저 호출 런처
- 북마크/히스토리 관리
- 제한적이지만 작동하는 앱

### 옵션 C: 다른 플랫폼 포팅
- Electron (데스크톱)
- Android TV
- Raspberry Pi

---

## 라이선스

MIT License - 자유롭게 사용하세요!

---

## 감사의 말

이 프로젝트는 webOS 플랫폼의 기술적 제약으로 완전한 브라우저를 구현하지 못했지만, 개발 과정에서 많은 것을 배울 수 있었습니다.

**특별히 감사드립니다:**
- LG webOS 개발자 커뮤니티
- Qt 프레임워크 팀
- webOS OSE (Open Source Edition) 프로젝트

---

**© 2026 webOS Browser Native Project**

**Made with ❤️ for LG HU715QW Projector**

*"실패한 프로젝트가 아니라, 성공적으로 배운 프로젝트"*
