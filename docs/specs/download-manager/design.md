# 다운로드 관리 — 기술 설계서

## 1. 참조
- **요구사항 분석서**: `docs/specs/download-manager/requirements.md`
- **프로젝트 가이드**: `CLAUDE.md`
- **관련 기능**: F-02 (웹뷰 통합), F-11 (설정 화면), F-13 (리모컨 단축키)
- **Qt 문서**:
  - QWebEngineDownloadItem (Qt 5.15)
  - QWebEngineProfile
  - QStandardPaths

---

## 2. 아키텍처 개요

### 2.1 레이어 구조
```
┌──────────────────────────────────────────────────┐
│                   UI Layer                       │
│  ┌────────────────┐        ┌──────────────────┐ │
│  │ BrowserWindow  │        │  DownloadPanel   │ │
│  │  (통합/표시)    │◄──────►│  (목록/제어 UI)  │ │
│  └────────────────┘        └──────────────────┘ │
└────────────────┬─────────────────────┬───────────┘
                 │                     │
                 ▼                     ▼
┌──────────────────────────────────────────────────┐
│                Service Layer                     │
│            ┌──────────────────┐                  │
│            │ DownloadManager  │                  │
│            │  (다운로드 관리)  │                  │
│            └──────────────────┘                  │
└────────────────┬─────────────────────────────────┘
                 │ Signal/Slot (Qt 비동기)
                 ▼
┌──────────────────────────────────────────────────┐
│             Qt WebEngine Layer                   │
│  ┌──────────────────┐    ┌───────────────────┐  │
│  │QWebEngineProfile │───►│QWebEngineDownload │  │
│  │                  │    │        Item       │  │
│  └──────────────────┘    └───────────────────┘  │
└────────────────┬─────────────────────────────────┘
                 │ Qt Network
                 ▼
         ┌────────────────┐
         │  File System   │
         │  (다운로드 폴더) │
         └────────────────┘
```

### 2.2 컴포넌트 다이어그램
```
┌─────────────────┐           ┌──────────────────┐
│ BrowserWindow   │           │   WebView        │
│                 │◄──────────│                  │
│  - downloadPanel│           │  - profile_      │
│  - downloadMgr_ │           └──────────────────┘
└─────────────────┘                    │
         △                             │
         │                    downloadRequested
         │                             │
    contains                           ▼
         │                   ┌──────────────────┐
         ▼                   │QWebEngineProfile │
┌─────────────────┐          │                  │
│  DownloadPanel  │          │ + downloadReq()  │
│                 │          └──────────────────┘
│  + show()       │                   │
│  + hide()       │                   │ emits
│  + refreshList()│                   ▼
└─────────────────┘          ┌──────────────────┐
         │                   │ DownloadManager  │
         │ uses              │                  │
         └──────────────────►│ + startDownload()│
                             │ + pauseDownload()│
                             │ + cancelDownload()│
                             └──────────────────┘
                                      │
                                      │ manages
                                      ▼
                             ┌──────────────────┐
                             │QWebEngineDownload│
                             │       Item       │
                             │                  │
                             │  + accept()      │
                             │  + pause()       │
                             │  + resume()      │
                             │  + cancel()      │
                             └──────────────────┘
```

### 2.3 데이터 흐름
```
웹 페이지 → 다운로드 링크 클릭 → QWebEngineProfile::downloadRequested
     ↓
DownloadManager::startDownload() → QWebEngineDownloadItem::accept()
     ↓
downloadProgress 시그널 → DownloadManager::onProgressChanged()
     ↓
DownloadPanel::updateProgress() → UI 업데이트 (프로그레스바)
     ↓
downloadFinished 시그널 → DownloadManager::onFinished()
     ↓
파일 시스템 저장 완료 → 토스트 알림 표시
```

---

## 3. 아키텍처 결정

### 결정 1: 다운로드 엔진

**선택지**:
- A) Qt WebEngineDownloadItem 사용 (Qt 내장)
- B) libcurl 직접 사용
- C) QNetworkAccessManager 사용

**결정**: **A) Qt WebEngineDownloadItem 사용**

**근거**:
- QWebEngineView와 완전히 통합되어 브라우저 세션 유지
- 쿠키, 인증 헤더 자동 전달 (로그인 필요한 다운로드 지원)
- 진행률, 일시정지/재개 API 기본 제공
- Qt 공식 지원으로 안정성 보장
- webOS Qt 5.15 환경에서 검증됨

**트레이드오프**:
- 포기: libcurl의 세밀한 네트워크 제어, QNetworkAccessManager의 커스터마이징
- 얻음: 브라우저 통합, 단순한 API, 세션 유지

### 결정 2: 다운로드 저장 경로

**선택지**:
- A) QStandardPaths::DownloadLocation (~/Downloads)
- B) 앱별 데이터 디렉토리 (~/.local/share/webosbrowser/downloads/)
- C) webOS 공용 다운로드 폴더 (/media/internal/downloads/)

**결정**: **A) QStandardPaths::DownloadLocation (우선), B) 대안**

**근거**:
- Qt 표준 API로 크로스 플랫폼 호환성 확보
- webOS에서 QStandardPaths::DownloadLocation이 공용 다운로드 폴더로 매핑됨
- 쓰기 권한 에러 시 자동으로 앱별 디렉토리로 폴백 가능
- 다른 앱에서도 다운로드 파일 접근 가능 (파일 관리자 등)

**트레이드오프**:
- 포기: 앱 전용 샌드박스 격리
- 얻음: 사용자 편의성, 파일 관리자 통합

### 결정 3: UI 패턴 (다운로드 패널 위치)

**선택지**:
- A) 하단 슬라이드업 패널 (북마크 패널과 동일)
- B) 전체 화면 오버레이 (모달)
- C) 우측 사이드 패널 (도킹)

**결정**: **A) 하단 슬라이드업 패널**

**근거**:
- 기존 북마크/히스토리 패널과 일관된 UX
- 리모컨 포커스 관리 로직 재사용 가능
- 대화면 프로젝터에서 하단 공간 활용 (상단 URL 바와 균형)
- 패널 표시 중에도 웹 페이지 부분 확인 가능
- 슬라이드 애니메이션으로 부드러운 전환

**트레이드오프**:
- 포기: 전체 화면 표시 (더 많은 항목 표시)
- 얻음: 일관된 UX, 멀티태스킹 가능

### 결정 4: 다운로드 상태 관리

**선택지**:
- A) DownloadManager가 QWebEngineDownloadItem 직접 보관
- B) DownloadItem 구조체로 메타데이터만 저장
- C) LS2 API로 DB에 영구 저장

**결정**: **A + B) 다운로드 중은 QWebEngineDownloadItem, 완료 후 DownloadItem 구조체**

**근거**:
- 다운로드 중: QWebEngineDownloadItem 필요 (pause/resume/cancel API)
- 완료 후: Qt 객체 유지 불필요 (메모리 절약), 메타데이터만 저장
- DownloadItem 구조체로 UI에 필요한 정보만 추출
- 완료된 다운로드는 앱 재시작 시 복원하지 않음 (LS2 저장은 선택 사항)

**트레이드오프**:
- 포기: 앱 재시작 후 완료 목록 자동 복원
- 얻음: 단순한 메모리 관리, 빠른 앱 시작

### 결정 5: 일시 정지/재개 기능 지원 여부

**선택지**:
- A) Qt 5.15 pause/resume API 사용
- B) 미지원 (Qt 버전 제약 고려)
- C) Qt 6 업그레이드 후 지원

**결정**: **A) Qt 5.15 pause/resume API 사용, 런타임 확인**

**근거**:
- Qt 5.10부터 QWebEngineDownloadItem::pause(), resume() 지원
- webOS Qt 5.15 환경에서 사용 가능
- 런타임에 `QWebEngineDownloadItem::isPaused()` 메서드 존재 확인
- 미지원 시 UI에서 일시 정지 버튼 비활성화

**트레이드오프**:
- 포기: 모든 환경에서 일시 정지 보장
- 얻음: 최신 Qt 기능 활용, 사용자 편의성 향상

---

## 4. 클래스 설계

### 4.1 DownloadManager (Service Layer)

**파일**: `src/services/DownloadManager.h`, `src/services/DownloadManager.cpp`

**역할**: 다운로드 항목의 생명주기 관리 (시작, 진행, 완료, 에러)

**클래스 정의**:
```cpp
namespace webosbrowser {

/**
 * @brief 다운로드 상태 열거형
 */
enum class DownloadState {
    Pending,     ///< 대기 중 (accept 전)
    InProgress,  ///< 다운로드 중
    Paused,      ///< 일시 정지
    Completed,   ///< 완료
    Cancelled,   ///< 취소됨
    Failed       ///< 실패
};

/**
 * @brief 다운로드 메타데이터 구조체
 */
struct DownloadItem {
    QString id;                  ///< 고유 ID (UUID)
    QString fileName;            ///< 파일명
    QString filePath;            ///< 저장 경로 (완료 후)
    QUrl url;                    ///< 원본 URL
    QString mimeType;            ///< MIME 타입 (application/pdf 등)
    qint64 bytesTotal;           ///< 전체 크기 (바이트)
    qint64 bytesReceived;        ///< 다운로드된 크기
    DownloadState state;         ///< 현재 상태
    QDateTime startTime;         ///< 시작 시각
    QDateTime finishTime;        ///< 완료 시각
    QString errorMessage;        ///< 에러 메시지 (실패 시)
    qreal speed;                 ///< 다운로드 속도 (bytes/sec)
    qint64 estimatedTimeLeft;    ///< 남은 시간 (초)

    // Qt 메타타입 등록용
    DownloadItem();
};

/**
 * @brief 다운로드 관리 서비스
 *
 * QWebEngineDownloadItem과 통합하여 다운로드 생명주기 관리
 * DownloadPanel에 시그널로 상태 변경 알림
 */
class DownloadManager : public QObject {
    Q_OBJECT

public:
    explicit DownloadManager(QObject* parent = nullptr);
    ~DownloadManager() override;

    DownloadManager(const DownloadManager&) = delete;
    DownloadManager& operator=(const DownloadManager&) = delete;

    // 다운로드 제어 API

    /**
     * @brief 다운로드 시작
     * @param downloadItem Qt WebEngine 다운로드 항목
     */
    void startDownload(QWebEngineDownloadItem* downloadItem);

    /**
     * @brief 다운로드 일시 정지
     * @param id 다운로드 ID
     */
    void pauseDownload(const QString& id);

    /**
     * @brief 다운로드 재개
     * @param id 다운로드 ID
     */
    void resumeDownload(const QString& id);

    /**
     * @brief 다운로드 취소
     * @param id 다운로드 ID
     */
    void cancelDownload(const QString& id);

    /**
     * @brief 목록에서 제거 (완료/취소/실패 항목만)
     * @param id 다운로드 ID
     */
    void removeDownload(const QString& id);

    /**
     * @brief 전체 다운로드 목록 조회
     * @return 다운로드 항목 리스트
     */
    QVector<DownloadItem> getDownloads() const;

    /**
     * @brief 단일 다운로드 조회
     * @param id 다운로드 ID
     * @return 다운로드 항목 (없으면 빈 객체)
     */
    DownloadItem getDownloadById(const QString& id) const;

    /**
     * @brief 다운로드 저장 경로 설정
     * @param path 저장 경로 (빈 문자열이면 기본값)
     */
    void setDownloadPath(const QString& path);

    /**
     * @brief 현재 저장 경로 조회
     * @return 저장 경로
     */
    QString downloadPath() const;

signals:
    /**
     * @brief 새 다운로드 추가됨
     * @param item 다운로드 항목
     */
    void downloadAdded(const DownloadItem& item);

    /**
     * @brief 다운로드 진행률 변경
     * @param id 다운로드 ID
     * @param bytesReceived 수신 바이트
     * @param bytesTotal 전체 바이트
     */
    void downloadProgressChanged(const QString& id, qint64 bytesReceived, qint64 bytesTotal);

    /**
     * @brief 다운로드 상태 변경
     * @param id 다운로드 ID
     * @param state 새 상태
     */
    void downloadStateChanged(const QString& id, DownloadState state);

    /**
     * @brief 다운로드 완료
     * @param item 완료된 항목
     */
    void downloadCompleted(const DownloadItem& item);

    /**
     * @brief 다운로드 실패
     * @param id 다운로드 ID
     * @param errorMessage 에러 메시지
     */
    void downloadFailed(const QString& id, const QString& errorMessage);

private slots:
    /**
     * @brief Qt WebEngine 다운로드 진행률 핸들러
     * @param bytesReceived 수신 바이트
     * @param bytesTotal 전체 바이트
     */
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

    /**
     * @brief Qt WebEngine 다운로드 완료 핸들러
     */
    void onDownloadFinished();

    /**
     * @brief Qt WebEngine 다운로드 상태 변경 핸들러
     * @param state Qt 다운로드 상태
     */
    void onDownloadStateChanged(QWebEngineDownloadItem::DownloadState state);

private:
    /**
     * @brief 다운로드 ID 생성 (UUID)
     * @return 고유 ID
     */
    QString generateDownloadId() const;

    /**
     * @brief 파일명 중복 처리
     * @param filePath 원본 파일 경로
     * @return 중복되지 않는 파일 경로
     */
    QString resolveFilePath(const QString& filePath) const;

    /**
     * @brief 다운로드 속도 계산
     * @param id 다운로드 ID
     * @return 속도 (bytes/sec)
     */
    qreal calculateSpeed(const QString& id) const;

    /**
     * @brief 남은 시간 추정
     * @param bytesReceived 수신 바이트
     * @param bytesTotal 전체 바이트
     * @param speed 다운로드 속도
     * @return 남은 시간 (초)
     */
    qint64 estimateTimeLeft(qint64 bytesReceived, qint64 bytesTotal, qreal speed) const;

    /**
     * @brief Qt 상태를 DownloadState로 변환
     * @param qtState Qt 다운로드 상태
     * @return DownloadState
     */
    DownloadState convertQtState(QWebEngineDownloadItem::DownloadState qtState) const;

    // 멤버 변수
    QString m_downloadPath;  ///< 다운로드 저장 경로
    QVector<DownloadItem> m_downloads;  ///< 다운로드 목록 (메타데이터)
    QMap<QString, QWebEngineDownloadItem*> m_activeDownloads;  ///< 진행 중 다운로드 (Qt 객체)
    QMap<QString, qint64> m_lastBytesReceived;  ///< 속도 계산용 이전 바이트
    QMap<QString, qint64> m_lastUpdateTime;     ///< 속도 계산용 이전 시각

    static constexpr int MAX_CONCURRENT_DOWNLOADS = 3;  ///< 최대 동시 다운로드 수
};

} // namespace webosbrowser
```

### 4.2 DownloadPanel (UI Layer)

**파일**: `src/ui/DownloadPanel.h`, `src/ui/DownloadPanel.cpp`

**역할**: 다운로드 목록 표시 및 제어 버튼 제공

**클래스 정의**:
```cpp
namespace webosbrowser {

/**
 * @brief 다운로드 목록 및 제어 패널
 *
 * 다운로드 항목 리스트, 진행률 프로그레스바, 제어 버튼 표시
 * 리모컨 키 이벤트 처리 (십자키, Enter, Back)
 */
class DownloadPanel : public QWidget {
    Q_OBJECT

public:
    explicit DownloadPanel(DownloadManager* downloadManager, QWidget* parent = nullptr);
    ~DownloadPanel() override;

    DownloadPanel(const DownloadPanel&) = delete;
    DownloadPanel& operator=(const DownloadPanel&) = delete;

    /**
     * @brief 패널 표시
     */
    void show();

    /**
     * @brief 패널 숨김
     */
    void hide();

    /**
     * @brief 다운로드 목록 갱신
     */
    void refreshDownloads();

signals:
    /**
     * @brief 패널 닫기 시그널
     */
    void panelClosed();

protected:
    /**
     * @brief 키 이벤트 핸들러 (리모컨 입력)
     * @param event 키 이벤트
     */
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    /**
     * @brief 일시 정지 버튼 클릭
     */
    void onPauseClicked();

    /**
     * @brief 재개 버튼 클릭
     */
    void onResumeClicked();

    /**
     * @brief 취소 버튼 클릭
     */
    void onCancelClicked();

    /**
     * @brief 열기 버튼 클릭 (완료된 항목)
     */
    void onOpenClicked();

    /**
     * @brief 삭제 버튼 클릭 (목록에서 제거)
     */
    void onDeleteClicked();

    /**
     * @brief 모두 일시 정지 버튼 클릭
     */
    void onPauseAllClicked();

    /**
     * @brief 모두 재개 버튼 클릭
     */
    void onResumeAllClicked();

    /**
     * @brief 완료된 항목 삭제 버튼 클릭
     */
    void onClearCompletedClicked();

    /**
     * @brief 다운로드 항목 선택 변경
     * @param current 현재 선택 항목
     * @param previous 이전 선택 항목
     */
    void onSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous);

    /**
     * @brief DownloadManager 시그널 핸들러: 다운로드 추가됨
     * @param item 다운로드 항목
     */
    void onDownloadAdded(const DownloadItem& item);

    /**
     * @brief DownloadManager 시그널 핸들러: 진행률 변경
     * @param id 다운로드 ID
     * @param bytesReceived 수신 바이트
     * @param bytesTotal 전체 바이트
     */
    void onDownloadProgressChanged(const QString& id, qint64 bytesReceived, qint64 bytesTotal);

    /**
     * @brief DownloadManager 시그널 핸들러: 상태 변경
     * @param id 다운로드 ID
     * @param state 새 상태
     */
    void onDownloadStateChanged(const QString& id, DownloadState state);

    /**
     * @brief DownloadManager 시그널 핸들러: 다운로드 완료
     * @param item 완료된 항목
     */
    void onDownloadCompleted(const DownloadItem& item);

    /**
     * @brief DownloadManager 시그널 핸들러: 다운로드 실패
     * @param id 다운로드 ID
     * @param errorMessage 에러 메시지
     */
    void onDownloadFailed(const QString& id, const QString& errorMessage);

private:
    DownloadManager* m_downloadManager;  ///< 다운로드 관리자

    // UI 컴포넌트
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_headerLayout;
    QHBoxLayout* m_actionLayout;
    QLabel* m_titleLabel;
    QPushButton* m_closeButton;
    QListWidget* m_downloadList;  ///< 다운로드 항목 리스트

    // 제어 버튼 (선택된 항목용)
    QPushButton* m_pauseButton;
    QPushButton* m_resumeButton;
    QPushButton* m_cancelButton;
    QPushButton* m_openButton;
    QPushButton* m_deleteButton;

    // 전체 제어 버튼
    QPushButton* m_pauseAllButton;
    QPushButton* m_resumeAllButton;
    QPushButton* m_clearCompletedButton;

    /**
     * @brief UI 초기화
     */
    void setupUI();

    /**
     * @brief 시그널/슬롯 연결
     */
    void setupConnections();

    /**
     * @brief 다운로드 항목 위젯 생성
     * @param item 다운로드 항목
     * @return 커스텀 위젯
     */
    QWidget* createDownloadItemWidget(const DownloadItem& item);

    /**
     * @brief 다운로드 항목 위젯 업데이트
     * @param id 다운로드 ID
     * @param item 다운로드 항목
     */
    void updateDownloadItemWidget(const QString& id, const DownloadItem& item);

    /**
     * @brief 선택된 다운로드 ID 가져오기
     * @return 다운로드 ID (선택 없으면 빈 문자열)
     */
    QString getSelectedDownloadId() const;

    /**
     * @brief 제어 버튼 활성화 상태 업데이트 (선택된 항목 기준)
     */
    void updateButtonStates();

    /**
     * @brief 토스트 메시지 표시
     * @param message 메시지
     */
    void showToast(const QString& message);

    /**
     * @brief 파일 크기 포맷 (바이트 → MB/GB)
     * @param bytes 바이트
     * @return 포맷된 문자열 (예: "12.5 MB")
     */
    QString formatFileSize(qint64 bytes) const;

    /**
     * @brief 다운로드 속도 포맷
     * @param bytesPerSec 초당 바이트
     * @return 포맷된 문자열 (예: "2.3 MB/s")
     */
    QString formatSpeed(qreal bytesPerSec) const;

    /**
     * @brief 남은 시간 포맷
     * @param seconds 초
     * @return 포맷된 문자열 (예: "3분 20초")
     */
    QString formatTimeLeft(qint64 seconds) const;
};

/**
 * @brief 다운로드 항목 커스텀 위젯
 *
 * QListWidget의 각 항목으로 사용
 * 파일명, 진행률, 속도, ETA, 상태 표시
 */
class DownloadItemWidget : public QWidget {
    Q_OBJECT

public:
    explicit DownloadItemWidget(const DownloadItem& item, QWidget* parent = nullptr);

    /**
     * @brief 진행률 업데이트
     * @param bytesReceived 수신 바이트
     * @param bytesTotal 전체 바이트
     */
    void updateProgress(qint64 bytesReceived, qint64 bytesTotal);

    /**
     * @brief 상태 업데이트
     * @param state 새 상태
     */
    void updateState(DownloadState state);

    /**
     * @brief 다운로드 항목 업데이트
     * @param item 다운로드 항목
     */
    void updateItem(const DownloadItem& item);

private:
    QLabel* m_fileNameLabel;       ///< 파일명
    QProgressBar* m_progressBar;   ///< 진행률 바
    QLabel* m_progressLabel;       ///< 진행률 텍스트 (12.5 MB / 50 MB)
    QLabel* m_speedLabel;          ///< 다운로드 속도 (2.3 MB/s)
    QLabel* m_etaLabel;            ///< 남은 시간 (3분 20초)
    QLabel* m_stateLabel;          ///< 상태 (진행중/완료/에러)

    /**
     * @brief UI 초기화
     * @param item 다운로드 항목
     */
    void setupUI(const DownloadItem& item);
};

} // namespace webosbrowser
```

### 4.3 BrowserWindow 통합

**파일**: `src/browser/BrowserWindow.h` (수정)

**추가 멤버**:
```cpp
// Forward declaration
class DownloadManager;
class DownloadPanel;

class BrowserWindow : public QMainWindow {
    Q_OBJECT

public:
    // ... 기존 코드 ...

    /**
     * @brief 다운로드 패널 표시
     */
    void showDownloadPanel();

    /**
     * @brief 다운로드 패널 숨김
     */
    void hideDownloadPanel();

    /**
     * @brief 다운로드 패널 토글
     */
    void toggleDownloadPanel();

private slots:
    /**
     * @brief 다운로드 완료 알림
     * @param item 완료된 다운로드 항목
     */
    void onDownloadCompleted(const DownloadItem& item);

    /**
     * @brief 다운로드 버튼 클릭 (NavBar 또는 단축키)
     */
    void onDownloadButtonClicked();

private:
    // ... 기존 멤버 ...

    DownloadManager* m_downloadManager;   ///< 다운로드 관리자
    DownloadPanel* m_downloadPanel;       ///< 다운로드 패널
};
```

### 4.4 WebView 통합 (QWebEngineProfile 연결)

**파일**: `src/browser/WebView.cpp` (수정)

**WebViewPrivate에 추가**:
```cpp
class WebViewPrivate {
public:
    // ... 기존 코드 ...

    QWebEngineProfile* profile;  ///< WebEngine 프로필 (다운로드 이벤트 수신용)

    /**
     * @brief 다운로드 이벤트 핸들러 연결
     * @param downloadManager 다운로드 관리자
     */
    void setupDownloadHandler(DownloadManager* downloadManager);
};

void WebViewPrivate::setupDownloadHandler(DownloadManager* downloadManager) {
    profile = webEngineView->page()->profile();

    QObject::connect(profile, &QWebEngineProfile::downloadRequested,
                     [downloadManager](QWebEngineDownloadItem* downloadItem) {
        qDebug() << "WebView: 다운로드 요청 -" << downloadItem->url().toString();
        downloadManager->startDownload(downloadItem);
    });
}
```

---

## 5. 시퀀스 흐름

### 5.1 주요 시나리오: 다운로드 시작

```
사용자 → 웹 페이지 → QWebEngineView → QWebEngineProfile → DownloadManager → 파일 시스템
  │         │              │                   │                    │
  │  다운로드 링크 클릭      │                   │                    │
  │─────────────────────►  │                   │                    │
  │                        │ downloadRequested │                    │
  │                        │──────────────────►│                    │
  │                        │                   │  startDownload()   │
  │                        │                   │───────────────────►│
  │                        │                   │                    │  accept()
  │                        │                   │                    │  setDownloadDirectory()
  │                        │                   │                    │  setDownloadFileName()
  │                        │                   │                    │──────────►
  │                        │                   │  downloadAdded ▲   │
  │                        │                   │◄─────────────┘    │
  │                        │                   │                    │
  │  DownloadPanel::show() │                   │                    │
  │◄───────────────────────┴───────────────────┘                    │
  │                                                                  │
  │  "다운로드 시작: file.pdf"                                        │
  │◄─────────────────────────────────────────────────────────────────┘
```

### 5.2 진행률 업데이트 시나리오

```
Qt Network → QWebEngineDownloadItem → DownloadManager → DownloadPanel → UI
  │               │                       │                 │
  │  progress     │                       │                 │
  │──────────────►│  downloadProgress     │                 │
  │               │──────────────────────►│                 │
  │               │                       │  onProgressChanged()
  │               │                       │  - 속도 계산     │
  │               │                       │  - ETA 추정      │
  │               │                       │                 │
  │               │    downloadProgressChanged              │
  │               │       (id, bytes, total)                │
  │               │─────────────────────────────────────────►│
  │               │                       │                 │  updateProgress()
  │               │                       │                 │  - 프로그레스바 업데이트
  │               │                       │                 │  - 속도/ETA 표시
  │               │                       │                 │──────────►
```

### 5.3 다운로드 완료 시나리오

```
Qt Network → QWebEngineDownloadItem → DownloadManager → BrowserWindow → DownloadPanel
  │               │                       │                 │                │
  │  finished     │                       │                 │                │
  │──────────────►│  onDownloadFinished() │                 │                │
  │               │──────────────────────►│                 │                │
  │               │                       │  - 파일 존재 확인 │                │
  │               │                       │  - 크기 검증     │                │
  │               │                       │  - 상태: Completed                │
  │               │                       │                 │                │
  │               │    downloadCompleted(item)              │                │
  │               │─────────────────────────────────────────►│                │
  │               │                       │                 │  showToast()   │
  │               │                       │                 │  "다운로드 완료: file.pdf"
  │               │                       │                 │                │
  │               │                       │   downloadStateChanged(Completed)│
  │               │─────────────────────────────────────────────────────────►│
  │               │                       │                 │                │  updateState()
  │               │                       │                 │                │  - 상태 레이블: "완료"
  │               │                       │                 │                │  - 열기 버튼 활성화
```

### 5.4 에러 시나리오: 네트워크 끊김

```
사용자 → 네트워크 → QWebEngineDownloadItem → DownloadManager → DownloadPanel
  │         │               │                       │                 │
  │  Wi-Fi OFF              │                       │                 │
  │─────────►               │                       │                 │
  │                         │  finished (error)     │                 │
  │                         │──────────────────────►│                 │
  │                         │                       │  onDownloadFinished()
  │                         │                       │  - state: Failed
  │                         │                       │  - errorMessage 저장
  │                         │                       │                 │
  │                         │    downloadFailed(id, "네트워크 에러")     │
  │                         │─────────────────────────────────────────►│
  │                         │                       │                 │  updateState()
  │                         │                       │                 │  - 상태: "실패"
  │                         │                       │                 │  - 에러 메시지 표시
  │                         │                       │                 │  - 재시도 버튼 (선택)
```

---

## 6. API 설계

### 6.1 DownloadManager 공개 API

| 메서드 | 파라미터 | 반환값 | 설명 |
|--------|---------|--------|------|
| `startDownload` | `QWebEngineDownloadItem*` | `void` | 다운로드 시작 (QWebEngineProfile::downloadRequested 핸들러에서 호출) |
| `pauseDownload` | `const QString& id` | `void` | 다운로드 일시 정지 (Qt 5.10+ 지원) |
| `resumeDownload` | `const QString& id` | `void` | 다운로드 재개 |
| `cancelDownload` | `const QString& id` | `void` | 다운로드 취소 (부분 파일 삭제) |
| `removeDownload` | `const QString& id` | `void` | 목록에서 제거 (완료/취소/실패 항목만) |
| `getDownloads` | - | `QVector<DownloadItem>` | 전체 다운로드 목록 조회 |
| `getDownloadById` | `const QString& id` | `DownloadItem` | 단일 다운로드 조회 |
| `setDownloadPath` | `const QString& path` | `void` | 다운로드 저장 경로 설정 |
| `downloadPath` | - | `QString` | 현재 저장 경로 조회 |

### 6.2 DownloadManager 시그널

| 시그널 | 파라미터 | 설명 |
|--------|---------|------|
| `downloadAdded` | `const DownloadItem&` | 새 다운로드 추가됨 (UI 갱신용) |
| `downloadProgressChanged` | `QString id, qint64 received, qint64 total` | 진행률 변경 (500ms 간격) |
| `downloadStateChanged` | `QString id, DownloadState state` | 상태 변경 (진행중 → 일시정지 등) |
| `downloadCompleted` | `const DownloadItem&` | 다운로드 완료 (토스트 알림용) |
| `downloadFailed` | `QString id, QString errorMessage` | 다운로드 실패 (에러 처리용) |

### 6.3 DownloadPanel 공개 API

| 메서드 | 파라미터 | 반환값 | 설명 |
|--------|---------|--------|------|
| `show` | - | `void` | 패널 표시 (슬라이드 애니메이션) |
| `hide` | - | `void` | 패널 숨김 |
| `refreshDownloads` | - | `void` | 다운로드 목록 갱신 (DownloadManager에서 조회) |

---

## 7. 데이터 모델

### 7.1 DownloadItem 구조체

| 필드 | 타입 | 설명 |
|------|------|------|
| `id` | `QString` | 고유 ID (UUID, 예: "a1b2c3d4-e5f6-...") |
| `fileName` | `QString` | 파일명 (예: "document.pdf") |
| `filePath` | `QString` | 저장 경로 (완료 후, 예: "/media/downloads/document.pdf") |
| `url` | `QUrl` | 원본 URL (예: "https://example.com/file.pdf") |
| `mimeType` | `QString` | MIME 타입 (예: "application/pdf") |
| `bytesTotal` | `qint64` | 전체 크기 (바이트) |
| `bytesReceived` | `qint64` | 다운로드된 크기 (바이트) |
| `state` | `DownloadState` | 현재 상태 (Pending, InProgress, Completed 등) |
| `startTime` | `QDateTime` | 시작 시각 |
| `finishTime` | `QDateTime` | 완료 시각 |
| `errorMessage` | `QString` | 에러 메시지 (실패 시) |
| `speed` | `qreal` | 다운로드 속도 (bytes/sec) |
| `estimatedTimeLeft` | `qint64` | 남은 시간 (초) |

### 7.2 DownloadState 열거형

| 값 | 설명 |
|----|------|
| `Pending` | 대기 중 (accept() 호출 전) |
| `InProgress` | 다운로드 중 |
| `Paused` | 일시 정지 |
| `Completed` | 완료 |
| `Cancelled` | 취소됨 |
| `Failed` | 실패 (네트워크 에러, 디스크 에러 등) |

---

## 8. 파일 시스템 설계

### 8.1 다운로드 저장 경로

**기본 경로**:
```cpp
QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
// webOS: "/media/internal/downloads/" 또는 "/home/root/Downloads/"
```

**폴백 경로** (쓰기 권한 없을 때):
```cpp
QString fallbackPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/downloads";
// webOS: "/home/root/.local/share/webosbrowser/downloads/"
```

### 8.2 파일명 중복 처리

**로직**:
```cpp
QString DownloadManager::resolveFilePath(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.baseName();  // "document"
    QString suffix = fileInfo.suffix();      // "pdf"
    QString dirPath = fileInfo.absolutePath();

    if (!QFile::exists(filePath)) {
        return filePath;  // 중복 없음
    }

    // 중복 시 번호 추가
    int counter = 1;
    QString newFilePath;
    do {
        newFilePath = QString("%1/%2 (%3).%4")
                      .arg(dirPath)
                      .arg(baseName)
                      .arg(counter++)
                      .arg(suffix);
    } while (QFile::exists(newFilePath));

    return newFilePath;
}
```

**예시**:
- 원본: `/media/downloads/document.pdf`
- 중복 시: `/media/downloads/document (1).pdf`
- 재중복 시: `/media/downloads/document (2).pdf`

### 8.3 디렉토리 자동 생성

```cpp
void DownloadManager::setDownloadPath(const QString& path) {
    QString targetPath = path.isEmpty()
                         ? QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
                         : path;

    QDir dir(targetPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "다운로드 디렉토리 생성 실패:" << targetPath;
            // 폴백 경로 사용
            targetPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/downloads";
            QDir fallbackDir(targetPath);
            fallbackDir.mkpath(".");
        }
    }

    m_downloadPath = targetPath;
}
```

---

## 9. 메모리 관리

### 9.1 메모리 최적화 전략

**원칙**:
1. **진행 중 다운로드**: QWebEngineDownloadItem 보관 (pause/resume API 필요)
2. **완료된 다운로드**: DownloadItem 구조체만 보관 (Qt 객체 해제)
3. **최대 항목 수**: 완료/취소 항목은 100개까지 유지 (오래된 것 자동 삭제)

**구현**:
```cpp
void DownloadManager::onDownloadFinished() {
    QWebEngineDownloadItem* downloadItem = qobject_cast<QWebEngineDownloadItem*>(sender());
    if (!downloadItem) return;

    // DownloadItem 구조체로 메타데이터 추출
    DownloadItem item = extractMetadata(downloadItem);

    // 완료된 다운로드는 Qt 객체 제거 (메모리 절약)
    m_activeDownloads.remove(item.id);

    // 메타데이터만 보관
    m_downloads.append(item);

    // 최대 100개 유지
    if (m_downloads.size() > 100) {
        m_downloads.removeFirst();
    }

    emit downloadCompleted(item);
}
```

### 9.2 스마트 포인터 사용

**QWebEngineDownloadItem 관리**:
```cpp
// DownloadManager 멤버
QMap<QString, QWebEngineDownloadItem*> m_activeDownloads;  // Qt parent-child 메모리 관리

// 다운로드 시작
void DownloadManager::startDownload(QWebEngineDownloadItem* downloadItem) {
    // Qt 객체는 QWebEngineProfile의 자식으로 자동 관리됨
    // 명시적 delete 불필요

    QString id = generateDownloadId();
    m_activeDownloads.insert(id, downloadItem);

    // 완료 시 맵에서 제거 (onDownloadFinished에서)
}
```

---

## 10. 성능 고려사항

### 10.1 진행률 업데이트 주기

**문제**: `downloadProgress` 시그널이 너무 자주 발생하면 UI 성능 저하

**해결책**: 쓰로틀링 (500ms 간격으로 UI 업데이트)

```cpp
void DownloadManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    QWebEngineDownloadItem* downloadItem = qobject_cast<QWebEngineDownloadItem*>(sender());
    QString id = findDownloadId(downloadItem);

    // 마지막 업데이트 시각 확인
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_lastUpdateTime.contains(id)) {
        qint64 elapsed = now - m_lastUpdateTime[id];
        if (elapsed < 500) {  // 500ms 이내면 스킵
            return;
        }
    }

    m_lastUpdateTime[id] = now;

    // 속도 계산 및 시그널 emit
    // ...
}
```

### 10.2 동시 다운로드 제한

**이유**: 네트워크 대역폭 분산, 메모리 사용량 제한

**구현**:
```cpp
constexpr int MAX_CONCURRENT_DOWNLOADS = 3;

void DownloadManager::startDownload(QWebEngineDownloadItem* downloadItem) {
    // 진행 중 다운로드 수 확인
    int activeCount = 0;
    for (const auto& item : m_downloads) {
        if (item.state == DownloadState::InProgress) {
            activeCount++;
        }
    }

    if (activeCount >= MAX_CONCURRENT_DOWNLOADS) {
        // 대기열에 추가 (선택 사항)
        qWarning() << "최대 동시 다운로드 수 도달 (3개)";
        downloadItem->cancel();
        return;
    }

    // 다운로드 시작
    downloadItem->accept();
}
```

### 10.3 UI 렌더링 최적화

**QListWidget 가상화**:
```cpp
// DownloadPanel 생성자
m_downloadList = new QListWidget(this);
m_downloadList->setUniformItemSizes(true);  // 균일한 항목 크기 (렌더링 최적화)
m_downloadList->setLayoutMode(QListView::Batched);  // 배치 레이아웃 (스크롤 성능 향상)
```

**커스텀 위젯 재사용**:
```cpp
void DownloadPanel::updateDownloadItemWidget(const QString& id, const DownloadItem& item) {
    // 새 위젯 생성 대신 기존 위젯 업데이트
    for (int i = 0; i < m_downloadList->count(); ++i) {
        QListWidgetItem* listItem = m_downloadList->item(i);
        if (listItem->data(Qt::UserRole).toString() == id) {
            DownloadItemWidget* widget = qobject_cast<DownloadItemWidget*>(
                m_downloadList->itemWidget(listItem)
            );
            widget->updateItem(item);  // 재사용
            return;
        }
    }
}
```

---

## 11. UI 레이아웃 구조

### 11.1 DownloadPanel 레이아웃

```
┌──────────────────────────────────────────────────────────┐
│                      DownloadPanel                       │
├──────────────────────────────────────────────────────────┤
│  [다운로드 (3)]                                  [닫기 X] │
├──────────────────────────────────────────────────────────┤
│  ┌────────────────────────────────────────────────────┐  │
│  │  document.pdf                              진행중   │  │
│  │  ████████████████░░░░░░░░░░░░░░░░  60%            │  │
│  │  12.5 MB / 20 MB  |  2.3 MB/s  |  3분 20초 남음    │  │
│  │  [일시정지] [취소]                                  │  │
│  └────────────────────────────────────────────────────┘  │
│  ┌────────────────────────────────────────────────────┐  │
│  │  image.png                                   완료   │  │
│  │  ████████████████████████████████████████  100%    │  │
│  │  5.2 MB / 5.2 MB                                   │  │
│  │  [열기] [삭제]                                      │  │
│  └────────────────────────────────────────────────────┘  │
│  ┌────────────────────────────────────────────────────┐  │
│  │  video.mp4                                   에러   │  │
│  │  ████████░░░░░░░░░░░░░░░░░░░░░░░░░░  25%           │  │
│  │  네트워크 연결이 끊겼습니다                         │  │
│  │  [재시도] [삭제]                                    │  │
│  └────────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────┤
│  [모두 일시정지] [모두 재개] [완료된 항목 삭제]          │
└──────────────────────────────────────────────────────────┘
```

### 11.2 리모컨 포커스 경로

```
십자키 위/아래: 다운로드 항목 간 이동
    ▲
    │
[document.pdf] ◄─── 십자키 좌/우 ───► [일시정지] [취소]
    │
    ▼
[image.png]    ◄─── 십자키 좌/우 ───► [열기] [삭제]
    │
    ▼
[video.mp4]    ◄─── 십자키 좌/우 ───► [재시도] [삭제]
    │
    ▼
[모두 일시정지] [모두 재개] [완료된 항목 삭제]
    │
    ▼
[닫기 X] ──► Back 버튼으로 패널 닫기
```

---

## 12. 에러 처리

### 12.1 에러 타입 및 대응

| 에러 타입 | 감지 방법 | 대응 방안 |
|----------|----------|----------|
| 네트워크 끊김 | `QWebEngineDownloadItem::state() == Interrupted` | 상태: Failed, 에러 메시지: "네트워크 연결이 끊겼습니다", 재시도 버튼 제공 |
| 디스크 공간 부족 | `downloadFinished` + 파일 크기 불일치 | 상태: Failed, 에러 메시지: "디스크 공간이 부족합니다", 부분 파일 삭제 |
| 파일 쓰기 권한 없음 | `QDir::mkpath()` 실패 | 폴백 경로로 재시도, 실패 시 에러 다이얼로그 표시 |
| 서버 에러 (403, 404) | `downloadFinished` + HTTP 상태 코드 | 상태: Failed, 에러 메시지: "서버에서 파일을 찾을 수 없습니다 (404)" |
| 타임아웃 | Qt Network 타임아웃 (60초) | 상태: Failed, 에러 메시지: "다운로드 타임아웃" |

### 12.2 에러 복구 로직

```cpp
void DownloadManager::onDownloadFinished() {
    QWebEngineDownloadItem* downloadItem = qobject_cast<QWebEngineDownloadItem*>(sender());
    QString id = findDownloadId(downloadItem);

    if (downloadItem->state() == QWebEngineDownloadItem::DownloadCompleted) {
        // 성공 처리
        handleSuccess(id, downloadItem);
    } else {
        // 에러 처리
        QString errorMessage;

        switch (downloadItem->state()) {
            case QWebEngineDownloadItem::DownloadInterrupted:
                errorMessage = "네트워크 연결이 끊겼습니다";
                break;
            case QWebEngineDownloadItem::DownloadCancelled:
                errorMessage = "사용자가 다운로드를 취소했습니다";
                break;
            default:
                errorMessage = "알 수 없는 에러가 발생했습니다";
        }

        updateDownloadState(id, DownloadState::Failed, errorMessage);
        emit downloadFailed(id, errorMessage);

        // 부분 다운로드 파일 삭제
        QString partialFilePath = downloadItem->downloadDirectory() + "/" + downloadItem->downloadFileName();
        QFile::remove(partialFilePath);
    }
}
```

---

## 13. 리모컨 입력 처리

### 13.1 키 매핑

| 리모컨 버튼 | Qt 키 | 동작 |
|------------|-------|------|
| 위/아래 십자키 | `Qt::Key_Up`, `Qt::Key_Down` | 다운로드 항목 간 이동 |
| 좌/우 십자키 | `Qt::Key_Left`, `Qt::Key_Right` | 버튼 간 이동 |
| Enter (선택) | `Qt::Key_Return`, `Qt::Key_Enter` | 버튼 클릭 또는 항목 더블클릭 (열기) |
| Back | `Qt::Key_Escape` | 패널 닫기 |
| 컬러 버튼 (Blue) | `Qt::Key_F1` (예시) | 다운로드 패널 토글 (F-13 연계) |

### 13.2 키 이벤트 핸들러

```cpp
void DownloadPanel::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Up:
            // 이전 다운로드 항목으로 이동
            {
                int currentRow = m_downloadList->currentRow();
                if (currentRow > 0) {
                    m_downloadList->setCurrentRow(currentRow - 1);
                }
            }
            break;

        case Qt::Key_Down:
            // 다음 다운로드 항목으로 이동
            {
                int currentRow = m_downloadList->currentRow();
                if (currentRow < m_downloadList->count() - 1) {
                    m_downloadList->setCurrentRow(currentRow + 1);
                }
            }
            break;

        case Qt::Key_Return:
        case Qt::Key_Enter:
            // 완료된 항목 열기 (더블클릭 동작)
            {
                QString id = getSelectedDownloadId();
                DownloadItem item = m_downloadManager->getDownloadById(id);
                if (item.state == DownloadState::Completed) {
                    openDownloadedFile(item.filePath);
                }
            }
            break;

        case Qt::Key_Escape:
            // 패널 닫기
            hide();
            emit panelClosed();
            break;

        default:
            QWidget::keyPressEvent(event);  // 부모 클래스로 전달
    }
}
```

---

## 14. 파일 열기 (완료된 다운로드)

### 14.1 webOS LS2 API 사용

**우선순위**: webOS 네이티브 앱으로 열기 시도

```cpp
void DownloadPanel::openDownloadedFile(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        showToast("파일을 찾을 수 없습니다");
        return;
    }

    QString mimeType = getMimeType(filePath);

    // LS2 API 호출 (webOS Application Manager)
    QString lunaCommand = QString(
        "luna://com.webos.service.applicationmanager/launch "
        "'{\"id\":\"com.webos.app.viewer\",\"params\":{\"target\":\"%1\"}}'")
        .arg(filePath);

    QProcess process;
    process.start(lunaCommand);
    process.waitForFinished(3000);

    if (process.exitCode() != 0) {
        // LS2 API 실패 시 Qt 대안 사용
        openWithQt(filePath);
    }
}
```

### 14.2 Qt 대안 (QDesktopServices)

**폴백**: LS2 API 실패 시

```cpp
void DownloadPanel::openWithQt(const QString& filePath) {
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    if (!QDesktopServices::openUrl(fileUrl)) {
        showToast("이 파일 형식을 열 수 없습니다");
    }
}
```

### 14.3 MIME 타입 감지

```cpp
QString DownloadPanel::getMimeType(const QString& filePath) const {
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(filePath);
    return mimeType.name();
}
```

---

## 15. 토스트 알림

### 15.1 webOS 토스트 API (선호)

```cpp
void BrowserWindow::onDownloadCompleted(const DownloadItem& item) {
    // LS2 API 호출
    QString lunaCommand = QString(
        "luna://com.webos.notification/createToast "
        "'{\"message\":\"다운로드 완료: %1\",\"iconUrl\":\"/usr/palm/applications/webosbrowser/icon.png\"}'")
        .arg(item.fileName);

    QProcess process;
    process.start(lunaCommand);
    process.waitForFinished(1000);
}
```

### 15.2 Qt 자체 구현 (대안)

```cpp
void DownloadPanel::showToast(const QString& message) {
    // 간단한 QLabel 기반 토스트
    QLabel* toast = new QLabel(message, this);
    toast->setStyleSheet(
        "QLabel {"
        "  background-color: rgba(0, 0, 0, 0.8);"
        "  color: white;"
        "  padding: 16px 24px;"
        "  border-radius: 8px;"
        "  font-size: 18px;"
        "}"
    );
    toast->setAlignment(Qt::AlignCenter);
    toast->setGeometry(
        (width() - 400) / 2,  // 중앙 정렬
        height() - 100,       // 하단
        400, 60
    );
    toast->show();

    // 3초 후 자동 삭제
    QTimer::singleShot(3000, toast, &QLabel::deleteLater);
}
```

---

## 16. 영향 범위 분석

### 16.1 수정 필요한 기존 파일

| 파일 경로 | 변경 내용 | 이유 |
|----------|----------|------|
| `src/browser/BrowserWindow.h` | `DownloadManager*`, `DownloadPanel*` 멤버 추가 | 다운로드 관리자/패널 통합 |
| `src/browser/BrowserWindow.cpp` | `showDownloadPanel()`, `onDownloadCompleted()` 구현 | 패널 표시 및 알림 처리 |
| `src/browser/WebView.cpp` | `QWebEngineProfile::downloadRequested` 연결 | 다운로드 이벤트 감지 |
| `src/browser/WebViewPrivate` | `setupDownloadHandler()` 메서드 추가 | DownloadManager와 연결 |
| `CMakeLists.txt` | DownloadManager, DownloadPanel 소스 추가 | 빌드 설정 |

### 16.2 새로 생성할 파일

| 파일 경로 | 역할 |
|----------|------|
| `src/services/DownloadManager.h` | 다운로드 관리 서비스 헤더 |
| `src/services/DownloadManager.cpp` | 다운로드 관리 서비스 구현 |
| `src/ui/DownloadPanel.h` | 다운로드 패널 UI 헤더 |
| `src/ui/DownloadPanel.cpp` | 다운로드 패널 UI 구현 |
| `src/models/DownloadItem.h` (선택) | DownloadItem 구조체 분리 (모델 레이어) |

### 16.3 영향 받는 기존 기능

| 기능 | 영향 내용 | 대응 방안 |
|------|----------|----------|
| F-02 (웹뷰 통합) | QWebEngineProfile에 downloadRequested 핸들러 추가 | WebView 초기화 로직에 setupDownloadHandler() 호출 |
| F-13 (리모컨 단축키) | 다운로드 패널 토글 단축키 추가 | KeyboardHandler에 Blue 버튼 → toggleDownloadPanel() 매핑 |
| F-11 (설정 화면) | 다운로드 저장 경로 설정 UI 추가 | 설정 화면에 "다운로드 폴더" 항목 추가, DownloadManager::setDownloadPath() 호출 |

---

## 17. 기술적 주의사항

### 17.1 Qt WebEngine 제약사항

- **pause/resume 제한**: Qt 5.10부터 지원. 런타임에 메서드 존재 확인 필요.
- **다운로드 경로 변경 타이밍**: `accept()` 호출 전에 `setDownloadDirectory()`, `setDownloadFileName()` 설정 필수.
- **QWebEngineDownloadItem 수명**: 다운로드 완료 후에도 Qt 객체가 유지되므로 명시적으로 맵에서 제거 필요.
- **브라우저 세션 유지**: 쿠키, 인증 헤더가 자동으로 다운로드 요청에 포함됨 (로그인 필요 파일 다운로드 지원).

### 17.2 파일 시스템 주의사항

- **쓰기 권한 확인**: 다운로드 시작 전 `QFileInfo::isWritable()` 확인.
- **디스크 공간 확인**: `QStorageInfo::bytesAvailable()` > `bytesTotal` 확인.
- **부분 파일 정리**: 다운로드 취소/실패 시 부분 파일(.crdownload 등) 삭제.
- **파일명 유효성 검증**: 특수 문자(`/`, `\`, `<`, `>` 등) 제거 또는 대체.

### 17.3 동시성 주의사항

- **진행률 업데이트 쓰로틀링**: UI 과부하 방지 (500ms 간격).
- **동시 다운로드 제한**: 최대 3개로 네트워크 대역폭 분산.
- **시그널/슬롯 비동기 처리**: DownloadManager는 워커 스레드 없이 메인 스레드에서 실행 (Qt 네트워크는 자체적으로 비동기).

### 17.4 메모리 주의사항

- **완료된 다운로드는 Qt 객체 제거**: 메타데이터만 DownloadItem 구조체로 보관.
- **최대 100개 항목 유지**: 오래된 완료/취소 항목 자동 삭제.
- **대용량 파일(1GB+) 주의**: 진행률 업데이트 빈도 조정 (1초 간격).

---

## 18. 테스트 계획

### 18.1 단위 테스트

| 테스트 케이스 | 입력 | 예상 출력 |
|-------------|------|----------|
| `resolveFilePath` | `/downloads/file.pdf` (존재) | `/downloads/file (1).pdf` |
| `calculateSpeed` | 1MB 수신, 1초 경과 | 1048576 bytes/sec |
| `estimateTimeLeft` | 10MB 수신, 50MB 전체, 1MB/s | 40초 |
| `formatFileSize` | 1048576 bytes | "1.0 MB" |
| `formatSpeed` | 2457600 bytes/sec | "2.3 MB/s" |

### 18.2 통합 테스트

| 테스트 시나리오 | 단계 | 검증 |
|---------------|------|------|
| PDF 다운로드 | 1. PDF 링크 클릭<br>2. 다운로드 시작<br>3. 완료 대기 | 1. downloadAdded 시그널 수신<br>2. 파일 존재 확인<br>3. 파일 크기 일치<br>4. downloadCompleted 시그널 수신 |
| 이미지 다운로드 | 동일 | 동일 |
| 대용량 파일(100MB+) | 동일 + 진행률 확인 | 진행률 0% → 100% 업데이트 확인 |
| 다운로드 일시 정지 | 1. 다운로드 시작<br>2. pauseDownload() 호출<br>3. resumeDownload() 호출 | 1. 상태: InProgress<br>2. 상태: Paused, 진행률 정지<br>3. 상태: InProgress, 진행률 재개 |
| 다운로드 취소 | 1. 다운로드 시작<br>2. cancelDownload() 호출 | 1. 상태: InProgress<br>2. 상태: Cancelled<br>3. 부분 파일 삭제 확인 |
| 네트워크 끊김 | 1. 다운로드 시작<br>2. Wi-Fi 끄기 | 1. 상태: InProgress<br>2. 상태: Failed<br>3. 에러 메시지: "네트워크 연결이 끊겼습니다" |
| 파일명 중복 | 1. file.pdf 다운로드<br>2. 다시 file.pdf 다운로드 | 2. file (1).pdf로 저장 |
| 리모컨 조작 | 1. 패널 열기<br>2. 십자키 이동<br>3. Enter 키 | 1. 패널 표시<br>2. 포커스 이동<br>3. 버튼 클릭 또는 파일 열기 |

### 18.3 실제 디바이스 테스트

**LG 프로젝터 HU715QW**:
- [ ] webOS QStandardPaths::DownloadLocation 경로 확인
- [ ] webOS LS2 API (토스트 알림, 파일 열기) 동작 확인
- [ ] 리모컨 단축키 (컬러 버튼) 매핑 확인
- [ ] 대화면 UI 가독성 (프로그레스바, 폰트 크기) 확인
- [ ] 다운로드 중 브라우징 성능 확인 (멀티태스킹)

---

## 19. 확장 가능성

### 19.1 향후 추가 가능 기능

- **다운로드 히스토리 저장**: LS2 API로 완료된 다운로드 영구 보관 (F-08 연계)
- **다운로드 우선순위**: 여러 다운로드 중 순서 조정
- **다운로드 대역폭 제한**: 설정에서 최대 속도 제한 (F-11 연계)
- **다운로드 재개 (Resume)**: 앱 재시작 후 부분 파일로부터 재개 (HTTP Range 요청)
- **다운로드 보안 스캔**: 실행 파일(.exe, .sh) 경고 표시
- **다운로드 동기화**: LG 계정 연동으로 다른 기기와 목록 공유

### 19.2 아키텍처 확장 지점

- **DownloadManager**: 다운로드 큐 관리 로직 추가 (우선순위 큐)
- **DownloadPanel**: 정렬 옵션 추가 (이름, 크기, 날짜)
- **StorageService**: 다운로드 히스토리 저장/조회 메서드 추가

---

## 변경 이력

| 날짜 | 변경 내용 | 이유 |
|------|-----------|------|
| 2026-02-14 | 최초 설계서 작성 | F-12 다운로드 관리 기능 구현을 위한 기술 설계 |
