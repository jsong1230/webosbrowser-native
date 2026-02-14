# 즐겨찾기 홈 화면 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: `docs/specs/favorites-home-screen/requirements.md`
- 기술 설계서: `docs/specs/favorites-home-screen/design.md`
- 기능 백로그: `docs/project/features.md` (F-15)

---

## 2. 구현 목표 및 범위

### 구현 목표
브라우저 시작 시 또는 홈 버튼 클릭 시 자주 방문한 웹사이트를 그리드 형태로 표시하는 홈 화면을 구현하여, 사용자가 리모컨으로 빠르게 자주 가는 사이트에 접근할 수 있도록 합니다.

### Phase 1 (MVP) — 이번 작업 범위
**자동 모드 구현 (북마크/히스토리 기반 타일 자동 생성)**
- HomeScreenService 구현 (타일 데이터 관리, LocalStorage)
- HomePage 컴포넌트 구현 (타일 그리드 UI, Spotlight 네비게이션)
- BrowserView 통합 (`about:home` 라우팅)
- NavigationBar 홈 버튼 수정 (홈 화면 이동)
- 빈 홈 화면 처리 (북마크/히스토리 없을 때 안내 메시지)
- 리모컨 키 매핑 (방향키 + 선택 버튼)
- 단위/컴포넌트/통합 테스트

### 제외 항목 (Phase 2, 3)
- **Phase 2 (편집 모드)**: 타일 수동 추가/삭제/순서 변경 → 사용자 피드백 후 확장
- **Phase 3 (폴리싱)**: 파비콘 캐싱(IndexedDB), 애니메이션, 고급 접근성 → M3 이후
- **범위 외**: 사이트 스크린샷, 홈 화면 위젯, 다중 홈 화면, 드래그 앤 드롭

---

## 3. 구현 Phase

### Phase 1-1: HomeScreenService 구현
**담당**: frontend-dev

#### Task 1-1-1: HomeScreenService 기본 API 구현
- [ ] `getTiles(count)` — 타일 설정 로드 및 자동/수동 모드 분기
- [ ] `getTopSitesByVisitCount(count)` — 북마크/히스토리 기반 상위 N개 선정
- [ ] `deduplicateByDomain(tiles)` — 도메인 중복 제거 (visitCount 높은 것 우선)
- [ ] `extractDomain(url)` — URL에서 도메인 추출 (www 제거)
- **파일**: `src/services/homeScreenService.js`
- **완료 기준**:
  - 북마크 우선 선정 (visitCount 역순 정렬)
  - 부족하면 히스토리에서 보충
  - 동일 도메인 중복 제거 완료
  - URL 파싱 실패 시 원본 반환

#### Task 1-1-2: LocalStorage 타일 설정 관리
- [ ] `loadTileConfig()` — LocalStorage에서 타일 설정 로드
- [ ] `saveTileConfig(config)` — LocalStorage에 타일 설정 저장
- [ ] 기본 설정 반환 (tileType: 'auto', tileCount: 6)
- [ ] LocalStorage 접근 실패 시 에러 처리
- **파일**: `src/services/homeScreenService.js`
- **완료 기준**:
  - 앱 재시작 후 타일 설정 유지
  - LocalStorage 실패 시 기본 설정 반환
  - 설정 구조: `{ version, tileIds, tileType, tileCount }`

#### Task 1-1-3: 북마크/히스토리 서비스 연동
- [ ] `bookmarkService.getAllBookmarks()` 호출 및 visitCount 정렬
- [ ] `historyService.getAllHistory()` 호출 및 visitCount 정렬
- [ ] 타일 데이터 구조 변환 (id, url, title, favicon, visitCount, source, domain)
- [ ] 성능 최적화 (상위 50개만 처리)
- **파일**: `src/services/homeScreenService.js`
- **완료 기준**:
  - 북마크 100개, 히스토리 1,000개 이상일 때도 1초 이내 처리
  - 타일 데이터 구조 통일 (bookmark/history 구분)

**예상 산출물**:
- `src/services/homeScreenService.js` (신규, 약 200줄)

**완료 기준**:
- 단위 테스트 통과 (getTiles, getTopSitesByVisitCount, deduplicateByDomain)
- 대용량 데이터 처리 1초 이내 (북마크 100개 + 히스토리 1,000개)

---

### Phase 1-2: HomePage 컴포넌트 구현
**담당**: frontend-dev
**의존성**: Phase 1-1 완료 후 시작

#### Task 1-2-1: HomePage 기본 구조 및 타일 로딩
- [ ] HomePage 컴포넌트 생성 (Props: onNavigate, onClose, className)
- [ ] State 관리 (tiles, isLoading, isError)
- [ ] useEffect로 HomeScreenService.getTiles() 호출
- [ ] 로딩 중 상태 처리 (LoadingSpinner 표시)
- [ ] 에러 상태 처리 (에러 메시지 표시)
- **파일**: `src/components/HomePage/HomePage.js`
- **완료 기준**:
  - 마운트 시 타일 로드 완료
  - 로딩/에러 상태 UI 표시
  - 타일 데이터 state 저장

#### Task 1-2-2: 타일 그리드 레이아웃 및 스타일링
- [ ] CSS Grid 레이아웃 구현 (3x2 고정 그리드, 타일 크기 300x300px)
- [ ] 타일 스타일 (파비콘 96x96px, 제목 28px, URL 18px)
- [ ] 포커스 스타일 (3px 테두리, scale 1.05, 그림자)
- [ ] 다크/라이트 테마 대응 (CSS 변수 사용)
- **파일**: `src/components/HomePage/HomePage.module.less`
- **완료 기준**:
  - 타일 크기 최소 200x200px (요구사항)
  - 3m 거리 가독성 확보 (폰트 크기 24px 이상)
  - 대비 비율 4.5:1 이상 (WCAG AA)

#### Task 1-2-3: 타일 렌더링 및 클릭 핸들러
- [ ] 타일 배열 렌더링 (map)
- [ ] 파비콘 표시 (기본 아이콘 폴백: Google Favicon API)
- [ ] 타일 클릭 핸들러 (onNavigate 콜백 호출)
- [ ] 북마크 타일 클릭 시 visitCount 증가 (bookmarkService)
- **파일**: `src/components/HomePage/HomePage.js`
- **완료 기준**:
  - 타일 클릭 시 WebView로 URL 전달
  - 파비콘 로드 실패 시 Google Favicon API 폴백
  - 북마크 타일 클릭 시 방문 횟수 증가

#### Task 1-2-4: Spotlight 네비게이션 (리모컨 지원)
- [ ] Spotlight 컨테이너 설정 (home-page)
- [ ] 각 타일에 spotlightId 지정 (tile-0, tile-1, ...)
- [ ] 초기 포커스 설정 (첫 번째 타일)
- [ ] 그리드 네비게이션 (방향키 상/하/좌/우)
- [ ] 선택 버튼으로 타일 클릭 이벤트 발생
- **파일**: `src/components/HomePage/HomePage.js`
- **완료 기준**:
  - 리모컨 방향키로 타일 간 포커스 이동
  - 포커스된 타일 명확히 표시 (하이라이트)
  - 선택 버튼으로 사이트 로드

#### Task 1-2-5: 빈 홈 화면 처리
- [ ] 타일 없을 때 (tiles.length === 0) 안내 메시지 렌더링
- [ ] "북마크 관리" 버튼 (BookmarkPanel 열기)
- [ ] "URL 입력" 버튼 (URLBar로 포커스 이동)
- [ ] 빈 상태 스타일링 (중앙 정렬, 아이콘, 버튼)
- **파일**: `src/components/HomePage/HomePage.js`, `HomePage.module.less`
- **완료 기준**:
  - 북마크 0개, 히스토리 0개일 때 안내 메시지 표시
  - "북마크 관리" 버튼으로 북마크 패널 열기
  - "URL 입력" 버튼으로 URLBar 포커스 이동

#### Task 1-2-6: 북마크/히스토리 변경 감지 및 타일 재로드
- [ ] bookmarkschanged 이벤트 리스너 등록
- [ ] historychanged 이벤트 리스너 등록
- [ ] 이벤트 발생 시 타일 재로드 (loadTiles)
- [ ] 컴포넌트 언마운트 시 이벤트 리스너 제거
- **파일**: `src/components/HomePage/HomePage.js`
- **완료 기준**:
  - 북마크 추가/삭제 시 홈 화면 자동 갱신
  - 페이지 방문 시 홈 화면 자동 갱신

**예상 산출물**:
- `src/components/HomePage/HomePage.js` (신규, 약 200줄)
- `src/components/HomePage/HomePage.module.less` (신규, 약 150줄)
- `src/components/HomePage/index.js` (신규, export)

**완료 기준**:
- AC-1 (홈 화면 표시) 통과
- AC-2 (자주 가는 사이트 자동 선정) 통과
- AC-4 (타일 클릭 시 사이트 로드) 통과
- AC-7 (빈 홈 화면 처리) 통과
- AC-8 (리모컨 키 매핑) 통과
- AC-9 (성능 및 가독성) 통과

---

### Phase 1-3: BrowserView 통합
**담당**: frontend-dev
**의존성**: Phase 1-2 완료 후 시작

#### Task 1-3-1: BrowserView 라우팅 로직 추가
- [ ] currentUrl === 'about:home' 조건 추가
- [ ] HomePage 컴포넌트 조건부 렌더링 (WebView 대신)
- [ ] handleHomePageNavigate 콜백 구현 (타일 클릭 시 URL 업데이트)
- [ ] TabContext UPDATE_TAB 디스패치 (활성 탭 URL 변경)
- **파일**: `src/views/BrowserView.js`
- **완료 기준**:
  - currentUrl이 'about:home'이면 HomePage 렌더링
  - HomePage에서 타일 클릭 시 WebView로 전환
  - URL 입력창도 함께 업데이트

#### Task 1-3-2: 앱 초기 URL 설정
- [ ] TabContext 초기 상태 수정 (createInitialTab)
- [ ] settingsService.loadSettings()에서 homepageUrl 로드
- [ ] homepageUrl이 없으면 'about:home' 기본값 사용
- [ ] 첫 번째 탭의 초기 URL 설정
- **파일**: `src/contexts/TabContext.js` (또는 `src/App/App.js`)
- **완료 기준**:
  - 앱 시작 시 첫 번째 탭에 홈 화면 표시
  - F-11 설정에서 홈페이지 URL 지정 시 해당 URL 우선

**예상 산출물**:
- `src/views/BrowserView.js` (수정, 약 20줄 추가)
- `src/contexts/TabContext.js` (수정, 약 10줄 추가)

**완료 기준**:
- AC-4 (브라우저 시작 시 홈 화면 표시) 통과
- 홈 화면 → 타일 클릭 → WebView 전환 정상 동작

---

### Phase 1-4: NavigationBar 홈 버튼 수정
**담당**: frontend-dev
**의존성**: Phase 1-3과 병렬 가능 (간단한 수정)

#### Task 1-4-1: NavigationBar 홈 버튼 동작 수정
- [ ] handleHome 콜백 구현 (onNavigate({ action: 'home', url: 'about:home' }))
- [ ] BrowserView에서 홈 버튼 클릭 이벤트 수신
- [ ] 활성 탭의 URL을 'about:home'으로 변경
- **파일**: `src/components/NavigationBar/NavigationBar.js`
- **완료 기준**:
  - 홈 버튼 클릭 시 HomePage 렌더링
  - 이미 홈 화면이면 동작 없음

**예상 산출물**:
- `src/components/NavigationBar/NavigationBar.js` (수정, 약 10줄 추가)

**완료 기준**:
- AC-3 (홈 버튼으로 홈 화면 이동) 통과

---

### Phase 1-5: 북마크/히스토리 서비스 이벤트 추가
**담당**: frontend-dev
**의존성**: Phase 1-1과 병렬 가능

#### Task 1-5-1: bookmarkService에 이벤트 발생 추가
- [ ] addBookmark() 완료 시 `window.dispatchEvent(new CustomEvent('bookmarkschanged'))`
- [ ] updateBookmark() 완료 시 이벤트 발생
- [ ] deleteBookmark() 완료 시 이벤트 발생
- **파일**: `src/services/bookmarkService.js`
- **완료 기준**:
  - 북마크 추가/수정/삭제 시 이벤트 발생
  - HomePage가 이벤트 수신 가능

#### Task 1-5-2: historyService에 이벤트 발생 추가
- [ ] addHistoryEntry() 완료 시 `window.dispatchEvent(new CustomEvent('historychanged'))`
- [ ] deleteHistoryEntry() 완료 시 이벤트 발생
- **파일**: `src/services/historyService.js`
- **완료 기준**:
  - 히스토리 추가/삭제 시 이벤트 발생
  - HomePage가 이벤트 수신 가능

**예상 산출물**:
- `src/services/bookmarkService.js` (수정, 약 5줄 추가)
- `src/services/historyService.js` (수정, 약 5줄 추가)

**완료 기준**:
- 북마크/히스토리 변경 시 HomePage 자동 갱신

---

### Phase 1-6: 테스트 작성
**담당**: test-runner
**의존성**: Phase 1-1 ~ 1-5 완료 후 시작

#### Task 1-6-1: HomeScreenService 단위 테스트
- [ ] `getTiles()` 기본 6개 타일 반환 테스트
- [ ] `getTopSitesByVisitCount()` visitCount 역순 정렬 테스트
- [ ] `deduplicateByDomain()` 동일 도메인 중복 제거 테스트
- [ ] `loadTileConfig()` LocalStorage 실패 시 기본값 반환 테스트
- [ ] `extractDomain()` URL 파싱 테스트
- **파일**: `src/services/__tests__/homeScreenService.test.js`
- **완료 기준**:
  - 모든 테스트 통과 (최소 10개 테스트 케이스)
  - 코드 커버리지 80% 이상

#### Task 1-6-2: HomePage 컴포넌트 테스트
- [ ] 타일 로딩 중 로딩 인디케이터 표시 테스트
- [ ] 타일 로드 성공 시 타일 그리드 렌더링 테스트
- [ ] 타일 클릭 시 onNavigate 콜백 호출 테스트
- [ ] 타일 없을 때 빈 상태 메시지 표시 테스트
- [ ] 북마크/히스토리 변경 이벤트 수신 시 재로드 테스트
- **파일**: `src/components/HomePage/__tests__/HomePage.test.js`
- **완료 기준**:
  - 모든 테스트 통과 (최소 8개 테스트 케이스)
  - 코드 커버리지 80% 이상

#### Task 1-6-3: BrowserView + HomePage 통합 테스트
- [ ] 홈 버튼 클릭 시 HomePage 렌더링 테스트
- [ ] HomePage 타일 클릭 시 WebView로 전환 테스트
- [ ] 앱 시작 시 홈 화면 표시 테스트
- [ ] 북마크 추가 후 홈 화면 갱신 테스트
- **파일**: `src/views/__tests__/BrowserView-HomePage.integration.test.js`
- **완료 기준**:
  - 모든 테스트 통과 (최소 5개 테스트 케이스)
  - 전체 흐름 정상 동작 검증

**예상 산출물**:
- `src/services/__tests__/homeScreenService.test.js` (신규, 약 150줄)
- `src/components/HomePage/__tests__/HomePage.test.js` (신규, 약 150줄)
- `src/views/__tests__/BrowserView-HomePage.integration.test.js` (신규, 약 100줄)

**완료 기준**:
- 전체 테스트 통과
- 코드 커버리지 80% 이상

---

### Phase 1-7: 코드 및 문서 리뷰
**담당**: code-reviewer
**의존성**: Phase 1-6 완료 후 시작

#### Task 1-7-1: 코드 리뷰
- [ ] HomeScreenService 로직 검증 (북마크 우선 선정, 도메인 중복 제거)
- [ ] HomePage 컴포넌트 Spotlight 설정 검증
- [ ] BrowserView 라우팅 로직 검증
- [ ] 에러 처리 완전성 검증
- [ ] 성능 최적화 확인 (대용량 데이터 처리)
- **완료 기준**:
  - 코드 리뷰 통과
  - 리팩토링 제안 반영

#### Task 1-7-2: 문서 리뷰
- [ ] requirements.md 완료 기준 매핑 검증
- [ ] design.md 구현 일치 확인
- [ ] 컴포넌트 문서 작성 필요 여부 확인
- **완료 기준**:
  - 문서 누락 항목 없음
  - AC-1 ~ AC-10 모두 충족

---

### Phase 1-8: 컴포넌트 문서 작성
**담당**: frontend-dev
**의존성**: Phase 1-7 완료 후 시작

#### Task 1-8-1: HomePage 컴포넌트 문서
- [ ] Props 인터페이스 문서화
- [ ] 사용 예시 (BrowserView에서 사용)
- [ ] 리모컨 키 매핑 (방향키, 선택 버튼)
- [ ] 주의사항 (북마크/히스토리 의존성)
- **파일**: `docs/components/HomePage.md`
- **완료 기준**:
  - 컴포넌트 문서 완성
  - 다른 개발자가 재사용 가능하도록 명확한 설명

#### Task 1-8-2: HomeScreenService 문서
- [ ] API 목록 문서화 (getTiles, getTopSitesByVisitCount, ...)
- [ ] 타일 데이터 구조 설명
- [ ] LocalStorage 스키마 설명
- [ ] 사용 예시
- **파일**: `docs/components/HomeScreenService.md`
- **완료 기준**:
  - 서비스 API 문서 완성
  - 개발자가 API 사용법 이해 가능

**예상 산출물**:
- `docs/components/HomePage.md` (신규, 약 100줄)
- `docs/components/HomeScreenService.md` (신규, 약 80줄)

**완료 기준**:
- 컴포넌트 및 서비스 문서 완성

---

## 4. 태스크 의존성 그래프

```
Phase 1-1: HomeScreenService 구현
  │
  ├─ Task 1-1-1: 기본 API 구현
  ├─ Task 1-1-2: LocalStorage 관리
  └─ Task 1-1-3: 북마크/히스토리 연동
        │
        ▼
Phase 1-2: HomePage 컴포넌트 구현
  │
  ├─ Task 1-2-1: 기본 구조 및 타일 로딩
  ├─ Task 1-2-2: 그리드 레이아웃 및 스타일링
  ├─ Task 1-2-3: 타일 렌더링 및 클릭 핸들러
  ├─ Task 1-2-4: Spotlight 네비게이션
  ├─ Task 1-2-5: 빈 홈 화면 처리
  └─ Task 1-2-6: 북마크/히스토리 변경 감지
        │
        ├──────────────┬──────────────┐
        ▼              ▼              ▼
Phase 1-3:       Phase 1-4:     Phase 1-5:
BrowserView      NavigationBar   이벤트 추가
통합             홈 버튼         (병렬 가능)
        │              │              │
        └──────────────┴──────────────┘
                       │
                       ▼
Phase 1-6: 테스트 작성
  │
  ├─ Task 1-6-1: 단위 테스트
  ├─ Task 1-6-2: 컴포넌트 테스트
  └─ Task 1-6-3: 통합 테스트
        │
        ▼
Phase 1-7: 코드 및 문서 리뷰
        │
        ▼
Phase 1-8: 컴포넌트 문서 작성
```

**병렬 실행 가능 구간**:
- Phase 1-3 (BrowserView 통합) + Phase 1-4 (NavigationBar 수정) + Phase 1-5 (이벤트 추가)
  - 서로 독립적인 수정 사항
  - 단, Phase 1-2 (HomePage) 완료 후 시작

---

## 5. 병렬 실행 판단

### 독립 태스크 분석
| Phase | 독립성 | 병렬 가능 여부 |
|-------|--------|---------------|
| Phase 1-1 (HomeScreenService) | 완전 독립 | ✅ 단독 실행 (기초 작업) |
| Phase 1-2 (HomePage) | Phase 1-1 의존 | ❌ Phase 1-1 완료 후 시작 |
| Phase 1-3 (BrowserView) | Phase 1-2 의존 | ✅ Phase 1-4, 1-5와 병렬 가능 |
| Phase 1-4 (NavigationBar) | Phase 1-2 의존 | ✅ Phase 1-3, 1-5와 병렬 가능 |
| Phase 1-5 (이벤트 추가) | 독립 (서비스 수정) | ✅ Phase 1-3, 1-4와 병렬 가능 |
| Phase 1-6 (테스트) | Phase 1-1~1-5 의존 | ❌ 모든 구현 완료 후 시작 |
| Phase 1-7 (리뷰) | Phase 1-6 의존 | ❌ 테스트 통과 후 시작 |
| Phase 1-8 (문서) | Phase 1-7 의존 | ❌ 리뷰 통과 후 시작 |

### Agent Team vs 순차 실행 권장

**권장 방식: 순차 실행**

**이유**:
1. **밀접한 연관성**:
   - HomePage와 HomeScreenService는 하나의 기능을 구성하는 핵심 요소
   - BrowserView 통합과 NavigationBar 수정은 간단한 라우팅 로직 추가 (약 30줄)
   - 분리 시 설계서 해석 중복 및 커뮤니케이션 오버헤드 발생

2. **병렬 구간 제한적**:
   - Phase 1-3 ~ 1-5만 병렬 가능 (약 30~40분 작업량)
   - 전체 작업 대비 병렬 구간 비중 낮음 (약 20%)

3. **단일 개발자 최적화**:
   - 설계서를 한 번 읽고 전체 흐름을 이해한 상태에서 작업하는 것이 효율적
   - HomePage → BrowserView → NavigationBar 순차 작업 시 컨텍스트 전환 최소화

4. **테스트 복잡도**:
   - 통합 테스트에서 모든 컴포넌트가 협업해야 함
   - 병렬 작업 시 통합 테스트 실패 위험 증가

**예외 상황 (Agent Team 사용 고려)**:
- Phase 2 (편집 모드) 구현 시:
  - 편집 UI (HomePage 수정) + 북마크 선택 다이얼로그 (신규) → 병렬 가능
- Phase 3 (폴리싱) 구현 시:
  - 파비콘 캐싱 (서비스) + 애니메이션 (CSS) → 병렬 가능

### 예상 작업 시간
| Phase | 순차 실행 시 예상 시간 | 병렬 실행 시 예상 시간 |
|-------|----------------------|----------------------|
| Phase 1-1 | 3~4시간 | 3~4시간 |
| Phase 1-2 | 4~5시간 | 4~5시간 |
| Phase 1-3~1-5 | 2~3시간 | 1~1.5시간 (병렬) |
| Phase 1-6 | 3~4시간 | 3~4시간 |
| Phase 1-7~1-8 | 1~2시간 | 1~2시간 |
| **총합** | **13~18시간** | **12.5~16.5시간** |

**결론**: 병렬 실행 시 약 30분~1.5시간 절약 (약 10% 단축) → 순차 실행 권장

---

## 6. 리스크 및 대응

### 리스크 1: LocalStorage 용량 초과
- **영향도**: 중간 (타일 설정 저장 실패)
- **발생 확률**: 낮음 (타일 설정은 메타데이터만 저장, 약 1KB)
- **대응 방안**:
  - LocalStorage 저장 실패 시 기본 설정 반환
  - 에러 로깅 및 사용자 안내 메시지 표시
  - Phase 2에서 IndexedDB 마이그레이션 고려

### 리스크 2: 파비콘 로드 실패
- **영향도**: 낮음 (기본 아이콘으로 대체 가능)
- **발생 확률**: 중간 (일부 사이트는 파비콘 없음)
- **대응 방안**:
  - Google Favicon API 폴백 (`https://www.google.com/s2/favicons?domain=...`)
  - 폴백도 실패 시 로컬 기본 아이콘 표시 (`/resources/icons/default-favicon.png`)
  - 파비콘 로드 에러는 무시 (사용자 경험에 큰 영향 없음)

### 리스크 3: 북마크/히스토리 없을 때 빈 홈 화면
- **영향도**: 높음 (신규 사용자 경험 저하)
- **발생 확률**: 높음 (첫 실행 시 북마크 0개)
- **대응 방안**:
  - 빈 홈 화면 안내 메시지 + 액션 버튼 제공
  - "북마크 관리" 버튼으로 북마크 추가 유도
  - "URL 입력" 버튼으로 즉시 웹 서핑 시작
  - Phase 2에서 기본 추천 사이트 제공 고려 (YouTube, Google, Naver 등)

### 리스크 4: 대용량 북마크/히스토리 성능 저하
- **영향도**: 중간 (홈 화면 로딩 1초 초과)
- **발생 확률**: 낮음 (일반 사용자는 북마크 100개 미만)
- **대응 방안**:
  - 북마크/히스토리 조회 시 상위 50개만 처리
  - visitCount 정렬 최적화 (내림차순 정렬 후 slice)
  - Phase 3에서 타일 데이터 캐싱 (메모리 또는 IndexedDB)

### 리스크 5: Spotlight 포커스 이슈
- **영향도**: 높음 (리모컨 네비게이션 불가)
- **발생 확률**: 중간 (Enact Spotlight 설정 복잡)
- **대응 방안**:
  - Spotlight 초기 포커스 명시 (첫 번째 타일)
  - 그리드 네비게이션 테스트 강화 (방향키 상/하/좌/우)
  - Enact 공식 문서 및 샘플 코드 참조
  - 실제 webOS 디바이스에서 리모컨 테스트

### 리스크 6: 북마크/히스토리 이벤트 누락
- **영향도**: 중간 (홈 화면 갱신 실패)
- **발생 확률**: 낮음 (이벤트 발생 코드 누락)
- **대응 방안**:
  - bookmarkService, historyService 모든 CRUD 메서드에 이벤트 발생 추가
  - 이벤트 발생 테스트 코드 작성
  - HomePage 이벤트 수신 로그 확인

---

## 7. 테스트 계획

### 단위 테스트 (HomeScreenService)
**대상**: `src/services/homeScreenService.js`

| 테스트 케이스 | 검증 내용 |
|-------------|----------|
| `getTiles(6)` 기본 동작 | 6개 타일 반환 |
| `getTopSitesByVisitCount(3)` 정렬 | visitCount 역순 정렬 |
| `deduplicateByDomain()` 중복 제거 | 동일 도메인 제거, visitCount 높은 것 우선 |
| `extractDomain()` URL 파싱 | www 제거, 파싱 실패 시 원본 반환 |
| `loadTileConfig()` 기본값 | LocalStorage 없을 때 기본 설정 반환 |
| `loadTileConfig()` 실패 처리 | LocalStorage 에러 시 기본 설정 반환 |
| `saveTileConfig()` 저장 성공 | LocalStorage에 JSON 저장 |
| 북마크 100개 + 히스토리 1,000개 | 1초 이내 처리 (성능 테스트) |

**도구**: Jest

**커버리지 목표**: 80% 이상

---

### 컴포넌트 테스트 (HomePage)
**대상**: `src/components/HomePage/HomePage.js`

| 테스트 케이스 | 검증 내용 |
|-------------|----------|
| 타일 로딩 중 | 로딩 인디케이터 표시 |
| 타일 로드 성공 | 타일 그리드 렌더링, 사이트 이름/파비콘 표시 |
| 타일 클릭 | onNavigate 콜백 호출, 올바른 URL 전달 |
| 타일 없을 때 | 빈 상태 메시지 표시 |
| 북마크 변경 이벤트 | 타일 재로드 |
| 히스토리 변경 이벤트 | 타일 재로드 |
| 파비콘 로드 실패 | Google Favicon API 폴백 |
| Spotlight 포커스 | 첫 번째 타일 초기 포커스 |

**도구**: Jest + Enact Testing Library

**커버리지 목표**: 80% 이상

---

### 통합 테스트 (BrowserView + HomePage)
**대상**: `src/views/BrowserView.js` + `src/components/HomePage/HomePage.js`

| 테스트 케이스 | 검증 내용 |
|-------------|----------|
| 앱 시작 | 홈 화면 표시 (currentUrl === 'about:home') |
| 홈 버튼 클릭 | HomePage 렌더링 |
| 타일 클릭 → WebView | URL 업데이트, WebView 렌더링 |
| 북마크 추가 → 홈 화면 갱신 | 새 북마크 타일 표시 |
| 홈 화면 → 타일 클릭 → 뒤로 가기 | 히스토리 관리 정상 동작 |

**도구**: Jest + Enact Testing Library

**커버리지 목표**: 주요 흐름 100% 커버

---

### 수동 테스트 (webOS 디바이스)
**대상**: LG 프로젝터 HU175QW

| 테스트 항목 | 검증 내용 |
|-----------|----------|
| 리모컨 방향키 네비게이션 | 타일 간 포커스 이동 (상/하/좌/우) |
| 리모컨 선택 버튼 | 타일 클릭 → 사이트 로드 |
| 리모컨 홈 버튼 | 홈 화면 이동 |
| 대화면 가독성 | 3m 거리에서 타일 제목/파비콘 식별 |
| 로딩 성능 | 홈 화면 로드 1초 이내 |
| 타일 클릭 반응 | 0.3초 이내 반응 |

**도구**: ares-install, ares-launch, ares-log

---

## 8. 예상 산출물

### 신규 파일
| 파일 경로 | 내용 | 예상 줄 수 |
|---------|------|----------|
| `src/services/homeScreenService.js` | 홈 화면 타일 데이터 관리 서비스 | 약 200줄 |
| `src/components/HomePage/HomePage.js` | 홈 화면 컴포넌트 | 약 200줄 |
| `src/components/HomePage/HomePage.module.less` | 홈 화면 스타일 | 약 150줄 |
| `src/components/HomePage/index.js` | export | 약 3줄 |
| `src/services/__tests__/homeScreenService.test.js` | 단위 테스트 | 약 150줄 |
| `src/components/HomePage/__tests__/HomePage.test.js` | 컴포넌트 테스트 | 약 150줄 |
| `src/views/__tests__/BrowserView-HomePage.integration.test.js` | 통합 테스트 | 약 100줄 |
| `docs/components/HomePage.md` | 컴포넌트 문서 | 약 100줄 |
| `docs/components/HomeScreenService.md` | 서비스 문서 | 약 80줄 |
| **총합** | | **약 1,133줄** |

### 수정 파일
| 파일 경로 | 수정 내용 | 예상 추가 줄 수 |
|---------|----------|---------------|
| `src/views/BrowserView.js` | HomePage 라우팅 로직 추가 | 약 20줄 |
| `src/components/NavigationBar/NavigationBar.js` | 홈 버튼 동작 수정 | 약 10줄 |
| `src/contexts/TabContext.js` | 초기 탭 URL 설정 (about:home) | 약 10줄 |
| `src/services/bookmarkService.js` | bookmarkschanged 이벤트 발생 | 약 5줄 |
| `src/services/historyService.js` | historychanged 이벤트 발생 | 약 5줄 |
| **총합** | | **약 50줄** |

### 총 코드량
- **신규 코드**: 약 1,133줄
- **수정 코드**: 약 50줄
- **전체**: 약 1,183줄

---

## 9. 완료 기준 (Acceptance Criteria 매핑)

| AC | 요구사항 | 검증 방법 | 담당 Phase |
|----|---------|---------|----------|
| AC-1 | 홈 화면 표시 (그리드, 파비콘, 포커스) | 컴포넌트 테스트 + 수동 테스트 | Phase 1-2 |
| AC-2 | 자주 가는 사이트 자동 선정 (visitCount) | 단위 테스트 (HomeScreenService) | Phase 1-1 |
| AC-3 | 홈 버튼으로 홈 화면 이동 | 통합 테스트 + 수동 테스트 | Phase 1-4 |
| AC-4 | 타일 클릭 시 사이트 로드 | 통합 테스트 | Phase 1-2, 1-3 |
| AC-5 | 타일 편집 기능 (선택적) | ⚠️ Phase 2에서 구현 | — |
| AC-6 | 홈 화면 데이터 영속성 | 단위 테스트 (loadTileConfig, saveTileConfig) | Phase 1-1 |
| AC-7 | 빈 홈 화면 처리 | 컴포넌트 테스트 | Phase 1-2 |
| AC-8 | 리모컨 키 매핑 | 수동 테스트 (webOS 디바이스) | Phase 1-2 |
| AC-9 | 성능 및 가독성 | 성능 테스트 + 수동 테스트 | Phase 1-1, 1-2 |
| AC-10 | 회귀 테스트 | 통합 테스트 | Phase 1-6 |

### 테스트 통과 기준
- [ ] 모든 단위 테스트 통과 (Jest)
- [ ] 모든 컴포넌트 테스트 통과 (Enact Testing Library)
- [ ] 모든 통합 테스트 통과
- [ ] 코드 커버리지 80% 이상
- [ ] webOS 디바이스 수동 테스트 통과 (리모컨 네비게이션, 가독성)
- [ ] 성능 요구사항 충족 (홈 화면 로드 1초 이내, 타일 클릭 0.3초 이내)

### 배포 기준
- [ ] AC-1, AC-2, AC-3, AC-4, AC-6, AC-7, AC-8, AC-9, AC-10 모두 통과
- [ ] 기존 기능 회귀 테스트 통과 (F-01 ~ F-14)
- [ ] 코드 리뷰 승인
- [ ] 컴포넌트 문서 완성

---

## 10. 추가 고려사항

### F-11 (환경 설정)과의 통합
- **홈페이지 URL 설정**:
  - F-11에서 홈페이지 URL 설정 시 해당 URL로 앱 시작
  - 홈페이지 URL이 없으면 `about:home` (홈 화면) 기본값
- **홈 버튼 동작**:
  - 홈 버튼은 항상 `about:home` (홈 화면)으로 이동
  - 홈페이지 URL과 홈 화면은 별개 개념

### Phase 2 (편집 모드) 준비
- **수동 모드 데이터 구조**:
  - `tileIds` 배열에 사용자가 선택한 북마크 ID 저장
  - `tileType: 'manual'`로 전환
- **편집 UI 설계**:
  - 편집 버튼, 타일 삭제 버튼, 사이트 추가 다이얼로그
  - 리모컨으로 순서 변경 (좌/우 화살표)

### Phase 3 (폴리싱) 준비
- **파비콘 캐싱**:
  - IndexedDB에 파비콘 URL 저장
  - 파비콘 이미지 Blob 저장 (선택적)
- **애니메이션**:
  - 타일 호버/포커스 애니메이션 (scale, shadow)
  - 페이지 전환 애니메이션 (fade out/in)
- **접근성**:
  - aria-label 추가 (각 타일에 "YouTube로 이동" 등)
  - 스크린 리더 지원 (webOS 호환성 확인 필요)

---

## 변경 이력
| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 최초 작성 | product-manager |
