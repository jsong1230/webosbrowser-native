# 기능 백로그 v2.0

> v1.0.0 완료 후 차기 버전을 위한 확장 기능

## 기능 목록

| ID | 기능명 | 설명 | 우선순위 | 의존성 | 병렬 그룹 | 충돌 영역 | 마일스톤 | 상태 |
|----|--------|------|----------|--------|-----------|-----------|----------|------|
| F-16 | 광고 차단 | 광고 도메인 차단, 필터 리스트 관리 | Should | F-02 | PG-4 | WebView, Settings | M4 | ⏳ 대기 |
| F-17 | 시크릿 모드 | 프라이버시 브라우징, 히스토리/쿠키 미저장 | Should | F-06 | PG-4 | TabManager, Storage | M4 | ⏳ 대기 |
| F-18 | 화면 분할 (PIP) | Picture-in-Picture, 멀티뷰 | Should | F-06 | PG-4 | TabManager, WebView | M4 | ⏳ 대기 |
| F-19 | 음성 검색 | LG ThinQ 연동, 음성 명령 | Could | F-09 | - | SearchEngine, ThinQ API | M5 | ⏳ 대기 |
| F-20 | 멀티 프로필 | 가족 구성원별 북마크/히스토리 분리 | Could | F-07, F-08 | PG-5 | Profile Manager | M5 | ⏳ 대기 |
| F-21 | LG 계정 동기화 | 북마크/설정 클라우드 백업 | Could | F-20 | PG-5 | Cloud Sync Service | M5 | ⏳ 대기 |
| F-22 | QR 코드 스캔 | 모바일 URL 공유, QR 코드 인식 | Could | F-02 | PG-5 | Camera API | M5 | ⏳ 대기 |
| F-23 | 읽기 모드 | 광고/사이드바 제거, 가독성 최적화 | Should | F-02 | - | WebView, Reader Mode | M4 | ⏳ 대기 |
| F-24 | 자동완성 개선 | URL 자동완성, 검색 제안 | Must | F-03, F-08 | - | URLBar, History | M4 | ⏳ 대기 |

### 병렬 그룹 규칙
- **같은 마일스톤** 내에서만 그룹 구성
- 그룹 내 기능 간 **상호 의존성 없음**
- 그룹 내 기능 간 **충돌 영역 미겹침**
- 그룹당 **최대 3개** 기능

#### PG-4 (M4 병렬 그룹)
- F-16 (광고 차단) - WebView + Settings
- F-17 (시크릿 모드) - TabManager + Storage
- F-18 (화면 분할) - TabManager + WebView (다른 영역)
- 충돌 가능: F-16과 F-18이 WebView 수정 → 순차 또는 2+1 분할

#### PG-5 (M5 병렬 그룹)
- F-20 (멀티 프로필) - Profile Manager
- F-21 (LG 계정 동기화) - Cloud Sync
- F-22 (QR 코드 스캔) - Camera API
- 충돌 없음: 완전히 독립적인 기능

### 상태 범례
- ⏳ 대기: 아직 시작하지 않음
- 🔄 진행중: 현재 개발 중
- ✅ 완료: 개발 + 테스트 + 리뷰 완료
- ⏸️ 보류: 일시 중단
- ❌ 취소: 구현하지 않기로 결정

---

## 기능별 상세 설명

### F-16: 광고 차단 (Ad Blocker)

**목표**: 성가신 광고를 차단하여 쾌적한 브라우징 환경 제공

**작업 내용**:
- AdBlockService: EasyList 기반 필터 리스트 로드
- WebView Request Interceptor: URL 패턴 매칭으로 광고 차단
- SettingsPanel: 광고 차단 ON/OFF, 커스텀 필터 추가
- 화이트리스트: 특정 사이트 광고 허용
- 차단 통계: 차단된 광고 수 표시

**예상 복잡도**: High (WebView API 조사 필요)

**완료 기준**:
- YouTube 광고 차단
- 뉴스 사이트 배너 광고 차단
- 설정에서 ON/OFF 가능
- 화이트리스트 동작

---

### F-17: 시크릿 모드 (Incognito Mode)

**목표**: 프라이버시 보호, 히스토리/쿠키 미저장

**작업 내용**:
- PrivacyTab: 시크릿 탭 타입 추가
- TabManager: 일반 탭과 시크릿 탭 분리 관리
- StorageService: 시크릿 모드에서 히스토리/쿠키 저장 비활성화
- UI 구분: 시크릿 탭 아이콘 (🕵️), 다크 테마
- 탭바: 시크릿 탭 표시

**예상 복잡도**: Medium

**완료 기준**:
- 시크릿 탭 생성 가능
- 히스토리 기록 안 됨
- 쿠키 저장 안 됨
- 탭 전환 시 구분 명확

---

### F-18: 화면 분할 (Picture-in-Picture)

**목표**: YouTube 동영상을 작은 창으로 보면서 다른 사이트 탐색

**작업 내용**:
- PIPManager: PIP 윈도우 관리
- WebView PIP API: Qt WebEngine PIP 지원 조사
- TabManager: PIP 탭과 메인 탭 동시 렌더링
- UI: PIP 컨트롤 (크기 조절, 위치 이동, 닫기)
- 리모컨: 채널 버튼으로 PIP <-> 메인 전환

**예상 복잡도**: High (webOS PIP API 제약)

**완료 기준**:
- YouTube 동영상 PIP 재생
- PIP 창 크기/위치 조절
- 메인 탭에서 다른 사이트 탐색
- 리모컨 조작 가능

---

### F-19: 음성 검색 (Voice Search)

**목표**: LG ThinQ 음성 인식으로 URL/검색어 입력

**작업 내용**:
- VoiceSearchService: LG ThinQ API 연동
- URLBar: 음성 입력 버튼 추가
- 음성 → 텍스트 변환: LG ASR (Automatic Speech Recognition)
- 검색 의도 파싱: URL vs 검색어 자동 판단
- 리모컨: 마이크 버튼 단축키

**예상 복잡도**: High (LG ThinQ API 조사 필요)

**완료 기준**:
- "네이버 열어줘" → naver.com 로드
- "날씨 검색" → Google 검색
- 리모컨 마이크 버튼 동작
- 한국어/영어 지원

---

### F-20: 멀티 프로필 (Multi-Profile)

**목표**: 가족 구성원별 브라우징 환경 분리

**작업 내용**:
- ProfileManager: 프로필 생성/전환/삭제
- Profile: 이름, 아바타, 북마크, 히스토리, 설정 분리
- ProfileSwitcher UI: 프로필 선택 화면
- StorageService: 프로필별 데이터 분리 저장
- 리모컨: 채널 Up/Down으로 프로필 빠른 전환

**예상 복잡도**: Medium

**완료 기준**:
- 최대 5개 프로필 생성
- 프로필 전환 시 북마크/히스토리 분리
- 프로필별 설정 저장
- 리모컨 빠른 전환

---

### F-21: LG 계정 동기화 (Account Sync)

**목표**: 북마크/설정을 LG 클라우드에 백업

**작업 내용**:
- CloudSyncService: LG 계정 API 연동
- 북마크 동기화: 로컬 ↔ 클라우드 양방향 동기화
- 설정 동기화: 검색 엔진, 테마 등
- 충돌 해결: 최신 타임스탬프 우선
- SettingsPanel: 계정 로그인/로그아웃, 동기화 ON/OFF

**예상 복잡도**: High (LG 계정 API 조사 필요)

**완료 기준**:
- LG 계정 로그인
- 북마크 자동 동기화
- 다른 webOS 기기에서 동일 북마크 확인
- 충돌 없이 병합

---

### F-22: QR 코드 스캔 (QR Code Scanner)

**목표**: 모바일에서 프로젝터로 URL 빠르게 공유

**작업 내용**:
- QRScannerService: 프로젝터 카메라 API 조사 (있으면)
- 대체 방안: 모바일 앱으로 QR 코드 생성 → 프로젝터 앱에서 표시
- QRPanel: QR 코드 스캔 화면 (카메라 뷰)
- URL 파싱: QR 코드에서 URL 추출
- 리모컨: Blue 버튼으로 QR 스캔 모드

**예상 복잡도**: Very High (카메라 API 없을 가능성)

**완료 기준**:
- 모바일에서 QR 코드 스캔
- URL 자동 로드
- 또는 모바일 → 프로젝터 URL 전송

---

### F-23: 읽기 모드 (Reader Mode)

**목표**: 광고/사이드바 제거, 본문만 깔끔하게 표시

**작업 내용**:
- ReaderModeService: Mozilla Readability.js 이식
- WebView: 페이지 HTML 파싱, 본문 추출
- ReaderView: 본문만 표시 (커스텀 폰트, 배경색)
- SettingsPanel: 폰트 크기, 배경색, 여백 조절
- NavigationBar: 읽기 모드 버튼 (📖)

**예상 복잡도**: Medium

**완료 기준**:
- 뉴스 기사 읽기 모드 변환
- 폰트 크기 조절
- 다크/라이트 테마
- 읽기 모드 ON/OFF 버튼

---

### F-24: 자동완성 개선 (Enhanced Autocomplete)

**목표**: URL 입력을 더 빠르고 정확하게

**작업 내용**:
- URLBar: 자동완성 드롭다운 (히스토리 + 북마크 + 검색 제안)
- HistoryService: 자주 방문한 사이트 우선 순위
- SearchEngine: Google Suggest API 연동
- 리모컨: 위/아래 화살표로 제안 선택
- 퍼지 매칭: 오타 보정

**예상 복잡도**: Low

**완료 기준**:
- "naver" 입력 시 "naver.com" 제안
- 히스토리 기반 자동완성
- Google 검색 제안 표시
- 리모컨 조작 가능

---

## 마일스톤 계획

### Milestone 4: 사용성 개선 (M4)

**목표**: 광고 차단, 시크릿 모드, 읽기 모드 등 일상 사용성 향상

**포함 기능**: F-16, F-17, F-23, F-24

**예상 작업량**: 10일

**병렬 실행**:
- F-16 (광고 차단) + F-17 (시크릿 모드) 병렬 가능
- F-23 (읽기 모드) 단독
- F-24 (자동완성) 단독

---

### Milestone 5: 고급 기능 (M5)

**목표**: LG ThinQ 연동, 프로필, 클라우드 동기화 등 차별화 기능

**포함 기능**: F-18, F-19, F-20, F-21, F-22

**예상 작업량**: 15일

**병렬 실행**:
- F-20 (멀티 프로필) + F-21 (계정 동기화) 순차 (의존성)
- F-18 (PIP) + F-22 (QR 스캔) 병렬 가능
- F-19 (음성 검색) 단독

---

## 우선순위 결정

### Phase 1 (빠르게 가치 전달)
1. **F-24: 자동완성 개선** (Low 복잡도, High 가치)
2. **F-23: 읽기 모드** (Medium 복잡도, High 가치)
3. **F-17: 시크릿 모드** (Medium 복잡도, High 가치)

### Phase 2 (차별화)
4. **F-16: 광고 차단** (High 복잡도, Very High 가치)
5. **F-20: 멀티 프로필** (Medium 복잡도, Medium 가치)

### Phase 3 (고급 기능)
6. **F-18: 화면 분할** (High 복잡도, Medium 가치)
7. **F-19: 음성 검색** (High 복잡도, LG ThinQ 의존)
8. **F-21: 계정 동기화** (High 복잡도, LG 계정 의존)
9. **F-22: QR 코드** (Very High 복잡도, 카메라 의존)

---

## 기술적 리스크

| 기능 | 리스크 | 대응 |
|------|--------|------|
| F-16 (광고 차단) | WebView Request Intercept API 제약 | Qt WebEngine setUrlRequestInterceptor 조사 |
| F-18 (PIP) | webOS PIP API 지원 불확실 | Qt Widget overlay 대체 방안 |
| F-19 (음성 검색) | LG ThinQ API 접근 권한 | webOS Voice Agent API 조사 |
| F-21 (계정 동기화) | LG 계정 API 문서 부족 | REST API 리버스 엔지니어링 |
| F-22 (QR 스캔) | 프로젝터 카메라 없음 | 모바일 앱 연동 대체 |

---

## 사용자 피드백 수집

v1.0.0 배포 후 사용자 피드백을 기반으로 우선순위 재조정:

1. **설문조사**: Google Forms (가장 필요한 기능 투표)
2. **GitHub Issues**: 기능 요청 이슈 트래킹
3. **사용 로그**: 가장 많이 사용되는 기능 분석

---

**© 2026 webOS Browser Native Project - v2.0 Roadmap**
