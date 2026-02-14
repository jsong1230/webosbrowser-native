#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QDateTime>
#include <QVector>
#include <QMap>
#include <QWebEngineDownloadItem>

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

    /**
     * @brief 기본 생성자
     */
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
    /**
     * @brief 생성자
     * @param parent 부모 QObject
     */
    explicit DownloadManager(QObject* parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~DownloadManager() override;

    // 복사 생성자 및 대입 연산자 삭제 (QObject 패턴)
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

    /**
     * @brief QWebEngineDownloadItem으로부터 ID 찾기
     * @param downloadItem Qt 다운로드 항목
     * @return 다운로드 ID (없으면 빈 문자열)
     */
    QString findDownloadId(QWebEngineDownloadItem* downloadItem) const;

    /**
     * @brief DownloadItem 업데이트 (내부 헬퍼)
     * @param id 다운로드 ID
     * @param state 새 상태
     * @param errorMessage 에러 메시지 (선택)
     */
    void updateDownloadState(const QString& id, DownloadState state, const QString& errorMessage = QString());

    // 멤버 변수
    QString m_downloadPath;  ///< 다운로드 저장 경로
    QVector<DownloadItem> m_downloads;  ///< 다운로드 목록 (메타데이터)
    QMap<QString, QWebEngineDownloadItem*> m_activeDownloads;  ///< 진행 중 다운로드 (Qt 객체)
    QMap<QString, qint64> m_lastBytesReceived;  ///< 속도 계산용 이전 바이트
    QMap<QString, qint64> m_lastUpdateTime;     ///< 속도 계산용 이전 시각 (msecs)

    static constexpr int MAX_CONCURRENT_DOWNLOADS = 3;  ///< 최대 동시 다운로드 수
};

} // namespace webosbrowser

// Qt 메타타입 등록
Q_DECLARE_METATYPE(webosbrowser::DownloadItem)
Q_DECLARE_METATYPE(webosbrowser::DownloadState)
