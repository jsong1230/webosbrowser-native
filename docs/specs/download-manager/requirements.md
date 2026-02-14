# 다운로드 관리 — 요구사항 분석서

## 1. 개요

### 기능명
F-12: 다운로드 관리 (Download Manager)

### 목적
웹 페이지에서 파일 다운로드 시작을 감지하고, 다운로드 진행 상황을 추적하며, 완료된 파일을 관리할 수 있는 네이티브 다운로드 관리 시스템을 구현합니다. Qt/webOS 환경에서 안정적이고 사용자 친화적인 다운로드 경험을 제공합니다.

### 대상 사용자
- 프로젝터로 웹 브라우징 중 문서, 이미지, 비디오 등을 다운로드하려는 사용자
- 다운로드 진행 상황을 실시간으로 확인하고 싶은 사용자
- 완료된 다운로드 파일을 관리(열기, 삭제)하려는 사용자

### 요청 배경
- **F-02 (웹뷰 통합) 완료**: WebView에서 다운로드 이벤트를 감지할 수 있는 기반 마련
- **webOS 파일 시스템 제약**: 다운로드 파일의 저장 위치를 webOS 정책에 맞게 설정 필요
- **리모컨 UX 최적화**: 다운로드 패널을 리모컨으로 조작 가능하도록 설계
- **M3 마일스톤**: PG-3(병렬 그룹 3)에 포함되어 F-13(리모컨 단축키), F-14(HTTPS 보안 표시)와 병렬 개발
- **우선순위 Should**: 핵심 브라우징 기능은 아니지만 사용자 편의성을 크게 향상시키는 기능

## 2. 유저 스토리

### US-1: 다운로드 시작 감지
- **As a** 프로젝터 사용자
- **I want** 웹 페이지에서 다운로드 링크를 클릭했을 때 브라우저가 자동으로 다운로드를 시작하기를
- **So that** 별도의 설정 없이 파일을 쉽게 다운로드할 수 있다

### US-2: 다운로드 진행 상황 확인
- **As a** 사용자
- **I want** 다운로드 중인 파일의 이름, 크기, 진행률(%)을 실시간으로 보고 싶다
- **So that** 다운로드가 얼마나 남았는지 알고 다른 작업을 할 수 있다

### US-3: 다운로드 일시 정지/재개
- **As a** 사용자
- **I want** 다운로드를 일시 정지했다가 나중에 재개할 수 있기를
- **So that** 네트워크가 불안정하거나 다른 작업을 우선할 때 제어권을 가질 수 있다

### US-4: 다운로드 취소
- **As a** 사용자
- **I want** 다운로드 중인 파일을 취소할 수 있기를
- **So that** 잘못 클릭했거나 불필요한 다운로드를 중단할 수 있다

### US-5: 완료된 다운로드 확인 및 관리
- **As a** 사용자
- **I want** 완료된 다운로드 목록을 보고, 파일을 열거나 삭제할 수 있기를
- **So that** 다운로드한 파일을 효율적으로 관리할 수 있다

### US-6: 다운로드 알림
- **As a** 사용자
- **I want** 다운로드가 완료되면 알림(토스트 메시지)을 받고 싶다
- **So that** 웹 브라우징 중에도 다운로드 완료를 놓치지 않을 수 있다

### US-7: 다운로드 패널 접근
- **As a** 사용자
- **I want** 리모컨 단축키(예: 컬러 버튼)로 다운로드 패널을 빠르게 열 수 있기를
- **So that** 언제든지 다운로드 상태를 확인할 수 있다

## 3. 기능 요구사항 (Functional Requirements)

### FR-1: 다운로드 감지 (WebView 통합)
- **설명**: QWebEngineView의 다운로드 이벤트를 감지하고 DownloadManager에 전달
- **우선순위**: Must
- **세부 요구사항**:
  - QWebEngineProfile의 `downloadRequested` 시그널 구독
  - 다운로드 항목(QWebEngineDownloadItem) 수신
  - 다운로드 메타데이터 추출:
    - 파일명 (다운로드 제안 파일명)
    - 파일 크기 (totalBytes)
    - MIME 타입 (예: application/pdf, image/png)
    - 원본 URL
  - DownloadManager 서비스로 다운로드 항목 전달
  - 자동 다운로드 시작 (사용자 승인 없이, 설정 가능)

### FR-2: DownloadManager 서비스 구현
- **설명**: 다운로드 항목의 생명주기를 관리하는 중앙 서비스 클래스
- **우선순위**: Must
- **세부 요구사항**:
  - 클래스 위치: `src/services/DownloadManager.h`, `src/services/DownloadManager.cpp`
  - 싱글톤 패턴 또는 QObject 기반 서비스
  - 다운로드 항목 리스트 관리:
    ```cpp
    struct DownloadItem {
        QString id;                  // 고유 ID (UUID)
        QString fileName;            // 파일명
        QString filePath;            // 저장 경로 (완료 후)
        QUrl url;                    // 원본 URL
        QString mimeType;            // MIME 타입
        qint64 bytesTotal;           // 전체 크기 (바이트)
        qint64 bytesReceived;        // 다운로드된 크기
        DownloadState state;         // 상태 (진행중, 일시정지, 완료, 에러)
        QDateTime startTime;         // 시작 시각
        QDateTime finishTime;        // 완료 시각
        QString errorMessage;        // 에러 메시지 (실패 시)
    };
    ```
  - 다운로드 상태 열거형:
    ```cpp
    enum class DownloadState {
        Pending,     // 대기 중
        InProgress,  // 다운로드 중
        Paused,      // 일시 정지
        Completed,   // 완료
        Cancelled,   // 취소됨
        Failed       // 실패
    };
    ```
  - 공개 API:
    - `void startDownload(QWebEngineDownloadItem *item)`: 다운로드 시작
    - `void pauseDownload(const QString &id)`: 일시 정지
    - `void resumeDownload(const QString &id)`: 재개
    - `void cancelDownload(const QString &id)`: 취소
    - `void removeDownload(const QString &id)`: 목록에서 제거 (완료/취소된 항목만)
    - `QList<DownloadItem> getDownloads()`: 전체 다운로드 목록 조회
  - 시그널:
    - `void downloadAdded(const DownloadItem &item)`: 새 다운로드 추가
    - `void downloadProgressChanged(const QString &id, qint64 bytesReceived, qint64 bytesTotal)`: 진행률 변경
    - `void downloadStateChanged(const QString &id, DownloadState state)`: 상태 변경
    - `void downloadCompleted(const DownloadItem &item)`: 다운로드 완료
    - `void downloadFailed(const QString &id, const QString &errorMessage)`: 다운로드 실패

### FR-3: 다운로드 저장 경로 설정
- **설명**: webOS 파일 시스템 정책에 맞춰 다운로드 파일 저장 위치 설정
- **우선순위**: Must
- **세부 요구사항**:
  - webOS 권장 경로 조사:
    - `/media/internal/downloads/` (선호)
    - 앱별 데이터 디렉토리: `~/.local/share/webosbrowser/downloads/`
  - Qt의 `QStandardPaths::DownloadLocation` 사용
  - 디렉토리 존재 확인 및 자동 생성 (`QDir::mkpath()`)
  - 파일명 중복 처리:
    - 중복 시 자동으로 번호 추가 (예: `file.pdf` → `file (1).pdf`)
  - 파일 시스템 권한 확인:
    - 쓰기 권한 없으면 에러 처리

### FR-4: 다운로드 진행률 추적
- **설명**: QWebEngineDownloadItem의 진행 상황을 실시간으로 추적
- **우선순위**: Must
- **세부 요구사항**:
  - QWebEngineDownloadItem의 `downloadProgress` 시그널 구독
  - 진행률 계산: `progress = (bytesReceived / bytesTotal) * 100`
  - 다운로드 속도 계산:
    - 이전 진행률 업데이트 시각과 현재 시각 차이
    - 속도 = (현재 bytesReceived - 이전 bytesReceived) / 시간 차이
    - 단위: KB/s, MB/s
  - 남은 시간 추정:
    - ETA = (bytesTotal - bytesReceived) / 다운로드 속도
  - 진행률 시그널 emit (100ms~500ms 간격)

### FR-5: 다운로드 일시 정지/재개
- **설명**: 다운로드를 일시 정지하고 재개하는 기능
- **우선순위**: Should
- **세부 요구사항**:
  - QWebEngineDownloadItem의 `pause()` 메서드 호출
  - QWebEngineDownloadItem의 `resume()` 메서드 호출
  - 상태 변경: `InProgress` ↔ `Paused`
  - UI에 일시 정지/재개 버튼 표시
  - 주의: Qt 5.15에서 pause/resume 지원 여부 확인 필요
    - 지원하지 않으면 해당 기능 비활성화

### FR-6: 다운로드 취소
- **설명**: 진행 중인 다운로드를 취소하고 부분 다운로드 파일 삭제
- **우선순위**: Must
- **세부 요구사항**:
  - QWebEngineDownloadItem의 `cancel()` 메서드 호출
  - 상태 변경: `InProgress` → `Cancelled`
  - 부분 다운로드 파일 삭제:
    - QFile::remove()로 임시 파일 삭제
  - 목록에서 즉시 제거 또는 "취소됨" 상태로 유지 (사용자 설정)

### FR-7: 다운로드 완료 처리
- **설명**: 다운로드 완료 시 파일 검증 및 알림
- **우선순위**: Must
- **세부 요구사항**:
  - QWebEngineDownloadItem의 `finished` 시그널 구독
  - 완료 시각 기록
  - 파일 존재 확인 (QFile::exists())
  - 파일 크기 검증 (다운로드 크기와 일치 확인)
  - 상태 변경: `InProgress` → `Completed`
  - `downloadCompleted` 시그널 emit
  - 알림 표시 (FR-8 연계)
  - LS2 API로 다운로드 기록 저장 (선택, 히스토리 연계)

### FR-8: 다운로드 완료 알림
- **설명**: 다운로드 완료 시 토스트 알림 표시
- **우선순위**: Should
- **세부 요구사항**:
  - webOS 토스트 알림 API 사용 (LS2 API: `luna://com.webos.notification/createToast`)
  - 알림 내용:
    - 제목: "다운로드 완료"
    - 메시지: "{파일명} 다운로드가 완료되었습니다"
    - 아이콘: 다운로드 완료 아이콘
  - 알림 클릭 시:
    - 다운로드 패널 열기 또는 파일 열기
  - 대안 (webOS API 제약 시):
    - Qt 자체 토스트 위젯 구현 (QLabel + QTimer)

### FR-9: 다운로드 에러 처리
- **설명**: 네트워크 에러, 디스크 공간 부족 등 다운로드 실패 처리
- **우선순위**: Must
- **세부 요구사항**:
  - QWebEngineDownloadItem의 `finished` 시그널에서 state 확인
  - 에러 타입 분류:
    - 네트워크 에러 (연결 끊김, 타임아웃)
    - 파일 시스템 에러 (디스크 공간 부족, 권한 없음)
    - 서버 에러 (404, 403, 500 등)
  - 상태 변경: `InProgress` → `Failed`
  - 에러 메시지 저장
  - `downloadFailed` 시그널 emit
  - UI에 에러 메시지 표시 (다운로드 패널)
  - 재시도 옵션 제공 (선택)

### FR-10: DownloadPanel UI 구현
- **설명**: 다운로드 목록과 제어 버튼을 표시하는 Qt 위젯 패널
- **우선순위**: Must
- **세부 요구사항**:
  - 클래스 위치: `src/ui/DownloadPanel.h`, `src/ui/DownloadPanel.cpp`
  - QWidget 기반 패널 (BrowserWindow 하단 또는 오버레이)
  - 다운로드 항목 리스트 (QListView 또는 QTableView):
    - 각 항목 표시:
      - 파일명
      - 파일 크기 (MB 단위)
      - 진행률 (프로그레스바 + %)
      - 다운로드 속도 (KB/s, MB/s)
      - 남은 시간 (ETA)
      - 상태 (진행중/일시정지/완료/에러)
  - 제어 버튼 (각 항목별):
    - 일시 정지/재개 버튼 (진행 중인 항목)
    - 취소 버튼 (진행 중/일시 정지 항목)
    - 열기 버튼 (완료된 항목)
    - 삭제 버튼 (완료/취소/에러 항목)
  - 전체 제어 버튼:
    - "모두 일시 정지" / "모두 재개"
    - "완료된 항목 삭제"
    - "패널 닫기"
  - 리모컨 포커스 최적화:
    - 십자키 위/아래로 항목 간 이동
    - 좌/우로 버튼 간 이동
    - Enter 키로 버튼 실행
  - 스타일링:
    - 대화면 가독성 (폰트 크기 16px 이상)
    - 진행률 프로그레스바 (QProgressBar)
    - 다크 테마 지원 (PRD 연계)

### FR-11: BrowserWindow에 DownloadPanel 통합
- **설명**: 다운로드 패널을 메인 윈도우에 통합하고 표시/숨김 제어
- **우선순위**: Must
- **세부 요구사항**:
  - BrowserWindow.h에 DownloadPanel 멤버 추가
  - 패널 표시 위치:
    - 옵션 A: 하단 슬라이드업 패널 (권장)
    - 옵션 B: 전체 화면 오버레이 (모달)
  - 표시/숨김 메서드:
    - `void showDownloadPanel()`: 패널 표시
    - `void hideDownloadPanel()`: 패널 숨김
    - `void toggleDownloadPanel()`: 토글
  - 리모컨 단축키 연결 (F-13 연계):
    - 컬러 버튼 (예: Blue) → 다운로드 패널 토글
  - 다운로드 진행 중 알림:
    - 패널이 닫혀 있어도 다운로드 개수 뱃지 표시 (NavBar)
  - DownloadManager 시그널 구독:
    - `downloadAdded` → 패널 자동 표시 (선택)
    - `downloadCompleted` → 알림 표시

### FR-12: 완료된 다운로드 파일 열기
- **설명**: 완료된 파일을 webOS 기본 앱으로 열기
- **우선순위**: Should
- **세부 요구사항**:
  - webOS LS2 API 사용: `luna://com.webos.service.applicationmanager/launch`
  - MIME 타입별 기본 앱 매핑:
    - PDF: 내장 PDF 뷰어
    - 이미지 (PNG, JPG): 내장 이미지 뷰어
    - 비디오 (MP4, MKV): 내장 비디오 플레이어
  - 대안 (LS2 API 제약 시):
    - Qt의 `QDesktopServices::openUrl(QUrl::fromLocalFile(filePath))`
  - 파일이 존재하지 않으면 에러 메시지 표시
  - 지원하지 않는 MIME 타입:
    - "이 파일 형식을 열 수 없습니다" 메시지 표시

### FR-13: 다운로드 기록 저장 (선택)
- **설명**: 완료된 다운로드를 LS2 API로 영구 저장
- **우선순위**: Could
- **세부 요구사항**:
  - LS2 API 사용: `luna://com.webos.service.db` (webOS 데이터베이스)
  - 저장 데이터:
    - 파일명
    - 파일 경로
    - 원본 URL
    - 다운로드 시각
    - 파일 크기
  - 앱 재시작 후에도 완료된 다운로드 목록 유지
  - 주의: F-08 (히스토리 관리)의 StorageService 재사용 가능

## 4. 비기능 요구사항 (Non-Functional Requirements)

### 성능
- **다운로드 속도**: 네트워크 속도의 90% 이상 활용 (오버헤드 최소화)
- **진행률 업데이트 주기**: 500ms 이내 (실시간 느낌)
- **UI 응답성**: 다운로드 중에도 브라우징 기능 정상 동작 (비동기 처리)
- **메모리 사용량**: 다운로드 항목 100개당 최대 10MB (메타데이터만 저장)
- **동시 다운로드**: 최대 3개 동시 진행 (네트워크 대역폭 고려)

### UX (리모컨 최적화)
- **포커스 관리**:
  - 다운로드 패널 내 항목 간 십자키로 이동
  - 버튼 간 좌/우 이동
  - 패널 닫기 시 이전 포커스로 복원
- **가독성**:
  - 진행률 프로그레스바 두껍게 표시 (최소 20px 높이)
  - 파일명, 진행률, 속도 명확히 구분 (폰트 크기 차이)
  - 에러 메시지는 빨간색으로 강조
- **피드백**:
  - 버튼 클릭 시 즉각적인 시각적 피드백 (버튼 색상 변경)
  - 다운로드 완료 시 토스트 알림
  - 에러 발생 시 명확한 에러 메시지

### 보안
- **파일 시스템 격리**: 다운로드 파일은 앱별 디렉토리 또는 공용 다운로드 폴더에만 저장
- **악성 파일 경고**: 실행 파일(.exe, .sh) 다운로드 시 경고 (선택)
- **HTTPS 다운로드 우선**: HTTP 다운로드 시 경고 표시 (F-14 연계)

### 호환성
- **Qt 버전**: Qt 5.15+ QWebEngineDownloadItem API 사용
- **webOS 파일 시스템**: FAT32, ext4 지원 확인
- **MIME 타입**: 일반적인 파일 타입 지원 (PDF, 이미지, 비디오, 문서)

### 안정성
- **네트워크 복원력**: 네트워크 일시 끊김 후 재연결 시 자동 재개 시도
- **디스크 공간 확인**: 다운로드 시작 전 충분한 공간 있는지 확인
- **크래시 방지**: 다운로드 중 앱 종료 시 부분 파일 정리 (다음 실행 시)

### 확장성
- **다운로드 큐**: 향후 다운로드 우선순위 기능 추가 가능
- **다운로드 히스토리**: F-08과 통합하여 다운로드 기록 조회 가능
- **다운로드 설정**: F-11(설정 화면)에서 저장 경로, 동시 다운로드 개수 등 설정 가능

## 5. 제약조건

### 플랫폼 제약
- **webOS 파일 시스템 권한**:
  - 일부 디렉토리는 쓰기 권한 제한 (루트 디렉토리, 시스템 폴더)
  - 앱별 데이터 디렉토리는 항상 접근 가능
- **webOS LS2 API 제약**:
  - 토스트 알림 API가 제한적일 수 있음 (대안: Qt 자체 구현)
  - 파일 열기 API가 일부 MIME 타입만 지원할 수 있음
- **리모컨 입력**:
  - 마우스/터치 없이 십자키로만 다운로드 패널 조작
  - 긴 파일명은 잘림 처리 필요 (툴팁 또는 스크롤)

### 기술 제약
- **Qt WebEngineDownloadItem 제약**:
  - pause/resume 기능이 Qt 5.15에서 제한적일 수 있음 (Qt 6에서 개선)
  - 다운로드 속도, ETA는 직접 계산 필요 (API 미제공)
- **메모리 제약**:
  - 대용량 파일(1GB+) 다운로드 시 메모리 사용량 모니터링 필요
  - 동시 다운로드 개수 제한으로 메모리 과부하 방지
- **네트워크**:
  - Wi-Fi 연결 필수 (유선랜 대안)
  - 프록시 설정은 webOS 시스템 설정 따름

### 의존성
- **선행 기능**:
  - F-02 (웹뷰 통합) 완료 필요 (QWebEngineProfile, downloadRequested 시그널)
- **후속 기능**:
  - F-11 (설정 화면): 다운로드 저장 경로, 자동 다운로드 on/off 설정
  - F-13 (리모컨 단축키): 다운로드 패널 단축키 (컬러 버튼)
- **병렬 가능**:
  - F-13 (리모컨 단축키): 충돌 없음 (KeyboardHandler vs DownloadManager)
  - F-14 (HTTPS 보안 표시): 충돌 없음 (SecurityIndicator vs DownloadPanel)

## 6. 용어 정의

| 용어 | 정의 |
|------|------|
| DownloadManager | 다운로드 항목의 생명주기를 관리하는 중앙 서비스 클래스 |
| DownloadPanel | 다운로드 목록과 제어 버튼을 표시하는 UI 패널 |
| QWebEngineDownloadItem | Qt WebEngine의 다운로드 항목 클래스 (다운로드 메타데이터 + 제어 API) |
| LS2 API | Luna Service 2 API, webOS의 시스템 서비스 API (비동기 메시지 버스) |
| MIME 타입 | 파일 형식을 나타내는 인터넷 표준 (예: application/pdf, image/png) |
| ETA | Estimated Time of Arrival, 다운로드 완료 예상 시간 |
| 토스트 알림 | 화면 하단에 짧은 시간 표시되는 알림 메시지 |
| 프로그레스바 | 진행 상황을 시각적으로 표시하는 UI 컴포넌트 (QProgressBar) |
| 싱글톤 패턴 | 클래스의 인스턴스가 하나만 존재하도록 보장하는 디자인 패턴 |

## 7. 범위 외 (Out of Scope)

### 이번 단계에서 하지 않는 것
- **다운로드 재개 (Resume)**: 앱 종료 후 재시작 시 부분 다운로드 파일로부터 재개 (구현 복잡도 높음)
- **다운로드 우선순위**: 여러 다운로드 중 우선순위 조정 기능
- **다운로드 스케줄링**: 특정 시간에 다운로드 시작하도록 예약
- **토렌트 다운로드**: P2P 다운로드는 지원하지 않음
- **다운로드 압축 해제**: ZIP, RAR 등 압축 파일 자동 해제 기능
- **다운로드 전 미리보기**: 파일 다운로드 전에 내용 미리보기 (이미지, 비디오)
- **다운로드 보안 스캔**: 바이러스/멀웨어 검사 (webOS 제약)
- **다운로드 동기화**: 다른 기기와 다운로드 목록 동기화 (LG 계정 연동)

### 추후 확장 가능 기능
- **다운로드 기록 검색**: 완료된 다운로드를 파일명, URL로 검색
- **다운로드 폴더 변경**: 설정에서 저장 경로 커스터마이징 (F-11 연계)
- **다운로드 대역폭 제한**: 다운로드 속도 제한으로 다른 네트워크 작업 보호
- **다운로드 재시도**: 실패한 다운로드 자동 재시도 (최대 3회)

## 8. 완료 기준 (Acceptance Criteria)

### AC-1: 다운로드 감지 완료
- [ ] WebView에서 다운로드 링크 클릭 시 `downloadRequested` 시그널 수신
- [ ] 파일명, 크기, URL, MIME 타입 추출 성공
- [ ] DownloadManager에 다운로드 항목 전달

### AC-2: DownloadManager 구현 완료
- [ ] `src/services/DownloadManager.h`, `DownloadManager.cpp` 생성
- [ ] DownloadItem 구조체 정의
- [ ] 다운로드 시작/일시정지/재개/취소 API 구현
- [ ] 시그널(downloadAdded, downloadProgressChanged 등) emit 확인

### AC-3: 다운로드 저장 경로 설정 완료
- [ ] QStandardPaths::DownloadLocation 경로 확인
- [ ] 디렉토리 자동 생성 (없으면)
- [ ] 파일명 중복 시 자동 번호 추가 확인
- [ ] 파일 쓰기 권한 확인 및 에러 처리

### AC-4: 다운로드 진행률 추적 완료
- [ ] `downloadProgress` 시그널 구독
- [ ] 진행률(%) 계산 및 UI 업데이트
- [ ] 다운로드 속도(KB/s) 계산
- [ ] 남은 시간(ETA) 추정

### AC-5: 다운로드 완료 처리 완료
- [ ] 다운로드 완료 시 파일 존재 확인
- [ ] 파일 크기 검증
- [ ] 상태 변경: `Completed`
- [ ] `downloadCompleted` 시그널 emit

### AC-6: 다운로드 에러 처리 완료
- [ ] 네트워크 에러 감지 (Wi-Fi 끄고 테스트)
- [ ] 디스크 공간 부족 에러 감지
- [ ] 상태 변경: `Failed`
- [ ] 에러 메시지 저장 및 UI 표시

### AC-7: DownloadPanel UI 구현 완료
- [ ] `src/ui/DownloadPanel.h`, `DownloadPanel.cpp` 생성
- [ ] 다운로드 목록 표시 (파일명, 크기, 진행률, 속도, ETA)
- [ ] 제어 버튼 (일시정지/재개/취소/열기/삭제) 동작
- [ ] 리모컨 포커스 이동 (십자키) 확인

### AC-8: BrowserWindow 통합 완료
- [ ] BrowserWindow에 DownloadPanel 멤버 추가
- [ ] 패널 표시/숨김 메서드 구현
- [ ] DownloadManager 시그널 구독 (downloadAdded, downloadCompleted)
- [ ] 다운로드 완료 시 알림 표시

### AC-9: 완료된 파일 열기 완료
- [ ] 완료된 항목의 "열기" 버튼 클릭 시 파일 열기
- [ ] QDesktopServices::openUrl() 또는 LS2 API 사용
- [ ] 지원하지 않는 MIME 타입 에러 처리

### AC-10: 실제 다운로드 테스트
- [ ] PDF 파일 다운로드 성공 (예: https://www.w3.org/WAI/ER/tests/xhtml/testfiles/resources/pdf/dummy.pdf)
- [ ] 이미지 파일 다운로드 성공 (예: PNG, JPG)
- [ ] 대용량 파일(100MB+) 다운로드 진행률 정상 표시
- [ ] 다운로드 중 일시 정지/재개 동작 (지원 시)
- [ ] 다운로드 취소 시 파일 삭제 확인

### AC-11: 리모컨 조작 확인
- [ ] 다운로드 패널 열기/닫기 (리모컨 단축키, F-13 연계)
- [ ] 십자키로 다운로드 항목 간 이동
- [ ] 좌/우 키로 버튼 간 이동
- [ ] Enter 키로 버튼 실행

### AC-12: 성능 기준 충족
- [ ] 다운로드 진행률 업데이트 주기: 500ms 이내
- [ ] 동시 다운로드 3개 진행 시 UI 응답성 정상
- [ ] 다운로드 항목 100개 시 메모리 사용량: 10MB 이하

### AC-13: 실제 디바이스 테스트
- [ ] LG 프로젝터 HU715QW에서 다운로드 동작 확인
- [ ] 리모컨으로 다운로드 패널 조작 확인
- [ ] 완료된 파일을 webOS 기본 앱으로 열기 확인

### AC-14: 문서화
- [ ] DownloadManager 클래스 주석 (한국어, Doxygen)
- [ ] DownloadPanel 클래스 주석
- [ ] 다운로드 워크플로우 문서화 (design.md)
- [ ] API 사용 예시 추가

## 9. 우선순위 및 복잡도

- **우선순위**: Should (핵심 기능은 아니지만 사용자 편의성 향상)
- **예상 복잡도**: High
  - QWebEngineDownloadItem API 학습
  - 다운로드 상태 관리 복잡도 (비동기 이벤트)
  - UI와 서비스 간 시그널/슬롯 연결
  - 파일 시스템 권한 및 에러 처리
  - 리모컨 포커스 최적화
  - 실제 디바이스 테스트 필요성
- **예상 소요 시간**: 2~3일
  - DownloadManager 서비스 구현: 6~8시간
  - DownloadPanel UI 구현: 6~8시간
  - BrowserWindow 통합: 2~3시간
  - 파일 시스템 및 에러 처리: 3~4시간
  - 테스트 및 디버깅: 4~6시간
  - 디바이스 테스트: 2~3시간
  - 문서화: 2~3시간

## 10. 리스크

| 리스크 | 확률 | 영향도 | 대응 방안 |
|--------|------|--------|-----------|
| Qt 5.15의 pause/resume 미지원 | 중 | 중 | API 지원 여부 확인, 미지원 시 해당 기능 비활성화. Qt 6 업그레이드 검토 |
| webOS 파일 시스템 권한 제약 | 중 | 고 | 앱별 데이터 디렉토리 사용, 쓰기 권한 에러 명확히 처리 |
| 대용량 파일 다운로드 시 메모리 과부하 | 중 | 중 | 동시 다운로드 개수 제한(3개), 메모리 사용량 모니터링 |
| 네트워크 불안정 시 다운로드 실패 | 고 | 중 | 에러 처리 강화, 재시도 옵션 제공 (선택) |
| webOS LS2 API 문서 부족 (토스트, 파일 열기) | 중 | 중 | Qt 자체 구현 대안 준비 (QDesktopServices, Qt 토스트 위젯) |
| 리모컨 포커스와 다운로드 패널 UI 충돌 | 중 | 중 | 포커스 경로 세밀 조정, 사용자 테스트로 UX 검증 |
| 다운로드 중 앱 크래시 시 부분 파일 잔존 | 낮음 | 낮음 | 앱 시작 시 임시 파일 정리 로직 추가 |

## 11. 의존성

### 선행 기능
- **F-02 (웹뷰 통합)**: 완료 필요 (QWebEngineProfile, downloadRequested 시그널)

### 후속 기능
- **F-11 (설정 화면)**: 다운로드 저장 경로, 자동 다운로드 설정
- **F-13 (리모컨 단축키)**: 다운로드 패널 단축키 (컬러 버튼)

### 병렬 가능 (PG-3)
- **F-13 (리모컨 단축키)**: 충돌 없음 (KeyboardHandler vs DownloadManager/DownloadPanel)
- **F-14 (HTTPS 보안 표시)**: 충돌 없음 (SecurityIndicator vs DownloadPanel)

## 12. 참고 자료

### Qt 문서
- Qt WebEngineDownloadItem: https://doc.qt.io/qt-5/qwebenginedownloaditem.html
- Qt WebEngineProfile: https://doc.qt.io/qt-5/qwebengineprofile.html
- Qt QStandardPaths: https://doc.qt.io/qt-5/qstandardpaths.html
- Qt QDesktopServices: https://doc.qt.io/qt-5/qdesktopservices.html
- Qt QProgressBar: https://doc.qt.io/qt-5/qprogressbar.html

### webOS 문서
- webOS LS2 API: https://webostv.developer.lge.com/develop/references/luna-service2-api/
- webOS Notification API: https://webostv.developer.lge.com/develop/references/luna-service2-api/com-webos-notification/
- webOS Application Manager: https://webostv.developer.lge.com/develop/references/luna-service2-api/com-webos-service-applicationmanager/

### 프로젝트 문서
- PRD: `docs/project/prd.md`
- CLAUDE.md: `/Users/jsong/dev/jsong1230-github/webosbrowser-native/CLAUDE.md`
- F-02 요구사항: `docs/specs/webview-integration/requirements.md`
- F-02 설계서: `docs/specs/webview-integration/design.md`

### 기술 참고
- Qt 시그널/슬롯: https://doc.qt.io/qt-5/signalsandslots.html
- 파일 다운로드 패턴: https://doc.qt.io/qt-5/qtwebengine-webenginewidgets-maps-example.html
- QListView 커스터마이징: https://doc.qt.io/qt-5/qlistview.html

## 13. 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초기 요구사항 분석서 작성 | product-manager |
