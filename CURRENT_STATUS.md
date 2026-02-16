# webOS Browser Native - 현재 상황 및 기술적 제약

## 프로젝트 목표
LG HU715QW 프로젝터에서 작동하는 웹 브라우저 Native App 개발

## 개발 현황 (2026-02-16)

### ✅ 완료된 작업

**1. 전체 UI 프레임워크 (v1.0.0)**
- 15개 기능 완전 구현
- URL 입력, 네비게이션, 북마크, 히스토리, 설정 등
- 리모컨 최적화 UI
- 완전한 설계 문서 (`docs/specs/`)

**2. WebView 구현 시도**
- ✅ WebView_stub.cpp: 시각화 스텁 (Mac 개발용)
- ✅ WebView_lunasvc.cpp: Luna Service 기반 (시스템 브라우저 호출)
- ❌ WebView.cpp: Qt WebEngine 버전 (프로젝터에 WebEngine 없음)

### ⚠️ 기술적 제약사항

**1. webOS TV 플랫폼의 한계**

| 항목 | 상태 | 설명 |
|------|------|------|
| 공식 Native SDK | ❌ 없음 | LG는 Web App만 공식 지원 |
| Qt WebEngine | ❌ 없음 | 프로젝터에 WebEngine 미설치 |
| Native App Shell | ❌ 크래시 | `type: "native_appshell"` 시도 시 리부팅 |
| Luna Service API | ⚠️ 제한적 | 시스템 브라우저를 별도 앱으로 실행 |
| 크로스 컴파일 | ⚠️ 복잡 | ARMv7 타겟, 툴체인 필요 |

**2. 프로젝터 환경**
- 모델: LG HU715QW-GL
- OS: webOS 6.3.1
- CPU: ARMv7 (32-bit ARM)
- Qt: 5.12.3 (WebEngine/WebKit/WebView 없음)
- 빌드 도구: 없음 (cmake, gcc 등 미설치)

**3. 시도한 방법들**

| 방법 | 결과 | 문제점 |
|------|------|--------|
| Web App (Enact) + `<iframe>` | ❌ | CORS, Same-Origin Policy |
| Web App (Enact) + `<webview>` | ❌ | Web App 샌드박스 제약 |
| Native App Shell + manifest.json | ❌ | 프로젝터 크래시 및 리부팅 |
| Qt WebEngine | ❌ | 프로젝터에 없음 |
| Luna Service API | ⚠️ | 시스템 브라우저 호출 (통합 불가) |
| 프로젝터 직접 빌드 | ❌ | 빌드 도구 없음 |
| ARM 크로스 컴파일 | ⚠️ | SDK/툴체인 필요 |

## 실행 가능한 옵션

### 옵션 A: 제한된 기능으로 완성 ⭐
**Luna Service 버전 사용**
- 시스템 브라우저를 호출하는 런처 앱
- URL 입력, 북마크, 히스토리 관리는 자체 구현
- 실제 웹 렌더링은 시스템 브라우저 위임
- **장점**: 실제로 작동함
- **단점**: 통합 브라우저 아님

### 옵션 B: 참고 프로젝트로 전환
**UI 프레임워크 완성본**
- 15개 기능 설계 및 구현 완료
- webOS Native App 아키텍처 참고 자료
- Qt/C++ 코드베이스
- **장점**: 학습/참고 가치
- **단점**: 실제 웹 렌더링 불가

### 옵션 C: 커뮤니티 NDK 사용
**webosbrew/meta-lg-webos-ndk**
- Ubuntu 16.04 + 200GB 환경 필요
- deprecated (2023년 7월)
- **장점**: 진짜 Native App 개발 가능
- **단점**: 매우 복잡, 시간 많이 소요

### 옵션 D: 다른 프로젝트
**LG webOS는 기본 브라우저 탑재**
- webOS용 다른 유용한 앱 개발
- 미디어 플레이어, 유틸리티, 게임 등

## 기술적 학습

### 발견한 사실들

1. **webOS TV != webOS OSE**
   - webOS OSE: 오픈소스, Native 개발 가능
   - webOS TV: 제한적, Web App 위주

2. **Native App Shell의 정체**
   - Chrome Extension 형태
   - `manifest.json` + `webview` 권한 필요
   - webOS TV에서는 불안정 (크래시 발생)

3. **Luna Service API**
   - webOS의 시스템 서비스 호출 메커니즘
   - `luna-send` 명령으로 테스트 가능
   - 제한적이지만 작동함

4. **Qt WebEngine vs WebView vs WebKit**
   - WebEngine: Chromium 기반, 무거움, 없음
   - WebView: 경량, 시스템 브라우저 사용, 없음
   - WebKit: Safari 기반, 없음

## 다음 단계 제안

### 즉시 실행 가능
1. **Luna Service 버전 문서화**
   - 제약사항 명시
   - 사용 가이드 작성
   - README 업데이트

2. **v1.0.0 릴리스**
   - UI 프레임워크 완성본
   - 참고 프로젝트로 공개
   - 기술적 제약 문서화

### 중장기 계획
1. **커뮤니티 NDK 환경 구축**
   - Ubuntu VM 준비
   - 빌드 환경 세팅
   - 실제 Native WebView 구현

2. **다른 플랫폼 고려**
   - Electron (크로스 플랫폼)
   - Android TV
   - Raspberry Pi

## 결론

webOS TV에서 완전한 통합 브라우저를 만드는 것은 **현재 LG의 정책상 사실상 불가능**합니다.

LG는 Web App 개발에 집중하고 있으며, Native App 개발은 공식 지원하지 않습니다.

**현실적인 선택:**
- Luna Service 버전으로 제한적 기능 제공
- 또는 참고 프로젝트로 완성하고 다른 방향 모색

---

© 2026 webOS Browser Native Project
기술적 제약으로 인한 개발 중단 문서
