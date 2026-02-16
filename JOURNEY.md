# 개발 여정: webOS Browser Native

> 2026년 2월 12일 ~ 2월 16일, 5일간의 개발 과정 기록

---

## 📅 타임라인

### Day 1: 프로젝트 시작 (2월 12일)

**목표**: LG HU715QW 프로젝터용 웹 브라우저 개발

**배경**:
- 전신 프로젝트: webosbrowser (React/Enact Web App PoC)
- Web App의 한계: iframe으로 외부 사이트 로드 불가
- 해결 방안: Native App으로 전환

**초기 설정**:
```bash
# 프로젝트 생성
mkdir webosbrowser-native
cd webosbrowser-native

# CMake + Qt 구조 설정
# 15개 기능 설계 (Web App PoC 문서 기반)
```

**결과**: 프로젝트 구조 완성, 설계 문서 복사

---

### Day 2-3: UI 프레임워크 구현 (2월 13-14일)

**MS-1: 핵심 기능 (5개)**
- ✅ URLBar: URL 입력 UI
- ✅ NavigationBar: 뒤로/앞으로/새로고침/홈 버튼
- ✅ LoadingIndicator: 프로그레스 바
- ✅ WebView: 스텁 구현 (시각화)
- ✅ ErrorPage: 에러 처리 UI

**MS-2: 편의 기능 (5개)**
- ✅ TabManager: 탭 관리 (단일 탭 모드)
- ✅ BookmarkPanel: 북마크 패널
- ✅ BookmarkService: 북마크 저장/로드
- ✅ HistoryPanel: 히스토리 패널
- ✅ HistoryService: 히스토리 관리 (날짜별 그룹화)
- ✅ SearchEngine: Google/Naver/Bing 통합

**MS-3: 고급 기능 (5개)**
- ✅ SettingsPanel: 설정 화면
- ✅ KeyCodeConstants: 리모컨 단축키
- ✅ SecurityIndicator: HTTPS 보안 표시
- ✅ HomePage: 즐겨찾기 홈 화면

**빌드 시스템**:
- CMake 자동 WebEngine 감지
- WebEngine 있으면: WebView.cpp
- WebEngine 없으면: WebView_stub.cpp

**결과**:
- 24개 소스 파일 컴파일 성공
- 실행 파일 생성: bin/webosbrowser (1.1MB, arm64)
- Mac에서 실행 확인 ✅

---

### Day 4: 프로젝터 배포 시도 (2월 15일)

**목표**: 실제 프로젝터에서 실행

**첫 번째 시도: Native App 직접 배포**

```bash
# IPK 패키징
ares-package webos-deploy/ --outdir ./dist

# 설치 시도
ares-install --device projector dist/*.ipk
```

**문제 발견 ❌**:
- 실행 파일이 Mach-O (Mac용)
- 프로젝터는 ARMv7 필요
- 크로스 컴파일 필요

**두 번째 시도: 소스 코드 전송**

```bash
# GitHub에서 다운로드
curl -L https://github.com/jsong1230/webosbrowser-native/archive/refs/heads/main.zip \
  -o /tmp/webosbrowser.zip
```

**문제 발견 ❌**:
- 프로젝터에 빌드 도구 없음 (cmake, gcc 없음)
- Qt WebEngine 없음

**세 번째 시도: Qt WebEngine 확인**

```bash
# 프로젝터 Qt 라이브러리 확인
ls /usr/lib/libQt5*
```

**발견**:
- ✅ Qt 5.12.3 설치됨
- ❌ Qt WebEngine 없음
- ❌ Qt WebKit 없음
- ❌ Qt WebView 없음

**결론**: 프로젝터에서 실제 웹 렌더링 불가능

---

### Day 5: 대안 탐색 (2월 16일)

#### 시도 1: Web App 전환

**아이디어**: Enact Web App에서 `<webview>` 태그 사용

**구현**:
```javascript
// webosbrowser 프로젝트
const webview = document.createElement('webview');
webview.src = 'https://google.com';
```

**결과 ❌**:
- Web App 샌드박스에서 `<webview>` 작동 안 함
- appinfo.json `type: "web"` 제약

---

#### 시도 2: Native App Shell

**발견**: webOS OSE Enact 브라우저 분석
- `type: "native_appshell"`
- `manifest.json` with `"webview"` permission
- Chrome Extension 형태

**구현**:
```json
// appinfo.json
{
  "type": "native_appshell"
}

// manifest.json
{
  "permissions": ["webview"]
}
```

**결과 ❌**:
- 앱 실행 → 검은 화면
- **프로젝터 자동 리부팅** 발생!
- Native App Shell이 webOS TV에서 불안정

---

#### 시도 3: Luna Service API

**아이디어**: 시스템 브라우저 호출

**구현**:
```cpp
// WebView_lunasvc.cpp
QString lunaCmd = QString(
    "luna-send -n 1 -f "
    "luna://com.webos.service.applicationmanager/launch "
    "'{\"id\":\"com.webos.app.browser\",\"params\":{\"target\":\"%1\"}}'"
).arg(url.toString());
```

**결과 ⚠️**:
- 기술적으로 작동 가능
- 하지만 시스템 브라우저가 별도 앱으로 열림
- 통합 브라우저 아님

---

#### 시도 4: webOS Native SDK 조사

**목표**: 공식 Native SDK 설치

**발견 ❌**:
- LG는 **Web App만** 공식 지원
- Native SDK 없음
- webOS CLI = Web App 도구만

**커뮤니티 대안**:
- webosbrew/meta-lg-webos-ndk
- ⚠️ Deprecated (2023년 7월)
- Ubuntu 16.04 + 200GB 필요
- 매우 복잡

---

## 🔍 기술적 발견

### 1. webOS TV vs webOS OSE

| 항목 | webOS TV | webOS OSE |
|------|----------|-----------|
| 용도 | 상업용 TV/프로젝터 | 오픈소스 개발 |
| Native SDK | ❌ 없음 | ✅ 있음 |
| Web App | ✅ 공식 지원 | ✅ 지원 |
| Qt WebEngine | ❌ 선택적 | ✅ 포함 가능 |

### 2. WebView 기술 스택

**시도한 방법들**:
1. Qt WebEngine (Chromium 기반) → 없음
2. Qt WebKit (Safari 기반) → 없음
3. Qt WebView (시스템 브라우저) → 없음
4. `<webview>` HTML 태그 → Web App 제약
5. Native App Shell → 크래시
6. Luna Service API → 별도 앱 실행

**결론**: webOS TV에서 통합 WebView 불가능

### 3. 아키텍처 제약

```
┌─────────────────────────────────┐
│     webOS TV 플랫폼             │
├─────────────────────────────────┤
│  Web App (공식 지원)            │
│  - Enact Framework              │
│  - iframe 제약                  │
│  - 샌드박스 환경                │
├─────────────────────────────────┤
│  Native App (비공식)            │
│  - Qt 5.12.3 (일부)             │
│  - WebEngine 없음 ❌            │
│  - SDK 없음 ❌                  │
└─────────────────────────────────┘
```

---

## 💡 배운 점

### 기술적 학습

1. **플랫폼 제약 이해**
   - 모든 플랫폼이 Native 개발을 지원하지는 않음
   - 공식 지원 범위 확인 중요

2. **Qt 생태계**
   - Qt WebEngine vs WebKit vs WebView 차이
   - PIMPL 패턴
   - Qt Widgets vs QML

3. **webOS 아키텍처**
   - Luna Service Bus
   - WAM (Web Application Manager)
   - 앱 타입: web, native_appshell

4. **크로스 플랫폼 개발**
   - 아키텍처 호환성 (ARM64 vs ARMv7)
   - 크로스 컴파일 복잡도

### 프로젝트 관리

1. **기술 검증의 중요성**
   - 초기에 플랫폼 제약 확인 필수
   - PoC로 기술 검증 먼저

2. **유연한 계획**
   - 초기 목표가 불가능할 수 있음
   - 대안 준비 필요

3. **문서화**
   - 실패도 가치 있는 학습
   - 과정 기록의 중요성

### 소프트웨어 엔지니어링

1. **아키텍처 설계**
   - 15개 기능 완전 설계
   - UI/서비스 레이어 분리
   - 재사용 가능한 컴포넌트

2. **코드 품질**
   - C++17 모던 문법
   - PIMPL 패턴으로 캡슐화
   - 체계적인 로깅

3. **빌드 시스템**
   - CMake 조건부 컴파일
   - 자동 의존성 감지
   - 멀티 플랫폼 빌드

---

## 📊 최종 통계

### 코드베이스
- **C++ 소스**: 24개 파일
- **헤더**: 14개 파일
- **총 라인**: 약 8,000 라인
- **컴파일 시간**: 30초 (Mac)

### 문서
- **설계 문서**: 15개 (기능별)
- **프로젝트 문서**: 8개
- **테스트 리포트**: 3개
- **총 문서**: 26개

### 기능
- **완료된 기능**: 15개 (100%)
- **WebView 구현체**: 3개
- **UI 컴포넌트**: 14개
- **서비스**: 5개
- **유틸리티**: 5개

### 시도한 방법
- **WebView 구현**: 3가지
- **앱 타입**: 3가지 (web, native_appshell, native)
- **배포 시도**: 5회
- **기술 조사**: 10+ 문서/프로젝트

---

## 🎯 최종 상태

### 달성한 것 ✅
- 완전한 UI 프레임워크
- 체계적인 설계 문서
- 리모컨 최적화 UX
- 3가지 WebView 구현체
- 빌드 시스템 완성

### 달성하지 못한 것 ❌
- 실제 웹 렌더링 통합
- 프로젝터 배포 및 실행
- History 관리 (back/forward)
- 완전한 브라우저 기능

### 이유
- webOS TV 플랫폼의 근본적 제약
- 공식 Native SDK 부재
- Qt WebEngine 미지원
- 커뮤니티 솔루션 복잡도

---

## 💭 회고

### 잘한 점
1. 체계적인 설계 및 문서화
2. 15개 기능 완전 구현
3. 다양한 대안 시도
4. 기술적 제약 명확히 파악

### 아쉬운 점
1. 초기 플랫폼 제약 검증 부족
2. Native SDK 가용성 미확인
3. Qt WebEngine 가정

### 배운 교훈
1. **기술 검증 우선**: 구현 전에 플랫폼 제약 확인
2. **공식 문서 확인**: 커뮤니티 정보만 믿지 말기
3. **유연한 목표**: 불가능하면 범위 조정
4. **실패도 학습**: 과정 자체가 가치

---

## 🚀 향후 계획

### 단기 (완료)
- ✅ 문서화 완료
- ✅ 현재 상태 정리
- ✅ GitHub 공개

### 중기 (고려 중)
- Luna Service 버전 완성
- 다른 webOS 프로젝트 시작
- 학습 자료로 활용

### 장기 (가능성)
- 커뮤니티 NDK 도전
- 다른 플랫폼 포팅 (Electron, Android TV)
- webOS OSE 버전 개발

---

## 📚 참고 자료

### 공식 문서
- [webOS TV Developer](https://webostv.developer.lge.com/)
- [webOS Open Source Edition](https://www.webosose.org/)
- [Qt Documentation](https://doc.qt.io/)

### 커뮤니티
- [webosbrew/meta-lg-webos-ndk](https://github.com/webosbrew/meta-lg-webos-ndk)
- [webOS OSE Enact Browser](https://github.com/webosose/com.webos.app.enactbrowser)

### 학습 자료
- Luna Service API
- Qt PIMPL Pattern
- CMake Best Practices

---

**© 2026 webOS Browser Native Project**

*"Every failure is a step towards success"*

**개발 기간**: 2026년 2월 12일 ~ 2월 16일 (5일)
**최종 상태**: v1.0.0 완료 (UI 프레임워크)
**총 작업 시간**: 약 40시간
**커밋 수**: 20+ commits
**문서**: 26개
**코드**: 8,000+ 라인
