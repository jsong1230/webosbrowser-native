# DownloadManager 컴포넌트 문서

## 개요

**위치**: `src/services/DownloadManager.h`, `src/services/DownloadManager.cpp`

**역할**: 다운로드 항목의 생명주기 관리 (시작, 진행, 완료, 에러)

**설계 문서**: `docs/specs/download-manager/design.md`

---

## 클래스 구조

### DownloadItem 구조체

```cpp
struct DownloadItem {
    QString id;                  // 고유 ID (UUID)
    QString fileName;            // 파일명
    QString filePath;            // 저장 경로 (완료 후)
    QUrl url;                    // 원본 URL
    QString mimeType;            // MIME 타입
    qint64 bytesTotal;           // 전체 크기 (바이트)
    qint64 bytesReceived;        // 다운로드된 크기
    DownloadState state;         // 현재 상태
    QDateTime startTime;         // 시작 시각
    QDateTime finishTime;        // 완료 시각
    QString errorMessage;        // 에러 메시지 (실패 시)
    qreal speed;                 // 다운로드 속도 (bytes/sec)
    qint64 estimatedTimeLeft;    // 남은 시간 (초)
};
```

### DownloadState 열거형

```cpp
enum class DownloadState {
    Pending,     // 대기 중 (accept 전)
    InProgress,  // 다운로드 중
    Paused,      // 일시 정지
    Completed,   // 완료
    Cancelled,   // 취소됨
    Failed       // 실패
};
```

### DownloadManager 클래스

```cpp
class DownloadManager : public QObject {
    Q_OBJECT

public:
    explicit DownloadManager(QObject* parent = nullptr);
    ~DownloadManager() override;

    // 다운로드 제어 API
    void startDownload(QWebEngineDownloadItem* downloadItem);
    void pauseDownload(const QString& id);
    void resumeDownload(const QString& id);
    void cancelDownload(const QString& id);
    void removeDownload(const QString& id);

    // 조회 API
    QVector<DownloadItem> getDownloads() const;
    DownloadItem getDownloadById(const QString& id) const;

    // 설정 API
    void setDownloadPath(const QString& path);
    QString downloadPath() const;

signals:
    void downloadAdded(const DownloadItem& item);
    void downloadProgressChanged(const QString& id, qint64 bytesReceived, qint64 bytesTotal);
    void downloadStateChanged(const QString& id, DownloadState state);
    void downloadCompleted(const DownloadItem& item);
    void downloadFailed(const QString& id, const QString& errorMessage);
};
```

---

## 주요 기능

### 1. 다운로드 시작

```cpp
void DownloadManager::startDownload(QWebEngineDownloadItem* downloadItem)
```

- QWebEngineProfile::downloadRequested 시그널에서 호출
- 고유 ID 생성 (UUID)
- 파일 저장 경로 설정 (QStandardPaths::DownloadLocation)
- 파일명 중복 처리 (자동 번호 추가)
- Qt 시그널 연결 (downloadProgress, finished, stateChanged)
- `accept()` 호출하여 다운로드 시작
- `downloadAdded` 시그널 emit

**동시 다운로드 제한**: 최대 3개 (MAX_CONCURRENT_DOWNLOADS)

### 2. 진행률 추적

```cpp
void DownloadManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
```

- **쓰로틀링**: 500ms 간격으로 UI 업데이트
- 다운로드 속도 계산 (bytes/sec)
- 남은 시간 추정 (ETA)
- `downloadProgressChanged` 시그널 emit

**속도 계산 로직**:
```cpp
qreal speed = (currentBytes - lastBytes) / timeDiff;
```

**ETA 추정 로직**:
```cpp
qint64 secondsLeft = (bytesTotal - bytesReceived) / speed;
```

### 3. 완료 및 에러 처리

```cpp
void DownloadManager::onDownloadFinished()
```

**성공 시**:
- 파일 존재 확인 (QFile::exists)
- 파일 크기 검증
- 상태 변경: `Completed`
- `downloadCompleted` 시그널 emit
- 활성 다운로드 목록에서 제거

**실패 시**:
- 에러 메시지 생성 (네트워크 끊김, 디스크 공간 부족 등)
- 상태 변경: `Failed`
- `downloadFailed` 시그널 emit
- 부분 파일 삭제 (QFile::remove)

### 4. 제어 API

**일시 정지**:
```cpp
downloadItem->pause();  // Qt 5.10+
```

**재개**:
```cpp
downloadItem->resume();
```

**취소**:
```cpp
downloadItem->cancel();
// 부분 파일 삭제
QFile::remove(item.filePath);
```

---

## 파일 저장 경로

### 기본 경로
```cpp
QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
// webOS: "/media/internal/downloads/" 또는 "/home/root/Downloads/"
```

### 폴백 경로
```cpp
QString fallbackPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/downloads";
// webOS: "/home/root/.local/share/webosbrowser/downloads/"
```

### 파일명 중복 처리

```cpp
QString DownloadManager::resolveFilePath(const QString& filePath)
```

- 중복 파일 존재 시 자동으로 번호 추가
- 예시: `document.pdf` → `document (1).pdf` → `document (2).pdf`

---

## 메모리 관리

### 전략

1. **진행 중 다운로드**: QWebEngineDownloadItem 보관 (pause/resume API 필요)
2. **완료된 다운로드**: DownloadItem 구조체만 보관 (Qt 객체 해제)
3. **최대 항목 수**: 완료/취소 항목은 100개까지 유지 (오래된 것 자동 삭제)

### Qt Parent-Child 메모리 관리

```cpp
// DownloadManager는 QObject 상속
// QWebEngineDownloadItem은 QWebEngineProfile의 자식으로 자동 관리
// 명시적 delete 불필요
```

---

## 사용 예시

### WebView에서 연결

```cpp
// WebViewPrivate::setupDownloadHandler
QWebEngineProfile* profile = webEngineView->page()->profile();
connect(profile, &QWebEngineProfile::downloadRequested,
        [downloadManager](QWebEngineDownloadItem* downloadItem) {
    downloadManager->startDownload(downloadItem);
});
```

### BrowserWindow에서 알림 처리

```cpp
// BrowserWindow::setupConnections
connect(downloadManager_, &DownloadManager::downloadCompleted,
        this, &BrowserWindow::onDownloadCompleted);

void BrowserWindow::onDownloadCompleted(const DownloadItem& item) {
    statusLabel_->setText("다운로드 완료: " + item.fileName);
}
```

---

## 성능 최적화

### 쓰로틀링

- 진행률 업데이트 주기: 500ms 간격
- UI 과부하 방지

### 동시 다운로드 제한

- 최대 3개 동시 진행
- 네트워크 대역폭 분산

### 메모리 최적화

- 완료된 다운로드는 Qt 객체 제거
- 메타데이터만 DownloadItem 구조체로 보관
- 최대 100개 항목 유지

---

## 주의사항

### Qt WebEngine 제약

- **pause/resume**: Qt 5.10부터 지원
- **다운로드 경로 변경 타이밍**: `accept()` 호출 전에 설정 필수
- **QWebEngineDownloadItem 수명**: 완료 후에도 Qt 객체 유지되므로 맵에서 제거 필요

### 파일 시스템

- **쓰기 권한 확인**: 다운로드 시작 전 확인
- **디스크 공간 확인**: 선택 사항 (QStorageInfo::bytesAvailable)
- **부분 파일 정리**: 취소/실패 시 자동 삭제

---

## 테스트

### 단위 테스트 항목

- resolveFilePath (파일명 중복 처리)
- calculateSpeed (속도 계산)
- estimateTimeLeft (ETA 추정)
- formatFileSize (바이트 포맷)

### 통합 테스트 항목

- PDF 다운로드
- 이미지 다운로드
- 대용량 파일 (100MB+)
- 파일명 중복
- 일시 정지/재개
- 취소
- 네트워크 끊김 에러

---

## 향후 확장

- **다운로드 재개 (Resume)**: HTTP Range 요청으로 부분 파일에서 재개
- **다운로드 우선순위**: 여러 다운로드 중 순서 조정
- **다운로드 대역폭 제한**: 설정에서 최대 속도 제한
- **다운로드 히스토리 저장**: LS2 API로 완료된 다운로드 영구 보관

---

## 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초기 구현 완료 | cpp-dev |
