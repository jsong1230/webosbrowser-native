# 히스토리 관리 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: docs/specs/history-management/requirements.md
- 기술 설계서: docs/specs/history-management/design.md
- 북마크 관리 구현 계획: docs/specs/bookmark-management/plan.md (유사 패턴 참조)

## 2. 구현 개요

### 목적
사용자가 방문한 웹 페이지를 자동으로 기록하고, 날짜별로 그룹화된 UI로 조회, 검색, 삭제할 수 있는 히스토리 관리 기능을 구현합니다.

### 핵심 구현 요소
1. **historyService**: IndexedDB 기반 히스토리 CRUD 서비스
2. **HistoryPanel**: 히스토리 목록 UI (VirtualList 기반 날짜별 그룹화)
3. **WebView 연동**: onLoadEnd 이벤트에서 자동 히스토리 기록
4. **중복 방지**: 1분 내 동일 URL 재방문 시 중복 기록 안 함
5. **용량 제한**: 최대 5,000개 히스토리, 오래된 항목 자동 삭제
6. **검색 및 삭제**: 제목/URL 검색, 개별/기간별/전체 삭제

### 기대 효과
- 사용자가 과거 방문 페이지를 쉽게 재방문 가능
- 자동 기록으로 북마크 없이도 편리한 히스토리 관리
- 리모컨 최적화 UI로 대화면 환경에서 편리한 탐색

---

## 3. 구현 Phase

### Phase 1: IndexedDB 스키마 및 historyService 구현
**담당**: frontend-dev
**의존성**: F-07 (indexedDBService.js 생성 완료)

#### 태스크
- [ ] **Task 1.1**: indexedDBService.js 확장 (history 오브젝트 스토어 추가)
  - 설명: F-07에서 생성된 indexedDBService.js에 history 오브젝트 스토어 추가
  - 스키마: { id, url, title, favicon, visitedAt, visitCount }
  - 인덱스: url, visitedAt, title, urlVisitedAt (복합 인덱스)
  - 위치: src/services/indexedDBService.js
  - 예상 시간: 30분

- [ ] **Task 1.2**: historyService 기본 CRUD 구현
  - 설명: IndexedDB 기반 히스토리 CRUD 함수 구현
  - 함수: recordVisit(), getAllHistory(), getHistoryById(), deleteHistory()
  - 중복 방지: getRecentVisitByUrl() 함수로 1분 내 동일 URL 체크
  - 위치: src/services/historyService.js
  - 예상 시간: 2시간

- [ ] **Task 1.3**: 용량 제한 및 기간별 삭제 함수 구현
  - 설명: pruneOldHistory(), deleteHistoryByDateRange(), deleteAllHistory() 구현
  - 용량 제한: 최대 5,000개, 오래된 항목부터 삭제
  - 기간별 삭제: startDate ~ endDate 범위 삭제
  - 위치: src/services/historyService.js
  - 예상 시간: 1.5시간

- [ ] **Task 1.4**: 검색 및 그룹화 함수 구현
  - 설명: searchHistory(), groupHistoryByDate() 구현
  - 검색: 제목/URL 부분 일치 필터링 (filter() 사용)
  - 그룹화: "오늘", "어제", "지난 7일", "이번 달", "이전" 그룹으로 분류
  - VirtualList용 평탄화 배열 생성 (날짜 헤더 + 히스토리 항목)
  - 위치: src/services/historyService.js
  - 예상 시간: 1.5시간

#### 산출물
- src/services/indexedDBService.js (수정)
- src/services/historyService.js (신규)

#### 완료 기준
- [ ] 히스토리 추가/조회/삭제 함수가 정상 동작
- [ ] 1분 내 동일 URL 재방문 시 중복 기록 안 함 (visitedAt, visitCount 업데이트)
- [ ] 히스토리 5,000개 초과 시 오래된 항목 자동 삭제
- [ ] 검색 함수가 제목/URL에서 부분 일치 정확히 필터링
- [ ] groupHistoryByDate() 함수가 날짜별 그룹화된 평탄화 배열 반환
- [ ] IndexedDB 오류 처리 (try-catch) 및 로깅 완료

---

### Phase 2: HistoryPanel UI 컴포넌트 구현
**담당**: frontend-dev
**의존성**: Phase 1 완료

#### 태스크
- [ ] **Task 2.1**: HistoryPanel 메인 컴포넌트 구현
  - 설명: 히스토리 목록 패널 컨테이너 (오버레이)
  - Props: visible, onClose, onHistorySelect, onHistoryDeleted
  - 상태 관리: historyItems, groupedHistory, searchQuery, 다이얼로그 상태
  - 데이터 로딩: useEffect로 IndexedDB에서 히스토리 로드
  - 위치: src/components/HistoryPanel/HistoryPanel.js
  - 예상 시간: 2시간

- [ ] **Task 2.2**: HistoryList (VirtualList) 구현
  - 설명: Enact VirtualList 래퍼, 날짜 헤더 + 히스토리 항목 렌더링
  - itemRenderer: type에 따라 DateHeader 또는 HistoryItem 조건부 렌더링
  - itemSize: 날짜 헤더 60px, 히스토리 항목 80px
  - 위치: src/components/HistoryPanel/HistoryList.js
  - 예상 시간: 1.5시간

- [ ] **Task 2.3**: HistoryItem 컴포넌트 구현
  - 설명: 히스토리 항목 카드 (파비콘, 제목, URL, 방문 시각 표시)
  - 이벤트: onClick (페이지 열기), onDelete (삭제)
  - Spotlight 통합: spotlightId, 포커스 표시 스타일
  - 컨텍스트 메뉴: 옵션 버튼 클릭 시 삭제 버튼 표시
  - 위치: src/components/HistoryPanel/HistoryItem.js
  - 예상 시간: 1.5시간

- [ ] **Task 2.4**: DateHeader 컴포넌트 구현
  - 설명: 날짜 그룹 헤더 (📅 "오늘", "어제" 등 표시)
  - Spotlight 설정: data-spotlight-disabled로 포커스 스킵
  - 스타일: 구분선 및 배경색으로 시각적 분리
  - 위치: src/components/HistoryPanel/DateHeader.js
  - 예상 시간: 30분

- [ ] **Task 2.5**: HistorySearchBar 컴포넌트 구현
  - 설명: 히스토리 검색 입력 필드 (가상 키보드 통합)
  - 검색어 입력 시 실시간 필터링
  - 검색어 클리어 버튼
  - 위치: src/components/HistoryPanel/HistorySearchBar.js
  - 예상 시간: 1시간

- [ ] **Task 2.6**: DeleteRangeDialog 구현
  - 설명: 기간별 삭제 다이얼로그 ("지난 1시간", "오늘", "지난 7일" 등)
  - Enact Dialog 사용, 리모컨 방향키로 선택 가능
  - 위치: src/components/HistoryPanel/DeleteRangeDialog.js
  - 예상 시간: 1시간

- [ ] **Task 2.7**: ConfirmDialog 구현
  - 설명: 삭제 확인 다이얼로그 ("히스토리를 삭제하시겠습니까?")
  - 확인/취소 버튼, 백 버튼으로 닫기
  - 위치: src/components/HistoryPanel/ConfirmDialog.js
  - 예상 시간: 30분

- [ ] **Task 2.8**: HistoryPanel 스타일 작성
  - 설명: CSS Modules 기반 스타일 (.module.less)
  - 대화면 최적화: 폰트 크기 최소 20px, 포커스 표시 3px 테두리
  - 패널 너비 600px, 우측 슬라이드 인 애니메이션
  - 위치: src/components/HistoryPanel/HistoryPanel.module.less
  - 예상 시간: 1.5시간

#### 산출물
- src/components/HistoryPanel/HistoryPanel.js
- src/components/HistoryPanel/HistoryList.js
- src/components/HistoryPanel/HistoryItem.js
- src/components/HistoryPanel/DateHeader.js
- src/components/HistoryPanel/HistorySearchBar.js
- src/components/HistoryPanel/DeleteRangeDialog.js
- src/components/HistoryPanel/ConfirmDialog.js
- src/components/HistoryPanel/HistoryPanel.module.less
- src/components/HistoryPanel/index.js

#### 완료 기준
- [ ] HistoryPanel 열기 시 IndexedDB에서 히스토리 로드, 날짜별 그룹화 표시
- [ ] VirtualList로 히스토리 5,000개도 부드러운 스크롤 (30fps 이상)
- [ ] 히스토리 항목 선택 시 페이지 열기 (onHistorySelect 콜백)
- [ ] 리모컨 방향키로 목록 탐색, 선택 버튼으로 실행, 백 버튼으로 패널 닫기
- [ ] 개별 삭제: 삭제 버튼 → 확인 다이얼로그 → 삭제 → 목록 갱신
- [ ] 기간별 삭제: "모두 삭제" 버튼 → 기간 선택 다이얼로그 → 삭제 → 목록 갱신
- [ ] 검색 입력 시 실시간 필터링, 검색 결과도 날짜별 그룹화 유지
- [ ] 빈 히스토리일 경우 "방문 기록이 없습니다" 메시지 표시
- [ ] 대화면 가독성: 폰트 크기, 포커스 표시, 색상 대비 기준 충족

---

### Phase 3: WebView 연동 및 자동 히스토리 기록
**담당**: frontend-dev
**의존성**: Phase 1, Phase 2 완료

#### 태스크
- [ ] **Task 3.1**: BrowserView에서 HistoryPanel 통합
  - 설명: BrowserView에 HistoryPanel 추가, 히스토리 버튼 클릭 시 패널 열기
  - 상태: showHistoryPanel
  - 핸들러: handleHistoryClick, handleHistorySelect
  - JSX: `<HistoryPanel visible={showHistoryPanel} onClose={...} onHistorySelect={...} />`
  - 위치: src/views/BrowserView.js
  - 예상 시간: 1시간

- [ ] **Task 3.2**: WebView onLoadEnd 콜백에서 자동 히스토리 기록
  - 설명: handleLoadEnd 핸들러에서 historyService.recordVisit() 호출
  - 기록 데이터: url, title (document.title)
  - 에러 처리: try-catch로 기록 실패 시 로깅, 사용자 인터페이스 차단 안 함
  - 위치: src/views/BrowserView.js
  - 예상 시간: 30분

- [ ] **Task 3.3**: NavigationBar에 히스토리 버튼 추가
  - 설명: 히스토리 버튼 추가 (icon="list")
  - Props: onHistoryClick
  - Spotlight: spotlightId="nav-history"
  - 위치: src/components/NavigationBar/NavigationBar.js
  - 예상 시간: 30분

#### 산출물
- src/views/BrowserView.js (수정)
- src/components/NavigationBar/NavigationBar.js (수정)

#### 완료 기준
- [ ] 페이지 방문 시 자동으로 히스토리에 기록 (onLoadEnd 이벤트)
- [ ] 동일 URL 1분 내 재방문 시 중복 기록 안 함 (visitedAt, visitCount 업데이트)
- [ ] 히스토리 버튼 클릭 시 HistoryPanel 열림
- [ ] 히스토리 항목 선택 시 WebView에서 페이지 로드 (setCurrentUrl)
- [ ] 페이지 로드 시작 시 로딩 인디케이터 표시 (F-05 연계)
- [ ] 히스토리 기록 실패 시 에러 로깅, 사용자 경험에 영향 없음

---

### Phase 4: URL 자동완성 연동 (선택적)
**담당**: frontend-dev
**의존성**: Phase 1, F-03 (URLBar) 완료
**우선순위**: Should (기본 히스토리 기능 완료 후 추가 가능)

#### 태스크
- [ ] **Task 4.1**: URLBar에서 히스토리 기반 자동완성 제공
  - 설명: 사용자 입력 시 historyService.searchHistory() 호출, 제안 목록 표시
  - 우선순위: 방문 빈도 (visitCount) → 최근 방문 (visitedAt)
  - 제안 항목: 제목, URL, 마지막 방문 시각
  - 최대 10개 제안
  - 위치: src/components/URLBar/URLBar.js
  - 예상 시간: 1.5시간

#### 산출물
- src/components/URLBar/URLBar.js (수정)

#### 완료 기준
- [ ] URLBar 입력 시 히스토리 기반 자동완성 제안 표시
- [ ] 제안 항목: 파비콘 (선택), 제목, URL, 마지막 방문 시각
- [ ] 방문 빈도 및 최근 방문 순으로 우선순위 정렬
- [ ] 방향키로 제안 탐색, Enter로 선택
- [ ] 최대 10개 제안 표시

---

### Phase 5: 리모컨 키 매핑 및 UX 최적화
**담당**: frontend-dev
**의존성**: Phase 2, Phase 3 완료

#### 태스크
- [ ] **Task 5.1**: Spotlight 통합 및 포커스 관리
  - 설명: HistoryPanel의 Spotlight 설정, 초기 포커스를 HistoryList로 이동
  - DateHeader 포커스 스킵 (data-spotlight-disabled)
  - 리모컨 방향키로 히스토리 항목 탐색
  - 위치: src/components/HistoryPanel/HistoryPanel.js
  - 예상 시간: 1시간

- [ ] **Task 5.2**: 리모컨 컬러 버튼 및 옵션 버튼 매핑
  - 설명: 컬러 버튼 (빨강, 노랑, 파랑), 옵션 버튼 이벤트 핸들러
  - 빨강: 히스토리 검색, 노랑: 개별 삭제, 파랑: 모두 삭제
  - 옵션: 컨텍스트 메뉴 열기 (삭제, 북마크 추가)
  - 위치: src/components/HistoryPanel/HistoryPanel.js
  - 예상 시간: 1시간

- [ ] **Task 5.3**: 토스트 메시지 통합
  - 설명: 히스토리 삭제 완료 시 토스트 메시지 표시
  - Enact Notification 사용 ("히스토리가 삭제되었습니다")
  - 위치: src/components/HistoryPanel/HistoryPanel.js
  - 예상 시간: 30분

#### 산출물
- src/components/HistoryPanel/HistoryPanel.js (수정)

#### 완료 기준
- [ ] 리모컨 방향키로 히스토리 목록 탐색, 선택 버튼으로 실행
- [ ] 백 버튼으로 HistoryPanel 닫기
- [ ] 컬러 버튼으로 검색/삭제 기능 실행
- [ ] 옵션 버튼으로 컨텍스트 메뉴 열기
- [ ] 히스토리 삭제 후 토스트 메시지 표시
- [ ] 포커스된 히스토리 항목이 3px 테두리로 명확하게 구분
- [ ] DateHeader는 포커스 스킵, 히스토리 항목만 탐색 가능

---

### Phase 6: 테스트 및 성능 최적화
**담당**: test-runner
**의존성**: Phase 1 ~ 5 완료

#### 태스크
- [ ] **Task 6.1**: historyService 단위 테스트
  - 설명: recordVisit, getAllHistory, deleteHistory, searchHistory, groupHistoryByDate 테스트
  - 테스트 케이스:
    - 히스토리 추가 → IndexedDB에 저장 확인
    - 1분 내 동일 URL 재방문 → visitedAt, visitCount 업데이트 확인
    - 히스토리 5,000개 초과 → pruneOldHistory() 자동 삭제 확인
    - 검색 → 제목/URL 부분 일치 필터링 확인
    - 날짜별 그룹화 → 그룹 개수 및 평탄화 배열 구조 확인
  - 위치: src/services/historyService.test.js
  - 예상 시간: 2시간

- [ ] **Task 6.2**: HistoryPanel 통합 테스트
  - 설명: HistoryPanel 렌더링, 히스토리 선택, 삭제, 검색 테스트
  - 테스트 케이스:
    - HistoryPanel 열기 → 히스토리 목록 표시 확인
    - 히스토리 항목 선택 → onHistorySelect 콜백 호출 확인
    - 개별 삭제 → 목록에서 제거, IndexedDB 삭제 확인
    - 전체 삭제 → 모든 히스토리 삭제 확인
    - 검색 입력 → 실시간 필터링 확인
  - 위치: src/components/HistoryPanel/HistoryPanel.test.js
  - 예상 시간: 2시간

- [ ] **Task 6.3**: 성능 테스트
  - 설명: 히스토리 5,000개 시나리오 성능 측정
  - 측정 항목:
    - IndexedDB 로딩 시간 (1초 이내)
    - groupHistoryByDate() 계산 시간 (100ms 이내)
    - VirtualList 스크롤 프레임레이트 (30fps 이상)
    - 검색 응답 속도 (0.3초 이내)
  - 위치: docs/specs/history-management/performance-test.md
  - 예상 시간: 1.5시간

- [ ] **Task 6.4**: 회귀 테스트
  - 설명: 전체 시나리오 수동 테스트 (webOS 프로젝터 실기 테스트)
  - 테스트 케이스:
    - 페이지 방문 → 히스토리 자동 기록 → 목록에 표시 → 실행 → 페이지 로드
    - 동일 URL 1분 내 재방문 → 중복 기록 안 함
    - 히스토리 5,000개 초과 → 오래된 항목 자동 삭제
    - 앱 재시작 → 히스토리 데이터 정상 로드
    - 리모컨 방향키로 모든 기능 접근 가능
  - 위치: 테스트 체크리스트 (docs/specs/history-management/test-checklist.md)
  - 예상 시간: 2시간

#### 산출물
- src/services/historyService.test.js
- src/components/HistoryPanel/HistoryPanel.test.js
- docs/specs/history-management/performance-test.md
- docs/specs/history-management/test-checklist.md

#### 완료 기준
- [ ] 모든 단위 테스트 통과 (Jest)
- [ ] 통합 테스트 통과 (Enact Testing Library)
- [ ] 성능 기준 충족: 로딩 1초, 그룹화 100ms, 스크롤 30fps, 검색 0.3초
- [ ] 회귀 테스트 체크리스트 100% 통과
- [ ] webOS 프로젝터 실기 테스트 완료 (리모컨 키 매핑 포함)

---

### Phase 7: 문서화 및 커밋
**담당**: doc-writer
**의존성**: Phase 6 완료

#### 태스크
- [ ] **Task 7.1**: HistoryPanel 컴포넌트 문서 작성
  - 설명: Props 인터페이스, 사용 예시, 리모컨 키 매핑
  - 위치: docs/components/HistoryPanel.md
  - 예상 시간: 1시간

- [ ] **Task 7.2**: historyService API 문서 작성
  - 설명: 함수 목록, 파라미터, 반환값, 사용 예시
  - 위치: docs/components/historyService.md
  - 예상 시간: 1시간

- [ ] **Task 7.3**: dev-log.md 업데이트
  - 설명: F-08 히스토리 관리 기능 완료 로그 추가
  - 내용: 구현 내용, 주요 결정 사항, 성능 측정 결과
  - 위치: docs/dev-log.md
  - 예상 시간: 30분

- [ ] **Task 7.4**: CHANGELOG.md 업데이트
  - 설명: F-08 히스토리 관리 기능 추가 항목 기록
  - 위치: CHANGELOG.md
  - 예상 시간: 30분

#### 산출물
- docs/components/HistoryPanel.md
- docs/components/historyService.md
- docs/dev-log.md (수정)
- CHANGELOG.md (수정)

#### 완료 기준
- [ ] HistoryPanel 컴포넌트 문서 작성 (Props, 사용 예시, 리모컨 키 매핑)
- [ ] historyService API 문서 작성 (함수 목록, 파라미터, 반환값)
- [ ] dev-log.md에 F-08 완료 로그 추가
- [ ] CHANGELOG.md에 F-08 변경 내역 추가

---

## 4. 태스크 의존성

### 의존성 다이어그램
```
Phase 1 (historyService)
  │
  ├──▶ Phase 2 (HistoryPanel UI)
  │         │
  │         └──▶ Phase 3 (WebView 연동)
  │                  │
  │                  ├──▶ Phase 4 (URL 자동완성) [선택적]
  │                  │
  │                  └──▶ Phase 5 (리모컨 키 매핑)
  │                           │
  └──────────────────────────┘
                                │
                                ▼
                         Phase 6 (테스트)
                                │
                                ▼
                         Phase 7 (문서화)
```

### 태스크별 의존성
| Task | 의존성 | 병렬 실행 가능 |
|------|--------|---------------|
| Task 1.1 ~ 1.4 | F-07 (indexedDBService) | 순차 실행 |
| Task 2.1 ~ 2.8 | Phase 1 완료 | Task 2.3 ~ 2.7 병렬 가능 |
| Task 3.1 ~ 3.3 | Phase 1, Phase 2 완료 | 순차 실행 |
| Task 4.1 | Phase 1, F-03 완료 | 독립 실행 가능 |
| Task 5.1 ~ 5.3 | Phase 2, Phase 3 완료 | 순차 실행 |
| Task 6.1 ~ 6.4 | Phase 1 ~ 5 완료 | Task 6.1 ~ 6.3 병렬 가능 |
| Task 7.1 ~ 7.4 | Phase 6 완료 | 병렬 가능 |

---

## 5. 병렬 실행 판단

### Agent Team 사용 여부: **NO (순차 실행 권장)**

### 판단 근거
1. **historyService와 HistoryPanel의 강한 결합**:
   - HistoryPanel이 historyService에 의존적 (API 인터페이스 변경 시 충돌 위험)
   - groupHistoryByDate() 반환 구조에 따라 HistoryList 렌더링 로직 결정

2. **F-07 (북마크 관리)와 유사한 패턴**:
   - 북마크 관리도 순차 개발로 완료 (bookmarkService → BookmarkPanel)
   - 같은 패턴 적용으로 일관성 유지

3. **IndexedDB 스키마 변경 충돌**:
   - indexedDBService.js 수정 시 F-07과 충돌 가능성
   - F-07 완료 후 F-08 시작 권장

4. **Phase 2 내부 병렬 가능 (소규모)**:
   - Task 2.3 ~ 2.7 (HistoryItem, DateHeader, SearchBar, Dialog) 독립적
   - 단, VirtualList (Task 2.2) 완료 후 통합 필요
   - 병렬 효율성 낮음 (각 태스크 1시간 미만)

### 병렬 실행 가능한 Phase (선택적)
- **Phase 2 (UI 컴포넌트)**: Task 2.3 ~ 2.7 병렬 가능하지만 효율성 낮음
- **Phase 6 (테스트)**: Task 6.1 ~ 6.3 병렬 가능
- **Phase 7 (문서화)**: Task 7.1 ~ 7.4 병렬 가능

### 권장 실행 방식
- **순차 파이프라인**: `/fullstack-feature "F-08 히스토리 관리"`
- Phase 1 → Phase 2 → Phase 3 → Phase 5 → Phase 6 → Phase 7
- Phase 4 (URL 자동완성)은 F-03 완료 후 별도로 추가 (Should 우선순위)

---

## 6. 예상 소요 시간

### Phase별 예상 시간
| Phase | 예상 시간 | 주요 작업 |
|-------|----------|----------|
| Phase 1 | 5.5시간 | historyService 구현 (CRUD, 중복 방지, 용량 제한, 검색) |
| Phase 2 | 9시간 | HistoryPanel UI (VirtualList, 컴포넌트, 스타일) |
| Phase 3 | 2시간 | WebView 연동 (자동 기록, 히스토리 버튼) |
| Phase 4 | 1.5시간 | URL 자동완성 연동 (선택적) |
| Phase 5 | 2.5시간 | 리모컨 키 매핑, Spotlight, 토스트 |
| Phase 6 | 7.5시간 | 테스트 (단위, 통합, 성능, 회귀) |
| Phase 7 | 3시간 | 문서화 (컴포넌트, 서비스, 로그, CHANGELOG) |
| **합계** | **31시간** | (Phase 4 제외 시 29.5시간) |

### 리스크 반영 예상 시간
- **보수적 추정**: 31시간 × 1.3 (30% 버퍼) = **40시간**
- **예상 일정**: 5일 (8시간/일 기준)

---

## 7. 리스크 및 대응 방안

### 리스크 1: IndexedDB 비동기 처리 복잡도
- **설명**: Promise 기반 IndexedDB API로 async/await 체인 복잡
- **영향**: 개발 시간 증가 (20%), 에러 처리 누락 가능성
- **대응**:
  - F-07에서 구현한 promisifyRequest() 유틸리티 재사용
  - 모든 IndexedDB 작업을 try-catch로 래핑
  - logger.js로 에러 로깅 일관성 유지

### 리스크 2: 히스토리 5,000개 성능 저하
- **설명**: groupHistoryByDate() 계산, VirtualList 렌더링 성능 저하
- **영향**: 목록 로딩 시간 증가, 스크롤 버벅임
- **대응**:
  - VirtualList로 가상 스크롤 (뷰포트만 렌더링)
  - groupHistoryByDate() 결과 캐싱 (검색어 변경 시에만 재계산)
  - pruneOldHistory()로 5,000개 제한 엄격 적용
  - 성능 테스트 (Phase 6)에서 프레임레이트 측정, 기준 미달 시 최적화

### 리스크 3: 중복 방지 로직 (1분 임계값) 정확도
- **설명**: getRecentVisitByUrl() 함수가 복합 인덱스 쿼리 실패 가능성
- **영향**: 중복 히스토리 기록, visitCount 부정확
- **대응**:
  - IndexedDB 복합 인덱스 (urlVisitedAt) 테스트 우선 실행
  - 인덱스 실패 시 전체 히스토리 로드 후 클라이언트 필터링으로 대체
  - 단위 테스트 (Task 6.1)에서 중복 방지 로직 우선 검증

### 리스크 4: F-07 (북마크 관리)와 IndexedDB 충돌
- **설명**: indexedDBService.js 수정 시 F-07과 스키마 충돌
- **영향**: F-07 기능 손상, 데이터 손실 가능성
- **대응**:
  - F-07 완료 후 F-08 시작 (의존성 명시)
  - indexedDBService.js 수정 시 F-07 회귀 테스트 필수
  - DB_VERSION은 F-07과 동일 (1) 유지, onupgradeneeded에서 스토어 존재 체크

### 리스크 5: WebView CORS 제약 (제목 추출 실패)
- **설명**: Cross-Origin 페이지의 document.title 접근 불가
- **영향**: 히스토리 제목이 URL로 표시 (가독성 저하)
- **대응**:
  - try-catch로 CORS 에러 무시, 제목 추출 실패 시 URL을 제목으로 사용
  - WebView.js의 extractTitle() 함수 활용 (F-02에서 이미 구현)
  - 파비콘 다운로드는 선택적 기능으로 우선순위 낮춤

### 리스크 6: F-03 (URLBar) 미완료 시 Phase 4 지연
- **설명**: URL 자동완성 연동은 F-03 완료 필수
- **영향**: Phase 4 시작 지연, 전체 일정 영향
- **대응**:
  - Phase 4는 선택적 기능 (Should 우선순위)
  - Phase 1 ~ 3 완료 후 먼저 F-08 기본 기능 검증
  - F-03 완료 후 Phase 4를 별도로 추가

---

## 8. 검증 계획

### 단위 테스트 (Jest)
- **대상**: historyService 모든 함수
- **테스트 케이스**:
  - recordVisit(): 히스토리 추가, 중복 방지 (1분 내 재방문), visitCount 증가
  - getAllHistory(): 전체 조회, visitedAt 역순 정렬
  - deleteHistory(): 개별 삭제, IndexedDB 반영 확인
  - deleteHistoryByDateRange(): 기간별 삭제, 개수 확인
  - deleteAllHistory(): 전체 삭제, 빈 배열 반환
  - pruneOldHistory(): 5,000개 초과 시 오래된 항목 삭제
  - searchHistory(): 제목/URL 부분 일치 필터링
  - groupHistoryByDate(): 날짜별 그룹화, 평탄화 배열 구조 확인

### 통합 테스트 (Enact Testing Library)
- **대상**: HistoryPanel 컴포넌트
- **테스트 케이스**:
  - HistoryPanel 열기 → 히스토리 목록 표시
  - 히스토리 항목 클릭 → onHistorySelect 콜백 호출
  - 개별 삭제 → 확인 다이얼로그 → 삭제 → 목록 갱신
  - 전체 삭제 → 확인 다이얼로그 → 삭제 → 빈 메시지 표시
  - 검색 입력 → 실시간 필터링 → 검색 결과 표시
  - 백 버튼 → 패널 닫기

### 성능 테스트
- **시나리오**: 히스토리 5,000개 생성 후 성능 측정
- **측정 항목**:
  - IndexedDB 로딩 시간 (getAllHistory): **목표 1초 이내**
  - groupHistoryByDate() 계산 시간: **목표 100ms 이내**
  - VirtualList 스크롤 프레임레이트: **목표 30fps 이상**
  - searchHistory() 응답 속도: **목표 0.3초 이내**
  - pruneOldHistory() 실행 시간: **목표 0.5초 이내**

### 회귀 테스트 (수동 테스트)
- **환경**: webOS 프로젝터 실기 테스트 (LG HU175QW)
- **테스트 체크리스트**:
  - [ ] 페이지 방문 → 히스토리 자동 기록 → 목록에 표시
  - [ ] 동일 URL 1분 내 재방문 → 중복 기록 안 함, visitCount 증가
  - [ ] 히스토리 선택 → 페이지 로드, 로딩 인디케이터 표시
  - [ ] 개별 삭제 → 목록에서 제거, 재시작 후 삭제 유지
  - [ ] 기간별 삭제 ("지난 7일") → 해당 기간 히스토리만 삭제
  - [ ] 전체 삭제 → 모든 히스토리 삭제, 빈 메시지 표시
  - [ ] 검색 입력 → 실시간 필터링, 결과 날짜별 그룹화 유지
  - [ ] 히스토리 5,000개 초과 → 오래된 항목 자동 삭제
  - [ ] 앱 재시작 → 히스토리 데이터 정상 로드
  - [ ] 리모컨 방향키로 목록 탐색, 선택 버튼으로 실행, 백 버튼으로 닫기
  - [ ] 컬러 버튼 (빨강, 노랑, 파랑) 기능 동작
  - [ ] DateHeader는 포커스 스킵, 히스토리 항목만 탐색

### 가독성 검증 (대화면 환경)
- **체크리스트**:
  - [ ] 폰트 크기: 히스토리 제목 20px, URL 16px, 날짜 헤더 22px
  - [ ] 포커스 표시: 3px 테두리, 명확한 배경색 변화
  - [ ] 색상 대비: 배경/전경 대비 비율 4.5:1 이상
  - [ ] 3m 거리에서 모든 텍스트 가독 가능

---

## 9. 완료 기준 (Definition of Done)

### 기능 완료 기준
- [ ] 페이지 방문 시 자동으로 히스토리에 기록 (onLoadEnd)
- [ ] 동일 URL 1분 내 재방문 시 중복 기록 안 함 (visitedAt, visitCount 업데이트)
- [ ] HistoryPanel에서 날짜별 그룹화된 히스토리 목록 표시
- [ ] 히스토리 항목 선택 시 WebView에서 페이지 로드
- [ ] 개별 삭제, 기간별 삭제, 전체 삭제 기능 동작
- [ ] 검색 입력 시 실시간 필터링, 날짜별 그룹화 유지
- [ ] 히스토리 5,000개 초과 시 오래된 항목 자동 삭제
- [ ] 앱 재시작 후 IndexedDB에서 히스토리 데이터 정상 로드

### 리모컨 UX 완료 기준
- [ ] 리모컨 방향키로 히스토리 목록 탐색 가능
- [ ] 선택 버튼으로 히스토리 실행
- [ ] 백 버튼으로 HistoryPanel 닫기
- [ ] 컬러 버튼으로 검색/삭제 기능 실행
- [ ] DateHeader는 포커스 스킵, 히스토리 항목만 탐색
- [ ] 포커스된 히스토리 항목이 3px 테두리로 명확하게 구분

### 성능 완료 기준
- [ ] 히스토리 로딩 시간 1초 이내
- [ ] groupHistoryByDate() 계산 시간 100ms 이내
- [ ] VirtualList 스크롤 프레임레이트 30fps 이상
- [ ] 검색 응답 속도 0.3초 이내

### 테스트 완료 기준
- [ ] historyService 단위 테스트 100% 통과 (Jest)
- [ ] HistoryPanel 통합 테스트 100% 통과 (Enact Testing Library)
- [ ] 성능 테스트 모든 기준 충족
- [ ] 회귀 테스트 체크리스트 100% 통과
- [ ] webOS 프로젝터 실기 테스트 완료

### 문서화 완료 기준
- [ ] HistoryPanel 컴포넌트 문서 작성 (Props, 사용 예시, 리모컨 키 매핑)
- [ ] historyService API 문서 작성 (함수 목록, 파라미터, 반환값)
- [ ] dev-log.md에 F-08 완료 로그 추가
- [ ] CHANGELOG.md에 F-08 변경 내역 추가

### 코드 품질 완료 기준
- [ ] ESLint 통과 (경고 없음)
- [ ] 모든 함수에 JSDoc 주석 작성
- [ ] PropTypes로 타입 검증 (HistoryPanel, HistoryItem 등)
- [ ] 에러 처리 (try-catch) 및 로깅 (logger.js) 일관성 유지
- [ ] 코드 리뷰 완료 (code-reviewer)

---

## 10. 참고 사항

### 유사 기능 참조
- **F-07 (북마크 관리)**: bookmarkService, BookmarkPanel 패턴 참조
- **F-02 (웹뷰 통합)**: onLoadEnd 이벤트, extractTitle() 함수 활용
- **F-03 (URL 입력 UI)**: 자동완성 통합 패턴 참조
- **F-05 (로딩 인디케이터)**: 히스토리 실행 시 로딩 표시

### IndexedDB 참고 자료
- IndexedDB API: https://developer.mozilla.org/en-US/docs/Web/API/IndexedDB_API
- IndexedDB 인덱스: https://developer.mozilla.org/en-US/docs/Web/API/IDBObjectStore/createIndex
- 복합 인덱스: https://developer.mozilla.org/en-US/docs/Web/API/IDBObjectStore/createIndex#using_array_keypaths

### Enact 참고 자료
- VirtualList: https://enactjs.com/docs/modules/moonstone/VirtualList/
- Panels: https://enactjs.com/docs/modules/moonstone/Panels/
- Dialog: https://enactjs.com/docs/modules/moonstone/Dialog/
- Spotlight: https://enactjs.com/docs/modules/spotlight/Spotlight/

### 날짜/시간 포맷 참고
- Date/Time Formatting: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/DateTimeFormat

---

## 11. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-13 | 최초 작성 | product-manager |
