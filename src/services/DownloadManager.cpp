#include "DownloadManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QUuid>
#include <QDebug>
#include <QDateTime>

namespace webosbrowser {

// DownloadItem 기본 생성자
DownloadItem::DownloadItem()
    : bytesTotal(0)
    , bytesReceived(0)
    , state(DownloadState::Pending)
    , speed(0.0)
    , estimatedTimeLeft(-1)
{
}

// DownloadManager 생성자
DownloadManager::DownloadManager(QObject* parent)
    : QObject(parent)
{
    // 기본 다운로드 경로 설정
    setDownloadPath(QString());
}

// DownloadManager 소멸자
DownloadManager::~DownloadManager()
{
    // Qt parent-child 메모리 관리로 자동 정리
}

void DownloadManager::startDownload(QWebEngineDownloadItem* downloadItem)
{
    if (!downloadItem) {
        qWarning() << "DownloadManager: null downloadItem";
        return;
    }

    // 진행 중 다운로드 수 확인
    int activeCount = 0;
    for (const auto& item : m_downloads) {
        if (item.state == DownloadState::InProgress) {
            activeCount++;
        }
    }

    if (activeCount >= MAX_CONCURRENT_DOWNLOADS) {
        qWarning() << "DownloadManager: 최대 동시 다운로드 수 도달 (3개)";
        downloadItem->cancel();
        return;
    }

    // 고유 ID 생성
    QString id = generateDownloadId();

    // 파일명 및 경로 설정
    QString fileName = downloadItem->downloadFileName();
    if (fileName.isEmpty()) {
        fileName = "download";
    }

    QString filePath = m_downloadPath + "/" + fileName;
    filePath = resolveFilePath(filePath);

    // 다운로드 경로 설정 (accept 전에 설정 필수)
    QFileInfo fileInfo(filePath);
    downloadItem->setDownloadDirectory(fileInfo.absolutePath());
    downloadItem->setDownloadFileName(fileInfo.fileName());

    // DownloadItem 구조체 생성
    DownloadItem item;
    item.id = id;
    item.fileName = fileInfo.fileName();
    item.filePath = filePath;
    item.url = downloadItem->url();
    item.mimeType = downloadItem->mimeType();
    item.bytesTotal = downloadItem->totalBytes();
    item.bytesReceived = 0;
    item.state = DownloadState::Pending;
    item.startTime = QDateTime::currentDateTime();
    item.speed = 0.0;
    item.estimatedTimeLeft = -1;

    // 다운로드 목록에 추가
    m_downloads.append(item);
    m_activeDownloads.insert(id, downloadItem);

    // Qt 시그널 연결
    connect(downloadItem, &QWebEngineDownloadItem::downloadProgress,
            this, &DownloadManager::onDownloadProgress);
    connect(downloadItem, &QWebEngineDownloadItem::finished,
            this, &DownloadManager::onDownloadFinished);
    connect(downloadItem, &QWebEngineDownloadItem::stateChanged,
            this, &DownloadManager::onDownloadStateChanged);

    // 다운로드 시작
    downloadItem->accept();

    // 상태 업데이트
    updateDownloadState(id, DownloadState::InProgress);

    qDebug() << "DownloadManager: 다운로드 시작 -" << item.fileName << "URL:" << item.url.toString();

    emit downloadAdded(item);
}

void DownloadManager::pauseDownload(const QString& id)
{
    if (!m_activeDownloads.contains(id)) {
        qWarning() << "DownloadManager: pauseDownload - ID 없음:" << id;
        return;
    }

    QWebEngineDownloadItem* downloadItem = m_activeDownloads[id];

    // Qt 5.10+ pause 지원 확인
    if (downloadItem->state() == QWebEngineDownloadItem::DownloadInProgress) {
        downloadItem->pause();
        qDebug() << "DownloadManager: 다운로드 일시정지 -" << id;
    } else {
        qWarning() << "DownloadManager: 일시정지 불가 - 상태:" << downloadItem->state();
    }
}

void DownloadManager::resumeDownload(const QString& id)
{
    if (!m_activeDownloads.contains(id)) {
        qWarning() << "DownloadManager: resumeDownload - ID 없음:" << id;
        return;
    }

    QWebEngineDownloadItem* downloadItem = m_activeDownloads[id];

    // Qt 5.10+ resume 지원 확인
    if (downloadItem->state() == QWebEngineDownloadItem::DownloadInProgress ||
        downloadItem->isPaused()) {
        downloadItem->resume();
        qDebug() << "DownloadManager: 다운로드 재개 -" << id;
    } else {
        qWarning() << "DownloadManager: 재개 불가 - 상태:" << downloadItem->state();
    }
}

void DownloadManager::cancelDownload(const QString& id)
{
    if (!m_activeDownloads.contains(id)) {
        qWarning() << "DownloadManager: cancelDownload - ID 없음:" << id;
        return;
    }

    QWebEngineDownloadItem* downloadItem = m_activeDownloads[id];

    // 다운로드 취소
    downloadItem->cancel();

    qDebug() << "DownloadManager: 다운로드 취소 -" << id;

    // 상태 업데이트
    updateDownloadState(id, DownloadState::Cancelled);

    // 부분 다운로드 파일 삭제
    DownloadItem item = getDownloadById(id);
    if (!item.filePath.isEmpty() && QFile::exists(item.filePath)) {
        QFile::remove(item.filePath);
        qDebug() << "DownloadManager: 부분 파일 삭제 -" << item.filePath;
    }

    // 활성 다운로드 목록에서 제거
    m_activeDownloads.remove(id);
}

void DownloadManager::removeDownload(const QString& id)
{
    // ID로 항목 찾기
    int index = -1;
    for (int i = 0; i < m_downloads.size(); ++i) {
        if (m_downloads[i].id == id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        qWarning() << "DownloadManager: removeDownload - ID 없음:" << id;
        return;
    }

    DownloadItem item = m_downloads[index];

    // 진행 중 다운로드는 제거 불가
    if (item.state == DownloadState::InProgress || item.state == DownloadState::Paused) {
        qWarning() << "DownloadManager: 진행 중 다운로드는 제거 불가:" << id;
        return;
    }

    // 목록에서 제거
    m_downloads.removeAt(index);
    qDebug() << "DownloadManager: 다운로드 제거 -" << id;
}

QVector<DownloadItem> DownloadManager::getDownloads() const
{
    return m_downloads;
}

DownloadItem DownloadManager::getDownloadById(const QString& id) const
{
    for (const auto& item : m_downloads) {
        if (item.id == id) {
            return item;
        }
    }
    return DownloadItem();  // 빈 객체 반환
}

void DownloadManager::setDownloadPath(const QString& path)
{
    QString targetPath = path;

    // 빈 경로면 기본 경로 사용
    if (targetPath.isEmpty()) {
        targetPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }

    // 디렉토리 존재 확인 및 생성
    QDir dir(targetPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "DownloadManager: 디렉토리 생성 실패:" << targetPath;

            // 폴백 경로 사용 (AppDataLocation)
            targetPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/downloads";
            QDir fallbackDir(targetPath);
            if (!fallbackDir.exists()) {
                fallbackDir.mkpath(".");
            }
            qDebug() << "DownloadManager: 폴백 경로 사용:" << targetPath;
        }
    }

    // 쓰기 권한 확인
    QFileInfo dirInfo(targetPath);
    if (!dirInfo.isWritable()) {
        qWarning() << "DownloadManager: 쓰기 권한 없음:" << targetPath;
        // 폴백 경로로 재시도
        targetPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/downloads";
        QDir fallbackDir(targetPath);
        fallbackDir.mkpath(".");
    }

    m_downloadPath = targetPath;
    qDebug() << "DownloadManager: 다운로드 경로 설정:" << m_downloadPath;
}

QString DownloadManager::downloadPath() const
{
    return m_downloadPath;
}

// Private Slots

void DownloadManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QWebEngineDownloadItem* downloadItem = qobject_cast<QWebEngineDownloadItem*>(sender());
    if (!downloadItem) {
        return;
    }

    QString id = findDownloadId(downloadItem);
    if (id.isEmpty()) {
        return;
    }

    // 쓰로틀링: 500ms 간격으로 UI 업데이트
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_lastUpdateTime.contains(id)) {
        qint64 elapsed = now - m_lastUpdateTime[id];
        if (elapsed < 500) {  // 500ms 이내면 스킵
            return;
        }
    }

    // 속도 계산용 이전 데이터 저장 (업데이트 전)
    qint64 lastBytes = m_lastBytesReceived.value(id, 0);
    qint64 lastTime = m_lastUpdateTime.value(id, now);

    // 속도 계산
    qreal speed = 0.0;
    if (lastTime != now) {
        qreal timeDiff = (now - lastTime) / 1000.0;  // 초 단위
        if (timeDiff > 0) {
            speed = (bytesReceived - lastBytes) / timeDiff;  // bytes/sec
        }
    }

    // 현재 데이터 업데이트
    m_lastBytesReceived[id] = bytesReceived;
    m_lastUpdateTime[id] = now;

    // DownloadItem 업데이트
    for (auto& item : m_downloads) {
        if (item.id == id) {
            item.bytesReceived = bytesReceived;
            item.bytesTotal = bytesTotal;
            item.speed = speed;

            // ETA 추정
            item.estimatedTimeLeft = estimateTimeLeft(bytesReceived, bytesTotal, speed);

            // 진행률 시그널 emit
            emit downloadProgressChanged(id, bytesReceived, bytesTotal);
            break;
        }
    }
}

void DownloadManager::onDownloadFinished()
{
    QWebEngineDownloadItem* downloadItem = qobject_cast<QWebEngineDownloadItem*>(sender());
    if (!downloadItem) {
        return;
    }

    QString id = findDownloadId(downloadItem);
    if (id.isEmpty()) {
        return;
    }

    // 다운로드 상태 확인
    if (downloadItem->state() == QWebEngineDownloadItem::DownloadCompleted) {
        // 성공 처리
        DownloadItem item = getDownloadById(id);
        item.finishTime = QDateTime::currentDateTime();

        // 파일 존재 확인
        if (!QFile::exists(item.filePath)) {
            qWarning() << "DownloadManager: 파일 없음 -" << item.filePath;
            updateDownloadState(id, DownloadState::Failed, "파일을 찾을 수 없습니다");
            emit downloadFailed(id, "파일을 찾을 수 없습니다");
            m_activeDownloads.remove(id);
            return;
        }

        // 파일 크기 검증
        QFileInfo fileInfo(item.filePath);
        qint64 fileSize = fileInfo.size();
        if (fileSize != item.bytesTotal && item.bytesTotal > 0) {
            qWarning() << "DownloadManager: 파일 크기 불일치 - 예상:" << item.bytesTotal << "실제:" << fileSize;
        }

        // 상태 업데이트
        for (auto& downloadItem : m_downloads) {
            if (downloadItem.id == id) {
                downloadItem.state = DownloadState::Completed;
                downloadItem.finishTime = item.finishTime;
                downloadItem.bytesReceived = downloadItem.bytesTotal;
                break;
            }
        }

        qDebug() << "DownloadManager: 다운로드 완료 -" << item.fileName;

        emit downloadStateChanged(id, DownloadState::Completed);
        emit downloadCompleted(getDownloadById(id));

        // 활성 다운로드 목록에서 제거
        m_activeDownloads.remove(id);

    } else {
        // 실패 처리
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

        qWarning() << "DownloadManager: 다운로드 실패 -" << id << errorMessage;

        updateDownloadState(id, DownloadState::Failed, errorMessage);
        emit downloadFailed(id, errorMessage);

        // 부분 다운로드 파일 삭제
        DownloadItem item = getDownloadById(id);
        if (!item.filePath.isEmpty() && QFile::exists(item.filePath)) {
            QFile::remove(item.filePath);
            qDebug() << "DownloadManager: 부분 파일 삭제 -" << item.filePath;
        }

        // 활성 다운로드 목록에서 제거
        m_activeDownloads.remove(id);
    }
}

void DownloadManager::onDownloadStateChanged(QWebEngineDownloadItem::DownloadState qtState)
{
    QWebEngineDownloadItem* downloadItem = qobject_cast<QWebEngineDownloadItem*>(sender());
    if (!downloadItem) {
        return;
    }

    QString id = findDownloadId(downloadItem);
    if (id.isEmpty()) {
        return;
    }

    DownloadState state = convertQtState(qtState);

    // 상태 업데이트
    for (auto& item : m_downloads) {
        if (item.id == id) {
            item.state = state;
            break;
        }
    }

    qDebug() << "DownloadManager: 상태 변경 -" << id << "state:" << static_cast<int>(state);

    emit downloadStateChanged(id, state);
}

// Private Methods

QString DownloadManager::generateDownloadId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString DownloadManager::resolveFilePath(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.baseName();
    QString suffix = fileInfo.suffix();
    QString dirPath = fileInfo.absolutePath();

    if (!QFile::exists(filePath)) {
        return filePath;  // 중복 없음
    }

    // 중복 시 번호 추가
    int counter = 1;
    QString newFilePath;
    do {
        if (suffix.isEmpty()) {
            newFilePath = QString("%1/%2 (%3)")
                          .arg(dirPath)
                          .arg(baseName)
                          .arg(counter++);
        } else {
            newFilePath = QString("%1/%2 (%3).%4")
                          .arg(dirPath)
                          .arg(baseName)
                          .arg(counter++)
                          .arg(suffix);
        }
    } while (QFile::exists(newFilePath));

    qDebug() << "DownloadManager: 파일명 중복 해결 -" << filePath << "→" << newFilePath;

    return newFilePath;
}

qreal DownloadManager::calculateSpeed(const QString& id) const
{
    if (!m_lastBytesReceived.contains(id) || !m_lastUpdateTime.contains(id)) {
        return 0.0;
    }

    // 현재 다운로드 정보
    DownloadItem item = getDownloadById(id);
    qint64 currentBytes = item.bytesReceived;
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    // 이전 정보
    qint64 lastBytes = m_lastBytesReceived[id];
    qint64 lastTime = m_lastUpdateTime[id];

    // 시간 차이 (초 단위)
    qreal timeDiff = (currentTime - lastTime) / 1000.0;
    if (timeDiff <= 0) {
        return 0.0;
    }

    // 속도 계산 (bytes/sec)
    qreal speed = (currentBytes - lastBytes) / timeDiff;

    return speed > 0 ? speed : 0.0;
}

qint64 DownloadManager::estimateTimeLeft(qint64 bytesReceived, qint64 bytesTotal, qreal speed) const
{
    if (speed <= 0 || bytesTotal <= 0 || bytesReceived >= bytesTotal) {
        return -1;  // 알 수 없음
    }

    qint64 bytesLeft = bytesTotal - bytesReceived;
    qint64 secondsLeft = static_cast<qint64>(bytesLeft / speed);

    return secondsLeft;
}

DownloadState DownloadManager::convertQtState(QWebEngineDownloadItem::DownloadState qtState) const
{
    switch (qtState) {
        case QWebEngineDownloadItem::DownloadRequested:
            return DownloadState::Pending;
        case QWebEngineDownloadItem::DownloadInProgress:
            return DownloadState::InProgress;
        case QWebEngineDownloadItem::DownloadCompleted:
            return DownloadState::Completed;
        case QWebEngineDownloadItem::DownloadCancelled:
            return DownloadState::Cancelled;
        case QWebEngineDownloadItem::DownloadInterrupted:
            return DownloadState::Failed;
        default:
            return DownloadState::Pending;
    }
}

QString DownloadManager::findDownloadId(QWebEngineDownloadItem* downloadItem) const
{
    for (auto it = m_activeDownloads.constBegin(); it != m_activeDownloads.constEnd(); ++it) {
        if (it.value() == downloadItem) {
            return it.key();
        }
    }
    return QString();
}

void DownloadManager::updateDownloadState(const QString& id, DownloadState state, const QString& errorMessage)
{
    for (auto& item : m_downloads) {
        if (item.id == id) {
            item.state = state;
            if (!errorMessage.isEmpty()) {
                item.errorMessage = errorMessage;
            }
            break;
        }
    }
}

} // namespace webosbrowser
