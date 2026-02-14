# DownloadPanel 컴포넌트 문서

## 개요

**위치**: `src/ui/DownloadPanel.h`, `src/ui/DownloadPanel.cpp`

**역할**: 다운로드 목록 표시 및 제어 버튼 제공

**설계 문서**: `docs/specs/download-manager/design.md`

---

## 클래스 구조

### DownloadPanel

```cpp
class DownloadPanel : public QWidget {
    Q_OBJECT

public:
    explicit DownloadPanel(DownloadManager* downloadManager, QWidget* parent = nullptr);
    ~DownloadPanel() override;

    void show();
    void hide();
    void refreshDownloads();

signals:
    void panelClosed();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    DownloadManager* m_downloadManager;
    QListWidget* m_downloadList;
    QPushButton* m_pauseButton;
    QPushButton* m_resumeButton;
    QPushButton* m_cancelButton;
    QPushButton* m_openButton;
    QPushButton* m_deleteButton;
    // ... 기타 UI 컴포넌트
};
```

### DownloadItemWidget

```cpp
class DownloadItemWidget : public QWidget {
    Q_OBJECT

public:
    explicit DownloadItemWidget(const DownloadItem& item, QWidget* parent = nullptr);

    void updateProgress(qint64 bytesReceived, qint64 bytesTotal);
    void updateState(DownloadState state);
    void updateItem(const DownloadItem& item);

private:
    QLabel* m_fileNameLabel;
    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;
    QLabel* m_speedLabel;
    QLabel* m_etaLabel;
    QLabel* m_stateLabel;
};
```

---

## UI 레이아웃

### 전체 구조

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
│  └────────────────────────────────────────────────────┘  │
│  ┌────────────────────────────────────────────────────┐  │
│  │  image.png                                   완료   │  │
│  │  ████████████████████████████████████████  100%    │  │
│  │  5.2 MB / 5.2 MB                                   │  │
│  └────────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────┤
│  [일시정지] [재개] [취소] [열기] [삭제]                  │
│  [모두 일시정지] [모두 재개] [완료된 항목 삭제]          │
└──────────────────────────────────────────────────────────┘
```

### DownloadItemWidget 상세

```
┌────────────────────────────────────────────────────┐
│  document.pdf                            진행중    │  ← 파일명 + 상태
│  ████████████████░░░░░░░░░░░░░░░░  60%            │  ← 프로그레스바
│  12.5 MB / 20 MB  |  2.3 MB/s  |  3분 20초 남음    │  ← 진행률 + 속도 + ETA
└────────────────────────────────────────────────────┘
```

---

## 주요 기능

### 1. 다운로드 목록 표시

```cpp
void DownloadPanel::refreshDownloads()
```

- DownloadManager에서 전체 다운로드 조회
- QListWidget에 DownloadItemWidget 추가
- 최신 다운로드가 위로 (역순 정렬)

### 2. 실시간 업데이트

```cpp
void DownloadPanel::onDownloadProgressChanged(const QString& id, qint64 bytesReceived, qint64 bytesTotal)
```

- ID로 항목 찾기
- DownloadItemWidget::updateProgress() 호출
- 프로그레스바, 속도, ETA 업데이트

### 3. 제어 버튼 동작

#### 일시 정지

```cpp
void DownloadPanel::onPauseClicked()
{
    QString id = getSelectedDownloadId();
    m_downloadManager->pauseDownload(id);
}
```

#### 재개

```cpp
void DownloadPanel::onResumeClicked()
{
    m_downloadManager->resumeDownload(id);
}
```

#### 취소

```cpp
void DownloadPanel::onCancelClicked()
{
    m_downloadManager->cancelDownload(id);
}
```

#### 열기 (완료된 파일)

```cpp
void DownloadPanel::onOpenClicked()
{
    DownloadItem item = m_downloadManager->getDownloadById(id);
    if (item.state == DownloadState::Completed) {
        openDownloadedFile(item.filePath);
    }
}
```

**파일 열기 로직**:
```cpp
QUrl fileUrl = QUrl::fromLocalFile(filePath);
if (!QDesktopServices::openUrl(fileUrl)) {
    showToast("이 파일 형식을 열 수 없습니다");
}
```

#### 삭제 (목록에서 제거)

```cpp
void DownloadPanel::onDeleteClicked()
{
    m_downloadManager->removeDownload(id);
    refreshDownloads();
}
```

### 4. 전체 제어 버튼

#### 모두 일시정지

```cpp
void DownloadPanel::onPauseAllClicked()
{
    QVector<DownloadItem> downloads = m_downloadManager->getDownloads();
    for (const auto& item : downloads) {
        if (item.state == DownloadState::InProgress) {
            m_downloadManager->pauseDownload(item.id);
        }
    }
}
```

#### 완료된 항목 삭제

```cpp
void DownloadPanel::onClearCompletedClicked()
{
    for (const auto& item : downloads) {
        if (item.state == DownloadState::Completed ||
            item.state == DownloadState::Cancelled ||
            item.state == DownloadState::Failed) {
            m_downloadManager->removeDownload(item.id);
        }
    }
    refreshDownloads();
}
```

---

## 리모컨 입력 처리

### 키 매핑

| 리모컨 버튼 | Qt 키 | 동작 |
|------------|-------|------|
| 위 십자키 | `Qt::Key_Up` | 이전 다운로드 항목으로 이동 |
| 아래 십자키 | `Qt::Key_Down` | 다음 다운로드 항목으로 이동 |
| Enter | `Qt::Key_Return` | 완료된 항목 열기 |
| Back | `Qt::Key_Escape` | 패널 닫기 |

### keyPressEvent 구현

```cpp
void DownloadPanel::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
        case Qt::Key_Up:
            int currentRow = m_downloadList->currentRow();
            if (currentRow > 0) {
                m_downloadList->setCurrentRow(currentRow - 1);
            }
            break;

        case Qt::Key_Down:
            if (currentRow < m_downloadList->count() - 1) {
                m_downloadList->setCurrentRow(currentRow + 1);
            }
            break;

        case Qt::Key_Return:
        case Qt::Key_Enter:
            // 완료된 항목 열기
            QString id = getSelectedDownloadId();
            DownloadItem item = m_downloadManager->getDownloadById(id);
            if (item.state == DownloadState::Completed) {
                openDownloadedFile(item.filePath);
            }
            break;

        case Qt::Key_Escape:
            // 패널 닫기
            hide();
            emit panelClosed();
            break;
    }
}
```

---

## 버튼 상태 관리

### updateButtonStates()

선택된 항목의 상태에 따라 버튼 활성화/비활성화

```cpp
void DownloadPanel::updateButtonStates()
{
    DownloadItem item = m_downloadManager->getDownloadById(selectedId);

    switch (item.state) {
        case DownloadState::InProgress:
            m_pauseButton->setEnabled(true);
            m_resumeButton->setEnabled(false);
            m_cancelButton->setEnabled(true);
            m_openButton->setEnabled(false);
            m_deleteButton->setEnabled(false);
            break;

        case DownloadState::Paused:
            m_pauseButton->setEnabled(false);
            m_resumeButton->setEnabled(true);
            m_cancelButton->setEnabled(true);
            m_openButton->setEnabled(false);
            m_deleteButton->setEnabled(false);
            break;

        case DownloadState::Completed:
            m_pauseButton->setEnabled(false);
            m_resumeButton->setEnabled(false);
            m_cancelButton->setEnabled(false);
            m_openButton->setEnabled(true);
            m_deleteButton->setEnabled(true);
            break;

        // ... 기타 상태
    }
}
```

---

## 토스트 알림

### 구현 (Qt 자체)

```cpp
void DownloadPanel::showToast(const QString& message)
{
    QLabel* toast = new QLabel(message, this);
    toast->setStyleSheet(
        "QLabel {"
        "  background-color: rgba(0, 0, 0, 0.85);"
        "  color: white;"
        "  padding: 16px 24px;"
        "  border-radius: 8px;"
        "  font-size: 16px;"
        "}"
    );
    toast->setAlignment(Qt::AlignCenter);

    // 중앙 하단 배치
    int x = (width() - toast->width()) / 2;
    int y = height() - toast->height() - 60;
    toast->move(x, y);
    toast->show();

    // 3초 후 자동 삭제
    QTimer::singleShot(3000, toast, &QLabel::deleteLater);
}
```

---

## 포맷 헬퍼 함수

### 파일 크기 포맷

```cpp
QString DownloadPanel::formatFileSize(qint64 bytes) const
{
    const qreal KB = 1024.0;
    const qreal MB = KB * 1024.0;
    const qreal GB = MB * 1024.0;

    if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / GB, 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / MB, 0, 'f', 1);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / KB, 0, 'f', 1);
    } else {
        return QString("%1 B").arg(bytes);
    }
}
```

### 다운로드 속도 포맷

```cpp
QString DownloadPanel::formatSpeed(qreal bytesPerSec) const
{
    if (bytesPerSec >= MB) {
        return QString("%1 MB/s").arg(bytesPerSec / MB, 0, 'f', 1);
    } else if (bytesPerSec >= KB) {
        return QString("%1 KB/s").arg(bytesPerSec / KB, 0, 'f', 1);
    } else {
        return QString("%1 B/s").arg(bytesPerSec, 0, 'f', 0);
    }
}
```

### 남은 시간 포맷

```cpp
QString DownloadPanel::formatTimeLeft(qint64 seconds) const
{
    if (seconds < 60) {
        return QString("%1초").arg(seconds);
    } else if (seconds < 3600) {
        int minutes = seconds / 60;
        int secs = seconds % 60;
        return QString("%1분 %2초").arg(minutes).arg(secs);
    } else {
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        return QString("%1시간 %2분").arg(hours).arg(minutes);
    }
}
```

---

## 스타일링

### 다크 테마

```cpp
setStyleSheet("QWidget { background-color: #252525; }");
```

### 프로그레스바

```cpp
m_progressBar->setStyleSheet(
    "QProgressBar {"
    "    border: 1px solid #555555;"
    "    border-radius: 4px;"
    "    background-color: #2A2A2A;"
    "    text-align: center;"
    "    color: #FFFFFF;"
    "}"
    "QProgressBar::chunk {"
    "    background-color: #4CAF50;"
    "    border-radius: 4px;"
    "}"
);
```

### 버튼

```cpp
QPushButton {
    font-size: 14px;
    color: #FFFFFF;
    background-color: #444444;
    border: none;
    border-radius: 4px;
    padding: 8px 16px;
    min-width: 80px;
}
QPushButton:hover {
    background-color: #555555;
}
QPushButton:pressed {
    background-color: #666666;
}
QPushButton:disabled {
    background-color: #2A2A2A;
    color: #888888;
}
```

### 상태 레이블 색상

| 상태 | 색상 |
|------|------|
| 진행 중 | #4CAF50 (초록) |
| 일시 정지 | #FFA500 (주황) |
| 완료 | #2196F3 (파랑) |
| 실패 | #F44336 (빨강) |
| 취소됨 | #888888 (회색) |

---

## 성능 최적화

### QListWidget 최적화

```cpp
m_downloadList->setUniformItemSizes(true);  // 균일한 항목 크기 (렌더링 최적화)
m_downloadList->setLayoutMode(QListView::Batched);  // 배치 레이아웃 (스크롤 성능)
```

### 위젯 재사용

```cpp
void DownloadPanel::updateDownloadItemWidget(const QString& id, const DownloadItem& item)
{
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

## BrowserWindow 통합

### 패널 표시/숨김

```cpp
// BrowserWindow.h
void showDownloadPanel();
void hideDownloadPanel();
void toggleDownloadPanel();

// BrowserWindow.cpp
void BrowserWindow::showDownloadPanel()
{
    if (downloadPanel_) {
        downloadPanel_->show();
        downloadPanel_->raise();  // 최상단으로
    }
}
```

### 다운로드 완료 알림

```cpp
// BrowserWindow::setupConnections
connect(downloadManager_, &DownloadManager::downloadCompleted,
        this, &BrowserWindow::onDownloadCompleted);

void BrowserWindow::onDownloadCompleted(const DownloadItem& item)
{
    statusLabel_->setText("다운로드 완료: " + item.fileName);
}
```

---

## 향후 확장

- **정렬 옵션**: 파일명, 크기, 날짜로 정렬
- **필터링**: 진행 중/완료/실패 항목 필터
- **컨텍스트 메뉴**: 우클릭 메뉴 (리모컨 대안)
- **드래그 앤 드롭**: 파일 관리자로 드래그

---

## 변경 이력

| 날짜 | 변경 내용 | 작성자 |
|------|-----------|--------|
| 2026-02-14 | 초기 구현 완료 | cpp-dev |
