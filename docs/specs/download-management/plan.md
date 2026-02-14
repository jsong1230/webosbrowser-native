# 다운로드 관리 기능 — 구현 계획서

## 1. 참조 문서
- 요구사항 분석서: [docs/specs/download-management/requirements.md](./requirements.md)
- 기술 설계서: [docs/specs/download-management/design.md](./design.md)

---

## 2. 구현 Phase

### Phase 1: 기반 서비스 구현 (Backend Logic)
**담당**: frontend-dev (프론트엔드 전용 프로젝트)

#### Task 1-1: 유틸리티 함수 구현
- **파일**: `src/utils/downloadHelper.js` (신규)
- **작업 내용**:
  - `extractFilename(url)`: URL에서 파일명 추출
  - `formatFileSize(bytes)`: 바이트를 MB/GB로 포맷팅 (예: "10.5 MB")
  - `formatSpeed(bytesPerSecond)`: 다운로드 속도 포맷팅 (예: "1.2 MB/s")
  - `calculateRemainingTime(downloadedBytes, totalBytes, speed)`: 남은 시간 추정 (초)
  - `formatTime(seconds)`: 초를 "약 X초 남음" 형식으로 변환
  - `isValidDownloadUrl(url)`: URL 유효성 검증 (http/https만 허용)
  - `getMimeType(filename)`: 파일 확장자로 MIME 타입 추측
- **완료 기준**: 단위 테스트 작성 및 통과
- **예상 소요 시간**: 1시간

#### Task 1-2: IndexedDB 스키마 확장
- **파일**: `src/services/indexedDBService.js` (수정)
- **작업 내용**:
  - DB 버전 업그레이드 (v1 → v2)
  - 새 Object Store 생성: `downloads`
    - Key Path: `id` (UUID)
    - 인덱스: `status`, `updatedAt`
  - Object Store 컬럼 정의 (설계서 4장 참조):
    - id, url, filename, fileSize, downloadedBytes, status, speed, remainingTime, errorMessage, blob, createdAt, updatedAt, completedAt
  - Migration 로직: 기존 bookmarks, history Object Store는 유지
- **완료 기준**: DB 버전 업그레이드 시 에러 없이 새 Object Store 생성 확인
- **예상 소요 시간**: 1시간

#### Task 1-3: downloadService 핵심 로직 구현
- **파일**: `src/services/downloadService.js` (신규)
- **작업 내용**:
  - `startDownload(url, filename, options)` 구현:
    - 동시 다운로드 수 체크 (최대 5개)
    - Fetch API 요청 시작
    - ReadableStream으로 진행률 모니터링
    - Content-Length 헤더로 파일 크기 파악
    - 1초마다 `onProgress` 콜백 호출 (진행률, 속도, 남은 시간)
    - 다운로드 완료 시 Blob으로 저장
    - IndexedDB에 메타데이터 + Blob 저장
    - `onComplete` 콜백 호출
  - `pauseDownload(id)` 구현:
    - AbortController로 Fetch 요청 취소
    - 다운로드된 바이트 수를 IndexedDB에 저장 (resumePosition)
    - 상태를 `paused`로 변경
  - `resumeDownload(id)` 구현:
    - IndexedDB에서 resumePosition 조회
    - HTTP Range 헤더로 Fetch 요청 재시작 (`Range: bytes={resumePosition}-`)
    - 서버 206 응답 확인 → 이어서 다운로드
    - 서버 Range 미지원 시 처음부터 재다운로드
  - `cancelDownload(id)` 구현:
    - AbortController로 Fetch 요청 취소
    - IndexedDB에서 다운로드 데이터 삭제
    - 상태를 `cancelled`로 변경
  - `deleteDownload(id)` 구현:
    - IndexedDB에서 다운로드 레코드 삭제 (메타데이터 + Blob)
  - `getDownloadHistory()` 구현:
    - IndexedDB에서 모든 다운로드 조회, `updatedAt` 역순 정렬
  - `getActiveDownloads()` 구현:
    - 상태가 `inProgress` 또는 `paused`인 다운로드만 필터링
  - `openDownloadedFile(id)` 구현 (선택 사항):
    - IndexedDB에서 Blob 조회
    - URL.createObjectURL()로 Blob URL 생성
    - window.open() 또는 iframe으로 파일 열기
- **완료 기준**: 단위 테스트 작성 (mock Fetch API), 정상 다운로드/일시정지/재개/취소 시나리오 통과
- **예상 소요 시간**: 4시간

---

### Phase 2: 상태 관리 구현 (Context)
**담당**: frontend-dev

#### Task 2-1: DownloadContext 구현
- **파일**: `src/contexts/DownloadContext.js` (신규)
- **작업 내용**:
  - Context 생성 (`DownloadContext`)
  - Provider 컴포넌트 (`DownloadProvider`)
  - Reducer 구현:
    - Actions: `ADD_DOWNLOAD`, `UPDATE_DOWNLOAD`, `REMOVE_DOWNLOAD`, `START_DOWNLOAD`, `PAUSE_DOWNLOAD`, `RESUME_DOWNLOAD`, `COMPLETE_DOWNLOAD`, `FAIL_DOWNLOAD`, `CANCEL_DOWNLOAD`
    - Reducer 로직: 설계서 6장 참조
  - 초기 상태:
    - `downloads: []`
    - `activeDownloadCount: 0`
  - 앱 시작 시 IndexedDB에서 기존 다운로드 복원 (진행 중인 다운로드는 `paused` 상태로 전환)
- **완료 기준**: Context Provider로 BrowserView를 감싸고, Context 값이 정상적으로 공유되는지 확인
- **예상 소요 시간**: 2시간

---

### Phase 3: UI 컴포넌트 구현 (Frontend)
**담당**: frontend-dev

#### Task 3-1: DownloadPanel 메인 컴포넌트 구현
- **파일**: `src/components/DownloadPanel/DownloadPanel.js` (신규)
- **작업 내용**:
  - Moonstone Panel 컴포넌트 사용
  - Props: `visible`, `onClose`
  - 상태:
    - `activeTab`: "진행 중" | "완료" | "전체"
  - DownloadContext에서 `downloads` 가져오기
  - 탭 전환 UI (TabLayout 사용)
  - 탭별 다운로드 필터링:
    - "진행 중": `status === 'inProgress' || status === 'paused'`
    - "완료": `status === 'completed'`
    - "전체": 모든 다운로드
  - Spotlight 설정:
    - 패널 열 때: 첫 번째 다운로드 항목으로 포커스
    - 리모컨 Back 키: 패널 닫기
    - 리모컨 좌/우 방향키: 탭 전환
  - DownloadList 컴포넌트 렌더링
- **완료 기준**: 다운로드 패널이 표시되고, 탭 전환이 리모컨으로 가능한지 확인
- **예상 소요 시간**: 2시간

#### Task 3-2: DownloadList 컴포넌트 구현
- **파일**: `src/components/DownloadPanel/DownloadList.js` (신규)
- **작업 내용**:
  - Props: `downloads`, `onPause`, `onResume`, `onCancel`, `onDelete`, `onOpen`, `onRetry`
  - VirtualList 사용 (성능 최적화, 다운로드가 많을 경우 대비)
  - 다운로드가 없을 경우 빈 상태 메시지 표시 ("다운로드 내역이 없습니다")
  - DownloadItem 컴포넌트를 리스트 항목으로 렌더링
  - Spotlight 포커스: 위/아래 방향키로 항목 간 이동
- **완료 기준**: 다운로드 목록이 리스트로 표시되고, 리모컨으로 항목 간 이동 가능한지 확인
- **예상 소요 시간**: 1.5시간

#### Task 3-3: DownloadItem 컴포넌트 구현
- **파일**: `src/components/DownloadItem/DownloadItem.js` (신규)
- **작업 내용**:
  - Props: `download`, `onPause`, `onResume`, `onCancel`, `onDelete`, `onOpen`, `onRetry`
  - 렌더링 정보:
    - 파일명 (최대 40자, 긴 파일명은 말줄임 `...`)
    - 파일 크기 (formatFileSize 사용)
    - 상태 아이콘:
      - 진행 중: 파란색 회전 스피너 아이콘
      - 일시정지: 회색 일시정지 아이콘
      - 완료: 녹색 체크 아이콘
      - 실패: 빨간색 경고 아이콘
    - 진행 중일 때만:
      - 프로그레스바 (Moonstone ProgressBar, 0-1 값)
      - 퍼센트 (예: "45%")
      - 다운로드 속도 (formatSpeed 사용, 예: "1.2 MB/s")
      - 남은 시간 (formatTime 사용, 예: "약 30초 남음")
    - 액션 버튼 (상태별 다르게 표시):
      - 진행 중: "일시정지" 버튼, "취소" 버튼
      - 일시정지: "재개" 버튼, "취소" 버튼
      - 완료: "열기" 버튼, "삭제" 버튼
      - 실패: "재시도" 버튼, "삭제" 버튼
  - Spotlightable 설정: 항목 전체를 Spotlightable 컴포넌트로 래핑
  - 선택 버튼 클릭 시: 첫 번째 액션 버튼에 포커스 이동
- **완료 기준**: 다운로드 상태별로 올바른 UI가 표시되는지 확인, 리모컨으로 버튼 실행 가능한지 확인
- **예상 소요 시간**: 3시간

#### Task 3-4: DownloadConfirmDialog 컴포넌트 구현
- **파일**: `src/components/DownloadPanel/DownloadConfirmDialog.js` (신규)
- **작업 내용**:
  - Props: `visible`, `url`, `filename`, `fileSize`, `onConfirm`, `onCancel`
  - Moonstone Dialog 컴포넌트 사용 (`type="alert"`)
  - 다이얼로그 내용:
    - 제목: "다운로드 하시겠습니까?"
    - 파일명 표시
    - 파일 크기 표시 (알 수 없으면 "크기: 알 수 없음")
    - URL 표시 (최대 60자, 긴 URL은 말줄임)
    - HTTP URL일 경우 보안 경고 메시지 표시 (빨간색 텍스트): "⚠ 이 사이트는 보안 연결이 아닙니다 (HTTP). HTTPS 사이트를 사용하는 것을 권장합니다."
  - 버튼: "확인", "취소"
  - Spotlight 포커스: "확인" 버튼에 기본 포커스
- **완료 기준**: 다운로드 시작 전 확인 다이얼로그가 표시되고, 확인/취소 동작이 정상적으로 작동하는지 확인
- **예상 소요 시간**: 1.5시간

#### Task 3-5: 스타일링 (CSS Modules)
- **파일**:
  - `src/components/DownloadPanel/DownloadPanel.module.less` (신규)
  - `src/components/DownloadPanel/DownloadItem.module.less` (신규)
  - `src/components/DownloadPanel/DownloadConfirmDialog.module.less` (신규)
- **작업 내용**:
  - 설계서 11장 UI/UX 설계 참조
  - 대화면 최적화:
    - 파일명 텍스트: 24px, 볼드, #FFFFFF
    - 진행률 텍스트: 20px, #CCCCCC
    - 상태 텍스트: 20px, 상태별 색상 (진행 중: #4A90E2, 완료: #7ED321, 실패: #D0021B)
    - 프로그레스바: 높이 10px, 파란색 (#4A90E2)
    - 다운로드 항목 간 여백: 20px
    - 액션 버튼: 120px × 48px, 좌우 간격 8px
  - Spotlight 포커스 스타일:
    - 포커스된 항목: 두꺼운 파란색 테두리 (4px)
    - 포커스된 버튼: 밝은 파란색 배경 (#5A9FE2)
  - 애니메이션:
    - 프로그레스바 이동 애니메이션 (transition: 0.5s ease)
    - 진행 중 아이콘 회전 애니메이션 (spin)
- **완료 기준**: 대화면(3m 거리)에서 텍스트 가독 가능, 포커스 상태가 명확히 보임
- **예상 소요 시간**: 2시간

---

### Phase 4: BrowserView 통합 (Integration)
**담당**: frontend-dev

#### Task 4-1: BrowserView에 다운로드 패널 통합
- **파일**: `src/views/BrowserView.js` (수정)
- **작업 내용**:
  - 상태 추가:
    - `showDownloadPanel`: 다운로드 패널 표시 여부
    - `downloadRequest`: 다운로드 확인 다이얼로그에 전달할 다운로드 요청 정보 (url, filename, fileSize)
  - DownloadPanel 컴포넌트 렌더링 (오버레이 형태)
  - 다운로드 버튼 클릭 핸들러 `handleDownloadClick()`:
    - 현재 페이지 URL을 다운로드 요청으로 설정
    - DownloadConfirmDialog 표시
  - 다운로드 확인 핸들러 `handleDownloadConfirm(request)`:
    - downloadService.startDownload() 호출
    - onProgress 콜백에서 DownloadContext.dispatch(UPDATE_DOWNLOAD) 호출
    - onComplete 콜백에서 DownloadContext.dispatch(COMPLETE_DOWNLOAD) 호출, 토스트 알림 표시
    - onError 콜백에서 DownloadContext.dispatch(FAIL_DOWNLOAD) 호출
  - 다운로드 패널 열기 핸들러 `handleOpenDownloadPanel()`:
    - `setShowDownloadPanel(true)`
  - 다운로드 패널 닫기 핸들러 `handleCloseDownloadPanel()`:
    - `setShowDownloadPanel(false)`
- **완료 기준**: BrowserView에서 다운로드 버튼 클릭 시 다운로드 확인 다이얼로그 표시, 확인 후 다운로드 시작, 패널에서 진행률 확인 가능
- **예상 소요 시간**: 2시간

#### Task 4-2: NavigationBar에 다운로드 버튼 추가
- **파일**: `src/components/NavigationBar/NavigationBar.js` (수정)
- **작업 내용**:
  - Props 추가: `onDownload`, `showDownloadButton`
  - 다운로드 버튼 추가 (Moonstone IconButton, icon="download")
  - 버튼 클릭 시 `onDownload` 콜백 호출
  - Spotlight 포커스 순서: 기존 버튼들 뒤에 다운로드 버튼 추가
  - 레이아웃 조정: 버튼 간 간격 유지 (가로 공간 확보)
- **완료 기준**: NavigationBar에 다운로드 버튼이 표시되고, 리모컨으로 포커스 및 클릭 가능한지 확인
- **예상 소요 시간**: 1시간

#### Task 4-3: 리모컨 다운로드 패널 단축키 추가
- **파일**: `src/hooks/useRemoteControl.js` (수정)
- **작업 내용**:
  - 리모컨 컬러 버튼 중 노란색 버튼을 다운로드 패널 단축키로 매핑
  - 노란색 버튼 클릭 시 `showDownloadPanel` 상태 토글
  - 키 코드 확인: webOS 리모컨 노란색 버튼 키 코드 (예: `Yellow` 또는 `406`)
- **완료 기준**: 리모컨 노란색 버튼 클릭 시 다운로드 패널이 열리고 닫히는지 확인
- **예상 소요 시간**: 0.5시간

#### Task 4-4: 백그라운드 다운로드 지속성 구현
- **파일**: `src/services/downloadService.js` (수정)
- **작업 내용**:
  - 다운로드는 DownloadContext에서 관리되므로 DownloadPanel이 닫혀도 계속 진행
  - 글로벌 다운로드 인디케이터 추가 (BrowserView 상단 바에 작은 아이콘):
    - 진행 중인 다운로드가 있을 때 아이콘 표시 (예: 파란색 화살표 아이콘)
    - 클릭 시 다운로드 패널 열기
  - 다운로드 완료 시 토스트 알림 표시 (3초간 표시 후 자동 사라짐)
    - Moonstone Notification 컴포넌트 사용
    - 메시지: "다운로드 완료: {파일명}"
- **완료 기준**: 다운로드 패널 닫아도 다운로드가 계속 진행되는지 확인, 글로벌 인디케이터 표시 확인, 완료 알림 표시 확인
- **예상 소요 시간**: 1.5시간

---

### Phase 5: 테스트 작성
**담당**: test-runner

#### Task 5-1: 단위 테스트 작성
- **파일**:
  - `src/utils/downloadHelper.test.js` (신규)
  - `src/services/downloadService.test.js` (신규)
  - `src/contexts/DownloadContext.test.js` (신규)
- **작업 내용**:
  - downloadHelper 유틸리티 함수 테스트:
    - extractFilename() 다양한 URL 패턴 테스트
    - formatFileSize(), formatSpeed(), formatTime() 경계값 테스트
    - isValidDownloadUrl() 유효/무효 URL 테스트
  - downloadService 테스트:
    - startDownload() 정상 다운로드 시나리오 (mock Fetch API)
    - pauseDownload() 일시정지 동작
    - resumeDownload() Range 헤더로 재개 (206 응답 mock)
    - cancelDownload() 다운로드 취소
    - 동시 다운로드 5개 초과 시 에러 처리
    - 네트워크 오류 시 에러 처리
  - DownloadContext reducer 테스트:
    - ADD_DOWNLOAD, UPDATE_DOWNLOAD, COMPLETE_DOWNLOAD 액션 처리
    - 중복 다운로드 ID 방지
- **완료 기준**: Jest 테스트 실행 시 모든 단위 테스트 통과
- **예상 소요 시간**: 3시간

#### Task 5-2: 통합 테스트 작성
- **파일**: `src/components/DownloadPanel/DownloadPanel.test.js` (신규)
- **작업 내용**:
  - 다운로드 시작 → 완료 플로우 테스트:
    - DownloadPanel에 항목 추가 확인
    - 진행률 업데이트 확인 (mock timer)
    - 완료 후 토스트 알림 표시 확인
    - IndexedDB에 Blob 저장 확인 (mock IndexedDB)
  - 일시정지 → 재개 플로우 테스트:
    - 일시정지 버튼 클릭 시 상태 변경 확인
    - 재개 버튼 클릭 시 다운로드 재시작 확인
  - 네트워크 오류 시 재시도 테스트:
    - 실패 상태 전환 확인
    - 재시도 버튼 클릭 시 다운로드 재시작 확인
  - 동시 다운로드 5개 제한 테스트:
    - 6번째 다운로드 시작 시 에러 메시지 표시 확인
- **완료 기준**: Enact Testing Library로 통합 테스트 실행 시 모든 시나리오 통과
- **예상 소요 시간**: 3시간

#### Task 5-3: E2E 테스트 작성 (선택 사항, 수동 테스트로 대체 가능)
- **작업 내용**:
  - 리모컨으로 다운로드 패널 열기 테스트 (노란색 버튼)
  - 리모컨으로 다운로드 조작 테스트 (위/아래 방향키, 선택 버튼)
  - 백그라운드 다운로드 테스트 (패널 닫고 다운로드 계속 진행 확인)
- **완료 기준**: webOS 프로젝터 실기기에서 수동 테스트 완료
- **예상 소요 시간**: 2시간 (수동 테스트)

---

### Phase 6: 코드 + 문서 리뷰
**담당**: code-reviewer

#### Task 6-1: 코드 리뷰
- **작업 내용**:
  - downloadService.js 코드 품질 검토:
    - Fetch API 에러 처리 누락 확인
    - AbortController 메모리 누수 확인
    - IndexedDB 트랜잭션 에러 처리 확인
  - DownloadPanel 컴포넌트 코드 품질 검토:
    - Spotlight 포커스 플로우 검증
    - PropTypes 정의 확인
    - 성능 최적화 (VirtualList 사용 확인)
  - ESLint 규칙 위반 확인
  - 주석 누락 확인 (복잡한 로직은 한글 주석 필수)
- **완료 기준**: 코드 리뷰 피드백 수정 완료
- **예상 소요 시간**: 2시간

#### Task 6-2: 사전 문서 검증
- **작업 내용**:
  - requirements.md, design.md, plan.md가 실제 구현과 일치하는지 확인
  - 변경 사항이 있을 경우 문서 업데이트
- **완료 기준**: 문서와 코드 일치 확인
- **예상 소요 시간**: 1시간

---

### Phase 7: 사후 문서 작성 (기술 문서)
**담당**: frontend-dev

#### Task 7-1: 컴포넌트 문서 작성
- **파일**:
  - `docs/components/DownloadPanel.md` (신규)
  - `docs/components/DownloadItem.md` (신규)
  - `docs/components/DownloadConfirmDialog.md` (신규)
- **작업 내용**:
  - 각 컴포넌트별 문서 작성:
    - Props 인터페이스
    - 사용 예시 코드
    - 리모컨 키 매핑
    - 상태 관리 방법
    - 주의사항
- **완료 기준**: 다른 개발자가 문서만 보고 컴포넌트를 사용할 수 있을 정도로 상세히 작성
- **예상 소요 시간**: 2시간

#### Task 7-2: 서비스 문서 작성
- **파일**: `docs/components/downloadService.md` (신규)
- **작업 내용**:
  - downloadService API 문서:
    - 각 함수의 파라미터, 반환값, 예외
    - 사용 예시 코드
    - Fetch API와 IndexedDB 통합 방법
    - HTTP Range 요청 주의사항
- **완료 기준**: API 문서 작성 완료
- **예상 소요 시간**: 1시간

---

### Phase 8: 마무리
**담당**: doc-writer

#### Task 8-1: 개발 진행 로그 업데이트
- **파일**: `docs/dev-log.md` (수정)
- **작업 내용**:
  - F-12 다운로드 관리 기능 완료 기록
  - 작업 기간, 주요 변경 사항, 이슈 및 해결 방법 기록
- **완료 기준**: dev-log.md 업데이트 완료
- **예상 소요 시간**: 0.5시간

#### Task 8-2: CHANGELOG 업데이트
- **파일**: `CHANGELOG.md` (수정)
- **작업 내용**:
  - 버전 업데이트 (예: v0.3.0)
  - 새 기능: F-12 다운로드 관리 추가
  - 변경 사항: NavigationBar에 다운로드 버튼 추가, IndexedDB 스키마 v2 업그레이드
- **완료 기준**: CHANGELOG.md 업데이트 완료
- **예상 소요 시간**: 0.5시간

#### Task 8-3: features.md 상태 업데이트
- **파일**: `docs/project/features.md` (수정)
- **작업 내용**:
  - F-12 상태를 "진행중 🔄" → "완료 ✅"로 변경
- **완료 기준**: features.md 업데이트 완료
- **예상 소요 시간**: 0.5시간

---

## 3. 태스크 의존성

```
Phase 1: 기반 서비스 구현
├─ Task 1-1: downloadHelper 유틸리티 (병렬 가능)
├─ Task 1-2: IndexedDB 스키마 확장 (병렬 가능)
└─ Task 1-3: downloadService 구현 (의존: Task 1-1, 1-2)

Phase 2: 상태 관리
└─ Task 2-1: DownloadContext 구현 (의존: Phase 1 완료)

Phase 3: UI 컴포넌트
├─ Task 3-1: DownloadPanel (의존: Phase 2)
│   └─ Task 3-2: DownloadList (의존: Task 3-1)
│       └─ Task 3-3: DownloadItem (의존: Task 3-2)
├─ Task 3-4: DownloadConfirmDialog (병렬 가능, Phase 2 의존)
└─ Task 3-5: 스타일링 (의존: Task 3-1, 3-2, 3-3, 3-4)

Phase 4: BrowserView 통합
├─ Task 4-1: BrowserView 통합 (의존: Phase 3)
├─ Task 4-2: NavigationBar 버튼 추가 (의존: Task 4-1)
├─ Task 4-3: 리모컨 단축키 추가 (병렬 가능, Phase 3 의존)
└─ Task 4-4: 백그라운드 다운로드 (의존: Task 4-1)

Phase 5: 테스트
├─ Task 5-1: 단위 테스트 (병렬 가능, Phase 1, 2 의존)
├─ Task 5-2: 통합 테스트 (의존: Phase 4)
└─ Task 5-3: E2E 테스트 (의존: Phase 4)

Phase 6: 리뷰
├─ Task 6-1: 코드 리뷰 (의존: Phase 5)
└─ Task 6-2: 사전 문서 검증 (병렬 가능, Phase 5 의존)

Phase 7: 사후 문서
├─ Task 7-1: 컴포넌트 문서 (병렬 가능, Phase 6 의존)
└─ Task 7-2: 서비스 문서 (병렬 가능, Phase 6 의존)

Phase 8: 마무리
├─ Task 8-1: dev-log 업데이트 (의존: Phase 7)
├─ Task 8-2: CHANGELOG 업데이트 (병렬 가능)
└─ Task 8-3: features.md 업데이트 (병렬 가능)
```

---

## 4. 병렬 실행 판단

### 병렬 배치 (PG-3) 분석
**features.md 기준**: F-12 (다운로드 관리), F-13 (리모컨 단축키), F-14 (HTTPS 보안 표시)

#### F-12 (다운로드 관리)
- **충돌 영역**:
  - `src/services/downloadService.js` (신규)
  - `src/components/DownloadPanel/` (신규)
  - `src/contexts/DownloadContext.js` (신규)
  - `src/services/indexedDBService.js` (수정 - downloads Object Store 추가)
  - `src/views/BrowserView.js` (수정 - DownloadPanel 통합)
  - `src/components/NavigationBar/NavigationBar.js` (수정 - 다운로드 버튼 추가)
  - `src/hooks/useRemoteControl.js` (수정 - 노란색 버튼 매핑)

#### F-13 (리모컨 단축키)
- **충돌 영역**:
  - `src/hooks/useRemoteControl.js` (수정 - 다양한 단축키 매핑)
  - `src/views/BrowserView.js` (수정 - 단축키 핸들러 연결)

#### F-14 (HTTPS 보안 표시)
- **충돌 영역**:
  - `src/components/URLBar/URLBar.js` (수정 - 보안 아이콘 추가)
  - `src/components/SecurityIndicator/` (신규)
  - `src/views/BrowserView.js` (수정 - 보안 상태 관리)

### 충돌 분석
| 파일 | F-12 | F-13 | F-14 | 충돌 여부 |
|------|------|------|------|----------|
| `src/views/BrowserView.js` | ✓ (DownloadPanel 통합) | ✓ (단축키 핸들러) | ✓ (보안 상태 관리) | **충돌 가능** |
| `src/hooks/useRemoteControl.js` | ✓ (노란색 버튼) | ✓ (다양한 단축키) | - | **충돌 가능** |
| `src/components/NavigationBar/NavigationBar.js` | ✓ (다운로드 버튼) | - | - | 충돌 없음 |
| `src/components/URLBar/URLBar.js` | - | - | ✓ (보안 아이콘) | 충돌 없음 |

### 결론: **병렬 실행 권장하지 않음 (순차 실행 권장)**

**근거**:
- `BrowserView.js`가 3개 기능 모두에서 수정 대상이므로 머지 충돌 가능성 높음
- `useRemoteControl.js`가 F-12와 F-13에서 수정 대상 (리모컨 키 매핑 충돌)
- F-12가 가장 복잡한 기능 (High 복잡도)으로 우선 완료 후 F-13, F-14를 순차 진행하는 것이 안전

### 대안: Agent Team 사용 불가
- 병렬 배치(PG-3)는 충돌 영역이 겹치므로 Agent Team 사용 시 머지 충돌 발생 위험
- **권장**: `/fullstack-feature` 명령어로 F-12 → F-13 → F-14 순차 진행

---

## 5. 예상 복잡도

| Phase | 복잡도 | 근거 |
|-------|--------|------|
| Phase 1: 기반 서비스 | **High** | Fetch API ReadableStream 진행률 모니터링, HTTP Range 요청, AbortController 관리, IndexedDB Blob 저장 등 기술적 난이도 높음 |
| Phase 2: 상태 관리 | **Medium** | DownloadContext + Reducer 패턴은 기존 TabContext와 유사하지만, 다운로드 복원 로직 추가 필요 |
| Phase 3: UI 컴포넌트 | **Medium** | Moonstone 컴포넌트 조합은 단순하지만, Spotlight 포커스 관리와 상태별 UI 분기 처리 필요 |
| Phase 4: BrowserView 통합 | **Medium** | 여러 컴포넌트 간 통합 작업, 리모컨 단축키 추가 |
| Phase 5: 테스트 | **High** | Fetch API mock, IndexedDB mock, 타이머 mock 등 복잡한 mock 환경 구축 필요 |
| Phase 6: 리뷰 | **Low** | 일반적인 코드 리뷰 |
| Phase 7: 사후 문서 | **Low** | 문서 작성 |
| Phase 8: 마무리 | **Low** | 로그 업데이트 |

**전체 복잡도**: **High**

---

## 6. 위험 요소 및 대응 방안

| 리스크 | 영향도 | 발생 확률 | 대응 방안 |
|--------|--------|----------|----------|
| **Fetch API ReadableStream 지원 부족** | High | Medium | webOS 브라우저가 ReadableStream을 지원하지 않을 경우, XMLHttpRequest로 대체 (진행률 모니터링 정확도 저하 감수) |
| **HTTP Range 요청 미지원 서버** | Medium | High | 서버가 Range를 지원하지 않을 경우, 일시정지/재개 기능은 "처음부터 재다운로드"로 fallback (사용자에게 안내 메시지 표시) |
| **IndexedDB Blob 저장 용량 제한** | High | Low | 대용량 파일(1GB 이상) 다운로드 시 경고 메시지 표시, 디스크 공간 부족 시 에러 처리 |
| **동시 다운로드 5개 제한 UX 불만** | Low | Low | 설정 화면에서 제한 수를 사용자가 조정할 수 있도록 추후 확장 가능 (이번 구현에서는 5개 고정) |
| **CORS 제약으로 다운로드 불가** | High | Medium | CORS 에러 발생 시, 새 탭으로 다운로드 링크를 열어 브라우저 기본 다운로드 사용 (사용자에게 안내) |
| **webOS 앱 종료 시 다운로드 중단** | Medium | High | 앱 종료 시 다운로드 상태를 IndexedDB에 저장, 앱 재시작 시 재개 가능하도록 안내 메시지 표시 (Service Worker 없음으로 백그라운드 다운로드 불가) |
| **BrowserView.js 머지 충돌 (PG-3)** | High | High | F-12 완료 후 F-13, F-14 순차 진행으로 충돌 방지 (병렬 실행 금지) |
| **리모컨 노란색 버튼 키 코드 불명확** | Low | Medium | webOS 디바이스에서 키 코드 로깅으로 확인 후 매핑 (문서화 필요) |

---

## 7. 테스트 전략

### 단위 테스트
- **대상**: downloadHelper, downloadService, DownloadContext reducer
- **도구**: Jest
- **커버리지 목표**: 80% 이상
- **중점 테스트**:
  - downloadService.startDownload() 정상 다운로드, 에러 처리
  - downloadService.resumeDownload() Range 헤더 재개, fallback
  - DownloadContext reducer 액션 처리, 중복 ID 방지

### 통합 테스트
- **대상**: DownloadPanel, BrowserView 통합
- **도구**: Enact Testing Library (React Testing Library 기반)
- **중점 테스트**:
  - 다운로드 시작 → 완료 플로우
  - 일시정지 → 재개 플로우
  - 네트워크 오류 → 재시도 플로우
  - 동시 다운로드 5개 제한

### E2E 테스트 (수동)
- **환경**: webOS 프로젝터 실기기 (HU175QW)
- **중점 테스트**:
  - 리모컨 노란색 버튼으로 다운로드 패널 열기
  - 리모컨 방향키로 다운로드 항목 이동, 액션 버튼 실행
  - 백그라운드 다운로드 (패널 닫고 다운로드 계속 진행)
  - 다양한 파일 형식 다운로드 (PDF, JPG, PNG, MP4, ZIP)
  - 대용량 파일 다운로드 (100MB 이상)

---

## 8. 예상 총 소요 시간

| Phase | 시간 | 누적 |
|-------|------|------|
| Phase 1: 기반 서비스 | 6시간 | 6시간 |
| Phase 2: 상태 관리 | 2시간 | 8시간 |
| Phase 3: UI 컴포넌트 | 10시간 | 18시간 |
| Phase 4: BrowserView 통합 | 5시간 | 23시간 |
| Phase 5: 테스트 | 8시간 | 31시간 |
| Phase 6: 리뷰 | 3시간 | 34시간 |
| Phase 7: 사후 문서 | 3시간 | 37시간 |
| Phase 8: 마무리 | 1.5시간 | 38.5시간 |

**총 예상 소요 시간**: **약 38.5시간** (약 5일, 하루 8시간 기준)

---

## 9. 완료 체크리스트

### 기능 완성도
- [ ] 웹 페이지에서 다운로드 버튼 클릭 시 다운로드 확인 다이얼로그 표시
- [ ] 다운로드 시작 시 DownloadPanel에 항목 추가, 진행률 실시간 업데이트
- [ ] 프로그레스바, 퍼센트, 속도, 남은 시간이 정확히 표시됨
- [ ] 진행 중 다운로드를 일시정지/재개할 수 있음 (서버 Range 지원 시)
- [ ] 진행 중 다운로드를 취소할 수 있음
- [ ] 다운로드 완료 시 토스트 알림 표시
- [ ] 완료된 다운로드 목록을 확인할 수 있음
- [ ] 실패한 다운로드를 재시도할 수 있음
- [ ] 다운로드 패널을 닫아도 백그라운드에서 다운로드 계속 진행
- [ ] 다운로드 이력이 IndexedDB에 저장되고, 앱 재시작 후에도 유지됨

### 성능
- [ ] 다운로드 진행률 업데이트 주기 1초 이내
- [ ] 동시 다운로드 5개까지 안정적으로 동작
- [ ] 다운로드 패널 열 때 이력 로딩 1초 이내
- [ ] 10MB 파일 다운로드 시 메모리 누수 없음

### UX
- [ ] 리모컨 방향키로 다운로드 항목 간 이동, 선택 버튼으로 액션 실행 가능
- [ ] 포커스된 항목과 버튼이 명확히 하이라이트됨 (두꺼운 테두리)
- [ ] 진행 중, 완료, 실패 상태가 아이콘 + 색상으로 명확히 구분됨
- [ ] 대화면(3m 거리)에서 파일명, 진행률, 상태 텍스트가 가독 가능 (최소 20px 폰트)

### 보안
- [ ] HTTP URL 다운로드 시 보안 경고 다이얼로그 표시
- [ ] HTTPS URL 다운로드는 경고 없이 진행

### 테스트
- [ ] 단위 테스트 작성 및 통과
- [ ] 통합 테스트 작성 및 통과
- [ ] E2E 테스트 (수동) 완료

### 문서
- [ ] 컴포넌트 문서 작성 (DownloadPanel, DownloadItem, DownloadConfirmDialog)
- [ ] 서비스 문서 작성 (downloadService)
- [ ] dev-log.md 업데이트
- [ ] CHANGELOG.md 업데이트
- [ ] features.md 상태 업데이트

---

## 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|----------|--------|
| 2026-02-13 | 초안 작성 | product-manager |
