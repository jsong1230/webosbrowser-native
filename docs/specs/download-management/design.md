# 다운로드 관리 기능 — 기술 설계서

## 1. 참조
- 요구사항 분석서: [docs/specs/download-management/requirements.md](./requirements.md)

---

## 2. 아키텍처 결정

### 결정 1: 다운로드 엔진 선택

- **선택지**:
  - A) webOS LS2 API (`com.webos.service.downloadmanager`) 사용
  - B) Fetch API + FileSystem API로 자체 구현
  - C) XMLHttpRequest + Blob 다운로드 (레거시)

- **결정**: **B) Fetch API + FileSystem API 우선, webOS LS2 API를 대체 옵션으로**

- **근거**:
  - webOS LS2 API는 webOS 4.x/5.x 버전별로 API 지원이 불명확하고, 일시정지/재개 기능이 없을 가능성 높음
  - Fetch API는 브라우저 표준 API로 진행률 모니터링(`ReadableStream`)을 지원하여 정확한 진행률 표시 가능
  - FileSystem API (또는 IndexedDB Blob 저장)로 다운로드 파일을 로컬에 저장 가능
  - webOS 제약이 확인될 경우 LS2 API로 전환할 수 있도록 추상화

- **트레이드오프**:
  - 포기: webOS 네이티브 다운로드 UI, 시스템 다운로드 매니저 통합
  - 획득: 정확한 진행률 모니터링, 일시정지/재개 제어, 크로스 플랫폼 호환성

---

### 결정 2: 다운로드 파일 저장 위치

- **선택지**:
  - A) webOS 파일 시스템 (`/media/internal/downloads/`)
  - B) IndexedDB Blob 저장 (브라우저 저장소)
  - C) webOS LS2 파일 API로 저장

- **결정**: **B) IndexedDB Blob 저장 (브라우저 저장소)**

- **근거**:
  - webOS 파일 시스템 접근 권한이 제한적일 수 있음 (appinfo.json 권한 필요)
  - IndexedDB는 브라우저 표준 API로 별도 권한 없이 사용 가능
  - Blob 객체로 다운로드 파일을 저장하고, URL.createObjectURL()로 브라우저에서 열기 가능
  - 다운로드 이력 (메타데이터)과 파일 데이터를 동일한 저장소에서 관리 가능

- **트레이드오프**:
  - 포기: webOS 파일 관리자 앱에서 파일 접근 불가 (브라우저 전용 저장소)
  - 획득: 권한 불필요, 구현 단순화, 크로스 브라우저 호환성

---

### 결정 3: 다운로드 상태 관리 방식

- **선택지**:
  - A) React Context API (전역 상태)
  - B) 로컬 useState + DownloadService 이벤트 리스너
  - C) Redux 같은 상태 관리 라이브러리

- **결정**: **A) React Context API (전역 상태)**

- **근거**:
  - 다운로드는 백그라운드에서 계속 실행되므로 컴포넌트 언마운트와 독립적인 전역 상태 필요
  - 기존 TabContext 패턴을 따라 DownloadContext를 생성하여 일관된 아키텍처 유지
  - 다운로드 상태(진행 중, 완료, 실패)를 BrowserView, DownloadPanel, 알림 토스트 등 여러 컴포넌트에서 공유 필요
  - Redux 도입은 과도한 복잡도 증가

- **트레이드오프**:
  - 포기: Redux DevTools 같은 디버깅 도구
  - 획득: 기존 패턴 일관성, 경량 구현

---

### 결정 4: 일시정지/재개 구현 방식

- **선택지**:
  - A) Fetch API AbortController로 취소 후 Range 헤더로 재시작
  - B) 일시정지 기능 미제공 (취소만 지원)
  - C) 다운로드 데이터를 청크 단위로 IndexedDB에 저장하여 재개

- **결정**: **A) Fetch API AbortController로 취소 후 Range 헤더로 재시작**

- **근거**:
  - HTTP Range 요청은 대부분의 웹 서버가 지원하므로 부분 다운로드 재개 가능
  - AbortController로 Fetch 요청을 취소하고, 다운로드된 바이트 수를 저장하여 Range 헤더로 재시작
  - 서버가 Range를 지원하지 않을 경우, 처음부터 재다운로드 (fallback)

- **트레이드오ff**:
  - 포기: 모든 서버 호환성 (Range 미지원 서버는 재시작 불가)
  - 획득: 표준 HTTP 프로토콜 사용, 네트워크 효율성

---

### 결정 5: 동시 다운로드 수 제한

- **선택지**:
  - A) 제한 없음 (사용자가 원하는 만큼 다운로드)
  - B) 최대 3개 동시 다운로드
  - C) 최대 5개 동시 다운로드

- **결정**: **C) 최대 5개 동시 다운로드**

- **근거**:
  - webOS 프로젝터 하드웨어 메모리 제약 고려 (과도한 동시 다운로드는 성능 저하 유발)
  - 네트워크 대역폭 분산 방지 (동시 다운로드가 많으면 각 다운로드 속도 저하)
  - 일반 사용자는 5개 이상 동시 다운로드를 거의 사용하지 않음

- **트레이드오프**:
  - 포기: 무제한 다운로드 자유도
  - 획득: 안정적인 성능, 네트워크 효율성

---

## 3. API 설계

### 다운로드 서비스 API (downloadService.js)

#### `startDownload(url, filename, options)`
- **목적**: 다운로드를 시작하고 진행 상태를 관리
- **파라미터**:
  - `url` (string): 다운로드할 파일 URL
  - `filename` (string): 저장할 파일명
  - `options` (object): `{ onProgress, onComplete, onError }`
- **반환**: `Promise<string>` — 다운로드 ID
- **동작**:
  1. 동시 다운로드 수 체크 (5개 이상이면 에러)
  2. Fetch API로 요청 시작
  3. ReadableStream으로 진행률 모니터링
  4. 다운로드 데이터를 Blob으로 저장
  5. IndexedDB에 파일 메타데이터 + Blob 저장
  6. `onProgress` 콜백 호출 (진행률, 속도, 남은 시간)
  7. 완료 시 `onComplete` 콜백 호출

#### `pauseDownload(downloadId)`
- **목적**: 진행 중인 다운로드를 일시정지
- **파라미터**: `downloadId` (string)
- **반환**: `Promise<void>`
- **동작**:
  1. AbortController로 Fetch 요청 취소
  2. 다운로드된 바이트 수를 IndexedDB에 저장 (재개 위치)
  3. 다운로드 상태를 `paused`로 변경

#### `resumeDownload(downloadId)`
- **목적**: 일시정지된 다운로드를 재개
- **파라미터**: `downloadId` (string)
- **반환**: `Promise<void>`
- **동작**:
  1. IndexedDB에서 다운로드된 바이트 수 조회
  2. HTTP Range 헤더로 Fetch 요청 재시작 (`Range: bytes={resumePosition}-`)
  3. 서버가 206 응답을 반환하면 이어서 다운로드
  4. 서버가 Range를 지원하지 않으면 처음부터 재다운로드

#### `cancelDownload(downloadId)`
- **목적**: 다운로드 취소
- **파라미터**: `downloadId` (string)
- **반환**: `Promise<void>`
- **동작**:
  1. AbortController로 Fetch 요청 취소
  2. IndexedDB에서 다운로드 데이터 삭제
  3. 다운로드 상태를 `cancelled`로 변경

#### `deleteDownload(downloadId)`
- **목적**: 완료/실패한 다운로드를 목록에서 삭제
- **파라미터**: `downloadId` (string)
- **반환**: `Promise<void>`
- **동작**:
  1. IndexedDB에서 다운로드 메타데이터 삭제
  2. 파일 Blob도 함께 삭제 (또는 사용자에게 파일 유지 여부 확인)

#### `getDownloadHistory()`
- **목적**: 모든 다운로드 이력 조회 (진행 중 + 완료 + 실패)
- **반환**: `Promise<Array<Download>>` — 다운로드 배열
- **동작**:
  1. IndexedDB에서 모든 다운로드 레코드 조회
  2. `updatedAt` 역순 정렬 (최신순)

#### `getActiveDownloads()`
- **목적**: 현재 진행 중인 다운로드 목록 조회
- **반환**: `Array<Download>` — 진행 중 다운로드 배열

#### `openDownloadedFile(downloadId)`
- **목적**: 다운로드 완료된 파일을 브라우저에서 열기 (선택 사항)
- **파라미터**: `downloadId` (string)
- **반환**: `Promise<void>`
- **동작**:
  1. IndexedDB에서 Blob 조회
  2. URL.createObjectURL()로 Blob URL 생성
  3. window.open() 또는 iframe으로 파일 열기

---

## 4. DB 설계

### 새 Object Store: `downloads`

| 컬럼 | 타입 | 제약조건 | 설명 |
|------|------|----------|------|
| id | String | PK, UUID | 다운로드 고유 식별자 |
| url | String | NOT NULL | 다운로드 파일 URL |
| filename | String | NOT NULL | 저장할 파일명 |
| fileSize | Number | NULL | 파일 크기 (바이트), 알 수 없으면 null |
| downloadedBytes | Number | DEFAULT 0 | 현재까지 다운로드된 바이트 수 |
| status | String | NOT NULL | 다운로드 상태 (`pending`, `inProgress`, `paused`, `completed`, `failed`, `cancelled`) |
| speed | Number | DEFAULT 0 | 다운로드 속도 (bytes/sec) |
| remainingTime | Number | NULL | 남은 시간 추정치 (초), 알 수 없으면 null |
| errorMessage | String | NULL | 실패 사유 |
| blob | Blob | NULL | 다운로드된 파일 데이터 (완료 시에만 저장) |
| createdAt | Number | NOT NULL | 다운로드 시작 시각 (Unix timestamp) |
| updatedAt | Number | NOT NULL | 마지막 업데이트 시각 (Unix timestamp) |
| completedAt | Number | NULL | 다운로드 완료 시각 (Unix timestamp) |

### 인덱스 계획

| 인덱스명 | 컬럼 | 용도 |
|---------|------|------|
| status | status | 진행 중 다운로드 필터링 |
| updatedAt | updatedAt | 최신순 정렬 |

### 다운로드 상태 흐름

```
pending → inProgress → completed
                 ↓
               paused → inProgress (재개)
                 ↓
               failed (재시도 가능)
                 ↓
             cancelled (영구 삭제)
```

---

## 5. 컴포넌트 아키텍처

### 컴포넌트 트리

```
BrowserView
├── URLBar
├── TabBar
├── WebView (다운로드 링크 클릭 감지)
├── NavigationBar
├── LoadingBar
└── DownloadPanel (신규)
    ├── DownloadList
    │   ├── DownloadItem (진행 중)
    │   │   ├── ProgressBar
    │   │   ├── PauseButton / ResumeButton
    │   │   ├── CancelButton
    │   │   └── SpeedIndicator
    │   └── DownloadItem (완료)
    │       ├── FileIcon
    │       ├── OpenButton
    │       └── DeleteButton
    └── DownloadConfirmDialog (신규)
```

### 새 컴포넌트 설명

#### DownloadPanel
- **역할**: 다운로드 목록 패널 (Moonstone Panel 사용)
- **Props**:
  - `visible` (boolean): 패널 표시 여부
  - `onClose` (function): 패널 닫기 콜백
- **상태**:
  - `activeTab` (string): "진행 중" | "완료" | "전체"
  - `downloads` (array): DownloadContext에서 가져온 다운로드 배열
- **Spotlight 설정**:
  - 패널 열 때: 첫 번째 다운로드 항목으로 포커스
  - 리모컨 Back 키: 패널 닫기
  - 리모컨 좌/우 방향키: 탭 전환 ("진행 중" ↔ "완료")
  - 리모컨 위/아래 방향키: 다운로드 항목 간 이동

#### DownloadList
- **역할**: 다운로드 항목 리스트 (VirtualList 사용하여 성능 최적화)
- **Props**:
  - `downloads` (array): 다운로드 배열
  - `onPause` (function): 일시정지 핸들러
  - `onResume` (function): 재개 핸들러
  - `onCancel` (function): 취소 핸들러
  - `onDelete` (function): 삭제 핸들러
  - `onOpen` (function): 파일 열기 핸들러
  - `onRetry` (function): 재시도 핸들러

#### DownloadItem
- **역할**: 개별 다운로드 항목
- **Props**:
  - `download` (object): 다운로드 객체
  - `onPause` / `onResume` / `onCancel` / `onDelete` / `onOpen` / `onRetry`
- **렌더링**:
  - 파일명 (최대 40자, 긴 파일명은 말줄임)
  - 파일 크기 (예: 10.5 MB)
  - 진행률 (프로그레스바 + 퍼센트)
  - 다운로드 속도 (진행 중일 때만, 예: 1.2 MB/s)
  - 남은 시간 (진행 중일 때만, 예: 약 30초 남음)
  - 상태 아이콘 (진행 중: 파란색 회전 아이콘, 완료: 녹색 체크, 실패: 빨간색 경고)
  - 액션 버튼 (상태별 다르게 표시)

#### DownloadConfirmDialog
- **역할**: 다운로드 시작 전 확인 다이얼로그
- **Props**:
  - `visible` (boolean)
  - `url` (string)
  - `filename` (string)
  - `fileSize` (number)
  - `onConfirm` (function)
  - `onCancel` (function)
- **렌더링**:
  - "다운로드 하시겠습니까?"
  - 파일명, 파일 크기, URL
  - HTTP URL일 경우 보안 경고 메시지 표시
  - 확인 / 취소 버튼 (Spotlight 포커스)

---

## 6. 상태 관리 (DownloadContext)

### Context 구조

```javascript
DownloadContext = {
	downloads: [
		{
			id: "uuid",
			url: "https://example.com/file.pdf",
			filename: "document.pdf",
			fileSize: 10485760, // 10 MB
			downloadedBytes: 5242880, // 5 MB
			status: "inProgress",
			speed: 1048576, // 1 MB/s
			remainingTime: 5, // 5초
			errorMessage: null,
			createdAt: 1676543210000,
			updatedAt: 1676543215000,
			completedAt: null
		},
		// ... 더 많은 다운로드
	],
	activeDownloadCount: 3, // 현재 진행 중인 다운로드 수
	dispatch: (action) => {} // Context reducer 디스패치
}
```

### Actions

| Action | Payload | 설명 |
|--------|---------|------|
| `ADD_DOWNLOAD` | `{ download }` | 새 다운로드 추가 (상태: pending) |
| `UPDATE_DOWNLOAD` | `{ id, updates }` | 다운로드 정보 업데이트 (진행률, 속도 등) |
| `REMOVE_DOWNLOAD` | `{ id }` | 다운로드 삭제 (목록에서 제거) |
| `START_DOWNLOAD` | `{ id }` | 다운로드 시작 (상태: inProgress) |
| `PAUSE_DOWNLOAD` | `{ id }` | 일시정지 (상태: paused) |
| `RESUME_DOWNLOAD` | `{ id }` | 재개 (상태: inProgress) |
| `COMPLETE_DOWNLOAD` | `{ id }` | 완료 (상태: completed) |
| `FAIL_DOWNLOAD` | `{ id, errorMessage }` | 실패 (상태: failed) |
| `CANCEL_DOWNLOAD` | `{ id }` | 취소 (상태: cancelled) |

---

## 7. 시퀀스 흐름

### 주요 시나리오: 다운로드 시작

```
사용자 → WebView (다운로드 링크 클릭)
  │
  │  이벤트 감지 (onDownloadRequest)
  │──────────────────────────────────▶
  │                                    │
  │  DownloadConfirmDialog 표시        │
  │◀──────────────────────────────────│
  │                                    │
  │  "확인" 버튼 클릭                   │
  │──────────────────────────────────▶│
  │                                    │
  │                  downloadService.startDownload()
  │                                    │──────────────▶ DownloadService
  │                                    │                    │
  │                                    │  Fetch API 요청    │
  │                                    │◀──────────────────│
  │                                    │                    │
  │                  DownloadContext.dispatch(ADD_DOWNLOAD)│
  │                                    │◀──────────────────│
  │                                    │                    │
  │  DownloadPanel 업데이트 (리스트)    │                    │
  │◀──────────────────────────────────│                    │
  │                                    │                    │
  │                  진행률 업데이트 (1초마다)               │
  │                                    │◀──────────────────│
  │                                    │                    │
  │  ProgressBar 업데이트               │                    │
  │◀──────────────────────────────────│                    │
  │                                    │                    │
  │                  다운로드 완료     │                    │
  │                                    │◀──────────────────│
  │                                    │                    │
  │                  IndexedDB 저장 (Blob + 메타데이터)    │
  │                                    │──────────────────▶ IndexedDB
  │                                    │                    │
  │                  DownloadContext.dispatch(COMPLETE)   │
  │                                    │◀──────────────────│
  │                                    │                    │
  │  토스트 알림 "다운로드 완료"        │                    │
  │◀──────────────────────────────────│                    │
```

---

### 주요 시나리오: 일시정지/재개

```
사용자 → DownloadPanel (일시정지 버튼 클릭)
  │
  │  onPause(downloadId)
  │──────────────────────────────────▶ DownloadPanel
  │                                    │
  │                  downloadService.pauseDownload(id)
  │                                    │──────────────▶ DownloadService
  │                                    │                    │
  │                                    │  AbortController.abort()
  │                                    │                    │
  │                                    │  IndexedDB 저장 (resumePosition)
  │                                    │──────────────────▶ IndexedDB
  │                                    │                    │
  │                  DownloadContext.dispatch(PAUSE)       │
  │                                    │◀──────────────────│
  │                                    │                    │
  │  일시정지 상태 표시 (회색 아이콘)   │                    │
  │◀──────────────────────────────────│                    │
  │                                    │                    │
  │  (사용자가 재개 버튼 클릭)          │                    │
  │──────────────────────────────────▶│                    │
  │                                    │                    │
  │                  downloadService.resumeDownload(id)   │
  │                                    │──────────────▶    │
  │                                    │                    │
  │                                    │  Fetch API (Range 헤더)
  │                                    │                    │
  │                                    │  서버 206 응답?    │
  │                                    │                    │
  │                                    │  Yes: 이어서 다운로드
  │                                    │  No: 처음부터 재다운로드
  │                                    │                    │
  │                  DownloadContext.dispatch(RESUME)     │
  │                                    │◀──────────────────│
  │                                    │                    │
  │  진행 중 상태 표시 (파란색 아이콘)  │                    │
  │◀──────────────────────────────────│                    │
```

---

### 에러 시나리오: 네트워크 오류

```
사용자 → 다운로드 진행 중 (네트워크 끊김)
  │
  │                  Fetch API 에러 발생
  │                                    │◀──────────────────
  │                                    │
  │                  downloadService.handleError()
  │                                    │──────────────▶ DownloadService
  │                                    │                    │
  │                  DownloadContext.dispatch(FAIL)       │
  │                                    │◀──────────────────│
  │                                    │                    │
  │  실패 상태 표시 (빨간색 경고 아이콘) │                    │
  │◀──────────────────────────────────│                    │
  │                                    │                    │
  │  재시도 버튼 표시                   │                    │
  │◀──────────────────────────────────│                    │
  │                                    │                    │
  │  (사용자가 재시도 버튼 클릭)        │                    │
  │──────────────────────────────────▶│                    │
  │                                    │                    │
  │                  downloadService.startDownload()      │
  │                                    │──────────────▶    │
  │                                    │                    │
  │  다운로드 재시작                    │                    │
  │◀──────────────────────────────────│                    │
```

---

## 8. BrowserView 통합

### 다운로드 시작 트리거

#### WebView에서 다운로드 이벤트 감지 방식

**Option 1: iframe `beforeunload` 이벤트 리스닝 (제약 많음)**
- CORS 제약으로 cross-origin iframe에서 이벤트 접근 불가
- 동일 origin에서만 동작

**Option 2: 다운로드 버튼 UI 제공 (권장)**
- BrowserView에 "다운로드" 버튼 추가
- 사용자가 수동으로 URL 입력하여 다운로드 시작
- NavigationBar에 다운로드 버튼 배치

**Option 3: 브라우저 컨텍스트 메뉴 (리모컨 제약)**
- 링크에 포커스 후 특정 리모컨 버튼(예: 노란색 버튼) 누르면 다운로드
- useRemoteControl 훅에서 다운로드 단축키 처리

**결정**: **Option 2 + Option 3 혼합**
- NavigationBar에 다운로드 버튼 추가 (현재 페이지 URL 다운로드)
- 리모컨 컬러 버튼(예: 노란색)으로 다운로드 패널 열기

### BrowserView 수정 사항

```javascript
// BrowserView.js 추가 상태
const [showDownloadPanel, setShowDownloadPanel] = useState(false)
const [downloadRequest, setDownloadRequest] = useState(null)

// 다운로드 버튼 클릭 핸들러
const handleDownloadClick = () => {
	setDownloadRequest({
		url: currentUrl,
		filename: extractFilename(currentUrl),
		fileSize: null // 알 수 없음
	})
}

// 다운로드 확인 핸들러
const handleDownloadConfirm = async (request) => {
	const downloadId = await downloadService.startDownload(
		request.url,
		request.filename,
		{
			onProgress: (progress) => {
				// DownloadContext 업데이트
				dispatch({ type: 'UPDATE_DOWNLOAD', payload: { id: downloadId, updates: progress } })
			},
			onComplete: () => {
				dispatch({ type: 'COMPLETE_DOWNLOAD', payload: { id: downloadId } })
				// 토스트 알림 표시
				showToast(`다운로드 완료: ${request.filename}`)
			},
			onError: (error) => {
				dispatch({ type: 'FAIL_DOWNLOAD', payload: { id: downloadId, errorMessage: error.message } })
			}
		}
	)
	setDownloadRequest(null)
}
```

### NavigationBar 다운로드 버튼 추가

```javascript
<NavigationBar
	// ... 기존 props
	onDownload={handleDownloadClick}
	showDownloadButton={true}
/>
```

---

## 9. 영향 범위 분석

### 수정 필요한 기존 파일

| 파일 경로 | 변경 내용 | 이유 |
|----------|----------|------|
| `src/views/BrowserView.js` | 다운로드 패널 통합, 다운로드 버튼 추가 | DownloadPanel을 BrowserView에 추가하여 표시 |
| `src/components/NavigationBar/NavigationBar.js` | 다운로드 버튼 추가 | 리모컨으로 다운로드 기능 접근 |
| `src/hooks/useRemoteControl.js` | 다운로드 패널 단축키 추가 (컬러 버튼) | 리모컨 노란색 버튼으로 다운로드 패널 열기 |
| `src/services/indexedDBService.js` | `downloads` Object Store 추가 | 다운로드 메타데이터 + Blob 저장 |

### 새로 생성할 파일

| 파일 경로 | 역할 |
|----------|------|
| `src/services/downloadService.js` | 다운로드 CRUD, Fetch API 래퍼, 진행률 모니터링 |
| `src/contexts/DownloadContext.js` | 다운로드 전역 상태 관리 (Context + Reducer) |
| `src/components/DownloadPanel/index.js` | DownloadPanel 진입점 |
| `src/components/DownloadPanel/DownloadPanel.js` | 다운로드 패널 메인 컴포넌트 |
| `src/components/DownloadPanel/DownloadPanel.module.less` | 다운로드 패널 스타일 |
| `src/components/DownloadPanel/DownloadList.js` | 다운로드 항목 리스트 |
| `src/components/DownloadPanel/DownloadItem.js` | 개별 다운로드 항목 |
| `src/components/DownloadPanel/DownloadItem.module.less` | 다운로드 항목 스타일 |
| `src/components/DownloadPanel/DownloadConfirmDialog.js` | 다운로드 시작 확인 다이얼로그 |
| `src/components/DownloadPanel/DownloadConfirmDialog.module.less` | 확인 다이얼로그 스타일 |
| `src/utils/downloadHelper.js` | 파일명 추출, 크기 포맷팅, 속도 계산 유틸리티 |

### 영향 받는 기존 기능

| 기능명 | 영향 내용 | 대응 방안 |
|--------|----------|----------|
| F-04 (페이지 탐색 컨트롤) | NavigationBar에 다운로드 버튼 추가 | 버튼 레이아웃 조정 (가로 공간 확보) |
| F-13 (리모컨 단축키) | 다운로드 패널 단축키 추가 | useRemoteControl에 노란색 버튼 매핑 추가 |
| IndexedDB 스키마 | 새 Object Store 추가 | indexedDBService.js의 DB 버전 업그레이드 |

---

## 10. 기술적 주의사항

### Fetch API 진행률 모니터링

- **ReadableStream 사용**:
  - `response.body.getReader()`로 스트림 리더 획득
  - `reader.read()`를 반복하여 청크 단위로 데이터 읽기
  - 각 청크의 `value.byteLength`를 누적하여 진행률 계산
  - `Content-Length` 헤더로 전체 파일 크기 파악 (없을 경우 진행률 표시 불가)

### HTTP Range 요청 주의사항

- **서버 지원 확인**:
  - 첫 요청 시 `Accept-Ranges: bytes` 헤더 확인
  - Range 미지원 서버는 재개 불가 (처음부터 재다운로드)
- **Range 요청 형식**:
  - `Range: bytes=1024-` (1024바이트부터 끝까지)
  - 서버는 206 Partial Content 응답 반환

### CORS 제약

- **Cross-origin 다운로드**:
  - 일부 서버는 CORS 정책으로 cross-origin 다운로드 차단
  - 이 경우 `mode: 'no-cors'`로 요청하면 진행률 모니터링 불가 (opaque response)
  - 대안: 새 탭으로 다운로드 링크 열기 (브라우저 기본 다운로드)

### IndexedDB Blob 저장 제약

- **저장 용량 제한**:
  - 브라우저별로 IndexedDB 저장 용량 제한 (일반적으로 수 GB)
  - 대용량 파일(1GB 이상) 다운로드 시 경고 메시지 표시
  - 디스크 공간 부족 시 에러 처리 필요

### 동시 다운로드 Race Condition

- **AbortController 관리**:
  - 각 다운로드마다 별도 AbortController 인스턴스 생성
  - 다운로드 취소 시 해당 컨트롤러만 abort 호출
- **상태 업데이트 충돌**:
  - DownloadContext의 dispatch는 큐에 순차 실행되므로 race condition 없음

### 백그라운드 다운로드 지속성

- **웹 앱 종료 시**:
  - webOS 앱이 종료되면 다운로드도 중단됨 (Service Worker 없음)
  - 다운로드 상태를 IndexedDB에 저장하여 앱 재시작 시 재개 가능
- **탭 전환 시**:
  - 다운로드는 TabContext와 독립적으로 DownloadContext에서 관리
  - 탭 전환해도 다운로드 계속 진행

### 보안 고려사항

- **HTTP vs HTTPS**:
  - HTTP URL 다운로드 시 DownloadConfirmDialog에서 보안 경고 표시
  - HTTPS 우선 권장 메시지
- **악성 파일 방지**:
  - 실행 파일(.exe, .sh, .apk 등) 다운로드 시 경고 메시지 (선택 사항)
  - MIME 타입 체크 (Content-Type 헤더)

---

## 11. UI/UX 설계 (Moonstone 컴포넌트)

### DownloadPanel 레이아웃

```
┌──────────────────────────────────────────────────────────┐
│  다운로드                                         [ X ]   │ ← Header (닫기 버튼)
├──────────────────────────────────────────────────────────┤
│  [ 진행 중 (2) ]  [ 완료 (5) ]  [ 전체 (7) ]            │ ← Tabs (리모컨 좌/우로 전환)
├──────────────────────────────────────────────────────────┤
│                                                          │
│  📄 document.pdf                            [Downloading]│ ← DownloadItem (진행 중)
│  ▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░░░░░░░░  45% (4.5 MB / 10 MB)      │
│  1.2 MB/s · 약 5초 남음                                   │
│  [ ⏸ 일시정지 ]  [ ✖ 취소 ]                              │
│                                                          │
│  📄 image.jpg                               [Completed] │ ← DownloadItem (완료)
│  ✓ 완료 (2.3 MB)                                        │
│  [ 📂 열기 ]  [ 🗑 삭제 ]                                │
│                                                          │
│  📄 video.mp4                                  [Failed] │ ← DownloadItem (실패)
│  ✗ 다운로드 실패: 네트워크 오류                          │
│  [ 🔄 재시도 ]  [ 🗑 삭제 ]                              │
│                                                          │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

### Moonstone 컴포넌트 사용

| UI 요소 | Moonstone 컴포넌트 | 설정 |
|---------|-------------------|------|
| 다운로드 패널 | `Panel` | `spotlightRestrict="self-only"` |
| 탭 바 | `TabLayout` | `orientation="horizontal"` |
| 다운로드 항목 | `Spottable('div')` | 커스텀 스타일 |
| 프로그레스바 | `ProgressBar` | `progress={0-1}`, `backgroundProgress={0}` |
| 액션 버튼 | `Button` | `size="small"`, `icon="pause"/"play"/"delete"` |
| 확인 다이얼로그 | `Dialog` | `type="alert"`, `buttons={['확인', '취소']}` |

### Spotlight 포커스 플로우

```
다운로드 패널 열림
  ↓
첫 번째 다운로드 항목에 포커스
  ↓
리모컨 위/아래 방향키: 다운로드 항목 간 이동
리모컨 좌/우 방향키: 탭 전환 ("진행 중" ↔ "완료")
리모컨 선택 버튼: 항목 클릭 → 액션 버튼에 포커스
리모컨 Back 키: 패널 닫기
```

### 대화면 최적화

| UI 요소 | 크기 | 색상 | 간격 |
|---------|------|------|------|
| 파일명 텍스트 | 24px, 볼드 | #FFFFFF | - |
| 진행률 텍스트 | 20px | #CCCCCC | - |
| 상태 텍스트 | 20px | 진행 중: #4A90E2, 완료: #7ED321, 실패: #D0021B | - |
| 프로그레스바 | 높이 10px | 파란색 (#4A90E2) | - |
| 다운로드 항목 간 여백 | - | - | 20px |
| 액션 버튼 크기 | 120px × 48px | - | 좌우 8px |

---

## 12. 아키텍처 다이어그램

### 전체 시스템 구조

```
┌───────────────────────────────────────────────────────────────┐
│                        BrowserView                            │
│  ┌────────────┐  ┌────────────┐  ┌────────────────────────┐  │
│  │  URLBar    │  │  TabBar    │  │  NavigationBar         │  │
│  │            │  │            │  │  [다운로드 버튼 추가]   │  │
│  └────────────┘  └────────────┘  └────────────────────────┘  │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │                    WebView                              │  │
│  │  (다운로드 링크 감지 - 리모컨 단축키)                   │  │
│  └─────────────────────────────────────────────────────────┘  │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │               DownloadPanel (오버레이)                  │  │
│  │  ┌────────────────────────────────────────────────────┐ │  │
│  │  │            DownloadList                            │ │  │
│  │  │  ┌──────────────────────────────────────────────┐  │ │  │
│  │  │  │  DownloadItem (진행 중)                      │  │ │  │
│  │  │  │    - ProgressBar                             │  │ │  │
│  │  │  │    - PauseButton / ResumeButton              │  │ │  │
│  │  │  │    - CancelButton                            │  │ │  │
│  │  │  └──────────────────────────────────────────────┘  │ │  │
│  │  │  ┌──────────────────────────────────────────────┐  │ │  │
│  │  │  │  DownloadItem (완료)                         │  │ │  │
│  │  │  │    - OpenButton                              │  │ │  │
│  │  │  │    - DeleteButton                            │  │ │  │
│  │  │  └──────────────────────────────────────────────┘  │ │  │
│  │  └────────────────────────────────────────────────────┘ │  │
│  └─────────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────────────────────┘
                           │
                           │ dispatch actions
                           ▼
┌───────────────────────────────────────────────────────────────┐
│                   DownloadContext (전역 상태)                 │
│  - downloads: Array<Download>                                │
│  - activeDownloadCount: number                               │
│  - dispatch(action)                                          │
└───────────────────────────────────────────────────────────────┘
                           │
                           │ service calls
                           ▼
┌───────────────────────────────────────────────────────────────┐
│                   downloadService.js                          │
│  - startDownload(url, filename, options)                     │
│  - pauseDownload(id)                                         │
│  - resumeDownload(id)                                        │
│  - cancelDownload(id)                                        │
│  - deleteDownload(id)                                        │
│  - getDownloadHistory()                                      │
│  - openDownloadedFile(id)                                    │
└───────────────────────────────────────────────────────────────┘
                           │
         ┌─────────────────┴─────────────────┐
         ▼                                    ▼
┌──────────────────────┐         ┌────────────────────────┐
│  Fetch API           │         │  IndexedDB             │
│  (ReadableStream)    │         │  Object Store:         │
│  - 진행률 모니터링   │         │    downloads           │
│  - Range 헤더 재개   │         │  - 메타데이터          │
│  - AbortController   │         │  - Blob 파일 데이터    │
└──────────────────────┘         └────────────────────────┘
```

---

## 13. 테스트 전략

### 단위 테스트

| 대상 | 테스트 케이스 |
|------|-------------|
| `downloadService.startDownload()` | - 정상 다운로드 (200 응답)<br>- Content-Length 없는 경우<br>- 동시 다운로드 수 초과 시 에러<br>- CORS 에러 처리 |
| `downloadService.pauseDownload()` | - 정상 일시정지<br>- 이미 일시정지된 다운로드 재호출 방지 |
| `downloadService.resumeDownload()` | - Range 헤더로 재개 (206 응답)<br>- Range 미지원 서버 (처음부터 재다운로드)<br>- 네트워크 오류 시 실패 처리 |
| DownloadContext reducer | - ADD_DOWNLOAD, UPDATE_DOWNLOAD, COMPLETE_DOWNLOAD 액션 처리<br>- 중복 다운로드 ID 방지 |
| DownloadItem 렌더링 | - 진행 중 상태: 프로그레스바, 일시정지 버튼<br>- 완료 상태: 체크 아이콘, 열기 버튼<br>- 실패 상태: 경고 아이콘, 재시도 버튼 |

### 통합 테스트

| 시나리오 | 검증 사항 |
|---------|----------|
| 다운로드 시작 → 완료 플로우 | - DownloadPanel에 항목 추가<br>- 진행률 실시간 업데이트<br>- 완료 후 토스트 알림 표시<br>- IndexedDB에 Blob 저장 확인 |
| 일시정지 → 재개 플로우 | - 일시정지 시 Fetch 요청 중단<br>- 재개 시 Range 헤더로 요청 재시작<br>- 진행률이 이어서 증가 |
| 네트워크 오류 시 재시도 | - 네트워크 끊김 시 실패 상태 전환<br>- 재시도 버튼 클릭 시 처음부터 다운로드 |
| 동시 다운로드 5개 제한 | - 6번째 다운로드 시작 시 에러 메시지<br>- 하나 완료 후 새 다운로드 시작 가능 |

### E2E 테스트

| 시나리오 | 검증 사항 |
|---------|----------|
| 리모컨으로 다운로드 패널 열기 | - 노란색 버튼 누르면 DownloadPanel 표시<br>- 첫 번째 항목에 포커스 |
| 리모컨으로 다운로드 조작 | - 위/아래 방향키로 항목 이동<br>- 선택 버튼으로 액션 버튼 실행 (일시정지, 취소, 열기 등) |
| 백그라운드 다운로드 | - 다운로드 패널 닫아도 다운로드 계속 진행<br>- 다른 탭으로 전환해도 다운로드 유지 |

---

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|----------|------|
| 2026-02-13 | 초안 작성 | - |
