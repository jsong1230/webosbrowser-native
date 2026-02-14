#include "DownloadPanel.h"
#include <QDesktopServices>
#include <QMimeDatabase>
#include <QFileInfo>
#include <QFile>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>

namespace webosbrowser {

// DownloadItemWidget 구현

DownloadItemWidget::DownloadItemWidget(const DownloadItem& item, QWidget* parent)
    : QWidget(parent)
{
    setupUI(item);
}

void DownloadItemWidget::setupUI(const DownloadItem& item)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 8, 12, 8);
    mainLayout->setSpacing(6);

    // 상단: 파일명 + 상태
    QHBoxLayout* topLayout = new QHBoxLayout();
    m_fileNameLabel = new QLabel(item.fileName, this);
    m_fileNameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF;");
    m_fileNameLabel->setWordWrap(false);
    m_fileNameLabel->setMaximumWidth(600);

    m_stateLabel = new QLabel(this);
    m_stateLabel->setStyleSheet("font-size: 14px; color: #AAAAAA;");
    updateState(item.state);

    topLayout->addWidget(m_fileNameLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_stateLabel);

    // 중앙: 프로그레스바
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_progressBar->setMinimumHeight(24);
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

    // 하단: 진행률 텍스트 + 속도 + ETA
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    m_progressLabel = new QLabel(this);
    m_progressLabel->setStyleSheet("font-size: 14px; color: #CCCCCC;");

    m_speedLabel = new QLabel(this);
    m_speedLabel->setStyleSheet("font-size: 14px; color: #CCCCCC;");

    m_etaLabel = new QLabel(this);
    m_etaLabel->setStyleSheet("font-size: 14px; color: #CCCCCC;");

    bottomLayout->addWidget(m_progressLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_speedLabel);
    bottomLayout->addWidget(new QLabel(" | ", this));
    bottomLayout->addWidget(m_etaLabel);

    // 레이아웃 조립
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);

    // 초기 데이터 설정
    updateItem(item);
}

void DownloadItemWidget::updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int progress = static_cast<int>((bytesReceived * 100) / bytesTotal);
        m_progressBar->setValue(progress);
    }

    QString progressText = QString("%1 / %2")
                           .arg(formatFileSize(bytesReceived))
                           .arg(formatFileSize(bytesTotal));
    m_progressLabel->setText(progressText);
}

void DownloadItemWidget::updateState(DownloadState state)
{
    QString stateText;
    QString stateColor = "#AAAAAA";

    switch (state) {
        case DownloadState::Pending:
            stateText = "대기 중";
            break;
        case DownloadState::InProgress:
            stateText = "진행 중";
            stateColor = "#4CAF50";
            break;
        case DownloadState::Paused:
            stateText = "일시 정지";
            stateColor = "#FFA500";
            break;
        case DownloadState::Completed:
            stateText = "완료";
            stateColor = "#2196F3";
            break;
        case DownloadState::Cancelled:
            stateText = "취소됨";
            stateColor = "#888888";
            break;
        case DownloadState::Failed:
            stateText = "실패";
            stateColor = "#F44336";
            break;
    }

    m_stateLabel->setText(stateText);
    m_stateLabel->setStyleSheet(QString("font-size: 14px; color: %1; font-weight: bold;").arg(stateColor));
}

void DownloadItemWidget::updateItem(const DownloadItem& item)
{
    // 파일명
    m_fileNameLabel->setText(item.fileName);

    // 진행률
    updateProgress(item.bytesReceived, item.bytesTotal);

    // 상태
    updateState(item.state);

    // 속도
    if (item.state == DownloadState::InProgress && item.speed > 0) {
        m_speedLabel->setText(formatSpeed(item.speed));
    } else {
        m_speedLabel->setText("");
    }

    // ETA
    if (item.state == DownloadState::InProgress && item.estimatedTimeLeft >= 0) {
        m_etaLabel->setText(formatTimeLeft(item.estimatedTimeLeft) + " 남음");
    } else {
        m_etaLabel->setText("");
    }
}

QString DownloadItemWidget::formatFileSize(qint64 bytes) const
{
    if (bytes < 0) {
        return "알 수 없음";
    }

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

QString DownloadItemWidget::formatSpeed(qreal bytesPerSec) const
{
    if (bytesPerSec <= 0) {
        return "";
    }

    const qreal KB = 1024.0;
    const qreal MB = KB * 1024.0;

    if (bytesPerSec >= MB) {
        return QString("%1 MB/s").arg(bytesPerSec / MB, 0, 'f', 1);
    } else if (bytesPerSec >= KB) {
        return QString("%1 KB/s").arg(bytesPerSec / KB, 0, 'f', 1);
    } else {
        return QString("%1 B/s").arg(bytesPerSec, 0, 'f', 0);
    }
}

QString DownloadItemWidget::formatTimeLeft(qint64 seconds) const
{
    if (seconds < 0) {
        return "알 수 없음";
    }

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

// DownloadPanel 구현

DownloadPanel::DownloadPanel(DownloadManager* downloadManager, QWidget* parent)
    : QWidget(parent)
    , m_downloadManager(downloadManager)
{
    setupUI();
    setupConnections();

    // 초기 상태: 숨김
    hide();
}

DownloadPanel::~DownloadPanel()
{
    // Qt parent-child 메모리 관리로 자동 정리
}

void DownloadPanel::setupUI()
{
    // 메인 레이아웃
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16, 16, 16, 16);
    m_mainLayout->setSpacing(12);

    // 헤더 (제목 + 닫기 버튼)
    m_headerLayout = new QHBoxLayout();
    m_titleLabel = new QLabel("다운로드", this);
    m_titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #FFFFFF;");

    m_closeButton = new QPushButton("×", this);
    m_closeButton->setFixedSize(40, 40);
    m_closeButton->setStyleSheet(
        "QPushButton {"
        "    font-size: 24px;"
        "    color: #FFFFFF;"
        "    background-color: #444444;"
        "    border: none;"
        "    border-radius: 20px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #555555;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #666666;"
        "}"
    );

    m_headerLayout->addWidget(m_titleLabel);
    m_headerLayout->addStretch();
    m_headerLayout->addWidget(m_closeButton);

    // 다운로드 리스트
    m_downloadList = new QListWidget(this);
    m_downloadList->setUniformItemSizes(true);
    m_downloadList->setLayoutMode(QListView::Batched);
    m_downloadList->setStyleSheet(
        "QListWidget {"
        "    background-color: #1E1E1E;"
        "    border: 1px solid #555555;"
        "    border-radius: 4px;"
        "}"
        "QListWidget::item {"
        "    border-bottom: 1px solid #333333;"
        "    padding: 4px;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: #2A2A2A;"
        "    border: 2px solid #4CAF50;"
        "}"
    );

    // 제어 버튼 (선택된 항목용)
    QHBoxLayout* itemButtonLayout = new QHBoxLayout();
    m_pauseButton = new QPushButton("일시 정지", this);
    m_resumeButton = new QPushButton("재개", this);
    m_cancelButton = new QPushButton("취소", this);
    m_openButton = new QPushButton("열기", this);
    m_deleteButton = new QPushButton("삭제", this);

    QString buttonStyle =
        "QPushButton {"
        "    font-size: 14px;"
        "    color: #FFFFFF;"
        "    background-color: #444444;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 8px 16px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #555555;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #666666;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #2A2A2A;"
        "    color: #888888;"
        "}";

    m_pauseButton->setStyleSheet(buttonStyle);
    m_resumeButton->setStyleSheet(buttonStyle);
    m_cancelButton->setStyleSheet(buttonStyle);
    m_openButton->setStyleSheet(buttonStyle);
    m_deleteButton->setStyleSheet(buttonStyle);

    itemButtonLayout->addWidget(m_pauseButton);
    itemButtonLayout->addWidget(m_resumeButton);
    itemButtonLayout->addWidget(m_cancelButton);
    itemButtonLayout->addWidget(m_openButton);
    itemButtonLayout->addWidget(m_deleteButton);
    itemButtonLayout->addStretch();

    // 전체 제어 버튼
    QHBoxLayout* globalButtonLayout = new QHBoxLayout();
    m_pauseAllButton = new QPushButton("모두 일시정지", this);
    m_resumeAllButton = new QPushButton("모두 재개", this);
    m_clearCompletedButton = new QPushButton("완료된 항목 삭제", this);

    m_pauseAllButton->setStyleSheet(buttonStyle);
    m_resumeAllButton->setStyleSheet(buttonStyle);
    m_clearCompletedButton->setStyleSheet(buttonStyle);

    globalButtonLayout->addWidget(m_pauseAllButton);
    globalButtonLayout->addWidget(m_resumeAllButton);
    globalButtonLayout->addWidget(m_clearCompletedButton);
    globalButtonLayout->addStretch();

    // 레이아웃 조립
    m_mainLayout->addLayout(m_headerLayout);
    m_mainLayout->addWidget(m_downloadList, 1);  // stretch factor 1
    m_mainLayout->addLayout(itemButtonLayout);
    m_mainLayout->addLayout(globalButtonLayout);

    setLayout(m_mainLayout);

    // 패널 스타일
    setStyleSheet("QWidget { background-color: #252525; }");

    // 최소 크기 설정
    setMinimumSize(800, 400);

    // 초기 버튼 상태
    updateButtonStates();
}

void DownloadPanel::setupConnections()
{
    // 닫기 버튼
    connect(m_closeButton, &QPushButton::clicked, this, [this]() {
        hide();
        emit panelClosed();
    });

    // 제어 버튼 (선택된 항목용)
    connect(m_pauseButton, &QPushButton::clicked, this, &DownloadPanel::onPauseClicked);
    connect(m_resumeButton, &QPushButton::clicked, this, &DownloadPanel::onResumeClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &DownloadPanel::onCancelClicked);
    connect(m_openButton, &QPushButton::clicked, this, &DownloadPanel::onOpenClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &DownloadPanel::onDeleteClicked);

    // 전체 제어 버튼
    connect(m_pauseAllButton, &QPushButton::clicked, this, &DownloadPanel::onPauseAllClicked);
    connect(m_resumeAllButton, &QPushButton::clicked, this, &DownloadPanel::onResumeAllClicked);
    connect(m_clearCompletedButton, &QPushButton::clicked, this, &DownloadPanel::onClearCompletedClicked);

    // 리스트 선택 변경
    connect(m_downloadList, &QListWidget::currentItemChanged,
            this, &DownloadPanel::onSelectionChanged);

    // DownloadManager 시그널 연결
    if (m_downloadManager) {
        connect(m_downloadManager, &DownloadManager::downloadAdded,
                this, &DownloadPanel::onDownloadAdded);
        connect(m_downloadManager, &DownloadManager::downloadProgressChanged,
                this, &DownloadPanel::onDownloadProgressChanged);
        connect(m_downloadManager, &DownloadManager::downloadStateChanged,
                this, &DownloadPanel::onDownloadStateChanged);
        connect(m_downloadManager, &DownloadManager::downloadCompleted,
                this, &DownloadPanel::onDownloadCompleted);
        connect(m_downloadManager, &DownloadManager::downloadFailed,
                this, &DownloadPanel::onDownloadFailed);
    }
}

void DownloadPanel::show()
{
    QWidget::show();
    refreshDownloads();
    qDebug() << "DownloadPanel: 패널 표시";
}

void DownloadPanel::hide()
{
    QWidget::hide();
    qDebug() << "DownloadPanel: 패널 숨김";
}

void DownloadPanel::refreshDownloads()
{
    // 리스트 클리어
    m_downloadList->clear();

    // DownloadManager에서 전체 다운로드 조회
    QVector<DownloadItem> downloads = m_downloadManager->getDownloads();

    // 리스트에 추가 (역순: 최신 다운로드가 위로)
    for (int i = downloads.size() - 1; i >= 0; --i) {
        const DownloadItem& item = downloads[i];

        // DownloadItemWidget 생성
        DownloadItemWidget* itemWidget = new DownloadItemWidget(item, this);

        // QListWidgetItem 생성
        QListWidgetItem* listItem = new QListWidgetItem(m_downloadList);
        listItem->setData(Qt::UserRole, item.id);  // ID 저장
        listItem->setSizeHint(itemWidget->sizeHint());

        // 커스텀 위젯 설정
        m_downloadList->addItem(listItem);
        m_downloadList->setItemWidget(listItem, itemWidget);
    }

    qDebug() << "DownloadPanel: 다운로드 목록 갱신 - 총" << downloads.size() << "개";
}

void DownloadPanel::keyPressEvent(QKeyEvent* event)
{
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
                if (!id.isEmpty()) {
                    DownloadItem item = m_downloadManager->getDownloadById(id);
                    if (item.state == DownloadState::Completed) {
                        openDownloadedFile(item.filePath);
                    }
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

// Private Slots

void DownloadPanel::onPauseClicked()
{
    QString id = getSelectedDownloadId();
    if (!id.isEmpty()) {
        m_downloadManager->pauseDownload(id);
        qDebug() << "DownloadPanel: 일시정지 -" << id;
    }
}

void DownloadPanel::onResumeClicked()
{
    QString id = getSelectedDownloadId();
    if (!id.isEmpty()) {
        m_downloadManager->resumeDownload(id);
        qDebug() << "DownloadPanel: 재개 -" << id;
    }
}

void DownloadPanel::onCancelClicked()
{
    QString id = getSelectedDownloadId();
    if (!id.isEmpty()) {
        m_downloadManager->cancelDownload(id);
        qDebug() << "DownloadPanel: 취소 -" << id;
    }
}

void DownloadPanel::onOpenClicked()
{
    QString id = getSelectedDownloadId();
    if (!id.isEmpty()) {
        DownloadItem item = m_downloadManager->getDownloadById(id);
        if (item.state == DownloadState::Completed) {
            openDownloadedFile(item.filePath);
        }
    }
}

void DownloadPanel::onDeleteClicked()
{
    QString id = getSelectedDownloadId();
    if (!id.isEmpty()) {
        m_downloadManager->removeDownload(id);
        refreshDownloads();
        qDebug() << "DownloadPanel: 삭제 -" << id;
    }
}

void DownloadPanel::onPauseAllClicked()
{
    QVector<DownloadItem> downloads = m_downloadManager->getDownloads();
    int count = 0;
    for (const auto& item : downloads) {
        if (item.state == DownloadState::InProgress) {
            m_downloadManager->pauseDownload(item.id);
            count++;
        }
    }
    qDebug() << "DownloadPanel: 모두 일시정지 -" << count << "개";
}

void DownloadPanel::onResumeAllClicked()
{
    QVector<DownloadItem> downloads = m_downloadManager->getDownloads();
    int count = 0;
    for (const auto& item : downloads) {
        if (item.state == DownloadState::Paused) {
            m_downloadManager->resumeDownload(item.id);
            count++;
        }
    }
    qDebug() << "DownloadPanel: 모두 재개 -" << count << "개";
}

void DownloadPanel::onClearCompletedClicked()
{
    QVector<DownloadItem> downloads = m_downloadManager->getDownloads();
    int count = 0;
    for (const auto& item : downloads) {
        if (item.state == DownloadState::Completed ||
            item.state == DownloadState::Cancelled ||
            item.state == DownloadState::Failed) {
            m_downloadManager->removeDownload(item.id);
            count++;
        }
    }
    refreshDownloads();
    qDebug() << "DownloadPanel: 완료된 항목 삭제 -" << count << "개";
}

void DownloadPanel::onSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
    Q_UNUSED(previous);

    if (current) {
        QString id = current->data(Qt::UserRole).toString();
        qDebug() << "DownloadPanel: 선택 변경 -" << id;
    }

    updateButtonStates();
}

void DownloadPanel::onDownloadAdded(const DownloadItem& item)
{
    qDebug() << "DownloadPanel: 다운로드 추가 -" << item.fileName;

    // 리스트 최상단에 추가
    DownloadItemWidget* itemWidget = new DownloadItemWidget(item, this);

    QListWidgetItem* listItem = new QListWidgetItem();
    listItem->setData(Qt::UserRole, item.id);
    listItem->setSizeHint(itemWidget->sizeHint());

    m_downloadList->insertItem(0, listItem);
    m_downloadList->setItemWidget(listItem, itemWidget);

    // 패널이 숨겨져 있으면 자동으로 표시 (선택사항)
    // show();
}

void DownloadPanel::onDownloadProgressChanged(const QString& id, qint64 bytesReceived, qint64 bytesTotal)
{
    updateDownloadItemWidget(id, m_downloadManager->getDownloadById(id));
}

void DownloadPanel::onDownloadStateChanged(const QString& id, DownloadState state)
{
    Q_UNUSED(state);
    updateDownloadItemWidget(id, m_downloadManager->getDownloadById(id));
    updateButtonStates();
}

void DownloadPanel::onDownloadCompleted(const DownloadItem& item)
{
    qDebug() << "DownloadPanel: 다운로드 완료 -" << item.fileName;
    updateDownloadItemWidget(item.id, item);
    showToast("다운로드 완료: " + item.fileName);
}

void DownloadPanel::onDownloadFailed(const QString& id, const QString& errorMessage)
{
    qDebug() << "DownloadPanel: 다운로드 실패 -" << id << errorMessage;
    updateDownloadItemWidget(id, m_downloadManager->getDownloadById(id));
    showToast("다운로드 실패: " + errorMessage);
}

// Private Methods

QWidget* DownloadPanel::createDownloadItemWidget(const DownloadItem& item)
{
    return new DownloadItemWidget(item, this);
}

void DownloadPanel::updateDownloadItemWidget(const QString& id, const DownloadItem& item)
{
    // ID로 항목 찾기
    for (int i = 0; i < m_downloadList->count(); ++i) {
        QListWidgetItem* listItem = m_downloadList->item(i);
        if (listItem->data(Qt::UserRole).toString() == id) {
            DownloadItemWidget* widget = qobject_cast<DownloadItemWidget*>(
                m_downloadList->itemWidget(listItem)
            );
            if (widget) {
                widget->updateItem(item);
            }
            break;
        }
    }
}

QString DownloadPanel::getSelectedDownloadId() const
{
    QListWidgetItem* currentItem = m_downloadList->currentItem();
    if (currentItem) {
        return currentItem->data(Qt::UserRole).toString();
    }
    return QString();
}

void DownloadPanel::updateButtonStates()
{
    QString id = getSelectedDownloadId();

    if (id.isEmpty()) {
        // 선택 항목 없음 - 모든 버튼 비활성화
        m_pauseButton->setEnabled(false);
        m_resumeButton->setEnabled(false);
        m_cancelButton->setEnabled(false);
        m_openButton->setEnabled(false);
        m_deleteButton->setEnabled(false);
        return;
    }

    DownloadItem item = m_downloadManager->getDownloadById(id);

    // 상태에 따라 버튼 활성화/비활성화
    switch (item.state) {
        case DownloadState::Pending:
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

        case DownloadState::Cancelled:
        case DownloadState::Failed:
            m_pauseButton->setEnabled(false);
            m_resumeButton->setEnabled(false);
            m_cancelButton->setEnabled(false);
            m_openButton->setEnabled(false);
            m_deleteButton->setEnabled(true);
            break;
    }
}

void DownloadPanel::showToast(const QString& message)
{
    // Qt 자체 토스트 구현 (QLabel 기반)
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
    toast->setWordWrap(true);
    toast->setMaximumWidth(600);
    toast->adjustSize();

    // 중앙 하단 배치
    int x = (width() - toast->width()) / 2;
    int y = height() - toast->height() - 60;
    toast->move(x, y);
    toast->show();

    // 3초 후 자동 삭제
    QTimer::singleShot(3000, toast, &QLabel::deleteLater);
}

void DownloadPanel::openDownloadedFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        showToast("파일을 찾을 수 없습니다: " + fileInfo.fileName());
        return;
    }

    // [보안] 다운로드 디렉토리 내부 파일인지 검증
    QString downloadDir = m_downloadManager->downloadPath();
    QString canonicalFilePath = fileInfo.canonicalFilePath();
    QString canonicalDownloadDir = QFileInfo(downloadDir).canonicalFilePath();

    if (!canonicalFilePath.startsWith(canonicalDownloadDir)) {
        qCritical() << "DownloadPanel: 경로 조작 시도 감지 -" << filePath;
        showToast("보안 오류: 잘못된 파일 경로");
        return;
    }

    // [보안] 실행 파일 경고
    QString suffix = fileInfo.suffix().toLower();
    QStringList executableExtensions = {"exe", "sh", "bat", "cmd", "app", "msi", "deb", "rpm"};
    if (executableExtensions.contains(suffix)) {
        QMessageBox::StandardButton reply = QMessageBox::warning(
            this,
            "실행 파일 경고",
            QString("실행 파일 (%1)을 열려고 합니다. 계속하시겠습니까?").arg(fileInfo.fileName()),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    // 파일 열기
    QUrl fileUrl = QUrl::fromLocalFile(canonicalFilePath);
    if (!QDesktopServices::openUrl(fileUrl)) {
        qWarning() << "DownloadPanel: 파일 열기 실패 -" << filePath;
        showToast("이 파일 형식을 열 수 없습니다");
    } else {
        qDebug() << "DownloadPanel: 파일 열기 -" << filePath;
    }
}

QString DownloadPanel::formatFileSize(qint64 bytes) const
{
    if (bytes < 0) {
        return "알 수 없음";
    }

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

QString DownloadPanel::formatSpeed(qreal bytesPerSec) const
{
    if (bytesPerSec <= 0) {
        return "";
    }

    const qreal KB = 1024.0;
    const qreal MB = KB * 1024.0;

    if (bytesPerSec >= MB) {
        return QString("%1 MB/s").arg(bytesPerSec / MB, 0, 'f', 1);
    } else if (bytesPerSec >= KB) {
        return QString("%1 KB/s").arg(bytesPerSec / KB, 0, 'f', 1);
    } else {
        return QString("%1 B/s").arg(bytesPerSec, 0, 'f', 0);
    }
}

QString DownloadPanel::formatTimeLeft(qint64 seconds) const
{
    if (seconds < 0) {
        return "알 수 없음";
    }

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

} // namespace webosbrowser
