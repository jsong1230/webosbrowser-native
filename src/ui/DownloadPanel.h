#pragma once

#include "../services/DownloadManager.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QProgressBar>
#include <QKeyEvent>

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
    /**
     * @brief 생성자
     * @param downloadManager 다운로드 관리자
     * @param parent 부모 위젯
     */
    explicit DownloadPanel(DownloadManager* downloadManager, QWidget* parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~DownloadPanel() override;

    // 복사 생성자 및 대입 연산자 삭제
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
     * @brief 완료된 파일 열기
     * @param filePath 파일 경로
     */
    void openDownloadedFile(const QString& filePath);

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
    /**
     * @brief 생성자
     * @param item 다운로드 항목
     * @param parent 부모 위젯
     */
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

    /**
     * @brief 파일 크기 포맷
     * @param bytes 바이트
     * @return 포맷된 문자열
     */
    QString formatFileSize(qint64 bytes) const;

    /**
     * @brief 다운로드 속도 포맷
     * @param bytesPerSec 초당 바이트
     * @return 포맷된 문자열
     */
    QString formatSpeed(qreal bytesPerSec) const;

    /**
     * @brief 남은 시간 포맷
     * @param seconds 초
     * @return 포맷된 문자열
     */
    QString formatTimeLeft(qint64 seconds) const;
};

} // namespace webosbrowser
